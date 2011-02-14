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

// convert between sequence of string and comma separated string

::rtl::OUString convertCommaSeparated(
    ::com::sun::star::uno::Sequence< ::rtl::OUString > const& i_rSeq)
{
    ::rtl::OUStringBuffer buf;
    ::comphelper::intersperse(
        ::comphelper::stl_begin(i_rSeq), ::comphelper::stl_end(i_rSeq),
        ::comphelper::OUStringBufferAppender(buf),
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ", " )));
    return buf.makeStringAndClear();
}

::com::sun::star::uno::Sequence< ::rtl::OUString >
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
    ::com::sun::star::uno::Sequence< ::rtl::OUString > kws(vec.size());
    std::copy(vec.begin(), vec.end(), stl_begin(kws));
    return kws;
}

#define IS_DIGIT(CHAR) (((CHAR) >= 48) && ((CHAR <= 57)))

template<typename IMPL_RTL_STRCODE, typename IMPL_RTL_USTRCODE>
    sal_Int32 SAL_CALL compareNaturalImpl(const IMPL_RTL_STRCODE* pStr1, const IMPL_RTL_STRCODE* pStr2)
{
    sal_Int32 nRet;
    do {
        while ( ((nRet = ((sal_Int32)(IMPL_RTL_USTRCODE(*pStr1)))-
                         ((sal_Int32)(IMPL_RTL_USTRCODE(*pStr2)))) == 0) &&
                *pStr2 )
        {
            pStr1++;
            pStr2++;
        }

        if(*pStr1 && *pStr2)
        {
            IMPL_RTL_STRCODE   c1 = (sal_Int32)IMPL_RTL_USTRCODE( *pStr1 );
            IMPL_RTL_STRCODE   c2 = (sal_Int32)IMPL_RTL_USTRCODE( *pStr2 );
            sal_Int64   number1 = 0;
            sal_Int64   number2 = 0;
            if(IS_DIGIT(c1) && IS_DIGIT(c2))
            {
              do
              {
                number1 = number1 * 10 + (c1 - '0');
                pStr1++;
                c1 = (sal_Int32)IMPL_RTL_USTRCODE( *pStr1 );
              } while(IS_DIGIT(c1));

              do
              {
                number2 = number2 * 10 + (c2 - '0');
                pStr2++;
                c2 = (sal_Int32)IMPL_RTL_USTRCODE( *pStr2 );
              } while(IS_DIGIT(c2));

              nRet = number1 - number2;
            }
        }
    } while(nRet == 0 && *pStr1 && *pStr2);

    return nRet;
}

sal_Int32 compareNatural( const ::rtl::OUString & rLHS, const ::rtl::OUString & rRHS ) SAL_THROW(())
{
    return compareNaturalImpl<sal_Unicode, sal_Unicode>(rLHS.pData->buffer, rRHS.pData->buffer);
}

sal_Int32 compareNatural( const ::rtl::OString & rLHS, const ::rtl::OString & rRHS ) SAL_THROW(())
{
    return compareNaturalImpl<sal_Char, unsigned char>(rLHS.pData->buffer, rRHS.pData->buffer);
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
