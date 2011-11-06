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



#ifndef _I18N_INDEXENTRYSUPPLIER_ASIAN_HXX_
#define _I18N_INDEXENTRYSUPPLIER_ASIAN_HXX_

#include <indexentrysupplier_common.hxx>
#include <osl/module.h>

namespace com { namespace sun { namespace star { namespace i18n {

//  ----------------------------------------------------
//  class IndexEntrySupplier_asian
//  ----------------------------------------------------

class IndexEntrySupplier_asian : public IndexEntrySupplier_Common {
public:
    IndexEntrySupplier_asian( const com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory >& rxMSF );
    ~IndexEntrySupplier_asian();

    rtl::OUString SAL_CALL getIndexCharacter( const rtl::OUString& rIndexEntry,
            const com::sun::star::lang::Locale& rLocale, const rtl::OUString& rAlgorithm )
            throw (com::sun::star::uno::RuntimeException);
    rtl::OUString SAL_CALL getIndexKey( const rtl::OUString& rIndexEntry,
            const rtl::OUString& rPhoneticEntry, const com::sun::star::lang::Locale& rLocale)
            throw (com::sun::star::uno::RuntimeException);
    sal_Int16 SAL_CALL compareIndexEntry(
            const rtl::OUString& rIndexEntry1, const rtl::OUString& rPhoneticEntry1,
            const com::sun::star::lang::Locale& rLocale1,
            const rtl::OUString& rIndexEntry2, const rtl::OUString& rPhoneticEntry2,
            const com::sun::star::lang::Locale& rLocale2 )
            throw (com::sun::star::uno::RuntimeException);
    rtl::OUString SAL_CALL getPhoneticCandidate( const rtl::OUString& rIndexEntry,
            const com::sun::star::lang::Locale& rLocale )
            throw (com::sun::star::uno::RuntimeException);
private:
    oslModule hModule;
};

} } } }
#endif
