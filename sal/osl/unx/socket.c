/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "system.h"

#include <osl/socket.h>
#include <osl/diagnose.h>
#include <osl/mutex.h>
#include <osl/signal.h>

#include <rtl/alloc.h>

#include <ctype.h>
#include <sal/types.h>

#include "sockimpl.h"

/* defines for poll */
#ifdef HAVE_POLL_H
#undef HAVE_POLL_H
#endif

#if defined(LINUX) || defined(NETBSD) || defined ( FREEBSD ) || \
    defined (MACOSX) || defined (OPENBSD) || defined(DRAGONFLY)
#include <sys/poll.h>
#define HAVE_POLL_H
#endif /* HAVE_POLL_H */

#if defined(SOLARIS)
#include <poll.h>
#define HAVE_POLL_H
#endif /* SOLARIS */

#ifndef HAVE_POLL_H
#define POLLIN  0x0001
#define POLLOUT 0x0002
#define POLLPRI 0x0004
#endif /* HAVE_POLL_H */


/* defines for shutdown */
#define SD_RECEIVE 0
#define SD_SEND 1
#define SD_BOTH 2


/*
    oslSocketAddr is a pointer to a Berkeley struct sockaddr.
    I refrained from using sockaddr_in because of possible further
    extensions of this socket-interface (IP-NG?).
    The intention was to hide all Berkeley data-structures from
    direct access past the osl-interface.

    The current implementation is internet (IP) centered. All
    the constructor-functions (osl_create...) take parameters
    that will probably make sense only in the IP-environment
    (e.g. because of using the dotted-address-format).

    If the interface will be extended to host other protocol-
    families, I expect no externally visible changes in the
    existing functions. You'll probably need only new
    constructor-functions who take the different address
    formats into consideration (maybe a long dotted address
    or whatever).
*/

/* _Note_ that I rely on the fact that oslSocketAddr and struct sockaddr */
/* are the same! I don't like it very much but see no other easy way to  */
/* conceal the struct sockaddr from the eyes of the user. */


#define OSL_INVALID_SOCKET      -1
#define OSL_SOCKET_ERROR        -1


/* Buffer size for gethostbyname */
#define MAX_HOSTBUFFER_SIZE 2048

/*****************************************************************************/
/* enum oslAddrFamily */
/*****************************************************************************/

/* map */
static unsigned long FamilyMap[]= {
    AF_INET,                    /* osl_Socket_FamilyInet    */
    AF_IPX,                     /* osl_Socket_FamilyIpx     */
    0                           /* osl_Socket_FamilyInvalid */
};

/* reverse map */
static oslAddrFamily osl_AddrFamilyFromNative(sal_uInt32 nativeType)
{
    oslAddrFamily i= (oslAddrFamily)0;

    while(i != osl_Socket_FamilyInvalid)
    {
        if(FamilyMap[i] == nativeType)
            return i;
        i = (oslAddrFamily) ( i + 1 );
    }

    return i;
}

/* macros */
#define FAMILY_FROM_NATIVE(y) osl_AddrFamilyFromNative(y)
#define FAMILY_TO_NATIVE(x) (short)FamilyMap[x]

/*****************************************************************************/
/* enum oslProtocol */
/*****************************************************************************/

/* map */
static sal_uInt32 ProtocolMap[]= {
    0,                          /* osl_Socket_ProtocolIp      */
    NSPROTO_IPX,                /* osl_Socket_ProtocolIpx     */
    NSPROTO_SPX,                /* osl_Socket_ProtocolSpx     */
    NSPROTO_SPXII,              /* osl_Socket_ProtocolSpxII   */
    0                           /* osl_Socket_ProtocolInvalid */
};

/* reverse map */
/* mfe: NOT USED
static oslProtocol osl_ProtocolFromNative(sal_uInt32 nativeType)
{
    oslProtocol i= (oslProtocol)0;

    while(i != osl_Socket_ProtocolInvalid)
    {
        if(ProtocolMap[i] == nativeType)
            return i;
        i = (oslProtocol) ( i + 1);
    }

    return i;
}
*/

/* macros */
#define PROTOCOL_FROM_NATIVE(y) osl_ProtocolFromNative(y)
#define PROTOCOL_TO_NATIVE(x)   ProtocolMap[x]


/*****************************************************************************/
/* enum oslSocketType */
/*****************************************************************************/

/* map */
static sal_uInt32 TypeMap[]= {
    SOCK_STREAM,                /* osl_Socket_TypeStream    */
    SOCK_DGRAM,                 /* osl_Socket_TypeDgram     */
    SOCK_RAW,                   /* osl_Socket_TypeRaw       */
    SOCK_RDM,                   /* osl_Socket_TypeRdm       */
    SOCK_SEQPACKET,             /* osl_Socket_TypeSeqPacket */
    0                           /* osl_Socket_TypeInvalid   */
};

/* reverse map */
static oslSocketType osl_SocketTypeFromNative(sal_uInt32 nativeType)
{
    oslSocketType i= (oslSocketType)0;

    while(i != osl_Socket_TypeInvalid)
    {
        if(TypeMap[i] == nativeType)
            return i;
        i = (oslSocketType)(i + 1);
    }

    return i;
}

/* macros */
#define TYPE_TO_NATIVE(x)       TypeMap[x]
#define TYPE_FROM_NATIVE(y)     osl_SocketTypeFromNative(y)


/*****************************************************************************/
/* enum oslSocketOption */
/*****************************************************************************/

/* map */
static sal_uInt32 OptionMap[]= {
    SO_DEBUG,                   /* osl_Socket_OptionDebug       */
    SO_ACCEPTCONN,              /* osl_Socket_OptionAcceptConn  */
    SO_REUSEADDR,               /* osl_Socket_OptionReuseAddr   */
    SO_KEEPALIVE,               /* osl_Socket_OptionKeepAlive   */
    SO_DONTROUTE,               /* osl_Socket_OptionDontRoute   */
    SO_BROADCAST,               /* osl_Socket_OptionBroadcast   */
    SO_USELOOPBACK,             /* osl_Socket_OptionUseLoopback */
    SO_LINGER,                  /* osl_Socket_OptionLinger      */
    SO_OOBINLINE,               /* osl_Socket_OptionOOBinLine   */
    SO_SNDBUF,                  /* osl_Socket_OptionSndBuf      */
    SO_RCVBUF,                  /* osl_Socket_OptionRcvBuf      */
    SO_SNDLOWAT,                /* osl_Socket_OptionSndLowat    */
    SO_RCVLOWAT,                /* osl_Socket_OptionRcvLowat    */
    SO_SNDTIMEO,                /* osl_Socket_OptionSndTimeo    */
    SO_RCVTIMEO,                /* osl_Socket_OptionRcvTimeo    */
    SO_ERROR,                   /* osl_Socket_OptionError       */
    SO_TYPE,                    /* osl_Socket_OptionType        */
    TCP_NODELAY,                /* osl_Socket_OptionTcpNoDelay  */
    0                           /* osl_Socket_OptionInvalid     */
};

/* reverse map */
/* mfe: NOT USED
static oslSocketOption osl_SocketOptionFromNative(sal_uInt32 nativeType)
{
    oslSocketOption i= (oslSocketOption)0;

    while(i != osl_Socket_OptionInvalid)
    {
        if(OptionMap[i] == nativeType)
            return i;
        i = (oslSocketOption) ( i + 1 );
    }

    return i;
}
*/
/* macros */
#define OPTION_TO_NATIVE(x)     OptionMap[x]
#define OPTION_FROM_NATIVE(y)       osl_SocketOptionFromNative(y)


/*****************************************************************************/
/* enum oslSocketOptionLevel */
/*****************************************************************************/

static sal_uInt32 OptionLevelMap[]= {
    SOL_SOCKET,                 /* osl_Socket_LevelSocket  */
    IPPROTO_TCP,                /* osl_Socket_LevelTcp     */
    0                           /* osl_Socket_LevelInvalid */
};

/* reverse map */
/* mfe: NOT USED
static oslSocketOptionLevel osl_SocketOptionLevelFromNative(sal_uInt32 nativeType)
{
    oslSocketOptionLevel i= (oslSocketOptionLevel)0;

    while(i != osl_Socket_LevelInvalid)
    {
        if(OptionLevelMap[i] == nativeType)
            return i;
        i = (oslSocketOptionLevel) ( i + 1 );
    }

    return i;
}
*/
/* macros */
#define OPTION_LEVEL_TO_NATIVE(x)       OptionLevelMap[x]
#define OPTION_LEVEL_FROM_NATIVE(y)     osl_SocketOptionLevelFromNative(y)

/*****************************************************************************/
/* enum oslSocketMsgFlag */
/*****************************************************************************/

static sal_uInt32 SocketMsgFlagMap[]= {
    0,                          /* osl_Socket_MsgNormal    */
    MSG_OOB,                    /* osl_Socket_MsgOOB       */
    MSG_PEEK,                   /* osl_Socket_MsgPeek      */
    MSG_DONTROUTE,              /* osl_Socket_MsgDontRoute */
    MSG_MAXIOVLEN,              /* osl_Socket_MsgMaxIOVLen */
    0                           /* osl_Socket_MsgInvalid   */
};

/* reverse map */
/* mfe: NOT USED
static oslSocketMsgFlag osl_SocketMsgFlagFromNative(sal_uInt32 nativeType)
{
    oslSocketMsgFlag i= (oslSocketMsgFlag)0;

    while(i != osl_Socket_MsgInvalid)
    {
        if(SocketMsgFlagMap[i] == nativeType)
            return i;
        i = (oslSocketMsgFlag) ( i + 1 );
    }

    return i;
}
*/

/* macros */
#define MSG_FLAG_TO_NATIVE(x)       SocketMsgFlagMap[x]
#define MSG_FLAG_FROM_NATIVE(y)     osl_SocketMsgFlagFromNative(y)


/*****************************************************************************/
/* enum oslSocketDirection */
/*****************************************************************************/

static sal_uInt32 SocketDirection[]= {
    SD_RECEIVE,                 /* osl_Socket_DirRead      */
    SD_SEND,                    /* osl_Socket_DirWrite     */
    SD_BOTH,                    /* osl_Socket_DirReadWrite */
    0                           /* osl_Socket_DirInvalid   */
};

/* reverse map */
/* mfe: NOT USED
static oslSocketDirection osl_SocketDirectionFromNative(sal_uInt32 nativeType)
{
    oslSocketDirection i= (oslSocketDirection)0;

    while(i != osl_Socket_DirInvalid)
    {
        if(SocketDirection[i] == nativeType)
            return i;
        i = (oslSocketDirection) ( i + 1 );
    }

    return i;
}
*/

