/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include <ctype.h>
#include <stdlib.h>
#include <float.h>
#include <errno.h>
#include <comphelper/string.hxx>
#include <tools/date.hxx>
#include <tools/debug.hxx>
#include <rtl/math.hxx>
#include <unotools/charclass.hxx>
#include <unotools/calendarwrapper.hxx>
#include <unotools/localedatawrapper.hxx>
#include <com/sun/star/i18n/CalendarFieldIndex.hpp>
#include <com/sun/star/i18n/LocaleCalendar.hpp>
#include <unotools/digitgroupingiterator.hxx>

#include <svl/zforlist.hxx>
#include "zforscan.hxx"
#include <svl/zformat.hxx>

#include "zforfind.hxx"

#ifndef DBG_UTIL
#define NF_TEST_CALENDAR 0
#else
#define NF_TEST_CALENDAR 0
#endif
#if NF_TEST_CALENDAR
#include <comphelper/processfactory.hxx>
#include <com/sun/star/i18n/XCalendar3.hpp>
#endif


const sal_uInt8 ImpSvNumberInputScan::nMatchedEndString    = 0x01;
const sal_uInt8 ImpSvNumberInputScan::nMatchedMidString    = 0x02;
const sal_uInt8 ImpSvNumberInputScan::nMatchedStartString  = 0x04;
const sal_uInt8 ImpSvNumberInputScan::nMatchedVirgin       = 0x08;
const sal_uInt8 ImpSvNumberInputScan::nMatchedUsedAsReturn = 0x10;

/* It is not clear how we want timezones to be handled. Convert them to local
 * time isn't wanted, as it isn't done in any other place and timezone
 * information isn't stored anywhere. Ignoring them and pretending local time
 * may be wrong too and might not be what the user expects. Keep the input as
 * string so that no information is lost.
 * Anyway, defining NF_RECOGNIZE_ISO8601_TIMEZONES to 1 would be the way how it
 * would work, together with the nTimezonePos handling in GetTimeRef(). */
#define NF_RECOGNIZE_ISO8601_TIMEZONES 0

static const sal_Unicode cNonBreakingSpace = 0xA0;

ImpSvNumberInputScan::ImpSvNumberInputScan( SvNumberFormatter* pFormatterP )
        :
        pUpperMonthText( NULL ),
        pUpperAbbrevMonthText( NULL ),
        pUpperGenitiveMonthText( NULL ),
        pUpperGenitiveAbbrevMonthText( NULL ),
        pUpperPartitiveMonthText( NULL ),
        pUpperPartitiveAbbrevMonthText( NULL ),
        pUpperDayText( NULL ),
        pUpperAbbrevDayText( NULL ),
        bTextInitialized( false ),
        bScanGenitiveMonths( false ),
        bScanPartitiveMonths( false ),
        eScannedType( NUMBERFORMAT_UNDEFINED ),
        eSetType( NUMBERFORMAT_UNDEFINED )
{
    pFormatter = pFormatterP;
    pNullDate = new Date(30,12,1899);
    nYear2000 = SvNumberFormatter::GetYear2000Default();
    Reset();
    ChangeIntl();
}


ImpSvNumberInputScan::~ImpSvNumberInputScan()
{
    Reset();
    delete pNullDate;
    delete [] pUpperMonthText;
    delete [] pUpperAbbrevMonthText;
    delete [] pUpperGenitiveMonthText;
    delete [] pUpperGenitiveAbbrevMonthText;
    delete [] pUpperPartitiveMonthText;
    delete [] pUpperPartitiveAbbrevMonthText;
    delete [] pUpperDayText;
    delete [] pUpperAbbrevDayText;
}


void ImpSvNumberInputScan::Reset()
{
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
    nMayBeMonthDate = 0;
    nAcceptedDatePattern = -2;
    nDatePatternStart = 0;
    nDatePatternNumbers = 0;
    nCanForceToIso8601 = 0;

    for (sal_uInt32 i = 0; i < SV_MAX_ANZ_INPUT_STRINGS; i++)
    {
        IsNum[i] = false;
        nNums[i] = 0;
    }
}



inline bool ImpSvNumberInputScan::MyIsdigit( sal_Unicode c )
{
    return c < 128 && isdigit( (unsigned char) c );
}


void TransformInput( SvNumberFormatter* pFormatter, OUString& rStr )
{
    sal_Int32 nPos, nLen;
    for ( nPos = 0, nLen = rStr.getLength(); nPos < nLen; ++nPos )
    {
        if ( 256 <= rStr[ nPos ] &&
             pFormatter->GetCharClass()->isDigit( rStr, nPos ) )
        {
            break;
        }
    }
    if ( nPos < nLen )
    {
        rStr = pFormatter->GetNatNum()->getNativeNumberString( rStr,
                                                               pFormatter->GetLanguageTag().getLocale(), 0 );
    }
}


/**
 * Only simple unsigned floating point values without any error detection,
 * decimal separator has to be '.'
 */
double ImpSvNumberInputScan::StringToDouble( const OUString& rStr, bool bForceFraction )
{
    double fNum = 0.0;
    double fFrac = 0.0;
    int nExp = 0;
    sal_Int32 nPos = 0;
    sal_Int32 nLen = rStr.getLength();
    bool bPreSep = !bForceFraction;

    while (nPos < nLen)
    {
        if (rStr[nPos] == '.')
        {
            bPreSep = false;
        }
        else if (bPreSep)
        {
            fNum = fNum * 10.0 + (double) (rStr[nPos] - '0');
        }
        else
        {
            fFrac = fFrac * 10.0 + (double) (rStr[nPos] - '0');
            --nExp;
        }
        nPos++;
    }
    if ( fFrac )
    {
        return fNum + ::rtl::math::pow10Exp( fFrac, nExp );
    }
    return fNum;
}


/**
 * Splits up the input into numbers and strings for further processing
 * (by the Turing machine).
 *
 * Starting state = GetChar
 * ---------------+-------------------+-----------------------------+---------------
 *  Old State     | Character read    | Event                       | New state
 * ---------------+-------------------+-----------------------------+---------------
 *  GetChar       | Number            | Symbol = Character          | GetValue
 *                | Else              | Symbol = Character          | GetString
 * ---------------|-------------------+-----------------------------+---------------
 *  GetValue      | Number            | Symbol = Symbol + Character | GetValue
 *                | Else              | Dec(CharPos)                | Stop
 * ---------------+-------------------+-----------------------------+---------------
 *  GetString     | Number            | Dec(CharPos)                | Stop
 *                | Else              | Symbol = Symbol + Character | GetString
 * ---------------+-------------------+-----------------------------+---------------
 */
enum ScanState  
{
    SsStop      = 0,
    SsStart     = 1,
    SsGetValue  = 2,
    SsGetString = 3
};

bool ImpSvNumberInputScan::NextNumberStringSymbol( const sal_Unicode*& pStr,
                                                   OUString& rSymbol )
{
    bool isNumber = false;
    sal_Unicode cToken;
    ScanState eState = SsStart;
    const sal_Unicode* pHere = pStr;
    sal_Int32 nChars = 0;

    while ( ((cToken = *pHere) != 0) && eState != SsStop)
    {
        pHere++;
        switch (eState)
        {
        case SsStart:
            if ( MyIsdigit( cToken ) )
            {
                eState = SsGetValue;
                isNumber = true;
            }
            else
            {
                eState = SsGetString;
            }
            nChars++;
            break;
        case SsGetValue:
            if ( MyIsdigit( cToken ) )
            {
                nChars++;
            }
            else
            {
                eState = SsStop;
                pHere--;
            }
            break;
        case SsGetString:
            if ( !MyIsdigit( cToken ) )
            {
                nChars++;
            }
            else
            {
                eState = SsStop;
                pHere--;
            }
            break;
        default:
            break;
        } 
    } 

    if ( nChars )
    {
        rSymbol = OUString( pStr, nChars );
    }
    else
    {
        rSymbol = "";
    }

    pStr = pHere;

    return isNumber;
}





bool ImpSvNumberInputScan::SkipThousands( const sal_Unicode*& pStr,
                                          OUString& rSymbol )
{
    bool res = false;
    OUStringBuffer sBuff(rSymbol);
    sal_Unicode cToken;
    const OUString& rThSep = pFormatter->GetNumThousandSep();
    const sal_Unicode* pHere = pStr;
    ScanState eState = SsStart;
    sal_Int32 nCounter = 0; 

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
                pHere += rThSep.getLength() - 1;
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
                sBuff.append(cToken);
                nCounter++;
                if (nCounter == 3)
                {
                    eState = SsStart;
                    res = true; 
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
        } 
    } 

    if (eState == SsGetValue) 
    {
        if ( nCounter )
        {
            sBuff.remove( sBuff.getLength() - nCounter, nCounter );
        }
        pHere -= nCounter + rThSep.getLength(); 
    }
    rSymbol = sBuff.makeStringAndClear();
    pStr = pHere;

    return res;
}


void ImpSvNumberInputScan::NumberStringDivision( const OUString& rString )
{
    const sal_Unicode* pStr = rString.getStr();
    const sal_Unicode* const pEnd = pStr + rString.getLength();
    while ( pStr < pEnd && nAnzStrings < SV_MAX_ANZ_INPUT_STRINGS )
    {
        if ( NextNumberStringSymbol( pStr, sStrArray[nAnzStrings] ) )
        {   
            IsNum[nAnzStrings] = true;
            nNums[nAnzNums] = nAnzStrings;
            nAnzNums++;
            if (nAnzStrings >= SV_MAX_ANZ_INPUT_STRINGS - 7 &&
                nPosThousandString == 0) 
            {
                if ( SkipThousands( pStr, sStrArray[nAnzStrings] ) )
                {
                    nPosThousandString = nAnzStrings;
                }
            }
        }
        else
        {
            IsNum[nAnzStrings] = false;
        }
        nAnzStrings++;
    }
}


/**
 * Whether rString contains rWhat at nPos
 */
bool ImpSvNumberInputScan::StringContainsImpl( const OUString& rWhat,
                                               const OUString& rString, sal_Int32 nPos )
{
    if ( nPos + rWhat.getLength() <= rString.getLength() )
    {
        return StringPtrContainsImpl( rWhat, rString.getStr(), nPos );
    }
    return false;
}


/**
 * Whether pString contains rWhat at nPos
 */
bool ImpSvNumberInputScan::StringPtrContainsImpl( const OUString& rWhat,
                                                  const sal_Unicode* pString, sal_Int32 nPos )
{
    if ( rWhat.isEmpty() )
    {
        return false;
    }
    const sal_Unicode* pWhat = rWhat.getStr();
    const sal_Unicode* const pEnd = pWhat + rWhat.getLength();
    const sal_Unicode* pStr = pString + nPos;
    while ( pWhat < pEnd )
    {
        if ( *pWhat != *pStr )
        {
            return false;
        }
        pWhat++;
        pStr++;
    }
    return true;
}


/**
 * Skips the supplied char
 */
inline bool ImpSvNumberInputScan::SkipChar( sal_Unicode c, const OUString& rString,
                                            sal_Int32& nPos )
{
    if ((nPos < rString.getLength()) && (rString[nPos] == c))
    {
        nPos++;
        return true;
    }
    return false;
}


/**
 * Skips blanks
 */
inline void ImpSvNumberInputScan::SkipBlanks( const OUString& rString,
                                              sal_Int32& nPos )
{
    if ( nPos < rString.getLength() )
    {
        const sal_Unicode* p = rString.getStr() + nPos;
        while ( *p == ' ' || *p == cNonBreakingSpace )
        {
            nPos++;
            p++;
        }
    }
}


/**
 * jump over rWhat in rString at nPos
 */
inline bool ImpSvNumberInputScan::SkipString( const OUString& rWhat,
                                              const OUString& rString, sal_Int32& nPos )
{
    if ( StringContains( rWhat, rString, nPos ) )
    {
        nPos = nPos + rWhat.getLength();
        return true;
    }
    return false;
}


/**
 * Recognizes exactly ,111 in {3} and {3,2} or ,11 in {3,2} grouping
 */
