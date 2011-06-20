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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
