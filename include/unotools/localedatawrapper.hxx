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

#ifndef INCLUDED_UNOTOOLS_LOCALEDATAWRAPPER_HXX
#define INCLUDED_UNOTOOLS_LOCALEDATAWRAPPER_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/i18n/LocaleItem.hpp>
#include <com/sun/star/i18n/LocaleDataItem2.hpp>
#include <com/sun/star/i18n/LanguageCountryInfo.hpp>
#include <com/sun/star/i18n/ForbiddenCharacters.hpp>
#include <com/sun/star/i18n/reservedWords.hpp>
#include <rtl/ustring.hxx>
#include <rtl/math.h>
#include <i18nlangtag/languagetag.hxx>
#include <unotools/unotoolsdllapi.h>
#include <memory>
#include <string_view>

namespace com::sun::star::uno { class XComponentContext; }
namespace com::sun::star::i18n { class XLocaleData5; }
namespace com::sun::star::i18n { struct Calendar2; }
namespace com::sun::star::i18n { struct Currency2; }
namespace com::sun::star::i18n { struct FormatElement; }
namespace com::sun::star::i18n { struct CalendarItem2; }

class Date;
namespace tools
{
class Time;
class Duration;
}
class CalendarWrapper;

enum class DateOrder {
    Invalid = -1,
    MDY = 0,
    DMY,
    YMD,
};

enum class LongDateOrder {
    Invalid = -1,
    MDY = 0,
    DMY,
    YMD,
    YDM
};

enum class MeasurementSystem {
    Metric,
    US
};

/**
 * This class can be accessed without locking because we load
 * all of the data in the constructor.
 */
class UNOTOOLS_DLLPUBLIC LocaleDataWrapper
{
    static  sal_uInt8                nLocaleDataChecking;    // 0:=dontknow, 1:=yes, 2:=no

    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    css::uno::Reference< css::i18n::XLocaleData5 >     xLD;
    LanguageTag                                        maLanguageTag;
    std::shared_ptr< css::i18n::Calendar2 >            xDefaultCalendar;
    std::shared_ptr< css::i18n::Calendar2 >            xSecondaryCalendar;
    css::i18n::LocaleDataItem2                         aLocaleDataItem;
    css::uno::Sequence< OUString >                     aDateAcceptancePatterns;
    css::uno::Sequence< sal_Int32 >                    aGrouping;
    // cached items
    OUString                aLocaleItem[css::i18n::LocaleItem::COUNT2];
    std::vector<OUString>   aReservedWords;
    OUString                aCurrSymbol;
    OUString                aCurrBankSymbol;
    DateOrder               nDateOrder;
    LongDateOrder           nLongDateOrder;
    sal_uInt16              nCurrPositiveFormat;
    sal_uInt16              nCurrNegativeFormat;
    sal_uInt16              nCurrDigits;

    void                loadData();
    void                loadDateAcceptancePatterns(const std::vector<OUString> & rOverrideDateAcceptancePatterns);

    const OUString&     getOneLocaleItem( sal_Int16 nItem ) const;

    const OUString&     getOneReservedWord( sal_Int16 nWord ) const;

    void                loadCurrencyFormats();

    void                scanCurrFormatImpl( std::u16string_view rCode,
                            sal_Int32 nStart, sal_Int32& nSign,
                            sal_Int32& nPar, sal_Int32& nNum,
                            sal_Int32& nBlank, sal_Int32& nSym ) const;

    void                loadDateOrders();
    LongDateOrder       scanDateOrderImpl( std::u16string_view rCode ) const;

    void                ImplAddFormatNum( rtl::OUStringBuffer& rBuf,
                            sal_Int64 nNumber, sal_uInt16 nDecimals,
                            bool bUseThousandSep, bool bTrailingZeros ) const;

    void                loadDigitGrouping();

public:
    LocaleDataWrapper(
        const css::uno::Reference< css::uno::XComponentContext > & rxContext,
        LanguageTag aLanguageTag
        );
    /**
        @param rOverrideDateAcceptancePatterns Override locale's date acceptance patterns.
            An empty sequence resets the patterns to the locale's pattern sequence.
     */
    LocaleDataWrapper(
        LanguageTag aLanguageTag,
        const std::vector<OUString> & rOverrideDateAcceptancePatterns = {}
        );
    ~LocaleDataWrapper();

