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

#include <string.h>

#include "system.h"

#include <osl/pipe.h>
#include <osl/diagnose.h>
#include <osl/thread.h>
#include <osl/mutex.h>
#include <osl/semaphor.h>
#include <osl/conditn.h>
#include <osl/interlck.h>
#include <osl/process.h>

#include <rtl/alloc.h>
#include <rtl/memory.h>

#define PIPESYSTEM      "\\\\.\\pipe\\"
#define PIPEPREFIX    "OSL_PIPE_"

typedef struct
{
    sal_uInt32 m_Size;
    sal_uInt32 m_ReadPos;
    sal_uInt32 m_WritePos;
    BYTE   m_Data[1];

} oslPipeBuffer;

/*****************************************************************************/
/* oslPipeImpl */
/*****************************************************************************/

struct oslPipeImpl {
    oslInterlockedCount  m_Reference;
    HANDLE               m_File;
    HANDLE               m_NamedObject;
    PSECURITY_ATTRIBUTES m_Security;
    HANDLE               m_ReadEvent;
    HANDLE               m_WriteEvent;
    HANDLE               m_AcceptEvent;
    rtl_uString*         m_Name;
    oslPipeError         m_Error;
    sal_Bool             m_bClosed;
};


/*****************************************************************************/
/* osl_create/destroy-PipeImpl */
/*****************************************************************************/

oslPipe __osl_createPipeImpl(void)
{
    oslPipe pPipe;

    pPipe = (oslPipe) rtl_allocateZeroMemory(sizeof(struct oslPipeImpl));

    pPipe->m_bClosed = sal_False;
    pPipe->m_Reference = 0;
    pPipe->m_Name = NULL;
    pPipe->m_File = INVALID_HANDLE_VALUE;
    pPipe->m_NamedObject = INVALID_HANDLE_VALUE;

    pPipe->m_ReadEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    pPipe->m_WriteEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    pPipe->m_AcceptEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    return pPipe;
}

void __osl_destroyPipeImpl(oslPipe pPipe)
{
    if (pPipe != NULL)
    {
        if ( pPipe->m_NamedObject != INVALID_HANDLE_VALUE && pPipe->m_NamedObject != NULL )
            CloseHandle( pPipe->m_NamedObject );

        if (pPipe->m_Security != NULL)
        {
            rtl_freeMemory(pPipe->m_Security->lpSecurityDescriptor);
            rtl_freeMemory(pPipe->m_Security);
        }

        CloseHandle(pPipe->m_ReadEvent);
        CloseHandle(pPipe->m_WriteEvent);
        CloseHandle(pPipe->m_AcceptEvent);

        if (pPipe->m_Name)
            rtl_uString_release(pPipe->m_Name);

        rtl_freeMemory(pPipe);
    }
}



/*****************************************************************************/
/* osl_createPipe  */
/*****************************************************************************/
oslPipe SAL_CALL osl_createPipe(rtl_uString *strPipeName, oslPipeOptions Options,
                       oslSecurity Security)
{
    rtl_uString* name = NULL;
    rtl_uString* path = NULL;
    rtl_uString* temp = NULL;
    oslPipe pPipe;

       PSECURITY_ATTRIBUTES  pSecAttr = NULL;

    rtl_uString_newFromAscii(&path, PIPESYSTEM);
    rtl_uString_newFromAscii(&name, PIPEPREFIX);

    if ( Security)
    {
        rtl_uString *Ident = NULL;
        rtl_uString *Delim = NULL;

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

            pSecDesc = (PSECURITY_DESCRIPTOR) rtl_allocateMemory(SECURITY_DESCRIPTOR_MIN_LENGTH);

            /* add a NULL disc. ACL to the security descriptor */
            OSL_VERIFY(InitializeSecurityDescriptor(pSecDesc, SECURITY_DESCRIPTOR_REVISION));
            OSL_VERIFY(SetSecurityDescriptorDacl(pSecDesc, TRUE, (PACL) NULL, FALSE));

            pSecAttr = rtl_allocateMemory(sizeof(SECURITY_ATTRIBUTES));
            pSecAttr->nLength = sizeof(SECURITY_ATTRIBUTES);
            pSecAttr->lpSecurityDescriptor = pSecDesc;
            pSecAttr->bInheritHandle = TRUE;
        }
    }

    rtl_uString_assign(&temp, name);
    rtl_uString_newConcat(&name, temp, strPipeName);

    /* alloc memory */
    pPipe= __osl_createPipeImpl();
    osl_atomic_increment(&(pPipe->m_Reference));

    /* build system pipe name */
    rtl_uString_assign(&temp, path);
    rtl_uString_newConcat(&path, temp, name);
    rtl_uString_release(temp);
    temp = NULL;

    if (Options & osl_Pipe_CREATE)
    {
        SetLastError( ERROR_SUCCESS );

        pPipe->m_NamedObject = CreateMutexW( NULL, FALSE, name->buffer );

        if ( pPipe->m_NamedObject != INVALID_HANDLE_VALUE && pPipe->m_NamedObject != NULL )
        {
            if ( GetLastError() != ERROR_ALREADY_EXISTS )
            {
                pPipe->m_Security = pSecAttr;
                rtl_uString_assign(&pPipe->m_Name, name);

                /* try to open system pipe */
                pPipe->m_File = CreateNamedPipeW(
                    path->buffer,
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
                CloseHandle( pPipe->m_NamedObject );
                pPipe->m_NamedObject = INVALID_HANDLE_VALUE;
            }
        }
    }
    else
    {
        BOOL    fPipeAvailable;

        do
        {
            /* free instance should be available first */
            fPipeAvailable = WaitNamedPipeW(path->buffer, NMPWAIT_WAIT_FOREVER);

            /* first try to open system pipe */
            if ( fPipeAvailable )
            {
                pPipe->m_File = CreateFileW(
                    path->buffer,
                    GENERIC_READ|GENERIC_WRITE,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                    NULL);

                if ( pPipe->m_File != INVALID_HANDLE_VALUE )
                {
                    // We got it !
                    rtl_uString_release( name );
                    rtl_uString_release( path );

                    return (pPipe);
                }
                else
                {
                    // Pipe instance maybe catched by another client -> try again
                }
            }
        } while ( fPipeAvailable );
    }

    /* if we reach here something went wrong */
    __osl_destroyPipeImpl(pPipe);

    return NULL;
}

