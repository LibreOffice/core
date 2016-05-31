/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_features.h>

#include <signal.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <sys/poll.h>
#include <fcntl.h>
#include <stdio.h>
#include <libgen.h>
#include <string.h>
#include <errno.h>

#include <desktop/exithelper.h>
#include <osl/process.h>
#include <osl/thread.h>
#include <rtl/bootstrap.h>
#include <rtl/digest.h>
#include <rtl/process.h>
#include <rtl/ustrbuf.h>
#include <sal/main.h>

#include "args.h"
#include "pagein.h"
#include "splashx.h"

#define PIPEDEFAULTPATH      "/tmp"
#define PIPEALTERNATEPATH    "/var/tmp"

/* Easier conversions: rtl_uString to rtl_String */
static rtl_String *
ustr_to_str( rtl_uString *pStr )
{
    rtl_String *pOut = NULL;

    rtl_uString2String( &pOut, rtl_uString_getStr( pStr ),
            rtl_uString_getLength( pStr ), osl_getThreadTextEncoding(), OUSTRING_TO_OSTRING_CVTFLAGS );

    return pOut;
}

/* Easier conversions: char * to rtl_uString */
static rtl_uString *
charp_to_ustr( const char *pStr )
{
    rtl_uString *pOut = NULL;

    rtl_string2UString( &pOut, pStr, strlen( pStr ), osl_getThreadTextEncoding(), OSTRING_TO_OUSTRING_CVTFLAGS );

    return pOut;
}

typedef struct {
    int        status_fd;
    oslProcess child;
} ChildInfo;

static int
child_info_get_status_fd (ChildInfo *info)
{
    return info->status_fd;
}

static void
child_info_destroy (ChildInfo *info)
{
    close (info->status_fd);
    osl_freeProcessHandle (info->child);
    free (info);
}

static ChildInfo *
child_spawn ( Args *args, sal_Bool bAllArgs, sal_Bool bWithStatus )
{
    rtl_uString *pApp = NULL, *pTmp = NULL;
    rtl_uString **ppArgs;
    sal_uInt32 nArgs, i;
    ChildInfo *info;
    int status_pipe[2];
    oslProcessError nError;

    info = calloc (1, sizeof (ChildInfo));

    /* create pipe */
    if ( pipe( status_pipe ) < 0 )
    {
        fprintf( stderr, "ERROR: no file handles\n");
        exit( 1 );
    }
    info->status_fd = status_pipe[0];

    /* application name */
    rtl_uString_newFromAscii( &pApp, "file://" );
    rtl_uString_newConcat( &pApp, pApp, args->pAppPath );
    rtl_uString_newFromAscii( &pTmp, "soffice.bin" );
    rtl_uString_newConcat( &pApp, pApp, pTmp );
    rtl_uString_release( pTmp );
    pTmp = NULL;

    /* copy args */
    nArgs = bAllArgs ? args->nArgsTotal : args->nArgsEnv;
    ppArgs = (rtl_uString **)calloc( nArgs + 1, sizeof( rtl_uString* ) );
    for ( i = 0; i < nArgs; ++i )
        ppArgs[i] = args->ppArgs[i];

    if( bWithStatus )
    {
        char buffer[64];

        /* add the pipe arg */
        snprintf (buffer, 63, "--splash-pipe=%d", status_pipe[1]);
        rtl_uString_newFromAscii( &pTmp, buffer );
        ppArgs[nArgs] = pTmp;
        ++nArgs;
    }

    /* start the main process */
    nError = osl_executeProcess( pApp, ppArgs, nArgs,
                                 osl_Process_NORMAL,
                                 NULL,
                                 NULL,
                                 NULL, 0,
                                 &info->child );

    if (pTmp)
        rtl_uString_release( pTmp );
    free (ppArgs);

    if ( nError != osl_Process_E_None )
    {
        fprintf( stderr, "ERROR %d forking process\n", nError );
        rtl_uString_release( pApp );
        _exit (1);
    }

    rtl_uString_release( pApp );
    close( status_pipe[1] );

    return info;
}

