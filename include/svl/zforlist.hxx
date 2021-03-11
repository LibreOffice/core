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
#include <tools/link.hxx>
#include <svl/ondemand.hxx>
#include <svl/nfkeytab.hxx>

#include <map>
#include <memory>
#include <set>

namespace com::sun::star::i18n { class XNumberFormatCode; }
namespace com::sun::star::i18n { struct Currency; }
namespace com::sun::star::i18n { struct NumberFormatCode; }

class Date;
class Color;
class CharClass;
class CalendarWrapper;

class ImpSvNumberformatScan;
class ImpSvNumberInputScan;
class SvNumberformat;
namespace com::sun::star::uno { class XComponentContext; }

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
    NF_DATETIME_ISO_YYYYMMDDTHHMMSS,        // 1997-10-08T01:23:45          ISO

    // XXX When adding values here, follow the comment above about
    // svx/source/items/numfmtsh.cxx

    NF_INDEX_TABLE_ENTRIES                  // == 60, reserved to not be used in i18npool locale data.

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
enum NfEvalDateFormat
{
    /** DateFormat only from International, default. */
    NF_EVALDATEFORMAT_INTL,

    /** DateFormat only from date format passed to function (if any).
        If no date format is passed then the DateFormat is taken from International. */
    NF_EVALDATEFORMAT_FORMAT,

    /** First try the DateFormat from International. If it doesn't match a
         valid date try the DateFormat from the date format passed. */
    NF_EVALDATEFORMAT_INTL_FORMAT,

    /** First try the DateFormat from the date format passed. If it doesn't
        match a valid date try the DateFormat from International. */
    NF_EVALDATEFORMAT_FORMAT_INTL
};


typedef std::map<sal_uInt32, SvNumberformat*> SvNumberFormatTable;
typedef std::map<sal_uInt16, sal_uInt32> SvNumberFormatterIndexTable;

typedef ::std::map< sal_uInt32, sal_uInt32> SvNumberFormatterMergeMap;

typedef ::std::set< LanguageType > NfInstalledLocales;


/** Language/country dependent currency entries
 */
class UNLESS_MERGELIBS(SVL_DLLPUBLIC) NfCurrencyEntry
{
    OUString        aSymbol;            /// currency symbol
    OUString        aBankSymbol;        /// currency abbreviation
    LanguageType    eLanguage;          /// language/country value
    sal_uInt16      nPositiveFormat;    /// position of symbol
    sal_uInt16      nNegativeFormat;    /// position of symbol and type and position of negative sign
    sal_uInt16      nDigits;            /// count of decimal digits
    sal_Unicode     cZeroChar;          /// which character is used for zeros as last decimal digits

                        NfCurrencyEntry( const NfCurrencyEntry& ) = delete;
    NfCurrencyEntry&    operator=( const NfCurrencyEntry& ) = delete;

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
                             negative sign) in other language settings */
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

class SvNumberFormatterRegistry_Impl;
class NfCurrencyTable;

class SVL_DLLPUBLIC SvNumberFormatter
{
    friend class SvNumberFormatterRegistry_Impl;
public:
    /**
     * We can't technically have an "infinite" value, so we use an arbitrary
     * upper precision threshold to represent the "unlimited" precision.
     */
    static const sal_uInt16 UNLIMITED_PRECISION;

    /**
     * Precision suitable for numbers displayed in input bar, for instance
     * Calc's formula input bar.
     */
    static const sal_uInt16 INPUTSTRING_PRECISION;

    /// Preferred ctor with service manager and language/country enum
    SvNumberFormatter(
        const css::uno::Reference< css::uno::XComponentContext >& rxContext,
        LanguageType eLang
        );

    ~SvNumberFormatter();

    /// Set CallBack to ColorTable
    void SetColorLink( const Link<sal_uInt16,Color*>& rColorTableCallBack );
    /// Do the CallBack to ColorTable
    Color* GetUserDefColor(sal_uInt16 nIndex);

    /// Change language/country, also input and format scanner
    void ChangeIntl( LanguageType eLnge );
    /// Change the reference null date
    void ChangeNullDate(sal_uInt16 nDay, sal_uInt16 nMonth, sal_Int16 nYear);
    /// Change standard precision
    void ChangeStandardPrec(short nPrec);
    /// Set zero value suppression
    void SetNoZero(bool bNZ);

    /** The language with which the formatter was initialized (system setting),
        NOT the current language after a ChangeIntl() */
    LanguageType GetLanguage() const;

    // Determine whether two format types are input compatible or not
    static bool IsCompatible(SvNumFormatType eOldType, SvNumFormatType eNewType);

