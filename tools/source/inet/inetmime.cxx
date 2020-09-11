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

#include <algorithm>
#include <limits>
#include <forward_list>
#include <memory>

#include <sal/log.hxx>
#include <rtl/ustring.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/tencinfo.h>
#include <tools/inetmime.hxx>
#include <rtl/character.hxx>

namespace {

rtl_TextEncoding getCharsetEncoding(const char * pBegin,
                                           const char * pEnd);

/** Check for US-ASCII white space character.

    @param nChar  Some UCS-4 character.

    @return  True if nChar is a US-ASCII white space character (US-ASCII
    0x09 or 0x20).
 */
bool isWhiteSpace(sal_uInt32 nChar)
{
    return nChar == '\t' || nChar == ' ';
}

/** Get the Base 64 digit weight of a US-ASCII character.

    @param nChar  Some UCS-4 character.

    @return  If nChar is a US-ASCII Base 64 digit character (US-ASCII
    'A'--'F', or 'a'--'f', '0'--'9', '+', or '/'), return the
    corresponding weight (0--63); if nChar is the US-ASCII Base 64 padding
    character (US-ASCII '='), return -1; otherwise, return -2.
 */
int getBase64Weight(sal_uInt32 nChar)
{
    return rtl::isAsciiUpperCase(nChar) ? int(nChar - 'A') :
           rtl::isAsciiLowerCase(nChar) ? int(nChar - 'a' + 26) :
           rtl::isAsciiDigit(nChar) ? int(nChar - '0' + 52) :
           nChar == '+' ? 62 :
           nChar == '/' ? 63 :
           nChar == '=' ? -1 : -2;
}

bool startsWithLineFolding(const sal_Unicode * pBegin,
                                            const sal_Unicode * pEnd)
{
    DBG_ASSERT(pBegin && pBegin <= pEnd,
               "startsWithLineFolding(): Bad sequence");

    return pEnd - pBegin >= 3 && pBegin[0] == 0x0D && pBegin[1] == 0x0A
           && isWhiteSpace(pBegin[2]); // CR, LF
}

rtl_TextEncoding translateFromMIME(rtl_TextEncoding
                                                        eEncoding)
{
#if defined(_WIN32)
    return eEncoding == RTL_TEXTENCODING_ISO_8859_1 ?
               RTL_TEXTENCODING_MS_1252 : eEncoding;
#else
    return eEncoding;
#endif
}

bool isMIMECharsetEncoding(rtl_TextEncoding eEncoding)
{
    return rtl_isOctetTextEncoding(eEncoding);
}

std::unique_ptr<sal_Unicode[]> convertToUnicode(const char * pBegin,
                                         const char * pEnd,
                                         rtl_TextEncoding eEncoding,
                                         sal_Size & rSize)
{
    if (eEncoding == RTL_TEXTENCODING_DONTKNOW)
        return nullptr;
    rtl_TextToUnicodeConverter hConverter
        = rtl_createTextToUnicodeConverter(eEncoding);
    rtl_TextToUnicodeContext hContext
        = rtl_createTextToUnicodeContext(hConverter);
    std::unique_ptr<sal_Unicode[]> pBuffer;
    sal_uInt32 nInfo;
    for (sal_Size nBufferSize = pEnd - pBegin;;
         nBufferSize += nBufferSize / 3 + 1)
    {
        pBuffer.reset(new sal_Unicode[nBufferSize]);
        sal_Size nSrcCvtBytes;
        rSize = rtl_convertTextToUnicode(
                    hConverter, hContext, pBegin, pEnd - pBegin, pBuffer.get(),
                    nBufferSize,
                    RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_ERROR
                        | RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR
                        | RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR,
                    &nInfo, &nSrcCvtBytes);
        if (nInfo != RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOOSMALL)
            break;
        pBuffer.reset();
        rtl_resetTextToUnicodeContext(hConverter, hContext);
    }
    rtl_destroyTextToUnicodeContext(hConverter, hContext);
    rtl_destroyTextToUnicodeConverter(hConverter);
    if (nInfo != 0)
    {
        pBuffer.reset();
    }
    return pBuffer;
}

/** Put the UTF-16 encoding of a UTF-32 character into a buffer.

    @param pBuffer  Points to a buffer, must not be null.

    @param nUTF32  A UTF-32 character, must be in the range 0..0x10FFFF.

    @return  A pointer past the UTF-16 characters put into the buffer
    (i.e., pBuffer + 1 or pBuffer + 2).
 */
sal_Unicode * putUTF32Character(sal_Unicode * pBuffer,
                                                 sal_uInt32 nUTF32)
{
    DBG_ASSERT(rtl::isUnicodeCodePoint(nUTF32), "putUTF32Character(): Bad char");
    if (nUTF32 < 0x10000)
        *pBuffer++ = sal_Unicode(nUTF32);
    else
    {
        nUTF32 -= 0x10000;
        *pBuffer++ = sal_Unicode(0xD800 | (nUTF32 >> 10));
        *pBuffer++ = sal_Unicode(0xDC00 | (nUTF32 & 0x3FF));
    }
    return pBuffer;
}

void writeUTF8(OStringBuffer & rSink, sal_uInt32 nChar)
{
    // See RFC 2279 for a discussion of UTF-8.
    DBG_ASSERT(nChar < 0x80000000, "writeUTF8(): Bad char");

    if (nChar < 0x80)
        rSink.append(char(nChar));
    else if (nChar < 0x800)
        rSink.append(char(nChar >> 6 | 0xC0))
             .append(char((nChar & 0x3F) | 0x80));
    else if (nChar < 0x10000)
        rSink.append(char(nChar >> 12 | 0xE0))
             .append(char((nChar >> 6 & 0x3F) | 0x80))
             .append(char((nChar & 0x3F) | 0x80));
    else if (nChar < 0x200000)
        rSink.append(char(nChar >> 18 | 0xF0))
             .append(char((nChar >> 12 & 0x3F) | 0x80))
             .append(char((nChar >> 6 & 0x3F) | 0x80))
             .append(char((nChar & 0x3F) | 0x80));
    else if (nChar < 0x4000000)
        rSink.append(char(nChar >> 24 | 0xF8))
             .append(char((nChar >> 18 & 0x3F) | 0x80))
             .append(char((nChar >> 12 & 0x3F) | 0x80))
             .append(char((nChar >> 6 & 0x3F) | 0x80))
             .append(char((nChar & 0x3F) | 0x80));
    else
        rSink.append(char(nChar >> 30 | 0xFC))
             .append(char((nChar >> 24 & 0x3F) | 0x80))
             .append(char((nChar >> 18 & 0x3F) | 0x80))
             .append(char((nChar >> 12 & 0x3F) | 0x80))
             .append(char((nChar >> 6 & 0x3F) | 0x80))
             .append(char((nChar & 0x3F) | 0x80));
}

bool translateUTF8Char(const char *& rBegin,
                                 const char * pEnd,
                                 sal_uInt32 & rCharacter)
{
    if (rBegin == pEnd || static_cast< unsigned char >(*rBegin) < 0x80
        || static_cast< unsigned char >(*rBegin) >= 0xFE)
        return false;

    int nCount;
    sal_uInt32 nMin;
    sal_uInt32 nUCS4;
    const char * p = rBegin;
    if (static_cast< unsigned char >(*p) < 0xE0)
    {
        nCount = 1;
        nMin = 0x80;
        nUCS4 = static_cast< unsigned char >(*p) & 0x1F;
    }
    else if (static_cast< unsigned char >(*p) < 0xF0)
    {
        nCount = 2;
        nMin = 0x800;
        nUCS4 = static_cast< unsigned char >(*p) & 0xF;
    }
    else if (static_cast< unsigned char >(*p) < 0xF8)
    {
        nCount = 3;
        nMin = 0x10000;
        nUCS4 = static_cast< unsigned char >(*p) & 7;
    }
    else if (static_cast< unsigned char >(*p) < 0xFC)
    {
        nCount = 4;
        nMin = 0x200000;
        nUCS4 = static_cast< unsigned char >(*p) & 3;
    }
    else
    {
        nCount = 5;
        nMin = 0x4000000;
        nUCS4 = static_cast< unsigned char >(*p) & 1;
    }
    ++p;

    for (; nCount-- > 0; ++p)
        if ((static_cast< unsigned char >(*p) & 0xC0) == 0x80)
            nUCS4 = (nUCS4 << 6) | (static_cast< unsigned char >(*p) & 0x3F);
        else
            return false;

    if (!rtl::isUnicodeCodePoint(nUCS4) || nUCS4 < nMin)
        return false;

    rCharacter = nUCS4;
    rBegin = p;
    return true;
}

void appendISO88591(OUStringBuffer & rText, char const * pBegin,
                    char const * pEnd);

struct Parameter
{
    OString m_aAttribute;
    OString m_aCharset;
    OString m_aLanguage;
    OString m_aValue;
    sal_uInt32 m_nSection;
    bool m_bExtended;

