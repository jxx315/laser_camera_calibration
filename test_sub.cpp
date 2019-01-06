#include "ros/ros.h"
#include "std_msgs/String.h"

#include <opencv-3.3.1-dev/opencv2/core.hpp>
#include <opencv-3.3.1-dev/opencv2/highgui.hpp>
#include <opencv-3.3.1-dev/opencv2/imgproc.hpp>
#include <opencv-3.3.1-dev/opencv2/opencv.hpp>
#include <rosbag/bag.h>
#include <rosbag/view.h>
// #include <std_msgs/Int32.h>
// #include <std_msgs/String.h>
#include <boost/foreach.hpp>
#include <sensor_msgs/Image.h>
#include <sensor_msgs/PointCloud2.h>
#include <sensor_msgs/image_encodings.h>
#include <sensor_msgs/image_encodings.h>

#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h> 
#define foreach BOOST_FOREACH


using namespace cv;

cv::Mat img_ ;

cv::Mat img_out;

sensor_msgs::Image  img_msg;   //最终写入bag的
void image_process(cv::Mat img) ;

static const std::string INPUT = "Input"; //定义输入窗口名称
static const std::string OUTPUT = "Output"; //定义输出窗口名称
//sensor_msgs::image  img_test;
/**
 * This tutorial demonstrates simple receipt of messages over the ROS system.
 */
// void chatterCallback(const std_msgs::String::ConstPtr& msg)
// {
// 	ROS_INFO("I heard: [%s]", msg->data.c_str());
// }
void grabImg(const sensor_msgs::ImageConstPtr& img)
{
    // Copy the ros image message to cv::Mat.
    cv_bridge::CvImageConstPtr cv_ptr;
    try
    {
        cv_ptr = cv_bridge::toCvShare(img,"bgr8");
    }
    catch (cv_bridge::Exception& e)
    {
        ROS_ERROR("cv_bridge exception: %s", e.what());
        return;
    }
        //Here we get the cv::Mat message ,that is cv_ptr->image
      cv_ptr->image.copyTo(img_);  
	  

	  image_process(img_);

      // img_out 转为 msgs ,等待写入bag
	  cv_bridge::CvImage img_bridge;
	  std_msgs::Header header; 
	  header = cv_ptr->header;
	  img_bridge = cv_bridge::CvImage(header, sensor_msgs::image_encodings::MONO8, img_out);
      img_bridge.toImageMsg(img_msg); // from cv_bridge to sensor_msgs::Image


	  
}


 void image_process(cv::Mat img) 
{
       
       cv::cvtColor(img, img_out, CV_RGB2GRAY);  //转换成灰度图象
	   //-----------------------------------去畸变----------------------------------------------
	//    Mat cameraMatrix = Mat::eye(3, 3, CV_64F);
	// 	//内参矩阵, 就算复制代码，也不要用我的参数。摄像头都不一样...
	// 	cameraMatrix.at<double>(0, 0) = 2.075319928848340e+03;
	// 	cameraMatrix.at<double>(0, 1) = 0;
	// 	cameraMatrix.at<double>(0, 2) = 1.273350420020438e+03;
	// 	cameraMatrix.at<double>(1, 1) = 2.073416138777327e+03;
	// 	cameraMatrix.at<double>(1, 2) = 6.898818022397845e+02;
	// 	//畸变参数，不要用我的参数~
	// 	Mat distCoeffs = Mat::zeros(5, 1, CV_64F);
	// 	distCoeffs.at<double>(0, 0) = 0.005961692158170;
	// 	distCoeffs.at<double>(1, 0) = 0.014892085926656;
	// 	distCoeffs.at<double>(2, 0) = -0.007714274148788;
	// 	distCoeffs.at<double>(3, 0) = -0.010548832296675;
	// 	distCoeffs.at<double>(4, 0) = 0;
	// 	Mat view, rview, map1, map2;
	// 	Size imageSize;
	// 	imageSize = img.size();
	// 	initUndistortRectifyMap(cameraMatrix, distCoeffs, Mat(),
	//                            getOptimalNewCameraMatrix(cameraMatrix, distCoeffs, imageSize, 1, imageSize, 0),
	// 							imageSize, CV_16SC2, map1, map2);
	// 	remap(img, img_out, map1, map2, INTER_LINEAR);
	   //---------------------------------------------------------------------------------
       cv::imshow(INPUT, img);
       cv::imshow(OUTPUT, img_out);
       cv::waitKey(1);
 }

int main(int argc, char *argv[])
{
	/**
	 * The ros::init() function needs to see argc and argv so that it can perform
	 * any ROS arguments and name remapping that were provided at the command line.
	 * For programmatic remappings you can use a different version of init() which takes
	 * remappings directly, but for most command-line programs, passing argc and argv is
	 * the easiest way to do it.  The third argument to init() is the name of the node.
	 *
	 * You must call one of the versions of ros::init() before using any other
	 * part of the ROS system.
	 */
	ros::init(argc, argv, "test");

	/**
	 * NodeHandle is the main access point to communications with the ROS system.
	 * The first NodeHandle constructed will fully initialize this node, and the last
	 * NodeHandle destructed will close down the node.
	 */
	ros::NodeHandle n;
    
    //ros::Publisher chatter_pub = n.advertise<sensor_msgs::image>("image_test", 1000);
	//ros::Rate loop_rate(10);
	

	//**********************************************************************************


    
    rosbag::Bag bag;
    bag.open("/home/jxx/Downloads/out_1.bag", rosbag::bagmode::Read);

	rosbag::Bag bag_out("out_1_fixed.bag", rosbag::bagmode::Write);
   
    std::vector<std::string> topics;
    topics.push_back(std::string("/usb_cam/image_raw"));
    topics.push_back(std::string("/velodyne_points"));
   
    rosbag::View view(bag, rosbag::TopicQuery(topics));
   
  
    foreach(rosbag::MessageInstance const m, view)
    {
         sensor_msgs::Image::ConstPtr s = m.instantiate<sensor_msgs::Image>();
           if (s != NULL)
           {
			 std::cout << "get image" << std::endl;
			 grabImg(s);
			 
			bag_out.write("/usb_cam/image_raw2", m.getTime(), img_msg);
			//bag_out.write("/usb_cam/image_raw2", ros::Time::now(), img_msg);
			
		
		   }    
			  
   
           sensor_msgs::PointCloud2::ConstPtr i = m.instantiate<sensor_msgs::PointCloud2>();
		   //ros::Time t2 = m.getTime();
           if (i != NULL)
		   {
				std::cout <<"get pc"<<std::endl;

				bag_out.write("/velodyne_points2", m.getTime(), i);
				//bag_out.write("/velodyne_points2", ros::Time::now(), i);
		   }
              
			   
    }
   
    bag.close();
	//***********************************************************************************
	ros::spin();

	return 0;
}