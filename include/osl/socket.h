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

#ifndef INCLUDED_OSL_SOCKET_H
#define INCLUDED_OSL_SOCKET_H

#include "rtl/ustring.h"
#include "osl/time.h"

#ifdef __cplusplus
extern "C" {
#endif

/* error returns */
#define OSL_INADDR_NONE             0xffffffff
#define OSL_INVALID_PORT (-1)

/**
    Opaque datatype SocketAddr.
*/
typedef struct oslSocketAddrImpl * oslSocketAddr;

/**
    Represents the address-family of a socket
*/
typedef enum {
    osl_Socket_FamilyInet,          /*!< IP (AF_INET)                           */
    osl_Socket_FamilyIpx,           /*!< Novell IPX/SPX (AF_IPX)                */
    osl_Socket_FamilyInvalid,       /*!< always last entry in enum!             */
    osl_Socket_Family_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
} oslAddrFamily;

/**
    represent a specific protocol within a address-family
*/
typedef enum {
    osl_Socket_ProtocolIp,          /*!< for all af_inet                        */
    osl_Socket_ProtocolIpx,         /*!< af_ipx datagram sockets (IPX)          */
    osl_Socket_ProtocolSpx,         /*!< af_ipx seqpacket or stream for SPX     */
    osl_Socket_ProtocolSpxII,       /*!< af_ipx seqpacket or stream for SPX II  */
    osl_Socket_ProtocolInvalid,     /*!< always last entry in enum              */
    osl_Socket_Protocol_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
} oslProtocol;

/**
    Represents the type of a socket
*/
typedef enum {
    osl_Socket_TypeStream,          /*!< stream socket                          */
    osl_Socket_TypeDgram,           /*!< datagram socket                        */
    osl_Socket_TypeRaw,             /*!< raw socket                             */
    osl_Socket_TypeRdm,             /*!< connectionless, message-oriented,
                                         reliably delivered message (RDM)
                                         sockets                                */
    osl_Socket_TypeSeqPacket,       /*!< connection-oriented and reliable
                                         two-way transport of ordered byte
                                         streams                                */
    osl_Socket_TypeInvalid,         /*!< always last entry in enum              */
    osl_Socket_Type_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
} oslSocketType;


/**
    Represents socket-options
*/
typedef enum {
    osl_Socket_OptionDebug,         /*!< record debugging info                  */
    osl_Socket_OptionAcceptConn,    /*!< listen for connection                  */
    osl_Socket_OptionReuseAddr,     /*!< bind to address already in use         */
    osl_Socket_OptionKeepAlive,     /*!< use keep-alive                         */
    osl_Socket_OptionDontRoute,     /*!< do not route packet, send direct to
                                         interface addresses                    */
    osl_Socket_OptionBroadcast,     /*!< send broadcast message                 */
    osl_Socket_OptionUseLoopback,   /*!< socket receives copy of everything
                                         sent on the socket                     */
    osl_Socket_OptionLinger,        /*!< don't immediately close - "linger"
                                         a while to allow for graceful
                                         connection closure                     */
    osl_Socket_OptionOOBinLine,     /*!< out-of-band (OOB) data placed in
                                         normal input queue (i.e. OOB inline)   */
    osl_Socket_OptionSndBuf,        /*!< send buffer                            */
    osl_Socket_OptionRcvBuf,        /*!< receive buffer                         */
    osl_Socket_OptionSndLowat,      /*!< send "low-water" mark - amount of
                                         available space in send buffer for
                                         select() to return "writable"          */
    osl_Socket_OptionRcvLowat,      /*!< receive "low-water" mark - amount of
                                         available space in receive buffer
                                         for select() to receive "readable"     */
    osl_Socket_OptionSndTimeo,      /*!< send timeout                           */
    osl_Socket_OptionRcvTimeo,      /*!< receive timeout                        */
    osl_Socket_OptionError,         /*!< socket error                           */
    osl_Socket_OptionType,          /*!< returns socket type (e.g. datagram,
                                         stream).                               */
    osl_Socket_OptionTcpNoDelay,    /*!< disable TCP Nagle algorithm            */
    osl_Socket_OptionInvalid,       /*!< always last entry in enum              */
    osl_Socket_Option_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
} oslSocketOption;

/**
    Represents the different socket-option levels
*/
typedef enum  {
    osl_Socket_LevelSocket,
    osl_Socket_LevelTcp,
    osl_Socket_LevelInvalid,        /*!< always last entry in enum              */
    osl_Socket_Level_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
} oslSocketOptionLevel;

/**
    Represents flags to be used with send/recv-calls.
*/
typedef enum {
    osl_Socket_MsgNormal,
    osl_Socket_MsgOOB,
    osl_Socket_MsgPeek,
    osl_Socket_MsgDontRoute,
    osl_Socket_MsgMaxIOVLen,
    osl_Socket_MsgInvalid,          /*!< always last entry in enum              */
    osl_Socket_Msg_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
} oslSocketMsgFlag;

/**
    Used by shutdown to denote which end of the socket to "close".
*/
typedef enum {
    osl_Socket_DirRead,
    osl_Socket_DirWrite,
    osl_Socket_DirReadWrite,
    osl_Socket_DirInvalid,          /*!< always last entry in enum              */
    osl_Socket_Dir_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
} oslSocketDirection;

/** Describes the various error socket error conditions, which may
    occur */
typedef enum {
    osl_Socket_E_None,              /*!< no error                               */
    osl_Socket_E_NotSocket,         /*!< Socket operation on non-socket         */
    osl_Socket_E_DestAddrReq,       /*!< Destination address required           */
    osl_Socket_E_MsgSize,           /*!< Message too long                       */
    osl_Socket_E_Prototype,         /*!< Protocol wrong type for socket         */
    osl_Socket_E_NoProtocol,        /*!< Protocol not available                 */
    osl_Socket_E_ProtocolNoSupport, /*!< Protocol not supported                 */
    osl_Socket_E_TypeNoSupport,     /*!< Socket type not supported              */
    osl_Socket_E_OpNotSupport,      /*!< Operation not supported on socket      */
    osl_Socket_E_PfNoSupport,       /*!< Protocol family not supported          */
    osl_Socket_E_AfNoSupport,       /*!< Address family not supported by        */
                                    /*!< protocol family                        */
    osl_Socket_E_AddrInUse,         /*!< Address already in use                 */
    osl_Socket_E_AddrNotAvail,      /*!< Can't assign requested address         */
    osl_Socket_E_NetDown,           /*!< Network is down                        */
    osl_Socket_E_NetUnreachable,    /*!< Network is unreachable                 */
    osl_Socket_E_NetReset,          /*!< Network dropped connection because
                                       of reset                                 */
    osl_Socket_E_ConnAborted,       /*!< Software caused connection abort       */
    osl_Socket_E_ConnReset,         /*!< Connection reset by peer               */
    osl_Socket_E_NoBufferSpace,     /*!< No buffer space available              */
    osl_Socket_E_IsConnected,       /*!< Socket is already connected            */
    osl_Socket_E_NotConnected,      /*!< Socket is not connected                */
    osl_Socket_E_Shutdown,          /*!< Can't send after socket shutdown       */
    osl_Socket_E_TooManyRefs,       /*!< Too many references: can't splice      */
    osl_Socket_E_TimedOut,          /*!< Connection timed out                   */
    osl_Socket_E_ConnRefused,       /*!< Connection refused                     */
    osl_Socket_E_HostDown,          /*!< Host is down                           */
    osl_Socket_E_HostUnreachable,   /*!< No route to host                       */
    osl_Socket_E_WouldBlock,        /*!< call would block on non-blocking socket */
    osl_Socket_E_Already,           /*!< operation already in progress          */
    osl_Socket_E_InProgress,        /*!< operation now in progress              */
    osl_Socket_E_InvalidError,      /*!< unmapped error: always last entry in enum */
    osl_Socket_E_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
} oslSocketError;

/** Common return codes of socket related functions.
 */
typedef enum {
    osl_Socket_Ok,                  /*!< successful completion                  */
    osl_Socket_Error,               /*!< error occurred, check
                                         osl_getLastSocketError() for details   */
    osl_Socket_TimedOut,            /*!< blocking operation timed out           */
    osl_Socket_Interrupted,         /*!< blocking operation was interrupted     */
    osl_Socket_InProgress,          /*!< nonblocking operation is in progress   */
    osl_Socket_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
} oslSocketResult;

typedef sal_uInt8 oslSocketIpxNetNumber[4];
typedef sal_uInt8 oslSocketIpxNodeNumber[6];

/**@} end section types
*/

/**@{ begin section oslSocket
*/

typedef struct oslSocketImpl * oslSocket;

/** Create a socket of the specified Family and Type. The semantic of
    the Protocol parameter depends on the given family and type.

    @returns 0 if socket could not be created, otherwise you get a handle
             to the allocated socket-datastructure.
*/
SAL_DLLPUBLIC oslSocket SAL_CALL osl_createSocket(
                            oslAddrFamily Family,
                            oslSocketType Type,
                            oslProtocol Protocol);

/** increases the refcount of the socket handle by one
 */
SAL_DLLPUBLIC void SAL_CALL osl_acquireSocket(oslSocket Socket);

/** decreases the refcount of the socket handle by one.

    If the refcount drops to zero, the underlying socket handle
    is destroyed and becomes invalid.
 */
SAL_DLLPUBLIC void SAL_CALL osl_releaseSocket(oslSocket Socket);

/** Retrieves the Address of the local end of the socket.
    Note that a socket must be bound or connected before
    a valid address can be returned.

    @returns 0 if socket-address could not be created, otherwise you get
             the created Socket-Address.
*/
SAL_DLLPUBLIC oslSocketAddr SAL_CALL osl_getLocalAddrOfSocket(oslSocket Socket);

/** Retrieves the Address of the remote end of the socket.
    Note that a socket must be connected before
    a valid address can be returned.
    @retval 0 if socket-address could not be created, otherwise you get
    the created Socket-Address.
*/
SAL_DLLPUBLIC oslSocketAddr SAL_CALL osl_getPeerAddrOfSocket(oslSocket Socket);

/** Binds the given address to the socket.
    @param[in] Socket
    @param[in] Addr
    @retval sal_False if the bind failed
    @retval sal_True if bind is successful
    @see osl_getLastSocketError()
*/
SAL_DLLPUBLIC sal_Bool SAL_CALL osl_bindAddrToSocket(
                           oslSocket Socket,
                           oslSocketAddr Addr);

/** Connects the socket to the given address.

    @param[in] Socket a bound socket.
    @param[in] Addr the peer address.
    @param pTimeout Timeout value or NULL for blocking.

    @retval osl_Socket_Ok on successful connection,
    @retval osl_Socket_TimedOut if operation timed out,
    @retval osl_Socket_Interrupted if operation was interrupted
    @retval osl_Socket_Error if the connection failed.
*/
SAL_DLLPUBLIC oslSocketResult SAL_CALL osl_connectSocketTo(
                              oslSocket Socket,
                              oslSocketAddr Addr,
                              const TimeValue* pTimeout);


/** Prepares the socket to act as an acceptor of incoming connections.
    You should call "listen" before you use "accept".
    @param[in] Socket The socket to listen on.
    @param[in] MaxPendingConnections denotes the length of the queue of
    pending connections for this socket. If MaxPendingConnections is
    -1, the systems default value will be used (Usually 5).
    @retval sal_False if the listen failed.
*/
SAL_DLLPUBLIC sal_Bool SAL_CALL osl_listenOnSocket(
                            oslSocket Socket,
                            sal_Int32 MaxPendingConnections);


/** Waits for an ingoing connection on the socket.
    This call blocks if there is no incoming connection present.
    @param[in] Socket The socket to accept the connection on.
    @param[in] pAddr if pAddr is != 0, the peers address is returned.
    @retval 0 if the accept-call failed, otherwise you get a socket
    representing the new connection.
*/
SAL_DLLPUBLIC oslSocket SAL_CALL osl_acceptConnectionOnSocket(
                            oslSocket Socket,
                            oslSocketAddr* pAddr);

/** Tries to receive BytesToRead data from the connected socket,
    if no error occurs. Note that incomplete recvs due to
    packet boundaries may occur.

    @param[in] Socket A connected socket to be used to listen on.
    @param[out] pBuffer Points to a buffer that will be filled with the received
    data.
    @param[in] BytesToRead The number of bytes to read. pBuffer must have at least
    this size.
    @param[in] Flag Modifier for the call. Valid values are:
    osl_Socket_MsgNormal
    osl_Socket_MsgOOB
    osl_Socket_MsgPeek
    osl_Socket_MsgDontRoute
    osl_Socket_MsgMaxIOVLen

    @return the number of received bytes.
*/
SAL_DLLPUBLIC sal_Int32 SAL_CALL osl_receiveSocket(
                            oslSocket Socket,
                            void* pBuffer,
                            sal_uInt32 BytesToRead,
                            oslSocketMsgFlag Flag);

/** Tries to receives BufferSize data from the (usually unconnected)
    (datagram-)socket, if no error occurs.

    @param[in] Socket A bound socket to be used to listen for a datagram.
    @param[out] SenderAddr A pointer to a created oslSocketAddr handle
    or to a null handle. After the call, it will contain the constructed
    oslSocketAddr of the datagrams sender. If pSenderAddr itself is 0,
    it is ignored.
    @param[out] pBuffer Points to a buffer that will be filled with the received
    datagram.
    @param[in] BufferSize The size of pBuffer.
    @param[in] Flag Modifier for the call. Valid values are:
    osl_Socket_MsgNormal
    osl_Socket_MsgOOB
    osl_Socket_MsgPeek
    osl_Socket_MsgDontRoute
    osl_Socket_MsgMaxIOVLen

    @return the number of received bytes.
*/
SAL_DLLPUBLIC sal_Int32 SAL_CALL osl_receiveFromSocket(
                            oslSocket Socket,
                            oslSocketAddr SenderAddr,
                            void* pBuffer,
                            sal_uInt32 BufferSize,
                            oslSocketMsgFlag Flag);

/** Tries to send BytesToSend data from the connected socket,
    if no error occurs.

    @param[in] Socket A connected socket.
    @param[in] pBuffer Points to a buffer that contains the send-data.
    @param[in] BytesToSend The number of bytes to send. pBuffer must have at least
    this size.
    @param[in] Flag Modifier for the call. Valid values are:
    @li osl_Socket_MsgNormal
    @li osl_Socket_MsgOOB
    @li osl_Socket_MsgPeek
    @li osl_Socket_MsgDontRoute
    @li osl_Socket_MsgMaxIOVLen

    @return the number of transferred bytes.
*/
SAL_DLLPUBLIC sal_Int32 SAL_CALL osl_sendSocket(
                            oslSocket Socket,
                            const void* pBuffer,
                            sal_uInt32 BytesToSend,
                            oslSocketMsgFlag Flag);

/** Tries to send one datagram with BytesToSend data to the given ReceiverAddr
    via the (implicitly unconnected) datagram-socket.
    Since there is only sent one packet, the function sends the data always complete
    even with incomplete packet boundaries.

    @param[in] Socket A bound or unbound socket. Socket will be bound
    after a successful call.

    @param[in] ReceiverAddr An initialized oslSocketAddress that contains
    the destination address for this send.

    @param[in] pBuffer Points to a buffer that contains the send-data.
    @param[in] BytesToSend The number of bytes to send. pBuffer must have at least
    this size.
    @param[in] Flag
    @parblock
        Modifier for the call. Valid values are:
        @li osl_Socket_MsgNormal
        @li osl_Socket_MsgOOB
        @li osl_Socket_MsgPeek
        @li osl_Socket_MsgDontRoute
        @li osl_Socket_MsgMaxIOVLen
    @endparblock

    @return the number of transferred bytes.
*/
SAL_DLLPUBLIC sal_Int32 SAL_CALL osl_sendToSocket(
                            oslSocket Socket,
                            oslSocketAddr ReceiverAddr,
                            const void* pBuffer,
                            sal_uInt32 BytesToSend,
                            oslSocketMsgFlag Flag);

/** Checks if read operations will block.

    You can specify a timeout-value in seconds/microseconds that denotes
    how long the operation will block if the Socket is not ready.

    @param Socket the Socket to perform the operation on.
    @param pTimeout if NULL, the operation will block without a timeout.

    @retval sal_True if read operations (recv, recvFrom, accept) on the Socket
        will NOT block;
    @retval sal_False if it would block or if an error occurred.
*/
SAL_DLLPUBLIC sal_Bool SAL_CALL osl_isReceiveReady(
                            oslSocket Socket,
                            const TimeValue* pTimeout);

/** Checks if send operations will block.
    You can specify a timeout-value in seconds/microseconds that denotes
    how long the operation will block if the Socket is not ready.

    @param Socket the Socket to perform the operation on.
    @param pTimeout if NULL, the operation will block without a timeout. Otherwise
        the time define by timeout value.

    @retval sal_True if send operations (send, sendTo) on the Socket
    will NOT block
    @retval sal_False if it would block or if an error occurred.
*/
SAL_DLLPUBLIC sal_Bool SAL_CALL osl_isSendReady(
                            oslSocket Socket,
                            const TimeValue* pTimeout);

/** Checks if a request for out-of-band data will block.
    You can specify a timeout-value in seconds/microseconds that denotes
    how long the operation will block if the Socket has no pending OOB data.

    @param Socket the Socket to perform the operation on.
    @param pTimeout if NULL, the operation will block without a timeout.

    @retval sal_True if OOB-request operations (recv with appropriate flags)
        on the Socket will NOT block
    @retval sal_False if it would block or if an error occurred.
*/
SAL_DLLPUBLIC sal_Bool SAL_CALL osl_isExceptionPending(
                            oslSocket Socket,
                            const TimeValue* pTimeout);

/** Shuts down communication on a connected socket.
    @param[in] Socket the Socket to perform the operation on.
    @param[in] Direction
    @parblock
        Direction denotes which end of the socket should be closed:
        @li osl_Socket_DirRead - closes read operations.
        @li osl_Socket_DirReadWrite - closes write operations.
        @li osl_Socket_DirWrite - closes read and write operations.
    @endparblock

    @retval sal_True if the socket could be closed down.
*/
SAL_DLLPUBLIC sal_Bool SAL_CALL osl_shutdownSocket(
                            oslSocket Socket,
                            oslSocketDirection Direction);

/** Retrieves attributes associated with the socket.

    @param Socket is the socket to query.
    @param Level
    @parblock
        Selects the level for which an option should be queried.
        Valid values are:
        @li osl_sol_socket - Socket Level
        @li osl_sol_tcp - Level of Transmission Control Protocol
    @endparblock

    @param Option
    @parblock
        Denotes the option to query.  Valid values (depending on the Level) are:
        @li osl_Socket_Option_Debug - (sal_Bool) Socket debug flag 1 = enabled, 0 = disabled.
        @li osl_Socket_OptionAcceptConn
        @li osl_Socket_OptionReuseAddr - (sal_Bool) Allows the socket to be bound to an address that is
            already in use. 1 = multiple bound allowed, 0 = no multiple bounds allowed
        @li osl_Socket_OptionKeepAlive (sal_Bool) Keepalive packets are sent by the underlying socket.
            1 = enabled, 0 = disabled
        @li osl_Socket_OptionDontRoute - (sal_Bool) Do not route: send directly to interface.
            1 = do not route , 0 = routing possible
        @li osl_Socket_OptionBroadcast - (sal_Bool) Transmission of broadcast messages are allowed on the socket.
            1 = transmission allowed, 0 = transmission disallowed
        @li osl_Socket_OptionUseLoopback
        @li osl_Socket_OptionLinger (sal_Int32) Linger on close if unsent data is present.
            0 = linger is off, > 0  = timeout in seconds.
        @li osl_Socket_OptionOOBinLine
        @li osl_Socket_OptionSndBuf (sal_Int32) Size of the send buffer in bytes. Data is sent after
            SndTimeo or when the buffer is full. This allows faster writing to the socket.
        @li osl_Socket_OptionRcvBuf (sal_Int32) Size of the receive buffer in bytes. Data is sent after
        SndTimeo or when the buffer is full. This allows faster writing to the socket and larger packet sizes.
        @li osl_Socket_OptionSndLowat
        @li osl_Socket_OptionRcvLowat
        @li osl_Socket_OptionSndTimeo (sal_Int32) Data is sent after this timeout. This allows gathering
            of data to send larger packages but increases latency times.
        @li osl_Socket_OptionRcvTimeo
        @li osl_Socket_OptionError
        @li osl_Socket_OptionType
        @li osl_Socket_OptionTcpNoDelay Disables the Nagle algorithm for send coalescing. (Do not
            collect data until a packet is full, instead send immediately.  This increases network traffic
            but might improve latency-times.)
            1 = disables the algorithm, 0 = keeps it enabled.

        If not above mentioned otherwise, the options are only valid for level osl_Socket_LevelSocket.
    @endparblock
    @param pBuffer Pointer to a buffer large enough to take the desired attribute-value.
    @param BufferLen contains the length of the Buffer.

    @return -1 if an error occurred or else the size of the data copied into pBuffer.

    @see osl_setSocketOption()
*/
SAL_DLLPUBLIC sal_Int32 SAL_CALL osl_getSocketOption(
                            oslSocket Socket,
                            oslSocketOptionLevel Level,
                            oslSocketOption Option,
                            void* pBuffer,
                            sal_uInt32 BufferLen);

/** Sets the sockets attributes.

    @param Socket is the socket to modify.
    @param Level
    @parblock
        selects the level for which an option should be changed.
        Valid values are:
        @li osl_sol_socket - Socket Level
        @li osl_sol_tcp - Level of Transmission Control Protocol
    @endparblock
    @param Option denotes the option to modify. See osl_setSocketOption() for more
        details.
    @param pBuffer Pointer to a Buffer which contains the attribute-value.
    @param BufferLen contains the length of the Buffer.

    @retval True if the option could be changed.
*/
SAL_DLLPUBLIC sal_Bool SAL_CALL osl_setSocketOption(
                            oslSocket Socket,
                            oslSocketOptionLevel Level,
                            oslSocketOption Option,
                            void* pBuffer,
                            sal_uInt32 BufferLen);

/** Enables/disables non-blocking-mode of the socket.

    @param Socket Change mode for this socket.
    @param On sal_True enables non-blocking mode, sal_False disables non-blocking mode.

    @retval sal_True if mode could be changed.
*/
SAL_DLLPUBLIC sal_Bool SAL_CALL osl_enableNonBlockingMode(
                            oslSocket Socket,
                            sal_Bool On);


/** Query state of non-blocking-mode of the socket.

    @param Socket Query mode for this socket.

    @retval True if non-blocking-mode is enabled.
*/
SAL_DLLPUBLIC sal_Bool SAL_CALL osl_isNonBlockingMode(oslSocket Socket);

/** Queries the socket for its type.

    @param[in] Socket The socket to query.

    @retval osl_Socket_TypeStream
    @retval osl_Socket_TypeDgram
    @retval osl_Socket_TypeRaw
    @retval osl_Socket_TypeRdm
    @retval osl_Socket_TypeSeqPacket
    @retval osl_invalid_SocketType if an error occurred
*/
SAL_DLLPUBLIC oslSocketType SAL_CALL osl_getSocketType(oslSocket Socket);

/** returns  a string which describes the last socket error.

    @param[in] Socket The socket to query.
    @param[out] strError The string that receives the error message.
*/
SAL_DLLPUBLIC void SAL_CALL osl_getLastSocketErrorDescription(
                            oslSocket Socket,
                            rtl_uString **strError);

/** Returns a constant describing the last error for the socket system.

    @retval osl_Socket_E_NONE if no error occurred
    @retval osl_invalid_SocketError if an unknown (unmapped)
            error occurred, otherwise an enum describing the error.
*/
SAL_DLLPUBLIC oslSocketError SAL_CALL osl_getLastSocketError(
                            oslSocket Socket);

/** Type for the representation of socket sets.
*/
typedef struct oslSocketSetImpl * oslSocketSet;

/** Creates a set of sockets to be used with osl_demultiplexSocketEvents().

    @return A oslSocketSet or 0 if creation failed.
*/
SAL_DLLPUBLIC oslSocketSet SAL_CALL osl_createSocketSet(void);

/** Destroys an oslSocketSet.
*/
SAL_DLLPUBLIC void SAL_CALL osl_destroySocketSet(oslSocketSet Set);

/** Clears the set from all previously added sockets.

    @param Set the set to be cleared.
*/
SAL_DLLPUBLIC void SAL_CALL osl_clearSocketSet(oslSocketSet Set);


/** Adds a socket to the set.

    @param Set the set were the socket is added.
    @param Socket the socket to be added.
*/
SAL_DLLPUBLIC void SAL_CALL osl_addToSocketSet(oslSocketSet Set, oslSocket Socket);

/** Removes a socket from the set.

    @param Set the set were the socket is removed from.
    @param Socket the socket to be removed.
*/
SAL_DLLPUBLIC void SAL_CALL osl_removeFromSocketSet(oslSocketSet Set, oslSocket Socket);

/** Checks if socket is in the set.

    @param Set the set to be checked.
    @param Socket check if this socket is in the set.

    @retval sal_True if socket is in the set.
*/
SAL_DLLPUBLIC sal_Bool SAL_CALL osl_isInSocketSet(oslSocketSet Set, oslSocket Socket);

/** Checks multiple sockets for events.

    @param IncomingSet Checks the sockets in this set for incoming events (read, accept). If the set is 0,
        it is just skipped.
    @param OutgoingSet Checks the sockets in this set for outgoing events (write, connect). If the set is 0,
        it is just skipped.
    @param OutOfBandSet Checks the sockets in this set for out-of-band events. If the set is 0, it is just
        skipped.
    @param pTimeout Address of the number of milliseconds to wait for events. If *pTimeout is -1, the call
        will block until an event or an error occurs.

    @return -1 on errors, otherwise the number of sockets with pending events. In case of timeout, the
        number might be 0.
*/
SAL_DLLPUBLIC sal_Int32 SAL_CALL osl_demultiplexSocketEvents(
                            oslSocketSet IncomingSet,
                            oslSocketSet OutgoingSet,
                            oslSocketSet OutOfBandSet,
                            const TimeValue* pTimeout);

/** Closes the socket terminating any ongoing dataflow.

    @param[in] Socket The socket to close.
 */
SAL_DLLPUBLIC void SAL_CALL osl_closeSocket(oslSocket Socket);


/** Retrieves n bytes from the stream and copies them into pBuffer.
    The function avoids incomplete reads due to packet boundaries.

    @param[in] Socket The socket to read from.
    @param[out] pBuffer receives the read data.
    @param[out] nSize the number of bytes to read. pBuffer must be large enough
        to hold the n bytes!

    @return the number of read bytes. The number will only be smaller than
        n if an exceptional condition (e.g. connection closed) occurs.
*/
SAL_DLLPUBLIC sal_Int32 SAL_CALL osl_readSocket(
                            oslSocket Socket,
                            void *pBuffer,
                            sal_Int32 nSize);


/** Writes n bytes from pBuffer to the stream. The method avoids
    incomplete writes due to packet boundaries.

    @param[out] Socket The socket to write to.
    @param[in] pBuffer contains the data to be written.
    @param[in] nSize the number of bytes to write.

    @return the number of written bytes. The number will only be smaller than
        nSize if an exceptional condition (e.g. connection closed) occurs.
*/
SAL_DLLPUBLIC sal_Int32 SAL_CALL osl_writeSocket(
                            oslSocket Socket,
                            const void *pBuffer,
                            sal_Int32 nSize);

/**@} end section oslSocket
*/
/**@{ begin section oslSocketAddr
*/

/** Creates a socket-address for the given family.
    @param Family If family == osl_Socket_FamilyInet the address is
                  set to INADDR_ANY port 0.
    @return 0 if address could not be created.
*/
SAL_DLLPUBLIC oslSocketAddr SAL_CALL osl_createEmptySocketAddr(
        oslAddrFamily Family);


/** Creates a new SocketAddress and fills it from Addr.
*/
SAL_DLLPUBLIC oslSocketAddr SAL_CALL osl_copySocketAddr(
        oslSocketAddr Addr);

/** Compares the values of two SocketAddresses.
    @retval sal_True if both addresses denote the same socket address.
    @retval sal_False if both addresses do not denote the same socket address.
*/
SAL_DLLPUBLIC sal_Bool SAL_CALL osl_isEqualSocketAddr(
    oslSocketAddr Addr1, oslSocketAddr Addr2);

/** Uses the systems name-service interface to find an address for strHostname.
    @param[in] strHostname The name for which you search for an address.
    @return The desired address if one could be found, otherwise 0.
    Don't forget to destroy the address if you don't need it any longer.
*/
SAL_DLLPUBLIC oslSocketAddr SAL_CALL osl_resolveHostname(
        rtl_uString *strHostname);

/** Create an internet address usable for sending broadcast datagrams.
    To limit the broadcast to your subnet, pass your hosts IP address
    in dotted decimal notation as first argument.
    @see    osl_sendToSocket()
    @see    oslSocketAddr
    @param[in]  strDottedAddr dotted decimal internet address, may be 0.
    @param[in]  Port port number in host byte order.
    @retval 0 if address could not be created.
*/
SAL_DLLPUBLIC oslSocketAddr SAL_CALL osl_createInetBroadcastAddr(
    rtl_uString *strDottedAddr, sal_Int32 Port);


/** Create an internet-address, consisting of host address and port.
    We interpret strDottedAddr as a dotted-decimal inet-addr
    (e.g. "141.99.128.50").
    @param[in] strDottedAddr  String with dotted address.
    @param[in] Port  portnumber in host byte order.
    @retval 0 if address could not be created.
*/
SAL_DLLPUBLIC oslSocketAddr SAL_CALL osl_createInetSocketAddr (
    rtl_uString *strDottedAddr, sal_Int32 Port);


/** Frees all resources allocated by Addr. The handle Addr must not
    be used after the call anymore.
*/
SAL_DLLPUBLIC void SAL_CALL osl_destroySocketAddr(
        oslSocketAddr Addr);

/** Looks up the port-number designated to the specified service/protocol-pair.
    (e.g. "ftp" "tcp").
    @retval OSL_INVALID_PORT if no appropriate entry was found, otherwise the port-number.
*/
SAL_DLLPUBLIC sal_Int32 SAL_CALL osl_getServicePort(
        rtl_uString *strServicename, rtl_uString *strProtocol);



/** Retrieves the address-family from the Addr.
    @return the family of the socket-address.
    In case of an unknown family you get osl_Socket_FamilyInvalid.
*/
SAL_DLLPUBLIC oslAddrFamily SAL_CALL osl_getFamilyOfSocketAddr(
        oslSocketAddr Addr);


/** Retrieves the internet port-number of Addr.
    @return the port-number of the address in host-byte order. If Addr
    is not an address of type osl_Socket_FamilyInet, it returns OSL_INVALID_PORT
*/
SAL_DLLPUBLIC sal_Int32 SAL_CALL osl_getInetPortOfSocketAddr(
        oslSocketAddr Addr);


/** Sets the Port of Addr.
    @param[in] Addr the SocketAddr to perform the operation on.
    @param[in] Port is expected in host byte-order.
    @retval sal_False if Addr is not an inet-addr.
*/
SAL_DLLPUBLIC sal_Bool SAL_CALL osl_setInetPortOfSocketAddr(
        oslSocketAddr Addr, sal_Int32 Port);


/** Returns the hostname represented by Addr.
    @param[in] Addr The socket address from which to extract the hostname.
    @param[out] strHostname The hostname represented by the address. If
    there is no hostname to be found, it returns 0.
*/
SAL_DLLPUBLIC oslSocketResult SAL_CALL osl_getHostnameOfSocketAddr(
        oslSocketAddr Addr, rtl_uString **strHostname);


/** Gets the address in dotted decimal format.

    @param[in] Addr The socket address from which to extract the dotted decimal address.
    @param[out] strDottedInetAddr Contains the dotted decimal address
    (e.g. 141.99.20.34) represented by the address.

    @retval If the address is invalid or not of type osl_Socket_FamilyInet, it returns 0.
    @retval osl_Socket_Ok
    @retval osl_Socket_Error
*/
SAL_DLLPUBLIC oslSocketResult SAL_CALL osl_getDottedInetAddrOfSocketAddr(
        oslSocketAddr Addr, rtl_uString **strDottedInetAddr);

/** Sets the addr field in the struct sockaddr with pByteSeq. pByteSeq must be in network byte order.
 */
SAL_DLLPUBLIC oslSocketResult SAL_CALL osl_setAddrOfSocketAddr(
        oslSocketAddr Addr, sal_Sequence *pByteSeq );

/** Returns the addr field in the struct sockaddr.
    @param[in] Addr The socket address from which to extract the ipaddress.
    @param[out] ppByteSeq After the call, *ppByteSeq contains the ipaddress
                     in network byteorder. *ppByteSeq may be 0 in case of an invalid socket handle.
    @retval osl_Socket_Ok
    @retval osl_Socket_Error
 */
SAL_DLLPUBLIC oslSocketResult SAL_CALL osl_getAddrOfSocketAddr(
        oslSocketAddr Addr, sal_Sequence **ppByteSeq );

/*
    Opaque datatype HostAddr.
*/
typedef struct oslHostAddrImpl * oslHostAddr;


/** Create an oslHostAddr from given hostname and socket address.
    @param[in] strHostname The hostname to be stored.
    @param[in] Addr The socket address to be stored.
    @return The created address or 0 upon failure.
*/
SAL_DLLPUBLIC oslHostAddr SAL_CALL osl_createHostAddr(
        rtl_uString *strHostname, const oslSocketAddr Addr);


/** Create an oslHostAddr by resolving the given strHostname.
    Successful name resolution should result in the fully qualified
    domain name (FQDN) and its address as hostname and socket address
    members of the resulting oslHostAddr.
    @param[in] strHostname The hostname to be resolved.
    @return The resulting address or 0 upon failure.
*/
SAL_DLLPUBLIC oslHostAddr SAL_CALL osl_createHostAddrByName(rtl_uString *strHostname);


/** Create an oslHostAddr by reverse resolution of the given Addr.
    Successful name resolution should result in the fully qualified
    domain name (FQDN) and its address as hostname and socket address
    members of the resulting oslHostAddr.
    @param[in] Addr The socket address to be reverse resolved.
    @return The resulting address or 0 upon failure.
*/
SAL_DLLPUBLIC oslHostAddr SAL_CALL osl_createHostAddrByAddr(const oslSocketAddr Addr);


/** Create a copy of the given Addr.
    @return The copied address or 0 upon failure.
*/
SAL_DLLPUBLIC oslHostAddr SAL_CALL osl_copyHostAddr(const oslHostAddr Addr);


/** Frees all resources allocated by Addr. The handle Addr must not
    be used after the call anymore.
*/
SAL_DLLPUBLIC void SAL_CALL osl_destroyHostAddr(oslHostAddr Addr);


/** Get the hostname member of Addr.
    @return The hostname or 0 upon failure.
*/
SAL_DLLPUBLIC void SAL_CALL osl_getHostnameOfHostAddr(const oslHostAddr Addr, rtl_uString **strHostname);


/** Get the socket address member of Addr.
    @return The socket address or 0 upon failure.
*/
SAL_DLLPUBLIC oslSocketAddr SAL_CALL osl_getSocketAddrOfHostAddr(const oslHostAddr Addr);

/** Retrieve this machines hostname.
    May not always be a fully qualified domain name (FQDN).
    @param  strLocalHostname out-parameter. The string that receives the local host name.
    @retval sal_True upon success
    @retval sal_False
*/
SAL_DLLPUBLIC oslSocketResult SAL_CALL osl_getLocalHostname(rtl_uString **strLocalHostname);


/**@} end section oslHostAddr
*/

#ifdef __cplusplus
}
#endif

#endif // INCLUDED_OSL_SOCKET_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
