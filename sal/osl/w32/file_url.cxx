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

#define UNICODE
#define _UNICODE
#include "systools/win32/uwinapi.h"

#include "file_url.hxx"
#include <sal/macros.h>
#include "file_error.hxx"

#include "rtl/alloc.h"
#include <rtl/ustring.hxx>
#include "osl/diagnose.h"
#include "osl/file.h"
#include "osl/mutex.h"

#include "path_helper.hxx"

#include <stdio.h>
#include <tchar.h>

#define WSTR_SYSTEM_ROOT_PATH               L"\\\\.\\"
#define WSTR_LONG_PATH_PREFIX               L"\\\\?\\"
#define WSTR_LONG_PATH_PREFIX_UNC           L"\\\\?\\UNC\\"

// FileURL functions

oslMutex g_CurrentDirectoryMutex = nullptr; /* Initialized in dllentry.c */

static bool IsValidFilePathComponent(
    sal_Unicode const * lpComponent, sal_Unicode const **lppComponentEnd,
    DWORD dwFlags)
{
        sal_Unicode const * lpComponentEnd = nullptr;
        sal_Unicode const * lpCurrent = lpComponent;
        bool    fValid = true;  /* Assume success */
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
                    SAL_FALLTHROUGH;
                case 0:
                case ' ':
                    if ( dwFlags & VALIDATEPATH_ALLOW_INVALID_SPACE_AND_PERIOD )
                        lpComponentEnd = lpCurrent;
                    else
                    {
                        lpComponentEnd = lpCurrent - 1;
                        fValid = false;
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
                fValid = false;
                break;
            default:
                /* Characters below ASCII 32 are not allowed */
                if ( *lpCurrent < ' ' )
                {
                    lpComponentEnd = lpCurrent;
                    fValid = false;
                }
                break;
            }
            cLast = *lpCurrent++;
        }

        /*  If we don't reached the end of the component the length of the component was to long
            ( See condition of while loop ) */
        if ( !lpComponentEnd )
        {
            fValid = false;
            lpComponentEnd = lpCurrent;
        }

        if ( fValid )
        {
            // Empty components are not allowed
            if ( lpComponentEnd - lpComponent < 1 )
                fValid = false;

            // If we reached the end of the string nullptr is returned
            else if ( !*lpComponentEnd )
                lpComponentEnd = nullptr;

        }

        if ( lppComponentEnd )
            *lppComponentEnd = lpComponentEnd;

        return fValid;
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
        bool    fValid = true;
        DWORD   dwPathType = PATHTYPE_ERROR;
        sal_Int32 nLength = rtl_uString_getLength( path );

        if ( dwFlags & VALIDATEPATH_ALLOW_RELATIVE )
            dwFlags |= VALIDATEPATH_ALLOW_ELLIPSE;

        if ( !lpszPath )
            fValid = false;

        DWORD   dwCandidatPathType = PATHTYPE_ERROR;

        if ( 0 == rtl_ustr_shortenedCompareIgnoreAsciiCase_WithLength( path->buffer, nLength, SAL_U(WSTR_LONG_PATH_PREFIX_UNC), SAL_N_ELEMENTS(WSTR_LONG_PATH_PREFIX_UNC) - 1, SAL_N_ELEMENTS(WSTR_LONG_PATH_PREFIX_UNC) - 1 ) )
        {
            /* This is long path in UNC notation */
            lpComponent = lpszPath + SAL_N_ELEMENTS(WSTR_LONG_PATH_PREFIX_UNC) - 1;
            dwCandidatPathType = PATHTYPE_ABSOLUTE_UNC | PATHTYPE_IS_LONGPATH;
        }
        else if ( 0 == rtl_ustr_shortenedCompareIgnoreAsciiCase_WithLength( path->buffer, nLength, SAL_U(WSTR_LONG_PATH_PREFIX), SAL_N_ELEMENTS(WSTR_LONG_PATH_PREFIX) - 1, SAL_N_ELEMENTS(WSTR_LONG_PATH_PREFIX) - 1 ) )
        {
            /* This is long path */
            lpComponent = lpszPath + SAL_N_ELEMENTS(WSTR_LONG_PATH_PREFIX) - 1;

            if ( _istalpha( lpComponent[0] ) && ':' == lpComponent[1] )
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
        else if ( _istalpha( lpszPath[0] ) && ':' == lpszPath[1] )
        {
            /* Local path verification. Must start with <drive>: */
            lpComponent = lpszPath + 2;
            dwCandidatPathType = PATHTYPE_ABSOLUTE_LOCAL;
        }

        if ( ( dwCandidatPathType & PATHTYPE_MASK_TYPE ) == PATHTYPE_ABSOLUTE_UNC )
        {
            fValid = IsValidFilePathComponent( lpComponent, &lpComponent, VALIDATEPATH_ALLOW_ELLIPSE );

            /* So far we have a valid servername. Now let's see if we also have a network resource */

            dwPathType = dwCandidatPathType;

            if ( fValid )
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

                    fValid = IsValidFilePathComponent( lpComponent, &lpComponent, 0 );

                    /* If we now reached the end of the path, everything is O.K. */

                    if ( fValid && (!lpComponent || !*++lpComponent ) )
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
                fValid = false;

            dwPathType = dwCandidatPathType;

            /* Now we are behind the backslash or it was a simple drive without backslash */

            if ( fValid && !*lpComponent )
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
            fValid = false;
            lpComponent = lpszPath;
        }

        /* Now validate each component of the path */
        while ( fValid && lpComponent )
        {
            // Correct path by merging consecutive slashes:
            if (*lpComponent == '\\' && corrected != nullptr) {
                sal_Int32 i = lpComponent - lpszPath;
                rtl_uString_newReplaceStrAt(corrected, path, i, 1, nullptr);
                    //TODO: handle out-of-memory
                lpszPath = (*corrected)->buffer;
                lpComponent = lpszPath + i;
            }

            fValid = IsValidFilePathComponent( lpComponent, &lpComponent, dwFlags | VALIDATEPATH_ALLOW_INVALID_SPACE_AND_PERIOD);

            if ( fValid && lpComponent )
            {
                lpComponent++;

                /* If the string behind the backslash is empty, we've done */

                if ( !*lpComponent )
                    lpComponent = nullptr;
            }
        }

        /* The path can be longer than MAX_PATH only in case it has the longpath prefix */
        if ( fValid && !( dwPathType &  PATHTYPE_IS_LONGPATH ) && rtl_ustr_getLength( lpszPath ) >= MAX_PATH )
        {
            fValid = false;
        }

        return fValid ? dwPathType : PATHTYPE_ERROR;
}

