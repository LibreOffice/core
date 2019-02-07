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

#include <rtl/character.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/textenc.h>
#include <rtl/textcvt.h>
#include <rtl/uri.h>
#include <rtl/ustrbuf.h>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.h>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <sal/macros.h>

#include <algorithm>
#include <cstddef>

namespace {

std::size_t const nCharClassSize = 128;

sal_Unicode const cEscapePrefix = 0x25; // '%'

int getHexWeight(sal_uInt32 nUtf32)
{
    return nUtf32 >= 0x30 && nUtf32 <= 0x39 ? // '0'--'9'
               static_cast< int >(nUtf32 - 0x30) :
           nUtf32 >= 0x41 && nUtf32 <= 0x46 ? // 'A'--'F'
               static_cast< int >(nUtf32 - 0x41 + 10) :
           nUtf32 >= 0x61 && nUtf32 <= 0x66 ? // 'a'--'f'
               static_cast< int >(nUtf32 - 0x61 + 10) :
               -1; // not a hex digit
}

bool isValid(sal_Bool const * pCharClass, sal_uInt32 nUtf32)
{
    return nUtf32 < nCharClassSize && pCharClass[nUtf32];
}

void writeUnicode(rtl_uString ** pBuffer, sal_Int32 * pCapacity,
                         sal_Unicode cChar)
{
    rtl_uStringbuffer_insert(pBuffer, pCapacity, (*pBuffer)->length, &cChar, 1);
}

enum EscapeType
{
    EscapeNo,
    EscapeChar,
    EscapeOctet
};

/** Read any of the following:

   @li sequence of escape sequences representing character from eCharset,
       translated to single UCS4 character; or
   @li pair of UTF-16 surrogates, translated to single UCS4 character; or
   @li  single UTF-16 character, extended to UCS4 character.
 */
sal_uInt32 readUcs4(sal_Unicode const ** pBegin, sal_Unicode const * pEnd,
                    bool bEncoded, rtl_TextEncoding eCharset,
                    EscapeType * pType)
{
    sal_uInt32 nChar = *(*pBegin)++;
    int nWeight1;
    int nWeight2;
    if (nChar == cEscapePrefix && bEncoded && pEnd - *pBegin >= 2
        && (nWeight1 = getHexWeight((*pBegin)[0])) >= 0
        && (nWeight2 = getHexWeight((*pBegin)[1])) >= 0)
    {
        *pBegin += 2;
        nChar = static_cast< sal_uInt32 >(nWeight1 << 4 | nWeight2);
        if (nChar <= 0x7F)
        {
            *pType = EscapeChar;
        }
        else if (eCharset == RTL_TEXTENCODING_UTF8)
        {
            if (nChar >= 0xC0 && nChar <= 0xF4)
            {
                sal_uInt32 nEncoded;
                int nShift;
                sal_uInt32 nMin;
                if (nChar <= 0xDF)
                {
                    nEncoded = (nChar & 0x1F) << 6;
                    nShift = 0;
                    nMin = 0x80;
                }
                else if (nChar <= 0xEF)
                {
                    nEncoded = (nChar & 0x0F) << 12;
                    nShift = 6;
                    nMin = 0x800;
                }
                else
                {
                    nEncoded = (nChar & 0x07) << 18;
                    nShift = 12;
                    nMin = 0x10000;
                }

                sal_Unicode const * p = *pBegin;
                bool bUTF8 = true;

                for (; nShift >= 0; nShift -= 6)
                {
                    if (pEnd - p < 3 || p[0] != cEscapePrefix
                        || (nWeight1 = getHexWeight(p[1])) < 8
                        || nWeight1 > 11
                        || (nWeight2 = getHexWeight(p[2])) < 0)
                    {
                        bUTF8 = false;
                        break;
                    }
                    p += 3;
                    nEncoded |= ((nWeight1 & 3) << 4 | nWeight2) << nShift;
                }
                if (bUTF8 && rtl::isUnicodeScalarValue(nEncoded)
                    && nEncoded >= nMin)
                {
                    *pBegin = p;
                    *pType = EscapeChar;
                    return nEncoded;
                }
            }
            *pType = EscapeOctet;
        }
        else
        {
            OStringBuffer aBuf;
            aBuf.append(static_cast< char >(nChar));
            rtl_TextToUnicodeConverter aConverter
                = rtl_createTextToUnicodeConverter(eCharset);
            sal_Unicode const * p = *pBegin;

            for (;;)
            {
                sal_Unicode aDst[2];
                sal_uInt32 nInfo;
                sal_Size nConverted;
                sal_Size nDstSize = rtl_convertTextToUnicode(
                    aConverter, nullptr, aBuf.getStr(), aBuf.getLength(), aDst,
                    SAL_N_ELEMENTS( aDst ),
                    (RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_ERROR
                     | RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR
                     | RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR),
                    &nInfo, &nConverted);

                if (nInfo == 0)
                {
                    assert( nConverted
                        == sal::static_int_cast< sal_uInt32 >(
                            aBuf.getLength()));

                    rtl_destroyTextToUnicodeConverter(aConverter);
                    *pBegin = p;
                    *pType = EscapeChar;

                    assert( nDstSize == 1
                        || (nDstSize == 2 && rtl::isHighSurrogate(aDst[0])
                            && rtl::isLowSurrogate(aDst[1])));

                    return nDstSize == 1
                        ? aDst[0] : rtl::combineSurrogates(aDst[0], aDst[1]);
                }
                if (nInfo == RTL_TEXTTOUNICODE_INFO_SRCBUFFERTOOSMALL
                         && pEnd - p >= 3 && p[0] == cEscapePrefix
                         && (nWeight1 = getHexWeight(p[1])) >= 0
                         && (nWeight2 = getHexWeight(p[2])) >= 0)
                {
                    p += 3;
                    aBuf.append(static_cast< char >(nWeight1 << 4 | nWeight2));
                }
                else if (nInfo == RTL_TEXTTOUNICODE_INFO_SRCBUFFERTOOSMALL
                         && p != pEnd && *p <= 0x7F)
                {
                    aBuf.append(static_cast< char >(*p++));
                }
                else
                {
                    assert(
                        (nInfo & RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOOSMALL)
                        == 0);
                    break;
                }
            }
            rtl_destroyTextToUnicodeConverter(aConverter);
            *pType = EscapeOctet;
        }
        return nChar;
    }

    *pType = EscapeNo;
    return rtl::isHighSurrogate(nChar) && *pBegin < pEnd
           && rtl::isLowSurrogate(**pBegin) ?
               rtl::combineSurrogates(nChar, *(*pBegin)++) : nChar;
}

void writeUcs4(rtl_uString ** pBuffer, sal_Int32 * pCapacity, sal_uInt32 nUtf32)
{
    assert(rtl::isUnicodeCodePoint(nUtf32));
    if (nUtf32 <= 0xFFFF)
    {
        writeUnicode(pBuffer, pCapacity, static_cast< sal_Unicode >(nUtf32));
    }
    else
    {
        nUtf32 -= 0x10000;
        writeUnicode(
            pBuffer, pCapacity,
            static_cast< sal_Unicode >(nUtf32 >> 10 | 0xD800));
        writeUnicode(
            pBuffer, pCapacity,
            static_cast< sal_Unicode >((nUtf32 & 0x3FF) | 0xDC00));
    }
}

void writeEscapeOctet(rtl_uString ** pBuffer, sal_Int32 * pCapacity,
                      sal_uInt32 nOctet)
{
    assert(nOctet <= 0xFF); // bad octet

    static sal_Unicode const aHex[16]
        = { 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
            0x41, 0x42, 0x43, 0x44, 0x45, 0x46 }; /* '0'--'9', 'A'--'F' */

    writeUnicode(pBuffer, pCapacity, cEscapePrefix);
    writeUnicode(pBuffer, pCapacity, aHex[nOctet >> 4]);
    writeUnicode(pBuffer, pCapacity, aHex[nOctet & 15]);
}

bool writeEscapeChar(rtl_uString ** pBuffer, sal_Int32 * pCapacity,
                     sal_uInt32 nUtf32, rtl_TextEncoding eCharset, bool bStrict)
{
    assert(rtl::isUnicodeCodePoint(nUtf32));
    if (eCharset == RTL_TEXTENCODING_UTF8)
    {
        if (nUtf32 < 0x80)
        {
            writeEscapeOctet(pBuffer, pCapacity, nUtf32);
        }
        else if (nUtf32 < 0x800)
        {
            writeEscapeOctet(pBuffer, pCapacity, nUtf32 >> 6 | 0xC0);
            writeEscapeOctet(pBuffer, pCapacity, (nUtf32 & 0x3F) | 0x80);
        }
        else if (nUtf32 < 0x10000)
        {
            writeEscapeOctet(pBuffer, pCapacity, nUtf32 >> 12 | 0xE0);
            writeEscapeOctet(pBuffer, pCapacity, (nUtf32 >> 6 & 0x3F) | 0x80);
            writeEscapeOctet(pBuffer, pCapacity, (nUtf32 & 0x3F) | 0x80);
        }
        else
        {
            writeEscapeOctet(pBuffer, pCapacity, nUtf32 >> 18 | 0xF0);
            writeEscapeOctet(pBuffer, pCapacity, (nUtf32 >> 12 & 0x3F) | 0x80);
            writeEscapeOctet(pBuffer, pCapacity, (nUtf32 >> 6 & 0x3F) | 0x80);
            writeEscapeOctet(pBuffer, pCapacity, (nUtf32 & 0x3F) | 0x80);
        }
    }
    else
    {
        rtl_UnicodeToTextConverter aConverter
            = rtl_createUnicodeToTextConverter(eCharset);
        sal_Unicode aSrc[2];
        sal_Size nSrcSize;
        if (nUtf32 <= 0xFFFF)
        {
            aSrc[0] = static_cast< sal_Unicode >(nUtf32);
            nSrcSize = 1;
        }
        else
        {
            aSrc[0] = static_cast< sal_Unicode >(
                ((nUtf32 - 0x10000) >> 10) | 0xD800);
            aSrc[1] = static_cast< sal_Unicode >(
                ((nUtf32 - 0x10000) & 0x3FF) | 0xDC00);
            nSrcSize = 2;
        }

        sal_Char aDst[32]; // FIXME  random value
        sal_uInt32 nInfo;
        sal_Size nConverted;
        sal_Size nDstSize = rtl_convertUnicodeToText(
            aConverter, nullptr, aSrc, nSrcSize, aDst, sizeof aDst,
            RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR
            | RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR
            | RTL_UNICODETOTEXT_FLAGS_FLUSH,
            &nInfo, &nConverted);
        assert((nInfo & RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL) == 0);
        rtl_destroyUnicodeToTextConverter(aConverter);

        if (nInfo == 0)
        {
            assert(nConverted == nSrcSize); // bad rtl_convertUnicodeToText

            for (sal_Size i = 0; i < nDstSize; ++i)
            {
                writeEscapeOctet(pBuffer, pCapacity,
                                 static_cast< unsigned char >(aDst[i]));
                    // FIXME  all octets are escaped, even if there is no need
            }
        }
        else
        {
            if (bStrict)
                return false;

            writeUcs4(pBuffer, pCapacity, nUtf32);
        }
    }
    return true;
}

struct Component
{
    sal_Unicode const * pBegin;
    sal_Unicode const * pEnd;

