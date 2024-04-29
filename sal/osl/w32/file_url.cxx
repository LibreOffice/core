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
#include <o3tl/string_view.hxx>

#include "path_helper.hxx"

// FileURL functions

namespace
{
constexpr std::u16string_view WSTR_SYSTEM_ROOT_PATH = u"\\\\.\\";
constexpr std::u16string_view WSTR_LONG_PATH_PREFIX = u"\\\\?\\";
constexpr std::u16string_view WSTR_LONG_PATH_PREFIX_UNC = u"\\\\?\\UNC\\";

// Internal functions that expect only backslashes as path separators

bool startsWithDriveColon(std::u16string_view s)
{
    return s.length() >= 2 && rtl::isAsciiAlpha(s[0]) && s[1] == ':';
}

bool startsWithDriveColonSlash(std::u16string_view s)
{
    return s.length() >= 3 && startsWithDriveColon(s) && s[2] == '\\';
}

bool startsWithSlashSlash(std::u16string_view s) { return o3tl::starts_with(s, u"\\\\"); }

// An absolute path starts either with \\ (an UNC or device path like \\.\ or \\?\)
// or with a ASCII alpha character followed by colon followed by backslash.
bool isAbsolute(std::u16string_view s)
{
    return startsWithSlashSlash(s) || startsWithDriveColonSlash(s);
}

bool onSameDrive(std::u16string_view s1, std::u16string_view s2)
{
    assert(startsWithDriveColon(s1) && startsWithDriveColon(s2));
    return rtl::toAsciiUpperCase(s1[0]) == rtl::toAsciiUpperCase(s2[0]) && s1[1] == s2[1];
}

sal_Int32 getRootLength(std::u16string_view path)
{
    assert(isAbsolute(path));
    size_t nResult = 0;
    if (startsWithSlashSlash(path))
    {
        // Cases:
        //   1. Device UNC: \\?\UNC\server\share or \\.\UNC\server\share
        //   2. Non-device UNC: \\server\share
        //   3. Device non-UNC: \\?\C: or \\.\C:
        bool bUNC = false;
        if (path.length() > 3 && (path[2] == '.' || path[2] == '?') && path[3] == '\\')
        {
            if (path.substr(4, 4) == u"UNC\\")
            {
                // \\?\UNC\server\share or \\.\UNC\server\share
                nResult = 8;
                bUNC = true;
            }
            else
            {
                // \\?\C: or \\.\C:
                assert(startsWithDriveColon(path.substr(4)));
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
            assert(nResult < path.length() && path[nResult] != '\\');
            // Skip server name and share name
            for (int nSlashes = 0; nResult < path.length(); ++nResult)
            {
                if (path[nResult] == '\\' && ++nSlashes == 2)
                    break;
            }
        }
    }
    else
    {
        // C:
        assert(startsWithDriveColon(path));
        nResult = 2;
    }
    return std::min(nResult, path.length());
}

std::u16string_view pathView(std::u16string_view path, bool bOnlyRoot)
{
    return bOnlyRoot ? path.substr(0, getRootLength(path)) : path;
}

OUString combinePath(std::u16string_view basePath, std::u16string_view relPath)
{
    const bool needSep = !o3tl::ends_with(basePath, u'\\');
    const auto sSeparator = needSep ? std::u16string_view(u"\\") : std::u16string_view();
    if (o3tl::starts_with(relPath, u'\\'))
        relPath.remove_prefix(1); // avoid two adjacent backslashes
    return OUString::Concat(basePath) + sSeparator + relPath;
}

OUString removeRelativeParts(const OUString& p)
{
    const sal_Int32 rootPos = getRootLength(p);
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
    std::optional<std::u16string_view>& roComponent,
    DWORD dwFlags)
{
        assert(roComponent);
        auto lpComponentEnd = roComponent->end();
        auto lpCurrent = roComponent->begin();
        bool bValid = lpCurrent != lpComponentEnd; // Empty components are not allowed
        sal_Unicode cLast = 0;

        while (bValid)
        {
            /* Path component length must not exceed MAX_PATH even if long path with "\\?\" prefix is used */
            if (lpCurrent - roComponent->begin() >= MAX_PATH)
            {
                bValid = false;
                break;
            }

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
                             1 == lpCurrent - roComponent->begin() )
                        {
                            /* Either do allow periods anywhere, or current directory */
                            lpComponentEnd = lpCurrent;
                            break;
                        }
                        else if ( 2 == lpCurrent - roComponent->begin() && '.' == roComponent->front() )
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
                        bValid = false;
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
                bValid = false;
                break;
            default:
                /* Characters below ASCII 32 are not allowed */
                if ( *lpCurrent < ' ' )
                    bValid = false;
                break;
            }

            if (lpCurrent != lpComponentEnd)
                cLast = *lpCurrent++;

            if (lpCurrent == lpComponentEnd)
                break;
        }