static sal_Int32 PathRemoveFileSpec(LPWSTR lpPath, LPWSTR lpFileName, sal_Int32 nFileBufLen )
{
    sal_Int32 nRemoved = 0;

    if ( nFileBufLen )
    {
        lpFileName[0] = 0;
        LPWSTR  lpLastBkSlash = _tcsrchr( lpPath, '\\' );
        LPWSTR  lpLastSlash = _tcsrchr( lpPath, '/' );
        LPWSTR  lpLastDelimiter = lpLastSlash > lpLastBkSlash ? lpLastSlash : lpLastBkSlash;

        if ( lpLastDelimiter )
        {
                sal_Int32 nDelLen = _tcslen( lpLastDelimiter );
                if ( 1 == nDelLen )
                {
                    if ( lpLastDelimiter > lpPath && *(lpLastDelimiter - 1) != ':' )
                    {
                        *lpLastDelimiter = 0;
                        *lpFileName = 0;
                        nRemoved = nDelLen;
                    }
                }
                else if ( nDelLen && nDelLen - 1 < nFileBufLen )
                {
                    _tcscpy( lpFileName, lpLastDelimiter + 1 );
                    *(++lpLastDelimiter) = 0;
                    nRemoved = nDelLen - 1;
                }
        }
    }

    return nRemoved;
}

// Undocumented in SHELL32.DLL ordinal 32
static LPWSTR PathAddBackslash(LPWSTR lpPath, sal_uInt32 nBufLen)
{
    LPWSTR  lpEndPath = nullptr;

    if ( lpPath )
    {
            std::size_t nLen = _tcslen(lpPath);

            if ( !nLen || ( lpPath[nLen-1] != '\\' && lpPath[nLen-1] != '/' && nLen < nBufLen - 1 ) )
            {
                lpEndPath = lpPath + nLen;
                *lpEndPath++ = '\\';
                *lpEndPath = 0;
            }
    }
    return lpEndPath;
}

