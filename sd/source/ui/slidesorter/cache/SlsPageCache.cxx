/*************************************************************************
 *
 *  $RCSfile: SlsPageCache.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:10:46 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

/** All classes and template classes in this file belong to the preview
    bitmap cache.  Its tasks are the creation and caching of preview bitmaps
    for page objects.  The template class GenericPageCache is the facade
    whose specialization will be visible to the using class.  Here is a list
    of all collaborating classes.  Note that this is a work in progress.
    The division into the classes is the first step of identifiying and
    separation of the responsibilities.  Especially the choices of making a
    class a template or not was made ad hoc and will likely change.

    - BitmapCache
    The low level cache of bitmaps.  Apart from storing bitmaps and keeping
    track of how much memory they use the BitmapCache class will have the
    responsibility of destroying bitmaps when it runs out of space.

    - template <class Queue, bool UseAheadOfTimeRequests>
    class RequestFactory
    This function object class is responsible for creating requests after a
    model change when all preview bitmaps become invalid.  It can be used to
    generate ahead-of-time requests.

    - class BitmapFactory
    The static class creates preview bitmaps for page objects.

    - template<class RequestData>
    class Request
    This simple class bundles the data that are necessary for the queue to
    manage the requests.  Apart from the RequestData, typically a
    view-object-contact object, that are the priority class and the priority
    inside that class.

    - template<class RequestData>
    class RequestDataComparator
    This function object compares request objects according to their request
    data (pointer).  It is used for finding a request object that is
    associated with a given request data.

    - template<class RequestData>
    class RequestComparator
    This function object compares request objects according to their
    priority.  Objects in a lower class (0) are processed earlier then
    objects in a higher class (1).  When two objects belong to the same
    priority class they are ordered according to their priority-in-class
    value.  The function object is used to maintain the order of request
    objects in a set data structure.

    - template<class RequestData, int ClassCount>
    class GenericRequestQueue
    The request queue maintains a priority queue where each request for the
    generation of a preview bitmap has a priority that consists of two
    values: The priority class that is a rough classification that allows
    the distinction of requests for visible and for non-visible page
    objects.  The second value is the priority inside that class.

    - class QueueProcessorBase
    This class exists because the IMPL_LINK macro can not handle a template
    class.

    - template <class Queue, class RequestData, class BitmapCache,
    class BitmapFactory>
    class QueueProcessor
    The queue processor is responsible for processing the requests in the
    request queue.  In this class this is done asynchronously with a timer.
    Alternative implementations may use threads or can even process requests
    synchronously.

    - template<class RequestData,
    class CreationManager,
    class BitmapCache,
    class RequestQueue,
    class QueueProcessor>
    class GenericPageCache
    The main class in this file uses all the ones above to implement a) a cache
    and b) a factory of preview bitmaps.
*/

#include "cache/SlsPageCache.hxx"

#include "SlsBitmapCache.hxx"
#include "SlsGenericPageCache.hxx"
#include "SlsGenericRequestQueue.hxx"
#include "SlsRequestFactory.hxx"
#include "SlsCacheCompactor.hxx"
#ifdef USE_THREAD
#include "SlsQueueProcessorThread.hxx"
#else
#include "SlsQueueProcessor.hxx"
#endif
#include "SlsPreviewBitmapFactory.hxx"

using namespace ::sd::slidesorter::model;
using namespace ::sd::slidesorter::view;

namespace sd { namespace slidesorter { namespace cache {

typedef GenericRequestQueue<PageCache::RequestData,2> Queue;


#ifdef USE_THREAD
typedef QueueProcessorThread<
    Queue,
    PageCache::RequestData,
    BitmapCache,
    PreviewBitmapFactory> Processor;
#else
typedef QueueProcessor<
    Queue,
    PageCache::RequestData,
    BitmapCache,
    PreviewBitmapFactory> Processor;
#endif

typedef GenericPageCache<
    PageCache::RequestData,
    RequestFactory<Queue,false>,
    BitmapCache,
    Queue,
    CompactionByReduction,
    Processor
    > BaseClass;


class PageCache::PageCacheImplementation
    : public BaseClass
{
public:
    PageCacheImplementation (
        view::SlideSorterView& rView,
        model::SlideSorterModel& rModel,
        sal_Int32 nMaximalCacheSize)
        : BaseClass (rView,rModel, nMaximalCacheSize)
    {}
};


PageCache::PageCache (
    view::SlideSorterView& rView,
    model::SlideSorterModel& rModel,
    sal_Int32 nMaximalCacheSize)
    : mpImplementation (::std::auto_ptr<PageCacheImplementation>(
        new PageCacheImplementation(rView,rModel,nMaximalCacheSize)))
{
}




PageCache::~PageCache (void)
{
}




BitmapEx PageCache::GetPreviewBitmap (
    RequestData& rRequestData,
    const Size& rSize)
{
    return mpImplementation->GetPreviewBitmap (rRequestData, rSize);
}




void PageCache::RequestPreviewBitmap (
    RequestData& rRequestData,
    const Size& rSize)
{
    return mpImplementation->RequestPreviewBitmap (rRequestData, rSize);
}




void PageCache::InvalidatePreviewBitmap (const RequestData& rRequestData)
{
    mpImplementation->InvalidatePreviewBitmap (rRequestData);
}




void PageCache::DecreaseRequestPriority (RequestData& rRequestData)
{
    mpImplementation->DecreaseRequestPriority(rRequestData);
}




void PageCache::IncreaseRequestPriority (RequestData& rRequestData)
{
    mpImplementation->IncreaseRequestPriority(rRequestData);
}




void PageCache::ReleasePreviewBitmap (RequestData& rRequestData)
{
    mpImplementation->ReleasePreviewBitmap(rRequestData);
}




void PageCache::InvalidateCache (void)
{
    mpImplementation->InvalidateCache();
}




void PageCache::SetPreciousFlag (RequestData& rRequestData, bool bIsPrecious)
{
    mpImplementation->SetPreciousFlag (rRequestData, bIsPrecious);
}


} } } // end of namespace ::sd::slidesorter::cache
