
#define SSID        "zyh" //改为你的热点名称, 不要有中文
#define PASSWORD    "Zyh@20000812"//改为你的WiFi密码Wi-Fi密码
#define DEVICEID    "589271493" //OneNet上的设备ID
String apiKey = "S4fVwVkIiN11Gx3V=1cuEeMuA7A=";//与你的设备绑定的APIKey

/***/
#define HOST_NAME   "api.heclouds.com"
#define HOST_PORT   (80)
#define INTERVAL_SENSOR   5000             //定义传感器采样时间间隔  597000
#define INTERVAL_NET      5000             //定义发送时间
//传感器部分================================   
#include <Wire.h>                                  //调用库  
#include<ESP8266.h>
#define  sensorPin_1  A0           //定义光照传感器接口   
#define  sensorPin_2  D12
#define IDLE_TIMEOUT_MS  3000      // Amount of time to wait (in milliseconds) with no data 
                                   // received before closing the connection.  If you know the server
                                   // you're accessing is quick to respond, you can reduce this value.

//WEBSITE     
char buf[10];

#define INTERVAL_sensor 2000
unsigned long sensorlastTime = millis();

float lightnessOLED;

#define INTERVAL_OLED 1000

String mCottenData;
String jsonToSend;

int led1=5;                                   //LED灯引脚设置
int led2=6;
int led3=10;
//传感器值的设置 
int TrigPin = 2;    //将Trig的引脚定义为2号口
int EchoPin = 3;    //将Echo的引脚定义为3号口
int buttonpin=13; //定义避障传感器接口 
int val;          //定义数字变量 val ,在下面储存避障传感器的数值
float dist;    //定义一个变量，在下面存储HC-SR04返回的值
float sensor_lux;                    //光照   
char  sensor_lux_c[7] ;    //换成char数组传输
char  dist_c[10];
String buttonstate;        //定义避障传感器状态
#include <SoftwareSerial.h>
#define EspSerial mySerial
#define UARTSPEED  9600
SoftwareSerial mySerial(9, 11); // 对于esp8266 /* RX:D3, TX:D2 */
ESP8266 wifi(&EspSerial);
//ESP8266 wifi(Serial1);                                      //定义一个ESP8266（wifi）的对象
unsigned long net_time1 = millis();                          //数据上传服务器时间
unsigned long sensor_time = millis();                        //传感器采样时间计时器

//int SensorData;                                   //用于存储传感器数据
String postString;                                //用于存储发送数据的字符串
//String jsonToSend;                                //用于存储发送的json格式参数

void setup(void)     //初始化函数  
{       
  //初始化串口波特率  
    Wire.begin();
    Serial.begin(115200);
    while (!Serial); // wait for Leonardo enumeration, others continue immediately
    Serial.print(F("setup begin\r\n"));
    delay(100);
    pinMode(led1,OUTPUT);         //定义四脚LED的输出模式
    pinMode(led2,OUTPUT);
    pinMode(led3,OUTPUT);
    pinMode(sensorPin_1, INPUT);  //定义光照传感器为输出接口
    pinMode(buttonpin,INPUT);    //定义避障传感器为输出接口
    pinMode(TrigPin, OUTPUT);    //通过定义将Arduino开发板上TrigPin引脚(2号口)的工作模式转化为输出模式
    pinMode(EchoPin, INPUT);    //通过定义将Arduino开发板上EchoPin引脚(2号口)的工作模式转化为输入模式
  WifiInit(EspSerial, UARTSPEED);

  Serial.print(F("FW Version:"));
  Serial.println(wifi.getVersion().c_str());

  if (wifi.setOprToStationSoftAP()) {
    Serial.print(F("to station + softap ok\r\n"));
  } else {
    Serial.print(F("to station + softap err\r\n"));
  }

  if (wifi.joinAP(SSID, PASSWORD)) {
    Serial.print(F("Join AP success\r\n"));

    Serial.print(F("IP:"));
    Serial.println( wifi.getLocalIP().c_str());
  } else {
    Serial.print(F("Join AP failure\r\n"));
  }

  if (wifi.disableMUX()) {
    Serial.print(F("single ok\r\n"));
  } else {
    Serial.print(F("single err\r\n"));
  }

  Serial.print(F("setup end\r\n"));
    
  
}
void loop(void)     //循环函数  
{   
  setcolorA();
  if (sensor_time > millis())  sensor_time = millis();  
    
  if(millis() - sensor_time > INTERVAL_SENSOR)              //传感器采样时间间隔  
  {  
    getSensorData();                                        //读串口中的传感器数据
    if(val){
      setcolorB();
    }
    if(dist<30&&sensor_lux>300){
      setcolorB();
    }
    else if(dist<30&&sensor_lux<300&&sensor_lux>100){
      setcolorC();
    }
    else if(dist<50&&sensor_lux<100){
      setcolorD();
    }
    sensor_time = millis();
  }  

    
  if (net_time1 > millis())  net_time1 = millis();
  
  if (millis() - net_time1 > INTERVAL_NET)                  //发送数据时间间隔
  {                
    updateSensorData();                                     //将数据上传到服务器的函数
    net_time1 = millis();
  }
  
}

