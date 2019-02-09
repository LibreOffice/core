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


#include <stdlib.h>
#include <comphelper/string.hxx>
#include <sal/log.hxx>
#include <tools/debug.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <unotools/charclass.hxx>
#include <unotools/localedatawrapper.hxx>
#include <com/sun/star/i18n/NumberFormatCode.hpp>
#include <com/sun/star/i18n/NumberFormatMapper.hpp>
#include <com/sun/star/i18n/XNumberFormatCode.hpp>

#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <unotools/digitgroupingiterator.hxx>

#include "zforscan.hxx"

#include <svl/nfsymbol.hxx>
using namespace svt;

const sal_Unicode cNoBreakSpace = 0xA0;
const sal_Unicode cNarrowNoBreakSpace = 0x202F;

const int MaxCntPost = 20; //max dec places allow by rtl_math_round

const NfKeywordTable ImpSvNumberformatScan::sEnglishKeyword =
{             // Syntax keywords in English (USA)
    //! All keywords MUST be UPPERCASE! In same order as NfKeywordIndex
    "",        // NF_KEY_NONE 0
    "E",       // NF_KEY_E Exponent
    "AM/PM",   // NF_KEY_AMPM AM/PM
    "A/P",     // NF_KEY_AP AM/PM short
    "M",       // NF_KEY_MI Minute
    "MM",      // NF_KEY_MMI Minute 02
    "M",       // NF_KEY_M month        (!)
    "MM",      // NF_KEY_MM month 02     (!)
    "MMM",     // NF_KEY_MMM month short name
    "MMMM",    // NF_KEY_MMMM month long name
    "MMMMM",   // NF_KEY_MMMMM first letter of month name
    "H",       // NF_KEY_H hour
    "HH",      // NF_KEY_HH hour 02
    "S",       // NF_KEY_S Second
    "SS",      // NF_KEY_SS Second 02
    "Q",       // NF_KEY_Q Quarter short 'Q'
    "QQ",      // NF_KEY_QQ Quarter long
    "D",       // NF_KEY_D day of month
    "DD",      // NF_KEY_DD day of month 02
    "DDD",     // NF_KEY_DDD day of week short
    "DDDD",    // NF_KEY_DDDD day of week long
    "YY",      // NF_KEY_YY year two digits
    "YYYY",    // NF_KEY_YYYY year four digits
    "NN",      // NF_KEY_NN Day of week short
    "NNN",     // NF_KEY_NNN Day of week long
    "NNNN",    // NF_KEY_NNNN Day of week long incl. separator
    "AAA",     // NF_KEY_AAA
    "AAAA",    // NF_KEY_AAAA
    "E",       // NF_KEY_EC
    "EE",      // NF_KEY_EEC
    "G",       // NF_KEY_G
    "GG",      // NF_KEY_GG
    "GGG",     // NF_KEY_GGG
    "R",       // NF_KEY_R
    "RR",      // NF_KEY_RR
    "WW",      // NF_KEY_WW Week of year
    "t",       // NF_KEY_THAI_T Thai T modifier, speciality of Thai Excel, only
                // used with Thai locale and converted to [NatNum1], only
                // exception as lowercase
    "CCC",     // NF_KEY_CCC Currency abbreviation
    "GENERAL", // NF_KEY_GENERAL General / Standard

    // Reserved words translated and color names follow:
    "TRUE",    // NF_KEY_TRUE boolean true
    "FALSE",   // NF_KEY_FALSE boolean false
    "BOOLEAN", // NF_KEY_BOOLEAN boolean
    "COLOR",   // NF_KEY_COLOR color
        // colours
    "BLACK",   // NF_KEY_BLACK
    "BLUE",    // NF_KEY_BLUE
    "GREEN",   // NF_KEY_GREEN
    "CYAN",    // NF_KEY_CYAN
    "RED",     // NF_KEY_RED
    "MAGENTA", // NF_KEY_MAGENTA
    "BROWN",   // NF_KEY_BROWN
    "GREY",    // NF_KEY_GREY
    "YELLOW",  // NF_KEY_YELLOW
    "WHITE"    // NF_KEY_WHITE
};

::std::vector<Color> ImpSvNumberformatScan::StandardColor;
bool ImpSvNumberformatScan::bStandardColorNeedInitialization = true;

// This vector will hold *only* the color names in German language.
::std::vector<OUString> ImpSvNumberformatScan::sGermanColorNames;

const OUString ImpSvNumberformatScan::sErrStr =  "###";

ImpSvNumberformatScan::ImpSvNumberformatScan( SvNumberFormatter* pFormatterP )
    : maNullDate( 30, 12, 1899)
    , eNewLnge(LANGUAGE_DONTKNOW)
    , eTmpLnge(LANGUAGE_DONTKNOW)
    , nCurrPos(-1)
    , meKeywordLocalization(KeywordLocalization::AllowEnglish)
{
    pFormatter = pFormatterP;
    xNFC = css::i18n::NumberFormatMapper::create( pFormatter->GetComponentContext() );
    bConvertMode = false;
    mbConvertDateOrder = false;
    bConvertSystemToSystem = false;
    bKeywordsNeedInit = true;            // locale dependent and not locale dependent keywords
    bCompatCurNeedInit = true;           // locale dependent compatibility currency strings

    if ( bStandardColorNeedInitialization )
    {
        bStandardColorNeedInitialization = false;
        StandardColor.push_back( COL_BLACK );
        StandardColor.push_back( COL_LIGHTBLUE );
        StandardColor.push_back( COL_LIGHTGREEN );
        StandardColor.push_back( COL_LIGHTCYAN );
        StandardColor.push_back( COL_LIGHTRED );
        StandardColor.push_back( COL_LIGHTMAGENTA );
        StandardColor.push_back( COL_BROWN );
        StandardColor.push_back( COL_GRAY );
        StandardColor.push_back( COL_YELLOW );
        StandardColor.push_back( COL_WHITE );

        static_assert( NF_KEY_BLACK - NF_KEY_COLOR == 1,        "bad FARBE(COLOR), SCHWARZ(BLACK) sequence");
        static_assert( NF_KEY_FIRSTCOLOR - NF_KEY_COLOR == 1,   "bad color sequence");
        static_assert( NF_MAX_DEFAULT_COLORS + 1 == 11,         "bad color count");
        static_assert( NF_KEY_WHITE - NF_KEY_COLOR + 1 == 11,   "bad color sequence count");

        sGermanColorNames.resize( NF_KEY_WHITE - NF_KEY_COLOR + 1 );
        sGermanColorNames[NF_KEY_COLOR - NF_KEY_COLOR]   = "FARBE";
        sGermanColorNames[NF_KEY_BLACK - NF_KEY_COLOR]   = "SCHWARZ";
        sGermanColorNames[NF_KEY_BLUE - NF_KEY_COLOR]    = "BLAU";
        sGermanColorNames[NF_KEY_GREEN - NF_KEY_COLOR]   = OUString( "GR" "\xDC" "N", 4, RTL_TEXTENCODING_ISO_8859_1 );
        sGermanColorNames[NF_KEY_CYAN - NF_KEY_COLOR]    = "CYAN";
        sGermanColorNames[NF_KEY_RED - NF_KEY_COLOR]     = "ROT";
        sGermanColorNames[NF_KEY_MAGENTA - NF_KEY_COLOR] = "MAGENTA";
        sGermanColorNames[NF_KEY_BROWN - NF_KEY_COLOR]   = "BRAUN";
        sGermanColorNames[NF_KEY_GREY - NF_KEY_COLOR]    = "GRAU";
        sGermanColorNames[NF_KEY_YELLOW - NF_KEY_COLOR]  = "GELB";
        sGermanColorNames[NF_KEY_WHITE - NF_KEY_COLOR]   = "WEISS";
    }

    nStandardPrec = 2;

    Reset();
}

ImpSvNumberformatScan::~ImpSvNumberformatScan()
{
    Reset();
}

void ImpSvNumberformatScan::ChangeIntl( KeywordLocalization eKeywordLocalization )
{
    meKeywordLocalization = eKeywordLocalization;
    bKeywordsNeedInit = true;
    bCompatCurNeedInit = true;
    // may be initialized by InitSpecialKeyword()
    sKeyword[NF_KEY_TRUE].clear();
    sKeyword[NF_KEY_FALSE].clear();
}

void ImpSvNumberformatScan::InitSpecialKeyword( NfKeywordIndex eIdx ) const
{
    switch ( eIdx )
    {
    case NF_KEY_TRUE :
        const_cast<ImpSvNumberformatScan*>(this)->sKeyword[NF_KEY_TRUE] =
            pFormatter->GetCharClass()->uppercase( pFormatter->GetLocaleData()->getTrueWord() );
        if ( sKeyword[NF_KEY_TRUE].isEmpty() )
        {
            SAL_WARN( "svl.numbers", "InitSpecialKeyword: TRUE_WORD?" );
            const_cast<ImpSvNumberformatScan*>(this)->sKeyword[NF_KEY_TRUE] = sEnglishKeyword[NF_KEY_TRUE];
        }
        break;
    case NF_KEY_FALSE :
        const_cast<ImpSvNumberformatScan*>(this)->sKeyword[NF_KEY_FALSE] =
            pFormatter->GetCharClass()->uppercase( pFormatter->GetLocaleData()->getFalseWord() );
        if ( sKeyword[NF_KEY_FALSE].isEmpty() )
        {
            SAL_WARN( "svl.numbers", "InitSpecialKeyword: FALSE_WORD?" );
            const_cast<ImpSvNumberformatScan*>(this)->sKeyword[NF_KEY_FALSE] = sEnglishKeyword[NF_KEY_FALSE];
        }
        break;
    default:
        SAL_WARN( "svl.numbers", "InitSpecialKeyword: unknown request" );
    }
}

void ImpSvNumberformatScan::InitCompatCur() const
{
    ImpSvNumberformatScan* pThis = const_cast<ImpSvNumberformatScan*>(this);
    // currency symbol for old style ("automatic") compatibility format codes
    pFormatter->GetCompatibilityCurrency( pThis->sCurSymbol, pThis->sCurAbbrev );
    // currency symbol upper case
    pThis->sCurString = pFormatter->GetCharClass()->uppercase( sCurSymbol );
    bCompatCurNeedInit = false;
}

void ImpSvNumberformatScan::InitKeywords() const
{
    if ( !bKeywordsNeedInit )
        return ;
    const_cast<ImpSvNumberformatScan*>(this)->SetDependentKeywords();
    bKeywordsNeedInit = false;
}

/** Extract the name of General, Standard, Whatever, ignoring leading modifiers
    such as [NatNum1]. */
static OUString lcl_extractStandardGeneralName( const OUString & rCode )
{
    OUString aStr;
    const sal_Unicode* p = rCode.getStr();
    const sal_Unicode* const pStop = p + rCode.getLength();
    const sal_Unicode* pBeg = p;    // name begins here
    bool bMod = false;
    bool bDone = false;
    while (p < pStop && !bDone)
    {
        switch (*p)
        {
        case '[':
            bMod = true;
            break;
        case ']':
            if (bMod)
            {
                bMod = false;
                pBeg = p+1;
            }
            // else: would be a locale data error, easily to be spotted in
            // UI dialog
            break;
        case ';':
            if (!bMod)
            {
                bDone = true;
                --p;    // put back, increment by one follows
            }
            break;
        }
        ++p;
        if (bMod)
        {
            pBeg = p;
        }
    }
    if (pBeg < p)
    {
        aStr = rCode.copy( pBeg - rCode.getStr(), p - pBeg);
    }
    return aStr;
}