/* macros */
#define DIRECTION_TO_NATIVE(x)      SocketDirection[x]
#define DIRECTION_FROM_NATIVE(y)    osl_SocketDirectionFromNative(y)

/*****************************************************************************/
/* enum oslSocketError */
/*****************************************************************************/

static struct
{
    int            errcode;
    oslSocketError error;
} SocketError[]= {
    { 0,               osl_Socket_E_None              }, /* no error */
    { ENOTSOCK,        osl_Socket_E_NotSocket         }, /* Socket operation on non-socket */
    { EDESTADDRREQ,    osl_Socket_E_DestAddrReq       }, /* Destination address required */
    { EMSGSIZE,        osl_Socket_E_MsgSize           }, /* Message too long */
    { EPROTOTYPE,      osl_Socket_E_Prototype         }, /* Protocol wrong type for socket */
    { ENOPROTOOPT,     osl_Socket_E_NoProtocol        }, /* Protocol not available */
    { EPROTONOSUPPORT, osl_Socket_E_ProtocolNoSupport }, /* Protocol not supported */
    { ESOCKTNOSUPPORT, osl_Socket_E_TypeNoSupport     }, /* Socket type not supported */
    { EOPNOTSUPP,      osl_Socket_E_OpNotSupport      }, /* Operation not supported on socket */
    { EPFNOSUPPORT,    osl_Socket_E_PfNoSupport       }, /* Protocol family not supported */
    { EAFNOSUPPORT,    osl_Socket_E_AfNoSupport       }, /* Address family not supported by */
                                                            /* protocol family */
    { EADDRINUSE,      osl_Socket_E_AddrInUse         }, /* Address already in use */
    { EADDRNOTAVAIL,   osl_Socket_E_AddrNotAvail      }, /* Can't assign requested address */
    { ENETDOWN,        osl_Socket_E_NetDown           }, /* Network is down */
    { ENETUNREACH,     osl_Socket_E_NetUnreachable    }, /* Network is unreachable */
    { ENETRESET,       osl_Socket_E_NetReset          }, /* Network dropped connection because */
                                                             /* of reset */
    { ECONNABORTED,    osl_Socket_E_ConnAborted       }, /* Software caused connection abort */
    { ECONNRESET,      osl_Socket_E_ConnReset         }, /* Connection reset by peer */
    { ENOBUFS,         osl_Socket_E_NoBufferSpace     }, /* No buffer space available */
    { EISCONN,         osl_Socket_E_IsConnected       }, /* Socket is already connected */
    { ENOTCONN,        osl_Socket_E_NotConnected      }, /* Socket is not connected */
    { ESHUTDOWN,       osl_Socket_E_Shutdown          }, /* Can't send after socket shutdown */
    { ETOOMANYREFS,    osl_Socket_E_TooManyRefs       }, /* Too many references: can't splice */
    { ETIMEDOUT,       osl_Socket_E_TimedOut          }, /* Connection timed out */
    { ECONNREFUSED,    osl_Socket_E_ConnRefused       }, /* Connection refused */
    { EHOSTDOWN,       osl_Socket_E_HostDown          }, /* Host is down */
    { EHOSTUNREACH,    osl_Socket_E_HostUnreachable   }, /* No route to host */
    { EWOULDBLOCK,     osl_Socket_E_WouldBlock        }, /* call would block on non-blocking socket */
    { EALREADY,        osl_Socket_E_Already           }, /* operation already in progress */
    { EINPROGRESS,     osl_Socket_E_InProgress        }, /* operation now in progress */
    { EAGAIN,          osl_Socket_E_WouldBlock        }, /* same as EWOULDBLOCK */
    { -1,              osl_Socket_E_InvalidError      }
};

/* map */
/* mfe: NOT USED
static int osl_NativeFromSocketError(oslSocketError errorCode)
{
    int i = 0;

    while ((SocketError[i].error != osl_Socket_E_InvalidError) &&
           (SocketError[i].error != errorCode)) i++;

    return SocketError[i].errcode;
}
*/

/* reverse map */
static oslSocketError osl_SocketErrorFromNative(int nativeType)
{
    int i = 0;

    while ((SocketError[i].error != osl_Socket_E_InvalidError) &&
           (SocketError[i].errcode != nativeType)) i++;

    return SocketError[i].error;
}

/* macros */
#define ERROR_TO_NATIVE(x)      osl_NativeFromSocketError(x)
#define ERROR_FROM_NATIVE(y)    osl_SocketErrorFromNative(y)

/*****************************************************************************/
/* local function prototypes */
/*****************************************************************************/

oslSocketAddr SAL_CALL osl_psz_createInetSocketAddr (
    const sal_Char* pszDottedAddr, sal_Int32 Port);

oslSocketAddr SAL_CALL osl_psz_createIpxSocketAddr (
    const sal_Char NetNumber[4],
    const sal_Char NodeNumber[6],
    sal_uInt32 SocketNumber);

oslHostAddr SAL_CALL osl_psz_createHostAddr (
    const sal_Char *pszHostname, const oslSocketAddr Addr);

oslHostAddr SAL_CALL osl_psz_createHostAddrByName (
    const sal_Char *pszHostname);

const sal_Char* SAL_CALL osl_psz_getHostnameOfHostAddr (
    const oslHostAddr Addr);

oslSocketResult SAL_CALL osl_psz_getLocalHostname (
    sal_Char *pBuffer, sal_uInt32 nBufLen);

oslSocketAddr SAL_CALL osl_psz_resolveHostname (
    const sal_Char* pszHostname);

sal_Int32 SAL_CALL osl_psz_getServicePort (
    const sal_Char* pszServicename, const sal_Char* pszProtocol);

oslSocketResult SAL_CALL osl_psz_getHostnameOfSocketAddr (
    oslSocketAddr Addr, sal_Char *pBuffer, sal_uInt32 BufferSize);

oslSocketResult SAL_CALL osl_psz_getDottedInetAddrOfSocketAddr (
    oslSocketAddr Addr, sal_Char *pBuffer, sal_uInt32 BufferSize);

void SAL_CALL osl_psz_getLastSocketErrorDescription (
    oslSocket Socket, sal_Char* pBuffer, sal_uInt32 BufferSize);

/*****************************************************************************/
/* osl_create/destroy-SocketImpl */
/*****************************************************************************/

#if OSL_DEBUG_LEVEL > 1
static sal_uInt32 g_nSocketImpl = 0;
static sal_uInt32 g_nSocketAddr = 0;

/* sorry, must be implemented otherwise */

#endif /* OSL_DEBUG_LEVEL */


oslSocket __osl_createSocketImpl(int Socket)
{
    oslSocket pSocket;

    pSocket = (oslSocket)calloc(1, sizeof(struct oslSocketImpl));

    pSocket->m_Socket = Socket;
    pSocket->m_nLastError = 0;
    pSocket->m_CloseCallback = 0;
    pSocket->m_CallbackArg = 0;
    pSocket->m_nRefCount = 1;

#if defined(LINUX)
    pSocket->m_bIsAccepting = sal_False;
#endif

#if OSL_DEBUG_LEVEL > 1
    g_nSocketImpl ++;
#endif
    return pSocket;
}

void __osl_destroySocketImpl(oslSocket Socket)
{
    if ( Socket != NULL)
        free((struct oslSocketImpl *) Socket);
#if OSL_DEBUG_LEVEL > 1
    g_nSocketImpl --;
#endif
}

static oslSocketAddr __osl_createSocketAddr(  )
{
    oslSocketAddr pAddr = (oslSocketAddr) rtl_allocateZeroMemory( sizeof( struct oslSocketAddrImpl ));
#if OSL_DEBUG_LEVEL > 1
    g_nSocketAddr ++;
#endif
    return pAddr;
}

static oslSocketAddr __osl_createSocketAddrWithFamily(
    oslAddrFamily family, sal_Int32 port, sal_uInt32 nAddr )
{
    oslSocketAddr pAddr;

    OSL_ASSERT( family == osl_Socket_FamilyInet );

    pAddr = __osl_createSocketAddr();
    switch( family )
    {
    case osl_Socket_FamilyInet:
    {
        struct sockaddr_in* pInetAddr= (struct sockaddr_in*)&(pAddr->m_sockaddr);

        pInetAddr->sin_family = FAMILY_TO_NATIVE(osl_Socket_FamilyInet);
        pInetAddr->sin_addr.s_addr = nAddr;
        pInetAddr->sin_port = (sal_uInt16)(port&0xffff);
        break;
       }
    default:
        pAddr->m_sockaddr.sa_family = FAMILY_TO_NATIVE(family);
    }
    return pAddr;
}

static oslSocketAddr __osl_createSocketAddrFromSystem( struct sockaddr *pSystemSockAddr )
{
    oslSocketAddr pAddr = __osl_createSocketAddr();
    memcpy( &(pAddr->m_sockaddr), pSystemSockAddr, sizeof( struct sockaddr ) );
    return pAddr;
}

static void __osl_destroySocketAddr( oslSocketAddr addr )
{
#if OSL_DEBUG_LEVEL > 1
    g_nSocketAddr --;
#endif
    rtl_freeMemory( addr );
}

/*****************************************************************************/
/* osl_createEmptySocketAddr */
/*****************************************************************************/
oslSocketAddr SAL_CALL osl_createEmptySocketAddr(oslAddrFamily Family)
{
    oslSocketAddr pAddr = 0;

    /* is it an internet-Addr? */
    if (Family == osl_Socket_FamilyInet)
    {
        pAddr = __osl_createSocketAddrWithFamily(Family, 0 , htonl(INADDR_ANY) );
    }
    else
    {
        pAddr = __osl_createSocketAddrWithFamily( Family , 0 , 0 );
    }

    return pAddr;
}

/*****************************************************************************/
/* osl_copySocketAddr */
/*****************************************************************************/
oslSocketAddr SAL_CALL osl_copySocketAddr(oslSocketAddr Addr)
{
    oslSocketAddr pCopy = 0;
    if (Addr)
    {
        pCopy = __osl_createSocketAddr();

        if (pCopy)
            memcpy(&(pCopy->m_sockaddr),&(Addr->m_sockaddr), sizeof(struct sockaddr));
    }
    return pCopy;
}

