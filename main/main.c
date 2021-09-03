#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_err.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_freertos_hooks.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "E:/ESP32/esp-idf-v4.2/components/wpa_supplicant/include/esp_supplicant/esp_wpa2.h"
#include "esp_wifi.h"
#include "E:/ESP32/esp-idf-v4.2/components/nvs_flash/include/nvs_flash.h"
#include "esp_netif.h"
#include "esp_smartconfig.h"
#include "esp_sntp.h"
#include "E:/ESP32/esp-idf-v4.2/examples/common_components/protocol_examples_common/include/protocol_examples_common.h"
#include "E:/ESP32/esp-idf-v4.2/examples/common_components/protocol_examples_common/include/addr_from_stdin.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
// Littlevgl 头文件
#include "lvgl/lvgl.h"	  // LVGL头文件
#include "lvgl_helpers.h" // 助手 硬件驱动相关
#include "lv_examples/src/lv_demo_widgets/lv_demo_widgets.h"

/* 宏定义WiFi更新标识码、WiFi名称和密码 */
#define MY_WIFI_UPDATE  520               /* 对数值进行修改表示更新NVS的WiFi名称和密码*/
#define MY_WIFI_SSID    "WiFi"
#define MY_WIFI_PASSWD  "passwd"

#define TAG "LittlevGL Demo"
#define LV_TICK_PERIOD_MS 10

/* 宏定义WiFi连接事件标志位、连接失败标志位及智能配网标志位 */
#define  CONNECTED_BIT        BIT0   //WiFi连接事件标志位
#define  ESPTOUCH_DONE_BIT    BIT1   //智能配网标志位
#define  WIFI_FAIL_BIT        BIT2   //WIFI连接错误标志位

/* 定义一个WiFi连接事件标志组句柄 */
static EventGroupHandle_t s_wifi_event_group;

//http组包宏，获取日历信息及城市拼音的http接口参数
#define WEB_SERVER "i.tianqi.com"
#define WEB_URL "http://i.tianqi.com/index.php?c=code&a=getcode&id=25&py="   //日历网页，自动获取IP物理位置，获取阴历信息

//http组包宏，获取城市代码的http接口参数
#define WEB_SERVER1 "toy1.weather.com.cn"
//http组包宏，获取天气的http接口参数
#define WEB_SERVER2 "www.weather.com.cn"


//http请求包  GET
static const char *REQUEST = "GET " WEB_URL " HTTP/1.1\n"
    "Host: "WEB_SERVER"\n"
    "User-Agent: Mozilla/5.0 (iPhone; CPU iPhone OS 11_0 like Mac OS X) AppleWebKit/604.1.38 (KHTML, like Gecko) Version/11.0 Mobile/15A372 Safari/604.1\n"
    "\n";

char *T_cspy = NULL; // 缓存数组指针--保存城市拼音
char cspy2[200] ="GET http://toy1.weather.com.cn/search?cityname=";
char *REQUEST1 = " HTTP/1.1\r\n"
    "Host: "WEB_SERVER1"\r\n"
    "User-Agent: Mozilla/5.0 (iPhone; CPU iPhone OS 11_0 like Mac OS X) AppleWebKit/604.1.38 (KHTML, like Gecko) Version/11.0 Mobile/15A372 Safari/604.1\r\n"
    "\r\n";
char *T_cspy2 =&cspy2[0];

char *T_csdm = NULL; // 缓存数组指针--保存城市代码
char p[200] ="GET http://www.weather.com.cn/weather/";
char *REQUEST2 = ".shtml HTTP/1.1\r\n"
	"Host: "WEB_SERVER2"\r\n"
	"User-Agent: Mozilla/5.0 (iPhone; CPU iPhone OS 11_0 like Mac OS X) AppleWebKit/604.1.38 (KHTML, like Gecko) Version/11.0 Mobile/15A372 Safari/604.1\r\n"
	"\r\n";
char *pp =&p[0];

struct 
{
   char  *rizi;      //日子汉字
   char  *xwtianqi;  //下午天气图标
   char  *tianqi;    //今日天气汉字
   char  *zgwendu;   //最高温度
   char  *zdwendu;   //最低温度
   char  *fengx;     //风向
   char  *fengji;    //风级
   char  biaozhiwei; //标志位
}weather1,weather2,weather3;

//图片申明
LV_IMG_DECLARE(ewm);     //配网二维码图片
LV_IMG_DECLARE(dl_0);    //哆啦1
LV_IMG_DECLARE(dl_1);    //哆啦2
LV_IMG_DECLARE(dl_2);    //哆啦3
LV_IMG_DECLARE(dl_3);    //哆啦4
LV_IMG_DECLARE(windows_kj0);    //Windows背景图
LV_IMG_DECLARE(windows_gd0);    //Windows滚动图1
LV_IMG_DECLARE(windows_gd1);    //Windows滚动图2
LV_IMG_DECLARE(windows_gd2);    //Windows滚动图3
LV_IMG_DECLARE(windows_gd3);    //Windows滚动图4
LV_IMG_DECLARE(windows_gd4);    //Windows滚动图5
LV_IMG_DECLARE(windows_gd5);    //Windows滚动图6
LV_IMG_DECLARE(windows_gd6);    //Windows滚动图7
LV_IMG_DECLARE(windows_gd7);    //Windows滚动图8
LV_IMG_DECLARE(windows_gd8);    //Windows滚动图9
LV_IMG_DECLARE(windows_gd9);    //Windows滚动图10
LV_IMG_DECLARE(windows_gd10);   //Windows滚动图11
LV_IMG_DECLARE(windows_gd11);   //Windows滚动图12
LV_IMG_DECLARE(bpbj0);   //表盘背景1
LV_IMG_DECLARE(bpbj1);   //表盘背景2
LV_IMG_DECLARE(bpbj2);   //表盘背景3
LV_IMG_DECLARE(bpbj3);   //表盘背景4
LV_IMG_DECLARE(bpbj4);   //表盘背景5
LV_IMG_DECLARE(bpbj5);   //表盘背景6
LV_IMG_DECLARE(bpbj6);   //表盘背景7
LV_IMG_DECLARE(bpbj7);   //表盘背景8
LV_IMG_DECLARE(bpbj8);   //表盘背景9
LV_IMG_DECLARE(bpbj9);   //表盘背景10
LV_IMG_DECLARE(tq_0);    //天气图标0 - 60*60
LV_IMG_DECLARE(tq_1);    //天气图标1
LV_IMG_DECLARE(tq_2);    //天气图标2
LV_IMG_DECLARE(tq_3);    //天气图标3
LV_IMG_DECLARE(tq_4);    //天气图标4
LV_IMG_DECLARE(tq_5);    //天气图标5
LV_IMG_DECLARE(tq_6);    //天气图标6
LV_IMG_DECLARE(tq_7);    //天气图标7
LV_IMG_DECLARE(tq_8);    //天气图标8
LV_IMG_DECLARE(tq_9);    //天气图标9
LV_IMG_DECLARE(tq_10);   //天气图标10
LV_IMG_DECLARE(tq_11);   //天气图标11
LV_IMG_DECLARE(tq_12);   //天气图标12
LV_IMG_DECLARE(tq_13);   //天气图标13
LV_IMG_DECLARE(tq_14);   //天气图标14
LV_IMG_DECLARE(tq_15);   //天气图标15
LV_IMG_DECLARE(tq_16);   //天气图标16
LV_IMG_DECLARE(tq_17);   //天气图标17
LV_IMG_DECLARE(tq_18);   //天气图标18
LV_IMG_DECLARE(tq_19);   //天气图标19
LV_IMG_DECLARE(tq_20);   //天气图标20
LV_IMG_DECLARE(tq_21);   //天气图标21
LV_IMG_DECLARE(tq_22);   //天气图标22
LV_IMG_DECLARE(tq_23);   //天气图标23
LV_IMG_DECLARE(tq_24);   //天气图标24
LV_IMG_DECLARE(tq_29);   //天气图标25
LV_IMG_DECLARE(tq_30);   //天气图标26
LV_IMG_DECLARE(tq_31);   //天气图标27
LV_IMG_DECLARE(tq_32);   //天气图标28
LV_IMG_DECLARE(tq_49);   //天气图标29
LV_IMG_DECLARE(tq_53);   //天气图标30
LV_IMG_DECLARE(tq_54);   //天气图标31
LV_IMG_DECLARE(tq_55);   //天气图标25
LV_IMG_DECLARE(tq_56);   //天气图标26
LV_IMG_DECLARE(tq_57);   //天气图标27
LV_IMG_DECLARE(tq_58);   //天气图标28
LV_IMG_DECLARE(tq_99);   //天气图标29
LV_IMG_DECLARE(tq_301);   //天气图标30
LV_IMG_DECLARE(tq_302);   //天气图标31
LV_IMG_DECLARE(wztb_32); //位置图标-32

//汉字声明
LV_IMG_DECLARE(myfont_cshz_18); //经典黑体简 - 18 -  显示城市名+农历信息+数字
LV_IMG_DECLARE(myfont_cshz_24); //经典黑体简 - 24 -  显示城市名+农历信息+数字
LV_IMG_DECLARE(myfont_3500hz_18); //经典黑体简 - 18 -  显示城市名+农历信息+数字

//img 图像控件声明，声明在外部回调中需要调用
lv_obj_t *img1 = NULL;  //开机windows图片对象
lv_obj_t *img2 = NULL;  //开机滚动图片对象
lv_obj_t *img3 = NULL;  //二维码图片对象
lv_obj_t *img4 = NULL;  //哆啦图片对象

//标签控件声明，声明在外部回调中需要调用
lv_obj_t *label2 = NULL;  //标签对象--显示农历等信息
lv_obj_t *label3 = NULL;  //标签对象--显示时间文本
lv_obj_t *label4 = NULL;  //标签对象--显示位置文本
lv_obj_t *label5 = NULL;  //标签对象--显示今天日子文本
lv_obj_t *label5_1 = NULL;  //标签对象--显示今天天气文本
lv_obj_t *label5_2 = NULL;  //标签对象--显示今天最低气温文本
lv_obj_t *label5_3 = NULL;  //标签对象--显示今天风向文本
lv_obj_t *label5_4 = NULL;  //标签对象--显示今天风力文本
lv_obj_t *label5_5 = NULL;  //标签对象--显示今天最高温度文本
lv_obj_t *label6 = NULL;  //标签对象--显示明天日子文本
lv_obj_t *label6_1 = NULL;  //标签对象--显示明天天气文本
lv_obj_t *label6_2 = NULL;  //标签对象--显示明天最低气温文本
lv_obj_t *label6_3 = NULL;  //标签对象--显示明天风向文本
lv_obj_t *label6_4 = NULL;  //标签对象--显示明天风力文本
lv_obj_t *label6_5 = NULL;  //标签对象--显示明天最高温度文本
lv_obj_t *label7 = NULL;  //标签对象--显示后天日子文本
lv_obj_t *label7_1 = NULL;  //标签对象--显示后天天气文本
lv_obj_t *label7_2 = NULL;  //标签对象--显示后天最低气温文本
lv_obj_t *label7_3 = NULL;  //标签对象--显示后天风向文本
lv_obj_t *label7_4 = NULL;  //标签对象--显示后天风力文本
lv_obj_t *label7_5 = NULL;  //标签对象--显示后天最高温度文本
//LVGL任务
lv_task_t *task1 = NULL;

//创建滚轮对象-显示农历等信息
lv_obj_t *roller1 = NULL;

//创建页签对象-显示天气信息
lv_obj_t *tabview = NULL;
//创建页签对象-显示背景图片
lv_obj_t *bjtuyq = NULL;

//创建一个信号量来处理对lvgl的并发调用
//如果您希望从其他线程/任务调用*任何*lvgl函数
//你应该锁定同一个信号灯！
SemaphoreHandle_t xGuiSemaphore; // 创建一个GUI信号量

static void lv_tick_task(void *arg); // LVGL 时钟任务
static void guiTask(void *pvParameter);	 // GUI任务
static void Bp_Task(void *pvParameter);	 //表盘任务
static void SmartConfig_Task(void *parm);  // SmartConfig任务
void img_test(void);

static void http_get_task(void *parm);  //网络天气任务
static void http_get_task1(void *parm);  //网络天气任务
static void http_get_task2(void *parm);  //天气网任务
static void Sntp_Task(void *parm); //Sntp任务
void initialize_sntp(void);
void getNowTime(void);

// LVGL 时钟任务
static void lv_tick_task(void *arg)
{
	(void)arg;
	lv_tick_inc(LV_TICK_PERIOD_MS);
}