    bool operator<(const Parameter& rhs) const // is used by std::list<Parameter>::sort
    {
        int nComp = m_aAttribute.compareTo(rhs.m_aAttribute);
        return nComp < 0 ||
                (nComp == 0 && m_nSection < rhs.m_nSection);
    }
    struct IsSameSection // is used to check container for duplicates with std::any_of
    {
        const OString& rAttribute;
        const sal_uInt32 nSection;
        bool operator()(const Parameter& r) const
        { return r.m_aAttribute == rAttribute && r.m_nSection == nSection; }
    };
};

typedef std::forward_list<Parameter> ParameterList;

bool parseParameters(ParameterList const & rInput,
                     INetContentTypeParameterList * pOutput);

//  appendISO88591

void appendISO88591(OUStringBuffer & rText, char const * pBegin,
                    char const * pEnd)
{
    sal_Int32 nLength = pEnd - pBegin;
    std::unique_ptr<sal_Unicode[]> pBuffer(new sal_Unicode[nLength]);
    for (sal_Unicode * p = pBuffer.get(); pBegin != pEnd;)
        *p++ = static_cast<unsigned char>(*pBegin++);
    rText.append(pBuffer.get(), nLength);
}

//  parseParameters

bool parseParameters(ParameterList const & rInput,
                     INetContentTypeParameterList * pOutput)
{
    if (pOutput)
        pOutput->clear();

    for (auto it = rInput.begin(), itPrev = rInput.end(); it != rInput.end() ; itPrev = it++)
    {
        if (it->m_nSection > 0
            && (itPrev == rInput.end()
                || itPrev->m_nSection != it->m_nSection - 1
                || itPrev->m_aAttribute != it->m_aAttribute))
            return false;
    }

    if (pOutput)
        for (auto it = rInput.begin(), itNext = rInput.begin(); it != rInput.end(); it = itNext)
        {
            bool bCharset = !it->m_aCharset.isEmpty();
            rtl_TextEncoding eEncoding = RTL_TEXTENCODING_DONTKNOW;
            if (bCharset)
                eEncoding
                    = getCharsetEncoding(it->m_aCharset.getStr(),
                                                   it->m_aCharset.getStr()
                                                       + it->m_aCharset.getLength());
            OUStringBuffer aValue(64);
            bool bBadEncoding = false;
            itNext = it;
            do
            {
                sal_Size nSize;
                std::unique_ptr<sal_Unicode[]> pUnicode
                    = convertToUnicode(itNext->m_aValue.getStr(),
                                                 itNext->m_aValue.getStr()
                                                     + itNext->m_aValue.getLength(),
                                                 bCharset && it->m_bExtended ?
                                                     eEncoding :
                                                     RTL_TEXTENCODING_UTF8,
                                                 nSize);
                if (!pUnicode && !(bCharset && it->m_bExtended))
                    pUnicode = convertToUnicode(
                                   itNext->m_aValue.getStr(),
                                   itNext->m_aValue.getStr()
                                       + itNext->m_aValue.getLength(),
                                   RTL_TEXTENCODING_ISO_8859_1, nSize);
                if (!pUnicode)
                {
                    bBadEncoding = true;
                    break;
                }
                aValue.append(pUnicode.get(), static_cast<sal_Int32>(nSize));
                ++itNext;
            }
            while (itNext != rInput.end() && itNext->m_nSection != 0);

            if (bBadEncoding)
            {
                aValue.setLength(0);
                itNext = it;
                do
                {
                    if (itNext->m_bExtended)
                    {
                        for (sal_Int32 i = 0; i < itNext->m_aValue.getLength(); ++i)
                            aValue.append(
                                static_cast<sal_Unicode>(
                                    static_cast<unsigned char>(itNext->m_aValue[i])
                                    | 0xF800)); // map to unicode corporate use sub area
                    }
                    else
                    {
                        for (sal_Int32 i = 0; i < itNext->m_aValue.getLength(); ++i)
                            aValue.append( itNext->m_aValue[i] );
                    }
                    ++itNext;
                }
                while (itNext != rInput.end() && itNext->m_nSection != 0);
            }
            auto const ret = pOutput->insert(
                {it->m_aAttribute,
                 {it->m_aCharset, it->m_aLanguage, aValue.makeStringAndClear(), !bBadEncoding}});
            SAL_INFO_IF(!ret.second, "tools",
                "INetMIME: dropping duplicate parameter: " << it->m_aAttribute);
        }
    return true;
}

/** Check whether some character is valid within an RFC 2045 <token>.

    @param nChar  Some UCS-4 character.

    @return  True if nChar is valid within an RFC 2047 <token> (US-ASCII
    'A'--'Z', 'a'--'z', '0'--'9', '!', '#', '$', '%', '&', ''', '*', '+',
    '-', '.', '^', '_', '`', '{', '|', '}', or '~').
 */
bool isTokenChar(sal_uInt32 nChar)
{
    static const bool aMap[128]
        = { false, false, false, false, false, false, false, false,
            false, false, false, false, false, false, false, false,
            false, false, false, false, false, false, false, false,
            false, false, false, false, false, false, false, false,
            false,  true, false,  true,  true,  true,  true,  true, // !"#$%&'
            false, false,  true,  true, false,  true,  true, false, //()*+,-./
             true,  true,  true,  true,  true,  true,  true,  true, //01234567
             true,  true, false, false, false, false, false, false, //89:;<=>?
            false,  true,  true,  true,  true,  true,  true,  true, //@ABCDEFG
             true,  true,  true,  true,  true,  true,  true,  true, //HIJKLMNO
             true,  true,  true,  true,  true,  true,  true,  true, //PQRSTUVW
             true,  true,  true, false, false, false,  true,  true, //XYZ[\]^_
             true,  true,  true,  true,  true,  true,  true,  true, //`abcdefg
             true,  true,  true,  true,  true,  true,  true,  true, //hijklmno
             true,  true,  true,  true,  true,  true,  true,  true, //pqrstuvw
             true,  true,  true,  true,  true,  true,  true, false  //xyz{|}~
          };
    return rtl::isAscii(nChar) && aMap[nChar];
}

const sal_Unicode * skipComment(const sal_Unicode * pBegin,
                                          const sal_Unicode * pEnd)
{
    DBG_ASSERT(pBegin && pBegin <= pEnd,
               "skipComment(): Bad sequence");

    if (pBegin != pEnd && *pBegin == '(')
    {
        sal_uInt32 nLevel = 0;
        for (const sal_Unicode * p = pBegin; p != pEnd;)
            switch (*p++)
            {
                case '(':
                    ++nLevel;
                    break;

                case ')':
                    if (--nLevel == 0)
                        return p;
                    break;

                case '\\':
                    if (p != pEnd)
                        ++p;
                    break;
            }
    }
    return pBegin;
}

const sal_Unicode * skipLinearWhiteSpaceComment(const sal_Unicode *
                                                              pBegin,
                                                          const sal_Unicode *
                                                              pEnd)
{
    DBG_ASSERT(pBegin && pBegin <= pEnd,
               "skipLinearWhiteSpaceComment(): Bad sequence");

    while (pBegin != pEnd)
        switch (*pBegin)
        {
            case '\t':
            case ' ':
                ++pBegin;
                break;

            case 0x0D: // CR
                if (startsWithLineFolding(pBegin, pEnd))
                    pBegin += 3;
                else
                    return pBegin;
                break;

            case '(':
            {
                const sal_Unicode * p = skipComment(pBegin, pEnd);
                if (p == pBegin)
                    return pBegin;
                pBegin = p;
                break;
            }

            default:
                return pBegin;
        }
    return pBegin;
}

const sal_Unicode * skipQuotedString(const sal_Unicode * pBegin,
                                               const sal_Unicode * pEnd)
{
    DBG_ASSERT(pBegin && pBegin <= pEnd,
               "skipQuotedString(): Bad sequence");

    if (pBegin != pEnd && *pBegin == '"')
        for (const sal_Unicode * p = pBegin + 1; p != pEnd;)
            switch (*p++)
            {
                case 0x0D: // CR
                    if (pEnd - p < 2 || *p++ != 0x0A // LF
                        || !isWhiteSpace(*p++))
                        return pBegin;
                    break;

                case '"':
                    return p;

                case '\\':
                    if (p != pEnd)
                        ++p;
                    break;
            }
    return pBegin;
}

sal_Unicode const * scanParameters(sal_Unicode const * pBegin,
                                             sal_Unicode const * pEnd,
                                             INetContentTypeParameterList *
                                                 pParameters)
{
    ParameterList aList;
    sal_Unicode const * pParameterBegin = pBegin;
    for (sal_Unicode const * p = pParameterBegin;;)
    {
        pParameterBegin = skipLinearWhiteSpaceComment(p, pEnd);
        if (pParameterBegin == pEnd || *pParameterBegin != ';')
            break;
        p = pParameterBegin + 1;

        sal_Unicode const * pAttributeBegin
            = skipLinearWhiteSpaceComment(p, pEnd);
        p = pAttributeBegin;
        bool bDowncaseAttribute = false;
        while (p != pEnd && isTokenChar(*p) && *p != '*')
        {
            bDowncaseAttribute = bDowncaseAttribute || rtl::isAsciiUpperCase(*p);
            ++p;
        }
        if (p == pAttributeBegin)
            break;
        OString aAttribute(pAttributeBegin, p - pAttributeBegin, RTL_TEXTENCODING_ASCII_US);
        if (bDowncaseAttribute)
            aAttribute = aAttribute.toAsciiLowerCase();

        sal_uInt32 nSection = 0;
        if (p != pEnd && *p == '*')
        {
            ++p;
            if (p != pEnd && rtl::isAsciiDigit(*p)
                && !INetMIME::scanUnsigned(p, pEnd, false, nSection))
                break;
        }

        bool bPresent = std::any_of(aList.begin(), aList.end(),
                                    Parameter::IsSameSection{aAttribute, nSection});
        if (bPresent)
            break;

        bool bExtended = false;
        if (p != pEnd && *p == '*')
        {
            ++p;
            bExtended = true;
        }

        p = skipLinearWhiteSpaceComment(p, pEnd);

        if (p == pEnd || *p != '=')
            break;

        p = skipLinearWhiteSpaceComment(p + 1, pEnd);

        OString aCharset;
        OString aLanguage;
        OString aValue;
        if (bExtended)
        {
            if (nSection == 0)
            {
                sal_Unicode const * pCharsetBegin = p;
                bool bDowncaseCharset = false;
                while (p != pEnd && isTokenChar(*p) && *p != '\'')
                {
                    bDowncaseCharset = bDowncaseCharset || rtl::isAsciiUpperCase(*p);
                    ++p;
                }
                if (p == pCharsetBegin)
                    break;
                if (pParameters)
                {
                    aCharset = OString(
                        pCharsetBegin,
                        p - pCharsetBegin,
                        RTL_TEXTENCODING_ASCII_US);
                    if (bDowncaseCharset)
                        aCharset = aCharset.toAsciiLowerCase();
                }

                if (p == pEnd || *p != '\'')
                    break;
                ++p;

                sal_Unicode const * pLanguageBegin = p;
                bool bDowncaseLanguage = false;
                int nLetters = 0;
                for (; p != pEnd; ++p)
                    if (rtl::isAsciiAlpha(*p))
                    {
                        if (++nLetters > 8)
                            break;
                        bDowncaseLanguage = bDowncaseLanguage
                                            || rtl::isAsciiUpperCase(*p);
                    }
                    else if (*p == '-')
                    {
                        if (nLetters == 0)
                            break;
                        nLetters = 0;
                    }
                    else
                        break;
                if (nLetters == 0 || nLetters > 8)
                    break;
                if (pParameters)
                {
                    aLanguage = OString(
                        pLanguageBegin,
                        p - pLanguageBegin,
                        RTL_TEXTENCODING_ASCII_US);
                    if (bDowncaseLanguage)
                        aLanguage = aLanguage.toAsciiLowerCase();
                }

                if (p == pEnd || *p != '\'')
                    break;
                ++p;
            }
            if (pParameters)
            {
                OStringBuffer aSink;
                while (p != pEnd)
                {
                    auto q = p;
                    sal_uInt32 nChar = INetMIME::getUTF32Character(q, pEnd);
                    if (rtl::isAscii(nChar) && !isTokenChar(nChar))
                        break;
                    p = q;
                    if (nChar == '%' && p + 1 < pEnd)
                    {
                        int nWeight1 = INetMIME::getHexWeight(p[0]);
                        int nWeight2 = INetMIME::getHexWeight(p[1]);
                        if (nWeight1 >= 0 && nWeight2 >= 0)
                        {
                            aSink.append(char(nWeight1 << 4 | nWeight2));
                            p += 2;
                            continue;
                        }
                    }
                    writeUTF8(aSink, nChar);
                }
                aValue = aSink.makeStringAndClear();
            }
            else
                while (p != pEnd && (isTokenChar(*p) || !rtl::isAscii(*p)))
                    ++p;
        }
        else if (p != pEnd && *p == '"')
            if (pParameters)
            {
                OStringBuffer aSink(256);
                bool bInvalid = false;
                for (++p;;)
                {
                    if (p == pEnd)
                    {
                        bInvalid = true;
                        break;
                    }
                    sal_uInt32 nChar = INetMIME::getUTF32Character(p, pEnd);
                    if (nChar == '"')
                        break;
                    else if (nChar == 0x0D) // CR
                    {
                        if (pEnd - p < 2 || *p++ != 0x0A // LF
                            || !isWhiteSpace(*p))
                        {
                            bInvalid = true;
                            break;
                        }
                        nChar = static_cast<unsigned char>(*p++);
                    }
                    else if (nChar == '\\')
                    {
                        if (p == pEnd)
                        {
                            bInvalid = true;
                            break;
                        }
                        nChar = INetMIME::getUTF32Character(p, pEnd);
                    }
                    writeUTF8(aSink, nChar);
                }
                if (bInvalid)
                    break;
                aValue = aSink.makeStringAndClear();
            }
            else
            {
                sal_Unicode const * pStringEnd = skipQuotedString(p, pEnd);
                if (p == pStringEnd)
                    break;
                p = pStringEnd;
            }
        else
        {
            sal_Unicode const * pTokenBegin = p;
            while (p != pEnd && (isTokenChar(*p) || !rtl::isAscii(*p)))
                ++p;
            if (p == pTokenBegin)
                break;
            if (pParameters)
                aValue = OString(
                    pTokenBegin, p - pTokenBegin,
                    RTL_TEXTENCODING_UTF8);
        }
        aList.emplace_front(Parameter{aAttribute, aCharset, aLanguage, aValue, nSection, bExtended});
    }
    aList.sort();
    return parseParameters(aList, pParameters) ? pParameterBegin : pBegin;
}

bool equalIgnoreCase(const char * pBegin1,
                               const char * pEnd1,
                               const char * pString2)
{
    DBG_ASSERT(pBegin1 && pBegin1 <= pEnd1 && pString2,
               "equalIgnoreCase(): Bad sequences");

    while (*pString2 != 0)
        if (pBegin1 == pEnd1
            || (rtl::toAsciiUpperCase(static_cast<unsigned char>(*pBegin1++))
                != rtl::toAsciiUpperCase(
                    static_cast<unsigned char>(*pString2++))))
            return false;
    return pBegin1 == pEnd1;
}

struct EncodingEntry
{
    char const * m_aName;
    rtl_TextEncoding m_eEncoding;
};

// The source for the following table is <ftp://ftp.iana.org/in-notes/iana/
// assignments/character-sets> as of Jan, 21 2000 12:46:00, unless  otherwise
// noted:
EncodingEntry const aEncodingMap[]
    = { { "US-ASCII", RTL_TEXTENCODING_ASCII_US },
        { "ANSI_X3.4-1968", RTL_TEXTENCODING_ASCII_US },
        { "ISO-IR-6", RTL_TEXTENCODING_ASCII_US },
        { "ANSI_X3.4-1986", RTL_TEXTENCODING_ASCII_US },
        { "ISO_646.IRV:1991", RTL_TEXTENCODING_ASCII_US },
        { "ASCII", RTL_TEXTENCODING_ASCII_US },
        { "ISO646-US", RTL_TEXTENCODING_ASCII_US },
        { "US", RTL_TEXTENCODING_ASCII_US },
        { "IBM367", RTL_TEXTENCODING_ASCII_US },
        { "CP367", RTL_TEXTENCODING_ASCII_US },
        { "CSASCII", RTL_TEXTENCODING_ASCII_US },
        { "ISO-8859-1", RTL_TEXTENCODING_ISO_8859_1 },
        { "ISO_8859-1:1987", RTL_TEXTENCODING_ISO_8859_1 },
        { "ISO-IR-100", RTL_TEXTENCODING_ISO_8859_1 },
        { "ISO_8859-1", RTL_TEXTENCODING_ISO_8859_1 },
        { "LATIN1", RTL_TEXTENCODING_ISO_8859_1 },
        { "L1", RTL_TEXTENCODING_ISO_8859_1 },
        { "IBM819", RTL_TEXTENCODING_ISO_8859_1 },
        { "CP819", RTL_TEXTENCODING_ISO_8859_1 },
        { "CSISOLATIN1", RTL_TEXTENCODING_ISO_8859_1 },
        { "ISO-8859-2", RTL_TEXTENCODING_ISO_8859_2 },
        { "ISO_8859-2:1987", RTL_TEXTENCODING_ISO_8859_2 },
        { "ISO-IR-101", RTL_TEXTENCODING_ISO_8859_2 },
        { "ISO_8859-2", RTL_TEXTENCODING_ISO_8859_2 },
        { "LATIN2", RTL_TEXTENCODING_ISO_8859_2 },
        { "L2", RTL_TEXTENCODING_ISO_8859_2 },
        { "CSISOLATIN2", RTL_TEXTENCODING_ISO_8859_2 },
        { "ISO-8859-3", RTL_TEXTENCODING_ISO_8859_3 },
        { "ISO_8859-3:1988", RTL_TEXTENCODING_ISO_8859_3 },
        { "ISO-IR-109", RTL_TEXTENCODING_ISO_8859_3 },
        { "ISO_8859-3", RTL_TEXTENCODING_ISO_8859_3 },
        { "LATIN3", RTL_TEXTENCODING_ISO_8859_3 },
        { "L3", RTL_TEXTENCODING_ISO_8859_3 },
        { "CSISOLATIN3", RTL_TEXTENCODING_ISO_8859_3 },
        { "ISO-8859-4", RTL_TEXTENCODING_ISO_8859_4 },
        { "ISO_8859-4:1988", RTL_TEXTENCODING_ISO_8859_4 },
        { "ISO-IR-110", RTL_TEXTENCODING_ISO_8859_4 },
        { "ISO_8859-4", RTL_TEXTENCODING_ISO_8859_4 },
        { "LATIN4", RTL_TEXTENCODING_ISO_8859_4 },
        { "L4", RTL_TEXTENCODING_ISO_8859_4 },
        { "CSISOLATIN4", RTL_TEXTENCODING_ISO_8859_4 },
        { "ISO-8859-5", RTL_TEXTENCODING_ISO_8859_5 },
        { "ISO_8859-5:1988", RTL_TEXTENCODING_ISO_8859_5 },
        { "ISO-IR-144", RTL_TEXTENCODING_ISO_8859_5 },
        { "ISO_8859-5", RTL_TEXTENCODING_ISO_8859_5 },
        { "CYRILLIC", RTL_TEXTENCODING_ISO_8859_5 },
        { "CSISOLATINCYRILLIC", RTL_TEXTENCODING_ISO_8859_5 },
        { "ISO-8859-6", RTL_TEXTENCODING_ISO_8859_6 },
        { "ISO_8859-6:1987", RTL_TEXTENCODING_ISO_8859_6 },
        { "ISO-IR-127", RTL_TEXTENCODING_ISO_8859_6 },
        { "ISO_8859-6", RTL_TEXTENCODING_ISO_8859_6 },
        { "ECMA-114", RTL_TEXTENCODING_ISO_8859_6 },
        { "ASMO-708", RTL_TEXTENCODING_ISO_8859_6 },
        { "ARABIC", RTL_TEXTENCODING_ISO_8859_6 },
        { "CSISOLATINARABIC", RTL_TEXTENCODING_ISO_8859_6 },
        { "ISO-8859-7", RTL_TEXTENCODING_ISO_8859_7 },
        { "ISO_8859-7:1987", RTL_TEXTENCODING_ISO_8859_7 },
        { "ISO-IR-126", RTL_TEXTENCODING_ISO_8859_7 },
        { "ISO_8859-7", RTL_TEXTENCODING_ISO_8859_7 },
        { "ELOT_928", RTL_TEXTENCODING_ISO_8859_7 },
        { "ECMA-118", RTL_TEXTENCODING_ISO_8859_7 },
        { "GREEK", RTL_TEXTENCODING_ISO_8859_7 },
        { "GREEK8", RTL_TEXTENCODING_ISO_8859_7 },
        { "CSISOLATINGREEK", RTL_TEXTENCODING_ISO_8859_7 },
        { "ISO-8859-8", RTL_TEXTENCODING_ISO_8859_8 },
        { "ISO_8859-8:1988", RTL_TEXTENCODING_ISO_8859_8 },
        { "ISO-IR-138", RTL_TEXTENCODING_ISO_8859_8 },
        { "ISO_8859-8", RTL_TEXTENCODING_ISO_8859_8 },
        { "HEBREW", RTL_TEXTENCODING_ISO_8859_8 },
        { "CSISOLATINHEBREW", RTL_TEXTENCODING_ISO_8859_8 },
        { "ISO-8859-9", RTL_TEXTENCODING_ISO_8859_9 },
        { "ISO_8859-9:1989", RTL_TEXTENCODING_ISO_8859_9 },
        { "ISO-IR-148", RTL_TEXTENCODING_ISO_8859_9 },
        { "ISO_8859-9", RTL_TEXTENCODING_ISO_8859_9 },
        { "LATIN5", RTL_TEXTENCODING_ISO_8859_9 },
        { "L5", RTL_TEXTENCODING_ISO_8859_9 },
        { "CSISOLATIN5", RTL_TEXTENCODING_ISO_8859_9 },
        { "ISO-8859-14", RTL_TEXTENCODING_ISO_8859_14 }, // RFC 2047
        { "ISO_8859-15", RTL_TEXTENCODING_ISO_8859_15 },
        { "ISO-8859-15", RTL_TEXTENCODING_ISO_8859_15 }, // RFC 2047
        { "MACINTOSH", RTL_TEXTENCODING_APPLE_ROMAN },
        { "MAC", RTL_TEXTENCODING_APPLE_ROMAN },
        { "CSMACINTOSH", RTL_TEXTENCODING_APPLE_ROMAN },
        { "IBM437", RTL_TEXTENCODING_IBM_437 },
        { "CP437", RTL_TEXTENCODING_IBM_437 },
        { "437", RTL_TEXTENCODING_IBM_437 },
        { "CSPC8CODEPAGE437", RTL_TEXTENCODING_IBM_437 },
        { "IBM850", RTL_TEXTENCODING_IBM_850 },
        { "CP850", RTL_TEXTENCODING_IBM_850 },
        { "850", RTL_TEXTENCODING_IBM_850 },
        { "CSPC850MULTILINGUAL", RTL_TEXTENCODING_IBM_850 },
        { "IBM860", RTL_TEXTENCODING_IBM_860 },
        { "CP860", RTL_TEXTENCODING_IBM_860 },
        { "860", RTL_TEXTENCODING_IBM_860 },
        { "CSIBM860", RTL_TEXTENCODING_IBM_860 },
        { "IBM861", RTL_TEXTENCODING_IBM_861 },
        { "CP861", RTL_TEXTENCODING_IBM_861 },
        { "861", RTL_TEXTENCODING_IBM_861 },
        { "CP-IS", RTL_TEXTENCODING_IBM_861 },
        { "CSIBM861", RTL_TEXTENCODING_IBM_861 },
        { "IBM863", RTL_TEXTENCODING_IBM_863 },
        { "CP863", RTL_TEXTENCODING_IBM_863 },
        { "863", RTL_TEXTENCODING_IBM_863 },
        { "CSIBM863", RTL_TEXTENCODING_IBM_863 },
        { "IBM865", RTL_TEXTENCODING_IBM_865 },
        { "CP865", RTL_TEXTENCODING_IBM_865 },
        { "865", RTL_TEXTENCODING_IBM_865 },
        { "CSIBM865", RTL_TEXTENCODING_IBM_865 },
        { "IBM775", RTL_TEXTENCODING_IBM_775 },
        { "CP775", RTL_TEXTENCODING_IBM_775 },
        { "CSPC775BALTIC", RTL_TEXTENCODING_IBM_775 },
        { "IBM852", RTL_TEXTENCODING_IBM_852 },
        { "CP852", RTL_TEXTENCODING_IBM_852 },
        { "852", RTL_TEXTENCODING_IBM_852 },
        { "CSPCP852", RTL_TEXTENCODING_IBM_852 },
        { "IBM855", RTL_TEXTENCODING_IBM_855 },
        { "CP855", RTL_TEXTENCODING_IBM_855 },
        { "855", RTL_TEXTENCODING_IBM_855 },
        { "CSIBM855", RTL_TEXTENCODING_IBM_855 },
        { "IBM857", RTL_TEXTENCODING_IBM_857 },
        { "CP857", RTL_TEXTENCODING_IBM_857 },
        { "857", RTL_TEXTENCODING_IBM_857 },
        { "CSIBM857", RTL_TEXTENCODING_IBM_857 },
        { "IBM862", RTL_TEXTENCODING_IBM_862 },
        { "CP862", RTL_TEXTENCODING_IBM_862 },
        { "862", RTL_TEXTENCODING_IBM_862 },
        { "CSPC862LATINHEBREW", RTL_TEXTENCODING_IBM_862 },
        { "IBM864", RTL_TEXTENCODING_IBM_864 },
        { "CP864", RTL_TEXTENCODING_IBM_864 },
        { "CSIBM864", RTL_TEXTENCODING_IBM_864 },
        { "IBM866", RTL_TEXTENCODING_IBM_866 },
        { "CP866", RTL_TEXTENCODING_IBM_866 },
        { "866", RTL_TEXTENCODING_IBM_866 },
        { "CSIBM866", RTL_TEXTENCODING_IBM_866 },
        { "IBM869", RTL_TEXTENCODING_IBM_869 },
        { "CP869", RTL_TEXTENCODING_IBM_869 },
        { "869", RTL_TEXTENCODING_IBM_869 },
        { "CP-GR", RTL_TEXTENCODING_IBM_869 },
        { "CSIBM869", RTL_TEXTENCODING_IBM_869 },
        { "WINDOWS-1250", RTL_TEXTENCODING_MS_1250 },
        { "WINDOWS-1251", RTL_TEXTENCODING_MS_1251 },
        { "WINDOWS-1253", RTL_TEXTENCODING_MS_1253 },
        { "WINDOWS-1254", RTL_TEXTENCODING_MS_1254 },
        { "WINDOWS-1255", RTL_TEXTENCODING_MS_1255 },
        { "WINDOWS-1256", RTL_TEXTENCODING_MS_1256 },
        { "WINDOWS-1257", RTL_TEXTENCODING_MS_1257 },
        { "WINDOWS-1258", RTL_TEXTENCODING_MS_1258 },
        { "SHIFT_JIS", RTL_TEXTENCODING_SHIFT_JIS },
        { "MS_KANJI", RTL_TEXTENCODING_SHIFT_JIS },
        { "CSSHIFTJIS", RTL_TEXTENCODING_SHIFT_JIS },
        { "GB2312", RTL_TEXTENCODING_GB_2312 },
        { "CSGB2312", RTL_TEXTENCODING_GB_2312 },
        { "BIG5", RTL_TEXTENCODING_BIG5 },
        { "CSBIG5", RTL_TEXTENCODING_BIG5 },
        { "EUC-JP", RTL_TEXTENCODING_EUC_JP },
        { "EXTENDED_UNIX_CODE_PACKED_FORMAT_FOR_JAPANESE",
          RTL_TEXTENCODING_EUC_JP },
        { "CSEUCPKDFMTJAPANESE", RTL_TEXTENCODING_EUC_JP },
        { "ISO-2022-JP", RTL_TEXTENCODING_ISO_2022_JP },
        { "CSISO2022JP", RTL_TEXTENCODING_ISO_2022_JP },
        { "ISO-2022-CN", RTL_TEXTENCODING_ISO_2022_CN },
        { "KOI8-R", RTL_TEXTENCODING_KOI8_R },
        { "CSKOI8R", RTL_TEXTENCODING_KOI8_R },
        { "UTF-7", RTL_TEXTENCODING_UTF7 },
        { "UTF-8", RTL_TEXTENCODING_UTF8 },
        { "ISO-8859-10", RTL_TEXTENCODING_ISO_8859_10 }, // RFC 2047
        { "ISO-8859-13", RTL_TEXTENCODING_ISO_8859_13 }, // RFC 2047
        { "EUC-KR", RTL_TEXTENCODING_EUC_KR },
        { "CSEUCKR", RTL_TEXTENCODING_EUC_KR },
        { "ISO-2022-KR", RTL_TEXTENCODING_ISO_2022_KR },
        { "CSISO2022KR", RTL_TEXTENCODING_ISO_2022_KR },
        { "ISO-10646-UCS-4", RTL_TEXTENCODING_UCS4 },
        { "CSUCS4", RTL_TEXTENCODING_UCS4 },
        { "ISO-10646-UCS-2", RTL_TEXTENCODING_UCS2 },
        { "CSUNICODE", RTL_TEXTENCODING_UCS2 } };

rtl_TextEncoding getCharsetEncoding(char const * pBegin,
                                              char const * pEnd)
{
    for (const EncodingEntry& i : aEncodingMap)
        if (equalIgnoreCase(pBegin, pEnd, i.m_aName))
            return i.m_eEncoding;
    return RTL_TEXTENCODING_DONTKNOW;
}

}

