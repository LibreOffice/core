/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: process.c,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2007-09-25 09:51:57 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "system.h"

#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif

#include <osl/diagnose.h>
//#include <osl/socket.h>

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _OSL_FILE_PATH_HELPER_H_
#include "file_path_helper.h"
#endif

#include "procimpl.h"
//#include "sockimpl.h"
//#include "secimpl.h"

#include <ctype.h>

// for exception logging
#include <stdio.h>
#include <setjmp.h>
#include "helpers/except.h"


#define MAX_ARGS 255
#define PIPENAMEMASK  "\\PIPE\\OSL_PIPE_%u"
#define SEMNAMEMASK   "\\SEM32\\OSL_SEM_%u"

typedef enum {
    MSG_DATA,
    MSG_END,
    MSG_ACK,
    MSG_REL,
    MSG_UNKNOWN
} MessageType;

typedef struct {
    MessageType       m_Type;
    oslDescriptorFlag m_Flags;
    oslDescriptorType m_Data;
    HANDLE            m_Value;
} Message;

typedef struct {
    HPIPE   m_hPipe;
} Pipe;

typedef struct _oslSocketCallbackArg {
    HANDLE  m_socket;
    Pipe*   m_pipe;
} oslSocketCallbackArg;

/* process termination queue */
static sal_Bool            bInitSessionTerm = sal_False;
static const sal_Char * const SessionTermQueueName = "\\QUEUES\\SESSIONS.QUE";
static HQUEUE             SessionTermQueue;


/******************************************************************************
 *
 *                  Function Declarations
 *
 *****************************************************************************/

oslProcessError SAL_CALL osl_psz_executeProcess(sal_Char *pszImageName,
                                                sal_Char *pszArguments[],
                                                oslProcessOption Options,
                                                oslSecurity Security,
                                                sal_Char *pszDirectory,
                                                sal_Char *pszEnvironments[],
                                                oslProcess *pProcess,
                                                oslFileHandle *pInputWrite,
                                                oslFileHandle *pOutputRead,
                                                oslFileHandle *pErrorRead );

/* implemented in file.c */
extern oslFileError FileURLToPath( char *, size_t, rtl_uString* );

static sal_Bool InitSessionTerm( void )
{
    DosCreateQueue( &SessionTermQueue, QUE_FIFO, (PCSZ) SessionTermQueueName );

    return sal_True;
}


/*----------------------------------------------------------------------------*/
/* Pipes!                                                                    */
/*----------------------------------------------------------------------------*/


#if 0 // YD

static Pipe* openPipe(sal_uInt32 pid)
{
    Pipe*  ppipe = 0;
    APIRET rc;
    HPIPE  hPipe;
    HMTX   hMutex;
    sal_Char   pipeName[ _MAX_PATH ];
    sal_Char   semName[ _MAX_PATH ];

    if (pid == 0)
    {
        PTIB pptib = NULL;
        PPIB pppib = NULL;

        /* get process ID */
        DosGetInfoBlocks( &pptib, &pppib );
        pid = pppib->pib_ulpid;

        /* build pipe and semaphore names */
        sprintf( pipeName, PIPENAMEMASK, pid );
        sprintf( semName,  SEMNAMEMASK,  pid );

        /* open or create semaphore */
        rc = DosCreateMutexSem( semName, &hMutex, 0, FALSE );
        if( rc == ERROR_DUPLICATE_NAME )
            DosOpenMutexSem( semName, &hMutex );

        rc = DosCreateNPipe( pipeName,           /* name of pipe */
                             &hPipe,
                             NP_ACCESS_DUPLEX,   /* 2 way pipe */
                             1,                 /* only 1 instance needed */
                             sizeof( Message ), /* OutBufferSize */
                             sizeof( Message ), /* InBufferSize */
                             0 );                 /* use default timeout */
        if( rc != NO_ERROR )
            return NULL;

        /* connect to open pipe */
        rc = DosConnectNPipe( hPipe );
        if( rc != NO_ERROR )
        {
            DosClose( hPipe );
            return NULL;
        }

        /* release mutex and remove handle */
        DosReleaseMutexSem( hMutex );
        DosCloseMutexSem( hMutex );
    }
    else
    {
        ULONG ulAction;

        /* build pipe and semaphore names */
        sprintf( pipeName, PIPENAMEMASK, pid );
        sprintf( semName,  SEMNAMEMASK,  pid );

        /* open or create semaphore */
        rc = DosCreateMutexSem( semName, &hMutex, 1, FALSE );
        if( rc == ERROR_DUPLICATE_NAME )
            DosOpenMutexSem( semName, &hMutex );

        /* wait for mutex and remove handle */
        DosRequestMutexSem( hMutex, SEM_INDEFINITE_WAIT );
        DosCloseMutexSem( hMutex );

        /* open pipe */
        rc = DosOpen( pipeName,          /* name of pipe */
                      &hPipe,
                      &ulAction,            /* result of DosOpen */
                      0,                    /* ./. */
                      FILE_NORMAL,
                      OPEN_ACTION_OPEN_IF_EXISTS,
                      OPEN_ACCESS_READWRITE |
                      OPEN_SHARE_DENYNONE,
                      NULL );              /* ./. */

        /* set mode of pipe (we need message based data transfer) */
        DosSetNPHState( hPipe, NP_READMODE_MESSAGE );
    }

    /* allocate intern pipe structure and store pipe handle */
    ppipe = malloc(sizeof(Pipe));
    ppipe->m_hPipe = hPipe;

    return ppipe;
}

/*----------------------------------------------------------------------------*/

static void closePipe(Pipe* pipe)
{
    /* close pipe */
    if( pipe->m_hPipe != 0 )
        DosClose( pipe->m_hPipe );

    /* free intern pipe structure */
    free(pipe);
}

/*----------------------------------------------------------------------------*/

