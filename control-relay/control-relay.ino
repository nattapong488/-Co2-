#include <WiFiClientSecure.h>
#include <PubSubClient.h>


const char* ssid = ""; // แก้ ssid
const char* password = ""; // แก้ password

bool check = false;

// Config MQTT Server
const char * topic = ""; // topic ชื่อ /server
#define mqtt_server "" // server
#define mqtt_port 1883// เลข port
#define mqtt_user "" // user
#define mqtt_password "" // password

// static ip***************************
IPAddress local_IP(, , , );
IPAddress gateway(, , , );
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(, , , );
//********************************

const int LED_PIN1 = 2;
const int RELAY = 26;
char *led_status = "OFF";

WiFiClient espClientt;
PubSubClient client(espClientt);

void setup() {
  //set static ip
  WiFi.config(local_IP, primaryDNS, gateway, subnet);

  // เชื่อมต่อ network
  pinMode(LED_PIN1, OUTPUT);
  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, HIGH);
  Serial.begin(115200);
  delay(1000);
  Serial.println();

  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  // จบการเชื่อมต่อ network

  client.setServer(mqtt_server, mqtt_port); // เชื่อมต่อ mqtt server
  client.setCallback(callback); // สร้างฟังก์ชันเมื่อมีการติดต่อจาก mqtt มา
}

void loop() {
  if (!client.connected()) {
    Serial.print("MQTT connecting...");
    if (client.connect("ESP32Client", mqtt_user, mqtt_password)) { // ถ้าเชื่อมต่อ mqtt สำเร็จ
      
      client.subscribe("esp32/Co2"); // ชื่อ topic ที่ต้องการติดตาม
      client.subscribe("esp32/relay-control"); // ชื่อ topic ที่ต้องการติดตาม
      client.subscribe("esp32/Co2-min");
      client.subscribe("esp32/Co2-max");
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
  client.loop();
}
int gas_min = 800;
int gas_max = 1300;
int gas = 0;
int re_gas = 0;
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("\n");
  Serial.println("Message from ");
  //Serial.println(topic);
  String msg = ""; //สร้าง string มาเก็บข้อมูลจาก payload
  int i = 0;
  String numstring;

  while (i < length) {
    msg += (char)payload[i++]; // อ่านข้อความจาก topic ที่ส่งมา

  }

  if (strcmp(topic, "esp32/Co2-min") == 0) {
    Serial.print("topic: ");
    Serial.println(topic);
    Serial.print("min-gas = ");
    /*if (strcmp(a, "min") == 0) {
      int longmsg = msg.length();
      numstring = msg.substring(3, longmsg);

      }*/
    gas_min = msg.toInt(); //แปลง msg type string เป็น int
    Serial.println(gas_min);
    Serial.print("boo: ");
    Serial.println(check);
  }
  else if (strcmp(topic, "esp32/Co2-max") == 0) {
    /*if (strcmp(a, "max") == 0) {
      int longmsg = msg.length();
      numstring = msg.substring(3, longmsg);

      }*/
    Serial.print("topic: ");
    Serial.println(topic);
    Serial.print("max-gas = ");

    gas_max = msg.toInt();
    Serial.println(gas_max);
    Serial.print("boo: ");
    Serial.println(check);
  }

  else if (strcmp(topic, "esp32/Co2") == 0) {
    Serial.print("topic: ");
    Serial.println(topic);
    Serial.print("receive Co2 ");
    Serial.println(msg); // แสดงข้อความที่ได้รับจาก topic จากตัวแรก
    //แปลง string เป็น int
    payload[length] = '\0';
    gas = atoi((char *)payload);
    //-------
    re_gas = gas;
    Serial.print("min: ");
    Serial.println(gas_min);
    Serial.print("max: ");
    Serial.println(gas_max);
    Serial.print("boo: ");
    Serial.println(check);
    //min-max

    // เช็ค auto
    // แสดงข้อความตอบกลับที่ส่งไปที่ตัวแรก
    //control_valve(msg);

    if (gas <= gas_min && check == true) {

      digitalWrite(RELAY, LOW);
      Serial.print("min = ");
      Serial.println(gas_min);
      Serial.println("open gas auto");

      client.publish("esp32/status-relay", "open gas auto");
      //Line_Notify1(message2);
    }
    if (gas >= gas_max && check == true) {

      digitalWrite(RELAY, HIGH);
      Serial.print("max = ");
      Serial.println(gas_max);
      Serial.println("close gas auto");
      client.publish("esp32/status-relay", "close gas auto");

    }

  }

  else if (strcmp(topic, "esp32/relay-control") == 0) {

    Serial.print("topic : ");
    Serial.println(topic);
    Serial.print("receive order ");
    Serial.println(msg);

    Serial.print("Co2 = ");
    Serial.println(re_gas);
    control_valve(msg);
    /* if (re_gas <= gas_min && check == true) {

       digitalWrite(RELAY, LOW);
       Serial.print("min = ");
       Serial.println(gas_min);
       Serial.println("open gas auto");

       client.publish("esp32/status-relay", "open gas auto");
       //Line_Notify1(message2);
      }
      if (re_gas >= gas_max && check == true) {

       digitalWrite(RELAY, HIGH);
       Serial.print("max = ");
       Serial.println(gas_max);
       Serial.println("close gas auto");
       client.publish("esp32/status-relay", "close gas auto");

      }*/
    Serial.print("boo: ");
    Serial.println(check);

  }
  Serial.println("\n");
}


void control_valve(String msgg) {
  //function
  Serial.println(msgg);
  if (msgg == "start") {
    check = true;
    client.publish("esp32/status-relay", "start gas auto");
  }
  else if (msgg == "stop") {
    check = false;
    digitalWrite(RELAY, HIGH);
    client.publish("esp32/status-relay", "stop gas auto");
  }
  else if (msgg == "on") {
    check = false;
    digitalWrite(RELAY, LOW);
    client.publish("esp32/status-relay", "open gas");
  }
  else if (msgg == "off") {
    check = false;
    digitalWrite(RELAY, HIGH);
    client.publish("esp32/status-relay", "close gas");
  }
  Serial.print("boo: ");
  Serial.println(check);


}
