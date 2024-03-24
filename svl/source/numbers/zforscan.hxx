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
#ifndef INCLUDED_SVL_SOURCE_NUMBERS_ZFORSCAN_HXX
#define INCLUDED_SVL_SOURCE_NUMBERS_ZFORSCAN_HXX

#include <i18nlangtag/lang.h>
#include <rtl/ustring.hxx>
#include <svl/nfkeytab.hxx>
#include <svl/nfsymbol.hxx>
#include <svl/numformat.hxx>
#include <svl/zforlist.hxx>
#include <tools/color.hxx>
#include <tools/date.hxx>
#include <unotools/localedatawrapper.hxx>

class SvNFLanguageData;
struct ImpSvNumberformatInfo;


const size_t NF_MAX_DEFAULT_COLORS   = 10;

// Hack: nThousand==1000 => "Default" occurs in format string
const sal_uInt16 FLAG_STANDARD_IN_FORMAT = 1000;

class ImpSvNumberformatScan
{
public:

    /** Specify what keyword localization is allowed when scanning the format code. */
    enum class KeywordLocalization
    {
        LocaleLegacy,   ///< unfortunately localized in few locales, otherwise English
        EnglishOnly,    ///< only English, no localized keywords
        AllowEnglish    ///< allow English keywords as well as localized keywords
    };

    explicit ImpSvNumberformatScan(SvNFLanguageData& rCurrentLanguageData,
                                   const SvNumberFormatter& rColorCallback);
    ~ImpSvNumberformatScan();
    void ChangeIntl( KeywordLocalization eKeywordLocalization = KeywordLocalization::AllowEnglish ); // Replaces Keywords

    void ChangeNullDate(sal_uInt16 nDay, sal_uInt16 nMonth, sal_Int16 nYear); // Replaces reference date
    void ChangeStandardPrec(sal_uInt16 nPrec); // Replaces standard precision

    sal_Int32 ScanFormat( OUString& rString ); // Call scan analysis

    void CopyInfo(ImpSvNumberformatInfo* pInfo,
                     sal_uInt16 nCnt); // Copies the FormatInfo
    sal_uInt16 GetResultStringsCnt() const      { return nResultStringsCnt; }

    const CharClass& GetChrCls() const          { return *mrCurrentLanguageData.GetCharClass(); }
    const LocaleDataWrapper& GetLoc() const     { return *mrCurrentLanguageData.GetLocaleData(); }

    const NfKeywordTable & GetKeywords() const
        {
            if ( bKeywordsNeedInit )
            {
                InitKeywords();
            }
            return sKeyword;
        }

    static const NfKeywordTable & GetEnglishKeywords()
        {
            return sEnglishKeyword;
        }

    // Keywords used in output like true and false
    const OUString& GetSpecialKeyword( NfKeywordIndex eIdx ) const
        {
            if ( sKeyword[eIdx].isEmpty() )
            {
                InitSpecialKeyword( eIdx );
            }
            return sKeyword[eIdx];
        }
    const OUString& GetTrueString() const     { return GetSpecialKeyword( NF_KEY_TRUE ); }
    const OUString& GetFalseString() const    { return GetSpecialKeyword( NF_KEY_FALSE ); }
    const OUString& GetRedString() const      { return GetKeywords()[NF_KEY_RED]; }
    const OUString& GetBooleanString() const  { return GetKeywords()[NF_KEY_BOOLEAN]; }
    static const ::std::vector<Color> & GetStandardColors()
        {
            return StandardColor;
        }
    static size_t GetMaxDefaultColors()
        {
            return NF_MAX_DEFAULT_COLORS;
        }

    const Date& GetNullDate() const           { return maNullDate; }
    const OUString& GetStandardName() const
        {
            if ( bKeywordsNeedInit )
            {
                InitKeywords();
            }
            return sNameStandardFormat;
        }
    sal_uInt16 GetStandardPrec() const          { return nStandardPrec; }
    static const Color& GetRedColor()           { return StandardColor[4]; }
    const Color* GetColor(OUString& sStr) const; // Set main colors or defines colors

    // the compatibility currency symbol for old automatic currency formats
    const OUString& GetCurSymbol() const
        {
            if ( bCompatCurNeedInit )
            {
                InitCompatCur();
            }
            return sCurSymbol;
        }

    // the compatibility currency abbreviation for CCC format code
    const OUString& GetCurAbbrev() const
        {
            if ( bCompatCurNeedInit )
            {
                InitCompatCur();
            }
            return sCurAbbrev;
        }

