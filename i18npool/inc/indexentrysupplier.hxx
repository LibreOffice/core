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

#include <com/sun/star/i18n/XExtendedIndexEntrySupplier.hpp>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

namespace com { namespace sun { namespace star { namespace i18n {


//  class IndexEntrySupplier

class IndexEntrySupplier : public cppu::WeakImplHelper
<
    com::sun::star::i18n::XExtendedIndexEntrySupplier,
    com::sun::star::lang::XServiceInfo
>
{
public:
    IndexEntrySupplier( const com::sun::star::uno::Reference < com::sun::star::uno::XComponentContext >& rxContext );

    // Methods
    virtual com::sun::star::uno::Sequence < com::sun::star::lang::Locale > SAL_CALL getLocaleList()
        throw (com::sun::star::uno::RuntimeException, std::exception) override;

    virtual com::sun::star::uno::Sequence < OUString > SAL_CALL getAlgorithmList(
        const com::sun::star::lang::Locale& rLocale )
        throw (com::sun::star::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL loadAlgorithm(
        const com::sun::star::lang::Locale& rLocale,
        const OUString& SortAlgorithm, sal_Int32 collatorOptions )
        throw (com::sun::star::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL usePhoneticEntry(
        const com::sun::star::lang::Locale& rLocale )
        throw (com::sun::star::uno::RuntimeException, std::exception) override;

    virtual OUString SAL_CALL getPhoneticCandidate( const OUString& IndexEntry,
        const com::sun::star::lang::Locale& rLocale )
        throw (com::sun::star::uno::RuntimeException, std::exception) override;

    virtual OUString SAL_CALL getIndexKey( const OUString& IndexEntry,
        const OUString& PhoneticEntry, const com::sun::star::lang::Locale& rLocale )
        throw (com::sun::star::uno::RuntimeException, std::exception) override;

    virtual sal_Int16 SAL_CALL compareIndexEntry( const OUString& IndexEntry1,
        const OUString& PhoneticEntry1, const com::sun::star::lang::Locale& rLocale1,
        const OUString& IndexEntry2, const OUString& PhoneticEntry2,
        const com::sun::star::lang::Locale& rLocale2 )
        throw (com::sun::star::uno::RuntimeException, std::exception) override;

    virtual OUString SAL_CALL getIndexCharacter( const OUString& IndexEntry,
        const com::sun::star::lang::Locale& rLocale, const OUString& SortAlgorithm )
        throw (com::sun::star::uno::RuntimeException, std::exception) override;

    virtual OUString SAL_CALL getIndexFollowPageWord( sal_Bool MorePages,
        const com::sun::star::lang::Locale& rLocale )
        throw (com::sun::star::uno::RuntimeException, std::exception) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw( com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName)
        throw( com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw( com::sun::star::uno::RuntimeException, std::exception ) override;

private:
    com::sun::star::uno::Reference < com::sun::star::i18n::XExtendedIndexEntrySupplier > xIES;
    com::sun::star::uno::Reference < com::sun::star::uno::XComponentContext > m_xContext;
    bool SAL_CALL createLocaleSpecificIndexEntrySupplier(const OUString& name) throw( com::sun::star::uno::RuntimeException );
    com::sun::star::uno::Reference < com::sun::star::i18n::XExtendedIndexEntrySupplier > SAL_CALL getLocaleSpecificIndexEntrySupplier(
        const com::sun::star::lang::Locale& rLocale, const OUString& rSortAlgorithm) throw (com::sun::star::uno::RuntimeException);

protected:
    com::sun::star::lang::Locale aLocale;
    OUString aSortAlgorithm;
};

} } } }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
