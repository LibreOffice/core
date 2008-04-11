/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: svpinst.cxx,v $
 * $Revision: 1.5 $
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

#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/poll.h>

#include "svpinst.hxx"
#include "svpframe.hxx"
#include "svpdummies.hxx"
#include "svpvd.hxx"
#include "svpbmp.hxx"

#include <vcl/salframe.hxx>
#include <vcl/svdata.hxx>
#include <vcl/salatype.hxx>
#include <vcl/saldatabasic.hxx>
#include <sal/types.h>

// plugin factory function
extern "C"
{
    SAL_DLLPUBLIC_EXPORT SalInstance* create_SalInstance()
    {
        SvpSalInstance* pInstance = new SvpSalInstance();
        SalData* pSalData = new SalData();
        pSalData->m_pInstance = pInstance;
        SetSalData( pSalData );
        return pInstance;
    }
}

SvpSalInstance* SvpSalInstance::s_pDefaultInstance = NULL;

SvpSalInstance::SvpSalInstance()
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
            fcntl (m_pTimeoutFDS[0], F_SETFD, flags);
        }
        if ((flags = fcntl (m_pTimeoutFDS[1], F_GETFD)) != -1)
        {
            flags |= FD_CLOEXEC;
            fcntl (m_pTimeoutFDS[1], F_SETFD, flags);
        }

        // set non-blocking I/O flag.
        if ((flags = fcntl (m_pTimeoutFDS[0], F_GETFL)) != -1)
        {
            flags |= O_NONBLOCK;
            fcntl (m_pTimeoutFDS[0], F_SETFL, flags);
        }
        if ((flags = fcntl (m_pTimeoutFDS[1], F_GETFL)) != -1)
        {
            flags |= O_NONBLOCK;
            fcntl (m_pTimeoutFDS[1], F_SETFL, flags);
        }
    }
    m_aEventGuard = osl_createMutex();
    if( s_pDefaultInstance == NULL )
        s_pDefaultInstance = this;
}

SvpSalInstance::~SvpSalInstance()
{
    if( s_pDefaultInstance == this )
        s_pDefaultInstance = NULL;

    // close 'wakeup' pipe.
    close (m_pTimeoutFDS[0]);
    close (m_pTimeoutFDS[1]);
    osl_destroyMutex( m_aEventGuard );
}

void SvpSalInstance::PostEvent( const SalFrame* pFrame, void* pData, USHORT nEvent )
{
    if( osl_acquireMutex( m_aEventGuard ) )
    {
        m_aUserEvents.push_back( SalUserEvent( pFrame, pData, nEvent ) );
        osl_releaseMutex( m_aEventGuard );
    }
    Wakeup();
}

