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

#include <functional>
#include <map>

class Color;
class ImpSvNumberformatScan;
class ImpSvNumberInputScan;
class SvNumberFormatterRegistry_Impl;
class SvNumberFormatter;
class NfCurrencyTable;

class SVL_DLLPUBLIC SvNFLanguageData
{
public:
    SvNFLanguageData(const css::uno::Reference<css::uno::XComponentContext>& rxContext,
                     LanguageType eLang, const SvNumberFormatter& rColorCallback);
    SvNFLanguageData(const SvNFLanguageData& rOther);
    ~SvNFLanguageData();

    const css::uno::Reference<css::uno::XComponentContext>& GetComponentContext() const
    {
        return xContext;
    }

    // return the corresponding LocaleData wrapper
    const LocaleDataWrapper* GetLocaleData() const;

    // return the corresponding CharacterClassification wrapper
    const CharClass* GetCharClass() const;

    // return the corresponding Calendar wrapper
    CalendarWrapper* GetCalendar() const;

    // return corresponding Transliteration wrapper
    const ::utl::TransliterationWrapper* GetTransliteration() const;

    //! The following method is not to be used from outside but must be
    //! public for the InputScanner.
    // return the current FormatScanner
    const ImpSvNumberformatScan* GetFormatScanner() const;

    // return current (!) Locale
    const LanguageTag& GetLanguageTag() const;

    /// Get compatibility ("automatic" old style) currency from I18N locale data
    void GetCompatibilityCurrency(OUString& rSymbol, OUString& rAbbrev) const;

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

    /// Return the decimal separator matching the given locale / LanguageType.
    OUString GetLangDecimalSep(LanguageType nLang) const;

    /// Change language/country, also input and format scanner
    void ChangeIntl(LanguageType eLnge);

    /** Set evaluation type and order of input date strings
        @see NfEvalDateFormat
     */
    void SetEvalDateFormat(NfEvalDateFormat eEDF) { eEvalDateFormat = eEDF; }
    NfEvalDateFormat GetEvalDateFormat() const { return eEvalDateFormat; }

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

    /** The language with which the formatter was initialized (system setting),
        NOT the current language after a ChangeIntl() */
    LanguageType GetIniLanguage() const { return IniLnge; }

    LanguageType ImpResolveLanguage(LanguageType eLnge) const;

    /// Return the reference date
    const Date& GetNullDate() const;

    sal_uInt16 GetCurrencyFormatStrings(NfWSStringsDtor&, const NfCurrencyEntry&, bool bBank) const;

    void ChangeStandardPrec(short nPrec);

    sal_uInt16 ExpandTwoDigitYear(sal_uInt16 nYear) const;

private:
    friend class SvNFEngine;
    friend class SvNFFormatData;
    friend class SvNumberFormatter;

    css::uno::Reference<css::uno::XComponentContext> xContext;

    const LanguageType IniLnge; // Initial language/country setting
    LanguageType ActLnge; // Current language/country setting
    LanguageTag aLanguageTag;
    OnDemandCharClass xCharClass; // CharacterClassification
    OnDemandLocaleDataWrapper xLocaleData; // LocaleData switched between SYSTEM, ENGLISH and other
    OnDemandTransliterationWrapper xTransliteration; // Transliteration loaded on demand
    OnDemandCalendarWrapper xCalendar; // Calendar loaded on demand
    std::unique_ptr<ImpSvNumberInputScan> pStringScanner; // Input string scanner
    std::unique_ptr<ImpSvNumberformatScan> pFormatScanner; // Format code string scanner

    // cached locale data items needed almost every time
    OUString aDecimalSep;
    OUString aDecimalSepAlt;
    OUString aThousandSep;
    OUString aDateSep;

    NfEvalDateFormat eEvalDateFormat; // DateFormat evaluation
};

