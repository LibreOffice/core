/*************************************************************************
 *
 *  $RCSfile: SlsGenericPageCache.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:10:17 $
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

#ifndef SD_SLIDESORTER_GENERIC_PAGE_CACHE_HXX
#define SD_SLIDESORTER_GENERIC_PAGE_CACHE_HXX

#include "SlsQueueProcessor.hxx"
#include "SlsPreviewBitmapFactory.hxx"
#include "view/SlsPageObjectViewObjectContact.hxx"


namespace sd { namespace slidesorter { namespace model {
class SlideSorterModel;
} } }

namespace sd { namespace slidesorter { namespace view {
class SlideSorterView;
} } }

namespace sd { namespace slidesorter { namespace cache {


template<class RequestData,
         class CreationManager,
         class BitmapCache,
         class RequestQueue,
         class CacheCompactionPolicy,
         class QueueProcessor>
class GenericPageCache
{
public:
    /** The page chache is created with references both to the view and the
        model so that it can fill itself with requests for all or just the
        visible pages.
    */
    GenericPageCache (
        view::SlideSorterView& rView,
        model::SlideSorterModel& rModel,
        sal_Int32 nMaximalCacheSize);

    ~GenericPageCache (void);

    /** Request a preview bitmap for the specified page object in the
        specified size.  The returned bitmap may be preview of the preview,
        i.e. either a scaled (up or down) version of a previous preview (of
        the wrong size) or an empty bitmap.  In this case a request for the
        generation of a new preview is created and inserted into the request
        queue.  When the preview is available the page shape will be told to
        paint itself again.  When it then calls this method again if
        receives the correctly sized preview bitmap.
        @param rRequestData
            This data is used to determine the preview.
        @param rSize
            The size of the requested preview bitmap.
        @return
            Returns a bitmap that is either empty, contains a scaled (up or
            down) version or is the requested bitmap.
    */
    BitmapEx GetPreviewBitmap (
        RequestData& rRequestData,
        const Size& rSize);

    /** When the requested preview bitmap does not yet exist or is not
        up-to-date then the rendering of one is scheduled.  Otherwise this
        method does nothing.
    */
    void RequestPreviewBitmap (
        RequestData& rRequestData,
        const Size& rSize);

    /** Tell the cache to replace the bitmap associated with the given
        request data with a new one that reflects recent changes in the
        content of the page object.
    */
    void InvalidatePreviewBitmap (const RequestData& rRequestData);

    /** Lower the priority with which the request associated with the given
        data will be processed.  Call this method when the visibility of a
        page object changes (from visible to not visible) and the request
        becomes a ahead-of-time request. When the request is already in the
        lowest class it will be removed.
    */
    void DecreaseRequestPriority (RequestData& rRequestData);

    /** Move the request associated with the given data into a higher
        priority class and increase its priority in that class above all
        other elements in the class.
    */
    void IncreaseRequestPriority (RequestData& rRequestData);

    /** Call this method when a view-object-contact object is being deleted
        and does not need (a) its current bitmap in the cache and (b) a
        requested new bitmap.
    */
    void ReleasePreviewBitmap (RequestData& rRequestData);

    /** Call this method when all preview bitmaps have to be generated anew.
        This is the case when the size of the page objects on the screen has
        changed or when the model has changed.
    */
    void InvalidateCache (void);

    /** With the precious flag you can control whether a bitmap can be
        removed or reduced in size to make room for other bitmaps or is so
        precious that it will not touched.  A typical use is to set the
        precious flag for exactly the visible pages.
    */
    void SetPreciousFlag (RequestData& rRequestData, bool bIsPrecious);

private:
    view::SlideSorterView& mrView;

    model::SlideSorterModel& mrModel;

    BitmapCache maBitmapCache;

    RequestQueue maRequestQueue;

    QueueProcessor* mpQueueProcessor;

    const sal_Int32 mnMaximalCacheSize;

    /** Remember whether the cache limit has been reached at least once
        after a Clear() call.  This is important because afterwards the
        cache will be constantly at its limit of capacity.  Therefore
        requests with another than the highest priority class will not be
        processed, the resulting preview bitmaps would be removed shortly
        afterwards.
    */
    bool mbLimitHasBeenReached;
};




