/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *               Novell, Inc.
 * Portions created by the Initial Developer are Copyright (C) 2010 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s): Jan Holesovsky <kendy@novell.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */
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

#include <osl/nlsupport.h>
#include <osl/process.h>
#include <osl/thread.h>
#include <rtl/bootstrap.h>
#include <rtl/digest.h>
#include <rtl/ustrbuf.h>
#include <sal/main.h>

#include "args.h"
#include "splashx.h"

#define IMG_SUFFIX           ".png"
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

/* Easier debugging of rtl_uString values. */
#if OSL_DEBUG_LEVEL > 0
static void
ustr_debug( const char *pMessage, rtl_uString *pStr )
{
    rtl_String *pOut = ustr_to_str( pStr );

    fprintf( stderr, "%s: %s\n", pMessage, rtl_string_getStr( pOut ) );

    rtl_string_release( pOut );
    return;
}
#else
#define ustr_debug( a, b ) {}
#endif

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
    char buffer[64];
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
    rtl_uString_newFromAscii( &pTmp, "/soffice.bin" );
    rtl_uString_newConcat( &pApp, pApp, pTmp );

    rtl_uString_new( &pTmp );

    /* copy args */
    nArgs = bAllArgs ? args->nArgsTotal : args->nArgsEnv;
    ppArgs = (rtl_uString **)calloc( nArgs + 1, sizeof( rtl_uString* ) );
    for ( i = 0; i < nArgs; ++i )
        ppArgs[i] = args->ppArgs[i];

    if( bWithStatus )
    {
        /* add the pipe arg */
        snprintf (buffer, 63, "--splash-pipe=%d", status_pipe[1]);
        ppArgs[nArgs] = NULL;
        rtl_uString_newFromAscii( &ppArgs[nArgs], buffer );
        ++nArgs;
    }

    /* start the main process */
    nError = osl_executeProcess( pApp, ppArgs, nArgs,
                                 osl_Process_NORMAL,
                                 NULL,
                                 NULL,
                                 NULL, 0,
                                 &info->child );

    if ( nError != osl_Process_E_None )
    {
        fprintf( stderr, "ERROR %d forking process", nError );
        ustr_debug( "", pApp );
        _exit (1);
    }

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

/* Path of the application. */
static rtl_uString *
get_app_path( const char *pAppExec )
{
    char pRealPath[PATH_MAX];
    rtl_uString *pResult;

    char *pPath = strdup( pAppExec );
    pPath = dirname( pPath );

    realpath( pPath, pRealPath );
    pResult = charp_to_ustr( pRealPath );
    free( pPath );

    return pResult;
}

