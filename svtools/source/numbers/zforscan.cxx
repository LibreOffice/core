/*************************************************************************
 *
 *  $RCSfile: zforscan.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: er $ $Date: 2000-11-23 13:00:06 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#pragma hdrstop

#include <stdlib.h>

#ifndef _DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif
#ifndef _INTN_HXX //autogen
#include <tools/intn.hxx>
#endif
#ifndef _ISOLANG_HXX
#include <tools/isolang.hxx>
#endif
#ifndef _SYSTEM_HXX //autogen
#include <vcl/system.hxx>
#endif
#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif
#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif
#ifndef _UNOTOOLS_NUMBERFORMATCODEWRAPPER_HXX
#include <unotools/numberformatcodewrapper.hxx>
#endif

#include "iniman.hxx"
#include "zforlist.hxx"
#include "zformat.hxx"

#define _ZFORSCAN_CXX
#include "zforscan.hxx"
#undef _ZFORSCAN_CXX

ImpSvNumberformatScan::ImpSvNumberformatScan( SvNumberFormatter* pFormatterP )
{
#ifdef DOS
#else
    pFormatter = pFormatterP;
    bConvertMode = FALSE;
    //! alle Schluesselwoerter muessen aus Grossbuchstaben bestehen !!
                                                    // 0 bleibt leer!!
    sKeyword[NF_KEY_E].AssignAscii( RTL_CONSTASCII_STRINGPARAM(     "E" ) );                    // Exp.
    sKeyword[NF_KEY_AMPM].AssignAscii( RTL_CONSTASCII_STRINGPARAM(  "AM/PM" ) );                // AM/PM
    sKeyword[NF_KEY_AP].AssignAscii( RTL_CONSTASCII_STRINGPARAM(    "A/P" ) );              // AM/PM
    sKeyword[NF_KEY_MI].AssignAscii( RTL_CONSTASCII_STRINGPARAM(    "M" ) );                    // Minute
    sKeyword[NF_KEY_MMI].AssignAscii( RTL_CONSTASCII_STRINGPARAM(   "MM" ) );                   // Minute
    sKeyword[NF_KEY_S].AssignAscii( RTL_CONSTASCII_STRINGPARAM(     "S" ) );                    // Sekunde  2
    sKeyword[NF_KEY_SS].AssignAscii( RTL_CONSTASCII_STRINGPARAM(    "SS" ) );                   // Sekunde 02
    sKeyword[NF_KEY_Q].AssignAscii( RTL_CONSTASCII_STRINGPARAM(     "Q" ) );                    // Quartal
    sKeyword[NF_KEY_QQ].AssignAscii( RTL_CONSTASCII_STRINGPARAM(    "QQ" ) );                   // Quartal lang
    sKeyword[NF_KEY_NN].AssignAscii( RTL_CONSTASCII_STRINGPARAM(    "NN" ) );                   // Wochentag kurz
    sKeyword[NF_KEY_NNN].AssignAscii( RTL_CONSTASCII_STRINGPARAM(   "NNN" ) );              // Wochentag lang
    sKeyword[NF_KEY_NNNN].AssignAscii( RTL_CONSTASCII_STRINGPARAM(  "NNNN" ) );             // Wochentag lang mit Sep
    sKeyword[NF_KEY_WW].AssignAscii( RTL_CONSTASCII_STRINGPARAM(    "WW" ) );                   // Kalenderwoche
    sKeyword[NF_KEY_CCC].AssignAscii( RTL_CONSTASCII_STRINGPARAM(   "CCC" ) );              // Waehrung Bank
    SetDependentKeywords();

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
#endif
}

ImpSvNumberformatScan::~ImpSvNumberformatScan()
{
    delete pNullDate;
    Reset();
}

void ImpSvNumberformatScan::SetDependentKeywords()
{
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::i18n;

    const CharClass* pCharClass = pFormatter->GetCharClass();
    const LocaleDataWrapper* pLocaleData = pFormatter->GetLocaleData();
    // #80023# be sure to generate keywords for the loaded Locale, not for the
    // requested Locale, otherwise number format codes might not match
    lang::Locale aLoadedLocale = pLocaleData->getLoadedLocale();
    LanguageType eLang = ConvertIsoNamesToLanguage( aLoadedLocale.Language, aLoadedLocale.Country );
    NumberFormatCodeWrapper aNumberFormatCode( pFormatter->GetServiceManager(), aLoadedLocale );

    NumberFormatCode aFormat = aNumberFormatCode.getFormatCode( NF_NUMBER_STANDARD );
    sNameStandardFormat = aFormat.Code;
    sKeyword[NF_KEY_GENERAL] = pCharClass->upper( sNameStandardFormat );

    // preset new calendar keywords
    sKeyword[NF_KEY_AAA].AssignAscii( RTL_CONSTASCII_STRINGPARAM(   "AAA" ) );
    sKeyword[NF_KEY_AAAA].AssignAscii( RTL_CONSTASCII_STRINGPARAM(  "AAAA" ) );
    sKeyword[NF_KEY_EC].AssignAscii( RTL_CONSTASCII_STRINGPARAM(    "E" ) );
    sKeyword[NF_KEY_EEC].AssignAscii( RTL_CONSTASCII_STRINGPARAM(   "EE" ) );
    sKeyword[NF_KEY_G].AssignAscii( RTL_CONSTASCII_STRINGPARAM(     "G" ) );
    sKeyword[NF_KEY_GG].AssignAscii( RTL_CONSTASCII_STRINGPARAM(    "GG" ) );
    sKeyword[NF_KEY_GGG].AssignAscii( RTL_CONSTASCII_STRINGPARAM(   "GGG" ) );

    switch ( eLang )
    {
        case LANGUAGE_GERMAN:
        case LANGUAGE_GERMAN_SWISS:
        case LANGUAGE_GERMAN_AUSTRIAN:
        case LANGUAGE_GERMAN_LUXEMBOURG:
        case LANGUAGE_GERMAN_LIECHTENSTEIN:
        {
            //! all capital letters
            sKeyword[NF_KEY_M].AssignAscii( RTL_CONSTASCII_STRINGPARAM(         "M" ) );            // Monat
            sKeyword[NF_KEY_MM].AssignAscii( RTL_CONSTASCII_STRINGPARAM(        "MM" ) );           // Monat
            sKeyword[NF_KEY_MMM].AssignAscii( RTL_CONSTASCII_STRINGPARAM(       "MMM" ) );      // Monat Jan
            sKeyword[NF_KEY_MMMM].AssignAscii( RTL_CONSTASCII_STRINGPARAM(      "MMMM" ) );     // Monat Januar
            sKeyword[NF_KEY_H].AssignAscii( RTL_CONSTASCII_STRINGPARAM(         "H" ) );            // Stunde  2
            sKeyword[NF_KEY_HH].AssignAscii( RTL_CONSTASCII_STRINGPARAM(        "HH" ) );           // Stunde 02
            sKeyword[NF_KEY_T].AssignAscii( RTL_CONSTASCII_STRINGPARAM(         "T" ) );
            sKeyword[NF_KEY_TT].AssignAscii( RTL_CONSTASCII_STRINGPARAM(        "TT" ) );
            sKeyword[NF_KEY_TTT].AssignAscii( RTL_CONSTASCII_STRINGPARAM(       "TTT" ) );
            sKeyword[NF_KEY_TTTT].AssignAscii( RTL_CONSTASCII_STRINGPARAM(      "TTTT" ) );
            sKeyword[NF_KEY_JJ].AssignAscii( RTL_CONSTASCII_STRINGPARAM(        "JJ" ) );
            sKeyword[NF_KEY_JJJJ].AssignAscii( RTL_CONSTASCII_STRINGPARAM(      "JJJJ" ) );
            sKeyword[NF_KEY_BOOLEAN].AssignAscii( RTL_CONSTASCII_STRINGPARAM(   "LOGISCH" ) );
            sKeyword[NF_KEY_COLOR].AssignAscii( RTL_CONSTASCII_STRINGPARAM(     "FARBE" ) );
            sKeyword[NF_KEY_BLACK].AssignAscii( RTL_CONSTASCII_STRINGPARAM(     "SCHWARZ" ) );
            sKeyword[NF_KEY_BLUE].AssignAscii( RTL_CONSTASCII_STRINGPARAM(      "BLAU" ) );
            sKeyword[NF_KEY_GREEN] = UniString( "GR" "\xDC" "N", RTL_TEXTENCODING_ISO_8859_1 );
            sKeyword[NF_KEY_CYAN].AssignAscii( RTL_CONSTASCII_STRINGPARAM(      "CYAN" ) );
            sKeyword[NF_KEY_RED].AssignAscii( RTL_CONSTASCII_STRINGPARAM(       "ROT" ) );
            sKeyword[NF_KEY_MAGENTA].AssignAscii( RTL_CONSTASCII_STRINGPARAM(   "MAGENTA" ) );
            sKeyword[NF_KEY_BROWN].AssignAscii( RTL_CONSTASCII_STRINGPARAM(     "BRAUN" ) );
            sKeyword[NF_KEY_GREY].AssignAscii( RTL_CONSTASCII_STRINGPARAM(      "GRAU" ) );
            sKeyword[NF_KEY_YELLOW].AssignAscii( RTL_CONSTASCII_STRINGPARAM(    "GELB" ) );
            sKeyword[NF_KEY_WHITE].AssignAscii( RTL_CONSTASCII_STRINGPARAM(     "WEISS" ) );
        }
        break;
        default:
        {
            // day
            switch ( eLang )
            {
                case LANGUAGE_ITALIAN       :
                case LANGUAGE_ITALIAN_SWISS :
                    sKeyword[NF_KEY_T].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "G" ) );
                    sKeyword[NF_KEY_TT].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "GG" ) );
                    sKeyword[NF_KEY_TTT].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "GGG" ) );
                    sKeyword[NF_KEY_TTTT].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "GGGG" ) );
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
                case LANGUAGE_FRENCH_MONACO     :
                    sKeyword[NF_KEY_T].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "J" ) );
                    sKeyword[NF_KEY_TT].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "JJ" ) );
                    sKeyword[NF_KEY_TTT].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "JJJ" ) );
                    sKeyword[NF_KEY_TTTT].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "JJJJ" ) );
                break;
                case LANGUAGE_FINNISH :
                    sKeyword[NF_KEY_T].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "P" ) );
                    sKeyword[NF_KEY_TT].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "PP" ) );
                    sKeyword[NF_KEY_TTT].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "PPP" ) );
                    sKeyword[NF_KEY_TTTT].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "PPPP" ) );
                break;
                default:
                    sKeyword[NF_KEY_T].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "D" ) );
                    sKeyword[NF_KEY_TT].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "DD" ) );
                    sKeyword[NF_KEY_TTT].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "DDD" ) );
                    sKeyword[NF_KEY_TTTT].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "DDDD" ) );
            }
            // month
            switch ( eLang )
            {
                case LANGUAGE_FINNISH :
                    sKeyword[NF_KEY_M].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "K" ) );
                    sKeyword[NF_KEY_MM].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "KK" ) );
                    sKeyword[NF_KEY_MMM].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "KKK" ) );
                    sKeyword[NF_KEY_MMMM].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "KKKK" ) );
                break;
                default:
                    sKeyword[NF_KEY_M].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "M" ) );
                    sKeyword[NF_KEY_MM].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "MM" ) );
                    sKeyword[NF_KEY_MMM].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "MMM" ) );
                    sKeyword[NF_KEY_MMMM].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "MMMM" ) );
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
                case LANGUAGE_FRENCH_MONACO     :
                case LANGUAGE_PORTUGUESE           :
                case LANGUAGE_PORTUGUESE_BRAZILIAN :
                case LANGUAGE_SPANISH             :
                case LANGUAGE_SPANISH_MEXICAN     :
                case LANGUAGE_SPANISH_MODERN      :
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
                    sKeyword[NF_KEY_JJ].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "AA" ) );
                    sKeyword[NF_KEY_JJJJ].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "AAAA" ) );
                    // must exchange the day of week name code, same as Xcl
                    sKeyword[NF_KEY_AAA].AssignAscii( RTL_CONSTASCII_STRINGPARAM(   "OOO" ) );
                    sKeyword[NF_KEY_AAAA].AssignAscii( RTL_CONSTASCII_STRINGPARAM(  "OOOO" ) );
                break;
                case LANGUAGE_DUTCH         :
                case LANGUAGE_DUTCH_BELGIAN :
                    sKeyword[NF_KEY_JJ].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "JJ" ) );
                    sKeyword[NF_KEY_JJJJ].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "JJJJ" ) );
                break;
                case LANGUAGE_FINNISH :
                    sKeyword[NF_KEY_JJ].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "VV" ) );
                    sKeyword[NF_KEY_JJJJ].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "VVVV" ) );
                break;
                default:
                    sKeyword[NF_KEY_JJ].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "YY" ) );
                    sKeyword[NF_KEY_JJJJ].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "YYYY" ) );
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
            sKeyword[NF_KEY_COLOR].AssignAscii( RTL_CONSTASCII_STRINGPARAM(     "COLOR" ) );
            sKeyword[NF_KEY_BLACK].AssignAscii( RTL_CONSTASCII_STRINGPARAM(     "BLACK" ) );
            sKeyword[NF_KEY_BLUE].AssignAscii( RTL_CONSTASCII_STRINGPARAM(      "BLUE" ) );
            sKeyword[NF_KEY_GREEN].AssignAscii( RTL_CONSTASCII_STRINGPARAM(     "GREEN" ) );
            sKeyword[NF_KEY_CYAN].AssignAscii( RTL_CONSTASCII_STRINGPARAM(      "CYAN" ) );
            sKeyword[NF_KEY_RED].AssignAscii( RTL_CONSTASCII_STRINGPARAM(       "RED" ) );
            sKeyword[NF_KEY_MAGENTA].AssignAscii( RTL_CONSTASCII_STRINGPARAM(   "MAGENTA" ) );
            sKeyword[NF_KEY_BROWN].AssignAscii( RTL_CONSTASCII_STRINGPARAM(     "BROWN" ) );
            sKeyword[NF_KEY_GREY].AssignAscii( RTL_CONSTASCII_STRINGPARAM(      "GREY" ) );
            sKeyword[NF_KEY_YELLOW].AssignAscii( RTL_CONSTASCII_STRINGPARAM(    "YELLOW" ) );
            sKeyword[NF_KEY_WHITE].AssignAscii( RTL_CONSTASCII_STRINGPARAM(     "WHITE" ) );
        }
        break;
    }

    // boolean keyords
    sKeyword[NF_KEY_TRUE] = pCharClass->upper( pLocaleData->getTrueWord() );
    if ( !sKeyword[NF_KEY_TRUE].Len() )
    {
        DBG_ERRORFILE( "SetDependentKeywords: TRUE_WORD?" );
        sKeyword[NF_KEY_TRUE].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "TRUE" ) );
    }
    sKeyword[NF_KEY_FALSE] = pCharClass->upper( pLocaleData->getFalseWord() );
    if ( !sKeyword[NF_KEY_FALSE].Len() )
    {
        DBG_ERRORFILE( "SetDependentKeywords: FALSE_WORD?" );
        sKeyword[NF_KEY_FALSE].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "FALSE" ) );
    }
    // quarter
    sKeyword[NF_KEY_QUARTER] = pLocaleData->getQuarterWord();
    if ( !sKeyword[NF_KEY_QUARTER].Len() )
    {
        DBG_ERRORFILE( "SetDependentKeywords: QUARTER_WORD?" );
        sKeyword[NF_KEY_QUARTER].AssignAscii( RTL_CONSTASCII_STRINGPARAM( "quarter" ) );
    }

    // currency symbol
    sCurString = pCharClass->upper( pLocaleData->getCurrSymbol() );
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
    USHORT i = 0;
    while (i < SC_MAX_ANZ_STANDARD_FARBEN &&
           sString != sKeyword[NF_KEY_FIRSTCOLOR+i] )
        i++;
    if (i >= SC_MAX_ANZ_STANDARD_FARBEN)
    {
        const String& rColorWord = sKeyword[NF_KEY_COLOR];
        xub_StrLen nPos = sString.Match(rColorWord);
        if (nPos > 0)
        {
            sStr.Erase(0, nPos);
            sStr.EraseLeadingChars();
            sStr.EraseTrailingChars();
            if (bConvertMode)
            {
                pFormatter->ChangeIntl(eNewLnge);
                sStr.Insert(rColorWord,0);  // Color -> FARBE
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
            sStr = sKeyword[NF_KEY_FIRSTCOLOR+i];           // red -> rot
            pFormatter->ChangeIntl(eTmpLnge);
        }
        else
            sStr = sKeyword[NF_KEY_FIRSTCOLOR+i];

        return &(StandardColor[i]);
    }
}

void ImpSvNumberformatScan::ChangeIntl()
{
    SetDependentKeywords();
}


short ImpSvNumberformatScan::GetKeyWord( const String& sSymbol, xub_StrLen nPos )
{
    String sString = pFormatter->GetCharClass()->toUpper( sSymbol, nPos, sSymbol.Len() - nPos );
    // #77026# for the Xcl perverts: the GENERAL keyword is recognized anywhere
    if ( sString.Search( sKeyword[NF_KEY_GENERAL] ) == 0 )
        return NF_KEY_GENERAL;
    //! MUST be a reverse search to find longer strings first
    short i = NF_KEYWORD_ENTRIES_COUNT-1;
    BOOL bFound;
    while ( i > NF_KEY_LASTKEYWORD_SO5 && !(bFound = (sString.Search(sKeyword[i]) == 0)) )
        i--;
    // new keywords take precedence over old keywords
    if ( !bFound )
    {   // skip the gap of colors et al between new and old keywords and search on
        i = NF_KEY_LASTKEYWORD;
        while ( i > 0 && sString.Search(sKeyword[i]) != 0 )
            i--;
        if ( i > NF_KEY_LASTOLDKEYWORD && sString != sKeyword[i] )
        {   // maybe something else?
            // e.g. new NNN is found in NNNN, NNNN must search on
            short j = i - 1;
            while ( j > 0 && sString.Search(sKeyword[j]) != 0 )
                j--;
            if ( j )
                return j;
        }
    }
    return i;       // 0 => not found
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
    const CharClass* pChrCls = pFormatter->GetCharClass();
    const xub_StrLen nStart = nPos;
    short eType;
    ScanState eState = SsStart;
    sSymbol.Erase();
    while ( nPos < rStr.Len() && eState != SsStop )
    {
        sal_Unicode cToken = rStr.GetChar( nPos++ );
        switch (eState)
        {
            case SsStart:
            {
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
                        eType = SYMBOLTYPE_DEL;
                        sSymbol += cToken;
                        eState = SsStop;
                    }
                    break;
                    case '*':
                    {
                        eType = SYMBOLTYPE_STAR;
                        sSymbol += cToken;
                        eState = SsGetStar;
                    }
                    break;
                    case '_':
                    {
                        eType = SYMBOLTYPE_BLANK;
                        sSymbol += cToken;
                        eState = SsGetBlank;
                    }
                    break;
#if NF_COMMENT_IN_FORMATSTRING
                    case '{':
                        eType = SYMBOLTYPE_COMMENT;
                        eState = SsStop;
                        sSymbol.Append( rStr.GetBuffer() + (nPos-1), rStr.Len() - (nPos-1) );
                        nPos = rStr.Len();
                    break;
#endif
                    case '"':
                        eType = SYMBOLTYPE_STRING;
                        eState = SsGetString;
                        sSymbol += cToken;
                    break;
                    case '\\':
                        eType = SYMBOLTYPE_STRING;
                        eState = SsGetChar;
                        sSymbol += cToken;
                    break;
                    case '$':
                    case '+':
                    case '(':
                    case ')':
                        eType = SYMBOLTYPE_STRING;
                        eState = SsStop;
                        sSymbol += cToken;
                    break;
                    default :
                    {
                        if ( pChrCls->isLetter( rStr, nPos-1 ) )
                        {
                            short nTmpType = GetKeyWord( rStr, nPos-1 );
                            if ( nTmpType )
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
                                        case '-' :  // E+ E- combine to one symbol
                                            sSymbol += cNext;
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
                                nPos += nLen;
                                eState = SsStop;
                            }
                            else
                            {
                                eState = SsGetWord;
                                sSymbol += cToken;
                            }
                        }
                        else
                        {
                            eType = SYMBOLTYPE_STRING;
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
                    {   // beginning of keyword, stop scan and put back
                        eType = SYMBOLTYPE_STRING;
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
                        case '.':                       // Sf. currency
                        {
                            String TestStr = pChrCls->upper(sSymbol);
                            TestStr += cToken;
                            if ( TestStr == sCurString )
                            {
                                sSymbol += cToken;
                                eState = SsStop;
                                eType = SYMBOLTYPE_STRING;
                            }
                        }
                        break;
                        case '/':                       // AM/PM, A/P
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
                        eType = SYMBOLTYPE_STRING;
                    }
                }
            }
            break;
            case SsGetStar:
            {
                eState = SsStop;
                sSymbol += cToken;
                nRepPos = (nPos - nStart) - 1;  // everytime > 0!!
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
        }                                   // of switch
    }                                       // of while
    if (eState == SsGetWord)
        eType = SYMBOLTYPE_STRING;
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
        nCPos = sString.Search(sCurString,nCPos);
        if (nCPos != STRING_NOTFOUND)
        {
            // in Quotes?
            xub_StrLen nQ = SvNumberformat::GetQuoteEnd( sString, nCPos );
            if ( nQ == STRING_NOTFOUND )
            {
                sal_Unicode c;
                if ( nCPos == 0 ||
                    ((c = sString.GetChar(xub_StrLen(nCPos-1))) != '"'
                            && c != '\\') )         // dm kann durch "dm
                {                                   // \d geschuetzt werden
                    nCurrPos = nCPos;
                    nCPos = STRING_NOTFOUND;        // Abbruch
                }
                else
                    nCPos++;                        // weitersuchen
            }
            else
                nCPos = nQ + 1;                     // weitersuchen
        }
    }
    nAnzStrings = 0;
    BOOL bStar = FALSE;                 // wird bei '*'Detektion gesetzt
    Reset();
    const LocaleDataWrapper* pLoc = pFormatter->GetLocaleData();
    sOldDecSep =      pLoc->getNumDecimalSep();
    sOldThousandSep = pLoc->getNumThousandSep();
    sOldDateSep =     pLoc->getDateSep();
    sOldTimeSep =     pLoc->getTimeSep();

    // If the group separator is a Non-Breaking Space (French) continue with a
    // normal space instead so queries on space work correctly.
    // The format string is adjusted to allow both.
    // For output of the format code string the LocaleData characters are used.
    if ( sOldThousandSep.GetChar(0) == 0xA0 && sOldThousandSep.Len() == 1 )
        sOldThousandSep = ' ';

    xub_StrLen nPos = 0;
    const xub_StrLen nLen = rString.Len();
    while (nPos < nLen && nAnzStrings < SC_MAX_ANZ_FORMAT_STRINGS)
    {
        nTypeArray[nAnzStrings] = Next_Symbol(rString, nPos, sStrArray[nAnzStrings]);
        if (nTypeArray[nAnzStrings] == SYMBOLTYPE_STAR)
        {                               // Ueberwachung des '*'
            if (bStar)
                return nPos;        // Fehler: doppelter '*'
            else
                bStar = TRUE;
        }
        nAnzStrings++;
    }

    return 0;                       // 0 => ok
}

void ImpSvNumberformatScan::SkipStrings(USHORT& i, xub_StrLen& nPos)
{
    while (i < nAnzStrings && (   nTypeArray[i] == SYMBOLTYPE_STRING
                               || nTypeArray[i] == SYMBOLTYPE_BLANK
                               || nTypeArray[i] == SYMBOLTYPE_STAR) )
    {
        nPos += sStrArray[i].Len();
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
        } while ( i > 0 && nTypeArray[i] == SYMBOLTYPE_EMPTY );
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
        while (i > 0 && (   nTypeArray[i] == SYMBOLTYPE_EMPTY
                         || nTypeArray[i] == SYMBOLTYPE_STRING
                         || nTypeArray[i] == SYMBOLTYPE_STAR
                         || nTypeArray[i] == SYMBOLTYPE_BLANK ) )
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
               (   nTypeArray[i] == SYMBOLTYPE_EMPTY
                || nTypeArray[i] == SYMBOLTYPE_STRING
                || nTypeArray[i] == SYMBOLTYPE_STAR
                || nTypeArray[i] == SYMBOLTYPE_BLANK))
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
            if ( nTypeArray[i] == SYMBOLTYPE_DEL &&
                    sStrArray[i].GetChar(0) == '/')
                bStop = TRUE;
            else if ( nTypeArray[i] == SYMBOLTYPE_DEL &&
                    sStrArray[i].GetChar(0) == ' ')
                res = FALSE;
        }
        if (!bStop)                                 // kein '/'
            res = FALSE;
    }
    else
        res = FALSE;                                // kein '/' mehr

    return res;
}

void ImpSvNumberformatScan::Reset()
{
    nAnzStrings = 0;
    nAnzResStrings = 0;
#if 0
// ER 20.06.97 14:05   nicht noetig, wenn nAnzStrings beachtet wird
    for (USHORT i = 0; i < SC_MAX_ANZ_FORMAT_STRINGS; i++)
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
}

xub_StrLen ImpSvNumberformatScan::ScanType(const String& rString)
{
    xub_StrLen nPos = 0;
    USHORT i = 0;                           // durchlaeuft die Symbole
    short eNewType;                         // neu erkannter Typ

    SkipStrings(i, nPos);                   // Ausgabestrings ueberlesen
    while (i < nAnzStrings)
    {
        if (nTypeArray[i] > 0)              // Schluesselwort
        {
            switch (nTypeArray[i])
            {
                case NF_KEY_E:                          // E
                    eNewType = NUMBERFORMAT_SCIENTIFIC;
                break;
                case NF_KEY_AMPM:                       // AM,A,PM,P
                case NF_KEY_AP:
                case NF_KEY_H:                          // H
                case NF_KEY_HH:                         // HH
                case NF_KEY_SS:                         // SS
                    eNewType = NUMBERFORMAT_TIME;
                break;
                case NF_KEY_S:                          // S(Sonderf. S Schilling)
                {
/*
                    if (eScannedType != NUMBERFORMAT_TIME &&
                        ( pFormatter->GetInternational()->GetLanguage() == LANGUAGE_GERMAN_AUSTRIAN ||
                          eNewLnge == LANGUAGE_GERMAN_AUSTRIAN) )
                    {
                        sal_Unicode cChar = PreviousChar(i);
                        if (cChar == '[')           // nur [s],0 am Anfang!
                            eNewType = NUMBERFORMAT_TIME;
                        else
                            eNewType = NUMBERFORMAT_UNDEFINED;
                    }
                    else
*/
                        eNewType = NUMBERFORMAT_TIME;
                }
                break;
                case NF_KEY_M:                          // M
                case NF_KEY_MM:                         // MM
                {                           // Sonderfall: Minute oder Monat
                    USHORT nIndexPre = PreviousKeyword(i);
                    USHORT nIndexNex = NextKeyword(i);
                    sal_Unicode cChar = PreviousChar(i);
                    if (nIndexPre == NF_KEY_H   ||  // H
                        nIndexPre == NF_KEY_HH  ||  // HH
                        nIndexNex == NF_KEY_S   ||  // S
                        nIndexNex == NF_KEY_SS  ||  // SS
                        cChar == '['  )     // [M
                    {
                        eNewType = NUMBERFORMAT_TIME;
                        nTypeArray[i] -= 2;         // 6 -> 4, 7 -> 5
                    }
                    else
                        eNewType = NUMBERFORMAT_DATE;
                }
                break;
                case NF_KEY_MMM:                // MMM
                case NF_KEY_MMMM:               // MMMM
                case NF_KEY_Q:                  // Q
                case NF_KEY_QQ:                 // QQ
                case NF_KEY_T:                  // T
                case NF_KEY_TT:                 // TT
                case NF_KEY_TTT:                // TTT
                case NF_KEY_TTTT:               // TTTT
                case NF_KEY_JJ:                 // JJ
                case NF_KEY_JJJJ:               // JJJ
                case NF_KEY_NN:                 // NN
                case NF_KEY_NNN:                // NNN
                case NF_KEY_NNNN:               // NNNN
                case NF_KEY_WW :                // WW
                case NF_KEY_AAA :               // AA
                case NF_KEY_AAAA :              // AAA
                case NF_KEY_EC :                // E
                case NF_KEY_EEC :               // EE
                case NF_KEY_G :                 // G
                case NF_KEY_GG :                // GG
                case NF_KEY_GGG :               // GGG
                    eNewType = NUMBERFORMAT_DATE;
                break;
                case NF_KEY_CCC:                // CCC
                    eNewType = NUMBERFORMAT_CURRENCY;
                break;
                case NF_KEY_GENERAL:            // Standard
                    eNewType = NUMBERFORMAT_NUMBER;
                break;
                default:
                    eNewType = NUMBERFORMAT_UNDEFINED;
                break;
            }
        }
        else                                // Steuerzeichen
        {
            switch ( sStrArray[i].GetChar(0) )
            {
                case '#':
                case '?':
                    eNewType = NUMBERFORMAT_NUMBER;
                break;
                case '0':
                {
                    if (eScannedType == NUMBERFORMAT_TIME)
                    {
                        USHORT nIndexPre = PreviousKeyword(i);
                        if ((nIndexPre == NF_KEY_S || nIndexPre == NF_KEY_SS)
                             && bDecSep)                    // S, SS ','
                            eNewType = NUMBERFORMAT_TIME;
                        else
                            return nPos;                    // Fehler
                    }
                    else
                        eNewType = NUMBERFORMAT_NUMBER;
                }
                break;
                case ',':
                case '.':
                {
                    bDecSep = TRUE;                         // fuer SS,0
                    eNewType = NUMBERFORMAT_UNDEFINED;
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
                            nTypeArray[i+1] == SYMBOLTYPE_STRING &&
                            sStrArray[i+1].GetChar(0) == '$' )
                    {
                        eNewType = NUMBERFORMAT_CURRENCY;
                    }
                    else
                    {
                        USHORT nIndexNex = NextKeyword(i);
                        if (nIndexNex == NF_KEY_H   ||  // H
                            nIndexNex == NF_KEY_HH  ||  // HH
                            nIndexNex == NF_KEY_M   ||  // M
                            nIndexNex == NF_KEY_MM  ||  // MM
                            nIndexNex == NF_KEY_S   ||  // S
                            nIndexNex == NF_KEY_SS   )  // SS
                            eNewType = NUMBERFORMAT_TIME;
                        else
                            return nPos;                // Fehler
                    }
                }
                break;
                case '@':
                    eNewType = NUMBERFORMAT_TEXT;
                break;
                default:
                    eNewType = NUMBERFORMAT_UNDEFINED;
                break;
            }
        }
        if (eScannedType == NUMBERFORMAT_UNDEFINED)
            eScannedType = eNewType;
        else if (eScannedType == NUMBERFORMAT_TEXT || eNewType == NUMBERFORMAT_TEXT)
            eScannedType = NUMBERFORMAT_TEXT;               // Text bleibt immer Text
        else if (eNewType == NUMBERFORMAT_UNDEFINED)
        {                                           // bleibt wie bisher
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
                        case NUMBERFORMAT_FRACTION:         // DD/MM
                        break;
                        default:
                        {
                            if (nCurrPos != STRING_NOTFOUND)
                                eScannedType = NUMBERFORMAT_UNDEFINED;
                            else if ( sStrArray[i] != sOldDateSep )
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
                        case NUMBERFORMAT_FRACTION:         // MM/SS
                        break;
                        default:
                        {
                            if (nCurrPos != STRING_NOTFOUND)
                                eScannedType = NUMBERFORMAT_UNDEFINED;
                            else if ( sStrArray[i] != sOldTimeSep )
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
                        case NUMBERFORMAT_FRACTION:         // DD/MM
                        break;
                        default:
                        {
                            if (nCurrPos != STRING_NOTFOUND)
                                eScannedType = NUMBERFORMAT_UNDEFINED;
                            else if ( sStrArray[i] != sOldDateSep
                                   && sStrArray[i] != sOldTimeSep )
                                return nPos;
                        }
                    }
                }
                break;
                case NUMBERFORMAT_PERCENT:
                {
                    switch (eNewType)
                    {
                        case NUMBERFORMAT_NUMBER:   // nur Zahl nach Prozent
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
                        case NUMBERFORMAT_NUMBER:   // nur Zahl nach E
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
                        case NUMBERFORMAT_NUMBER:           // nur Zahl nach Bruch
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
        nPos += sStrArray[i].Len();         // Korrekturposition
        i++;
        SkipStrings(i, nPos);
    }
    if (bConvertMode)                       // hier muss Spr./Land ggfs.
    {                                       // gewechselt werden
        pFormatter->ChangeIntl(eNewLnge);
    }

    if ((eScannedType == NUMBERFORMAT_NUMBER || eScannedType == NUMBERFORMAT_UNDEFINED)
         && nCurrPos != STRING_NOTFOUND)
                                                                // Check, ob DM
        eScannedType = NUMBERFORMAT_CURRENCY;
/*
        String sString = pFormatter->GetCharClass()->upper(rString);
        USHORT nCPos = 0;
        while (nCPos != STRING_NOTFOUND)
        {
            nCPos = sString.Search(sCurString,nCPos);
            if (nCPos != STRING_NOTFOUND)
            {
                if ( nCPos == 0 ||
                    (nCPos > 0 && sString[(USHORT)(nCPos-1)] != '"'
                               && sString[(USHORT)(nCPos-1)] != '\\') )// dm kann durch "dm
                {                                       // \d geschuetzt werden
                    eScannedType = NUMBERFORMAT_CURRENCY;
                    nCPos = STRING_NOTFOUND;            // Abbruch
                }
                else
                    nCPos++;                            // weitersuchen
            }
        }
    }
*/
    if (eScannedType == NUMBERFORMAT_UNDEFINED)
        eScannedType = NUMBERFORMAT_DEFINED;
//      eScannedType = NUMBERFORMAT_TEXT;
    return 0;                               // Alles ok
}

xub_StrLen ImpSvNumberformatScan::FinalScan( String& rString, String& rComment )
{
    const International* pIntl = pFormatter->GetInternational();
    const LocaleDataWrapper* pLoc = pFormatter->GetLocaleData();

    xub_StrLen nPos = 0;                    // Korrekturposition
    USHORT i = 0;                           // durchlaeuft die Symbole
    USHORT nCounter = 0;                    // Zaehlt Ziffern
    nAnzResStrings = nAnzStrings;           // Zaehlt uebrigbleibende Symbole
    bDecSep = FALSE;                        // falls schon in TypeCeck benutzt

    switch (eScannedType)
    {
        case NUMBERFORMAT_TEXT:
        case NUMBERFORMAT_DEFINED:
        {
            while (i < nAnzStrings)
            {
                switch (nTypeArray[i])
                {
                    case SYMBOLTYPE_BLANK:
                    case SYMBOLTYPE_STAR:
                    break;
                    case SYMBOLTYPE_COMMENT:
                    {
                        String& rStr = sStrArray[i];
                        nPos += rStr.Len();
                        SvNumberformat::EraseCommentBraces( rStr );
                        rComment += rStr;
                        nTypeArray[i] = SYMBOLTYPE_EMPTY;
                        nAnzResStrings--;
                    }
                    break;
                    case NF_KEY_GENERAL :   // #77026# "General" is the same as "@"
                    break;
                    default:
                    {
                        if ( nTypeArray[i] != SYMBOLTYPE_DEL ||
                                sStrArray[i].GetChar(0) != '@' )
                            nTypeArray[i] = SYMBOLTYPE_STRING;
                    }
                    break;
                }
                nPos += sStrArray[i].Len();
                i++;
            }                                       // of while
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
                if (eScannedType == NUMBERFORMAT_FRACTION &&    // special case
                    nTypeArray[i] == SYMBOLTYPE_DEL &&          // # ### #/#
                    StringEqualsChar( sOldThousandSep, ' ' ) && // e.g. France or Sweden
                    StringEqualsChar( sStrArray[i], ' ' ) &&
                    !bFrac                          &&
                    IsLastBlankBeforeFrac(i) )
                {
                    nTypeArray[i] = SYMBOLTYPE_STRING;          // del->string
                }                                               // kein Taus.p.


                if (nTypeArray[i] == SYMBOLTYPE_BLANK   ||
                    nTypeArray[i] == SYMBOLTYPE_STAR    ||
                    nTypeArray[i] == NF_KEY_CCC         ||  // CCC
                    nTypeArray[i] == NF_KEY_GENERAL )       // Standard
                {
                    if (nTypeArray[i] == NF_KEY_GENERAL)
                    {
                        nThousand = FLAG_STANDARD_IN_FORMAT;
                        if ( bConvertMode )
                            sStrArray[i] = sNameStandardFormat;
                    }
                    nPos += sStrArray[i].Len();
                    i++;
                }
                else if (nTypeArray[i] == SYMBOLTYPE_STRING ||  // Strings oder
                         nTypeArray[i] > 0)                     // Keywords
                {
                    if (eScannedType == NUMBERFORMAT_SCIENTIFIC &&
                             nTypeArray[i] == NF_KEY_E)         // E+
                    {
                        if (bExp)                               // doppelt
                            return nPos;
                        bExp = TRUE;
                        nExpPos = i;
                        if (bDecSep)
                            nCntPost = nCounter;
                        else
                            nCntPre = nCounter;
                        nCounter = 0;
                        nTypeArray[i] = SYMBOLTYPE_EXP;
                    }
                    else if (eScannedType == NUMBERFORMAT_FRACTION &&
                             sStrArray[i].GetChar(0) == ' ')
                    {
                        if (!bBlank && !bFrac)  // nicht doppelt oder hinter /
                        {
                            if (bDecSep && nCounter > 0)    // Nachkommastellen
                                return nPos;                // Fehler
                            bBlank = TRUE;
                            nBlankPos = i;
                            nCntPre = nCounter;
                            nCounter = 0;
                        }
                        nTypeArray[i] = SYMBOLTYPE_FRACBLANK;
                    }
                    else
                        nTypeArray[i] = SYMBOLTYPE_STRING;
                    nPos += sStrArray[i].Len();
                    i++;
                }
                else if (nTypeArray[i] == SYMBOLTYPE_DEL)
                {
                    sal_Unicode cHere = sStrArray[i].GetChar(0);
                    switch ( cHere )
                    {
                        case '#':
                        case '0':
                        case '?':
                        {
                            if (nThousand > 0)                  // #... #
                                return nPos;                    // Fehler
                            else if (bFrac && cHere == '0')
                                return nPos;                    // 0 im Nenner
                            nTypeArray[i] = SYMBOLTYPE_DIGIT;
                            String& rStr = sStrArray[i];
                            nPos += rStr.Len();
                            i++;
                            nCounter++;
                            while (i < nAnzStrings &&
                                (sStrArray[i].GetChar(0) == '#' ||
                                    sStrArray[i].GetChar(0) == '0' ||
                                    sStrArray[i].GetChar(0) == '?')
                                )
                            {
                                rStr += sStrArray[i];
                                nPos += sStrArray[i].Len();
                                nTypeArray[i] = SYMBOLTYPE_EMPTY;
                                nAnzResStrings--;
                                nCounter++;
                                i++;
                            }
                        }
                        break;
                        case '-':
                        {
                            if ( bDecSep && nDecPos < i &&
                                    nTypeArray[nDecPos] == SYMBOLTYPE_DECSEP )
                            {   // "0,--"
                                nTypeArray[i] = SYMBOLTYPE_DIGIT;
                                String& rStr = sStrArray[i];
                                nPos += rStr.Len();
                                i++;
                                nCounter++;
                                while (i < nAnzStrings &&
                                        (sStrArray[i].GetChar(0) == '-') )
                                {
                                    rStr += sStrArray[i];
                                    nPos += sStrArray[i].Len();
                                    nTypeArray[i] = SYMBOLTYPE_EMPTY;
                                    nAnzResStrings--;
                                    nCounter++;
                                    i++;
                                }
                            }
                            else
                            {
                                nTypeArray[i] = SYMBOLTYPE_STRING;
                                nPos += sStrArray[i].Len();
                                i++;
                            }
                        }
                        break;
                        case '.':
                        case ',':
                        case '\'':
                        case ' ':
                        {
                            sal_Unicode cSep = cHere;   // remember
                            if ( StringEqualsChar( sOldThousandSep, cSep ) )
                            {
                                if (bConvertMode)
                                {
                                    sStrArray[i].Replace( 0, 1, pLoc->getNumThousandSep() );
                                }
                                // previous char with skip empty
                                sal_Unicode cPre = PreviousChar(i);
                                sal_Unicode cNext;
                                if (bExp || bBlank || bFrac)
                                {   // after E, / or ' '
                                    if ( !StringEqualsChar( sOldThousandSep, ' ' ) )
                                    {
                                        nPos += sStrArray[i].Len();
                                        nTypeArray[i] = SYMBOLTYPE_EMPTY;
                                        nAnzResStrings--;
                                        i++;                // eat it
                                    }
                                    else
                                        nTypeArray[i] = SYMBOLTYPE_STRING;
                                }
                                else if (i > 0 && i < nAnzStrings-1   &&
                                    (cPre == '#' || cPre == '0')      &&
                                    ((cNext = NextChar(i)) == '#' || cNext == '0')
                                    )                   // #.#
                                {
                                    nPos += sStrArray[i].Len();
                                    if (!bThousand)                 // only once
                                    {   // set hard, in case of Non-Breaking Space
                                        sStrArray[i].Replace( 0, 1, pLoc->getNumThousandSep() );
                                        nTypeArray[i] = SYMBOLTYPE_THSEP;
                                        bThousand = TRUE;
                                        cThousandFill = sStrArray[i+1].GetChar(0);
                                    }
                                    else                            // eat it
                                    {
                                        nTypeArray[i] = SYMBOLTYPE_EMPTY;
                                        nAnzResStrings--;
                                    }
                                    i++;
                                }
                                else if (i > 0 && (cPre == '#' || cPre == '0')
                                    && PreviousType(i) == SYMBOLTYPE_DIGIT
                                    && nThousand < FLAG_STANDARD_IN_FORMAT )
                                {                                   // #....
                                    if ( StringEqualsChar( sOldThousandSep, ' ' ) )
                                    {   // strange, those French..
                                        BOOL bFirst = TRUE;
                                        String& rStr = sStrArray[i];
                                        //  set a hard Non-Breaking Space
                                        const String& rSepF = pLoc->getNumThousandSep();
                                        while ( i < nAnzStrings
                                            && sStrArray[i] == sOldThousandSep
                                            && StringEqualsChar( sOldThousandSep, NextChar(i) ) )
                                        {   // last was a space or another space
                                            // is following => separator
                                            nPos += sStrArray[i].Len();
                                            if ( bFirst )
                                            {
                                                bFirst = FALSE;
                                                rStr.Replace( 0, 1, rSepF );
                                                nTypeArray[i] = SYMBOLTYPE_THSEP;
                                            }
                                            else
                                            {
                                                rStr += rSepF;
                                                nTypeArray[i] = SYMBOLTYPE_EMPTY;
                                                nAnzResStrings--;
                                            }
                                            nThousand++;
                                            i++;
                                        }
                                        if ( i < nAnzStrings-1
                                            && sStrArray[i] == sOldThousandSep )
                                        {   // something following last space
                                            // => space if currency contained,
                                            // else separator
                                            nPos += sStrArray[i].Len();
                                            if ( (nPos <= nCurrPos &&
                                                    nCurrPos < nPos + sStrArray[i+1].Len())
                                                || nTypeArray[i+1] == NF_KEY_CCC
                                                || (i < nAnzStrings-2 &&
                                                sStrArray[i+1].GetChar(0) == '[' &&
                                                sStrArray[i+2].GetChar(0) == '$') )
                                            {
                                                nTypeArray[i] = SYMBOLTYPE_STRING;
                                            }
                                            else
                                            {
                                                if ( bFirst )
                                                {
                                                    bFirst = FALSE;
                                                    rStr.Replace( 0, 1, rSepF );
                                                    nTypeArray[i] = SYMBOLTYPE_THSEP;
                                                }
                                                else
                                                {
                                                    rStr += rSepF;
                                                    nTypeArray[i] = SYMBOLTYPE_EMPTY;
                                                    nAnzResStrings--;
                                                }
                                                nThousand++;
                                            }
                                            i++;
                                        }
                                    }
                                    else
                                    {
                                        nTypeArray[i] = SYMBOLTYPE_THSEP;
                                        String& rStr = sStrArray[i];
                                        nPos += rStr.Len();
                                        i++;
                                        nThousand++;
                                        while ( i < nAnzStrings &&
                                            sStrArray[i] == sOldThousandSep )
                                        {
                                            nThousand++;
                                            rStr += cSep;
                                            nPos += sStrArray[i].Len();
                                            nTypeArray[i] = SYMBOLTYPE_EMPTY;
                                            nAnzResStrings--;
                                            i++;
                                        }
                                    }
                                }
                                else                    // any grsep
                                {
                                    nTypeArray[i] = SYMBOLTYPE_STRING;
                                    String& rStr = sStrArray[i];
                                    nPos += rStr.Len();
                                    i++;
                                    while ( i < nAnzStrings &&
                                        sStrArray[i] == sOldThousandSep )
                                    {
                                        rStr += sStrArray[i];
                                        nPos += sStrArray[i].Len();
                                        nTypeArray[i] = SYMBOLTYPE_EMPTY;
                                        nAnzResStrings--;
                                        i++;
                                    }
                                }
                            }
                            else if ( StringEqualsChar( sOldDecSep, cSep ) )
                            {
                                if (bConvertMode)
                                {
                                    sStrArray[i].Replace( 0, 1, pLoc->getNumDecimalSep() );
                                }
                                if (bBlank || bFrac)    // , behind /, ' '
                                    return nPos;        // error
                                else if (bExp)          // behind E
                                {
                                    nPos += sStrArray[i].Len();
                                    nTypeArray[i] = SYMBOLTYPE_EMPTY;
                                    nAnzResStrings--;
                                    i++;                // eat it
                                }
                                else if (bDecSep)       // any ,
                                {
                                    nTypeArray[i] = SYMBOLTYPE_STRING;
                                    String& rStr = sStrArray[i];
                                    nPos += rStr.Len();
                                    i++;
                                    while ( i < nAnzStrings &&
                                        sStrArray[i] == sOldDecSep )
                                    {
                                        rStr += sStrArray[i];
                                        nPos += sStrArray[i].Len();
                                        nTypeArray[i] = SYMBOLTYPE_EMPTY;
                                        nAnzResStrings--;
                                        i++;
                                    }
                                }
                                else
                                {
                                    nTypeArray[i] = SYMBOLTYPE_DECSEP;
                                    bDecSep = TRUE;
                                    nDecPos = i;
                                    nCntPre = nCounter;
                                    nCounter = 0;
                                    nPos += sStrArray[i].Len();
                                    i++;
                                }
                            }                           // of else = DecSep
                            else                        // . without meaning
                            {
                                if (cSep == ' ' &&
                                    eScannedType == NUMBERFORMAT_FRACTION &&
                                    StringEqualsChar( sStrArray[i], ' ' ) )
                                {
                                    if (!bBlank && !bFrac)  // no dups
                                    {                       // or behind /
                                        if (bDecSep && nCounter > 0)// dec.
                                            return nPos;            // error
                                        bBlank = TRUE;
                                        nBlankPos = i;
                                        nCntPre = nCounter;
                                        nCounter = 0;
                                    }
                                    nTypeArray[i] = SYMBOLTYPE_STRING;
                                    nPos += sStrArray[i].Len();
                                }
                                else
                                {
                                    nTypeArray[i] = SYMBOLTYPE_STRING;
                                    String& rStr = sStrArray[i];
                                    nPos += rStr.Len();
                                    i++;
                                    while (i < nAnzStrings &&
                                        StringEqualsChar( sStrArray[i], cSep ) )
                                    {
                                        rStr += sStrArray[i];
                                        nPos += sStrArray[i].Len();
                                        nTypeArray[i] = SYMBOLTYPE_EMPTY;
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
                                        (nTypeArray[i-1] != SYMBOLTYPE_DIGIT &&
                                         nTypeArray[i-1] != SYMBOLTYPE_EMPTY) )
                                    return nPos ? nPos : 1; // /? not allowed
                                else if (!bFrac || (bDecSep && nCounter > 0))
                                {
                                    bFrac = TRUE;
                                    nCntPost = nCounter;
                                    nCounter = 0;
                                    nTypeArray[i] = SYMBOLTYPE_FRAC;
                                    nPos += sStrArray[i].Len();
                                    i++;
                                }
                                else                // / doppelt od. , imZaehl
                                    return nPos;    // Fehler
                            }
                            else
                            {
                                nTypeArray[i] = SYMBOLTYPE_STRING;
                                nPos += sStrArray[i].Len();
                                i++;
                            }
                        }
                        break;
                        case '[' :
                        {
                            if ( eScannedType == NUMBERFORMAT_CURRENCY &&
                                    i < nAnzStrings-1 &&
                                    nTypeArray[i+1] == SYMBOLTYPE_STRING &&
                                    sStrArray[i+1].GetChar(0) == '$' )
                            {   // [$DM-xxx]
                                // ab SV_NUMBERFORMATTER_VERSION_NEW_CURR
                                nPos += sStrArray[i].Len();         // [
                                nTypeArray[i] = SYMBOLTYPE_CURRDEL;
                                nPos += sStrArray[++i].Len();       // $
                                sStrArray[i-1] += sStrArray[i];     // [$
                                nTypeArray[i] = SYMBOLTYPE_EMPTY;
                                nAnzResStrings--;
                                if ( ++i >= nAnzStrings )
                                    return nPos;        // Fehler
                                nPos += sStrArray[i].Len();         // DM
                                String& rStr = sStrArray[i];
                                String* pStr = &sStrArray[i];
                                nTypeArray[i] = SYMBOLTYPE_CURRENCY;    // wandeln
                                BOOL bHadDash = FALSE;
                                i++;
                                while ( i < nAnzStrings &&
                                        sStrArray[i].GetChar(0) != ']' )
                                {
                                    nPos += sStrArray[i].Len();
                                    if ( bHadDash )
                                    {
                                        *pStr += sStrArray[i];
                                        nTypeArray[i] = SYMBOLTYPE_EMPTY;
                                        nAnzResStrings--;
                                    }
                                    else
                                    {
                                        if ( sStrArray[i].GetChar(0) == '-' )
                                        {
                                            bHadDash = TRUE;
                                            pStr = &sStrArray[i];
                                            nTypeArray[i] = SYMBOLTYPE_CURREXT;
                                        }
                                        else
                                        {
                                            *pStr += sStrArray[i];
                                            nTypeArray[i] = SYMBOLTYPE_EMPTY;
                                            nAnzResStrings--;
                                        }
                                    }
                                    i++;
                                }
                                if ( rStr.Len() && i < nAnzStrings &&
                                        sStrArray[i].GetChar(0) == ']' )
                                {
                                    nTypeArray[i] = SYMBOLTYPE_CURRDEL;
                                    nPos += sStrArray[i].Len();
                                    i++;
                                }
                                else
                                    return nPos;        // Fehler
                            }
                            else
                            {
                                nTypeArray[i] = SYMBOLTYPE_STRING;
                                nPos += sStrArray[i].Len();
                                i++;
                            }
                        }
                        break;
                        default:                    // andere Dels
                        {
                            nTypeArray[i] = SYMBOLTYPE_STRING;
                            nPos += sStrArray[i].Len();
                            i++;
                        }
                        break;
                    }                               // of switch (Del)
                }                                   // of else Del
                else if ( nTypeArray[i] == SYMBOLTYPE_COMMENT )
                {
                    String& rStr = sStrArray[i];
                    nPos += rStr.Len();
                    SvNumberformat::EraseCommentBraces( rStr );
                    rComment += rStr;
                    nTypeArray[i] = SYMBOLTYPE_EMPTY;
                    nAnzResStrings--;
                    i++;
                }
                else
                {
                    DBG_ERRORFILE( "unknown SYMBOLTYPE_..." );
                    nPos += sStrArray[i].Len();
                    i++;
                }
            }                                       // of while
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
            if (nThousand == 0 && bThousand)        // Expansion Tausenderpunkt:
            {
                USHORT nMaxPos;
                if (bFrac)
                {
                    if (bBlank)
                        nMaxPos = nBlankPos;
                    else
                        nMaxPos = 0;                // keine Expansion
                }
                else if (bDecSep)                   // , vorhanden
                    nMaxPos = nDecPos;
                else if (bExp)                      // E vorhanden
                    nMaxPos = nExpPos;
                else                                // sonst bis Ende
                    nMaxPos = i;
                i = 0;
                long nCount = nCntPre;
                while (i < nMaxPos && nTypeArray[i] != SYMBOLTYPE_THSEP)                    // nur bis zum ,
                {
                    if (nTypeArray[i] == SYMBOLTYPE_DIGIT)
                        nCount -= sStrArray[i].Len();
                    i++;
                }
                USHORT nPosThSep = i;               // Position merken
                i++;                                // Ziffern hinter .
                xub_StrLen nFill = 0;
                if (nCount > 0)                     // muesste immer sein
                    nFill = xub_StrLen(nCount % 3);
                if (nFill)
                {
                    nFill = 3 - nFill;
                    if (i < nMaxPos)
                        for (xub_StrLen k = 0; k < nFill; k++)
                            sStrArray[i].Insert(cThousandFill,0);
                    nCntPre += USHORT(nFill);
                }
                nCount = 0;                         // Aufuellen mit .
                while (i < nMaxPos)                 // nach hinten
                {
                    if (nTypeArray[i] == SYMBOLTYPE_DIGIT)
                    {
                        xub_StrLen nLen = sStrArray[i].Len();
                        if (nCount+nLen > 3)
                        {                           // hier muss . dazwischen
                            xub_StrLen nAnz = (nLen+nCount-4)/3+1;
                            xub_StrLen InPos = 3-nCount;
                            for (xub_StrLen k = 0; k < nAnz; k++)
                            {
                                sStrArray[i].Insert(
                                        pLoc->getNumThousandSep(),InPos);
                                InPos += 4;
                            }
                            nCount = sStrArray[i].Len() - InPos + 3;
                        }
                        else
                            nCount += sStrArray[i].Len();
                    }
                    i++;
                }
                nCount = 0;                         // Aufuellen mit .
                i = nPosThSep;                      // nach vorn
                while (i > 0)
                {
                    i--;
                    if (nTypeArray[i] == SYMBOLTYPE_DIGIT)
                    {
                        xub_StrLen nLen = sStrArray[i].Len();
                        if (nCount+nLen > 3)
                        {                           // hier muss . dazwischen
                            xub_StrLen nAnz = (nLen+nCount-4)/3+1;
                            xub_StrLen InPos = nLen + nCount - 3;
                            for (xub_StrLen k = 0; k < nAnz; k++)
                            {
                                sStrArray[i].Insert(
                                    pLoc->getNumThousandSep(),InPos);
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
        break;                                      // of NUMBERFORMAT_NUMBER
        case NUMBERFORMAT_DATE:
        {
            while (i < nAnzStrings)
            {
                switch (nTypeArray[i])
                {
                    case SYMBOLTYPE_BLANK:
                    case SYMBOLTYPE_STAR:
                    case SYMBOLTYPE_STRING:
                    break;
                    case SYMBOLTYPE_COMMENT:
                    {
                        String& rStr = sStrArray[i];
                        nPos += rStr.Len();
                        SvNumberformat::EraseCommentBraces( rStr );
                        rComment += rStr;
                        nTypeArray[i] = SYMBOLTYPE_EMPTY;
                        nAnzResStrings--;
                    }
                    break;
                    case SYMBOLTYPE_DEL:
                    {
                        if (bConvertMode && sStrArray[i] == sOldDateSep)
                            sStrArray[i].Replace( 0, 1, pLoc->getDateSep() );
                        nTypeArray[i] = SYMBOLTYPE_STRING;
                    }
                    break;
                    case NF_KEY_M:                          // M
                    case NF_KEY_MM:                         // MM
                    case NF_KEY_MMM:                        // MMM
                    case NF_KEY_MMMM:                       // MMMM
                    case NF_KEY_Q:                          // Q
                    case NF_KEY_QQ:                         // QQ
                    case NF_KEY_T:                          // T
                    case NF_KEY_TT:                         // TT
                    case NF_KEY_TTT:                        // TTT
                    case NF_KEY_TTTT:                       // TTTT
                    case NF_KEY_JJ:                         // JJ
                    case NF_KEY_JJJJ:                       // JJJJ
                    case NF_KEY_NN:                         // NN
                    case NF_KEY_NNN:                        // NNN
                    case NF_KEY_NNNN:                       // NNNN
                    case NF_KEY_WW :                        // WW
                    case NF_KEY_AAA :                       // AA
                    case NF_KEY_AAAA :                      // AAA
                    case NF_KEY_EC :                        // E
                    case NF_KEY_EEC :                       // EE
                    case NF_KEY_G :                         // G
                    case NF_KEY_GG :                        // GG
                    case NF_KEY_GGG :                       // GGG
                        sStrArray[i] = sKeyword[nTypeArray[i]]; // tTtT -> TTTT
                    break;
                    default:                            // andere Keywords
                        nTypeArray[i] = SYMBOLTYPE_STRING;
                    break;
                }
                nPos += sStrArray[i].Len();
                i++;
            }                                       // of while
        }
        break;                                      // of NUMBERFORMAT_DATE
        case NUMBERFORMAT_TIME:
        {
            while (i < nAnzStrings)
            {
                switch (nTypeArray[i])
                {
                    case SYMBOLTYPE_BLANK:
                    case SYMBOLTYPE_STAR:
                    {
                        nPos += sStrArray[i].Len();
                        i++;
                    }
                    break;
                    case SYMBOLTYPE_DEL:
                    {
                        switch( sStrArray[i].GetChar(0) )
                        {
                            case '0':
                            {
                                if (bDecSep)
                                {
                                    nTypeArray[i] = SYMBOLTYPE_DIGIT;
                                    String& rStr = sStrArray[i];
                                    i++;
                                    nPos += sStrArray[i].Len();
                                    nCounter++;
                                    while (i < nAnzStrings &&
                                           sStrArray[i].GetChar(0) == '0')
                                    {
                                        rStr += sStrArray[i];
                                        nPos += sStrArray[i].Len();
                                        nTypeArray[i] = SYMBOLTYPE_EMPTY;
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
                            break;
                            case '.':
                            case ',':
                            {
                                bDecSep = TRUE;
                                nTypeArray[i] = SYMBOLTYPE_STRING;
                                nPos += sStrArray[i].Len();
                                i++;
                            }
                            break;
                            case '[':
                            {
                                if (bThousand)              // doppelt
                                    return nPos;
                                bThousand = TRUE;           // bei Time frei
                                sal_Unicode cChar = pFormatter->GetCharClass()->upper( NextChar(i) ).GetChar(0);
                                if ( cChar == sKeyword[NF_KEY_H].GetChar(0) )
                                    nThousand = 1;      // H
                                else if ( cChar == sKeyword[NF_KEY_MI].GetChar(0) )
                                    nThousand = 2;      // M
                                else if ( cChar == sKeyword[NF_KEY_S].GetChar(0) )
                                    nThousand = 3;      // S
                                else
                                    return nPos;
                                nPos += sStrArray[i].Len();
                                i++;
                            }
                            case ']':
                            {
                                if (!bThousand)             // kein [ vorher
                                    return nPos;
                                nPos += sStrArray[i].Len();
                                i++;
                            }
                            break;
                            default:
                            {
                                if (bConvertMode &&
                                    sStrArray[i] == sOldTimeSep)
                                    sStrArray[i].Replace( 0, 1, pLoc->getTimeSep() );
                                nTypeArray[i] = SYMBOLTYPE_STRING;
                                nPos += sStrArray[i].Len();
                                i++;
                            }
                            break;
                        }
                    }
                    break;
                    case SYMBOLTYPE_STRING:
                    {
                        nPos += sStrArray[i].Len();
                        i++;
                    }
                    break;
                    case SYMBOLTYPE_COMMENT:
                    {
                        String& rStr = sStrArray[i];
                        nPos += rStr.Len();
                        SvNumberformat::EraseCommentBraces( rStr );
                        rComment += rStr;
                        nTypeArray[i] = SYMBOLTYPE_EMPTY;
                        nAnzResStrings--;
                        i++;
                    }
                    break;
                    case NF_KEY_AMPM:                       // AM/PM
                    case NF_KEY_AP:                         // A/P
                    {
                        bExp = TRUE;                    // missbraucht fuer A/P
                        sStrArray[i] = sKeyword[nTypeArray[i]]; // tTtT -> TTTT
                        nPos += sStrArray[i].Len();
                        i++;
                    }
                    break;
                    case NF_KEY_MI:                         // M
                    case NF_KEY_MMI:                        // MM
                    case NF_KEY_H:                          // H
                    case NF_KEY_HH:                         // HH
                    case NF_KEY_S:                          // S
                    case NF_KEY_SS:                         // SS
                    {
                        sStrArray[i] = sKeyword[nTypeArray[i]]; // tTtT -> TTTT
                        nPos += sStrArray[i].Len();
                        i++;
                    }
                    break;
                    default:                            // andere Keywords
                    {
                        nTypeArray[i] = SYMBOLTYPE_STRING;
                        nPos += sStrArray[i].Len();
                        i++;
                    }
                    break;
                }
            }                                       // of while
            nCntPost = nCounter;                    // Zaehler der Nullen
            if (bExp)
                nCntExp = 1;                        // merkt AM/PM
        }
        break;                                      // of NUMBERFORMAT_TIME
        case NUMBERFORMAT_DATETIME:
        {
            while (i < nAnzStrings)
            {
                switch (nTypeArray[i])
                {
                    case SYMBOLTYPE_BLANK:
                    case SYMBOLTYPE_STAR:
                    case SYMBOLTYPE_STRING:
                    break;
                    case SYMBOLTYPE_COMMENT:
                    {
                        String& rStr = sStrArray[i];
                        nPos += rStr.Len();
                        SvNumberformat::EraseCommentBraces( rStr );
                        rComment += rStr;
                        nTypeArray[i] = SYMBOLTYPE_EMPTY;
                        nAnzResStrings--;
                    }
                    break;
                    case SYMBOLTYPE_DEL:
                    {
                        if (bConvertMode && sStrArray[i] == sOldDateSep)
                            sStrArray[i].Replace( 0, 1, pLoc->getDateSep() );
                        if (bConvertMode && sStrArray[i] == sOldTimeSep)
                            sStrArray[i].Replace( 0, 1, pLoc->getTimeSep() );
                        nTypeArray[i] = SYMBOLTYPE_STRING;
                    }
                    break;
                    case NF_KEY_AMPM:                       // AM/PM
                    case NF_KEY_AP:                         // A/P
                    {
                        bExp = TRUE;                    // missbraucht fuer A/P
                        sStrArray[i] = sKeyword[nTypeArray[i]]; // tTtT -> TTTT
                    }
                    break;
                    case NF_KEY_MI:                         // M
                    case NF_KEY_MMI:                        // MM
                    case NF_KEY_H:                          // H
                    case NF_KEY_HH:                         // HH
                    case NF_KEY_S:                          // S
                    case NF_KEY_SS:                         // SS
                    case NF_KEY_M:                          // M
                    case NF_KEY_MM:                         // MM
                    case NF_KEY_MMM:                        // MMM
                    case NF_KEY_MMMM:                       // MMMM
                    case NF_KEY_Q:                          // Q
                    case NF_KEY_QQ:                         // QQ
                    case NF_KEY_T:                          // T
                    case NF_KEY_TT:                         // TT
                    case NF_KEY_TTT:                        // TTT
                    case NF_KEY_TTTT:                       // TTTT
                    case NF_KEY_JJ:                         // JJ
                    case NF_KEY_JJJJ:                       // JJJJ
                    case NF_KEY_NN:                         // NN
                    case NF_KEY_NNN:                        // NNN
                    case NF_KEY_NNNN:                       // NNNN
                    case NF_KEY_WW :                        // WW
                    case NF_KEY_AAA :                       // AA
                    case NF_KEY_AAAA :                      // AAA
                    case NF_KEY_EC :                        // E
                    case NF_KEY_EEC :                       // EE
                    case NF_KEY_G :                         // G
                    case NF_KEY_GG :                        // GG
                    case NF_KEY_GGG :                       // GGG
                        sStrArray[i] = sKeyword[nTypeArray[i]]; // tTtT -> TTTT
                    break;
                    default:                            // andere Keywords
                        nTypeArray[i] = SYMBOLTYPE_STRING;
                    break;
                }
                nPos += sStrArray[i].Len();
                i++;
            }                                       // of while
            if (bExp)
                nCntExp = 1;                        // merkt AM/PM
        }
        break;                                      // of Sc_N_F_DATETIME
        default:
        break;
    }
    if (eScannedType == NUMBERFORMAT_SCIENTIFIC &&
        (nCntPre + nCntPost == 0 || nCntExp == 0))
        return nPos;
    else if (eScannedType == NUMBERFORMAT_FRACTION && (nCntExp > 8 || nCntExp == 0))
        return nPos;
    rString.Erase();
    for (i = 0; i < nAnzStrings; i++)
    {
        if ( nTypeArray[i] != SYMBOLTYPE_EMPTY )
            rString += sStrArray[i];
        if ( sStrArray[i].Len() > 1 && (nTypeArray[i] == SYMBOLTYPE_STRING ||
                nTypeArray[i] == SYMBOLTYPE_CURRENCY) )
        {
            String& rStr = sStrArray[i];
            sal_Unicode c = rStr.GetChar(0);
            xub_StrLen n;
            if ( c == '"' && rStr.GetChar( (n = xub_StrLen(rStr.Len()-1)) ) == '"' )
            {
                rStr.Erase(n,1);
                rStr.Erase(0,1);
            }
            else if ( c == '\\' )
                rStr.Erase(0,1);
        }
    }
                                                    // Strings zusammenfassen:
    i = 0;
    while (i < nAnzStrings)
    {
        if (nTypeArray[i] == SYMBOLTYPE_STRING)
        {
            USHORT iPos = i;
            USHORT j = i;
            i++;
            while ( i < nAnzStrings && nTypeArray[i] == SYMBOLTYPE_STRING )
            {
                sStrArray[iPos] += sStrArray[i];
                nTypeArray[i] = SYMBOLTYPE_EMPTY;
                nAnzResStrings--;
                i++;
            }
            if (eScannedType == NUMBERFORMAT_CURRENCY)
            {                                           // dM -> DM
                xub_StrLen nCPos;
                String sTmpStr = pFormatter->GetCharClass()->upper(sStrArray[j]);
                nCPos = sTmpStr.Search(sCurString);
                if ( nCPos != STRING_NOTFOUND )
                {
                    const String& rCurr = pLoc->getCurrSymbol();
                    sStrArray[j].Replace( nCPos, rCurr.Len(), rCurr );
                }
            }
        }
        i++;
    }
    return 0;
}

xub_StrLen ImpSvNumberformatScan::ScanFormat( String& rString, String& rComment )
{
    xub_StrLen res = Symbol_Division(rString);  //lexikalische Analyse
    if (!res)
        res = ScanType(rString);            // Erkennung des Formattyps
    if (!res)
        res = FinalScan( rString, rComment );   // Typabhaengige Endanalyse
    return res;                             // res = Kontrollposition
                                            // res = 0 => Format ok
}

void ImpSvNumberformatScan::CopyInfo(ImpSvNumberformatInfo* pInfo, USHORT nAnz)
{
    USHORT i,j;
    j = 0;
    i = 0;
    while (i < nAnz && j < SC_MAX_ANZ_FORMAT_STRINGS)
    {
        if (nTypeArray[j] != SYMBOLTYPE_EMPTY)
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