/*****************************************************************************/
/* osl_isEqualSocketAddr */
/*****************************************************************************/
sal_Bool SAL_CALL osl_isEqualSocketAddr (
    oslSocketAddr Addr1,
    oslSocketAddr Addr2)
{
    struct sockaddr* pAddr1 = NULL;
    struct sockaddr* pAddr2 = NULL;

    OSL_ASSERT(Addr1);
    OSL_ASSERT(Addr2);
    pAddr1 = &(Addr1->m_sockaddr);
    pAddr2 = &(Addr2->m_sockaddr);

    OSL_ASSERT(pAddr1);
    OSL_ASSERT(pAddr2);
    if (pAddr1 == pAddr2)
    {
        return (sal_True);
    }

    if (pAddr1->sa_family == pAddr2->sa_family)
    {
        switch (pAddr1->sa_family)
        {
            case AF_INET:
            {
                struct sockaddr_in* pInetAddr1= (struct sockaddr_in*)pAddr1;
                struct sockaddr_in* pInetAddr2= (struct sockaddr_in*)pAddr2;

                if ((pInetAddr1->sin_family == pInetAddr2->sin_family) &&
                    (pInetAddr1->sin_addr.s_addr == pInetAddr2->sin_addr.s_addr) &&
                    (pInetAddr1->sin_port == pInetAddr2->sin_port))
                    return (sal_True);
            }

            default:
            {
                return (memcmp(pAddr1, pAddr2, sizeof(struct sockaddr)) == 0);
            }
        }
    }

    return (sal_False);
}

/*****************************************************************************/
/* osl_createInetBroadcastAddr */
/*****************************************************************************/
oslSocketAddr SAL_CALL osl_createInetBroadcastAddr (
    rtl_uString *strDottedAddr,
    sal_Int32    Port)
{
    sal_uInt32    nAddr = OSL_INADDR_NONE;
    oslSocketAddr pAddr;

    if (strDottedAddr && strDottedAddr->length)
    {
        /* Dotted host address for limited broadcast */
        rtl_String *pDottedAddr = NULL;

        rtl_uString2String (
            &pDottedAddr, strDottedAddr->buffer, strDottedAddr->length,
            RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);

        nAddr = inet_addr (pDottedAddr->buffer);
        rtl_string_release (pDottedAddr);
    }

    if (nAddr != OSL_INADDR_NONE)
    {
        /* Limited broadcast */
        nAddr = ntohl(nAddr);
        if (IN_CLASSA(nAddr))
        {
            nAddr &= IN_CLASSA_NET;
            nAddr |= IN_CLASSA_HOST;
        }
        else if (IN_CLASSB(nAddr))
        {
            nAddr &= IN_CLASSB_NET;
            nAddr |= IN_CLASSB_HOST;
        }
        else if (IN_CLASSC(nAddr))
        {
            nAddr &= IN_CLASSC_NET;
            nAddr |= IN_CLASSC_HOST;
        }
        else
        {
            /* No broadcast in class D */
            return ((oslSocketAddr)NULL);
        }
        nAddr = htonl(nAddr);
    }

    pAddr = __osl_createSocketAddrWithFamily( osl_Socket_FamilyInet, htons(Port), nAddr );
    return pAddr;
}

/*****************************************************************************/
/* osl_createInetSocketAddr */
/*****************************************************************************/
oslSocketAddr SAL_CALL osl_createInetSocketAddr (
    rtl_uString *ustrDottedAddr,
    sal_Int32    Port)
{
    rtl_String* strDottedAddr=0;
    oslSocketAddr Addr;
    sal_Char* pszDottedAddr=0;

    if ( ustrDottedAddr != 0 )
    {
        rtl_uString2String( &strDottedAddr,
                            rtl_uString_getStr(ustrDottedAddr),
                            rtl_uString_getLength(ustrDottedAddr),
                            RTL_TEXTENCODING_UTF8,
                            OUSTRING_TO_OSTRING_CVTFLAGS);
        pszDottedAddr = rtl_string_getStr(strDottedAddr);
    }


    Addr = osl_psz_createInetSocketAddr(pszDottedAddr, Port);

    if ( strDottedAddr != 0 )
    {
        rtl_string_release(strDottedAddr);
    }

    return Addr;
}

oslSocketAddr SAL_CALL osl_psz_createInetSocketAddr (
    const sal_Char* pszDottedAddr,
    sal_Int32       Port)
{
    oslSocketAddr pAddr = 0;
    sal_Int32 Addr = inet_addr(pszDottedAddr);
    if(Addr != -1)
    {
        /* valid dotted addr */
        pAddr = __osl_createSocketAddrWithFamily( osl_Socket_FamilyInet, htons(Port) , Addr );
    }
    return pAddr;
}

/*****************************************************************************/
/* osl_setAddrOfSocketAddr */
/*****************************************************************************/
oslSocketResult SAL_CALL osl_setAddrOfSocketAddr( oslSocketAddr pAddr, sal_Sequence *pByteSeq )
{
    oslSocketResult res = osl_Socket_Error;

    OSL_ASSERT( pAddr );
    OSL_ASSERT( pByteSeq );

    if( pAddr && pByteSeq )
    {
        struct sockaddr_in * pSystemInetAddr;

        OSL_ASSERT( pAddr->m_sockaddr.sa_family == FAMILY_TO_NATIVE( osl_Socket_FamilyInet ) );
        OSL_ASSERT( pByteSeq->nElements == 4 );

        pSystemInetAddr = (struct sockaddr_in * ) &(pAddr->m_sockaddr);
        memcpy( &(pSystemInetAddr->sin_addr) , pByteSeq->elements , 4 );
        res = osl_Socket_Ok;
    }
    return res;
}

/*****************************************************************************/
/* osl_getAddrOfSocketAddr */
/*****************************************************************************/
oslSocketResult SAL_CALL osl_getAddrOfSocketAddr( oslSocketAddr pAddr, sal_Sequence **ppByteSeq )
{
    oslSocketResult res = osl_Socket_Error;

    OSL_ASSERT( pAddr );
    OSL_ASSERT( ppByteSeq );

    if( pAddr && ppByteSeq )
    {
        struct sockaddr_in * pSystemInetAddr = (struct sockaddr_in * ) &(pAddr->m_sockaddr);
        rtl_byte_sequence_constructFromArray( ppByteSeq , (sal_Int8 *) &(pSystemInetAddr->sin_addr),4);
        res = osl_Socket_Ok;
    }
    return res;
}


/*****************************************************************************/
/* _osl_getFullQualifiedDomainName */
/*****************************************************************************/

/** try to figure out a full-qualified hostname, by adding the current domain
    as given by the domainname program to the given hostname.
    This function MUST NOT call gethostbyname since pHostName allready points
    to data returned by gethostname and would be garbled: use gethostname_r
    instead!
 */

/* wrap around different interfaces to reentrant gethostbyname */
static struct hostent* _osl_gethostbyname_r (
    const char *name, struct hostent *result,
    char *buffer, int buflen, int *h_errnop)
{
#if defined(LINUX) || defined(ANDROID) || (defined(FREEBSD) && (__FreeBSD_version >= 601103)) || defined(DRAGONFLY)
    struct hostent *__result; /* will be the same as result */
    int __error;
    __error = gethostbyname_r (name, result, buffer, buflen,
                 &__result, h_errnop);
    return __error ? NULL : __result ;
#elif defined(AIX)
    *h_errnop = gethostbyname_r (name, result, (struct hostent_data *)buffer);
    (void)buflen;
    return *h_errnop ? NULL : result ;
#else
    return gethostbyname_r( name, result, buffer, buflen, h_errnop);
#endif
}

static sal_Char* _osl_getFullQualifiedDomainName (const sal_Char *pHostName)
{
#   define DOMAINNAME_LENGTH 512
    struct hostent  aHostByName;
    struct hostent *pHostByName;
    sal_Char        pQualifiedHostBuffer[ MAX_HOSTBUFFER_SIZE ];
    sal_Char  *pFullQualifiedName = NULL;
    int     nErrorNo;

    pHostByName = _osl_gethostbyname_r (
        pHostName,
        &aHostByName, pQualifiedHostBuffer,
        sizeof(pQualifiedHostBuffer), &nErrorNo );
    if (pHostByName != NULL)
    {
        pFullQualifiedName = strdup(pHostByName->h_name);
    }
    return pFullQualifiedName;
}
/*****************************************************************************/
/* _osl_isFullQualifiedDomainName */
/*****************************************************************************/
static sal_Bool _osl_isFullQualifiedDomainName (const sal_Char *pHostName)
{
    /* a FQDN (aka 'hostname.domain.top_level_domain' )
     * is a name which contains a dot '.' in it ( would
     * match as well for 'hostname.' but is good enough
     * for now )*/
    return (sal_Bool)( strchr( pHostName, (int)'.' ) != NULL );
}

/*****************************************************************************/
/* oslHostAddr */
/*****************************************************************************/
struct oslHostAddrImpl
{
    sal_Char        *pHostName;
    oslSocketAddr   pSockAddr;
};

static oslHostAddr _osl_hostentToHostAddr (const struct hostent *he)
{
    oslHostAddr pAddr= NULL;
    oslSocketAddr pSockAddr = 0;

    sal_Char        *cn;

    if ((he == NULL) || (he->h_name == NULL) || (he->h_addr_list[0] == NULL))
        return ((oslHostAddr)NULL);

    if (_osl_isFullQualifiedDomainName(he->h_name))
    {
        cn= (sal_Char *)malloc(strlen (he->h_name) + 1);
        OSL_ASSERT(cn);
        if (cn == NULL)
            return ((oslHostAddr)NULL);

        strcpy(cn, he->h_name);
    }
    else
    {
        cn =_osl_getFullQualifiedDomainName (he->h_name);
        OSL_ASSERT(cn);
        if (cn == NULL)
            return ((oslHostAddr)NULL);
    }

    pSockAddr = __osl_createSocketAddr();
    OSL_ASSERT(pSockAddr);
    if (pSockAddr == NULL)
    {
        free(cn);
        return ((oslHostAddr)NULL);
    }

    pSockAddr->m_sockaddr.sa_family= he->h_addrtype;
    if (pSockAddr->m_sockaddr.sa_family == FAMILY_TO_NATIVE(osl_Socket_FamilyInet))
    {
        struct sockaddr_in *sin= (struct sockaddr_in *)&(pSockAddr->m_sockaddr);
        memcpy (
            &(sin->sin_addr.s_addr),
            he->h_addr_list[0],
            he->h_length);
    }
    else
    {
        /* unknown address family */
        /* future extensions for new families might be implemented here */

        OSL_TRACE("_osl_hostentToHostAddr: unknown address family.");
        OSL_ASSERT(sal_False);

        __osl_destroySocketAddr( pSockAddr );
        free (cn);
        return ((oslHostAddr)NULL);
    }

    pAddr= (oslHostAddr) malloc(sizeof(struct oslHostAddrImpl));
    OSL_ASSERT(pAddr);
    if (pAddr == NULL)
    {
        __osl_destroySocketAddr( pSockAddr );
        free (cn);
        return ((oslHostAddr)NULL);
    }

    pAddr->pHostName= cn;
    pAddr->pSockAddr= pSockAddr;

    return pAddr;
}

