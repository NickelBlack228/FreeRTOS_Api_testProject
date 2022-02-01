#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif


#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <FreeRTOS.h>
#include "stream_buffer.h"

//static StreamBufferHandle_t stream_buffer_ex = NULL; //без понятия, надо ли использовать буфферы для работы с данными

const char *ssid = "yourssid";
const char *pwd = "yourpwd";

long inc_number;
double dec_number;

//TaskHandle_t Task1; 
//TaskHandle_t Task2;

WebServer server(80); /*declare webserver on 80 port*/

StaticJsonDocument<250> jsonDocument; /*declate json document*/
char buffer[250];

void connectingToNetwork() {
  WiFi.begin(ssid, pwd);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
 
  Serial.print("Connected. IP: ");
  Serial.println(WiFi.localIP());
}

void setup_routing() {
  server.on("/tasks", tasks_function);
  server.on("/task1", task_1_function_server);
  server.on("/task2", task_2_function_server); /*creating pages */
  server.begin(); /*starting our server */
}

void task_1_function_server(){
  create_json("task_1", dec_number);
  server.send(200, "application/json", buffer);
}

void task_2_function_server(){
  create_json("task_2", inc_number);
      server.send(200, "application/json", buffer);
}

void create_json(String task_name, double value) {
  jsonDocument.clear();
  jsonDocument["task_name"] =  task_name;
  jsonDocument["value"] = value;
  serializeJson(jsonDocument, buffer);
}

void add_json(String task_name, double value) {
  JsonObject obj = jsonDocument.createNestedObject();
  obj["task_name"] = task_name;
  obj["value"] = value;
}



void tasks_function() {
  jsonDocument.clear();
  add_json("task_1", dec_number);
  add_json("task_2", inc_number);
  serializeJson(jsonDocument, buffer);
  server.send(200, "application/json", buffer);
}

void task_2_function( void *pvParameters );
void task_1_function( void *pvParameters );

void setup_task1() {
   xTaskCreatePinnedToCore(
    task_1_function, /*Executeble function name*/
    "TaskDec", /*Task name*/
    8192, /*Stack size of task */
    NULL, /*Task parameter*/
    3, /*Priority of the task */
    NULL, /*Task handle to keep track of created task */
    ARDUINO_RUNNING_CORE); /*task pinned on 0 core*/
}

void setup_task2() {
   xTaskCreatePinnedToCore(
    task_2_function,
    "TaskInc",
    8192,
    NULL,
    3,
    NULL,
    ARDUINO_RUNNING_CORE); /*task pinned on 1 core*/
}

void setup() {
  Serial.begin(115200);
  connectingToNetwork();
  setup_task1();
  setup_task2();
  setup_routing();
}

void loop() {
   server.handleClient();
}

void task_1_function(void * parameter) {
  for (;;) {
      dec_number-=0.5;
      vTaskDelay(2000);
  }
}

void task_2_function(void * parameter) {
  for (;;) {
      inc_number +=1;
      vTaskDelay(1000);
  }
}
