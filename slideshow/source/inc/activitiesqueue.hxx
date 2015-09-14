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

#ifndef INCLUDED_SLIDESHOW_SOURCE_INC_ACTIVITIESQUEUE_HXX
#define INCLUDED_SLIDESHOW_SOURCE_INC_ACTIVITIESQUEUE_HXX

#include <deque>

#include "activity.hxx"
#include "unoviewcontainer.hxx"

#include <canvas/elapsedtime.hxx>

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>


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
                const std::shared_ptr< ::canvas::tools::ElapsedTime >&  pPresTimer );
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
            std::shared_ptr< ::canvas::tools::ElapsedTime > const &
            getTimer() const { return mpTimer; }

        private:
            std::shared_ptr< ::canvas::tools::ElapsedTime > mpTimer;

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
#endif // INCLUDED_SLIDESHOW_SOURCE_INC_ACTIVITIESQUEUE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
