/*************************************************************************
 *
 *  $RCSfile: socket.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: martin.maher $ $Date: 2000-09-29 14:45:43 $
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

#ifndef _OSL_SOCKET_H_
#define _OSL_SOCKET_H_

#ifndef _RTL_USTRING_H_
#   include <rtl/ustring.h>
#endif

#ifndef _OSL_TYPES_H_
#   include <osl/types.h>
#endif

#ifndef _RTL_TENCINFO_H
#   include <rtl/tencinfo.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* error returns */
#define OSL_INADDR_NONE             0xffffffff
#define OSL_INVALID_PORT            0xffffffff
#define OSL_INVALID_IPX_SOCKET_NO   0xffffffff

/**@{ begin section types
*/

/*
    Opaque datatype SocketAddr.
*/
typedef void* oslSocketAddr;


/*
    Represents the address-family of a socket
*/
typedef enum {
    osl_Socket_FamilyInet,      /* IP */
    osl_Socket_FamilyIpx,       /* Novell IPX/SPX */
    osl_Socket_FamilyInvalid,   /* always last entry in enum! */
    osl_Socket_Family_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
} oslAddrFamily;

/*
    represent a specific protocol within a address-family
*/
typedef enum {
    osl_Socket_ProtocolIp,      /* for all af_inet */
    osl_Socket_ProtocolIpx,     /* af_ipx datagram sockets (IPX) */
    osl_Socket_ProtocolSpx,     /* af_ipx seqpacket or stream for SPX */
    osl_Socket_ProtocolSpxII,   /* af_ipx seqpacket or stream for SPX II */
    osl_Socket_ProtocolInvalid,
    osl_Socket_Protocol_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
} oslProtocol;


/*
    Represents the type of a socket
*/
typedef enum {
    osl_Socket_TypeStream,
    osl_Socket_TypeDgram,
    osl_Socket_TypeRaw,
    osl_Socket_TypeRdm,
    osl_Socket_TypeSeqPacket,
    osl_Socket_TypeInvalid,     /* always last entry in enum! */
    osl_Socket_Type_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
} oslSocketType;


/*
    Represents socket-options
*/
typedef enum {
    osl_Socket_OptionDebug,
    osl_Socket_OptionAcceptConn,
    osl_Socket_OptionReuseAddr,
    osl_Socket_OptionKeepAlive,
    osl_Socket_OptionDontRoute,
    osl_Socket_OptionBroadcast,
    osl_Socket_OptionUseLoopback,
    osl_Socket_OptionLinger,
    osl_Socket_OptionOOBinLine,
    osl_Socket_OptionSndBuf,
    osl_Socket_OptionRcvBuf,
    osl_Socket_OptionSndLowat,
    osl_Socket_OptionRcvLowat,
    osl_Socket_OptionSndTimeo,
    osl_Socket_OptionRcvTimeo,
    osl_Socket_OptionError,
    osl_Socket_OptionType,
    osl_Socket_OptionTcpNoDelay,
    osl_Socket_OptionInvalid,       /* always last entry in enum! */
    osl_Socket_Option_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
} oslSocketOption;

/*
    Represents the different socket-option levels
*/
typedef enum  {
    osl_Socket_LevelSocket,
    osl_Socket_LevelTcp,
    osl_Socket_LevelInvalid,            /* always last entry in enum! */
    osl_Socket_Level_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
} oslSocketOptionLevel;


/*
    Represents flags to be used with send/recv-calls.
*/
typedef enum {
    osl_Socket_MsgNormal,
    osl_Socket_MsgOOB,
    osl_Socket_MsgPeek,
    osl_Socket_MsgDontRoute,
    osl_Socket_MsgMaxIOVLen,
    osl_Socket_MsgInvalid,          /* always last entry in enum! */
    osl_Socket_Msg_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
} oslSocketMsgFlag;