    /** Get table of formats of a specific type of a locale. A format FIndex is
        tested whether it has the type and locale requested, if it doesn't
        match FIndex returns the default format for the type/locale. If no
        specific format is to be selected FIndex may be initialized to 0. */
    SvNumberFormatTable& GetEntryTable(SvNumFormatType eType,
                                       sal_uInt32& FIndex,
                                       LanguageType eLnge);

    /** Get table of formats of a specific type of a language/country.
        FIndex returns the default format of that type.
        If the language/country was never touched before new entries are generated */
    SvNumberFormatTable& ChangeCL(SvNumFormatType eType,
                                  sal_uInt32& FIndex,
                                  LanguageType eLnge);

    /** Get table of formats of the same type as FIndex; eType and rLnge are
        set accordingly. An unknown format is set to Standard/General */
    SvNumberFormatTable& GetFirstEntryTable(SvNumFormatType& eType,
                                            sal_uInt32& FIndex,
                                            LanguageType& rLnge);

    /// Delete an entry including the format it is referring to
    void DeleteEntry(sal_uInt32 nKey);

    /** Create new entry of a format code string for language/country.
        @return
            <TRUE/> if string new and ok and inserted.
             <FALSE/> if string already exists or an unresolvable parse error
             occurred, in which case nCheckPos is the error position within rString.
            If the error occurs at position 0 or rString is empty nCheckPos
            will be 1, so an error in the string is always indicated by
             nCheckPos not being zero.
            The content of the rString variable can be changed and corrected
             by the method.
            nType contains the type of the format.
            nKey contains the index key of the format.
     */
    bool PutEntry( OUString& rString, sal_Int32& nCheckPos, SvNumFormatType& nType, sal_uInt32& nKey,
                   LanguageType eLnge = LANGUAGE_DONTKNOW );

    /** Same as <method>PutEntry</method> but the format code string is
         considered to be of language/country eLnge and is converted to
        language/country eNewLnge */
    bool PutandConvertEntry( OUString& rString, sal_Int32& nCheckPos,
                             SvNumFormatType& nType, sal_uInt32& nKey,
                             LanguageType eLnge, LanguageType eNewLnge,
                             bool bConvertDateOrder );

    /** Same as <method>PutandConvertEntry</method> but the format code string
         is considered to be of the System language/country eLnge and is
        converted to another System language/country eNewLnge. In this case
         the automatic currency is converted too. */
    bool PutandConvertEntrySystem( OUString& rString, sal_Int32& nCheckPos,
                                   SvNumFormatType& nType, sal_uInt32& nKey,
                                   LanguageType eLnge, LanguageType eNewLnge );

    /** Similar to <method>PutEntry</method> and
        <method>PutandConvertEntry</method> or
        <method>PutandConvertEntrySystem</method>, the format code string
        passed is considered to be of language/country eLnge. If
        eLnge==LANGUAGE_SYSTEM the format code has to match eSysLnge, and if
        eSysLnge is not the current application locale the format code is
        converted to the current locale. Additionally, if the format code
        represents an old "automatic" currency format, it is converted to the
        new default currency format of the eLnge locale. The rString format
        code passed as an argument may get adapted in case eLnge was used (or
        is LANGUAGE_SYSTEM and eSysLnge is identical); in case it wasn't the
        method works on a copy instead, otherwise the resulting string would
        not match eSysLnge anymore.

        <p> This method was introduced to handle the legacy currency formats of
        the "autotbl.fmt" file used by Calc and Writer and convert them to
        fixed currency codes of the actual currency. Note that in the case of
        legacy currency formats no special attribution is converted, only the
        default currency format of the locale is chosen, and that new fixed
        currency codes are of course not converted to other currencies. The
        method may also be used as a general method taking, converting and
        inserting almost arbitrary format codes. To insert or use, for example,
        the default currency format code matching the current locale, the
        method could be called with<br/>

        <code>
        GetIndexPuttingAndConverting( "0 $", LANGUAGE_SYSTEM, LANGUAGE_ENGLISH_US, ...);
        </code>

        @return
            The index key of the resulting number format. If the format code
            was empty, could not be converted or has errors, the eLnge locale's
            standard number format is chosen instead. The index key is
            guaranteed to represent some valid number format. If
            rNewInserted==false and rCheckPos>0 the format code has errors
            and/or could not be converted.
     */
    sal_uInt32 GetIndexPuttingAndConverting( OUString & rString, LanguageType eLnge,
                                             LanguageType eSysLnge, SvNumFormatType & rType,
                                             bool & rNewInserted, sal_Int32 & rCheckPos );

