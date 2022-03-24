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

#include <sal/config.h>
#include <sal/log.hxx>

#include <algorithm>
#include <optional>
#include <stack>
#include <string_view>

#include <systools/win32/uwinapi.h>

#include "file_url.hxx"
#include "file_error.hxx"

#include <rtl/alloc.h>
#include <rtl/character.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/mutex.h>
#include <o3tl/char16_t2wchar_t.hxx>

#include "path_helper.hxx"

#define WSTR_SYSTEM_ROOT_PATH               u"\\\\.\\"
#define WSTR_LONG_PATH_PREFIX               u"\\\\?\\"
#define WSTR_LONG_PATH_PREFIX_UNC           u"\\\\?\\UNC\\"

// FileURL functions

namespace
{
// Internal functions that expect only backslashes as path separators

bool startsWithDriveColon(const sal_Unicode* p) { return rtl::isAsciiAlpha(p[0]) && p[1] == ':'; }

bool startsWithDriveColon(const rtl_uString* p) { return startsWithDriveColon(p->buffer); }

bool startsWithDriveColonSlash(const rtl_uString* p)
{
    return startsWithDriveColon(p) && p->buffer[2] == '\\';
}

bool startsWithSlashSlash(const sal_Unicode* p) { return p[0] == '\\' && p[1] == '\\'; }

bool startsWithSlashSlash(const rtl_uString* p) { return startsWithSlashSlash(p->buffer); }

// An absolute path starts either with \\ (an UNC or device path like \\.\ or \\?\)
// or with a ASCII alpha character followed by colon followed by backslash.
bool isAbsolute(const rtl_uString* p)
{
    return startsWithSlashSlash(p) || startsWithDriveColonSlash(p);
}

bool onSameDrive(const rtl_uString* p1, const rtl_uString* p2)
{
    return rtl::toAsciiUpperCase(p1->buffer[0]) == rtl::toAsciiUpperCase(p2->buffer[0])
           && rtl::toAsciiUpperCase(p1->buffer[1]) == rtl::toAsciiUpperCase(p2->buffer[1]);
}

sal_Int32 getRootLength(const rtl_uString* path)
{
    assert(isAbsolute(path));
    const sal_Unicode* p = path->buffer;
    sal_Int32 nResult = 0;
    if (startsWithSlashSlash(p))
    {
        // Cases:
        //   1. Device UNC: \\?\UNC\server\share or \\.\UNC\server\share
        //   2. Non-device UNC: \\server\share
        //   3. Device non-UNC: \\?\C: or \\.\C:
        bool bUNC = false;
        if ((p[2] == '.' || p[2] == '?') && p[3] == '\\')
        {
            if (p[4] == 'U' && p[5] == 'N' && p[6] == 'C' && p[7] == '\\')
            {
                // \\?\UNC\server\share or \\.\UNC\server\share
                nResult = 8;
                bUNC = true;
            }
            else
            {
                // \\?\C: or \\.\C:
                assert(startsWithDriveColon(p + 4));
                nResult = 6;
            }
        }
        else
        {
            // \\server\share
            nResult = 2;
            bUNC = true;
        }
        if (bUNC)
        {
            // \\?\UNC\server\share or \\.\UNC\server\share or \\server\share
            assert(nResult < path->length && p[nResult] != '\\');
            // Skip server name and share name
            for (int nSlashes = 0; nResult < path->length; ++nResult)
            {
                if (p[nResult] == '\\' && ++nSlashes == 2)
                    break;
            }
        }
    }
    else
    {
        // C:
        assert(startsWithDriveColon(p));
        nResult = 2;
    }
    return std::min(nResult, path->length);
}

std::u16string_view pathView(const rtl_uString* path, bool bOnlyRoot)
{
    return std::u16string_view(path->buffer, bOnlyRoot ? getRootLength(path) : path->length);
}

OUString combinePath(std::u16string_view basePath, const sal_Unicode* relPath)
{
    const bool needSep = basePath.back() != '\\' && relPath[0] != '\\';
    const auto sSeparator = needSep ? std::u16string_view(u"\\") : std::u16string_view();
    if (basePath.back() == '\\' && relPath[0] == '\\')
        ++relPath; // avoid two adjacent backslashes
    return OUString::Concat(basePath) + sSeparator + relPath;
}

OUString removeRelativeParts(const OUString& p)
{
    const sal_Int32 rootPos = getRootLength(p.pData);
    OUStringBuffer buf(p.getLength());
    buf.append(p.subView(0, rootPos));
    std::stack<sal_Int32> partPositions;
    bool bAfterSlash = false;
    for (sal_Int32 i = rootPos; i < p.getLength(); ++i)
    {
        sal_Unicode c = p[i];
        if (c == '\\')
        {
            if (i + 1 < p.getLength() && p[i + 1] == '.')
            {
                if (i + 2 == p.getLength() || p[i + 2] == '\\')
                {
                    // 1. Skip current directory (\.\ or trailing \.)
                    ++i; // process next slash: it may start another "\.\"
                }
                else if (p[i + 2] == '.' && (i + 3 == p.getLength() || p[i + 3] == '\\'))
                {
                    // 2. For parent directory (\..\), drop previous part and skip
                    if (bAfterSlash && partPositions.size())
                        partPositions.pop();
                    sal_Int32 nParentPos = partPositions.size() ? partPositions.top() : rootPos;
                    if (partPositions.size())
                        partPositions.pop();
                    buf.truncate(nParentPos);
                    bAfterSlash = false; // we have just removed slash after parent part
                    i += 2; // process next slash: it may start another "\.\"
                }
            }
            if (bAfterSlash)
                continue; // 3. Skip double backslashes (\\)
            partPositions.push(buf.getLength());
            bAfterSlash = true;
        }
        else
            bAfterSlash = false;

        buf.append(c);
    }
    return buf.makeStringAndClear();
}
}