static sal_Bool
child_exited_wait (ChildInfo *info, sal_Bool bShortWait)
{
    TimeValue t = { 0, 250 /* ms */ * 1000 * 1000 };
    if (!bShortWait)
        t.Seconds = 1024;
    return osl_joinProcessWithTimeout (info->child, &t) != osl_Process_E_TimedOut;
}

static int
child_get_exit_code (ChildInfo *info)
{
    oslProcessInfo inf;

    inf.Code = -1;
    inf.Size = sizeof (inf);
    if (osl_getProcessInfo (info->child, osl_Process_EXITCODE, &inf) != osl_Process_E_None)
    {
        fprintf (stderr, "Warning: failed to fetch libreoffice exit status\n");
        return -1;
    }
    return inf.Code;
}

typedef enum { ProgressContinue, ProgressRestart, ProgressExit } ProgressStatus;

/* Path of the application, with trailing slash. */
static rtl_uString *
get_app_path( const char *pAppExec )
{
    char pRealPath[PATH_MAX];
    rtl_uString *pResult;
    sal_Int32 len;
    char* dummy;

    char *pOrigPath = strdup( pAppExec );
    char *pPath = dirname( pOrigPath );

    dummy = realpath( pPath, pRealPath );
    (void)dummy;
    pResult = charp_to_ustr( pRealPath );
    free( pOrigPath );

    len = rtl_uString_getLength(pResult);
    if (len > 0 && rtl_uString_getStr(pResult)[len - 1] != '/')
    {
        rtl_uString *pSlash = NULL;
        rtl_uString_newFromAscii(&pSlash, "/");
        rtl_uString_newConcat(&pResult, pResult, pSlash);
        rtl_uString_release(pSlash);
    }

    return pResult;
}

/* Compute the OOo md5 hash from 'pText' */
static rtl_uString *
get_md5hash( rtl_uString *pText )
{
    rtl_uString *pResult = NULL;
    sal_Int32 nCapacity = 100;
    unsigned char *pData = NULL;
    sal_uInt32 nSize = 0;
    rtlDigest digest;
    sal_uInt32 md5_key_len = 0;
    sal_uInt8* md5_buf = NULL;
    sal_uInt32 i = 0;

    if ( !pText )
        return NULL;

    pData = (unsigned char *)rtl_uString_getStr( pText );
    nSize = rtl_uString_getLength( pText ) * sizeof( sal_Unicode );
    if ( !pData )
        return NULL;

    digest = rtl_digest_create( rtl_Digest_AlgorithmMD5 );
    if ( digest == NULL )
        return NULL;

    md5_key_len = rtl_digest_queryLength( digest );
    md5_buf = (sal_uInt8 *)calloc( md5_key_len, sizeof( sal_uInt8 ) );

    rtl_digest_init( digest, pData , nSize );
    rtl_digest_update( digest, pData, nSize );
    rtl_digest_get( digest, md5_buf, md5_key_len );
    rtl_digest_destroy( digest );

    /* create hex-value string from the MD5 value to keep
       the string size minimal */
    rtl_uString_new_WithLength( &pResult, nCapacity );
    for ( ; i < md5_key_len; ++i )
    {
        char val[3];
        snprintf( val, 3, "%x", md5_buf[i] ); /* sic! we ignore some of the 0's */

        rtl_uStringbuffer_insert_ascii( &pResult, &nCapacity, rtl_uString_getLength( pResult ),
                val, strlen( val ) );
    }

    /* cleanup */
    free( md5_buf );

    return pResult;
}

