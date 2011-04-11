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

#include "oox/token/tokenmap.hxx"

#include <string.h>
#include <rtl/strbuf.hxx>
#include <rtl/string.hxx>
#include "oox/token/tokens.hxx"

namespace oox {
// ============================================================================

using ::com::sun::star::uno::Sequence;
using ::rtl::OString;
using ::rtl::OUString;

// ============================================================================

namespace {
// include auto-generated Perfect_Hash
#include "tokenhash.inc"
} // namespace

// ============================================================================

TokenMap::TokenMap() :
    maTokenNames( static_cast< size_t >( XML_TOKEN_COUNT ) )
{
    static const sal_Char* sppcTokenNames[] =
    {
// include auto-generated C array with token names as C strings
#include "tokennames.inc"
        ""
    };

    const sal_Char* const* ppcTokenName = sppcTokenNames;
    for( TokenNameVector::iterator aIt = maTokenNames.begin(), aEnd = maTokenNames.end(); aIt != aEnd; ++aIt, ++ppcTokenName )
    {
        OString aUtf8Token( *ppcTokenName );
        aIt->maUniName = OStringToOUString( aUtf8Token, RTL_TEXTENCODING_UTF8 );
        aIt->maUtf8Name = Sequence< sal_Int8 >( reinterpret_cast< const sal_Int8* >( aUtf8Token.getStr() ), aUtf8Token.getLength() );
    }

#if OSL_DEBUG_LEVEL > 0
    // check that the perfect_hash is in sync with the token name list
    bool bOk = true;
    for( sal_Int32 nToken = 0; bOk && (nToken < XML_TOKEN_COUNT); ++nToken )
    {
        // check that the getIdentifier <-> getToken roundtrip works
        OString aUtf8Name = OUStringToOString( maTokenNames[ nToken ].maUniName, RTL_TEXTENCODING_UTF8 );
        struct xmltoken* pToken = Perfect_Hash::in_word_set( aUtf8Name.getStr(), aUtf8Name.getLength() );
        bOk = pToken && (pToken->nToken == nToken);
        OSL_ENSURE( bOk, ::rtl::OStringBuffer( "TokenMap::TokenMap - token list broken, #" ).
            append( nToken ).append( ", '" ).append( aUtf8Name ).append( '\'' ).getStr() );
    }
#endif
}

TokenMap::~TokenMap()
{
}

OUString TokenMap::getUnicodeTokenName( sal_Int32 nToken ) const
{
    if( (0 <= nToken) && (static_cast< size_t >( nToken ) < maTokenNames.size()) )
        return maTokenNames[ static_cast< size_t >( nToken ) ].maUniName;
    return OUString();
}

sal_Int32 TokenMap::getTokenFromUnicode( const OUString& rUnicodeName ) const
{
    OString aUtf8Name = OUStringToOString( rUnicodeName, RTL_TEXTENCODING_UTF8 );
    struct xmltoken* pToken = Perfect_Hash::in_word_set( aUtf8Name.getStr(), aUtf8Name.getLength() );
    return pToken ? pToken->nToken : XML_TOKEN_INVALID;
}

Sequence< sal_Int8 > TokenMap::getUtf8TokenName( sal_Int32 nToken ) const
{
    if( (0 <= nToken) && (static_cast< size_t >( nToken ) < maTokenNames.size()) )
        return maTokenNames[ static_cast< size_t >( nToken ) ].maUtf8Name;
    return Sequence< sal_Int8 >();
}

sal_Int32 TokenMap::getTokenFromUtf8( const Sequence< sal_Int8 >& rUtf8Name ) const
{
    struct xmltoken* pToken = Perfect_Hash::in_word_set(
        reinterpret_cast< const char* >( rUtf8Name.getConstArray() ), rUtf8Name.getLength() );
    return pToken ? pToken->nToken : XML_TOKEN_INVALID;
}

// ============================================================================

} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */