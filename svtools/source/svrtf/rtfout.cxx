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
#include <tools/string.hxx>
#include <rtl/string.hxx>
#include <rtl/ustrbuf.hxx>
#include <svtools/rtfkeywd.hxx>
#include <svtools/rtfout.hxx>


#if defined(UNX)
const sal_Char RTFOutFuncs::sNewLine = '\012';
#else
const sal_Char RTFOutFuncs::sNewLine[] = "\015\012";
#endif


SvStream& RTFOutFuncs::Out_Char(SvStream& rStream, sal_Unicode c,
    int *pUCMode, rtl_TextEncoding eDestEnc, sal_Bool bWriteHelpFile)
{
    const sal_Char* pStr = 0;
    switch (c)
    {
    case 0x1:
    case 0x2:
        // this are control character of our textattributes and will never be
        // written
        break;
    case 0xA0:
        rStream << "\\~";
        break;
    case 0xAD:
        rStream << "\\-";
        break;
    case 0x2011:
        rStream << "\\_";
        break;
    case '\n':
        pStr = OOO_STRING_SVTOOLS_RTF_LINE;
        break;
    case '\t':
        pStr = OOO_STRING_SVTOOLS_RTF_TAB;
        break;
    default:
        if(!bWriteHelpFile)
        {
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
        }

        switch (c)
        {
            case '\\':
            case '}':
            case '{':
                rStream << '\\' << (sal_Char)c;
                break;
            default:
                if (c >= ' ' && c <= '~')
                    rStream << (sal_Char)c;
                else
                {
                    //If we can't convert to the dest encoding, or if
                    //its an uncommon multibyte sequence which most
                    //readers won't be able to handle correctly, then
                    //If we can't convert to the dest encoding, then
                    //export as unicode
                    OUString sBuf(&c, 1);
                    OString sConverted;
                    sal_uInt32 nFlags =
                        RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR |
                        RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR;
                    bool bWriteAsUnicode = !(sBuf.convertToString(&sConverted,
                                         eDestEnc, nFlags))
                                            || (RTL_TEXTENCODING_UTF8==eDestEnc); // #i43933# do not export UTF-8 chars in RTF;
                    if (bWriteAsUnicode)
                    {
                        sBuf.convertToString(&sConverted,
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
                            rStream << "\\uc"
                                << OString::valueOf(nLen).getStr() << " ";
                            *pUCMode = nLen;
                        }
                        rStream << "\\u"
                            << OString::valueOf(
                                static_cast<sal_Int32>(c)).getStr();
                    }

                    for (sal_Int32 nI = 0; nI < nLen; ++nI)
                    {
                        rStream << "\\'";
                        Out_Hex(rStream, sConverted.getStr()[nI], 2);
                    }
                }
                break;
        }
        break;
    }

    if (pStr)
        rStream << pStr << ' ';

    return rStream;
}

SvStream& RTFOutFuncs::Out_String( SvStream& rStream, const OUString& rStr,
    rtl_TextEncoding eDestEnc, sal_Bool bWriteHelpFile)
{
    int nUCMode = 1;
    for (sal_Int32 n = 0; n < rStr.getLength(); ++n)
        Out_Char(rStream, rStr[n], &nUCMode, eDestEnc, bWriteHelpFile);
    if (nUCMode != 1)
      rStream << "\\uc1"<< " "; // #i47831# add an additional whitespace, so that "document whitespaces" are not ignored.;
    return rStream;
}

SvStream& RTFOutFuncs::Out_Hex( SvStream& rStream, sal_uLong nHex, sal_uInt8 nLen )
{
    sal_Char aNToABuf[] = "0000000000000000";

    DBG_ASSERT( nLen < sizeof(aNToABuf), "zu viele Stellen" );
    if( nLen >= sizeof(aNToABuf) )
        nLen = (sizeof(aNToABuf)-1);

    // Pointer an das Bufferende setzen
    sal_Char* pStr = aNToABuf + (sizeof(aNToABuf)-1);
    for( sal_uInt8 n = 0; n < nLen; ++n )
    {
        *(--pStr) = (sal_Char)(nHex & 0xf ) + 48;
        if( *pStr > '9' )
            *pStr += 39;
        nHex >>= 4;
    }
    return rStream << pStr;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
