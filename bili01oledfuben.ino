/**********************************************************************
 bilibili粉丝数监视器+天气显示

基于flyAkari 会飞的阿卡林 bilibili UID:751219 的代码修改
感谢UP：Hans叫泽涵UP： 小年轻只爱她提供的灵感修改！
 **********************************************************************/
/* 4pin IIC引脚，正面看，从左到右依次为GND、VCC、SCL、SDA
 *      ESP01  ---  OLED
 *      3.3V     ---  VCC
 *      G (GND)  ---  GND
 *      0(GPIO0)---  SCL
 *      2(GPIO2)---  SDA
 */
#include <Arduino.h>
#include <U8g2lib.h>
#include <stdio.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif
U8G2_SSD1306_128X64_NONAME_1_SW_I2C u8g2(U8G2_R0, /* clock=*/ 0, /* data=*/ 2, /* reset=*/ U8X8_PIN_NONE); //修改为你的引脚
const unsigned char col[] U8X8_PROGMEM= { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
0x00,0x00,0x00,0x00,0x00,0x80,0x03,0x00,0x00,0x00,0x00,0x00,0x80,0x03,0x00,0x00,
0x00,0x00,0x00,0xC0,0x07,0x00,0x00,0x00,0x00,0x00,0xC0,0x0F,0x00,0x00,0x00,0x00,
0x00,0xE0,0x0F,0x00,0x00,0x00,0x00,0x00,0xE0,0x0F,0x00,0x00,0x00,0x00,0x00,0xF0,
0x1F,0x00,0x00,0x00,0x00,0x00,0xF0,0x1F,0x00,0x00,0x00,0x00,0x00,0xF8,0x3F,0x00,
0x00,0x00,0x00,0x00,0xF8,0x7F,0x00,0x00,0x00,0x00,0x00,0xFC,0xFF,0x00,0x00,0x00,
0x00,0xC0,0xFF,0xFF,0x0F,0x00,0x00,0xC0,0xFF,0xFF,0xFF,0xFF,0x03,0x00,0xF0,0xFF,
0xFF,0xFF,0xFF,0x1F,0x00,0xF0,0xFF,0xFF,0xFF,0xFF,0x1F,0x00,0xE0,0xFF,0xFF,0xFF,
0xFF,0x0F,0x00,0xC0,0xFF,0xDF,0xF7,0xFF,0x07,0x00,0x80,0xFF,0x8F,0xE3,0xFF,0x07,
0x00,0x00,0xFF,0xBF,0xEF,0xFF,0x03,0x00,0x00,0xFF,0xBF,0xEF,0xFF,0x03,0x00,0x00,
0xFE,0x9F,0xE7,0xFF,0x00,0x00,0x00,0xFC,0xDF,0xF7,0x7F,0x00,0x00,0x00,0xF8,0xFF,
0xFF,0x3F,0x00,0x00,0x00,0xF0,0xFF,0xFF,0x1F,0x00,0x00,0x00,0xE0,0xFF,0xFF,0x0F,
0x00,0x00,0x00,0xE0,0xFF,0xFF,0x0F,0x00,0x00,0x00,0xE0,0xFF,0xFF,0x0F,0x00,0x00,
0x00,0xE0,0xFF,0xFF,0x0F,0x00,0x00,0x00,0xE0,0xFF,0xFF,0x0F,0x00,0x00,0x00,0xF0,
0xFF,0xFF,0x1F,0x00,0x00,0x00,0xF0,0xFF,0xFF,0x1F,0x00,0x00,0x00,0xF0,0xFF,0xFF,
0x1F,0x00,0x00,0x00,0xF0,0xFF,0xFF,0x1F,0x00,0x00,0x00,0xF0,0x7F,0xFC,0x1F,0x00,
0x00,0x00,0xF0,0x1F,0xF0,0x3F,0x00,0x00,0x00,0xF8,0x0F,0xE0,0x3F,0x00,0x00,0x00,
0xF8,0x03,0x80,0x3F,0x00,0x00,0x00,0xF8,0x00,0x00,0x3E,0x00,0x00,0x00,0x38,0x00,
0x00,0x38,0x00,0x00,0x00,0x08,0x00,0x00,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};//换成你喜欢的图片
static const unsigned char PROGMEM cor[][32] =
{
{0x00,0x04,0x00,0x04,0xDE,0x7F,0x12,0x04,0x92,0x3F,0x12,0x04,0xD2,0x7F,0x1E,0x00,0x92,0x3F,0x92,0x20,0x92,0x3F,0x92,0x20,0x9E,0x3F,0x92,0x20,0x80,0x28,0x80,0x10},//"晴",
{0x00,0x00,0xBE,0x3F,0xA2,0x20,0x92,0x20,0x92,0x20,0x8A,0x3F,0x92,0x20,0x92,0x20,0xA2,0x20,0xA2,0x3F,0xA2,0x20,0x96,0x20,0x4A,0x20,0x42,0x20,0x22,0x28,0x12,0x10},//"阴",
{0x40,0x00,0x40,0x00,0xE0,0x0F,0x10,0x04,0x1C,0x02,0x20,0x01,0xC0,0x02,0x30,0x01,0x8E,0x1F,0x40,0x10,0x30,0x08,0x4C,0x04,0x80,0x02,0x80,0x01,0x70,0x00,0x0E,0x00},//"多",
{0x00,0x00,0xFC,0x1F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x7F,0x40,0x00,0x20,0x00,0x20,0x00,0x10,0x02,0x08,0x04,0x04,0x08,0xFE,0x1F,0x04,0x10,0x00,0x10},//"云",
{0x80,0x00,0x80,0x00,0x80,0x00,0x80,0x00,0x80,0x00,0x88,0x08,0x88,0x10,0x88,0x20,0x84,0x20,0x84,0x40,0x82,0x40,0x81,0x40,0x80,0x00,0x80,0x00,0xA0,0x00,0x40,0x00},//"小",
{0x80,0x00,0x80,0x00,0x80,0x00,0x80,0x00,0xFC,0x1F,0x84,0x10,0x84,0x10,0x84,0x10,0x84,0x10,0x84,0x10,0xFC,0x1F,0x84,0x10,0x80,0x00,0x80,0x00,0x80,0x00,0x80,0x00},//"中",
{0x80,0x00,0x80,0x00,0x80,0x00,0x80,0x00,0x80,0x00,0xFF,0x7F,0x80,0x00,0x80,0x00,0x40,0x01,0x40,0x01,0x20,0x02,0x20,0x02,0x10,0x04,0x08,0x08,0x04,0x10,0x03,0x60},//*"大",
{0x00,0x00,0xFF,0x7F,0x80,0x00,0x80,0x00,0x80,0x00,0xFE,0x3F,0x82,0x20,0x82,0x20,0x92,0x22,0xA2,0x24,0x82,0x20,0x92,0x22,0xA2,0x24,0x82,0x20,0x82,0x28,0x02,0x10},//*"雨",
{0xFC,0x1F,0x80,0x00,0xFE,0x7F,0x82,0x40,0xB9,0x2E,0x80,0x00,0xB8,0x0E,0x00,0x00,0xFC,0x1F,0x00,0x10,0x00,0x10,0xF8,0x1F,0x00,0x10,0x00,0x10,0xFC,0x1F,0x00,0x10}//*"雪",
};
//---------------修改此处""内的信息-----------------------
const char *ssid = "MERCURY_85A8";          //WiFi名
const char *password = "88888888";  //WiFi密码
String biliuid = "4694767";         //bilibili UID
const char* xinzhi= "GET /v3/weather/now.json?key=你的密钥&location=你的城市&language=zh-Hans&unit=c HTTP/1.1\r\n";//-- 使用时请修改为当前你的私秘钥和你的城市拼音
//-------------------------------------------------------

