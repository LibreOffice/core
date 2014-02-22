/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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
    
    
        DBG_ASSERT (maPreviewSize.Width()<1000 && maPreviewSize.Height()<1000,
        "GenericPageCache<>::GetPreviewBitmap(): bitmap requested with large width. "
        "This may indicate an error.");
}




GenericPageCache::~GenericPageCache (void)
{
    if (mpQueueProcessor.get() != NULL)
        mpQueueProcessor->Stop();
    maRequestQueue.Clear();
    if (mpQueueProcessor.get() != NULL)
        mpQueueProcessor->Terminate();
    mpQueueProcessor.reset();

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
            mbDoSuperSampling,
            mpCacheContext));
}




void GenericPageCache::ChangePreviewSize (
    const Size& rPreviewSize,
    const bool bDoSuperSampling)
{
    if (rPreviewSize!=maPreviewSize || bDoSuperSampling!=mbDoSuperSampling)
    {
        
        
        DBG_ASSERT (maPreviewSize.Width()<1000 && maPreviewSize.Height()<1000,
            "GenericPageCache<>::GetPreviewBitmap(): bitmap requested with large width. "
            "This may indicate an error.");

        if (mpBitmapCache.get() != NULL)
        {
            mpBitmapCache = PageCacheManager::Instance()->ChangeSize(
                mpBitmapCache, maPreviewSize, rPreviewSize);
            if (mpQueueProcessor.get() != NULL)
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
    OSL_ASSERT(aKey != NULL);

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

    
    
    
    RequestPreviewBitmap(aKey, bMayBeUpToDate);

    return aPreview;
}




Bitmap GenericPageCache::GetMarkedPreviewBitmap (
    const CacheKey aKey,
    const bool bResize)
{
    OSL_ASSERT(aKey != NULL);

    ProvideCacheAndProcessor();
    const SdrPage* pPage = mpCacheContext->GetPage(aKey);
    Bitmap aMarkedPreview (mpBitmapCache->GetMarkedBitmap(pPage));
    const Size aBitmapSize (aMarkedPreview.GetSizePixel());
    if (bResize && aBitmapSize != maPreviewSize)
    {
        
        
        if (aBitmapSize.Width()>0 && aBitmapSize.Height()>0)
        {
            aMarkedPreview.Scale(maPreviewSize);
        }
    }

    return aMarkedPreview;
}




void GenericPageCache::SetMarkedPreviewBitmap (
    const CacheKey aKey,
    const Bitmap& rMarkedBitmap)
{
    OSL_ASSERT(aKey != NULL);

    ProvideCacheAndProcessor();
    const SdrPage* pPage = mpCacheContext->GetPage(aKey);
    mpBitmapCache->SetMarkedBitmap(pPage, rMarkedBitmap);
}




void GenericPageCache::RequestPreviewBitmap (
    const CacheKey aKey,
    const bool bMayBeUpToDate)
{
    OSL_ASSERT(aKey != NULL);

    const SdrPage* pPage = mpCacheContext->GetPage(aKey);

    ProvideCacheAndProcessor();

    
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
    
    ::boost::shared_ptr<cache::PageCacheManager> pCacheManager (
        cache::PageCacheManager::Instance());
    if (pCacheManager)
        return pCacheManager->InvalidatePreviewBitmap(
            mpCacheContext->GetModel(),
            aKey);
    else if (mpBitmapCache.get() != NULL)
        return mpBitmapCache->InvalidateBitmap(mpCacheContext->GetPage(aKey));
    else
        return false;
}

void GenericPageCache::InvalidateCache (const bool bUpdateCache)
{
    if (mpBitmapCache)
    {
        
        
        
        mpQueueProcessor->Stop();
        maRequestQueue.Clear();

        
        
        
        mpBitmapCache->InvalidateCache();
        if (bUpdateCache)
            RequestFactory()(maRequestQueue, mpCacheContext);
    }
}




void GenericPageCache::SetPreciousFlag (
    const CacheKey aKey,
    const bool bIsPrecious)
{
    ProvideCacheAndProcessor();

    
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
            
            
            maRequestQueue.RemoveRequest(aKey);
        }
        else
            maRequestQueue.ChangeClass(aKey,NOT_VISIBLE);
    }

    mpBitmapCache->SetPrecious(mpCacheContext->GetPage(aKey), bIsPrecious);
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



} } } 



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
