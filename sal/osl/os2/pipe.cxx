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

//#define INCL_DOSERRORS
#include "system.h"

#include <osl/pipe.h>
#include <osl/diagnose.h>
#include <osl/thread.h>
#include <osl/mutex.h>
#include <osl/semaphor.h>
#include <osl/conditn.h>
#include <osl/interlck.h>
#include <osl/process.h>
#include <rtl/ustring.hxx>

#define PIPENAMEMASK    "OSL_PIPE_%s"
#define SECPIPENAMEMASK "OSL_PIPE_%s_%s"

typedef enum {
    MSG_SYN,
    MSG_FIN,
    MSG_DATA,
    MSG_UNKNOWN
} MessageType;

struct oslPipeImpl {
    oslInterlockedCount m_Reference;
    HPIPE               hPipe;
    HMTX                m_NamedObject;
    APIRET              nLastError;
    //oslSecurity       m_Security;
    sal_Bool            m_bClosed;
};

/* default size for input/output buffer */
static const ULONG ulBufSize = 4096;

/* OS/2 path for pipes */
static const CHAR  pszPipePath[] = "\\PIPE\\";
static const UCHAR nPipePathLen  = sizeof (pszPipePath) - 1;

/* global last error value to be returned from oslGetLastPipeError */
static APIRET ngLastError;

using rtl::OString;
using rtl::OUString;
using rtl::OUStringToOString;

/*****************************************************************************/
/* osl_create/destroy-PipeImpl */
/*****************************************************************************/

static oslInterlockedCount nPipes = 0;

oslPipe __osl_createPipeImpl(void)
{
    oslPipe pPipe;

    pPipe = (oslPipe) calloc(1,sizeof(struct oslPipeImpl));

    pPipe->m_bClosed = sal_False;
    pPipe->m_Reference = 1;
    pPipe->hPipe = NULL;
    pPipe->m_NamedObject = NULL;

    return pPipe;
}

void __osl_destroyPipeImpl(oslPipe pPipe)
{
    if (pPipe != NULL)
    {
        DosCloseMutexSem( pPipe->m_NamedObject);
        free(pPipe);
    }
}


/*****************************************************************************/
/* osl_createPipe  */
/*****************************************************************************/
oslPipe SAL_CALL osl_createPipe(rtl_uString *ustrPipeName, oslPipeOptions Options,
                       oslSecurity Security)
{
    oslPipe pPipe;

    ULONG  ulAction;
    CHAR   strPipeNameBuffer [CCHMAXPATHCOMP];
    rtl_String* strPipeName=0;
    sal_Char* pszPipeName=0;

    /* check parameters */
    OSL_ASSERT( ustrPipeName );
    //YD 17/04/06 OSL_ASSERT( Security == 0 );

    /* allocate impl-structure */
    pPipe = __osl_createPipeImpl();
    if (!pPipe)
    {
        OSL_TRACE( "osl_createPipe failed allocating memory.\n" );
        return NULL;
    }

    /* create pipe name */
    OString sPipe = OUStringToOString(ustrPipeName, RTL_TEXTENCODING_ASCII_US);
#if OSL_DEBUG_LEVEL > 0
    debug_printf("osl_createPipe options 0x%x\n", Options);
#endif

    switch( Options )
    {
    case osl_Pipe_OPEN:
        {
            APIRET  fPipeAvailable;

            sprintf (strPipeNameBuffer, "\\PIPE\\OSL_PIPE_%s", sPipe.getStr());
#if OSL_DEBUG_LEVEL > 0
            debug_printf("osl_createPipe %s\n", strPipeNameBuffer);
#endif
            ngLastError = DosOpen( (PCSZ)strPipeNameBuffer,
                                &(pPipe->hPipe), &ulAction,
                                0, FILE_NORMAL, FILE_OPEN,
                                OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYREADWRITE,
                                (PEAOP2) NULL);
            // if pipe is busy, wait for it
            if (ngLastError == ERROR_PIPE_BUSY)
                do
                {
                    /* free instance should be available first */
                    fPipeAvailable = DosWaitNPipe( (PCSZ)strPipeNameBuffer, -1);
                    /* first try to open system pipe */
                    if ( fPipeAvailable == NO_ERROR )
                    {
                        // We got it !
                        ngLastError = NO_ERROR;
                        break;
                    }
                    // Pipe instance maybe catched by another client -> try again
                    printf("osl_createPipe wait for Pipe available\n");
                } while ( fPipeAvailable );
        }
        break;
    case osl_Pipe_CREATE:
        {
            sprintf (strPipeNameBuffer, "\\SEM32\\OSL_SEM_%s", sPipe.getStr());
            // check if semaphore exists (pipe create must fail for existig pipes)
            ngLastError = DosCreateMutexSem( (PCSZ)strPipeNameBuffer, &(pPipe->m_NamedObject), 0, TRUE );
            if (ngLastError)
                break;

            sprintf (strPipeNameBuffer, "\\PIPE\\OSL_PIPE_%s", sPipe.getStr());
#if OSL_DEBUG_LEVEL > 0
            debug_printf("osl_createPipe %s\n", strPipeNameBuffer);
#endif
            ngLastError = DosCreateNPipe( (PCSZ)strPipeNameBuffer,
                            &(pPipe->hPipe),
                            NP_ACCESS_DUPLEX,    /* open pipe for read and write access */
                            0xFF,                /* allow unlimited number of instances */
                            ulBufSize,           /* output buffer size */
                            ulBufSize,           /* input buffer size */
                            0L                   /* use default time-out time */
                          );
        }
        break;
    default:
        ngLastError = ERROR_INVALID_PARAMETER;
    }

    /* if failed, release allocated memory */
    if (ngLastError)
    {
        OSL_TRACE( "osl_createPipe failed %s the pipe %s, Error Code %d.\n",
                   Options == osl_Pipe_OPEN ? "opening" : "creating",
                   strPipeNameBuffer,
                   ngLastError );
        __osl_destroyPipeImpl(pPipe);
        return NULL;
    }

    pPipe->m_Reference= 1;
    pPipe->m_bClosed  = sal_False;
    //pPipe->m_Security = Security;
    pPipe->nLastError = NO_ERROR;
    return (oslPipe)pPipe;
}

