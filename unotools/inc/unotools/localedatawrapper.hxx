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

#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#define _UNOTOOLS_LOCALEDATAWRAPPER_HXX

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <com/sun/star/i18n/XLocaleData4.hpp>
#include <com/sun/star/i18n/LocaleItem.hpp>
#include <com/sun/star/i18n/reservedWords.hpp>
#include <rtl/ustring.hxx>
#include <unotools/readwritemutexguard.hxx>
#include "unotools/unotoolsdllapi.h"

namespace com { namespace sun { namespace star {
    namespace lang {
        class XMultiServiceFactory;
    }
}}}
class Date;
class Time;
class CalendarWrapper;


enum DateFormat {
    MDY,
    DMY,
    YMD
};


enum MeasurementSystem {
    MEASURE_METRIC,
    MEASURE_US
};


class UNOTOOLS_DLLPUBLIC LocaleDataWrapper : private boost::noncopyable
{
    static  sal_uInt8                nLocaleDataChecking;    // 0:=dontknow, 1:=yes, 2:=no

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >    xSMgr;
    ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XLocaleData4 >            xLD;
    ::com::sun::star::lang::Locale                                                      aLocale;
    ::boost::shared_ptr< ::com::sun::star::i18n::Calendar2 >                            xDefaultCalendar;
    ::com::sun::star::i18n::LocaleDataItem                                              aLocaleDataItem;
    ::com::sun::star::uno::Sequence< ::rtl::OUString >                                  aReservedWordSeq;
    ::com::sun::star::uno::Sequence< ::rtl::OUString >                                  aDateAcceptancePatterns;
    ::com::sun::star::uno::Sequence< sal_Int32 >                                        aGrouping;
    // cached items
    rtl::OUString               aLocaleItem[::com::sun::star::i18n::LocaleItem::COUNT];
    rtl::OUString               aReservedWord[::com::sun::star::i18n::reservedWords::COUNT];
    rtl::OUString               aCurrSymbol;
    rtl::OUString               aCurrBankSymbol;
    int                         nDateFormat;
    int                         nLongDateFormat;
    sal_uInt16                      nCurrPositiveFormat;
    sal_uInt16                      nCurrNegativeFormat;
    sal_uInt16                      nCurrDigits;
    sal_Bool                        bLocaleDataItemValid;
    sal_Bool                        bReservedWordValid;
    mutable ::utl::ReadWriteMutex   aMutex;

    // dummies, to be implemented or provided by XML locale data
    sal_Unicode                 cCurrZeroChar;

                                // whenever Locale changes
    void                invalidateData();

    void                getOneLocaleItemImpl( sal_Int16 nItem );
    const rtl::OUString& getOneLocaleItem( sal_Int16 nItem ) const;

    void                getOneReservedWordImpl( sal_Int16 nWord );
    const rtl::OUString& getOneReservedWord( sal_Int16 nWord ) const;

    void                getCurrSymbolsImpl();
    void                getCurrFormatsImpl();

    void                scanCurrFormatImpl( const rtl::OUString& rCode,
                            sal_Int32 nStart, sal_Int32& nSign,
                            sal_Int32& nPar, sal_Int32& nNum,
                            sal_Int32& nBlank, sal_Int32& nSym );

    void                getDateFormatsImpl();
    DateFormat          scanDateFormatImpl( const rtl::OUString& rCode );

    void                getDefaultCalendarImpl();

    sal_Unicode*        ImplAddFormatNum( sal_Unicode* pBuf,
                            sal_Int64 nNumber, sal_uInt16 nDecimals,
                            sal_Bool bUseThousandSep, sal_Bool bTrailingZeros ) const;

    void                getDigitGroupingImpl();

public:
                                LocaleDataWrapper(
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & xSF,
                                    const ::com::sun::star::lang::Locale& rLocale
                                    );
                                ~LocaleDataWrapper();

    /** Get the service factory, meant to be able to create a CalendarWrapper
        from a LocaleDataWrapper. Note that the service factory may be
        non-existent if this LocaleDataWrapper was created without one and
        lives "on the grassland". The CalendarWrapper ctor can handle that
        though. */
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::lang::XMultiServiceFactory > & getServiceFactory()
        const { return xSMgr; }