    /** Get the service factory, meant to be able to create a CalendarWrapper
        from a LocaleDataWrapper. Note that the service factory may be
        non-existent if this LocaleDataWrapper was created without one and
        lives "on the grassland". The CalendarWrapper ctor can handle that
        though. */
    const css::uno::Reference<
        css::uno::XComponentContext > & getComponentContext()
        const { return m_xContext; }

    /// get current requested Locale
    const   LanguageTag&        getLanguageTag() const;

    /// get current loaded Locale, which might differ from the requested Locale
    LanguageTag                 getLoadedLanguageTag() const;

    // Wrapper implementations of service LocaleData

    css::i18n::LanguageCountryInfo getLanguageCountryInfo() const;
    /// NOTE: this wraps XLocaleData5::getLocaleItem2() in fact.
    const css::i18n::LocaleDataItem2& getLocaleItem() const;
    /// NOTE: this wraps XLocaleData3::getAllCalendars2() in fact.
    css::uno::Sequence< css::i18n::Calendar2 > getAllCalendars() const;
    /// NOTE: this wraps XLocaleData2::getAllCurrencies2() in fact.
    css::uno::Sequence< css::i18n::Currency2 > getAllCurrencies() const;
    css::uno::Sequence< css::i18n::FormatElement > getAllFormats() const;
    css::i18n::ForbiddenCharacters getForbiddenCharacters() const;
    const css::uno::Sequence< css::lang::Locale > & getAllInstalledLocaleNames() const;
    const css::uno::Sequence< OUString > & getDateAcceptancePatterns() const;


    /// same as the wrapper implementation but static
    static const css::uno::Sequence< css::lang::Locale > & getInstalledLocaleNames();

    /** Get LanguageTypes for all installed locales which are unambiguous
        convertible back and forth between locale ISO strings and MS-LCID
        LanguageType. Upon the first time the function is called when
        locale data checking is enabled, messages are shown for locales not
        matching, excluding already known problems.
        (e.g. used in number formatter dialog init)
     */
    static const std::vector< LanguageType > & getInstalledLanguageTypes();

    /// maps the LocaleData string to the International enum
    static MeasurementSystem mapMeasurementStringToEnum( std::u16string_view rMS );

    /// Convenience method to obtain the default calendar.
    const std::shared_ptr< css::i18n::Calendar2 >& getDefaultCalendar() const;

    /// Convenience method to obtain the day names of the default calendar.
    css::uno::Sequence< css::i18n::CalendarItem2 > const & getDefaultCalendarDays() const;

    /// Convenience method to obtain the month names of the default calendar.
    css::uno::Sequence< css::i18n::CalendarItem2 > const & getDefaultCalendarMonths() const;

    /** If the secondary calendar, if any, is of the name passed AND number
        formats using it usually use the E or EE keyword (EC|EEC). */
    bool doesSecondaryCalendarUseEC( std::u16string_view rName ) const;

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
    const css::uno::Sequence< sal_Int32 >& getDigitGrouping() const;

    // Functionality of class International methods, LocaleItem

