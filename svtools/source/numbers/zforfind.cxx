/*************************************************************************
 *
 *  $RCSfile: zforfind.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: er $ $Date: 2000-11-18 21:46:43 $
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
#ifndef _UNOTOOLS_CALENDARWRAPPER_HXX
#include <unotools/calendarwrapper.hxx>
#endif
#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif
#ifndef _COM_SUN_STAR_I18N_CALENDARFIELDINDEX_HPP_
#include <com/sun/star/i18n/CalendarFieldIndex.hpp>
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
#define MyIsdigit(c)    (isdigit((unsigned sal_Unicode)(c)))
#endif

//---------------------------------------------------------------------------
//      Konstruktor

ImpSvNumberInputScan::ImpSvNumberInputScan( SvNumberFormatter* pFormatterP )
        :
        pUpperMonthText( NULL ),
        pUpperAbbrevMonthText( NULL ),
        pUpperDayText( NULL ),
        pUpperAbbrevDayText( NULL )
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
    delete [] pUpperMonthText;
    delete [] pUpperAbbrevMonthText;
    delete [] pUpperDayText;
    delete [] pUpperAbbrevDayText;
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

double ImpSvNumberInputScan::StringToDouble( const String& rStr )
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
        const sal_Unicode*& pStr,
        String& rSymbol )
{
    BOOL isNumber = FALSE;
    sal_Unicode cToken;
    ScanState eState = SsStart;
    register const sal_Unicode* pHere = pStr;
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
        const sal_Unicode*& pStr,
        String& rSymbol )
{
    BOOL res = FALSE;
    sal_Unicode cToken;
    const String& rThSep = pFormatter->GetLocaleData()->getNumThousandSep();
    register const sal_Unicode* pHere = pStr;
    ScanState eState = SsStart;
    xub_StrLen nCounter;                                // counts 3 digits

    while ( ((cToken = *pHere) != 0) && eState != SsStop)
    {
        pHere++;
        switch (eState)
        {
            case SsStart:
                if ( StringPtrContains( rThSep, pHere-1, 0 ) )
                {
                    nCounter = 0;
                    eState = SsGetValue;
                    pHere += rThSep.Len()-1;
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
                        res = TRUE;                 // .000 combination found
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

    if (eState == SsGetValue)               // break witth less than 3 digits
    {
        if ( nCounter )
            rSymbol.Erase( rSymbol.Len() - nCounter, nCounter );
        pHere -= nCounter + rThSep.Len();       // put back ThSep also
    }
    pStr = pHere;

    return res;
}


//---------------------------------------------------------------------------
//      NumberStringDivision

void ImpSvNumberInputScan::NumberStringDivision( const String& rString )
{
    register const sal_Unicode* pStr = rString.GetBuffer();
    register const sal_Unicode* const pEnd = pStr + rString.Len();
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
// Whether rString contains rWhat at nPos

BOOL ImpSvNumberInputScan::StringContainsImpl( const String& rWhat,
            const String& rString, xub_StrLen nPos )
{
    if ( nPos + rWhat.Len() <= rString.Len() )
        return StringPtrContainsImpl( rWhat, rString.GetBuffer(), nPos );
    return FALSE;
}


//---------------------------------------------------------------------------
// Whether pString contains rWhat at nPos

BOOL ImpSvNumberInputScan::StringPtrContainsImpl( const String& rWhat,
            const sal_Unicode* pString, xub_StrLen nPos )
{
    if ( rWhat.Len() == 0 )
        return FALSE;
    register const sal_Unicode* pWhat = rWhat.GetBuffer();
    register const sal_Unicode* const pEnd = pWhat + rWhat.Len();
    register const sal_Unicode* pStr = pString + nPos;
    while ( pWhat < pEnd )
    {
        if ( *pWhat != *pStr )
            return FALSE;
        pWhat++;
        pStr++;
    }
    return TRUE;
}


//---------------------------------------------------------------------------
//      SkipChar
//
// ueberspringt genau das angegebene Zeichen

inline BOOL ImpSvNumberInputScan::SkipChar( sal_Unicode c, const String& rString,
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

inline void ImpSvNumberInputScan::SkipBlanks( const String& rString,
        xub_StrLen& nPos )
{
    if ( nPos < rString.Len() )
    {
        register const sal_Unicode* p = rString.GetBuffer() + nPos;
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

inline BOOL ImpSvNumberInputScan::SkipString( const String& rWhat,
        const String& rString, xub_StrLen& nPos )
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
        const String& rString,
        xub_StrLen& nPos,
        USHORT nStringPos )
{
    const String& rSep = pFormatter->GetLocaleData()->getNumThousandSep();
    if (   rString == rSep                                  // nothing else
        && nStringPos < nAnzStrings - 1                     // safety first!
        && IsNum[nStringPos+1]                              // number follows
        && (   sStrArray[nStringPos+1].Len() == 3           // with 3 digits
            || nPosThousandString == nStringPos+1 ) )       // or concatenated
    {
        nPos += rSep.Len();
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

short ImpSvNumberInputScan::GetLogical( const String& rString )
{
    short res;

    if (rString.Len() < 4)              // kein Platz fuer mind 4 Buch.
        res = 0;
    else
    {
        const ImpSvNumberformatScan* pFS = pFormatter->GetFormatScanner();
        if ( rString == pFS->GetTrueString() )
            res = 1;
        else if ( rString == pFS->GetFalseString() )
            res = -1;
        else
            res = 0;
    }

    return res;
}


//---------------------------------------------------------------------------
//      GetMonth
//
// Converts a string containing a month name (JAN, January) at nPos into the
// month number (negative if abbreviated), returns 0 if nothing found

short ImpSvNumberInputScan::GetMonth( const String& rString, xub_StrLen& nPos )
{
    short res = 0;      // no month found

    if (rString.Len() > nPos)                           // only if needed
    {
        if ( !bTextInitialized )
            InitText();
        sal_Int16 nMonths = pFormatter->GetCalendar()->getNumberOfMonthsInYear();
        for ( sal_Int16 i = 0; i < nMonths; i++ )
        {
            if ( StringContains( pUpperMonthText[i], rString, nPos ) )
            {                                           // full names first
                nPos += pUpperMonthText[i].Len();
                res = i+1;
                break;  // for
            }
            else if ( StringContains( pUpperAbbrevMonthText[i], rString, nPos ) )
            {                                           // abbreviated
                nPos += pUpperAbbrevMonthText[i].Len();
                res = -(i+1);                           // negative
                break;  // for
            }
        }
    }

    return res;
}


//---------------------------------------------------------------------------
//      GetDayOfWeek
//
// Converts a string containing a DayOfWeek name (Mon, Monday) at nPos into the
// DayOfWeek number + 1 (negative if abbreviated), returns 0 if nothing found

short ImpSvNumberInputScan::GetDayOfWeek( const String& rString, xub_StrLen& nPos )
{
    short res = 0;      // no day found

    if (rString.Len() > nPos)                           // only if needed
    {
        if ( !bTextInitialized )
            InitText();
        sal_Int16 nDays = pFormatter->GetCalendar()->getNumberOfDaysInWeek();
        for ( sal_Int16 i = 0; i < nDays; i++ )
        {
            if ( StringContains( pUpperDayText[i], rString, nPos ) )
            {                                           // full names first
                nPos += pUpperDayText[i].Len();
                res = i + 1;
                break;  // for
            }
            if ( StringContains( pUpperAbbrevDayText[i], rString, nPos ) )
            {                                           // abbreviated
                nPos += pUpperAbbrevDayText[i].Len();
                res = -(i + 1);                         // negative
                break;  // for
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

BOOL ImpSvNumberInputScan::GetCurrency( const String& rString, xub_StrLen& nPos,
            const SvNumberformat* pFormat )
{
    if ( rString.Len() > nPos )
    {
        if ( !bTextInitialized )
            InitText();
        if ( StringContains( aUpperCurrSymbol, rString, nPos ) )
        {
            nPos += aUpperCurrSymbol.Len();
            return TRUE;
        }
        if ( pFormat )
        {
            String aSymbol, aExtension;
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

BOOL ImpSvNumberInputScan::GetTimeAmPm( const String& rString, xub_StrLen& nPos )
{

    if ( rString.Len() > nPos )
    {
        const CharClass* pChr = pFormatter->GetCharClass();
        const LocaleDataWrapper* pLoc = pFormatter->GetLocaleData();
        if ( StringContains( pChr->upper( pLoc->getTimeAM() ), rString, nPos ) )
        {
            nAmPm = 1;
            nPos += pLoc->getTimeAM().Len();
            return TRUE;
        }
        else if ( StringContains( pChr->upper( pLoc->getTimePM() ), rString, nPos ) )
        {
            nAmPm = -1;
            nPos += pLoc->getTimePM().Len();
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

inline BOOL ImpSvNumberInputScan::GetDecSep( const String& rString, xub_StrLen& nPos )
{
    if ( rString.Len() > nPos )
    {
        const String& rSep = pFormatter->GetLocaleData()->getNumDecimalSep();
        if ( rString.Equals( rSep, nPos, rSep.Len() ) )
        {
            nPos += rSep.Len();
            return TRUE;
        }
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

short ImpSvNumberInputScan::GetSign( const String& rString, xub_StrLen& nPos )
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

short ImpSvNumberInputScan::GetESign( const String& rString, xub_StrLen& nPos )
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
    sal_Unicode* pChar = NULL;
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
        String s100Sec( '.' );
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
    // preset invalid month number
    USHORT nRes = pFormatter->GetCalendar()->getNumberOfMonthsInYear();

    if (sStrArray[nNums[nIndex]].Len() <= 2)
    {
        USHORT nNum = (USHORT) sStrArray[nNums[nIndex]].ToInt32();
        if ( 0 < nNum && nNum <= nRes )
            nRes = nNum - 1;        // zero based for CalendarFieldIndex::MONTH
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
    using namespace ::com::sun::star::i18n;
    NfEvalDateFormat eEDF;
    int nFormatOrder;
    if ( pFormat && ((pFormat->GetType() & NUMBERFORMAT_DATE) == NUMBERFORMAT_DATE) )
    {
        eEDF = pFormat->ImpGetScan().GetNumberformatter()->GetEvalDateFormat();
        switch ( eEDF )
        {
            case NF_EVALDATEFORMAT_INTL :
            case NF_EVALDATEFORMAT_FORMAT :
                nFormatOrder = 1;       // only one loop
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

    const LocaleDataWrapper* pLoc = pFormatter->GetLocaleData();
    CalendarWrapper* pCal = pFormatter->GetCalendar();
    for ( int nTryOrder = 1; nTryOrder <= nFormatOrder; nTryOrder++ )
    {
        pCal->setGregorianDateTime( aDt );
        DateFormat DateFmt;
        switch ( eEDF )
        {
            case NF_EVALDATEFORMAT_INTL :
                DateFmt = pLoc->getDateFormat();
            break;
            case NF_EVALDATEFORMAT_FORMAT :
                DateFmt = pFormat->GetDateOrder();
            break;
            case NF_EVALDATEFORMAT_INTL_FORMAT :
                if ( nTryOrder == 1 )
                    DateFmt = pLoc->getDateFormat();
                else
                    DateFmt = pFormat->GetDateOrder();
            break;
            case NF_EVALDATEFORMAT_FORMAT_INTL :
                if ( nTryOrder == 2 )
                    DateFmt = pLoc->getDateFormat();
                else
                    DateFmt = pFormat->GetDateOrder();
            break;
            default:
                DBG_ERROR( "ImpSvNumberInputScan::GetDateRef: unknown NfEvalDateFormat" );
        }

        res = TRUE;
        nCounter = 0;

        switch (nAnzNums)       // count of numbers in string
        {
            case 0:                 // none
                if (nMonthPos)          // only month (Jan)
                {
                    pCal->setValue( CalendarFieldIndex::DAY_OF_MONTH, 1 );
                    pCal->setValue( CalendarFieldIndex::MONTH, Abs(nMonth)-1 );
                }
                else
                    res = FALSE;
                break;

            case 1:                 // only one number
                nCounter = 1;
                switch (nMonthPos)  // where is the month
                {
                    case 0:             // not found => only day entered
                        pCal->setValue( CalendarFieldIndex::DAY_OF_MONTH, ImplGetDay(0) );
                        break;
                    case 1:             // month at the beginning (Jan 01)
                        pCal->setValue( CalendarFieldIndex::MONTH, Abs(nMonth)-1 );
                        switch (DateFmt)
                        {
                            case MDY:
                            case YMD:
                                pCal->setValue( CalendarFieldIndex::DAY_OF_MONTH, ImplGetDay(0) );
                                break;
                            case DMY:
                                pCal->setValue( CalendarFieldIndex::DAY_OF_MONTH, 1 );
                                pCal->setValue( CalendarFieldIndex::YEAR, ImplGetYear(0) );
                                break;
                            default:
                                res = FALSE;
                                break;
                        }
                        break;
                    case 3:             // month at the end (10 Jan)
                        pCal->setValue( CalendarFieldIndex::MONTH, Abs(nMonth)-1 );
                        switch (DateFmt)
                        {
                            case DMY:
                                pCal->setValue( CalendarFieldIndex::DAY_OF_MONTH, ImplGetDay(0) );
                                break;
                            case YMD:
                                pCal->setValue( CalendarFieldIndex::DAY_OF_MONTH, 1 );
                                pCal->setValue( CalendarFieldIndex::YEAR, ImplGetYear(0) );
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

            case 2:                 // 2 numbers
                nCounter = 2;
                switch (nMonthPos)  // where is the month
                {
                    case 0:             // not found
                        switch (DateFmt)
                        {
                            case MDY:
                                pCal->setValue( CalendarFieldIndex::DAY_OF_MONTH, ImplGetDay(1) );
                                pCal->setValue( CalendarFieldIndex::MONTH, ImplGetMonth(0) );
                                break;
                            case DMY:
                                pCal->setValue( CalendarFieldIndex::DAY_OF_MONTH, ImplGetDay(0) );
                                pCal->setValue( CalendarFieldIndex::MONTH, ImplGetMonth(1) );
#if 0
//! TODO: howto with an XCalendar?
                                if (!aDt.IsValid())             // 2nd try
                                {
                                    pCal->setValue( CalendarFieldIndex::DAY_OF_MONTH, 1 );
                                    pCal->setValue( CalendarFieldIndex::MONTH, ImplGetMonth(0) );
                                    pCal->setValue( CalendarFieldIndex::YEAR, ImplGetYear(1) );
                                }
#endif
                                break;
                            case YMD:
                                pCal->setValue( CalendarFieldIndex::DAY_OF_MONTH, ImplGetDay(1) );
                                pCal->setValue( CalendarFieldIndex::MONTH, ImplGetMonth(0) );
#if 0
//! TODO: howto with an XCalendar?
                                if (!aDt.IsValid())             // 2nd try
                                {
                                    pCal->setValue( CalendarFieldIndex::DAY_OF_MONTH, 1 );
                                    pCal->setValue( CalendarFieldIndex::MONTH, ImplGetMonth(1) );
                                    pCal->setValue( CalendarFieldIndex::YEAR, ImplGetYear(0) );
                                }
#endif
                                break;
                            default:
                                res = FALSE;
                                break;
                        }
                        break;
                    case 1:             // month at the beginning (Jan 01 01)
                        switch (DateFmt)
                        {
                            case MDY:
                                pCal->setValue( CalendarFieldIndex::DAY_OF_MONTH, ImplGetDay(0) );
                                pCal->setValue( CalendarFieldIndex::MONTH, Abs(nMonth)-1 );
                                pCal->setValue( CalendarFieldIndex::YEAR, ImplGetYear(1) );
                                break;
                            default:
                                res = FALSE;
                                break;
                        }
                        break;
                    case 2:             // month in the middle (10 Jan 94)
                        pCal->setValue( CalendarFieldIndex::MONTH, Abs(nMonth)-1 );
                        switch (DateFmt)
                        {
                            case DMY:
                                pCal->setValue( CalendarFieldIndex::DAY_OF_MONTH, ImplGetDay(0) );
                                pCal->setValue( CalendarFieldIndex::YEAR, ImplGetYear(1) );
                                break;
                            case YMD:
                                pCal->setValue( CalendarFieldIndex::DAY_OF_MONTH, ImplGetDay(1) );
                                pCal->setValue( CalendarFieldIndex::YEAR, ImplGetYear(0) );
                                break;
                            default:
                                res = FALSE;
                                break;
                        }
                        break;
                    default:            // else, e.g. month at the end (94 10 Jan)
                        res = FALSE;
                        break;
                }   // switch (nMonthPos)
                break;

            default:                // more than two numbers (31.12.94 8:23) (31.12. 8:23)
                switch (nMonthPos)  // where is the month
                {
                    case 0:             // not found
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
                                pCal->setValue( CalendarFieldIndex::DAY_OF_MONTH, ImplGetDay(1) );
                                pCal->setValue( CalendarFieldIndex::MONTH, ImplGetMonth(0) );
                                if ( nCounter > 2 )
                                    pCal->setValue( CalendarFieldIndex::YEAR, ImplGetYear(2) );
                                break;
                            case DMY:
                                pCal->setValue( CalendarFieldIndex::DAY_OF_MONTH, ImplGetDay(0) );
                                pCal->setValue( CalendarFieldIndex::MONTH, ImplGetMonth(1) );
                                if ( nCounter > 2 )
                                    pCal->setValue( CalendarFieldIndex::YEAR, ImplGetYear(2) );
                                break;
                            case YMD:
                                if ( nCounter > 2 )
                                    pCal->setValue( CalendarFieldIndex::DAY_OF_MONTH, ImplGetDay(2) );
                                pCal->setValue( CalendarFieldIndex::MONTH, ImplGetMonth(1) );
                                pCal->setValue( CalendarFieldIndex::YEAR, ImplGetYear(0) );
                                break;
                            default:
                                res = FALSE;
                                break;
                        }
                        break;
                    case 1:             // month at the beginning (Jan 01 01 8:23)
                        nCounter = 2;
                        switch (DateFmt)
                        {
                            case MDY:
                                pCal->setValue( CalendarFieldIndex::DAY_OF_MONTH, ImplGetDay(0) );
                                pCal->setValue( CalendarFieldIndex::MONTH, Abs(nMonth)-1 );
                                pCal->setValue( CalendarFieldIndex::YEAR, ImplGetYear(1) );
                                break;
                            default:
                                res = FALSE;
                                break;
                        }
                        break;
                    case 2:             // month in the middle (10 Jan 94 8:23)
                        nCounter = 2;
                        pCal->setValue( CalendarFieldIndex::MONTH, Abs(nMonth)-1 );
                        switch (DateFmt)
                        {
                            case DMY:
                                pCal->setValue( CalendarFieldIndex::DAY_OF_MONTH, ImplGetDay(0) );
                                pCal->setValue( CalendarFieldIndex::YEAR, ImplGetYear(1) );
                                break;
                            case YMD:
                                pCal->setValue( CalendarFieldIndex::DAY_OF_MONTH, ImplGetDay(1) );
                                pCal->setValue( CalendarFieldIndex::YEAR, ImplGetYear(2) );
                                break;
                            default:
                                res = FALSE;
                                break;
                        }
                        break;
                    default:            // else, e.g. month at the end (94 10 Jan 8:23)
                        nCounter = 2;
                        res = FALSE;
                        break;
                }   // switch (nMonthPos)
                break;
        }   // switch (nAnzNums)

        aDt = pCal->getGregorianDateTime();
        if ( nTryOrder < nFormatOrder )
        {
            if ( res && aDt.IsValid() )
                nTryOrder = nFormatOrder;   // break for
            else
                aDt = Date();               // next try
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

BOOL ImpSvNumberInputScan::ScanStartString( const String& rString,
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
    else if ( nMonth = GetMonth(rString, nPos) )    // month (Jan 1)?
    {
        eScannedType = NUMBERFORMAT_DATE;           // !!! it IS a date !!!
        nMonthPos = 1;                              // month at the beginning
        if ( nMonth < 0 )
            SkipChar( '.', rString, nPos );         // abbreviated
        SkipBlanks(rString, nPos);
    }
    else if ( nDayOfWeek = GetDayOfWeek( rString, nPos ) )
    {   // day of week is just parsed away
        eScannedType = NUMBERFORMAT_DATE;           // !!! it IS a date !!!
        if ( nPos < rString.Len() )
        {
            if ( nDayOfWeek < 0 )
            {   // abbreviated
                if ( rString.GetChar( nPos ) == '.' )
                    ++nPos;
            }
            else
            {   // full long name
                SkipBlanks(rString, nPos);
                SkipString( pFormatter->GetLocaleData()->getLongDateDayOfWeekSep(), rString, nPos );
            }
            SkipBlanks(rString, nPos);
            if ( nMonth = GetMonth(rString, nPos) ) // month (Jan 1)?
            {
                nMonthPos = 1;                      // month a the beginning
                if ( nMonth < 0 )
                    SkipChar( '.', rString, nPos ); // abbreviated
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
        const String& rString,
        USHORT nStringPos )
{
    xub_StrLen nPos = 0;

    SkipBlanks(rString, nPos);
    if (GetDecSep(rString, nPos))                   // decimal separator?
    {
        if (nDecPos == 1 || nDecPos == 3)           // ,12,4 or 1,E2,1
            return FALSE;
        else if (nDecPos == 2)                      // , dup: 12,4,
        {
            if (bDecSepInDateSeps)                  // , also date separator
            {
                if (eScannedType != NUMBERFORMAT_UNDEFINED &&
                    eScannedType != NUMBERFORMAT_DATE)  // already another type
                    return FALSE;
                eScannedType = NUMBERFORMAT_DATE;   // !!! it IS a date
                SkipBlanks(rString, nPos);
            }
            else
                return FALSE;
        }
        else
        {
            nDecPos = 2;                            // , in mid string
            SkipBlanks(rString, nPos);
        }
    }

    if (SkipChar('/', rString, nPos))               // fraction?
    {
        if (   eScannedType != NUMBERFORMAT_UNDEFINED   // already another type
            && eScannedType != NUMBERFORMAT_DATE)       // except date
            return FALSE;                               // => jan/31/1994
        else if (    eScannedType != NUMBERFORMAT_DATE      // analyzed date until now
                 && (    eSetType == NUMBERFORMAT_FRACTION  // and preset was fraction
                     || (nAnzNums == 3                      // or 3 numbers
                         && nStringPos > 2) ) )             // and what ???
        {
            SkipBlanks(rString, nPos);
            eScannedType = NUMBERFORMAT_FRACTION;   // !!! it IS a fraction
        }
        else
            nPos--;                                 // put '/' back
    }

    if (GetThousandSep(rString, nPos, nStringPos))  // 1,000
    {
        if (   eScannedType != NUMBERFORMAT_UNDEFINED   // already another type
            && eScannedType != NUMBERFORMAT_CURRENCY)   // except currency
            return FALSE;
        nThousand++;
    }

    const LocaleDataWrapper* pLoc = pFormatter->GetLocaleData();
    const String& rDate = pLoc->getDateSep();
    const String& rTime = pLoc->getTimeSep();
    sal_Unicode cTime = rTime.GetChar(0);
    SkipBlanks(rString, nPos);
    if (                      SkipString(rDate, rString, nPos)  // 10., 10-, 10/
        || ((cTime != '.') && SkipChar('.',   rString, nPos))   // TRICKY:
        || ((cTime != '/') && SkipChar('/',   rString, nPos))   // short boolean
        || ((cTime != '-') && SkipChar('-',   rString, nPos)) ) // evaluation!
    {
        if (   eScannedType != NUMBERFORMAT_UNDEFINED   // already another type
            && eScannedType != NUMBERFORMAT_DATE)       // except date
            return FALSE;
        SkipBlanks(rString, nPos);
        eScannedType = NUMBERFORMAT_DATE;           // !!! it IS a date
        short nTmpMonth = GetMonth(rString, nPos);  // 10. Jan 94
        if (nMonth && nTmpMonth)                    // month dup
            return FALSE;
        if (nTmpMonth)
        {
            nMonth = nTmpMonth;
            nMonthPos = 2;                          // month in the middle
            if ( nMonth < 0 )
                SkipChar( '.', rString, nPos );     // abbreviated
            SkipString( pLoc->getLongDateMonthSep(), rString, nPos );
            SkipBlanks(rString, nPos);
        }
    }

    short nTempMonth = GetMonth(rString, nPos);     // month in the middle (10 Jan 94)
    if (nTempMonth)
    {
        if (nMonth != 0)                            // month dup
            return FALSE;
        if (   eScannedType != NUMBERFORMAT_UNDEFINED   // already another type
            && eScannedType != NUMBERFORMAT_DATE)       // except date
            return FALSE;
        eScannedType = NUMBERFORMAT_DATE;           // !!! it IS a date
        nMonth = nTempMonth;
        nMonthPos = 2;                              // month in the middle
        if ( nMonth < 0 )
            SkipChar( '.', rString, nPos );         // abbreviated
        SkipString( pLoc->getLongDateMonthSep(), rString, nPos );
        SkipBlanks(rString, nPos);
    }

    if (    SkipChar('E', rString, nPos)            // 10E, 10e, 10,Ee
         || SkipChar('e', rString, nPos) )
    {
        if (eScannedType != NUMBERFORMAT_UNDEFINED) // already another type
            return FALSE;
        else
        {
            SkipBlanks(rString, nPos);
            eScannedType = NUMBERFORMAT_SCIENTIFIC; // !!! it IS scientific
            if (    nThousand+2 == nAnzNums         // special case 1.E2
                 && nDecPos == 2 )
                nDecPos = 3;                        // 1,100.E2 1,100,100.E3
        }
        nESign = GetESign(rString, nPos);           // signed exponent?
        SkipBlanks(rString, nPos);
    }

    if ( SkipString(rTime, rString, nPos) )         // time separator?
    {
        if (nDecPos)                                // already , => error
            return FALSE;
        if (   (   eScannedType == NUMBERFORMAT_DATE        // already date type
                || eScannedType == NUMBERFORMAT_DATETIME)   // or date time
            && nAnzNums > 3)                                // and more than 3 numbers? (31.Dez.94 8:23)
        {
            SkipBlanks(rString, nPos);
            eScannedType = NUMBERFORMAT_DATETIME;   // !!! it IS date with time
        }
        else if (   eScannedType != NUMBERFORMAT_UNDEFINED  // already another type
                 && eScannedType != NUMBERFORMAT_TIME)      // except time
            return FALSE;
        else
        {
            SkipBlanks(rString, nPos);
            eScannedType = NUMBERFORMAT_TIME;       // !!! it IS a time
        }
        if ( !nTimePos )
            nTimePos = nStringPos + 1;
    }

    // #68232# recognize long date separators like ", " in "September 5, 1999"
    if ( nPos < rString.Len() && eScannedType == NUMBERFORMAT_DATE
            && nMonthPos == 1 && pLoc->getLongDateFormat() == MDY )
    {
        if ( SkipString( pLoc->getLongDateDaySep(), rString, nPos )  )
            SkipBlanks( rString, nPos );
    }

    if (nPos < rString.Len())                       // not everything consumed?
        return FALSE;

    return TRUE;
}


//---------------------------------------------------------------------------
//      ScanEndString
//
// Schlussteil analysieren
// Alles weg => TRUE
// sonst     => FALSE

BOOL ImpSvNumberInputScan::ScanEndString( const String& rString,
        const SvNumberformat* pFormat )
{
    xub_StrLen nPos = 0;

    SkipBlanks(rString, nPos);
    if (GetDecSep(rString, nPos))                   // decimal separator?
    {
        if (nDecPos == 1 || nDecPos == 3)           // ,12,4 or 12,E4,
            return FALSE;
        else if (nDecPos == 2)                      // , dup: 12,4,
        {
            if (bDecSepInDateSeps)                  // , also date sep
            {
                if (eScannedType != NUMBERFORMAT_UNDEFINED &&
                    eScannedType != NUMBERFORMAT_DATE)  // already another type
                    return FALSE;
                eScannedType = NUMBERFORMAT_DATE;   // !!! it IS a date
                SkipBlanks(rString, nPos);
            }
            else
                return FALSE;
        }
        else
        {
            nDecPos = 3;                            // , in end string
            SkipBlanks(rString, nPos);
        }
    }

    if (   nSign == 0                               // conflict - not signed
        && eScannedType != NUMBERFORMAT_DATE)       // and not date
//!? catch time too?
    {                                               // not signed yet
        nSign = GetSign(rString, nPos);             // 1- DM
        if (nNegCheck)                              // '(' as sign
            return FALSE;
    }

    SkipBlanks(rString, nPos);
    if (nNegCheck && SkipChar(')', rString, nPos))  // skip ')' if appropriate
    {
        nNegCheck = 0;
        SkipBlanks(rString, nPos);
    }

    if ( GetCurrency(rString, nPos, pFormat) )      // currency symbol?
    {
        if (eScannedType != NUMBERFORMAT_UNDEFINED) // currency dup
            return FALSE;
        else
        {
            SkipBlanks(rString, nPos);
            eScannedType = NUMBERFORMAT_CURRENCY;
        }                                           // behind currency a '-' is allowed
        if (nSign == 0)                             // not signed yet
        {
            nSign = GetSign(rString, nPos);         // DM -
            SkipBlanks(rString, nPos);
            if (nNegCheck)                          // 3 DM (
                return FALSE;
        }
        if ( nNegCheck && eScannedType == NUMBERFORMAT_CURRENCY
                       && SkipChar(')', rString, nPos) )
        {
            nNegCheck = 0;                          // ')' skipped
            SkipBlanks(rString, nPos);              // only if currency
        }
    }

    if ( SkipChar('%', rString, nPos) )             // 1 %
    {
        if (eScannedType != NUMBERFORMAT_UNDEFINED) // already another type
            return FALSE;
        SkipBlanks(rString, nPos);
        eScannedType = NUMBERFORMAT_PERCENT;
    }

    const LocaleDataWrapper* pLoc = pFormatter->GetLocaleData();
    const String& rDate = pLoc->getDateSep();
    const String& rTime = pLoc->getTimeSep();
    if ( SkipString(rTime, rString, nPos) )         // 10:
    {
        if (nDecPos)                                // already , => error
            return FALSE;
        if (eScannedType == NUMBERFORMAT_DATE && nAnzNums > 2) // 31.Dez.94 8:
        {
            SkipBlanks(rString, nPos);
            eScannedType = NUMBERFORMAT_DATETIME;
        }
        else if (eScannedType != NUMBERFORMAT_UNDEFINED &&
                 eScannedType != NUMBERFORMAT_TIME) // already another type
            return FALSE;
        else
        {
            SkipBlanks(rString, nPos);
            eScannedType = NUMBERFORMAT_TIME;
        }
    }

    sal_Unicode cTime = rTime.GetChar(0);
    if (                      SkipString(rDate, rString, nPos)  // 10., 10-, 10/
        || ((cTime != '.') && SkipChar('.',   rString, nPos))   // TRICKY:
        || ((cTime != '/') && SkipChar('/',   rString, nPos))   // short boolean
        || ((cTime != '-') && SkipChar('-',   rString, nPos)) ) // evaluation!
    {
        if (eScannedType != NUMBERFORMAT_UNDEFINED &&
            eScannedType != NUMBERFORMAT_DATE)          // already another type
            return FALSE;
        else
        {
            SkipBlanks(rString, nPos);
            eScannedType = NUMBERFORMAT_DATE;
        }
        short nTmpMonth = GetMonth(rString, nPos);  // 10. Jan
        if (nMonth && nTmpMonth)                    // month dup
            return FALSE;
        if (nTmpMonth)
        {
            nMonth = nTmpMonth;
            nMonthPos = 3;                          // month at end
            if ( nMonth < 0 )
                SkipChar( '.', rString, nPos );     // abbreviated
            SkipBlanks(rString, nPos);
        }
    }

    short nTempMonth = GetMonth(rString, nPos);     // 10 Jan
    if (nTempMonth)
    {
        if (nMonth)                                 // month dup
            return FALSE;
        if (eScannedType != NUMBERFORMAT_UNDEFINED &&
            eScannedType != NUMBERFORMAT_DATE)      // already another type
            return FALSE;
        eScannedType = NUMBERFORMAT_DATE;
        nMonth = nTempMonth;
        nMonthPos = 3;                              // month at end
        if ( nMonth < 0 )
            SkipChar( '.', rString, nPos );         // abbreviated
        SkipBlanks(rString, nPos);
    }

    if (GetTimeAmPm(rString, nPos))
    {
        if (eScannedType != NUMBERFORMAT_UNDEFINED &&
            eScannedType != NUMBERFORMAT_TIME &&
            eScannedType != NUMBERFORMAT_DATETIME)  // already another type
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
        if (eScannedType == NUMBERFORMAT_CURRENCY)  // only if currency
        {
            nNegCheck = 0;                          // skip ')'
            SkipBlanks(rString, nPos);
        }
        else
            return FALSE;
    }

    if ( nPos < rString.Len() &&
            (eScannedType == NUMBERFORMAT_DATE
            || eScannedType == NUMBERFORMAT_DATETIME) )
    {   // day of week is just parsed away
        xub_StrLen nOldPos = nPos;
        const String& rSep = pFormatter->GetLocaleData()->getLongDateDayOfWeekSep();
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
                {   // short
                    if ( rString.GetChar( nPos ) == '.' )
                        ++nPos;
                }
                SkipBlanks(rString, nPos);
            }
        }
        else
            nPos = nOldPos;
    }

    if (nPos < rString.Len())                       // everything consumed?
    {
        // does input EndString equal EndString in Format?
        if ( !ScanStringNumFor( rString, nPos, pFormat, 0xFFFF ) )
            return FALSE;
    }

    return TRUE;
}


BOOL ImpSvNumberInputScan::ScanStringNumFor(
        const String& rString,          // zu scannender String
        xub_StrLen nPos,                    // Position bis zu der abgearbeitet war
        const SvNumberformat* pFormat,      // das zu matchende Format
        USHORT nString )                    // TeilString des TeilFormats
                                            // normalerweise 0 oder 0xFFFF
{
    const International* pIntl = pFormatter->GetInternational();
    if ( !pFormat )
        return FALSE;
    const String* pStr;
    String aString( rString );
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
        const String& rString,              // zu analysierender String
        double& fOutNumber,                     // OUT: Ergebnis als Zahl, wenn moeglich
        const SvNumberformat* pFormat )         // evtl. gesetztes Zahlenformat
{
    Reset();                                    // Anfangszustand
    // NoMoreUpperNeeded, alle Vergleiche auf UpperCase
    String aString( pFormatter->GetCharClass()->upper( rString ) );
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
            String& rStrArray = sStrArray[0];
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
                        sStrArray[i] == pFormatter->GetLocaleData()->getNumDecimalSep())
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
                        sStrArray[i] == pFormatter->GetLocaleData()->getNumDecimalSep())
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
// Initialize uppercase months, weekdays and currency

void ImpSvNumberInputScan::InitText()
{
    sal_Int16 j, nElems;
    const CharClass* pChrCls = pFormatter->GetCharClass();
    const LocaleDataWrapper* pLoc = pFormatter->GetLocaleData();
    const CalendarWrapper* pCal = pFormatter->GetCalendar();
    delete [] pUpperMonthText;
    delete [] pUpperAbbrevMonthText;
    ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::CalendarItem > xElems
        = pCal->getMonths();
    nElems = xElems.getLength();
    pUpperMonthText = new String[nElems];
    pUpperAbbrevMonthText = new String[nElems];
    for ( j=0; j<nElems; j++ )
    {
        pUpperMonthText[j] = pChrCls->upper( xElems[j].FullName );
        pUpperAbbrevMonthText[j] = pChrCls->upper( xElems[j].AbbrevName );
    }
    delete [] pUpperDayText;
    delete [] pUpperAbbrevDayText;
    xElems = pCal->getDays();
    nElems = xElems.getLength();
    pUpperDayText = new String[nElems];
    pUpperAbbrevDayText = new String[nElems];
    for ( j=0; j<nElems; j++ )
    {
        pUpperDayText[j] = pChrCls->upper( xElems[j].FullName );
        pUpperAbbrevDayText[j] = pChrCls->upper( xElems[j].AbbrevName );
    }
    aUpperCurrSymbol = pChrCls->upper( pLoc->getCurrSymbol() );
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
    sal_Unicode cDecSep = pFormatter->GetLocaleData()->getNumDecimalSep().GetChar(0);
    bDecSepInDateSeps = ( cDecSep == '-' ||
                          cDecSep == '/' ||
                          cDecSep == '.' ||
                          cDecSep == pFormatter->GetLocaleData()->getDateSep().GetChar(0) );
    bTextInitialized = FALSE;
}


//---------------------------------------------------------------------------
//      ChangeNullDate

void ImpSvNumberInputScan::ChangeNullDate(
        const USHORT nDay,
        const USHORT nMonth,
        const USHORT nYear )
{
    if ( pNullDate )
        *pNullDate = Date(nDay, nMonth, nYear);
    else
        pNullDate = new Date(nDay, nMonth, nYear);
}


//---------------------------------------------------------------------------
//      IsNumberFormat
//
// => String als Zahl darstellbar

BOOL ImpSvNumberInputScan::IsNumberFormat(
        const String& rString,              // zu analysierender String
        short& F_Type,                          // IN: alter Typ, OUT: neuer Typ
        double& fOutNumber,                     // OUT: Zahl, wenn Umwandlung moeglich
        const SvNumberformat* pFormat )         // evtl. gesetztes Zahlenformat
{
    // in den zerlegten Strings gibt es keine Null-Laengen Strings mehr!

    String sResString;                      // die Eingabe fuer atof
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