/*****************************************************************************/
/* osl_createHostAddr */
/*****************************************************************************/
oslHostAddr SAL_CALL osl_createHostAddr (
    rtl_uString        *ustrHostname,
    const oslSocketAddr Addr)
{
    oslHostAddr HostAddr;
    rtl_String* strHostname=0;
    sal_Char* pszHostName=0;

    if ( ustrHostname != 0 )
    {
        rtl_uString2String( &strHostname,
                            rtl_uString_getStr(ustrHostname),
                            rtl_uString_getLength(ustrHostname),
                            RTL_TEXTENCODING_UTF8,
                            OUSTRING_TO_OSTRING_CVTFLAGS );
        pszHostName = rtl_string_getStr(strHostname);
    }

    HostAddr = osl_psz_createHostAddr(pszHostName,Addr);

    if ( strHostname != 0 )
    {
        rtl_string_release(strHostname);
    }

    return HostAddr;
}

oslHostAddr SAL_CALL osl_psz_createHostAddr (
    const sal_Char     *pszHostname,
    const oslSocketAddr pAddr)
{
    oslHostAddr pHostAddr;
    sal_Char            *cn;

    OSL_ASSERT(pszHostname && pAddr);
    if ((pszHostname == NULL) || (pAddr == NULL))
        return ((oslHostAddr)NULL);

    cn = (sal_Char *)malloc(strlen (pszHostname) + 1);
    OSL_ASSERT(cn);
    if (cn == NULL)
        return ((oslHostAddr)NULL);

    strcpy (cn, pszHostname);

    pHostAddr= (oslHostAddr) malloc(sizeof(struct oslHostAddrImpl));
    OSL_ASSERT(pHostAddr);
    if (pHostAddr == NULL)
    {
        free (cn);
        return ((oslHostAddr)NULL);
    }

    pHostAddr->pHostName= cn;
    pHostAddr->pSockAddr= osl_copySocketAddr( pAddr );

    return pHostAddr;
}

/*****************************************************************************/
/* osl_createHostAddrByName */
/*****************************************************************************/
oslHostAddr SAL_CALL osl_createHostAddrByName(rtl_uString *ustrHostname)
{
    oslHostAddr HostAddr;
    rtl_String* strHostname=0;
    sal_Char* pszHostName=0;

    if ( ustrHostname != 0 )
    {
        rtl_uString2String( &strHostname,
                            rtl_uString_getStr(ustrHostname),
                            rtl_uString_getLength(ustrHostname),
                            RTL_TEXTENCODING_UTF8,
                            OUSTRING_TO_OSTRING_CVTFLAGS );
        pszHostName=rtl_string_getStr(strHostname);
    }

    HostAddr = osl_psz_createHostAddrByName(pszHostName);

    if ( strHostname != 0 )
    {
        rtl_string_release(strHostname);
    }

    return HostAddr;
}

oslHostAddr SAL_CALL osl_psz_createHostAddrByName (const sal_Char *pszHostname)
{
    struct      hostent  aHe;
    struct      hostent *pHe;
    sal_Char    heBuffer[ MAX_HOSTBUFFER_SIZE ];
    int         nErrorNo;

    pHe = _osl_gethostbyname_r (
        pszHostname,
        &aHe, heBuffer,
        sizeof(heBuffer), &nErrorNo );

    return _osl_hostentToHostAddr (pHe);
}

/*****************************************************************************/
/* osl_createHostAddrByAddr */
/*****************************************************************************/
oslHostAddr SAL_CALL osl_createHostAddrByAddr (const oslSocketAddr pAddr)
{
    OSL_ASSERT(pAddr);

    if (pAddr == NULL)
        return ((oslHostAddr)NULL);

    if (pAddr->m_sockaddr.sa_family == FAMILY_TO_NATIVE(osl_Socket_FamilyInet))
    {
        const struct sockaddr_in *sin= (const struct sockaddr_in *)&(pAddr->m_sockaddr);
        struct hostent *he;

        if (sin->sin_addr.s_addr == htonl(INADDR_ANY))
            return ((oslHostAddr)NULL);

        he= gethostbyaddr((sal_Char *)&(sin->sin_addr),
                          sizeof (sin->sin_addr),
                          sin->sin_family);
        return _osl_hostentToHostAddr (he);
    }

    return ((oslHostAddr)NULL);
}

/*****************************************************************************/
/* osl_copyHostAddr */
/*****************************************************************************/
oslHostAddr SAL_CALL osl_copyHostAddr (const oslHostAddr pAddr)
{
    OSL_ASSERT(pAddr);

    if (pAddr)
        return osl_psz_createHostAddr (pAddr->pHostName, pAddr->pSockAddr);
    else
        return ((oslHostAddr)NULL);
}

/*****************************************************************************/
/* osl_getHostnameOfHostAddr */
/*****************************************************************************/
void SAL_CALL osl_getHostnameOfHostAddr (
    const oslHostAddr   Addr,
    rtl_uString       **ustrHostname)
{
    const sal_Char* pHostname=0;

    pHostname = osl_psz_getHostnameOfHostAddr(Addr);

    rtl_uString_newFromAscii (ustrHostname, pHostname);

    return;
}

const sal_Char* SAL_CALL osl_psz_getHostnameOfHostAddr (const oslHostAddr pAddr)
{
    if (pAddr)
        return pAddr->pHostName;
    else
        return NULL;
}

/*****************************************************************************/
/* osl_getSocketAddrOfHostAddr */
/*****************************************************************************/
oslSocketAddr SAL_CALL osl_getSocketAddrOfHostAddr (const oslHostAddr pAddr)
{
    OSL_ASSERT(pAddr);

    if (pAddr)
        return ((oslSocketAddr)(pAddr->pSockAddr));
    else
        return NULL;
}

/*****************************************************************************/
/* osl_destroyHostAddr */
/*****************************************************************************/
void SAL_CALL osl_destroyHostAddr (oslHostAddr pAddr)
{
    if (pAddr)
    {
        if (pAddr->pHostName)
            free (pAddr->pHostName);
        if (pAddr->pSockAddr)
            osl_destroySocketAddr (pAddr->pSockAddr);
        free (pAddr);
    }
}

/*****************************************************************************/
/* osl_getLocalHostname */
/*****************************************************************************/
oslSocketResult SAL_CALL osl_getLocalHostname(rtl_uString **ustrLocalHostname)
{
    oslSocketResult Result;
    sal_Char pszHostname[1024];

    pszHostname[0] = '\0';

    Result = osl_psz_getLocalHostname(pszHostname,sizeof(pszHostname));

    rtl_uString_newFromAscii(ustrLocalHostname,pszHostname);

    return Result;
}

oslSocketResult SAL_CALL osl_psz_getLocalHostname (
    sal_Char *pBuffer, sal_uInt32 nBufLen)
{
    static sal_Char LocalHostname[256] = "";

    if (strlen(LocalHostname) == 0)
    {
        const sal_Char *pStr;

#ifdef SYSV
        struct utsname uts;

        if (uname(&uts) < 0)
            return osl_Socket_Error;

        if ((strlen(uts.nodename) + 1) > nBufLen)
            return osl_Socket_Error;

        strncpy(LocalHostname, uts.nodename, sizeof( LocalHostname ));
#else  /* BSD compatible */

        if (gethostname(LocalHostname, sizeof(LocalHostname)-1) != 0)
            return osl_Socket_Error;
        LocalHostname[sizeof(LocalHostname)-1] = 0;
#endif /* SYSV */

        /* check if we have an FQDN */
        if (strchr(LocalHostname, '.') == NULL)
        {
            oslHostAddr Addr;

            /* no, determine it via dns */
            Addr = osl_psz_createHostAddrByName(LocalHostname);

            if ((pStr = osl_psz_getHostnameOfHostAddr(Addr)) != NULL)
            {
                strcpy(LocalHostname, pStr);
            }
            osl_destroyHostAddr(Addr);
        }
    }

    if (strlen(LocalHostname) > 0)
    {
        strncpy(pBuffer, LocalHostname, nBufLen);
        pBuffer[nBufLen - 1] = '\0';

        return osl_Socket_Ok;
    }

    return osl_Socket_Error;
}

/*****************************************************************************/
/* osl_resolveHostname */
/*****************************************************************************/
oslSocketAddr SAL_CALL osl_resolveHostname(rtl_uString *ustrHostname)
{
    oslSocketAddr Addr;
    rtl_String* strHostname=0;
    sal_Char* pszHostName=0;

    if ( ustrHostname != 0 )
    {
        rtl_uString2String( &strHostname,
                            rtl_uString_getStr(ustrHostname),
                            rtl_uString_getLength(ustrHostname),
                            RTL_TEXTENCODING_UTF8,
                            OUSTRING_TO_OSTRING_CVTFLAGS );
        pszHostName = rtl_string_getStr(strHostname);
    }


    Addr = osl_psz_resolveHostname(pszHostName);

    if ( strHostname != 0 )
    {
        rtl_string_release(strHostname);
    }


    return Addr;
}


oslSocketAddr SAL_CALL osl_psz_resolveHostname(const sal_Char* pszHostname)
{
    struct oslHostAddrImpl *pAddr = (oslHostAddr)osl_psz_createHostAddrByName(pszHostname);

    if (pAddr)
    {
        oslSocketAddr SockAddr = osl_copySocketAddr(pAddr->pSockAddr);

        osl_destroyHostAddr(pAddr);

        return (SockAddr);
    }

    return ((oslSocketAddr)NULL);
}

/*****************************************************************************/
/* osl_getServicePort */
/*****************************************************************************/
sal_Int32 SAL_CALL osl_getServicePort(rtl_uString *ustrServicename, rtl_uString *ustrProtocol)
{
    sal_Int32 nPort;
    rtl_String* strServicename=0;
    rtl_String* strProtocol=0;
    sal_Char* pszServiceName=0;
    sal_Char* pszProtocol=0;

    if ( ustrServicename != 0 )
    {
        rtl_uString2String( &strServicename,
                            rtl_uString_getStr(ustrServicename),
                            rtl_uString_getLength(ustrServicename),
                            RTL_TEXTENCODING_UTF8,
                            OUSTRING_TO_OSTRING_CVTFLAGS );
        pszServiceName = rtl_string_getStr(strServicename);
    }

    if ( ustrProtocol != 0 )
    {
        rtl_uString2String( &strProtocol,
                            rtl_uString_getStr(ustrProtocol),
                            rtl_uString_getLength(ustrProtocol),
                            RTL_TEXTENCODING_UTF8,
                            OUSTRING_TO_OSTRING_CVTFLAGS );
        pszProtocol = rtl_string_getStr(strProtocol);
    }

    nPort = osl_psz_getServicePort(pszServiceName,pszProtocol);

    if ( strServicename != 0 )
    {
        rtl_string_release(strServicename);
    }

    if ( strProtocol != 0 )
    {
        rtl_string_release(strProtocol);
    }


    return nPort;
}