    Component(): pBegin(nullptr), pEnd(nullptr) {}

    bool isPresent() const { return pBegin != nullptr; }

    sal_Int32 getLength() const;
};

sal_Int32 Component::getLength() const
{
    assert(isPresent()); // taking length of non-present component
    return static_cast< sal_Int32 >(pEnd - pBegin);
}

struct Components
{
    Component aScheme;
    Component aAuthority;
    Component aPath;
    Component aQuery;
    Component aFragment;
};

void parseUriRef(rtl_uString const * pUriRef, Components * pComponents)
{
    // This algorithm is liberal and accepts various forms of illegal input.

    sal_Unicode const * pBegin = pUriRef->buffer;
    sal_Unicode const * pEnd = pBegin + pUriRef->length;
    sal_Unicode const * pPos = pBegin;

    if (pPos != pEnd && rtl::isAsciiAlpha(*pPos))
    {
        for (sal_Unicode const * p = pPos + 1; p != pEnd; ++p)
        {
            if (*p == ':')
            {
                pComponents->aScheme.pBegin = pBegin;
                pComponents->aScheme.pEnd = ++p;
                pPos = p;
                break;
            }

            if (!rtl::isAsciiAlphanumeric(*p) && *p != '+' && *p != '-'
                     && *p != '.')
            {
                break;
            }
        }
    }

    if (pEnd - pPos >= 2 && pPos[0] == '/' && pPos[1] == '/')
    {
        pComponents->aAuthority.pBegin = pPos;
        pPos += 2;
        while (pPos != pEnd && *pPos != '/' && *pPos != '?' && *pPos != '#')
        {
            ++pPos;
        }

        pComponents->aAuthority.pEnd = pPos;
    }

    pComponents->aPath.pBegin = pPos;
    while (pPos != pEnd && *pPos != '?' && * pPos != '#')
    {
        ++pPos;
    }

    pComponents->aPath.pEnd = pPos;

    if (pPos != pEnd && *pPos == '?')
    {
        pComponents->aQuery.pBegin = pPos++;
        while (pPos != pEnd && * pPos != '#')
        {
            ++pPos;
        }

        pComponents->aQuery.pEnd = pPos;
    }

    if (pPos != pEnd)
    {
        assert(*pPos == '#');
        pComponents->aFragment.pBegin = pPos;
        pComponents->aFragment.pEnd = pEnd;
    }
}

void appendPath(
    OUStringBuffer & buffer, sal_Int32 bufferStart, bool precedingSlash,
    sal_Unicode const * pathBegin, sal_Unicode const * pathEnd)
{
    while (precedingSlash || pathBegin != pathEnd)
    {
        sal_Unicode const * p = pathBegin;
        while (p != pathEnd && *p != '/')
        {
            ++p;
        }

        std::size_t n = p - pathBegin;
        if (n == 1 && pathBegin[0] == '.')
        {
            // input begins with "." -> remove from input (and done):
            //  i.e., !precedingSlash -> !precedingSlash
            // input begins with "./" -> remove from input:
            //  i.e., !precedingSlash -> !precedingSlash
            // input begins with "/." -> replace with "/" in input (and not yet
            // done):
            //  i.e., precedingSlash -> precedingSlash
            // input begins with "/./" -> replace with "/" in input:
            //  i.e., precedingSlash -> precedingSlash
        }
        else if (n == 2 && pathBegin[0] == '.' && pathBegin[1] == '.')
        {
            // input begins with ".." -> remove from input (and done):
            //  i.e., !precedingSlash -> !precedingSlash
            // input begins with "../" -> remove from input
            //  i.e., !precedingSlash -> !precedingSlash
            // input begins with "/.." -> replace with "/" in input, and shrink
            // output (not yet done):
            //  i.e., precedingSlash -> precedingSlash
            // input begins with "/../" -> replace with "/" in input, and shrink
            // output:
            //  i.e., precedingSlash -> precedingSlash
            if (precedingSlash)
            {
                buffer.truncate(
                    bufferStart
                    + std::max<sal_Int32>(
                        rtl_ustr_lastIndexOfChar_WithLength(
                            buffer.getStr() + bufferStart,
                            buffer.getLength() - bufferStart, '/'),
                        0));
            }
        }
        else
        {
            if (precedingSlash)
                buffer.append('/');

            buffer.append(pathBegin, n);
            precedingSlash = p != pathEnd;
        }
        pathBegin = p + (p == pathEnd ? 0 : 1);
    }
}

}

