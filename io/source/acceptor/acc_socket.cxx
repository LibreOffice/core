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

#include "acceptor.hxx"

#include <exception>
#include <unordered_set>

#include <osl/mutex.hxx>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/connection/XConnectionBroadcaster.hpp>
#include <com/sun/star/connection/ConnectionSetupException.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <cppuhelper/implbase.hxx>

using namespace ::osl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::connection;


namespace io_acceptor {
    template<class T>
    struct ReferenceHash
    {
        size_t operator () (const css::uno::Reference<T> & ref) const
        {
            return reinterpret_cast<size_t>(ref.get());
        }
    };

    template<class T>
    struct ReferenceEqual
    {
        bool operator () (const css::uno::Reference<T> & op1,
                          const css::uno::Reference<T> & op2) const
        {
            return op1.get() == op2.get();
        }
    };


    typedef std::unordered_set< css::uno::Reference< css::io::XStreamListener>,
                                ReferenceHash< css::io::XStreamListener>,
                                ReferenceEqual< css::io::XStreamListener> >
            XStreamListener_hash_set;


    class SocketConnection : public ::cppu::WeakImplHelper<
        css::connection::XConnection,
        css::connection::XConnectionBroadcaster>

    {
    public:
        explicit SocketConnection( const OUString & sConnectionDescription );

        virtual sal_Int32 SAL_CALL read( css::uno::Sequence< sal_Int8 >& aReadBytes,
                                         sal_Int32 nBytesToRead ) override;
        virtual void SAL_CALL write( const css::uno::Sequence< sal_Int8 >& aData ) override;
        virtual void SAL_CALL flush(  ) override;
        virtual void SAL_CALL close(  ) override;
        virtual OUString SAL_CALL getDescription(  ) override;

        // XConnectionBroadcaster
        virtual void SAL_CALL addStreamListener(const css::uno::Reference< css::io::XStreamListener>& aListener) override;
        virtual void SAL_CALL removeStreamListener(const css::uno::Reference< css::io::XStreamListener>& aListener) override;

    public:
        void completeConnectionString();

        ::osl::StreamSocket m_socket;
        ::osl::SocketAddr m_addr;
        oslInterlockedCount m_nStatus;
        OUString m_sDescription;

        ::osl::Mutex _mutex;
        bool     _started;
        bool     _closed;
        bool     _error;
        XStreamListener_hash_set _listeners;
    };

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


    SocketConnection::SocketConnection( const OUString &sConnectionDescription) :
        m_nStatus( 0 ),
        m_sDescription( sConnectionDescription ),
        _started(false),
        _closed(false),
        _error(false)
    {
        // make it unique
        m_sDescription += ",uniqueValue=" ;
        m_sDescription += OUString::number(
            sal::static_int_cast< sal_Int64 >(
                reinterpret_cast< sal_IntPtr >(&m_socket)) );
    }

    void SocketConnection::completeConnectionString()
    {
        OUStringBuffer buf( 256 );
        buf.append( ",peerPort=" );
        buf.append( (sal_Int32) m_socket.getPeerPort() );
        buf.append( ",peerHost=" );
        buf.append( m_socket.getPeerHost( ) );

        buf.append( ",localPort=" );
        buf.append( (sal_Int32) m_socket.getLocalPort() );
        buf.append( ",localHost=" );
        buf.append( m_socket.getLocalHost() );

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

            sal_Int32 i = m_socket.read(
                aReadBytes.getArray(), aReadBytes.getLength());

            if(i != nBytesToRead)
            {
                OUString message("acc_socket.cxx:SocketConnection::read: error - ");
                message +=  m_socket.getErrorAsString();

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
            OUString message("acc_socket.cxx:SocketConnection::read: error - connection already closed");

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
                OUString message("acc_socket.cxx:SocketConnection::write: error - ");
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
            OUString message("acc_socket.cxx:SocketConnection::write: error - connection already closed");

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
        // ensure close is called only once
        if(  1 == osl_atomic_increment( (&m_nStatus) ) )
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

    SocketAcceptor::SocketAcceptor( const OUString &sSocketName,
                                    sal_uInt16 nPort,
                                    bool bTcpNoDelay,
                                    const OUString &sConnectionDescription) :
        m_sSocketName( sSocketName ),
        m_sConnectionDescription( sConnectionDescription ),
        m_nPort( nPort ),
        m_bTcpNoDelay( bTcpNoDelay ),
        m_bClosed( false )
    {
    }


    void SocketAcceptor::init()
    {
        if( ! m_addr.setPort( m_nPort ) )
        {
            throw ConnectionSetupException(
                "acc_socket.cxx:SocketAcceptor::init - error - invalid tcp/ip port " +
                OUString::number( m_nPort ));
        }
        if( ! m_addr.setHostname( m_sSocketName.pData ) )
        {
            throw ConnectionSetupException(
                "acc_socket.cxx:SocketAcceptor::init - error - invalid host " + m_sSocketName );
        }
        m_socket.setOption( osl_Socket_OptionReuseAddr, 1);

        if(! m_socket.bind(m_addr) )
        {
            throw ConnectionSetupException(
                "acc_socket.cxx:SocketAcceptor::init - error - couldn't bind on " +
                m_sSocketName + ":" + OUString::number(m_nPort));
        }

        if(! m_socket.listen() )
        {
            throw ConnectionSetupException(
                "acc_socket.cxx:SocketAcceptor::init - error - can't listen on " +
                m_sSocketName  + ":" + OUString::number(m_nPort) );
        }
    }

    Reference< XConnection > SocketAcceptor::accept( )
    {
        SocketConnection *pConn = new SocketConnection( m_sConnectionDescription );

        if( m_socket.acceptConnection( pConn->m_socket )!= osl_Socket_Ok )
        {
            // stopAccepting was called
            delete pConn;
            return Reference < XConnection > ();
        }
        if( m_bClosed )
        {
            delete pConn;
            return Reference < XConnection > ();
        }

        pConn->completeConnectionString();
        OUString remoteHostname = pConn->m_addr.getHostname();
        // we enable tcpNoDelay for loopback connections because
        // it can make a significant speed difference on linux boxes.
        if( m_bTcpNoDelay || remoteHostname == "localhost" ||
            remoteHostname.startsWith("127.0.0.") )
        {
            sal_Int32 nTcpNoDelay = sal_Int32(true);
            pConn->m_socket.setOption( osl_Socket_OptionTcpNoDelay , &nTcpNoDelay,
                                       sizeof( nTcpNoDelay ) , osl_Socket_LevelTcp );
        }

        return Reference < XConnection > ( static_cast<XConnection *>(pConn) );
    }

    void SocketAcceptor::stopAccepting()
    {
        m_bClosed = true;
        m_socket.close();
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