static bool IsValidFilePathComponent(
    sal_Unicode const * lpComponent, sal_Unicode const **lppComponentEnd,
    DWORD dwFlags)
{
        sal_Unicode const * lpComponentEnd = nullptr;
        sal_Unicode const * lpCurrent = lpComponent;
        bool    bValid = true;  /* Assume success */
        sal_Unicode cLast = 0;

        /* Path component length must not exceed MAX_PATH even if long path with "\\?\" prefix is used */

        while ( !lpComponentEnd && lpCurrent && lpCurrent - lpComponent < MAX_PATH )
        {
            switch ( *lpCurrent )
            {
                /* Both backslash and slash determine the end of a path component */
            case '\0':
            case '/':
            case '\\':
                switch ( cLast )
                {
                    /* Component must not end with '.' or blank and can't be empty */

                case '.':
                    if ( dwFlags & VALIDATEPATH_ALLOW_ELLIPSE )
                    {
                        if ( (dwFlags & VALIDATEPATH_ALLOW_INVALID_SPACE_AND_PERIOD) ||
                             1 == lpCurrent - lpComponent )
                        {
                            /* Either do allow periods anywhere, or current directory */
                            lpComponentEnd = lpCurrent;
                            break;
                        }
                        else if ( 2 == lpCurrent - lpComponent && '.' == *lpComponent )
                        {
                            /* Parent directory is O.K. */
                            lpComponentEnd = lpCurrent;
                            break;
                        }
                    }
                    [[fallthrough]];
                case 0:
                case ' ':
                    if ( dwFlags & VALIDATEPATH_ALLOW_INVALID_SPACE_AND_PERIOD )
                        lpComponentEnd = lpCurrent;
                    else
                    {
                        lpComponentEnd = lpCurrent - 1;
                        bValid = false;
                    }
                    break;
                default:
                    lpComponentEnd = lpCurrent;
                    break;
                }
                break;
                /* The following characters are reserved */
            case '?':
            case '*':
            case '<':
            case '>':
            case '\"':
            case '|':
            case ':':
                lpComponentEnd = lpCurrent;
                bValid = false;
                break;
            default:
                /* Characters below ASCII 32 are not allowed */
                if ( *lpCurrent < ' ' )
                {
                    lpComponentEnd = lpCurrent;
                    bValid = false;
                }
                break;
            }
            cLast = *lpCurrent++;
        }

        /*  If we don't reached the end of the component the length of the component was too long
            (See condition of while loop) */
        if ( !lpComponentEnd )
        {
            bValid = false;
            lpComponentEnd = lpCurrent;
        }

        if ( bValid )
        {
            // Empty components are not allowed
            if ( lpComponentEnd - lpComponent < 1 )
                bValid = false;

            // If we reached the end of the string nullptr is returned
            else if ( !*lpComponentEnd )
                lpComponentEnd = nullptr;

        }

        if ( lppComponentEnd )
            *lppComponentEnd = lpComponentEnd;

        return bValid;
}