// Same as GetLongPathName but also 95/NT4
static DWORD GetCaseCorrectPathNameEx(
    LPWSTR  lpszPath,   // path buffer to convert
    sal_uInt32 cchBuffer,      // size of path buffer
    DWORD   nSkipLevels,
    bool bCheckExistence )
{
        ::osl::LongPathBuffer< WCHAR > szFile( MAX_PATH + 1 );
        sal_Int32 nRemoved = PathRemoveFileSpec( lpszPath, szFile, MAX_PATH + 1 );
        sal_Int32 nLastStepRemoved = nRemoved;
        while ( nLastStepRemoved && szFile[0] == 0 )
        {
            // remove separators
            nLastStepRemoved = PathRemoveFileSpec( lpszPath, szFile, MAX_PATH + 1 );
            nRemoved += nLastStepRemoved;
        }

        if ( nRemoved )
        {
            bool bSkipThis = false;

            if ( 0 == _tcscmp( szFile, TEXT("..") ) )
            {
                bSkipThis = true;
                nSkipLevels += 1;
            }
            else if ( 0 == _tcscmp( szFile, TEXT(".") ) )
            {
                bSkipThis = true;
            }
            else if ( nSkipLevels )
            {
                bSkipThis = true;
                nSkipLevels--;
            }
            else
                bSkipThis = false;

            if ( !GetCaseCorrectPathNameEx( lpszPath, cchBuffer, nSkipLevels, bCheckExistence ) )
                return 0;

            PathAddBackslash( lpszPath, cchBuffer );

            /* Analyze parent if not only a trailing backslash was cutted but a real file spec */
            if ( !bSkipThis )
            {
                if ( bCheckExistence )
                {
                    ::osl::LongPathBuffer< WCHAR > aShortPath( MAX_LONG_PATH );
                    _tcscpy( aShortPath, lpszPath );
                    _tcscat( aShortPath, szFile );

                    WIN32_FIND_DATA aFindFileData;
                    HANDLE  hFind = FindFirstFile( aShortPath, &aFindFileData );

                    if ( IsValidHandle(hFind) )
                    {
                        _tcscat( lpszPath, aFindFileData.cFileName[0] ? aFindFileData.cFileName : aFindFileData.cAlternateFileName );

                        FindClose( hFind );
                    }
                    else
                        lpszPath[0] = 0;
                }
                else
                {
                    /* add the segment name back */
                    _tcscat( lpszPath, szFile );
                }
            }
        }
        else
        {
            /* File specification can't be removed therefore the short path is either a drive
               or a network share. If still levels to skip are left, the path specification
               tries to travel below the file system root */
            if ( nSkipLevels )
                    lpszPath[0] = 0;
            else
                _tcsupr( lpszPath );
        }

        return _tcslen( lpszPath );
}

DWORD GetCaseCorrectPathName(
    LPCWSTR lpszShortPath,  // file name
    LPWSTR  lpszLongPath,   // path buffer
    sal_uInt32 cchBuffer,      // size of path buffer
    bool bCheckExistence
)
{
    /* Special handling for "\\.\" as system root */
    if ( lpszShortPath && 0 == wcscmp( lpszShortPath, WSTR_SYSTEM_ROOT_PATH ) )
    {
        if ( cchBuffer >= SAL_N_ELEMENTS(WSTR_SYSTEM_ROOT_PATH) )
        {
            wcscpy( lpszLongPath, WSTR_SYSTEM_ROOT_PATH );
            return SAL_N_ELEMENTS(WSTR_SYSTEM_ROOT_PATH) - 1;
        }
        else
        {
            return SAL_N_ELEMENTS(WSTR_SYSTEM_ROOT_PATH) - 1;
        }
    }
    else if ( lpszShortPath )
    {
        if ( _tcslen( lpszShortPath ) <= cchBuffer )
        {
            _tcscpy( lpszLongPath, lpszShortPath );
            return GetCaseCorrectPathNameEx( lpszLongPath, cchBuffer, 0, bCheckExistence );
        }
    }

    return 0;
}

