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

#include <sal/config.h>

#include <utility>

#include "system.hxx"

#include <osl/socket.h>
#include <osl/mutex.h>
#include <osl/signal.h>

#include <rtl/alloc.h>
#include <rtl/byteseq.h>
#include <rtl/ustring.hxx>
#include <assert.h>
#include <sal/types.h>
#include <sal/log.hxx>

#include "sockimpl.hxx"
#include "unixerrnostring.hxx"

/* defines for poll */
#ifdef HAVE_POLL_H
#undef HAVE_POLL_H
#endif

#if defined(LINUX) || defined(NETBSD) || defined ( FREEBSD ) || \
    defined (MACOSX) || defined (OPENBSD) || defined(DRAGONFLY)
#include <sys/poll.h>
#define HAVE_POLL_H
#endif /* HAVE_POLL_H */

#if defined(__sun)
#include <poll.h>
#define HAVE_POLL_H
#endif /* __sun */

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

static const unsigned long FamilyMap[]= {
    AF_INET,                    /* osl_Socket_FamilyInet    */
    AF_IPX,                     /* osl_Socket_FamilyIpx     */
    0                           /* osl_Socket_FamilyInvalid */
};

static oslAddrFamily osl_AddrFamilyFromNative(sal_uInt32 nativeType)
{
    oslAddrFamily i= oslAddrFamily(0);

    while(i != osl_Socket_FamilyInvalid)
    {
        if(FamilyMap[i] == nativeType)
            return i;
        i = static_cast<oslAddrFamily>( i + 1 );
    }

    return i;
}

#define FAMILY_FROM_NATIVE(y) osl_AddrFamilyFromNative(y)
#define FAMILY_TO_NATIVE(x) static_cast<short>(FamilyMap[x])

static const sal_uInt32 ProtocolMap[]= {
    0,                          /* osl_Socket_ProtocolIp      */
    NSPROTO_IPX,                /* osl_Socket_ProtocolIpx     */
    NSPROTO_SPX,                /* osl_Socket_ProtocolSpx     */
    NSPROTO_SPXII,              /* osl_Socket_ProtocolSpxII   */
    0                           /* osl_Socket_ProtocolInvalid */
};

#define PROTOCOL_TO_NATIVE(x)   ProtocolMap[x]

static const sal_uInt32 TypeMap[]= {
    SOCK_STREAM,                /* osl_Socket_TypeStream    */
    SOCK_DGRAM,                 /* osl_Socket_TypeDgram     */
    SOCK_RAW,                   /* osl_Socket_TypeRaw       */
    SOCK_RDM,                   /* osl_Socket_TypeRdm       */
    SOCK_SEQPACKET,             /* osl_Socket_TypeSeqPacket */
    0                           /* osl_Socket_TypeInvalid   */
};

static oslSocketType osl_SocketTypeFromNative(sal_uInt32 nativeType)
{
    oslSocketType i= oslSocketType(0);

    while(i != osl_Socket_TypeInvalid)
    {
        if(TypeMap[i] == nativeType)
            return i;
        i = static_cast<oslSocketType>(i + 1);
    }

    return i;
}

#define TYPE_TO_NATIVE(x)       TypeMap[x]
#define TYPE_FROM_NATIVE(y)     osl_SocketTypeFromNative(y)