void ImpSvNumberformatScan::SetDependentKeywords()
{
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;

    const CharClass* pCharClass = pFormatter->GetCharClass();
    const LocaleDataWrapper* pLocaleData = pFormatter->GetLocaleData();
    // #80023# be sure to generate keywords for the loaded Locale, not for the
    // requested Locale, otherwise number format codes might not match
    const LanguageTag& rLoadedLocale = pLocaleData->getLoadedLanguageTag();
    LanguageType eLang = rLoadedLocale.getLanguageType( false);

    bool bL10n = (meKeywordLocalization != KeywordLocalization::EnglishOnly);
    if (bL10n)
    {
        // Check if this actually is a locale that uses any localized keywords,
        // if not then disable localized keywords completely.
        if ( !eLang.anyOf( LANGUAGE_GERMAN,
                    LANGUAGE_GERMAN_SWISS,
                    LANGUAGE_GERMAN_AUSTRIAN,
                    LANGUAGE_GERMAN_LUXEMBOURG,
                    LANGUAGE_GERMAN_LIECHTENSTEIN,
                    LANGUAGE_DUTCH,
                    LANGUAGE_DUTCH_BELGIAN,
                    LANGUAGE_FRENCH,
                    LANGUAGE_FRENCH_BELGIAN,
                    LANGUAGE_FRENCH_CANADIAN,
                    LANGUAGE_FRENCH_SWISS,
                    LANGUAGE_FRENCH_LUXEMBOURG,
                    LANGUAGE_FRENCH_MONACO,
                    LANGUAGE_FINNISH,
                    LANGUAGE_ITALIAN,
                    LANGUAGE_ITALIAN_SWISS,
                    LANGUAGE_DANISH,
                    LANGUAGE_NORWEGIAN,
                    LANGUAGE_NORWEGIAN_BOKMAL,
                    LANGUAGE_NORWEGIAN_NYNORSK,
                    LANGUAGE_SWEDISH,
                    LANGUAGE_SWEDISH_FINLAND,
                    LANGUAGE_PORTUGUESE,
                    LANGUAGE_PORTUGUESE_BRAZILIAN,
                    LANGUAGE_SPANISH_MODERN,
                    LANGUAGE_SPANISH_DATED,
                    LANGUAGE_SPANISH_MEXICAN,
                    LANGUAGE_SPANISH_GUATEMALA,
                    LANGUAGE_SPANISH_COSTARICA,
                    LANGUAGE_SPANISH_PANAMA,
                    LANGUAGE_SPANISH_DOMINICAN_REPUBLIC,
                    LANGUAGE_SPANISH_VENEZUELA,
                    LANGUAGE_SPANISH_COLOMBIA,
                    LANGUAGE_SPANISH_PERU,
                    LANGUAGE_SPANISH_ARGENTINA,
                    LANGUAGE_SPANISH_ECUADOR,
                    LANGUAGE_SPANISH_CHILE,
                    LANGUAGE_SPANISH_URUGUAY,
                    LANGUAGE_SPANISH_PARAGUAY,
                    LANGUAGE_SPANISH_BOLIVIA,
                    LANGUAGE_SPANISH_EL_SALVADOR,
                    LANGUAGE_SPANISH_HONDURAS,
                    LANGUAGE_SPANISH_NICARAGUA,
                    LANGUAGE_SPANISH_PUERTO_RICO ))
        {
            bL10n = false;
            meKeywordLocalization = KeywordLocalization::EnglishOnly;
        }
    }

    // Init the current NfKeywordTable with English keywords.
    sKeyword = sEnglishKeyword;

    // Set the uppercase localized General name, e.g. Standard -> STANDARD
    i18n::NumberFormatCode aFormat = xNFC->getFormatCode( NF_NUMBER_STANDARD, rLoadedLocale.getLocale() );
    sNameStandardFormat = lcl_extractStandardGeneralName( aFormat.Code );
    sKeyword[NF_KEY_GENERAL] = pCharClass->uppercase( sNameStandardFormat );

    // Thai T NatNum special. Other locale's small letter 't' results in upper
    // case comparison not matching but length does in conversion mode. Ugly.
    if (eLang == LANGUAGE_THAI)
    {
        sKeyword[NF_KEY_THAI_T] = "T";
    }
    else
    {
        sKeyword[NF_KEY_THAI_T] = sEnglishKeyword[NF_KEY_THAI_T];
    }

    // boolean keywords
    InitSpecialKeyword( NF_KEY_TRUE );
    InitSpecialKeyword( NF_KEY_FALSE );

    // Boolean equivalent format codes that are written to Excel files, may
    // have been written to ODF as well, specifically if such loaded Excel file
    // was saved as ODF, and shall result in proper Boolean again.
    // "TRUE";"TRUE";"FALSE"
    sBooleanEquivalent1 = "\"" + sKeyword[NF_KEY_TRUE] + "\";\"" +
        sKeyword[NF_KEY_TRUE] + "\";\"" + sKeyword[NF_KEY_FALSE] + "\"";
    // [>0]"TRUE";[<0]"TRUE";"FALSE"
    sBooleanEquivalent2 = "[>0]\"" + sKeyword[NF_KEY_TRUE] + "\";[<0]\"" +
        sKeyword[NF_KEY_TRUE] + "\";\"" + sKeyword[NF_KEY_FALSE] + "\"";

    // compatibility currency strings
    InitCompatCur();

    if (!bL10n)
        return;

    // All locale dependent keywords overrides follow.

    if ( eLang.anyOf(
            LANGUAGE_GERMAN,
            LANGUAGE_GERMAN_SWISS,
            LANGUAGE_GERMAN_AUSTRIAN,
            LANGUAGE_GERMAN_LUXEMBOURG,
            LANGUAGE_GERMAN_LIECHTENSTEIN))
    {
        //! all capital letters
        sKeyword[NF_KEY_M] =         "M";     // month 1
        sKeyword[NF_KEY_MM] =        "MM";    // month 01
        sKeyword[NF_KEY_MMM] =       "MMM";   // month Jan
        sKeyword[NF_KEY_MMMM] =      "MMMM";  // month Januar
        sKeyword[NF_KEY_MMMMM] =     "MMMMM"; // month J
        sKeyword[NF_KEY_H] =         "H";     // hour 2
        sKeyword[NF_KEY_HH] =        "HH";    // hour 02
        sKeyword[NF_KEY_D] =         "T";
        sKeyword[NF_KEY_DD] =        "TT";
        sKeyword[NF_KEY_DDD] =       "TTT";
        sKeyword[NF_KEY_DDDD] =      "TTTT";
        sKeyword[NF_KEY_YY] =        "JJ";
        sKeyword[NF_KEY_YYYY] =      "JJJJ";
        sKeyword[NF_KEY_BOOLEAN] =   "LOGISCH";
        sKeyword[NF_KEY_COLOR] =     sGermanColorNames[NF_KEY_COLOR - NF_KEY_COLOR];
        sKeyword[NF_KEY_BLACK] =     sGermanColorNames[NF_KEY_BLACK - NF_KEY_COLOR];
        sKeyword[NF_KEY_BLUE] =      sGermanColorNames[NF_KEY_BLUE - NF_KEY_COLOR];
        sKeyword[NF_KEY_GREEN] =     sGermanColorNames[NF_KEY_GREEN - NF_KEY_COLOR];
        sKeyword[NF_KEY_CYAN] =      sGermanColorNames[NF_KEY_CYAN - NF_KEY_COLOR];
        sKeyword[NF_KEY_RED] =       sGermanColorNames[NF_KEY_RED - NF_KEY_COLOR];
        sKeyword[NF_KEY_MAGENTA] =   sGermanColorNames[NF_KEY_MAGENTA - NF_KEY_COLOR];
        sKeyword[NF_KEY_BROWN] =     sGermanColorNames[NF_KEY_BROWN - NF_KEY_COLOR];
        sKeyword[NF_KEY_GREY] =      sGermanColorNames[NF_KEY_GREY - NF_KEY_COLOR];
        sKeyword[NF_KEY_YELLOW] =    sGermanColorNames[NF_KEY_YELLOW - NF_KEY_COLOR];
        sKeyword[NF_KEY_WHITE] =     sGermanColorNames[NF_KEY_WHITE - NF_KEY_COLOR];
    }
    else
    {
        // day
        if ( eLang.anyOf(
                LANGUAGE_ITALIAN,
                LANGUAGE_ITALIAN_SWISS))
        {
            sKeyword[NF_KEY_D] = "G";
            sKeyword[NF_KEY_DD] = "GG";
            sKeyword[NF_KEY_DDD] = "GGG";
            sKeyword[NF_KEY_DDDD] = "GGGG";
            // must exchange the era code, same as Xcl
            sKeyword[NF_KEY_G] = "X";
            sKeyword[NF_KEY_GG] = "XX";
            sKeyword[NF_KEY_GGG] = "XXX";
        }
        else if ( eLang.anyOf(
                 LANGUAGE_FRENCH,
                 LANGUAGE_FRENCH_BELGIAN,
                 LANGUAGE_FRENCH_CANADIAN,
                 LANGUAGE_FRENCH_SWISS,
                 LANGUAGE_FRENCH_LUXEMBOURG,
                 LANGUAGE_FRENCH_MONACO))
        {
            sKeyword[NF_KEY_D] = "J";
            sKeyword[NF_KEY_DD] = "JJ";
            sKeyword[NF_KEY_DDD] = "JJJ";
            sKeyword[NF_KEY_DDDD] = "JJJJ";
        }
        else if ( eLang == LANGUAGE_FINNISH )
        {
            sKeyword[NF_KEY_D] = "P";
            sKeyword[NF_KEY_DD] = "PP";
            sKeyword[NF_KEY_DDD] = "PPP";
            sKeyword[NF_KEY_DDDD] = "PPPP";
        }

        // month
        if ( eLang == LANGUAGE_FINNISH )
        {
            sKeyword[NF_KEY_M] = "K";
            sKeyword[NF_KEY_MM] = "KK";
            sKeyword[NF_KEY_MMM] = "KKK";
            sKeyword[NF_KEY_MMMM] = "KKKK";
            sKeyword[NF_KEY_MMMMM] = "KKKKK";
        }

        // year
        if ( eLang.anyOf(
            LANGUAGE_ITALIAN,
            LANGUAGE_ITALIAN_SWISS,
            LANGUAGE_FRENCH,
            LANGUAGE_FRENCH_BELGIAN,
            LANGUAGE_FRENCH_CANADIAN,
            LANGUAGE_FRENCH_SWISS,
            LANGUAGE_FRENCH_LUXEMBOURG,
            LANGUAGE_FRENCH_MONACO,
            LANGUAGE_PORTUGUESE,
            LANGUAGE_PORTUGUESE_BRAZILIAN,
            LANGUAGE_SPANISH_MODERN,
            LANGUAGE_SPANISH_DATED,
            LANGUAGE_SPANISH_MEXICAN,
            LANGUAGE_SPANISH_GUATEMALA,
            LANGUAGE_SPANISH_COSTARICA,
            LANGUAGE_SPANISH_PANAMA,
            LANGUAGE_SPANISH_DOMINICAN_REPUBLIC,
            LANGUAGE_SPANISH_VENEZUELA,
            LANGUAGE_SPANISH_COLOMBIA,
            LANGUAGE_SPANISH_PERU,
            LANGUAGE_SPANISH_ARGENTINA,
            LANGUAGE_SPANISH_ECUADOR,
            LANGUAGE_SPANISH_CHILE,
            LANGUAGE_SPANISH_URUGUAY,
            LANGUAGE_SPANISH_PARAGUAY,
            LANGUAGE_SPANISH_BOLIVIA,
            LANGUAGE_SPANISH_EL_SALVADOR,
            LANGUAGE_SPANISH_HONDURAS,
            LANGUAGE_SPANISH_NICARAGUA,
            LANGUAGE_SPANISH_PUERTO_RICO))
        {
            sKeyword[NF_KEY_YY] = "AA";
            sKeyword[NF_KEY_YYYY] = "AAAA";
            // must exchange the day of week name code, same as Xcl
            sKeyword[NF_KEY_AAA] =   "OOO";
            sKeyword[NF_KEY_AAAA] =  "OOOO";
        }
        else if ( eLang.anyOf(
             LANGUAGE_DUTCH,
             LANGUAGE_DUTCH_BELGIAN))
        {
            sKeyword[NF_KEY_YY] = "JJ";
            sKeyword[NF_KEY_YYYY] = "JJJJ";
        }
        else if ( eLang == LANGUAGE_FINNISH )
        {
            sKeyword[NF_KEY_YY] = "VV";
            sKeyword[NF_KEY_YYYY] = "VVVV";
        }

        // hour
        if ( eLang.anyOf(
             LANGUAGE_DUTCH,
             LANGUAGE_DUTCH_BELGIAN))
        {
            sKeyword[NF_KEY_H] = "U";
            sKeyword[NF_KEY_HH] = "UU";
        }
        else if ( eLang.anyOf(
            LANGUAGE_FINNISH,
            LANGUAGE_SWEDISH,
            LANGUAGE_SWEDISH_FINLAND,
            LANGUAGE_DANISH,
            LANGUAGE_NORWEGIAN,
            LANGUAGE_NORWEGIAN_BOKMAL,
            LANGUAGE_NORWEGIAN_NYNORSK))
        {
            sKeyword[NF_KEY_H] = "T";
            sKeyword[NF_KEY_HH] = "TT";
        }
    }
}

void ImpSvNumberformatScan::ChangeNullDate(sal_uInt16 nDay, sal_uInt16 nMonth, sal_Int16 nYear)
{
    maNullDate = Date(nDay, nMonth, nYear);
    if (!maNullDate.IsValidDate())
    {
        maNullDate.Normalize();
        SAL_WARN("svl.numbers","ImpSvNumberformatScan::ChangeNullDate - not valid"
                " d: " << nDay << " m: " << nMonth << " y: " << nYear << " normalized to"
                " d: " << maNullDate.GetDay() << " m: " << maNullDate.GetMonth() << " y: " << maNullDate.GetYear());
    }
}

void ImpSvNumberformatScan::ChangeStandardPrec(sal_uInt16 nPrec)
{
    nStandardPrec = nPrec;
}

Color* ImpSvNumberformatScan::GetColor(OUString& sStr)
{
    OUString sString = pFormatter->GetCharClass()->uppercase(sStr);
    const NfKeywordTable & rKeyword = GetKeywords();
    size_t i = 0;
    while (i < NF_MAX_DEFAULT_COLORS && sString != rKeyword[NF_KEY_FIRSTCOLOR+i] )
    {
        i++;
    }
    if (i >= NF_MAX_DEFAULT_COLORS && meKeywordLocalization == KeywordLocalization::AllowEnglish)
    {
        LanguageType eLang = pFormatter->GetLocaleData()->getLoadedLanguageTag().getLanguageType( false);
        if ( eLang.anyOf(
                    LANGUAGE_GERMAN,
                    LANGUAGE_GERMAN_SWISS,
                    LANGUAGE_GERMAN_AUSTRIAN,
                    LANGUAGE_GERMAN_LUXEMBOURG,
                    LANGUAGE_GERMAN_LIECHTENSTEIN )) // only German uses localized color names
        {
            size_t j = 0;
            while ( j < NF_MAX_DEFAULT_COLORS && sString != sEnglishKeyword[NF_KEY_FIRSTCOLOR + j] )
            {
                ++j;
            }
            if ( j < NF_MAX_DEFAULT_COLORS )
            {
                i = j;
            }
        }
    }

    enum ColorKeywordConversion
    {
        None,
        GermanToEnglish,
        EnglishToGerman
    } eColorKeywordConversion(None);

    if (bConvertMode)
    {
        const bool bFromGerman = eTmpLnge.anyOf(
                LANGUAGE_GERMAN,
                LANGUAGE_GERMAN_SWISS,
                LANGUAGE_GERMAN_AUSTRIAN,
                LANGUAGE_GERMAN_LUXEMBOURG,
                LANGUAGE_GERMAN_LIECHTENSTEIN);
        const bool bToGerman = eNewLnge.anyOf(
                LANGUAGE_GERMAN,
                LANGUAGE_GERMAN_SWISS,
                LANGUAGE_GERMAN_AUSTRIAN,
                LANGUAGE_GERMAN_LUXEMBOURG,
                LANGUAGE_GERMAN_LIECHTENSTEIN);
        if (bFromGerman && !bToGerman)
            eColorKeywordConversion = ColorKeywordConversion::GermanToEnglish;
        else if (!bFromGerman && bToGerman)
            eColorKeywordConversion = ColorKeywordConversion::EnglishToGerman;
    }

    Color* pResult = nullptr;
    if (i >= NF_MAX_DEFAULT_COLORS)
    {
        const OUString& rColorWord = rKeyword[NF_KEY_COLOR];
        bool bL10n = true;
        if ((bL10n = sString.startsWith(rColorWord)) ||
                ((meKeywordLocalization == KeywordLocalization::AllowEnglish) &&
                 sString.startsWith(sEnglishKeyword[NF_KEY_COLOR])))
        {
            sal_Int32 nPos = (bL10n ? rColorWord.getLength() : sEnglishKeyword[NF_KEY_COLOR].getLength());
            sStr = sStr.copy(nPos);
            sStr = comphelper::string::strip(sStr, ' ');
            switch (eColorKeywordConversion)
            {
                case ColorKeywordConversion::None:
                    sStr = rColorWord + sStr;
                break;
                case ColorKeywordConversion::GermanToEnglish:
                    sStr = sEnglishKeyword[NF_KEY_COLOR] + sStr;                    // Farbe -> COLOR
                break;
                case ColorKeywordConversion::EnglishToGerman:
                    sStr = sGermanColorNames[NF_KEY_COLOR - NF_KEY_COLOR] + sStr;   // Color -> FARBE
                break;
            }
            sString = sString.copy(nPos);
            sString = comphelper::string::strip(sString, ' ');

            if ( CharClass::isAsciiNumeric( sString ) )
            {
                sal_Int32 nIndex = sString.toInt32();
                if (nIndex > 0 && nIndex <= 64)
                {
                    pResult = pFormatter->GetUserDefColor(static_cast<sal_uInt16>(nIndex)-1);
                }
            }
        }
    }
    else
    {
        sStr.clear();
        switch (eColorKeywordConversion)
        {
            case ColorKeywordConversion::None:
                sStr = rKeyword[NF_KEY_FIRSTCOLOR+i];
            break;
            case ColorKeywordConversion::GermanToEnglish:
                sStr = sEnglishKeyword[NF_KEY_FIRSTCOLOR + i];                  // Rot -> RED
            break;
            case ColorKeywordConversion::EnglishToGerman:
                sStr = sGermanColorNames[NF_KEY_FIRSTCOLOR - NF_KEY_COLOR + i]; // Red -> ROT
            break;
        }
        pResult = &(StandardColor[i]);
    }
    return pResult;
}