        if ( bValid )
        {
            // Empty components are not allowed
            if (lpComponentEnd - roComponent->begin() < 1)
                bValid = false;
            // If we reached the end of the string nullopt is returned
            else if (lpComponentEnd == roComponent->end())
                roComponent.reset();
            else
                roComponent->remove_prefix(lpComponentEnd - roComponent->begin());
        }

        return bValid;
}

static sal_Int32 countInitialSeparators(std::u16string_view path) {
    size_t n = 0;
    while (n < path.length() && (path[n] == '\\' || path[n] == '/'))
        ++n;
    return n;
}

DWORD IsValidFilePath(const OUString& path, DWORD dwFlags, OUString* corrected)
{
        std::optional<std::u16string_view> oComponent = path;
        bool    bValid = true;
        DWORD   dwPathType = PATHTYPE_ERROR;

        if ( dwFlags & VALIDATEPATH_ALLOW_RELATIVE )
            dwFlags |= VALIDATEPATH_ALLOW_ELLIPSE;

        DWORD   dwCandidatPathType = PATHTYPE_ERROR;

        if (path.matchIgnoreAsciiCase(WSTR_LONG_PATH_PREFIX_UNC))
        {
            /* This is long path in UNC notation */
            oComponent = path.subView(WSTR_LONG_PATH_PREFIX_UNC.size());
            dwCandidatPathType = PATHTYPE_ABSOLUTE_UNC | PATHTYPE_IS_LONGPATH;
        }
        else if (path.matchIgnoreAsciiCase(WSTR_LONG_PATH_PREFIX))
        {
            /* This is long path */
            oComponent = path.subView(WSTR_LONG_PATH_PREFIX.size());

            if (startsWithDriveColon(*oComponent))
            {
                oComponent->remove_prefix(2);
                dwCandidatPathType = PATHTYPE_ABSOLUTE_LOCAL | PATHTYPE_IS_LONGPATH;
            }
        }
        else if ( 2 == countInitialSeparators(path) )
        {
            /* The UNC path notation */
            oComponent = path.subView(2);
            dwCandidatPathType = PATHTYPE_ABSOLUTE_UNC;
        }
        else if (startsWithDriveColon(path))
        {
            /* Local path verification. Must start with <drive>: */
            oComponent = path.subView(2);
            dwCandidatPathType = PATHTYPE_ABSOLUTE_LOCAL;
        }

        if ( ( dwCandidatPathType & PATHTYPE_MASK_TYPE ) == PATHTYPE_ABSOLUTE_UNC )
        {
            bValid = IsValidFilePathComponent(oComponent, VALIDATEPATH_ALLOW_ELLIPSE);

            /* So far we have a valid servername. Now let's see if we also have a network resource */

            dwPathType = dwCandidatPathType;

            if ( bValid )
            {
                if (oComponent)
                {
                    oComponent->remove_prefix(1);
                    if (oComponent->empty())
                        oComponent.reset();
                }

                if (!oComponent)
                {
                    dwPathType |= PATHTYPE_IS_SERVER;
                }
                else
                {
                    /* Now test the network resource */

                    bValid = IsValidFilePathComponent(oComponent, 0);

                    /* If we now reached the end of the path, everything is O.K. */

                    if (bValid)
                    {
                        if (oComponent)
                        {
                            oComponent->remove_prefix(1);
                            if (oComponent->empty())
                                oComponent.reset();
                        }
                        if (!oComponent)
                            dwPathType |= PATHTYPE_IS_VOLUME;
                    }
                }
            }
        }
        else if (  ( dwCandidatPathType & PATHTYPE_MASK_TYPE ) == PATHTYPE_ABSOLUTE_LOCAL )
        {
            if (1 == countInitialSeparators(*oComponent))
                oComponent->remove_prefix(1);
            else if (!oComponent->empty())
                bValid = false;

            dwPathType = dwCandidatPathType;

            /* Now we are behind the backslash or it was a simple drive without backslash */

            if (bValid && oComponent->empty())
            {
                oComponent.reset();
                dwPathType |= PATHTYPE_IS_VOLUME;
            }
        }
        else if ( dwFlags & VALIDATEPATH_ALLOW_RELATIVE )
        {
            /* Can be a relative path */
            oComponent = path;

            /* Relative path can start with a backslash */

            if (1 == countInitialSeparators(*oComponent))
            {
                oComponent->remove_prefix(1);
                if (oComponent->empty())
                    oComponent.reset();
            }

            dwPathType = PATHTYPE_RELATIVE;
        }
        else
        {
            /* Anything else is an error */
            bValid = false;
        }

        /* Now validate each component of the path */
        OUString lastCorrected = path;
        while (bValid && oComponent)
        {
            // Correct path by merging consecutive slashes:
            if (o3tl::starts_with(*oComponent, u"\\") && corrected != nullptr) {
                sal_Int32 i = oComponent->data() - lastCorrected.getStr();
                *corrected = lastCorrected.replaceAt(i, 1, {});
                    //TODO: handle out-of-memory
                lastCorrected = *corrected;
                oComponent = lastCorrected.subView(i);
            }

            bValid = IsValidFilePathComponent(oComponent, dwFlags | VALIDATEPATH_ALLOW_INVALID_SPACE_AND_PERIOD);

            if (bValid && oComponent)
            {
                oComponent->remove_prefix(1);

                /* If the string behind the backslash is empty, we've done */

                if (oComponent->empty())
                    oComponent.reset();
            }
        }

        /* The path can be longer than MAX_PATH only in case it has the longpath prefix */
        if (bValid && !(dwPathType & PATHTYPE_IS_LONGPATH) && path.getLength() >= MAX_PATH)
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

    if (!bValidEncoded)
        return std::nullopt;

    return OStringToOUString(aBuffer, RTL_TEXTENCODING_UTF8);
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

// A helper that makes sure that for existing part of the path, the case is correct.
// Unlike GetLongPathNameW that it wraps, this function does not require the path to exist.
static OUString GetCaseCorrectPathName(std::u16string_view sysPath)
{
    // Prepare a null-terminated string first.
    // Neither OUString, nor u16string_view are guaranteed to be null-terminated
    osl::LongPathBuffer<wchar_t> szPath(sysPath.size() + WSTR_LONG_PATH_PREFIX_UNC.size() + 1);
    wchar_t* const pPath = szPath;
    wchar_t* pEnd = pPath;
    size_t sysPathOffset = 0;
    if (sysPath.size() >= MAX_PATH && isAbsolute(sysPath)
        && !o3tl::starts_with(sysPath, WSTR_LONG_PATH_PREFIX))
    {
        // Allow GetLongPathNameW consume long paths
        std::u16string_view prefix = WSTR_LONG_PATH_PREFIX;
        if (startsWithSlashSlash(sysPath))
        {
            sysPathOffset = 2; // skip leading "\\"
            prefix = WSTR_LONG_PATH_PREFIX_UNC;
        }
        pEnd = std::copy(prefix.begin(), prefix.end(), pEnd);
    }
    wchar_t* const pStart = pEnd;
    pEnd = std::copy(sysPath.begin() + sysPathOffset, sysPath.end(), pStart);
    *pEnd = 0;
    osl::LongPathBuffer<wchar_t> aBuf(MAX_LONG_PATH);
    while (pEnd > pStart)
    {
        std::u16string_view curPath(o3tl::toU(pPath), pEnd - pPath);
        if (curPath == u"\\\\" || curPath == WSTR_SYSTEM_ROOT_PATH
            || curPath == WSTR_LONG_PATH_PREFIX
            || o3tl::equalsIgnoreAsciiCase(curPath, WSTR_LONG_PATH_PREFIX_UNC))
            break; // Do not check if the special path prefix exists itself

        DWORD nNewLen = GetLongPathNameW(pPath, aBuf, aBuf.getBufSizeInSymbols());
        if (nNewLen == 0)
        {
            // Error?
            const DWORD err = GetLastError();
            if (err == ERROR_FILE_NOT_FOUND || err == ERROR_PATH_NOT_FOUND)
            {
                // Check the base path; skip possible trailing separator
                size_t sepPos = curPath.substr(0, curPath.size() - 1).rfind(u'\\');
                if (sepPos != std::u16string_view::npos)
                {
                    pEnd = pPath + sepPos;
                    *pEnd = 0;
                    continue;
                }
            }
            else
            {
                SAL_WARN("sal.osl", "GetLongPathNameW: Windows error code "
                                        << err << " processing path " << OUString(curPath));
            }
            break; // All other errors, or no separators left
        }
        assert(nNewLen < aBuf.getBufSizeInSymbols());
        // Combine the case-correct leading part with the non-existing trailing part
        return OUString::Concat(std::u16string_view(o3tl::toU(aBuf), nNewLen))
               + sysPath.substr(pEnd - pStart + sysPathOffset);
    };
    return OUString(sysPath); // We found no existing parts - just assume it's OK
}

oslFileError osl_getSystemPathFromFileURL_(const OUString& strURL, rtl_uString **pustrPath, bool bAllowRelative)
{
    OUString sTempPath;
    oslFileError        nError = osl_File_E_INVAL;  /* Assume failure */

    /*  If someone hasn't encoded the complete URL we convert it to UTF8 now to prevent from
        having a mixed encoded URL later */

    OString sUTF8 = OUStringToOString(strURL, RTL_TEXTENCODING_UTF8);

    /* If the length of strUTF8 and strURL differs it indicates that the URL was not correct encoded */

    SAL_WARN_IF(
        sUTF8.getLength() != strURL.getLength() &&
        strURL.matchIgnoreAsciiCase("file:\\")
        , "sal.osl"
        ,"osl_getSystemPathFromFileURL: \"" << strURL << "\" is not encoded !!!");

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
                    sDecodedURL = GetCaseCorrectPathName(sDecodedURL->subView(nSkip));
                    if (sDecodedURL->getLength() <= MAX_PATH - 12
                        || sDecodedURL->startsWith(WSTR_SYSTEM_ROOT_PATH)
                        || sDecodedURL->startsWith(WSTR_LONG_PATH_PREFIX))
                    {
                        sTempPath = *sDecodedURL;
                    }
                    else if (sDecodedURL->startsWith("\\\\"))
                    {
                        /* it should be an UNC path, use the according prefix */
                        sTempPath = OUString::Concat(WSTR_LONG_PATH_PREFIX_UNC) + sDecodedURL->subView(2);
                    }
                    else
                    {
                        sTempPath = WSTR_LONG_PATH_PREFIX + *sDecodedURL;
                    }
                }
            }

            if (IsValidFilePath(sTempPath, VALIDATEPATH_ALLOW_ELLIPSE, &sTempPath))
                nError = osl_File_E_None;
        }
        else if ( bAllowRelative )  /* This maybe a relative file URL */
        {
            /* In future the relative path could be converted to absolute if it is too long */
            sTempPath = *sDecodedURL;

            if (IsValidFilePath(sTempPath, VALIDATEPATH_ALLOW_RELATIVE | VALIDATEPATH_ALLOW_ELLIPSE, &sTempPath))
                nError = osl_File_E_None;
        }
        else
          SAL_INFO_IF(nError, "sal.osl",
              "osl_getSystemPathFromFileURL: \"" << strURL << "\" is not an absolute FileURL");

    }

    if ( osl_File_E_None == nError )
        rtl_uString_assign(pustrPath, sTempPath.pData);

    SAL_INFO_IF(nError, "sal.osl",
        "osl_getSystemPathFromFileURL: \"" << strURL << "\" is not a FileURL");

    return nError;
}

