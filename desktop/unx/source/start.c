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

#include <osl/nlsupport.h>
#include <osl/process.h>
#include <osl/thread.h>
#include <rtl/bootstrap.h>
#include <rtl/digest.h>
#include <rtl/ustrbuf.h>
#include <sal/main.h>

#include "splashx.h"

/*
 * magic argument - if passed, not passed onto soffice.bin but we exit
 * immediately if we fail to control the process. Used to avoid doing
 * an un-conditional pagein
 */
#define QSEND_AND_REPORT "-qsend-and-report"

#define IMG_SUFFIX       ".png"
#define PIPEDEFAULTPATH      "/tmp"
#define PIPEALTERNATEPATH    "/var/tmp"

typedef enum { ProgressContinue, ProgressRestart, ProgressExit } ProgressStatus;

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

        if ( rtl_uString_getLength( pTmp ) == 0 ||
             !rtl_ustr_ascii_compare( pTmp->buffer, QSEND_AND_REPORT ) )
            continue;

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
    sal_Int32 nIndex = 0;
    int i = 0;

    /* get the value from the ini file */
    rtl_uString_newFromAscii( &pKey, name );
    rtl_bootstrap_get_from_handle( handle, pKey, &pValue, NULL );

    /* the value is several numbers delimited by ',' - parse it */
    if ( rtl_uString_getLength( pValue ) > 0 )
    {
        rtl_uString *pToken = NULL;

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
load_splash_defaults( rtl_uString *pAppPath, sal_Bool *pInhibitSplash )
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
        *pInhibitSplash = sal_True;

    splash_setup( bar, frame, pos[0], pos[1], size[0], size[1] );

    /* cleanup */
    rtl_bootstrap_args_close( handle );
    rtl_uString_release( pSettings );
    rtl_uString_release( pTmp );
}

#define BUFFER_LEN 255

/* Read the percent to show in splash. */
static ProgressStatus
read_percent( int status_fd, int *pPercent )
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
    nRead = read( status_fd, pBuffer + nNotProcessed, BUFFER_LEN - nNotProcessed );
    if ( nRead < 0 )
        return sal_False;

    nRead += nNotProcessed;
    pBuffer[nRead] = '\0';

    /* skip old data */
    pBegin = pBuffer;
    pNext = pBuffer;
    for ( pIter = pBuffer; *pIter; ++pIter )
        if ( *pIter == '\n' )
        {
            pBegin = pNext;
            pNext = pIter + 1;
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

/* Periodically update the splash & the percent acconding to what
   status_fd says */
static ProgressStatus
show_splash( int status_fd )
{
    int nRetval;
    struct pollfd aPfd;

    int nPercent = 0;
    sal_Bool bFinish = sal_False;
    ProgressStatus eResult;

    /* we want to watch status_fd */
    aPfd.fd = status_fd;
    aPfd.events = POLLIN;

#if OSL_DEBUG_LEVEL > 0
    fprintf( stderr, "Starting main loop, status fd: %d\n", status_fd );
#endif

    /* main loop */
    do {
        splash_draw_progress( nPercent );

        /* read from pipe if data available */
        nRetval = poll( &aPfd, 1, 50 );
        if ( aPfd.revents & ( POLLERR | POLLHUP | POLLNVAL ) )
            bFinish = sal_True;
        else if ( nRetval > 0 )
        {
            eResult = read_percent( status_fd, &nPercent );
            bFinish = ( eResult != ProgressContinue );
        }
        else if ( nRetval < 0 )
            bFinish = sal_True;
    } while ( !bFinish );

#if OSL_DEBUG_LEVEL > 0
    fprintf( stderr, "Finishing, result is %s\n",
            ( eResult == ProgressContinue )? "continue" : ( ( eResult == ProgressRestart )? "restart" : "exit" ) );
#endif
    return eResult;
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
        fprintf( stderr, "ERROR: /proc not mounted - OO.o is unlikely to work well if at all" );
        exit( 1 );
    }
#endif
}

/* Start the OOo application */
static sal_Bool
fork_app( rtl_uString *pAppPath, int *status_fd )
{
    rtl_uString *pApp = NULL, *pTmp = NULL, *pArg = NULL;
    rtl_uString **ppArgs;
    sal_uInt32 nArgs, i;

    oslProcess aProcess;
    oslProcessError nError;
    int status_pipe[2];

    system_checks();

    /* application name */
    rtl_uString_newFromAscii( &pApp, "file://" );
    rtl_uString_newConcat( &pApp, pApp, pAppPath );
    rtl_uString_newFromAscii( &pTmp, "/soffice.bin" );
    rtl_uString_newConcat( &pApp, pApp, pTmp );

    rtl_uString_new( &pTmp );

    /* copy args */
    nArgs = osl_getCommandArgCount();
    ppArgs = (rtl_uString **)calloc( nArgs + 1, sizeof( rtl_uString* ) );
    for ( i = 0; i < nArgs; ++i )
    {
        ppArgs[i] = NULL;
        osl_getCommandArg( i, &pTmp );
        rtl_uString_newFromString( &(ppArgs[i]), pTmp );
    }

    /* create pipe */
    if ( pipe( status_pipe ) < 0 )
    {
        fprintf( stderr, "ERROR: no file handles\n");
        exit( 1 );
    }

    /* add the pipe arg */
    sal_Unicode pUnicode[RTL_USTR_MAX_VALUEOFINT32];
    rtl_ustr_valueOfInt32( pUnicode, status_pipe[1], 10 );

    rtl_uString_newFromAscii( &pArg, "-splash-pipe=" );
    rtl_uString_newFromStr( &pTmp, pUnicode );
    rtl_uString_newConcat( &pArg, pArg, pTmp );

    ppArgs[nArgs] = NULL;
    rtl_uString_newFromString( &(ppArgs[nArgs]), pArg );
    ++nArgs;

    /* start the OOo process */
    nError = osl_executeProcess( pApp, ppArgs, nArgs,
            osl_Process_DETACHED | osl_Process_NORMAL,
            NULL,
            NULL,
            NULL, 0,
            &aProcess );

    *status_fd = status_pipe[0];
    close( status_pipe[1] );

    if ( nError != osl_Process_E_None )
    {
        fprintf( stderr, "ERROR %d forking process", nError );
        ustr_debug( "", pApp );
        return sal_False;
    }

    return sal_True;
}