/* Construct the pipe name */
static rtl_uString *
get_pipe_path( rtl_uString *pAppPath )
{
    rtl_uString *pPath = NULL, *pTmp = NULL, *pUserInstallation = NULL;
    rtl_uString *pResult = NULL, *pBasePath = NULL, *pAbsUserInstallation = NULL;
    rtlBootstrapHandle handle;
    rtl_uString *pMd5hash = NULL;
    sal_Unicode pUnicode[RTL_USTR_MAX_VALUEOFINT32];

    /* setup bootstrap filename */
    rtl_uString_newFromAscii( &pPath, "file://" );
    rtl_uString_newConcat( &pPath, pPath, pAppPath );
    rtl_uString_newConcat( &pPath, pPath, pTmp );
    rtl_uString_newFromAscii( &pTmp, SAL_CONFIGFILE( "bootstrap" ) );
    rtl_uString_newConcat( &pPath, pPath, pTmp );

    /* read userinstallation value */
    handle = rtl_bootstrap_args_open( pPath );

    rtl_uString_newFromAscii( &pTmp, "UserInstallation" );
    rtl_bootstrap_get_from_handle( handle, pTmp, &pUserInstallation, NULL );

    rtl_bootstrap_args_close( handle );

    /* turn it into an absolute path - unwinding symlinks etc. */
    if ( osl_getProcessWorkingDir (&pBasePath) ||
         osl_getAbsoluteFileURL( pBasePath, pUserInstallation, &pAbsUserInstallation ) )
        rtl_uString_newFromString (&pAbsUserInstallation, pUserInstallation);

    /* create the pipe name */
    pMd5hash = get_md5hash( pAbsUserInstallation );
    if ( !pMd5hash )
        rtl_uString_new( &pMd5hash );

    if ( access( PIPEDEFAULTPATH, W_OK ) == 0 )
        rtl_uString_newFromAscii( &pResult, PIPEDEFAULTPATH );
    else if ( access( PIPEALTERNATEPATH, W_OK ) == 0 )
        rtl_uString_newFromAscii( &pResult, PIPEALTERNATEPATH );
    else
    {
        fprintf( stderr, "ERROR: no valid pipe path found.\n" );
        exit( 1 );
    }

    rtl_uString_newFromAscii( &pTmp, "/OSL_PIPE_" );
    rtl_uString_newConcat( &pResult, pResult, pTmp );

    rtl_ustr_valueOfInt32( pUnicode, (int)getuid(), 10 );
    rtl_uString_newFromStr( &pTmp, pUnicode );
    rtl_uString_newConcat( &pResult, pResult, pTmp );

    rtl_uString_newFromAscii( &pTmp, "_SingleOfficeIPC_" );
    rtl_uString_newConcat( &pResult, pResult, pTmp );

    rtl_uString_newConcat( &pResult, pResult, pMd5hash );

    /* cleanup */
    rtl_uString_release( pMd5hash );
    rtl_uString_release( pPath );
    rtl_uString_release( pTmp );
    if ( pBasePath )
    {
        rtl_uString_release( pBasePath );
    }
    rtl_uString_release( pUserInstallation );
    rtl_uString_release( pAbsUserInstallation );

    return pResult;
}

/* Get fd of the pipe of the already running OOo. */
static int
connect_pipe( rtl_uString *pPipePath )
{
    int fd;
    size_t len;
    struct sockaddr_un addr;

    rtl_String *pPipeStr = ustr_to_str( pPipePath );

    memset( &addr, 0, sizeof( addr ) );

    if ( ( fd = socket( AF_UNIX, SOCK_STREAM, 0 ) ) < 0 )
        return fd;

    (void)fcntl( fd, F_SETFD, FD_CLOEXEC );

    addr.sun_family = AF_UNIX;
    strncpy( addr.sun_path, rtl_string_getStr( pPipeStr ), sizeof( addr.sun_path ) - 1 );
    rtl_string_release( pPipeStr );

/* cut / paste from osl's pipe.c */
#if defined(FREEBSD)
    len = SUN_LEN( &addr );
#else
    len = sizeof( addr );
#endif

    if ( connect( fd, (struct sockaddr *)&addr, len ) < 0 )
    {
        close(fd);
        fd = -1;
    }
    return fd;
}

