/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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
#include <comphelper/stl_types.hxx>

#include <com/sun/star/i18n/BreakIterator.hpp>
#include <com/sun/star/i18n/CharType.hpp>
#include <com/sun/star/i18n/Collator.hpp>


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

OString stripStart(const OString &rIn, sal_Char c)
{
    return tmpl_stripStart<OString, sal_Char>(rIn, c);
}

OUString stripStart(const OUString &rIn, sal_Unicode c)
{
    return tmpl_stripStart<OUString, sal_Unicode>(rIn, c);
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

OString stripEnd(const OString &rIn, sal_Char c)
{
    return tmpl_stripEnd<OString, sal_Char>(rIn, c);
}

OUString stripEnd(const OUString &rIn, sal_Unicode c)
{
    return tmpl_stripEnd<OUString, sal_Unicode>(rIn, c);
}

OString strip(const OString &rIn, sal_Char c)
{
    return stripEnd(stripStart(rIn, c), c);
}

OUString strip(const OUString &rIn, sal_Unicode c)
{
    return stripEnd(stripStart(rIn, c), c);
}

namespace
{
    template <typename T, typename C> sal_Int32 tmpl_getTokenCount(const T &rIn,
        C cTok)
    {
        
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

sal_Int32 getTokenCount(const OString &rIn, sal_Char cTok)
{
    return tmpl_getTokenCount<OString, sal_Char>(rIn, cTok);
}

sal_Int32 getTokenCount(const OUString &rIn, sal_Unicode cTok)
{
    return tmpl_getTokenCount<OUString, sal_Unicode>(rIn, cTok);
}

sal_uInt32 decimalStringToNumber(
    OUString const & str )
{
    sal_uInt32 result = 0;
    for( sal_Int32 i = 0 ; i < str.getLength() ; )
    {
        sal_uInt32 c = str.iterateCodePoints(&i);
        sal_uInt32 value = 0;
        if( c <= 0x0039)    
            value = c - 0x0030;
        else if( c >= 0x1D7F6 )    
            value = c - 0x1D7F6;
        else if( c >= 0x1D7EC ) 
            value = c - 0x1D7EC;
        else if( c >= 0x1D7E2 ) 
            value = c - 0x1D7E2;
        else if( c >= 0x1D7D8 ) 
            value = c - 0x1D7D8;
        else if( c >= 0x1D7CE ) 
            value = c - 0x1D7CE;
        else if( c >= 0x11066 ) 
            value = c - 0x11066;
        else if( c >= 0x104A0 ) 
            value = c - 0x104A0;
        else if( c >= 0xFF10 ) 
            value = c - 0xFF10;
        else if( c >= 0xABF0 ) 
            value = c - 0xABF0;
        else if( c >= 0xAA50 ) 
            value = c - 0xAA50;
        else if( c >= 0xA9D0 ) 
            value = c - 0xA9D0;
        else if( c >= 0xA900 ) 
            value = c - 0xA900;
        else if( c >= 0xA8D0 ) 
            value = c - 0xA8D0;
        else if( c >= 0xA620 ) 
            value = c - 0xA620;
        else if( c >= 0x1C50 ) 
            value = c - 0x1C50;
        else if( c >= 0x1C40 ) 
            value = c - 0x1C40;
        else if( c >= 0x1BB0 ) 
            value = c - 0x1BB0;
        else if( c >= 0x1B50 ) 
            value = c - 0x1B50;
        else if( c >= 0x1A90 ) 
            value = c - 0x1A90;
        else if( c >= 0x1A80 ) 
            value = c - 0x1A80;
        else if( c >= 0x19D0 ) 
            value = c - 0x19D0;
        else if( c >= 0x1946 ) 
            value = c - 0x1946;
        else if( c >= 0x1810 ) 
            value = c - 0x1810;
        else if( c >= 0x17E0 ) 
            value = c - 0x17E0;
        else if( c >= 0x1090 ) 
            value = c - 0x1090;
        else if( c >= 0x1040 ) 
            value = c - 0x1040;
        else if( c >= 0x0F20 ) 
            value = c - 0x0F20;
        else if( c >= 0x0ED0 ) 
            value = c - 0x0ED0;
        else if( c >= 0x0E50 ) 
            value = c - 0x0E50;
        else if( c >= 0x0D66 ) 
            value = c - 0x0D66;
        else if( c >= 0x0CE6 ) 
            value = c - 0x0CE6;
        else if( c >= 0x0C66 ) 
            value = c - 0x0C66;
        else if( c >= 0x0BE6 ) 
            value = c - 0x0BE6;
        else if( c >= 0x0B66 ) 
            value = c - 0x0B66;
        else if( c >= 0x0AE6 ) 
            value = c - 0x0AE6;
        else if( c >= 0x0A66 ) 
            value = c - 0x0A66;
        else if( c >= 0x09E6 ) 
            value = c - 0x09E6;
        else if( c >= 0x0966 ) 
            value = c - 0x0966;
        else if( c >= 0x07C0 ) 
            value = c - 0x07C0;
        else if( c >= 0x06F0 ) 
            value = c - 0x06F0;
        else if( c >= 0x0660 ) 
            value = c - 0x0660;
        result = result * 10 + value;
    }
    return result;
}

using namespace ::com::sun::star;



OUString convertCommaSeparated(
    uno::Sequence< OUString > const& i_rSeq)
{
    OUStringBuffer buf;
    ::comphelper::intersperse(
        i_rSeq.begin(), i_rSeq.end(), ::comphelper::OUStringBufferAppender(buf), OUString( ", " ));
    return buf.makeStringAndClear();
}

uno::Sequence< OUString >
    convertCommaSeparated( OUString const& i_rString )
{
    std::vector< OUString > vec;
    sal_Int32 idx = 0;
    do {
      OUString kw =
        i_rString.getToken(0, static_cast<sal_Unicode> (','), idx);
      kw = kw.trim();
      if (!kw.isEmpty()) {
          vec.push_back(kw);
      }
    } while (idx >= 0);
    uno::Sequence< OUString > kws(vec.size());
    std::copy(vec.begin(), vec.end(), kws.begin());
    return kws;
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

    while (nLHSFirstDigitPos < rLHS.getLength() || nRHSFirstDigitPos < rRHS.getLength())
    {
        sal_Int32 nLHSChunkLen;
        sal_Int32 nRHSChunkLen;

        
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

        
        
        

        sal_uInt32 nLHS = comphelper::string::decimalStringToNumber(rLHS.copy(nLHSFirstDigitPos, nLHSChunkLen));
        sal_uInt32 nRHS = comphelper::string::decimalStringToNumber(rRHS.copy(nRHSFirstDigitPos, nRHSChunkLen));

        nRet = nLHS-nRHS;
        if (nRet != 0)
            break;
    }

    
    
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
    m_xCollator = i18n::Collator::create( rContext );
    m_xCollator->loadDefaultCollator(m_aLocale, 0);
    m_xBI = i18n::BreakIterator::create( rContext );
}

namespace
{
    
    
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

bool isdigitAsciiString(const OString &rString)
{
    return tmpl_is_OPER_AsciiString<isdigitAscii>(rString);
}

bool isdigitAsciiString(const OUString &rString)
{
    return tmpl_is_OPER_AsciiString<isdigitAscii>(rString);
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

OUString reverseString(const OUString &rStr)
{
    return tmpl_reverseString<OUString, OUStringBuffer>(rStr);
}

OString reverseString(const OString &rStr)
{
    return tmpl_reverseString<OString, OStringBuffer>(rStr);
}

sal_Int32 indexOfAny(OUString const& rIn,
        sal_Unicode const*const pChars, sal_Int32 const nPos)
{
    for (sal_Int32 i = nPos; i < rIn.getLength(); ++i)
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

OUString setToken(const OUString& rIn, sal_Int32 nToken, sal_Unicode cTok,
    const OUString& rNewToken)
{
    const sal_Unicode* pStr = rIn.getStr();
    sal_Int32 nLen = rIn.getLength();
    sal_Int32 nTok = 0;
    sal_Int32 nFirstChar = 0;
    sal_Int32 i = 0;

    
    while ( i < nLen )
    {
        
        if (*pStr == cTok)
        {
            ++nTok;

            if (nTok == nToken)
                nFirstChar = i+1;
            else if (nTok > nToken)
                break;
        }

        ++pStr,
        ++i;
    }

    if (nTok >= nToken)
        return rIn.replaceAt(nFirstChar, i-nFirstChar, rNewToken);
    return rIn;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