short ImpSvNumberformatScan::GetKeyWord( const OUString& sSymbol, sal_Int32 nPos, bool& rbFoundEnglish ) const
{
    OUString sString = pFormatter->GetCharClass()->uppercase( sSymbol, nPos, sSymbol.getLength() - nPos );
    const NfKeywordTable & rKeyword = GetKeywords();
    // #77026# for the Xcl perverts: the GENERAL keyword is recognized anywhere
    if (sString.startsWith( rKeyword[NF_KEY_GENERAL] ))
    {
        return NF_KEY_GENERAL;
    }
    if ((meKeywordLocalization == KeywordLocalization::AllowEnglish) &&
            sString.startsWith( sEnglishKeyword[NF_KEY_GENERAL]))
    {
        rbFoundEnglish = true;
        return NF_KEY_GENERAL;
    }

    // MUST be a reverse search to find longer strings first,
    // new keywords take precedence over old keywords,
    // skip colors et al after keywords.
    short i = NF_KEY_LASTKEYWORD;
    while (i > 0 && !sString.startsWith( rKeyword[i]))
    {
        i--;
    }
    if (i == 0 && meKeywordLocalization == KeywordLocalization::AllowEnglish)
    {
        // No localized (if so) keyword, try English keywords if keywords
        // are localized. That was already checked in SetDependentKeywords().
        i = NF_KEY_LASTKEYWORD;
        while (i > 0 && !sString.startsWith( sEnglishKeyword[i]))
        {
            i--;
        }
    }

    // The Thai T NatNum modifier during Xcl import.
    if (i == 0 && bConvertMode &&
        sString[0] == 'T' &&
        eTmpLnge == LANGUAGE_ENGLISH_US &&
        MsLangId::getRealLanguage( eNewLnge) == LANGUAGE_THAI)
    {
        i = NF_KEY_THAI_T;
    }
    return i; // 0 => not found
}

/**
 * Next_Symbol
 *
 * Splits up the input for further processing (by the Turing machine).
 *
 * Starting state = SsStar
 *
 * ---------------+-------------------+---------------------------+---------------
 * Old state      | Character read    | Event                     | New state
 * ---------------+-------------------+---------------------------+---------------
 * SsStart        | Character         | Symbol = Character        | SsGetWord
 *                |    "              | Type = String             | SsGetString
 *                |    \              | Type = String             | SsGetChar
 *                |    *              | Type = Star               | SsGetStar
 *                |    _              | Type = Blank              | SsGetBlank
 *                | @ # 0 ? / . , % [ | Symbol = Character;       |
 *                | ] ' Blank         | Type = Control character  | SsStop
 *                | $ - + ( ) :       | Type  = String;           |
 *                | Else              | Symbol = Character        | SsStop
 * ---------------|-------------------+---------------------------+---------------
 * SsGetChar      | Else              | Symbol = Character        | SsStop
 * ---------------+-------------------+---------------------------+---------------
 * GetString      | "                 |                           | SsStop
 *                | Else              | Symbol += Character       | GetString
 * ---------------+-------------------+---------------------------+---------------
 * SsGetWord      | Character         | Symbol += Character       |
 *                | + -        (E+ E-)| Symbol += Character       | SsStop
 *                | /          (AM/PM)| Symbol += Character       |
 *                | Else              | Pos--, if Key Type = Word | SsStop
 * ---------------+-------------------+---------------------------+---------------
 * SsGetStar      | Else              | Symbol += Character       | SsStop
 *                |                   | Mark special case *       |
 * ---------------+-------------------+---------------------------+---------------
 * SsGetBlank     | Else              | Symbol + =Character       | SsStop
 *                |                   | Mark special case  _      |
 * ---------------------------------------------------------------+--------------
 *
 * If we recognize a keyword in the state SsGetWord (even as the symbol's start text)
 * we write back the rest of the characters!
 */

enum ScanState
{
    SsStop      = 0,
    SsStart     = 1,
    SsGetChar   = 2,
    SsGetString = 3,
    SsGetWord   = 4,
    SsGetStar   = 5,
    SsGetBlank  = 6
};

short ImpSvNumberformatScan::Next_Symbol( const OUString& rStr,
                                          sal_Int32& nPos,
                                          OUString& sSymbol ) const
{
    InitKeywords();
    const CharClass* pChrCls = pFormatter->GetCharClass();
    const LocaleDataWrapper* pLoc = pFormatter->GetLocaleData();
    short eType = 0;
    ScanState eState = SsStart;
    OUStringBuffer sSymbolBuffer;
    while ( nPos < rStr.getLength() && eState != SsStop )
    {
        sal_Unicode cToken = rStr[nPos++];
        switch (eState)
        {
        case SsStart:
            // Fetch any currency longer than one character and don't get
            // confused later on by "E/" or other combinations of letters
            // and meaningful symbols. Necessary for old automatic currency.
            // #96158# But don't do it if we're starting a "[...]" section,
            // for example a "[$...]" new currency symbol to not parse away
            // "$U" (symbol) of "[$UYU]" (abbreviation).
            if ( nCurrPos >= 0 && sCurString.getLength() > 1 &&
                 nPos-1 + sCurString.getLength() <= rStr.getLength() &&
                 !(nPos > 1 && rStr[nPos-2] == '[') )
            {
                OUString aTest = pChrCls->uppercase( rStr.copy( nPos-1, sCurString.getLength() ) );
                if ( aTest == sCurString )
                {
                    sSymbol = rStr.copy( --nPos, sCurString.getLength() );
                    nPos = nPos + sSymbol.getLength();
                    eType = NF_SYMBOLTYPE_STRING;
                    return eType;
                }
            }
            switch (cToken)
            {
            case '#':
            case '0':
            case '?':
            case '%':
            case '@':
            case '[':
            case ']':
            case ',':
            case '.':
            case '/':
            case '\'':
            case ' ':
            case ':':
            case '-':
                eType = NF_SYMBOLTYPE_DEL;
                sSymbolBuffer.append(OUStringLiteral1(cToken));
                eState = SsStop;
                break;
            case '*':
                eType = NF_SYMBOLTYPE_STAR;
                sSymbolBuffer.append(OUStringLiteral1(cToken));
                eState = SsGetStar;
                break;
            case '_':
                eType = NF_SYMBOLTYPE_BLANK;
                sSymbolBuffer.append(OUStringLiteral1(cToken));
                eState = SsGetBlank;
                break;
            case '"':
                eType = NF_SYMBOLTYPE_STRING;
                eState = SsGetString;
                sSymbolBuffer.append(OUStringLiteral1(cToken));
                break;
            case '\\':
                eType = NF_SYMBOLTYPE_STRING;
                eState = SsGetChar;
                sSymbolBuffer.append(OUStringLiteral1(cToken));
                break;
            case '$':
            case '+':
            case '(':
            case ')':
                eType = NF_SYMBOLTYPE_STRING;
                eState = SsStop;
                sSymbolBuffer.append(OUStringLiteral1(cToken));
                break;
            default :
                if (StringEqualsChar( pFormatter->GetNumDecimalSep(), cToken) ||
                    StringEqualsChar( pFormatter->GetNumThousandSep(), cToken) ||
                    StringEqualsChar( pFormatter->GetDateSep(), cToken) ||
                    StringEqualsChar( pLoc->getTimeSep(), cToken) ||
                    StringEqualsChar( pLoc->getTime100SecSep(), cToken))
                {
                    // Another separator than pre-known ASCII
                    eType = NF_SYMBOLTYPE_DEL;
                    sSymbolBuffer.append(OUStringLiteral1(cToken));
                    eState = SsStop;
                }
                else if ( pChrCls->isLetter( rStr, nPos-1 ) )
                {
                    bool bFoundEnglish = false;
                    short nTmpType = GetKeyWord( rStr, nPos-1, bFoundEnglish);
                    if ( nTmpType )
                    {
                        bool bCurrency = false;
                        // "Automatic" currency may start with keyword,
                        // like "R" (Rand) and 'R' (era)
                        if ( nCurrPos >= 0 &&
                             nPos-1 + sCurString.getLength() <= rStr.getLength() &&
                             sCurString.startsWith( bFoundEnglish ? sEnglishKeyword[nTmpType] : sKeyword[nTmpType]))
                        {
                            OUString aTest = pChrCls->uppercase( rStr.copy( nPos-1, sCurString.getLength() ) );
                            if ( aTest == sCurString )
                            {
                                bCurrency = true;
                            }
                        }
                        if ( bCurrency )
                        {
                            eState = SsGetWord;
                            sSymbolBuffer.append(OUStringLiteral1(cToken));
                        }
                        else
                        {
                            eType = nTmpType;
                            // The code to be advanced is the detected keyword,
                            // not necessarily the locale's keyword, but the
                            // symbol is to be the locale's keyword.
                            sal_Int32 nLen;
                            if (bFoundEnglish)
                            {
                                nLen = sEnglishKeyword[eType].getLength();
                                // Use the locale's General keyword name, not uppercase.
                                sSymbolBuffer = (eType == NF_KEY_GENERAL ? sNameStandardFormat : sKeyword[eType]);
                            }
                            else
                            {
                                nLen = sKeyword[eType].getLength();
                                // Preserve a locale's keyword's case as entered.
                                sSymbolBuffer = rStr.copy( nPos-1, nLen);
                            }
                            if ((eType == NF_KEY_E || IsAmbiguousE(eType)) && nPos < rStr.getLength())
                            {
                                sal_Unicode cNext = rStr[nPos];
                                switch ( cNext )
                                {
                                case '+' :
                                case '-' :  // E+ E- combine to one symbol
                                    sSymbolBuffer.append(OUStringLiteral1(cNext));
                                    eType = NF_KEY_E;
                                    nPos++;
                                    break;
                                case '0' :
                                case '#' :  // scientific E without sign
                                    eType = NF_KEY_E;
                                    break;
                                }
                            }
                            nPos--;
                            nPos = nPos + nLen;
                            eState = SsStop;
                        }
                    }
                    else
                    {
                        eState = SsGetWord;
                        sSymbolBuffer.append(OUStringLiteral1(cToken));
                    }
                }
                else
                {
                    eType = NF_SYMBOLTYPE_STRING;
                    eState = SsStop;
                    sSymbolBuffer.append(OUStringLiteral1(cToken));
                }
                break;
            }
            break;
        case SsGetChar:
            sSymbolBuffer.append(OUStringLiteral1(cToken));
            eState = SsStop;
            break;
        case SsGetString:
            if (cToken == '"')
            {
                eState = SsStop;
            }
            sSymbolBuffer.append(OUStringLiteral1(cToken));
            break;
        case SsGetWord:
            if ( pChrCls->isLetter( rStr, nPos-1 ) )
            {
                bool bFoundEnglish = false;
                short nTmpType = GetKeyWord( rStr, nPos-1, bFoundEnglish);
                if ( nTmpType )
                {
                    // beginning of keyword, stop scan and put back
                    eType = NF_SYMBOLTYPE_STRING;
                    eState = SsStop;
                    nPos--;
                }
                else
                {
                    sSymbolBuffer.append(OUStringLiteral1(cToken));
                }
            }
            else
            {
                bool bDontStop = false;
                sal_Unicode cNext;
                switch (cToken)
                {
                case '/': // AM/PM, A/P
                    if (nPos < rStr.getLength())
                    {
                        cNext = rStr[nPos];
                        if ( cNext == 'P' || cNext == 'p' )
                        {
                            sal_Int32 nLen = sSymbolBuffer.getLength();
                            if ( 1 <= nLen &&
                                    (sSymbolBuffer[0] == 'A' || sSymbolBuffer[0] == 'a') &&
                                    (nLen == 1 ||
                                     (nLen == 2 && (sSymbolBuffer[1] == 'M' || sSymbolBuffer[1] == 'm')
                                      && (rStr[nPos + 1] == 'M' || rStr[nPos + 1] == 'm'))))
                            {
                                sSymbolBuffer.append(OUStringLiteral1(cToken));
                                bDontStop = true;
                            }
                        }
                    }
                    break;
                }
                // anything not recognized will stop the scan
                if (!bDontStop)
                {
                    eState = SsStop;
                    nPos--;
                    eType = NF_SYMBOLTYPE_STRING;
                }
            }
            break;
        case SsGetStar:
            eState = SsStop;
            sSymbolBuffer.append(OUStringLiteral1(cToken));
            break;
        case SsGetBlank:
            eState = SsStop;
            sSymbolBuffer.append(OUStringLiteral1(cToken));
            break;
        default:
            break;
        } // of switch
    } // of while
    if (eState == SsGetWord)
    {
        eType = NF_SYMBOLTYPE_STRING;
    }
    sSymbol = sSymbolBuffer.makeStringAndClear();
    return eType;
}

sal_Int32 ImpSvNumberformatScan::Symbol_Division(const OUString& rString)
{
    nCurrPos = -1;
    // Do we have some sort of currency?
    OUString sString = pFormatter->GetCharClass()->uppercase(rString);
    sal_Int32 nCPos = 0;
    while (nCPos >= 0 && nCPos < sString.getLength())
    {
        nCPos = sString.indexOf(GetCurString(),nCPos);
        if (nCPos >= 0)
        {
            // In Quotes?
            sal_Int32 nQ = SvNumberformat::GetQuoteEnd( sString, nCPos );
            if ( nQ < 0 )
            {
                sal_Unicode c;
                if ( nCPos == 0 ||
                    ((c = sString[nCPos-1]) != '"'
                            && c != '\\') ) // dm can be protected by "dm \d
                {
                    nCurrPos = nCPos;
                    nCPos = -1;
                }
                else
                {
                    nCPos++; // Continue search
                }
            }
            else
            {
                nCPos = nQ + 1; // Continue search
            }
        }
    }
    nStringsCnt = 0;
    bool bStar = false; // Is set on detecting '*'
    Reset();

    sal_Int32 nPos = 0;
    const sal_Int32 nLen = rString.getLength();
    while (nPos < nLen && nStringsCnt < NF_MAX_FORMAT_SYMBOLS)
    {
        nTypeArray[nStringsCnt] = Next_Symbol(rString, nPos, sStrArray[nStringsCnt]);
        if (nTypeArray[nStringsCnt] == NF_SYMBOLTYPE_STAR)
        { // Monitoring the '*'
            if (bStar)
            {
                return nPos; // Error: double '*'
            }
            else
            {
                // Valid only if there is a character following, else we are
                // at the end of a code that does not have a fill character
                // (yet?).
                if (sStrArray[nStringsCnt].getLength() < 2)
                    return nPos;
                bStar = true;
            }
        }
        nStringsCnt++;
    }

    return 0; // 0 => ok
}