/*
    Used by shutdown to denote which end of the socket to "close".
*/
typedef enum {
    osl_Socket_DirRead,
    osl_Socket_DirWrite,
    osl_Socket_DirReadWrite,
    osl_Socket_DirInvalid,          /* always last entry in enum! */
    osl_Socket_Dir_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
} oslSocketDirection;


typedef enum {
    osl_Socket_E_None,              /* no error */
    osl_Socket_E_NotSocket,         /* Socket operation on non-socket */
    osl_Socket_E_DestAddrReq,       /* Destination address required */
    osl_Socket_E_MsgSize,           /* Message too long */
    osl_Socket_E_Prototype,         /* Protocol wrong type for socket */
    osl_Socket_E_NoProtocol,        /* Protocol not available */
    osl_Socket_E_ProtocolNoSupport, /* Protocol not supported */
    osl_Socket_E_TypeNoSupport,     /* Socket type not supported */
    osl_Socket_E_OpNotSupport,      /* Operation not supported on socket */
    osl_Socket_E_PfNoSupport,       /* Protocol family not supported */
    osl_Socket_E_AfNoSupport,       /* Address family not supported by */
                                    /* protocol family */
    osl_Socket_E_AddrInUse,         /* Address already in use */
    osl_Socket_E_AddrNotAvail,      /* Can't assign requested address */
    osl_Socket_E_NetDown,           /* Network is down */
    osl_Socket_E_NetUnreachable,    /* Network is unreachable */
    osl_Socket_E_NetReset,          /* Network dropped connection because */
                                    /* of reset */
    osl_Socket_E_ConnAborted,       /* Software caused connection abort */
    osl_Socket_E_ConnReset,         /* Connection reset by peer */
    osl_Socket_E_NoBufferSpace,     /* No buffer space available */
    osl_Socket_E_IsConnected,       /* Socket is already connected */
    osl_Socket_E_NotConnected,      /* Socket is not connected */
    osl_Socket_E_Shutdown,          /* Can't send after socket shutdown */
    osl_Socket_E_TooManyRefs,       /* Too many references: can't splice */
    osl_Socket_E_TimedOut,          /* Connection timed out */
    osl_Socket_E_ConnRefused,       /* Connection refused */
    osl_Socket_E_HostDown,          /* Host is down */
    osl_Socket_E_HostUnreachable,   /* No route to host */
    osl_Socket_E_WouldBlock,        /* call would block on non-blocking socket */
    osl_Socket_E_Already,           /* operation already in progress */
    osl_Socket_E_InProgress,        /* operation now in progress */
    osl_Socket_E_InvalidError,      /* unmapped error: always last entry in enum! */
    osl_Socket_E_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
} oslSocketError;


typedef enum {
    osl_Socket_Ok,          /* successful completion */
    osl_Socket_Error,       /* error occured, check osl_getLastSocketError() for details */
    osl_Socket_TimedOut,    /* blocking operation timed out */
    osl_Socket_Interrupted, /* blocking operation was interrupted */
    osl_Socket_InProgress,  /* nonblocking operation is in progress */
    osl_Socket_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
} oslSocketResult;

typedef sal_uInt8 oslSocketIpxNetNumber[4];
typedef sal_uInt8 oslSocketIpxNodeNumber[6];

/**@} end section types
*/

/**@{ begin section oslSocketAddr
*/


/** Creates a socket-address for the given family.
    If family == osl_af_inet the address is set to INADDR_ANY
    port 0.
    @return 0 if address could not be created.
*/
oslSocketAddr SAL_CALL osl_createEmptySocketAddr(oslAddrFamily Family);


/** Create a new SocketAddress and fill it from Addr.
*/
oslSocketAddr SAL_CALL osl_copySocketAddr(oslSocketAddr Addr);

/** Compare to SocketAddress.
*/
sal_Bool SAL_CALL osl_isEqualSocketAddr(
    oslSocketAddr Addr1, oslSocketAddr Addr2);


