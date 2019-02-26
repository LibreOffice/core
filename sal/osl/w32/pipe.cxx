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

#include "system.h"

#include <osl/pipe.h>
#include <osl/diagnose.h>
#include <osl/thread.h>
#include <osl/mutex.h>
#include <osl/conditn.h>
#include <osl/interlck.h>
#include <osl/process.h>
#include <rtl/alloc.h>
#include <sal/log.hxx>
#include <o3tl/char16_t2wchar_t.hxx>

#include <cassert>
#include <string.h>

#define PIPESYSTEM      "\\\\.\\pipe\\"
#define PIPEPREFIX      "OSL_PIPE_"

typedef struct
{
    sal_uInt32           m_Size;
    sal_uInt32           m_ReadPos;
    sal_uInt32           m_WritePos;
    BYTE                 m_Data[1];

} oslPipeBuffer;

struct oslPipeImpl
{
    oslInterlockedCount  m_Reference;
    HANDLE               m_File;
    HANDLE               m_NamedObject;
    PSECURITY_ATTRIBUTES m_Security;
    HANDLE               m_ReadEvent;
    HANDLE               m_WriteEvent;
    HANDLE               m_AcceptEvent;
    rtl_uString*         m_Name;
    oslPipeError         m_Error;
    bool                 m_bClosed;
};

static oslPipe osl_createPipeImpl(void)
{
    oslPipe pPipe;

    pPipe = static_cast< oslPipe >(rtl_allocateZeroMemory(sizeof(struct oslPipeImpl)));

    pPipe->m_bClosed = false;
    pPipe->m_Reference = 0;
    pPipe->m_Name = nullptr;
    pPipe->m_File = INVALID_HANDLE_VALUE;
    pPipe->m_NamedObject = nullptr;

    pPipe->m_ReadEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    pPipe->m_WriteEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    pPipe->m_AcceptEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);

    return pPipe;
}

static void osl_destroyPipeImpl(oslPipe pPipe)
{
    if (pPipe)
    {
        if (pPipe->m_NamedObject)
            CloseHandle(pPipe->m_NamedObject);

        if (pPipe->m_Security)
        {
            free(pPipe->m_Security->lpSecurityDescriptor);
            free(pPipe->m_Security);
        }

        CloseHandle(pPipe->m_ReadEvent);
        CloseHandle(pPipe->m_WriteEvent);
        CloseHandle(pPipe->m_AcceptEvent);

        if (pPipe->m_Name)
            rtl_uString_release(pPipe->m_Name);

        free(pPipe);
    }
}

oslPipe SAL_CALL osl_createPipe(rtl_uString *strPipeName, oslPipeOptions Options,
                       oslSecurity Security)
{
    rtl_uString* name = nullptr;
    rtl_uString* path = nullptr;
    rtl_uString* temp = nullptr;
    oslPipe pPipe;

    PSECURITY_ATTRIBUTES pSecAttr = nullptr;

    rtl_uString_newFromAscii(&path, PIPESYSTEM);
    rtl_uString_newFromAscii(&name, PIPEPREFIX);

    if (Security)
    {
        rtl_uString *Ident = nullptr;
        rtl_uString *Delim = nullptr;

        OSL_VERIFY(osl_getUserIdent(Security, &Ident));
        rtl_uString_newFromAscii(&Delim, "_");

        rtl_uString_newConcat(&temp, name, Ident);
        rtl_uString_newConcat(&name, temp, Delim);

        rtl_uString_release(Ident);
        rtl_uString_release(Delim);
    }
    else
    {
        if (Options & osl_Pipe_CREATE)
        {
            PSECURITY_DESCRIPTOR pSecDesc;

            pSecDesc = static_cast< PSECURITY_DESCRIPTOR >(malloc(SECURITY_DESCRIPTOR_MIN_LENGTH));

            /* add a NULL disc. ACL to the security descriptor */
            OSL_VERIFY(InitializeSecurityDescriptor(pSecDesc, SECURITY_DESCRIPTOR_REVISION));
            OSL_VERIFY(SetSecurityDescriptorDacl(pSecDesc, TRUE, nullptr, FALSE));

            pSecAttr = static_cast< PSECURITY_ATTRIBUTES >(malloc(sizeof(SECURITY_ATTRIBUTES)));
            pSecAttr->nLength = sizeof(SECURITY_ATTRIBUTES);
            pSecAttr->lpSecurityDescriptor = pSecDesc;
            pSecAttr->bInheritHandle = TRUE;
        }
    }

    rtl_uString_assign(&temp, name);
    rtl_uString_newConcat(&name, temp, strPipeName);

    /* alloc memory */
    pPipe = osl_createPipeImpl();

    assert(pPipe);  // if osl_createPipeImpl() cannot init. a new pipe, this is a failure

    osl_atomic_increment(&(pPipe->m_Reference));

    /* build system pipe name */
    rtl_uString_assign(&temp, path);
    rtl_uString_newConcat(&path, temp, name);
    rtl_uString_release(temp);
    temp = nullptr;

    if (Options & osl_Pipe_CREATE)
    {
        SetLastError(ERROR_SUCCESS);

        pPipe->m_NamedObject = CreateMutexW(nullptr, FALSE, o3tl::toW(name->buffer));

        if (pPipe->m_NamedObject)
        {
            if (GetLastError() != ERROR_ALREADY_EXISTS)
            {
                pPipe->m_Security = pSecAttr;
                rtl_uString_assign(&pPipe->m_Name, name);

                /* try to open system pipe */
                pPipe->m_File = CreateNamedPipeW(
                    o3tl::toW(path->buffer),
                    PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
                    PIPE_WAIT | PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE,
                    PIPE_UNLIMITED_INSTANCES,
                    4096, 4096,
                    NMPWAIT_WAIT_FOREVER,
                    pPipe->m_Security);

                if (pPipe->m_File != INVALID_HANDLE_VALUE)
                {
                    rtl_uString_release( name );
                    rtl_uString_release( path );

                    return pPipe;
                }
            }
            else
            {
                CloseHandle(pPipe->m_NamedObject);
                pPipe->m_NamedObject = nullptr;
            }
        }
    }
    else
    {
        BOOL bPipeAvailable;

        do
        {
            /* free instance should be available first */
            bPipeAvailable = WaitNamedPipeW(o3tl::toW(path->buffer), NMPWAIT_WAIT_FOREVER);

            /* first try to open system pipe */
            if (bPipeAvailable)
            {
                pPipe->m_File = CreateFileW(
                    o3tl::toW(path->buffer),
                    GENERIC_READ|GENERIC_WRITE,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    nullptr,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                    nullptr);

                if (pPipe->m_File != INVALID_HANDLE_VALUE)
                {
                    // We got it !
                    rtl_uString_release(name);
                    rtl_uString_release(path);

                    // We should try to transfer our privilege to become foreground process
                    // to the other process, so that it may show popups (otherwise, they might
                    // be blocked by SPI_GETFOREGROUNDLOCKTIMEOUT setting -
                    // see SystemParametersInfo function at MSDN
                    ULONG ServerProcessId = 0;
                    if (GetNamedPipeServerProcessId(pPipe->m_File, &ServerProcessId))
                        AllowSetForegroundWindow(ServerProcessId);

                    return pPipe;
                }
                else
                {
                    // Pipe instance maybe caught by another client -> try again
                }
            }
        } while (bPipeAvailable);
    }

    /* if we reach here something went wrong */
    osl_destroyPipeImpl(pPipe);

    return nullptr;
}