inline bool ImpSvNumberInputScan::GetThousandSep( const OUString& rString,
                                                  sal_Int32& nPos,
                                                  sal_uInt16 nStringPos )
{
    const OUString& rSep = pFormatter->GetNumThousandSep();
    
    bool bSpaceBreak = rSep[0] == (sal_Unicode)0xa0 && rString[0] == (sal_Unicode)0x20 &&
        rSep.getLength() == 1 && rString.getLength() == 1;
    if (!((rString == rSep || bSpaceBreak) &&      
           nStringPos < nAnzStrings - 1 &&         
           IsNum[ nStringPos + 1 ] ))              
    {
        return false; 
    }

    utl::DigitGroupingIterator aGrouping( pFormatter->GetLocaleData()->getDigitGrouping());
    
    /* FIXME: this could be refined to match ,## in {3,2} only if ,##,## or
     * ,##,### and to match ,### in {3,2} only if it's the last. However,
     * currently there is no track kept where group separators occur. In {3,2}
     * #,###,### and #,##,## would be valid input, which maybe isn't even bad
     * for #,###,###. Other combinations such as #,###,## maybe not. */
    sal_Int32 nLen = sStrArray[ nStringPos + 1 ].getLength();
    if (nLen == aGrouping.get() ||                  
        nLen == aGrouping.advance().get() ||        
        nPosThousandString == nStringPos + 1 )      
    {
        nPos = nPos + rSep.getLength();
        return true;
    }
    return false;
}


/**
 * Conversion of text to logical value
 *  "true" =>  1:
 *  "false"=> -1:
 *  else   =>  0:
 */
short ImpSvNumberInputScan::GetLogical( const OUString& rString )
{
    short res;

    const ImpSvNumberformatScan* pFS = pFormatter->GetFormatScanner();
    if ( rString == pFS->GetTrueString() )
    {
        res = 1;
    }
    else if ( rString == pFS->GetFalseString() )
    {
        res = -1;
    }
    else
    {
        res = 0;
    }
    return res;
}


/**
 * Converts a string containing a month name (JAN, January) at nPos into the
 * month number (negative if abbreviated), returns 0 if nothing found
 */
short ImpSvNumberInputScan::GetMonth( const OUString& rString, sal_Int32& nPos )
{
    
    
    static const OUString aSeptCorrect("SEPT");
    static const OUString aSepShortened("SEP");

    short res = 0; 

    if (rString.getLength() > nPos) 
    {
        if ( !bTextInitialized )
        {
            InitText();
        }
        sal_Int16 nMonths = pFormatter->GetCalendar()->getNumberOfMonthsInYear();
        for ( sal_Int16 i = 0; i < nMonths; i++ )
        {
            if ( bScanGenitiveMonths && StringContains( pUpperGenitiveMonthText[i], rString, nPos ) )
            {   
                nPos = nPos + pUpperGenitiveMonthText[i].getLength();
                res = i + 1;
                break;  
            }
            else if ( bScanGenitiveMonths && StringContains( pUpperGenitiveAbbrevMonthText[i], rString, nPos ) )
            {   
                nPos = nPos + pUpperGenitiveAbbrevMonthText[i].getLength();
                res = sal::static_int_cast< short >(-(i+1)); 
                break;  
            }
            else if ( bScanPartitiveMonths && StringContains( pUpperPartitiveMonthText[i], rString, nPos ) )
            {   
                nPos = nPos + pUpperPartitiveMonthText[i].getLength();
                res = i+1;
                break;  
            }
            else if ( bScanPartitiveMonths && StringContains( pUpperPartitiveAbbrevMonthText[i], rString, nPos ) )
            {   
                nPos = nPos + pUpperPartitiveAbbrevMonthText[i].getLength();
                res = sal::static_int_cast< short >(-(i+1)); 
                break;  
            }
            else if ( StringContains( pUpperMonthText[i], rString, nPos ) )
            {   
                nPos = nPos + pUpperMonthText[i].getLength();
                res = i+1;
                break;  
            }
            else if ( StringContains( pUpperAbbrevMonthText[i], rString, nPos ) )
            {   
                nPos = nPos + pUpperAbbrevMonthText[i].getLength();
                res = sal::static_int_cast< short >(-(i+1)); 
                break;  
            }
            else if ( i == 8 && pUpperAbbrevMonthText[i] == aSeptCorrect &&
                    StringContains( aSepShortened, rString, nPos ) )
            {   
                nPos = nPos + aSepShortened.getLength();
                res = sal::static_int_cast< short >(-(i+1)); 
                break;  
            }
        }
    }

    return res;
}


/**
 * Converts a string containing a DayOfWeek name (Mon, Monday) at nPos into the
 * DayOfWeek number + 1 (negative if abbreviated), returns 0 if nothing found
 */
int ImpSvNumberInputScan::GetDayOfWeek( const OUString& rString, sal_Int32& nPos )
{
    int res = 0; 

    if (rString.getLength() > nPos) 
    {
        if ( !bTextInitialized )
        {
            InitText();
        }
        sal_Int16 nDays = pFormatter->GetCalendar()->getNumberOfDaysInWeek();
        for ( sal_Int16 i = 0; i < nDays; i++ )
        {
            if ( StringContains( pUpperDayText[i], rString, nPos ) )
            {   
                nPos = nPos + pUpperDayText[i].getLength();
                res = i + 1;
                break;  
            }
            if ( StringContains( pUpperAbbrevDayText[i], rString, nPos ) )
            {   
                nPos = nPos + pUpperAbbrevDayText[i].getLength();
                res = -(i + 1); 
                break;  
            }
        }
    }

    return res;
}


/**
 * Reading a currency symbol
 * '$'   => true
 * else => false
 */
bool ImpSvNumberInputScan::GetCurrency( const OUString& rString, sal_Int32& nPos,
                                        const SvNumberformat* pFormat )
{
    if ( rString.getLength() > nPos )
    {
        if ( !aUpperCurrSymbol.getLength() )
        {   
            LanguageType eLang = (pFormat ? pFormat->GetLanguage() : pFormatter->GetLanguage());
            aUpperCurrSymbol = pFormatter->GetCharClass()->uppercase(
                SvNumberFormatter::GetCurrencyEntry( eLang ).GetSymbol() );
        }
        if ( StringContains( aUpperCurrSymbol, rString, nPos ) )
        {
            nPos = nPos + aUpperCurrSymbol.getLength();
            return true;
        }
        if ( pFormat )
        {
            OUString aSymbol, aExtension;
            if ( pFormat->GetNewCurrencySymbol( aSymbol, aExtension ) )
            {
                if ( aSymbol.getLength() <= rString.getLength() - nPos )
                {
                    aSymbol = pFormatter->GetCharClass()->uppercase(aSymbol);
                    if ( StringContains( aSymbol, rString, nPos ) )
                    {
                        nPos = nPos + aSymbol.getLength();
                        return true;
                    }
                }
            }
        }
    }

    return false;
}


/**
 * Reading the time period specifier (AM/PM) for the 12 hour clock
 *
 *  Returns:
 *   "AM" or "PM" => true
 *   else         => false
 *
 *  nAmPos:
 *   "AM"  =>  1
 *   "PM"  => -1
 *   else =>  0
*/
bool ImpSvNumberInputScan::GetTimeAmPm( const OUString& rString, sal_Int32& nPos )
{

    if ( rString.getLength() > nPos )
    {
        const CharClass* pChr = pFormatter->GetCharClass();
        const LocaleDataWrapper* pLoc = pFormatter->GetLocaleData();
        if ( StringContains( pChr->uppercase( pLoc->getTimeAM() ), rString, nPos ) )
        {
            nAmPm = 1;
            nPos = nPos + pLoc->getTimeAM().getLength();
            return true;
        }
        else if ( StringContains( pChr->uppercase( pLoc->getTimePM() ), rString, nPos ) )
        {
            nAmPm = -1;
            nPos = nPos + pLoc->getTimePM().getLength();
            return true;
        }
    }

    return false;
}


/**
 * Read a decimal separator (',')
 * ','   => true
 * else => false
 */
inline bool ImpSvNumberInputScan::GetDecSep( const OUString& rString, sal_Int32& nPos )
{
    if ( rString.getLength() > nPos )
    {
        const OUString& rSep = pFormatter->GetNumDecimalSep();
        if ( rString.match( rSep, nPos) )
        {
            nPos = nPos + rSep.getLength();
            return true;
        }
    }
    return false;
}


/**
 * Reading a hundredth seconds separator
 */
inline bool ImpSvNumberInputScan::GetTime100SecSep( const OUString& rString, sal_Int32& nPos )
{
    if ( rString.getLength() > nPos )
    {
        const OUString& rSep = pFormatter->GetLocaleData()->getTime100SecSep();
        if ( rString.match( rSep, nPos ))
        {
            nPos = nPos + rSep.getLength();
            return true;
        }
    }
    return false;
}


/**
 * Read a sign including brackets
 * '+'   =>  1
 * '-'   => -1
 *  '('   => -1, nNegCheck = 1
 * else =>  0
 */
int ImpSvNumberInputScan::GetSign( const OUString& rString, sal_Int32& nPos )
{
    if (rString.getLength() > nPos)
        switch (rString[ nPos ])
        {
        case '+':
            nPos++;
            return 1;
        case '(': 
            nNegCheck = 1;
            
        case '-':
            nPos++;
            return -1;
        default:
            break;
        }

    return 0;
}


/**
 * Read a sign with an exponent
 * '+'   =>  1
 * '-'   => -1
 * else =>  0
 */
short ImpSvNumberInputScan::GetESign( const OUString& rString, sal_Int32& nPos )
{
    if (rString.getLength() > nPos)
    {
        switch (rString[nPos])
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
    }
    return 0;
}


/**
 * i counts string portions, j counts numbers thereof.
 * It should had been called SkipNumber instead.
 */
inline bool ImpSvNumberInputScan::GetNextNumber( sal_uInt16& i, sal_uInt16& j )
{
    if ( i < nAnzStrings && IsNum[i] )
    {
        j++;
        i++;
        return true;
    }
    return false;
}


bool ImpSvNumberInputScan::GetTimeRef( double& fOutNumber,
                                       sal_uInt16 nIndex, 
                                       sal_uInt16 nAnz )  
{
    bool bRet = true;
    sal_uInt16 nHour;
    sal_uInt16 nMinute = 0;
    sal_uInt16 nSecond = 0;
    double fSecond100 = 0.0;
    sal_uInt16 nStartIndex = nIndex;

    if (nTimezonePos)
    {
        
        for (sal_uInt16 j=0; j<nAnzNums; ++j)
        {
            if (nNums[j] == nTimezonePos)
            {
                
                if (nStartIndex < j && j - nStartIndex < nAnz)
                {
                    nAnz = j - nStartIndex;
                }
                break;  
            }
        }
    }

    if (nDecPos == 2 && (nAnz == 3 || nAnz == 2)) 
    {
        nHour = 0;
    }
    else if (nIndex - nStartIndex < nAnz)
    {
        nHour   = (sal_uInt16) sStrArray[nNums[nIndex++]].toInt32();
    }
    else
    {
        nHour = 0;
        bRet = false;
        SAL_WARN( "svl.numbers", "ImpSvNumberInputScan::GetTimeRef: bad number index");
    }
    if (nDecPos == 2 && nAnz == 2) 
    {
        nMinute = 0;
    }
    else if (nIndex - nStartIndex < nAnz)
    {
        nMinute = (sal_uInt16) sStrArray[nNums[nIndex++]].toInt32();
    }
    if (nIndex - nStartIndex < nAnz)
    {
        nSecond = (sal_uInt16) sStrArray[nNums[nIndex++]].toInt32();
    }
    if (nIndex - nStartIndex < nAnz)
    {
        fSecond100 = StringToDouble( sStrArray[nNums[nIndex]], true );
    }
    if (nAmPm && nHour > 12) 
    {
        bRet = false;
    }
    else if (nAmPm == -1 && nHour != 12) 
    {
        nHour += 12;
    }
    else if (nAmPm == 1 && nHour == 12) 
    {
        nHour = 0;
    }
    fOutNumber = ((double)nHour*3600 +
                  (double)nMinute*60 +
                  (double)nSecond +
                  fSecond100)/86400.0;
    return bRet;
}


sal_uInt16 ImpSvNumberInputScan::ImplGetDay( sal_uInt16 nIndex )
{
    sal_uInt16 nRes = 0;

    if (sStrArray[nNums[nIndex]].getLength() <= 2)
    {
        sal_uInt16 nNum = (sal_uInt16) sStrArray[nNums[nIndex]].toInt32();
        if (nNum <= 31)
        {
            nRes = nNum;
        }
    }

    return nRes;
}


