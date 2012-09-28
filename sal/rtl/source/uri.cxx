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

#include "surrogates.hxx"

#include "osl/diagnose.h"
#include "rtl/strbuf.hxx"
#include "rtl/textenc.h"
#include "rtl/textcvt.h"
#include "rtl/uri.h"
#include "rtl/ustrbuf.h"
#include "rtl/ustrbuf.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "sal/macros.h"

#include <cstddef>

namespace {

std::size_t const nCharClassSize = 128;

sal_Unicode const cEscapePrefix = 0x25; // '%'

inline bool isDigit(sal_uInt32 nUtf32)
{
    return nUtf32 >= 0x30 && nUtf32 <= 0x39; // '0'--'9'
}

inline bool isAlpha(sal_uInt32 nUtf32)
{
    // 'A'--'Z', 'a'--'z'
    return (
            (nUtf32 >= 0x41 && nUtf32 <= 0x5A) ||
            (nUtf32 >= 0x61 && nUtf32 <= 0x7A)
           );
}

inline bool isHighSurrogate(sal_uInt32 nUtf16)
{
    return SAL_RTL_IS_HIGH_SURROGATE(nUtf16);
}

inline bool isLowSurrogate(sal_uInt32 nUtf16)
{
    return SAL_RTL_IS_LOW_SURROGATE(nUtf16);
}

inline sal_uInt32 combineSurrogates(sal_uInt32 high, sal_uInt32 low)
{
    return SAL_RTL_COMBINE_SURROGATES(high, low);
}

inline int getHexWeight(sal_uInt32 nUtf32)
{
    return nUtf32 >= 0x30 && nUtf32 <= 0x39 ? // '0'--'9'
               static_cast< int >(nUtf32 - 0x30) :
           nUtf32 >= 0x41 && nUtf32 <= 0x46 ? // 'A'--'F'
               static_cast< int >(nUtf32 - 0x41 + 10) :
           nUtf32 >= 0x61 && nUtf32 <= 0x66 ? // 'a'--'f'
               static_cast< int >(nUtf32 - 0x61 + 10) :
               -1; // not a hex digit
}

inline bool isValid(sal_Bool const * pCharClass, sal_uInt32 nUtf32)
{
    return nUtf32 < nCharClassSize && pCharClass[nUtf32];
}

inline void writeUnicode(rtl_uString ** pBuffer, sal_Int32 * pCapacity,
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

/* Read any of the following:

   - sequence of escape sequences representing character from eCharset,
     translated to single UCS4 character; or

   - pair of UTF-16 surrogates, translated to single UCS4 character; or

   _ single UTF-16 character, extended to UCS4 character.
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
            *pType = EscapeChar;
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
                        bUTF8 = sal_False;
                        break;
                    }
                    p += 3;
                    nEncoded |= ((nWeight1 & 3) << 4 | nWeight2) << nShift;
                }
                if (bUTF8 && nEncoded >= nMin && !isHighSurrogate(nEncoded)
                    && !isLowSurrogate(nEncoded) && nEncoded <= 0x10FFFF)
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
            rtl::OStringBuffer aBuf;
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
                    aConverter, 0, aBuf.getStr(), aBuf.getLength(), aDst,
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
                        || (nDstSize == 2 && isHighSurrogate(aDst[0])
                            && isLowSurrogate(aDst[1])));
                    return nDstSize == 1
                        ? aDst[0] : combineSurrogates(aDst[0], aDst[1]);
                }
                else if (nInfo == RTL_TEXTTOUNICODE_INFO_SRCBUFFERTOSMALL
                         && pEnd - p >= 3 && p[0] == cEscapePrefix
                         && (nWeight1 = getHexWeight(p[1])) >= 0
                         && (nWeight2 = getHexWeight(p[2])) >= 0)
                {
                    p += 3;
                    aBuf.append(static_cast< char >(nWeight1 << 4 | nWeight2));
                }
                else if (nInfo == RTL_TEXTTOUNICODE_INFO_SRCBUFFERTOSMALL
                         && p != pEnd && *p <= 0x7F)
                {
                    aBuf.append(static_cast< char >(*p++));
                }
                else
                {
                    assert(
                        (nInfo & RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOSMALL)
                        == 0);
                    break;
                }
            }
            rtl_destroyTextToUnicodeConverter(aConverter);
            *pType = EscapeOctet;
        }
        return nChar;
    }
    else
    {
        *pType = EscapeNo;
        return isHighSurrogate(nChar) && *pBegin < pEnd
               && isLowSurrogate(**pBegin) ?
                   combineSurrogates(nChar, *(*pBegin)++) : nChar;
    }
}

void writeUcs4(rtl_uString ** pBuffer, sal_Int32 * pCapacity, sal_uInt32 nUtf32)
{
    assert(nUtf32 <= 0x10FFFF); // bad UTF-32 char
    if (nUtf32 <= 0xFFFF) {
        writeUnicode(
            pBuffer, pCapacity, static_cast< sal_Unicode >(nUtf32));
    } else {
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
    assert(nUtf32 <= 0x10FFFF); // bad UTF-32 char
    if (eCharset == RTL_TEXTENCODING_UTF8) {
        if (nUtf32 < 0x80)
            writeEscapeOctet(pBuffer, pCapacity, nUtf32);
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
    } else {
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
            aConverter, 0, aSrc, nSrcSize, aDst, sizeof aDst,
            RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR
            | RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR
            | RTL_UNICODETOTEXT_FLAGS_FLUSH,
            &nInfo, &nConverted);
        assert((nInfo & RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL) == 0);
        rtl_destroyUnicodeToTextConverter(aConverter);
        if (nInfo == 0) {
            assert(nConverted == nSrcSize); // bad rtl_convertUnicodeToText
            for (sal_Size i = 0; i < nDstSize; ++i)
                writeEscapeOctet(pBuffer, pCapacity,
                                 static_cast< unsigned char >(aDst[i]));
                    // FIXME  all octets are escaped, even if there is no need
        } else {
            if (bStrict) {
                return false;
            } else {
                writeUcs4(pBuffer, pCapacity, nUtf32);
            }
        }
    }
    return true;
}

struct Component
{
    sal_Unicode const * pBegin;
    sal_Unicode const * pEnd;

    inline Component(): pBegin(0), pEnd(0) {}

    inline bool isPresent() const { return pBegin != 0; }

    inline sal_Int32 getLength() const;
};

inline sal_Int32 Component::getLength() const
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

    if (pPos != pEnd && isAlpha(*pPos))
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
            else if (!isAlpha(*p) && !isDigit(*p) && *p != '+' && *p != '-'
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
            ++pPos;
        pComponents->aAuthority.pEnd = pPos;
    }

    pComponents->aPath.pBegin = pPos;
    while (pPos != pEnd && *pPos != '?' && * pPos != '#')
        ++pPos;
    pComponents->aPath.pEnd = pPos;

    if (pPos != pEnd && *pPos == '?')
    {
        pComponents->aQuery.pBegin = pPos++;
        while (pPos != pEnd && * pPos != '#')
            ++pPos;
        pComponents->aQuery.pEnd = pPos;
    }

    if (pPos != pEnd)
    {
        assert(*pPos == '#');
        pComponents->aFragment.pBegin = pPos;
        pComponents->aFragment.pEnd = pEnd;
    }
}

rtl::OUString joinPaths(Component const & rBasePath, Component const & rRelPath)
{
    assert(rBasePath.isPresent() && *rBasePath.pBegin == '/');
    assert(rRelPath.isPresent());

    // The invariant of aBuffer is that it always starts and ends with a slash
    // (until probably right at the end of the algorithm, when the last segment
    // of rRelPath is added, which does not necessarily end in a slash):
    rtl::OUStringBuffer aBuffer(rBasePath.getLength() + rRelPath.getLength());
        // XXX  numeric overflow

    // Segments "." and ".." within rBasePath are not conisdered special (but
    // are also not removed by ".." segments within rRelPath), RFC 2396 seems a
    // bit unclear about this point:
    sal_Int32 nFixed = 1;
    sal_Unicode const * p = rBasePath.pBegin + 1;
    for (sal_Unicode const * q = p; q != rBasePath.pEnd; ++q)
        if (*q == '/')
        {
            if (
                (q - p == 1 && p[0] == '.') ||
                (q - p == 2 && p[0] == '.' && p[1] == '.')
               )
            {
                nFixed = q + 1 - rBasePath.pBegin;
            }
            p = q + 1;
        }
    aBuffer.append(rBasePath.pBegin, p - rBasePath.pBegin);

    p = rRelPath.pBegin;
    if (p != rRelPath.pEnd)
        for (;;)
        {
            sal_Unicode const * q = p;
            sal_Unicode const * r;
            for (;;)
            {
                if (q == rRelPath.pEnd)
                {
                    r = q;
                    break;
                }
                if (*q == '/')
                {
                    r = q + 1;
                    break;
                }
                ++q;
            }
            if (q - p == 2 && p[0] == '.' && p[1] == '.')
            {
                // Erroneous excess segments ".." within rRelPath are left
                // intact, as the examples in RFC 2396, section C.2, suggest:
                sal_Int32 i = aBuffer.getLength() - 1;
                if (i < nFixed)
                {
                    aBuffer.append(p, r - p);
                    nFixed += 3;
                }
                else
                {
                    while (i > 0 && aBuffer[i - 1] != '/')
                        --i;
                    aBuffer.setLength(i);
                }
            }
            else if (q - p != 1 || *p != '.')
                aBuffer.append(p, r - p);
            if (q == rRelPath.pEnd)
                break;
            p = q + 1;
        }

    return aBuffer.makeStringAndClear();
}

}

sal_Bool const * SAL_CALL rtl_getUriCharClass(rtl_UriCharClass eCharClass)
    SAL_THROW_EXTERN_C()
{
    static sal_Bool const aCharClass[][nCharClassSize]
    = {{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* None */
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* !"#$%&'()*+,-./*/
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0123456789:;<=>?*/
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*@ABCDEFGHIJKLMNO*/
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*PQRSTUVWXYZ[\]^_*/
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*`abcdefghijklmno*/
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0  /*pqrstuvwxyz{|}~ */
       },
       { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* Uric */
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 1, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* !"#$%&'()*+,-./*/
         1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, /*0123456789:;<=>?*/
         1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /*@ABCDEFGHIJKLMNO*/
         1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, /*PQRSTUVWXYZ[\]^_*/
         0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /*`abcdefghijklmno*/
         1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0  /*pqrstuvwxyz{|}~ */
       },
       { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* UricNoSlash */
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 1, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, /* !"#$%&'()*+,-./*/
         1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, /*0123456789:;<=>?*/
         1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /*@ABCDEFGHIJKLMNO*/
         1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, /*PQRSTUVWXYZ[\]^_*/
         0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /*`abcdefghijklmno*/
         1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0  /*pqrstuvwxyz{|}~ */
       },
       { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* RelSegment */
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 1, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, /* !"#$%&'()*+,-./*/
         1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 0, /*0123456789:;<=>?*/
         1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /*@ABCDEFGHIJKLMNO*/
         1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, /*PQRSTUVWXYZ[\]^_*/
         0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /*`abcdefghijklmno*/
         1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0  /*pqrstuvwxyz{|}~ */
       },
       { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* RegName */
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 1, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, /* !"#$%&'()*+,-./*/
         1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, /*0123456789:;<=>?*/
         1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /*@ABCDEFGHIJKLMNO*/
         1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, /*PQRSTUVWXYZ[\]^_*/
         0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /*`abcdefghijklmno*/
         1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0  /*pqrstuvwxyz{|}~ */
       },
       { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* Userinfo */
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 1, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, /* !"#$%&'()*+,-./*/
         1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, /*0123456789:;<=>?*/
         0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /*@ABCDEFGHIJKLMNO*/
         1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, /*PQRSTUVWXYZ[\]^_*/
         0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /*`abcdefghijklmno*/
         1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0  /*pqrstuvwxyz{|}~ */
       },
       { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* Pchar */
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 1, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, /* !"#$%&'()*+,-./*/
         1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, /*0123456789:;<=>?*/
         1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /*@ABCDEFGHIJKLMNO*/
         1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, /*PQRSTUVWXYZ[\]^_*/
         0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /*`abcdefghijklmno*/
         1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0  /*pqrstuvwxyz{|}~ */
       },
       { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* UnoParamValue */
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 1, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, /* !"#$%&'()*+,-./*/
         1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, /*0123456789:;<=>?*/
         1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /*@ABCDEFGHIJKLMNO*/
         1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, /*PQRSTUVWXYZ[\]^_*/
         0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /*`abcdefghijklmno*/
         1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0  /*pqrstuvwxyz{|}~ */
       }};
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
                writeUnicode(pResult, &nCapacity,
                             static_cast< sal_Unicode >(nUtf32));
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
                writeUnicode(pResult, &nCapacity,
                             static_cast< sal_Unicode >(nUtf32));
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
    *pResult = rtl_uStringBuffer_makeStringAndClear( pResult, &nCapacity );
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
                case EscapeNo:
                    writeUcs4(pResult, &nCapacity, nUtf32);
                    break;

                case EscapeOctet:
                    if (eMechanism == rtl_UriDecodeStrict) {
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
    // If pRelUriRef starts with a scheme component it is an absolute URI
    // reference, and we are done (i.e., this algorithm does not support
    // backwards-compatible relative URIs starting with a scheme component, see
    // RFC 2396, section 5.2, step 3):
    Components aRelComponents;
    parseUriRef(pRelUriRef, &aRelComponents);
    if (aRelComponents.aScheme.isPresent())
    {
        rtl_uString_assign(pResult, pRelUriRef);
        return true;
    }

    // Parse pBaseUriRef; if the scheme component is not present or not valid,
    // or the path component is not empty and starts with anything but a slash,
    // an exception is raised:
    Components aBaseComponents;
    parseUriRef(pBaseUriRef, &aBaseComponents);
    if (!aBaseComponents.aScheme.isPresent())
    {
        rtl::OUString aMessage(pBaseUriRef);
        aMessage += rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            " does not start with a scheme component"));
        rtl_uString_assign(pException,
                           const_cast< rtl::OUString & >(aMessage).pData);
        return false;
    }
    if (aBaseComponents.aPath.pBegin != aBaseComponents.aPath.pEnd
        && *aBaseComponents.aPath.pBegin != '/')
    {
        rtl::OUString aMessage(pBaseUriRef);
        aMessage += rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "path component does not start with slash"));
        rtl_uString_assign(pException, aMessage.pData);
        return false;
    }

    // Use the algorithm from RFC 2396, section 5.2, to turn the relative URI
    // into an absolute one (if the relative URI is a reference to the "current
    // document," the "current document" is here taken to be the base URI):
    rtl::OUStringBuffer aBuffer;
    aBuffer.append(aBaseComponents.aScheme.pBegin,
                   aBaseComponents.aScheme.getLength());
    if (aRelComponents.aAuthority.isPresent())
    {
        aBuffer.append(aRelComponents.aAuthority.pBegin,
                       aRelComponents.aAuthority.getLength());
        aBuffer.append(aRelComponents.aPath.pBegin,
                       aRelComponents.aPath.getLength());
        if (aRelComponents.aQuery.isPresent())
            aBuffer.append(aRelComponents.aQuery.pBegin,
                           aRelComponents.aQuery.getLength());
    }
    else
    {
        if (aBaseComponents.aAuthority.isPresent())
            aBuffer.append(aBaseComponents.aAuthority.pBegin,
                           aBaseComponents.aAuthority.getLength());
        if (aRelComponents.aPath.pBegin == aRelComponents.aPath.pEnd
            && !aRelComponents.aQuery.isPresent())
        {
            aBuffer.append(aBaseComponents.aPath.pBegin,
                           aBaseComponents.aPath.getLength());
            if (aBaseComponents.aQuery.isPresent())
                aBuffer.append(aBaseComponents.aQuery.pBegin,
                               aBaseComponents.aQuery.getLength());
        }
        else
        {
            if (*aRelComponents.aPath.pBegin == '/')
                aBuffer.append(aRelComponents.aPath.pBegin,
                               aRelComponents.aPath.getLength());
            else
                aBuffer.append(joinPaths(aBaseComponents.aPath,
                                         aRelComponents.aPath));
            if (aRelComponents.aQuery.isPresent())
                aBuffer.append(aRelComponents.aQuery.pBegin,
                               aRelComponents.aQuery.getLength());
        }
    }
    if (aRelComponents.aFragment.isPresent())
        aBuffer.append(aRelComponents.aFragment.pBegin,
                       aRelComponents.aFragment.getLength());
    rtl_uString_assign(pResult, aBuffer.makeStringAndClear().pData);
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
