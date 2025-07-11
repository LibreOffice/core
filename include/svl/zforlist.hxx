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
#ifndef INCLUDED_SVL_ZFORLIST_HXX
#define INCLUDED_SVL_ZFORLIST_HXX

#include <config_options.h>
#include <svl/svldllapi.h>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>
#include <o3tl/typed_flags_set.hxx>
#include <i18nlangtag/lang.h>
#include <com/sun/star/util/NumberFormat.hpp>
#include <unotools/localedatawrapper.hxx>

#include <map>
#include <unordered_map>

namespace com::sun::star::i18n { struct Currency; }

class SvNumberformat;

#define SV_COUNTRY_LANGUAGE_OFFSET     10000  // Max count of formats per country/language
#define SV_MAX_COUNT_STANDARD_FORMATS  100    // Max count of builtin default formats per CL

constexpr size_t NF_MAX_FORMAT_SYMBOLS = 100;

/// The built-in @ Text format, offset within a locale, key in the locale the
/// number formatter was constructed with.
constexpr sal_uInt32 NF_STANDARD_FORMAT_TEXT = SV_MAX_COUNT_STANDARD_FORMATS;

constexpr sal_uInt32 NUMBERFORMAT_ENTRY_NOT_FOUND  = 0xffffffff;   /// MAX_ULONG

enum class SvNumFormatType : sal_Int16
{
    /** selects all number formats.
      */
     ALL = css::util::NumberFormat::ALL, // 0
     /** selects only user-defined number formats.
      */
     DEFINED = css::util::NumberFormat::DEFINED,  // 1
     /** selects date formats.
      */
     DATE = css::util::NumberFormat::DATE, // 2
     /** selects time formats.
      */
     TIME = css::util::NumberFormat::TIME, // 4
     /** selects currency formats.
      */
     CURRENCY = css::util::NumberFormat::CURRENCY,  // 8
     /** selects decimal number formats.
      */
     NUMBER = css::util::NumberFormat::NUMBER, // 16
     /** selects scientific number formats.
      */
     SCIENTIFIC = css::util::NumberFormat::SCIENTIFIC, // 32
     /** selects number formats for fractions.
      */
     FRACTION = css::util::NumberFormat::FRACTION, // 64
     /** selects percentage number formats.
      */
     PERCENT = css::util::NumberFormat::PERCENT, // 128
     /** selects text number formats.
      */
     TEXT = css::util::NumberFormat::TEXT, // 256
     /** selects number formats which contain date and time.
      */
     DATETIME = DATE | TIME, // 6
     /** selects boolean number formats.
      */
     LOGICAL = css::util::NumberFormat::LOGICAL, // 1024
     /** is used as a return value if no format exists.
      */
     UNDEFINED = css::util::NumberFormat::UNDEFINED, // 2048
     /** @internal is used to flag an empty sub format.
         @since LibreOffice 5.1
      */
     EMPTY = css::util::NumberFormat::EMPTY, // 4096
    /** @internal selects a time duration format.
        8192 + TIME (4)
        @since LibreOffice 6.2
     */
     DURATION = css::util::NumberFormat::DURATION, // 8196
};
namespace o3tl {
    template<> struct typed_flags<SvNumFormatType> : is_typed_flags<SvNumFormatType, 0x3dff> {};
}

/** enum values for <method>SvNumberFormatter::GetFormatIndex</method>

    <p>
     Builtin standard formats, order should be also the arrangement in the
    dialog list box representation.</p>

    <p>
     Date specials:<ul>
    <li>SYSTEM: As set in System Regional Settings.
    <li>SYS: short/long defined, order and separators from System Regional Settings.
    <li>DEF: short/long and order defined, separators from System Regional Settings.
    <li>DIN: all settings hard coded as DIN (Deutsche Industrie Norm) and EN (European Norm) require.
    <li>all other: hard coded
    </ul>

    Do NOT insert any new values!
    The values here correspond with those in offapi/com/sun/star/i18n/NumberFormatIndex.idl
    You may append values though after NF_INDEX_TABLE_LOCALE_DATA_DEFAULTS.
 */
enum NfIndexTableOffset
{
    NF_NUMERIC_START = 0,

    NF_NUMBER_START = NF_NUMERIC_START,
    NF_NUMBER_STANDARD = NF_NUMBER_START,   // Standard/General
    NF_NUMBER_INT,                          // 0
    NF_NUMBER_DEC2,                         // 0.00
    NF_NUMBER_1000INT,                      // #,##0
    NF_NUMBER_1000DEC2,                     // #,##0.00
    NF_NUMBER_SYSTEM,                       // #,##0.00 or whatever is set in System Regional Settings
    NF_NUMBER_END = NF_NUMBER_SYSTEM,