/* Callback is called when the socket is closed by the owner */
static void* socketCloseCallback(void* pArg)
{

    if (pArg != NULL)
    {
        oslSocketCallbackArg* callbackArg = (oslSocketCallbackArg*) pArg;
        Message msg;
        sal_uInt32  nbytes;

        msg.m_Type  = MSG_REL;
        msg.m_Data  = osl_Process_TypeNone;
        msg.m_Flags = 0;
        msg.m_Value = callbackArg->m_socket;
        DosWrite( callbackArg->m_pipe->m_hPipe, &msg, sizeof(msg), &nbytes );

        if( (DosRead(callbackArg->m_pipe->m_hPipe, &msg, sizeof(msg), &nbytes ) == NO_ERROR)
            && (nbytes == sizeof(msg)))
        {
            if (msg.m_Type == MSG_END)
                closePipe(callbackArg->m_pipe);
        }

        free (callbackArg);
    }

    return NULL;
}

/*----------------------------------------------------------------------------*/

static sal_Bool sendIOResources(Pipe* pipe, oslIOResource ioRes[], HANDLE hChild )
{
    sal_uInt32         nbytes;
    Message      msg;
    oslIOResource*  pIORes = ioRes;
    SOCKET          socket;
    APIRET          rc;
    HEV             hevSocketFreed;
    sal_Char            szTemp[ 20 ];
    oslSocketCallbackArg*   callbackArg;

    /* send messages with resources through pipe */
    while (pIORes->Type != osl_Process_TypeNone)
    {
        switch (pIORes->Type)
        {
            /* create message from resource */
            case osl_Process_TypeSocket:

                socket = ((oslSocketImpl *)(pIORes->Descriptor.Socket))->m_Socket;

                ((oslSocketImpl *)(pIORes->Descriptor.Socket))->m_Flags |= OSL_SOCKET_FLAGS_SHARED;

                if (pIORes->Flags & osl_Process_DFWAIT)
                {
                    callbackArg = malloc(sizeof(oslSocketCallbackArg));

                    callbackArg->m_socket = socket;
                    callbackArg->m_pipe = pipe;

                    ((oslSocketImpl *)(pIORes->Descriptor.Socket))->m_CloseCallback = &socketCloseCallback;
                    ((oslSocketImpl *)(pIORes->Descriptor.Socket))->m_CallbackArg = callbackArg;

                }

                msg.m_Type  = MSG_DATA;
                msg.m_Data  = osl_Process_TypeSocket;
                msg.m_Flags = pIORes->Flags;
                msg.m_Value = socket;

                OSL_TRACE( "sendIOResources (PTSOCKET): %d\n", socket );

                /* send message with socket */
                if (   (DosWrite( pipe->m_hPipe, &msg, sizeof(msg), &nbytes ) != NO_ERROR)
                    || (nbytes != sizeof(msg)))
                    return sal_False;

                break;

            default:
                OSL_TRACE("Not implemented");
                OSL_ASSERT(sal_False);
                break;
        }

        pIORes++;
    }

    /* send MSG_END as end marker */
    msg.m_Type = MSG_END;
    msg.m_Data = osl_Process_TypeNone;
    DosWrite( pipe->m_hPipe, &msg, sizeof(msg), &nbytes );

    /* wait for acknoledge from receiver */
    if( (DosRead(pipe->m_hPipe, &msg, sizeof(msg), &nbytes ) == NO_ERROR)
        && (nbytes == sizeof(msg))
        && ((msg.m_Type == MSG_ACK) || (msg.m_Type == MSG_END)))
    {
        if (msg.m_Type == MSG_END)
            closePipe(pipe);

        return sal_True;
    }
    else
    {
        closePipe(pipe);

        return sal_False;
    }
}

/*----------------------------------------------------------------------------*/

oslProcessError SAL_CALL osl_getIOResources(oslIOResource Resources[], sal_uInt32 Max)
{
    oslProcessError ret = osl_Process_E_Unknown;
    int             wait = 0;
    int  i = 0;
    Message msg;
    sal_uInt32   nbytes;
    Pipe*   pipe;
    sal_Char    szTemp[ 20 ];

    /* open pipe (build name from own process ID) */
    pipe = openPipe( 0 );

    /* get messages with resources from pipe */
    while( i < Max )
    {
        /* read until MSG_END received or DosRead failed */
        if (   (DosRead(pipe->m_hPipe, &msg, sizeof(msg), &nbytes ) != NO_ERROR)
            || (nbytes < sizeof(msg))
            || (msg.m_Type == MSG_END))
            break;
        else
        {
            /* get resource data from message */
            switch (msg.m_Type)
            {
                case MSG_DATA:
                    switch ( msg.m_Data )
                    {
                        case osl_Process_TypeSocket:
                        {
                            oslSocketImpl* pSockImpl = __osl_createSocketImpl( (SOCKET)msg.m_Value );

                            OSL_TRACE( "getIOResources (PTSOCKET): %d\n", (SOCKET)msg.m_Value );

                            Resources[i].Type = osl_Process_TypeSocket;
                            Resources[i].Flags = msg.m_Flags;
                            Resources[i].Descriptor.Socket = (oslSocket)pSockImpl;

                            if (msg.m_Flags & osl_Process_DFWAIT)
                                wait++;

                            i++;
                            break;
                        }

                        default:
                           OSL_TRACE("Not implemented");
                           OSL_ASSERT(sal_False);
                           break;
                    }
            }
        }
    }

    Resources[i].Type = osl_Process_TypeNone;

    if (msg.m_Type == MSG_END)
    {
        if (wait > 0)
        {
            msg.m_Type  = MSG_ACK;
            msg.m_Data  = osl_Process_TypeNone;
            msg.m_Flags = 0;
            DosWrite( pipe->m_hPipe, &msg, sizeof(msg), &nbytes );

            do
            {
                if ((! DosRead( pipe->m_hPipe, &msg, sizeof(msg), &nbytes ))
                    || (nbytes != sizeof(msg))
                    || (msg.m_Type != MSG_REL))
                    break;

                for (i = 0; Resources[i].Type != osl_Process_TypeNone; i++)
                    if (((oslSocketImpl *)Resources[i].Descriptor.Socket)->m_Socket
                        == (SOCKET)msg.m_Value)
                    {
                        OSL_ASSERT(Resources[i].Flags & osl_Process_DFWAIT);

                        Resources[i].Flags &= ~osl_Process_DFWAIT;

                        if (--wait > 0)
                        {
                            msg.m_Type  = MSG_ACK;
                            msg.m_Data  = osl_Process_TypeNone;
                            msg.m_Flags = 0;
                            DosWrite( pipe->m_hPipe, &msg, sizeof(msg), &nbytes );
                        }

                        break;
                    }
            }
            while (wait > 0);
        }

        msg.m_Type  = MSG_END;
        msg.m_Data  = osl_Process_TypeNone;
        msg.m_Flags = 0;
        DosWrite( pipe->m_hPipe, &msg, sizeof(msg), &nbytes );

        ret = osl_Process_E_None;
    }

    closePipe(pipe);

    return ret;
}

