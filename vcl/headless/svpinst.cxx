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

#include <sal/config.h>

#include <mutex>

#include <pthread.h>
#include <sys/time.h>

#include <sal/types.h>

#include <vcl/virdev.hxx>
#include <vcl/inputtypes.hxx>
#include <vcl/lok.hxx>

#include <headless/svpinst.hxx>
#include <headless/svpframe.hxx>
#include <headless/svpdummies.hxx>
#include <headless/svpvd.hxx>
#ifdef IOS
#  include <quartz/salbmp.h>
#  include <quartz/salgdi.h>
#  include <quartz/salvd.h>
#else
#  include <cairo.h>
#  include <headless/svpgdi.hxx>
#endif
#include <headless/svpbmp.hxx>

#include <salframe.hxx>
#include <svdata.hxx>
// FIXME: remove when we re-work the svp mainloop
#include <unx/salunxtime.h>
#include <tools/debug.hxx>

SvpSalInstance* SvpSalInstance::s_pDefaultInstance = nullptr;

#ifndef NDEBUG
static bool g_CheckedMutex = false;

#define DBG_TESTSVPYIELDMUTEX() \
do { \
    if (!g_CheckedMutex) \
    { \
        assert(dynamic_cast<SvpSalYieldMutex*>(GetYieldMutex()) != nullptr \
            && "This SvpSalInstance function requires use of SvpSalYieldMutex"); \
        g_CheckedMutex = true; \
    } \
} while(false)

#else // NDEBUG
#define DBG_TESTSVPYIELDMUTEX() ((void)0)
#endif

#if !defined(ANDROID) && !defined(IOS) && !defined(EMSCRIPTEN)

static void atfork_child()
{
    if (SvpSalInstance::s_pDefaultInstance != nullptr)
    {
        SvpSalInstance::s_pDefaultInstance->CloseWakeupPipe();
    }
}

#endif

SvpSalInstance::SvpSalInstance( std::unique_ptr<SalYieldMutex> pMutex )
    : SalGenericInstance( std::move(pMutex) )
{
    m_aTimeout.tv_sec       = 0;
    m_aTimeout.tv_usec      = 0;
    m_nTimeoutMS            = 0;

    m_MainThread = osl::Thread::getCurrentIdentifier();
    if( s_pDefaultInstance == nullptr )
        s_pDefaultInstance = this;
#if !defined(ANDROID) && !defined(IOS) && !defined(EMSCRIPTEN)
    pthread_atfork(nullptr, nullptr, atfork_child);
#endif
}

SvpSalInstance::~SvpSalInstance()
{
    if( s_pDefaultInstance == this )
        s_pDefaultInstance = nullptr;
    CloseWakeupPipe();
}

void SvpSalInstance::CloseWakeupPipe()
{
    SvpSalYieldMutex *const pMutex(dynamic_cast<SvpSalYieldMutex*>(GetYieldMutex()));
    if (!pMutex)
        return;
    while (!pMutex->m_FeedbackPipe.empty())
        pMutex->m_FeedbackPipe.pop();
}

void SvpSalInstance::TriggerUserEventProcessing()
{
    Wakeup();
}

void SvpSalInstance::Wakeup(SvpRequest const request)
{
    DBG_TESTSVPYIELDMUTEX();

    ImplSVData* pSVData = ImplGetSVData();
    if (pSVData->mpWakeCallback && pSVData->mpPollClosure)
        pSVData->mpWakeCallback(pSVData->mpPollClosure);

    SvpSalYieldMutex *const pMutex(static_cast<SvpSalYieldMutex*>(GetYieldMutex()));
    std::scoped_lock<std::mutex> g(pMutex->m_WakeUpMainMutex);
    if (request != SvpRequest::NONE)
        pMutex->m_Request = request;
    pMutex->m_wakeUpMain = true;
    pMutex->m_WakeUpMainCond.notify_one();
}

bool SvpSalInstance::CheckTimeout( bool bExecuteTimers )
{
    bool bRet = false;
    if( m_aTimeout.tv_sec ) // timer is started
    {
        timeval aTimeOfDay;
        gettimeofday( &aTimeOfDay, nullptr );
        if( aTimeOfDay >= m_aTimeout )
        {
            bRet = true;
            if( bExecuteTimers )
            {
                // timed out, update timeout
                m_aTimeout = aTimeOfDay;
                m_aTimeout += m_nTimeoutMS;

                osl::Guard< comphelper::SolarMutex > aGuard( GetYieldMutex() );

                // notify
                ImplSVData* pSVData = ImplGetSVData();
                if( pSVData->maSchedCtx.mpSalTimer )
                    pSVData->maSchedCtx.mpSalTimer->CallCallback();
            }
        }
    }
    return bRet;
}

