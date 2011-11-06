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



#ifndef INCLUDED_SLIDESHOW_CONTINUOUSACTIVITYBASE_HXX
#define INCLUDED_SLIDESHOW_CONTINUOUSACTIVITYBASE_HXX

#include "simplecontinuousactivitybase.hxx"

namespace slideshow
{
    namespace internal
    {
        /** Simple, continuous animation.

            This class implements a simple, continuous
            animation. Only addition to ActivityBase class is an
            explicit animation duration and a minimal number of
            frames to display.
        */
        class ContinuousActivityBase : public SimpleContinuousActivityBase
        {
        public:
            ContinuousActivityBase( const ActivityParameters& rParms );

            using SimpleContinuousActivityBase::perform;

            /** Hook for derived classes

                This method will be called from perform(), already
                equipped with the modified time (nMinNumberOfFrames, repeat,
                acceleration and deceleration taken into account).

                @param nModifiedTime
                Already accelerated/decelerated and repeated time, always
                in the [0,1] range.

                @param nRepeatCount
                Number of full repeats already performed
            */
            virtual void perform( double nModifiedTime, sal_uInt32 nRepeatCount ) const = 0;

            /// From SimpleContinuousActivityBase class
            virtual void simplePerform( double      nSimpleTime,
                                        sal_uInt32  nRepeatCount ) const;
        };
    }
}

#endif /* INCLUDED_SLIDESHOW_CONTINUOUSACTIVITYBASE_HXX */
