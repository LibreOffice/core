/*************************************************************************
 *
 *  $RCSfile: acc_socket.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: svesik $ $Date: 2000-11-23 14:39:46 $
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
#include "acceptor.hxx"

#include <hash_set>

#include <com/sun/star/connection/XConnectionBroadcaster.hpp>
#include <com/sun/star/connection/ConnectionSetupException.hpp>

#include <cppuhelper/implbase2.hxx>

using namespace ::osl;
using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::connection;


namespace stoc_acceptor {
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


    typedef ::std::hash_set< ::com::sun::star::uno::Reference< ::com::sun::star::io::XStreamListener>,
                             ReferenceHash< ::com::sun::star::io::XStreamListener>,
                             ReferenceEqual< ::com::sun::star::io::XStreamListener> >
            XStreamListener_hash_set;


    class SocketConnection : public ::cppu::WeakImplHelper2<
        ::com::sun::star::connection::XConnection,
        ::com::sun::star::connection::XConnectionBroadcaster>

    {
    public:
        SocketConnection( const ::rtl::OUString & s , sal_uInt16 nPort , sal_Bool bIgnoreClose );

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
        virtual ::rtl::OUString SAL_CALL getDescription(  )
            throw(::com::sun::star::uno::RuntimeException);

        // XConnectionBroadcaster
        virtual void SAL_CALL addStreamListener(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStreamListener>& aListener)
            throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeStreamListener(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStreamListener>& aListener)
            throw(::com::sun::star::uno::RuntimeException);

    public:
        ::vos::OStreamSocket m_socket;
        ::vos::OInetSocketAddr m_addr;
        oslInterlockedCount m_nStatus;
        ::rtl::OUString m_sDescription;
        sal_Bool m_bIgnoreClose;

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

    callError::callError(const Any & any)
        : any(any)
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


    SocketConnection::SocketConnection( const OUString &s,
                                        sal_uInt16 nPort ,
                                        sal_Bool bIgnoreClose) :
        m_nStatus( 0 ),
        m_sDescription( OUString::createFromAscii( "socket:" ) ),
        m_bIgnoreClose( bIgnoreClose ),
        _started(sal_False),
        _closed(sal_False),
        _error(sal_False)
    {
        m_sDescription += s;
        m_sDescription += OUString::createFromAscii( ":" );
        m_sDescription += OUString::valueOf( (sal_Int32) nPort , 10 );
        m_sDescription += OUString::createFromAscii( ":" );

        // make it unique
        m_sDescription += OUString::valueOf( (sal_Int64) (::vos::OObject * ) &m_socket , 10 );
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
                OUString errMessage;
                m_socket.getError(errMessage);

                OUString message(RTL_CONSTASCII_USTRINGPARAM("acc_socket.cxx:SocketConnection::read: error - "));
                message += errMessage;

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
            OUString message(RTL_CONSTASCII_USTRINGPARAM("acc_socket.cxx:SocketConnection::read: error - connection already closed"));

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
                OUString errMessage;
                m_socket.getError(errMessage);

                OUString message(RTL_CONSTASCII_USTRINGPARAM("acc_socket.cxx:SocketConnection::write: error - "));
                message += errMessage;

                IOException ioException(message, Reference<XInterface>(static_cast<XConnection *>(this)));

                Any any;
                any <<= ioException;

                notifyListeners(this, &_error, callError(any));

                throw ioException;
            }
        }
        else
        {
            OUString message(RTL_CONSTASCII_USTRINGPARAM("acc_socket.cxx:SocketConnection::write: error - connection already closed"));

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
        if( ! m_bIgnoreClose && 1 == osl_incrementInterlockedCount( (&m_nStatus) ) )
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

    SocketAcceptor::SocketAcceptor( const OUString &sSocketName ,
                                    sal_uInt16 nPort ,
                                    sal_Bool bIgnoreClose ) :
        m_bClosed( sal_False ),
        m_sSocketName( sSocketName ),
        m_nPort( nPort ),
        m_bIgnoreClose( bIgnoreClose )
    {
    }


    void SocketAcceptor::init()
    {
        m_addr.setPort( m_nPort );
#ifdef ENABLEUNICODE
        m_addr.setAddr( m_sSocketName.pData );
#else
        OString o = OUStringToOString( m_sSocketName , RTL_TEXTENCODING_ASCII_US );
        m_addr.setAddr( o.pData->buffer );
#endif
        m_socket.setReuseAddr(1);

        if(! m_socket.bind(m_addr) )
        {
            throw ConnectionSetupException(
                OUString(RTL_CONSTASCII_USTRINGPARAM("acc_socket.cxx:SocketAcceptor::init - error - couldn't bind")),
                Reference<XInterface>());
        }

        if(! m_socket.listen() )
        {
            throw ConnectionSetupException(
                OUString(RTL_CONSTASCII_USTRINGPARAM("acc_socket.cxx:SocketAcceptor::init - error - can not listen")),
                Reference<XInterface>());
        }
    }

    Reference< XConnection > SocketAcceptor::accept( )
    {

        SocketConnection *pConn = new SocketConnection( m_sSocketName , m_nPort, m_bIgnoreClose );

        if( m_socket.acceptConnection( pConn->m_socket, pConn->m_addr )!= osl_Socket_Ok )
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

        pConn->m_socket.setTcpNoDelay( 1 );

        return Reference < XConnection > ( (XConnection * ) pConn );
    }

    void SocketAcceptor::stopAccepting()
    {
        m_bClosed = sal_True;
        m_socket.close();
    }
}