class SVL_DLLPUBLIC SvNFFormatData
{
private:
    typedef std::map<sal_uInt32, std::unique_ptr<SvNumberformat>> FormatEntryMap;
    FormatEntryMap aFTable; // Table of format keys to format entries
    typedef std::map<sal_uInt32, sal_uInt32> DefaultFormatKeysMap;
    DefaultFormatKeysMap aDefaultFormatKeys; // Table of default standard to format keys
    sal_uInt32 MaxCLOffset; // Max language/country offset used
    sal_uInt32 nDefaultSystemCurrencyFormat; // NewCurrency matching SYSTEM locale
    bool bNoZero; // Zero value suppression

public:
    SvNFFormatData();
    ~SvNFFormatData();

public:
    const SvNumberformat* GetFormatEntry(sal_uInt32 nKey) const;

    SvNumFormatType GetType(sal_uInt32 nFIndex) const;

    /// Whether format index nFIndex is of type text or not
    bool IsTextFormat(sal_uInt32 nFIndex) const;

    OUString GetCalcCellReturn(sal_uInt32 nFormat) const;

    /** Whether nFormat is of type css::util::NumberFormat::CURRENCY and the format code
        contains a new SYMBOLTYPE_CURRENCY and if so which one [$xxx-nnn].
        If ppEntry is not NULL and exactly one entry is found, a [$xxx-nnn] is
        returned, even if the format code only contains [$xxx] !
     */
    bool GetNewCurrencySymbolString(sal_uInt32 nFormat, OUString& rSymbol,
                                    const NfCurrencyEntry** ppEntry, bool* pBank = nullptr) const;

private:
    SvNFFormatData(const SvNFFormatData&) = delete;
    SvNFFormatData& operator=(const SvNFFormatData&) = delete;

    friend class SvNFEngine;
    friend class SvNumberFormatter;

    SVL_DLLPRIVATE sal_uInt32 GetStandardFormat(SvNFLanguageData& rCurrentLanguage,
                                                const NativeNumberWrapper* pNatNum,
                                                SvNumFormatType eType, LanguageType eLnge);

    // Obtain the format entry for a given key index.
    SVL_DLLPRIVATE SvNumberformat* GetFormatEntry(sal_uInt32 nKey);

    SVL_DLLPRIVATE SvNumberformat* GetEntry(sal_uInt32 nKey) const;

    /// Return whether zero suppression is switched on
    SVL_DLLPRIVATE bool GetNoZero() const { return bNoZero; }
    SVL_DLLPRIVATE void SetNoZero(bool bNZ) { bNoZero = bNZ; }

    SVL_DLLPRIVATE sal_uInt32 ImpIsEntry(std::u16string_view rString, sal_uInt32 nCLOffset,
                                         LanguageType eLnge) const;

    // Return CLOffset or (MaxCLOffset + SV_COUNTRY_LANGUAGE_OFFSET) if new language/country
    SVL_DLLPRIVATE sal_uInt32 ImpGetCLOffset(LanguageType eLnge) const;

    // Generate builtin formats provided by i18n behind CLOffset,
    // if bNoAdditionalFormats==false also generate additional i18n formats.
    SVL_DLLPRIVATE void ImpGenerateFormats(SvNFLanguageData& rCurrentLanguage,
                                           const NativeNumberWrapper* pNatNum, sal_uInt32 CLOffset,
                                           bool bNoAdditionalFormats);

    // Create builtin formats for language/country if necessary, return CLOffset
    SVL_DLLPRIVATE sal_uInt32 ImpGenerateCL(SvNFLanguageData& rCurrentLanguage,
                                            const NativeNumberWrapper* pNatNum, LanguageType eLnge);

    // Generate additional formats provided by i18n
    SVL_DLLPRIVATE void ImpGenerateAdditionalFormats(
        SvNFLanguageData& rCurrentLanguage, const NativeNumberWrapper* pNatNum, sal_uInt32 CLOffset,
        css::uno::Reference<css::i18n::XNumberFormatCode> const& rNumberFormatCode,
        bool bAfterChangingSystemCL);