//  INetMIME

// static
bool INetMIME::isAtomChar(sal_uInt32 nChar)
{
    static const bool aMap[128]
        = { false, false, false, false, false, false, false, false,
            false, false, false, false, false, false, false, false,
            false, false, false, false, false, false, false, false,
            false, false, false, false, false, false, false, false,
            false,  true, false,  true,  true,  true,  true,  true, // !"#$%&'
            false, false,  true,  true, false,  true, false,  true, //()*+,-./
             true,  true,  true,  true,  true,  true,  true,  true, //01234567
             true,  true, false, false, false,  true, false,  true, //89:;<=>?
            false,  true,  true,  true,  true,  true,  true,  true, //@ABCDEFG
             true,  true,  true,  true,  true,  true,  true,  true, //HIJKLMNO
             true,  true,  true,  true,  true,  true,  true,  true, //PQRSTUVW
             true,  true,  true, false, false, false,  true,  true, //XYZ[\]^_
             true,  true,  true,  true,  true,  true,  true,  true, //`abcdefg
             true,  true,  true,  true,  true,  true,  true,  true, //hijklmno
             true,  true,  true,  true,  true,  true,  true,  true, //pqrstuvw
             true,  true,  true,  true,  true,  true,  true, false  //xyz{|}~
          };
    return rtl::isAscii(nChar) && aMap[nChar];
}

