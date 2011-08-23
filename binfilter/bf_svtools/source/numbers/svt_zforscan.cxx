/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// MARKER(update_precomp.py): autogen include statement, do not remove

#include <stdlib.h>

#include <tools/debug.hxx>
#include <i18npool/mslangid.hxx>
#include <unotools/charclass.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/numberformatcodewrapper.hxx>
#include <rtl/instance.hxx>

#include <bf_svtools/zforlist.hxx>
#include <bf_svtools/zformat.hxx>

#define _ZFORSCAN_CXX
#include "zforscan.hxx"
#undef _ZFORSCAN_CXX

#include "nfsymbol.hxx"

namespace binfilter
{

const sal_Unicode cNonBreakingSpace = 0xA0;

namespace 
{
    struct ImplEnglishColors
    {
        const String* operator()()
        {
            static const String aEnglishColors[NF_MAX_DEFAULT_COLORS] =
            {
                String( RTL_CONSTASCII_USTRINGPARAM( "BLACK" ) ),
                String( RTL_CONSTASCII_USTRINGPARAM( "BLUE" ) ),
                String( RTL_CONSTASCII_USTRINGPARAM( "GREEN" ) ),
                String( RTL_CONSTASCII_USTRINGPARAM( "CYAN" ) ),
                String( RTL_CONSTASCII_USTRINGPARAM( "RED" ) ),
                String( RTL_CONSTASCII_USTRINGPARAM( "MAGENTA" ) ),
                String( RTL_CONSTASCII_USTRINGPARAM( "BROWN" ) ),
                String( RTL_CONSTASCII_USTRINGPARAM( "GREY" ) ),
                String( RTL_CONSTASCII_USTRINGPARAM( "YELLOW" ) ),
                String( RTL_CONSTASCII_USTRINGPARAM( "WHITE" ) )
            };
            return &aEnglishColors[0];
        }
    };

    struct theEnglishColors
            : public rtl::StaticAggregate< const String, ImplEnglishColors> {};

}

ImpSvNumberformatScan::ImpSvNumberformatScan( SvNumberFormatter* pFormatterP )
{
    pFormatter = pFormatterP;
    bConvertMode = FALSE;
    //! All keywords MUST be UPPERCASE!
    sKeyword[NF_KEY_E].AssignAscii( RTL_CONSTASCII_STRINGPARAM( 	"E" ) );		// Exponent
    sKeyword[NF_KEY_AMPM].AssignAscii( RTL_CONSTASCII_STRINGPARAM( 	"AM/PM" ) );	// AM/PM
    sKeyword[NF_KEY_AP].AssignAscii( RTL_CONSTASCII_STRINGPARAM( 	"A/P" ) );		// AM/PM short
    sKeyword[NF_KEY_MI].AssignAscii( RTL_CONSTASCII_STRINGPARAM( 	"M" ) );		// Minute
    sKeyword[NF_KEY_MMI].AssignAscii( RTL_CONSTASCII_STRINGPARAM( 	"MM" ) );		// Minute 02
    sKeyword[NF_KEY_S].AssignAscii( RTL_CONSTASCII_STRINGPARAM( 	"S" ) );		// Second
    sKeyword[NF_KEY_SS].AssignAscii( RTL_CONSTASCII_STRINGPARAM( 	"SS" ) );		// Second 02
    sKeyword[NF_KEY_Q].AssignAscii( RTL_CONSTASCII_STRINGPARAM( 	"Q" ) );		// Quarter short 'Q'
    sKeyword[NF_KEY_QQ].AssignAscii( RTL_CONSTASCII_STRINGPARAM( 	"QQ" ) );		// Quarter long
    sKeyword[NF_KEY_NN].AssignAscii( RTL_CONSTASCII_STRINGPARAM( 	"NN" ) );		// Day of week short
    sKeyword[NF_KEY_NNN].AssignAscii( RTL_CONSTASCII_STRINGPARAM( 	"NNN" ) );		// Day of week long
    sKeyword[NF_KEY_NNNN].AssignAscii( RTL_CONSTASCII_STRINGPARAM( 	"NNNN" ) );		// Day of week long incl. separator
    sKeyword[NF_KEY_WW].AssignAscii( RTL_CONSTASCII_STRINGPARAM( 	"WW" ) );		// Week of year
    sKeyword[NF_KEY_CCC].AssignAscii( RTL_CONSTASCII_STRINGPARAM( 	"CCC" ) );		// Currency abbreviation
    bKeywordsNeedInit = TRUE;   // locale dependent keywords
    bCompatCurNeedInit = TRUE;  // locale dependent compatibility currency strings

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

    sErrStr.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "###" ) );
    Reset();
}

ImpSvNumberformatScan::~ImpSvNumberformatScan()
{
    delete pNullDate;
    Reset();
}


void ImpSvNumberformatScan::ChangeIntl()
{
    bKeywordsNeedInit = TRUE;
    bCompatCurNeedInit = TRUE;
    // may be initialized by InitSpecialKeyword()
    sKeyword[NF_KEY_TRUE].Erase();
    sKeyword[NF_KEY_FALSE].Erase();
}


void ImpSvNumberformatScan::InitSpecialKeyword( NfKeywordIndex eIdx ) const
{
    switch ( eIdx )
    {
        case NF_KEY_TRUE :
            ((ImpSvNumberformatScan*)this)->sKeyword[NF_KEY_TRUE] =
                pFormatter->GetCharClass()->upper(
                pFormatter->GetLocaleData()->getTrueWord() );
            if ( !sKeyword[NF_KEY_TRUE].Len() )
            {
                DBG_ERRORFILE( "InitSpecialKeyword: TRUE_WORD?" );
                ((ImpSvNumberformatScan*)this)->sKeyword[NF_KEY_TRUE].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "TRUE" ) );
            }
        break;
        case NF_KEY_FALSE :
            ((ImpSvNumberformatScan*)this)->sKeyword[NF_KEY_FALSE] =
                pFormatter->GetCharClass()->upper(
                pFormatter->GetLocaleData()->getFalseWord() );
            if ( !sKeyword[NF_KEY_FALSE].Len() )
            {
                DBG_ERRORFILE( "InitSpecialKeyword: FALSE_WORD?" );
                ((ImpSvNumberformatScan*)this)->sKeyword[NF_KEY_FALSE].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "FALSE" ) );
            }
        break;
        default:
            DBG_ERRORFILE( "InitSpecialKeyword: unknown request" );
    }
}


void ImpSvNumberformatScan::InitCompatCur() const
{
    ImpSvNumberformatScan* pThis = (ImpSvNumberformatScan*)this;
    // currency symbol for old style ("automatic") compatibility format codes
    pFormatter->GetCompatibilityCurrency( pThis->sCurSymbol, pThis->sCurAbbrev );
    // currency symbol upper case
    pThis->sCurString = pFormatter->GetCharClass()->upper( sCurSymbol );
    bCompatCurNeedInit = FALSE;
}


void ImpSvNumberformatScan::InitKeywords() const
{
    if ( !bKeywordsNeedInit )
        return ;
    ((ImpSvNumberformatScan*)this)->SetDependentKeywords();
    bKeywordsNeedInit = FALSE;
}


