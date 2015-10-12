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

#include "screenupdater.hxx"
#include "listenercontainer.hxx"

#include <osl/diagnose.h>

#include <boost/shared_ptr.hpp>
#include <boost/mem_fn.hpp>
#include <vector>
#include <algorithm>

namespace {
    class UpdateLock : public ::slideshow::internal::ScreenUpdater::UpdateLock
    {
    public:
        UpdateLock (::slideshow::internal::ScreenUpdater& rUpdater, const bool bStartLocked);
        virtual ~UpdateLock();
        virtual void Activate() override;
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

        // (a) no update necessary at all

        // (b) no ViewUpdate-generated update
        //     I. update all views requested -> for_each( mrViewContainer )
        //    II. update some views requested -> for_each( maViewUpdateRequests )

        // (c) ViewUpdate-triggered update - update all views


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
            for( const auto& pView : mpImpl->mrViewContainer )
            {
                if( mpImpl->mbViewClobbered )
                    pView->paintScreen();
                else
                    pView->updateScreen();
            }
        }
        else if( !mpImpl->maViewUpdateRequests.empty() )
        {
            // update notified views only
            for( const auto& rViewUpdateRequest : mpImpl->maViewUpdateRequests )
            {
                // TODO(P1): this is O(n^2) in the number of views, if
                // lots of views notify updates.
                const UnoViewVector::const_iterator aEndOfViews(
                    mpImpl->mrViewContainer.end() );
                UnoViewVector::const_iterator aFoundView;
                if( (aFoundView=std::find(mpImpl->mrViewContainer.begin(),
                                          aEndOfViews,
                                          rViewUpdateRequest.first)) != aEndOfViews )
                {
                    if( rViewUpdateRequest.second )
                        (*aFoundView)->paintScreen(); // force-paint
                    else
                        (*aFoundView)->updateScreen(); // update changes only
                }
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
        for( auto const& pView : mpImpl->mrViewContainer )
            pView->updateScreen();
    }

    void ScreenUpdater::lockUpdates()
    {
        ++mpImpl->mnLockCount;
        OSL_ASSERT(mpImpl->mnLockCount>0);
    }

    void ScreenUpdater::unlockUpdates()
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




UpdateLock::~UpdateLock()
{
    if (mbIsActivated)
        mrUpdater.unlockUpdates();
}




void UpdateLock::Activate()
{
    if ( ! mbIsActivated)
    {
        mbIsActivated = true;
        mrUpdater.lockUpdates();
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
