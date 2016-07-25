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
#include <unotools/numberformatcodewrapper.hxx>
#include <rtl/instance.hxx>

#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <unotools/digitgroupingiterator.hxx>

#include "zforscan.hxx"

#include <svl/nfsymbol.hxx>
using namespace svt;

const sal_Unicode cNoBreakSpace = 0xA0;
const sal_Unicode cNarrowNoBreakSpace = 0x202F;

namespace
{
    struct ImplEnglishColors
    {
        const OUString* operator()()
        {
            static const OUString aEnglishColors[NF_MAX_DEFAULT_COLORS] =
            {
                OUString( "BLACK" ),
                OUString( "BLUE" ),
                OUString( "GREEN" ),
                OUString( "CYAN" ),
                OUString( "RED" ),
                OUString( "MAGENTA" ),
                OUString( "BROWN" ),
                OUString( "GREY" ),
                OUString( "YELLOW" ),
                OUString( "WHITE" )
            };
            return &aEnglishColors[0];
        }
    };

    struct theEnglishColors
            : public rtl::StaticAggregate< const OUString, ImplEnglishColors> {};

}

ImpSvNumberformatScan::ImpSvNumberformatScan( SvNumberFormatter* pFormatterP )
    : eNewLnge(LANGUAGE_DONTKNOW)
    , eTmpLnge(LANGUAGE_DONTKNOW)
    , nCurrPos(-1)
{
    pFormatter = pFormatterP;
    bConvertMode = false;
    bConvertSystemToSystem = false;
    //! All keywords MUST be UPPERCASE!
    sKeyword[NF_KEY_E] =     "E";        // Exponent
    sKeyword[NF_KEY_AMPM] =  "AM/PM";    // AM/PM
    sKeyword[NF_KEY_AP] =    "A/P";      // AM/PM short
    sKeyword[NF_KEY_MI] =    "M";        // Minute
    sKeyword[NF_KEY_MMI] =   "MM";       // Minute 02
    sKeyword[NF_KEY_S] =     "S";        // Second
    sKeyword[NF_KEY_SS] =    "SS";       // Second 02
    sKeyword[NF_KEY_Q] =     "Q";        // Quarter short 'Q'
    sKeyword[NF_KEY_QQ] =    "QQ";       // Quarter long
    sKeyword[NF_KEY_NN] =    "NN";       // Day of week short
    sKeyword[NF_KEY_NNN] =   "NNN";      // Day of week long
    sKeyword[NF_KEY_NNNN] =  "NNNN";     // Day of week long incl. separator
    sKeyword[NF_KEY_WW] =    "WW";       // Week of year
    sKeyword[NF_KEY_CCC] =   "CCC";      // Currency abbreviation
    bKeywordsNeedInit = true;            // locale dependent keywords
    bCompatCurNeedInit = true;           // locale dependent compatibility currency strings

    StandardColor[0]  =  Color(COL_BLACK);
    StandardColor[1]  =  Color(COL_LIGHTBLUE);
    StandardColor[2]  =  Color(COL_LIGHTGREEN);
    StandardColor[3]  =  Color(COL_LIGHTCYAN);
    StandardColor[4]  =  Color(COL_LIGHTRED);
    StandardColor[5]  =  Color(COL_LIGHTMAGENTA);
    StandardColor[6]  =  Color(COL_BROWN);
    StandardColor[7]  =  Color(COL_GRAY);
    StandardColor[8]  =  Color(COL_YELLOW);
    StandardColor[9]  =  Color(COL_WHITE);

    pNullDate = new Date(30,12,1899);
    nStandardPrec = 2;

    sErrStr =  "###";
    Reset();
}

ImpSvNumberformatScan::~ImpSvNumberformatScan()
{
    delete pNullDate;
    Reset();
}