static sal_Int32 countInitialSeparators(sal_Unicode const * path) {
    sal_Unicode const * p = path;
    while (*p == '\\' || *p == '/') {
        ++p;
    }
    return p - path;
}

DWORD IsValidFilePath(rtl_uString *path, DWORD dwFlags, rtl_uString **corrected)
{
        sal_Unicode const * lpszPath = path->buffer;
        sal_Unicode const * lpComponent = lpszPath;
        bool    bValid = true;
        DWORD   dwPathType = PATHTYPE_ERROR;
        sal_Int32 nLength = rtl_uString_getLength( path );

        if ( dwFlags & VALIDATEPATH_ALLOW_RELATIVE )
            dwFlags |= VALIDATEPATH_ALLOW_ELLIPSE;

        DWORD   dwCandidatPathType = PATHTYPE_ERROR;

        if ( 0 == rtl_ustr_shortenedCompareIgnoreAsciiCase_WithLength( path->buffer, nLength, WSTR_LONG_PATH_PREFIX_UNC, SAL_N_ELEMENTS(WSTR_LONG_PATH_PREFIX_UNC) - 1, SAL_N_ELEMENTS(WSTR_LONG_PATH_PREFIX_UNC) - 1 ) )
        {
            /* This is long path in UNC notation */
            lpComponent = lpszPath + SAL_N_ELEMENTS(WSTR_LONG_PATH_PREFIX_UNC) - 1;
            dwCandidatPathType = PATHTYPE_ABSOLUTE_UNC | PATHTYPE_IS_LONGPATH;
        }
        else if ( 0 == rtl_ustr_shortenedCompareIgnoreAsciiCase_WithLength( path->buffer, nLength, WSTR_LONG_PATH_PREFIX, SAL_N_ELEMENTS(WSTR_LONG_PATH_PREFIX) - 1, SAL_N_ELEMENTS(WSTR_LONG_PATH_PREFIX) - 1 ) )
        {
            /* This is long path */
            lpComponent = lpszPath + SAL_N_ELEMENTS(WSTR_LONG_PATH_PREFIX) - 1;

            if (startsWithDriveColon(lpComponent))
            {
                lpComponent += 2;
                dwCandidatPathType = PATHTYPE_ABSOLUTE_LOCAL | PATHTYPE_IS_LONGPATH;
            }
        }
        else if ( 2 == countInitialSeparators( lpszPath ) )
        {
            /* The UNC path notation */
            lpComponent = lpszPath + 2;
            dwCandidatPathType = PATHTYPE_ABSOLUTE_UNC;
        }
        else if (startsWithDriveColon(lpszPath))
        {
            /* Local path verification. Must start with <drive>: */
            lpComponent = lpszPath + 2;
            dwCandidatPathType = PATHTYPE_ABSOLUTE_LOCAL;
        }

        if ( ( dwCandidatPathType & PATHTYPE_MASK_TYPE ) == PATHTYPE_ABSOLUTE_UNC )
        {
            bValid = IsValidFilePathComponent( lpComponent, &lpComponent, VALIDATEPATH_ALLOW_ELLIPSE );

            /* So far we have a valid servername. Now let's see if we also have a network resource */

            dwPathType = dwCandidatPathType;

            if ( bValid )
            {
                if ( lpComponent &&  !*++lpComponent )
                    lpComponent = nullptr;

                if ( !lpComponent )
                {
                    dwPathType |= PATHTYPE_IS_SERVER;
                }
                else
                {
                    /* Now test the network resource */

                    bValid = IsValidFilePathComponent( lpComponent, &lpComponent, 0 );

                    /* If we now reached the end of the path, everything is O.K. */

                    if ( bValid && (!lpComponent || !*++lpComponent ) )
                    {
                        lpComponent = nullptr;
                        dwPathType |= PATHTYPE_IS_VOLUME;
                    }
                }
            }
        }
        else if (  ( dwCandidatPathType & PATHTYPE_MASK_TYPE ) == PATHTYPE_ABSOLUTE_LOCAL )
        {
            if ( 1 == countInitialSeparators( lpComponent ) )
                lpComponent++;
            else if ( *lpComponent )
                bValid = false;

            dwPathType = dwCandidatPathType;

            /* Now we are behind the backslash or it was a simple drive without backslash */

            if ( bValid && !*lpComponent )
            {
                lpComponent = nullptr;
                dwPathType |= PATHTYPE_IS_VOLUME;
            }
        }
        else if ( dwFlags & VALIDATEPATH_ALLOW_RELATIVE )
        {
            /* Can be a relative path */
            lpComponent = lpszPath;

            /* Relative path can start with a backslash */

            if ( 1 == countInitialSeparators( lpComponent ) )
            {
                lpComponent++;
                if ( !*lpComponent )
                    lpComponent = nullptr;
            }

            dwPathType = PATHTYPE_RELATIVE;
        }
        else
        {
            /* Anything else is an error */
            bValid = false;
            lpComponent = lpszPath;
        }

        /* Now validate each component of the path */
        rtl_uString * lastCorrected = path;
        while ( bValid && lpComponent )
        {
            // Correct path by merging consecutive slashes:
            if (*lpComponent == '\\' && corrected != nullptr) {
                sal_Int32 i = lpComponent - lpszPath;
                rtl_uString_newReplaceStrAt(corrected, lastCorrected, i, 1, nullptr);
                    //TODO: handle out-of-memory
                lastCorrected = *corrected;
                lpszPath = (*corrected)->buffer;
                lpComponent = lpszPath + i;
            }

            bValid = IsValidFilePathComponent( lpComponent, &lpComponent, dwFlags | VALIDATEPATH_ALLOW_INVALID_SPACE_AND_PERIOD);

            if ( bValid && lpComponent )
            {
                lpComponent++;

                /* If the string behind the backslash is empty, we've done */

                if ( !*lpComponent )
                    lpComponent = nullptr;
            }
        }

        /* The path can be longer than MAX_PATH only in case it has the longpath prefix */
        if ( bValid && !( dwPathType &  PATHTYPE_IS_LONGPATH ) && rtl_ustr_getLength( lpszPath ) >= MAX_PATH )
        {
            bValid = false;
        }

        return bValid ? dwPathType : PATHTYPE_ERROR;
}