oslFileError osl_getFileURLFromSystemPath( rtl_uString* strPath, rtl_uString** pstrURL )
{
    oslFileError nError = osl_File_E_INVAL; /* Assume failure */
    OUString sTempURL;
    DWORD dwPathType = PATHTYPE_ERROR;

    if (strPath)
        dwPathType = IsValidFilePath(OUString::unacquired(&strPath), VALIDATEPATH_ALLOW_RELATIVE, nullptr);

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
                    static_assert(WSTR_LONG_PATH_PREFIX_UNC.size() == 8,
                                  "Unexpected long path UNC prefix!");

                    /* generate the normal UNC path */
                    sTempPath = "\\\\" + sPath.copy(8).replace('\\', '/');
                    break;

                case PATHTYPE_ABSOLUTE_LOCAL:
                    static_assert(WSTR_LONG_PATH_PREFIX.size() == 4,
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
        "osl_getFileURLFromSystemPath: \"" << OUString::unacquired(&strPath) << "\" is not a systemPath");
    return nError;
}

oslFileError SAL_CALL osl_getSystemPathFromFileURL(
    rtl_uString *ustrURL, rtl_uString **pustrPath)
{
    return osl_getSystemPathFromFileURL_(OUString::unacquired(&ustrURL), pustrPath, true);
}