static const sal_uInt32 OptionMap[]= {
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

#define OPTION_TO_NATIVE(x)     OptionMap[x]

static const sal_uInt32 OptionLevelMap[]= {
    SOL_SOCKET,                 /* osl_Socket_LevelSocket  */
    IPPROTO_TCP,                /* osl_Socket_LevelTcp     */
    0                           /* osl_Socket_LevelInvalid */
};

#define OPTION_LEVEL_TO_NATIVE(x)       OptionLevelMap[x]

static const sal_uInt32 SocketMsgFlagMap[]= {
    0,                          /* osl_Socket_MsgNormal    */
    MSG_OOB,                    /* osl_Socket_MsgOOB       */
    MSG_PEEK,                   /* osl_Socket_MsgPeek      */
    MSG_DONTROUTE,              /* osl_Socket_MsgDontRoute */
    MSG_MAXIOVLEN,              /* osl_Socket_MsgMaxIOVLen */
    0                           /* osl_Socket_MsgInvalid   */
};

#define MSG_FLAG_TO_NATIVE(x)       SocketMsgFlagMap[x]

static const sal_uInt32 SocketDirection[]= {
    SD_RECEIVE,                 /* osl_Socket_DirRead      */
    SD_SEND,                    /* osl_Socket_DirWrite     */
    SD_BOTH,                    /* osl_Socket_DirReadWrite */
    0                           /* osl_Socket_DirInvalid   */
};

#define DIRECTION_TO_NATIVE(x)      SocketDirection[x]

static const struct
{
    int const      errcode;
    oslSocketError const error;
} SocketError[]= {
    { 0,               osl_Socket_E_None              }, /* no error */
    { ENOTSOCK,        osl_Socket_E_NotSocket         }, /* Socket operation on non-socket */
    { EDESTADDRREQ,    osl_Socket_E_DestAddrReq       }, /* Destination address required */
    { EMSGSIZE,        osl_Socket_E_MsgSize           }, /* Message too long */
    { EPROTOTYPE,      osl_Socket_E_Prototype         }, /* Protocol wrong type for socket */
    { ENOPROTOOPT,     osl_Socket_E_NoProtocol        }, /* Protocol not available */
    { EPROTONOSUPPORT, osl_Socket_E_ProtocolNoSupport }, /* Protocol not supported */
#ifdef ESOCKTNOSUPPORT
    { ESOCKTNOSUPPORT, osl_Socket_E_TypeNoSupport     }, /* Socket type not supported */
#endif
    { EOPNOTSUPP,      osl_Socket_E_OpNotSupport      }, /* Operation not supported on socket */
    { EPFNOSUPPORT,    osl_Socket_E_PfNoSupport       }, /* Protocol family not supported */
    { EAFNOSUPPORT,    osl_Socket_E_AfNoSupport       }, /* Address family not supported by
                                                            protocol family */
    { EADDRINUSE,      osl_Socket_E_AddrInUse         }, /* Address already in use */
    { EADDRNOTAVAIL,   osl_Socket_E_AddrNotAvail      }, /* Can't assign requested address */
    { ENETDOWN,        osl_Socket_E_NetDown           }, /* Network is down */
    { ENETUNREACH,     osl_Socket_E_NetUnreachable    }, /* Network is unreachable */
    { ENETRESET,       osl_Socket_E_NetReset          }, /* Network dropped connection because
                                                            of reset */
    { ECONNABORTED,    osl_Socket_E_ConnAborted       }, /* Software caused connection abort */
    { ECONNRESET,      osl_Socket_E_ConnReset         }, /* Connection reset by peer */
    { ENOBUFS,         osl_Socket_E_NoBufferSpace     }, /* No buffer space available */
    { EISCONN,         osl_Socket_E_IsConnected       }, /* Socket is already connected */
    { ENOTCONN,        osl_Socket_E_NotConnected      }, /* Socket is not connected */
    { ESHUTDOWN,       osl_Socket_E_Shutdown          }, /* Can't send after socket shutdown */
#ifdef ETOOMANYREFS
    { ETOOMANYREFS,    osl_Socket_E_TooManyRefs       }, /* Too many references: can't splice */
#endif
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

static oslSocketError osl_SocketErrorFromNative(int nativeType)
{
    int i = 0;

    while ((SocketError[i].error != osl_Socket_E_InvalidError) &&
           (SocketError[i].errcode != nativeType)) i++;

    return SocketError[i].error;
}

#define ERROR_FROM_NATIVE(y)    osl_SocketErrorFromNative(y)

static oslSocketAddr osl_psz_createInetSocketAddr (
    const sal_Char* pszDottedAddr, sal_Int32 Port);

static oslHostAddr osl_psz_createHostAddr (
    const sal_Char *pszHostname, const oslSocketAddr Addr);

static oslHostAddr osl_psz_createHostAddrByName (
    const sal_Char *pszHostname);

static const sal_Char* osl_psz_getHostnameOfHostAddr (
    const oslHostAddr Addr);

static oslSocketAddr osl_psz_resolveHostname (
    const sal_Char* pszHostname);

static sal_Int32 osl_psz_getServicePort (
    const sal_Char* pszServicename, const sal_Char* pszProtocol);

static void osl_psz_getLastSocketErrorDescription (
    oslSocket Socket, sal_Char* pBuffer, sal_uInt32 BufferSize);

static oslSocket createSocketImpl(int Socket)
{
    oslSocket pSocket;

    pSocket = static_cast<oslSocket>(calloc(1, sizeof(struct oslSocketImpl)));

    pSocket->m_Socket = Socket;
    pSocket->m_nLastError = 0;
    pSocket->m_nRefCount = 1;

#if defined(CLOSESOCKET_DOESNT_WAKE_UP_ACCEPT)
    pSocket->m_bIsAccepting = false;
#endif

    return pSocket;
}

static void destroySocketImpl(oslSocket Socket)
{
    if ( Socket != nullptr)
        free(Socket);
}

static oslSocketAddr createSocketAddr()
{
    oslSocketAddr pAddr = static_cast<oslSocketAddr>(rtl_allocateZeroMemory( sizeof( struct oslSocketAddrImpl )));
    return pAddr;
}

static oslSocketAddr createSocketAddrWithFamily(
    oslAddrFamily family, sal_Int32 port, sal_uInt32 nAddr )
{
    oslSocketAddr pAddr;

    SAL_WARN_IF( family != osl_Socket_FamilyInet, "sal.osl", "creating socket for non-IP address family" );

    pAddr = createSocketAddr();
    switch( family )
    {
    case osl_Socket_FamilyInet:
    {
        struct sockaddr_in* pInetAddr= reinterpret_cast<sockaddr_in*>(&pAddr->m_sockaddr);

        pInetAddr->sin_family = FAMILY_TO_NATIVE(osl_Socket_FamilyInet);
        pInetAddr->sin_addr.s_addr = nAddr;
        pInetAddr->sin_port = static_cast<sal_uInt16>(port&0xffff);
        break;
       }
    default:
        pAddr->m_sockaddr.sa_family = FAMILY_TO_NATIVE(family);
    }
    return pAddr;
}

static oslSocketAddr createSocketAddrFromSystem( struct sockaddr *pSystemSockAddr )
{
    oslSocketAddr pAddr = createSocketAddr();
    memcpy( &(pAddr->m_sockaddr), pSystemSockAddr, sizeof( struct sockaddr ) );
    return pAddr;
}

static void destroySocketAddr( oslSocketAddr addr )
{
    free( addr );
}

oslSocketAddr SAL_CALL osl_createEmptySocketAddr(oslAddrFamily Family)
{
    oslSocketAddr pAddr = nullptr;

    /* is it an internet-Addr? */
    if (Family == osl_Socket_FamilyInet)
    {
        pAddr = createSocketAddrWithFamily(Family, 0 , htonl(INADDR_ANY) );
    }
    else
    {
        pAddr = createSocketAddrWithFamily( Family , 0 , 0 );
    }

    return pAddr;
}

oslSocketAddr SAL_CALL osl_copySocketAddr(oslSocketAddr Addr)
{
    oslSocketAddr pCopy = nullptr;
    if (Addr)
    {
        pCopy = createSocketAddr();

        if (pCopy)
            memcpy(&(pCopy->m_sockaddr),&(Addr->m_sockaddr), sizeof(struct sockaddr));
    }
    return pCopy;
}

sal_Bool SAL_CALL osl_isEqualSocketAddr (
    oslSocketAddr Addr1,
    oslSocketAddr Addr2)
{
    struct sockaddr* pAddr1 = nullptr;
    struct sockaddr* pAddr2 = nullptr;

    assert(Addr1 && Addr2);
    pAddr1 = &(Addr1->m_sockaddr);
    pAddr2 = &(Addr2->m_sockaddr);

    if (pAddr1 == pAddr2)
    {
        return true;
    }

    if (pAddr1->sa_family == pAddr2->sa_family)
    {
        switch (pAddr1->sa_family)
        {
            case AF_INET:
            {
                struct sockaddr_in* pInetAddr1= reinterpret_cast<sockaddr_in*>(pAddr1);
                struct sockaddr_in* pInetAddr2= reinterpret_cast<sockaddr_in*>(pAddr2);

                if ((pInetAddr1->sin_family == pInetAddr2->sin_family) &&
                    (pInetAddr1->sin_addr.s_addr == pInetAddr2->sin_addr.s_addr) &&
                    (pInetAddr1->sin_port == pInetAddr2->sin_port))
                    return true;
                [[fallthrough]];
            }

            default:
            {
                return (memcmp(pAddr1, pAddr2, sizeof(struct sockaddr)) == 0);
            }
        }
    }

    return false;
}

oslSocketAddr SAL_CALL osl_createInetBroadcastAddr (
    rtl_uString *strDottedAddr,
    sal_Int32    Port)
{
    sal_uInt32    nAddr = OSL_INADDR_NONE;
    oslSocketAddr pAddr;

    if (strDottedAddr && strDottedAddr->length)
    {
        /* Dotted host address for limited broadcast */
        rtl_String *pDottedAddr = nullptr;

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
            return nullptr;
        }
        nAddr = htonl(nAddr);
    }

    pAddr = createSocketAddrWithFamily( osl_Socket_FamilyInet, htons(Port), nAddr );
    return pAddr;
}

oslSocketAddr SAL_CALL osl_createInetSocketAddr (
    rtl_uString *ustrDottedAddr,
    sal_Int32    Port)
{
    rtl_String* strDottedAddr=nullptr;
    oslSocketAddr Addr;
    sal_Char* pszDottedAddr=nullptr;

    if ( ustrDottedAddr != nullptr )
    {
        rtl_uString2String( &strDottedAddr,
                            rtl_uString_getStr(ustrDottedAddr),
                            rtl_uString_getLength(ustrDottedAddr),
                            RTL_TEXTENCODING_UTF8,
                            OUSTRING_TO_OSTRING_CVTFLAGS);
        pszDottedAddr = rtl_string_getStr(strDottedAddr);
    }

    Addr = pszDottedAddr ? osl_psz_createInetSocketAddr(pszDottedAddr, Port) : nullptr;

    if ( strDottedAddr != nullptr )
    {
        rtl_string_release(strDottedAddr);
    }

    return Addr;
}

oslSocketAddr osl_psz_createInetSocketAddr (
    const sal_Char* pszDottedAddr,
    sal_Int32       Port)
{
    oslSocketAddr pAddr = nullptr;
    sal_Int32 Addr = inet_addr(pszDottedAddr);
    if(Addr != -1)
    {
        /* valid dotted addr */
        pAddr = createSocketAddrWithFamily( osl_Socket_FamilyInet, htons(Port) , Addr );
    }
    return pAddr;
}

oslSocketResult SAL_CALL osl_setAddrOfSocketAddr( oslSocketAddr pAddr, sal_Sequence *pByteSeq )
{
    oslSocketResult res = osl_Socket_Error;

    SAL_WARN_IF( !pAddr, "sal.osl", "setting address of undefined socket address" );
    SAL_WARN_IF( !pByteSeq, "sal.osl", "setting undefined address for socket address" );

    if( pAddr && pByteSeq )
    {
        struct sockaddr_in * pSystemInetAddr;

        assert( pAddr->m_sockaddr.sa_family == FAMILY_TO_NATIVE( osl_Socket_FamilyInet ) );
        assert( pByteSeq->nElements == 4 );

        pSystemInetAddr = reinterpret_cast<sockaddr_in *>(&pAddr->m_sockaddr);
        memcpy( &(pSystemInetAddr->sin_addr) , pByteSeq->elements , 4 );
        res = osl_Socket_Ok;
    }
    return res;
}

oslSocketResult SAL_CALL osl_getAddrOfSocketAddr( oslSocketAddr pAddr, sal_Sequence **ppByteSeq )
{
    oslSocketResult res = osl_Socket_Error;

    SAL_WARN_IF( !pAddr, "sal.osl", "getting address of undefined socket address" );
    SAL_WARN_IF( !ppByteSeq, "sal.osl", "getting address to undefined address pointer" );

    if( pAddr && ppByteSeq )
    {
        struct sockaddr_in * pSystemInetAddr = reinterpret_cast<sockaddr_in *>(&pAddr->m_sockaddr);
        rtl_byte_sequence_constructFromArray( ppByteSeq, reinterpret_cast<sal_Int8 *>(&pSystemInetAddr->sin_addr), 4);
        res = osl_Socket_Ok;
    }
    return res;
}

/** try to figure out a full-qualified hostname, by adding the current domain
    as given by the domainname program to the given hostname.
    This function MUST NOT call gethostbyname since pHostName already points
    to data returned by gethostname and would be garbled: use gethostname_r
    instead!
 */

/* wrap around different interfaces to reentrant gethostbyname */
static struct hostent* osl_gethostbyname_r (
    const char *name, struct hostent *result,
    char *buffer, int buflen, int *h_errnop)
{
#if defined(LINUX) || defined(ANDROID) || defined(FREEBSD) || defined(DRAGONFLY)
    struct hostent *result_; /* will be the same as result */
    int e;
    e = gethostbyname_r (name, result, buffer, buflen,
                 &result_, h_errnop);
    return e ? nullptr : result_ ;
#elif defined(AIX)
    *h_errnop = gethostbyname_r (name, result, (struct hostent_data *)buffer);
    (void)buflen;
    return *h_errnop ? NULL : result ;
#else
    return gethostbyname_r( name, result, buffer, buflen, h_errnop);
#endif
}

static bool isFullQualifiedDomainName (const sal_Char *pHostName)
{
    /* a FQDN (aka 'hostname.domain.top_level_domain' )
     * is a name which contains a dot '.' in it ( would
     * match as well for 'hostname.' but is good enough
     * for now )*/
    return strchr( pHostName, int('.') ) != nullptr;
}

static sal_Char* getFullQualifiedDomainName (const sal_Char *pHostName)
{
    sal_Char  *pFullQualifiedName = nullptr;

    if (isFullQualifiedDomainName(pHostName))
    {
        pFullQualifiedName = strdup(pHostName);
    }
    else
    {
        struct hostent  aHostByName;
        struct hostent *pHostByName;
        sal_Char        pQualifiedHostBuffer[ MAX_HOSTBUFFER_SIZE ];
        int     nErrorNo;

        pHostByName = osl_gethostbyname_r (
            pHostName,
            &aHostByName, pQualifiedHostBuffer,
            sizeof(pQualifiedHostBuffer), &nErrorNo );
        if (pHostByName != nullptr)
        {
            pFullQualifiedName = strdup(pHostByName->h_name);
        }
    }

    return pFullQualifiedName;
}

struct oslHostAddrImpl
{
    sal_Char        *pHostName;
    oslSocketAddr   pSockAddr;
};

static oslHostAddr hostentToHostAddr (const struct hostent *he)
{
    oslHostAddr pAddr= nullptr;
    oslSocketAddr pSockAddr = nullptr;

    sal_Char        *cn;

    if ((he == nullptr) || (he->h_name == nullptr) || (he->h_addr_list[0] == nullptr))
        return nullptr;

    cn = getFullQualifiedDomainName (he->h_name);
    SAL_WARN_IF( !cn, "sal.osl", "couldn't get full qualified domain name" );
    if (cn == nullptr)
        return nullptr;

    pSockAddr = createSocketAddr();
    SAL_WARN_IF( !pSockAddr, "sal.osl", "insufficient memory" );
    if (pSockAddr == nullptr)
    {
        free(cn);
        return nullptr;
    }

    pSockAddr->m_sockaddr.sa_family= he->h_addrtype;
    if (pSockAddr->m_sockaddr.sa_family == FAMILY_TO_NATIVE(osl_Socket_FamilyInet))
    {
        struct sockaddr_in *sin= reinterpret_cast<sockaddr_in *>(&pSockAddr->m_sockaddr);
        memcpy (
            &(sin->sin_addr.s_addr),
            he->h_addr_list[0],
            he->h_length);
    }
    else
    {
        /* unknown address family */
        /* future extensions for new families might be implemented here */

        SAL_WARN( "sal.osl", "unknown address family" );

        destroySocketAddr( pSockAddr );
        free (cn);
        return nullptr;
    }

    pAddr= static_cast<oslHostAddr>(malloc(sizeof(struct oslHostAddrImpl)));
    SAL_WARN_IF( !pAddr, "sal.osl", "allocation error" );
    if (pAddr == nullptr)
    {
        destroySocketAddr( pSockAddr );
        free (cn);
        return nullptr;
    }

    pAddr->pHostName= cn;
    pAddr->pSockAddr= pSockAddr;

    return pAddr;
}

oslHostAddr SAL_CALL osl_createHostAddr (
    rtl_uString        *ustrHostname,
    const oslSocketAddr Addr)
{
    oslHostAddr HostAddr;
    rtl_String* strHostname=nullptr;
    sal_Char* pszHostName=nullptr;

    if ( ustrHostname != nullptr )
    {
        rtl_uString2String( &strHostname,
                            rtl_uString_getStr(ustrHostname),
                            rtl_uString_getLength(ustrHostname),
                            RTL_TEXTENCODING_UTF8,
                            OUSTRING_TO_OSTRING_CVTFLAGS );
        pszHostName = rtl_string_getStr(strHostname);
    }

    HostAddr = osl_psz_createHostAddr(pszHostName,Addr);

    if ( strHostname != nullptr )
    {
        rtl_string_release(strHostname);
    }

    return HostAddr;
}

oslHostAddr osl_psz_createHostAddr (
    const sal_Char     *pszHostname,
    const oslSocketAddr pAddr)
{
    oslHostAddr pHostAddr;
    sal_Char            *cn;

    SAL_WARN_IF( !pszHostname, "sal.osl", "undefined hostname" );
    SAL_WARN_IF( !pAddr, "sal.osl", "undefined address" );
    if ((pszHostname == nullptr) || (pAddr == nullptr))
        return nullptr;

    cn = strdup(pszHostname);
    SAL_WARN_IF( !cn, "sal.osl", "insufficient memory" );
    if (cn == nullptr)
        return nullptr;

    pHostAddr= static_cast<oslHostAddr>(malloc(sizeof(struct oslHostAddrImpl)));
    SAL_WARN_IF( !pHostAddr, "sal.osl", "allocation error" );
    if (pHostAddr == nullptr)
    {
        free (cn);
        return nullptr;
    }

    pHostAddr->pHostName= cn;
    pHostAddr->pSockAddr= osl_copySocketAddr( pAddr );

    return pHostAddr;
}

oslHostAddr SAL_CALL osl_createHostAddrByName(rtl_uString *ustrHostname)
{
    oslHostAddr HostAddr;
    rtl_String* strHostname=nullptr;
    sal_Char* pszHostName=nullptr;

    if ( ustrHostname != nullptr )
    {
        rtl_uString2String( &strHostname,
                            rtl_uString_getStr(ustrHostname),
                            rtl_uString_getLength(ustrHostname),
                            RTL_TEXTENCODING_UTF8,
                            OUSTRING_TO_OSTRING_CVTFLAGS );
        pszHostName=rtl_string_getStr(strHostname);
    }

    HostAddr = osl_psz_createHostAddrByName(pszHostName);

    if ( strHostname != nullptr )
    {
        rtl_string_release(strHostname);
    }

    return HostAddr;
}

oslHostAddr osl_psz_createHostAddrByName (const sal_Char *pszHostname)
{
    struct      hostent  aHe;
    struct      hostent *pHe;
    sal_Char    heBuffer[ MAX_HOSTBUFFER_SIZE ];
    int         nErrorNo;

    pHe = osl_gethostbyname_r (
        pszHostname,
        &aHe, heBuffer,
        sizeof(heBuffer), &nErrorNo );

    return hostentToHostAddr (pHe);
}

oslHostAddr SAL_CALL osl_createHostAddrByAddr (const oslSocketAddr pAddr)
{
    SAL_WARN_IF( !pAddr, "sal.osl", "undefined address" );

    if (pAddr == nullptr)
        return nullptr;

    if (pAddr->m_sockaddr.sa_family == FAMILY_TO_NATIVE(osl_Socket_FamilyInet))
    {
        const struct sockaddr_in *sin = reinterpret_cast<sockaddr_in *>(&pAddr->m_sockaddr);
        if (sin->sin_addr.s_addr == htonl(INADDR_ANY))
            return nullptr;

        char host[MAX_HOSTBUFFER_SIZE];
        int res = getnameinfo(&pAddr->m_sockaddr, sizeof(struct sockaddr_in),
                              host, sizeof(host), nullptr, 0, NI_NAMEREQD);
        if (res != 0)
            return nullptr;

        sal_Char *cn = getFullQualifiedDomainName(host);
        SAL_WARN_IF( !cn, "sal.osl", "couldn't get full qualified domain name" );
        if (cn == nullptr)
            return nullptr;

        oslSocketAddr pSockAddr = createSocketAddr();
        SAL_WARN_IF( !pSockAddr, "sal.osl", "insufficient memory" );
        if (pSockAddr == nullptr)
        {
            free(cn);
            return nullptr;
        }

        memcpy(&pSockAddr->m_sockaddr, &pAddr->m_sockaddr, sizeof(pAddr->m_sockaddr));

        oslHostAddr pHostAddr = static_cast<oslHostAddr>(malloc(sizeof(struct oslHostAddrImpl)));
        SAL_WARN_IF( !pAddr, "sal.osl", "allocation error" );
        if (pHostAddr == nullptr)
        {
            destroySocketAddr(pSockAddr);
            free(cn);
            return nullptr;
        }

        pHostAddr->pHostName = cn;
        pHostAddr->pSockAddr = pSockAddr;

        return pHostAddr;
    }

    return nullptr;
}

oslHostAddr SAL_CALL osl_copyHostAddr (const oslHostAddr pAddr)
{
    SAL_WARN_IF( !pAddr, "sal.osl", "undefined address" );

    if (pAddr)
        return osl_psz_createHostAddr (pAddr->pHostName, pAddr->pSockAddr);
    return nullptr;
}

void SAL_CALL osl_getHostnameOfHostAddr (
    const oslHostAddr   Addr,
    rtl_uString       **ustrHostname)
{
    const sal_Char* pHostname=nullptr;

    pHostname = osl_psz_getHostnameOfHostAddr(Addr);

    rtl_uString_newFromAscii (ustrHostname, pHostname);
}

const sal_Char* osl_psz_getHostnameOfHostAddr (const oslHostAddr pAddr)
{
    if (pAddr)
        return pAddr->pHostName;
    return nullptr;
}

oslSocketAddr SAL_CALL osl_getSocketAddrOfHostAddr (const oslHostAddr pAddr)
{
    SAL_WARN_IF( !pAddr, "sal.osl", "undefined address" );

    if (pAddr)
        return pAddr->pSockAddr;
    return nullptr;
}

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

oslSocketResult SAL_CALL osl_getLocalHostname(rtl_uString **ustrLocalHostname)
{
    static auto const init = []() -> std::pair<oslSocketResult, OUString> {
            sal_Char LocalHostname[256] = "";

#ifdef SYSV
            struct utsname uts;

            if (uname(&uts) < 0)
                return {osl_Socket_Error, OUString()};

            if ((strlen(uts.nodename) + 1) > nBufLen)
                return {osl_Socket_Error, OUString()};

            strncpy(LocalHostname, uts.nodename, sizeof( LocalHostname ));
#else  /* BSD compatible */
            if (gethostname(LocalHostname, sizeof(LocalHostname)-1) != 0)
                return {osl_Socket_Error, OUString()};
#endif /* SYSV */
            LocalHostname[sizeof(LocalHostname)-1] = 0;

            /* check if we have an FQDN */
            if (strchr(LocalHostname, '.') == nullptr)
            {
                oslHostAddr Addr;

                /* no, determine it via dns */
                Addr = osl_psz_createHostAddrByName(LocalHostname);

                const sal_Char *pStr;
                if ((pStr = osl_psz_getHostnameOfHostAddr(Addr)) != nullptr)
                {
                    strncpy(LocalHostname, pStr, sizeof( LocalHostname ));
                    LocalHostname[sizeof(LocalHostname)-1] = 0;
                }
                osl_destroyHostAddr(Addr);
            }

            if (LocalHostname[0] != '\0')
            {
                return {osl_Socket_Ok, OUString::createFromAscii(LocalHostname)};
            }

            return {osl_Socket_Error, OUString()};
        }();

    rtl_uString_assign(ustrLocalHostname,init.second.pData);

    return init.first;
}

oslSocketAddr SAL_CALL osl_resolveHostname(rtl_uString *ustrHostname)
{
    oslSocketAddr Addr;
    rtl_String* strHostname=nullptr;
    sal_Char* pszHostName=nullptr;

    if ( ustrHostname != nullptr )
    {
        rtl_uString2String( &strHostname,
                            rtl_uString_getStr(ustrHostname),
                            rtl_uString_getLength(ustrHostname),
                            RTL_TEXTENCODING_UTF8,
                            OUSTRING_TO_OSTRING_CVTFLAGS );
        pszHostName = rtl_string_getStr(strHostname);
    }

    Addr = osl_psz_resolveHostname(pszHostName);

    if ( strHostname != nullptr )
    {
        rtl_string_release(strHostname);
    }

    return Addr;
}

oslSocketAddr osl_psz_resolveHostname(const sal_Char* pszHostname)
{
    struct oslHostAddrImpl *pAddr = osl_psz_createHostAddrByName(pszHostname);

    if (pAddr)
    {
        oslSocketAddr SockAddr = osl_copySocketAddr(pAddr->pSockAddr);

        osl_destroyHostAddr(pAddr);

        return SockAddr;
    }

    return nullptr;
}

sal_Int32 SAL_CALL osl_getServicePort(rtl_uString *ustrServicename, rtl_uString *ustrProtocol)
{
    sal_Int32 nPort;
    rtl_String* strServicename=nullptr;
    rtl_String* strProtocol=nullptr;
    sal_Char* pszServiceName=nullptr;
    sal_Char* pszProtocol=nullptr;

    if ( ustrServicename != nullptr )
    {
        rtl_uString2String( &strServicename,
                            rtl_uString_getStr(ustrServicename),
                            rtl_uString_getLength(ustrServicename),
                            RTL_TEXTENCODING_UTF8,
                            OUSTRING_TO_OSTRING_CVTFLAGS );
        pszServiceName = rtl_string_getStr(strServicename);
    }

    if ( ustrProtocol != nullptr )
    {
        rtl_uString2String( &strProtocol,
                            rtl_uString_getStr(ustrProtocol),
                            rtl_uString_getLength(ustrProtocol),
                            RTL_TEXTENCODING_UTF8,
                            OUSTRING_TO_OSTRING_CVTFLAGS );
        pszProtocol = rtl_string_getStr(strProtocol);
    }

    nPort = osl_psz_getServicePort(pszServiceName,pszProtocol);

    if ( strServicename != nullptr )
    {
        rtl_string_release(strServicename);
    }

    if ( strProtocol != nullptr )
    {
        rtl_string_release(strProtocol);
    }

    return nPort;
}

sal_Int32 osl_psz_getServicePort(const sal_Char* pszServicename,
                        const sal_Char* pszProtocol)
{
    struct servent* ps;

    ps= getservbyname(pszServicename, pszProtocol);

    if (ps != nullptr)
        return ntohs(ps->s_port);

    return OSL_INVALID_PORT;
}

void SAL_CALL osl_destroySocketAddr(oslSocketAddr pAddr)
{
    destroySocketAddr( pAddr );
}

oslAddrFamily SAL_CALL osl_getFamilyOfSocketAddr(oslSocketAddr pAddr)
{
    SAL_WARN_IF( !pAddr, "sal.osl", "undefined address" );

    if (pAddr)
        return FAMILY_FROM_NATIVE(pAddr->m_sockaddr.sa_family);
    return osl_Socket_FamilyInvalid;
}

sal_Int32 SAL_CALL osl_getInetPortOfSocketAddr(oslSocketAddr pAddr)
{
    SAL_WARN_IF( !pAddr, "sal.osl", "undefined address" );

    if( pAddr )
    {
        struct sockaddr_in* pSystemInetAddr= reinterpret_cast<sockaddr_in*>(&pAddr->m_sockaddr);

        if ( pSystemInetAddr->sin_family == FAMILY_TO_NATIVE(osl_Socket_FamilyInet))
            return ntohs(pSystemInetAddr->sin_port);
    }
    return OSL_INVALID_PORT;
}

sal_Bool SAL_CALL osl_setInetPortOfSocketAddr(oslSocketAddr pAddr, sal_Int32 Port)
{
    SAL_WARN_IF( !pAddr, "sal.osl", "undefined address" );

    if( pAddr )
    {
        struct sockaddr_in* pSystemInetAddr= reinterpret_cast<sockaddr_in*>(&pAddr->m_sockaddr);
        if ( pSystemInetAddr->sin_family == FAMILY_TO_NATIVE(osl_Socket_FamilyInet))
        {
            pSystemInetAddr->sin_port= htons(static_cast<short>(Port));
            return true;
        }
    }

    /* this is not a inet-addr => can't set port */
    return false;
}

oslSocketResult SAL_CALL osl_getHostnameOfSocketAddr(oslSocketAddr Addr, rtl_uString **ustrHostname)
{
    oslHostAddr pHostAddr= osl_createHostAddrByAddr(Addr);

    if (!pHostAddr)
    {
        return osl_Socket_Error;
    }

    rtl_uString_newFromAscii(ustrHostname,pHostAddr->pHostName);

    osl_destroyHostAddr(pHostAddr);

    return osl_Socket_Ok;
}

oslSocketResult SAL_CALL osl_getDottedInetAddrOfSocketAddr(oslSocketAddr Addr, rtl_uString **ustrDottedInetAddr)
{
    if( !Addr )
    {
        return osl_Socket_Error;
    }

    struct sockaddr_in* pSystemInetAddr = reinterpret_cast<sockaddr_in *>(&Addr->m_sockaddr);

    if (pSystemInetAddr->sin_family != FAMILY_TO_NATIVE(osl_Socket_FamilyInet))
    {
        return osl_Socket_Error;
    }

    rtl_uString_newFromAscii(ustrDottedInetAddr,inet_ntoa(pSystemInetAddr->sin_addr));

    return osl_Socket_Ok;

}

oslSocket SAL_CALL osl_createSocket(
    oslAddrFamily Family,
    oslSocketType Type,
    oslProtocol Protocol)
{
    oslSocket pSocket;

    /* alloc memory */
    pSocket= createSocketImpl(OSL_INVALID_SOCKET);

    /* create socket */
    pSocket->m_Socket= socket(FAMILY_TO_NATIVE(Family),
                                TYPE_TO_NATIVE(Type),
                                PROTOCOL_TO_NATIVE(Protocol));

    /* creation failed => free memory */
    if(pSocket->m_Socket == OSL_INVALID_SOCKET)
    {
        int nErrno = errno;
        SAL_WARN( "sal.osl", "socket creation failed: " << UnixErrnoString(nErrno) );

        destroySocketImpl(pSocket);
        pSocket= nullptr;
    }
    else
    {
        sal_Int32 nFlags=0;
        /* set close-on-exec flag */
        if ((nFlags = fcntl(pSocket->m_Socket, F_GETFD, 0)) != -1)
        {
            nFlags |= FD_CLOEXEC;
            if (fcntl(pSocket->m_Socket, F_SETFD, nFlags) == -1)
            {
                pSocket->m_nLastError=errno;
                int nErrno = errno;
                SAL_WARN( "sal.osl", "failed changing socket flags: " << UnixErrnoString(nErrno) );
            }
        }
        else
        {
            pSocket->m_nLastError=errno;
        }
    }

    return pSocket;
}

void SAL_CALL osl_acquireSocket(oslSocket pSocket)
{
    osl_atomic_increment(&(pSocket->m_nRefCount));
}

void SAL_CALL osl_releaseSocket(oslSocket pSocket)
{
    if (pSocket && osl_atomic_decrement(&(pSocket->m_nRefCount)) == 0)
    {
#if defined(CLOSESOCKET_DOESNT_WAKE_UP_ACCEPT)
        if (pSocket->m_bIsAccepting)
        {
            SAL_WARN( "sal.osl", "attempt to destroy socket while accepting" );
            return;
        }
#endif /* CLOSESOCKET_DOESNT_WAKE_UP_ACCEPT */
        osl_closeSocket(pSocket);
        destroySocketImpl(pSocket);
    }
}

void SAL_CALL osl_closeSocket(oslSocket pSocket)
{
    /* socket already invalid */
    if (!pSocket)
        return;

    pSocket->m_nLastError=0;
    sal_Int32 nFD = pSocket->m_Socket;

    if (nFD == OSL_INVALID_SOCKET)
        return;

    pSocket->m_Socket = OSL_INVALID_SOCKET;

    sal_Int32 nRet=0;
#if defined(CLOSESOCKET_DOESNT_WAKE_UP_ACCEPT)
    pSocket->m_bIsInShutdown = true;

    if (pSocket->m_bIsAccepting)
    {
        union {
            struct sockaddr aSockAddr;
            struct sockaddr_in aSockAddrIn;
        } s;
        socklen_t nSockLen = sizeof(s.aSockAddr);

        nRet = getsockname(nFD, &s.aSockAddr, &nSockLen);
        if (nRet < 0)
        {
            int nErrno = errno;
            SAL_WARN( "sal.osl", "getsockname call failed: " << UnixErrnoString(nErrno) );
        }

        if (s.aSockAddr.sa_family == AF_INET)
        {
            if (s.aSockAddrIn.sin_addr.s_addr == htonl(INADDR_ANY))
            {
                s.aSockAddrIn.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
            }

            int nConnFD = socket(AF_INET, SOCK_STREAM, 0);
            if (nConnFD < 0)
            {
                int nErrno = errno;
                SAL_WARN( "sal.osl", "socket call failed: " << UnixErrnoString(nErrno) );
            }
            else
            {
                nRet = connect(nConnFD, &s.aSockAddr, sizeof(s.aSockAddr));
                if (nRet < 0)
                {
                    int nErrno = errno;
                    SAL_WARN( "sal.osl", "connect call failed: " << UnixErrnoString(nErrno) );
                }
                close(nConnFD);
            }
        }
        pSocket->m_bIsAccepting = false;
    }
#endif /* CLOSESOCKET_DOESNT_WAKE_UP_ACCEPT */

    nRet=close(nFD);
    if (nRet != 0)
    {
        pSocket->m_nLastError=errno;
        int nErrno = errno;
        SAL_WARN( "sal.osl", "closeSocket close failed: " << UnixErrnoString(nErrno) );
    }

    pSocket->m_Socket = OSL_INVALID_SOCKET;
}

/* Note from function creator: I rely on the fact that oslSocketAddr and struct sockaddr
   are the same! I don't like it very much but see no other easy way to conceal
   the struct sockaddr from the eyes of the user. */
oslSocketAddr SAL_CALL osl_getLocalAddrOfSocket(oslSocket pSocket)
{
    socklen_t AddrLen;
    struct sockaddr Addr;
    oslSocketAddr  pAddr;

    if (pSocket == nullptr) /* ENOTSOCK */
        return nullptr;

    AddrLen= sizeof(struct sockaddr);

    if (getsockname(pSocket->m_Socket, &Addr, &AddrLen) == OSL_SOCKET_ERROR)
        return nullptr;

    pAddr = createSocketAddrFromSystem( &Addr );
    return pAddr;
}

oslSocketAddr SAL_CALL osl_getPeerAddrOfSocket(oslSocket pSocket)
{
    socklen_t AddrLen;
    struct sockaddr Addr;

    SAL_WARN_IF( !pSocket, "sal.osl", "undefined socket" );
    if ( pSocket == nullptr )
    {
        return nullptr;
    }

    pSocket->m_nLastError=0;
    AddrLen= sizeof(struct sockaddr);

    if(getpeername(pSocket->m_Socket, &Addr, &AddrLen) == OSL_SOCKET_ERROR)
    {
        pSocket->m_nLastError=errno;
        return nullptr;
    }
    return createSocketAddrFromSystem( &Addr );
}

sal_Bool SAL_CALL osl_bindAddrToSocket(oslSocket pSocket,
                             oslSocketAddr pAddr)
{
    int nRet;

    SAL_WARN_IF( !pSocket, "sal.osl", "undefined socket" );
    SAL_WARN_IF( !pAddr, "sal.osl", "undefined address" );
    if ( pSocket == nullptr || pAddr == nullptr )
    {
        return false;
    }

    pSocket->m_nLastError=0;

    nRet = bind(pSocket->m_Socket, &(pAddr->m_sockaddr), sizeof(struct sockaddr));

    if ( nRet == OSL_SOCKET_ERROR)
    {
        pSocket->m_nLastError=errno;
        return false;
    }

    return true;
}

sal_Bool SAL_CALL osl_listenOnSocket(oslSocket pSocket,
                           sal_Int32 MaxPendingConnections)
{
    int nRet;

    SAL_WARN_IF( !pSocket, "sal.osl", "undefined socket" );
    if ( pSocket == nullptr )
    {
        return false;
    }

    pSocket->m_nLastError=0;

    nRet = listen(pSocket->m_Socket,
                  MaxPendingConnections == -1 ?
                  SOMAXCONN :
                  MaxPendingConnections);
    if ( nRet == OSL_SOCKET_ERROR)
    {
        pSocket->m_nLastError=errno;
        return false;
    }

    return true;
}

oslSocketResult SAL_CALL osl_connectSocketTo(oslSocket pSocket,
                                    oslSocketAddr pAddr,
                                    const TimeValue* pTimeout)
{
    fd_set   WriteSet;
    fd_set   ExcptSet;
    int      ReadyHandles;
    struct timeval  tv;

    SAL_WARN_IF( !pSocket, "sal.osl", "undefined socket" );

    if ( pSocket == nullptr )
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

        if (errno == EWOULDBLOCK || errno == EINPROGRESS)
        {
            pSocket->m_nLastError=EINPROGRESS;
            return osl_Socket_InProgress;
        }

        pSocket->m_nLastError=errno;
        int nErrno = errno;
        SAL_WARN( "sal.osl", "connection failed: " << UnixErrnoString(nErrno) );
        return osl_Socket_Error;
    }

    /* set socket temporarily to non-blocking */
    if( !osl_enableNonBlockingMode(pSocket, true) )
        SAL_WARN( "sal.osl", "failed to enable non-blocking mode" );

    /* initiate connect */
    if(connect(pSocket->m_Socket,
               &(pAddr->m_sockaddr),
               sizeof(struct sockaddr)) != OSL_SOCKET_ERROR)
    {
       /* immediate connection */
        osl_enableNonBlockingMode(pSocket, false);

        return osl_Socket_Ok;
    }

    /* really an error or just delayed? */
    if (errno != EINPROGRESS)
    {
        pSocket->m_nLastError=errno;
        int nErrno = errno;
        SAL_WARN( "sal.osl", "connection failed: " << UnixErrnoString(nErrno) );

        osl_enableNonBlockingMode(pSocket, false);
        return osl_Socket_Error;
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
                         nullptr,
                         PTR_FD_SET(WriteSet),
                         PTR_FD_SET(ExcptSet),
                         pTimeout ? &tv : nullptr);

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
            {
                osl_enableNonBlockingMode(pSocket, false);
                return osl_Socket_Ok;
            }
            else
            {
                pSocket->m_nLastError = (nSockOpt == 0) ? nErrorCode : errno;
                return osl_Socket_Error;
            }
        }
        else
        {
            return osl_Socket_Error;
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
        pSocket->m_nLastError=errno;
        return osl_Socket_Error;
    }
    else    /* timeout */
    {
        pSocket->m_nLastError=errno;
        return osl_Socket_TimedOut;
    }
}

