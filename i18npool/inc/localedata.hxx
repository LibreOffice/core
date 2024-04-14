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
#pragma once

#include <com/sun/star/i18n/XLocaleData5.hpp>


#include <cppuhelper/implbase.hxx>

#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/i18n/Calendar.hpp>
#include <com/sun/star/i18n/LocaleDataItem2.hpp>
#include <com/sun/star/i18n/UnicodeScript.hpp>
#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <vector>
#include <string_view>
#include <optional>
#include <osl/module.hxx>


#include <com/sun/star/lang/XServiceInfo.hpp>

namespace com::sun::star::beans { struct PropertyValue; }
namespace com::sun::star::container { class XIndexAccess; }
namespace com::sun::star::i18n { struct Currency; }
namespace com::sun::star::i18n { struct Currency2; }
namespace com::sun::star::i18n { struct FormatElement; }
namespace com::sun::star::lang { struct Locale; }

struct LocaleDataLookupTableItem;

namespace i18npool {

enum class DataLocaleLibrary {
    EN,
    ES,
    EURO,
    OTHERS
};


struct LocaleDataLookupTableItem
{
    DataLocaleLibrary dllName;
    osl::Module *module;
    const char* localeName;
    css::lang::Locale aLocale;

    LocaleDataLookupTableItem(DataLocaleLibrary name, osl::Module* m, const char* lname) : dllName(std::move(name)), module(m), localeName(lname)
    {
    }
    bool equals(const css::lang::Locale& rLocale) const
    {
        return (rLocale == aLocale);
    }
};

class LocaleDataImpl final : public cppu::WeakImplHelper
<
    css::i18n::XLocaleData5,
    css::lang::XServiceInfo
>
{
public:
    LocaleDataImpl();
    virtual ~LocaleDataImpl() override;

    static rtl::Reference<LocaleDataImpl> get() { return new LocaleDataImpl; }

    static css::uno::Sequence< css::i18n::CalendarItem > downcastCalendarItems( const css::uno::Sequence< css::i18n::CalendarItem2 > & rCi );
    static css::i18n::Calendar downcastCalendar( const css::i18n::Calendar2 & rC );

    /** Generates a <Language>_<Country> or <Variant> (if Language=="qlt")
        string suitable as part of service name with all '-' replaced by '_' */
    static OUString getFirstLocaleServiceName( const css::lang::Locale & rLocale );
    /** Generates fallback strings suitable as parts of service names,
        excluding the one obtained via getFirstLocaleServiceName() */
    static ::std::vector< OUString > getFallbackLocaleServiceNames( const css::lang::Locale & rLocale );

    virtual css::i18n::LanguageCountryInfo SAL_CALL getLanguageCountryInfo( const css::lang::Locale& rLocale ) override;
    virtual css::i18n::LocaleDataItem SAL_CALL getLocaleItem( const css::lang::Locale& rLocale ) override;
    virtual css::uno::Sequence< css::i18n::Calendar2 > SAL_CALL getAllCalendars2( const css::lang::Locale& rLocale ) override;
    virtual css::uno::Sequence< css::i18n::Calendar > SAL_CALL getAllCalendars( const css::lang::Locale& rLocale ) override;
    virtual css::uno::Sequence< css::i18n::Currency > SAL_CALL getAllCurrencies( const css::lang::Locale& rLocale ) override;
    virtual css::uno::Sequence< css::i18n::Currency2 > SAL_CALL getAllCurrencies2( const css::lang::Locale& rLocale ) override;
    virtual css::uno::Sequence< css::i18n::FormatElement > SAL_CALL getAllFormats( const css::lang::Locale& rLocale ) override;
    virtual css::uno::Sequence< css::i18n::Implementation > SAL_CALL getCollatorImplementations( const css::lang::Locale& rLocale ) override;
    /// @throws css::uno::RuntimeException
    OUString getCollatorRuleByAlgorithm( const css::lang::Locale& rLocale, std::u16string_view algorithm );
    virtual css::uno::Sequence< OUString > SAL_CALL getTransliterations( const css::lang::Locale& rLocale ) override;
    virtual css::i18n::ForbiddenCharacters SAL_CALL getForbiddenCharacters( const css::lang::Locale& rLocale ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getReservedWord( const css::lang::Locale& rLocale ) override ;
    /// @throws css::uno::RuntimeException
    css::uno::Sequence< OUString > getBreakIteratorRules( const css::lang::Locale& rLocale ) ;
    virtual css::uno::Sequence< css::lang::Locale > SAL_CALL getAllInstalledLocaleNames() override;

    virtual css::uno::Sequence< OUString > SAL_CALL getSearchOptions( const css::lang::Locale& rLocale ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getCollationOptions( const css::lang::Locale& rLocale ) override;
    /// @throws css::uno::RuntimeException
    css::uno::Sequence< css::uno::Sequence< css::beans::PropertyValue > > getContinuousNumberingLevels( const css::lang::Locale& rLocale );
    /// @throws css::uno::RuntimeException
    css::uno::Sequence< css::uno::Reference< css::container::XIndexAccess > > getOutlineNumberingLevels( const css::lang::Locale& rLocale );

    // XLocaleData4
    virtual css::uno::Sequence< OUString > SAL_CALL getDateAcceptancePatterns( const css::lang::Locale& rLocale ) override;

    // XLocaleData5
    virtual css::i18n::LocaleDataItem2 SAL_CALL getLocaleItem2( const css::lang::Locale& rLocale ) override;

    // following methods are used by indexentry service
    /// @throws css::uno::RuntimeException
    css::uno::Sequence< OUString > getIndexAlgorithm( const css::lang::Locale& rLocale );
    /// @throws css::uno::RuntimeException
    OUString getDefaultIndexAlgorithm( const css::lang::Locale& rLocale );
    /// @throws css::uno::RuntimeException
    OUString getIndexKeysByAlgorithm(
        const css::lang::Locale& rLocale, std::u16string_view algorithm );
    /// @throws css::uno::RuntimeException
    OUString getIndexModuleByAlgorithm( const css::lang::Locale& rLocale, std::u16string_view algorithm );
    /// @throws css::uno::RuntimeException
    css::uno::Sequence< css::i18n::UnicodeScript > getUnicodeScripts( const css::lang::Locale& rLocale );
    /// @throws css::uno::RuntimeException
    css::uno::Sequence< OUString > getFollowPageWords( const css::lang::Locale& rLocale );
    /// @throws css::uno::RuntimeException
    bool hasPhonetic( const css::lang::Locale& rLocale );
    /// @throws css::uno::RuntimeException
    bool isPhonetic( const css::lang::Locale& rLocale, std::u16string_view algorithm );
    /// @throws css::uno::RuntimeException
    OUString getHangingCharacters( const css::lang::Locale& rLocale );

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

private:
    ::std::optional< LocaleDataLookupTableItem > moCachedItem;
    css::i18n::Calendar2 ref_cal;
    OUString ref_name;

    /// @throws css::uno::RuntimeException
    oslGenericFunction getFunctionSymbol( const css::lang::Locale& rLocale, const char* pFunction );
    OUString const * getIndexArray(const css::lang::Locale& rLocale, sal_Int16& indexCount);
    OUString const * getIndexArrayForAlgorithm(const css::lang::Locale& rLocale, std::u16string_view rAlgorithm);
    /// @throws css::uno::RuntimeException
    css::uno::Sequence< css::i18n::CalendarItem2 > &
        getCalendarItemByName(const OUString& name,
        const css::lang::Locale& rLocale,
        const css::uno::Sequence< css::i18n::Calendar2 >& calendarsSeq,
        sal_Int16 item);

    /// Helper to obtain a sequence of days, months, gmonths or eras.
    ///
    /// @throws css::uno::RuntimeException
    css::uno::Sequence< css::i18n::CalendarItem2 > getCalendarItems(
            OUString const * allCalendars,
            sal_Int16 & rnOffset,
            const sal_Int16 nWhichItem,
            const sal_Int16 nCalendar,
            const css::lang::Locale & rLocale,
            const css::uno::Sequence< css::i18n::Calendar2 > & calendarsSeq );

};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
