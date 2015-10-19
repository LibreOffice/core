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
#ifndef INCLUDED_I18NPOOL_INC_INDEXENTRYSUPPLIER_DEFAULT_HXX
#define INCLUDED_I18NPOOL_INC_INDEXENTRYSUPPLIER_DEFAULT_HXX

#include <indexentrysupplier_common.hxx>

namespace com { namespace sun { namespace star { namespace i18n {

class Index;


//  class IndexEntrySupplier_Unicode

class IndexEntrySupplier_Unicode : public IndexEntrySupplier_Common {
public:
    IndexEntrySupplier_Unicode( const css::uno::Reference < css::uno::XComponentContext >& rxContext );
    virtual ~IndexEntrySupplier_Unicode();

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

private:
    Index *index;
};

struct IndexKey {
    sal_Unicode key;
    OUString mkey;
    OUString desc;
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
    Index(const css::uno::Reference < css::uno::XComponentContext >& rxContext);
    ~Index();

    void init(const css::lang::Locale& rLocale, const OUString& algorithm) throw (css::uno::RuntimeException);

    void makeIndexKeys(const css::lang::Locale &rLocale, const OUString &algorithm) throw (css::uno::RuntimeException);
    sal_Int16 getIndexWeight(const OUString& rIndexEntry);
    OUString getIndexDescription(const OUString& rIndexEntry);

    IndexTable tables[MAX_TABLES];
    sal_Int16 table_count;
    IndexKey keys[MAX_KEYS];
    sal_Int16 key_count;
    sal_Int16 mkeys[MAX_KEYS];
    sal_Int16 mkey_count;
    OUString skipping_chars;
    CollatorImpl *collator;
    sal_Int16 compare(sal_Unicode c1, sal_Unicode c2);
};

} } } }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