SalFrame* SvpSalInstance::CreateChildFrame( SystemParentData* /*pParent*/, SalFrameStyleFlags nStyle )
{
    return new SvpSalFrame( this, nullptr, nStyle );
}

SalFrame* SvpSalInstance::CreateFrame( SalFrame* pParent, SalFrameStyleFlags nStyle )
{
    return new SvpSalFrame( this, pParent, nStyle );
}

void SvpSalInstance::DestroyFrame( SalFrame* pFrame )
{
    delete pFrame;
}

SalObject* SvpSalInstance::CreateObject( SalFrame*, SystemWindowData*, bool )
{
    return new SvpSalObject;
}

void SvpSalInstance::DestroyObject( SalObject* pObject )
{
    delete pObject;
}

#ifndef IOS

std::unique_ptr<SalVirtualDevice> SvpSalInstance::CreateVirtualDevice(SalGraphics& rGraphics,
                                                       tools::Long &nDX, tools::Long &nDY,
                                                       DeviceFormat /*eFormat*/,
                                                       const SystemGraphicsData* pGd)
{
    SvpSalGraphics *pSvpSalGraphics = dynamic_cast<SvpSalGraphics*>(&rGraphics);
    assert(pSvpSalGraphics);
#ifndef ANDROID
    // tdf#127529 normally pPreExistingTarget is null and we are a true virtualdevice drawing to a backing buffer.
    // Occasionally, for canvas/slideshow, pPreExistingTarget is pre-provided as a hack to use the vcl drawing
    // apis to render onto a preexisting cairo surface. The necessity for that precedes the use of cairo in vcl proper
    cairo_surface_t* pPreExistingTarget = pGd ? static_cast<cairo_surface_t*>(pGd->pSurface) : nullptr;
#else
    //ANDROID case
    (void)pGd;
    cairo_surface_t* pPreExistingTarget = nullptr;
#endif
    std::unique_ptr<SalVirtualDevice> xNew(new SvpSalVirtualDevice(pSvpSalGraphics->getSurface(), pPreExistingTarget));
    if (!xNew->SetSize(nDX, nDY))
        xNew.reset();
    return xNew;
}

cairo_surface_t* get_underlying_cairo_surface(const VirtualDevice& rDevice)
{
    return static_cast<SvpSalVirtualDevice*>(rDevice.mpVirDev.get())->GetSurface();
}

const cairo_font_options_t* SvpSalInstance::GetCairoFontOptions()
{
    static cairo_font_options_t *gOptions = nullptr;
    if (!gOptions)
    {
        gOptions = cairo_font_options_create();
        cairo_font_options_set_antialias(gOptions, CAIRO_ANTIALIAS_GRAY);
    }
    return gOptions;
}

#else // IOS

const cairo_font_options_t* SvpSalInstance::GetCairoFontOptions()
{
    return nullptr;
}

#endif

SalTimer* SvpSalInstance::CreateSalTimer()
{
    return new SvpSalTimer( this );
}

SalSystem* SvpSalInstance::CreateSalSystem()
{
    return new SvpSalSystem();
}

std::shared_ptr<SalBitmap> SvpSalInstance::CreateSalBitmap()
{
#ifdef IOS
    return std::make_shared<QuartzSalBitmap>();
#else
    return std::make_shared<SvpSalBitmap>();
#endif
}

void SvpSalInstance::ProcessEvent( SalUserEvent aEvent )
{
    DBG_TESTSVPYIELDMUTEX();

    aEvent.m_pFrame->CallCallback( aEvent.m_nEvent, aEvent.m_pData );
    if( aEvent.m_nEvent == SalEvent::Resize )
    {
        // this would be a good time to post a paint
        const SvpSalFrame* pSvpFrame = static_cast<const SvpSalFrame*>( aEvent.m_pFrame);
        pSvpFrame->PostPaint();
    }

    SvpSalYieldMutex *const pMutex(static_cast<SvpSalYieldMutex*>(GetYieldMutex()));
    pMutex->m_NonMainWaitingYieldCond.set();
}

SvpSalYieldMutex::SvpSalYieldMutex()
{
}

