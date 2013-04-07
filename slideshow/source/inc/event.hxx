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
#ifndef INCLUDED_SLIDESHOW_EVENT_HXX
#define INCLUDED_SLIDESHOW_EVENT_HXX

#include "disposable.hxx"
#include <rtl/ustring.hxx>
#include <boost/shared_ptr.hpp>
#include <vector>

namespace slideshow {
namespace internal {

/** Definition of Event interface
 */
class Event : public Disposable
{
public:
    Event (const OUString& rsDescription) : msDescription(rsDescription) {}

    /** Execute the event.

        @return true, if event was successfully executed.
    */
    virtual bool fire() = 0;

    /** Query whether this event is still charged, i.e. able
        to fire.

        Inactive events are ignored by the normal event
        containers (EventQueue, UserEventQueue etc.), and no
        explicit fire() is called upon them.

        @return true, if this event has already been fired.
    */
    virtual bool isCharged() const = 0;

    /** Query the activation time instant this event shall be
        fired, if it was inserted at instant nCurrentTime into
        the queue.

        @param nCurrentTime
        The time from which the activation time is to be
        calculated from.

        @return the time instant in seconds, on which this
        event is to be fired.
    */
    virtual double getActivationTime( double nCurrentTime ) const = 0;

    const OUString& GetDescription (void) const { return msDescription; }

private:
    const OUString msDescription;
};

typedef ::boost::shared_ptr< Event > EventSharedPtr;
typedef ::std::vector< EventSharedPtr > VectorOfEvents;

} // namespace internal
} // namespace presentation

#endif /* INCLUDED_SLIDESHOW_EVENT_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
