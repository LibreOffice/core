/*************************************************************************
 *
 *  $RCSfile: socket.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jbu $ $Date: 2001-03-19 16:54:15 $
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
#ifndef _OSL_SOCKET_HXX_
#define _OSL_SOCKET_HXX_

#ifndef _OSL_SOCKET_DECL_HXX_
#include <osl/socket_decl.hxx>
#endif

namespace osl
{
    //______________________________________________________________________________
    inline SocketAddr::SocketAddr()
        : m_handle( osl_createEmptySocketAddr( osl_Socket_FamilyInet ) )
    {}

    //______________________________________________________________________________
    inline SocketAddr::SocketAddr(const SocketAddr& Addr)
        : m_handle( osl_copySocketAddr( Addr.m_handle ) )
    {
    }

    //______________________________________________________________________________
    inline SocketAddr::SocketAddr(oslSocketAddr Addr)
        : m_handle( osl_copySocketAddr( Addr ) )
    {
    }

    //______________________________________________________________________________
    inline SocketAddr::SocketAddr(oslSocketAddr Addr, __osl_socket_NoCopy nocopy )
        : m_handle( Addr )
    {
    }

    //______________________________________________________________________________
    inline SocketAddr::SocketAddr( const ::rtl::OUString& strAddrOrHostName, sal_Int32 nPort)
        : m_handle( osl_createInetSocketAddr( strAddrOrHostName.pData, nPort ) )
    {
        if(! m_handle )
        {
            m_handle = osl_resolveHostname(strAddrOrHostName.pData);

            // host found?
            if(m_handle)
            {
                osl_setInetPortOfSocketAddr(m_handle, nPort);
            }
            else
            {
                osl_destroySocketAddr( m_handle );
                m_handle = 0;
            }
        }
    }

    //______________________________________________________________________________
    inline SocketAddr::~SocketAddr()
    {
        if( m_handle )
            osl_destroySocketAddr( m_handle );
    }

    //______________________________________________________________________________
    inline ::rtl::OUString SocketAddr::getHostname( oslSocketResult *pResult ) const
    {
        ::rtl::OUString hostname;
        oslSocketResult result = osl_getHostnameOfSocketAddr( m_handle, &(hostname.pData) );
        if( pResult )
            *pResult = result;
        return hostname;
    }

    //______________________________________________________________________________
    inline sal_Int32 SAL_CALL SocketAddr::getPort() const
    {
        return osl_getInetPortOfSocketAddr(m_handle);
    }

    //______________________________________________________________________________
    inline sal_Bool SAL_CALL SocketAddr::setPort( sal_Int32 nPort )
    {
        return osl_setInetPortOfSocketAddr(m_handle, nPort );
    }

    inline sal_Bool SAL_CALL SocketAddr::setHostname( const ::rtl::OUString &sDottedIpOrHostname )
    {
        *this = SocketAddr( sDottedIpOrHostname , getPort() );
        return is();
    }

    //______________________________________________________________________________
    inline sal_Bool SAL_CALL SocketAddr::setAddr( const ::rtl::ByteSequence & address )
    {
        return osl_setAddrOfSocketAddr( m_handle, address.getHandle() );
    }

    inline ::rtl::ByteSequence SAL_CALL SocketAddr::getAddr( oslSocketResult *pResult ) const
    {
        ::rtl::ByteSequence sequence;
        oslSocketResult result = osl_getAddrOfSocketAddr( m_handle,(sal_Sequence **) &sequence );
        if( pResult )
            *pResult = result;
        return sequence;
    }

    //______________________________________________________________________________
    inline SocketAddr & SAL_CALL SocketAddr::operator= (oslSocketAddr Addr)
    {
        oslSocketAddr pNewAddr = osl_copySocketAddr( Addr );
        if( m_handle )
            osl_destroySocketAddr( m_handle );
        m_handle = pNewAddr;
        return *this;
    }

    //______________________________________________________________________________
    inline SocketAddr & SAL_CALL SocketAddr::operator= (const SocketAddr& Addr)
    {
        *this = (Addr.getHandle());
        return *this;
    }

    inline SocketAddr & SAL_CALL SocketAddr::assign( oslSocketAddr Addr, __osl_socket_NoCopy nocopy )
    {
        if( m_handle )
            osl_destroySocketAddr( m_handle );
        m_handle = Addr;
        return *this;
    }

    //______________________________________________________________________________
    inline sal_Bool SAL_CALL SocketAddr::operator== (oslSocketAddr Addr) const
    {
        return osl_isEqualSocketAddr( m_handle, Addr );
    }

    inline oslSocketAddr SocketAddr::getHandle() const
    {
        return m_handle;
    }

    //______________________________________________________________________________
    inline sal_Bool SocketAddr::is() const
    {
        return m_handle != 0;
    }

    // (static method)______________________________________________________________
    inline ::rtl::OUString SAL_CALL SocketAddr::getLocalHostname( oslSocketResult *pResult )
    {
        ::rtl::OUString hostname;
        oslSocketResult result = osl_getLocalHostname( &(hostname.pData) );
        if(pResult )
            result = *pResult;
        return hostname;
    }

    // (static method)______________________________________________________________
    inline void SAL_CALL SocketAddr::resolveHostname(
        const ::rtl::OUString & strHostName, SocketAddr &Addr)
    {
        Addr = SocketAddr( osl_resolveHostname( strHostName.pData ) , SAL_NO_COPY );
    }

    // (static method)______________________________________________________________
    inline sal_Int32 SAL_CALL SocketAddr::getServicePort(
            const ::rtl::OUString& strServiceName,
            const ::rtl::OUString & strProtocolName )
    {
        return osl_getServicePort( strServiceName.pData, strProtocolName.pData );
    }

    //______________________________________________________________________________
    inline Socket::Socket(oslSocketType Type,
                          oslAddrFamily Family,
                          oslProtocol   Protocol)
        : m_handle( osl_createSocket(Family, Type, Protocol) )
    {}

    //______________________________________________________________________________
    inline Socket::Socket( oslSocket socketHandle, __sal_NoAcquire noacquire )
        : m_handle( socketHandle )
    {}

    //______________________________________________________________________________
    inline Socket::Socket( oslSocket socketHandle )
        : m_handle( socketHandle )
    {
        osl_acquireSocket( m_handle );
    }

    //______________________________________________________________________________
    inline Socket::Socket( const Socket & socket )
        : m_handle( socket.getHandle() )
    {
        osl_acquireSocket( m_handle );
    }

    //______________________________________________________________________________
    inline Socket::~Socket()
    {
        osl_releaseSocket( m_handle );
    }

    //______________________________________________________________________________
    inline Socket& Socket::operator= ( oslSocket socketHandle)
    {
        osl_acquireSocket( socketHandle );
        osl_releaseSocket( m_handle );
        m_handle = socketHandle;
        return *this;
    }

    //______________________________________________________________________________
    inline Socket&  Socket::operator= (const Socket& sock)
    {
        return (*this) = sock.getHandle();
    }

    //______________________________________________________________________________
    inline sal_Bool Socket::operator==( const Socket& rSocket ) const
    {
        return m_handle == rSocket.getHandle();
    }

    //______________________________________________________________________________
    inline sal_Bool Socket::operator==( const oslSocket socketHandle ) const
    {
        return m_handle == socketHandle;
    }

    //______________________________________________________________________________
    inline void Socket::shutdown( oslSocketDirection Direction )
    {
        osl_shutdownSocket( m_handle , Direction );
    }

    //______________________________________________________________________________
    inline void Socket::close()
    {
        osl_closeSocket( m_handle );
    }

    //______________________________________________________________________________
    inline void Socket::getLocalAddr( SocketAddr & addr) const
    {
        addr.assign( osl_getLocalAddrOfSocket( m_handle ) , SAL_NO_COPY );
    }

    //______________________________________________________________________________
    inline sal_Int32 Socket::getLocalPort() const
    {
        SocketAddr addr( 0 );
        getLocalAddr( addr );
        return addr.getPort();
    }

    //______________________________________________________________________________
    inline ::rtl::OUString Socket::getLocalHost() const
    {
        SocketAddr addr( 0 );
        getLocalAddr( addr );
        return addr.getHostname();
    }

    //______________________________________________________________________________
    inline void Socket::getPeerAddr( SocketAddr &addr ) const
    {
        addr.assign( osl_getPeerAddrOfSocket( m_handle ), SAL_NO_COPY );
    }

    //______________________________________________________________________________
    inline sal_Int32 Socket::getPeerPort() const
    {
        SocketAddr addr( 0 );
        getPeerAddr( addr );
        return addr.getPort();
    }

    //______________________________________________________________________________
    inline ::rtl::OUString Socket::getPeerHost() const
    {
        SocketAddr addr( 0 );
        getPeerAddr( addr );
        return addr.getHostname();
    }

    //______________________________________________________________________________
    inline sal_Bool Socket::bind(const SocketAddr& LocalInterface)
    {
        return osl_bindAddrToSocket( m_handle , LocalInterface.getHandle() );
    }

    //______________________________________________________________________________
    inline sal_Bool Socket::isRecvReady(const TimeValue *pTimeout ) const
    {
        return osl_isReceiveReady( m_handle , pTimeout );
    }

    //______________________________________________________________________________
    inline sal_Bool Socket::isSendReady(const TimeValue *pTimeout ) const
    {
        return osl_isSendReady( m_handle, pTimeout );
    }

    //______________________________________________________________________________
    inline sal_Bool Socket::isExceptionPending(const TimeValue *pTimeout ) const
    {
        return osl_isExceptionPending( m_handle, pTimeout );
    }

    //______________________________________________________________________________
    inline oslSocketType Socket::getType() const
    {
        return osl_getSocketType( m_handle );
    }

    //______________________________________________________________________________
    inline sal_Int32  Socket::getOption(
        oslSocketOption Option,
        void* pBuffer,
        sal_uInt32 BufferLen,
        oslSocketOptionLevel Level) const
    {
        return osl_getSocketOption( m_handle, Level, Option, pBuffer , BufferLen );
    }

    //______________________________________________________________________________
    inline sal_Bool Socket::setOption(  oslSocketOption Option,
                                        void* pBuffer,
                                        sal_uInt32 BufferLen,
                                        oslSocketOptionLevel Level ) const
    {
        return osl_setSocketOption( m_handle, Level, Option , pBuffer, BufferLen );
    }

    //______________________________________________________________________________
    inline sal_Bool Socket::setOption( oslSocketOption option, sal_Int32 nValue  )
    {
        return setOption( option, &nValue, sizeof( nValue ) );
    }

    //______________________________________________________________________________
    inline sal_Int32 Socket::getOption( oslSocketOption option ) const
    {
        sal_Int32 n;
        getOption( option, &n, sizeof( n ) );
        return n;
    }

    //______________________________________________________________________________
    inline sal_Bool Socket::enableNonBlockingMode( sal_Bool bNonBlockingMode)
    {
        return osl_enableNonBlockingMode( m_handle , bNonBlockingMode );
    }

    //______________________________________________________________________________
    inline sal_Bool Socket::isNonBlockingMode() const
    {
        return osl_isNonBlockingMode( m_handle );
    }

    //______________________________________________________________________________
    inline void SAL_CALL Socket::clearError() const
    {
        sal_Int32 err = 0;
        getOption(osl_Socket_OptionError, &err, sizeof(err));
    }

    //______________________________________________________________________________
    inline oslSocketError Socket::getError() const
    {
        return osl_getLastSocketError( m_handle );
    }

    //______________________________________________________________________________
    inline ::rtl::OUString Socket::getErrorAsString( ) const
    {
        ::rtl::OUString error;
        osl_getLastSocketErrorDescription( m_handle, &(error.pData) );
        return error;
    }

    //______________________________________________________________________________
    inline oslSocket Socket::getHandle() const
    {
        return m_handle;
    }

    //______________________________________________________________________________
    inline StreamSocket::StreamSocket(oslAddrFamily Family,
                                      oslProtocol Protocol,
                                      oslSocketType Type )
        : Socket( Type, Family, Protocol )
    {}

    //______________________________________________________________________________
    inline StreamSocket::StreamSocket( oslSocket socketHandle, __sal_NoAcquire noacquire )
        : Socket( socketHandle, noacquire )
    {}

    //______________________________________________________________________________
    inline StreamSocket::StreamSocket( oslSocket socketHandle )
        : Socket( socketHandle )
    {}

    //______________________________________________________________________________
    inline StreamSocket::StreamSocket( const StreamSocket & socket )
        : Socket( socket )
    {}

    //______________________________________________________________________________
    inline sal_Int32 StreamSocket::read(void* pBuffer, sal_uInt32 n)
    {
        return osl_readSocket( m_handle, pBuffer, n );
    }

    //______________________________________________________________________________
    inline sal_Int32 StreamSocket::write(const void* pBuffer, sal_uInt32 n)
    {
        return osl_writeSocket( m_handle, pBuffer, n );
    }


    //______________________________________________________________________________
    inline sal_Int32 StreamSocket::recv(void* pBuffer,
                                        sal_uInt32 BytesToRead,
                                        oslSocketMsgFlag Flag)
    {
        return osl_receiveSocket( m_handle, pBuffer,BytesToRead, Flag );
    }

    //______________________________________________________________________________
    inline sal_Int32 StreamSocket::send(const void* pBuffer,
                                        sal_uInt32 BytesToSend,
                                        oslSocketMsgFlag Flag)
    {
        return osl_sendSocket( m_handle, pBuffer, BytesToSend, Flag );
    }

    //______________________________________________________________________________
      inline ConnectorSocket::ConnectorSocket(oslAddrFamily Family,
                                            oslProtocol Protocol,
                                            oslSocketType   Type)
        : StreamSocket( Family, Protocol ,Type )
    {}

    //______________________________________________________________________________
    inline oslSocketResult ConnectorSocket::connect( const SocketAddr& TargetHost,
                                                     const TimeValue* pTimeout )
    {
        return osl_connectSocketTo( m_handle , TargetHost.getHandle(), pTimeout );
    }

    //______________________________________________________________________________
    inline AcceptorSocket::AcceptorSocket(oslAddrFamily Family ,
                                          oslProtocol   Protocol ,
                                          oslSocketType Type )
        : Socket( Type, Family, Protocol )
    {}

    //______________________________________________________________________________
    inline sal_Bool AcceptorSocket::listen(sal_Int32 MaxPendingConnections)
    {
        return osl_listenOnSocket( m_handle, MaxPendingConnections );
    }

    //______________________________________________________________________________
    inline oslSocketResult AcceptorSocket::acceptConnection( StreamSocket& Connection)
    {
        oslSocket o = osl_acceptConnectionOnSocket( m_handle, 0 );
        oslSocketResult status = osl_Socket_Ok;
        if( o )
        {
            Connection = StreamSocket( o , SAL_NO_ACQUIRE );
        }
        else
        {
            Connection = StreamSocket();
            status = osl_Socket_Error;
        }
        return status;
    }

    //______________________________________________________________________________
    inline oslSocketResult AcceptorSocket::acceptConnection(
        StreamSocket&   Connection, SocketAddr & PeerAddr)
    {
        // TODO change in/OUT parameter
        oslSocket o = osl_acceptConnectionOnSocket( m_handle, (oslSocketAddr *)&PeerAddr );
        oslSocketResult status = osl_Socket_Ok;
        if( o )
        {
            Connection = StreamSocket( o , SAL_NO_ACQUIRE );
        }
        else
        {
            Connection = StreamSocket();
            status = osl_Socket_Error;
        }
        return status;
    }

    //______________________________________________________________________________
    inline DatagramSocket::DatagramSocket(oslAddrFamily Family,
                                          oslProtocol   Protocol,
                                          oslSocketType Type)
        : Socket( Type, Family, Protocol )
    {}

    //______________________________________________________________________________
    inline sal_Int32 DatagramSocket::recvFrom(void*  pBuffer,
                                              sal_uInt32 BufferSize,
                                              SocketAddr* pSenderAddr,
                                              oslSocketMsgFlag Flag )
    {
        sal_Int32 nByteRead;
        if( pSenderAddr )
        {
            // TODO : correct the out-parameter pSenderAddr outparameter
              nByteRead = osl_receiveFromSocket( m_handle, pSenderAddr->getHandle() , pBuffer,
                                                 BufferSize, Flag);
//              nByteRead = osl_receiveFromSocket( m_handle, *(oslSocketAddr**) &pSenderAddr , pBuffer,
//                                                 BufferSize, Flag);
        }
        else
        {
            nByteRead = osl_receiveFromSocket( m_handle, 0 , pBuffer , BufferSize ,  Flag );
        }
        return nByteRead;
    }

    //______________________________________________________________________________
    inline sal_Int32  DatagramSocket::sendTo( const SocketAddr& ReceiverAddr,
                                              const void* pBuffer,
                                              sal_uInt32 BufferSize,
                                              oslSocketMsgFlag Flag )
    {
        return osl_sendToSocket( m_handle, ReceiverAddr.getHandle(), pBuffer, BufferSize, Flag );
    }
}
#endif
