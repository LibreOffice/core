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
#ifndef _ZFORSCAN_HXX
#define _ZFORSCAN_HXX

#include <i18nlangtag/lang.h>
#include <rtl/ustring.hxx>
#include <svl/nfkeytab.hxx>
#include <svl/nfsymbol.hxx>
#include <tools/color.hxx>
#include <tools/date.hxx>

class SvNumberFormatter;
struct ImpSvNumberformatInfo;


const size_t NF_MAX_FORMAT_SYMBOLS   = 100;
const size_t NF_MAX_DEFAULT_COLORS   = 10;

// Hack: nThousand==1000 => "Default" occurs in format string
const sal_uInt16 FLAG_STANDARD_IN_FORMAT = 1000;

class ImpSvNumberformatScan
{
public:

    ImpSvNumberformatScan( SvNumberFormatter* pFormatter );
    ~ImpSvNumberformatScan();
    void ChangeIntl(); // Replaces Keywords

    void ChangeNullDate(sal_uInt16 nDay, sal_uInt16 nMonth, sal_uInt16 nYear); // Replaces reference date
    void ChangeStandardPrec(sal_uInt16 nPrec); // Replaces standard precision

    sal_Int32 ScanFormat( OUString& rString ); // Call scan analysis

    void CopyInfo(ImpSvNumberformatInfo* pInfo,
                     sal_uInt16 nAnz); // Copies the FormatInfo
    sal_uInt16 GetAnzResStrings() const         { return nAnzResStrings; }

    const CharClass& GetChrCls() const          { return *pFormatter->GetCharClass(); }
    const LocaleDataWrapper& GetLoc() const     { return *pFormatter->GetLocaleData(); }
    CalendarWrapper& GetCal() const             { return *pFormatter->GetCalendar(); }