    // the compatibility currency symbol upper case for old automatic currency formats
    const OUString& GetCurString() const
        {
            if ( bCompatCurNeedInit )
            {
                InitCompatCur();
            }
            return sCurString;
        }

    /// Replace Boolean equivalent format codes with proper Boolean format.
    bool ReplaceBooleanEquivalent( OUString& rString );

    void SetConvertMode(LanguageType eTmpLge, LanguageType eNewLge,
            bool bSystemToSystem, bool bConvertDateOrder)
    {
        bConvertMode = true;
        eNewLnge = eNewLge;
        eTmpLnge = eTmpLge;
        bConvertSystemToSystem = bSystemToSystem;
        mbConvertDateOrder = bConvertDateOrder;
    }
    // Only changes the bool variable, in order to temporarily pause the convert mode
    void SetConvertMode(bool bMode) { bConvertMode = bMode; }
    bool GetConvertMode() const     { return bConvertMode; }
    LanguageType GetNewLnge() const { return eNewLnge; } // Read access on ConvertMode and convert country/language
    LanguageType GetTmpLnge() const { return eTmpLnge; } // Read access on StartCountry/Language
    void SetNewLnge( LanguageType e ) { eNewLnge = e; }  // Set new convert country/language

                                                /// get Thai T speciality
    sal_uInt8 GetNatNumModifier() const      { return nNatNumModifier; }
                                                /// set Thai T speciality
    void SetNatNumModifier( sal_uInt8 n )    { nNatNumModifier = n; }

    SvNFLanguageData& GetCurrentLanguageData() { return mrCurrentLanguageData; } // Access to formatter (for zformat.cxx)

    /// Get type scanned (so far).
    SvNumFormatType GetScannedType() const { return eScannedType; }

    const SvNumberFormatter& getColorCallback() const { return mrColorCallback; }

    static constexpr OUString sErrStr = u"#FMT"_ustr; // String for error output

private: // Private section
    NfKeywordTable sKeyword;                    // Syntax keywords
    static const NfKeywordTable sEnglishKeyword; // English Syntax keywords
    static const ::std::vector<Color> StandardColor;  // Standard color array
    Date maNullDate;                            // 30Dec1899
    OUString sNameStandardFormat;               // "Standard"
    sal_uInt16 nStandardPrec;                   // Default Precision for Standardformat
    SvNFLanguageData& mrCurrentLanguageData;    // Reference to the Language Data
    const SvNumberFormatter& mrColorCallback;   // Reference to the Color Callback supplier
    css::uno::Reference< css::i18n::XNumberFormatCode > xNFC;

    OUString sStrArray[NF_MAX_FORMAT_SYMBOLS];  // Array of symbols
    short nTypeArray[NF_MAX_FORMAT_SYMBOLS];    // Array of infos
                                                // External Infos:
    sal_uInt16 nResultStringsCnt;               // Result symbol count
    SvNumFormatType eScannedType;               // Type according to scan
    bool bThousand;                             // With thousands marker
    sal_uInt16 nThousand;                       // Counts ... series
    sal_uInt16 nCntPre;                         // Counts digits of integral part
    sal_uInt16 nCntPost;                        // Counts digits of fractional part
    sal_uInt16 nCntExp;                         // Counts exponent digits AM/PM
                                                // Internal info:
    sal_uInt16 nStringsCnt;                     // Symbol count
    sal_uInt16 nExpPos;                         // Internal position of E
    sal_uInt16 nBlankPos;                       // Internal position of the Blank
    short nDecPos;                              // Internal position of the ,
    bool bExp;                                  // Set when reading E
    bool bFrac;                                 // Set when reading /
    bool bBlank;                                // Set when reading ' ' (Fraction)
    bool bDecSep;                               // Set on first ,
    mutable bool bKeywordsNeedInit;             // Locale dependent keywords need to be initialized
    mutable bool bCompatCurNeedInit;            // Locale dependent compatibility currency need to be initialized
    OUString sCurSymbol;                        // Currency symbol for compatibility format codes
    OUString sCurString;                        // Currency symbol in upper case
    OUString sCurAbbrev;                        // Currency abbreviation
    OUString sBooleanEquivalent1;               // "TRUE";"TRUE";"FALSE"
    OUString sBooleanEquivalent2;               // [>0]"TRUE";[<0]"TRUE";"FALSE"

    bool bConvertMode;                          // Set in the convert mode
    bool mbConvertDateOrder;                    // Set in the convert mode whether to convert date particles order

