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

#include <iostream>
#include <string.h>
#include <ooxml/resourceids.hxx>
#include "OOXMLFastTokenHandler.hxx"

#if defined __clang__
#if __has_warning("-Wdeprecated-register")
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-register"
#endif
#endif
#include "gperffasttoken.hxx"
#if defined __clang__
#if __has_warning("-Wdeprecated-register")
#pragma GCC diagnostic pop
#endif
#endif

namespace writerfilter {
namespace ooxml
{

using namespace ::std;

OOXMLFastTokenHandler::OOXMLFastTokenHandler
(css::uno::Reference< css::uno::XComponentContext > const & context)
: m_xContext(context)
{}

// ::com::sun::star::xml::sax::XFastTokenHandler:
::sal_Int32 SAL_CALL OOXMLFastTokenHandler::getToken(const OUString & Identifier)
    throw (css::uno::RuntimeException, std::exception)
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

OUString SAL_CALL OOXMLFastTokenHandler::getIdentifier(::sal_Int32)
    throw (css::uno::RuntimeException, std::exception)
{
    // we use a Boost tokenmap, but tokenmaps cannot be indexed by an integer
    for (;;) { std::abort(); } // avoid "must return a value" warnings
}

css::uno::Sequence< ::sal_Int8 > SAL_CALL OOXMLFastTokenHandler::getUTF8Identifier(::sal_Int32)
    throw (css::uno::RuntimeException, std::exception)
{
    // we use a Boost tokenmap, but tokenmaps cannot be indexed by an integer
    for (;;) { std::abort(); } // avoid "must return a value" warnings
}

sal_Int32 OOXMLFastTokenHandler::getTokenDirect( const char *pStr, sal_Int32 nLength ) const
{
    struct tokenmap::token * pToken =
        tokenmap::Perfect_Hash::in_word_set( pStr, nLength );

    sal_Int32 nResult = pToken != NULL ? pToken->nToken : OOXML_FAST_TOKENS_END;

#ifdef DEBUG_TOKEN
    clog << "getTokenFromUTF8: "
         << string(pStr, nLength)
         << ", " << nResult
         << (pToken == NULL ? ", failed" : "") << endl;
#endif

    return nResult;
}

::sal_Int32 SAL_CALL OOXMLFastTokenHandler::getTokenFromUTF8
(const css::uno::Sequence< ::sal_Int8 > & Identifier) throw (css::uno::RuntimeException, std::exception)
{
    return getTokenDirect(reinterpret_cast<const char *>
                          (Identifier.getConstArray()),
                          Identifier.getLength());
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