#endif // 0 YD

/*----------------------------------------------------------------------------*/

/**********************************************
 osl_executeProcess_WithRedirectedIO
 *********************************************/

oslProcessError SAL_CALL osl_executeProcess_WithRedirectedIO(
                                            rtl_uString *ustrImageName,
                                            rtl_uString *ustrArguments[],
                                            sal_uInt32   nArguments,
                                            oslProcessOption Options,
                                            oslSecurity Security,
                                            rtl_uString *ustrWorkDir,
                                            rtl_uString *ustrEnvironment[],
                                            sal_uInt32   nEnvironmentVars,
                                            oslProcess *pProcess,
                                            oslFileHandle   *pInputWrite,
                                            oslFileHandle   *pOutputRead,
                                            oslFileHandle   *pErrorRead
                                            )
{

    oslProcessError Error;
    sal_Char* pszWorkDir=0;
    sal_Char** pArguments=0;
    sal_Char** pEnvironment=0;
    unsigned int index;

    char szImagePath[PATH_MAX] = "";
    char szWorkDir[PATH_MAX] = "";

    if (Options & osl_Process_SEARCHPATH)
    {
/* ID FIXME
        const rtl::OUString PATH (RTL_CONSTASCII_USTRINGPARAM("PATH"));

        rtl_uString * pSearchPath = 0;
        osl_getEnvironment (PATH.pData, &pSearchPath);
        if (pSearchPath)
        {
            rtl_uString * pSearchResult = 0;
            osl_searchPath (ustrImageName, pSearchPath, &pSearchResult);
            if (pSearchResult)
            {
                rtl_uString_assign (ustrImageName, pSearchResult);
                rtl_uString_release (pSearchResult);
            }
            rtl_uString_release (pSearchPath);
        }
*/
    }

    if ( ustrImageName && ustrImageName->length )
    {
        FileURLToPath( szImagePath, PATH_MAX, ustrImageName );
    }

    if ( ustrWorkDir != 0 && ustrWorkDir->length )
    {
        FileURLToPath( szWorkDir, PATH_MAX, ustrWorkDir );
        pszWorkDir = szWorkDir;
    }

    if ( pArguments == 0 && nArguments > 0 )
    {
        pArguments = (sal_Char**) malloc( ( nArguments + 2 ) * sizeof(sal_Char*) );
    }


    for ( index = 0 ; index < nArguments ; ++index )
    {
        rtl_String* strArg =0;


        rtl_uString2String( &strArg,
                            rtl_uString_getStr(ustrArguments[index]),
                            rtl_uString_getLength(ustrArguments[index]),
                            osl_getThreadTextEncoding(),
                            OUSTRING_TO_OSTRING_CVTFLAGS );

        pArguments[index]=strdup(rtl_string_getStr(strArg));
        rtl_string_release(strArg);
        pArguments[index+1]=0;
    }

    for ( index = 0 ; index < nEnvironmentVars ; ++index )
    {
        rtl_String* strEnv=0;

        if ( pEnvironment == 0 )
        {
            pEnvironment = (sal_Char**) malloc( ( nEnvironmentVars + 2 ) * sizeof(sal_Char*) );
        }

        rtl_uString2String( &strEnv,
                            rtl_uString_getStr(ustrEnvironment[index]),
                            rtl_uString_getLength(ustrEnvironment[index]),
                            osl_getThreadTextEncoding(),
                            OUSTRING_TO_OSTRING_CVTFLAGS );

        pEnvironment[index]=strdup(rtl_string_getStr(strEnv));
        rtl_string_release(strEnv);
        pEnvironment[index+1]=0;
    }


    Error = osl_psz_executeProcess(szImagePath,
                                   pArguments,
                                   Options,
                                   Security,
                                   pszWorkDir,
                                   pEnvironment,
                                   pProcess,
                                   pInputWrite,
                                   pOutputRead,
                                   pErrorRead
                                   );

    if ( pArguments != 0 )
    {
        for ( index = 0 ; index < nArguments ; ++index )
        {
            if ( pArguments[index] != 0 )
            {
                free(pArguments[index]);
            }
        }
        free(pArguments);
    }

    if ( pEnvironment != 0 )
    {
        for ( index = 0 ; index < nEnvironmentVars ; ++index )
        {
            if ( pEnvironment[index] != 0 )
            {
                free(pEnvironment[index]);
            }
        }
        free(pEnvironment);
    }

    return Error;
}

/**********************************************
 osl_executeProcess
 *********************************************/

oslProcessError SAL_CALL osl_executeProcess(
                                            rtl_uString *ustrImageName,
                                            rtl_uString *ustrArguments[],
                                            sal_uInt32   nArguments,
                                            oslProcessOption Options,
                                            oslSecurity Security,
                                            rtl_uString *ustrWorkDir,
                                            rtl_uString *ustrEnvironment[],
                                            sal_uInt32   nEnvironmentVars,
                                            oslProcess *pProcess
                                            )
{
    return osl_executeProcess_WithRedirectedIO(
        ustrImageName,
        ustrArguments,
        nArguments,
        Options,
        Security,
        ustrWorkDir,
        ustrEnvironment,
        nEnvironmentVars,
        pProcess,
        NULL,
        NULL,
        NULL
        );
}