void ImpSvNumberformatScan::SkipStrings(sal_uInt16& i, sal_Int32& nPos) const
{
    while (i < nStringsCnt && (   nTypeArray[i] == NF_SYMBOLTYPE_STRING
                               || nTypeArray[i] == NF_SYMBOLTYPE_BLANK
                               || nTypeArray[i] == NF_SYMBOLTYPE_STAR) )
    {
        nPos = nPos + sStrArray[i].getLength();
        i++;
    }
}

sal_uInt16 ImpSvNumberformatScan::PreviousKeyword(sal_uInt16 i) const
{
    short res = 0;
    if (i > 0 && i < nStringsCnt)
    {
        i--;
        while (i > 0 && nTypeArray[i] <= 0)
        {
            i--;
        }
        if (nTypeArray[i] > 0)
        {
            res = nTypeArray[i];
        }
    }
    return res;
}

sal_uInt16 ImpSvNumberformatScan::NextKeyword(sal_uInt16 i) const
{
    short res = 0;
    if (i < nStringsCnt-1)
    {
        i++;
        while (i < nStringsCnt-1 && nTypeArray[i] <= 0)
        {
            i++;
        }
        if (nTypeArray[i] > 0)
        {
            res = nTypeArray[i];
        }
    }
    return res;
}

short ImpSvNumberformatScan::PreviousType( sal_uInt16 i ) const
{
    if ( i > 0 && i < nStringsCnt )
    {
        do
        {
            i--;
        }
        while ( i > 0 && nTypeArray[i] == NF_SYMBOLTYPE_EMPTY );
        return nTypeArray[i];
    }
    return 0;
}

sal_Unicode ImpSvNumberformatScan::PreviousChar(sal_uInt16 i) const
{
    sal_Unicode res = ' ';
    if (i > 0 && i < nStringsCnt)
    {
        i--;
        while (i > 0 &&
               ( nTypeArray[i] == NF_SYMBOLTYPE_EMPTY ||
                 nTypeArray[i] == NF_SYMBOLTYPE_STRING ||
                 nTypeArray[i] == NF_SYMBOLTYPE_STAR ||
                 nTypeArray[i] == NF_SYMBOLTYPE_BLANK ))
        {
            i--;
        }
        if (sStrArray[i].getLength() > 0)
        {
            res = sStrArray[i][sStrArray[i].getLength()-1];
        }
    }
    return res;
}

sal_Unicode ImpSvNumberformatScan::NextChar(sal_uInt16 i) const
{
    sal_Unicode res = ' ';
    if (i < nStringsCnt-1)
    {
        i++;
        while (i < nStringsCnt-1 &&
               ( nTypeArray[i] == NF_SYMBOLTYPE_EMPTY ||
                 nTypeArray[i] == NF_SYMBOLTYPE_STRING ||
                 nTypeArray[i] == NF_SYMBOLTYPE_STAR ||
                 nTypeArray[i] == NF_SYMBOLTYPE_BLANK))
        {
            i++;
        }
        if (sStrArray[i].getLength() > 0)
        {
            res = sStrArray[i][0];
        }
    }
    return res;
}

bool ImpSvNumberformatScan::IsLastBlankBeforeFrac(sal_uInt16 i) const
{
    bool res = true;
    if (i < nStringsCnt-1)
    {
        bool bStop = false;
        i++;
        while (i < nStringsCnt-1 && !bStop)
        {
            i++;
            if ( nTypeArray[i] == NF_SYMBOLTYPE_DEL &&
                 sStrArray[i][0] == '/')
            {
                bStop = true;
            }
            else if ( ( nTypeArray[i] == NF_SYMBOLTYPE_DEL  &&
                        sStrArray[i][0] == ' ')             ||
                        nTypeArray[i] == NF_SYMBOLTYPE_STRING ) // integer/fraction delimiter can also be a string
            {
                res = false;
            }
        }
        if (!bStop) // no '/'{
        {
            res = false;
        }
    }
    else
    {
        res = false; // no '/' any more
    }
    return res;
}

void ImpSvNumberformatScan::Reset()
{
    nStringsCnt = 0;
    nResultStringsCnt = 0;
    eScannedType = SvNumFormatType::UNDEFINED;
    bExp = false;
    bThousand = false;
    nThousand = 0;
    bDecSep = false;
    nDecPos = sal_uInt16(-1);
    nExpPos = sal_uInt16(-1);
    nBlankPos = sal_uInt16(-1);
    nCntPre = 0;
    nCntPost = 0;
    nCntExp = 0;
    bFrac = false;
    bBlank = false;
    nNatNumModifier = 0;
}

bool ImpSvNumberformatScan::Is100SecZero( sal_uInt16 i, bool bHadDecSep ) const
{
    sal_uInt16 nIndexPre = PreviousKeyword( i );
    return (nIndexPre == NF_KEY_S || nIndexPre == NF_KEY_SS) &&
            (bHadDecSep ||
             ( i > 0 && nTypeArray[i-1] == NF_SYMBOLTYPE_STRING));
              // SS"any"00  take "any" as a valid decimal separator
}

sal_Int32 ImpSvNumberformatScan::ScanType()
{
    const LocaleDataWrapper* pLoc = pFormatter->GetLocaleData();

    sal_Int32 nPos = 0;
    sal_uInt16 i = 0;
    SvNumFormatType eNewType;
    bool bMatchBracket = false;
    bool bHaveGeneral = false; // if General/Standard encountered
    bool bIsTimeDetected =false;   // hour or second found in format
    bool bHaveMinute = false;

    SkipStrings(i, nPos);
    while (i < nStringsCnt)
    {
        if (nTypeArray[i] > 0)
        {   // keyword
            sal_uInt16 nIndexPre;
            sal_uInt16 nIndexNex;

            switch (nTypeArray[i])
            {
            case NF_KEY_E:                          // E
                eNewType = SvNumFormatType::SCIENTIFIC;
                break;
            case NF_KEY_H:                          // H
            case NF_KEY_HH:                         // HH
                bIsTimeDetected = true;
                [[fallthrough]];
            case NF_KEY_S:                          // S
            case NF_KEY_SS:                         // SS
                if ( !bHaveMinute )
                    bIsTimeDetected = true;
                [[fallthrough]];
            case NF_KEY_AMPM:                       // AM,A,PM,P
            case NF_KEY_AP:
                eNewType = SvNumFormatType::TIME;
                break;
            case NF_KEY_M:                          // M
            case NF_KEY_MM:                         // MM
            case NF_KEY_MI:                         // M  minute detected in Finnish
            case NF_KEY_MMI:                        // MM
                /* Minute or month.
                   Minute if one of:
                   * preceded by time keyword H (ignoring separators)
                   * followed by time keyword S (ignoring separators)
                   * H or S was detected and this is the first M following
                   * preceded by '[' amount bracket
                   Else month.
                   That are the Excel rules. BUT, we break it because certainly
                   in something like {HH YYYY-MM-DD} the MM is NOT meant to be
                   minute, so not if MM is between YY and DD or DD and YY.
                   Actually not if any date specific keyword followed a time
                   setting keyword.
                */
                nIndexPre = PreviousKeyword(i);
                nIndexNex = NextKeyword(i);
                if (nIndexPre == NF_KEY_H   ||      // H
                    nIndexPre == NF_KEY_HH  ||      // HH
                    nIndexNex == NF_KEY_S   ||      // S
                    nIndexNex == NF_KEY_SS  ||      // SS
                    bIsTimeDetected         ||      // tdf#101147
                    PreviousChar(i) == '['  )       // [M
                {
                    eNewType = SvNumFormatType::TIME;
                    if ( nTypeArray[i] == NF_KEY_M || nTypeArray[i] == NF_KEY_MM )
                    {
                        nTypeArray[i] -= 2;             // 6 -> 4, 7 -> 5
                    }
                    bIsTimeDetected = false;        // next M should be month
                    bHaveMinute = true;
                }
                else
                {
                    eNewType = SvNumFormatType::DATE;
                    if ( nTypeArray[i] == NF_KEY_MI || nTypeArray[i] == NF_KEY_MMI )
                    {   // follow resolution of tdf#33689 for Finnish
                        nTypeArray[i] += 2;             // 4 -> 6, 5 -> 7
                    }
                }
                break;
            case NF_KEY_MMM:                        // MMM
            case NF_KEY_MMMM:                       // MMMM
            case NF_KEY_MMMMM:                      // MMMMM
            case NF_KEY_Q:                          // Q
            case NF_KEY_QQ:                         // QQ
            case NF_KEY_D:                          // D
            case NF_KEY_DD:                         // DD
            case NF_KEY_DDD:                        // DDD
            case NF_KEY_DDDD:                       // DDDD
            case NF_KEY_YY:                         // YY
            case NF_KEY_YYYY:                       // YYYY
            case NF_KEY_NN:                         // NN
            case NF_KEY_NNN:                        // NNN
            case NF_KEY_NNNN:                       // NNNN
            case NF_KEY_WW :                        // WW
            case NF_KEY_AAA :                       // AAA
            case NF_KEY_AAAA :                      // AAAA
            case NF_KEY_EC :                        // E
            case NF_KEY_EEC :                       // EE
            case NF_KEY_G :                         // G
            case NF_KEY_GG :                        // GG
            case NF_KEY_GGG :                       // GGG
            case NF_KEY_R :                         // R
            case NF_KEY_RR :                        // RR
                eNewType = SvNumFormatType::DATE;
                bIsTimeDetected = false;
                break;
            case NF_KEY_CCC:                        // CCC
                eNewType = SvNumFormatType::CURRENCY;
                break;
            case NF_KEY_GENERAL:                    // Standard
                eNewType = SvNumFormatType::NUMBER;
                bHaveGeneral = true;
                break;
            default:
                eNewType = SvNumFormatType::UNDEFINED;
                break;
            }
        }
        else
        {                                           // control character
            switch ( sStrArray[i][0] )
            {
            case '#':
            case '?':
                eNewType = SvNumFormatType::NUMBER;
                break;
            case '0':
                if ( eScannedType & SvNumFormatType::TIME )
                {
                    if ( Is100SecZero( i, bDecSep ) )
                    {
                        bDecSep = true;                 // subsequent 0's
                        eNewType = SvNumFormatType::TIME;
                    }
                    else
                    {
                        return nPos;                    // Error
                    }
                }
                else
                {
                    eNewType = SvNumFormatType::NUMBER;
                }
                break;
            case '%':
                eNewType = SvNumFormatType::PERCENT;
                break;
            case '/':
                eNewType = SvNumFormatType::FRACTION;
                break;
            case '[':
                if ( i < nStringsCnt-1 &&
                     nTypeArray[i+1] == NF_SYMBOLTYPE_STRING &&
                     sStrArray[i+1][0] == '$' )
                {
                    eNewType = SvNumFormatType::CURRENCY;
                    bMatchBracket = true;
                }
                else if ( i < nStringsCnt-1 &&
                          nTypeArray[i+1] == NF_SYMBOLTYPE_STRING &&
                          sStrArray[i+1][0] == '~' )
                {
                    eNewType = SvNumFormatType::DATE;
                    bMatchBracket = true;
                }
                else
                {
                    sal_uInt16 nIndexNex = NextKeyword(i);
                    if (nIndexNex == NF_KEY_H   ||  // H
                        nIndexNex == NF_KEY_HH  ||  // HH
                        nIndexNex == NF_KEY_M   ||  // M
                        nIndexNex == NF_KEY_MM  ||  // MM
                        nIndexNex == NF_KEY_S   ||  // S
                        nIndexNex == NF_KEY_SS   )  // SS
                        eNewType = SvNumFormatType::TIME;
                    else
                    {
                        return nPos;                // Error
                    }
                }
                break;
            case '@':
                eNewType = SvNumFormatType::TEXT;
                break;
            default:
                if (pLoc->getTime100SecSep() == sStrArray[i])
                {
                    bDecSep = true;                  // for SS,0
                }
                eNewType = SvNumFormatType::UNDEFINED;
                break;
            }
        }
        if (eScannedType == SvNumFormatType::UNDEFINED)
        {
            eScannedType = eNewType;
        }
        else if (eScannedType == SvNumFormatType::TEXT || eNewType == SvNumFormatType::TEXT)
        {
            eScannedType = SvNumFormatType::TEXT; // Text always remains text
        }
        else if (eNewType == SvNumFormatType::UNDEFINED)
        { // Remains as is
        }
        else if (eScannedType != eNewType)
        {
            switch (eScannedType)
            {
            case SvNumFormatType::DATE:
                switch (eNewType)
                {
                case SvNumFormatType::TIME:
                    eScannedType = SvNumFormatType::DATETIME;
                    break;
                case SvNumFormatType::FRACTION:         // DD/MM
                    break;
                default:
                    if (nCurrPos >= 0)
                    {
                        eScannedType = SvNumFormatType::UNDEFINED;
                    }
                    else if ( sStrArray[i] != pFormatter->GetDateSep() )
                    {
                        return nPos;
                    }
                }
                break;
            case SvNumFormatType::TIME:
                switch (eNewType)
                {
                case SvNumFormatType::DATE:
                    eScannedType = SvNumFormatType::DATETIME;
                    break;
                case SvNumFormatType::FRACTION:         // MM/SS
                    break;
                default:
                    if (nCurrPos >= 0)
                    {
                        eScannedType = SvNumFormatType::UNDEFINED;
                    }
                    else if (pLoc->getTimeSep() != sStrArray[i])
                    {
                        return nPos;
                    }
                    break;
                }
                break;
            case SvNumFormatType::DATETIME:
                switch (eNewType)
                {
                case SvNumFormatType::TIME:
                case SvNumFormatType::DATE:
                    break;
                case SvNumFormatType::FRACTION:         // DD/MM
                    break;
                default:
                    if (nCurrPos >= 0)
                    {
                        eScannedType = SvNumFormatType::UNDEFINED;
                    }
                    else if ( pFormatter->GetDateSep() != sStrArray[i] &&
                              pLoc->getTimeSep() != sStrArray[i] )
                    {
                        return nPos;
                    }
                }
                break;
            case SvNumFormatType::PERCENT:
                switch (eNewType)
                {
                case SvNumFormatType::NUMBER:   // Only number to percent
                    break;
                default:
                    return nPos;
                }
                break;
            case SvNumFormatType::SCIENTIFIC:
                switch (eNewType)
                {
                case SvNumFormatType::NUMBER:   // Only number to E
                    break;
                default:
                    return nPos;
                }
                break;
            case SvNumFormatType::NUMBER:
                switch (eNewType)
                {
                case SvNumFormatType::SCIENTIFIC:
                case SvNumFormatType::PERCENT:
                case SvNumFormatType::FRACTION:
                case SvNumFormatType::CURRENCY:
                    eScannedType = eNewType;
                    break;
                default:
                    if (nCurrPos >= 0)
                    {
                        eScannedType = SvNumFormatType::UNDEFINED;
                    }
                    else
                    {
                        return nPos;
                    }
                }
                break;
            case SvNumFormatType::FRACTION:
                switch (eNewType)
                {
                case SvNumFormatType::NUMBER:   // Only number to fraction
                    break;
                default:
                    return nPos;
                }
                break;
            default:
                break;
            }
        }
        nPos = nPos + sStrArray[i].getLength(); // Position of correction
        i++;
        if ( bMatchBracket )
        {   // no type detection inside of matching brackets if [$...], [~...]
            while ( bMatchBracket && i < nStringsCnt )
            {
                if ( nTypeArray[i] == NF_SYMBOLTYPE_DEL
                     && sStrArray[i][0] == ']' )
                {
                    bMatchBracket = false;
                }
                else
                {
                    nTypeArray[i] = NF_SYMBOLTYPE_STRING;
                }
                nPos = nPos + sStrArray[i].getLength();
                i++;
            }
            if ( bMatchBracket )
            {
                return nPos; // missing closing bracket at end of code
            }
        }
        SkipStrings(i, nPos);
    }

    if ((eScannedType == SvNumFormatType::NUMBER ||
         eScannedType == SvNumFormatType::UNDEFINED) &&
        nCurrPos >= 0 && !bHaveGeneral)
    {
        eScannedType = SvNumFormatType::CURRENCY; // old "automatic" currency
    }
    if (eScannedType == SvNumFormatType::UNDEFINED)
    {
        eScannedType = SvNumFormatType::DEFINED;
    }
    return 0; // All is fine
}

