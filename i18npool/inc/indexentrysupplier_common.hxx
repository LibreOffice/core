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

#ifndef _I18N_INDEXENTRYSUPPLIER_COMMON_HXX_
#define _I18N_INDEXENTRYSUPPLIER_COMMON_HXX_

#include <com/sun/star/i18n/XExtendedIndexEntrySupplier.hpp>
#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <collatorImpl.hxx>

namespace com { namespace sun { namespace star { namespace i18n {


//  class IndexEntrySupplier_Common


class IndexEntrySupplier_Common : public cppu::WeakImplHelper2
<
    com::sun::star::i18n::XExtendedIndexEntrySupplier,
    com::sun::star::lang::XServiceInfo
>
{
public:
    IndexEntrySupplier_Common( const com::sun::star::uno::Reference < com::sun::star::uno::XComponentContext >& rxContext );
    ~IndexEntrySupplier_Common();

    virtual com::sun::star::uno::Sequence < com::sun::star::lang::Locale > SAL_CALL getLocaleList()
        throw (com::sun::star::uno::RuntimeException);

    virtual com::sun::star::uno::Sequence < OUString > SAL_CALL getAlgorithmList(
        const com::sun::star::lang::Locale& rLocale )
        throw (com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL usePhoneticEntry(
        const com::sun::star::lang::Locale& rLocale )
        throw (com::sun::star::uno::RuntimeException);

    virtual OUString SAL_CALL getPhoneticCandidate( const OUString& IndexEntry,
        const com::sun::star::lang::Locale& rLocale )
        throw (com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL loadAlgorithm(
        const com::sun::star::lang::Locale& rLocale,
        const OUString& SortAlgorithm, sal_Int32 collatorOptions )
        throw (com::sun::star::uno::RuntimeException);

    virtual OUString SAL_CALL getIndexKey( const OUString& IndexEntry,
        const OUString& PhoneticEntry, const com::sun::star::lang::Locale& rLocale )
        throw (com::sun::star::uno::RuntimeException);

    virtual sal_Int16 SAL_CALL compareIndexEntry( const OUString& IndexEntry1,
        const OUString& PhoneticEntry1, const com::sun::star::lang::Locale& rLocale1,
        const OUString& IndexEntry2, const OUString& PhoneticEntry2,
        const com::sun::star::lang::Locale& rLocale2 )
        throw (com::sun::star::uno::RuntimeException);

    virtual OUString SAL_CALL getIndexCharacter( const OUString& rIndexEntry,
        const com::sun::star::lang::Locale& rLocale, const OUString& rSortAlgorithm )
        throw (com::sun::star::uno::RuntimeException);

    virtual OUString SAL_CALL getIndexFollowPageWord( sal_Bool MorePages,
        const com::sun::star::lang::Locale& rLocale )
        throw (com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw( com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName)
        throw( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw( com::sun::star::uno::RuntimeException );

protected:
    const sal_Char *implementationName;
    sal_Bool usePhonetic;
    CollatorImpl *collator;
    const OUString& SAL_CALL getEntry( const OUString& IndexEntry,
        const OUString& PhoneticEntry, const com::sun::star::lang::Locale& rLocale )
        throw (com::sun::star::uno::RuntimeException);
    com::sun::star::lang::Locale aLocale;
    OUString aAlgorithm;
};

} } } }
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
