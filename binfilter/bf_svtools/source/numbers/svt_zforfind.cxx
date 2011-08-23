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


#include <ctype.h>
#include <stdlib.h>
#include <float.h>
#include <errno.h>

#ifndef _DATE_HXX //autogen
#include <tools/date.hxx>
#endif
#ifndef _DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif
#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
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

#include <bf_svtools/zforlist.hxx>         // NUMBERFORMAT_XXX
#include "zforscan.hxx"
#include <bf_svtools/zformat.hxx>

#define _ZFORFIND_CXX
#include "zforfind.hxx"
#undef _ZFORFIND_CXX


#ifndef DBG_UTIL
#define NF_TEST_CALENDAR 0
#else
#define NF_TEST_CALENDAR 0
#endif
#if NF_TEST_CALENDAR
#include <comphelper/processfactory.hxx>
#include <com/sun/star/i18n/XExtendedCalendar.hpp>
#endif

namespace binfilter
{


const BYTE ImpSvNumberInputScan::nMatchedEndString    = 0x01;
const BYTE ImpSvNumberInputScan::nMatchedMidString    = 0x02;
const BYTE ImpSvNumberInputScan::nMatchedStartString  = 0x04;
const BYTE ImpSvNumberInputScan::nMatchedVirgin       = 0x08;
const BYTE ImpSvNumberInputScan::nMatchedUsedAsReturn = 0x10;

/* It is not clear how we want timezones to be handled. Convert them to local
 * time isn't wanted, as it isn't done in any other place and timezone
 * information isn't stored anywhere. Ignoring them and pretending local time
 * may be wrong too and might not be what the user expects. Keep the input as
 * string so that no information is lost.
 * Anyway, defining NF_RECOGNIZE_ISO8601_TIMEZONES to 1 would be the way how it
 * would work, together with the nTimezonePos handling in GetTimeRef(). */
#define NF_RECOGNIZE_ISO8601_TIMEZONES 0

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
    nMatchedAllStrings = nMatchedVirgin;
    nMayBeIso8601 = 0;
    nTimezonePos = 0;
}


//---------------------------------------------------------------------------
//
// static
inline BOOL ImpSvNumberInputScan::MyIsdigit( sal_Unicode c )
{
    // If the input string wouldn't be converted using TransformInput() we'd
    // to use something similar to the following and to adapt many places.
#if 0
    // use faster isdigit() if possible
    if ( c < 128 )
        return isdigit( (unsigned char) c ) != 0;
    if ( c < 256 )
        return FALSE;
    String aTmp( c );
    return pFormatter->GetCharClass()->isDigit( aTmp, 0 );
#else
    return c < 128 && isdigit( (unsigned char) c );
#endif
}


//---------------------------------------------------------------------------
//
void ImpSvNumberInputScan::TransformInput( String& rStr )
{
    xub_StrLen nPos, nLen;
    for ( nPos = 0, nLen = rStr.Len(); nPos < nLen; ++nPos )
    {
        if ( 256 <= rStr.GetChar( nPos ) &&
                pFormatter->GetCharClass()->isDigit( rStr, nPos ) )
            break;
    }
    if ( nPos < nLen )
        rStr = pFormatter->GetNatNum()->getNativeNumberString( rStr,
                pFormatter->GetLocale(), 0 );
}


//---------------------------------------------------------------------------
//      StringToDouble
//
// Only simple unsigned floating point values without any error detection,
// decimal separator has to be '.'