static bool osl_decodeURL_( rtl_String* strUTF8, rtl_uString** pstrDecodedURL )
{
    sal_Char        *pBuffer;
    const sal_Char  *pSrcEnd;
    const sal_Char  *pSrc;
    sal_Char        *pDest;
    sal_Int32       nSrcLen;
    bool        bValidEncoded = true;   /* Assume success */

    /* The resulting decoded string length is shorter or equal to the source length */

    nSrcLen = rtl_string_getLength(strUTF8);
    pBuffer = static_cast<sal_Char*>(rtl_allocateMemory(nSrcLen + 1));

    pDest = pBuffer;
    pSrc = rtl_string_getStr(strUTF8);
    pSrcEnd = pSrc + nSrcLen;

    /* Now decode the URL what should result in an UTF8 string */
    while ( bValidEncoded && pSrc < pSrcEnd )
    {
        switch ( *pSrc )
        {
        case '%':
            {
                sal_Char    aToken[3];
                sal_Char    aChar;

                pSrc++;
                aToken[0] = *pSrc++;
                aToken[1] = *pSrc++;
                aToken[2] = 0;

                aChar = (sal_Char)strtoul( aToken, nullptr, 16 );

                /* The chars are path delimiters and must not be encoded */

                if ( 0 == aChar || '\\' == aChar || '/' == aChar || ':' == aChar )
                    bValidEncoded = false;
                else
                    *pDest++ = aChar;
            }
            break;
        case '\0':
        case '#':
        case '?':
            bValidEncoded = false;
            break;
        default:
            *pDest++ = *pSrc++;
            break;
        }
    }

    *pDest++ = 0;

    if ( bValidEncoded )
    {
        rtl_string2UString( pstrDecodedURL, pBuffer, rtl_str_getLength(pBuffer), RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS );
        assert(*pstrDecodedURL);
    }

    rtl_freeMemory( pBuffer );

    return bValidEncoded;
}

static void osl_encodeURL_( rtl_uString *strURL, rtl_String **pstrEncodedURL )
{
    /* Encode non ascii characters within the URL */

    rtl_String      *strUTF8 = nullptr;
    sal_Char        *pszEncodedURL;
    const sal_Char  *pURLScan;
    sal_Char        *pURLDest;
    sal_Int32       nURLScanLen;
    sal_Int32       nURLScanCount;

    rtl_uString2String( &strUTF8, rtl_uString_getStr( strURL ), rtl_uString_getLength( strURL ), RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS );

    pszEncodedURL = static_cast<sal_Char*>(rtl_allocateMemory( (rtl_string_getLength( strUTF8 ) * 3 + 1)  * sizeof(sal_Char) ));

    pURLDest = pszEncodedURL;
    pURLScan = rtl_string_getStr( strUTF8 );
    nURLScanLen = rtl_string_getLength( strUTF8 );
    nURLScanCount = 0;

    while ( nURLScanCount < nURLScanLen )
    {
        sal_Char cCurrent = *pURLScan;
        switch ( cCurrent )
        {
        default:
            if (!( ( cCurrent >= 'a' && cCurrent <= 'z' ) || ( cCurrent >= 'A' && cCurrent <= 'Z' ) || ( cCurrent >= '0' && cCurrent <= '9' ) ) )
            {
                sprintf( pURLDest, "%%%02X", (unsigned char)cCurrent );
                pURLDest += 3;
                break;
            }
            SAL_FALLTHROUGH;
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
            *pURLDest++ = cCurrent;
            break;
        case 0:
            break;
        }

        pURLScan++;
        nURLScanCount++;
    }

    *pURLDest = 0;

    rtl_string_release( strUTF8 );
    rtl_string_newFromStr( pstrEncodedURL, pszEncodedURL );
    rtl_freeMemory( pszEncodedURL );
}

