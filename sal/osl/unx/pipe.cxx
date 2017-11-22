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
#include <rtl/bootstrap.h>
#include <sal/log.hxx>

#include "sockimpl.hxx"
#include "secimpl.hxx"

#include <cassert>

#define PIPEDEFAULTPATH     "/tmp"
#define PIPEALTERNATEPATH   "/var/tmp"

#define PIPENAMEMASK        "OSL_PIPE_%s"
#define SECPIPENAMEMASK     "OSL_PIPE_%s_%s"

oslPipe SAL_CALL osl_psz_createPipe(const sal_Char *pszPipeName, oslPipeOptions Options, oslSecurity Security);

static struct
{
    int            errcode;
    oslPipeError   error;
} PipeError[]= {
    { 0,               osl_Pipe_E_None              },  /* no error */
    { EPROTOTYPE,      osl_Pipe_E_NoProtocol        },  /* Protocol wrong type for socket */
    { ENOPROTOOPT,     osl_Pipe_E_NoProtocol        },  /* Protocol not available */
    { EPROTONOSUPPORT, osl_Pipe_E_NoProtocol        },  /* Protocol not supported */
    { ESOCKTNOSUPPORT, osl_Pipe_E_NoProtocol        },  /* Socket type not supported */
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

static bool
cpyBootstrapSocketPath(sal_Char *name, size_t len)
{
    bool bRet = false;
    rtl_uString *pName = nullptr, *pValue = nullptr;

    rtl_uString_newFromAscii(&pName, "OSL_SOCKET_PATH");

    if (rtl_bootstrap_get(pName, &pValue, nullptr))
    {
        if (pValue && pValue->length > 0)
        {
            rtl_String *pStrValue = nullptr;

            rtl_uString2String(&pStrValue, pValue->buffer,
                               pValue->length, RTL_TEXTENCODING_UTF8,
                               OUSTRING_TO_OSTRING_CVTFLAGS);
            if (pStrValue)
            {
                if (pStrValue->length > 0)
                {
                    size_t nCopy = (len-1 < (size_t)pStrValue->length) ? len-1 : (size_t)pStrValue->length;
                    strncpy (name, pStrValue->buffer, nCopy);
                    name[nCopy] = '\0';
                    bRet = (size_t)pStrValue->length < len;
                }
                rtl_string_release(pStrValue);
            }
        }
        rtl_uString_release(pName);
    }
    return bRet;
}

oslPipe SAL_CALL osl_psz_createPipe(const sal_Char *pszPipeName, oslPipeOptions Options,
                                    oslSecurity Security)
{
    int Flags;
    size_t len;
    struct sockaddr_un addr;

    sal_Char name[PATH_MAX+1];
    size_t nNameLength = 0;
    bool bNameTooLong = false;
    oslPipe pPipe;

    if (access(PIPEDEFAULTPATH, W_OK) == 0)
        strncpy(name, PIPEDEFAULTPATH, sizeof(name));
    else if (access(PIPEALTERNATEPATH, W_OK) == 0)
        strncpy(name, PIPEALTERNATEPATH, sizeof(name));
    else if (!cpyBootstrapSocketPath (name, sizeof (name)))
        return nullptr;

    name[sizeof(name)-1] = '\0';  // ensure the string is NULL-terminated
    nNameLength = strlen(name);
    bNameTooLong = nNameLength > sizeof(name) - 2;

    if (!bNameTooLong)
    {
        size_t nRealLength = 0;

        strcat(name, "/");
        ++nNameLength;

        if (Security)
        {
            sal_Char Ident[256];

            Ident[0] = '\0';

            OSL_VERIFY(osl_psz_getUserIdent(Security, Ident, sizeof(Ident)));

            nRealLength = snprintf(&name[nNameLength], sizeof(name) - nNameLength, SECPIPENAMEMASK, Ident, pszPipeName);
        }
        else
        {
            nRealLength = snprintf(&name[nNameLength], sizeof(name) - nNameLength, PIPENAMEMASK, pszPipeName);
        }

        bNameTooLong = nRealLength > sizeof(name) - nNameLength - 1;
    }

    if (bNameTooLong)
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
        SAL_WARN("sal.osl.pipe", "socket() failed: " << strerror(errno));
        destroyPipeImpl(pPipe);
        return nullptr;
    }

    /* set close-on-exec flag */
    if ((Flags = fcntl(pPipe->m_Socket, F_GETFD, 0)) != -1)
    {
        Flags |= FD_CLOEXEC;
        if (fcntl(pPipe->m_Socket, F_SETFD, Flags) == -1)
        {
            SAL_WARN("sal.osl.pipe", "fcntl() failed: " << strerror(errno));
        }
    }

    memset(&addr, 0, sizeof(addr));

    SAL_INFO("sal.osl.pipe", "new pipe on fd " << pPipe->m_Socket << " '" << name << "'");

    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, name, sizeof(addr.sun_path) - 1);