/* Escape: "," -> "\\,", "\0" -> "\\0", "\\" -> "\\\\" */
static rtl_uString *
escape_path( rtl_uString *pToEscape )
{
    rtl_uString *pBuffer = NULL;
    sal_Int32 nCapacity = 1000;
    sal_Int32 i = 0;
    sal_Int32 nEscapeLength = rtl_uString_getLength( pToEscape );

    rtl_uString_new_WithLength( &pBuffer, nCapacity );

    for ( ; i < nEscapeLength; ++i )
    {
        sal_Unicode c = pToEscape->buffer[i];
        switch ( c )
        {
            case (sal_Unicode)'\0':
                rtl_uStringbuffer_insert_ascii( &pBuffer, &nCapacity,
                        rtl_uString_getLength( pBuffer ),
                        RTL_CONSTASCII_STRINGPARAM( "\\0" ) );
                break;
            case (sal_Unicode)',':
                rtl_uStringbuffer_insert_ascii( &pBuffer, &nCapacity,
                        rtl_uString_getLength( pBuffer ),
                        RTL_CONSTASCII_STRINGPARAM( "\\," ) );
                break;
            case (sal_Unicode)'\\':
                rtl_uStringbuffer_insert_ascii( &pBuffer, &nCapacity,
                        rtl_uString_getLength( pBuffer ),
                        RTL_CONSTASCII_STRINGPARAM( "\\\\" ) );
                break;
            default:
                rtl_uStringbuffer_insert( &pBuffer, &nCapacity,
                        rtl_uString_getLength( pBuffer ),
                        &c, 1 );
        }
    }

    return pBuffer;
}

/* Send args to the OOo instance (using the 'fd' file descriptor) */
static sal_Bool
send_args( int fd, rtl_uString *pCwdPath )
{
    rtl_uString *pBuffer = NULL, *pTmp = NULL;
    sal_Int32 nCapacity = 1000;
    rtl_String *pOut = NULL;
    sal_Bool bResult;
    size_t nLen;
    rtl_uString *pEscapedCwdPath = escape_path( pCwdPath );
    sal_uInt32 nArg = 0;
    sal_uInt32 nArgCount = rtl_getAppCommandArgCount();

    rtl_uString_new_WithLength( &pBuffer, nCapacity );
    rtl_uString_new( &pTmp );

    rtl_uStringbuffer_insert_ascii( &pBuffer, &nCapacity,
            rtl_uString_getLength( pBuffer ),
            RTL_CONSTASCII_STRINGPARAM( "InternalIPC::Arguments" ) );

    if ( rtl_uString_getLength( pEscapedCwdPath ) )
    {
        rtl_uStringbuffer_insert_ascii( &pBuffer, &nCapacity,
            rtl_uString_getLength( pBuffer ),
            RTL_CONSTASCII_STRINGPARAM( "1" ) );
        rtl_uStringbuffer_insert( &pBuffer, &nCapacity,
                    rtl_uString_getLength( pBuffer ),
                    rtl_uString_getStr( pEscapedCwdPath ),
                    rtl_uString_getLength( pEscapedCwdPath ) );
    }
    else
    {
        rtl_uStringbuffer_insert_ascii( &pBuffer, &nCapacity,
            rtl_uString_getLength( pBuffer ),
            RTL_CONSTASCII_STRINGPARAM( "0" ) );
    }

    for ( nArg = 0; nArg < nArgCount; ++nArg )
    {
        rtl_uString *pEscapedTmp = NULL;
        rtl_uStringbuffer_insert_ascii( &pBuffer, &nCapacity,
                rtl_uString_getLength( pBuffer ),
                ",", 1 );

        rtl_getAppCommandArg( nArg, &pTmp );

        pEscapedTmp = escape_path( pTmp );

        rtl_uStringbuffer_insert( &pBuffer, &nCapacity,
                rtl_uString_getLength( pBuffer ),
                rtl_uString_getStr( pEscapedTmp ),
                rtl_uString_getLength( pEscapedTmp ) );

        rtl_uString_release( pEscapedTmp );
    }

    if ( !rtl_convertUStringToString(
             &pOut, rtl_uString_getStr( pBuffer ),
             rtl_uString_getLength( pBuffer ), RTL_TEXTENCODING_UTF8,
             ( RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR
               | RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR ) ) )
    {
        fprintf( stderr, "ERROR: cannot convert arguments to UTF-8\n" );
        exit( 1 );
    }

    nLen = rtl_string_getLength( pOut ) + 1;
    bResult = ( write( fd, rtl_string_getStr( pOut ), nLen ) == (ssize_t) nLen );

    if ( bResult )
    {
        char resp[ strlen( "InternalIPC::ProcessingDone" ) ];
        ssize_t n = read( fd, resp, SAL_N_ELEMENTS( resp ) );
        bResult = n == (ssize_t) SAL_N_ELEMENTS( resp )
            && (memcmp(
                    resp, "InternalIPC::ProcessingDone",
                    SAL_N_ELEMENTS( resp ) )
                == 0);
    }

    /* cleanup */
    rtl_uString_release( pEscapedCwdPath );
    rtl_uString_release( pBuffer );
    rtl_uString_release( pTmp );
    rtl_string_release( pOut );

    return bResult;
}