oslFileError osl_getSystemPathFromFileURL_( rtl_uString *strURL, rtl_uString **pustrPath, bool bAllowRelative )
{
    rtl_String          *strUTF8 = nullptr;
    rtl_uString         *strDecodedURL = nullptr;
    rtl_uString         *strTempPath = nullptr;
    sal_uInt32          nDecodedLen;
    bool            bValidEncoded;
    oslFileError        nError = osl_File_E_INVAL;  /* Assume failure */

    /*  If someone hasn't encoded the complete URL we convert it to UTF8 now to prevent from
        having a mixed encoded URL later */

    rtl_uString2String( &strUTF8, rtl_uString_getStr( strURL ), rtl_uString_getLength( strURL ), RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS );

    /* If the length of strUTF8 and strURL differs it indicates that the URL was not correct encoded */

    SAL_WARN_IF(
        strUTF8->length != strURL->length &&
        0 == rtl_ustr_ascii_shortenedCompareIgnoreAsciiCase_WithLength( strURL->buffer, strURL->length, "file:\\\\", 7 )
        , "sal.osl"
        ,"osl_getSystemPathFromFileURL: \"" << rtl::OUString(strURL) << "\" is not encoded !!!");

    bValidEncoded = osl_decodeURL_( strUTF8, &strDecodedURL );

    /* Release the encoded UTF8 string */
    rtl_string_release( strUTF8 );

    if ( bValidEncoded )
    {
        /* Replace backslashes and pipes */

        rtl_uString_newReplace( &strDecodedURL, strDecodedURL, '/', '\\' );
        rtl_uString_newReplace( &strDecodedURL, strDecodedURL, '|', ':' );

        const sal_Unicode *pDecodedURL = rtl_uString_getStr( strDecodedURL );
        nDecodedLen = rtl_uString_getLength( strDecodedURL );

        /* Must start with "file://" */
        if ( 0 == rtl_ustr_ascii_shortenedCompareIgnoreAsciiCase_WithLength( pDecodedURL, nDecodedLen, "file:\\\\", 7 ) )
        {
            sal_uInt32  nSkip;

            if ( 0 == rtl_ustr_ascii_shortenedCompareIgnoreAsciiCase_WithLength( pDecodedURL, nDecodedLen, "file:\\\\\\", 8 ) )
                nSkip = 8;
            else if (
                0 == rtl_ustr_ascii_shortenedCompareIgnoreAsciiCase_WithLength( pDecodedURL, nDecodedLen, "file:\\\\localhost\\", 17 ) ||
                0 == rtl_ustr_ascii_shortenedCompareIgnoreAsciiCase_WithLength( pDecodedURL, nDecodedLen, "file:\\\\127.0.0.1\\", 17 )
                      )
                nSkip = 17;
            else
                nSkip = 5;

            /* Indicates local root */
            if ( nDecodedLen == nSkip )
                rtl_uString_newFromStr_WithLength( &strTempPath, SAL_U(WSTR_SYSTEM_ROOT_PATH), SAL_N_ELEMENTS(WSTR_SYSTEM_ROOT_PATH) - 1 );
            else
            {
                /* do not separate the directory and file case, so the maximal path length without prefix is MAX_PATH-12 */
                if ( nDecodedLen - nSkip <= MAX_PATH - 12 )
                {
                    rtl_uString_newFromStr_WithLength( &strTempPath, pDecodedURL + nSkip, nDecodedLen - nSkip );
                }
                else
                {
                    ::osl::LongPathBuffer< sal_Unicode > aBuf( MAX_LONG_PATH );
                    sal_uInt32 nNewLen = GetCaseCorrectPathName( SAL_W(pDecodedURL) + nSkip,
                                                                 ::osl::mingw_reinterpret_cast<LPWSTR>(aBuf),
                                                                 aBuf.getBufSizeInSymbols(),
                                                                 false );

                    if ( nNewLen <= MAX_PATH - 12
                      || 0 == rtl_ustr_shortenedCompareIgnoreAsciiCase_WithLength( pDecodedURL + nSkip, nDecodedLen - nSkip, SAL_U(WSTR_SYSTEM_ROOT_PATH), SAL_N_ELEMENTS(WSTR_SYSTEM_ROOT_PATH) - 1, SAL_N_ELEMENTS(WSTR_SYSTEM_ROOT_PATH) - 1 )
                      || 0 == rtl_ustr_shortenedCompareIgnoreAsciiCase_WithLength( pDecodedURL + nSkip, nDecodedLen - nSkip, SAL_U(WSTR_LONG_PATH_PREFIX), SAL_N_ELEMENTS(WSTR_LONG_PATH_PREFIX) - 1, SAL_N_ELEMENTS(WSTR_LONG_PATH_PREFIX) - 1 ) )
                    {
                        rtl_uString_newFromStr_WithLength( &strTempPath, aBuf, nNewLen );
                    }
                    else if ( pDecodedURL[nSkip] == '\\' && pDecodedURL[nSkip+1] == '\\' )
                    {
                        /* it should be an UNC path, use the according prefix */
                        rtl_uString *strSuffix = nullptr;
                        rtl_uString *strPrefix = nullptr;
                        rtl_uString_newFromStr_WithLength( &strPrefix, SAL_U(WSTR_LONG_PATH_PREFIX_UNC), SAL_N_ELEMENTS( WSTR_LONG_PATH_PREFIX_UNC ) - 1 );
                        rtl_uString_newFromStr_WithLength( &strSuffix, aBuf + 2, nNewLen - 2 );

                        rtl_uString_newConcat( &strTempPath, strPrefix, strSuffix );

                        rtl_uString_release( strPrefix );
                        rtl_uString_release( strSuffix );
                    }
                    else
                    {
                        rtl_uString *strSuffix = nullptr;
                        rtl_uString *strPrefix = nullptr;
                        rtl_uString_newFromStr_WithLength( &strPrefix, SAL_U(WSTR_LONG_PATH_PREFIX), SAL_N_ELEMENTS( WSTR_LONG_PATH_PREFIX ) - 1 );
                        rtl_uString_newFromStr_WithLength( &strSuffix, aBuf, nNewLen );

                        rtl_uString_newConcat( &strTempPath, strPrefix, strSuffix );

                        rtl_uString_release( strPrefix );
                        rtl_uString_release( strSuffix );
                    }
                }
            }

            if ( IsValidFilePath( strTempPath, VALIDATEPATH_ALLOW_ELLIPSE, &strTempPath ) )
                nError = osl_File_E_None;
        }
        else if ( bAllowRelative )  /* This maybe a relative file URL */
        {
            /* In future the relative path could be converted to absolute if it is too long */
            rtl_uString_assign( &strTempPath, strDecodedURL );

            if ( IsValidFilePath( strTempPath, VALIDATEPATH_ALLOW_RELATIVE | VALIDATEPATH_ALLOW_ELLIPSE, &strTempPath ) )
                nError = osl_File_E_None;
        }
        else
          SAL_INFO_IF(nError, "sal.osl",
              "osl_getSystemPathFromFileURL: \"" << rtl::OUString(strURL) << "\" is not an absolute FileURL");

    }

    if ( strDecodedURL )
        rtl_uString_release( strDecodedURL );

    if ( osl_File_E_None == nError )
        rtl_uString_assign( pustrPath, strTempPath );

    if ( strTempPath )
        rtl_uString_release( strTempPath );

    SAL_INFO_IF(nError, "sal.osl",
        "osl_getSystemPathFromFileURL: \"" << rtl::OUString(strURL) << "\" is not a FileURL");

    return nError;
}

