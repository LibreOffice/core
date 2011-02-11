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

#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#define _UNOTOOLS_LOCALEDATAWRAPPER_HXX

#include <tools/string.hxx>
#include <com/sun/star/i18n/XLocaleData2.hpp>
#include <com/sun/star/i18n/LocaleItem.hpp>
#include <com/sun/star/i18n/reservedWords.hpp>
#include <unotools/readwritemutexguard.hxx>
#include "unotools/unotoolsdllapi.h"

#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif


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


class UNOTOOLS_DLLPUBLIC LocaleDataWrapper
{
    static  sal_uInt8                nLocaleDataChecking;    // 0:=dontknow, 1:=yes, 2:=no

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >    xSMgr;
    ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XLocaleData2 >            xLD;
    ::com::sun::star::lang::Locale                                                      aLocale;
    ::boost::shared_ptr< ::com::sun::star::i18n::Calendar >                             xDefaultCalendar;
    ::com::sun::star::i18n::LocaleDataItem                                              aLocaleDataItem;
    ::com::sun::star::uno::Sequence< ::rtl::OUString >                                  aReservedWordSeq;
    ::com::sun::star::uno::Sequence< sal_Int32 >                                        aGrouping;
    // cached items
    String                      aLocaleItem[::com::sun::star::i18n::LocaleItem::COUNT];
    String                      aReservedWord[::com::sun::star::i18n::reservedWords::COUNT];
    String                      aCurrSymbol;
    String                      aCurrBankSymbol;
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


                                // not implemented, prevent usage
                                LocaleDataWrapper( const LocaleDataWrapper& );
            LocaleDataWrapper&  operator=( const LocaleDataWrapper& );

                                // whenever Locale changes
            void                invalidateData();

            void                getOneLocaleItemImpl( sal_Int16 nItem );
            const String&       getOneLocaleItem( sal_Int16 nItem ) const;

            void                getOneReservedWordImpl( sal_Int16 nWord );
            const String&       getOneReservedWord( sal_Int16 nWord ) const;

            void                getCurrSymbolsImpl();
            void                getCurrFormatsImpl();

            void                scanCurrFormatImpl( const String& rCode,
                                    xub_StrLen nStart, xub_StrLen& nSign,
                                    xub_StrLen& nPar, xub_StrLen& nNum,
                                    xub_StrLen& nBlank, xub_StrLen& nSym );

            void                getDateFormatsImpl();
            DateFormat          scanDateFormatImpl( const String& rCode );

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
    ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::Calendar > getAllCalendars() const;
    /// NOTE: this wraps XLocaleData2::getAllCurrencies2() in fact.
    ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::Currency2 > getAllCurrencies() const;
    ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::FormatElement > getAllFormats() const;
    ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::Implementation > getCollatorImplementations() const;
    ::com::sun::star::uno::Sequence< ::rtl::OUString > getTransliterations() const;
    ::com::sun::star::i18n::ForbiddenCharacters getForbiddenCharacters() const;
    ::com::sun::star::uno::Sequence< ::rtl::OUString > getReservedWord() const;
    ::com::sun::star::uno::Sequence< ::com::sun::star::lang::Locale > getAllInstalledLocaleNames() const;

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
            MeasurementSystem   mapMeasurementStringToEnum( const String& rMS ) const;

    /// Convenience method to obtain the default calendar.
    const ::boost::shared_ptr< ::com::sun::star::i18n::Calendar > getDefaultCalendar() const;

    /// Convenience method to obtain the day names of the default calendar.
    const ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::CalendarItem > getDefaultCalendarDays() const;

    /// Convenience method to obtain the month names of the default calendar.
    const ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::CalendarItem > getDefaultCalendarMonths() const;

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

