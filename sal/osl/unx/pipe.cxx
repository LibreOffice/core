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

#include "system.hxx"

#include <osl/pipe.h>
#include <osl/diagnose.h>
#include <osl/thread.h>
#include <osl/interlck.h>
#include <rtl/string.h>
#include <rtl/ustring.h>
#include <rtl/bootstrap.hxx>
#include <sal/log.hxx>

#include "sockimpl.hxx"
#include "secimpl.hxx"
#include "unixerrnostring.hxx"

#include <cassert>
#include <cstring>

#define PIPEDEFAULTPATH     "/tmp"
#define PIPEALTERNATEPATH   "/var/tmp"

static oslPipe osl_psz_createPipe(const sal_Char *pszPipeName, oslPipeOptions Options, oslSecurity Security);

static struct
{
    int const      errcode;
    oslPipeError   error;
} const PipeError[]= {
    { 0,               osl_Pipe_E_None              },  /* no error */
    { EPROTOTYPE,      osl_Pipe_E_NoProtocol        },  /* Protocol wrong type for socket */
    { ENOPROTOOPT,     osl_Pipe_E_NoProtocol        },  /* Protocol not available */
    { EPROTONOSUPPORT, osl_Pipe_E_NoProtocol        },  /* Protocol not supported */
#ifdef ESOCKTNOSUPPORT
    { ESOCKTNOSUPPORT, osl_Pipe_E_NoProtocol        },  /* Socket type not supported */
#endif
    { EPFNOSUPPORT,    osl_Pipe_E_NoProtocol        },  /* Protocol family not supported */
    { EAFNOSUPPORT,    osl_Pipe_E_NoProtocol        },  /* Address family not supported by */
                                                        /* protocol family */
    { ENETRESET,       osl_Pipe_E_NetworkReset      },  /* Network dropped connection because */
                                                         /* of reset */
    { ECONNABORTED,    osl_Pipe_E_ConnectionAbort   },  /* Software caused connection abort */
    { ECONNRESET,      osl_Pipe_E_ConnectionReset   },  /* Connection reset by peer */
    { ENOBUFS,         osl_Pipe_E_NoBufferSpace     },  /* No buffer space available */
    { ETIMEDOUT,       osl_Pipe_E_TimedOut          },  /* Connection timed out */
    { ECONNREFUSED,    osl_Pipe_E_ConnectionRefused },  /* Connection refused */
    { -1,              osl_Pipe_E_invalidError      }
};

static oslPipeError osl_PipeErrorFromNative(int nativeType)
{
    int i = 0;

    while ((PipeError[i].error != osl_Pipe_E_invalidError) &&
           (PipeError[i].errcode != nativeType))
    {
        i++;
    }

    return PipeError[i].error;
}

static oslPipe createPipeImpl()
{
    oslPipe pPipeImpl;

    pPipeImpl = static_cast< oslPipe >(calloc(1, sizeof(struct oslPipeImpl)));
    if (!pPipeImpl)
        return nullptr;

    pPipeImpl->m_nRefCount = 1;
    pPipeImpl->m_bClosed = false;
#if defined(CLOSESOCKET_DOESNT_WAKE_UP_ACCEPT)
    pPipeImpl->m_bIsInShutdown = false;
    pPipeImpl->m_bIsAccepting = false;
#endif

    return pPipeImpl;
}

static void destroyPipeImpl(oslPipe pImpl)
{
    if (pImpl)
        free(pImpl);
}

oslPipe SAL_CALL osl_createPipe(rtl_uString *ustrPipeName, oslPipeOptions Options, oslSecurity Security)
{
    oslPipe pPipe = nullptr;
    rtl_String* strPipeName = nullptr;

    if (ustrPipeName)
    {
        rtl_uString2String(&strPipeName,
                           rtl_uString_getStr(ustrPipeName),
                           rtl_uString_getLength(ustrPipeName),
                           osl_getThreadTextEncoding(),
                           OUSTRING_TO_OSTRING_CVTFLAGS);
        sal_Char* pszPipeName = rtl_string_getStr(strPipeName);
        pPipe = osl_psz_createPipe(pszPipeName, Options, Security);

        if (strPipeName)
            rtl_string_release(strPipeName);
    }

    return pPipe;

}