void SAL_CALL osl_acquirePipe( oslPipe pPipe )
{
    osl_atomic_increment( &(pPipe->m_Reference) );
}

void SAL_CALL osl_releasePipe( oslPipe pPipe )
{
//      OSL_ASSERT( pPipe );

    if( 0 == pPipe )
        return;

    if( 0 == osl_atomic_decrement( &(pPipe->m_Reference) ) )
    {
        if( ! pPipe->m_bClosed )
            osl_closePipe( pPipe );

        __osl_destroyPipeImpl( pPipe );
    }
}

void SAL_CALL osl_closePipe( oslPipe pPipe )
{
    if( pPipe && ! pPipe->m_bClosed )
    {
        pPipe->m_bClosed = sal_True;
        /* if we have a system pipe close it */
        if (pPipe->m_File != INVALID_HANDLE_VALUE)
        {
            /*          FlushFileBuffers(pPipe->m_File); */
            DisconnectNamedPipe(pPipe->m_File);
            CloseHandle(pPipe->m_File);
        }
    }
}

/*****************************************************************************/
/* osl_acceptPipe  */
/*****************************************************************************/
oslPipe SAL_CALL osl_acceptPipe(oslPipe pPipe)
{
    oslPipe  pAcceptedPipe = NULL;

    OVERLAPPED   os;

    DWORD nBytesTransfered;
    rtl_uString* path = NULL;
    rtl_uString* temp = NULL;

    OSL_ASSERT(pPipe);
    OSL_ASSERT(pPipe->m_File != INVALID_HANDLE_VALUE);

    memset(&os, 0, sizeof(OVERLAPPED));
    os.hEvent = pPipe->m_AcceptEvent;
    ResetEvent(pPipe->m_AcceptEvent);

    if ( !ConnectNamedPipe(pPipe->m_File, &os))
    {
        switch ( GetLastError() )
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
                if( !GetOverlappedResult( pPipe->m_File, &os, &nBytesTransfered, TRUE ) )
                {
                    // Possible error could be that between ConnectNamedPipe and GetOverlappedResult a connect
                    // took place.

                    switch ( GetLastError() )
                    {
                    case ERROR_PIPE_CONNECTED:  // Pipe was already connected
                    case ERROR_NO_DATA:         // Pipe was connected but client has already closed -> ver fast client ;-)
                        break;                  // Everything's fine !!!
                    default:
                        // Something went wrong
                        return 0;
                    }
                }
                break;
            default:                    // All other error say that somethings going wrong.
                return 0;
        }
    }

    pAcceptedPipe = __osl_createPipeImpl();
    OSL_ASSERT(pAcceptedPipe);

    osl_atomic_increment(&(pAcceptedPipe->m_Reference));
    rtl_uString_assign(&pAcceptedPipe->m_Name, pPipe->m_Name);
    pAcceptedPipe->m_File = pPipe->m_File;

    rtl_uString_newFromAscii(&temp, PIPESYSTEM);
    rtl_uString_newConcat(&path, temp, pPipe->m_Name);
    rtl_uString_release(temp);

    // prepare for next accept
    pPipe->m_File =
        CreateNamedPipeW(path->buffer,
            PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
            PIPE_WAIT | PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE,
            PIPE_UNLIMITED_INSTANCES,
            4096, 4096,
            NMPWAIT_WAIT_FOREVER,
            pAcceptedPipe->m_Security);
    rtl_uString_release( path );

    return pAcceptedPipe;
}

