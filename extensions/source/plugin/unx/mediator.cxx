/*************************************************************************
 *
 *  $RCSfile: mediator.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-23 09:57:36 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
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

    int nItems = m_aMessageQueue.Count();

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
            for( int i = 0; i < m_aMessageQueue.Count(); i++ )
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
            for( int i = 0; i < m_aMessageQueue.Count(); i++ )
                if( ! ( m_aMessageQueue.GetObject( i )->m_nID & 0xff000000 ) )
                    return m_aMessageQueue.Remove( i );
            if( ! bWait )
                return NULL;
        }
        WaitForMessage();
    }
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
            if( m_pMediator && read( m_pMediator->m_nSocket, pBuffer, nHeader[ 1 ] ) == nHeader[ 1 ] )
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
