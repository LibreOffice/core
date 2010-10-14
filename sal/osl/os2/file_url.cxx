/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <ctype.h>
#include "system.h"

#include <limits.h>

#include <errno.h>

#include <stdlib.h>

#include <strings.h>

#include <unistd.h>
#include <osl/file.h>
#include <osl/security.h>
#include <rtl/uri.h>
#include <osl/diagnose.h>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.h>

#include <osl/thread.h>
#include <osl/file.hxx>
#include <osl/mutex.h>
#include <osl/process.h>
#include "file_error_transl.h"

#include "file_url.h"
#include "file_path_helper.hxx"

#include "uunxapi.hxx"

#include <wchar.h>
#include <wctype.h>

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

oslMutex g_CurrentDirectoryMutex;


/***************************************************
 * forward
 **************************************************/

void _osl_warnFile(const char*, rtl_uString*);
rtl_uString*  oslMakeUStrFromPsz(const sal_Char* pszStr,rtl_uString** uStr);

extern "C" int UnicodeToText(char *, size_t, const sal_Unicode *, sal_Int32);
extern "C" int TextToUnicode(const char* text, size_t text_buffer_size, sal_Unicode* unic_text, sal_Int32 unic_text_buffer_size);

/***************************************************
 * namespace directives
 **************************************************/

using namespace osl;

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
/*  osl_getFileURLFromSystemPath */
/****************************************************************************/

BOOL WINAPI IsValidFilePathComponent(
    LPCTSTR lpComponent, LPCTSTR *lppComponentEnd, DWORD dwFlags)
{
    LPCTSTR lpComponentEnd = NULL;
    LPCTSTR lpCurrent = lpComponent;
    BOOL    fValid = TRUE;  /* Assume success */
    TCHAR   cLast = 0;

    /* Path component length must not exceed MAX_PATH */

    while ( !lpComponentEnd && lpCurrent && lpCurrent - lpComponent < _MAX_PATH )
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
                    if ( 1 == lpCurrent - lpComponent )
                    {
                        /* Current directory is O.K. */
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
            case 0:
            case ' ':
                lpComponentEnd = lpCurrent - 1;
                fValid = FALSE;
                break;
            default:
                lpComponentEnd = lpCurrent;
                break;
            }
            break;
            /* '?' and '*' are valid wildcards but not valid file name characters */
        case '?':
        case '*':
            if ( dwFlags & VALIDATEPATH_ALLOW_WILDCARDS )
                break;
            /* The following characters are reserved */
        case '<':
        case '>':
        case '\"':
        case '|':
        case ':':
            lpComponentEnd = lpCurrent;
            fValid = FALSE;
            break;
        default:
            /* Characters below ASCII 32 are not allowed */
            if ( *lpCurrent < ' ' )
            {
                lpComponentEnd = lpCurrent;
                fValid = FALSE;
            }
            break;
        }
        cLast = *lpCurrent++;
    }

    /*  If we don't reached the end of the component the length of the component was to long
        ( See condition of while loop ) */
    if ( !lpComponentEnd )
    {
        fValid = FALSE;
        lpComponentEnd = lpCurrent;
    }

    /* Test wether the component specifies a device name what is not allowed */

    // MT: PERFORMANCE:
    // This is very expensive. A lot of calls to _tcsicmp.
    // in SRC6870m71 67.000 calls of this method while empty office start result into more than 1.500.00 calls of _tcsicmp!
    // Possible optimizations
    // - Array should be const static
    // - Sorted array, use binary search
    // - More intelligent check for com1-9, lpt1-9
    // Maybe make szComponent upper case, don't search case intensitive
    // Talked to HRO: Could be removed. Shouldn't be used in OOo, and if used for something like a filename, it will lead to an error anyway.
    /*
    if ( fValid )
    {
        LPCTSTR alpDeviceNames[] =
        {
            TEXT("CON"),
            TEXT("PRN"),
            TEXT("AUX"),
            TEXT("CLOCK$"),
            TEXT("NUL"),
            TEXT("LPT1"),
            TEXT("LPT2"),
            TEXT("LPT3"),
            TEXT("LPT4"),
            TEXT("LPT5"),
            TEXT("LPT6"),
            TEXT("LPT7"),
            TEXT("LPT8"),
            TEXT("LPT9"),
            TEXT("COM1"),
            TEXT("COM2"),
            TEXT("COM3"),
            TEXT("COM4"),
            TEXT("COM5"),
            TEXT("COM6"),
            TEXT("COM7"),
            TEXT("COM8"),
            TEXT("COM9")
        };

        TCHAR   szComponent[MAX_PATH];
        int     nComponentLength;
        LPCTSTR lpDot;
        int     i;

        // A device name with an extension is also invalid
        lpDot = _tcschr( lpComponent, '.' );

        if ( !lpDot || lpDot > lpComponentEnd )
            nComponentLength = lpComponentEnd - lpComponent;
        else
            nComponentLength = lpDot - lpComponent;

        _tcsncpy( szComponent, lpComponent, nComponentLength );
        szComponent[nComponentLength] = 0;

        for ( i = 0; i < sizeof( alpDeviceNames ) / sizeof(LPCTSTR); i++ )
        {
            if ( 0 == _tcsicmp( szComponent, alpDeviceNames[i] ) )
            {
                lpComponentEnd = lpComponent;
                fValid = FALSE;
                break;
            }
        }
    }
    */

    if ( fValid )
    {
        // Empty components are not allowed
        if ( lpComponentEnd - lpComponent < 1 )
            fValid = FALSE;

        // If we reached the end of the string NULL is returned
        else if ( !*lpComponentEnd )
            lpComponentEnd = NULL;

    }

    if ( lppComponentEnd )
        *lppComponentEnd = lpComponentEnd;

    return fValid;
}

