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
#ifndef INCLUDED_EXTENSIONS_SOURCE_PLUGIN_INC_PLUGIN_UNX_MEDIATOR_HXX
#define INCLUDED_EXTENSIONS_SOURCE_PLUGIN_INC_PLUGIN_UNX_MEDIATOR_HXX

#include <string.h>
#include <tools/link.hxx>
#include <tools/solar.h>
#include <osl/pipe.hxx>
#include <osl/mutex.hxx>
#include <osl/conditn.hxx>
#include <osl/thread.hxx>

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

    sal_uLong   ExtractULONG();
    char*       GetString();
    sal_uInt32  GetUINT32();
    void*       GetBytes( sal_uLong& );
    void*       GetBytes() { sal_uLong nBytes; return GetBytes( nBytes ); }
};

class MediatorListener;

class Mediator
{
    friend class MediatorListener;
protected:
    int                                 m_nSocket;

    std::vector<MediatorMessage*>       m_aMessageQueue;
    osl::Mutex m_aQueueMutex;
    osl::Mutex m_aSendMutex;
    // only one thread can send a message at any given time
    osl::Condition                      m_aNewMessageCdtn;
    MediatorListener*                   m_pListener;
    // thread to fill the queue

    sal_uLong                           m_nCurrentID;
    // will be constantly increased with each message sent
    bool                                m_bValid;

    Link<Mediator*,void>                m_aConnectionLostHdl;
    Link<Mediator*,void>                m_aNewMessageHdl;
public:
    Mediator( int nSocket );
    virtual ~Mediator();

    // mark mediator as invalid. No more messages will be processed,
    // SendMessage, WaitForMessage, TransactMessage will return immediately
    // with error
    void invalidate() { m_bValid = false; }

    sal_uLong SendMessage( sal_uLong nBytes, const char* pBytes, sal_uLong nMessageID = 0 );

    bool WaitForMessage( sal_uLong nTimeOut = 5000 );
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

    MediatorMessage* GetNextMessage( bool bWait = false );


    void SetConnectionLostHdl( const Link<Mediator*,void>& rLink )
        {
            m_aConnectionLostHdl = rLink;
        }

    void SetNewMessageHdl( const Link<Mediator*,void>& rLink )
        {
            m_aNewMessageHdl = rLink;
        }
};

class MediatorListener : public osl::Thread
{
    friend class Mediator;
private:
    Mediator*       m_pMediator;
    ::osl::Mutex    m_aMutex;

    MediatorListener( Mediator* );
    virtual ~MediatorListener();

    virtual void run() SAL_OVERRIDE;
    virtual void onTerminated() SAL_OVERRIDE;
};

#endif // INCLUDED_EXTENSIONS_SOURCE_PLUGIN_INC_PLUGIN_UNX_MEDIATOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