/** Uses the systems name-service interface to find an address for strHostname.
    @param strHostname [in] The name for which you search for an address.
    @return The desired address if one could be found, otherwise 0.
    Don't forget to destroy the address if you don't need it any longer.
*/
oslSocketAddr SAL_CALL osl_resolveHostname(rtl_uString *strHostname);


/** Create an internet address usable for sending broadcast datagrams.
    To limit the broadcast to your subnet, pass your hosts IP address
    in dotted decimal notation as first argument.
    @see    osl_sendToSocket(..., oslSocketAddr ReceiverAddr, ...).
    @param  strDottedAddr [in] dotted decimal internet address, may be 0.
    @param  Port          [in] port number in host byte order.
    @return 0 if address could not be created.
*/
oslSocketAddr SAL_CALL osl_createInetBroadcastAddr (
    rtl_uString *strDottedAddr, sal_Int32 Port);


/** Create an internet-address, consisting of hostaddress and port.
    We interpret strDottedAddr as a dotted-decimal inet-addr
    (e.g. "141.99.128.50").
    @param strDottedAddr [in] String with dotted address.
    @param Port [in] portnumber in host byte order.
    @return 0 if address could not be created.
*/
oslSocketAddr SAL_CALL osl_createInetSocketAddr (
    rtl_uString *strDottedAddr, sal_Int32 Port);


/** Create an IPX address.
*/
oslSocketAddr SAL_CALL osl_createIpxSocketAddr(rtl_uString * strNetNumber,
                                                 rtl_uString * strNodeNumber,
                                               sal_uInt32 SocketNumber);

/** Free all memory allocated by pAddress.
*/
void SAL_CALL osl_destroySocketAddr(oslSocketAddr Addr);



/** Looks up the port-number designated to the specified service/protocol-pair.
    (e.g. "ftp" "tcp").
    @return OSL_INVALID_PORT if no appropriate entry was found, otherwise the port-number.
*/
sal_Int32 SAL_CALL osl_getServicePort(rtl_uString *strServicename, rtl_uString *strProtocol);



/** Retrieves the address-family from the Addr.
    @return the family of the socket-address.
    In case of an unknown family you get osl_invalid_AddrFamily.
*/
oslAddrFamily SAL_CALL osl_getFamilyOfSocketAddr(oslSocketAddr Addr);


/** Retrieves the internet port-number of Addr.
    @return the port-number of the address in host-byte order. If Addr
    is not an address of type osl_af_inet, it returns OSL_INVALID_PORT
*/
sal_Int32 SAL_CALL osl_getInetPortOfSocketAddr(oslSocketAddr Addr);


/** Sets the Port of Addr.
    @param Port [in] is expected in host byte-order.
    @return False if Addr is not an inet-addr.
*/
sal_Bool SAL_CALL osl_setInetPortOfSocketAddr(oslSocketAddr Addr, sal_Int32 Port);


/** Gets the hostname represented by Addr.
    @return the hostname represented by the address. If
    there is no hostname to be found, it returns 0.
*/
oslSocketResult SAL_CALL osl_getHostnameOfSocketAddr(oslSocketAddr Addr, rtl_uString **strHostname);


/** Gets the address in dotted decimal format.
    @return the dotted decimal address (e.g. 141.99.20.34) represented
    by the address. If the address is invalid or not of type osl_af_inet,
    it returns 0.
*/
oslSocketResult SAL_CALL osl_getDottedInetAddrOfSocketAddr(oslSocketAddr Addr, rtl_uString **strDottedInetAddr);


/** Gets the IPX Net-Number of the address.
    @return the (4 bytes long) net-number or 0 if not an IPX address.
*/
oslSocketResult SAL_CALL osl_getIpxNetNumber(oslSocketAddr Addr, oslSocketIpxNetNumber NetNumber);