//#####################################################
DWORD WINAPI IsValidFilePath(LPCTSTR lpszPath, LPCTSTR *lppError, DWORD dwFlags)
{
    LPCTSTR lpComponent;
    BOOL    fValid = TRUE;
    DWORD   dwPathType = PATHTYPE_ERROR;

    if ( dwFlags & VALIDATEPATH_ALLOW_RELATIVE )
        dwFlags |= VALIDATEPATH_ALLOW_ELLIPSE;

    if ( !lpszPath )
    {
        fValid = FALSE;
        lpComponent = lpszPath;
    }

    /* Test for UNC path notation */
    if ( 2 == _tcsspn( lpszPath, CHARSET_SEPARATOR ) )
    {
        /* Place the pointer behind the leading to backslashes */

        lpComponent = lpszPath + 2;

        fValid = IsValidFilePathComponent( lpComponent, &lpComponent, VALIDATEPATH_ALLOW_ELLIPSE );

        /* So far we have a valid servername. Now let's see if we also have a network resource */

        dwPathType = PATHTYPE_ABSOLUTE_UNC;

        if ( fValid )
        {
            if ( lpComponent &&  !*++lpComponent )
                lpComponent = NULL;

            if ( !lpComponent )
            {
#if 0
                /* We only have a Server specification what is invalid */

                lpComponent = lpszPath;
                fValid = FALSE;
#else
                dwPathType |= PATHTYPE_IS_SERVER;
#endif
            }
            else
            {
                /* Now test the network resource */

                fValid = IsValidFilePathComponent( lpComponent, &lpComponent, 0 );

                /* If we now reached the end of the path, everything is O.K. */


                if ( fValid && (!lpComponent || lpComponent && !*++lpComponent ) )
                {
                    lpComponent = NULL;
                    dwPathType |= PATHTYPE_IS_VOLUME;
                }
            }
        }
    }

    /* Local path verification. Must start with <drive>: */
    else if ( _istalpha( lpszPath[0] ) && ':' == lpszPath[1] )
    {
        /* Place pointer behind correct drive specification */

        lpComponent = lpszPath + 2;

        if ( 1 == _tcsspn( lpComponent, CHARSET_SEPARATOR ) )
            lpComponent++;
        else if ( *lpComponent )
            fValid = FALSE;

        dwPathType = PATHTYPE_ABSOLUTE_LOCAL;

        /* Now we are behind the backslash or it was a simple drive without backslash */

        if ( fValid && !*lpComponent )
        {
            lpComponent = NULL;
            dwPathType |= PATHTYPE_IS_VOLUME;
        }
    }

    /* Can be a relative path */
    else if ( dwFlags & VALIDATEPATH_ALLOW_RELATIVE )
    {
        lpComponent = lpszPath;

        /* Relative path can start with a backslash */

        if ( 1 == _tcsspn( lpComponent, CHARSET_SEPARATOR ) )
        {
            lpComponent++;
            if ( !*lpComponent )
                lpComponent = NULL;
        }

        dwPathType = PATHTYPE_RELATIVE;
    }

    /* Anything else is an error */
    else
    {
        fValid = FALSE;
        lpComponent = lpszPath;
    }

    /* Now validate each component of the path */
    while ( fValid && lpComponent )
    {
        fValid = IsValidFilePathComponent( lpComponent, &lpComponent, dwFlags );

        if ( fValid && lpComponent )
        {
            lpComponent++;

            /* If the string behind the backslash is empty, we've done */

            if ( !*lpComponent )
                lpComponent = NULL;
        }
    }

    if ( fValid && _tcslen( lpszPath ) >= _MAX_PATH )
    {
        fValid = FALSE;
        lpComponent = lpszPath + _MAX_PATH;
    }

    if ( lppError )
        *lppError = lpComponent;

    return fValid ? dwPathType : PATHTYPE_ERROR;
}