static std::optional<OUString> osl_decodeURL_(const OString& sUTF8)
{
    const char  *pSrcEnd;
    const char  *pSrc;
    bool        bValidEncoded = true;   /* Assume success */

    /* The resulting decoded string length is shorter or equal to the source length */

    const sal_Int32 nSrcLen = sUTF8.getLength();
    OStringBuffer aBuffer(nSrcLen + 1);

    pSrc = sUTF8.getStr();
    pSrcEnd = pSrc + nSrcLen;

    /* Now decode the URL what should result in a UTF-8 string */
    while ( bValidEncoded && pSrc < pSrcEnd )
    {
        switch ( *pSrc )
        {
        case '%':
            {
                char    aToken[3];
                char    aChar;

                pSrc++;
                aToken[0] = *pSrc++;
                aToken[1] = *pSrc++;
                aToken[2] = 0;

                aChar = static_cast<char>(strtoul( aToken, nullptr, 16 ));

                /* The chars are path delimiters and must not be encoded */

                if ( 0 == aChar || '\\' == aChar || '/' == aChar || ':' == aChar )
                    bValidEncoded = false;
                else
                    aBuffer.append(aChar);
            }
            break;
        case '\0':
        case '#':
        case '?':
            bValidEncoded = false;
            break;
        default:
            aBuffer.append(*pSrc++);
            break;
        }
    }

    return bValidEncoded ? OUString(aBuffer.getStr(), aBuffer.getLength(), RTL_TEXTENCODING_UTF8)
                         : std::optional<OUString>();
}

