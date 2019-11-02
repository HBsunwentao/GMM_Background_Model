/*
简单的高斯背景模型
作者：厦门大学2019级硕士 孙文涛
Github网址：
翻版必究

A SIMPLE GAUSSION BACKGROUND MODEL TO DETECT THE FROEGROUND
Author:Wentao Sun
github:
*/
#include <stdio.h>
#include<stdlib.h>
#include<math.h>
#include<windows.h>
#include<string.h>
#define M 800*3		//Image's max rows
#define N 1000		//Image's max cols
#define GAUSS_NUM 4
typedef unsigned char BYTE;

typedef struct tagPICSIZE  {
    int width;//Image's width
    int height;//Image's height
    int length;//Image's total bytes
    int offset;//前三部分字节数
}PICSIZE;




//定义全局数组用于存储后续图像数据信息
BYTE PictureIn[M*N];//读文件的
BYTE PictureOut[M*N];//写入文件的
FILE *fInput;   //输入文件
FILE *fOutput;//输出文件
FILE *nameInput; // name file
FILE *nameOutput; //
BITMAPFILEHEADER FileHeader; //图像第一部分存储变量
BITMAPINFOHEADER InfoHeader;//图像第二部分存储变量
RGBQUAD Quad[256];//调色板存储的数组
PICSIZE PiSize;  //存储图像大小信息的结构体变量

// Define some variables for .Buling GMM model to 
// detect foreground and background. 
//This is one gaussion model
LEARNING_RATE =0.001;
INITIAL_SIGMA =30;
INITIAL_WEIGHT =0.05;
IMAGE_NUMBER = 287;

typedef struct onePixelGaussModel 
{
	int mean;
	int sigma;
	double weight;
}PICMODEL;

typedef struct onePixelGaussModel2 
{
	PICMODEL gauss[GAUSS_NUM];
	int num;
}PICMODEL2;

PICMODEL2 GaussModel[M*N];


	

int main()
{
	int i,j,k;
	char FileName[20];
	void ColorToGray();
	void ModelInit();
	int judge(int j,int k, int *dif,int pixel);
	void update1(int j,int k, int *dif , int pixel, int kk);
	void update2(int j, int k, int *dif ,int pixel);
	
	nameInput=fopen("names.txt","rb");
	nameOutput=fopen("outname.txt" ,"rb");
	for(i=0;i<IMAGE_NUMBER;i++)
	{
		// first step read the image from files
		fscanf(nameInput,"%s",FileName);
		fInput=fopen(FileName,"rb");
		fread(&FileHeader,sizeof(BITMAPFILEHEADER),1,fInput);
        fread(&InfoHeader,sizeof(BITMAPINFOHEADER),1,fInput);
		PiSize.width=InfoHeader.biWidth;
		PiSize.height=InfoHeader.biHeight;
		PiSize.offset=FileHeader.bfOffBits;
		if(InfoHeader.biBitCount==24)PiSize.width=PiSize.width*3;            //判断是否是真彩图，并读取第三部分调色板  Palette 存放在Quad数组中
			else fread(Quad,sizeof(RGBQUAD),pow(2,InfoHeader.biBitCount),fInput);
		PiSize.length=PiSize.width*PiSize.height;

		fseek(fInput,PiSize.offset,0);                                      //读取ImageDate位图数据
		fread(PictureIn,sizeof(BYTE),PiSize.length,fInput);
		
		ColorToGray();
		// second step to build the model
		if(i==0){ModelInit();}
		
		for(j=0;j<PiSize.height;j++)
		{
			for(k=0;k<PiSize.width/3;k++)
			{
				int pixel,dif[GAUSS_NUM];
				pixel =PictureOut[j*PiSize.width+k*3];
				
				if(judge(j,k,dif,pixel)==1)
				{
					PictureOut[j*PiSize.width+k*3]=0;
					PictureOut[j*PiSize.width+k*3+1]=0;
					PictureOut[j*PiSize.width+k*3+2]=0;
					
				}
				else
				{
					PictureOut[j*PiSize.width+k*3]=255;
					PictureOut[j*PiSize.width+k*3+1]=255;
					PictureOut[j*PiSize.width+k*3+2]=255;
					update2(j,k,dif,pixel);
				}
				
				gaussWeightRegu(j,k);
			}
		}
		fscanf(nameOutput,"%s",FileName);
		pOutput(FileName,PiSize.length);
		

	}
	
	

	
		return 0;
}

