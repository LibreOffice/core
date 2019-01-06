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

#ifndef INCLUDED_SD_SOURCE_UI_SLIDESORTER_CACHE_SLSREQUESTQUEUE_HXX
#define INCLUDED_SD_SOURCE_UI_SLIDESORTER_CACHE_SLSREQUESTQUEUE_HXX

#include "SlsRequestPriorityClass.hxx"
#include <cache/SlsCacheContext.hxx>
#include <osl/mutex.hxx>
#include <svx/sdrpageuser.hxx>

#include <memory>

namespace sd { namespace slidesorter { namespace cache {

/** The request queue stores requests that are described by the Request
    sorted according to priority class and then priority.
*/
class RequestQueue : public sdr::PageUser
{
public:
    explicit RequestQueue (const SharedCacheContext& rpCacheContext);
    virtual ~RequestQueue();

    /** Insert a request with highest or lowest priority in its priority
        class.  When the request is already present then it is first
        removed.  This effect is then a re-prioritization.
        @param aKey
            The request.
        @param eRequestClass
            The priority class in which to insert the request with highest
            or lowest priority.
        @param bInsertWithHighestPriority
            When this flag is <TRUE/> the request is inserted with highest
            priority in its class.  When <FALSE/> the request is inserted
            with lowest priority.
    */
    void AddRequest (
        CacheKey aKey,
        RequestPriorityClass eRequestClass);

    /** Remove the specified request from the queue.
        @param aKey
            It is OK when the specified request is not a member of the
            queue.
    */
#if OSL_DEBUG_LEVEL >=2
bool
#else
void
#endif
    RemoveRequest (CacheKey aKey);

    /** Change the priority class of the specified request.
    */
    void ChangeClass (
        CacheKey aKey,
        RequestPriorityClass eNewRequestClass);

    /** Get the request with the highest priority int the highest priority class.
    */
    CacheKey GetFront();

    // For debugging.
    RequestPriorityClass GetFrontPriorityClass();

    /** Really a synonym for RemoveRequest(GetFront());
    */
    void PopFront();

    /** Returns <TRUE/> when there is no element in the queue.
    */
    bool IsEmpty();

    /** Remove all requests from the queue.  This resets the minimum and
        maximum priorities to their default values.
    */
    void Clear();

    /** Return the mutex that guards the access to the priority queue.
    */
    ::osl::Mutex& GetMutex() { return maMutex;}

    /** Ensure we don't hand out a page deleted before anyone got a
        chance to process it
    */
    virtual void PageInDestruction(const SdrPage& rPage) override;

private:
    ::osl::Mutex maMutex;
    class Container;
    std::unique_ptr<Container> mpRequestQueue;
    SharedCacheContext mpCacheContext;

    /** A lower bound of the lowest priority of all elements in the queues.
        The start value is 0.  It is assigned and then decreased every time
        when an element is inserted or marked as the request with lowest
        priority.
    */
    int mnMinimumPriority;
    /** An upper bound of the highest priority of all elements in the queues.
        The start value is 1.  It is assigned and then increased every time
        when an element is inserted or marked as the request with highest
        priority.
    */
    int mnMaximumPriority;
};

} } } // end of namespace ::sd::slidesorter::cache

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
