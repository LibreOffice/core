/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef SD_SLIDESORTER_REQUEST_QUEUE_HXX
#define SD_SLIDESORTER_REQUEST_QUEUE_HXX

#include "SlsRequestPriorityClass.hxx"
#include "cache/SlsCacheContext.hxx"
#include "taskpane/SlideSorterCacheDisplay.hxx"
#include <drawdoc.hxx>
#include "osl/mutex.hxx"


namespace sd { namespace slidesorter { namespace cache {

class RequestData;

/** The request queue stores requests that are described by the RequestData
    sorted according to priority class and then priority.
*/
class RequestQueue
{
public:
    RequestQueue (const SharedCacheContext& rpCacheContext);
    ~RequestQueue (void);

    /** Insert a request with highest or lowest priority in its priority
        class.  When the request is already present then it is first
        removed.  This effect is then a re-prioritization.
        @param rRequestData
            The request.
        @param eRequestClass
            The priority class in which to insert the request with highest
            or lowest priority.
        @param bInsertWithHighestPriority
            When this flag is <true/> the request is inserted with highes
            priority in its class.  When <false/> the request is inserted
            with lowest priority.
    */
    void AddRequest (
        CacheKey aKey,
        RequestPriorityClass eRequestClass,
        bool bInsertWithHighestPriority = false);

    /** Remove the specified request from the queue.
        @param rRequestData
            It is OK when the specified request is not a member of the
            queue.
        @return
            Returns <true/> when the request has been successfully been
            removed from the queue.  Otherwise, e.g. because the request was
            not a member of the queue, <false/> is returned.
    */
    bool RemoveRequest (CacheKey aKey);

    /** Change the priority class of the specified request.
    */
    void ChangeClass (
        CacheKey aKey,
        RequestPriorityClass eNewRequestClass);

    /** Get the request with the highest priority int the highest priority class.
    */
    CacheKey GetFront (void);

    // For debugging.
    RequestPriorityClass GetFrontPriorityClass (void);

    /** Really a synonym for RemoveRequest(GetFront());
    */
    void PopFront (void);

    /** Returns <true/> when there is no element in the queue.
    */
    bool IsEmpty (void);

    /** Remove all requests from the queue.  This resets the minimum and
        maximum priorities to their default values.
    */
    void Clear (void);

    /** Return the mutex that guards the access to the priority queue.
    */
    ::osl::Mutex& GetMutex (void);

private:
    ::osl::Mutex maMutex;
    class Container;
    ::boost::scoped_ptr<Container> mpRequestQueue;
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