/* Compute the OOo md5 hash from 'pText' */
static rtl_uString *
get_md5hash( rtl_uString *pText )
{
    rtl_uString *pResult = NULL;
    sal_Int32 nCapacity = 100;

#if OSL_DEBUG_LEVEL > 0
    fprintf (stderr, "Generate pipe md5 for '%s'\n", ustr_to_str (pText)->buffer);
#endif

    if ( !pText )
        return NULL;

    unsigned char *pData = (unsigned char *)rtl_uString_getStr( pText );
    sal_uInt32   nSize = rtl_uString_getLength( pText ) * sizeof( sal_Unicode );
    if ( !pData )
        return NULL;

    rtlDigest digest = rtl_digest_create( rtl_Digest_AlgorithmMD5 );
    if ( digest == 0 )
        return NULL;

    sal_uInt32 md5_key_len = rtl_digest_queryLength( digest );
    sal_uInt8 *md5_buf = (sal_uInt8 *)calloc( md5_key_len, sizeof( sal_uInt8 ) );

    rtl_digest_init( digest, pData , nSize );
    rtl_digest_update( digest, pData, nSize );
    rtl_digest_get( digest, md5_buf, md5_key_len );
    rtl_digest_destroy( digest );

    /* create hex-value string from the MD5 value to keep
       the string size minimal */
    rtl_uString_new_WithLength( &pResult, nCapacity );
    sal_uInt32 i = 0;
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

    /* setup bootstrap filename */
    rtl_uString_newFromAscii( &pPath, "file://" );
    rtl_uString_newConcat( &pPath, pPath, pAppPath );
    rtl_uString_newFromAscii( &pTmp, "/" );
    rtl_uString_newConcat( &pPath, pPath, pTmp );
    rtl_uString_newFromAscii( &pTmp, SAL_CONFIGFILE( "bootstrap" ) );
    rtl_uString_newConcat( &pPath, pPath, pTmp );

    ustr_debug( "bootstap", pPath );

    /* read userinstallation value */
    rtlBootstrapHandle handle = rtl_bootstrap_args_open( pPath );

    rtl_uString_newFromAscii( &pTmp, "UserInstallation" );
    rtl_bootstrap_get_from_handle( handle, pTmp, &pUserInstallation, NULL );

    rtl_bootstrap_args_close( handle );

    /* turn it into an absolute path - unwinding symlinks etc. */
    if ( osl_getProcessWorkingDir (&pBasePath) ||
         osl_getAbsoluteFileURL( pBasePath, pUserInstallation, &pAbsUserInstallation ) )
        rtl_uString_newFromString (&pAbsUserInstallation, pUserInstallation);

    /* create the pipe name */
    ustr_debug( "user installation", pAbsUserInstallation );
    rtl_uString *pMd5hash = get_md5hash( pAbsUserInstallation );
    if ( !pMd5hash )
        rtl_uString_new( &pMd5hash );

    if ( access( PIPEDEFAULTPATH, R_OK|W_OK ) == 0 )
        rtl_uString_newFromAscii( &pResult, PIPEDEFAULTPATH );
    else
        rtl_uString_newFromAscii( &pResult, PIPEALTERNATEPATH );

    rtl_uString_newFromAscii( &pTmp, "/OSL_PIPE_" );
    rtl_uString_newConcat( &pResult, pResult, pTmp );

    sal_Unicode pUnicode[RTL_USTR_MAX_VALUEOFINT32];
    rtl_ustr_valueOfInt32( pUnicode, (int)getuid(), 10 );
    rtl_uString_newFromStr( &pTmp, pUnicode );
    rtl_uString_newConcat( &pResult, pResult, pTmp );

    rtl_uString_newFromAscii( &pTmp, "_SingleOfficeIPC_" );
    rtl_uString_newConcat( &pResult, pResult, pTmp );

    rtl_uString_newConcat( &pResult, pResult, pMd5hash );

    ustr_debug( "result", pResult );

    /* cleanup */
    rtl_uString_release( pPath );
    rtl_uString_release( pTmp );
    rtl_uString_release( pBasePath );
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

    fcntl( fd, F_SETFD, FD_CLOEXEC );

    addr.sun_family = AF_UNIX;
    strncpy( addr.sun_path, rtl_string_getStr( pPipeStr ), sizeof( addr.sun_path ) );
    rtl_string_release( pPipeStr );

/* cut / paste from osl's pipe.c */
#if defined(FREEBSD)
    len = SUN_LEN( &addr );
#else
    len = sizeof( addr );
#endif

    if ( connect( fd, (struct sockaddr *)&addr, len ) < 0 )
        return -1;

    return fd;
}