sal_Bool _osl_decodeURL( rtl_String* strUTF8, rtl_uString** pstrDecodedURL )
{
    sal_Char        *pBuffer;
    const sal_Char  *pSrcEnd;
    const sal_Char  *pSrc;
    sal_Char        *pDest;
    sal_Int32       nSrcLen;
    sal_Bool        bValidEncoded = sal_True;   /* Assume success */

    /* The resulting decoded string length is shorter or equal to the source length */

    nSrcLen = rtl_string_getLength(strUTF8);
    pBuffer = reinterpret_cast<sal_Char*>(rtl_allocateMemory(nSrcLen + 1));

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

                aChar = (sal_Char)strtoul( aToken, NULL, 16 );

                /* The chars are path delimiters and must not be encoded */

                if ( 0 == aChar || '\\' == aChar || '/' == aChar || ':' == aChar )
                    bValidEncoded = sal_False;
                else
                    *pDest++ = aChar;
            }
            break;
        default:
            *pDest++ = *pSrc++;
            break;
        }
    }

    *pDest++ = 0;

    if ( bValidEncoded ) {
        rtl_string2UString( pstrDecodedURL, pBuffer, rtl_str_getLength(pBuffer), RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS );
        OSL_ASSERT(*pstrDecodedURL != 0);
    }

    rtl_freeMemory( pBuffer );

    return bValidEncoded;
}

