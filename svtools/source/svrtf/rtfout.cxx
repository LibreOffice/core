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

#include <tools/debug.hxx>
#include <tools/stream.hxx>
#include <tools/solar.h>
#include <rtl/string.hxx>
#include <rtl/ustrbuf.hxx>
#include <svtools/rtfkeywd.hxx>
#include <svtools/rtfout.hxx>

namespace {

SvStream& Out_Hex( SvStream& rStream, sal_uLong nHex, sal_uInt8 nLen )
{
    sal_Char aNToABuf[] = "0000000000000000";

    DBG_ASSERT( nLen < sizeof(aNToABuf), "too many places" );
    if( nLen >= sizeof(aNToABuf) )
        nLen = (sizeof(aNToABuf)-1);

    // set pointer to end of buffer
    sal_Char* pStr = aNToABuf + (sizeof(aNToABuf)-1);
    for( sal_uInt8 n = 0; n < nLen; ++n )
    {
        *(--pStr) = static_cast<sal_Char>(nHex & 0xf ) + 48;
        if( *pStr > '9' )
            *pStr += 39;
        nHex >>= 4;
    }
    return rStream.WriteCharPtr( pStr );
}

// Ideally, this function should work on (sal_uInt32) Unicode scalar values
// instead of (sal_Unicode) UTF-16 code units.  However, at least "Rich Text
// Format (RTF) Specification Version 1.9.1" available at
// <https://www.microsoft.com/en-us/download/details.aspx?id=10725> does not
// look like it allows non-BMP Unicode characters >= 0x10000 in the \uN notation
// (it only talks about "Unicode character", but then explains how values of N
// greater than 32767 will be expressed as negative signed 16-bit numbers, so
// that smells like \uN is limited to BMP).
// However the "Mathematics" section has an example that shows the code point
// U+1D44E being encoded as UTF-16 surrogate pair "\u-10187?\u-9138?", so
// sal_Unicode actually works fine here.
SvStream& Out_Char(SvStream& rStream, sal_Unicode c,
    int *pUCMode, rtl_TextEncoding eDestEnc)
{
    const sal_Char* pStr = nullptr;
    switch (c)
    {
    case 0x1:
    case 0x2:
        // this are control character of our textattributes and will never be
        // written
        break;
    case 0xA0:
        rStream.WriteCharPtr( "\\~" );
        break;
    case 0xAD:
        rStream.WriteCharPtr( "\\-" );
        break;
    case 0x2011:
        rStream.WriteCharPtr( "\\_" );
        break;
    case '\n':
        pStr = OOO_STRING_SVTOOLS_RTF_LINE;
        break;
    case '\t':
        pStr = OOO_STRING_SVTOOLS_RTF_TAB;
        break;
    default:
        switch(c)
        {
            case 149:
                pStr = OOO_STRING_SVTOOLS_RTF_BULLET;
                break;
            case 150:
                pStr = OOO_STRING_SVTOOLS_RTF_ENDASH;
                break;
            case 151:
                pStr = OOO_STRING_SVTOOLS_RTF_EMDASH;
                break;
            case 145:
                pStr = OOO_STRING_SVTOOLS_RTF_LQUOTE;
                break;
            case 146:
                pStr = OOO_STRING_SVTOOLS_RTF_RQUOTE;
                break;
            case 147:
                pStr = OOO_STRING_SVTOOLS_RTF_LDBLQUOTE;
                break;
            case 148:
                pStr = OOO_STRING_SVTOOLS_RTF_RDBLQUOTE;
                break;
        }

        if (pStr)
            break;

        switch (c)
        {
            case '\\':
            case '}':
            case '{':
                rStream.WriteChar( '\\' ).WriteChar( char(c) );
                break;
            default:
                if (c >= ' ' && c <= '~')
                    rStream.WriteChar( char(c) );
                else
                {
                    //If we can't convert to the dest encoding, or if
                    //it's an uncommon multibyte sequence which most
                    //readers won't be able to handle correctly, then
                    //export as unicode
                    OUString sBuf(&c, 1);
                    OString sConverted;
                    sal_uInt32 const nFlags =
                        RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR |
                        RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR;
                    bool bWriteAsUnicode = !(sBuf.convertToString(&sConverted,
                                         eDestEnc, nFlags))
                                            || (RTL_TEXTENCODING_UTF8==eDestEnc); // #i43933# do not export UTF-8 chars in RTF;
                    if (bWriteAsUnicode)
                    {
                        (void)sBuf.convertToString(&sConverted,
                            eDestEnc, OUSTRING_TO_OSTRING_CVTFLAGS);
                    }
                    const sal_Int32 nLen = sConverted.getLength();

                    if (bWriteAsUnicode && pUCMode)
                    {
                        // then write as unicode - character
                        if (*pUCMode != nLen)
                        {
                            // #i47831# add an additional whitespace, so that
                            // "document whitespaces" are not ignored.;
                            rStream.WriteCharPtr( "\\uc" )
                               .WriteCharPtr( OString::number(nLen).getStr() ).WriteCharPtr( " " );
                            *pUCMode = nLen;
                        }
                        rStream.WriteCharPtr( "\\u" )
                           .WriteCharPtr( OString::number(
                                static_cast<sal_Int32>(c)).getStr() );
                    }

                    for (sal_Int32 nI = 0; nI < nLen; ++nI)
                    {
                        rStream.WriteCharPtr( "\\'" );
                        Out_Hex(rStream, sConverted[nI], 2);
                    }
                }
                break;
        }
        break;
    }

    if (pStr)
        rStream.WriteCharPtr( pStr ).WriteChar( ' ' );

    return rStream;
}

}

SvStream& RTFOutFuncs::Out_String( SvStream& rStream, const OUString& rStr,
    rtl_TextEncoding eDestEnc)
{
    int nUCMode = 1;
    for (sal_Int32 n = 0; n < rStr.getLength(); ++n)
        Out_Char(rStream, rStr[n], &nUCMode, eDestEnc);
    if (nUCMode != 1)
      rStream.WriteCharPtr( "\\uc1" ).WriteCharPtr( " " ); // #i47831# add an additional whitespace, so that "document whitespaces" are not ignored.;
    return rStream;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
