/*************************************************************************
 *
 *  $RCSfile: file_url.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: tra $ $Date: 2002-11-29 10:37:46 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _LIMITS_H
#include <limits.h>
#endif

#ifndef _ERRNO_H
#include <errno.h>
#endif

#ifndef _STRINGS_H
#include <strings.h>
#endif

#ifndef _UNISTD_H
#include <unistd.h>
#endif

#ifndef _OSL_FILE_H_
#include <osl/file.h>
#endif

#ifndef _OSL_SECURITY_H_
#include <osl/security.h>
#endif

#ifndef _RTL_URI_H_
#include <rtl/uri.h>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _RTL_USTRBUF_H_
#include <rtl/ustrbuf.h>
#endif

#ifndef _OSL_TREAD_H_
#include <osl/thread.h>
#endif

#ifndef _FILE_ERROR_TRANSL_H_
#include "file_error_transl.h"
#endif

#ifndef _FILE_URL_H_
#include "file_url.h"
#endif

 #ifndef _OSL_FILE_PATH_HELPER_H_
 #include "file_path_helper.h"
 #endif

/***************************************************
 * forward
 **************************************************/

extern "C" int UnicodeToText(char *, size_t, const sal_Unicode *, sal_Int32);

/***************************************************
 * constants
 **************************************************/

const sal_Unicode OSL_FILE_CHAR_SLASH = ((sal_Unicode)'/');
const sal_Unicode OSL_FILE_CHAR_COLON = ((sal_Unicode)':');
const sal_Unicode OSL_FILE_CHAR_DOT   = ((sal_Unicode)'.');

/******************************************************************************
 *
 *                  Exported Module Functions
 *
 *****************************************************************************/

/* a slightly modified version of Pchar in rtl/source/uri.c */
const sal_Bool uriCharClass[128] =
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* Pchar but without encoding slashes */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 1, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* !"#$%&'()*+,-./*/
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, /*0123456789:;<=>?*/
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /*@ABCDEFGHIJKLMNO*/
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, /*PQRSTUVWXYZ[\]^_*/
  0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /*`abcdefghijklmno*/
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0  /*pqrstuvwxyz{|}~ */
};


/* check for top wrong usage strings */
/*
static sal_Bool findWrongUsage( const sal_Unicode *path, sal_Int32 len )
{
    rtl_uString *pTmp = NULL;
    sal_Bool bRet;

    rtl_uString_newFromStr_WithLength( &pTmp, path, len );

    rtl_ustr_toAsciiLowerCase_WithLength( pTmp->buffer, pTmp->length );

    bRet = ( 0 == rtl_ustr_ascii_shortenedCompare_WithLength( pTmp->buffer, pTmp->length, "ftp://", 6 ) ) ||
           ( 0 == rtl_ustr_ascii_shortenedCompare_WithLength( pTmp->buffer, pTmp->length, "http://", 7 ) ) ||
           ( 0 == rtl_ustr_ascii_shortenedCompare_WithLength( pTmp->buffer, pTmp->length, "vnd.sun.star", 12 ) ) ||
           ( 0 == rtl_ustr_ascii_shortenedCompare_WithLength( pTmp->buffer, pTmp->length, "private:", 8 ) ) ||
           ( 0 == rtl_ustr_ascii_shortenedCompare_WithLength( pTmp->buffer, pTmp->length, "slot:", 5) );

    rtl_uString_release( pTmp );
    return bRet;
}
*/

/****************************************************************************/
/*  osl_getSystemPathFromFileURL */
/****************************************************************************/