    const OUString&       getDateSep() const
                                    { return getOneLocaleItem( css::i18n::LocaleItem::DATE_SEPARATOR ); }
    const OUString&       getNumThousandSep() const
                                    { return getOneLocaleItem( css::i18n::LocaleItem::THOUSAND_SEPARATOR ); }
    const OUString&       getNumDecimalSep() const
                                    { return getOneLocaleItem( css::i18n::LocaleItem::DECIMAL_SEPARATOR ); }
    const OUString&       getNumDecimalSepAlt() const
                                    { return getOneLocaleItem( css::i18n::LocaleItem::DECIMAL_SEPARATOR_ALTERNATIVE ); }
    const OUString&       getTimeSep() const
                                    { return getOneLocaleItem( css::i18n::LocaleItem::TIME_SEPARATOR ); }
    const OUString&       getTime100SecSep() const
                                    { return getOneLocaleItem( css::i18n::LocaleItem::TIME_100SEC_SEPARATOR ); }
    const OUString&       getListSep() const
                            { return getOneLocaleItem( css::i18n::LocaleItem::LIST_SEPARATOR ); }
    const OUString&       getQuotationMarkStart() const
                            { return getOneLocaleItem( css::i18n::LocaleItem::SINGLE_QUOTATION_START ); }
    const OUString&       getQuotationMarkEnd() const
                            { return getOneLocaleItem( css::i18n::LocaleItem::SINGLE_QUOTATION_END ); }
    const OUString&       getDoubleQuotationMarkStart() const
                            { return getOneLocaleItem( css::i18n::LocaleItem::DOUBLE_QUOTATION_START ); }
    const OUString&       getDoubleQuotationMarkEnd() const
                            { return getOneLocaleItem( css::i18n::LocaleItem::DOUBLE_QUOTATION_END ); }
    MeasurementSystem     getMeasurementSystemEnum() const
                            { return mapMeasurementStringToEnum( getOneLocaleItem( css::i18n::LocaleItem::MEASUREMENT_SYSTEM ) ); }
    const OUString&       getTimeAM() const
                            { return getOneLocaleItem( css::i18n::LocaleItem::TIME_AM ); }
    const OUString&       getTimePM() const
                            { return getOneLocaleItem( css::i18n::LocaleItem::TIME_PM ); }
    const OUString&       getLongDateDayOfWeekSep() const
                            { return getOneLocaleItem( css::i18n::LocaleItem::LONG_DATE_DAY_OF_WEEK_SEPARATOR ); }
    const OUString&       getLongDateDaySep() const
                            { return getOneLocaleItem( css::i18n::LocaleItem::LONG_DATE_DAY_SEPARATOR ); }
    const OUString&       getLongDateMonthSep() const
                            { return getOneLocaleItem( css::i18n::LocaleItem::LONG_DATE_MONTH_SEPARATOR ); }
    const OUString&       getLongDateYearSep() const
                            { return getOneLocaleItem( css::i18n::LocaleItem::LONG_DATE_YEAR_SEPARATOR ); }

    /** A wrapper around rtl::math::stringToDouble() using the locale dependent
        decimal separator, group separator, and if needed decimal separator
        alternative.

        The decimal separator is tried first, if the conversion does not match
        the entire string then the decimal separator alternative is tried if it
        occurs in the string and was the reason to stop.

        Leading blanks are skipped, trailing blanks are not skipped. The number
        is parsed up to the first non-floating point number character, same as
        rtl::math::stringToDouble() does. The caller is responsible for proper
        error checking and end comparison.

        @param  aString
                The string to parse as floating point number.
        @param  bUseGroupSep
                Whether group separator is used/accepted during parsing.
        @param  pStatus
                Pointer to receive the conversion status as in
                rtl::math::stringToDouble().
        @param  pParseEnd
                Pointer to receive the parse end (exclusive) as in
                rtl::math::stringToDouble().
        @return The floating point number as parsed.
     */
    double              stringToDouble( std::u16string_view aString, bool bUseGroupSep,
                                        rtl_math_ConversionStatus* pStatus, sal_Int32* pParseEnd ) const;

    /** A wrapper around rtl_math_uStringToDouble() using the locale dependent
        decimal separator, group separator, and if needed decimal separator
        alternative.

        The decimal separator is tried first, if the conversion does not match
        the entire string then the decimal separator alternative is tried if it
        occurs in the string and was the reason to stop.

        Leading blanks are skipped, trailing blanks are not skipped. The number
        is parsed up to the first non-floating point number character, same as
        rtl_math_uStringToDouble() does. The caller is responsible for proper
        error checking and end comparison.

        @param  pBegin
                The string position to start parsing a floating point number.
        @param  pEnd
                The string position to stop parsing, exclusive.
        @param  bUseGroupSep
                Whether group separator is used/accepted during parsing.
        @param  pStatus
                Pointer to receive the conversion status as in
                rtl_math_uStringToDouble().
        @param  pParseEnd
                Pointer to receive the parse end (exclusive) as in
                rtl_math_uStringToDouble().
        @return The floating point number as parsed.
     */
    double              stringToDouble( const sal_Unicode* pBegin, const sal_Unicode* pEnd, bool bUseGroupSep,
                                        rtl_math_ConversionStatus* pStatus, const sal_Unicode** ppParseEnd ) const;

