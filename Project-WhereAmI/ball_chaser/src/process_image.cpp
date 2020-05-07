#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    ROS_INFO_STREAM("Driving Robot");

    // Request a service and pass the velocities to it to drive the robot
    ball_chaser::DriveToTarget srv;
    srv.request.linear_x = lin_x;
    srv.request.angular_z = ang_z;

    if(!client.call(srv))
        ROS_ERROR("Failed to call service drive_robot");
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{

    int white_pixel = 255;
    int pix_step = -1;
    // Loop through each pixel in the image and check if there's a bright white one

    for (int i=0; i<img.height * img.step; i+=3){
        if(img.data[i] == white_pixel && img.data[i+1] == white_pixel && img.data[i+2] == white_pixel){
	    pix_step = i % img.step;
	    break;
	}
    } 
    // Then, identify if this pixel falls in the left, mid, or right side of the image
    int img_pos = 10;
    if (pix_step <= img.step *0.3 && pix_step >=0){
	img_pos = 0; // Left
    }
    else if (pix_step > img.step *0.3 && pix_step < img.step *0.7){
	img_pos = 1; // Mid
    }
    else if (pix_step > img.step *0.7){
	img_pos = 2; // Right
    }

    // Depending on the white ball position, call the drive_bot function and pass velocities to it
    // Request a stop when there's no white ball seen by the camera
    float lin_x = 0.0;
    float ang_z = 0.0;
    switch(img_pos){
        case 0: ang_z = 0.5;
	    break;
	case 1: lin_x = 0.5;
	    break;
	case 2: ang_z = -0.5;
	    break;
	default: lin_x = 0.0;
	    ang_z = 0.0;
	    break;
    }
    drive_robot(lin_x,ang_z);
}

int main(int argc, char** argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // Handle ROS communication events
    ros::spin();

    return 0;
}
