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

#include <rtl/strbuf.hxx>
#include <rtl/tencinfo.h>
#include <tools/inetmime.hxx>

namespace unnamed_tools_inetmime {} using namespace unnamed_tools_inetmime;
    // unnamed namespaces don't work well yet

namespace unnamed_tools_inetmime {

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

void appendISO88591(UniString & rText, sal_Char const * pBegin,
                    sal_Char const * pEnd);

}

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
    INetMIMECharsetList_Impl(): m_pFirst(0) {}

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

namespace unnamed_tools_inetmime {

struct Parameter
{
    Parameter * m_pNext;
    rtl::OString m_aAttribute;
    rtl::OString m_aCharset;
    rtl::OString m_aLanguage;
    rtl::OString m_aValue;
    sal_uInt32 m_nSection;
    bool m_bExtended;

    inline Parameter(Parameter * pTheNext, const rtl::OString& rTheAttribute,
                     const rtl::OString& rTheCharset,
                     const rtl::OString& rTheLanguage,
                     const rtl::OString& rTheValue, sal_uInt32 nTheSection,
                     bool bTheExtended);
};

inline Parameter::Parameter(Parameter * pTheNext,
                            const rtl::OString& rTheAttribute,
                            const rtl::OString& rTheCharset,
                            const rtl::OString& rTheLanguage,
                            const rtl::OString& rTheValue,
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

    ParameterList(): m_pList(0) {}

    inline ~ParameterList();

    Parameter ** find(const rtl::OString& rAttribute, sal_uInt32 nSection,
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

}

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

namespace unnamed_tools_inetmime {

void appendISO88591(UniString & rText, sal_Char const * pBegin,
                    sal_Char const * pEnd)
{
    xub_StrLen nLength = static_cast< xub_StrLen >(pEnd - pBegin);
    sal_Unicode * pBuffer = new sal_Unicode[nLength];
    for (sal_Unicode * p = pBuffer; pBegin != pEnd;)
        *p++ = sal_uChar(*pBegin++);
    rText.Append(pBuffer, nLength);
    delete[] pBuffer;
}

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

rtl_TextEncoding
INetMIMECharsetList_Impl::getPreferredEncoding(rtl_TextEncoding eDefault)
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

Parameter ** ParameterList::find(const rtl::OString& rAttribute,
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

namespace unnamed_tools_inetmime {

bool parseParameters(ParameterList const & rInput,
                     INetContentTypeParameterList * pOutput)
{
    if (pOutput)
        pOutput->Clear();

    Parameter * pPrev = 0;
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
                    = INetMIME::getCharsetEncoding(p->m_aCharset.getStr(),
                                                   p->m_aCharset.getStr()
                                                       + rInput.m_pList->
                                                             m_aCharset.
                                                                 getLength());
            UniString aValue;
            bool bBadEncoding = false;
            Parameter * pNext = p;
            do
            {
                sal_Size nSize;
                sal_Unicode * pUnicode
                    = INetMIME::convertToUnicode(pNext->m_aValue.getStr(),
                                                 pNext->m_aValue.getStr()
                                                     + pNext->m_aValue.getLength(),
                                                 bCharset && p->m_bExtended ?
                                                     eEncoding :
                                                     RTL_TEXTENCODING_UTF8,
                                                 nSize);
                if (!pUnicode && !(bCharset && p->m_bExtended))
                    pUnicode = INetMIME::convertToUnicode(
                                   pNext->m_aValue.getStr(),
                                   pNext->m_aValue.getStr()
                                       + pNext->m_aValue.getLength(),
                                   RTL_TEXTENCODING_ISO_8859_1, nSize);
                if (!pUnicode)
                {
                    bBadEncoding = true;
                    break;
                }
                aValue += rtl::OUString(pUnicode, static_cast<sal_Int32>(nSize));
                delete[] pUnicode;
                pNext = pNext->m_pNext;
            }
            while (pNext && pNext->m_nSection > 0);
            if (bBadEncoding)
            {
                aValue.Erase();
                for (pNext = p;;)
                {
                    if (pNext->m_bExtended)
                    {
                        for (sal_Int32 i = 0; i < pNext->m_aValue.getLength(); ++i)
                            aValue += sal_Unicode(
                                sal_Unicode(
                                    sal_uChar(pNext->m_aValue[i]))
                                | 0xF800);
                    }
                    else
                    {
                        for (sal_Int32 i = 0; i < pNext->m_aValue.getLength(); ++i)
                            aValue += sal_Unicode(sal_uChar(pNext->m_aValue[i]));
                    }
                    pNext = pNext->m_pNext;
                    if (!pNext || pNext->m_nSection == 0)
                        break;
                };
            }
            pOutput->Append(new INetContentTypeParameter(p->m_aAttribute,
                                                             p->m_aCharset,
                                                             p->m_aLanguage,
                                                             aValue,
                                                             !bBadEncoding));
            p = pNext;
        }
    return true;
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
    return isUSASCII(nChar) && aMap[nChar];
}

// static
bool INetMIME::isTokenChar(sal_uInt32 nChar)
{
    static const sal_Char aMap[128]
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
    return isUSASCII(nChar) && aMap[nChar];
}

// static
bool INetMIME::isEncodedWordTokenChar(sal_uInt32 nChar)
{
    static const sal_Char aMap[128]
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
    return isUSASCII(nChar) && aMap[nChar];
}

// static
bool INetMIME::isIMAPAtomChar(sal_uInt32 nChar)
{
    static const sal_Char aMap[128]
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
    return isUSASCII(nChar) && aMap[nChar];
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
bool INetMIME::equalIgnoreCase(const sal_Char * pBegin1,
                               const sal_Char * pEnd1,
                               const sal_Char * pString2)
{
    DBG_ASSERT(pBegin1 && pBegin1 <= pEnd1 && pString2,
               "INetMIME::equalIgnoreCase(): Bad sequences");

    while (*pString2 != 0)
        if (pBegin1 == pEnd1
            || toUpperCase(*pBegin1++) != toUpperCase(*pString2++))
            return false;
    return pBegin1 == pEnd1;
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
            || toUpperCase(*pBegin1++) != toUpperCase(*pString2++))
            return false;
    return pBegin1 == pEnd1;
}

// static
const sal_Unicode * INetMIME::skipLinearWhiteSpace(const sal_Unicode * pBegin,
                                                   const sal_Unicode * pEnd)
{
    DBG_ASSERT(pBegin && pBegin <= pEnd,
               "INetMIME::skipLinearWhiteSpace(): Bad sequence");

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

            default:
                return pBegin;
        }
    return pBegin;
}

// static
const sal_Char * INetMIME::skipComment(const sal_Char * pBegin,
                                       const sal_Char * pEnd)
{
    DBG_ASSERT(pBegin && pBegin <= pEnd,
               "INetMIME::skipComment(): Bad sequence");

    if (pBegin != pEnd && *pBegin == '(')
    {
        sal_uInt32 nLevel = 0;
        for (const sal_Char * p = pBegin; p != pEnd;)
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

// static
const sal_Unicode * INetMIME::skipComment(const sal_Unicode * pBegin,
                                          const sal_Unicode * pEnd)
{
    DBG_ASSERT(pBegin && pBegin <= pEnd,
               "INetMIME::skipComment(): Bad sequence");

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

// static
const sal_Char * INetMIME::skipLinearWhiteSpaceComment(const sal_Char *
                                                           pBegin,
                                                       const sal_Char * pEnd)
{
    DBG_ASSERT(pBegin && pBegin <= pEnd,
               "INetMIME::skipLinearWhiteSpaceComment(): Bad sequence");

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
                const sal_Char * p = skipComment(pBegin, pEnd);
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

// static
const sal_Unicode * INetMIME::skipLinearWhiteSpaceComment(const sal_Unicode *
                                                              pBegin,
                                                          const sal_Unicode *
                                                              pEnd)
{
    DBG_ASSERT(pBegin && pBegin <= pEnd,
               "INetMIME::skipLinearWhiteSpaceComment(): Bad sequence");

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

// static
const sal_Char * INetMIME::skipQuotedString(const sal_Char * pBegin,
                                            const sal_Char * pEnd)
{
    DBG_ASSERT(pBegin && pBegin <= pEnd,
               "INetMIME::skipQuotedString(): Bad sequence");

    if (pBegin != pEnd && *pBegin == '"')
        for (const sal_Char * p = pBegin + 1; p != pEnd;)
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

// static
const sal_Unicode * INetMIME::skipQuotedString(const sal_Unicode * pBegin,
                                               const sal_Unicode * pEnd)
{
    DBG_ASSERT(pBegin && pBegin <= pEnd,
               "INetMIME::skipQuotedString(): Bad sequence");

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

// static
bool INetMIME::scanUnsigned(const sal_Char *& rBegin, const sal_Char * pEnd,
                            bool bLeadingZeroes, sal_uInt32 & rValue)
{
    sal_uInt64 nTheValue = 0;
    const sal_Char * p = rBegin;
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
const sal_Unicode * INetMIME::scanQuotedBlock(const sal_Unicode * pBegin,
                                              const sal_Unicode * pEnd,
                                              sal_uInt32 nOpening,
                                              sal_uInt32 nClosing,
                                              sal_Size & rLength,
                                              bool & rModify)
{
    DBG_ASSERT(pBegin && pBegin <= pEnd,
               "INetMIME::scanQuotedBlock(): Bad sequence");

    if (pBegin != pEnd && *pBegin == nOpening)
    {
        ++rLength;
        ++pBegin;
        while (pBegin != pEnd)
            if (*pBegin == nClosing)
            {
                ++rLength;
                return ++pBegin;
            }
            else
            {
                sal_uInt32 c = *pBegin++;
                switch (c)
                {
                    case 0x0D: // CR
                        if (pBegin != pEnd && *pBegin == 0x0A) // LF
                            if (pEnd - pBegin >= 2 && isWhiteSpace(pBegin[1]))
                            {
                                ++rLength;
                                rModify = true;
                                pBegin += 2;
                            }
                            else
                            {
                                rLength += 3;
                                rModify = true;
                                ++pBegin;
                            }
                        else
                            ++rLength;
                        break;

                    case '\\':
                        ++rLength;
                        if (pBegin != pEnd)
                        {
                            if (startsWithLineBreak(pBegin, pEnd)
                                && (pEnd - pBegin < 3
                                    || !isWhiteSpace(pBegin[2])))
                            {
                                rLength += 3;
                                rModify = true;
                                pBegin += 2;
                            }
                            else
                                ++pBegin;
                        }
                        break;

                    default:
                        ++rLength;
                        if (!isUSASCII(c))
                            rModify = true;
                        break;
                }
            }
    }
    return pBegin;
}

// static
sal_Char const * INetMIME::scanParameters(sal_Char const * pBegin,
                                          sal_Char const * pEnd,
                                          INetContentTypeParameterList *
                                              pParameters)
{
    ParameterList aList;
    sal_Char const * pParameterBegin = pBegin;
    for (sal_Char const * p = pParameterBegin;; pParameterBegin = p)
    {
        pParameterBegin = skipLinearWhiteSpaceComment(p, pEnd);
        if (pParameterBegin == pEnd || *pParameterBegin != ';')
            break;
        p = pParameterBegin + 1;

        sal_Char const * pAttributeBegin = skipLinearWhiteSpaceComment(p,
                                                                       pEnd);
        p = pAttributeBegin;
        bool bDowncaseAttribute = false;
        while (p != pEnd && isTokenChar(*p) && *p != '*')
        {
            bDowncaseAttribute = bDowncaseAttribute || isUpperCase(*p);
            ++p;
        }
        if (p == pAttributeBegin)
            break;
        rtl::OString aAttribute(pAttributeBegin, (p - pAttributeBegin));
        if (bDowncaseAttribute)
            aAttribute = aAttribute.toAsciiLowerCase();

        sal_uInt32 nSection = 0;
        if (p != pEnd && *p == '*')
        {
            ++p;
            if (p != pEnd && isDigit(*p)
                && !scanUnsigned(p, pEnd, false, nSection))
                break;
        }

        bool bPresent;
        Parameter ** pPos = aList.find(aAttribute, nSection, bPresent);

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

        rtl::OString aCharset;
        rtl::OString aLanguage;
        rtl::OStringBuffer aValue;
        if (bExtended)
        {
            if (nSection == 0)
            {
                sal_Char const * pCharsetBegin = p;
                bool bDowncaseCharset = false;
                while (p != pEnd && isTokenChar(*p) && *p != '\'')
                {
                    bDowncaseCharset = bDowncaseCharset || isUpperCase(*p);
                    ++p;
                }
                if (p == pCharsetBegin)
                    break;
                if (pParameters)
                {
                    aCharset = rtl::OString(
                        pCharsetBegin, p - pCharsetBegin);
                    if (bDowncaseCharset)
                        aCharset = aCharset.toAsciiLowerCase();
                }

                if (p == pEnd || *p != '\'')
                    break;
                ++p;

                sal_Char const * pLanguageBegin = p;
                bool bDowncaseLanguage = false;
                int nLetters = 0;
                for (; p != pEnd; ++p)
                    if (isAlpha(*p))
                    {
                        if (++nLetters > 8)
                            break;
                        bDowncaseLanguage = bDowncaseLanguage
                                            || isUpperCase(*p);
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
                    aLanguage = rtl::OString(
                        pLanguageBegin, p - pLanguageBegin);
                    if (bDowncaseLanguage)
                        aLanguage = aLanguage.toAsciiLowerCase();
                }

                if (p == pEnd || *p != '\'')
                    break;
                ++p;
            }
            if (pParameters)
            {
                while (p != pEnd && (isTokenChar(*p) || !isUSASCII(*p)))
                {
                    if (*p == '%')
                    {
                        if (p + 2 < pEnd)
                        {
                            int nWeight1 = getHexWeight(p[1]);
                            int nWeight2 = getHexWeight(p[2]);
                            if (nWeight1 >= 0 && nWeight2 >= 0)
                            {
                                aValue.append(sal_Char(nWeight1 << 4 | nWeight2));
                                p += 3;
                                continue;
                            }
                        }
                    }
                    aValue.append(*p++);
                }
            }
            else
                while (p != pEnd && (isTokenChar(*p) || !isUSASCII(*p)))
                    ++p;
        }
        else if (p != pEnd && *p == '"')
        {
            if (pParameters)
            {
                bool bInvalid = false;
                for (++p;;)
                {
                    if (p == pEnd)
                    {
                        bInvalid = true;
                        break;
                    }
                    else if (*p == '"')
                    {
                        ++p;
                        break;
                    }
                    else if (*p == 0x0D) // CR
                    {
                        if (pEnd - p < 3 || p[1] != 0x0A // LF
                            || !isWhiteSpace(p[2]))
                        {
                            bInvalid = true;
                            break;
                        }
                        p += 2;
                    }
                    else if (*p == '\\' && ++p == pEnd)
                    {
                        bInvalid = true;
                        break;
                    }
                    aValue.append(*p++);
                }
                if (bInvalid)
                    break;
            }
            else
            {
                sal_Char const * pStringEnd = skipQuotedString(p, pEnd);
                if (p == pStringEnd)
                    break;
                p = pStringEnd;
            }
        }
        else
        {
            sal_Char const * pTokenBegin = p;
            while (p != pEnd && (isTokenChar(*p) || !isUSASCII(*p)))
                ++p;
            if (p == pTokenBegin)
                break;
            if (pParameters)
                aValue.append(pTokenBegin, static_cast< sal_Int32 >(p - pTokenBegin));
        }

        if (!bPresent)
            *pPos = new Parameter(*pPos, aAttribute, aCharset, aLanguage,
                aValue.makeStringAndClear(), nSection, bExtended);
    }
    return parseParameters(aList, pParameters) ? pParameterBegin : pBegin;
}

// static
sal_Unicode const * INetMIME::scanParameters(sal_Unicode const * pBegin,
                                             sal_Unicode const * pEnd,
                                             INetContentTypeParameterList *
                                                 pParameters)
{
    ParameterList aList;
    sal_Unicode const * pParameterBegin = pBegin;
    for (sal_Unicode const * p = pParameterBegin;; pParameterBegin = p)
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
            bDowncaseAttribute = bDowncaseAttribute || isUpperCase(*p);
            ++p;
        }
        if (p == pAttributeBegin)
            break;
        rtl::OString aAttribute = rtl::OString(
            pAttributeBegin, p - pAttributeBegin,
            RTL_TEXTENCODING_ASCII_US);
        if (bDowncaseAttribute)
            aAttribute = aAttribute.toAsciiLowerCase();

        sal_uInt32 nSection = 0;
        if (p != pEnd && *p == '*')
        {
            ++p;
            if (p != pEnd && isDigit(*p)
                && !scanUnsigned(p, pEnd, false, nSection))
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

        rtl::OString aCharset;
        rtl::OString aLanguage;
        rtl::OString aValue;
        if (bExtended)
        {
            if (nSection == 0)
            {
                sal_Unicode const * pCharsetBegin = p;
                bool bDowncaseCharset = false;
                while (p != pEnd && isTokenChar(*p) && *p != '\'')
                {
                    bDowncaseCharset = bDowncaseCharset || isUpperCase(*p);
                    ++p;
                }
                if (p == pCharsetBegin)
                    break;
                if (pParameters)
                {
                    aCharset = rtl::OString(
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
                    if (isAlpha(*p))
                    {
                        if (++nLetters > 8)
                            break;
                        bDowncaseLanguage = bDowncaseLanguage
                                            || isUpperCase(*p);
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
                    aLanguage = rtl::OString(
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
                INetMIMEStringOutputSink
                    aSink(0, INetMIMEOutputSink::NO_LINE_LENGTH_LIMIT);
                while (p != pEnd)
                {
                    sal_uInt32 nChar = INetMIME::getUTF32Character(p, pEnd);
                    if (isUSASCII(nChar) && !isTokenChar(nChar))
                        break;
                    if (nChar == '%' && p + 1 < pEnd)
                    {
                        int nWeight1 = getHexWeight(p[0]);
                        int nWeight2 = getHexWeight(p[1]);
                        if (nWeight1 >= 0 && nWeight2 >= 0)
                        {
                            aSink << sal_Char(nWeight1 << 4 | nWeight2);
                            p += 2;
                            continue;
                        }
                    }
                    INetMIME::writeUTF8(aSink, nChar);
                }
                aValue = aSink.takeBuffer();
            }
            else
                while (p != pEnd && (isTokenChar(*p) || !isUSASCII(*p)))
                    ++p;
        }
        else if (p != pEnd && *p == '"')
            if (pParameters)
            {
                INetMIMEStringOutputSink
                    aSink(0, INetMIMEOutputSink::NO_LINE_LENGTH_LIMIT);
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
                        nChar = sal_uChar(*p++);
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
                    INetMIME::writeUTF8(aSink, nChar);
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
            while (p != pEnd && (isTokenChar(*p) || !isUSASCII(*p)))
                ++p;
            if (p == pTokenBegin)
                break;
            if (pParameters)
                aValue = rtl::OString(
                    pTokenBegin, p - pTokenBegin,
                    RTL_TEXTENCODING_UTF8);
        }

        *pPos = new Parameter(*pPos, aAttribute, aCharset, aLanguage, aValue,
                              nSection, bExtended);
    }
    return parseParameters(aList, pParameters) ? pParameterBegin : pBegin;
}

// static
const sal_Char * INetMIME::getCharsetName(rtl_TextEncoding eEncoding)
{
    if (rtl_isOctetTextEncoding(eEncoding))
    {
        char const * p = rtl_getMimeCharsetFromTextEncoding(eEncoding);
        DBG_ASSERT(p, "INetMIME::getCharsetName(): Unsupported encoding");
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
                OSL_FAIL("INetMIME::getCharsetName(): Unsupported encoding");
                return 0;
        }
}

namespace unnamed_tools_inetmime {

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

template< typename T >
inline rtl_TextEncoding getCharsetEncoding_Impl(T const * pBegin,
                                                T const * pEnd)
{
    for (sal_Size i = 0; i < sizeof aEncodingMap / sizeof (EncodingEntry);
         ++i)
        if (INetMIME::equalIgnoreCase(pBegin, pEnd, aEncodingMap[i].m_aName))
            return aEncodingMap[i].m_eEncoding;
    return RTL_TEXTENCODING_DONTKNOW;
}

}

// static
rtl_TextEncoding INetMIME::getCharsetEncoding(sal_Char const * pBegin,
                                              sal_Char const * pEnd)
{
    return getCharsetEncoding_Impl(pBegin, pEnd);
}

// static
INetMIMECharsetList_Impl *
INetMIME::createPreferredCharsetList(rtl_TextEncoding eEncoding)
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
            OSL_FAIL("INetMIME::createPreferredCharsetList():"
                          " Unsupported encoding");
            break;
    }
    pList->prepend(Charset(RTL_TEXTENCODING_ISO_8859_1, aISO88591Ranges));
    pList->prepend(Charset(RTL_TEXTENCODING_ASCII_US, aUSASCIIRanges));
    return pList;
}

// static
sal_Unicode * INetMIME::convertToUnicode(const sal_Char * pBegin,
                                         const sal_Char * pEnd,
                                         rtl_TextEncoding eEncoding,
                                         sal_Size & rSize)
{
    if (eEncoding == RTL_TEXTENCODING_DONTKNOW)
        return 0;
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
        pBuffer = 0;
    }
    return pBuffer;
}

// static
sal_Char * INetMIME::convertFromUnicode(const sal_Unicode * pBegin,
                                        const sal_Unicode * pEnd,
                                        rtl_TextEncoding eEncoding,
                                        sal_Size & rSize)
{
    if (eEncoding == RTL_TEXTENCODING_DONTKNOW)
        return 0;
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
        pBuffer = 0;
    }
    return pBuffer;
}

// static
void INetMIME::writeUTF8(INetMIMEOutputSink & rSink, sal_uInt32 nChar)
{
    // See RFC 2279 for a discussion of UTF-8.
    DBG_ASSERT(nChar < 0x80000000, "INetMIME::writeUTF8(): Bad char");

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

// static
void INetMIME::writeHeaderFieldBody(INetMIMEOutputSink & rSink,
                                    HeaderFieldType eType,
                                    const rtl::OUString& rBody,
                                    rtl_TextEncoding ePreferredEncoding,
                                    bool bInitialSpace)
{
    if (eType == HEADER_FIELD_TEXT)
    {
        INetMIMEEncodedWordOutputSink
            aOutput(rSink, INetMIMEEncodedWordOutputSink::CONTEXT_TEXT,
                    bInitialSpace ?
                        INetMIMEEncodedWordOutputSink::SPACE_ALWAYS :
                        INetMIMEEncodedWordOutputSink::SPACE_NO,
                    ePreferredEncoding);
        aOutput.write(rBody.getStr(), rBody.getStr() + rBody.getLength());
        aOutput.flush();
    }
    else
    {
        enum Brackets { BRACKETS_OUTSIDE, BRACKETS_OPENING, BRACKETS_INSIDE };
        Brackets eBrackets = BRACKETS_OUTSIDE;

        const sal_Unicode * pBodyPtr = rBody.getStr();
        const sal_Unicode * pBodyEnd = pBodyPtr + rBody.getLength();
        while (pBodyPtr != pBodyEnd)
            switch (*pBodyPtr)
            {
                case '\t':
                case ' ':
                    // A WSP adds to accumulated space:
                    bInitialSpace = true;
                    ++pBodyPtr;
                    break;

                case '(':
                {
                    // Write a pending '<' if necessary:
                    if (eBrackets == BRACKETS_OPENING)
                    {
                        if (rSink.getColumn() + (bInitialSpace ? 1 : 0)
                                >= rSink.getLineLengthLimit())
                            rSink << INetMIMEOutputSink::endl << ' ';
                        else if (bInitialSpace)
                            rSink << ' ';
                        rSink << '<';
                        bInitialSpace = false;
                        eBrackets = BRACKETS_INSIDE;
                    }

                    // Write the comment, introducing encoded-words where
                    // necessary:
                    int nLevel = 0;
                    INetMIMEEncodedWordOutputSink
                        aOutput(
                            rSink,
                            INetMIMEEncodedWordOutputSink::CONTEXT_COMMENT,
                            INetMIMEEncodedWordOutputSink::SPACE_NO,
                            ePreferredEncoding);
                    while (pBodyPtr != pBodyEnd)
                        switch (*pBodyPtr)
                        {
                            case '(':
                                aOutput.flush();
                                if (rSink.getColumn()
                                            + (bInitialSpace ? 1 : 0)
                                        >= rSink.getLineLengthLimit())
                                    rSink << INetMIMEOutputSink::endl << ' ';
                                else if (bInitialSpace)
                                    rSink << ' ';
                                rSink << '(';
                                bInitialSpace = false;
                                ++nLevel;
                                ++pBodyPtr;
                                break;

                            case ')':
                                aOutput.flush();
                                if (rSink.getColumn()
                                        >= rSink.getLineLengthLimit())
                                    rSink << INetMIMEOutputSink::endl << ' ';
                                rSink << ')';
                                ++pBodyPtr;
                                if (--nLevel == 0)
                                    goto comment_done;
                                break;

                            case '\\':
                                if (++pBodyPtr == pBodyEnd)
                                    break;
                            default:
                                aOutput << *pBodyPtr++;
                                break;
                        }
                comment_done:
                    break;
                }

                case '<':
                    // Write an already pending '<' if necessary:
                    if (eBrackets == BRACKETS_OPENING)
                    {
                        if (rSink.getColumn() + (bInitialSpace ? 1 : 0)
                                >= rSink.getLineLengthLimit())
                            rSink << INetMIMEOutputSink::endl << ' ';
                        else if (bInitialSpace)
                            rSink << ' ';
                        rSink << '<';
                        bInitialSpace = false;
                    }

                    // Remember this '<' as pending, and open a bracketed
                    // block:
                    eBrackets = BRACKETS_OPENING;
                    ++pBodyPtr;
                    break;

                case '>':
                    // Write a pending '<' if necessary:
                    if (eBrackets == BRACKETS_OPENING)
                    {
                        if (rSink.getColumn() + (bInitialSpace ? 1 : 0)
                                >= rSink.getLineLengthLimit())
                            rSink << INetMIMEOutputSink::endl << ' ';
                        else if (bInitialSpace)
                            rSink << ' ';
                        rSink << '<';
                        bInitialSpace = false;
                    }

                    // Write this '>', and close any bracketed block:
                    if (rSink.getColumn() + (bInitialSpace ? 1 : 0)
                            >= rSink.getLineLengthLimit())
                        rSink << INetMIMEOutputSink::endl << ' ';
                    else if (bInitialSpace)
                        rSink << ' ';
                    rSink << '>';
                    bInitialSpace = false;
                    eBrackets = BRACKETS_OUTSIDE;
                    ++pBodyPtr;
                    break;

                case ',':
                case ':':
                case ';':
                case '\\':
                case ']':
                    // Write a pending '<' if necessary:
                    if (eBrackets == BRACKETS_OPENING)
                    {
                        if (rSink.getColumn() + (bInitialSpace ? 1 : 0)
                                >= rSink.getLineLengthLimit())
                            rSink << INetMIMEOutputSink::endl << ' ';
                        else if (bInitialSpace)
                            rSink << ' ';
                        rSink << '<';
                        bInitialSpace = false;
                        eBrackets = BRACKETS_INSIDE;
                    }

                    // Write this specials:
                    if (rSink.getColumn() + (bInitialSpace ? 1 : 0)
                            >= rSink.getLineLengthLimit())
                        rSink << INetMIMEOutputSink::endl << ' ';
                    else if (bInitialSpace)
                        rSink << ' ';
                    rSink << sal_Char(*pBodyPtr++);
                    bInitialSpace = false;
                    break;

                case '\x0D': // CR
                    // A <CRLF WSP> adds to accumulated space, a <CR> not
                    // followed by <LF WSP> starts 'junk':
                    if (startsWithLineFolding(pBodyPtr, pBodyEnd))
                    {
                        bInitialSpace = true;
                        pBodyPtr += 3;
                        break;
                    }
                default:
                {
                    // The next token is either one of <"." / "@" / atom /
                    // quoted-string / domain-literal>, or it's 'junk'; if it
                    // is not 'junk', it is either a 'phrase' (i.e., it may
                    // contain encoded-words) or a 'non-phrase' (i.e., it may
                    // not contain encoded-words):
                    enum Entity { ENTITY_JUNK, ENTITY_NON_PHRASE,
                                  ENTITY_PHRASE };
                    Entity eEntity = ENTITY_JUNK;
                    switch (*pBodyPtr)
                    {
                        case '.':
                        case '@':
                        case '[':
                            // A token of <"." / "@" / domain-literal> always
                            // starts a 'non-phrase':
                            eEntity = ENTITY_NON_PHRASE;
                            break;

                        default:
                            if (isUSASCII(*pBodyPtr)
                                && !isAtomChar(*pBodyPtr))
                            {
                                eEntity = ENTITY_JUNK;
                                break;
                            }
                        case '"':
                            // A token of <atom / quoted-string> can either be
                            // a 'phrase' or a 'non-phrase':
                            switch (eType)
                            {
                                case HEADER_FIELD_STRUCTURED:
                                    eEntity = ENTITY_NON_PHRASE;
                                    break;

                                case HEADER_FIELD_PHRASE:
                                    eEntity = ENTITY_PHRASE;
                                    break;

                                case HEADER_FIELD_MESSAGE_ID:
                                    // A 'phrase' if and only if outside any
                                    // bracketed block:
                                    eEntity
                                        = eBrackets == BRACKETS_OUTSIDE ?
                                              ENTITY_PHRASE :
                                              ENTITY_NON_PHRASE;
                                    break;

                                case HEADER_FIELD_ADDRESS:
                                {
                                    // A 'non-phrase' if and only if, after
                                    // skipping this token and any following
                                    // <linear-white-space> and <comment>s,
                                    // there is no token left, or the next
                                    // token is any of <"." / "@" / ">" / ","
                                    // / ";">, or the next token is <":"> and
                                    // is within a bracketed block:
                                    const sal_Unicode * pLookAhead = pBodyPtr;
                                    if (*pLookAhead == '"')
                                    {
                                        pLookAhead
                                            = skipQuotedString(pLookAhead,
                                                               pBodyEnd);
                                        if (pLookAhead == pBodyPtr)
                                            pLookAhead = pBodyEnd;
                                    }
                                    else
                                        while (pLookAhead != pBodyEnd
                                               && (isAtomChar(*pLookAhead)
                                                   || !isUSASCII(
                                                           *pLookAhead)))
                                            ++pLookAhead;
                                    while (pLookAhead != pBodyEnd)
                                        switch (*pLookAhead)
                                        {
                                            case '\t':
                                            case ' ':
                                                ++pLookAhead;
                                                break;

                                            case '(':
                                            {
                                                const sal_Unicode * pPast
                                                    = skipComment(pLookAhead,
                                                                  pBodyEnd);
                                                pLookAhead
                                                    = pPast == pLookAhead ?
                                                          pBodyEnd : pPast;
                                                break;
                                            }

                                            case ',':
                                            case '.':
                                            case ';':
                                            case '>':
                                            case '@':
                                                eEntity = ENTITY_NON_PHRASE;
                                                goto entity_determined;

                                            case ':':
                                                eEntity
                                                    = eBrackets
                                                         == BRACKETS_OUTSIDE ?
                                                          ENTITY_PHRASE :
                                                          ENTITY_NON_PHRASE;
                                                goto entity_determined;

                                            case '\x0D': // CR
                                                if (startsWithLineFolding(
                                                        pLookAhead, pBodyEnd))
                                                {
                                                    pLookAhead += 3;
                                                    break;
                                                }
                                            default:
                                                eEntity = ENTITY_PHRASE;
                                                goto entity_determined;
                                        }
                                    eEntity = ENTITY_NON_PHRASE;
                                entity_determined:
                                    break;
                                }

                                case HEADER_FIELD_TEXT:
                                    OSL_ASSERT(false);
                                    break;
                            }

                            // In a 'non-phrase', a non-US-ASCII character
                            // cannot be part of an <atom>, but instead the
                            // whole entity is 'junk' rather than 'non-
                            // phrase':
                            if (eEntity == ENTITY_NON_PHRASE
                                && !isUSASCII(*pBodyPtr))
                                eEntity = ENTITY_JUNK;
                            break;
                    }

                    switch (eEntity)
                    {
                        case ENTITY_JUNK:
                        {
                            // Write a pending '<' if necessary:
                            if (eBrackets == BRACKETS_OPENING)
                            {
                                if (rSink.getColumn()
                                            + (bInitialSpace ? 1 : 0)
                                        >= rSink.getLineLengthLimit())
                                    rSink << INetMIMEOutputSink::endl << ' ';
                                else if (bInitialSpace)
                                    rSink << ' ';
                                rSink << '<';
                                bInitialSpace = false;
                                eBrackets = BRACKETS_INSIDE;
                            }

                            // Calculate the length of in- and output:
                            const sal_Unicode * pStart = pBodyPtr;
                            sal_Size nLength = 0;
                            bool bModify = false;
                            bool bEnd = false;
                            while (pBodyPtr != pBodyEnd && !bEnd)
                                switch (*pBodyPtr)
                                {
                                    case '\x0D': // CR
                                        if (startsWithLineFolding(pBodyPtr,
                                                                  pBodyEnd))
                                            bEnd = true;
                                        else if (startsWithLineBreak(
                                                     pBodyPtr, pBodyEnd))
                                        {
                                            nLength += 3;
                                            bModify = true;
                                            pBodyPtr += 2;
                                        }
                                        else
                                        {
                                            ++nLength;
                                            ++pBodyPtr;
                                        }
                                        break;

                                    case '\t':
                                    case ' ':
                                        bEnd = true;
                                        break;

                                    default:
                                        if (isVisible(*pBodyPtr))
                                            bEnd = true;
                                        else if (isUSASCII(*pBodyPtr))
                                        {
                                            ++nLength;
                                            ++pBodyPtr;
                                        }
                                        else
                                        {
                                            nLength += getUTF8OctetCount(
                                                           *pBodyPtr++);
                                            bModify = true;
                                        }
                                        break;
                                }

                            // Write the output:
                            if (rSink.getColumn() + (bInitialSpace ? 1 : 0)
                                        + nLength
                                    > rSink.getLineLengthLimit())
                                rSink << INetMIMEOutputSink::endl << ' ';
                            else if (bInitialSpace)
                                rSink << ' ';
                            bInitialSpace = false;
                            if (bModify)
                                while (pStart != pBodyPtr)
                                    if (startsWithLineBreak(pStart, pBodyPtr))
                                    {
                                        rSink << "\x0D\\\x0A"; // CR, '\', LF
                                        pStart += 2;
                                    }
                                    else
                                        writeUTF8(rSink, *pStart++);
                            else
                                rSink.write(pStart, pBodyPtr);
                            break;
                        }

                        case ENTITY_NON_PHRASE:
                        {
                            // Calculate the length of in- and output:
                            const sal_Unicode * pStart = pBodyPtr;
                            sal_Size nLength = 0;
                            bool bBracketedBlock = false;
                            bool bSymbol = *pStart != '.' && *pStart != '@';
                            bool bModify = false;
                            bool bEnd = false;
                            while (pBodyPtr != pBodyEnd && !bEnd)
                                switch (*pBodyPtr)
                                {
                                    case '\t':
                                    case ' ':
                                    case '\x0D': // CR
                                    {
                                        const sal_Unicode * pLookAhead
                                            = skipLinearWhiteSpace(pBodyPtr,
                                                                   pBodyEnd);
                                        if (pLookAhead < pBodyEnd
                                            && (bSymbol ?
                                                    isAtomChar(*pLookAhead)
                                                    || *pLookAhead == '"'
                                                    || *pLookAhead == '[' :
                                                    *pLookAhead == '.'
                                                    || *pLookAhead == '@'
                                                    || (*pLookAhead == '>'
                                                       && eType
                                                    >= HEADER_FIELD_MESSAGE_ID
                                                       && eBrackets
                                                         == BRACKETS_OPENING)))
                                        {
                                            bModify = true;
                                            pBodyPtr = pLookAhead;
                                        }
                                        else
                                            bEnd = true;
                                        break;
                                    }

                                    case '"':
                                        if (bSymbol)
                                        {
                                            pBodyPtr
                                                = scanQuotedBlock(pBodyPtr,
                                                                  pBodyEnd,
                                                                  '"', '"',
                                                                  nLength,
                                                                  bModify);
                                            bSymbol = false;
                                        }
                                        else
                                            bEnd = true;
                                        break;

                                    case '[':
                                        if (bSymbol)
                                        {
                                            pBodyPtr
                                                = scanQuotedBlock(pBodyPtr,
                                                                  pBodyEnd,
                                                                  '[', ']',
                                                                  nLength,
                                                                  bModify);
                                            bSymbol = false;
                                        }
                                        else
                                            bEnd = true;
                                        break;

                                    case '.':
                                    case '@':
                                        if (bSymbol)
                                            bEnd = true;
                                        else
                                        {
                                            ++nLength;
                                            bSymbol = true;
                                            ++pBodyPtr;
                                        }
                                        break;

                                    case '>':
                                        if (eBrackets == BRACKETS_OPENING
                                            && eType
                                                   >= HEADER_FIELD_MESSAGE_ID)
                                        {
                                            ++nLength;
                                            bBracketedBlock = true;
                                            ++pBodyPtr;
                                        }
                                        bEnd = true;
                                        break;

                                    default:
                                        if (isAtomChar(*pBodyPtr) && bSymbol)
                                        {
                                            while (pBodyPtr != pBodyEnd
                                                   && isAtomChar(*pBodyPtr))
                                            {
                                                ++nLength;
                                                ++pBodyPtr;
                                            }
                                            bSymbol = false;
                                        }
                                        else
                                        {
                                            if (!isUSASCII(*pBodyPtr))
                                                bModify = true;
                                            bEnd = true;
                                        }
                                        break;
                                }

                            // Write a pending '<' if necessary:
                            if (eBrackets == BRACKETS_OPENING
                                && !bBracketedBlock)
                            {
                                if (rSink.getColumn()
                                            + (bInitialSpace ? 1 : 0)
                                        >= rSink.getLineLengthLimit())
                                    rSink << INetMIMEOutputSink::endl << ' ';
                                else if (bInitialSpace)
                                    rSink << ' ';
                                rSink << '<';
                                bInitialSpace = false;
                                eBrackets = BRACKETS_INSIDE;
                            }

                            // Write the output:
                            if (rSink.getColumn() + (bInitialSpace ? 1 : 0)
                                        + nLength
                                    > rSink.getLineLengthLimit())
                                rSink << INetMIMEOutputSink::endl << ' ';
                            else if (bInitialSpace)
                                rSink << ' ';
                            bInitialSpace = false;
                            if (bBracketedBlock)
                            {
                                rSink << '<';
                                eBrackets = BRACKETS_OUTSIDE;
                            }
                            if (bModify)
                            {
                                enum Mode { MODE_PLAIN, MODE_QUOTED_STRING,
                                            MODE_DOMAIN_LITERAL };
                                Mode eMode = MODE_PLAIN;
                                while (pStart != pBodyPtr)
                                    switch (*pStart)
                                    {
                                        case '\x0D': // CR
                                            if (startsWithLineFolding(
                                                    pStart, pBodyPtr))
                                            {
                                                if (eMode != MODE_PLAIN)
                                                    rSink << sal_Char(
                                                                 pStart[2]);
                                                pStart += 3;
                                            }
                                            else if (startsWithLineBreak(
                                                         pStart, pBodyPtr))
                                            {
                                                rSink << "\x0D\\\x0A";
                                                    // CR, '\', LF
                                                pStart += 2;
                                            }
                                            else
                                            {
                                                rSink << '\x0D'; // CR
                                                ++pStart;
                                            }
                                            break;

                                        case '\t':
                                        case ' ':
                                            if (eMode != MODE_PLAIN)
                                                rSink << sal_Char(*pStart);
                                            ++pStart;
                                            break;

                                        case '"':
                                            if (eMode == MODE_PLAIN)
                                                eMode = MODE_QUOTED_STRING;
                                            else if (eMode
                                                        == MODE_QUOTED_STRING)
                                                eMode = MODE_PLAIN;
                                            rSink << '"';
                                            ++pStart;
                                            break;

                                        case '[':
                                            if (eMode == MODE_PLAIN)
                                                eMode = MODE_DOMAIN_LITERAL;
                                            rSink << '[';
                                            ++pStart;
                                            break;

                                        case ']':
                                            if (eMode == MODE_DOMAIN_LITERAL)
                                                eMode = MODE_PLAIN;
                                            rSink << ']';
                                            ++pStart;
                                            break;

                                        case '\\':
                                            rSink << '\\';
                                            if (++pStart < pBodyPtr)
                                                writeUTF8(rSink, *pStart++);
                                            break;

                                        default:
                                            writeUTF8(rSink, *pStart++);
                                            break;
                                    }
                            }
                            else
                                rSink.write(pStart, pBodyPtr);
                            break;
                        }

                        case ENTITY_PHRASE:
                        {
                            // Write a pending '<' if necessary:
                            if (eBrackets == BRACKETS_OPENING)
                            {
                                if (rSink.getColumn()
                                            + (bInitialSpace ? 1 : 0)
                                        >= rSink.getLineLengthLimit())
                                    rSink << INetMIMEOutputSink::endl << ' ';
                                else if (bInitialSpace)
                                    rSink << ' ';
                                rSink << '<';
                                bInitialSpace = false;
                                eBrackets = BRACKETS_INSIDE;
                            }

                            // Calculate the length of in- and output:
                            const sal_Unicode * pStart = pBodyPtr;
                            bool bQuotedString = false;
                            bool bEnd = false;
                            while (pBodyPtr != pBodyEnd && !bEnd)
                                switch (*pBodyPtr)
                                {
                                    case '\t':
                                    case ' ':
                                    case '\x0D': // CR
                                        if (bQuotedString)
                                            ++pBodyPtr;
                                        else
                                        {
                                            const sal_Unicode * pLookAhead
                                                = skipLinearWhiteSpace(
                                                      pBodyPtr, pBodyEnd);
                                            if (pLookAhead != pBodyEnd
                                                && (isAtomChar(*pLookAhead)
                                                    || !isUSASCII(*pLookAhead)
                                                    || *pLookAhead == '"'))
                                                pBodyPtr = pLookAhead;
                                            else
                                                bEnd = true;
                                        }
                                        break;

                                    case '"':
                                        bQuotedString = !bQuotedString;
                                        ++pBodyPtr;
                                        break;

                                    case '\\':
                                        if (bQuotedString)
                                        {
                                            if (++pBodyPtr != pBodyEnd)
                                                ++pBodyPtr;
                                        }
                                        else
                                            bEnd = true;
                                        break;

                                    default:
                                        if (bQuotedString
                                            || isAtomChar(*pBodyPtr)
                                            || !isUSASCII(*pBodyPtr))
                                            ++pBodyPtr;
                                        else
                                            bEnd = true;
                                        break;
                                }

                            // Write the phrase, introducing encoded-words
                            // where necessary:
                            INetMIMEEncodedWordOutputSink
                                aOutput(
                                    rSink,
                                INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE,
                                    bInitialSpace ?
                                 INetMIMEEncodedWordOutputSink::SPACE_ALWAYS :
                                 INetMIMEEncodedWordOutputSink::SPACE_ENCODED,
                               ePreferredEncoding);
                            while (pStart != pBodyPtr)
                                switch (*pStart)
                                {
                                    case '"':
                                        ++pStart;
                                        break;

                                    case '\\':
                                        if (++pStart != pBodyPtr)
                                            aOutput << *pStart++;
                                        break;

                                    case '\x0D': // CR
                                        pStart += 2;
                                        aOutput << *pStart++;
                                        break;

                                    default:
                                        aOutput << *pStart++;
                                        break;
                                }
                            bInitialSpace = aOutput.flush();
                            break;
                        }
                    }
                    break;
                }
            }
    }
}

// static
bool INetMIME::translateUTF8Char(const sal_Char *& rBegin,
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
                   "INetMIME::translateUTF8Char(): Bad conversion");
        rCharacter = *pBuffer;
        delete[] pBuffer;
    }
    rBegin = p;
    return true;
}

// static
rtl::OUString INetMIME::decodeHeaderFieldBody(HeaderFieldType eType,
                                          const rtl::OString& rBody)
{
    // Due to a bug in INetCoreRFC822MessageStream::ConvertTo7Bit(), old
    // versions of StarOffice send mails with header fields where encoded
    // words can be preceded by '=', ',', '.', '"', or '(', and followed by
    // '=', ',', '.', '"', ')', without any required white space in between.
    // And there appear to exist some broken mailers that only encode single
    // letters within words, like "Appel
    // =?iso-8859-1?Q?=E0?=t=?iso-8859-1?Q?=E9?=moin", so it seems best to
    // detect encoded words even when not propperly surrounded by white space.
    //
    // Non US-ASCII characters in rBody are treated as ISO-8859-1.
    //
    // encoded-word = "=?"
    //     1*(%x21 / %x23-27 / %x2A-2B / %x2D / %30-39 / %x41-5A / %x5E-7E)
    //     ["*" 1*8ALPHA *("-" 1*8ALPHA)] "?"
    //     ("B?" *(4base64) (4base64 / 3base64 "=" / 2base64 "==")
    //      / "Q?" 1*(%x21-3C / %x3E / %x40-7E / "=" 2HEXDIG))
    //     "?="
    //
    // base64 = ALPHA / DIGIT / "+" / "/"

    const sal_Char * pBegin = rBody.getStr();
    const sal_Char * pEnd = pBegin + rBody.getLength();

    UniString sDecoded;
    const sal_Char * pCopyBegin = pBegin;

    /* bool bStartEncodedWord = true; */
    const sal_Char * pWSPBegin = pBegin;
    bool bQuotedEncodedText = false;
    sal_uInt32 nCommentLevel = 0;

    for (const sal_Char * p = pBegin; p != pEnd;)
    {
        rtl::OUString sEncodedText;
        if (p != pEnd && *p == '=' /* && bStartEncodedWord */)
        {
            const sal_Char * q = p + 1;
            bool bEncodedWord = q != pEnd && *q++ == '?';

            rtl_TextEncoding eCharsetEncoding = RTL_TEXTENCODING_DONTKNOW;
            if (bEncodedWord)
            {
                const sal_Char * pCharsetBegin = q;
                const sal_Char * pLanguageBegin = 0;
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
                                if (pLanguageBegin != 0)
                                {
                                    if (nAlphaCount == 0)
                                        pLanguageBegin = 0;
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
                                              pLanguageBegin == 0
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
                                if (pLanguageBegin != 0
                                    && (!isAlpha(cChar) || ++nAlphaCount > 8))
                                    pLanguageBegin = 0;
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

            rtl::OStringBuffer sText;
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

            sal_Unicode * pUnicodeBuffer = 0;
            sal_Size nUnicodeSize = 0;
            if (bEncodedWord)
            {
                pUnicodeBuffer
                    = convertToUnicode(sText.getStr(),
                                       sText.getStr() + sText.getLength(),
                                       eCharsetEncoding, nUnicodeSize);
                if (pUnicodeBuffer == 0)
                    bEncodedWord = false;
            }

            if (bEncodedWord)
            {
                appendISO88591(sDecoded, pCopyBegin, pWSPBegin);
                if (eType == HEADER_FIELD_TEXT)
                    sDecoded.Append(
                        pUnicodeBuffer,
                        static_cast< xub_StrLen >(nUnicodeSize));
                else if (nCommentLevel == 0)
                {
                    sEncodedText = rtl::OUString(pUnicodeBuffer, nUnicodeSize);
                    if (!bQuotedEncodedText)
                    {
                        const sal_Unicode * pTextPtr = pUnicodeBuffer;
                        const sal_Unicode * pTextEnd = pTextPtr
                                                           + nUnicodeSize;
                        for (; pTextPtr != pTextEnd; ++pTextPtr)
                            if (!isEncodedWordTokenChar(*pTextPtr))
                            {
                                bQuotedEncodedText = true;
                                break;
                            }
                    }
                }
                else
                {
                    const sal_Unicode * pTextPtr = pUnicodeBuffer;
                    const sal_Unicode * pTextEnd = pTextPtr + nUnicodeSize;
                    for (; pTextPtr != pTextEnd; ++pTextPtr)
                    {
                        switch (*pTextPtr)
                        {
                            case '(':
                            case ')':
                            case '\\':
                            case '\x0D':
                            case '=':
                                sDecoded += '\\';
                                break;
                        }
                        sDecoded += *pTextPtr;
                    }
                }
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
        {
            if (bQuotedEncodedText)
            {
                sDecoded += '"';
                const sal_Unicode * pTextPtr = sEncodedText.getStr();
                const sal_Unicode * pTextEnd = pTextPtr + sEncodedText.getLength();
                for (;pTextPtr != pTextEnd; ++pTextPtr)
                {
                    switch (*pTextPtr)
                    {
                        case '"':
                        case '\\':
                        case '\x0D':
                            sDecoded += '\\';
                            break;
                    }
                    sDecoded += *pTextPtr;
                }
                sDecoded += '"';
            }
            else
                sDecoded += sEncodedText;
            bQuotedEncodedText = false;
        }

        if (p == pEnd)
            break;

        switch (*p++)
        {
            case '"':
                if (eType != HEADER_FIELD_TEXT && nCommentLevel == 0)
                {
                    const sal_Char * pQuotedStringEnd
                        = skipQuotedString(p - 1, pEnd);
                    p = pQuotedStringEnd == p - 1 ? pEnd : pQuotedStringEnd;
                }
                /* bStartEncodedWord = true; */
                break;

            case '(':
                if (eType != HEADER_FIELD_TEXT)
                    ++nCommentLevel;
                /* bStartEncodedWord = true; */
                break;

            case ')':
                if (nCommentLevel > 0)
                    --nCommentLevel;
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
                    xub_StrLen nUTF16Len = static_cast< xub_StrLen >(
                        putUTF32Character(aUTF16Buf, nCharacter) - aUTF16Buf);
                    sDecoded.Append(aUTF16Buf, nUTF16Len);
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

//  INetMIMEOutputSink

// virtual
sal_Size INetMIMEOutputSink::writeSequence(const sal_Char * pSequence)
{
    sal_Size nLength = rtl_str_getLength(pSequence);
    writeSequence(pSequence, pSequence + nLength);
    return nLength;
}

// virtual
void INetMIMEOutputSink::writeSequence(const sal_uInt32 * pBegin,
                                       const sal_uInt32 * pEnd)
{
    DBG_ASSERT(pBegin && pBegin <= pEnd,
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

// virtual
void INetMIMEOutputSink::writeSequence(const sal_Unicode * pBegin,
                                       const sal_Unicode * pEnd)
{
    DBG_ASSERT(pBegin && pBegin <= pEnd,
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

// virtual
ErrCode INetMIMEOutputSink::getError() const
{
    return ERRCODE_NONE;
}

void INetMIMEOutputSink::writeLineEnd()
{
    static const sal_Char aCRLF[2] = { 0x0D, 0x0A };
    writeSequence(aCRLF, aCRLF + 2);
    m_nColumn = 0;
}

//  INetMIMEStringOutputSink

// virtual
void INetMIMEStringOutputSink::writeSequence(const sal_Char * pBegin,
                                             const sal_Char * pEnd)
{
    OSL_ENSURE(pBegin && pBegin <= pEnd,
               "INetMIMEStringOutputSink::writeSequence(): Bad sequence");

    m_aBuffer.append(pBegin, pEnd - pBegin);
}

// virtual
ErrCode INetMIMEStringOutputSink::getError() const
{
    return ERRCODE_NONE;
}

//  INetMIMEEncodedWordOutputSink

static const sal_Char aEscape[128]
    = { INetMIMEEncodedWordOutputSink::CONTEXT_TEXT | INetMIMEEncodedWordOutputSink::CONTEXT_COMMENT | INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE,   // 0x00
        INetMIMEEncodedWordOutputSink::CONTEXT_TEXT | INetMIMEEncodedWordOutputSink::CONTEXT_COMMENT | INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE,   // 0x01
        INetMIMEEncodedWordOutputSink::CONTEXT_TEXT | INetMIMEEncodedWordOutputSink::CONTEXT_COMMENT | INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE,   // 0x02
        INetMIMEEncodedWordOutputSink::CONTEXT_TEXT | INetMIMEEncodedWordOutputSink::CONTEXT_COMMENT | INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE,   // 0x03
        INetMIMEEncodedWordOutputSink::CONTEXT_TEXT | INetMIMEEncodedWordOutputSink::CONTEXT_COMMENT | INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE,   // 0x04
        INetMIMEEncodedWordOutputSink::CONTEXT_TEXT | INetMIMEEncodedWordOutputSink::CONTEXT_COMMENT | INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE,   // 0x05
        INetMIMEEncodedWordOutputSink::CONTEXT_TEXT | INetMIMEEncodedWordOutputSink::CONTEXT_COMMENT | INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE,   // 0x06
        INetMIMEEncodedWordOutputSink::CONTEXT_TEXT | INetMIMEEncodedWordOutputSink::CONTEXT_COMMENT | INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE,   // 0x07
        INetMIMEEncodedWordOutputSink::CONTEXT_TEXT | INetMIMEEncodedWordOutputSink::CONTEXT_COMMENT | INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE,   // 0x08
        INetMIMEEncodedWordOutputSink::CONTEXT_TEXT | INetMIMEEncodedWordOutputSink::CONTEXT_COMMENT | INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE,   // 0x09
        INetMIMEEncodedWordOutputSink::CONTEXT_TEXT | INetMIMEEncodedWordOutputSink::CONTEXT_COMMENT | INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE,   // 0x0A
        INetMIMEEncodedWordOutputSink::CONTEXT_TEXT | INetMIMEEncodedWordOutputSink::CONTEXT_COMMENT | INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE,   // 0x0B
        INetMIMEEncodedWordOutputSink::CONTEXT_TEXT | INetMIMEEncodedWordOutputSink::CONTEXT_COMMENT | INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE,   // 0x0C
        INetMIMEEncodedWordOutputSink::CONTEXT_TEXT | INetMIMEEncodedWordOutputSink::CONTEXT_COMMENT | INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE,   // 0x0D
        INetMIMEEncodedWordOutputSink::CONTEXT_TEXT | INetMIMEEncodedWordOutputSink::CONTEXT_COMMENT | INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE,   // 0x0E
        INetMIMEEncodedWordOutputSink::CONTEXT_TEXT | INetMIMEEncodedWordOutputSink::CONTEXT_COMMENT | INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE,   // 0x0F
        INetMIMEEncodedWordOutputSink::CONTEXT_TEXT | INetMIMEEncodedWordOutputSink::CONTEXT_COMMENT | INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE,   // 0x10
        INetMIMEEncodedWordOutputSink::CONTEXT_TEXT | INetMIMEEncodedWordOutputSink::CONTEXT_COMMENT | INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE,   // 0x11
        INetMIMEEncodedWordOutputSink::CONTEXT_TEXT | INetMIMEEncodedWordOutputSink::CONTEXT_COMMENT | INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE,   // 0x12
        INetMIMEEncodedWordOutputSink::CONTEXT_TEXT | INetMIMEEncodedWordOutputSink::CONTEXT_COMMENT | INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE,   // 0x13
        INetMIMEEncodedWordOutputSink::CONTEXT_TEXT | INetMIMEEncodedWordOutputSink::CONTEXT_COMMENT | INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE,   // 0x14
        INetMIMEEncodedWordOutputSink::CONTEXT_TEXT | INetMIMEEncodedWordOutputSink::CONTEXT_COMMENT | INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE,   // 0x15
        INetMIMEEncodedWordOutputSink::CONTEXT_TEXT | INetMIMEEncodedWordOutputSink::CONTEXT_COMMENT | INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE,   // 0x16
        INetMIMEEncodedWordOutputSink::CONTEXT_TEXT | INetMIMEEncodedWordOutputSink::CONTEXT_COMMENT | INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE,   // 0x17
        INetMIMEEncodedWordOutputSink::CONTEXT_TEXT | INetMIMEEncodedWordOutputSink::CONTEXT_COMMENT | INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE,   // 0x18
        INetMIMEEncodedWordOutputSink::CONTEXT_TEXT | INetMIMEEncodedWordOutputSink::CONTEXT_COMMENT | INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE,   // 0x19
        INetMIMEEncodedWordOutputSink::CONTEXT_TEXT | INetMIMEEncodedWordOutputSink::CONTEXT_COMMENT | INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE,   // 0x1A
        INetMIMEEncodedWordOutputSink::CONTEXT_TEXT | INetMIMEEncodedWordOutputSink::CONTEXT_COMMENT | INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE,   // 0x1B
        INetMIMEEncodedWordOutputSink::CONTEXT_TEXT | INetMIMEEncodedWordOutputSink::CONTEXT_COMMENT | INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE,   // 0x1C
        INetMIMEEncodedWordOutputSink::CONTEXT_TEXT | INetMIMEEncodedWordOutputSink::CONTEXT_COMMENT | INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE,   // 0x1D
        INetMIMEEncodedWordOutputSink::CONTEXT_TEXT | INetMIMEEncodedWordOutputSink::CONTEXT_COMMENT | INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE,   // 0x1E
        INetMIMEEncodedWordOutputSink::CONTEXT_TEXT | INetMIMEEncodedWordOutputSink::CONTEXT_COMMENT | INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE,   // 0x1F
        0,   // ' '
        0,   // '!'
        INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE,   // '"'
        INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE,   // '#'
        INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE,   // '$'
        INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE,   // '%'
        INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE,   // '&'
        INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE,   // '''
        INetMIMEEncodedWordOutputSink::CONTEXT_COMMENT | INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE,   // '('
        INetMIMEEncodedWordOutputSink::CONTEXT_COMMENT | INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE,   // ')'
        0,   // '*'
        0,   // '+'
        INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE,   // ','
        0,   // '-'
        INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE,   // '.'
        0,   // '/'
        0,   // '0'
        0,   // '1'
        0,   // '2'
        0,   // '3'
        0,   // '4'
        0,   // '5'
        0,   // '6'
        0,   // '7'
        0,   // '8'
        0,   // '9'
        INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE,   // ':'
        INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE,   // ';'
        INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE,   // '<'
        INetMIMEEncodedWordOutputSink::CONTEXT_TEXT | INetMIMEEncodedWordOutputSink::CONTEXT_COMMENT | INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE,   // '='
        INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE,   // '>'
        INetMIMEEncodedWordOutputSink::CONTEXT_TEXT | INetMIMEEncodedWordOutputSink::CONTEXT_COMMENT | INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE,   // '?'
        INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE,   // '@'
        0,   // 'A'
        0,   // 'B'
        0,   // 'C'
        0,   // 'D'
        0,   // 'E'
        0,   // 'F'
        0,   // 'G'
        0,   // 'H'
        0,   // 'I'
        0,   // 'J'
        0,   // 'K'
        0,   // 'L'
        0,   // 'M'
        0,   // 'N'
        0,   // 'O'
        0,   // 'P'
        0,   // 'Q'
        0,   // 'R'
        0,   // 'S'
        0,   // 'T'
        0,   // 'U'
        0,   // 'V'
        0,   // 'W'
        0,   // 'X'
        0,   // 'Y'
        0,   // 'Z'
        INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE,   // '['
        INetMIMEEncodedWordOutputSink::CONTEXT_COMMENT | INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE,   // '\'
        INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE,   // ']'
        INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE,   // '^'
        INetMIMEEncodedWordOutputSink::CONTEXT_TEXT | INetMIMEEncodedWordOutputSink::CONTEXT_COMMENT | INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE,   // '_'
        INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE,   // '`'
        0,   // 'a'
        0,   // 'b'
        0,   // 'c'
        0,   // 'd'
        0,   // 'e'
        0,   // 'f'
        0,   // 'g'
        0,   // 'h'
        0,   // 'i'
        0,   // 'j'
        0,   // 'k'
        0,   // 'l'
        0,   // 'm'
        0,   // 'n'
        0,   // 'o'
        0,   // 'p'
        0,   // 'q'
        0,   // 'r'
        0,   // 's'
        0,   // 't'
        0,   // 'u'
        0,   // 'v'
        0,   // 'w'
        0,   // 'x'
        0,   // 'y'
        0,   // 'z'
        INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE,   // '{'
        INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE,   // '|'
        INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE,   // '}'
        INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE,   // '~'
        INetMIMEEncodedWordOutputSink::CONTEXT_TEXT | INetMIMEEncodedWordOutputSink::CONTEXT_COMMENT | INetMIMEEncodedWordOutputSink::CONTEXT_PHRASE }; // DEL

inline bool
INetMIMEEncodedWordOutputSink::needsEncodedWordEscape(sal_uInt32 nChar) const
{
    return !INetMIME::isUSASCII(nChar) || aEscape[nChar] & m_eContext;
}

void INetMIMEEncodedWordOutputSink::finish(bool bWriteTrailer)
{
    if (m_eInitialSpace == SPACE_ALWAYS && m_nExtraSpaces == 0)
        m_nExtraSpaces = 1;

    if (m_eEncodedWordState == STATE_SECOND_EQUALS)
    {
        // If the text is already an encoded word, copy it verbatim:
        sal_uInt32 nSize = m_pBufferEnd - m_pBuffer;
        switch (m_ePrevCoding)
        {
            case CODING_QUOTED:
                m_rSink << '"';
            case CODING_NONE:
                if (m_eInitialSpace == SPACE_ENCODED && m_nExtraSpaces == 0)
                    m_nExtraSpaces = 1;
                for (; m_nExtraSpaces > 1; --m_nExtraSpaces)
                {
                    if (m_rSink.getColumn() >= m_rSink.getLineLengthLimit())
                        m_rSink << INetMIMEOutputSink::endl;
                    m_rSink << ' ';
                }
                if (m_nExtraSpaces == 1)
                {
                    if (m_rSink.getColumn() + nSize
                            >= m_rSink.getLineLengthLimit())
                        m_rSink << INetMIMEOutputSink::endl;
                    m_rSink << ' ';
                }
                break;

            case CODING_ENCODED:
            {
                const sal_Char * pCharsetName
                 = INetMIME::getCharsetName(m_ePrevMIMEEncoding);
                while (m_nExtraSpaces-- > 0)
                {
                    if (m_rSink.getColumn()
                            > m_rSink.getLineLengthLimit() - 3)
                        m_rSink << "?=" << INetMIMEOutputSink::endl << " =?"
                                << pCharsetName << "?Q?";
                    m_rSink << '_';
                }
                m_rSink << "?=";
            }
            case CODING_ENCODED_TERMINATED:
                if (m_rSink.getColumn() + nSize
                        > m_rSink.getLineLengthLimit() - 1)
                    m_rSink << INetMIMEOutputSink::endl;
                m_rSink << ' ';
                break;
        }
        m_rSink.write(m_pBuffer, m_pBufferEnd);
        m_eCoding = CODING_ENCODED_TERMINATED;
    }
    else
    {
        // If the text itself is too long to fit into a single line, make it
        // into multiple encoded words:
        switch (m_eCoding)
        {
            case CODING_NONE:
                if (m_nExtraSpaces == 0)
                {
                    DBG_ASSERT(m_ePrevCoding == CODING_NONE
                               || m_pBuffer == m_pBufferEnd,
                               "INetMIMEEncodedWordOutputSink::finish():"
                                   " Bad state");
                    if (m_rSink.getColumn() + (m_pBufferEnd - m_pBuffer)
                            > m_rSink.getLineLengthLimit())
                        m_eCoding = CODING_ENCODED;
                }
                else
                {
                    OSL_ASSERT(m_pBufferEnd >= m_pBuffer);
                    if (static_cast< std::size_t >(m_pBufferEnd - m_pBuffer)
                        > m_rSink.getLineLengthLimit() - 1)
                    {
                        m_eCoding = CODING_ENCODED;
                    }
                }
                break;

            case CODING_QUOTED:
                if (m_nExtraSpaces == 0)
                {
                    DBG_ASSERT(m_ePrevCoding == CODING_NONE,
                               "INetMIMEEncodedWordOutputSink::finish():"
                                   " Bad state");
                    if (m_rSink.getColumn() + (m_pBufferEnd - m_pBuffer)
                                + m_nQuotedEscaped
                            > m_rSink.getLineLengthLimit() - 2)
                        m_eCoding = CODING_ENCODED;
                }
                else if ((m_pBufferEnd - m_pBuffer) + m_nQuotedEscaped
                             > m_rSink.getLineLengthLimit() - 3)
                    m_eCoding = CODING_ENCODED;
                break;

            default:
                break;
        }

        switch (m_eCoding)
        {
            case CODING_NONE:
                switch (m_ePrevCoding)
                {
                    case CODING_QUOTED:
                        if (m_rSink.getColumn() + m_nExtraSpaces
                                    + (m_pBufferEnd - m_pBuffer)
                                < m_rSink.getLineLengthLimit())
                            m_eCoding = CODING_QUOTED;
                        else
                            m_rSink << '"';
                        break;

                    case CODING_ENCODED:
                        m_rSink << "?=";
                        break;

                    default:
                        break;
                }
                for (; m_nExtraSpaces > 1; --m_nExtraSpaces)
                {
                    if (m_rSink.getColumn() >= m_rSink.getLineLengthLimit())
                        m_rSink << INetMIMEOutputSink::endl;
                    m_rSink << ' ';
                }
                if (m_nExtraSpaces == 1)
                {
                    if (m_rSink.getColumn() + (m_pBufferEnd - m_pBuffer)
                            >= m_rSink.getLineLengthLimit())
                        m_rSink << INetMIMEOutputSink::endl;
                    m_rSink << ' ';
                }
                m_rSink.write(m_pBuffer, m_pBufferEnd);
                if (m_eCoding == CODING_QUOTED && bWriteTrailer)
                {
                    m_rSink << '"';
                    m_eCoding = CODING_NONE;
                }
                break;

            case CODING_QUOTED:
            {
                bool bInsertLeadingQuote = true;
                sal_uInt32 nSize = (m_pBufferEnd - m_pBuffer)
                                       + m_nQuotedEscaped + 2;
                switch (m_ePrevCoding)
                {
                    case CODING_QUOTED:
                        if (m_rSink.getColumn() + m_nExtraSpaces + nSize - 1
                                < m_rSink.getLineLengthLimit())
                        {
                            bInsertLeadingQuote = false;
                            --nSize;
                        }
                        else
                            m_rSink << '"';
                        break;

                    case CODING_ENCODED:
                        m_rSink << "?=";
                        break;

                    default:
                        break;
                }
                for (; m_nExtraSpaces > 1; --m_nExtraSpaces)
                {
                    if (m_rSink.getColumn() >= m_rSink.getLineLengthLimit())
                        m_rSink << INetMIMEOutputSink::endl;
                    m_rSink << ' ';
                }
                if (m_nExtraSpaces == 1)
                {
                    if (m_rSink.getColumn() + nSize
                            >= m_rSink.getLineLengthLimit())
                        m_rSink << INetMIMEOutputSink::endl;
                    m_rSink << ' ';
                }
                if (bInsertLeadingQuote)
                    m_rSink << '"';
                for (const sal_Unicode * p = m_pBuffer; p != m_pBufferEnd;
                     ++p)
                {
                    if (INetMIME::needsQuotedStringEscape(*p))
                        m_rSink << '\\';
                    m_rSink << sal_Char(*p);
                }
                if (bWriteTrailer)
                {
                    m_rSink << '"';
                    m_eCoding = CODING_NONE;
                }
                break;
            }

            case CODING_ENCODED:
            {
                rtl_TextEncoding eCharsetEncoding
                    = m_pEncodingList->
                          getPreferredEncoding(RTL_TEXTENCODING_UTF8);
                rtl_TextEncoding eMIMEEncoding
                    = INetMIME::translateToMIME(eCharsetEncoding);

                // The non UTF-8 code will only work for stateless single byte
                // character encodings (see also below):
                sal_Char * pTargetBuffer = NULL;
                sal_Size nTargetSize = 0;
                sal_uInt32 nSize;
                if (eMIMEEncoding == RTL_TEXTENCODING_UTF8)
                {
                    nSize = 0;
                    for (sal_Unicode const * p = m_pBuffer;
                         p != m_pBufferEnd;)
                    {
                        sal_uInt32 nUTF32
                            = INetMIME::getUTF32Character(p, m_pBufferEnd);
                        nSize += needsEncodedWordEscape(nUTF32) ?
                                     3 * INetMIME::getUTF8OctetCount(nUTF32) :
                                     1;
                            // only US-ASCII characters (that are converted to
                            // a single byte by UTF-8) need no encoded word
                            // escapes...
                    }
                }
                else
                {
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
                        pTargetBuffer = NULL;
                        rtl_resetUnicodeToTextContext(hConverter, hContext);
                    }
                    rtl_destroyUnicodeToTextContext(hConverter, hContext);
                    rtl_destroyUnicodeToTextConverter(hConverter);

                    nSize = nTargetSize;
                    for (sal_Size k = 0; k < nTargetSize; ++k)
                        if (needsEncodedWordEscape(sal_uChar(
                                                       pTargetBuffer[k])))
                            nSize += 2;
                }

                const sal_Char * pCharsetName
                    = INetMIME::getCharsetName(eMIMEEncoding);
                sal_uInt32 nWrapperSize = rtl_str_getLength(pCharsetName) + 7;
                    // '=?', '?Q?', '?='

                switch (m_ePrevCoding)
                {
                    case CODING_QUOTED:
                        m_rSink << '"';
                    case CODING_NONE:
                        if (m_eInitialSpace == SPACE_ENCODED
                            && m_nExtraSpaces == 0)
                            m_nExtraSpaces = 1;
                        nSize += nWrapperSize;
                        for (; m_nExtraSpaces > 1; --m_nExtraSpaces)
                        {
                            if (m_rSink.getColumn()
                                    >= m_rSink.getLineLengthLimit())
                                m_rSink << INetMIMEOutputSink::endl;
                            m_rSink << ' ';
                        }
                        if (m_nExtraSpaces == 1)
                        {
                            if (m_rSink.getColumn() + nSize
                                    >= m_rSink.getLineLengthLimit())
                            m_rSink << INetMIMEOutputSink::endl;
                            m_rSink << ' ';
                        }
                        m_rSink << "=?" << pCharsetName << "?Q?";
                        break;

                    case CODING_ENCODED:
                        if (m_ePrevMIMEEncoding != eMIMEEncoding
                            || m_rSink.getColumn() + m_nExtraSpaces + nSize
                                   > m_rSink.getLineLengthLimit() - 2)
                        {
                            m_rSink << "?=";
                            if (m_rSink.getColumn() + nWrapperSize
                                        + m_nExtraSpaces + nSize
                                    > m_rSink.getLineLengthLimit() - 1)
                                m_rSink << INetMIMEOutputSink::endl;
                            m_rSink << " =?" << pCharsetName << "?Q?";
                        }
                        while (m_nExtraSpaces-- > 0)
                        {
                            if (m_rSink.getColumn()
                                    > m_rSink.getLineLengthLimit() - 3)
                                m_rSink << "?=" << INetMIMEOutputSink::endl
                                        << " =?" << pCharsetName << "?Q?";
                            m_rSink << '_';
                        }
                        break;

                    case CODING_ENCODED_TERMINATED:
                        if (m_rSink.getColumn() + nWrapperSize
                                    + m_nExtraSpaces + nSize
                                > m_rSink.getLineLengthLimit() - 1)
                            m_rSink << INetMIMEOutputSink::endl;
                        m_rSink << " =?" << pCharsetName << "?Q?";
                        while (m_nExtraSpaces-- > 0)
                        {
                            if (m_rSink.getColumn()
                                    > m_rSink.getLineLengthLimit() - 3)
                                m_rSink << "?=" << INetMIMEOutputSink::endl
                                        << " =?" << pCharsetName << "?Q?";
                            m_rSink << '_';
                        }
                        break;
                }

                // The non UTF-8 code will only work for stateless single byte
                // character encodings (see also above):
                if (eMIMEEncoding == RTL_TEXTENCODING_UTF8)
                {
                    bool bInitial = true;
                    for (sal_Unicode const * p = m_pBuffer;
                         p != m_pBufferEnd;)
                    {
                        sal_uInt32 nUTF32
                            = INetMIME::getUTF32Character(p, m_pBufferEnd);
                        bool bEscape = needsEncodedWordEscape(nUTF32);
                        sal_uInt32 nWidth
                            = bEscape ?
                                  3 * INetMIME::getUTF8OctetCount(nUTF32) : 1;
                            // only US-ASCII characters (that are converted to
                            // a single byte by UTF-8) need no encoded word
                            // escapes...
                        if (!bInitial
                            && m_rSink.getColumn() + nWidth + 2
                                   > m_rSink.getLineLengthLimit())
                            m_rSink << "?=" << INetMIMEOutputSink::endl
                                    << " =?" << pCharsetName << "?Q?";
                        if (bEscape)
                        {
                            DBG_ASSERT(
                                nUTF32 < 0x10FFFF,
                                "INetMIMEEncodedWordOutputSink::finish():"
                                    " Bad char");
                            if (nUTF32 < 0x80)
                                INetMIME::writeEscapeSequence(m_rSink,
                                                              nUTF32);
                            else if (nUTF32 < 0x800)
                            {
                                INetMIME::writeEscapeSequence(m_rSink,
                                                              (nUTF32 >> 6)
                                                                  | 0xC0);
                                INetMIME::writeEscapeSequence(m_rSink,
                                                              (nUTF32 & 0x3F)
                                                                  | 0x80);
                            }
                            else if (nUTF32 < 0x10000)
                            {
                                INetMIME::writeEscapeSequence(m_rSink,
                                                              (nUTF32 >> 12)
                                                                  | 0xE0);
                                INetMIME::writeEscapeSequence(m_rSink,
                                                              ((nUTF32 >> 6)
                                                                      & 0x3F)
                                                                  | 0x80);
                                INetMIME::writeEscapeSequence(m_rSink,
                                                              (nUTF32 & 0x3F)
                                                                  | 0x80);
                            }
                            else
                            {
                                INetMIME::writeEscapeSequence(m_rSink,
                                                              (nUTF32 >> 18)
                                                                  | 0xF0);
                                INetMIME::writeEscapeSequence(m_rSink,
                                                              ((nUTF32 >> 12)
                                                                      & 0x3F)
                                                                  | 0x80);
                                INetMIME::writeEscapeSequence(m_rSink,
                                                              ((nUTF32 >> 6)
                                                                      & 0x3F)
                                                                  | 0x80);
                                INetMIME::writeEscapeSequence(m_rSink,
                                                              (nUTF32 & 0x3F)
                                                                  | 0x80);
                            }
                        }
                        else
                            m_rSink << sal_Char(nUTF32);
                        bInitial = false;
                    }
                }
                else
                {
                    for (sal_Size k = 0; k < nTargetSize; ++k)
                    {
                        sal_uInt32 nUCS4 = sal_uChar(pTargetBuffer[k]);
                        bool bEscape = needsEncodedWordEscape(nUCS4);
                        if (k > 0
                            && m_rSink.getColumn() + (bEscape ? 5 : 3)
                                   > m_rSink.getLineLengthLimit())
                            m_rSink << "?=" << INetMIMEOutputSink::endl
                                    << " =?" << pCharsetName << "?Q?";
                        if (bEscape)
                            INetMIME::writeEscapeSequence(m_rSink, nUCS4);
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

    m_eInitialSpace = SPACE_NO;
    m_nExtraSpaces = 0;
    m_pEncodingList->reset();
    m_pBufferEnd = m_pBuffer;
    m_ePrevCoding = m_eCoding;
    m_eCoding = CODING_NONE;
    m_nQuotedEscaped = 0;
    m_eEncodedWordState = STATE_INITIAL;
}

INetMIMEEncodedWordOutputSink::~INetMIMEEncodedWordOutputSink()
{
    rtl_freeMemory(m_pBuffer);
    delete m_pEncodingList;
}

INetMIMEEncodedWordOutputSink &
INetMIMEEncodedWordOutputSink::operator <<(sal_uInt32 nChar)
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
                if (INetMIME::isEncodedWordTokenChar(nChar))
                    m_eEncodedWordState = STATE_CHARSET;
                else
                    m_eEncodedWordState = STATE_BAD;
                break;

            case STATE_CHARSET:
                if (nChar == '?')
                    m_eEncodedWordState = STATE_SECOND_QUESTION;
                else if (!INetMIME::isEncodedWordTokenChar(nChar))
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
        enum { TENQ = 1,   // CONTEXT_TEXT, CODING_ENCODED
               CENQ = 2,   // CONTEXT_COMMENT, CODING_ENCODED
               PQTD = 4,   // CONTEXT_PHRASE, CODING_QUOTED
               PENQ = 8 }; // CONTEXT_PHRASE, CODING_ENCODED
        static const sal_Char aMinimal[128]
            = { TENQ | CENQ        | PENQ,   // 0x00
                TENQ | CENQ        | PENQ,   // 0x01
                TENQ | CENQ        | PENQ,   // 0x02
                TENQ | CENQ        | PENQ,   // 0x03
                TENQ | CENQ        | PENQ,   // 0x04
                TENQ | CENQ        | PENQ,   // 0x05
                TENQ | CENQ        | PENQ,   // 0x06
                TENQ | CENQ        | PENQ,   // 0x07
                TENQ | CENQ        | PENQ,   // 0x08
                TENQ | CENQ        | PENQ,   // 0x09
                TENQ | CENQ        | PENQ,   // 0x0A
                TENQ | CENQ        | PENQ,   // 0x0B
                TENQ | CENQ        | PENQ,   // 0x0C
                TENQ | CENQ        | PENQ,   // 0x0D
                TENQ | CENQ        | PENQ,   // 0x0E
                TENQ | CENQ        | PENQ,   // 0x0F
                TENQ | CENQ        | PENQ,   // 0x10
                TENQ | CENQ        | PENQ,   // 0x11
                TENQ | CENQ        | PENQ,   // 0x12
                TENQ | CENQ        | PENQ,   // 0x13
                TENQ | CENQ        | PENQ,   // 0x14
                TENQ | CENQ        | PENQ,   // 0x15
                TENQ | CENQ        | PENQ,   // 0x16
                TENQ | CENQ        | PENQ,   // 0x17
                TENQ | CENQ        | PENQ,   // 0x18
                TENQ | CENQ        | PENQ,   // 0x19
                TENQ | CENQ        | PENQ,   // 0x1A
                TENQ | CENQ        | PENQ,   // 0x1B
                TENQ | CENQ        | PENQ,   // 0x1C
                TENQ | CENQ        | PENQ,   // 0x1D
                TENQ | CENQ        | PENQ,   // 0x1E
                TENQ | CENQ        | PENQ,   // 0x1F
                                        0,   // ' '
                                        0,   // '!'
                              PQTD       ,   // '"'
                                        0,   // '#'
                                        0,   // '$'
                                        0,   // '%'
                                        0,   // '&'
                                        0,   // '''
                       CENQ | PQTD       ,   // '('
                       CENQ | PQTD       ,   // ')'
                                        0,   // '*'
                                        0,   // '+'
                              PQTD       ,   // ','
                                        0,   // '-'
                              PQTD       ,   // '.'
                                        0,   // '/'
                                        0,   // '0'
                                        0,   // '1'
                                        0,   // '2'
                                        0,   // '3'
                                        0,   // '4'
                                        0,   // '5'
                                        0,   // '6'
                                        0,   // '7'
                                        0,   // '8'
                                        0,   // '9'
                              PQTD       ,   // ':'
                              PQTD       ,   // ';'
                              PQTD       ,   // '<'
                                        0,   // '='
                              PQTD       ,   // '>'
                                        0,   // '?'
                              PQTD       ,   // '@'
                                        0,   // 'A'
                                        0,   // 'B'
                                        0,   // 'C'
                                        0,   // 'D'
                                        0,   // 'E'
                                        0,   // 'F'
                                        0,   // 'G'
                                        0,   // 'H'
                                        0,   // 'I'
                                        0,   // 'J'
                                        0,   // 'K'
                                        0,   // 'L'
                                        0,   // 'M'
                                        0,   // 'N'
                                        0,   // 'O'
                                        0,   // 'P'
                                        0,   // 'Q'
                                        0,   // 'R'
                                        0,   // 'S'
                                        0,   // 'T'
                                        0,   // 'U'
                                        0,   // 'V'
                                        0,   // 'W'
                                        0,   // 'X'
                                        0,   // 'Y'
                                        0,   // 'Z'
                              PQTD       ,   // '['
                       CENQ | PQTD       ,   // '\'
                              PQTD       ,   // ']'
                                        0,   // '^'
                                        0,   // '_'
                                        0,   // '`'
                                        0,   // 'a'
                                        0,   // 'b'
                                        0,   // 'c'
                                        0,   // 'd'
                                        0,   // 'e'
                                        0,   // 'f'
                                        0,   // 'g'
                                        0,   // 'h'
                                        0,   // 'i'
                                        0,   // 'j'
                                        0,   // 'k'
                                        0,   // 'l'
                                        0,   // 'm'
                                        0,   // 'n'
                                        0,   // 'o'
                                        0,   // 'p'
                                        0,   // 'q'
                                        0,   // 'r'
                                        0,   // 's'
                                        0,   // 't'
                                        0,   // 'u'
                                        0,   // 'v'
                                        0,   // 'w'
                                        0,   // 'x'
                                        0,   // 'y'
                                        0,   // 'z'
                                        0,   // '{'
                                        0,   // '|'
                                        0,   // '}'
                                        0,   // '~'
                TENQ | CENQ        | PENQ }; // DEL
        Coding eNewCoding = !INetMIME::isUSASCII(nChar) ? CODING_ENCODED :
                            m_eContext == CONTEXT_PHRASE ?
                                Coding(aMinimal[nChar] >> 2) :
                            aMinimal[nChar] & m_eContext ? CODING_ENCODED :
                                                           CODING_NONE;
        if (eNewCoding > m_eCoding)
            m_eCoding = eNewCoding;
        if (m_eCoding == CODING_QUOTED
            && INetMIME::needsQuotedStringEscape(nChar))
            ++m_nQuotedEscaped;

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

//  INetContentTypeParameterList

void INetContentTypeParameterList::Clear()
{
    maEntries.clear();
}

const INetContentTypeParameter *
INetContentTypeParameterList::find(const rtl::OString& rAttribute) const
{
    boost::ptr_vector<INetContentTypeParameter>::const_iterator iter;
    for (iter = maEntries.begin(); iter != maEntries.end(); ++iter)
    {
        if (iter->m_sAttribute.equalsIgnoreAsciiCase(rAttribute))
            return &(*iter);
    }

    return NULL;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
