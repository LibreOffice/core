/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tokenmap.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:54:28 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include <string.h>
#include <osl/mutex.hxx>
#include <rtl/strbuf.hxx>
#include <com/sun/star/xml/sax/FastToken.hpp>
#include "oox/core/fasttokenhandler.hxx"
#include "tokens.hxx"

using ::rtl::OString;
using ::rtl::OStringBuffer;
using ::rtl::OUString;
using ::rtl::OUStringToOString;
using ::osl::Mutex;
using ::osl::MutexGuard;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::xml::sax::FastToken::DONTKNOW;

namespace oox {

#include "tokens.inc"
#include "tokenwords.inc"

// ============================================================================

namespace {

Mutex& lclGetTokenMutex()
{
    static Mutex aMutex;
    return aMutex;
}

} // namespace

// ============================================================================

FastTokenHandler::FastTokenHandler()
{
#if OSL_DEBUG_LEVEL > 0
    MutexGuard aGuard( lclGetTokenMutex() );
    bool bOk = true;
    for( sal_Int32 nIdx = 0; bOk && (nIdx < XML_TOKEN_COUNT); ++nIdx )
    {
        // check that the getIdentifier <-> getToken roundtrip works
        OUString aToken = getIdentifier( nIdx );
        bOk = getToken( aToken ) == nIdx;
        OSL_ENSURE( bOk, OStringBuffer( "FastTokenHandler::FastTokenHandler - token list broken, #" ).
            append( nIdx ).append( ", '" ).
            append( OUStringToOString( aToken, RTL_TEXTENCODING_ASCII_US ) ).append( '\'' ).getStr() );
    }
#endif
}

FastTokenHandler::~FastTokenHandler()
{
}

sal_Int32 FastTokenHandler::getToken( const OUString& rIdentifier ) throw( RuntimeException )
{
    MutexGuard aGuard( lclGetTokenMutex() );

    OString aUTF8 = OUStringToOString( rIdentifier, RTL_TEXTENCODING_UTF8 );

    struct xmltoken * t = Perfect_Hash::in_word_set( aUTF8.getStr(), aUTF8.getLength() );
    return t ? t->nToken : DONTKNOW;
}

OUString FastTokenHandler::getIdentifier( sal_Int32 nToken ) throw( RuntimeException )
{
    MutexGuard aGuard( lclGetTokenMutex() );

    if( nToken >= XML_TOKEN_COUNT )
        return OUString();

    static OUString aTokens[XML_TOKEN_COUNT];

    if( aTokens[nToken].getLength() == 0 )
        aTokens[nToken] = OUString::createFromAscii( tokentowordlist[nToken] );

    return aTokens[nToken];
}

sal_Int32 FastTokenHandler::getTokenFromUTF8( const Sequence< sal_Int8 >& rIdentifier ) throw( RuntimeException )
{
    MutexGuard aGuard( lclGetTokenMutex() );

    struct xmltoken * t = Perfect_Hash::in_word_set( reinterpret_cast< const char* >( rIdentifier.getConstArray() ), rIdentifier.getLength());
    return t ? t->nToken : DONTKNOW;
}

// ============================================================================

} // namespace oox

