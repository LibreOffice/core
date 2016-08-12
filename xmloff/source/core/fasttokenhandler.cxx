/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xmloff/fasttokenhandler.hxx"

#include <xmloff/token/tokens.hxx>

namespace xmloff {

namespace {
#if defined __clang__
#if __has_warning("-Wdeprecated-register")
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-register"
#endif
#endif
#include "tokenhash.inc"
#if defined __clang__
#if __has_warning("-Wdeprecated-register")
#pragma GCC diagnostic pop
#endif
#endif
} // namespace

namespace token {

using namespace css;

TokenMap::TokenMap() :
    maTokenNames( static_cast< size_t >( XML_TOKEN_COUNT ) )
{
    static const sal_Char* sppcTokenNames[] =
    {
#include "tokennames.inc"
        ""
    };

    const sal_Char* const* ppcTokenName = sppcTokenNames;
    for( std::vector< uno::Sequence< sal_Int8 > >::iterator aIt = maTokenNames.begin(), aEnd = maTokenNames.end();
            aIt != aEnd; ++aIt, ++ppcTokenName )
    {
        OString aUtf8Token( *ppcTokenName );
        *aIt = uno::Sequence< sal_Int8 >( reinterpret_cast< const sal_Int8* >(
                    aUtf8Token.getStr() ), aUtf8Token.getLength() );
    }
}

TokenMap::~TokenMap()
{
}

sal_Int32 TokenMap::getTokenFromUnicode( const OUString& rUnicodeName ) const
{
    OString aUtf8Name = OUStringToOString( rUnicodeName, RTL_TEXTENCODING_UTF8 );
    const struct xmltoken* pToken = Perfect_Hash::in_word_set( aUtf8Name.getStr(), aUtf8Name.getLength() );
    return pToken ? pToken->nToken : XML_TOKEN_INVALID;
}

sal_Int32 TokenMap::getTokenPerfectHash( const char *pStr, sal_Int32 nLength ) const
{
    const struct xmltoken *pToken = Perfect_Hash::in_word_set( pStr, nLength );
    return pToken ? pToken->nToken : XML_TOKEN_INVALID;
}

FastTokenHandler::FastTokenHandler() :
    mrTokenMap( StaticTokenMap::get() )
{
}

FastTokenHandler::~FastTokenHandler()
{
}

// XFastTokenHandler
uno::Sequence< sal_Int8 > FastTokenHandler::getUTF8Identifier( sal_Int32 nToken )
    throw (uno::RuntimeException, std::exception)
{
    return mrTokenMap.getUtf8TokenName( nToken );
}

sal_Int32 FastTokenHandler::getTokenFromUTF8( const uno::Sequence< sal_Int8 >& rIdentifier )
    throw (uno::RuntimeException, std::exception)
{
    return mrTokenMap.getTokenFromUtf8( rIdentifier );
}

// Much faster direct C++ shortcut
sal_Int32 FastTokenHandler::getTokenDirect( const char* pToken, sal_Int32 nLength ) const
{
    return mrTokenMap.getTokenFromUTF8( pToken, nLength );
}

} // namespace token
} // namespace xmloff

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