/** Gets the IPX Node-Number of the address.
    @return the (6 bytes long) net-number or 0 if not an IPX address.
*/
oslSocketResult SAL_CALL osl_getIpxNodeNumber(oslSocketAddr Addr, oslSocketIpxNodeNumber NodeNumber);

/** Gets the IPX Socket-Number of the address.
    @return the IPX socket number or OSL_INVALID_IPX_SOCKET_NO if not an IPX address.
*/
sal_Int32 SAL_CALL osl_getIpxSocketNumber(oslSocketAddr Addr);

/**@} end section oslSocketAddr
*/

/**@{ begin section oslHostAddr
*/

/*
    Opaque datatype HostAddr.
*/
typedef void* oslHostAddr;


/** Create an oslHostAddr from given hostname and socket address.
    @param strHostname [in] The hostname to be stored.
    @param Addr [in] The socket address to be stored.
    @return The created address or 0 upon failure.
*/
oslHostAddr SAL_CALL osl_createHostAddr(rtl_uString *strHostname, const oslSocketAddr Addr);


/** Create an oslHostAddr by resolving the given strHostname.
    Successful name resolution should result in the fully qualified
    domain name (FQDN) and it's address as hostname and socket address
    members of the resulting oslHostAddr.
    @param strHostname [in] The hostname to be resolved.
    @return The resulting address or 0 upon failure.
*/
oslHostAddr SAL_CALL osl_createHostAddrByName(rtl_uString *strHostname);


/** Create an oslHostAddr by reverse resolution of the given Addr.
    Successful name resolution should result in the fully qualified
    domain name (FQDN) and it's address as hostname and socket address
    members of the resulting oslHostAddr.
    @param Addr [in] The socket address to be reverse resolved.
    @return The resulting address or 0 upon failure.
*/
oslHostAddr SAL_CALL osl_createHostAddrByAddr(const oslSocketAddr Addr);


/** Create a copy of the given Addr.
    @return The copied address or 0 upon failure.
*/
oslHostAddr SAL_CALL osl_copyHostAddr(const oslHostAddr Addr);


/** Free all memory allocated by Addr.
*/
void SAL_CALL osl_destroyHostAddr(oslHostAddr Addr);


/** Get the hostname member of Addr.
    @return The hostname or 0 upon failure.
*/
void SAL_CALL osl_getHostnameOfHostAddr(const oslHostAddr Addr, rtl_uString **strHostname);


/** Get the socket address member of Addr.
    @return The socket address or 0 upon failure.
*/
oslSocketAddr SAL_CALL osl_getSocketAddrOfHostAddr(const oslHostAddr Addr);

/** Retrieve this machines hostname.
    May not always be a fully qualified domain name (FQDN).
    @param  strLocalHostname [out] The string that receives the local host name.
    @return True upon success, False otherwise.
*/
oslSocketResult SAL_CALL osl_getLocalHostname(rtl_uString **strLocalHostname);


/**@} end section oslHostAddr
*/

/**@{ begin section oslSocket
*/


/*-***************************************************************************/
/* oslSocket */
/*-***************************************************************************/

typedef void* oslSocket;

/** Create a socket of the specified Family and Type. The semantic of
    the Protocol parameter depends on the given family and type.
    @return 0 if socket could not be created, otherwise you get a handle
    to the allocated socket-datastructure.
*/
oslSocket SAL_CALL osl_createSocket(oslAddrFamily   Family,
                                    oslSocketType   Type,
                                    oslProtocol     Protocol);

/** Create a socket as a copy of another.
    @return 0 if socket could not be created, otherwise you get a handle
    to the allocated socket-datastructure.
*/
oslSocket SAL_CALL osl_copySocket(oslSocket Socket);

/** Closes the socket and frees the Socket data-structure.
    For a graceful termination of a connection, you should call
    osl_shutdownSocket() first.
    @param Socket [in] The Socket to be closed and destroyed.
*/
void SAL_CALL osl_destroySocket(oslSocket Socket);