bool ImpSvNumberformatScan::InsertSymbol( sal_uInt16 & nPos, svt::NfSymbolType eType, const OUString& rStr )
{
    if (nStringsCnt >= NF_MAX_FORMAT_SYMBOLS || nPos > nStringsCnt)
    {
        return false;
    }
    if (nPos > 0 && nTypeArray[nPos-1] == NF_SYMBOLTYPE_EMPTY)
    {
        --nPos; // reuse position
    }
    else
    {
        if (static_cast<size_t>(nStringsCnt + 1) >= NF_MAX_FORMAT_SYMBOLS)
        {
            return false;
        }
        ++nStringsCnt;
        for (size_t i = nStringsCnt; i > nPos; --i)
        {
            nTypeArray[i] = nTypeArray[i-1];
            sStrArray[i] = sStrArray[i-1];
        }
    }
    ++nResultStringsCnt;
    nTypeArray[nPos] = static_cast<short>(eType);
    sStrArray[nPos] = rStr;
    return true;
}

int ImpSvNumberformatScan::FinalScanGetCalendar( sal_Int32& nPos, sal_uInt16& i,
                                                 sal_uInt16& rResultStringsCnt )
{
    if ( i < nStringsCnt-1 &&
         sStrArray[i][0] == '[' &&
         nTypeArray[i+1] == NF_SYMBOLTYPE_STRING &&
         sStrArray[i+1][0] == '~' )
    {
        // [~calendarID]
        nPos = nPos + sStrArray[i].getLength();           // [
        nTypeArray[i] = NF_SYMBOLTYPE_CALDEL;
        nPos = nPos + sStrArray[++i].getLength();         // ~
        sStrArray[i-1] += sStrArray[i];                   // [~
        nTypeArray[i] = NF_SYMBOLTYPE_EMPTY;
        rResultStringsCnt--;
        if ( ++i >= nStringsCnt )
        {
            return -1; // error
        }
        nPos = nPos + sStrArray[i].getLength();           // calendarID
        OUString& rStr = sStrArray[i];
        nTypeArray[i] = NF_SYMBOLTYPE_CALENDAR;          // convert
        i++;
        while ( i < nStringsCnt && sStrArray[i][0] != ']' )
        {
            nPos = nPos + sStrArray[i].getLength();
            rStr += sStrArray[i];
            nTypeArray[i] = NF_SYMBOLTYPE_EMPTY;
            rResultStringsCnt--;
            i++;
        }
        if ( rStr.getLength() && i < nStringsCnt &&
             sStrArray[i][0] == ']' )
        {
            nTypeArray[i] = NF_SYMBOLTYPE_CALDEL;
            nPos = nPos + sStrArray[i].getLength();
            i++;
        }
        else
        {
            return -1; // error
        }
        return 1;
    }
    return 0;
}

bool ImpSvNumberformatScan::IsDateFragment( size_t nPos1, size_t nPos2 ) const
{
    return nPos2 - nPos1 == 2 && nTypeArray[nPos1+1] == NF_SYMBOLTYPE_DATESEP;
}

void ImpSvNumberformatScan::SwapArrayElements( size_t nPos1, size_t nPos2 )
{
    std::swap( nTypeArray[nPos1], nTypeArray[nPos2]);
    std::swap( sStrArray[nPos1], sStrArray[nPos2]);
}

