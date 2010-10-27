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

#ifndef INCLUDED_SLIDESHOW_EVENTQUEUE_HXX
#define INCLUDED_SLIDESHOW_EVENTQUEUE_HXX

#include <canvas/elapsedtime.hxx>
#include <osl/mutex.hxx>

#include "event.hxx"

#include <boost/noncopyable.hpp>
#include <functional>
#include <queue>
#include <vector>


/* Definition of ActivitiesQueue class */

namespace slideshow
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

            /** Add the given event to the queue. The event is fired
                at, or shortly after, its Event::getActivationTime instant.
                The difference to addEvent() is that events added during
                process() are postponed to next process().
             */
            bool addEventForNextRound( const EventSharedPtr& event );

            /** Another way to control the order of asynchronous event
                exeqution.  Use this method to schedule events that are to
                be executed after all regular events that have no delay,
                even when they schedule new regular events without delay.
            */
            bool addEventWhenQueueIsEmpty (const EventSharedPtr& rpEvent);

            /** Process the event queue.

                This method executes all events whose timeout has
                expired when calling this method (i.e. all events
                whose scheduled time is less or equal the current
                time).

                Check for the next available event's timeout via
                nextTimeout(), or whether the queue is empty
                altogether via isEmpty().
             */
            void process();

            /** Query state of the queue

                @return false, if queue is empty, true otherwise
             */
            bool isEmpty() const;

            /** Query timeout for the topmost event in the queue.

                @return Timeout in seconds, until the next event is
                ready. The time returned here is relative to the pres
                timer (i.e. the timer specified at the EventQueue
                constructor). When the queue is empty (i.e. isEmpty()
                returns true), the returned value is the highest
                representable double value
                (::std::numeric_limits<double>::max()). If the topmost
                event in the queue is already pending, the timeout
                returned here will actually be negative.
            */
            double nextTimeout() const;

            /** Remove all pending events from the queue.
             */
            void clear();

            /** Forces an empty queue, firing all events immediately
                without minding any times.
                @attention do only call from event loop, this calls process_()!
             */
            void forceEmpty();

            /** Gets the queue's timer object.
             */
            ::boost::shared_ptr< ::canvas::tools::ElapsedTime > const &
            getTimer() const { return mpTimer; }

        private:
            mutable ::osl::Mutex      maMutex;

            struct EventEntry : public ::std::unary_function<EventEntry, bool>
            {
                EventSharedPtr  pEvent;
                double          nTime;

                bool operator<( const EventEntry& ) const; // to leverage priority_queue's default compare

                EventEntry( EventSharedPtr const& p, double t )
                    : pEvent(p), nTime(t) {}
            };

            typedef ::std::priority_queue< EventEntry > ImplQueueType;
            ImplQueueType                   maEvents;
            typedef ::std::vector<EventEntry> EventEntryVector;
            EventEntryVector                maNextEvents;
            ImplQueueType                   maNextNextEvents;
            void process_( bool bFireAllEvents );

            // perform timing of events via relative time
            // measurements. The world time starts, when the
            // EventQueue object is created
            ::boost::shared_ptr< ::canvas::tools::ElapsedTime > mpTimer;
        };

    }
}
#endif /* INCLUDED_SLIDESHOW_EVENTQUEUE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
