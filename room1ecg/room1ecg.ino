#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>

const char* ssid = "SRKR_IDEALAB";
const char* password = "Tech$9889";
const char* mqtt_server = "192.168.68.182";  // Local IP address of Raspberry Pi
const char* username = "";
const char* pass = "";

#define Relay1   D0
#define Relay2   D1
#define IRPIN1   D4
#define IRPIN2   D3
#define sensor   A0
#define loplus   D5
#define lominu   D6

WiFiClient espClient;
PubSubClient client(espClient);

#define sub1 "device1/relay1"
#define sub2 "device1/relay2"

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  if (strstr(topic, sub1)) {
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
    }
    Serial.println();
    if ((char)payload[0] == '1') {
      digitalWrite(Relay1, HIGH);
    } else {
      digitalWrite(Relay1, LOW);
    }
  }
  else if (strstr(topic, sub2)) {
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
    }
    Serial.println();
    if ((char)payload[0] == '1') {
      digitalWrite(Relay2, HIGH);
    } else {
      digitalWrite(Relay2, LOW);
    }
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), username, pass)) {
      Serial.println("connected");
      client.subscribe(sub1);
      client.subscribe(sub2);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  pinMode(Relay1, OUTPUT);
  pinMode(Relay2, OUTPUT);
  pinMode(IRPIN1, INPUT);
  pinMode(IRPIN2, INPUT);
  pinMode(loplus, INPUT);
  pinMode(lominu, INPUT);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  static unsigned long lastMsg = 0;
  if (now - lastMsg > 2000) {
    int bs1 = digitalRead(IRPIN1);
    int bs2 = digitalRead(IRPIN2);
    int Svalue = analogRead(sensor) / 15;

    // Read ECG sensor value (placeholder code)
    int ecgValue = analogRead(A0);

    lastMsg = now;
    char str_ir_data1[10];
    char str_ir_data2[10];
    char str_ecg_value[10];
   
    dtostrf(bs1, 4, 4, str_ir_data1);
    dtostrf(bs2, 4, 4, str_ir_data2);
    dtostrf(ecgValue, 4, 4, str_ecg_value);

    Serial.print("ir1 - ");
    Serial.println(str_ir_data1);
    client.publish("device1/call1", str_ir_data1);
    Serial.print("ir2 - ");
    Serial.println(str_ir_data2);
    client.publish("device1/call2", str_ir_data2);

    Serial.print("ECG Value - ");
    Serial.println(str_ecg_value);
    client.publish("device1/ecg", str_ecg_value);
  }
}
