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

#ifndef INCLUDED_SD_SOURCE_UI_SLIDESORTER_CACHE_SLSQUEUEPROCESSOR_HXX
#define INCLUDED_SD_SOURCE_UI_SLIDESORTER_CACHE_SLSQUEUEPROCESSOR_HXX

#include "cache/SlsPageCache.hxx"
#include "SlsRequestPriorityClass.hxx"
#include "SlsBitmapFactory.hxx"
#include "view/SlideSorterView.hxx"
#include "tools/IdleDetection.hxx"
#include "SlsBitmapCache.hxx"
#include "sdpage.hxx"
#include "Window.hxx"

#include <svx/svdpagv.hxx>
#include <vcl/svapp.hxx>
#include <vcl/timer.hxx>

#include <functional>

namespace sd { namespace slidesorter { namespace view {
class SlideSorterView;
} } }

namespace sd { namespace slidesorter { namespace cache {

class BitmapCache;
class RequestQueue;

/** This queue processor is timer based, i.e. when an entry is added to the
    queue and the processor is started with Start() in the base class a
    timer is started that eventually calls ProcessRequest().  This is
    repeated until the queue is empty or Stop() is called.
*/
class QueueProcessor
{
public:
    typedef ::std::function<bool ()> IdleDetectionCallback;
    QueueProcessor (
        RequestQueue& rQueue,
        const ::boost::shared_ptr<BitmapCache>& rpCache,
        const Size& rPreviewSize,
        const bool bDoSuperSampling,
        const SharedCacheContext& rpCacheContext);
    virtual ~QueueProcessor();

    /** Start the processor.  This implementation is timer based and waits
        an defined amount of time that depends on the given argument before
        the next entry in the queue is processed.
        @param nPriorityClass
            A priority class of 0 tells the processor that a high priority
            request is waiting in the queue.  The time to wait is thus
            shorter then that for a low priority request (denoted by a value
            of 1.)  When the timer is already running it is not modified.
    */
    void Start (int nPriorityClass = 0);
    void Stop();
    void Pause();
    void Resume();

    void SetPreviewSize (
        const Size& rSize,
        const bool bDoSuperSampling);

    /** Use this method when the page cache is (maybe) using a different
        BitmapCache.  This is usually necessary after calling
        PageCacheManager::ChangeSize().
    */
    void SetBitmapCache (const ::boost::shared_ptr<BitmapCache>& rpCache);

private:
    /** This mutex is used to guard the queue processor.  Be careful not to
        mix its use with that of the solar mutex.
    */
    ::osl::Mutex maMutex;

    Timer  maTimer;
    DECL_LINK_TYPED(ProcessRequestHdl, Timer *, void);
    sal_uInt32 mnTimeBetweenHighPriorityRequests;
    sal_uInt32 mnTimeBetweenLowPriorityRequests;
    sal_uInt32 mnTimeBetweenRequestsWhenNotIdle;
    Size maPreviewSize;
    bool mbDoSuperSampling;
    SharedCacheContext mpCacheContext;
    RequestQueue& mrQueue;
    ::boost::shared_ptr<BitmapCache> mpCache;
    BitmapFactory maBitmapFactory;
    bool mbIsPaused;

    void ProcessRequests();
    void ProcessOneRequest (
        CacheKey aKey,
        const RequestPriorityClass ePriorityClass);
};

} } } // end of namespace ::sd::slidesorter::cache

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