int judge(int j,int k, int *dif,int pixel)
{
	void update1(int j,int k, int *dif , int pixel, int kk);
	int flag=0;
	int num=0;
	int kk; 
	num =GaussModel[j*PiSize.width+k*3].num;
	for(kk=0;kk<num;kk++)
	{
		dif[kk]=abs(pixel-GaussModel[j*PiSize.width+k*3].gauss[kk].mean);
		if(dif[kk]<=(1.5*GaussModel[j*PiSize.width+k*3].gauss[kk].sigma))
		{
			update1(j,k,dif,pixel,kk);
			flag=1;
		}
	}
	return flag;
}
void update1(int j,int k, int *dif , int pixel, int kk)
{
	int num=0;
	int mean,sigma;
	double weight;
	num =GaussModel[j*PiSize.width+k*3].num;
	
	mean =GaussModel[j*PiSize.width+k*3].gauss[kk].mean;
	sigma=GaussModel[j*PiSize.width+k*3].gauss[kk].sigma;
	weight=GaussModel[j*PiSize.width+k*3].gauss[kk].weight;
	
	
	GaussModel[j*PiSize.width+k*3].gauss[kk].mean =(1-LEARNING_RATE)*mean+LEARNING_RATE*pixel;
	GaussModel[j*PiSize.width+k*3].gauss[kk].sigma=(1-LEARNING_RATE)*sigma+LEARNING_RATE*dif[kk];
	GaussModel[j*PiSize.width+k*3].gauss[kk].weight=weight+ LEARNING_RATE*(1-weight);
}

void update2(int j, int k, int *dif ,int pixel)
{
	int num=0,kk;
	int pos,sigma;
	double weight;
	float xiao;
	xiao=1000;
	pos =0;
	num =GaussModel[j*PiSize.width+k*3].num;
	
	if(num!=GAUSS_NUM)
	{
		GaussModel[j*PiSize.width+k*3].gauss[num].mean=pixel;
		GaussModel[j*PiSize.width+k*3].gauss[num].sigma=INITIAL_SIGMA;
		GaussModel[j*PiSize.width+k*3].gauss[num].weight=INITIAL_WEIGHT;
		GaussModel[j*PiSize.width+k*3].num =num+1;
	}
	else
	{
		for(kk=0;kk<num;kk++)
		{
			sigma=GaussModel[j*PiSize.width+k*3].gauss[kk].sigma;
			weight=GaussModel[j*PiSize.width+k*3].gauss[kk].weight;
			if(xiao>(1.0*weight/sigma))
			{
				xiao=1.0*weight/sigma;
				pos =kk;
			}
		}
		GaussModel[j*PiSize.width+k*3].gauss[pos].mean=pixel;
		GaussModel[j*PiSize.width+k*3].gauss[pos].sigma=INITIAL_SIGMA;
		GaussModel[j*PiSize.width+k*3].gauss[pos].weight=INITIAL_WEIGHT;
	}

}

void gaussWeightRegu(int j, int k)
{
	int num;
	int kk;
	double sum,weight;
	sum=0;
	num = GaussModel[j*PiSize.width+k*3].num;
	for(kk=0;kk<num;kk++)
	{
		sum=sum+GaussModel[j*PiSize.width+k*3].gauss[kk].weight;
	}
	
	for(kk=0;kk<num;kk++)
	{
		weight = GaussModel[j*PiSize.width+k*3].gauss[kk].weight;
		GaussModel[j*PiSize.width+k*3].gauss[kk].weight = weight / sum;
	}
}

void ModelInit()
{
	int j,k;
	int kk;
	for(j=0;j<PiSize.height;j++)
		{
			for(k=0;k<PiSize.width/3;k++)
			{
				GaussModel[j*PiSize.width+k*3].num =1;
				for(kk=0;kk<GAUSS_NUM;kk++)
				{
					GaussModel[j*PiSize.width+k*3].gauss[kk].mean =0;
					GaussModel[j*PiSize.width+k*3].gauss[kk].sigma=INITIAL_SIGMA;
					GaussModel[j*PiSize.width+k*3].gauss[kk].weight=INITIAL_WEIGHT;
				}
				
			}
		}
}