sal_Int32 SAL_CALL osl_psz_getServicePort(const sal_Char* pszServicename,
                        const sal_Char* pszProtocol)
{
    struct servent* ps;

    ps= getservbyname(pszServicename, pszProtocol);

    if (ps != 0)
        return ntohs(ps->s_port);

    return OSL_INVALID_PORT;
}

/*****************************************************************************/
/* osl_destroySocketAddr */
/*****************************************************************************/
void SAL_CALL osl_destroySocketAddr(oslSocketAddr pAddr)
{
    __osl_destroySocketAddr( pAddr );
}

/*****************************************************************************/
/* osl_getFamilyOfSocketAddr */
/*****************************************************************************/
oslAddrFamily SAL_CALL osl_getFamilyOfSocketAddr(oslSocketAddr pAddr)
{
    OSL_ASSERT(pAddr);

    if (pAddr)
        return FAMILY_FROM_NATIVE(pAddr->m_sockaddr.sa_family);
    else
        return osl_Socket_FamilyInvalid;
}

/*****************************************************************************/
/* osl_getInetPortOfSocketAddr */
/*****************************************************************************/
sal_Int32 SAL_CALL osl_getInetPortOfSocketAddr(oslSocketAddr pAddr)
{
    OSL_ASSERT(pAddr);
    if( pAddr )
    {
        struct sockaddr_in* pSystemInetAddr= (struct sockaddr_in*)&(pAddr->m_sockaddr);

        if ( pSystemInetAddr->sin_family == FAMILY_TO_NATIVE(osl_Socket_FamilyInet))
            return ntohs(pSystemInetAddr->sin_port);
    }
    return OSL_INVALID_PORT;
}

/*****************************************************************************/
/* osl_setInetPortOfSocketAddr */
/*****************************************************************************/
sal_Bool SAL_CALL osl_setInetPortOfSocketAddr(oslSocketAddr pAddr, sal_Int32 Port)
{
    OSL_ASSERT(pAddr);
    if( pAddr )
    {
        struct sockaddr_in* pSystemInetAddr= (struct sockaddr_in*)&(pAddr->m_sockaddr);
        if ( pSystemInetAddr->sin_family == FAMILY_TO_NATIVE(osl_Socket_FamilyInet))
        {
            pSystemInetAddr->sin_port= htons((short)Port);
            return sal_True;
        }
    }

    /* this is not a inet-addr => can't set port */
    return sal_False;
}

/*****************************************************************************/
/* osl_getHostnameOfSocketAddr */
/*****************************************************************************/
oslSocketResult SAL_CALL osl_getHostnameOfSocketAddr(oslSocketAddr Addr, rtl_uString **ustrHostname)
{
    oslSocketResult Result;
    sal_Char pszHostname[1024];

    pszHostname[0] = '\0';

    Result = osl_psz_getHostnameOfSocketAddr(Addr,pszHostname,sizeof(pszHostname));

    rtl_uString_newFromAscii(ustrHostname,pszHostname);

    return Result;
}


oslSocketResult SAL_CALL osl_psz_getHostnameOfSocketAddr(oslSocketAddr pAddr,
                                            sal_Char *pBuffer, sal_uInt32 BufferSize)
{
    oslHostAddr pHostAddr= (oslHostAddr )osl_createHostAddrByAddr(pAddr);

    if (pHostAddr)
    {
        strncpy(pBuffer, pHostAddr->pHostName, BufferSize);

        pBuffer[BufferSize - 1] = '\0';

        osl_destroyHostAddr(pHostAddr);

        return osl_Socket_Ok;
    }

    return osl_Socket_Error;
}

/*****************************************************************************/
/* osl_getDottedInetAddrOfSocketAddr */
/*****************************************************************************/
oslSocketResult SAL_CALL osl_getDottedInetAddrOfSocketAddr(oslSocketAddr Addr, rtl_uString **ustrDottedInetAddr)
{
    oslSocketResult Result;
    sal_Char pszDottedInetAddr[1024];

    pszDottedInetAddr[0] = '\0';

    Result = osl_psz_getDottedInetAddrOfSocketAddr(Addr,pszDottedInetAddr,sizeof(pszDottedInetAddr));

    rtl_uString_newFromAscii(ustrDottedInetAddr,pszDottedInetAddr);

    return Result;

}

oslSocketResult SAL_CALL osl_psz_getDottedInetAddrOfSocketAddr(oslSocketAddr pAddr,
                                                  sal_Char *pBuffer, sal_uInt32 BufferSize)
{
    OSL_ASSERT(pAddr);

    if( pAddr )
    {
        struct sockaddr_in* pSystemInetAddr = ( struct sockaddr_in * ) &(pAddr->m_sockaddr);

        if (pSystemInetAddr->sin_family == FAMILY_TO_NATIVE(osl_Socket_FamilyInet))
        {
            strncpy(pBuffer, inet_ntoa(pSystemInetAddr->sin_addr), BufferSize);
            pBuffer[BufferSize - 1] = '\0';

            return osl_Socket_Ok;
        }
    }

    return osl_Socket_Error;
}

/*****************************************************************************/
/* osl_createSocket  */
/*****************************************************************************/
oslSocket SAL_CALL osl_createSocket(oslAddrFamily   Family,
                           oslSocketType    Type,
                           oslProtocol      Protocol)
{
    int            Flags;
    oslSocket pSocket;

    /* alloc memory */
    pSocket= __osl_createSocketImpl(OSL_INVALID_SOCKET);

    /* create socket */
    pSocket->m_Socket= socket(FAMILY_TO_NATIVE(Family),
                                TYPE_TO_NATIVE(Type),
                                PROTOCOL_TO_NATIVE(Protocol));

    /* creation failed => free memory */
    if(pSocket->m_Socket == OSL_INVALID_SOCKET)
    {
        OSL_TRACE("osl_createSocket failed. Errno: %d; %s\n",
                  errno,
                  strerror(errno));

        __osl_destroySocketImpl((pSocket));
        pSocket= 0;
    }
    else
    {
        /* set close-on-exec flag */
        if ((Flags = fcntl(pSocket->m_Socket, F_GETFD, 0)) != -1)
        {
            Flags |= FD_CLOEXEC;
            if (fcntl(pSocket->m_Socket, F_SETFD, Flags) == -1)
            {
                pSocket->m_nLastError=errno;
                OSL_TRACE("osl_createSocket failed changing socket flags. Errno: %d; %s\n",
                          errno,
                          strerror(errno));
            }
        }
        else
        {
            pSocket->m_nLastError=errno;
        }


        pSocket->m_CloseCallback    = NULL;
        pSocket->m_CallbackArg  = NULL;
    }

    return pSocket;
}

void SAL_CALL osl_acquireSocket(oslSocket pSocket)
{
    osl_atomic_increment( &(pSocket->m_nRefCount ) );
}

void SAL_CALL osl_releaseSocket( oslSocket pSocket )
{
    if( pSocket && 0 == osl_atomic_decrement( &(pSocket->m_nRefCount) ) )
    {
#if defined(LINUX)
    if ( pSocket->m_bIsAccepting == sal_True )
    {
        OSL_FAIL("osl_destroySocket : attempt to destroy socket while accepting\n");
        return;
    }
#endif /* LINUX */
        osl_closeSocket( pSocket );
        __osl_destroySocketImpl( pSocket );
    }
}



/*****************************************************************************/
/* osl_closeSocket  */
/*****************************************************************************/
void SAL_CALL osl_closeSocket(oslSocket pSocket)
{
    int nRet;
    int nFD;

    /* socket already invalid */
    if(pSocket==0)
        return;

    pSocket->m_nLastError=0;
    nFD = pSocket->m_Socket;

    if (nFD == OSL_INVALID_SOCKET)
        return;

    pSocket->m_Socket = OSL_INVALID_SOCKET;

#if defined(LINUX)
    pSocket->m_bIsInShutdown = sal_True;

    if ( pSocket->m_bIsAccepting == sal_True )
    {
        int nConnFD;
        union {
            struct sockaddr aSockAddr;
            struct sockaddr_in aSockAddrIn;
        } s;
        socklen_t nSockLen = sizeof(s.aSockAddr);

        nRet = getsockname(nFD, &s.aSockAddr, &nSockLen);
        if ( nRet < 0 )
        {
            OSL_TRACE("getsockname call failed with error: %s", strerror(errno));
        }

        if ( s.aSockAddr.sa_family == AF_INET )
        {
            if ( s.aSockAddrIn.sin_addr.s_addr == htonl(INADDR_ANY) )
            {
                s.aSockAddrIn.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
            }

            nConnFD = socket(AF_INET, SOCK_STREAM, 0);
            if ( nConnFD < 0 )
            {
                OSL_TRACE("socket call failed with error: %s", strerror(errno));
            }

            nRet = connect(nConnFD, &s.aSockAddr, sizeof(s.aSockAddr));
            if ( nRet < 0 )
            {
                OSL_TRACE("connect call failed with error: %s", strerror(errno));
            }
            close(nConnFD);
        }
        pSocket->m_bIsAccepting = sal_False;
    }
#endif /* LINUX */

    /* registrierten Callback ausfuehren */
    if (pSocket->m_CloseCallback != NULL)
    {
        pSocket->m_CloseCallback(pSocket->m_CallbackArg);
    }

    nRet=close(nFD);
    if ( nRet != 0 )
    {
        pSocket->m_nLastError=errno;
        OSL_TRACE("closeSocket close error '%s'",strerror(errno));
    }

    pSocket->m_Socket = OSL_INVALID_SOCKET;
}

/*****************************************************************************/
/* osl_getLocalAddrOfSocket  */
/* Note that I rely on the fact that oslSocketAddr and struct sockaddr */
/* are the same! I don't like it very much but see no other easy way to conceal */
/* the struct sockaddr from the eyes of the user. */
/*****************************************************************************/
oslSocketAddr SAL_CALL osl_getLocalAddrOfSocket(oslSocket pSocket)
{
    socklen_t AddrLen;
    struct sockaddr Addr;
    oslSocketAddr  pAddr;

    if (pSocket == NULL) /* ENOTSOCK */
        return ((oslSocketAddr)NULL);

    AddrLen= sizeof(struct sockaddr);

    if (getsockname(pSocket->m_Socket, &Addr, &AddrLen) == OSL_SOCKET_ERROR)
        return ((oslSocketAddr)NULL);

    pAddr = __osl_createSocketAddrFromSystem( &Addr );
    return pAddr;
}