    /// set a new Locale to request
            void                setLocale( const ::com::sun::star::lang::Locale& rLocale );

    /// get current requested Locale
    const ::com::sun::star::lang::Locale& getLocale() const;

    /// get current loaded Locale, which might differ from the requested Locale
    ::com::sun::star::lang::Locale getLoadedLocale() const;


    // Wrapper implementations of service LocaleData

    ::com::sun::star::i18n::LanguageCountryInfo getLanguageCountryInfo() const;
    ::com::sun::star::i18n::LocaleDataItem getLocaleItem() const;
    /// NOTE: this wraps XLocaleData3::getAllCalendars2() in fact.
    ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::Calendar2 > getAllCalendars() const;
    /// NOTE: this wraps XLocaleData2::getAllCurrencies2() in fact.
    ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::Currency2 > getAllCurrencies() const;
    ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::FormatElement > getAllFormats() const;
    ::com::sun::star::i18n::ForbiddenCharacters getForbiddenCharacters() const;
    ::com::sun::star::uno::Sequence< ::rtl::OUString > getReservedWord() const;
    ::com::sun::star::uno::Sequence< ::com::sun::star::lang::Locale > getAllInstalledLocaleNames() const;
    ::com::sun::star::uno::Sequence< ::rtl::OUString > getDateAcceptancePatterns() const;

    /** Override locale's date acceptance patterns.
        An empty sequence resets the patterns to the locale's pattern sequence.
     */
    void setDateAcceptancePatterns( const ::com::sun::star::uno::Sequence< ::rtl::OUString > & rPatterns );

    /// same as the wrapper implementation but static
    static ::com::sun::star::uno::Sequence< ::com::sun::star::lang::Locale > getInstalledLocaleNames();

    /** Get LanguageTypes for all installed locales which are unambiguous
        convertible back and forth between locale ISO strings and MS-LCID
        LanguageType. Upon the first time the function is called when
        locale data checking is enabled, messages are shown for locales not
        matching, excluding already known problems.
        (e.g. used in number formatter dialog init)
     */
    static ::com::sun::star::uno::Sequence< sal_uInt16 > getInstalledLanguageTypes();

    /// maps the LocaleData string to the International enum
    MeasurementSystem   mapMeasurementStringToEnum( const rtl::OUString& rMS ) const;

    /// Convenience method to obtain the default calendar.
    const ::boost::shared_ptr< ::com::sun::star::i18n::Calendar2 > getDefaultCalendar() const;

    /// Convenience method to obtain the day names of the default calendar.
    const ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::CalendarItem2 > getDefaultCalendarDays() const;

    /// Convenience method to obtain the month names of the default calendar.
    const ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::CalendarItem2 > getDefaultCalendarMonths() const;

    /** Obtain digit grouping. The usually known grouping by thousands (#,###)
        is actually only one of possible groupings. Another one, for example,
        used in India is group by 3 and then by 2 indefinitely (#,##,###). The
        integer sequence returned here specifies grouping from right to left
        (!), with a 0 entry designating the end of rules and the previous value
        to be repeated indefinitely. Hence the sequence {3,0} specifies the
        usual grouping by thousands, whereas the sequence {3,2,0} specifies
        Indian grouping. The sal_Int32* getConstArray() can be passed directly
        to the ::rtl::math::doubleToString() methods as argument for the
        pGroups parameter. */
    const ::com::sun::star::uno::Sequence< sal_Int32 > getDigitGrouping() const;

    // Functionality of class International methods, LocaleItem

