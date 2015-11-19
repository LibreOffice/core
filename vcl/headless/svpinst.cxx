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
#include <fcntl.h>
#include <sys/time.h>
#include <sys/poll.h>

#include <sal/types.h>

#include <vcl/apptypes.hxx>

#include "headless/svpinst.hxx"
#include "headless/svpframe.hxx"
#include "headless/svpdummies.hxx"
#include "headless/svpvd.hxx"
#ifdef IOS
#include "quartz/salbmp.h"
#include "quartz/salgdi.h"
#include "quartz/salvd.h"
#endif
#include "headless/svpbmp.hxx"
#include "headless/svpgdi.hxx"

#include <salframe.hxx>
#include <svdata.hxx>
#include <generic/gendata.hxx>
#include <basebmp/scanlineformats.hxx>
// FIXME: remove when we re-work the svp mainloop
#include <unx/salunxtime.h>

using namespace basebmp;

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

SvpSalInstance::SvpSalInstance( SalYieldMutex *pMutex ) :
    SalGenericInstance( pMutex )
{
    m_aTimeout.tv_sec       = 0;
    m_aTimeout.tv_usec      = 0;
    m_nTimeoutMS            = 0;

    m_pTimeoutFDS[0] = m_pTimeoutFDS[1] = -1;
    if (pipe (m_pTimeoutFDS) != -1)
    {
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
    m_aEventGuard = osl_createMutex();
    if( s_pDefaultInstance == nullptr )
        s_pDefaultInstance = this;
}

SvpSalInstance::~SvpSalInstance()
{
    if( s_pDefaultInstance == this )
        s_pDefaultInstance = nullptr;

    // close 'wakeup' pipe.
    close (m_pTimeoutFDS[0]);
    close (m_pTimeoutFDS[1]);
    osl_destroyMutex( m_aEventGuard );
}

void SvpSalInstance::PostEvent(const SalFrame* pFrame, ImplSVEvent* pData, sal_uInt16 nEvent)
{
    if( osl_acquireMutex( m_aEventGuard ) )
    {
        m_aUserEvents.push_back( SalUserEvent( pFrame, pData, nEvent ) );
        osl_releaseMutex( m_aEventGuard );
    }
    Wakeup();
}

#ifdef ANDROID
bool SvpSalInstance::PostedEventsInQueue()
{
    bool result = false;
    if( osl_acquireMutex( m_aEventGuard ) )
    {
        result = m_aUserEvents.size() > 0;
        osl_releaseMutex( m_aEventGuard );
    }
    return result;
}
#endif

void SvpSalInstance::deregisterFrame( SalFrame* pFrame )
{
    m_aFrames.remove( pFrame );

    if( osl_acquireMutex( m_aEventGuard ) )
    {
        // cancel outstanding events for this frame
        if( ! m_aUserEvents.empty() )
        {
            std::list< SalUserEvent >::iterator it = m_aUserEvents.begin();
            do
            {
                if( it->m_pFrame    == pFrame )
                {
                    it = m_aUserEvents.erase( it );
                }
                else
                    ++it;
            } while( it != m_aUserEvents.end() );
        }
        osl_releaseMutex( m_aEventGuard );
    }
}

void SvpSalInstance::Wakeup()
{
    OSL_VERIFY(write (m_pTimeoutFDS[1], "", 1) == 1);
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
    return new SvpSalFrame( this, nullptr, nStyle, SVP_CAIRO_FORMAT, pParent );
}

SalFrame* SvpSalInstance::CreateFrame( SalFrame* pParent, SalFrameStyleFlags nStyle )
{
    return new SvpSalFrame( this, pParent, nStyle, SVP_CAIRO_FORMAT );
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

SalI18NImeStatus* SvpSalInstance::CreateI18NImeStatus()
{
    return new SvpImeStatus();
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
    if( osl_acquireMutex( m_aEventGuard ) )
    {
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
        osl_releaseMutex( m_aEventGuard );
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
                if( it->m_nEvent == SALEVENT_RESIZE )
                {
                    // this would be a good time to post a paint
                    const SvpSalFrame* pSvpFrame = static_cast<const SvpSalFrame*>(it->m_pFrame);
                    pSvpFrame->PostPaint(false);
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

Format SvpSalInstance::getBaseBmpFormatForBitCount( sal_uInt16 nBitCount )
{
    switch( nBitCount )
    {
        case 1:
            return Format::OneBitMsbPal;
        case 4:
            return Format::FourBitMsbPal;
        case 8:
            return Format::EightBitPal;
        case 16:
#ifdef OSL_BIGENDIAN
            return Format::SixteenBitMsbTcMask;
#else
            return Format::SixteenBitLsbTcMask;
#endif
        case 32:
            return Format::ThirtyTwoBitTcMaskBGRA;
        default:
            return SVP_CAIRO_FORMAT;
     }

}

Format SvpSalInstance::getBaseBmpFormatForDeviceFormat(DeviceFormat eFormat)
{
    switch (eFormat)
    {
        case DeviceFormat::BITMASK:
            return Format::OneBitMsbPal;
        default:
            return SVP_CAIRO_FORMAT;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