oslFileError SAL_CALL osl_getSystemPathFromFileURL( rtl_uString *ustrFileURL, rtl_uString **pustrSystemPath )
{
    sal_Int32 nIndex = 0;
    rtl_uString * pTmp = NULL;

    sal_Unicode encodedSlash[3] = { '%', '2', 'F' };

    /* temporary hack: if already system path, return ustrFileURL */
    /*
    if( (sal_Unicode) '/' == ustrFileURL->buffer[0] )
    {
        OSL_ENSURE( 0, "osl_getSystemPathFromFileURL: input is already system path" );
        rtl_uString_assign( pustrSystemPath, ustrFileURL );
        return osl_File_E_None;
    }
    */

    /* a valid file url may not start with '/' */
    if( ( 0 == ustrFileURL->length ) || ( (sal_Unicode) '/' == ustrFileURL->buffer[0] ) )
    {
        return osl_File_E_INVAL;
    }

    /* search for encoded slashes (%2F) and decode every single token if we find one */
    if( -1 != rtl_ustr_indexOfStr_WithLength( ustrFileURL->buffer, ustrFileURL->length, encodedSlash, 3 ) )
    {
        rtl_uString * ustrPathToken = NULL;
        sal_Int32 nOffset = 7;

        do
        {
            nOffset += nIndex;

            /* break url down in '/' devided tokens tokens */
            nIndex = rtl_ustr_indexOfChar_WithLength( ustrFileURL->buffer + nOffset, ustrFileURL->length - nOffset, (sal_Unicode) '/' );

            /* copy token to new string */
            rtl_uString_newFromStr_WithLength( &ustrPathToken, ustrFileURL->buffer + nOffset,
                -1 == nIndex ? ustrFileURL->length - nOffset : nIndex++ );

            /* decode token */
            rtl_uriDecode( ustrPathToken, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8, &pTmp );

            /* the result should not contain any '/' */
            if( -1 != rtl_ustr_indexOfChar_WithLength( pTmp->buffer, pTmp->length, (sal_Unicode) '/' ) )
            {
                rtl_uString_release( pTmp );
                rtl_uString_release( ustrPathToken );

                return osl_File_E_INVAL;
            }

        } while( -1 != nIndex );

        /* release temporary string and restore index variable */
        rtl_uString_release( ustrPathToken );
        nIndex = 0;
    }

    /* protocol and server should not be encoded, so decode the whole string */
    rtl_uriDecode( ustrFileURL, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8, &pTmp );

    /* check if file protocol specified */
    /* FIXME: use rtl_ustr_ascii_shortenedCompareIgnoreCase_WithLength when available */
    if( 7 <= pTmp->length )
    {
        rtl_uString * pProtocol = NULL;
        rtl_uString_newFromStr_WithLength( &pProtocol, pTmp->buffer, 7 );

        /* protocol is case insensitive */
        rtl_ustr_toAsciiLowerCase_WithLength( pProtocol->buffer, pProtocol->length );

        if( 0 == rtl_ustr_ascii_shortenedCompare_WithLength( pProtocol->buffer, pProtocol->length,"file://", 7 ) )
            nIndex = 7;

        rtl_uString_release( pProtocol );
    }

    /* skip "localhost" or "127.0.0.1" if "file://" is specified */
    /* FIXME: use rtl_ustr_ascii_shortenedCompareIgnoreCase_WithLength when available */
    if( nIndex && ( 10 <= pTmp->length - nIndex ) )
    {
        rtl_uString * pServer = NULL;
        rtl_uString_newFromStr_WithLength( &pServer, pTmp->buffer + nIndex, 10 );

        /* server is case insensitive */
        rtl_ustr_toAsciiLowerCase_WithLength( pServer->buffer, pServer->length );

        if( ( 0 == rtl_ustr_ascii_shortenedCompare_WithLength( pServer->buffer, pServer->length,"localhost/", 10 ) ) ||
            ( 0 == rtl_ustr_ascii_shortenedCompare_WithLength( pServer->buffer, pServer->length,"127.0.0.1/", 10 ) ) )
        {
            /* don't exclude the '/' */
            nIndex += 9;
        }

        rtl_uString_release( pServer );
    }

    if( nIndex )
        rtl_uString_newFromStr_WithLength( &pTmp, pTmp->buffer + nIndex, pTmp->length - nIndex );

    /* check if system path starts with ~ or ~user and replace it with the appropriate home dir */
    if( (sal_Unicode) '~' == pTmp->buffer[0] )
    {
        /* check if another user is specified */
        if( ( 1 == pTmp->length ) || ( (sal_Unicode)'/' == pTmp->buffer[1] ) )
        {
            rtl_uString *pTmp2 = NULL;

            /* osl_getHomeDir returns file URL */
            osl_getHomeDir( osl_getCurrentSecurity(), &pTmp2 );

            /* remove "file://" prefix */
            rtl_uString_newFromStr_WithLength( &pTmp2, pTmp2->buffer + 7, pTmp2->length - 7 );

            /* replace '~' in original string */
            rtl_uString_newReplaceStrAt( &pTmp, pTmp, 0, 1, pTmp2 );
            rtl_uString_release( pTmp2 );
        }

        else
        {
            /* FIXME: replace ~user with users home directory */
            return osl_File_E_INVAL;
        }
    }

    /* temporary check for top 5 wrong usage strings (which are valid but unlikly filenames) */
    /*
    OSL_ASSERT( !findWrongUsage( pTmp->buffer, pTmp->length ) );
    */

    *pustrSystemPath = pTmp;
    return osl_File_E_None;
}