    // currency
    const OUString&    getCurrSymbol() const;
    const OUString&    getCurrBankSymbol() const;
    sal_uInt16              getCurrPositiveFormat() const;
    sal_uInt16              getCurrNegativeFormat() const;
    sal_uInt16              getCurrDigits() const;

    // simple date and time formatting
    DateOrder          getDateOrder() const;
    LongDateOrder      getLongDateOrder() const;
    /// only numerical values of Gregorian calendar
    OUString       getDate( const Date& rDate ) const;
    OUString       getTime( const tools::Time& rTime, bool bSec = true,
                            bool b100Sec = false ) const;
    OUString       getDuration( const tools::Duration& rDuration,
                            bool bSec = true, bool b100Sec = false ) const;

                        /** Simple number formatting
                            @param nNumber
                                value * 10**nDecimals
                            @param bTrailingZeros
                            </sal_True>  := always display trailing zeros in
                                decimal places, even if integer value.
                            </sal_False> := trailing zeros are only displayed
                                if the value is not an integer value.
                         */
    OUString       getNum( sal_Int64 nNumber, sal_uInt16 nDecimals,
                            bool bUseThousandSep = true,
                            bool bTrailingZeros = true ) const;

                        /// "Secure" currency formatted string.
    OUString       getCurr( sal_Int64 nNumber, sal_uInt16 nDecimals,
                            std::u16string_view rCurrencySymbol,
                            bool bUseThousandSep = true ) const;

    // dummy returns, to be implemented
    sal_Unicode         getCurrZeroChar() const
                                    { return '0'; }
    static bool         isNumLeadingZero()
                                    { return true; }
                                /// standard decimal places
    static sal_uInt16    getNumDigits()
                                    { return 2; }
    static bool         isNumTrailingZeros()
                                    { return true; }

    // reserved words

    const OUString&        getTrueWord() const
                                    { return getOneReservedWord( css::i18n::reservedWords::TRUE_WORD ); }
    const OUString&        getFalseWord() const
                                    { return getOneReservedWord( css::i18n::reservedWords::FALSE_WORD ); }
    const OUString&        getAboveWord() const
                                    { return getOneReservedWord( css::i18n::reservedWords::ABOVE_WORD ); }
    const OUString&        getBelowWord() const
                                    { return getOneReservedWord( css::i18n::reservedWords::BELOW_WORD ); }
    /// return a quarter abbreviation string matching nQuarter (0..3) => "Q1" .. "Q2"
    const OUString&        getQuarterAbbreviation( sal_Int16 nQuarter ) const
                                    { return getOneReservedWord( css::i18n::reservedWords::QUARTER1_ABBREVIATION + nQuarter ); }

    /** Return whether locale data checks are enabled.
        Checks are enabled if the environment variable
        OOO_ENABLE_LOCALE_DATA_CHECKS is set to 'Y' or 'Yes' (or any other
        string starting with 'Y') or '1'.
        Also used in conjunction with the number formatter. */
    static  bool        areChecksEnabled()
                                    {
                                        if (nLocaleDataChecking == 0)
                                            evaluateLocaleDataChecking();
                                        return nLocaleDataChecking == 1;
                                    }

    /** Append locale info to string, used with locale data checking.
        A string similar to "de_DE requested\n en_US loaded" is appended. */
    OUString appendLocaleInfo(std::u16string_view rDebugMsg) const;

    /** Output a message during locale data checking. The (UTF-8) string is
        written to stderr and in a non-product build or if DBG_UTIL is enabled
        also raised as an assertion message box. */
    static  void                outputCheckMessage( std::u16string_view rMsg );
    static  void                outputCheckMessage( const char* pStr);

    LocaleDataWrapper(const LocaleDataWrapper&) = delete;
    LocaleDataWrapper& operator=(const LocaleDataWrapper&) = delete;

private:

    const css::lang::Locale &  getMyLocale() const;

    static  void                evaluateLocaleDataChecking();
};

#endif // INCLUDED_UNOTOOLS_LOCALEDATAWRAPPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