//分割函数
//str，要分割的字符串
//s，要分割的符号
//e，要分割的第几个符号
char *splitx(char *str,char *s,char *e){
    char *t1=NULL;
    char *t2=NULL;
    char *t3=NULL;
    uint8_t lensx=0;
    //在字符串str中查找第一次出现字符串s的位置，不包含终止符“\0”
    t1=strstr(str,s);
    if(t1==NULL){
        ESP_LOGI("FUNC_splitx","t1 valave is NULL");
        return ESP_OK;
    }
    //在字符串t1中查找第一次出现字符串e的位置，不包含终止符“\0”
    t2=strstr(t1,e);
    if(t2==NULL){
        ESP_LOGI("FUNC_splitx","t2 valave is NULL");
        return ESP_OK;
    }
    //printf("t1=%d\n t2=%d\n s=%d\n",strlen(t1),strlen(t2),strlen(s));
    lensx=strlen(t1)-strlen(t2)-strlen(s);//strlen返回计数器值（不包含“\0”）
    //printf("lensx=%d\n",lensx);
    char t='\0';
    t3=(char *)malloc(sizeof(char)*lensx+1);//malloc动态内存分配
    memset(t3,t,sizeof(char)*lensx+1); //为新申请的内存做初始化
    //把t1所指向的字符串中以t1地址开始的前lensx个字节复制到t3所指的数组中，并返回被复制后的t3
    strncpy(t3,t1+sizeof(char)*strlen(s),lensx);//strncpy将指定长度的字符串复制到字符数组中
    return t3;
 }

//初始化显示以及开机界面
//任务函数
static void guiTask(void *pvParameter)
{
	(void)pvParameter;
	xGuiSemaphore = xSemaphoreCreateMutex(); // 创建GUI信号量
	lv_init();								 // 初始化LittlevGL
	lvgl_driver_init();						 // 初始化液晶SPI驱动 触摸芯片SPI/IIC驱动

	// 初始化缓存
	static lv_color_t buf1[DISP_BUF_SIZE];
	static lv_color_t buf2[DISP_BUF_SIZE];
	static lv_disp_buf_t disp_buf;
	uint32_t size_in_px = DISP_BUF_SIZE;
	lv_disp_buf_init(&disp_buf, buf1, buf2, size_in_px);

	// 添加并注册触摸驱动
	lv_disp_drv_t disp_drv;
	lv_disp_drv_init(&disp_drv);
	disp_drv.flush_cb = disp_driver_flush;
	disp_drv.buffer = &disp_buf;
	lv_disp_drv_register(&disp_drv);

	// 定期处理GUI回调
	const esp_timer_create_args_t periodic_timer_args = {
		.callback = &lv_tick_task,
		.name = "periodic_gui"};
	esp_timer_handle_t periodic_timer;
	ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
	ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, LV_TICK_PERIOD_MS * 1000));

    //显示开机图片
    //创建LVGL图片刷新任务
	img_test();

	while (1)
	{
		vTaskDelay(1);
		// 尝试锁定信号量，如果成功，调用处理LVGL任务
		if (xSemaphoreTake(xGuiSemaphore, (TickType_t)10) == pdTRUE)
		{
			lv_task_handler();			   // 处理LVGL任务
			xSemaphoreGive(xGuiSemaphore); // 释放信号量
		}
	}
	vTaskDelete(NULL); // 删除任务  防止出现问题，否则不会执行到这里
}


static void wifi_event_handler(void* arg, esp_event_base_t event_base,int32_t event_id, void* event_data)
{
	static int retry_num = 0;           /* 记录wifi重连次数 */
	// STA开始工作
	if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
		esp_wifi_connect();   //WIFI连接
		xTaskCreate(SmartConfig_Task, "SmartConfig_Task", 1024*4, NULL, 4, NULL);//创建动态任务SmartConfig_Task
	}
	// STA 断开事件
	else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
		esp_wifi_connect();   //WIFI连接
		retry_num++;
		ESP_LOGI(TAG,"retry to connect to the AP %d times. \n",retry_num);
		/* WiFi重连次数大于6 */
		if (retry_num > 6){xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);}//WIFI连接失败置1
		xEventGroupClearBits(s_wifi_event_group, CONNECTED_BIT);  //将指定的事件位清零
	} 
	// STA 获取到IP
	else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
		xEventGroupSetBits(s_wifi_event_group, CONNECTED_BIT);  //事件组设置   正在测试的事件组，要等待的事件组中的位
		xEventGroupClearBits(s_wifi_event_group, WIFI_FAIL_BIT);  //连接成功，将连接失败标志位清0
	} 
	// SmartConfig 扫描完成  貌似是做好了扫描准备工作
	else if (event_base == SC_EVENT && event_id == SC_EVENT_SCAN_DONE) {
		ESP_LOGI(TAG, "Scan done");  //串口打印--我准备好了随时接收空中局域网消息
	}
	// SmartConfig 找到信道
	else if (event_base == SC_EVENT && event_id == SC_EVENT_FOUND_CHANNEL) {
		ESP_LOGI(TAG, "Found channel");   //找到空中报文  可以获取到WIFI密码及名称  
	}
	// SmartConfig 获取到WIFI名和密码
	else if (event_base == SC_EVENT && event_id == SC_EVENT_GOT_SSID_PSWD) {
		ESP_LOGI(TAG, "Got SSID and password");
		smartconfig_event_got_ssid_pswd_t *evt = (smartconfig_event_got_ssid_pswd_t *)event_data;//赋值账号和密码
		wifi_config_t wifi_config;  //WIFI配置
		char ssid[33] = { 0 };  //创建数组
		char password[65] = { 0 };
		bzero(&wifi_config, sizeof(wifi_config_t));  //置字节字符串s的前n个字节为零
		memcpy(wifi_config.sta.ssid, evt->ssid, sizeof(wifi_config.sta.ssid));//从源source中拷贝n个字节到目标destin中
		memcpy(wifi_config.sta.password, evt->password, sizeof(wifi_config.sta.password));//拷贝密码
		wifi_config.sta.bssid_set = evt->bssid_set;
		if (wifi_config.sta.bssid_set == true) {
			memcpy(wifi_config.sta.bssid, evt->bssid, sizeof(wifi_config.sta.bssid));
		}

		memcpy(ssid, evt->ssid, sizeof(evt->ssid));   //拷贝账号
		memcpy(password, evt->password, sizeof(evt->password));  //拷贝密码
		ESP_LOGI(TAG, "SSID:%s", ssid);   //打印WIFI名称
		ESP_LOGI(TAG, "PASSWORD:%s", password);  //打印WIFI密码

		/* 将得到的WiFi名称和密码存入NVS*/
		nvs_handle wificfg_nvs_handler;
		ESP_ERROR_CHECK( nvs_open("WiFi_cfg", NVS_READWRITE, &wificfg_nvs_handler) );
		ESP_ERROR_CHECK( nvs_set_str(wificfg_nvs_handler,"wifi_ssid",ssid) );
		ESP_ERROR_CHECK( nvs_set_str(wificfg_nvs_handler,"wifi_passwd",password) );
		ESP_ERROR_CHECK( nvs_commit(wificfg_nvs_handler) ); /* 提交 */
		nvs_close(wificfg_nvs_handler);                     /* 关闭 */ 
		ESP_LOGI(TAG,"smartconfig save wifi_cfg to NVS .\n"); //保存到NVS

		// 断开默认的
		ESP_ERROR_CHECK( esp_wifi_disconnect() );
		// 设置获取的ap和密码到寄存器
		ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
		// 连接获取的ssid和密码
		ESP_ERROR_CHECK( esp_wifi_connect() );
	}
	// SmartConfig 发送应答完成
	else if (event_base == SC_EVENT && event_id == SC_EVENT_SEND_ACK_DONE) {
		xEventGroupSetBits(s_wifi_event_group, ESPTOUCH_DONE_BIT);
	}	
}

//LVGL任务二  
//刷新表盘哆啦图片
void task2_cb(void)
{
    static uint8_t i=0;
	if(i==0){i=1;lv_img_set_src(img4, &dl_0);}
	else if(i==1){i=2;lv_img_set_src(img4, &dl_1);}
	else if(i==2){i=3;lv_img_set_src(img4, &dl_2);}
	else if(i==3){i=0;lv_img_set_src(img4, &dl_3);}
}

//LVGL任务三 
//刷新页签滚动
void task3_cb(void)
{
   static uint8_t page_id = 0,page_id1 = 0;
   page_id++;

   if(page_id==4)
   {
	  page_id = 0;
      page_id1++;
   }
    if(page_id1==4)
   {
	  page_id1 = 0;
   }
   lv_tabview_set_tab_act(tabview,page_id,LV_ANIM_ON);//带有切换动画效果
   lv_tabview_set_tab_act(bjtuyq,page_id1,LV_ANIM_ON);//带有切换动画效果
}

