/*************************************************************************
 *
 *  $RCSfile: file_url.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-26 16:46:03 $
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

#ifndef __OSL_SYSTEM_H__
#include "system.h"
#endif

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

#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif

#ifndef _OSL_PROCESS_H_
#include <osl/process.h>
#endif

#ifndef _FILE_ERROR_TRANSL_H_
#include "file_error_transl.h"
#endif

#ifndef _FILE_URL_H_
#include "file_url.h"
#endif

#ifndef _OSL_FILE_PATH_HELPER_HXX_
#include "file_path_helper.hxx"
#endif

#ifndef _OSL_UUNXAPI_HXX_
#include "uunxapi.hxx"
#endif

/***************************************************

 General note

 This file contains the part that handles File URLs.

 File URLs as scheme specific notion of URIs
 (RFC2396) may be handled platform independend, but
 will not in osl which is considered wrong.
 Future version of osl should handle File URLs this
 way. In rtl/uri there is already an URI parser etc.
 so this code should be consolidated.

 **************************************************/



/***************************************************
 * forward
 **************************************************/

extern "C" int UnicodeToText(char *, size_t, const sal_Unicode *, sal_Int32);
extern "C" int TextToUnicode(const char* text, size_t text_buffer_size, sal_Unicode* unic_text, sal_Int32 unic_text_buffer_size);

/***************************************************
 * namespace directives
 **************************************************/

using namespace osl;

/***************************************************
 * constants
 **************************************************/

const sal_Unicode UNICHAR_SLASH = ((sal_Unicode)'/');
const sal_Unicode UNICHAR_COLON = ((sal_Unicode)':');
const sal_Unicode UNICHAR_DOT   = ((sal_Unicode)'.');

/******************************************************************************
 *
 *                  Exported Module Functions
 *
 *****************************************************************************/

