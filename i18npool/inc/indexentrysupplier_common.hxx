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

#ifndef INCLUDED_I18NPOOL_INC_INDEXENTRYSUPPLIER_COMMON_HXX
#define INCLUDED_I18NPOOL_INC_INDEXENTRYSUPPLIER_COMMON_HXX

#include <com/sun/star/i18n/XExtendedIndexEntrySupplier.hpp>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <collatorImpl.hxx>

namespace com { namespace sun { namespace star { namespace i18n {


//  class IndexEntrySupplier_Common


class IndexEntrySupplier_Common : public cppu::WeakImplHelper
<
    css::i18n::XExtendedIndexEntrySupplier,
    css::lang::XServiceInfo
>
{
public:
    IndexEntrySupplier_Common( const css::uno::Reference < css::uno::XComponentContext >& rxContext );
    virtual ~IndexEntrySupplier_Common();

    virtual css::uno::Sequence < css::lang::Locale > SAL_CALL getLocaleList()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence < OUString > SAL_CALL getAlgorithmList(
        const css::lang::Locale& rLocale )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL usePhoneticEntry(
        const css::lang::Locale& rLocale )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual OUString SAL_CALL getPhoneticCandidate( const OUString& IndexEntry,
        const css::lang::Locale& rLocale )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL loadAlgorithm(
        const css::lang::Locale& rLocale,
        const OUString& SortAlgorithm, sal_Int32 collatorOptions )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual OUString SAL_CALL getIndexKey( const OUString& IndexEntry,
        const OUString& PhoneticEntry, const css::lang::Locale& rLocale )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Int16 SAL_CALL compareIndexEntry( const OUString& IndexEntry1,
        const OUString& PhoneticEntry1, const css::lang::Locale& rLocale1,
        const OUString& IndexEntry2, const OUString& PhoneticEntry2,
        const css::lang::Locale& rLocale2 )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual OUString SAL_CALL getIndexCharacter( const OUString& rIndexEntry,
        const css::lang::Locale& rLocale, const OUString& rSortAlgorithm )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual OUString SAL_CALL getIndexFollowPageWord( sal_Bool MorePages,
        const css::lang::Locale& rLocale )
        throw (css::uno::RuntimeException, std::exception) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName)
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw( css::uno::RuntimeException, std::exception ) override;

protected:
    const sal_Char *   implementationName;
    bool               usePhonetic;
    CollatorImpl*      collator;
    css::lang::Locale  aLocale;
    OUString           aAlgorithm;

    const OUString& SAL_CALL getEntry( const OUString& IndexEntry,
        const OUString& PhoneticEntry, const css::lang::Locale& rLocale )
        throw (css::uno::RuntimeException);
};

} } } }
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