void ImpSvNumberformatScan::SetDependentKeywords()
{
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;

    const CharClass* pCharClass = pFormatter->GetCharClass();
    const LocaleDataWrapper* pLocaleData = pFormatter->GetLocaleData();
    // #80023# be sure to generate keywords for the loaded Locale, not for the
    // requested Locale, otherwise number format codes might not match
    lang::Locale aLoadedLocale = pLocaleData->getLoadedLocale();
    LanguageType eLang = MsLangId::convertLocaleToLanguage( aLoadedLocale );
    NumberFormatCodeWrapper aNumberFormatCode( pFormatter->GetServiceManager(), aLoadedLocale );

    i18n::NumberFormatCode aFormat = aNumberFormatCode.getFormatCode( NF_NUMBER_STANDARD );
    sNameStandardFormat = aFormat.Code;
    sKeyword[NF_KEY_GENERAL] = pCharClass->upper( sNameStandardFormat );

    // preset new calendar keywords
    sKeyword[NF_KEY_AAA].AssignAscii( RTL_CONSTASCII_STRINGPARAM(	"AAA" ) );
    sKeyword[NF_KEY_AAAA].AssignAscii( RTL_CONSTASCII_STRINGPARAM(	"AAAA" ) );
    sKeyword[NF_KEY_EC].AssignAscii( RTL_CONSTASCII_STRINGPARAM(	"E" ) );
    sKeyword[NF_KEY_EEC].AssignAscii( RTL_CONSTASCII_STRINGPARAM(	"EE" ) );
    sKeyword[NF_KEY_G].AssignAscii( RTL_CONSTASCII_STRINGPARAM(		"G" ) );
    sKeyword[NF_KEY_GG].AssignAscii( RTL_CONSTASCII_STRINGPARAM(	"GG" ) );
    sKeyword[NF_KEY_GGG].AssignAscii( RTL_CONSTASCII_STRINGPARAM(	"GGG" ) );
    sKeyword[NF_KEY_R].AssignAscii( RTL_CONSTASCII_STRINGPARAM(		"R" ) );
    sKeyword[NF_KEY_RR].AssignAscii( RTL_CONSTASCII_STRINGPARAM(	"RR" ) );

    // Thai T NatNum special. Other locale's small letter 't' results in upper
    // case comparison not matching but length does in conversion mode. Ugly.
    if (eLang == LANGUAGE_THAI)
        sKeyword[NF_KEY_THAI_T].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "T"));
    else
        sKeyword[NF_KEY_THAI_T].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "t"));

    switch ( eLang )
    {
        case LANGUAGE_GERMAN:
        case LANGUAGE_GERMAN_SWISS:
        case LANGUAGE_GERMAN_AUSTRIAN:
        case LANGUAGE_GERMAN_LUXEMBOURG:
        case LANGUAGE_GERMAN_LIECHTENSTEIN:
        {
            //! all capital letters
            sKeyword[NF_KEY_M].AssignAscii( RTL_CONSTASCII_STRINGPARAM( 		"M" ) ); 			// month 1
            sKeyword[NF_KEY_MM].AssignAscii( RTL_CONSTASCII_STRINGPARAM( 		"MM" ) );			// month 01
            sKeyword[NF_KEY_MMM].AssignAscii( RTL_CONSTASCII_STRINGPARAM( 		"MMM" ) );		// month Jan
            sKeyword[NF_KEY_MMMM].AssignAscii( RTL_CONSTASCII_STRINGPARAM( 		"MMMM" ) );	// month Januar
            sKeyword[NF_KEY_MMMMM].AssignAscii( RTL_CONSTASCII_STRINGPARAM( 	"MMMMM" ) );// month J
            sKeyword[NF_KEY_H].AssignAscii( RTL_CONSTASCII_STRINGPARAM( 		"H" ) ); 			// hour 2
            sKeyword[NF_KEY_HH].AssignAscii( RTL_CONSTASCII_STRINGPARAM( 		"HH" ) );			// hour 02
            sKeyword[NF_KEY_D].AssignAscii( RTL_CONSTASCII_STRINGPARAM( 		"T" ) );
            sKeyword[NF_KEY_DD].AssignAscii( RTL_CONSTASCII_STRINGPARAM( 		"TT" ) );
            sKeyword[NF_KEY_DDD].AssignAscii( RTL_CONSTASCII_STRINGPARAM( 		"TTT" ) );
            sKeyword[NF_KEY_DDDD].AssignAscii( RTL_CONSTASCII_STRINGPARAM( 		"TTTT" ) );
            sKeyword[NF_KEY_YY].AssignAscii( RTL_CONSTASCII_STRINGPARAM( 		"JJ" ) );
            sKeyword[NF_KEY_YYYY].AssignAscii( RTL_CONSTASCII_STRINGPARAM( 		"JJJJ" ) );
            sKeyword[NF_KEY_BOOLEAN].AssignAscii( RTL_CONSTASCII_STRINGPARAM( 	"LOGISCH" ) );
            sKeyword[NF_KEY_COLOR].AssignAscii( RTL_CONSTASCII_STRINGPARAM( 	"FARBE" ) );
            sKeyword[NF_KEY_BLACK].AssignAscii( RTL_CONSTASCII_STRINGPARAM( 	"SCHWARZ" ) );
            sKeyword[NF_KEY_BLUE].AssignAscii( RTL_CONSTASCII_STRINGPARAM( 		"BLAU" ) );
            sKeyword[NF_KEY_GREEN] = UniString( "GR" "\xDC" "N", RTL_TEXTENCODING_ISO_8859_1 );
            sKeyword[NF_KEY_CYAN].AssignAscii( RTL_CONSTASCII_STRINGPARAM( 		"CYAN" ) );
            sKeyword[NF_KEY_RED].AssignAscii( RTL_CONSTASCII_STRINGPARAM( 		"ROT" ) );
            sKeyword[NF_KEY_MAGENTA].AssignAscii( RTL_CONSTASCII_STRINGPARAM( 	"MAGENTA" ) );
            sKeyword[NF_KEY_BROWN].AssignAscii( RTL_CONSTASCII_STRINGPARAM( 	"BRAUN" ) );
            sKeyword[NF_KEY_GREY].AssignAscii( RTL_CONSTASCII_STRINGPARAM( 		"GRAU" ) );
            sKeyword[NF_KEY_YELLOW].AssignAscii( RTL_CONSTASCII_STRINGPARAM( 	"GELB" ) );
            sKeyword[NF_KEY_WHITE].AssignAscii( RTL_CONSTASCII_STRINGPARAM( 	"WEISS" ) );
        }
        break;
        default:
        {
            // day
            switch ( eLang )
            {
                case LANGUAGE_ITALIAN       :
                case LANGUAGE_ITALIAN_SWISS :
                    sKeyword[NF_KEY_D].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "G" ) );
                    sKeyword[NF_KEY_DD].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "GG" ) );
                    sKeyword[NF_KEY_DDD].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "GGG" ) );
                    sKeyword[NF_KEY_DDDD].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "GGGG" ) );
                    // must exchange the era code, same as Xcl
                    sKeyword[NF_KEY_G].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "X" ) );
                    sKeyword[NF_KEY_GG].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "XX" ) );
                    sKeyword[NF_KEY_GGG].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "XXX" ) );
                break;
                case LANGUAGE_FRENCH            :
                case LANGUAGE_FRENCH_BELGIAN    :
                case LANGUAGE_FRENCH_CANADIAN   :
                case LANGUAGE_FRENCH_SWISS      :
                case LANGUAGE_FRENCH_LUXEMBOURG :
                case LANGUAGE_FRENCH_MONACO		:
                    sKeyword[NF_KEY_D].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "J" ) );
                    sKeyword[NF_KEY_DD].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "JJ" ) );
                    sKeyword[NF_KEY_DDD].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "JJJ" ) );
                    sKeyword[NF_KEY_DDDD].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "JJJJ" ) );
                break;
                case LANGUAGE_FINNISH :
                    sKeyword[NF_KEY_D].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "P" ) );
                    sKeyword[NF_KEY_DD].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "PP" ) );
                    sKeyword[NF_KEY_DDD].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "PPP" ) );
                    sKeyword[NF_KEY_DDDD].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "PPPP" ) );
                break;
                default:
                    sKeyword[NF_KEY_D].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "D" ) );
                    sKeyword[NF_KEY_DD].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "DD" ) );
                    sKeyword[NF_KEY_DDD].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "DDD" ) );
                    sKeyword[NF_KEY_DDDD].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "DDDD" ) );
            }
            // month
            switch ( eLang )
            {
                case LANGUAGE_FINNISH :
                    sKeyword[NF_KEY_M].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "K" ) );
                    sKeyword[NF_KEY_MM].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "KK" ) );
                    sKeyword[NF_KEY_MMM].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "KKK" ) );
                    sKeyword[NF_KEY_MMMM].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "KKKK" ) );
                    sKeyword[NF_KEY_MMMMM].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "KKKKK" ) );
                break;
                default:
                    sKeyword[NF_KEY_M].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "M" ) );
                    sKeyword[NF_KEY_MM].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "MM" ) );
                    sKeyword[NF_KEY_MMM].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "MMM" ) );
                    sKeyword[NF_KEY_MMMM].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "MMMM" ) );
                    sKeyword[NF_KEY_MMMMM].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "MMMMM" ) );
            }
            // year
            switch ( eLang )
            {
                case LANGUAGE_ITALIAN       :
                case LANGUAGE_ITALIAN_SWISS :
                case LANGUAGE_FRENCH            :
                case LANGUAGE_FRENCH_BELGIAN    :
                case LANGUAGE_FRENCH_CANADIAN   :
                case LANGUAGE_FRENCH_SWISS      :
                case LANGUAGE_FRENCH_LUXEMBOURG :
                case LANGUAGE_FRENCH_MONACO		:
                case LANGUAGE_PORTUGUESE           :
                case LANGUAGE_PORTUGUESE_BRAZILIAN :
                case LANGUAGE_SPANISH_MODERN      :
                case LANGUAGE_SPANISH_DATED       :
                case LANGUAGE_SPANISH_MEXICAN     :
                case LANGUAGE_SPANISH_GUATEMALA   :
                case LANGUAGE_SPANISH_COSTARICA   :
                case LANGUAGE_SPANISH_PANAMA      :
                case LANGUAGE_SPANISH_DOMINICAN_REPUBLIC :
                case LANGUAGE_SPANISH_VENEZUELA   :
                case LANGUAGE_SPANISH_COLOMBIA    :
                case LANGUAGE_SPANISH_PERU        :
                case LANGUAGE_SPANISH_ARGENTINA   :
                case LANGUAGE_SPANISH_ECUADOR     :
                case LANGUAGE_SPANISH_CHILE       :
                case LANGUAGE_SPANISH_URUGUAY     :
                case LANGUAGE_SPANISH_PARAGUAY    :
                case LANGUAGE_SPANISH_BOLIVIA     :
                case LANGUAGE_SPANISH_EL_SALVADOR :
                case LANGUAGE_SPANISH_HONDURAS    :
                case LANGUAGE_SPANISH_NICARAGUA   :
                case LANGUAGE_SPANISH_PUERTO_RICO :
                    sKeyword[NF_KEY_YY].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "AA" ) );
                    sKeyword[NF_KEY_YYYY].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "AAAA" ) );
                    // must exchange the day of week name code, same as Xcl
                    sKeyword[NF_KEY_AAA].AssignAscii( RTL_CONSTASCII_STRINGPARAM(	"OOO" ) );
                    sKeyword[NF_KEY_AAAA].AssignAscii( RTL_CONSTASCII_STRINGPARAM(	"OOOO" ) );
                break;
                case LANGUAGE_DUTCH         :
                case LANGUAGE_DUTCH_BELGIAN :
                    sKeyword[NF_KEY_YY].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "JJ" ) );
                    sKeyword[NF_KEY_YYYY].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "JJJJ" ) );
                break;
                case LANGUAGE_FINNISH :
                    sKeyword[NF_KEY_YY].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "VV" ) );
                    sKeyword[NF_KEY_YYYY].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "VVVV" ) );
                break;
                default:
                    sKeyword[NF_KEY_YY].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "YY" ) );
                    sKeyword[NF_KEY_YYYY].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "YYYY" ) );
            }
            // hour
            switch ( eLang )
            {
                case LANGUAGE_DUTCH         :
                case LANGUAGE_DUTCH_BELGIAN :
                    sKeyword[NF_KEY_H].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "U" ) );
                    sKeyword[NF_KEY_HH].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "UU" ) );
                break;
                case LANGUAGE_FINNISH :
                case LANGUAGE_SWEDISH         :
                case LANGUAGE_SWEDISH_FINLAND :
                case LANGUAGE_DANISH :
                case LANGUAGE_NORWEGIAN         :
                case LANGUAGE_NORWEGIAN_BOKMAL  :
                case LANGUAGE_NORWEGIAN_NYNORSK :
                    sKeyword[NF_KEY_H].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "T" ) );
                    sKeyword[NF_KEY_HH].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "TT" ) );
                break;
                default:
                    sKeyword[NF_KEY_H].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "H" ) );
                    sKeyword[NF_KEY_HH].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "HH" ) );
            }
            // boolean
            sKeyword[NF_KEY_BOOLEAN].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "BOOLEAN" ) );
            // colours
            sKeyword[NF_KEY_COLOR].AssignAscii( RTL_CONSTASCII_STRINGPARAM( 	"COLOR" ) );
            sKeyword[NF_KEY_BLACK].AssignAscii( RTL_CONSTASCII_STRINGPARAM( 	"BLACK" ) );
            sKeyword[NF_KEY_BLUE].AssignAscii( RTL_CONSTASCII_STRINGPARAM( 		"BLUE" ) );
            sKeyword[NF_KEY_GREEN].AssignAscii( RTL_CONSTASCII_STRINGPARAM( 	"GREEN" ) );
            sKeyword[NF_KEY_CYAN].AssignAscii( RTL_CONSTASCII_STRINGPARAM( 		"CYAN" ) );
            sKeyword[NF_KEY_RED].AssignAscii( RTL_CONSTASCII_STRINGPARAM( 		"RED" ) );
            sKeyword[NF_KEY_MAGENTA].AssignAscii( RTL_CONSTASCII_STRINGPARAM( 	"MAGENTA" ) );
            sKeyword[NF_KEY_BROWN].AssignAscii( RTL_CONSTASCII_STRINGPARAM( 	"BROWN" ) );
            sKeyword[NF_KEY_GREY].AssignAscii( RTL_CONSTASCII_STRINGPARAM( 		"GREY" ) );
            sKeyword[NF_KEY_YELLOW].AssignAscii( RTL_CONSTASCII_STRINGPARAM( 	"YELLOW" ) );
            sKeyword[NF_KEY_WHITE].AssignAscii( RTL_CONSTASCII_STRINGPARAM( 	"WHITE" ) );
        }
        break;
    }

    // boolean keyords
    InitSpecialKeyword( NF_KEY_TRUE );
    InitSpecialKeyword( NF_KEY_FALSE );

    // compatibility currency strings
    InitCompatCur();
}


void ImpSvNumberformatScan::ChangeNullDate(USHORT nDay, USHORT nMonth, USHORT nYear)
{
    if ( pNullDate )
        *pNullDate = Date(nDay, nMonth, nYear);
    else
        pNullDate = new Date(nDay, nMonth, nYear);
}

void ImpSvNumberformatScan::ChangeStandardPrec(short nPrec)
{
    nStandardPrec = nPrec;
}

Color* ImpSvNumberformatScan::GetColor(String& sStr)
{
    String sString = pFormatter->GetCharClass()->upper(sStr);
    const String* pKeyword = GetKeywords();
    size_t i = 0;
    while (i < NF_MAX_DEFAULT_COLORS &&
           sString != pKeyword[NF_KEY_FIRSTCOLOR+i] )
        i++;
    if ( i >= NF_MAX_DEFAULT_COLORS )
    {
        const String* pEnglishColors = theEnglishColors::get();
        size_t j = 0;
        while ( j < NF_MAX_DEFAULT_COLORS &&
                sString != pEnglishColors[j] )
            ++j;
        if ( j < NF_MAX_DEFAULT_COLORS )
            i = j;
    }
    if (i >= NF_MAX_DEFAULT_COLORS)
    {
        const String& rColorWord = pKeyword[NF_KEY_COLOR];
        xub_StrLen nPos = sString.Match(rColorWord);
        if (nPos > 0)
        {
            sStr.Erase(0, nPos);
            sStr.EraseLeadingChars();
            sStr.EraseTrailingChars();
            if (bConvertMode)
            {
                pFormatter->ChangeIntl(eNewLnge);
                sStr.Insert( GetKeywords()[NF_KEY_COLOR], 0 );  // Color -> FARBE
                pFormatter->ChangeIntl(eTmpLnge);
            }
            else
                sStr.Insert(rColorWord,0);
            sString.Erase(0, nPos);
            sString.EraseLeadingChars();
            sString.EraseTrailingChars();

            if ( CharClass::isAsciiNumeric( sString ) )
            {
                long nIndex = sString.ToInt32();
                if (nIndex > 0 && nIndex <= 64)
                    return pFormatter->GetUserDefColor((USHORT)nIndex-1);
                else
                    return NULL;
            }
            else
                return NULL;
        }
        else
            return NULL;
    }
    else
    {
        sStr.Erase();
        if (bConvertMode)
        {
            pFormatter->ChangeIntl(eNewLnge);
            sStr = GetKeywords()[NF_KEY_FIRSTCOLOR+i];           // red -> rot
            pFormatter->ChangeIntl(eTmpLnge);
        }
        else
            sStr = pKeyword[NF_KEY_FIRSTCOLOR+i];

        return &(StandardColor[i]);
    }
}