/**********************************************
 osl_executeProcess

oslProcessError SAL_CALL osl_executeProcess(
                                            rtl_uString *ustrImageName,
                                            rtl_uString *ustrArguments[],
                                            sal_uInt32   nArguments,
                                            oslProcessOption Options,
                                            oslSecurity Security,
                                            rtl_uString *ustrWorkDir,
                                            rtl_uString *ustrEnvironment[],
                                            sal_uInt32   nEnvironmentVars,
                                            oslProcess *pProcess
                                            )
 *********************************************/

/**********************************************
 osl_psz_executeProcess
 *********************************************/

oslProcessError SAL_CALL osl_psz_executeProcess(sal_Char *pszImageName,
                                                sal_Char *pszArguments[],
                                                oslProcessOption Options,
                                                oslSecurity Security,
                                                sal_Char *pszDirectory,
                                                sal_Char *pszEnvironments[],
                                                oslProcess *pProcess,
                                                oslFileHandle   *pInputWrite,
                                                oslFileHandle   *pOutputRead,
                                                oslFileHandle   *pErrorRead
                                                )
{
    ULONG ulSessID  = 0;          /* Session ID returned          */
    PID pidProcess;
    APIRET rc;
    sal_Char* pStr;
    sal_Char*        args;
    sal_Char*        envs;
    int i;
    int n = 1;
    oslProcessImpl* pProcImpl;
    ULONG nAppType, nOwnAppType;
    ULONG nCurrentDisk, nDriveMap, nBufSize;
       int   first = 0;
    sal_Char path[ _MAX_PATH ];
    sal_Char currentDir[ _MAX_PATH ];
    sal_Char ownfilename[ _MAX_PATH ];
    RESULTCODES resultCode;
    char** p;

    /* get imagename from arg list, if not specified */
    if (pszImageName == NULL)
        pszImageName = pszArguments[first++];

    OSL_ASSERT(pszImageName != NULL);

    /* check application type */
    rc = DosQueryAppType( (PCSZ) pszImageName, &nAppType );
    if( rc != NO_ERROR )
    {
        if( (rc == ERROR_FILE_NOT_FOUND) || (rc == ERROR_PATH_NOT_FOUND) )
            return osl_Process_E_NotFound;
        else
            return osl_Process_E_Unknown;
    }

    /* backup current disk information */
    if(DosQueryCurrentDisk(&nCurrentDisk, &nDriveMap))
    {
        nCurrentDisk = 0;
    }

    /* backup current directory information */
    nBufSize = _MAX_PATH;
    if(DosQueryCurrentDir(0, (BYTE*)currentDir, &nBufSize))
    {
        *currentDir = '\0';
    }

    /* change to working directory */
    if(pszDirectory && pszDirectory[1] == ':')
    {
        BYTE nDrive = toupper(pszDirectory[0]) - 'A' + 1;

        if(NO_ERROR == DosSetDefaultDisk(nDrive))
        {
            DosSetCurrentDir((PSZ) pszDirectory);
        }
    }

    /* query current executable filename and application type */
    {
        CHAR    szName[CCHMAXPATH];
        PPIB    ppib;
        PTIB    ptib;
        APIRET  rc;
        rc = DosGetInfoBlocks(&ptib, &ppib);
        rc = DosQueryModuleName(ppib->pib_hmte, sizeof(szName), szName);
        DosQueryAppType( (PCSZ)szName, &nOwnAppType );
    }

    /* combination of flags WAIT and DETACHED not supported */
    if( (Options & osl_Process_DETACHED) && (Options & osl_Process_WAIT) )
        Options &= !osl_Process_DETACHED;

    /* start in same session if possible and detached flag not set */
    if( ((nAppType & 0x00000007) == (nOwnAppType & 0x00000007))
/*      && ((Options & osl_Process_DETACHED) == 0) */ )
    {
        CHAR szbuf[CCHMAXPATH];

        /* calculate needed space for arguments */
        n = strlen( pszImageName ) + 1;
        if( pszArguments )
               for (i = first; pszArguments[i] != NULL; i++)
                   n += strlen(pszArguments[i]) + 1;

        /* allocate space for arguments */
        args = (sal_Char*)malloc(n + 1);
        pStr = args;

        /* add program name as first string to arguments */
        memcpy(pStr, pszImageName, strlen( pszImageName ) );
        pStr += strlen( pszImageName );
        *pStr++ = '\0';

        /* add given strings to arguments */
        if( pszArguments )
            for (i = first; pszArguments[i] != NULL; i++)
            {
                memcpy(pStr, pszArguments[i], strlen( pszArguments[i] ) );
                pStr += strlen( pszArguments[i] );
                if (pszArguments[i+1] != NULL)
                    *pStr++ = ' ';
            }

        /* set end marker for arguments */
        *pStr++ = '\0';
        *pStr = '\0';

        OSL_TRACE( "osl_executeProcess with DosExecPgm (args: %s)\n", args );

        /* calculate needed space for environment: since enviroment var search
           is a linear scan of the current enviroment, we place new variables
           before existing ones; so the child will find new definitions before
           olders; this doesn't require us to replace existing vars */
        // existing enviroment size
        n = 0;
        p = environ;
        while( *p)
        {
            int l = strlen( *p);
            n += l + 1;
            p++;
        }
        // new env size (if exists)
        if( pszEnvironments )
        {
            for (i = 0; pszEnvironments[i] != NULL; i++)
                   n += strlen(pszEnvironments[i]) + 1;
        }
        /* allocate space for environment */
        envs = (sal_Char*)malloc(n + 1);
        pStr = envs;

        // add new vars
        if( pszEnvironments )
        {
            /* add given strings to environment */
            for (i = 0; pszEnvironments[i] != NULL; i++)
            {
                memcpy(pStr, pszEnvironments[i], strlen( pszEnvironments[i] ) );
                pStr += strlen( pszEnvironments[i] );
                *pStr++ = '\0';
            }
        }
        // add existing vars
        p = environ;
        while( *p)
        {
            memcpy(pStr, *p, strlen( *p ) );
            pStr += strlen( *p );
            *pStr++ = '\0';
            p++;
        }
        /* set end marker for environment */
        *pStr = '\0';


        if(Options & osl_Process_DETACHED)
        {
            rc = DosExecPgm( szbuf, sizeof( szbuf ), EXEC_BACKGROUND,
                             (PSZ) args, (PSZ) envs, &resultCode, (PSZ) pszImageName );
        }
        else
        {
            rc = DosExecPgm( szbuf, sizeof( szbuf ), EXEC_ASYNCRESULT,
                             (PSZ) args, (PSZ) envs, &resultCode, (PSZ) pszImageName );
        }


        pidProcess = resultCode.codeTerminate;

        /* cleanup */
        free(envs);
           free(args);

        /* error handling */
        if( rc != NO_ERROR )
            return osl_Process_E_Unknown;
    }

    else
    {
        STARTDATA SData = { 0 };
        UCHAR achObjBuf[ 256 ] = { 0 };

        /* combine arguments separated by spaces */
        if( pszArguments )
        {
            for (i = first; pszArguments[i] != NULL; i++)
                n += strlen(pszArguments[i]) + 1;
            args = (sal_Char*)malloc(n);
            *args = '\0';
            for (i = first; pszArguments[i] != NULL; i++)
            {
                strcat(args, pszArguments[i]);
                strcat(args, " ");
            }
        }
        else
            args = NULL;

        /* combine environment separated by NULL */
        if( pszEnvironments )
        {
            for (i = 0; pszEnvironments[i] != NULL; i++)
                n += strlen(pszEnvironments[i]) + 1;
            envs = (sal_Char*)malloc(n + 1);
            pStr = (sal_Char*)envs;
            for (i = 0; pszEnvironments[i] != NULL; i++)
            {
                memcpy(pStr, pszEnvironments[i], strlen( pszEnvironments[i] ) );
                pStr += strlen( pszEnvironments[i] );
                *pStr = '\0';
                pStr++;
            }
            *pStr = '\0';
        }
        else
            envs = NULL;

        /* initialize data structure */
        memset( &SData, 0, sizeof( STARTDATA ) );
        SData.Length  = sizeof(STARTDATA);

        OSL_TRACE( "osl_executeProcess with DosStartSession (args: %s)\n", args );

        /* OS/2 Application ? */
        if(nAppType & 0x00000007)
        {

            /* inherit options from parent */
            SData.InheritOpt = SSF_INHERTOPT_PARENT;

            switch (Options & (osl_Process_NORMAL | osl_Process_MINIMIZED |
                            osl_Process_MAXIMIZED | osl_Process_FULLSCREEN))
            {
                case osl_Process_MINIMIZED:
                    SData.SessionType = SSF_TYPE_DEFAULT;
                    SData.PgmControl |= SSF_CONTROL_MINIMIZE;
                    break;

                case osl_Process_MAXIMIZED:
                    SData.SessionType = SSF_TYPE_DEFAULT;
                    SData.PgmControl |= SSF_CONTROL_MAXIMIZE;
                    break;

                case osl_Process_FULLSCREEN:
                    SData.SessionType = SSF_TYPE_FULLSCREEN;
                    break;

                default:
                    SData.SessionType = SSF_TYPE_DEFAULT;
            } /* switch */
        }


        if( Options & osl_Process_DETACHED )
        {
            /* start an independent session */
            SData.Related = SSF_RELATED_INDEPENDENT;
            SData.TermQ = NULL;
        }
        else
        {
            /* start a child session and set Termination Queue */
            SData.Related = SSF_RELATED_CHILD;

            if(! bInitSessionTerm)
                bInitSessionTerm = InitSessionTerm();

            SData.TermQ = (BYTE*) SessionTermQueueName;
        }

        SData.FgBg  = SSF_FGBG_FORE;      /* start session in foreground  */
        SData.TraceOpt = SSF_TRACEOPT_NONE;   /* No trace                */

        SData.PgmTitle = NULL;
        SData.PgmInputs = (BYTE*)args;
        SData.PgmName = (PSZ) pszImageName;
        SData.Environment = (BYTE*)envs;

        if( Options & osl_Process_HIDDEN )
            SData.PgmControl |= SSF_CONTROL_INVISIBLE;
        else
            SData.PgmControl |= SSF_CONTROL_VISIBLE;

        SData.ObjectBuffer  = (PSZ) achObjBuf;
        SData.ObjectBuffLen = (ULONG) sizeof(achObjBuf);


        /* Start the session */
        rc = DosStartSession( &SData, &ulSessID, &pidProcess );

        /* ignore error "session started in background" */
        if( rc == ERROR_SMG_START_IN_BACKGROUND )
            rc = NO_ERROR;


        if(envs)
            free(envs);
        if(args)
            free(args);

        if( rc != NO_ERROR )
            return osl_Process_E_Unknown;

    } /* else */


    /* restore current disk */
    if(nCurrentDisk)
    {
        DosSetDefaultDisk(nCurrentDisk);
    }

    /* restore current drive information */
    if(*currentDir)
    {
        DosSetCurrentDir((PCSZ)currentDir);
    }

    /* allocate intern process structure and store child process ID */
    pProcImpl = (oslProcessImpl*)malloc(sizeof(oslProcessImpl));
    pProcImpl->pProcess = pidProcess;
    pProcImpl->nSessionID = ulSessID;

       pProcImpl->bResultCodeValid = FALSE;

    if( Options & osl_Process_WAIT )
    {
        /* process in same session ? */
        if( ulSessID == 0 )
        {
            PID pidEnded;

            rc = DosWaitChild( DCWA_PROCESS, DCWW_WAIT, &resultCode,
                    &pidEnded, pidProcess );

            if( rc == NO_ERROR )
            {
                pProcImpl->nResultCode = resultCode.codeResult;
                pProcImpl->bResultCodeValid = TRUE;
            }
        }
        else
        {
            ULONG pcbData, ulElement = 0;
            REQUESTDATA rdData;
            BYTE bPriority;
            struct {
                USHORT SessionID;
                USHORT ReturnValue;
            } *pvBuffer;

            /* search/wait for the correct entry in termination queue */
            while( ( rc = DosPeekQueue( SessionTermQueue, &rdData, &pcbData,
                            (PPVOID) &pvBuffer, &ulElement, DCWW_WAIT,
                            &bPriority, NULLHANDLE )) == NO_ERROR )
            {

                if( pvBuffer->SessionID == pProcImpl->nSessionID )
                {
                    pProcImpl->nResultCode = pvBuffer->ReturnValue;
                    pProcImpl->bResultCodeValid = TRUE;

                    /* remove item from queue */
                    rc = DosReadQueue( SessionTermQueue, &rdData, &pcbData,
                           (PPVOID)&pvBuffer, ulElement, DCWW_WAIT,
                           &bPriority, NULLHANDLE );

                    break;
                }
            } /* while */
        }
    } /* if */

    *pProcess = (oslProcess)pProcImpl;

    if( rc == NO_ERROR )
        return osl_Process_E_None;
    else

        return osl_Process_E_Unknown;
}