    // called by SvNumberFormatterRegistry_Impl::Notify if the default system currency changes
    SVL_DLLPRIVATE void ResetDefaultSystemCurrency();

    SVL_DLLPRIVATE bool PutEntry(SvNFLanguageData& rCurrentLanguage,
                                 const NativeNumberWrapper* pNatNum, OUString& rString,
                                 sal_Int32& nCheckPos, SvNumFormatType& nType, sal_uInt32& nKey,
                                 LanguageType eLnge = LANGUAGE_DONTKNOW,
                                 bool bReplaceBooleanEquivalent = true);

    SVL_DLLPRIVATE SvNumberformat*
    ImpInsertFormat(SvNFLanguageData& rCurrentLanguage, const NativeNumberWrapper* pNatNum,
                    const css::i18n::NumberFormatCode& rCode, sal_uInt32 nPos,
                    bool bAfterChangingSystemCL = false, sal_Int16 nOrgIndex = 0);

    SVL_DLLPRIVATE sal_uInt32 ImpGetStandardFormat(SvNFLanguageData& rCurrentLanguage,
                                                   const NativeNumberWrapper* pNatNum,
                                                   SvNumFormatType eType, sal_uInt32 CLOffset,
                                                   LanguageType eLnge);

    // Return the format index of the currency format of the current locale.
    // Format is created if not already present.
    SVL_DLLPRIVATE sal_uInt32 ImpGetDefaultCurrencyFormat(SvNFLanguageData& rCurrentLanguage,
                                                          const NativeNumberWrapper* pNatNum,
                                                          sal_uInt32 CLOffset, LanguageType eLnge);

    // Return the format index of the currency format of the system locale.
    // Format is created if not already present.
    SVL_DLLPRIVATE sal_uInt32 ImpGetDefaultSystemCurrencyFormat(SvNFLanguageData& rCurrentLanguage,
                                                                const NativeNumberWrapper* pNatNum);

    SVL_DLLPRIVATE std::pair<sal_uInt32, bool>
    ImpGetDefaultFormat(SvNumFormatType nType, sal_uInt32 nSearch, sal_uInt32 CLOffset) const;

    SVL_DLLPRIVATE static sal_Int32
    ImpGetFormatCodeIndex(const SvNFLanguageData& rCurrentLanguage,
                          css::uno::Sequence<css::i18n::NumberFormatCode>& rSeq,
                          const NfIndexTableOffset nTabOff);

    SVL_DLLPRIVATE static void ImpAdjustFormatCodeDefault(const SvNFLanguageData& rCurrentLanguage,
                                                          css::i18n::NumberFormatCode* pFormatArr,
                                                          sal_Int32 nCnt);
};

class SVL_DLLPUBLIC SvNFEngine
{
public:
    typedef std::function<sal_uInt32(SvNFLanguageData& rCurrentLanguage,
                                     const NativeNumberWrapper* pNatNum, LanguageType eLnge)>
        GetCLOffset;
    typedef std::function<void(sal_uInt32 nSearch, sal_uInt32 nFormat)> CacheFormat;

    typedef std::function<sal_uInt32(SvNFLanguageData& rCurrentLanguage,
                                     const NativeNumberWrapper* pNatNum, sal_uInt32 CLOffset,
                                     LanguageType eLnge)>
        GetDefaultCurrency;

    struct Accessor
    {
        GetCLOffset mGetCLOffset;
        CacheFormat mCacheFormat;
        GetDefaultCurrency mGetDefaultCurrency;
    };

    static Accessor GetRWPolicy(SvNFFormatData& rFormatData);
    static Accessor GetROPolicy(const SvNFFormatData& rFormatData);

    static void ChangeIntl(SvNFLanguageData& rCurrentLanguage, LanguageType eLnge);
    static void ChangeNullDate(SvNFLanguageData& rCurrentLanguage, sal_uInt16 nDay,
                               sal_uInt16 nMonth, sal_Int16 nYear);

