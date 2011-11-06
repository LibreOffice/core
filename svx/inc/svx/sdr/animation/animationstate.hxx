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



#ifndef _SDR_ANIMATION_ANIMATIONSTATE_HXX
#define _SDR_ANIMATION_ANIMATIONSTATE_HXX

#include <sal/types.h>
#include <vector>
#include <svx/sdr/animation/scheduler.hxx>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////
// predeclarations

namespace sdr
{
    namespace contact
    {
        class ViewObjectContact;
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace animation
    {
        class PrimitiveAnimation : public Event
        {
        protected:
            // the animated VOC
            sdr::contact::ViewObjectContact&                    mrVOContact;

            // the vetor of animated primitives
            drawinglayer::primitive2d::Primitive2DSequence      maAnimatedPrimitives;

            // local helpers
            double getSmallestNextTime(double fCurrentTime);
            void prepareNextEvent();

        public:
            // basic constructor.
            PrimitiveAnimation(sdr::contact::ViewObjectContact& rVOContact, const drawinglayer::primitive2d::Primitive2DSequence& rAnimatedPrimitives);

            // destructor
            virtual ~PrimitiveAnimation();

            // execute event, from base class Event
            virtual void Trigger(sal_uInt32 nTime);
        };
    } // end of namespace animation
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_ANIMATION_ANIMATIONSTATE_HXX

// eof