    /** Create a format code string using format nIndex as a template and
        applying other settings (passed from the dialog) */
    OUString GenerateFormat(sal_uInt32 nIndex,
                            LanguageType eLnge = LANGUAGE_DONTKNOW,
                            bool bThousand = false, bool IsRed = false,
                            sal_uInt16 nPrecision = 0, sal_uInt16 nLeadingCnt = 1);

    /** Analyze an input string
        @return
            <TRUE/> if input is a number or is matching a format F_Index
                F_Index is set to a matching format if number, the value is
                returned in fOutNumber
            <FALSE/> if input is not a number
     */
    bool IsNumberFormat( const OUString& sString, sal_uInt32& F_Index, double& fOutNumber,
                         SvNumInputOptions eInputOptions = SvNumInputOptions::NONE );

    /// Format a number according to a format index, return string and color
    void GetOutputString( const double& fOutNumber, sal_uInt32 nFIndex,
                          OUString& sOutString, const Color** ppColor, bool bUseStarFormat = false );

    /** Format a string according to a format index, return string and color.
        Formats only if the format code is of type text or the 4th subcode
        of a format code is specified, otherwise sOutString will be == "" */
    void GetOutputString( const OUString& sString, sal_uInt32 nFIndex,
                          OUString& sOutString, const Color** ppColor, bool bUseStarFormat = false );

    /** Format a number according to the standard default format matching
        the given format index. rOutString will be the real cell string (e.g.
        a number rounded by the cell format, which rounded value is used
        in the filtering condition now), instead of the EditFormat string
        (e.g a not rounded value, which is visible during editing).*/
    void GetInputLineString( const double& fOutNumber,
                             sal_uInt32 nFIndex, OUString& rOutString, bool bFiltering = false );

    /** Format a number according to a format code string to be scanned.
        @return
            <FALSE/> if format code contains an error
            <TRUE/> else, in which case the string and color are returned.
     */
    bool GetPreviewString(const OUString& sFormatString,
                          double fPreviewNumber,
                          OUString& sOutString,
                          const Color** ppColor,
                          LanguageType eLnge,
                          bool bUseStarFormat = false );

    /** Same as <method>GetPreviewString</method> but the format code string
        may be either language/country eLnge or en_US english US */
    bool GetPreviewStringGuess( const OUString& sFormatString, double fPreviewNumber,
                                OUString& sOutString, const Color** ppColor,
                                LanguageType eLnge = LANGUAGE_DONTKNOW );

    /** Format a string according to a format code string to be scanned.
        @return
            <FALSE/> if format code contains an error
            <TRUE/> else, in which case the string and color are returned.
     */
    bool GetPreviewString( const OUString& sFormatString, const OUString& sPreviewString,
                           OUString& sOutString, const Color** ppColor,
                           LanguageType eLnge = LANGUAGE_DONTKNOW );

    /** Test whether the format code string is already present in container
        @return
            NUMBERFORMAT_ENTRY_NOT_FOUND if not found, else the format index.
     */
    sal_uInt32 TestNewString( const OUString& sFormatString,
                              LanguageType eLnge = LANGUAGE_DONTKNOW );

    /// Whether format index nFIndex is of type text or not
    bool IsTextFormat(sal_uInt32 nFIndex) const;

    /// Get additional info of a format index, e.g. for dialog box
    void GetFormatSpecialInfo(sal_uInt32 nFormat, bool& bThousand, bool& IsRed,
                              sal_uInt16& nPrecision, sal_uInt16& nLeadingCnt);

    /// Count of decimals
    sal_uInt16 GetFormatPrecision( sal_uInt32 nFormat ) const;

    /// Count of integer digits
    sal_uInt16 GetFormatIntegerDigits( sal_uInt32 nFormat ) const;

    /** Get additional info of a format code string, e.g. for dialog box.
        Uses a temporary parse, if possible use only if format code is not
        present in container yet, otherwise ineffective.
        @return
            0 if format code string parsed without errors, otherwise error
            position (like nCheckPos on <method>PutEntry</method>)
     */
    sal_uInt32 GetFormatSpecialInfo( const OUString&, bool& bThousand, bool& IsRed,
                                     sal_uInt16& nPrecision, sal_uInt16& nLeadingCnt,
                                     LanguageType eLnge = LANGUAGE_DONTKNOW );

    /// Get return string for Calc CELL() function, "G", "D1", ...
    OUString GetCalcCellReturn( sal_uInt32 nFormat ) const;

    /// Check if format code string may be deleted by user
    bool IsUserDefined( std::u16string_view sStr, LanguageType eLnge = LANGUAGE_DONTKNOW );