oslFileError osl_getFileURLFromSystemPath( rtl_uString* strPath, rtl_uString** pstrURL )
{
    oslFileError nError = osl_File_E_INVAL; /* Assume failure */
    rtl_uString *strTempURL = nullptr;
    DWORD dwPathType = PATHTYPE_ERROR;

    if (strPath)
        dwPathType = IsValidFilePath(strPath, VALIDATEPATH_ALLOW_RELATIVE, nullptr);

    if (dwPathType)
    {
        rtl_uString *strTempPath = nullptr;

        if ( dwPathType & PATHTYPE_IS_LONGPATH )
        {
            rtl_uString *strBuffer = nullptr;
            sal_uInt32 nIgnore = 0;
            sal_uInt32 nLength = 0;

            /* the path has the longpath prefix, lets remove it */
            switch ( dwPathType & PATHTYPE_MASK_TYPE )
            {
                case PATHTYPE_ABSOLUTE_UNC:
                    nIgnore = SAL_N_ELEMENTS( WSTR_LONG_PATH_PREFIX_UNC ) - 1;
                    OSL_ENSURE( nIgnore == 8, "Unexpected long path UNC prefix!" );

                    /* generate the normal UNC path */
                    nLength = rtl_uString_getLength( strPath );
                    rtl_uString_newFromStr_WithLength( &strBuffer, strPath->buffer + nIgnore - 2, nLength - nIgnore + 2 );
                    strBuffer->buffer[0] = '\\';

                    rtl_uString_newReplace( &strTempPath, strBuffer, '\\', '/' );
                    rtl_uString_release( strBuffer );
                    break;

                case PATHTYPE_ABSOLUTE_LOCAL:
                    nIgnore = SAL_N_ELEMENTS( WSTR_LONG_PATH_PREFIX ) - 1;
                    OSL_ENSURE( nIgnore == 4, "Unexpected long path prefix!" );

                    /* generate the normal path */
                    nLength = rtl_uString_getLength( strPath );
                    rtl_uString_newFromStr_WithLength( &strBuffer, strPath->buffer + nIgnore, nLength - nIgnore );

                    rtl_uString_newReplace( &strTempPath, strBuffer, '\\', '/' );
                    rtl_uString_release( strBuffer );
                    break;

                default:
                    OSL_FAIL( "Unexpected long path format!" );
                    rtl_uString_newReplace( &strTempPath, strPath, '\\', '/' );
                    break;
            }
        }
        else
        {
            /* Replace backslashes */
            rtl_uString_newReplace( &strTempPath, strPath, '\\', '/' );
        }

        switch ( dwPathType & PATHTYPE_MASK_TYPE )
        {
        case PATHTYPE_RELATIVE:
            rtl_uString_assign( &strTempURL, strTempPath );
            nError = osl_File_E_None;
            break;
        case PATHTYPE_ABSOLUTE_UNC:
            rtl_uString_newFromAscii( &strTempURL, "file:" );
            rtl_uString_newConcat( &strTempURL, strTempURL, strTempPath );
            nError = osl_File_E_None;
            break;
        case PATHTYPE_ABSOLUTE_LOCAL:
            rtl_uString_newFromAscii( &strTempURL, "file:///" );
            rtl_uString_newConcat( &strTempURL, strTempURL, strTempPath );
            nError = osl_File_E_None;
            break;
        default:
            break;
        }

        /* Release temp path */
        rtl_uString_release( strTempPath );
    }

    if ( osl_File_E_None == nError )
    {
        rtl_String  *strEncodedURL = nullptr;

        /* Encode the URL */
        osl_encodeURL_( strTempURL, &strEncodedURL );

        /* Provide URL via unicode string */
        rtl_string2UString( pstrURL, rtl_string_getStr(strEncodedURL), rtl_string_getLength(strEncodedURL), RTL_TEXTENCODING_ASCII_US, OUSTRING_TO_OSTRING_CVTFLAGS );
        assert(*pstrURL);
        rtl_string_release( strEncodedURL );
    }

    /* Release temp URL */
    if ( strTempURL )
        rtl_uString_release( strTempURL );

    SAL_INFO_IF(nError, "sal.osl",
        "osl_getFileURLFromSystemPath: \"" << rtl::OUString(strPath) << "\" is not a systemPath");
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

    /* First try to interpret the file name as an URL even a relative one */
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
            LPCWSTR lpszSearchPath = ustrSystemSearchPath && ustrSystemSearchPath->length ? SAL_W(ustrSystemSearchPath->buffer) : nullptr;
            LPCWSTR lpszSearchFile = SAL_W(ustrSysPath->buffer);

            /* Allocate space for buffer according to previous returned count of required chars */
            /* +1 is not necessary if we follow MSDN documentation but for robustness we do so */
            nBufferLength = dwResult + 1;
            lpBuffer = lpBuffer ?
                static_cast<LPWSTR>(rtl_reallocateMemory(lpBuffer, nBufferLength * sizeof(WCHAR))) :
                static_cast<LPWSTR>(rtl_allocateMemory(nBufferLength * sizeof(WCHAR)));

            dwResult = SearchPath( lpszSearchPath, lpszSearchFile, nullptr, nBufferLength, lpBuffer, &lpszFilePart );
        } while ( dwResult && dwResult >= nBufferLength );

        /*  ... until an error occurs or buffer is large enough.
            dwResult == nBufferLength can not happen according to documentation but lets be robust ;-) */

        if ( dwResult )
        {
            rtl_uString_newFromStr( &ustrSysPath, SAL_U(lpBuffer) );
            error = osl_getFileURLFromSystemPath( ustrSysPath, pustrPath );
        }
        else
        {
            WIN32_FIND_DATA aFindFileData;
            HANDLE  hFind;

            /* something went wrong, perhaps the path was absolute */
            error = oslTranslateFileError( GetLastError() );

            hFind = FindFirstFile( SAL_W(ustrSysPath->buffer), &aFindFileData );

            if ( IsValidHandle(hFind) )
            {
                error = osl_getFileURLFromSystemPath( ustrSysPath, pustrPath );
                FindClose( hFind );
            }
        }

        rtl_freeMemory( lpBuffer );
    }

    if ( ustrSysPath )
        rtl_uString_release( ustrSysPath );

    if ( ustrUNCPath )
        rtl_uString_release( ustrUNCPath );

    return error;
}