void setcolorA(){                                   //设置A级别光照强度
  analogWrite(led1,63);
  analogWrite(led2,63);
  analogWrite(led3,63);
}
void setcolorB(){                                   //设置B级别光照强度
  analogWrite(led1,127);
  analogWrite(led2,127);
  analogWrite(led3,127);
}
void setcolorC(){                                   //设置C级别光照强度
  analogWrite(led1,191);
  analogWrite(led2,191);
  analogWrite(led3,191);
}
void setcolorD(){                                   //设置D级别光照强度
  analogWrite(led1,255);
  analogWrite(led2,255);
  analogWrite(led3,255);
}
void getSensorData(){  
    sensor_lux = analogRead(A0);         //获取光照    
    delay(1000);
    dtostrf(sensor_lux, 3, 1, sensor_lux_c);
    digitalWrite(TrigPin, LOW);    //发送低电平、发一个短时间脉冲去TrigPin
    delayMicroseconds(5);
    digitalWrite(TrigPin, HIGH);    //发送高电平、发一个短时间脉冲去TrigPin
    delayMicroseconds(10);
    digitalWrite(TrigPin, LOW);    //发送低电平脉冲去TrigPin
    dist = pulseIn(EchoPin, HIGH) / 58.00; //将pulseIn换算成cm，并赋值给dist变量
    dtostrf(dist, 8, 2, dist_c);
    /**
     声波在常温常压的空气中传播的速度344米/秒，进行单位转化，34400厘米/秒
     继续做单位转化，为0.0344厘米/微秒，反过来，即29.069微秒/厘米，即声波传播一厘米需要29.069微秒
     而检测距离，是先发送声波，后接收，声波走的是二倍的距离，
     也就是说，超声波测距检测出1厘米，需要58.13微秒，即58.13
     pulseIn的时间是微秒，所以换算成cm，需要除以58
  */
     val=digitalRead(buttonpin);
     if(val){
      buttonstate="LightImprove ";
     }
     else{
      buttonstate="LightLowest ";
     }
     Serial.println(val);
     Serial.print(dist);    //在串口打印输出dist变量
     Serial.println("cm");    //在串口打印输出cm单位
}
void updateSensorData() {
  if (wifi.createTCP(HOST_NAME, HOST_PORT)) { //建立TCP连接，如果失败，不能发送该数据
    Serial.print("create tcp ok\r\n");

jsonToSend="{\"Lightness\":";
    dtostrf(sensor_lux,1,2,buf);
    jsonToSend+="\""+String(buf)+"\"";
    jsonToSend+=",\"Distance\":";
    dtostrf(dist,1,2,buf);
    jsonToSend+="\""+String(buf)+"\"";
    jsonToSend+=",\"State\":";
    jsonToSend+="\""+buttonstate+"\"";
    jsonToSend+="}";


    postString="POST /devices/";
    postString+=DEVICEID;
    postString+="/datapoints?type=3 HTTP/1.1";
    postString+="\r\n";
    postString+="api-key:";
    postString+=apiKey;
    postString+="\r\n";
    postString+="Host:api.heclouds.com\r\n";
    postString+="Connection:close\r\n";
    postString+="Content-Length:";
    postString+=jsonToSend.length();
    postString+="\r\n";
    postString+="\r\n";
    postString+=jsonToSend;
    postString+="\r\n";
    postString+="\r\n";
    postString+="\r\n";

  const char *postArray = postString.c_str();                 //将str转化为char数组
  Serial.println(postArray);
  wifi.send((const uint8_t*)postArray, strlen(postArray));    //send发送命令，参数必须是这两种格式，尤其是(const uint8_t*)
  Serial.println("send success");   
     if (wifi.releaseTCP()) {                                 //释放TCP连接
        Serial.print("release tcp ok\r\n");
        } 
     else {
        Serial.print("release tcp err\r\n");
        }
      postArray = NULL;                                       //清空数组，等待下次传输数据
  
  } else {
    Serial.print("create tcp err\r\n");
  }
}