//=====  GenericPageCache =====================================================

template<class RequestData,
         class CreationManager,
         class BitmapCache,
         class RequestQueue,
         class CompactionPolicy,
         class QueueProcessor>
GenericPageCache<
    RequestData,
    CreationManager,
    BitmapCache,
    RequestQueue,
    CompactionPolicy, QueueProcessor
    >::GenericPageCache (
        view::SlideSorterView& rView,
        model::SlideSorterModel& rModel,
        sal_Int32 nMaximalCacheSize)
        : mrView(rView),
          mrModel(rModel),
          maBitmapCache (),
          maRequestQueue(),
          mpQueueProcessor(
              new QueueProcessor(mrView,maRequestQueue,maBitmapCache)),
          mnMaximalCacheSize(nMaximalCacheSize),
          mbLimitHasBeenReached (false)
{
}




template<class RequestData,
         class CreationManager,
         class BitmapCache,
         class RequestQueue,
         class CompactionPolicy,
         class QueueProcessor>
GenericPageCache<
    RequestData, CreationManager, BitmapCache, RequestQueue,
    CompactionPolicy, QueueProcessor
    >::~GenericPageCache (void)
{
    OSL_TRACE("terminating thread %p", mpQueueProcessor);
    mpQueueProcessor->Stop();
    maRequestQueue.Clear();
    mpQueueProcessor->Terminate();
    //    delete mpQueueProcessor;
    OSL_TRACE("thread %p stopped and terminated", mpQueueProcessor);
}




template<class RequestData,
         class CreationManager,
         class BitmapCache,
         class RequestQueue,
         class CompactionPolicy,
         class QueueProcessor>
BitmapEx GenericPageCache<
    RequestData, CreationManager, BitmapCache, RequestQueue,
    CompactionPolicy, QueueProcessor
    >::GetPreviewBitmap (
        RequestData& rRequestData,
        const Size& rSize)
{
    BitmapEx aPreview (maBitmapCache.GetBitmap (rRequestData.GetPage()));
    Size aBitmapSize (aPreview.GetSizePixel());
    if (aBitmapSize != rSize)
    {
        OSL_ASSERT (rSize.Width() < 1000);
        // The bitmap has the wrong size.

        // Scale the bitmap to the desired size when that is possible,
        // i.e. the bitmap is not empty.
        if (aBitmapSize.Width() != 0 && aBitmapSize.Height())
            // BMP_SCALE_NONE               0x00000000UL
            // BMP_SCALE_FAST               0x00000001UL
            // BMP_SCALE_INTERPOLATE        0x00000002UL
            aPreview.Scale (rSize, BMP_SCALE_FAST);
    }

    // Request the creation of a correctly sized preview bitmap.  We do this
    // even when the size of the bitmap in the cache is correct because its
    // content may be not up-to-date anymore.
    RequestPreviewBitmap (rRequestData, rSize);

    return aPreview;
}




template<class RequestData,
         class CreationManager,
         class BitmapCache,
         class RequestQueue,
         class CompactionPolicy,
         class QueueProcessor>
void GenericPageCache<
    RequestData, CreationManager, BitmapCache, RequestQueue,
    CompactionPolicy, QueueProcessor
    >::RequestPreviewBitmap (
        RequestData& rRequestData,
        const Size& rSize)
{
    const SdrPage* pPage = rRequestData.GetPage();

    // Determine if the available bitmap is up to date.
    bool bIsUpToDate = maBitmapCache.BitmapIsUpToDate (pPage);
    if (bIsUpToDate)
    {
        BitmapEx aPreview (maBitmapCache.GetBitmap (pPage));
        if (aPreview.GetSizePixel() != rSize)
            bIsUpToDate = false;
    }

    if ( ! bIsUpToDate)
    {
        // No, the bitmap is not up-to-date.  Request a new one.
        maRequestQueue.InsertFrontRequest (
            rRequestData,
            rRequestData.GetPageDescriptor().IsVisible() ? 0 : 1);
        mpQueueProcessor->Start();
    }

    // Reduce the cache size if it grew too large.
    if (maBitmapCache.GetSize() > mnMaximalCacheSize)
    {
        mbLimitHasBeenReached = true;
        CompactionPolicy()(maBitmapCache, mnMaximalCacheSize);
    }
}