/** Retrieves the Address of the local end of the socket.
    Note that a socket must be bound or connected before
    a vaild address can be returned.
    @return 0 if socket-address could not be created, otherwise you get
    the created Socket-Address.
*/
oslSocketAddr SAL_CALL osl_getLocalAddrOfSocket(oslSocket Socket);

/** Retrieves the Address of the remote end of the socket.
    Note that a socket must be connected before
    a vaild address can be returned.
    @return 0 if socket-address could not be created, otherwise you get
    the created Socket-Address.
*/
oslSocketAddr SAL_CALL osl_getPeerAddrOfSocket(oslSocket Socket);

/** Binds the given address to the socket.
    @param Socket [in]
    @param Address [in]
    @return False if the bind failed.
*/
sal_Bool SAL_CALL osl_bindAddrToSocket(oslSocket Socket,
                                       oslSocketAddr Addr);

/** Connects the socket to the given address.

    @param Socket [in] a bound socket.
    @param Addr [in] the peer address.
    @param pTimeout Timeout value or NULL for blocking.

    @return osl_sock_result_ok on successful connection,
            osl_sock_result_timeout if operation timed out,
            osl_sock_result_interrupted if operation was interrupted
            osl_sock_result_error if the connection failed.
*/
oslSocketResult SAL_CALL osl_connectSocketTo(oslSocket Socket,
                                             oslSocketAddr Addr,
                                             const TimeValue* pTimeout);


/** Prepares the socket to act as an acceptor of incoming connections.
    You should call "listen" before you use "accept".
    @param MaxPendingConnections [in] denotes the length of the queue of
    pending connections for this socket. If MaxPendingConnections is
    -1, the systems default value will be used (Usually 5).
    @return False if the listen failed.
*/
sal_Bool SAL_CALL osl_listenOnSocket(oslSocket Socket,
                           sal_Int32 MaxPendingConnections);


/** Waits for an ingoing connection on the socket.
    This call blocks if there is no incoming connection present.
    @param pAddr [in] if pAddr is != 0, the peers address is returned.
    @return 0 if the accept-call failed, otherwise you get a socket
    representing the new connection.
*/
oslSocket SAL_CALL osl_acceptConnectionOnSocket(oslSocket Socket,
                                       oslSocketAddr* pAddr);

/** Tries to receive BytesToRead data from the connected socket,
    if no error occurs. Note that incomplete recvs due to
    packet boundaries may occur.

    @param Socket [in] A connected socket to be used to listen on.
    @param pBuffer [out] Points to a buffer that will be filled with the received
    data.
    @param BytesToRead [in] The number of bytes to read. pBuffer must have at least
    this size.
    @param Flag [in] Modifier for the call. Valid values are:
    <ul>
    <li> osl_msg_normal,
    <li> osl_msg_oob,
    <li> osl_msg_peek,
    <li> osl_msg_dontroute,
    <li> osl_msg_maxiovlen,
    </ul>

    @return the number of received bytes.
*/
sal_Int32 SAL_CALL osl_receiveSocket(oslSocket Socket,
                          void* pBuffer,
                           sal_uInt32 BytesToRead,
                          oslSocketMsgFlag Flag);

/** Tries to receives BufferSize data from the (usually unconnected)
    (datagram-)socket, if no error occurs.

    @param Socket [in] A bound socket to be used to listen for a datagram.
    @param SenderAddr [out] An initialized oslSocketAddress. It will be
    filled with the address of the datagrams sender. If SenderAddr is 0,
    it is ignored.
    @param pBuffer [out] Points to a buffer that will be filled with the received
    datagram.
    @param BufferSize [in] The size of pBuffer.
    @param Flag [in] Modifier for the call. Valid values are:
    <ul>
    <li> osl_msg_normal,
    <li> osl_msg_oob,
    <li> osl_msg_peek,
    <li> osl_msg_dontroute,
    <li> osl_msg_maxiovlen,
    </ul>

    @return the number of received bytes.
*/
sal_Int32 SAL_CALL osl_receiveFromSocket(oslSocket Socket,
                               oslSocketAddr SenderAddr,
                              void* pBuffer,
                              sal_uInt32 BufferSize,
                              oslSocketMsgFlag Flag);

