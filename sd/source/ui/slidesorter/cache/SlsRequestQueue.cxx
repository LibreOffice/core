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

#include "precompiled_sd.hxx"

#include "SlsRequestQueue.hxx"

#include <set>


#undef VERBOSE
//#define VERBOSE

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
        {
            if (rRequest1.meClass == rRequest2.meClass)
                return (rRequest1.mnPriorityInClass > rRequest2.mnPriorityInClass);
            else
                return (rRequest1.meClass < rRequest2.meClass);
        }
    };
    /** Request data is compared arbitrarily by their addresses in memory.
        This just establishes an order so that the STL containers are happy.
        The order is not semantically interpreted.
    */
    class DataComparator { public:
        DataComparator (const Request&rRequest):maKey(rRequest.maKey){}
        DataComparator (const CacheKey aKey):maKey(aKey){}
        bool operator() (const Request& rRequest) { return maKey == rRequest.maKey; }
    private: const CacheKey maKey;
    };

    CacheKey maKey;
    sal_Int32 mnPriorityInClass;
    RequestPriorityClass meClass;
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
      mpRequestQueue(new Container()),
      mpCacheContext(rpCacheContext),
      mnMinimumPriority(0),
      mnMaximumPriority(1)
{
}




RequestQueue::~RequestQueue (void)
{
}




void RequestQueue::AddRequest (
    CacheKey aKey,
    RequestPriorityClass eRequestClass,
    bool /*bInsertWithHighestPriority*/)
{
    ::osl::MutexGuard aGuard (maMutex);

    OSL_ASSERT(eRequestClass>=MIN__CLASS && eRequestClass<=MAX__CLASS);

    // If the request is already a member of the queue then remove it so
    // that the following insertion will use the new prioritization.
#ifdef VERBOSE
    bool bRemoved =
#endif
        RemoveRequest(aKey);

    // The priority of the request inside its priority class is defined by
    // the page number.  This ensures a strict top-to-bottom, left-to-right
    // order.
    sal_Int32 nPriority (mpCacheContext->GetPriority(aKey));
    Request aRequest (aKey, nPriority, eRequestClass);
    mpRequestQueue->insert(aRequest);

    SSCD_SET_REQUEST_CLASS(rRequestData.GetPage(),eRequestClass);

#ifdef VERBOSE
    OSL_TRACE("%s request for page %d with priority class %d",
        bRemoved?"replaced":"added",
        (rRequestData.GetPage()->GetPageNum()-1)/2,
        eRequestClass);
#endif
}




bool RequestQueue::RemoveRequest (
    CacheKey aKey)
{
    bool bRequestWasRemoved (false);
    ::osl::MutexGuard aGuard (maMutex);

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
            mpRequestQueue->erase(aRequestIterator);
            bRequestWasRemoved = true;

            if (bRequestWasRemoved)
            {
                SSCD_SET_STATUS(rRequest.GetPage(),NONE);
            }
        }
        else
            break;
    }

    return bRequestWasRemoved;
}




void RequestQueue::ChangeClass (
    CacheKey aKey,
    RequestPriorityClass eNewRequestClass)
{
    ::osl::MutexGuard aGuard (maMutex);

    OSL_ASSERT(eNewRequestClass>=MIN__CLASS && eNewRequestClass<=MAX__CLASS);

    Container::const_iterator iRequest (
        ::std::find_if (
            mpRequestQueue->begin(),
            mpRequestQueue->end(),
            Request::DataComparator(aKey)));
    if (iRequest!=mpRequestQueue->end() && iRequest->meClass!=eNewRequestClass)
    {
        AddRequest(aKey, eNewRequestClass, true);
        SSCD_SET_REQUEST_CLASS(rRequestData.GetPage(),eNewRequestClass);
    }
}




CacheKey RequestQueue::GetFront (void)
{
    ::osl::MutexGuard aGuard (maMutex);

    if (mpRequestQueue->empty())
        throw ::com::sun::star::uno::RuntimeException(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                "RequestQueue::GetFront(): queue is empty")),
            NULL);

    return mpRequestQueue->begin()->maKey;
}




RequestPriorityClass RequestQueue::GetFrontPriorityClass (void)
{
    ::osl::MutexGuard aGuard (maMutex);

    if (mpRequestQueue->empty())
        throw ::com::sun::star::uno::RuntimeException(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                "RequestQueue::GetFrontPriorityClass(): queue is empty")),
            NULL);

    return mpRequestQueue->begin()->meClass;
}




void RequestQueue::PopFront (void)
{
    ::osl::MutexGuard aGuard (maMutex);

    if ( ! mpRequestQueue->empty())
    {
        SSCD_SET_STATUS(maRequestQueue.begin()->mpData->GetPage(),NONE);

        mpRequestQueue->erase(mpRequestQueue->begin());

        // Reset the priority counter if possible.
        if (mpRequestQueue->empty())
        {
            mnMinimumPriority = 0;
            mnMaximumPriority = 1;
        }
    }
}




bool RequestQueue::IsEmpty (void)
{
    ::osl::MutexGuard aGuard (maMutex);
    return mpRequestQueue->empty();
}




void RequestQueue::Clear (void)
{
    ::osl::MutexGuard aGuard (maMutex);

    mpRequestQueue->clear();
    mnMinimumPriority = 0;
    mnMaximumPriority = 1;
}




::osl::Mutex& RequestQueue::GetMutex (void)
{
    return maMutex;
}


} } } // end of namespace ::sd::slidesorter::cache



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
