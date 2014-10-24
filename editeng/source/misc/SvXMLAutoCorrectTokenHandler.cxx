/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <SvXMLAutoCorrectTokenHandler.hxx>
#include <xmloff/xmltoken.hxx>
#include <com/sun/star/xml/sax/FastToken.hpp>

using namespace ::css::uno;
using namespace ::xmloff::token;

SvXMLAutoCorrectTokenHandler::SvXMLAutoCorrectTokenHandler()
{
}

SvXMLAutoCorrectTokenHandler::~SvXMLAutoCorrectTokenHandler()
{
}

sal_Int32 SAL_CALL SvXMLAutoCorrectTokenHandler::getTokenFromUTF8( const Sequence< sal_Int8 >& Identifier )
     throw (::css::uno::RuntimeException, std::exception)
{
    switch( Identifier.getLength() )
    {
        case 4:     return XML_NAME;
        case 5:     return XML_BLOCK;
        case 10:    return XML_BLOCK_LIST;
        case 16:    return XML_ABBREVIATED_NAME;
        default:    return css::xml::sax::FastToken::DONTKNOW;
    }
}

Sequence< sal_Int8 > SAL_CALL SvXMLAutoCorrectTokenHandler::getUTF8Identifier( sal_Int32 )
     throw (::css::uno::RuntimeException, std::exception)
{
    return Sequence< sal_Int8 >();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