/** Tries to send BytesToSend data from the connected socket,
    if no error occurs.

    @param Socket [in] A connected socket.
    @param pBuffer [in] Points to a buffer that contains the send-data.
    @param BytesToSend [in] The number of bytes to send. pBuffer must have at least
    this size.
    @param Flag [in] Modifier for the call. Valid values are:
    <ul>
    <li> osl_msg_normal,
    <li> osl_msg_oob,
    <li> osl_msg_peek,
    <li> osl_msg_dontroute,
    <li> osl_msg_maxiovlen,
    </ul>

    @return the number of transfered bytes.
*/
sal_Int32 SAL_CALL osl_sendSocket(oslSocket Socket,
                       const void* pBuffer,
                        sal_uInt32 BytesToSend,
                       oslSocketMsgFlag Flag);

/** Tries to send one datagram with BytesToSend data to the given ReceiverAddr
    via the (implicitly unconnected) datagram-socket.
    Since we only send one packet, we don't need to concern ourselfes here with
    incomplete sends due to packet boundaries.

    @param Socket [in] A bound or unbound socket. Socket will be bound
    after a successful call.

    @param ReceiverAddr [in] An initialized oslSocketAddress that contains
    the destination address for this send.

    @param pBuffer [in] Points to a buffer that contains the send-data.
    @param BytesToSend [in] The number of bytes to send. pBuffer must have at least
    this size.
    @param Flag [in] Modifier for the call. Valid values are:
    <ul>
    <li> osl_msg_normal,
    <li> osl_msg_oob,
    <li> osl_msg_peek,
    <li> osl_msg_dontroute,
    <li> osl_msg_maxiovlen,
    </ul>

    @return the number of transfered bytes.
*/
sal_Int32 SAL_CALL osl_sendToSocket(oslSocket Socket,
                         oslSocketAddr ReceiverAddr,
                          const void* pBuffer,
                         sal_uInt32 BytesToSend,
                         oslSocketMsgFlag Flag);

/** Checks if read operations will block.
    You can specify a timeout-value in seconds/microseconds that denotes
    how long the operation will block if the Socket is not ready.
    @return True if read operations (recv, recvFrom, accept) on the Socket
    will NOT block; False if it would block or if an error occured.

    @param Socket the Socket to perfom the operation on.
    @param pTimeout if NULL, the operation will block without a timeout. Otherwise
    the time define by timeout value.
*/
sal_Bool SAL_CALL osl_isReceiveReady(oslSocket Socket, const TimeValue* pTimeout);

/** Checks if send operations will block.
    You can specify a timeout-value in seconds/microseconds that denotes
    how long the operation will block if the Socket is not ready.
    @return True if send operations (send, sendTo) on the Socket
    will NOT block; False if it would block or if an error occured.

    @param Socket the Socket to perfom the operation on.
    @param pTimeout if NULL, the operation will block without a timeout. Otherwise
    the time define by timeout value.
*/
sal_Bool SAL_CALL osl_isSendReady(oslSocket Socket, const TimeValue* pTimeout);

/** Checks if a request for out-of-band data will block.
    You can specify a timeout-value in seconds/microseconds that denotes
    how long the operation will block if the Socket has no pending OOB data.
    @return True if OOB-request operations (recv with appropriate flags)
    on the Socket will NOT block; False if it would block or if an error occured.

    @param Socket the Socket to perfom the operation on.
    @param pTimeout if NULL, the operation will block without a timeout. Otherwise
    the time define by timeout value.
*/
sal_Bool SAL_CALL osl_isExceptionPending(oslSocket Socket, const TimeValue* pTimeout);

