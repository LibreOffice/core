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
#ifndef INCLUDED_I18NPOOL_INC_LOCALEDATA_HXX
#define INCLUDED_I18NPOOL_INC_LOCALEDATA_HXX

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <comphelper/processfactory.hxx>

#include <com/sun/star/i18n/XLocaleData4.hpp>


#include <cppuhelper/implbase.hxx>
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
#include <com/sun/star/uno/XInterface.hpp>
#include <rtl/ustring.hxx>
#include <vector>
#include <memory>
#include <osl/module.hxx>


#include <defaultnumberingprovider.hxx>
#include <com/sun/star/style/NumberingType.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

struct LocaleDataLookupTableItem;

namespace com { namespace sun { namespace star { namespace i18n {

inline bool operator ==(const css::lang::Locale& l1, const css::lang::Locale& l2) {
    return l1.Language == l2.Language && l1.Country == l2.Country && l1.Variant == l2.Variant;
};

class LocaleDataImpl : public cppu::WeakImplHelper
<
    css::i18n::XLocaleData4,
    css::lang::XServiceInfo
>
{
public:
    LocaleDataImpl();
    virtual ~LocaleDataImpl();

    static css::uno::Sequence< css::i18n::CalendarItem > downcastCalendarItems( const css::uno::Sequence< css::i18n::CalendarItem2 > & rCi );
    static css::i18n::Calendar downcastCalendar( const css::i18n::Calendar2 & rC );

    /** Generates a <Language>_<Country> or <Variant> (if Language=="qlt")
        string suitable as part of service name with all '-' replaced by '_' */
    static OUString getFirstLocaleServiceName( const css::lang::Locale & rLocale );
    /** Generates fallback strings suitable as parts of service names,
        excluding the one obtained via getFirstLocaleServiceName() */
    static ::std::vector< OUString > getFallbackLocaleServiceNames( const css::lang::Locale & rLocale );

    virtual LanguageCountryInfo SAL_CALL getLanguageCountryInfo( const css::lang::Locale& rLocale ) throw(css::uno::RuntimeException, std::exception) override;
    virtual LocaleDataItem SAL_CALL getLocaleItem( const css::lang::Locale& rLocale ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< Calendar2 > SAL_CALL getAllCalendars2( const css::lang::Locale& rLocale ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< Calendar > SAL_CALL getAllCalendars( const css::lang::Locale& rLocale ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< Currency > SAL_CALL getAllCurrencies( const css::lang::Locale& rLocale ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< Currency2 > SAL_CALL getAllCurrencies2( const css::lang::Locale& rLocale ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< FormatElement > SAL_CALL getAllFormats( const css::lang::Locale& rLocale ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< Implementation > SAL_CALL getCollatorImplementations( const css::lang::Locale& rLocale ) throw(css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getCollatorRuleByAlgorithm( const css::lang::Locale& rLocale, const OUString& algorithm ) throw(css::uno::RuntimeException, std::exception);
    virtual css::uno::Sequence< OUString > SAL_CALL getTransliterations( const css::lang::Locale& rLocale ) throw(css::uno::RuntimeException, std::exception) override;
    virtual ForbiddenCharacters SAL_CALL getForbiddenCharacters( const css::lang::Locale& rLocale ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getReservedWord( const css::lang::Locale& rLocale ) throw(css::uno::RuntimeException, std::exception) override ;
    virtual css::uno::Sequence< OUString > SAL_CALL getBreakIteratorRules( const css::lang::Locale& rLocale ) throw(css::uno::RuntimeException) ;
    virtual css::uno::Sequence< css::lang::Locale > SAL_CALL getAllInstalledLocaleNames() throw(css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence< OUString > SAL_CALL getSearchOptions( const css::lang::Locale& rLocale ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getCollationOptions( const css::lang::Locale& rLocale ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::uno::Sequence< beans::PropertyValue > > SAL_CALL getContinuousNumberingLevels( const css::lang::Locale& rLocale ) throw(css::uno::RuntimeException, std::exception);
    virtual css::uno::Sequence< css::uno::Reference< container::XIndexAccess > > SAL_CALL getOutlineNumberingLevels( const css::lang::Locale& rLocale ) throw(css::uno::RuntimeException, std::exception);

    // XLocaleData4
    virtual css::uno::Sequence< OUString > SAL_CALL getDateAcceptancePatterns( const css::lang::Locale& rLocale ) throw(css::uno::RuntimeException, std::exception) override;

    // following methods are used by indexentry service
    virtual css::uno::Sequence< OUString > SAL_CALL getIndexAlgorithm( const css::lang::Locale& rLocale ) throw(css::uno::RuntimeException);
    virtual OUString SAL_CALL getDefaultIndexAlgorithm( const css::lang::Locale& rLocale ) throw(css::uno::RuntimeException);
    virtual OUString SAL_CALL getIndexKeysByAlgorithm( const css::lang::Locale& rLocale, const OUString& algorithm ) throw(css::uno::RuntimeException);
    virtual OUString SAL_CALL getIndexModuleByAlgorithm( const css::lang::Locale& rLocale, const OUString& algorithm ) throw(css::uno::RuntimeException);
    virtual css::uno::Sequence< UnicodeScript > SAL_CALL getUnicodeScripts( const css::lang::Locale& rLocale ) throw(css::uno::RuntimeException);
    virtual css::uno::Sequence< OUString > SAL_CALL getFollowPageWords( const css::lang::Locale& rLocale ) throw(css::uno::RuntimeException);
    virtual bool SAL_CALL hasPhonetic( const css::lang::Locale& rLocale ) throw(css::uno::RuntimeException);
    virtual bool SAL_CALL isPhonetic( const css::lang::Locale& rLocale, const OUString& algorithm ) throw(css::uno::RuntimeException);
    virtual OUString SAL_CALL getHangingCharacters( const css::lang::Locale& rLocale ) throw(css::uno::RuntimeException, std::exception);

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception ) override;

private:
    friend bool operator ==(const css::lang::Locale& l1, const css::lang::Locale& l2);

    ::std::unique_ptr< LocaleDataLookupTableItem > cachedItem;
    css::i18n::Calendar2 ref_cal;
    OUString ref_name;

    oslGenericFunction SAL_CALL getFunctionSymbol( const css::lang::Locale& rLocale, const sal_Char* pFunction ) throw( css::uno::RuntimeException, std::exception );
    sal_Unicode ** SAL_CALL getIndexArray(const css::lang::Locale& rLocale, sal_Int16& indexCount);
    sal_Unicode ** SAL_CALL getIndexArrayForAlgorithm(const css::lang::Locale& rLocale, const OUString& rAlgorithm);
    css::uno::Sequence< css::i18n::CalendarItem2 > &
        getCalendarItemByName(const OUString& name,
        const css::lang::Locale& rLocale,
        const css::uno::Sequence< css::i18n::Calendar2 >& calendarsSeq,
        sal_Int16 item) throw( css::uno::RuntimeException );

    /// Helper to obtain a sequence of days, months, gmonths or eras.
    css::uno::Sequence< css::i18n::CalendarItem2 > getCalendarItems(
            sal_Unicode const * const * const allCalendars,
            sal_Int16 & rnOffset,
            const sal_Int16 nWhichItem,
            const sal_Int16 nCalendar,
            const css::lang::Locale & rLocale,
            const css::uno::Sequence< css::i18n::Calendar2 > & calendarsSeq )
        throw( css::uno::RuntimeException );

};

} } } }

#endif // INCLUDED_I18NPOOL_INC_LOCALEDATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
