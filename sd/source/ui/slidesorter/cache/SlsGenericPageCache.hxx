/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlsGenericPageCache.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-10-24 07:40:48 $
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
#ifndef SD_SLIDESORTER_GENERIC_PAGE_CACHE_HXX
#define SD_SLIDESORTER_GENERIC_PAGE_CACHE_HXX

#include "SlsQueueProcessor.hxx"
#include "SlsRequestPriorityClass.hxx"
#include "model/SlideSorterModel.hxx"
#include "view/SlsPageObjectViewObjectContact.hxx"

namespace sd { namespace slidesorter { namespace model {
class SlideSorterModel;
} } }

namespace sd { namespace slidesorter { namespace view {
class SlideSorterView;
} } }

namespace sd { namespace slidesorter { namespace cache {


/** This basically is the implementation class for the PageCache class.  It
    allows the PageCache class to hide the template parameters.
*/
template<class RequestData,
         class CreationManager,
         class RequestQueue,
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
        const Size& rPreviewSize);

    ~GenericPageCache (void);

    /** Change the size of the preview bitmaps.  This may be caused by a
        resize of the slide sorter window or a change of the number of
        columns.
    */
    void ChangePreviewSize (const Size& rPreviewSize);

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
        @param rRequestData
            This data is used to determine the preview.
        @param rSize
            The size of the requested preview bitmap in pixel coordinates.
        @param bMayBeUpToDate
            This flag helps the method to determine whether an existing
            preview that matches the request is up to date.  If the caller
            know that it is not then by passing <FALSE/> he tells us that we
            do not have to check the up-to-date flag a second time.  If
            unsure pass <TRUE/>.
    */
    void RequestPreviewBitmap (
        RequestData& rRequestData,
        const Size& rSize,
        bool bMayBeUpToDate = true);

    /** Tell the cache to replace the bitmap associated with the given
        request data with a new one that reflects recent changes in the
        content of the page object.
    */
    void InvalidatePreviewBitmap (const RequestData& rRequestData);

    /** Call this method when a view-object-contact object is being deleted
        and does not need (a) its current bitmap in the cache and (b) a
        requested a new bitmap.
    */
    void ReleasePreviewBitmap (RequestData& rRequestData);

    /** Call this method when all preview bitmaps have to be generated anew.
        This is the case when the size of the page objects on the screen has
        changed or when the model has changed.
    */
    void InvalidateCache (bool bUpdateCache);

    /** With the precious flag you can control whether a bitmap can be
        removed from the cache or reduced in size to make room for other
        bitmaps or is so precious that it will not be touched.  A typical
        use is to set the precious flag for the visible pages.
    */
    void SetPreciousFlag (RequestData& rRequestData, bool bIsPrecious);

    /** Return <TRUE/> when there is no preview bitmap in the cache.
    */
    bool IsEmpty (void) const;

    /** Return the view in which the preview bitmaps are displayed.
    */
    view::SlideSorterView& GetView (void) const;

    /** Return the model for whose slides the previews are created.
    */
    model::SlideSorterModel& GetModel (void) const;

private:
    view::SlideSorterView& mrView;

    model::SlideSorterModel& mrModel;

    ::boost::shared_ptr<BitmapCache> mpBitmapCache;

    RequestQueue maRequestQueue;

    ::std::auto_ptr<QueueProcessor> mpQueueProcessor;

    /** The current size of preview bitmaps.
    */
    Size maPreviewSize;

