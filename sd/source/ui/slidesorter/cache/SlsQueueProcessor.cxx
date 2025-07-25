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

#include "SlsQueueProcessor.hxx"
#include "SlsRequestQueue.hxx"
#include "SlsBitmapCache.hxx"

#include <sdpage.hxx>
#include <comphelper/profilezone.hxx>
#include <utility>
#include <comphelper/diagnose_ex.hxx>

namespace sd::slidesorter::cache {

//=====  QueueProcessor  ======================================================

QueueProcessor::QueueProcessor (
    RequestQueue& rQueue,
    std::shared_ptr<BitmapCache> pCache,
    const Size& rPreviewSize,
    const bool bDoSuperSampling,
    SharedCacheContext pCacheContext)
    : maTimer("sd::QueueProcessor maTimer"),
      maPreviewSize(rPreviewSize),
      mbDoSuperSampling(bDoSuperSampling),
      mpCacheContext(std::move(pCacheContext)),
      mrQueue(rQueue),
      mpCache(std::move(pCache))
{
    maTimer.SetInvokeHandler (LINK(this,QueueProcessor,ProcessRequestHdl));
    maTimer.SetTimeout (10);
}

QueueProcessor::~QueueProcessor()
{
}

void QueueProcessor::Start (int nPriorityClass)
{
    if ( ! maTimer.IsActive())
    {
        if (nPriorityClass == 0)
            maTimer.SetTimeout (10);
        else
            maTimer.SetTimeout (100);
        maTimer.Start();
    }
}

void QueueProcessor::Stop()
{
    if (maTimer.IsActive())
        maTimer.Stop();
}

void QueueProcessor::SetPreviewSize (
    const Size& rPreviewSize,
    const bool bDoSuperSampling)
{
    maPreviewSize = rPreviewSize;
    mbDoSuperSampling = bDoSuperSampling;
}

IMPL_LINK_NOARG(QueueProcessor, ProcessRequestHdl, Timer *, void)
{
    ProcessRequests();
}

void QueueProcessor::ProcessRequests()
{
    assert(mpCacheContext);

    // Never process more than one request at a time in order to prevent the
    // lock up of the edit view.
    if ( ! mrQueue.IsEmpty()
        &&  mpCacheContext->IsIdle())
    {
        CacheKey aKey = nullptr;
        RequestPriorityClass ePriorityClass (NOT_VISIBLE);
        {
            ::osl::MutexGuard aGuard (mrQueue.GetMutex());

            if ( ! mrQueue.IsEmpty())
            {
                // Get the request with the highest priority from the queue.
                ePriorityClass = mrQueue.GetFrontPriorityClass();
                aKey = mrQueue.GetFront();
                mrQueue.PopFront();
            }
        }

        if (aKey != nullptr)
            ProcessOneRequest(aKey, ePriorityClass);
    }

    // Schedule the processing of the next element(s).
    {
        ::osl::MutexGuard aGuard (mrQueue.GetMutex());
        if ( ! mrQueue.IsEmpty())
            Start(mrQueue.GetFrontPriorityClass());
        else
        {
            comphelper::ProfileZone aZone("QueueProcessor finished processing all elements");
        }
    }
}

void QueueProcessor::ProcessOneRequest (
    CacheKey aKey,
    const RequestPriorityClass ePriorityClass)
{
    try
    {
        std::scoped_lock aGuard (maMutex);

        // Create a new preview bitmap and store it in the cache.
        if (mpCache != nullptr && mpCacheContext)
        {
            const SdPage* pSdPage = dynamic_cast<const SdPage*>(mpCacheContext->GetPage(aKey));
            if (pSdPage != nullptr)
            {
                const BitmapEx aPreview (
                    maBitmapFactory.CreateBitmap(*pSdPage, maPreviewSize, mbDoSuperSampling));
                mpCache->SetBitmap (pSdPage, aPreview, ePriorityClass!=NOT_VISIBLE);

                // Initiate a repaint of the new preview.
                mpCacheContext->NotifyPreviewCreation(aKey);
            }
        }
    }
    catch (css::uno::Exception &)
    {
        TOOLS_WARN_EXCEPTION( "sd", "QueueProcessor");
    }
}

void QueueProcessor::SetBitmapCache (
    const std::shared_ptr<BitmapCache>& rpCache)
{
    mpCache = rpCache;
}

} // end of namespace ::sd::slidesorter::cache

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
