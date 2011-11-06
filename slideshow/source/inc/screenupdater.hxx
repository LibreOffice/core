/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef INCLUDED_SLIDESHOW_SCREENUPDATER_HXX
#define INCLUDED_SLIDESHOW_SCREENUPDATER_HXX

#include "viewupdate.hxx"
#include "unoviewcontainer.hxx"
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

/* Definition of ScreenUpdater class */

namespace slideshow
{
    namespace internal
    {
        /** Screen updater

            This class handles and synchronizes screen updates
            centrally. Therefore, it can hold a number of ViewUpdate
            objects, which are polled for pending updates on
            commitUpdates(). Furthermore, external code can request
            updates via notifyUpdate() calls. If neither such an
            update was requested, nor any of the registered ViewUpdate
            objects report any pending update, commitUpdates() does
            nothing.
         */
        class ScreenUpdater : boost::noncopyable
        {
        public:
            explicit ScreenUpdater( UnoViewContainer const& rViewContainer );
            ~ScreenUpdater();

            /** Notify screen update

                This method records a screen content update request
                for all views.
            */
            void notifyUpdate();

            /** Notify screen update

                This method records a screen content update request
                for the given view.

                @param rView
                The view that needs an update

                @param bViewClobbered
                When true, notifies update that view content is
                clobbered by external circumstances (e.g. by another
                application), and needs update even if the
                implementation 'thinks' it does not need to render
                something to screen.
            */
            void notifyUpdate( const UnoViewSharedPtr& rView, bool bViewClobbered=false );

            /** Commits collected update actions
             */
            void commitUpdates();

            /** Register ViewUpdate

                @param rViewUpdate
                Add this ViewUpdate to the list that's asked for
                pending updates
             */
            void addViewUpdate( ViewUpdateSharedPtr const& rViewUpdate );

            /** Unregister ViewUpdate

                @param rViewUpdate
                Remove this ViewUpdate from the list that's asked for
                pending updates
             */
            void removeViewUpdate( ViewUpdateSharedPtr const& );

            /** A wart.

                Used to fire an immediate screen update. Currently
                needed for the wait symbol, since switching that on
                and off does get to commitUpdates()
             */
            void requestImmediateUpdate();

            class UpdateLock {public: virtual void Activate (void) = 0; };

            /** Call this method to create a lock instead of calling
                lockUpdates() and unlockUpdates() directly.
                @param bStartLocked
                    When <TRUE/> then the UpdateLock is created already
                    locked. When <FALSE/> then Activate() has to be called in order
                    to lock the lock.
            */
            ::boost::shared_ptr<UpdateLock> createLock (const bool bStartLocked);

            /** Lock updates to prevent intermediate repaints.
            */
            void lockUpdates (void);

            /** When called as often as lockUpdates() then commitUpdates()
                is called.
            */
            void unlockUpdates (void);

        private:
            struct ImplScreenUpdater;
            boost::scoped_ptr<ImplScreenUpdater> mpImpl;

        };
    }
}

#endif /* INCLUDED_SLIDESHOW_SCREENUPDATER_HXX */
