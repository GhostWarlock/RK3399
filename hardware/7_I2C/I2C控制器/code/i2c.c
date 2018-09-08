#include "i2c.h"
#include "uart.h"
#include "printf.h"
#include "timer.h"


//GPIO1_B3/I2C4_SDA
//GPIO1_B4/I2C4_SCL

void i2c_init(void)
{
    //1.GPIO1_B3/I2C4_SDA��GPIO1_B4/I2C4_SCL����Ϊ��������,ע���λҪ����Ϊ1����д;
    PMUGRF_GPIO1B_IOMUX |= ((0xFFFF0000 << 0) | (0x01 << 6) | (0x01 << 8));

    //2.����SCKʱ��
    //3.ע��/ʹ���ж�
}


void eeprom_write(unsigned char addr, unsigned char data)
{
    //0.��տ��ƼĴ�����ʹ��
    I2C4->CON &= ~(0x7F << 0);
    I2C4->IPD &= ~(0x7F << 0);
    I2C4->CON |= 0x01 << 0; //ʹ��

    //1.����ģʽ:transmit only
    I2C4->CON &= ~(0x03 << 1);

    //2.��ʼ�ź�
    I2C4->CON |= 0x01 << 3; //��ʼ�ź�
    while(!(I2C4->IPD & (0x01 << 4))); //�ȴ���ʼ�źŷ���
    I2C4->IPD |=  (0x01 << 4); //�忪ʼ�źű�־

    //3.I2C�ӻ���ַ+д��ַ+���� (3���ֽ�)
    I2C4->TXDATA0 = 0xA0 | (addr << 8) | (data << 16);
    I2C4->MTXCNT = 3;
    while(!(I2C4->IPD & (0x01 << 2))); //MTXCNT data transmit finished interrupt pending bit
    I2C4->IPD |=  (0x01 << 2);

    //4.�����ź�
    I2C4->CON &= ~(0x01 << 3); //�ֶ����start(ע��:ǰ��Ŀ�ʼ�źſ���λ���ۻ��Զ���0,ʵ��û��,��������ֶ���,�����ǿ�ʼ�ź�)
    I2C4->CON |= (0x01 << 4);
    while(!(I2C4->IPD & (0x01 << 5)));
    I2C4->IPD |=  (0x01 << 5);
}

//�Զ����ʹӻ���ַ�ʹӻ��Ĵ�����ַ
unsigned char eeprom_read(unsigned char addr)
{
    unsigned char data = 0;

    //0.��տ��ƼĴ�����ʹ��
    I2C4->CON &= ~(0x7F << 0);
    I2C4->IPD &= ~(0x7F << 0);
    I2C4->CON |= 0x01 << 0; //ʹ��

    //�����յ�ack,����ֹͣ����(�Ǳ���)
    //I2C4->CON |=  (0x01<<6); //stop transaction when NAK handshake is received

    //1.����ģʽ:transmit address (device + register address) --> restart --> transmit address �C> receive only
    I2C4->CON |=  (0x01 << 1); //�Զ����ʹӻ���ַ�ʹӻ��Ĵ�����ַ

    //2.�ӻ���ַ
    I2C4->MRXADDR = (0xA0 | (1 << 24));

    //3.�ӻ��Ĵ�����ַ
    I2C4->MRXRADDR = (addr | (1 << 24)); //��ַֻ��6λ,����6λ��ô��?

    //4.��ʼ�ź�
    I2C4->CON |=  (0x01 << 3);
    while(!(I2C4->IPD & (0x01 << 4)));
    I2C4->IPD |=  (0x01 << 4);

    //5.����һ�������Ҳ���Ӧ
    I2C4->CON |= (0x01 << 5);
    I2C4->MRXCNT = 1;
    while(!(I2C4->IPD & (0x01 << 3)));
    I2C4->IPD |=  (0x01 << 3);

    //6.�����ź�
    I2C4->CON &= ~(0x01 << 3); //�ֶ����start
    I2C4->CON |= (0x01 << 4);
    while(!(I2C4->IPD & (0x01 << 5)));
    I2C4->IPD |=  (0x01 << 5);

    return (I2C4->RXDATA0 & 0xFF);
}

