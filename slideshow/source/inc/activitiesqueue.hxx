/*************************************************************************
 *
 *  $RCSfile: activitiesqueue.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 19:11:35 $
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

#ifndef _SLIDESHOW_ACTIVITIESQUEUE_HXX
#define _SLIDESHOW_ACTIVITIESQUEUE_HXX

#include <list>
#include <vector>

#include <activity.hxx>
#include <layermanager.hxx>
#include <unoviewcontainer.hxx>


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
        class ActivitiesQueue
        {
        public:
            /** Create an ActivitiesQueue.

                @param rViews
                Container of presentation views, which will all be called
                with updateScreen(), when a full round of activities
                has been performed.
             */
            ActivitiesQueue( const UnoViewContainer& rViews );

            ~ActivitiesQueue();

            /** Add the given activity to the queue.
             */
            bool addActivity( const ActivitySharedPtr& );

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

            /** Set a LayerManager for update() calls.

                A LayerManager set via this method will receive one
                update() call per animation frame. This is handy to
                avoid multiple redraws. Note that every
                setLayerManager() call will overwrite any previously
                set.
             */
            void setLayerManager( const LayerManagerSharedPtr& rMgr );

            /** Remove all pending activities from the queue.
             */
            void clear();

        private:
            // default: disabled copy/assignment
            ActivitiesQueue(const ActivitiesQueue&);
            ActivitiesQueue& operator=( const ActivitiesQueue& );

            typedef ::std::list< ActivitySharedPtr >        ActivityQueue;

            LayerManagerSharedPtr   mpLayerManager; // for screen updates

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

            const UnoViewContainer& mrViews;                    // Container of presentation views,
                                                                // which will all be called with
                                                                // updateScreen(), when a full
                                                                // round of activities has been
                                                                // performed.

            bool                    mbCurrentRoundNeedsScreenUpdate;
        };

    }
}
#endif /* _SLIDESHOW_ACTIVITIESQUEUE_HXX */
