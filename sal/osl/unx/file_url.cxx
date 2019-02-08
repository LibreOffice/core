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

#include "file_url.hxx"

#include "system.hxx"

#include <cassert>
#include <stdexcept>
#include <limits.h>
#include <errno.h>
#include <strings.h>
#include <unistd.h>

#include <osl/file.hxx>
#include <osl/security.hxx>
#include <osl/socket.h>
#include <osl/diagnose.h>
#include <osl/thread.h>
#include <osl/process.h>

#include <rtl/character.hxx>
#include <rtl/uri.h>
#include <rtl/uri.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.h>
#include <rtl/textcvt.h>
#include <sal/log.hxx>

#include "file_error_transl.hxx"
#include "file_path_helper.hxx"

#include "uunxapi.hxx"

/** @file

 General note

 This file contains the part that handles File URLs.

 File URLs as scheme specific notion of URIs
 (RFC2396) may be handled platform independent, but
 will not in osl which is considered wrong.
 Future version of osl should handle File URLs this
 way. In rtl/uri there is already an URI parser etc.
 so this code should be consolidated.

*/

using namespace osl;

namespace {

// A slightly modified version of Pchar in rtl/source/uri.c, but without
// encoding slashes:
const sal_Bool uriCharClass[128] = {
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false,
    false,  true, false, false,  true, false,  true,  true,  //  !"#$%&'
     true,  true,  true,  true,  true,  true,  true,  true,  // ()*+,-./
     true,  true,  true,  true,  true,  true,  true,  true,  // 01234567
     true,  true,  true, false, false,  true, false, false,  // 89:;<=>?
     true,  true,  true,  true,  true,  true,  true,  true,  // @ABCDEFG
     true,  true,  true,  true,  true,  true,  true,  true,  // HIJKLMNO
     true,  true,  true,  true,  true,  true,  true,  true,  // PQRSTUVW
     true,  true,  true, false, false, false, false,  true,  // XYZ[\]^_
    false,  true,  true,  true,  true,  true,  true,  true,  // `abcdefg
     true,  true,  true,  true,  true,  true,  true,  true,  // hijklmno
     true,  true,  true,  true,  true,  true,  true,  true,  // pqrstuvw
     true,  true,  true, false, false, false,  true, false}; // xyz{|}~

}

oslFileError SAL_CALL osl_getCanonicalName( rtl_uString* ustrFileURL, rtl_uString** pustrValidURL )
{
    OSL_FAIL("osl_getCanonicalName not implemented");

    rtl_uString_newFromString(pustrValidURL, ustrFileURL);
    return osl_File_E_None;
}

