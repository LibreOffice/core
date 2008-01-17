/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tokenmap.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:06:07 $
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

#include <com/sun/star/xml/sax/FastToken.hpp>

#include "oox/core/fasttokenhandler.hxx"
#include "tokens.hxx"

using ::rtl::OUString;
using ::osl::Mutex;
using ::osl::MutexGuard;
using namespace ::com::sun::star::xml::sax;

#ifdef WNT
#pragma warning(disable:4129)
#endif

namespace oox
{

#include "tokens.cxx"

Mutex& getTokenMutex()
{
    static Mutex aMutex;
    return aMutex;
}

::sal_Int32 FastTokenHandler::getToken( const ::rtl::OUString& Identifier ) throw (::com::sun::star::uno::RuntimeException)
{
    MutexGuard guard( getTokenMutex() );

    rtl::OString aUTF8( Identifier.getStr(), Identifier.getLength(), RTL_TEXTENCODING_UTF8 );

    struct xmltoken * t = Perfect_Hash::in_word_set( aUTF8, aUTF8.getLength() );
    if( t )
        return t->nToken;
    else
        return FastToken::DONTKNOW;
}

::rtl::OUString FastTokenHandler::getIdentifier( ::sal_Int32 nToken ) throw (::com::sun::star::uno::RuntimeException)
{
    MutexGuard guard( getTokenMutex() );

    if( nToken >= XML_TOKEN_COUNT )
        return OUString();

    static OUString aTokens[XML_TOKEN_COUNT];

    if( aTokens[nToken].getLength() == 0 )
        aTokens[nToken] = OUString::createFromAscii(wordlist[nToken].name);

    return aTokens[nToken];
}

::sal_Int32 FastTokenHandler::getTokenFromUTF8( const ::com::sun::star::uno::Sequence< ::sal_Int8 >& Identifier ) throw (::com::sun::star::uno::RuntimeException)
{
    MutexGuard guard( getTokenMutex() );

    struct xmltoken * t = Perfect_Hash::in_word_set((const char*)Identifier.getConstArray(), Identifier.getLength());
    if( t )
        return t->nToken;
    else
        return FastToken::DONTKNOW;
}

}


