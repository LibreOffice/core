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


#ifndef _I18N_LOCALEDATA_HXX_
#define _I18N_LOCALEDATA_HXX_

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <comphelper/processfactory.hxx>

#include <com/sun/star/i18n/XLocaleData2.hpp>


#include <cppuhelper/implbase2.hxx> // helper for implementations
#include <cppu/macros.hxx>
#include <com/sun/star/uno/Reference.h>

#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/i18n/Calendar.hpp>
#include <com/sun/star/i18n/FormatElement.hpp>
#include <com/sun/star/i18n/Currency.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/i18n/LocaleDataItem.hpp>
#include <com/sun/star/i18n/UnicodeScript.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/XInterface.hpp>
#include <rtl/ustring.hxx>
#include <vector>
#include <memory>
#include <osl/module.hxx>

//
#include <defaultnumberingprovider.hxx>
#include <com/sun/star/style/NumberingType.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
//
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

struct LocaleDataLookupTableItem;

namespace com { namespace sun { namespace star { namespace i18n {

inline sal_Bool operator ==(const com::sun::star::lang::Locale& l1, const com::sun::star::lang::Locale& l2) {
    return l1.Language == l2.Language && l1.Country == l2.Country && l1.Variant == l2.Variant;
};

class LocaleData : public cppu::WeakImplHelper2
<
    XLocaleData2,
    com::sun::star::lang::XServiceInfo
>
{
public:
    LocaleData();
    ~LocaleData();

    virtual LanguageCountryInfo SAL_CALL getLanguageCountryInfo( const com::sun::star::lang::Locale& rLocale ) throw(com::sun::star::uno::RuntimeException);
    virtual LocaleDataItem SAL_CALL getLocaleItem( const com::sun::star::lang::Locale& rLocale ) throw(com::sun::star::uno::RuntimeException);
    virtual com::sun::star::uno::Sequence< Calendar > SAL_CALL getAllCalendars( const com::sun::star::lang::Locale& rLocale ) throw(com::sun::star::uno::RuntimeException);
    virtual com::sun::star::uno::Sequence< Currency > SAL_CALL getAllCurrencies( const com::sun::star::lang::Locale& rLocale ) throw(com::sun::star::uno::RuntimeException);
    virtual com::sun::star::uno::Sequence< Currency2 > SAL_CALL getAllCurrencies2( const com::sun::star::lang::Locale& rLocale ) throw(com::sun::star::uno::RuntimeException);
    virtual com::sun::star::uno::Sequence< FormatElement > SAL_CALL getAllFormats( const com::sun::star::lang::Locale& rLocale ) throw(com::sun::star::uno::RuntimeException);
    virtual com::sun::star::uno::Sequence< Implementation > SAL_CALL getCollatorImplementations( const com::sun::star::lang::Locale& rLocale ) throw(com::sun::star::uno::RuntimeException);
    virtual rtl::OUString SAL_CALL getCollatorRuleByAlgorithm( const com::sun::star::lang::Locale& rLocale, const rtl::OUString& algorithm ) throw(com::sun::star::uno::RuntimeException);
    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getTransliterations( const com::sun::star::lang::Locale& rLocale ) throw(com::sun::star::uno::RuntimeException);
    virtual ForbiddenCharacters SAL_CALL getForbiddenCharacters( const com::sun::star::lang::Locale& rLocale ) throw(com::sun::star::uno::RuntimeException);
    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getReservedWord( const com::sun::star::lang::Locale& rLocale ) throw(com::sun::star::uno::RuntimeException) ;
    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getBreakIteratorRules( const com::sun::star::lang::Locale& rLocale ) throw(com::sun::star::uno::RuntimeException) ;
    virtual com::sun::star::uno::Sequence< com::sun::star::lang::Locale > SAL_CALL getAllInstalledLocaleNames() throw(com::sun::star::uno::RuntimeException);

    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSearchOptions( const com::sun::star::lang::Locale& rLocale ) throw(com::sun::star::uno::RuntimeException);
    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getCollationOptions( const com::sun::star::lang::Locale& rLocale ) throw(com::sun::star::uno::RuntimeException);
    virtual com::sun::star::uno::Sequence< com::sun::star::uno::Sequence< beans::PropertyValue > > SAL_CALL getContinuousNumberingLevels( const com::sun::star::lang::Locale& rLocale ) throw(com::sun::star::uno::RuntimeException);
    virtual com::sun::star::uno::Sequence< com::sun::star::uno::Reference< container::XIndexAccess > > SAL_CALL getOutlineNumberingLevels( const com::sun::star::lang::Locale& rLocale ) throw(com::sun::star::uno::RuntimeException);

    // following methods are used by indexentry service
    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getIndexAlgorithm( const com::sun::star::lang::Locale& rLocale ) throw(com::sun::star::uno::RuntimeException);
    virtual rtl::OUString SAL_CALL getDefaultIndexAlgorithm( const com::sun::star::lang::Locale& rLocale ) throw(com::sun::star::uno::RuntimeException);
    virtual rtl::OUString SAL_CALL getIndexKeysByAlgorithm( const com::sun::star::lang::Locale& rLocale, const rtl::OUString& algorithm ) throw(com::sun::star::uno::RuntimeException);
    virtual rtl::OUString SAL_CALL getIndexModuleByAlgorithm( const com::sun::star::lang::Locale& rLocale, const rtl::OUString& algorithm ) throw(com::sun::star::uno::RuntimeException);
    virtual com::sun::star::uno::Sequence< UnicodeScript > SAL_CALL getUnicodeScripts( const com::sun::star::lang::Locale& rLocale ) throw(com::sun::star::uno::RuntimeException);
    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getFollowPageWords( const com::sun::star::lang::Locale& rLocale ) throw(com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasPhonetic( const com::sun::star::lang::Locale& rLocale ) throw(com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isPhonetic( const com::sun::star::lang::Locale& rLocale, const rtl::OUString& algorithm ) throw(com::sun::star::uno::RuntimeException);
    virtual rtl::OUString SAL_CALL getHangingCharacters( const com::sun::star::lang::Locale& rLocale ) throw(com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName() throw( com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames() throw( com::sun::star::uno::RuntimeException );

private :
    friend sal_Bool operator ==(const com::sun::star::lang::Locale& l1, const com::sun::star::lang::Locale& l2);

    ::std::auto_ptr< LocaleDataLookupTableItem > cachedItem;

    oslGenericFunction SAL_CALL getFunctionSymbol( const com::sun::star::lang::Locale& rLocale, const sal_Char* pFunction ) throw( com::sun::star::uno::RuntimeException );
    oslGenericFunction SAL_CALL getFunctionSymbolByName( const rtl::OUString& localeName, const sal_Char* pFunction );
    sal_Unicode ** SAL_CALL getIndexArray(const com::sun::star::lang::Locale& rLocale, sal_Int16& indexCount);
    sal_Unicode ** SAL_CALL getIndexArrayForAlgorithm(const com::sun::star::lang::Locale& rLocale, const rtl::OUString& rAlgorithm);
    com::sun::star::i18n::Calendar ref_cal;
    rtl::OUString ref_name;
    com::sun::star::uno::Sequence< com::sun::star::i18n::CalendarItem > &
        getCalendarItemByName(const rtl::OUString& name,
        const com::sun::star::lang::Locale& rLocale,
        const com::sun::star::uno::Sequence< com::sun::star::i18n::Calendar >& calendarsSeq,
        sal_Int16 len, sal_Int16 item) throw( com::sun::star::uno::RuntimeException );
};

} } } }

#endif // _I18N_LOCALEDATA_HXX_