    /** Return the format index of the format code string for language/country,
        or NUMBERFORMAT_ENTRY_NOT_FOUND */
    sal_uInt32 GetEntryKey( std::u16string_view sStr, LanguageType eLnge = LANGUAGE_DONTKNOW );

    /// Return the format for a format index
    const SvNumberformat* GetEntry( sal_uInt32 nKey ) const;

    /// Obtain substituted GetFormatEntry(), i.e. system formats.
    const SvNumberformat* GetSubstitutedEntry( sal_uInt32 nKey, sal_uInt32 & o_rNewKey ) const;

    /// Return the format index of the standard default number format for language/country
    sal_uInt32 GetStandardIndex(LanguageType eLnge = LANGUAGE_DONTKNOW);

    /// Return the format index of the default format of a type for language/country
    sal_uInt32 GetStandardFormat(SvNumFormatType eType, LanguageType eLnge = LANGUAGE_DONTKNOW);

    /** Return the format index of the default format of a type for language/country.
        Maybe not the default format but a special builtin format, e.g. for
        NF_TIME_HH_MMSS00, if that format is passed in nFIndex. */
    sal_uInt32 GetStandardFormat( sal_uInt32 nFIndex, SvNumFormatType eType, LanguageType eLnge );

    /** Return the format index of the default format of a type for language/country.
        Maybe not the default format but a special builtin format, e.g. for
        NF_TIME_HH_MMSS00, or NF_TIME_HH_MMSS if fNumber >= 1.0  */
    sal_uInt32 GetStandardFormat( double fNumber, sal_uInt32 nFIndex, SvNumFormatType eType,
                                  LanguageType eLnge );

    /// Whether nFIndex is a special builtin format
    bool IsSpecialStandardFormat( sal_uInt32 nFIndex, LanguageType eLnge );

    /** Return a time format that best matches fNumber. */
    sal_uInt32 GetTimeFormat( double fNumber, LanguageType eLnge, bool bForceDuration );

    /** Return a format and type that best matches the value of fNumber if
        fNumber is assumed to be a date, time or datetime value, but unknown
        which. Originally introduced for Chart databrowser editor, probably
        should not be used otherwise. */
    sal_uInt32 GuessDateTimeFormat( SvNumFormatType& rType, double fNumber, LanguageType eLnge );

    /** Return the corresponding edit format of a format. */
    sal_uInt32 GetEditFormat( double fNumber, sal_uInt32 nFIndex, SvNumFormatType eType,
                              LanguageType eLnge, SvNumberformat const * pFormat );

    /// Return the reference date
    const Date& GetNullDate() const;
    /// Return the standard decimal precision
    sal_uInt16 GetStandardPrec() const;
    /// Return whether zero suppression is switched on
    bool GetNoZero() const;
    /** Get the type of a format (or css::util::NumberFormat::UNDEFINED if no entry),
         but with css::util::NumberFormat::DEFINED masked out */
    SvNumFormatType GetType(sal_uInt32 nFIndex) const;

    /// As the name says
    void ClearMergeTable();
    /// Merge in all new entries from rNewTable and return a table of resulting new format indices
    SvNumberFormatterIndexTable* MergeFormatter(SvNumberFormatter& rNewTable);

    /// Whether a merge table is present or not
    bool HasMergeFormatTable() const;
    /// Return the new format index for an old format index, if a merge table exists
    sal_uInt32 GetMergeFormatIndex( sal_uInt32 nOldFmt ) const;

    /** Convert the ugly old tools' Table type bloated with new'ed sal_uInt32
        entries merge table to ::std::map with old index key and new index key.
        @ATTENTION! Also clears the old table using ClearMergeTable() */
    SvNumberFormatterMergeMap ConvertMergeTableToMap();

    /** Return the format index of a builtin format for a specific language/country.
        If nFormat is not a builtin format nFormat is returned. */
    sal_uInt32 GetFormatForLanguageIfBuiltIn( sal_uInt32 nFormat,
                                              LanguageType eLnge = LANGUAGE_DONTKNOW );

    /** Return the format index for a builtin format of a specific language
        @see NfIndexTableOffset
     */
    sal_uInt32 GetFormatIndex( NfIndexTableOffset, LanguageType eLnge = LANGUAGE_DONTKNOW );

    /** Return enum index of a format index of a builtin format,
        NF_INDEX_TABLE_ENTRIES if it's not a builtin format.
        @see NfIndexTableOffset
     */
    NfIndexTableOffset GetIndexTableOffset( sal_uInt32 nFormat ) const;

    /** Set evaluation type and order of input date strings
        @see NfEvalDateFormat
     */
    void SetEvalDateFormat( NfEvalDateFormat eEDF );
    NfEvalDateFormat GetEvalDateFormat() const;