#if 0

    /* send resources to child process through pipe */
    if (pResources)
    {
        Pipe* pipe;
        pipe = openPipe( pidProcess );

        sendIOResources(pipe, pResources, 0 );
    }

        /* windows application ? */
        else if( nAppType & ( FAPPTYP_WINDOWSREAL | FAPPTYP_WINDOWSPROT |
                          FAPPTYP_WINDOWSPROT31 ) )
        {
            /* use pszImageName as first argument */
            if( args )
            {
                sal_Char* cp = malloc( strlen( pszImageName ) + strlen( args ) + 2 );
                strcpy( cp, pszImageName );
                strcat( cp, " " );
                strcat( cp, args );
                free( args );
                args = cp;
            }
            else
            {
                args = strdup( pszImageName );
            }

            /* inherit options from shell */
            SData.InheritOpt = SSF_INHERTOPT_SHELL;

            switch (Options & (osl_Process_NORMAL | osl_Process_MINIMIZED |
                            osl_Process_MAXIMIZED | osl_Process_FULLSCREEN))
            {
                case osl_Process_MINIMIZED:
                    pszImageName = "WINOS2.COM";
                    SData.SessionType = PROG_31_ENHSEAMLESSCOMMON;
                    SData.PgmControl |= SSF_CONTROL_MINIMIZE;
                    break;

                case osl_Process_MAXIMIZED:
                    pszImageName = "WINOS2.COM";
                    SData.SessionType = PROG_31_ENHSEAMLESSCOMMON;
                    SData.PgmControl |= SSF_CONTROL_MAXIMIZE;
                    break;

                case osl_Process_FULLSCREEN:
                    pszImageName = "WIN.COM";
                    SData.SessionType = SSF_TYPE_VDM;
                    break;

                default:
                    pszImageName = "WINOS2.COM";
                    SData.SessionType = PROG_31_ENHSEAMLESSCOMMON;
            } /* switch */

            /* call win[os2].com */
            if( osl_searchPath(pszImageName, NULL, '\0', path,
                               sizeof(path)) == osl_Process_E_None)
                pszImageName = path;

        }

        /* dos application */
        else
        {
            /* inherit options from shell */
            SData.InheritOpt = SSF_INHERTOPT_SHELL;

            switch (Options & (osl_Process_NORMAL | osl_Process_MINIMIZED |
                            osl_Process_MAXIMIZED | osl_Process_FULLSCREEN))
            {
                case osl_Process_MINIMIZED:
                    SData.SessionType = SSF_TYPE_WINDOWEDVDM;
                    SData.PgmControl |= SSF_CONTROL_MINIMIZE;
                    break;

                case osl_Process_MAXIMIZED:
                    SData.SessionType = SSF_TYPE_WINDOWEDVDM;
                    SData.PgmControl |= SSF_CONTROL_MAXIMIZE;
                    break;

                case osl_Process_FULLSCREEN:
                    SData.SessionType = SSF_TYPE_VDM;
                    break;

                default:
                    SData.SessionType = SSF_TYPE_WINDOWEDVDM;
            } /* switch */

        }
