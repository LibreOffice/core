/*************************************************************************
 *
 *  $RCSfile: socket.c,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-14 09:48:11 $
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

#include "system.h"

#include <osl/socket.h>
#include <osl/diagnose.h>
#include <osl/mutex.h>
#include <osl/signal.h>

#ifndef _OSL_TIME_H_
#include <osl/time.h>
#endif

#include <ctype.h>

#include "sockimpl.h"

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

oslSocketImpl* __osl_createSocketImpl(int Socket)
{
    oslSocketImpl* pSockImpl;

    pSockImpl = (oslSocketImpl*)calloc(1, sizeof(oslSocketImpl));

    pSockImpl->m_Socket = Socket;
    pSockImpl->m_nLastError = 0;
    pSockImpl->m_CloseCallback = 0;
    pSockImpl->m_CallbackArg = 0;
#if defined(LINUX)
    pSockImpl->m_bIsAccepting = sal_False;
#endif

    return (pSockImpl);
}

void __osl_destroySocketImpl(oslSocketImpl *pImpl)
{
    if (pImpl != NULL)
        free(pImpl);
}

/*****************************************************************************/
/* osl_createEmptySocketAddr */
/*****************************************************************************/
oslSocketAddr SAL_CALL osl_createEmptySocketAddr(oslAddrFamily Family)
{

    struct sockaddr* pAddr;

    pAddr= (struct sockaddr*)malloc(sizeof(struct sockaddr));

    /* is it an internet-address? */
    if(Family == osl_Socket_FamilyInet)
    {
        struct sockaddr_in* pInetAddr= (struct sockaddr_in*)pAddr;

        pInetAddr->sin_family= FAMILY_TO_NATIVE(osl_Socket_FamilyInet);
        pInetAddr->sin_addr.s_addr= htonl(INADDR_ANY);
        pInetAddr->sin_port= 0;
    }
    else
    {
        memset(pAddr, 0, sizeof(struct sockaddr));
        pAddr->sa_family= FAMILY_TO_NATIVE(Family);
    }

    return (oslSocketAddr)pAddr;
}

/*****************************************************************************/
/* osl_copySocketAddr */
/*****************************************************************************/
oslSocketAddr SAL_CALL osl_copySocketAddr(oslSocketAddr Addr)
{
    struct sockaddr* pAddr;

    pAddr= (struct sockaddr*)malloc(sizeof(struct sockaddr));

    memcpy(pAddr, Addr, sizeof(struct sockaddr));

    return pAddr;
}

