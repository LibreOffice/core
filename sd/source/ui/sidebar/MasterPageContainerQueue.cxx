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

#include "MasterPageContainerQueue.hxx"

#include "tools/IdleDetection.hxx"

#include <set>

namespace sd { namespace sidebar {

const sal_Int32 MasterPageContainerQueue::snDelayedCreationTimeout (15);
const sal_Int32 MasterPageContainerQueue::snDelayedCreationTimeoutWhenNotIdle (100);
const sal_Int32 MasterPageContainerQueue::snMasterPagePriorityBoost (5);
const sal_Int32 MasterPageContainerQueue::snWaitForMoreRequestsPriorityThreshold (-10);
sal_uInt32 MasterPageContainerQueue::snWaitForMoreRequestsCount(15);



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
                
                return r1.mnPriority > r2.mnPriority;
            }
            else
            {
                
                
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






class MasterPageContainerQueue::RequestQueue
    : public ::std::set<PreviewCreationRequest,PreviewCreationRequest::Compare>
{
public:
    RequestQueue (void) {}
};






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

        
        RequestQueue::iterator iRequest (::std::find_if(
            mpRequestQueue->begin(),
            mpRequestQueue->end(),
            PreviewCreationRequest::CompareToken(rpDescriptor->maToken)));
        
        
        if (iRequest != mpRequestQueue->end())
            if (iRequest->mnPriority < nPriority)
            {
                mpRequestQueue->erase(iRequest);
                iRequest = mpRequestQueue->end();
            }

        
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

    
    int nCost (0);
    if (rpDescriptor->mpPreviewProvider.get() != NULL)
    {
        nCost = rpDescriptor->mpPreviewProvider->GetCostIndex();
        if (rpDescriptor->mpPreviewProvider->NeedsPageObject())
            if (rpDescriptor->mpPageObjectProvider.get() != NULL)
                nCost += rpDescriptor->mpPageObjectProvider->GetCostIndex();
    }

    
    
    nPriority = -nCost;

    
    
    nPriority -= rpDescriptor->maToken / 3;

    
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
        if (mpRequestQueue->empty())
            break;

        
        sal_Int32 nIdleState (tools::IdleDetection::GetIdleState());
        if (nIdleState != tools::IdleDetection::IDET_IDLE)
        {
            if ((nIdleState&tools::IdleDetection::IDET_FULL_SCREEN_SHOW_ACTIVE) != 0)
                bIsShowingFullScreenShow = true;
            break;
        }

        PreviewCreationRequest aRequest (*mpRequestQueue->begin());

        
        
        
        
        if (aRequest.mnPriority < snWaitForMoreRequestsPriorityThreshold
            && (mnRequestsServedCount+mpRequestQueue->size() < snWaitForMoreRequestsCount))
        {
            
            
            
            
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


} } 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
