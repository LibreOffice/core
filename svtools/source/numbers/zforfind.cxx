/*************************************************************************
 *
 *  $RCSfile: zforfind.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:59:03 $
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

#include <ctype.h>
#include <stdlib.h>
#include <float.h>

#ifndef _INTN_HXX //autogen
#include <tools/intn.hxx>
#endif
#ifndef _DATE_HXX //autogen
#include <tools/date.hxx>
#endif
#ifndef _DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif
#ifndef _SYSTEM_HXX //autogen
#include <vcl/system.hxx>
#endif
#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif

#include "zforlist.hxx"         // NUMBERFORMAT_XXX
#include "zforscan.hxx"
#include "zformat.hxx"

#define _ZFORFIND_CXX
#include "zforfind.hxx"
#undef _ZFORFIND_CXX


// wenn's geht das schnellere isdigit benutzen
#ifdef ENABLEUNICODE
//#define MyIsdigit(c)  (pFormatter->GetCharClass()->isDigit(c))
#define MyIsdigit(c)    ((c) < 256 && isdigit((unsigned char)(c)))
#else
#define MyIsdigit(c)    (isdigit((unsigned xub_Unicode)(c)))
#endif

//---------------------------------------------------------------------------
//      Konstruktor

ImpSvNumberInputScan::ImpSvNumberInputScan( SvNumberFormatter* pFormatterP )
{
    pFormatter = pFormatterP;
    pNullDate = new Date(30,12,1899);
    nYear2000 = SvNumberFormatter::GetYear2000Default();
    Reset();
    ChangeIntl();
}


//---------------------------------------------------------------------------
//      Destruktor

ImpSvNumberInputScan::~ImpSvNumberInputScan()
{
    Reset();
    delete pNullDate;
}


//---------------------------------------------------------------------------
//      Reset

void ImpSvNumberInputScan::Reset()
{
#if 0
// ER 16.06.97 18:56 Vorbelegung erfolgt jetzt in NumberStringDivision,
// wozu immer alles loeschen wenn einiges wieder benutzt oder gar nicht
// gebraucht wird..
    for (USHORT i = 0; i < SV_MAX_ANZ_INPUT_STRINGS; i++)
    {
        sStrArray[i].Erase();
        nNums[i] = SV_MAX_ANZ_INPUT_STRINGS-1;
        IsNum[i] = FALSE;
    }
#endif
    nMonth       = 0;
    nMonthPos    = 0;
    nTimePos     = 0;
    nSign        = 0;
    nESign       = 0;
    nDecPos      = 0;
    nNegCheck    = 0;
    nAnzStrings  = 0;
    nAnzNums     = 0;
    nThousand    = 0;
    eScannedType = NUMBERFORMAT_UNDEFINED;
    nAmPm        = 0;
    nPosThousandString = 0;
    nLogical     = 0;
    nStringScanNumFor = 0;
    nStringScanSign = 0;
}


//---------------------------------------------------------------------------
//      StringToDouble
//
// nur Vorzeichenlose Dezimalzahlen

double ImpSvNumberInputScan::StringToDouble( const XubString& rStr )
{
    double fNum = 0.0;
    xub_StrLen nPos = 0;
    xub_StrLen nLen = rStr.Len();
    FASTBOOL bPreComma = TRUE;
    double fBas;

    while (nPos < nLen)
    {
        if (rStr.GetChar(nPos) == '.')
        {
            bPreComma = FALSE;
            fBas = 1.0;
        }
        else if (bPreComma)
        {
            fNum *= 10.0;
            fNum += (double) (rStr.GetChar(nPos)-'0');
        }
        else
        {
            fBas *= 10.0;
            fNum += ((double)(rStr.GetChar(nPos)-'0'))/fBas;
        }
        nPos++;
    }
    return fNum;
}


//---------------------------------------------------------------------------
//       NextNumberStringSymbol
//
// Zerlegt die Eingabe in Zahlen und Strings fuer die weitere
// Verarbeitung (Turing-Maschine).
//---------------------------------------------------------------------------
// Ausgangs Zustand = GetChar
//---------------+-------------------+-----------------------+---------------
// Alter Zustand | gelesenes Zeichen | Aktion                | Neuer Zustand
//---------------+-------------------+-----------------------+---------------
// GetChar       | Ziffer            | Symbol=Zeichen        | GetValue
//               | Sonst             | Symbol=Zeichen        | GetString
//---------------|-------------------+-----------------------+---------------
// GetValue      | Ziffer            | Symbol=Symbol+Zeichen | GetValue
//               | Sonst             | Dec(CharPos)          | Stop
//---------------+-------------------+-----------------------+---------------
// GetString     | Ziffer            | Dec(CharPos)          | Stop
//               | Sonst             | Symbol=Symbol+Zeichen | GetString
//---------------+-------------------+-----------------------+---------------

enum ScanState              // States der Turing-Maschine
{
    SsStop      = 0,
    SsStart     = 1,
    SsGetValue  = 2,
    SsGetString = 3
};

BOOL ImpSvNumberInputScan::NextNumberStringSymbol(
        const xub_Unicode*& pStr,
        XubString& rSymbol )
{
    BOOL isNumber = FALSE;
    xub_Unicode cToken;
    ScanState eState = SsStart;
    register const xub_Unicode* pHere = pStr;
    register xub_StrLen nChars = 0;

    while ( ((cToken = *pHere) != 0) && eState != SsStop)
    {
        pHere++;
        switch (eState)
        {
            case SsStart:
                if ( MyIsdigit( cToken ) )
                {
                    eState = SsGetValue;
                    isNumber = TRUE;
                }
                else
                    eState = SsGetString;
                nChars++;
                break;
            case SsGetValue:
                if ( MyIsdigit( cToken ) )
                    nChars++;
                else
                {
                    eState = SsStop;
                    pHere--;
                }
                break;
            case SsGetString:
                if ( !MyIsdigit( cToken ) )
                    nChars++;
                else
                {
                    eState = SsStop;
                    pHere--;
                }
                break;
            default:
                break;
        }   // switch
    }   // while

    if ( nChars )
        rSymbol.Assign( pStr, nChars );
    else
        rSymbol.Erase();

    pStr = pHere;

    return isNumber;
}


//---------------------------------------------------------------------------
//      SkipThousands

BOOL ImpSvNumberInputScan::SkipThousands(
        const xub_Unicode*& pStr,
        XubString& rSymbol )
{
    BOOL res = FALSE;
    xub_Unicode cToken;
    register const xub_Unicode* pHere = pStr;
    ScanState eState = SsStart;
    xub_StrLen nCounter;                                // zaehlt 3er Paare

    while ( ((cToken = *pHere) != 0) && eState != SsStop)
    {
        pHere++;
        switch (eState)
        {
            case SsStart:
                if (cToken == pFormatter->GetInternational()->GetNumThousandSep())
                {
                    nCounter = 0;
                    eState = SsGetValue;
                }
                else
                {
                    eState = SsStop;
                    pHere--;
                }
                break;
            case SsGetValue:
                if ( MyIsdigit( cToken ) )
                {
                    rSymbol += cToken;
                    nCounter++;
                    if (nCounter == 3)
                    {
                        eState = SsStart;
                        res = TRUE;                 // .000 Kombination gef.
                    }
                }
                else
                {
                    eState = SsStop;
                    pHere--;
                }
                break;
            default:
                break;
        }   // switch
    }   // while

    if (eState == SsGetValue)               // Abbruch mit weniger als 3 St.
    {
        if ( nCounter )
            rSymbol.Erase( rSymbol.Len() - nCounter, nCounter );
        pHere -= nCounter + 1;               // auch . mit zurueck
    }
    pStr = pHere;

    return res;
}


//---------------------------------------------------------------------------
//      NumberStringDivision

void ImpSvNumberInputScan::NumberStringDivision( const XubString& rString )
{
    register const xub_Unicode* pStr = rString.GetBuffer();
    register const xub_Unicode* const pEnd = pStr + rString.Len();
    while ( pStr < pEnd && nAnzStrings < SV_MAX_ANZ_INPUT_STRINGS )
    {
        if ( NextNumberStringSymbol( pStr, sStrArray[nAnzStrings] ) )
        {                                               // Zahl
            IsNum[nAnzStrings] = TRUE;
            nNums[nAnzNums] = nAnzStrings;
            nAnzNums++;
            if (nAnzStrings >= SV_MAX_ANZ_INPUT_STRINGS - 7 &&
                nPosThousandString == 0)                // nur einmal
                if ( SkipThousands( pStr, sStrArray[nAnzStrings] ) )
                    nPosThousandString = nAnzStrings;
        }
        else
        {
            IsNum[nAnzStrings] = FALSE;
        }
        nAnzStrings++;
    }
}


//---------------------------------------------------------------------------
// if rString contains rWhat at nPos

BOOL ImpSvNumberInputScan::StringContains( const XubString& rWhat,
            const XubString& rString, xub_StrLen nPos )
{
    if ( nPos + rWhat.Len() <= rString.Len() )
    {
        register const xub_Unicode* pWhat = rWhat.GetBuffer();
        register const xub_Unicode* const pEnd = pWhat + rWhat.Len();
        register const xub_Unicode* pStr = rString.GetBuffer() + nPos;
        while ( pWhat < pEnd )
        {
            if ( *pWhat != *pStr )
                return FALSE;
            pWhat++;
            pStr++;
        }
        return TRUE;
    }
    return FALSE;
}


//---------------------------------------------------------------------------
//      SkipChar
//
// ueberspringt genau das angegebene Zeichen

inline BOOL ImpSvNumberInputScan::SkipChar( xub_Unicode c, const XubString& rString,
        xub_StrLen& nPos )
{
    if ((nPos < rString.Len()) && (rString.GetChar(nPos) == c))
    {
        nPos++;
        return TRUE;
    }
    return FALSE;
}


//---------------------------------------------------------------------------
//      SkipBlanks
//
// Ueberspringt Leerzeichen

inline void ImpSvNumberInputScan::SkipBlanks( const XubString& rString,
        xub_StrLen& nPos )
{
    if ( nPos < rString.Len() )
    {
        register const xub_Unicode* p = rString.GetBuffer() + nPos;
        while ( *p == ' ' )
        {
            nPos++;
            p++;
        }
    }
}


//---------------------------------------------------------------------------
//      SkipString
//
// jump over rWhat in rString at nPos

inline BOOL ImpSvNumberInputScan::SkipString( const XubString& rWhat,
        const XubString& rString, xub_StrLen& nPos )
{
    if ( StringContains( rWhat, rString, nPos ) )
    {
        nPos += rWhat.Len();
        return TRUE;
    }
    return FALSE;
}


//---------------------------------------------------------------------------
//      GetThousandSep
//
// erkennt genau .111 als Tausenderpunkt

inline BOOL ImpSvNumberInputScan::GetThousandSep(
        const XubString& rString,
        xub_StrLen& nPos,
        USHORT nStringPos )
{
    if (   rString.Len() == 1                               // keine Blanks
        && nPos == 0                                        // safety first!
        && rString.GetChar(nPos) == pFormatter->GetInternational()->GetNumThousandSep() // '.'
        && nStringPos < nAnzStrings - 1                     // safety first!
        && IsNum[nStringPos+1]                              // Zahl folgt
        && (   sStrArray[nStringPos+1].Len() == 3           // mit 3 Stellen
            || nPosThousandString == nStringPos+1 ) )       // oder bereits zusammengefasst
    {
        nPos++;
        return TRUE;
    }
    return FALSE;
}


//---------------------------------------------------------------------------
//      GetLogical
//
// Umwandlung Text in logischen Wert
// "TRUE" =>  1:
// "FALSE"=> -1:
// sonst  =>  0:

short ImpSvNumberInputScan::GetLogical( const XubString& rString )
{
    short res;

    if (rString.Len() < 4)              // kein Platz fuer mind 4 Buch.
        res = 0;
    else
    {
        const xub_Unicode *pTrue, *pFalse;
        LanguageType eLnge = pFormatter->GetInternational()->GetLanguage();
        ImpSvNumberformatScan::GetLogicalKeywords( eLnge, pTrue, pFalse );
#if 0
// NoMoreUpperNeeded
        XubString sBool( pTrue );
        USHORT nLen = rString.Len();
        if ( nLen == sBool.Len() && pFormatter->GetInternational()->CompareEqual(
                rString, sBool, INTN_COMPARE_IGNORECASE ) )
        {                                                   // zuerst True
            res = 1;                                        // True -> 1
        }
        else
        {
            sBool = pFalse;
            if ( nLen == sBool.Len() && pFormatter->GetInternational()->CompareEqual(
                    rString, sBool, INTN_COMPARE_IGNORECASE ) )
            {                                               // dann False
                res = -1;                                   // False -> -1
            }
            else
                res = 0;                                    // sonst -> 0
        }
#else
    if ( rString == pTrue )
        res = 1;
    else if ( rString == pFalse )
        res = -1;
    else
        res = 0;
#endif
    }

    return res;
}


//---------------------------------------------------------------------------
//      GetMonth
//
// Wandelt String mit Monatsbezeichnung (JAN, Januar) in Zahl des Monats um
// gibt 0 zurueck, wenn nix gefunden wird.

short ImpSvNumberInputScan::GetMonth( const XubString& rString, xub_StrLen& nPos )
{
    short res = 0;

    if (rString.Len() > nPos)                           // nur wenn Platz
    {
        if ( !bTextInitialized )
            InitText();
//      XubString sString( pFormatter->GetCharClass()->upper(rString) );        // NoMoreUpperNeeded
        for (int i = 0; i < 12; i++)                    // 12 Monate
        {
            if ( StringContains( aUpperMonthText[i], rString, nPos ) )
            {                                           // zuerst den langen
                nPos += aUpperMonthText[i].Len();
                res = i+1;                              // lange positiv
                break;                                  // Ende for
            }
            else if ( StringContains( aUpperAbbrevMonthText[i], rString, nPos ) )
            {                                           // dann den kurzen
                nPos += aUpperAbbrevMonthText[i].Len();
                res = -(i+1);                           // kurze negativ
                break;                                  // Ende for
            }
        }
    }

    return res;                                         // kein Monat gefunden
}


//---------------------------------------------------------------------------
//      GetDayOfWeek
//
// Wandelt String mit Wochentagname (Mo, Montag) in Zahl des Wochentags um
// (enum DayOfWeek + 1 !), gibt 0 zurueck, wenn nix gefunden wird.

short ImpSvNumberInputScan::GetDayOfWeek( const XubString& rString, xub_StrLen& nPos )
{
    short res = 0;

    if (rString.Len() > nPos)                           // nur wenn Platz
    {
        if ( !bTextInitialized )
            InitText();
//      XubString sString( pFormatter->GetCharClass()->upper(rString) );        // NoMoreUpperNeeded
        for (short i = 0; i < 7; i++)                   // 7 Wochentage
        {
            if ( StringContains( aUpperDayText[i], rString, nPos ) )
            {                                           // zuerst den langen
                nPos += aUpperDayText[i].Len();
                res = i + 1;                            // lange positiv
                break;                                  // Ende for
            }
            if ( StringContains( aUpperAbbrevDayText[i], rString, nPos ) )
            {                                           // dann den kurzen
                nPos += aUpperAbbrevDayText[i].Len();
                res = -(i + 1);                         // kurze negativ
                break;                                  // Ende for
            }
        }
    }

    return res;
}


//---------------------------------------------------------------------------
//      GetCurrency
//
// Lesen eines Waehrungssysmbols
// '$'   => TRUE
// sonst => FALSE

BOOL ImpSvNumberInputScan::GetCurrency( const XubString& rString, xub_StrLen& nPos,
            const SvNumberformat* pFormat )
{
    if ( rString.Len() > nPos )
    {
        if ( !bTextInitialized )
            InitText();
//      XubString sString( pFormatter->GetCharClass()->upper(rString) );        // NoMoreUpperNeeded
        if ( StringContains( aUpperCurrSymbol, rString, nPos ) )
        {
            nPos += aUpperCurrSymbol.Len();
            return TRUE;
        }
        if ( pFormat )
        {
            XubString aSymbol, aExtension;
            if ( pFormat->GetNewCurrencySymbol( aSymbol, aExtension ) )
            {
                if ( aSymbol.Len() <= rString.Len() - nPos )
                {
                    pFormatter->GetCharClass()->toUpper( aSymbol );
                    if ( StringContains( aSymbol, rString, nPos ) )
                    {
                        nPos += aSymbol.Len();
                        return TRUE;
                    }
                }
            }
        }
    }

    return FALSE;
}


//---------------------------------------------------------------------------
//      GetTimeAmPm
//
// Lesen des Zeitsymbols (AM od. PM) f. kurze Zeitangabe
//
// Rueckgabe:
//  "AM" od. "PM" => TRUE
//  sonst         => FALSE
//
// nAmPos:
//  "AM"  =>  1
//  "PM"  => -1
//  sonst =>  0

BOOL ImpSvNumberInputScan::GetTimeAmPm( const XubString& rString, xub_StrLen& nPos )
{
//  XubString sString( pFormatter->GetCharClass()->upper(rString) );        // NoMoreUpperNeeded

    //! Internationalisierung?
    static const XubString aUpperAM( RTL_CONSTASCII_USTRINGPARAM( "AM" ) );
    static const XubString aUpperPM( RTL_CONSTASCII_USTRINGPARAM( "PM" ) );

    if ( rString.Len() > nPos )
    {
        if ( StringContains( aUpperAM, rString, nPos ) )
        {
            nAmPm = 1;
            nPos += aUpperAM.Len();
            return TRUE;
        }
        else if ( StringContains( aUpperPM, rString, nPos ) )
        {
            nAmPm = -1;
            nPos += aUpperPM.Len();
            return TRUE;
        }
    }

    return FALSE;
}


//---------------------------------------------------------------------------
//      GetDecSep
//
// Lesen eines Dezimaltrenners (',')
// ','   => TRUE
// sonst => FALSE

inline BOOL ImpSvNumberInputScan::GetDecSep( const XubString& rString, xub_StrLen& nPos )
{
    if ( rString.Len() > nPos
            && rString.GetChar(nPos) == pFormatter->GetInternational()->GetNumDecimalSep() )
    {
        nPos++;
        return TRUE;
    }
    return FALSE;
}


//---------------------------------------------------------------------------
//      GetSign
//
// Lesen eines Vorzeichens, auch Klammer !?!
// '+'   =>  1
// '-'   => -1
// '('   => -1, nNegCheck = 1
// sonst =>  0

short ImpSvNumberInputScan::GetSign( const XubString& rString, xub_StrLen& nPos )
{
    if (rString.Len() > nPos)
        switch (rString.GetChar(nPos))
        {
            case '+':
                nPos++;
                return 1;
                break;
            case '(':               // '(' aehnlich wie '-' ?!?
                nNegCheck = 1;
                //! fallthru
            case '-':
                nPos++;
                return -1;
                break;
            default:
                break;
        }

    return 0;
}


//---------------------------------------------------------------------------
//      GetESign
//
// Lesen eines Vorzeichens, gedacht fuer Exponent ?!?
// '+'   =>  1
// '-'   => -1
// sonst =>  0

short ImpSvNumberInputScan::GetESign( const XubString& rString, xub_StrLen& nPos )
{
    if (rString.Len() > nPos)
        switch (rString.GetChar(nPos))
        {
            case '+':
                nPos++;
                return 1;
                break;
            case '-':
                nPos++;
                return -1;
                break;
            default:
                break;
        }

    return 0;
}


//---------------------------------------------------------------------------
//      GetNextNumber
//
// i zaehlt Strings, j zaehlt Numbers, eigentlich sollte das SkipNumber heissen

inline BOOL ImpSvNumberInputScan::GetNextNumber( USHORT& i, USHORT& j )
{
    if ( IsNum[i] )
    {
        j++;
        i++;
        return TRUE;
    }
    return FALSE;
}


//---------------------------------------------------------------------------
//      GetTimeRef

void ImpSvNumberInputScan::GetTimeRef(
        double& fOutNumber,
        USHORT nIndex,          // j-Wert fuer den ersten Zeitstring der Eingabe (default 0)
        USHORT nAnz )           // Anzahl der Zeitstrings
{
    xub_Unicode* pChar = NULL;
    USHORT nHour;
    USHORT nMinute = 0;
    USHORT nSecond = 0;
    double fSecond100 = 0.0;
    USHORT nStartIndex = nIndex;

    if (nDecPos == 2 && nAnz == 3)                      // 20:45,5
        nHour = 0;
    else
        nHour   = (USHORT) sStrArray[nNums[nIndex++]].ToInt32();
    if (nIndex - nStartIndex < nAnz)
        nMinute = (USHORT) sStrArray[nNums[nIndex++]].ToInt32();
    if (nIndex - nStartIndex < nAnz)
        nSecond = (USHORT) sStrArray[nNums[nIndex++]].ToInt32();
    if (nIndex - nStartIndex < nAnz)
    {
        XubString s100Sec( '.' );
        s100Sec += sStrArray[nNums[nIndex]];
        fSecond100 = StringToDouble( s100Sec );
    }
    if (nAmPm == -1 && nHour != 12)             // PM
        nHour += 12;
    else if (nAmPm == 1 && nHour == 12)         // 12 AM
        nHour = 0;

    fOutNumber = ((double)nHour*3600 +
                  (double)nMinute*60 +
                  (double)nSecond +
                  fSecond100)/86400.0;
}


//---------------------------------------------------------------------------
//      ImplGetDay

USHORT ImpSvNumberInputScan::ImplGetDay( USHORT nIndex )
{
    USHORT nRes = 0;

    if (sStrArray[nNums[nIndex]].Len() <= 2)
    {
        USHORT nNum = (USHORT) sStrArray[nNums[nIndex]].ToInt32();
        if (nNum <= 31)
            nRes = nNum;
    }

    return nRes;
}


//---------------------------------------------------------------------------
//      ImplGetMonth

USHORT ImpSvNumberInputScan::ImplGetMonth( USHORT nIndex )
{
    USHORT nRes = 0;

    if (sStrArray[nNums[nIndex]].Len() <= 2)
    {
        USHORT nNum = (USHORT) sStrArray[nNums[nIndex]].ToInt32();
        if (nNum <= 12)
            nRes = nNum;
    }

    return nRes;
}


//---------------------------------------------------------------------------
//      ImplGetYear
//
// 30 -> 1930, 29 -> 2029, oder 56 -> 1756, 55 -> 1855, ...

USHORT ImpSvNumberInputScan::ImplGetYear( USHORT nIndex )
{
    USHORT nYear = 0;

    if (sStrArray[nNums[nIndex]].Len() <= 4)
    {
        nYear = (USHORT) sStrArray[nNums[nIndex]].ToInt32();
        nYear = SvNumberFormatter::ExpandTwoDigitYear( nYear, nYear2000 );
    }

    return nYear;
}


//---------------------------------------------------------------------------
//      GetDateRef

BOOL ImpSvNumberInputScan::GetDateRef( Date& aDt, USHORT& nCounter,
        const SvNumberformat* pFormat )
{
    NfEvalDateFormat eEDF;
    int nFormatOrder;
    if ( pFormat && ((pFormat->GetType() & NUMBERFORMAT_DATE) == NUMBERFORMAT_DATE) )
    {
        eEDF = pFormat->ImpGetScan().GetNumberformatter()->GetEvalDateFormat();
        switch ( eEDF )
        {
            case NF_EVALDATEFORMAT_INTL :
            case NF_EVALDATEFORMAT_FORMAT :
                nFormatOrder = 1;       // nur ein Durchlauf
            break;
            default:
                nFormatOrder = 2;
        }
    }
    else
    {
        eEDF = NF_EVALDATEFORMAT_INTL;
        nFormatOrder = 1;
    }
    BOOL res = TRUE;

    const International* pIntl = pFormatter->GetInternational();
    for ( int nTryOrder = 1; nTryOrder <= nFormatOrder; nTryOrder++ )
    {
        DateFormat DateFmt;
        switch ( eEDF )
        {
            case NF_EVALDATEFORMAT_INTL :
                DateFmt = pIntl->GetDateFormat();
            break;
            case NF_EVALDATEFORMAT_FORMAT :
                DateFmt = pFormat->GetDateOrder();
            break;
            case NF_EVALDATEFORMAT_INTL_FORMAT :
                if ( nTryOrder == 1 )
                    DateFmt = pIntl->GetDateFormat();
                else
                    DateFmt = pFormat->GetDateOrder();
            break;
            case NF_EVALDATEFORMAT_FORMAT_INTL :
                if ( nTryOrder == 2 )
                    DateFmt = pIntl->GetDateFormat();
                else
                    DateFmt = pFormat->GetDateOrder();
            break;
            default:
                DBG_ERROR( "ImpSvNumberInputScan::GetDateRef: unknown NfEvalDateFormat" );
        }

        res = TRUE;
        nCounter = 0;

        switch (nAnzNums)       // Anzahl der Zahlen im String
        {
            case 0:                 // gar keine
                if (nMonthPos)          // nur Monat (Jan)
                {
                    aDt.SetDay(1);
                    aDt.SetMonth((USHORT)Abs(nMonth));
                }
                else
                    res = FALSE;
                break;

            case 1:                 // nur eine Zahl
                nCounter = 1;
                switch (nMonthPos)  // Wo steht der Monat
                {
                    case 0:             // nicht gefunden => nur Tag eingegeben
                        aDt.SetDay(ImplGetDay(0));
                        break;
                    case 1:             // Monat am Anfang (Jan 01)
                        aDt.SetMonth((USHORT)Abs(nMonth));
                        switch (DateFmt)
                        {
                            case MDY:
                            case YMD:
                                aDt.SetDay(ImplGetDay(0));
                                break;
                            case DMY:
                                aDt.SetDay(1);
                                aDt.SetYear(ImplGetYear(0));
                                break;
                            default:
                                res = FALSE;
                                break;
                        }
                        break;
                    case 3:             // Monat am Ende (10 Jan)
                        aDt.SetMonth((USHORT)Abs(nMonth));
                        switch (DateFmt)
                        {
                            case DMY:
                                aDt.SetDay(ImplGetDay(0));
                                break;
                            case YMD:
                                aDt.SetDay(1);
                                aDt.SetYear(ImplGetYear(0));
                                break;
                            default:
                                res = FALSE;
                                break;
                        }
                        break;
                    default:
                        res = FALSE;
                        break;
                }   // switch (nMonthPos)
                break;

            case 2:                 // 2 Zahlen
                nCounter = 2;
                switch (nMonthPos)  // Wo steht der Monat
                {
                    case 0:             // nicht gefunden
                        switch (DateFmt)
                        {
                            case MDY:
                                aDt.SetDay(ImplGetDay(1));
                                aDt.SetMonth(ImplGetMonth(0));
                                break;
                            case DMY:
                                aDt.SetDay(ImplGetDay(0));
                                aDt.SetMonth(ImplGetMonth(1));
                                if (!aDt.IsValid())             // 2. Versuch
                                {
                                    aDt.SetDay(1);
                                    aDt.SetMonth(ImplGetMonth(0));
                                    aDt.SetYear(ImplGetYear(1));
                                }
                                break;
                            case YMD:
                                aDt.SetDay(ImplGetDay(1));
                                aDt.SetMonth(ImplGetMonth(0));
                                if (!aDt.IsValid())             // 2. Versuch
                                {
                                    aDt.SetDay(1);
                                    aDt.SetMonth(ImplGetMonth(1));
                                    aDt.SetYear(ImplGetYear(0));
                                }
                                break;
                            default:
                                res = FALSE;
                                break;
                        }
                        break;
                    case 1:             // Monat am Anfang (Jan 01 01)
                        switch (DateFmt)
                        {
                            case MDY:
                                aDt.SetDay(ImplGetDay(0));
                                aDt.SetMonth((USHORT)Abs(nMonth));
                                aDt.SetYear(ImplGetYear(1));
                                break;
                            default:
                                res = FALSE;
                                break;
                        }
                        break;
                    case 2:             // Monat in der Mitte (10 Jan 94)
                        aDt.SetMonth((USHORT)Abs(nMonth));
                        switch (DateFmt)
                        {
                            case DMY:
                                aDt.SetDay(ImplGetDay(0));
                                aDt.SetYear(ImplGetYear(1));
                                break;
                            case YMD:
                                aDt.SetDay(ImplGetDay(1));
                                aDt.SetYear(ImplGetYear(0));
                                break;
                            default:
                                res = FALSE;
                                break;
                        }
                        break;
                    default:            // sonst., z.B. Monat am Ende (94 10 Jan)
                        res = FALSE;
                        break;
                }   // switch (nMonthPos)
                break;

            default:                // mehr als zwei (31.12.94 8:23) (31.12. 8:23)
                switch (nMonthPos)  // Wo steht der Monat
                {
                    case 0:             // nicht gefunden
                        nCounter = 3;
                        if ( nTimePos > 1 )
                        {   // find first time number index (should only be 3 or 2 anyway)
                            for ( USHORT j = 0; j < nAnzNums; j++ )
                            {
                                if ( nNums[j] == nTimePos - 2 )
                                {
                                    nCounter = j;
                                    break;  // for
                                }
                            }
                        }
                        switch (DateFmt)
                        {
                            case MDY:
                                aDt.SetDay(ImplGetDay(1));
                                aDt.SetMonth(ImplGetMonth(0));
                                if ( nCounter > 2 )
                                    aDt.SetYear(ImplGetYear(2));
                                break;
                            case DMY:
                                aDt.SetDay(ImplGetDay(0));
                                aDt.SetMonth(ImplGetMonth(1));
                                if ( nCounter > 2 )
                                    aDt.SetYear(ImplGetYear(2));
                                break;
                            case YMD:
                                if ( nCounter > 2 )
                                    aDt.SetDay(ImplGetDay(2));
                                aDt.SetMonth(ImplGetMonth(1));
                                aDt.SetYear(ImplGetYear(0));
                                break;
                            default:
                                res = FALSE;
                                break;
                        }
                        break;
                    case 1:             // Monat am Anfang (Jan 01 01 8:23)
                        nCounter = 2;
                        switch (DateFmt)
                        {
                            case MDY:
                                aDt.SetDay(ImplGetDay(0));
                                aDt.SetMonth((USHORT)Abs(nMonth));
                                aDt.SetYear(ImplGetYear(1));
                                break;
                            default:
                                res = FALSE;
                                break;
                        }
                        break;
                    case 2:             // Monat in der Mitte (10 Jan 94 8:23)
                        nCounter = 2;
                        aDt.SetMonth((USHORT)Abs(nMonth));
                        switch (DateFmt)
                        {
                            case DMY:
                                aDt.SetDay(ImplGetDay(0));
                                aDt.SetYear(ImplGetYear(1));
                                break;
                            case YMD:
                                aDt.SetDay(ImplGetDay(1));
                                aDt.SetYear(ImplGetYear(0));
                                break;
                            default:
                                res = FALSE;
                                break;
                        }
                        break;
                    default:            // sonst., z.B. Monat am Ende (94 10 Jan 8:23)
                        nCounter = 2;
                        res = FALSE;
                        break;
                }   // switch (nMonthPos)
                break;
        }   // switch (nAnzNums)

        if ( nTryOrder < nFormatOrder )
        {
            if ( res && aDt.IsValid() )
                nTryOrder = nFormatOrder;   // break for
            else
                aDt = Date();               // naechster
        }
    }

    return res;
}


//---------------------------------------------------------------------------
//      ScanStartString
//
// ersten String analysieren
// Alles weg => TRUE
// sonst     => FALSE

BOOL ImpSvNumberInputScan::ScanStartString( const XubString& rString,
        const SvNumberformat* pFormat )
{
    xub_StrLen nPos = 0;
    short nDayOfWeek;

    SkipBlanks(rString, nPos);
    if ( nSign = GetSign(rString, nPos) )           // Vorzeichen?
        SkipBlanks(rString, nPos);

    if ( GetDecSep(rString, nPos) )                 // Dezimaltrenner (,) im Startstring
    {
        nDecPos = 1;
        SkipBlanks(rString, nPos);
    }
    else if ( GetCurrency(rString, nPos, pFormat) ) // Waehrung (DM 1)?
    {
        eScannedType = NUMBERFORMAT_CURRENCY;       // !!! es ist Geld !!!
        SkipBlanks(rString, nPos);
        if (nSign == 0)                             // noch kein Vorzeichen
            if ( nSign = GetSign(rString, nPos) )   // DM -1
                SkipBlanks(rString, nPos);
    }
    else if ( nMonth = GetMonth(rString, nPos) )    // Monat (Jan 1)?
    {
        eScannedType = NUMBERFORMAT_DATE;           // !!! es ist eine Datum !!!
        nMonthPos = 1;                              // Monat an 1. Pos
        if ( nMonth < 0 )
            SkipChar( '.', rString, nPos );         // abgekuerzt
        SkipBlanks(rString, nPos);
    }
    else if ( nDayOfWeek = GetDayOfWeek( rString, nPos ) )
    {   // Wochentag wird nur weggeparst
        eScannedType = NUMBERFORMAT_DATE;           // !!! es ist eine Datum !!!
        if ( nPos < rString.Len() )
        {
            if ( nDayOfWeek < 0 )
            {   // kurz
                if ( rString.GetChar( nPos ) == '.' )
                    ++nPos;
            }
            else
            {   // lang
                SkipBlanks(rString, nPos);
                SkipString( pFormatter->GetInternational()->GetLongDateDayOfWeekSep(), rString, nPos );
            }
            SkipBlanks(rString, nPos);
            if ( nMonth = GetMonth(rString, nPos) ) // Monat (Jan 1)?
            {
                nMonthPos = 1;                      // Monat an 1. Pos
                if ( nMonth < 0 )
                    SkipChar( '.', rString, nPos ); // abgekuerzt
                SkipBlanks(rString, nPos);
            }
        }
    }

    if (nPos < rString.Len())                       // noch nicht alles weg
    {
        // eingegebener StartString gleich StartString im Format?
        if ( !ScanStringNumFor( rString, nPos, pFormat, 0 ) )
            return FALSE;
    }

    return TRUE;
}


//---------------------------------------------------------------------------
//      ScanMidString
//
// String in der Mitte analysieren
// Alles weg => TRUE
// sonst     => FALSE

BOOL ImpSvNumberInputScan::ScanMidString(
        const XubString& rString,
        USHORT nStringPos )
{
    const International* pIntl = pFormatter->GetInternational();
    xub_StrLen nPos = 0;

    SkipBlanks(rString, nPos);
    if (GetDecSep(rString, nPos))                   // Dezimaltrenner?
    {
        if (nDecPos == 1 || nDecPos == 3)           // ,12,4 oder 1,E2,1
            return FALSE;
        else if (nDecPos == 2)                      // , doppelt 12,4,
        {
            if (bDecSepInDateSeps)                  // , auch Date Sep
            {
                if (eScannedType != NUMBERFORMAT_UNDEFINED &&
                    eScannedType != NUMBERFORMAT_DATE)  // schon anderer Typ
                    return FALSE;
                eScannedType = NUMBERFORMAT_DATE;   // !!! Es ist ein Datum !!!
                SkipBlanks(rString, nPos);
            }
            else
                return FALSE;
        }
        else
        {
            nDecPos = 2;                            // , im Mittelstring
            SkipBlanks(rString, nPos);
        }
    }

    if (SkipChar('/', rString, nPos))               // Bruch?
    {
        if (   eScannedType != NUMBERFORMAT_UNDEFINED   // schon anderes Format
            && eScannedType != NUMBERFORMAT_DATE)       // ausser Datum
            return FALSE;                           // => jan/31/1994
        else if (    eScannedType != NUMBERFORMAT_DATE      // bisher als Datum analysiert
                 && (    eSetType == NUMBERFORMAT_FRACTION  // und Vorgabe war Bruch
                     || (nAnzNums == 3                      //  oder 3 Zahlen
                         && nStringPos > 2) ) )             //   und ???
        {
            SkipBlanks(rString, nPos);
            eScannedType = NUMBERFORMAT_FRACTION;   // !!! es ist ein Bruch
        }
        else
            nPos--;                                 // '/' zurueck
    }

    if (GetThousandSep(rString, nPos, nStringPos))  // 1.000
    {
        if (   eScannedType != NUMBERFORMAT_UNDEFINED   // schon anderes Format
            && eScannedType != NUMBERFORMAT_CURRENCY)   // ausser Waehrung
            return FALSE;
        nThousand++;
    }

    SkipBlanks(rString, nPos);
    xub_Unicode cDate = pIntl->GetDateSep();
    xub_Unicode cTime = pIntl->GetTimeSep();
    if (                      SkipChar(cDate, rString, nPos)    // 10., 10-, 10/
        || ((cTime != '.') && SkipChar('.',   rString, nPos))   // TRICKY:
        || ((cTime != '/') && SkipChar('/',   rString, nPos))   // short boolean
        || ((cTime != '-') && SkipChar('-',   rString, nPos)) ) // evaluation!
    {
        if (   eScannedType != NUMBERFORMAT_UNDEFINED   // schon anderes Format
            && eScannedType != NUMBERFORMAT_DATE)       // ausser Datum
            return FALSE;
        SkipBlanks(rString, nPos);
        eScannedType = NUMBERFORMAT_DATE;           // !!! es ist ein Datum
        short nTmpMonth = GetMonth(rString, nPos);  // 10. Jan 94
        if (nMonth && nTmpMonth)                    // Monat doppelt
            return FALSE;
        if (nTmpMonth)
        {
            nMonth = nTmpMonth;
            nMonthPos = 2;                          // Monat in der Mitte
            if ( nMonth < 0 )
                SkipChar( '.', rString, nPos );     // abgekuerzt
            SkipString( pIntl->GetLongDateMonthSep(), rString, nPos );
            SkipBlanks(rString, nPos);
        }
    }

    short nTempMonth = GetMonth(rString, nPos);     // Monat in der Mitte (10 Jan 94)
    if (nTempMonth)
    {
        if (nMonth != 0)                            // Monat doppelt
            return FALSE;
        if (   eScannedType != NUMBERFORMAT_UNDEFINED   // schon anderes Format
            && eScannedType != NUMBERFORMAT_DATE)       // ausser Datum
            return FALSE;
        eScannedType = NUMBERFORMAT_DATE;           // !!! es ist ein Datum
        nMonth = nTempMonth;
        nMonthPos = 2;                              // Monat in der Mitte
        if ( nMonth < 0 )
            SkipChar( '.', rString, nPos );         // abgekuerzt
        SkipString( pIntl->GetLongDateMonthSep(), rString, nPos );
        SkipBlanks(rString, nPos);
    }

    if (    SkipChar('E', rString, nPos)            // 10E, 10e, 10,Ee
         || SkipChar('e', rString, nPos) )
    {
        if (eScannedType != NUMBERFORMAT_UNDEFINED) // schon anderes Format
            return FALSE;
        else
        {
            SkipBlanks(rString, nPos);
            eScannedType = NUMBERFORMAT_SCIENTIFIC; // !!! es ist eine Zahl im E-Format
            if (    nThousand+2 == nAnzNums         // Sonderfall 1,E2
                 && nDecPos == 2 )
                nDecPos = 3;                        // 1.100,E2 1.100.100,E3
        }
        nESign = GetESign(rString, nPos);           // Vorzeichen vom Exponent?
        SkipBlanks(rString, nPos);
    }

    if ( SkipChar(cTime, rString, nPos) )           // Zeittrenner?
    {
        if (nDecPos)                                // schon , => Fehler
            return FALSE;
        if (   (   eScannedType == NUMBERFORMAT_DATE        // bereits Datumsformat
                || eScannedType == NUMBERFORMAT_DATETIME)   // erkannt und
            && nAnzNums > 3)                                // mehr als 3 Zahlen? (31.Dez.94 8:23)
        {
            SkipBlanks(rString, nPos);
            eScannedType = NUMBERFORMAT_DATETIME;   // !!! es ist Datum mit Uhrzeit
        }
        else if (   eScannedType != NUMBERFORMAT_UNDEFINED  // schon anderes Format
                 && eScannedType != NUMBERFORMAT_TIME)      // ausser Zeit
            return FALSE;
        else
        {
            SkipBlanks(rString, nPos);
            eScannedType = NUMBERFORMAT_TIME;       // !!! es ist eine Zeit
        }
        if ( !nTimePos )
            nTimePos = nStringPos + 1;
    }

    // #68232# recognize long date separators like ", " in "September 5, 1999"
    if ( nPos < rString.Len() && eScannedType == NUMBERFORMAT_DATE
            && nMonthPos == 1 && pIntl->GetLongDateFormat() == MDY )
    {
        if ( SkipString( pIntl->GetLongDateDaySep(), rString, nPos )  )
            SkipBlanks( rString, nPos );
    }

    if (nPos < rString.Len())                       // noch nicht alles weg?
        return FALSE;

    return TRUE;
}


//---------------------------------------------------------------------------
//      ScanEndString
//
// Schlussteil analysieren
// Alles weg => TRUE
// sonst     => FALSE

BOOL ImpSvNumberInputScan::ScanEndString( const XubString& rString,
        const SvNumberformat* pFormat )
{
    const International* pIntl = pFormatter->GetInternational();
    xub_StrLen nPos = 0;

    SkipBlanks(rString, nPos);
    if (GetDecSep(rString, nPos))                   // Dezimaltrenner?
    {
        if (nDecPos == 1 || nDecPos == 3)           // ,12,4 oder 12,E4,
            return FALSE;
        else if (nDecPos == 2)                      // , doppelt 12,4,
        {
            if (bDecSepInDateSeps)                  // , auch Date Sep
            {
                if (eScannedType != NUMBERFORMAT_UNDEFINED &&
                    eScannedType != NUMBERFORMAT_DATE)  // schon anderer Typ
                    return FALSE;
                eScannedType = NUMBERFORMAT_DATE;   // !!! es ist ein Datum
                SkipBlanks(rString, nPos);
            }
            else
                return FALSE;
        }
        else
        {
            nDecPos = 3;                            // , im Endstring
            SkipBlanks(rString, nPos);
        }
    }

    if (   nSign == 0                               // Konflikt - kein Vorzeichen
        && eScannedType != NUMBERFORMAT_DATE)       // und nicht Datum
//? Uhrzeit auch abfangen ?
    {                                               // noch kein Vorzeichen
        nSign = GetSign(rString, nPos);             // 1- DM
        if (nNegCheck)                              // '(' als Vorzeichen
            return FALSE;
    }

    SkipBlanks(rString, nPos);
    if (nNegCheck && SkipChar(')', rString, nPos))  // ggf. ')' ueberlesen
    {
        nNegCheck = 0;
        SkipBlanks(rString, nPos);
    }

    if ( GetCurrency(rString, nPos, pFormat) )      // Waehrungssymbol?
    {
        if (eScannedType != NUMBERFORMAT_UNDEFINED) // DM doppelt oder
            return FALSE;
        else
        {
            SkipBlanks(rString, nPos);
            eScannedType = NUMBERFORMAT_CURRENCY;
        }                                           // hinter DM - erlaubt
        if (nSign == 0)                             // noch kein Vorzeichen
        {
            nSign = GetSign(rString, nPos);         // DM -
            SkipBlanks(rString, nPos);
            if (nNegCheck)                          // 3 DM (
                return FALSE;
        }
        if ( nNegCheck && eScannedType == NUMBERFORMAT_CURRENCY
                       && SkipChar(')', rString, nPos) )
        {
            nNegCheck = 0;                          // ggf. ')' ueberlesen
            SkipBlanks(rString, nPos);              // nur bei Waehrung
        }
    }

    if ( SkipChar('%', rString, nPos) )             // 1 %
    {
        if (eScannedType != NUMBERFORMAT_UNDEFINED) // schon anderes Format
            return FALSE;
        SkipBlanks(rString, nPos);
        eScannedType = NUMBERFORMAT_PERCENT;
    }

    xub_Unicode cTime = pIntl->GetTimeSep();
    if ( SkipChar(cTime, rString, nPos) )           // 10:
    {
        if (nDecPos)                                // schon , => Fehler
            return FALSE;
        if (eScannedType == NUMBERFORMAT_DATE && nAnzNums > 2) // 31.Dez.94 8:
        {
            SkipBlanks(rString, nPos);
            eScannedType = NUMBERFORMAT_DATETIME;
        }
        else if (eScannedType != NUMBERFORMAT_UNDEFINED &&
                 eScannedType != NUMBERFORMAT_TIME) // schon anderes Format
            return FALSE;
        else
        {
            SkipBlanks(rString, nPos);
            eScannedType = NUMBERFORMAT_TIME;
        }
    }

    xub_Unicode cDate = pIntl->GetDateSep();
    if (                      SkipChar(cDate, rString, nPos)    // 10., 10-, 10/
        || ((cTime != '.') && SkipChar('.',   rString, nPos))   // TRICKY:
        || ((cTime != '/') && SkipChar('/',   rString, nPos))   // short boolean
        || ((cTime != '-') && SkipChar('-',   rString, nPos)) ) // evaluation!
    {
        if (eScannedType != NUMBERFORMAT_UNDEFINED &&
            eScannedType != NUMBERFORMAT_DATE)          // schon anderes Format
            return FALSE;
        else
        {
            SkipBlanks(rString, nPos);
            eScannedType = NUMBERFORMAT_DATE;
        }
        short nTmpMonth = GetMonth(rString, nPos);  // 10. Jan
        if (nMonth && nTmpMonth)                    // Monat doppelt
            return FALSE;
        if (nTmpMonth)
        {
            nMonth = nTmpMonth;
            nMonthPos = 3;                          // Monat hinten
            if ( nMonth < 0 )
                SkipChar( '.', rString, nPos );     // abgekuerzt
            SkipBlanks(rString, nPos);
        }
    }

    short nTempMonth = GetMonth(rString, nPos);     // 10 Jan
    if (nTempMonth)
    {
        if (nMonth)                                 // Monat doppelt
            return FALSE;
        if (eScannedType != NUMBERFORMAT_UNDEFINED &&
            eScannedType != NUMBERFORMAT_DATE)          // schon anderes Format
            return FALSE;
        eScannedType = NUMBERFORMAT_DATE;
        nMonth = nTempMonth;
        nMonthPos = 3;                              // Monat hinten
        if ( nMonth < 0 )
            SkipChar( '.', rString, nPos );         // abgekuerzt
        SkipBlanks(rString, nPos);
    }

    if (GetTimeAmPm(rString, nPos))
    {
        if (eScannedType != NUMBERFORMAT_UNDEFINED &&
            eScannedType != NUMBERFORMAT_TIME &&
            eScannedType != NUMBERFORMAT_DATETIME)  // schon anderes Format
            return FALSE;
        else
        {
            SkipBlanks(rString, nPos);
            if ( eScannedType != NUMBERFORMAT_DATETIME )
                eScannedType = NUMBERFORMAT_TIME;
        }
    }

    if ( nNegCheck && SkipChar(')', rString, nPos) )
    {
        if (eScannedType == NUMBERFORMAT_CURRENCY)  // nur bei Waehrung
        {
            nNegCheck = 0;                          // ggf. ')' ueberlesen
            SkipBlanks(rString, nPos);
        }
        else
            return FALSE;
    }

    if ( nPos < rString.Len() &&
            (eScannedType == NUMBERFORMAT_DATE
            || eScannedType == NUMBERFORMAT_DATETIME) )
    {   // Wochentag wird nur weggeparst
        xub_StrLen nOldPos = nPos;
        const XubString& rSep = pIntl->GetLongDateDayOfWeekSep();
        if ( StringContains( rSep, rString, nPos ) )
        {
            nPos += rSep.Len();
            SkipBlanks(rString, nPos);
        }
        short nDayOfWeek;
        if ( nDayOfWeek = GetDayOfWeek( rString, nPos ) )
        {
            if ( nPos < rString.Len() )
            {
                if ( nDayOfWeek < 0 )
                {   // kurz
                    if ( rString.GetChar( nPos ) == '.' )
                        ++nPos;
                }
                SkipBlanks(rString, nPos);
            }
        }
        else
            nPos = nOldPos;
    }

    if (nPos < rString.Len())                       // alles weg?
    {
        // eingegebener EndString gleich EndString im Format?
        if ( !ScanStringNumFor( rString, nPos, pFormat, 0xFFFF ) )
            return FALSE;
    }

    return TRUE;
}


BOOL ImpSvNumberInputScan::ScanStringNumFor(
        const XubString& rString,           // zu scannender String
        xub_StrLen nPos,                    // Position bis zu der abgearbeitet war
        const SvNumberformat* pFormat,      // das zu matchende Format
        USHORT nString )                    // TeilString des TeilFormats
                                            // normalerweise 0 oder 0xFFFF
{
    const International* pIntl = pFormatter->GetInternational();
    if ( !pFormat )
        return FALSE;
    const XubString* pStr;
    XubString aString( rString );
    BOOL bFound = FALSE;
    BOOL bFirst = TRUE;
    BOOL bContinue = TRUE;
    USHORT nSub;
    do
    {
        // wenn am Anfang das zweite/dritte Teilformat gefunden wurde darunter
        // nicht mehr suchen
        nSub = nStringScanNumFor;
        do
        {   // TeilFormate durchprobieren, erst positiv, dann negativ, dann anderes,
            // letztes (Text) nicht
            pStr = pFormat->GetNumForString( nSub, nString, TRUE );
            if ( pStr && pIntl->CompareEqual( aString, *pStr, INTN_COMPARE_IGNORECASE ) )
            {
                bFound = TRUE;
                bContinue = FALSE;
            }
            else if ( nSub < 2 )
                ++nSub;
            else
                bContinue = FALSE;
        } while ( bContinue );
        if ( !bFound && bFirst && nPos )
        {   // uebriggelassenen SubString probieren
            bFirst = FALSE;
            aString.Erase( 0, nPos );
            bContinue = TRUE;
        }
    } while ( bContinue );

    if ( !bFound )
    {
        if ( (nString == 0) && !bFirst && (nSign < 0)
                && pFormat->IsNegativeRealNegative() )
        {   // simpel doppelt negiert? --1
            aString.EraseAllChars( ' ' );
            if ( (aString.Len() == 1) && (aString.GetChar(0) == '-') )
            {
                bFound = TRUE;
                nStringScanSign = -1;
                nSub = 0;       //! nicht 1
            }
        }
        if ( !bFound )
            return FALSE;
    }
    else if ( (nSub == 1) && pFormat->IsNegativeRealNegative() )
    {   // negativ
        if ( nStringScanSign < 0 )
        {
            if ( (nSign < 0) && (nStringScanNumFor != 1) )
                nStringScanSign = 1;        // dreifach negiert --1 yyy
        }
        else if ( nStringScanSign == 0 )
        {
            if ( nSign < 0 )
            {   // nSign und nStringScanSign werden spaeter verknuepft,
                // Vorzeichen umkehren wenn doppelt negiert
                if ( (nString == 0) && !bFirst
                        && SvNumberformat::HasStringNegativeSign( aString ) )
                    nStringScanSign = -1;   // direkte doppelte Negierung
                else if ( pFormat->IsNegativeWithoutSign() )
                    nStringScanSign = -1;   // indirekte doppelte Negierung
            }
            else
                nStringScanSign = -1;
        }
        else    // > 0
            nStringScanSign = -1;
    }
    nStringScanNumFor = nSub;
    return TRUE;
}


//---------------------------------------------------------------------------
//      IsNumberFormatMain
//
// erkennt folgende Typen: Zahl Z, Exp.darst. E, Bruch B, Prozent P
//                         Waehrung W, Datum D, Uhrzeit U
//                         sonst Text T <=> return FALSE; )

BOOL ImpSvNumberInputScan::IsNumberFormatMain(
        const XubString& rString,               // zu analysierender String
        double& fOutNumber,                     // OUT: Ergebnis als Zahl, wenn moeglich
        const SvNumberformat* pFormat )         // evtl. gesetztes Zahlenformat
{
    Reset();                                    // Anfangszustand
    // NoMoreUpperNeeded, alle Vergleiche auf UpperCase
    XubString aString( pFormatter->GetCharClass()->upper( rString ) );
    NumberStringDivision(aString);              // Zerlegung in Zahlen/Strings
    if (nAnzStrings >= SV_MAX_ANZ_INPUT_STRINGS)    // zuviele Einzelteile
        return FALSE;                           // Njet, Nope, ...

    if (nAnzNums == 0)                          // keine Zahl in der Eingabe
    {
        if ( nAnzStrings > 0 )
        {
            // hier kann das Original geaendert werden, wird nicht mehr
            // gebraucht, das erspart Kopiererei und ToUpper in GetLogical
            // und ist im Zusammenspiel schneller
            XubString& rStrArray = sStrArray[0];
            rStrArray.EraseTrailingChars( ' ' );
            rStrArray.EraseLeadingChars( ' ' );
            if ( nLogical = GetLogical( rStrArray ) )
            {
                eScannedType = NUMBERFORMAT_LOGICAL;// !!! es ist ein BOOL
                return TRUE;
            }
            else
                return FALSE;                   // simple Text
        }
        else
            return FALSE;                       // simple Text
    }

    USHORT i = 0;                               // markiert Symbole
    USHORT j = 0;                               // markiert nur Zahlen

    switch ( nAnzNums )
    {
        case 1 :                                // Genau 1 Zahl in der Eingabe
        {                                           // nAnzStrings >= 1
            if (GetNextNumber(i,j))                 // i=1,0
            {                                       // Zahl am Anfang
                if (eSetType == NUMBERFORMAT_FRACTION)  // Sonderfall Bruch 1 = 1/1
                {
                    if (i >= nAnzStrings ||         // kein Endstring oder ,
                        sStrArray[i].GetChar(0) == pFormatter->GetInternational()->GetNumDecimalSep())
                    {
                        eScannedType = NUMBERFORMAT_FRACTION;
                        return TRUE;
                    }
                }
            }
            else
            {                                       // Analyse des Anfangsstrings
                if (!ScanStartString( sStrArray[i], pFormat ))  // i=0
                    return FALSE;                   // schon fehlerhaft
                i++;                                // naechstes Symbol, i=1
            }
            GetNextNumber(i,j);                     // i=1,2
            if (eSetType == NUMBERFORMAT_FRACTION)  // Sonderfall Bruch -1 = -1/1
            {
                if (nSign && !nNegCheck &&          // Vorzeichen +, -
                    eScannedType == NUMBERFORMAT_UNDEFINED && // nicht D oder C
                    nDecPos == 0 &&                 // kein Dezimalkomma vorher
                    (i >= nAnzStrings ||            // kein Endstring oder ,
                        sStrArray[i].GetChar(0) == pFormatter->GetInternational()->GetNumDecimalSep())
                )
                {
                    eScannedType = NUMBERFORMAT_FRACTION;
                    return TRUE;
                }
            }
            if (i < nAnzStrings && !ScanEndString( sStrArray[i], pFormat ))
                    return FALSE;
        }
        break;
        case 2 :                                // Genau 2 Zahlen in Eingabe
        {                                           // nAnzStrings >= 3
            if (!GetNextNumber(i,j))                // i=1,0
            {                                       // Analyse des Anfangsstrings
                if (!ScanStartString( sStrArray[i], pFormat ))
                    return FALSE;                   // schon fehlerhaft
                i++;                                // i=1
            }
            GetNextNumber(i,j);                     // i=1,2
            if (!ScanMidString(sStrArray[i], i))
                return FALSE;
            i++;                                    // naechstes Symbol, i=2,3
            GetNextNumber(i,j);                     // i=3,4
            if (i < nAnzStrings && !ScanEndString( sStrArray[i], pFormat ))
                return FALSE;
            if (eSetType == NUMBERFORMAT_FRACTION)  // Sonderfall  -1.200, als Bruch
            {
                if (!nNegCheck  &&                  // kein Vorzeichen '('
                    eScannedType == NUMBERFORMAT_UNDEFINED &&
                    (nDecPos == 0 || nDecPos == 3)  // kein Dezimalz. oder hinten
                )
                {
                    eScannedType = NUMBERFORMAT_FRACTION;
                    return TRUE;
                }
            }
        }
        break;
        case 3 :                                // Genau 3 Zahlen in Eingabe
        {                                           // nAnzStrings >= 5
            if (!GetNextNumber(i,j))                // i=1,0
            {                                       // Analyse des Anfangsstrings
                if (!ScanStartString( sStrArray[i], pFormat ))
                    return FALSE;                   // schon fehlerhaft
                i++;                                // i=1
                if (nDecPos == 1)                   // , am Anfang => Fehler
                    return FALSE;
            }
            GetNextNumber(i,j);                     // i=1,2
            if (!ScanMidString(sStrArray[i], i))
                return FALSE;
            i++;                                    // i=2,3
            if (eScannedType == NUMBERFORMAT_SCIENTIFIC)    // E nur am Ende
                return FALSE;
            GetNextNumber(i,j);                     // i=3,4
            if (!ScanMidString(sStrArray[i], i))
                return FALSE;
            i++;                                    // i=4,5
            GetNextNumber(i,j);                     // i=5,6
            if (i < nAnzStrings && !ScanEndString( sStrArray[i], pFormat ))
                return FALSE;
            if (eSetType == NUMBERFORMAT_FRACTION)// Sonderfall  -1.200.100, als Bruch
            {
                if (!nNegCheck  &&                  // kein Vorzeichen '('
                    eScannedType == NUMBERFORMAT_UNDEFINED &&
                    (nDecPos == 0 || nDecPos == 3)  // kein Dezimalz. oder hinten
                )
                {
                    eScannedType = NUMBERFORMAT_FRACTION;
                    return TRUE;
                }
            }
            if ( eScannedType == NUMBERFORMAT_FRACTION && nDecPos )
                return FALSE;                       // #36857# kein echter Bruch
        }
        break;
        default:                                // Mehr als 3 Zahlen in Eingabe
        {                                           // nAnzStrings >= 7
            if (!GetNextNumber(i,j))                // i=1,0
            {                                       // Analyse des Anfangsstrings
                if (!ScanStartString( sStrArray[i], pFormat ))
                    return FALSE;                   // schon fehlerhaft
                i++;                                // i=1
                if (nDecPos == 1)                   // , am Anfang => Fehler
                    return FALSE;
            }
            GetNextNumber(i,j);                     // i=1,2
            if (!ScanMidString(sStrArray[i], i))
                return FALSE;
            i++;                                    // i=2,3
            USHORT nThOld = 10;                     // != 0 oder 1
            while (nThOld != nThousand && j < nAnzNums-1)
                                                    // mindestens ein Mal
                                                    // aber eine Zahl noch lassen
            {                                       // Abarbeitung Tausenderpkte.
                nThOld = nThousand;
                if (eScannedType == NUMBERFORMAT_SCIENTIFIC)    // E nur am Ende
                    return FALSE;
                GetNextNumber(i,j);
                if (i < nAnzStrings && !ScanMidString(sStrArray[i], i))
                    return FALSE;
                i++;
            }
            if (eScannedType == NUMBERFORMAT_DATE ||    // Abarbeitung langes Datum
                eScannedType == NUMBERFORMAT_TIME ||    // lange Uhrzeit
                eScannedType == NUMBERFORMAT_UNDEFINED) // oder lange Zahl
            {
                for (USHORT k = j; k < nAnzNums-1; k++)
                {
                    if (eScannedType == NUMBERFORMAT_SCIENTIFIC)    // E nur am Ende
                        return FALSE;
                    GetNextNumber(i,j);
                    if (i < nAnzStrings && !ScanMidString(sStrArray[i], i))
                        return FALSE;
                    i++;
                }
            }
            GetNextNumber(i,j);
            if (i < nAnzStrings && !ScanEndString( sStrArray[i], pFormat ))
                return FALSE;
            if (eSetType == NUMBERFORMAT_FRACTION)// Sonderfall  -1.200.100, als Bruch
            {
                if (!nNegCheck  &&                  // kein Vorzeichen '('
                    eScannedType == NUMBERFORMAT_UNDEFINED &&
                    (nDecPos == 0 || nDecPos == 3)  // kein Dezimalz. oder hinten
                )
                {
                    eScannedType = NUMBERFORMAT_FRACTION;
                    return TRUE;
                }
            }
            if ( eScannedType == NUMBERFORMAT_FRACTION && nDecPos )
                return FALSE;                       // #36857# kein echter Bruch
        }
    }

    if (eScannedType == NUMBERFORMAT_UNDEFINED)     // alles andere sollte bereits
        eScannedType = NUMBERFORMAT_NUMBER;           // erkannt sein

    return TRUE;
}


//---------------------------------------------------------------------------
// Die 12 Monate und 7 Wochentage initialisieren

void ImpSvNumberInputScan::InitText()
{
    int j;
    const International* pIntl = pFormatter->GetInternational();
    const CharClass* pChrCls = pFormatter->GetCharClass();
    for ( j=0; j<12; j++ )
    {
        aUpperMonthText[j] = pChrCls->upper( pIntl->GetMonthText(j+1) );
        aUpperAbbrevMonthText[j] = pChrCls->upper( pIntl->GetAbbrevMonthText(j+1) );
    }
    for ( j=0; j<7; j++ )
    {
        aUpperDayText[j] = pChrCls->upper( pIntl->GetDayText( (DayOfWeek) j ) );
        aUpperAbbrevDayText[j] = pChrCls->upper( pIntl->GetAbbrevDayText( (DayOfWeek) j ) );
    }
    aUpperCurrSymbol = pChrCls->upper( pIntl->GetCurrSymbol() );
    bTextInitialized = TRUE;
}


//===========================================================================
//          P U B L I C

//---------------------------------------------------------------------------
//      ChangeIntl
//
// MUST be called if International/Locale is changed

void ImpSvNumberInputScan::ChangeIntl()
{
    xub_Unicode cDecSep = pFormatter->GetInternational()->GetNumDecimalSep();
    bDecSepInDateSeps = ( cDecSep == '-' ||
                          cDecSep == '/' ||
                          cDecSep == '.' ||
                          cDecSep == pFormatter->GetInternational()->GetDateSep());
    bTextInitialized = FALSE;
}


//---------------------------------------------------------------------------
//      ChangeNullDate

void ImpSvNumberInputScan::ChangeNullDate(
        const USHORT nDay,
        const USHORT nMonth,
        const USHORT nYear )
{
    delete pNullDate;
    pNullDate = new Date(nDay, nMonth, nYear);
}


//---------------------------------------------------------------------------
//      IsNumberFormat
//
// => String als Zahl darstellbar

BOOL ImpSvNumberInputScan::IsNumberFormat(
        const XubString& rString,               // zu analysierender String
        short& F_Type,                          // IN: alter Typ, OUT: neuer Typ
        double& fOutNumber,                     // OUT: Zahl, wenn Umwandlung moeglich
        const SvNumberformat* pFormat )         // evtl. gesetztes Zahlenformat
{
    // in den zerlegten Strings gibt es keine Null-Laengen Strings mehr!

    XubString sResString;                       // die Eingabe fuer atof
    BOOL res;                                   // Rueckgabewert
    eSetType = F_Type;                          // Typ der Zelle

    if ( !rString.Len() )
        res = FALSE;
    else if (rString.Len() > 308)                   // frueher 100
        res = FALSE;
    else
        res = IsNumberFormatMain(rString, fOutNumber, pFormat);

    if (res)
    {
        if (   nNegCheck                            // ')' nicht gefunden
            || (eScannedType == NUMBERFORMAT_TIME   // Zeit mit Vorzeichen
                && nSign) )
            res = FALSE;
        else                                        // Check der Zahlanzahlen
        {
            switch (eScannedType)                   // Analyseergebnis pruefen
            {
                case NUMBERFORMAT_PERCENT:          // alle Zahlen
                case NUMBERFORMAT_CURRENCY:
                case NUMBERFORMAT_NUMBER:
                    if (nDecPos == 1)               // ,05
                    {
                        if (nAnzNums != 1)
                            res = FALSE;
                    }
                    else if (nDecPos == 2)          // 1,05
                    {
                        if (nAnzNums != nThousand+2)
                            res = FALSE;
                    }
                    else                            // 1.100 oder 1.100,
                    {
                        if (nAnzNums != nThousand+1)
                            res = FALSE;
                    }
                    break;

                case NUMBERFORMAT_SCIENTIFIC:       // wissenschaftl. Format 1,0e-2
                    if (nDecPos == 1)               // ,05
                    {
                        if (nAnzNums != 2)
                            res = FALSE;
                    }
                    else if (nDecPos == 2)          // 1,05
                    {
                        if (nAnzNums != nThousand+3)
                            res = FALSE;
                    }
                    else                            // 1.100 oder 1.100,
                    {
                        if (nAnzNums != nThousand+2)
                            res = FALSE;
                    }
                    break;

                case NUMBERFORMAT_DATE:             // Datum
                    if (nMonth)
                    {
                        if (nAnzNums > 2)
                            res = FALSE;
                    }
                    else
                    {
                        if (nAnzNums > 3)
                            res = FALSE;
                    }
                    break;

                case NUMBERFORMAT_TIME:             // Uhrzeit
                    if (nDecPos)
                    {
                        if (nAnzNums > 4)
                            res = FALSE;
                    }
                    else
                    {
                        if (nAnzNums > 3)
                            res = FALSE;
                    }
                    break;

                case NUMBERFORMAT_DATETIME:         // Datum + Uhrzeit
                    if (nMonth)
                    {
                        if (nAnzNums > 5)
                            res = FALSE;
                    }
                    else
                    {
                        if (nAnzNums > 6)
                            res = FALSE;
                    }
                    break;

                default:
                    break;
            }   // switch
        }   // else
    }   // if (res)

    if (res)                                    // Bestimmung der Zahl:
    {
        switch (eScannedType)
        {
            case NUMBERFORMAT_LOGICAL:          // Logisch
                if      (nLogical ==  1)
                    fOutNumber = 1.0;           // True
                else if (nLogical == -1)
                    fOutNumber = 0.0;           // False
                else
                    res = FALSE;                // Oops
                break;

            case NUMBERFORMAT_PERCENT:          // Zahlen
            case NUMBERFORMAT_CURRENCY:
            case NUMBERFORMAT_NUMBER:
            case NUMBERFORMAT_SCIENTIFIC:       // erstmal Zahlanteil
                if (nDecPos == 1)                           // , am Anfang
                {
                    sResString.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "0." ) );
                    sResString += sStrArray[nNums[0]];
                }
                else
                {   USHORT k;
                    sResString = sStrArray[nNums[0]];
                    for ( k = 1; k <= nThousand; k++)
                        sResString += sStrArray[nNums[k]];  // Vorkommateil
                    if (nDecPos == 2)                       // in der Mitte
                    {
                        sResString += '.';
                        sResString += sStrArray[nNums[k]];
                    }
                }

                if (eScannedType != NUMBERFORMAT_SCIENTIFIC)
                    fOutNumber = StringToDouble(sResString);
                else                                        // Nachbehandlung Exponent
                {
                    USHORT i;
                    double fExp;

                    if (nDecPos == 2)
                        fExp = StringToDouble(sStrArray[nNums[nThousand+2]]);
                    else
                        fExp = StringToDouble(sStrArray[nNums[nThousand+1]]);
                    if (fExp > 308.0)
                    {
                        F_Type = NUMBERFORMAT_TEXT;         // Ueberlauf -> Text
                        if (nESign == -1)
                            fOutNumber = 0.0;
                        else
                            fOutNumber = DBL_MAX;
/*!*/                   return TRUE;
                    }
                    else
                    {
                        fOutNumber = StringToDouble(sResString);
                        if (nESign == -1)
                        {
#ifdef S390
                            // S390 DBL_MIN = 5.397605347e-79
                            if (fExp == 79.0  && fOutNumber < 5.397605348)
#else
                            if (fExp == 308.0 && fOutNumber < DBL_MIN*1.0E308)
#endif
                            {
                                eScannedType = NUMBERFORMAT_TEXT;   // Ueberlauf -> Text
                                fOutNumber = 0.0;
                            }
                            else
                                for (i=0; i < (USHORT) fExp; i++)
                                    fOutNumber /= 10.0;
                        }
                        else
                        {
#ifdef S390
                            if (fExp == 75.0 && fOutNumber > DBL_MAX/1.0E75)
#else
                            if (fExp == 308.0 && fOutNumber > DBL_MAX/1.0E308)
#endif
                            {
                                fOutNumber = DBL_MAX;
                                eScannedType = NUMBERFORMAT_TEXT;   // Ueberlauf -> Text
                            }
                            else
                                for (i=0; i < (USHORT) fExp; i++)
                                    fOutNumber *= 10.0;
                        }
                    }
                }

                if ( nStringScanSign )
                {
                    if ( nSign )
                        nSign *= nStringScanSign;
                    else
                        nSign = nStringScanSign;
                }
                if ( nSign < 0 )                            // Vorzeichen dazu
                    fOutNumber = -fOutNumber;

                if (eScannedType == NUMBERFORMAT_PERCENT)   // durch 100 dividieren
                    fOutNumber/= 100.0;
                break;

            case NUMBERFORMAT_FRACTION:         // Bruch
                if (nAnzNums == 1)
                    fOutNumber = StringToDouble(sStrArray[nNums[0]]);
                else if (nAnzNums == 2)
                {
                    if (nThousand == 1)
                    {
                        sResString = sStrArray[nNums[0]];
                        sResString += sStrArray[nNums[1]];  // Vorkommateil
                        fOutNumber = StringToDouble(sResString);
                    }
                    else
                    {
                        double fZaehler = StringToDouble(sStrArray[nNums[0]]);
                        double fNenner = StringToDouble(sStrArray[nNums[1]]);
                        if (fNenner != 0.0)
                            fOutNumber = fZaehler/fNenner;
                        else
                            res = FALSE;
                    }
                }
                else                                        // nAnzNums > 2
                {
                    USHORT k = 1;
                    sResString = sStrArray[nNums[0]];
                    if (nThousand > 0)
                        for (k = 1; k <= nThousand; k++)
                            sResString += sStrArray[nNums[k]];
                    fOutNumber = StringToDouble(sResString);

                    if (k == nAnzNums-2)
                    {
                        double fZaehler = StringToDouble(sStrArray[nNums[k]]);
                        double fNenner = StringToDouble(sStrArray[nNums[k+1]]);
                        if (fNenner != 0.0)
                            fOutNumber += fZaehler/fNenner;
                        else
                            res = FALSE;
                    }
                }

                if ( nStringScanSign )
                {
                    if ( nSign )
                        nSign *= nStringScanSign;
                    else
                        nSign = nStringScanSign;
                }
                if ( nSign < 0 )                            // Vorzeichen dazu
                    fOutNumber = -fOutNumber;
                break;

            case NUMBERFORMAT_TIME:             // Uhrzeit
                GetTimeRef(fOutNumber, 0, nAnzNums);
                break;

            case NUMBERFORMAT_DATE:             // Datum
            {
                Date aDt;                                   // heute
                USHORT nCounter = 0;                        // hier dummy
                res = GetDateRef(aDt, nCounter, pFormat);   // Datum->aDt
                if (aDt.IsValid())
                {
                    long nDate = (long) (aDt - *pNullDate); // erst nach long!!
                    fOutNumber = (double) nDate;            // vorsichtshalber
                }
                else
                    res = FALSE;
            }
            break;

            case NUMBERFORMAT_DATETIME:         // Datum mit Uhrzeit
            {
                Date aDt;                                   // heute
                USHORT nCounter;                            // hier wichtig
                res = GetDateRef(aDt, nCounter, pFormat);   // Datum->aDt
                double fTime;
                GetTimeRef(fTime, nCounter, nAnzNums-nCounter);
                if (aDt.IsValid())
                {
                    long nDate = (long) (aDt - *pNullDate);
                    fOutNumber = (double) nDate + fTime;
                }
                else
                    res = FALSE;
            }
            break;

            default:
                break;
        }
    }

    if (res)        // Ueberlauf -> Text
    {
        if      (fOutNumber < -DBL_MAX) // -1.7E308
        {
            F_Type = NUMBERFORMAT_TEXT;
            fOutNumber = -DBL_MAX;
            return TRUE;
        }
        else if (fOutNumber >  DBL_MAX) // 1.7E308
        {
            F_Type = NUMBERFORMAT_TEXT;
            fOutNumber = DBL_MAX;
            return TRUE;
        }
    }

    if (res == FALSE)
    {
        eScannedType = NUMBERFORMAT_TEXT;
        fOutNumber = 0.0;
    }

    F_Type = eScannedType;
    return res;
}



