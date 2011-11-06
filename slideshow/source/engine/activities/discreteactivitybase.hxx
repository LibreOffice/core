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



#ifndef INCLUDED_SLIDESHOW_DISCRETEACTIVITYBASE_HXX
#define INCLUDED_SLIDESHOW_DISCRETEACTIVITYBASE_HXX

#include "activitybase.hxx"
#include "wakeupevent.hxx"

#include <vector>


namespace slideshow
{
    namespace internal
    {
        /** Specialization of ActivityBase for discrete time activities.

            A discrete time activity is one where time proceeds in
            discrete steps, i.e. at given time instants.
        */
        class DiscreteActivityBase : public ActivityBase
        {
        public:
            DiscreteActivityBase( const ActivityParameters& rParms );

            /** Hook for derived classes.

                This method is called for each discrete time
                instant, with nFrame denoting the frame number
                (starting with 0)

                @param nFrame
                Current frame number.

                @param nRepeatCount
                Number of full repeats already performed
            */
            virtual void perform( sal_uInt32 nFrame, sal_uInt32 nRepeatCount ) const = 0;
            virtual void dispose();
            virtual bool perform();

        protected:
            virtual void startAnimation();

            sal_uInt32 calcFrameIndex( sal_uInt32       nCurrCalls,
                                       ::std::size_t    nVectorSize ) const;

            sal_uInt32 calcRepeatCount( sal_uInt32      nCurrCalls,
                                        ::std::size_t   nVectorSize ) const;

            ::std::size_t getNumberOfKeyTimes() const { return maDiscreteTimes.size(); }

        private:
            WakeupEventSharedPtr            mpWakeupEvent;
            const ::std::vector< double >   maDiscreteTimes;
            const double                    mnSimpleDuration;
            sal_uInt32                      mnCurrPerformCalls;
        };
    }
}

#endif /* INCLUDED_SLIDESHOW_DISCRETEACTIVITYBASE_HXX */