static OUString osl_encodeURL_(std::u16string_view sURL)
{
    /* Encode non ascii characters within the URL */

    const char     *pURLScan;
    sal_Int32       nURLScanLen;
    sal_Int32       nURLScanCount;

    OString sUTF8 = OUStringToOString(sURL, RTL_TEXTENCODING_UTF8);

    OUStringBuffer sEncodedURL(sUTF8.getLength() * 3 + 1);
    pURLScan = sUTF8.getStr();
    nURLScanLen = sUTF8.getLength();
    nURLScanCount = 0;

    while ( nURLScanCount < nURLScanLen )
    {
        char cCurrent = *pURLScan;
        switch ( cCurrent )
        {
        default:
            if (!( ( cCurrent >= 'a' && cCurrent <= 'z' ) || ( cCurrent >= 'A' && cCurrent <= 'Z' ) || ( cCurrent >= '0' && cCurrent <= '9' ) ) )
            {
                char buf[3];
                sprintf( buf, "%02X", static_cast<unsigned char>(cCurrent) );
                sEncodedURL.append('%').appendAscii(buf, 2);
                break;
            }
            [[fallthrough]];
        case '!':
        case '\'':
        case '(':
        case ')':
        case '*':
        case '-':
        case '.':
        case '_':
        case '~':
        case '$':
        case '&':
        case '+':
        case ',':
        case '=':
        case '@':
        case ':':
        case '/':
        case '\\':
        case '|':
            sEncodedURL.appendAscii(&cCurrent, 1);
            break;
        case 0:
            break;
        }

        pURLScan++;
        nURLScanCount++;
    }

    return sEncodedURL.makeStringAndClear();
}

oslFileError osl_getSystemPathFromFileURL_( rtl_uString *strURL, rtl_uString **pustrPath, bool bAllowRelative )
{
    OUString sTempPath;
    oslFileError        nError = osl_File_E_INVAL;  /* Assume failure */

    /*  If someone hasn't encoded the complete URL we convert it to UTF8 now to prevent from
        having a mixed encoded URL later */

    OString sUTF8 = OUStringToOString(OUString::unacquired(&strURL), RTL_TEXTENCODING_UTF8);

    /* If the length of strUTF8 and strURL differs it indicates that the URL was not correct encoded */

    SAL_WARN_IF(
        sUTF8.getLength() != strURL->length &&
        0 == rtl_ustr_ascii_shortenedCompareIgnoreAsciiCase_WithLength( strURL->buffer, strURL->length, "file:\\", 6 )
        , "sal.osl"
        ,"osl_getSystemPathFromFileURL: \"" << OUString(strURL) << "\" is not encoded !!!");

    if (auto sDecodedURL = osl_decodeURL_(sUTF8))
    {
        /* Replace backslashes and pipes */

        sDecodedURL = sDecodedURL->replace('/', '\\').replace('|', ':');

        /* Must start with "file:/" */
        if ( sDecodedURL->startsWithIgnoreAsciiCase("file:\\") )
        {
            sal_uInt32  nSkip;

            if ( sDecodedURL->startsWithIgnoreAsciiCase("file:\\\\\\") )
                nSkip = 8;
            else if (
                sDecodedURL->startsWithIgnoreAsciiCase("file:\\\\localhost\\") ||
                sDecodedURL->startsWithIgnoreAsciiCase("file:\\\\127.0.0.1\\")
                      )
                nSkip = 17;
            else if ( sDecodedURL->startsWithIgnoreAsciiCase("file:\\\\") )
                nSkip = 5;
            else
                nSkip = 6;

            const sal_uInt32 nDecodedLen = sDecodedURL->getLength();

            /* Indicates local root */
            if ( nDecodedLen == nSkip )
                sTempPath = WSTR_SYSTEM_ROOT_PATH;
            else
            {
                /* do not separate the directory and file case, so the maximal path length without prefix is MAX_PATH-12 */
                if ( nDecodedLen - nSkip <= MAX_PATH - 12 )
                {
                    sTempPath = sDecodedURL->subView(nSkip);
                }
                else
                {
                    ::osl::LongPathBuffer< sal_Unicode > aBuf( MAX_LONG_PATH );
                    sal_uInt32 nNewLen = GetLongPathNameW( o3tl::toW(sDecodedURL->getStr()) + nSkip,
                                                                 o3tl::toW(aBuf),
                                                                 aBuf.getBufSizeInSymbols() );

                    if ( nNewLen <= MAX_PATH - 12
                      || sDecodedURL->matchIgnoreAsciiCase(WSTR_SYSTEM_ROOT_PATH, nSkip)
                      || sDecodedURL->matchIgnoreAsciiCase(WSTR_LONG_PATH_PREFIX, nSkip) )
                    {
                        sTempPath = std::u16string_view(aBuf, nNewLen);
                    }
                    else if ( sDecodedURL->match("\\\\", nSkip) )
                    {
                        /* it should be an UNC path, use the according prefix */
                        sTempPath = OUString::Concat(WSTR_LONG_PATH_PREFIX_UNC) + std::u16string_view(aBuf + 2, nNewLen - 2);
                    }
                    else
                    {
                        sTempPath = OUString::Concat(WSTR_LONG_PATH_PREFIX) + std::u16string_view(aBuf, nNewLen);
                    }
                }
            }

            if ( IsValidFilePath( sTempPath.pData, VALIDATEPATH_ALLOW_ELLIPSE, &sTempPath.pData ) )
                nError = osl_File_E_None;
        }
        else if ( bAllowRelative )  /* This maybe a relative file URL */
        {
            /* In future the relative path could be converted to absolute if it is too long */
            sTempPath = *sDecodedURL;

            if ( IsValidFilePath( sTempPath.pData, VALIDATEPATH_ALLOW_RELATIVE | VALIDATEPATH_ALLOW_ELLIPSE, &sTempPath.pData ) )
                nError = osl_File_E_None;
        }
        else
          SAL_INFO_IF(nError, "sal.osl",
              "osl_getSystemPathFromFileURL: \"" << OUString(strURL) << "\" is not an absolute FileURL");

    }

    if ( osl_File_E_None == nError )
        rtl_uString_assign( pustrPath, sTempPath.pData );

    SAL_INFO_IF(nError, "sal.osl",
        "osl_getSystemPathFromFileURL: \"" << OUString(strURL) << "\" is not a FileURL");

    return nError;
}

