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

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <rsctools.hxx>

#include <rtl/textcvt.h>
#include <rtl/textenc.h>
#include <rtl/alloc.h>

char * RscChar::MakeUTF8( char * pStr, sal_uInt16 nTextEncoding )
{
    sal_Size nMaxUniCodeBuf = strlen( pStr ) + 1;
    if( nMaxUniCodeBuf * 6 > 0x0FFFFF )
        RscExit( 10 );

    char *          pOrgStr = new char[ nMaxUniCodeBuf ];
    sal_uInt32      nOrgLen = 0;

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
                            nChar = nChar * 8 + (sal_uInt8)*pStr - (sal_uInt8)'0';
                            ++pStr;
                            i++;
                        }
                        if( nChar > 255 )
                        {
                            // value is too big, or more than 3 digits
                            delete [] pOrgStr;
                            return nullptr;
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
                                nChar = nChar * 16 + (sal_uInt8)*pStr - (sal_uInt8)'0';
                            else if( isupper( *pStr ) )
                                nChar = nChar * 16 + (sal_uInt8)*pStr - (sal_uInt8)'A' +10;
                            else
                                nChar = nChar * 16 + (sal_uInt8)*pStr - (sal_uInt8)'a' +10;
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
    sal_Size nUniSize = rtl_convertTextToUnicode( hConv, nullptr,
                                                pOrgStr, nOrgLen,
                                                pUniCode, nMaxUniCodeBuf,
                                                RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_DEFAULT
                                                | RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_DEFAULT
                                                | RTL_TEXTTOUNICODE_FLAGS_INVALID_DEFAULT
                                                | RTL_TEXTTOUNICODE_FLAGS_FLUSH,
                                                &nInfo,
                                                &nSrcCvtBytes );

    rtl_destroyTextToUnicodeConverter( hConv );
    delete[] pOrgStr, pOrgStr = nullptr;

    hConv = rtl_createUnicodeToTextConverter( RTL_TEXTENCODING_UTF8 );
    // factor of 6 is the maximum size of an UNICODE character as utf8
    char * pUtf8 = static_cast<char *>(rtl_allocateMemory( nUniSize * 6 ));
    rtl_convertUnicodeToText( hConv, nullptr,
                            pUniCode, nUniSize,
                            pUtf8, nUniSize * 6,
                            RTL_UNICODETOTEXT_FLAGS_UNDEFINED_DEFAULT
                            | RTL_UNICODETOTEXT_FLAGS_INVALID_DEFAULT
                            | RTL_UNICODETOTEXT_FLAGS_FLUSH,
                            &nInfo,
                            &nSrcCvtBytes );

    rtl_destroyTextToUnicodeConverter( hConv );
    delete[] pUniCode, pUniCode = nullptr;

    return pUtf8;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