void ImpSvNumberformatScan::ChangeIntl()
{
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
            const_cast<ImpSvNumberformatScan*>(this)->sKeyword[NF_KEY_TRUE] = "TRUE";
        }
        break;
    case NF_KEY_FALSE :
        const_cast<ImpSvNumberformatScan*>(this)->sKeyword[NF_KEY_FALSE] =
            pFormatter->GetCharClass()->uppercase( pFormatter->GetLocaleData()->getFalseWord() );
        if ( sKeyword[NF_KEY_FALSE].isEmpty() )
        {
            SAL_WARN( "svl.numbers", "InitSpecialKeyword: FALSE_WORD?" );
            const_cast<ImpSvNumberformatScan*>(this)->sKeyword[NF_KEY_FALSE] = "FALSE";
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
    NumberFormatCodeWrapper aNumberFormatCode( pFormatter->GetComponentContext(),
            rLoadedLocale.getLocale() );

    i18n::NumberFormatCode aFormat = aNumberFormatCode.getFormatCode( NF_NUMBER_STANDARD );
    sNameStandardFormat = lcl_extractStandardGeneralName( aFormat.Code);
    sKeyword[NF_KEY_GENERAL] = pCharClass->uppercase( sNameStandardFormat );

    // preset new calendar keywords
    sKeyword[NF_KEY_AAA] =   "AAA";
    sKeyword[NF_KEY_AAAA] =  "AAAA";
    sKeyword[NF_KEY_EC] =    "E";
    sKeyword[NF_KEY_EEC] =   "EE";
    sKeyword[NF_KEY_G] =     "G";
    sKeyword[NF_KEY_GG] =    "GG";
    sKeyword[NF_KEY_GGG] =   "GGG";
    sKeyword[NF_KEY_R] =     "R";
    sKeyword[NF_KEY_RR] =    "RR";

    // Thai T NatNum special. Other locale's small letter 't' results in upper
    // case comparison not matching but length does in conversion mode. Ugly.
    if (eLang == LANGUAGE_THAI)
    {
        sKeyword[NF_KEY_THAI_T] = "T";
    }
    else
    {
        sKeyword[NF_KEY_THAI_T] = "t";
    }
    switch ( eLang )
    {
    case LANGUAGE_GERMAN:
    case LANGUAGE_GERMAN_SWISS:
    case LANGUAGE_GERMAN_AUSTRIAN:
    case LANGUAGE_GERMAN_LUXEMBOURG:
    case LANGUAGE_GERMAN_LIECHTENSTEIN:
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
        sKeyword[NF_KEY_COLOR] =     "FARBE";
        sKeyword[NF_KEY_BLACK] =     "SCHWARZ";
        sKeyword[NF_KEY_BLUE] =      "BLAU";
        sKeyword[NF_KEY_GREEN] = OUString( "GR" "\xDC" "N", 4, RTL_TEXTENCODING_ISO_8859_1 );
        sKeyword[NF_KEY_CYAN] =      "CYAN";
        sKeyword[NF_KEY_RED] =       "ROT";
        sKeyword[NF_KEY_MAGENTA] =   "MAGENTA";
        sKeyword[NF_KEY_BROWN] =     "BRAUN";
        sKeyword[NF_KEY_GREY] =      "GRAU";
        sKeyword[NF_KEY_YELLOW] =    "GELB";
        sKeyword[NF_KEY_WHITE] =     "WEISS";
        break;
    default:
        // day
        switch ( eLang )
        {
        case LANGUAGE_ITALIAN:
        case LANGUAGE_ITALIAN_SWISS:
            sKeyword[NF_KEY_D] = "G";
            sKeyword[NF_KEY_DD] = "GG";
            sKeyword[NF_KEY_DDD] = "GGG";
            sKeyword[NF_KEY_DDDD] = "GGGG";
            // must exchange the era code, same as Xcl
            sKeyword[NF_KEY_G] = "X";
            sKeyword[NF_KEY_GG] = "XX";
            sKeyword[NF_KEY_GGG] = "XXX";
            break;
        case LANGUAGE_FRENCH:
        case LANGUAGE_FRENCH_BELGIAN:
        case LANGUAGE_FRENCH_CANADIAN:
        case LANGUAGE_FRENCH_SWISS:
        case LANGUAGE_FRENCH_LUXEMBOURG:
        case LANGUAGE_FRENCH_MONACO:
            sKeyword[NF_KEY_D] = "J";
            sKeyword[NF_KEY_DD] = "JJ";
            sKeyword[NF_KEY_DDD] = "JJJ";
            sKeyword[NF_KEY_DDDD] = "JJJJ";
            break;
        case LANGUAGE_FINNISH:
            sKeyword[NF_KEY_D] = "P";
            sKeyword[NF_KEY_DD] = "PP";
            sKeyword[NF_KEY_DDD] = "PPP";
            sKeyword[NF_KEY_DDDD] = "PPPP";
            break;
        default:
            sKeyword[NF_KEY_D] = "D";
            sKeyword[NF_KEY_DD] = "DD";
            sKeyword[NF_KEY_DDD] = "DDD";
            sKeyword[NF_KEY_DDDD] = "DDDD";
        }
        // month
        switch ( eLang )
        {
        case LANGUAGE_FINNISH:
            sKeyword[NF_KEY_M] = "K";
            sKeyword[NF_KEY_MM] = "KK";
            sKeyword[NF_KEY_MMM] = "KKK";
            sKeyword[NF_KEY_MMMM] = "KKKK";
            sKeyword[NF_KEY_MMMMM] = "KKKKK";
            break;
        default:
            sKeyword[NF_KEY_M] = "M";
            sKeyword[NF_KEY_MM] = "MM";
            sKeyword[NF_KEY_MMM] = "MMM";
            sKeyword[NF_KEY_MMMM] = "MMMM";
            sKeyword[NF_KEY_MMMMM] = "MMMMM";
        }
        // year
        switch ( eLang )
        {
        case LANGUAGE_ITALIAN:
        case LANGUAGE_ITALIAN_SWISS:
        case LANGUAGE_FRENCH:
        case LANGUAGE_FRENCH_BELGIAN:
        case LANGUAGE_FRENCH_CANADIAN:
        case LANGUAGE_FRENCH_SWISS:
        case LANGUAGE_FRENCH_LUXEMBOURG:
        case LANGUAGE_FRENCH_MONACO:
        case LANGUAGE_PORTUGUESE:
        case LANGUAGE_PORTUGUESE_BRAZILIAN:
        case LANGUAGE_SPANISH_MODERN:
        case LANGUAGE_SPANISH_DATED:
        case LANGUAGE_SPANISH_MEXICAN:
        case LANGUAGE_SPANISH_GUATEMALA:
        case LANGUAGE_SPANISH_COSTARICA:
        case LANGUAGE_SPANISH_PANAMA:
        case LANGUAGE_SPANISH_DOMINICAN_REPUBLIC:
        case LANGUAGE_SPANISH_VENEZUELA:
        case LANGUAGE_SPANISH_COLOMBIA:
        case LANGUAGE_SPANISH_PERU:
        case LANGUAGE_SPANISH_ARGENTINA:
        case LANGUAGE_SPANISH_ECUADOR:
        case LANGUAGE_SPANISH_CHILE:
        case LANGUAGE_SPANISH_URUGUAY:
        case LANGUAGE_SPANISH_PARAGUAY:
        case LANGUAGE_SPANISH_BOLIVIA:
        case LANGUAGE_SPANISH_EL_SALVADOR:
        case LANGUAGE_SPANISH_HONDURAS:
        case LANGUAGE_SPANISH_NICARAGUA:
        case LANGUAGE_SPANISH_PUERTO_RICO:
            sKeyword[NF_KEY_YY] = "AA";
            sKeyword[NF_KEY_YYYY] = "AAAA";
            // must exchange the day of week name code, same as Xcl
            sKeyword[NF_KEY_AAA] =   "OOO";
            sKeyword[NF_KEY_AAAA] =  "OOOO";
            break;
        case LANGUAGE_DUTCH:
        case LANGUAGE_DUTCH_BELGIAN:
            sKeyword[NF_KEY_YY] = "JJ";
            sKeyword[NF_KEY_YYYY] = "JJJJ";
            break;
        case LANGUAGE_FINNISH:
            sKeyword[NF_KEY_YY] = "VV";
            sKeyword[NF_KEY_YYYY] = "VVVV";
            break;
        default:
            sKeyword[NF_KEY_YY] = "YY";
            sKeyword[NF_KEY_YYYY] = "YYYY";
        }
        // hour
        switch ( eLang )
        {
        case LANGUAGE_DUTCH:
        case LANGUAGE_DUTCH_BELGIAN:
            sKeyword[NF_KEY_H] = "U";
            sKeyword[NF_KEY_HH] = "UU";
            break;
        case LANGUAGE_FINNISH:
        case LANGUAGE_SWEDISH:
        case LANGUAGE_SWEDISH_FINLAND:
        case LANGUAGE_DANISH:
        case LANGUAGE_NORWEGIAN:
        case LANGUAGE_NORWEGIAN_BOKMAL:
        case LANGUAGE_NORWEGIAN_NYNORSK:
            sKeyword[NF_KEY_H] = "T";
            sKeyword[NF_KEY_HH] = "TT";
            break;
        default:
            sKeyword[NF_KEY_H] = "H";
            sKeyword[NF_KEY_HH] = "HH";
        }
        // boolean
        sKeyword[NF_KEY_BOOLEAN] = "BOOLEAN";
        // colours
        sKeyword[NF_KEY_COLOR] =     "COLOR";
        sKeyword[NF_KEY_BLACK] =     "BLACK";
        sKeyword[NF_KEY_BLUE] =      "BLUE";
        sKeyword[NF_KEY_GREEN] =     "GREEN";
        sKeyword[NF_KEY_CYAN] =      "CYAN";
        sKeyword[NF_KEY_RED] =       "RED";
        sKeyword[NF_KEY_MAGENTA] =   "MAGENTA";
        sKeyword[NF_KEY_BROWN] =     "BROWN";
        sKeyword[NF_KEY_GREY] =      "GREY";
        sKeyword[NF_KEY_YELLOW] =    "YELLOW";
        sKeyword[NF_KEY_WHITE] =     "WHITE";
        break;
    }

    // boolean keywords
    InitSpecialKeyword( NF_KEY_TRUE );
    InitSpecialKeyword( NF_KEY_FALSE );

    // compatibility currency strings
    InitCompatCur();
}