oslFileError SAL_CALL osl_searchFileURL(
    rtl_uString *ustrFileName,
    rtl_uString *ustrSystemSearchPath,
    rtl_uString **pustrPath)
{
    OUString     ustrUNCPath;
    OUString     ustrSysPath;
    oslFileError    error;

    /* First try to interpret the file name as a URL even a relative one */
    error = osl_getSystemPathFromFileURL_(OUString::unacquired(&ustrFileName), &ustrUNCPath.pData, true);

    /* So far we either have an UNC path or something invalid
       Now create a system path */
    if ( osl_File_E_None == error )
        error = osl_getSystemPathFromFileURL_(ustrUNCPath, &ustrSysPath.pData, true);

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
            LPCWSTR lpszSearchFile = o3tl::toW(ustrSysPath.getStr());

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
            ustrSysPath = o3tl::toU(lpBuffer);
            error = osl_getFileURLFromSystemPath(ustrSysPath.pData, pustrPath);
        }
        else
        {
            WIN32_FIND_DATAW aFindFileData;
            HANDLE  hFind;

            /* something went wrong, perhaps the path was absolute */
            error = oslTranslateFileError( GetLastError() );

            hFind = FindFirstFileW(o3tl::toW(ustrSysPath.getStr()), &aFindFileData);

            if ( IsValidHandle(hFind) )
            {
                error = osl_getFileURLFromSystemPath(ustrSysPath.pData, pustrPath);
                FindClose( hFind );
            }
        }

        free( lpBuffer );
    }

    return error;
}