/* Check if 'pArg' is -pCmpWith or --pCmpWith */
static sal_Bool
arg_check( rtl_uString *pArg, const char *pCmpWith )
{
    sal_Unicode *pUnicode = rtl_uString_getStr( pArg );

    if ( pUnicode[0] == (sal_Unicode)'-' )
        pUnicode++;
    else
        return sal_False;

    /* tolerate -- prefixes etc. */
    if ( pUnicode[0] == (sal_Unicode)'-' )
        pUnicode++;

    return !rtl_ustr_ascii_compare( pUnicode, pCmpWith );
}

static const char *ppInhibit[] = {
    "nologo", "headless", "invisible", "help", "h", "?", "minimized",
    NULL };
static const char *ppTwoArgs[] = {
    "pt", "display",
    NULL };

/* Read command line parameters and return whether we display the splash. */
static sal_Bool
get_inhibit_splash()
{
    rtl_uString *pTmp = NULL;
    sal_Bool bSkipNextArg = sal_False;
    const char **ppIter;

    rtl_uString_new( &pTmp );

    sal_uInt32 nArg;
    sal_uInt32 nArgCount = osl_getCommandArgCount();
    for ( nArg = 0; nArg < nArgCount; ++nArg )
    {
        if ( bSkipNextArg )
        {
            bSkipNextArg = sal_False;
            continue;
        }

        osl_getCommandArg( nArg, &pTmp );

        /* check for inhibit splash params */
        for ( ppIter = ppInhibit; *ppIter; ++ppIter )
        {
            if ( arg_check( pTmp, *ppIter ) )
            {
                rtl_uString_release( pTmp );
                return sal_True;
            }
        }
        /* check for 2 arguments params */
        for ( ppIter = ppTwoArgs; *ppIter; ++ppIter )
        {
            if ( arg_check( pTmp, *ppIter ) )
            {
                bSkipNextArg = sal_True;
                break;
            }
        }
    }

    /* cleanup */
    rtl_uString_release( pTmp );

    return sal_False;
}

SAL_IMPLEMENT_MAIN_WITH_ARGS( argc, argv )
{
    int fd = 0, status_fd = 0;
    sal_Bool bInhibitSplash, bSendAndReport;
    sal_Bool bSentArgs = sal_False;
    rtl_uString *pAppPath = NULL;
    rtl_uString *pPipePath = NULL;
    ProgressStatus eResult = ProgressExit;

    /* turn SIGPIPE into an error */
    signal( SIGPIPE, SIG_IGN );

    bInhibitSplash = get_inhibit_splash();

    pAppPath = get_app_path( argv[0] );
    if ( !pAppPath )
    {
        fprintf( stderr, "ERROR: Can't read app link\n" );
        exit( 1 );
    }
    ustr_debug( "App path", pAppPath );

    bSendAndReport = argc > 1 && !strcmp (argv[1], QSEND_AND_REPORT);

    pPipePath = get_pipe_path( pAppPath );

    if ( ( fd = connect_pipe( pPipePath ) ) >= 0 )
    {
        rtl_uString *pCwdPath = NULL;
        osl_getProcessWorkingDir( &pCwdPath );

        bSentArgs = send_args( fd, pCwdPath );
    }
#if OSL_DEBUG_LEVEL > 0
    else
        ustr_debug( "Failed to connect to pipe", pPipePath );
#endif

    if ( !bSendAndReport && !bSentArgs )
    {
        /* we have to exec the binary */
        do {
            if ( !fork_app( pAppPath, &status_fd ) )
                return 1;

            if ( !bInhibitSplash )
            {
                load_splash_image( pAppPath );
                load_splash_defaults( pAppPath, &bInhibitSplash );
            }

            if ( !bInhibitSplash && splash_create_window( argc, argv ) )
            {
                splash_draw_progress( 0 );
                eResult = show_splash( status_fd );
                splash_close_window();
            }

            close( status_fd );
        } while ( eResult == ProgressRestart );
    }

    /* cleanup */
    rtl_uString_release( pAppPath );
    rtl_uString_release( pPipePath );

    close( fd );

    return bSendAndReport? !bSentArgs : 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