void ImpSvNumberformatScan::ChangeNullDate(sal_uInt16 nDay, sal_uInt16 nMonth, sal_uInt16 nYear)
{
    if ( pNullDate )
        *pNullDate = Date(nDay, nMonth, nYear);
    else
        pNullDate = new Date(nDay, nMonth, nYear);
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
    if ( i >= NF_MAX_DEFAULT_COLORS )
    {
        const OUString* pEnglishColors = theEnglishColors::get();
        size_t j = 0;
        while ( j < NF_MAX_DEFAULT_COLORS && sString != pEnglishColors[j] )
        {
            ++j;
        }
        if ( j < NF_MAX_DEFAULT_COLORS )
        {
            i = j;
        }
    }

    Color* pResult = nullptr;
    if (i >= NF_MAX_DEFAULT_COLORS)
    {
        const OUString& rColorWord = rKeyword[NF_KEY_COLOR];
        if (sString.startsWith(rColorWord))
        {
            sal_Int32 nPos = rColorWord.getLength();
            sStr = sStr.copy(nPos);
            sStr = comphelper::string::strip(sStr, ' ');
            if (bConvertMode)
            {
                pFormatter->ChangeIntl(eNewLnge);
                sStr = GetKeywords()[NF_KEY_COLOR] + sStr; // Color -> FARBE
                pFormatter->ChangeIntl(eTmpLnge);
            }
            else
            {
                sStr = rColorWord + sStr;
            }
            sString = sString.copy(nPos);
            sString = comphelper::string::strip(sString, ' ');

            if ( CharClass::isAsciiNumeric( sString ) )
            {
                long nIndex = sString.toInt32();
                if (nIndex > 0 && nIndex <= 64)
                {
                    pResult = pFormatter->GetUserDefColor((sal_uInt16)nIndex-1);
                }
            }
        }
    }
    else
    {
        sStr.clear();
        if (bConvertMode)
        {
            pFormatter->ChangeIntl(eNewLnge);
            sStr = GetKeywords()[NF_KEY_FIRSTCOLOR+i]; // red -> rot
            pFormatter->ChangeIntl(eTmpLnge);
        }
        else
        {
            sStr = rKeyword[NF_KEY_FIRSTCOLOR+i];
        }
        pResult = &(StandardColor[i]);
    }
    return pResult;
}

