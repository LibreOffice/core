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

#ifndef INCLUDED_I18NPOOL_INC_INDEXENTRYSUPPLIER_JA_PHONETIC_HXX
#define INCLUDED_I18NPOOL_INC_INDEXENTRYSUPPLIER_JA_PHONETIC_HXX

#include <indexentrysupplier_common.hxx>

namespace com { namespace sun { namespace star { namespace i18n {


//  class IndexEntrySupplier_ja_phonetic


class IndexEntrySupplier_ja_phonetic : public IndexEntrySupplier_Common {
public:
    IndexEntrySupplier_ja_phonetic( const com::sun::star::uno::Reference < com::sun::star::uno::XComponentContext >& rxContext ) : IndexEntrySupplier_Common(rxContext) {
        implementationName = "com.sun.star.i18n.IndexEntrySupplier_ja_phonetic";
    };
    virtual OUString SAL_CALL getIndexCharacter( const OUString& rIndexEntry,\
        const com::sun::star::lang::Locale& rLocale, const OUString& rSortAlgorithm ) \
        throw (com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual OUString SAL_CALL getIndexKey( const OUString& IndexEntry, \
        const OUString& PhoneticEntry, const com::sun::star::lang::Locale& rLocale )\
        throw (com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual sal_Int16 SAL_CALL compareIndexEntry( const OUString& IndexEntry1,\
        const OUString& PhoneticEntry1, const com::sun::star::lang::Locale& rLocale1,\
        const OUString& IndexEntry2, const OUString& PhoneticEntry2,\
        const com::sun::star::lang::Locale& rLocale2 )\
        throw (com::sun::star::uno::RuntimeException, std::exception) override;\
};

#define INDEXENTRYSUPPLIER_JA_PHONETIC( algorithm ) \
class IndexEntrySupplier_##algorithm : public IndexEntrySupplier_ja_phonetic {\
public:\
    IndexEntrySupplier_##algorithm (const com::sun::star::uno::Reference < com::sun::star::uno::XComponentContext >& rxContext) : IndexEntrySupplier_ja_phonetic (rxContext) {\
        implementationName = "com.sun.star.i18n.IndexEntrySupplier_"#algorithm;\
    };\
    virtual sal_Bool SAL_CALL loadAlgorithm(\
        const com::sun::star::lang::Locale& rLocale,\
        const OUString& SortAlgorithm, sal_Int32 collatorOptions ) \
        throw (com::sun::star::uno::RuntimeException, std::exception) override;\
};

INDEXENTRYSUPPLIER_JA_PHONETIC( ja_phonetic_alphanumeric_first_by_syllable )
INDEXENTRYSUPPLIER_JA_PHONETIC( ja_phonetic_alphanumeric_first_by_consonant )
INDEXENTRYSUPPLIER_JA_PHONETIC( ja_phonetic_alphanumeric_last_by_syllable )
INDEXENTRYSUPPLIER_JA_PHONETIC( ja_phonetic_alphanumeric_last_by_consonant )

} } } }
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
