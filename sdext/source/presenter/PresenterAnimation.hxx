/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef SDEXT_PRESENTER_ANIMATION_HXX
#define SDEXT_PRESENTER_ANIMATION_HXX

#include <sal/types.h>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>

namespace sdext { namespace presenter {

/** Base class for animations handled by a PresenterAnimator object.
    A PresenterAnimation objects basically states when it wants to be
    started, how long it runs, and in what steps it wants to be called back
    while running.
    When a PresenterAnimation object is active/running its Run() method is
    called back with increasing values between 0 and 1.
*/
class PresenterAnimation
    : private ::boost::noncopyable
{
public:
    /** Create a new PresenterAnimation object.
        @param nStartDelay
            The delay in ms (milliseconds) from this call until the new
            object is to be activated.
        @param nTotalDuration
            The duration in ms the Run() method is to be called with
            increasing values between 0 and 1.
        @param nStepDuration
            The duration between calls to Run().  This leads to approximately
            nTotalDuration/nStepDuration calls to Run().  The exact duration
            of each step may vary depending on system load an other influences.
    */
    PresenterAnimation (
        const sal_uInt64 nStartDelay,
        const sal_uInt64 nTotalDuration,
        const sal_uInt64 nStepDuration);
    virtual ~PresenterAnimation (void);

    /** Return the absolute start time in a system dependent format.
        At about this time the Run() method will be called with a value of 0.
    */
    sal_uInt64 GetStartTime (void);

    /** Return the absolute end time in a system dependent format.
        At about this time the Run() method will be called with a value of 1.
    */
    sal_uInt64 GetEndTime (void);

    /** Return the duration of each step in ms.
    */
    sal_uInt64 GetStepDuration (void);

    typedef ::boost::function<void(void)> Callback;

    /** Add a callback that is executed before Run() is called for the first
        time.
    */
    void AddStartCallback (const Callback& rCallback);

    /** Add a callback that is executed after Run() is called for the last
        time.
    */
    void AddEndCallback (const Callback& rCallback);

    /** Called with nProgress taking on values between 0 and 1.
        @param nProgress
            A value between 0 and 1.
        @param nCurrentTime
            Current time in a system dependent format.
    */
    virtual void Run (const double nProgress, const sal_uInt64 nCurrentTime) = 0;

    /** Called just before Run() is called for the first time to trigger the
        callbacks registered via the <method>AddStartCallback()</method>.
    */
    void RunStartCallbacks (void);

    /** Called just after Run() is called for the last time to trigger the
        callbacks registered via the <method>AddEndCallback()</method>.
    */
    void RunEndCallbacks (void);

private:
    const sal_uInt64 mnStartTime;
    const sal_uInt64 mnTotalDuration;
    const sal_uInt64 mnStepDuration;
    ::boost::scoped_ptr<std::vector<Callback> > mpStartCallbacks;
    ::boost::scoped_ptr<std::vector<Callback> > mpEndCallbacks;
};

sal_uInt64 GetCurrentTime (void);
inline sal_uInt32 GetSeconds (const sal_uInt64 nTime) { return sal_uInt32(nTime / 1000); }
inline sal_uInt32 GetNanoSeconds (const sal_uInt64 nTime) { return sal_uInt32((nTime % 1000) * 1000000); }

typedef ::boost::shared_ptr<PresenterAnimation> SharedPresenterAnimation;


} } // end of namespace ::sdext::presenter

#endif