sal_uInt16 ImpSvNumberInputScan::ImplGetMonth( sal_uInt16 nIndex )
{
    
    sal_uInt16 nRes = pFormatter->GetCalendar()->getNumberOfMonthsInYear();

    if (sStrArray[nNums[nIndex]].getLength() <= 2)
    {
        sal_uInt16 nNum = (sal_uInt16) sStrArray[nNums[nIndex]].toInt32();
        if ( 0 < nNum && nNum <= nRes )
        {
            nRes = nNum - 1; 
        }
    }

    return nRes;
}


/**
 * 30 -> 1930, 29 -> 2029, or 56 -> 1756, 55 -> 1855, ...
 */
sal_uInt16 ImpSvNumberInputScan::ImplGetYear( sal_uInt16 nIndex )
{
    sal_uInt16 nYear = 0;

    sal_Int32 nLen = sStrArray[nNums[nIndex]].getLength();
    if (nLen <= 4)
    {
        nYear = (sal_uInt16) sStrArray[nNums[nIndex]].toInt32();
        
        
        if (nYear < 100 && nLen < 3)
        {
            nYear = SvNumberFormatter::ExpandTwoDigitYear( nYear, nYear2000 );
        }
    }

    return nYear;
}


bool ImpSvNumberInputScan::MayBeIso8601()
{
    if (nMayBeIso8601 == 0)
    {
        nMayBeIso8601 = 1;
        sal_Int32 nLen = ((nAnzNums >= 1 && nNums[0] < nAnzStrings) ? sStrArray[nNums[0]].getLength() : 0);
        if (nLen)
        {
            sal_Int32 n;
            if (nAnzNums >= 3 && nNums[2] < nAnzStrings &&
                comphelper::string::equals(sStrArray[nNums[0]+1], '-') && 
                (n = sStrArray[nNums[1]].toInt32()) >= 1 && n <= 12 &&  
                comphelper::string::equals(sStrArray[nNums[1]+1], '-') && 
                (n = sStrArray[nNums[2]].toInt32()) >= 1 && n <= 31)    
            {
                
                
                nMayBeIso8601 = (nLen >= 4 ? 4 : (nLen == 3 ? 3 : (nLen > 0 ? 2 : 1)));
            }
        }
    }
    return nMayBeIso8601 > 1;
}


bool ImpSvNumberInputScan::CanForceToIso8601( DateFormat eDateFormat )
{
    if (nCanForceToIso8601 == 0)
    {

        if (!MayBeIso8601())
        {
            nCanForceToIso8601 = 1;
        }
        else if (nMayBeIso8601 >= 3)
        {
            nCanForceToIso8601 = 2; 
        }
        else
        {
            nCanForceToIso8601 = 1;
        }

        sal_Int32 n;
        switch (eDateFormat)
        {
        case DMY:               
            if ((n = sStrArray[nNums[0]].toInt32()) < 1 || n > 31)
            {
                nCanForceToIso8601 = 2;
            }
            break;
        case MDY:               
            if ((n = sStrArray[nNums[0]].toInt32()) < 1 || n > 12)
            {
                nCanForceToIso8601 = 2;
            }
            break;
        case YMD:               
            nCanForceToIso8601 = 2;
            break;
        }
    }
    return nCanForceToIso8601 > 1;
}


bool ImpSvNumberInputScan::MayBeMonthDate()
{
    if (nMayBeMonthDate == 0)
    {
        nMayBeMonthDate = 1;
        if (nAnzNums >= 2 && nNums[1] < nAnzStrings)
        {
            
            const OUString& rM = sStrArray[ nNums[ 0 ] + 1 ];
            if (rM.getLength() >= 3 && rM[0] == (sal_Unicode)'-' && rM[ rM.getLength() - 1] == (sal_Unicode)'-')
            {
                
                
                
                bool bYear1 = (sStrArray[nNums[0]].getLength() >= 3);
                bool bYear2 = (sStrArray[nNums[1]].getLength() >= 3);
                sal_Int32 n;
                bool bDay1 = (!bYear1 && (n = sStrArray[nNums[0]].toInt32()) >= 1 && n <= 31);
                bool bDay2 = (!bYear2 && (n = sStrArray[nNums[1]].toInt32()) >= 1 && n <= 31);

                if (bDay1 && !bDay2)
                {
                    nMayBeMonthDate = 2;        
                }
                else if (!bDay1 && bDay2)
                {
                    nMayBeMonthDate = 3;        
                }
                else if (bDay1 && bDay2)
                {
                    if (bYear1 && !bYear2)
                    {
                        nMayBeMonthDate = 3;    
                    }
                    else if (!bYear1 && bYear2)
                    {
                        nMayBeMonthDate = 2;    
                    }
                }
            }
        }
    }
    return nMayBeMonthDate > 1;
}


bool ImpSvNumberInputScan::IsAcceptedDatePattern( sal_uInt16 nStartPatternAt )
{
    if (nAcceptedDatePattern >= -1)
    {
        return (nAcceptedDatePattern >= 0);
    }
    if (!nAnzNums)
    {
        nAcceptedDatePattern = -1;
    }
    else if (!sDateAcceptancePatterns.getLength())
    {
        sDateAcceptancePatterns = pFormatter->GetLocaleData()->getDateAcceptancePatterns();
        SAL_WARN_IF( !sDateAcceptancePatterns.getLength(), "svl.numbers", "ImpSvNumberInputScan::IsAcceptedDatePattern: no date acceptance patterns");
        nAcceptedDatePattern = (sDateAcceptancePatterns.getLength() ? -2 : -1);
    }

    if (nAcceptedDatePattern == -1)
    {
        return false;
    }
    nDatePatternStart = nStartPatternAt; 

    for (sal_Int32 nPattern=0; nPattern < sDateAcceptancePatterns.getLength(); ++nPattern)
    {
        sal_uInt16 nNext = nDatePatternStart;
        nDatePatternNumbers = 0;
        bool bOk = true;
        const OUString& rPat = sDateAcceptancePatterns[nPattern];
        sal_Int32 nPat = 0;
        for ( ; nPat < rPat.getLength() && bOk && nNext < nAnzStrings; ++nPat, ++nNext)
        {
            switch (rPat[nPat])
            {
            case 'Y':
            case 'M':
            case 'D':
                bOk = IsNum[nNext];
                if (bOk)
                    ++nDatePatternNumbers;
                break;
            default:
                bOk = !IsNum[nNext];
                if (bOk)
                {
                    const sal_Int32 nLen = sStrArray[nNext].getLength();
                    bOk = (rPat.indexOf( sStrArray[nNext], nPat) == nPat);
                    if (bOk)
                    {
                        nPat += nLen - 1;
                    }
                    else if (nPat + nLen > rPat.getLength() && sStrArray[nNext][ nLen - 1 ] == ' ')
                    {
                        using namespace comphelper::string;
                        
                        OUStringBuffer aBuf(sStrArray[nNext]);
                        aBuf.stripEnd((sal_Unicode)' ');
                        
                        
                        padToLength(aBuf, rPat.getLength() - nPat, ' ');
                        OUString aStr = aBuf.makeStringAndClear();
                        bOk = (rPat.indexOf( aStr, nPat) == nPat);
                        if (bOk)
                        {
                            nPat += aStr.getLength() - 1;
                        }
                    }
                }
                break;
            }
        }
        if (bOk)
        {
            
            if (nNext < nAnzStrings)
            {
                
                
                
                
                
                sal_Int32 nPos = 0;
                sal_uInt16 nCheck;
                if (nPat > 0 && nNext > 0)
                {
                    
                    sal_Int32 nPatCheck = nPat - 1;
                    switch (rPat[nPatCheck])
                    {
                        case 'Y':
                        case 'M':
                        case 'D':
                            nCheck = nNext;
                            break;
                        default:
                            {
                                nCheck = nNext - 1;
                                
                                
                                sal_Unicode c;
                                do
                                {
                                    ++nPos;
                                } while ((c = rPat[--nPatCheck]) != 'Y' && c != 'M' && c != 'D');
                            }
                    }
                }
                else
                {
                    nCheck = nNext;
                }
                if (!IsNum[nCheck])
                {
                    
                    SkipBlanks( sStrArray[nCheck], nPos);
                    if (nPos == sStrArray[nCheck].getLength())
                    {
                        nAcceptedDatePattern = nPattern;
                        return true;
                    }
                }
            }
            else if (nPat == rPat.getLength())
            {
                
                nAcceptedDatePattern = nPattern;
                return true;
            }
            
        }
    }
    nAcceptedDatePattern = -1;
    return false;
}


bool ImpSvNumberInputScan::SkipDatePatternSeparator( sal_uInt16 nParticle, sal_Int32 & rPos )
{
    
    if (!IsAcceptedDatePattern( (nAnzNums ? nNums[0] : 0)))
    {
        return false;
    }
    if (nParticle < nDatePatternStart || nParticle >= nAnzStrings || IsNum[nParticle])
    {
        return false;
    }
    sal_uInt16 nNext = nDatePatternStart;
    const OUString& rPat = sDateAcceptancePatterns[nAcceptedDatePattern];
    for (sal_Int32 nPat = 0; nPat < rPat.getLength() && nNext < nAnzStrings; ++nPat, ++nNext)
    {
        switch (rPat[nPat])
        {
        case 'Y':
        case 'M':
        case 'D':
            break;
        default:
            if (nNext == nParticle)
            {
                const sal_Int32 nLen = sStrArray[nNext].getLength();
                bool bOk = (rPat.indexOf( sStrArray[nNext], nPat) == nPat);
                if (!bOk && (nPat + nLen > rPat.getLength() && sStrArray[nNext][ nLen - 1 ] == (sal_Unicode)' '))
                {
                    
                    
                    using namespace comphelper::string;
                    OUStringBuffer aBuf(sStrArray[nNext]);
                    aBuf.stripEnd((sal_Unicode)' ');
                    padToLength(aBuf, rPat.getLength() - nPat, ' ');
                    bOk = (rPat.indexOf( aBuf.makeStringAndClear(), nPat) == nPat);
                }
                if (bOk)
                {
                    rPos = nLen; 
                    return true;
                }
                else
                    return false;
            }
            nPat += sStrArray[nNext].getLength() - 1;
            break;
        }
    }
    return false;
}


sal_uInt16 ImpSvNumberInputScan::GetDatePatternNumbers()
{
    
    if (!IsAcceptedDatePattern( (nAnzNums ? nNums[0] : 0)))
    {
        return 0;
    }
    return nDatePatternNumbers;
}


sal_uInt32 ImpSvNumberInputScan::GetDatePatternOrder()
{
    
    if (!IsAcceptedDatePattern( (nAnzNums ? nNums[0] : 0)))
    {
        return 0;
    }
    sal_uInt32 nOrder = 0;
    const OUString& rPat = sDateAcceptancePatterns[nAcceptedDatePattern];
    for (sal_Int32 nPat = 0; nPat < rPat.getLength() && !(nOrder & 0xff0000); ++nPat)
    {
        switch (rPat[nPat])
        {
        case 'Y':
        case 'M':
        case 'D':
            nOrder = (nOrder << 8) | rPat[nPat];
            break;
        }
    }
    return nOrder;
}


DateFormat ImpSvNumberInputScan::GetDateOrder()
{
    sal_uInt32 nOrder = GetDatePatternOrder();
    if (!nOrder)
    {
        return pFormatter->GetLocaleData()->getDateFormat();
    }
    switch ((nOrder & 0xff0000) >> 16)
    {
    case 'Y':
        if ((((nOrder & 0xff00) >> 8) == 'M') && ((nOrder & 0xff) == 'D'))
        {
            return YMD;
        }
        break;
    case 'M':
        if ((((nOrder & 0xff00) >> 8) == 'D') && ((nOrder & 0xff) == 'Y'))
        {
            return MDY;
        }
        break;
    case 'D':
        if ((((nOrder & 0xff00) >> 8) == 'M') && ((nOrder & 0xff) == 'Y'))
        {
            return DMY;
        }
        break;
    default:
    case 0:
        switch ((nOrder & 0xff00) >> 8)
        {
        case 'Y':
            switch ((nOrder & 0xff))
            {
            case 'M':
                return YMD;
            }
            break;
        case 'M':
            switch ((nOrder & 0xff))
            {
            case 'Y':
                return DMY;
            case 'D':
                return MDY;
            }
            break;
        case 'D':
            switch ((nOrder & 0xff))
            {
            case 'Y':
                return MDY;
            case 'M':
                return DMY;
            }
            break;
        default:
        case 0:
            switch ((nOrder & 0xff))
            {
            case 'Y':
                return YMD;
            case 'M':
                return MDY;
            case 'D':
                return DMY;
            }
            break;
        }
    }
    SAL_WARN( "svl.numbers", "ImpSvNumberInputScan::GetDateOrder: undefined, falling back to locale's default");
    return pFormatter->GetLocaleData()->getDateFormat();
}

