#include <stdio.h>
#include <linux/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>


#define I2C_DEV_NAME "/dev/i2c-4"
int fd;
//IIC地址 寄存器地址 数据指针
int func_write_regs(unsigned char devaddr, unsigned char reg, unsigned char *buf)
{
    struct i2c_rdwr_ioctl_data work_queue;
    int ret;
    int data_len = 1; // 写入数据的长度

    work_queue.nmsgs = 1;
    work_queue.msgs = (struct i2c_msg *)malloc(sizeof(struct i2c_msg));
    if (!work_queue.msgs) {
        printf("memory alloc failed (msgs)\n");
        return -1;
    }

    work_queue.msgs[0].addr  = devaddr;
    work_queue.msgs[0].flags = 0; // 写操作
    work_queue.msgs[0].len   = data_len + 1;
    work_queue.msgs[0].buf   = (unsigned char *)malloc(data_len + 1);
    if (!work_queue.msgs[0].buf) {
        printf("memory alloc failed (buf)\n");
        free(work_queue.msgs);
        return -1;
    }

    work_queue.msgs[0].buf[0] = reg;
    memcpy(&work_queue.msgs[0].buf[1], buf, data_len);

    ret = ioctl(fd, I2C_RDWR, &work_queue);
    if (ret < 0) {
        perror("ioctl I2C_RDWR failed");
    }

    free(work_queue.msgs[0].buf);
    free(work_queue.msgs);
    return ret;
}

int func_read_regs(unsigned char devaddr,unsigned char reg, unsigned char *read_data)
{
	struct i2c_rdwr_ioctl_data work_queue;
	int ret;
	work_queue.nmsgs = 1;
	/* 消息数量 */
	work_queue.msgs = (struct i2c_msg *)malloc(work_queue.nmsgs * sizeof(struct i2c_msg));
	if (!work_queue.msgs) {
		printf("Memory alloc error\n");
		close(fd);
		return -1;
	} 
	(work_queue.msgs[0]).len = 1;
	(work_queue.msgs[0]).flags = 0; //写数据的标志位
	(work_queue.msgs[0]).addr = devaddr;
	(work_queue.msgs[0]).buf = (unsigned char *)malloc(1);
	(work_queue.msgs[0]).buf[0] = reg;
	ret = ioctl(fd, I2C_RDWR, (unsigned long)&work_queue);
	if (ret < 0) {
		printf("error during I2C_RDWR ioctl with error code %d\n", ret);
		return ret;
	} 
	(work_queue.msgs[0]).len = 1;
	(work_queue.msgs[0]).flags = 1; //读数据的标志位
	(work_queue.msgs[0]).addr = devaddr;
	ret = ioctl(fd, I2C_RDWR, (unsigned long) &work_queue);
	if (ret < 0) {
		printf("Error during I2C_RDWR ioctl with error code: %d\n", ret);
		return ret;
	} else {
	// printf("read val:%02x\n", (work_queue.msgs[0]).buf[0]);
		*read_data=work_queue.msgs[0].buf[0];
	}
	free(work_queue.msgs[0].buf);
	free(work_queue.msgs);
	return ret;
}
int main(){
	int ret;
	char write_value=0x00;
	char read_value;
	fd = open(I2C_DEV_NAME, O_RDWR);
	printf("%d\r\n",fd);
	if (fd < 0) {
		perror(I2C_DEV_NAME);
		printf("can not open i2c device %s\n", I2C_DEV_NAME);
		exit(0);
	}
	ret = func_read_regs(0x68, 0x75, & read_value);
	if (ret < 0) {
		printf("read error! \n");
		close(fd);
		return ret;
	} 
	printf("read val:%02x\r\n", read_value);
	usleep(100000);
	ret = func_write_regs(0x68, 0x80, & write_value);
	if (ret < 0) {
		printf("write error! \n");
		close(fd);
		return ret;
	}
	close(fd);
	return 0;
}