static OString
getBootstrapSocketPath()
{
    OUString pValue;

    if (rtl::Bootstrap::get("OSL_SOCKET_PATH", pValue))
    {
        return OUStringToOString(pValue, RTL_TEXTENCODING_UTF8);
    }
    return "";
}

static oslPipe osl_psz_createPipe(const sal_Char *pszPipeName, oslPipeOptions Options,
                                    oslSecurity Security)
{
    int Flags;
    size_t len;
    struct sockaddr_un addr;

    OString name;
    oslPipe pPipe;

    if (access(PIPEDEFAULTPATH, W_OK) == 0)
        name = PIPEDEFAULTPATH;
    else if (access(PIPEALTERNATEPATH, W_OK) == 0)
        name = PIPEALTERNATEPATH;
    else {
        name = getBootstrapSocketPath ();
    }

    name += "/";

    if (Security)
    {
        sal_Char Ident[256];

        Ident[0] = '\0';

        OSL_VERIFY(osl_psz_getUserIdent(Security, Ident, sizeof(Ident)));

        name += OStringLiteral("OSL_PIPE_") + Ident + "_" + pszPipeName;
    }
    else
    {
        name += OStringLiteral("OSL_PIPE_") + pszPipeName;
    }

    if (sal_uInt32(name.getLength()) >= sizeof addr.sun_path)
    {
        SAL_WARN("sal.osl.pipe", "osl_createPipe: pipe name too long");
        return nullptr;
    }

    /* alloc memory */
    pPipe = createPipeImpl();

    if (!pPipe)
        return nullptr;

    /* create socket */
    pPipe->m_Socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (pPipe->m_Socket < 0)
    {
        SAL_WARN("sal.osl.pipe", "socket() failed: " << UnixErrnoString(errno));
        destroyPipeImpl(pPipe);
        return nullptr;
    }

    /* set close-on-exec flag */
    if ((Flags = fcntl(pPipe->m_Socket, F_GETFD, 0)) != -1)
    {
        Flags |= FD_CLOEXEC;
        if (fcntl(pPipe->m_Socket, F_SETFD, Flags) == -1)
        {
            SAL_WARN("sal.osl.pipe", "fcntl() failed: " << UnixErrnoString(errno));
        }
    }

    memset(&addr, 0, sizeof(addr));

    SAL_INFO("sal.osl.pipe", "new pipe on fd " << pPipe->m_Socket << " '" << name << "'");

    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, name.getStr()); // safe, see check above
#if defined(FREEBSD)
    len = SUN_LEN(&addr);
#else
    len = sizeof(addr);
#endif

    if (Options & osl_Pipe_CREATE)
    {
        struct stat status;

        /* check if there exists an orphan filesystem entry */
        if ((stat(name.getStr(), &status) == 0) &&
            (S_ISSOCK(status.st_mode) || S_ISFIFO(status.st_mode)))
        {
            if (connect(pPipe->m_Socket, reinterpret_cast< sockaddr* >(&addr), len) >= 0)
            {
                close (pPipe->m_Socket);
                destroyPipeImpl(pPipe);
                return nullptr;
            }

            unlink(name.getStr());
        }

        /* ok, fs clean */
        if (bind(pPipe->m_Socket, reinterpret_cast< sockaddr* >(&addr), len) < 0)
        {
            SAL_WARN("sal.osl.pipe", "bind() failed: " << UnixErrnoString(errno));
            close(pPipe->m_Socket);
            destroyPipeImpl(pPipe);
            return nullptr;
        }

        /*  Only give access to all if no security handle was specified, otherwise security
            depends on umask */

        if (!Security)
            chmod(name.getStr(),S_IRWXU | S_IRWXG |S_IRWXO);

        strcpy(pPipe->m_Name, name.getStr()); // safe, see check above

        if (listen(pPipe->m_Socket, 5) < 0)
        {
            SAL_WARN("sal.osl.pipe", "listen() failed: " << UnixErrnoString(errno));
            // cid#1255391 warns about unlink(name) after stat(name, &status)
            // above, but the intervening call to bind makes those two clearly
            // unrelated, as it would fail if name existed at that point in
            // time:
            // coverity[toctou] - this is bogus
            unlink(name.getStr());   /* remove filesystem entry */
            close(pPipe->m_Socket);
            destroyPipeImpl(pPipe);
            return nullptr;
        }

        return pPipe;
    }

    /* osl_pipe_OPEN */
    if (access(name.getStr(), F_OK) != -1)
    {
        if (connect(pPipe->m_Socket, reinterpret_cast< sockaddr* >(&addr), len) >= 0)
            return pPipe;

        SAL_WARN("sal.osl.pipe", "connect() failed: " << UnixErrnoString(errno));
    }

    close (pPipe->m_Socket);
    destroyPipeImpl(pPipe);
    return nullptr;
}