short ImpSvNumberformatScan::GetKeyWord( const String& sSymbol, xub_StrLen nPos )
{
    String sString = pFormatter->GetCharClass()->toUpper( sSymbol, nPos, sSymbol.Len() - nPos );
    const String* pKeyword = GetKeywords();
    // #77026# for the Xcl perverts: the GENERAL keyword is recognized anywhere
    if ( sString.Search( pKeyword[NF_KEY_GENERAL] ) == 0 )
        return NF_KEY_GENERAL;
    //! MUST be a reverse search to find longer strings first
    short i = NF_KEYWORD_ENTRIES_COUNT-1;
    BOOL bFound = FALSE;
    for ( ; i > NF_KEY_LASTKEYWORD_SO5; --i )
    {
        bFound = sString.Search(pKeyword[i]) == 0;
        if ( bFound )
        {
            break;
        }
    }
    // new keywords take precedence over old keywords
    if ( !bFound )
    {	// skip the gap of colors et al between new and old keywords and search on
        i = NF_KEY_LASTKEYWORD;
        while ( i > 0 && sString.Search(pKeyword[i]) != 0 )
            i--;
        if ( i > NF_KEY_LASTOLDKEYWORD && sString != pKeyword[i] )
        {	// found something, but maybe it's something else?
            // e.g. new NNN is found in NNNN, for NNNN we must search on
            short j = i - 1;
            while ( j > 0 && sString.Search(pKeyword[j]) != 0 )
                j--;
            if ( j && pKeyword[j].Len() > pKeyword[i].Len() )
                return j;
        }
    }
    // The Thai T NatNum modifier during Xcl import.
    if (i == 0 && bConvertMode && sString.GetChar(0) == 'T' && eTmpLnge ==
            LANGUAGE_ENGLISH_US && MsLangId::getRealLanguage( eNewLnge) ==
            LANGUAGE_THAI)
        i = NF_KEY_THAI_T;
    return i;		// 0 => not found
}

//---------------------------------------------------------------------------
// Next_Symbol
//---------------------------------------------------------------------------
// Zerlegt die Eingabe in Symbole fuer die weitere
// Verarbeitung (Turing-Maschine).
//---------------------------------------------------------------------------
// Ausgangs Zustand = SsStart
//---------------+-------------------+-----------------------+---------------
// Alter Zustand | gelesenes Zeichen | Aktion                | Neuer Zustand
//---------------+-------------------+-----------------------+---------------
// SsStart       | Buchstabe         | Symbol=Zeichen        | SsGetWord
//               |    "              | Typ = String          | SsGetString
//               |    \              | Typ = String          | SsGetChar
//               |    *              | Typ = Star            | SsGetStar
//               |    _              | Typ = Blank           | SsGetBlank
//               | @ # 0 ? / . , % [ | Symbol = Zeichen;     |
//               | ] ' Blank         | Typ = Steuerzeichen   | SsStop
//               | $ - + ( ) :       | Typ    = String;      |
//               | |                 | Typ    = Comment      | SsStop
//               | Sonst             | Symbol = Zeichen      | SsStop
//---------------|-------------------+-----------------------+---------------
// SsGetChar     | Sonst             | Symbol=Zeichen        | SsStop
//---------------+-------------------+-----------------------+---------------
// GetString     | "                 |                       | SsStop
//               | Sonst             | Symbol+=Zeichen       | GetString
//---------------+-------------------+-----------------------+---------------
// SsGetWord     | Buchstabe         | Symbol += Zeichen     |
//               | + -        (E+ E-)| Symbol += Zeichen     | SsStop
//               | /          (AM/PM)| Symbol += Zeichen     |
//               | Sonst             | Pos--, if Key Typ=Word| SsStop
//---------------+-------------------+-----------------------+---------------
// SsGetStar     | Sonst             | Symbol+=Zeichen       | SsStop
//               |                   | markiere Sonderfall * |
//---------------+-------------------+-----------------------+---------------
// SsGetBlank    | Sonst             | Symbol+=Zeichen       | SsStop
//               |                   | markiere Sonderfall _ |
//---------------+-------------------+-----------------------+---------------
// Wurde im State SsGetWord ein Schluesselwort erkannt (auch als
// Anfangsteilwort des Symbols)
// so werden die restlichen Buchstaben zurueckgeschrieben !!

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