/** Shuts down communication on a connected socket.
    @param Direction denotes which end of the socket
    should be closed:
    <ul>
    <li> osl_shut_read  closes read operations.
    <li> osl_shut_write closes write operations.
    <li> osl_shut_readwrite closes read and write operations.
    </ul>
    @return True if the socket could be closed down.
*/
sal_Bool SAL_CALL osl_shutdownSocket(oslSocket Socket,
                           oslSocketDirection Direction);

/** Retrieves attributes associated with the socket.
    @param Socket is the socket to query.

    @param Level selects the level for which an option should be queried.
    Valid values are:
    <ul>
    <li> osl_sol_socket:    Socket Level
    <li> osl_sol_tcp:       Level of Transmission Control Protocol
    </ul>

    @param Option denotes the option to query.
    Valid values (depending on the Level) are:
    <ul>
    <li> osl_so_debug,
    <li> osl_so_acceptconn,
    <li> osl_so_reuseaddr,
    <li> osl_so_keepalive,
    <li> osl_so_dontroute,
    <li> osl_so_broadcast,
    <li> osl_so_useloopback,
    <li> osl_so_linger,
    <li> osl_so_oobinline,
    <li> osl_so_sndbuf,
    <li> osl_so_rcvbuf,
    <li> osl_so_sndlowat,
    <li> osl_so_rcvlowat,
    <li> osl_so_sndtimeo,
    <li> osl_so_rcvtimeo,
    <li> osl_so_error,
    <li> osl_so_type,
    <li> osl_so_tcp_nodelay,    (sol_tcp)
    </ul>
    If not above mentioned otherwise, the options are only valid for
    level sol_socket.

    @param pBuffer Pointer to a Buffer with enough room to take the desired
    attribute-value.

    @param BufferSize contains the length of the Buffer.

    @return -1 if an error occured or else the size of the data copied into
    pBuffer.
*/
sal_Int32 SAL_CALL osl_getSocketOption(oslSocket            Socket,
                               oslSocketOptionLevel Level,
                            oslSocketOption      Option,
                            void*                pBuffer,
                            sal_uInt32               BufferLen);

/** Sets the sockets attributes.

    @param Socket is the socket to modify.

    @param Level selects the level for which an option should be changed.
    Valid values are:
    <ul>
    <li> osl_sol_socket:    Socket Level
    <li> osl_sol_tcp:       Level of Transmission Control Protocol
    </ul>

    @param Option denotes the option to modify.
    Valid values (depending on the Level) are:
    <ul>
    <li> osl_so_debug,
    <li> osl_so_acceptconn,
    <li> osl_so_reuseaddr,
    <li> osl_so_keepalive,
    <li> osl_so_dontroute,
    <li> osl_so_broadcast,
    <li> osl_so_useloopback,
    <li> osl_so_linger,
    <li> osl_so_oobinline,
    <li> osl_so_sndbuf,
    <li> osl_so_rcvbuf,
    <li> osl_so_sndlowat,
    <li> osl_so_rcvlowat,
    <li> osl_so_sndtimeo,
    <li> osl_so_rcvtimeo,
    <li> osl_so_error,
    <li> osl_so_type,
    <li> osl_so_tcp_nodelay,    (sol_tcp)
    </ul>
    If not above mentioned otherwise, the options are only valid for
    level sol_socket.

    @param pBuffer Pointer to a Buffer which contains the attribute-value.

    @param BufferSize contains the length of the Buffer.

    @return True if the option could be changed.
*/
sal_Bool SAL_CALL osl_setSocketOption(oslSocket Socket,
                            oslSocketOptionLevel    Level,
                            oslSocketOption         Option,
                            void*                   pBuffer,
                            sal_uInt32                  BufferLen);

