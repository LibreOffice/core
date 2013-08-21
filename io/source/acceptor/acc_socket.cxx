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

#include <boost/unordered_set.hpp>
#include <algorithm>

#include <rtl/ustrbuf.hxx>
#include <com/sun/star/connection/XConnectionBroadcaster.hpp>
#include <com/sun/star/connection/ConnectionSetupException.hpp>

#include <cppuhelper/implbase2.hxx>

using namespace ::osl;
using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::connection;


namespace io_acceptor {
    template<class T>
    struct ReferenceHash
    {
        size_t operator () (const ::com::sun::star::uno::Reference<T> & ref) const
        {
            return (size_t)ref.get();
        }
    };

    template<class T>
    struct ReferenceEqual
    {
        sal_Bool operator () (const ::com::sun::star::uno::Reference<T> & op1,
                              const ::com::sun::star::uno::Reference<T> & op2) const
        {
            return op1.get() == op2.get();
        }
    };


    typedef ::boost::unordered_set< ::com::sun::star::uno::Reference< ::com::sun::star::io::XStreamListener>,
                             ReferenceHash< ::com::sun::star::io::XStreamListener>,
                             ReferenceEqual< ::com::sun::star::io::XStreamListener> >
            XStreamListener_hash_set;


    class SocketConnection : public ::cppu::WeakImplHelper2<
        ::com::sun::star::connection::XConnection,
        ::com::sun::star::connection::XConnectionBroadcaster>