sal_Int32 ImpSvNumberformatScan::FinalScan( OUString& rString )
{
    const LocaleDataWrapper* pLoc = pFormatter->GetLocaleData();

    // save values for convert mode
    OUString sOldDecSep       = pFormatter->GetNumDecimalSep();
    OUString sOldThousandSep  = pFormatter->GetNumThousandSep();
    OUString sOldDateSep      = pFormatter->GetDateSep();
    OUString sOldTimeSep      = pLoc->getTimeSep();
    OUString sOldTime100SecSep= pLoc->getTime100SecSep();
    OUString sOldCurSymbol    = GetCurSymbol();
    OUString sOldCurString = GetCurString();
    sal_Unicode cOldKeyH    = sKeyword[NF_KEY_H][0];
    sal_Unicode cOldKeyMI   = sKeyword[NF_KEY_MI][0];
    sal_Unicode cOldKeyS    = sKeyword[NF_KEY_S][0];
    DateOrder eOldDateOrder = pLoc->getDateOrder();
    sal_uInt16 nDayPos, nMonthPos, nYearPos;
    nDayPos = nMonthPos = nYearPos = SAL_MAX_UINT16;

    // If the group separator is a No-Break Space (French) continue with a
    // normal space instead so queries on space work correctly.
    // The same for Narrow No-Break Space just in case some locale uses it.
    // The format string is adjusted to allow both.
    // For output of the format code string the LocaleData characters are used.
    if ( (sOldThousandSep[0] == cNoBreakSpace || sOldThousandSep[0] == cNarrowNoBreakSpace) &&
            sOldThousandSep.getLength() == 1 )
    {
        sOldThousandSep = " ";
    }
    bool bNewDateOrder = false;
    // change locale data et al
    if (bConvertMode)
    {
        pFormatter->ChangeIntl(eNewLnge);
        //! pointer may have changed
        pLoc = pFormatter->GetLocaleData();
        //! init new keywords
        InitKeywords();
        // Adapt date order to target locale, but Excel does not handle date
        // particle re-ordering for the target locale when loading documents,
        // though it does exchange separators, tdf#113889
        bNewDateOrder = (mbConvertDateOrder && eOldDateOrder != pLoc->getDateOrder());
    }
    const CharClass* pChrCls = pFormatter->GetCharClass();

    sal_Int32 nPos = 0;                    // error correction position
    sal_uInt16 i = 0;                      // symbol loop counter
    sal_uInt16 nCounter = 0;               // counts digits
    nResultStringsCnt = nStringsCnt;       // counts remaining symbols
    bDecSep = false;                       // reset in case already used in TypeCheck
    bool bThaiT = false;                   // Thai T NatNum modifier present
    bool bTimePart = false;
    bool bDenomin = false;                 // Set when reading end of denominator

    switch (eScannedType)
    {
    case SvNumFormatType::TEXT:
    case SvNumFormatType::DEFINED:
        while (i < nStringsCnt)
        {
            switch (nTypeArray[i])
            {
            case NF_SYMBOLTYPE_BLANK:
            case NF_SYMBOLTYPE_STAR:
                break;
            case NF_KEY_GENERAL : // #77026# "General" is the same as "@"
                break;
            default:
                if ( nTypeArray[i] != NF_SYMBOLTYPE_DEL ||
                     sStrArray[i][0] != '@' )
                {
                    nTypeArray[i] = NF_SYMBOLTYPE_STRING;
                }
                break;
            }
            nPos = nPos + sStrArray[i].getLength();
            i++;
        } // of while
        break;

    case SvNumFormatType::NUMBER:
    case SvNumFormatType::PERCENT:
    case SvNumFormatType::CURRENCY:
    case SvNumFormatType::SCIENTIFIC:
    case SvNumFormatType::FRACTION:
        while (i < nStringsCnt)
        {
            // TODO: rechecking eScannedType is unnecessary.
            // This switch-case is for eScannedType == SvNumFormatType::FRACTION anyway
            if (eScannedType == SvNumFormatType::FRACTION &&        // special case
                nTypeArray[i] == NF_SYMBOLTYPE_DEL &&           // # ### #/#
                StringEqualsChar( sOldThousandSep, ' ' ) &&     // e.g. France or Sweden
                StringEqualsChar( sStrArray[i], ' ' ) &&
                !bFrac                          &&
                IsLastBlankBeforeFrac(i) )
            {
                nTypeArray[i] = NF_SYMBOLTYPE_STRING;           // del->string
            }                                                   // No thousands marker

            if (nTypeArray[i] == NF_SYMBOLTYPE_BLANK    ||
                nTypeArray[i] == NF_SYMBOLTYPE_STAR ||
                nTypeArray[i] == NF_KEY_CCC         ||          // CCC
                nTypeArray[i] == NF_KEY_GENERAL )               // Standard
            {
                if (nTypeArray[i] == NF_KEY_GENERAL)
                {
                    nThousand = FLAG_STANDARD_IN_FORMAT;
                    if ( bConvertMode )
                    {
                        sStrArray[i] = sNameStandardFormat;
                    }
                }
                nPos = nPos + sStrArray[i].getLength();
                i++;
            }
            else if (nTypeArray[i] == NF_SYMBOLTYPE_STRING ||   // No Strings or
                     nTypeArray[i] > 0)                         // Keywords
            {
                if (eScannedType == SvNumFormatType::SCIENTIFIC &&
                    nTypeArray[i] == NF_KEY_E)                  // E+
                {
                    if (bExp)                                   // Double
                    {
                        return nPos;
                    }
                    bExp = true;
                    nExpPos = i;
                    if (bDecSep)
                    {
                        nCntPost = nCounter;
                    }
                    else
                    {
                        nCntPre = nCounter;
                    }
                    nCounter = 0;
                    nTypeArray[i] = NF_SYMBOLTYPE_EXP;
                }
                else if (eScannedType == SvNumFormatType::FRACTION &&
                    (sStrArray[i][0] == ' ' || ( nTypeArray[i] == NF_SYMBOLTYPE_STRING && (sStrArray[i][0] < '0' || sStrArray[i][0] > '9') ) ) )
                {
                    if (!bBlank && !bFrac) // Not double or after a /
                    {
                        if (bDecSep && nCounter > 0) // Decimal places
                        {
                            return nPos; // Error
                        }
                        if (sStrArray[i][0] == ' ' ||  nCounter > 0 )   // treat string as integer/fraction delimiter only if there is integer
                        {
                            bBlank = true;
                            nBlankPos = i;
                            nCntPre = nCounter;
                            nCounter = 0;
                            nTypeArray[i] = NF_SYMBOLTYPE_FRACBLANK;
                        }
                    }
                    else if ( sStrArray[i][0] == ' ' )
                        nTypeArray[i] = NF_SYMBOLTYPE_FRACBLANK;
                    else if ( bFrac && ( nCounter > 0 ) )
                        bDenomin = true; // following elements are no more part of denominator
                }
                else if (nTypeArray[i] == NF_KEY_THAI_T)
                {
                    bThaiT = true;
                    sStrArray[i] = sKeyword[nTypeArray[i]];
                }
                else if (sStrArray[i][0] >= '0' &&
                         sStrArray[i][0] <= '9' && !bDenomin) // denominator was not yet found
                {
                    OUString sDiv;
                    sal_uInt16 j = i;
                    while(j < nStringsCnt && sStrArray[j][0] >= '0' && sStrArray[j][0] <= '9')
                    {
                        sDiv += sStrArray[j++];
                    }
                    assert(j > 0 && "if i is 0, first iteration through loop is guaranteed by surrounding if condition");
                    if (OUString::number(sDiv.toInt32()) == sDiv)
                    {
                        // Found a Divisor
                        while (i < j)
                        {
                            nTypeArray[i++] = NF_SYMBOLTYPE_FRAC_FDIV;
                        }
                        i = j - 1; // Stop the loop
                        if (nCntPost)
                        {
                            nCounter = nCntPost;
                        }
                        else if (nCntPre)
                        {
                            nCounter = nCntPre;
                        }
                        // don't artificially increment nCntPre for forced denominator
                        if ( ( eScannedType != SvNumFormatType::FRACTION ) && (!nCntPre) )
                        {
                            nCntPre++;
                        }
                        if ( bFrac )
                            bDenomin = true; // next content should be treated as outside denominator
                    }
                }
                else
                {
                    if ( bFrac && ( nCounter > 0 ) )
                        bDenomin = true;    // next content should be treated as outside denominator
                    nTypeArray[i] = NF_SYMBOLTYPE_STRING;
                }
                nPos = nPos + sStrArray[i].getLength();
                i++;
            }
            else if (nTypeArray[i] == NF_SYMBOLTYPE_DEL)
            {
                sal_Unicode cHere = sStrArray[i][0];
                sal_Unicode cSaved = cHere;
                // Handle not pre-known separators in switch.
                sal_Unicode cSimplified;
                if (StringEqualsChar( pFormatter->GetNumThousandSep(), cHere))
                {
                    cSimplified = ',';
                }
                else if (StringEqualsChar( pFormatter->GetNumDecimalSep(), cHere))
                {
                    cSimplified = '.';
                }
                else
                {
                    cSimplified = cHere;
                }

                OUString& rStr = sStrArray[i];

                switch ( cSimplified )
                {
                case '#':
                case '0':
                case '?':
                    if (nThousand > 0)                  // #... #
                    {
                        return nPos;                    // Error
                    }
                    if ( !bDenomin )
                    {
                        nTypeArray[i] = NF_SYMBOLTYPE_DIGIT;
                        nPos = nPos + rStr.getLength();
                        i++;
                        nCounter++;
                        while (i < nStringsCnt &&
                              (sStrArray[i][0] == '#' ||
                               sStrArray[i][0] == '0' ||
                               sStrArray[i][0] == '?'))
                        {
                            nTypeArray[i] = NF_SYMBOLTYPE_DIGIT;
                            nPos = nPos + sStrArray[i].getLength();
                            nCounter++;
                            i++;
                        }
                    }
                    else // after denominator, treat any character as text
                    {
                        nTypeArray[i] = NF_SYMBOLTYPE_STRING;
                        nPos = nPos + sStrArray[i].getLength();
                    }
                    break;
                case '-':
                    if ( bDecSep && nDecPos+1 == i &&
                         nTypeArray[nDecPos] == NF_SYMBOLTYPE_DECSEP )
                    {
                        // "0.--"
                        nTypeArray[i] = NF_SYMBOLTYPE_DIGIT;
                        nPos = nPos + rStr.getLength();
                        i++;
                        nCounter++;
                        while (i < nStringsCnt &&
                               (sStrArray[i][0] == '-') )
                        {
                            // If more than two dashes are present in
                            // currency formats the last dash will be
                            // interpreted literally as a minus sign.
                            // Has to be this ugly. Period.
                            if ( eScannedType == SvNumFormatType::CURRENCY
                                 && rStr.getLength() >= 2 &&
                                 (i == nStringsCnt-1 ||
                                  sStrArray[i+1][0] != '-') )
                            {
                                break;
                            }
                            rStr += sStrArray[i];
                            nPos = nPos + sStrArray[i].getLength();
                            nTypeArray[i] = NF_SYMBOLTYPE_EMPTY;
                            nResultStringsCnt--;
                            nCounter++;
                            i++;
                        }
                    }
                    else
                    {
                        nTypeArray[i] = NF_SYMBOLTYPE_STRING;
                        nPos = nPos + sStrArray[i].getLength();
                        i++;
                    }
                    break;
                case '.':
                case ',':
                case '\'':
                case ' ':
                    if ( StringEqualsChar( sOldThousandSep, cSaved ) )
                    {
                        // previous char with skip empty
                        sal_Unicode cPre = PreviousChar(i);
                        sal_Unicode cNext;
                        if (bExp || bBlank || bFrac)
                        {
                            // after E, / or ' '
                            if ( !StringEqualsChar( sOldThousandSep, ' ' ) )
                            {
                                nPos = nPos + sStrArray[i].getLength();
                                nTypeArray[i] = NF_SYMBOLTYPE_EMPTY;
                                nResultStringsCnt--;
                                i++; // eat it
                            }
                            else
                            {
                                nTypeArray[i] = NF_SYMBOLTYPE_STRING;
                                if ( bFrac && (nCounter > 0) )
                                    bDenomin = true; // end of denominator
                            }
                        }
                        else if (i > 0 && i < nStringsCnt-1   &&
                                 (cPre == '#' || cPre == '0' || cPre == '?')      &&
                                 ((cNext = NextChar(i)) == '#' || cNext == '0' || cNext == '?')) // #,#
                        {
                            nPos = nPos + sStrArray[i].getLength();
                            if (!bThousand) // only once
                            {
                                bThousand = true;
                            }
                            // Eat it, will be reinserted at proper grouping positions further down.
                            nTypeArray[i] = NF_SYMBOLTYPE_EMPTY;
                            nResultStringsCnt--;
                            i++;
                        }
                        else if (i > 0 && (cPre == '#' || cPre == '0' || cPre == '?')
                                 && PreviousType(i) == NF_SYMBOLTYPE_DIGIT
                                 && nThousand < FLAG_STANDARD_IN_FORMAT )
                        {   // #,,,,
                            if ( StringEqualsChar( sOldThousandSep, ' ' ) )
                            {
                                // strange, those French..
                                bool bFirst = true;
                                //  set a hard No-Break Space or ConvertMode
                                const OUString& rSepF = pFormatter->GetNumThousandSep();
                                while ( i < nStringsCnt &&
                                        sStrArray[i] == sOldThousandSep &&
                                        StringEqualsChar( sOldThousandSep, NextChar(i) ) )
                                {   // last was a space or another space
                                    // is following => separator
                                    nPos = nPos + sStrArray[i].getLength();
                                    if ( bFirst )
                                    {
                                        bFirst = false;
                                        rStr = rSepF;
                                        nTypeArray[i] = NF_SYMBOLTYPE_THSEP;
                                    }
                                    else
                                    {
                                        rStr += rSepF;
                                        nTypeArray[i] = NF_SYMBOLTYPE_EMPTY;
                                        nResultStringsCnt--;
                                    }
                                    nThousand++;
                                    i++;
                                }
                                if ( i < nStringsCnt-1 &&
                                     sStrArray[i] == sOldThousandSep )
                                {
                                    // something following last space
                                    // => space if currency contained,
                                    // else separator
                                    nPos = nPos + sStrArray[i].getLength();
                                    if ( (nPos <= nCurrPos &&
                                          nCurrPos < nPos + sStrArray[i+1].getLength()) ||
                                         nTypeArray[i+1] == NF_KEY_CCC ||
                                         (i < nStringsCnt-2 &&
                                          sStrArray[i+1][0] == '[' &&
                                          sStrArray[i+2][0] == '$') )
                                    {
                                        nTypeArray[i] = NF_SYMBOLTYPE_STRING;
                                    }
                                    else
                                    {
                                        if ( bFirst )
                                        {
                                            rStr = rSepF;
                                            nTypeArray[i] = NF_SYMBOLTYPE_THSEP;
                                        }
                                        else
                                        {
                                            rStr += rSepF;
                                            nTypeArray[i] = NF_SYMBOLTYPE_EMPTY;
                                            nResultStringsCnt--;
                                        }
                                        nThousand++;
                                    }
                                    i++;
                                }
                            }
                            else
                            {
                                do
                                {
                                    nThousand++;
                                    nTypeArray[i] = NF_SYMBOLTYPE_THSEP;
                                    nPos = nPos + sStrArray[i].getLength();
                                    sStrArray[i] = pFormatter->GetNumThousandSep();
                                    i++;
                                }
                                while (i < nStringsCnt && sStrArray[i] == sOldThousandSep);
                            }
                        }
                        else // any grsep
                        {
                            nTypeArray[i] = NF_SYMBOLTYPE_STRING;
                            nPos = nPos + rStr.getLength();
                            i++;
                            while ( i < nStringsCnt && sStrArray[i] == sOldThousandSep )
                            {
                                rStr += sStrArray[i];
                                nPos = nPos + sStrArray[i].getLength();
                                nTypeArray[i] = NF_SYMBOLTYPE_EMPTY;
                                nResultStringsCnt--;
                                i++;
                            }
                        }
                    }
                    else if ( StringEqualsChar( sOldDecSep, cSaved ) )
                    {
                        if (bBlank || bFrac)    // . behind / or ' '
                        {
                            return nPos;        // error
                        }
                        else if (bExp)          // behind E
                        {
                            nPos = nPos + sStrArray[i].getLength();
                            nTypeArray[i] = NF_SYMBOLTYPE_EMPTY;
                            nResultStringsCnt--;
                            i++;                // eat it
                        }
                        else if (bDecSep)       // any .
                        {
                            nTypeArray[i] = NF_SYMBOLTYPE_STRING;
                            nPos = nPos + rStr.getLength();
                            i++;
                            while ( i < nStringsCnt && sStrArray[i] == sOldDecSep )
                            {
                                rStr += sStrArray[i];
                                nPos = nPos + sStrArray[i].getLength();
                                nTypeArray[i] = NF_SYMBOLTYPE_EMPTY;
                                nResultStringsCnt--;
                                i++;
                            }
                        }
                        else
                        {
                            nPos = nPos + sStrArray[i].getLength();
                            nTypeArray[i] = NF_SYMBOLTYPE_DECSEP;
                            sStrArray[i] = pFormatter->GetNumDecimalSep();
                            bDecSep = true;
                            nDecPos = i;
                            nCntPre = nCounter;
                            nCounter = 0;

                            i++;
                        }
                    } // of else = DecSep
                    else // . without meaning
                    {
                        if (cSaved == ' ' &&
                            eScannedType == SvNumFormatType::FRACTION &&
                            StringEqualsChar( sStrArray[i], ' ' ) )
                        {
                            if (!bBlank && !bFrac)  // no dups
                            {                       // or behind /
                                if (bDecSep && nCounter > 0) // dec.
                                {
                                    return nPos; // error
                                }
                                bBlank = true;
                                nBlankPos = i;
                                nCntPre = nCounter;
                                nCounter = 0;
                            }
                            if ( bFrac && (nCounter > 0) )
                                bDenomin = true; // next content is not part of denominator
                            nTypeArray[i] = NF_SYMBOLTYPE_STRING;
                            nPos = nPos + sStrArray[i].getLength();
                        }
                        else
                        {
                            nTypeArray[i] = NF_SYMBOLTYPE_STRING;
                            if ( bFrac && (nCounter > 0) )
                                bDenomin = true; // next content is not part of denominator
                            nPos = nPos + rStr.getLength();
                            i++;
                            while (i < nStringsCnt && StringEqualsChar( sStrArray[i], cSaved ) )
                            {
                                rStr += sStrArray[i];
                                nPos = nPos + sStrArray[i].getLength();
                                nTypeArray[i] = NF_SYMBOLTYPE_EMPTY;
                                nResultStringsCnt--;
                                i++;
                            }
                        }
                    }
                    break;
                case '/':
                    if (eScannedType == SvNumFormatType::FRACTION)
                    {
                        if ( i == 0 ||
                             (nTypeArray[i-1] != NF_SYMBOLTYPE_DIGIT &&
                              nTypeArray[i-1] != NF_SYMBOLTYPE_EMPTY) )
                        {
                            return nPos ? nPos : 1; // /? not allowed
                        }
                        else if (!bFrac || (bDecSep && nCounter > 0))
                        {
                            bFrac = true;
                            nCntPost = nCounter;
                            nCounter = 0;
                            nTypeArray[i] = NF_SYMBOLTYPE_FRAC;
                            nPos = nPos + sStrArray[i].getLength();
                            i++;
                        }
                        else // / double or in , in the denominator
                        {
                            return nPos; // Error
                        }
                    }
                    else
                    {
                        nTypeArray[i] = NF_SYMBOLTYPE_STRING;
                        nPos = nPos + sStrArray[i].getLength();
                        i++;
                    }
                    break;
                case '[' :
                    if ( eScannedType == SvNumFormatType::CURRENCY &&
                         i < nStringsCnt-1 &&
                         nTypeArray[i+1] == NF_SYMBOLTYPE_STRING &&
                         sStrArray[i+1][0] == '$' )
                    {
                        // [$DM-xxx]
                        nPos = nPos + sStrArray[i].getLength();     // [
                        nTypeArray[i] = NF_SYMBOLTYPE_CURRDEL;
                        nPos = nPos + sStrArray[++i].getLength();   // $
                        sStrArray[i-1] += sStrArray[i];             // [$
                        nTypeArray[i] = NF_SYMBOLTYPE_EMPTY;
                        nResultStringsCnt--;
                        if ( ++i >= nStringsCnt )
                        {
                            return nPos; // Error
                        }
                        nPos = nPos + sStrArray[i].getLength();     // DM
                        OUString* pStr = &sStrArray[i];
                        nTypeArray[i] = NF_SYMBOLTYPE_CURRENCY; // convert
                        bool bHadDash = false;
                        i++;
                        while ( i < nStringsCnt && sStrArray[i][0] != ']' )
                        {
                            nPos = nPos + sStrArray[i].getLength();
                            if ( bHadDash )
                            {
                                *pStr += sStrArray[i];
                                nTypeArray[i] = NF_SYMBOLTYPE_EMPTY;
                                nResultStringsCnt--;
                            }
                            else
                            {
                                if ( sStrArray[i][0] == '-' )
                                {
                                    bHadDash = true;
                                    pStr = &sStrArray[i];
                                    nTypeArray[i] = NF_SYMBOLTYPE_CURREXT;
                                }
                                else
                                {
                                    *pStr += sStrArray[i];
                                    nTypeArray[i] = NF_SYMBOLTYPE_EMPTY;
                                    nResultStringsCnt--;
                                }
                            }
                            i++;
                        }
                        if ( rStr.getLength() && i < nStringsCnt && sStrArray[i][0] == ']' )
                        {
                            nTypeArray[i] = NF_SYMBOLTYPE_CURRDEL;
                            nPos = nPos + sStrArray[i].getLength();
                            i++;
                        }
                        else
                        {
                            return nPos; // Error
                        }
                    }
                    else
                    {
                        nTypeArray[i] = NF_SYMBOLTYPE_STRING;
                        nPos = nPos + sStrArray[i].getLength();
                        i++;
                    }
                    break;
                default: // Other Dels
                    if (eScannedType == SvNumFormatType::PERCENT && cHere == '%')
                    {
                        nTypeArray[i] = NF_SYMBOLTYPE_PERCENT;
                    }
                    else
                    {
                        nTypeArray[i] = NF_SYMBOLTYPE_STRING;
                    }
                    nPos = nPos + sStrArray[i].getLength();
                    i++;
                    break;
                } // of switch (Del)
            } // of else Del
            else
            {
                SAL_WARN( "svl.numbers", "unknown NF_SYMBOLTYPE_..." );
                nPos = nPos + sStrArray[i].getLength();
                i++;
            }
        } // of while
        if (eScannedType == SvNumFormatType::FRACTION)
        {
            if (bFrac)
            {
                nCntExp = nCounter;
            }
            else if (bBlank)
            {
                nCntPost = nCounter;
            }
            else
            {
                nCntPre = nCounter;
            }
        }
        else
        {
            if (bExp)
            {
                nCntExp = nCounter;
            }
            else if (bDecSep)
            {
                nCntPost = nCounter;
            }
            else
            {
                nCntPre = nCounter;
            }
        }
        if (bThousand) // Expansion of grouping separators
        {
            sal_uInt16 nMaxPos;
            if (bFrac)
            {
                if (bBlank)
                {
                    nMaxPos = nBlankPos;
                }
                else
                {
                    nMaxPos = 0;                // no grouping
                }
            }
            else if (bDecSep)                   // decimal separator present
            {
                nMaxPos = nDecPos;
            }
            else if (bExp)                      // 'E' exponent present
            {
                nMaxPos = nExpPos;
            }
            else                                // up to end
            {
                nMaxPos = i;
            }
            // Insert separators at proper positions.
            sal_Int32 nCount = 0;
            utl::DigitGroupingIterator aGrouping( pLoc->getDigitGrouping());
            size_t nFirstDigitSymbol = nMaxPos;
            size_t nFirstGroupingSymbol = nMaxPos;
            i = nMaxPos;
            while (i-- > 0)
            {
                if (nTypeArray[i] == NF_SYMBOLTYPE_DIGIT)
                {
                    nFirstDigitSymbol = i;
                    nCount = nCount + sStrArray[i].getLength(); // MSC converts += to int and then warns, so ...
                    // Insert separator only if not leftmost symbol.
                    if (i > 0 && nCount >= aGrouping.getPos())
                    {
                        DBG_ASSERT( sStrArray[i].getLength() == 1,
                                    "ImpSvNumberformatScan::FinalScan: combined digits in group separator insertion");
                        if (!InsertSymbol( i, NF_SYMBOLTYPE_THSEP, pFormatter->GetNumThousandSep()))
                        {
                            // nPos isn't correct here, but signals error
                            return nPos;
                        }
                        // i may have been decremented by 1
                        nFirstDigitSymbol = i + 1;
                        nFirstGroupingSymbol = i;
                        aGrouping.advance();
                    }
                }
            }
            // Generated something like "string",000; remove separator again.
            if (nFirstGroupingSymbol < nFirstDigitSymbol)
            {
                nTypeArray[nFirstGroupingSymbol] = NF_SYMBOLTYPE_EMPTY;
                nResultStringsCnt--;
            }
        }
        // Combine digits into groups to save memory (Info will be copied
        // later, taking only non-empty symbols).
        for (i = 0; i < nStringsCnt; ++i)
        {
            if (nTypeArray[i] == NF_SYMBOLTYPE_DIGIT)
            {
                OUString& rStr = sStrArray[i];
                while (++i < nStringsCnt && nTypeArray[i] == NF_SYMBOLTYPE_DIGIT)
                {
                    rStr += sStrArray[i];
                    nTypeArray[i] = NF_SYMBOLTYPE_EMPTY;
                    nResultStringsCnt--;
                }
            }
        }
        break; // of SvNumFormatType::NUMBER
    case SvNumFormatType::DATE:
        while (i < nStringsCnt)
        {
            switch (nTypeArray[i])
            {
            case NF_SYMBOLTYPE_BLANK:
            case NF_SYMBOLTYPE_STAR:
            case NF_SYMBOLTYPE_STRING:
                nPos = nPos + sStrArray[i].getLength();
                i++;
                break;
            case NF_SYMBOLTYPE_DEL:
                int nCalRet;
                if (sStrArray[i] == sOldDateSep)
                {
                    nTypeArray[i] = NF_SYMBOLTYPE_DATESEP;
                    nPos = nPos + sStrArray[i].getLength();
                    if (bConvertMode)
                    {
                        sStrArray[i] = pFormatter->GetDateSep();
                    }
                    i++;
                }
                else if ( (nCalRet = FinalScanGetCalendar( nPos, i, nResultStringsCnt )) != 0 )
                {
                    if ( nCalRet < 0  )
                    {
                        return nPos; // error
                    }
                }
                else
                {
                    nTypeArray[i] = NF_SYMBOLTYPE_STRING;
                    nPos = nPos + sStrArray[i].getLength();
                    i++;
                }
                break;
            case NF_KEY_THAI_T :
                bThaiT = true;
                [[fallthrough]];
            case NF_KEY_M:                          // M
            case NF_KEY_MM:                         // MM
            case NF_KEY_MMM:                        // MMM
            case NF_KEY_MMMM:                       // MMMM
            case NF_KEY_MMMMM:                      // MMMMM
            case NF_KEY_Q:                          // Q
            case NF_KEY_QQ:                         // QQ
            case NF_KEY_D:                          // D
            case NF_KEY_DD:                         // DD
            case NF_KEY_DDD:                        // DDD
            case NF_KEY_DDDD:                       // DDDD
            case NF_KEY_YY:                         // YY
            case NF_KEY_YYYY:                       // YYYY
            case NF_KEY_NN:                         // NN
            case NF_KEY_NNN:                        // NNN
            case NF_KEY_NNNN:                       // NNNN
            case NF_KEY_WW :                        // WW
            case NF_KEY_AAA :                       // AAA
            case NF_KEY_AAAA :                      // AAAA
            case NF_KEY_EC :                        // E
            case NF_KEY_EEC :                       // EE
            case NF_KEY_G :                         // G
            case NF_KEY_GG :                        // GG
            case NF_KEY_GGG :                       // GGG
            case NF_KEY_R :                         // R
            case NF_KEY_RR :                        // RR
                sStrArray[i] = sKeyword[nTypeArray[i]]; // tTtT -> TTTT
                nPos = nPos + sStrArray[i].getLength();
                if (bNewDateOrder)
                {
                    // For simple numeric date formats record date order and
                    // later rearrange.
                    switch (nTypeArray[i])
                    {
                        case NF_KEY_M:
                        case NF_KEY_MM:
                            if (nMonthPos == SAL_MAX_UINT16)
                                nMonthPos = i;
                            else
                                bNewDateOrder = false;
                        break;
                        case NF_KEY_D:
                        case NF_KEY_DD:
                            if (nDayPos == SAL_MAX_UINT16)
                                nDayPos = i;
                            else
                                bNewDateOrder = false;
                        break;
                        case NF_KEY_YY:
                        case NF_KEY_YYYY:
                            if (nYearPos == SAL_MAX_UINT16)
                                nYearPos = i;
                            else
                                bNewDateOrder = false;
                        break;
                        default:
                            ;   // nothing
                    }
                }
                i++;
                break;
            default: // Other keywords
                nTypeArray[i] = NF_SYMBOLTYPE_STRING;
                nPos = nPos + sStrArray[i].getLength();
                i++;
                break;
            }
        } // of while
        break; // of SvNumFormatType::DATE
    case SvNumFormatType::TIME:
        while (i < nStringsCnt)
        {
            sal_Unicode cChar;

            switch (nTypeArray[i])
            {
            case NF_SYMBOLTYPE_BLANK:
            case NF_SYMBOLTYPE_STAR:
                nPos = nPos + sStrArray[i].getLength();
                i++;
                break;
            case NF_SYMBOLTYPE_DEL:
                switch( sStrArray[i][0] )
                {
                case '0':
                    if ( Is100SecZero( i, bDecSep ) )
                    {
                        bDecSep = true;
                        nTypeArray[i] = NF_SYMBOLTYPE_DIGIT;
                        OUString& rStr = sStrArray[i];
                        nCounter++;
                        i++;
                        while (i < nStringsCnt &&
                               sStrArray[i][0] == '0')
                        {
                            rStr += sStrArray[i];
                            nTypeArray[i] = NF_SYMBOLTYPE_EMPTY;
                            nResultStringsCnt--;
                            nCounter++;
                            i++;
                        }
                        nPos += rStr.getLength();
                    }
                    else
                    {
                        return nPos;
                    }
                    break;
                case '#':
                case '?':
                    return nPos;
                case '[':
                    if (bThousand) // Double
                    {
                        return nPos;
                    }
                    bThousand = true; // Empty for Time
                    cChar = pChrCls->uppercase(OUString(NextChar(i)))[0];
                    if ( cChar == cOldKeyH )
                    {
                        nThousand = 1;      // H
                    }
                    else if ( cChar == cOldKeyMI )
                    {
                        nThousand = 2;      // M
                    }
                    else if ( cChar == cOldKeyS )
                    {
                        nThousand = 3;      // S
                    }
                    else
                    {
                        return nPos;
                    }
                    nPos = nPos + sStrArray[i].getLength();
                    i++;
                    break;
                case ']':
                    if (!bThousand) // No preceding [
                    {
                        return nPos;
                    }
                    nPos = nPos + sStrArray[i].getLength();
                    i++;
                    break;
                default:
                    nPos = nPos + sStrArray[i].getLength();
                    if ( sStrArray[i] == sOldTimeSep )
                    {
                        nTypeArray[i] = NF_SYMBOLTYPE_TIMESEP;
                        if ( bConvertMode )
                        {
                            sStrArray[i] = pLoc->getTimeSep();
                        }
                    }
                    else if ( sStrArray[i] == sOldTime100SecSep )
                    {
                        bDecSep = true;
                        nTypeArray[i] = NF_SYMBOLTYPE_TIME100SECSEP;
                        if ( bConvertMode )
                        {
                            sStrArray[i] = pLoc->getTime100SecSep();
                        }
                    }
                    else
                    {
                        nTypeArray[i] = NF_SYMBOLTYPE_STRING;
                    }
                    i++;
                    break;
                }
                break;
            case NF_SYMBOLTYPE_STRING:
                nPos = nPos + sStrArray[i].getLength();
                i++;
                break;
            case NF_KEY_AMPM:                       // AM/PM
            case NF_KEY_AP:                         // A/P
                bExp = true;                        // Abuse for A/P
                sStrArray[i] = sKeyword[nTypeArray[i]]; // tTtT -> TTTT
                nPos = nPos + sStrArray[i].getLength();
                i++;
                break;
            case NF_KEY_THAI_T :
                bThaiT = true;
                [[fallthrough]];
            case NF_KEY_MI:                         // M
            case NF_KEY_MMI:                        // MM
            case NF_KEY_H:                          // H
            case NF_KEY_HH:                         // HH
            case NF_KEY_S:                          // S
            case NF_KEY_SS:                         // SS
                sStrArray[i] = sKeyword[nTypeArray[i]]; // tTtT -> TTTT
                nPos = nPos + sStrArray[i].getLength();
                i++;
                break;
            default: // Other keywords
                nTypeArray[i] = NF_SYMBOLTYPE_STRING;
                nPos = nPos + sStrArray[i].getLength();
                i++;
                break;
            }
        }                                       // of while
        nCntPost = nCounter;                    // Zero counter
        if (bExp)
        {
            nCntExp = 1;                        // Remembers AM/PM
        }
        break;                                 // of SvNumFormatType::TIME
    case SvNumFormatType::DATETIME:
        while (i < nStringsCnt)
        {
            int nCalRet;
            switch (nTypeArray[i])
            {
            case NF_SYMBOLTYPE_BLANK:
            case NF_SYMBOLTYPE_STAR:
            case NF_SYMBOLTYPE_STRING:
                nPos = nPos + sStrArray[i].getLength();
                i++;
                break;
            case NF_SYMBOLTYPE_DEL:
                if ( (nCalRet = FinalScanGetCalendar( nPos, i, nResultStringsCnt )) != 0 )
                {
                    if ( nCalRet < 0  )
                    {
                        return nPos; // Error
                    }
                }
                else
                {
                    switch( sStrArray[i][0] )
                    {
                    case '0':
                        if (bTimePart && Is100SecZero(i, bDecSep) && nCounter < MaxCntPost)
                        {
                            bDecSep = true;
                            nTypeArray[i] = NF_SYMBOLTYPE_DIGIT;
                            OUString& rStr = sStrArray[i];
                            nCounter++;
                            i++;
                            while (i < nStringsCnt &&
                                   sStrArray[i][0] == '0' && nCounter < MaxCntPost)
                            {
                                rStr += sStrArray[i];
                                nTypeArray[i] = NF_SYMBOLTYPE_EMPTY;
                                nResultStringsCnt--;
                                nCounter++;
                                i++;
                            }
                            nPos += rStr.getLength();
                        }
                        else
                        {
                            return nPos;
                        }
                        break;
                    case '#':
                    case '?':
                        return nPos;
                    default:
                        nPos = nPos + sStrArray[i].getLength();
                        if (bTimePart)
                        {
                            if ( sStrArray[i] == sOldTimeSep )
                            {
                                nTypeArray[i] = NF_SYMBOLTYPE_TIMESEP;
                                if ( bConvertMode )
                                {
                                    sStrArray[i] = pLoc->getTimeSep();
                                }
                            }
                            else if ( sStrArray[i] == sOldTime100SecSep )
                            {
                                bDecSep = true;
                                nTypeArray[i] = NF_SYMBOLTYPE_TIME100SECSEP;
                                if ( bConvertMode )
                                {
                                    sStrArray[i] = pLoc->getTime100SecSep();
                                }
                            }
                            else
                            {
                                nTypeArray[i] = NF_SYMBOLTYPE_STRING;
                            }
                        }
                        else
                        {
                            if ( sStrArray[i] == sOldDateSep )
                            {
                                nTypeArray[i] = NF_SYMBOLTYPE_DATESEP;
                                if (bConvertMode)
                                    sStrArray[i] = pFormatter->GetDateSep();
                            }
                            else
                            {
                                nTypeArray[i] = NF_SYMBOLTYPE_STRING;
                            }
                        }
                        i++;
                        break;
                    }
                }
                break;
            case NF_KEY_AMPM:                       // AM/PM
            case NF_KEY_AP:                         // A/P
                bTimePart = true;
                bExp = true;                        // Abuse for A/P
                sStrArray[i] = sKeyword[nTypeArray[i]]; // tTtT -> TTTT
                nPos = nPos + sStrArray[i].getLength();
                i++;
                break;
            case NF_KEY_MI:                         // M
            case NF_KEY_MMI:                        // MM
            case NF_KEY_H:                          // H
            case NF_KEY_HH:                         // HH
            case NF_KEY_S:                          // S
            case NF_KEY_SS:                         // SS
                bTimePart = true;
                sStrArray[i] = sKeyword[nTypeArray[i]]; // tTtT -> TTTT
                nPos = nPos + sStrArray[i].getLength();
                i++;
                break;
            case NF_KEY_M:                          // M
            case NF_KEY_MM:                         // MM
            case NF_KEY_MMM:                        // MMM
            case NF_KEY_MMMM:                       // MMMM
            case NF_KEY_MMMMM:                      // MMMMM
            case NF_KEY_Q:                          // Q
            case NF_KEY_QQ:                         // QQ
            case NF_KEY_D:                          // D
            case NF_KEY_DD:                         // DD
            case NF_KEY_DDD:                        // DDD
            case NF_KEY_DDDD:                       // DDDD
            case NF_KEY_YY:                         // YY
            case NF_KEY_YYYY:                       // YYYY
            case NF_KEY_NN:                         // NN
            case NF_KEY_NNN:                        // NNN
            case NF_KEY_NNNN:                       // NNNN
            case NF_KEY_WW :                        // WW
            case NF_KEY_AAA :                       // AAA
            case NF_KEY_AAAA :                      // AAAA
            case NF_KEY_EC :                        // E
            case NF_KEY_EEC :                       // EE
            case NF_KEY_G :                         // G
            case NF_KEY_GG :                        // GG
            case NF_KEY_GGG :                       // GGG
            case NF_KEY_R :                         // R
            case NF_KEY_RR :                        // RR
                bTimePart = false;
                sStrArray[i] = sKeyword[nTypeArray[i]]; // tTtT -> TTTT
                nPos = nPos + sStrArray[i].getLength();
                if (bNewDateOrder)
                {
                    // For simple numeric date formats record date order and
                    // later rearrange.
                    switch (nTypeArray[i])
                    {
                        case NF_KEY_M:
                        case NF_KEY_MM:
                            if (nMonthPos == SAL_MAX_UINT16)
                                nMonthPos = i;
                            else
                                bNewDateOrder = false;
                        break;
                        case NF_KEY_D:
                        case NF_KEY_DD:
                            if (nDayPos == SAL_MAX_UINT16)
                                nDayPos = i;
                            else
                                bNewDateOrder = false;
                        break;
                        case NF_KEY_YY:
                        case NF_KEY_YYYY:
                            if (nYearPos == SAL_MAX_UINT16)
                                nYearPos = i;
                            else
                                bNewDateOrder = false;
                        break;
                        default:
                            ;   // nothing
                    }
                }
                i++;
                break;
            case NF_KEY_THAI_T :
                bThaiT = true;
                sStrArray[i] = sKeyword[nTypeArray[i]];
                nPos = nPos + sStrArray[i].getLength();
                i++;
                break;
            default: // Other keywords
                nTypeArray[i] = NF_SYMBOLTYPE_STRING;
                nPos = nPos + sStrArray[i].getLength();
                i++;
                break;
            }
        } // of while
        nCntPost = nCounter; // decimals (100th seconds)
        if (bExp)
        {
            nCntExp = 1; // Remembers AM/PM
        }
        break; // of SvNumFormatType::DATETIME
    default:
        break;
    }
    if (eScannedType == SvNumFormatType::SCIENTIFIC &&
        (nCntPre + nCntPost == 0 || nCntExp == 0))
    {
        return nPos;
    }
    else if (eScannedType == SvNumFormatType::FRACTION && (nCntExp > 8 || nCntExp == 0))
    {
        return nPos;
    }
    if (bThaiT && !GetNatNumModifier())
    {
        SetNatNumModifier(1);
    }
    if ( bConvertMode )
    {
        if (bNewDateOrder && sOldDateSep == "-")
        {
            // Keep ISO formats Y-M-D, Y-M and M-D
            if (IsDateFragment( nYearPos, nMonthPos))
            {
                nTypeArray[nYearPos+1] = NF_SYMBOLTYPE_STRING;
                sStrArray[nYearPos+1] = sOldDateSep;
                bNewDateOrder = false;
            }
            if (IsDateFragment( nMonthPos, nDayPos))
            {
                nTypeArray[nMonthPos+1] = NF_SYMBOLTYPE_STRING;
                sStrArray[nMonthPos+1] = sOldDateSep;
                bNewDateOrder = false;
            }
        }
        if (bNewDateOrder)
        {
            // Rearrange date order to the target locale if the original order
            // includes date separators and is adjacent.
            /* TODO: for incomplete dates trailing separators need to be
             * handled according to the locale's usage, e.g. en-US M/D should
             * be converted to de-DE D.M. and vice versa. As is, it's
             * M/D -> D.M and D.M. -> M/D/ where specifically the latter looks
             * odd. Check accepted date patterns and append/remove? */
            switch (eOldDateOrder)
            {
                case DateOrder::DMY:
                    switch (pLoc->getDateOrder())
                    {
                        case DateOrder::MDY:
                            // Convert only if the actual format is not of YDM
                            // order (which would be a completely unusual order
                            // anyway, but..), e.g. YYYY.DD.MM not to
                            // YYYY/MM/DD
                            if (IsDateFragment( nDayPos, nMonthPos) && !IsDateFragment( nYearPos, nDayPos))
                                SwapArrayElements( nDayPos, nMonthPos);
                        break;
                        case DateOrder::YMD:
                            if (nYearPos != SAL_MAX_UINT16)
                            {
                                if (IsDateFragment( nDayPos, nMonthPos) && IsDateFragment( nMonthPos, nYearPos))
                                    SwapArrayElements( nDayPos, nYearPos);
                            }
                            else
                            {
                                if (IsDateFragment( nDayPos, nMonthPos))
                                    SwapArrayElements( nDayPos, nMonthPos);
                            }
                        break;
                        default:
                            ;   // nothing
                    }
                break;
                case DateOrder::MDY:
                    switch (pLoc->getDateOrder())
                    {
                        case DateOrder::DMY:
                            // Convert only if the actual format is not of YMD
                            // order, e.g. YYYY/MM/DD not to YYYY.DD.MM
                            /* TODO: convert such to DD.MM.YYYY instead? */
                            if (IsDateFragment( nMonthPos, nDayPos) && !IsDateFragment( nYearPos, nMonthPos))
                                SwapArrayElements( nMonthPos, nDayPos);
                        break;
                        case DateOrder::YMD:
                            if (nYearPos != SAL_MAX_UINT16)
                            {
                                if (IsDateFragment( nMonthPos, nDayPos) && IsDateFragment( nDayPos, nYearPos))
                                {
                                    SwapArrayElements( nYearPos, nMonthPos);    // YDM
                                    SwapArrayElements( nYearPos, nDayPos);      // YMD
                                }
                            }
                        break;
                        default:
                            ;   // nothing
                    }
                break;
                case DateOrder::YMD:
                    switch (pLoc->getDateOrder())
                    {
                        case DateOrder::DMY:
                            if (nYearPos != SAL_MAX_UINT16)
                            {
                                if (IsDateFragment( nYearPos, nMonthPos) && IsDateFragment( nMonthPos, nDayPos))
                                    SwapArrayElements( nYearPos, nDayPos);
                            }
                            else
                            {
                                if (IsDateFragment( nMonthPos, nDayPos))
                                    SwapArrayElements( nMonthPos, nDayPos);
                            }
                        break;
                        case DateOrder::MDY:
                            if (nYearPos != SAL_MAX_UINT16)
                            {
                                if (IsDateFragment( nYearPos, nMonthPos) && IsDateFragment( nMonthPos, nDayPos))
                                {
                                    SwapArrayElements( nYearPos, nDayPos);      // DMY
                                    SwapArrayElements( nYearPos, nMonthPos);    // MDY
                                }
                            }
                        break;
                        default:
                            ;   // nothing
                    }
                break;
                default:
                    ;   // nothing
            }
        }
        // strings containing keywords of the target locale must be quoted, so
        // the user sees the difference and is able to edit the format string
        for ( i=0; i < nStringsCnt; i++ )
        {
            if ( nTypeArray[i] == NF_SYMBOLTYPE_STRING &&
                 sStrArray[i][0] != '\"' )
            {
                if ( bConvertSystemToSystem && eScannedType == SvNumFormatType::CURRENCY )
                {
                    // don't stringize automatic currency, will be converted
                    if ( sStrArray[i] == sOldCurSymbol )
                    {
                        continue; // for
                    }
                    // DM might be split into D and M
                    if ( sStrArray[i].getLength() < sOldCurSymbol.getLength() &&
                         pChrCls->uppercase( sStrArray[i], 0, 1 )[0] ==
                         sOldCurString[0] )
                    {
                        OUString aTmp( sStrArray[i] );
                        sal_uInt16 j = i + 1;
                        while ( aTmp.getLength() < sOldCurSymbol.getLength() &&
                                j < nStringsCnt &&
                                nTypeArray[j] == NF_SYMBOLTYPE_STRING )
                        {
                            aTmp += sStrArray[j++];
                        }
                        if ( pChrCls->uppercase( aTmp ) == sOldCurString )
                        {
                            sStrArray[i++] = aTmp;
                            for ( ; i<j; i++ )
                            {
                                nTypeArray[i] = NF_SYMBOLTYPE_EMPTY;
                                nResultStringsCnt--;
                            }
                            i = j - 1;
                            continue; // for
                        }
                    }
                }
                OUString& rStr = sStrArray[i];
                sal_Int32 nLen = rStr.getLength();
                for ( sal_Int32 j = 0; j < nLen; j++ )
                {
                    bool bFoundEnglish = false;
                    if ( (j == 0 || rStr[j - 1] != '\\') && GetKeyWord( rStr, j, bFoundEnglish) )
                    {
                        rStr = "\"" + rStr + "\"";
                        break; // for
                    }
                }
            }
        }
    }
    // Concatenate strings, remove quotes for output, and rebuild the format string
    rString.clear();
    i = 0;
    while (i < nStringsCnt)
    {
        sal_Int32 nStringPos;
        sal_Int32 nArrPos = 0;
        sal_uInt16 iPos = i;
        switch ( nTypeArray[i] )
        {
        case NF_SYMBOLTYPE_STRING :
        case NF_SYMBOLTYPE_FRACBLANK :
            nStringPos = rString.getLength();
            do
            {
                if (sStrArray[i].getLength() == 2 &&
                    sStrArray[i][0] == '\\')
                {
                    // Unescape some simple forms of symbols even in the UI
                    // visible string to prevent duplicates that differ
                    // only in notation, originating from import.
                    // e.g. YYYY-MM-DD and YYYY\-MM\-DD are identical,
                    // but 0\ 000 0 and 0 000 0 in a French locale are not.

                    sal_Unicode c = sStrArray[i][1];

                    switch (c)
                    {
                    case '+':
                    case '-':
                        rString += OUStringLiteral1(c);
                        break;
                    case ' ':
                    case '.':
                    case '/':
                        if (!(eScannedType & SvNumFormatType::DATE) &&
                            (StringEqualsChar( pFormatter->GetNumThousandSep(), c) ||
                             StringEqualsChar( pFormatter->GetNumDecimalSep(), c) ||
                             (c == ' ' &&
                              (StringEqualsChar( pFormatter->GetNumThousandSep(), cNoBreakSpace) ||
                               StringEqualsChar( pFormatter->GetNumThousandSep(), cNarrowNoBreakSpace)))))
                        {
                            rString += sStrArray[i];
                        }
                        else if ((eScannedType & SvNumFormatType::DATE) &&
                                 StringEqualsChar( pFormatter->GetDateSep(), c))
                        {
                            rString += sStrArray[i];
                        }
                        else if ((eScannedType & SvNumFormatType::TIME) &&
                                 (StringEqualsChar( pLoc->getTimeSep(), c) ||
                                  StringEqualsChar( pLoc->getTime100SecSep(), c)))
                        {
                            rString += sStrArray[i];
                        }
                        else if (eScannedType & SvNumFormatType::FRACTION)
                        {
                            rString += sStrArray[i];
                        }
                        else
                        {
                            rString += OUStringLiteral1(c);
                        }
                        break;
                    default:
                        rString += sStrArray[i];
                    }
                }
                else
                {
                    rString += sStrArray[i];
                }
                if ( RemoveQuotes( sStrArray[i] ) > 0 )
                {
                    // update currency up to quoted string
                    if ( eScannedType == SvNumFormatType::CURRENCY )
                    {
                        // dM -> DM  or  DM -> $  in old automatic
                        // currency formats, oh my ..., why did we ever introduce them?
                        OUString aTmp( pChrCls->uppercase( sStrArray[iPos], nArrPos,
                                                           sStrArray[iPos].getLength()-nArrPos ) );
                        sal_Int32 nCPos = aTmp.indexOf( sOldCurString );
                        if ( nCPos >= 0 )
                        {
                            const OUString& rCur = bConvertMode && bConvertSystemToSystem ?
                                GetCurSymbol() : sOldCurSymbol;
                            sStrArray[iPos] = sStrArray[iPos].replaceAt( nArrPos + nCPos,
                                                                         sOldCurString.getLength(),
                                                                         rCur );
                            rString = rString.replaceAt( nStringPos + nCPos,
                                                         sOldCurString.getLength(),
                                                         rCur );
                        }
                        nStringPos = rString.getLength();
                        if ( iPos == i )
                        {
                            nArrPos = sStrArray[iPos].getLength();
                        }
                        else
                        {
                            nArrPos = sStrArray[iPos].getLength() + sStrArray[i].getLength();
                        }
                    }
                }
                if ( iPos != i )
                {
                    sStrArray[iPos] += sStrArray[i];
                    nTypeArray[i] = NF_SYMBOLTYPE_EMPTY;
                    nResultStringsCnt--;
                }
                i++;
            }
            while ( i < nStringsCnt && nTypeArray[i] == NF_SYMBOLTYPE_STRING );

            if ( i < nStringsCnt )
            {
                i--; // enter switch on next symbol again
            }
            if ( eScannedType == SvNumFormatType::CURRENCY && nStringPos < rString.getLength() )
            {
                // same as above, since last RemoveQuotes
                OUString aTmp( pChrCls->uppercase( sStrArray[iPos], nArrPos,
                                                   sStrArray[iPos].getLength()-nArrPos ) );
                sal_Int32 nCPos = aTmp.indexOf( sOldCurString );
                if ( nCPos >= 0 )
                {
                    const OUString& rCur = bConvertMode && bConvertSystemToSystem ?
                        GetCurSymbol() : sOldCurSymbol;
                    sStrArray[iPos] = sStrArray[iPos].replaceAt( nArrPos + nCPos,
                                                                 sOldCurString.getLength(),
                                                                 rCur );
                    rString = rString.replaceAt( nStringPos + nCPos,
                                                 sOldCurString.getLength(), rCur );
                }
            }
            break;
        case NF_SYMBOLTYPE_CURRENCY :
            rString += sStrArray[i];
            RemoveQuotes( sStrArray[i] );
            break;
        case NF_KEY_THAI_T:
            if (bThaiT && GetNatNumModifier() == 1)
            {
                // Remove T from format code, will be replaced with a [NatNum1] prefix.
                nTypeArray[i] = NF_SYMBOLTYPE_EMPTY;
                nResultStringsCnt--;
            }
            else
            {
                rString += sStrArray[i];
            }
            break;
        case NF_SYMBOLTYPE_EMPTY :
            // nothing
            break;
        default:
            rString += sStrArray[i];
        }
        i++;
    }
    return 0;
}