//表盘显示任务
static void Bp_Task(void *pvParameter)
{
	int i=0;
    //tabview 分页标签控件
	bjtuyq = lv_tabview_create(lv_scr_act(), NULL);// 在主屏幕上创建一个 tabview
	lv_obj_set_size(bjtuyq,240,240);  //设置页签的大小
	lv_obj_align(bjtuyq,NULL,LV_ALIGN_IN_TOP_LEFT,0,0);//位置
	lv_tabview_set_btns_pos(bjtuyq,LV_TABVIEW_TAB_POS_NONE);//设置页面选择按钮栏为 - 无
	lv_obj_t *bj1 = lv_tabview_add_tab(bjtuyq, "b1");	// 在tabview上添加6个标签页
	lv_obj_t *bj2 = lv_tabview_add_tab(bjtuyq, "b2");
	lv_obj_t *bj3 = lv_tabview_add_tab(bjtuyq, "b3");
	lv_obj_t *bj4 = lv_tabview_add_tab(bjtuyq, "b4");

    //显示表盘背景
	lv_obj_t *img5 = lv_img_create(bj1, NULL);
	lv_img_set_src(img5, &bpbj2);
	lv_obj_align(img5, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
	lv_page_set_scrollbar_mode(bj1, LV_SCRLBAR_MODE_OFF);  //从不显示页面滚动条 - 整个页签

    //显示表盘背景
	lv_obj_t *img6 = lv_img_create(bj2, NULL);
	lv_img_set_src(img6, &bpbj4);
	lv_obj_align(img6, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
	lv_page_set_scrollbar_mode(bj2, LV_SCRLBAR_MODE_OFF);  //从不显示页面滚动条 - 整个页签
	//显示表盘背景
	lv_obj_t *img7 = lv_img_create(bj3, NULL);
	lv_img_set_src(img7, &bpbj5);
	lv_obj_align(img7, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
	lv_page_set_scrollbar_mode(bj3, LV_SCRLBAR_MODE_OFF);  //从不显示页面滚动条 - 整个页签
	//显示表盘背景
	lv_obj_t *img8 = lv_img_create(bj4, NULL);
	lv_img_set_src(img8, &bpbj3);
	lv_obj_align(img8, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
	lv_page_set_scrollbar_mode(bj4, LV_SCRLBAR_MODE_OFF);  //从不显示页面滚动条 - 整个页签

    //文本-电量换颜色
	// static lv_style_t text_style1;	// 创建一个风格
	// lv_style_init(&text_style1);	// 初始化风格
	// lv_style_set_text_font(&text_style1, LV_STATE_DEFAULT, &lv_font_montserrat_24); //样式字体大小
    // lv_style_set_text_color(&text_style1, LV_STATE_DEFAULT, LV_COLOR_WHITE); // 设置字体颜色

	// lv_obj_t *label1 = lv_label_create(lv_scr_act(), NULL); //创建标签1
	// lv_label_set_text(label1, LV_SYMBOL_BATTERY_3);		//文本显示-电量
	// lv_obj_align(label1, NULL, LV_ALIGN_IN_TOP_LEFT, 200, 5); //显示位置
    // lv_obj_add_style(label1,LV_LABEL_PART_MAIN, &text_style1);  //使用样式

    //显示哆啦A梦图片
	img4 = lv_img_create(lv_scr_act(), NULL);				 //创建图片对象4
	lv_img_set_src(img4, &dl_0);							 //显示A梦
	lv_obj_align(img4, NULL, LV_ALIGN_IN_TOP_LEFT, 145, 39); //A梦显示位置

    //创建LVGL任务-哆啦A梦动图
	lv_task_t *task2 = lv_task_create((lv_task_cb_t)task2_cb, 200, LV_TASK_PRIO_HIGH, NULL); //创建LVGL图片刷新任务

    //显示位置图标
	lv_obj_t *wztb = lv_img_create(lv_scr_act(), NULL);
	lv_img_set_src(wztb, &wztb_32);
	lv_obj_align(wztb, NULL, LV_ALIGN_IN_TOP_LEFT, 5, 102);  //位置图标显示位置

    //创建背景样式
    static lv_style_t bg_style;//定义一个样式
    lv_style_init(&bg_style);	// 初始化风格
    lv_style_set_text_font(&bg_style, LV_STATE_DEFAULT, &myfont_cshz_24); //城市24号字体
	lv_style_set_text_color(&bg_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);//文本颜色

    //创建背景样式
    static lv_style_t bghz_style;//定义一个样式
    lv_style_init(&bghz_style);	// 初始化风格
    lv_style_set_text_font(&bghz_style, LV_STATE_DEFAULT, &myfont_3500hz_18); //城市24号字体
	lv_style_set_text_color(&bghz_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);//文本颜色

    //Label2 长文本动态滚动显示
	label2 = lv_label_create(lv_scr_act(), NULL);	// 在主屏幕创建一个标签
	lv_label_set_long_mode(label2, LV_LABEL_LONG_SROLL_CIRC);	// 标签长内容框，保持控件宽度，内容过长循环滚动
	lv_obj_set_width(label2, 100);		// 设置标签宽度
	lv_obj_set_height(label2, 24);  //设置标签高度
	lv_label_set_text(label2, NULL);// 设置显示文本
	lv_obj_align(label2, NULL, LV_ALIGN_IN_TOP_LEFT, 10, 40);			// 对齐到中心偏下
    lv_obj_add_style(label2,LV_LABEL_PART_MAIN, &bg_style);  //使用样式

    //创建页签背景样式
    static lv_style_t yq_style;//定义一个样式
    lv_style_init(&yq_style);	// 初始化风格
	lv_style_set_text_font(&yq_style, LV_STATE_DEFAULT, &myfont_cshz_18); //城市18号字体
    lv_style_set_bg_opa(&yq_style, LV_STATE_DEFAULT, LV_OPA_30);  //设置透明度
    lv_style_set_bg_color(&yq_style, LV_STATE_DEFAULT, LV_COLOR_GRAY);  //背景
    lv_style_set_text_color(&yq_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);//文本颜色

    //tabview 分页标签控件
	tabview = lv_tabview_create(lv_scr_act(), NULL);// 在主屏幕上创建一个 tabview
	lv_obj_set_size(tabview,240,100);  //设置页签的大小
	lv_obj_align(tabview,NULL,LV_ALIGN_IN_TOP_LEFT,0,139);//位置
	lv_tabview_set_btns_pos(tabview,LV_TABVIEW_TAB_POS_NONE);//设置页面选择按钮栏为 - 无
	lv_obj_t *tab1 = lv_tabview_add_tab(tabview, "Tab 1");	// 在tabview上添加6个标签页
	lv_obj_t *tab2 = lv_tabview_add_tab(tabview, "Tab 2");
	lv_obj_t *tab3 = lv_tabview_add_tab(tabview, "Tab 3");
	lv_obj_t *tab4 = lv_tabview_add_tab(tabview, "Tab 4");
    lv_obj_add_style(tabview,LV_LABEL_PART_MAIN, &yq_style);  //使用样式

    //显示天气图标
	lv_obj_t *TQ_TB1 = lv_img_create(tab1, NULL);
	lv_img_set_src(TQ_TB1, NULL);  //显示天气图标
	lv_obj_align(TQ_TB1, NULL, LV_ALIGN_IN_TOP_LEFT, 5, 18);  //天气图标左中显示
    lv_page_set_scrollbar_mode(tab1, LV_SCRLBAR_MODE_OFF);  //从不显示页面滚动条 - 整个页签

	lv_obj_t *TQ_TB2 = lv_img_create(tab2, NULL);
	lv_img_set_src(TQ_TB2, NULL);  //显示天气图标
	lv_obj_align(TQ_TB2, NULL, LV_ALIGN_IN_TOP_LEFT, 5, 18);  //天气图标左中显示
    lv_page_set_scrollbar_mode(tab2, LV_SCRLBAR_MODE_OFF);  //从不显示页面滚动条 - 整个页签

	lv_obj_t *TQ_TB3 = lv_img_create(tab3, NULL);
	lv_img_set_src(TQ_TB3, NULL);  //显示天气图标
	lv_obj_align(TQ_TB3, NULL, LV_ALIGN_IN_TOP_LEFT, 5, 18);  //天气图标左中显示
	lv_page_set_scrollbar_mode(tab3, LV_SCRLBAR_MODE_OFF);  //从不显示页面滚动条 - 整个页签

	lv_obj_t * label = lv_label_create(tab4, NULL);			// 在第二个标签页上添加一个标签控件
	lv_label_set_text(label, "征服自己,就能征服一切!!!");		// 设置标签内容
	lv_obj_align(label, NULL, LV_ALIGN_IN_TOP_LEFT,20,35); //显示位置 对齐方式--跟随位置图片
	lv_obj_add_style(label,LV_LABEL_PART_MAIN, &bghz_style);  //使用样式
    lv_page_set_scrollbar_mode(tab4, LV_SCRLBAR_MODE_OFF);  //从不显示页面滚动条 - 整个页签

    //创建LVGL任务-页签动图
	lv_task_t *task3 = lv_task_create((lv_task_cb_t)task3_cb, 3000, LV_TASK_PRIO_MID, NULL); //创建LVGL图片刷新任务

    //Label4 位置文本显示
	label4 = lv_label_create(lv_scr_act(), NULL);   //创建时间文本标签
	lv_label_set_text(label4, NULL);	//文本显示--位置信息
	lv_obj_align(label4, wztb, LV_ALIGN_IN_TOP_LEFT,32,7); //显示位置 对齐方式--跟随位置图片
	lv_obj_add_style(label4,LV_LABEL_PART_MAIN, &yq_style);  //使用样式

    //Label5 今天日子文本显示
	label5 = lv_label_create(tab1, NULL);   //创建时间文本标签
	lv_label_set_text(label5, NULL);	//文本显示--位置信息
	lv_obj_align(label5, NULL, LV_ALIGN_IN_TOP_LEFT,90,8); //显示位置 对齐方式--
	lv_obj_add_style(label5,LV_LABEL_PART_MAIN, &bghz_style);  //使用样式

	//Label5_1 今天天气文本显示
	label5_1 = lv_label_create(tab1, NULL);   //创建时间文本标签
	lv_label_set_text(label5_1, NULL);	//文本显示--位置信息
	lv_obj_align(label5_1, NULL, LV_ALIGN_IN_TOP_LEFT,90,31); //显示位置 对齐方式--
	lv_obj_add_style(label5_1,LV_LABEL_PART_MAIN, &bghz_style);  //使用样式

	//Label5_2 今天最低气温文本显示
	label5_2 = lv_label_create(tab1, NULL);   //创建时间文本标签
	lv_label_set_text(label5_2, NULL);	//文本显示--位置信息
	lv_obj_align(label5_2, NULL, LV_ALIGN_IN_TOP_LEFT,144,54); //显示位置 对齐方式--
	lv_obj_add_style(label5_2,LV_LABEL_PART_MAIN, &bghz_style);  //使用样式

	//Label5_3 今天风向文本显示
	label5_3 = lv_label_create(tab1, NULL);   //创建时间文本标签
	lv_label_set_text(label5_3, NULL);	//文本显示--位置信息
	lv_obj_align(label5_3, NULL, LV_ALIGN_IN_TOP_LEFT,182,54); //显示位置 对齐方式--跟随天气图片
	lv_obj_add_style(label5_3,LV_LABEL_PART_MAIN, &bghz_style);  //使用样式

	//Label5_4 今天风力文本显示
	label5_4 = lv_label_create(tab1, NULL);   //创建时间文本标签
	lv_label_set_text(label5_4, NULL);	//文本显示--位置信息
	lv_obj_align(label5_4, NULL, LV_ALIGN_IN_TOP_LEFT,90,77); //显示位置 对齐方式--跟随天气图片
	lv_obj_add_style(label5_4,LV_LABEL_PART_MAIN, &bghz_style);  //使用样式

	//Label5_5 今天最高温度显示
	label5_5 = lv_label_create(tab1, NULL);   //创建时间文本标签
	lv_label_set_text(label5_5, NULL);	//文本显示--位置信息
	lv_obj_align(label5_5, NULL, LV_ALIGN_IN_TOP_LEFT,90,54); //显示位置 对齐方式--跟随天气图片
	lv_obj_add_style(label5_5,LV_LABEL_PART_MAIN, &bghz_style);  //使用样式

	//Label5_5 今天最高温度显示
	lv_obj_t *label5_6 = lv_label_create(tab1, NULL);   //创建时间文本标签
	lv_label_set_text(label5_6, "/");	//文本显示--位置信息
	lv_obj_align(label5_6, NULL, LV_ALIGN_IN_TOP_LEFT,126,54); //显示位置 对齐方式--跟随天气图片
	lv_obj_add_style(label5_6,LV_LABEL_PART_MAIN, &bghz_style);  //使用样式
/*****************************************************/
    //Label6 明天日子文本显示
	label6 = lv_label_create(tab2, NULL);   //创建时间文本标签
	lv_label_set_text(label6, NULL);	//文本显示--位置信息
	lv_obj_align(label6, NULL, LV_ALIGN_IN_TOP_LEFT,90,8); //显示位置 对齐方式--跟随天气图片
	lv_obj_add_style(label6,LV_LABEL_PART_MAIN, &bghz_style);  //使用样式

	//Label6_1 明天天气文本显示
	label6_1 = lv_label_create(tab2, NULL);   //创建时间文本标签
	lv_label_set_text(label6_1, NULL);	//文本显示--位置信息
	lv_obj_align(label6_1, NULL, LV_ALIGN_IN_TOP_LEFT,90,31); //显示位置 对齐方式--
	lv_obj_add_style(label6_1,LV_LABEL_PART_MAIN, &bghz_style);  //使用样式

	//Label5_2 明天最低气温文本显示
	label6_2 = lv_label_create(tab2, NULL);   //创建时间文本标签
	lv_label_set_text(label6_2, NULL);	//文本显示--位置信息
	lv_obj_align(label6_2, NULL, LV_ALIGN_IN_TOP_LEFT,144,54); //显示位置 对齐方式--
	lv_obj_add_style(label6_2,LV_LABEL_PART_MAIN, &bghz_style);  //使用样式

	//Label5_3 明天风向文本显示
	label6_3 = lv_label_create(tab2, NULL);   //创建时间文本标签
	lv_label_set_text(label6_3, NULL);	//文本显示--位置信息
	lv_obj_align(label6_3, NULL, LV_ALIGN_IN_TOP_LEFT,182,54); //显示位置 对齐方式--跟随天气图片
	lv_obj_add_style(label6_3,LV_LABEL_PART_MAIN, &bghz_style);  //使用样式

	//Label6_4 明天风力文本显示
	label6_4 = lv_label_create(tab2, NULL);   //创建时间文本标签
	lv_label_set_text(label6_4, NULL);	//文本显示--位置信息
	lv_obj_align(label6_4, NULL, LV_ALIGN_IN_TOP_LEFT,90,77); //显示位置 对齐方式--跟随天气图片
	lv_obj_add_style(label6_4,LV_LABEL_PART_MAIN, &bghz_style);  //使用样式

	//Label5_5 明天最高温度显示
	label6_5 = lv_label_create(tab2, NULL);   //创建时间文本标签
	lv_label_set_text(label6_5, NULL);	//文本显示--位置信息
	lv_obj_align(label6_5, NULL, LV_ALIGN_IN_TOP_LEFT,90,54); //显示位置 对齐方式--跟随天气图片
	lv_obj_add_style(label6_5,LV_LABEL_PART_MAIN, &bghz_style);  //使用样式

	//Label5_5 明天最高温度显示
	lv_obj_t *label6_6 = lv_label_create(tab2, NULL);   //创建时间文本标签
	lv_label_set_text(label6_6, "/");	//文本显示--位置信息
	lv_obj_align(label6_6, NULL, LV_ALIGN_IN_TOP_LEFT,126,54); //显示位置 对齐方式--跟随天气图片
	lv_obj_add_style(label6_6,LV_LABEL_PART_MAIN, &bghz_style);  //使用样式
/*****************************************************/
    //Label7  后天日子文本显示
	label7 = lv_label_create(tab3, NULL);   //创建时间文本标签
	lv_label_set_text(label7, NULL);	//文本显示--位置信息
	lv_obj_align(label7, NULL, LV_ALIGN_IN_TOP_LEFT,90,8); //显示位置 对齐方式--跟随天气图片
	lv_obj_add_style(label7,LV_LABEL_PART_MAIN, &bghz_style);  //使用样式

	//Label7_1 后天天气文本显示
	label7_1 = lv_label_create(tab3, NULL);   //创建时间文本标签
	lv_label_set_text(label7_1, NULL);	//文本显示--位置信息
	lv_obj_align(label7_1, NULL, LV_ALIGN_IN_TOP_LEFT,90,31); //显示位置 对齐方式--
	lv_obj_add_style(label7_1,LV_LABEL_PART_MAIN, &bghz_style);  //使用样式

	//Label7_2 后天最低气温文本显示
	label7_2 = lv_label_create(tab3, NULL);   //创建时间文本标签
	lv_label_set_text(label7_2, NULL);	//文本显示--位置信息
	lv_obj_align(label7_2, NULL, LV_ALIGN_IN_TOP_LEFT,144,54); //显示位置 对齐方式--
	lv_obj_add_style(label7_2,LV_LABEL_PART_MAIN, &bghz_style);  //使用样式

	//Label7_3 后天风向文本显示
	label7_3 = lv_label_create(tab3, NULL);   //创建时间文本标签
	lv_label_set_text(label7_3, NULL);	//文本显示--位置信息
	lv_obj_align(label7_3, NULL, LV_ALIGN_IN_TOP_LEFT,182,54); //显示位置 对齐方式--跟随天气图片
	lv_obj_add_style(label7_3,LV_LABEL_PART_MAIN, &bghz_style);  //使用样式

	//Label7_4 后天风力文本显示
	label7_4 = lv_label_create(tab3, NULL);   //创建时间文本标签
	lv_label_set_text(label7_4, NULL);	//文本显示--位置信息
	lv_obj_align(label7_4, NULL, LV_ALIGN_IN_TOP_LEFT,90,77); //显示位置 对齐方式--跟随天气图片
	lv_obj_add_style(label7_4,LV_LABEL_PART_MAIN, &bghz_style);  //使用样式

	//Label7_5 后天最高温度显示
	label7_5 = lv_label_create(tab3, NULL);   //创建时间文本标签
	lv_label_set_text(label7_5, NULL);	//文本显示--位置信息
	lv_obj_align(label7_5, NULL, LV_ALIGN_IN_TOP_LEFT,90,54); //显示位置 对齐方式--跟随天气图片
	lv_obj_add_style(label7_5,LV_LABEL_PART_MAIN, &bghz_style);  //使用样式

	//Label7_6 后天温度分割显示
	lv_obj_t *label7_6 = lv_label_create(tab3, NULL);   //创建时间文本标签
	lv_label_set_text(label7_6, "/");	//文本显示--位置信息
	lv_obj_align(label7_6, NULL, LV_ALIGN_IN_TOP_LEFT,126,54); //显示位置 对齐方式--跟随天气图片
	lv_obj_add_style(label7_6,LV_LABEL_PART_MAIN, &bghz_style);  //使用样式

	while (1)
	{   
		if(weather1.biaozhiwei==1)
		{
			lv_label_set_text(label5,weather1.rizi);	    //文本显示--位置信息
            lv_label_set_text(label5_1, weather1.tianqi);	//文本显示--位置信息
			lv_label_set_text(label5_2, weather1.zdwendu);	//文本显示--位置信息
			lv_label_set_text(label5_3, weather1.fengx);	//文本显示--位置信息
			lv_label_set_text(label5_4, weather1.fengji);	//文本显示--位置信息
			lv_label_set_text(label5_5, weather1.zgwendu);	//文本显示--位置信息

			lv_label_set_text(label6,weather2.rizi);	    //文本显示--位置信息
            lv_label_set_text(label6_1, weather2.tianqi);	//文本显示--位置信息
			lv_label_set_text(label6_2, weather2.zdwendu);	//文本显示--位置信息
			lv_label_set_text(label6_3, weather2.fengx);	//文本显示--位置信息
			lv_label_set_text(label6_4, weather2.fengji);	//文本显示--位置信息
			lv_label_set_text(label6_5, weather2.zgwendu);	//文本显示--位置信息
			
			lv_label_set_text(label7,weather3.rizi);	    //文本显示--位置信息
            lv_label_set_text(label7_1, weather3.tianqi);	//文本显示--位置信息
			lv_label_set_text(label7_2, weather3.zdwendu);	//文本显示--位置信息
			lv_label_set_text(label7_3, weather3.fengx);	//文本显示--位置信息
			lv_label_set_text(label7_4, weather3.fengji);	//文本显示--位置信息
			lv_label_set_text(label7_5, weather3.zgwendu);	//文本显示--位置信息

			// lv_img_set_src(TQ_TB1, &tq_1);  //显示天气图标
			// lv_img_set_src(TQ_TB2, &tq_1);  //显示天气图标
			// lv_img_set_src(TQ_TB3, &tq_9);  //显示天气图标
            
			i=atoi(weather1.xwtianqi);
			switch(i)
			{
                case 0: lv_img_set_src(TQ_TB1, &tq_0); break; //显示天气图标
                case 1: lv_img_set_src(TQ_TB1, &tq_1); break; //显示天气图标
                case 2: lv_img_set_src(TQ_TB1, &tq_2); break; //显示天气图标
				case 3: lv_img_set_src(TQ_TB1, &tq_3); break; //显示天气图标
				case 4: lv_img_set_src(TQ_TB1, &tq_4); break; //显示天气图标
				case 5: lv_img_set_src(TQ_TB1, &tq_5); break; //显示天气图标
				case 6: lv_img_set_src(TQ_TB1, &tq_6); break; //显示天气图标
				case 7: lv_img_set_src(TQ_TB1, &tq_7); break; //显示天气图标
				case 8: lv_img_set_src(TQ_TB1, &tq_8); break; //显示天气图标
				case 9: lv_img_set_src(TQ_TB1, &tq_9); break; //显示天气图标
				case 10: lv_img_set_src(TQ_TB1, &tq_10); break; //显示天气图标
				case 11: lv_img_set_src(TQ_TB1, &tq_11); break; //显示天气图标
				case 12: lv_img_set_src(TQ_TB1, &tq_12); break; //显示天气图标
				case 13: lv_img_set_src(TQ_TB1, &tq_13); break; //显示天气图标
				case 14: lv_img_set_src(TQ_TB1, &tq_14); break; //显示天气图标
				case 15: lv_img_set_src(TQ_TB1, &tq_15); break; //显示天气图标
				case 16: lv_img_set_src(TQ_TB1, &tq_16); break; //显示天气图标
				case 17: lv_img_set_src(TQ_TB1, &tq_17); break; //显示天气图标
				case 18: lv_img_set_src(TQ_TB1, &tq_18); break; //显示天气图标
				case 19: lv_img_set_src(TQ_TB1, &tq_19); break; //显示天气图标
				case 20: lv_img_set_src(TQ_TB1, &tq_20); break; //显示天气图标
				case 21: lv_img_set_src(TQ_TB1, &tq_21); break; //显示天气图标
				case 22: lv_img_set_src(TQ_TB1, &tq_22); break; //显示天气图标
				case 23: lv_img_set_src(TQ_TB1, &tq_23); break; //显示天气图标
				case 24: lv_img_set_src(TQ_TB1, &tq_24); break; //显示天气图标
				case 29: lv_img_set_src(TQ_TB1, &tq_29); break; //显示天气图标
				case 30: lv_img_set_src(TQ_TB1, &tq_30); break; //显示天气图标
				case 31: lv_img_set_src(TQ_TB1, &tq_31); break; //显示天气图标
				case 32: lv_img_set_src(TQ_TB1, &tq_32); break; //显示天气图标
				case 49: lv_img_set_src(TQ_TB1, &tq_49); break; //显示天气图标
				case 53: lv_img_set_src(TQ_TB1, &tq_53); break; //显示天气图标
				case 54: lv_img_set_src(TQ_TB1, &tq_54); break; //显示天气图标
				case 55: lv_img_set_src(TQ_TB1, &tq_55); break; //显示天气图标
				case 56: lv_img_set_src(TQ_TB1, &tq_56); break; //显示天气图标
				case 57: lv_img_set_src(TQ_TB1, &tq_57); break; //显示天气图标
				case 58: lv_img_set_src(TQ_TB1, &tq_58); break; //显示天气图标
				case 301: lv_img_set_src(TQ_TB1, &tq_301); break; //显示天气图标
				case 302: lv_img_set_src(TQ_TB1, &tq_302); break; //显示天气图标
				default: lv_img_set_src(TQ_TB1, &tq_99); break; //显示天气图标
			}

			i=atoi(weather2.xwtianqi);
			switch(i)
			{
                case 0: lv_img_set_src(TQ_TB2, &tq_0); break; //显示天气图标
                case 1: lv_img_set_src(TQ_TB2, &tq_1); break; //显示天气图标
                case 2: lv_img_set_src(TQ_TB2, &tq_2); break; //显示天气图标
				case 3: lv_img_set_src(TQ_TB2, &tq_3); break; //显示天气图标
				case 4: lv_img_set_src(TQ_TB2, &tq_4); break; //显示天气图标
				case 5: lv_img_set_src(TQ_TB2, &tq_5); break; //显示天气图标
				case 6: lv_img_set_src(TQ_TB2, &tq_6); break; //显示天气图标
				case 7: lv_img_set_src(TQ_TB2, &tq_7); break; //显示天气图标
				case 8: lv_img_set_src(TQ_TB2, &tq_8); break; //显示天气图标
				case 9: lv_img_set_src(TQ_TB2, &tq_9); break; //显示天气图标
				case 10: lv_img_set_src(TQ_TB2, &tq_10); break; //显示天气图标
				case 11: lv_img_set_src(TQ_TB2, &tq_11); break; //显示天气图标
				case 12: lv_img_set_src(TQ_TB2, &tq_12); break; //显示天气图标
				case 13: lv_img_set_src(TQ_TB2, &tq_13); break; //显示天气图标
				case 14: lv_img_set_src(TQ_TB2, &tq_14); break; //显示天气图标
				case 15: lv_img_set_src(TQ_TB2, &tq_15); break; //显示天气图标
				case 16: lv_img_set_src(TQ_TB2, &tq_16); break; //显示天气图标
				case 17: lv_img_set_src(TQ_TB2, &tq_17); break; //显示天气图标
				case 18: lv_img_set_src(TQ_TB2, &tq_18); break; //显示天气图标
				case 19: lv_img_set_src(TQ_TB2, &tq_19); break; //显示天气图标
				case 20: lv_img_set_src(TQ_TB2, &tq_20); break; //显示天气图标
				case 21: lv_img_set_src(TQ_TB2, &tq_21); break; //显示天气图标
				case 22: lv_img_set_src(TQ_TB2, &tq_22); break; //显示天气图标
				case 23: lv_img_set_src(TQ_TB2, &tq_23); break; //显示天气图标
				case 24: lv_img_set_src(TQ_TB2, &tq_24); break; //显示天气图标
				case 29: lv_img_set_src(TQ_TB2, &tq_29); break; //显示天气图标
				case 30: lv_img_set_src(TQ_TB2, &tq_30); break; //显示天气图标
				case 31: lv_img_set_src(TQ_TB2, &tq_31); break; //显示天气图标
				case 32: lv_img_set_src(TQ_TB2, &tq_32); break; //显示天气图标
				case 49: lv_img_set_src(TQ_TB2, &tq_49); break; //显示天气图标
				case 53: lv_img_set_src(TQ_TB2, &tq_53); break; //显示天气图标
				case 54: lv_img_set_src(TQ_TB2, &tq_54); break; //显示天气图标
				case 55: lv_img_set_src(TQ_TB2, &tq_55); break; //显示天气图标
				case 56: lv_img_set_src(TQ_TB2, &tq_56); break; //显示天气图标
				case 57: lv_img_set_src(TQ_TB2, &tq_57); break; //显示天气图标
				case 58: lv_img_set_src(TQ_TB2, &tq_58); break; //显示天气图标
				case 301: lv_img_set_src(TQ_TB2, &tq_301); break; //显示天气图标
				case 302: lv_img_set_src(TQ_TB2, &tq_302); break; //显示天气图标				
				default: lv_img_set_src(TQ_TB2, &tq_99); break; //显示天气图标
			}

			i=atoi(weather3.xwtianqi);
			switch(i)
			{
                case 0: lv_img_set_src(TQ_TB3, &tq_0); break; //显示天气图标
                case 1: lv_img_set_src(TQ_TB3, &tq_1); break; //显示天气图标
                case 2: lv_img_set_src(TQ_TB3, &tq_2); break; //显示天气图标
				case 3: lv_img_set_src(TQ_TB3, &tq_3); break; //显示天气图标
				case 4: lv_img_set_src(TQ_TB3, &tq_4); break; //显示天气图标
				case 5: lv_img_set_src(TQ_TB3, &tq_5); break; //显示天气图标
				case 6: lv_img_set_src(TQ_TB3, &tq_6); break; //显示天气图标
				case 7: lv_img_set_src(TQ_TB3, &tq_7); break; //显示天气图标
				case 8: lv_img_set_src(TQ_TB3, &tq_8); break; //显示天气图标
				case 9: lv_img_set_src(TQ_TB3, &tq_9); break; //显示天气图标
				case 10: lv_img_set_src(TQ_TB3, &tq_10); break; //显示天气图标
				case 11: lv_img_set_src(TQ_TB3, &tq_11); break; //显示天气图标
				case 12: lv_img_set_src(TQ_TB3, &tq_12); break; //显示天气图标
				case 13: lv_img_set_src(TQ_TB3, &tq_13); break; //显示天气图标
				case 14: lv_img_set_src(TQ_TB3, &tq_14); break; //显示天气图标
				case 15: lv_img_set_src(TQ_TB3, &tq_15); break; //显示天气图标
				case 16: lv_img_set_src(TQ_TB3, &tq_16); break; //显示天气图标
				case 17: lv_img_set_src(TQ_TB3, &tq_17); break; //显示天气图标
				case 18: lv_img_set_src(TQ_TB3, &tq_18); break; //显示天气图标
				case 19: lv_img_set_src(TQ_TB3, &tq_19); break; //显示天气图标
				case 20: lv_img_set_src(TQ_TB3, &tq_20); break; //显示天气图标
				case 21: lv_img_set_src(TQ_TB3, &tq_21); break; //显示天气图标
				case 22: lv_img_set_src(TQ_TB3, &tq_22); break; //显示天气图标
				case 23: lv_img_set_src(TQ_TB3, &tq_23); break; //显示天气图标
				case 24: lv_img_set_src(TQ_TB3, &tq_24); break; //显示天气图标
				case 29: lv_img_set_src(TQ_TB3, &tq_29); break; //显示天气图标
				case 30: lv_img_set_src(TQ_TB3, &tq_30); break; //显示天气图标
				case 31: lv_img_set_src(TQ_TB3, &tq_31); break; //显示天气图标
				case 32: lv_img_set_src(TQ_TB3, &tq_32); break; //显示天气图标
				case 49: lv_img_set_src(TQ_TB3, &tq_49); break; //显示天气图标
				case 53: lv_img_set_src(TQ_TB3, &tq_53); break; //显示天气图标
				case 54: lv_img_set_src(TQ_TB3, &tq_54); break; //显示天气图标
				case 55: lv_img_set_src(TQ_TB3, &tq_55); break; //显示天气图标
				case 56: lv_img_set_src(TQ_TB3, &tq_56); break; //显示天气图标
				case 57: lv_img_set_src(TQ_TB3, &tq_57); break; //显示天气图标
				case 58: lv_img_set_src(TQ_TB3, &tq_58); break; //显示天气图标
				case 301: lv_img_set_src(TQ_TB3, &tq_301); break; //显示天气图标
				case 302: lv_img_set_src(TQ_TB3, &tq_302); break; //显示天气图标				
				default: lv_img_set_src(TQ_TB3, &tq_99); break; //显示天气图标
			}

			for(int countdown = 500; countdown >= 0; countdown--) {
			vTaskDelay(1000 / portTICK_PERIOD_MS);
            }
		}
	}
	vTaskDelete(NULL); // 删除任务  防止出现问题，否则不会执行到这里
}

//联网成功后
void Successful_WIFI(void)
{
	xTaskCreate(Sntp_Task, "Sntp_Task", 1024*4, NULL, 3, NULL);//创建动态任务-网络时间任务
	xTaskCreate(http_get_task, "http_get_task", 1024*4, NULL, 4, NULL);  //创建动态任务-网络农历任务
	// 如果要使用任务创建图形，则需要创建固定任务,否则可能会出现诸如内存损坏等问题
	//创建动态任务--表盘任务
	xTaskCreatePinnedToCore(Bp_Task, "Bp_Task", 1024*10, NULL, 5, NULL, 1); //表盘显示任务
}

// SmartConfig 任务
static void SmartConfig_Task(void *parm)
{
	EventBits_t Bits,uxBits;

	while (1){
		//死等事件组：CONNECTED_BIT | ESPTOUCH_DONE_BIT   事件组等待位
		Bits = xEventGroupWaitBits(s_wifi_event_group, CONNECTED_BIT | WIFI_FAIL_BIT, true, false, portMAX_DELAY);
        //如果WIFI连接成功
		if(Bits & CONNECTED_BIT) {
            ESP_LOGI(TAG, "WiFi Connected to ap A");  //打印WIFI已连接AP
			lv_obj_del(img1); //删除开机windows图片对象
	        lv_obj_del(img2); //删除开机滚动图片对象
	        lv_task_del(task1);  //删除LVGL任务1  开机滚动
			task1 = NULL;
            Successful_WIFI();
			vTaskDelete(NULL);
		}
        //如果WIFI连接失败
		else if (Bits & WIFI_FAIL_BIT){
            ESP_LOGI(TAG, "WiFi connection failed A");  //打印WIFI连接失败
			lv_obj_del(img1); //删除开机windows图片对象
			lv_obj_del(img2); //删除开机滚动图片对象
			lv_task_del(task1);  //删除LVGL任务1  开机滚动
			task1 = NULL;
			//显示二维码图片
			img3 = lv_img_create(lv_scr_act(), NULL); //创建图像对象
			lv_img_set_src(img3, &ewm);	//将像素贴图设置为按图像显示
			lv_obj_align(img3, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);

			//微信配网
			ESP_ERROR_CHECK(esp_smartconfig_set_type(SC_TYPE_AIRKISS));
			//开始SmartConfig
			smartconfig_start_config_t cfg = SMARTCONFIG_START_CONFIG_DEFAULT();  //默认值
			ESP_ERROR_CHECK( esp_smartconfig_start(&cfg) );  //开始
			while (1){
				//死等事件组：CONNECTED_BIT | ESPTOUCH_DONE_BIT   事件组等待位  连接成功  和  smartconfing配网事件
				uxBits = xEventGroupWaitBits(s_wifi_event_group, CONNECTED_BIT | ESPTOUCH_DONE_BIT, true, false, portMAX_DELAY);
				//WIFI已连接AP
				if(uxBits & CONNECTED_BIT) {
					ESP_LOGI(TAG, "WiFi Connected to ap B");  //打印WIFI已连接AP
				}
				//SmartConfig结束
				else if (uxBits & ESPTOUCH_DONE_BIT){
					ESP_LOGI(TAG, "smartconfig over");  //打印配置结束
                    esp_smartconfig_stop();  //配网停止
					lv_obj_del(img3);	//删除二维码图片对象
                    Successful_WIFI();
					vTaskDelete(NULL);
				}
			}
		}
	}
}

//农历信息任务函数
static void http_get_task(void *parm)
{
    //socket的属性按IP版本设置问题
    const struct addrinfo hints = {
        .ai_family = AF_INET,  //ai_family参数指定调用者期待返回的套接口地址结构的类型。如果指定AF_INET，那么函数就不能返回任何IPV6相关的地址信息；
        .ai_socktype = SOCK_STREAM,  //SOCK_STREAM表示这个套接字是连接的一个端点。
    };
    struct addrinfo *res;
    struct in_addr *addr;
    int s, r;
    char recv_buf[512];  //512字节的网页数据缓冲区

	char csxx[70];  //保存城市-星期-农历-阳历等信息字符串  13+18+10+35
	char *T_csxx = csxx; // 缓存数组指针
	static char rwbz =1;   //防止获取城市代码任务再次进入

    while(1) {

        int err = getaddrinfo(WEB_SERVER, "80", &hints, &res);  //自动生成这些参数

        if(err != 0 || res == NULL) {  //如果返回参数不等于0  或者  指针地址为空
            ESP_LOGE(TAG, "DNS lookup failed err=%d res=%p", err, res);  //DNS查找失败错误
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;   //终止当前的循环  跳过后面的程序
        }

        /*用于打印已解析IP的代码。
        注意：inet\u ntoa是不可重入的，请查看ipaddr\u ntoa\r中的“真实”代码*/
        addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
        //inet_ntoa()是编程语言，功能是将网络地址转换成“.”点隔的字符串格式。
        //打印DNS   DNS lookup succeeded.   IP=182.118.39.166
        ESP_LOGI(TAG, "DNS lookup succeeded. IP=%s", inet_ntoa(*addr));  //DNS查找成功

        //创建套接字
        //ai_family 为地址族（Address Family），也就是 IP 地址类型，常用的有 AF_INET 和 AF_INET6。AF 是“Address Family”的简写，INET是“Inetnet”的简写。
        //ai_socktype 为数据传输方式/套接字类型，常用的有 SOCK_STREAM（流格式套接字/面向连接的套接字） 和 SOCK_DGRAM（数据报套接字/无连接的套接字）
        // 0 表示传输协议，常用的有 IPPROTO_TCP 和 IPPTOTO_UDP，分别表示 TCP 传输协议和 UDP 传输协议。
        //将 0 的值设为 0，系统会自动推演出应该使用什么协议   简化写法
        s = socket(res->ai_family, res->ai_socktype, 0);
        if(s < 0) {
            ESP_LOGE(TAG, "... Failed to allocate socket.");  //分配套接字失败。
            //由getaddrinfo返回的存储空间，包括addrinfo结构、ai_addr结构和ai_canonname字符串，都是用malloc动态获取的。
            //这些空间可调用 freeaddrinfo释放。
            freeaddrinfo(res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;   //终止循环
        }
        ESP_LOGI(TAG, "... allocated socket\r\n");  //分配的套接字

        //connect()用于建立与指定socket的连接。
        //s：标识一个未连接socket
        //ai_addr：指向要连接套接字的sockaddr结构体的指针
        //ai_addrlen：sockaddr结构体的字节长度
        if(connect(s, res->ai_addr, res->ai_addrlen) != 0) {
            ESP_LOGE(TAG, "... socket connect failed errno=%d", errno);//套接字连接失败错误号=
            close(s);//关闭文件
            freeaddrinfo(res); //释放由getaddrinfo返回的存储空间
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue; 
        }

        ESP_LOGI(TAG, "... connected");
        freeaddrinfo(res);//释放由getaddrinfo返回的存储空间

        if (write(s, REQUEST, strlen(REQUEST)) < 0) {
            ESP_LOGE(TAG, "... socket send failed");//套接字发送失败
            close(s);//关闭文件
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }

        ESP_LOGI(TAG, "... socket send success");  //套接字发送成功

            //城市--汉字
            char *substr="/></div>"; //查找第一处字符串  遇到"时前面加\; 
            char *substr1="/></div>";//字符串的左部分
			char *substr2="天气";   //字符串的右部分

			//城市拼音信息
			char *substr12="href=\"http://"; //查找第一处字符串  遇到"时前面加\; 
            char *substr13="href=\"http://";//字符串的左部分
			char *substr14=".tianqi.com/?tq\"";   //字符串的右部分

			//公历-年月日
            char *substr3="wtwind\">"; //查找第一处字符串  遇到"时前面加\; 
            char *substr4="wtwind\">";//字符串的左部分
			char *substr5="星期";   //字符串的右部分

			//星期信息
			char *substr6="日  "; //查找第一处字符串  遇到"时前面加\; 
            char *substr7="日  ";//字符串的左部分
			char *substr8=" <br>";   //字符串的右部分

            //农历信息
			char *substr9=" <br>"; //查找第一处字符串  遇到"时前面加\; 
            char *substr10=" <br>";//字符串的左部分
			char *substr11="</div>";   //字符串的右部分

			char *st=NULL;  //暂存第一次出现字符串的首位置
			char *st1=NULL;  //要提取的字符串首位置
            
        /*读取HTTP响应*/
        do {
            bzero(recv_buf, sizeof(recv_buf));//recv_buf要置0的数据的起始地址；sizeof(recv_buf)要置0的数据字节个数
            //函数从打开的设备或文件中读取数据。
            //返回值：成功返回读取的字节数，出错返回-1并设置errno，如果在调read之前已到达文件末尾，则这次read返回0
            //S为要读的文件   
            //读上来的数据保存在缓冲区recv_buf中
            //字节数
            r = read(s, recv_buf, sizeof(recv_buf)-1);  //局部变量r表示系统实际所读取的字符数量

            //获取城市信息
            st=strstr(recv_buf,substr); //在字符串recv_buf中查找第一次出现字符串的位置，不包含终止符'\0'
			if(st!=NULL){  //不为空说明找到了字符串
			 st1=splitx(st,substr1,substr2);//将字符串ST分割，分割符号"temp\":\""，要分割的第几个符号
			 st=NULL;
			}
			if(st1!=NULL&&st1!=ESP_OK){  //如果ST1不为空并且ST1不为OK  --正常时
            lv_label_set_text(label4, st1); //文本显示位置信息
			printf("城市=%s\n",st1);	 //打印获取到的
			st1=NULL;  //将ST1置空
			}

			//获取城市拼音信息
            st=strstr(recv_buf,substr12); //在字符串recv_buf中查找第一次出现字符串的位置，不包含终止符'\0'
			if(st!=NULL){  //不为空说明找到了字符串
			 st1=splitx(st,substr13,substr14);//将字符串ST分割，分割符号"temp\":\""，要分割的第几个符号
			 st=NULL;
			}
			if(st1!=NULL&&st1!=ESP_OK){  //如果ST1不为空并且ST1不为OK  --正常时
			T_cspy = st1;
			printf("城市拼音=%s\n",T_cspy);	 //打印获取到的
			if(rwbz)
			{  
				rwbz=0;  //任务标志置0  防重入
				xTaskCreate(http_get_task1, "http_get_task1", 1024*4, NULL, 3, NULL);  //创建动态任务-网络获得城市代码任务   只创建一次
		    }
			st1=NULL;  //将ST1置空
			}

			//获取公历-年月日信息
            st=strstr(recv_buf,substr3); //在字符串recv_buf中查找第一次出现字符串的位置，不包含终止符'\0'
			if(st!=NULL){  //不为空说明找到了字符串
			 st1=splitx(st,substr4,substr5);//将字符串ST分割，分割符号"temp\":\""，要分割的第几个符号
			 st=NULL;
			}
			if(st1!=NULL&&st1!=ESP_OK){  //如果ST1不为空并且ST1不为OK  --正常时

			memcpy(T_csxx, st1, 17); //城市复制到
			T_csxx[17] = ' ';
            T_csxx += 18;
			printf("公历=%s\n",st1);	 //打印获取到的
			st1=NULL;  //将ST1置空
			}

			//获取星期信息
            st=strstr(recv_buf,substr6); //在字符串recv_buf中查找第一次出现字符串的位置，不包含终止符'\0'
			if(st!=NULL){  //不为空说明找到了字符串
			 st1=splitx(st,substr7,substr8);//将字符串ST分割，分割符号"temp\":\""，要分割的第几个符号
			 st=NULL;
			}
			if(st1!=NULL&&st1!=ESP_OK){  //如果ST1不为空并且ST1不为OK  --正常时
			memcpy(T_csxx, st1, 9); //城市复制到
			T_csxx[9] = ' ';
            T_csxx += 10;
			printf("星期=%s\n",st1);	 //打印获取到的
			st1=NULL;  //将ST1置空
			}

			//获取农历信息
            st=strstr(recv_buf,substr9); //在字符串recv_buf中查找第一次出现字符串的位置，不包含终止符'\0'
			if(st!=NULL){  //不为空说明找到了字符串
			 st1=splitx(st,substr10,substr11);//将字符串ST分割，分割符号"temp\":\""，要分割的第几个符号
			 st=NULL;
			}
			if(st1!=NULL&&st1!=ESP_OK){  //如果ST1不为空并且ST1不为OK  --正常时
		    T_csxx[31] = ' ';
			T_csxx[32] = ' ';
			T_csxx[33] = ' ';
			T_csxx[34] = ' ';
			memcpy(T_csxx, st1, 35); //城市复制到
			T_csxx[35] = '\0';
            T_csxx += 36;   
			lv_label_set_text(label2, csxx);// 设置显示文本
			printf("农历=%s\n",st1);	 //打印获取到的
			st1=NULL;  //将ST1置空
            printf("csxx=%s\n",csxx);	 //打印获取到的			
			break;
			}
        } while(r > 0);  //如果实际读取的字符数量大于0   说明内容没有读取完
        //打印剩余多少内容没有读取
        ESP_LOGI(TAG, "... done reading from socket. Last read return=%d errno=%d\r\n", r, errno);
        close(s);  //关闭文件

        for(int countdown = 360; countdown >= 0; countdown--) {
            ESP_LOGI(TAG, "%d... ", countdown);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        ESP_LOGI(TAG, "Starting again!");  //打印重新开始
    }
}

//获取城市代码-任务函数  
static void http_get_task1(void *parm)
{
    //socket的属性按IP版本设置问题
    const struct addrinfo hints = {
        .ai_family = AF_INET,  //ai_family参数指定调用者期待返回的套接口地址结构的类型。如果指定AF_INET，那么函数就不能返回任何IPV6相关的地址信息；
        .ai_socktype = SOCK_STREAM,  //SOCK_STREAM表示这个套接字是连接的一个端点。
    };
    struct addrinfo *res;
    struct in_addr *addr;
    int s, r;
    char recv_buf[512];  //512字节的网页数据缓冲区

	strcat(cspy2, T_cspy);
	strcat(cspy2, REQUEST1);

    while(1) {
        int err = getaddrinfo(WEB_SERVER1, "80", &hints, &res);  //自动生成这些参数

        if(err != 0 || res == NULL) {  //如果返回参数不等于0  或者  指针地址为空
            ESP_LOGE(TAG, "DNS lookup failed err=%d res=%p", err, res);  //DNS查找失败错误
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;   //终止当前的循环  跳过后面的程序
        }

        addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
        //打印DNS   DNS lookup succeeded.   IP=182.118.39.166
        ESP_LOGI(TAG, "DNS lookup succeeded. IP=%s", inet_ntoa(*addr));  //DNS查找成功
        //创建套接字
        s = socket(res->ai_family, res->ai_socktype, 0);
        if(s < 0) {
            ESP_LOGE(TAG, "... Failed to allocate socket.");  //分配套接字失败。
            freeaddrinfo(res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;   //终止循环
        }
        ESP_LOGI(TAG, "... allocated socket\r\n");  //分配的套接字
        //connect()用于建立与指定socket的连接。
        if(connect(s, res->ai_addr, res->ai_addrlen) != 0) {
            ESP_LOGE(TAG, "... socket connect failed errno=%d", errno);//套接字连接失败错误号=
            close(s);//关闭文件
            freeaddrinfo(res); //释放由getaddrinfo返回的存储空间
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue; 
        }
        ESP_LOGI(TAG, "... connected");
        freeaddrinfo(res);//释放由getaddrinfo返回的存储空间

        if (write(s, T_cspy2, strlen(T_cspy2)) < 0) {
            ESP_LOGE(TAG, "... socket send failed");//套接字发送失败
            close(s);//关闭文件
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... socket send success");  //套接字发送成功

            //城市代码数组
            char *substr="([{\"ref\":\""; //查找第一处字符串  遇到"时前面加\; 
            char *substr1="([{\"ref\":\"";//字符串的左部分
			char *substr2="~";   //字符串的右部分

			char *st=NULL;  //暂存第一次出现字符串的首位置
			char *st1=NULL;  //要提取的字符串首位置
            
        /*读取HTTP响应*/
        do {
            bzero(recv_buf, sizeof(recv_buf));//recv_buf要置0的数据的起始地址；sizeof(recv_buf)要置0的数据字节个数
            //函数从打开的设备或文件中读取数据。
            r = read(s, recv_buf, sizeof(recv_buf)-1);  //局部变量r表示系统实际所读取的字符数量

            //获取城市信息
            st=strstr(recv_buf,substr); //在字符串recv_buf中查找第一次出现字符串的位置，不包含终止符'\0'
			if(st!=NULL){  //不为空说明找到了字符串
			 st1=splitx(st,substr1,substr2);//将字符串ST分割，分割符号"temp\":\""，要分割的第几个符号
			 st=NULL;
			}
			if(st1!=NULL&&st1!=ESP_OK){  //如果ST1不为空并且ST1不为OK  --正常时
			printf("城市代码=%s\n",st1);	 //打印获取到的
			T_csdm = st1;  //城市代码  指针地址赋值 到另一个指针变量的地址
			printf("T_csdm=%s\n",T_csdm);	 //打印获取到的
            xTaskCreate(http_get_task2, "http_get_task2", 1024*6, NULL, 3, NULL);  //创建动态任务-天气网任务
            vTaskDelete(NULL);  //获取城市成功-删除当前任务
			st1=NULL;  //将ST1置空
			break;
			}
        } while(r > 0);  //如果实际读取的字符数量大于0   说明内容没有读取完

        //打印剩余多少内容没有读取
        ESP_LOGI(TAG, "... done reading from socket. Last read return=%d errno=%d\r\n", r, errno);
        close(s);  //关闭文件
        for(int countdown = 40; countdown >= 0; countdown--) {
            ESP_LOGI(TAG, "%d... ", countdown);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        ESP_LOGI(TAG, "Starting again 1!");  //打印重新开始
    }
}

//天气网任务函数  
static void http_get_task2(void *parm)
{
    //socket的属性按IP版本设置问题
    const struct addrinfo hints = {
        .ai_family = AF_INET,  //ai_family参数指定调用者期待返回的套接口地址结构的类型。如果指定AF_INET，那么函数就不能返回任何IPV6相关的地址信息；
        .ai_socktype = SOCK_STREAM,  //SOCK_STREAM表示这个套接字是连接的一个端点。
    };
    struct addrinfo *res;
    struct in_addr *addr;
    int s, r, i=1 , j=1;

    char recv_buf[1263];  //512字节的网页数据缓冲区  356
    char *T_buf=NULL;
	strcat(p, T_csdm);  //将城市代码数组  放到  P字符串后面
	strcat(p, REQUEST2); //将完整请求头  放到  P字符串后面

	//内容开头
	//日子汉字
	char *substr="（今天）</h1>";
	char *substr1="<h1>";
	char *substr2="</h1>";
	//下午天气图标
	char *substr3="<big class=\"png40 n";
	char *substr4="\"></big>";
	//今日天气汉字
	char *substr5="class=\"wea\">";
	char *substr6="</p>";
	//最高温度
	char *substr7="<span>";
	char *substr8="</span>";
	//最低温度
	char *substr9="<i>";
	char *substr10="</i>";
	//风向
	char *substr11="<span title=\"";
	char *substr12="\" class=";
	//风级
	char *substr13="<i>";
	char *substr14="</i>";

	char *st=NULL;  //暂存第一次出现字符串的首位置
	char *st1=NULL;  //暂存第一次出现字符串的首位置

    while(1) {
        int err = getaddrinfo(WEB_SERVER2, "80", &hints, &res);  //自动生成这些参数
        if(err != 0 || res == NULL) {  //如果返回参数不等于0  或者  指针地址为空
            ESP_LOGE(TAG, "DNS lookup failed err=%d res=%p", err, res);  //DNS查找失败错误
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;   //终止当前的循环  跳过后面的程序
        }
        addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
        ESP_LOGI(TAG, "DNS lookup succeeded. IP=%s", inet_ntoa(*addr));  //DNS查找成功
        //创建套接字
        s = socket(res->ai_family, res->ai_socktype, 0);
        if(s < 0) {
            ESP_LOGE(TAG, "... Failed to allocate socket.");  //分配套接字失败。
            freeaddrinfo(res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;   //终止循环
        }
        ESP_LOGI(TAG, "... allocated socket\r\n");  //分配的套接字
        if(connect(s, res->ai_addr, res->ai_addrlen) != 0) {
            ESP_LOGE(TAG, "... socket connect failed errno=%d", errno);//套接字连接失败错误号=
            close(s);//关闭文件
            freeaddrinfo(res); //释放由getaddrinfo返回的存储空间
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue; 
        }
        ESP_LOGI(TAG, "... connected");
        freeaddrinfo(res);//释放由getaddrinfo返回的存储空间
        if (write(s, pp, strlen(pp)) < 0) {
            ESP_LOGE(TAG, "... socket send failed");//套接字发送失败
            close(s);//关闭文件
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... socket send success");  //套接字发送成功

        /*读取HTTP响应*/
        do {
            bzero(recv_buf, sizeof(recv_buf));//recv_buf要置0的数据的起始地址；sizeof(recv_buf)要置0的数据字节个数
            //函数从打开的设备或文件中读取数据。
            r = read(s, recv_buf, sizeof(recv_buf)-1);  //局部变量r表示系统实际所读取的字符数量
            //获取城市信息
            st=strstr(recv_buf,substr); //在字符串recv_buf中查找第一次出现字符串的位置，不包含终止符'\0'
			if(st!=NULL){  //不为空说明找到了字符串
			 printf("recv_buf=%s\n",recv_buf);	 //打印获取到的

			 st=strstr(recv_buf,substr1); //在字符串recv_buf中查找第一次出现字符串的位置，不包含终止符'\0'
			 st1=splitx(st,substr1,substr2);//将字符串ST分割，分割符号"temp\":\""，要分割的第几个符号
			 if(st1!=NULL&&st1!=ESP_OK){  //如果ST1不为空并且ST1不为OK  --正常时
			  weather1.rizi=st1;
			  printf("今天=%s\n",weather1.rizi);	 //打印获取到的
			 }

			 st=strstr(recv_buf,substr3); //在字符串recv_buf中查找第一次出现字符串的位置，不包含终止符'\0'
			 st1=splitx(st,substr3,substr4);//将字符串ST分割，分割符号"temp\":\""，要分割的第几个符号
			 if(st1!=NULL&&st1!=ESP_OK){  //如果ST1不为空并且ST1不为OK  --正常时
			  weather1.xwtianqi=st1;
			  printf("今天图标=%s\n",weather1.xwtianqi);	 //打印获取到的
			 }

			 st=strstr(recv_buf,substr5); //在字符串recv_buf中查找第一次出现字符串的位置，不包含终止符'\0'
			 st1=splitx(st,substr5,substr6);//将字符串ST分割，分割符号"temp\":\""，要分割的第几个符号
			 if(st1!=NULL&&st1!=ESP_OK){  //如果ST1不为空并且ST1不为OK  --正常时
			  weather1.tianqi=st1;
			  printf("今天天气=%s\n",weather1.tianqi);	 //打印获取到的
			 }

			 st=strstr(recv_buf,substr7); //在字符串recv_buf中查找第一次出现字符串的位置，不包含终止符'\0'
			 st1=splitx(st,substr7,substr8);//将字符串ST分割，分割符号"temp\":\""，要分割的第几个符号
			 if(st1!=NULL&&st1!=ESP_OK){  //如果ST1不为空并且ST1不为OK  --正常时
			  weather1.zgwendu=st1;
			  printf("今天最高温度=%s\n",weather1.zgwendu);	 //打印获取到的
			 }

			 st=strstr(recv_buf,substr9); //在字符串recv_buf中查找第一次出现字符串的位置，不包含终止符'\0'
			 st1=splitx(st,substr9,substr10);//将字符串ST分割，分割符号"temp\":\""，要分割的第几个符号
			 if(st1!=NULL&&st1!=ESP_OK){  //如果ST1不为空并且ST1不为OK  --正常时
			  weather1.zdwendu=st1;
			  printf("今天最低温度=%s\n",weather1.zdwendu);	 //打印获取到的
			 }

			 st=strstr(recv_buf,substr11); //在字符串recv_buf中查找第一次出现字符串的位置，不包含终止符'\0'
			 st1=splitx(st,substr11,substr12);//将字符串ST分割，分割符号"temp\":\""，要分割的第几个符号
			 if(st1!=NULL&&st1!=ESP_OK){  //如果ST1不为空并且ST1不为OK  --正常时
			  weather1.fengx=st1;
			  printf("今天风向=%s\n",weather1.fengx);	 //打印获取到的
			 }			 

			 st=strstr(recv_buf,substr11); //在字符串recv_buf中查找第一次出现字符串的位置，不包含终止符'\0'
			 st1=splitx(st,substr13,substr14);//将字符串ST分割，分割符号"temp\":\""，要分割的第几个符号
			 if(st1!=NULL&&st1!=ESP_OK){  //如果ST1不为空并且ST1不为OK  --正常时
			  weather1.fengji=st1;
			  printf("今天风力=%s\n",weather1.fengji);	 //打印获取到的
			 }
             T_buf=&recv_buf[0];  //512字节的网页数据缓冲区  356
			 T_buf=T_buf+480;
			 //printf("T_buf=%s\n",T_buf);	 //打印获取到的
			 st=strstr(T_buf,substr1); //在字符串recv_buf中查找第一次出现字符串的位置，不包含终止符'\0'
			 st1=splitx(st,substr1,substr2);//将字符串ST分割，分割符号"temp\":\""，要分割的第几个符号
			 if(st1!=NULL&&st1!=ESP_OK){  //如果ST1不为空并且ST1不为OK  --正常时
			  weather2.rizi=st1;
			  printf("明天=%s\n",weather2.rizi);	 //打印获取到的
			 }	

			 st=strstr(T_buf,substr3); //在字符串recv_buf中查找第一次出现字符串的位置，不包含终止符'\0'
			 st1=splitx(st,substr3,substr4);//将字符串ST分割，分割符号"temp\":\""，要分割的第几个符号
			 if(st1!=NULL&&st1!=ESP_OK){  //如果ST1不为空并且ST1不为OK  --正常时
			  weather2.xwtianqi=st1;
			  printf("明天图标=%s\n",weather2.xwtianqi);	 //打印获取到的
			 }

			 st=strstr(T_buf,substr5); //在字符串recv_buf中查找第一次出现字符串的位置，不包含终止符'\0'
			 st1=splitx(st,substr5,substr6);//将字符串ST分割，分割符号"temp\":\""，要分割的第几个符号
			 if(st1!=NULL&&st1!=ESP_OK){  //如果ST1不为空并且ST1不为OK  --正常时
			  weather2.tianqi=st1;
			  printf("明天天气=%s\n",weather2.tianqi);	 //打印获取到的
			 }

			 st=strstr(T_buf,substr7); //在字符串recv_buf中查找第一次出现字符串的位置，不包含终止符'\0'
			 st1=splitx(st,substr7,substr8);//将字符串ST分割，分割符号"temp\":\""，要分割的第几个符号
			 if(st1!=NULL&&st1!=ESP_OK){  //如果ST1不为空并且ST1不为OK  --正常时
			  weather2.zgwendu=st1;
			  printf("明天最高温度=%s\n",weather2.zgwendu);	 //打印获取到的
			 }

			 st=strstr(T_buf,substr5); //在字符串recv_buf中查找第一次出现字符串的位置，不包含终止符'\0'
			 st1=splitx(st,substr9,substr10);//将字符串ST分割，分割符号"temp\":\""，要分割的第几个符号
			 if(st1!=NULL&&st1!=ESP_OK){  //如果ST1不为空并且ST1不为OK  --正常时
			  weather2.zdwendu=st1;
			  printf("明天最低温度=%s\n",weather2.zdwendu);	 //打印获取到的
			 }

			 st=strstr(T_buf,substr11); //在字符串recv_buf中查找第一次出现字符串的位置，不包含终止符'\0'
			 st1=splitx(st,substr11,substr12);//将字符串ST分割，分割符号"temp\":\""，要分割的第几个符号
			 if(st1!=NULL&&st1!=ESP_OK){  //如果ST1不为空并且ST1不为OK  --正常时
			  weather2.fengx=st1;
			  printf("明天风向=%s\n",weather2.fengx);	 //打印获取到的
			 }			 

			 st=strstr(T_buf,substr11); //在字符串recv_buf中查找第一次出现字符串的位置，不包含终止符'\0'
			 st1=splitx(st,substr13,substr14);//将字符串ST分割，分割符号"temp\":\""，要分割的第几个符号
			 if(st1!=NULL&&st1!=ESP_OK){  //如果ST1不为空并且ST1不为OK  --正常时
			  weather2.fengji=st1;
			  printf("明天风力=%s\n",weather2.fengji);	 //打印获取到的
			 }
			 T_buf=T_buf+340;	  		 		 	 
             //printf("T_buf=%s\n",T_buf);	 //打印获取到的
			 st=strstr(T_buf,substr1); //在字符串recv_buf中查找第一次出现字符串的位置，不包含终止符'\0'
			 st1=splitx(st,substr1,substr2);//将字符串ST分割，分割符号"temp\":\""，要分割的第几个符号
			 if(st1!=NULL&&st1!=ESP_OK){  //如果ST1不为空并且ST1不为OK  --正常时
			  weather3.rizi=st1;
			  printf("后天=%s\n",weather3.rizi);	 //打印获取到的
			 }	

			 st=strstr(T_buf,substr3); //在字符串recv_buf中查找第一次出现字符串的位置，不包含终止符'\0'
			 st1=splitx(st,substr3,substr4);//将字符串ST分割，分割符号"temp\":\""，要分割的第几个符号
			 if(st1!=NULL&&st1!=ESP_OK){  //如果ST1不为空并且ST1不为OK  --正常时
			  weather3.xwtianqi=st1;
			  printf("后天图标=%s\n",weather3.xwtianqi);	 //打印获取到的
			 }

			 st=strstr(T_buf,substr5); //在字符串recv_buf中查找第一次出现字符串的位置，不包含终止符'\0'
			 st1=splitx(st,substr5,substr6);//将字符串ST分割，分割符号"temp\":\""，要分割的第几个符号
			 if(st1!=NULL&&st1!=ESP_OK){  //如果ST1不为空并且ST1不为OK  --正常时
			  weather3.tianqi=st1;
			  printf("后天天气=%s\n",weather3.tianqi);	 //打印获取到的
			 }

			 st=strstr(T_buf,substr7); //在字符串recv_buf中查找第一次出现字符串的位置，不包含终止符'\0'
			 st1=splitx(st,substr7,substr8);//将字符串ST分割，分割符号"temp\":\""，要分割的第几个符号
			 if(st1!=NULL&&st1!=ESP_OK){  //如果ST1不为空并且ST1不为OK  --正常时
			  weather3.zgwendu=st1;
			  printf("后天最高温度=%s\n",weather3.zgwendu);	 //打印获取到的
			 }

			 st=strstr(T_buf,substr5); //在字符串recv_buf中查找第一次出现字符串的位置，不包含终止符'\0'
			 st1=splitx(st,substr9,substr10);//将字符串ST分割，分割符号"temp\":\""，要分割的第几个符号
			 if(st1!=NULL&&st1!=ESP_OK){  //如果ST1不为空并且ST1不为OK  --正常时
			  weather3.zdwendu=st1;
			  printf("后天最低温度=%s\n",weather3.zdwendu);	 //打印获取到的
			 }

			 st=strstr(T_buf,substr11); //在字符串recv_buf中查找第一次出现字符串的位置，不包含终止符'\0'
			 st1=splitx(st,substr11,substr12);//将字符串ST分割，分割符号"temp\":\""，要分割的第几个符号
			 if(st1!=NULL&&st1!=ESP_OK){  //如果ST1不为空并且ST1不为OK  --正常时
			  weather3.fengx=st1;
			  printf("后天风向=%s\n",weather3.fengx);	 //打印获取到的
			 }			 

			 st=strstr(T_buf,substr11); //在字符串recv_buf中查找第一次出现字符串的位置，不包含终止符'\0'
			 st1=splitx(st,substr13,substr14);//将字符串ST分割，分割符号"temp\":\""，要分割的第几个符号
			 if(st1!=NULL&&st1!=ESP_OK){  //如果ST1不为空并且ST1不为OK  --正常时
			  weather3.fengji=st1;
			  printf("后天风力=%s\n",weather3.fengji);	 //打印获取到的
			 }

			 st1=NULL;  //将ST1置空
			 st=NULL;
			 break;
			}
        } while(r > 0);  //如果实际读取的字符数量大于0   说明内容没有读取完

        //打印剩余多少内容没有读取
        ESP_LOGI(TAG, "... done reading from socket. Last read return=%d errno=%d\r\n", r, errno);
        close(s);  //关闭文件

        i=strcmp(weather1.rizi,weather2.rizi);
        j=strcmp(weather3.rizi,weather2.rizi);
        for(int countdown = 300; countdown >= 0; countdown--) {
			if((i==0)||(j==0))  //说明采的数据有问题
		    {
			   break;
            }else weather1.biaozhiwei=1;
            ESP_LOGI(TAG, "%d... ", countdown);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }

    }
}

//SNTP获取并显示网络时间
static void Sntp_Task(void *parm)
{
	initialize_sntp(); //初始化Sntp

    static lv_style_t font_style;  //创建一个样式
    lv_style_init(&font_style);  //初始化样式
    lv_style_set_text_font(&font_style, LV_STATE_DEFAULT, &lv_font_montserrat_24);  //设置字体为24号

	label3 = lv_label_create(lv_scr_act(), NULL);   //创建时间文本标签
	lv_style_set_text_color(&font_style, LV_STATE_DEFAULT, LV_COLOR_WHITE); // 设置字体颜色
	lv_label_set_text(label3, NULL);	//文本显示--时分秒
	lv_obj_align(label3, NULL, LV_ALIGN_IN_TOP_LEFT,10,10); //显示位置 对齐方式
    lv_obj_add_style(label3,LV_LABEL_PART_MAIN, &font_style);  //使用样式

	while (1)
	{
		getNowTime();	 //打印时间
		//sntp_stop();  //停止SNTP
		vTaskDelay(100); //系统延时1S
	}
}

//初始化Sntp
void initialize_sntp(void)
{
	ESP_LOGI(TAG, "------------Initializing SNTP-----------");
	sntp_setoperatingmode(SNTP_OPMODE_POLL);  //设置操作模式为单播模式
	sntp_setservername(0, "ntp1.aliyun.com"); //设置访问服务器
	sntp_init();
}

//打印当前网络时间
void getNowTime(void)
{
	time_t now = 0;
	struct tm timeinfo = {0};
	char strftime_buf[30]; // 实际时间的字符串

	uint8_t C_Str = 0;				 // 字符串字节计数
	char A_Str_Data[20] = {0};		 // 【"日期"】字符串数组
	char *T_A_Str_Data = A_Str_Data; // 缓存数组指针
	char A_Str_Clock[10] = {0};		 // 【"时间"】字符串数组
	char *Str_Head_Week;			 // 【"星期"】字符串首地址
	char *Str_Head_Month;			 // 【"月份"】字符串首地址
	char *Str_Head_Day;				 // 【"日数"】字符串首地址
	char *Str_Head_Clock;			 // 【"时钟"】字符串首地址
	char *Str_Head_Year;			 // 【"年份"】字符串首地址

	setenv("TZ", "CST-8", 1); //改变或增加环境变量的内容  环境变量名称，变量内容，是否要改变已存在的环境变量
	tzset();				  //设置环境变量

	time(&now);					  //获取网络时间， 64bit的秒计数
	localtime_r(&now, &timeinfo); //转换成具体的时间参数  C语言自带函数-可重入函数，线程安全

	ESP_LOGI(TAG, "-------current time: %d:%d:%d:%d:%d:%d:%d:%d:%d", //显示获取到的时间戳
			 timeinfo.tm_isdst, timeinfo.tm_yday,
			 timeinfo.tm_wday, timeinfo.tm_year,
			 timeinfo.tm_mon, timeinfo.tm_mday,
			 timeinfo.tm_hour, timeinfo.tm_min,
			 timeinfo.tm_sec);
	strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo); //将时间格式化   保存地址，最大字符数，定义格式，指向TM结构的指针

	ESP_LOGI(TAG, "The current date / time in China is: %s", strftime_buf); //打印解析过的时间年月日

	// 【"年份" + ' '】填入日期数组
	Str_Head_Year = strftime_buf; // 设置起始地址
	while (*Str_Head_Year)		  // 找到【"实际时间"】字符串的结束字符'\0'
		Str_Head_Year++;		  //地址自增1
	// 【注：API返回的实际时间字符串，最后还有一个换行符，所以这里 -5】
	Str_Head_Year -= 4; // 获取【"年份"】字符串的首地址
	T_A_Str_Data[4] = ' ';
	memcpy(T_A_Str_Data, Str_Head_Year, 4); // 【"年份" + ' '】填入日期数组
	T_A_Str_Data += 5;						// 指向【"年份" + ' '】字符串的后面的地址

	// 获取【日期】字符串的首地址
	Str_Head_Week = strftime_buf;
	// "星期" 字符串的首地址
	Str_Head_Month = strstr(Str_Head_Week, " ") + 1; // "月份" 字符串的首地址
	Str_Head_Day = strstr(Str_Head_Month, " ") + 2;	 // "日数" 字符串的首地址
	Str_Head_Clock = strstr(Str_Head_Day, " ") + 1;	 // "时钟" 字符串的首地址

	// 【"月份" + ' '】填入日期数组
	C_Str = Str_Head_Day - Str_Head_Month;		 // 【"月份" + ' '】的字节数
	memcpy(T_A_Str_Data, Str_Head_Month, C_Str); // 【"月份" + ' '】填入日期数组
	T_A_Str_Data += C_Str;						 // 指向【"月份" + ' '】字符串的后面的地址

	// 【"日数" + ' '】填入日期数组
	C_Str = Str_Head_Clock - Str_Head_Day;	   // 【"日数" + ' '】的字节数
	memcpy(T_A_Str_Data, Str_Head_Day, C_Str); // 【"日数" + ' '】填入日期数组
	T_A_Str_Data += C_Str;					   // 指向【"日数" + ' '】字符串的后面的地址

	// 【"星期" + ' '】填入日期数组
	C_Str = Str_Head_Month - Str_Head_Week - 1; // 【"星期"】的字节数
	memcpy(T_A_Str_Data, Str_Head_Week, C_Str); // 【"星期"】填入日期数组
	T_A_Str_Data += C_Str;						// 指向【"星期"】字符串的后面的地址

    //时间确定。连接到WiFi并通过NTP获得时间
    if (timeinfo.tm_year > (2016 - 1900)) {
		// LCD显示【"日期"】、【"时钟"】字符串
		memcpy(A_Str_Clock, Str_Head_Clock, 8); // 【"时钟"】字符串填入时钟数组
		A_Str_Clock[8] = '\0';
		lv_label_set_text(label3, A_Str_Clock); //文本显示--动态文本（字符串形式）LCD显示时间
    }
}

//LVGL开机图任务
void task1_cb(void)
{
	static uint8_t page_id = 0;
	if (page_id == 0){page_id = 1;lv_img_set_src(img2, &windows_gd0);}
	else if (page_id == 1){page_id = 2;lv_img_set_src(img2, &windows_gd1);}
	else if (page_id == 2){page_id = 3;lv_img_set_src(img2, &windows_gd2);}
	else if (page_id == 3){page_id = 4;lv_img_set_src(img2, &windows_gd3);}
	else if (page_id == 4){page_id = 5;lv_img_set_src(img2, &windows_gd4);}
	else if (page_id == 5){page_id = 6;lv_img_set_src(img2, &windows_gd5);}
	else if (page_id == 6){page_id = 7;lv_img_set_src(img2, &windows_gd6);}
	else if (page_id == 7){page_id = 8;lv_img_set_src(img2, &windows_gd7);}
	else if (page_id == 8){page_id = 9;lv_img_set_src(img2, &windows_gd8);}
	else if (page_id == 9){page_id = 10;lv_img_set_src(img2, &windows_gd9);}
	else if (page_id == 10){page_id = 11;lv_img_set_src(img2, &windows_gd10);}
	else if (page_id == 11){page_id = 0;lv_img_set_src(img2, &windows_gd11);}
	//lv_task_del(task1);task1 = NULL;
}

//显示开机图片
//创建LVGL图片刷新任务
void img_test(void)
{
	//显示开机背景--windows
	img1 = lv_img_create(lv_scr_act(), NULL);
	lv_img_set_src(img1, &windows_kj0);
	lv_obj_align(img1, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
    //显示滚动图片第一张
	img2 = lv_img_create(lv_scr_act(), NULL);
	lv_img_set_src(img2, &windows_gd0);
	lv_obj_align(img2, NULL, LV_ALIGN_IN_TOP_LEFT, 78, 164);
    //创建一个LVGL任务--每100个节拍运行一次
	task1 = lv_task_create((lv_task_cb_t)task1_cb, 100, LV_TASK_PRIO_MID, NULL);
}

// 主函数
void app_main()
{
	ESP_LOGI(TAG, "\nAPP is start!~\n");
	ESP_ERROR_CHECK( nvs_flash_init() );  //初始化存储
    ESP_ERROR_CHECK(esp_netif_init());  //初始化网络接口

    /* 初始化非易失性存储库 (NVS) */
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        /* NVS分区被截断，需要删除,然后重新初始化NVS */
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );

	// 如果要使用任务创建图形，则需要创建固定任务,否则可能会出现诸如内存损坏等问题
	xTaskCreatePinnedToCore(guiTask, "gui", 4096*2, NULL, 4, NULL, 1);  //初始化显示以及开机界面

    /* 定义一个NVS操作句柄 */
    nvs_handle wificfg_nvs_handler1;
    /* 打开一个NVS命名空间 */
    ESP_ERROR_CHECK( nvs_open("WiFi_cfg", NVS_READWRITE, &wificfg_nvs_handler1) );

    uint32_t wifi_update = 0;
    err = nvs_get_u32(wificfg_nvs_handler1,"wifi_update",&wifi_update);
    if(MY_WIFI_UPDATE == wifi_update )
        ESP_LOGI(TAG,"wifi_cfg needn't to update. \n");
    else
    {
        ESP_LOGI(TAG,"wifi_cfg update now... \n");
        ESP_ERROR_CHECK( nvs_set_str(wificfg_nvs_handler1,"wifi_ssid",MY_WIFI_SSID) );
        ESP_ERROR_CHECK( nvs_set_str(wificfg_nvs_handler1,"wifi_passwd",MY_WIFI_PASSWD) ); 
        ESP_ERROR_CHECK( nvs_set_u32(wificfg_nvs_handler1,"wifi_update",MY_WIFI_UPDATE) );
        ESP_LOGI(TAG,"wifi_cfg update ok. \n");
    }
    ESP_ERROR_CHECK( nvs_commit(wificfg_nvs_handler1) ); /* 提交 */
    nvs_close(wificfg_nvs_handler1);                     /* 关闭 */

	s_wifi_event_group = xEventGroupCreate();  //创建WIFI连接事件组
	ESP_ERROR_CHECK(esp_event_loop_create_default());//默认事件循环
	esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta(); //创建默认wifi站点--STA
    assert(sta_netif);  //断言  为假终止程序
	// wifi设置:默认设置，等待SmartConfig配置
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
	// 注册wifi事件  分别是WIFI事件，IP地址事件，smartconfig事件
	ESP_ERROR_CHECK( esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL) );
	ESP_ERROR_CHECK( esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL) );
	ESP_ERROR_CHECK( esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL) );

    //取出NVS中的WIFI账号和密码--取出NVS中的WIFI账号和密码--取出NVS中的WIFI账号和密码
    nvs_handle wificfg_nvs_handler; /* 定义一个NVS操作句柄 */
    char wifi_ssid[32] = { 0 };     /* 定义一个数组用来存储ssid*/
    char wifi_passwd[65] = { 0 };   /* 定义一个数组用来存储passwd */
    size_t len;
    /* 打开一个NVS命名空间 */
    ESP_ERROR_CHECK( nvs_open("WiFi_cfg", NVS_READWRITE, &wificfg_nvs_handler) );
    len = sizeof(wifi_ssid);    /* 从NVS中获取ssid */
    ESP_ERROR_CHECK( nvs_get_str(wificfg_nvs_handler,"wifi_ssid",wifi_ssid,&len) );
    len = sizeof(wifi_passwd);      /* 从NVS中获取passwd */
    ESP_ERROR_CHECK( nvs_get_str(wificfg_nvs_handler,"wifi_passwd",wifi_passwd,&len) );
    ESP_ERROR_CHECK( nvs_commit(wificfg_nvs_handler) ); /* 提交 */
    nvs_close(wificfg_nvs_handler);                     /* 关闭 */
    /* 设置WiFi连接的ssid和password参数 */
    wifi_config_t wifi_config;
    bzero(&wifi_config, sizeof(wifi_config_t)); /* 将结构体数据清零 */
    memcpy(wifi_config.sta.ssid, wifi_ssid, sizeof(wifi_config.sta.ssid));
    memcpy(wifi_config.sta.password, wifi_passwd, sizeof(wifi_config.sta.password));

	// 设置sta模式
	ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
	/* 设置WiFi连接的参数，主要是ssid和password */
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
	// 启动wifi
	ESP_ERROR_CHECK( esp_wifi_start() );
}


