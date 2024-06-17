#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define sensor A0
int Svalue;
const char* ssid = "SRKR_IDEALAB";
const char* password = "Tech$9889";
const char* mqtt_server = "192.168.68.182";  // Local IP address of Raspberry Pi
const char* username = "";
const char* pass = "";

char str_temp_cel[10];
char str_temp_faren[10];
char str_heart_data[10];
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;
const int oneWireBus = 4;     
// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);
// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);
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
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), username, pass)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883); // Start the DS18B20 sensor
  sensors.begin();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    sensors.requestTemperatures(); 
    float temperatureC = sensors.getTempCByIndex(0);
    dtostrf(temperatureC, 4, 2, str_temp_cel);
    float temperatureF = sensors.getTempFByIndex(0);
    dtostrf(temperatureF, 4, 2, str_temp_faren);
    Serial.print("Temperature: ");
    Serial.print(temperatureC);
    Serial.print("ºC - ");
    Serial.print(temperatureF);
    Serial.println("ºF");
    Svalue = analogRead(sensor) / 14;
    dtostrf(Svalue, 4, 4, str_heart_data);
    lastMsg = now;
    Serial.print("Publish message: ");
    Serial.print("Heartbeat - "); 
    Serial.println(str_heart_data);
    client.publish("device1/bpm", str_heart_data);
    client.publish("device1/tempc", str_temp_cel);
    client.publish("device1/tempF", str_temp_faren);
  }
}
