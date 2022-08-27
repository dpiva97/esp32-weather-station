#include <stdlib.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

Adafruit_BME280 bme; // I2C
#define TEMPERATURE_BUS 33

const char *ssid = "ssid"; //wifi name
const char *password = "password"; //wifi password
const String serverName  = "serverName"; //api server name

OneWire oneWire(TEMPERATURE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress airThermometer = {0x28, 0xF4, 0xBC, 0x26, 0x0, 0x0, 0x80, 0x2B};

float tempair;
float tempBme;
float humidity;
float pressure;

//init wifi connection
void wifi_connect()
{
    int connectTries = 0;

    WiFi.mode(WIFI_OFF);
    delay(200);
    WiFi.mode(WIFI_STA);

    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");

        if (connectTries > 20)
            wifi_connect();
        connectTries++;
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println(" ");
}

//activate sleeping mode
//sleeptime is the number of minutes
void sleep(int sleeptime)
{
    Serial.println("Weather station going to sleep...");
    esp_sleep_enable_timer_wakeup(sleeptime * 1000000);
    delay(1000);
    Serial.flush();
    esp_deep_sleep_start();
}

//read data from sensors
void read_data()
{
    sensors.requestTemperatures();
    delay(10);
    tempair = sensors.getTempC(airThermometer);
    tempBme = bme.readTemperature();
    pressure = bme.readPressure() / 100;
    humidity = bme.readHumidity();
}

//mock for tests
void read_data_mock()
{
    tempair = 1.999;
    tempBme = 2.449;
    pressure = 3.77;
    humidity = 4.8;
}

//print readed data on serial monitor
void print_data()
{
    Serial.print("Air temperature [°C]: ");
    Serial.println(tempair);
    Serial.print("Temperature : ");
    Serial.println(tempBme);
    Serial.print("Humidity: ");
    Serial.println(humidity);
    Serial.print("Pressure: ");
    Serial.println(pressure);
    Serial.println();
}

//send data to external api
void push_to_be()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        WiFiClient client;
        HTTPClient http;
        char httpRequestData[110];
        sprintf(httpRequestData, "example-api?temp=%.1f&temp2=%.1f&hum=%i&pres=%.1f", tempair, tempBme, int(humidity), pressure);
        Serial.println(serverName + httpRequestData);
        http.begin(serverName + httpRequestData);
        http.addHeader("Content-Type", "application/json");
        int httpCode = http.GET();
        if (httpCode < 0)
        {
            Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
        Serial.print("HTTP Response code: ");
        Serial.println(httpCode);
        http.end();
    }
}

void setup()
{
    Serial.begin(115200);
    Serial.println("\nWeather station powered on.\n");
    wifi_connect();

    sensors.begin();
    sensors.getAddress(airThermometer, 1);
    sensors.setResolution(airThermometer, 15);
    bme.begin(0x76);
}

void loop()
{
    read_data();
    //read_data_mock();
    print_data();
    push_to_be();
    sleep(600);
}