/*****************************************************************************/
/* osl_getPeerAddrOfSocket  */
/*****************************************************************************/
oslSocketAddr SAL_CALL osl_getPeerAddrOfSocket(oslSocket pSocket)
{
    socklen_t AddrLen;
    struct sockaddr Addr;

    OSL_ASSERT(pSocket);
    if ( pSocket == 0 )
    {
        return 0;
    }

    pSocket->m_nLastError=0;
    AddrLen= sizeof(struct sockaddr);

    if(getpeername(pSocket->m_Socket, &Addr, &AddrLen) == OSL_SOCKET_ERROR)
    {
        pSocket->m_nLastError=errno;
        return 0;
    }
    return __osl_createSocketAddrFromSystem( &Addr );
}

/*****************************************************************************/
/* osl_bindAddrToSocket  */
/*****************************************************************************/
sal_Bool SAL_CALL osl_bindAddrToSocket(oslSocket pSocket,
                             oslSocketAddr pAddr)
{
    int nRet;

    OSL_ASSERT(pSocket && pAddr );
    if ( pSocket == 0 || pAddr == 0 )
    {
        return sal_False;
    }

    pSocket->m_nLastError=0;

    nRet = bind(pSocket->m_Socket, &(pAddr->m_sockaddr), sizeof(struct sockaddr));

    if ( nRet == OSL_SOCKET_ERROR)
    {
        pSocket->m_nLastError=errno;
        return sal_False;
    }

    return sal_True;
}


/*****************************************************************************/
/* osl_listenOnSocket  */
/*****************************************************************************/
sal_Bool SAL_CALL osl_listenOnSocket(oslSocket pSocket,
                           sal_Int32 MaxPendingConnections)
{
    int nRet;

    OSL_ASSERT(pSocket);
    if ( pSocket == 0 )
    {
        return sal_False;
    }

    pSocket->m_nLastError=0;

    nRet = listen(pSocket->m_Socket,
                  MaxPendingConnections == -1 ?
                  SOMAXCONN :
                  MaxPendingConnections);
    if ( nRet == OSL_SOCKET_ERROR)
    {
        pSocket->m_nLastError=errno;
        return sal_False;
    }

    return sal_True;
}


/*****************************************************************************/
/* osl_connectSocketTo  */
/*****************************************************************************/
oslSocketResult SAL_CALL osl_connectSocketTo(oslSocket pSocket,
                                    oslSocketAddr pAddr,
                                    const TimeValue* pTimeout)
{
    fd_set   WriteSet;
    fd_set   ExcptSet;
    int      ReadyHandles;
    struct timeval  tv;
    oslSocketResult Result= osl_Socket_Ok;

    OSL_PRECOND(pSocket, "osl_connectSocketTo(): need a valid socket!\n");

    if ( pSocket == 0 )
    {
        return osl_Socket_Error;
    }

    pSocket->m_nLastError=0;

    if (osl_isNonBlockingMode(pSocket))
    {
        if (connect(pSocket->m_Socket,
                    &(pAddr->m_sockaddr),
                    sizeof(struct sockaddr)) != OSL_SOCKET_ERROR)
            return osl_Socket_Ok;
        else
            if (errno == EWOULDBLOCK || errno == EINPROGRESS)
            {
                pSocket->m_nLastError=EINPROGRESS;
                return osl_Socket_InProgress;
            }


        pSocket->m_nLastError=errno;
        OSL_TRACE("can't connect : '%s'",strerror(errno));
        return osl_Socket_Error;
    }

    /* set socket temporarily to non-blocking */
    OSL_VERIFY(osl_enableNonBlockingMode(pSocket, sal_True));

    /* initiate connect */
    if(connect(pSocket->m_Socket,
               &(pAddr->m_sockaddr),
               sizeof(struct sockaddr)) != OSL_SOCKET_ERROR)
    {
       /* immediate connection */
        osl_enableNonBlockingMode(pSocket, sal_False);

        return osl_Socket_Ok;
    }
    else
    {
        /* really an error or just delayed? */
        if (errno != EINPROGRESS)
        {
            pSocket->m_nLastError=errno;
            OSL_TRACE(
                "osl_connectSocketTo(): connect failed: errno: %d (%s)\n",
                errno, strerror(errno));

            osl_enableNonBlockingMode(pSocket, sal_False);
            return osl_Socket_Error;
        }
    }


    /* prepare select set for socket  */
    FD_ZERO(&WriteSet);
    FD_ZERO(&ExcptSet);
    FD_SET(pSocket->m_Socket, &WriteSet);
    FD_SET(pSocket->m_Socket, &ExcptSet);

    /* prepare timeout */
    if (pTimeout)
    {
        /* divide milliseconds into seconds and microseconds */
        tv.tv_sec=  pTimeout->Seconds;
        tv.tv_usec= pTimeout->Nanosec / 1000L;
    }

    /* select */
    ReadyHandles= select(pSocket->m_Socket+1,
                         0,
                         PTR_FD_SET(WriteSet),
                         PTR_FD_SET(ExcptSet),
                         (pTimeout) ? &tv : 0);

    if (ReadyHandles > 0)  /* connected */
    {
        if ( FD_ISSET(pSocket->m_Socket, &WriteSet ) )
        {
            int nErrorCode = 0;
            socklen_t nErrorSize = sizeof( nErrorCode );

            int nSockOpt;

            nSockOpt = getsockopt ( pSocket->m_Socket, SOL_SOCKET, SO_ERROR,
                                    &nErrorCode, &nErrorSize );
            if ( (nSockOpt == 0) && (nErrorCode == 0))
                Result = osl_Socket_Ok;
            else
                Result = osl_Socket_Error;
        }
        else
        {
            Result= osl_Socket_Error;
        }
    }
    else if (ReadyHandles < 0)  /* error */
    {
        if (errno == EBADF) /* most probably interrupted by close() */
        {
            /* do not access pSockImpl because it is about to be or */
            /* already destroyed */
            return osl_Socket_Interrupted;
        }
        else
        {
            pSocket->m_nLastError=errno;
            Result= osl_Socket_Error;
        }
    }
    else    /* timeout */
    {
        pSocket->m_nLastError=errno;
        Result= osl_Socket_TimedOut;
    }

    osl_enableNonBlockingMode(pSocket, sal_False);

    return Result;
}


/*****************************************************************************/
/* osl_acceptConnectionOnSocket  */
/*****************************************************************************/
oslSocket SAL_CALL osl_acceptConnectionOnSocket(oslSocket pSocket,
                        oslSocketAddr* ppAddr)
{
    struct sockaddr Addr;
    int Connection, Flags;
    oslSocket pConnectionSockImpl;

    socklen_t AddrLen = sizeof(struct sockaddr);
    OSL_ASSERT(pSocket);
    if ( pSocket == 0 )
    {
        return 0;
    }

    pSocket->m_nLastError=0;
#if defined(LINUX)
    pSocket->m_bIsAccepting = sal_True;
#endif /* LINUX */

    if( ppAddr && *ppAddr )
    {
        osl_destroySocketAddr( *ppAddr );
        *ppAddr = 0;
    }

    /* prevent Linux EINTR behaviour */
    do
    {
        Connection = accept(pSocket->m_Socket, &Addr, &AddrLen);
    } while (Connection == -1 && errno == EINTR);


    /* accept failed? */
    if( Connection == OSL_SOCKET_ERROR )
    {
        pSocket->m_nLastError=errno;
        OSL_TRACE("osl_acceptConnectionOnSocket : accept error '%s'",strerror(errno));

#if defined(LINUX)
        pSocket->m_bIsAccepting = sal_False;
#endif /* LINUX */
        return 0;
    }

    OSL_ASSERT(AddrLen == sizeof(struct sockaddr));


#if defined(LINUX)
    if ( pSocket->m_bIsInShutdown == sal_True )
    {
        close(Connection);
        OSL_TRACE("osl_acceptConnectionOnSocket : close while accept");
        return 0;
    }
#endif /* LINUX */


    if(ppAddr)
    {
        *ppAddr= __osl_createSocketAddrFromSystem(&Addr);
    }

    /* alloc memory */
    pConnectionSockImpl= __osl_createSocketImpl(OSL_INVALID_SOCKET);

    /* set close-on-exec flag */
    if ((Flags = fcntl(Connection, F_GETFD, 0)) != -1)
    {
        Flags |= FD_CLOEXEC;
        if (fcntl(Connection, F_SETFD, Flags) == -1)
        {
            pSocket->m_nLastError=errno;
            OSL_TRACE("osl_acceptConnectionOnSocket failed changing socket flags. Errno: %d (%s)\n",
                      errno,
                      strerror(errno));
        }

    }

    pConnectionSockImpl->m_Socket           = Connection;
    pConnectionSockImpl->m_nLastError       = 0;
    pConnectionSockImpl->m_CloseCallback    = NULL;
    pConnectionSockImpl->m_CallbackArg      = NULL;
#if defined(LINUX)
    pConnectionSockImpl->m_bIsAccepting     = sal_False;

    pSocket->m_bIsAccepting = sal_False;
#endif /* LINUX */
    return pConnectionSockImpl;
}

/*****************************************************************************/
/* osl_receiveSocket  */
/*****************************************************************************/
sal_Int32 SAL_CALL osl_receiveSocket(oslSocket pSocket,
                          void* pBuffer,
                          sal_uInt32 BytesToRead,
                          oslSocketMsgFlag Flag)
{
    int nRead;

    OSL_ASSERT(pSocket);
    if ( pSocket == 0 )
    {
        OSL_TRACE("osl_receiveSocket : Invalid socket");
        return -1;
    }

    pSocket->m_nLastError=0;

    do
    {
        nRead =  recv(pSocket->m_Socket,
                      (sal_Char*)pBuffer,
                      BytesToRead,
                      MSG_FLAG_TO_NATIVE(Flag));
    } while ( nRead < 0 && errno == EINTR );

    if ( nRead < 0 )
    {
        pSocket->m_nLastError=errno;
        OSL_TRACE("osl_receiveSocket failed : %i '%s'",nRead,strerror(errno));
    }
    else if ( nRead == 0 )
    {
        OSL_TRACE("osl_receiveSocket failed : %i '%s'",nRead,"EOL");
    }

    return nRead;
}


