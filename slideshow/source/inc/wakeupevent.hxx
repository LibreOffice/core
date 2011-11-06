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


#ifndef INCLUDED_SLIDESHOW_WAKEUPEVENT_HXX
#define INCLUDED_SLIDESHOW_WAKEUPEVENT_HXX

#include <canvas/elapsedtime.hxx>

#include "event.hxx"
#include "activitiesqueue.hxx"

#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>

namespace slideshow {
namespace internal {

/** Little helper class, used to set Activities active again
    after some sleep period.

    Clients can use this class to schedule wakeup events at
    the EventQueue, to avoid busy-waiting for the next
    discrete time instant.
*/
class WakeupEvent : public Event,
                    private ::boost::noncopyable
{
public:
    WakeupEvent(
        ::boost::shared_ptr< ::canvas::tools::ElapsedTime > const& pTimeBase,
        ActivitiesQueue & rActivityQueue );

    virtual void dispose();
    virtual bool fire();
    virtual bool isCharged() const;
    virtual double getActivationTime( double nCurrentTime ) const;

    /// Start the internal timer
    void start();

    /** Set the next timeout this object should generate.

        @param nextTime
        Absolute time, measured from the last start() call,
        when this event should wakeup the Activity again. If
        your time is relative, simply call start() just before
        every setNextTimeout() call.
    */
    void setNextTimeout( double nextTime );

    /** Set activity to wakeup.

        The activity given here will be reinserted into the
        ActivitiesQueue, once the timeout is reached.
    */
    void setActivity( const ActivitySharedPtr& rActivity );

private:
    ::canvas::tools::ElapsedTime    maTimer;
    double                          mnNextTime;
    ActivitySharedPtr               mpActivity;
    ActivitiesQueue&                mrActivityQueue;
};

typedef ::boost::shared_ptr< WakeupEvent > WakeupEventSharedPtr;

} // namespace internal
} // namespace presentation

#endif /* INCLUDED_SLIDESHOW_WAKEUPEVENT_HXX */