void SvpSalInstance::CancelEvent( const SalFrame* pFrame, void* pData, USHORT nEvent )
{
    if( osl_acquireMutex( m_aEventGuard ) )
    {
        if( ! m_aUserEvents.empty() )
        {
            std::list< SalUserEvent >::iterator it = m_aUserEvents.begin();
            do
            {
                if( it->m_pFrame    == pFrame   &&
                    it->m_pData     == pData    &&
                    it->m_nEvent    == nEvent )
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
    write (m_pTimeoutFDS[1], "", 1);
}


// -=-= timeval =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
inline int operator >= ( const timeval &t1, const timeval &t2 )
{
    if( t1.tv_sec == t2.tv_sec )
        return t1.tv_usec >= t2.tv_usec;
    return t1.tv_sec > t2.tv_sec;
}
inline timeval &operator += ( timeval &t1, ULONG t2 )
{
    t1.tv_sec  += t2 / 1000;
    t1.tv_usec += t2 ? (t2 % 1000) * 1000 : 500;
    if( t1.tv_usec > 1000000 )
    {
        t1.tv_sec++;
        t1.tv_usec -= 1000000;
    }
    return t1;
}
inline int operator > ( const timeval &t1, const timeval &t2 )
{
    if( t1.tv_sec == t2.tv_sec )
        return t1.tv_usec > t2.tv_usec;
    return t1.tv_sec > t2.tv_sec;
}

bool SvpSalInstance::CheckTimeout( bool bExecuteTimers )
{
    bool bRet = false;
    if( m_aTimeout.tv_sec ) // timer is started
    {
        timeval aTimeOfDay;
        gettimeofday( &aTimeOfDay, 0 );
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
                if( pSVData->mpSalTimer )
                    pSVData->mpSalTimer->CallCallback();
            }
        }
    }
    return bRet;
}

SalFrame* SvpSalInstance::CreateChildFrame( SystemParentData* pParent, ULONG nStyle )
{
    return new SvpSalFrame( this, NULL, nStyle, pParent );
}

SalFrame* SvpSalInstance::CreateFrame( SalFrame* pParent, ULONG nStyle )
{
    return new SvpSalFrame( this, pParent, nStyle );
}

void SvpSalInstance::DestroyFrame( SalFrame* pFrame )
{
    delete pFrame;
}

SalObject* SvpSalInstance::CreateObject( SalFrame*, SystemWindowData*, BOOL )
{
    return new SvpSalObject();
}

void SvpSalInstance::DestroyObject( SalObject* pObject )
{
    delete pObject;
}

SalVirtualDevice* SvpSalInstance::CreateVirtualDevice( SalGraphics*,
                                                       long nDX, long nDY,
                                                       USHORT nBitCount, const SystemGraphicsData* )
{
    SvpSalVirtualDevice* pNew = new SvpSalVirtualDevice( nBitCount );
    pNew->SetSize( nDX, nDY );
    return pNew;
}

void SvpSalInstance::DestroyVirtualDevice( SalVirtualDevice* pDevice )
{
    delete pDevice;
}

SalTimer* SvpSalInstance::CreateSalTimer()
{
    return new SvpSalTimer( this );
}

SalOpenGL* SvpSalInstance::CreateSalOpenGL( SalGraphics* )
{
    return new SvpSalOpenGL();
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
    return new SvpSalBitmap();
}

vos::IMutex* SvpSalInstance::GetYieldMutex()
{
    return &m_aYieldMutex;
}

ULONG SvpSalInstance::ReleaseYieldMutex()
{
    if ( m_aYieldMutex.GetThreadId() ==
         NAMESPACE_VOS(OThread)::getCurrentIdentifier() )
    {
        ULONG nCount = m_aYieldMutex.GetAcquireCount();
        ULONG n = nCount;
        while ( n )
        {
            m_aYieldMutex.release();
            n--;
        }

        return nCount;
    }
    else
        return 0;
}

void SvpSalInstance::AcquireYieldMutex( ULONG nCount )
{
    while ( nCount )
    {
        m_aYieldMutex.acquire();
        nCount--;
    }
}

void SvpSalInstance::Yield( bool bWait, bool bHandleAllCurrentEvents )
{
    // first, check for already queued events.

    // release yield mutex
    std::list< SalUserEvent > aEvents;
    ULONG nAcquireCount = ReleaseYieldMutex();
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
            it->m_pFrame->CallCallback( it->m_nEvent, it->m_pData );
            if( it->m_nEvent == SALEVENT_RESIZE )
            {
                // this would be a good time to post a paint
                const SvpSalFrame* pSvpFrame = static_cast<const SvpSalFrame*>(it->m_pFrame);
                pSvpFrame->PostPaint();
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
            gettimeofday (&Timeout, 0);
            nTimeoutMS = m_aTimeout.tv_sec*1000 + m_aTimeout.tv_usec/1000
                         - Timeout.tv_sec*1000 - Timeout.tv_usec/1000;
            if( nTimeoutMS < 0 )
                nTimeoutMS = 0;
        }
        else
            nTimeoutMS = -1; // wait until something happens

        // release yield mutex
        nAcquireCount = ReleaseYieldMutex();
        // poll
        struct pollfd aPoll;
        aPoll.fd = m_pTimeoutFDS[0];
        aPoll.events = POLLIN;
        aPoll.revents = 0;
        poll( &aPoll, 1, nTimeoutMS );

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
}

bool SvpSalInstance::AnyInput( USHORT nType )
{
    if( (nType & INPUT_TIMER) != 0 )
        return CheckTimeout( false );
    return false;
}

SalMenu* SvpSalInstance::CreateMenu( BOOL )
{
    return NULL;
}

void SvpSalInstance::DestroyMenu( SalMenu* )
{
}

SalMenuItem* SvpSalInstance::CreateMenuItem( const SalItemParams* )
{
    return NULL;
}

void SvpSalInstance::DestroyMenuItem( SalMenuItem* )
{
}

SalSession* SvpSalInstance::CreateSalSession()
{
    return NULL;
}

void* SvpSalInstance::GetConnectionIdentifier( ConnectionIdentifierType& rReturnedType, int& rReturnedBytes )
{
    rReturnedBytes  = 1;
    rReturnedType   = AsciiCString;
    return const_cast<char*>("");
}

// -------------------------------------------------------------------------
//
// SalYieldMutex
//
// -------------------------------------------------------------------------

SvpSalYieldMutex::SvpSalYieldMutex()
{
    mnCount     = 0;
    mnThreadId  = 0;
}

void SvpSalYieldMutex::acquire()
{
    OMutex::acquire();
    mnThreadId = NAMESPACE_VOS(OThread)::getCurrentIdentifier();
    mnCount++;
}

void SvpSalYieldMutex::release()
{
    if ( mnThreadId == NAMESPACE_VOS(OThread)::getCurrentIdentifier() )
    {
        if ( mnCount == 1 )
            mnThreadId = 0;
        mnCount--;
    }
    OMutex::release();
}

sal_Bool SvpSalYieldMutex::tryToAcquire()
{
    if ( OMutex::tryToAcquire() )
    {
        mnThreadId = NAMESPACE_VOS(OThread)::getCurrentIdentifier();
        mnCount++;
        return sal_True;
    }
    else
        return sal_False;
}

// ---------------
// - SalTimer -
// ---------------

void SvpSalInstance::StopTimer()
{
    m_aTimeout.tv_sec   = 0;
    m_aTimeout.tv_usec  = 0;
    m_nTimeoutMS        = 0;
}

void SvpSalInstance::StartTimer( ULONG nMS )
{
    timeval Timeout (m_aTimeout); // previous timeout.
    gettimeofday (&m_aTimeout, 0);

    m_nTimeoutMS  = nMS;
    m_aTimeout    += m_nTimeoutMS;

    if ((Timeout > m_aTimeout) || (Timeout.tv_sec == 0))
    {
        // Wakeup from previous timeout (or stopped timer).
        Wakeup();
    }
}

SvpSalTimer::~SvpSalTimer()
{
}

void SvpSalTimer::Stop()
{
    m_pInstance->StopTimer();
}

void SvpSalTimer::Start( ULONG nMS )
{
    m_pInstance->StartTimer( nMS );
}