bool ImpSvNumberInputScan::GetDateRef( double& fDays, sal_uInt16& nCounter,
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
            nFormatOrder = 1; 
            break;
        default:
            nFormatOrder = 2;
            if ( nMatchedAllStrings )
            {
                eEDF = NF_EVALDATEFORMAT_FORMAT_INTL;
                
            }
        }
    }
    else
    {
        eEDF = NF_EVALDATEFORMAT_INTL;
        nFormatOrder = 1;
    }
    bool res = true;

    const LocaleDataWrapper* pLoc = pFormatter->GetLocaleData();
    CalendarWrapper* pCal = pFormatter->GetCalendar();
    for ( int nTryOrder = 1; nTryOrder <= nFormatOrder; nTryOrder++ )
    {
        pCal->setGregorianDateTime( Date( Date::SYSTEM ) ); 
        OUString aOrgCalendar; 
        DateFormat DateFmt;
        bool bFormatTurn;
        switch ( eEDF )
        {
        case NF_EVALDATEFORMAT_INTL :
            bFormatTurn = false;
            DateFmt = GetDateOrder();
            break;
        case NF_EVALDATEFORMAT_FORMAT :
            bFormatTurn = true;
            DateFmt = pFormat->GetDateOrder();
            break;
        case NF_EVALDATEFORMAT_INTL_FORMAT :
            if ( nTryOrder == 1 )
            {
                bFormatTurn = false;
                DateFmt = GetDateOrder();
            }
            else
            {
                bFormatTurn = true;
                DateFmt = pFormat->GetDateOrder();
            }
            break;
        case NF_EVALDATEFORMAT_FORMAT_INTL :
            if ( nTryOrder == 2 )
            {
                bFormatTurn = false;
                DateFmt = GetDateOrder();
            }
            else
            {
                bFormatTurn = true;
                DateFmt = pFormat->GetDateOrder();
            }
            break;
        default:
            SAL_WARN( "svl.numbers", "ImpSvNumberInputScan::GetDateRef: unknown NfEvalDateFormat" );
            DateFmt = YMD;
            bFormatTurn = false;
        }
        if ( bFormatTurn )
        {
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
#ifdef THE_FUTURE
            if ( pFormat->IsOtherCalendar( nStringScanNumFor ) )
            {
                pFormat->SwitchToOtherCalendar( aOrgCalendar, fOrgDateTime );
            }
            else
            {
                pFormat->SwitchToSpecifiedCalendar( aOrgCalendar, fOrgDateTime,
                                                    nStringScanNumFor );
            }
#endif
        }

        res = true;
        nCounter = 0;
        
        pCal->setValue( CalendarFieldIndex::DAY_OF_MONTH, 1 );

        switch (nAnzNums) 
        {
        case 0:                 
            if (nMonthPos)      
            {
                pCal->setValue( CalendarFieldIndex::MONTH, std::abs(nMonth)-1 );
            }
            else
            {
                res = false;
            }
            break;

        case 1:                 
            nCounter = 1;
            switch (nMonthPos)  
            {
            case 0:             
            {
                
                
                
                
                
                sal_uInt32 nDateOrder = (bFormatTurn ?
                                         pFormat->GetExactDateOrder() :
                                         GetDatePatternOrder());
                switch (nDateOrder)
                {
                case 'Y':
                    pCal->setValue( CalendarFieldIndex::YEAR, ImplGetYear(0) );
                    break;
                case 'M':
                    pCal->setValue( CalendarFieldIndex::MONTH, ImplGetMonth(0) );
                    break;
                case 'D':
                default:
                    pCal->setValue( CalendarFieldIndex::DAY_OF_MONTH, ImplGetDay(0) );
                    break;
                }
                break;
            }
            case 1:             
                pCal->setValue( CalendarFieldIndex::MONTH, std::abs(nMonth)-1 );
                switch (DateFmt)
                {
                case MDY:
                case YMD:
                {
                    sal_uInt16 nDay = ImplGetDay(0);
                    sal_uInt16 nYear = ImplGetYear(0);
                    if (nDay == 0 || nDay > 32)
                    {
                        pCal->setValue( CalendarFieldIndex::YEAR, nYear);
                    }
                    else
                    {
                        pCal->setValue( CalendarFieldIndex::DAY_OF_MONTH, ImplGetDay(0) );
                    }
                    break;
                }
                case DMY:
                    pCal->setValue( CalendarFieldIndex::YEAR, ImplGetYear(0) );
                    break;
                default:
                    res = false;
                    break;
                }
                break;
            case 3:             
                pCal->setValue( CalendarFieldIndex::MONTH, std::abs(nMonth)-1 );
                switch (DateFmt)
                {
                case DMY:
                    pCal->setValue( CalendarFieldIndex::DAY_OF_MONTH, ImplGetDay(0) );
                    break;
                case YMD:
                    pCal->setValue( CalendarFieldIndex::YEAR, ImplGetYear(0) );
                    break;
                default:
                    res = false;
                    break;
                }
                break;
            default:
                res = false;
                break;
            }   
            break;

        case 2:                 
            nCounter = 2;
            switch (nMonthPos)  
            {
            case 0:             
            {
                sal_uInt32 nExactDateOrder = (bFormatTurn ?
                                              pFormat->GetExactDateOrder() :
                                              GetDatePatternOrder());
                bool bIsExact = (0xff < nExactDateOrder && nExactDateOrder <= 0xffff);
                if (!bIsExact && bFormatTurn && IsAcceptedDatePattern( nNums[0]))
                {
                    
                    
                    
                    
                    
                    nExactDateOrder = GetDatePatternOrder();
                    bIsExact = (0xff < nExactDateOrder && nExactDateOrder <= 0xffff);
                }
                bool bHadExact;
                if (bIsExact)
                {
                    
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
                    SAL_WARN_IF( !bHadExact, "svl.numbers", "ImpSvNumberInputScan::GetDateRef: error in exact date order");
                }
                else
                {
                    bHadExact = false;
                }
                
                
                
                if ((bFormatTurn || !bIsExact) && (!bHadExact || !pCal->isValid()))
                {
                    if ( !bHadExact && nExactDateOrder )
                    {
                        pCal->setGregorianDateTime( Date( Date::SYSTEM ) ); 
                    }
                    switch (DateFmt)
                    {
                    case MDY:
                        
                        pCal->setValue( CalendarFieldIndex::DAY_OF_MONTH, ImplGetDay(1) );
                        pCal->setValue( CalendarFieldIndex::MONTH, ImplGetMonth(0) );
                        if ( !pCal->isValid() )             
                        {                                   
                            pCal->setValue( CalendarFieldIndex::DAY_OF_MONTH, 1 );
                            pCal->setValue( CalendarFieldIndex::MONTH, ImplGetMonth(0) );
                            pCal->setValue( CalendarFieldIndex::YEAR, ImplGetYear(1) );
                        }
                        break;
                    case DMY:
                        
                        pCal->setValue( CalendarFieldIndex::DAY_OF_MONTH, ImplGetDay(0) );
                        pCal->setValue( CalendarFieldIndex::MONTH, ImplGetMonth(1) );
                        if ( !pCal->isValid() )             
                        {                                   
                            pCal->setValue( CalendarFieldIndex::DAY_OF_MONTH, 1 );
                            pCal->setValue( CalendarFieldIndex::MONTH, ImplGetMonth(0) );
                            pCal->setValue( CalendarFieldIndex::YEAR, ImplGetYear(1) );
                        }
                        break;
                    case YMD:
                        
                        pCal->setValue( CalendarFieldIndex::DAY_OF_MONTH, ImplGetDay(1) );
                        pCal->setValue( CalendarFieldIndex::MONTH, ImplGetMonth(0) );
                        if ( !pCal->isValid() )             
                        {                                   
                            pCal->setValue( CalendarFieldIndex::DAY_OF_MONTH, 1 );
                            pCal->setValue( CalendarFieldIndex::MONTH, ImplGetMonth(1) );
                            pCal->setValue( CalendarFieldIndex::YEAR, ImplGetYear(0) );
                        }
                        break;
                    default:
                        res = false;
                        break;
                    }
                }
            }
            break;
            case 1:             
            {
                
                
                
                pCal->setValue( CalendarFieldIndex::MONTH, std::abs(nMonth)-1 );
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
                break;
            }
            case 2:             
            {
                pCal->setValue( CalendarFieldIndex::MONTH, std::abs(nMonth)-1 );
                DateFormat eDF = (MayBeMonthDate() ? (nMayBeMonthDate == 2 ? DMY : YMD) : DateFmt);
                switch (eDF)
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
                    res = false;
                    break;
                }
                break;
            }
            default:            
                res = false;
                break;
            }   
            break;

        default:                
            switch (nMonthPos)  
            {
            case 0:             
            {
                nCounter = 3;
                if ( nTimePos > 1 )
                {   
                    for ( sal_uInt16 j = 0; j < nAnzNums; j++ )
                    {
                        if ( nNums[j] == nTimePos - 2 )
                        {
                            nCounter = j;
                            break; 
                        }
                    }
                }
                
                DateFormat eDF = (CanForceToIso8601( DateFmt) ? YMD : DateFmt);
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
                    res = false;
                    break;
                }
                break;
            }
            case 1:             
                nCounter = 2;
                switch (DateFmt)
                {
                case MDY:
                    pCal->setValue( CalendarFieldIndex::DAY_OF_MONTH, ImplGetDay(0) );
                    pCal->setValue( CalendarFieldIndex::MONTH, std::abs(nMonth)-1 );
                    pCal->setValue( CalendarFieldIndex::YEAR, ImplGetYear(1) );
                    break;
                default:
                    res = false;
                    break;
                }
                break;
            case 2:             
                nCounter = 2;
                pCal->setValue( CalendarFieldIndex::MONTH, std::abs(nMonth)-1 );
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
                    res = false;
                    break;
                }
                break;
            default:            
                nCounter = 2;
                res = false;
                break;
            }   
            break;
        }   

        if ( res && pCal->isValid() )
        {
            double fDiff = DateTime(*pNullDate) - pCal->getEpochStart();
            fDays = ::rtl::math::approxFloor( pCal->getLocalDateTime() );
            fDays -= fDiff;
            nTryOrder = nFormatOrder; 
        }
        else
        {
            res = false;
        }
        if ( aOrgCalendar.getLength() )
        {
            pCal->loadCalendar( aOrgCalendar, pLoc->getLanguageTag().getLocale() ); 
        }
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
                {0,0,0}
            };
            lang::Locale aLocale;
            bool bValid;
            sal_Int16 nDay, nMyMonth, nYear, nHour, nMinute, nSecond;
            sal_Int16 nDaySet, nMonthSet, nYearSet, nHourSet, nMinuteSet, nSecondSet;
            sal_Int16 nZO, nDST1, nDST2, nDST, nZOmillis, nDST1millis, nDST2millis, nDSTmillis;
            sal_Int32 nZoneInMillis, nDST1InMillis, nDST2InMillis;
            uno::Reference< uno::XComponentContext > xContext =
                ::comphelper::getProcessComponentContext();
            uno::Reference< i18n::XCalendar3 > xCal = i18n::LocaleCalendar::create(xContext);
            for ( const entry* p = cals; p->lan; ++p )
            {
                aLocale.Language = OUString::createFromAscii( p->lan );
                aLocale.Country  = OUString::createFromAscii( p->cou );
                xCal->loadCalendar( OUString::createFromAscii( p->cal ),
                                    aLocale );
                double nDateTime = 0.0; 
                nZO           = xCal->getValue( i18n::CalendarFieldIndex::ZONE_OFFSET );
                nZOmillis     = xCal->getValue( i18n::CalendarFieldIndex::ZONE_OFFSET_SECOND_MILLIS );
                nZoneInMillis = static_cast<sal_Int32>(nZO) * 60000 +
                    (nZO < 0 ? -1 : 1) * static_cast<sal_uInt16>(nZOmillis);
                nDST1         = xCal->getValue( i18n::CalendarFieldIndex::DST_OFFSET );
                nDST1millis   = xCal->getValue( i18n::CalendarFieldIndex::DST_OFFSET_SECOND_MILLIS );
                nDST1InMillis = static_cast<sal_Int32>(nDST1) * 60000 +
                    (nDST1 < 0 ? -1 : 1) * static_cast<sal_uInt16>(nDST1millis);
                nDateTime    -= (double)(nZoneInMillis + nDST1InMillis) / 1000.0 / 60.0 / 60.0 / 24.0;
                xCal->setDateTime( nDateTime );
                nDST2         = xCal->getValue( i18n::CalendarFieldIndex::DST_OFFSET );
                nDST2millis   = xCal->getValue( i18n::CalendarFieldIndex::DST_OFFSET_SECOND_MILLIS );
                nDST2InMillis = static_cast<sal_Int32>(nDST2) * 60000 +
                    (nDST2 < 0 ? -1 : 1) * static_cast<sal_uInt16>(nDST2millis);
                if ( nDST1InMillis != nDST2InMillis )
                {
                    nDateTime = 0.0 - (double)(nZoneInMillis + nDST2InMillis) / 1000.0 / 60.0 / 60.0 / 24.0;
                    xCal->setDateTime( nDateTime );
                }
                nDaySet    = xCal->getValue( i18n::CalendarFieldIndex::DAY_OF_MONTH );
                nMonthSet  = xCal->getValue( i18n::CalendarFieldIndex::MONTH );
                nYearSet   = xCal->getValue( i18n::CalendarFieldIndex::YEAR );
                nHourSet   = xCal->getValue( i18n::CalendarFieldIndex::HOUR );
                nMinuteSet = xCal->getValue( i18n::CalendarFieldIndex::MINUTE );
                nSecondSet = xCal->getValue( i18n::CalendarFieldIndex::SECOND );
                nZO        = xCal->getValue( i18n::CalendarFieldIndex::ZONE_OFFSET );
                nZOmillis  = xCal->getValue( i18n::CalendarFieldIndex::ZONE_OFFSET_SECOND_MILLIS );
                nDST       = xCal->getValue( i18n::CalendarFieldIndex::DST_OFFSET );
                nDSTmillis = xCal->getValue( i18n::CalendarFieldIndex::DST_OFFSET_SECOND_MILLIS );
                xCal->setValue( i18n::CalendarFieldIndex::DAY_OF_MONTH, nDaySet );
                xCal->setValue( i18n::CalendarFieldIndex::MONTH, nMonthSet );
                xCal->setValue( i18n::CalendarFieldIndex::YEAR, nYearSet );
                xCal->setValue( i18n::CalendarFieldIndex::HOUR, nHourSet );
                xCal->setValue( i18n::CalendarFieldIndex::MINUTE, nMinuteSet );
                xCal->setValue( i18n::CalendarFieldIndex::SECOND, nSecondSet );
                bValid  = xCal->isValid();
                nDay    = xCal->getValue( i18n::CalendarFieldIndex::DAY_OF_MONTH );
                nMyMonth= xCal->getValue( i18n::CalendarFieldIndex::MONTH );
                nYear   = xCal->getValue( i18n::CalendarFieldIndex::YEAR );
                nHour   = xCal->getValue( i18n::CalendarFieldIndex::HOUR );
                nMinute = xCal->getValue( i18n::CalendarFieldIndex::MINUTE );
                nSecond = xCal->getValue( i18n::CalendarFieldIndex::SECOND );
                bValid = bValid && nDay == nDaySet && nMyMonth == nMonthSet && nYear ==
                    nYearSet && nHour == nHourSet && nMinute == nMinuteSet && nSecond
                    == nSecondSet;
            }
        }
