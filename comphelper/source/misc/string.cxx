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

#include "sal/config.h"

#include <cstddef>
#include <string.h>
#include <vector>
#include <algorithm>

#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/string.hxx>
#include <rtl/strbuf.hxx>
#include <sal/types.h>

#include <comphelper/string.hxx>
#include <comphelper/stlunosequence.hxx>
#include <comphelper/stl_types.hxx>

#include <com/sun/star/i18n/BreakIterator.hpp>
#include <com/sun/star/i18n/CharType.hpp>


namespace comphelper { namespace string {

namespace
{
    template <typename T, typename C> T tmpl_stripStart(const T &rIn,
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

rtl::OString stripStart(const rtl::OString &rIn, sal_Char c)
{
    return tmpl_stripStart<rtl::OString, sal_Char>(rIn, c);
}

rtl::OUString stripStart(const rtl::OUString &rIn, sal_Unicode c)
{
    return tmpl_stripStart<rtl::OUString, sal_Unicode>(rIn, c);
}

namespace
{
    template <typename T, typename C> T tmpl_stripEnd(const T &rIn,
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

rtl::OString stripEnd(const rtl::OString &rIn, sal_Char c)
{
    return tmpl_stripEnd<rtl::OString, sal_Char>(rIn, c);
}

rtl::OUString stripEnd(const rtl::OUString &rIn, sal_Unicode c)
{
    return tmpl_stripEnd<rtl::OUString, sal_Unicode>(rIn, c);
}

rtl::OString strip(const rtl::OString &rIn, sal_Char c)
{
    return stripEnd(stripStart(rIn, c), c);
}

rtl::OUString strip(const rtl::OUString &rIn, sal_Unicode c)
{
    return stripEnd(stripStart(rIn, c), c);
}

namespace
{
    template <typename T, typename C> sal_Int32 tmpl_getTokenCount(const T &rIn,
        C cTok)
    {
        // Empty String: TokenCount by Definition is 0
        if (rIn.isEmpty())
            return 0;

        sal_Int32 nTokCount = 1;
        for (sal_Int32 i = 0; i < rIn.getLength(); ++i)
        {
            if (rIn[i] == cTok)
                ++nTokCount;
        }
        return nTokCount;
    }
}

sal_Int32 getTokenCount(const rtl::OString &rIn, sal_Char cTok)
{
    return tmpl_getTokenCount<rtl::OString, sal_Char>(rIn, cTok);
}

sal_Int32 getTokenCount(const rtl::OUString &rIn, sal_Unicode cTok)
{
    return tmpl_getTokenCount<rtl::OUString, sal_Unicode>(rIn, cTok);
}

sal_uInt32 decimalStringToNumber(
    ::rtl::OUString const & str )
{
    sal_uInt32 result = 0;
    for( sal_Int32 i = 0 ; i < str.getLength() ; )
    {
        sal_uInt32 c = str.iterateCodePoints(&i);
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
        else if( c >= 0x0B66 ) // oriya digits
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

::rtl::OUString convertCommaSeparated(
    uno::Sequence< ::rtl::OUString > const& i_rSeq)
{
    ::rtl::OUStringBuffer buf;
    ::comphelper::intersperse(
        ::comphelper::stl_begin(i_rSeq), ::comphelper::stl_end(i_rSeq),
        ::comphelper::OUStringBufferAppender(buf),
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ", " )));
    return buf.makeStringAndClear();
}

uno::Sequence< ::rtl::OUString >
    convertCommaSeparated( ::rtl::OUString const& i_rString )
{
    std::vector< ::rtl::OUString > vec;
    sal_Int32 idx = 0;
    do {
      ::rtl::OUString kw =
        i_rString.getToken(0, static_cast<sal_Unicode> (','), idx);
      kw = kw.trim();
      if (!kw.isEmpty()) {
          vec.push_back(kw);
      }
    } while (idx >= 0);
    uno::Sequence< ::rtl::OUString > kws(vec.size());
    std::copy(vec.begin(), vec.end(), stl_begin(kws));
    return kws;
}


sal_Int32 compareNatural( const ::rtl::OUString & rLHS, const ::rtl::OUString & rRHS,
    const uno::Reference< i18n::XCollator > &rCollator,
    const uno::Reference< i18n::XBreakIterator > &rBI,
    const lang::Locale &rLocale )
{
    sal_Int32 nRet = 0;

    sal_Int32 nLHSLastNonDigitPos = 0;
    sal_Int32 nRHSLastNonDigitPos = 0;
    sal_Int32 nLHSFirstDigitPos = 0;
    sal_Int32 nRHSFirstDigitPos = 0;

    while (nLHSFirstDigitPos < rLHS.getLength() || nRHSFirstDigitPos < rRHS.getLength())
    {
        sal_Int32 nLHSChunkLen;
        sal_Int32 nRHSChunkLen;

        //Compare non digit block as normal strings
        nLHSFirstDigitPos = rBI->nextCharBlock(rLHS, nLHSLastNonDigitPos,
            rLocale, i18n::CharType::DECIMAL_DIGIT_NUMBER);
        nRHSFirstDigitPos = rBI->nextCharBlock(rRHS, nRHSLastNonDigitPos,
            rLocale, i18n::CharType::DECIMAL_DIGIT_NUMBER);
        if (nLHSFirstDigitPos == -1)
            nLHSFirstDigitPos = rLHS.getLength();
        if (nRHSFirstDigitPos == -1)
            nRHSFirstDigitPos = rRHS.getLength();
        nLHSChunkLen = nLHSFirstDigitPos - nLHSLastNonDigitPos;
        nRHSChunkLen = nRHSFirstDigitPos - nRHSLastNonDigitPos;

        nRet = rCollator->compareSubstring(rLHS, nLHSLastNonDigitPos,
            nLHSChunkLen, rRHS, nRHSLastNonDigitPos, nRHSChunkLen);
        if (nRet != 0)
            break;

        //Compare digit block as one number vs another
        nLHSLastNonDigitPos = rBI->endOfCharBlock(rLHS, nLHSFirstDigitPos,
            rLocale, i18n::CharType::DECIMAL_DIGIT_NUMBER);
        nRHSLastNonDigitPos = rBI->endOfCharBlock(rRHS, nRHSFirstDigitPos,
            rLocale, i18n::CharType::DECIMAL_DIGIT_NUMBER);
        if (nLHSLastNonDigitPos == -1)
            nLHSLastNonDigitPos = rLHS.getLength();
        if (nRHSLastNonDigitPos == -1)
            nRHSLastNonDigitPos = rRHS.getLength();
        nLHSChunkLen = nLHSLastNonDigitPos - nLHSFirstDigitPos;
        nRHSChunkLen = nRHSLastNonDigitPos - nRHSFirstDigitPos;

        //To-Do: Possibly scale down those unicode codepoints that relate to
        //numbers outside of the normal 0-9 range, e.g. see GetLocalizedChar in
        //vcl

        sal_uInt32 nLHS = comphelper::string::decimalStringToNumber(rLHS.copy(nLHSFirstDigitPos, nLHSChunkLen));
        sal_uInt32 nRHS = comphelper::string::decimalStringToNumber(rRHS.copy(nRHSFirstDigitPos, nRHSChunkLen));

        nRet = nLHS-nRHS;
        if (nRet != 0)
            break;
    }

    //Squeeze these down to -1, 0, 1 in case it gets casted to a StringCompare
    if (nRet > 0)
        nRet = 1;
    else if (nRet < 0)
        nRet = -1;

    return nRet;
}

NaturalStringSorter::NaturalStringSorter(
    const uno::Reference< uno::XComponentContext > &rContext,
    const lang::Locale &rLocale) : m_aLocale(rLocale)
{
    uno::Reference< lang::XMultiComponentFactory > xFactory(rContext->getServiceManager(),
        uno::UNO_SET_THROW);

    m_xCollator = uno::Reference< i18n::XCollator >(xFactory->createInstanceWithContext(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.i18n.Collator")), rContext),
            uno::UNO_QUERY_THROW);
    m_xCollator->loadDefaultCollator(m_aLocale, 0);
    m_xBI = i18n::BreakIterator::create( rContext );
}

namespace
{
    //do OPER on each element of the string, return false
    //if any OPER is false, true otherwise
    template <bool (*OPER)(sal_Unicode), typename T>
    bool tmpl_is_OPER_AsciiString(const T &rString)
    {
        for (sal_Int32 i = 0; i < rString.getLength(); ++i)
        {
            if (!OPER(rString[i]))
                return false;
        }
        return true;
    }
}

bool isdigitAsciiString(const rtl::OString &rString)
{
    return tmpl_is_OPER_AsciiString<isdigitAscii>(rString);
}

bool isdigitAsciiString(const rtl::OUString &rString)
{
    return tmpl_is_OPER_AsciiString<isdigitAscii>(rString);
}

namespace
{
    template <typename T, typename U> T* string_alloc(sal_Int32 nLen)
    {
        //Clearly this is somewhat cosy with the sal implmentation

        //rtl_[u]String contains U buffer[1], so an input of nLen
        //allocates a buffer of nLen + 1 and we'll ensure a null termination

        T* newStr =
            (sal::static_int_cast< sal_uInt32 >(nLen)
               <= ((SAL_MAX_UINT32 - sizeof (T))
                   / sizeof (U)))
            ? (T*) rtl_allocateMemory(
                sizeof (T) + nLen * sizeof (U))
            : NULL;

        if (!newStr)
            throw std::bad_alloc();

        newStr->refCount = 1;
        newStr->length = nLen;
        newStr->buffer[nLen] = 0;
        return newStr;
    }
}

rtl_uString * SAL_CALL rtl_uString_alloc(sal_Int32 nLen)
{
    return string_alloc<rtl_uString, sal_Unicode>(nLen);
}

rtl_String * SAL_CALL rtl_string_alloc(sal_Int32 nLen)
{
    return string_alloc<rtl_String, sal_Char>(nLen);
}

namespace
{
    template <typename T, typename O> T tmpl_reverseString(const T &rIn)
    {
        if (rIn.isEmpty())
            return rIn;

        sal_Int32 i = rIn.getLength();
        O sBuf(i);
        while (i)
            sBuf.append(rIn[--i]);
        return sBuf.makeStringAndClear();
    }
}

rtl::OUString reverseString(const rtl::OUString &rStr)
{
    return tmpl_reverseString<rtl::OUString, rtl::OUStringBuffer>(rStr);
}

rtl::OString reverseString(const rtl::OString &rStr)
{
    return tmpl_reverseString<rtl::OString, rtl::OStringBuffer>(rStr);
}


} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
