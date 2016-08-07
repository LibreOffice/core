/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

// include auto-generated Perfect_Hash class
#include <token/tokenhash.inc>

} // namespace

// ============================================================================

TokenMap::TokenMap() :
    maTokenNames( static_cast< size_t >( XML_TOKEN_COUNT ) )
{
    static const sal_Char* sppcTokenNames[] =
    {
// include auto-generated C array with token names as C strings
#include <token/tokennames.inc>
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
    // check that the Perfect_Hash is in sync with the token name list
    bool bOk = true;
    for( sal_Int32 nToken = 0; bOk && (nToken < XML_TOKEN_COUNT); ++nToken )
    {
        // check that the getIdentifier <-> getToken roundtrip works
        OString aUtf8Name = OUStringToOString( maTokenNames[ nToken ].maUniName, RTL_TEXTENCODING_UTF8 );
        const XMLTokenInfo* pTokenInfo = Perfect_Hash::getTokenInfo( aUtf8Name.getStr(), aUtf8Name.getLength() );
        bOk = pTokenInfo && (pTokenInfo->mnToken == nToken);
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
    const XMLTokenInfo* pTokenInfo = Perfect_Hash::getTokenInfo( aUtf8Name.getStr(), aUtf8Name.getLength() );
    return pTokenInfo ? pTokenInfo->mnToken : XML_TOKEN_INVALID;
}

Sequence< sal_Int8 > TokenMap::getUtf8TokenName( sal_Int32 nToken ) const
{
    if( (0 <= nToken) && (static_cast< size_t >( nToken ) < maTokenNames.size()) )
        return maTokenNames[ static_cast< size_t >( nToken ) ].maUtf8Name;
    return Sequence< sal_Int8 >();
}

sal_Int32 TokenMap::getTokenFromUtf8( const Sequence< sal_Int8 >& rUtf8Name ) const
{
    const XMLTokenInfo* pTokenInfo = Perfect_Hash::getTokenInfo(
        reinterpret_cast< const char* >( rUtf8Name.getConstArray() ), rUtf8Name.getLength() );
    return pTokenInfo ? pTokenInfo->mnToken : XML_TOKEN_INVALID;
}

// ============================================================================

} // namespace oox