/*****************************************************************************/
/* osl_receiveFromSocket  */
/*****************************************************************************/
sal_Int32 SAL_CALL osl_receiveFromSocket(oslSocket pSocket,
                              oslSocketAddr pSenderAddr,
                              void* pBuffer,
                              sal_uInt32 BufferSize,
                              oslSocketMsgFlag Flag)
{
    int nRead;
    struct sockaddr *pSystemSockAddr = 0;
    socklen_t AddrLen = 0;
    if( pSenderAddr )
    {
        AddrLen = sizeof( struct sockaddr );
        pSystemSockAddr = &(pSenderAddr->m_sockaddr);
    }

    OSL_ASSERT(pSocket);
    if ( pSocket == 0 )
    {
        OSL_TRACE("osl_receiveFromSocket : Invalid socket");
        return -1;
    }

    pSocket->m_nLastError=0;

    nRead = recvfrom(pSocket->m_Socket,
                     (sal_Char*)pBuffer,
                     BufferSize,
                     MSG_FLAG_TO_NATIVE(Flag),
                     pSystemSockAddr,
                     &AddrLen);

    if ( nRead < 0 )
    {
        pSocket->m_nLastError=errno;
        OSL_TRACE("osl_receiveFromSocket failed : %i '%s'",nRead,strerror(errno));
    }
    else if ( nRead == 0 )
    {
        OSL_TRACE("osl_receiveSocket failed : %i '%s'",nRead,"EOL");
    }

    return nRead;
}


/*****************************************************************************/
/* osl_sendSocket  */
/*****************************************************************************/
sal_Int32 SAL_CALL osl_sendSocket(oslSocket pSocket,
                       const void* pBuffer,
                       sal_uInt32 BytesToSend,
                       oslSocketMsgFlag Flag)
{
    int nWritten;

    OSL_ASSERT(pSocket);
    if ( pSocket == 0 )
    {
        OSL_TRACE("osl_sendSocket : Invalid socket");
        return -1;
    }

    pSocket->m_nLastError=0;

    do
    {
        nWritten = send(pSocket->m_Socket,
                        (sal_Char*)pBuffer,
                        BytesToSend,
                        MSG_FLAG_TO_NATIVE(Flag));
    } while ( nWritten < 0 && errno == EINTR );


    if ( nWritten < 0 )
    {
        pSocket->m_nLastError=errno;
        OSL_TRACE("osl_sendSocket failed : %i '%s'",nWritten,strerror(errno));
    }
    else if ( nWritten == 0 )
    {
        OSL_TRACE("osl_sendSocket failed : %i '%s'",nWritten,"EOL");
    }

    return nWritten;
}

/*****************************************************************************/
/* osl_sendToSocket  */
/*****************************************************************************/
sal_Int32 SAL_CALL osl_sendToSocket(oslSocket pSocket,
                         oslSocketAddr ReceiverAddr,
                         const void* pBuffer,
                         sal_uInt32 BytesToSend,
                         oslSocketMsgFlag Flag)
{
    int nWritten;

    struct sockaddr *pSystemSockAddr = 0;
    int AddrLen = 0;
    if( ReceiverAddr )
    {
        pSystemSockAddr = &(ReceiverAddr->m_sockaddr);
        AddrLen = sizeof( struct sockaddr );
    }

    OSL_ASSERT(pSocket);
    if ( pSocket == 0 )
    {
        OSL_TRACE("osl_sendToSocket : Invalid socket");
        return -1;
    }

    pSocket->m_nLastError=0;

    /* ReceiverAddr might be 0 when used on a connected socket. */
    /* Then sendto should behave like send. */

    nWritten = sendto(pSocket->m_Socket,
                      (sal_Char*)pBuffer,
                      BytesToSend,
                      MSG_FLAG_TO_NATIVE(Flag),
                      pSystemSockAddr,
                      AddrLen);

    if ( nWritten < 0 )
    {
        pSocket->m_nLastError=errno;
        OSL_TRACE("osl_sendToSocket failed : %i '%s'",nWritten,strerror(errno));
    }
    else if ( nWritten == 0 )
    {
        OSL_TRACE("osl_sendToSocket failed : %i '%s'",nWritten,"EOL");
    }

    return nWritten;
}

/*****************************************************************************/
/* osl_readSocket  */
/*****************************************************************************/
sal_Int32 SAL_CALL osl_readSocket (
    oslSocket pSocket, void *pBuffer, sal_Int32 n )
{
    sal_uInt8 * Ptr = (sal_uInt8 *)pBuffer;
    sal_uInt32 BytesRead= 0;
    sal_uInt32 BytesToRead= n;

    OSL_ASSERT( pSocket);

    /* loop until all desired bytes were read or an error occurred */
    while (BytesToRead > 0)
    {
        sal_Int32 RetVal;
        RetVal= osl_receiveSocket(pSocket,
                                   Ptr,
                                   BytesToRead,
                                   osl_Socket_MsgNormal);

        /* error occurred? */
        if(RetVal <= 0)
        {
            break;
        }

        BytesToRead -= RetVal;
        BytesRead += RetVal;
        Ptr += RetVal;
    }

    return BytesRead;
}

/*****************************************************************************/
/* osl_writeSocket  */
/*****************************************************************************/
sal_Int32 SAL_CALL osl_writeSocket(
    oslSocket pSocket, const void *pBuffer, sal_Int32 n )
{
    /* loop until all desired bytes were send or an error occurred */
    sal_uInt32 BytesSend= 0;
    sal_uInt32 BytesToSend= n;
    sal_uInt8 *Ptr = ( sal_uInt8 * )pBuffer;

    OSL_ASSERT( pSocket );

    while (BytesToSend > 0)
    {
        sal_Int32 RetVal;

        RetVal= osl_sendSocket( pSocket,Ptr,BytesToSend,osl_Socket_MsgNormal);

        /* error occurred? */
        if(RetVal <= 0)
        {
            break;
        }

        BytesToSend -= RetVal;
        BytesSend += RetVal;
        Ptr += RetVal;

    }
    return BytesSend;
}

/*****************************************************************************/
/* __osl_socket_poll */
/*****************************************************************************/

#ifdef HAVE_POLL_H /* poll() */

sal_Bool __osl_socket_poll (
    oslSocket        pSocket,
    const TimeValue* pTimeout,
    short            nEvent)
{
    struct pollfd fds;
    int           timeout;
    int           result;

    OSL_ASSERT(0 != pSocket);
    if (0 == pSocket)
      return sal_False; /* EINVAL */

    pSocket->m_nLastError = 0;

    fds.fd      = pSocket->m_Socket;
    fds.events  = nEvent;
    fds.revents = 0;

    timeout = -1;
    if (pTimeout)
    {
        /* Convert to [ms] */
        timeout  = pTimeout->Seconds * 1000;
        timeout += pTimeout->Nanosec / (1000 * 1000);
    }

    result = poll (&fds, 1, timeout);
    if (result < 0)
    {
        pSocket->m_nLastError = errno;
        OSL_TRACE("__osl_socket_poll(): poll error: %d (%s)",
                  errno, strerror(errno));
        return sal_False;
    }
    if (result == 0)
    {
        /* Timeout */
        return sal_False;
    }

    return ((fds.revents & nEvent) == nEvent);
}

#else  /* select() */

sal_Bool __osl_socket_poll (
    oslSocket        pSocket,
    const TimeValue* pTimeout,
    short            nEvent)
{
    fd_set         fds;
    struct timeval tv;
    int            result;

    OSL_ASSERT(0 != pSocket);
    if (0 == pSocket)
      return sal_False; /* EINVAL */

    pSocket->m_nLastError = 0;

    FD_ZERO(&fds);
    FD_SET(pSocket->m_Socket, &fds);

    if (pTimeout)
    {
        /* Convert to 'timeval' */
        tv.tv_sec  = pTimeout->Seconds;
        tv.tv_usec = pTimeout->Nanosec / 1000;
    }

    result = select (
        pSocket->m_Socket + 1,
        (nEvent == POLLIN ) ? PTR_FD_SET(fds) : NULL,
        (nEvent == POLLOUT) ? PTR_FD_SET(fds) : NULL,
        (nEvent == POLLPRI) ? PTR_FD_SET(fds) : NULL,
        (pTimeout)          ? &tv             : NULL);

    if (result < 0)
    {
        pSocket->m_nLastError = errno;
        OSL_TRACE("__osl_socket_poll(): select error: %d (%s)",
                  errno, strerror(errno));
        return sal_False;
    }
    if (result == 0)
    {
        /* Timeout */
        return sal_False;
    }

    return (FD_ISSET(pSocket->m_Socket, &fds) ? sal_True : sal_False);
}

#endif /* HAVE_POLL_H */

/*****************************************************************************/
/* osl_isReceiveReady  */
/*****************************************************************************/
sal_Bool SAL_CALL osl_isReceiveReady (
    oslSocket pSocket, const TimeValue* pTimeout)
{
    OSL_ASSERT(pSocket);
    if (pSocket == NULL)
    {
        /* ENOTSOCK */
        return sal_False;
    }

    return __osl_socket_poll (pSocket, pTimeout, POLLIN);
}

/*****************************************************************************/
/* osl_isSendReady  */
/*****************************************************************************/
sal_Bool SAL_CALL osl_isSendReady (
    oslSocket pSocket, const TimeValue* pTimeout)
{
    OSL_ASSERT(pSocket);
    if (pSocket == NULL)
    {
        /* ENOTSOCK */
        return sal_False;
    }

    return __osl_socket_poll (pSocket, pTimeout, POLLOUT);
}

/*****************************************************************************/
/* osl_isExceptionPending  */
/*****************************************************************************/
sal_Bool SAL_CALL osl_isExceptionPending (
    oslSocket pSocket, const TimeValue* pTimeout)
{
    OSL_ASSERT(pSocket);
    if (pSocket == NULL)
    {
        /* ENOTSOCK */
        return sal_False;
    }

    return __osl_socket_poll (pSocket, pTimeout, POLLPRI);
}

/*****************************************************************************/
/* osl_shutdownSocket  */
/*****************************************************************************/
sal_Bool SAL_CALL osl_shutdownSocket(oslSocket pSocket,
                           oslSocketDirection Direction)
{
    int nRet;

    OSL_ASSERT(pSocket);
    if ( pSocket == 0 )
    {
        return sal_False;
    }

    pSocket->m_nLastError=0;

    nRet=shutdown(pSocket->m_Socket, DIRECTION_TO_NATIVE(Direction));
    if (nRet != 0 )
    {
        pSocket->m_nLastError=errno;
        OSL_TRACE("shutdown error '%s'",strerror(errno));
    }
    return (nRet==0);
}