/* a slightly modified version of Pchar in rtl/source/uri.c */
const sal_Bool uriCharClass[128] =
{
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* Pchar but without encoding slashes */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 1, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* !"#$%&'()*+,-./  */
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, /* 0123456789:;<=>? */
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* @ABCDEFGHIJKLMNO */
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, /* PQRSTUVWXYZ[\]^_ */
  0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* `abcdefghijklmno */
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0  /* pqrstuvwxyz{|}~  */
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
 * osl_getSystemPathFromFileURL_Ex - helper function
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
        if (bAllowRelative || (UNICHAR_SLASH == temp->buffer[0]))
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

namespace /* private */
{

    /******************************************************
     * Helper function, return a pinter to the final '\0'
     * of a string
     ******************************************************/

    sal_Unicode* ustrtoend(sal_Unicode* pStr)
    {
        return (pStr + rtl_ustr_getLength(pStr));
    }

    /*********************************************

     ********************************************/
    sal_Unicode* ustrcpy(const sal_Unicode* s, sal_Unicode* d)
    {
        const sal_Unicode* sc = s;
        sal_Unicode*       dc = d;

        while (*dc++ = *sc++)
            /**/;

        return d;
    }

    /*********************************************

     ********************************************/

    sal_Unicode* ustrncpy(const sal_Unicode* s, sal_Unicode* d, unsigned int n)
    {
        const sal_Unicode* sc = s;
        sal_Unicode*       dc = d;
        unsigned int       i  = n;

        while (i--)
            *dc++ = *sc++;

        if (n)
            *dc = 0;

        return d;
    }

    /*********************************************

     ********************************************/

    sal_Unicode* ustrchrcat(const sal_Unicode chr, sal_Unicode* d)
    {
        sal_Unicode* p = ustrtoend(d);
        *p++ = chr;
        *p   = 0;
        return d;
    }

    /*********************************************

     ********************************************/

    sal_Unicode* ustrcat(const sal_Unicode* s, sal_Unicode* d)
    {
        sal_Unicode* dc = ustrtoend(d);
        ustrcpy(s, dc);
        return d;
    }

    /******************************************************
     *
     ******************************************************/

    bool _islastchr(sal_Unicode* pStr, sal_Unicode Chr)
    {
           sal_Unicode* p = ustrtoend(pStr);
           if (p > pStr)
               p--;
           return (*p == Chr);
    }

    /******************************************************
     * Ensure that the given string has the specified last
     * character if necessary append it
     ******************************************************/

    sal_Unicode* _strensurelast(sal_Unicode* pStr, sal_Unicode Chr)
    {
        if (!_islastchr(pStr, Chr))
            ustrchrcat(Chr, pStr);
        return pStr;
    }

    /******************************************************
     * Remove the last part of a path, a path that has
     * only a '/' or no '/' at all will be returned
     * unmodified
     ******************************************************/

    sal_Unicode* _rmlastpathtoken(sal_Unicode* aPath)
    {
        /*  we always may skip -2 because we
               may at least stand on a '/' but
               either there is no other character
               before this '/' or it's another
               character than the '/'
        */
        sal_Unicode* p = ustrtoend(aPath) - 2;

        // move back to the next path separator
        // or to the start of the string
        while ((p > aPath) && (*p != UNICHAR_SLASH))
            p--;

        if (p >= aPath)
        {
            if (UNICHAR_SLASH == *p)
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

    oslFileError _osl_resolvepath(
        /*inout*/ sal_Unicode* path,
        /*inout*/ sal_Unicode* current_pos,
        /*in   */ sal_Unicode* sentinel,
        /*inout*/ bool* failed)
    {
        oslFileError ferr = osl_File_E_None;

        if (!*failed)
        {
            char unresolved_path[PATH_MAX];
            if (!UnicodeToText(unresolved_path, sizeof(unresolved_path), path, rtl_ustr_getLength(path)))
                return oslTranslateFileError(OSL_FET_ERROR, ENAMETOOLONG);

            char resolved_path[PATH_MAX];
            if (realpath(unresolved_path, resolved_path))
            {
                if (!TextToUnicode(resolved_path, strlen(resolved_path), path, PATH_MAX))
                    return oslTranslateFileError(OSL_FET_ERROR, ENAMETOOLONG);

                current_pos = ustrtoend(path) - 1;
            }
            else
            {
                if (EACCES == errno || ENOTDIR == errno || ENOENT == errno)
                    *failed = true;
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

    oslFileError osl_getAbsoluteFileURL_impl_(const rtl::OUString& unresolved_path, rtl::OUString& resolved_path)
    {
        // the given unresolved path must not exceed PATH_MAX
        if (unresolved_path.getLength() >= (PATH_MAX - 2))
            return oslTranslateFileError(OSL_FET_ERROR, ENAMETOOLONG);

        sal_Unicode        path_resolved_so_far[PATH_MAX];
        sal_Unicode        realpath_buffer[PATH_MAX];
        const sal_Unicode* punresolved = unresolved_path.getStr();
        sal_Unicode*       presolvedsf = path_resolved_so_far;

        // reserve space for leading '/' and trailing '\0'
        // do not exceed this limit
        sal_Unicode* sentinel = path_resolved_so_far + PATH_MAX - 2;

        // if realpath fails with error ENOTDIR, EACCES or ENOENT
        // we will not call it again, because _osl_realpath should also
        // work with non existing directories etc.
        bool realpath_failed = false;
        oslFileError ferr;

        path_resolved_so_far[0] = '\0';

        while (*punresolved != '\0')
        {
            // ignore '/.' , skip one part back when '/..'

            if ((UNICHAR_DOT == *punresolved) && (UNICHAR_SLASH == *presolvedsf))
            {
                if ('\0' == *(punresolved + 1))
                {
                    punresolved++;
                    continue;
                }
                else if (UNICHAR_SLASH == *(punresolved + 1))
                {
                    punresolved += 2;
                    continue;
                }
                else if ((UNICHAR_DOT == *(punresolved + 1)) && ('\0' == *(punresolved + 2) || (UNICHAR_SLASH == *(punresolved + 2))))
                {
                    _rmlastpathtoken(path_resolved_so_far);

                    presolvedsf = ustrtoend(path_resolved_so_far) - 1;

                    if (UNICHAR_SLASH == *(punresolved + 2))
                        punresolved += 3;
                    else
                        punresolved += 2;

                    continue;
                }
                else // a file or directory name may start with '.'
                {
                    if ((presolvedsf = ustrtoend(path_resolved_so_far)) > sentinel)
                        return oslTranslateFileError(OSL_FET_ERROR, ENAMETOOLONG);

                    ustrchrcat(*punresolved++, path_resolved_so_far);

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
            else if (UNICHAR_SLASH == *punresolved)
            {
                if ((presolvedsf = ustrtoend(path_resolved_so_far)) > sentinel)
                    return oslTranslateFileError(OSL_FET_ERROR, ENAMETOOLONG);

                ustrchrcat(*punresolved++, path_resolved_so_far);

                if (!realpath_failed)
                {
                    ferr = _osl_resolvepath(
                        path_resolved_so_far,
                        presolvedsf,
                        sentinel,
                        &realpath_failed);

                    if (osl_File_E_None != ferr)
                        return ferr;

                    if (!_islastchr(path_resolved_so_far, UNICHAR_SLASH))
                    {
                        if ((presolvedsf = ustrtoend(path_resolved_so_far)) > sentinel)
                            return oslTranslateFileError(OSL_FET_ERROR, ENAMETOOLONG);

                        ustrchrcat(UNICHAR_SLASH, path_resolved_so_far);
                    }
                }
            }
            else // any other character
            {
                if ((presolvedsf = ustrtoend(path_resolved_so_far)) > sentinel)
                    return oslTranslateFileError(OSL_FET_ERROR, ENAMETOOLONG);

                ustrchrcat(*punresolved++, path_resolved_so_far);

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

        sal_Int32 len = rtl_ustr_getLength(path_resolved_so_far);

        OSL_ASSERT(len < PATH_MAX);

        resolved_path = rtl::OUString(path_resolved_so_far, len);

        return osl_File_E_None;
    }

} // end namespace private


/******************************************************
 * osl_getAbsoluteFileURL
 ******************************************************/

oslFileError osl_getAbsoluteFileURL(rtl_uString*  ustrBaseDirURL, rtl_uString* ustrRelativeURL, rtl_uString** pustrAbsoluteURL)
{
    FileBase::RC  rc;
    rtl::OUString unresolved_path;

    rc = FileBase::getSystemPathFromFileURL(rtl::OUString(ustrRelativeURL), unresolved_path);

    if(FileBase::E_None != rc)
        return oslFileError(rc);

    if (systemPathIsRelativePath(unresolved_path))
    {
        rtl::OUString base_path;
        rc = (FileBase::RC) osl_getSystemPathFromFileURL_Ex(ustrBaseDirURL, &base_path.pData, sal_False);

        if (FileBase::E_None != rc)
            return oslFileError(rc);

        rtl::OUString abs_path;
        systemPathMakeAbsolutePath(base_path, unresolved_path, abs_path);

        unresolved_path = abs_path;
    }

    rtl::OUString resolved_path;
    rc = (FileBase::RC) osl_getAbsoluteFileURL_impl_(unresolved_path, resolved_path);

    if (FileBase::E_None == rc)
    {
        rc = (FileBase::RC) osl_getFileURLFromSystemPath(resolved_path.pData, pustrAbsoluteURL);
        OSL_ASSERT(FileBase::E_None == rc);
    }

    return oslFileError(rc);
}


namespace /* private */
{

    /*********************************************
     No separate error code if unicode to text
     conversion or getenv fails because for the
     caller there is no difference why a file
     could not be found in $PATH
     ********************************************/

    bool find_in_PATH(const rtl::OUString& file_path, rtl::OUString& result)
    {
        bool          bfound = false;
        rtl::OUString path   = rtl::OUString::createFromAscii("PATH");
        rtl::OUString env_path;

        if (osl_Process_E_None == osl_getEnvironment(path.pData, &env_path.pData))
            bfound = osl::searchPath(file_path, env_path, result);

        return bfound;
    }

    /*********************************************
     No separate error code if unicode to text
     conversion or getcwd fails because for the
     caller there is no difference why a file
     could not be found in CDW
     ********************************************/

    bool find_in_CWD(const rtl::OUString& file_path, rtl::OUString& result)
    {
        bool bfound = false;
        rtl::OUString cwd_url;

        if (osl_Process_E_None == osl_getProcessWorkingDir(&cwd_url.pData))
        {
            rtl::OUString cwd;
            FileBase::getSystemPathFromFileURL(cwd_url, cwd);
            bfound = osl::searchPath(file_path, cwd, result);
        }
        return bfound;
    }

    /*********************************************

     ********************************************/

    bool find_in_searchPath(const rtl::OUString& file_path, rtl_uString* search_path, rtl::OUString& result)
    {
        return (search_path && osl::searchPath(file_path, rtl::OUString(search_path), result));
    }

} // end namespace private


/****************************************************************************
 *  osl_searchFileURL
 ***************************************************************************/

oslFileError osl_searchFileURL(rtl_uString* ustrFilePath, rtl_uString* ustrSearchPath, rtl_uString** pustrURL)
{
    OSL_PRECOND(ustrFilePath && pustrURL, "osl_searchFileURL: invalid parameter");

    FileBase::RC  rc;
    rtl::OUString file_path;

    // try to interpret search path as file url else assume it's a system path list
    rc = FileBase::getSystemPathFromFileURL(rtl::OUString(ustrFilePath), file_path);
    if ((FileBase::E_None != rc) && (FileBase::E_INVAL == rc))
        file_path = ustrFilePath;
    else if (FileBase::E_None != rc)
        return oslFileError(rc);

    bool          bfound = false;
    rtl::OUString result;

    if (find_in_searchPath(file_path, ustrSearchPath, result) ||
        find_in_PATH(file_path, result) ||
        find_in_CWD(file_path, result))
    {
        rtl::OUString resolved;

        if (osl::realpath(result, resolved))
        {
            oslFileError osl_error = osl_getFileURLFromSystemPath(resolved.pData, pustrURL);
            OSL_ASSERT(osl_File_E_None == osl_error);
            bfound = true;
        }
    }
    return bfound ? osl_File_E_None : osl_File_E_NOENT;
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