/*****************************************************************************/
/* osl_copyPipe  */
/*****************************************************************************/
oslPipe SAL_CALL osl_copyPipe(oslPipe pPipe)
{
    //oslPipe* pPipe = (oslPipe*) Pipe;
    oslPipe pNewPipe;


    /* check parameter */
    OSL_ASSERT (pPipe);

    /* allocate impl-structure */
    pNewPipe = __osl_createPipeImpl();
    if (!pNewPipe) return NULL;

    /* create new handle */
    pNewPipe->hPipe = (HPIPE) -1;
    ngLastError  = DosDupHandle( pPipe->hPipe, &(pNewPipe->hPipe) );

    /* if failed, release allocated memory */
    if (ngLastError)
    {
        OSL_TRACE( "osl_copyPipe failed duplicating pipe handle, Error-Code: %d.\n",
                   ngLastError );
        free (pNewPipe);
        return NULL;
    }

    pNewPipe->nLastError = NO_ERROR;
    return (oslPipe)pNewPipe;
}

void SAL_CALL osl_acquirePipe( oslPipe pPipe )
{
    osl_incrementInterlockedCount( &(pPipe->m_Reference) );
}

void SAL_CALL osl_releasePipe( oslPipe pPipe )
{
//      OSL_ASSERT( pPipe );

    if( 0 == pPipe )
        return;

    if( 0 == osl_decrementInterlockedCount( &(pPipe->m_Reference) ) )
    {
        if( ! pPipe->m_bClosed )
            osl_closePipe( pPipe );

        __osl_destroyPipeImpl( pPipe );
    }
}

/*****************************************************************************/
/* osl_destroyPipe  */
/*************close****************************************************************/
void SAL_CALL osl_closePipe(oslPipe pPipe)
{
    //oslPipe* pPipe = (oslPipe*) Pipe;
    /* check parameter */
    OSL_ASSERT (pPipe);

    if( pPipe && ! pPipe->m_bClosed )
    {
        pPipe->m_bClosed = sal_True;
        /* if we have a system pipe close it */
        if (pPipe->hPipe != 0)
        {
            /* disconnect client */
            DosDisConnectNPipe (pPipe->hPipe);

            /* close the pipe */
            DosClose (pPipe->hPipe);
        }
    }
}

