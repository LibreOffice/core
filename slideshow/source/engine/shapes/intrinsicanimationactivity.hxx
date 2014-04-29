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



#ifndef INCLUDED_SLIDESHOW_INTRINSICANIMATIONACTIVITY_HXX
#define INCLUDED_SLIDESHOW_INTRINSICANIMATIONACTIVITY_HXX

#include "wakeupevent.hxx"
#include "activity.hxx"
#include "slideshowcontext.hxx"
#include "drawshape.hxx"
#include "tools.hxx"

/* Definition of IntrinsicAnimationActivity class */

namespace slideshow
{
    namespace internal
    {
        /** Create an IntrinsicAnimationActivity.

            This is an Activity interface implementation for intrinsic
            shape animations. Intrinsic shape animations are
            animations directly within a shape, e.g. drawing layer
            animations, or GIF animations.

            @param rContext
            Common slideshow objects

            @param rDrawShape
            Shape to control the intrinsic animation for

            @param rWakeupEvent
            Externally generated wakeup event, to set this
            activity to sleep during inter-frame intervals. Must
            come from the outside, since wakeup event and this
            object have mutual references to each other.

            @param rTimeouts
            Vector of timeout values, to wait before the next
            frame is shown.
        */
        ActivitySharedPtr createIntrinsicAnimationActivity(
            const SlideShowContext&         rContext,
            const DrawShapeSharedPtr&       rDrawShape,
            const WakeupEventSharedPtr&     rWakeupEvent,
            const ::std::vector<double>&    rTimeouts,
            ::std::size_t                   nNumLoops,
            CycleMode                       eCycleMode );
    }
}

#endif /* INCLUDED_SLIDESHOW_INTRINSICANIMATIONACTIVITY_HXX */