void SAL_CALL osl_acquirePipe(oslPipe pPipe)
{
    osl_atomic_increment(&(pPipe->m_Reference));
}

void SAL_CALL osl_releasePipe(oslPipe pPipe)
{
    if (!pPipe)
        return;

    if (osl_atomic_decrement(&(pPipe->m_Reference)) == 0)
    {
        if (!pPipe->m_bClosed)
            osl_closePipe(pPipe);

        osl_destroyPipeImpl(pPipe);
    }
}

void SAL_CALL osl_closePipe(oslPipe pPipe)
{
    if (pPipe && !pPipe->m_bClosed)
    {
        pPipe->m_bClosed = true;
        /* if we have a system pipe close it */
        if (pPipe->m_File != INVALID_HANDLE_VALUE)
        {
            DisconnectNamedPipe(pPipe->m_File);
            CloseHandle(pPipe->m_File);
        }
    }
}

oslPipe SAL_CALL osl_acceptPipe(oslPipe pPipe)
{
    oslPipe pAcceptedPipe = nullptr;

    OVERLAPPED os;

    DWORD nBytesTransfered;
    rtl_uString* path = nullptr;
    rtl_uString* temp = nullptr;

    SAL_WARN_IF(!pPipe, "sal.osl.pipe", "osl_acceptPipe: invalid pipe");
    if (!pPipe)
        return nullptr;

    SAL_WARN_IF(pPipe->m_File == INVALID_HANDLE_VALUE, "sal.osl.pipe", "osl_acceptPipe: invalid handle");

    memset(&os, 0, sizeof(OVERLAPPED));
    os.hEvent = pPipe->m_AcceptEvent;
    ResetEvent(pPipe->m_AcceptEvent);

    if (!ConnectNamedPipe(pPipe->m_File, &os))
    {
        switch (GetLastError())
        {
            case ERROR_PIPE_CONNECTED:  // Client already connected to pipe
            case ERROR_NO_DATA:         // Client was connected but has already closed pipe end
                                        // should only appear in nonblocking mode but in fact does
                                        // in blocking asynchronous mode.
                break;
            case ERROR_PIPE_LISTENING:  // Only for nonblocking mode but see ERROR_NO_DATA
            case ERROR_IO_PENDING:      // This is normal if not client is connected yet
            case ERROR_MORE_DATA:       // Should not happen
                // blocking call to accept
                if( !GetOverlappedResult(pPipe->m_File, &os, &nBytesTransfered, TRUE))
                {
                    // Possible error could be that between ConnectNamedPipe and
                    // GetOverlappedResult a connect took place.

                    switch (GetLastError())
                    {
                    case ERROR_PIPE_CONNECTED:  // Pipe was already connected
                    case ERROR_NO_DATA:         // Pipe was connected but client has already closed -> ver fast client ;-)
                        break;                  // Everything's fine !!!
                    default:
                        // Something went wrong
                        return nullptr;
                    }
                }
                break;
            default:                    // All other error say that somethings going wrong.
                return nullptr;
        }
    }

    pAcceptedPipe = osl_createPipeImpl();
    assert(pAcceptedPipe);  // should never be the case that an oslPipe cannot be initialized

    osl_atomic_increment(&(pAcceptedPipe->m_Reference));
    rtl_uString_assign(&pAcceptedPipe->m_Name, pPipe->m_Name);
    pAcceptedPipe->m_File = pPipe->m_File;

    rtl_uString_newFromAscii(&temp, PIPESYSTEM);
    rtl_uString_newConcat(&path, temp, pPipe->m_Name);
    rtl_uString_release(temp);

    // prepare for next accept
    pPipe->m_File =
        CreateNamedPipeW(o3tl::toW(path->buffer),
            PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
            PIPE_WAIT | PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE,
            PIPE_UNLIMITED_INSTANCES,
            4096, 4096,
            NMPWAIT_WAIT_FOREVER,
            pAcceptedPipe->m_Security);
    rtl_uString_release(path);

    return pAcceptedPipe;
}