oslFileError SAL_CALL osl_getAbsoluteFileURL( rtl_uString* ustrBaseURL, rtl_uString* ustrRelativeURL, rtl_uString** pustrAbsoluteURL )
{
    oslFileError    eError;
    rtl_uString     *ustrRelSysPath = nullptr;
    rtl_uString     *ustrBaseSysPath = nullptr;

    if ( ustrBaseURL && ustrBaseURL->length )
    {
        eError = osl_getSystemPathFromFileURL_( ustrBaseURL, &ustrBaseSysPath, false );
        OSL_ENSURE( osl_File_E_None == eError, "osl_getAbsoluteFileURL called with relative or invalid base URL" );

        eError = osl_getSystemPathFromFileURL_( ustrRelativeURL, &ustrRelSysPath, true );
    }
    else
    {
        eError = osl_getSystemPathFromFileURL_( ustrRelativeURL, &ustrRelSysPath, false );
        OSL_ENSURE( osl_File_E_None == eError, "osl_getAbsoluteFileURL called with empty base URL and/or invalid relative URL" );
    }

    if ( !eError )
    {
        ::osl::LongPathBuffer< sal_Unicode > aBuffer( MAX_LONG_PATH );
        ::osl::LongPathBuffer< sal_Unicode > aCurrentDir( MAX_LONG_PATH );
        LPWSTR  lpFilePart = nullptr;
        DWORD   dwResult;

/*@@@ToDo
  Bad, bad hack, this only works if the base path
  really exists which is not necessary according
  to RFC2396
  The whole FileURL implementation should be merged
  with the rtl/uri class.
*/
        if ( ustrBaseSysPath )
        {
            osl_acquireMutex( g_CurrentDirectoryMutex );

            GetCurrentDirectoryW( aCurrentDir.getBufSizeInSymbols(), ::osl::mingw_reinterpret_cast<LPWSTR>(aCurrentDir) );
            SetCurrentDirectoryW( SAL_W(ustrBaseSysPath->buffer) );
        }

        dwResult = GetFullPathNameW( SAL_W(ustrRelSysPath->buffer), aBuffer.getBufSizeInSymbols(), ::osl::mingw_reinterpret_cast<LPWSTR>(aBuffer), &lpFilePart );

        if ( ustrBaseSysPath )
        {
            SetCurrentDirectoryW( ::osl::mingw_reinterpret_cast<LPCWSTR>(aCurrentDir) );

            osl_releaseMutex( g_CurrentDirectoryMutex );
        }

        if ( dwResult )
        {
            if ( dwResult >= aBuffer.getBufSizeInSymbols() )
                eError = osl_File_E_INVAL;
            else
            {
                rtl_uString *ustrAbsSysPath = nullptr;

                rtl_uString_newFromStr( &ustrAbsSysPath, aBuffer );

                eError = osl_getFileURLFromSystemPath( ustrAbsSysPath, pustrAbsoluteURL );

                if ( ustrAbsSysPath )
                    rtl_uString_release( ustrAbsSysPath );
            }
        }
        else
            eError = oslTranslateFileError( GetLastError() );
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
