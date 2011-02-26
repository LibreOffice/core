/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include "precompiled_sd.hxx"

#include "SlsQueueProcessor.hxx"
#include "SlsCacheConfiguration.hxx"
#include "SlsRequestQueue.hxx"

namespace sd { namespace slidesorter { namespace cache {


//=====  QueueProcessor  ======================================================

QueueProcessor::QueueProcessor (
    RequestQueue& rQueue,
    const ::boost::shared_ptr<BitmapCache>& rpCache,
    const Size& rPreviewSize,
    const SharedCacheContext& rpCacheContext)
    : maMutex(),
      maTimer(),
      mnTimeBetweenHighPriorityRequests (10/*ms*/),
      mnTimeBetweenLowPriorityRequests (100/*ms*/),
      mnTimeBetweenRequestsWhenNotIdle (1000/*ms*/),
      maPreviewSize(rPreviewSize),
      mpCacheContext(rpCacheContext),
      mrQueue(rQueue),
      mpCache(rpCache),
      maBitmapFactory(),
      mbIsPaused(false)
{
    // Look into the configuration if there for overriding values.
    ::com::sun::star::uno::Any aTimeBetweenReqeusts;
    aTimeBetweenReqeusts = CacheConfiguration::Instance()->GetValue(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TimeBetweenHighPriorityRequests")));
    if (aTimeBetweenReqeusts.has<sal_Int32>())
        aTimeBetweenReqeusts >>= mnTimeBetweenHighPriorityRequests;

    aTimeBetweenReqeusts = CacheConfiguration::Instance()->GetValue(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TimeBetweenLowPriorityRequests")));
    if (aTimeBetweenReqeusts.has<sal_Int32>())
        aTimeBetweenReqeusts >>= mnTimeBetweenLowPriorityRequests;

    aTimeBetweenReqeusts = CacheConfiguration::Instance()->GetValue(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TimeBetweenRequestsDuringShow")));
    if (aTimeBetweenReqeusts.has<sal_Int32>())
        aTimeBetweenReqeusts >>= mnTimeBetweenRequestsWhenNotIdle;

    maTimer.SetTimeoutHdl (LINK(this,QueueProcessor,ProcessRequestHdl));
    maTimer.SetTimeout (mnTimeBetweenHighPriorityRequests);
}





QueueProcessor::~QueueProcessor (void)
{
}




void QueueProcessor::Start (int nPriorityClass)
{
    if (mbIsPaused)
        return;
    if ( ! maTimer.IsActive())
    {
        if (nPriorityClass == 0)
            maTimer.SetTimeout (mnTimeBetweenHighPriorityRequests);
        else
            maTimer.SetTimeout (mnTimeBetweenLowPriorityRequests);
        maTimer.Start();
    }
}




void QueueProcessor::Stop (void)
{
    if (maTimer.IsActive())
        maTimer.Stop();
}




void QueueProcessor::Pause (void)
{
    mbIsPaused = true;
}




void QueueProcessor::Resume (void)
{
    mbIsPaused = false;
    if ( ! mrQueue.IsEmpty())
        Start(mrQueue.GetFrontPriorityClass());
}




void QueueProcessor::Terminate (void)
{
}




void QueueProcessor::SetPreviewSize (const Size& rPreviewSize)
{
    maPreviewSize = rPreviewSize;
}




IMPL_LINK(QueueProcessor, ProcessRequestHdl, Timer*, EMPTYARG)
{
    ProcessRequests();
    return 1;
}




void QueueProcessor::ProcessRequests (void)
{
    OSL_ASSERT(mpCacheContext.get()!=NULL);

    // Never process more than one request at a time in order to prevent the
    // lock up of the edit view.
    if ( ! mrQueue.IsEmpty()
        && ! mbIsPaused
        &&  mpCacheContext->IsIdle())
    {
        CacheKey aKey = NULL;
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

        if (aKey != NULL)
            ProcessOneRequest(aKey, ePriorityClass);
    }

    // Schedule the processing of the next element(s).
    {
        ::osl::MutexGuard aGuard (mrQueue.GetMutex());
        if ( ! mrQueue.IsEmpty())
            Start(mrQueue.GetFrontPriorityClass());
    }
}




void QueueProcessor::ProcessOneRequest (
    CacheKey aKey,
    const RequestPriorityClass ePriorityClass)
{
    try
    {
        ::osl::MutexGuard aGuard (maMutex);

        // Create a new preview bitmap and store it in the cache.
        if (mpCache.get() != NULL
            && mpCacheContext.get() != NULL)
        {
            const SdPage* pSdPage = dynamic_cast<const SdPage*>(mpCacheContext->GetPage(aKey));
            if (pSdPage != NULL)
            {
                const ::boost::shared_ptr<BitmapEx> pPreview (
                    maBitmapFactory.CreateBitmap(*pSdPage, maPreviewSize));
                mpCache->SetBitmap (
                    pSdPage,
                    pPreview,
                    ePriorityClass!=NOT_VISIBLE);

                // Initiate a repaint of the new preview.
                mpCacheContext->NotifyPreviewCreation(aKey, pPreview);
            }
        }
    }
    catch (::com::sun::star::uno::RuntimeException &aException)
    {
        (void) aException;
        OSL_FAIL("RuntimeException caught in QueueProcessor");
    }
    catch (::com::sun::star::uno::Exception &aException)
    {
        (void) aException;
        OSL_FAIL("Exception caught in QueueProcessor");
    }
}




void QueueProcessor::RemoveRequest (CacheKey aKey)
{
    (void)aKey;
    // See the method declaration above for an explanation why this makes sense.
    ::osl::MutexGuard aGuard (maMutex);
}




void QueueProcessor::SetBitmapCache (
    const ::boost::shared_ptr<BitmapCache>& rpCache)
{
    mpCache = rpCache;
}


} } } // end of namespace ::sd::slidesorter::cache

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
