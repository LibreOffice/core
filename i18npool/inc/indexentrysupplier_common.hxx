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



#ifndef _I18N_INDEXENTRYSUPPLIER_COMMON_HXX_
#define _I18N_INDEXENTRYSUPPLIER_COMMON_HXX_

#include <com/sun/star/i18n/XExtendedIndexEntrySupplier.hpp>
#include <cppuhelper/implbase2.hxx> // helper for implementations
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <collatorImpl.hxx>

namespace com { namespace sun { namespace star { namespace i18n {

//  ----------------------------------------------------
//  class IndexEntrySupplier_Common
//  ----------------------------------------------------

class IndexEntrySupplier_Common : public cppu::WeakImplHelper2
<
    com::sun::star::i18n::XExtendedIndexEntrySupplier,
    com::sun::star::lang::XServiceInfo
>
{
public:
    IndexEntrySupplier_Common( const com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory >& rxMSF );
    ~IndexEntrySupplier_Common();

    virtual com::sun::star::uno::Sequence < com::sun::star::lang::Locale > SAL_CALL getLocaleList()
        throw (com::sun::star::uno::RuntimeException);

    virtual com::sun::star::uno::Sequence < rtl::OUString > SAL_CALL getAlgorithmList(
        const com::sun::star::lang::Locale& rLocale )
        throw (com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL usePhoneticEntry(
        const com::sun::star::lang::Locale& rLocale )
        throw (com::sun::star::uno::RuntimeException);

    virtual rtl::OUString SAL_CALL getPhoneticCandidate( const rtl::OUString& IndexEntry,
        const com::sun::star::lang::Locale& rLocale )
        throw (com::sun::star::uno::RuntimeException);

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

    virtual rtl::OUString SAL_CALL getIndexFollowPageWord( sal_Bool MorePages,
        const com::sun::star::lang::Locale& rLocale )
        throw (com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName()
        throw( com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName)
        throw( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames()
        throw( com::sun::star::uno::RuntimeException );

protected:
    const sal_Char *implementationName;
    sal_Bool usePhonetic;
    CollatorImpl *collator;
    const rtl::OUString& SAL_CALL getEntry( const rtl::OUString& IndexEntry,
        const rtl::OUString& PhoneticEntry, const com::sun::star::lang::Locale& rLocale )
        throw (com::sun::star::uno::RuntimeException);
    com::sun::star::lang::Locale aLocale;
    rtl::OUString aAlgorithm;
};

} } } }
#endif
