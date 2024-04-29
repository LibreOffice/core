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

#include <sal/config.h>

#include <cassert>
#include <cstddef>
#include <string_view>
#include <utility>
#include <vector>
#include <algorithm>

#include <o3tl/safeint.hxx>
#include <o3tl/string_view.hxx>
#include <rtl/character.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/string.hxx>
#include <rtl/strbuf.hxx>
#include <sal/log.hxx>
#include <sal/types.h>

#include <comphelper/string.hxx>
#include <comphelper/stl_types.hxx>
#include <comphelper/sequence.hxx>

#include <com/sun/star/i18n/BreakIterator.hpp>
#include <com/sun/star/i18n/CharType.hpp>
#include <com/sun/star/i18n/Collator.hpp>


namespace comphelper::string {

namespace
{
    template <typename T, typename C> T tmpl_stripStart(const T &rIn,
        const C cRemove)
    {
        if (rIn.empty())
            return rIn;

        typename T::size_type i = 0;

        while (i < rIn.size())
        {
            if (rIn[i] != cRemove)
                break;
            ++i;
        }

        return rIn.substr(i);
    }
    template <typename T, typename C> T tmpl_stripStartString(const T &rIn,
        const C cRemove)
    {
        if (rIn.isEmpty())
            return rIn;

        sal_Int32 i = 0;

        while (i < rIn.getLength())
        {
            if (rIn[i] != cRemove)
                break;
            ++i;
        }

        return rIn.copy(i);
    }
}

OString stripStart(const OString& rIn, char c)
{
    return tmpl_stripStartString<OString, char>(rIn, c);
}

std::string_view stripStart(std::string_view rIn, char c)
{
    return tmpl_stripStart<std::string_view, char>(rIn, c);
}

OUString stripStart(const OUString& rIn, sal_Unicode c)
{
    return tmpl_stripStartString<OUString, sal_Unicode>(rIn, c);
}

std::u16string_view stripStart(std::u16string_view rIn, sal_Unicode c)
{
    return tmpl_stripStart<std::u16string_view, sal_Unicode>(rIn, c);
}

namespace
{
    template <typename T, typename C> T tmpl_stripEnd(const T &rIn,
        const C cRemove)
    {
        if (rIn.empty())
            return rIn;

        typename T::size_type i = rIn.size();

        while (i > 0)
        {
            if (rIn[i-1] != cRemove)
                break;
            --i;
        }

        return rIn.substr(0, i);
    }
    template <typename T, typename C> T tmpl_stripEndString(const T &rIn,
        const C cRemove)
    {
        if (rIn.isEmpty())
            return rIn;

        sal_Int32 i = rIn.getLength();

        while (i > 0)
        {
            if (rIn[i-1] != cRemove)
                break;
            --i;
        }

        return rIn.copy(0, i);
    }
}

OString stripEnd(const OString& rIn, char c)
{
    return tmpl_stripEndString<OString, char>(rIn, c);
}

std::string_view stripEnd(std::string_view rIn, char c)
{
    return tmpl_stripEnd<std::string_view, char>(rIn, c);
}

OUString stripEnd(const OUString& rIn, sal_Unicode c)
{
    return tmpl_stripEndString<OUString, sal_Unicode>(rIn, c);
}

std::u16string_view stripEnd(std::u16string_view rIn, sal_Unicode c)
{
    return tmpl_stripEnd<std::u16string_view, sal_Unicode>(rIn, c);
}

namespace
{
    template <typename T, typename C> T tmpl_strip(const T &rIn,
        const C cRemove)
    {
        if (rIn.empty())
            return rIn;

        typename T::size_type end = rIn.size();
        while (end > 0)
        {
            if (rIn[end-1] != cRemove)
                break;
            --end;
        }

        typename T::size_type start = 0;
        while (start < end)
        {
            if (rIn[start] != cRemove)
                break;
            ++start;
        }

        return rIn.substr(start, end - start);
    }
    template <typename T, typename C> T tmpl_stripString(const T &rIn,
        const C cRemove)
    {
        if (rIn.isEmpty())
            return rIn;

        sal_Int32 end = rIn.getLength();
        while (end > 0)
        {
            if (rIn[end-1] != cRemove)
                break;
            --end;
        }
        sal_Int32 start = 0;
        while (start < end)
        {
            if (rIn[start] != cRemove)
                break;
            ++start;
        }

        return rIn.copy(start, end - start);
    }
}

OString strip(const OString& rIn, char c)
{
    return tmpl_stripString<OString, char>(rIn, c);
}

std::string_view strip(std::string_view rIn, char c)
{
    return tmpl_strip<std::string_view, char>(rIn, c);
}

OUString strip(const OUString& rIn, sal_Unicode c)
{
    return tmpl_stripString<OUString, sal_Unicode>(rIn, c);
}

std::u16string_view strip(std::u16string_view rIn, sal_Unicode c)
{
    return tmpl_strip<std::u16string_view, sal_Unicode>(rIn, c);
}

namespace
{
    template <typename T, typename C> sal_Int32 tmpl_getTokenCount( T rIn,
        C cTok)
    {
        // Empty String: TokenCount by Definition is 0
        if (rIn.empty())
            return 0;

        sal_Int32 nTokCount = 1;
        for (typename T::size_type i = 0; i < rIn.size(); ++i)
        {
            if (rIn[i] == cTok)
                ++nTokCount;
        }
        return nTokCount;
    }
}

sal_Int32 getTokenCount(std::string_view rIn, char cTok)
{
    return tmpl_getTokenCount<std::string_view, char>(rIn, cTok);
}

sal_Int32 getTokenCount(std::u16string_view rIn, sal_Unicode cTok)
{
    return tmpl_getTokenCount<std::u16string_view, sal_Unicode>(rIn, cTok);
}

sal_uInt32 decimalStringToNumber(std::u16string_view str)
{
    sal_uInt32 result = 0;
    for( std::size_t i = 0; i < str.size(); )
    {
        sal_uInt32 c = o3tl::iterateCodePoints(str, &i);
        sal_uInt32 value = 0;
        if( c <= 0x0039)    // ASCII decimal digits, most common
            value = c - 0x0030;
        else if( c >= 0x1D7F6 )    // mathematical monospace digits
            value = c - 0x1D7F6;
        else if( c >= 0x1D7EC ) // mathematical sans-serif bold digits
            value = c - 0x1D7EC;
        else if( c >= 0x1D7E2 ) // mathematical sans-serif digits
            value = c - 0x1D7E2;
        else if( c >= 0x1D7D8 ) // mathematical double-struck digits
            value = c - 0x1D7D8;
        else if( c >= 0x1D7CE ) // mathematical bold digits
            value = c - 0x1D7CE;
        else if( c >= 0x11066 ) // brahmi digits
            value = c - 0x11066;
        else if( c >= 0x104A0 ) // osmanya digits
            value = c - 0x104A0;
        else if( c >= 0xFF10 ) // fullwidth digits
            value = c - 0xFF10;
        else if( c >= 0xABF0 ) // meetei mayek digits
            value = c - 0xABF0;
        else if( c >= 0xAA50 ) // cham digits
            value = c - 0xAA50;
        else if( c >= 0xA9D0 ) // javanese digits
            value = c - 0xA9D0;
        else if( c >= 0xA900 ) // kayah li digits
            value = c - 0xA900;
        else if( c >= 0xA8D0 ) // saurashtra digits
            value = c - 0xA8D0;
        else if( c >= 0xA620 ) // vai digits
            value = c - 0xA620;
        else if( c >= 0x1C50 ) // ol chiki digits
            value = c - 0x1C50;
        else if( c >= 0x1C40 ) // lepcha digits
            value = c - 0x1C40;
        else if( c >= 0x1BB0 ) // sundanese digits
            value = c - 0x1BB0;
        else if( c >= 0x1B50 ) // balinese digits
            value = c - 0x1B50;
        else if( c >= 0x1A90 ) // tai tham tham digits
            value = c - 0x1A90;
        else if( c >= 0x1A80 ) // tai tham hora digits
            value = c - 0x1A80;
        else if( c >= 0x19D0 ) // new tai lue digits
            value = c - 0x19D0;
        else if( c >= 0x1946 ) // limbu digits
            value = c - 0x1946;
        else if( c >= 0x1810 ) // mongolian digits
            value = c - 0x1810;
        else if( c >= 0x17E0 ) // khmer digits
            value = c - 0x17E0;
        else if( c >= 0x1090 ) // myanmar shan digits
            value = c - 0x1090;
        else if( c >= 0x1040 ) // myanmar digits
            value = c - 0x1040;
        else if( c >= 0x0F20 ) // tibetan digits
            value = c - 0x0F20;
        else if( c >= 0x0ED0 ) // lao digits
            value = c - 0x0ED0;
        else if( c >= 0x0E50 ) // thai digits
            value = c - 0x0E50;
        else if( c >= 0x0D66 ) // malayalam digits
            value = c - 0x0D66;
        else if( c >= 0x0CE6 ) // kannada digits
            value = c - 0x0CE6;
        else if( c >= 0x0C66 ) // telugu digits
            value = c - 0x0C66;
        else if( c >= 0x0BE6 ) // tamil digits
            value = c - 0x0BE6;
        else if( c >= 0x0B66 ) // odia digits
            value = c - 0x0B66;
        else if( c >= 0x0AE6 ) // gujarati digits
            value = c - 0x0AE6;
        else if( c >= 0x0A66 ) // gurmukhi digits
            value = c - 0x0A66;
        else if( c >= 0x09E6 ) // bengali digits
            value = c - 0x09E6;
        else if( c >= 0x0966 ) // devanagari digit
            value = c - 0x0966;
        else if( c >= 0x07C0 ) // nko digits
            value = c - 0x07C0;
        else if( c >= 0x06F0 ) // extended arabic-indic digits
            value = c - 0x06F0;
        else if( c >= 0x0660 ) // arabic-indic digits
            value = c - 0x0660;
        result = result * 10 + value;
    }
    return result;
}

using namespace ::com::sun::star;

// convert between sequence of string and comma separated string

OUString convertCommaSeparated(
    uno::Sequence< OUString > const& i_rSeq)
{
    OUStringBuffer buf;
    ::comphelper::intersperse(
        i_rSeq.begin(), i_rSeq.end(), ::comphelper::OUStringBufferAppender(buf), u", "_ustr);
    return buf.makeStringAndClear();
}

std::vector<OUString>
    split(std::u16string_view rStr, sal_Unicode cSeparator)
{
    std::vector< OUString > vec;
    std::size_t idx = 0;
    do
    {
        std::u16string_view kw = o3tl::getToken(rStr, cSeparator, idx);
        kw = o3tl::trim(kw);
        if (!kw.empty())
        {
            vec.push_back(OUString(kw));
        }

    } while (idx != std::u16string_view::npos);

    return vec;
}

uno::Sequence< OUString >
    convertCommaSeparated( std::u16string_view i_rString )
{
    std::vector< OUString > vec = split(i_rString, ',');
    return comphelper::containerToSequence(vec);
}

OString join(std::string_view rSeparator, const std::vector<OString>& rSequence)
{
    OStringBuffer aBuffer;
    for (size_t i = 0; i < rSequence.size(); ++i)
    {
        if (i != 0)
            aBuffer.append(rSeparator);
        aBuffer.append(rSequence[i]);
    }
    return aBuffer.makeStringAndClear();
}

sal_Int32 compareNatural( const OUString & rLHS, const OUString & rRHS,
    const uno::Reference< i18n::XCollator > &rCollator,
    const uno::Reference< i18n::XBreakIterator > &rBI,
    const lang::Locale &rLocale )
{
    sal_Int32 nRet = 0;

    sal_Int32 nLHSLastNonDigitPos = 0;
    sal_Int32 nRHSLastNonDigitPos = 0;
    sal_Int32 nLHSFirstDigitPos = 0;
    sal_Int32 nRHSFirstDigitPos = 0;

    // Check if the string starts with a digit
    sal_Int32 nStartsDigitLHS = rBI->endOfCharBlock(rLHS, nLHSFirstDigitPos, rLocale, i18n::CharType::DECIMAL_DIGIT_NUMBER);
    sal_Int32 nStartsDigitRHS = rBI->endOfCharBlock(rRHS, nRHSFirstDigitPos, rLocale, i18n::CharType::DECIMAL_DIGIT_NUMBER);

    if (nStartsDigitLHS > 0 && nStartsDigitRHS > 0)
    {
        sal_uInt32 nLHS = comphelper::string::decimalStringToNumber(rLHS.subView(0, nStartsDigitLHS));
        sal_uInt32 nRHS = comphelper::string::decimalStringToNumber(rRHS.subView(0, nStartsDigitRHS));

        if (nLHS != nRHS)
            return nLHS < nRHS ? -1 : 1;
        nLHSLastNonDigitPos = nStartsDigitLHS;
        nRHSLastNonDigitPos = nStartsDigitRHS;
    }
    else if (nStartsDigitLHS > 0)
        return -1;
    else if (nStartsDigitRHS > 0)
        return 1;

    while (nLHSFirstDigitPos < rLHS.getLength() || nRHSFirstDigitPos < rRHS.getLength())
    {
        sal_Int32 nLHSChunkLen;
        sal_Int32 nRHSChunkLen;

        //Compare non digit block as normal strings
        nLHSFirstDigitPos = rBI->nextCharBlock(rLHS, nLHSLastNonDigitPos, rLocale, i18n::CharType::DECIMAL_DIGIT_NUMBER);
        nRHSFirstDigitPos = rBI->nextCharBlock(rRHS, nRHSLastNonDigitPos, rLocale, i18n::CharType::DECIMAL_DIGIT_NUMBER);

        if (nLHSFirstDigitPos == -1)
            nLHSFirstDigitPos = rLHS.getLength();

        if (nRHSFirstDigitPos == -1)
            nRHSFirstDigitPos = rRHS.getLength();

        nLHSChunkLen = nLHSFirstDigitPos - nLHSLastNonDigitPos;
        nRHSChunkLen = nRHSFirstDigitPos - nRHSLastNonDigitPos;

        nRet = rCollator->compareSubstring(rLHS, nLHSLastNonDigitPos, nLHSChunkLen, rRHS, nRHSLastNonDigitPos, nRHSChunkLen);
        if (nRet != 0)
            break;

        //Compare digit block as one number vs another
        nLHSLastNonDigitPos = rBI->endOfCharBlock(rLHS, nLHSFirstDigitPos, rLocale, i18n::CharType::DECIMAL_DIGIT_NUMBER);
        nRHSLastNonDigitPos = rBI->endOfCharBlock(rRHS, nRHSFirstDigitPos, rLocale, i18n::CharType::DECIMAL_DIGIT_NUMBER);
        if (nLHSLastNonDigitPos == -1)
            nLHSLastNonDigitPos = rLHS.getLength();
        if (nRHSLastNonDigitPos == -1)
            nRHSLastNonDigitPos = rRHS.getLength();
        nLHSChunkLen = nLHSLastNonDigitPos - nLHSFirstDigitPos;
        nRHSChunkLen = nRHSLastNonDigitPos - nRHSFirstDigitPos;

        //To-Do: Possibly scale down those unicode codepoints that relate to
        //numbers outside of the normal 0-9 range, e.g. see GetLocalizedChar in
        //vcl

        sal_uInt32 nLHS = comphelper::string::decimalStringToNumber(rLHS.subView(nLHSFirstDigitPos, nLHSChunkLen));
        sal_uInt32 nRHS = comphelper::string::decimalStringToNumber(rRHS.subView(nRHSFirstDigitPos, nRHSChunkLen));

        if (nLHS != nRHS)
        {
            nRet = (nLHS < nRHS) ? -1 : 1;
            break;
        }
    }

    return nRet;
}

NaturalStringSorter::NaturalStringSorter(
    const uno::Reference< uno::XComponentContext > &rContext,
    lang::Locale aLocale) : m_aLocale(std::move(aLocale))
{
    m_xCollator = i18n::Collator::create( rContext );
    m_xCollator->loadDefaultCollator(m_aLocale, 0);
    m_xBI = i18n::BreakIterator::create( rContext );
}

bool isdigitAsciiString(std::string_view rString)
{
    return std::all_of(
        rString.data(), rString.data() + rString.size(),
        [](unsigned char c){ return rtl::isAsciiDigit(c); });
}

bool isdigitAsciiString(std::u16string_view rString)
{
    return std::all_of(
        rString.data(), rString.data() + rString.size(),
        [](sal_Unicode c){ return rtl::isAsciiDigit(c); });
}

OUString reverseString(std::u16string_view rStr)
{
    if (rStr.empty())
        return OUString();

    std::size_t i = rStr.size();
    OUStringBuffer sBuf(static_cast<sal_Int32>(i));
    while (i)
        sBuf.append(rStr[--i]);
    return sBuf.makeStringAndClear();
}

OUString reverseCodePoints(std::u16string_view str) {
    auto const len = str.size();
    OUStringBuffer buf(len);
    for (sal_Int32 i = len; i != 0;) {
        buf.appendUtf32(o3tl::iterateCodePoints(str, &i, -1));
    }
    return buf.makeStringAndClear();
}

sal_Int32 indexOfAny(std::u16string_view rIn,
        sal_Unicode const*const pChars, sal_Int32 const nPos)
{
    for (std::u16string_view::size_type i = nPos; i < rIn.size(); ++i)
    {
        sal_Unicode const c = rIn[i];
        for (sal_Unicode const* pChar = pChars; *pChar; ++pChar)
        {
            if (c == *pChar)
            {
                return i;
            }
        }
    }
    return -1;
}

OUString removeAny(std::u16string_view rIn,
        sal_Unicode const*const pChars)
{
    OUStringBuffer buf;
    bool isFound(false);
    for (std::u16string_view::size_type i = 0; i < rIn.size(); ++i)
    {
        sal_Unicode const c = rIn[i];
        bool removeC(false);
        for (sal_Unicode const* pChar = pChars; *pChar; ++pChar)
        {
            if (c == *pChar)
            {
                removeC = true;
                break;
            }
        }
        if (removeC)
        {
            if (!isFound)
            {
                if (i > 0)
                {
                    buf.append(rIn.substr(0, i));
                }
                isFound = true;
            }
        }
        else if (isFound)
        {
            buf.append(c);
        }
    }
    return isFound ? buf.makeStringAndClear() : OUString(rIn);
}

OUString setToken(const OUString& rIn, sal_Int32 nToken, sal_Unicode cTok,
    std::u16string_view rNewToken)
{
    sal_Int32 nLen = rIn.getLength();
    sal_Int32 nTok = 0;
    sal_Int32 nFirstChar = 0;
    sal_Int32 i = 0;

    // Determine token position and length
    while ( i < nLen )
    {
        // Increase token count if match
        if (rIn[i] == cTok)
        {
            ++nTok;

            if (nTok == nToken)
                nFirstChar = i+1;
            else if (nTok > nToken)
                break;
        }

        ++i;
    }

    if (nTok >= nToken)
        return rIn.replaceAt(nFirstChar, i-nFirstChar, rNewToken);
    return rIn;
}

/** Similar to OUString::replaceAt, but for an OUStringBuffer.

    Replace n = count characters
    from position index in this string with newStr.
 */
void replaceAt(OUStringBuffer& rIn, sal_Int32 nIndex, sal_Int32 nCount, std::u16string_view newStr )
{
    assert(nIndex >= 0 && nIndex <= rIn.getLength());
    assert(nCount >= 0);
    assert(nCount <= rIn.getLength() - nIndex);

    /* Append? */
    const sal_Int32 nOldLength = rIn.getLength();
    if ( nIndex == nOldLength )
    {
        rIn.append(newStr);
        return;
    }

    sal_Int32 nNewLength = nOldLength + newStr.size() - nCount;
    if (newStr.size() > o3tl::make_unsigned(nCount))
        rIn.ensureCapacity(nOldLength + newStr.size() - nCount);

    sal_Unicode* pStr = const_cast<sal_Unicode*>(rIn.getStr());
    memmove(pStr + nIndex + newStr.size(), pStr + nIndex + nCount, nOldLength - nIndex + nCount);
    memcpy(pStr + nIndex, newStr.data(), newStr.size());

    rIn.setLength(nNewLength);
}

OUString sanitizeStringSurrogates(const OUString& rString)
{
    sal_Int32 i=0;
    while (i < rString.getLength())
    {
        sal_Unicode c = rString[i];
        if (rtl::isHighSurrogate(c))
        {
            if (i+1 == rString.getLength()
                || !rtl::isLowSurrogate(rString[i+1]))
            {
                SAL_WARN("comphelper", "Surrogate error: high without low");
                return rString.copy(0, i);
            }
            ++i;    //skip correct low
        }
        if (rtl::isLowSurrogate(c)) //bare low without preceding high
        {
            SAL_WARN("comphelper", "Surrogate error: low without high");
            return rString.copy(0, i);
        }
        ++i;
    }
    return rString;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