oslSocket SAL_CALL osl_acceptConnectionOnSocket(oslSocket pSocket,
                        oslSocketAddr* ppAddr)
{
    struct sockaddr Addr;
    int Connection, Flags;
    oslSocket pConnectionSockImpl;

    socklen_t AddrLen = sizeof(struct sockaddr);
    SAL_WARN_IF( !pSocket, "sal.osl", "undefined socket" );
    if ( pSocket == nullptr )
    {
        return nullptr;
    }

    pSocket->m_nLastError=0;
#if defined(CLOSESOCKET_DOESNT_WAKE_UP_ACCEPT)
    pSocket->m_bIsAccepting = true;
#endif /* CLOSESOCKET_DOESNT_WAKE_UP_ACCEPT */

    if( ppAddr && *ppAddr )
    {
        osl_destroySocketAddr( *ppAddr );
        *ppAddr = nullptr;
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
        int nErrno = errno;
        SAL_WARN( "sal.osl", "accept connection failed: " << UnixErrnoString(nErrno) );

#if defined(CLOSESOCKET_DOESNT_WAKE_UP_ACCEPT)
        pSocket->m_bIsAccepting = false;
#endif /* CLOSESOCKET_DOESNT_WAKE_UP_ACCEPT */
        return nullptr;
    }

    assert(AddrLen == sizeof(struct sockaddr));

#if defined(CLOSESOCKET_DOESNT_WAKE_UP_ACCEPT)
    if ( pSocket->m_bIsInShutdown )
    {
        close(Connection);
        SAL_WARN( "sal.osl", "close while accept" );
        return nullptr;
    }
#endif /* CLOSESOCKET_DOESNT_WAKE_UP_ACCEPT */

    if(ppAddr)
    {
        *ppAddr= createSocketAddrFromSystem(&Addr);
    }

    /* alloc memory */
    pConnectionSockImpl= createSocketImpl(OSL_INVALID_SOCKET);

    /* set close-on-exec flag */
    if ((Flags = fcntl(Connection, F_GETFD, 0)) != -1)
    {
        Flags |= FD_CLOEXEC;
        if (fcntl(Connection, F_SETFD, Flags) == -1)
        {
            pSocket->m_nLastError=errno;
            int nErrno = errno;
            SAL_WARN( "sal.osl", "fcntl failed: " << UnixErrnoString(nErrno) );
        }

    }

    pConnectionSockImpl->m_Socket           = Connection;
    pConnectionSockImpl->m_nLastError       = 0;
#if defined(CLOSESOCKET_DOESNT_WAKE_UP_ACCEPT)
    pConnectionSockImpl->m_bIsAccepting     = false;

    pSocket->m_bIsAccepting = false;
#endif /* CLOSESOCKET_DOESNT_WAKE_UP_ACCEPT */
    return pConnectionSockImpl;
}