oslFileError osl_getFileURLFromSystemPath( rtl_uString* strPath, rtl_uString** pstrURL )
{
    oslFileError nError = osl_File_E_INVAL; /* Assume failure */
    OUString sTempURL;
    DWORD dwPathType = PATHTYPE_ERROR;

    if (strPath)
        dwPathType = IsValidFilePath(strPath, VALIDATEPATH_ALLOW_RELATIVE, nullptr);

    if (dwPathType)
    {
        OUString sTempPath;
        const OUString& sPath = OUString::unacquired(&strPath);

        if ( dwPathType & PATHTYPE_IS_LONGPATH )
        {
            /* the path has the longpath prefix, lets remove it */
            switch ( dwPathType & PATHTYPE_MASK_TYPE )
            {
                case PATHTYPE_ABSOLUTE_UNC:
                    static_assert(SAL_N_ELEMENTS(WSTR_LONG_PATH_PREFIX_UNC) - 1 == 8,
                                  "Unexpected long path UNC prefix!");

                    /* generate the normal UNC path */
                    sTempPath = "\\\\" + sPath.copy(8).replace('\\', '/');
                    break;

                case PATHTYPE_ABSOLUTE_LOCAL:
                    static_assert(SAL_N_ELEMENTS(WSTR_LONG_PATH_PREFIX) - 1 == 4,
                                  "Unexpected long path prefix!");

                    /* generate the normal path */
                    sTempPath = sPath.copy(4).replace('\\', '/');
                    break;

                default:
                    OSL_FAIL( "Unexpected long path format!" );
                    sTempPath = sPath.replace('\\', '/');
                    break;
            }
        }
        else
        {
            /* Replace backslashes */
            sTempPath = sPath.replace('\\', '/');
        }

        switch ( dwPathType & PATHTYPE_MASK_TYPE )
        {
        case PATHTYPE_RELATIVE:
            sTempURL = sTempPath;
            nError = osl_File_E_None;
            break;
        case PATHTYPE_ABSOLUTE_UNC:
            sTempURL = "file:" + sTempPath;
            nError = osl_File_E_None;
            break;
        case PATHTYPE_ABSOLUTE_LOCAL:
            sTempURL = "file:///" + sTempPath;
            nError = osl_File_E_None;
            break;
        default:
            break;
        }
    }

    if ( osl_File_E_None == nError )
    {
        /* Encode the URL */
        rtl_uString_assign(pstrURL, osl_encodeURL_(sTempURL).pData);
        OSL_ASSERT(*pstrURL != nullptr);
    }

    SAL_INFO_IF(nError, "sal.osl",
        "osl_getFileURLFromSystemPath: \"" << OUString(strPath) << "\" is not a systemPath");
    return nError;
}

