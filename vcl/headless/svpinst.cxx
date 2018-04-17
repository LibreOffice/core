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

#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/poll.h>

#include <sal/types.h>

#include <vcl/inputtypes.hxx>
#include <vcl/opengl/OpenGLContext.hxx>

#include <headless/svpinst.hxx>
#include <headless/svpframe.hxx>
#include <headless/svpdummies.hxx>
#include <headless/svpvd.hxx>
#ifdef IOS
#include <quartz/salbmp.h>
#include <quartz/salgdi.h>
#include <quartz/salvd.h>
#endif
#include <headless/svpbmp.hxx>
#include <headless/svpgdi.hxx>

#include <salframe.hxx>
#include <svdata.hxx>
#include <unx/gendata.hxx>
// FIXME: remove when we re-work the svp mainloop
#include <unx/salunxtime.h>

SvpSalInstance* SvpSalInstance::s_pDefaultInstance = nullptr;

#if !defined(ANDROID) && !defined(IOS)

static void atfork_child()
{
    if (SvpSalInstance::s_pDefaultInstance != nullptr)
    {
        SvpSalInstance::s_pDefaultInstance->CloseWakeupPipe(false);
        SvpSalInstance::s_pDefaultInstance->CreateWakeupPipe(false);
    }
}

#endif

SvpSalInstance::SvpSalInstance( SalYieldMutex *pMutex ) :
    SalGenericInstance( pMutex )
{
    m_aTimeout.tv_sec       = 0;
    m_aTimeout.tv_usec      = 0;
    m_nTimeoutMS            = 0;

#ifndef IOS
    m_pTimeoutFDS[0] = m_pTimeoutFDS[1] = -1;
    CreateWakeupPipe(true);
#endif
    if( s_pDefaultInstance == nullptr )
        s_pDefaultInstance = this;
#if !defined(ANDROID) && !defined(IOS)
    pthread_atfork(nullptr, nullptr, atfork_child);
#endif
}

SvpSalInstance::~SvpSalInstance()
{
    if( s_pDefaultInstance == this )
        s_pDefaultInstance = nullptr;
#ifndef IOS
    CloseWakeupPipe(true);
#endif
}

#ifndef IOS

void SvpSalInstance::CloseWakeupPipe(bool log)
{
    if (m_pTimeoutFDS[0] != -1)
    {
        if (log)
        {
            SAL_INFO("vcl.headless", "CloseWakeupPipe: Closing inherited wakeup pipe: [" << m_pTimeoutFDS[0] << "," << m_pTimeoutFDS[1] << "]");
        }
        close (m_pTimeoutFDS[0]);
        close (m_pTimeoutFDS[1]);
        m_pTimeoutFDS[0] = m_pTimeoutFDS[1] = -1;
    }
}

void SvpSalInstance::CreateWakeupPipe(bool log)
{
    if (pipe (m_pTimeoutFDS) == -1)
    {
        if (log)
        {
            SAL_WARN("vcl.headless", "Could not create wakeup pipe: " << strerror(errno));
        }
    }
    else
    {
        if (log)
        {
            SAL_INFO("vcl.headless", "CreateWakeupPipe: Created wakeup pipe: [" << m_pTimeoutFDS[0] << "," << m_pTimeoutFDS[1] << "]");
        }

        // initialize 'wakeup' pipe.
        int flags;

        // set close-on-exec descriptor flag.
        if ((flags = fcntl (m_pTimeoutFDS[0], F_GETFD)) != -1)
        {
            flags |= FD_CLOEXEC;
            (void)fcntl(m_pTimeoutFDS[0], F_SETFD, flags);
        }
        if ((flags = fcntl (m_pTimeoutFDS[1], F_GETFD)) != -1)
        {
            flags |= FD_CLOEXEC;
            (void)fcntl(m_pTimeoutFDS[1], F_SETFD, flags);
        }

        // set non-blocking I/O flag.
        if ((flags = fcntl(m_pTimeoutFDS[0], F_GETFL)) != -1)
        {
            flags |= O_NONBLOCK;
            (void)fcntl(m_pTimeoutFDS[0], F_SETFL, flags);
        }
        if ((flags = fcntl(m_pTimeoutFDS[1], F_GETFL)) != -1)
        {
            flags |= O_NONBLOCK;
            (void)fcntl(m_pTimeoutFDS[1], F_SETFL, flags);
        }
    }
}

#endif

void SvpSalInstance::TriggerUserEventProcessing()
{
    Wakeup();
}