//#############################################
void _osl_encodeURL( rtl_uString *strURL, rtl_String **pstrEncodedURL )
{
    /* Encode non ascii characters within the URL */

    rtl_String      *strUTF8 = NULL;
    sal_Char        *pszEncodedURL;
    const sal_Char  *pURLScan;
    sal_Char        *pURLDest;
    sal_Int32       nURLScanLen;
    sal_Int32       nURLScanCount;

    rtl_uString2String( &strUTF8, rtl_uString_getStr( strURL ), rtl_uString_getLength( strURL ), RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS );

    pszEncodedURL = (sal_Char*) rtl_allocateMemory( (rtl_string_getLength( strUTF8 ) * 3 + 1)  * sizeof(sal_Char) );

    pURLDest = pszEncodedURL;
    pURLScan = rtl_string_getStr( strUTF8 );
    nURLScanLen = rtl_string_getLength( strUTF8 );
    nURLScanCount = 0;

    while ( nURLScanCount < nURLScanLen )
    {
        sal_Char    cCurrent = *pURLScan;

        switch ( cCurrent )
        {
        default:
            if (!( ( cCurrent >= 'a' && cCurrent <= 'z' ) || ( cCurrent >= 'A' && cCurrent <= 'Z' ) || ( cCurrent >= '0' && cCurrent <= '9' ) ) )
            {
                sprintf( pURLDest, "%%%02X", (unsigned char)cCurrent );
                pURLDest += 3;
                break;
            }
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

//#############################################
oslFileError SAL_CALL _osl_getFileURLFromSystemPath( rtl_uString* strPath, rtl_uString** pstrURL )
{
    oslFileError nError = osl_File_E_INVAL; /* Assume failure */
    rtl_uString *strTempURL = NULL;
    DWORD dwPathType = PATHTYPE_ERROR;

    if (strPath)
        dwPathType = IsValidFilePath(strPath->buffer, NULL, VALIDATEPATH_ALLOW_RELATIVE);

    if (dwPathType)
    {
        rtl_uString *strTempPath = NULL;

        /* Replace backslashes */

        rtl_uString_newReplace( &strTempPath, strPath, '\\', '/' );

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
        rtl_String  *strEncodedURL = NULL;

        /* Encode the URL */

        _osl_encodeURL( strTempURL, &strEncodedURL );

        /* Provide URL via unicode string */

        rtl_string2UString( pstrURL, rtl_string_getStr(strEncodedURL), rtl_string_getLength(strEncodedURL), RTL_TEXTENCODING_ASCII_US, OUSTRING_TO_OSTRING_CVTFLAGS );
        OSL_ASSERT(*pstrURL != 0);
        rtl_string_release( strEncodedURL );
    }

    /* Release temp URL */

    if ( strTempURL )
        rtl_uString_release( strTempURL );

    /*
    OSL_ENSURE_FILE( !nError, "osl_getFileURLFromSystemPath: \"%s\" is not a systemPath !!!", strPath );
    */

    return nError;
}

oslFileError SAL_CALL osl_getFileURLFromSystemPath( rtl_uString *ustrSystemPath, rtl_uString **pustrFileURL )
{
    return _osl_getFileURLFromSystemPath( ustrSystemPath, pustrFileURL );
#if 0
    static const sal_Unicode pDoubleSlash[2] = { '/', '/' };

    rtl_uString *pTmp = NULL;
    sal_Int32 nIndex;

    if( 0 == ustrSystemPath->length )
        return osl_File_E_INVAL;

    /* YD convert '\' to '/' */
    rtl_ustr_replaceChar( ustrSystemPath->buffer, '\\', '/' );

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
#endif
}

//#############################################
oslFileError SAL_CALL _osl_getSystemPathFromFileURL( rtl_uString *strURL, rtl_uString **pustrPath, sal_Bool bAllowRelative )
{
    rtl_String          *strUTF8 = NULL;
    rtl_uString         *strDecodedURL = NULL;
    rtl_uString         *strTempPath = NULL;
    const sal_Unicode   *pDecodedURL;
    sal_uInt32          nDecodedLen;
    sal_Bool            bValidEncoded;
    oslFileError        nError = osl_File_E_INVAL;  /* Assume failure */

    /*  If someone hasn't encoded the complete URL we convert it to UTF8 now to prevent from
        having a mixed encoded URL later */

    rtl_uString2String( &strUTF8, rtl_uString_getStr( strURL ), rtl_uString_getLength( strURL ), RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS );

    /* If the length of strUTF8 and strURL differs it indicates that the URL was not correct encoded */

    OSL_ENSURE_FILE(
        strUTF8->length == strURL->length ||
        0 != rtl_ustr_ascii_shortenedCompare_WithLength( strURL->buffer, strURL->length, "file:\\\\", 7 )
        ,"osl_getSystemPathFromFileURL: \"%s\" is not encoded !!!", strURL );

    bValidEncoded = _osl_decodeURL( strUTF8, &strDecodedURL );

    /* Release the encoded UTF8 string */

    rtl_string_release( strUTF8 );


    if ( bValidEncoded )
    {
        /* Replace backslashes and pipes */

        rtl_uString_newReplace( &strDecodedURL, strDecodedURL, '/', '\\' );
        rtl_uString_newReplace( &strDecodedURL, strDecodedURL, '|', ':' );

        pDecodedURL = rtl_uString_getStr( strDecodedURL );
        nDecodedLen = rtl_uString_getLength( strDecodedURL );

        /* Must start with "file://" */

        if ( 0 == rtl_ustr_ascii_shortenedCompare_WithLength( pDecodedURL, nDecodedLen, "file:\\\\", 7 ) )
        {
            sal_uInt32  nSkip;

            if ( 0 == rtl_ustr_ascii_shortenedCompare_WithLength( pDecodedURL, nDecodedLen, "file:\\\\\\", 8 ) )
                nSkip = 8;
            else if (
                0 == rtl_ustr_ascii_shortenedCompare_WithLength( pDecodedURL, nDecodedLen, "file:\\\\localhost\\", 17 ) ||
                0 == rtl_ustr_ascii_shortenedCompare_WithLength( pDecodedURL, nDecodedLen, "file:\\\\127.0.0.1\\", 17 )
            )
                nSkip = 17;
            else
                nSkip = 5;

            /* Indicates local root */
            if ( nDecodedLen == nSkip )
                rtl_uString_newFromStr_WithLength( &strTempPath, (const sal_Unicode*)WSTR_SYSTEM_ROOT_PATH, ELEMENTS_OF_ARRAY(WSTR_SYSTEM_ROOT_PATH) - 1 );
            else
                rtl_uString_newFromStr_WithLength( &strTempPath, pDecodedURL + nSkip, nDecodedLen - nSkip );

            if ( IsValidFilePath( strTempPath->buffer, NULL, VALIDATEPATH_ALLOW_ELLIPSE ) )
                nError = osl_File_E_None;
        }
        else if ( bAllowRelative )  /* This maybe a relative file URL */
        {
            rtl_uString_assign( &strTempPath, strDecodedURL );

            if ( IsValidFilePath( strTempPath->buffer, NULL, VALIDATEPATH_ALLOW_RELATIVE | VALIDATEPATH_ALLOW_ELLIPSE ) )
                nError = osl_File_E_None;
        }
    /*
        else
            OSL_ENSURE_FILE( !nError, "osl_getSystemPathFromFileURL: \"%s\" is not an absolute FileURL !!!", strURL );
     */

    }

    if ( strDecodedURL )
        rtl_uString_release( strDecodedURL );

    if ( osl_File_E_None == nError )
        rtl_uString_assign( pustrPath, strTempPath );

    if ( strTempPath )
        rtl_uString_release( strTempPath );

    /*
    OSL_ENSURE_FILE( !nError, "osl_getSystemPathFromFileURL: \"%s\" is not a FileURL !!!", strURL );
    */

    return nError;
}

/****************************************************************************/
/*  osl_getSystemPathFromFileURL */
/****************************************************************************/

oslFileError SAL_CALL osl_getSystemPathFromFileURL( rtl_uString *ustrFileURL, rtl_uString **pustrSystemPath )
{
    return _osl_getSystemPathFromFileURL( ustrFileURL, pustrSystemPath, sal_True );
#if 0
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
#endif // 0
}


/****************************************************************************
 * osl_getSystemPathFromFileURL_Ex - helper function
 * clients may specify if they want to accept relative
 * URLs or not
 ****************************************************************************/

oslFileError osl_getSystemPathFromFileURL_Ex(
    rtl_uString *ustrFileURL, rtl_uString **pustrSystemPath, sal_Bool bAllowRelative)
{
    return _osl_getSystemPathFromFileURL( ustrFileURL, pustrSystemPath, bAllowRelative);
#if 0
    rtl_uString* temp = 0;
    oslFileError osl_error = osl_getSystemPathFromFileURL(ustrFileURL, &temp);

    if (osl_File_E_None == osl_error)
    {
        if (bAllowRelative
            || (UNICHAR_SLASH == temp->buffer[0])
            || (UNICHAR_COLON == temp->buffer[1] && UNICHAR_SLASH == temp->buffer[2]))
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
#endif
}

namespace /* private */
{

#if 0 // YD

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

        while ((*dc++ = *sc++))
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

#endif // 0 // YD

} // end namespace private

#if OSL_DEBUG_LEVEL > 0

    //#####################################################
    void _osl_warnFile( const char *message, rtl_uString *ustrFile )
    {
        char szBuffer[2048];

        if (ustrFile)
        {
            rtl_String  *strFile = NULL;

            rtl_uString2String( &strFile, rtl_uString_getStr( ustrFile ), rtl_uString_getLength( ustrFile ), osl_getThreadTextEncoding(), OUSTRING_TO_OSTRING_CVTFLAGS );
            snprintf( szBuffer, sizeof(szBuffer), message, strFile->buffer );
            rtl_string_release( strFile );

            message = szBuffer;
        }
        OSL_ENSURE( 0, message );
    }

#endif // OSL_DEBUG_LEVEL > 0

/******************************************************
 * osl_getAbsoluteFileURL
 ******************************************************/

//oslFileError osl_getAbsoluteFileURL(rtl_uString*  ustrBaseDirURL, rtl_uString* ustrRelativeURL, rtl_uString** pustrAbsoluteURL)
oslFileError SAL_CALL osl_getAbsoluteFileURL( rtl_uString* ustrBaseURL, rtl_uString* ustrRelativeURL, rtl_uString** pustrAbsoluteURL )
{
    oslFileError    eError;
    rtl_uString     *ustrRelSysPath = NULL;
    rtl_uString     *ustrBaseSysPath = NULL;

    if ( ustrBaseURL && ustrBaseURL->length )
    {
        eError = _osl_getSystemPathFromFileURL( ustrBaseURL, &ustrBaseSysPath, sal_False );
        OSL_ENSURE( osl_File_E_None == eError, "osl_getAbsoluteFileURL called with relative or invalid base URL" );

        eError = _osl_getSystemPathFromFileURL( ustrRelativeURL, &ustrRelSysPath, sal_True );
    }
    else
    {
        eError = _osl_getSystemPathFromFileURL( ustrRelativeURL, &ustrRelSysPath, sal_False );
        OSL_ENSURE( osl_File_E_None == eError, "osl_getAbsoluteFileURL called with empty base URL and/or invalid relative URL" );
    }

    if ( !eError )
    {
        CHAR    szBuffer[_MAX_PATH];
        CHAR    szRelSysPath[_MAX_PATH];
        CHAR    szCurrentDir[_MAX_PATH];
        int     result;
        char*   cwd;
        int     rc;

/*@@@ToDo
  Bad, bad hack, this only works if the base path
  really exists which is not necessary according
  to RFC2396
  The whole FileURL implementation should be merged
  with the rtl/uri class.
*/
        if ( ustrBaseSysPath )
        {
            CHAR    szBaseSysPath[_MAX_PATH];

            if (!g_CurrentDirectoryMutex)
                g_CurrentDirectoryMutex = osl_createMutex();

            osl_acquireMutex( g_CurrentDirectoryMutex );

            cwd = getcwd( szCurrentDir, sizeof(szCurrentDir) );
            UnicodeToText( szBaseSysPath, sizeof(szBaseSysPath), ustrBaseSysPath->buffer, ustrBaseSysPath->length);
            rc = chdir( szBaseSysPath);
        }

        UnicodeToText( szRelSysPath, sizeof(szRelSysPath), ustrRelSysPath->buffer, ustrRelSysPath->length);
        result = !_abspath( szBuffer, szRelSysPath, sizeof(szBuffer));

        if ( ustrBaseSysPath )
        {
            rc = chdir( szCurrentDir );

            osl_releaseMutex( g_CurrentDirectoryMutex );
        }

        if ( result )
        {
                rtl_uString *ustrAbsSysPath = NULL;

                oslMakeUStrFromPsz( szBuffer, &ustrAbsSysPath);

                eError = osl_getFileURLFromSystemPath( ustrAbsSysPath, pustrAbsoluteURL );

                if ( ustrAbsSysPath )
                    rtl_uString_release( ustrAbsSysPath );
        }
        else
            eError = osl_File_E_INVAL;
    }

    if ( ustrBaseSysPath )
        rtl_uString_release( ustrBaseSysPath );

    if ( ustrRelSysPath )
        rtl_uString_release( ustrRelSysPath );

    return  eError;
#if 0
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
#endif // 0
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


/****************************************************************************
 * FileURLToPath
 ***************************************************************************/

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