sal_Int32 SAL_CALL osl_receiveSocket(oslSocket pSocket,
                          void* pBuffer,
                          sal_uInt32 BytesToRead,
                          oslSocketMsgFlag Flag)
{
    int nRead;

    SAL_WARN_IF( !pSocket, "sal.osl", "undefined socket" );
    if ( pSocket == nullptr )
    {
        return -1;
    }

    pSocket->m_nLastError=0;

    do
    {
        nRead =  recv(pSocket->m_Socket,
                      pBuffer,
                      BytesToRead,
                      MSG_FLAG_TO_NATIVE(Flag));
    } while ( nRead < 0 && errno == EINTR );

    if ( nRead < 0 )
    {
        pSocket->m_nLastError=errno;
        int nErrno = errno;
        SAL_WARN( "sal.osl", "receive socket [" << nRead << "] failed: " << UnixErrnoString(nErrno) );
    }
    else if ( nRead == 0 )
    {
        SAL_WARN( "sal.osl", "receive socket [" << nRead << "] failed: EOL" );
    }

    return nRead;
}

sal_Int32 SAL_CALL osl_receiveFromSocket(oslSocket pSocket,
                              oslSocketAddr pSenderAddr,
                              void* pBuffer,
                              sal_uInt32 BufferSize,
                              oslSocketMsgFlag Flag)
{
    int nRead;
    struct sockaddr *pSystemSockAddr = nullptr;
    socklen_t AddrLen = 0;
    if( pSenderAddr )
    {
        AddrLen = sizeof( struct sockaddr );
        pSystemSockAddr = &(pSenderAddr->m_sockaddr);
    }

    SAL_WARN_IF( !pSocket, "sal.osl", "undefined socket" );
    if ( pSocket == nullptr )
    {
        return -1;
    }

    pSocket->m_nLastError=0;

    nRead = recvfrom(pSocket->m_Socket,
                     pBuffer,
                     BufferSize,
                     MSG_FLAG_TO_NATIVE(Flag),
                     pSystemSockAddr,
                     &AddrLen);

    if ( nRead < 0 )
    {
        pSocket->m_nLastError=errno;
        int nErrno = errno;
        SAL_WARN( "sal.osl", "receive socket [" << nRead << "] failed: " << UnixErrnoString(nErrno) );
    }
    else if ( nRead == 0 )
    {
        SAL_WARN( "sal.osl", "receive socket [" << nRead << "] failed: EOL" );
    }

    return nRead;
}

