#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h> /*PPSIX 终端控制定义*/
#include "serial.h"
int fd;
unsigned char receive_buff[100];
unsigned int receive_num = 0;
unsigned char send_buff[100] = "test serial port";
unsigned int send_num = 0;
/*
* @description : 串口发送函数
* @param - fd : 文件描述符
* @param - *p_send_buff: 要发送数据缓冲区首地址
* @param - count: 要发送数据长度
* @return : 执行结果
*/
int func_send_frame(int fd, const unsigned char *p_send_buff, const int count)
{
	int Result = 0;
	Result = write(fd, p_send_buff, count);
	if (Result == -1) {
		perror("write");
		return 0;
	}
	return Result;
} 
/*
* @description : ESP8266配置函数
* @return : 执行结果
*/

int esp8266_command(int fd, unsigned char *command) {
    int send_num = func_send_frame(fd, command, strlen(command));
    if (send_num > 0) {
        printf("[nwrite=%d] %s\n", send_num, command); // 打印发送的数据
        return 0; // 成功
    } else {
        return -1; // 失败
    }
}

/*
* @description : ESP8266初始化函数
* @return : 执行结果
*/
int esp8266_init()
{
	int Result = 0;
    unsigned char *commands[] = {
        "AT+RST\r\n",
        "AT+CWMODE=1\r\n",
        "AT+CWJAP=\"test\",\"88888888\"\r\n",
        "AT+MQTTUSERCFG=0,1,\"esp8266_test\",\"\",\"\",0,0,\"\"\r\n",
        "AT+MQTTCONN=0,\"mqtt.eclipseprojects.io\",1883,0\r\n"
    };
	for (int i = 0; i < sizeof(commands) / sizeof(commands[0]); i++) {
        if (esp8266_command(fd, (unsigned char *)commands[i]) < 0) {
            Result = -1;
			printf("未发送数据-1\n"); //打印接收的数据
        }
		tcflush(fd, TCIFLUSH); //串口数据刷新
		receive_num = func_receive_frame(fd, receive_buff, sizeof(receive_buff));
		while(receive_num<=0){
			printf("未收到数据,卡在循环处\n"); 
			receive_num = func_receive_frame(fd, receive_buff, sizeof(receive_buff));
		}
		//读取串口收到的数据
		if (receive_num > 0) {
			printf("[nread=%d] ", receive_num);
			printf("%s\n", receive_buff); //打印接收的数据
		}
		sleep(10);
    }
	return Result;
}

/*
@description : 串口接收函数
* @param - fd : 文件描述符
* @param - *p_receive_buff: 接收数据缓冲区首地址
* @param - count: 最大接收数据长度
* @return : 执行结果
*/
int func_receive_frame(int fd, unsigned char *p_receive_buff, const int count)
{
	// 阻塞用法
	int nread = 0;
	fd_set rd;
	int retval = 0;
	struct timeval timeout = {0, 500};
	FD_ZERO(&rd);
	FD_SET(fd, &rd);
	memset(p_receive_buff, 0x0, count);
	retval = select(fd + 1, &rd, NULL, NULL, &timeout);
	switch(retval){
		case 0:
			nread = 0;
		break;
		case -1:
			printf("select%s\n", strerror(errno));
			nread = -1;
		break;
		default:
			nread = read(fd, p_receive_buff, count); //读串口
		break;
	}
	return nread;
} 
/*
@description : 主函数
* @return : 执行结果
*/
int main()
{
	int result = 0;
	struct termios newtio, oldtio;
	//打开串口 uart9 设置可读写， 不被输入影响， 不等待外设响应
	fd = open("/dev/ttyS9", O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd < 0) {
		perror("/dev/ttyS9");
		printf("Can't Open Serial Port %s \n", "/dev/ttyS9");
		exit(0);
	} 
	else { 
	//设置串口参数 oldtio保存旧参数
		if (tcgetattr(fd, &oldtio) != 0) {
		perror("tcgetattr");
		return -1;
	}
	bzero(&newtio, sizeof(newtio)); //清空串口设置
	newtio.c_cflag |= CLOCAL | CREAD; //打开接收标志和忽略控制线
	newtio.c_cflag &= ~CSIZE; //清除数据位设置
	newtio.c_cflag |= CS8; //设置数据位为 8 位
	newtio.c_cflag &= ~PARENB; //无校验位
	cfsetispeed(&newtio, B115200); //设置输入波特率为 9600
	cfsetospeed(&newtio, B115200); //设置输出波特率为 9600
	newtio.c_cflag &= ~CSTOPB; //设置停止位 1
	newtio.c_cc[VTIME] = 0; //不使用超时控制
	newtio.c_cc[VMIN] = 0; //不等待字符
	tcflush(fd, TCIFLUSH); //串口数据刷新
	//设置新参数
	if ((tcsetattr(fd, TCSANOW, &newtio)) != 0) {
		perror("com set error");
		return -1;
	}
	printf("set done!\n");
	}
	esp8266_init();
	while (1) {
		
		esp8266_command(fd,"AT+MQTTPUB=0,\"esp8266_xyx\",\"test666\",0,0\r\n");
		receive_num = func_receive_frame(fd, receive_buff, sizeof(receive_buff));
		while(receive_num<=0){
			printf("未收到数据,卡在循环处\n"); 
			receive_num = func_receive_frame(fd, receive_buff, sizeof(receive_buff));
		}
		//读取串口收到的数据
		if (receive_num > 0) {
			printf("[nread=%d] ", receive_num);
			printf("%s\n", receive_buff); //打印接收的数据
		}
		sleep(10);
		
	}
	close(fd);
	exit(0);
}

