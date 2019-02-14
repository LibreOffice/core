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

#include "SlsRequestQueue.hxx"
#include <sdpage.hxx>

#include <sal/log.hxx>

#include <set>

namespace sd { namespace slidesorter { namespace cache {

/** This class extends the actual request data with additional information
    that is used by the priority queues.
*/
class Request
{
public:
    Request (
        CacheKey aKey, sal_Int32 nPriority, RequestPriorityClass eClass)
        : maKey(aKey), mnPriorityInClass(nPriority), meClass(eClass)
    {}
    /** Sort requests according to priority classes and then to priorities.
    */
    class Comparator { public:
        bool operator() (const Request& rRequest1, const Request& rRequest2)
            const
        {
            if (rRequest1.meClass == rRequest2.meClass)
            {
                if (rRequest1.mnPriorityInClass == rRequest2.mnPriorityInClass)
                {
                    return rRequest1.maKey < rRequest2.maKey;
                }
                return rRequest1.mnPriorityInClass > rRequest2.mnPriorityInClass;
            }
            return rRequest1.meClass < rRequest2.meClass;
        }
    };
    /** Request data is compared arbitrarily by their addresses in memory.
        This just establishes an order so that the STL containers are happy.
        The order is not semantically interpreted.
    */
    class DataComparator
    {
    public:
        explicit DataComparator (const CacheKey aKey)
            : maKey(aKey)
        {
        }
        bool operator() (const Request& rRequest) const
        {
            return maKey == rRequest.maKey;
        }
    private:
        const CacheKey maKey;
    };

    CacheKey const maKey;
    sal_Int32 const mnPriorityInClass;
    RequestPriorityClass const meClass;
};

class RequestQueue::Container
    : public ::std::set<
        Request,
        Request::Comparator>
{
};

//=====  GenericRequestQueue  =================================================

RequestQueue::RequestQueue (const SharedCacheContext& rpCacheContext)
    : maMutex(),
      mpRequestQueue(new Container),
      mpCacheContext(rpCacheContext),
      mnMinimumPriority(0),
      mnMaximumPriority(1)
{
}

RequestQueue::~RequestQueue()
{
    Clear();
}

void RequestQueue::AddRequest (
    CacheKey aKey,
    RequestPriorityClass eRequestClass)
{
    ::osl::MutexGuard aGuard (maMutex);

    assert(eRequestClass>=MIN_CLASS && eRequestClass<=MAX_CLASS);

    // If the request is already a member of the queue then remove it so
    // that the following insertion will use the new prioritization.
#if OSL_DEBUG_LEVEL >=2
    bool bRemoved =
#endif
    RemoveRequest(aKey);

    // The priority of the request inside its priority class is defined by
    // the page number.  This ensures a strict top-to-bottom, left-to-right
    // order.
    sal_Int32 nPriority (mpCacheContext->GetPriority(aKey));
    Request aRequest (aKey, nPriority, eRequestClass);

    std::pair<Container::iterator,bool> ret = mpRequestQueue->insert(aRequest);
    bool bInserted = ret.second;

    if (bInserted)
    {
        SdrPage *pPage = const_cast<SdrPage*>(aRequest.maKey);
        pPage->AddPageUser(*this);
    }

#if OSL_DEBUG_LEVEL >=2
    SAL_INFO("sd.sls", OSL_THIS_FUNC << ": " << (bRemoved?"replaced":"added")
        << " request for page " << ((aKey->GetPageNum()-1)/2)
        << " with priority class " << static_cast<int>(eRequestClass));
#endif
}

void RequestQueue::PageInDestruction(const SdrPage& rPage)
{
    //remove any requests pending for this page which is going away now
    RemoveRequest(&rPage);
}

#if OSL_DEBUG_LEVEL >=2
bool
#else
void
#endif
RequestQueue::RemoveRequest(
    CacheKey aKey)
{
    ::osl::MutexGuard aGuard (maMutex);
#if OSL_DEBUG_LEVEL >=2
    bool bIsRemoved = false;
#endif
    while(true)
    {
        Container::const_iterator aRequestIterator = ::std::find_if (
            mpRequestQueue->begin(),
            mpRequestQueue->end(),
            Request::DataComparator(aKey));
        if (aRequestIterator != mpRequestQueue->end())
        {
            if (aRequestIterator->mnPriorityInClass == mnMinimumPriority+1)
                mnMinimumPriority++;
            else if (aRequestIterator->mnPriorityInClass == mnMaximumPriority-1)
                mnMaximumPriority--;

            SdrPage *pPage = const_cast<SdrPage*>(aRequestIterator->maKey);
            pPage->RemovePageUser(*this);
            mpRequestQueue->erase(aRequestIterator);
#if OSL_DEBUG_LEVEL >=2
            bIsRemoved = true;
#endif
        }
        else
            break;
    }
#if OSL_DEBUG_LEVEL >=2
    return bIsRemoved;
#endif

}

void RequestQueue::ChangeClass (
    CacheKey aKey,
    RequestPriorityClass eNewRequestClass)
{
    ::osl::MutexGuard aGuard (maMutex);

    assert(eNewRequestClass>=MIN_CLASS && eNewRequestClass<=MAX_CLASS);

    Container::const_iterator iRequest (
        ::std::find_if (
            mpRequestQueue->begin(),
            mpRequestQueue->end(),
            Request::DataComparator(aKey)));
    if (iRequest!=mpRequestQueue->end() && iRequest->meClass!=eNewRequestClass)
    {
        AddRequest(aKey, eNewRequestClass);
    }
}

CacheKey RequestQueue::GetFront()
{
    ::osl::MutexGuard aGuard (maMutex);

    if (mpRequestQueue->empty())
        throw css::uno::RuntimeException("RequestQueue::GetFront(): queue is empty",
            nullptr);

    return mpRequestQueue->begin()->maKey;
}

RequestPriorityClass RequestQueue::GetFrontPriorityClass()
{
    ::osl::MutexGuard aGuard (maMutex);

    if (mpRequestQueue->empty())
        throw css::uno::RuntimeException("RequestQueue::GetFrontPriorityClass(): queue is empty",
            nullptr);

    return mpRequestQueue->begin()->meClass;
}

void RequestQueue::PopFront()
{
    ::osl::MutexGuard aGuard (maMutex);

    if (  mpRequestQueue->empty())
        return;

    Container::const_iterator aIter(mpRequestQueue->begin());
    SdrPage *pPage = const_cast<SdrPage*>(aIter->maKey);
    pPage->RemovePageUser(*this);
    mpRequestQueue->erase(aIter);

    // Reset the priority counter if possible.
    if (mpRequestQueue->empty())
    {
        mnMinimumPriority = 0;
        mnMaximumPriority = 1;
    }
}

bool RequestQueue::IsEmpty()
{
    ::osl::MutexGuard aGuard (maMutex);
    return mpRequestQueue->empty();
}

void RequestQueue::Clear()
{
    ::osl::MutexGuard aGuard (maMutex);

    for (const auto& rItem : *mpRequestQueue)
    {
        SdrPage *pPage = const_cast<SdrPage*>(rItem.maKey);
        pPage->RemovePageUser(*this);
    }

    mpRequestQueue->clear();
    mnMinimumPriority = 0;
    mnMaximumPriority = 1;
}

} } } // end of namespace ::sd::slidesorter::cache

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
