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
#include <osl/thread.h>

#include <osl/diagnose.h>

#ifndef _OSL_FILE_PATH_HELPER_H_
#include "file_path_helper.h"
#endif

#include "procimpl.h"

#include <ctype.h>

#include <rtl/ustring.hxx>

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

/******************************************************************************
 *
 *                  Functions for starting a process
 *
 *****************************************************************************/

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

    int     rc, pid;
    int     saveOutput = -1, saveInput = -1, saveError = -1;
    int     stdOutput[2] = { -1, -1 }, stdInput[2] = { -1, -1 }, stdError[2] = { -1, -1 };
    FILE    *i, *o, *e;

    if (pInputWrite)
        pipe( stdInput);
    if (pOutputRead)
        pipe( stdOutput);
    if (pErrorRead)
        pipe( stdError);

    fcntl( stdInput[0], F_SETFD, FD_CLOEXEC);
    fcntl( stdInput[1], F_SETFD, FD_CLOEXEC);
    fcntl( stdOutput[0], F_SETFD, FD_CLOEXEC);
    fcntl( stdOutput[1], F_SETFD, FD_CLOEXEC);
    fcntl( stdError[0], F_SETFD, FD_CLOEXEC);
    fcntl( stdError[1], F_SETFD, FD_CLOEXEC);

    saveInput = dup( STDIN_FILENO);
    fcntl( saveInput, F_SETFD, FD_CLOEXEC);
    dup2( stdInput[0], STDIN_FILENO );
    close( stdInput[0] );

    saveOutput = dup( STDOUT_FILENO);
    fcntl( saveOutput, F_SETFD, FD_CLOEXEC);
    dup2( stdOutput[1], STDOUT_FILENO );
    close( stdOutput[1] );

    saveError = dup( STDERR_FILENO);
    fcntl( saveError, F_SETFD, FD_CLOEXEC);
    dup2( stdError[1], STDERR_FILENO );
    close( stdError[1] );

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

    if ( pInputWrite )
        *(pInputWrite) = osl_createFileHandleFromFD( stdInput[1] );

    if ( pOutputRead )
        *(pOutputRead) = osl_createFileHandleFromFD( stdOutput[0] );

    if ( pErrorRead )
        *(pErrorRead) = osl_createFileHandleFromFD( stdError[0] );

    // restore handles
    dup2( saveInput, STDIN_FILENO);
    close( saveInput);
    dup2( saveOutput, STDOUT_FILENO);
    close( saveOutput);
    dup2( saveError, STDERR_FILENO);
    close( saveError);

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
            // YD DosStartSession requires low-mem buffers!
            args = (sal_Char*)_tmalloc(n);
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
            // YD DosStartSession requires low-mem buffers!
            envs = (sal_Char*)_tmalloc(n + 1);
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
            _tfree(envs);
        if(args)
            _tfree(args);

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
        osl_joinProcess(pProcImpl);

    *pProcess = (oslProcess)pProcImpl;

    if( rc == NO_ERROR )
        return osl_Process_E_None;
    else

        return osl_Process_E_Unknown;
}

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
    return osl_joinProcess( Process);
}

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
