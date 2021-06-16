#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SimpleDHT.h>

#define MOTOR D5
#define LED D6 //led

#define sensorSoil A0 //pin sensor LDR

const char *ssid = "OPPO F11";    //silakan disesuaikan sendiri
const char *password = "evosevosevos"; //silakan disesuaikan sendiri

const char *mqtt_server = "ec2-54-242-188-102.compute-1.amazonaws.com";

WiFiClient espClient;
PubSubClient client(espClient);

SimpleDHT11 dht11(D7);

long now = millis();
long lastMeasure = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];

int soilVal = 0;

void ledON()
{
  digitalWrite(LED, LOW);
}

void ledOFF()
{
  digitalWrite(LED, HIGH);
}

void pumpON()
{
  digitalWrite(MOTOR, HIGH);
  delay(3000);
}

void pumpOFF()
{
  digitalWrite(MOTOR, LOW);
}

void setup_wifi()
{
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
    messageTemp += (char)payload[i];
  }
  Serial.println();

  if (String(topic) == "room/lamp")
  {
    Serial.print("Changing Room lamp to ");
    if (messageTemp == "on")
    {
      ledON();
      Serial.print("On");
    }
    else if (messageTemp == "off")
    {
      ledOFF();
      Serial.print("Off");
    }
  }
  else if (String(topic) == "room/motor")
  {
    Serial.print("Changing Water pump to ");
    if (messageTemp == "on")
    {
      pumpON();
      Serial.print("On");
    }
    else if (messageTemp == "off")
    {
      pumpOFF();
      Serial.print("Off");
    }
  }
  Serial.println();
}

void reconnect()
{
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");

    // Attempt to connect
    if (client.connect("ESP8266Client", "jti", "123"))
    {
      Serial.println("connected");
      client.subscribe("room/lamp");
      client.subscribe("room/motor");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup()
{
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  pinMode(MOTOR, OUTPUT);
  ledOFF();
  Serial.println("Mqtt Node-RED");
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
  if (!client.loop())
  {
    client.connect("ESP8266Client");
  }
  now = millis();
  if (now - lastMeasure > 3000)
  {
    lastMeasure = now;
    int err = SimpleDHTErrSuccess;
    
    soilVal = analogRead(sensorSoil);
    byte temperature = 0;
    byte humidity = 0;
    if ((err = dht11.read(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess)
    {
      Serial.print("Pembacaan DHT11 gagal, err=");
      Serial.println(err);
      delay(1000);
      return;
    }
    static char temperatureTemp[7];
    static char humidityTemp[7];
    static char soilTemp[7];
    dtostrf(temperature, 4, 2, temperatureTemp);
    dtostrf(humidity, 4, 2, humidityTemp);
    dtostrf(soilVal, 4, 2, soilTemp);
    Serial.print("Temperature = ");
    Serial.println(temperatureTemp);
    Serial.print("Humidity = ");
    Serial.println(humidityTemp);
    Serial.print("Soil Moisture = ");
    Serial.println(soilTemp);
    if (soilVal >= 600)
    {
      pumpON();
    }
    else
    {
      pumpOFF();
    }
    client.publish("room/suhu", temperatureTemp);
    client.publish("room/humidity", humidityTemp);
    client.publish("room/soil", soilTemp);
  }
}
