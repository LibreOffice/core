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

#include <config_features.h>
#include <sal/config.h>

#include <mutex>

#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/poll.h>

#include <sal/types.h>
#include <sal/log.hxx>

#include <vcl/virdev.hxx>
#include <vcl/inputtypes.hxx>
#include <vcl/lok.hxx>
#if HAVE_FEATURE_UI
# include <vcl/opengl/OpenGLContext.hxx>
#endif

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
#include <unx/gendata.hxx>
// FIXME: remove when we re-work the svp mainloop
#include <unx/salunxtime.h>
#include <comphelper/lok.hxx>
#include <tools/debug.hxx>
#include <tools/time.hxx>

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

SvpSalInstance::SvpSalInstance( std::unique_ptr<SalYieldMutex> pMutex )
    : SalGenericInstance( std::move(pMutex) )
{
    m_aTimeout.tv_sec       = 0;
    m_aTimeout.tv_usec      = 0;
    m_nTimeoutMS            = 0;

    m_MainThread = osl::Thread::getCurrentIdentifier();
    if( s_pDefaultInstance == nullptr )
        s_pDefaultInstance = this;
}

SvpSalInstance::~SvpSalInstance()
{
    if( s_pDefaultInstance == this )
        s_pDefaultInstance = nullptr;
}

void SvpSalInstance::TriggerUserEventProcessing()
{
    Wakeup();
}

void SvpSalInstance::Wakeup()
{
    if (IsMainThread())
        return;

    if (vcl::lok::isUnipoll())
    {
        ImplSVData* pSVData = ImplGetSVData();
        if (pSVData->mpWakeCallback && pSVData->mpPollClosure)
            pSVData->mpWakeCallback(pSVData->mpPollClosure);
    }
    else
    {
        DBG_TESTSVPYIELDMUTEX();
        SvpSalYieldMutex *const pMutex(static_cast<SvpSalYieldMutex*>(GetYieldMutex()));
        pMutex->m_MainYieldCondition.notify_all();
    }
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
    std::unique_ptr<SalVirtualDevice> pNew(new SvpSalVirtualDevice(pSvpSalGraphics->getSurface(), pPreExistingTarget));
    pNew->SetSize( nDX, nDY );
    return pNew;
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
    aEvent.m_pFrame->CallCallback( aEvent.m_nEvent, aEvent.m_pData );
    if( aEvent.m_nEvent == SalEvent::Resize )
    {
        // this would be a good time to post a paint
        const SvpSalFrame* pSvpFrame = static_cast<const SvpSalFrame*>( aEvent.m_pFrame);
        pSvpFrame->PostPaint();
    }
}

SvpSalYieldMutex::SvpSalYieldMutex()
    : m_WaitCondition(m_NonMainYieldMutex)
    , m_EventCondition(m_NonMainYieldMutex)
    , m_MainYieldCondition(m_MainYieldMutex)
{
}

SvpSalYieldMutex::~SvpSalYieldMutex()
{
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

bool SvpSalInstance::ImplYield(bool bWait)
{
    DBG_TESTSVPYIELDMUTEX();
    DBG_TESTSOLARMUTEX();
    assert(IsMainThread());

    bool bWasEvent = DispatchUserEvents(true);
    const bool bTimeout = CheckTimeout();
    bWasEvent = bTimeout || bWasEvent;

    sal_Int64 nTimeoutMicroS = 0;
    const bool bMustSleep = bWait && !bWasEvent;
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

    SvpSalYieldMutex *const pMutex(static_cast<SvpSalYieldMutex*>(GetYieldMutex()));
    pMutex->m_bWasEvent = bWasEvent;

    if (vcl::lok::isUnipoll())
    {
        ImplSVData* pSVData = ImplGetSVData();
        if (pSVData->mpPollClosure)
        {
            int nPollResult = 0;
            if (!bMustSleep || (nTimeoutMicroS != 0))
                nPollResult = pSVData->mpPollCallback(pSVData->mpPollClosure, 0);
            if (bMustSleep && nPollResult == 0)
            {
                pMutex->m_EventCondition.notify_all();
                nPollResult = pSVData->mpPollCallback(pSVData->mpPollClosure, nTimeoutMicroS);
            }
            if (nPollResult < 0)
                pSVData->maAppData.mbAppQuit = true;
            bWasEvent = bWasEvent || (nPollResult != 0);
        }
    }
    else if (bMustSleep)
    {
        assert(!bWasEvent);
        if (nTimeoutMicroS == 0)
        {
            pMutex->m_EventCondition.notify_all();
            return bWasEvent;
        }

        auto wakeup = [pMutex]() { pMutex->m_EventCondition.notify_all(); return false; };

        if (nTimeoutMicroS == -1)
            pMutex->m_MainYieldCondition.wait(wakeup);
        else
        {
            int nTimeoutMS = nTimeoutMicroS / 1000;
            if (nTimeoutMicroS % 1000)
                nTimeoutMS += 1;
            pMutex->m_MainYieldCondition.wait_for(std::chrono::milliseconds(nTimeoutMS), wakeup);
        }
    }

    if (bWasEvent)
        pMutex->m_WaitCondition.notify_all();
    pMutex->m_EventCondition.notify_all();

    return bWasEvent;
}

bool SvpSalInstance::DoYield(bool bWait, bool)
{
    DBG_TESTSVPYIELDMUTEX();
    DBG_TESTSOLARMUTEX();

    bool bWasEvent(false);
    SvpSalYieldMutex *const pMutex(static_cast<SvpSalYieldMutex*>(GetYieldMutex()));

    if (IsMainThread())
        bWasEvent = ImplYield(bWait);
    else
    {
        SolarMutexReleaser aReleaser;
        if (bWait)
        {
            pMutex->m_WaitCondition.wait();
            bWasEvent = true;
        }
        else
        {
            // prevent ABBA deadlock
            std::unique_lock<std::mutex> g(pMutex->m_MainYieldCondition.mutex());
            const bool bIsSleeping = pMutex->m_MainYieldCondition.isSleeping();
            pMutex->m_EventCondition.wait([bIsSleeping, &g]()
            {
                g.unlock();
                return bIsSleeping;
            });
            if (bIsSleeping)
                bWasEvent = false;
            else
                bWasEvent = pMutex->m_bWasEvent;
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

std::shared_ptr<vcl::BackendCapabilities> SvpSalInstance::GetBackendCapabilities()
{
    auto pBackendCapabilities = SalInstance::GetBackendCapabilities();
#if 0 // LO code is not yet bitmap32-ready.
#ifndef IOS
    // Note: This code is used for iOS, too. Let's not use 32-bit bitmaps with included alpha on iOS for now.
    pBackendCapabilities->mbSupportsBitmap32 = true;
#endif
#endif
    return pBackendCapabilities;
}

//obviously doesn't actually do anything, it's just a nonfunctional stub

#if HAVE_FEATURE_UI

namespace {

class SvpOpenGLContext : public OpenGLContext
{
    GLWindow m_aGLWin;
private:
    virtual const GLWindow& getOpenGLWindow() const override { return m_aGLWin; }
    virtual GLWindow& getModifiableOpenGLWindow() override { return m_aGLWin; }
};

}

OpenGLContext* SvpSalInstance::CreateOpenGLContext()
{
    return new SvpOpenGLContext;
}

#else

class SvpOpenGLContext
{
};

OpenGLContext* SvpSalInstance::CreateOpenGLContext()
{
    return nullptr;
}

#endif

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
