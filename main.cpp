#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>
#include <mutex>
#include <thread>
#include <unistd.h>
#include <string>
#include <istream>

using std::cout;
using std::cin;
using std::endl;
using std::this_thread::sleep_for;
using std::string;

//std::mutex mtx_; // 排他制御用ミューテックス
string keyin;
bool exit_ = false;

void ThreadA()
{
    cin >> keyin;
    if(keyin == "q"){
        //std::lock_guard<std::mutex> lock(mtx_);
        exit_ = true;
    }
}

int main(int argc, char *argv[])
{
    
	cv::VideoCapture gstreamer;
	//gst-launchと同じコマンドを実行することができる。
	//  gstreamer.open("videotestsrc  ! appsink");                                 //カラーバー
	//	gstreamer.open("videotestsrc pattern=ball ! appsink");      //ボール
	//	gstreamer.open("videotestsrc pattern=snow ! appsink");   //じゃみじゃみ
    // gstreamer.open("v4l2src \
    // ! device=/dev/video0 \
    // ! image/jpeg,width=1280,height=720,framerate=30/1 \
    // ! jpegdec \
    // ! appsink");

    gstreamer.open("v4l2src device=/dev/video0 \
    ! image/jpeg,width=640,height=360,framerate=30/1 \
    ! jpegdec \
    ! videoconvert \
    ! appsink");

    // gstreamer.open("v4l2src device=/dev/video0 \
    // ! video/x-raw,width=1280,height=720,framerate=10/1 \
    // ! appsink");

	if (!gstreamer.isOpened()) 
	{
		printf("=ERR= fail to open\n");
		return -1;
	}

    int    width, height, fourcc; // 作成する動画ファイルの設定
    // fourcc = cv::VideoWriter::fourcc('m', 'p', '4', 'v'); // ビデオフォーマットの指定( ISO MPEG-4 / .mp4)
    fourcc = cv::VideoWriter::fourcc('V', 'P', '8', '0'); // ビデオフォーマットの指定( VP8 / .webm)
    double fps;
    width  = (int)gstreamer.get(cv::CAP_PROP_FRAME_WIDTH);	// フレーム横幅を取得
    height = (int)gstreamer.get(cv::CAP_PROP_FRAME_HEIGHT);	// フレーム縦幅を取得
    fps    = 10;				// フレームレートを取得

    cv::VideoWriter writer; // 動画ファイルを書き出すためのオブジェクトを宣言する
	// if(!writer.open("Video.mp4", fourcc, fps, cv::Size(width, height))){
    //if(!writer.open("appsrc ! videoconvert ! vp8enc ! filesink location=Video.webm", 
    if(!writer.open("Video.webm", 
    fourcc, fps, cv::Size(width, height))){
        std::cout << "writer.open() filed." <<endl;
        return -1;
    }

    std::thread th_a(ThreadA);  // 終了待ちスレッド
    std::cout << "exit 'q' & Enter." <<endl;

    auto start = std::chrono::system_clock::now();
    int64_t rem = 0;

	while (1)
	{
        auto end = std::chrono::system_clock::now();       // 計測終了時刻を保存
        auto dur = end - start;        // 要した時間を計算
        auto msec = std::chrono::duration_cast<std::chrono::microseconds>(dur).count();
        
        cv::Mat GstCap;
        gstreamer >> GstCap;
        cv::imshow("Gstreamer test", GstCap);

        if(msec + rem >= 100000){
            rem = msec - 100000;

            writer << GstCap;
            start = std::chrono::system_clock::now();
            std::cout << "loop." <<endl;
            
        }

        int key = cv::waitKey(1);
		
        if(exit_){
            writer.release();
            break;
        }
	}

    th_a.join();

	return 0;
}