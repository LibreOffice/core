/*************************************************************************
 *
 *  $RCSfile: socket.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-16 15:25:47 $
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

#ifndef _VOS_SOCKET_HXX_
#define _VOS_SOCKET_HXX_

#ifndef _VOS_TYPES_HXX_
#   include <vos/types.hxx>
#endif
#ifndef _VOS_OBJECT_HXX_
#   include <vos/object.hxx>
#endif
#ifndef _VOS_ISTREAM_HXX_
#   include <vos/istream.hxx>
#endif
#ifndef _VOS_REFERMCE_HXX_
#   include <vos/refernce.hxx>
#endif
#ifndef _VOS_REFOBJ_HXX_
#   include <vos/refobj.hxx>
#endif
#ifndef _RTL_USTRING_
#   include <rtl/ustring>
#endif
#ifndef _OSL_SOCKET_H_
#   include <osl/socket.h>
#endif

#ifndef _OSL_TIME_H_
#include <osl/time.h>
#endif

#ifdef _USE_NAMESPACE
namespace vos
{
#endif

/** Base data types
*/
class ISocketTypes
{
public:

    ISocketTypes() { }
    virtual ~ISocketTypes() { }

    /*
        Represents the address-family of a socket
    */
    enum TAddrFamily {
        TFamily_Inet = osl_Socket_FamilyInet,       /* IP */
        TFamily_Ipx  = osl_Socket_FamilyIpx,        /* Novell IPX/SPX */
        TFamily_Invalid = osl_Socket_FamilyInvalid
    };

    /*
        represent a specific protocol within a address-family
    */
    enum TProtocol {
        TProtocol_Ip    = osl_Socket_ProtocolIp,        /* for all af_inet */
        TProtocol_Ipx   = osl_Socket_ProtocolIpx,       /* af_ipx datagram sockets (IPX) */
        TProtocol_Spx   = osl_Socket_ProtocolSpx,       /* af_ipx seqpacket or stream for SPX */
        TProtocol_SpxII = osl_Socket_ProtocolSpxII,     /* af_ipx seqpacket or stream for SPX II */
        TProtocol_Invalid = osl_Socket_ProtocolInvalid
    };

    /*
        Represents the type of a socket
    */
    enum TSocketType {
        TType_Stream    = osl_Socket_TypeStream,
        TType_Dgram     = osl_Socket_TypeDgram,
        TType_Raw       = osl_Socket_TypeRaw,
        TType_Rdm       = osl_Socket_TypeRdm,
        TType_SeqPacket = osl_Socket_TypeSeqPacket,
        TType_Invalid = osl_Socket_TypeInvalid
    };

    /*
        Represents socket-options
    */
    enum TSocketOption {
        TOption_Debug       = osl_Socket_OptionDebug,
        TOption_AcceptConn  = osl_Socket_OptionAcceptConn,
        TOption_ReuseAddr   = osl_Socket_OptionReuseAddr,
        TOption_KeepAlive   = osl_Socket_OptionKeepAlive,
        TOption_DontRoute   = osl_Socket_OptionDontRoute,
        TOption_Broadcast   = osl_Socket_OptionBroadcast,
        TOption_UseLoopback = osl_Socket_OptionUseLoopback,
        TOption_Linger      = osl_Socket_OptionLinger,
        TOption_OOBinLine   = osl_Socket_OptionOOBinLine,
        TOption_SndBuf      = osl_Socket_OptionSndBuf,
        TOption_RcvBuf      = osl_Socket_OptionRcvBuf,
        TOption_SndLowat    = osl_Socket_OptionSndLowat,
        TOption_RcvLowat    = osl_Socket_OptionRcvLowat,
        TOption_SndTimeo    = osl_Socket_OptionSndTimeo,
        TOption_RcvTimeo    = osl_Socket_OptionRcvTimeo,
        TOption_Error       = osl_Socket_OptionError,
        TOption_Type        = osl_Socket_OptionType,
        TOption_TcpNoDelay  = osl_Socket_OptionTcpNoDelay,
        TOption_Invalid     = osl_Socket_OptionInvalid
    };

    /*
        Represents the different socket-option levels
    */
    enum TSocketOptionLevel {
        TLevel_Socket = osl_Socket_LevelSocket,
        TLevel_Tcp    = osl_Socket_LevelTcp,
        TLevel_Invalid = osl_Socket_LevelInvalid
    };

    /*
        Represents flags to be used with send/recv-calls.
    */
    enum TSocketMsgFlag {
        TMsg_Normal    = osl_Socket_MsgNormal,
        TMsg_OOB       = osl_Socket_MsgOOB,
        TMsg_Peek      = osl_Socket_MsgPeek,
        TMsg_DontRoute = osl_Socket_MsgDontRoute,
        TMsg_MaxIOVLen = osl_Socket_MsgMaxIOVLen,
        TMsg_Invalid   = osl_Socket_MsgInvalid
    };