void SAL_CALL osl_acquirePipe(oslPipe pPipe)
{
    osl_atomic_increment(&(pPipe->m_nRefCount));
}

void SAL_CALL osl_releasePipe(oslPipe pPipe)
{
    if (!pPipe)
        return;

    if (osl_atomic_decrement(&(pPipe->m_nRefCount)) == 0)
    {
        if (!pPipe->m_bClosed)
            osl_closePipe(pPipe);

        destroyPipeImpl(pPipe);
    }
}

void SAL_CALL osl_closePipe(oslPipe pPipe)
{
    int nRet;
    int ConnFD;

    if (!pPipe)
        return;

    if (pPipe->m_bClosed)
        return;

    ConnFD = pPipe->m_Socket;

    /* Thread does not return from accept on linux, so
       connect to the accepting pipe
     */
#if defined(CLOSESOCKET_DOESNT_WAKE_UP_ACCEPT)
    struct sockaddr_un addr;

    if (pPipe->m_bIsAccepting)
    {
        pPipe->m_bIsInShutdown = true;
        pPipe->m_Socket = -1;

        int fd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (fd < 0)
        {
            SAL_WARN("sal.osl.pipe", "socket() failed: " << UnixErrnoString(errno));
            return;
        }

        memset(&addr, 0, sizeof(addr));

        SAL_INFO("sal.osl.pipe", "osl_destroyPipe : Pipe Name '" << pPipe->m_Name << "'");

        addr.sun_family = AF_UNIX;
        strcpy(addr.sun_path, pPipe->m_Name); // safe, as both are same size

        nRet = connect(fd, reinterpret_cast< sockaddr* >(&addr), sizeof(addr));
        if (nRet < 0)
            SAL_WARN("sal.osl.pipe", "connect() failed: " << UnixErrnoString(errno));

        close(fd);
    }
#endif /* CLOSESOCKET_DOESNT_WAKE_UP_ACCEPT */

    nRet = shutdown(ConnFD, 2);
    if (nRet < 0)
        SAL_WARN("sal.osl.pipe", "shutdown() failed: " << UnixErrnoString(errno));

    nRet = close(ConnFD);
    if (nRet < 0)
        SAL_WARN("sal.osl.pipe", "close() failed: " << UnixErrnoString(errno));

    /* remove filesystem entry */
    if (pPipe->m_Name[0] != '\0')
        unlink(pPipe->m_Name);

    pPipe->m_bClosed = true;
}

oslPipe SAL_CALL osl_acceptPipe(oslPipe pPipe)
{
    int s;
    oslPipe pAcceptedPipe;

    SAL_WARN_IF(!pPipe, "sal.osl.pipe", "invalid pipe");
    if (!pPipe)
        return nullptr;

    assert(pPipe->m_Name[0] != '\0');  // you cannot have an empty pipe name

#if defined(CLOSESOCKET_DOESNT_WAKE_UP_ACCEPT)
    pPipe->m_bIsAccepting = true;
#endif

    s = accept(pPipe->m_Socket, nullptr, nullptr);

#if defined(CLOSESOCKET_DOESNT_WAKE_UP_ACCEPT)
    pPipe->m_bIsAccepting = false;
#endif

    if (s < 0)
    {
        SAL_WARN("sal.osl.pipe", "accept() failed: " << UnixErrnoString(errno));
        return nullptr;
    }

#if defined(CLOSESOCKET_DOESNT_WAKE_UP_ACCEPT)
    if (pPipe->m_bIsInShutdown)
    {
        close(s);
        return nullptr;
    }
#endif /* CLOSESOCKET_DOESNT_WAKE_UP_ACCEPT */

    /* alloc memory */
    pAcceptedPipe = createPipeImpl();

    assert(pAcceptedPipe);  // should never be the case that an oslPipe cannot be initialized
    if (!pAcceptedPipe)
    {
        close(s);
        return nullptr;
    }

    /* set close-on-exec flag */
    int flags;
    if ((flags = fcntl(s, F_GETFD, 0)) >= 0)
    {
        flags |= FD_CLOEXEC;
        if (fcntl(s, F_SETFD, flags) < 0)
            SAL_WARN("sal.osl.pipe", "fcntl() failed: " <<  UnixErrnoString(errno));
    }

    pAcceptedPipe->m_Socket = s;

    return pAcceptedPipe;
}

