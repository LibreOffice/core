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



#ifndef SDEXT_PRESENTER_ANIMATOR_HXX
#define SDEXT_PRESENTER_ANIMATOR_HXX

#include "PresenterAnimation.hxx"
#include <cppuhelper/basemutex.hxx>
#include <map>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

namespace sdext { namespace presenter {

/** Simple animation management.  Call AddAnimation to run animations
    concurrently or one of the other.  See PresenterAnimation for details of
    how to specify animations.
*/
class PresenterAnimator
    : private ::boost::noncopyable,
      private ::cppu::BaseMutex
{
public:
    PresenterAnimator (void);
    virtual ~PresenterAnimator (void);

    /** Add an animation.  The time at which to start and end this animation
        is provided by the animation itself.
    */
    void AddAnimation (const SharedPresenterAnimation& rpAnimation);

private:
    typedef ::std::multimap<sal_uInt64,SharedPresenterAnimation> AnimationList;
    AnimationList maFutureAnimations;
    AnimationList maActiveAnimations;
    sal_Int32 mnCurrentTaskId;
    sal_uInt64 mnNextTime;

    void Process (void);
    void ActivateAnimations (const sal_uInt64 nCurrentTime);
    void ScheduleNextRun (void);
    void ScheduleNextRun (const sal_uInt64 nStartTime);

};

} } // end of namespace ::sdext::presenter

#endif