sal_Bool const * SAL_CALL rtl_getUriCharClass(rtl_UriCharClass eCharClass)
    SAL_THROW_EXTERN_C()
{
    static sal_Bool const aCharClass[][nCharClassSize] = {
        {false, false, false, false, false, false, false, false,// None
         false, false, false, false, false, false, false, false,
         false, false, false, false, false, false, false, false,
         false, false, false, false, false, false, false, false,
         false, false, false, false, false, false, false, false,   //  !"#$%&'
         false, false, false, false, false, false, false, false,   // ()*+,-./
         false, false, false, false, false, false, false, false,   // 01234567
         false, false, false, false, false, false, false, false,   // 89:;<=>?
         false, false, false, false, false, false, false, false,   // @ABCDEFG
         false, false, false, false, false, false, false, false,   // HIJKLMNO
         false, false, false, false, false, false, false, false,   // PQRSTUVW
         false, false, false, false, false, false, false, false,   // XYZ[\]^_
         false, false, false, false, false, false, false, false,   // `abcdefg
         false, false, false, false, false, false, false, false,   // hijklmno
         false, false, false, false, false, false, false, false,   // pqrstuvw
         false, false, false, false, false, false, false, false},  // xyz{|}~
        {false, false, false, false, false, false, false, false,// Uric
         false, false, false, false, false, false, false, false,
         false, false, false, false, false, false, false, false,
         false, false, false, false, false, false, false, false,
         false,  true, false, false,  true, false,  true,  true,   //  !"#$%&'
          true,  true,  true,  true,  true,  true,  true,  true,   // ()*+,-./
          true,  true,  true,  true,  true,  true,  true,  true,   // 01234567
          true,  true,  true,  true, false,  true, false,  true,   // 89:;<=>?
          true,  true,  true,  true,  true,  true,  true,  true,   // @ABCDEFG
          true,  true,  true,  true,  true,  true,  true,  true,   // HIJKLMNO
          true,  true,  true,  true,  true,  true,  true,  true,   // PQRSTUVW
          true,  true,  true,  true, false,  true, false,  true,   // XYZ[\]^_
         false,  true,  true,  true,  true,  true,  true,  true,   // `abcdefg
          true,  true,  true,  true,  true,  true,  true,  true,   // hijklmno
          true,  true,  true,  true,  true,  true,  true,  true,   // pqrstuvw
          true,  true,  true, false, false, false,  true, false},  // xyz{|}~
        {false, false, false, false, false, false, false, false,// UricNoSlash
         false, false, false, false, false, false, false, false,
         false, false, false, false, false, false, false, false,
         false, false, false, false, false, false, false, false,
         false,  true, false, false,  true, false,  true,  true,   //  !"#$%&'
          true,  true,  true,  true,  true,  true,  true, false,   // ()*+,-./
          true,  true,  true,  true,  true,  true,  true,  true,   // 01234567
          true,  true,  true,  true, false,  true, false,  true,   // 89:;<=>?
          true,  true,  true,  true,  true,  true,  true,  true,   // @ABCDEFG
          true,  true,  true,  true,  true,  true,  true,  true,   // HIJKLMNO
          true,  true,  true,  true,  true,  true,  true,  true,   // PQRSTUVW
          true,  true,  true, false, false, false, false,  true,   // XYZ[\]^_
         false,  true,  true,  true,  true,  true,  true,  true,   // `abcdefg
          true,  true,  true,  true,  true,  true,  true,  true,   // hijklmno
          true,  true,  true,  true,  true,  true,  true,  true,   // pqrstuvw
          true,  true,  true, false, false, false,  true, false},  // xyz{|}~
        {false, false, false, false, false, false, false, false,// RelSegment
         false, false, false, false, false, false, false, false,
         false, false, false, false, false, false, false, false,
         false, false, false, false, false, false, false, false,
         false,  true, false, false,  true, false,  true,  true,   //  !"#$%&'
          true,  true,  true,  true,  true,  true,  true, false,   // ()*+,-./
          true,  true,  true,  true,  true,  true,  true,  true,   // 01234567
          true,  true, false,  true, false,  true, false, false,   // 89:;<=>?
          true,  true,  true,  true,  true,  true,  true,  true,   // @ABCDEFG
          true,  true,  true,  true,  true,  true,  true,  true,   // HIJKLMNO
          true,  true,  true,  true,  true,  true,  true,  true,   // PQRSTUVW
          true,  true,  true, false, false, false, false,  true,   // XYZ[\]^_
         false,  true,  true,  true,  true,  true,  true,  true,   // `abcdefg
          true,  true,  true,  true,  true,  true,  true,  true,   // hijklmno
          true,  true,  true,  true,  true,  true,  true,  true,   // pqrstuvw
          true,  true,  true, false, false, false,  true, false},  // xyz{|}~
        {false, false, false, false, false, false, false, false,// RegName
         false, false, false, false, false, false, false, false,
         false, false, false, false, false, false, false, false,
         false, false, false, false, false, false, false, false,
         false,  true, false, false,  true, false,  true,  true,   //  !"#$%&'
          true,  true,  true,  true,  true,  true,  true, false,   // ()*+,-./
          true,  true,  true,  true,  true,  true,  true,  true,   // 01234567
          true,  true,  true,  true, false,  true, false, false,   // 89:;<=>?
          true,  true,  true,  true,  true,  true,  true,  true,   // @ABCDEFG
          true,  true,  true,  true,  true,  true,  true,  true,   // HIJKLMNO
          true,  true,  true,  true,  true,  true,  true,  true,   // PQRSTUVW
          true,  true,  true, false, false, false, false,  true,   // XYZ[\]^_
         false,  true,  true,  true,  true,  true,  true,  true,   // `abcdefg
          true,  true,  true,  true,  true,  true,  true,  true,   // hijklmno
          true,  true,  true,  true,  true,  true,  true,  true,   // pqrstuvw
          true,  true,  true, false, false, false,  true, false},  // xyz{|}~
        {false, false, false, false, false, false, false, false,// Userinfo
         false, false, false, false, false, false, false, false,
         false, false, false, false, false, false, false, false,
         false, false, false, false, false, false, false, false,
         false,  true, false, false,  true, false,  true,  true,   //  !"#$%&'
          true,  true,  true,  true,  true,  true,  true, false,   // ()*+,-./
          true,  true,  true,  true,  true,  true,  true,  true,   // 01234567
          true,  true,  true,  true, false,  true, false, false,   // 89:;<=>?
         false,  true,  true,  true,  true,  true,  true,  true,   // @ABCDEFG
          true,  true,  true,  true,  true,  true,  true,  true,   // HIJKLMNO
          true,  true,  true,  true,  true,  true,  true,  true,   // PQRSTUVW
          true,  true,  true, false, false, false, false,  true,   // XYZ[\]^_
         false,  true,  true,  true,  true,  true,  true,  true,   // `abcdefg
          true,  true,  true,  true,  true,  true,  true,  true,   // hijklmno
          true,  true,  true,  true,  true,  true,  true,  true,   // pqrstuvw
          true,  true,  true, false, false, false,  true, false},  // xyz{|}~
        {false, false, false, false, false, false, false, false,// Pchar
         false, false, false, false, false, false, false, false,
         false, false, false, false, false, false, false, false,
         false, false, false, false, false, false, false, false,
         false,  true, false, false,  true, false,  true,  true,   //  !"#$%&'
          true,  true,  true,  true,  true,  true,  true, false,   // ()*+,-./
          true,  true,  true,  true,  true,  true,  true,  true,   // 01234567
          true,  true,  true, false, false,  true, false, false,   // 89:;<=>?
          true,  true,  true,  true,  true,  true,  true,  true,   // @ABCDEFG
          true,  true,  true,  true,  true,  true,  true,  true,   // HIJKLMNO
          true,  true,  true,  true,  true,  true,  true,  true,   // PQRSTUVW
          true,  true,  true, false, false, false, false,  true,   // XYZ[\]^_
         false,  true,  true,  true,  true,  true,  true,  true,   // `abcdefg
          true,  true,  true,  true,  true,  true,  true,  true,   // hijklmno
          true,  true,  true,  true,  true,  true,  true,  true,   // pqrstuvw
          true,  true,  true, false, false, false,  true, false},  // xyz{|}~
        {false, false, false, false, false, false, false, false,// UnoParamValue
         false, false, false, false, false, false, false, false,
         false, false, false, false, false, false, false, false,
         false, false, false, false, false, false, false, false,
         false,  true, false, false,  true, false,  true,  true,   //  !"#$%&'
          true,  true,  true,  true, false,  true,  true,  true,   // ()*+,-./
          true,  true,  true,  true,  true,  true,  true,  true,   // 01234567
          true,  true,  true, false, false, false, false,  true,   // 89:;<=>?
          true,  true,  true,  true,  true,  true,  true,  true,   // @ABCDEFG
          true,  true,  true,  true,  true,  true,  true,  true,   // HIJKLMNO
          true,  true,  true,  true,  true,  true,  true,  true,   // PQRSTUVW
          true,  true,  true, false, false, false, false,  true,   // XYZ[\]^_
         false,  true,  true,  true,  true,  true,  true,  true,   // `abcdefg
          true,  true,  true,  true,  true,  true,  true,  true,   // hijklmno
          true,  true,  true,  true,  true,  true,  true,  true,   // pqrstuvw
          true,  true,  true, false, false, false,  true, false}}; // xyz{|}~

    assert(
        (eCharClass >= 0
         && (sal::static_int_cast< std::size_t >(eCharClass)
             < SAL_N_ELEMENTS(aCharClass)))); // bad eCharClass
    return aCharClass[eCharClass];
}