short ImpSvNumberformatScan::Next_Symbol( const String& rStr,
            xub_StrLen& nPos, String& sSymbol )
{
    if ( bKeywordsNeedInit )
        InitKeywords();
    const CharClass* pChrCls = pFormatter->GetCharClass();
    const LocaleDataWrapper* pLoc = pFormatter->GetLocaleData();
    const xub_StrLen nStart = nPos;
    short eType = 0;
    ScanState eState = SsStart;
    sSymbol.Erase();
    while ( nPos < rStr.Len() && eState != SsStop )
    {
        sal_Unicode cToken = rStr.GetChar( nPos++ );
        switch (eState)
        {
            case SsStart:
            {
                // Fetch any currency longer than one character and don't get
                // confused later on by "E/" or other combinations of letters
                // and meaningful symbols. Necessary for old automatic currency.
                // #96158# But don't do it if we're starting a "[...]" section,
                // for example a "[$...]" new currency symbol to not parse away
                // "$U" (symbol) of "[$UYU]" (abbreviation).
                if ( nCurrPos != STRING_NOTFOUND && sCurString.Len() > 1 &&
                        nPos-1 + sCurString.Len() <= rStr.Len() &&
                        !(nPos > 1 && rStr.GetChar( nPos-2 ) == '[') )
                {
                    String aTest( rStr.Copy( nPos-1, sCurString.Len() ) );
                    pChrCls->toUpper( aTest );
                    if ( aTest == sCurString )
                    {
                        sSymbol = rStr.Copy( --nPos, sCurString.Len() );
                        nPos = nPos + sSymbol.Len();
                        eState = SsStop;
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
                    {
                        eType = NF_SYMBOLTYPE_DEL;
                        sSymbol += cToken;
                        eState = SsStop;
                    }
                    break;
                    case '*':
                    {
                        eType = NF_SYMBOLTYPE_STAR;
                        sSymbol += cToken;
                        eState = SsGetStar;
                    }
                    break;
                    case '_':
                    {
                        eType = NF_SYMBOLTYPE_BLANK;
                        sSymbol += cToken;
                        eState = SsGetBlank;
                    }
                    break;
#if NF_COMMENT_IN_FORMATSTRING
                    case '{':
                        eType = NF_SYMBOLTYPE_COMMENT;
                        eState = SsStop;
                        sSymbol.Append( rStr.GetBuffer() + (nPos-1), rStr.Len() - (nPos-1) );
                        nPos = rStr.Len();
                    break;
#endif
                    case '"':
                        eType = NF_SYMBOLTYPE_STRING;
                        eState = SsGetString;
                        sSymbol += cToken;
                    break;
                    case '\\':
                        eType = NF_SYMBOLTYPE_STRING;
                        eState = SsGetChar;
                        sSymbol += cToken;
                    break;
                    case '$':
                    case '+':
                    case '(':
                    case ')':
                        eType = NF_SYMBOLTYPE_STRING;
                        eState = SsStop;
                        sSymbol += cToken;
                    break;
                    default :
                    {
                        if (StringEqualsChar( pFormatter->GetNumDecimalSep(), cToken) ||
                                StringEqualsChar( pFormatter->GetNumThousandSep(), cToken) ||
                                StringEqualsChar( pFormatter->GetDateSep(), cToken) ||
                                StringEqualsChar( pLoc->getTimeSep(), cToken) ||
                                StringEqualsChar( pLoc->getTime100SecSep(), cToken))
                        {
                            // Another separator than pre-known ASCII
                            eType = NF_SYMBOLTYPE_DEL;
                            sSymbol += cToken;
                            eState = SsStop;
                        }
                        else if ( pChrCls->isLetter( rStr, nPos-1 ) )
                        {
                            short nTmpType = GetKeyWord( rStr, nPos-1 );
                            if ( nTmpType )
                            {
                                BOOL bCurrency = FALSE;
                                // "Automatic" currency may start with keyword,
                                // like "R" (Rand) and 'R' (era)
                                if ( nCurrPos != STRING_NOTFOUND &&
                                    nPos-1 + sCurString.Len() <= rStr.Len() &&
                                    sCurString.Search( sKeyword[nTmpType] ) == 0 )
                                {
                                    String aTest( rStr.Copy( nPos-1, sCurString.Len() ) );
                                    pChrCls->toUpper( aTest );
                                    if ( aTest == sCurString )
                                        bCurrency = TRUE;
                                }
                                if ( bCurrency )
                                {
                                    eState = SsGetWord;
                                    sSymbol += cToken;
                                }
                                else
                                {
                                    eType = nTmpType;
                                    xub_StrLen nLen = sKeyword[eType].Len();
                                    sSymbol = rStr.Copy( nPos-1, nLen );
                                    if ( eType == NF_KEY_E || IsAmbiguousE( eType ) )
                                    {
                                        sal_Unicode cNext = rStr.GetChar(nPos);
                                        switch ( cNext )
                                        {
                                            case '+' :
                                            case '-' :	// E+ E- combine to one symbol
                                                sSymbol += cNext;
                                                eType = NF_KEY_E;
                                                nPos++;
                                            break;
                                            case '0' :
                                            case '#' :	// scientific E without sign
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
                                sSymbol += cToken;
                            }
                        }
                        else
                        {
                            eType = NF_SYMBOLTYPE_STRING;
                            eState = SsStop;
                            sSymbol += cToken;
                        }
                    }
                    break;
                }
            }
            break;
            case SsGetChar:
            {
                sSymbol += cToken;
                eState = SsStop;
            }
            break;
            case SsGetString:
            {
                if (cToken == '"')
                    eState = SsStop;
                sSymbol += cToken;
            }
            break;
            case SsGetWord:
            {
                if ( pChrCls->isLetter( rStr, nPos-1 ) )
                {
                    short nTmpType = GetKeyWord( rStr, nPos-1 );
                    if ( nTmpType )
                    {	// beginning of keyword, stop scan and put back
                        eType = NF_SYMBOLTYPE_STRING;
                        eState = SsStop;
                        nPos--;
                    }
                    else
                        sSymbol += cToken;
                }
                else
                {
                    BOOL bDontStop = FALSE;
                    switch (cToken)
                    {
                        case '/':						// AM/PM, A/P
                        {
                            sal_Unicode cNext = rStr.GetChar(nPos);
                            if ( cNext == 'P' || cNext == 'p' )
                            {
                                xub_StrLen nLen = sSymbol.Len();
                                if ( 1 <= nLen
                                        && (sSymbol.GetChar(0) == 'A' || sSymbol.GetChar(0) == 'a')
                                        && (nLen == 1 || (nLen == 2
                                            && (sSymbol.GetChar(1) == 'M' || sSymbol.GetChar(1) == 'm')
                                            && (rStr.GetChar(nPos+1) == 'M' || rStr.GetChar(nPos+1) == 'm'))) )
                                {
                                    sSymbol += cToken;
                                    bDontStop = TRUE;
                                }
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
            }
            break;
            case SsGetStar:
            {
                eState = SsStop;
                sSymbol += cToken;
                nRepPos = (nPos - nStart) - 1;	// everytime > 0!!
            }
            break;
            case SsGetBlank:
            {
                eState = SsStop;
                sSymbol += cToken;
            }
            break;
            default:
            break;
        }									// of switch
    } 										// of while
    if (eState == SsGetWord)
        eType = NF_SYMBOLTYPE_STRING;
    return eType;
}

xub_StrLen ImpSvNumberformatScan::Symbol_Division(const String& rString)
{
    nCurrPos = STRING_NOTFOUND;
                                                    // Ist Waehrung im Spiel?
    String sString = pFormatter->GetCharClass()->upper(rString);
    xub_StrLen nCPos = 0;
    while (nCPos != STRING_NOTFOUND)
    {
        nCPos = sString.Search(GetCurString(),nCPos);
        if (nCPos != STRING_NOTFOUND)
        {
            // in Quotes?
            xub_StrLen nQ = SvNumberformat::GetQuoteEnd( sString, nCPos );
            if ( nQ == STRING_NOTFOUND )
            {
                sal_Unicode c;
                if ( nCPos == 0 ||
                    ((c = sString.GetChar(xub_StrLen(nCPos-1))) != '"'
                            && c != '\\') )			// dm kann durch "dm
                {                   				// \d geschuetzt werden
                    nCurrPos = nCPos;
                    nCPos = STRING_NOTFOUND;		// Abbruch
                }
                else
                    nCPos++;						// weitersuchen
            }
            else
                nCPos = nQ + 1;						// weitersuchen
        }
    }
    nAnzStrings = 0;
    BOOL bStar = FALSE;					// wird bei '*'Detektion gesetzt
    Reset();

    xub_StrLen nPos = 0;
    const xub_StrLen nLen = rString.Len();
    while (nPos < nLen && nAnzStrings < NF_MAX_FORMAT_SYMBOLS)
    {
        nTypeArray[nAnzStrings] = Next_Symbol(rString, nPos, sStrArray[nAnzStrings]);
        if (nTypeArray[nAnzStrings] == NF_SYMBOLTYPE_STAR)
        {								// Ueberwachung des '*'
            if (bStar)
                return nPos;		// Fehler: doppelter '*'
            else
                bStar = TRUE;
        }
        nAnzStrings++;
    }

    return 0;						// 0 => ok
}

void ImpSvNumberformatScan::SkipStrings(USHORT& i, xub_StrLen& nPos)
{
    while (i < nAnzStrings && (   nTypeArray[i] == NF_SYMBOLTYPE_STRING
                               || nTypeArray[i] == NF_SYMBOLTYPE_BLANK
                               || nTypeArray[i] == NF_SYMBOLTYPE_STAR) )
    {
        nPos = nPos + sStrArray[i].Len();
        i++;
    }
}


USHORT ImpSvNumberformatScan::PreviousKeyword(USHORT i)
{
    short res = 0;
    if (i > 0 && i < nAnzStrings)
    {
        i--;
        while (i > 0 && nTypeArray[i] <= 0)
            i--;
        if (nTypeArray[i] > 0)
            res = nTypeArray[i];
    }
    return res;
}

USHORT ImpSvNumberformatScan::NextKeyword(USHORT i)
{
    short res = 0;
    if (i < nAnzStrings-1)
    {
        i++;
        while (i < nAnzStrings-1 && nTypeArray[i] <= 0)
            i++;
        if (nTypeArray[i] > 0)
            res = nTypeArray[i];
    }
    return res;
}

short ImpSvNumberformatScan::PreviousType( USHORT i )
{
    if ( i > 0 && i < nAnzStrings )
    {
        do
        {
            i--;
        } while ( i > 0 && nTypeArray[i] == NF_SYMBOLTYPE_EMPTY );
        return nTypeArray[i];
    }
    return 0;
}

sal_Unicode ImpSvNumberformatScan::PreviousChar(USHORT i)
{
    sal_Unicode res = ' ';
    if (i > 0 && i < nAnzStrings)
    {
        i--;
        while (i > 0 && ( 	nTypeArray[i] == NF_SYMBOLTYPE_EMPTY
                         || nTypeArray[i] == NF_SYMBOLTYPE_STRING
                         || nTypeArray[i] == NF_SYMBOLTYPE_STAR
                         || nTypeArray[i] == NF_SYMBOLTYPE_BLANK ) )
            i--;
        if (sStrArray[i].Len() > 0)
            res = sStrArray[i].GetChar(xub_StrLen(sStrArray[i].Len()-1));
    }
    return res;
}

sal_Unicode ImpSvNumberformatScan::NextChar(USHORT i)
{
    sal_Unicode res = ' ';
    if (i < nAnzStrings-1)
    {
        i++;
        while (i < nAnzStrings-1 &&
               (   nTypeArray[i] == NF_SYMBOLTYPE_EMPTY
                || nTypeArray[i] == NF_SYMBOLTYPE_STRING
                || nTypeArray[i] == NF_SYMBOLTYPE_STAR
                || nTypeArray[i] == NF_SYMBOLTYPE_BLANK))
            i++;
        if (sStrArray[i].Len() > 0)
            res = sStrArray[i].GetChar(0);
    }
    return res;
}

BOOL ImpSvNumberformatScan::IsLastBlankBeforeFrac(USHORT i)
{
    BOOL res = TRUE;
    if (i < nAnzStrings-1)
    {
        BOOL bStop = FALSE;
        i++;
        while (i < nAnzStrings-1 && !bStop)
        {
            i++;
            if ( nTypeArray[i] == NF_SYMBOLTYPE_DEL &&
                    sStrArray[i].GetChar(0) == '/')
                bStop = TRUE;
            else if ( nTypeArray[i] == NF_SYMBOLTYPE_DEL &&
                    sStrArray[i].GetChar(0) == ' ')
                res = FALSE;
        }
        if (!bStop)									// kein '/'
            res = FALSE;
    }
    else
        res = FALSE;								// kein '/' mehr

    return res;
}

void ImpSvNumberformatScan::Reset()
{
    nAnzStrings = 0;
    nAnzResStrings = 0;
#if 0
// ER 20.06.97 14:05   nicht noetig, wenn nAnzStrings beachtet wird
    for (size_t i = 0; i < NF_MAX_FORMAT_SYMBOLS; i++)
    {
        sStrArray[i].Erase();
        nTypeArray[i] = 0;
    }
#endif
    eScannedType = NUMBERFORMAT_UNDEFINED;
    nRepPos = 0;
    bExp = FALSE;
    bThousand = FALSE;
    nThousand = 0;
    bDecSep = FALSE;
    nDecPos =  -1;
    nExpPos = (USHORT) -1;
    nBlankPos = (USHORT) -1;
    nCntPre = 0;
    nCntPost = 0;
    nCntExp = 0;
    bFrac = FALSE;
    bBlank = FALSE;
    nNatNumModifier = 0;
}


BOOL ImpSvNumberformatScan::Is100SecZero( USHORT i, BOOL bHadDecSep )
{
    USHORT nIndexPre = PreviousKeyword( i );
    return (nIndexPre == NF_KEY_S || nIndexPre == NF_KEY_SS)
            && (bHadDecSep                 // S, SS ','
            || (i>0 && nTypeArray[i-1] == NF_SYMBOLTYPE_STRING));
                // SS"any"00  take "any" as a valid decimal separator
}


xub_StrLen ImpSvNumberformatScan::ScanType(const String&)
{
    const LocaleDataWrapper* pLoc = pFormatter->GetLocaleData();

    xub_StrLen nPos = 0;
    USHORT i = 0;
    short eNewType;
    BOOL bMatchBracket = FALSE;

    SkipStrings(i, nPos);
    while (i < nAnzStrings)
    {
        if (nTypeArray[i] > 0)
        {                                       // keyword
            switch (nTypeArray[i])
            {
                case NF_KEY_E:			 				// E
                    eNewType = NUMBERFORMAT_SCIENTIFIC;
                break;
                case NF_KEY_AMPM:		 				// AM,A,PM,P
                case NF_KEY_AP:
                case NF_KEY_H:							// H
                case NF_KEY_HH:							// HH
                case NF_KEY_S:							// S
                case NF_KEY_SS:							// SS
                    eNewType = NUMBERFORMAT_TIME;
                break;
                case NF_KEY_M:			 				// M
                case NF_KEY_MM:			 				// MM
                {                                       // minute or month
                    USHORT nIndexPre = PreviousKeyword(i);
                    USHORT nIndexNex = NextKeyword(i);
                    sal_Unicode cChar = PreviousChar(i);
                    if (nIndexPre == NF_KEY_H	|| 	// H
                        nIndexPre == NF_KEY_HH	|| 	// HH
                        nIndexNex == NF_KEY_S	|| 	// S
                        nIndexNex == NF_KEY_SS	||  // SS
                        cChar == '['  )     // [M
                    {
                        eNewType = NUMBERFORMAT_TIME;
                        nTypeArray[i] -= 2;			// 6 -> 4, 7 -> 5
                    }
                    else
                        eNewType = NUMBERFORMAT_DATE;
                }
                break;
                case NF_KEY_MMM:				// MMM
                case NF_KEY_MMMM:				// MMMM
                case NF_KEY_MMMMM:				// MMMMM
                case NF_KEY_Q:					// Q
                case NF_KEY_QQ:					// QQ
                case NF_KEY_D:					// D
                case NF_KEY_DD:					// DD
                case NF_KEY_DDD:				// DDD
                case NF_KEY_DDDD:				// DDDD
                case NF_KEY_YY:					// YY
                case NF_KEY_YYYY:				// YYYY
                case NF_KEY_NN:					// NN
                case NF_KEY_NNN:				// NNN
                case NF_KEY_NNNN:				// NNNN
                case NF_KEY_WW :				// WW
                case NF_KEY_AAA :				// AAA
                case NF_KEY_AAAA :				// AAAA
                case NF_KEY_EC :				// E
                case NF_KEY_EEC :				// EE
                case NF_KEY_G :					// G
                case NF_KEY_GG :				// GG
                case NF_KEY_GGG :				// GGG
                case NF_KEY_R :					// R
                case NF_KEY_RR :				// RR
                    eNewType = NUMBERFORMAT_DATE;
                break;
                case NF_KEY_CCC:				// CCC
                    eNewType = NUMBERFORMAT_CURRENCY;
                break;
                case NF_KEY_GENERAL:			// Standard
                    eNewType = NUMBERFORMAT_NUMBER;
                break;
                default:
                    eNewType = NUMBERFORMAT_UNDEFINED;
                break;
            }
        }
        else
        {                                       // control character
            switch ( sStrArray[i].GetChar(0) )
            {
                case '#':
                case '?':
                    eNewType = NUMBERFORMAT_NUMBER;
                break;
                case '0':
                {
                    if ( (eScannedType & NUMBERFORMAT_TIME) == NUMBERFORMAT_TIME )
                    {
                        if ( Is100SecZero( i, bDecSep ) )
                        {
                            bDecSep = TRUE;                 // subsequent 0's
                            eNewType = NUMBERFORMAT_TIME;
                        }
                        else
                            return nPos;                    // Error
                    }
                    else
                        eNewType = NUMBERFORMAT_NUMBER;
                }
                break;
                case '%':
                    eNewType = NUMBERFORMAT_PERCENT;
                break;
                case '/':
                    eNewType = NUMBERFORMAT_FRACTION;
                break;
                case '[':
                {
                    if ( i < nAnzStrings-1 &&
                            nTypeArray[i+1] == NF_SYMBOLTYPE_STRING &&
                            sStrArray[i+1].GetChar(0) == '$' )
                    {	// as of SV_NUMBERFORMATTER_VERSION_NEW_CURR
                        eNewType = NUMBERFORMAT_CURRENCY;
                        bMatchBracket = TRUE;
                    }
                    else if ( i < nAnzStrings-1 &&
                            nTypeArray[i+1] == NF_SYMBOLTYPE_STRING &&
                            sStrArray[i+1].GetChar(0) == '~' )
                    {	// as of SV_NUMBERFORMATTER_VERSION_CALENDAR
                        eNewType = NUMBERFORMAT_DATE;
                        bMatchBracket = TRUE;
                    }
                    else
                    {
                        USHORT nIndexNex = NextKeyword(i);
                        if (nIndexNex == NF_KEY_H	|| 	// H
                            nIndexNex == NF_KEY_HH	|| 	// HH
                            nIndexNex == NF_KEY_M	|| 	// M
                            nIndexNex == NF_KEY_MM	|| 	// MM
                            nIndexNex == NF_KEY_S	|| 	// S
                            nIndexNex == NF_KEY_SS   )	// SS
                            eNewType = NUMBERFORMAT_TIME;
                        else
                            return nPos;                // Error
                    }
                }
                break;
                case '@':
                    eNewType = NUMBERFORMAT_TEXT;
                break;
                default:
                    if ( sStrArray[i] == pLoc->getTime100SecSep() )
                        bDecSep = TRUE;                     // for SS,0
                    eNewType = NUMBERFORMAT_UNDEFINED;
                break;
            }
        }
        if (eScannedType == NUMBERFORMAT_UNDEFINED)
            eScannedType = eNewType;
        else if (eScannedType == NUMBERFORMAT_TEXT || eNewType == NUMBERFORMAT_TEXT)
            eScannedType = NUMBERFORMAT_TEXT;				// Text bleibt immer Text
        else if (eNewType == NUMBERFORMAT_UNDEFINED)
        {											// bleibt wie bisher
        }
        else if (eScannedType != eNewType)
        {
            switch (eScannedType)
            {
                case NUMBERFORMAT_DATE:
                {
                    switch (eNewType)
                    {
                        case NUMBERFORMAT_TIME:
                            eScannedType = NUMBERFORMAT_DATETIME;
                        break;
                        case NUMBERFORMAT_FRACTION: 		// DD/MM
                        break;
                        default:
                        {
                            if (nCurrPos != STRING_NOTFOUND)
                                eScannedType = NUMBERFORMAT_UNDEFINED;
                            else if ( sStrArray[i] != pFormatter->GetDateSep() )
                                return nPos;
                        }
                    }
                }
                break;
                case NUMBERFORMAT_TIME:
                {
                    switch (eNewType)
                    {
                        case NUMBERFORMAT_DATE:
                            eScannedType = NUMBERFORMAT_DATETIME;
                        break;
                        case NUMBERFORMAT_FRACTION: 		// MM/SS
                        break;
                        default:
                        {
                            if (nCurrPos != STRING_NOTFOUND)
                                eScannedType = NUMBERFORMAT_UNDEFINED;
                            else if ( sStrArray[i] != pLoc->getTimeSep() )
                                return nPos;
                        }
                    }
                }
                break;
                case NUMBERFORMAT_DATETIME:
                {
                    switch (eNewType)
                    {
                        case NUMBERFORMAT_TIME:
                        case NUMBERFORMAT_DATE:
                        break;
                        case NUMBERFORMAT_FRACTION: 		// DD/MM
                        break;
                        default:
                        {
                            if (nCurrPos != STRING_NOTFOUND)
                                eScannedType = NUMBERFORMAT_UNDEFINED;
                            else if ( sStrArray[i] != pFormatter->GetDateSep()
                                   && sStrArray[i] != pLoc->getTimeSep() )
                                return nPos;
                        }
                    }
                }
                break;
                case NUMBERFORMAT_PERCENT:
                {
                    switch (eNewType)
                    {
                        case NUMBERFORMAT_NUMBER:	// nur Zahl nach Prozent
                        break;
                        default:
                            return nPos;
                    }
                }
                break;
                case NUMBERFORMAT_SCIENTIFIC:
                {
                    switch (eNewType)
                    {
                        case NUMBERFORMAT_NUMBER:	// nur Zahl nach E
                        break;
                        default:
                            return nPos;
                    }
                }
                break;
                case NUMBERFORMAT_NUMBER:
                {
                    switch (eNewType)
                    {
                        case NUMBERFORMAT_SCIENTIFIC:
                        case NUMBERFORMAT_PERCENT:
                        case NUMBERFORMAT_FRACTION:
                        case NUMBERFORMAT_CURRENCY:
                            eScannedType = eNewType;
                        break;
                        default:
                            if (nCurrPos != STRING_NOTFOUND)
                                eScannedType = NUMBERFORMAT_UNDEFINED;
                            else
                                return nPos;
                    }
                }
                break;
                case NUMBERFORMAT_FRACTION:
                {
                    switch (eNewType)
                    {
                        case NUMBERFORMAT_NUMBER:			// nur Zahl nach Bruch
                        break;
                        default:
                            return nPos;
                    }
                }
                break;
                default:
                break;
            }
        }
        nPos = nPos + sStrArray[i].Len();			// Korrekturposition
        i++;
        if ( bMatchBracket )
        {   // no type detection inside of matching brackets if [$...], [~...]
            while ( bMatchBracket && i < nAnzStrings )
            {
                if ( nTypeArray[i] == NF_SYMBOLTYPE_DEL
                        && sStrArray[i].GetChar(0) == ']' )
                    bMatchBracket = FALSE;
                else
                    nTypeArray[i] = NF_SYMBOLTYPE_STRING;
                nPos = nPos + sStrArray[i].Len();
                i++;
            }
            if ( bMatchBracket )
                return nPos;    // missing closing bracket at end of code
        }
        SkipStrings(i, nPos);
    }

    if ((eScannedType == NUMBERFORMAT_NUMBER || eScannedType == NUMBERFORMAT_UNDEFINED)
         && nCurrPos != STRING_NOTFOUND)
        eScannedType = NUMBERFORMAT_CURRENCY;	// old "automatic" currency
    if (eScannedType == NUMBERFORMAT_UNDEFINED)
        eScannedType = NUMBERFORMAT_DEFINED;
    return 0;								// Alles ok
}


int ImpSvNumberformatScan::FinalScanGetCalendar( xub_StrLen& nPos, USHORT& i,
            USHORT& rAnzResStrings )
{
    if ( sStrArray[i].GetChar(0) == '[' &&
            i < nAnzStrings-1 &&
            nTypeArray[i+1] == NF_SYMBOLTYPE_STRING &&
            sStrArray[i+1].GetChar(0) == '~' )
    {	// [~calendarID]
        // as of SV_NUMBERFORMATTER_VERSION_CALENDAR
        nPos = nPos + sStrArray[i].Len();			// [
        nTypeArray[i] = NF_SYMBOLTYPE_CALDEL;
        nPos = nPos + sStrArray[++i].Len();		// ~
        sStrArray[i-1] += sStrArray[i];		// [~
        nTypeArray[i] = NF_SYMBOLTYPE_EMPTY;
        rAnzResStrings--;
        if ( ++i >= nAnzStrings )
            return -1;		// error
        nPos = nPos + sStrArray[i].Len();			// calendarID
        String& rStr = sStrArray[i];
        nTypeArray[i] = NF_SYMBOLTYPE_CALENDAR;	// convert
        i++;
        while ( i < nAnzStrings &&
                sStrArray[i].GetChar(0) != ']' )
        {
            nPos = nPos + sStrArray[i].Len();
            rStr += sStrArray[i];
            nTypeArray[i] = NF_SYMBOLTYPE_EMPTY;
            rAnzResStrings--;
            i++;
        }
        if ( rStr.Len() && i < nAnzStrings &&
                sStrArray[i].GetChar(0) == ']' )
        {
            nTypeArray[i] = NF_SYMBOLTYPE_CALDEL;
            nPos = nPos + sStrArray[i].Len();
            i++;
        }
        else
            return -1;		// error
        return 1;
    }
    return 0;
}

xub_StrLen ImpSvNumberformatScan::FinalScan( String& rString, String& rComment )
{
    const LocaleDataWrapper* pLoc = pFormatter->GetLocaleData();

    // save values for convert mode
    String sOldDecSep       = pFormatter->GetNumDecimalSep();
    String sOldThousandSep  = pFormatter->GetNumThousandSep();
    String sOldDateSep      = pFormatter->GetDateSep();
    String sOldTimeSep		= pLoc->getTimeSep();
    String sOldTime100SecSep= pLoc->getTime100SecSep();
    String sOldCurSymbol    = GetCurSymbol();
    String sOldCurString    = GetCurString();
    sal_Unicode cOldKeyH    = sKeyword[NF_KEY_H].GetChar(0);
    sal_Unicode cOldKeyMI   = sKeyword[NF_KEY_MI].GetChar(0);
    sal_Unicode cOldKeyS    = sKeyword[NF_KEY_S].GetChar(0);

    // If the group separator is a Non-Breaking Space (French) continue with a
    // normal space instead so queries on space work correctly.
    // The format string is adjusted to allow both.
    // For output of the format code string the LocaleData characters are used.
    if ( sOldThousandSep.GetChar(0) == cNonBreakingSpace && sOldThousandSep.Len() == 1 )
        sOldThousandSep = ' ';

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

    xub_StrLen nPos = 0;                    // error correction position
    USHORT i = 0;                           // symbol loop counter
    USHORT nCounter = 0;                    // counts digits
    nAnzResStrings = nAnzStrings;           // counts remaining symbols
    bDecSep = FALSE;                        // reset in case already used in TypeCheck
    bool bThaiT = false;                    // Thai T NatNum modifier present

    switch (eScannedType)
    {
        case NUMBERFORMAT_TEXT:
        case NUMBERFORMAT_DEFINED:
        {
            while (i < nAnzStrings)
            {
                switch (nTypeArray[i])
                {
                    case NF_SYMBOLTYPE_BLANK:
                    case NF_SYMBOLTYPE_STAR:
                    break;
                    case NF_SYMBOLTYPE_COMMENT:
                    {
                        String& rStr = sStrArray[i];
                        nPos = nPos + rStr.Len();
                        SvNumberformat::EraseCommentBraces( rStr );
                        rComment += rStr;
                        nTypeArray[i] = NF_SYMBOLTYPE_EMPTY;
                        nAnzResStrings--;
                    }
                    break;
                    case NF_KEY_GENERAL :	// #77026# "General" is the same as "@"
                    break;
                    default:
                    {
                        if ( nTypeArray[i] != NF_SYMBOLTYPE_DEL ||
                                sStrArray[i].GetChar(0) != '@' )
                            nTypeArray[i] = NF_SYMBOLTYPE_STRING;
                    }
                    break;
                }
                nPos = nPos + sStrArray[i].Len();
                i++;
            }										// of while
        }
        break;
        case NUMBERFORMAT_NUMBER:
        case NUMBERFORMAT_PERCENT:
        case NUMBERFORMAT_CURRENCY:
        case NUMBERFORMAT_SCIENTIFIC:
        case NUMBERFORMAT_FRACTION:
        {
            sal_Unicode cThousandFill = ' ';
            while (i < nAnzStrings)
            {
                if (eScannedType == NUMBERFORMAT_FRACTION &&  	// special case
                    nTypeArray[i] == NF_SYMBOLTYPE_DEL && 			// # ### #/#
                    StringEqualsChar( sOldThousandSep, ' ' ) && // e.g. France or Sweden
                    StringEqualsChar( sStrArray[i], ' ' ) &&
                    !bFrac                          &&
                    IsLastBlankBeforeFrac(i) )
                {
                    nTypeArray[i] = NF_SYMBOLTYPE_STRING;			// del->string
                }                                               // kein Taus.p.


                if (nTypeArray[i] == NF_SYMBOLTYPE_BLANK	||
                    nTypeArray[i] == NF_SYMBOLTYPE_STAR	||
                    nTypeArray[i] == NF_KEY_CCC			||	// CCC
                    nTypeArray[i] == NF_KEY_GENERAL )		// Standard
                {
                    if (nTypeArray[i] == NF_KEY_GENERAL)
                    {
                        nThousand = FLAG_STANDARD_IN_FORMAT;
                        if ( bConvertMode )
                            sStrArray[i] = sNameStandardFormat;
                    }
                    nPos = nPos + sStrArray[i].Len();
                    i++;
                }
                else if (nTypeArray[i] == NF_SYMBOLTYPE_STRING ||  // Strings oder
                         nTypeArray[i] > 0) 					// Keywords
                {
                    if (eScannedType == NUMBERFORMAT_SCIENTIFIC &&
                             nTypeArray[i] == NF_KEY_E) 		// E+
                    {
                        if (bExp) 								// doppelt
                            return nPos;
                        bExp = TRUE;
                        nExpPos = i;
                        if (bDecSep)
                            nCntPost = nCounter;
                        else
                            nCntPre = nCounter;
                        nCounter = 0;
                        nTypeArray[i] = NF_SYMBOLTYPE_EXP;
                    }
                    else if (eScannedType == NUMBERFORMAT_FRACTION &&
                             sStrArray[i].GetChar(0) == ' ')
                    {
                        if (!bBlank && !bFrac)	// nicht doppelt oder hinter /
                        {
                            if (bDecSep && nCounter > 0)	// Nachkommastellen
                                return nPos;				// Fehler
                            bBlank = TRUE;
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
                    else
                        nTypeArray[i] = NF_SYMBOLTYPE_STRING;
                    nPos = nPos + sStrArray[i].Len();
                    i++;
                }
                else if (nTypeArray[i] == NF_SYMBOLTYPE_DEL)
                {
                    sal_Unicode cHere = sStrArray[i].GetChar(0);
                    // Handle not pre-known separators in switch.
                    sal_Unicode cSimplified;
                    if (StringEqualsChar( pFormatter->GetNumThousandSep(), cHere))
                        cSimplified = ',';
                    else if (StringEqualsChar( pFormatter->GetNumDecimalSep(), cHere))
                        cSimplified = '.';
                    else
                        cSimplified = cHere;
                    switch ( cSimplified )
                    {
                        case '#':
                        case '0':
                        case '?':
                        {
                            if (nThousand > 0)					// #... #
                                return nPos;					// Fehler
                            else if (bFrac && cHere == '0')
                                return nPos;					// 0 im Nenner
                            nTypeArray[i] = NF_SYMBOLTYPE_DIGIT;
                            String& rStr = sStrArray[i];
                            nPos = nPos + rStr.Len();
                            i++;
                            nCounter++;
                            while (i < nAnzStrings &&
                                (sStrArray[i].GetChar(0) == '#' ||
                                    sStrArray[i].GetChar(0) == '0' ||
                                    sStrArray[i].GetChar(0) == '?')
                                )
                            {
                                rStr += sStrArray[i];
                                nPos = nPos + sStrArray[i].Len();
                                nTypeArray[i] = NF_SYMBOLTYPE_EMPTY;
                                nAnzResStrings--;
                                nCounter++;
                                i++;
                            }
                        }
                        break;
                        case '-':
                        {
                            if ( bDecSep && nDecPos+1 == i &&
                                    nTypeArray[nDecPos] == NF_SYMBOLTYPE_DECSEP )
                            {   // "0.--"
                                nTypeArray[i] = NF_SYMBOLTYPE_DIGIT;
                                String& rStr = sStrArray[i];
                                nPos = nPos + rStr.Len();
                                i++;
                                nCounter++;
                                while (i < nAnzStrings &&
                                        (sStrArray[i].GetChar(0) == '-') )
                                {
                                    // If more than two dashes are present in
                                    // currency formats the last dash will be
                                    // interpreted literally as a minus sign.
                                    // Has to be this ugly. Period.
                                    if ( eScannedType == NUMBERFORMAT_CURRENCY
                                            && rStr.Len() >= 2 &&
                                            (i == nAnzStrings-1 ||
                                            sStrArray[i+1].GetChar(0) != '-') )
                                        break;
                                    rStr += sStrArray[i];
                                    nPos = nPos + sStrArray[i].Len();
                                    nTypeArray[i] = NF_SYMBOLTYPE_EMPTY;
                                    nAnzResStrings--;
                                    nCounter++;
                                    i++;
                                }
                            }
                            else
                            {
                                nTypeArray[i] = NF_SYMBOLTYPE_STRING;
                                nPos = nPos + sStrArray[i].Len();
                                i++;
                            }
                        }
                        break;
                        case '.':
                        case ',':
                        case '\'':
                        case ' ':
                        {
                            sal_Unicode cSep = cHere;	// remember
                            if ( StringEqualsChar( sOldThousandSep, cSep ) )
                            {
                                // previous char with skip empty
                                sal_Unicode cPre = PreviousChar(i);
                                sal_Unicode cNext;
                                if (bExp || bBlank || bFrac)
                                {	// after E, / or ' '
                                    if ( !StringEqualsChar( sOldThousandSep, ' ' ) )
                                    {
                                        nPos = nPos + sStrArray[i].Len();
                                        nTypeArray[i] = NF_SYMBOLTYPE_EMPTY;
                                        nAnzResStrings--;
                                        i++; 				// eat it
                                    }
                                    else
                                        nTypeArray[i] = NF_SYMBOLTYPE_STRING;
                                }
                                else if (i > 0 && i < nAnzStrings-1   &&
                                    (cPre == '#' || cPre == '0')      &&
                                    ((cNext = NextChar(i)) == '#' || cNext == '0')
                                    )					// #,#
                                {
                                    nPos = nPos + sStrArray[i].Len();
                                    if (!bThousand)					// only once
                                    {   // set hard, in case of Non-Breaking Space or ConvertMode
                                        sStrArray[i] = pFormatter->GetNumThousandSep();
                                        nTypeArray[i] = NF_SYMBOLTYPE_THSEP;
                                        bThousand = TRUE;
                                        cThousandFill = sStrArray[i+1].GetChar(0);
                                    }
                                    else							// eat it
                                    {
                                        nTypeArray[i] = NF_SYMBOLTYPE_EMPTY;
                                        nAnzResStrings--;
                                    }
                                    i++;
                                }
                                else if (i > 0 && (cPre == '#' || cPre == '0')
                                    && PreviousType(i) == NF_SYMBOLTYPE_DIGIT
                                    && nThousand < FLAG_STANDARD_IN_FORMAT )
                                {									// #,,,,
                                    if ( StringEqualsChar( sOldThousandSep, ' ' ) )
                                    {	// strange, those French..
                                        BOOL bFirst = TRUE;
                                        String& rStr = sStrArray[i];
                                        //  set a hard Non-Breaking Space or ConvertMode
                                        const String& rSepF = pFormatter->GetNumThousandSep();
                                        while ( i < nAnzStrings
                                            && sStrArray[i] == sOldThousandSep
                                            && StringEqualsChar( sOldThousandSep, NextChar(i) ) )
                                        {	// last was a space or another space
                                            // is following => separator
                                            nPos = nPos + sStrArray[i].Len();
                                            if ( bFirst )
                                            {
                                                bFirst = FALSE;
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
                                        if ( i < nAnzStrings-1
                                            && sStrArray[i] == sOldThousandSep )
                                        {	// something following last space
                                            // => space if currency contained,
                                            // else separator
                                            nPos = nPos + sStrArray[i].Len();
                                            if ( (nPos <= nCurrPos &&
                                                    nCurrPos < nPos + sStrArray[i+1].Len())
                                                || nTypeArray[i+1] == NF_KEY_CCC
                                                || (i < nAnzStrings-2 &&
                                                sStrArray[i+1].GetChar(0) == '[' &&
                                                sStrArray[i+2].GetChar(0) == '$') )
                                            {
                                                nTypeArray[i] = NF_SYMBOLTYPE_STRING;
                                            }
                                            else
                                            {
                                                if ( bFirst )
                                                {
                                                    bFirst = FALSE;
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
                                            nPos = nPos + sStrArray[i].Len();
                                            sStrArray[i] = pFormatter->GetNumThousandSep();
                                            i++;
                                        } while (i < nAnzStrings &&
                                                sStrArray[i] == sOldThousandSep);
                                    }
                                }
                                else 					// any grsep
                                {
                                    nTypeArray[i] = NF_SYMBOLTYPE_STRING;
                                    String& rStr = sStrArray[i];
                                    nPos = nPos + rStr.Len();
                                    i++;
                                    while ( i < nAnzStrings &&
                                        sStrArray[i] == sOldThousandSep )
                                    {
                                        rStr += sStrArray[i];
                                        nPos = nPos + sStrArray[i].Len();
                                        nTypeArray[i] = NF_SYMBOLTYPE_EMPTY;
                                        nAnzResStrings--;
                                        i++;
                                    }
                                }
                            }
                            else if ( StringEqualsChar( sOldDecSep, cSep ) )
                            {
                                if (bBlank || bFrac)    // . behind / or ' '
                                    return nPos;		// error
                                else if (bExp)			// behind E
                                {
                                    nPos = nPos + sStrArray[i].Len();
                                    nTypeArray[i] = NF_SYMBOLTYPE_EMPTY;
                                    nAnzResStrings--;
                                    i++; 				// eat it
                                }
                                else if (bDecSep)		// any .
                                {
                                    nTypeArray[i] = NF_SYMBOLTYPE_STRING;
                                    String& rStr = sStrArray[i];
                                    nPos = nPos + rStr.Len();
                                    i++;
                                    while ( i < nAnzStrings &&
                                        sStrArray[i] == sOldDecSep )
                                    {
                                        rStr += sStrArray[i];
                                        nPos = nPos + sStrArray[i].Len();
                                        nTypeArray[i] = NF_SYMBOLTYPE_EMPTY;
                                        nAnzResStrings--;
                                        i++;
                                    }
                                }
                                else
                                {
                                    nPos = nPos + sStrArray[i].Len();
                                    nTypeArray[i] = NF_SYMBOLTYPE_DECSEP;
                                    sStrArray[i] = pFormatter->GetNumDecimalSep();
                                    bDecSep = TRUE;
                                    nDecPos = i;
                                    nCntPre = nCounter;
                                    nCounter = 0;

                                    i++;
                                }
                            } 							// of else = DecSep
                            else						// . without meaning
                            {
                                if (cSep == ' ' &&
                                    eScannedType == NUMBERFORMAT_FRACTION &&
                                    StringEqualsChar( sStrArray[i], ' ' ) )
                                {
                                    if (!bBlank && !bFrac)	// no dups
                                    {	                    // or behind /
                                        if (bDecSep && nCounter > 0)// dec.
                                            return nPos;			// error
                                        bBlank = TRUE;
                                        nBlankPos = i;
                                        nCntPre = nCounter;
                                        nCounter = 0;
                                    }
                                    nTypeArray[i] = NF_SYMBOLTYPE_STRING;
                                    nPos = nPos + sStrArray[i].Len();
                                }
                                else
                                {
                                    nTypeArray[i] = NF_SYMBOLTYPE_STRING;
                                    String& rStr = sStrArray[i];
                                    nPos = nPos + rStr.Len();
                                    i++;
                                    while (i < nAnzStrings &&
                                        StringEqualsChar( sStrArray[i], cSep ) )
                                    {
                                        rStr += sStrArray[i];
                                        nPos = nPos + sStrArray[i].Len();
                                        nTypeArray[i] = NF_SYMBOLTYPE_EMPTY;
                                        nAnzResStrings--;
                                        i++;
                                    }
                                }
                            }
                        }
                        break;
                        case '/':
                        {
                            if (eScannedType == NUMBERFORMAT_FRACTION)
                            {
                                if ( i == 0 ||
                                        (nTypeArray[i-1] != NF_SYMBOLTYPE_DIGIT &&
                                         nTypeArray[i-1] != NF_SYMBOLTYPE_EMPTY) )
                                    return nPos ? nPos : 1;	// /? not allowed
                                else if (!bFrac || (bDecSep && nCounter > 0))
                                {
                                    bFrac = TRUE;
                                    nCntPost = nCounter;
                                    nCounter = 0;
                                    nTypeArray[i] = NF_SYMBOLTYPE_FRAC;
                                    nPos = nPos + sStrArray[i].Len();
                                    i++;
                                }
                                else 				// / doppelt od. , imZaehl
                                    return nPos;	// Fehler
                            }
                            else
                            {
                                nTypeArray[i] = NF_SYMBOLTYPE_STRING;
                                nPos = nPos + sStrArray[i].Len();
                                i++;
                            }
                        }
                        break;
                        case '[' :
                        {
                            if ( eScannedType == NUMBERFORMAT_CURRENCY &&
                                    i < nAnzStrings-1 &&
                                    nTypeArray[i+1] == NF_SYMBOLTYPE_STRING &&
                                    sStrArray[i+1].GetChar(0) == '$' )
                            {	// [$DM-xxx]
                                // ab SV_NUMBERFORMATTER_VERSION_NEW_CURR
                                nPos = nPos + sStrArray[i].Len();			// [
                                nTypeArray[i] = NF_SYMBOLTYPE_CURRDEL;
                                nPos = nPos + sStrArray[++i].Len();		// $
                                sStrArray[i-1] += sStrArray[i];		// [$
                                nTypeArray[i] = NF_SYMBOLTYPE_EMPTY;
                                nAnzResStrings--;
                                if ( ++i >= nAnzStrings )
                                    return nPos;		// Fehler
                                nPos = nPos + sStrArray[i].Len();			// DM
                                String& rStr = sStrArray[i];
                                String* pStr = &sStrArray[i];
                                nTypeArray[i] = NF_SYMBOLTYPE_CURRENCY;	// wandeln
                                BOOL bHadDash = FALSE;
                                i++;
                                while ( i < nAnzStrings &&
                                        sStrArray[i].GetChar(0) != ']' )
                                {
                                    nPos = nPos + sStrArray[i].Len();
                                    if ( bHadDash )
                                    {
                                        *pStr += sStrArray[i];
                                        nTypeArray[i] = NF_SYMBOLTYPE_EMPTY;
                                        nAnzResStrings--;
                                    }
                                    else
                                    {
                                        if ( sStrArray[i].GetChar(0) == '-' )
                                        {
                                            bHadDash = TRUE;
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
                                if ( rStr.Len() && i < nAnzStrings &&
                                        sStrArray[i].GetChar(0) == ']' )
                                {
                                    nTypeArray[i] = NF_SYMBOLTYPE_CURRDEL;
                                    nPos = nPos + sStrArray[i].Len();
                                    i++;
                                }
                                else
                                    return nPos;		// Fehler
                            }
                            else
                            {
                                nTypeArray[i] = NF_SYMBOLTYPE_STRING;
                                nPos = nPos + sStrArray[i].Len();
                                i++;
                            }
                        }
                        break;
                        default:					// andere Dels
                        {
                            if (eScannedType == NUMBERFORMAT_PERCENT &&
                                    cHere == '%')
                                nTypeArray[i] = NF_SYMBOLTYPE_PERCENT;
                            else
                                nTypeArray[i] = NF_SYMBOLTYPE_STRING;
                            nPos = nPos + sStrArray[i].Len();
                            i++;
                        }
                        break;
                    }								// of switch (Del)
                }									// of else Del
                else if ( nTypeArray[i] == NF_SYMBOLTYPE_COMMENT )
                {
                    String& rStr = sStrArray[i];
                    nPos = nPos + rStr.Len();
                    SvNumberformat::EraseCommentBraces( rStr );
                    rComment += rStr;
                    nTypeArray[i] = NF_SYMBOLTYPE_EMPTY;
                    nAnzResStrings--;
                    i++;
                }
                else
                {
                    DBG_ERRORFILE( "unknown NF_SYMBOLTYPE_..." );
                    nPos = nPos + sStrArray[i].Len();
                    i++;
                }
            }                                  		// of while
            if (eScannedType == NUMBERFORMAT_FRACTION)
            {
                if (bFrac)
                    nCntExp = nCounter;
                else if (bBlank)
                    nCntPost = nCounter;
                else
                    nCntPre = nCounter;
            }
            else
            {
                if (bExp)
                    nCntExp = nCounter;
                else if (bDecSep)
                    nCntPost = nCounter;
                else
                    nCntPre = nCounter;
            }
            if (nThousand == 0 && bThousand)		// Expansion Tausenderpunkt:
            {
                USHORT nMaxPos;
                if (bFrac)
                {
                    if (bBlank)
                        nMaxPos = nBlankPos;
                    else
                        nMaxPos = 0;				// keine Expansion
                }
                else if (bDecSep)					// , vorhanden
                    nMaxPos = nDecPos;
                else if (bExp)						// E vorhanden
                    nMaxPos = nExpPos;
                else								// sonst bis Ende
                    nMaxPos = i;
                i = 0;
                long nCount = nCntPre;
                while (i < nMaxPos && nTypeArray[i] != NF_SYMBOLTYPE_THSEP)					// nur bis zum ,
                {
                    if (nTypeArray[i] == NF_SYMBOLTYPE_DIGIT)
                        nCount -= sStrArray[i].Len();
                    i++;
                }
                USHORT nPosThSep = i;				// Position merken
                i++;								// Ziffern hinter .
                xub_StrLen nFill = 0;
                if (nCount > 0)						// muesste immer sein
                    nFill = xub_StrLen(nCount % 3);
                if (nFill)
                {
                    nFill = 3 - nFill;
                    if (i < nMaxPos)
                        for (xub_StrLen k = 0; k < nFill; k++)
                            sStrArray[i].Insert(cThousandFill,0);
                    nCntPre = nCntPre + USHORT(nFill);
                }
                nCount = 0;							// Aufuellen mit .
                while (i < nMaxPos)					// nach hinten
                {
                    if (nTypeArray[i] == NF_SYMBOLTYPE_DIGIT)
                    {
                        xub_StrLen nLen = sStrArray[i].Len();
                        if (nCount+nLen > 3)
                        {							// hier muss . dazwischen
                            xub_StrLen nAnz =
                                sal::static_int_cast< xub_StrLen >(
                                    (nLen+nCount-4)/3+1);
                            xub_StrLen InPos =
                                sal::static_int_cast< xub_StrLen >(3-nCount);
                            for (xub_StrLen k = 0; k < nAnz; k++)
                            {
                                sStrArray[i].Insert(
                                        pFormatter->GetNumThousandSep(),InPos);
                                InPos += 4;
                            }
                            nCount = sStrArray[i].Len() - InPos + 3;
                        }
                        else
                            nCount += sStrArray[i].Len();
                    }
                    i++;
                }
                nCount = 0;							// Aufuellen mit .
                i = nPosThSep;						// nach vorn
                while (i > 0)
                {
                    i--;
                    if (nTypeArray[i] == NF_SYMBOLTYPE_DIGIT)
                    {
                        xub_StrLen nLen = sStrArray[i].Len();
                        if (nCount+nLen > 3)
                        {							// hier muss . dazwischen
                            xub_StrLen nAnz =
                                sal::static_int_cast< xub_StrLen >(
                                    (nLen+nCount-4)/3+1);
                            xub_StrLen InPos =
                                sal::static_int_cast< xub_StrLen >(
                                    nLen + nCount - 3);
                            for (xub_StrLen k = 0; k < nAnz; k++)
                            {
                                sStrArray[i].Insert(
                                    pFormatter->GetNumThousandSep(),InPos);
                                InPos -= 3;
                            }
                            nCount = InPos + 3;
                        }
                        else
                            nCount += sStrArray[i].Len();
                    }
                }
            }
        }
        break;										// of NUMBERFORMAT_NUMBER
        case NUMBERFORMAT_DATE:
        {
            while (i < nAnzStrings)
            {
                switch (nTypeArray[i])
                {
                    case NF_SYMBOLTYPE_BLANK:
                    case NF_SYMBOLTYPE_STAR:
                    case NF_SYMBOLTYPE_STRING:
                        nPos = nPos + sStrArray[i].Len();
                        i++;
                    break;
                    case NF_SYMBOLTYPE_COMMENT:
                    {
                        String& rStr = sStrArray[i];
                        nPos = nPos + rStr.Len();
                        SvNumberformat::EraseCommentBraces( rStr );
                        rComment += rStr;
                        nTypeArray[i] = NF_SYMBOLTYPE_EMPTY;
                        nAnzResStrings--;
                        i++;
                    }
                    break;
                    case NF_SYMBOLTYPE_DEL:
                    {
                        int nCalRet;
                        if (sStrArray[i] == sOldDateSep)
                        {
                            nTypeArray[i] = NF_SYMBOLTYPE_DATESEP;
                            nPos = nPos + sStrArray[i].Len();
                            if (bConvertMode)
                                sStrArray[i] = pFormatter->GetDateSep();
                            i++;
                        }
                        else if ( (nCalRet = FinalScanGetCalendar( nPos, i, nAnzResStrings )) != 0 )
                        {
                            if ( nCalRet < 0  )
                                return nPos;		// error
                        }
                        else
                        {
                            nTypeArray[i] = NF_SYMBOLTYPE_STRING;
                            nPos = nPos + sStrArray[i].Len();
                            i++;
                        }
                    }
                    break;
                    case NF_KEY_THAI_T :
                        bThaiT = true;
                        // fall thru
                    case NF_KEY_M:							// M
                    case NF_KEY_MM:							// MM
                    case NF_KEY_MMM:						// MMM
                    case NF_KEY_MMMM:						// MMMM
                    case NF_KEY_MMMMM:						// MMMMM
                    case NF_KEY_Q:							// Q
                    case NF_KEY_QQ:							// QQ
                    case NF_KEY_D:							// D
                    case NF_KEY_DD:							// DD
                    case NF_KEY_DDD:						// DDD
                    case NF_KEY_DDDD:						// DDDD
                    case NF_KEY_YY:							// YY
                    case NF_KEY_YYYY:						// YYYY
                    case NF_KEY_NN:							// NN
                    case NF_KEY_NNN:						// NNN
                    case NF_KEY_NNNN:						// NNNN
                    case NF_KEY_WW :						// WW
                    case NF_KEY_AAA :						// AAA
                    case NF_KEY_AAAA :						// AAAA
                    case NF_KEY_EC :						// E
                    case NF_KEY_EEC :						// EE
                    case NF_KEY_G :							// G
                    case NF_KEY_GG :						// GG
                    case NF_KEY_GGG :						// GGG
                    case NF_KEY_R :							// R
                    case NF_KEY_RR :						// RR
                        sStrArray[i] = sKeyword[nTypeArray[i]];	// tTtT -> TTTT
                        nPos = nPos + sStrArray[i].Len();
                        i++;
                    break;
                    default:							// andere Keywords
                        nTypeArray[i] = NF_SYMBOLTYPE_STRING;
                        nPos = nPos + sStrArray[i].Len();
                        i++;
                    break;
                }
            }										// of while
        }
        break;										// of NUMBERFORMAT_DATE
        case NUMBERFORMAT_TIME:
        {
            while (i < nAnzStrings)
            {
                switch (nTypeArray[i])
                {
                    case NF_SYMBOLTYPE_BLANK:
                    case NF_SYMBOLTYPE_STAR:
                    {
                        nPos = nPos + sStrArray[i].Len();
                        i++;
                    }
                    break;
                    case NF_SYMBOLTYPE_DEL:
                    {
                        switch( sStrArray[i].GetChar(0) )
                        {
                            case '0':
                            {
                                if ( Is100SecZero( i, bDecSep ) )
                                {
                                    bDecSep = TRUE;
                                    nTypeArray[i] = NF_SYMBOLTYPE_DIGIT;
                                    String& rStr = sStrArray[i];
                                    i++;
                                    nPos = nPos + sStrArray[i].Len();
                                    nCounter++;
                                    while (i < nAnzStrings &&
                                           sStrArray[i].GetChar(0) == '0')
                                    {
                                        rStr += sStrArray[i];
                                        nPos = nPos + sStrArray[i].Len();
                                        nTypeArray[i] = NF_SYMBOLTYPE_EMPTY;
                                        nAnzResStrings--;
                                        nCounter++;
                                        i++;
                                    }
                                }
                                else
                                    return nPos;
                            }
                            break;
                            case '#':
                            case '?':
                                return nPos;
                            case '[':
                            {
                                if (bThousand)				// doppelt
                                    return nPos;
                                bThousand = TRUE;			// bei Time frei
                                sal_Unicode cChar = pChrCls->upper( NextChar(i) ).GetChar(0);
                                if ( cChar == cOldKeyH )
                                    nThousand = 1;		// H
                                else if ( cChar == cOldKeyMI )
                                    nThousand = 2;		// M
                                else if ( cChar == cOldKeyS )
                                    nThousand = 3;		// S
                                else
                                    return nPos;
                                nPos = nPos + sStrArray[i].Len();
                                i++;
                            }
                            break;
                            case ']':
                            {
                                if (!bThousand)				// kein [ vorher
                                    return nPos;
                                nPos = nPos + sStrArray[i].Len();
                                i++;
                            }
                            break;
                            default:
                            {
                                nPos = nPos + sStrArray[i].Len();
                                if ( sStrArray[i] == sOldTimeSep )
                                {
                                    nTypeArray[i] = NF_SYMBOLTYPE_TIMESEP;
                                    if ( bConvertMode )
                                        sStrArray[i] = pLoc->getTimeSep();
                                }
                                else if ( sStrArray[i] == sOldTime100SecSep )
                                {
                                    bDecSep = TRUE;
                                    nTypeArray[i] = NF_SYMBOLTYPE_TIME100SECSEP;
                                    if ( bConvertMode )
                                        sStrArray[i] = pLoc->getTime100SecSep();
                                }
                                else
                                    nTypeArray[i] = NF_SYMBOLTYPE_STRING;
                                i++;
                            }
                            break;
                        }
                    }
                    break;
                    case NF_SYMBOLTYPE_STRING:
                    {
                        nPos = nPos + sStrArray[i].Len();
                        i++;
                    }
                    break;
                    case NF_SYMBOLTYPE_COMMENT:
                    {
                        String& rStr = sStrArray[i];
                        nPos = nPos + rStr.Len();
                        SvNumberformat::EraseCommentBraces( rStr );
                        rComment += rStr;
                        nTypeArray[i] = NF_SYMBOLTYPE_EMPTY;
                        nAnzResStrings--;
                        i++;
                    }
                    break;
                    case NF_KEY_AMPM:						// AM/PM
                    case NF_KEY_AP:							// A/P
                    {
                        bExp = TRUE;					// missbraucht fuer A/P
                        sStrArray[i] = sKeyword[nTypeArray[i]];	// tTtT -> TTTT
                        nPos = nPos + sStrArray[i].Len();
                        i++;
                    }
                    break;
                    case NF_KEY_THAI_T :
                        bThaiT = true;
                        // fall thru
                    case NF_KEY_MI:							// M
                    case NF_KEY_MMI:						// MM
                    case NF_KEY_H:							// H
                    case NF_KEY_HH:							// HH
                    case NF_KEY_S:							// S
                    case NF_KEY_SS:							// SS
                    {
                        sStrArray[i] = sKeyword[nTypeArray[i]];	// tTtT -> TTTT
                        nPos = nPos + sStrArray[i].Len();
                        i++;
                    }
                    break;
                    default:							// andere Keywords
                    {
                        nTypeArray[i] = NF_SYMBOLTYPE_STRING;
                        nPos = nPos + sStrArray[i].Len();
                        i++;
                    }
                    break;
                }
            }                   					// of while
            nCntPost = nCounter;					// Zaehler der Nullen
            if (bExp)
                nCntExp = 1;						// merkt AM/PM
        }
        break;										// of NUMBERFORMAT_TIME
        case NUMBERFORMAT_DATETIME:
        {
            BOOL bTimePart = FALSE;
            while (i < nAnzStrings)
            {
                switch (nTypeArray[i])
                {
                    case NF_SYMBOLTYPE_BLANK:
                    case NF_SYMBOLTYPE_STAR:
                    case NF_SYMBOLTYPE_STRING:
                        nPos = nPos + sStrArray[i].Len();
                        i++;
                    break;
                    case NF_SYMBOLTYPE_COMMENT:
                    {
                        String& rStr = sStrArray[i];
                        nPos = nPos + rStr.Len();
                        SvNumberformat::EraseCommentBraces( rStr );
                        rComment += rStr;
                        nTypeArray[i] = NF_SYMBOLTYPE_EMPTY;
                        nAnzResStrings--;
                        i++;
                    }
                    break;
                    case NF_SYMBOLTYPE_DEL:
                    {
                        int nCalRet;
                        if ( (nCalRet = FinalScanGetCalendar( nPos, i, nAnzResStrings )) != 0 )
                        {
                            if ( nCalRet < 0  )
                                return nPos;		// error
                        }
                        else
                        {
                            switch( sStrArray[i].GetChar(0) )
                            {
                                case '0':
                                {
                                    if ( bTimePart && Is100SecZero( i, bDecSep ) )
                                    {
                                        bDecSep = TRUE;
                                        nTypeArray[i] = NF_SYMBOLTYPE_DIGIT;
                                        String& rStr = sStrArray[i];
                                        i++;
                                        nPos = nPos + sStrArray[i].Len();
                                        nCounter++;
                                        while (i < nAnzStrings &&
                                            sStrArray[i].GetChar(0) == '0')
                                        {
                                            rStr += sStrArray[i];
                                            nPos = nPos + sStrArray[i].Len();
                                            nTypeArray[i] = NF_SYMBOLTYPE_EMPTY;
                                            nAnzResStrings--;
                                            nCounter++;
                                            i++;
                                        }
                                    }
                                    else
                                        return nPos;
                                }
                                break;
                                case '#':
                                case '?':
                                    return nPos;
                                default:
                                {
                                    nPos = nPos + sStrArray[i].Len();
                                    if (bTimePart)
                                    {
                                        if ( sStrArray[i] == sOldTimeSep )
                                        {
                                            nTypeArray[i] = NF_SYMBOLTYPE_TIMESEP;
                                            if ( bConvertMode )
                                                sStrArray[i] = pLoc->getTimeSep();
                                        }
                                        else if ( sStrArray[i] == sOldTime100SecSep )
                                        {
                                            bDecSep = TRUE;
                                            nTypeArray[i] = NF_SYMBOLTYPE_TIME100SECSEP;
                                            if ( bConvertMode )
                                                sStrArray[i] = pLoc->getTime100SecSep();
                                        }
                                        else
                                            nTypeArray[i] = NF_SYMBOLTYPE_STRING;
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
                                            nTypeArray[i] = NF_SYMBOLTYPE_STRING;
                                    }
                                    i++;
                                }
                            }
                        }
                    }
                    break;
                    case NF_KEY_AMPM:						// AM/PM
                    case NF_KEY_AP:							// A/P
                    {
                        bTimePart = TRUE;
                        bExp = TRUE;					// missbraucht fuer A/P
                        sStrArray[i] = sKeyword[nTypeArray[i]];	// tTtT -> TTTT
                        nPos = nPos + sStrArray[i].Len();
                        i++;
                    }
                    break;
                    case NF_KEY_MI:							// M
                    case NF_KEY_MMI:						// MM
                    case NF_KEY_H:							// H
                    case NF_KEY_HH:							// HH
                    case NF_KEY_S:							// S
                    case NF_KEY_SS:							// SS
                        bTimePart = TRUE;
                        sStrArray[i] = sKeyword[nTypeArray[i]];	// tTtT -> TTTT
                        nPos = nPos + sStrArray[i].Len();
                        i++;
                    break;
                    case NF_KEY_M:							// M
                    case NF_KEY_MM:							// MM
                    case NF_KEY_MMM:						// MMM
                    case NF_KEY_MMMM:						// MMMM
                    case NF_KEY_MMMMM:						// MMMMM
                    case NF_KEY_Q:							// Q
                    case NF_KEY_QQ:							// QQ
                    case NF_KEY_D:							// D
                    case NF_KEY_DD:							// DD
                    case NF_KEY_DDD:						// DDD
                    case NF_KEY_DDDD:						// DDDD
                    case NF_KEY_YY:							// YY
                    case NF_KEY_YYYY:						// YYYY
                    case NF_KEY_NN:							// NN
                    case NF_KEY_NNN:						// NNN
                    case NF_KEY_NNNN:						// NNNN
                    case NF_KEY_WW :						// WW
                    case NF_KEY_AAA :						// AAA
                    case NF_KEY_AAAA :						// AAAA
                    case NF_KEY_EC :						// E
                    case NF_KEY_EEC :						// EE
                    case NF_KEY_G :							// G
                    case NF_KEY_GG :						// GG
                    case NF_KEY_GGG :						// GGG
                    case NF_KEY_R :							// R
                    case NF_KEY_RR :						// RR
                        bTimePart = FALSE;
                        sStrArray[i] = sKeyword[nTypeArray[i]];	// tTtT -> TTTT
                        nPos = nPos + sStrArray[i].Len();
                        i++;
                    break;
                    case NF_KEY_THAI_T :
                        bThaiT = true;
                        sStrArray[i] = sKeyword[nTypeArray[i]];
                        nPos = nPos + sStrArray[i].Len();
                        i++;
                    break;
                    default:							// andere Keywords
                        nTypeArray[i] = NF_SYMBOLTYPE_STRING;
                        nPos = nPos + sStrArray[i].Len();
                        i++;
                    break;
                }
            }										// of while
            nCntPost = nCounter;                    // decimals (100th seconds)
            if (bExp)
                nCntExp = 1;						// merkt AM/PM
        }
        break;										// of NUMBERFORMAT_DATETIME
        default:
        break;
    }
    if (eScannedType == NUMBERFORMAT_SCIENTIFIC &&
        (nCntPre + nCntPost == 0 || nCntExp == 0))
        return nPos;
    else if (eScannedType == NUMBERFORMAT_FRACTION && (nCntExp > 8 || nCntExp == 0))
        return nPos;

    if (bThaiT && !GetNatNumModifier())
        SetNatNumModifier(1);

    if ( bConvertMode )
    {	// strings containing keywords of the target locale must be quoted, so
        // the user sees the difference and is able to edit the format string
        for ( i=0; i < nAnzStrings; i++ )
        {
            if ( nTypeArray[i] == NF_SYMBOLTYPE_STRING &&
                    sStrArray[i].GetChar(0) != '\"' )
            {
                if ( bConvertSystemToSystem && eScannedType == NUMBERFORMAT_CURRENCY )
                {	// don't stringize automatic currency, will be converted
                    if ( sStrArray[i] == sOldCurSymbol )
                        continue;	// for
                    // DM might be splitted into D and M
                    if ( sStrArray[i].Len() < sOldCurSymbol.Len() &&
                            pChrCls->toUpper( sStrArray[i], 0, 1 ).GetChar(0) ==
                            sOldCurString.GetChar(0) )
                    {
                        String aTmp( sStrArray[i] );
                        USHORT j = i + 1;
                        while ( aTmp.Len() < sOldCurSymbol.Len() &&
                                j < nAnzStrings &&
                                nTypeArray[j] == NF_SYMBOLTYPE_STRING )
                        {
                            aTmp += sStrArray[j++];
                        }
                        if ( pChrCls->upper( aTmp ) == sOldCurString )
                        {
                            sStrArray[i++] = aTmp;
                            for ( ; i<j; i++ )
                            {
                                nTypeArray[i] = NF_SYMBOLTYPE_EMPTY;
                                nAnzResStrings--;
                            }
                            i = j - 1;
                            continue;	// for
                        }
                    }
                }
                String& rStr = sStrArray[i];
                xub_StrLen nLen = rStr.Len();
                for ( xub_StrLen j=0; j<nLen; j++ )
                {
                    if ( (j == 0 || rStr.GetChar(j-1) != '\\') && GetKeyWord( rStr, j ) )
                    {
                        rStr.Insert( '\"', 0 );
                        rStr += '\"';
                        break;	// for
                    }
                }
            }
        }
    }
    // concatenate strings, remove quotes for output, and rebuild the format string
    rString.Erase();
    i = 0;
    while (i < nAnzStrings)
    {
        switch ( nTypeArray[i] )
        {
            case NF_SYMBOLTYPE_STRING :
            {
                xub_StrLen nStringPos = rString.Len();
                xub_StrLen nArrPos = 0;
                USHORT iPos = i;
                do
                {
                    if (sStrArray[i].Len() == 2 &&
                            sStrArray[i].GetChar(0) == '\\')
                    {
                        // Unescape some simple forms of symbols even in the UI
                        // visible string to prevent duplicates that differ
                        // only in notation, originating from import.
                        // e.g. YYYY-MM-DD and YYYY\-MM\-DD are identical,
                        // but 0\ 000 0 and 0 000 0 in a French locale are not.
                        sal_Unicode c = sStrArray[i].GetChar(1);
                        switch (c)
                        {
                            case '+':
                            case '-':
                                rString += c;
                                break;
                            case ' ':
                            case '.':
                            case '/':
                                if (((eScannedType & NUMBERFORMAT_DATE) == 0)
                                        && (StringEqualsChar(
                                                pFormatter->GetNumThousandSep(),
                                                c) || StringEqualsChar(
                                                    pFormatter->GetNumDecimalSep(),
                                                    c) || (c == ' ' &&
                                                        StringEqualsChar(
                                                            pFormatter->GetNumThousandSep(),
                                                            cNonBreakingSpace))))
                                    rString += sStrArray[i];
                                else if ((eScannedType & NUMBERFORMAT_DATE) &&
                                        StringEqualsChar(
                                            pFormatter->GetDateSep(), c))
                                    rString += sStrArray[i];
                                else if ((eScannedType & NUMBERFORMAT_TIME) &&
                                        (StringEqualsChar( pLoc->getTimeSep(),
                                                           c) ||
                                         StringEqualsChar(
                                             pLoc->getTime100SecSep(), c)))
                                    rString += sStrArray[i];
                                else if (eScannedType & NUMBERFORMAT_FRACTION)
                                    rString += sStrArray[i];
                                else
                                    rString += c;
                                break;
                            default:
                                rString += sStrArray[i];
                        }
                    }
                    else
                        rString += sStrArray[i];
                    if ( RemoveQuotes( sStrArray[i] ) > 0 )
                    {	// update currency up to quoted string
                        if ( eScannedType == NUMBERFORMAT_CURRENCY )
                        {	// dM -> DM  or  DM -> $  in old automatic
                            // currency formats, oh my ..., why did we ever
                            // introduce them?
                            String aTmp( pChrCls->toUpper(
                                sStrArray[iPos], nArrPos,
                                sStrArray[iPos].Len()-nArrPos ) );
                            xub_StrLen nCPos = aTmp.Search( sOldCurString );
                            if ( nCPos != STRING_NOTFOUND )
                            {
                                const String& rCur =
                                    bConvertMode && bConvertSystemToSystem ?
                                    GetCurSymbol() : sOldCurSymbol;
                                sStrArray[iPos].Replace( nArrPos+nCPos,
                                    sOldCurString.Len(), rCur );
                                rString.Replace( nStringPos+nCPos,
                                    sOldCurString.Len(), rCur );
                            }
                            nStringPos = rString.Len();
                            if ( iPos == i )
                                nArrPos = sStrArray[iPos].Len();
                            else
                                nArrPos = sStrArray[iPos].Len() + sStrArray[i].Len();
                        }
                    }
                    if ( iPos != i )
                    {
                        sStrArray[iPos] += sStrArray[i];
                        nTypeArray[i] = NF_SYMBOLTYPE_EMPTY;
                        nAnzResStrings--;
                    }
                    i++;
                } while ( i < nAnzStrings && nTypeArray[i] == NF_SYMBOLTYPE_STRING );
                if ( i < nAnzStrings )
                    i--;	// enter switch on next symbol again
                if ( eScannedType == NUMBERFORMAT_CURRENCY && nStringPos < rString.Len() )
                {	// same as above, since last RemoveQuotes
                    String aTmp( pChrCls->toUpper(
                        sStrArray[iPos], nArrPos,
                        sStrArray[iPos].Len()-nArrPos ) );
                    xub_StrLen nCPos = aTmp.Search( sOldCurString );
                    if ( nCPos != STRING_NOTFOUND )
                    {
                        const String& rCur =
                            bConvertMode && bConvertSystemToSystem ?
                            GetCurSymbol() : sOldCurSymbol;
                        sStrArray[iPos].Replace( nArrPos+nCPos,
                            sOldCurString.Len(), rCur );
                        rString.Replace( nStringPos+nCPos,
                            sOldCurString.Len(), rCur );
                    }
                }
            }
            break;
            case NF_SYMBOLTYPE_CURRENCY :
            {
                rString += sStrArray[i];
                RemoveQuotes( sStrArray[i] );
            }
            break;
            case NF_KEY_THAI_T:
                if (bThaiT && GetNatNumModifier() == 1)
                {   // Remove T from format code, will be replaced with a [NatNum1] prefix.
                    nTypeArray[i] = NF_SYMBOLTYPE_EMPTY;
                    nAnzResStrings--;
                }
                else
                    rString += sStrArray[i];
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


xub_StrLen ImpSvNumberformatScan::RemoveQuotes( String& rStr )
{
    if ( rStr.Len() > 1 )
    {
        sal_Unicode c = rStr.GetChar(0);
        xub_StrLen n;
        if ( c == '"' && rStr.GetChar( (n = xub_StrLen(rStr.Len()-1)) ) == '"' )
        {
            rStr.Erase(n,1);
            rStr.Erase(0,1);
            return 2;
        }
        else if ( c == '\\' )
        {
            rStr.Erase(0,1);
            return 1;
        }
    }
    return 0;
}


xub_StrLen ImpSvNumberformatScan::ScanFormat( String& rString, String& rComment )
{
    xub_StrLen res = Symbol_Division(rString);	//lexikalische Analyse
    if (!res)
        res = ScanType(rString);            // Erkennung des Formattyps
    if (!res)
        res = FinalScan( rString, rComment );	// Typabhaengige Endanalyse
    return res;								// res = Kontrollposition
                                            // res = 0 => Format ok
}

void ImpSvNumberformatScan::CopyInfo(ImpSvNumberformatInfo* pInfo, USHORT nAnz)
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


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
