/*************************************************************************
 *
 *  $RCSfile: ctr_socket.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: jbu $ $Date: 2000-11-28 08:20:57 $
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

#include "connector.hxx"
#include <rtl/ustrbuf.hxx>

using namespace ::osl;
using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::connection;


namespace stoc_connector {
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


    SocketConnection::SocketConnection( const OUString &s, sal_uInt16 nPort, const OUString &sConnectionDescription ) :
        m_nStatus( 0 ),
        m_sDescription( sConnectionDescription ),
        _started(sal_False),
        _closed(sal_False),
        _error(sal_False)
    {
        // make it unique
        m_sDescription += OUString( RTL_CONSTASCII_USTRINGPARAM( ",uniqueValue=" ) );
        m_sDescription += OUString::valueOf( (sal_Int64) &m_socket , 10 );
    }

    void SocketConnection::completeConnectionString()
    {
        OUString sHost;
        sal_Int32 nPort;

        nPort = m_socket.getPeerPort();
        m_socket.getPeerHost( sHost );

        OUStringBuffer buf( 256 );
        buf.appendAscii( ",peerPort=" );
        buf.append( (sal_Int32) nPort );
        buf.appendAscii( ",peerHost=" );
        buf.append( sHost );


        nPort = m_socket.getLocalPort();
        m_socket.getLocalHost( sHost );

        buf.appendAscii( ",localPort=" );
        buf.append( (sal_Int32) nPort );
        buf.appendAscii( ",localHost=" );
        buf.append( sHost );

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
            sal_Int32 i = m_socket.read( aReadBytes.getArray()  , aReadBytes.getLength() );

            if(i != nBytesToRead)
            {
                OUString errMessage;
                m_socket.getError(errMessage);

                OUString message(RTL_CONSTASCII_USTRINGPARAM("ctr_socket.cxx:SocketConnection::read: error - "));
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
            OUString message(RTL_CONSTASCII_USTRINGPARAM("ctr_socket.cxx:SocketConnection::read: error - connection already closed"));

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

                OUString message(RTL_CONSTASCII_USTRINGPARAM("ctr_socket.cxx:SocketConnection::write: error - "));
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
            OUString message(RTL_CONSTASCII_USTRINGPARAM("ctr_socket.cxx:SocketConnection::write: error - connection already closed"));

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
            // ensure that close is called only once
        if( 1 == osl_incrementInterlockedCount( (&m_nStatus) ) )
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
}