#define BUFFER_LEN 255

/* Read the percent to show in splash. */
static ProgressStatus
read_percent( ChildInfo *info, int *pPercent )
{
    static char pBuffer[BUFFER_LEN + 1];
    static char *pNext = pBuffer;
    static ssize_t nRead = 0;

    char *pBegin;
    char *pIter;
    char c;

    /* from the last call */
    int nNotProcessed = nRead - ( pNext - pBuffer );
    if ( nNotProcessed >= BUFFER_LEN )
        return ProgressContinue;

    memmove( pBuffer, pNext, nNotProcessed );

    /* read data */
    nRead = read( child_info_get_status_fd (info),
                  pBuffer + nNotProcessed, BUFFER_LEN - nNotProcessed );
    if ( nRead < 0 ) {
        if (errno == EINTR)
            return ProgressContinue;
        return ProgressExit;
    }

    nRead += nNotProcessed;
    pBuffer[nRead] = '\0';

    /* skip old data */
    pBegin = pBuffer;
    pNext = pBuffer;
    for ( pIter = pBuffer; *pIter; ++pIter )
    {
        if ( *pIter == '\n' )
        {
            pBegin = pNext;
            pNext = pIter + 1;
        }
    }

    if ( !strncasecmp( pBegin, "end", 3 ) )
        return ProgressExit;
    else if ( !strncasecmp( pBegin, "restart", 7 ) )
        return ProgressRestart;
    else if ( sscanf( pBegin, "%d%c", pPercent, &c ) == 2 && c == '%' )
        return ProgressContinue;

    /* unexpected - let's exit the splash to be safe */
    return ProgressExit;
}

/* Simple system check. */
static void
system_checks( void )
{
#ifdef LINUX
    struct stat buf;

    /* check proc is mounted - lots of things fail otherwise */
    if ( stat( "/proc/version", &buf ) != 0 )
    {
        fprintf( stderr, "ERROR: /proc not mounted - LibreOffice is unlikely to work well if at all\n" );
        exit( 1 );
    }
#endif
}

void
exec_pagein (Args *args)
{
    rtl_String * path = ustr_to_str(args->pAppPath);
    pagein_execute(rtl_string_getStr(path), "pagein-common");
    if (args->pPageinType) {
        pagein_execute(rtl_string_getStr(path), args->pPageinType);
    }
    rtl_string_release(path);
}

#if HAVE_FEATURE_JAVA

static void extend_library_path (const char *new_element)
{
    rtl_uString *pEnvName=NULL, *pOrigEnvVar=NULL, *pNewEnvVar=NULL;
    const char *pathname;
#ifdef AIX
    pathname = "LIBPATH";
#else
    pathname = "LD_LIBRARY_PATH";
#endif

    rtl_uString_newFromAscii( &pEnvName, pathname );
    rtl_uString_newFromAscii( &pNewEnvVar, new_element );

    osl_getEnvironment( pEnvName, &pOrigEnvVar );
    if (pOrigEnvVar && pOrigEnvVar->length)
    {
        rtl_uString *pDelim = NULL;
        rtl_uString_newFromAscii( &pDelim, ":" );
        rtl_uString_newConcat( &pNewEnvVar, pNewEnvVar, pDelim );
        rtl_uString_newConcat( &pNewEnvVar, pNewEnvVar, pOrigEnvVar );
        rtl_uString_release( pDelim );
    }

    osl_setEnvironment( pEnvName, pNewEnvVar );

    if (pOrigEnvVar)
        rtl_uString_release( pOrigEnvVar );
    rtl_uString_release( pNewEnvVar );
    rtl_uString_release( pEnvName );
}

