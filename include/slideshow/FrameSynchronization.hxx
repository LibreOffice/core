//
// Created by silver on 02.05.21.
//
#pragma once
#include <canvas/elapsedtime.hxx>
class FrameSynchronization
{
public:
    /** Create new object with a predefined duration between two frames.
        @param nFrameDuration
            The preferred duration between the display of two frames in
            seconds.
    */
    explicit FrameSynchronization (const double nFrameDuration);

    /** Set the current time as the time at which the current frame is
        displayed.  From this the target time of the next frame is derived.
    */
    void MarkCurrentFrame();

    /** When there is time left until the next frame is due then wait.
        Otherwise return without delay.
    */
    void Synchronize();

    /** Activate frame synchronization when an animation is active and
        frames are to be displayed in a steady rate.  While active
        Synchronize() will wait until the frame duration time has passed.
    */
    void Activate();

    /** Deactivate frame synchronization when no animation is active and the
        time between frames depends on user actions and other external
        sources.  While deactivated Synchronize() will return without delay.
    */
    void Deactivate();

private:
    /** The timer that is used for synchronization is independent from the
        one used by SlideShowImpl: it is not paused or modified by
        animations.
    */
    canvas::tools::ElapsedTime maTimer;
    /** Time between the display of frames.  Enforced only when mbIsActive
        is <TRUE/>.
    */
    const double mnFrameDuration;
    /** Time (of maTimer) when the next frame shall be displayed.
        Synchronize() will wait until this time.
    */
    double mnNextFrameTargetTime;
    /** Synchronize() will wait only when this flag is <TRUE/>.  Otherwise
        it returns immediately.
    */
    bool mbIsActive;
};