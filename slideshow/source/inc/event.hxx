/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef INCLUDED_SLIDESHOW_EVENT_HXX
#define INCLUDED_SLIDESHOW_EVENT_HXX

#include "disposable.hxx"
#include "debug.hxx"
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
#if OSL_DEBUG_LEVEL > 1
    Event (const ::rtl::OUString& rsDescription) : msDescription(rsDescription) {};
#endif

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

#if OSL_DEBUG_LEVEL > 1
    ::rtl::OUString GetDescription (void) const { return msDescription; }

private:
    const ::rtl::OUString msDescription;
#endif
};

typedef ::boost::shared_ptr< Event > EventSharedPtr;
typedef ::std::vector< EventSharedPtr > VectorOfEvents;

} // namespace internal
} // namespace presentation

#endif /* INCLUDED_SLIDESHOW_EVENT_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