#endif

/*----------------------------------------------------------------------------*/

oslProcessError SAL_CALL osl_terminateProcess(oslProcess Process)
{
    if (Process == NULL)
        return osl_Process_E_Unknown;

    /* Stop the session */
    DosStopSession( STOP_SESSION_SPECIFIED, ((oslProcessImpl*)Process)->nSessionID );

    return osl_Process_E_None;
}

/*----------------------------------------------------------------------------*/

oslProcess SAL_CALL osl_getProcess(oslProcessIdentifier Ident)
{
    HANDLE        hProcess;
    oslProcessImpl* pProcImpl;

    /* check, if given PID is a valid process */
    if (FALSE)
    {
        pProcImpl = (oslProcessImpl*)malloc(sizeof(oslProcessImpl));
/*
        pProcImpl->pProcess = pidProcess;
        pProcImpl->nSessionID = ulSessID;
*/
    }
    else
        pProcImpl = NULL;

    return (pProcImpl);
}

/*----------------------------------------------------------------------------*/

void SAL_CALL osl_freeProcessHandle(oslProcess Process)
{
    /* free intern process structure */
    if (Process != NULL)
        free((oslProcessImpl*)Process);
}

/*----------------------------------------------------------------------------*/

oslProcessError SAL_CALL osl_joinProcess(oslProcess Process)
{
    oslProcessImpl* pProcImpl = (oslProcessImpl*) Process;
    APIRET rc;

    if (Process == NULL)
        return osl_Process_E_Unknown;

    /* process of same session ? */
    if( pProcImpl->nSessionID == 0 )
    {
        RESULTCODES resultCode;
        PID pidEnded;

        rc = DosWaitChild( DCWA_PROCESS, DCWW_WAIT, &resultCode,
                &pidEnded, pProcImpl->pProcess );

        if( rc == NO_ERROR )
        {
            pProcImpl->nResultCode = resultCode.codeResult;
            pProcImpl->bResultCodeValid = TRUE;

            return osl_Process_E_None;
        }
    }
    else
    {
        ULONG pcbData, ulElement = 0;
        REQUESTDATA rdData;
        BYTE bPriority;
        struct {
            USHORT SessionID;
            USHORT ReturnValue;
        } *pvBuffer;

        /* search/wait for the correct entry in termination queue */
        while( ( rc = DosPeekQueue( SessionTermQueue, &rdData, &pcbData,
                        (PPVOID) &pvBuffer, &ulElement, DCWW_WAIT,
                        &bPriority, NULLHANDLE )) == NO_ERROR )
        {

            if( pvBuffer->SessionID == pProcImpl->nSessionID )
            {
                pProcImpl->nResultCode = pvBuffer->ReturnValue;
                pProcImpl->bResultCodeValid = TRUE;

                /* remove item from queue */
                rc = DosReadQueue( SessionTermQueue, &rdData, &pcbData,
                       (PPVOID)&pvBuffer, ulElement, DCWW_WAIT,
                       &bPriority, NULLHANDLE );

                if( rc == NO_ERROR )
                    return osl_Process_E_None;
                else
                    return osl_Process_E_Unknown;
            }
        } /* while */
    }
    return osl_Process_E_Unknown;
}

