/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _I18N_CHARACTERCLASSIFICATIONIMPL_HXX_
#define _I18N_CHARACTERCLASSIFICATIONIMPL_HXX_

#include <com/sun/star/i18n/XCharacterClassification.hpp>
#include <cppuhelper/implbase2.hxx> // helper for implementations
#include <vector>
#include <com/sun/star/i18n/KCharacterType.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

namespace com { namespace sun { namespace star { namespace i18n {

class CharacterClassificationImpl : public cppu::WeakImplHelper2
<
    XCharacterClassification,
    com::sun::star::lang::XServiceInfo
>
{
public:

    CharacterClassificationImpl( const com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory >& rxMSF );
    virtual ~CharacterClassificationImpl();

    virtual rtl::OUString SAL_CALL toUpper( const rtl::OUString& Text,
        sal_Int32 nPos, sal_Int32 nCount, const com::sun::star::lang::Locale& rLocale )
        throw(com::sun::star::uno::RuntimeException);
    virtual rtl::OUString SAL_CALL toLower( const rtl::OUString& Text,
        sal_Int32 nPos, sal_Int32 nCount, const com::sun::star::lang::Locale& rLocale )
        throw(com::sun::star::uno::RuntimeException);
    virtual rtl::OUString SAL_CALL toTitle( const rtl::OUString& Text, sal_Int32 nPos,
        sal_Int32 nCount, const com::sun::star::lang::Locale& rLocale )
        throw(com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getType( const rtl::OUString& Text, sal_Int32 nPos )
        throw(com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getCharacterDirection( const rtl::OUString& Text, sal_Int32 nPos )
        throw(com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getScript( const rtl::OUString& Text, sal_Int32 nPos )
        throw(com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getCharacterType( const rtl::OUString& text, sal_Int32 nPos,
        const com::sun::star::lang::Locale& rLocale )
        throw(com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getStringType( const rtl::OUString& text, sal_Int32 nPos,
        sal_Int32 nCount, const com::sun::star::lang::Locale& rLocale )
        throw(com::sun::star::uno::RuntimeException);
    virtual ParseResult SAL_CALL parseAnyToken( const rtl::OUString& Text, sal_Int32 nPos,
        const com::sun::star::lang::Locale& rLocale, sal_Int32 nStartCharFlags,
        const rtl::OUString& userDefinedCharactersStart, sal_Int32 nContCharFlags,
        const rtl::OUString& userDefinedCharactersCont )
        throw(com::sun::star::uno::RuntimeException);
    virtual ParseResult SAL_CALL parsePredefinedToken( sal_Int32 nTokenType,
        const rtl::OUString& Text, sal_Int32 nPos, const com::sun::star::lang::Locale& rLocale,
        sal_Int32 nStartCharFlags, const rtl::OUString& userDefinedCharactersStart,
        sal_Int32 nContCharFlags, const rtl::OUString& userDefinedCharactersCont )
        throw(com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void)
                throw( com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName)
        throw( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void)
        throw( com::sun::star::uno::RuntimeException );

private:
    struct lookupTableItem {
        lookupTableItem(const com::sun::star::lang::Locale& rLocale, const rtl::OUString& rName,
        com::sun::star::uno::Reference < XCharacterClassification >& rxCI) :
        aLocale(rLocale), aName(rName), xCI(rxCI) {};
        com::sun::star::lang::Locale aLocale;
        rtl::OUString aName;
        com::sun::star::uno::Reference < XCharacterClassification > xCI;
        sal_Bool SAL_CALL equals(const com::sun::star::lang::Locale& rLocale) {
        return aLocale.Language == rLocale.Language &&
            aLocale.Country == rLocale.Country &&
            aLocale.Variant == rLocale.Variant;
        };
    };
    std::vector<lookupTableItem*> lookupTable;
    lookupTableItem *cachedItem;

    com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory > xMSF;
    com::sun::star::uno::Reference < XCharacterClassification > xUCI;

    com::sun::star::uno::Reference < XCharacterClassification > SAL_CALL
    getLocaleSpecificCharacterClassification(const com::sun::star::lang::Locale& rLocale) throw(com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL
    createLocaleSpecificCharacterClassification(const rtl::OUString& serviceName, const com::sun::star::lang::Locale& rLocale);

};

} } } }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