const unsigned long HTTP_TIMEOUT = 5000;
WiFiClient client;
HTTPClient http;
String response;
int follower = 0;
void parseUserData(String content)  // Json数据解析并串口打印.可参考https://www.bilibili.com/video/av65322772
{
 const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(1) + 2*JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(6) + 210;
 //DynamicJsonBuffer jsonBuffer(capacity);
 DynamicJsonDocument jsonBuffer(capacity);
 deserializeJson(jsonBuffer, content);
 //JsonObject& root = jsonBuffer.parseObject(content);
 //JsonObject& results_0 = root["results"][0];
 JsonObject results_0=jsonBuffer["results"][0];
 JsonObject results_0_location = results_0["location"];
 //JsonObject& results_0_location = results_0["location"];
 const char* results_0_location_id = results_0_location["id"];
 const char* results_0_location_name = results_0_location["name"];
 const char* results_0_location_country = results_0_location["country"];
 const char* results_0_location_path = results_0_location["path"];
 const char* results_0_location_timezone = results_0_location["timezone"];
 const char* results_0_location_timezone_offset = results_0_location["timezone_offset"];
 
 JsonObject results_0_now = results_0["now"];
 const char* results_0_now_text = results_0_now["text"];
 const char* results_0_now_code = results_0_now["code"];
 const char* results_0_now_temperature = results_0_now["temperature"];
 const char* results_0_last_update = results_0["last_update"];
  u8g2.setFont(u8g2_font_unifont_t_chinese2); 
    u8g2.setFontDirection(0);
    u8g2.setCursor(50, 40);
    u8g2.print(results_0_now_temperature);
    u8g2.setCursor(50, 30);
  u8g2.print("TEMP:");
  u8g2.setCursor(50, 50);
  u8g2.print("WEATHER:");
 
 switch(int(results_0_now["code"])){
  case 1:
  case 0:
  u8g2.drawXBMP( 50 , 50 , 16 , 16 , cor[0] ); 
 //u8g2.print("Sunny");
 break;
 case 4:
 case 5:
 case 6:
 case 7:
 case 8:
 u8g2.drawXBMP( 50 , 50 , 16 , 16 , cor[2] );
 u8g2.drawXBMP( 66 , 50 , 16 , 16 , cor[3] );
 break;
 case 9:
 u8g2.drawXBMP( 50 , 50 , 16 , 16 , cor[1] );
 break;
 case 13:
 u8g2.drawXBMP( 50 , 50 , 16 , 16 , cor[4] );
 u8g2.drawXBMP( 66 , 50 , 16 , 16 , cor[7] );
 break;
 case 14:
 u8g2.drawXBMP( 50 , 50 , 16 , 16 , cor[5] );
 u8g2.drawXBMP( 66 , 50 , 16 , 16 , cor[7] );
 break;
 case 11:
 case 12:
 case 15:
 case 16:
 case 17:
 case 18:
 case 19:
  u8g2.drawXBMP( 50 , 50 , 16 , 16 , cor[6] );
 u8g2.drawXBMP( 66 , 50 , 16 , 16 , cor[7] );
 break;
 case 22:
 u8g2.drawXBMP( 50 , 50 , 16 , 16 , cor[4] );
 u8g2.drawXBMP( 66 , 50 , 16 , 16 , cor[8] );
 break;
 case 23:
 u8g2.drawXBMP( 50 , 50 , 16 , 16 , cor[5] );
 u8g2.drawXBMP( 66 , 50 , 16 , 16 , cor[8] );
 break;
 case 24:
 case 25:
 u8g2.drawXBMP( 50 , 50 , 16 , 16 , cor[6] );
 u8g2.drawXBMP( 66 , 50 , 16 , 16 , cor[8] );
 break;
 }
 Serial.println(results_0_location_name);                       //通过串口打印出需要的信息
 Serial.println(results_0_now_text);
 Serial.println(results_0_now_code);
 Serial.println(results_0_now_temperature);
 Serial.println(results_0_last_update);
 Serial.print("\r\n");
}