    {
    public:
        SocketConnection( const OUString & sConnectionDescription );
        ~SocketConnection();

        virtual sal_Int32 SAL_CALL read( ::com::sun::star::uno::Sequence< sal_Int8 >& aReadBytes,
                                         sal_Int32 nBytesToRead )
            throw(::com::sun::star::io::IOException,
                  ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL write( const ::com::sun::star::uno::Sequence< sal_Int8 >& aData )
            throw(::com::sun::star::io::IOException,
                  ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL flush(  ) throw(
            ::com::sun::star::io::IOException,
            ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL close(  )
            throw(::com::sun::star::io::IOException,
                  ::com::sun::star::uno::RuntimeException);
        virtual OUString SAL_CALL getDescription(  )
            throw(::com::sun::star::uno::RuntimeException);

        // XConnectionBroadcaster
        virtual void SAL_CALL addStreamListener(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStreamListener>& aListener)
            throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeStreamListener(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStreamListener>& aListener)
            throw(::com::sun::star::uno::RuntimeException);

    public:
        void completeConnectionString();

        ::osl::StreamSocket m_socket;
        ::osl::SocketAddr m_addr;
        oslInterlockedCount m_nStatus;
        OUString m_sDescription;

        ::osl::Mutex _mutex;
        sal_Bool     _started;
        sal_Bool     _closed;
        sal_Bool     _error;
        XStreamListener_hash_set _listeners;
    };

    template<class T>
    void notifyListeners(SocketConnection * pCon, sal_Bool * notified, T t)
    {
          XStreamListener_hash_set listeners;

        {
            ::osl::MutexGuard guard(pCon->_mutex);
            if(!*notified)
            {
                *notified = sal_True;
                listeners = pCon->_listeners;
            }
        }

        ::std::for_each(listeners.begin(), listeners.end(), t);
    }

    static void callStarted(Reference<XStreamListener> xStreamListener)
    {
        xStreamListener->started();
    }

    struct callError {
        const Any & any;

        callError(const Any & any);

        void operator () (Reference<XStreamListener> xStreamListener);
    };

    callError::callError(const Any & aAny)
        : any(aAny)
    {
    }

    void callError::operator () (Reference<XStreamListener> xStreamListener)
    {
        xStreamListener->error(any);
    }

    static void callClosed(Reference<XStreamListener> xStreamListener)
    {
        xStreamListener->closed();
    }


    SocketConnection::SocketConnection( const OUString &sConnectionDescription) :
        m_nStatus( 0 ),
        m_sDescription( sConnectionDescription ),
        _started(sal_False),
        _closed(sal_False),
        _error(sal_False)
    {
        // make it unique
        m_sDescription += ",uniqueValue=" ;
        m_sDescription += OUString::number(
            sal::static_int_cast< sal_Int64 >(
                reinterpret_cast< sal_IntPtr >(&m_socket)),
            10 );
    }

    SocketConnection::~SocketConnection()
    {
    }

    void SocketConnection::completeConnectionString()
    {
        OUStringBuffer buf( 256 );
        buf.appendAscii( ",peerPort=" );
        buf.append( (sal_Int32) m_socket.getPeerPort() );
        buf.appendAscii( ",peerHost=" );
        buf.append( m_socket.getPeerHost( ) );

        buf.appendAscii( ",localPort=" );
        buf.append( (sal_Int32) m_socket.getLocalPort() );
        buf.appendAscii( ",localHost=" );
        buf.append( m_socket.getLocalHost() );

        m_sDescription += buf.makeStringAndClear();
    }

    sal_Int32 SocketConnection::read( Sequence < sal_Int8 > & aReadBytes , sal_Int32 nBytesToRead )
            throw(::com::sun::star::io::IOException,
                  ::com::sun::star::uno::RuntimeException)
    {
        if( ! m_nStatus )
        {
            notifyListeners(this, &_started, callStarted);

            if( aReadBytes.getLength() != nBytesToRead )
            {
                aReadBytes.realloc( nBytesToRead );
            }

            sal_Int32 i = 0;
            i = m_socket.read( aReadBytes.getArray()  , aReadBytes.getLength() );

            if(i != nBytesToRead)
            {
                OUString message("acc_socket.cxx:SocketConnection::read: error - ");
                message +=  m_socket.getErrorAsString();

                IOException ioException(message, Reference<XInterface>(static_cast<XConnection *>(this)));

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

            IOException ioException(message, Reference<XInterface>(static_cast<XConnection *>(this)));

            Any any;
            any <<= ioException;

            notifyListeners(this, &_error, callError(any));

            throw ioException;
        }
    }

    void SocketConnection::write( const Sequence < sal_Int8 > &seq )
            throw(::com::sun::star::io::IOException,
                  ::com::sun::star::uno::RuntimeException)
    {
        if( ! m_nStatus )
        {
            if( m_socket.write( seq.getConstArray() , seq.getLength() ) != seq.getLength() )
            {
                OUString message("acc_socket.cxx:SocketConnection::write: error - ");
                message += m_socket.getErrorAsString();

                IOException ioException(message, Reference<XInterface>(static_cast<XConnection *>(this)));

                Any any;
                any <<= ioException;

                notifyListeners(this, &_error, callError(any));

                throw ioException;
            }
        }
        else
        {
            OUString message("acc_socket.cxx:SocketConnection::write: error - connection already closed");

            IOException ioException(message, Reference<XInterface>(static_cast<XConnection *>(this)));

            Any any;
            any <<= ioException;

            notifyListeners(this, &_error, callError(any));

            throw ioException;
        }
    }

    void SocketConnection::flush( )
            throw(::com::sun::star::io::IOException,
                  ::com::sun::star::uno::RuntimeException)
    {

    }

    void SocketConnection::close()
            throw(::com::sun::star::io::IOException,
                  ::com::sun::star::uno::RuntimeException)
    {
        // enshure close is called only once
        if(  1 == osl_atomic_increment( (&m_nStatus) ) )
        {
            m_socket.shutdown();
            notifyListeners(this, &_closed, callClosed);
        }
    }

    OUString SocketConnection::getDescription()
            throw( ::com::sun::star::uno::RuntimeException)
    {
        return m_sDescription;
    }


    // XConnectionBroadcaster
    void SAL_CALL SocketConnection::addStreamListener(const Reference<XStreamListener> & aListener) throw(RuntimeException)
    {
        MutexGuard guard(_mutex);

        _listeners.insert(aListener);
    }

    void SAL_CALL SocketConnection::removeStreamListener(const Reference<XStreamListener> & aListener) throw(RuntimeException)
    {
        MutexGuard guard(_mutex);

        _listeners.erase(aListener);
    }

    SocketAcceptor::SocketAcceptor( const OUString &sSocketName,
                                    sal_uInt16 nPort,
                                    sal_Bool bTcpNoDelay,
                                    const OUString &sConnectionDescription) :
        m_sSocketName( sSocketName ),
        m_sConnectionDescription( sConnectionDescription ),
        m_nPort( nPort ),
        m_bTcpNoDelay( bTcpNoDelay ),
        m_bClosed( sal_False )
    {
    }


    void SocketAcceptor::init()
    {
        if( ! m_addr.setPort( m_nPort ) )
        {
            OUStringBuffer message( 128 );
            message.appendAscii( "acc_socket.cxx:SocketAcceptor::init - error - invalid tcp/ip port " );
            message.append( (sal_Int32) m_nPort );
            throw ConnectionSetupException(
                message.makeStringAndClear() , Reference< XInterface> () );
        }
        if( ! m_addr.setHostname( m_sSocketName.pData ) )
        {
            OUStringBuffer message( 128 );
            message.appendAscii( "acc_socket.cxx:SocketAcceptor::init - error - invalid host " );
            message.append( m_sSocketName );
            throw ConnectionSetupException(
                message.makeStringAndClear(), Reference< XInterface > () );
        }
        m_socket.setOption( osl_Socket_OptionReuseAddr, 1);

        if(! m_socket.bind(m_addr) )
        {
            OUStringBuffer message( 128 );
            message.appendAscii( "acc_socket.cxx:SocketAcceptor::init - error - couldn't bind on " );
            message.append( m_sSocketName ).appendAscii( ":" ).append((sal_Int32)m_nPort);
            throw ConnectionSetupException(
                message.makeStringAndClear(),
                Reference<XInterface>());
        }

        if(! m_socket.listen() )
        {
            OUStringBuffer message( 128 );
            message.appendAscii( "acc_socket.cxx:SocketAcceptor::init - error - can't listen on " );
            message.append( m_sSocketName ).appendAscii( ":" ).append( (sal_Int32) m_nPort);
            throw ConnectionSetupException( message.makeStringAndClear(),Reference<XInterface>() );
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
        if( m_bTcpNoDelay )
        {
            sal_Int32 nTcpNoDelay = sal_True;
            pConn->m_socket.setOption( osl_Socket_OptionTcpNoDelay , &nTcpNoDelay,
                                       sizeof( nTcpNoDelay ) , osl_Socket_LevelTcp );
        }

        return Reference < XConnection > ( (XConnection * ) pConn );
    }

    void SocketAcceptor::stopAccepting()
    {
        m_bClosed = sal_True;
        m_socket.close();
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
