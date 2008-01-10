/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OOXMLFastTokenHandler.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-10 11:58:24 $
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

#include <iostream>
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
