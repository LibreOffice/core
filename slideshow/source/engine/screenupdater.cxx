/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "screenupdater.hxx"
#include "listenercontainer.hxx"

#include <boost/shared_ptr.hpp>
#include <boost/mem_fn.hpp>
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

        
        UpdateRequestVector                    maViewUpdateRequests;

        
        UnoViewContainer const&                mrViewContainer;

        
        bool                                   mbUpdateAllRequest;

        
        bool                                   mbViewClobbered;

        
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

        
        //
        
        //
        
        
        
        //
        
        //

        
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
            
            std::for_each( mpImpl->mrViewContainer.begin(),
                           mpImpl->mrViewContainer.end(),
                           mpImpl->mbViewClobbered ?
                           boost::mem_fn(&View::paintScreen) :
                           boost::mem_fn(&View::updateScreen) );
        }
        else if( !mpImpl->maViewUpdateRequests.empty() )
        {
            
            UpdateRequestVector::const_iterator aIter(
                mpImpl->maViewUpdateRequests.begin() );
            const UpdateRequestVector::const_iterator aEnd(
                mpImpl->maViewUpdateRequests.end() );
            while( aIter != aEnd )
            {
                
                
                const UnoViewVector::const_iterator aEndOfViews(
                    mpImpl->mrViewContainer.end() );
                UnoViewVector::const_iterator aFoundView;
                if( (aFoundView=std::find(mpImpl->mrViewContainer.begin(),
                                          aEndOfViews,
                                          aIter->first)) != aEndOfViews )
                {
                    if( aIter->second )
                        (*aFoundView)->paintScreen(); 
                    else
                        (*aFoundView)->updateScreen(); 
                }

                ++aIter;
            }
        }

        
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


} 
} 

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