#endif  

    }

    return res;
}


/**
 * Analyze first string
 * All gone => true
 * else     => false
 */
bool ImpSvNumberInputScan::ScanStartString( const OUString& rString,
                                            const SvNumberformat* pFormat )
{
    sal_Int32 nPos = 0;

    
    SkipBlanks(rString, nPos);

    
    nSign = GetSign(rString, nPos);
    if ( nSign )           
    {
        SkipBlanks(rString, nPos);
    }
    
    if ( nMatchedAllStrings && !(nSign && rString.getLength() == 1) )
    {
        
        
        
        if ( ScanStringNumFor( rString, nPos, pFormat, 0, true ) )
        {
            nMatchedAllStrings |= nMatchedStartString;
        }
        else
        {
            nMatchedAllStrings = 0;
        }
    }

    if ( GetDecSep(rString, nPos) )                 
    {
        nDecPos = 1;
        SkipBlanks(rString, nPos);
    }
    else if ( GetCurrency(rString, nPos, pFormat) ) 
    {
        eScannedType = NUMBERFORMAT_CURRENCY;       
        SkipBlanks(rString, nPos);
        if (nSign == 0)                             
        {
            nSign = GetSign(rString, nPos);
            if ( nSign )   
            {
                SkipBlanks(rString, nPos);
            }
        }
        if ( GetDecSep(rString, nPos) )             
        {
            nDecPos = 1;
            SkipBlanks(rString, nPos);
        }
    }
    else
    {
        nMonth = GetMonth(rString, nPos);
        if ( nMonth )    
        {
            eScannedType = NUMBERFORMAT_DATE;       
            nMonthPos = 1;                          
            if ( nMonth < 0 )
            {
                SkipChar( '.', rString, nPos );     
            }
            SkipBlanks(rString, nPos);
        }
        else
        {
            int nDayOfWeek = GetDayOfWeek( rString, nPos );
            if ( nDayOfWeek )
            {
                
                eScannedType = NUMBERFORMAT_DATE;       
                if ( nPos < rString.getLength() )
                {
                    if ( nDayOfWeek < 0 )
                    {
                        
                        if ( rString[ nPos ] == (sal_Unicode)'.' )
                        {
                            ++nPos;
                        }
                    }
                    else
                    {
                        
                        SkipBlanks(rString, nPos);
                        SkipString( pFormatter->GetLocaleData()->getLongDateDayOfWeekSep(), rString, nPos );
                    }
                    SkipBlanks(rString, nPos);
                    nMonth = GetMonth(rString, nPos);
                    if ( nMonth ) 
                    {
                        nMonthPos = 1; 
                        if ( nMonth < 0 )
                        {
                            SkipChar( '.', rString, nPos ); 
                        }
                        SkipBlanks(rString, nPos);
                    }
                }
                if (!nMonth)
                {
                    
                    IsAcceptedDatePattern( 1);
                }
            }
        }
    }

    
    if (nPos < rString.getLength())
    {
        while (SkipChar ('-', rString, nPos) || SkipChar ('/', rString, nPos))
            ; 
    }
    if (nPos < rString.getLength()) 
    {
        
        
        if ( !ScanStringNumFor( rString, nPos, pFormat, 0 ) )
        {
            return MatchedReturn();
        }
    }

    return true;
}


/**
 * Analyze string in the middle
 * All gone => true
 * else     => false
 */
bool ImpSvNumberInputScan::ScanMidString( const OUString& rString,
                                          sal_uInt16 nStringPos, const SvNumberformat* pFormat )
{
    sal_Int32 nPos = 0;
    short eOldScannedType = eScannedType;

    if ( nMatchedAllStrings )
    {   
        
        
        if ( ScanStringNumFor( rString, 0, pFormat, nStringPos ) )
        {
            nMatchedAllStrings |= nMatchedMidString;
        }
        else
        {
            nMatchedAllStrings = 0;
        }
    }

    SkipBlanks(rString, nPos);
    if (GetDecSep(rString, nPos))                   
    {
        if (nDecPos == 1 || nDecPos == 3)           
        {
            return MatchedReturn();
        }
        else if (nDecPos == 2)                      
        {
            if (bDecSepInDateSeps ||                
                SkipDatePatternSeparator( nStringPos, nPos))
            {
                if ( eScannedType != NUMBERFORMAT_UNDEFINED &&
                     eScannedType != NUMBERFORMAT_DATE &&
                     eScannedType != NUMBERFORMAT_DATETIME)  
                {
                    return MatchedReturn();
                }
                if (eScannedType == NUMBERFORMAT_UNDEFINED)
                {
                    eScannedType = NUMBERFORMAT_DATE; 
                }
                SkipBlanks(rString, nPos);
            }
            else
            {
                return MatchedReturn();
            }
        }
        else
        {
            nDecPos = 2;                            
            SkipBlanks(rString, nPos);
        }
    }
    else if ( ((eScannedType & NUMBERFORMAT_TIME) == NUMBERFORMAT_TIME) &&
              GetTime100SecSep( rString, nPos ) )
    {                                               
        if ( nDecPos )
        {
            return MatchedReturn();
        }
        nDecPos = 2;                                
        SkipBlanks(rString, nPos);
    }

    if (SkipChar('/', rString, nPos))               
    {
        if ( eScannedType != NUMBERFORMAT_UNDEFINED &&  
             eScannedType != NUMBERFORMAT_DATE)       
        {
            return MatchedReturn();                     
        }
        else if (eScannedType != NUMBERFORMAT_DATE &&    
                 ( eSetType == NUMBERFORMAT_FRACTION ||  
                   (nAnzNums == 3 &&                     
                    (nStringPos == 3 ||                  
                     (nStringPos == 4 && nSign)))))      
        {
            SkipBlanks(rString, nPos);
            if (nPos == rString.getLength())
            {
                eScannedType = NUMBERFORMAT_FRACTION;   
                if (eSetType == NUMBERFORMAT_FRACTION &&
                    nAnzNums == 2 &&
                    (nStringPos == 1 ||                     
                     (nStringPos == 2 && nSign)))           
                {
                    return true;                            
                }
            }
        }
        else
        {
            nPos--;                                 
        }
    }

    if (GetThousandSep(rString, nPos, nStringPos))  
    {
        if ( eScannedType != NUMBERFORMAT_UNDEFINED &&   
             eScannedType != NUMBERFORMAT_CURRENCY)      
        {
            return MatchedReturn();
        }
        nThousand++;
    }

    const LocaleDataWrapper* pLoc = pFormatter->GetLocaleData();
    bool bDate = SkipDatePatternSeparator( nStringPos, nPos);   
    if (!bDate)
    {
        const OUString& rDate = pFormatter->GetDateSep();
        SkipBlanks(rString, nPos);
        bDate = SkipString( rDate, rString, nPos);      
    }
    if (bDate || ((MayBeIso8601() || MayBeMonthDate()) &&    
                  SkipChar( '-', rString, nPos)))
    {
        if ( eScannedType != NUMBERFORMAT_UNDEFINED &&  
             eScannedType != NUMBERFORMAT_DATE)       
        {
            return MatchedReturn();
        }
        SkipBlanks(rString, nPos);
        eScannedType = NUMBERFORMAT_DATE;           
        short nTmpMonth = GetMonth(rString, nPos);  
        if (nMonth && nTmpMonth)                    
        {
            return MatchedReturn();
        }
        if (nTmpMonth)
        {
            nMonth = nTmpMonth;
            nMonthPos = 2;                          
            if ( nMonth < 0 && SkipChar( '.', rString, nPos ) )
                ;   
            else if ( SkipChar( '-', rString, nPos ) )
                ;   
                    
            else
            {
                SkipString( pLoc->getLongDateMonthSep(), rString, nPos );
            }
            SkipBlanks(rString, nPos);
        }
    }

    short nTempMonth = GetMonth(rString, nPos);     
    if (nTempMonth)
    {
        if (nMonth != 0)                            
        {
            return MatchedReturn();
        }
        if ( eScannedType != NUMBERFORMAT_UNDEFINED &&  
             eScannedType != NUMBERFORMAT_DATE)         
        {
            return MatchedReturn();
        }
        eScannedType = NUMBERFORMAT_DATE;           
        nMonth = nTempMonth;
        nMonthPos = 2;                              
        if ( nMonth < 0 )
        {
            SkipChar( '.', rString, nPos );         
        }
        SkipString( pLoc->getLongDateMonthSep(), rString, nPos );
        SkipBlanks(rString, nPos);
    }

    if ( SkipChar('E', rString, nPos) ||            
         SkipChar('e', rString, nPos) )
    {
        if (eScannedType != NUMBERFORMAT_UNDEFINED) 
        {
            return MatchedReturn();
        }
        else
        {
            SkipBlanks(rString, nPos);
            eScannedType = NUMBERFORMAT_SCIENTIFIC; 
            if ( nThousand+2 == nAnzNums && nDecPos == 2 ) 
            {
                nDecPos = 3;                        
            }
        }
        nESign = GetESign(rString, nPos);           
        SkipBlanks(rString, nPos);
    }

    const OUString& rTime = pLoc->getTimeSep();
    if ( SkipString(rTime, rString, nPos) )         
    {
        if (nDecPos)                                
        {
            if (bDecSepInDateSeps)                  
            {
                if ( eScannedType != NUMBERFORMAT_DATE &&    
                     eScannedType != NUMBERFORMAT_DATETIME)  
                {
                    return MatchedReturn();
                }
                if (eScannedType == NUMBERFORMAT_DATE)
                {
                    nDecPos = 0;                    
                }
            }
            else
            {
                return MatchedReturn();
            }
        }
        if ((eScannedType == NUMBERFORMAT_DATE ||        
             eScannedType == NUMBERFORMAT_DATETIME) &&   
            nAnzNums > 3)                                
        {
            SkipBlanks(rString, nPos);
            eScannedType = NUMBERFORMAT_DATETIME;   
        }
        else if ( eScannedType != NUMBERFORMAT_UNDEFINED &&  
                  eScannedType != NUMBERFORMAT_TIME)         
        {
            return MatchedReturn();
        }
        else
        {
            SkipBlanks(rString, nPos);
            eScannedType = NUMBERFORMAT_TIME;       
        }
        if ( !nTimePos )
        {
            nTimePos = nStringPos + 1;
        }
    }

    if (nPos < rString.getLength())
    {
        switch (eScannedType)
        {
        case NUMBERFORMAT_DATE:
            if (nMonthPos == 1 && pLoc->getLongDateFormat() == MDY)
            {
                
                if (SkipString( pLoc->getLongDateDaySep(), rString, nPos ))
                {
                    SkipBlanks( rString, nPos );
                }
            }
            else if (nStringPos == 5 && nPos == 0 && rString.getLength() == 1 &&
                     rString[ 0 ] == 'T' && MayBeIso8601())
            {
                
                ++nPos;
            }
            break;
#if NF_RECOGNIZE_ISO8601_TIMEZONES
        case NUMBERFORMAT_DATETIME:
            if (nPos == 0 && rString.getLength() == 1 && nStringPos >= 9 && MayBeIso8601())
            {
                
                switch (rString[ 0 ])
                {
                case '+':
                case '-':
                    if (nStringPos == nAnzStrings - 2 ||
                        nStringPos == nAnzStrings - 4)
                    {
                        ++nPos;     
                        
                        if (!nTimezonePos)
                        {
                            nTimezonePos = nStringPos + 1;
                        }
                    }
                    break;
                case ':':
                    if (nTimezonePos && nStringPos >= 11 &&
                        nStringPos == nAnzStrings - 2)
                    {
                        ++nPos;     
                    }
                    break;
                }
            }
            break;
#endif
        }
    }

    if (nPos < rString.getLength()) 
    {
        if ( nMatchedAllStrings & ~nMatchedVirgin )
        {
            eScannedType = eOldScannedType;
        }
        else
        {
            return false;
        }
    }

    return true;
}