namespace {

oslFileError getSystemPathFromFileUrl(
    OUString const & url, OUString * path, bool resolveHome)
{
    assert(path != nullptr);
    // For compatibility with assumptions in other parts of the code base,
    // assume that anything starting with a slash is a system path instead of a
    // (relative) file URL (except if it starts with two slashes, in which case
    // it is a relative URL with an authority component):
    if (url.isEmpty()
        || (url[0] == '/' && (url.getLength() == 1 || url[1] != '/')))
    {
        return osl_File_E_INVAL;
    }
    // Check for non file scheme:
    sal_Int32 i = 0;
    if (rtl::isAsciiAlpha(url[0])) {
        for (sal_Int32 j = 1; j != url.getLength(); ++j) {
            auto c = url[j];
            if (c == ':') {
                if (rtl_ustr_ascii_compareIgnoreAsciiCase_WithLengths(
                        url.pData->buffer, j,
                        RTL_CONSTASCII_STRINGPARAM("file"))
                    != 0)
                {
                    return osl_File_E_INVAL;
                }
                i = j + 1;
                break;
            }
            if (!rtl::isAsciiAlphanumeric(c) && c != '+' && c != '-'
                       && c != '.')
            {
                break;
            }
        }
    }
    // Handle query or fragment:
    if (url.indexOf('?', i) != -1 || url.indexOf('#', i) != -1)
        return osl_File_E_INVAL;
    // Handle authority, supporting a host of "localhost", "127.0.0.1", or the exact value (e.g.,
    // not supporting an additional final dot, for simplicity) reported by osl_getLocalHostname
    // (and, in each case, ignoring case of ASCII letters):
    if (url.getLength() - i >= 2 && url[i] == '/' && url[i + 1] == '/')
    {
        i += 2;
        sal_Int32 j = url.indexOf('/', i);
        if (j == -1)
            j = url.getLength();
        if (j != i
            && (rtl_ustr_ascii_compareIgnoreAsciiCase_WithLengths(
                    url.pData->buffer + i, j - i,
                    RTL_CONSTASCII_STRINGPARAM("localhost"))
                != 0)
            && (rtl_ustr_ascii_compareIgnoreAsciiCase_WithLengths(
                    url.pData->buffer + i, j - i,
                    RTL_CONSTASCII_STRINGPARAM("127.0.0.1"))
                != 0))
        {
            OUString hostname;
            if (osl_getLocalHostname(&hostname.pData) != osl_Socket_Ok
                || (rtl_ustr_compareIgnoreAsciiCase_WithLength(
                        url.pData->buffer + i, j - i, hostname.getStr(), hostname.getLength())
                    != 0))
            {
                return osl_File_E_INVAL;
            }
        }
        i = j;
    }
    // Handle empty path:
    if (i == url.getLength())
    {
        *path = "/";
        return osl_File_E_None;
    }
    // Path must not contain %2F:
    if (url.indexOf("%2F", i) != -1 || url.indexOf("%2f", i) != -1)
        return osl_File_E_INVAL;

    *path = rtl::Uri::decode(
        url.copy(i), rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8);
    // Path must not contain %2F:
    if (path->indexOf('\0') != -1)
        return osl_File_E_INVAL;

    // Handle ~ notation:
    if (resolveHome && path->getLength() >= 2 && (*path)[1] == '~')
    {
        sal_Int32 j = path->indexOf('/', 2);
        if (j == -1)
            j = path->getLength();

        if (j == 2)
        {
            OUString home;
            if (!osl::Security().getHomeDir(home))
            {
                SAL_WARN("sal.file", "osl::Security::getHomeDir failed");
                return osl_File_E_INVAL;
            }

            i = url.indexOf('/', i + 1);

            if (i == -1)
                i = url.getLength();
            else
                ++i;

            //TODO: cheesy way of ensuring home's path ends in slash:
            if (!home.isEmpty() && home[home.getLength() - 1] != '/')
                home += "/";
            try
            {
                home = rtl::Uri::convertRelToAbs(home, url.copy(i));
            }
            catch (rtl::MalformedUriException & e)
            {
                SAL_WARN("sal.file", "rtl::MalformedUriException " << e.getMessage());
                return osl_File_E_INVAL;
            }
            return getSystemPathFromFileUrl(home, path, false);
        }
        // FIXME: replace ~user with user's home directory
        return osl_File_E_INVAL;
    }
    return osl_File_E_None;
}

}

oslFileError SAL_CALL osl_getSystemPathFromFileURL( rtl_uString *ustrFileURL, rtl_uString **pustrSystemPath )
{
    OUString path;
    oslFileError e;
    try
    {
        e = getSystemPathFromFileUrl(
            OUString::unacquired(&ustrFileURL), &path, true);
    }
    catch (std::length_error &)
    {
        e = osl_File_E_RANGE;
    }

    if (e == osl_File_E_None)
        rtl_uString_assign(pustrSystemPath, path.pData);

    return e;
}

