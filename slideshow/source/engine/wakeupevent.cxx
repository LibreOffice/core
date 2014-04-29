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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_slideshow.hxx"

// must be first
#include <canvas/debug.hxx>
#include <canvas/verbosetrace.hxx>

#include <wakeupevent.hxx>


namespace slideshow
{
    namespace internal
    {
        WakeupEvent::WakeupEvent(
            boost::shared_ptr<canvas::tools::ElapsedTime> const & pTimeBase,
            ActivitiesQueue& rActivityQueue ) :
#if OSL_DEBUG_LEVEL > 1
            Event(::rtl::OUString::createFromAscii("WakeupEvent")),
#endif
            maTimer(pTimeBase),
            mnNextTime(0.0),
            mpActivity(),
            mrActivityQueue( rActivityQueue )
        {
        }

        void WakeupEvent::dispose()
        {
            mpActivity.reset();
        }

        bool WakeupEvent::fire()
        {
            if( !mpActivity )
                return false;

            return mrActivityQueue.addActivity( mpActivity );
        }

        bool WakeupEvent::isCharged() const
        {
            // this event won't expire, we fire every time we're
            // re-inserted into the event queue.
            return true;
        }

        double WakeupEvent::getActivationTime( double nCurrentTime ) const
        {
            const double nElapsedTime( maTimer.getElapsedTime() );

            return ::std::max( nCurrentTime,
                               nCurrentTime - nElapsedTime + mnNextTime );
        }

        void WakeupEvent::start()
        {
            // start timer
            maTimer.reset();
        }

        void WakeupEvent::setNextTimeout( double rNextTime )
        {
            mnNextTime = rNextTime;
        }

        void WakeupEvent::setActivity( const ActivitySharedPtr& rActivity )
        {
            mpActivity = rActivity;
        }
    }
}
