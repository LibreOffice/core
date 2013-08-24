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

#include "file_url.h"

#include "system.h"

#include <limits.h>
#include <errno.h>
#include <strings.h>
#include <unistd.h>

#include "osl/file.hxx"
#include <osl/security.h>
#include <osl/diagnose.h>
#include <osl/thread.h>
#include <osl/process.h>

#include <rtl/uri.h>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.h>
#include "rtl/textcvt.h"

#include "file_error_transl.h"
#include "file_path_helper.hxx"

#include "uunxapi.hxx"

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

using namespace osl;

const sal_Unicode UNICHAR_SLASH = ((sal_Unicode)'/');
const sal_Unicode UNICHAR_COLON = ((sal_Unicode)':');
const sal_Unicode UNICHAR_DOT   = ((sal_Unicode)'.');

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

oslFileError SAL_CALL osl_getCanonicalName( rtl_uString* ustrFileURL, rtl_uString** pustrValidURL )
{
    OSL_FAIL("osl_getCanonicalName not implemented");

    rtl_uString_newFromString(pustrValidURL, ustrFileURL);
    return osl_File_E_None;
}

oslFileError SAL_CALL osl_getSystemPathFromFileURL( rtl_uString *ustrFileURL, rtl_uString **pustrSystemPath )
{
    sal_Int32 nIndex;
    rtl_uString * pTmp = NULL;

    sal_Unicode encodedSlash[3] = { '%', '2', 'F' };
    sal_Unicode protocolDelimiter[3] = { ':', '/', '/' };

    /* temporary hack: if already system path, return ustrFileURL */
    /*
    if( (sal_Unicode) '/' == ustrFileURL->buffer[0] )
    {
        OSL_FAIL( "osl_getSystemPathFromFileURL: input is already system path" );
        rtl_uString_assign( pustrSystemPath, ustrFileURL );
        return osl_File_E_None;
    }
    */

    /* a valid file url may not start with '/' */
    if( ( 0 == ustrFileURL->length ) || ( (sal_Unicode) '/' == ustrFileURL->buffer[0] ) )
    {
        return osl_File_E_INVAL;
    }

    /* Check for non file:// protocols */

    nIndex = rtl_ustr_indexOfStr_WithLength( ustrFileURL->buffer, ustrFileURL->length, protocolDelimiter, 3 );
    if ( -1 != nIndex && (4 != nIndex || 0 != rtl_ustr_ascii_shortenedCompare_WithLength( ustrFileURL->buffer, ustrFileURL->length,"file", 4 ) ) )
    {
        return osl_File_E_INVAL;
    }

    /* search for encoded slashes (%2F) and decode every single token if we find one */

    nIndex = 0;

    if( -1 != rtl_ustr_indexOfStr_WithLength( ustrFileURL->buffer, ustrFileURL->length, encodedSlash, 3 ) )
    {
        rtl_uString * ustrPathToken = NULL;
        sal_Int32 nOffset = 7;

        do
        {
            nOffset += nIndex;

            /* break url down in '/' divided tokens tokens */
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
            oslSecurity pSecurity = osl_getCurrentSecurity();
            osl_getHomeDir( pSecurity , &pTmp2 );
            osl_freeSecurityHandle( pSecurity );

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

    rtl_uString_assign ( pustrSystemPath, pTmp );
    rtl_uString_release ( pTmp );
    return osl_File_E_None;
}

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
            OSL_FAIL( "osl_getFileURLFromSystemPath: input is already file URL" );
            rtl_uString_assign( pustrFileURL, ustrSystemPath );
        }
        else
        {
            rtl_uString *pTmp2 = NULL;

            OSL_FAIL( "osl_getFileURLFromSystemPath: input is wrong file URL" );
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
            oslSecurity pSecurity = osl_getCurrentSecurity();
            osl_getHomeDir( pSecurity , &pTmp );
            osl_freeSecurityHandle( pSecurity );

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

        /* remove all occurrences of '//' */
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

namespace
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

    sal_Unicode* ustrchrcat(const sal_Unicode chr, sal_Unicode* d)
    {
        sal_Unicode* p = ustrtoend(d);
        *p++ = chr;
        *p   = 0;
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

}

oslFileError osl_getAbsoluteFileURL(rtl_uString*  ustrBaseDirURL, rtl_uString* ustrRelativeURL, rtl_uString** pustrAbsoluteURL)
{
    // Work around the below call to getSystemPathFromFileURL rejecting input
    // that starts with "/" (for whatever reason it behaves that way; but
    // changing that would start to break lots of tests at least):
    rtl::OUString relUrl(ustrRelativeURL);
    if (relUrl.startsWith("//")) {
        relUrl = "file:" + relUrl;
    } else if (relUrl.startsWith("/")) {
        relUrl = "file://" + relUrl;
    }

    FileBase::RC  rc;
    rtl::OUString unresolved_path;

    rc = FileBase::getSystemPathFromFileURL(relUrl, unresolved_path);

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

    static bool allow_symlinks = getenv("SAL_ALLOW_LINKOO_SYMLINKS") != 0;
        // getenv is not thread safe, so minimize use of result
    if (!allow_symlinks)
    {
        rc = (FileBase::RC) osl_getAbsoluteFileURL_impl_(unresolved_path, resolved_path);
    }
    else
    {
        // SAL_ALLOW_LINKOO_SYMLINKS environment variable:
        // for linkoo to work, we need to let the symlinks to the libraries untouched
        rtl::OUString base;
        sal_Int32 last_slash = unresolved_path.lastIndexOf( UNICHAR_SLASH );

        if (last_slash >= 0 && last_slash + 1 < unresolved_path.getLength()
            && ! ( last_slash + 2 == unresolved_path.getLength() && unresolved_path.matchAsciiL(RTL_CONSTASCII_STRINGPARAM("."), last_slash + 1) )
            && ! ( last_slash + 3 == unresolved_path.getLength() && unresolved_path.matchAsciiL(RTL_CONSTASCII_STRINGPARAM(".."), last_slash + 1) ))
        {
            base = unresolved_path.copy(last_slash+1);
            unresolved_path = unresolved_path.copy(0, last_slash);
        }

        rc = (FileBase::RC) osl_getAbsoluteFileURL_impl_(unresolved_path, resolved_path);

        if (!base.isEmpty())
        {
            resolved_path += rtl::OUString( UNICHAR_SLASH );
            resolved_path += base;
        }
    }

    if (FileBase::E_None == rc)
    {
        rc = (FileBase::RC) osl_getFileURLFromSystemPath(resolved_path.pData, pustrAbsoluteURL);
        OSL_ASSERT(FileBase::E_None == rc);
    }

    return oslFileError(rc);
}

namespace
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
        rtl::OUString path("PATH");
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

}

oslFileError osl_searchFileURL(rtl_uString* ustrFilePath, rtl_uString* ustrSearchPath, rtl_uString** pustrURL)
{
    OSL_PRECOND(ustrFilePath && pustrURL, "osl_searchFileURL: invalid parameter");

    FileBase::RC  rc;
    rtl::OUString file_path;

    // try to interpret search path as file url else assume it's a system path list
    rc = FileBase::getSystemPathFromFileURL(rtl::OUString(ustrFilePath), file_path);
    if (FileBase::E_INVAL == rc)
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
#if OSL_DEBUG_LEVEL > 0
            oslFileError osl_error =
#endif
                osl_getFileURLFromSystemPath(resolved.pData, pustrURL);
            OSL_ASSERT(osl_File_E_None == osl_error);
            bfound = true;
        }
    }
    return bfound ? osl_File_E_None : osl_File_E_NOENT;
}