/*****************************************************************************/
/* osl_acceptPipe  */
/*****************************************************************************/
oslPipe SAL_CALL osl_acceptPipe(oslPipe pPipe)
{

#define PINFO ((PIPEINFO *) &PipeInfoBuffer)

    ///oslPipe* pPipe = (oslPipe*) Pipe;
    oslPipe pNewPipe;
    BYTE     PipeInfoBuffer[sizeof(PIPEINFO) + CCHMAXPATHCOMP];

    /* check parameter */
    OSL_ASSERT (pPipe);

    /* get pipe information */
    pPipe->nLastError = DosQueryNPipeInfo(pPipe->hPipe,
                                         1,
                                         (PVOID) &PipeInfoBuffer,
                                         sizeof(PipeInfoBuffer));

    if (pPipe->nLastError)
    {
        OSL_TRACE( "osl_acceptPipe failed for requesting pipe information.\n",
                   pPipe->nLastError );
        return NULL;
    }

    /* create a new instance of the pipe if possible */
    if (PINFO->cbMaxInst == -1 ||                   /* unlimited instances */
        PINFO->cbMaxInst > PINFO->cbCurInst)
    {
        HPIPE hPipe;

        pNewPipe = __osl_createPipeImpl();

        if (!pNewPipe)
        {
            OSL_TRACE( "osl_acceptPipe failed creating new instance.\n", ngLastError );
            free(pNewPipe);
            return NULL;
        }

        //pNewPipe->m_Security = pPipe->m_Security;

        pNewPipe->nLastError =
            DosCreateNPipe( (PCSZ)PINFO->szName,
                            &(pNewPipe->hPipe),
                            NP_ACCESS_DUPLEX,    /* open pipe for read and write access */
                            0xFF,                /* allow unlimited number of instances */
                            ulBufSize,           /* output buffer size */
                            ulBufSize,           /* input buffer size */
                            0L                   /* use default time-out time */
                          );

        if (pNewPipe->nLastError)
        {
            OSL_TRACE( "osl_acceptPipe failed creating new named pipe, Error-Code: %d.\n",
                       pNewPipe->nLastError );
            free(pNewPipe);
            return NULL;
        }

        /* switch pipe handles */
        hPipe = pPipe->hPipe;
        pPipe->hPipe  = pNewPipe->hPipe;
        pNewPipe->hPipe = hPipe;

        /* connect new handle to client */
        pNewPipe->nLastError = DosConnectNPipe( pNewPipe->hPipe );

        /* if failed, release allocated memory */
        if (pNewPipe->nLastError)
        {
            OSL_TRACE( "osl_acceptPipe failed connecting pipe to client, Error-Code: %d.\n",
                       pNewPipe->nLastError );

            osl_closePipe((oslPipe)pNewPipe);
            return NULL;
        }
        return (oslPipe)pNewPipe;
    }
    else
    {
        /* connect original handle to client */
        pPipe->nLastError = DosConnectNPipe( pPipe->hPipe );

        if (pPipe->nLastError)
        {
            OSL_TRACE( "osl_acceptPipe failed connecting pipe to client, Error-Code: %d.\n",
                       pPipe->nLastError );
            return NULL;
        }

        return (oslPipe)pPipe;
    }
}

/*****************************************************************************/
/* osl_receivePipe  */
/*****************************************************************************/
sal_Int32 SAL_CALL osl_receivePipe(oslPipe pPipe,
                        void* pBuffer,
                        sal_Int32 BytesToRead)
{
    //oslPipe* pPipe = (oslPipe*) Pipe;
    ULONG  ulActual;

    /* check parameter */
    OSL_ASSERT (pPipe);

    /* read data from pipe */
    pPipe->nLastError = DosRead( pPipe->hPipe, pBuffer, BytesToRead, &ulActual );

    /* return -1 if failed */
    if( pPipe->nLastError )
    {
        OSL_TRACE( "osl_receivePipe failed receiving from Pipe, Error-Code: %d.\n",
                   pPipe->nLastError );
        return -1;
    }

    return ulActual;
}


/*****************************************************************************/
/* osl_sendPipe  */
/*****************************************************************************/
sal_Int32 SAL_CALL osl_sendPipe(oslPipe pPipe,
                       const void* pBuffer,
                       sal_Int32 BytesToSend)
{
    //oslPipe* pPipe = (oslPipe*) Pipe;
    ULONG  ulActual;

    /* check parameter */
    OSL_ASSERT (pPipe);

    /* read data from pipe */
    pPipe->nLastError = DosWrite( pPipe->hPipe, (PVOID) pBuffer, BytesToSend, &ulActual );

    /* return -1 if failed */
    if( pPipe->nLastError )
    {
        OSL_TRACE( "osl_receivePipe failed writing to Pipe, Error-Code: %d.\n",
                   pPipe->nLastError );
        return -1;
    }

    return ulActual;
}


/*****************************************************************************/
/* osl_getLastPipeError  */
/*****************************************************************************/

oslPipeError SAL_CALL osl_getLastPipeError(oslPipe pPipe)
{
    //oslPipe* pPipe = (oslPipe*) Pipe;
    APIRET rc;

    /* return local error value if possible */
    if (pPipe)
    {
        rc = pPipe->nLastError;
        pPipe->nLastError = NO_ERROR;
    } else
        rc = ngLastError;

    /* map OS/2 error values */
    switch (rc)
    {
    case NO_ERROR:                return osl_Pipe_E_None;
    case ERROR_PATH_NOT_FOUND:    return osl_Pipe_E_NotFound;
    case ERROR_NOT_ENOUGH_MEMORY: return osl_Pipe_E_NoBufferSpace;
    default:                      return osl_Pipe_E_invalidError;
    }
}

/*****************************************************************************/

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


/******************************************************************************
 *
 *                  New io resource transfer functions
 *
 *****************************************************************************/


/**********************************************
 osl_sendResourcePipe
 *********************************************/

sal_Bool osl_sendResourcePipe(oslPipe pPipe, oslSocket pSocket)
{
    sal_Bool bRet = sal_False;

    return bRet;
}

/**********************************************
 osl_receiveResourcePipe
 *********************************************/

oslSocket osl_receiveResourcePipe(oslPipe pPipe)
{
    oslSocket pSocket=0;

    return (oslSocket) pSocket;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
