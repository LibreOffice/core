/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: rscchar.cxx,v $
 * $Revision: 1.7 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_rsc.hxx"
/****************** I N C L U D E S **************************************/
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#ifndef _TABLE_HXX //autogen
#include <tools/table.hxx>
#endif

// Solar Definitionen
#include <tools/solar.h>
#include <rsctools.hxx>

#include <rtl/textcvt.h>
#include <rtl/textenc.h>
#include <rtl/alloc.h>

/*************************************************************************
|*
|*    RscChar::MakeChar()
|*
|*    Beschreibung      Der String wird nach C-Konvention umgesetzt
|*    Ersterstellung    MM 20.03.91
|*    Letzte Aenderung  MM 20.03.91
|*
*************************************************************************/
char * RscChar::MakeUTF8( char * pStr, UINT16 nTextEncoding )
{
    sal_Size        nMaxUniCodeBuf = strlen( pStr ) + 1;
    char *          pOrgStr = new char[ nMaxUniCodeBuf ];
    sal_uInt32      nOrgLen = 0;

    if( nMaxUniCodeBuf * 6 > 0x0FFFFF )
        RscExit( 10 );

    char cOld = '1';
    while( cOld != 0 )
    {
        char c;

        if( *pStr == '\\' )
        {
            ++pStr;
            switch( *pStr )
            {
                case 'a':
                    c = '\a';
                    break;
                case 'b':
                    c = '\b';
                    break;
                case 'f':
                    c = '\f';
                    break;
                case 'n':
                    c = '\n';
                    break;
                case 'r':
                    c = '\r';
                    break;
                case 't':
                    c = '\t';
                    break;
                case 'v':
                    c = '\v';
                    break;
                case '\\':
                    c = '\\';
                    break;
                case '?':
                    c = '\?';
                    break;
                case '\'':
                    c = '\'';
                    break;
                case '\"':
                    c = '\"';
                    break;
                default:
                {
                    if( '0' <= *pStr && '7' >= *pStr )
                    {
                        sal_uInt16  nChar = 0;
                        int  i = 0;
                        while( '0' <= *pStr && '7' >= *pStr && i != 3 )
                        {
                            nChar = nChar * 8 + (BYTE)*pStr - (BYTE)'0';
                            ++pStr;
                            i++;
                        }
                        if( nChar > 255 )
                        {
                            rtl_freeMemory( pOrgStr );

                            // Wert zu gross, oder kein 3 Ziffern
                            return( NULL );
                        }
                        c = (char)nChar;
                        pStr--;
                    }
                    else if( 'x' == *pStr )
                    {
                        sal_uInt16  nChar = 0;
                        int  i = 0;
                        ++pStr;
                        while( isxdigit( *pStr ) && i != 2 )
                        {
                            if( isdigit( *pStr ) )
                                nChar = nChar * 16 + (BYTE)*pStr - (BYTE)'0';
                            else if( isupper( *pStr ) )
                                nChar = nChar * 16 + (BYTE)*pStr - (BYTE)'A' +10;
                            else
                                nChar = nChar * 16 + (BYTE)*pStr - (BYTE)'a' +10;
                            ++pStr;
                            i++;
                        }
                        c = (char)nChar;
                        pStr--;
                    }
                    else
                        c = *pStr;
                };
            }
        }
        else
            c = *pStr;
        pOrgStr[ nOrgLen++ ] = c;
        cOld = *pStr;
        pStr++;
    }

    sal_Unicode *   pUniCode = new sal_Unicode[ nMaxUniCodeBuf ];
    rtl_TextToUnicodeConverter hConv = rtl_createTextToUnicodeConverter( nTextEncoding );

    sal_uInt32 nInfo;
    sal_Size   nSrcCvtBytes;
    sal_Size nUniSize = rtl_convertTextToUnicode( hConv, 0,
                                                pOrgStr, nOrgLen,
                                                pUniCode, nMaxUniCodeBuf,
                                                RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_DEFAULT
                                                | RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_DEFAULT
                                                | RTL_TEXTTOUNICODE_FLAGS_INVALID_DEFAULT
                                                | RTL_TEXTTOUNICODE_FLAGS_FLUSH,
                                                &nInfo,
                                                &nSrcCvtBytes );

    rtl_destroyTextToUnicodeConverter( hConv );

    hConv = rtl_createUnicodeToTextConverter( RTL_TEXTENCODING_UTF8 );
    // factor fo 6 is the maximum size of an UNICODE character as utf8
    char * pUtf8 = (char *)rtl_allocateMemory( nUniSize * 6 );
    rtl_convertUnicodeToText( hConv, 0,
                            pUniCode, nUniSize,
                            pUtf8, nUniSize * 6,
                            RTL_UNICODETOTEXT_FLAGS_UNDEFINED_DEFAULT
                            | RTL_UNICODETOTEXT_FLAGS_INVALID_DEFAULT
                            | RTL_UNICODETOTEXT_FLAGS_FLUSH,
                            &nInfo,
                            &nSrcCvtBytes );

    rtl_destroyTextToUnicodeConverter( hConv );

    delete[] pUniCode;
    delete[] pOrgStr;

    return pUtf8;
};