    const rtl::OUString&       getDateSep() const
                                    { return getOneLocaleItem( ::com::sun::star::i18n::LocaleItem::DATE_SEPARATOR ); }
    const rtl::OUString&       getNumThousandSep() const
                                    { return getOneLocaleItem( ::com::sun::star::i18n::LocaleItem::THOUSAND_SEPARATOR ); }
    const rtl::OUString&       getNumDecimalSep() const
                                    { return getOneLocaleItem( ::com::sun::star::i18n::LocaleItem::DECIMAL_SEPARATOR ); }
    const rtl::OUString&       getTimeSep() const
                                    { return getOneLocaleItem( ::com::sun::star::i18n::LocaleItem::TIME_SEPARATOR ); }
    const rtl::OUString&       getTime100SecSep() const
                                    { return getOneLocaleItem( ::com::sun::star::i18n::LocaleItem::TIME_100SEC_SEPARATOR ); }
    const rtl::OUString&       getListSep() const
                            { return getOneLocaleItem( ::com::sun::star::i18n::LocaleItem::LIST_SEPARATOR ); }
    const rtl::OUString&       getQuotationMarkStart() const
                            { return getOneLocaleItem( ::com::sun::star::i18n::LocaleItem::SINGLE_QUOTATION_START ); }
    const rtl::OUString&       getQuotationMarkEnd() const
                            { return getOneLocaleItem( ::com::sun::star::i18n::LocaleItem::SINGLE_QUOTATION_END ); }
    const rtl::OUString&       getDoubleQuotationMarkStart() const
                            { return getOneLocaleItem( ::com::sun::star::i18n::LocaleItem::DOUBLE_QUOTATION_START ); }
    const rtl::OUString&       getDoubleQuotationMarkEnd() const
                            { return getOneLocaleItem( ::com::sun::star::i18n::LocaleItem::DOUBLE_QUOTATION_END ); }
    const rtl::OUString&       getMeasurementSystem() const
                            { return getOneLocaleItem( ::com::sun::star::i18n::LocaleItem::MEASUREMENT_SYSTEM ); }
    MeasurementSystem   getMeasurementSystemEnum() const
                            { return mapMeasurementStringToEnum( getOneLocaleItem( ::com::sun::star::i18n::LocaleItem::MEASUREMENT_SYSTEM ) ); }
    const rtl::OUString&       getTimeAM() const
                            { return getOneLocaleItem( ::com::sun::star::i18n::LocaleItem::TIME_AM ); }
    const rtl::OUString&       getTimePM() const
                            { return getOneLocaleItem( ::com::sun::star::i18n::LocaleItem::TIME_PM ); }
    const rtl::OUString&       getLongDateDayOfWeekSep() const
                            { return getOneLocaleItem( ::com::sun::star::i18n::LocaleItem::LONG_DATE_DAY_OF_WEEK_SEPARATOR ); }
    const rtl::OUString&       getLongDateDaySep() const
                            { return getOneLocaleItem( ::com::sun::star::i18n::LocaleItem::LONG_DATE_DAY_SEPARATOR ); }
    const rtl::OUString&       getLongDateMonthSep() const
                            { return getOneLocaleItem( ::com::sun::star::i18n::LocaleItem::LONG_DATE_MONTH_SEPARATOR ); }
    const rtl::OUString&       getLongDateYearSep() const
                            { return getOneLocaleItem( ::com::sun::star::i18n::LocaleItem::LONG_DATE_YEAR_SEPARATOR ); }

    // currency
    const rtl::OUString&    getCurrSymbol() const;
    const rtl::OUString&    getCurrBankSymbol() const;
    sal_uInt16              getCurrPositiveFormat() const;
    sal_uInt16              getCurrNegativeFormat() const;
    sal_uInt16              getCurrDigits() const;

    // simple date and time formatting
    DateFormat          getDateFormat() const;
    DateFormat          getLongDateFormat() const;
    /// only numerical values of Gregorian calendar
    rtl::OUString       getDate( const Date& rDate ) const;
    rtl::OUString       getTime( const Time& rTime, sal_Bool bSec = sal_True,
                            sal_Bool b100Sec = sal_False ) const;
    rtl::OUString       getDuration( const Time& rTime,
                            sal_Bool bSec = sal_True, sal_Bool b100Sec = sal_False ) const;

                        /** The CalendarWrapper already <b>MUST</b>
                            have loaded a calendar.
                            @param nDisplayDayOfWeek
                                0 := abbreviated name
                                1 := full name
                            @param bDayOfMonthWithLeadingZero
                                <FALSE/> := without leading zero
                                <TRUE/>  := with leading zero if <10
                            @param nDisplayMonth
                                0 := abbreviated name
                                1 := full name
                            @param bTwoDigitYear
                                <FALSE/> := full year
                                <TRUE/>  := year % 100
                         */
    rtl::OUString       getLongDate( const Date& rDate,
                            CalendarWrapper& rCal,
                            sal_Int16 nDisplayDayOfWeek = 1,
                            sal_Bool bDayOfMonthWithLeadingZero = sal_False,
                            sal_Int16 nDisplayMonth = 1,
                            sal_Bool bTwoDigitYear = sal_False
                            ) const;