/****************************************************************************/
/*  osl_getFileURLFromSystemPath */
/****************************************************************************/

oslFileError SAL_CALL osl_getFileURLFromSystemPath( rtl_uString *ustrSystemPath, rtl_uString **pustrFileURL )
{
    static const sal_Unicode pDoubleSlash[2] = { '/', '/' };

    rtl_uString *pTmp = NULL;
    sal_Int32 nIndex;

    if( 0 == ustrSystemPath->length )
        return osl_File_E_INVAL;

    /* temporary hack: if already file url, return ustrSystemPath */

    if( 0 == rtl_ustr_ascii_shortenedCompare_WithLength( ustrSystemPath->buffer, ustrSystemPath->length,"file:", 5 ) )
    {
    /*
        if( 0 == rtl_ustr_ascii_shortenedCompare_WithLength( ustrSystemPath->buffer, ustrSystemPath->length,"file://", 7 ) )
        {
            OSL_ENSURE( 0, "osl_getFileURLFromSystemPath: input is already file URL" );
            rtl_uString_assign( pustrFileURL, ustrSystemPath );
        }
        else
        {
            rtl_uString *pTmp2 = NULL;

            OSL_ENSURE( 0, "osl_getFileURLFromSystemPath: input is wrong file URL" );
            rtl_uString_newFromStr_WithLength( pustrFileURL, ustrSystemPath->buffer + 5, ustrSystemPath->length - 5 );
            rtl_uString_newFromAscii( &pTmp2, "file://" );
            rtl_uString_newConcat( pustrFileURL, *pustrFileURL, pTmp2 );
            rtl_uString_release( pTmp2 );
        }
        return osl_File_E_None;
        */
        return osl_File_E_INVAL;
    }


    /* check if system path starts with ~ or ~user and replace it with the appropriate home dir */
    if( (sal_Unicode) '~' == ustrSystemPath->buffer[0] )
    {
        /* check if another user is specified */
        if( ( 1 == ustrSystemPath->length ) || ( (sal_Unicode)'/' == ustrSystemPath->buffer[1] ) )
        {
            /* osl_getHomeDir returns file URL */
            osl_getHomeDir( osl_getCurrentSecurity(), &pTmp );

            /* remove "file://" prefix */
            rtl_uString_newFromStr_WithLength( &pTmp, pTmp->buffer + 7, pTmp->length - 7 );

            /* replace '~' in original string */
            rtl_uString_newReplaceStrAt( &pTmp, ustrSystemPath, 0, 1, pTmp );
        }

        else
        {
            /* FIXME: replace ~user with users home directory */
            return osl_File_E_INVAL;
        }
    }

    /* check if initial string contains double instances of '/' */
    nIndex = rtl_ustr_indexOfStr_WithLength( ustrSystemPath->buffer, ustrSystemPath->length, pDoubleSlash, 2 );
    if( -1 != nIndex )
    {
        sal_Int32 nSrcIndex;
        sal_Int32 nDeleted = 0;

        /* if pTmp is not already allocated, copy ustrSystemPath for modification */
        if( NULL == pTmp )
            rtl_uString_newFromString( &pTmp, ustrSystemPath );

        /* adapt index to pTmp */
        nIndex += pTmp->length - ustrSystemPath->length;

        /* remove all occurances of '//' */
        for( nSrcIndex = nIndex + 1; nSrcIndex < pTmp->length; nSrcIndex++ )
        {
            if( ((sal_Unicode) '/' == pTmp->buffer[nSrcIndex]) && ((sal_Unicode) '/' == pTmp->buffer[nIndex]) )
                nDeleted++;
            else
                pTmp->buffer[++nIndex] = pTmp->buffer[nSrcIndex];
        }

        /* adjust length member */
        pTmp->length -= nDeleted;
    }

    if( NULL == pTmp )
        rtl_uString_assign( &pTmp, ustrSystemPath );

    /* temporary check for top 5 wrong usage strings (which are valid but unlikly filenames) */
    /*
    OSL_ASSERT( !findWrongUsage( pTmp->buffer, pTmp->length ) );
    */

    /* file URLs must be URI encoded */
    rtl_uriEncode( pTmp, uriCharClass, rtl_UriEncodeIgnoreEscapes, RTL_TEXTENCODING_UTF8, pustrFileURL );

    rtl_uString_release( pTmp );

    /* absolute urls should start with 'file://' */
    if( (sal_Unicode)'/' == (*pustrFileURL)->buffer[0] )
    {
        rtl_uString *pProtocol = NULL;

        rtl_uString_newFromAscii( &pProtocol, "file://" );
        rtl_uString_newConcat( pustrFileURL, pProtocol, *pustrFileURL );
        rtl_uString_release( pProtocol );
    }

    return osl_File_E_None;
}