/**
 * Analyze the end
 * All gone => true
 * else     => false
 */
bool ImpSvNumberInputScan::ScanEndString( const OUString& rString,
                                          const SvNumberformat* pFormat )
{
    sal_Int32 nPos = 0;

    if ( nMatchedAllStrings )
    {   
        
        
        if ( ScanStringNumFor( rString, 0, pFormat, 0xFFFF ) )
        {
            nMatchedAllStrings |= nMatchedEndString;
        }
        else
        {
            nMatchedAllStrings = 0;
        }
    }

    SkipBlanks(rString, nPos);
    if (GetDecSep(rString, nPos))                   
    {
        if (nDecPos == 1 || nDecPos == 3)           
        {
            return MatchedReturn();
        }
        else if (nDecPos == 2)                      
        {
            if (bDecSepInDateSeps ||                
                SkipDatePatternSeparator( nAnzStrings-1, nPos))
            {
                if ( eScannedType != NUMBERFORMAT_UNDEFINED &&
                     eScannedType != NUMBERFORMAT_DATE &&
                     eScannedType != NUMBERFORMAT_DATETIME)  
                {
                    return MatchedReturn();
                }
                if (eScannedType == NUMBERFORMAT_UNDEFINED)
                {
                    eScannedType = NUMBERFORMAT_DATE;   
                }
                SkipBlanks(rString, nPos);
            }
            else
            {
                return MatchedReturn();
            }
        }
        else
        {
            nDecPos = 3;                            
            SkipBlanks(rString, nPos);
        }
    }

    bool bSignDetectedHere = false;
    if ( nSign == 0  &&                             
         eScannedType != NUMBERFORMAT_DATE)         
                                                    
    {                                               
        nSign = GetSign(rString, nPos);             
        if (nNegCheck)                              
        {
            return MatchedReturn();
        }
        if (nSign)
        {
            bSignDetectedHere = true;
        }
    }

    SkipBlanks(rString, nPos);
    if (nNegCheck && SkipChar(')', rString, nPos))  
    {
        nNegCheck = 0;
        SkipBlanks(rString, nPos);
    }

    if ( GetCurrency(rString, nPos, pFormat) )      
    {
        if (eScannedType != NUMBERFORMAT_UNDEFINED) 
        {
            return MatchedReturn();
        }
        else
        {
            SkipBlanks(rString, nPos);
            eScannedType = NUMBERFORMAT_CURRENCY;
        }                                           
        if (nSign == 0)                             
        {
            nSign = GetSign(rString, nPos);         
            SkipBlanks(rString, nPos);
            if (nNegCheck)                          
            {
                return MatchedReturn();
            }
        }
        if ( nNegCheck && eScannedType == NUMBERFORMAT_CURRENCY &&
             SkipChar(')', rString, nPos) )
        {
            nNegCheck = 0;                          
            SkipBlanks(rString, nPos);              
        }
    }

    if ( SkipChar('%', rString, nPos) )             
    {
        if (eScannedType != NUMBERFORMAT_UNDEFINED) 
        {
            return MatchedReturn();
        }
        SkipBlanks(rString, nPos);
        eScannedType = NUMBERFORMAT_PERCENT;
    }

    const LocaleDataWrapper* pLoc = pFormatter->GetLocaleData();
    const OUString& rTime = pLoc->getTimeSep();
    if ( SkipString(rTime, rString, nPos) )         
    {
        if (nDecPos)                                
        {
            return MatchedReturn();
        }
        if (eScannedType == NUMBERFORMAT_DATE && nAnzNums > 2) 
        {
            SkipBlanks(rString, nPos);
            eScannedType = NUMBERFORMAT_DATETIME;
        }
        else if (eScannedType != NUMBERFORMAT_UNDEFINED &&
                 eScannedType != NUMBERFORMAT_TIME) 
        {
            return MatchedReturn();
        }
        else
        {
            SkipBlanks(rString, nPos);
            eScannedType = NUMBERFORMAT_TIME;
        }
        if ( !nTimePos )
        {
            nTimePos = nAnzStrings;
        }
    }

    bool bDate = SkipDatePatternSeparator( nAnzStrings-1, nPos);   
    if (!bDate)
    {
        const OUString& rDate = pFormatter->GetDateSep();
        bDate = SkipString( rDate, rString, nPos);      
    }
    if (bDate && bSignDetectedHere)
    {
        nSign = 0;                                  
    }
    if (bDate || ((MayBeIso8601() || MayBeMonthDate())
                  && SkipChar( '-', rString, nPos)))
    {
        if (eScannedType != NUMBERFORMAT_UNDEFINED &&
            eScannedType != NUMBERFORMAT_DATE)          
        {
            return MatchedReturn();
        }
        else
        {
            SkipBlanks(rString, nPos);
            eScannedType = NUMBERFORMAT_DATE;
        }
        short nTmpMonth = GetMonth(rString, nPos);  
        if (nMonth && nTmpMonth)                    
        {
            return MatchedReturn();
        }
        if (nTmpMonth)
        {
            nMonth = nTmpMonth;
            nMonthPos = 3;                          
            if ( nMonth < 0 )
            {
                SkipChar( '.', rString, nPos );     
            }
            SkipBlanks(rString, nPos);
        }
    }

    short nTempMonth = GetMonth(rString, nPos);     
    if (nTempMonth)
    {
        if (nMonth)                                 
        {
            return MatchedReturn();
        }
        if (eScannedType != NUMBERFORMAT_UNDEFINED &&
            eScannedType != NUMBERFORMAT_DATE)      
        {
            return MatchedReturn();
        }
        eScannedType = NUMBERFORMAT_DATE;
        nMonth = nTempMonth;
        nMonthPos = 3;                              
        if ( nMonth < 0 )
        {
            SkipChar( '.', rString, nPos );         
        }
        SkipBlanks(rString, nPos);
    }

    sal_Int32 nOrigPos = nPos;
    if (GetTimeAmPm(rString, nPos))
    {
        if (eScannedType != NUMBERFORMAT_UNDEFINED &&
            eScannedType != NUMBERFORMAT_TIME &&
            eScannedType != NUMBERFORMAT_DATETIME)  
        {
            return MatchedReturn();
        }
        else
        {
            
            
            if (eScannedType != NUMBERFORMAT_TIME && nDecPos == 2 && nAnzNums == 2)
            {
                nPos = nOrigPos; 
            }
            else
            {
                SkipBlanks(rString, nPos);
                if ( eScannedType != NUMBERFORMAT_DATETIME )
                {
                    eScannedType = NUMBERFORMAT_TIME;
                }
            }
        }
    }

    if ( nNegCheck && SkipChar(')', rString, nPos) )
    {
        if (eScannedType == NUMBERFORMAT_CURRENCY)  
        {
            nNegCheck = 0;                          
            SkipBlanks(rString, nPos);
        }
        else
        {
            return MatchedReturn();
        }
    }

    if ( nPos < rString.getLength() &&
         (eScannedType == NUMBERFORMAT_DATE ||
          eScannedType == NUMBERFORMAT_DATETIME) )
    {
        
        sal_Int32 nOldPos = nPos;
        const OUString& rSep = pFormatter->GetLocaleData()->getLongDateDayOfWeekSep();
        if ( StringContains( rSep, rString, nPos ) )
        {
            nPos = nPos + rSep.getLength();
            SkipBlanks(rString, nPos);
        }
        int nDayOfWeek = GetDayOfWeek( rString, nPos );
        if ( nDayOfWeek )
        {
            if ( nPos < rString.getLength() )
            {
                if ( nDayOfWeek < 0 )
                {   
                    if ( rString[ nPos ] == (sal_Unicode)'.' )
                    {
                        ++nPos;
                    }
                }
                SkipBlanks(rString, nPos);
            }
        }
        else
        {
            nPos = nOldPos;
        }
    }

#if NF_RECOGNIZE_ISO8601_TIMEZONES
    if (nPos == 0 && eScannedType == NUMBERFORMAT_DATETIME &&
        rString.getLength() == 1 && rString[ 0 ] == (sal_Unicode)'Z' && MayBeIso8601())
    {
        
        ++nPos;
    }
#endif

    if (nPos < rString.getLength()) 
    {
        
        if ( !ScanStringNumFor( rString, nPos, pFormat, 0xFFFF ) )
        {
            return false;
        }
    }

    return true;
}