sal_Int32 ImpSvNumberformatScan::RemoveQuotes( OUString& rStr )
{
    if ( rStr.getLength() > 1 )
    {
        sal_Unicode c = rStr[0];
        sal_Int32 n = rStr.getLength() - 1;
        if ( c == '"' && rStr[n] == '"' )
        {
            rStr = rStr.copy( 1, n-1);
            return 2;
        }
        else if ( c == '\\' )
        {
            rStr = rStr.copy(1);
            return 1;
        }
    }
    return 0;
}

sal_Int32 ImpSvNumberformatScan::ScanFormat( OUString& rString )
{
    sal_Int32 res = Symbol_Division(rString); // Lexical analysis
    if (!res)
    {
        res = ScanType(); // Recognizing the Format type
    }
    if (!res)
    {
        res = FinalScan( rString ); // Type dependent final analysis
    }
    return res; // res = control position; res = 0 => Format ok
}

void ImpSvNumberformatScan::CopyInfo(ImpSvNumberformatInfo* pInfo, sal_uInt16 nCnt)
{
    size_t i,j;
    j = 0;
    i = 0;
    while (i < nCnt && j < NF_MAX_FORMAT_SYMBOLS)
    {
        if (nTypeArray[j] != NF_SYMBOLTYPE_EMPTY)
        {
            pInfo->sStrArray[i]  = sStrArray[j];
            pInfo->nTypeArray[i] = nTypeArray[j];
            i++;
        }
        j++;
    }
    pInfo->eScannedType = eScannedType;
    pInfo->bThousand    = bThousand;
    pInfo->nThousand    = nThousand;
    pInfo->nCntPre      = nCntPre;
    pInfo->nCntPost     = nCntPost;
    pInfo->nCntExp      = nCntExp;
}

void ImpSvNumberformatScan::ReplaceBooleanEquivalent( OUString& rString )
{
    InitKeywords();
    /* TODO: compare case insensitive? Or rather leave as is and case not
     * matching indicates user supplied on purpose? Written to file / generated
     * was always uppercase. */
    if (rString == sBooleanEquivalent1 || rString == sBooleanEquivalent2)
        rString = GetKeywords()[NF_KEY_BOOLEAN];
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
