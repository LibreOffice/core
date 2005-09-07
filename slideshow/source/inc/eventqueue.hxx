/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: eventqueue.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 21:11:43 $
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

#ifndef _SLIDESHOW_EVENTQUEUE_HXX
#define _SLIDESHOW_EVENTQUEUE_HXX

#ifndef _CANVAS_ELAPSEDTIME_HXX
#include <canvas/elapsedtime.hxx>
#endif

#include <queue>
#include <event.hxx>

#include <boost/utility.hpp> // for boost::noncopyable
#include <functional>

/* Definition of ActivitiesQueue class */

namespace presentation
{
    namespace internal
    {
        /** This class handles events in a presentation. Events are
            time instants where e.g. effects start.
         */
        class EventQueue : private ::boost::noncopyable
        {
        public:
            EventQueue(
                ::boost::shared_ptr< ::canvas::tools::ElapsedTime >
                const & pPresTimer );

            ~EventQueue();

            /** Add the given event to the queue. The event is fired
                at, or shortly after, its Event::getActivationTime instant.
             */
            bool addEvent( const EventSharedPtr& event );

            /** Process the event queue.

                This method performs the smallest atomic processing
                possible on the queue (typically, this means one event
                get processed).

                @return Timeout in seconds, until the next event is
                ready. The time returned here is relative to the pres
                timer (i.e. the timer specified at the EventQueue
                constructor). When the queue is empty (i.e. isEmpty()
                returns true), the returned value is the highest
                representable double value
                (::std::numeric_limits<double>::max()).
             */
            double process();

            /** Query state of the queue

                @return false, if queue is empty, true otherwise
             */
            bool isEmpty();

            /** Remove all pending events from the queue.
             */
            void clear();

            /** Gets the queue's timer object.
             */
            ::boost::shared_ptr< ::canvas::tools::ElapsedTime > const &
            getTimer() const { return mpTimer; }

        private:
            struct EventEntry : public ::std::unary_function<EventEntry, bool>
            {
                EventSharedPtr  pEvent;
                double          nTime;

                bool operator<( const EventEntry& ) const; // to leverage priority_queue's default compare
            };

            typedef ::std::priority_queue< EventEntry > ImplQueueType;

            ImplQueueType                   maEvents;

            // perform timing of events via relative time
            // measurements. The world time starts, when the
            // EventQueue object is created
            ::boost::shared_ptr< ::canvas::tools::ElapsedTime > mpTimer;
        };

    }
}
#endif /* _SLIDESHOW_EVENTQUEUE_HXX */
