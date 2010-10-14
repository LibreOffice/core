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

#ifndef _I18N_INDEXENTRYSUPPLIER_JA_PHONETIC_HXX_
#define _I18N_INDEXENTRYSUPPLIER_JA_PHONETIC_HXX_

#include <indexentrysupplier_common.hxx>

namespace com { namespace sun { namespace star { namespace i18n {

//  ----------------------------------------------------
//  class IndexEntrySupplier_ja_phonetic
//  ----------------------------------------------------

class IndexEntrySupplier_ja_phonetic : public IndexEntrySupplier_Common {
public:
    IndexEntrySupplier_ja_phonetic( const com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory >& rxMSF ) : IndexEntrySupplier_Common(rxMSF) {
        implementationName = "com.sun.star.i18n.IndexEntrySupplier_ja_phonetic";
    };
    virtual rtl::OUString SAL_CALL getIndexCharacter( const rtl::OUString& rIndexEntry,\
        const com::sun::star::lang::Locale& rLocale, const rtl::OUString& rSortAlgorithm ) \
        throw (com::sun::star::uno::RuntimeException);\
    virtual rtl::OUString SAL_CALL getIndexKey( const rtl::OUString& IndexEntry, \
        const rtl::OUString& PhoneticEntry, const com::sun::star::lang::Locale& rLocale )\
        throw (com::sun::star::uno::RuntimeException);\
    virtual sal_Int16 SAL_CALL compareIndexEntry( const rtl::OUString& IndexEntry1,\
        const rtl::OUString& PhoneticEntry1, const com::sun::star::lang::Locale& rLocale1,\
        const rtl::OUString& IndexEntry2, const ::rtl::OUString& PhoneticEntry2,\
        const com::sun::star::lang::Locale& rLocale2 )\
        throw (com::sun::star::uno::RuntimeException);\
};

#define INDEXENTRYSUPPLIER_JA_PHONETIC( algorithm ) \
class IndexEntrySupplier_##algorithm : public IndexEntrySupplier_ja_phonetic {\
public:\
    IndexEntrySupplier_##algorithm (const com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory >& rxMSF) : IndexEntrySupplier_ja_phonetic (rxMSF) {\
        implementationName = "com.sun.star.i18n.IndexEntrySupplier_"#algorithm;\
    };\
    virtual sal_Bool SAL_CALL loadAlgorithm(\
        const com::sun::star::lang::Locale& rLocale,\
        const rtl::OUString& SortAlgorithm, sal_Int32 collatorOptions ) \
        throw (com::sun::star::uno::RuntimeException);\
};

INDEXENTRYSUPPLIER_JA_PHONETIC( ja_phonetic_alphanumeric_first_by_syllable )
INDEXENTRYSUPPLIER_JA_PHONETIC( ja_phonetic_alphanumeric_first_by_consonant )
INDEXENTRYSUPPLIER_JA_PHONETIC( ja_phonetic_alphanumeric_last_by_syllable )
INDEXENTRYSUPPLIER_JA_PHONETIC( ja_phonetic_alphanumeric_last_by_consonant )

} } } }
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
