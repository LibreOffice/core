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

#ifndef INCLUDED_SLIDESHOW_SOURCE_INC_SCREENUPDATER_HXX
#define INCLUDED_SLIDESHOW_SOURCE_INC_SCREENUPDATER_HXX

#include "viewupdate.hxx"
#include "unoviewcontainer.hxx"
#include <memory>

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
        class ScreenUpdater
        {
        public:
            explicit ScreenUpdater( UnoViewContainer const& rViewContainer );
            ~ScreenUpdater();
            ScreenUpdater(const ScreenUpdater&) = delete;
            ScreenUpdater& operator=(const ScreenUpdater&) = delete;

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
            void notifyUpdate( const UnoViewSharedPtr& rView, bool bViewClobbered );

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

            class UpdateLock {
            public:
                virtual void Activate() = 0;

            protected:
                ~UpdateLock() {}
            };

            /** Call this method to create a lock instead of calling
                lockUpdates() and unlockUpdates() directly.
            */
            ::std::shared_ptr<UpdateLock> createLock();

            /** Lock updates to prevent intermediate repaints.
            */
            void lockUpdates();

            /** When called as often as lockUpdates() then commitUpdates()
                is called.
            */
            void unlockUpdates();

        private:
            struct ImplScreenUpdater;
            std::unique_ptr<ImplScreenUpdater> mpImpl;

        };
    }
}

#endif // INCLUDED_SLIDESHOW_SOURCE_INC_SCREENUPDATER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