    /*
        Used by shutdown to denote which end of the socket to "close".
    */
    enum TSocketDirection {
        TDirection_Read      = osl_Socket_DirRead,
        TDirection_Write     = osl_Socket_DirWrite,
        TDirection_ReadWrite = osl_Socket_DirReadWrite,
        TDirection_Invalid   = osl_Socket_DirInvalid
    };

    enum TSocketError {
        E_None              = osl_Socket_E_None,              /* no error */
        E_NotSocket         = osl_Socket_E_NotSocket,         /* Socket operation on non-socket */
        E_DestAddrReq       = osl_Socket_E_DestAddrReq,       /* Destination address required */
        E_MsgSize           = osl_Socket_E_MsgSize,           /* Message too sal_Int32 */
        E_Prototype         = osl_Socket_E_Prototype,         /* Protocol wrong type for socket */
        E_NoProtocol        = osl_Socket_E_NoProtocol,        /* Protocol not available */
        E_ProtocolNoSupport = osl_Socket_E_ProtocolNoSupport, /* Protocol not supported */
        E_TypeNoSupport     = osl_Socket_E_TypeNoSupport,     /* Socket type not supported */
        E_OpNotSupport      = osl_Socket_E_OpNotSupport,      /* Operation not supported on socket */
        E_PfNoSupport       = osl_Socket_E_PfNoSupport,       /* Protocol family not supported */
        E_AfNoSupport       = osl_Socket_E_AfNoSupport,       /* Address family not supported by */
                                                                /* protocol family */
        E_AddrInUse         = osl_Socket_E_AddrInUse,         /* Address already in use */
        E_AddrNotAvail      = osl_Socket_E_AddrNotAvail,      /* Can't assign requested address */
        E_NetDown           = osl_Socket_E_NetDown,           /* Network is down */
        E_NetUnreachable    = osl_Socket_E_NetUnreachable,    /* Network is unreachable */
        E_NetReset          = osl_Socket_E_NetReset,          /* Network dropped connection because */
                                                                /* of reset */
        E_ConnAborted       = osl_Socket_E_ConnAborted,       /* Software caused connection abort */
        E_ConnReset         = osl_Socket_E_ConnReset,         /* Connection reset by peer */
        E_NoBufferSpace     = osl_Socket_E_NoBufferSpace,     /* No buffer space available */
        E_IsConnected       = osl_Socket_E_IsConnected,       /* Socket is already connected */
        E_NotConnected      = osl_Socket_E_NotConnected,      /* Socket is not connected */
        E_Shutdown          = osl_Socket_E_Shutdown,          /* Can't send after socket shutdown */
        E_TooManyRefs       = osl_Socket_E_TooManyRefs,       /* Too many references: can't splice */
        E_TimedOut          = osl_Socket_E_TimedOut,          /* Connection timed out */
        E_ConnRefused       = osl_Socket_E_ConnRefused,       /* Connection refused */
        E_HostDown          = osl_Socket_E_HostDown,          /* Host is down */
        E_HostUnreachable   = osl_Socket_E_HostUnreachable,   /* No route to host */
        E_WouldBlock        = osl_Socket_E_WouldBlock,        /* call would block on non-blocking socket */
        E_Already           = osl_Socket_E_Already,           /* operation already in progress */
        E_InProgress        = osl_Socket_E_InProgress,        /* operation now in progress */

        E_Invalid = osl_Socket_E_InvalidError   /* unmapped error */
    };

    enum TResult {
        TResult_Ok          = osl_Socket_Ok,          /* successful completion */
        TResult_Error       = osl_Socket_Error,       /* error occured, check osl_getLastSocketError() for details */
        TResult_TimedOut    = osl_Socket_TimedOut,    /* blocking operation timed out */
        TResult_Interrupted = osl_Socket_Interrupted, /* blocking operation was interrupted */
        TResult_InProgress  = osl_Socket_InProgress   /* nonblocking operation is in progress */
    };
};


/** Base class for socket addresses.
*/
class ISocketAddr : public NAMESPACE_VOS(ISocketTypes)
{
public:
    virtual ~ISocketAddr() { }


    virtual TAddrFamily SAL_CALL getFamily() const= 0;
    virtual TResult SAL_CALL getHostname(::rtl::OUString& strHostName) const= 0;
    virtual SAL_CALL operator oslSocketAddr() const= 0;
    virtual void SAL_CALL operator= (oslSocketAddr Addr)= 0;
    virtual sal_Bool SAL_CALL operator== (oslSocketAddr Addr)= 0;
};

class OSocketAddr : public NAMESPACE_VOS(ISocketAddr),
            public NAMESPACE_VOS(OObject)