SvpSalYieldMutex::~SvpSalYieldMutex()
{
}

void SvpSalYieldMutex::doAcquire(sal_uInt32 const nLockCount)
{
    auto *const pInst = static_cast<SvpSalInstance*>(GetSalInstance());
    if (pInst && pInst->IsMainThread())
    {
        if (m_bNoYieldLock)
            return;

        do
        {
            SvpRequest request = SvpRequest::NONE;
            {
                std::unique_lock<std::mutex> g(m_WakeUpMainMutex);
                if (m_aMutex.tryToAcquire()) {
                    // if there's a request, the other thread holds m_aMutex
                    assert(m_Request == SvpRequest::NONE);
                    m_wakeUpMain = false;
                    break;
                }
                m_WakeUpMainCond.wait(g, [this]() { return m_wakeUpMain; });
                m_wakeUpMain = false;
                std::swap(m_Request, request);
            }
            if (request != SvpRequest::NONE)
            {
                // nested Yield on behalf of another thread
                assert(!m_bNoYieldLock);
                m_bNoYieldLock = true;
                bool const bEvents = pInst->DoYield(false, request == SvpRequest::MainThreadDispatchAllEvents);
                m_bNoYieldLock = false;
                {
                    std::lock_guard lock(m_FeedbackMutex);
                    m_FeedbackPipe.push(bEvents);
                }
                m_FeedbackCV.notify_all();
            }
        }
        while (true);
    }
    else
    {
        m_aMutex.acquire();
    }
    ++m_nCount;
    SalYieldMutex::doAcquire(nLockCount - 1);
}

sal_uInt32 SvpSalYieldMutex::doRelease(bool const bUnlockAll)
{
    auto *const pInst = static_cast<SvpSalInstance*>(GetSalInstance());
    if (pInst && pInst->IsMainThread())
    {
        if (m_bNoYieldLock)
            return 1;
        else
            return SalYieldMutex::doRelease(bUnlockAll);
    }
    sal_uInt32 nCount;
    {
        // read m_nCount before doRelease
        bool const isReleased(bUnlockAll || m_nCount == 1);
        nCount = comphelper::SolarMutex::doRelease( bUnlockAll );

        if (isReleased)
        {
            if (vcl::lok::isUnipoll())
            {
                if (pInst)
                    pInst->Wakeup();
            }
            else
            {
                std::scoped_lock<std::mutex> g(m_WakeUpMainMutex);
                m_wakeUpMain = true;
                m_WakeUpMainCond.notify_one();
            }
        }
    }
    return nCount;
}

bool SvpSalYieldMutex::IsCurrentThread() const
{
    if (GetSalInstance()->IsMainThread() && m_bNoYieldLock)
        return true;
    else
        return SalYieldMutex::IsCurrentThread();
}

bool SvpSalInstance::IsMainThread() const
{
    return osl::Thread::getCurrentIdentifier() == m_MainThread;
}

void SvpSalInstance::updateMainThread()
{
    if (!IsMainThread())
    {
        m_MainThread = osl::Thread::getCurrentIdentifier();
        ImplGetSVData()->mnMainThreadId = osl::Thread::getCurrentIdentifier();
    }
}