void SAL_CALL rtl_uriEncode(rtl_uString * pText, sal_Bool const * pCharClass,
                            rtl_UriEncodeMechanism eMechanism,
                            rtl_TextEncoding eCharset, rtl_uString ** pResult)
    SAL_THROW_EXTERN_C()
{
    assert(!pCharClass[0x25]); // make sure the percent sign is encoded...

    sal_Unicode const * p = pText->buffer;
    sal_Unicode const * pEnd = p + pText->length;
    sal_Int32 nCapacity = pText->length;
    rtl_uString_new_WithLength(pResult, nCapacity);

    while (p < pEnd)
    {
        EscapeType eType;
        sal_uInt32 nUtf32 = readUcs4(
            &p, pEnd,
            (eMechanism == rtl_UriEncodeKeepEscapes
             || eMechanism == rtl_UriEncodeCheckEscapes
             || eMechanism == rtl_UriEncodeStrictKeepEscapes),
            eCharset, &eType);

        switch (eType)
        {
        case EscapeNo:
            if (isValid(pCharClass, nUtf32)) // implies nUtf32 <= 0x7F
            {
                writeUnicode(pResult, &nCapacity,
                             static_cast< sal_Unicode >(nUtf32));
            }
            else if (!writeEscapeChar(
                         pResult, &nCapacity, nUtf32, eCharset,
                         (eMechanism == rtl_UriEncodeStrict
                          || eMechanism == rtl_UriEncodeStrictKeepEscapes)))
            {
                rtl_uString_new(pResult);
                return;
            }
            break;

        case EscapeChar:
            if (eMechanism == rtl_UriEncodeCheckEscapes
                && isValid(pCharClass, nUtf32)) // implies nUtf32 <= 0x7F
            {
                writeUnicode(pResult, &nCapacity,
                             static_cast< sal_Unicode >(nUtf32));
            }
            else if (!writeEscapeChar(
                         pResult, &nCapacity, nUtf32, eCharset,
                         (eMechanism == rtl_UriEncodeStrict
                          || eMechanism == rtl_UriEncodeStrictKeepEscapes)))
            {
                rtl_uString_new(pResult);
                return;
            }
            break;

        case EscapeOctet:
            writeEscapeOctet(pResult, &nCapacity, nUtf32);
            break;
        }
    }
    *pResult = rtl_uStringBuffer_makeStringAndClear(pResult, &nCapacity);
}