sal_Int32 SAL_CALL osl_sendSocket(oslSocket pSocket,
                       const void* pBuffer,
                       sal_uInt32 BytesToSend,
                       oslSocketMsgFlag Flag)
{
    int nWritten;

    SAL_WARN_IF( !pSocket, "sal.osl", "undefined socket" );
    if ( pSocket == nullptr )
    {
        return -1;
    }

    pSocket->m_nLastError=0;

    do
    {
        nWritten = send(pSocket->m_Socket,
                        pBuffer,
                        BytesToSend,
                        MSG_FLAG_TO_NATIVE(Flag));
    } while ( nWritten < 0 && errno == EINTR );

    if ( nWritten < 0 )
    {
        pSocket->m_nLastError=errno;
        int nErrno = errno;
        SAL_WARN( "sal.osl", "send socket [" << nWritten << "] failed: " << UnixErrnoString(nErrno) );
    }
    else if ( nWritten == 0 )
    {
        SAL_WARN( "sal.osl", "send socket [" << nWritten << "] failed: EOL" );
    }

    return nWritten;
}

sal_Int32 SAL_CALL osl_sendToSocket(oslSocket pSocket,
                         oslSocketAddr ReceiverAddr,
                         const void* pBuffer,
                         sal_uInt32 BytesToSend,
                         oslSocketMsgFlag Flag)
{
    int nWritten;

    struct sockaddr *pSystemSockAddr = nullptr;
    int AddrLen = 0;
    if( ReceiverAddr )
    {
        pSystemSockAddr = &(ReceiverAddr->m_sockaddr);
        AddrLen = sizeof( struct sockaddr );
    }

    SAL_WARN_IF( !pSocket, "sal.osl", "undefined socket" );
    if ( pSocket == nullptr )
    {
        return -1;
    }

    pSocket->m_nLastError=0;

    /* ReceiverAddr might be 0 when used on a connected socket. */
    /* Then sendto should behave like send. */

    nWritten = sendto(pSocket->m_Socket,
                      pBuffer,
                      BytesToSend,
                      MSG_FLAG_TO_NATIVE(Flag),
                      pSystemSockAddr,
                      AddrLen);

    if ( nWritten < 0 )
    {
        pSocket->m_nLastError=errno;
        int nErrno = errno;
        SAL_WARN( "sal.osl", "send socket [" << nWritten << "] failed: " << UnixErrnoString(nErrno) );
    }
    else if ( nWritten == 0 )
    {
        SAL_WARN( "sal.osl", "send socket [" << nWritten << "] failed: EOL" );
    }

    return nWritten;
}