/***************************************************************************/

//YD FIXME incomplete!
oslProcessError SAL_CALL osl_joinProcessWithTimeout(oslProcess Process, const TimeValue* pTimeout)
{
    oslProcessError osl_error = osl_Process_E_Unknown;

    if (NULL == Process)
        return osl_Process_E_Unknown;

    return osl_error;
}

/*----------------------------------------------------------------------------*/
#if 0 // YD FIXME
oslProcessError SAL_CALL osl_getExecutableFile( rtl_uString **ppustrFile)
{
    PTIB pptib = NULL;
    PPIB pppib = NULL;

#if 0
    DosGetInfoBlocks(&pptib, &pppib);

    if( NO_ERROR == DosQueryPathInfo( pppib->pib_pchcmd, FIL_QUERYFULLNAME,
                *ppustrFile->buffer, rtl_ustr_getLength( ppustrFile) ) )
        return osl_Process_E_None;
    else
#endif
        return osl_Process_E_Unknown;
}
#endif

/*----------------------------------------------------------------------------*/

oslProcessError SAL_CALL osl_getCommandArgs( sal_Char* pszBuffer, sal_uInt32 Max)
{

    static int  CmdLen = -1;
    static sal_Char CmdLine[_MAX_CMD];

    OSL_ASSERT(pszBuffer);
    OSL_ASSERT(Max > 1);

    /* Query commandline during first call of function only */
    if (CmdLen < 0)
    {
        sal_Bool bEscaped = sal_False;
        sal_Bool bSeparated = sal_True;
        sal_Char* pszBufferOrg = pszBuffer;
        sal_Char* pszCmdLine;

        /* get pointer to commandline */
        {
            PTIB pptib = NULL;
            PPIB pppib = NULL;

            DosGetInfoBlocks(&pptib, &pppib);
            pszCmdLine = pppib->pib_pchcmd;
        }

        /* skip first string */
        while( *pszCmdLine )
            pszCmdLine++;

        /* concatenate commandline arguments for the given string */
        Max -= 2;
        while ( !((*pszCmdLine == '\0') && (*(pszCmdLine + 1) == '\0')) && (Max > 0))
        {
            /*
             *  C-Runtime expects char to be unsigned and so to be
             *  preceeded with 00 instead of FF when converting to int
             */
            int n = *((unsigned char *) pszCmdLine);
            if (! (isspace(n) || (*pszCmdLine == '\0')) )
            {
                if (*pszCmdLine == '"')
                {
                    if (*(pszCmdLine + 1) != '"')
                        bEscaped = ! bEscaped;
                    else
                    {
                        pszCmdLine++;
                        *pszBuffer++ = *pszCmdLine;
                        Max--;
                    }
                }
                else
                {
                    *pszBuffer++ = *pszCmdLine;
                    Max--;
                }
                bSeparated = sal_False;
            }
            else
            {
                if (bEscaped)
                    *pszBuffer++ = *pszCmdLine;
                else
                    if (! bSeparated)
                    {
                        *pszBuffer++ = '\0';
                        bSeparated = sal_True;
                    }
                Max--;
            }

            pszCmdLine++;
        }

        *pszBuffer++ = '\0';
        *pszBuffer++ = '\0';

        /* restore pointer and save commandline for next query */
        CmdLen = pszBuffer - pszBufferOrg;
        pszBuffer = pszBufferOrg;
        memcpy( CmdLine, pszBuffer, CmdLen );
    }
    else
       memcpy( pszBuffer, CmdLine, CmdLen );

    OSL_TRACE( "osl_getCommandArgs (args: %s)\n", pszBuffer );

    return osl_Process_E_None;
}

/*----------------------------------------------------------------------------*/
#if 0
oslProcessError SAL_CALL osl_getEnvironment(rtl_uString* pszName, rtl_uString **ustrValue)
{
    //sal_Char* pszEnv;

    /* search for environment string and copy value if found */
    if (DosScanEnv((sal_Char*)pszName, ustrValue) == NO_ERROR)
    {
        //strncpy(pszBuffer, pszEnv, Max);
        //pszBuffer[Max - 1] = '\0';

        return osl_Process_E_None;
    }

    return osl_Process_E_NotFound;
}
#endif

/*----------------------------------------------------------------------------*/