/*****************************************************************************/
/* osl_isEqualSocketAddr */
/*****************************************************************************/
sal_Bool SAL_CALL osl_isEqualSocketAddr (
    oslSocketAddr Addr1,
    oslSocketAddr Addr2)
{
    struct sockaddr* pAddr1= (struct sockaddr*)Addr1;
    struct sockaddr* pAddr2= (struct sockaddr*)Addr2;

    OSL_ASSERT(pAddr1);
    OSL_ASSERT(pAddr2);

    if ( pAddr1 ==0 || pAddr2 == 0 )
    {
        return sal_False;
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

            case AF_IPX:
            {
                struct sockaddr_ipx* pIpxAddr1= (struct sockaddr_ipx*)pAddr1;
                struct sockaddr_ipx* pIpxAddr2= (struct sockaddr_ipx*)pAddr2;

                if ((pIpxAddr1->sa_family == pIpxAddr2->sa_family) &&
                    (memcmp(&pIpxAddr1->sa_netnum, &pIpxAddr2->sa_netnum, sizeof(pIpxAddr1->sa_netnum)) == 0) &&
                    (memcmp(&pIpxAddr1->sa_nodenum, &pIpxAddr2->sa_nodenum, sizeof(pIpxAddr1->sa_nodenum)) == 0) &&
                    (pIpxAddr1->sa_socket == pIpxAddr2->sa_socket))
                    return (sal_True);
            }

            default:
            {
                return (memcmp(pAddr1, Addr2, sizeof(struct sockaddr)) == 0);
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
    struct sockaddr_in *pAddr = NULL;
    sal_uInt32          nAddr = OSL_INADDR_NONE;

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

    pAddr = (struct sockaddr_in*) malloc (sizeof(struct sockaddr_in));
    if (pAddr)
    {
        pAddr->sin_family      = FAMILY_TO_NATIVE(osl_Socket_FamilyInet);
        pAddr->sin_port        = htons((sal_uInt16)(Port & 0xffff));
        pAddr->sin_addr.s_addr = nAddr; /* Already in network order */
    }
    return ((oslSocketAddr)pAddr);
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
    sal_Int32 Addr;

    Addr= inet_addr(pszDottedAddr);

    if(Addr != -1)
    {
        /* valid dotted addr */

        struct sockaddr_in* pInetAddr;
        pInetAddr= (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));

        OSL_ASSERT(pInetAddr);

        pInetAddr->sin_family= FAMILY_TO_NATIVE(osl_Socket_FamilyInet);
        pInetAddr->sin_port=    htons((sal_uInt16)Port);
        pInetAddr->sin_addr.s_addr= Addr;   /* already in right order */

        return (oslSocketAddr)pInetAddr;
    }

    return (oslSocketAddr)0;
}


/*****************************************************************************/
/* osl_createIpxSocketAddr */
/*****************************************************************************/
oslSocketAddr SAL_CALL osl_createIpxSocketAddr(rtl_uString * ustrNetNumber,
                                                 rtl_uString * ustrNodeNumber,
                                               sal_uInt32 SocketNumber)
{
    oslSocketAddr Addr;
    sal_Char NetNumber[4];
    sal_Char NodeNumber[4];
    rtl_String* strNetNumber=0;
    rtl_String* strNodeNumber=0;
    sal_Char* pszNetNumber=0;
    sal_Char* pszNodeNumber=0;

    if ( ustrNetNumber != 0 )
    {
        rtl_uString2String( &strNetNumber,
                            rtl_uString_getStr(ustrNetNumber),
                            rtl_uString_getLength(ustrNetNumber),
                            RTL_TEXTENCODING_UTF8,
                            OUSTRING_TO_OSTRING_CVTFLAGS );
        pszNetNumber = rtl_string_getStr(strNetNumber);
    }

    if ( ustrNodeNumber != 0 )
    {
        rtl_uString2String( &strNodeNumber,
                            rtl_uString_getStr(ustrNodeNumber),
                            rtl_uString_getLength(ustrNodeNumber),
                            RTL_TEXTENCODING_UTF8,
                            OUSTRING_TO_OSTRING_CVTFLAGS );
        pszNodeNumber = rtl_string_getStr(strNodeNumber);
    }

    memcpy(NetNumber,pszNetNumber,sizeof(NetNumber));
    memcpy(NodeNumber,pszNodeNumber,sizeof(NodeNumber));

    Addr = osl_psz_createIpxSocketAddr(NetNumber,NodeNumber,SocketNumber);

    if ( strNetNumber != 0 )
    {
        rtl_string_release(strNetNumber);
    }

    if ( strNodeNumber != 0 )
    {
        rtl_string_release(strNodeNumber);
    }

    return Addr;
}

oslSocketAddr SAL_CALL osl_psz_createIpxSocketAddr (
    const sal_Char NetNumber[4],
    const sal_Char NodeNumber[6],
    sal_uInt32     SocketNumber)
{
    struct sockaddr_ipx* pIpxAddr;

    pIpxAddr= (struct sockaddr_ipx*)malloc(sizeof(struct sockaddr));

    OSL_ASSERT(pIpxAddr);

    pIpxAddr->sa_family= FAMILY_TO_NATIVE(osl_Socket_FamilyIpx);
    memcpy(&pIpxAddr->sa_netnum, NetNumber, sizeof(pIpxAddr->sa_netnum));
    memcpy(&pIpxAddr->sa_nodenum, NodeNumber, sizeof(pIpxAddr->sa_nodenum));
    pIpxAddr->sa_socket= (sal_uInt16)SocketNumber;

    return (oslSocketAddr)pIpxAddr;
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
#ifdef LINUX
    struct hostent *__result; /* will be the same as result */
    int __error;
    __error = gethostbyname_r (name, result, buffer, buflen,
                 &__result, h_errnop);
    return __error ? NULL : __result ;
#else
    return gethostbyname_r( name, result, buffer, buflen, h_errnop);
#endif
}

static sal_Bool  _osl_getDomainName (sal_Char *buffer, sal_Int32 bufsiz)
{
    sal_Bool result;
    int      p[2];

    result = sal_False;
    if (pipe (p) == 0)
    {
        pid_t pid;
        int nStatus;

        pid = fork();
        if (pid == 0)
        {
            char *argv[] =
            {
                "/bin/domainname",
                NULL
            };

            close (p[0]);
            dup2  (p[1], 1);
            close (p[1]);

            if (execvp ("/bin/domainname", argv) < 0)
            {
                exit(-1);
            }
        }
        else if (pid > 0)
        {
            sal_Int32 k = 0, n = bufsiz;

            close (p[1]);
            if ((k = read (p[0], buffer, n - 1)) > 0)
            {
                buffer[k] = 0;
                if (buffer[k - 1] == '\n')
                    buffer[k - 1] = 0;
                result = sal_True;
            }
            close (p[0]);
        }
        else
        {
            close (p[0]);
            close (p[1]);
        }

        waitpid (pid, &nStatus, 0);
    }
    return (result);
}

static sal_Char* _osl_getFullQualifiedDomainName (const sal_Char *pHostName)
{
#   define DOMAINNAME_LENGTH 512
    sal_uInt32          nLengthOfHostName;
    static sal_uInt32   nLengthOfDomainName = 0;
    static sal_Char    *pDomainName = NULL;

    sal_Char  *pFullQualifiedName;
#if 0  /* OBSOLETE */
    FILE      *pPipeToDomainnameExe;
#endif /* OBSOLETE */

    /* get a '\0' terminated domainname */

    /* read default domainname default from environment */
    if (nLengthOfDomainName == 0)
    {
        sal_Char *pEnvDomain;

        pEnvDomain = getenv ("STAR_OVERRIDE_DOMAINNAME");
        if (pEnvDomain)
        {
            pDomainName = strdup (pEnvDomain);
            nLengthOfDomainName = strlen (pDomainName);
        }
    }

#if 1  /* NEW */
    if (nLengthOfDomainName == 0)
    {
        sal_Char pDomainNameBuffer[ DOMAINNAME_LENGTH ];

        pDomainNameBuffer[0] = '\0';

        if (_osl_getDomainName (pDomainNameBuffer, DOMAINNAME_LENGTH))
        {
            pDomainName = strdup (pDomainNameBuffer);
            nLengthOfDomainName = strlen (pDomainName);
        }
    }

#endif /* NEW */
#if 0  /* OBSOLETE */
#ifdef SCO

    /* call 'domainname > /usr/tmp/some-tmp-file', since
       popen read pclose do block or core-dump,
       (even the pipe-stuff that comes with pthreads) */
    if (nLengthOfDomainName == 0)
    {
        sal_Char  tmp_name[ L_tmpnam ];
        FILE     *tmp_file;
        sal_Char  domain_call [ L_tmpnam + 16 ] = "domainname > ";

        tmp_name[0] = '\0';

        tmpnam ( tmp_name );
        strcat ( domain_call, tmp_name );
        if (   (system ( domain_call ) == 0)
            && ((tmp_file = fopen( tmp_name, "r" )) != NULL ) )
        {
            sal_Char  pDomainNameBuffer[ DOMAINNAME_LENGTH ];

            pDomainNameBuffer[0] = '\0';

            if ( fgets ( pDomainNameBuffer, DOMAINNAME_LENGTH, tmp_file ) )
            {
                pDomainName = strdup( pDomainNameBuffer );
                nLengthOfDomainName = strlen( pDomainName );
                if (   ( nLengthOfDomainName > 0 )
                    && ( pDomainName[ nLengthOfDomainName - 1] == '\n' ) )
                    pDomainName[ --nLengthOfDomainName ] = '\0';
            }
            fclose ( tmp_file );
        }
        unlink( tmp_name );
    }

#else /* !SCO */

    /* read the domainname from pipe to the program domainname */
    if (   (nLengthOfDomainName == 0)
        && (pPipeToDomainnameExe = popen( "domainname", "r")) )
    {
        sal_Char  c;
        sal_Char  pDomainNameBuffer[ DOMAINNAME_LENGTH ];
        sal_Char *pDomainNamePointer;

        pDomainNameBuffer[0] = '\0';

        pDomainNamePointer = pDomainNameBuffer;
        while (    ((c = getc( pPipeToDomainnameExe )) != EOF)
                && (nLengthOfDomainName < (DOMAINNAME_LENGTH - 1)) )
        {
            if (! isspace(c))
            {
                 nLengthOfDomainName++ ;
                   *pDomainNamePointer++ = (sal_Char)c;
            }
        }
        *pDomainNamePointer = '\0';
        pDomainName = strdup( pDomainNameBuffer );

        pclose( pPipeToDomainnameExe );
    }

#endif /* !SCO */
#endif /* OBSOLETE */

    /* compose hostname and domainname */
    nLengthOfHostName = strlen( pHostName );
    pFullQualifiedName = (sal_Char*) malloc( (nLengthOfHostName + 1
                            + nLengthOfDomainName + 1) * sizeof(sal_Char) );
    memcpy( pFullQualifiedName, pHostName,
        (nLengthOfHostName + 1) * sizeof(sal_Char) );

    if ( nLengthOfDomainName > 0 )
    {
        /* fqdn = hostname + '.' + domainname + '\0' */
        pFullQualifiedName[ nLengthOfHostName ] = '.';
        memcpy( pFullQualifiedName + nLengthOfHostName + 1, pDomainName,
            nLengthOfDomainName + 1 );
    }

    /* check whether full-qualified name and hostname point to the same host
     * should almost always be true */
    if ( nLengthOfDomainName > 0 )
    {
        struct hostent *pQualifiedHostByName;
        struct hostent *pHostByName;
        sal_Bool        bHostsAreEqual;

        /* buffer for calls to reentrant version of gethostbyname */
        struct hostent  aHostByName, aQualifiedHostByName;
        sal_Char        pHostBuffer[ 256 ];
        sal_Char        pQualifiedHostBuffer[ 256 ];
        int     nErrorNo;

        pHostBuffer[0] = '\0';
        pQualifiedHostBuffer[0] = '\0';

        /* get list of addresses */
        pQualifiedHostByName = _osl_gethostbyname_r (
            pFullQualifiedName,
            &aQualifiedHostByName, pQualifiedHostBuffer,
            sizeof(pQualifiedHostBuffer), &nErrorNo );
        pHostByName = _osl_gethostbyname_r (
            pHostName,
            &aHostByName, pHostBuffer,
            sizeof(pHostBuffer), &nErrorNo );

        /* compare addresses */
        bHostsAreEqual = False;
        if ( pQualifiedHostByName && pHostByName )
        {
            sal_Char **p, **q;
            struct in_addr in;

            /* lists are expected to be (very) short */
            for ( p = pQualifiedHostByName->h_addr_list; *p != NULL; p++ )
            {
                for ( q = pHostByName->h_addr_list; *q != NULL; q++ )
                {
                    /* in.s_addr may be in_addr_t or uint32_t or heaven knows */
                    if ( memcmp( *p, *q, sizeof(in.s_addr) ) == 0 )
                    {
                        bHostsAreEqual = True;
                        break;
                    }
                }
                if ( bHostsAreEqual )
                    break;
            }
        }

        /* very strange case, but have to believe it: reduce the
         * full qualified name to the unqualified host name */
        if ( !bHostsAreEqual )
        {
            OSL_TRACE("_osl_getFullQualifiedDomainName: "
                      "suspect FQDN: %s\n", pFullQualifiedName);

            pFullQualifiedName[ nLengthOfHostName ] = '\0';
            pFullQualifiedName = (sal_Char*)realloc ( pFullQualifiedName,
                                (nLengthOfHostName + 1) * sizeof( sal_Char ));
        }
    }

    /* always return a hostname looked up as carefully as possible
     * this string must be freed by the caller */
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
typedef struct oslHostAddrImpl_t {
    sal_Char            *pHostName;
    struct sockaddr *pSockAddr;
} oslHostAddrImpl;

static oslHostAddr _osl_hostentToHostAddr (const struct hostent *he)
{
    oslHostAddrImpl *pAddr= NULL;
    struct sockaddr *sa;
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

    sa= (struct sockaddr *)malloc (sizeof (struct sockaddr));
    OSL_ASSERT(sa);
    if (sa == NULL)
    {
        free(cn);
        return ((oslHostAddr)NULL);
    }
    memset (sa, 0, sizeof (struct sockaddr));

    sa->sa_family= he->h_addrtype;
    if (sa->sa_family == FAMILY_TO_NATIVE(osl_Socket_FamilyInet))
    {
        struct sockaddr_in *sin= (struct sockaddr_in *)sa;
        memcpy(&(sin->sin_addr.s_addr),
               he->h_addr_list[0],
               he->h_length);
    }
    else if (sa->sa_family == FAMILY_TO_NATIVE(osl_Socket_FamilyIpx))
    {
        struct sockaddr_ipx *spx= (struct sockaddr_ipx *)sa;

        /* not quite sure what ipx/spx defines as a host-address:  */
        /* just the hostnumber of the combination of net- and hostnumber */
        /* I guess the later, but am prepared for anything */

        switch (he->h_length)
        {
            case sizeof(spx->sa_netnum) + sizeof(spx->sa_nodenum):
            case sizeof(spx->sa_netnum):
                memcpy (
                    &(spx->sa_netnum),
                    he->h_addr_list[0],
                    he->h_length);
                break;

            case sizeof(spx->sa_nodenum):
                memcpy (
                    &(spx->sa_nodenum),
                    he->h_addr_list[0],
                    he->h_length);
                break;

            default:
                OSL_TRACE("_osl_hostentToHostAddr: "
                          "unknown IPX/SPX address format.\n");
                OSL_ASSERT(sal_False);

                free (sa);
                free (cn);
                return ((oslHostAddr)NULL);
        }
    }
    else
    {
        /* unknown address family */
        /* future extensions for new families might be implemented here */

        OSL_TRACE("_osl_hostentToHostAddr: unknown address family.\n");
        OSL_ASSERT(sal_False);

        free (sa);
        free (cn);
        return ((oslHostAddr)NULL);
    }

    pAddr= (oslHostAddrImpl *)malloc (sizeof (oslHostAddrImpl));
    OSL_ASSERT(pAddr);
    if (pAddr == NULL)
    {
        free (sa);
        free (cn);
        return ((oslHostAddr)NULL);
    }

    pAddr->pHostName= cn;
    pAddr->pSockAddr= sa;

    return ((oslHostAddr)pAddr);
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
    const oslSocketAddr Addr)
{
    oslHostAddrImpl *pAddr;
    struct sockaddr *sa;
    sal_Char            *cn;

    OSL_ASSERT(pszHostname && Addr);
    if ((pszHostname == NULL) || (Addr == NULL))
        return ((oslHostAddr)NULL);

    cn = (sal_Char *)malloc(strlen (pszHostname) + 1);
    OSL_ASSERT(cn);
    if (cn == NULL)
        return ((oslHostAddr)NULL);

    strcpy (cn, pszHostname);

    sa= (struct sockaddr *)osl_copySocketAddr (Addr);
    if (sa == NULL)
    {
        free (cn);
        return ((oslHostAddr)NULL);
    }

    pAddr= (oslHostAddrImpl *)malloc(sizeof (oslHostAddrImpl));
    OSL_ASSERT(pAddr);
    if (pAddr == NULL)
    {
        free (cn);
        free (sa);
        return ((oslHostAddr)NULL);
    }

    pAddr->pHostName= cn;
    pAddr->pSockAddr= sa;

    return ((oslHostAddr)pAddr);
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
    struct hostent *he;
        oslHostAddr addr;

    static oslMutex mutex = NULL;

    if (mutex == NULL)
        mutex = osl_createMutex();

    osl_acquireMutex(mutex);

    he = gethostbyname((sal_Char *)pszHostname);
    addr = _osl_hostentToHostAddr (he);

    osl_releaseMutex(mutex);

    return addr;
}

/*****************************************************************************/
/* osl_createHostAddrByAddr */
/*****************************************************************************/
oslHostAddr SAL_CALL osl_createHostAddrByAddr (const oslSocketAddr Addr)
{
    const struct sockaddr *pAddr= (const struct sockaddr *)Addr;
    OSL_ASSERT(pAddr);

    if (pAddr == NULL)
        return ((oslHostAddr)NULL);

    if (pAddr->sa_family == FAMILY_TO_NATIVE(osl_Socket_FamilyInet))
    {
        const struct sockaddr_in *sin= (const struct sockaddr_in *)pAddr;
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
oslHostAddr SAL_CALL osl_copyHostAddr (const oslHostAddr Addr)
{
    oslHostAddrImpl *pAddr= (oslHostAddrImpl *)Addr;
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

const sal_Char* SAL_CALL osl_psz_getHostnameOfHostAddr (const oslHostAddr Addr)
{
    const oslHostAddrImpl *pAddr= (const oslHostAddrImpl *)Addr;
    OSL_ASSERT(pAddr);

    if (pAddr)
        return pAddr->pHostName;
    else
        return NULL;
}

/*****************************************************************************/
/* osl_getSocketAddrOfHostAddr */
/*****************************************************************************/
oslSocketAddr SAL_CALL osl_getSocketAddrOfHostAddr (const oslHostAddr Addr)
{
    const oslHostAddrImpl *pAddr= (const oslHostAddrImpl *)Addr;
    OSL_ASSERT(pAddr);

    if (pAddr)
        return ((oslSocketAddr)(pAddr->pSockAddr));
    else
        return NULL;
}

/*****************************************************************************/
/* osl_destroyHostAddr */
/*****************************************************************************/
void SAL_CALL osl_destroyHostAddr (oslHostAddr Addr)
{
    oslHostAddrImpl *pAddr= (oslHostAddrImpl *)Addr;
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

        strcpy(LocalHostname, uts.nodename);
#else  /* BSD compatible */

        if (gethostname(LocalHostname, sizeof(LocalHostname)) != 0)
            return osl_Socket_Error;
#endif /* SYSV */

        /* check if we have an FQDN */
        if (strchr(LocalHostname, '.') == NULL)
        {
            oslHostAddr Addr;

            /* no, determine it via dns */
            Addr = osl_psz_createHostAddrByName(LocalHostname);

            if ((pStr = osl_psz_getHostnameOfHostAddr(Addr)) != NULL)
            {
#if 0  /* OBSOLETE */
                sal_Char* pChr;
#endif /* OBSOLETE */
                strcpy(LocalHostname, pStr);

#if 0  /* OBSOLETE */
                /* already done by _osl_getFullQualifiedDomainName() with
                   much better heuristics, so this may be contraproductive */

                /* no FQDN, last try append domain name */
                if ((pChr = strchr(LocalHostname, '.')) == NULL)
                {
                    FILE *fp;

                    pChr = &LocalHostname[strlen(LocalHostname)];

                    if ( (fp = popen("domainname", "r")) != 0 )
                    {
                        int c;

                        *pChr++ = '.';

                        while ((c = getc(fp)) != EOF)
                        {
                            if (! isspace(c))
                                *pChr++ = (sal_Char)c;
                        }

                        *pChr = '\0';

                        fclose(fp);
                    }
                    else
                        LocalHostname[0] = '\0';
                }
#endif /* OBSOLETE */

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
    oslHostAddrImpl *pAddr=
        (oslHostAddrImpl *)osl_psz_createHostAddrByName (pszHostname);

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
void SAL_CALL osl_destroySocketAddr(oslSocketAddr Addr)
{
    if(Addr)
        free(Addr);
}

/*****************************************************************************/
/* osl_getFamilyOfSocketAddr */
/*****************************************************************************/
oslAddrFamily SAL_CALL osl_getFamilyOfSocketAddr(oslSocketAddr Addr)
{
    struct sockaddr* pAddr= (struct sockaddr*)Addr;
    OSL_ASSERT(pAddr);

    if (pAddr)
        return FAMILY_FROM_NATIVE(pAddr->sa_family);
    else
        return osl_Socket_FamilyInvalid;
}

/*****************************************************************************/
/* osl_getInetPortOfSocketAddr */
/*****************************************************************************/
sal_Int32 SAL_CALL osl_getInetPortOfSocketAddr(oslSocketAddr Addr)
{
    struct sockaddr_in* pAddr= (struct sockaddr_in*)Addr;
    OSL_ASSERT(pAddr);

    if (pAddr && (pAddr->sin_family == FAMILY_TO_NATIVE(osl_Socket_FamilyInet)))
        return ntohs(pAddr->sin_port);
    else
        return OSL_INVALID_PORT;
}

/*****************************************************************************/
/* osl_setInetPortOfSocketAddr */
/*****************************************************************************/
sal_Bool SAL_CALL osl_setInetPortOfSocketAddr(oslSocketAddr Addr, sal_Int32 Port)
{
    struct sockaddr_in* pAddr= (struct sockaddr_in*)Addr;
    OSL_ASSERT(pAddr);

    if (pAddr && (pAddr->sin_family == FAMILY_TO_NATIVE(osl_Socket_FamilyInet)))
    {
        pAddr->sin_port= htons((short)Port);

        return sal_True;
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


oslSocketResult SAL_CALL osl_psz_getHostnameOfSocketAddr(oslSocketAddr Addr,
                                            sal_Char *pBuffer, sal_uInt32 BufferSize)
{
    oslHostAddrImpl *pAddr= (oslHostAddrImpl *)osl_createHostAddrByAddr(Addr);

    if (pAddr)
    {
        strncpy(pBuffer, pAddr->pHostName, BufferSize);

        pBuffer[BufferSize - 1] = '\0';

        osl_destroyHostAddr(pAddr);

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

oslSocketResult SAL_CALL osl_psz_getDottedInetAddrOfSocketAddr(oslSocketAddr Addr,
                                                  sal_Char *pBuffer, sal_uInt32 BufferSize)
{
    struct sockaddr_in* pAddr;

    pAddr= (struct sockaddr_in*)Addr;
    OSL_ASSERT(pAddr);

    if (pAddr == NULL)
        return osl_Socket_Error;

    if (pAddr->sin_family != FAMILY_TO_NATIVE(osl_Socket_FamilyInet))
        return osl_Socket_Error;

    strncpy(pBuffer, inet_ntoa(pAddr->sin_addr), BufferSize);
    pBuffer[BufferSize - 1] = '\0';

    return osl_Socket_Ok;
}


/*****************************************************************************/
/* osl_getIpxNetNumber  */
/*****************************************************************************/
oslSocketResult SAL_CALL osl_getIpxNetNumber(oslSocketAddr Addr,
                                    oslSocketIpxNetNumber NetNumber)

{
    struct sockaddr_ipx* pAddr;

    pAddr= (struct sockaddr_ipx*)Addr;

    OSL_ASSERT(pAddr);

    if (pAddr && (pAddr->sa_family == FAMILY_TO_NATIVE(osl_Socket_FamilyIpx)))
    {
        memcpy(NetNumber, pAddr->sa_netnum, sizeof(NetNumber));

        return osl_Socket_Ok;
    }
    else
        return osl_Socket_Error;
}


/*****************************************************************************/
/* osl_getIpxNodeNumber  */
/*****************************************************************************/
oslSocketResult SAL_CALL osl_getIpxNodeNumber(oslSocketAddr Addr,
                                     oslSocketIpxNodeNumber NodeNumber)

{
    struct sockaddr_ipx* pAddr;

    pAddr= (struct sockaddr_ipx*)Addr;

    OSL_ASSERT(pAddr);

    if (pAddr && (pAddr->sa_family == FAMILY_TO_NATIVE(osl_Socket_FamilyIpx)))
    {
        memcpy(NodeNumber, pAddr->sa_nodenum, sizeof(NodeNumber));

        return osl_Socket_Ok;
    }
    else
        return osl_Socket_Error;
}


/*****************************************************************************/
/* osl_getIpxSocketNumber  */
/*****************************************************************************/
sal_Int32 SAL_CALL osl_getIpxSocketNumber(oslSocketAddr Addr)
{
    struct sockaddr_ipx* pAddr= (struct sockaddr_ipx*)Addr;
    OSL_ASSERT(pAddr);

    if (pAddr && (pAddr->sa_family == FAMILY_TO_NATIVE(osl_Socket_FamilyIpx)))
        return pAddr->sa_socket;
    else
        return OSL_INVALID_IPX_SOCKET_NO;
}

/*****************************************************************************/
/* osl_createSocket  */
/*****************************************************************************/
oslSocket SAL_CALL osl_createSocket(oslAddrFamily   Family,
                           oslSocketType    Type,
                           oslProtocol      Protocol)
{
    int            Flags;
    oslSocketImpl* pSockImpl;

    /* alloc memory */
    pSockImpl= __osl_createSocketImpl(OSL_INVALID_SOCKET);

    /* create socket */
    pSockImpl->m_Socket= socket(FAMILY_TO_NATIVE(Family),
                                TYPE_TO_NATIVE(Type),
                                PROTOCOL_TO_NATIVE(Protocol));

    /* creation failed => free memory */
    if(pSockImpl->m_Socket == OSL_INVALID_SOCKET)
    {
        OSL_TRACE("osl_createSocket failed. Errno: %d; %s\n",
                  errno,
                  strerror(errno));

        __osl_destroySocketImpl(pSockImpl);
        pSockImpl= 0;
    }
    else
    {
        /* set close-on-exec flag */
        if ((Flags = fcntl(pSockImpl->m_Socket, F_GETFD, 0)) != -1)
        {
            Flags |= FD_CLOEXEC;
            if (fcntl(pSockImpl->m_Socket, F_SETFD, Flags) == -1)
            {
                pSockImpl->m_nLastError=errno;
                OSL_TRACE("osl_createSocket failed changing socket flags. Errno: %d; %s\n",
                          errno,
                          strerror(errno));
            }
        }
        else
        {
            pSockImpl->m_nLastError=errno;
        }


        pSockImpl->m_CloseCallback  = NULL;
        pSockImpl->m_CallbackArg    = NULL;
    }

    return (oslSocket)pSockImpl;
}

/*****************************************************************************/
/* osl_copySocket  */
/*****************************************************************************/
oslSocket SAL_CALL osl_copySocket(oslSocket Socket)
{
    oslSocketImpl* pSockImpl;
    oslSocketImpl* pParamSockImpl;

    OSL_ASSERT(Socket);
    if ( Socket == 0 )
    {
        return 0;
    }

    /* alloc memory */
    pSockImpl= __osl_createSocketImpl(OSL_INVALID_SOCKET);

    OSL_ASSERT(pSockImpl);

    if(pSockImpl==0)
        return 0;

    pParamSockImpl= (oslSocketImpl*)Socket;

    /* copy socket */
    memcpy(pSockImpl, pParamSockImpl, sizeof(oslSocketImpl));

    pSockImpl->m_nLastError=0;

    return (oslSocket)pSockImpl;
}


/*****************************************************************************/
/* osl_destroySocket  */
/*****************************************************************************/
void SAL_CALL osl_destroySocket(oslSocket Socket)
{
    oslSocketImpl* pSockImpl = (oslSocketImpl*)Socket;

    /* socket already invalid */
    if(Socket==0)
        return;

#if defined(LINUX)
    if ( pSockImpl->m_bIsAccepting == sal_True )
    {
        OSL_ENSURE(0, "osl_destroySocket : attempt to destroy socket while accepting\n");
        return;
    }
#endif

    osl_closeSocket(Socket);

    /* free memory */
    __osl_destroySocketImpl(pSockImpl);
}

/*****************************************************************************/
/* osl_closeSocket  */
/*****************************************************************************/
void SAL_CALL osl_closeSocket(oslSocket Socket)
{
    oslSocketImpl* pSockImpl;
    int nRet;
    int nFD;

    /* socket already invalid */
    if(Socket==0)
        return;

    pSockImpl= (oslSocketImpl*)Socket;
    pSockImpl->m_nLastError=0;
    nFD = pSockImpl->m_Socket;

    pSockImpl->m_Socket = OSL_INVALID_SOCKET;

#if defined(LINUX)
    pSockImpl->m_bIsInShutdown = sal_True;

    if ( pSockImpl->m_bIsAccepting == sal_True )
    {
        int nConnFD;
        struct sockaddr aSockAddr;
        socklen_t nSockLen = sizeof(aSockAddr);

        nRet = getsockname(nFD, &aSockAddr, &nSockLen);
#if defined(DEBUG)
        if ( nRet < 0 )
        {
            perror("getsockname");
        }
#endif /* DEBUG */

        if ( aSockAddr.sa_family == AF_INET )
        {
            struct sockaddr_in* pSockAddrIn = (struct sockaddr_in*) &aSockAddr;

            if ( pSockAddrIn->sin_addr.s_addr == htonl(INADDR_ANY) )
            {
                pSockAddrIn->sin_addr.s_addr = htonl(INADDR_LOOPBACK);
            }

            nConnFD = socket(AF_INET, SOCK_STREAM, 0);
#if defined(DEBUG)
            if ( nConnFD < 0 )
            {
                perror("socket");
            }
#endif /* DEBUG */

            nRet = connect(nConnFD, &aSockAddr, sizeof(aSockAddr));
#if defined(DEBUG)
            if ( nRet < 0 )
            {
                perror("connect");
            }
#endif /* DEBUG */
            close(nConnFD);
        }
    }
#endif /* LINUX */

    /* registrierten Callback ausfuehren */
    if (pSockImpl->m_CloseCallback != NULL)
    {
        pSockImpl->m_CloseCallback(pSockImpl->m_CallbackArg);
    }

    nRet=close(nFD);
    if ( nRet != 0 )
    {
        pSockImpl->m_nLastError=errno;
        OSL_TRACE("closeSocket close error '%s'\n",strerror(errno));
    }

    pSockImpl->m_Socket = OSL_INVALID_SOCKET;
}

/*****************************************************************************/
/* osl_getLocalAddrOfSocket  */
/* Note that I rely on the fact that oslSocketAddr and struct sockaddr */
/* are the same! I don't like it very much but see no other easy way to conceal */
/* the struct sockaddr from the eyes of the user. */
/*****************************************************************************/
oslSocketAddr SAL_CALL osl_getLocalAddrOfSocket(oslSocket Socket)
{
    oslSocketImpl* pSockImpl;
#ifdef LINUX
    socklen_t AddrLen;
#else
    /* mfe: Solaris 'cc +w' means Addrlen should be signed! */
    /*      it's really defined as 'int*' in /usr/include/sys/socket.h! */
    /*      the man page says it expects a 'size_t' */
    int AddrLen;
#endif
    struct sockaddr Addr;

    OSL_ASSERT(Socket);
    if ( Socket == 0 )
    {
        return 0;
    }

    pSockImpl= (oslSocketImpl*)Socket;
    pSockImpl->m_nLastError=0;
    AddrLen= sizeof(struct sockaddr);

    if(getsockname(pSockImpl->m_Socket, &Addr, PTR_SIZE_T(AddrLen)) == OSL_SOCKET_ERROR)
    {
        pSockImpl->m_nLastError=errno;
        return 0;
    }


    return osl_copySocketAddr((oslSocketAddr)&Addr);
}

/*****************************************************************************/
/* osl_getPeerAddrOfSocket  */
/*****************************************************************************/
oslSocketAddr SAL_CALL osl_getPeerAddrOfSocket(oslSocket Socket)
{
    oslSocketImpl* pSockImpl;
    sal_uInt32 AddrLen;
    struct sockaddr Addr;

    OSL_ASSERT(Socket);
    if ( Socket == 0 )
    {
        return 0;
    }

    pSockImpl= (oslSocketImpl*)Socket;
    pSockImpl->m_nLastError=0;
    AddrLen= sizeof(struct sockaddr);

    if(getpeername(pSockImpl->m_Socket, &Addr, PTR_SIZE_T(AddrLen)) == OSL_SOCKET_ERROR)
    {
        pSockImpl->m_nLastError=errno;
        return 0;
    }


    return osl_copySocketAddr((oslSocketAddr)&Addr);
}

/*****************************************************************************/
/* osl_bindAddrToSocket  */
/*****************************************************************************/
sal_Bool SAL_CALL osl_bindAddrToSocket(oslSocket Socket,
                             oslSocketAddr Addr)
{
    oslSocketImpl* pSockImpl;
    int nRet;

    OSL_ASSERT(Socket);
    if ( Socket == 0 )
    {
        return sal_False;
    }

    pSockImpl= (oslSocketImpl*)Socket;
    pSockImpl->m_nLastError=0;

    nRet = bind(pSockImpl->m_Socket, (struct sockaddr*)Addr,
                sizeof(struct sockaddr));

    if ( nRet == OSL_SOCKET_ERROR)
    {
        pSockImpl->m_nLastError=errno;
        return sal_False;
    }

    return sal_True;
}


/*****************************************************************************/
/* osl_listenOnSocket  */
/*****************************************************************************/
sal_Bool SAL_CALL osl_listenOnSocket(oslSocket Socket,
                           sal_Int32 MaxPendingConnections)
{
    oslSocketImpl* pSockImpl;
    int nRet;

    OSL_ASSERT(Socket);
    if ( Socket == 0 )
    {
        return sal_False;
    }

    pSockImpl= (oslSocketImpl*)Socket;
    pSockImpl->m_nLastError=0;

    nRet = listen(pSockImpl->m_Socket,
                  MaxPendingConnections == -1 ?
                  SOMAXCONN :
                  MaxPendingConnections);
    if ( nRet == OSL_SOCKET_ERROR)
    {
        pSockImpl->m_nLastError=errno;
        return sal_False;
    }

    return sal_True;
}


/*****************************************************************************/
/* osl_connectSocketTo  */
/*****************************************************************************/
oslSocketResult SAL_CALL osl_connectSocketTo(oslSocket Socket,
                                    oslSocketAddr Addr,
                                    const TimeValue* pTimeout)
{
    oslSocketImpl* pSockImpl;
    fd_set   WriteSet;
    fd_set   ExcptSet;
    int      ReadyHandles;
    struct timeval  tv;
    oslSocketResult Result= osl_Socket_Ok;

    OSL_PRECOND(Socket, "osl_connectSocketTo(): need a valid socket!\n");

    pSockImpl= (oslSocketImpl*)Socket;

    if ( pSockImpl == 0 )
    {
        return osl_Socket_Error;
    }

    pSockImpl->m_nLastError=0;

    if (osl_isNonBlockingMode(Socket))
    {
        if (connect(pSockImpl->m_Socket,
                    (struct sockaddr*)Addr,
                    sizeof(struct sockaddr)) != OSL_SOCKET_ERROR)
            return osl_Socket_Ok;
        else
            if (errno == EWOULDBLOCK || errno == EINPROGRESS)
            {
                pSockImpl->m_nLastError=EINPROGRESS;
                return osl_Socket_InProgress;
            }


        pSockImpl->m_nLastError=errno;
        OSL_TRACE("can't connect : '%s'",strerror(errno));
        return osl_Socket_Error;
    }

    /* set socket temporarily to non-blocking */
    OSL_VERIFY(osl_enableNonBlockingMode(Socket, sal_True));

    /* initiate connect */
    if(connect(pSockImpl->m_Socket,
               (struct sockaddr*)Addr,
               sizeof(struct sockaddr)) != OSL_SOCKET_ERROR)
    {
       /* immediate connection */
        osl_enableNonBlockingMode(Socket, sal_False);

        return osl_Socket_Ok;
    }
    else
    {
        /* really an error or just delayed? */
        if (errno != EINPROGRESS)
        {
            pSockImpl->m_nLastError=errno;
            OSL_TRACE(
                "osl_connectSocketTo(): connect failed: errno: %d (%s)\n",
                errno, strerror(errno));

            osl_enableNonBlockingMode(Socket, sal_False);
            return osl_Socket_Error;
        }
    }


    /* prepare select set for socket  */
    FD_ZERO(&WriteSet);
    FD_ZERO(&ExcptSet);
    FD_SET(pSockImpl->m_Socket, &WriteSet);
    FD_SET(pSockImpl->m_Socket, &ExcptSet);

    /* prepare timeout */
    if (pTimeout)
    {
        /* divide milliseconds into seconds and microseconds */
        tv.tv_sec=  pTimeout->Seconds;
        tv.tv_usec= pTimeout->Nanosec / 1000L;
    }

    /* select */
    ReadyHandles= select(pSockImpl->m_Socket+1,
                         0,
                         PTR_FD_SET(WriteSet),
                         PTR_FD_SET(ExcptSet),
                         (pTimeout) ? &tv : 0);

    if (ReadyHandles > 0)  /* connected */
    {
        if ( FD_ISSET(pSockImpl->m_Socket, &WriteSet ) )
        {
            int nErrorCode = 0;
#ifdef SOLARIS
/*  mfe: Solaris 'cc +w' means 5th argument should be a 'int*'!
         it's really defined as 'int*' in /usr/include/sys/socket.h!
         the man page says it expects a 'size_t*'
*/
            int nErrorSize = sizeof( nErrorCode );
#else
            size_t nErrorSize = sizeof( nErrorCode );
#endif

            int nSockOpt;

            nSockOpt = getsockopt ( pSockImpl->m_Socket, SOL_SOCKET, SO_ERROR,
#ifdef SOLARIS
/*  mfe: Solaris 'cc +w' means 4th argument should be a 'char*'!
         it's really defined as 'char*' in /usr/include/sys/socket.h!
         the man page says it expects a 'void*'
*/
                                    (char*)
#endif
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
            pSockImpl->m_nLastError=errno;
            Result= osl_Socket_Error;
        }
    }
    else    /* timeout */
    {
        pSockImpl->m_nLastError=errno;
        Result= osl_Socket_TimedOut;
    }

    osl_enableNonBlockingMode(Socket, sal_False);

    return Result;
}


/*****************************************************************************/
/* osl_acceptConnectionOnSocket  */
/*****************************************************************************/
oslSocket SAL_CALL osl_acceptConnectionOnSocket(oslSocket Socket,
                        oslSocketAddr* pAddr)
{
    oslSocketImpl* pSockImpl;
    struct sockaddr Addr;
    int Connection, Flags;
    sal_uInt32 AddrLen = sizeof(struct sockaddr);
    oslSocketImpl* pConnectionSockImpl;

    OSL_ASSERT(Socket);
    if ( Socket == 0 )
    {
        return 0;
    }

    pSockImpl= (oslSocketImpl*)Socket;
    pSockImpl->m_nLastError=0;
#if defined(LINUX)
    pSockImpl->m_bIsAccepting = sal_True;
#endif

    /* prevent Linux EINTR behaviour */
    do
    {
        Connection = accept(pSockImpl->m_Socket, (struct sockaddr*)&Addr, PTR_SIZE_T(AddrLen));
    } while (Connection == -1 && errno == EINTR);


    /* accept failed? */
    if( Connection == OSL_SOCKET_ERROR )
    {
        pSockImpl->m_nLastError=errno;

#if defined(DEBUG)
        fprintf(stderr,"osl_acceptConnectionOnSocket : accept error '%s'\n",strerror(errno));
#endif

#if defined(LINUX)
        pSockImpl->m_bIsAccepting = sal_False;
#endif
        return 0;
    }

    OSL_ASSERT(AddrLen == sizeof(struct sockaddr));


#if defined(LINUX)
    if ( pSockImpl->m_bIsInShutdown == sal_True )
    {
        close(Connection);
#if defined(DEBUG)
        fprintf(stderr,"osl_acceptConnectionOnSocket : close while accept\n");
#endif
        return 0;
    }
#endif


    if(pAddr)
    {
        *pAddr= osl_copySocketAddr((oslSocketAddr)&Addr);
    }

    /* alloc memory */
    pConnectionSockImpl= __osl_createSocketImpl(OSL_INVALID_SOCKET);

    /* set close-on-exec flag */
    if ((Flags = fcntl(Connection, F_GETFD, 0)) != -1)
    {
        Flags |= FD_CLOEXEC;
        if (fcntl(Connection, F_SETFD, Flags) == -1)
        {
            pSockImpl->m_nLastError=errno;
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

    pSockImpl->m_bIsAccepting = sal_False;
#endif



    return (oslSocket)pConnectionSockImpl;
}

/*****************************************************************************/
/* osl_receiveSocket  */
/*****************************************************************************/
sal_Int32 SAL_CALL osl_receiveSocket(oslSocket Socket,
                          void* pBuffer,
                          sal_uInt32 BytesToRead,
                          oslSocketMsgFlag Flag)
{
    oslSocketImpl* pSockImpl;
    int nRead;

    OSL_ASSERT(Socket);
    if ( Socket == 0 )
    {
        OSL_TRACE("osl_receiveSocket : Invalid socket");
        return -1;
    }

    pSockImpl = (oslSocketImpl*)Socket;
    pSockImpl->m_nLastError=0;

    do
    {
        nRead =  recv(pSockImpl->m_Socket,
                      (sal_Char*)pBuffer,
                      BytesToRead,
                      MSG_FLAG_TO_NATIVE(Flag));
    } while ( nRead < 0 && errno == EINTR );


/*      OSL_TRACE("osl_receiveSocket : read on FD '%i' '%i' Bytes\n",pSockImpl->m_Socket, BytesToRead); */

    if ( nRead < 0 )
    {
        pSockImpl->m_nLastError=errno;
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
sal_Int32 SAL_CALL osl_receiveFromSocket(oslSocket Socket,
                              oslSocketAddr SenderAddr,
                              void* pBuffer,
                              sal_uInt32 BufferSize,
                              oslSocketMsgFlag Flag)
{
    oslSocketImpl* pSockImpl;
    int nRead;
    sal_uInt32 AddrLen= SenderAddr == 0 ? 0 : sizeof(struct sockaddr);

    OSL_ASSERT(Socket);
    if ( Socket == 0 )
    {
        OSL_TRACE("osl_receiveFromSocket : Invalid socket");
        return -1;
    }

    pSockImpl= (oslSocketImpl*)Socket;
    pSockImpl->m_nLastError=0;

    nRead = recvfrom(pSockImpl->m_Socket,
                     (sal_Char*)pBuffer,
                     BufferSize,
                     MSG_FLAG_TO_NATIVE(Flag),
                     (struct sockaddr*)SenderAddr,
                     PTR_SIZE_T(AddrLen));

/*      OSL_TRACE("osl_receiveFromSocket : read on FD '%i' '%i' Bytes\n",pSockImpl->m_Socket, BufferSize); */

    if ( nRead < 0 )
    {
        pSockImpl->m_nLastError=errno;
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
sal_Int32 SAL_CALL osl_sendSocket(oslSocket Socket,
                       const void* pBuffer,
                       sal_uInt32 BytesToSend,
                       oslSocketMsgFlag Flag)
{
    oslSocketImpl* pSockImpl;
    int nWritten;

    OSL_ASSERT(Socket);
    if ( Socket == 0 )
    {
        OSL_TRACE("osl_sendSocket : Invalid socket");
        return -1;
    }

    pSockImpl = (oslSocketImpl*)Socket;
    pSockImpl->m_nLastError=0;

/*      OSL_TRACE("osl_sendSocket : sending on FD '%i' '%i' Bytes\n",pSockImpl->m_Socket, BytesToSend); */

    do
    {
        nWritten = send(pSockImpl->m_Socket,
                        (sal_Char*)pBuffer,
                        BytesToSend,
                        MSG_FLAG_TO_NATIVE(Flag));
    } while ( nWritten < 0 && errno == EINTR );


    if ( nWritten < 0 )
    {
        pSockImpl->m_nLastError=errno;
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
sal_Int32 SAL_CALL osl_sendToSocket(oslSocket Socket,
                         oslSocketAddr ReceiverAddr,
                         const void* pBuffer,
                         sal_uInt32 BytesToSend,
                         oslSocketMsgFlag Flag)
{
    oslSocketImpl* pSockImpl;
    int nWritten;

    OSL_ASSERT(Socket);
    if ( Socket == 0 )
    {
        OSL_TRACE("osl_sendToSocket : Invalid socket");
        return -1;
    }

    pSockImpl= (oslSocketImpl*)Socket;
    pSockImpl->m_nLastError=0;

    /* ReceiverAddr might be 0 when used on a connected socket. */
    /* Then sendto should behave like send. */

/*      OSL_TRACE("osl_sendToSocket : sending on FD '%i' '%i' Bytes\n",pSockImpl->m_Socket, BytesToSend); */

    nWritten = sendto(pSockImpl->m_Socket,
                      (sal_Char*)pBuffer,
                      BytesToSend,
                      MSG_FLAG_TO_NATIVE(Flag),
                      (struct sockaddr*)ReceiverAddr,
                      ReceiverAddr == 0 ? 0 : sizeof(struct sockaddr));

    if ( nWritten < 0 )
    {
        pSockImpl->m_nLastError=errno;
        OSL_TRACE("osl_sendToSocket failed : %i '%s'",nWritten,strerror(errno));
    }
    else if ( nWritten == 0 )
    {
        OSL_TRACE("osl_sendToSocket failed : %i '%s'",nWritten,"EOL");
    }

    return nWritten;
}


/*****************************************************************************/
/* osl_isReceiveReady  */
/*****************************************************************************/
sal_Bool SAL_CALL osl_isReceiveReady(oslSocket Socket, const TimeValue* pTimeout)
{
    fd_set fds;
    struct timeval tv;
    oslSocketImpl* pSockImpl;
    int result;

    OSL_ASSERT(Socket);
    if ( Socket == 0 )
    {
        return sal_False;
    }

    pSockImpl= (oslSocketImpl*)Socket;
    pSockImpl->m_nLastError=0;

    FD_ZERO(&fds);
    FD_SET(pSockImpl->m_Socket, &fds);

    if (pTimeout)
    {
        tv.tv_sec  = pTimeout->Seconds;
        tv.tv_usec = pTimeout->Nanosec / 1000L;
    }

    result= select(pSockImpl->m_Socket+1,       /* highest socketno to monitor */
                   PTR_FD_SET(fds),             /* check read operations */
                   0,                           /* check write ops */
                   0,                           /* ckeck for OOB */
                   (pTimeout) ? &tv : 0);       /* use timeout? */

    if(result < 0)     /* error */
    {
        pSockImpl->m_nLastError=errno;
        OSL_TRACE("osl_isReceiveReady(): select-error: %d (%s)\n", errno, strerror(errno));
        return sal_False;
    }

    if(result == 0)    /* timeout */
    {
        pSockImpl->m_nLastError=errno;
        return sal_False;
    }

    return sal_True;
}


/*****************************************************************************/
/* osl_isSendReady  */
/*****************************************************************************/
sal_Bool SAL_CALL osl_isSendReady(oslSocket Socket, const TimeValue* pTimeout)
{
    fd_set fds;
    struct timeval tv;
    oslSocketImpl* pSockImpl;
    int nRet;

    OSL_ASSERT(Socket);
    if ( Socket == 0 )
    {
        return sal_False;
    }

    pSockImpl= (oslSocketImpl*)Socket;
    pSockImpl->m_nLastError=0;

    FD_ZERO(&fds);
    FD_SET(pSockImpl->m_Socket, &fds);

    if (pTimeout)
    {
        tv.tv_sec  = pTimeout->Seconds;
        tv.tv_usec = pTimeout->Nanosec / 1000L;
    }

    nRet = select(pSockImpl->m_Socket+1,        /* highest socketno to monitor */
                  0,                            /* check read operations */
                  PTR_FD_SET(fds),              /* check write ops */
                  0,                            /* ckeck for OOB */
                  (pTimeout) ? &tv : 0);    /* use timeout? */

    if ( nRet < 0 )
    {
        pSockImpl->m_nLastError=errno;
        OSL_TRACE("osl_isSendReady(): select-error: %d (%s)\n", errno, strerror(errno));
        return sal_False;
    }

    if( nRet == 0 )
    {
        pSockImpl->m_nLastError=errno;
        return sal_False;
    }

    return sal_True;
}

/*****************************************************************************/
/* osl_isExceptionPending  */
/*****************************************************************************/
sal_Bool SAL_CALL osl_isExceptionPending(oslSocket Socket, const TimeValue* pTimeout)
{
    fd_set fds;
    struct timeval tv;
    oslSocketImpl* pSockImpl;
    int nRet;

    OSL_ASSERT(Socket);
    if ( Socket == 0 )
    {
        return sal_False;
    }

    pSockImpl= (oslSocketImpl*)Socket;
    pSockImpl->m_nLastError=0;

    FD_ZERO(&fds);
    FD_SET(pSockImpl->m_Socket, &fds);

    if (pTimeout)
    {
        tv.tv_sec  = pTimeout->Seconds;
        tv.tv_usec = pTimeout->Nanosec / 1000L;
    }

    nRet = select(pSockImpl->m_Socket+1,        /* highest socketno to monitor */
                  0,                            /* check read operations */
                  0,                            /* check write ops */
                  PTR_FD_SET(fds),              /* ckeck for OOB */
                  (pTimeout) ? &tv : 0);        /* use timeout? */

    if ( nRet < 0 )
    {
        pSockImpl->m_nLastError=errno;
        OSL_TRACE("osl_isExceptionPending(): select-error: %d (%s)\n", errno, strerror(errno));
        return sal_False;
    }

    if( nRet == 0 )
    {
        pSockImpl->m_nLastError=errno;
        return sal_False;
    }

    return sal_True;
}

/*****************************************************************************/
/* osl_shutdownSocket  */
/*****************************************************************************/
sal_Bool SAL_CALL osl_shutdownSocket(oslSocket Socket,
                           oslSocketDirection Direction)
{
    oslSocketImpl* pSockImpl;
    int nRet;

    OSL_ASSERT(Socket);
    if ( Socket == 0 )
    {
        return sal_False;
    }

    pSockImpl= (oslSocketImpl*)Socket;
    pSockImpl->m_nLastError=0;

    nRet=shutdown(pSockImpl->m_Socket, DIRECTION_TO_NATIVE(Direction));
    if (nRet != 0 )
    {
        pSockImpl->m_nLastError=errno;
#ifdef DEBUG
        fprintf(stderr,"shutdown error '%s'\n",strerror(errno));
#endif
    }
    return (nRet==0);
}


/*****************************************************************************/
/* osl_getSocketOption  */
/*****************************************************************************/
sal_Int32 SAL_CALL osl_getSocketOption(oslSocket Socket,
                            oslSocketOptionLevel    Level,
                            oslSocketOption         Option,
                            void*                   pBuffer,
                            sal_uInt32                  BufferLen)
{
    oslSocketImpl* pSockImpl;

    OSL_ASSERT(Socket);
    if ( Socket == 0 )
    {
        return -1;
    }

    pSockImpl= (oslSocketImpl*)Socket;
    pSockImpl->m_nLastError=0;

    if(getsockopt(pSockImpl->m_Socket,
                  OPTION_LEVEL_TO_NATIVE(Level),
                  OPTION_TO_NATIVE(Option),
                  (sal_Char*)pBuffer,
                  PTR_SIZE_T(BufferLen)) == -1)
    {
        pSockImpl->m_nLastError=errno;
        return -1;
    }

    return BufferLen;
}

/*****************************************************************************/
/* osl_setSocketOption  */
/*****************************************************************************/
sal_Bool SAL_CALL osl_setSocketOption(oslSocket Socket,
                            oslSocketOptionLevel    Level,
                            oslSocketOption         Option,
                            void*                   pBuffer,
                            sal_uInt32                  BufferLen)
{
    oslSocketImpl* pSockImpl;
    int nRet;

    OSL_ASSERT(Socket);
    if ( Socket == 0 )
    {
        return sal_False;
    }

    pSockImpl= (oslSocketImpl*)Socket;
    pSockImpl->m_nLastError=0;

    nRet = setsockopt(pSockImpl->m_Socket,
                      OPTION_LEVEL_TO_NATIVE(Level),
                      OPTION_TO_NATIVE(Option),
                      (sal_Char*)pBuffer,
                      BufferLen);

    if ( nRet < 0 )
    {
        pSockImpl->m_nLastError=errno;
        return sal_False;
    }

    return sal_True;
}

/*****************************************************************************/
/* osl_enableNonBlockingMode  */
/*****************************************************************************/
sal_Bool SAL_CALL osl_enableNonBlockingMode(oslSocket Socket,
                                  sal_Bool On)
{
    int flags;
    oslSocketImpl* pSockImpl;
    int nRet;

    OSL_ASSERT(Socket);
    if ( Socket == 0 )
    {
        return sal_False;
    }

    pSockImpl= (oslSocketImpl*)Socket;
    pSockImpl->m_nLastError=0;

    flags = fcntl(pSockImpl->m_Socket, F_GETFL, 0);

    if (On)
        flags |= O_NONBLOCK;
    else
        flags &= ~(O_NONBLOCK);

    nRet = fcntl(pSockImpl->m_Socket, F_SETFL, flags);

    if  ( nRet < 0 )
    {
        pSockImpl->m_nLastError=errno;
        return sal_False;
    }

    return sal_True;
}

/*****************************************************************************/
/* osl_isNonBlockingMode  */
/*****************************************************************************/
sal_Bool SAL_CALL osl_isNonBlockingMode(oslSocket Socket)
{
    int flags;
    oslSocketImpl* pSockImpl;

    OSL_ASSERT(Socket);
    if ( Socket == 0 )
    {
        return sal_False;
    }

    pSockImpl= (oslSocketImpl*)Socket;
    pSockImpl->m_nLastError=0;

    flags = fcntl(pSockImpl->m_Socket, F_GETFL, 0);

    if (flags == -1 || !(flags & O_NONBLOCK))
        return sal_False;
    else
        return sal_True;
}

/*****************************************************************************/
/* osl_getSocketType  */
/*****************************************************************************/
oslSocketType SAL_CALL osl_getSocketType(oslSocket Socket)
{
    oslSocketImpl* pSockImpl;
    int Type=0;
    sal_uInt32 TypeSize= sizeof(Type);

    OSL_ASSERT(Socket);
    if ( Socket == 0 )
    {
        return osl_Socket_TypeInvalid;
    }

    pSockImpl= (oslSocketImpl*)Socket;
    pSockImpl->m_nLastError=0;

    if(getsockopt(pSockImpl->m_Socket,
                  OPTION_LEVEL_TO_NATIVE(osl_Socket_LevelSocket),
                  OPTION_TO_NATIVE(osl_Socket_OptionType),
                  (sal_Char*)&Type,
                  PTR_SIZE_T(TypeSize)) == -1)
    {
        /* error */
        pSockImpl->m_nLastError=errno;
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


void SAL_CALL osl_psz_getLastSocketErrorDescription(oslSocket Socket, sal_Char* pBuffer, sal_uInt32 BufferSize)
{
    oslSocketImpl* pSockImpl = (oslSocketImpl*) Socket;

    /* make shure pBuffer will be a zero-terminated string even when strncpy has to cut */
    pBuffer[BufferSize-1]= '\0';

    if ( pSockImpl == 0 )
    {
        strncpy(pBuffer, strerror(EINVAL), BufferSize-1);
        return;
    }

    strncpy(pBuffer, strerror(pSockImpl->m_nLastError), BufferSize-1);
    return;
}

/*****************************************************************************/
/* osl_getLastSocketError  */
/*****************************************************************************/
oslSocketError SAL_CALL osl_getLastSocketError(oslSocket Socket)
{
    oslSocketImpl* pSockImpl = (oslSocketImpl*) Socket;

    if ( pSockImpl == 0 )
    {
        return ERROR_FROM_NATIVE(EINVAL);
    }

    return ERROR_FROM_NATIVE(pSockImpl->m_nLastError);
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
void SAL_CALL osl_addToSocketSet(oslSocketSet Set, oslSocket Socket)
{
    oslSocketImpl* pSockImpl;
    TSocketSetImpl* pSet;

    OSL_ASSERT(Set);
    OSL_ASSERT(Socket);

    if ( Set == 0 || Socket == 0)
    {
        return;
    }

    pSockImpl= (oslSocketImpl*)Socket;
    pSet= (TSocketSetImpl*)Set;

    /* correct max handle */
    if(pSockImpl->m_Socket > pSet->m_MaxHandle)
        pSet->m_MaxHandle= pSockImpl->m_Socket;
    FD_SET(pSockImpl->m_Socket, &pSet->m_Set);

}

/*****************************************************************************/
/* osl_removeFromSocketSet  */
/*****************************************************************************/
void SAL_CALL osl_removeFromSocketSet(oslSocketSet Set, oslSocket Socket)
{
    oslSocketImpl* pSockImpl;
    TSocketSetImpl* pSet;

    OSL_ASSERT(Set);
    OSL_ASSERT(Socket);

    if ( Set == 0 || Socket == 0)
    {
        return;
    }

    pSockImpl= (oslSocketImpl*)Socket;
    pSet= (TSocketSetImpl*)Set;

    /* correct max handle */
    if(pSockImpl->m_Socket == pSet->m_MaxHandle)
    {
        /* not optimal, since the next used descriptor might be */
        /* much smaller than m_Socket-1, but it will do */
        pSet->m_MaxHandle--;
        if(pSet->m_MaxHandle < 0)
        {
            pSet->m_MaxHandle= 0;   /* avoid underflow */
        }
    }

    FD_CLR(pSockImpl->m_Socket, &pSet->m_Set);
}

/*****************************************************************************/
/* osl_isInSocketSet  */
/*****************************************************************************/
sal_Bool SAL_CALL osl_isInSocketSet(oslSocketSet Set, oslSocket Socket)
{
    oslSocketImpl* pSockImpl;
    TSocketSetImpl* pSet;

    OSL_ASSERT(Set);
    OSL_ASSERT(Socket);
    if ( Set == 0 || Socket == 0 )
    {
        return sal_False;
    }

    pSockImpl= (oslSocketImpl*)Socket;
    pSet= (TSocketSetImpl*)Set;

    return (FD_ISSET(pSockImpl->m_Socket, &pSet->m_Set) != 0);
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