/*****************************************************************************/
/* osl_getSocketOption  */
/*****************************************************************************/
sal_Int32 SAL_CALL osl_getSocketOption(oslSocket pSocket,
                            oslSocketOptionLevel    Level,
                            oslSocketOption         Option,
                            void*                   pBuffer,
                            sal_uInt32                  BufferLen)
{
    socklen_t nOptLen = (socklen_t) BufferLen;

    OSL_ASSERT(pSocket);
    if ( pSocket == 0 )
    {
        return -1;
    }

    pSocket->m_nLastError=0;

    if(getsockopt(pSocket->m_Socket,
                  OPTION_LEVEL_TO_NATIVE(Level),
                  OPTION_TO_NATIVE(Option),
                  (sal_Char*)pBuffer,
                  &nOptLen) == -1)
    {
        pSocket->m_nLastError=errno;
        return -1;
    }

    return BufferLen;
}

/*****************************************************************************/
/* osl_setSocketOption  */
/*****************************************************************************/
sal_Bool SAL_CALL osl_setSocketOption(oslSocket pSocket,
                            oslSocketOptionLevel    Level,
                            oslSocketOption         Option,
                            void*                   pBuffer,
                            sal_uInt32                  BufferLen)
{
    int nRet;

    OSL_ASSERT(pSocket);
    if ( pSocket == 0 )
    {
        return sal_False;
    }

    pSocket->m_nLastError=0;

    nRet = setsockopt(pSocket->m_Socket,
                      OPTION_LEVEL_TO_NATIVE(Level),
                      OPTION_TO_NATIVE(Option),
                      (sal_Char*)pBuffer,
                      BufferLen);

    if ( nRet < 0 )
    {
        pSocket->m_nLastError=errno;
        return sal_False;
    }

    return sal_True;
}

/*****************************************************************************/
/* osl_enableNonBlockingMode  */
/*****************************************************************************/
sal_Bool SAL_CALL osl_enableNonBlockingMode(oslSocket pSocket,
                                  sal_Bool On)
{
    int flags;
    int nRet;

    OSL_ASSERT(pSocket);
    if ( pSocket == 0 )
    {
        return sal_False;
    }

    pSocket->m_nLastError=0;

    flags = fcntl(pSocket->m_Socket, F_GETFL, 0);

    if (On)
        flags |= O_NONBLOCK;
    else
        flags &= ~(O_NONBLOCK);

    nRet = fcntl(pSocket->m_Socket, F_SETFL, flags);

    if  ( nRet < 0 )
    {
        pSocket->m_nLastError=errno;
        return sal_False;
    }

    return sal_True;
}

/*****************************************************************************/
/* osl_isNonBlockingMode  */
/*****************************************************************************/
sal_Bool SAL_CALL osl_isNonBlockingMode(oslSocket pSocket)
{
    int flags;

    OSL_ASSERT(pSocket);
    if ( pSocket == 0 )
    {
        return sal_False;
    }

    pSocket->m_nLastError=0;

    flags = fcntl(pSocket->m_Socket, F_GETFL, 0);

    if (flags == -1 || !(flags & O_NONBLOCK))
        return sal_False;
    else
        return sal_True;
}

/*****************************************************************************/
/* osl_getSocketType  */
/*****************************************************************************/
oslSocketType SAL_CALL osl_getSocketType(oslSocket pSocket)
{
    int Type=0;
    socklen_t TypeSize= sizeof(Type);

    OSL_ASSERT(pSocket);
    if ( pSocket == 0 )
    {
        return osl_Socket_TypeInvalid;
    }

    pSocket->m_nLastError=0;

    if(getsockopt(pSocket->m_Socket,
                  OPTION_LEVEL_TO_NATIVE(osl_Socket_LevelSocket),
                  OPTION_TO_NATIVE(osl_Socket_OptionType),
                  (sal_Char*)&Type,
                  &TypeSize) == -1)
    {
        /* error */
        pSocket->m_nLastError=errno;
        return osl_Socket_TypeInvalid;
    }

    return TYPE_FROM_NATIVE(Type);

}

/*****************************************************************************/
/* osl_getLastSocketErrorDescription  */
/*****************************************************************************/
void SAL_CALL osl_getLastSocketErrorDescription(oslSocket Socket, rtl_uString **ustrError)
{
    sal_Char pszError[1024];

    pszError[0] = '\0';

    osl_psz_getLastSocketErrorDescription(Socket,pszError,sizeof(pszError));

    rtl_uString_newFromAscii(ustrError,pszError);

    return;
}


void SAL_CALL osl_psz_getLastSocketErrorDescription(oslSocket pSocket, sal_Char* pBuffer, sal_uInt32 BufferSize)
{
    /* make shure pBuffer will be a zero-terminated string even when strncpy has to cut */
    pBuffer[BufferSize-1]= '\0';

    if ( pSocket == 0 )
    {
        strncpy(pBuffer, strerror(EINVAL), BufferSize-1);
        return;
    }

    strncpy(pBuffer, strerror(pSocket->m_nLastError), BufferSize-1);
    return;
}

/*****************************************************************************/
/* osl_getLastSocketError  */
/*****************************************************************************/
oslSocketError SAL_CALL osl_getLastSocketError(oslSocket pSocket)
{
    if ( pSocket == 0 )
    {
        return ERROR_FROM_NATIVE(EINVAL);
    }

    return ERROR_FROM_NATIVE(pSocket->m_nLastError);
}

/*****************************************************************************/
/* SocketSet                                                                 */
/*****************************************************************************/
typedef struct _TSocketSetImpl
{
    int     m_MaxHandle;    /* for select(), the largest descriptor in the set */
    fd_set  m_Set;          /* the set of descriptors */

} TSocketSetImpl;

/*****************************************************************************/
/* osl_createSocketSet  */
/*****************************************************************************/
oslSocketSet SAL_CALL osl_createSocketSet()
{
    TSocketSetImpl* pSet;

    pSet= (TSocketSetImpl*)malloc(sizeof(TSocketSetImpl));

    OSL_ASSERT(pSet);

    if(pSet)
    {
        pSet->m_MaxHandle= 0;
        FD_ZERO(&pSet->m_Set);
    }

    return (oslSocketSet)pSet;
}

/*****************************************************************************/
/* osl_destroySocketSet  */
/*****************************************************************************/
void SAL_CALL osl_destroySocketSet(oslSocketSet Set)
{
    if(Set)
        free(Set);
}

/*****************************************************************************/
/* osl_clearSocketSet  */
/*****************************************************************************/
void SAL_CALL osl_clearSocketSet(oslSocketSet Set)
{
    TSocketSetImpl* pSet;
    OSL_ASSERT(Set);
    if ( Set == 0 )
    {
        return;
    }

    pSet= (TSocketSetImpl*)Set;
    pSet->m_MaxHandle= 0;

    FD_ZERO(&pSet->m_Set);
}

/*****************************************************************************/
/* osl_addToSocketSet  */
/*****************************************************************************/
void SAL_CALL osl_addToSocketSet(oslSocketSet Set, oslSocket pSocket)
{
    TSocketSetImpl* pSet;

    OSL_ASSERT(Set);
    OSL_ASSERT(pSocket);

    if ( Set == 0 || pSocket == 0)
    {
        return;
    }

    pSet= (TSocketSetImpl*)Set;

    /* correct max handle */
    if(pSocket->m_Socket > pSet->m_MaxHandle)
        pSet->m_MaxHandle= pSocket->m_Socket;
    FD_SET(pSocket->m_Socket, &pSet->m_Set);

}

/*****************************************************************************/
/* osl_removeFromSocketSet  */
/*****************************************************************************/
void SAL_CALL osl_removeFromSocketSet(oslSocketSet Set, oslSocket pSocket)
{
    TSocketSetImpl* pSet;

    OSL_ASSERT(Set);
    OSL_ASSERT(pSocket);

    if ( Set == 0 || pSocket == 0)
    {
        return;
    }

    pSet= (TSocketSetImpl*)Set;

    /* correct max handle */
    if(pSocket->m_Socket == pSet->m_MaxHandle)
    {
        /* not optimal, since the next used descriptor might be */
        /* much smaller than m_Socket-1, but it will do */
        pSet->m_MaxHandle--;
        if(pSet->m_MaxHandle < 0)
        {
            pSet->m_MaxHandle= 0;   /* avoid underflow */
        }
    }

    FD_CLR(pSocket->m_Socket, &pSet->m_Set);
}

/*****************************************************************************/
/* osl_isInSocketSet  */
/*****************************************************************************/
sal_Bool SAL_CALL osl_isInSocketSet(oslSocketSet Set, oslSocket pSocket)
{
    TSocketSetImpl* pSet;

    OSL_ASSERT(Set);
    OSL_ASSERT(pSocket);
    if ( Set == 0 || pSocket == 0 )
    {
        return sal_False;
    }

    pSet= (TSocketSetImpl*)Set;

    return (FD_ISSET(pSocket->m_Socket, &pSet->m_Set) != 0);
}

/*****************************************************************************/
/* osl_demultiplexSocketEvents  */
/*****************************************************************************/
sal_Int32 SAL_CALL osl_demultiplexSocketEvents(oslSocketSet IncomingSet,
                                    oslSocketSet OutgoingSet,
                                    oslSocketSet OutOfBandSet,
                                    const TimeValue* pTimeout)
{
    int MaxHandle= 0;
    struct timeval  tv;
    TSocketSetImpl* pInSet;
    TSocketSetImpl* pOutSet;
    TSocketSetImpl* pOOBSet;

    if (pTimeout)
    {
        /* non-blocking call */
        tv.tv_sec  = pTimeout->Seconds;
        tv.tv_usec = pTimeout->Nanosec / 1000L;
    }

    /* map opaque data to impl-types */
    pInSet=  (TSocketSetImpl*)IncomingSet;
    pOutSet= (TSocketSetImpl*)OutgoingSet;
    pOOBSet= (TSocketSetImpl*)OutOfBandSet;

    /* get max handle from all sets */
    if (pInSet)
        MaxHandle= pInSet->m_MaxHandle;

    if (pOutSet && (pOutSet->m_MaxHandle > MaxHandle))
        MaxHandle= pOutSet->m_MaxHandle;

    if (pOOBSet && (pOOBSet->m_MaxHandle > MaxHandle))
        MaxHandle= pOOBSet->m_MaxHandle;

    return select(MaxHandle+1,
                  pInSet  ? PTR_FD_SET(pInSet->m_Set)  : 0,
                  pOutSet ? PTR_FD_SET(pOutSet->m_Set) : 0,
                  pOOBSet ? PTR_FD_SET(pOOBSet->m_Set) : 0,
                  pTimeout ? &tv : 0);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