{
    VOS_DECLARE_CLASSINFO(NAMESPACE_VOS(OSocketAddr));
public:

    /** Creates socket address of unknown type.
    */
    OSocketAddr();

    /** Copy constructor.
    */
    OSocketAddr(const OSocketAddr& Addr);

    /**
    */
    OSocketAddr(oslSocketAddr Addr);

    /** destroys underlying oslSocketAddress
    */
    virtual ~OSocketAddr();

    /** Queries the socket for its address family.
        @return the address family of the socket.
    */
    virtual TAddrFamily SAL_CALL getFamily() const;

    /** Cast Object to the underlying oslSocketAddr.
    */
    virtual SAL_CALL operator oslSocketAddr() const;

    /** Converts the address to a (human readable) domain-name.
        @return the hostname represented by the address.
        On failure returns the empty string.
    */
    virtual TResult SAL_CALL getHostname(::rtl::OUString& strHostName) const;

    /** Get the hostname for the local interface.
        @return the hostname or an error.
    */
    static TResult SAL_CALL getLocalHostname(::rtl::OUString& strLocalHostName);

    /** Tries to find an address for a host.
        @return A new created socket-address or 0 if the name could not be found.
    */
    static oslSocketAddr SAL_CALL resolveHostname(const ::rtl::OUString& strHostName);

    /** Wraps itself around the osl Socket-Address.
        The object assumes ownership of the Addr, it
        will be destroyed by destructor(). If the socket is already attached to
        an oslSocketAddr, the existing one will be destroyed.
    */
    virtual void SAL_CALL operator= (oslSocketAddr Addr);

    /** Compares to Addr
    */
    virtual sal_Bool SAL_CALL operator== (oslSocketAddr Addr);

    /** Makes a copy of Addr.
    */
    OSocketAddr& SAL_CALL operator= (const OSocketAddr& Addr);


protected:

    oslSocketAddr m_SockAddr;
};


/** Represents an internet-address.
*/
class OInetSocketAddr : public NAMESPACE_VOS(OSocketAddr)
{
    VOS_DECLARE_CLASSINFO(NAMESPACE_VOS(OInetSocketAddr));
public:

    /** Creates an empty internet-address (INADDR_ANY).
    */
    OInetSocketAddr();

    /** Wraps itself around the osl Socket-Address.
        The object assumes ownership of the Addr, it
        will be destroyed by ~OInetSocketAddr().
    */
    OInetSocketAddr(oslSocketAddr Addr);

    /**
        Create a socket address either from a dotted decimal address
        (e.g. 141.99.128.50) or a hostname (e.g. www.stardiv.de).
    */
    OInetSocketAddr(const ::rtl::OUString& strAddrOrHostName, sal_Int32 Port);

    /**
        Copy constructor.
    */
    OInetSocketAddr(const OInetSocketAddr& Addr);

    /**
    */
    OInetSocketAddr(const OSocketAddr& Addr);


    virtual ~OInetSocketAddr();

    /**
    */
    virtual void SAL_CALL operator= (oslSocketAddr Addr);

    /**
    */
    virtual sal_Bool SAL_CALL operator== (oslSocketAddr Addr);

    /**
    */
    OInetSocketAddr& SAL_CALL operator= (const OInetSocketAddr& Addr);

    /**
    */
    OInetSocketAddr& SAL_CALL operator= (const OSocketAddr& Addr);

    /**
        Tries to find the port associated with the given service/protocol-
        pair (e.g. "ftp"/"tcp").
        @return the port number in host-byte order or CVOS_PORT_NONE
        if no service/protocol pair could be found.
    */
    static sal_Int32 SAL_CALL getServicePort(const ::rtl::OUString& strServiceName,
                              const ::rtl::OUString& strProtocolName= ::rtl::OUString::createFromAscii( "tcp" ) );


    /** Delivers the port number of the address.
        @return the port in host-byte order or or OSL_INVALID_PORT on errors.
    */
    sal_Int32   SAL_CALL getPort() const;

    /** Sets the port number of the address.
        @return False if the port couldn't be set
        (e.g because the address is not of family TFamily_Inet).
    */
    sal_Bool    SAL_CALL setPort(sal_Int32 Port);

    /** @return the dotted-address-form (141.99.128.90) of this address.
        On failure returns the empty string.
    */
    TResult SAL_CALL getDottedAddr(::rtl::OUString& strDottedAddr) const;

    /** Sets the host-part of the address from the dotted-address-form (141.99.128.90)
        or from a hostname.
        @param strDottedAddrOrHostname the address in dotted form or a hostname.
    */
    sal_Bool SAL_CALL setAddr(const ::rtl::OUString& strDottedAddrOrHostname);

};

/** Represents an IPX/SPX address.
*/
class OIpxSocketAddr :  public NAMESPACE_VOS(OSocketAddr)
{
    VOS_DECLARE_CLASSINFO(NAMESPACE_VOS(OIpxSocketAddr));
public:

    typedef oslSocketIpxNetNumber  TIpxNetNumber;
    typedef oslSocketIpxNodeNumber TIpxNodeNumber;

    /** Creates an empty ipx-address.
    */
    OIpxSocketAddr();

    /** Wraps itself around the osl Socket-Address.
        The object assumes ownership of the Addr, it
        will be destroyed by the destructor.
    */
    OIpxSocketAddr(oslSocketAddr Addr);

    /** Create an IPX/SPX socketaddress from native parameters.
    */
    OIpxSocketAddr(const ::rtl::OUString& strNetNumber,
                   const ::rtl::OUString& strNodeNumber,
                   sal_uInt32 SocketNumber);

    /** Copy constructor.
    */
    OIpxSocketAddr(const OIpxSocketAddr& Addr);

    /**
    */
    OIpxSocketAddr(const OSocketAddr& Addr);

    virtual  ~OIpxSocketAddr();

    /**
    */
    virtual void SAL_CALL operator= (oslSocketAddr Addr);

    /**
    */
    virtual sal_Bool SAL_CALL operator== (oslSocketAddr Addr);

    /**
    */
    OIpxSocketAddr& SAL_CALL operator= (const OIpxSocketAddr& Addr);

    /**
    */
    OIpxSocketAddr& SAL_CALL operator= (const OSocketAddr& Addr);

    /**
    */
    TResult SAL_CALL getNetNumber(TIpxNetNumber& NetNumber) const;

    /**
    */
    TResult SAL_CALL getNodeNumber(TIpxNodeNumber& NodeNumber) const;

    /**
    */
    sal_uInt32 SAL_CALL getSocketNumber() const;

    /** Builds a human readable string in the format network.node:socket.
        The numbers are given in hexadecimal form.
    */
    void SAL_CALL getAddressString(::rtl::OUString& strAddressString) const;
};