/* Escape: "," -> "\\,", "\0" -> "\\0", "\\" -> "\\\\" */
static rtl_uString *
escape_path( rtl_uString *pToEscape )
{
    rtl_uString *pBuffer = NULL;
    sal_Int32 nCapacity = 1000;

    rtl_uString_new_WithLength( &pBuffer, nCapacity );

    sal_Int32 i = 0;
    sal_Int32 nEscapeLength = rtl_uString_getLength( pToEscape );
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
    rtl_uStringbuffer_insert_ascii( &pBuffer, &nCapacity,
        rtl_uString_getLength( pBuffer ),
        RTL_CONSTASCII_STRINGPARAM( "0" ) );

    sal_Bool bDontConvertNext = sal_False;
    sal_uInt32 nArg;
    sal_uInt32 nArgCount = osl_getCommandArgCount();
    for ( nArg = 0; nArg < nArgCount; ++nArg )
    {
        rtl_uStringbuffer_insert_ascii( &pBuffer, &nCapacity,
                rtl_uString_getLength( pBuffer ),
                ",", 1 );

        osl_getCommandArg( nArg, &pTmp );

        // this is not a param, we have to prepend filenames with file://
        // FIXME: improve the check
        if ( ( pTmp->buffer[0] != (sal_Unicode)'-' ) )
        {
            sal_Int32 nFirstColon = rtl_ustr_indexOfChar_WithLength( pTmp->buffer, pTmp->length, ':' );
            sal_Int32 nFirstSlash = rtl_ustr_indexOfChar_WithLength( pTmp->buffer, pTmp->length, '/' );

            // check that pTmp is not an URI yet
            // note ".uno" ".slot" & "vnd.sun.star.script" are special urls that
            // don't expect a following '/'

             const char* schemes[] = { "slot:",  ".uno:", "vnd.sun.star.script:" };
             sal_Bool bIsSpecialURL = sal_False;
             int index = 0;
             int len =  SAL_N_ELEMENTS(schemes);
             for ( ; index < len; ++index )
             {
                 if ( rtl_ustr_indexOfAscii_WithLength( pTmp->buffer
                     , pTmp->length , schemes[ index ], strlen(schemes[ index ] ))  == 0  )
                 {
                     bIsSpecialURL = sal_True;
                     break;
                 }
             }

            if ( !bIsSpecialURL && ( nFirstColon < 1 || ( nFirstSlash != nFirstColon + 1 ) ) )
            {
                // some of the switches (currently just -pt) don't want to
                // have the filenames as URIs
                if ( !bDontConvertNext )
                    osl_getAbsoluteFileURL( pCwdPath, pTmp, &pTmp );
            }
        }

        // don't convert filenames with some of the switches
        // (currently just -pt)
        bDontConvertNext = !rtl_ustr_ascii_compareIgnoreAsciiCase( pTmp->buffer, "-pt" );

        rtl_uString *pEscapedTmp = escape_path( pTmp );

        rtl_uStringbuffer_insert( &pBuffer, &nCapacity,
                rtl_uString_getLength( pBuffer ),
                rtl_uString_getStr( pEscapedTmp ),
                rtl_uString_getLength( pEscapedTmp ) );

        rtl_uString_release( pEscapedTmp );
    }

    ustr_debug( "Pass args", pBuffer );

    pOut = ustr_to_str( pBuffer );

    nLen = rtl_string_getLength( pOut ) + 1;
    bResult = ( write( fd, rtl_string_getStr( pOut ), nLen ) == (ssize_t) nLen );

    /* cleanup */
    rtl_uString_release( pEscapedCwdPath );
    rtl_uString_release( pBuffer );
    rtl_uString_release( pTmp );
    rtl_string_release( pOut );

    return bResult;
}

static void
load_splash_image( rtl_uString *pUAppPath )
{
    char *pBuffer, *pSuffix, *pLocale;
    int nLocSize;
    rtl_Locale *pLoc = NULL;
    rtl_String *pLang, *pCountry, *pAppPath;

    osl_getProcessLocale (&pLoc);
    pLang = ustr_to_str (pLoc->Language);
    pCountry = ustr_to_str (pLoc->Country);

    nLocSize = strlen (pLang->buffer) + strlen (pCountry->buffer) + 8;
    pLocale = malloc (nLocSize);
    pLocale[0] = '-';
    strcpy (pLocale + 1, pLang->buffer);
    strcat (pLocale, "_");
    strcat (pLocale, pCountry->buffer);

    pAppPath = ustr_to_str (pUAppPath);
    pBuffer = malloc (pAppPath->length + nLocSize + 256);
    strcpy (pBuffer, pAppPath->buffer);
    pSuffix = pBuffer + pAppPath->length;

    strcpy (pSuffix, "/edition/intro");
    strcat (pSuffix, pLocale);
    strcat (pSuffix, IMG_SUFFIX);
    if ( splash_load_bmp( pBuffer ) )
        goto cleanup;

    strcpy (pSuffix, "/edition/intro" IMG_SUFFIX);
    if ( splash_load_bmp( pBuffer ) )
        goto cleanup;

    strcpy (pSuffix, "/intro");
    strcat (pSuffix, pLocale);
    strcat (pSuffix, IMG_SUFFIX);
    if ( splash_load_bmp( pBuffer ) )
        goto cleanup;

    strcpy (pSuffix, "/intro" IMG_SUFFIX);
    if ( splash_load_bmp( pBuffer ) )
        goto cleanup;

    fprintf (stderr, "Failed to find intro image\n");

 cleanup:
    free (pLocale);
    free (pBuffer);
}