short ImpSvNumberformatScan::GetKeyWord( const OUString& sSymbol, sal_Int32 nPos )
{
    OUString sString = pFormatter->GetCharClass()->uppercase( sSymbol, nPos, sSymbol.getLength() - nPos );
    const NfKeywordTable & rKeyword = GetKeywords();
    // #77026# for the Xcl perverts: the GENERAL keyword is recognized anywhere
    if ( sString.startsWith( rKeyword[NF_KEY_GENERAL] ))
    {
        return NF_KEY_GENERAL;
    }
    //! MUST be a reverse search to find longer strings first
    short i = NF_KEYWORD_ENTRIES_COUNT-1;
    bool bFound = false;
    for ( ; i > NF_KEY_LASTKEYWORD_SO5; --i )
    {
        bFound = sString.startsWith(rKeyword[i]);
        if ( bFound )
        {
            break;
        }
    }
    // new keywords take precedence over old keywords
    if ( !bFound )
    {
        // skip the gap of colors et al between new and old keywords and search on
        i = NF_KEY_LASTKEYWORD;
        while ( i > 0 && sString.indexOf(rKeyword[i]) != 0 )
        {
            i--;
        }
        if ( i > NF_KEY_LASTOLDKEYWORD && sString != rKeyword[i] )
        {
            // found something, but maybe it's something else?
            // e.g. new NNN is found in NNNN, for NNNN we must search on
            short j = i - 1;
            while ( j > 0 && sString.indexOf(rKeyword[j]) != 0 )
            {
                j--;
            }
            if ( j && rKeyword[j].getLength() > rKeyword[i].getLength() )
            {
                return j;
            }
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
                                          OUString& sSymbol )
{
    if ( bKeywordsNeedInit )
    {
        InitKeywords();
    }
    const CharClass* pChrCls = pFormatter->GetCharClass();
    const LocaleDataWrapper* pLoc = pFormatter->GetLocaleData();
    short eType = 0;
    ScanState eState = SsStart;
    sSymbol.clear();
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
                sSymbol += OUString(cToken);
                eState = SsStop;
                break;
            case '*':
                eType = NF_SYMBOLTYPE_STAR;
                sSymbol += OUString(cToken);
                eState = SsGetStar;
                break;
            case '_':
                eType = NF_SYMBOLTYPE_BLANK;
                sSymbol += OUString(cToken);
                eState = SsGetBlank;
                break;
            case '"':
                eType = NF_SYMBOLTYPE_STRING;
                eState = SsGetString;
                sSymbol += OUString(cToken);
                break;
            case '\\':
                eType = NF_SYMBOLTYPE_STRING;
                eState = SsGetChar;
                sSymbol += OUString(cToken);
                break;
            case '$':
            case '+':
            case '(':
            case ')':
                eType = NF_SYMBOLTYPE_STRING;
                eState = SsStop;
                sSymbol += OUString(cToken);
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
                    sSymbol += OUString(cToken);
                    eState = SsStop;
                }
                else if ( pChrCls->isLetter( rStr, nPos-1 ) )
                {
                    short nTmpType = GetKeyWord( rStr, nPos-1 );
                    if ( nTmpType )
                    {
                        bool bCurrency = false;
                        // "Automatic" currency may start with keyword,
                        // like "R" (Rand) and 'R' (era)
                        if ( nCurrPos >= 0 &&
                             nPos-1 + sCurString.getLength() <= rStr.getLength() &&
                             sCurString.startsWith( sKeyword[nTmpType] ) )
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
                            sSymbol += OUString(cToken);
                        }
                        else
                        {
                            eType = nTmpType;
                            sal_Int32 nLen = sKeyword[eType].getLength();
                            sSymbol = rStr.copy( nPos-1, nLen );
                            if ((eType == NF_KEY_E || IsAmbiguousE(eType)) && nPos < rStr.getLength())
                            {
                                sal_Unicode cNext = rStr[nPos];
                                switch ( cNext )
                                {
                                case '+' :
                                case '-' :  // E+ E- combine to one symbol
                                    sSymbol += OUString(cNext);
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
                        sSymbol += OUString(cToken);
                    }
                }
                else
                {
                    eType = NF_SYMBOLTYPE_STRING;
                    eState = SsStop;
                    sSymbol += OUString(cToken);
                }
                break;
            }
            break;
        case SsGetChar:
            sSymbol += OUString(cToken);
            eState = SsStop;
            break;
        case SsGetString:
            if (cToken == '"')
            {
                eState = SsStop;
            }
            sSymbol += OUString(cToken);
            break;
        case SsGetWord:
            if ( pChrCls->isLetter( rStr, nPos-1 ) )
            {
                short nTmpType = GetKeyWord( rStr, nPos-1 );
                if ( nTmpType )
                {
                    // beginning of keyword, stop scan and put back
                    eType = NF_SYMBOLTYPE_STRING;
                    eState = SsStop;
                    nPos--;
                }
                else
                {
                    sSymbol += OUString(cToken);
                }
            }
            else
            {
                bool bDontStop = false;
                sal_Unicode cNext;
                switch (cToken)
                {
                case '/': // AM/PM, A/P
                    cNext = rStr[nPos];
                    if ( cNext == 'P' || cNext == 'p' )
                    {
                        sal_Int32 nLen = sSymbol.getLength();
                        if ( 1 <= nLen &&
                             (sSymbol[0] == 'A' || sSymbol[0] == 'a') &&
                             (nLen == 1 ||
                              (nLen == 2 && (sSymbol[1] == 'M' || sSymbol[1] == 'm')
                               && (rStr[nPos + 1] == 'M' || rStr[nPos + 1] == 'm'))))
                        {
                            sSymbol += OUString(cToken);
                            bDontStop = true;
                        }
                    }
                    break;
                }
                // anything not recognized will stop the scan
                if ( eState != SsStop && !bDontStop )
                {
                    eState = SsStop;
                    nPos--;
                    eType = NF_SYMBOLTYPE_STRING;
                }
            }
            break;
        case SsGetStar:
            eState = SsStop;
            sSymbol += OUString(cToken);
            break;
        case SsGetBlank:
            eState = SsStop;
            sSymbol += OUString(cToken);
            break;
        default:
            break;
        } // of switch
    } // of while
    if (eState == SsGetWord)
    {
        eType = NF_SYMBOLTYPE_STRING;
    }
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
    nAnzStrings = 0;
    bool bStar = false; // Is set on detecting '*'
    Reset();

    sal_Int32 nPos = 0;
    const sal_Int32 nLen = rString.getLength();
    while (nPos < nLen && nAnzStrings < NF_MAX_FORMAT_SYMBOLS)
    {
        nTypeArray[nAnzStrings] = Next_Symbol(rString, nPos, sStrArray[nAnzStrings]);
        if (nTypeArray[nAnzStrings] == NF_SYMBOLTYPE_STAR)
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
                if (sStrArray[nAnzStrings].getLength() < 2)
                    return nPos;
                bStar = true;
            }
        }
        nAnzStrings++;
    }

    return 0; // 0 => ok
}

void ImpSvNumberformatScan::SkipStrings(sal_uInt16& i, sal_Int32& nPos)
{
    while (i < nAnzStrings && (   nTypeArray[i] == NF_SYMBOLTYPE_STRING
                               || nTypeArray[i] == NF_SYMBOLTYPE_BLANK
                               || nTypeArray[i] == NF_SYMBOLTYPE_STAR) )
    {
        nPos = nPos + sStrArray[i].getLength();
        i++;
    }
}

