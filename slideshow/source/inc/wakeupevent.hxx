/*************************************************************************
 *
 *  $RCSfile: wakeupevent.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 19:24:32 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SLIDESHOW_WAKEUPEVENT_HXX
#define _SLIDESHOW_WAKEUPEVENT_HXX

#ifndef BOOST_BIND_HPP_INCLUDED
#include <boost/bind.hpp>
#endif
#ifndef _CANVAS_ELAPSEDTIME_HXX
#include <canvas/elapsedtime.hxx>
#endif

#include <event.hxx>
#include <activitiesqueue.hxx>


namespace presentation
{
    namespace internal
    {
        /** Little helper class, used to set Activities active again
            after some sleep period.

            Clients can use this class to schedule wakeup events at
            the EventQueue, to avoid busy-waiting for the next
            discrete time instant.
        */
        class WakeupEvent : public Event
        {
        public:
            WakeupEvent( ActivitiesQueue& rActivityQueue );

            virtual void dispose();
            virtual bool fire();
            virtual bool wasFired() const;
            virtual double getActivationTime( double nCurrentTime ) const;

            /// Start the internal timer
            void start();

            /** Set the next timeout this object should generate.

                @param rNextTime
                Absolute time, measured from the last start() call,
                when this event should wakeup the Activity again. If
                your time is relative, simply call start() just before
                every setNextTimeout() call.
            */
            void setNextTimeout( double rNextTime );

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
    }
}

#endif /* _SLIDESHOW_WAKEUPEVENT_HXX */