    NF_SCIENTIFIC_START,
    NF_SCIENTIFIC_000E000 = NF_SCIENTIFIC_START,    // 0.00E+000
    NF_SCIENTIFIC_000E00,                           // 0.00E+00
    NF_SCIENTIFIC_END = NF_SCIENTIFIC_000E00,

    NF_PERCENT_START,
    NF_PERCENT_INT = NF_PERCENT_START,      // 0%
    NF_PERCENT_DEC2,                        // 0.00%
    NF_PERCENT_END = NF_PERCENT_DEC2,

    NF_FRACTION_START,
    NF_FRACTION_1D = NF_FRACTION_START,      // # ?/?
    NF_FRACTION_2D,                          // # ??/??
    NF_FRACTION_END = NF_FRACTION_2D,

    NF_NUMERIC_END = NF_FRACTION_END,

    NF_CURRENCY_START,
    NF_CURRENCY_1000INT = NF_CURRENCY_START,// #,##0 DM
    NF_CURRENCY_1000DEC2,                   // #,##0.00 DM
    NF_CURRENCY_1000INT_RED,                // #,##0 DM         negative in red
    NF_CURRENCY_1000DEC2_RED,               // #,##0.00 DM      negative in red
    NF_CURRENCY_1000DEC2_CCC,               // #,##0.00 DEM     currency abbreviation
    NF_CURRENCY_1000DEC2_DASHED,            // #,##0.-- DM
    NF_CURRENCY_END = NF_CURRENCY_1000DEC2_DASHED,

    NF_DATE_START,
    NF_DATE_SYSTEM_SHORT = NF_DATE_START,   // 08.10.97
    NF_DATE_SYSTEM_LONG,                    // Wednesday, 8. October 1997
    NF_DATE_SYS_DDMMYY,                     // 08.10.97
    NF_DATE_SYS_DDMMYYYY,                   // 08.10.1997                   THE edit format, formatindex="21"
    NF_DATE_SYS_DMMMYY,                     // 8. Oct 97
    NF_DATE_SYS_DMMMYYYY,                   // 8. Oct 1997
    NF_DATE_DIN_DMMMYYYY,                   // 8. Oct. 1997                 DIN
    NF_DATE_SYS_DMMMMYYYY,                  // 8. October 1997
    NF_DATE_DIN_DMMMMYYYY,                  // 8. October 1997              DIN
    NF_DATE_SYS_NNDMMMYY,                   // Wed, 8. Okt 97
    NF_DATE_DEF_NNDDMMMYY,                  // Wed 08.Okt 97
    NF_DATE_SYS_NNDMMMMYYYY,                // Wed, 8. Oktober 1997
    NF_DATE_SYS_NNNNDMMMMYYYY,              // Wednesday, 8. Oktober 1997
    NF_DATE_DIN_MMDD,                       // 10-08                        DIN
    NF_DATE_DIN_YYMMDD,                     // 97-10-08                     DIN
    NF_DATE_DIN_YYYYMMDD,                   // 1997-10-08                   DIN
    NF_DATE_ISO_YYYYMMDD = NF_DATE_DIN_YYYYMMDD, // 1997-10-08              ISO clarify with name, formatindex="33"
    NF_DATE_SYS_MMYY,                       // 10.97
    NF_DATE_SYS_DDMMM,                      // 08.Oct
    NF_DATE_MMMM,                           // October
    NF_DATE_QQJJ,                           // 4. Quarter 97
    NF_DATE_WW,                             // week of year
    NF_DATE_END = NF_DATE_WW,

    NF_TIME_START,
    NF_TIME_HHMM = NF_TIME_START,           // HH:MM
    NF_TIME_HHMMSS,                         // HH:MM:SS
    NF_TIME_HHMMAMPM,                       // HH:MM AM/PM
    NF_TIME_HHMMSSAMPM,                     // HH:MM:SS AM/PM
    NF_TIME_HH_MMSS,                        // [HH]:MM:SS                   formatindex="43"
    NF_TIME_MMSS00,                         // MM:SS,00                     formatindex="44"
    NF_TIME_HH_MMSS00,                      // [HH]:MM:SS,00                formatindex="45"
    NF_TIME_END = NF_TIME_HH_MMSS00,