/****************************************************************************
 * _osl_getSystemPathFromFileURL - helper function
 * clients may specify if they want to accept relative
 * URLs or not
 ****************************************************************************/

oslFileError osl_getSystemPathFromFileURL_Ex(
    rtl_uString *ustrFileURL, rtl_uString **pustrSystemPath, sal_Bool bAllowRelative)
{
    rtl_uString* temp = 0;
    oslFileError osl_error = osl_getSystemPathFromFileURL(ustrFileURL, &temp);

    if (osl_File_E_None == osl_error)
    {
        if (bAllowRelative || (OSL_FILE_CHAR_SLASH == temp->buffer[0]))
        {
            *pustrSystemPath = temp;
        }
        else
        {
            rtl_uString_release(temp);
            osl_error = osl_File_E_INVAL;
        }
    }

    return osl_error;
}

/******************************************************
 * Helper function, return a pinter to the final '\0'
 * of a string
 ******************************************************/

static char* _strtoend(char* pStr)
{
    char* p = pStr;
    while(*p) p++;
    return p;
}

/******************************************************
 * Append one character to a string
 ******************************************************/

static char* _strcatchr(char* pStr, char Chr)
{
    char* p = _strtoend(pStr);
    *p++ = Chr;
    *p = '\0';
    return pStr;
}

/******************************************************
 *
 ******************************************************/

static int _islastchr(char* pStr, char Chr)
{
   char* p = _strtoend(pStr);
   if (p > pStr)
       p--;
   return (*p == Chr);
}

/******************************************************
 * Ensure that the given string has the specified last
 * character if necessary append it
 ******************************************************/

static char* _strensurelast(char* pStr, char Chr)
{
    if (!_islastchr(pStr, Chr))
        _strcatchr(pStr, Chr);
    return pStr;
}

/******************************************************
 * Remove the last part of a path, a path that has
 * only a '/' or no '/' at all will be returned
 * unmodified
 ******************************************************/

static char* _rmlastpathtoken(char* aPath)
{
    /* we always may skip -2 because we
       may at least stand on a '/' but
       either there is no other character
       before this '/' or it's another
       character than the '/'
    */
    char* p = _strtoend(aPath) - 2;

    // move back to the next path separator
    // or to the start of the string
    while (p > aPath && *p != '/')
        p--;

    if (p >= aPath)
    {
        if ('/' == *p)
        {
            p++;
           *p = '\0';
        }
        else
        {
           *p = '\0';
        }
    }

    return aPath;
}

/******************************************************
 *
 ******************************************************/

static oslFileError _osl_resolvepath(
    /*inout*/ char* path,
    /*inout*/ char* current_pos,
    char* sentinel,
    /*inout*/ sal_Bool* failed)
{
    oslFileError ferr = osl_File_E_None;

    if (!*failed)
    {
        char realpath_buffer[PATH_MAX];

        if (realpath(path, realpath_buffer))
        {
            strcpy(path, realpath_buffer);
            current_pos = _strtoend(path) - 1;
        }
        else
        {
            if (EACCES == errno || ENOTDIR == errno || ENOENT == errno)
                *failed = sal_True;
            else
                ferr = oslTranslateFileError(OSL_FET_ERROR, errno);
        }
    }

    return ferr;
}