oslFileError FileURLToPath(char * buffer, size_t bufLen, rtl_uString* ustrFileURL)
{
    rtl_uString* ustrSystemPath = NULL;
    oslFileError osl_error      = osl_getSystemPathFromFileURL(ustrFileURL, &ustrSystemPath);

    if(osl_File_E_None != osl_error)
        return osl_error;

    osl_systemPathRemoveSeparator(ustrSystemPath);

    /* convert unicode path to text */
    if(!UnicodeToText( buffer, bufLen, ustrSystemPath->buffer, ustrSystemPath->length))
        osl_error = oslTranslateFileError(OSL_FET_ERROR, errno);

    rtl_uString_release(ustrSystemPath);

    return osl_error;
}

namespace
{
    class UnicodeToTextConverter_Impl
    {
        rtl_UnicodeToTextConverter m_converter;

        UnicodeToTextConverter_Impl()
            : m_converter (rtl_createUnicodeToTextConverter (osl_getThreadTextEncoding()))
        {}

        ~UnicodeToTextConverter_Impl()
        {
            rtl_destroyUnicodeToTextConverter (m_converter);
        }
    public:
        static UnicodeToTextConverter_Impl & getInstance()
        {
            static UnicodeToTextConverter_Impl g_theConverter;
            return g_theConverter;
        }

