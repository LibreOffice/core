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

#include "oox/token/tokenmap.hxx"

#include <string.h>
#include <rtl/strbuf.hxx>
#include <rtl/string.hxx>
#include "oox/token/tokens.hxx"

namespace oox {

using ::com::sun::star::uno::Sequence;

namespace {
// include auto-generated Perfect_Hash
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
}

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
        *aIt = Sequence< sal_Int8 >( reinterpret_cast< const sal_Int8* >( aUtf8Token.getStr() ), aUtf8Token.getLength() );
    }

    for (unsigned char c = 'a'; c <= 'z'; c++)
    {
        const struct xmltoken* pToken = Perfect_Hash::in_word_set(
                reinterpret_cast< const char* >( &c ), 1 );
        mnAlphaTokens[ c - 'a' ] = pToken ? pToken->nToken : XML_TOKEN_INVALID;
    }
}

TokenMap::~TokenMap()
{
}

sal_Int32 TokenMap::getTokenFromUnicode( const OUString& rUnicodeName )
{
    OString aUtf8Name = OUStringToOString( rUnicodeName, RTL_TEXTENCODING_UTF8 );
    const struct xmltoken* pToken = Perfect_Hash::in_word_set( aUtf8Name.getStr(), aUtf8Name.getLength() );
    return pToken ? pToken->nToken : XML_TOKEN_INVALID;
}

sal_Int32 TokenMap::getTokenPerfectHash( const char *pStr, sal_Int32 nLength )
{
    const struct xmltoken* pToken = Perfect_Hash::in_word_set( pStr, nLength );
    return pToken ? pToken->nToken : XML_TOKEN_INVALID;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
