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

#include <cppuhelper/supportsservice.hxx>
#include <characterclassificationImpl.hxx>
#include <localedata.hxx>
#include <rtl/ustrbuf.hxx>

using namespace com::sun::star::uno;
using namespace ::com::sun::star::i18n;
using namespace com::sun::star::lang;

namespace i18npool {

CharacterClassificationImpl::CharacterClassificationImpl(
        const Reference < XComponentContext >& rxContext ) : m_xContext( rxContext )
{
    if (createLocaleSpecificCharacterClassification("Unicode", Locale()))
        xUCI = cachedItem->xCI;
}

CharacterClassificationImpl::~CharacterClassificationImpl() {
}


OUString SAL_CALL
CharacterClassificationImpl::toUpper( const OUString& Text, sal_Int32 nPos,
        sal_Int32 nCount, const Locale& rLocale )
{
    return getLocaleSpecificCharacterClassification(rLocale)->toUpper(Text, nPos, nCount, rLocale);
}

OUString SAL_CALL
CharacterClassificationImpl::toLower( const OUString& Text, sal_Int32 nPos,
        sal_Int32 nCount, const Locale& rLocale )
{
    return getLocaleSpecificCharacterClassification(rLocale)->toLower(Text, nPos, nCount, rLocale);
}

OUString SAL_CALL
CharacterClassificationImpl::toTitle( const OUString& Text, sal_Int32 nPos,
        sal_Int32 nCount, const Locale& rLocale )
{
    return getLocaleSpecificCharacterClassification(rLocale)->toTitle(Text, nPos, nCount, rLocale);
}

sal_Int16 SAL_CALL
CharacterClassificationImpl::getType( const OUString& Text, sal_Int32 nPos )
{
    if (xUCI.is())
        return xUCI->getType(Text, nPos);
    throw RuntimeException();
}

sal_Int16 SAL_CALL
CharacterClassificationImpl::getCharacterDirection( const OUString& Text, sal_Int32 nPos )
{
    if (xUCI.is())
        return xUCI->getCharacterDirection(Text, nPos);
    throw RuntimeException();
}

sal_Int16 SAL_CALL
CharacterClassificationImpl::getScript( const OUString& Text, sal_Int32 nPos )
{
    if (xUCI.is())
        return xUCI->getScript(Text, nPos);
    throw RuntimeException();
}

sal_Int32 SAL_CALL
CharacterClassificationImpl::getCharacterType( const OUString& Text, sal_Int32 nPos,
        const Locale& rLocale )
{
    return getLocaleSpecificCharacterClassification(rLocale)->getCharacterType(Text, nPos, rLocale);
}

sal_Int32 SAL_CALL
CharacterClassificationImpl::getStringType( const OUString& Text, sal_Int32 nPos,
        sal_Int32 nCount, const Locale& rLocale )
{
    return getLocaleSpecificCharacterClassification(rLocale)->getStringType(Text, nPos, nCount, rLocale);
}

ParseResult SAL_CALL CharacterClassificationImpl::parseAnyToken(
        const OUString& Text, sal_Int32 nPos, const Locale& rLocale,
        sal_Int32 startCharTokenType, const OUString& userDefinedCharactersStart,
        sal_Int32 contCharTokenType, const OUString& userDefinedCharactersCont )
{
    return getLocaleSpecificCharacterClassification(rLocale)->parseAnyToken(Text, nPos, rLocale,
            startCharTokenType,userDefinedCharactersStart,
            contCharTokenType, userDefinedCharactersCont);
}


ParseResult SAL_CALL CharacterClassificationImpl::parsePredefinedToken(
        sal_Int32 nTokenType, const OUString& Text, sal_Int32 nPos,
        const Locale& rLocale, sal_Int32 startCharTokenType,
        const OUString& userDefinedCharactersStart, sal_Int32 contCharTokenType,
        const OUString& userDefinedCharactersCont )
{
    return getLocaleSpecificCharacterClassification(rLocale)->parsePredefinedToken(
            nTokenType, Text, nPos, rLocale, startCharTokenType, userDefinedCharactersStart,
            contCharTokenType, userDefinedCharactersCont);
}

bool CharacterClassificationImpl::createLocaleSpecificCharacterClassification(const OUString& serviceName, const Locale& rLocale)
{
    // to share service between same Language but different Country code, like zh_CN and zh_SG
    for (size_t l = 0; l < lookupTable.size(); l++) {
        cachedItem = lookupTable[l].get();
        if (serviceName == cachedItem->aName) {
            lookupTable.emplace_back( new lookupTableItem(rLocale, serviceName, cachedItem->xCI) );
            cachedItem = lookupTable.back().get();
            return true;
        }
    }

    Reference < XInterface > xI = m_xContext->getServiceManager()->createInstanceWithContext(
            "com.sun.star.i18n.CharacterClassification_" + serviceName, m_xContext);

    Reference < XCharacterClassification > xCI;
    if ( xI.is() ) {
        xCI.set( xI, UNO_QUERY );
        if (xCI.is()) {
            lookupTable.emplace_back( new lookupTableItem(rLocale, serviceName, xCI) );
            cachedItem = lookupTable.back().get();
            return true;
        }
    }
    return false;
}

Reference < XCharacterClassification > const &
CharacterClassificationImpl::getLocaleSpecificCharacterClassification(const Locale& rLocale)
{
    // reuse instance if locale didn't change
    if (cachedItem && cachedItem->equals(rLocale))
        return cachedItem->xCI;
    else {
        for (auto & i : lookupTable) {
            cachedItem = i.get();
            if (cachedItem->equals(rLocale))
                return cachedItem->xCI;
        }

        // Load service with name <base>_<lang>_<country> or
        // <base>_<bcp47> and fallbacks.
        bool bLoaded = createLocaleSpecificCharacterClassification(
                LocaleDataImpl::getFirstLocaleServiceName( rLocale), rLocale);
        if (!bLoaded)
        {
            ::std::vector< OUString > aFallbacks( LocaleDataImpl::getFallbackLocaleServiceNames( rLocale));
            for (const auto& rFallback : aFallbacks)
            {
                bLoaded = createLocaleSpecificCharacterClassification(rFallback, rLocale);
                if (bLoaded)
                    break;
            }
        }
        if (bLoaded)
            return cachedItem->xCI;
        else if (xUCI.is())
        {
            lookupTable.emplace_back( new lookupTableItem(rLocale, "Unicode", xUCI) );
            cachedItem = lookupTable.back().get();
            return cachedItem->xCI;
        }
    }
    throw RuntimeException();
}

OUString SAL_CALL
CharacterClassificationImpl::getImplementationName()
{
    return OUString("com.sun.star.i18n.CharacterClassification");
}

sal_Bool SAL_CALL
CharacterClassificationImpl::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SAL_CALL
CharacterClassificationImpl::getSupportedServiceNames()
{
    Sequence< OUString > aRet { "com.sun.star.i18n.CharacterClassification" };
    return aRet;
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_i18n_CharacterClassification_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new i18npool::CharacterClassificationImpl(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