/** Represents a socket.
*/
class OSocket : public NAMESPACE_VOS(ISocketTypes),
                public NAMESPACE_VOS(OReference),
                public NAMESPACE_VOS(OObject)
{
    VOS_DECLARE_CLASSINFO(NAMESPACE_VOS(OSocket));

protected:
    typedef ORefObj<oslSocket> SockRef;

    SockRef* m_pSockRef;

    TimeValue* m_pSendTimeout;
    TimeValue* m_pRecvTimeout;

public:

    /** Does not create a socket. Use assignment operator to
        make this a useable socket.
    */
    OSocket();

    /** Creates a socket.
        @param Family
        @param Type
        @param Protocol
    */
    OSocket(TSocketType Type,
            TAddrFamily Family = TFamily_Inet,
            TProtocol   Protocol = TProtocol_Ip);

    /** Copy constructor.
    */
    OSocket(const OSocket& sock);

    /** Creates socket as wrapper around the underlying oslSocket.
        @param Socket
    */
    OSocket(oslSocket Socket);

    /** Destructor. Destroys the underlying oslSocket.
    */
    virtual ~OSocket();

    /** Create a socket with the given attributes.
        If socket was already created, the old one will be discarded.
        @param Type
        @param Family
        @param Protocol
        @return True if socket was successfully created.
    */
    sal_Bool SAL_CALL create(TSocketType Type,
                   TAddrFamily Family = TFamily_Inet,
                   TProtocol   Protocol = TProtocol_Ip);

    /** Assignment operator. If socket was already created, the old one will
        be discarded.
    */
    OSocket& SAL_CALL operator= (const OSocket& sock);

    /** Allow cast to underlying oslSocket.
    */
    SAL_CALL operator oslSocket() const;

    /** Checks if the socket is valid.
        @return True if the object represents a valid socket.
    */
    sal_Bool SAL_CALL isValid() const;

    sal_Bool SAL_CALL operator==( const OSocket& rSocket )
    {
        return m_pSockRef == rSocket.m_pSockRef;
    }

    /** Closes the socket.
    */
    virtual void SAL_CALL close();

    /** Retrieves the address of the local interface of this socket.
        @param Addr [out] receives the address.
    */
    void SAL_CALL getLocalAddr(OSocketAddr& Addr) const;

    /** Get the local port of the socket.
        @return the port number or OSL_INVALID_PORT on errors.
    */
    sal_Int32   SAL_CALL getLocalPort() const;

    /** Get the hostname for the local interface.
        @return the hostname or an empty string ("").
    */
    TResult SAL_CALL getLocalHost(::rtl::OUString& strLocalHost) const;

    /** Retrieves the address of the remote host of this socket.
        @param Addr [out] receives the address.
    */
    void SAL_CALL getPeerAddr(OSocketAddr& Addr) const;

    /** Get the remote port of the socket.
        @return the port number or OSL_INVALID_PORT on errors.
    */
    sal_Int32   SAL_CALL getPeerPort() const;

    /** Get the hostname for the remote interface.
        @return the hostname or an empty string ("").
    */
    TResult SAL_CALL getPeerHost(::rtl::OUString& strPeerHost) const;

    /** Binds the socket to the specified (local) interface.
        @param LocalInterface Address of the Interface
        @return True if bind was successful.
    */
    sal_Bool SAL_CALL bind(const OSocketAddr& LocalInterface);


    /** Blocking send operations will unblock after the send-timeout.
        @return 0 for disables timeout else timeout value.
    */
    const TimeValue* SAL_CALL getSendTimeout() const
        { return m_pSendTimeout; }

    /** Blocking receive operations will unblock after the send-timeout.
        @return 0 for disables timeout else timeout value.
    */
    const TimeValue* SAL_CALL getRecvTimeout() const
        { return m_pRecvTimeout; }

    /** Blocking send operations will unblock after the send-timeout.
        @param time pTimeout == 0 disables timeout. pTimeout != 0 sets timeout value.
    */
    void SAL_CALL setSendTimeout(const TimeValue* pTimeout = 0);

    /** Blocking receive operations will unblock after the send-timeout.
        @param time pTimeout == 0 disables timeout. pTimeout != 0 sets timeout value.
    */
    void SAL_CALL setRecvTimeout(const TimeValue* pTimeout = 0);

    /** Checks if read operations will block.
        You can specify a timeout-value in seconds/nanoseconds that denotes
        how sal_Int32 the operation will block if the Socket is not ready.
        @return True if read operations (recv, recvFrom, accept) on the Socket
        will NOT block; False if it would block or if an error occured.

        @param pTimeout if 0, the operation will block without a timeout. Otherwise
        the specified amout of time.
    */
    sal_Bool    SAL_CALL isRecvReady(const TimeValue* pTimeout = 0) const;

    /** Checks if send operations will block.
        You can specify a timeout-value in seconds/nanoseconds that denotes
        how sal_Int32 the operation will block if the Socket is not ready.
        @return True if send operations (send, sendTo) on the Socket
        will NOT block; False if it would block or if an error occured.

        @param pTimeout if 0, the operation will block without a timeout. Otherwise
        the specified amout of time.
    */
    sal_Bool    SAL_CALL isSendReady(const TimeValue* pTimeout = 0) const;

    /** Checks if a request for out-of-band data will block.
        You can specify a timeout-value in seconds/nanoseconds that denotes
        how sal_Int32 the operation will block if the Socket has no pending OOB data.

        @return True if OOB-request operations (recv with appropriate flags)
        on the Socket will NOT block; False if it would block or if an error occured.

        @param pTimeout if 0, the operation will block without a timeout. Otherwise
        the specified amout of time.
    */
    sal_Bool    SAL_CALL isExceptionPending(const TimeValue* pTimeout = 0) const;

    /** Retrieves option-attributes associated with the socket.
        @param Option The attribute to query.
        Valid values (depending on the Level) are:
        <ul>
        <li> TOption_Debug
        <li> TOption_AcceptConn
        <li> TOption_ReuseAddr
        <li> TOption_KeepAlive
        <li> TOption_DontRoute
        <li> TOption_Broadcast
        <li> TOption_UseLoopback
        <li> TOption_Linger
        <li> TOption_OOBinLine
        <li> TOption_SndBuf
        <li> TOption_RcvBuf
        <li> TOption_SndLowat
        <li> TOption_RcvLowat
        <li> TOption_SndTimeo
        <li> TOption_RcvTimeo
        <li> TOption_Error
        <li> TOption_Type
        <li> TOption_TcpNoDelay
        </ul>
        If not above mentioned otherwise, the options are only valid for
        level TLevel_Socket.

        @param pBuffer The Buffer will be filled with the attribute.

        @param BufferSize The size of pBuffer.

        @param Level The option level. Valid values are:
        <ul>
        <li> TLevel_Socket : Socket Level
        <li> TLevel_Tcp    : Level of Transmission Control Protocol
        </ul>

        @return The size of the attribute copied into pBuffer ot -1 if an error
        occured.
    */
    sal_Int32 SAL_CALL getOption(TSocketOption Option,
                      void* pBuffer,
                      sal_uInt32 BufferLen,
                      TSocketOptionLevel Level= TLevel_Socket) const;

    /** Sets the sockets attributes.

        @param Option denotes the option to modify.
        Valid values (depending on the Level) are:
        <ul>
        <li> TOption_Debug
        <li> TOption_AcceptConn
        <li> TOption_ReuseAddr
        <li> TOption_KeepAlive
        <li> TOption_DontRoute
        <li> TOption_Broadcast
        <li> TOption_UseLoopback
        <li> TOption_Linger
        <li> TOption_OOBinLine
        <li> TOption_SndBuf
        <li> TOption_RcvBuf
        <li> TOption_SndLowat
        <li> TOption_RcvLowat
        <li> TOption_SndTimeo
        <li> TOption_RcvTimeo
        <li> TOption_Error
        <li> TOption_Type
        <li> TOption_TcpNoDelay
        </ul>
        If not above mentioned otherwise, the options are only valid for
        level TLevel_Socket.

        @param pBuffer Pointer to a Buffer which contains the attribute-value.

        @param BufferSize contains the length of the Buffer.

        @param Level selects the level for which an option should be changed.
        Valid values are:
        <ul>
        <li> TLevel_Socket : Socket Level
        <li> TLevel_Tcp    : Level of Transmission Control Protocol
        </ul>

        @return True if the option could be changed.
    */
    sal_Bool SAL_CALL setOption(TSocketOption Option,
                      void* pBuffer,
                      sal_uInt32 BufferLen,
                      TSocketOptionLevel Level= TLevel_Socket) const;


    /** Enables/disables non-blocking mode of the socket.
        @param On If True, non-blocking mode will be switched on, if False
        socket will become a blocking socket, which is the default behaviour of a
        socket.
        @return True if mode could be set.
    */
    sal_Bool SAL_CALL enableNonBlockingMode(sal_Bool On= sal_True);

    /** Query non-blocking mode of the socket.
    @return True if non-blocking mode is set.
    */
    sal_Bool SAL_CALL isNonBlockingMode() const;

    /** Queries the socket for its type.
        @return one of:
        <ul>
        <li> TType_Stream
        <li> TType_Dgram
        <li> TType_Raw
        <li> TType_Rdm
        <li> TType_SeqPacket
        <li> TType_Invalid
        </ul>
    */
    TSocketType SAL_CALL getType() const;


    /** Gets and clears the error status of the socket.
        @returns the current error state.
    */
    sal_Int32   SAL_CALL clearError() const;

    /** Enables/Disables debugging.
        @param opt 1 sets, 0 resets, -1 won't change anything
        @return the previous setting
    */
    sal_Int32   SAL_CALL setDebug(sal_Int32 opt = -1) const;

    /** Allow the socket to be bound to an address that is already in use.
        @param opt 1 sets, 0 resets, -1 won't change anything
        @return the previous setting
    */
    sal_Int32   SAL_CALL setReuseAddr(sal_Int32 opt = -1) const;

    /** Send keepalive-packets.
        @param opt 1 sets, 0 resets, -1 won't change anything
        @return the previous setting
    */
    sal_Int32   SAL_CALL setKeepAlive(sal_Int32 opt = -1) const;

    /** Do not route: send directly to interface.
        @param opt 1 sets, 0 resets, -1 won't change anything
        @return the previous setting
    */
    sal_Int32   SAL_CALL setDontRoute(sal_Int32 opt = -1) const;


    /** Allow transmission of broadcast messages on the socket.
        @param opt 1 sets, 0 resets, -1 won't change anything
        @return the previous setting
    */
    sal_Int32   SAL_CALL setBroadcast(sal_Int32 opt = -1) const;

    /** Receive out-of-band data in the normal data stream.
        @param opt 1 sets, 0 resets, -1 won't change anything
        @return the previous setting
    */
    sal_Int32   SAL_CALL setOobinline(sal_Int32 opt = -1) const;

    /** Linger on close if unsent data is present.
        @param time values > 0 enable lingering with a timeout of time in seconds.
        If time is 0, lingering will be disabled. If time is -1 no changes will occur.
        @return the previous setting (0 == off, > 0 timeout-value in seconds).
    */
    sal_Int32   SAL_CALL setLinger(sal_Int32 time = -1) const;

    /** Specify buffer size for sends.
        You might want to use getOption() to check if the size changes were
        really successful.
        @param size Size >= 0 sets the size, -1 won't change anything.
        @return the previous setting
    */
    sal_Int32   SAL_CALL setSendBufSize(sal_Int32 size =-1) const;

    /** Specify buffer size for receives.
        You might want to use getOption() to check if the size changes were
        really successful.
        @param size Size >= 0 sets the size, -1 won't change anything.
        @return the previous setting
    */
    sal_Int32   SAL_CALL setRecvBufSize(sal_Int32 size =-1) const;

    /** Disables the Nagle algorithm for send coalescing. (Do not
        collect data until a packet is full, instead send immediatly.
        This increases network traffic but might improve response-times.)
        @param opt 1 sets, 0 resets, -1 won't change anything
        @return the previous setting
    */
    sal_Int32   SAL_CALL setTcpNoDelay(sal_Int32 sz =-1) const;

    /** Builds a string with the last error-message for the socket.
        @param pBuffer is filled with the error message.
        @param nSize the size of pBuffer. The message will be cut
        sal_Int16 if the buffer isn't large enough, but still remains
        a valid zero-terminated string.
    */
    void SAL_CALL getError(::rtl::OUString& strError) const;

    /** Delivers a constant decribing the last error for the socket system.
        @return ENONE if no error occured, invalid_SocketError if
        an unknown (unmapped) error occured, otherwise an enum describing the
        error.
    */
    TSocketError SAL_CALL getError() const;

};