// static
bool INetMIME::isIMAPAtomChar(sal_uInt32 nChar)
{
    static const bool aMap[128]
        = { false, false, false, false, false, false, false, false,
            false, false, false, false, false, false, false, false,
            false, false, false, false, false, false, false, false,
            false, false, false, false, false, false, false, false,
            false,  true, false,  true,  true, false,  true,  true, // !"#$%&'
            false, false, false,  true,  true,  true,  true,  true, //()*+,-./
             true,  true,  true,  true,  true,  true,  true,  true, //01234567
             true,  true,  true,  true,  true,  true,  true,  true, //89:;<=>?
             true,  true,  true,  true,  true,  true,  true,  true, //@ABCDEFG
             true,  true,  true,  true,  true,  true,  true,  true, //HIJKLMNO
             true,  true,  true,  true,  true,  true,  true,  true, //PQRSTUVW
             true,  true,  true,  true, false,  true,  true,  true, //XYZ[\]^_
             true,  true,  true,  true,  true,  true,  true,  true, //`abcdefg
             true,  true,  true,  true,  true,  true,  true,  true, //hijklmno
             true,  true,  true,  true,  true,  true,  true,  true, //pqrstuvw
             true,  true,  true, false,  true,  true,  true, false  //xyz{|}~
          };
    return rtl::isAscii(nChar) && aMap[nChar];
}