oslFileError SAL_CALL osl_getSystemPathFromFileURL(
    rtl_uString *ustrURL, rtl_uString **pustrPath)
{
    return osl_getSystemPathFromFileURL_( ustrURL, pustrPath, true );
}

oslFileError SAL_CALL osl_searchFileURL(
    rtl_uString *ustrFileName,
    rtl_uString *ustrSystemSearchPath,
    rtl_uString **pustrPath)
{
    rtl_uString     *ustrUNCPath = nullptr;
    rtl_uString     *ustrSysPath = nullptr;
    oslFileError    error;

    /* First try to interpret the file name as a URL even a relative one */
    error = osl_getSystemPathFromFileURL_( ustrFileName, &ustrUNCPath, true );

    /* So far we either have an UNC path or something invalid
       Now create a system path */
    if ( osl_File_E_None == error )
        error = osl_getSystemPathFromFileURL_( ustrUNCPath, &ustrSysPath, true );

    if ( osl_File_E_None == error )
    {
        DWORD   nBufferLength;
        DWORD   dwResult;
        LPWSTR  lpBuffer = nullptr;
        LPWSTR  lpszFilePart;

        /* Repeat calling SearchPath ...
           Start with MAX_PATH for the buffer. In most cases this
           will be enough and does not force the loop to run twice */
        dwResult = MAX_PATH;

        do
        {
            /* If search path is empty use a nullptr pointer instead according to MSDN documentation of SearchPath */
            LPCWSTR lpszSearchPath = ustrSystemSearchPath && ustrSystemSearchPath->length ? o3tl::toW(ustrSystemSearchPath->buffer) : nullptr;
            LPCWSTR lpszSearchFile = o3tl::toW(ustrSysPath->buffer);

            /* Allocate space for buffer according to previous returned count of required chars */
            /* +1 is not necessary if we follow MSDN documentation but for robustness we do so */
            nBufferLength = dwResult + 1;
            lpBuffer = lpBuffer ?
                static_cast<LPWSTR>(realloc(lpBuffer, nBufferLength * sizeof(WCHAR))) :
                static_cast<LPWSTR>(malloc(nBufferLength * sizeof(WCHAR)));

            dwResult = SearchPathW( lpszSearchPath, lpszSearchFile, nullptr, nBufferLength, lpBuffer, &lpszFilePart );
        } while ( dwResult && dwResult >= nBufferLength );

        /*  ... until an error occurs or buffer is large enough.
            dwResult == nBufferLength can not happen according to documentation but lets be robust ;-) */

        if ( dwResult )
        {
            rtl_uString_newFromStr( &ustrSysPath, o3tl::toU(lpBuffer) );
            error = osl_getFileURLFromSystemPath( ustrSysPath, pustrPath );
        }
        else
        {
            WIN32_FIND_DATAW aFindFileData;
            HANDLE  hFind;

            /* something went wrong, perhaps the path was absolute */
            error = oslTranslateFileError( GetLastError() );

            hFind = FindFirstFileW( o3tl::toW(ustrSysPath->buffer), &aFindFileData );

            if ( IsValidHandle(hFind) )
            {
                error = osl_getFileURLFromSystemPath( ustrSysPath, pustrPath );
                FindClose( hFind );
            }
        }

        free( lpBuffer );
    }

    if ( ustrSysPath )
        rtl_uString_release( ustrSysPath );

    if ( ustrUNCPath )
        rtl_uString_release( ustrUNCPath );

    return error;
}