        sal_Size convert(
            sal_Unicode const * pSrcBuf, sal_Size nSrcChars, sal_Char * pDstBuf, sal_Size nDstBytes,
            sal_uInt32 nFlags, sal_uInt32 * pInfo, sal_Size * pSrcCvtChars)
        {
            OSL_ASSERT(m_converter != 0);
            return rtl_convertUnicodeToText (
                m_converter, 0, pSrcBuf, nSrcChars, pDstBuf, nDstBytes, nFlags, pInfo, pSrcCvtChars);
        }
    };
}

int UnicodeToText( char * buffer, size_t bufLen, const sal_Unicode * uniText, sal_Int32 uniTextLen )
{
    sal_uInt32   nInfo = 0;
    sal_Size     nSrcChars = 0;

    sal_Size nDestBytes = UnicodeToTextConverter_Impl::getInstance().convert (
        uniText, uniTextLen, buffer, bufLen,
        OUSTRING_TO_OSTRING_CVTFLAGS | RTL_UNICODETOTEXT_FLAGS_FLUSH, &nInfo, &nSrcChars);

    if( nInfo & RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL )
    {
        errno = EOVERFLOW;
        return 0;
    }

    /* ensure trailing '\0' */
    buffer[nDestBytes] = '\0';
    return nDestBytes;
}

namespace
{
    class TextToUnicodeConverter_Impl
    {
        rtl_TextToUnicodeConverter m_converter;

        TextToUnicodeConverter_Impl()
            : m_converter (rtl_createTextToUnicodeConverter (osl_getThreadTextEncoding()))
        {}

        ~TextToUnicodeConverter_Impl()
        {
            rtl_destroyTextToUnicodeConverter (m_converter);
        }

    public:
        static TextToUnicodeConverter_Impl & getInstance()
        {
            static TextToUnicodeConverter_Impl g_theConverter;
            return g_theConverter;
        }

        sal_Size convert(
            sal_Char const * pSrcBuf, sal_Size nSrcBytes, sal_Unicode * pDstBuf, sal_Size nDstChars,
            sal_uInt32 nFlags, sal_uInt32 * pInfo, sal_Size * pSrcCvtBytes)
        {
            OSL_ASSERT(m_converter != 0);
            return rtl_convertTextToUnicode (
                m_converter, 0, pSrcBuf, nSrcBytes, pDstBuf, nDstChars, nFlags, pInfo, pSrcCvtBytes);
        }
    };
}

int TextToUnicode(
    const char*  text,
    size_t       text_buffer_size,
    sal_Unicode* unic_text,
    sal_Int32    unic_text_buffer_size)
{
    sal_uInt32 nInfo = 0;
    sal_Size   nSrcChars = 0;

    sal_Size nDestBytes = TextToUnicodeConverter_Impl::getInstance().convert(
        text,  text_buffer_size, unic_text, unic_text_buffer_size,
        OSTRING_TO_OUSTRING_CVTFLAGS | RTL_TEXTTOUNICODE_FLAGS_FLUSH, &nInfo, &nSrcChars);

    if (nInfo & RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOSMALL)
    {
        errno = EOVERFLOW;
        return 0;
    }

    /* ensure trailing '\0' */
    unic_text[nDestBytes] = '\0';
    return nDestBytes;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
