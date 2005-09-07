/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: event.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 21:10:48 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SLIDESHOW_EVENT_HXX
#define _SLIDESHOW_EVENT_HXX

#include <boost/shared_ptr.hpp>
#include <vector>

#include <disposable.hxx>


/* Definition of Event interface */

namespace presentation
{
    namespace internal
    {

        class Event : public Disposable
        {
        public:
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
        };

        typedef ::boost::shared_ptr< Event > EventSharedPtr;
        typedef ::std::vector< EventSharedPtr > VectorOfEvents;

    }
}

#endif /* _SLIDESHOW_EVENT_HXX */
