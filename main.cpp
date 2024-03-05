#include <opencv2/opencv.hpp>
#include <iostream>

// Define your custom memory block structure
struct MemBlock {
    void* ptr;
    size_t size;
};

// Define your custom memory allocator class
class CustomAllocator {
private:
    void* baseAddress;

public:
    CustomAllocator(void* baseAddr) : baseAddress(baseAddr) {}

    MemBlock* allocateMemory(size_t size) {
        MemBlock* block = new MemBlock;
        if (block) {
            block->ptr = static_cast<char*>(baseAddress) + size; // Use the custom base address
            block->size = size;
        }
        return block;
    }
};

void detectAndDrawMovingObjects(cv::VideoCapture& cap, CustomAllocator& allocator) {
    cv::Ptr<cv::BackgroundSubtractor> pMOG2 = cv::createBackgroundSubtractorMOG2();

    cv::Mat frame, fgMask, fgMaskFiltered;

    while (true) {
        cap >> frame;
        if (frame.empty())
            break;

        // Allocate memory using the custom allocator
        size_t imageSize = frame.total() * frame.elemSize();
        MemBlock* imageBlock = allocator.allocateMemory(imageSize);

        if (imageBlock) {
            // Process video using the allocated memory
            std::cout << "Memory allocated successfully at address: " << imageBlock->ptr << std::endl;

            // You can use imageBlock->ptr for image processing

            // Apply background subtraction
            pMOG2->apply(frame, fgMask);

            // Threshold the foreground mask to get binary image
            cv::threshold(fgMask, fgMask, 128, 255, cv::THRESH_BINARY);

            // Apply morphological operations to reduce noise
            cv::morphologyEx(fgMask, fgMaskFiltered, cv::MORPH_OPEN, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)));
            cv::morphologyEx(fgMaskFiltered, fgMaskFiltered, cv::MORPH_CLOSE, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)));

            // Find contours in the binary image
            std::vector<std::vector<cv::Point>> contours;
            cv::findContours(fgMaskFiltered, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

            // Draw a red rectangle around each moving object
            for (const auto& contour : contours) {
                cv::Rect boundingBox = cv::boundingRect(contour);
                cv::rectangle(frame, boundingBox, cv::Scalar(0, 0, 255), 2);  // Red rectangle
            }

            // Display the resulting video with red rectangles
            cv::imshow("Moving Objects", frame);

            // Release the allocated memory
            delete imageBlock;

        } else {
            std::cout << "Memory allocation failed!" << std::endl;
        }

        // Break the loop if 'ESC' key is pressed
        if (cv::waitKey(30) == 27)
            break;
    }
}

int main() {
    void* baseMemoryAddress;

    // Prompt the user to enter the base address of memory
    std::cout << "Enter the base address of memory: ";
    std::string userInput;
    std::getline(std::cin, userInput);

    // Convert the user input to a void pointer
    baseMemoryAddress = reinterpret_cast<void*>(std::strtoull(userInput.c_str(), nullptr, 0));

    // Prompt the user to enter the video location
    std::cout << "Enter the video location: ";
    std::string videoLocation;
    std::getline(std::cin, videoLocation);

    // Modified string
    std::string modifiedvideoLocation;

    // Loop through the original string and add escape sequences for backslashes
    for (char c : videoLocation) {
        if (c == '\\') {
            modifiedvideoLocation += "\\\\";
        } else {
            modifiedvideoLocation += c;
        }
    }

    // Print the modified string
    std::cout << "Modified String: " << modifiedvideoLocation << std::endl;

    cv::VideoCapture cap(modifiedvideoLocation);
    if (!cap.isOpened()) {
        std::cerr << "Error opening video file." << std::endl;
        return -1;
    }

    CustomAllocator allocator(baseMemoryAddress);
    detectAndDrawMovingObjects(cap, allocator);

    cap.release();
    cv::destroyAllWindows();

    return 0;
}
