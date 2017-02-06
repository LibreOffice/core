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


#include "connector.hxx"
#include <com/sun/star/io/IOException.hpp>
#include <rtl/ustrbuf.hxx>
#include <exception>

using namespace ::osl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::connection;


namespace stoc_connector {
    template<class T>
    void notifyListeners(SocketConnection * pCon, bool * notified, T t)
    {
          XStreamListener_hash_set listeners;

        {
            ::osl::MutexGuard guard(pCon->_mutex);
            if(!*notified)
            {
                *notified = true;
                listeners = pCon->_listeners;
            }
        }

        for(auto& listener : listeners)
            t(listener);
    }


    static void callStarted(const Reference<XStreamListener>& xStreamListener)
    {
        xStreamListener->started();
    }

    struct callError {
        const Any & any;

        explicit callError(const Any & any);

        void operator () (const Reference<XStreamListener>& xStreamListener);
    };

    callError::callError(const Any & aAny)
        : any(aAny)
    {
    }

    void callError::operator () (const Reference<XStreamListener>& xStreamListener)
    {
        xStreamListener->error(any);
    }

    static void callClosed(const Reference<XStreamListener>& xStreamListener)
    {
        xStreamListener->closed();
    }


    SocketConnection::SocketConnection( const OUString &sConnectionDescription ) :
        m_nStatus( 0 ),
        m_sDescription( sConnectionDescription ),
        _started(false),
        _closed(false),
        _error(false)
    {
        // make it unique
        m_sDescription += ",uniqueValue=";
        m_sDescription += OUString::number(
            sal::static_int_cast< sal_Int64 >(
                reinterpret_cast< sal_IntPtr >(&m_socket)) );
    }

    SocketConnection::~SocketConnection()
    {
    }

    void SocketConnection::completeConnectionString()
    {
        sal_Int32 nPort;

        nPort = m_socket.getPeerPort();

        OUStringBuffer buf( 256 );
        buf.append( ",peerPort=" );
        buf.append( (sal_Int32) nPort );
        buf.append( ",peerHost=" );
        buf.append( m_socket.getPeerHost() );

        buf.append( ",localPort=" );
        buf.append( (sal_Int32) nPort );
        buf.append( ",localHost=" );
        buf.append( m_socket.getLocalHost( ) );

        m_sDescription += buf.makeStringAndClear();
    }

    sal_Int32 SocketConnection::read( Sequence < sal_Int8 > & aReadBytes , sal_Int32 nBytesToRead )
    {
        if( ! m_nStatus )
        {
            notifyListeners(this, &_started, callStarted);

            if( aReadBytes.getLength() != nBytesToRead )
            {
                aReadBytes.realloc( nBytesToRead );
            }
            sal_Int32 i = m_socket.read( aReadBytes.getArray()  , aReadBytes.getLength() );

            if(i != nBytesToRead && m_socket.getError() != osl_Socket_E_None)
            {
                OUString message("ctr_socket.cxx:SocketConnection::read: error - ");
                message += m_socket.getErrorAsString();

                IOException ioException(message, static_cast<XConnection *>(this));

                Any any;
                any <<= ioException;

                notifyListeners(this, &_error, callError(any));

                throw ioException;
            }

            return i;
        }
        else
        {
            OUString message("ctr_socket.cxx:SocketConnection::read: error - connection already closed");

            IOException ioException(message, static_cast<XConnection *>(this));

            Any any;
            any <<= ioException;

            notifyListeners(this, &_error, callError(any));

            throw ioException;
        }
    }

    void SocketConnection::write( const Sequence < sal_Int8 > &seq )
    {
        if( ! m_nStatus )
        {
            if( m_socket.write( seq.getConstArray() , seq.getLength() ) != seq.getLength() )
            {
                OUString message("ctr_socket.cxx:SocketConnection::write: error - ");
                message += m_socket.getErrorAsString();

                IOException ioException(message, static_cast<XConnection *>(this));

                Any any;
                any <<= ioException;

                notifyListeners(this, &_error, callError(any));

                throw ioException;
            }
        }
        else
        {
            OUString message("ctr_socket.cxx:SocketConnection::write: error - connection already closed");

            IOException ioException(message, static_cast<XConnection *>(this));

            Any any;
            any <<= ioException;

            notifyListeners(this, &_error, callError(any));

            throw ioException;
        }
    }

    void SocketConnection::flush( )
    {

    }

    void SocketConnection::close()
    {
            // ensure that close is called only once
        if( 1 == osl_atomic_increment( (&m_nStatus) ) )
        {
            m_socket.shutdown();
            notifyListeners(this, &_closed, callClosed);
        }
    }

    OUString SocketConnection::getDescription()
    {
        return m_sDescription;
    }


    // XConnectionBroadcaster
    void SAL_CALL SocketConnection::addStreamListener(const Reference<XStreamListener> & aListener)
    {
        MutexGuard guard(_mutex);

        _listeners.insert(aListener);
    }

    void SAL_CALL SocketConnection::removeStreamListener(const Reference<XStreamListener> & aListener)
    {
        MutexGuard guard(_mutex);

        _listeners.erase(aListener);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