    NF_DATETIME_START,
    NF_DATETIME_SYSTEM_SHORT_HHMM = NF_DATETIME_START,  // 08.10.97 01:23
    NF_DATETIME_SYS_DDMMYYYY_HHMMSS,        // 08.10.1997 01:23:45          THE edit format, formatindex="47"
    NF_DATETIME_END = NF_DATETIME_SYS_DDMMYYYY_HHMMSS,

    NF_BOOLEAN,                             // BOOLEAN
    NF_TEXT,                                // @

    NF_INDEX_TABLE_LOCALE_DATA_DEFAULTS,    // == 50, old number of predefined entries, i18npool locale data additions start after this

    // From here on are values of new predefined and built-in formats that are
    // not in the original NumberFormatIndex.idl

    // XXX Values appended here must also get a corresponding entry in
    // svl/source/numbers/zforlist.cxx indexTable[] in the same order.

    // XXX The dialog's number format shell assumes start/end spans
    // (NF_..._START and NF_..._END above) to fill its categories with builtin
    // formats, make new formats known to svx/source/items/numfmtsh.cxx
    // SvxNumberFormatShell::FillEListWithStd_Impl(), otherwise they will not
    // be listed at all. Yes that is ugly.
    // DATETIME formats need to be added to
    // SvxNumberFormatShell::FillEListWithDateTime_Impl().

    // New predefined format added to i18npool locale data.
    NF_DATETIME_SYS_DDMMYYYY_HHMM = NF_INDEX_TABLE_LOCALE_DATA_DEFAULTS,  // 08.10.1997 01:23  formatindex="50"

    // No i18npool defined locale data between here and NF_INDEX_TABLE_ENTRIES.
    NF_INDEX_TABLE_RESERVED_START,

    NF_FRACTION_3D = NF_INDEX_TABLE_RESERVED_START,  // # ???/???
    NF_FRACTION_2,                          // # ?/2
    NF_FRACTION_4,                          // # ?/4
    NF_FRACTION_8,                          // # ?/8
    NF_FRACTION_16,                         // # ??/16
    NF_FRACTION_10,                         // # ??/10
    NF_FRACTION_100,                        // # ??/100

    NF_DATETIME_ISO_YYYYMMDD_HHMMSS,        // 1997-10-08 01:23:45          ISO (with blank instead of T)
    NF_DATETIME_ISO_YYYYMMDD_HHMMSS000,     // 1997-10-08 01:23:45.678      not quite ISO with locale's separator
    NF_DATETIME_ISO_YYYYMMDDTHHMMSS,        // 1997-10-08T01:23:45          ISO
    NF_DATETIME_ISO_YYYYMMDDTHHMMSS000,     // 1997-10-08T01:23:45,678      ISO with milliseconds and ',' or '.'

    // XXX When adding values here, follow the comment above about
    // svx/source/items/numfmtsh.cxx

    NF_INDEX_TABLE_ENTRIES                  // == 62, reserved to not be used in i18npool locale data.

    // XXX Adding values above may increment the reserved area that can't be
    // used by i18npool's locale data FormatCode definitions, see the
    // description at i18npool/source/localedata/data/locale.dtd for ELEMENT
    // FormatCode what the current convention's value is. In that case, the
    // used formatIndex values in i18npool/source/localedata/data/*.xml will
    // have to be adjusted.
    // Overlapping the area will bail out with a check in
    // SvNumberFormatter::ImpInsertFormat() in debug builds.
};


// #45717# IsNumberFormat( "98-10-24", 30, x ), YMD Format set with DMY
// International settings doesn't recognize the string as a date.
/** enum values for <method>SvNumberFormatter::SetEvalDateFormat</method>

    <p>How <method>ImpSvNumberInputScan::GetDateRef</method> shall take the
     DateFormat order (YMD,DMY,MDY) into account, if called from IsNumberFormat
    with a date format to match against.
 */
enum class NfEvalDateFormat
{
    /** DateFormat only from International, default. */
    International,

    /** DateFormat only from date format passed to function (if any).
        If no date format is passed then the DateFormat is taken from International. */
    Format,

    /** First try the DateFormat from International. If it doesn't match a
         valid date try the DateFormat from the date format passed. */
    InternationalThenFormat,

    /** First try the DateFormat from the date format passed. If it doesn't
        match a valid date try the DateFormat from International. */
    FormatThenInternational
};


/// This table is std::map because it needs to preserve insertion order,
/// because the formats are roughly ordered from most to least common, and some
/// parts of the UI want to show them in that order.
typedef std::map<sal_uInt32, SvNumberformat*> SvNumberFormatTable;
typedef std::unordered_map<sal_uInt16, sal_uInt32> SvNumberFormatterIndexTable;
typedef std::unordered_map< sal_uInt32, sal_uInt32> SvNumberFormatterMergeMap;