    /** Set TwoDigitYearStart, how the input string scanner handles a two digit year.
        Default from VCL: 1930, 30-99 19xx, 00-29 20xx

        <p> Historically (prior to src513e) it was a two digit number determining
        until which number the string scanner recognizes a year to be 20xx,
        default <= 29 is used by SFX/OfaMiscCfg.
        The name Year2000 is kept although the actual functionality is now a
        TwoDigitYearStart which might be in any century.
     */
    void    SetYear2000( sal_uInt16 nVal );
    sal_uInt16  GetYear2000() const;
    static  sal_uInt16  GetYear2000Default();

    sal_uInt16  ExpandTwoDigitYear( sal_uInt16 nYear ) const;
    static sal_uInt16 ExpandTwoDigitYear( sal_uInt16 nYear, sal_uInt16 nTwoDigitYearStart );

    /// Return the decimal separator matching the locale of the given format
    OUString GetFormatDecimalSep( sal_uInt32 nFormat ) const;

    /// Return the decimal separator matching the given locale / LanguageType.
    OUString GetLangDecimalSep( LanguageType nLang ) const;

    static void resetTheCurrencyTable();

    /// Return a NfCurrencyTable with pointers to <type>NfCurrencyEntry</type> entries
    static const NfCurrencyTable& GetTheCurrencyTable();

    /** Searches, according to the default locale currency, an entry of the
        CurrencyTable which is <bold>not</bold> the first (LANGUAGE_SYSTEM) entry.
        @return
            <NULL/> if not found
            else pointer to NfCurrencyEntry
     */
    static const NfCurrencyEntry*  MatchSystemCurrency();

    /** Return a NfCurrencyEntry matching a language/country.
        If language/country is LANGUAGE_SYSTEM a <method>MatchSystemCurrency</method>
        call is tried to get an entry. If that fails or the corresponding
        language/country is not present the entry for LANGUAGE_SYSTEM is returned.
     */
    static const NfCurrencyEntry&  GetCurrencyEntry( LanguageType );

    /** Return a NfCurrencyEntry pointer matching a language/country
        and currency abbreviation (AKA banking symbol).
        This method is meant for the configuration of the default currency.
        @return
            <NULL/> if not found
            else pointer to NfCurrencyEntry
     */
    static const NfCurrencyEntry*  GetCurrencyEntry( std::u16string_view rAbbrev,
                                                     LanguageType eLang );

    /** Return a NfCurrencyEntry pointer matching the symbol
        combination of a LegacyOnly currency. Note that this means only that
        the currency matching both symbols was once used in the Office, but is
        not offered in dialogs anymore. It doesn't even mean that the currency
        symbol combination is valid, since the reason for removing it may have
        been just that. #i61657#
        @return
            A matching entry, or else <NULL/>.
     */
    static const NfCurrencyEntry* GetLegacyOnlyCurrencyEntry( std::u16string_view rSymbol, std::u16string_view rAbbrev );

    /** Set the default system currency. The combination of abbreviation and
        language must match an existent element of theCurrencyTable. If not,
        the SYSTEM (current locale) entry becomes the default.
        This method is meant for the configuration of the default currency.
     */
    static void SetDefaultSystemCurrency( std::u16string_view rAbbrev, LanguageType eLang );

    /** Get all standard formats for a specific currency, formats are
        appended to the NfWSStringsDtor list.
        @param bBank
            <TRUE/>: generate only format strings with currency abbreviation
            <FALSE/>: mixed format strings
        @return
            position of default format
     */
    sal_uInt16  GetCurrencyFormatStrings( NfWSStringsDtor&, const NfCurrencyEntry&,
                                          bool bBank ) const;

    /** Whether nFormat is of type css::util::NumberFormat::CURRENCY and the format code
        contains a new SYMBOLTYPE_CURRENCY and if so which one [$xxx-nnn].
        If ppEntry is not NULL and exactly one entry is found, a [$xxx-nnn] is
        returned, even if the format code only contains [$xxx] !
     */
    bool    GetNewCurrencySymbolString( sal_uInt32 nFormat, OUString& rSymbol,
                                        const NfCurrencyEntry** ppEntry,
                                        bool* pBank = nullptr ) const;

