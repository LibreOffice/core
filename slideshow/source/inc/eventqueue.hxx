/*************************************************************************
 *
 *  $RCSfile: eventqueue.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 19:16:11 $
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

#ifndef _SLIDESHOW_EVENTQUEUE_HXX
#define _SLIDESHOW_EVENTQUEUE_HXX

#ifndef _CANVAS_ELAPSEDTIME_HXX
#include <canvas/elapsedtime.hxx>
#endif

#include <queue>

#include <event.hxx>


/* Definition of ActivitiesQueue class */

namespace presentation
{
    namespace internal
    {
        /** This class handles events in a presentation. Events are
            time instants where e.g. effects start.
         */
        class EventQueue
        {
        public:
            EventQueue();

            ~EventQueue();

            /** Add the given event to the queue. The event is fired
                at, or shortly after, its Event::getActivationTime instant.
             */
            bool addEvent( const EventSharedPtr& event );

            /** Process the event queue.

                This method performs the smallest atomic processing
                possible on the queue (typically, this means one event
                get processed).

                @param pTimeoutForNextCall
                Timeout in seconds, until the next event is ready. The
                interval can be used by the caller to perform other
                tasks. This is permitted to be NULL.
             */
            void process( double* pTimeoutForNextCall );

            /** Query state of the queue

                @return false, if queue is empty, true otherwise
             */
            bool isEmpty();

            /** Remove all pending events from the queue.
             */
            void clear();

        private:
            // default: disabled copy/assignment
            EventQueue(const EventQueue&);
            EventQueue& operator=( const EventQueue& );

            struct EventEntry
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
            ::canvas::tools::ElapsedTime    maElapsedTime;
        };

    }
}
#endif /* _SLIDESHOW_EVENTQUEUE_HXX */
