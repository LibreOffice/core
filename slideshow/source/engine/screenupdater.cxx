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
#include "precompiled_slideshow.hxx"

#include "screenupdater.hxx"
#include "listenercontainer.hxx"

#include <boost/bind.hpp>
#include <vector>
#include <algorithm>

namespace {
    class UpdateLock : public ::slideshow::internal::ScreenUpdater::UpdateLock
    {
    public:
        UpdateLock (::slideshow::internal::ScreenUpdater& rUpdater, const bool bStartLocked);
        virtual ~UpdateLock (void);
        virtual void Activate (void);
    private:
        ::slideshow::internal::ScreenUpdater& mrUpdater;
        bool mbIsActivated;
    };
}

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

        /// The screen is updated only when mnLockCount==0
        sal_Int32 mnLockCount;

        explicit ImplScreenUpdater( UnoViewContainer const& rViewContainer ) :
            maUpdaters(),
            maViewUpdateRequests(),
            mrViewContainer(rViewContainer),
            mbUpdateAllRequest(false),
            mbViewClobbered(false),
            mnLockCount(0)
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
        if (mpImpl->mnLockCount > 0)
            return;

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
        if (mpImpl->mnLockCount > 0)
            return;

        // TODO(F2): This will interfere with other updates, since it
        // happens out-of-sync with main animation loop. Might cause
        // artifacts.
        std::for_each( mpImpl->mrViewContainer.begin(),
                       mpImpl->mrViewContainer.end(),
                       boost::mem_fn(&View::updateScreen) );
    }

    void ScreenUpdater::lockUpdates (void)
    {
        ++mpImpl->mnLockCount;
        OSL_ASSERT(mpImpl->mnLockCount>0);
    }

    void ScreenUpdater::unlockUpdates (void)
    {
        OSL_ASSERT(mpImpl->mnLockCount>0);
        if (mpImpl->mnLockCount > 0)
        {
            --mpImpl->mnLockCount;
            if (mpImpl->mnLockCount)
                commitUpdates();
        }
    }

    ::boost::shared_ptr<ScreenUpdater::UpdateLock> ScreenUpdater::createLock (const bool bStartLocked)
    {
        return ::boost::shared_ptr<ScreenUpdater::UpdateLock>(new ::UpdateLock(*this, bStartLocked));
    }


} // namespace internal
} // namespace slideshow

namespace {

UpdateLock::UpdateLock (
    ::slideshow::internal::ScreenUpdater& rUpdater,
    const bool bStartLocked)
    : mrUpdater(rUpdater),
      mbIsActivated(false)
{
    if (bStartLocked)
        Activate();
}




UpdateLock::~UpdateLock (void)
{
    if (mbIsActivated)
        mrUpdater.unlockUpdates();
}




void UpdateLock::Activate (void)
{
    if ( ! mbIsActivated)
    {
        mbIsActivated = true;
        mrUpdater.lockUpdates();
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