template<class RequestData,
         class CreationManager,
         class BitmapCache,
         class RequestQueue,
         class CompactionPolicy,
        class QueueProcessor>
void GenericPageCache<
    RequestData, CreationManager, BitmapCache, RequestQueue,
    CompactionPolicy, QueueProcessor
    >::InvalidatePreviewBitmap (const RequestData& rRequestData)
{
    maBitmapCache.InvalidateBitmap (rRequestData.GetPage());
}




template<class RequestData,
         class CreationManager,
         class BitmapCache,
         class RequestQueue,
         class CompactionPolicy,
        class QueueProcessor>
void GenericPageCache<
    RequestData, CreationManager, BitmapCache, RequestQueue,
    CompactionPolicy, QueueProcessor
    >::DecreaseRequestPriority (RequestData& rRequestData)
{
    if (mbLimitHasBeenReached)
        maRequestQueue.RemoveRequest (rRequestData);
    else
        maRequestQueue.ChangePriorityClass (rRequestData,+1);
}




template<class RequestData,
         class CreationManager,
         class BitmapCache,
         class RequestQueue,
         class CompactionPolicy,
        class QueueProcessor>
void GenericPageCache<
    RequestData, CreationManager, BitmapCache, RequestQueue,
    CompactionPolicy, QueueProcessor
    >::IncreaseRequestPriority (RequestData& rRequestData)
{
    maRequestQueue.ChangePriorityClass (rRequestData,-1);
}




template<class RequestData,
         class CreationManager,
         class BitmapCache,
         class RequestQueue,
         class CompactionPolicy,
        class QueueProcessor>
void GenericPageCache<
    RequestData, CreationManager, BitmapCache, RequestQueue,
    CompactionPolicy, QueueProcessor
    >::ReleasePreviewBitmap (RequestData& rRequestData)
{
    mpQueueProcessor->RemoveRequest (rRequestData);
    maRequestQueue.RemoveRequest (rRequestData);

    // We do not relase the preview bitmap that is associated with the page
    // of the given request data because this method is called when the
    // request data, typically a view-object-contact object, is destroyed.
    // The page object usually lives longer than that and thus the preview
    // bitmap may be used later on.
    //    maBitmapCache.ReleaseBitmap (rRequestData);
}




template<class RequestData,
         class RequestFactory,
         class BitmapCache,
         class RequestQueue,
         class CompactionPolicy,
        class QueueProcessor>
void GenericPageCache<
    RequestData, RequestFactory, BitmapCache, RequestQueue,
    CompactionPolicy, QueueProcessor
    >::InvalidateCache (void)
{
    // 1.) Stop the timer while the queues are being updated.
    mpQueueProcessor->Stop();

    // 2.) Clear the request queue of their current content.
    maRequestQueue.Clear();
    mbLimitHasBeenReached = false;

    // 3.) Create the new requests for filling the cache with at least the
    // visible previews.
    RequestFactory()(mrModel, mrView,maRequestQueue);

    // 4.) Start the timer again.
    mpQueueProcessor->Start();
}




template<class RequestData,
         class RequestFactory,
         class BitmapCache,
         class RequestQueue,
         class CompactionPolicy,
        class QueueProcessor>
void GenericPageCache<
    RequestData, RequestFactory, BitmapCache, RequestQueue,
    CompactionPolicy, QueueProcessor
    >::SetPreciousFlag (RequestData& rRequestData, bool bIsPrecious)
{
    maBitmapCache.SetPrecious (rRequestData.GetPage(), bIsPrecious);
}



} } } // end of namespace ::sd::slidesorter::cache

#endif