    /** Look up the corresponding NfCurrencyEntry matching
        rSymbol (may be CurrencySymbol or CurrencyAbbreviation) and possibly
        a rExtension (being yyy of [$xxx-yyy]) or a given language/country
        value. Tries to match a rSymbol with rExtension first, then with
         eFormatLanguage, then rSymbol only. This is because a currency entry
        might have been constructed using I18N locale data where a used locale
        of a currency format code must not necessarily match the locale of
        the locale data itself, e.g. [$HK$-40C] (being "zh_HK" locale) in
        zh_CN locale data. Here the rExtension would have the value 0x40c but
         eFormatLanguage of the number format would have the value of zh_CN
         locale, the value with which the corresponding CurrencyEntry is
        constructed.

        @param bFoundBank
            Only used for output.
             If the return value is not <NULL/> this value is set to <TRUE/> if
             the matching entry was found by comparing rSymbol against the
             CurrencyAbbreviation (AKA BankSymbol).
             If the return value is <NULL/> the value of bFoundBank is undefined.
        @param rSymbol
            Currency symbol, preferably obtained of a format by a call to
            <method>SvNumberformat::GetNewCurrencySymbol()</method>
        @param rExtension
            Currency extension, preferably obtained of a format by a call to
            <method>SvNumberformat::GetNewCurrencySymbol()</method>
        @param eFormatLanguage
            The language/country value of the format of which rSymbol and
            rExtension are obtained (<method>SvNumberformat::GetLanguage()</method>).
        @param bOnlyStringLanguage
            If <TRUE/> only entries with language/country of rExtension are
            checked, no match on eFormatLanguage. If rExtension is empty all
             entries are checked.
        @return
            The matching entry if unique (in which case bFoundBank is set),
             else <NULL/>.
     */
    static const NfCurrencyEntry* GetCurrencyEntry( bool & bFoundBank,
                                                    std::u16string_view rSymbol,
                                                    const OUString& rExtension,
                                                    LanguageType eFormatLanguage,
                                                    bool bOnlyStringLanguage = false );

    /// Get compatibility ("automatic" old style) currency from I18N locale data
    void GetCompatibilityCurrency( OUString& rSymbol, OUString& rAbbrev ) const;

    /// Fill rList with the language/country codes that have been allocated
    void    GetUsedLanguages( std::vector<LanguageType>& rList );

    /// Fill a NfKeywordIndex table with keywords of a language/country
    void    FillKeywordTable( NfKeywordTable& rKeywords, LanguageType eLang );

    /** Fill a NfKeywordIndex table with keywords usable in Excel export with
        GetFormatStringForExcel() or SvNumberformat::GetMappedFormatstring() */
    void    FillKeywordTableForExcel( NfKeywordTable& rKeywords );

    /** Return a format code string suitable for Excel export.

        @param  rTempFormatter
                SvNumberFormatter to use if a non-en-US format code needs to be
                converted and put, should not be the same formatter to not
                pollute the entries of this one here.
     */
    OUString GetFormatStringForExcel( sal_uInt32 nKey, const NfKeywordTable& rKeywords,
            SvNumberFormatter& rTempFormatter ) const;

    /** Return a keyword for a language/country and NfKeywordIndex
        for XML import, to generate number format strings. */
    OUString GetKeyword( LanguageType eLnge, sal_uInt16 nIndex );

    /** Return the GENERAL keyword in proper case ("General") for a
        language/country, used in XML import */
    OUString GetStandardName( LanguageType eLnge );

    /** Check if a specific locale has supported locale data. */
    static bool IsLocaleInstalled( LanguageType eLang );

    /** Obtain NfKeywordTable used with a format, possibly localized.

        XXX NOTE: the content (actual keywords) is only valid as long as the
        locale context of the associated ImpSvNumberformatScan instance does
        not change to a locale with different keywords, which may happen
        anytime with a call (implicit or explicit) to
        SvNumberFormatter::ChangeIntl(). If needed longer, copy-create another
        NfKeywordTable instance or copy individual elements.

        If the format specified with nKey does not exist, the content of the
        NfKeywordTable matches the locale with which the SvNumberFormatter
        instance was created and initialized.

        This function preliminary exists for unit tests and otherwise is
        pretty much useless.
     */
    const NfKeywordTable & GetKeywords( sal_uInt32 nKey );

    /** Access for unit tests. */
    const NfKeywordTable & GetEnglishKeywords() const;

    /** Access for unit tests. */
    const std::vector<Color> & GetStandardColors() const;

    /** Access for unit tests. */
    size_t GetMaxDefaultColors() const;

