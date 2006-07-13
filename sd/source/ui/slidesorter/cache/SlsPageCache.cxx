/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlsPageCache.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-13 10:29:14 $
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

#include "cache/SlsPageCache.hxx"
#include "cache/SlsPageCacheManager.hxx"
#include "SlsBitmapCache.hxx"
#include "SlsGenericPageCache.hxx"
#include "SlsGenericRequestQueue.hxx"
#include "SlsRequestFactory.hxx"
#include "SlsQueueProcessor.hxx"
#include "SlsPreviewBitmapFactory.hxx"


using namespace ::sd::slidesorter::model;
using namespace ::sd::slidesorter::view;
using namespace ::com::sun::star;


namespace sd { namespace slidesorter { namespace cache {


typedef GenericRequestQueue<PageCache::RequestData> Queue;
typedef QueueProcessor<Queue, PageCache::RequestData, PreviewBitmapFactory> Processor;
typedef GenericPageCache< PageCache::RequestData, RequestFactory<Queue,false>, Queue, Processor
    > BaseClass;


/** The implementation class simply hides the actual GenericPageCache base
    class.
*/
class PageCache::PageCacheImplementation
    : public BaseClass
{
public:
    PageCacheImplementation (
        view::SlideSorterView& rView,
        model::SlideSorterModel& rModel,
        const Size& rPreviewSize)
        : BaseClass (rView,rModel, rPreviewSize)
    {}
};




//===== PageCache =============================================================

PageCache::PageCache (
    view::SlideSorterView& rView,
    model::SlideSorterModel& rModel,
    const Size& rPreviewSize)
    : mpImplementation(NULL)
{
    mpImplementation.reset(new PageCacheImplementation(rView,rModel,rPreviewSize));
}




PageCache::~PageCache (void)
{
}




void PageCache::ChangeSize(const Size& rPreviewSize)
{
    mpImplementation->ChangePreviewSize(rPreviewSize);
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




void PageCache::ReleasePreviewBitmap (RequestData& rRequestData)
{
    mpImplementation->ReleasePreviewBitmap(rRequestData);
}




void PageCache::InvalidateCache (bool bUpdateCache)
{
    mpImplementation->InvalidateCache(bUpdateCache);
}




void PageCache::SetPreciousFlag (RequestData& rRequestData, bool bIsPrecious)
{
    mpImplementation->SetPreciousFlag (rRequestData, bIsPrecious);
}




} } } // end of namespace ::sd::slidesorter::cache
