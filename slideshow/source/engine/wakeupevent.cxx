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


#include <wakeupevent.hxx>


namespace slideshow
{
    namespace internal
    {
        WakeupEvent::WakeupEvent(
            std::shared_ptr<canvas::tools::ElapsedTime> const & pTimeBase,
            ActivitiesQueue& rActivityQueue ) :
            Event("WakeupEvent"),
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