/* Fill 'array' with values of the key 'name'.
   Its value is a comma delimited list of integers */
static void
get_bootstrap_value( int *array, int size, rtlBootstrapHandle handle, const char *name )
{
    rtl_uString *pKey = NULL, *pValue = NULL;

    /* get the value from the ini file */
    rtl_uString_newFromAscii( &pKey, name );
    rtl_bootstrap_get_from_handle( handle, pKey, &pValue, NULL );

    /* the value is several numbers delimited by ',' - parse it */
    if ( rtl_uString_getLength( pValue ) > 0 )
    {
        rtl_uString *pToken = NULL;
        int i = 0;
        sal_Int32 nIndex = 0;
        for ( ; ( nIndex >= 0 ) && ( i < size ); ++i )
        {
            nIndex = rtl_uString_getToken( &pToken, pValue, 0, ',', nIndex );
            array[i] = rtl_ustr_toInt32( rtl_uString_getStr( pToken ), 10 );
        }

        rtl_uString_release( pToken );
    }

    /* cleanup */
    rtl_uString_release( pKey );
    rtl_uString_release( pValue );
}

/* Load the colors and size of the splash. */
static void
load_splash_defaults( rtl_uString *pAppPath, sal_Bool *bNoDefaults )
{
    rtl_uString *pSettings = NULL, *pTmp = NULL;
    rtlBootstrapHandle handle;

    /* costruct the sofficerc file location */
    rtl_uString_newFromAscii( &pSettings, "file://" );
    rtl_uString_newConcat( &pSettings, pSettings, pAppPath );
    rtl_uString_newFromAscii( &pTmp, "/" );
    rtl_uString_newConcat( &pSettings, pSettings, pTmp );
    rtl_uString_newFromAscii( &pTmp, SAL_CONFIGFILE( "soffice" ) );
    rtl_uString_newConcat( &pSettings, pSettings, pTmp );

    /* use it as the bootstrap file */
    handle = rtl_bootstrap_args_open( pSettings );

    int logo[1] =  { -1 },
        bar[3] =   { -1, -1, -1 },
        frame[3] = { -1, -1, -1 },
        pos[2] =   { -1, -1 },
        size[2] =  { -1, -1 };

    /* get the values */
    get_bootstrap_value( logo,  1, handle, "Logo" );
    get_bootstrap_value( bar,   3, handle, "ProgressBarColor" );
    get_bootstrap_value( frame, 3, handle, "ProgressFrameColor" );
    get_bootstrap_value( pos,   2, handle, "ProgressPosition" );
    get_bootstrap_value( size,  2, handle, "ProgressSize" );

    if ( logo[0] == 0 )
        *bNoDefaults = sal_True;

    splash_setup( bar, frame, pos[0], pos[1], size[0], size[1] );

    /* cleanup */
    rtl_bootstrap_args_close( handle );
    rtl_uString_release( pSettings );
    rtl_uString_release( pTmp );
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

    /* from the last call */
    int nNotProcessed = nRead - ( pNext - pBuffer );
    if ( nNotProcessed >= BUFFER_LEN )
        return sal_False;

    memmove( pBuffer, pNext, nNotProcessed );

    /* read data */
    nRead = read( child_info_get_status_fd (info),
                  pBuffer + nNotProcessed, BUFFER_LEN - nNotProcessed );
    if ( nRead < 0 )
        return sal_False;

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

#if OSL_DEBUG_LEVEL > 0
    fprintf( stderr, "Got status: %s\n", pBegin );
#endif
    if ( !strncasecmp( pBegin, "end", 3 ) )
        return ProgressExit;
    else if ( !strncasecmp( pBegin, "restart", 7 ) )
        return ProgressRestart;
    else if ( sscanf( pBegin, "%d%%", pPercent ) )
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
        fprintf( stderr, "ERROR: /proc not mounted - LibreOffice is unlikely to work well if at all" );
        exit( 1 );
    }