/*************************************************************************
|*
|*    RscChar::MakeChar()
|*
|*    Beschreibung      Der String wird nach C-Konvention umgesetzt
|*    Ersterstellung    MM 20.03.91
|*    Letzte Aenderung  MM 20.03.91
|*
*************************************************************************/
char * RscChar::MakeUTF8FromL( char * pStr )
{
    sal_Size        nUniPos = 0;
    sal_Unicode *   pUniCode = new sal_Unicode[ strlen( pStr ) + 1 ];

    char cOld = '1';
    while( cOld != 0 )
    {
        sal_Unicode  c;
        if( *pStr == '\\' )
        {
            ++pStr;
            switch( *pStr )
            {
                case 'a':
                    c = '\a';
                    break;
                case 'b':
                    c = '\b';
                    break;
                case 'f':
                    c = '\f';
                    break;
                case 'n':
                    c = '\n';
                    break;
                case 'r':
                    c = '\r';
                    break;
                case 't':
                    c = '\t';
                    break;
                case 'v':
                    c = '\v';
                    break;
                case '\\':
                    c = '\\';
                    break;
                case '?':
                    c = '\?';
                    break;
                case '\'':
                    c = '\'';
                    break;
                case '\"':
                    c = '\"';
                    break;
                default:
                {
                    if( '0' <= *pStr && '7' >= *pStr )
                    {
                        UINT32  nChar = 0;
                        int  i = 0;
                        while( '0' <= *pStr && '7' >= *pStr && i != 6 )
                        {
                            nChar = nChar * 8 + (BYTE)*pStr - (BYTE)'0';
                            ++pStr;
                            i++;
                        }
                        if( nChar > 0xFFFF )
                            // Wert zu gross, oder kein 3 Ziffern
                            return( FALSE );
                        c = (UINT16)nChar;
                        pStr--;
                    }
                    else if( 'x' == *pStr || 'X' == *pStr )
                    {
                        UINT32  nChar = 0;
                        int  i = 0;
                        ++pStr;
                        while( isxdigit( *pStr ) && i != 4 )
                        {
                            if( isdigit( *pStr ) )
                                nChar = nChar * 16 + (BYTE)*pStr - (BYTE)'0';
                            else if( isupper( *pStr ) )
                                nChar = nChar * 16 + (BYTE)*pStr - (BYTE)'A' +10;
                            else
                                nChar = nChar * 16 + (BYTE)*pStr - (BYTE)'a' +10;
                            ++pStr;
                            i++;
                        }
                        c = (UINT16)nChar;
                        pStr--;
                    }
                    else
                        c = *pStr;
                };
            }
        }
        else
            c = *pStr;

        pUniCode[ nUniPos++ ] = c;
        cOld = *pStr;
        pStr++;
    }

    // factor fo 6 is the maximum size of an UNICODE character as utf8
    sal_Size nMaxUtf8Len = nUniPos * 6;
    if( nUniPos * 6 > 0x0FFFFF )
        RscExit( 10 );

    char * pUtf8 = (char *)rtl_allocateMemory( nMaxUtf8Len );
    rtl_TextToUnicodeConverter hConv = rtl_createUnicodeToTextConverter( RTL_TEXTENCODING_UTF8 );

    sal_uInt32 nInfo;
    sal_Size   nSrcCvtBytes;
    rtl_convertUnicodeToText( hConv, 0,
                              pUniCode, nUniPos,
                              pUtf8, nMaxUtf8Len,
                              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_DEFAULT
                              | RTL_UNICODETOTEXT_FLAGS_INVALID_DEFAULT
                              | RTL_UNICODETOTEXT_FLAGS_FLUSH,
                              &nInfo,
                              &nSrcCvtBytes );

    rtl_destroyUnicodeToTextConverter( hConv );

    delete[] pUniCode;

    return pUtf8;
};

