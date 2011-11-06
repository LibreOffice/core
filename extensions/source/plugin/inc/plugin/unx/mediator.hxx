/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _MEDIATOR_HXX
#define _MEDIATOR_HXX

#include <string.h>
#include <stdarg.h>
#include <tools/string.hxx>
#include <tools/link.hxx>
#include <vos/pipe.hxx>
#include <vos/mutex.hxx>
#include <vos/conditn.hxx>
#include <vos/thread.hxx>
#if OSL_DEBUG_LEVEL > 1
#include <stdio.h>
#endif

#include <vector>

struct MediatorMessage
{
    sal_uLong   m_nID;
    sal_uLong   m_nBytes;
    char*   m_pBytes;
    char*   m_pRun;

    MediatorMessage() : m_nID( 0 ), m_nBytes( 0 ),
        m_pBytes( NULL ), m_pRun( NULL ) {}
    MediatorMessage( sal_uLong nID, sal_uLong nBytes, char* pBytes ) :
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

    void Set( sal_uLong nBytes, char* pBytes )
        {
            if( m_pBytes )
                delete [] m_pBytes;
            m_nBytes = nBytes;
            m_pBytes = new char[ m_nBytes ];
            memcpy( m_pBytes, pBytes, (size_t)m_nBytes );
        }

    sal_uLong   ExtractULONG();
    char*   GetString();
    sal_uInt32  GetUINT32();
    void*   GetBytes( sal_uLong& );
    void*   GetBytes() { sal_uLong nBytes; return GetBytes( nBytes ); }

    void    Rewind() { m_pRun = NULL; }
};

class MediatorListener;

class Mediator
{
    friend class MediatorListener;
protected:
    int                                 m_nSocket;

    std::vector<MediatorMessage*>       m_aMessageQueue;
    vos::OMutex             m_aQueueMutex;
    vos::OMutex             m_aSendMutex;
    // only one thread can send a message at any given time
    vos::OCondition         m_aNewMessageCdtn;
    MediatorListener*                   m_pListener;
    // thread to fill the queue

    sal_uLong                               m_nCurrentID;
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

    sal_uLong SendMessage( sal_uLong nBytes, const char* pBytes, sal_uLong nMessageID = 0 );
    sal_uLong SendMessage( const ByteString& rMessage, sal_uLong nMessageID = 0 )
        {
            return SendMessage( rMessage.Len(), rMessage.GetBuffer(), nMessageID );
        }

    sal_Bool WaitForMessage( sal_uLong nTimeOut = 5000 );
    // timeout in ms
    // TRUE:  Message came in
    // FALSE: timed out
    // if timeout is set, WaitForMessage will wait even if there are messages
    // in the queue

    virtual MediatorMessage* WaitForAnswer( sal_uLong nMessageID );
    // wait for an answer message ( ID >= 1 << 24 )
    // the message will be removed from the queue and returned

    MediatorMessage* TransactMessage( sal_uLong nBytes, char* pBytes );
    // sends a message and waits for an answer

    MediatorMessage* GetNextMessage( sal_Bool bWait = sal_False );


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

class MediatorListener : public vos:: OThread
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
