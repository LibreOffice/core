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
            Event(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("WakeupEvent"))),
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
            // this event won't expire, we fire everytime we're
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
