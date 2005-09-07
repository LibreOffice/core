/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: activitiesqueue.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 21:03:23 $
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

#ifndef _SLIDESHOW_ACTIVITIESQUEUE_HXX
#define _SLIDESHOW_ACTIVITIESQUEUE_HXX

#include <deque>

#include <activity.hxx>
#include <unoviewcontainer.hxx>
#include <eventmultiplexer.hxx>

#include "canvas/elapsedtime.hxx"

#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp> // for boost::noncopyable


/* Definition of ActivitiesQueue class */

namespace presentation
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

                @param rEventMultiplexer
                Handles screen updates for us, when a full round of
                activities has been performed.
             */
            ActivitiesQueue(
                const ::boost::shared_ptr< ::canvas::tools::ElapsedTime >&  pPresTimer,
                EventMultiplexer&                                           rEventMultiplexer );

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

            /** Query state of the queue

                @return false, if queue is empty, true otherwise
             */
            bool isEmpty();

            /** Remove all pending activities from the queue.
             */
            void clear();

            /** Gets the queue's timer object.
             */
            ::boost::shared_ptr< ::canvas::tools::ElapsedTime > const &
            getTimer() const { return mpTimer; }

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

            EventMultiplexer&       mrEventMultiplexer;

            bool                    mbCurrentRoundNeedsScreenUpdate;
        };

    }
}
#endif /* _SLIDESHOW_ACTIVITIESQUEUE_HXX */