#endif
}

/* re-use the pagein code */
extern int pagein_execute (int argc, char **argv);

void
exec_pagein (Args *args)
{
    char *argv[5];
    rtl_String *app_path;

    app_path = ustr_to_str (args->pAppPath);

    argv[0] = "dummy-pagein";
    argv[1] = malloc (app_path->length + sizeof ("-L/../basis-link/program") + 2);
    strcpy (argv[1], "-L");
    strcat (argv[1], app_path->buffer);
    strcat (argv[1], "/../basis-link/program");
    argv[2] = "@pagein-common";
    argv[3] = (char *)args->pPageinType;
    argv[4] = NULL;

    pagein_execute (args->pPageinType ? 4 : 3, argv);

    free (argv[1]);
}

static void extend_library_path (const char *new_element)
{
    const char *pathname;
#ifdef AIX
    pathname = "LIBPATH";
#else
    pathname = "LD_LIBRARY_PATH";
#endif
    char *buffer;
    char *oldpath;

    oldpath = getenv (pathname);
    buffer = malloc (strlen (new_element) + strlen (pathname) +
                     (oldpath ? strlen (oldpath) : 0)+ 4);
    strcpy (buffer, pathname);
    strcpy (buffer, "=");
    strcpy (buffer, new_element);
    if (oldpath) {
        strcat (buffer, ":");
        strcat (buffer, oldpath);
    }

    /* deliberately leak buffer - many OS' don't dup at this point */
    putenv (buffer);
}

static void
exec_javaldx (Args *args)
{
    char newpath[4096];
    sal_uInt32 nArgs;
    rtl_uString *pApp;
    rtl_uString **ppArgs;
    rtl_uString *pTmp, *pTmp2;
    rtl_uString *pEnvironment[1] = { NULL };

    ppArgs = (rtl_uString **)calloc( args->nArgsEnv + 2, sizeof( rtl_uString* ) );

    for ( nArgs = 0; nArgs < args->nArgsEnv; ++nArgs )
        ppArgs[nArgs] = args->ppArgs[nArgs];

    /* Use absolute path to redirectrc */
    pTmp = NULL;
    rtl_uString_newFromAscii( &pTmp, "-env:INIFILENAME=vnd.sun.star.pathname:" );
    rtl_uString_newConcat( &pTmp, pTmp, args->pAppPath );
    pTmp2 = NULL;
    rtl_uString_newFromAscii( &pTmp2, "/redirectrc" );
    rtl_uString_newConcat( &pTmp, pTmp, pTmp2 );
    ppArgs[nArgs] = pTmp;
    rtl_uString_release (pTmp2);
    nArgs++;

    oslProcess javaldx = NULL;
    oslFileHandle fileOut= 0;
    oslProcessError err;

    /* And also to javaldx */
    pApp = NULL;
    rtl_uString_newFromAscii( &pApp, "file://" );
    rtl_uString_newConcat( &pApp, pApp, args->pAppPath );
    pTmp = NULL;
    rtl_uString_newFromAscii( &pTmp, "/../ure/bin/javaldx" );
    rtl_uString_newConcat( &pApp, pApp, pTmp );

    /* unset to avoid bogus console output */
    rtl_uString_newFromAscii( &pEnvironment[0], "G_SLICE" );

    err = osl_executeProcess_WithRedirectedIO( pApp, ppArgs, nArgs,
                                               osl_Process_NORMAL,
                                               NULL, // security
                                               NULL, // work dir
                                               pEnvironment, 1,
                                               &javaldx, // process handle
                                               NULL,
                                               &fileOut,
                                               NULL);

    if( err != osl_Process_E_None)
    {
        fprintf (stderr, "Warning: failed to launch javaldx - java may not fuction correctly\n");
        return;
    } else {
        char *chomp;
        sal_uInt64 bytes_read;

        /* Magically osl_readLine doesn't work with pipes with E_SPIPE - so be this lame instead: */
        while (osl_readFile (fileOut, newpath, SAL_N_ELEMENTS (newpath), &bytes_read) == osl_File_E_INTR);

        if (bytes_read <= 0) {
            fprintf (stderr, "Warning: failed to read path from javaldx\n");
            return;
        }
        newpath[bytes_read] = '\0';

        if ((chomp = strstr (newpath, "\n")))
            *chomp = '\0';
    }

#if OSL_DEBUG_LEVEL > 0
    fprintf (stderr, "Adding javaldx path of '%s'\n", newpath);
#endif
    extend_library_path (newpath);

    osl_freeProcessHandle(javaldx);
}

