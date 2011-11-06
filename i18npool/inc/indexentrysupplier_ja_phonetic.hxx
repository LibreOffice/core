/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