sal_Int32 SAL_CALL osl_receivePipe(oslPipe pPipe,
                        void* pBuffer,
                        sal_Int32 BytesToRead)
{
    DWORD nBytes;
    OVERLAPPED os;

    assert(pPipe);

    memset(&os, 0, sizeof(OVERLAPPED));
    os.hEvent = pPipe->m_ReadEvent;

    ResetEvent(pPipe->m_ReadEvent);

    if (!ReadFile(pPipe->m_File, pBuffer, BytesToRead, &nBytes, &os) &&
        ((GetLastError() != ERROR_IO_PENDING) ||
         !GetOverlappedResult(pPipe->m_File, &os, &nBytes, TRUE)))
    {
        DWORD lastError = GetLastError();

        if (lastError == ERROR_MORE_DATA)
        {
            nBytes = BytesToRead;
        }
        else
        {
            if (lastError == ERROR_PIPE_NOT_CONNECTED)
                nBytes = 0;
            else
                nBytes = DWORD(-1);

            pPipe->m_Error = osl_Pipe_E_ConnectionAbort;
        }
    }

    return nBytes;
}

sal_Int32 SAL_CALL osl_sendPipe(oslPipe pPipe,
                       const void* pBuffer,
                       sal_Int32 BytesToSend)
{
    DWORD nBytes;
    OVERLAPPED os;

    assert(pPipe);

    memset(&os, 0, sizeof(OVERLAPPED));
    os.hEvent = pPipe->m_WriteEvent;
    ResetEvent(pPipe->m_WriteEvent);

    if (!WriteFile(pPipe->m_File, pBuffer, BytesToSend, &nBytes, &os) &&
        ((GetLastError() != ERROR_IO_PENDING) ||
          !GetOverlappedResult(pPipe->m_File, &os, &nBytes, TRUE)))
    {
        if (GetLastError() == ERROR_PIPE_NOT_CONNECTED)
            nBytes = 0;
        else
            nBytes = DWORD(-1);

        pPipe->m_Error = osl_Pipe_E_ConnectionAbort;
    }

    return nBytes;
}

sal_Int32 SAL_CALL osl_writePipe(oslPipe pPipe, const void *pBuffer , sal_Int32 n)
{
    /* loop until all desired bytes were send or an error occurred */
    sal_Int32 BytesSend = 0;
    sal_Int32 BytesToSend = n;

    SAL_WARN_IF(!pPipe, "sal.osl.pipe", "osl_writePipe: invalid pipe");
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

sal_Int32 SAL_CALL osl_readPipe(oslPipe pPipe, void *pBuffer, sal_Int32 n)
{
    /* loop until all desired bytes were read or an error occurred */
    sal_Int32 BytesRead = 0;
    sal_Int32 BytesToRead = n;

    SAL_WARN_IF(!pPipe, "sal.osl.pipe", "osl_readPipe: invalid pipe");
    while (BytesToRead > 0)
    {
        sal_Int32 RetVal;
        RetVal= osl_receivePipe(pPipe, pBuffer, BytesToRead);

        /* error occurred? */
        if(RetVal <= 0)
            break;

        BytesToRead -= RetVal;
        BytesRead += RetVal;
        pBuffer= static_cast< sal_Char* >(pBuffer) + RetVal;
    }
    return BytesRead;
}

oslPipeError SAL_CALL osl_getLastPipeError(oslPipe pPipe)
{
    oslPipeError Error;

    if (pPipe)
    {
        Error = pPipe->m_Error;
        pPipe->m_Error = osl_Pipe_E_None;
    }
    else
    {
        Error = osl_Pipe_E_NotFound;
    }

    return Error;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
