/*************************************************************************
 *
 *  $RCSfile: socket_decl.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jbu $ $Date: 2001-03-14 16:28:31 $
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

#ifndef _OSL_SOCKET_DECL_HXX_
#define _OSL_SOCKET_DECL_HXX_

#ifndef _OSL_SOCKET_H_
#include <osl/socket.h>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _RTL_BYTESEQ_HXX_
#include <rtl/byteseq.hxx>
#endif

namespace osl
{
    /** The class should be understood as a reference to a socket address handle ( struct sockaddr ).

        The handle is mutable.
     */
    enum __osl_socket_NoCopy { SAL_NO_COPY };

    class SocketAddr
    {
    protected:
        oslSocketAddr m_handle;
    public:

        /** Creates socket address of unknown type.
         */
        inline SocketAddr();

        /** Copy constructor.
         */
        inline SocketAddr(const SocketAddr& Addr);

        /** The SocketAddr takes over the responsibility of the handle ( which means,
            that the handle gets destructed by the destructor of this reference=
         */
        inline SocketAddr(const oslSocketAddr , __osl_socket_NoCopy nocopy );

        /**
         */
        inline SocketAddr(oslSocketAddr Addr);

        /** tcpip-specif constructor.
            @param strAddrOrHostName strAddrOrHostName hostname or dotted ip-number of the network
                                     interface, the socket shall be created on.
            @param nPort             tcp-ip port number
         */
        inline SocketAddr( const ::rtl::OUString& strAddrOrHostName, sal_Int32 nPort );

        /** destroys underlying oslSocketAddress
         */
        inline ~SocketAddr();

        /** checks, if the SocketAddr was created successful.
         */
        inline sal_Bool is() const;

        /** Converts the address to a (human readable) domain-name.
            @param pResult 0, if you are not interested in errors,
                           otherwise *pResult contains an error code on failure
                           or osl_Socket_Ok on success
            @return the hostname of this SocketAddr or an empty string on failure.
        */
        inline ::rtl::OUString SAL_CALL getHostname( oslSocketResult *pResult = 0 ) const;

        inline sal_Bool SAL_CALL setHostname( const ::rtl::OUString &sDottedIpOrHostname );

        /** Returns the port number of the address.
            @return the port in host-byte order or or OSL_INVALID_PORT on errors.
        */
        inline sal_Int32 SAL_CALL getPort() const;

        /** Sets the port number of the address.
           @return true if successfule.
         */
        inline sal_Bool SAL_CALL setPort( sal_Int32 nPort );

        /** Sets the address of the underlying socket address struct in network byte order.
            @return true on success, false signales falure.
         */
        inline sal_Bool SAL_CALL setAddr( const ::rtl::ByteSequence & address );

        /** Returns the address of the underlying socket in network byte order
          */
        inline ::rtl::ByteSequence  SAL_CALL getAddr( oslSocketResult *pResult = 0 ) const;

        /** assign the handle to this reference. The previous handle is release.
        */
        inline SocketAddr & SAL_CALL operator= (oslSocketAddr Addr);

        /**
         */
        inline SocketAddr & SAL_CALL operator= (const SocketAddr& Addr);

        /** Returns true if the underlying handle is identical to the Addr handle.
         */
        inline sal_Bool SAL_CALL operator== (oslSocketAddr Addr) const;

        /** Returns true if the underlying handle is identical to the Addr handle.
         */
        inline sal_Bool SAL_CALL operator== (const SocketAddr & Addr) const;

        /** Returns the underlying SocketAddr handle without copyconstructing it.
         */
        inline oslSocketAddr SAL_CALL getHandle() const;

        /** Get the hostname for the local interface.
            @param after the call *pResult contains osl_Socket_Ok on success or
                   an error on failure.
            @return the hostname
        */
        static inline ::rtl::OUString SAL_CALL getLocalHostname( oslSocketResult *pResult = 0);

        /** Tries to find an address for a host.
            @return A new created socket-address or 0 if the name could not be found.
        */
        static inline SocketAddr SAL_CALL resolveHostname(const ::rtl::OUString & strHostName);

        /**
           Tries to find the port associated with the given service/protocol-
           pair (e.g. "ftp"/"tcp").
           @return the port number in host-byte order or CVOS_PORT_NONE
           if no service/protocol pair could be found.
        */
        static inline sal_Int32 SAL_CALL getServicePort(
            const ::rtl::OUString& strServiceName,
            const ::rtl::OUString & strProtocolName= ::rtl::OUString::createFromAscii( "tcp" ) );
    };


    class Socket
    {
    protected:
        oslSocket m_handle;
    protected:
        /** Creates a socket.
            @param Family
            @param Type
            @param Protocol
        */
        inline Socket(oslSocketType Type,
                      oslAddrFamily Family = osl_Socket_FamilyInet,
                      oslProtocol   Protocol = osl_Socket_ProtocolIp);
    public:
        inline Socket( );

        inline Socket( const Socket & socket );

        inline Socket( oslSocket socketHandle );

        inline Socket( oslSocket socketHandle, __sal_NoAcquire noacquire );

        /** Destructor. Releases the underlying handle
         */
        inline ~Socket();

        /** Assignment operator. If socket was already created, the old one will
            be discarded.
        */
        inline Socket& SAL_CALL operator= ( oslSocket socketHandle);

        /** Assignment operator. If socket was already created, the old one will
            be discarded.
        */
        inline Socket& SAL_CALL operator= (const Socket& sock);

        /**
           @return true, when the underlying handle of both
                         Socket references are identical.
                   false otherwise
         */
        inline sal_Bool SAL_CALL operator==( const Socket& rSocket ) const ;

        /**
           @return true, when the underlying handle of this socket reference
                         and the parameter is identical.
                   false otherwise
         */
        inline sal_Bool SAL_CALL operator==( const oslSocket socketHandle ) const;

        /** Closes a definite direction of the bidirectional stream

           @param Direction
         */
        inline void SAL_CALL shutdown( oslSocketDirection Direction = osl_Socket_DirReadWrite );

        /** Closes a socket.
            Note that closing a socket is identical to shutdown( osl_Socket_DirReadWrite ),
            as the operating system distinguish both cases, both functions or offered in this API.
         */
        inline void SAL_CALL close();

        /** Retrieves the address of the local interface of this socket.
            @return Addr [out] receives the address.
        */
        inline SocketAddr SAL_CALL getLocalAddr() const;

        /** Get the local port of the socket.
            @return the port number or OSL_INVALID_PORT on errors.
        */
        inline sal_Int32    SAL_CALL getLocalPort() const;

        /** Get the hostname for the local interface.
            @return the hostname or an empty string ("").
        */
        inline ::rtl::OUString SAL_CALL getLocalHost() const;

        /** Retrieves the address of the remote host of this socket.
            @param Addr [out] receives the address.
        */
        inline SocketAddr SAL_CALL getPeerAddr() const;

        /** Get the remote port of the socket.
            @return the port number or OSL_INVALID_PORT on errors.
        */
        inline sal_Int32    SAL_CALL getPeerPort() const;

        /** Get the hostname for the remote interface.
            @return the hostname or an empty string ("").
        */
        inline ::rtl::OUString SAL_CALL getPeerHost() const;

        /** Binds the socket to the specified (local) interface.
            @param LocalInterface Address of the Interface
            @return True if bind was successful.
        */
        inline sal_Bool SAL_CALL bind(const SocketAddr& LocalInterface);


        /** Checks if read operations will block.
            You can specify a timeout-value in seconds/nanoseconds that denotes
            how sal_Int32 the operation will block if the Socket is not ready.
            @return True if read operations (recv, recvFrom, accept) on the Socket
            will NOT block; False if it would block or if an error occured.

            @param pTimeout if 0, the operation will block without a timeout. Otherwise
            the specified amout of time.
        */
        inline sal_Bool SAL_CALL isRecvReady(const TimeValue *pTimeout = 0) const;

        /** Checks if send operations will block.
            You can specify a timeout-value in seconds/nanoseconds that denotes
            how sal_Int32 the operation will block if the Socket is not ready.
            @return True if send operations (send, sendTo) on the Socket
            will NOT block; False if it would block or if an error occured.

            @param pTimeout if 0, the operation will block without a timeout. Otherwise
            the specified amout of time.
        */
        inline sal_Bool SAL_CALL isSendReady(const TimeValue *pTimeout = 0) const;


        /** Checks if a request for out-of-band data will block.
            You can specify a timeout-value in seconds/nanoseconds that denotes
            how sal_Int32 the operation will block if the Socket has no pending OOB data.

            @return True if OOB-request operations (recv with appropriate flags)
            on the Socket will NOT block; False if it would block or if an error occured.

            @param pTimeout if 0, the operation will block without a timeout. Otherwise
            the specified amout of time.
        */
        inline sal_Bool SAL_CALL isExceptionPending(const TimeValue *pTimeout = 0) const;


        /** Queries the socket for its type.
            @return one of:
        */
        inline oslSocketType    SAL_CALL getType() const;

        /** Retrieves option-attributes associated with the socket.
            @param Option The attribute to query.
            Valid values (depending on the Level) are:

            - osl_Socket_Option_Debug
              (sal_Bool) Socket debug flag 1 = enabled, 0 = disabled.

            - osl_Socket_OptionAcceptConn
            - osl_Socket_OptionReuseAddr
              (sal_Bool) Allows the socket to be bound to an address that is
              already in use.
              1 = multiple bound allowed, 0 = no multiple bounds allowed

            - osl_Socket_OptionKeepAlive
               (sal_Bool) Keepalive packets are sent by the underlying socket.
               1 = enabled, 0 = disabled

            - osl_Socket_OptionDontRoute
               (sal_Bool) Do not route: send directly to interface.
                  1 = do not route , 0 = routing possible

            - osl_Socket_OptionBroadcast
               (sal_Bool) Transmission of broadcast messages are allowed on the socket.
               1 = transmission allowed, 0 = transmission disallowed

            - osl_Socket_OptionUseLoopback

            - osl_Socket_OptionLinger
               (sal_Int32) Linger on close if unsent data is present.
               0 = linger is off, > 0  = timeout in seconds.

            - osl_Socket_OptionOOBinLine


            - osl_Socket_OptionSndBuf
              (sal_Int32) Size of the send buffer in bytes. Data is sent after
              SndTimeo or when the buffer is full. This allows faster writing
              to the socket.

            - osl_Socket_OptionRcvBuf
              (sal_Int32) Size of the receive buffer in bytes. Data is sent after
              SndTimeo or when the buffer is full. This allows faster writing
              to the socket and larger packet sizes.

            - osl_Socket_OptionSndLowat

            - osl_Socket_OptionRcvLowat

            - osl_Socket_OptionSndTimeo
              (sal_Int32) Data is sent after this timeout. This allows gathering
              of data to send larger packages but increases latency times.

            - osl_Socket_OptionRcvTimeo

            - osl_Socket_OptionError
            - osl_Socket_OptionType

            - osl_Socket_OptionTcpNoDelay
              Disables the Nagle algorithm for send coalescing. (Do not
              collect data until a packet is full, instead send immediatly.
              This increases network traffic but might improve latency-times.)
              1 = disables the algorithm, 0 = keeps it enabled.

            If not above mentioned otherwise, the options are only valid for
            level TLevel_Socket.

            @param pBuffer The Buffer will be filled with the attribute.

            @param BufferSize The size of pBuffer.

            @param Level The option level. Valid values are:
            - osl_Socket_LevelSocket : Socket Level
            - osl_Socket_LevelTcp    : Level of Transmission Control Protocol

            @return The size of the attribute copied into pBuffer ot -1 if an error
            occured.
        */
        inline sal_Int32 SAL_CALL getOption(
            oslSocketOption Option,
            void* pBuffer,
            sal_uInt32 BufferLen,
            oslSocketOptionLevel Level= osl_Socket_LevelSocket) const;

        /** Sets the sockets attributes.

            @param Option denotes the option to modify.
            Valid values (depending on the Level) are:
            - osl_Socket_Option_Debug
            - osl_Socket_OptionAcceptConn
            - osl_Socket_OptionReuseAddr
            - osl_Socket_OptionKeepAlive
            - osl_Socket_OptionDontRoute
            - osl_Socket_OptionBroadcast
            - osl_Socket_OptionUseLoopback
            - osl_Socket_OptionLinger
            - osl_Socket_OptionOOBinLine
            - osl_Socket_OptionSndBuf
            - osl_Socket_OptionRcvBuf
            - osl_Socket_OptionSndLowat
            - osl_Socket_OptionRcvLowat
            - osl_Socket_OptionSndTimeo
            - osl_Socket_OptionRcvTimeo
            - osl_Socket_OptionError
            - osl_Socket_OptionType
            - osl_Socket_OptionTcpNoDelay

            If not above mentioned otherwise, the options are only valid for
            level osl_Socket_LevelSocket.

            @param pBuffer Pointer to a Buffer which contains the attribute-value.

            @param BufferSize contains the length of the Buffer.

            @param Level selects the level for which an option should be changed.
            Valid values are:
            - osl_Socket_evel_Socket : Socket Level
            - osl_Socket_Level_Tcp   : Level of Transmission Control Protocol

            @return True if the option could be changed.
        */
        inline sal_Bool SAL_CALL setOption( oslSocketOption Option,
                                            void* pBuffer,
                                            sal_uInt32 BufferLen,
                                            oslSocketOptionLevel Level= osl_Socket_LevelSocket ) const;

        /** Convenience function for setting sal_Bool and sal_Int32 option values.
            @see setOption for a complete description of possible options.
         */
        inline sal_Bool setOption( oslSocketOption option, sal_Int32 nValue  );

        /** Convenience function for retrieving sal_Bool and sal_Int32 option values.
            @see setOption for a complete description of possible options.
         */
        inline sal_Int32 getOption( oslSocketOption option ) const;

        /** Enables/disables non-blocking mode of the socket.
            @param bNonBlockingMode If True, blocking mode will be switched off
                   If false, the socket will become a blocking
                   socket (which is the default behaviour of a socket).
            @return True if mode could be set.
        */
        inline sal_Bool SAL_CALL enableNonBlockingMode( sal_Bool bNonBlockingMode);

        /** Query blocking mode of the socket.
            @return True if non-blocking mode is set.
        */
        inline sal_Bool SAL_CALL isNonBlockingMode() const;


        /** Gets clears the error status
            @returns the current error state.
        */
        inline void SAL_CALL clearError() const;

        /** Delivers a constant decribing the last error for the socket system.
            @return ENONE if no error occured, invalid_SocketError if
            an unknown (unmapped) error occured, otherwise an enum describing the
            error.
        */
        inline oslSocketError getError() const;

        /** Builds a string with the last error-message for the socket.
         */
        inline ::rtl::OUString getErrorAsString( ) const;

        /** Returns the underlying handle unacquired  (The caller must acquire it to keep it)
         */
        inline oslSocket getHandle() const;
    };


    class StreamSocket : public Socket
    {
    public:
        /** Creates a socket that can accept connections.
            @param Type For some protocols it might be desirable to
            use a different type than sock_stream (like sock_seqpacket).
            Therefore we do not hide this parameter here.
        */
          inline StreamSocket(oslAddrFamily Family = osl_Socket_FamilyInet,
                            oslProtocol Protocol = osl_Socket_ProtocolIp,
                            oslSocketType   Type = osl_Socket_TypeStream);

        inline StreamSocket( const StreamSocket & );

        inline StreamSocket( oslSocket Socket , __sal_NoAcquire noacquire );

        inline StreamSocket( oslSocket Socket );

        /** Retrieves n bytes from the stream and copies them into pBuffer.
            The method avoids incomplete reads due to packet boundaries.
            @param pBuffer receives the read data.
            @param n the number of bytes to read. pBuffer must be large enough
            to hold the n bytes!
            @return the number of read bytes. The number will only be smaller than
            n if an exceptional condition (e.g. connection closed) occurs.
        */
        inline sal_Int32 SAL_CALL read(void* pBuffer, sal_uInt32 n);

        /** Writes n bytes from pBuffer to the stream. The method avoids
            incomplete writes due to packet boundaries.
            @param pBuffer contains the data to be written.
            @param n the number of bytes to write.
            @return the number of written bytes. The number will only be smaller than
            n if an exceptional condition (e.g. connection closed) occurs.
        */
        inline sal_Int32 SAL_CALL write(const void* pBuffer, sal_uInt32 n);


        /** Tries to receives BytesToRead data from the connected socket,

            @param pBuffer [out] Points to a buffer that will be filled with the received
            data.
            @param BytesToRead [in] The number of bytes to read. pBuffer must have at least
            this size.
            @param Flag [in] Modifier for the call. Valid values are:

            - TMsg_Normal
            - TMsg_OOB
            - TMsg_Peek
            - TMsg_DontRoute
            - TMsg_MaxIOVLen
            @return the number of received bytes.
        */
        inline sal_Int32 SAL_CALL recv(void* pBuffer,
                                       sal_uInt32 BytesToRead,
                                       oslSocketMsgFlag flags= osl_Socket_MsgNormal);

        /** Tries to sends BytesToSend data from the connected socket.

            @param pBuffer [in] Points to a buffer that contains the send-data.
            @param BytesToSend [in] The number of bytes to send. pBuffer must have at least
            this size.
            @param Flag [in] Modifier for the call. Valid values are:
            - TMsg_Normal
            - TMsg_OOB
            - TMsg_Peek
            - TMsg_DontRoute
            - TMsg_MaxIOVLen

            @return the number of transfered bytes.
        */
        sal_Int32 SAL_CALL send(const void* pBuffer,
                                sal_uInt32 BytesToSend,
                                oslSocketMsgFlag= osl_Socket_MsgNormal);
    };

    class ConnectorSocket : public StreamSocket
    {
    public:
        /** Creates a socket that can connect to a (remote) host.
            @param Type For some protocols it might be desirable to
            use a different type than sock_stream (like sock_seqpacket).
            Therefore we do not hide this parameter here.
        */
          ConnectorSocket(oslAddrFamily Family = osl_Socket_FamilyInet,
                        oslProtocol Protocol = osl_Socket_ProtocolIp,
                        oslSocketType   Type = osl_Socket_TypeStream);


        /** Connects the socket to a (remote) host.
            @param TargetHost The address of the target.
            @param pTimeOut The timeout for blocking.
            @return result_ok if connected successfully,
            result_timeout on timeout,
            result_interrupted if unblocked forcefully (by close()),
            result_error if connect failed.
        */
        oslSocketResult SAL_CALL connect(const SocketAddr& TargetHost, const TimeValue* pTimeout = 0);
    };

    class AcceptorSocket : public Socket
    {
    public:
        inline AcceptorSocket(oslAddrFamily Family = osl_Socket_FamilyInet,
                              oslProtocol   Protocol = osl_Socket_ProtocolIp,
                              oslSocketType Type = osl_Socket_TypeStream);

        /** Prepare a socket for the accept-call. The socket must have been
            bound before to the local address.
            @param MaxPendingConnections The maximum number of pending
            connections (waiting to be accepted) on this socket. If you use
            -1, a system default value is used.
            @return True if call was successful.
        */
        inline sal_Bool SAL_CALL listen(sal_Int32 MaxPendingConnections= -1);

        /** Accepts incoming connections on the socket. You must
            precede this call with bind() and listen().
            @param Connection receives the incoming connection.
            @return result_ok: if a connection has been accepted,
            result_timeout: if m_RecvTimeout milliseconds passed without connect,
            result_error: on errors.
        */
        inline oslSocketResult SAL_CALL acceptConnection( StreamSocket& Connection);

        /** Accepts incoming connections on the socket. You must
            precede this call with bind() and listen().
            @param PeerAddr receives the address of the connecting entity
            (your communication partner).
            @param Connection receives the incoming connection.
            @return True if a connection has been accepted, False on errors.
            @return result_ok: if a connection has been accepted,
            result_timeout: if m_RecvTimeout milliseconds passed without connect,
            result_error: on errors.
        */
        inline oslSocketResult SAL_CALL acceptConnection( StreamSocket& Connection, SocketAddr & PeerAddr);
    };



    /** A connectionless socket to send and receive datagrams.
     */
    class DatagramSocket : public Socket
    {
    public:

        /** Creates a datagram socket.
            @param Type is sock_dgram by default.
        */
        inline DatagramSocket(oslAddrFamily Family= osl_Socket_FamilyInet,
                              oslProtocol   Protocol= osl_Socket_ProtocolIp,
                              oslSocketType Type= osl_Socket_TypeDgram);

        /** Tries to receives BufferSize data from the socket, if no error occurs.

            @param pSenderAddr [out] You must provide pointer to a SocketAddr.
            It will be  filled with the address of the datagrams sender.
            If pSenderAddr is 0, it is ignored.
            @param pBuffer [out] Points to a buffer that will be filled with the received
            datagram.
            @param BufferSize [in] The size of pBuffer.
            @param Flag [in] Modifier for the call. Valid values are:
            <ul>
            <li> TMsg_Normal
            <li> TMsg_OOB
            <li> TMsg_Peek
            <li> TMsg_DontRoute
            <li> TMsg_MaxIOVLen
            </ul>

            @return the number of received bytes.
        */
        inline sal_Int32 SAL_CALL recvFrom(void*  pBuffer,
                                           sal_uInt32 BufferSize,
                                           SocketAddr* pSenderAddr= 0,
                                           oslSocketMsgFlag Flag= osl_Socket_MsgNormal);

        /** Tries to send one datagram with BytesToSend data to the given ReceiverAddr.
            Since we only send one packet, we don't need to concern ourselfes here with
            incomplete sends due to packet boundaries.

            @param ReceiverAddr [in] A SocketAddr that contains
            the destination address for this send.

            @param pBuffer [in] Points to a buffer that contains the send-data.
            @param BufferSize [in] The number of bytes to send. pBuffer must have at least
            this size.
            @param Flag [in] Modifier for the call. Valid values are:

            - osl_Socket_MsgNormal
            - osl_Socket_MsgOOB
            - osl_Socket_MsgPeek
            - osl_Socket_MsgDontRoute
            - osl_Socket_MsgMaxIOVLen
            </ul>

            @return the number of transfered bytes.
        */
        inline sal_Int32    SAL_CALL sendTo( const SocketAddr& ReceiverAddr,
                                             const void* pBuffer,
                                             sal_uInt32 BufferSize,
                                             oslSocketMsgFlag Flag= osl_Socket_MsgNormal);
    };

}

#endif