oslFileError SAL_CALL osl_getAbsoluteFileURL( rtl_uString* ustrBaseURL, rtl_uString* ustrRelativeURL, rtl_uString** pustrAbsoluteURL )
{
    oslFileError eError = osl_File_E_None;
    rtl_uString     *ustrRelSysPath = nullptr;
    rtl_uString     *ustrBaseSysPath = nullptr;

    if ( ustrBaseURL && ustrBaseURL->length )
    {
        eError = osl_getSystemPathFromFileURL_( ustrBaseURL, &ustrBaseSysPath, false );
        OSL_ENSURE( osl_File_E_None == eError, "osl_getAbsoluteFileURL called with relative or invalid base URL" );
    }
    if (eError == osl_File_E_None)
    {
        eError = osl_getSystemPathFromFileURL_(ustrRelativeURL, &ustrRelSysPath,
                                               ustrBaseSysPath != nullptr);
        OSL_ENSURE( osl_File_E_None == eError, "osl_getAbsoluteFileURL called with empty base URL and/or invalid relative URL" );
    }

    if ( !eError )
    {
        OUString sResultPath;
/*@@@ToDo
  The whole FileURL implementation should be merged
  with the rtl/uri class.
*/
        // If ustrRelSysPath is absolute, we don't need ustrBaseSysPath.
        if (ustrBaseSysPath && !isAbsolute(ustrRelSysPath))
        {
            // ustrBaseSysPath is known here to be a valid absolute path -> its first two characters
            // are ASCII (either alpha + colon, or double backslashes)

            // Don't use SetCurrentDirectoryW together with GetFullPathNameW, because:
            // (a) it needs synchronization and may affect threads that may access relative paths;
            // (b) it would give wrong results for non-existing base path (allowed by RFC2396).

            if (startsWithDriveColon(ustrRelSysPath))
            {
                // Special case: a path relative to a specific drive's current directory.
                // Should we error out here?

                // If ustrBaseSysPath is on the same drive as ustrRelSysPath, then take base path
                // as is; otherwise, use current directory on ustrRelSysPath's drive as base path
                if (onSameDrive(ustrRelSysPath, ustrBaseSysPath))
                {
                    sResultPath = combinePath(OUString::unacquired(&ustrBaseSysPath),
                                              ustrRelSysPath->buffer + 2);
                }
                else
                {
                    // Call GetFullPathNameW to get current directory on ustrRelSysPath's drive
                    wchar_t baseDrive[3] = { ustrRelSysPath->buffer[0], ':' }; // just "C:"
                    osl::LongPathBuffer<wchar_t> aBuf(MAX_LONG_PATH);
                    DWORD dwResult
                        = GetFullPathNameW(baseDrive, aBuf.getBufSizeInSymbols(), aBuf, nullptr);
                    if (dwResult)
                    {
                        if (dwResult >= aBuf.getBufSizeInSymbols())
                            eError = osl_File_E_INVAL;
                        else
                            sResultPath = combinePath(o3tl::toU(aBuf), ustrRelSysPath->buffer + 2);
                    }
                    else
                        eError = oslTranslateFileError(GetLastError());
                }
            }
            else
            {
                // Is this a rooted relative path (starting with a backslash)?
                // Then we need only root from base. E.g.,
                // ustrBaseSysPath is "\\server\share\path1\" and ustrRelSysPath is "\path2\to\file"
                //   => \\server\share\path2\to\file
                // ustrBaseSysPath is "D:\path1\" and ustrRelSysPath is "\path2\to\file"
                //   => D:\path2\to\file
                auto sBaseView(pathView(ustrBaseSysPath, ustrRelSysPath->buffer[0] == '\\'));
                sResultPath = combinePath(sBaseView, ustrRelSysPath->buffer);
            }
        }
        else
            sResultPath = OUString::unacquired(&ustrRelSysPath);

        if (eError == osl_File_E_None)
        {
            sResultPath = removeRelativeParts(sResultPath);
            eError = osl_getFileURLFromSystemPath(sResultPath.pData, pustrAbsoluteURL);
        }
    }

    if ( ustrBaseSysPath )
        rtl_uString_release( ustrBaseSysPath );

    if ( ustrRelSysPath )
        rtl_uString_release( ustrRelSysPath );

    return  eError;
}

oslFileError SAL_CALL osl_getCanonicalName( rtl_uString *strRequested, rtl_uString **strValid )
{
    rtl_uString_newFromString(strValid, strRequested);
    return osl_File_E_None;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