bool ImpSvNumberInputScan::ScanStringNumFor( const OUString& rString,       
                                             sal_Int32 nPos,                
                                             const SvNumberformat* pFormat, 
                                             sal_uInt16 nString,            
                                             bool bDontDetectNegation)      
{
    if ( !pFormat )
    {
        return false;
    }
    const ::utl::TransliterationWrapper* pTransliteration = pFormatter->GetTransliteration();
    const OUString* pStr;
    OUString aString( rString );
    bool bFound = false;
    bool bFirst = true;
    bool bContinue = true;
    sal_uInt16 nSub;
    do
    {
        
        
        nSub = nStringScanNumFor;
        do
        {   
            
            pStr = pFormat->GetNumForString( nSub, nString, true );
            if ( pStr && pTransliteration->isEqual( aString, *pStr ) )
            {
                bFound = true;
                bContinue = false;
            }
            else if ( nSub < 2 )
            {
                ++nSub;
            }
            else
            {
                bContinue = false;
            }
        }
        while ( bContinue );
        if ( !bFound && bFirst && nPos )
        {
            
            bFirst = false;
            aString = aString.copy(nPos);
            bContinue = true;
        }
    }
    while ( bContinue );

    if ( !bFound )
    {
        if ( !bDontDetectNegation && (nString == 0) &&
             !bFirst && (nSign < 0) && pFormat->IsSecondSubformatRealNegative() )
        {
            
            aString = comphelper::string::remove(aString, ' ');
            if ( (aString.getLength() == 1) && (aString[0] == '-') )
            {
                bFound = true;
                nStringScanSign = -1;
                nSub = 0; 
            }
        }
        if ( !bFound )
        {
            return false;
        }
    }
    else if ( !bDontDetectNegation && (nSub == 1) &&
              pFormat->IsSecondSubformatRealNegative() )
    {
        
        if ( nStringScanSign < 0 )
        {
            if ( (nSign < 0) && (nStringScanNumFor != 1) )
            {
                nStringScanSign = 1; 
            }
        }
        else if ( nStringScanSign == 0 )
        {
            if ( nSign < 0 )
            {   
                
                if ( (nString == 0) && !bFirst &&
                     SvNumberformat::HasStringNegativeSign( aString ) )
                {
                    nStringScanSign = -1; 
                }
                else if ( pFormat->IsNegativeWithoutSign() )
                {
                    nStringScanSign = -1; 
                }
            }
            else
            {
                nStringScanSign = -1;
            }
        }
        else    
        {
            nStringScanSign = -1;
        }
    }
    nStringScanNumFor = nSub;
    return true;
}


/**
 * Recognizes types of number, exponential, fraction, percent, currency, date, time.
 * Else text => return false
 */
bool ImpSvNumberInputScan::IsNumberFormatMain( const OUString& rString,        
                                               const SvNumberformat* pFormat ) 
{
    Reset();
    NumberStringDivision( rString );             
    if (nAnzStrings >= SV_MAX_ANZ_INPUT_STRINGS) 
    {
        return false;                            
    }
    if (nAnzNums == 0)                           
    {
        if ( nAnzStrings > 0 )
        {
            
            
            sStrArray[0] = comphelper::string::strip(sStrArray[0], ' ');
            OUString& rStrArray = sStrArray[0];
            nLogical = GetLogical( rStrArray );
            if ( nLogical )
            {
                eScannedType = NUMBERFORMAT_LOGICAL; 
                nMatchedAllStrings &= ~nMatchedVirgin;
                return true;
            }
            else
            {
                return false;                   
            }
        }
        else
        {
            return false;                       
        }
    }

    sal_uInt16 i = 0;                           
    sal_uInt16 j = 0;                           

    switch ( nAnzNums )
    {
    case 1 :                                
        
        if (GetNextNumber(i,j)) 
        {   
            if (eSetType == NUMBERFORMAT_FRACTION)  
            {
                if (i >= nAnzStrings || 
                    sStrArray[i] == pFormatter->GetNumDecimalSep())
                {
                    eScannedType = NUMBERFORMAT_FRACTION;
                    nMatchedAllStrings &= ~nMatchedVirgin;
                    return true;
                }
            }
        }
        else
        {                                   
            if (!ScanStartString( sStrArray[i], pFormat ))  
            {
                return false;               
            }
            i++;                            
        }
        GetNextNumber(i,j);                 
        if (eSetType == NUMBERFORMAT_FRACTION)  
        {
            if (nSign && !nNegCheck &&      
                eScannedType == NUMBERFORMAT_UNDEFINED &&   
                nDecPos == 0 &&             
                (i >= nAnzStrings ||        
                 sStrArray[i] == pFormatter->GetNumDecimalSep())
                )
            {
                eScannedType = NUMBERFORMAT_FRACTION;
                nMatchedAllStrings &= ~nMatchedVirgin;
                return true;
            }
        }
        if (i < nAnzStrings && !ScanEndString( sStrArray[i], pFormat ))
        {
            return false;
        }
        break;
    case 2 :                                
                                            
        if (!GetNextNumber(i,j))            
        {                                   
            if (!ScanStartString( sStrArray[i], pFormat ))
                return false;               
            i++;                            
        }
        GetNextNumber(i,j);                 
        if ( !ScanMidString( sStrArray[i], i, pFormat ) )
        {
            return false;
        }
        i++;                                
        GetNextNumber(i,j);                 
        if (i < nAnzStrings && !ScanEndString( sStrArray[i], pFormat ))
        {
            return false;
        }
        if (eSetType == NUMBERFORMAT_FRACTION)  
        {
            if (!nNegCheck  &&                  
                eScannedType == NUMBERFORMAT_UNDEFINED &&
                (nDecPos == 0 || nDecPos == 3)  
                )
            {
                eScannedType = NUMBERFORMAT_FRACTION;
                nMatchedAllStrings &= ~nMatchedVirgin;
                return true;
            }
        }
        break;
    case 3 :                                
                                            
        if (!GetNextNumber(i,j))            
        {                                   
            if (!ScanStartString( sStrArray[i], pFormat ))
            {
                return false;               
            }
            i++;                            
            if (nDecPos == 1)               
            {
                return false;
            }
        }
        GetNextNumber(i,j);                 
        if ( !ScanMidString( sStrArray[i], i, pFormat ) )
        {
            return false;
        }
        i++;                                
        if (eScannedType == NUMBERFORMAT_SCIENTIFIC)    
        {
            return false;
        }
        GetNextNumber(i,j);                 
        if ( !ScanMidString( sStrArray[i], i, pFormat ) )
        {
            return false;
        }
        i++;                                
        GetNextNumber(i,j);                 
        if (i < nAnzStrings && !ScanEndString( sStrArray[i], pFormat ))
        {
            return false;
        }
        if (eSetType == NUMBERFORMAT_FRACTION)  
        {
            if (!nNegCheck  &&                  
                eScannedType == NUMBERFORMAT_UNDEFINED &&
                (nDecPos == 0 || nDecPos == 3)  
                )
            {
                eScannedType = NUMBERFORMAT_FRACTION;
                nMatchedAllStrings &= ~nMatchedVirgin;
                return true;
            }
        }
        if ( eScannedType == NUMBERFORMAT_FRACTION && nDecPos )
        {
            return false;                   
        }
        break;
    default:                                
                                            
        if (!GetNextNumber(i,j))            
        {                                   
            if (!ScanStartString( sStrArray[i], pFormat ))
                return false;               
            i++;                            
            if (nDecPos == 1)               
                return false;
        }
        GetNextNumber(i,j);                 
        if ( !ScanMidString( sStrArray[i], i, pFormat ) )
        {
            return false;
        }
        i++;                                
        {
            sal_uInt16 nThOld = 10;                 
            while (nThOld != nThousand && j < nAnzNums-1) 
                                                          
            {                                             
                nThOld = nThousand;
                if (eScannedType == NUMBERFORMAT_SCIENTIFIC)    
                {
                    return false;
                }
                GetNextNumber(i,j);
                if ( i < nAnzStrings && !ScanMidString( sStrArray[i], i, pFormat ) )
                {
                    return false;
                }
                i++;
            }
        }
        if (eScannedType == NUMBERFORMAT_DATE ||    
            eScannedType == NUMBERFORMAT_TIME ||    
            eScannedType == NUMBERFORMAT_UNDEFINED) 
        {
            for (sal_uInt16 k = j; k < nAnzNums-1; k++)
            {
                if (eScannedType == NUMBERFORMAT_SCIENTIFIC)    
                {
                    return false;
                }
                GetNextNumber(i,j);
                if ( i < nAnzStrings && !ScanMidString( sStrArray[i], i, pFormat ) )
                {
                    return false;
                }
                i++;
            }
        }
        GetNextNumber(i,j);
        if (i < nAnzStrings && !ScanEndString( sStrArray[i], pFormat ))
        {
            return false;
        }
        if (eSetType == NUMBERFORMAT_FRACTION)  
        {
            if (!nNegCheck  &&                  
                eScannedType == NUMBERFORMAT_UNDEFINED &&
                (nDecPos == 0 || nDecPos == 3)  
                )
            {
                eScannedType = NUMBERFORMAT_FRACTION;
                nMatchedAllStrings &= ~nMatchedVirgin;
                return true;
            }
        }
        if ( eScannedType == NUMBERFORMAT_FRACTION && nDecPos )
        {
            return false;                       
        }
        break;
    }

    if (eScannedType == NUMBERFORMAT_UNDEFINED)
    {
        nMatchedAllStrings &= ~nMatchedVirgin;
        
        bool bDidMatch = (nMatchedAllStrings != 0);
        if ( nMatchedAllStrings )
        {
            bool bMatch = (pFormat ? pFormat->IsNumForStringElementCountEqual(
                               nStringScanNumFor, nAnzStrings, nAnzNums ) : false);
            if ( !bMatch )
            {
                nMatchedAllStrings = 0;
            }
        }
        if ( nMatchedAllStrings )
        {
            eScannedType = eSetType;
        }
        else if ( bDidMatch )
        {
            return false;
        }
        else
        {
            eScannedType = NUMBERFORMAT_NUMBER;
            
        }
    }
    else if ( eScannedType == NUMBERFORMAT_DATE )
    {
        
        nMatchedAllStrings &= ~nMatchedVirgin;
        bool bWasReturn = ((nMatchedAllStrings & nMatchedUsedAsReturn) != 0);
        if ( nMatchedAllStrings )
        {
            bool bMatch = (pFormat ? pFormat->IsNumForStringElementCountEqual(
                               nStringScanNumFor, nAnzStrings, nAnzNums ) : false);
            if ( !bMatch )
            {
                nMatchedAllStrings = 0;
            }
        }
        if ( nMatchedAllStrings )
        {
            eScannedType = eSetType;
        }
        else if ( bWasReturn )
        {
            return false;
        }
    }
    else
    {
        nMatchedAllStrings = 0; 
    }
    return true;
}


/**
 * Return true or false depending on the nMatched... state and remember usage
 */
bool ImpSvNumberInputScan::MatchedReturn()
{
    if ( nMatchedAllStrings & ~nMatchedVirgin )
    {
        nMatchedAllStrings |= nMatchedUsedAsReturn;
        return true;
    }
    return false;
}


/**
 * Initialize uppercase months and weekdays
 */
