/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: tokenmap.cxx,v $
 * $Revision: 1.4 $
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
#include <rtl/strbuf.hxx>
#include <rtl/string.hxx>
#include "tokens.hxx"
#include "oox/helper/containerhelper.hxx"

using ::rtl::OString;
using ::rtl::OUString;
using ::com::sun::star::uno::Sequence;

namespace oox {

// ============================================================================

namespace {

// include auto-generated token lists
#include "tokens.inc"
#include "tokenwords.inc"

} // namespace

// ============================================================================

TokenMap::TokenMap() :
    maTokenNames( static_cast< size_t >( XML_TOKEN_COUNT ) )
{
    const sal_Char* const* ppcTokenWord = xmltokenwordlist;
    for( TokenNameVector::iterator aIt = maTokenNames.begin(), aEnd = maTokenNames.end(); aIt != aEnd; ++aIt, ++ppcTokenWord )
    {
        OString aUtf8Token( *ppcTokenWord );
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
        OSL_ENSURE( bOk, ::rtl::OStringBuffer( "FastTokenHandler::FastTokenHandler - token list broken, #" ).
            append( nToken ).append( ", '" ).append( aUtf8Name ).append( '\'' ).getStr() );
    }
#endif
}

TokenMap::~TokenMap()
{
}

OUString TokenMap::getUnicodeTokenName( sal_Int32 nToken ) const
{
    const TokenName* pTokenName = ContainerHelper::getVectorElement( maTokenNames, nToken );
    return pTokenName ? pTokenName->maUniName : OUString();
}

sal_Int32 TokenMap::getTokenFromUnicode( const OUString& rUnicodeName ) const
{
    OString aUtf8Name = OUStringToOString( rUnicodeName, RTL_TEXTENCODING_UTF8 );
    struct xmltoken* pToken = Perfect_Hash::in_word_set( aUtf8Name.getStr(), aUtf8Name.getLength() );
    return pToken ? pToken->nToken : XML_TOKEN_INVALID;
}

Sequence< sal_Int8 > TokenMap::getUtf8TokenName( sal_Int32 nToken ) const
{
    const TokenName* pTokenName = ContainerHelper::getVectorElement( maTokenNames, nToken );
    return pTokenName ? pTokenName->maUtf8Name : Sequence< sal_Int8 >();
}

sal_Int32 TokenMap::getTokenFromUtf8( const Sequence< sal_Int8 >& rUtf8Name ) const
{
    struct xmltoken* pToken = Perfect_Hash::in_word_set(
        reinterpret_cast< const char* >( rUtf8Name.getConstArray() ), rUtf8Name.getLength() );
    return pToken ? pToken->nToken : XML_TOKEN_INVALID;
}

// ============================================================================

} // namespace oox