/** A socket to send or receive a stream of data.
*/
class OStreamSocket : public NAMESPACE_VOS(OSocket),
                      public NAMESPACE_VOS(IStream)
{
    VOS_DECLARE_CLASSINFO(NAMESPACE_VOS(OStreamSocket));
public:

    /** Creates an unattached socket. You must attach the socket to an oslSocket
        e.g. by using the operator=(oslSocket), before you can use the stream-
        functionality of the object.
    */
    OStreamSocket();



    /** Creates socket as wrapper around the underlying oslSocket.
        @param Socket
    */
    OStreamSocket(oslSocket Socket);


    /** Copy constructor.
        @param Socket
    */
    OStreamSocket(const OStreamSocket& Socket);


    /**
    */
    OStreamSocket(const OSocket& Socket);

    /** Destructor. Calls shutdown(readwrite) and close().
    */
    virtual ~OStreamSocket();

    /** Closes the socket after calling shutdown.
    */
    virtual void SAL_CALL close();

    /** Attaches the oslSocket to this object. If the object
        already was attached to an oslSocket, the old one will
        be closed and destroyed.
        @param Socket.
    */
    OStreamSocket& SAL_CALL operator=(oslSocket Socket);

    /**
    */
    OStreamSocket& SAL_CALL operator=(const OSocket& Socket);

    /**
    */
    OStreamSocket& SAL_CALL operator=(const OStreamSocket& Socket);

    /** Retrieves n bytes from the stream and copies them into pBuffer.
        The method avoids incomplete reads due to packet boundaries.
        @param pBuffer receives the read data.
        @param n the number of bytes to read. pBuffer must be large enough
        to hold the n bytes!
        @return the number of read bytes. The number will only be smaller than
        n if an exceptional condition (e.g. connection closed) occurs.
    */
    virtual sal_Int32 SAL_CALL read(void* pBuffer, sal_uInt32 n) const;

    /** Writes n bytes from pBuffer to the stream. The method avoids
        incomplete writes due to packet boundaries.
        @param pBuffer contains the data to be written.
        @param n the number of bytes to write.
        @return the number of written bytes. The number will only be smaller than
        n if an exceptional condition (e.g. connection closed) occurs.
    */
    virtual sal_Int32 SAL_CALL write(const void* pBuffer, sal_uInt32 n);

    /** Checks if socket is closed.
        @return True if socket is closed.
    */
    virtual sal_Bool SAL_CALL isEof() const;

    /** Tries to receives BytesToRead data from the connected socket,

        @param pBuffer [out] Points to a buffer that will be filled with the received
        data.
        @param BytesToRead [in] The number of bytes to read. pBuffer must have at least
        this size.
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
    sal_Int32   SAL_CALL recv(void* pBuffer,
                 sal_uInt32 BytesToRead,
                 TSocketMsgFlag Flag= TMsg_Normal);


    /** Tries to sends BytesToSend data from the connected socket.

        @param pBuffer [in] Points to a buffer that contains the send-data.
        @param BytesToSend [in] The number of bytes to send. pBuffer must have at least
        this size.
        @param Flag [in] Modifier for the call. Valid values are:
        <ul>
        <li> TMsg_Normal
        <li> TMsg_OOB
        <li> TMsg_Peek
        <li> TMsg_DontRoute
        <li> TMsg_MaxIOVLen
        </ul>

        @return the number of transfered bytes.
    */
    sal_Int32 SAL_CALL send(const void* pBuffer,
                 sal_uInt32 BytesToSend,
                 TSocketMsgFlag Flag= TMsg_Normal);

    /** Closes a connection in a controlled manner.
        @param Direction Says which "end" of the socket is to be closed.
    */
    sal_Bool SAL_CALL shutdown(TSocketDirection Direction= TDirection_ReadWrite);
protected:

    /** Creates a socket. This constructor is used only by derived classes
        (e.g. OConnectorSocket).
        @param Family
        @param Protocol
        @param Type For some protocols it might be desirable to
                    use a different type than sock_stream (like sock_seqpacket).
                    Therefore we do not hide this parameter here.
    */
    OStreamSocket(TAddrFamily Family,
                  TProtocol   Protocol,
                  TSocketType Type= TType_Stream);


};


/** A socket to accept incoming connections.
*/
class OAcceptorSocket : public NAMESPACE_VOS(OSocket)
{
    VOS_DECLARE_CLASSINFO(NAMESPACE_VOS(OAcceptorSocket));
public:

    /** Creates a socket that can accept connections.
        @param Type For some protocols it might be desirable to
                    use a different type than sock_stream (like sock_seqpacket).
                    Therefore we do not hide this parameter here.
    */
    OAcceptorSocket(TAddrFamily Family= TFamily_Inet,
                    TProtocol   Protocol= TProtocol_Ip,
                    TSocketType Type= TType_Stream);

    /** Copy constructor.
    */
    OAcceptorSocket(const OAcceptorSocket& Socket);

    /** Destructor. Closes the socket and destroys the underlying oslSocket.
    */
    virtual ~OAcceptorSocket();

    /** Closes the socket. Also calls shutdown, needed to unblock
        accept on some systems.
    */
    virtual void SAL_CALL close();

    /** Prepare a socket for the accept-call.
        @param MaxPendingConnections The maximum number of pending
        connections (waiting to be accepted) on this socket. If you use
        -1, a system default value is used.
        @return True if call was successful.
    */
    sal_Bool SAL_CALL listen(sal_Int32 MaxPendingConnections= -1);

