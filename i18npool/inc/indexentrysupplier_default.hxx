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
#ifndef _I18N_INDEXENTRYSUPPLIER_DEFAULT_HXX_
#define _I18N_INDEXENTRYSUPPLIER_DEFAULT_HXX_

#include <indexentrysupplier_common.hxx>

namespace com { namespace sun { namespace star { namespace i18n {

class Index;

//  ----------------------------------------------------
//  class IndexEntrySupplier_Unicode
//  ----------------------------------------------------
class IndexEntrySupplier_Unicode : public IndexEntrySupplier_Common {
public:
    IndexEntrySupplier_Unicode( const com::sun::star::uno::Reference < com::sun::star::uno::XComponentContext >& rxContext );
    ~IndexEntrySupplier_Unicode();

    virtual sal_Bool SAL_CALL loadAlgorithm(
        const com::sun::star::lang::Locale& rLocale,
        const rtl::OUString& SortAlgorithm, sal_Int32 collatorOptions )
        throw (com::sun::star::uno::RuntimeException);

    virtual rtl::OUString SAL_CALL getIndexKey( const rtl::OUString& IndexEntry,
        const rtl::OUString& PhoneticEntry, const com::sun::star::lang::Locale& rLocale )
        throw (com::sun::star::uno::RuntimeException);

    virtual sal_Int16 SAL_CALL compareIndexEntry( const rtl::OUString& IndexEntry1,
        const rtl::OUString& PhoneticEntry1, const com::sun::star::lang::Locale& rLocale1,
        const rtl::OUString& IndexEntry2, const ::rtl::OUString& PhoneticEntry2,
        const com::sun::star::lang::Locale& rLocale2 )
        throw (com::sun::star::uno::RuntimeException);

    virtual rtl::OUString SAL_CALL getIndexCharacter( const rtl::OUString& rIndexEntry,
        const com::sun::star::lang::Locale& rLocale, const rtl::OUString& rSortAlgorithm )
        throw (com::sun::star::uno::RuntimeException);

private:
    Index *index;
};

struct IndexKey {
    sal_Unicode key;
    rtl::OUString mkey;
    rtl::OUString desc;
};

class IndexTable
{
public:
    IndexTable();
    ~IndexTable();

    void init(sal_Unicode start_, sal_Unicode end_, IndexKey* keys, sal_Int16 key_count, Index *index);

    sal_Unicode start;
    sal_Unicode end;
    sal_uInt8 *table;
};

#define MAX_KEYS 0xff
#define MAX_TABLES 20

class Index
{
public:
    Index(const com::sun::star::uno::Reference < com::sun::star::uno::XComponentContext >& rxContext);
    ~Index();

    void init(const com::sun::star::lang::Locale& rLocale, const rtl::OUString& algorithm) throw (com::sun::star::uno::RuntimeException);

    void makeIndexKeys(const com::sun::star::lang::Locale &rLocale, const rtl::OUString &algorithm) throw (com::sun::star::uno::RuntimeException);
    sal_Int16 getIndexWeight(const rtl::OUString& rIndexEntry);
    rtl::OUString getIndexDescription(const rtl::OUString& rIndexEntry);

    IndexTable tables[MAX_TABLES];
    sal_Int16 table_count;
    IndexKey keys[MAX_KEYS];
    sal_Int16 key_count;
    sal_Int16 mkeys[MAX_KEYS];
    sal_Int16 mkey_count;
    rtl::OUString skipping_chars;
    CollatorImpl *collator;
    sal_Int16 compare(sal_Unicode c1, sal_Unicode c2);
};

} } } }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
