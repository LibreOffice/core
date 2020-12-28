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
#ifndef INCLUDED_I18NPOOL_INC_INDEXENTRYSUPPLIER_HXX
#define INCLUDED_I18NPOOL_INC_INDEXENTRYSUPPLIER_HXX

#include <sal/config.h>

#include <string_view>

#include <com/sun/star/i18n/XExtendedIndexEntrySupplier.hpp>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>

namespace com::sun::star::uno { class XComponentContext; }

namespace i18npool {



class IndexEntrySupplier final : public cppu::WeakImplHelper
<
    css::i18n::XExtendedIndexEntrySupplier,
    css::lang::XServiceInfo
>
{
public:
    IndexEntrySupplier( const css::uno::Reference < css::uno::XComponentContext >& rxContext );

    // Methods
    virtual css::uno::Sequence < css::lang::Locale > SAL_CALL getLocaleList() override;

    virtual css::uno::Sequence < OUString > SAL_CALL getAlgorithmList(
        const css::lang::Locale& rLocale ) override;

    virtual sal_Bool SAL_CALL loadAlgorithm(
        const css::lang::Locale& rLocale,
        const OUString& SortAlgorithm, sal_Int32 collatorOptions ) override;

    virtual sal_Bool SAL_CALL usePhoneticEntry(
        const css::lang::Locale& rLocale ) override;

    virtual OUString SAL_CALL getPhoneticCandidate( const OUString& IndexEntry,
        const css::lang::Locale& rLocale ) override;

    virtual OUString SAL_CALL getIndexKey( const OUString& IndexEntry,
        const OUString& PhoneticEntry, const css::lang::Locale& rLocale ) override;

    virtual sal_Int16 SAL_CALL compareIndexEntry( const OUString& IndexEntry1,
        const OUString& PhoneticEntry1, const css::lang::Locale& rLocale1,
        const OUString& IndexEntry2, const OUString& PhoneticEntry2,
        const css::lang::Locale& rLocale2 ) override;

    virtual OUString SAL_CALL getIndexCharacter( const OUString& IndexEntry,
        const css::lang::Locale& rLocale, const OUString& SortAlgorithm ) override;

    virtual OUString SAL_CALL getIndexFollowPageWord( sal_Bool MorePages,
        const css::lang::Locale& rLocale ) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

private:
    css::uno::Reference < css::i18n::XExtendedIndexEntrySupplier > xIES;
    css::uno::Reference < css::uno::XComponentContext > m_xContext;
    /// @throws css::uno::RuntimeException
    bool createLocaleSpecificIndexEntrySupplier(std::u16string_view name);
    /// @throws css::uno::RuntimeException
    css::uno::Reference < css::i18n::XExtendedIndexEntrySupplier > const & getLocaleSpecificIndexEntrySupplier(
        const css::lang::Locale& rLocale, const OUString& rSortAlgorithm);

    css::lang::Locale aLocale;
    OUString          aSortAlgorithm;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