sal_Int32 SAL_CALL osl_receivePipe(oslPipe pPipe,
                        void* pBuffer,
                        sal_Int32 BytesToRead)
{
    int nRet = 0;

    SAL_WARN_IF(!pPipe, "sal.osl.pipe", "osl_receivePipe: invalid pipe");
    if (!pPipe)
    {
        SAL_WARN("sal.osl.pipe", "osl_receivePipe: Invalid socket");
        errno=EINVAL;
        return -1;
    }

    nRet = recv(pPipe->m_Socket, pBuffer, BytesToRead, 0);

    if (nRet < 0)
        SAL_WARN("sal.osl.pipe", "recv() failed: " << UnixErrnoString(errno));

    return nRet;
}

sal_Int32 SAL_CALL osl_sendPipe(oslPipe pPipe,
                       const void* pBuffer,
                       sal_Int32 BytesToSend)
{
    int nRet=0;

    SAL_WARN_IF(!pPipe, "sal.osl.pipe", "osl_sendPipe: invalid pipe");
    if (!pPipe)
    {
        SAL_WARN("sal.osl.pipe", "osl_sendPipe: Invalid socket");
        errno=EINVAL;
        return -1;
    }

    nRet = send(pPipe->m_Socket, pBuffer, BytesToSend, 0);

    if (nRet <= 0)
        SAL_WARN("sal.osl.pipe", "send() failed: " << UnixErrnoString(errno));

    return nRet;
}

oslPipeError SAL_CALL osl_getLastPipeError(SAL_UNUSED_PARAMETER oslPipe)
{
    return osl_PipeErrorFromNative(errno);
}

sal_Int32 SAL_CALL osl_writePipe(oslPipe pPipe, const void *pBuffer, sal_Int32 n)
{
    /* loop until all desired bytes were send or an error occurred */
    sal_Int32 BytesSend = 0;
    sal_Int32 BytesToSend = n;

    SAL_WARN_IF(!pPipe, "sal.osl.pipe", "osl_writePipe: invalid pipe"); // osl_sendPipe detects invalid pipe
    while (BytesToSend > 0)
    {
        sal_Int32 RetVal;

        RetVal= osl_sendPipe(pPipe, pBuffer, BytesToSend);

        /* error occurred? */
        if (RetVal <= 0)
            break;

        BytesToSend -= RetVal;
        BytesSend += RetVal;
        pBuffer= static_cast< sal_Char const* >(pBuffer) + RetVal;
    }

    return BytesSend;
}

sal_Int32 SAL_CALL osl_readPipe( oslPipe pPipe, void *pBuffer , sal_Int32 n )
{
    /* loop until all desired bytes were read or an error occurred */
    sal_Int32 BytesRead = 0;
    sal_Int32 BytesToRead = n;

    SAL_WARN_IF(!pPipe, "sal.osl.pipe", "osl_readPipe: invalid pipe"); // osl_receivePipe detects invalid pipe
    while (BytesToRead > 0)
    {
        sal_Int32 RetVal;
        RetVal= osl_receivePipe(pPipe, pBuffer, BytesToRead);

        /* error occurred? */
        if (RetVal <= 0)
            break;

        BytesToRead -= RetVal;
        BytesRead += RetVal;
        pBuffer= static_cast< sal_Char* >(pBuffer) + RetVal;
    }

    return BytesRead;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
