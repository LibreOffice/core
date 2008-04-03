/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlsRequestQueue.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 14:21:37 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

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
            When this flag is <TRUE/> the request is inserted with highes
            priority in its class.  When <FALSE/> the request is inserted
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
            Returns <TRUE/> when the request has been successfully been
            removed from the queue.  Otherwise, e.g. because the request was
            not a member of the queue, <FALSE/> is returned.
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

    /** Returns <TRUE/> when there is no element in the queue.
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
