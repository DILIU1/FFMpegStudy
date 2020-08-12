#include "stdafx.h"
#include <fstream>


void SaveFrame(AVFrame *pFrame, int width, int height, int iFrame) {
	FILE *pFile;		//�ļ�ָ��
	char szFilename[32];//�ļ������ַ�����
	int y;				//

	sprintf(szFilename, "frame%04d.ppm", iFrame);	//�����ļ���
	pFile = fopen(szFilename, "wb");			//���ļ���ֻд��
	if (pFile == NULL) {
		return;
	}

	//getch();

	fprintf(pFile, "P6\n%d %d\n255\n", width, height);//���ĵ��м��룬������룬��ȻPPM�ļ��޷���ȡ

	for (y = 0; y < height; y++) {
		fwrite(pFrame->data[0] + y * pFrame->linesize[0], 1, width * 3, pFile);
	}

	fclose(pFile);

}


int _tmain(int argc, _TCHAR* argv[])
{

	char filepath[] = "nwn.mp4";

	//(1)����ע�������е��ļ���ʽ�ͱ�������Ŀ� �������ǽ����Զ���ʹ���ڱ��򿪵ĺ��ʸ�ʽ���ļ�  ֻ��Ҫע��һ��
	av_register_all();
	avformat_network_init();
	AVFormatContext *pFormatCtx;
	pFormatCtx = avformat_alloc_context();

	//(2)��һ���ļ�
	if (avformat_open_input(&pFormatCtx, filepath, NULL, NULL) != 0) 
	{
		return -1;
	}

	//(3)������ļ��е�������Ϣ
	if(avformat_find_stream_info(pFormatCtx,0)<0)
	{
		return -1;
	}

	//(4)dump����Ϣ
	av_dump_format(pFormatCtx,0, filepath,0);


	int i;
	AVCodecContext *pCodecCtx;

	int videoStream = -1;
	//pFormatCtx->Streams ������һ��pFormatCtx->nb_streams ��ָ��
	for(i=0; i<pFormatCtx->nb_streams;i++)
	{
		if(pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			videoStream = i;
			break;
		}
	}

	if(videoStream == -1)
	{
		return -1;
	}

	pCodecCtx = pFormatCtx->streams[videoStream]->codec;

	AVCodec *pCodec;


	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);

	if(pCodec == NULL)
	{
		fprintf(stderr, "Unsupported codec!\n");
		return -1;
	}

	if(avcodec_open(pCodecCtx,pCodec) < 0)
	{
		return -1;
	}

	AVFrame *pFrame;
	pFrame = avcodec_alloc_frame();

	AVFrame *pFrameRGB = avcodec_alloc_frame();
	if(pFrameRGB == NULL)
	{
		return -1;
	}
	uint8_t *buffer;
	int numBytes;
	numBytes = avpicture_get_size(PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);

	buffer = (uint8_t*)av_malloc(numBytes * sizeof(uint8_t));

	//������������������
	avpicture_fill((AVPicture*)pFrameRGB, buffer, PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);


	int frameFinished;
	AVPacket packet;
	i = 0;
	while(av_read_frame(pFormatCtx,&packet)>=0)
	{
		if(packet.stream_index == videoStream)
		{
			avcodec_decode_video2(pCodecCtx,pFrame,&frameFinished,&packet);
			if(frameFinished)
			{
				//�ɰ汾
				//img_convert((AVPicture *)pFrameRGB, PIX_FMT_RGB24,(AVPicture*)pFrame, pCodecCtx->pix_fmt,pCodecCtx->width, pCodecCtx->height);

				SwsContext *img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);
				sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize);

				if(++i <=5)
				{
					SaveFrame(pFrameRGB, pCodecCtx->width, pCodecCtx->height, i);
				}
			}
			av_free_packet(&packet);
		}
	}
	av_free(buffer);
	av_free(pFrame);
	av_free(pFrameRGB);

	avcodec_close(pCodecCtx);

	avformat_close_input(&pFormatCtx);


	return 0;
}