    struct InputScannerPrivateAccess { friend class ImpSvNumberInputScan; private: InputScannerPrivateAccess() {} };
    /** Access for input scanner to temporarily (!) switch locales. */
    OnDemandLocaleDataWrapper& GetOnDemandLocaleDataWrapper( const InputScannerPrivateAccess& ) { return xLocaleData; }

private:
    mutable ::osl::Mutex m_aMutex;
    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    LanguageTag maLanguageTag;
    std::map<sal_uInt32, std::unique_ptr<SvNumberformat>> aFTable;            // Table of format keys to format entries
    typedef std::map<sal_uInt32, sal_uInt32> DefaultFormatKeysMap;
    DefaultFormatKeysMap aDefaultFormatKeys; // Table of default standard to format keys
    std::unique_ptr<SvNumberFormatTable> pFormatTable;      // For the UI dialog
    std::unique_ptr<SvNumberFormatterIndexTable> pMergeTable; // List of indices for merging two formatters
    std::unique_ptr<CharClass> pCharClass;                  // CharacterClassification
    OnDemandLocaleDataWrapper xLocaleData;  // LocaleData switched between SYSTEM, ENGLISH and other
    OnDemandTransliterationWrapper xTransliteration;    // Transliteration loaded on demand
    OnDemandCalendarWrapper xCalendar;      // Calendar loaded on demand
    OnDemandNativeNumberWrapper xNatNum;    // Native number service loaded on demand
    std::unique_ptr<ImpSvNumberInputScan> pStringScanner;   // Input string scanner
    std::unique_ptr<ImpSvNumberformatScan> pFormatScanner;  // Format code string scanner
    Link<sal_uInt16,Color*> aColorLink;     // User defined color table CallBack
    sal_uInt32 MaxCLOffset;                     // Max language/country offset used
    sal_uInt32 nDefaultSystemCurrencyFormat;        // NewCurrency matching SYSTEM locale
    LanguageType IniLnge;                   // Initialized setting language/country
    LanguageType ActLnge;                   // Current setting language/country
    NfEvalDateFormat eEvalDateFormat;       // DateFormat evaluation
    bool bNoZero;                           // Zero value suppression

    // cached locale data items needed almost any time
    OUString aDecimalSep;
    OUString aDecimalSepAlt;
    OUString aThousandSep;
    OUString aDateSep;

    SVL_DLLPRIVATE static volatile bool         bCurrencyTableInitialized;
    SVL_DLLPRIVATE static sal_uInt16            nSystemCurrencyPosition;
    SVL_DLLPRIVATE static SvNumberFormatterRegistry_Impl* pFormatterRegistry;

    // get the registry, create one if none exists
    SVL_DLLPRIVATE static SvNumberFormatterRegistry_Impl& GetFormatterRegistry();

    // called by ctors
    SVL_DLLPRIVATE void ImpConstruct( LanguageType eLang );

    // Generate builtin formats provided by i18n behind CLOffset,
    // if bNoAdditionalFormats==false also generate additional i18n formats.
    SVL_DLLPRIVATE void ImpGenerateFormats( sal_uInt32 CLOffset, bool bNoAdditionalFormats );

    // Generate additional formats provided by i18n
    SVL_DLLPRIVATE void ImpGenerateAdditionalFormats( sal_uInt32 CLOffset,
                                                      css::uno::Reference< css::i18n::XNumberFormatCode > const & rNumberFormatCode,
                                                      bool bAfterChangingSystemCL );

    SVL_DLLPRIVATE SvNumberformat* ImpInsertFormat( const css::i18n::NumberFormatCode& rCode,
                                                    sal_uInt32 nPos,
                                                    bool bAfterChangingSystemCL = false,
                                                    sal_Int16 nOrgIndex = 0 );

    // Return CLOffset or (MaxCLOffset + SV_COUNTRY_LANGUAGE_OFFSET) if new language/country
    SVL_DLLPRIVATE sal_uInt32 ImpGetCLOffset(LanguageType eLnge) const;

    // Test whether format code already exists, then return index key,
    // otherwise NUMBERFORMAT_ENTRY_NOT_FOUND
    SVL_DLLPRIVATE sal_uInt32 ImpIsEntry( std::u16string_view rString,
                                          sal_uInt32 CLOffset,
                                          LanguageType eLnge );

    // Create builtin formats for language/country if necessary, return CLOffset
    SVL_DLLPRIVATE sal_uInt32 ImpGenerateCL( LanguageType eLnge );

    // Create theCurrencyTable with all <type>NfCurrencyEntry</type>
    SVL_DLLPRIVATE static void ImpInitCurrencyTable();

    // Return the format index of the currency format of the system locale.
    // Format is created if not already present.
    SVL_DLLPRIVATE sal_uInt32   ImpGetDefaultSystemCurrencyFormat();

    // Return the format index of the currency format of the current locale.
    // Format is created if not already present.
    SVL_DLLPRIVATE sal_uInt32   ImpGetDefaultCurrencyFormat();