/** Enables/disables non-blocking-mode of the socket.
    @param Socket Change mode for this socket.
    @param On True: enable non-blocking mode, False: disable non-blocking mode.
    @return True if mode could be changed.
*/
sal_Bool SAL_CALL osl_enableNonBlockingMode(oslSocket Socket,
                                  sal_Bool On);


/** Query state of non-blocking-mode of the socket.
    @param Socket Query mode for this socket.
    @return True if non-blocking-mode is enabled.
*/
sal_Bool SAL_CALL osl_isNonBlockingMode(oslSocket Socket);


/** Queries the socket for its type.
    @return one of:
    <ul>
    <li> osl_sock_stream,
    <li> osl_sock_dgram,
    <li> osl_sock_raw,
    <li> osl_sock_rdm,
    <li> osl_sock_seqpacket,
    <li> osl_invalid_SocketType, if an error occured
    </ul>

*/
oslSocketType SAL_CALL osl_getSocketType(oslSocket Socket);

/** Delivers a string which describes the last socket error.
    @param strError The string that receives the error message.
    than the provided buffer, it will be cut short. Buffer sizes about 128 chars
    should be large enough.
*/
void SAL_CALL osl_getLastSocketErrorDescription(oslSocket Socket, rtl_uString **strError);

/** Delivers a constant decribing the last error for the socket system.
    @return osl_Socket_E_NONE if no error occured, osl_invalid_SocketError if
    an unknown (unmapped) error occured, otherwise an enum describing the
    error.
*/
oslSocketError SAL_CALL osl_getLastSocketError(oslSocket Socket);

/** Type for the representation of socket sets.
*/
typedef void* oslSocketSet;

/** Creates a set of sockets to be used with osl_demultiplexSocketEvents().
    @return A oslSocketSet or 0 if creation failed.
*/
oslSocketSet SAL_CALL osl_createSocketSet();

/** Destroys a oslSocketSet.
*/
void SAL_CALL osl_destroySocketSet(oslSocketSet Set);

/** Clears the set from all previously added sockets.
    @param Set the set to be cleared.
*/
void SAL_CALL osl_clearSocketSet(oslSocketSet Set);


/** Adds a socket to the set.
    @param Set the set were the socket is added.
    @param Socket the socket to be added.
*/
void SAL_CALL osl_addToSocketSet(oslSocketSet Set, oslSocket Socket);

/** Removes a socket from the set.
    @param Set the set were the socket is removed from.
    @param Socket the socket to be removed.
*/
void SAL_CALL osl_removeFromSocketSet(oslSocketSet Set, oslSocket Socket);

/** Checks if socket is in the set.
    @param Set the set to be checked.
    @param Socket check if this socket is in the set.
    @return True if socket is in the set.
*/
sal_Bool SAL_CALL osl_isInSocketSet(oslSocketSet Set, oslSocket Socket);

/** Checks multiple sockets for events.
    @param IncomingSet Checks the sockets in this set
    for incoming events (read, accept). If the set is 0,
    it is just skipped.
    @param OutgoingSet Checks the sockets in this set
    for outgoing events (write, connect). If the set is 0,
    it is just skipped.
    @param OutOfBandSet Checks the sockets in this set
    for out-of-band events. If the set is 0, it is just skipped.
    @param msTimeout Number of milliseconds to wait for events. If
    msTimeout is -1, the call will block until an event or an error
    occurs.
    @return -1 on errors, otherwise the number of sockets with
    pending events. In case of timeout, the number might be 0.
*/
sal_Int32 SAL_CALL osl_demultiplexSocketEvents(oslSocketSet IncomingSet,
                                    oslSocketSet OutgoingSet,
                                    oslSocketSet OutOfBandSet,
                                    const TimeValue* pTimeout);

void SAL_CALL osl_closeSocket(oslSocket Socket);


/**@} end section oslSocket
*/



#ifdef __cplusplus
}
#endif

#endif  /* _OSL_SOCKET_H_ */

