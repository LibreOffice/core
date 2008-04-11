/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: mediator.cxx,v $
 * $Revision: 1.11 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"
#include <errno.h>
#include <unistd.h>

#include <plugin/unx/mediator.hxx>
#include <vcl/svapp.hxx>

#define MEDIATOR_MAGIC 0xf7a8d2f4

Mediator::Mediator( int nSocket ) :
        m_nSocket( nSocket ),
        m_pListener( NULL ),
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
            ::vos::OGuard aGuard( m_pListener->m_aMutex );
            m_pListener->m_pMediator = NULL;
        }
        m_pListener = NULL;
        if( m_bValid )
        {
            ULONG aHeader[3];
            aHeader[0] = 0;
            aHeader[1] = 0;
            aHeader[2] = MEDIATOR_MAGIC;
            write( m_nSocket, aHeader, sizeof( aHeader ) );
        }
        // kick the thread out of its run method; it deletes itself
        close( m_nSocket );
    }
    else
        close( m_nSocket );
    while( m_aMessageQueue.Count() )
        delete m_aMessageQueue.Remove( (ULONG)0 );
}


ULONG Mediator::SendMessage( ULONG nBytes, const char* pBytes, ULONG nMessageID )
{
    if( ! m_pListener )
        return 0;

    NAMESPACE_VOS(OGuard) aGuard( m_aSendMutex );
    if( ! nMessageID )
        nMessageID = m_nCurrentID;

    m_nCurrentID++;
    if( m_nCurrentID >= 1 << 24 ) // protection against overflow
        m_nCurrentID = 1;

    if( ! m_bValid )
        return nMessageID;

    ULONG* pBuffer = new ULONG[ (nBytes/sizeof(ULONG)) + 4 ];
    pBuffer[ 0 ] = nMessageID;
    pBuffer[ 1 ] = nBytes;
    pBuffer[ 2 ] = MEDIATOR_MAGIC;
    memcpy( &pBuffer[3], pBytes, (size_t)nBytes );
    write( m_nSocket, pBuffer, nBytes + 3*sizeof( ULONG ) );
    delete [] pBuffer;

    return nMessageID;
}

BOOL Mediator::WaitForMessage( ULONG nTimeOut )
{
    if( ! m_pListener )
        return FALSE;

    ULONG nItems = m_aMessageQueue.Count();

    if( ! nTimeOut && nItems > 0 )
        return TRUE;

    TimeValue aValue;
    aValue.Seconds = nTimeOut/1000;
    aValue.Nanosec = ( nTimeOut % 1000 ) * 1000;

    while( m_aMessageQueue.Count() == nItems )
    {
        m_aNewMessageCdtn.wait( & aValue );
        m_aNewMessageCdtn.reset();
        if( nTimeOut && m_aMessageQueue.Count() == nItems )
            return FALSE;
    }
    return TRUE;
}

MediatorMessage* Mediator::WaitForAnswer( ULONG nMessageID )
{
    nMessageID &= 0x00ffffff;
    while( m_pListener )
    {
        {
            NAMESPACE_VOS(OGuard) aGuard( m_aQueueMutex );
            for( ULONG i = 0; i < m_aMessageQueue.Count(); i++ )
            {
                ULONG nID = m_aMessageQueue.GetObject( i )->m_nID;
                if(  ( nID & 0xff000000 ) &&
                     ( ( nID & 0x00ffffff ) == nMessageID ) )
                    return m_aMessageQueue.Remove( i );
            }
        }
        WaitForMessage( 10 );
    }
    return NULL;
}

MediatorMessage* Mediator::GetNextMessage( BOOL bWait )
{
    while( m_pListener )
    {
        {
            // guard must be after WaitForMessage, else the listener
            // cannot insert a new one -> deadlock
            NAMESPACE_VOS(OGuard) aGuard( m_aQueueMutex );
            for( ULONG i = 0; i < m_aMessageQueue.Count(); i++ )
                if( ! ( m_aMessageQueue.GetObject( i )->m_nID & 0xff000000 ) )
                    return m_aMessageQueue.Remove( i );
            if( ! bWait )
                return NULL;
        }
        WaitForMessage();
    }
    return NULL;
}

