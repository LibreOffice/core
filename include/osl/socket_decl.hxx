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

#ifndef INCLUDED_OSL_SOCKET_DECL_HXX
#define INCLUDED_OSL_SOCKET_DECL_HXX

#include "sal/config.h"

#include <cstddef>

#include "osl/socket.h"
#include "rtl/ustring.hxx"
#include "rtl/byteseq.hxx"

namespace osl
{
    enum __osl_socket_NoCopy { SAL_NO_COPY };

    /** The class should be understood as a reference to a socket address handle (struct sockaddr).

        The handle is mutable.
     */
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

#if defined LIBO_INTERNAL_ONLY
        inline SocketAddr(SocketAddr && other);
#endif

        /** The SocketAddr takes over the responsibility of the handle (which means
            that the handle gets destructed by the destructor of this reference)

            @param Addr     a handle
            @param nocopy   use SAL_NO_COPY
         */
        inline SocketAddr(const oslSocketAddr Addr, __osl_socket_NoCopy nocopy );

        /** Copyconstructs the oslSocketAddr handle.

            @param Addr     a handle
         */
        inline SocketAddr(oslSocketAddr Addr);

        /** TCP/IP-specific constructor.

            @param strAddrOrHostName strAddrOrHostName hostname or dotted ip-number of the network
                                     interface, the socket shall be created on.
            @param nPort             tcp-ip port number
         */
        inline SocketAddr(const ::rtl::OUString& strAddrOrHostName, sal_Int32 nPort);

        /** destroys underlying oslSocketAddress
         */
        inline ~SocketAddr();

        /** Checks if the SocketAddr was created successful.

            @retval true    if there is a valid underlying handle
            @retval false   no valid underlying handle
         */
        inline bool is() const;

        /** Converts the address to a (human readable) domain-name.

            @param[out] pResult value of 0 if you are not interested in errors,
                           otherwise *pResult contains an error code on failure
                           or osl_Socket_Ok on success

            @return the hostname of this SocketAddr or an empty string on failure.

            @see osl_getHostnameOfSocketAddr
        */
        inline ::rtl::OUString SAL_CALL getHostname(oslSocketResult *pResult = NULL) const;

        /** Sets the IP address or hostname of the SocketAddress

           @param[in] sDottedIpOrHostname   IP address or hostname

           @retval true     success
           @retval false    failure
         */
        inline bool SAL_CALL setHostname(const ::rtl::OUString &sDottedIpOrHostname);

        /** Returns the port number of the address.

            @return the port in host-byte order or OSL_INVALID_PORT on errors.
        */
        inline sal_Int32 SAL_CALL getPort() const;

        /** Sets the port number of the address.

           @param[in] nPort port number

           @retval true     success
           @retval false    failure
         */
        inline bool SAL_CALL setPort(sal_Int32 nPort);

        /** Sets the address of the underlying socket address struct in network byte order.

           @retval true     success
           @retval false    failure
         */
        inline bool SAL_CALL setAddr(const ::rtl::ByteSequence & address);

        /** Returns the address of the underlying socket in network byte order
          */
        inline ::rtl::ByteSequence SAL_CALL getAddr(oslSocketResult *pResult = NULL) const;

        /** assign the handle to this reference. The previous handle is released.
        */
        inline SocketAddr & SAL_CALL operator= (oslSocketAddr Addr);

        inline SocketAddr & SAL_CALL operator= (const SocketAddr& Addr);

#if defined LIBO_INTERNAL_ONLY
        inline SocketAddr & operator =(SocketAddr && other);
#endif

        /** Assigns the socket addr without copyconstructing it.
            @param Addr the socket address.
            @param nocopy use SAL_NO_COPY
          */
        inline SocketAddr & SAL_CALL assign( oslSocketAddr Addr, __osl_socket_NoCopy nocopy );

        /** Returns true if the underlying handle is identical to the Addr handle.
         */
        inline bool SAL_CALL operator== (oslSocketAddr Addr) const;