oslFileError SAL_CALL osl_getFileURLFromSystemPath( rtl_uString *ustrSystemPath, rtl_uString **pustrFileURL )
{
    static const sal_Unicode pDoubleSlash[2] = { '/', '/' };

    rtl_uString *pTmp = nullptr;
    sal_Int32 nIndex;

    if( ustrSystemPath->length == 0 )
        return osl_File_E_INVAL;

    /* temporary hack: if already file url, return ustrSystemPath */

    if( rtl_ustr_ascii_shortenedCompare_WithLength( ustrSystemPath->buffer, ustrSystemPath->length,"file:", 5 ) == 0 )
        return osl_File_E_INVAL;

    /* check if system path starts with ~ or ~user and replace it with the appropriate home dir */
    if( ustrSystemPath->buffer[0] == '~' )
    {
        /* check if another user is specified */
        if( ( ustrSystemPath->length == 1 ) ||
            ( ustrSystemPath->buffer[1] == '/' ) )
        {
            /* osl_getHomeDir returns file URL */
            oslSecurity pSecurity = osl_getCurrentSecurity();
            osl_getHomeDir( pSecurity , &pTmp );
            osl_freeSecurityHandle( pSecurity );

            if (!pTmp)
                return osl_File_E_INVAL;

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

    /* check if initial string contains repeated '/' characters */
    nIndex = rtl_ustr_indexOfStr_WithLength( ustrSystemPath->buffer, ustrSystemPath->length, pDoubleSlash, 2 );
    if( nIndex != -1 )
    {
        sal_Int32 nSrcIndex;
        sal_Int32 nDeleted = 0;

        /* if pTmp is not already allocated, copy ustrSystemPath for modification */
        if( pTmp == nullptr )
            rtl_uString_newFromString( &pTmp, ustrSystemPath );

        /* adapt index to pTmp */
        nIndex += pTmp->length - ustrSystemPath->length;

        /* replace repeated '/' characters with a single '/' */
        for( nSrcIndex = nIndex + 1; nSrcIndex < pTmp->length; nSrcIndex++ )
        {
            if( (pTmp->buffer[nSrcIndex] == '/') && (pTmp->buffer[nIndex] == '/') )
                nDeleted++;
            else
                pTmp->buffer[++nIndex] = pTmp->buffer[nSrcIndex];
        }

        /* adjust length member */
        pTmp->length -= nDeleted;
    }

    if( pTmp == nullptr )
        rtl_uString_assign( &pTmp, ustrSystemPath );

    /* file URLs must be URI encoded */
    rtl_uriEncode( pTmp, uriCharClass, rtl_UriEncodeIgnoreEscapes, RTL_TEXTENCODING_UTF8, pustrFileURL );

    rtl_uString_release( pTmp );

    /* absolute urls should start with 'file://' */
    if( (*pustrFileURL)->buffer[0] == '/' )
    {
        rtl_uString *pProtocol = nullptr;

        rtl_uString_newFromAscii( &pProtocol, "file://" );
        rtl_uString_newConcat( pustrFileURL, pProtocol, *pustrFileURL );
        rtl_uString_release( pProtocol );
    }

    return osl_File_E_None;
}

/*
 * relative URLs are not accepted
 */
oslFileError osl_getSystemPathFromFileURL_Ex(
    rtl_uString *ustrFileURL, rtl_uString **pustrSystemPath)
{
    rtl_uString* temp = nullptr;
    oslFileError osl_error = osl_getSystemPathFromFileURL(ustrFileURL, &temp);

    if (osl_error == osl_File_E_None)
    {
        if (temp->buffer[0] == '/')
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

    /** Helper function, return a pinter to the final '\0'
        of a string
    */

    sal_Unicode* ustrtoend(sal_Unicode* pStr)
    {
        return (pStr + rtl_ustr_getLength(pStr));
    }

    sal_Unicode* ustrchrcat(const sal_Unicode chr, sal_Unicode* d)
    {
        sal_Unicode* p = ustrtoend(d);
        *p++ = chr;
        *p   = 0;
        return d;
    }

    bool _islastchr(sal_Unicode* pStr, sal_Unicode Chr)
    {
           sal_Unicode* p = ustrtoend(pStr);
           if (p > pStr)
               p--;
           return (*p == Chr);
    }

    /**
     Remove the last part of a path, a path that has
     only a '/' or no '/' at all will be returned
     unmodified
    */

    sal_Unicode* _rmlastpathtoken(sal_Unicode* aPath)
    {
        /* we may always skip -2 because we
           may at least stand on a '/' but
           either there is no other character
           before this '/' or it's another
           character than the '/'
        */
        sal_Unicode* p = ustrtoend(aPath) - 2;

        /* move back to the next path separator
           or to the start of the string */
        while ((p > aPath) && (*p != '/'))
            p--;

        if (p >= aPath)
        {
            if (*p == '/')
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

    oslFileError _osl_resolvepath(
        /*inout*/ sal_Unicode* path,
        /*inout*/ bool* failed)
    {
        oslFileError ferr = osl_File_E_None;

        if (!*failed)
        {
            char unresolved_path[PATH_MAX];
            if (!UnicodeToText(unresolved_path, sizeof(unresolved_path), path, rtl_ustr_getLength(path)))
                return oslTranslateFileError(ENAMETOOLONG);

            char resolved_path[PATH_MAX];
            if (realpath(unresolved_path, resolved_path))
            {
                if (!TextToUnicode(resolved_path, strlen(resolved_path), path, PATH_MAX))
                    return oslTranslateFileError(ENAMETOOLONG);
            }
            else
            {
                if (EACCES == errno || ENOTDIR == errno || ENOENT == errno)
                    *failed = true;
                else
                    ferr = oslTranslateFileError(errno);
            }
        }

        return ferr;
    }

    /**
       Works even with non existing paths. The resulting path must not exceed
       PATH_MAX else osl_File_E_NAMETOOLONG is the result
    */

    oslFileError osl_getAbsoluteFileURL_impl_(const OUString& unresolved_path, OUString& resolved_path)
    {
        /* the given unresolved path must not exceed PATH_MAX */
        if (unresolved_path.getLength() >= (PATH_MAX - 2))
            return oslTranslateFileError(ENAMETOOLONG);

        sal_Unicode        path_resolved_so_far[PATH_MAX];
        const sal_Unicode* punresolved = unresolved_path.getStr();
        sal_Unicode*       presolvedsf = path_resolved_so_far;

        /* reserve space for leading '/' and trailing '\0'
           do not exceed this limit */
        sal_Unicode* sentinel = path_resolved_so_far + PATH_MAX - 2;

        /* if realpath fails with error ENOTDIR, EACCES or ENOENT
           we will not call it again, because _osl_realpath should also
           work with non existing directories etc. */
        bool realpath_failed = false;
        oslFileError ferr;

        path_resolved_so_far[0] = '\0';

        while (*punresolved != '\0')
        {
            /* ignore '/.' , skip one part back when '/..' */
            if ((*punresolved == '.') && (*presolvedsf == '/'))
            {
                if (*(punresolved + 1) == '\0')
                {
                    punresolved++;
                    continue;
                }
                if (*(punresolved + 1) == '/')
                {
                    punresolved += 2;
                    continue;
                }
                if ((*(punresolved + 1) == '.') && (*(punresolved + 2) == '\0' || (*(punresolved + 2) == '/')))
                {
                    _rmlastpathtoken(path_resolved_so_far);

                    presolvedsf = ustrtoend(path_resolved_so_far) - 1;

                    if (*(punresolved + 2) == '/')
                        punresolved += 3;
                    else
                        punresolved += 2;

                    continue;
                }

                /* a file or directory name may start with '.' */
                if ((presolvedsf = ustrtoend(path_resolved_so_far)) > sentinel)
                    return oslTranslateFileError(ENAMETOOLONG);

                ustrchrcat(*punresolved++, path_resolved_so_far);

                if (*punresolved == '\0' && !realpath_failed)
                {
                    ferr = _osl_resolvepath(
                        path_resolved_so_far,
                        &realpath_failed);

                    if (ferr != osl_File_E_None)
                        return ferr;
                }
            }
            else if (*punresolved == '/')
            {
                if ((presolvedsf = ustrtoend(path_resolved_so_far)) > sentinel)
                    return oslTranslateFileError(ENAMETOOLONG);

                ustrchrcat(*punresolved++, path_resolved_so_far);

                if (!realpath_failed)
                {
                    ferr = _osl_resolvepath(
                        path_resolved_so_far,
                        &realpath_failed);

                    if (ferr != osl_File_E_None)
                        return ferr;

                    if (!_islastchr(path_resolved_so_far, '/'))
                    {
                        if ((presolvedsf = ustrtoend(path_resolved_so_far)) > sentinel)
                            return oslTranslateFileError(ENAMETOOLONG);

                        ustrchrcat('/', path_resolved_so_far);
                    }
                }
            }
            else // any other character
            {
                if ((presolvedsf = ustrtoend(path_resolved_so_far)) > sentinel)
                    return oslTranslateFileError(ENAMETOOLONG);

                ustrchrcat(*punresolved++, path_resolved_so_far);

                if (*punresolved == '\0' && !realpath_failed)
                {
                    ferr = _osl_resolvepath(
                        path_resolved_so_far,
                        &realpath_failed);

                    if (ferr != osl_File_E_None)
                        return ferr;
                }
            }
        }

        sal_Int32 len = rtl_ustr_getLength(path_resolved_so_far);

        OSL_ASSERT(len < PATH_MAX);

        resolved_path = OUString(path_resolved_so_far, len);

        return osl_File_E_None;
    }

}

oslFileError osl_getAbsoluteFileURL(
        rtl_uString* ustrBaseDirURL,
        rtl_uString* ustrRelativeURL,
        rtl_uString** pustrAbsoluteURL)
{
    /* Work around the below call to getSystemPathFromFileURL rejecting input
       that starts with "/" (for whatever reason it behaves that way; but
       changing that would start to break lots of tests at least) */
    OUString relUrl(ustrRelativeURL);
    if (relUrl.startsWith("//"))
        relUrl = "file:" + relUrl;
    else if (relUrl.startsWith("/"))
        relUrl = "file://" + relUrl;

    OUString unresolved_path;

    FileBase::RC frc = FileBase::getSystemPathFromFileURL(relUrl, unresolved_path);
    if (frc != FileBase::E_None)
        return oslFileError(frc);

    if (systemPathIsRelativePath(unresolved_path))
    {
        OUString base_path;
        oslFileError rc = osl_getSystemPathFromFileURL_Ex(ustrBaseDirURL, &base_path.pData);
        if (rc != osl_File_E_None)
            return rc;

        OUString abs_path;
        systemPathMakeAbsolutePath(base_path, unresolved_path, abs_path);

        unresolved_path = abs_path;
    }

    OUString resolved_path;
    oslFileError rc = osl_getAbsoluteFileURL_impl_(unresolved_path, resolved_path);
    if (rc == osl_File_E_None)
    {
        rc = osl_getFileURLFromSystemPath(resolved_path.pData, pustrAbsoluteURL);
        OSL_ASSERT(osl_File_E_None == rc);
    }

    return rc;
}

namespace osl {
namespace detail {

    /**
     No separate error code if unicode to text conversion or getenv fails because for the
     caller there is no difference why a file could not be found in $PATH
    */
    bool find_in_PATH(const OUString& file_path, OUString& result)
    {
        bool bfound = false;
        OUString path("PATH");
        OUString env_path;

        if (osl_getEnvironment(path.pData, &env_path.pData) == osl_Process_E_None)
            bfound = osl::searchPath(file_path, env_path, result);

        return bfound;
    }
}
}

namespace
{
    /**
     No separate error code if unicode to text conversion or getcwd fails because for the
     caller there is no difference why a file could not be found in CDW
    */
    bool find_in_CWD(const OUString& file_path, OUString& result)
    {
        bool bfound = false;
        OUString cwd_url;

        if (osl_getProcessWorkingDir(&cwd_url.pData) == osl_Process_E_None)
        {
            OUString cwd;
            FileBase::getSystemPathFromFileURL(cwd_url, cwd);
            bfound = osl::searchPath(file_path, cwd, result);
        }
        return bfound;
    }

    bool find_in_searchPath(const OUString& file_path, rtl_uString* search_path, OUString& result)
    {
        return (search_path && osl::searchPath(file_path, OUString(search_path), result));
    }

}

oslFileError osl_searchFileURL(rtl_uString* ustrFilePath, rtl_uString* ustrSearchPath, rtl_uString** pustrURL)
{
    OSL_PRECOND(ustrFilePath && pustrURL, "osl_searchFileURL: invalid parameter");

    FileBase::RC  rc;
    OUString file_path;

    // try to interpret search path as file url else assume it's a system path list
    rc = FileBase::getSystemPathFromFileURL(ustrFilePath, file_path);
    if (rc == FileBase::E_INVAL)
        file_path = ustrFilePath;
    else if (rc != FileBase::E_None)
        return oslFileError(rc);

    bool          bfound = false;
    OUString result;

    if (find_in_searchPath(file_path, ustrSearchPath, result) ||
        osl::detail::find_in_PATH(file_path, result) ||
        find_in_CWD(file_path, result))
    {
        OUString resolved;

        if (osl::realpath(result, resolved))
        {
            oslFileError osl_error = osl_getFileURLFromSystemPath(resolved.pData, pustrURL);
            SAL_WARN_IF(osl_File_E_None != osl_error, "sal.file", "osl_getFileURLFromSystemPath failed");
            bfound = true;
        }
    }
    return bfound ? osl_File_E_None : osl_File_E_NOENT;
}

oslFileError FileURLToPath(char * buffer, size_t bufLen, rtl_uString* ustrFileURL)
{
    rtl_uString* ustrSystemPath = nullptr;
    oslFileError osl_error      = osl_getSystemPathFromFileURL(ustrFileURL, &ustrSystemPath);

    if(osl_error != osl_File_E_None)
        return osl_error;

    osl_systemPathRemoveSeparator(ustrSystemPath);

    /* convert unicode path to text */
    if(!UnicodeToText( buffer, bufLen, ustrSystemPath->buffer, ustrSystemPath->length))
        osl_error = oslTranslateFileError(errno);

    rtl_uString_release(ustrSystemPath);

    return osl_error;
}

namespace
{
    class UnicodeToTextConverter_Impl
    {
        rtl_UnicodeToTextConverter const m_converter;

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
            OSL_ASSERT(m_converter != nullptr);
            return rtl_convertUnicodeToText (
                m_converter, nullptr, pSrcBuf, nSrcChars, pDstBuf, nDstBytes, nFlags, pInfo, pSrcCvtChars);
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
        rtl_TextToUnicodeConverter const m_converter;

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
            OSL_ASSERT(m_converter != nullptr);
            return rtl_convertTextToUnicode (
                m_converter, nullptr, pSrcBuf, nSrcBytes, pDstBuf, nDstChars, nFlags, pInfo, pSrcCvtBytes);
        }
    };
}

int TextToUnicode(
    const char*  text,
    size_t text_buffer_size,
    sal_Unicode* unic_text,
    sal_Int32 unic_text_buffer_size)
{
    sal_uInt32 nInfo = 0;
    sal_Size nSrcChars = 0;

    sal_Size nDestBytes = TextToUnicodeConverter_Impl::getInstance().convert(
        text, text_buffer_size, unic_text, unic_text_buffer_size,
        OSTRING_TO_OUSTRING_CVTFLAGS | RTL_TEXTTOUNICODE_FLAGS_FLUSH, &nInfo, &nSrcChars);

    if (nInfo & RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOOSMALL)
    {
        errno = EOVERFLOW;
        return 0;
    }

    /* ensure trailing '\0' */
    unic_text[nDestBytes] = '\0';
    return nDestBytes;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