#if defined(FREEBSD)
    len = SUN_LEN(&addr);
#else
    len = sizeof(addr);
#endif

    if (Options & osl_Pipe_CREATE)
    {
        struct stat status;

        /* check if there exists an orphan filesystem entry */
        if ((stat(name, &status) == 0) &&
            (S_ISSOCK(status.st_mode) || S_ISFIFO(status.st_mode)))
        {
            if (connect(pPipe->m_Socket, reinterpret_cast< sockaddr* >(&addr), len) >= 0)
            {
                close (pPipe->m_Socket);
                destroyPipeImpl(pPipe);
                return nullptr;
            }

            unlink(name);
        }

        /* ok, fs clean */
        if (bind(pPipe->m_Socket, reinterpret_cast< sockaddr* >(&addr), len) < 0)
        {
            SAL_WARN("sal.osl.pipe", "bind() failed: " << strerror(errno));
            close(pPipe->m_Socket);
            destroyPipeImpl(pPipe);
            return nullptr;
        }

        /*  Only give access to all if no security handle was specified, otherwise security
            depends on umask */

        if (!Security)
            chmod(name,S_IRWXU | S_IRWXG |S_IRWXO);

        strncpy(pPipe->m_Name, name, sizeof(pPipe->m_Name) - 1);

        if (listen(pPipe->m_Socket, 5) < 0)
        {
            SAL_WARN("sal.osl.pipe", "listen() failed: " << strerror(errno));
            // coverity[toctou] cid#1255391 warns about unlink(name) after
            // stat(name, &status) above, but the intervening call to bind makes
            // those two clearly unrelated, as it would fail if name existed at
            // that point in time:
            unlink(name);   /* remove filesystem entry */
            close(pPipe->m_Socket);
            destroyPipeImpl(pPipe);
            return nullptr;
        }

        return pPipe;
    }

    /* osl_pipe_OPEN */
    if (access(name, F_OK) != -1)
    {
        if (connect(pPipe->m_Socket, reinterpret_cast< sockaddr* >(&addr), len) >= 0)
            return pPipe;

        SAL_WARN("sal.osl.pipe", "connect() failed: " << strerror(errno));
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
            SAL_WARN("sal.osl.pipe", "socket() failed: " << strerror(errno));
            return;
        }

        memset(&addr, 0, sizeof(addr));

        SAL_INFO("sal.osl.pipe", "osl_destroyPipe : Pipe Name '" << pPipe->m_Name << "'");

        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, pPipe->m_Name, sizeof(addr.sun_path) - 1);

        nRet = connect(fd, reinterpret_cast< sockaddr* >(&addr), sizeof(addr));
        if (nRet < 0)
            SAL_WARN("sal.osl.pipe", "connect() failed: " << strerror(errno));

        close(fd);
    }
#endif /* CLOSESOCKET_DOESNT_WAKE_UP_ACCEPT */

    nRet = shutdown(ConnFD, 2);
    if (nRet < 0)
        SAL_WARN("sal.osl.pipe", "shutdown() failed: " << strerror(errno));

    nRet = close(ConnFD);
    if (nRet < 0)
        SAL_WARN("sal.osl.pipe", "close() failed: " << strerror(errno));

    /* remove filesystem entry */
    if (strlen(pPipe->m_Name) > 0)
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

    assert(strlen(pPipe->m_Name) > 0);  // you cannot have an empty pipe name

#if defined(CLOSESOCKET_DOESNT_WAKE_UP_ACCEPT)
    pPipe->m_bIsAccepting = true;
#endif

    s = accept(pPipe->m_Socket, nullptr, nullptr);

#if defined(CLOSESOCKET_DOESNT_WAKE_UP_ACCEPT)
    pPipe->m_bIsAccepting = false;
#endif

    if (s < 0)
    {
        SAL_WARN("sal.osl.pipe", "accept() failed: " << strerror(errno));
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
            SAL_WARN("sal.osl.pipe", "fcntl() failed: " <<  strerror(errno));
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
        SAL_WARN("sal.osl.pipe", "recv() failed: " << strerror(errno));

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
        SAL_WARN("sal.osl.pipe", "send() failed: " << strerror(errno));

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
