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

#include "salframe.hxx"
#include "svdata.hxx"
#include "unx/gendata.hxx"
// FIXME: remove when we re-work the svp mainloop
#include "unx/salunxtime.h"

bool SvpSalInstance::isFrameAlive( const SalFrame* pFrame ) const
{
    for( std::list< SalFrame* >::const_iterator it = m_aFrames.begin();
         it != m_aFrames.end(); ++it )
    {
        if( *it == pFrame )
        {
            return true;
        }
    }
    return false;
}

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

void SvpSalInstance::PostEvent(const SalFrame* pFrame, ImplSVEvent* pData, SalEvent nEvent)
{
    {
        osl::MutexGuard g(m_aEventGuard);
        m_aUserEvents.push_back( SalUserEvent( pFrame, pData, nEvent ) );
    }
    Wakeup();
}

#ifdef ANDROID
bool SvpSalInstance::PostedEventsInQueue()
{
    bool result = false;
    {
        osl::MutexGuard g(m_aEventGuard);
        result = m_aUserEvents.size() > 0;
    }
    return result;
}
#endif

void SvpSalInstance::deregisterFrame( SalFrame* pFrame )
{
    m_aFrames.remove( pFrame );

    osl::MutexGuard g(m_aEventGuard);
    // cancel outstanding events for this frame
    if( ! m_aUserEvents.empty() )
    {
        std::list< SalUserEvent >::iterator it = m_aUserEvents.begin();
        do
        {
            if( it->m_pFrame == pFrame )
            {
                if (it->m_nEvent == SalEvent::UserEvent)
                {
                    delete it->m_pData;
                }
                it = m_aUserEvents.erase( it );
            }
            else
                ++it;
        } while( it != m_aUserEvents.end() );
    }
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

                osl::Guard< comphelper::SolarMutex > aGuard( mpSalYieldMutex );

                // notify
                ImplSVData* pSVData = ImplGetSVData();
                if( pSVData->mpSalTimer )
                {
                    bool idle = true; // TODO
                    pSVData->mpSalTimer->CallCallback( idle );
                }
            }
        }
    }
    return bRet;
}

SalFrame* SvpSalInstance::CreateChildFrame( SystemParentData* pParent, SalFrameStyleFlags nStyle )
{
    return new SvpSalFrame( this, nullptr, nStyle, pParent );
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

SalVirtualDevice* SvpSalInstance::CreateVirtualDevice( SalGraphics* /* pGraphics */,
                                                       long &nDX, long &nDY,
                                                       DeviceFormat eFormat,
                                                       const SystemGraphicsData* /* pData */ )
{
    SvpSalVirtualDevice* pNew = new SvpSalVirtualDevice(eFormat);
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

bool SvpSalInstance::DoYield(bool bWait, bool bHandleAllCurrentEvents, sal_uLong const nReleased)
{
    (void) nReleased;
    assert(nReleased == 0); // not implemented
    // first, check for already queued events.

    // release yield mutex
    std::list< SalUserEvent > aEvents;
    sal_uLong nAcquireCount = ReleaseYieldMutex();
    {
        osl::MutexGuard g(m_aEventGuard);
        if( ! m_aUserEvents.empty() )
        {
            if( bHandleAllCurrentEvents )
            {
                aEvents = m_aUserEvents;
                m_aUserEvents.clear();
            }
            else
            {
                aEvents.push_back( m_aUserEvents.front() );
                m_aUserEvents.pop_front();
            }
        }
    }
    // acquire yield mutex again
    AcquireYieldMutex( nAcquireCount );

    bool bEvent = !aEvents.empty();
    if( bEvent )
    {
        for( std::list<SalUserEvent>::const_iterator it = aEvents.begin(); it != aEvents.end(); ++it )
        {
            if ( isFrameAlive( it->m_pFrame ) )
            {
                it->m_pFrame->CallCallback( it->m_nEvent, it->m_pData );
                if( it->m_nEvent == SalEvent::Resize )
                {
                    // this would be a good time to post a paint
                    const SvpSalFrame* pSvpFrame = static_cast<const SvpSalFrame*>(it->m_pFrame);
                    pSvpFrame->PostPaint();
                }
            }
        }
    }

    bEvent = CheckTimeout() || bEvent;

    if (bWait && ! bEvent )
    {
        int nTimeoutMS = 0;
        if (m_aTimeout.tv_sec) // Timer is started.
        {
            timeval Timeout;
            // determine remaining timeout.
            gettimeofday (&Timeout, nullptr);
            nTimeoutMS = (m_aTimeout.tv_sec - Timeout.tv_sec) * 1000
                         + m_aTimeout.tv_usec/1000 - Timeout.tv_usec/1000;
            if( nTimeoutMS < 0 )
                nTimeoutMS = 0;
        }
        else
            nTimeoutMS = -1; // wait until something happens

        DoReleaseYield(nTimeoutMS);
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
    sal_uLong nAcquireCount = ReleaseYieldMutex();

    (void)poll( &aPoll, 1, nTimeoutMS );

    // acquire yield mutex again
    AcquireYieldMutex( nAcquireCount );

    // clean up pipe
    if( (aPoll.revents & POLLIN) != 0 )
    {
        int buffer;
        while (read (m_pTimeoutFDS[0], &buffer, sizeof(buffer)) > 0)
            continue;
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

void* SvpSalInstance::GetConnectionIdentifier( ConnectionIdentifierType& rReturnedType, int& rReturnedBytes )
{
    rReturnedBytes  = 1;
    rReturnedType   = AsciiCString;
    return const_cast<char*>("");
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
