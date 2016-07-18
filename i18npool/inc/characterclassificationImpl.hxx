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
#ifndef INCLUDED_I18NPOOL_INC_CHARACTERCLASSIFICATIONIMPL_HXX
#define INCLUDED_I18NPOOL_INC_CHARACTERCLASSIFICATIONIMPL_HXX

#include <com/sun/star/i18n/XCharacterClassification.hpp>
#include <cppuhelper/implbase.hxx>
#include <vector>
#include <com/sun/star/i18n/KCharacterType.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

namespace com { namespace sun { namespace star { namespace i18n {

class CharacterClassificationImpl : public cppu::WeakImplHelper
<
    XCharacterClassification,
    css::lang::XServiceInfo
>
{
public:

    CharacterClassificationImpl( const css::uno::Reference < css::uno::XComponentContext >& rxContext );
    virtual ~CharacterClassificationImpl();

    virtual OUString SAL_CALL toUpper( const OUString& Text,
        sal_Int32 nPos, sal_Int32 nCount, const css::lang::Locale& rLocale )
        throw(css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL toLower( const OUString& Text,
        sal_Int32 nPos, sal_Int32 nCount, const css::lang::Locale& rLocale )
        throw(css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL toTitle( const OUString& Text, sal_Int32 nPos,
        sal_Int32 nCount, const css::lang::Locale& rLocale )
        throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Int16 SAL_CALL getType( const OUString& Text, sal_Int32 nPos )
        throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Int16 SAL_CALL getCharacterDirection( const OUString& Text, sal_Int32 nPos )
        throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Int16 SAL_CALL getScript( const OUString& Text, sal_Int32 nPos )
        throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getCharacterType( const OUString& text, sal_Int32 nPos,
        const css::lang::Locale& rLocale )
        throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getStringType( const OUString& text, sal_Int32 nPos,
        sal_Int32 nCount, const css::lang::Locale& rLocale )
        throw(css::uno::RuntimeException, std::exception) override;
    virtual ParseResult SAL_CALL parseAnyToken( const OUString& Text, sal_Int32 nPos,
        const css::lang::Locale& rLocale, sal_Int32 nStartCharFlags,
        const OUString& userDefinedCharactersStart, sal_Int32 nContCharFlags,
        const OUString& userDefinedCharactersCont )
        throw(css::uno::RuntimeException, std::exception) override;
    virtual ParseResult SAL_CALL parsePredefinedToken( sal_Int32 nTokenType,
        const OUString& Text, sal_Int32 nPos, const css::lang::Locale& rLocale,
        sal_Int32 nStartCharFlags, const OUString& userDefinedCharactersStart,
        sal_Int32 nContCharFlags, const OUString& userDefinedCharactersCont )
        throw(css::uno::RuntimeException, std::exception) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName)
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw( css::uno::RuntimeException, std::exception ) override;

private:
    struct lookupTableItem {
        lookupTableItem(const css::lang::Locale& rLocale, const OUString& rName,
        css::uno::Reference < XCharacterClassification >& rxCI) :
        aLocale(rLocale), aName(rName), xCI(rxCI) {};
        css::lang::Locale aLocale;
        OUString aName;
        css::uno::Reference < XCharacterClassification > xCI;
        bool SAL_CALL equals(const css::lang::Locale& rLocale) {
        return aLocale.Language == rLocale.Language &&
            aLocale.Country == rLocale.Country &&
            aLocale.Variant == rLocale.Variant;
        };
    };
    std::vector<lookupTableItem*> lookupTable;
    lookupTableItem *cachedItem;

    css::uno::Reference < css::uno::XComponentContext > m_xContext;
    css::uno::Reference < XCharacterClassification > xUCI;

    css::uno::Reference < XCharacterClassification > const & SAL_CALL getLocaleSpecificCharacterClassification(const css::lang::Locale& rLocale)
        throw(css::uno::RuntimeException);
    bool SAL_CALL createLocaleSpecificCharacterClassification(const OUString& serviceName, const css::lang::Locale& rLocale);

};

} } } }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