/** Language/country dependent currency entries
 */
class UNLESS_MERGELIBS(SVL_DLLPUBLIC) NfCurrencyEntry final
{
    OUString        aSymbol;            /// currency symbol
    OUString        aBankSymbol;        /// currency abbreviation
    LanguageType    eLanguage;          /// language/country value
    sal_uInt16      nPositiveFormat;    /// position of symbol
    sal_uInt16      nNegativeFormat;    /// position of symbol and type and position of negative sign
    sal_uInt16      nDigits;            /// count of decimal digits
    sal_Unicode     cZeroChar;          /// which character is used for zeros as last decimal digits

private:

                        // nDecimalFormat := 0, 1, 2
                        // #,##0 or #,##0.00 or #,##0.-- is returned
    SVL_DLLPRIVATE OUString Impl_BuildFormatStringNumChars( const LocaleDataWrapper&, sal_uInt16 nDecimalFormat) const;

public:

    NfCurrencyEntry( const LocaleDataWrapper& rLocaleData,
                     LanguageType eLang );
    NfCurrencyEntry( const css::i18n::Currency & rCurr,
                     const LocaleDataWrapper& rLocaleData,
                     LanguageType eLang );

                        /// Symbols and language identical
    bool                operator==( const NfCurrencyEntry& r ) const;

    const OUString&     GetSymbol() const           { return aSymbol; }
    const OUString&     GetBankSymbol() const       { return aBankSymbol; }
    LanguageType        GetLanguage() const         { return eLanguage; }
    sal_uInt16          GetPositiveFormat() const   { return nPositiveFormat; }
    sal_uInt16          GetNegativeFormat() const   { return nNegativeFormat; }
    sal_uInt16          GetDigits() const           { return nDigits; }

                        /** Only to resolve system locale for currency list. */
    void                SetLanguage( LanguageType nLang ) { eLanguage = nLang; }

                        /** [$DM-407] (bBank==false) or [$DEM] (bBank==true)
                            is returned. If bBank==false and
                            bWithoutExtension==true only [$DM] */
    OUString            BuildSymbolString(bool bBank, bool bWithoutExtension = false) const;

                        /** #,##0.00 [$DM-407] is returned, separators
                              from rLoc,    incl. minus sign but without [RED] */
    OUString            BuildPositiveFormatString(bool bBank, const LocaleDataWrapper&,
                                                  sal_uInt16 nDecimalFormat = 1) const;
    OUString            BuildNegativeFormatString(bool bBank, const LocaleDataWrapper&,
                                                  sal_uInt16 nDecimalFormat = 1) const;

                        /** [$DM-407] (or [$DEM] if bBank==true)
                            is appended/prepended to rStr, incl. minus sign */
    void                CompletePositiveFormatString(OUStringBuffer& rStr, bool bBank,
                                                     sal_uInt16 nPosiFormat) const;
    void                CompleteNegativeFormatString(OUStringBuffer& rStr, bool bBank,
                                                     sal_uInt16 nNegaFormat) const;

                        /// rSymStr is appended/prepended to rStr, incl. minus sign
    static  void        CompletePositiveFormatString(OUStringBuffer& rStr,
                                                     std::u16string_view rSymStr, sal_uInt16 nPosiFormat);
    static  void        CompleteNegativeFormatString(OUStringBuffer& rStr,
                                                     std::u16string_view rSymStr, sal_uInt16 nNegaFormat);

                        /** Representation of a currency (symbol position and
                             negative sign) in other locales */
    static  sal_uInt16  GetEffectivePositiveFormat( sal_uInt16 nIntlFormat,
                                                    sal_uInt16 nCurrFormat, bool bBank );
    static  sal_uInt16  GetEffectiveNegativeFormat( sal_uInt16 nIntlFormat,
                                                    sal_uInt16 nCurrFormat, bool bBank );

    /// General Unicode Euro symbol
    static sal_Unicode   GetEuroSymbol() { return u'\x20AC'; }
};

typedef std::vector< OUString > NfWSStringsDtor;

/** Input options to be used with IsNumberFormat() */
enum class SvNumInputOptions : sal_uInt16
{
    NONE = 0,
    LAX_TIME = 1    ///< allow input of minutes or seconds >59
};
namespace o3tl {
    template<> struct typed_flags<SvNumInputOptions> : is_typed_flags<SvNumInputOptions, 0x0001> {};
}


#endif // INCLUDED_SVL_ZFORLIST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