void ImpSvNumberInputScan::InitText()
{
    sal_Int32 j, nElems;
    const CharClass* pChrCls = pFormatter->GetCharClass();
    const CalendarWrapper* pCal = pFormatter->GetCalendar();

    delete [] pUpperMonthText;
    delete [] pUpperAbbrevMonthText;
    ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::CalendarItem2 > xElems
        = pCal->getMonths();
    nElems = xElems.getLength();
    pUpperMonthText = new OUString[nElems];
    pUpperAbbrevMonthText = new OUString[nElems];
    for ( j = 0; j < nElems; j++ )
    {
        pUpperMonthText[j] = pChrCls->uppercase( xElems[j].FullName );
        pUpperAbbrevMonthText[j] = pChrCls->uppercase( xElems[j].AbbrevName );
    }

    delete [] pUpperGenitiveMonthText;
    delete [] pUpperGenitiveAbbrevMonthText;
    xElems = pCal->getGenitiveMonths();
    bScanGenitiveMonths = (nElems != xElems.getLength());
    nElems = xElems.getLength();
    pUpperGenitiveMonthText = new OUString[nElems];
    pUpperGenitiveAbbrevMonthText = new OUString[nElems];
    for ( j = 0; j < nElems; j++ )
    {
        pUpperGenitiveMonthText[j] = pChrCls->uppercase( xElems[j].FullName );
        pUpperGenitiveAbbrevMonthText[j] = pChrCls->uppercase( xElems[j].AbbrevName );
        if (!bScanGenitiveMonths &&
            (pUpperGenitiveMonthText[j] != pUpperMonthText[j] ||
             pUpperGenitiveAbbrevMonthText[j] != pUpperAbbrevMonthText[j]))
        {
            bScanGenitiveMonths = true;
        }
    }

    delete [] pUpperPartitiveMonthText;
    delete [] pUpperPartitiveAbbrevMonthText;
    xElems = pCal->getPartitiveMonths();
    bScanPartitiveMonths = (nElems != xElems.getLength());
    nElems = xElems.getLength();
    pUpperPartitiveMonthText = new OUString[nElems];
    pUpperPartitiveAbbrevMonthText = new OUString[nElems];
    for ( j = 0; j < nElems; j++ )
    {
        pUpperPartitiveMonthText[j] = pChrCls->uppercase( xElems[j].FullName );
        pUpperPartitiveAbbrevMonthText[j] = pChrCls->uppercase( xElems[j].AbbrevName );
        if (!bScanPartitiveMonths &&
            (pUpperPartitiveMonthText[j] != pUpperGenitiveMonthText[j] ||
             pUpperPartitiveAbbrevMonthText[j] != pUpperGenitiveAbbrevMonthText[j]))
        {
            bScanPartitiveMonths = true;
        }
    }

    delete [] pUpperDayText;
    delete [] pUpperAbbrevDayText;
    xElems = pCal->getDays();
    nElems = xElems.getLength();
    pUpperDayText = new OUString[nElems];
    pUpperAbbrevDayText = new OUString[nElems];
    for ( j = 0; j < nElems; j++ )
    {
        pUpperDayText[j] = pChrCls->uppercase( xElems[j].FullName );
        pUpperAbbrevDayText[j] = pChrCls->uppercase( xElems[j].AbbrevName );
    }

    bTextInitialized = true;
}


/**
 * MUST be called if International/Locale is changed
 */
void ImpSvNumberInputScan::ChangeIntl()
{
    sal_Unicode cDecSep = pFormatter->GetNumDecimalSep()[0];
    bDecSepInDateSeps = ( cDecSep == (sal_Unicode)'-' ||
                          cDecSep == pFormatter->GetDateSep()[0] );
    bTextInitialized = false;
    aUpperCurrSymbol ="";
    InvalidateDateAcceptancePatterns();
}


void ImpSvNumberInputScan::InvalidateDateAcceptancePatterns()
{
    if (sDateAcceptancePatterns.getLength())
    {
        sDateAcceptancePatterns = ::com::sun::star::uno::Sequence< OUString >();
    }
}


void ImpSvNumberInputScan::ChangeNullDate( const sal_uInt16 Day,
                                           const sal_uInt16 Month,
                                           const sal_uInt16 Year )
{
    if ( pNullDate )
    {
        *pNullDate = Date(Day, Month, Year);
    }
    else
    {
        pNullDate = new Date(Day, Month, Year);
    }
}


/**
 * Does rString represent a number (also date, time et al)
 */
bool ImpSvNumberInputScan::IsNumberFormat( const OUString& rString,         
                                           short& F_Type,                   
                                           double& fOutNumber,              
                                           const SvNumberformat* pFormat )  
{
    OUString aString;
    bool res; 
    sal_uInt16 k;
    eSetType = F_Type; 

    if ( !rString.getLength() )
    {
        res = false;
    }
    else if (rString.getLength() > 308) 
    {
        res = false;
    }
    else
    {
        
        aString = pFormatter->GetCharClass()->uppercase( rString );
        
        TransformInput(pFormatter, aString);
        res = IsNumberFormatMain( aString, pFormat );
    }

    if (res)
    {
        if ( nNegCheck ||                             
             (nSign && (eScannedType == NUMBERFORMAT_DATE ||
                        eScannedType == NUMBERFORMAT_DATETIME))) 
        {
            res = false;
        }
        else
        {                                           
            switch (eScannedType)
            {
            case NUMBERFORMAT_PERCENT:
            case NUMBERFORMAT_CURRENCY:
            case NUMBERFORMAT_NUMBER:
                if (nDecPos == 1)               
                {
                    
                    if ( nMatchedAllStrings )
                    {
                        nThousand = nAnzNums - 1;
                    }
                    else if ( nAnzNums != 1 )
                    {
                        res = false;
                    }
                }
                else if (nDecPos == 2)          
                {
                    
                    if ( nMatchedAllStrings )
                    {
                        nThousand = nAnzNums - 1;
                    }
                    else if ( nAnzNums != nThousand+2 )
                    {
                        res = false;
                    }
                }
                else                            
                {
                    
                    if ( nMatchedAllStrings )
                    {
                        nThousand = nAnzNums - 1;
                    }
                    else if ( nAnzNums != nThousand+1 )
                    {
                        res = false;
                    }
                }
                break;

            case NUMBERFORMAT_SCIENTIFIC:       
                if (nDecPos == 1)               
                {
                    if (nAnzNums != 2)
                    {
                        res = false;
                    }
                }
                else if (nDecPos == 2)          
                {
                    if (nAnzNums != nThousand+3)
                    {
                        res = false;
                    }
                }
                else                            
                {
                    if (nAnzNums != nThousand+2)
                    {
                        res = false;
                    }
                }
                break;

            case NUMBERFORMAT_DATE:
                if (nMonth)
                {                               
                    if (nAnzNums > 2)
                    {
                        res = false;
                    }
                }
                else
                {
                    if (nAnzNums > 3)
                    {
                        res = false;
                    }
                    else
                    {
                        
                        
                        
                        
                        
                        
                        
                        res = (GetDatePatternNumbers() == nAnzNums)
                            || MayBeIso8601() || nMatchedAllStrings;
                    }
                }
                break;

            case NUMBERFORMAT_TIME:
                if (nDecPos)
                {                               
                    if (nAnzNums > 4)
                    {
                        res = false;
                    }
                }
                else
                {
                    if (nAnzNums > 3)
                    {
                        res = false;
                    }
                }
                break;

            case NUMBERFORMAT_DATETIME:
                if (nMonth)
                {                               
                    if (nDecPos)
                    {                           
                        if (nAnzNums > 6)
                        {
                            res = false;
                        }
                    }
                    else
                    {
                        if (nAnzNums > 5)
                        {
                            res = false;
                        }
                    }
                }
                else
                {
                    if (nDecPos)
                    {                           
                        if (nAnzNums > 7)
                        {
                            res = false;
                        }
                    }
                    else
                    {
                        if (nAnzNums > 6)
                        {
                            res = false;
                        }
                    }
                    if (res)
                    {
                        res = IsAcceptedDatePattern( nNums[0]) || MayBeIso8601() || nMatchedAllStrings;
                    }
                }
                break;

            default:
                break;
            }   
        }   
    }   

    OUStringBuffer sResString;

    if (res)
    {                                       
        switch (eScannedType)
        {
        case NUMBERFORMAT_LOGICAL:
            if (nLogical ==  1)
            {
                fOutNumber = 1.0;           
            }
            else if (nLogical == -1)
            {
                fOutNumber = 0.0;           
            }
            else
            {
                res = false;                
            }
            break;

        case NUMBERFORMAT_PERCENT:
        case NUMBERFORMAT_CURRENCY:
        case NUMBERFORMAT_NUMBER:
        case NUMBERFORMAT_SCIENTIFIC:
        case NUMBERFORMAT_DEFINED:          
            if ( nDecPos == 1 )             
            {
                sResString.append("0.");
            }

            for ( k = 0; k <= nThousand; k++)
            {
                sResString.append(sStrArray[nNums[k]]);  
            }
            if ( nDecPos == 2 && k < nAnzNums )     
            {
                sResString.append('.');
                sal_uInt16 nStop = (eScannedType == NUMBERFORMAT_SCIENTIFIC ?
                                    nAnzNums-1 : nAnzNums);
                for ( ; k < nStop; k++)
                {
                    sResString.append(sStrArray[nNums[k]]);  
                }
            }

            if (eScannedType != NUMBERFORMAT_SCIENTIFIC)
            {
                fOutNumber = StringToDouble(sResString.makeStringAndClear());
            }
            else
            {                                           
                sResString.append('E');
                if ( nESign == -1 )
                {
                    sResString.append('-');
                }
                sResString.append(sStrArray[nNums[nAnzNums-1]]);
                rtl_math_ConversionStatus eStatus;
                fOutNumber = ::rtl::math::stringToDouble( sResString.makeStringAndClear(), '.', ',', &eStatus, NULL );
                if ( eStatus == rtl_math_ConversionStatus_OutOfRange )
                {
                    F_Type = NUMBERFORMAT_TEXT;         
                    if (nESign == -1)
                    {
                        fOutNumber = 0.0;
                    }
                    else
                    {
                        fOutNumber = DBL_MAX;
                    }
                    return true;
                }
            }

            if ( nStringScanSign )
            {
                if ( nSign )
                {
                    nSign *= nStringScanSign;
                }
                else
                {
                    nSign = nStringScanSign;
                }
            }
            if ( nSign < 0 )
            {
                fOutNumber = -fOutNumber;
            }

            if (eScannedType == NUMBERFORMAT_PERCENT)
            {
                fOutNumber/= 100.0;
            }
            break;

        case NUMBERFORMAT_FRACTION:
            if (nAnzNums == 1)
            {
                fOutNumber = StringToDouble(sStrArray[nNums[0]]);
            }
            else if (nAnzNums == 2)
            {
                if (nThousand == 1)
                {
                    sResString = sStrArray[nNums[0]];
                    sResString.append(sStrArray[nNums[1]]); 
                    fOutNumber = StringToDouble(sResString.makeStringAndClear());
                }
                else
                {
                    double fZaehler = StringToDouble(sStrArray[nNums[0]]);
                    double fNenner = StringToDouble(sStrArray[nNums[1]]);
                    if (fNenner != 0.0)
                    {
                        fOutNumber = fZaehler/fNenner;
                    }
                    else
                    {
                        res = false;
                    }
                }
            }
            else 
            {
                k = 1;
                sResString = sStrArray[nNums[0]];
                if (nThousand > 0)
                {
                    for (; k <= nThousand; k++)
                    {
                        sResString.append(sStrArray[nNums[k]]);
                    }
                }
                fOutNumber = StringToDouble(sResString.makeStringAndClear());

                if (k == nAnzNums-2)
                {
                    double fZaehler = StringToDouble(sStrArray[nNums[k]]);
                    double fNenner = StringToDouble(sStrArray[nNums[k + 1]]);
                    if (fNenner != 0.0)
                    {
                        fOutNumber += fZaehler/fNenner;
                    }
                    else
                    {
                        res = false;
                    }
                }
            }

            if ( nStringScanSign )
            {
                if ( nSign )
                {
                    nSign *= nStringScanSign;
                }
                else
                {
                    nSign = nStringScanSign;
                }
            }
            if ( nSign < 0 )
            {
                fOutNumber = -fOutNumber;
            }
            break;

        case NUMBERFORMAT_TIME:
            res = GetTimeRef(fOutNumber, 0, nAnzNums);
            if ( nSign < 0 )
            {
                fOutNumber = -fOutNumber;
            }
            break;

        case NUMBERFORMAT_DATE:
            res = GetDateRef( fOutNumber, k, pFormat );
            break;

        case NUMBERFORMAT_DATETIME:
            res = GetDateRef( fOutNumber, k, pFormat );
            if ( res )
            {
                double fTime;
                res = GetTimeRef( fTime, k, nAnzNums - k );
                fOutNumber += fTime;
            }
            break;

        default:
            SAL_WARN( "svl.numbers", "Some number recognized but what's it?" );
            fOutNumber = 0.0;
            break;
        }
    }

    if (res) 
    {
        if (fOutNumber < -DBL_MAX) 
        {
            F_Type = NUMBERFORMAT_TEXT;
            fOutNumber = -DBL_MAX;
            return true;
        }
        else if (fOutNumber >  DBL_MAX) 
        {
            F_Type = NUMBERFORMAT_TEXT;
            fOutNumber = DBL_MAX;
            return true;
        }
    }

    if (res == false)
    {
        eScannedType = NUMBERFORMAT_TEXT;
        fOutNumber = 0.0;
    }

    F_Type = eScannedType;
    return res;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