SAL_IMPLEMENT_MAIN_WITH_ARGS( argc, argv )
{
    int fd = 0;
    sal_Bool bSentArgs = sal_False;
    rtl_uString *pPipePath = NULL;
    Args *args;

    /* turn SIGPIPE into an error */
    signal( SIGPIPE, SIG_IGN );

    args = args_parse ();
    args->pAppPath = get_app_path( argv[0] );
    if ( !args->pAppPath )
    {
        fprintf( stderr, "ERROR: Can't read app link\n" );
        exit( 1 );
    }
    ustr_debug( "App path", args->pAppPath );

#ifndef ENABLE_QUICKSTART_LIBPNG
    /* we can't load and render it anyway */
    args->bInhibitSplash = sal_True;
#endif

    pPipePath = get_pipe_path( args->pAppPath );

    if ( ( fd = connect_pipe( pPipePath ) ) >= 0 )
    {
        rtl_uString *pCwdPath = NULL;
        osl_getProcessWorkingDir( &pCwdPath );

        bSentArgs = send_args( fd, pCwdPath );

        close( fd );
    }
#if OSL_DEBUG_LEVEL > 0
    else
        ustr_debug( "Failed to connect to pipe", pPipePath );
#endif

    if ( !bSentArgs )
    {
        /* we have to prepare for, and exec the binary */
        int nPercent = 0;
        ChildInfo *info;
        sal_Bool bAllArgs = sal_True;
        sal_Bool bHaveWindow = sal_False;
        sal_Bool bShortWait, bRestart;

        /* sanity check pieces */
        system_checks();

        /* load splash image and create window */
        if ( !args->bInhibitSplash )
        {
            sal_Bool bNoDefaults = sal_False;
            load_splash_image( args->pAppPath );
            load_splash_defaults( args->pAppPath, &bNoDefaults );

            if (!bNoDefaults &&
                ( bHaveWindow = splash_create_window( argc, argv ) ) )
                splash_draw_progress( 0 );
        }

        /* pagein */
        if (!args->bInhibitJavaLdx)
            exec_pagein (args);

        /* javaldx */
        if (!args->bInhibitJavaLdx)
            exec_javaldx (args);

        do {
            bRestart = sal_False;

            /* fast updates if we have somewhere to update it to */
            bShortWait = bHaveWindow;

            /* Periodically update the splash & the percent according
               to what status_fd says, poll quickly only while starting */
            info = child_spawn (args, bAllArgs, bShortWait);
            while (!child_exited_wait (info, bShortWait))
            {
                ProgressStatus eResult;

                splash_draw_progress( nPercent );
                eResult = read_percent( info, &nPercent );
                if (eResult != ProgressContinue)
                {
                    splash_close_window ();
                    bShortWait = sal_False;
                }

#if OSL_DEBUG_LEVEL > 0
                fprintf( stderr, "Polling, result is %s\n",
                         ( eResult == ProgressContinue )? "continue" :
                         ( ( eResult == ProgressRestart )? "restart" : "exit" ) );
#endif
            }

#if OSL_DEBUG_LEVEL > 0
            fprintf (stderr, "Exited with code '%d'\n", child_get_exit_code (info));
#endif

            switch (child_get_exit_code (info)) {
            case 79: // re-start with just -env: parameters
                fprintf (stderr, "FIXME: re-start with just -env: params !\n");
                bRestart = sal_True;
                bAllArgs = sal_False;
                break;
            case 81: // re-start with all arguments
                fprintf (stderr, "FIXME: re-start with all params !\n");
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
    rtl_uString_release( pPipePath );
    args_free (args);

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