    const NfKeywordTable & GetKeywords() const
        {
            if ( bKeywordsNeedInit )
            {
                InitKeywords();
            }
            return sKeyword;
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
    const OUString& GetColorString() const    { return GetKeywords()[NF_KEY_COLOR]; }
    const OUString& GetRedString() const      { return GetKeywords()[NF_KEY_RED]; }
    const OUString& GetBooleanString() const  { return GetKeywords()[NF_KEY_BOOLEAN]; }
    const OUString& GetErrorString() const    { return sErrStr; }

    Date* GetNullDate() const                   { return pNullDate; }
    const OUString& GetStandardName() const
        {
            if ( bKeywordsNeedInit )
            {
                InitKeywords();
            }
            return sNameStandardFormat;
        }
    sal_uInt16 GetStandardPrec() const          { return nStandardPrec; }
    const Color& GetRedColor() const            { return StandardColor[4]; }
    Color* GetColor(OUString& sStr); // Set main colors or defines colors

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

    void SetConvertMode(LanguageType eTmpLge, LanguageType eNewLge,
            bool bSystemToSystem = false )
    {
        bConvertMode = true;
        eNewLnge = eNewLge;
        eTmpLnge = eTmpLge;
        bConvertSystemToSystem = bSystemToSystem;
    }
    // Only changes the bool variable, in order to temporarily pause the convert mode
    void SetConvertMode(bool bMode) { bConvertMode = bMode; }
    bool GetConvertMode() const     { return bConvertMode; }
    LanguageType GetNewLnge() const { return eNewLnge; } // Read access on ConvertMode and convert country/language
    LanguageType GetTmpLnge() const { return eTmpLnge; } // Read access on StartCountry/Language

                                                /// get Thai T speciality
    sal_uInt8 GetNatNumModifier() const      { return nNatNumModifier; }
                                                /// set Thai T speciality
    void SetNatNumModifier( sal_uInt8 n )    { nNatNumModifier = n; }

    SvNumberFormatter* GetNumberformatter() { return pFormatter; } // Access to formatter (for zformat.cxx)

private: // Private section
    NfKeywordTable sKeyword;                    // Syntax keywords
    Color StandardColor[NF_MAX_DEFAULT_COLORS]; // Standard color array
    Date* pNullDate;                            // 30Dec1899
    OUString sNameStandardFormat;               // "Standard"
    sal_uInt16 nStandardPrec;                   // Default Precision for Standardformat
    SvNumberFormatter* pFormatter;              // Pointer to the FormatList

    OUString sStrArray[NF_MAX_FORMAT_SYMBOLS];  // Array of symbols
    short nTypeArray[NF_MAX_FORMAT_SYMBOLS];    // Array of infos
                                                // External Infos:
    sal_uInt16 nAnzResStrings;                  // Result symbol count
    short eScannedType;                         // Type according to scan
    bool bThousand;                             // With thousands marker
    sal_uInt16 nThousand;                       // Counts .... series
    sal_uInt16 nCntPre;                         // Counts digits of integral part
    sal_uInt16 nCntPost;                        // Counts digits of fractional part
    sal_uInt16 nCntExp;                         // Counts exponent digits AM/PM
                                                // Internal info:
    sal_uInt16 nAnzStrings;                     // Symbol count
    sal_uInt16 nRepPos;                         // Position of a '*'
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
    OUString sErrStr;                           // String for error output

    bool bConvertMode;                          // Set in the convert mode

    LanguageType eNewLnge;                      // Language/country which the scanned string is converted to (for Excel filter)
    LanguageType eTmpLnge;                      // Language/country which the scanned string is converted from (for Excel filter)

    bool bConvertSystemToSystem;                // Whether the conversion is from one system locale to another system locale
                                                // (in this case the automatic currency symbol is converted too).

    sal_Int32 nCurrPos;                         // Position of currency symbol

    sal_uInt8 nNatNumModifier;                  // Thai T speciality

    void InitKeywords() const;
    void InitSpecialKeyword( NfKeywordIndex eIdx ) const;
    void InitCompatCur() const;

    void SetDependentKeywords();
                                                // Sets the language dependent keywords
    void SkipStrings(sal_uInt16& i, sal_Int32& nPos);// Skips StringSymbols
    sal_uInt16 PreviousKeyword(sal_uInt16 i);   // Returns index of the preceding one
                                                // Keyword or 0
    sal_uInt16 NextKeyword(sal_uInt16 i);       // Returns index of the next one
                                                // Keyword or 0
    sal_Unicode PreviousChar(sal_uInt16 i);     // Returns last char before index skips EMPTY, STRING, STAR, BLANK
    sal_Unicode NextChar(sal_uInt16 i);         // Returns first following char
    short PreviousType( sal_uInt16 i );         // Returns type before position skips EMPTY
    bool IsLastBlankBeforeFrac(sal_uInt16 i);   // True <=> there won't be a ' ' until the '/'
    void Reset();                               // Reset all variables before starting the analysis
    short GetKeyWord( const OUString& sSymbol,  // Determine keyword at nPos
                      sal_Int32 nPos );         // Return 0 <=> not found

    inline bool IsAmbiguousE( short nKey )      // whether nKey is ambiguous E of NF_KEY_E/NF_KEY_EC
        {
            return (nKey == NF_KEY_EC || nKey == NF_KEY_E) &&
                (GetKeywords()[NF_KEY_EC] == GetKeywords()[NF_KEY_E]);
        }

    // if 0 at strArray[i] is of S,00 or SS,00 or SS"any"00 in ScanType() or FinalScan()
    bool Is100SecZero( sal_uInt16 i, bool bHadDecSep );

    short Next_Symbol(const OUString& rStr,
                      sal_Int32& nPos,
                      OUString& sSymbol); // Next Symbol
    sal_Int32 Symbol_Division(const OUString& rString);// Initial lexical scan
    sal_Int32 ScanType(); // Analysis of the Format type
    sal_Int32 FinalScan( OUString& rString ); // Final analysis with supplied type

    // -1:= error, return nPos in FinalScan; 0:= no calendar, 1:= calendar found
    int FinalScanGetCalendar( sal_Int32& nPos, sal_uInt16& i, sal_uInt16& nAnzResStrings );

    /** Insert symbol into nTypeArray and sStrArray, e.g. grouping separator.
        If at nPos-1 a symbol type NF_SYMBOLTYPE_EMPTY is present, that is
        reused instead of shifting all one up and nPos is decremented! */
    bool InsertSymbol( sal_uInt16 & nPos, svt::NfSymbolType eType, const OUString& rStr );

    static inline bool StringEqualsChar( const OUString& rStr, sal_Unicode ch )
        { return rStr[0] == ch && rStr.getLength() == 1; }
        // Yes, for efficiency get the character first and then compare length
        // because in most places where this is used the string is one char.

    // remove "..." and \... quotes from rStr, return how many chars removed
    static sal_Int32 RemoveQuotes( OUString& rStr );
};

#endif  // _ZFORSCAN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
