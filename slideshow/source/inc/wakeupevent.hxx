/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SLIDESHOW_SOURCE_INC_WAKEUPEVENT_HXX
#define INCLUDED_SLIDESHOW_SOURCE_INC_WAKEUPEVENT_HXX

#include <canvas/elapsedtime.hxx>

#include "event.hxx"
#include "activitiesqueue.hxx"

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
        std::shared_ptr< ::canvas::tools::ElapsedTime > const& pTimeBase,
        ActivitiesQueue & rActivityQueue );

    virtual void dispose() override;
    virtual bool fire() override;
    virtual bool isCharged() const override;
    virtual double getActivationTime( double nCurrentTime ) const override;

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

#endif // INCLUDED_SLIDESHOW_SOURCE_INC_WAKEUPEVENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