// static
bool INetMIME::equalIgnoreCase(const sal_Unicode * pBegin1,
                               const sal_Unicode * pEnd1,
                               const char * pString2)
{
    DBG_ASSERT(pBegin1 && pBegin1 <= pEnd1 && pString2,
               "INetMIME::equalIgnoreCase(): Bad sequences");

    while (*pString2 != 0)
        if (pBegin1 == pEnd1
            || (rtl::toAsciiUpperCase(*pBegin1++)
                != rtl::toAsciiUpperCase(
                    static_cast<unsigned char>(*pString2++))))
            return false;
    return pBegin1 == pEnd1;
}

// static
bool INetMIME::scanUnsigned(const sal_Unicode *& rBegin,
                            const sal_Unicode * pEnd, bool bLeadingZeroes,
                            sal_uInt32 & rValue)
{
    sal_uInt64 nTheValue = 0;
    const sal_Unicode * p = rBegin;
    for ( ; p != pEnd; ++p)
    {
        int nWeight = getWeight(*p);
        if (nWeight < 0)
            break;
        nTheValue = 10 * nTheValue + nWeight;
        if (nTheValue > std::numeric_limits< sal_uInt32 >::max())
            return false;
    }
    if (nTheValue == 0 && (p == rBegin || (!bLeadingZeroes && p - rBegin != 1)))
        return false;
    rBegin = p;
    rValue = sal_uInt32(nTheValue);
    return true;
}