/*****************************************************************************/
/* osl_receivePipe  */
/*****************************************************************************/
sal_Int32 SAL_CALL osl_receivePipe(oslPipe pPipe,
                        void* pBuffer,
                        sal_Int32 BytesToRead)
{
    DWORD nBytes;
    OVERLAPPED   os;

    OSL_ASSERT(pPipe);

    memset(&os, 0, sizeof(OVERLAPPED));
    os.hEvent = pPipe->m_ReadEvent;

    ResetEvent(pPipe->m_ReadEvent);

    if (! ReadFile(pPipe->m_File, pBuffer, BytesToRead, &nBytes, &os) &&
        ((GetLastError() != ERROR_IO_PENDING) ||
         ! GetOverlappedResult(pPipe->m_File, &os, &nBytes, TRUE)))
    {
        DWORD lastError = GetLastError();

        if (lastError == ERROR_MORE_DATA)
            nBytes = BytesToRead;
          else
          {
              if (lastError == ERROR_PIPE_NOT_CONNECTED)
                nBytes = 0;
            else
                nBytes = (DWORD) -1;

             pPipe->m_Error = osl_Pipe_E_ConnectionAbort;
        }
    }

    return (nBytes);
}

/*****************************************************************************/
/* osl_sendPipe  */
/*****************************************************************************/
sal_Int32 SAL_CALL osl_sendPipe(oslPipe pPipe,
                       const void* pBuffer,
                       sal_Int32 BytesToSend)
{
    DWORD nBytes;
    OVERLAPPED   os;

    OSL_ASSERT(pPipe);

    memset(&os, 0, sizeof(OVERLAPPED));
    os.hEvent = pPipe->m_WriteEvent;
    ResetEvent(pPipe->m_WriteEvent);

    if (! WriteFile(pPipe->m_File, pBuffer, BytesToSend, &nBytes, &os) &&
        ((GetLastError() != ERROR_IO_PENDING) ||
          ! GetOverlappedResult(pPipe->m_File, &os, &nBytes, TRUE)))
    {
          if (GetLastError() == ERROR_PIPE_NOT_CONNECTED)
            nBytes = 0;
        else
            nBytes = (DWORD) -1;

         pPipe->m_Error = osl_Pipe_E_ConnectionAbort;
    }

    return (nBytes);
}

sal_Int32 SAL_CALL osl_writePipe( oslPipe pPipe, const void *pBuffer , sal_Int32 n )
{
    /* loop until all desired bytes were send or an error occurred */
    sal_Int32 BytesSend= 0;
    sal_Int32 BytesToSend= n;

    OSL_ASSERT(pPipe);
    while (BytesToSend > 0)
    {
        sal_Int32 RetVal;

        RetVal= osl_sendPipe(pPipe, pBuffer, BytesToSend);

        /* error occurred? */
        if(RetVal <= 0)
        {
            break;
        }

        BytesToSend -= RetVal;
        BytesSend += RetVal;
        pBuffer= (sal_Char*)pBuffer + RetVal;
    }

    return BytesSend;
}

sal_Int32 SAL_CALL osl_readPipe( oslPipe pPipe, void *pBuffer , sal_Int32 n )
{
    /* loop until all desired bytes were read or an error occurred */
    sal_Int32 BytesRead= 0;
    sal_Int32 BytesToRead= n;

    OSL_ASSERT( pPipe );
    while (BytesToRead > 0)
    {
        sal_Int32 RetVal;
        RetVal= osl_receivePipe(pPipe, pBuffer, BytesToRead);

        /* error occurred? */
        if(RetVal <= 0)
        {
            break;
        }

        BytesToRead -= RetVal;
        BytesRead += RetVal;
        pBuffer= (sal_Char*)pBuffer + RetVal;
    }
    return BytesRead;
}


/*****************************************************************************/
/* osl_getLastPipeError  */
/*****************************************************************************/
oslPipeError SAL_CALL osl_getLastPipeError(oslPipe pPipe)
{
    oslPipeError Error;

    if (pPipe != NULL)
    {
        Error = pPipe->m_Error;
        pPipe->m_Error = osl_Pipe_E_None;
    }
    else
        Error = osl_Pipe_E_NotFound;

    return (Error);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
