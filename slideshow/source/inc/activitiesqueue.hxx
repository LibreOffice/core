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

#ifndef INCLUDED_SLIDESHOW_ACTIVITIESQUEUE_HXX
#define INCLUDED_SLIDESHOW_ACTIVITIESQUEUE_HXX

#include <deque>

#include "activity.hxx"
#include "unoviewcontainer.hxx"

#include <canvas/elapsedtime.hxx>

#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp> // for boost::noncopyable


/* Definition of ActivitiesQueue class */

namespace slideshow
{
    namespace internal
    {
        /** This class handles the XSprite updates needed for
            animations, such as moves, scales etc. You can add
            activity objects to this class, which are called in a
            round-robin fashion.
        */
        class ActivitiesQueue : private ::boost::noncopyable
        {
        public:
            /** Create an ActivitiesQueue.

                @param pPresTimer
                Pointer to global presentation timer. Used for
                adjusting and holding global presentation time.
             */
            ActivitiesQueue(
                const ::boost::shared_ptr< ::canvas::tools::ElapsedTime >&  pPresTimer );
            ~ActivitiesQueue();

            /** Add the given activity to the queue.
             */
            bool addActivity( const ActivitySharedPtr& pActivity );

            /** Process the activities queue.

                This method performs the smallest atomic processing
                possible on the queue (typically, this means one
                activity get processed).
             */
            void process();

            /** Call all dequeued activities' dequeued() method
             */
            void processDequeued();

            /** Query state of the queue

                @return false, if queue is empty, true otherwise
             */
            bool isEmpty() const;

            /** Remove all pending activities from the queue.
             */
            void clear();

            /** Gets the queue's timer object.
             */
            ::boost::shared_ptr< ::canvas::tools::ElapsedTime > const &
            getTimer() const { return mpTimer; }

            /** returns number of all activities, waiting, reinserted and dequeued
             */
            std::size_t size() const
            {
                return maCurrentActivitiesWaiting.size() + maCurrentActivitiesReinsert.size() + maDequeuedActivities.size();
            }

        private:
            ::boost::shared_ptr< ::canvas::tools::ElapsedTime > mpTimer;

            typedef ::std::deque< ActivitySharedPtr > ActivityQueue;

            ActivityQueue           maCurrentActivitiesWaiting;  // currently running
                                                                 // activities, that still
                                                                 // await processing for this
                                                                 // round

            ActivityQueue           maCurrentActivitiesReinsert;    // currently running
                                                                      // activities, that are
                                                                      // already processed for
                                                                      // this round, and wants
                                                                    // to be reinserted next
                                                                    // round

            ActivityQueue           maDequeuedActivities; // This list collects all activities which did not request
                                                          // a reinsertion. After the screen update has been
                                                          // performed, those are notified via dequeued(). This
                                                          // facilitates cleanup actions taking place _after_ the
                                                          // current frame has been displayed.
        };

    }
}
#endif /* INCLUDED_SLIDESHOW_ACTIVITIESQUEUE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
