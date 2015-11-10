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

#include <errno.h>
#include <unistd.h>

#include <plugin/unx/mediator.hxx>
#include <sal/log.hxx>
#include <vcl/svapp.hxx>
#include <memory>

#define MEDIATOR_MAGIC 0xf7a8d2f4

Mediator::Mediator( int nSocket ) :
        m_nSocket( nSocket ),
        m_pListener( nullptr ),
        m_nCurrentID( 1 ),
        m_bValid( true )
{
    m_pListener = new MediatorListener( this );
    m_pListener->create();
}

Mediator::~Mediator()
{
    if( m_pListener )
    {
        {
            ::osl::MutexGuard aGuard( m_pListener->m_aMutex );
            m_pListener->m_pMediator = nullptr;
        }
        m_pListener = nullptr;
        if( m_bValid )
        {
            sal_uLong aHeader[3];
            aHeader[0] = 0;
            aHeader[1] = 0;
            aHeader[2] = MEDIATOR_MAGIC;
            ssize_t nToWrite = sizeof(aHeader);
            bool bSuccess = (nToWrite == write(m_nSocket, aHeader, nToWrite));
            SAL_WARN_IF(!bSuccess, "extensions.plugin", "short write");
        }
        // kick the thread out of its run method; it deletes itself
        close( m_nSocket );
    }
    else
        close( m_nSocket );
    for( std::vector< MediatorMessage* >::iterator it = m_aMessageQueue.begin();
         it != m_aMessageQueue.end(); ++it )
    {
        delete *it;
    }
}


sal_uLong Mediator::SendMessage( sal_uLong nBytes, const char* pBytes, sal_uLong nMessageID )
{
    if( ! m_pListener )
        return 0;

    osl::MutexGuard aGuard( m_aSendMutex );
    if( ! nMessageID )
        nMessageID = m_nCurrentID;

    m_nCurrentID++;
    if( m_nCurrentID >= 1 << 24 ) // protection against overflow
        m_nCurrentID = 1;

    if( ! m_bValid )
        return nMessageID;

    std::unique_ptr<sal_uLong[]> pBuffer(new sal_uLong[ (nBytes/sizeof(sal_uLong)) + 4 ]);
    pBuffer[ 0 ] = nMessageID;
    pBuffer[ 1 ] = nBytes;
    pBuffer[ 2 ] = MEDIATOR_MAGIC;
    memcpy( &pBuffer[3], pBytes, (size_t)nBytes );
    ssize_t nToWrite = nBytes + 3*sizeof( sal_uLong );
    bool bSuccess = (nToWrite == write( m_nSocket, pBuffer.get(), nToWrite ));
    SAL_WARN_IF(!bSuccess, "extensions.plugin", "short write");

    return nMessageID;
}

bool Mediator::WaitForMessage( sal_uLong nTimeOut )
{
    if( ! m_pListener )
        return false;

    size_t nItems = m_aMessageQueue.size();

    if( ! nTimeOut && nItems > 0 )
        return true;

    TimeValue aValue;
    aValue.Seconds = nTimeOut/1000;
    aValue.Nanosec = ( nTimeOut % 1000 ) * 1000;

    while( m_aMessageQueue.size() == nItems )
    {
        m_aNewMessageCdtn.wait( & aValue );
        m_aNewMessageCdtn.reset();
        if( nTimeOut && m_aMessageQueue.size() == nItems )
            return false;
    }
    return true;
}

MediatorMessage* Mediator::WaitForAnswer( sal_uLong nMessageID )
{
    nMessageID &= 0x00ffffff;
    while( m_pListener )
    {
        {
            osl::MutexGuard aGuard( m_aQueueMutex );
            for( size_t i = 0; i < m_aMessageQueue.size(); i++ )
            {
                MediatorMessage* pMessage = m_aMessageQueue[ i ];
                sal_uLong nID = pMessage->m_nID;
                if(  ( nID & 0xff000000 ) &&
                     ( ( nID & 0x00ffffff ) == nMessageID ) )
                {
                    m_aMessageQueue.erase( m_aMessageQueue.begin() + i );
                    return pMessage;
                }
            }
        }
        WaitForMessage( 10 );
    }
    return nullptr;
}

MediatorMessage* Mediator::GetNextMessage( bool bWait )
{
    while( m_pListener )
    {
        {
            // guard must be after WaitForMessage, else the listener
            // cannot insert a new one -> deadlock
            osl::MutexGuard aGuard( m_aQueueMutex );
            for( size_t i = 0; i < m_aMessageQueue.size(); i++ )
            {
                MediatorMessage* pMessage = m_aMessageQueue[ i ];
                if( ! ( pMessage->m_nID & 0xff000000 ) )
                {
                    m_aMessageQueue.erase( m_aMessageQueue.begin() + i );
                    return pMessage;
                }
            }
            if( ! bWait )
                return nullptr;
        }
        WaitForMessage();
    }
    return nullptr;
}

MediatorMessage* Mediator::TransactMessage( sal_uLong nBytes, char* pBytes )
{
    sal_uLong nID = SendMessage( nBytes, pBytes );
    return WaitForAnswer( nID );
}

MediatorListener::MediatorListener( Mediator* pMediator ) :
        m_pMediator( pMediator )
{
}