    static sal_uInt32 GetFormatIndex(SvNFLanguageData& rCurrentLanguage, const Accessor& rFuncs,
                                     const NativeNumberWrapper* pNatNum, NfIndexTableOffset nTabOff,
                                     LanguageType eLnge);

    static sal_uInt32 GetFormatForLanguageIfBuiltIn(SvNFLanguageData& rCurrentLanguage,
                                                    const NativeNumberWrapper* pNatNum,
                                                    const Accessor& rFunc, sal_uInt32 nFormat,
                                                    LanguageType eLnge);

    static bool IsNumberFormat(SvNFLanguageData& rCurrentLanguage,
                               const SvNFFormatData& rFormatData,
                               const NativeNumberWrapper* pNatNum, const Accessor& rFunc,
                               const OUString& sString, sal_uInt32& F_Index, double& fOutNumber,
                               SvNumInputOptions eInputOptions = SvNumInputOptions::NONE);

    static sal_uInt32 GetStandardFormat(SvNFLanguageData& rCurrentLanguage,
                                        const SvNFFormatData& rFormatData,
                                        const NativeNumberWrapper* pNatNum, const Accessor& rFunc,
                                        SvNumFormatType eType, LanguageType eLnge);

    static sal_uInt32 GetStandardFormat(SvNFLanguageData& rCurrentLanguage,
                                        const SvNFFormatData& rFormatData,
                                        const NativeNumberWrapper* pNatNum, const Accessor& rFuncs,
                                        sal_uInt32 nFIndex, SvNumFormatType eType,
                                        LanguageType eLnge);

    static sal_uInt32 GetStandardFormat(SvNFLanguageData& rCurrentLanguage,
                                        const SvNFFormatData& rFormatData,
                                        const NativeNumberWrapper* pNatNum, const Accessor& rFuncs,
                                        double fNumber, sal_uInt32 nFIndex, SvNumFormatType eType,
                                        LanguageType eLnge);

    static sal_uInt32 GetStandardIndex(SvNFLanguageData& rCurrentLanguage,
                                       const SvNFFormatData& rFormatData,
                                       const NativeNumberWrapper* pNatNum, const Accessor& rFuncs,
                                       LanguageType eLnge = LANGUAGE_DONTKNOW);

    static sal_uInt32 GetTimeFormat(SvNFLanguageData& rCurrentLanguage,
                                    const SvNFFormatData& rFormatData,
                                    const NativeNumberWrapper* pNatNum, const Accessor& rFuncs,
                                    double fNumber, LanguageType eLnge, bool bForceDuration);

    /// Return the decimal separator matching the locale of the given format
    static OUString GetFormatDecimalSep(SvNFLanguageData& rCurrentLanguage,
                                        const SvNFFormatData& rFormatData, sal_uInt32 nFormat);

    /// Count of decimals
    static sal_uInt16 GetFormatPrecision(SvNFLanguageData& rCurrentLanguage,
                                         const SvNFFormatData& rFormatData, sal_uInt32 nFormat);

    static void GetInputLineString(SvNFLanguageData& rCurrentLanguage,
                                   const SvNFFormatData& rFormatData,
                                   const NativeNumberWrapper* pNatNum, const Accessor& rFuncs,
                                   const double& fOutNumber, sal_uInt32 nFIndex,
                                   OUString& sOutString, bool bFiltering, bool bForceSystemLocale);

    static sal_uInt32 GetEditFormat(SvNFLanguageData& rCurrentLanguage,
                                    const SvNFFormatData& rFormatData,
                                    const NativeNumberWrapper* pNatNum, const Accessor& rFuncs,
                                    double fNumber, sal_uInt32 nFIndex, SvNumFormatType eType,
                                    const SvNumberformat* pFormat, LanguageType eForLocale);