                        /** Simple number formatting
                            @param nNumber
                                value * 10**nDecimals
                            @param bTrailingZeros
                            </sal_True>  := always display trailing zeros in
                                decimal places, even if integer value.
                            </sal_False> := trailing zeros are only displayed
                                if the value is not an integer value.
                         */
    rtl::OUString       getNum( sal_Int64 nNumber, sal_uInt16 nDecimals,
                            sal_Bool bUseThousandSep = sal_True,
                            sal_Bool bTrailingZeros = sal_True ) const;

                        /// "Secure" currency formatted string.
    rtl::OUString       getCurr( sal_Int64 nNumber, sal_uInt16 nDecimals,
                            const rtl::OUString& rCurrencySymbol,
                            sal_Bool bUseThousandSep = sal_True ) const;
                        /** Default currency formatted string, use with
                            care as default currency may change in any
                            locale, for example, DEM -> EUR */
    rtl::OUString       getCurr( sal_Int64 nNumber, sal_uInt16 nDecimals,
                                sal_Bool bUseThousandSep = sal_True ) const
                            { return getCurr( nNumber, nDecimals,
                                getCurrSymbol(), bUseThousandSep ); }

    // dummy returns, to be implemented
    inline  sal_Unicode         getCurrZeroChar() const
                                    { return cCurrZeroChar; }
    inline  sal_Bool                isNumLeadingZero() const
                                    { return sal_True; }
                                /// standard decimal places
    inline  sal_uInt16              getNumDigits() const
                                    { return 2; }
    inline  sal_Bool                isNumTrailingZeros() const
                                    { return sal_True; }


    // reserved words

    const rtl::OUString&        getTrueWord() const
                                    { return getOneReservedWord( ::com::sun::star::i18n::reservedWords::TRUE_WORD ); }
    const rtl::OUString&        getFalseWord() const
                                    { return getOneReservedWord( ::com::sun::star::i18n::reservedWords::FALSE_WORD ); }
    /// return a quarter string matching nQuarter (0..3) => "1st quarter" .. "4th quarter"
    const rtl::OUString&        getQuarterWord( sal_Int16 nQuarter ) const
                                    { return getOneReservedWord( ::com::sun::star::i18n::reservedWords::QUARTER1_WORD + nQuarter ); }
    const rtl::OUString&        getAboveWord() const
                                    { return getOneReservedWord( ::com::sun::star::i18n::reservedWords::ABOVE_WORD ); }
    const rtl::OUString&        getBelowWord() const
                                    { return getOneReservedWord( ::com::sun::star::i18n::reservedWords::BELOW_WORD ); }
    /// return a quarter abbreviation string matching nQuarter (0..3) => "Q1" .. "Q2"
    const rtl::OUString&        getQuarterAbbreviation( sal_Int16 nQuarter ) const
                                    { return getOneReservedWord( ::com::sun::star::i18n::reservedWords::QUARTER1_ABBREVIATION + nQuarter ); }

    /** Return whether locale data checks are enabled.
        Checks are enabled if the environment variable
        OOO_ENABLE_LOCALE_DATA_CHECKS is set to 'Y' or 'Yes' (or any other
        string starting with 'Y') or '1'.
        Also used in conjunction with the number formatter. */
    static  inline  bool        areChecksEnabled()
                                    {
                                        if (nLocaleDataChecking == 0)
                                            evaluateLocaleDataChecking();
                                        return nLocaleDataChecking == 1;
                                    }

    /** Append locale info to string, used with locale data checking.
        A string similar to "de_DE requested\n en_US loaded" is appended. */
    rtl::OUString appendLocaleInfo(const rtl::OUString& rDebugMsg) const;

    /** Ouput a message during locale data checking. The (UTF-8) string is
        written to stderr and in a non-product build or if DBG_UTIL is enabled
        also raised as an assertion message box. */
    static  void                outputCheckMessage( const rtl::OUString& rMsg );
    static  void                outputCheckMessage( const char* pStr);

private:
    static  void                evaluateLocaleDataChecking();
};


#endif // _UNOTOOLS_LOCALEDATAWRAPPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
