/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mediator.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 14:52:33 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _MEDIATOR_HXX
#define _MEDIATOR_HXX

#include <string.h>
#include <stdarg.h>

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _LIST_HXX
#include <tools/list.hxx>
#endif
#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif
#ifndef _VOS_PIPE_HXX_
#include <vos/pipe.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _VOS_CONDITN_HXX_
#include <vos/conditn.hxx>
#endif
#ifndef _VOS_THREAD_HXX_
#include <vos/thread.hxx>
#endif
#if OSL_DEBUG_LEVEL > 1
#include <stdio.h>
#endif

struct MediatorMessage
{
    ULONG   m_nID;
    ULONG   m_nBytes;
    char*   m_pBytes;
    char*   m_pRun;

    MediatorMessage() : m_nID( 0 ), m_nBytes( 0 ),
        m_pBytes( NULL ), m_pRun( NULL ) {}
    MediatorMessage( ULONG nID, ULONG nBytes, char* pBytes ) :
            m_nID( nID ),m_nBytes( nBytes ), m_pRun( NULL )
        {
            m_pBytes = new char[ m_nBytes ];
            memcpy( m_pBytes, pBytes, (size_t)m_nBytes );
        }

    ~MediatorMessage()
        {
            if( m_pBytes )
                delete [] m_pBytes;
        }

    void Set( ULONG nBytes, char* pBytes )
        {
            if( m_pBytes )
                delete [] m_pBytes;
            m_nBytes = nBytes;
            m_pBytes = new char[ m_nBytes ];
            memcpy( m_pBytes, pBytes, (size_t)m_nBytes );
        }

    ULONG   ExtractULONG();
    char*   GetString();
    UINT32  GetUINT32();
    void*   GetBytes( ULONG& );
    void*   GetBytes() { ULONG nBytes; return GetBytes( nBytes ); }

    void    Rewind() { m_pRun = NULL; }
};

DECLARE_LIST( MediatorMessageList, MediatorMessage* )

class MediatorListener;

class Mediator
{
    friend class MediatorListener;
protected:
    int                                 m_nSocket;

    MediatorMessageList                 m_aMessageQueue;
    NAMESPACE_VOS(OMutex)               m_aQueueMutex;
    NAMESPACE_VOS(OMutex)               m_aSendMutex;
    // only one thread can send a message at any given time
    NAMESPACE_VOS(OCondition)           m_aNewMessageCdtn;
    MediatorListener*                   m_pListener;
    // thread to fill the queue

    ULONG                               m_nCurrentID;
    // will be constantly increased with each message sent
    bool                                m_bValid;

    Link                                m_aConnectionLostHdl;
    Link                                m_aNewMessageHdl;
public:
    Mediator( int nSocket );
    ~Mediator();

    // mark mediator as invalid. No more messages will be processed,
    // SendMessage, WaitForMessage, TransactMessage will return immediatly
    // with error
    void invalidate() { m_bValid = false; }

    ULONG SendMessage( ULONG nBytes, const char* pBytes, ULONG nMessageID = 0 );
    ULONG SendMessage( const ByteString& rMessage, ULONG nMessageID = 0 )
        {
            return SendMessage( rMessage.Len(), rMessage.GetBuffer(), nMessageID );
        }

    BOOL WaitForMessage( ULONG nTimeOut = 5000 );
    // timeout in ms
    // TRUE:  Message came in
    // FALSE: timed out
    // if timeout is set, WaitForMessage will wait even if there are messages
    // in the queue

    virtual MediatorMessage* WaitForAnswer( ULONG nMessageID );
    // wait for an answer message ( ID >= 1 << 24 )
    // the message will be removed from the queue and returned

    MediatorMessage* TransactMessage( ULONG nBytes, char* pBytes );
    // sends a message and waits for an answer

    MediatorMessage* GetNextMessage( BOOL bWait = FALSE );


    Link SetConnectionLostHdl( const Link& rLink )
        {
            Link aRet = m_aConnectionLostHdl;
            m_aConnectionLostHdl = rLink;
            return aRet;
        }

    Link SetNewMessageHdl( const Link& rLink )
        {
            Link aRet = m_aNewMessageHdl;
            m_aNewMessageHdl = rLink;
            return aRet;
        }
};

class MediatorListener : public NAMESPACE_VOS( OThread )
{
    friend class Mediator;
  private:
    Mediator*       m_pMediator;
    ::vos::OMutex   m_aMutex;

    MediatorListener( Mediator* );
    ~MediatorListener();

    virtual void run();
    virtual void onTerminated();
};

inline void medDebug( int condition, const char* pFormat, ... )
{
#if OSL_DEBUG_LEVEL > 1
    if( condition )
    {
        va_list ap;
        va_start( ap, pFormat );
        vfprintf( stderr, pFormat, ap );
        va_end( ap );
    }
#else
    (void)condition;
    (void)pFormat;
#endif
}

#endif // _MEDIATOR_HXX