oslFileError SAL_CALL osl_getAbsoluteFileURL( rtl_uString* ustrBaseURL, rtl_uString* ustrRelativeURL, rtl_uString** pustrAbsoluteURL )
{
    oslFileError eError = osl_File_E_None;
    OUString     ustrRelSysPath;
    OUString     ustrBaseSysPath;

    if ( ustrBaseURL && ustrBaseURL->length )
    {
        eError = osl_getSystemPathFromFileURL_(OUString::unacquired(&ustrBaseURL), &ustrBaseSysPath.pData, false);
        OSL_ENSURE( osl_File_E_None == eError, "osl_getAbsoluteFileURL called with relative or invalid base URL" );
    }
    if (eError == osl_File_E_None)
    {
        eError = osl_getSystemPathFromFileURL_(OUString::unacquired(&ustrRelativeURL), &ustrRelSysPath.pData,
                                               !ustrBaseSysPath.isEmpty());
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
        if (!ustrBaseSysPath.isEmpty() && !isAbsolute(ustrRelSysPath))
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
                    sResultPath = combinePath(ustrBaseSysPath, ustrRelSysPath.subView(2));
                }
                else
                {
                    // Call GetFullPathNameW to get current directory on ustrRelSysPath's drive
                    wchar_t baseDrive[3] = { ustrRelSysPath[0], ':', 0 }; // just "C:"
                    osl::LongPathBuffer<wchar_t> aBuf(MAX_LONG_PATH);
                    DWORD dwResult
                        = GetFullPathNameW(baseDrive, aBuf.getBufSizeInSymbols(), aBuf, nullptr);
                    if (dwResult)
                    {
                        if (dwResult >= aBuf.getBufSizeInSymbols())
                            eError = osl_File_E_INVAL;
                        else
                            sResultPath = combinePath(o3tl::toU(aBuf), ustrRelSysPath.subView(2));
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
                auto sBaseView(pathView(ustrBaseSysPath, ustrRelSysPath.startsWith("\\")));
                sResultPath = combinePath(sBaseView, ustrRelSysPath);
            }
        }
        else
            sResultPath = ustrRelSysPath;

        if (eError == osl_File_E_None)
        {
            sResultPath = removeRelativeParts(sResultPath);
            eError = osl_getFileURLFromSystemPath(sResultPath.pData, pustrAbsoluteURL);
        }
    }

    return  eError;
}

oslFileError SAL_CALL osl_getCanonicalName( rtl_uString *strRequested, rtl_uString **strValid )
{
    rtl_uString_newFromString(strValid, strRequested);
    return osl_File_E_None;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