void setup()
{
    Serial.begin(9600);
    while (!Serial)
        continue;
    Serial.println("bilibili fans monitor, version v1.2");
     u8g2.begin();
  u8g2.enableUTF8Print(); 
    
  
    
    Serial.println("OLED Ready");

    Serial.print("Connecting WiFi...");
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED){
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

bool getJson()
{
    bool r = false;
    http.setTimeout(HTTP_TIMEOUT);
    http.begin("http://api.bilibili.com/x/relation/stat?vmid=" + biliuid);
    int httpCode = http.GET();
    if (httpCode > 0){
        if (httpCode == HTTP_CODE_OK){
            response = http.getString();
            //Serial.println(response);
            r = true;
        }
    }else{
        Serial.printf("[HTTP] GET JSON failed, error: %s\n", http.errorToString(httpCode).c_str());
        u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_ncenB08_tr); 
    u8g2.setFontDirection(0);
    u8g2.setCursor(0, 32);
  u8g2.print("Error");
  } while ( u8g2.nextPage() );
        r = false;
    }
    http.end();
    return r;
}

bool parseJson(String json)
{
    const size_t capacity = JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) + 70;
    DynamicJsonDocument doc(capacity);
    deserializeJson(doc, json);

    int code = doc["code"];
    const char *message = doc["message"];

    if (code != 0){
        Serial.print("[API]Code:");
        Serial.print(code);
        Serial.print(" Message:");
        Serial.println(message);

        return false;
    }

    JsonObject data = doc["data"];
    unsigned long data_mid = data["mid"];
    int data_follower = data["follower"];
    if (data_mid == 0){
        Serial.println("[JSON] FORMAT ERROR");
        return false;
    }
    Serial.print("UID: ");
    Serial.print(data_mid);
    Serial.print(" follower: ");
    Serial.println(data_follower);

    follower = data_follower;
    return true;
}
void tqyb()
{
if(client.connect("api.seniverse.com", 80)==1)              //连接服务器并判断是否连接成功，若成功就发送GET 请求数据下发       
  {                                           
   client.print(xinzhi); //心知天气的URL格式          
   client.print("Host:api.seniverse.com\r\n");
   client.print("Accept-Language:zh-cn\r\n");
   client.print("Connection:close\r\n\r\n"); //向心知天气的服务器发送请求。
 
                 
   String status_code = client.readStringUntil('\r');        //读取GET数据，服务器返回的状态码，若成功则返回状态码200
   Serial.println(status_code);
      
   if(client.find("\r\n\r\n")==1)                            //跳过返回的数据头，直接读取后面的JSON数据，
     {
      String json_from_server=client.readStringUntil('\n');  //读取返回的JSON数据
      Serial.println(json_from_server);
      parseUserData(json_from_server);                      //将读取的JSON数据，传送到JSON解析函数中进行显示。
     }
  }
else                                        
  { 
   Serial.println("connection failed this time");
   delay(5000);                                            //请求失败等5秒
  } 
     
client.stop();                                            //关闭HTTP客户端，采用HTTP短链接，数据请求完毕后要客户端要主动断开https://blog.csdn.net/huangjin0507/article/details/52396580
delay(5000); 
}
void loop()
{
  if (WiFi.status() == WL_CONNECTED){
        if (getJson()){
            if (parseJson(response)){
                u8g2.firstPage();
  do {
    tqyb();
    u8g2.setFont(u8g2_font_ncenB08_tr); 
    u8g2.setFontDirection(0);
    u8g2.setCursor(50, 10);
  u8g2.print("Follower:");
    u8g2.setCursor(50, 20);
  u8g2.print(follower);
  u8g2.drawXBMP( 0 , 0 , 50 , 50 , col );
  } while ( u8g2.nextPage() );
            }
        }
    }
    else{
        Serial.println("[WiFi] Waiting to reconnect...");
        u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_ncenB08_tr); 
    u8g2.setFontDirection(0);
    u8g2.setCursor(0, 32);
  u8g2.print("Error");
  u8g2.drawXBMP( 50 , 0 , 50 , 50 , col );
  } while ( u8g2.nextPage() );
    }
    delay(1000);
}