sal_Int32 SAL_CALL osl_readSocket (
    oslSocket pSocket, void *pBuffer, sal_Int32 n )
{
    sal_uInt8 * Ptr = static_cast<sal_uInt8 *>(pBuffer);
    sal_uInt32 BytesRead= 0;
    sal_uInt32 BytesToRead= n;

    SAL_WARN_IF( !pSocket, "sal.osl", "undefined socket" );

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

sal_Int32 SAL_CALL osl_writeSocket(
    oslSocket pSocket, const void *pBuffer, sal_Int32 n )
{
    /* loop until all desired bytes were send or an error occurred */
    sal_uInt32 BytesSend= 0;
    sal_uInt32 BytesToSend= n;
    sal_uInt8 const *Ptr = static_cast<sal_uInt8 const *>(pBuffer);

    SAL_WARN_IF( !pSocket, "sal.osl", "undefined socket" );

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

#ifdef HAVE_POLL_H /* poll() */

static bool socket_poll (
    oslSocket        pSocket,
    const TimeValue* pTimeout,
    short            nEvent)
{
    struct pollfd fds;
    int           timeout;
    int           result;

    SAL_WARN_IF( !pSocket, "sal.osl", "undefined socket" );
    if (pSocket == nullptr)
      return false; /* EINVAL */

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
        int nErrno = errno;
        SAL_WARN( "sal.osl", "poll failed: " << UnixErrnoString(nErrno) );
        return false;
    }
    if (result == 0)
    {
        /* Timeout */
        return false;
    }

    return ((fds.revents & nEvent) == nEvent);
}

#else  /* select() */

static sal_Bool socket_poll (
    oslSocket        pSocket,
    const TimeValue* pTimeout,
    short            nEvent)
{
    fd_set         fds;
    struct timeval tv;
    int            result;

    SAL_WARN_IF( !pSocket, "sal.osl", "undefined socket" );
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
        int nErrno = errno;
        SAL_WARN( "sal.osl", "select failed: " << UnixErrnoString(nErrno) );
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

sal_Bool SAL_CALL osl_isReceiveReady (
    oslSocket pSocket, const TimeValue* pTimeout)
{
    SAL_WARN_IF( !pSocket, "sal.osl", "undefined socket" );
    if (pSocket == nullptr)
    {
        /* ENOTSOCK */
        return false;
    }

    return socket_poll (pSocket, pTimeout, POLLIN);
}

sal_Bool SAL_CALL osl_isSendReady (
    oslSocket pSocket, const TimeValue* pTimeout)
{
    SAL_WARN_IF( !pSocket, "sal.osl", "undefined socket" );
    if (pSocket == nullptr)
    {
        /* ENOTSOCK */
        return false;
    }

    return socket_poll (pSocket, pTimeout, POLLOUT);
}

sal_Bool SAL_CALL osl_isExceptionPending (
    oslSocket pSocket, const TimeValue* pTimeout)
{
    SAL_WARN_IF( !pSocket, "sal.osl", "undefined socket" );
    if (pSocket == nullptr)
    {
        /* ENOTSOCK */
        return false;
    }

    return socket_poll (pSocket, pTimeout, POLLPRI);
}

sal_Bool SAL_CALL osl_shutdownSocket(oslSocket pSocket,
                           oslSocketDirection Direction)
{
    int nRet;

    SAL_WARN_IF( !pSocket, "sal.osl", "undefined socket" );
    if ( pSocket == nullptr )
    {
        return false;
    }

    pSocket->m_nLastError=0;

    nRet=shutdown(pSocket->m_Socket, DIRECTION_TO_NATIVE(Direction));
    if (nRet != 0 )
    {
        pSocket->m_nLastError=errno;
        int nErrno = errno;
        SAL_WARN( "sal.osl", "shutdown failed: " << UnixErrnoString(nErrno) );
    }
    return (nRet==0);
}

sal_Int32 SAL_CALL osl_getSocketOption(oslSocket pSocket,
                            oslSocketOptionLevel    Level,
                            oslSocketOption         Option,
                            void*                   pBuffer,
                            sal_uInt32                  BufferLen)
{
    socklen_t nOptLen = static_cast<socklen_t>(BufferLen);

    SAL_WARN_IF( !pSocket, "sal.osl", "undefined socket" );
    if ( pSocket == nullptr )
    {
        return -1;
    }

    pSocket->m_nLastError=0;

    if(getsockopt(pSocket->m_Socket,
                  OPTION_LEVEL_TO_NATIVE(Level),
                  OPTION_TO_NATIVE(Option),
                  pBuffer,
                  &nOptLen) == -1)
    {
        pSocket->m_nLastError=errno;
        return -1;
    }

    return nOptLen;
}

sal_Bool SAL_CALL osl_setSocketOption(oslSocket pSocket,
                            oslSocketOptionLevel    Level,
                            oslSocketOption         Option,
                            void*                   pBuffer,
                            sal_uInt32                  BufferLen)
{
    int nRet;

    SAL_WARN_IF( !pSocket, "sal.osl", "undefined socket" );
    if ( pSocket == nullptr )
    {
        return false;
    }

    pSocket->m_nLastError=0;

    nRet = setsockopt(pSocket->m_Socket,
                      OPTION_LEVEL_TO_NATIVE(Level),
                      OPTION_TO_NATIVE(Option),
                      pBuffer,
                      BufferLen);

    if ( nRet < 0 )
    {
        pSocket->m_nLastError=errno;
        return false;
    }

    return true;
}

sal_Bool SAL_CALL osl_enableNonBlockingMode(oslSocket pSocket,
                                  sal_Bool On)
{
    int flags;
    int nRet;

    SAL_WARN_IF( !pSocket, "sal.osl", "undefined socket" );
    if ( pSocket == nullptr )
    {
        return false;
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
        return false;
    }

    return true;
}

sal_Bool SAL_CALL osl_isNonBlockingMode(oslSocket pSocket)
{
    int flags;

    SAL_WARN_IF( !pSocket, "sal.osl", "undefined socket" );
    if ( pSocket == nullptr )
    {
        return false;
    }

    pSocket->m_nLastError=0;

    flags = fcntl(pSocket->m_Socket, F_GETFL, 0);

    if (flags == -1 || !(flags & O_NONBLOCK))
        return false;

    return true;
}

oslSocketType SAL_CALL osl_getSocketType(oslSocket pSocket)
{
    int Type=0;
    socklen_t TypeSize= sizeof(Type);

    SAL_WARN_IF( !pSocket, "sal.osl", "undefined socket" );
    if ( pSocket == nullptr )
    {
        return osl_Socket_TypeInvalid;
    }

    pSocket->m_nLastError=0;

    if(getsockopt(pSocket->m_Socket,
                  OPTION_LEVEL_TO_NATIVE(osl_Socket_LevelSocket),
                  OPTION_TO_NATIVE(osl_Socket_OptionType),
                  &Type,
                  &TypeSize) == -1)
    {
        /* error */
        pSocket->m_nLastError=errno;
        return osl_Socket_TypeInvalid;
    }

    return TYPE_FROM_NATIVE(Type);

}

void SAL_CALL osl_getLastSocketErrorDescription(oslSocket Socket, rtl_uString **ustrError)
{
    sal_Char pszError[1024];

    pszError[0] = '\0';

    osl_psz_getLastSocketErrorDescription(Socket,pszError,sizeof(pszError));

    rtl_uString_newFromAscii(ustrError,pszError);
}

void osl_psz_getLastSocketErrorDescription(oslSocket pSocket, sal_Char* pBuffer, sal_uInt32 BufferSize)
{
    /* make sure pBuffer will be a zero-terminated string even when strncpy has to cut */
    pBuffer[BufferSize-1]= '\0';

    if ( pSocket == nullptr )
    {
        strncpy(pBuffer, strerror(EINVAL), BufferSize-1);
        return;
    }

    strncpy(pBuffer, strerror(pSocket->m_nLastError), BufferSize-1);
}

oslSocketError SAL_CALL osl_getLastSocketError(oslSocket pSocket)
{
    if ( pSocket == nullptr )
    {
        return ERROR_FROM_NATIVE(EINVAL);
    }

    return ERROR_FROM_NATIVE(pSocket->m_nLastError);
}

struct oslSocketSetImpl
{
    int     m_MaxHandle;    /* for select(), the largest descriptor in the set */
    fd_set  m_Set;          /* the set of descriptors */

};

oslSocketSet SAL_CALL osl_createSocketSet()
{
    oslSocketSetImpl* pSet;

    pSet= static_cast<oslSocketSetImpl*>(malloc(sizeof(oslSocketSetImpl)));

    SAL_WARN_IF( !pSet, "sal.osl", "allocation error" );

    if(pSet)
    {
        pSet->m_MaxHandle= 0;
        FD_ZERO(&pSet->m_Set);
    }

    return pSet;
}

void SAL_CALL osl_destroySocketSet(oslSocketSet Set)
{
    if(Set)
        free(Set);
}

void SAL_CALL osl_clearSocketSet(oslSocketSet Set)
{
    SAL_WARN_IF( !Set, "sal.osl", "undefined socket set" );
    if ( Set == nullptr )
    {
        return;
    }

    Set->m_MaxHandle= 0;

    FD_ZERO(&Set->m_Set);
}

void SAL_CALL osl_addToSocketSet(oslSocketSet Set, oslSocket pSocket)
{
    SAL_WARN_IF( !Set, "sal.osl", "undefined socket set" );
    SAL_WARN_IF( !pSocket, "sal.osl", "undefined socket" );

    if ( Set == nullptr || pSocket == nullptr)
    {
        return;
    }

    /* correct max handle */
    if(pSocket->m_Socket > Set->m_MaxHandle)
        Set->m_MaxHandle= pSocket->m_Socket;
    FD_SET(pSocket->m_Socket, &Set->m_Set);

}

void SAL_CALL osl_removeFromSocketSet(oslSocketSet Set, oslSocket pSocket)
{
    SAL_WARN_IF( !Set, "sal.osl", "undefined socket set" );
    SAL_WARN_IF( !pSocket, "sal.osl", "undefined socket" );

    if ( Set == nullptr || pSocket == nullptr)
    {
        return;
    }

    /* correct max handle */
    if(pSocket->m_Socket == Set->m_MaxHandle)
    {
        /* not optimal, since the next used descriptor might be */
        /* much smaller than m_Socket-1, but it will do */
        Set->m_MaxHandle--;
        if(Set->m_MaxHandle < 0)
        {
            Set->m_MaxHandle= 0;   /* avoid underflow */
        }
    }

    FD_CLR(pSocket->m_Socket, &Set->m_Set);
}

sal_Bool SAL_CALL osl_isInSocketSet(oslSocketSet Set, oslSocket pSocket)
{
    SAL_WARN_IF( !Set, "sal.osl", "undefined socket set" );
    SAL_WARN_IF( !pSocket, "sal.osl", "undefined socket" );
    if ( Set == nullptr || pSocket == nullptr )
    {
        return false;
    }

    return bool(FD_ISSET(pSocket->m_Socket, &Set->m_Set));
}

sal_Int32 SAL_CALL osl_demultiplexSocketEvents(oslSocketSet IncomingSet,
                                    oslSocketSet OutgoingSet,
                                    oslSocketSet OutOfBandSet,
                                    const TimeValue* pTimeout)
{
    int MaxHandle= 0;
    struct timeval  tv;

    if (pTimeout)
    {
        /* non-blocking call */
        tv.tv_sec  = pTimeout->Seconds;
        tv.tv_usec = pTimeout->Nanosec / 1000L;
    }

    /* get max handle from all sets */
    if (IncomingSet)
        MaxHandle= IncomingSet->m_MaxHandle;

    if (OutgoingSet && (OutgoingSet->m_MaxHandle > MaxHandle))
        MaxHandle= OutgoingSet->m_MaxHandle;

    if (OutOfBandSet && (OutOfBandSet->m_MaxHandle > MaxHandle))
        MaxHandle= OutOfBandSet->m_MaxHandle;

    return select(MaxHandle+1,
                  IncomingSet  ? PTR_FD_SET(IncomingSet->m_Set)  : nullptr,
                  OutgoingSet ? PTR_FD_SET(OutgoingSet->m_Set) : nullptr,
                  OutOfBandSet ? PTR_FD_SET(OutOfBandSet->m_Set) : nullptr,
                  pTimeout ? &tv : nullptr);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