sal_uInt16 ImpSvNumberformatScan::PreviousKeyword(sal_uInt16 i)
{
    short res = 0;
    if (i > 0 && i < nAnzStrings)
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

sal_uInt16 ImpSvNumberformatScan::NextKeyword(sal_uInt16 i)
{
    short res = 0;
    if (i < nAnzStrings-1)
    {
        i++;
        while (i < nAnzStrings-1 && nTypeArray[i] <= 0)
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

short ImpSvNumberformatScan::PreviousType( sal_uInt16 i )
{
    if ( i > 0 && i < nAnzStrings )
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

sal_Unicode ImpSvNumberformatScan::PreviousChar(sal_uInt16 i)
{
    sal_Unicode res = ' ';
    if (i > 0 && i < nAnzStrings)
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

sal_Unicode ImpSvNumberformatScan::NextChar(sal_uInt16 i)
{
    sal_Unicode res = ' ';
    if (i < nAnzStrings-1)
    {
        i++;
        while (i < nAnzStrings-1 &&
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

bool ImpSvNumberformatScan::IsLastBlankBeforeFrac(sal_uInt16 i)
{
    bool res = true;
    if (i < nAnzStrings-1)
    {
        bool bStop = false;
        i++;
        while (i < nAnzStrings-1 && !bStop)
        {
            i++;
            if ( nTypeArray[i] == NF_SYMBOLTYPE_DEL &&
                 sStrArray[i][0] == '/')
            {
                bStop = true;
            }
            else if ( nTypeArray[i] == NF_SYMBOLTYPE_DEL &&
                      sStrArray[i][0] == ' ')
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
    nAnzStrings = 0;
    nAnzResStrings = 0;
    eScannedType = css::util::NumberFormat::UNDEFINED;
    bExp = false;
    bThousand = false;
    nThousand = 0;
    bDecSep = false;
    nDecPos = (sal_uInt16)-1;
    nExpPos = (sal_uInt16)-1;
    nBlankPos = (sal_uInt16)-1;
    nCntPre = 0;
    nCntPost = 0;
    nCntExp = 0;
    bFrac = false;
    bBlank = false;
    nNatNumModifier = 0;
}

bool ImpSvNumberformatScan::Is100SecZero( sal_uInt16 i, bool bHadDecSep )
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
    short eNewType;
    bool bMatchBracket = false;
    bool bHaveGeneral = false; // if General/Standard encountered

    SkipStrings(i, nPos);
    while (i < nAnzStrings)
    {
        if (nTypeArray[i] > 0)
        {   // keyword
            sal_uInt16 nIndexPre;
            sal_uInt16 nIndexNex;
            sal_Unicode cChar;

            switch (nTypeArray[i])
            {
            case NF_KEY_E:                          // E
                eNewType = css::util::NumberFormat::SCIENTIFIC;
                break;
            case NF_KEY_AMPM:                       // AM,A,PM,P
            case NF_KEY_AP:
            case NF_KEY_H:                          // H
            case NF_KEY_HH:                         // HH
            case NF_KEY_S:                          // S
            case NF_KEY_SS:                         // SS
                eNewType = css::util::NumberFormat::TIME;
                break;
            case NF_KEY_M:                          // M
            case NF_KEY_MM:                         // MM
                /* Minute or month.
                   Minute if one of:
                   * preceded by time keyword H or S (ignoring separators)
                   * followed by time keyword S (ignoring separators)
                   * preceded by '[' amount bracket
                   Else month.
                */
                nIndexPre = PreviousKeyword(i);
                nIndexNex = NextKeyword(i);
                cChar = PreviousChar(i);
                if (nIndexPre == NF_KEY_H   ||      // H
                    nIndexPre == NF_KEY_HH  ||      // HH
                    nIndexPre == NF_KEY_S   ||      // S before M tdf#95339
                    nIndexPre == NF_KEY_SS  ||      // SS
                    nIndexNex == NF_KEY_S   ||      // S
                    nIndexNex == NF_KEY_SS  ||      // SS
                    cChar == '['  )                 // [M
                {
                    eNewType = css::util::NumberFormat::TIME;
                    nTypeArray[i] -= 2;             // 6 -> 4, 7 -> 5
                }
                else
                {
                    eNewType = css::util::NumberFormat::DATE;
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
                eNewType = css::util::NumberFormat::DATE;
                break;
            case NF_KEY_CCC:                        // CCC
                eNewType = css::util::NumberFormat::CURRENCY;
                break;
            case NF_KEY_GENERAL:                    // Standard
                eNewType = css::util::NumberFormat::NUMBER;
                bHaveGeneral = true;
                break;
            default:
                eNewType = css::util::NumberFormat::UNDEFINED;
                break;
            }
        }
        else
        {                                           // control character
            switch ( sStrArray[i][0] )
            {
            case '#':
            case '?':
                eNewType = css::util::NumberFormat::NUMBER;
                break;
            case '0':
                if ( (eScannedType & css::util::NumberFormat::TIME) == css::util::NumberFormat::TIME )
                {
                    if ( Is100SecZero( i, bDecSep ) )
                    {
                        bDecSep = true;                 // subsequent 0's
                        eNewType = css::util::NumberFormat::TIME;
                    }
                    else
                    {
                        return nPos;                    // Error
                    }
                }
                else
                {
                    eNewType = css::util::NumberFormat::NUMBER;
                }
                break;
            case '%':
                eNewType = css::util::NumberFormat::PERCENT;
                break;
            case '/':
                eNewType = css::util::NumberFormat::FRACTION;
                break;
            case '[':
                if ( i < nAnzStrings-1 &&
                     nTypeArray[i+1] == NF_SYMBOLTYPE_STRING &&
                     sStrArray[i+1][0] == '$' )
                {
                    eNewType = css::util::NumberFormat::CURRENCY;
                    bMatchBracket = true;
                }
                else if ( i < nAnzStrings-1 &&
                          nTypeArray[i+1] == NF_SYMBOLTYPE_STRING &&
                          sStrArray[i+1][0] == '~' )
                {
                    eNewType = css::util::NumberFormat::DATE;
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
                        eNewType = css::util::NumberFormat::TIME;
                    else
                    {
                        return nPos;                // Error
                    }
                }
                break;
            case '@':
                eNewType = css::util::NumberFormat::TEXT;
                break;
            default:
                if (pLoc->getTime100SecSep().equals(sStrArray[i]))
                {
                    bDecSep = true;                  // for SS,0
                }
                eNewType = css::util::NumberFormat::UNDEFINED;
                break;
            }
        }
        if (eScannedType == css::util::NumberFormat::UNDEFINED)
        {
            eScannedType = eNewType;
        }
        else if (eScannedType == css::util::NumberFormat::TEXT || eNewType == css::util::NumberFormat::TEXT)
        {
            eScannedType = css::util::NumberFormat::TEXT; // Text always remains text
        }
        else if (eNewType == css::util::NumberFormat::UNDEFINED)
        { // Remains as is
        }
        else if (eScannedType != eNewType)
        {
            switch (eScannedType)
            {
            case css::util::NumberFormat::DATE:
                switch (eNewType)
                {
                case css::util::NumberFormat::TIME:
                    eScannedType = css::util::NumberFormat::DATETIME;
                    break;
                case css::util::NumberFormat::FRACTION:         // DD/MM
                    break;
                default:
                    if (nCurrPos >= 0)
                    {
                        eScannedType = css::util::NumberFormat::UNDEFINED;
                    }
                    else if ( sStrArray[i] != OUString(pFormatter->GetDateSep()) )
                    {
                        return nPos;
                    }
                }
                break;
            case css::util::NumberFormat::TIME:
                switch (eNewType)
                {
                case css::util::NumberFormat::DATE:
                    eScannedType = css::util::NumberFormat::DATETIME;
                    break;
                case css::util::NumberFormat::FRACTION:         // MM/SS
                    break;
                default:
                    if (nCurrPos >= 0)
                    {
                        eScannedType = css::util::NumberFormat::UNDEFINED;
                    }
                    else if (!pLoc->getTimeSep().equals(sStrArray[i]))
                    {
                        return nPos;
                    }
                    break;
                }
                break;
            case css::util::NumberFormat::DATETIME:
                switch (eNewType)
                {
                case css::util::NumberFormat::TIME:
                case css::util::NumberFormat::DATE:
                    break;
                case css::util::NumberFormat::FRACTION:         // DD/MM
                    break;
                default:
                    if (nCurrPos >= 0)
                    {
                        eScannedType = css::util::NumberFormat::UNDEFINED;
                    }
                    else if ( OUString(pFormatter->GetDateSep()) != sStrArray[i] &&
                              !pLoc->getTimeSep().equals(sStrArray[i]) )
                    {
                        return nPos;
                    }
                }
                break;
            case css::util::NumberFormat::PERCENT:
                switch (eNewType)
                {
                case css::util::NumberFormat::NUMBER:   // Only number to percent
                    break;
                default:
                    return nPos;
                }
                break;
            case css::util::NumberFormat::SCIENTIFIC:
                switch (eNewType)
                {
                case css::util::NumberFormat::NUMBER:   // Only number to E
                    break;
                default:
                    return nPos;
                }
                break;
            case css::util::NumberFormat::NUMBER:
                switch (eNewType)
                {
                case css::util::NumberFormat::SCIENTIFIC:
                case css::util::NumberFormat::PERCENT:
                case css::util::NumberFormat::FRACTION:
                case css::util::NumberFormat::CURRENCY:
                    eScannedType = eNewType;
                    break;
                default:
                    if (nCurrPos >= 0)
                    {
                        eScannedType = css::util::NumberFormat::UNDEFINED;
                    }
                    else
                    {
                        return nPos;
                    }
                }
                break;
            case css::util::NumberFormat::FRACTION:
                switch (eNewType)
                {
                case css::util::NumberFormat::NUMBER:   // Only number to fraction
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
            while ( bMatchBracket && i < nAnzStrings )
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

    if ((eScannedType == css::util::NumberFormat::NUMBER ||
         eScannedType == css::util::NumberFormat::UNDEFINED) &&
        nCurrPos >= 0 && !bHaveGeneral)
    {
        eScannedType = css::util::NumberFormat::CURRENCY; // old "automatic" currency
    }
    if (eScannedType == css::util::NumberFormat::UNDEFINED)
    {
        eScannedType = css::util::NumberFormat::DEFINED;
    }
    return 0; // All is fine
}

bool ImpSvNumberformatScan::InsertSymbol( sal_uInt16 & nPos, svt::NfSymbolType eType, const OUString& rStr )
{
    if (nAnzStrings >= NF_MAX_FORMAT_SYMBOLS || nPos > nAnzStrings)
    {
        return false;
    }
    if (nPos > 0 && nTypeArray[nPos-1] == NF_SYMBOLTYPE_EMPTY)
    {
        --nPos; // reuse position
    }
    else
    {
        if ((size_t) (nAnzStrings + 1) >= NF_MAX_FORMAT_SYMBOLS)
        {
            return false;
        }
        ++nAnzStrings;
        for (size_t i = nAnzStrings; i > nPos; --i)
        {
            nTypeArray[i] = nTypeArray[i-1];
            sStrArray[i] = sStrArray[i-1];
        }
    }
    ++nAnzResStrings;
    nTypeArray[nPos] = static_cast<short>(eType);
    sStrArray[nPos] = rStr;
    return true;
}

int ImpSvNumberformatScan::FinalScanGetCalendar( sal_Int32& nPos, sal_uInt16& i,
                                                 sal_uInt16& rAnzResStrings )
{
    if ( i < nAnzStrings-1 &&
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
        rAnzResStrings--;
        if ( ++i >= nAnzStrings )
        {
            return -1; // error
        }
        nPos = nPos + sStrArray[i].getLength();           // calendarID
        OUString& rStr = sStrArray[i];
        nTypeArray[i] = NF_SYMBOLTYPE_CALENDAR;          // convert
        i++;
        while ( i < nAnzStrings && sStrArray[i][0] != ']' )
        {
            nPos = nPos + sStrArray[i].getLength();
            rStr += sStrArray[i];
            nTypeArray[i] = NF_SYMBOLTYPE_EMPTY;
            rAnzResStrings--;
            i++;
        }
        if ( rStr.getLength() && i < nAnzStrings &&
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
    // change locale data et al
    if (bConvertMode)
    {
        pFormatter->ChangeIntl(eNewLnge);
        //! pointer may have changed
        pLoc = pFormatter->GetLocaleData();
        //! init new keywords
        InitKeywords();
    }
    const CharClass* pChrCls = pFormatter->GetCharClass();

    sal_Int32 nPos = 0;                    // error correction position
    sal_uInt16 i = 0;                      // symbol loop counter
    sal_uInt16 nCounter = 0;               // counts digits
    nAnzResStrings = nAnzStrings;          // counts remaining symbols
    bDecSep = false;                       // reset in case already used in TypeCheck
    bool bThaiT = false;                   // Thai T NatNum modifier present
    bool bTimePart = false;

    switch (eScannedType)
    {
    case css::util::NumberFormat::TEXT:
    case css::util::NumberFormat::DEFINED:
        while (i < nAnzStrings)
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

    case css::util::NumberFormat::NUMBER:
    case css::util::NumberFormat::PERCENT:
    case css::util::NumberFormat::CURRENCY:
    case css::util::NumberFormat::SCIENTIFIC:
    case css::util::NumberFormat::FRACTION:
        while (i < nAnzStrings)
        {
            // TODO: rechecking eScannedType is unnecessary.
            // This switch-case is for eScannedType == css::util::NumberFormat::FRACTION anyway
            if (eScannedType == css::util::NumberFormat::FRACTION &&        // special case
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
                if (eScannedType == css::util::NumberFormat::SCIENTIFIC &&
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
                else if (eScannedType == css::util::NumberFormat::FRACTION &&
                         sStrArray[i][0] == ' ')
                {
                    if (!bBlank && !bFrac) // Not double or after a /
                    {
                        if (bDecSep && nCounter > 0) // Decimal places
                        {
                            return nPos; // Error
                        }
                        bBlank = true;
                        nBlankPos = i;
                        nCntPre = nCounter;
                        nCounter = 0;
                    }
                    nTypeArray[i] = NF_SYMBOLTYPE_FRACBLANK;
                }
                else if (nTypeArray[i] == NF_KEY_THAI_T)
                {
                    bThaiT = true;
                    sStrArray[i] = sKeyword[nTypeArray[i]];
                }
                else if (sStrArray[i][0] >= '0' &&
                         sStrArray[i][0] <= '9')
                {
                    OUString sDiv;
                    sal_uInt16 j = i;
                    while(j < nAnzStrings)
                    {
                        sDiv += sStrArray[j++];
                    }
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
                        if ( ( eScannedType != css::util::NumberFormat::FRACTION ) && (!nCntPre) )
                        {
                            nCntPre++;
                        }
                    }
                }
                else
                {
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
                    else if (bFrac && cHere == '0')
                    {
                        return nPos;                    // Denominator is 0
                    }
                    nTypeArray[i] = NF_SYMBOLTYPE_DIGIT;
                    nPos = nPos + rStr.getLength();
                    i++;
                    nCounter++;
                    while (i < nAnzStrings &&
                           (sStrArray[i][0] == '#' ||
                            sStrArray[i][0] == '0' ||
                            sStrArray[i][0] == '?'))
                    {
                        nTypeArray[i] = NF_SYMBOLTYPE_DIGIT;
                        nPos = nPos + sStrArray[i].getLength();
                        nCounter++;
                        i++;
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
                        while (i < nAnzStrings &&
                               (sStrArray[i][0] == '-') )
                        {
                            // If more than two dashes are present in
                            // currency formats the last dash will be
                            // interpreted literally as a minus sign.
                            // Has to be this ugly. Period.
                            if ( eScannedType == css::util::NumberFormat::CURRENCY
                                 && rStr.getLength() >= 2 &&
                                 (i == nAnzStrings-1 ||
                                  sStrArray[i+1][0] != '-') )
                            {
                                break;
                            }
                            rStr += sStrArray[i];
                            nPos = nPos + sStrArray[i].getLength();
                            nTypeArray[i] = NF_SYMBOLTYPE_EMPTY;
                            nAnzResStrings--;
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
                                nAnzResStrings--;
                                i++; // eat it
                            }
                            else
                            {
                                nTypeArray[i] = NF_SYMBOLTYPE_STRING;
                            }
                        }
                        else if (i > 0 && i < nAnzStrings-1   &&
                                 (cPre == '#' || cPre == '0')      &&
                                 ((cNext = NextChar(i)) == '#' || cNext == '0')) // #,#
                        {
                            nPos = nPos + sStrArray[i].getLength();
                            if (!bThousand) // only once
                            {
                                bThousand = true;
                            }
                            // Eat it, will be reinserted at proper grouping positions further down.
                            nTypeArray[i] = NF_SYMBOLTYPE_EMPTY;
                            nAnzResStrings--;
                            i++;
                        }
                        else if (i > 0 && (cPre == '#' || cPre == '0')
                                 && PreviousType(i) == NF_SYMBOLTYPE_DIGIT
                                 && nThousand < FLAG_STANDARD_IN_FORMAT )
                        {   // #,,,,
                            if ( StringEqualsChar( sOldThousandSep, ' ' ) )
                            {
                                // strange, those French..
                                bool bFirst = true;
                                //  set a hard No-Break Space or ConvertMode
                                const OUString& rSepF = pFormatter->GetNumThousandSep();
                                while ( i < nAnzStrings &&
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
                                        nAnzResStrings--;
                                    }
                                    nThousand++;
                                    i++;
                                }
                                if ( i < nAnzStrings-1 &&
                                     sStrArray[i] == sOldThousandSep )
                                {
                                    // something following last space
                                    // => space if currency contained,
                                    // else separator
                                    nPos = nPos + sStrArray[i].getLength();
                                    if ( (nPos <= nCurrPos &&
                                          nCurrPos < nPos + sStrArray[i+1].getLength()) ||
                                         nTypeArray[i+1] == NF_KEY_CCC ||
                                         (i < nAnzStrings-2 &&
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
                                            nAnzResStrings--;
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
                                while (i < nAnzStrings && sStrArray[i] == sOldThousandSep);
                            }
                        }
                        else // any grsep
                        {
                            nTypeArray[i] = NF_SYMBOLTYPE_STRING;
                            nPos = nPos + rStr.getLength();
                            i++;
                            while ( i < nAnzStrings && sStrArray[i] == sOldThousandSep )
                            {
                                rStr += sStrArray[i];
                                nPos = nPos + sStrArray[i].getLength();
                                nTypeArray[i] = NF_SYMBOLTYPE_EMPTY;
                                nAnzResStrings--;
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
                            nAnzResStrings--;
                            i++;                // eat it
                        }
                        else if (bDecSep)       // any .
                        {
                            nTypeArray[i] = NF_SYMBOLTYPE_STRING;
                            nPos = nPos + rStr.getLength();
                            i++;
                            while ( i < nAnzStrings && sStrArray[i] == sOldDecSep )
                            {
                                rStr += sStrArray[i];
                                nPos = nPos + sStrArray[i].getLength();
                                nTypeArray[i] = NF_SYMBOLTYPE_EMPTY;
                                nAnzResStrings--;
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
                            eScannedType == css::util::NumberFormat::FRACTION &&
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
                            nTypeArray[i] = NF_SYMBOLTYPE_STRING;
                            nPos = nPos + sStrArray[i].getLength();
                        }
                        else
                        {
                            nTypeArray[i] = NF_SYMBOLTYPE_STRING;
                            nPos = nPos + rStr.getLength();
                            i++;
                            while (i < nAnzStrings && StringEqualsChar( sStrArray[i], cSaved ) )
                            {
                                rStr += sStrArray[i];
                                nPos = nPos + sStrArray[i].getLength();
                                nTypeArray[i] = NF_SYMBOLTYPE_EMPTY;
                                nAnzResStrings--;
                                i++;
                            }
                        }
                    }
                    break;
                case '/':
                    if (eScannedType == css::util::NumberFormat::FRACTION)
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
                    if ( eScannedType == css::util::NumberFormat::CURRENCY &&
                         i < nAnzStrings-1 &&
                         nTypeArray[i+1] == NF_SYMBOLTYPE_STRING &&
                         sStrArray[i+1][0] == '$' )
                    {
                        // [$DM-xxx]
                        nPos = nPos + sStrArray[i].getLength();     // [
                        nTypeArray[i] = NF_SYMBOLTYPE_CURRDEL;
                        nPos = nPos + sStrArray[++i].getLength();   // $
                        sStrArray[i-1] += sStrArray[i];             // [$
                        nTypeArray[i] = NF_SYMBOLTYPE_EMPTY;
                        nAnzResStrings--;
                        if ( ++i >= nAnzStrings )
                        {
                            return nPos; // Error
                        }
                        nPos = nPos + sStrArray[i].getLength();     // DM
                        OUString* pStr = &sStrArray[i];
                        nTypeArray[i] = NF_SYMBOLTYPE_CURRENCY; // convert
                        bool bHadDash = false;
                        i++;
                        while ( i < nAnzStrings && sStrArray[i][0] != ']' )
                        {
                            nPos = nPos + sStrArray[i].getLength();
                            if ( bHadDash )
                            {
                                *pStr += sStrArray[i];
                                nTypeArray[i] = NF_SYMBOLTYPE_EMPTY;
                                nAnzResStrings--;
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
                                    nAnzResStrings--;
                                }
                            }
                            i++;
                        }
                        if ( rStr.getLength() && i < nAnzStrings && sStrArray[i][0] == ']' )
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
                    if (eScannedType == css::util::NumberFormat::PERCENT && cHere == '%')
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
        if (eScannedType == css::util::NumberFormat::FRACTION)
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
                nAnzResStrings--;
            }
        }
        // Combine digits into groups to save memory (Info will be copied
        // later, taking only non-empty symbols).
        for (i = 0; i < nAnzStrings; ++i)
        {
            if (nTypeArray[i] == NF_SYMBOLTYPE_DIGIT)
            {
                OUString& rStr = sStrArray[i];
                while (++i < nAnzStrings && nTypeArray[i] == NF_SYMBOLTYPE_DIGIT)
                {
                    rStr += sStrArray[i];
                    nTypeArray[i] = NF_SYMBOLTYPE_EMPTY;
                    nAnzResStrings--;
                }
            }
        }
        break; // of css::util::NumberFormat::NUMBER
    case css::util::NumberFormat::DATE:
        while (i < nAnzStrings)
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
                else if ( (nCalRet = FinalScanGetCalendar( nPos, i, nAnzResStrings )) != 0 )
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
                SAL_FALLTHROUGH;
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
                i++;
                break;
            default: // Other keywords
                nTypeArray[i] = NF_SYMBOLTYPE_STRING;
                nPos = nPos + sStrArray[i].getLength();
                i++;
                break;
            }
        } // of while
        break; // of css::util::NumberFormat::DATE
    case css::util::NumberFormat::TIME:
        while (i < nAnzStrings)
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
                        i++;
                        nPos = nPos + sStrArray[i].getLength();
                        nCounter++;
                        while (i < nAnzStrings &&
                               sStrArray[i][0] == '0')
                        {
                            rStr += sStrArray[i];
                            nPos = nPos + sStrArray[i].getLength();
                            nTypeArray[i] = NF_SYMBOLTYPE_EMPTY;
                            nAnzResStrings--;
                            nCounter++;
                            i++;
                        }
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
                SAL_FALLTHROUGH;
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
        break;                                 // of css::util::NumberFormat::TIME
    case css::util::NumberFormat::DATETIME:
        while (i < nAnzStrings)
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
                if ( (nCalRet = FinalScanGetCalendar( nPos, i, nAnzResStrings )) != 0 )
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
                        if ( bTimePart && Is100SecZero( i, bDecSep ) )
                        {
                            bDecSep = true;
                            nTypeArray[i] = NF_SYMBOLTYPE_DIGIT;
                            OUString& rStr = sStrArray[i];
                            i++;
                            nPos = nPos + sStrArray[i].getLength();
                            nCounter++;
                            while (i < nAnzStrings &&
                                   sStrArray[i][0] == '0')
                            {
                                rStr += sStrArray[i];
                                nPos = nPos + sStrArray[i].getLength();
                                nTypeArray[i] = NF_SYMBOLTYPE_EMPTY;
                                nAnzResStrings--;
                                nCounter++;
                                i++;
                            }
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
        break; // of css::util::NumberFormat::DATETIME
    default:
        break;
    }
    if (eScannedType == css::util::NumberFormat::SCIENTIFIC &&
        (nCntPre + nCntPost == 0 || nCntExp == 0))
    {
        return nPos;
    }
    else if (eScannedType == css::util::NumberFormat::FRACTION && (nCntExp > 8 || nCntExp == 0))
    {
        return nPos;
    }
    if (bThaiT && !GetNatNumModifier())
    {
        SetNatNumModifier(1);
    }
    if ( bConvertMode )
    {
        // strings containing keywords of the target locale must be quoted, so
        // the user sees the difference and is able to edit the format string
        for ( i=0; i < nAnzStrings; i++ )
        {
            if ( nTypeArray[i] == NF_SYMBOLTYPE_STRING &&
                 sStrArray[i][0] != '\"' )
            {
                if ( bConvertSystemToSystem && eScannedType == css::util::NumberFormat::CURRENCY )
                {
                    // don't stringize automatic currency, will be converted
                    if ( sStrArray[i] == sOldCurSymbol )
                    {
                        continue; // for
                    }
                    // DM might be splitted into D and M
                    if ( sStrArray[i].getLength() < sOldCurSymbol.getLength() &&
                         pChrCls->uppercase( sStrArray[i], 0, 1 )[0] ==
                         sOldCurString[0] )
                    {
                        OUString aTmp( sStrArray[i] );
                        sal_uInt16 j = i + 1;
                        while ( aTmp.getLength() < sOldCurSymbol.getLength() &&
                                j < nAnzStrings &&
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
                                nAnzResStrings--;
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
                    if ( (j == 0 || rStr[j - 1] != '\\') && GetKeyWord( rStr, j ) )
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
    while (i < nAnzStrings)
    {
        sal_Int32 nStringPos;
        sal_Int32 nArrPos = 0;
        sal_uInt16 iPos = i;
        switch ( nTypeArray[i] )
        {
        case NF_SYMBOLTYPE_STRING :
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
                        rString += OUString(c);
                        break;
                    case ' ':
                    case '.':
                    case '/':
                        if (((eScannedType & css::util::NumberFormat::DATE) == 0) &&
                            (StringEqualsChar( pFormatter->GetNumThousandSep(), c) ||
                             StringEqualsChar( pFormatter->GetNumDecimalSep(), c) ||
                             (c == ' ' &&
                              (StringEqualsChar( pFormatter->GetNumThousandSep(), cNoBreakSpace) ||
                               StringEqualsChar( pFormatter->GetNumThousandSep(), cNarrowNoBreakSpace)))))
                        {
                            rString += sStrArray[i];
                        }
                        else if ((eScannedType & css::util::NumberFormat::DATE) &&
                                 StringEqualsChar( pFormatter->GetDateSep(), c))
                        {
                            rString += sStrArray[i];
                        }
                        else if ((eScannedType & css::util::NumberFormat::TIME) &&
                                 (StringEqualsChar( pLoc->getTimeSep(), c) ||
                                  StringEqualsChar( pLoc->getTime100SecSep(), c)))
                        {
                            rString += sStrArray[i];
                        }
                        else if (eScannedType & css::util::NumberFormat::FRACTION)
                        {
                            rString += sStrArray[i];
                        }
                        else
                        {
                            rString += OUString(c);
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
                    if ( eScannedType == css::util::NumberFormat::CURRENCY )
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
                    nAnzResStrings--;
                }
                i++;
            }
            while ( i < nAnzStrings && nTypeArray[i] == NF_SYMBOLTYPE_STRING );

            if ( i < nAnzStrings )
            {
                i--; // enter switch on next symbol again
            }
            if ( eScannedType == css::util::NumberFormat::CURRENCY && nStringPos < rString.getLength() )
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
                nAnzResStrings--;
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

void ImpSvNumberformatScan::CopyInfo(ImpSvNumberformatInfo* pInfo, sal_uInt16 nAnz)
{
    size_t i,j;
    j = 0;
    i = 0;
    while (i < nAnz && j < NF_MAX_FORMAT_SYMBOLS)
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
