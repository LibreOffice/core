/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <cppuhelper/supportsservice.hxx>
#include <i18npool/characterclassification.hxx>
#include <localedata.hxx>
#include <cclass_unicode.hxx>
#include <cpo/uno/XComponentContext.hpp>

using namespace ::cpo::uno;
using namespace cpo::uno;
using namespace ::com::sun::star::i18n;
using namespace com::sun::star::lang;

namespace i18npool {

CharacterClassification::CharacterClassification(
        const Reference < XComponentContext >& rxContext ) : m_xContext( rxContext )
{
    m_xUCI = new cclass_Unicode(m_xContext);
}

CharacterClassification::~CharacterClassification() {
}


OUString
CharacterClassification::toUpper( const OUString& Text, sal_Int32 nPos,
        sal_Int32 nCount, const Locale& rLocale )
{
    return m_xUCI->toUpper(Text, nPos, nCount, rLocale);
}

OUString
CharacterClassification::toLower( const OUString& Text, sal_Int32 nPos,
        sal_Int32 nCount, const Locale& rLocale )
{
    return m_xUCI->toLower(Text, nPos, nCount, rLocale);
}

OUString
CharacterClassification::toTitle( const OUString& Text, sal_Int32 nPos,
        sal_Int32 nCount, const Locale& rLocale )
{
    return m_xUCI->toTitle(Text, nPos, nCount, rLocale);
}

sal_Int16
CharacterClassification::getType( const OUString& Text, sal_Int32 nPos )
{
    return m_xUCI->getType(Text, nPos);
}

sal_Int16
CharacterClassification::getCharacterDirection( const OUString& Text, sal_Int32 nPos )
{
    return m_xUCI->getCharacterDirection(Text, nPos);
}

sal_Int16
CharacterClassification::getScript( const OUString& Text, sal_Int32 nPos )
{
    return m_xUCI->getScript(Text, nPos);
}

sal_Int32
CharacterClassification::getCharacterType( const OUString& Text, sal_Int32 nPos,
        const Locale& rLocale )
{
    return m_xUCI->getCharacterType(Text, nPos, rLocale);
}

sal_Int32
CharacterClassification::getStringType( const OUString& Text, sal_Int32 nPos,
        sal_Int32 nCount, const Locale& rLocale )
{
    return m_xUCI->getStringType(Text, nPos, nCount, rLocale);
}

ParseResult CharacterClassification::parseAnyToken(
        const OUString& Text, sal_Int32 nPos, const Locale& rLocale,
        sal_Int32 startCharTokenType, const OUString& userDefinedCharactersStart,
        sal_Int32 contCharTokenType, const OUString& userDefinedCharactersCont )
{
    return m_xUCI->parseAnyToken(Text, nPos, rLocale,
            startCharTokenType,userDefinedCharactersStart,
            contCharTokenType, userDefinedCharactersCont);
}


ParseResult CharacterClassification::parsePredefinedToken(
        sal_Int32 nTokenType, const OUString& Text, sal_Int32 nPos,
        const Locale& rLocale, sal_Int32 startCharTokenType,
        const OUString& userDefinedCharactersStart, sal_Int32 contCharTokenType,
        const OUString& userDefinedCharactersCont )
{
    return m_xUCI->parsePredefinedToken(
            nTokenType, Text, nPos, rLocale, startCharTokenType, userDefinedCharactersStart,
            contCharTokenType, userDefinedCharactersCont);
}

OUString
CharacterClassification::getImplementationName()
{
    return u"com.sun.star.i18n.CharacterClassification"_ustr;
}

bool
CharacterClassification::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString >
CharacterClassification::getSupportedServiceNames()
{
    return { u"com.sun.star.i18n.CharacterClassification"_ustr };
}

}

extern "C" SAL_DLLPUBLIC_EXPORT cpo::uno::XInterface *
com_sun_star_i18n_CharacterClassification_get_implementation(
    cpo::uno::XComponentContext *context,
    cpo::uno::Sequence<cpo::uno::Any> const &)
{
    return cppu::acquire(new i18npool::CharacterClassification(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
