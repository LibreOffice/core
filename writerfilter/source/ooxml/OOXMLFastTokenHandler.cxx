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

#include <iostream>
#include <string.h>
#include <ooxml/resourceids.hxx>
#include "OOXMLFastTokenHandler.hxx"
#include "gperffasttoken.hxx"

namespace writerfilter {
namespace ooxml
{

using namespace ::std;

OOXMLFastTokenHandler::OOXMLFastTokenHandler
(css::uno::Reference< css::uno::XComponentContext > const & context) 
: m_xContext(context)
{}

// ::com::sun::star::xml::sax::XFastTokenHandler:
::sal_Int32 SAL_CALL OOXMLFastTokenHandler::getToken(const ::rtl::OUString & Identifier) 
    throw (css::uno::RuntimeException)
{        
    ::sal_Int32 nResult = OOXML_FAST_TOKENS_END;

    struct tokenmap::token * pToken =
        tokenmap::Perfect_Hash::in_word_set
        (OUStringToOString(Identifier, RTL_TEXTENCODING_ASCII_US).getStr(), 
         Identifier.getLength());

    if (pToken != NULL)
        nResult = pToken->nToken;

#ifdef DEBUG_TOKEN
    clog << "getToken: " 
         << OUStringToOString(Identifier, RTL_TEXTENCODING_ASCII_US).getStr() 
         << ", " << nResult
         << endl;
#endif

    return nResult;
}

::rtl::OUString SAL_CALL OOXMLFastTokenHandler::getIdentifier(::sal_Int32 Token) 
    throw (css::uno::RuntimeException)
{
    ::rtl::OUString sResult;

    if ( Token >= 0 || Token < OOXML_FAST_TOKENS_END )
    {
        static ::rtl::OUString aTokens[OOXML_FAST_TOKENS_END];
        
        if (aTokens[Token].getLength() == 0)
            aTokens[Token] = ::rtl::OUString::createFromAscii
                (tokenmap::wordlist[Token].name);
    }
        
    return sResult;
}

css::uno::Sequence< ::sal_Int8 > SAL_CALL OOXMLFastTokenHandler::getUTF8Identifier(::sal_Int32 Token) 
    throw (css::uno::RuntimeException)
{
    if ( Token < 0  || Token >= OOXML_FAST_TOKENS_END )
        return css::uno::Sequence< ::sal_Int8 >();
        
    return css::uno::Sequence< ::sal_Int8 >(reinterpret_cast< const sal_Int8 *>(tokenmap::wordlist[Token].name), strlen(tokenmap::wordlist[Token].name));
}

::sal_Int32 SAL_CALL OOXMLFastTokenHandler::getTokenFromUTF8
(const css::uno::Sequence< ::sal_Int8 > & Identifier) throw (css::uno::RuntimeException)
{
    ::sal_Int32 nResult = OOXML_FAST_TOKENS_END;
    
    struct tokenmap::token * pToken =
        tokenmap::Perfect_Hash::in_word_set
        (reinterpret_cast<const char *>(Identifier.getConstArray()), 
         Identifier.getLength());

    if (pToken != NULL)
        nResult = pToken->nToken;

#ifdef DEBUG_TOKEN
    clog << "getTokenFromUTF8: " 
         << string(reinterpret_cast<const char *>
                   (Identifier.getConstArray()), Identifier.getLength())
         << ", " << nResult
         << (pToken == NULL ? ", failed" : "") << endl;
#endif

    return nResult;
}

}}