    /** Accepts incoming connections on the socket. You must
        precede this call with bind() and listen().
        @param Connection receives the incoming connection.
        @return result_ok: if a connection has been accepted,
                result_timeout: if m_RecvTimeout milliseconds passed without connect,
                result_error: on errors.
    */
    TResult SAL_CALL acceptConnection(OStreamSocket& Connection);

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
    TResult SAL_CALL acceptConnection(OStreamSocket&    Connection,
                             OSocketAddr& PeerAddr);

};


/** A socket to initiate a conenction.
*/
class OConnectorSocket : public NAMESPACE_VOS(OStreamSocket)
{
    VOS_DECLARE_CLASSINFO(NAMESPACE_VOS(OConnectorSocket));
public:

    /** Creates a socket that can accept connections.
        @param Type For some protocols it might be desirable to
                    use a different type than sock_stream (like sock_seqpacket).
                    Therefore we do not hide this parameter here.
    */
    OConnectorSocket(TAddrFamily Family= TFamily_Inet,
                     TProtocol   Protocol= TProtocol_Ip,
                     TSocketType Type= TType_Stream);

    /** Copy constructor. Doesn't duplicate oslSocket.
    */
    OConnectorSocket(const OConnectorSocket& Socket);

    /** Destructor. Relies on ~OStreamSocket to close down connection gracefully.
    */
    virtual ~OConnectorSocket();

    /** Connects the socket to a (remote) host.
        @param TargetHost The address of the target.
        @param msTimeout The timeout in milliseconds. Use -1 to block.
        @return result_ok if connected successfully,
                result_timeout on timeout,
                result_interrupted if unblocked forcefully (by close()),
                result_error if connect failed.
    */
    TResult SAL_CALL connect(const OSocketAddr& TargetHost, const TimeValue* pTimeout = 0);
};


/** A connectionless socket to send and receive datagrams.
*/
class ODatagramSocket : public NAMESPACE_VOS(OSocket)
{
    VOS_DECLARE_CLASSINFO(NAMESPACE_VOS(ODatagramSocket));
public:

    /** Creates a datagram socket.
        @param Type is sock_dgram by default.
    */
    ODatagramSocket(TAddrFamily Family= TFamily_Inet,
                    TProtocol   Protocol= TProtocol_Ip,
                    TSocketType Type= TType_Dgram);

    /** Copy constructor.
    */
    ODatagramSocket(const ODatagramSocket& Socket);

    /** Destructor. Closes the socket.
    */
    virtual ~ODatagramSocket();


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
    sal_Int32   SAL_CALL recvFrom(void*  pBuffer,
                     sal_uInt32 BufferSize,
                     OSocketAddr* pSenderAddr= 0,
                     TSocketMsgFlag Flag= TMsg_Normal);

    /** Tries to send one datagram with BytesToSend data to the given ReceiverAddr.
        Since we only send one packet, we don't need to concern ourselfes here with
        incomplete sends due to packet boundaries.

        @param ReceiverAddr [in] A SocketAddr that contains
        the destination address for this send.

        @param pBuffer [in] Points to a buffer that contains the send-data.
        @param BufferSize [in] The number of bytes to send. pBuffer must have at least
        this size.
        @param Flag [in] Modifier for the call. Valid values are:
        <ul>
        <li> TMsg_Normal
        <li> TMsg_OOB
        <li> TMsg_Peek
        <li> TMsg_DontRoute
        <li> TMsg_MaxIOVLen
        </ul>

        @return the number of transfered bytes.
    */
    sal_Int32   SAL_CALL sendTo(const OSocketAddr& ReceiverAddr,
                   const void* pBuffer,
                   sal_uInt32 BufferSize,
                   TSocketMsgFlag Flag= TMsg_Normal);
};



#ifdef _USE_NAMESPACE
}
#endif

#endif // _VOS_SOCKET_HXX_