// static
sal_Unicode const * INetMIME::scanContentType(
    OUString const & rStr, OUString * pType,
    OUString * pSubType, INetContentTypeParameterList * pParameters)
{
    sal_Unicode const * pBegin = rStr.getStr();
    sal_Unicode const * pEnd = pBegin + rStr.getLength();
    sal_Unicode const * p = skipLinearWhiteSpaceComment(pBegin, pEnd);
    sal_Unicode const * pTypeBegin = p;
    while (p != pEnd && isTokenChar(*p))
    {
        ++p;
    }
    if (p == pTypeBegin)
        return nullptr;
    sal_Unicode const * pTypeEnd = p;

    p = skipLinearWhiteSpaceComment(p, pEnd);
    if (p == pEnd || *p++ != '/')
        return nullptr;

    p = skipLinearWhiteSpaceComment(p, pEnd);
    sal_Unicode const * pSubTypeBegin = p;
    while (p != pEnd && isTokenChar(*p))
    {
        ++p;
    }
    if (p == pSubTypeBegin)
        return nullptr;
    sal_Unicode const * pSubTypeEnd = p;

    if (pType != nullptr)
    {
        *pType = OUString(pTypeBegin, pTypeEnd - pTypeBegin).toAsciiLowerCase();
    }
    if (pSubType != nullptr)
    {
        *pSubType = OUString(pSubTypeBegin, pSubTypeEnd - pSubTypeBegin)
            .toAsciiLowerCase();
    }

    return scanParameters(p, pEnd, pParameters);
}