MediatorMessage* Mediator::TransactMessage( ULONG nBytes, char* pBytes )
{
    ULONG nID = SendMessage( nBytes, pBytes );
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
    bool bRun = true;
    while( schedule() && m_pMediator && bRun )
    {
        ULONG nHeader[ 3 ];
        int nBytes;

        if( m_pMediator && ( nBytes = read( m_pMediator->m_nSocket, nHeader, sizeof( nHeader ) ) ) == sizeof( nHeader ) && nHeader[2] == MEDIATOR_MAGIC)
        {
            if( nHeader[ 0 ] == 0 && nHeader[ 1 ] == 0 )
                return;
            char* pBuffer = new char[ nHeader[ 1 ] ];
            if( m_pMediator && (ULONG)read( m_pMediator->m_nSocket, pBuffer, nHeader[ 1 ] ) == nHeader[ 1 ] )
            {
                ::vos::OGuard aMyGuard( m_aMutex );
                {
                    NAMESPACE_VOS(OGuard)
                        aGuard( m_pMediator->m_aQueueMutex );
                    MediatorMessage* pMessage =
                        new MediatorMessage( nHeader[ 0 ], nHeader[ 1 ], pBuffer );
                    m_pMediator->m_aMessageQueue.Insert( pMessage, LIST_APPEND );
                }
                m_pMediator->m_aNewMessageCdtn.set();
                m_pMediator->m_aNewMessageHdl.Call( m_pMediator );
            }
            else
            {
                medDebug( 1, "got incomplete MediatorMessage: { %d, %d, %*s }\n",
                          nHeader[0], nHeader[1], nHeader[1], pBuffer );
                bRun = false;
            }
            delete [] pBuffer;
        }
        else
        {
            medDebug( 1, "got incomplete message header of %d bytes ( nHeader = [ %u, %u ] ), errno is %d\n",
                      nBytes, nHeader[ 0 ], nHeader[ 1 ], (int)errno );
            bRun = false;
        }
    }
}

void MediatorListener::onTerminated()
{
    if( m_pMediator )
    {
        m_pMediator->m_aConnectionLostHdl.Call( m_pMediator );
        m_pMediator->m_pListener = NULL;
    }
    delete this;
}

ULONG MediatorMessage::ExtractULONG()
{
    if( ! m_pRun )
        m_pRun = m_pBytes;

    medDebug( (ULONG)(m_pRun - m_pBytes) >= m_nBytes, "Overflow in MediatorMessage::ExtractULONG\n" );
    ULONG nCount;
    memcpy( &nCount, m_pRun, sizeof( ULONG ) );
    m_pRun += sizeof( ULONG );
    return nCount;
}

void* MediatorMessage::GetBytes( ULONG& rBytes )
{
    if( ! m_pRun )
        m_pRun = m_pBytes;

    medDebug( (ULONG)(m_pRun - m_pBytes) >= m_nBytes, "Overflow in MediatorMessage::GetBytes\n" );
    ULONG nBytes = ExtractULONG();

    if( nBytes == 0 )
        return NULL;

    medDebug( (ULONG)(m_pRun - m_pBytes) >= m_nBytes, "Overflow in MediatorMessage::GetBytes\n" );
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

    medDebug( (ULONG)(m_pRun - m_pBytes) >= m_nBytes, "Overflow in MediatorMessage::GetString\n" );
    ULONG nBytes = ExtractULONG();

    if( nBytes == 0 )
        return NULL;

    medDebug( (ULONG)(m_pRun - m_pBytes) >= m_nBytes, "Overflow in MediatorMessage::GetString\n" );
    char* pBuffer = new char[ nBytes+1 ];
    memcpy( pBuffer, m_pRun, nBytes );
    pBuffer[ nBytes ] = 0;
    m_pRun += nBytes;
    return pBuffer;
}

UINT32 MediatorMessage::GetUINT32()
{
    if( ! m_pRun )
        m_pRun = m_pBytes;

    medDebug( (ULONG)(m_pRun - m_pBytes) >= m_nBytes, "Overflow in MediatorMessage::GetUINT32\n" );
    ULONG nBytes = ExtractULONG();
    medDebug( nBytes != sizeof( UINT32 ), "No UINT32 in MediatorMessage::GetUINT32\n" );
    medDebug( (ULONG)(m_pRun - m_pBytes) >= m_nBytes, "Overflow in MediatorMessage::GetUINT32\n" );
    UINT32 nRet;
    memcpy( &nRet, m_pRun, sizeof( nRet ) );
    m_pRun += sizeof( UINT32 );
    return nRet;
}
