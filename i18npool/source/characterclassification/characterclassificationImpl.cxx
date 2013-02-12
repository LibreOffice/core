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


#include <characterclassificationImpl.hxx>
#include <rtl/ustrbuf.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

namespace com { namespace sun { namespace star { namespace i18n {

CharacterClassificationImpl::CharacterClassificationImpl(
        const Reference < uno::XComponentContext >& rxContext ) : m_xContext( rxContext )
{
        if (createLocaleSpecificCharacterClassification(OUString("Unicode"), Locale()))
            xUCI = cachedItem->xCI;
}

CharacterClassificationImpl::~CharacterClassificationImpl() {
        // Clear lookuptable
        for (size_t l = 0; l < lookupTable.size(); l++)
            delete lookupTable[l];
        lookupTable.clear();
}


OUString SAL_CALL
CharacterClassificationImpl::toUpper( const OUString& Text, sal_Int32 nPos,
        sal_Int32 nCount, const Locale& rLocale ) throw(RuntimeException)
{
        return getLocaleSpecificCharacterClassification(rLocale)->toUpper(Text, nPos, nCount, rLocale);
}

OUString SAL_CALL
CharacterClassificationImpl::toLower( const OUString& Text, sal_Int32 nPos,
        sal_Int32 nCount, const Locale& rLocale ) throw(RuntimeException)
{
        return getLocaleSpecificCharacterClassification(rLocale)->toLower(Text, nPos, nCount, rLocale);
}

OUString SAL_CALL
CharacterClassificationImpl::toTitle( const OUString& Text, sal_Int32 nPos,
        sal_Int32 nCount, const Locale& rLocale ) throw(RuntimeException)
{
        return getLocaleSpecificCharacterClassification(rLocale)->toTitle(Text, nPos, nCount, rLocale);
}

sal_Int16 SAL_CALL
CharacterClassificationImpl::getType( const OUString& Text, sal_Int32 nPos )
        throw(RuntimeException)
{
        if (xUCI.is())
            return xUCI->getType(Text, nPos);
        throw RuntimeException();
}

sal_Int16 SAL_CALL
CharacterClassificationImpl::getCharacterDirection( const OUString& Text, sal_Int32 nPos )
        throw(RuntimeException)
{
        if (xUCI.is())
            return xUCI->getCharacterDirection(Text, nPos);
        throw RuntimeException();
}

sal_Int16 SAL_CALL
CharacterClassificationImpl::getScript( const OUString& Text, sal_Int32 nPos )
        throw(RuntimeException)
{
        if (xUCI.is())
            return xUCI->getScript(Text, nPos);
        throw RuntimeException();
}

sal_Int32 SAL_CALL
CharacterClassificationImpl::getCharacterType( const OUString& Text, sal_Int32 nPos,
        const Locale& rLocale ) throw(RuntimeException)
{
        return getLocaleSpecificCharacterClassification(rLocale)->getCharacterType(Text, nPos, rLocale);
}

sal_Int32 SAL_CALL
CharacterClassificationImpl::getStringType( const OUString& Text, sal_Int32 nPos,
        sal_Int32 nCount, const Locale& rLocale ) throw(RuntimeException)
{
        return getLocaleSpecificCharacterClassification(rLocale)->getStringType(Text, nPos, nCount, rLocale);
}

ParseResult SAL_CALL CharacterClassificationImpl::parseAnyToken(
        const OUString& Text, sal_Int32 nPos, const Locale& rLocale,
        sal_Int32 startCharTokenType, const OUString& userDefinedCharactersStart,
        sal_Int32 contCharTokenType, const OUString& userDefinedCharactersCont )
        throw(RuntimeException)
{
        return getLocaleSpecificCharacterClassification(rLocale)->parseAnyToken(Text, nPos, rLocale,
                startCharTokenType,userDefinedCharactersStart,
                contCharTokenType, userDefinedCharactersCont);
}


ParseResult SAL_CALL CharacterClassificationImpl::parsePredefinedToken(
        sal_Int32 nTokenType, const OUString& Text, sal_Int32 nPos,
        const Locale& rLocale, sal_Int32 startCharTokenType,
        const OUString& userDefinedCharactersStart, sal_Int32 contCharTokenType,
        const OUString& userDefinedCharactersCont ) throw(RuntimeException)
{
        return getLocaleSpecificCharacterClassification(rLocale)->parsePredefinedToken(
                nTokenType, Text, nPos, rLocale, startCharTokenType, userDefinedCharactersStart,
                contCharTokenType, userDefinedCharactersCont);
}

sal_Bool SAL_CALL CharacterClassificationImpl::createLocaleSpecificCharacterClassification(const OUString& serviceName, const Locale& rLocale)
{
        // to share service between same Language but different Country code, like zh_CN and zh_SG
        for (size_t l = 0; l < lookupTable.size(); l++) {
            cachedItem = lookupTable[l];
            if (serviceName == cachedItem->aName) {
                lookupTable.push_back( cachedItem = new lookupTableItem(rLocale, serviceName, cachedItem->xCI) );
                return sal_True;
            }
        }

        Reference < XInterface > xI = m_xContext->getServiceManager()->createInstanceWithContext(
             OUString("com.sun.star.i18n.CharacterClassification_") + serviceName, m_xContext);

        Reference < XCharacterClassification > xCI;
        if ( xI.is() ) {
            xCI.set( xI, UNO_QUERY );
            if (xCI.is()) {
                lookupTable.push_back( cachedItem =  new lookupTableItem(rLocale, serviceName, xCI) );
                return sal_True;
            }
        }
        return sal_False;
}

Reference < XCharacterClassification > SAL_CALL
CharacterClassificationImpl::getLocaleSpecificCharacterClassification(const Locale& rLocale)
        throw(RuntimeException)
{
        // reuse instance if locale didn't change
        if (cachedItem && cachedItem->equals(rLocale))
            return cachedItem->xCI;
        else {
            for (size_t i = 0; i < lookupTable.size(); i++) {
                cachedItem = lookupTable[i];
                if (cachedItem->equals(rLocale))
                    return cachedItem->xCI;
            }

            static sal_Unicode under = (sal_Unicode)'_';
            sal_Int32 l = rLocale.Language.getLength();
            sal_Int32 c = rLocale.Country.getLength();
            sal_Int32 v = rLocale.Variant.getLength();
            OUStringBuffer aBuf(l+c+v+3);

                    // load service with name <base>_<lang>_<country>_<varian>
            if ((l > 0 && c > 0 && v > 0 &&
                    createLocaleSpecificCharacterClassification(aBuf.append(rLocale.Language).append(under).append(
                                    rLocale.Country).append(under).append(rLocale.Variant).makeStringAndClear(), rLocale)) ||
                    // load service with name <base>_<lang>_<country>
                (l > 0 && c > 0 &&
                    createLocaleSpecificCharacterClassification(aBuf.append(rLocale.Language).append(under).append(
                                    rLocale.Country).makeStringAndClear(), rLocale)) ||
                (l > 0 && c > 0 && rLocale.Language.compareToAscii("zh") == 0 &&
                                    (rLocale.Country.compareToAscii("HK") == 0 ||
                                    rLocale.Country.compareToAscii("MO") == 0) &&
                    // if the country code is HK or MO, one more step to try TW.
                    createLocaleSpecificCharacterClassification(aBuf.append(rLocale.Language).append(under).append(
                                    "TW").makeStringAndClear(), rLocale)) ||
                (l > 0 &&
                    // load service with name <base>_<lang>
                    createLocaleSpecificCharacterClassification(rLocale.Language, rLocale))) {
                return cachedItem->xCI;
            } else if (xUCI.is()) {
                lookupTable.push_back( cachedItem = new lookupTableItem(rLocale, OUString("Unicode"), xUCI) );
                return cachedItem->xCI;
            }
        }
        throw RuntimeException();
}

const sal_Char cClass[] = "com.sun.star.i18n.CharacterClassification";

OUString SAL_CALL
CharacterClassificationImpl::getImplementationName(void)
                throw( RuntimeException )
{
    return OUString::createFromAscii(cClass);
}

sal_Bool SAL_CALL
CharacterClassificationImpl::supportsService(const rtl::OUString& rServiceName)
                throw( RuntimeException )
{
    return !rServiceName.compareToAscii(cClass);
}

Sequence< OUString > SAL_CALL
CharacterClassificationImpl::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    aRet[0] = OUString::createFromAscii(cClass);
    return aRet;
}

} } } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