// static
OUString INetMIME::decodeHeaderFieldBody(const OString& rBody)
{
    // Due to a bug in INetCoreRFC822MessageStream::ConvertTo7Bit(), old
    // versions of StarOffice send mails with header fields where encoded
    // words can be preceded by '=', ',', '.', '"', or '(', and followed by
    // '=', ',', '.', '"', ')', without any required white space in between.
    // And there appear to exist some broken mailers that only encode single
    // letters within words, like "Appel
    // =?iso-8859-1?Q?=E0?=t=?iso-8859-1?Q?=E9?=moin", so it seems best to
    // detect encoded words even when not properly surrounded by white space.

    // Non US-ASCII characters in rBody are treated as ISO-8859-1.

    // encoded-word = "=?"
    //     1*(%x21 / %x23-27 / %x2A-2B / %x2D / %30-39 / %x41-5A / %x5E-7E)
    //     ["*" 1*8ALPHA *("-" 1*8ALPHA)] "?"
    //     ("B?" *(4base64) (4base64 / 3base64 "=" / 2base64 "==")
    //      / "Q?" 1*(%x21-3C / %x3E / %x40-7E / "=" 2HEXDIG))
    //     "?="

    // base64 = ALPHA / DIGIT / "+" / "/"

    const char * pBegin = rBody.getStr();
    const char * pEnd = pBegin + rBody.getLength();

    OUStringBuffer sDecoded;
    const char * pCopyBegin = pBegin;

    /* bool bStartEncodedWord = true; */
    const char * pWSPBegin = pBegin;

    for (const char * p = pBegin; p != pEnd;)
    {
        if (*p == '=' /* && bStartEncodedWord */)
        {
            const char * q = p + 1;
            bool bEncodedWord = q != pEnd && *q++ == '?';

            rtl_TextEncoding eCharsetEncoding = RTL_TEXTENCODING_DONTKNOW;
            if (bEncodedWord)
            {
                const char * pCharsetBegin = q;
                const char * pLanguageBegin = nullptr;
                int nAlphaCount = 0;
                for (bool bDone = false; !bDone;)
                    if (q == pEnd)
                    {
                        bEncodedWord = false;
                        bDone = true;
                    }
                    else
                    {
                        char cChar = *q++;
                        switch (cChar)
                        {
                            case '*':
                                pLanguageBegin = q - 1;
                                nAlphaCount = 0;
                                break;

                            case '-':
                                if (pLanguageBegin != nullptr)
                                {
                                    if (nAlphaCount == 0)
                                        pLanguageBegin = nullptr;
                                    else
                                        nAlphaCount = 0;
                                }
                                break;

                            case '?':
                                if (pCharsetBegin == q - 1)
                                    bEncodedWord = false;
                                else
                                {
                                    eCharsetEncoding
                                        = getCharsetEncoding(
                                              pCharsetBegin,
                                              pLanguageBegin == nullptr
                                              || nAlphaCount == 0 ?
                                                  q - 1 : pLanguageBegin);
                                    bEncodedWord = isMIMECharsetEncoding(
                                                       eCharsetEncoding);
                                    eCharsetEncoding
                                        = translateFromMIME(eCharsetEncoding);
                                }
                                bDone = true;
                                break;

                            default:
                                if (pLanguageBegin != nullptr
                                    && (!rtl::isAsciiAlpha(
                                            static_cast<unsigned char>(cChar))
                                        || ++nAlphaCount > 8))
                                    pLanguageBegin = nullptr;
                                break;
                        }
                    }
            }

            bool bEncodingB = false;
            if (bEncodedWord)
            {
                if (q == pEnd)
                    bEncodedWord = false;
                else
                {
                    switch (*q++)
                    {
                        case 'B':
                        case 'b':
                            bEncodingB = true;
                            break;

                        case 'Q':
                        case 'q':
                            bEncodingB = false;
                            break;

                        default:
                            bEncodedWord = false;
                            break;
                    }
                }
            }

            bEncodedWord = bEncodedWord && q != pEnd && *q++ == '?';

            OStringBuffer sText;
            if (bEncodedWord)
            {
                if (bEncodingB)
                {
                    for (bool bDone = false; !bDone;)
                    {
                        if (pEnd - q < 4)
                        {
                            bEncodedWord = false;
                            bDone = true;
                        }
                        else
                        {
                            bool bFinal = false;
                            int nCount = 3;
                            sal_uInt32 nValue = 0;
                            for (int nShift = 18; nShift >= 0; nShift -= 6)
                            {
                                int nWeight = getBase64Weight(*q++);
                                if (nWeight == -2)
                                {
                                    bEncodedWord = false;
                                    bDone = true;
                                    break;
                                }
                                if (nWeight == -1)
                                {
                                    if (!bFinal)
                                    {
                                        if (nShift >= 12)
                                        {
                                            bEncodedWord = false;
                                            bDone = true;
                                            break;
                                        }
                                        bFinal = true;
                                        nCount = nShift == 6 ? 1 : 2;
                                    }
                                }
                                else
                                    nValue |= nWeight << nShift;
                            }
                            if (bEncodedWord)
                            {
                                for (int nShift = 16; nCount-- > 0; nShift -= 8)
                                    sText.append(char(nValue >> nShift & 0xFF));
                                if (*q == '?')
                                {
                                    ++q;
                                    bDone = true;
                                }
                                if (bFinal && !bDone)
                                {
                                    bEncodedWord = false;
                                    bDone = true;
                                }
                            }
                        }
                    }
                }
                else
                {
                    const char * pEncodedTextBegin = q;
                    const char * pEncodedTextCopyBegin = q;
                    for (bool bDone = false; !bDone;)
                        if (q == pEnd)
                        {
                            bEncodedWord = false;
                            bDone = true;
                        }
                        else
                        {
                            sal_uInt32 nChar = static_cast<unsigned char>(*q++);
                            switch (nChar)
                            {
                                case '=':
                                {
                                    if (pEnd - q < 2)
                                    {
                                        bEncodedWord = false;
                                        bDone = true;
                                        break;
                                    }
                                    int nDigit1 = getHexWeight(q[0]);
                                    int nDigit2 = getHexWeight(q[1]);
                                    if (nDigit1 < 0 || nDigit2 < 0)
                                    {
                                        bEncodedWord = false;
                                        bDone = true;
                                        break;
                                    }
                                    sText.append(rBody.copy(
                                        (pEncodedTextCopyBegin - pBegin),
                                        (q - 1 - pEncodedTextCopyBegin)));
                                    sText.append(char(nDigit1 << 4 | nDigit2));
                                    q += 2;
                                    pEncodedTextCopyBegin = q;
                                    break;
                                }

                                case '?':
                                    if (q - pEncodedTextBegin > 1)
                                        sText.append(rBody.copy(
                                            (pEncodedTextCopyBegin - pBegin),
                                            (q - 1 - pEncodedTextCopyBegin)));
                                    else
                                        bEncodedWord = false;
                                    bDone = true;
                                    break;

                                case '_':
                                    sText.append(rBody.copy(
                                        (pEncodedTextCopyBegin - pBegin),
                                        (q - 1 - pEncodedTextCopyBegin)));
                                    sText.append(' ');
                                    pEncodedTextCopyBegin = q;
                                    break;

                                default:
                                    if (!isVisible(nChar))
                                    {
                                        bEncodedWord = false;
                                        bDone = true;
                                    }
                                    break;
                            }
                        }
                }
            }

            bEncodedWord = bEncodedWord && q != pEnd && *q++ == '=';

            std::unique_ptr<sal_Unicode[]> pUnicodeBuffer;
            sal_Size nUnicodeSize = 0;
            if (bEncodedWord)
            {
                pUnicodeBuffer
                    = convertToUnicode(sText.getStr(),
                                       sText.getStr() + sText.getLength(),
                                       eCharsetEncoding, nUnicodeSize);
                if (!pUnicodeBuffer)
                    bEncodedWord = false;
            }

            if (bEncodedWord)
            {
                appendISO88591(sDecoded, pCopyBegin, pWSPBegin);
                sDecoded.append(
                    pUnicodeBuffer.get(),
                    static_cast< sal_Int32 >(nUnicodeSize));
                pUnicodeBuffer.reset();
                p = q;
                pCopyBegin = p;

                pWSPBegin = p;
                while (p != pEnd && isWhiteSpace(*p))
                    ++p;
                /* bStartEncodedWord = p != pWSPBegin; */
                continue;
            }
        }

        if (p == pEnd)
            break;

        switch (*p++)
        {
            case '"':
                /* bStartEncodedWord = true; */
                break;

            case '(':
                /* bStartEncodedWord = true; */
                break;

            case ')':
                /* bStartEncodedWord = false; */
                break;

            default:
            {
                const char * pUTF8Begin = p - 1;
                const char * pUTF8End = pUTF8Begin;
                sal_uInt32 nCharacter = 0;
                if (translateUTF8Char(pUTF8End, pEnd, nCharacter))
                {
                    appendISO88591(sDecoded, pCopyBegin, p - 1);
                    sal_Unicode aUTF16Buf[2];
                    sal_Int32 nUTF16Len = putUTF32Character(aUTF16Buf, nCharacter) - aUTF16Buf;
                    sDecoded.append(aUTF16Buf, nUTF16Len);
                    p = pUTF8End;
                    pCopyBegin = p;
                }
                /* bStartEncodedWord = false; */
                break;
            }
        }
        pWSPBegin = p;
    }

    appendISO88591(sDecoded, pCopyBegin, pEnd);
    return sDecoded.makeStringAndClear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