void SAL_CALL rtl_uriDecode(rtl_uString * pText,
                            rtl_UriDecodeMechanism eMechanism,
                            rtl_TextEncoding eCharset, rtl_uString ** pResult)
    SAL_THROW_EXTERN_C()
{
    switch (eMechanism)
    {
    case rtl_UriDecodeNone:
        rtl_uString_assign(pResult, pText);
        break;

    case rtl_UriDecodeToIuri:
        eCharset = RTL_TEXTENCODING_UTF8;
        [[fallthrough]];
    default: // rtl_UriDecodeWithCharset, rtl_UriDecodeStrict
        {
            sal_Unicode const * p = pText->buffer;
            sal_Unicode const * pEnd = p + pText->length;
            sal_Int32 nCapacity = pText->length;
            rtl_uString_new_WithLength(pResult, nCapacity);

            while (p < pEnd)
            {
                EscapeType eType;
                sal_uInt32 nUtf32 = readUcs4(&p, pEnd, true, eCharset, &eType);
                switch (eType)
                {
                case EscapeChar:
                    if (nUtf32 <= 0x7F && eMechanism == rtl_UriDecodeToIuri)
                    {
                        writeEscapeOctet(pResult, &nCapacity, nUtf32);
                        break;
                    }
                    [[fallthrough]];

                case EscapeNo:
                    writeUcs4(pResult, &nCapacity, nUtf32);
                    break;

                case EscapeOctet:
                    if (eMechanism == rtl_UriDecodeStrict)
                    {
                        rtl_uString_new(pResult);
                        return;
                    }
                    writeEscapeOctet(pResult, &nCapacity, nUtf32);
                    break;
                }
            }

            *pResult = rtl_uStringBuffer_makeStringAndClear( pResult, &nCapacity );
        }
        break;
    }
}

