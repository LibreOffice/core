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
#include <rtl/alloc.h>

#include "sockimpl.h"

extern "C" {

/*
    oslSocketAddr is a pointer to a Berkeley struct sockaddr.
    I refrained from using sockaddr_in because of possible further
    extensions of this socket-interface (IP-NG?).
    The intention was to hide all Berkeley data-structures from
    direct access past the osl-interface.

    The current implementation is internet (IP) centered. All
    the constructor-functions (osl_create...) take parameters
    that will probably make sense only in the IP-environment
    (e.g. because of using the dotted-Addr-format).

    If the interface will be extended to host other protocol-
    families, I expect no externally visible changes in the
    existing functions. You'll probably need only new
    constructor-functions who take the different Addr
    formats into consideration (maybe a long dotted Addr
    or whatever).
*/

/*
  _Note_ that I rely on the fact that oslSocketAddr and struct sockaddr
  are the same! I don't like it very much but see no other easy way to
  conceal the struct sockaddr from the eyes of the user.
*/

#define OSL_INVALID_SOCKET      INVALID_SOCKET          /* WIN32 */
#define OSL_SOCKET_ERROR        SOCKET_ERROR            /* WIN32 */

/*****************************************************************************/
/* enum oslAddrFamily */
/*****************************************************************************/

/* map */
static DWORD FamilyMap[]= {
    AF_INET,                    /* osl_Socket_FamilyInet */
    AF_IPX,                     /* osl_Socket_FamilyIpx */
    0                           /* osl_Socket_FamilyInvalid */
};

/* reverse map */
static oslAddrFamily osl_AddrFamilyFromNative(DWORD nativeType)
{
    oslAddrFamily i= (oslAddrFamily) 0;
    while(i != osl_Socket_FamilyInvalid)
    {
        if(FamilyMap[i] == nativeType)
            return i;
        i = (oslAddrFamily) ( (int)i + 1);
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
static DWORD ProtocolMap[]= {
    0,                          /* osl_Socket_FamilyInet */
    NSPROTO_IPX,                /* osl_Socket_FamilyIpx */
    NSPROTO_SPX,                /* osl_Socket_ProtocolSpx */
    NSPROTO_SPXII,              /* osl_Socket_ProtocolSpx_ii */
    0                           /* osl_Socket_ProtocolInvalid */
};

/* macros */
#define PROTOCOL_FROM_NATIVE(y) osl_ProtocolFromNative(y)
#define PROTOCOL_TO_NATIVE(x)   ProtocolMap[x]

/*****************************************************************************/
/* enum oslSocketType */
/*****************************************************************************/

/* map */
static DWORD TypeMap[]= {
    SOCK_STREAM,                /* osl_Socket_TypeStream */
    SOCK_DGRAM,                 /* osl_Socket_TypeDgram  */
    SOCK_RAW,                   /* osl_Socket_TypeRaw */
    SOCK_RDM,                   /* osl_Socket_TypeRdm */
    SOCK_SEQPACKET,             /* osl_Socket_TypeSeqPacket */
    0                           /* osl_Socket_TypeInvalid */
};

/* reverse map */
static oslSocketType osl_SocketTypeFromNative(DWORD nativeType)
{
    oslSocketType i= (oslSocketType)0;
    while(i != osl_Socket_TypeInvalid)
    {
        if(TypeMap[i] == nativeType)
            return i;
        i = (oslSocketType)((int)i+1);
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
static DWORD OptionMap[]= {
    SO_DEBUG,                   /* osl_Socket_OptionDebug */
    SO_ACCEPTCONN,              /* osl_Socket_OptionAcceptConn */
    SO_REUSEADDR,               /* osl_Socket_OptionReuseAddr */
    SO_KEEPALIVE,               /* osl_Socket_OptionKeepAlive */
    SO_DONTROUTE,               /* osl_Socket_OptionDontRoute */
    SO_BROADCAST,               /* osl_Socket_OptionBroadcast */
    SO_USELOOPBACK,             /* osl_Socket_OptionUseLoopback */
    SO_LINGER,                  /* osl_Socket_OptionLinger */
    SO_OOBINLINE,               /* osl_Socket_OptionOOBinLine */
    SO_SNDBUF,                  /* osl_Socket_OptionSndBuf */
    SO_RCVBUF,                  /* osl_Socket_OptionRcvBuf */
    SO_SNDLOWAT,                /* osl_Socket_OptionSndLowat */
    SO_RCVLOWAT,                /* osl_Socket_OptionRcvLowat */
    SO_SNDTIMEO,                /* osl_Socket_OptionSndTimeo */
    SO_RCVTIMEO,                /* osl_Socket_OptionRcvTimeo */
    SO_ERROR,                   /* osl_Socket_OptionError */
    SO_TYPE,                    /* osl_Socket_OptionType */
    TCP_NODELAY,                /* osl_Socket_OptionTcpNoDelay */
    0                           /* osl_Socket_OptionInvalid */
};

/* macros */
#define OPTION_TO_NATIVE(x)     OptionMap[x]
#define OPTION_FROM_NATIVE(y)   osl_SocketOptionFromNative(y)

/*****************************************************************************/
/* enum oslSocketOptionLevel */
/*****************************************************************************/

static DWORD OptionLevelMap[]= {
    SOL_SOCKET,                 /* osl_Socket_LevelSocket */
    IPPROTO_TCP,                /* osl_Socket_LevelTcp */
    0                           /* osl_invalid_SocketLevel */
};

/* macros */
#define OPTION_LEVEL_TO_NATIVE(x)       OptionLevelMap[x]
#define OPTION_LEVEL_FROM_NATIVE(y)     osl_SocketOptionLevelFromNative(y)

/*****************************************************************************/
/* enum oslSocketMsgFlag */
/*****************************************************************************/

static DWORD SocketMsgFlagMap[]= {
    0,                          /* osl_Socket_MsgNormal */
    MSG_OOB,                    /* osl_Socket_MsgOOB */
    MSG_PEEK,                   /* osl_Socket_MsgPeek */
    MSG_DONTROUTE,              /* osl_Socket_MsgDontRoute */
    MSG_MAXIOVLEN               /* osl_Socket_MsgMaxIOVLen */
};

/* macros */
#define MSG_FLAG_TO_NATIVE(x)       SocketMsgFlagMap[x]
#define MSG_FLAG_FROM_NATIVE(y)     osl_SocketMsgFlagFromNative(y)

/*****************************************************************************/
/* enum oslSocketDirection */
/*****************************************************************************/

static DWORD SocketDirection[]= {
    SD_RECEIVE,                 /* osl_Socket_DirRead */
    SD_SEND,                    /* osl_Socket_DirWrite */
    SD_BOTH                     /* osl_Socket_DirReadwrite */
};

/* macros */
#define DIRECTION_TO_NATIVE(x)      SocketDirection[x]
#define DIRECTION_FROM_NATIVE(y)    osl_SocketDirectionFromNative(y)

/*****************************************************************************/
/* enum oslSocketError */
/*****************************************************************************/

static int SocketError[]= {

    0,                  /* no error */
    WSAENOTSOCK,            /* Socket operation on non-socket */
    WSAEDESTADDRREQ,        /* Destination address required */
    WSAEMSGSIZE,            /* Message too long */
    WSAEPROTOTYPE,          /* Protocol wrong type for socket */
    WSAENOPROTOOPT,     /* Protocol not available */
    WSAEPROTONOSUPPORT, /* Protocol not supported */
    WSAESOCKTNOSUPPORT, /* Socket type not supported */
    WSAEOPNOTSUPP,          /* Operation not supported on socket */
    WSAEPFNOSUPPORT,        /* Protocol family not supported */
    WSAEAFNOSUPPORT,        /* Address family not supported by */
                            /* protocol family */
    WSAEADDRINUSE,          /* Address already in use */
    WSAEADDRNOTAVAIL,       /* Can't assign requested address */
    WSAENETDOWN,            /* Network is down */
    WSAENETUNREACH,     /* Network is unreachable */
    WSAENETRESET,           /* Network dropped connection because */
                            /* of reset */
    WSAECONNABORTED,        /* Software caused connection abort */
    WSAECONNRESET,          /* Connection reset by peer */
    WSAENOBUFS,         /* No buffer space available */
    WSAEISCONN,         /* Socket is already connected */
    WSAENOTCONN,            /* Socket is not connected */
    WSAESHUTDOWN,           /* Can't send after socket shutdown */
    WSAETOOMANYREFS,        /* Too many references: can't splice */
    WSAETIMEDOUT,           /* Connection timed out */
    WSAECONNREFUSED,        /* Connection refused */
    WSAEHOSTDOWN,           /* Host is down */
    WSAEHOSTUNREACH,        /* No route to host */
    WSAEWOULDBLOCK,     /* call would block on non-blocking socket */
    WSAEALREADY,            /* operation already in progress */
    WSAEINPROGRESS      /* operation now in progress */
};

/* reverse map */
static oslSocketError osl_SocketErrorFromNative(int nativeType)
{
    oslSocketError i= (oslSocketError)0;

    while(i != osl_Socket_E_InvalidError)
    {
        if(SocketError[i] == nativeType)
            return i;

        i = (oslSocketError)( (int) i + 1);
    }
    return i;
}

/* macros */
#define ERROR_TO_NATIVE(x)      SocketError[x]
#define ERROR_FROM_NATIVE(y)    osl_SocketErrorFromNative(y)

/*****************************************************************************/
/* oslSocketDialupImpl */
/*****************************************************************************/
static oslSocketDialupImpl *pDialupImpl = NULL;

/*
 * __osl_createSocketDialupImpl.
 */
static oslSocketDialupImpl* __osl_createSocketDialupImpl (void)
{
    oslSocketDialupImpl *pImpl;
    pImpl = (oslSocketDialupImpl*)rtl_allocateZeroMemory( sizeof (oslSocketDialupImpl));

    InitializeCriticalSection (&pImpl->m_hMutex);

    return (pImpl);
}

/*
 * __osl_initSocketDialupImpl.
 */
static void __osl_initSocketDialupImpl (oslSocketDialupImpl *pImpl)
{
#ifdef SOCKET_USE_AUTODIAL
    if (pImpl)
    {
        HINSTANCE hModule;

        EnterCriticalSection (&pImpl->m_hMutex);

        hModule = LoadLibrary (INTERNET_MODULE_NAME);
        if (!(hModule <= (HINSTANCE)HINSTANCE_ERROR))
        {
            pImpl->m_pfnAttemptConnect = (INTERNETATTEMPTCONNECT)
                (GetProcAddress (hModule, "InternetAttemptConnect"));
            pImpl->m_pfnAutodial = (INTERNETAUTODIAL)
                (GetProcAddress (hModule, "InternetAutodial"));
            pImpl->m_pfnAutodialHangup = (INTERNETAUTODIALHANGUP)
                (GetProcAddress (hModule, "InternetAutodialHangup"));
            pImpl->m_pfnGetConnectedState = (INTERNETGETCONNECTEDSTATE)
                (GetProcAddress (hModule, "InternetGetConnectedState"));
            pImpl->m_hModule = hModule;
        }

        LeaveCriticalSection (&pImpl->m_hMutex);
    }
#else
    (void)pImpl;
#endif
}

/*
 * __osl_destroySocketDialupImpl.
 */
static void __osl_destroySocketDialupImpl (oslSocketDialupImpl *pImpl)
{
    if (pImpl)
    {
        EnterCriticalSection (&pImpl->m_hMutex);

        if (pImpl->m_dwFlags & INTERNET_CONNECTION_HANGUP)
        {
            if (pImpl->m_pfnAutodialHangup)
            {
                (pImpl->m_pfnAutodialHangup)(0);
                pImpl->m_dwFlags &= ~INTERNET_CONNECTION_HANGUP;
            }
        }

        if (pImpl->m_hModule)
            FreeLibrary (pImpl->m_hModule);

        LeaveCriticalSection (&pImpl->m_hMutex);
        DeleteCriticalSection (&pImpl->m_hMutex);

        rtl_freeMemory (pImpl);
    }
}

/*
 * __osl_querySocketDialupImpl.
 */
static sal_Bool __osl_querySocketDialupImpl (void)
{
    sal_Bool result;

    if (pDialupImpl == NULL)
    {
        pDialupImpl = __osl_createSocketDialupImpl();
        __osl_initSocketDialupImpl (pDialupImpl);
    }

    EnterCriticalSection (&pDialupImpl->m_hMutex);

    result = sal_True;
    if (pDialupImpl->m_pfnGetConnectedState)
    {
        DWORD dwFlags = 0;

        result = (sal_Bool)(pDialupImpl->m_pfnGetConnectedState)(&dwFlags, 0);
        pDialupImpl->m_dwFlags |= dwFlags;
    }

    LeaveCriticalSection (&pDialupImpl->m_hMutex);
    return (result);
}

/*
 * __osl_attemptSocketDialupImpl.
 */
static sal_Bool __osl_attemptSocketDialupImpl (void)
{
    sal_Bool result;

    if (pDialupImpl == NULL)
    {
        pDialupImpl = __osl_createSocketDialupImpl();
        __osl_initSocketDialupImpl (pDialupImpl);
    }

    EnterCriticalSection (&pDialupImpl->m_hMutex);

    result = __osl_querySocketDialupImpl();
    if (!result)
    {
        result = sal_True;
        if (pDialupImpl->m_pfnAutodial)
        {
            result = (sal_Bool)(pDialupImpl->m_pfnAutodial)(0, 0);
            if (result)
                pDialupImpl->m_dwFlags |= INTERNET_CONNECTION_HANGUP;
            else
                WSASetLastError (WSAENETDOWN);
        }
    }

    LeaveCriticalSection (&pDialupImpl->m_hMutex);
    return (result);
}

/*****************************************************************************/
/* oslSocketImpl */
/*****************************************************************************/
static sal_uInt32 g_nSocketImpl = 0;

#if OSL_DEBUG_LEVEL > 1
static sal_uInt32 g_nSocketAddr = 0;
struct LeakWarning
{
    ~LeakWarning()
    {
        if( g_nSocketImpl )
            OSL_TRACE( "sal_socket: %d socket instances leak" , g_nSocketImpl );
        if( g_nSocketAddr )
            OSL_TRACE( "sal_socket: %d socket address instances leak" , g_nSocketAddr );
    }
};
LeakWarning socketWarning;
#endif

/*
 * __osl_createSocketImpl.
 */
oslSocket __osl_createSocketImpl(SOCKET Socket)
{
    oslSocket pSockImpl = (oslSocket) rtl_allocateZeroMemory( sizeof(struct oslSocketImpl));
    pSockImpl->m_Socket = Socket;
    pSockImpl->m_nRefCount = 1;

    g_nSocketImpl++;

    return (pSockImpl);
}

/*
 * __osl_destroySocketImpl.
 */
void __osl_destroySocketImpl(oslSocketImpl *pImpl)
{
    if (pImpl)
    {
        if (--g_nSocketImpl == 0)
        {
            __osl_destroySocketDialupImpl (pDialupImpl);
            pDialupImpl = NULL;
        }
        rtl_freeMemory (pImpl);
    }
}
/*****************************************************************************/
static oslSocketAddr __osl_createSocketAddr(  )
{
    oslSocketAddr pAddr = (oslSocketAddr) rtl_allocateZeroMemory( sizeof( struct oslSocketAddrImpl ));
    pAddr->m_nRefCount = 1;
#if OSL_DEBUG_LEVEL > 1
    g_nSocketAddr ++;
#endif
    return pAddr;
}

static oslSocketAddr __osl_createSocketAddrWithFamily(
    oslAddrFamily family, sal_Int32 port, sal_uInt32 nAddr )
{
    OSL_ASSERT( family == osl_Socket_FamilyInet );

    oslSocketAddr pAddr = __osl_createSocketAddr();
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
    memcpy( &(pAddr->m_sockaddr), pSystemSockAddr, sizeof( sockaddr ) );
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
// @deprecated, to be removed
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
sal_Bool SAL_CALL osl_isEqualSocketAddr(oslSocketAddr Addr1, oslSocketAddr Addr2)
{
    OSL_ASSERT(Addr1);
    OSL_ASSERT(Addr2);
    struct sockaddr* pAddr1= &(Addr1->m_sockaddr);
    struct sockaddr* pAddr2= &(Addr2->m_sockaddr);

    OSL_ASSERT(pAddr1);
    OSL_ASSERT(pAddr2);

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

    oslSocketAddr pAddr =
        __osl_createSocketAddrWithFamily( osl_Socket_FamilyInet, htons( (sal_uInt16) Port), nAddr );
    return pAddr;
}

/*****************************************************************************/
/* osl_createInetSocketAddr */
/*****************************************************************************/
oslSocketAddr SAL_CALL osl_createInetSocketAddr (
    rtl_uString *strDottedAddr,
    sal_Int32    Port)
{
    sal_uInt32 Addr;
    rtl_String  *pDottedAddr=NULL;

    rtl_uString2String(
        &pDottedAddr, strDottedAddr->buffer, strDottedAddr->length,
        RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);

    Addr= inet_addr (pDottedAddr->buffer);
    rtl_string_release (pDottedAddr);

    oslSocketAddr pAddr = 0;
    if(Addr != OSL_INADDR_NONE)
    {
        pAddr = __osl_createSocketAddrWithFamily( osl_Socket_FamilyInet, htons( (sal_uInt16)Port), Addr );
    }
    return pAddr;
}

oslSocketResult SAL_CALL osl_setAddrOfSocketAddr( oslSocketAddr pAddr, sal_Sequence *pByteSeq )
{
    OSL_ASSERT( pAddr );
    OSL_ASSERT( pByteSeq );

    oslSocketResult res = osl_Socket_Error;
    if( pAddr && pByteSeq )
    {
        OSL_ASSERT( pAddr->m_sockaddr.sa_family == FAMILY_TO_NATIVE( osl_Socket_FamilyInet ) );
        OSL_ASSERT( pByteSeq->nElements == 4 );
        struct sockaddr_in * pSystemInetAddr = (struct sockaddr_in * ) &(pAddr->m_sockaddr);
        memcpy( &(pSystemInetAddr->sin_addr) , pByteSeq->elements , 4 );
        res = osl_Socket_Ok;
    }
    return res;
}

/** Returns the addr field in the struct sockaddr. ppByteSeq is in network byteorder. *ppByteSeq may
    either be 0 or contain a constructed sal_Sequence.
 */
oslSocketResult SAL_CALL osl_getAddrOfSocketAddr( oslSocketAddr pAddr, sal_Sequence **ppByteSeq )
{
    OSL_ASSERT( pAddr );
    OSL_ASSERT( ppByteSeq );

    oslSocketResult res = osl_Socket_Error;
    if( pAddr && ppByteSeq )
    {
        struct sockaddr_in * pSystemInetAddr = (struct sockaddr_in * ) &(pAddr->m_sockaddr);
        rtl_byte_sequence_constructFromArray( ppByteSeq , (sal_Int8 *) &(pSystemInetAddr->sin_addr),4);
        res = osl_Socket_Ok;
    }
    return res;
}

/*****************************************************************************/
/* oslHostAddr */
/*****************************************************************************/
struct oslHostAddrImpl {
    rtl_uString     *pHostName;
    oslSocketAddr   pSockAddr;
} ;

static oslHostAddr __osl_hostentToHostAddr (const struct hostent *he)
{
    oslHostAddr pAddr= NULL;
    oslSocketAddr pSocketAddr = 0;

    rtl_uString     *cn= NULL;

    if ((he == NULL) || (he->h_name == NULL) || (he->h_addr_list[0] == NULL))
        return ((oslHostAddr)NULL);

    rtl_string2UString(
        &cn, he->h_name, strlen(he->h_name),
        RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);
    OSL_ASSERT(cn != 0);

    pSocketAddr = __osl_createSocketAddr();

    if (pSocketAddr == NULL)
    {
        rtl_uString_release(cn);
        return ((oslHostAddr)NULL);
    }

    pSocketAddr->m_sockaddr.sa_family = he->h_addrtype;
    if (pSocketAddr->m_sockaddr.sa_family == FAMILY_TO_NATIVE(osl_Socket_FamilyInet))
    {
        struct sockaddr_in *sin= (struct sockaddr_in *)&(pSocketAddr->m_sockaddr);
        memcpy (
            &(sin->sin_addr.s_addr),
            he->h_addr_list[0],
            he->h_length);
    }
    else
    {
        /* unknown address family */
        /* future extensions for new families might be implemented here */

        OSL_TRACE("_osl_hostentToHostAddr(): unknown address family.");
        OSL_ASSERT(sal_False);

        __osl_destroySocketAddr( pSocketAddr );
        rtl_uString_release(cn);
        return ((oslHostAddr)NULL);
    }

    pAddr= (oslHostAddr )rtl_allocateMemory (sizeof (struct oslHostAddrImpl));

    if (pAddr == NULL)
    {
        __osl_destroySocketAddr( pSocketAddr );
        rtl_uString_release(cn);
        return ((oslHostAddr)NULL);
    }

    pAddr->pHostName= cn;
    pAddr->pSockAddr= pSocketAddr;

    return pAddr;
}

/*****************************************************************************/
/* osl_createHostAddr */
/*****************************************************************************/
oslHostAddr SAL_CALL osl_createHostAddr (
    rtl_uString         *strHostname,
    const oslSocketAddr  pSocketAddr)
{
    oslHostAddr pAddr;
    rtl_uString     *cn= NULL;

    if ((strHostname == NULL)  || (strHostname->length == 0) || (pSocketAddr == NULL))
        return ((oslHostAddr)NULL);

    rtl_uString_newFromString( &cn, strHostname);

    if ( ! pSocketAddr )
    {
        rtl_uString_release(cn);
        return ((oslHostAddr)NULL);
    }

    pAddr= (oslHostAddr)rtl_allocateMemory (sizeof (struct oslHostAddrImpl));

    if (pAddr == NULL)
    {
        rtl_uString_release(cn);
        return ((oslHostAddr)NULL);
    }

    pAddr->pHostName= cn;
    pAddr->pSockAddr= osl_copySocketAddr( pSocketAddr );

    return ((oslHostAddr)pAddr);
}

/*****************************************************************************/
/* osl_createHostAddrByName */
/*****************************************************************************/
oslHostAddr SAL_CALL osl_createHostAddrByName(rtl_uString *strHostname)
{
    if ((strHostname == NULL) || (strHostname->length == 0))
        return ((oslHostAddr)NULL);

    if (__osl_attemptSocketDialupImpl())
    {
        struct hostent *he;
        rtl_String     *Hostname= NULL;

        rtl_uString2String(
            &Hostname, strHostname->buffer, strHostname->length,
            RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);

        he= gethostbyname (Hostname->buffer);

        rtl_string_release (Hostname);
        return __osl_hostentToHostAddr (he);
    }
    return ((oslHostAddr)NULL);
}

/*****************************************************************************/
/* osl_createHostAddrByAddr */
/*****************************************************************************/
oslHostAddr SAL_CALL osl_createHostAddrByAddr(const oslSocketAddr pAddr)
{
    if (pAddr == NULL)
        return ((oslHostAddr)NULL);

    if (pAddr->m_sockaddr.sa_family == FAMILY_TO_NATIVE(osl_Socket_FamilyInet))
    {
        const struct sockaddr_in *sin= (const struct sockaddr_in *)&(pAddr->m_sockaddr);

        if (sin->sin_addr.s_addr == htonl(INADDR_ANY))
            return ((oslHostAddr)NULL);

        if (__osl_attemptSocketDialupImpl())
        {
            struct hostent *he;
            he= gethostbyaddr ((const sal_Char *)&(sin->sin_addr),
                               sizeof (sin->sin_addr),
                               sin->sin_family);
            return __osl_hostentToHostAddr (he);
        }
    }

    return ((oslHostAddr)NULL);
}

/*****************************************************************************/
/* osl_copyHostAddr */
/*****************************************************************************/
oslHostAddr SAL_CALL osl_copyHostAddr(const oslHostAddr Addr)
{
    oslHostAddr pAddr= (oslHostAddr)Addr;

    if (pAddr)
        return osl_createHostAddr (pAddr->pHostName, pAddr->pSockAddr);
    else
        return ((oslHostAddr)NULL);
}

/*****************************************************************************/
/* osl_getHostnameOfHostAddr */
/*****************************************************************************/
void SAL_CALL osl_getHostnameOfHostAddr(
    const oslHostAddr pAddr, rtl_uString **strHostname)
{
    if (pAddr)
        rtl_uString_assign (strHostname, pAddr->pHostName);
    else
        rtl_uString_new (strHostname);
}

/*****************************************************************************/
/* osl_getSocketAddrOfHostAddr */
/*****************************************************************************/
oslSocketAddr SAL_CALL osl_getSocketAddrOfHostAddr(const oslHostAddr pAddr)
{
    if (pAddr)
        return (const oslSocketAddr)(pAddr->pSockAddr);
    else
        return NULL;
}

/*****************************************************************************/
/* osl_destroyHostAddr */
/*****************************************************************************/
void SAL_CALL osl_destroyHostAddr(oslHostAddr pAddr)
{
    if (pAddr)
    {
        if (pAddr->pHostName)
            rtl_uString_release (pAddr->pHostName);
        if (pAddr->pSockAddr)
            osl_destroySocketAddr( pAddr->pSockAddr );

        rtl_freeMemory (pAddr);
    }
}

/*****************************************************************************/
/* osl_getLocalHostname */
/*****************************************************************************/
oslSocketResult SAL_CALL osl_getLocalHostname (rtl_uString **strLocalHostname)
{
    static sal_Unicode LocalHostname[256] = {0};

    if (rtl_ustr_getLength(LocalHostname) == 0)
    {
        sal_Char Host[256]= "";
        if (gethostname(Host, sizeof(Host)) == 0)
        {
            /* check if we have an FQDN */
            if (strchr(Host, '.') == NULL)
            {
                oslHostAddr pAddr;
                rtl_uString     *hostName= NULL;

                rtl_string2UString(
                    &hostName, Host, strlen(Host),
                    RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);
                OSL_ASSERT(hostName != 0);

                /* no, determine it via dns */
                pAddr = osl_createHostAddrByName(hostName);
                rtl_uString_release (hostName);

                if (pAddr && pAddr->pHostName)
                    memcpy(LocalHostname, pAddr->pHostName->buffer, sizeof(sal_Unicode)*(rtl_ustr_getLength(pAddr->pHostName->buffer)+1));
                else
                    memset(LocalHostname, 0, sizeof(LocalHostname));

                osl_destroyHostAddr ((oslHostAddr)pAddr);
            }
        }
    }

    if (rtl_ustr_getLength(LocalHostname) > 0)
    {
        rtl_uString_newFromStr (strLocalHostname, LocalHostname);
        return osl_Socket_Ok;
    }

    return osl_Socket_Error;
}

/*****************************************************************************/
/* osl_resolveHostname */
/*****************************************************************************/
oslSocketAddr SAL_CALL osl_resolveHostname(rtl_uString* strHostname)
{
    oslHostAddr pAddr=
        (oslHostAddr )osl_createHostAddrByName (strHostname);
    if (pAddr)
    {
        oslSocketAddr SockAddr = osl_copySocketAddr( pAddr->pSockAddr );
        osl_destroyHostAddr(pAddr);
        return (SockAddr);
    }
    return ((oslSocketAddr)NULL);
}

/*****************************************************************************/
/* osl_getServicePort */
/*****************************************************************************/
sal_Int32 SAL_CALL osl_getServicePort (
    rtl_uString* strServicename,
    rtl_uString* strProtocol)
{
    struct servent* ps;

    rtl_String *str_Servicename=NULL;
    rtl_String *str_Protocol=NULL;

    rtl_uString2String(
        &str_Servicename,
        rtl_uString_getStr(strServicename),
        rtl_uString_getLength(strServicename),
        RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);
    rtl_uString2String(
        &str_Protocol,
        rtl_uString_getStr(strProtocol),
        rtl_uString_getLength(strProtocol),
        RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);

    ps= getservbyname(
        rtl_string_getStr(str_Servicename),
        rtl_string_getStr(str_Protocol));

    rtl_string_release( str_Servicename );
    rtl_string_release( str_Protocol );

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
    if( pAddr )
    {
        struct sockaddr_in* pSystemInetAddr= (struct sockaddr_in*)&(pAddr->m_sockaddr);

        if ( (pSystemInetAddr->sin_family == FAMILY_TO_NATIVE(osl_Socket_FamilyInet)))
            return ntohs(pSystemInetAddr->sin_port);
    }
    return OSL_INVALID_PORT;
}

/*****************************************************************************/
/* osl_setInetPortOfSocketAddr */
/*****************************************************************************/
sal_Bool SAL_CALL osl_setInetPortOfSocketAddr (
    oslSocketAddr pAddr,
    sal_Int32     Port)
{
    if (pAddr == NULL)
        return sal_False;

    struct sockaddr_in* pSystemInetAddr= (struct sockaddr_in*)&(pAddr->m_sockaddr);

    if (pSystemInetAddr->sin_family != FAMILY_TO_NATIVE(osl_Socket_FamilyInet))
        return sal_False;

    pSystemInetAddr->sin_port= htons((short)Port);
    return sal_True;
}

/*****************************************************************************/
/* osl_getHostnameOfSocketAddr */
/*****************************************************************************/
oslSocketResult SAL_CALL osl_getHostnameOfSocketAddr (
    oslSocketAddr   Addr,
    rtl_uString   **strHostName)
{
    oslHostAddr pAddr= osl_createHostAddrByAddr (Addr);

    if (pAddr)
    {
        rtl_uString_newFromString(strHostName, pAddr->pHostName);

        osl_destroyHostAddr(pAddr);

        return osl_Socket_Ok;
    }

    return osl_Socket_Error;
}

/*****************************************************************************/
/* osl_getDottedInetAddrOfSocketAddr */
/*****************************************************************************/
oslSocketResult SAL_CALL osl_getDottedInetAddrOfSocketAddr (
    oslSocketAddr   pAddr,
    rtl_uString   **strDottedInetAddr)
{
    sal_Char           *pDotted;

    if (pAddr == NULL)
        return osl_Socket_Error;

    struct sockaddr_in *pSystemInetAddr = (struct sockaddr_in*) &(pAddr->m_sockaddr);
    if (pSystemInetAddr->sin_family != FAMILY_TO_NATIVE(osl_Socket_FamilyInet))
        return osl_Socket_Error;

    pDotted = inet_ntoa (pSystemInetAddr->sin_addr);
    rtl_string2UString(
        strDottedInetAddr, pDotted, strlen (pDotted),
        RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);
    OSL_ASSERT(*strDottedInetAddr != 0);

    return osl_Socket_Ok;
}

/*****************************************************************************/
/* osl_createSocket  */
/*****************************************************************************/
oslSocket SAL_CALL osl_createSocket (
    oslAddrFamily Family,
    oslSocketType Type,
    oslProtocol   Protocol)
{
    /* alloc memory */
    oslSocket pSocket = __osl_createSocketImpl(0);

    if (pSocket == NULL)
        return 0;

    /* create socket */
    pSocket->m_Socket= socket(FAMILY_TO_NATIVE(Family),
                                TYPE_TO_NATIVE(Type),
                                PROTOCOL_TO_NATIVE(Protocol));

    /* creation failed => free memory */
    if(pSocket->m_Socket == OSL_INVALID_SOCKET)
    {
        __osl_destroySocketImpl(pSocket);
        pSocket= 0;
    }
    else
    {
        pSocket->m_Flags            = 0;
        pSocket->m_CloseCallback    = NULL;
        pSocket->m_CallbackArg  = NULL;
    }

    return pSocket;
}

void SAL_CALL osl_acquireSocket( oslSocket pSocket )
{
    osl_atomic_increment( &(pSocket->m_nRefCount) );
}

void SAL_CALL osl_releaseSocket( oslSocket pSocket )
{
    if( pSocket && 0 == osl_atomic_decrement( &(pSocket->m_nRefCount) ) )
    {
        osl_closeSocket( pSocket );
        __osl_destroySocketImpl( pSocket );
    }
}

/*****************************************************************************/
/* osl_closeSocket  */
/*****************************************************************************/
void SAL_CALL osl_closeSocket(oslSocket pSocket)
{
    /* socket already invalid */
    if(pSocket==0)
        return;

    /* close */
    closesocket(pSocket->m_Socket);

    pSocket->m_Socket = OSL_INVALID_SOCKET;

    /* registrierten Callback ausfuehren */
    if (pSocket->m_CloseCallback != NULL)
    {
        pSocket->m_CloseCallback(pSocket->m_CallbackArg);
    }
}

/*****************************************************************************/
/* osl_getLocalAddrOfSocket  */
/* Note that I rely on the fact that oslSocketAddr and struct sockaddr */
/* are the same! I don't like it very much but see no other easy way */
/* to conceal the struct sockaddr from the eyes of the user. */
/*****************************************************************************/
oslSocketAddr SAL_CALL osl_getLocalAddrOfSocket(oslSocket pSocket)
{
    struct sockaddr Addr;
    int             AddrLen;

    if (pSocket == NULL) /* ENOTSOCK */
        return ((oslSocketAddr)NULL);

    AddrLen= sizeof(struct sockaddr);

    if (getsockname(pSocket->m_Socket, &Addr, &AddrLen) == OSL_SOCKET_ERROR)
        return ((oslSocketAddr)NULL);

    oslSocketAddr pAddr = __osl_createSocketAddrFromSystem( &Addr );
    return pAddr;
}

/*****************************************************************************/
/* osl_getPeerAddrOfSocket  */
/*****************************************************************************/
oslSocketAddr SAL_CALL osl_getPeerAddrOfSocket(oslSocket pSocket)
{
    struct sockaddr Addr;
    int             AddrLen;

    if (pSocket == NULL) /* ENOTSOCK */
        return ((oslSocketAddr)NULL);

    AddrLen= sizeof(struct sockaddr);

    if (getpeername(pSocket->m_Socket, &Addr, &AddrLen) == OSL_SOCKET_ERROR)
        return ((oslSocketAddr)NULL);

    oslSocketAddr pAddr = __osl_createSocketAddrFromSystem( &Addr );
    return pAddr;
}

/*****************************************************************************/
/* osl_bindAddrToSocket  */
/*****************************************************************************/
sal_Bool SAL_CALL osl_bindAddrToSocket ( oslSocket pSocket, oslSocketAddr pAddr)
{
    OSL_ASSERT( pAddr );

    if (pSocket == NULL) /* ENOTSOCK */
        return sal_False;

    return (bind(pSocket->m_Socket,
                 &(pAddr->m_sockaddr),
                 sizeof(struct sockaddr)) != OSL_SOCKET_ERROR);
}

/*****************************************************************************/
/* osl_connectSocketTo  */
/*****************************************************************************/
oslSocketResult SAL_CALL osl_connectSocketTo (
    oslSocket        pSocket,
    oslSocketAddr    pAddr,
    const TimeValue* pTimeout)
{

    if (pSocket == NULL) /* ENOTSOCK */
        return osl_Socket_Error;

    if (pAddr == NULL) /* EDESTADDRREQ */
        return osl_Socket_Error;

    if (!__osl_attemptSocketDialupImpl()) /* ENETDOWN */
        return osl_Socket_Error;

    if (pTimeout == NULL)
    {
        if(connect(pSocket->m_Socket,
                   &(pAddr->m_sockaddr),
                    sizeof(struct sockaddr)) == OSL_SOCKET_ERROR)
            return osl_Socket_Error;
        else
            return osl_Socket_Ok;
    }
    else
    {
        fd_set          fds;
        int             error;
        struct timeval  tv;
        unsigned long   Param;
        oslSocketResult Result= osl_Socket_Ok;

        if (pSocket->m_Flags & OSL_SOCKET_FLAGS_NONBLOCKING)
        {
            if (connect(pSocket->m_Socket,
                        &(pAddr->m_sockaddr),
                        sizeof(struct sockaddr)) == OSL_SOCKET_ERROR)
            {
                switch (WSAGetLastError())
                {
                    case WSAEWOULDBLOCK:
                    case WSAEINPROGRESS:
                        return osl_Socket_InProgress;

                    default:
                        return osl_Socket_Error;
                }
            }
            else
                return osl_Socket_Ok;
        }

        /* set socket temporarily to non-blocking */
        Param= 1;
        OSL_VERIFY(ioctlsocket(
            pSocket->m_Socket, FIONBIO, &Param) != OSL_SOCKET_ERROR);

        /* initiate connect */
        if (connect(pSocket->m_Socket,
                     &(pAddr->m_sockaddr),
                    sizeof(struct sockaddr)) != OSL_SOCKET_ERROR)
        {
           /* immediate connection */

            Param= 0;
            ioctlsocket(pSocket->m_Socket, FIONBIO, &Param);

            return osl_Socket_Ok;
        }
        else
        {
            error = WSAGetLastError();

            /* really an error or just delayed? */
            if (error != WSAEWOULDBLOCK && error != WSAEINPROGRESS)
            {
                 Param= 0;
                 ioctlsocket(pSocket->m_Socket, FIONBIO, &Param);

                 return osl_Socket_Error;
            }
        }

        /* prepare select set for socket  */
        FD_ZERO(&fds);
        FD_SET(pSocket->m_Socket, &fds);

        /* divide milliseconds into seconds and microseconds */
        tv.tv_sec=  pTimeout->Seconds;
        tv.tv_usec= pTimeout->Nanosec / 1000L;

        /* select */
        error= select(pSocket->m_Socket+1,
                       0,
                      &fds,
                      0,
                      &tv);

        if (error > 0)  /* connected */
        {
            OSL_POSTCOND(
                FD_ISSET(pSocket->m_Socket, &fds),
                "osl_connectSocketTo(): select returned but socket not set\n");

            Result= osl_Socket_Ok;

        }
        else if(error < 0)  /* error */
        {
            /* errno == EBADF: most probably interrupted by close() */
            if(WSAGetLastError() == WSAEBADF)
            {
                /* do not access pSockImpl because it is about to be or */
                /* already destroyed */
                return osl_Socket_Interrupted;
            }
            else
                Result= osl_Socket_Error;

        }
        else    /* timeout */
            Result= osl_Socket_TimedOut;


        /* clean up */
        Param= 0;
        ioctlsocket(pSocket->m_Socket, FIONBIO, &Param);

        return Result;
    }
}

/*****************************************************************************/
/* osl_listenOnSocket  */
/*****************************************************************************/
sal_Bool SAL_CALL osl_listenOnSocket (
    oslSocket  pSocket,
    sal_Int32  MaxPendingConnections)
{
    if (pSocket == NULL) /* ENOTSOCK */
        return sal_False;

    return (listen(pSocket->m_Socket,
                   MaxPendingConnections == -1 ?
                   SOMAXCONN :
                   MaxPendingConnections) != OSL_SOCKET_ERROR);
}

/*****************************************************************************/
/* osl_acceptConnectionOnSocket  */
/*****************************************************************************/
oslSocket SAL_CALL osl_acceptConnectionOnSocket (
    oslSocket      pSocket,
    oslSocketAddr* ppAddr)
{
    if (pSocket == NULL) /* ENOTSOCK */
        return ((oslSocket)NULL);

    SOCKET          Connection;
    if(ppAddr)
    {
        if( *ppAddr )
        {
            osl_destroySocketAddr( *ppAddr );
            *ppAddr = 0;
        }
        int AddrLen= sizeof(struct sockaddr);

        /* user wants to know peer Addr */
        struct sockaddr Addr;

        Connection= accept(pSocket->m_Socket, &Addr, &AddrLen);
        OSL_ASSERT(AddrLen == sizeof(struct sockaddr));

        if(Connection != static_cast<SOCKET>(OSL_SOCKET_ERROR))
            *ppAddr= __osl_createSocketAddrFromSystem(&Addr);
        else
            *ppAddr = NULL;
    }
    else
    {
        /* user is not interested in peer-addr */
        Connection= accept(pSocket->m_Socket, 0, 0);
    }

    /* accept failed? */
    if(Connection == static_cast<SOCKET>(OSL_SOCKET_ERROR))
        return ((oslSocket)NULL);

    /* alloc memory */
    oslSocket  pConnectionSocket;
    pConnectionSocket= __osl_createSocketImpl(Connection);

    pConnectionSocket->m_Flags          = 0;
    pConnectionSocket->m_CloseCallback  = NULL;
    pConnectionSocket->m_CallbackArg    = NULL;

    return pConnectionSocket;
}

/*****************************************************************************/
/* osl_receiveSocket  */
/*****************************************************************************/
sal_Int32 SAL_CALL osl_receiveSocket (
    oslSocket        pSocket,
    void*            pBuffer,
    sal_uInt32       BytesToRead,
    oslSocketMsgFlag Flag)
{
    if (pSocket == NULL) /* ENOTSOCK */
        return osl_Socket_Error;

    return recv(pSocket->m_Socket,
                (sal_Char*)pBuffer,
                BytesToRead,
                MSG_FLAG_TO_NATIVE(Flag));
}

/*****************************************************************************/
/* osl_receiveFromSocket  */
/*****************************************************************************/
sal_Int32 SAL_CALL osl_receiveFromSocket (
    oslSocket        pSocket,
    oslSocketAddr    SenderAddr,
    void*            pBuffer,
    sal_uInt32       BufferSize,
    oslSocketMsgFlag Flag)
{
    struct sockaddr *pSystemSockAddr = 0;
    int AddrLen = 0;
    if( SenderAddr )
    {
        AddrLen = sizeof( struct sockaddr );
        pSystemSockAddr = &(SenderAddr->m_sockaddr);
    }

    if (pSocket == NULL) /* ENOTSOCK */
        return osl_Socket_Error;

    return recvfrom(pSocket->m_Socket,
                     (sal_Char*)pBuffer,
                     BufferSize,
                     MSG_FLAG_TO_NATIVE(Flag),
                     pSystemSockAddr,
                     &AddrLen);
}

/*****************************************************************************/
/* osl_sendSocket  */
/*****************************************************************************/
sal_Int32 SAL_CALL osl_sendSocket (
    oslSocket        pSocket,
    const void*      pBuffer,
    sal_uInt32       BytesToSend,
    oslSocketMsgFlag Flag)
{
    if (pSocket == NULL) /* ENOTSOCK */
        return osl_Socket_Error;

    return send(pSocket->m_Socket,
                (sal_Char*)pBuffer,
                BytesToSend,
                MSG_FLAG_TO_NATIVE(Flag));
}

/*****************************************************************************/
/* osl_sendToSocket  */
/*****************************************************************************/
sal_Int32 SAL_CALL osl_sendToSocket (
    oslSocket        pSocket,
    oslSocketAddr    ReceiverAddr,
    const void*      pBuffer,
    sal_uInt32       BytesToSend,
    oslSocketMsgFlag Flag)
{
    if (pSocket == NULL) /* ENOTSOCK */
        return osl_Socket_Error;

    /* ReceiverAddr might be 0 when used on a connected socket. */
    /* Then sendto should behave like send. */

    struct sockaddr *pSystemSockAddr = 0;
    if( ReceiverAddr )
        pSystemSockAddr = &(ReceiverAddr->m_sockaddr);

    return sendto(pSocket->m_Socket,
                  (sal_Char*)pBuffer,
                  BytesToSend,
                  MSG_FLAG_TO_NATIVE(Flag),
                  pSystemSockAddr,
                  pSystemSockAddr == 0 ? 0 : sizeof(struct sockaddr));
}

/*****************************************************************************/
/* osl_readSocket  */
/*****************************************************************************/
sal_Int32 SAL_CALL osl_readSocket( oslSocket pSocket, void *pBuffer, sal_Int32 n )
{
    sal_uInt8 * Ptr = (sal_uInt8 *)pBuffer;

    OSL_ASSERT( pSocket);

    /* loop until all desired bytes were read or an error occurred */
    sal_uInt32 BytesRead= 0;
    sal_uInt32 BytesToRead= n;
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
sal_Int32 SAL_CALL osl_writeSocket( oslSocket pSocket, const void *pBuffer, sal_Int32 n )
{
    OSL_ASSERT( pSocket );

    /* loop until all desired bytes were send or an error occurred */
    sal_uInt32 BytesSend= 0;
    sal_uInt32 BytesToSend= n;
    sal_uInt8 *Ptr = ( sal_uInt8 * )pBuffer;
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
/* osl_isReceiveReady  */
/*****************************************************************************/
sal_Bool SAL_CALL osl_isReceiveReady (
    oslSocket        pSocket,
    const TimeValue* pTimeout)
{
    fd_set         fds;
    struct timeval tv;

    if (pSocket == NULL) /* ENOTSOCK */
        return sal_False;

    FD_ZERO(&fds);
    FD_SET(pSocket->m_Socket, &fds);

    if (pTimeout)
    {
        tv.tv_sec  = pTimeout->Seconds;
        tv.tv_usec = pTimeout->Nanosec / 1000L;
    }

    return (select(pSocket->m_Socket + 1,       /* no of sockets to monitor */
                   &fds,                        /* check read operations */
                   0,                           /* check write ops */
                   0,                           /* ckeck for OOB */
                   (pTimeout) ? &tv : 0)==1);   /* use timeout? */
}

/*****************************************************************************/
/* osl_isSendReady  */
/*****************************************************************************/
sal_Bool SAL_CALL osl_isSendReady (
    oslSocket        pSocket,
    const TimeValue* pTimeout)
{
    fd_set         fds;
    struct timeval tv;

    if (pSocket == NULL) /* ENOTSOCK */
        return sal_False;

    FD_ZERO(&fds);
    FD_SET(pSocket->m_Socket, &fds);

    if (pTimeout)
    {
        tv.tv_sec  = pTimeout->Seconds;
        tv.tv_usec = pTimeout->Nanosec / 1000L;
    }

    return (select(pSocket->m_Socket + 1,       /* no of sockets to monitor */
                   0,                           /* check read operations */
                   &fds,                        /* check write ops */
                   0,                           /* ckeck for OOB */
                   (pTimeout) ? &tv : 0)==1);   /* use timeout? */
}

/*****************************************************************************/
/* osl_isExceptionPending  */
/*****************************************************************************/
sal_Bool SAL_CALL osl_isExceptionPending (
    oslSocket        pSocket,
    const TimeValue* pTimeout)
{
    fd_set         fds;
    struct timeval tv;

    if (pSocket == NULL) /* ENOTSOCK */
        return sal_False;

    FD_ZERO(&fds);
    FD_SET(pSocket->m_Socket, &fds);

    if (pTimeout)
    {
        tv.tv_sec  = pTimeout->Seconds;
        tv.tv_usec = pTimeout->Nanosec / 1000L;
    }

    return (select(pSocket->m_Socket + 1,       /* no of sockets to monitor */
                   0,                           /* check read operations */
                   0,                           /* check write ops */
                   &fds,                        /* ckeck for OOB */
                   (pTimeout) ? &tv : 0)==1);   /* use timeout? */
}

/*****************************************************************************/
/* osl_shutdownSocket  */
/*****************************************************************************/
sal_Bool SAL_CALL osl_shutdownSocket (
    oslSocket          pSocket,
    oslSocketDirection Direction)
{
    if (pSocket == NULL) /* ENOTSOCK */
        return sal_False;

    return (shutdown(pSocket->m_Socket, DIRECTION_TO_NATIVE(Direction))==0);
}

/*****************************************************************************/
/* osl_getSocketOption  */
/*****************************************************************************/
sal_Int32 SAL_CALL osl_getSocketOption (
    oslSocket            pSocket,
    oslSocketOptionLevel Level,
    oslSocketOption      Option,
    void*                pBuffer,
    sal_uInt32           BufferLen)
{
    if (pSocket == NULL) /* ENOTSOCK */
        return osl_Socket_Error;

    if (getsockopt(pSocket->m_Socket,
                    OPTION_LEVEL_TO_NATIVE(Level),
                   OPTION_TO_NATIVE(Option),
                   (sal_Char*)pBuffer,
                   (int*)&BufferLen) == -1)
    {
        return -1;
    }

    return (sal_Int32)BufferLen;
}

/*****************************************************************************/
/* osl_setSocketOption  */
/*****************************************************************************/
sal_Bool SAL_CALL osl_setSocketOption (
    oslSocket            pSocket,
    oslSocketOptionLevel Level,
    oslSocketOption      Option,
    void*                pBuffer,
    sal_uInt32           BufferLen)
{
    if (pSocket == NULL) /* ENOTSOCK */
        return sal_False;

    return(setsockopt(pSocket->m_Socket,
                      OPTION_LEVEL_TO_NATIVE(Level),
                      OPTION_TO_NATIVE(Option),
                      (sal_Char*)pBuffer,
                      BufferLen) == 0);
}

/*****************************************************************************/
/* osl_enableNonBlockingMode  */
/*****************************************************************************/
sal_Bool SAL_CALL osl_enableNonBlockingMode ( oslSocket pSocket, sal_Bool  On)
{
    unsigned long  Param= On ? 1 : 0;

    if (pSocket == NULL) /* ENOTSOCK */
        return sal_False;

    pSocket->m_Flags = Param ?
        (pSocket->m_Flags |  OSL_SOCKET_FLAGS_NONBLOCKING) :
        (pSocket->m_Flags & ~OSL_SOCKET_FLAGS_NONBLOCKING) ;

    return (
        ioctlsocket(pSocket->m_Socket, FIONBIO, &Param) != OSL_SOCKET_ERROR);
}

/*****************************************************************************/
/* osl_isNonBlockingMode  */
/*****************************************************************************/
sal_Bool SAL_CALL osl_isNonBlockingMode(oslSocket pSocket)
{
    if (pSocket == NULL) /* ENOTSOCK */
        return sal_False;

    return (sal_Bool)((pSocket->m_Flags & OSL_SOCKET_FLAGS_NONBLOCKING) != 0);
}

/*****************************************************************************/
/* osl_getSocketType  */
/*****************************************************************************/
oslSocketType SAL_CALL osl_getSocketType(oslSocket pSocket)
{
    int            Type=0;
    int            TypeSize= sizeof(Type);

    if (pSocket == NULL) /* ENOTSOCK */
        return osl_Socket_TypeInvalid;

    if(getsockopt(pSocket->m_Socket,
                  OPTION_LEVEL_TO_NATIVE(osl_Socket_LevelSocket),
                  OPTION_TO_NATIVE(osl_Socket_OptionType),
                  (sal_Char *)&Type,
                  &TypeSize) == -1)
    {
        /* error */
        return osl_Socket_TypeInvalid;
    }

    return TYPE_FROM_NATIVE(Type);
}

/*****************************************************************************/
/* osl_getLastSocketErrorDescription  */
/*****************************************************************************/
void SAL_CALL osl_getLastSocketErrorDescription (
    oslSocket  /*Socket*/,
    rtl_uString **strError)
{
    int error;

    switch(error = WSAGetLastError())
    {
        case WSAENOTSOCK:
            rtl_uString_newFromAscii (strError, "WSAENOTSOCK, Socket operation on non-socket. A socket created in one process is used by another process.");
            break;

        case WSAEDESTADDRREQ:
            rtl_uString_newFromAscii (strError, "WSAEDESTADDRREQ, Destination Addr required");
            break;

        case WSAEMSGSIZE:
            rtl_uString_newFromAscii (strError, "WSAEMSGSIZE, Message too long");
            break;

        case WSAEPROTOTYPE:
            rtl_uString_newFromAscii (strError, "WSAEPROTOTYPE, Protocol wrong type for socket");
            break;

        case WSAENOPROTOOPT:
            rtl_uString_newFromAscii (strError, "WSAENOPROTOOPT, Protocol not available");
            break;

        case WSAEPROTONOSUPPORT:
            rtl_uString_newFromAscii (strError, "WSAEPROTONOSUPPORT, Protocol not supported");
            break;

        case WSAESOCKTNOSUPPORT:
            rtl_uString_newFromAscii (strError, "WSAESOCKTNOSUPPORT, Socket type not supported");
            break;

        case WSAEOPNOTSUPP:
            rtl_uString_newFromAscii (strError, "WSAEOPNOTSUPP, Operation not supported on socket");
            break;

        case WSAEPFNOSUPPORT:
            rtl_uString_newFromAscii (strError, "WSAEPFNOSUPPORT, Protocol family not supported");
            break;

        case WSAEAFNOSUPPORT:
            rtl_uString_newFromAscii (strError, "WSEAFNOSUPPORT, Addr family not supported by protocol family");
            break;

        case WSAEADDRINUSE:
            rtl_uString_newFromAscii (strError, "WSAEADDRINUSE, Triggered by bind() because a process went down without closing a socket.");
            break;

        case WSAEADDRNOTAVAIL:
            rtl_uString_newFromAscii (strError, "WSAEADDRNOTAVAIL, Can't assign requested Addr");
            break;

        case WSAENETDOWN:
            rtl_uString_newFromAscii (strError, "WSAENETDOWN, Network is down");
            break;

        case WSAENETUNREACH:
            rtl_uString_newFromAscii (strError, "WSAENETUNREACH, Network is unreachable");
            break;

        case WSAENETRESET:
            rtl_uString_newFromAscii (strError, "WSAENETRESET, Network dropped connection or reset");
            break;

        case WSAECONNABORTED:
            rtl_uString_newFromAscii (strError, "WSAECONNABORTED, Software caused connection abort");
            break;

        case WSAECONNRESET:
            rtl_uString_newFromAscii (strError, "WSAECONNRESET, Connection reset by peer");
            break;

        case WSAENOBUFS:
            rtl_uString_newFromAscii (strError, "WSAENOBUFS, No buffer space available.");
            break;

        case WSAEISCONN:
            rtl_uString_newFromAscii (strError, "WSAEISCONN, Socket is already connected");
            break;

        case WSAENOTCONN:
            rtl_uString_newFromAscii (strError, "WSAENOTCONN, Socket is not connected");
            break;

        case WSAESHUTDOWN:
            rtl_uString_newFromAscii (strError, "WSAESHUTDOWN, Can't send after socket shutdown");
            break;

        case WSAETIMEDOUT:
            rtl_uString_newFromAscii (strError, "WSAETIMEDOUT, Connection timed out");
            break;

        case WSAECONNREFUSED:
            rtl_uString_newFromAscii (strError, "WSAECONNREFUSED, Connection refused");
            break;

        case WSAEHOSTDOWN:
            rtl_uString_newFromAscii (strError, "WSAEHOSTDOWN, Networking subsystem not started");
            break;

        case WSAEHOSTUNREACH:
            rtl_uString_newFromAscii (strError, "WSAEHOSTUNREACH, No route to host");
            break;

        case WSAEWOULDBLOCK:
            rtl_uString_newFromAscii (strError, "WSAEWOULDBLOCK, Operation would block");
            break;

        case WSAEINPROGRESS:
            rtl_uString_newFromAscii (strError, "WSAEINPROGRESS, Operation now in progress");
            break;

        case WSAEALREADY:
            rtl_uString_newFromAscii (strError, "WSAEALREADY, Operation already in progress");
            break;

        case WSAEINTR:
            rtl_uString_newFromAscii (strError, "WSAEALREADY, Operation was interrupted");
            break;

        case WSAEBADF:
            rtl_uString_newFromAscii (strError, "WSAEBADF, Bad file number");
            break;

        case WSAEACCES:
            rtl_uString_newFromAscii (strError, "WSAEACCES, Access is denied");
            break;

        case WSAEFAULT:
            rtl_uString_newFromAscii (strError, "WSAEFAULT, Bad memory Addr");
            break;

        case WSAEINVAL:
            rtl_uString_newFromAscii (strError, "WSAEINVAL, The socket has not been bound with bind() or is already connected");
            break;

        case WSAEMFILE:
            rtl_uString_newFromAscii (strError, "WSAEMFILE, No more file descriptors are available");
            break;

        case WSAETOOMANYREFS:
            rtl_uString_newFromAscii (strError, "WSAETOOMANYREFS, Undocumented WinSock error");
            break;

        case WSAENAMETOOLONG:
            rtl_uString_newFromAscii (strError, "WSAENAMETOOLONG, Undocumented WinSock error");
            break;

        case WSAENOTEMPTY:
            rtl_uString_newFromAscii (strError, "WSAENOTEMPTY, Undocumented WinSock error");
            break;

        case WSAEPROCLIM:
            rtl_uString_newFromAscii (strError, "WSAEPROCLIM, Undocumented WinSock error");
            break;

        case WSAEUSERS:
            rtl_uString_newFromAscii (strError, "WSAEUSERS, Undocumented WinSock error");
            break;

        case WSAEDQUOT:
            rtl_uString_newFromAscii (strError, "WSAEDQUOT, Undocumented WinSock error");
            break;

        case WSAESTALE:
            rtl_uString_newFromAscii (strError, "WSAESTALE, Undocumented WinSock error");
            break;

        case WSAEREMOTE:
            rtl_uString_newFromAscii (strError, "WSAEREMOTE, Undocumented WinSock error");
            break;

        case WSAEDISCON:
            rtl_uString_newFromAscii (strError, "WSAEDISCON, Circuit was gracefully terminated");
            break;

        case WSASYSNOTREADY:
            rtl_uString_newFromAscii (strError, "WSASYSNOTREADY, The underlying network subsystem is not ready for network communication");
            break;

        case WSAVERNOTSUPPORTED:
            rtl_uString_newFromAscii (strError, "WSAVERNOTSUPPORTED, The version of Windows Sockets API support requested is not provided by this particular Windows Sockets implementation");
            break;

        case WSANOTINITIALISED:
            rtl_uString_newFromAscii (strError, "WSANOTINITIALISED, WSAStartup() has not been called");
            break;

        case WSAHOST_NOT_FOUND:
            rtl_uString_newFromAscii (strError, "WSAHOST_NOT_FOUND, Authoritative answer host not found");
            break;

        case WSATRY_AGAIN:
            rtl_uString_newFromAscii (strError, "WSATRY_AGAIN, Non-authoritative answer host not found or SERVERFAIL");
            break;

        case WSANO_RECOVERY:
            rtl_uString_newFromAscii (strError, "WSANO_RECOVERY, Non recoverable errors, FORMERR, REFUSED, NOTIMP");
            break;

        case WSANO_DATA:
            rtl_uString_newFromAscii (strError, "WSANO_DATA or WSANO_ADDRESS, Valid name, no data record of requested type");
            break;

        default:
        {
            sal_Unicode message[128];

            wsprintfW(reinterpret_cast<LPWSTR>(message), L"Unknown WinSock Error Number %d", error);
            rtl_uString_newFromStr (strError, message);
        }

        return;

    }
}

/*****************************************************************************/
/* osl_getLastSocketError  */
/*****************************************************************************/
oslSocketError SAL_CALL osl_getLastSocketError(oslSocket /*Socket*/)
{
    return ERROR_FROM_NATIVE(WSAGetLastError());
}

/*****************************************************************************/
/* SocketSet                                                                 */
/*****************************************************************************/
typedef struct _TSocketSetImpl
{
    fd_set  m_Set;          /* the set of descriptors */

} TSocketSetImpl;

/*****************************************************************************/
/* osl_createSocketSet  */
/*****************************************************************************/
oslSocketSet SAL_CALL osl_createSocketSet()
{
    TSocketSetImpl* pSet;

    pSet = (TSocketSetImpl*) rtl_allocateMemory(sizeof(TSocketSetImpl));

    if(pSet)
    {
        FD_ZERO(&pSet->m_Set);
    }

    return (oslSocketSet)pSet;
}

/*****************************************************************************/
/* osl_destroySocketSet  */
/*****************************************************************************/
void SAL_CALL osl_destroySocketSet (oslSocketSet Set)
{
    if(Set)
        rtl_freeMemory(Set);
}

/*****************************************************************************/
/* osl_clearSocketSet  */
/*****************************************************************************/
void SAL_CALL osl_clearSocketSet (oslSocketSet Set)
{
    TSocketSetImpl* pSet;

    pSet= (TSocketSetImpl*)Set;

    if (pSet)
        FD_ZERO(&pSet->m_Set);
}

/*****************************************************************************/
/* osl_addToSocketSet  */
/*****************************************************************************/
void SAL_CALL osl_addToSocketSet (
    oslSocketSet Set,
    oslSocket    Socket)
{
    TSocketSetImpl* pSet;
    oslSocketImpl*  pSockImpl;

    pSet= (TSocketSetImpl*)Set;
    pSockImpl= (oslSocketImpl*)Socket;

    if (pSet && pSockImpl)
        FD_SET(pSockImpl->m_Socket, &pSet->m_Set);
}

/*****************************************************************************/
/* osl_removeFromSocketSet  */
/*****************************************************************************/
void SAL_CALL osl_removeFromSocketSet (
    oslSocketSet Set,
    oslSocket    Socket)
{
    TSocketSetImpl* pSet;
    oslSocketImpl*  pSockImpl;

    pSet= (TSocketSetImpl*)Set;
    pSockImpl= (oslSocketImpl*)Socket;

    if (pSet && pSockImpl)
        FD_CLR(pSockImpl->m_Socket, &pSet->m_Set);
}

/*****************************************************************************/
/* osl_isInSocketSet  */
/*****************************************************************************/
sal_Bool SAL_CALL osl_isInSocketSet (
    oslSocketSet Set,
    oslSocket    Socket)
{
    TSocketSetImpl* pSet;
    oslSocketImpl*  pSockImpl;

    pSet= (TSocketSetImpl*)Set;
    pSockImpl= (oslSocketImpl*)Socket;

    if (pSet && pSockImpl)
        return (FD_ISSET(pSockImpl->m_Socket, &pSet->m_Set) != 0);
    else
        return sal_False;
}

/*****************************************************************************/
/* osl_demultiplexSocketEvents  */
/*****************************************************************************/
sal_Int32 SAL_CALL osl_demultiplexSocketEvents (
    oslSocketSet IncomingSet,
    oslSocketSet OutgoingSet,
    oslSocketSet OutOfBandSet,
    const TimeValue* pTimeout)
{
    int             MaxHandle= 0;
    struct timeval  tv;
    TSocketSetImpl* pInSet;
    TSocketSetImpl* pOutSet;
    TSocketSetImpl* pOOBSet;

    if(pTimeout)
    {
        /* divide milliseconds into seconds and microseconds */
        tv.tv_sec  = pTimeout->Seconds;
        tv.tv_usec = pTimeout->Nanosec / 1000L;
    }

    /* map opaque data to impl-types */
    pInSet= (TSocketSetImpl*)IncomingSet;
    pOutSet= (TSocketSetImpl*)OutgoingSet;
    pOOBSet= (TSocketSetImpl*)OutOfBandSet;

    return select(MaxHandle,                /* redundant in WIN32 */
                  pInSet ? &pInSet->m_Set : 0,
                  pOutSet ? &pOutSet->m_Set : 0,
                  pOOBSet ? &pOOBSet->m_Set : 0,
                  pTimeout ? &tv : 0);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