    /** Both bitmap cache and queue processor are created on demand by this
        method.
    */
    void ProvideCacheAndProcessor (void);
};




//=====  GenericPageCache =====================================================

template<class RequestData,
         class CreationManager,
         class RequestQueue,
         class QueueProcessor>
GenericPageCache<
    RequestData, CreationManager, RequestQueue, QueueProcessor
    >::GenericPageCache (
        view::SlideSorterView& rView,
        model::SlideSorterModel& rModel,
        const Size& rPreviewSize)
        : mrView(rView),
          mrModel(rModel),
          mpBitmapCache(),
          maRequestQueue(),
          mpQueueProcessor(),
          maPreviewSize(rPreviewSize)
{
}




template<class RequestData,
         class CreationManager,
         class RequestQueue,
         class QueueProcessor>
GenericPageCache<
    RequestData, CreationManager, RequestQueue, QueueProcessor
    >::~GenericPageCache (void)
{
    OSL_TRACE("terminating queue processor %p", mpQueueProcessor.get());
    if (mpQueueProcessor.get() != NULL)
        mpQueueProcessor->Stop();
    maRequestQueue.Clear();
    if (mpQueueProcessor.get() != NULL)
        mpQueueProcessor->Terminate();
    mpQueueProcessor.reset();
    OSL_TRACE("queue processor stopped and terminated");

    if (mpBitmapCache.get() != NULL)
        PageCacheManager::Instance()->ReleaseCache(mpBitmapCache);
    mpBitmapCache.reset();
}




template<class RequestData,
         class CreationManager,
         class RequestQueue,
         class QueueProcessor>
void GenericPageCache<
    RequestData, CreationManager, RequestQueue, QueueProcessor
    >::ProvideCacheAndProcessor (void)
{
    if (mpBitmapCache.get() == NULL)
        mpBitmapCache = PageCacheManager::Instance()->GetCache(
            mrModel.GetDocument(),
            maPreviewSize);

    if (mpQueueProcessor.get() == NULL)
        mpQueueProcessor.reset(new QueueProcessor(mrView,maRequestQueue,*mpBitmapCache));
}




template<class RequestData,
         class CreationManager,
         class RequestQueue,
         class QueueProcessor>
void GenericPageCache<
    RequestData, CreationManager, RequestQueue, QueueProcessor
    >::ChangePreviewSize (const Size& rPreviewSize)
{
    if (mpBitmapCache.get() != NULL)
        mpBitmapCache = PageCacheManager::Instance()->ChangeSize(
            mpBitmapCache, maPreviewSize, rPreviewSize);
    maPreviewSize = rPreviewSize;
}




template<class RequestData,
         class CreationManager,
         class RequestQueue,
         class QueueProcessor>
BitmapEx GenericPageCache<
    RequestData, CreationManager, RequestQueue, QueueProcessor
    >::GetPreviewBitmap (
        RequestData& rRequestData,
        const Size& rSize)
{
    BitmapEx aPreview;
    bool bMayBeUpToDate = true;
    ProvideCacheAndProcessor();
    if (mpBitmapCache->HasBitmap(rRequestData.GetPage()))
    {
        ::boost::shared_ptr<BitmapEx> pPreview(mpBitmapCache->GetBitmap(rRequestData.GetPage()));
        OSL_ASSERT(pPreview.get() != NULL);
        aPreview = *pPreview;
        Size aBitmapSize (aPreview.GetSizePixel());
        if (aBitmapSize != rSize)
        {
            // The bitmap has the wrong size.
            DBG_ASSERT (rSize.Width() < 1000,
                "GenericPageCache<>::GetPreviewBitmap(): bitmap requested with large width. "
                "This may indicate an error.");

            // Scale the bitmap to the desired size when that is possible,
            // i.e. the bitmap is not empty.
            if (aBitmapSize.Width()>0 && aBitmapSize.Height()>0)
                aPreview.Scale (rSize, BMP_SCALE_FAST);
        }
        bMayBeUpToDate = true;
    }
    else
        bMayBeUpToDate = false;

    // Request the creation of a correctly sized preview bitmap.  We do this
    // even when the size of the bitmap in the cache is correct because its
    // content may be not up-to-date anymore.
    RequestPreviewBitmap (rRequestData, rSize, bMayBeUpToDate);

    return aPreview;
}




template<class RequestData,
         class CreationManager,
         class RequestQueue,
         class QueueProcessor>
void GenericPageCache<
    RequestData, CreationManager, RequestQueue, QueueProcessor
    >::RequestPreviewBitmap (
        RequestData& rRequestData,
        const Size& rSize,
        bool bMayBeUpToDate)
{
    const SdrPage* pPage = rRequestData.GetPage();

    ProvideCacheAndProcessor();

    // Determine if the available bitmap is up to date.
    bool bIsUpToDate = false;
    if (bMayBeUpToDate)
        bIsUpToDate = mpBitmapCache->BitmapIsUpToDate (pPage);
    if (bIsUpToDate)
    {
        BitmapEx aPreview (*mpBitmapCache->GetBitmap (pPage));
        if (aPreview.GetSizePixel() != rSize)
            bIsUpToDate = false;
    }

    if ( ! bIsUpToDate)
    {
        // No, the bitmap is not up-to-date.  Request a new one.
        RequestPriorityClass ePriorityClass (NOT_VISIBLE);
        if (rRequestData.GetPageDescriptor().IsVisible())
            if (mpBitmapCache->HasBitmap(pPage))
                ePriorityClass = VISIBLE_OUTDATED_PREVIEW;
            else
                ePriorityClass = VISIBLE_NO_PREVIEW;
        maRequestQueue.AddRequest(rRequestData, ePriorityClass);
        mpQueueProcessor->Start(ePriorityClass);
    }
}




template<class RequestData,
         class CreationManager,
         class RequestQueue,
        class QueueProcessor>
void GenericPageCache<
    RequestData, CreationManager, RequestQueue, QueueProcessor
    >::InvalidatePreviewBitmap (const RequestData& rRequestData)
{
    if (mpBitmapCache.get() != NULL)
        mpBitmapCache->InvalidateBitmap(rRequestData.GetPage());
}




template<class RequestData,
         class CreationManager,
         class RequestQueue,
        class QueueProcessor>
void GenericPageCache<
    RequestData, CreationManager, RequestQueue, QueueProcessor
    >::ReleasePreviewBitmap (RequestData& rRequestData)
{
    if (mpBitmapCache.get() != NULL)
    {
        mpQueueProcessor->RemoveRequest (rRequestData);
        maRequestQueue.RemoveRequest (rRequestData);
    }

    // We do not relase the preview bitmap that is associated with the page
    // of the given request data because this method is called when the
    // request data, typically a view-object-contact object, is destroyed.
    // The page object usually lives longer than that and thus the preview
    // bitmap may be used later on.
}




template<class RequestData,
         class RequestFactory,
         class RequestQueue,
        class QueueProcessor>
void GenericPageCache<
    RequestData, RequestFactory, RequestQueue, QueueProcessor
    >::InvalidateCache (bool bUpdateCache)
{
    if (mpBitmapCache.get() != NULL)
    {
        // When the cache is being invalidated then it makes no sense to
        // continue creating preview bitmaps.  However, this may be
        // re-started below.
        mpQueueProcessor->Stop();
        maRequestQueue.Clear();

        // Mark the previews in the cache as not being up-to-date anymore.
        // Depending on the given bUpdateCache flag we start to create new
        // preview bitmaps.
        mpBitmapCache->InvalidateCache();
        if (bUpdateCache)
            RequestFactory()(mrModel, mrView,maRequestQueue);
    }
}




template<class RequestData,
         class RequestFactory,
         class RequestQueue,
        class QueueProcessor>
void GenericPageCache<
    RequestData, RequestFactory, RequestQueue, QueueProcessor
    >::SetPreciousFlag (RequestData& rRequestData, bool bIsPrecious)
{
    ProvideCacheAndProcessor();

    // Change the request priority class according to the new precious flag.
    if (bIsPrecious)
    {
        if (mpBitmapCache->HasBitmap(rRequestData.GetPage()))
            maRequestQueue.ChangeClass(rRequestData,VISIBLE_OUTDATED_PREVIEW);
        else
            maRequestQueue.ChangeClass(rRequestData,VISIBLE_NO_PREVIEW);
    }
    else
    {
        if (mpBitmapCache->IsFull())
        {
            // When the bitmap cache is full then requests for slides that
            // are not visible are removed.
            maRequestQueue.RemoveRequest(rRequestData);
        }
        else
            maRequestQueue.ChangeClass(rRequestData,NOT_VISIBLE);
    }

    mpBitmapCache->SetPrecious(rRequestData.GetPage(), bIsPrecious);
}




template<class RequestData,
         class RequestFactory,
         class RequestQueue,
        class QueueProcessor>
bool GenericPageCache<
    RequestData, RequestFactory, RequestQueue, QueueProcessor
    >::IsEmpty (void) const
{
    if (mpBitmapCache.get() != NULL)
        return mpBitmapCache->IsEmpty();
    else
        return true;
}



template<class RequestData,
         class CreationManager,
         class RequestQueue,
         class QueueProcessor>
model::SlideSorterModel& GenericPageCache<
    RequestData, CreationManager, RequestQueue, QueueProcessor
    >::GetModel (void) const
{
    return mrModel;
}




template<class RequestData,
         class CreationManager,
         class RequestQueue,
         class QueueProcessor>
view::SlideSorterView& GenericPageCache<
    RequestData, CreationManager, RequestQueue, QueueProcessor
    >::GetView (void) const
{
    return mrView;
}




} } } // end of namespace ::sd::slidesorter::cache

#endif