    static void GetOutputString(SvNFLanguageData& rCurrentLanguage,
                                const SvNFFormatData& rFormatData,
                                const NativeNumberWrapper* pNatNum, const Accessor& rFuncs,
                                const double& fOutNumber, sal_uInt32 nFIndex, OUString& sOutString,
                                const Color** ppColor, bool bUseStarFormat);

    static void GetOutputString(SvNFLanguageData& rCurrentLanguage,
                                const SvNFFormatData& rFormatData, const OUString& sString,
                                sal_uInt32 nFIndex, OUString& sOutString, const Color** ppColor,
                                bool bUseStarFormat);

    static bool GetPreviewString(SvNFLanguageData& rCurrentLanguage,
                                 const SvNFFormatData& rFormatData,
                                 const NativeNumberWrapper* pNatNum, const Accessor& rFuncs,
                                 const OUString& sFormatString, double fPreviewNumber,
                                 OUString& sOutString, const Color** ppColor, LanguageType eLnge,
                                 bool bUseStarFormat);

    static bool GetPreviewString(SvNFLanguageData& rCurrentLanguage,
                                 const SvNFFormatData& rFormatData,
                                 const NativeNumberWrapper* pNatNum, const Accessor& rFuncs,
                                 const OUString& sFormatString, const OUString& sPreviewString,
                                 OUString& sOutString, const Color** ppColor,
                                 LanguageType eLnge = LANGUAGE_DONTKNOW);

    static bool GetPreviewStringGuess(SvNFLanguageData& rCurrentLanguage,
                                      const SvNFFormatData& rFormatData,
                                      const NativeNumberWrapper* pNatNum, const Accessor& rFuncs,
                                      const OUString& sFormatString, double fPreviewNumber,
                                      OUString& sOutString, const Color** ppColor,
                                      LanguageType eLnge);

    static OUString GenerateFormat(SvNFLanguageData& rCurrentLanguage,
                                   const SvNFFormatData& rFormatData,
                                   const NativeNumberWrapper* pNatNum, const Accessor& rFuncs,
                                   sal_uInt32 nIndex, LanguageType eLnge, bool bThousand,
                                   bool IsRed, sal_uInt16 nPrecision, sal_uInt16 nLeadingZeros);

private:
    static sal_uInt32 ImpGetDefaultFormat(const SvNFFormatData& rFormatData,
                                          const SvNFEngine::CacheFormat& rFunc,
                                          SvNumFormatType nType, sal_uInt32 CLOffset);
    static sal_uInt32
    ImpGetStandardFormat(SvNFLanguageData& rCurrentLanguage, const SvNFFormatData& rFormatData,
                         const NativeNumberWrapper* pNatNum, const SvNFEngine::Accessor& rFuncs,
                         SvNumFormatType eType, sal_uInt32 CLOffset, LanguageType eLnge);

    static sal_uInt32 DefaultCurrencyRW(SvNFFormatData& rFormatData,
                                        SvNFLanguageData& rCurrentLanguage,
                                        const NativeNumberWrapper* pNatNum, sal_uInt32 CLOffset,
                                        LanguageType eLnge);
    static sal_uInt32 DefaultCurrencyRO(const SvNFFormatData& rFormatData, SvNFLanguageData&,
                                        const NativeNumberWrapper*, sal_uInt32 CLOffset,
                                        LanguageType eLnge);

    static sal_uInt32 GetCLOffsetRW(SvNFFormatData& rFormatData, SvNFLanguageData& rCurrentLanguage,
                                    const NativeNumberWrapper* pNatNum, LanguageType eLnge);
    static sal_uInt32 GetCLOffsetRO(const SvNFFormatData& rFormatData, SvNFLanguageData&,
                                    const NativeNumberWrapper*, LanguageType eLnge);

    static void CacheFormatRW(SvNFFormatData& rFormatData, sal_uInt32 nSearch, sal_uInt32 nFormat);
    static void CacheFormatRO(const SvNFFormatData& rFormatData, sal_uInt32 nSearch,
                              sal_uInt32 nFormat);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
