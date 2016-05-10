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

#include <cstddef>
#include <limits>
#include <memory>

#include <osl/diagnose.h>
#include <rtl/alloc.h>
#include <rtl/ustring.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/tencinfo.h>
#include <tools/inetmime.hxx>
#include <rtl/character.hxx>

namespace {

/** Check for US-ASCII white space character.

    @param nChar  Some UCS-4 character.

    @return  True if nChar is a US-ASCII white space character (US-ASCII
    0x09 or 0x20).
 */
inline bool isWhiteSpace(sal_uInt32 nChar);

/** Check whether some character is valid within an RFC 2045 <token>.

    @param nChar  Some UCS-4 character.

    @return  True if nChar is valid within an RFC 2047 <token> (US-ASCII
    'A'--'Z', 'a'--'z', '0'--'9', '!', '#', '$', '%', '&', ''', '*', '+',
    '-', '.', '^', '_', '`', '{', '|', '}', or '~').
 */
bool isTokenChar(sal_uInt32 nChar);

/** Check whether some character is valid within an RFC 2047 <token>.

    @param nChar  Some UCS-4 character.

    @return  True if nChar is valid within an RFC 2047 <token> (US-ASCII
    'A'--'Z', 'a'--'z', '0'--'9', '!', '#', '$', '%', '&', ''', '*', '+',
    '-', '^', '_', '`', '{', '|', '}', or '~').
 */
bool isEncodedWordTokenChar(sal_uInt32 nChar);

/** Get the Base 64 digit weight of a US-ASCII character.

    @param nChar  Some UCS-4 character.

    @return  If nChar is a US-ASCII Base 64 digit character (US-ASCII
    'A'--'F', or 'a'--'f', '0'--'9', '+', or '/'), return the
    corresponding weight (0--63); if nChar is the US-ASCII Base 64 padding
    character (US-ASCII '='), return -1; otherwise, return -2.
 */
inline int getBase64Weight(sal_uInt32 nChar);

inline bool startsWithLineFolding(const sal_Unicode * pBegin,
                                         const sal_Unicode * pEnd);

const sal_Unicode * skipComment(const sal_Unicode * pBegin,
                                       const sal_Unicode * pEnd);

const sal_Unicode * skipLinearWhiteSpaceComment(const sal_Unicode *
                                                           pBegin,
                                                       const sal_Unicode *
                                                           pEnd);

const sal_Unicode * skipQuotedString(const sal_Unicode * pBegin,
                                            const sal_Unicode * pEnd);

sal_Unicode const * scanParameters(sal_Unicode const * pBegin,
                                          sal_Unicode const * pEnd,
                                          INetContentTypeParameterList *
                                              pParameters);

inline rtl_TextEncoding translateToMIME(rtl_TextEncoding
                                                   eEncoding);

inline rtl_TextEncoding translateFromMIME(rtl_TextEncoding
                                                     eEncoding);

const sal_Char * getCharsetName(rtl_TextEncoding eEncoding);

rtl_TextEncoding getCharsetEncoding(const sal_Char * pBegin,
                                           const sal_Char * pEnd);

inline bool isMIMECharsetEncoding(rtl_TextEncoding eEncoding);

sal_Unicode * convertToUnicode(const sal_Char * pBegin,
                                      const sal_Char * pEnd,
                                      rtl_TextEncoding eEncoding,
                                      sal_Size & rSize);

sal_Char * convertFromUnicode(const sal_Unicode * pBegin,
                                     const sal_Unicode * pEnd,
                                     rtl_TextEncoding eEncoding,
                                     sal_Size & rSize);

inline void writeEscapeSequence(INetMIMEOutputSink & rSink,
                                       sal_uInt32 nChar);

void writeUTF8(INetMIMEOutputSink & rSink, sal_uInt32 nChar);

bool translateUTF8Char(const sal_Char *& rBegin,
                              const sal_Char * pEnd,
                              rtl_TextEncoding eEncoding,
                              sal_uInt32 & rCharacter);

/** Put the UTF-16 encoding of a UTF-32 character into a buffer.

    @param pBuffer  Points to a buffer, must not be null.

    @param nUTF32  An UTF-32 character, must be in the range 0..0x10FFFF.

    @return  A pointer past the UTF-16 characters put into the buffer
    (i.e., pBuffer + 1 or pBuffer + 2).
 */
inline sal_Unicode * putUTF32Character(sal_Unicode * pBuffer,
                                              sal_uInt32 nUTF32);

inline bool isWhiteSpace(sal_uInt32 nChar)
{
    return nChar == '\t' || nChar == ' ';
}

inline int getBase64Weight(sal_uInt32 nChar)
{
    return rtl::isAsciiUpperCase(nChar) ? int(nChar - 'A') :
           rtl::isAsciiLowerCase(nChar) ? int(nChar - 'a' + 26) :
           rtl::isAsciiDigit(nChar) ? int(nChar - '0' + 52) :
           nChar == '+' ? 62 :
           nChar == '/' ? 63 :
           nChar == '=' ? -1 : -2;
}

inline bool startsWithLineFolding(const sal_Unicode * pBegin,
                                            const sal_Unicode * pEnd)
{
    DBG_ASSERT(pBegin && pBegin <= pEnd,
               "startsWithLineFolding(): Bad sequence");

    return pEnd - pBegin >= 3 && pBegin[0] == 0x0D && pBegin[1] == 0x0A
           && isWhiteSpace(pBegin[2]); // CR, LF
}

inline rtl_TextEncoding translateToMIME(rtl_TextEncoding eEncoding)
{
#if defined WNT
    return eEncoding == RTL_TEXTENCODING_MS_1252 ?
               RTL_TEXTENCODING_ISO_8859_1 : eEncoding;
#else // WNT
    return eEncoding;
#endif // WNT
}

inline rtl_TextEncoding translateFromMIME(rtl_TextEncoding
                                                        eEncoding)
{
#if defined WNT
    return eEncoding == RTL_TEXTENCODING_ISO_8859_1 ?
               RTL_TEXTENCODING_MS_1252 : eEncoding;
#else
    return eEncoding;
#endif
}

inline bool isMIMECharsetEncoding(rtl_TextEncoding eEncoding)
{
    return rtl_isOctetTextEncoding(eEncoding);
}

sal_Unicode * convertToUnicode(const sal_Char * pBegin,
                                         const sal_Char * pEnd,
                                         rtl_TextEncoding eEncoding,
                                         sal_Size & rSize)
{
    if (eEncoding == RTL_TEXTENCODING_DONTKNOW)
        return nullptr;
    rtl_TextToUnicodeConverter hConverter
        = rtl_createTextToUnicodeConverter(eEncoding);
    rtl_TextToUnicodeContext hContext
        = rtl_createTextToUnicodeContext(hConverter);
    sal_Unicode * pBuffer;
    sal_uInt32 nInfo;
    for (sal_Size nBufferSize = pEnd - pBegin;;
         nBufferSize += nBufferSize / 3 + 1)
    {
        pBuffer = new sal_Unicode[nBufferSize];
        sal_Size nSrcCvtBytes;
        rSize = rtl_convertTextToUnicode(
                    hConverter, hContext, pBegin, pEnd - pBegin, pBuffer,
                    nBufferSize,
                    RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_ERROR
                        | RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR
                        | RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR,
                    &nInfo, &nSrcCvtBytes);
        if (nInfo != RTL_TEXTTOUNICODE_INFO_DESTBUFFERTOSMALL)
            break;
        delete[] pBuffer;
        rtl_resetTextToUnicodeContext(hConverter, hContext);
    }
    rtl_destroyTextToUnicodeContext(hConverter, hContext);
    rtl_destroyTextToUnicodeConverter(hConverter);
    if (nInfo != 0)
    {
        delete[] pBuffer;
        pBuffer = nullptr;
    }
    return pBuffer;
}

sal_Char * convertFromUnicode(const sal_Unicode * pBegin,
                                        const sal_Unicode * pEnd,
                                        rtl_TextEncoding eEncoding,
                                        sal_Size & rSize)
{
    if (eEncoding == RTL_TEXTENCODING_DONTKNOW)
        return nullptr;
    rtl_UnicodeToTextConverter hConverter
        = rtl_createUnicodeToTextConverter(eEncoding);
    rtl_UnicodeToTextContext hContext
        = rtl_createUnicodeToTextContext(hConverter);
    sal_Char * pBuffer;
    sal_uInt32 nInfo;
    for (sal_Size nBufferSize = pEnd - pBegin;;
         nBufferSize += nBufferSize / 3 + 1)
    {
        pBuffer = new sal_Char[nBufferSize];
        sal_Size nSrcCvtBytes;
        rSize = rtl_convertUnicodeToText(
                    hConverter, hContext, pBegin, pEnd - pBegin, pBuffer,
                    nBufferSize,
                    RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR
                        | RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR
                        | RTL_UNICODETOTEXT_FLAGS_UNDEFINED_REPLACE
                        | RTL_UNICODETOTEXT_FLAGS_UNDEFINED_REPLACESTR,
                    &nInfo, &nSrcCvtBytes);
        if (nInfo != RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL)
            break;
        delete[] pBuffer;
        rtl_resetUnicodeToTextContext(hConverter, hContext);
    }
    rtl_destroyUnicodeToTextContext(hConverter, hContext);
    rtl_destroyUnicodeToTextConverter(hConverter);
    if (nInfo != 0)
    {
        delete[] pBuffer;
        pBuffer = nullptr;
    }
    return pBuffer;
}

inline sal_Unicode * putUTF32Character(sal_Unicode * pBuffer,
                                                 sal_uInt32 nUTF32)
{
    DBG_ASSERT(nUTF32 <= 0x10FFFF, "putUTF32Character(): Bad char");
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

inline void writeEscapeSequence(INetMIMEOutputSink & rSink,
                                          sal_uInt32 nChar)
{
    DBG_ASSERT(nChar <= 0xFF, "writeEscapeSequence(): Bad char");
    rSink << '=' << sal_uInt8(INetMIME::getHexDigit(nChar >> 4))
          << sal_uInt8(INetMIME::getHexDigit(nChar & 15));
}

void writeUTF8(INetMIMEOutputSink & rSink, sal_uInt32 nChar)
{
    // See RFC 2279 for a discussion of UTF-8.
    DBG_ASSERT(nChar < 0x80000000, "writeUTF8(): Bad char");

    if (nChar < 0x80)
        rSink << sal_Char(nChar);
    else if (nChar < 0x800)
        rSink << sal_Char(nChar >> 6 | 0xC0)
              << sal_Char((nChar & 0x3F) | 0x80);
    else if (nChar < 0x10000)
        rSink << sal_Char(nChar >> 12 | 0xE0)
              << sal_Char((nChar >> 6 & 0x3F) | 0x80)
              << sal_Char((nChar & 0x3F) | 0x80);
    else if (nChar < 0x200000)
        rSink << sal_Char(nChar >> 18 | 0xF0)
              << sal_Char((nChar >> 12 & 0x3F) | 0x80)
              << sal_Char((nChar >> 6 & 0x3F) | 0x80)
              << sal_Char((nChar & 0x3F) | 0x80);
    else if (nChar < 0x4000000)
        rSink << sal_Char(nChar >> 24 | 0xF8)
              << sal_Char((nChar >> 18 & 0x3F) | 0x80)
              << sal_Char((nChar >> 12 & 0x3F) | 0x80)
              << sal_Char((nChar >> 6 & 0x3F) | 0x80)
              << sal_Char((nChar & 0x3F) | 0x80);
    else
        rSink << sal_Char(nChar >> 30 | 0xFC)
              << sal_Char((nChar >> 24 & 0x3F) | 0x80)
              << sal_Char((nChar >> 18 & 0x3F) | 0x80)
              << sal_Char((nChar >> 12 & 0x3F) | 0x80)
              << sal_Char((nChar >> 6 & 0x3F) | 0x80)
              << sal_Char((nChar & 0x3F) | 0x80);
}

bool translateUTF8Char(const sal_Char *& rBegin,
                                 const sal_Char * pEnd,
                                 rtl_TextEncoding eEncoding,
                                 sal_uInt32 & rCharacter)
{
    if (rBegin == pEnd || static_cast< unsigned char >(*rBegin) < 0x80
        || static_cast< unsigned char >(*rBegin) >= 0xFE)
        return false;

    int nCount;
    sal_uInt32 nMin;
    sal_uInt32 nUCS4;
    const sal_Char * p = rBegin;
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

    if (nUCS4 < nMin || nUCS4 > 0x10FFFF)
        return false;

    if (eEncoding >= RTL_TEXTENCODING_UCS4)
        rCharacter = nUCS4;
    else
    {
        sal_Unicode aUTF16[2];
        const sal_Unicode * pUTF16End = putUTF32Character(aUTF16, nUCS4);
        sal_Size nSize;
        sal_Char * pBuffer = convertFromUnicode(aUTF16, pUTF16End, eEncoding,
                                                nSize);
        if (!pBuffer)
            return false;
        DBG_ASSERT(nSize == 1,
                   "translateUTF8Char(): Bad conversion");
        rCharacter = *pBuffer;
        delete[] pBuffer;
    }
    rBegin = p;
    return true;
}

class Charset
{
    rtl_TextEncoding m_eEncoding;
    const sal_uInt32 * m_pRanges;

public:
    inline Charset(rtl_TextEncoding eTheEncoding,
                   const sal_uInt32 * pTheRanges);

    rtl_TextEncoding getEncoding() const { return m_eEncoding; }

    bool contains(sal_uInt32 nChar) const;
};

inline Charset::Charset(rtl_TextEncoding eTheEncoding,
                        const sal_uInt32 * pTheRanges):
    m_eEncoding(eTheEncoding),
    m_pRanges(pTheRanges)
{
    DBG_ASSERT(m_pRanges, "Charset::Charset(): Bad ranges");
}

void appendISO88591(OUString & rText, sal_Char const * pBegin,
                    sal_Char const * pEnd);

class INetMIMECharsetList_Impl
{
    struct Node
    {
        Charset m_aCharset;
        bool m_bDisabled;
        Node * m_pNext;

        inline Node(const Charset & rTheCharset, bool bTheDisabled,
                    Node * pTheNext);
    };

    Node * m_pFirst;

public:
    INetMIMECharsetList_Impl(): m_pFirst(nullptr) {}

    ~INetMIMECharsetList_Impl();

    void prepend(const Charset & rCharset)
    { m_pFirst = new Node(rCharset, false, m_pFirst); }

    void includes(sal_uInt32 nChar);

    rtl_TextEncoding getPreferredEncoding(rtl_TextEncoding eDefault
                                              = RTL_TEXTENCODING_DONTKNOW)
        const;

    void reset();
};

inline INetMIMECharsetList_Impl::Node::Node(const Charset & rTheCharset,
                                            bool bTheDisabled,
                                            Node * pTheNext):
    m_aCharset(rTheCharset),
    m_bDisabled(bTheDisabled),
    m_pNext(pTheNext)
{}

struct Parameter
{
    Parameter * m_pNext;
    OString m_aAttribute;
    OString m_aCharset;
    OString m_aLanguage;
    OString m_aValue;
    sal_uInt32 m_nSection;
    bool m_bExtended;

    inline Parameter(Parameter * pTheNext, const OString& rTheAttribute,
                     const OString& rTheCharset,
                     const OString& rTheLanguage,
                     const OString& rTheValue, sal_uInt32 nTheSection,
                     bool bTheExtended);
};

inline Parameter::Parameter(Parameter * pTheNext,
                            const OString& rTheAttribute,
                            const OString& rTheCharset,
                            const OString& rTheLanguage,
                            const OString& rTheValue,
                            sal_uInt32 nTheSection, bool bTheExtended):
    m_pNext(pTheNext),
    m_aAttribute(rTheAttribute),
    m_aCharset(rTheCharset),
    m_aLanguage(rTheLanguage),
    m_aValue(rTheValue),
    m_nSection(nTheSection),
    m_bExtended(bTheExtended)
{}

struct ParameterList
{
    Parameter * m_pList;

    ParameterList(): m_pList(nullptr) {}

    inline ~ParameterList();

    Parameter ** find(const OString& rAttribute, sal_uInt32 nSection,
                      bool & rPresent);
};

inline ParameterList::~ParameterList()
{
    while (m_pList)
    {
        Parameter * pNext = m_pList->m_pNext;
        delete m_pList;
        m_pList = pNext;
    }
}

bool parseParameters(ParameterList const & rInput,
                     INetContentTypeParameterList * pOutput);

//  Charset

bool Charset::contains(sal_uInt32 nChar) const
{
    for (const sal_uInt32 * p = m_pRanges;;)
    {
        if (nChar < *p++)
            return false;
        if (nChar <= *p++)
            return true;
    }
}

//  appendISO88591

void appendISO88591(OUString & rText, sal_Char const * pBegin,
                    sal_Char const * pEnd)
{
    sal_Int32 nLength = pEnd - pBegin;
    std::unique_ptr<sal_Unicode[]> pBuffer(new sal_Unicode[nLength]);
    for (sal_Unicode * p = pBuffer.get(); pBegin != pEnd;)
        *p++ = static_cast<unsigned char>(*pBegin++);
    rText += OUString(pBuffer.get(), nLength);
}

//  INetMIMECharsetList_Impl

INetMIMECharsetList_Impl::~INetMIMECharsetList_Impl()
{
    while (m_pFirst)
    {
        Node * pRemove = m_pFirst;
        m_pFirst = m_pFirst->m_pNext;
        delete pRemove;
    }
}

void INetMIMECharsetList_Impl::includes(sal_uInt32 nChar)
{
    for (Node * p = m_pFirst; p; p = p->m_pNext)
        if (!(p->m_bDisabled || p->m_aCharset.contains(nChar)))
            p->m_bDisabled = true;
}

rtl_TextEncoding INetMIMECharsetList_Impl::getPreferredEncoding(rtl_TextEncoding eDefault)
    const
{
    for (Node * p = m_pFirst; p; p = p->m_pNext)
        if (!p->m_bDisabled)
            return p->m_aCharset.getEncoding();
    return eDefault;
}

void INetMIMECharsetList_Impl::reset()
{
    for (Node * p = m_pFirst; p; p = p->m_pNext)
        p->m_bDisabled = false;
}

//  ParameterList

Parameter ** ParameterList::find(const OString& rAttribute,
                                 sal_uInt32 nSection, bool & rPresent)
{
    Parameter ** p = &m_pList;
    for (; *p; p = &(*p)->m_pNext)
    {
        sal_Int32 nCompare = rAttribute.compareTo((*p)->m_aAttribute);
        if (nCompare > 0)
            break;
        else if (nCompare == 0)
        {
            if (nSection > (*p)->m_nSection)
                break;
            else if (nSection == (*p)->m_nSection)
            {
                rPresent = true;
                return p;
            }
        }
    }
    rPresent = false;
    return p;
}

//  parseParameters

bool parseParameters(ParameterList const & rInput,
                     INetContentTypeParameterList * pOutput)
{
    if (pOutput)
        pOutput->clear();

    Parameter * pPrev = nullptr;
    for (Parameter * p = rInput.m_pList; p; p = p->m_pNext)
    {
        if (p->m_nSection > 0
            && (!pPrev
                || pPrev->m_nSection != p->m_nSection - 1
                || pPrev->m_aAttribute != p->m_aAttribute))
            return false;
        pPrev = p;
    }

    if (pOutput)
        for (Parameter * p = rInput.m_pList; p;)
        {
            bool bCharset = !p->m_aCharset.isEmpty();
            rtl_TextEncoding eEncoding = RTL_TEXTENCODING_DONTKNOW;
            if (bCharset)
                eEncoding
                    = getCharsetEncoding(p->m_aCharset.getStr(),
                                                   p->m_aCharset.getStr()
                                                       + rInput.m_pList->
                                                             m_aCharset.
                                                                 getLength());
            OUString aValue;
            bool bBadEncoding = false;
            Parameter * pNext = p;
            do
            {
                sal_Size nSize;
                sal_Unicode * pUnicode
                    = convertToUnicode(pNext->m_aValue.getStr(),
                                                 pNext->m_aValue.getStr()
                                                     + pNext->m_aValue.getLength(),
                                                 bCharset && p->m_bExtended ?
                                                     eEncoding :
                                                     RTL_TEXTENCODING_UTF8,
                                                 nSize);
                if (!pUnicode && !(bCharset && p->m_bExtended))
                    pUnicode = convertToUnicode(
                                   pNext->m_aValue.getStr(),
                                   pNext->m_aValue.getStr()
                                       + pNext->m_aValue.getLength(),
                                   RTL_TEXTENCODING_ISO_8859_1, nSize);
                if (!pUnicode)
                {
                    bBadEncoding = true;
                    break;
                }
                aValue += OUString(pUnicode, static_cast<sal_Int32>(nSize));
                delete[] pUnicode;
                pNext = pNext->m_pNext;
            }
            while (pNext && pNext->m_nSection > 0);
            if (bBadEncoding)
            {
                aValue.clear();
                for (pNext = p;;)
                {
                    if (pNext->m_bExtended)
                    {
                        for (sal_Int32 i = 0; i < pNext->m_aValue.getLength(); ++i)
                            aValue += OUString(sal_Unicode(
                                sal_Unicode(
                                    static_cast<unsigned char>(pNext->m_aValue[i]))
                                | 0xF800));
                    }
                    else
                    {
                        for (sal_Int32 i = 0; i < pNext->m_aValue.getLength(); ++i)
                            aValue += OUString( sal_Unicode(static_cast<unsigned char>(pNext->m_aValue[i])) );
                    }
                    pNext = pNext->m_pNext;
                    if (!pNext || pNext->m_nSection == 0)
                        break;
                };
            }
            auto const ret = pOutput->insert(
                {p->m_aAttribute,
                 {aValue}});
            SAL_INFO_IF(!ret.second, "tools",
                "INetMIME: dropping duplicate parameter: " << p->m_aAttribute);
            p = pNext;
        }
    return true;
}

INetMIMECharsetList_Impl *
createPreferredCharsetList(rtl_TextEncoding eEncoding)
{
    static const sal_uInt32 aUSASCIIRanges[] = { 0, 0x7F, sal_uInt32(-1) };

    static const sal_uInt32 aISO88591Ranges[] = { 0, 0xFF, sal_uInt32(-1) };
        // <ftp://ftp.unicode.org/Public/MAPPINGS/ISO8859/8859-1.TXT> version
        // 1.0 of 1999 July 27

    static const sal_uInt32 aISO88592Ranges[]
        = { 0, 0xA0, 0xA4, 0xA4, 0xA7, 0xA8, 0xAD, 0xAD, 0xB0, 0xB0,
            0xB4, 0xB4, 0xB8, 0xB8, 0xC1, 0xC2, 0xC4, 0xC4, 0xC7, 0xC7,
            0xC9, 0xC9, 0xCB, 0xCB, 0xCD, 0xCE, 0xD3, 0xD4, 0xD6, 0xD7,
            0xDA, 0xDA, 0xDC, 0xDD, 0xDF, 0xDF, 0xE1, 0xE2, 0xE4, 0xE4,
            0xE7, 0xE7, 0xE9, 0xE9, 0xEB, 0xEB, 0xED, 0xEE, 0xF3, 0xF4,
            0xF6, 0xF7, 0xFA, 0xFA, 0xFC, 0xFD, 0x102, 0x107, 0x10C, 0x111,
            0x118, 0x11B, 0x139, 0x13A, 0x13D, 0x13E, 0x141, 0x144,
            0x147, 0x148, 0x150, 0x151, 0x154, 0x155, 0x158, 0x15B,
            0x15E, 0x165, 0x16E, 0x171, 0x179, 0x17E, 0x2C7, 0x2C7,
            0x2D8, 0x2D9, 0x2DB, 0x2DB, 0x2DD, 0x2DD, sal_uInt32(-1) };
        // <ftp://ftp.unicode.org/Public/MAPPINGS/ISO8859/8859-2.TXT> version
        // 1.0 of 1999 July 27

    static const sal_uInt32 aISO88593Ranges[]
        = { 0, 0xA0, 0xA3, 0xA4, 0xA7, 0xA8, 0xAD, 0xAD, 0xB0, 0xB0,
            0xB2, 0xB5, 0xB7, 0xB8, 0xBD, 0xBD, 0xC0, 0xC2, 0xC4, 0xC4,
            0xC7, 0xCF, 0xD1, 0xD4, 0xD6, 0xD7, 0xD9, 0xDC, 0xDF, 0xE2,
            0xE4, 0xE4, 0xE7, 0xEF, 0xF1, 0xF4, 0xF6, 0xF7, 0xF9, 0xFC,
            0x108, 0x10B, 0x11C, 0x121, 0x124, 0x127, 0x130, 0x131,
            0x134, 0x135, 0x15C, 0x15F, 0x16C, 0x16D, 0x17B, 0x17C,
            0x2D8, 0x2D9, sal_uInt32(-1) };
        // <ftp://ftp.unicode.org/Public/MAPPINGS/ISO8859/8859-3.TXT> version
        // 1.0 of 1999 July 27

    static const sal_uInt32 aISO88594Ranges[]
        = { 0, 0xA0, 0xA4, 0xA4, 0xA7, 0xA8, 0xAD, 0xAD, 0xAF, 0xB0,
            0xB4, 0xB4, 0xB8, 0xB8, 0xC1, 0xC6, 0xC9, 0xC9, 0xCB, 0xCB,
            0xCD, 0xCE, 0xD4, 0xD8, 0xDA, 0xDC, 0xDF, 0xDF, 0xE1, 0xE6,
            0xE9, 0xE9, 0xEB, 0xEB, 0xED, 0xEE, 0xF4, 0xF8, 0xFA, 0xFC,
            0x100, 0x101, 0x104, 0x105, 0x10C, 0x10D, 0x110, 0x113,
            0x116, 0x119, 0x122, 0x123, 0x128, 0x12B, 0x12E, 0x12F,
            0x136, 0x138, 0x13B, 0x13C, 0x145, 0x146, 0x14A, 0x14D,
            0x156, 0x157, 0x160, 0x161, 0x166, 0x16B, 0x172, 0x173,
            0x17D, 0x17E, 0x2C7, 0x2C7, 0x2D9, 0x2D9, 0x2DB, 0x2DB,
            sal_uInt32(-1) };
        // <ftp://ftp.unicode.org/Public/MAPPINGS/ISO8859/8859-4.TXT> version
        // 1.0 of 1999 July 27

    static const sal_uInt32 aISO88595Ranges[]
        = { 0, 0xA0, 0xA7, 0xA7, 0xAD, 0xAD, 0x401, 0x40C, 0x40E, 0x44F,
            0x451, 0x45C, 0x45E, 0x45F, 0x2116, 0x2116, sal_uInt32(-1) };
        // <ftp://ftp.unicode.org/Public/MAPPINGS/ISO8859/8859-5.TXT> version
        // 1.0 of 1999 July 27

    static const sal_uInt32 aISO88596Ranges[]
        = { 0, 0xA0, 0xA4, 0xA4, 0xAD, 0xAD, 0x60C, 0x60C, 0x61B, 0x61B,
            0x61F, 0x61F, 0x621, 0x63A, 0x640, 0x652, sal_uInt32(-1) };
        // <ftp://ftp.unicode.org/Public/MAPPINGS/ISO8859/8859-6.TXT> version
        // 1.0 of 1999 July 27

    static const sal_uInt32 aISO88597Ranges[]
        = { 0, 0xA0, 0xA3, 0xA3, 0xA6, 0xA9, 0xAB, 0xAD, 0xB0, 0xB3,
            0xB7, 0xB7, 0xBB, 0xBB, 0xBD, 0xBD, 0x384, 0x386, 0x388, 0x38A,
            0x38C, 0x38C, 0x38E, 0x3A1, 0x3A3, 0x3CE, 0x2015, 0x2015,
            0x2018, 0x2019, sal_uInt32(-1) };
        // <ftp://ftp.unicode.org/Public/MAPPINGS/ISO8859/8859-7.TXT> version
        // 1.0 of 1999 July 27

    static const sal_uInt32 aISO88598Ranges[]
        = { 0, 0xA0, 0xA2, 0xA9, 0xAB, 0xB9, 0xBB, 0xBE, 0xD7, 0xD7,
            0xF7, 0xF7, 0x5D0, 0x5EA, 0x200E, 0x200F, 0x2017, 0x2017,
            sal_uInt32(-1) };
        // <ftp://ftp.unicode.org/Public/MAPPINGS/ISO8859/8859-8.TXT> version
        // 1.1 of 2000-Jan-03

    static const sal_uInt32 aISO88599Ranges[]
        = { 0, 0xCF, 0xD1, 0xDC, 0xDF, 0xEF, 0xF1, 0xFC, 0xFF, 0xFF,
            0x11E, 0x11F, 0x130, 0x131, 0x15E, 0x15F, sal_uInt32(-1) };
        // <ftp://ftp.unicode.org/Public/MAPPINGS/ISO8859/8859-9.TXT> version
        // 1.0 of 1999 July 27

    static const sal_uInt32 aISO885910Ranges[]
        = { 0, 0xA0, 0xA7, 0xA7, 0xAD, 0xAD, 0xB0, 0xB0, 0xB7, 0xB7,
            0xC1, 0xC6, 0xC9, 0xC9, 0xCB, 0xCB, 0xCD, 0xD0, 0xD3, 0xD6,
            0xD8, 0xD8, 0xDA, 0xDF, 0xE1, 0xE6, 0xE9, 0xE9, 0xEB, 0xEB,
            0xED, 0xF0, 0xF3, 0xF6, 0xF8, 0xF8, 0xFA, 0xFE, 0x100, 0x101,
            0x104, 0x105, 0x10C, 0x10D, 0x110, 0x113, 0x116, 0x119,
            0x122, 0x123, 0x128, 0x12B, 0x12E, 0x12F, 0x136, 0x138,
            0x13B, 0x13C, 0x145, 0x146, 0x14A, 0x14D, 0x160, 0x161,
            0x166, 0x16B, 0x172, 0x173, 0x17D, 0x17E, 0x2015, 0x2015,
            sal_uInt32(-1) };
        // <ftp://ftp.unicode.org/Public/MAPPINGS/ISO8859/8859-10.TXT> version
        // 1.1 of 1999 October 11

    static const sal_uInt32 aISO885913Ranges[]
        = { 0, 0xA0, 0xA2, 0xA4, 0xA6, 0xA7, 0xA9, 0xA9, 0xAB, 0xAE,
            0xB0, 0xB3, 0xB5, 0xB7, 0xB9, 0xB9, 0xBB, 0xBE, 0xC4, 0xC6,
            0xC9, 0xC9, 0xD3, 0xD3, 0xD5, 0xD8, 0xDC, 0xDC, 0xDF, 0xDF,
            0xE4, 0xE6, 0xE9, 0xE9, 0xF3, 0xF3, 0xF5, 0xF8, 0xFC, 0xFC,
            0x100, 0x101, 0x104, 0x107, 0x10C, 0x10D, 0x112, 0x113,
            0x116, 0x119, 0x122, 0x123, 0x12A, 0x12B, 0x12E, 0x12F,
            0x136, 0x137, 0x13B, 0x13C, 0x141, 0x146, 0x14C, 0x14D,
            0x156, 0x157, 0x15A, 0x15B, 0x160, 0x161, 0x16A, 0x16B,
            0x172, 0x173, 0x179, 0x17E, 0x2019, 0x2019, 0x201C, 0x201E,
            sal_uInt32(-1) };
        // <ftp://ftp.unicode.org/Public/MAPPINGS/ISO8859/8859-13.TXT> version
        // 1.0 of 1999 July 27

    static const sal_uInt32 aISO885914Ranges[]
        = { 0, 0xA0, 0xA3, 0xA3, 0xA7, 0xA7, 0xA9, 0xA9, 0xAD, 0xAE,
            0xB6, 0xB6, 0xC0, 0xCF, 0xD1, 0xD6, 0xD8, 0xDD, 0xDF, 0xEF,
            0xF1, 0xF6, 0xF8, 0xFD, 0xFF, 0xFF, 0x10A, 0x10B, 0x120, 0x121,
            0x174, 0x178, 0x1E02, 0x1E03, 0x1E0A, 0x1E0B, 0x1E1E, 0x1E1F,
            0x1E40, 0x1E41, 0x1E56, 0x1E57, 0x1E60, 0x1E61, 0x1E6A, 0x1E6B,
            0x1E80, 0x1E85, 0x1EF2, 0x1EF3, sal_uInt32(-1) };
        // <ftp://ftp.unicode.org/Public/MAPPINGS/ISO8859/8859-14.TXT> version
        // 1.0 of 1999 July 27

    static const sal_uInt32 aISO885915Ranges[]
        = { 0, 0xA3, 0xA5, 0xA5, 0xA7, 0xA7, 0xA9, 0xB3, 0xB5, 0xB7,
            0xB9, 0xBB, 0xBF, 0xFF, 0x152, 0x153, 0x160, 0x161, 0x178, 0x178,
            0x17D, 0x17E, 0x20AC, 0x20AC, sal_uInt32(-1) };
        // <ftp://ftp.unicode.org/Public/MAPPINGS/ISO8859/8859-15.TXT> version
        // 1.0 of 1999 July 27

    static const sal_uInt32 aKOI8RRanges[]
        = { 0, 0x7F, 0xA0, 0xA0, 0xA9, 0xA9, 0xB0, 0xB0, 0xB2, 0xB2,
            0xB7, 0xB7, 0xF7, 0xF7, 0x401, 0x401, 0x410, 0x44F, 0x451, 0x451,
            0x2219, 0x221A, 0x2248, 0x2248, 0x2264, 0x2265, 0x2320, 0x2321,
            0x2500, 0x2500, 0x2502, 0x2502, 0x250C, 0x250C, 0x2510, 0x2510,
            0x2514, 0x2514, 0x2518, 0x2518, 0x251C, 0x251C, 0x2524, 0x2524,
            0x252C, 0x252C, 0x2534, 0x2534, 0x253C, 0x253C, 0x2550, 0x256C,
            0x2580, 0x2580, 0x2584, 0x2584, 0x2588, 0x2588, 0x258C, 0x258C,
            0x2590, 0x2593, 0x25A0, 0x25A0, sal_uInt32(-1) };
        // <ftp://ftp.unicode.org/Public/MAPPINGS/VENDORS/MISC/KOI8-R.TXT>
        // version 1.0 of 18 August 1999

#if defined WNT
    static const sal_uInt32 aWindows1252Ranges[]
        = { 0, 0x7F, 0xA0, 0xFF, 0x152, 0x153, 0x160, 0x161, 0x178, 0x178,
            0x17D, 0x17E, 0x192, 0x192, 0x2C6, 0x2C6, 0x2DC, 0x2DC,
            0x2013, 0x2014, 0x2018, 0x201A, 0x201C, 0x201E, 0x2020, 0x2022,
            0x2026, 0x2026, 0x2030, 0x2030, 0x2039, 0x203A, 0x20AC, 0x20AC,
            0x2122, 0x2122, sal_uInt32(-1) };
        // <ftp://ftp.unicode.org/Public/MAPPINGS/VENDORS/MICSFT/WINDOWS/
        // CP1252.TXT> version 2.01 of 04/15/98
#endif // WNT

    INetMIMECharsetList_Impl * pList = new INetMIMECharsetList_Impl;
    switch (eEncoding)
    {
        case RTL_TEXTENCODING_MS_1252:
#if defined WNT
            pList->prepend(Charset(RTL_TEXTENCODING_MS_1252,
                                   aWindows1252Ranges));
#endif // WNT
        case RTL_TEXTENCODING_ISO_8859_1:
        case RTL_TEXTENCODING_UTF7:
        case RTL_TEXTENCODING_UTF8:
            break;

        case RTL_TEXTENCODING_ISO_8859_2:
            pList->prepend(Charset(RTL_TEXTENCODING_ISO_8859_2,
                                   aISO88592Ranges));
            break;

        case RTL_TEXTENCODING_ISO_8859_3:
            pList->prepend(Charset(RTL_TEXTENCODING_ISO_8859_3,
                                   aISO88593Ranges));
            break;

        case RTL_TEXTENCODING_ISO_8859_4:
            pList->prepend(Charset(RTL_TEXTENCODING_ISO_8859_4,
                                   aISO88594Ranges));
            break;

        case RTL_TEXTENCODING_ISO_8859_5:
            pList->prepend(Charset(RTL_TEXTENCODING_ISO_8859_5,
                                   aISO88595Ranges));
            break;

        case RTL_TEXTENCODING_ISO_8859_6:
            pList->prepend(Charset(RTL_TEXTENCODING_ISO_8859_6,
                                   aISO88596Ranges));
            break;

        case RTL_TEXTENCODING_ISO_8859_7:
            pList->prepend(Charset(RTL_TEXTENCODING_ISO_8859_7,
                                   aISO88597Ranges));
            break;

        case RTL_TEXTENCODING_ISO_8859_8:
            pList->prepend(Charset(RTL_TEXTENCODING_ISO_8859_8,
                                   aISO88598Ranges));
            break;

        case RTL_TEXTENCODING_ISO_8859_9:
            pList->prepend(Charset(RTL_TEXTENCODING_ISO_8859_9,
                                   aISO88599Ranges));
            break;

        case RTL_TEXTENCODING_ISO_8859_10:
            pList->prepend(Charset(RTL_TEXTENCODING_ISO_8859_10,
                                   aISO885910Ranges));
            break;

        case RTL_TEXTENCODING_ISO_8859_13:
            pList->prepend(Charset(RTL_TEXTENCODING_ISO_8859_13,
                                   aISO885913Ranges));
            break;

        case RTL_TEXTENCODING_ISO_8859_14:
            pList->prepend(Charset(RTL_TEXTENCODING_ISO_8859_14,
                                   aISO885914Ranges));
            break;

        case RTL_TEXTENCODING_ISO_8859_15:
            pList->prepend(Charset(RTL_TEXTENCODING_ISO_8859_15,
                                   aISO885915Ranges));
            break;

        case RTL_TEXTENCODING_MS_1250:
            pList->prepend(Charset(RTL_TEXTENCODING_ISO_8859_2,
                                   aISO88592Ranges));
            break;

        case RTL_TEXTENCODING_MS_1251:
            pList->prepend(Charset(RTL_TEXTENCODING_ISO_8859_5,
                                   aISO88595Ranges));
            break;

        case RTL_TEXTENCODING_MS_1253:
            pList->prepend(Charset(RTL_TEXTENCODING_ISO_8859_7,
                                   aISO88597Ranges));
            break;

        case RTL_TEXTENCODING_MS_1254:
            pList->prepend(Charset(RTL_TEXTENCODING_ISO_8859_9,
                                   aISO88599Ranges));
            break;

        case RTL_TEXTENCODING_MS_1255:
            pList->prepend(Charset(RTL_TEXTENCODING_ISO_8859_8,
                                   aISO88598Ranges));
            break;

        case RTL_TEXTENCODING_MS_1256:
            pList->prepend(Charset(RTL_TEXTENCODING_ISO_8859_6,
                                   aISO88596Ranges));
            break;

        case RTL_TEXTENCODING_MS_1257:
            pList->prepend(Charset(RTL_TEXTENCODING_ISO_8859_4,
                                   aISO88594Ranges));
            break;

        case RTL_TEXTENCODING_KOI8_R:
            pList->prepend(Charset(RTL_TEXTENCODING_ISO_8859_5,
                                   aISO88595Ranges));
            pList->prepend(Charset(RTL_TEXTENCODING_KOI8_R, aKOI8RRanges));
            break;

        default: //@@@ more cases are missing!
            OSL_FAIL("createPreferredCharsetList():"
                          " Unsupported encoding");
            break;
    }
    pList->prepend(Charset(RTL_TEXTENCODING_ISO_8859_1, aISO88591Ranges));
    pList->prepend(Charset(RTL_TEXTENCODING_ASCII_US, aUSASCIIRanges));
    return pList;
}

class INetMIMEEncodedWordOutputSink
{
private:
    enum { BUFFER_SIZE = 256 };

    enum Coding { CODING_NONE, CODING_ENCODED, CODING_ENCODED_TERMINATED };

    enum EncodedWordState { STATE_INITIAL, STATE_FIRST_EQUALS,
                            STATE_FIRST_QUESTION, STATE_CHARSET,
                            STATE_SECOND_QUESTION, STATE_ENCODING,
                            STATE_THIRD_QUESTION, STATE_ENCODED_TEXT,
                            STATE_FOURTH_QUESTION, STATE_SECOND_EQUALS,
                            STATE_BAD };

    INetMIMEOutputSink & m_rSink;
    sal_uInt32 m_nExtraSpaces;
    INetMIMECharsetList_Impl * m_pEncodingList;
    sal_Unicode * m_pBuffer;
    sal_uInt32 m_nBufferSize;
    sal_Unicode * m_pBufferEnd;
    Coding m_ePrevCoding;
    rtl_TextEncoding m_ePrevMIMEEncoding;
    Coding m_eCoding;
    EncodedWordState m_eEncodedWordState;

    void finish(bool bWriteTrailer);

public:
    inline INetMIMEEncodedWordOutputSink(INetMIMEOutputSink & rTheSink,
                                         rtl_TextEncoding ePreferredEncoding);

    ~INetMIMEEncodedWordOutputSink();

    INetMIMEEncodedWordOutputSink & WriteUInt32(sal_uInt32 nChar);

    inline void write(const sal_Unicode * pBegin, const sal_Unicode * pEnd);

    inline bool flush();
};

inline INetMIMEEncodedWordOutputSink::INetMIMEEncodedWordOutputSink(
           INetMIMEOutputSink & rTheSink, rtl_TextEncoding ePreferredEncoding):
    m_rSink(rTheSink),
    m_nExtraSpaces(0),
    m_pEncodingList(createPreferredCharsetList(ePreferredEncoding)),
    m_ePrevCoding(CODING_NONE),
    m_ePrevMIMEEncoding(RTL_TEXTENCODING_DONTKNOW),
    m_eCoding(CODING_NONE),
    m_eEncodedWordState(STATE_INITIAL)
{
    m_nBufferSize = BUFFER_SIZE;
    m_pBuffer = static_cast< sal_Unicode * >(rtl_allocateMemory(
                                                 m_nBufferSize
                                                     * sizeof (sal_Unicode)));
    m_pBufferEnd = m_pBuffer;
}


inline void INetMIMEEncodedWordOutputSink::write(const sal_Unicode * pBegin,
                                                 const sal_Unicode * pEnd)
{
    DBG_ASSERT(pBegin && pBegin <= pEnd,
               "INetMIMEEncodedWordOutputSink::write(): Bad sequence");

    while (pBegin != pEnd)
        WriteUInt32(*pBegin++);
}

inline bool INetMIMEEncodedWordOutputSink::flush()
{
    finish(true);
    return m_ePrevCoding != CODING_NONE;
}

static const bool aEscape[128]
    = { true,   // 0x00
        true,   // 0x01
        true,   // 0x02
        true,   // 0x03
        true,   // 0x04
        true,   // 0x05
        true,   // 0x06
        true,   // 0x07
        true,   // 0x08
        true,   // 0x09
        true,   // 0x0A
        true,   // 0x0B
        true,   // 0x0C
        true,   // 0x0D
        true,   // 0x0E
        true,   // 0x0F
        true,   // 0x10
        true,   // 0x11
        true,   // 0x12
        true,   // 0x13
        true,   // 0x14
        true,   // 0x15
        true,   // 0x16
        true,   // 0x17
        true,   // 0x18
        true,   // 0x19
        true,   // 0x1A
        true,   // 0x1B
        true,   // 0x1C
        true,   // 0x1D
        true,   // 0x1E
        true,   // 0x1F
        false,  // ' '
        false,  // '!'
        false,  // '"'
        false,  // '#'
        false,  // '$'
        false,  // '%'
        false,  // '&'
        false,  // '''
        false,  // '('
        false,  // ')'
        false,  // '*'
        false,  // '+'
        false,  // ','
        false,  // '-'
        false,  // '.'
        false,  // '/'
        false,  // '0'
        false,  // '1'
        false,  // '2'
        false,  // '3'
        false,  // '4'
        false,  // '5'
        false,  // '6'
        false,  // '7'
        false,  // '8'
        false,  // '9'
        false,  // ':'
        false,  // ';'
        false,  // '<'
        true,   // '='
        false,  // '>'
        true,   // '?'
        false,  // '@'
        false,  // 'A'
        false,  // 'B'
        false,  // 'C'
        false,  // 'D'
        false,  // 'E'
        false,  // 'F'
        false,  // 'G'
        false,  // 'H'
        false,  // 'I'
        false,  // 'J'
        false,  // 'K'
        false,  // 'L'
        false,  // 'M'
        false,  // 'N'
        false,  // 'O'
        false,  // 'P'
        false,  // 'Q'
        false,  // 'R'
        false,  // 'S'
        false,  // 'T'
        false,  // 'U'
        false,  // 'V'
        false,  // 'W'
        false,  // 'X'
        false,  // 'Y'
        false,  // 'Z'
        false,  // '['
        false,  // '\'
        false,  // ']'
        false,  // '^'
        true,   // '_'
        false,  // '`'
        false,  // 'a'
        false,  // 'b'
        false,  // 'c'
        false,  // 'd'
        false,  // 'e'
        false,  // 'f'
        false,  // 'g'
        false,  // 'h'
        false,  // 'i'
        false,  // 'j'
        false,  // 'k'
        false,  // 'l'
        false,  // 'm'
        false,  // 'n'
        false,  // 'o'
        false,  // 'p'
        false,  // 'q'
        false,  // 'r'
        false,  // 's'
        false,  // 't'
        false,  // 'u'
        false,  // 'v'
        false,  // 'w'
        false,  // 'x'
        false,  // 'y'
        false,  // 'z'
        false,  // '{'
        false,  // '|'
        false,  // '}'
        false,  // '~'
        true }; // DEL

inline bool
needsEncodedWordEscape(sal_uInt32 nChar)
{
    return !rtl::isAscii(nChar) || aEscape[nChar];
}

void INetMIMEEncodedWordOutputSink::finish(bool bWriteTrailer)
{
    if (m_eEncodedWordState == STATE_SECOND_EQUALS)
    {
        // If the text is already an encoded word, copy it verbatim:
        switch (m_ePrevCoding)
        {
            case CODING_NONE:
                while (m_nExtraSpaces-- > 0)
                {
                    m_rSink << ' ';
                }
                break;

            case CODING_ENCODED:
            {
                while (m_nExtraSpaces-- > 0)
                {
                    m_rSink << '_';
                }
                m_rSink << "?=";
                SAL_FALLTHROUGH;
            }
            case CODING_ENCODED_TERMINATED:
                m_rSink << ' ';
                break;
        }
        m_rSink.write(m_pBuffer, m_pBufferEnd);
        m_eCoding = CODING_ENCODED_TERMINATED;
    }
    else
    {
        switch (m_eCoding)
        {
            case CODING_NONE:
                switch (m_ePrevCoding)
                {
                    case CODING_ENCODED:
                        m_rSink << "?=";
                        break;

                    default:
                        break;
                }
                while (m_nExtraSpaces-- > 0)
                {
                    m_rSink << ' ';
                }
                m_rSink.write(m_pBuffer, m_pBufferEnd);
                break;

            case CODING_ENCODED:
            {
                rtl_TextEncoding eCharsetEncoding
                    = m_pEncodingList->
                          getPreferredEncoding(RTL_TEXTENCODING_UTF8);
                rtl_TextEncoding eMIMEEncoding
                    = translateToMIME(eCharsetEncoding);

                const sal_Char * pCharsetName
                    = getCharsetName(eMIMEEncoding);

                switch (m_ePrevCoding)
                {
                    case CODING_NONE:
                        while (m_nExtraSpaces-- > 0)
                        {
                            m_rSink << ' ';
                        }
                        m_rSink << "=?" << pCharsetName << "?Q?";
                        break;

                    case CODING_ENCODED:
                        if (m_ePrevMIMEEncoding != eMIMEEncoding)
                        {
                            m_rSink << "?= =?" << pCharsetName << "?Q?";
                        }
                        while (m_nExtraSpaces-- > 0)
                        {
                            m_rSink << '_';
                        }
                        break;

                    case CODING_ENCODED_TERMINATED:
                        m_rSink << " =?" << pCharsetName << "?Q?";
                        while (m_nExtraSpaces-- > 0)
                        {
                            m_rSink << '_';
                        }
                        break;
                }

                // The non UTF-8 code will only work for stateless single byte
                // character encodings:
                if (eMIMEEncoding == RTL_TEXTENCODING_UTF8)
                {
                    for (sal_Unicode const * p = m_pBuffer;
                         p != m_pBufferEnd;)
                    {
                        sal_uInt32 nUTF32
                            = INetMIME::getUTF32Character(p, m_pBufferEnd);
                        bool bEscape = needsEncodedWordEscape(nUTF32);
                        if (bEscape)
                        {
                            DBG_ASSERT(
                                nUTF32 < 0x10FFFF,
                                "INetMIMEEncodedWordOutputSink::finish():"
                                    " Bad char");
                            if (nUTF32 < 0x80)
                                writeEscapeSequence(m_rSink,
                                                              nUTF32);
                            else if (nUTF32 < 0x800)
                            {
                                writeEscapeSequence(m_rSink,
                                                              (nUTF32 >> 6)
                                                                  | 0xC0);
                                writeEscapeSequence(m_rSink,
                                                              (nUTF32 & 0x3F)
                                                                  | 0x80);
                            }
                            else if (nUTF32 < 0x10000)
                            {
                                writeEscapeSequence(m_rSink,
                                                              (nUTF32 >> 12)
                                                                  | 0xE0);
                                writeEscapeSequence(m_rSink,
                                                              ((nUTF32 >> 6)
                                                                      & 0x3F)
                                                                  | 0x80);
                                writeEscapeSequence(m_rSink,
                                                              (nUTF32 & 0x3F)
                                                                  | 0x80);
                            }
                            else
                            {
                                writeEscapeSequence(m_rSink,
                                                              (nUTF32 >> 18)
                                                                  | 0xF0);
                                writeEscapeSequence(m_rSink,
                                                              ((nUTF32 >> 12)
                                                                      & 0x3F)
                                                                  | 0x80);
                                writeEscapeSequence(m_rSink,
                                                              ((nUTF32 >> 6)
                                                                      & 0x3F)
                                                                  | 0x80);
                                writeEscapeSequence(m_rSink,
                                                              (nUTF32 & 0x3F)
                                                                  | 0x80);
                            }
                        }
                        else
                            m_rSink << sal_Char(nUTF32);
                    }
                }
                else
                {
                    sal_Char * pTargetBuffer = nullptr;
                    sal_Size nTargetSize = 0;
                    rtl_UnicodeToTextConverter hConverter
                        = rtl_createUnicodeToTextConverter(eCharsetEncoding);
                    rtl_UnicodeToTextContext hContext
                        = rtl_createUnicodeToTextContext(hConverter);
                    for (sal_Size nBufferSize = m_pBufferEnd - m_pBuffer;;
                         nBufferSize += nBufferSize / 3 + 1)
                    {
                        pTargetBuffer = new sal_Char[nBufferSize];
                        sal_uInt32 nInfo;
                        sal_Size nSrcCvtBytes;
                        nTargetSize
                            = rtl_convertUnicodeToText(
                                  hConverter, hContext, m_pBuffer,
                                  m_pBufferEnd - m_pBuffer, pTargetBuffer,
                                  nBufferSize,
                                  RTL_UNICODETOTEXT_FLAGS_UNDEFINED_IGNORE
                                     | RTL_UNICODETOTEXT_FLAGS_INVALID_IGNORE,
                                  &nInfo, &nSrcCvtBytes);
                        if (!(nInfo
                                  & RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL))
                            break;
                        delete[] pTargetBuffer;
                        pTargetBuffer = nullptr;
                        rtl_resetUnicodeToTextContext(hConverter, hContext);
                    }
                    rtl_destroyUnicodeToTextContext(hConverter, hContext);
                    rtl_destroyUnicodeToTextConverter(hConverter);
                    for (sal_Size k = 0; k < nTargetSize; ++k)
                    {
                        sal_uInt32 nUCS4 = static_cast<unsigned char>(pTargetBuffer[k]);
                        bool bEscape = needsEncodedWordEscape(nUCS4);
                        if (bEscape)
                            writeEscapeSequence(m_rSink, nUCS4);
                        else
                            m_rSink << sal_Char(nUCS4);
                    }
                    delete[] pTargetBuffer;
                }

                if (bWriteTrailer)
                {
                    m_rSink << "?=";
                    m_eCoding = CODING_ENCODED_TERMINATED;
                }

                m_ePrevMIMEEncoding = eMIMEEncoding;
                break;
            }

            default:
                OSL_ASSERT(false);
                break;
        }
    }

    m_nExtraSpaces = 0;
    m_pEncodingList->reset();
    m_pBufferEnd = m_pBuffer;
    m_ePrevCoding = m_eCoding;
    m_eCoding = CODING_NONE;
    m_eEncodedWordState = STATE_INITIAL;
}

INetMIMEEncodedWordOutputSink::~INetMIMEEncodedWordOutputSink()
{
    rtl_freeMemory(m_pBuffer);
    delete m_pEncodingList;
}

INetMIMEEncodedWordOutputSink &
INetMIMEEncodedWordOutputSink::WriteUInt32(sal_uInt32 nChar)
{
    if (nChar == ' ')
    {
        if (m_pBufferEnd != m_pBuffer)
            finish(false);
        ++m_nExtraSpaces;
    }
    else
    {
        // Check for an already encoded word:
        switch (m_eEncodedWordState)
        {
            case STATE_INITIAL:
                if (nChar == '=')
                    m_eEncodedWordState = STATE_FIRST_EQUALS;
                else
                    m_eEncodedWordState = STATE_BAD;
                break;

            case STATE_FIRST_EQUALS:
                if (nChar == '?')
                    m_eEncodedWordState = STATE_FIRST_EQUALS;
                else
                    m_eEncodedWordState = STATE_BAD;
                break;

            case STATE_FIRST_QUESTION:
                if (isEncodedWordTokenChar(nChar))
                    m_eEncodedWordState = STATE_CHARSET;
                else
                    m_eEncodedWordState = STATE_BAD;
                break;

            case STATE_CHARSET:
                if (nChar == '?')
                    m_eEncodedWordState = STATE_SECOND_QUESTION;
                else if (!isEncodedWordTokenChar(nChar))
                    m_eEncodedWordState = STATE_BAD;
                break;

            case STATE_SECOND_QUESTION:
                if (nChar == 'B' || nChar == 'Q'
                    || nChar == 'b' || nChar == 'q')
                    m_eEncodedWordState = STATE_ENCODING;
                else
                    m_eEncodedWordState = STATE_BAD;
                break;

            case STATE_ENCODING:
                if (nChar == '?')
                    m_eEncodedWordState = STATE_THIRD_QUESTION;
                else
                    m_eEncodedWordState = STATE_BAD;
                break;

            case STATE_THIRD_QUESTION:
                if (INetMIME::isVisible(nChar) && nChar != '?')
                    m_eEncodedWordState = STATE_ENCODED_TEXT;
                else
                    m_eEncodedWordState = STATE_BAD;
                break;

            case STATE_ENCODED_TEXT:
                if (nChar == '?')
                    m_eEncodedWordState = STATE_FOURTH_QUESTION;
                else if (!INetMIME::isVisible(nChar))
                    m_eEncodedWordState = STATE_BAD;
                break;

            case STATE_FOURTH_QUESTION:
                if (nChar == '=')
                    m_eEncodedWordState = STATE_SECOND_EQUALS;
                else
                    m_eEncodedWordState = STATE_BAD;
                break;

            case STATE_SECOND_EQUALS:
                m_eEncodedWordState = STATE_BAD;
                break;

            case STATE_BAD:
                break;
        }

        // Update encoding:
        m_pEncodingList->includes(nChar);

        // Update coding:
        static const bool aMinimal[128]
            = { true,   // 0x00
                true,   // 0x01
                true,   // 0x02
                true,   // 0x03
                true,   // 0x04
                true,   // 0x05
                true,   // 0x06
                true,   // 0x07
                true,   // 0x08
                true,   // 0x09
                true,   // 0x0A
                true,   // 0x0B
                true,   // 0x0C
                true,   // 0x0D
                true,   // 0x0E
                true,   // 0x0F
                true,   // 0x10
                true,   // 0x11
                true,   // 0x12
                true,   // 0x13
                true,   // 0x14
                true,   // 0x15
                true,   // 0x16
                true,   // 0x17
                true,   // 0x18
                true,   // 0x19
                true,   // 0x1A
                true,   // 0x1B
                true,   // 0x1C
                true,   // 0x1D
                true,   // 0x1E
                true,   // 0x1F
                false,  // ' '
                false,  // '!'
                false,  // '"'
                false,  // '#'
                false,  // '$'
                false,  // '%'
                false,  // '&'
                false,  // '''
                false,  // '('
                false,  // ')'
                false,  // '*'
                false,  // '+'
                false,  // ','
                false,  // '-'
                false,  // '.'
                false,  // '/'
                false,  // '0'
                false,  // '1'
                false,  // '2'
                false,  // '3'
                false,  // '4'
                false,  // '5'
                false,  // '6'
                false,  // '7'
                false,  // '8'
                false,  // '9'
                false,  // ':'
                false,  // ';'
                false,  // '<'
                false,  // '='
                false,  // '>'
                false,  // '?'
                false,  // '@'
                false,  // 'A'
                false,  // 'B'
                false,  // 'C'
                false,  // 'D'
                false,  // 'E'
                false,  // 'F'
                false,  // 'G'
                false,  // 'H'
                false,  // 'I'
                false,  // 'J'
                false,  // 'K'
                false,  // 'L'
                false,  // 'M'
                false,  // 'N'
                false,  // 'O'
                false,  // 'P'
                false,  // 'Q'
                false,  // 'R'
                false,  // 'S'
                false,  // 'T'
                false,  // 'U'
                false,  // 'V'
                false,  // 'W'
                false,  // 'X'
                false,  // 'Y'
                false,  // 'Z'
                false,  // '['
                false,  // '\'
                false,  // ']'
                false,  // '^'
                false,  // '_'
                false,  // '`'
                false,  // 'a'
                false,  // 'b'
                false,  // 'c'
                false,  // 'd'
                false,  // 'e'
                false,  // 'f'
                false,  // 'g'
                false,  // 'h'
                false,  // 'i'
                false,  // 'j'
                false,  // 'k'
                false,  // 'l'
                false,  // 'm'
                false,  // 'n'
                false,  // 'o'
                false,  // 'p'
                false,  // 'q'
                false,  // 'r'
                false,  // 's'
                false,  // 't'
                false,  // 'u'
                false,  // 'v'
                false,  // 'w'
                false,  // 'x'
                false,  // 'y'
                false,  // 'z'
                false,  // '{'
                false,  // '|'
                false,  // '}'
                false,  // '~'
                true }; // DEL
        Coding eNewCoding = !rtl::isAscii(nChar) ? CODING_ENCODED :
                            aMinimal[nChar] ? CODING_ENCODED : CODING_NONE;
        if (eNewCoding > m_eCoding)
            m_eCoding = eNewCoding;

        // Append to buffer:
        if (sal_uInt32(m_pBufferEnd - m_pBuffer) == m_nBufferSize)
        {
            m_pBuffer
                = static_cast< sal_Unicode * >(
                      rtl_reallocateMemory(m_pBuffer,
                                           (m_nBufferSize + BUFFER_SIZE)
                                               * sizeof (sal_Unicode)));
            m_pBufferEnd = m_pBuffer + m_nBufferSize;
            m_nBufferSize += BUFFER_SIZE;
        }
        *m_pBufferEnd++ = sal_Unicode(nChar);
    }
    return *this;
}

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

bool isEncodedWordTokenChar(sal_uInt32 nChar)
{
    static const bool aMap[128]
        = { false, false, false, false, false, false, false, false,
            false, false, false, false, false, false, false, false,
            false, false, false, false, false, false, false, false,
            false, false, false, false, false, false, false, false,
            false,  true, false,  true,  true,  true,  true,  true, // !"#$%&'
            false, false,  true,  true, false,  true, false, false, //()*+,-./
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
        OString aAttribute = OString(
            pAttributeBegin, p - pAttributeBegin,
            RTL_TEXTENCODING_ASCII_US);
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

        bool bPresent;
        Parameter ** pPos = aList.find(aAttribute, nSection, bPresent);
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
                INetMIMEOutputSink aSink;
                while (p != pEnd)
                {
                    sal_uInt32 nChar = INetMIME::getUTF32Character(p, pEnd);
                    if (rtl::isAscii(nChar) && !isTokenChar(nChar))
                        break;
                    if (nChar == '%' && p + 1 < pEnd)
                    {
                        int nWeight1 = INetMIME::getHexWeight(p[0]);
                        int nWeight2 = INetMIME::getHexWeight(p[1]);
                        if (nWeight1 >= 0 && nWeight2 >= 0)
                        {
                            aSink << sal_Char(nWeight1 << 4 | nWeight2);
                            p += 2;
                            continue;
                        }
                    }
                    writeUTF8(aSink, nChar);
                }
                aValue = aSink.takeBuffer();
            }
            else
                while (p != pEnd && (isTokenChar(*p) || !rtl::isAscii(*p)))
                    ++p;
        }
        else if (p != pEnd && *p == '"')
            if (pParameters)
            {
                INetMIMEOutputSink aSink;
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
                aValue = aSink.takeBuffer();
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

        *pPos = new Parameter(*pPos, aAttribute, aCharset, aLanguage, aValue,
                              nSection, bExtended);
    }
    return parseParameters(aList, pParameters) ? pParameterBegin : pBegin;
}

const sal_Char * getCharsetName(rtl_TextEncoding eEncoding)
{
    if (rtl_isOctetTextEncoding(eEncoding))
    {
        char const * p = rtl_getMimeCharsetFromTextEncoding(eEncoding);
        DBG_ASSERT(p, "getCharsetName(): Unsupported encoding");
        return p;
    }
    else
        switch (eEncoding)
        {
            case RTL_TEXTENCODING_UCS4:
                return "ISO-10646-UCS-4";

            case RTL_TEXTENCODING_UCS2:
                return "ISO-10646-UCS-2";

            default:
                OSL_FAIL("getCharsetName(): Unsupported encoding");
                return nullptr;
        }
}

bool equalIgnoreCase(const sal_Char * pBegin1,
                               const sal_Char * pEnd1,
                               const sal_Char * pString2)
{
    DBG_ASSERT(pBegin1 && pBegin1 <= pEnd1 && pString2,
               "equalIgnoreCase(): Bad sequences");

    while (*pString2 != 0)
        if (pBegin1 == pEnd1
            || rtl::toAsciiUpperCase(*pBegin1++) != rtl::toAsciiUpperCase(*pString2++))
            return false;
    return pBegin1 == pEnd1;
}

struct EncodingEntry
{
    sal_Char const * m_aName;
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

rtl_TextEncoding getCharsetEncoding(sal_Char const * pBegin,
                                              sal_Char const * pEnd)
{
    for (sal_Size i = 0; i < sizeof aEncodingMap / sizeof (EncodingEntry);
         ++i)
        if (equalIgnoreCase(pBegin, pEnd, aEncodingMap[i].m_aName))
            return aEncodingMap[i].m_eEncoding;
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
sal_uInt32 INetMIME::getHexDigit(int nWeight)
{
    DBG_ASSERT(nWeight >= 0 && nWeight < 16,
               "INetMIME::getHexDigit(): Bad weight");

    static const sal_Char aDigits[16]
        = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C',
            'D', 'E', 'F' };
    return aDigits[nWeight];
}

// static
bool INetMIME::equalIgnoreCase(const sal_Unicode * pBegin1,
                               const sal_Unicode * pEnd1,
                               const sal_Char * pString2)
{
    DBG_ASSERT(pBegin1 && pBegin1 <= pEnd1 && pString2,
               "INetMIME::equalIgnoreCase(): Bad sequences");

    while (*pString2 != 0)
        if (pBegin1 == pEnd1
            || rtl::toAsciiUpperCase(*pBegin1++) != rtl::toAsciiUpperCase(*pString2++))
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
    sal_Unicode const * pBegin, sal_Unicode const * pEnd, OUString * pType,
    OUString * pSubType, INetContentTypeParameterList * pParameters)
{
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
void INetMIME::writeHeaderFieldBody(INetMIMEOutputSink & rSink,
                                    const OUString& rBody,
                                    rtl_TextEncoding ePreferredEncoding)
{
    INetMIMEEncodedWordOutputSink aOutput(rSink, ePreferredEncoding);
    aOutput.write(rBody.getStr(), rBody.getStr() + rBody.getLength());
    aOutput.flush();
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

    const sal_Char * pBegin = rBody.getStr();
    const sal_Char * pEnd = pBegin + rBody.getLength();

    OUString sDecoded;
    const sal_Char * pCopyBegin = pBegin;

    /* bool bStartEncodedWord = true; */
    const sal_Char * pWSPBegin = pBegin;

    for (const sal_Char * p = pBegin; p != pEnd;)
    {
        OUString sEncodedText;
        if (p != pEnd && *p == '=' /* && bStartEncodedWord */)
        {
            const sal_Char * q = p + 1;
            bool bEncodedWord = q != pEnd && *q++ == '?';

            rtl_TextEncoding eCharsetEncoding = RTL_TEXTENCODING_DONTKNOW;
            if (bEncodedWord)
            {
                const sal_Char * pCharsetBegin = q;
                const sal_Char * pLanguageBegin = nullptr;
                int nAlphaCount = 0;
                for (bool bDone = false; !bDone;)
                    if (q == pEnd)
                    {
                        bEncodedWord = false;
                        bDone = true;
                    }
                    else
                    {
                        sal_Char cChar = *q++;
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
                                    && (!rtl::isAsciiAlpha(cChar) || ++nAlphaCount > 8))
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
                                    sText.append(sal_Char(nValue >> nShift & 0xFF));
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
                    const sal_Char * pEncodedTextBegin = q;
                    const sal_Char * pEncodedTextCopyBegin = q;
                    for (bool bDone = false; !bDone;)
                        if (q == pEnd)
                        {
                            bEncodedWord = false;
                            bDone = true;
                        }
                        else
                        {
                            sal_uInt32 nChar = *q++;
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
                                    sText.append(sal_Char(nDigit1 << 4 | nDigit2));
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

            sal_Unicode * pUnicodeBuffer = nullptr;
            sal_Size nUnicodeSize = 0;
            if (bEncodedWord)
            {
                pUnicodeBuffer
                    = convertToUnicode(sText.getStr(),
                                       sText.getStr() + sText.getLength(),
                                       eCharsetEncoding, nUnicodeSize);
                if (pUnicodeBuffer == nullptr)
                    bEncodedWord = false;
            }

            if (bEncodedWord)
            {
                appendISO88591(sDecoded, pCopyBegin, pWSPBegin);
                sDecoded += OUString(
                    pUnicodeBuffer,
                    static_cast< sal_Int32 >(nUnicodeSize));
                delete[] pUnicodeBuffer;
                p = q;
                pCopyBegin = p;

                pWSPBegin = p;
                while (p != pEnd && isWhiteSpace(*p))
                    ++p;
                /* bStartEncodedWord = p != pWSPBegin; */
                continue;
            }
        }

        if (!sEncodedText.isEmpty())
            sDecoded += sEncodedText;

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
                const sal_Char * pUTF8Begin = p - 1;
                const sal_Char * pUTF8End = pUTF8Begin;
                sal_uInt32 nCharacter = 0;
                if (translateUTF8Char(pUTF8End, pEnd, RTL_TEXTENCODING_UCS4,
                                      nCharacter))
                {
                    appendISO88591(sDecoded, pCopyBegin, p - 1);
                    sal_Unicode aUTF16Buf[2];
                    sal_Int32 nUTF16Len = putUTF32Character(aUTF16Buf, nCharacter) - aUTF16Buf;
                    sDecoded += OUString(aUTF16Buf, nUTF16Len);
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
    return sDecoded;
}

void INetMIMEOutputSink::writeSequence(const sal_Char * pBegin,
                                       const sal_Char * pEnd)
{
    OSL_ENSURE(pBegin && pBegin <= pEnd,
               "INetMIMEOutputSink::writeSequence(): Bad sequence");

    m_aBuffer.append(pBegin, pEnd - pBegin);
}

sal_Size INetMIMEOutputSink::writeSequence(const sal_Char * pSequence)
{
    sal_Size nLength = rtl_str_getLength(pSequence);
    writeSequence(pSequence, pSequence + nLength);
    return nLength;
}

void INetMIMEOutputSink::writeSequence(const sal_Unicode * pBegin,
                                       const sal_Unicode * pEnd)
{
    assert(pBegin && pBegin <= pEnd &&
               "INetMIMEOutputSink::writeSequence(): Bad sequence");

    sal_Char * pBufferBegin = new sal_Char[pEnd - pBegin];
    sal_Char * pBufferEnd = pBufferBegin;
    while (pBegin != pEnd)
    {
        DBG_ASSERT(*pBegin < 256,
                   "INetMIMEOutputSink::writeSequence(): Bad octet");
        *pBufferEnd++ = sal_Char(*pBegin++);
    }
    writeSequence(pBufferBegin, pBufferEnd);
    delete[] pBufferBegin;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
