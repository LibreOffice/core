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

#include "MasterPageContainerQueue.hxx"
#include "MasterPageContainerProviders.hxx"

#include <tools/IdleDetection.hxx>
#include <PreviewRenderer.hxx>

#include <set>

namespace sd { namespace sidebar {

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
    SharedMasterPageDescriptor const mpDescriptor;
    int const mnPriority;
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
        MasterPageContainer::Token const maToken;
        explicit CompareToken(MasterPageContainer::Token aToken) : maToken(aToken) {}
        bool operator() (const PreviewCreationRequest& rRequest) const
            { return maToken==rRequest.mpDescriptor->maToken; }
    };
};

//===== MasterPageContainerQueue::RequestQueue ================================

class MasterPageContainerQueue::RequestQueue
    : public ::std::set<PreviewCreationRequest,PreviewCreationRequest::Compare>
{
public:
    RequestQueue() {}
};

//===== MasterPageContainerQueue ==============================================

MasterPageContainerQueue* MasterPageContainerQueue::Create (
    const std::weak_ptr<ContainerAdapter>& rpContainer)
{
    MasterPageContainerQueue* pQueue = new MasterPageContainerQueue(rpContainer);
    pQueue->LateInit();
    return pQueue;
}

MasterPageContainerQueue::MasterPageContainerQueue (
    const std::weak_ptr<ContainerAdapter>& rpContainer)
    : mpWeakContainer(rpContainer),
      mpRequestQueue(new RequestQueue()),
      maDelayedPreviewCreationTimer(),
      mnRequestsServedCount(0)
{
}

MasterPageContainerQueue::~MasterPageContainerQueue()
{
    maDelayedPreviewCreationTimer.Stop();
    while ( ! mpRequestQueue->empty())
        mpRequestQueue->erase(mpRequestQueue->begin());
}

void MasterPageContainerQueue::LateInit()
{
    // Set up the timer for the delayed creation of preview bitmaps.
    maDelayedPreviewCreationTimer.SetTimeout (snDelayedCreationTimeout);
    maDelayedPreviewCreationTimer.SetInvokeHandler(
        LINK(this,MasterPageContainerQueue,DelayedPreviewCreation) );
}

bool MasterPageContainerQueue::RequestPreview (const SharedMasterPageDescriptor& rpDescriptor)
{
    bool bSuccess (false);
    if (rpDescriptor.get() != nullptr
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
    const SharedMasterPageDescriptor& rpDescriptor)
{
    sal_Int32 nPriority;

    // The cost is used as a starting value.
    int nCost (0);
    if (rpDescriptor->mpPreviewProvider != nullptr)
    {
        nCost = rpDescriptor->mpPreviewProvider->GetCostIndex();
        if (rpDescriptor->mpPreviewProvider->NeedsPageObject())
            if (rpDescriptor->mpPageObjectProvider != nullptr)
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

IMPL_LINK(MasterPageContainerQueue, DelayedPreviewCreation, Timer*, pTimer, void)
{
    bool bIsShowingFullScreenShow (false);
    bool bWaitForMoreRequests (false);

    do
    {
        if (mpRequestQueue->empty())
            break;

        // First check whether the system is idle.
        tools::IdleState nIdleState (tools::IdleDetection::GetIdleState(nullptr));
        if (nIdleState != tools::IdleState::Idle)
        {
            if (nIdleState & tools::IdleState::FullScreenShowActive)
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

        if (aRequest.mpDescriptor.get() != nullptr)
        {
            mnRequestsServedCount += 1;
            if ( ! mpWeakContainer.expired())
            {
                std::shared_ptr<ContainerAdapter> pContainer (mpWeakContainer);
                if (pContainer != nullptr)
                    pContainer->UpdateDescriptor(aRequest.mpDescriptor,false,true,true);
            }
        }
    }
    while (false);

    if (!mpRequestQueue->empty() && ! bWaitForMoreRequests)
    {
        int nTimeout (snDelayedCreationTimeout);
        if (bIsShowingFullScreenShow)
            nTimeout = snDelayedCreationTimeoutWhenNotIdle;
        maDelayedPreviewCreationTimer.SetTimeout(nTimeout);
        pTimer->Start();
    }
}

bool MasterPageContainerQueue::HasRequest (MasterPageContainer::Token aToken) const
{
    return std::any_of(
        mpRequestQueue->begin(),
        mpRequestQueue->end(),
        PreviewCreationRequest::CompareToken(aToken));
}

bool MasterPageContainerQueue::IsEmpty() const
{
    return mpRequestQueue->empty();
}

void MasterPageContainerQueue::ProcessAllRequests()
{
    snWaitForMoreRequestsCount = 0;
    if (!mpRequestQueue->empty())
        maDelayedPreviewCreationTimer.Start();
}

} } // end of namespace sd::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