/*
pInput() 过程功能是读取指定位置的BMP图像文件
S1先读取 BITMAPFILEHEADER 和  BITMAPINFOHEADER 两部分
分别存放在FileHeader 和 InfoHeader 两个结构体变量里面
S2 算出图像色彩位数是否是24位真彩图， 并确定第四部分ImageDate位图数据的字节数
*/
void pInput()  //BMP图像的读入部分
{
    	char FileName[20];
	    //printf("please enter the file name\n");
	    //scanf("%s",FileName);
        //while((fInput=fopen(FileName,"rb"))==NULL)
       //{
        //   printf("sorry,we can not open this file,please Re Input\n");
        //   scanf("%s",FileName);
        //}
		

       fread(&FileHeader,sizeof(BITMAPFILEHEADER),1,fInput);
       fread(&InfoHeader,sizeof(BITMAPINFOHEADER),1,fInput);
	PiSize.width=InfoHeader.biWidth;
	PiSize.height=InfoHeader.biHeight;
	PiSize.offset=FileHeader.bfOffBits;
    if(InfoHeader.biBitCount==24)PiSize.width=PiSize.width*3;            //判断是否是真彩图，并读取第三部分调色板  Palette 存放在Quad数组中
        else fread(Quad,sizeof(RGBQUAD),pow(2,InfoHeader.biBitCount),fInput);
    PiSize.length=PiSize.width*PiSize.height;

    fseek(fInput,PiSize.offset,0);                                      //读取ImageDate位图数据
    fread(PictureIn,sizeof(BYTE),PiSize.length,fInput);
}









/*
生成变化完成的BMP格式图像文件
*/
void pOutput(char OutName[20],int length)
{
    while((fOutput=fopen(OutName,"wb"))==NULL)
    {
        printf("sorry we can not build a file,please check your computer");
        return;
    }
	fwrite(&FileHeader,sizeof(BITMAPFILEHEADER),1,fOutput);
	fwrite(&InfoHeader,sizeof(BITMAPINFOHEADER),1,fOutput);
	if(InfoHeader.biBitCount!=24)fwrite(Quad,sizeof(RGBQUAD),pow(2,InfoHeader.biBitCount),fOutput);
	fwrite(PictureOut,sizeof(BYTE),length,fOutput);
	fclose(fOutput);
	fclose(fInput);
	printf("result is in %s. It has been processed succesfully\n",OutName);
}




/*
灰度图变化
对真彩色：算每个像素点的灰度值求出，然后使RGB三色全部赋值为此灰度
对非真彩色图：直接修改对调色板RGB三色的数值为三者和的平均数
*/
void ColorToGray()
{
    void pOutput();
    int i,j;
    pInput();
    if(InfoHeader.biBitCount==24)
	for(j=0;j<PiSize.height;j++)
	{
	  for(i=0;i<PiSize.width/3;i++)
	  {
		int sum;//求每个像素点的灰度值
		sum=PictureIn[j*PiSize.width+i*3]*0.299+PictureIn[j*PiSize.width+i*3+1]*0.578+PictureIn[j*PiSize.width+i*3+2]*0.114;
	    PictureOut[j*PiSize.width+i*3]=sum;//将RGB三通道 全部赋值为 同一灰度值 实现照片变黑白
		PictureOut[j*PiSize.width+i*3+1]=sum;
		PictureOut[j*PiSize.width+i*3+2]=sum;
	  }
	}
	else
    {
        for(i=0;i<PiSize.length;i++)PictureOut[i]=PictureIn[i];
        BYTE sum;
        for(i=0;i<pow(2,InfoHeader.biBitCount);i++)//将调色板不同颜色转化为灰度
        {

           sum=(Quad[i].rgbBlue+Quad[i].rgbGreen+Quad[i].rgbRed)/3;
            Quad[i].rgbBlue=sum;
            Quad[i].rgbGreen=sum;
            Quad[i].rgbRed=sum;
        }
    }
    //pOutput("color to grey.bmp",PiSize.length);

}