MediatorListener::~MediatorListener()
{
}

void MediatorListener::run()
{
    osl_setThreadName("MediatorListener");

    bool bRun = true;
    while( schedule() && m_pMediator && bRun )
    {
        sal_uLong nHeader[ 3 ];
        int nBytes;

        if( ( nBytes = read( m_pMediator->m_nSocket, nHeader, sizeof( nHeader ) ) ) == sizeof( nHeader ) && nHeader[2] == MEDIATOR_MAGIC)
        {
            if( nHeader[ 0 ] == 0 && nHeader[ 1 ] == 0 )
                return;
            std::unique_ptr<char[]> pBuffer(new char[ nHeader[ 1 ] ]);
            if( m_pMediator && (sal_uLong)read( m_pMediator->m_nSocket, pBuffer.get(), nHeader[ 1 ] ) == nHeader[ 1 ] )
            {
                ::osl::MutexGuard aMyGuard( m_aMutex );
                {
                    osl::MutexGuard
                        aGuard( m_pMediator->m_aQueueMutex );
                    MediatorMessage* pMessage =
                        new MediatorMessage( nHeader[ 0 ], nHeader[ 1 ], pBuffer.get() );
                    m_pMediator->m_aMessageQueue.push_back( pMessage );
                }
                m_pMediator->m_aNewMessageCdtn.set();
                m_pMediator->m_aNewMessageHdl.Call( m_pMediator );
            }
            else
            {
                SAL_WARN(
                    "extensions.plugin",
                    "got incomplete MediatorMessage: { " << nHeader[0] << ", "
                        << nHeader[1] << ", ... }");
                bRun = false;
            }
        }
        else
        {
            SAL_WARN(
                "extensions.plugin",
                "got incomplete message header of " << nBytes
                    << " bytes (nHeader = [" << nHeader[0] << ", " << nHeader[1]
                    << "]), errno is " << errno);
            bRun = false;
        }
    }
}

void MediatorListener::onTerminated()
{
    if( m_pMediator )
    {
        m_pMediator->m_aConnectionLostHdl.Call( m_pMediator );
        m_pMediator->m_pListener = nullptr;
    }
    delete this;
}

sal_uLong MediatorMessage::ExtractULONG()
{
    if( ! m_pRun )
        m_pRun = m_pBytes;

    SAL_WARN_IF(
        (sal_uLong)(m_pRun - m_pBytes) >= m_nBytes, "extensions.plugin",
        "overflow in MediatorMessage::ExtractULONG");
    sal_uLong nCount;
    memcpy( &nCount, m_pRun, sizeof( sal_uLong ) );
    m_pRun += sizeof( sal_uLong );
    return nCount;
}

void* MediatorMessage::GetBytes( sal_uLong& rBytes )
{
    if( ! m_pRun )
        m_pRun = m_pBytes;

    SAL_WARN_IF(
        (sal_uLong)(m_pRun - m_pBytes) >= m_nBytes, "extensions.plugin",
        "overflow in MediatorMessage::GetBytes");
    sal_uLong nBytes = ExtractULONG();

    if( nBytes == 0 )
        return nullptr;

    SAL_WARN_IF(
        (sal_uLong)(m_pRun - m_pBytes) >= m_nBytes, "extensions.plugin",
        "overflow in MediatorMessage::GetBytes");
    char* pBuffer = new char[ nBytes ];
    memcpy( pBuffer, m_pRun, nBytes );
    m_pRun += nBytes;
    rBytes = nBytes;
    return pBuffer;
}

char* MediatorMessage::GetString()
{
    if( ! m_pRun )
        m_pRun = m_pBytes;

    SAL_WARN_IF(
        (sal_uLong)(m_pRun - m_pBytes) >= m_nBytes, "extensions.plugin",
        "overflow in MediatorMessage::GetString");
    sal_uLong nBytes = ExtractULONG();

    if( nBytes == 0 )
        return nullptr;

    SAL_WARN_IF(
        (sal_uLong)(m_pRun - m_pBytes) >= m_nBytes, "extensions.plugin",
        "overflow in MediatorMessage::GetString");
    char* pBuffer = new char[ nBytes+1 ];
    memcpy( pBuffer, m_pRun, nBytes );
    pBuffer[ nBytes ] = 0;
    m_pRun += nBytes;
    return pBuffer;
}

sal_uInt32 MediatorMessage::GetUINT32()
{
    if( ! m_pRun )
        m_pRun = m_pBytes;

    SAL_WARN_IF(
        (sal_uLong)(m_pRun - m_pBytes) >= m_nBytes, "extensions.plugin",
        "overflow in MediatorMessage::GetUINT32");
    sal_uLong nBytes = ExtractULONG();
    SAL_WARN_IF(
        nBytes != sizeof( sal_uInt32 ), "extensions.plugin",
        "no sal_uInt32 in MediatorMessage::GetUINT32");
    SAL_WARN_IF(
        (sal_uLong)(m_pRun - m_pBytes) >= m_nBytes, "extensions.plugin",
        "overflow in MediatorMessage::GetUINT32");
    sal_uInt32 nRet;
    memcpy( &nRet, m_pRun, sizeof( nRet ) );
    m_pRun += sizeof( sal_uInt32 );
    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