void SvpSalInstance::Wakeup()
{
#ifndef IOS
    OSL_VERIFY(write (m_pTimeoutFDS[1], "", 1) == 1);
#endif
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

                osl::Guard< comphelper::SolarMutex > aGuard( mpSalYieldMutex.get() );

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
    return new SvpSalObject();
}

void SvpSalInstance::DestroyObject( SalObject* pObject )
{
    delete pObject;
}

#ifndef IOS

SalVirtualDevice* SvpSalInstance::CreateVirtualDevice( SalGraphics* pGraphics,
                                                       long &nDX, long &nDY,
                                                       DeviceFormat eFormat,
                                                       const SystemGraphicsData* /* pData */ )
{
    SvpSalGraphics *pSvpSalGraphics = dynamic_cast<SvpSalGraphics*>(pGraphics);
    assert(pSvpSalGraphics);
    SvpSalVirtualDevice* pNew = new SvpSalVirtualDevice(eFormat, pSvpSalGraphics->getSurface());
    pNew->SetSize( nDX, nDY );
    return pNew;
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

SalBitmap* SvpSalInstance::CreateSalBitmap()
{
#ifdef IOS
    return new QuartzSalBitmap();
#else
    return new SvpSalBitmap();
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


bool SvpSalInstance::DoYield(bool bWait, bool bHandleAllCurrentEvents)
{
    // first, process current user events
    bool bEvent = DispatchUserEvents( bHandleAllCurrentEvents );
    if ( !bHandleAllCurrentEvents && bEvent )
        return true;

    bEvent = CheckTimeout() || bEvent;

    if (bWait && ! bEvent )
    {
        int nTimeoutMS = 0;
        if (m_aTimeout.tv_sec) // Timer is started.
        {
            timeval Timeout;
            // determine remaining timeout.
            gettimeofday (&Timeout, nullptr);
            if ( m_aTimeout > Timeout )
            {
                int nTimeoutMicroS = m_aTimeout.tv_usec - Timeout.tv_usec;
                nTimeoutMS = (m_aTimeout.tv_sec - Timeout.tv_sec) * 1000
                           + nTimeoutMicroS / 1000;
                if ( nTimeoutMicroS % 1000 )
                    nTimeoutMS += 1;
            }
        }
        else
            nTimeoutMS = -1; // wait until something happens

        DoReleaseYield(nTimeoutMS);
    }
    else if ( bEvent )
    {
        // Drain the wakeup pipe
        int buffer;
        while (read (m_pTimeoutFDS[0], &buffer, sizeof(buffer)) > 0);
    }

    return bEvent;
}

void SvpSalInstance::DoReleaseYield( int nTimeoutMS )
{
    // poll
    struct pollfd aPoll;
    aPoll.fd = m_pTimeoutFDS[0];
    aPoll.events = POLLIN;
    aPoll.revents = 0;

    // release yield mutex
    sal_uInt32 nAcquireCount = ReleaseYieldMutexAll();

    (void)poll( &aPoll, 1, nTimeoutMS );

    // acquire yield mutex again
    AcquireYieldMutex( nAcquireCount );

    // clean up pipe
    if( (aPoll.revents & POLLIN) != 0 )
    {
        int buffer;
        while (read (m_pTimeoutFDS[0], &buffer, sizeof(buffer)) > 0);
    }
}

bool SvpSalInstance::AnyInput( VclInputFlags nType )
{
    if( nType & VclInputFlags::TIMER )
        return CheckTimeout( false );
    return false;
}

SalSession* SvpSalInstance::CreateSalSession()
{
    return nullptr;
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

void SvpSalInstance::StartTimer( sal_uLong nMS )
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

//obviously doesn't actually do anything, its just a nonfunctional stub

#ifdef LIBO_HEADLESS

class SvpOpenGLContext
{
};

OpenGLContext* SvpSalInstance::CreateOpenGLContext()
{
    return nullptr;
}

#else

class SvpOpenGLContext : public OpenGLContext
{
    GLWindow m_aGLWin;
private:
    virtual const GLWindow& getOpenGLWindow() const override { return m_aGLWin; }
    virtual GLWindow& getModifiableOpenGLWindow() override { return m_aGLWin; }
};

OpenGLContext* SvpSalInstance::CreateOpenGLContext()
{
    return new SvpOpenGLContext;
}

#endif

SvpSalTimer::~SvpSalTimer()
{
}

void SvpSalTimer::Stop()
{
    m_pInstance->StopTimer();
}

void SvpSalTimer::Start( sal_uLong nMS )
{
    m_pInstance->StartTimer( nMS );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
