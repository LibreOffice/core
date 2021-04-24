//
// Created by silver on 02.05.21.
//

#include <slideshow/FrameSynchronization.hxx>
#include <osl/thread.hxx>
#include <chrono>

FrameSynchronization::FrameSynchronization (const double nFrameDuration)
    : maTimer(),
      mnFrameDuration(nFrameDuration),
      mnNextFrameTargetTime(0),
      mbIsActive(false)
{
    MarkCurrentFrame();
}

void FrameSynchronization::MarkCurrentFrame()
{
    mnNextFrameTargetTime = maTimer.getElapsedTime() + mnFrameDuration;
}

void FrameSynchronization::Synchronize()
{
    if (mbIsActive)
    {
        // Do busy waiting for now.
        for(;;)
        {
            double remainingTime = mnNextFrameTargetTime - maTimer.getElapsedTime();
            if(remainingTime <= 0)
                break;
            // Try to sleep most of it.
            int remainingMilliseconds = remainingTime * 1000;
            if(remainingMilliseconds > 2)
                osl::Thread::wait(std::chrono::milliseconds(remainingMilliseconds - 2));
        }
    }

    MarkCurrentFrame();
}

void FrameSynchronization::Activate()
{
    mbIsActive = true;
}

void FrameSynchronization::Deactivate()
{
    mbIsActive = false;
}