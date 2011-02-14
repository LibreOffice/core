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

#include "precompiled_comphelper.hxx"
#include "sal/config.h"

#include <cstddef>
#include <string.h>
#include <vector>
#include <algorithm>

#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/types.h>

#include <comphelper/string.hxx>
#include <comphelper/stlunosequence.hxx>
#include <comphelper/stl_types.hxx>

#include <com/sun/star/i18n/CharType.hpp>


namespace comphelper { namespace string {

rtl::OUString searchAndReplaceAsciiL(
    rtl::OUString const & source, char const * from, sal_Int32 fromLength,
    rtl::OUString const & to, sal_Int32 beginAt, sal_Int32 * replacedAt)
{
    sal_Int32 n = source.indexOfAsciiL(from, fromLength, beginAt);
    if (replacedAt != NULL) {
        *replacedAt = n;
    }
    return n == -1 ? source : source.replaceAt(n, fromLength, to);
}

::rtl::OUString searchAndReplaceAllAsciiWithAscii(
    const ::rtl::OUString& _source, const sal_Char* _from, const sal_Char* _to,
    const sal_Int32 _beginAt )
{
    sal_Int32 fromLength = strlen( _from );
    sal_Int32 n = _source.indexOfAsciiL( _from, fromLength, _beginAt );
    if ( n == -1 )
        return _source;

    ::rtl::OUString dest( _source );
    ::rtl::OUString to( ::rtl::OUString::createFromAscii( _to ) );
    do
    {
        dest = dest.replaceAt( n, fromLength, to );
        n = dest.indexOfAsciiL( _from, fromLength, n + to.getLength() );
    }
    while ( n != -1 );

    return dest;
}

::rtl::OUString& searchAndReplaceAsciiI(
    ::rtl::OUString & _source, sal_Char const * _asciiPattern, ::rtl::OUString const & _replace,
    sal_Int32 _beginAt, sal_Int32 * _replacedAt )
{
    sal_Int32 fromLength = strlen( _asciiPattern );
    sal_Int32 n = _source.indexOfAsciiL( _asciiPattern, fromLength, _beginAt );
    if ( _replacedAt != NULL )
        *_replacedAt = n;

    if ( n != -1 )
        _source = _source.replaceAt( n, fromLength, _replace );

    return _source;
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
      if (kw.getLength() > 0) {
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
    const lang::Locale &rLocale) SAL_THROW(())
{
    sal_Int16 nRet = 0;

    sal_Int16 nLHSLastNonDigitPos = 0;
    sal_Int16 nRHSLastNonDigitPos = 0;
    sal_Int16 nLHSFirstDigitPos = 0;
    sal_Int16 nRHSFirstDigitPos = 0;

    while (nLHSFirstDigitPos < rLHS.getLength() || nRHSFirstDigitPos < rRHS.getLength())
    {
        sal_Int16 nLHSChunkLen;
        sal_Int16 nRHSChunkLen;

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
            return nRet;

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
        sal_Int32 nLHS = rLHS.copy(nLHSFirstDigitPos, nLHSChunkLen).toInt32();
        sal_Int32 nRHS = rRHS.copy(nRHSFirstDigitPos, nRHSChunkLen).toInt32();

        nRet = nLHS-nRHS;
        if (nRet != 0)
            return nRet;
    }

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
    m_xBI = uno::Reference< i18n::XBreakIterator >(xFactory->createInstanceWithContext(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.i18n.BreakIterator")), rContext),
            uno::UNO_QUERY_THROW);
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
