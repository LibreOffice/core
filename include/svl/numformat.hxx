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

#include <svl/svldllapi.h>
#include <com/sun/star/i18n/XNumberFormatCode.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <i18nlangtag/lang.h>
#include <tools/link.hxx>
#include <svl/nfkeytab.hxx>
#include <svl/ondemand.hxx>
#include <svl/zforlist.hxx>
#include <unotools/charclass.hxx>

#include <map>

class Color;
class ImpSvNumberformatScan;
class ImpSvNumberInputScan;
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

    /// ctor with service manager and language/country enum
    SvNumberFormatter(const css::uno::Reference<css::uno::XComponentContext>& rxContext,
                      LanguageType eLang);

    ~SvNumberFormatter();

    /// Set CallBack to ColorTable
    void SetColorLink(const Link<sal_uInt16, Color*>& rColorTableCallBack);
    /// Do the CallBack to ColorTable
    Color* GetUserDefColor(sal_uInt16 nIndex) const;

    /// Change language/country, also input and format scanner
    void ChangeIntl(LanguageType eLnge);
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
    SvNumberFormatTable& GetEntryTable(SvNumFormatType eType, sal_uInt32& FIndex,
                                       LanguageType eLnge);

    /** Get table of formats of a specific type of a language/country.
        FIndex returns the default format of that type.
        If the language/country was never touched before new entries are generated */
    SvNumberFormatTable& ChangeCL(SvNumFormatType eType, sal_uInt32& FIndex, LanguageType eLnge);

    /** Get table of formats of the same type as FIndex; eType and rLnge are
        set accordingly. An unknown format is set to Standard/General */
    SvNumberFormatTable& GetFirstEntryTable(SvNumFormatType& eType, sal_uInt32& FIndex,
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
    bool PutEntry(OUString& rString, sal_Int32& nCheckPos, SvNumFormatType& nType, sal_uInt32& nKey,
                  LanguageType eLnge = LANGUAGE_DONTKNOW, bool bReplaceBooleanEquivalent = true);

    /** Same as <method>PutEntry</method> but the format code string is
         considered to be of language/country eLnge and is converted to
        language/country eNewLnge */
    bool PutandConvertEntry(OUString& rString, sal_Int32& nCheckPos, SvNumFormatType& nType,
                            sal_uInt32& nKey, LanguageType eLnge, LanguageType eNewLnge,
                            bool bConvertDateOrder, bool bReplaceBooleanEquivalent = true);

    /** Same as <method>PutandConvertEntry</method> but the format code string
         is considered to be of the System language/country eLnge and is
        converted to another System language/country eNewLnge. In this case
         the automatic currency is converted too. */
    bool PutandConvertEntrySystem(OUString& rString, sal_Int32& nCheckPos, SvNumFormatType& nType,
                                  sal_uInt32& nKey, LanguageType eLnge, LanguageType eNewLnge);

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
    sal_uInt32 GetIndexPuttingAndConverting(OUString& rString, LanguageType eLnge,
                                            LanguageType eSysLnge, SvNumFormatType& rType,
                                            bool& rNewInserted, sal_Int32& rCheckPos);

    /** Create a format code string using format nIndex as a template and
        applying other settings (passed from the dialog) */
    OUString GenerateFormat(sal_uInt32 nIndex, LanguageType eLnge = LANGUAGE_DONTKNOW,
                            bool bThousand = false, bool IsRed = false, sal_uInt16 nPrecision = 0,
                            sal_uInt16 nLeadingCnt = 1);

    /** Analyze an input string
        @return
            <TRUE/> if input is a number or is matching a format F_Index
                F_Index is set to a matching format if number, the value is
                returned in fOutNumber
            <FALSE/> if input is not a number
     */
    bool IsNumberFormat(const OUString& sString, sal_uInt32& F_Index, double& fOutNumber,
                        SvNumInputOptions eInputOptions = SvNumInputOptions::NONE);

    /// Format a number according to a format index, return string and color
    void GetOutputString(const double& fOutNumber, sal_uInt32 nFIndex, OUString& sOutString,
                         const Color** ppColor, bool bUseStarFormat = false);

    /** Format a string according to a format index, return string and color.
        Formats only if the format code is of type text or the 4th subcode
        of a format code is specified, otherwise sOutString will be == "" */
    void GetOutputString(const OUString& sString, sal_uInt32 nFIndex, OUString& sOutString,
                         const Color** ppColor, bool bUseStarFormat = false);

    /** Format a number according to the standard default format matching
        the given format index. rOutString will be the real cell string (e.g.
        a number rounded by the cell format, which rounded value is used
        in the filtering condition now), instead of the EditFormat string
        (e.g a not rounded value, which is visible during editing).*/
    void GetInputLineString(const double& fOutNumber, sal_uInt32 nFIndex, OUString& rOutString,
                            bool bFiltering = false, bool bForceSystemLocale = false);

    /** Format a number according to a format code string to be scanned.
        @return
            <FALSE/> if format code contains an error
            <TRUE/> else, in which case the string and color are returned.
     */
    bool GetPreviewString(const OUString& sFormatString, double fPreviewNumber,
                          OUString& sOutString, const Color** ppColor, LanguageType eLnge,
                          bool bUseStarFormat = false);

    /** Same as <method>GetPreviewString</method> but the format code string
        may be either language/country eLnge or en_US english US */
    bool GetPreviewStringGuess(const OUString& sFormatString, double fPreviewNumber,
                               OUString& sOutString, const Color** ppColor,
                               LanguageType eLnge = LANGUAGE_DONTKNOW);

    /** Format a string according to a format code string to be scanned.
        @return
            <FALSE/> if format code contains an error
            <TRUE/> else, in which case the string and color are returned.
     */
    bool GetPreviewString(const OUString& sFormatString, const OUString& sPreviewString,
                          OUString& sOutString, const Color** ppColor,
                          LanguageType eLnge = LANGUAGE_DONTKNOW);

    /** Test whether the format code string is already present in container
        @return
            NUMBERFORMAT_ENTRY_NOT_FOUND if not found, else the format index.
     */
    sal_uInt32 TestNewString(const OUString& sFormatString, LanguageType eLnge = LANGUAGE_DONTKNOW);

    /// Whether format index nFIndex is of type text or not
    bool IsTextFormat(sal_uInt32 nFIndex) const;

    /// Whether format index nFIndex has NatNum12 modifier
    bool IsNatNum12(sal_uInt32 nFIndex) const;

    /// Get additional info of a format index, e.g. for dialog box
    void GetFormatSpecialInfo(sal_uInt32 nFormat, bool& bThousand, bool& IsRed,
                              sal_uInt16& nPrecision, sal_uInt16& nLeadingCnt);

    /// Count of decimals
    sal_uInt16 GetFormatPrecision(sal_uInt32 nFormat) const;

    /// Count of integer digits
    sal_uInt16 GetFormatIntegerDigits(sal_uInt32 nFormat) const;

    /** Get additional info of a format code string, e.g. for dialog box.
        Uses a temporary parse, if possible use only if format code is not
        present in container yet, otherwise ineffective.
        @return
            0 if format code string parsed without errors, otherwise error
            position (like nCheckPos on <method>PutEntry</method>)
     */
    sal_uInt32 GetFormatSpecialInfo(const OUString&, bool& bThousand, bool& IsRed,
                                    sal_uInt16& nPrecision, sal_uInt16& nLeadingCnt,
                                    LanguageType eLnge = LANGUAGE_DONTKNOW);

    /// Get return string for Calc CELL() function, "G", "D1", ...
    OUString GetCalcCellReturn(sal_uInt32 nFormat) const;

    bool IsUserDefined(sal_uInt32 F_Index) const;

    /// Check if format code string may be deleted by user
    bool IsUserDefined(std::u16string_view sStr, LanguageType eLnge = LANGUAGE_DONTKNOW);

    /** Return the format index of the format code string for language/country,
        or NUMBERFORMAT_ENTRY_NOT_FOUND */
    sal_uInt32 GetEntryKey(std::u16string_view sStr, LanguageType eLnge = LANGUAGE_DONTKNOW);

    /// Return the format for a format index
    const SvNumberformat* GetEntry(sal_uInt32 nKey) const;

    /// Obtain substituted GetFormatEntry(), i.e. system formats.
    const SvNumberformat* GetSubstitutedEntry(sal_uInt32 nKey, sal_uInt32& o_rNewKey) const;

    /// Return the format index of the standard default number format for language/country
    sal_uInt32 GetStandardIndex(LanguageType eLnge = LANGUAGE_DONTKNOW);

    /// Return the format index of the default format of a type for language/country
    sal_uInt32 GetStandardFormat(SvNumFormatType eType, LanguageType eLnge = LANGUAGE_DONTKNOW);

    /** Return the format index of the default format of a type for language/country.
        Maybe not the default format but a special builtin format, e.g. for
        NF_TIME_HH_MMSS00, if that format is passed in nFIndex. */
    sal_uInt32 GetStandardFormat(sal_uInt32 nFIndex, SvNumFormatType eType, LanguageType eLnge);

    /** Return the format index of the default format of a type for language/country.
        Maybe not the default format but a special builtin format, e.g. for
        NF_TIME_HH_MMSS00, or NF_TIME_HH_MMSS if fNumber >= 1.0  */
    sal_uInt32 GetStandardFormat(double fNumber, sal_uInt32 nFIndex, SvNumFormatType eType,
                                 LanguageType eLnge);

    /** Return a time format that best matches fNumber. */
    sal_uInt32 GetTimeFormat(double fNumber, LanguageType eLnge, bool bForceDuration);

    /** Return a format and type that best matches the value of fNumber if
        fNumber is assumed to be a date, time or datetime value, but unknown
        which. Originally introduced for Chart databrowser editor, probably
        should not be used otherwise. */
    sal_uInt32 GuessDateTimeFormat(SvNumFormatType& rType, double fNumber, LanguageType eLnge);

    /** Return the corresponding edit format of a format.

        nFIndex, eType and pFormat (if not nullptr) are assumed to match each
        other / be of one format. The locale to use is obtained from pFormat,
        if nullptr then LANGUAGE_SYSTEM is used. This can be overridden by
        specifying eForLocale other than LANGUAGE_DONTKNOW.
     */
    sal_uInt32 GetEditFormat(double fNumber, sal_uInt32 nFIndex, SvNumFormatType eType,
                             SvNumberformat const* pFormat,
                             LanguageType eForLocale = LANGUAGE_DONTKNOW);

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
    sal_uInt32 GetMergeFormatIndex(sal_uInt32 nOldFmt) const;

    /** Convert the ugly old tools' Table type bloated with new'ed sal_uInt32
        entries merge table to ::std::map with old index key and new index key.
        @ATTENTION! Also clears the old table using ClearMergeTable() */
    SvNumberFormatterMergeMap ConvertMergeTableToMap();

    /** Return the format index of a builtin format for a specific language/country.
        If nFormat is not a builtin format nFormat is returned. */
    sal_uInt32 GetFormatForLanguageIfBuiltIn(sal_uInt32 nFormat,
                                             LanguageType eLnge = LANGUAGE_DONTKNOW);

    /** Return the format index for a builtin format of a specific language
        @see NfIndexTableOffset
     */
    sal_uInt32 GetFormatIndex(NfIndexTableOffset, LanguageType eLnge = LANGUAGE_DONTKNOW);

    /** Return enum index of a format index of a builtin format,
        NF_INDEX_TABLE_ENTRIES if it's not a builtin format.
        @see NfIndexTableOffset
     */
    static NfIndexTableOffset GetIndexTableOffset(sal_uInt32 nFormat);

    /** Set evaluation type and order of input date strings
        @see NfEvalDateFormat
     */
    void SetEvalDateFormat(NfEvalDateFormat eEDF);
    NfEvalDateFormat GetEvalDateFormat() const;

    /** Set TwoDigitYearStart, how the input string scanner handles a two digit year.
        Default from VCL: 1930, 30-99 19xx, 00-29 20xx

        <p> Historically (prior to src513e) it was a two digit number determining
        until which number the string scanner recognizes a year to be 20xx,
        default <= 29 is used by SFX/OfaMiscCfg.
        The name Year2000 is kept although the actual functionality is now a
        TwoDigitYearStart which might be in any century.
     */
    void SetYear2000(sal_uInt16 nVal);
    sal_uInt16 GetYear2000() const;
    static sal_uInt16 GetYear2000Default();

    sal_uInt16 ExpandTwoDigitYear(sal_uInt16 nYear) const;
    static sal_uInt16 ExpandTwoDigitYear(sal_uInt16 nYear, sal_uInt16 nTwoDigitYearStart);

    /// Return the decimal separator matching the locale of the given format
    OUString GetFormatDecimalSep(sal_uInt32 nFormat) const;

    /// Return the decimal separator matching the given locale / LanguageType.
    OUString GetLangDecimalSep(LanguageType nLang) const;

    static void resetTheCurrencyTable();

    /// Return a NfCurrencyTable with pointers to <type>NfCurrencyEntry</type> entries
    static const NfCurrencyTable& GetTheCurrencyTable();

    /** Searches, according to the default locale currency, an entry of the
        CurrencyTable which is <bold>not</bold> the first (LANGUAGE_SYSTEM) entry.
        @return
            <NULL/> if not found
            else pointer to NfCurrencyEntry
     */
    static const NfCurrencyEntry* MatchSystemCurrency();

    /** Return a NfCurrencyEntry matching a language/country.
        If language/country is LANGUAGE_SYSTEM a <method>MatchSystemCurrency</method>
        call is tried to get an entry. If that fails or the corresponding
        language/country is not present the entry for LANGUAGE_SYSTEM is returned.
     */
    static const NfCurrencyEntry& GetCurrencyEntry(LanguageType);

    /** Return a NfCurrencyEntry pointer matching a language/country
        and currency abbreviation (AKA banking symbol).
        This method is meant for the configuration of the default currency.
        @return
            <NULL/> if not found
            else pointer to NfCurrencyEntry
     */
    static const NfCurrencyEntry* GetCurrencyEntry(std::u16string_view rAbbrev, LanguageType eLang);

    /** Return a NfCurrencyEntry pointer matching the symbol
        combination of a LegacyOnly currency. Note that this means only that
        the currency matching both symbols was once used in the Office, but is
        not offered in dialogs anymore. It doesn't even mean that the currency
        symbol combination is valid, since the reason for removing it may have
        been just that. #i61657#
        @return
            A matching entry, or else <NULL/>.
     */
    static const NfCurrencyEntry* GetLegacyOnlyCurrencyEntry(std::u16string_view rSymbol,
                                                             std::u16string_view rAbbrev);

    /** Set the default system currency. The combination of abbreviation and
        language must match an existent element of theCurrencyTable. If not,
        the SYSTEM (current locale) entry becomes the default.
        This method is meant for the configuration of the default currency.
     */
    static void SetDefaultSystemCurrency(std::u16string_view rAbbrev, LanguageType eLang);

    /** Get all standard formats for a specific currency, formats are
        appended to the NfWSStringsDtor list.
        @param bBank
            <TRUE/>: generate only format strings with currency abbreviation
            <FALSE/>: mixed format strings
        @return
            position of default format
     */
    sal_uInt16 GetCurrencyFormatStrings(NfWSStringsDtor&, const NfCurrencyEntry&, bool bBank) const;

    /** Whether nFormat is of type css::util::NumberFormat::CURRENCY and the format code
        contains a new SYMBOLTYPE_CURRENCY and if so which one [$xxx-nnn].
        If ppEntry is not NULL and exactly one entry is found, a [$xxx-nnn] is
        returned, even if the format code only contains [$xxx] !
     */
    bool GetNewCurrencySymbolString(sal_uInt32 nFormat, OUString& rSymbol,
                                    const NfCurrencyEntry** ppEntry, bool* pBank = nullptr) const;

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
    static const NfCurrencyEntry* GetCurrencyEntry(bool& bFoundBank, std::u16string_view rSymbol,
                                                   std::u16string_view rExtension,
                                                   LanguageType eFormatLanguage,
                                                   bool bOnlyStringLanguage = false);

    /// Get compatibility ("automatic" old style) currency from I18N locale data
    void GetCompatibilityCurrency(OUString& rSymbol, OUString& rAbbrev) const;

    /// Fill rList with the language/country codes that have been allocated
    void GetUsedLanguages(std::vector<LanguageType>& rList);

    /// Fill a NfKeywordIndex table with keywords of a language/country
    void FillKeywordTable(NfKeywordTable& rKeywords, LanguageType eLang);

    /** Fill a NfKeywordIndex table with keywords usable in Excel export with
        GetFormatStringForExcel() or SvNumberformat::GetMappedFormatstring() */
    void FillKeywordTableForExcel(NfKeywordTable& rKeywords);

    /** Return a format code string suitable for Excel export.

        @param  rTempFormatter
                SvNumberFormatter to use if a non-en-US format code needs to be
                converted and put, should not be the same formatter to not
                pollute the entries of this one here.
     */
    OUString GetFormatStringForExcel(sal_uInt32 nKey, const NfKeywordTable& rKeywords,
                                     SvNumberFormatter& rTempFormatter) const;

    /** Return a keyword for a language/country and NfKeywordIndex
        for XML import, to generate number format strings. */
    OUString GetKeyword(LanguageType eLnge, sal_uInt16 nIndex);

    /** Return the GENERAL keyword in proper case ("General") for a
        language/country, used in XML import */
    OUString GetStandardName(LanguageType eLnge);

    /** Check if a specific locale has supported locale data. */
    static bool IsLocaleInstalled(LanguageType eLang);

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
    const NfKeywordTable& GetKeywords(sal_uInt32 nKey);

    /** Access for unit tests. */
    const NfKeywordTable& GetEnglishKeywords() const;

    /** Access for unit tests. */
    const std::vector<Color>& GetStandardColors() const;

    /** Access for unit tests. */
    size_t GetMaxDefaultColors() const;

    struct InputScannerPrivateAccess
    {
        friend class ImpSvNumberInputScan;

    private:
        InputScannerPrivateAccess() {}
    };
    /** Access for input scanner to temporarily (!) switch locales. */
    OnDemandLocaleDataWrapper& GetOnDemandLocaleDataWrapper(const InputScannerPrivateAccess&)
    {
        return xLocaleData;
    }

private:
    mutable ::osl::Mutex m_aMutex;
    css::uno::Reference<css::uno::XComponentContext> m_xContext;
    const LanguageType IniLnge; // Initialized setting language/country
    LanguageType ActLnge; // Current setting language/country
    LanguageTag maLanguageTag;
    std::map<sal_uInt32, std::unique_ptr<SvNumberformat>>
        aFTable; // Table of format keys to format entries
    typedef std::map<sal_uInt32, sal_uInt32> DefaultFormatKeysMap;
    DefaultFormatKeysMap aDefaultFormatKeys; // Table of default standard to format keys
    std::unique_ptr<SvNumberFormatTable> pFormatTable; // For the UI dialog
    std::unique_ptr<SvNumberFormatterIndexTable>
        pMergeTable; // List of indices for merging two formatters
    OnDemandCharClass xCharClass; // CharacterClassification
    OnDemandLocaleDataWrapper xLocaleData; // LocaleData switched between SYSTEM, ENGLISH and other
    OnDemandTransliterationWrapper xTransliteration; // Transliteration loaded on demand
    OnDemandCalendarWrapper xCalendar; // Calendar loaded on demand
    OnDemandNativeNumberWrapper xNatNum; // Native number service loaded on demand
    std::unique_ptr<ImpSvNumberInputScan> pStringScanner; // Input string scanner
    std::unique_ptr<ImpSvNumberformatScan> pFormatScanner; // Format code string scanner
    Link<sal_uInt16, Color*> aColorLink; // User defined color table CallBack
    sal_uInt32 MaxCLOffset; // Max language/country offset used
    sal_uInt32 nDefaultSystemCurrencyFormat; // NewCurrency matching SYSTEM locale
    NfEvalDateFormat eEvalDateFormat; // DateFormat evaluation
    bool bNoZero; // Zero value suppression

    // cached locale data items needed almost any time
    OUString aDecimalSep;
    OUString aDecimalSepAlt;
    OUString aThousandSep;
    OUString aDateSep;

    SVL_DLLPRIVATE static volatile bool bCurrencyTableInitialized;
    SVL_DLLPRIVATE static sal_uInt16 nSystemCurrencyPosition;
    SVL_DLLPRIVATE static SvNumberFormatterRegistry_Impl* pFormatterRegistry;

    // get the registry, create one if none exists
    SVL_DLLPRIVATE static SvNumberFormatterRegistry_Impl& GetFormatterRegistry();

    // Generate builtin formats provided by i18n behind CLOffset,
    // if bNoAdditionalFormats==false also generate additional i18n formats.
    SVL_DLLPRIVATE void ImpGenerateFormats(sal_uInt32 CLOffset, bool bNoAdditionalFormats);

    // Generate additional formats provided by i18n
    SVL_DLLPRIVATE void ImpGenerateAdditionalFormats(
        sal_uInt32 CLOffset,
        css::uno::Reference<css::i18n::XNumberFormatCode> const& rNumberFormatCode,
        bool bAfterChangingSystemCL);

    SVL_DLLPRIVATE SvNumberformat* ImpInsertFormat(const css::i18n::NumberFormatCode& rCode,
                                                   sal_uInt32 nPos,
                                                   bool bAfterChangingSystemCL = false,
                                                   sal_Int16 nOrgIndex = 0);

    // Return CLOffset or (MaxCLOffset + SV_COUNTRY_LANGUAGE_OFFSET) if new language/country
    SVL_DLLPRIVATE sal_uInt32 ImpGetCLOffset(LanguageType eLnge) const;

    // Test whether format code already exists, then return index key,
    // otherwise NUMBERFORMAT_ENTRY_NOT_FOUND
    SVL_DLLPRIVATE sal_uInt32 ImpIsEntry(std::u16string_view rString, sal_uInt32 CLOffset,
                                         LanguageType eLnge) const;

    // Create builtin formats for language/country if necessary, return CLOffset
    SVL_DLLPRIVATE sal_uInt32 ImpGenerateCL(LanguageType eLnge);

    // Create theCurrencyTable with all <type>NfCurrencyEntry</type>
    SVL_DLLPRIVATE static void ImpInitCurrencyTable();

    // Return the format index of the currency format of the system locale.
    // Format is created if not already present.
    SVL_DLLPRIVATE sal_uInt32 ImpGetDefaultSystemCurrencyFormat();

    // Return the format index of the currency format of the current locale.
    // Format is created if not already present.
    SVL_DLLPRIVATE sal_uInt32 ImpGetDefaultCurrencyFormat();

    // Return the default format for a given type and current locale.
    // May ONLY be called from within GetStandardFormat().
    SVL_DLLPRIVATE sal_uInt32 ImpGetDefaultFormat(SvNumFormatType nType);

    // Return the index in a sequence of format codes matching an enum of
    // NfIndexTableOffset. If not found 0 is returned. If the sequence doesn't
    // contain any format code elements a default element is created and inserted.
    SVL_DLLPRIVATE sal_Int32 ImpGetFormatCodeIndex(
        css::uno::Sequence<css::i18n::NumberFormatCode>& rSeq, const NfIndexTableOffset nTabOff);

    // Adjust a sequence of format codes to contain only one (THE) default
    // instead of multiple defaults for short/medium/long types.
    // If there is no medium but a short and a long default the long is taken.
    // Non-PRODUCT version may check locale data for matching defaults in one
    // FormatElement group.
    SVL_DLLPRIVATE void ImpAdjustFormatCodeDefault(css::i18n::NumberFormatCode* pFormatArr,
                                                   sal_Int32 nCount);

    // Obtain the format entry for a given key index.
    SVL_DLLPRIVATE SvNumberformat* GetFormatEntry(sal_uInt32 nKey);
    SVL_DLLPRIVATE const SvNumberformat* GetFormatEntry(sal_uInt32 nKey) const;

    // used as a loop body inside of GetNewCurrencySymbolString() and GetCurrencyEntry()
    static bool ImpLookupCurrencyEntryLoopBody(const NfCurrencyEntry*& pFoundEntry,
                                               bool& bFoundBank, const NfCurrencyEntry* pData,
                                               sal_uInt16 nPos, std::u16string_view rSymbol);

    // link to be set at <method>SvtSysLocaleOptions::SetCurrencyChangeLink()</method>
    DECL_DLLPRIVATE_STATIC_LINK(SvNumberFormatter, CurrencyChangeLink, LinkParamNone*, void);

    // return position of a special character
    sal_Int32 ImpPosToken(const OUStringBuffer& sFormat, sal_Unicode token,
                          sal_Int32 nStartPos = 0) const;

    // Substitute a format during GetFormatEntry(), i.e. system formats.
    SvNumberformat* ImpSubstituteEntry(SvNumberformat* pFormat, sal_uInt32* o_pRealKey = nullptr);

    // Whether nFIndex is a special builtin format
    SVL_DLLPRIVATE bool ImpIsSpecialStandardFormat(sal_uInt32 nFIndex, LanguageType eLnge);

    // called by SvNumberFormatterRegistry_Impl::Notify if the default system currency changes
    SVL_DLLPRIVATE void ResetDefaultSystemCurrency();

    // own mutex, may also be used by internal class SvNumberFormatterRegistry_Impl
    static ::osl::Mutex& GetGlobalMutex();
    ::osl::Mutex& GetInstanceMutex() const { return m_aMutex; }

public:
    // Called by SvNumberFormatterRegistry_Impl::Notify if the system locale's
    // date acceptance patterns change.
    void InvalidateDateAcceptancePatterns();

    // Replace the SYSTEM language/country format codes. Called upon change of
    // the user configurable locale.
    // Old compatibility codes are replaced, user defined are converted, and
    // new format codes are appended.
    void ReplaceSystemCL(LanguageType eOldLanguage);

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
    bool IsDecimalSep(std::u16string_view rStr) const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