static void
exec_javaldx (Args *args)
{
    char newpath[4096];
    sal_uInt32 nArgs;
    rtl_uString *pApp;
    rtl_uString **ppArgs;
    rtl_uString *pTmp, *pTmp2;

    oslProcess javaldx = NULL;
    oslFileHandle fileOut= NULL;
    oslProcessError err;

    ppArgs = (rtl_uString **)calloc( args->nArgsEnv + 2, sizeof( rtl_uString* ) );

    for ( nArgs = 0; nArgs < args->nArgsEnv; ++nArgs )
        ppArgs[nArgs] = args->ppArgs[nArgs];

    /* Use absolute path to redirectrc */
    pTmp = NULL;
    rtl_uString_newFromAscii( &pTmp, "-env:INIFILENAME=vnd.sun.star.pathname:" );
    rtl_uString_newConcat( &pTmp, pTmp, args->pAppPath );
    pTmp2 = NULL;
    rtl_uString_newFromAscii( &pTmp2, "redirectrc" );
    rtl_uString_newConcat( &pTmp, pTmp, pTmp2 );
    ppArgs[nArgs] = pTmp;
    rtl_uString_release (pTmp2);
    nArgs++;

    /* And also to javaldx */
    pApp = NULL;
    rtl_uString_newFromAscii( &pApp, "file://" );
    rtl_uString_newConcat( &pApp, pApp, args->pAppPath );
    pTmp = NULL;
    rtl_uString_newFromAscii( &pTmp, "javaldx" );
    rtl_uString_newConcat( &pApp, pApp, pTmp );
    rtl_uString_release( pTmp );

    err = osl_executeProcess_WithRedirectedIO( pApp, ppArgs, nArgs,
                                               osl_Process_NORMAL,
                                               NULL, // security
                                               NULL, // work dir
                                               NULL, 0,
                                               &javaldx, // process handle
                                               NULL,
                                               &fileOut,
                                               NULL);

    rtl_uString_release( ppArgs[nArgs-1] );
    rtl_uString_release( pApp );
    free( ppArgs );

    if( err != osl_Process_E_None)
    {
        fprintf (stderr, "Warning: failed to launch javaldx - java may not function correctly\n");
        if (javaldx)
            osl_freeProcessHandle(javaldx);
        if (fileOut)
            osl_closeFile(fileOut);
        return;
    } else {
        char *chomp;
        sal_uInt64 bytes_read;

        /* Magically osl_readLine doesn't work with pipes with E_SPIPE - so be this lame instead: */
        while (osl_readFile (fileOut, newpath, SAL_N_ELEMENTS (newpath), &bytes_read) == osl_File_E_INTR);

        if (bytes_read <= 0) {
            fprintf (stderr, "Warning: failed to read path from javaldx\n");
            if (javaldx)
                osl_freeProcessHandle(javaldx);
            if (fileOut)
                osl_closeFile(fileOut);
            return;
        }
        newpath[bytes_read] = '\0';

        if ((chomp = strstr (newpath, "\n")))
            *chomp = '\0';
    }

    extend_library_path (newpath);

    if (javaldx)
        osl_freeProcessHandle(javaldx);
    if (fileOut)
        osl_closeFile(fileOut);
}

#endif

// has to be a global :(
oslProcess * volatile g_pProcess = NULL;

void sigterm_handler(int ignored)
{
    (void) ignored;
    if (g_pProcess)
    {
        // forward signal to soffice.bin
        osl_terminateProcess(g_pProcess);
    }
    _exit(255);
}