/******************************************************
 * Works even with non existing paths. The resulting
 * path must not exceed PATH_MAX else
 * osl_File_E_NAMETOOLONG is the result
 ******************************************************/

static oslFileError osl_realpath_impl_(const char *path_unresolved, char* path_resolved)
{
    char path_resolved_so_far[PATH_MAX];
    char realpath_buffer[PATH_MAX];

    char* punresolved = const_cast<char*>(path_unresolved);
    char* presolvedsf = path_resolved_so_far;

    /* reserve space for leading '/' and trailing '\0'
       do not exceed this limit */
    char* sentinel = path_resolved_so_far + PATH_MAX - 2;

    /* if realpath fails with error ENOTDIR, EACCES or ENOENT
       we will not call it again, because _osl_realpath should also
       work with non existing directories etc. */
    sal_Bool realpath_failed = sal_False;
    oslFileError ferr;

    OSL_PRECOND(punresolved && path_resolved, "_osl_realpath: Invalid parameter");
    OSL_PRECOND(punresolved[0] == '/', "_osl_realpath: Bad parameter, not absolute path given");

    /* the given unresolved path must not exceed PATH_MAX */
    if (strlen(punresolved) >= PATH_MAX - 2)
        return oslTranslateFileError(OSL_FET_ERROR, ENAMETOOLONG);

    path_resolved_so_far[0] = '\0';

    while (*punresolved != '\0')
    {
        /* ignore '/.' , skip one part back when '/..' */

        if ('.' == *punresolved && *presolvedsf == '/' )
        {
            if ('\0' == *(punresolved + 1))
            {
                punresolved++;
                continue;
            }
            else if ('/' == *(punresolved + 1))
            {
                punresolved += 2;
                continue;
            }
            else if ('.' == *(punresolved + 1) && ('\0' == *(punresolved + 2) || '/' == *(punresolved + 2)))
            {
                _rmlastpathtoken(path_resolved_so_far);

                presolvedsf = _strtoend(path_resolved_so_far) - 1;

                if ('/' == *(punresolved + 2))
                    punresolved += 3;
                else
                    punresolved += 2;

                continue;
            }
            else /* a file or directory name may start with '.' */
            {
                if ((presolvedsf = _strtoend(path_resolved_so_far)) > sentinel)
                    return oslTranslateFileError(OSL_FET_ERROR, ENAMETOOLONG);

                _strcatchr(path_resolved_so_far, *punresolved++);

                if ('\0' == *punresolved && !realpath_failed)
                {
                    ferr = _osl_resolvepath(
                        path_resolved_so_far,
                        presolvedsf,
                        sentinel,
                        &realpath_failed);

                    if (osl_File_E_None != ferr)
                        return ferr;
                }
            }
        }
        else if ('/' == *punresolved)
        {
            if ((presolvedsf = _strtoend(path_resolved_so_far)) > sentinel)
                return oslTranslateFileError(OSL_FET_ERROR, ENAMETOOLONG);

            _strcatchr(path_resolved_so_far, *punresolved++);

            if (!realpath_failed)
            {
                ferr = _osl_resolvepath(
                    path_resolved_so_far,
                    presolvedsf,
                    sentinel,
                    &realpath_failed);

                if (osl_File_E_None != ferr)
                    return ferr;

                if (!_islastchr(path_resolved_so_far, '/'))
                {
                    if ((presolvedsf = _strtoend(path_resolved_so_far)) > sentinel)
                    return oslTranslateFileError(OSL_FET_ERROR, ENAMETOOLONG);

                    _strcatchr(path_resolved_so_far, '/');
                }
            }
        }
        else // any other character
        {
            if ((presolvedsf = _strtoend(path_resolved_so_far)) > sentinel)
                return oslTranslateFileError(OSL_FET_ERROR, ENAMETOOLONG);

            _strcatchr(path_resolved_so_far, *punresolved++);

            if ('\0' == *punresolved && !realpath_failed)
            {
                ferr = _osl_resolvepath(
                    path_resolved_so_far,
                    presolvedsf,
                    sentinel,
                    &realpath_failed);

                if (osl_File_E_None != ferr)
                    return ferr;
            }
        }
    }

    OSL_ASSERT(strlen(path_resolved_so_far) < PATH_MAX);

    strcpy(path_resolved, path_resolved_so_far);

    return osl_File_E_None;
}

