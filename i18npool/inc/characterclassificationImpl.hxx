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
#ifndef _I18N_CHARACTERCLASSIFICATIONIMPL_HXX_
#define _I18N_CHARACTERCLASSIFICATIONIMPL_HXX_

#include <com/sun/star/i18n/XCharacterClassification.hpp>
#include <cppuhelper/implbase2.hxx>
#include <vector>
#include <com/sun/star/i18n/KCharacterType.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

namespace com { namespace sun { namespace star { namespace i18n {

class CharacterClassificationImpl : public cppu::WeakImplHelper2
<
    XCharacterClassification,
    com::sun::star::lang::XServiceInfo
>
{
public:

    CharacterClassificationImpl( const com::sun::star::uno::Reference < com::sun::star::uno::XComponentContext >& rxContext );
    virtual ~CharacterClassificationImpl();

    virtual OUString SAL_CALL toUpper( const OUString& Text,
        sal_Int32 nPos, sal_Int32 nCount, const com::sun::star::lang::Locale& rLocale )
        throw(com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL toLower( const OUString& Text,
        sal_Int32 nPos, sal_Int32 nCount, const com::sun::star::lang::Locale& rLocale )
        throw(com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL toTitle( const OUString& Text, sal_Int32 nPos,
        sal_Int32 nCount, const com::sun::star::lang::Locale& rLocale )
        throw(com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int16 SAL_CALL getType( const OUString& Text, sal_Int32 nPos )
        throw(com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int16 SAL_CALL getCharacterDirection( const OUString& Text, sal_Int32 nPos )
        throw(com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int16 SAL_CALL getScript( const OUString& Text, sal_Int32 nPos )
        throw(com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL getCharacterType( const OUString& text, sal_Int32 nPos,
        const com::sun::star::lang::Locale& rLocale )
        throw(com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL getStringType( const OUString& text, sal_Int32 nPos,
        sal_Int32 nCount, const com::sun::star::lang::Locale& rLocale )
        throw(com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ParseResult SAL_CALL parseAnyToken( const OUString& Text, sal_Int32 nPos,
        const com::sun::star::lang::Locale& rLocale, sal_Int32 nStartCharFlags,
        const OUString& userDefinedCharactersStart, sal_Int32 nContCharFlags,
        const OUString& userDefinedCharactersCont )
        throw(com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ParseResult SAL_CALL parsePredefinedToken( sal_Int32 nTokenType,
        const OUString& Text, sal_Int32 nPos, const com::sun::star::lang::Locale& rLocale,
        sal_Int32 nStartCharFlags, const OUString& userDefinedCharactersStart,
        sal_Int32 nContCharFlags, const OUString& userDefinedCharactersCont )
        throw(com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName(void)
                throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName)
        throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(void)
        throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

private:
    struct lookupTableItem {
        lookupTableItem(const com::sun::star::lang::Locale& rLocale, const OUString& rName,
        com::sun::star::uno::Reference < XCharacterClassification >& rxCI) :
        aLocale(rLocale), aName(rName), xCI(rxCI) {};
        com::sun::star::lang::Locale aLocale;
        OUString aName;
        com::sun::star::uno::Reference < XCharacterClassification > xCI;
        sal_Bool SAL_CALL equals(const com::sun::star::lang::Locale& rLocale) {
        return aLocale.Language == rLocale.Language &&
            aLocale.Country == rLocale.Country &&
            aLocale.Variant == rLocale.Variant;
        };
    };
    std::vector<lookupTableItem*> lookupTable;
    lookupTableItem *cachedItem;

    com::sun::star::uno::Reference < com::sun::star::uno::XComponentContext > m_xContext;
    com::sun::star::uno::Reference < XCharacterClassification > xUCI;

    com::sun::star::uno::Reference < XCharacterClassification > SAL_CALL
    getLocaleSpecificCharacterClassification(const com::sun::star::lang::Locale& rLocale) throw(com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL
    createLocaleSpecificCharacterClassification(const OUString& serviceName, const com::sun::star::lang::Locale& rLocale);

};

} } } }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