double ImpSvNumberInputScan::StringToDouble( const String& rStr, BOOL bForceFraction )
{
    double fNum = 0.0;
    double fFrac = 0.0;
    int nExp = 0;
    xub_StrLen nPos = 0;
    xub_StrLen nLen = rStr.Len();
    BOOL bPreSep = !bForceFraction;

    while (nPos < nLen)
    {
        if (rStr.GetChar(nPos) == '.')
            bPreSep = FALSE;
        else if (bPreSep)
            fNum = fNum * 10.0 + (double) (rStr.GetChar(nPos) - '0');
        else
        {
            fFrac = fFrac * 10.0 + (double) (rStr.GetChar(nPos) - '0');
            --nExp;
        }
        nPos++;
    }
    if ( fFrac )
        return fNum + ::rtl::math::pow10Exp( fFrac, nExp );
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
    const String& rThSep = pFormatter->GetNumThousandSep();
    register const sal_Unicode* pHere = pStr;
    ScanState eState = SsStart;
    xub_StrLen nCounter = 0;                                // counts 3 digits

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
    const sal_Unicode* pStr = rString.GetBuffer();
    const sal_Unicode* const pEnd = pStr + rString.Len();
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
        nPos = nPos + rWhat.Len();
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
    const String& rSep = pFormatter->GetNumThousandSep();
    // Is it an ordinary space instead of a non-breaking space?
    bool bSpaceBreak = rSep.GetChar(0) == 0xa0 && rString.GetChar(0) == 0x20 &&
        rSep.Len() == 1 && rString.Len() == 1;
    if ( (rString == rSep || bSpaceBreak)                   // nothing else
        && nStringPos < nAnzStrings - 1                     // safety first!
        && IsNum[nStringPos+1]                              // number follows
        && (   sStrArray[nStringPos+1].Len() == 3           // with 3 digits
            || nPosThousandString == nStringPos+1 ) )       // or concatenated
    {
        nPos = nPos + rSep.Len();
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
    // #102136# The correct English form of month September abbreviated is
    // SEPT, but almost every data contains SEP instead.
    static const String aSeptCorrect( RTL_CONSTASCII_USTRINGPARAM( "SEPT" ) );
    static const String aSepShortened( RTL_CONSTASCII_USTRINGPARAM( "SEP" ) );

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
                nPos = nPos + pUpperMonthText[i].Len();
                res = i+1;
                break;  // for
            }
            else if ( StringContains( pUpperAbbrevMonthText[i], rString, nPos ) )
            {                                           // abbreviated
                nPos = nPos + pUpperAbbrevMonthText[i].Len();
                res = sal::static_int_cast< short >(-(i+1)); // negative
                break;  // for
            }
            else if ( i == 8 && pUpperAbbrevMonthText[i] == aSeptCorrect &&
                    StringContains( aSepShortened, rString, nPos ) )
            {                                           // #102136# SEPT/SEP
                nPos = nPos + aSepShortened.Len();
                res = sal::static_int_cast< short >(-(i+1)); // negative
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

int ImpSvNumberInputScan::GetDayOfWeek( const String& rString, xub_StrLen& nPos )
{
    int res = 0;      // no day found

    if (rString.Len() > nPos)                           // only if needed
    {
        if ( !bTextInitialized )
            InitText();
        sal_Int16 nDays = pFormatter->GetCalendar()->getNumberOfDaysInWeek();
        for ( sal_Int16 i = 0; i < nDays; i++ )
        {
            if ( StringContains( pUpperDayText[i], rString, nPos ) )
            {                                           // full names first
                nPos = nPos + pUpperDayText[i].Len();
                res = i + 1;
                break;  // for
            }
            if ( StringContains( pUpperAbbrevDayText[i], rString, nPos ) )
            {                                           // abbreviated
                nPos = nPos + pUpperAbbrevDayText[i].Len();
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
        if ( !aUpperCurrSymbol.Len() )
        {   // if no format specified the currency of the initialized formatter
            LanguageType eLang = (pFormat ? pFormat->GetLanguage() :
                pFormatter->GetLanguage());
            aUpperCurrSymbol = pFormatter->GetCharClass()->upper(
                SvNumberFormatter::GetCurrencyEntry( eLang ).GetSymbol() );
        }
        if ( StringContains( aUpperCurrSymbol, rString, nPos ) )
        {
            nPos = nPos + aUpperCurrSymbol.Len();
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
                        nPos = nPos + aSymbol.Len();
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
            nPos = nPos + pLoc->getTimeAM().Len();
            return TRUE;
        }
        else if ( StringContains( pChr->upper( pLoc->getTimePM() ), rString, nPos ) )
        {
            nAmPm = -1;
            nPos = nPos + pLoc->getTimePM().Len();
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
        const String& rSep = pFormatter->GetNumDecimalSep();
        if ( rString.Equals( rSep, nPos, rSep.Len() ) )
        {
            nPos = nPos + rSep.Len();
            return TRUE;
        }
    }
    return FALSE;
}


//---------------------------------------------------------------------------
// read a hundredth seconds separator

inline BOOL ImpSvNumberInputScan::GetTime100SecSep( const String& rString, xub_StrLen& nPos )
{
    if ( rString.Len() > nPos )
    {
        const String& rSep = pFormatter->GetLocaleData()->getTime100SecSep();
        if ( rString.Equals( rSep, nPos, rSep.Len() ) )
        {
            nPos = nPos + rSep.Len();
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

int ImpSvNumberInputScan::GetSign( const String& rString, xub_StrLen& nPos )
{
    if (rString.Len() > nPos)
        switch (rString.GetChar(nPos))
        {
            case '+':
                nPos++;
                return 1;
            case '(':               // '(' aehnlich wie '-' ?!?
                nNegCheck = 1;
                //! fallthru
            case '-':
                nPos++;
                return -1;
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
            case '-':
                nPos++;
                return -1;
            default:
                break;
        }

    return 0;
}


//---------------------------------------------------------------------------
//      GetNextNumber
//
// i counts string portions, j counts numbers thereof.
// It should had been called SkipNumber instead.

inline BOOL ImpSvNumberInputScan::GetNextNumber( USHORT& i, USHORT& j )
{
    if ( i < nAnzStrings && IsNum[i] )
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
        USHORT nIndex,          // j-value of the first numeric time part of input, default 0
        USHORT nAnz )           // count of numeric time parts
{
    USHORT nHour;
    USHORT nMinute = 0;
    USHORT nSecond = 0;
    double fSecond100 = 0.0;
    USHORT nStartIndex = nIndex;

    if (nTimezonePos)
    {
        // find first timezone number index and adjust count
        for (USHORT j=0; j<nAnzNums; ++j)
        {
            if (nNums[j] == nTimezonePos)
            {
                // nAnz is not total count, but count of time relevant strings.
                if (nStartIndex < j && j - nStartIndex < nAnz)
                    nAnz = j - nStartIndex;
                break;  // for
            }
        }
    }

    if (nDecPos == 2 && (nAnz == 3 || nAnz == 2))   // 20:45.5 or 45.5
        nHour = 0;
    else if (nIndex - nStartIndex < nAnz)
        nHour   = (USHORT) sStrArray[nNums[nIndex++]].ToInt32();
    else
    {
        nHour = 0;
        DBG_ERRORFILE( "ImpSvNumberInputScan::GetTimeRef: bad number index");
    }
    if (nDecPos == 2 && nAnz == 2)                  // 45.5
        nMinute = 0;
    else if (nIndex - nStartIndex < nAnz)
        nMinute = (USHORT) sStrArray[nNums[nIndex++]].ToInt32();
    if (nIndex - nStartIndex < nAnz)
        nSecond = (USHORT) sStrArray[nNums[nIndex++]].ToInt32();
    if (nIndex - nStartIndex < nAnz)
        fSecond100 = StringToDouble( sStrArray[nNums[nIndex]], TRUE );
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

bool ImpSvNumberInputScan::MayBeIso8601()
{
    if (nMayBeIso8601 == 0)
    {
        if (nAnzNums >= 3 && nNums[0] < nAnzStrings &&
                sStrArray[nNums[0]].ToInt32() > 31)
            nMayBeIso8601 = 1;
        else
            nMayBeIso8601 = 2;
    }
    return nMayBeIso8601 == 1;
}

//---------------------------------------------------------------------------
//      GetDateRef

BOOL ImpSvNumberInputScan::GetDateRef( double& fDays, USHORT& nCounter,
        const SvNumberformat* pFormat )
{
    using namespace ::com::sun::star::i18n;
    NfEvalDateFormat eEDF;
    int nFormatOrder;
    if ( pFormat && ((pFormat->GetType() & NUMBERFORMAT_DATE) == NUMBERFORMAT_DATE) )
    {
        eEDF = pFormatter->GetEvalDateFormat();
        switch ( eEDF )
        {
            case NF_EVALDATEFORMAT_INTL :
            case NF_EVALDATEFORMAT_FORMAT :
                nFormatOrder = 1;       // only one loop
            break;
            default:
                nFormatOrder = 2;
                if ( nMatchedAllStrings )
                    eEDF = NF_EVALDATEFORMAT_FORMAT_INTL;
                    // we have a complete match, use it
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
        pCal->setGregorianDateTime( Date() );       // today
        String aOrgCalendar;        // empty => not changed yet
        DateFormat DateFmt;
        BOOL bFormatTurn;
        switch ( eEDF )
        {
            case NF_EVALDATEFORMAT_INTL :
                bFormatTurn = FALSE;
                DateFmt = pLoc->getDateFormat();
            break;
            case NF_EVALDATEFORMAT_FORMAT :
                bFormatTurn = TRUE;
                DateFmt = pFormat->GetDateOrder();
            break;
            case NF_EVALDATEFORMAT_INTL_FORMAT :
                if ( nTryOrder == 1 )
                {
                    bFormatTurn = FALSE;
                    DateFmt = pLoc->getDateFormat();
                }
                else
                {
                    bFormatTurn = TRUE;
                    DateFmt = pFormat->GetDateOrder();
                }
            break;
            case NF_EVALDATEFORMAT_FORMAT_INTL :
                if ( nTryOrder == 2 )
                {
                    bFormatTurn = FALSE;
                    DateFmt = pLoc->getDateFormat();
                }
                else
                {
                    bFormatTurn = TRUE;
                    DateFmt = pFormat->GetDateOrder();
                }
            break;
            default:
                DBG_ERROR( "ImpSvNumberInputScan::GetDateRef: unknown NfEvalDateFormat" );
                DateFmt = YMD;
                bFormatTurn = FALSE;
        }
        if ( bFormatTurn )
        {
#if 0
/* TODO:
We are currently not able to fully support a switch to another calendar during
input for the following reasons:
1. We do have a problem if both (locale's default and format's) calendars
   define the same YMD order and use the same date separator, there is no way
   to distinguish between them if the input results in valid calendar input for
   both calendars. How to solve? Would NfEvalDateFormat be sufficient? Should
   it always be set to NF_EVALDATEFORMAT_FORMAT_INTL and thus the format's
   calendar be preferred? This could be confusing if a Calc cell was formatted
   different to the locale's default and has no content yet, then the user has
   no clue about the format or calendar being set.
2. In Calc cell edit mode a date is always displayed and edited using the
   default edit format of the default calendar (normally being Gregorian). If
   input was ambiguous due to issue #1 we'd need a mechanism to tell that a
   date was edited and not newly entered. Not feasible. Otherwise we'd need a
   mechanism to use a specific edit format with a specific calendar according
   to the format set.
3. For some calendars like Japanese Gengou we'd need era input, which isn't
   implemented at all. Though this is a rare and special case, forcing a
   calendar dependent edit format as suggested in item #2 might require era
   input, if it shouldn't result in a fallback to Gregorian calendar.
4. Last and least: the GetMonth() method currently only matches month names of
   the default calendar. Alternating month names of the actual format's
   calendar would have to be implemented. No problem.

*/
            if ( pFormat->IsOtherCalendar( nStringScanNumFor ) )
                pFormat->SwitchToOtherCalendar( aOrgCalendar, fOrgDateTime );
            else
                pFormat->SwitchToSpecifiedCalendar( aOrgCalendar, fOrgDateTime,
                        nStringScanNumFor );
#endif
        }

        res = TRUE;
        nCounter = 0;
        // For incomplete dates, always assume first day of month if not specified.
        pCal->setValue( CalendarFieldIndex::DAY_OF_MONTH, 1 );

        switch (nAnzNums)       // count of numbers in string
        {
            case 0:                 // none
                if (nMonthPos)          // only month (Jan)
                    pCal->setValue( CalendarFieldIndex::MONTH, Abs(nMonth)-1 );
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
                    {
                        bool bHadExact;
                        sal_uInt32 nExactDateOrder = (bFormatTurn ? pFormat->GetExactDateOrder() : 0);
                        if ( 0xff < nExactDateOrder && nExactDateOrder <= 0xffff )
                        {   // formatted as date and exactly 2 parts
                            bHadExact = true;
                            switch ( (nExactDateOrder >> 8) & 0xff )
                            {
                                case 'Y':
                                    pCal->setValue( CalendarFieldIndex::YEAR, ImplGetYear(0) );
                                break;
                                case 'M':
                                    pCal->setValue( CalendarFieldIndex::MONTH, ImplGetMonth(0) );
                                break;
                                case 'D':
                                    pCal->setValue( CalendarFieldIndex::DAY_OF_MONTH, ImplGetDay(0) );
                                break;
                                default:
                                    bHadExact = false;
                            }
                            switch ( nExactDateOrder & 0xff )
                            {
                                case 'Y':
                                    pCal->setValue( CalendarFieldIndex::YEAR, ImplGetYear(1) );
                                break;
                                case 'M':
                                    pCal->setValue( CalendarFieldIndex::MONTH, ImplGetMonth(1) );
                                break;
                                case 'D':
                                    pCal->setValue( CalendarFieldIndex::DAY_OF_MONTH, ImplGetDay(1) );
                                break;
                                default:
                                    bHadExact = false;
                            }
                        }
                        else
                            bHadExact = false;
                        if ( !bHadExact || !pCal->isValid() )
                        {
                            if ( !bHadExact && nExactDateOrder )
                                pCal->setGregorianDateTime( Date() );   // reset today
                            switch (DateFmt)
                            {
                                case MDY:
                                    // M D
                                    pCal->setValue( CalendarFieldIndex::DAY_OF_MONTH, ImplGetDay(1) );
                                    pCal->setValue( CalendarFieldIndex::MONTH, ImplGetMonth(0) );
                                    break;
                                case DMY:
                                    // D M
                                    pCal->setValue( CalendarFieldIndex::DAY_OF_MONTH, ImplGetDay(0) );
                                    pCal->setValue( CalendarFieldIndex::MONTH, ImplGetMonth(1) );
                                    if ( !pCal->isValid() )             // 2nd try
                                    {                                   // M Y
                                        pCal->setValue( CalendarFieldIndex::DAY_OF_MONTH, 1 );
                                        pCal->setValue( CalendarFieldIndex::MONTH, ImplGetMonth(0) );
                                        pCal->setValue( CalendarFieldIndex::YEAR, ImplGetYear(1) );
                                    }
                                    break;
                                case YMD:
                                    // M D
                                    pCal->setValue( CalendarFieldIndex::DAY_OF_MONTH, ImplGetDay(1) );
                                    pCal->setValue( CalendarFieldIndex::MONTH, ImplGetMonth(0) );
                                    if ( !pCal->isValid() )             // 2nd try
                                    {                                   // Y M
                                        pCal->setValue( CalendarFieldIndex::DAY_OF_MONTH, 1 );
                                        pCal->setValue( CalendarFieldIndex::MONTH, ImplGetMonth(1) );
                                        pCal->setValue( CalendarFieldIndex::YEAR, ImplGetYear(0) );
                                    }
                                    break;
                                default:
                                    res = FALSE;
                                    break;
                            }
                        }
                    }
                    break;
                    case 1:             // month at the beginning (Jan 01 01)
                    {
                        // The input is valid as MDY in almost any
                        // constellation, there is no date order (M)YD except if
                        // set in a format applied.
                        pCal->setValue( CalendarFieldIndex::MONTH, Abs(nMonth)-1 );
                        sal_uInt32 nExactDateOrder = (bFormatTurn ? pFormat->GetExactDateOrder() : 0);
                        if ((((nExactDateOrder >> 8) & 0xff) == 'Y') && ((nExactDateOrder & 0xff) == 'D'))
                        {
                            pCal->setValue( CalendarFieldIndex::DAY_OF_MONTH, ImplGetDay(1) );
                            pCal->setValue( CalendarFieldIndex::YEAR, ImplGetYear(0) );
                        }
                        else
                        {
                            pCal->setValue( CalendarFieldIndex::DAY_OF_MONTH, ImplGetDay(0) );
                            pCal->setValue( CalendarFieldIndex::YEAR, ImplGetYear(1) );
                        }
                    }
                    break;
                    case 2:             // month in the middle (10 Jan 94)
                        pCal->setValue( CalendarFieldIndex::MONTH, Abs(nMonth)-1 );
                        switch (DateFmt)
                        {
                            case MDY:   // yes, "10-Jan-94" is valid
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
                    {
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
                        // ISO 8601 yyyy-mm-dd forced recognition
                        DateFormat eDF = (MayBeIso8601() ? YMD : DateFmt);
                        switch (eDF)
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
                                pCal->setValue( CalendarFieldIndex::YEAR, ImplGetYear(0) );
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

        if ( res && pCal->isValid() )
        {
            double fDiff = DateTime(*pNullDate) - pCal->getEpochStart();
            fDays = ::rtl::math::approxFloor( pCal->getLocalDateTime() );
            fDays -= fDiff;
            nTryOrder = nFormatOrder;   // break for
        }
        else
            res = FALSE;

        if ( aOrgCalendar.Len() )
            pCal->loadCalendar( aOrgCalendar, pLoc->getLocale() );  // restore calendar

#if NF_TEST_CALENDAR
{
    using namespace ::com::sun::star;
    struct entry { const char* lan; const char* cou; const char* cal; };
    const entry cals[] = {
        { "en", "US",  "gregorian" },
        { "ar", "TN",      "hijri" },
        { "he", "IL",     "jewish" },
        { "ja", "JP",     "gengou" },
        { "ko", "KR", "hanja_yoil" },
        { "th", "TH",   "buddhist" },
        { "zh", "TW",        "ROC" },
        0
    };
    lang::Locale aLocale;
    sal_Bool bValid;
    sal_Int16 nDay, nMonth, nYear, nHour, nMinute, nSecond;
    sal_Int16 nDaySet, nMonthSet, nYearSet, nHourSet, nMinuteSet, nSecondSet;
    sal_Int16 nZO, nDST1, nDST2, nDST;
    uno::Reference< lang::XMultiServiceFactory > xSMgr =
        ::comphelper::getProcessServiceFactory();
    uno::Reference< ::com::sun::star::i18n::XExtendedCalendar > xCal(
            xSMgr->createInstance( ::rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "com.sun.star.i18n.LocaleCalendar" ) ) ),
            uno::UNO_QUERY );
    for ( const entry* p = cals; p->lan; ++p )
    {
        aLocale.Language = ::rtl::OUString::createFromAscii( p->lan );
        aLocale.Country = ::rtl::OUString::createFromAscii( p->cou );
        xCal->loadCalendar( ::rtl::OUString::createFromAscii( p->cal ),
                aLocale );
        double nDateTime = 0.0;     // 1-Jan-1970 00:00:00
        nZO        = xCal->getValue( i18n::CalendarFieldIndex::ZONE_OFFSET );
        nDST1      = xCal->getValue( i18n::CalendarFieldIndex::DST_OFFSET );
        nDateTime -= (double)(nZO + nDST1) / 60.0 / 24.0;
        xCal->setDateTime( nDateTime );
        nDST2      = xCal->getValue( i18n::CalendarFieldIndex::DST_OFFSET );
        if ( nDST1 != nDST2 )
        {
            nDateTime = 0.0 - (double)(nZO + nDST2) / 60.0 / 24.0;
            xCal->setDateTime( nDateTime );
        }
        nDaySet    = xCal->getValue( i18n::CalendarFieldIndex::DAY_OF_MONTH );
        nMonthSet  = xCal->getValue( i18n::CalendarFieldIndex::MONTH );
        nYearSet   = xCal->getValue( i18n::CalendarFieldIndex::YEAR );
        nHourSet   = xCal->getValue( i18n::CalendarFieldIndex::HOUR );
        nMinuteSet = xCal->getValue( i18n::CalendarFieldIndex::MINUTE );
        nSecondSet = xCal->getValue( i18n::CalendarFieldIndex::SECOND );
        nZO        = xCal->getValue( i18n::CalendarFieldIndex::ZONE_OFFSET );
        nDST       = xCal->getValue( i18n::CalendarFieldIndex::DST_OFFSET );
        xCal->setValue( i18n::CalendarFieldIndex::DAY_OF_MONTH, nDaySet );
        xCal->setValue( i18n::CalendarFieldIndex::MONTH, nMonthSet );
        xCal->setValue( i18n::CalendarFieldIndex::YEAR, nYearSet );
        xCal->setValue( i18n::CalendarFieldIndex::HOUR, nHourSet );
        xCal->setValue( i18n::CalendarFieldIndex::MINUTE, nMinuteSet );
        xCal->setValue( i18n::CalendarFieldIndex::SECOND, nSecondSet );
        bValid  = xCal->isValid();
        nDay    = xCal->getValue( i18n::CalendarFieldIndex::DAY_OF_MONTH );
        nMonth  = xCal->getValue( i18n::CalendarFieldIndex::MONTH );
        nYear   = xCal->getValue( i18n::CalendarFieldIndex::YEAR );
        nHour   = xCal->getValue( i18n::CalendarFieldIndex::HOUR );
        nMinute = xCal->getValue( i18n::CalendarFieldIndex::MINUTE );
        nSecond = xCal->getValue( i18n::CalendarFieldIndex::SECOND );
        bValid = bValid && nDay == nDaySet && nMonth == nMonthSet && nYear ==
            nYearSet && nHour == nHourSet && nMinute == nMinuteSet && nSecond
            == nSecondSet;
    }
}
#endif  // NF_TEST_CALENDAR

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
    int nDayOfWeek;

    // First of all, eat leading blanks
    SkipBlanks(rString, nPos);

    // Yes, nMatchedAllStrings should know about the sign position
    nSign = GetSign(rString, nPos);
    if ( nSign )           // sign?
        SkipBlanks(rString, nPos);

    // #102371# match against format string only if start string is not a sign character
    if ( nMatchedAllStrings && !(nSign && rString.Len() == 1) )
    {   // Match against format in any case, so later on for a "x1-2-3" input
        // we may distinguish between a xy-m-d (or similar) date and a x0-0-0
        // format. No sign detection here!
        if ( ScanStringNumFor( rString, nPos, pFormat, 0, TRUE ) )
            nMatchedAllStrings |= nMatchedStartString;
        else
            nMatchedAllStrings = 0;
    }

    if ( GetDecSep(rString, nPos) )                 // decimal separator in start string
    {
        nDecPos = 1;
        SkipBlanks(rString, nPos);
    }
    else if ( GetCurrency(rString, nPos, pFormat) ) // currency (DM 1)?
    {
        eScannedType = NUMBERFORMAT_CURRENCY;       // !!! it IS currency !!!
        SkipBlanks(rString, nPos);
        if (nSign == 0)                             // no sign yet
        {
            nSign = GetSign(rString, nPos);
            if ( nSign )   // DM -1
                SkipBlanks(rString, nPos);
        }
    }
    else
    {
        nMonth = GetMonth(rString, nPos);
        if ( nMonth )    // month (Jan 1)?
        {
            eScannedType = NUMBERFORMAT_DATE;       // !!! it IS a date !!!
            nMonthPos = 1;                          // month at the beginning
            if ( nMonth < 0 )
                SkipChar( '.', rString, nPos );     // abbreviated
            SkipBlanks(rString, nPos);
        }
        else
        {
            nDayOfWeek = GetDayOfWeek( rString, nPos );
            if ( nDayOfWeek )
            {   // day of week is just parsed away
                eScannedType = NUMBERFORMAT_DATE;       // !!! it IS a date !!!
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
                    nMonth = GetMonth(rString, nPos);
                    if ( nMonth ) // month (Jan 1)?
                    {
                        nMonthPos = 1;                  // month a the beginning
                        if ( nMonth < 0 )
                            SkipChar( '.', rString, nPos ); // abbreviated
                        SkipBlanks(rString, nPos);
                    }
                }
            }
        }
    }

    if (nPos < rString.Len())                       // not everything consumed
    {
        // Does input StartString equal StartString of format?
        // This time with sign detection!
        if ( !ScanStringNumFor( rString, nPos, pFormat, 0 ) )
            return MatchedReturn();
    }

    return TRUE;
}


//---------------------------------------------------------------------------
//      ScanMidString
//
// String in der Mitte analysieren
// Alles weg => TRUE
// sonst     => FALSE

BOOL ImpSvNumberInputScan::ScanMidString( const String& rString,
        USHORT nStringPos, const SvNumberformat* pFormat )
{
    xub_StrLen nPos = 0;
    short eOldScannedType = eScannedType;

    if ( nMatchedAllStrings )
    {   // Match against format in any case, so later on for a "1-2-3-4" input
        // we may distinguish between a y-m-d (or similar) date and a 0-0-0-0
        // format.
        if ( ScanStringNumFor( rString, 0, pFormat, nStringPos ) )
            nMatchedAllStrings |= nMatchedMidString;
        else
            nMatchedAllStrings = 0;
    }

    SkipBlanks(rString, nPos);
    if (GetDecSep(rString, nPos))                   // decimal separator?
    {
        if (nDecPos == 1 || nDecPos == 3)           // .12.4 or 1.E2.1
            return MatchedReturn();
        else if (nDecPos == 2)                      // . dup: 12.4.
        {
            if (bDecSepInDateSeps)                  // . also date separator
            {
                if (    eScannedType != NUMBERFORMAT_UNDEFINED &&
                        eScannedType != NUMBERFORMAT_DATE &&
                        eScannedType != NUMBERFORMAT_DATETIME)  // already another type
                    return MatchedReturn();
                if (eScannedType == NUMBERFORMAT_UNDEFINED)
                    eScannedType = NUMBERFORMAT_DATE;   // !!! it IS a date
                SkipBlanks(rString, nPos);
            }
            else
                return MatchedReturn();
        }
        else
        {
            nDecPos = 2;                            // . in mid string
            SkipBlanks(rString, nPos);
        }
    }
    else if ( ((eScannedType & NUMBERFORMAT_TIME) == NUMBERFORMAT_TIME)
            && GetTime100SecSep( rString, nPos ) )
    {                                               // hundredth seconds separator
        if ( nDecPos )
            return MatchedReturn();
        nDecPos = 2;                                // . in mid string
        SkipBlanks(rString, nPos);
    }

    if (SkipChar('/', rString, nPos))               // fraction?
    {
        if (   eScannedType != NUMBERFORMAT_UNDEFINED   // already another type
            && eScannedType != NUMBERFORMAT_DATE)       // except date
            return MatchedReturn();                               // => jan/31/1994
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
            return MatchedReturn();
        nThousand++;
    }

    const LocaleDataWrapper* pLoc = pFormatter->GetLocaleData();
    const String& rDate = pFormatter->GetDateSep();
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
            return MatchedReturn();
        SkipBlanks(rString, nPos);
        eScannedType = NUMBERFORMAT_DATE;           // !!! it IS a date
        short nTmpMonth = GetMonth(rString, nPos);  // 10. Jan 94
        if (nMonth && nTmpMonth)                    // month dup
            return MatchedReturn();
        if (nTmpMonth)
        {
            nMonth = nTmpMonth;
            nMonthPos = 2;                          // month in the middle
            if ( nMonth < 0 && SkipChar( '.', rString, nPos ) )
                ;   // short month may be abbreviated Jan.
            else if ( SkipChar( '-', rString, nPos ) )
                ;   // #79632# recognize 17-Jan-2001 to be a date
                    // #99065# short and long month name
            else
                SkipString( pLoc->getLongDateMonthSep(), rString, nPos );
            SkipBlanks(rString, nPos);
        }
    }

    short nTempMonth = GetMonth(rString, nPos);     // month in the middle (10 Jan 94)
    if (nTempMonth)
    {
        if (nMonth != 0)                            // month dup
            return MatchedReturn();
        if (   eScannedType != NUMBERFORMAT_UNDEFINED   // already another type
            && eScannedType != NUMBERFORMAT_DATE)       // except date
            return MatchedReturn();
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
            return MatchedReturn();
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
        if (nDecPos)                                // already . => maybe error
        {
            if (bDecSepInDateSeps)                  // . also date sep
            {
                if (    eScannedType != NUMBERFORMAT_DATE &&    // already another type than date
                        eScannedType != NUMBERFORMAT_DATETIME)  // or date time
                    return MatchedReturn();
                if (eScannedType == NUMBERFORMAT_DATE)
                    nDecPos = 0;                    // reset for time transition
            }
            else
                return MatchedReturn();
        }
        if (   (   eScannedType == NUMBERFORMAT_DATE        // already date type
                || eScannedType == NUMBERFORMAT_DATETIME)   // or date time
            && nAnzNums > 3)                                // and more than 3 numbers? (31.Dez.94 8:23)
        {
            SkipBlanks(rString, nPos);
            eScannedType = NUMBERFORMAT_DATETIME;   // !!! it IS date with time
        }
        else if (   eScannedType != NUMBERFORMAT_UNDEFINED  // already another type
                 && eScannedType != NUMBERFORMAT_TIME)      // except time
            return MatchedReturn();
        else
        {
            SkipBlanks(rString, nPos);
            eScannedType = NUMBERFORMAT_TIME;       // !!! it IS a time
        }
        if ( !nTimePos )
            nTimePos = nStringPos + 1;
    }

    if (nPos < rString.Len())
    {
        switch (eScannedType)
        {
            case NUMBERFORMAT_DATE:
                if (nMonthPos == 1 && pLoc->getLongDateFormat() == MDY)
                {
                    // #68232# recognize long date separators like ", " in "September 5, 1999"
                    if (SkipString( pLoc->getLongDateDaySep(), rString, nPos ))
                        SkipBlanks( rString, nPos );
                }
                else if (nStringPos == 5 && nPos == 0 && rString.Len() == 1 &&
                        rString.GetChar(0) == 'T' && MayBeIso8601())
                {
                    // ISO 8601 combined date and time, yyyy-mm-ddThh:mm
                    ++nPos;
                }
                break;
#if NF_RECOGNIZE_ISO8601_TIMEZONES
            case NUMBERFORMAT_DATETIME:
                if (nPos == 0 && rString.Len() == 1 && nStringPos >= 9 &&
                        MayBeIso8601())
                {
                    // ISO 8601 timezone offset
                    switch (rString.GetChar(0))
                    {
                        case '+':
                        case '-':
                            if (nStringPos == nAnzStrings-2 ||
                                    nStringPos == nAnzStrings-4)
                            {
                                ++nPos;     // yyyy-mm-ddThh:mm[:ss]+xx[[:]yy]
                                // nTimezonePos needed for GetTimeRef()
                                if (!nTimezonePos)
                                    nTimezonePos = nStringPos + 1;
                            }
                            break;
                        case ':':
                            if (nTimezonePos && nStringPos >= 11 &&
                                    nStringPos == nAnzStrings-2)
                                ++nPos;     // yyyy-mm-ddThh:mm[:ss]+xx:yy
                            break;
                    }
                }
                break;
#endif
        }
    }

    if (nPos < rString.Len())                       // not everything consumed?
    {
        if ( nMatchedAllStrings & ~nMatchedVirgin )
            eScannedType = eOldScannedType;
        else
            return FALSE;
    }

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

    if ( nMatchedAllStrings )
    {   // Match against format in any case, so later on for a "1-2-3-4" input
        // we may distinguish between a y-m-d (or similar) date and a 0-0-0-0
        // format.
        if ( ScanStringNumFor( rString, 0, pFormat, 0xFFFF ) )
            nMatchedAllStrings |= nMatchedEndString;
        else
            nMatchedAllStrings = 0;
    }

    SkipBlanks(rString, nPos);
    if (GetDecSep(rString, nPos))                   // decimal separator?
    {
        if (nDecPos == 1 || nDecPos == 3)           // .12.4 or 12.E4.
            return MatchedReturn();
        else if (nDecPos == 2)                      // . dup: 12.4.
        {
            if (bDecSepInDateSeps)                  // . also date sep
            {
                if (    eScannedType != NUMBERFORMAT_UNDEFINED &&
                        eScannedType != NUMBERFORMAT_DATE &&
                        eScannedType != NUMBERFORMAT_DATETIME)  // already another type
                    return MatchedReturn();
                if (eScannedType == NUMBERFORMAT_UNDEFINED)
                    eScannedType = NUMBERFORMAT_DATE;   // !!! it IS a date
                SkipBlanks(rString, nPos);
            }
            else
                return MatchedReturn();
        }
        else
        {
            nDecPos = 3;                            // . in end string
            SkipBlanks(rString, nPos);
        }
    }

    if (   nSign == 0                               // conflict - not signed
        && eScannedType != NUMBERFORMAT_DATE)       // and not date
//!? catch time too?
    {                                               // not signed yet
        nSign = GetSign(rString, nPos);             // 1- DM
        if (nNegCheck)                              // '(' as sign
            return MatchedReturn();
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
            return MatchedReturn();
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
                return MatchedReturn();
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
            return MatchedReturn();
        SkipBlanks(rString, nPos);
        eScannedType = NUMBERFORMAT_PERCENT;
    }

    const LocaleDataWrapper* pLoc = pFormatter->GetLocaleData();
    const String& rDate = pFormatter->GetDateSep();
    const String& rTime = pLoc->getTimeSep();
    if ( SkipString(rTime, rString, nPos) )         // 10:
    {
        if (nDecPos)                                // already , => error
            return MatchedReturn();
        if (eScannedType == NUMBERFORMAT_DATE && nAnzNums > 2) // 31.Dez.94 8:
        {
            SkipBlanks(rString, nPos);
            eScannedType = NUMBERFORMAT_DATETIME;
        }
        else if (eScannedType != NUMBERFORMAT_UNDEFINED &&
                 eScannedType != NUMBERFORMAT_TIME) // already another type
            return MatchedReturn();
        else
        {
            SkipBlanks(rString, nPos);
            eScannedType = NUMBERFORMAT_TIME;
        }
        if ( !nTimePos )
            nTimePos = nAnzStrings;
    }

    sal_Unicode cTime = rTime.GetChar(0);
    if (                      SkipString(rDate, rString, nPos)  // 10., 10-, 10/
        || ((cTime != '.') && SkipChar('.',   rString, nPos))   // TRICKY:
        || ((cTime != '/') && SkipChar('/',   rString, nPos))   // short boolean
        || ((cTime != '-') && SkipChar('-',   rString, nPos)) ) // evaluation!
    {
        if (eScannedType != NUMBERFORMAT_UNDEFINED &&
            eScannedType != NUMBERFORMAT_DATE)          // already another type
            return MatchedReturn();
        else
        {
            SkipBlanks(rString, nPos);
            eScannedType = NUMBERFORMAT_DATE;
        }
        short nTmpMonth = GetMonth(rString, nPos);  // 10. Jan
        if (nMonth && nTmpMonth)                    // month dup
            return MatchedReturn();
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
            return MatchedReturn();
        if (eScannedType != NUMBERFORMAT_UNDEFINED &&
            eScannedType != NUMBERFORMAT_DATE)      // already another type
            return MatchedReturn();
        eScannedType = NUMBERFORMAT_DATE;
        nMonth = nTempMonth;
        nMonthPos = 3;                              // month at end
        if ( nMonth < 0 )
            SkipChar( '.', rString, nPos );         // abbreviated
        SkipBlanks(rString, nPos);
    }

    xub_StrLen nOrigPos = nPos;
    if (GetTimeAmPm(rString, nPos))
    {
        if (eScannedType != NUMBERFORMAT_UNDEFINED &&
            eScannedType != NUMBERFORMAT_TIME &&
            eScannedType != NUMBERFORMAT_DATETIME)  // already another type
            return MatchedReturn();
        else
        {
            // If not already scanned as time, 6.78am does not result in 6
            // seconds and 78 hundredths in the morning. Keep as suffix.
            if (eScannedType != NUMBERFORMAT_TIME && nDecPos == 2 && nAnzNums == 2)
                nPos = nOrigPos;     // rewind am/pm
            else
            {
                SkipBlanks(rString, nPos);
                if ( eScannedType != NUMBERFORMAT_DATETIME )
                    eScannedType = NUMBERFORMAT_TIME;
            }
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
            return MatchedReturn();
    }

    if ( nPos < rString.Len() &&
            (eScannedType == NUMBERFORMAT_DATE
            || eScannedType == NUMBERFORMAT_DATETIME) )
    {   // day of week is just parsed away
        xub_StrLen nOldPos = nPos;
        const String& rSep = pFormatter->GetLocaleData()->getLongDateDayOfWeekSep();
        if ( StringContains( rSep, rString, nPos ) )
        {
            nPos = nPos + rSep.Len();
            SkipBlanks(rString, nPos);
        }
        int nDayOfWeek = GetDayOfWeek( rString, nPos );
        if ( nDayOfWeek )
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

#if NF_RECOGNIZE_ISO8601_TIMEZONES
    if (nPos == 0 && eScannedType == NUMBERFORMAT_DATETIME &&
            rString.Len() == 1 && rString.GetChar(0) == 'Z' && MayBeIso8601())
    {
        // ISO 8601 timezone UTC yyyy-mm-ddThh:mmZ
        ++nPos;
    }
#endif

    if (nPos < rString.Len())                       // everything consumed?
    {
        // does input EndString equal EndString in Format?
        if ( !ScanStringNumFor( rString, nPos, pFormat, 0xFFFF ) )
            return FALSE;
    }

    return TRUE;
}


BOOL ImpSvNumberInputScan::ScanStringNumFor(
        const String& rString,          // String to scan
        xub_StrLen nPos,                // Position until which was consumed
        const SvNumberformat* pFormat,  // The format to match
        USHORT nString,                 // Substring of format, 0xFFFF => last
        BOOL bDontDetectNegation        // Suppress sign detection
        )
{
    if ( !pFormat )
        return FALSE;
    const ::utl::TransliterationWrapper* pTransliteration = pFormatter->GetTransliteration();
    const String* pStr;
    String aString( rString );
    BOOL bFound = FALSE;
    BOOL bFirst = TRUE;
    BOOL bContinue = TRUE;
    USHORT nSub;
    do
    {
        // Don't try "lower" subformats ff the very first match was the second
        // or third subformat.
        nSub = nStringScanNumFor;
        do
        {   // Step through subformats, first positive, then negative, then
            // other, but not the last (text) subformat.
            pStr = pFormat->GetNumForString( nSub, nString, TRUE );
            if ( pStr && pTransliteration->isEqual( aString, *pStr ) )
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
        {   // try remaining substring
            bFirst = FALSE;
            aString.Erase( 0, nPos );
            bContinue = TRUE;
        }
    } while ( bContinue );

    if ( !bFound )
    {
        if ( !bDontDetectNegation && (nString == 0) && !bFirst && (nSign < 0)
                && pFormat->IsNegativeRealNegative() )
        {   // simply negated twice? --1
            aString.EraseAllChars( ' ' );
            if ( (aString.Len() == 1) && (aString.GetChar(0) == '-') )
            {
                bFound = TRUE;
                nStringScanSign = -1;
                nSub = 0;       //! not 1
            }
        }
        if ( !bFound )
            return FALSE;
    }
    else if ( !bDontDetectNegation && (nSub == 1) &&
            pFormat->IsNegativeRealNegative() )
    {   // negative
        if ( nStringScanSign < 0 )
        {
            if ( (nSign < 0) && (nStringScanNumFor != 1) )
                nStringScanSign = 1;        // triple negated --1 yyy
        }
        else if ( nStringScanSign == 0 )
        {
            if ( nSign < 0 )
            {   // nSign and nStringScanSign will be combined later,
                // flip sign if doubly negated
                if ( (nString == 0) && !bFirst
                        && SvNumberformat::HasStringNegativeSign( aString ) )
                    nStringScanSign = -1;   // direct double negation
                else if ( pFormat->IsNegativeWithoutSign() )
                    nStringScanSign = -1;   // indirect double negation
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
// Recognizes types of number, exponential, fraction, percent, currency, date, time.
// Else text => return FALSE

BOOL ImpSvNumberInputScan::IsNumberFormatMain(
        const String& rString,                  // string to be analyzed
        double& ,                               // OUT: result as number, if possible
        const SvNumberformat* pFormat )         // maybe number format set to match against
{
    Reset();
    NumberStringDivision( rString );            // breakdown into strings and numbers
    if (nAnzStrings >= SV_MAX_ANZ_INPUT_STRINGS) // too many elements
        return FALSE;                           // Njet, Nope, ...

    if (nAnzNums == 0)                          // no number in input
    {
        if ( nAnzStrings > 0 )
        {
            // Here we may change the original, we don't need it anymore.
            // This saves copies and ToUpper() in GetLogical() and is faster.
            String& rStrArray = sStrArray[0];
            rStrArray.EraseTrailingChars( ' ' );
            rStrArray.EraseLeadingChars( ' ' );
            nLogical = GetLogical( rStrArray );
            if ( nLogical )
            {
                eScannedType = NUMBERFORMAT_LOGICAL; // !!! it's a BOOLEAN
                nMatchedAllStrings &= ~nMatchedVirgin;
                return TRUE;
            }
            else
                return FALSE;                   // simple text
        }
        else
            return FALSE;                       // simple text
    }

    USHORT i = 0;                               // mark any symbol
    USHORT j = 0;                               // mark only numbers

    switch ( nAnzNums )
    {
        case 1 :                                // Exactly 1 number in input
        {                                       // nAnzStrings >= 1
            if (GetNextNumber(i,j))             // i=1,0
            {                                   // Number at start
                if (eSetType == NUMBERFORMAT_FRACTION)  // Fraction 1 = 1/1
                {
                    if (i >= nAnzStrings ||     // no end string nor decimal separator
                        sStrArray[i] == pFormatter->GetNumDecimalSep())
                    {
                        eScannedType = NUMBERFORMAT_FRACTION;
                        nMatchedAllStrings &= ~nMatchedVirgin;
                        return TRUE;
                    }
                }
            }
            else
            {                                   // Analyze start string
                if (!ScanStartString( sStrArray[i], pFormat ))  // i=0
                    return FALSE;               // already an error
                i++;                            // next symbol, i=1
            }
            GetNextNumber(i,j);                 // i=1,2
            if (eSetType == NUMBERFORMAT_FRACTION)  // Fraction -1 = -1/1
            {
                if (nSign && !nNegCheck &&      // Sign +, -
                    eScannedType == NUMBERFORMAT_UNDEFINED &&   // not date or currency
                    nDecPos == 0 &&             // no previous decimal separator
                    (i >= nAnzStrings ||        // no end string nor decimal separator
                        sStrArray[i] == pFormatter->GetNumDecimalSep())
                )
                {
                    eScannedType = NUMBERFORMAT_FRACTION;
                    nMatchedAllStrings &= ~nMatchedVirgin;
                    return TRUE;
                }
            }
            if (i < nAnzStrings && !ScanEndString( sStrArray[i], pFormat ))
                    return FALSE;
        }
        break;
        case 2 :                                // Exactly 2 numbers in input
        {                                       // nAnzStrings >= 3
            if (!GetNextNumber(i,j))            // i=1,0
            {                                   // Analyze start string
                if (!ScanStartString( sStrArray[i], pFormat ))
                    return FALSE;               // already an error
                i++;                            // i=1
            }
            GetNextNumber(i,j);                 // i=1,2
            if ( !ScanMidString( sStrArray[i], i, pFormat ) )
                return FALSE;
            i++;                                // next symbol, i=2,3
            GetNextNumber(i,j);                 // i=3,4
            if (i < nAnzStrings && !ScanEndString( sStrArray[i], pFormat ))
                return FALSE;
            if (eSetType == NUMBERFORMAT_FRACTION)  // -1,200. as fraction
            {
                if (!nNegCheck  &&                  // no sign '('
                    eScannedType == NUMBERFORMAT_UNDEFINED &&
                    (nDecPos == 0 || nDecPos == 3)  // no decimal separator or at end
                    )
                {
                    eScannedType = NUMBERFORMAT_FRACTION;
                    nMatchedAllStrings &= ~nMatchedVirgin;
                    return TRUE;
                }
            }
        }
        break;
        case 3 :                                // Exactly 3 numbers in input
        {                                       // nAnzStrings >= 5
            if (!GetNextNumber(i,j))            // i=1,0
            {                                   // Analyze start string
                if (!ScanStartString( sStrArray[i], pFormat ))
                    return FALSE;               // already an error
                i++;                            // i=1
                if (nDecPos == 1)               // decimal separator at start => error
                    return FALSE;
            }
            GetNextNumber(i,j);                 // i=1,2
            if ( !ScanMidString( sStrArray[i], i, pFormat ) )
                return FALSE;
            i++;                                // i=2,3
            if (eScannedType == NUMBERFORMAT_SCIENTIFIC)    // E only at end
                return FALSE;
            GetNextNumber(i,j);                 // i=3,4
            if ( !ScanMidString( sStrArray[i], i, pFormat ) )
                return FALSE;
            i++;                                // i=4,5
            GetNextNumber(i,j);                 // i=5,6
            if (i < nAnzStrings && !ScanEndString( sStrArray[i], pFormat ))
                return FALSE;
            if (eSetType == NUMBERFORMAT_FRACTION)  // -1,200,100. as fraction
            {
                if (!nNegCheck  &&                  // no sign '('
                    eScannedType == NUMBERFORMAT_UNDEFINED &&
                    (nDecPos == 0 || nDecPos == 3)  // no decimal separator or at end
                )
                {
                    eScannedType = NUMBERFORMAT_FRACTION;
                    nMatchedAllStrings &= ~nMatchedVirgin;
                    return TRUE;
                }
            }
            if ( eScannedType == NUMBERFORMAT_FRACTION && nDecPos )
                return FALSE;                   // #36857# not a real fraction
        }
        break;
        default:                                // More than 3 numbers in input
        {                                       // nAnzStrings >= 7
            if (!GetNextNumber(i,j))            // i=1,0
            {                                   // Analyze startstring
                if (!ScanStartString( sStrArray[i], pFormat ))
                    return FALSE;               // already an error
                i++;                            // i=1
                if (nDecPos == 1)               // decimal separator at start => error
                    return FALSE;
            }
            GetNextNumber(i,j);                 // i=1,2
            if ( !ScanMidString( sStrArray[i], i, pFormat ) )
                return FALSE;
            i++;                                // i=2,3
            USHORT nThOld = 10;                 // just not 0 or 1
            while (nThOld != nThousand && j < nAnzNums-1)
                                                // Execute at least one time
                                                // but leave one number.
            {                                   // Loop over group separators
                nThOld = nThousand;
                if (eScannedType == NUMBERFORMAT_SCIENTIFIC)    // E only at end
                    return FALSE;
                GetNextNumber(i,j);
                if ( i < nAnzStrings && !ScanMidString( sStrArray[i], i, pFormat ) )
                    return FALSE;
                i++;
            }
            if (eScannedType == NUMBERFORMAT_DATE ||    // long date or
                eScannedType == NUMBERFORMAT_TIME ||    // long time or
                eScannedType == NUMBERFORMAT_UNDEFINED) // long number
            {
                for (USHORT k = j; k < nAnzNums-1; k++)
                {
                    if (eScannedType == NUMBERFORMAT_SCIENTIFIC)    // E only at endd
                        return FALSE;
                    GetNextNumber(i,j);
                    if ( i < nAnzStrings && !ScanMidString( sStrArray[i], i, pFormat ) )
                        return FALSE;
                    i++;
                }
            }
            GetNextNumber(i,j);
            if (i < nAnzStrings && !ScanEndString( sStrArray[i], pFormat ))
                return FALSE;
            if (eSetType == NUMBERFORMAT_FRACTION)  // -1,200,100. as fraction
            {
                if (!nNegCheck  &&                  // no sign '('
                    eScannedType == NUMBERFORMAT_UNDEFINED &&
                    (nDecPos == 0 || nDecPos == 3)  // no decimal separator or at end
                )
                {
                    eScannedType = NUMBERFORMAT_FRACTION;
                    nMatchedAllStrings &= ~nMatchedVirgin;
                    return TRUE;
                }
            }
            if ( eScannedType == NUMBERFORMAT_FRACTION && nDecPos )
                return FALSE;                       // #36857# not a real fraction
        }
    }

    if (eScannedType == NUMBERFORMAT_UNDEFINED)
    {
        nMatchedAllStrings &= ~nMatchedVirgin;
        // did match including nMatchedUsedAsReturn
        BOOL bDidMatch = (nMatchedAllStrings != 0);
        if ( nMatchedAllStrings )
        {
            BOOL bMatch = (pFormat ? pFormat->IsNumForStringElementCountEqual(
                        nStringScanNumFor, nAnzStrings, nAnzNums ) : FALSE);
            if ( !bMatch )
                nMatchedAllStrings = 0;
        }
        if ( nMatchedAllStrings )
            eScannedType = eSetType;
        else if ( bDidMatch )
            return FALSE;
        else
            eScannedType = NUMBERFORMAT_NUMBER;
            // everything else should have been recognized by now
    }
    else if ( eScannedType == NUMBERFORMAT_DATE )
    {   // the very relaxed date input checks may interfere with a preset format
        nMatchedAllStrings &= ~nMatchedVirgin;
        BOOL bWasReturn = ((nMatchedAllStrings & nMatchedUsedAsReturn) != 0);
        if ( nMatchedAllStrings )
        {
            BOOL bMatch = (pFormat ? pFormat->IsNumForStringElementCountEqual(
                        nStringScanNumFor, nAnzStrings, nAnzNums ) : FALSE);
            if ( !bMatch )
                nMatchedAllStrings = 0;
        }
        if ( nMatchedAllStrings )
            eScannedType = eSetType;
        else if ( bWasReturn )
            return FALSE;
    }
    else
        nMatchedAllStrings = 0;  // reset flag to no substrings matched

    return TRUE;
}


//---------------------------------------------------------------------------
// return TRUE or FALSE depending on the nMatched... state and remember usage
BOOL ImpSvNumberInputScan::MatchedReturn()
{
    if ( nMatchedAllStrings & ~nMatchedVirgin )
    {
        nMatchedAllStrings |= nMatchedUsedAsReturn;
        return TRUE;
    }
    return FALSE;
}


//---------------------------------------------------------------------------
// Initialize uppercase months and weekdays

void ImpSvNumberInputScan::InitText()
{
    sal_Int32 j, nElems;
    const CharClass* pChrCls = pFormatter->GetCharClass();
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
    sal_Unicode cDecSep = pFormatter->GetNumDecimalSep().GetChar(0);
    bDecSepInDateSeps = ( cDecSep == '-' ||
                          cDecSep == '/' ||
                          cDecSep == '.' ||
                          cDecSep == pFormatter->GetDateSep().GetChar(0) );
    bTextInitialized = FALSE;
    aUpperCurrSymbol.Erase();
}


//---------------------------------------------------------------------------
//      ChangeNullDate

void ImpSvNumberInputScan::ChangeNullDate(
        const USHORT Day,
        const USHORT Month,
        const USHORT Year )
{
    if ( pNullDate )
        *pNullDate = Date(Day, Month, Year);
    else
        pNullDate = new Date(Day, Month, Year);
}


//---------------------------------------------------------------------------
//      IsNumberFormat
//
// => does rString represent a number (also date, time et al)

BOOL ImpSvNumberInputScan::IsNumberFormat(
        const String& rString,                  // string to be analyzed
        short& F_Type,                          // IN: old type, OUT: new type
        double& fOutNumber,                     // OUT: number if convertable
        const SvNumberformat* pFormat )         // maybe a number format to match against
{
    String sResString;
    String aString;
    BOOL res;                                   // return value
    eSetType = F_Type;                          // old type set

    if ( !rString.Len() )
        res = FALSE;
    else if (rString.Len() > 308)               // arbitrary
        res = FALSE;
    else
    {
        // NoMoreUpperNeeded, all comparisons on UpperCase
        aString = pFormatter->GetCharClass()->upper( rString );
        // convert native number to ASCII if necessary
        TransformInput( aString );
        res = IsNumberFormatMain( aString, fOutNumber, pFormat );
    }

    if (res)
    {
        if ( nNegCheck                              // ')' not found for '('
                || (nSign && (eScannedType == NUMBERFORMAT_DATE
                    || eScannedType == NUMBERFORMAT_DATETIME))
            )                                       // signed date/datetime
            res = FALSE;
        else
        {                                           // check count of partial number strings
            switch (eScannedType)
            {
                case NUMBERFORMAT_PERCENT:
                case NUMBERFORMAT_CURRENCY:
                case NUMBERFORMAT_NUMBER:
                    if (nDecPos == 1)               // .05
                    {
                        // matched MidStrings function like group separators
                        if ( nMatchedAllStrings )
                            nThousand = nAnzNums - 1;
                        else if ( nAnzNums != 1 )
                            res = FALSE;
                    }
                    else if (nDecPos == 2)          // 1.05
                    {
                        // matched MidStrings function like group separators
                        if ( nMatchedAllStrings )
                            nThousand = nAnzNums - 1;
                        else if ( nAnzNums != nThousand+2 )
                            res = FALSE;
                    }
                    else                            // 1,100 or 1,100.
                    {
                        // matched MidStrings function like group separators
                        if ( nMatchedAllStrings )
                            nThousand = nAnzNums - 1;
                        else if ( nAnzNums != nThousand+1 )
                            res = FALSE;
                    }
                    break;

                case NUMBERFORMAT_SCIENTIFIC:       // 1.0e-2
                    if (nDecPos == 1)               // .05
                    {
                        if (nAnzNums != 2)
                            res = FALSE;
                    }
                    else if (nDecPos == 2)          // 1.05
                    {
                        if (nAnzNums != nThousand+3)
                            res = FALSE;
                    }
                    else                            // 1,100 or 1,100.
                    {
                        if (nAnzNums != nThousand+2)
                            res = FALSE;
                    }
                    break;

                case NUMBERFORMAT_DATE:
                    if (nMonth)
                    {                               // month name and numbers
                        if (nAnzNums > 2)
                            res = FALSE;
                    }
                    else
                    {
                        if (nAnzNums > 3)
                            res = FALSE;
                    }
                    break;

                case NUMBERFORMAT_TIME:
                    if (nDecPos)
                    {                               // hundredth seconds included
                        if (nAnzNums > 4)
                            res = FALSE;
                    }
                    else
                    {
                        if (nAnzNums > 3)
                            res = FALSE;
                    }
                    break;

                case NUMBERFORMAT_DATETIME:
                    if (nMonth)
                    {                               // month name and numbers
                        if (nDecPos)
                        {                           // hundredth seconds included
                            if (nAnzNums > 6)
                                res = FALSE;
                        }
                        else
                        {
                            if (nAnzNums > 5)
                                res = FALSE;
                        }
                    }
                    else
                    {
                        if (nDecPos)
                        {                           // hundredth seconds included
                            if (nAnzNums > 7)
                                res = FALSE;
                        }
                        else
                        {
                            if (nAnzNums > 6)
                                res = FALSE;
                        }
                    }
                    break;

                default:
                    break;
            }   // switch
        }   // else
    }   // if (res)

    if (res)
    {                                           // we finally have a number
        switch (eScannedType)
        {
            case NUMBERFORMAT_LOGICAL:
                if      (nLogical ==  1)
                    fOutNumber = 1.0;           // True
                else if (nLogical == -1)
                    fOutNumber = 0.0;           // False
                else
                    res = FALSE;                // Oops
                break;

            case NUMBERFORMAT_PERCENT:
            case NUMBERFORMAT_CURRENCY:
            case NUMBERFORMAT_NUMBER:
            case NUMBERFORMAT_SCIENTIFIC:
            case NUMBERFORMAT_DEFINED:          // if no category detected handle as number
            {
                if ( nDecPos == 1 )                         // . at start
                    sResString.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "0." ) );
                else
                    sResString.Erase();
                USHORT k;
                for ( k = 0; k <= nThousand; k++)
                    sResString += sStrArray[nNums[k]];  // integer part
                if ( nDecPos == 2 && k < nAnzNums )     // . somewhere
                {
                    sResString += '.';
                    USHORT nStop = (eScannedType == NUMBERFORMAT_SCIENTIFIC ?
                            nAnzNums-1 : nAnzNums);
                    for ( ; k < nStop; k++)
                        sResString += sStrArray[nNums[k]];  // fractional part
                }

                if (eScannedType != NUMBERFORMAT_SCIENTIFIC)
                    fOutNumber = StringToDouble(sResString);
                else
                {                                           // append exponent
                    sResString += 'E';
                    if ( nESign == -1 )
                        sResString += '-';
                    sResString += sStrArray[nNums[nAnzNums-1]];
                    rtl_math_ConversionStatus eStatus;
                    fOutNumber = ::rtl::math::stringToDouble(
                        sResString, '.', ',', &eStatus, NULL );
                    if ( eStatus == rtl_math_ConversionStatus_OutOfRange )
                    {
                        F_Type = NUMBERFORMAT_TEXT;         // overflow/underflow -> Text
                        if (nESign == -1)
                            fOutNumber = 0.0;
                        else
                            fOutNumber = DBL_MAX;
/*!*/                   return TRUE;
                    }
                }

                if ( nStringScanSign )
                {
                    if ( nSign )
                        nSign *= nStringScanSign;
                    else
                        nSign = nStringScanSign;
                }
                if ( nSign < 0 )
                    fOutNumber = -fOutNumber;

                if (eScannedType == NUMBERFORMAT_PERCENT)
                    fOutNumber/= 100.0;
            }
            break;

            case NUMBERFORMAT_FRACTION:
                if (nAnzNums == 1)
                    fOutNumber = StringToDouble(sStrArray[nNums[0]]);
                else if (nAnzNums == 2)
                {
                    if (nThousand == 1)
                    {
                        sResString = sStrArray[nNums[0]];
                        sResString += sStrArray[nNums[1]];  // integer part
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
                if ( nSign < 0 )
                    fOutNumber = -fOutNumber;
                break;

            case NUMBERFORMAT_TIME:
                GetTimeRef(fOutNumber, 0, nAnzNums);
                if ( nSign < 0 )
                    fOutNumber = -fOutNumber;
                break;

            case NUMBERFORMAT_DATE:
            {
                USHORT nCounter = 0;                        // dummy here
                res = GetDateRef( fOutNumber, nCounter, pFormat );
            }
            break;

            case NUMBERFORMAT_DATETIME:
            {
                USHORT nCounter = 0;                        // needed here
                res = GetDateRef( fOutNumber, nCounter, pFormat );
                if ( res )
                {
                    double fTime;
                    GetTimeRef( fTime, nCounter, nAnzNums - nCounter );
                    fOutNumber += fTime;
                }
            }
            break;

            default:
                DBG_ERRORFILE( "Some number recognized but what's it?" );
                fOutNumber = 0.0;
                break;
        }
    }

    if (res)        // overflow/underflow -> Text
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



}