bool SvpSalInstance::ImplYield(bool bWait, bool bHandleAllCurrentEvents)
{
    DBG_TESTSVPYIELDMUTEX();
    DBG_TESTSOLARMUTEX();
    assert(IsMainThread());

    bool bWasEvent = DispatchUserEvents(bHandleAllCurrentEvents);
    if (!bHandleAllCurrentEvents && bWasEvent)
        return true;

    bWasEvent = CheckTimeout() || bWasEvent;
    const bool bMustSleep = bWait && !bWasEvent;

    // This is wrong and must be removed!
    // We always want to drop the SolarMutex on yield; that is the whole point of yield.
    if (!bMustSleep)
        return bWasEvent;

    sal_Int64 nTimeoutMicroS = 0;
    if (bMustSleep)
    {
        if (m_aTimeout.tv_sec) // Timer is started.
        {
            timeval Timeout;
            // determine remaining timeout.
            gettimeofday (&Timeout, nullptr);
            if (m_aTimeout > Timeout)
                nTimeoutMicroS = ((m_aTimeout.tv_sec - Timeout.tv_sec) * 1000 * 1000 +
                                  (m_aTimeout.tv_usec - Timeout.tv_usec));
        }
        else
            nTimeoutMicroS = -1; // wait until something happens
    }

    SolarMutexReleaser aReleaser;

    if (vcl::lok::isUnipoll())
    {
        ImplSVData* pSVData = ImplGetSVData();
        if (pSVData->mpPollClosure)
        {
            int nPollResult = pSVData->mpPollCallback(pSVData->mpPollClosure, nTimeoutMicroS);
            if (nPollResult < 0)
                pSVData->maAppData.mbAppQuit = true;
            bWasEvent = bWasEvent || (nPollResult != 0);
        }
    }
    else if (bMustSleep)
    {
        SvpSalYieldMutex *const pMutex(static_cast<SvpSalYieldMutex*>(GetYieldMutex()));
        std::unique_lock<std::mutex> g(pMutex->m_WakeUpMainMutex);
        // wait for doRelease() or Wakeup() to set the condition
        if (nTimeoutMicroS == -1)
        {
            pMutex->m_WakeUpMainCond.wait(g,
                    [pMutex]() { return pMutex->m_wakeUpMain; });
        }
        else
        {
            int nTimeoutMS = nTimeoutMicroS / 1000;
            if (nTimeoutMicroS % 1000)
                nTimeoutMS += 1;
            pMutex->m_WakeUpMainCond.wait_for(g,
                    std::chrono::milliseconds(nTimeoutMS),
                    [pMutex]() { return pMutex->m_wakeUpMain; });
        }
        // here no need to check m_Request because Acquire will do it
    }

    return bWasEvent;
}

bool SvpSalInstance::DoYield(bool bWait, bool bHandleAllCurrentEvents)
{
    DBG_TESTSVPYIELDMUTEX();
    DBG_TESTSOLARMUTEX();

    bool bWasEvent(false);
    SvpSalYieldMutex *const pMutex(static_cast<SvpSalYieldMutex*>(GetYieldMutex()));

    if (IsMainThread())
    {
        bWasEvent = ImplYield(bWait, bHandleAllCurrentEvents);
        if (bWasEvent)
            pMutex->m_NonMainWaitingYieldCond.set(); // wake up other threads
    }
    else
    {
        // TODO: use a SolarMutexReleaser here and drop the m_bNoYieldLock usage
        Wakeup(bHandleAllCurrentEvents
                ? SvpRequest::MainThreadDispatchAllEvents
                : SvpRequest::MainThreadDispatchOneEvent);

        // blocking read (for synchronisation)
        {
            std::unique_lock lock(pMutex->m_FeedbackMutex);
            pMutex->m_FeedbackCV.wait(lock, [pMutex] { return !pMutex->m_FeedbackPipe.empty(); });
            bWasEvent = pMutex->m_FeedbackPipe.front();
            pMutex->m_FeedbackPipe.pop();
        }
        if (!bWasEvent && bWait)
        {
            // block & release YieldMutex until the main thread does something
            pMutex->m_NonMainWaitingYieldCond.reset();
            SolarMutexReleaser aReleaser;
            pMutex->m_NonMainWaitingYieldCond.wait();
        }
    }

    return bWasEvent;
}

bool SvpSalInstance::AnyInput( VclInputFlags nType )
{
    if( nType & VclInputFlags::TIMER )
        return CheckTimeout( false );
    return false;
}

OUString SvpSalInstance::GetConnectionIdentifier()
{
    return OUString();
}

void SvpSalInstance::StopTimer()
{
    m_aTimeout.tv_sec   = 0;
    m_aTimeout.tv_usec  = 0;
    m_nTimeoutMS        = 0;
}

void SvpSalInstance::StartTimer( sal_uInt64 nMS )
{
    timeval aPrevTimeout (m_aTimeout);
    gettimeofday (&m_aTimeout, nullptr);

    m_nTimeoutMS  = nMS;
    m_aTimeout    += m_nTimeoutMS;

    if ((aPrevTimeout > m_aTimeout) || (aPrevTimeout.tv_sec == 0))
    {
        // Wakeup from previous timeout (or stopped timer).
        Wakeup();
    }
}

void SvpSalInstance::AddToRecentDocumentList(const OUString&, const OUString&, const OUString&)
{
}

SvpSalTimer::~SvpSalTimer()
{
}

void SvpSalTimer::Stop()
{
    m_pInstance->StopTimer();
}

void SvpSalTimer::Start( sal_uInt64 nMS )
{
    m_pInstance->StartTimer( nMS );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