        /** Returns true if the underlying handle is identical to the Addr handle.
         */
        inline bool SAL_CALL operator== (const SocketAddr & Addr) const;

        /** Returns the underlying SocketAddr handle without copyconstructing it.
         */
        inline oslSocketAddr SAL_CALL getHandle() const;

        /** Get the hostname for the local interface.
            @param pResult after the call *pResult contains osl_Socket_Ok on success or
                   an error on failure.
            @return the hostname
        */
        static inline ::rtl::OUString SAL_CALL getLocalHostname( oslSocketResult *pResult = NULL);

        /** Tries to find an address for a host.
            @see osl_resolveHostname()
            @return A new created socket-address or 0 if the name could not be found.
        */
        static inline void SAL_CALL resolveHostname(
            const ::rtl::OUString & strHostName , SocketAddr & Addr );

        /**
           Tries to find the port associated with the given service/protocol-
           pair (e.g. "ftp"/"tcp").
           @return the port number in host-byte order or <code>OSL_INVALID_PORT</code>
           if no service/protocol pair could be found.
        */
        static inline sal_Int32 SAL_CALL getServicePort(
            const ::rtl::OUString& strServiceName,
            const ::rtl::OUString & strProtocolName= ::rtl::OUString("tcp") );
    };


    class Socket
    {
    protected:
        oslSocket m_handle;
    protected:
        /** Creates a socket. Note it's protected.
            @param Type
            @param Family
            @param Protocol
        */
        inline Socket(oslSocketType Type,
                      oslAddrFamily Family = osl_Socket_FamilyInet,
                      oslProtocol   Protocol = osl_Socket_ProtocolIp);
    public:
        inline Socket( );

        inline Socket( const Socket & socket );

        inline Socket( oslSocket socketHandle );

        /** The instance takes over the handle's ownership without acquiring the
            handle, but releases it within the dtor.
            @param socketHandle the handle
            @param noacquire use SAL_NO_ACQUIRE
         */
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
           @return <code>true</code>, when the underlying handle of both
                         Socket instances are identical, <code>false</code> otherwise.
         */
        inline bool SAL_CALL operator==( const Socket& rSocket ) const ;

        /**
           @return <code>true</code>, when the underlying handle of both
                         Socket instances are identical, <code>false</code> otherwise.
         */
        inline bool SAL_CALL operator==( const oslSocket socketHandle ) const;

        /** Closes a definite or both directions of the bidirectional stream.

           @param Direction
           @see osl_shutdownSocket()
         */
        inline void SAL_CALL shutdown( oslSocketDirection Direction = osl_Socket_DirReadWrite );

        /** Closes a socket.
            Note that closing a socket is identical to shutdown( osl_Socket_DirReadWrite ),
            as the operating system distinguish both cases, both functions or offered in this API.
            @see osl_closeSocket()
         */
        inline void SAL_CALL close();

        /** Retrieves the address of the local interface of this socket.
            @param Addr [out] receives the address.
            @see osl_getLocalAddrOfSocket()
        */
        inline void SAL_CALL getLocalAddr( SocketAddr &Addr ) const;

        /** Get the local port of the socket. Usually used after bind().
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
        inline void SAL_CALL getPeerAddr( SocketAddr & Addr) const;

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
        inline bool SAL_CALL bind(const SocketAddr& LocalInterface);

        /** Checks if read operations will block.

            You can specify a timeout-value in seconds/nanoseconds that denotes
            how the operation will block if the Socket is not ready.
            @return <code>true</code> if read operations (recv, recvFrom, accept) on the Socket
            will NOT block; <code>false</code> if it would block or if an error occurred.

            @param pTimeout if 0, the operation will block without a timeout. Otherwise
            the specified amount of time.
        */
        inline bool SAL_CALL isRecvReady(const TimeValue *pTimeout = NULL) const;

        /** Checks if send operations will block.

            You can specify a timeout-value in seconds/nanoseconds that denotes
            how the operation will block if the Socket is not ready.
            @return <code>true</code> if send operations (send, sendTo) on the Socket
            will NOT block; <code>false</code> if it would block or if an error occurred.

            @param pTimeout if 0, the operation will block without a timeout. Otherwise
            the specified amount of time.
        */
        inline bool SAL_CALL isSendReady(const TimeValue *pTimeout = NULL) const;


        /** Checks if a request for out-of-band data will block.

            You can specify a timeout-value in seconds/nanoseconds that denotes
            how the operation will block if the Socket has no pending OOB data.

            @return <code>true</code> if OOB-request operations (recv with appropriate flags)
            on the Socket will NOT block; <code>false</code> if it would block or if
            an error occurred.

            @param pTimeout if 0, the operation will block without a timeout. Otherwise
            the specified amount of time.
        */
        inline bool SAL_CALL isExceptionPending(const TimeValue *pTimeout = NULL) const;


        /** Queries the socket for its type.

            @retval osl_Socket_TypeStream
            @retval osl_Socket_TypeDgram
            @retval osl_Socket_TypeRaw
            @retval osl_Socket_TypeRdm
            @retval osl_Socket_TypeSeqPacket
            @retval osl_invalid_SocketType if an error occurred
        */
        inline oslSocketType    SAL_CALL getType() const;

        /** Retrieves option-attributes associated with the socket.
            @param Option The attribute to query.
            Valid values (depending on the Level) are:
            <ul>
            <li> <code>osl_Socket_Option_Debug</code><br>
                 (sal_Bool) Socket debug flag 1 = enabled, 0 = disabled.

            <li> <code>osl_Socket_OptionAcceptConn</code><br>
            <li> <code>osl_Socket_OptionReuseAddr</code><br>
              (sal_Bool) Allows the socket to be bound to an address that is
              already in use.
              1 = multiple bound allowed, 0 = no multiple bounds allowed

            <li><code>osl_Socket_OptionKeepAlive</code><br>
               (sal_Bool) Keepalive packets are sent by the underlying socket.
               1 = enabled, 0 = disabled

            <li><code>osl_Socket_OptionDontRoute</code><br>
               (sal_Bool) Do not route: send directly to interface.
                  1 = do not route , 0 = routing possible

            <li><code>osl_Socket_OptionBroadcast</code><br>
               (sal_Bool) Transmission of broadcast messages are allowed on the socket.
               1 = transmission allowed, 0 = transmission disallowed

            <li><code>osl_Socket_OptionUseLoopback</code><br>

            <li><code>osl_Socket_OptionLinger</code><br>
               (linger) Linger on close if unsent data is present.
               linger has two members: l_onoff, l_linger
               l_onoff = 0 is off, l_onoff > 0 and l_linger= timeout in seconds.

            <li><code>osl_Socket_OptionOOBinLine</code><br>


            <li><code>osl_Socket_OptionSndBuf</code><br>
              (sal_Int32) Size of the send buffer in bytes. Data is sent after
              SndTimeo or when the buffer is full. This allows faster writing
              to the socket.

            <li><code>osl_Socket_OptionRcvBuf</code><br>
              (sal_Int32) Size of the receive buffer in bytes. Data is sent after
              SndTimeo or when the buffer is full. This allows faster writing
              to the socket and larger packet sizes.

            <li><code>osl_Socket_OptionSndLowat</code><br>

            <li><code>osl_Socket_OptionRcvLowat</code><br>

            <li><code>osl_Socket_OptionSndTimeo</code><br>
              (sal_Int32) Data is sent after this timeout. This allows gathering
              of data to send larger packages but increases latency times.

            <li><code>osl_Socket_OptionRcvTimeo</code><br>

            <li><code>osl_Socket_OptionError</code><br>
            <li><code>osl_Socket_OptionType</code><br>

            <li><code>osl_Socket_OptionTcpNoDelay</code><br>
              Disables the Nagle algorithm for send coalescing. (Do not
              collect data until a packet is full, instead send immediately.
              This increases network traffic but might improve latency-times.)
              1 = disables the algorithm, 0 = keeps it enabled.
            </ul>

              If not above mentioned otherwise, the options are only valid for
              level <code>osl_Socket_LevelSocket</code>.
            @param pBuffer The Buffer will be filled with the attribute.

            @param BufferLen The size of pBuffer.

            @param Level The option level.

            Valid values are:
            <ul>
            <li><code>osl_Socket_LevelSocket</code> : Socket Level
            <li><code>osl_Socket_LevelTcp</code>    : Level of Transmission Control Protocol
            </ul>
            @return The size of the attribute copied into pBuffer or -1 if an error
            occurred.
        */
        inline sal_Int32 SAL_CALL getOption(
            oslSocketOption Option,
            void* pBuffer,
            sal_uInt32 BufferLen,
            oslSocketOptionLevel Level= osl_Socket_LevelSocket) const;

        /** Sets the sockets attributes.

            @param Option denotes the option to modify.
            Valid values (depending on the Level) are:
            <ul>
            <li> osl_Socket_Option_Debug
            <li> osl_Socket_OptionAcceptConn
            <li> osl_Socket_OptionReuseAddr
            <li> osl_Socket_OptionKeepAlive
            <li> osl_Socket_OptionDontRoute
            <li> osl_Socket_OptionBroadcast
            <li> osl_Socket_OptionUseLoopback
            <li> osl_Socket_OptionLinger
            <li> osl_Socket_OptionOOBinLine
            <li> osl_Socket_OptionSndBuf
            <li> osl_Socket_OptionRcvBuf
            <li> osl_Socket_OptionSndLowat
            <li> osl_Socket_OptionRcvLowat
            <li> osl_Socket_OptionSndTimeo
            <li> osl_Socket_OptionRcvTimeo
            <li> osl_Socket_OptionError
            <li> osl_Socket_OptionType
            <li> osl_Socket_OptionTcpNoDelay
            </ul>

            If not above mentioned otherwise, the options are only valid for
            level osl_Socket_LevelSocket.

            @param pBuffer Pointer to a Buffer which contains the attribute-value.

            @param BufferLen contains the length of the Buffer.

            @param Level selects the level for which an option should be changed.
            Valid values are:
            <ul>
            <li> osl_Socket_evel_Socket : Socket Level
            <li> osl_Socket_Level_Tcp   : Level of Transmission Control Protocol
            </ul>

            @return True if the option could be changed.
        */
        inline bool SAL_CALL setOption( oslSocketOption Option,
                                            void* pBuffer,
                                            sal_uInt32 BufferLen,
                                            oslSocketOptionLevel Level= osl_Socket_LevelSocket ) const;

        /** Convenience function for setting sal_Bool and sal_Int32 option values.
            @see setOption()
         */
        inline bool setOption( oslSocketOption option, sal_Int32 nValue  );

        /** Convenience function for retrieving sal_Bool and sal_Int32 option values.
            @see setOption()
         */
        inline sal_Int32 getOption( oslSocketOption option ) const;

        /** Enables/disables non-blocking mode of the socket.
            @param bNonBlockingMode If <code>true</code>, blocking mode will be switched off
                   If <code>false</code>, the socket will become a blocking
                   socket (which is the default behaviour of a socket).
            @return <code>true</code> if mode could be set.
        */
        inline bool SAL_CALL enableNonBlockingMode( bool bNonBlockingMode);

        /** Query blocking mode of the socket.
            @return <code>true</code> if non-blocking mode is set.
        */
        inline bool SAL_CALL isNonBlockingMode() const;


        /** clears the error status
        */
        inline void SAL_CALL clearError() const;

        /** returns a constant describing the last error for the socket system.

            @return osl_Socket_E_NONE if no error occurred, invalid_SocketError if
            an unknown (unmapped) error occurred, otherwise an enum describing the
            error.
            @see osl_getLastSocketError()
        */
        inline oslSocketError getError() const;

        /** Builds a string with the last error-message for the socket.
         */
        inline ::rtl::OUString getErrorAsString( ) const;

        /** Returns the underlying handle unacquired  (The caller must acquire it to keep it).
         */
        inline oslSocket getHandle() const;
    };


    class StreamSocket : public Socket
    {
    public:
        /** Creates a socket.
            @param Family the Family of the socket (Inet by default)
            @param Protocol the Protocon of the socket (IP by default)
            @param Type For some protocols it might be desirable to
            use a different type than <code>osl_Socket_TypeStream</code>
            (like <code>osl_Socket_TypeSeqPacket</code>).
            Therefore this parameter is not hidden.
        */
          inline StreamSocket(oslAddrFamily Family = osl_Socket_FamilyInet,
                            oslProtocol Protocol = osl_Socket_ProtocolIp,
                            oslSocketType   Type = osl_Socket_TypeStream);

        inline StreamSocket( oslSocket Socket , __sal_NoAcquire noacquire );

        inline StreamSocket( oslSocket Socket );

        /** Retrieves n bytes from the stream and copies them into pBuffer.
            The method avoids incomplete reads due to packet boundaries and is thus
            blocking.
            @param pBuffer receives the read data. pBuffer must be large enough
            to hold n bytes.
            @param n the number of bytes to read.
            @return the number of read bytes. The number will only be smaller than
            n if an exceptional condition (e.g. connection closed) occurs.
        */
        inline sal_Int32 SAL_CALL read(void* pBuffer, sal_uInt32 n);

        /** Writes n bytes from pBuffer to the stream. The method avoids
            incomplete writes due to packet boundaries and is thus blocking.
            @param pBuffer contains the data to be written.
            @param n the number of bytes to write.
            @return the number of written bytes. The number will only be smaller than
            n if an exceptional condition (e.g. connection closed) occurs.
        */
        inline sal_Int32 SAL_CALL write(const void* pBuffer, sal_uInt32 n);


        /** Tries to receive BytesToRead data from the connected socket,

            @param[out] pBuffer Points to a buffer that will be filled with the received
            data. pBuffer must have at least have a size of BytesToRead.
            @param[in] BytesToRead The number of bytes to read.
            @param[in] flags Modifier for the call. Valid values are:

            <ul>
            <li><code>osl_Socket_MsgNormal</code>
            <li><code>osl_Socket_MsgOOB</code>
            <li><code>osl_Socket_MsgPeek</code>
            <li><code>osl_Socket_MsgDontRoute</code>
            <li><code>osl_Socket_MsgMaxIOVLen</code>
            </ul>
            @return the number of received bytes, which may be less than BytesToRead.
        */
        inline sal_Int32 SAL_CALL recv(void* pBuffer,
                                       sal_uInt32 BytesToRead,
                                       oslSocketMsgFlag flags= osl_Socket_MsgNormal);

        /** Tries to send BytesToSend data to the connected socket.

            @param pBuffer [in] Points to a buffer that contains the send-data.
            @param BytesToSend [in] The number of bytes to send. pBuffer must have at least
            this size.
            @param Flag [in] Modifier for the call. Valid values are:
            <ul>
            <li><code>osl_Socket_MsgNormal</code>
            <li><code>osl_Socket_MsgOOB</code>
            <li><code>osl_Socket_MsgPeek</code>
            <li><code>osl_Socket_MsgDontRoute</code>
            <li><code>osl_Socket_MsgMaxIOVLen</code>
            </ul>

            @return the number of transferred bytes. It may be less than BytesToSend.
        */
        sal_Int32 SAL_CALL send(const void* pBuffer,
                                sal_uInt32 BytesToSend,
                                oslSocketMsgFlag= osl_Socket_MsgNormal);
    };

    class ConnectorSocket : public StreamSocket
    {
    public:
        /** Creates a socket that can connect to a (remote) host.
            @param Family the Family of the socket (Inet by default)
            @param Protocol the Protocon of the socket (IP by default)
            @param Type For some protocols it might be desirable to
            use a different type than sock_stream <code>osl_Socket_TypeSeqPacket</code>
            (like <code>osl_Socket_TypeSeqPacket</code>).
            Therefore we do not hide this parameter here.
        */
          ConnectorSocket(oslAddrFamily Family = osl_Socket_FamilyInet,
                        oslProtocol Protocol = osl_Socket_ProtocolIp,
                        oslSocketType   Type = osl_Socket_TypeStream);


        /** Connects the socket to a (remote) host.
            @param TargetHost The address of the target.
            @param pTimeout The timeout for blocking. If 0, a default system dependent timeout
                            us used.
            @return <code> osl_Socket_Ok</code> if connected successfully,
            <code>osl_Socket_TimedOut</code> on timeout,
            <code>osl_Socket_Interrupted</code> if unblocked forcefully (by osl::Socket::close()),
            <code>osl_Socket_Error</code> if connect failed.
        */
        oslSocketResult SAL_CALL connect(const SocketAddr& TargetHost, const TimeValue* pTimeout = NULL);
    };

    /** Allows to accept socket connections.
     */
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
            @return <code>true</code> if call was successful.
        */
        inline bool SAL_CALL listen(sal_Int32 MaxPendingConnections= -1);

        /** Accepts incoming connections on the socket. You must
            precede this call with osl::Socket::bind() and listen().
            @param Connection receives the incoming connection.
            @return <code>osl_Socket_Ok</code>, if a connection has been accepted,
            <code>osl_Socket_TimedOut</code>, if m_RecvTimeout milliseconds passed without connect,
            <code>osl_Socket_Error</code> on errors.
        */
        inline oslSocketResult SAL_CALL acceptConnection( StreamSocket& Connection);

        /** Accepts incoming connections on the socket. You must
            precede this call with osl::Socket::bind() and listen().
            @param PeerAddr receives the address of the connecting entity
            (your communication partner).
            @param Connection receives the incoming connection.
            @return <code>osl_Socket_Ok</code>, if a connection has been accepted,
            <code>osl_Socket_TimedOut</code>, if m_RecvTimeout milliseconds passed without connect,
            <code>osl_Socket_Error</code> on errors.
        */
        inline oslSocketResult SAL_CALL acceptConnection( StreamSocket& Connection, SocketAddr & PeerAddr);
    };


    /** A connectionless socket to send and receive datagrams.
     */
    class DatagramSocket : public Socket
    {
    public:

        /** Creates a datagram socket.
            @param Family the Family of the socket (Inet by default)
            @param Protocol the Protocon of the socket (IP by default)
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
            <li><code>osl_Socket_MsgNormal</code>
            <li><code>osl_Socket_MsgOOB</code>
            <li><code>osl_Socket_MsgPeek</code>
            <li><code>osl_Socket_MsgDontRoute</code>
            <li><code>osl_Socket_MsgMaxIOVLen</code>
            </ul>

            @return the number of received bytes.
        */
        inline sal_Int32 SAL_CALL recvFrom(void*  pBuffer,
                                           sal_uInt32 BufferSize,
                                           SocketAddr* pSenderAddr= NULL,
                                           oslSocketMsgFlag Flag= osl_Socket_MsgNormal);

        /** Tries to send one datagram with BytesToSend size to the given ReceiverAddr.
            Since there is only send one packet, the function doesn't care about
            packet boundaries.

            @param ReceiverAddr [in] A SocketAddr that contains
            the destination address for this send.

            @param pBuffer [in] Points to a buffer that contains the send-data.
            @param BufferSize [in] The number of bytes to send. pBuffer must have at least
            this size.
            @param Flag [in] Modifier for the call. Valid values are:

            <ul>
            <li><code>osl_Socket_MsgNormal</code>
            <li><code>osl_Socket_MsgOOB</code>
            <li><code>osl_Socket_MsgPeek</code>
            <li><code>osl_Socket_MsgDontRoute</code>
            <li><code>osl_Socket_MsgMaxIOVLen</code>
            </ul>

            @return the number of transferred bytes.
        */
        inline sal_Int32    SAL_CALL sendTo( const SocketAddr& ReceiverAddr,
                                             const void* pBuffer,
                                             sal_uInt32 BufferSize,
                                             oslSocketMsgFlag Flag= osl_Socket_MsgNormal);
    };

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
