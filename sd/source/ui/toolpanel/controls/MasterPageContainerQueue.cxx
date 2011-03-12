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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "MasterPageContainerQueue.hxx"

#include "tools/IdleDetection.hxx"

#include <set>

namespace sd { namespace toolpanel { namespace controls {

const sal_Int32 MasterPageContainerQueue::snDelayedCreationTimeout (15);
const sal_Int32 MasterPageContainerQueue::snDelayedCreationTimeoutWhenNotIdle (100);
const sal_Int32 MasterPageContainerQueue::snMasterPagePriorityBoost (5);
const sal_Int32 MasterPageContainerQueue::snWaitForMoreRequestsPriorityThreshold (-10);
sal_uInt32 MasterPageContainerQueue::snWaitForMoreRequestsCount(15);

//===== MasterPageContainerQueue::PreviewCreationRequest ======================

class MasterPageContainerQueue::PreviewCreationRequest
{
public:
    PreviewCreationRequest (const SharedMasterPageDescriptor& rpDescriptor, int nPriority)
        : mpDescriptor(rpDescriptor),
          mnPriority(nPriority)
    {}
    SharedMasterPageDescriptor mpDescriptor;
    int mnPriority;
    class Compare
    {
    public:
        bool operator() (const PreviewCreationRequest& r1,const PreviewCreationRequest& r2) const
        {
            if (r1.mnPriority != r2.mnPriority)
            {
                // Prefer requests with higher priority.
                return r1.mnPriority > r2.mnPriority;
            }
            else
            {
                // Prefer tokens that have been earlier created (those with lower
                // value).
                return r1.mpDescriptor->maToken < r2.mpDescriptor->maToken;
            }
        }
    };
    class CompareToken
    {
        public:
        MasterPageContainer::Token maToken;
        CompareToken(MasterPageContainer::Token aToken) : maToken(aToken) {}
        bool operator() (const PreviewCreationRequest& rRequest) const
            { return maToken==rRequest.mpDescriptor->maToken; }
    };
};




//===== MasterPageContainerQueue::RequestQueue ================================

class MasterPageContainerQueue::RequestQueue
    : public ::std::set<PreviewCreationRequest,PreviewCreationRequest::Compare>
{
public:
    RequestQueue (void) {}
};




//===== MasterPageContainerQueue ==============================================

MasterPageContainerQueue* MasterPageContainerQueue::Create (
    const ::boost::weak_ptr<ContainerAdapter>& rpContainer)
{
    MasterPageContainerQueue* pQueue = new MasterPageContainerQueue(rpContainer);
    pQueue->LateInit();
    return pQueue;
}




MasterPageContainerQueue::MasterPageContainerQueue (
    const ::boost::weak_ptr<ContainerAdapter>& rpContainer)
    : mpWeakContainer(rpContainer),
      mpRequestQueue(new RequestQueue()),
      maDelayedPreviewCreationTimer(),
      mnRequestsServedCount(0)
{
}




MasterPageContainerQueue::~MasterPageContainerQueue (void)
{
    maDelayedPreviewCreationTimer.Stop();
    while ( ! mpRequestQueue->empty())
        mpRequestQueue->erase(mpRequestQueue->begin());
}




void MasterPageContainerQueue::LateInit (void)
{
    // Set up the timer for the delayed creation of preview bitmaps.
    maDelayedPreviewCreationTimer.SetTimeout (snDelayedCreationTimeout);
    Link aLink (LINK(this,MasterPageContainerQueue,DelayedPreviewCreation));
    maDelayedPreviewCreationTimer.SetTimeoutHdl(aLink);
}




bool MasterPageContainerQueue::RequestPreview (const SharedMasterPageDescriptor& rpDescriptor)
{
    bool bSuccess (false);
    if (rpDescriptor.get() != NULL
        && rpDescriptor->maLargePreview.GetSizePixel().Width() == 0)
    {
        sal_Int32 nPriority (CalculatePriority(rpDescriptor));

        // Add a new or replace an existing request.
        RequestQueue::iterator iRequest (::std::find_if(
            mpRequestQueue->begin(),
            mpRequestQueue->end(),
            PreviewCreationRequest::CompareToken(rpDescriptor->maToken)));
        // When a request for the same token exists then the lowest of the
        // two priorities is used.
        if (iRequest != mpRequestQueue->end())
            if (iRequest->mnPriority < nPriority)
            {
                mpRequestQueue->erase(iRequest);
                iRequest = mpRequestQueue->end();
            }

        // Add a new request when none exists (or has just been erased).
        if (iRequest == mpRequestQueue->end())
        {
            mpRequestQueue->insert(PreviewCreationRequest(rpDescriptor,nPriority));
            maDelayedPreviewCreationTimer.Start();
            bSuccess = true;
        }
    }
    return bSuccess;
}




sal_Int32 MasterPageContainerQueue::CalculatePriority (
    const SharedMasterPageDescriptor& rpDescriptor) const
{
    sal_Int32 nPriority;

    // The cost is used as a starting value.
    int nCost (0);
    if (rpDescriptor->mpPreviewProvider.get() != NULL)
    {
        nCost = rpDescriptor->mpPreviewProvider->GetCostIndex();
        if (rpDescriptor->mpPreviewProvider->NeedsPageObject())
            if (rpDescriptor->mpPageObjectProvider.get() != NULL)
                nCost += rpDescriptor->mpPageObjectProvider->GetCostIndex();
    }

    // Its negative value is used so that requests with a low cost are
    // preferred over those with high costs.
    nPriority = -nCost;

    // Add a term that introduces an order based on the appearance in the
    // AllMasterPagesSelector.
    nPriority -= rpDescriptor->maToken / 3;

    // Process requests for the CurrentMasterPagesSelector first.
    if (rpDescriptor->meOrigin == MasterPageContainer::MASTERPAGE)
        nPriority += snMasterPagePriorityBoost;

    return nPriority;
}




IMPL_LINK(MasterPageContainerQueue, DelayedPreviewCreation, Timer*, pTimer)
{
    bool bIsShowingFullScreenShow (false);
    bool bWaitForMoreRequests (false);

    do
    {
        if (mpRequestQueue->size() == 0)
            break;

        // First check whether the system is idle.
        sal_Int32 nIdleState (tools::IdleDetection::GetIdleState());
        if (nIdleState != tools::IdleDetection::IDET_IDLE)
        {
            if ((nIdleState&tools::IdleDetection::IDET_FULL_SCREEN_SHOW_ACTIVE) != 0)
                bIsShowingFullScreenShow = true;
            break;
        }

        PreviewCreationRequest aRequest (*mpRequestQueue->begin());

        // Check if the request should really be processed right now.
        // Reasons to not do it are when its cost is high and not many other
        // requests have been inserted into the queue that would otherwise
        // be processed first.
        if (aRequest.mnPriority < snWaitForMoreRequestsPriorityThreshold
            && (mnRequestsServedCount+mpRequestQueue->size() < snWaitForMoreRequestsCount))
        {
            // Wait for more requests before this one is processed.  Note
            // that the queue processing is not started anew when this
            // method is left.  That is done when the next request is
            // inserted.
            bWaitForMoreRequests = true;
            break;
        }

        mpRequestQueue->erase(mpRequestQueue->begin());

        if (aRequest.mpDescriptor.get() != NULL)
        {
            mnRequestsServedCount += 1;
            if ( ! mpWeakContainer.expired())
            {
                ::boost::shared_ptr<ContainerAdapter> pContainer (mpWeakContainer);
                if (pContainer.get() != NULL)
                    pContainer->UpdateDescriptor(aRequest.mpDescriptor,false,true,true);
            }
        }
    }
    while (false);

    if (mpRequestQueue->size() > 0 && ! bWaitForMoreRequests)
    {
        int nTimeout (snDelayedCreationTimeout);
        if (bIsShowingFullScreenShow)
            nTimeout = snDelayedCreationTimeoutWhenNotIdle;
        maDelayedPreviewCreationTimer.SetTimeout(nTimeout);
        pTimer->Start();
    }

    return 0;
}




bool MasterPageContainerQueue::HasRequest (MasterPageContainer::Token aToken) const
{
    RequestQueue::iterator iRequest (::std::find_if(
        mpRequestQueue->begin(),
        mpRequestQueue->end(),
        PreviewCreationRequest::CompareToken(aToken)));
    return (iRequest != mpRequestQueue->end());
}




bool MasterPageContainerQueue::IsEmpty (void) const
{
    return mpRequestQueue->empty();
}




void MasterPageContainerQueue::ProcessAllRequests (void)
{
    snWaitForMoreRequestsCount = 0;
    if (mpRequestQueue->size() > 0)
        maDelayedPreviewCreationTimer.Start();
}


} } } // end of namespace ::sd::toolpanel::controls

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