/******************************************************
 * Helper function, check if the path starts with a '/'
 * then its absolute
 *
 * @precond aPath must not be 0
 ******************************************************/

static int osl_is_relative_path_impl_(const rtl_uString* aPath)
{
    OSL_PRECOND(aPath, "Invalid parameter");
    return ((sal_Unicode)'/' != aPath->buffer[0]);
}

/******************************************************
 * Helper function, concatenate two paths base + rel
 * -> base/rel
 ******************************************************/

static void _pathcat(
    const rtl_uString* ustrBasePath, const rtl_uString* ustrRelativePath, rtl_uString** pustrConcatedPath)
{
    sal_Int32 needed_capacity = ustrBasePath->length + ustrRelativePath->length + 1;

    OSL_PRECOND(ustrBasePath && ustrRelativePath, "Invalid parameter");
    OSL_PRECOND(pustrConcatedPath && !*pustrConcatedPath, "Invalid parameter");

    rtl_uString_new_WithLength(pustrConcatedPath, needed_capacity);

    rtl_uStringbuffer_insert(pustrConcatedPath, &needed_capacity, 0, ustrBasePath->buffer, ustrBasePath->length);

    OSL_ASSERT(((rtl_uString*)*pustrConcatedPath)->length);

    if (ustrBasePath->buffer[ustrBasePath->length - 1] != (sal_Unicode)'/')
        rtl_uStringbuffer_insert_ascii(pustrConcatedPath, &needed_capacity, ((rtl_uString*)*pustrConcatedPath)->length, "/", 1);

    rtl_uStringbuffer_insert(
        pustrConcatedPath,
        &needed_capacity,
        ((rtl_uString*)*pustrConcatedPath)->length,
        ustrRelativePath->buffer,
        ustrRelativePath->length);
}

/******************************************************
 *
 ******************************************************/

oslFileError osl_getAbsoluteFileURL(
    rtl_uString*  ustrBaseDirURL, rtl_uString* ustrRelativeURL, rtl_uString** pustrAbsoluteURL)
{
    oslFileError ferr                 = osl_File_E_INVAL;
    rtl_uString* ustrPath             = NULL;
    char         asciiPath[PATH_MAX];

    ferr = osl_getSystemPathFromFileURL(ustrRelativeURL, &ustrPath);

    if(osl_File_E_None != ferr)
        return ferr;

    if(osl_is_relative_path_impl_(ustrPath))
    {
        rtl_uString* ustrBasePath     = NULL;
        rtl_uString* ustrConcatedPath = NULL;

        /* concatinate base and relative path */
        ferr = osl_getSystemPathFromFileURL(ustrBaseDirURL, &ustrBasePath);

        if (osl_File_E_None != ferr)
        {
            rtl_uString_release(ustrPath);
            return ferr;
        }

        _pathcat(ustrBasePath, ustrPath, &ustrConcatedPath);

        rtl_uString_release(ustrBasePath);
        rtl_uString_release(ustrPath);

        ustrPath = ustrConcatedPath;
    }

    /* convert unicode path to text */
    if (UnicodeToText(asciiPath, PATH_MAX, ustrPath->buffer, ustrPath->length))
    {
        char realPath[PATH_MAX];

        /* use realpath to determine absolute path */
        ferr = osl_realpath_impl_(asciiPath, realPath);

        if (osl_File_E_None == ferr)
        {
            rtl_uString* ustrRealPath = NULL;

            /* convert file name to unicode */
            rtl_string2UString(
                &ustrRealPath,
                realPath,
                strlen(realPath),
                osl_getThreadTextEncoding(),
                OSTRING_TO_OUSTRING_CVTFLAGS);

            /* file urls must be encoded */
            ferr = osl_getFileURLFromSystemPath(ustrRealPath, pustrAbsoluteURL);

            OSL_ASSERT(osl_File_E_None == ferr);

            rtl_uString_release(ustrRealPath);
        }
    }
    else
    {
       ferr = osl_File_E_INVAL;
    }

    rtl_uString_release(ustrPath);

    return ferr;
}

/****************************************************************************
 *  ImplSearchPath
 ***************************************************************************/