    LanguageType eNewLnge;                      // Language/country which the scanned string is converted to (for Excel filter)
    LanguageType eTmpLnge;                      // Language/country which the scanned string is converted from (for Excel filter)

    bool bConvertSystemToSystem;                // Whether the conversion is from one system locale to another system locale
                                                // (in this case the automatic currency symbol is converted too).

    sal_Int32 nCurrPos;                         // Position of currency symbol

    sal_uInt8 nNatNumModifier;                  // Thai T speciality

    KeywordLocalization meKeywordLocalization;  ///< which keywords localization to scan

    // Copy assignment is forbidden and not implemented.
    ImpSvNumberformatScan (const ImpSvNumberformatScan &) = delete;
    ImpSvNumberformatScan & operator= (const ImpSvNumberformatScan &) = delete;

    void InitKeywords() const;
    void InitSpecialKeyword( NfKeywordIndex eIdx ) const;
    void InitCompatCur() const;

    void SetDependentKeywords();
                                                // Sets the language dependent keywords
    void SkipStrings(sal_uInt16& i, sal_Int32& nPos) const;// Skips StringSymbols
    sal_uInt16 PreviousKeyword(sal_uInt16 i) const;  // Returns index of the preceding one
                                                // Keyword or 0
    sal_uInt16 NextKeyword(sal_uInt16 i) const; // Returns index of the next one
                                                // Keyword or 0
    sal_Unicode PreviousChar(sal_uInt16 i) const; // Returns last char before index skips EMPTY, STRING, STAR, BLANK
    sal_Unicode NextChar(sal_uInt16 i) const;   // Returns first following char
    short PreviousType( sal_uInt16 i ) const;   // Returns type before position skips EMPTY
    bool IsLastBlankBeforeFrac(sal_uInt16 i) const; // True <=> there won't be a ' ' until the '/'
    void Reset();                               // Reset all variables before starting the analysis

    /** Determine keyword at nPos.
        @param  rbFoundEnglish set if English instead of locale's keyword
                found, never cleared, thus init with false.
        @return 0 if not found, else keyword enumeration.
     */
    short GetKeyWord( const OUString& sSymbol,
                      sal_Int32 nPos,
                      bool& rbFoundEnglish ) const;

    bool IsAmbiguousE( short nKey ) const  // whether nKey is ambiguous E of NF_KEY_E/NF_KEY_EC
        {
            return (nKey == NF_KEY_EC || nKey == NF_KEY_E) &&
                (GetKeywords()[NF_KEY_EC] == GetKeywords()[NF_KEY_E]);
        }

    // if 0 at strArray[i] is of S,00 or SS,00 or SS"any"00 in ScanType() or FinalScan()
    bool Is100SecZero( sal_uInt16 i, bool bHadDecSep ) const;

    short Next_Symbol(const OUString& rStr,
                      sal_Int32& nPos,
                      OUString& sSymbol) const; // Next Symbol
    sal_Int32 Symbol_Division(const OUString& rString);// Initial lexical scan
    sal_Int32 ScanType(); // Analysis of the Format type
    sal_Int32 FinalScan( OUString& rString ); // Final analysis with supplied type

    // -1:= error, return nPos in FinalScan; 0:= no calendar, 1:= calendar found
    int FinalScanGetCalendar( sal_Int32& nPos, sal_uInt16& i, sal_uInt16& nResultStringsCnt );

    /** Insert symbol into nTypeArray and sStrArray, e.g. grouping separator.
        If at nPos-1 a symbol type NF_SYMBOLTYPE_EMPTY is present, that is
        reused instead of shifting all one up and nPos is decremented! */
    bool InsertSymbol( sal_uInt16 & nPos, svt::NfSymbolType eType, const OUString& rStr );

    /** Whether two key symbols are adjacent separated by date separator.
        This can only be used at the end of FinalScan() after
        NF_SYMBOLTYPE_DATESEP has already been set.
     */
    bool IsDateFragment( size_t nPos1, size_t nPos2 ) const;

    /** Swap nTypeArray and sStrArray elements at positions. */
    void SwapArrayElements( size_t nPos1, size_t nPos2 );

    Color* GetUserDefColor(sal_uInt16 nIndex) const;

    static bool StringEqualsChar( std::u16string_view rStr, sal_Unicode ch )
        { return rStr.size() == 1 && rStr[0] == ch; }

    // remove "..." and \... quotes from rStr, return how many chars removed
    static sal_Int32 RemoveQuotes( OUString& rStr );
};

#endif // INCLUDED_SVL_SOURCE_NUMBERS_ZFORSCAN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
