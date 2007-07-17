/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: screenupdater.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2007-07-17 15:12:43 $
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

        private:
            struct ImplScreenUpdater;
            boost::scoped_ptr<ImplScreenUpdater> mpImpl;
        };
    }
}

#endif /* INCLUDED_SLIDESHOW_SCREENUPDATER_HXX */