SAL_IMPLEMENT_MAIN_WITH_ARGS( argc, argv )
{
    sal_Bool bSentArgs = sal_False;
    const char* pUsePlugin;
    rtl_uString *pPipePath = NULL;
    Args *args;
    int status = 0;
    struct splash* splash = NULL;
    struct sigaction sigpipe_action;
    struct sigaction sigterm_action;

    /* turn SIGPIPE into an error */
    memset(&sigpipe_action, 0, sizeof(struct sigaction));
    sigpipe_action.sa_handler = SIG_IGN;
    sigemptyset(&sigpipe_action.sa_mask);
    sigaction(SIGPIPE, &sigpipe_action, NULL);
    memset(&sigterm_action, 0, sizeof(struct sigaction));
    sigterm_action.sa_handler = &sigterm_handler;
    sigemptyset(&sigterm_action.sa_mask);
    sigaction(SIGTERM, &sigterm_action, NULL);

    args = args_parse ();
    args->pAppPath = get_app_path( argv[0] );
    if ( !args->pAppPath )
    {
        fprintf( stderr, "ERROR: Can't read app link\n" );
        exit( 1 );
    }

#ifndef ENABLE_QUICKSTART_LIBPNG
    /* we can't load and render it anyway */
    args->bInhibitSplash = sal_True;
#endif

    pUsePlugin = getenv( "SAL_USE_VCLPLUGIN" );
    if ( pUsePlugin && !strcmp(pUsePlugin, "svp") )
        args->bInhibitSplash = sal_True;

    if ( !args->bInhibitPipe && getenv("LIBO_FLATPAK") == NULL )
    {
        int fd = 0;
        pPipePath = get_pipe_path( args->pAppPath );

        if ( ( fd = connect_pipe( pPipePath ) ) >= 0 )
        {
            // Wait for answer
            char resp[ strlen( "InternalIPC::SendArguments" ) + 1];
            ssize_t n = read( fd, resp, SAL_N_ELEMENTS( resp ) );
            if (n == (ssize_t) SAL_N_ELEMENTS( resp )
                && (memcmp(
                resp, "InternalIPC::SendArguments",
                SAL_N_ELEMENTS( resp ) - 1) == 0)) {
                rtl_uString *pCwdPath = NULL;
                osl_getProcessWorkingDir( &pCwdPath );

                // Then send args
                bSentArgs = send_args( fd, pCwdPath );
           }

            close( fd );
        }
    }

    if ( !bSentArgs )
    {
        /* we have to prepare for, and exec the binary */
        int nPercent = 0;
        ChildInfo *info;
        sal_Bool bAllArgs = sal_True;
        sal_Bool bShortWait, bRestart;

        /* sanity check pieces */
        system_checks();

        /* load splash image and create window */
        if ( !args->bInhibitSplash )
        {
            splash = splash_create(args->pAppPath, argc, argv);
        }

        /* pagein */
        if (!args->bInhibitPagein)
            exec_pagein (args);

        /* javaldx */
#if HAVE_FEATURE_JAVA
        if (!args->bInhibitJavaLdx)
            exec_javaldx (args);
#endif

        do
        {
            bRestart = sal_False;

            /* fast updates if we have somewhere to update it to */
            bShortWait = splash ? sal_True : sal_False;

            /* Periodically update the splash & the percent according
               to what status_fd says, poll quickly only while starting */
            info = child_spawn (args, bAllArgs, bShortWait);
            g_pProcess = info->child;
            while (!child_exited_wait (info, bShortWait))
            {
                ProgressStatus eResult;

                splash_draw_progress( splash, nPercent );
                eResult = read_percent( info, &nPercent );
                if (eResult != ProgressContinue)
                {
                    splash_destroy(splash);
                    splash = NULL;
                    bShortWait = sal_False;
                }

            }


            status = child_get_exit_code(info);
            g_pProcess = NULL; // reset
            switch (status) {
            case EXITHELPER_CRASH_WITH_RESTART: // re-start with just -env: parameters
                bRestart = sal_True;
                bAllArgs = sal_False;
                break;
            case EXITHELPER_NORMAL_RESTART: // re-start with all arguments
                bRestart = sal_True;
                bAllArgs = sal_True;
                break;
            default:
                break;
            }

            child_info_destroy (info);
        } while (bRestart);
    }

    /* cleanup */
    if ( pPipePath )
        rtl_uString_release( pPipePath );
    args_free (args);

    return status;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