    inline  const String&       getDateSep() const
                                    { return getOneLocaleItem( ::com::sun::star::i18n::LocaleItem::DATE_SEPARATOR ); }
    inline  const String&       getNumThousandSep() const
                                    { return getOneLocaleItem( ::com::sun::star::i18n::LocaleItem::THOUSAND_SEPARATOR ); }
    inline  const String&       getNumDecimalSep() const
                                    { return getOneLocaleItem( ::com::sun::star::i18n::LocaleItem::DECIMAL_SEPARATOR ); }
    inline  const String&       getTimeSep() const
                                    { return getOneLocaleItem( ::com::sun::star::i18n::LocaleItem::TIME_SEPARATOR ); }
    inline  const String&       getTime100SecSep() const
                                    { return getOneLocaleItem( ::com::sun::star::i18n::LocaleItem::TIME_100SEC_SEPARATOR ); }
    inline  const String&       getListSep() const
                                    { return getOneLocaleItem( ::com::sun::star::i18n::LocaleItem::LIST_SEPARATOR ); }
    inline  const String&       getQuotationMarkStart() const
                                    { return getOneLocaleItem( ::com::sun::star::i18n::LocaleItem::SINGLE_QUOTATION_START ); }
    inline  const String&       getQuotationMarkEnd() const
                                    { return getOneLocaleItem( ::com::sun::star::i18n::LocaleItem::SINGLE_QUOTATION_END ); }
    inline  const String&       getDoubleQuotationMarkStart() const
                                    { return getOneLocaleItem( ::com::sun::star::i18n::LocaleItem::DOUBLE_QUOTATION_START ); }
    inline  const String&       getDoubleQuotationMarkEnd() const
                                    { return getOneLocaleItem( ::com::sun::star::i18n::LocaleItem::DOUBLE_QUOTATION_END ); }
    inline  const String&       getMeasurementSystem() const
                                    { return getOneLocaleItem( ::com::sun::star::i18n::LocaleItem::MEASUREMENT_SYSTEM ); }
    inline  MeasurementSystem   getMeasurementSystemEnum() const
                                    { return mapMeasurementStringToEnum( getOneLocaleItem( ::com::sun::star::i18n::LocaleItem::MEASUREMENT_SYSTEM ) ); }
    inline  const String&       getTimeAM() const
                                    { return getOneLocaleItem( ::com::sun::star::i18n::LocaleItem::TIME_AM ); }
    inline  const String&       getTimePM() const
                                    { return getOneLocaleItem( ::com::sun::star::i18n::LocaleItem::TIME_PM ); }
    inline  const String&       getLongDateDayOfWeekSep() const
                                    { return getOneLocaleItem( ::com::sun::star::i18n::LocaleItem::LONG_DATE_DAY_OF_WEEK_SEPARATOR ); }
    inline  const String&       getLongDateDaySep() const
                                    { return getOneLocaleItem( ::com::sun::star::i18n::LocaleItem::LONG_DATE_DAY_SEPARATOR ); }
    inline  const String&       getLongDateMonthSep() const
                                    { return getOneLocaleItem( ::com::sun::star::i18n::LocaleItem::LONG_DATE_MONTH_SEPARATOR ); }
    inline  const String&       getLongDateYearSep() const
                                    { return getOneLocaleItem( ::com::sun::star::i18n::LocaleItem::LONG_DATE_YEAR_SEPARATOR ); }

    // currency
            const String&       getCurrSymbol() const;
            const String&       getCurrBankSymbol() const;
            sal_uInt16              getCurrPositiveFormat() const;
            sal_uInt16              getCurrNegativeFormat() const;
            sal_uInt16              getCurrDigits() const;

    // simple date and time formatting
            DateFormat          getDateFormat() const;
            DateFormat          getLongDateFormat() const;
                                /// only numerical values of Gregorian calendar
            String              getDate( const Date& rDate ) const;
            String              getTime( const Time& rTime, sal_Bool bSec = sal_True,
                                    sal_Bool b100Sec = sal_False ) const;
            String              getDuration( const Time& rTime,
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
            String              getLongDate( const Date& rDate,
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
            String              getNum( sal_Int64 nNumber, sal_uInt16 nDecimals,
                                    sal_Bool bUseThousandSep = sal_True,
                                    sal_Bool bTrailingZeros = sal_True ) const;

                                /// "Secure" currency formatted string.
            String              getCurr( sal_Int64 nNumber, sal_uInt16 nDecimals,
                                    const String& rCurrencySymbol,
                                    sal_Bool bUseThousandSep = sal_True ) const;
                                /** Default currency formatted string, use with
                                    care as default currency may change in any
                                    locale, for example, DEM -> EUR */
            String              getCurr( sal_Int64 nNumber, sal_uInt16 nDecimals,
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

    inline  const String&       getTrueWord() const
                                    { return getOneReservedWord( ::com::sun::star::i18n::reservedWords::TRUE_WORD ); }
    inline  const String&       getFalseWord() const
                                    { return getOneReservedWord( ::com::sun::star::i18n::reservedWords::FALSE_WORD ); }
    /// return a quarter string matching nQuarter (0..3) => "1st quarter" .. "4th quarter"
    inline  const String&       getQuarterWord( sal_Int16 nQuarter ) const
                                    { return getOneReservedWord( ::com::sun::star::i18n::reservedWords::QUARTER1_WORD + nQuarter ); }
    inline  const String&       getAboveWord() const
                                    { return getOneReservedWord( ::com::sun::star::i18n::reservedWords::ABOVE_WORD ); }
    inline  const String&       getBelowWord() const
                                    { return getOneReservedWord( ::com::sun::star::i18n::reservedWords::BELOW_WORD ); }
    /// return a quarter abbreviation string matching nQuarter (0..3) => "Q1" .. "Q2"
    inline  const String&       getQuarterAbbreviation( sal_Int16 nQuarter ) const
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
            String&             appendLocaleInfo( String& rDebugMsg ) const;

    /** Ouput a message during locale data checking. The (UTF-8) string is
        written to stderr and in a non-product build or if DBG_UTIL is enabled
        also raised as an assertion message box. */
    static  void                outputCheckMessage( const String& rMsg );
    static  void                outputCheckMessage( const char* pStr);

private:
    static  void                evaluateLocaleDataChecking();
};


#endif // _UNOTOOLS_LOCALEDATAWRAPPER_HXX