sal_Bool SAL_CALL rtl_uriConvertRelToAbs(rtl_uString * pBaseUriRef,
                                         rtl_uString * pRelUriRef,
                                         rtl_uString ** pResult,
                                         rtl_uString ** pException)
    SAL_THROW_EXTERN_C()
{
    // Use the strict parser algorithm from RFC 3986, section 5.2, to turn the
    // relative URI into an absolute one:
    OUStringBuffer aBuffer;
    Components aRelComponents;
    parseUriRef(pRelUriRef, &aRelComponents);

    if (aRelComponents.aScheme.isPresent())
    {
        aBuffer.append(aRelComponents.aScheme.pBegin,
                       aRelComponents.aScheme.getLength());

        if (aRelComponents.aAuthority.isPresent())
        {
            aBuffer.append(aRelComponents.aAuthority.pBegin,
                           aRelComponents.aAuthority.getLength());
        }

        appendPath(
            aBuffer, aBuffer.getLength(), false, aRelComponents.aPath.pBegin,
            aRelComponents.aPath.pEnd);

        if (aRelComponents.aQuery.isPresent())
        {
            aBuffer.append(aRelComponents.aQuery.pBegin,
                           aRelComponents.aQuery.getLength());
        }
    }
    else
    {
        Components aBaseComponents;
        parseUriRef(pBaseUriRef, &aBaseComponents);
        if (!aBaseComponents.aScheme.isPresent())
        {
            rtl_uString_assign(
                pException,
                (OUString(
                    "<" + OUString(pBaseUriRef)
                    + "> does not start with a scheme component")
                 .pData));
            return false;
        }

        aBuffer.append(aBaseComponents.aScheme.pBegin,
                       aBaseComponents.aScheme.getLength());
        if (aRelComponents.aAuthority.isPresent())
        {
            aBuffer.append(aRelComponents.aAuthority.pBegin,
                           aRelComponents.aAuthority.getLength());
            appendPath(
                aBuffer, aBuffer.getLength(), false,
                aRelComponents.aPath.pBegin, aRelComponents.aPath.pEnd);

            if (aRelComponents.aQuery.isPresent())
            {
                aBuffer.append(aRelComponents.aQuery.pBegin,
                               aRelComponents.aQuery.getLength());
            }
        }
        else
        {
            if (aBaseComponents.aAuthority.isPresent())
            {
                aBuffer.append(aBaseComponents.aAuthority.pBegin,
                               aBaseComponents.aAuthority.getLength());
            }

            if (aRelComponents.aPath.pBegin == aRelComponents.aPath.pEnd)
            {
                aBuffer.append(aBaseComponents.aPath.pBegin,
                               aBaseComponents.aPath.getLength());
                if (aRelComponents.aQuery.isPresent())
                {
                    aBuffer.append(aRelComponents.aQuery.pBegin,
                                   aRelComponents.aQuery.getLength());
                }
                else if (aBaseComponents.aQuery.isPresent())
                {
                    aBuffer.append(aBaseComponents.aQuery.pBegin,
                                   aBaseComponents.aQuery.getLength());
                }
            }
            else
            {
                if (*aRelComponents.aPath.pBegin == '/')
                {
                    appendPath(
                        aBuffer, aBuffer.getLength(), false,
                        aRelComponents.aPath.pBegin, aRelComponents.aPath.pEnd);
                }
                else if (aBaseComponents.aAuthority.isPresent()
                         && aBaseComponents.aPath.pBegin
                            == aBaseComponents.aPath.pEnd)
                {
                    appendPath(
                        aBuffer, aBuffer.getLength(), true,
                        aRelComponents.aPath.pBegin, aRelComponents.aPath.pEnd);
                }
                else
                {
                    sal_Int32 n = aBuffer.getLength();
                    sal_Int32 i = rtl_ustr_lastIndexOfChar_WithLength(
                        aBaseComponents.aPath.pBegin,
                        aBaseComponents.aPath.getLength(), '/');

                    if (i >= 0)
                    {
                        appendPath(
                            aBuffer, n, false, aBaseComponents.aPath.pBegin,
                            aBaseComponents.aPath.pBegin + i);
                    }

                    appendPath(
                        aBuffer, n, i >= 0, aRelComponents.aPath.pBegin,
                        aRelComponents.aPath.pEnd);
                }

                if (aRelComponents.aQuery.isPresent())
                {
                    aBuffer.append(aRelComponents.aQuery.pBegin,
                                   aRelComponents.aQuery.getLength());
                }
            }
        }
    }
    if (aRelComponents.aFragment.isPresent())
    {
        aBuffer.append(aRelComponents.aFragment.pBegin,
                       aRelComponents.aFragment.getLength());
    }

    rtl_uString_assign(pResult, aBuffer.makeStringAndClear().pData);
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
