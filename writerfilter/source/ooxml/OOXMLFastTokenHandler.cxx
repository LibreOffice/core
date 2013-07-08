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

OUString SAL_CALL OOXMLFastTokenHandler::getIdentifier(::sal_Int32 Token)
    throw (css::uno::RuntimeException)
{
    OUString sResult;

#if 0
    //FIXME this is broken: tokenmap::wordlist is not indexed by Token!
    if ( Token >= 0 || Token < OOXML_FAST_TOKENS_END )
    {
        static OUString aTokens[OOXML_FAST_TOKENS_END];

        if (aTokens[Token].getLength() == 0)
            aTokens[Token] = OUString::createFromAscii
                (tokenmap::wordlist[Token].name);
    }
#else
    (void) Token;
#endif

    return sResult;
}

css::uno::Sequence< ::sal_Int8 > SAL_CALL OOXMLFastTokenHandler::getUTF8Identifier(::sal_Int32 Token)
    throw (css::uno::RuntimeException)
{
#if 0
    if ( Token < 0  || Token >= OOXML_FAST_TOKENS_END )
#endif
        return css::uno::Sequence< ::sal_Int8 >();

#if 0
    //FIXME this is broken: tokenmap::wordlist is not indexed by Token!
    return css::uno::Sequence< ::sal_Int8 >(reinterpret_cast< const sal_Int8 *>(tokenmap::wordlist[Token].name), strlen(tokenmap::wordlist[Token].name));
#else
    (void) Token;
#endif
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