static char * ImplSearchPath( char * buffer, size_t buflen, const char * filePath, const char * searchPath, char separator )
{
    char *pPathItem;
    char *pc;
    size_t nFilePathLen;

    pPathItem    = const_cast<char*>(searchPath);
    nFilePathLen = strlen(filePath);

    do
    {
        char path[PATH_MAX];
        size_t len;

        /* extract path item */
        pc = strchr( pPathItem, separator );
        len = pc ? pc - pPathItem : strlen( pPathItem );

        /* copy path entry to buffer and append file Path */
        if( PATH_MAX > len + nFilePathLen + 1 )
        {
            strncpy( path, pPathItem, len );
            path[len++] = '/';
            strcpy( path + len, filePath );

            if( 0 <= access( path, F_OK ) )
                return realpath( path, buffer );
        }

        pPathItem = pc + 1;
    }
    while( NULL != pc );

    return NULL;
}

/****************************************************************************
 *  osl_searchFileURL
 ***************************************************************************/

oslFileError osl_searchFileURL( rtl_uString* ustrFilePath, rtl_uString* ustrSearchPath, rtl_uString** pustrURL )
{
    rtl_uString* ustrPath = NULL;
    oslFileError eRet;

    char searchPath[PATH_MAX];
    char filePath[PATH_MAX];
    char path[PATH_MAX] = "";

    OSL_ASSERT( ustrFilePath );
    OSL_ASSERT( ustrSearchPath );
    OSL_ASSERT( pustrURL );

    /* file path may also be an URL */
    eRet = FileURLToPath( filePath, PATH_MAX, ustrFilePath );

    if( eRet != osl_File_E_None )
    {
        if( eRet == osl_File_E_INVAL )
        {
            /* seems not to be an URL, so expect it to be a system patrh */
            if( ! UnicodeToText( filePath, PATH_MAX, ustrFilePath->buffer, ustrFilePath->length ) )
                return osl_File_E_INVAL;
        }
        else
            return eRet;
    }

    /* if a search path is specified, it is no file URL */
    if( ustrSearchPath->length && UnicodeToText( searchPath, PATH_MAX, ustrSearchPath->buffer, ustrSearchPath->length )  )
    {
        if( NULL == ImplSearchPath( path, PATH_MAX, filePath, searchPath, ';' ) )
           *path = '\0';
    }

    /* did we already find something ? */
    if( '\0' == *path )
    {
        char * pEnvPath = getenv("PATH");

        /* fallback to PATH env var */
        if( ( NULL == pEnvPath ) || ( NULL == ImplSearchPath( path, PATH_MAX, filePath, pEnvPath , ':' ) ) )
        {
            char workdir[PATH_MAX];

            /* last try: current working dir */
            if( ( NULL == getcwd( workdir, PATH_MAX ) ) || ( NULL == ImplSearchPath( path, PATH_MAX, filePath, workdir , ':' ) ) )
                return osl_File_E_NOENT;
         }
    }

    /* did we find something, then convert it */
    if( *path )
    {
        /* convert file name to unicode */
        rtl_string2UString( &ustrPath, path, strlen( path ), osl_getThreadTextEncoding(), OSTRING_TO_OUSTRING_CVTFLAGS );

        /* file urls must be encoded */
        osl_getFileURLFromSystemPath( ustrPath, pustrURL );
        rtl_uString_release( ustrPath );
    }

    return osl_File_E_None;
}

/****************************************************************************
 * FileURLToPath
 ***************************************************************************/

oslFileError FileURLToPath(char * buffer, size_t bufLen, rtl_uString* ustrFileURL)
{
    rtl_uString* ustrSystemPath = NULL;
    oslFileError osl_error      = osl_getSystemPathFromFileURL(ustrFileURL, &ustrSystemPath);

    OSL_ASSERT(osl_File_E_None == osl_error);

    if(osl_File_E_None != osl_error)
        return osl_error;

    osl_systemPathRemoveSeparator(ustrSystemPath);

    /* convert unicode path to text */
    if(!UnicodeToText( buffer, bufLen, ustrSystemPath->buffer, ustrSystemPath->length))
        osl_error = oslTranslateFileError(OSL_FET_ERROR, errno);

    rtl_uString_release(ustrSystemPath);

    return osl_error;
}
