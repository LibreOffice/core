/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: screenupdater.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2007-07-19 14:47:27 $
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
#include "precompiled_slideshow.hxx"

#include "screenupdater.hxx"
#include "listenercontainer.hxx"

#include <boost/bind.hpp>
#include <vector>
#include <algorithm>

namespace slideshow
{
namespace internal
{
    typedef std::vector<
        std::pair<UnoViewSharedPtr,bool> > UpdateRequestVector;

    struct ScreenUpdater::ImplScreenUpdater
    {
        /** List of registered ViewUpdaters, to consult for necessary
            updates
        */
        ThreadUnsafeListenerContainer<
            ViewUpdateSharedPtr,
            std::vector<ViewUpdateSharedPtr> > maUpdaters;

        /// Views that have been notified for update
        UpdateRequestVector                    maViewUpdateRequests;

        /// List of View. Used to issue screen updates on.
        UnoViewContainer const&                mrViewContainer;

        /// True, if a notifyUpdate() for all views has been issued.
        bool                                   mbUpdateAllRequest;

        /// True, if at least one notifyUpdate() call had bViewClobbered set
        bool                                   mbViewClobbered;

        explicit ImplScreenUpdater( UnoViewContainer const& rViewContainer ) :
            maUpdaters(),
            maViewUpdateRequests(),
            mrViewContainer(rViewContainer),
            mbUpdateAllRequest(false),
            mbViewClobbered(false)
        {}
    };

    ScreenUpdater::ScreenUpdater( UnoViewContainer const& rViewContainer ) :
        mpImpl(new ImplScreenUpdater(rViewContainer) )
    {
    }

    ScreenUpdater::~ScreenUpdater()
    {
        // outline because of pimpl
    }

    void ScreenUpdater::notifyUpdate()
    {
        mpImpl->mbUpdateAllRequest = true;
    }

    void ScreenUpdater::notifyUpdate( const UnoViewSharedPtr& rView,
                                      bool                    bViewClobbered )
    {
        mpImpl->maViewUpdateRequests.push_back(
            std::make_pair(rView, bViewClobbered) );

        if( bViewClobbered )
            mpImpl->mbViewClobbered = true;
    }

    void ScreenUpdater::commitUpdates()
    {
        // cases:
        //
        // (a) no update necessary at all
        //
        // (b) no ViewUpdate-generated update
        //     I. update all views requested -> for_each( mrViewContainer )
        //    II. update some views requested -> for_each( maViewUpdateRequests )
        //
        // (c) ViewUpdate-triggered update - update all views
        //

        // any ViewUpdate-triggered updates?
        const bool bViewUpdatesNeeded(
            mpImpl->maUpdaters.apply(
                boost::mem_fn(&ViewUpdate::needsUpdate)) );

        if( bViewUpdatesNeeded )
        {
            mpImpl->maUpdaters.applyAll(
                boost::mem_fn((bool (ViewUpdate::*)())&ViewUpdate::update) );
        }

        if( bViewUpdatesNeeded ||
            mpImpl->mbUpdateAllRequest )
        {
            // unconditionally update all views
            std::for_each( mpImpl->mrViewContainer.begin(),
                           mpImpl->mrViewContainer.end(),
                           mpImpl->mbViewClobbered ?
                           boost::mem_fn(&View::paintScreen) :
                           boost::mem_fn(&View::updateScreen) );
        }
        else if( !mpImpl->maViewUpdateRequests.empty() )
        {
            // update notified views only
            UpdateRequestVector::const_iterator aIter(
                mpImpl->maViewUpdateRequests.begin() );
            const UpdateRequestVector::const_iterator aEnd(
                mpImpl->maViewUpdateRequests.end() );
            while( aIter != aEnd )
            {
                // TODO(P1): this is O(n^2) in the number of views, if
                // lots of views notify updates.
                const UnoViewVector::const_iterator aEndOfViews(
                    mpImpl->mrViewContainer.end() );
                UnoViewVector::const_iterator aFoundView;
                if( (aFoundView=std::find(mpImpl->mrViewContainer.begin(),
                                          aEndOfViews,
                                          aIter->first)) != aEndOfViews )
                {
                    if( aIter->second )
                        (*aFoundView)->paintScreen(); // force-paint
                    else
                        (*aFoundView)->updateScreen(); // update changes only
                }

                ++aIter;
            }
        }

        // done - clear requests
        mpImpl->mbViewClobbered = false;
        mpImpl->mbUpdateAllRequest = false;
        UpdateRequestVector().swap( mpImpl->maViewUpdateRequests );
    }

    void ScreenUpdater::addViewUpdate( ViewUpdateSharedPtr const& rViewUpdate )
    {
        mpImpl->maUpdaters.add( rViewUpdate );
    }

    void ScreenUpdater::removeViewUpdate( ViewUpdateSharedPtr const& rViewUpdate )
    {
        mpImpl->maUpdaters.remove( rViewUpdate );
    }

    void ScreenUpdater::requestImmediateUpdate()
    {
        // TODO(F2): This will interfere with other updates, since it
        // happens out-of-sync with main animation loop. Might cause
        // artifacts.
        std::for_each( mpImpl->mrViewContainer.begin(),
                       mpImpl->mrViewContainer.end(),
                       boost::mem_fn(&View::updateScreen) );
    }

} // namespace internal
} // namespace slideshow