    // Return the default format for a given type and current locale.
    // May ONLY be called from within GetStandardFormat().
    SVL_DLLPRIVATE sal_uInt32   ImpGetDefaultFormat( SvNumFormatType nType );

    // Return the index in a sequence of format codes matching an enum of
    // NfIndexTableOffset. If not found 0 is returned. If the sequence doesn't
    // contain any format code elements a default element is created and inserted.
    SVL_DLLPRIVATE sal_Int32 ImpGetFormatCodeIndex( css::uno::Sequence< css::i18n::NumberFormatCode >& rSeq,
                                                    const NfIndexTableOffset nTabOff );

    // Adjust a sequence of format codes to contain only one (THE) default
    // instead of multiple defaults for short/medium/long types.
    // If there is no medium but a short and a long default the long is taken.
    // Non-PRODUCT version may check locale data for matching defaults in one
    // FormatElement group.
    SVL_DLLPRIVATE void ImpAdjustFormatCodeDefault( css::i18n::NumberFormatCode * pFormatArr,
                                                         sal_Int32 nCount );

    // Obtain the format entry for a given key index.
    SVL_DLLPRIVATE       SvNumberformat* GetFormatEntry( sal_uInt32 nKey );
    SVL_DLLPRIVATE const SvNumberformat* GetFormatEntry( sal_uInt32 nKey ) const;

    // used as a loop body inside of GetNewCurrencySymbolString() and GetCurrencyEntry()
    static bool ImpLookupCurrencyEntryLoopBody(
        const NfCurrencyEntry*& pFoundEntry, bool& bFoundBank, const NfCurrencyEntry* pData,
        sal_uInt16 nPos, std::u16string_view rSymbol );

    // link to be set at <method>SvtSysLocaleOptions::SetCurrencyChangeLink()</method>
    DECL_DLLPRIVATE_STATIC_LINK( SvNumberFormatter, CurrencyChangeLink, LinkParamNone*, void );

    // return position of a special character
    sal_Int32 ImpPosToken ( const OUStringBuffer & sFormat, sal_Unicode token, sal_Int32 nStartPos = 0 ) const;

    // Substitute a format during GetFormatEntry(), i.e. system formats.
    SvNumberformat* ImpSubstituteEntry( SvNumberformat* pFormat, sal_uInt32 * o_pRealKey = nullptr );

    // own mutex, may also be used by internal class SvNumberFormatterRegistry_Impl
    static ::osl::Mutex& GetGlobalMutex();
    ::osl::Mutex& GetInstanceMutex() const { return m_aMutex; }

public:

    // called by SvNumberFormatterRegistry_Impl::Notify if the default system currency changes
    void ResetDefaultSystemCurrency();

    // Called by SvNumberFormatterRegistry_Impl::Notify if the system locale's
    // date acceptance patterns change.
    void InvalidateDateAcceptancePatterns();

    // Replace the SYSTEM language/country format codes. Called upon change of
    // the user configurable locale.
    // Old compatibility codes are replaced, user defined are converted, and
    // new format codes are appended.
    void ReplaceSystemCL( LanguageType eOldLanguage );

    const css::uno::Reference<css::uno::XComponentContext>& GetComponentContext() const;

    //! The following method is not to be used from outside but must be
    //! public for the InputScanner.
    // return the current FormatScanner
    const ImpSvNumberformatScan* GetFormatScanner() const;

    //! The following methods are not to be used from outside but must be
    //! public for the InputScanner and FormatScanner.

    // return current (!) Locale
    const LanguageTag& GetLanguageTag() const;

    // return corresponding Transliteration wrapper
    const ::utl::TransliterationWrapper* GetTransliteration() const;

    // return the corresponding CharacterClassification wrapper
    const CharClass* GetCharClass() const;

    // return the corresponding LocaleData wrapper
    const LocaleDataWrapper* GetLocaleData() const;

    // return the corresponding Calendar wrapper
    CalendarWrapper* GetCalendar() const;

    // return the corresponding NativeNumberSupplier wrapper
    const NativeNumberWrapper* GetNatNum() const;

    // cached locale data items

    // return the corresponding decimal separator
    const OUString& GetNumDecimalSep() const;

    // return the corresponding decimal separator alternative
    const OUString& GetNumDecimalSepAlt() const;

    // return the corresponding group (AKA thousand) separator
    const OUString& GetNumThousandSep() const;

    // return the corresponding date separator
    const OUString& GetDateSep() const;

    // checks for decimal separator and optional alternative
    bool IsDecimalSep( std::u16string_view rStr ) const;
};

#endif // INCLUDED_SVL_ZFORLIST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