oslProcessError SAL_CALL osl_getProcessInfo(oslProcess Process, oslProcessData Fields,
                                   oslProcessInfo* pInfo)
{
    if (! pInfo || (pInfo->Size != sizeof(oslProcessInfo)))
        return osl_Process_E_Unknown;

    pInfo->Fields = 0;

    if (Fields & osl_Process_IDENTIFIER)
    {
        if( Process == NULL )
        {
            PTIB pptib = NULL;
            PPIB pppib = NULL;

            DosGetInfoBlocks( &pptib, &pppib );
            pInfo->Ident = pppib->pib_ulpid;
        }
        else
            pInfo->Ident = ((oslProcessImpl*)Process)->pProcess;

        pInfo->Fields |= osl_Process_IDENTIFIER;
    }

    if (Fields & osl_Process_EXITCODE)
    {
        oslProcessImpl* pProcImpl = (oslProcessImpl*) Process;

        if( pProcImpl->bResultCodeValid )
        {
            pInfo->Code = pProcImpl->nResultCode;
            pInfo->Fields |= osl_Process_EXITCODE;
        }
        else
        {
            APIRET rc;

            if( pProcImpl->nSessionID == 0 )
            {
                RESULTCODES resultCode;
                PID pidEnded;

                rc = DosWaitChild( DCWA_PROCESS, DCWW_WAIT, &resultCode,
                        &pidEnded, pProcImpl->pProcess );

                if( rc == NO_ERROR )
                {
                    pProcImpl->nResultCode = resultCode.codeResult;
                    pProcImpl->bResultCodeValid = TRUE;

                    pInfo->Code = pProcImpl->nResultCode;
                    pInfo->Fields |= osl_Process_EXITCODE;

                    return osl_Process_E_None;
                }
            }
            else
            {
                ULONG pcbData, ulElement = 0;
                REQUESTDATA rdData;
                BYTE bPriority;
                struct {
                    USHORT SessionID;
                    USHORT ReturnValue;
                } *pvBuffer;

                /* search/wait for the correct entry in termination queue */
                while( ( rc = DosPeekQueue( SessionTermQueue, &rdData, &pcbData,
                                (PPVOID) &pvBuffer, &ulElement, DCWW_WAIT,
                                &bPriority, NULLHANDLE )) == NO_ERROR )
                {

                    if( pvBuffer->SessionID == pProcImpl->nSessionID )
                    {
                        pProcImpl->nResultCode = pvBuffer->ReturnValue;
                        pProcImpl->bResultCodeValid = TRUE;

                        pInfo->Code = pProcImpl->nResultCode;
                        pInfo->Fields |= osl_Process_EXITCODE;

                        /* remove item from queue */
                        rc = DosReadQueue( SessionTermQueue, &rdData, &pcbData,
                               (PPVOID)&pvBuffer, ulElement, DCWW_WAIT,
                               &bPriority, NULLHANDLE );

                        break;
                    }
                }
            }
        }
    }

    if (Fields & osl_Process_HEAPUSAGE)
    {
    }
    if (Fields & osl_Process_CPUTIMES)
    {
    }

    return (pInfo->Fields == Fields) ? osl_Process_E_None : osl_Process_E_Unknown;
}

/*----------------------------------------------------------------------------*/
#if 0
oslProcessError SAL_CALL osl_searchPath(const sal_Char* pszName, const sal_Char* pszPath,
                               sal_Char Separator, sal_Char *pszBuffer, sal_uInt32 Max)
{

    OSL_ASSERT(pszName != NULL);

    if( (Separator == '\0') || (Separator == ';') )
    {
        APIRET rc;

        rc = DosSearchPath( SEARCH_IGNORENETERRS | SEARCH_ENVIRONMENT | SEARCH_CUR_DIRECTORY,
                            pszPath == NULL ? "PATH" : (sal_Char*) pszPath,
                            (sal_Char*) pszName,
                            pszBuffer, Max );

        return( rc == NO_ERROR ? osl_Process_E_None : osl_Process_E_NotFound );
    }
    else
    {
        sal_Char path[_MAX_PATH];
        sal_Char buffer[_MAX_ENV];
        sal_Char* pszEnv;

        if (pszPath == NULL)
            pszPath = "PATH";

        if (DosScanEnv((sal_Char*)pszPath, &pszEnv) == NO_ERROR)
        {
            sal_Char *pstr, *pchr = buffer;

            strncpy(buffer, pszEnv, Max);
            pszBuffer[Max - 1] = '\0';

            while (*pchr != '\0')
            {
                pstr = path;

                while ((*pchr != '\0') && (*pchr != Separator))
                    *pstr++ = *pchr++;

                if  ((pstr > path) &&
                     ((*(pstr - 1) != ':') && (*(pstr - 1) != '\\')))
                    *pstr++ = '\\';

                *pstr = '\0';

                strcat(path, pszName);

                if (access(path, 0) == 0)
                {
                    sal_Char* pName;

                    if( (DosQueryPathInfo( path, FIL_QUERYFULLNAME, path, sizeof( path ) ) != NO_ERROR )
                        || (strlen( path ) >= (size_t)Max) )
                        return osl_Process_E_Unknown;

                    strcpy(pszBuffer, path);

                    return osl_Process_E_None;
                }

                if (*pchr == Separator)
                    pchr++;
            }
        }

        return osl_Process_E_NotFound;
    }

}
#endif

/*----------------------------------------------------------------------------*/




static CHAR        szOOoExe[CCHMAXPATH];

static FILE* APIENTRY _oslExceptOpenLogFile(VOID)
{
   FILE        *file;
   DATETIME    DT;
   PPIB        pib;
   PSZ         slash;

   // get executable fullpath
   DosGetInfoBlocks(NULL, &pib);
   DosQueryModuleName(pib->pib_hmte, sizeof(szOOoExe), szOOoExe);
   // truncate to exe name
   slash = (PSZ)strrchr( szOOoExe, '.');
   *slash = '\0';
   // make log path
   strcat( szOOoExe, ".log");

   file = fopen( szOOoExe, "a");
   if (!file) { // e.g. readonly drive
      // try again, usually C exist and is writable
      file = fopen( "c:\\OOo.log", "a");
   }
   if (file) {
        DosGetDateTime(&DT);
        fprintf(file, "\nTrap message -- Date: %04d-%02d-%02d, Time: %02d:%02d:%02d\n",
            DT.year, DT.month, DT.day,
            DT.hours, DT.minutes, DT.seconds);
        fprintf(file, "-------------------------------------------------------\n"
                      "\nAn internal error occurred (Built " __DATE__ "-" __TIME__ ").\n");

   }

   // ok, return handle
   return (file);
}

int SAL_CALL osl_ProcessHook( int (*sal_main)( int, char**), int argc, char ** argv)
{
   int rc = -1;

   excRegisterHooks(_oslExceptOpenLogFile, NULL, NULL, FALSE);

   TRY_LOUD(excpt1) {

    osl_setCommandArgs(argc, argv);
    rc = sal_main(argc, argv);

   } CATCH(excpt1) {
   } END_CATCH();

   return rc;
}
