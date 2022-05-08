#include "Adafruit_CCS811.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
Adafruit_CCS811 ccs;
#include <stdlib.h>

//config Wifi
const char* ssid = ""; // แก้ ssid
const char* password = ""; // แก้ password
//end config wifi

const char * topic = "esp32/Co2";
#define mqtt_server "" // server
#define mqtt_port 1883// เลข port
#define mqtt_user "" // user
#define mqtt_password "" // password

WiFiClient espClient; //สร้างตัวแปรเชื่อมต่อ wifi
PubSubClient client(espClient); //กำหนด MQTT ให้ใช้ Wifi ที่ตั้งไว้

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  //เริ่มการเชื่อมต่อ wifi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  //end connect network
  Serial.println("CCS811 test");
  //ตรวจสอบว่า sensor ทำงานไหม
  if (!ccs.begin()) {
    Serial.println("Failed to start sensor! Please check your wiring.");
    while (1);
  }
  
  // Wait for the sensor to be ready
  while (!ccs.available());

  client.setServer(mqtt_server, mqtt_port); // เชื่อมต่อ mqtt server
  client.setCallback(callback); // สร้างฟังก์ชันเมื่อมีการติดต่อจาก mqtt มา
}

void loop() {
  if (!client.connected()) {
    Serial.print("MQTT connecting...");
    if (client.connect("ESP32Client", mqtt_user, mqtt_password)) { // ถ้าเชื่อมต่อ mqtt สำเร็จ
      client.subscribe(topic); // ชื่อ topic ที่ต้องการติดตาม
      Serial.println("connected");
    }
    // ในกรณีเชื่อมต่อ mqtt ไม่สำเร็จ
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000); // หน่วงเวลา 5 วินาที แล้วลองใหม่
      return;
    }
  }
  int co2 = ccs.geteCO2();
  char mqtt_payload[30] = "";
  snprintf (mqtt_payload, 30, "%ld", co2); //จัดรูปแบบสตริงก่อนส่งไป MQTT
  client.publish(topic, mqtt_payload); //publish topic,ข้อความ
  delay(1000);
  if (ccs.available()) {   //แสดงผลออก serial monitor 
    if (!ccs.readData()) {
      /*
        Serial.print("CO2: ");
        Serial.print(ccs.geteCO2());
        Serial.print("ppm, TVOC: ");
        Serial.println(ccs.getTVOC());*/
    }
    else {
      Serial.println("ERROR!");
      while (1);
    }
  }
  client.loop();  //สั่งให้ตัวแปรของ mqtt ทำงานตลอดเวลา
}


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message from ");
  Serial.println(topic);
  String msg = "";
  int i = 0;

  while (i < length) {
    msg += (char)payload[i++]; // อ่านข้อความจาก topic ที่ส่งมา
  }
  //Serial.print("receive ");
  Serial.println(msg); // แสดงข้อความที่ได้รับจาก topic

}
