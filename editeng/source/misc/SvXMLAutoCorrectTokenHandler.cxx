/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "SvXMLAutoCorrectTokenHandler.hxx"
#include <xmloff/xmltoken.hxx>
#if defined __clang__
#if __has_warning("-Wdeprecated-register")
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-register"
#endif
#endif
#include <tokens.cxx>
#if defined __clang__
#if __has_warning("-Wdeprecated-register")
#pragma GCC diagnostic pop
#endif
#endif

using namespace css::uno;
using namespace ::xmloff::token;

SvXMLAutoCorrectTokenHandler::SvXMLAutoCorrectTokenHandler()
{
}

SvXMLAutoCorrectTokenHandler::~SvXMLAutoCorrectTokenHandler()
{
}

sal_Int32 SAL_CALL SvXMLAutoCorrectTokenHandler::getTokenFromUTF8( const Sequence< sal_Int8 >& Identifier )
{
    return getTokenDirect( std::string_view(reinterpret_cast< const char* >( Identifier.getConstArray() ), Identifier.getLength()) );
}

Sequence< sal_Int8 > SAL_CALL SvXMLAutoCorrectTokenHandler::getUTF8Identifier( sal_Int32 )
{
    return Sequence< sal_Int8 >();
}

sal_Int32 SvXMLAutoCorrectTokenHandler::getTokenDirect(std::string_view token) const
{
    const struct xmltoken* pToken = Perfect_Hash::in_word_set(token.data(), token.size());
    return pToken ? pToken->nToken : XML_TOKEN_INVALID;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
