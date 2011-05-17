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
    IndexEntrySupplier_Unicode( const com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory >& rxMSF );
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
    Index(const com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory >& rxMSF);
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
