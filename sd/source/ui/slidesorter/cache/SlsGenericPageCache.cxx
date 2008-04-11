/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SlsGenericPageCache.cxx,v $
 *
 * $Revision: 1.3 $
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

#include "SlsGenericPageCache.hxx"

#include "SlsQueueProcessor.hxx"
#include "SlsRequestPriorityClass.hxx"
#include "SlsRequestFactory.hxx"
#include "cache/SlsPageCacheManager.hxx"
#include "model/SlideSorterModel.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "view/SlsPageObjectViewObjectContact.hxx"
#include "controller/SlideSorterController.hxx"

namespace sd { namespace slidesorter { namespace cache {

GenericPageCache::GenericPageCache (
    const Size& rPreviewSize,
    const SharedCacheContext& rpCacheContext)
    : mpBitmapCache(),
      maRequestQueue(rpCacheContext),
      mpQueueProcessor(),
      mpCacheContext(rpCacheContext),
      maPreviewSize(rPreviewSize)
{
}




GenericPageCache::~GenericPageCache (void)
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




void GenericPageCache::ProvideCacheAndProcessor (void)
{
    if (mpBitmapCache.get() == NULL)
        mpBitmapCache = PageCacheManager::Instance()->GetCache(
            mpCacheContext->GetModel(),
            maPreviewSize);

    if (mpQueueProcessor.get() == NULL)
        mpQueueProcessor.reset(new QueueProcessor(
            maRequestQueue,
            mpBitmapCache,
            maPreviewSize,
            mpCacheContext));
}




void GenericPageCache::ChangePreviewSize (const Size& rPreviewSize)
{
    if (rPreviewSize != maPreviewSize)
    {
        if (mpBitmapCache.get() != NULL)
        {
            mpBitmapCache = PageCacheManager::Instance()->ChangeSize(
                mpBitmapCache, maPreviewSize, rPreviewSize);
            if (mpQueueProcessor.get() != NULL)
            {
                mpQueueProcessor->SetPreviewSize(rPreviewSize);
                mpQueueProcessor->SetBitmapCache(mpBitmapCache);
            }
        }
        maPreviewSize = rPreviewSize;
    }
}




BitmapEx GenericPageCache::GetPreviewBitmap (
    CacheKey aKey,
    const Size& rSize)
{
    OSL_ASSERT(aKey != NULL);

    BitmapEx aPreview;
    bool bMayBeUpToDate = true;
    ProvideCacheAndProcessor();
    const SdrPage* pPage = mpCacheContext->GetPage(aKey);
    if (mpBitmapCache->HasBitmap(pPage))
    {
        ::boost::shared_ptr<BitmapEx> pPreview(mpBitmapCache->GetBitmap(pPage));
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
    RequestPreviewBitmap(aKey, rSize, bMayBeUpToDate);

    return aPreview;
}




void GenericPageCache::RequestPreviewBitmap (
    CacheKey aKey,
    const Size& rSize,
    bool bMayBeUpToDate)
{
    OSL_ASSERT(aKey != NULL);

    const SdrPage* pPage = mpCacheContext->GetPage(aKey);

    ProvideCacheAndProcessor();

    // Determine if the available bitmap is up to date.
    bool bIsUpToDate = false;
    if (bMayBeUpToDate)
        bIsUpToDate = mpBitmapCache->BitmapIsUpToDate (pPage);
    if (bIsUpToDate)
    {
        ::boost::shared_ptr<BitmapEx> pPreview (mpBitmapCache->GetBitmap(pPage));
        if (pPreview.get()==NULL || pPreview->GetSizePixel()!=rSize)
              bIsUpToDate = false;
    }

    if ( ! bIsUpToDate)
    {
        // No, the bitmap is not up-to-date.  Request a new one.
        RequestPriorityClass ePriorityClass (NOT_VISIBLE);
        if (mpCacheContext->IsVisible(aKey))
            if (mpBitmapCache->HasBitmap(pPage))
                ePriorityClass = VISIBLE_OUTDATED_PREVIEW;
            else
                ePriorityClass = VISIBLE_NO_PREVIEW;
        maRequestQueue.AddRequest(aKey, ePriorityClass);
        mpQueueProcessor->Start(ePriorityClass);
    }
}




void GenericPageCache::InvalidatePreviewBitmap (CacheKey aKey)
{
    if (mpBitmapCache.get() != NULL)
        mpBitmapCache->InvalidateBitmap(mpCacheContext->GetPage(aKey));
}




void GenericPageCache::ReleasePreviewBitmap (CacheKey aKey)
{
    if (mpBitmapCache.get() != NULL)
    {
        // Suspend the queue processing temporarily to avoid the reinsertion
        // of the request that is to be deleted.
        mpQueueProcessor->Stop();

        maRequestQueue.RemoveRequest(aKey);
        mpQueueProcessor->RemoveRequest(aKey);

        // Resume the queue processing.
        if ( ! maRequestQueue.IsEmpty())
        {
            try
            {
                mpQueueProcessor->Start(maRequestQueue.GetFrontPriorityClass());
            }
            catch (::com::sun::star::uno::RuntimeException)
            {
            }
        }
    }

    // We do not relase the preview bitmap that is associated with the page
    // of the given request data because this method is called when the
    // request data, typically a view-object-contact object, is destroyed.
    // The page object usually lives longer than that and thus the preview
    // bitmap may be used later on.
}




void GenericPageCache::InvalidateCache (bool bUpdateCache)
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
            RequestFactory()(maRequestQueue, mpCacheContext);
    }
}




void GenericPageCache::SetPreciousFlag (CacheKey aKey, bool bIsPrecious)
{
    ProvideCacheAndProcessor();

    // Change the request priority class according to the new precious flag.
    if (bIsPrecious)
    {
        if (mpBitmapCache->HasBitmap(mpCacheContext->GetPage(aKey)))
            maRequestQueue.ChangeClass(aKey,VISIBLE_OUTDATED_PREVIEW);
        else
            maRequestQueue.ChangeClass(aKey,VISIBLE_NO_PREVIEW);
    }
    else
    {
        if (mpBitmapCache->IsFull())
        {
            // When the bitmap cache is full then requests for slides that
            // are not visible are removed.
            maRequestQueue.RemoveRequest(aKey);
        }
        else
            maRequestQueue.ChangeClass(aKey,NOT_VISIBLE);
    }

    mpBitmapCache->SetPrecious(mpCacheContext->GetPage(aKey), bIsPrecious);
}




bool GenericPageCache::IsEmpty (void) const
{
    if (mpBitmapCache.get() != NULL)
        return mpBitmapCache->IsEmpty();
    else
        return true;
}




void GenericPageCache::Pause (void)
{
    ProvideCacheAndProcessor();
    if (mpQueueProcessor.get() != NULL)
        mpQueueProcessor->Pause();
}




void GenericPageCache::Resume (void)
{
    ProvideCacheAndProcessor();
    if (mpQueueProcessor.get() != NULL)
        mpQueueProcessor->Resume();
}



} } } // end of namespace ::sd::slidesorter::cache
