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

#include "SlsGenericPageCache.hxx"

#include "SlsQueueProcessor.hxx"
#include "SlsRequestPriorityClass.hxx"
#include "SlsRequestFactory.hxx"
#include "cache/SlsPageCacheManager.hxx"
#include "model/SlideSorterModel.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "controller/SlideSorterController.hxx"

namespace sd { namespace slidesorter { namespace cache {

GenericPageCache::GenericPageCache (
    const Size& rPreviewSize,
    const bool bDoSuperSampling,
    const SharedCacheContext& rpCacheContext)
    : mpBitmapCache(),
      maRequestQueue(rpCacheContext),
      mpQueueProcessor(),
      mpCacheContext(rpCacheContext),
      maPreviewSize(rPreviewSize),
      mbDoSuperSampling(bDoSuperSampling)
{
    // A large size may indicate an error of the caller.  After all we
    // are creating previews.
        DBG_ASSERT (maPreviewSize.Width()<1000 && maPreviewSize.Height()<1000,
        "GenericPageCache<>::GetPreviewBitmap(): bitmap requested with large width. "
        "This may indicate an error.");
}

GenericPageCache::~GenericPageCache()
{
    if (mpQueueProcessor.get() != nullptr)
        mpQueueProcessor->Stop();
    maRequestQueue.Clear();
    mpQueueProcessor.reset();

    if (mpBitmapCache.get() != nullptr)
        PageCacheManager::Instance()->ReleaseCache(mpBitmapCache);
    mpBitmapCache.reset();
}

void GenericPageCache::ProvideCacheAndProcessor()
{
    if (mpBitmapCache.get() == nullptr)
        mpBitmapCache = PageCacheManager::Instance()->GetCache(
            mpCacheContext->GetModel(),
            maPreviewSize);

    if (mpQueueProcessor.get() == nullptr)
        mpQueueProcessor.reset(new QueueProcessor(
            maRequestQueue,
            mpBitmapCache,
            maPreviewSize,
            mbDoSuperSampling,
            mpCacheContext));
}

void GenericPageCache::ChangePreviewSize (
    const Size& rPreviewSize,
    const bool bDoSuperSampling)
{
    if (rPreviewSize!=maPreviewSize || bDoSuperSampling!=mbDoSuperSampling)
    {
        // A large size may indicate an error of the caller.  After all we
        // are creating previews.
        DBG_ASSERT (maPreviewSize.Width()<1000 && maPreviewSize.Height()<1000,
            "GenericPageCache<>::GetPreviewBitmap(): bitmap requested with large width. "
            "This may indicate an error.");

        if (mpBitmapCache.get() != nullptr)
        {
            mpBitmapCache = PageCacheManager::Instance()->ChangeSize(
                mpBitmapCache, maPreviewSize, rPreviewSize);
            if (mpQueueProcessor.get() != nullptr)
            {
                mpQueueProcessor->SetPreviewSize(rPreviewSize, bDoSuperSampling);
                mpQueueProcessor->SetBitmapCache(mpBitmapCache);
            }
        }
        maPreviewSize = rPreviewSize;
        mbDoSuperSampling = bDoSuperSampling;
    }
}

Bitmap GenericPageCache::GetPreviewBitmap (
    const CacheKey aKey,
    const bool bResize)
{
    OSL_ASSERT(aKey != nullptr);

    Bitmap aPreview;
    bool bMayBeUpToDate = true;
    ProvideCacheAndProcessor();
    const SdrPage* pPage = mpCacheContext->GetPage(aKey);
    if (mpBitmapCache->HasBitmap(pPage))
    {
        aPreview = mpBitmapCache->GetBitmap(pPage);
        const Size aBitmapSize (aPreview.GetSizePixel());
        if (aBitmapSize != maPreviewSize)
        {
            // Scale the bitmap to the desired size when that is possible,
            // i.e. the bitmap is not empty.
            if (bResize && aBitmapSize.Width()>0 && aBitmapSize.Height()>0)
            {
                aPreview.Scale(maPreviewSize);
            }
            bMayBeUpToDate = false;
        }
        else
            bMayBeUpToDate = true;
    }
    else
        bMayBeUpToDate = false;

    // Request the creation of a correctly sized preview bitmap.  We do this
    // even when the size of the bitmap in the cache is correct because its
    // content may be not up-to-date anymore.
    RequestPreviewBitmap(aKey, bMayBeUpToDate);

    return aPreview;
}

Bitmap GenericPageCache::GetMarkedPreviewBitmap (
    const CacheKey aKey)
{
    OSL_ASSERT(aKey != nullptr);

    ProvideCacheAndProcessor();
    const SdrPage* pPage = mpCacheContext->GetPage(aKey);
    Bitmap aMarkedPreview (mpBitmapCache->GetMarkedBitmap(pPage));

    return aMarkedPreview;
}

void GenericPageCache::SetMarkedPreviewBitmap (
    const CacheKey aKey,
    const Bitmap& rMarkedBitmap)
{
    OSL_ASSERT(aKey != nullptr);

    ProvideCacheAndProcessor();
    const SdrPage* pPage = mpCacheContext->GetPage(aKey);
    mpBitmapCache->SetMarkedBitmap(pPage, rMarkedBitmap);
}

void GenericPageCache::RequestPreviewBitmap (
    const CacheKey aKey,
    const bool bMayBeUpToDate)
{
    OSL_ASSERT(aKey != nullptr);

    const SdrPage* pPage = mpCacheContext->GetPage(aKey);

    ProvideCacheAndProcessor();

    // Determine if the available bitmap is up to date.
    bool bIsUpToDate = false;
    if (bMayBeUpToDate)
        bIsUpToDate = mpBitmapCache->BitmapIsUpToDate (pPage);
    if (bIsUpToDate)
    {
        const Bitmap aPreview (mpBitmapCache->GetBitmap(pPage));
        if (aPreview.IsEmpty() || aPreview.GetSizePixel()!=maPreviewSize)
              bIsUpToDate = false;
    }

    if ( ! bIsUpToDate)
    {
        // No, the bitmap is not up-to-date.  Request a new one.
        RequestPriorityClass ePriorityClass (NOT_VISIBLE);
        if (mpCacheContext->IsVisible(aKey))
        {
            if (mpBitmapCache->HasBitmap(pPage))
                ePriorityClass = VISIBLE_OUTDATED_PREVIEW;
            else
                ePriorityClass = VISIBLE_NO_PREVIEW;
        }
        maRequestQueue.AddRequest(aKey, ePriorityClass);
        mpQueueProcessor->Start(ePriorityClass);
    }
}

bool GenericPageCache::InvalidatePreviewBitmap (const CacheKey aKey)
{
    // Invalidate the page in all caches that reference it, not just this one.
    std::shared_ptr<cache::PageCacheManager> pCacheManager (
        cache::PageCacheManager::Instance());
    if (pCacheManager)
        return pCacheManager->InvalidatePreviewBitmap(
            mpCacheContext->GetModel(),
            aKey);
    else if (mpBitmapCache.get() != nullptr)
        return mpBitmapCache->InvalidateBitmap(mpCacheContext->GetPage(aKey));
    else
        return false;
}

void GenericPageCache::InvalidateCache ()
{
    if (mpBitmapCache)
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
        RequestFactory()(maRequestQueue, mpCacheContext);
    }
}

void GenericPageCache::SetPreciousFlag (
    const CacheKey aKey,
    const bool bIsPrecious)
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

void GenericPageCache::Pause()
{
    ProvideCacheAndProcessor();
    if (mpQueueProcessor.get() != nullptr)
        mpQueueProcessor->Pause();
}

void GenericPageCache::Resume()
{
    ProvideCacheAndProcessor();
    if (mpQueueProcessor.get() != nullptr)
        mpQueueProcessor->Resume();
}

} } } // end of namespace ::sd::slidesorter::cache

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
