#include "window.h"

window_t::window_t() {
    set_default_size(1080, 720);

    cv_video_capture.open(0);
    cv_video_capture.set(CV_CAP_PROP_FRAME_WIDTH, 1080);
    cv_video_capture.set(CV_CAP_PROP_FRAME_HEIGHT, 720);

    /* add OpenCV view to main window*/
    add(cv_view);
    show_all();

    /* listen for camera in a separate thread*/
    capture_thread = std::thread([&] {
        cv::Mat frame;
        is_cam_closed = false;
        while (cv_video_capture.isOpened() && !is_cam_closed) {
            if (cv_video_capture.read(frame)) {
                capture_mutex.lock();
                        frame_from_video_capture_thread = frame.clone();
                        capture_mutex.unlock();
                        signal_new_frame_ready.emit();
            }
        }
    });

    /* copy camera mat in thread*/
    signal_new_frame_ready.connect([&] {
        capture_mutex.lock();
        camMat = frame_from_video_capture_thread.clone();
        capture_mutex.unlock();

        cv_view.set_cv_mat(camMat);
    });
}

window_t::~window_t() {
    is_cam_closed = true;
    if (capture_thread.joinable())
        capture_thread.join();
    cv_video_capture.release();
}
