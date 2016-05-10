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

#include <ctype.h>
#include <cstdlib>
#include <float.h>
#include <errno.h>
#include <comphelper/string.hxx>
#include <sal/log.hxx>
#include <tools/date.hxx>
#include <rtl/math.hxx>
#include <unotools/charclass.hxx>
#include <unotools/calendarwrapper.hxx>
#include <unotools/localedatawrapper.hxx>
#include <com/sun/star/i18n/CalendarFieldIndex.hpp>
#include <com/sun/star/i18n/LocaleCalendar2.hpp>
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
#include <com/sun/star/i18n/XCalendar4.hpp>
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

static const sal_Unicode cNoBreakSpace = 0xA0;
static const sal_Unicode cNarrowNoBreakSpace = 0x202F;

ImpSvNumberInputScan::ImpSvNumberInputScan( SvNumberFormatter* pFormatterP )
        :
        pUpperMonthText( nullptr ),
        pUpperAbbrevMonthText( nullptr ),
        pUpperGenitiveMonthText( nullptr ),
        pUpperGenitiveAbbrevMonthText( nullptr ),
        pUpperPartitiveMonthText( nullptr ),
        pUpperPartitiveAbbrevMonthText( nullptr ),
        pUpperDayText( nullptr ),
        pUpperAbbrevDayText( nullptr ),
        bTextInitialized( false ),
        bScanGenitiveMonths( false ),
        bScanPartitiveMonths( false ),
        eScannedType( css::util::NumberFormat::UNDEFINED ),
        eSetType( css::util::NumberFormat::UNDEFINED )
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
    nDayOfWeek   = 0;
    nTimePos     = 0;
    nSign        = 0;
    nESign       = 0;
    nDecPos      = 0;
    nNegCheck    = 0;
    nAnzStrings  = 0;
    nAnzNums     = 0;
    nThousand    = 0;
    eScannedType = css::util::NumberFormat::UNDEFINED;
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


// static
inline bool ImpSvNumberInputScan::MyIsdigit( sal_Unicode c )
{
    return c < 128 && isdigit( (unsigned char) c );
}

// native number transliteration if necessary
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
enum ScanState  // States of the Turing machine
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
        } // switch
    } // while

    if ( nChars )
    {
        rSymbol = OUString( pStr, nChars );
    }
    else
    {
        rSymbol.clear();
    }

    pStr = pHere;

    return isNumber;
}


// FIXME: should be grouping; it is only used though in case nAnzStrings is
// near SV_MAX_ANZ_INPUT_STRINGS, in NumberStringDivision().

bool ImpSvNumberInputScan::SkipThousands( const sal_Unicode*& pStr,
                                          OUString& rSymbol )
{
    bool res = false;
    OUStringBuffer sBuff(rSymbol);
    sal_Unicode cToken;
    const OUString& rThSep = pFormatter->GetNumThousandSep();
    const sal_Unicode* pHere = pStr;
    ScanState eState = SsStart;
    sal_Int32 nCounter = 0; // counts 3 digits

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
                    res = true; // .000 combination found
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
        } // switch
    } // while

    if (eState == SsGetValue) // break with less than 3 digits
    {
        if ( nCounter )
        {
            sBuff.remove( sBuff.getLength() - nCounter, nCounter );
        }
        pHere -= nCounter + rThSep.getLength(); // put back ThSep also
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
        {   // Number
            IsNum[nAnzStrings] = true;
            nNums[nAnzNums] = nAnzStrings;
            nAnzNums++;
            if (nAnzStrings >= SV_MAX_ANZ_INPUT_STRINGS - 7 &&
                nPosThousandString == 0) // Only once
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
 * Whether rString contains word rWhat at nPos
 */
bool ImpSvNumberInputScan::StringContainsWord( const OUString& rWhat,
                                               const OUString& rString, sal_Int32 nPos )
{
    if (rWhat.isEmpty() || rString.getLength() < nPos + rWhat.getLength())
        return false;

    if (StringPtrContainsImpl( rWhat, rString.getStr(), nPos))
    {
        nPos += rWhat.getLength();
        if (nPos == rString.getLength())
            return true;    // word at end of string

        /* TODO: we COULD invoke bells and whistles word break iterator to find
         * the next boundary, but really ... this is called for date input, so
         * how many languages do not separate the day and month names in some
         * form? */

        // Check simple ASCII first before invoking i18n or anything else.
        const sal_Unicode c = rString[nPos];

        // Common separating ASCII characters in date context.
        switch (c)
        {
            case ' ':
            case '-':
            case '.':
            case '/':
                return true;
            default:
                ;   // nothing
        }

        if (rtl::isAsciiAlphanumeric( c ))
            return false;   // Alpha or numeric is not word gap.

        sal_Int32 nIndex = nPos;
        const sal_uInt32 uc = rString.iterateCodePoints( &nIndex);
        if (nPos+1 < nIndex)
            return true;    // Surrogate, assume these to be new words.
        (void)uc;

        const sal_Int32 nType = pFormatter->GetCharClass()->getCharacterType( rString, nPos);
        using namespace ::com::sun::star::i18n;

        if ((nType & (KCharacterType::UPPER | KCharacterType::LOWER | KCharacterType::DIGIT)) != 0)
            return false;   // Alpha or numeric is not word gap.

        if ((nType & (KCharacterType::LETTER)) != 0)
            return true;    // Letter other than alpha is new word. (Is it?)

        return true;        // Catch all remaining as gap until we know better.
    }

    return false;
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
        while ( *p == ' ' || *p == cNoBreakSpace || *p == cNarrowNoBreakSpace )
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
    // Is it an ordinary space instead of a no-break space?
    bool bSpaceBreak = (rSep[0] == cNoBreakSpace || rSep[0] == cNarrowNoBreakSpace) &&
        rString[0] == (sal_Unicode)0x20 &&
        rSep.getLength() == 1 && rString.getLength() == 1;
    if (!((rString == rSep || bSpaceBreak) &&      // nothing else
           nStringPos < nAnzStrings - 1 &&         // safety first!
           IsNum[ nStringPos + 1 ] ))              // number follows
    {
        return false; // no? => out
    }

    utl::DigitGroupingIterator aGrouping( pFormatter->GetLocaleData()->getDigitGrouping());
    // Match ,### in {3} or ,## in {3,2}
    /* FIXME: this could be refined to match ,## in {3,2} only if ,##,## or
     * ,##,### and to match ,### in {3,2} only if it's the last. However,
     * currently there is no track kept where group separators occur. In {3,2}
     * #,###,### and #,##,## would be valid input, which maybe isn't even bad
     * for #,###,###. Other combinations such as #,###,## maybe not. */
    sal_Int32 nLen = sStrArray[ nStringPos + 1 ].getLength();
    if (nLen == aGrouping.get() ||                  // with 3 (or so) digits
        nLen == aGrouping.advance().get() ||        // or with 2 (or 3 or so) digits
        nPosThousandString == nStringPos + 1 )      // or concatenated
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
    // #102136# The correct English form of month September abbreviated is
    // SEPT, but almost every data contains SEP instead.
    static const char aSeptCorrect[] = "SEPT";
    static const char aSepShortened[] = "SEP";

    short res = 0; // no month found

    if (rString.getLength() > nPos) // only if needed
    {
        if ( !bTextInitialized )
        {
            InitText();
        }
        sal_Int16 nMonths = pFormatter->GetCalendar()->getNumberOfMonthsInYear();
        for ( sal_Int16 i = 0; i < nMonths; i++ )
        {
            if ( bScanGenitiveMonths && StringContainsWord( pUpperGenitiveMonthText[i], rString, nPos ) )
            {   // genitive full names first
                nPos = nPos + pUpperGenitiveMonthText[i].getLength();
                res = i + 1;
                break;  // for
            }
            else if ( bScanGenitiveMonths && StringContainsWord( pUpperGenitiveAbbrevMonthText[i], rString, nPos ) )
            {   // genitive abbreviated
                nPos = nPos + pUpperGenitiveAbbrevMonthText[i].getLength();
                res = sal::static_int_cast< short >(-(i+1)); // negative
                break;  // for
            }
            else if ( bScanPartitiveMonths && StringContainsWord( pUpperPartitiveMonthText[i], rString, nPos ) )
            {   // partitive full names
                nPos = nPos + pUpperPartitiveMonthText[i].getLength();
                res = i+1;
                break;  // for
            }
            else if ( bScanPartitiveMonths && StringContainsWord( pUpperPartitiveAbbrevMonthText[i], rString, nPos ) )
            {   // partitive abbreviated
                nPos = nPos + pUpperPartitiveAbbrevMonthText[i].getLength();
                res = sal::static_int_cast< short >(-(i+1)); // negative
                break;  // for
            }
            else if ( StringContainsWord( pUpperMonthText[i], rString, nPos ) )
            {   // noun full names
                nPos = nPos + pUpperMonthText[i].getLength();
                res = i+1;
                break;  // for
            }
            else if ( StringContainsWord( pUpperAbbrevMonthText[i], rString, nPos ) )
            {   // noun abbreviated
                nPos = nPos + pUpperAbbrevMonthText[i].getLength();
                res = sal::static_int_cast< short >(-(i+1)); // negative
                break;  // for
            }
            else if ( i == 8 && pUpperAbbrevMonthText[i] == aSeptCorrect &&
                    StringContainsWord( aSepShortened, rString, nPos ) )
            {   // #102136# SEPT/SEP
                nPos = nPos + strlen(aSepShortened);
                res = sal::static_int_cast< short >(-(i+1)); // negative
                break;  // for
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
    int res = 0; // no day found

    if (rString.getLength() > nPos) // only if needed
    {
        if ( !bTextInitialized )
        {
            InitText();
        }
        sal_Int16 nDays = pFormatter->GetCalendar()->getNumberOfDaysInWeek();
        for ( sal_Int16 i = 0; i < nDays; i++ )
        {
            if ( StringContainsWord( pUpperDayText[i], rString, nPos ) )
            {   // full names first
                nPos = nPos + pUpperDayText[i].getLength();
                res = i + 1;
                break;  // for
            }
            if ( StringContainsWord( pUpperAbbrevDayText[i], rString, nPos ) )
            {   // abbreviated
                nPos = nPos + pUpperAbbrevDayText[i].getLength();
                res = -(i + 1); // negative
                break;  // for
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
        {   // if no format specified the currency of the initialized formatter
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
        case '(': // '(' similar to '-' ?!?
            nNegCheck = 1;
            SAL_FALLTHROUGH;
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
                                       sal_uInt16 nIndex, // j-value of the first numeric time part of input, default 0
                                       sal_uInt16 nAnz )  // count of numeric time parts
{
    bool bRet = true;
    sal_uInt16 nHour;
    sal_uInt16 nMinute = 0;
    sal_uInt16 nSecond = 0;
    double fSecond100 = 0.0;
    sal_uInt16 nStartIndex = nIndex;

    if (nTimezonePos)
    {
        // find first timezone number index and adjust count
        for (sal_uInt16 j=0; j<nAnzNums; ++j)
        {
            if (nNums[j] == nTimezonePos)
            {
                // nAnz is not total count, but count of time relevant strings.
                if (nStartIndex < j && j - nStartIndex < nAnz)
                {
                    nAnz = j - nStartIndex;
                }
                break;  // for
            }
        }
    }

    if (nDecPos == 2 && (nAnz == 3 || nAnz == 2)) // 20:45.5 or 45.5
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
    if (nDecPos == 2 && nAnz == 2) // 45.5
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
    if (nAmPm && nHour > 12) // not a valid AM/PM clock time
    {
        bRet = false;
    }
    else if (nAmPm == -1 && nHour != 12) // PM
    {
        nHour += 12;
    }
    else if (nAmPm == 1 && nHour == 12) // 12 AM
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
    // Preset invalid month number
    sal_uInt16 nRes = pFormatter->GetCalendar()->getNumberOfMonthsInYear();

    if (sStrArray[nNums[nIndex]].getLength() <= 2)
    {
        sal_uInt16 nNum = (sal_uInt16) sStrArray[nNums[nIndex]].toInt32();
        if ( 0 < nNum && nNum <= nRes )
        {
            nRes = nNum - 1; // zero based for CalendarFieldIndex::MONTH
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
        // A year < 100 entered with at least 3 digits with leading 0 is taken
        // as is without expansion.
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
                sStrArray[nNums[0]+1] == "-" && // separator year-month
                (n = sStrArray[nNums[1]].toInt32()) >= 1 && n <= 12 &&  // month
                sStrArray[nNums[1]+1] == "-" && // separator month-day
                (n = sStrArray[nNums[2]].toInt32()) >= 1 && n <= 31)    // day
            {
                // Year (nNums[0]) value not checked, may be anything, but
                // length (number of digits) is checked.
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
            nCanForceToIso8601 = 2; // at least 3 digits in year
        }
        else
        {
            nCanForceToIso8601 = 1;
        }

        sal_Int32 n;
        switch (eDateFormat)
        {
        case DMY:               // "day" value out of range => ISO 8601 year
            if ((n = sStrArray[nNums[0]].toInt32()) < 1 || n > 31)
            {
                nCanForceToIso8601 = 2;
            }
            break;
        case MDY:               // "month" value out of range => ISO 8601 year
            if ((n = sStrArray[nNums[0]].toInt32()) < 1 || n > 12)
            {
                nCanForceToIso8601 = 2;
            }
            break;
        case YMD:               // always possible
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
            // "-Jan-"
            const OUString& rM = sStrArray[ nNums[ 0 ] + 1 ];
            if (rM.getLength() >= 3 && rM[0] == (sal_Unicode)'-' && rM[ rM.getLength() - 1] == (sal_Unicode)'-')
            {
                // Check year length assuming at least 3 digits (including
                // leading zero). Two digit years 1..31 are out of luck here
                // and may be taken as day of month.
                bool bYear1 = (sStrArray[nNums[0]].getLength() >= 3);
                bool bYear2 = (sStrArray[nNums[1]].getLength() >= 3);
                sal_Int32 n;
                bool bDay1 = (!bYear1 && (n = sStrArray[nNums[0]].toInt32()) >= 1 && n <= 31);
                bool bDay2 = (!bYear2 && (n = sStrArray[nNums[1]].toInt32()) >= 1 && n <= 31);

                if (bDay1 && !bDay2)
                {
                    nMayBeMonthDate = 2;        // dd-month-yy
                }
                else if (!bDay1 && bDay2)
                {
                    nMayBeMonthDate = 3;        // yy-month-dd
                }
                else if (bDay1 && bDay2)
                {
                    // Ambiguous ##-MMM-## date, but some big vendor's database
                    // reports write this crap, assume this always to be
                    nMayBeMonthDate = 2;        // dd-month-yy
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
    nDatePatternStart = nStartPatternAt; // remember start particle

    const sal_Int32 nMonthsInYear = pFormatter->GetCalendar()->getNumberOfMonthsInYear();

    for (sal_Int32 nPattern=0; nPattern < sDateAcceptancePatterns.getLength(); ++nPattern)
    {
        sal_uInt16 nNext = nDatePatternStart;
        nDatePatternNumbers = 0;
        bool bOk = true;
        const OUString& rPat = sDateAcceptancePatterns[nPattern];
        sal_Int32 nPat = 0;
        for ( ; nPat < rPat.getLength() && bOk && nNext < nAnzStrings; ++nPat, ++nNext)
        {
            const sal_Unicode c = rPat[nPat];
            switch (c)
            {
            case 'Y':
            case 'M':
            case 'D':
                bOk = IsNum[nNext];
                if (bOk && (c == 'M' || c == 'D'))
                {
                    // Check the D and M cases for plausibility. This also
                    // prevents recognition of date instead of number with a
                    // numeric group input if date separator is identical to
                    // group separator, for example with D.M as a pattern and
                    // #.### as a group.
                    sal_Int32 nMaxLen, nMaxVal;
                    switch (c)
                    {
                        case 'M':
                            nMaxLen = 2;
                            nMaxVal = nMonthsInYear;
                            break;
                        case 'D':
                            nMaxLen = 2;
                            nMaxVal = 31;
                            break;
                        default:
                            // This merely exists against
                            // -Werror=maybe-uninitialized, which is nonsense
                            // after the (c == 'M' || c == 'D') check above,
                            // but ...
                            nMaxLen = 2;
                            nMaxVal = 31;
                    }
                    bOk = (sStrArray[nNext].getLength() <= nMaxLen);
                    if (bOk)
                    {
                        sal_Int32 nNum = sStrArray[nNext].toInt32();
                        bOk = (1 <= nNum && nNum <= nMaxVal);
                    }
                }
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
                        // Trailing blanks in input.
                        OUStringBuffer aBuf(sStrArray[nNext]);
                        aBuf.stripEnd();
                        // Expand again in case of pattern "M. D. " and
                        // input "M. D.  ", maybe fetched far, but..
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
            // Check for trailing characters mismatch.
            if (nNext < nAnzStrings)
            {
                // Pattern end but not input end.
                // A trailing blank may be part of the current pattern input,
                // if pattern is "D.M." and input is "D.M. hh:mm" last was
                // ". ", or may be following the current pattern input, if
                // pattern is "D.M" and input is "D.M hh:mm" last was "M".
                sal_Int32 nPos = 0;
                sal_uInt16 nCheck;
                if (nPat > 0 && nNext > 0)
                {
                    // nPat is one behind after the for loop.
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
                                // Advance position in input to match length of
                                // non-YMD (separator) characters in pattern.
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
                    // Trailing (or separating if time follows) blanks are ok.
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
                // Input end and pattern end => match.
                nAcceptedDatePattern = nPattern;
                return true;
            }
            // else Input end but not pattern end, no match.
        }
    }
    nAcceptedDatePattern = -1;
    return false;
}


bool ImpSvNumberInputScan::SkipDatePatternSeparator( sal_uInt16 nParticle, sal_Int32 & rPos )
{
    // If not initialized yet start with first number, if any.
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
                    // The same ugly trailing blanks check as in
                    // IsAcceptedDatePattern().
                    using namespace comphelper::string;
                    OUStringBuffer aBuf(sStrArray[nNext]);
                    aBuf.stripEnd();
                    padToLength(aBuf, rPat.getLength() - nPat, ' ');
                    bOk = (rPat.indexOf( aBuf.makeStringAndClear(), nPat) == nPat);
                }
                if (bOk)
                {
                    rPos = nLen; // yes, set, not add!
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
    // If not initialized yet start with first number, if any.
    if (!IsAcceptedDatePattern( (nAnzNums ? nNums[0] : 0)))
    {
        return 0;
    }
    return nDatePatternNumbers;
}


sal_uInt32 ImpSvNumberInputScan::GetDatePatternOrder()
{
    // If not initialized yet start with first number, if any.
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
    if ( pFormat && (pFormat->GetType() & css::util::NumberFormat::DATE) )
    {
        eEDF = pFormatter->GetEvalDateFormat();
        switch ( eEDF )
        {
        case NF_EVALDATEFORMAT_INTL :
        case NF_EVALDATEFORMAT_FORMAT :
            nFormatOrder = 1; // only one loop
            break;
        default:
            nFormatOrder = 2;
            if ( nMatchedAllStrings )
            {
                eEDF = NF_EVALDATEFORMAT_FORMAT_INTL;
                // we have a complete match, use it
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
        pCal->setGregorianDateTime( Date( Date::SYSTEM ) ); // today
        OUString aOrgCalendar; // empty => not changed yet
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
        // For incomplete dates, always assume first day of month if not specified.
        pCal->setValue( CalendarFieldIndex::DAY_OF_MONTH, 1 );

        switch (nAnzNums) // count of numbers in string
        {
        case 0:                 // none
            if (nMonthPos)      // only month (Jan)
            {
                pCal->setValue( CalendarFieldIndex::MONTH, std::abs(nMonth)-1 );
            }
            else
            {
                res = false;
            }
            break;

        case 1:                 // only one number
            nCounter = 1;
            switch (nMonthPos)  // where is the month
            {
            case 0:             // not found
            {
                // If input matched a date pattern, use the pattern
                // to determine if it is a day, month or year. The
                // pattern should have only one single value then,
                // 'D-', 'M-' or 'Y-'. If input did not match a
                // pattern assume the usual day of current month.
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
            case 1:             // month at the beginning (Jan 01)
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
            case 3:             // month at the end (10 Jan)
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
            }   // switch (nMonthPos)
            break;

        case 2:                 // 2 numbers
            nCounter = 2;
            switch (nMonthPos)  // where is the month
            {
            case 0:             // not found
            {
                sal_uInt32 nExactDateOrder = (bFormatTurn ?
                                              pFormat->GetExactDateOrder() :
                                              GetDatePatternOrder());
                bool bIsExact = (0xff < nExactDateOrder && nExactDateOrder <= 0xffff);
                if (!bIsExact && bFormatTurn && IsAcceptedDatePattern( nNums[0]))
                {
                    // If input does not match format but pattern, use pattern
                    // instead, even if eEDF==NF_EVALDATEFORMAT_FORMAT_INTL.
                    // For example, format has "Y-M-D" and pattern is "D.M.",
                    // input with 2 numbers can't match format and 31.12. would
                    // lead to 1931-12-01 (fdo#54344)
                    nExactDateOrder = GetDatePatternOrder();
                    bIsExact = (0xff < nExactDateOrder && nExactDateOrder <= 0xffff);
                }
                bool bHadExact;
                if (bIsExact)
                {
                    // formatted as date and exactly 2 parts
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
                // If input matched against a date acceptance pattern
                // do not attempt to mess around with guessing the
                // order, either it matches or it doesn't.
                if ((bFormatTurn || !bIsExact) && (!bHadExact || !pCal->isValid()))
                {
                    if ( !bHadExact && nExactDateOrder )
                    {
                        pCal->setGregorianDateTime( Date( Date::SYSTEM ) ); // reset today
                    }
                    switch (DateFmt)
                    {
                    case MDY:
                        // M D
                        pCal->setValue( CalendarFieldIndex::DAY_OF_MONTH, ImplGetDay(1) );
                        pCal->setValue( CalendarFieldIndex::MONTH, ImplGetMonth(0) );
                        if ( !pCal->isValid() )             // 2nd try
                        {                                   // M Y
                            pCal->setValue( CalendarFieldIndex::DAY_OF_MONTH, 1 );
                            pCal->setValue( CalendarFieldIndex::MONTH, ImplGetMonth(0) );
                            pCal->setValue( CalendarFieldIndex::YEAR, ImplGetYear(1) );
                        }
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
                        res = false;
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
            case 2:             // month in the middle (10 Jan 94)
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
            default:            // else, e.g. month at the end (94 10 Jan)
                res = false;
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
                    for ( sal_uInt16 j = 0; j < nAnzNums; j++ )
                    {
                        if ( nNums[j] == nTimePos - 2 )
                        {
                            nCounter = j;
                            break; // for
                        }
                    }
                }
                // ISO 8601 yyyy-mm-dd forced recognition
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
            case 1:             // month at the beginning (Jan 01 01 8:23)
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
            case 2:             // month in the middle (10 Jan 94 8:23)
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
            default:            // else, e.g. month at the end (94 10 Jan 8:23)
                nCounter = 2;
                res = false;
                break;
            }   // switch (nMonthPos)
            break;
        }   // switch (nAnzNums)

        if ( res && pCal->isValid() )
        {
            double fDiff = DateTime(*pNullDate) - pCal->getEpochStart();
            fDays = ::rtl::math::approxFloor( pCal->getLocalDateTime() );
            fDays -= fDiff;
            nTryOrder = nFormatOrder; // break for
        }
        else
        {
            res = false;
        }
        if ( aOrgCalendar.getLength() )
        {
            pCal->loadCalendar( aOrgCalendar, pLoc->getLanguageTag().getLocale() ); // restore calendar
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
            uno::Reference< i18n::XCalendar4 > xCal = i18n::LocaleCalendar2::create(xContext);
            for ( const entry* p = cals; p->lan; ++p )
            {
                aLocale.Language = OUString::createFromAscii( p->lan );
                aLocale.Country  = OUString::createFromAscii( p->cou );
                xCal->loadCalendar( OUString::createFromAscii( p->cal ),
                                    aLocale );
                double nDateTime = 0.0; // 1-Jan-1970 00:00:00
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
#endif  // NF_TEST_CALENDAR

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

    // First of all, eat leading blanks
    SkipBlanks(rString, nPos);

    // Yes, nMatchedAllStrings should know about the sign position
    nSign = GetSign(rString, nPos);
    if ( nSign )           // sign?
    {
        SkipBlanks(rString, nPos);
    }
    // #102371# match against format string only if start string is not a sign character
    if ( nMatchedAllStrings && !(nSign && rString.getLength() == 1) )
    {
        // Match against format in any case, so later on for a "x1-2-3" input
        // we may distinguish between a xy-m-d (or similar) date and a x0-0-0
        // format. No sign detection here!
        if ( ScanStringNumFor( rString, nPos, pFormat, 0, true ) )
        {
            nMatchedAllStrings |= nMatchedStartString;
        }
        else
        {
            nMatchedAllStrings = 0;
        }
    }

    if ( GetDecSep(rString, nPos) )                 // decimal separator in start string
    {
        nDecPos = 1;
        SkipBlanks(rString, nPos);
    }
    else if ( GetCurrency(rString, nPos, pFormat) ) // currency (DM 1)?
    {
        eScannedType = css::util::NumberFormat::CURRENCY;       // !!! it IS currency !!!
        SkipBlanks(rString, nPos);
        if (nSign == 0)                             // no sign yet
        {
            nSign = GetSign(rString, nPos);
            if ( nSign )   // DM -1
            {
                SkipBlanks(rString, nPos);
            }
        }
        if ( GetDecSep(rString, nPos) )             // decimal separator follows currency
        {
            nDecPos = 1;
            SkipBlanks(rString, nPos);
        }
    }
    else
    {
        const sal_Int32 nMonthStart = nPos;
        short nTempMonth = GetMonth(rString, nPos);
        if (nTempMonth < 0)
        {
            // Short month and day names may be identical in some locales, e.g.
            // "mar" for "martes" or "marzo" in Spanish.
            // Do not let a month name immediately take precedence if a day
            // name was meant instead. Assume that both could be valid, until
            // encountered differently or the final evaluation in
            // IsNumberFormat() checks, but continue with weighing the month
            // name higher unless we have both day of week and month name here.
            sal_Int32 nTempPos = nMonthStart;
            nDayOfWeek = GetDayOfWeek( rString, nTempPos);
            if (nDayOfWeek < 0)
            {
                SkipChar( '.', rString, nTempPos ); // abbreviated
                SkipString( pFormatter->GetLocaleData()->getLongDateDayOfWeekSep(), rString, nTempPos );
                SkipBlanks( rString, nTempPos);
                short nTempTempMonth = GetMonth( rString, nTempPos);
                if (nTempTempMonth)
                {
                    // Fall into the else branch below that handles both.
                    nTempMonth = 0;
                    nPos = nMonthStart;
                    nDayOfWeek = 0;
                    // Do not set nDayOfWeek hereafter, anywhere.
                }
            }
        }
        if ( nTempMonth )    // month (Jan 1)?
        {
            // Jan1 without separator is not a date, unless it is followed by a
            // separator and a (year) number.
            if (nPos < rString.getLength() || (nAnzStrings >= 4 && nAnzNums >= 2))
            {
                eScannedType = css::util::NumberFormat::DATE;   // !!! it IS a date !!!
                nMonth = nTempMonth;
                nMonthPos = 1;                      // month at the beginning
                if ( nMonth < 0 )
                {
                    SkipChar( '.', rString, nPos ); // abbreviated
                }
                SkipBlanks(rString, nPos);
            }
            else
            {
                nPos = nMonthStart;                 // rewind month
            }
        }
        else
        {
            int nTempDayOfWeek = GetDayOfWeek( rString, nPos );
            if ( nTempDayOfWeek )
            {
                // day of week is just parsed away
                eScannedType = css::util::NumberFormat::DATE;       // !!! it IS a date !!!
                if ( nPos < rString.getLength() )
                {
                    if ( nTempDayOfWeek < 0 )
                    {
                        // abbreviated
                        if ( rString[ nPos ] == (sal_Unicode)'.' )
                        {
                            ++nPos;
                        }
                    }
                    else
                    {
                        // full long name
                        SkipBlanks(rString, nPos);
                        SkipString( pFormatter->GetLocaleData()->getLongDateDayOfWeekSep(), rString, nPos );
                    }
                    SkipBlanks(rString, nPos);
                    nTempMonth = GetMonth(rString, nPos);
                    if ( nTempMonth ) // month (Jan 1)?
                    {
                        // Jan1 without separator is not a date, unless it is followed by a
                        // separator and a (year) number.
                        if (nPos < rString.getLength() || (nAnzStrings >= 4 && nAnzNums >= 2))
                        {
                            nMonth = nTempMonth;
                            nMonthPos = 1; // month a the beginning
                            if ( nMonth < 0 )
                            {
                                SkipChar( '.', rString, nPos ); // abbreviated
                            }
                            SkipBlanks(rString, nPos);
                        }
                        else
                        {
                            nPos = nMonthStart;                 // rewind month
                        }
                    }
                }
                if (!nMonth)
                {
                    // Determine and remember following date pattern, if any.
                    IsAcceptedDatePattern( 1);
                }
            }
        }
    }

    // skip any trailing '-' or '/' chars
    if (nPos < rString.getLength())
    {
        while (SkipChar ('-', rString, nPos) || SkipChar ('/', rString, nPos))
            ; // do nothing
    }
    if (nPos < rString.getLength()) // not everything consumed
    {
        // Does input StartString equal StartString of format?
        // This time with sign detection!
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
    {   // Match against format in any case, so later on for a "1-2-3-4" input
        // we may distinguish between a y-m-d (or similar) date and a 0-0-0-0
        // format.
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
    if (GetDecSep(rString, nPos))                   // decimal separator?
    {
        if (nDecPos == 1 || nDecPos == 3)           // .12.4 or 1.E2.1
        {
            return MatchedReturn();
        }
        else if (nDecPos == 2)                      // . dup: 12.4.
        {
            if (bDecSepInDateSeps ||                // . also date separator
                SkipDatePatternSeparator( nStringPos, nPos))
            {
                if ( eScannedType != css::util::NumberFormat::UNDEFINED &&
                     eScannedType != css::util::NumberFormat::DATE &&
                     eScannedType != css::util::NumberFormat::DATETIME)  // already another type
                {
                    return MatchedReturn();
                }
                if (eScannedType == css::util::NumberFormat::UNDEFINED)
                {
                    eScannedType = css::util::NumberFormat::DATE; // !!! it IS a date
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
            nDecPos = 2;                            // . in mid string
            SkipBlanks(rString, nPos);
        }
    }
    else if ( (eScannedType & css::util::NumberFormat::TIME) &&
              GetTime100SecSep( rString, nPos ) )
    {                                               // hundredth seconds separator
        if ( nDecPos )
        {
            return MatchedReturn();
        }
        nDecPos = 2;                                // . in mid string
        SkipBlanks(rString, nPos);
    }

    if (SkipChar('/', rString, nPos))               // fraction?
    {
        if ( eScannedType != css::util::NumberFormat::UNDEFINED &&  // already another type
             eScannedType != css::util::NumberFormat::DATE)       // except date
        {
            return MatchedReturn();                     // => jan/31/1994
        }
        else if (eScannedType != css::util::NumberFormat::DATE &&    // analyzed no date until now
                 ( eSetType == css::util::NumberFormat::FRACTION ||  // and preset was fraction
                   (nAnzNums == 3 &&                     // or 3 numbers
                    (nStringPos == 3 ||                  // and 3rd string particle
                     (nStringPos == 4 && nSign)))))      // or 4th  if signed
        {
            SkipBlanks(rString, nPos);
            if (nPos == rString.getLength())
            {
                eScannedType = css::util::NumberFormat::FRACTION;   // !!! it IS a fraction (so far)
                if (eSetType == css::util::NumberFormat::FRACTION &&
                    nAnzNums == 2 &&
                    (nStringPos == 1 ||                     // for 4/5
                     (nStringPos == 2 && nSign)))           // or signed -4/5
                {
                    return true;                            // don't fall into date trap
                }
            }
        }
        else
        {
            nPos--;                                 // put '/' back
        }
    }

    if (GetThousandSep(rString, nPos, nStringPos))  // 1,000
    {
        if ( eScannedType != css::util::NumberFormat::UNDEFINED &&   // already another type
             eScannedType != css::util::NumberFormat::CURRENCY)      // except currency
        {
            return MatchedReturn();
        }
        nThousand++;
    }

    const LocaleDataWrapper* pLoc = pFormatter->GetLocaleData();
    bool bDate = SkipDatePatternSeparator( nStringPos, nPos);   // 12/31  31.12.  12/31/1999  31.12.1999
    if (!bDate)
    {
        const OUString& rDate = pFormatter->GetDateSep();
        SkipBlanks(rString, nPos);
        bDate = SkipString( rDate, rString, nPos);      // 10.  10-  10/
    }
    if (bDate || ((MayBeIso8601() || MayBeMonthDate()) &&    // 1999-12-31  31-Dec-1999
                  SkipChar( '-', rString, nPos)))
    {
        if ( eScannedType != css::util::NumberFormat::UNDEFINED &&  // already another type
             eScannedType != css::util::NumberFormat::DATE)       // except date
        {
            return MatchedReturn();
        }
        SkipBlanks(rString, nPos);
        eScannedType = css::util::NumberFormat::DATE;           // !!! it IS a date
        short nTmpMonth = GetMonth(rString, nPos);  // 10. Jan 94
        if (nMonth && nTmpMonth)                    // month dup
        {
            return MatchedReturn();
        }
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
            {
                SkipString( pLoc->getLongDateMonthSep(), rString, nPos );
            }
            SkipBlanks(rString, nPos);
        }
    }

    const sal_Int32 nMonthStart = nPos;
    short nTempMonth = GetMonth(rString, nPos);     // month in the middle (10 Jan 94)
    if (nTempMonth)
    {
        if (nMonth != 0)                            // month dup
        {
            return MatchedReturn();
        }
        if ( eScannedType != css::util::NumberFormat::UNDEFINED &&  // already another type
             eScannedType != css::util::NumberFormat::DATE)         // except date
        {
            return MatchedReturn();
        }
        if (nMonthStart > 0 && nPos < rString.getLength())  // 10Jan or Jan94 without separator are not dates
        {
            eScannedType = css::util::NumberFormat::DATE;       // !!! it IS a date
            nMonth = nTempMonth;
            nMonthPos = 2;                          // month in the middle
            if ( nMonth < 0 )
            {
                SkipChar( '.', rString, nPos );     // abbreviated
            }
            SkipString( pLoc->getLongDateMonthSep(), rString, nPos );
            SkipBlanks(rString, nPos);
        }
        else
        {
            nPos = nMonthStart;                     // rewind month
        }
    }

    if ( SkipChar('E', rString, nPos) ||            // 10E, 10e, 10,Ee
         SkipChar('e', rString, nPos) )
    {
        if (eScannedType != css::util::NumberFormat::UNDEFINED) // already another type
        {
            return MatchedReturn();
        }
        else
        {
            SkipBlanks(rString, nPos);
            eScannedType = css::util::NumberFormat::SCIENTIFIC; // !!! it IS scientific
            if ( nThousand+2 == nAnzNums && nDecPos == 2 ) // special case 1.E2
            {
                nDecPos = 3;                        // 1,100.E2 1,100,100.E3
            }
        }
        nESign = GetESign(rString, nPos);           // signed exponent?
        SkipBlanks(rString, nPos);
    }

    const OUString& rTime = pLoc->getTimeSep();
    if ( SkipString(rTime, rString, nPos) )         // time separator?
    {
        if (nDecPos)                                // already . => maybe error
        {
            if (bDecSepInDateSeps)                  // . also date sep
            {
                if ( eScannedType != css::util::NumberFormat::DATE &&    // already another type than date
                     eScannedType != css::util::NumberFormat::DATETIME)  // or date time
                {
                    return MatchedReturn();
                }
                if (eScannedType == css::util::NumberFormat::DATE)
                {
                    nDecPos = 0;                    // reset for time transition
                }
            }
            else
            {
                return MatchedReturn();
            }
        }
        if ((eScannedType == css::util::NumberFormat::DATE ||        // already date type
             eScannedType == css::util::NumberFormat::DATETIME) &&   // or date time
            nAnzNums > 3)                                // and more than 3 numbers? (31.Dez.94 8:23)
        {
            SkipBlanks(rString, nPos);
            eScannedType = css::util::NumberFormat::DATETIME;   // !!! it IS date with time
        }
        else if ( eScannedType != css::util::NumberFormat::UNDEFINED &&  // already another type
                  eScannedType != css::util::NumberFormat::TIME)         // except time
        {
            return MatchedReturn();
        }
        else
        {
            SkipBlanks(rString, nPos);
            eScannedType = css::util::NumberFormat::TIME;       // !!! it IS a time
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
        case css::util::NumberFormat::DATE:
            if (nMonthPos == 1 && pLoc->getLongDateFormat() == MDY)
            {
                // #68232# recognize long date separators like ", " in "September 5, 1999"
                if (SkipString( pLoc->getLongDateDaySep(), rString, nPos ))
                {
                    SkipBlanks( rString, nPos );
                }
            }
            else if (nPos == 0 && rString.getLength() == 1 && MayBeIso8601())
            {
                if (nStringPos == 5 && rString[0] == 'T')
                {
                    // ISO 8601 combined date and time, yyyy-mm-ddThh:mm
                    ++nPos;
                }
                else if (nStringPos == 7 && rString[0] == ':')
                {
                    // ISO 8601 combined date and time, the time part; we reach
                    // here if the locale's separator is not ':' so it couldn't
                    // be detected above in the time block.
                    if (nAnzNums >= 5)
                        eScannedType = css::util::NumberFormat::DATETIME;
                    ++nPos;
                }
            }
            break;
        case css::util::NumberFormat::DATETIME:
            if (nPos == 0 && rString.getLength() == 1 && MayBeIso8601())
            {
                if (nStringPos == 9 && rString[0] == ':')
                {
                    // ISO 8601 combined date and time, the time part continued.
                    ++nPos;
                }
            }
#if NF_RECOGNIZE_ISO8601_TIMEZONES
            else if (nPos == 0 && rString.getLength() == 1 && nStringPos >= 9 && MayBeIso8601())
            {
                // ISO 8601 timezone offset
                switch (rString[ 0 ])
                {
                case '+':
                case '-':
                    if (nStringPos == nAnzStrings - 2 ||
                        nStringPos == nAnzStrings - 4)
                    {
                        ++nPos;     // yyyy-mm-ddThh:mm[:ss]+xx[[:]yy]
                        // nTimezonePos needed for GetTimeRef()
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
                        ++nPos;     // yyyy-mm-ddThh:mm[:ss]+xx:yy
                    }
                    break;
                }
            }
#endif
            break;
        }
    }

    if (nPos < rString.getLength()) // not everything consumed?
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
    {   // Match against format in any case, so later on for a "1-2-3-4" input
        // we may distinguish between a y-m-d (or similar) date and a 0-0-0-0
        // format.
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
    if (GetDecSep(rString, nPos))                   // decimal separator?
    {
        if (nDecPos == 1 || nDecPos == 3)           // .12.4 or 12.E4.
        {
            return MatchedReturn();
        }
        else if (nDecPos == 2)                      // . dup: 12.4.
        {
            if (bDecSepInDateSeps ||                // . also date separator
                SkipDatePatternSeparator( nAnzStrings-1, nPos))
            {
                if ( eScannedType != css::util::NumberFormat::UNDEFINED &&
                     eScannedType != css::util::NumberFormat::DATE &&
                     eScannedType != css::util::NumberFormat::DATETIME)  // already another type
                {
                    return MatchedReturn();
                }
                if (eScannedType == css::util::NumberFormat::UNDEFINED)
                {
                    eScannedType = css::util::NumberFormat::DATE;   // !!! it IS a date
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
            nDecPos = 3;                            // . in end string
            SkipBlanks(rString, nPos);
        }
    }

    bool bSignDetectedHere = false;
    if ( nSign == 0  &&                             // conflict - not signed
         eScannedType != css::util::NumberFormat::DATE)         // and not date
                                                    //!? catch time too?
    {                                               // not signed yet
        nSign = GetSign(rString, nPos);             // 1- DM
        if (nNegCheck)                              // '(' as sign
        {
            return MatchedReturn();
        }
        if (nSign)
        {
            bSignDetectedHere = true;
        }
    }

    SkipBlanks(rString, nPos);
    if (nNegCheck && SkipChar(')', rString, nPos))  // skip ')' if appropriate
    {
        nNegCheck = 0;
        SkipBlanks(rString, nPos);
    }

    if ( GetCurrency(rString, nPos, pFormat) )      // currency symbol?
    {
        if (eScannedType != css::util::NumberFormat::UNDEFINED) // currency dup
        {
            return MatchedReturn();
        }
        else
        {
            SkipBlanks(rString, nPos);
            eScannedType = css::util::NumberFormat::CURRENCY;
        }                                           // behind currency a '-' is allowed
        if (nSign == 0)                             // not signed yet
        {
            nSign = GetSign(rString, nPos);         // DM -
            SkipBlanks(rString, nPos);
            if (nNegCheck)                          // 3 DM (
            {
                return MatchedReturn();
            }
        }
        if ( nNegCheck && eScannedType == css::util::NumberFormat::CURRENCY &&
             SkipChar(')', rString, nPos) )
        {
            nNegCheck = 0;                          // ')' skipped
            SkipBlanks(rString, nPos);              // only if currency
        }
    }

    if ( SkipChar('%', rString, nPos) )             // 1%
    {
        if (eScannedType != css::util::NumberFormat::UNDEFINED) // already another type
        {
            return MatchedReturn();
        }
        SkipBlanks(rString, nPos);
        eScannedType = css::util::NumberFormat::PERCENT;
    }

    const LocaleDataWrapper* pLoc = pFormatter->GetLocaleData();
    const OUString& rTime = pLoc->getTimeSep();
    if ( SkipString(rTime, rString, nPos) )         // 10:
    {
        if (nDecPos)                                // already , => error
        {
            return MatchedReturn();
        }
        if (eScannedType == css::util::NumberFormat::DATE && nAnzNums > 2) // 31.Dez.94 8:
        {
            SkipBlanks(rString, nPos);
            eScannedType = css::util::NumberFormat::DATETIME;
        }
        else if (eScannedType != css::util::NumberFormat::UNDEFINED &&
                 eScannedType != css::util::NumberFormat::TIME) // already another type
        {
            return MatchedReturn();
        }
        else
        {
            SkipBlanks(rString, nPos);
            eScannedType = css::util::NumberFormat::TIME;
        }
        if ( !nTimePos )
        {
            nTimePos = nAnzStrings;
        }
    }

    bool bDate = SkipDatePatternSeparator( nAnzStrings-1, nPos);   // 12/31  31.12.  12/31/1999  31.12.1999
    if (!bDate)
    {
        const OUString& rDate = pFormatter->GetDateSep();
        bDate = SkipString( rDate, rString, nPos);      // 10.  10-  10/
    }
    if (bDate && bSignDetectedHere)
    {
        nSign = 0;                                  // 'D-' takes precedence over signed date
    }
    if (bDate || ((MayBeIso8601() || MayBeMonthDate())
                  && SkipChar( '-', rString, nPos)))
    {
        if (eScannedType != css::util::NumberFormat::UNDEFINED &&
            eScannedType != css::util::NumberFormat::DATE)          // already another type
        {
            return MatchedReturn();
        }
        else
        {
            SkipBlanks(rString, nPos);
            eScannedType = css::util::NumberFormat::DATE;
        }
        short nTmpMonth = GetMonth(rString, nPos);  // 10. Jan
        if (nMonth && nTmpMonth)                    // month dup
        {
            return MatchedReturn();
        }
        if (nTmpMonth)
        {
            nMonth = nTmpMonth;
            nMonthPos = 3;                          // month at end
            if ( nMonth < 0 )
            {
                SkipChar( '.', rString, nPos );     // abbreviated
            }
            SkipBlanks(rString, nPos);
        }
    }

    const sal_Int32 nMonthStart = nPos;
    short nTempMonth = GetMonth(rString, nPos);     // 10 Jan
    if (nTempMonth)
    {
        if (nMonth)                                 // month dup
        {
            return MatchedReturn();
        }
        if (eScannedType != css::util::NumberFormat::UNDEFINED &&
            eScannedType != css::util::NumberFormat::DATE)      // already another type
        {
            return MatchedReturn();
        }
        if (nMonthStart > 0)                        // 10Jan without separator is not a date
        {
            eScannedType = css::util::NumberFormat::DATE;
            nMonth = nTempMonth;
            nMonthPos = 3;                          // month at end
            if ( nMonth < 0 )
            {
                SkipChar( '.', rString, nPos );     // abbreviated
            }
            SkipBlanks(rString, nPos);
        }
        else
        {
            nPos = nMonthStart;                     // rewind month
        }
    }

    sal_Int32 nOrigPos = nPos;
    if (GetTimeAmPm(rString, nPos))
    {
        if (eScannedType != css::util::NumberFormat::UNDEFINED &&
            eScannedType != css::util::NumberFormat::TIME &&
            eScannedType != css::util::NumberFormat::DATETIME)  // already another type
        {
            return MatchedReturn();
        }
        else
        {
            // If not already scanned as time, 6.78am does not result in 6
            // seconds and 78 hundredths in the morning. Keep as suffix.
            if (eScannedType != css::util::NumberFormat::TIME && nDecPos == 2 && nAnzNums == 2)
            {
                nPos = nOrigPos; // rewind am/pm
            }
            else
            {
                SkipBlanks(rString, nPos);
                if ( eScannedType != css::util::NumberFormat::DATETIME )
                {
                    eScannedType = css::util::NumberFormat::TIME;
                }
            }
        }
    }

    if ( nNegCheck && SkipChar(')', rString, nPos) )
    {
        if (eScannedType == css::util::NumberFormat::CURRENCY)  // only if currency
        {
            nNegCheck = 0;                          // skip ')'
            SkipBlanks(rString, nPos);
        }
        else
        {
            return MatchedReturn();
        }
    }

    if ( nPos < rString.getLength() &&
         (eScannedType == css::util::NumberFormat::DATE ||
          eScannedType == css::util::NumberFormat::DATETIME) )
    {
        // day of week is just parsed away
        sal_Int32 nOldPos = nPos;
        const OUString& rSep = pFormatter->GetLocaleData()->getLongDateDayOfWeekSep();
        if ( StringContains( rSep, rString, nPos ) )
        {
            nPos = nPos + rSep.getLength();
            SkipBlanks(rString, nPos);
        }
        int nTempDayOfWeek = GetDayOfWeek( rString, nPos );
        if ( nTempDayOfWeek )
        {
            if ( nPos < rString.getLength() )
            {
                if ( nTempDayOfWeek < 0 )
                {   // short
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
    if (nPos == 0 && eScannedType == css::util::NumberFormat::DATETIME &&
        rString.getLength() == 1 && rString[ 0 ] == (sal_Unicode)'Z' && MayBeIso8601())
    {
        // ISO 8601 timezone UTC yyyy-mm-ddThh:mmZ
        ++nPos;
    }
#endif

    if (nPos < rString.getLength()) // everything consumed?
    {
        // does input EndString equal EndString in Format?
        if ( !ScanStringNumFor( rString, nPos, pFormat, 0xFFFF ) )
        {
            return false;
        }
    }

    return true;
}


bool ImpSvNumberInputScan::ScanStringNumFor( const OUString& rString,       // String to scan
                                             sal_Int32 nPos,                // Position until which was consumed
                                             const SvNumberformat* pFormat, // The format to match
                                             sal_uInt16 nString,            // Substring of format, 0xFFFF => last
                                             bool bDontDetectNegation)      // Suppress sign detection
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
        // Don't try "lower" subformats ff the very first match was the second
        // or third subformat.
        nSub = nStringScanNumFor;
        do
        {   // Step through subformats, first positive, then negative, then
            // other, but not the last (text) subformat.
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
            // try remaining substring
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
            // simply negated twice? --1
            aString = comphelper::string::remove(aString, ' ');
            if ( (aString.getLength() == 1) && (aString[0] == '-') )
            {
                bFound = true;
                nStringScanSign = -1;
                nSub = 0; //! not 1
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
        // negative
        if ( nStringScanSign < 0 )
        {
            if ( (nSign < 0) && (nStringScanNumFor != 1) )
            {
                nStringScanSign = 1; // triple negated --1 yyy
            }
        }
        else if ( nStringScanSign == 0 )
        {
            if ( nSign < 0 )
            {   // nSign and nStringScanSign will be combined later,
                // flip sign if doubly negated
                if ( (nString == 0) && !bFirst &&
                     SvNumberformat::HasStringNegativeSign( aString ) )
                {
                    nStringScanSign = -1; // direct double negation
                }
                else if ( pFormat->IsNegativeWithoutSign() )
                {
                    nStringScanSign = -1; // indirect double negation
                }
            }
            else
            {
                nStringScanSign = -1;
            }
        }
        else    // > 0
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
bool ImpSvNumberInputScan::IsNumberFormatMain( const OUString& rString,        // string to be analyzed
                                               const SvNumberformat* pFormat ) // maybe number format set to match against
{
    Reset();
    NumberStringDivision( rString );             // breakdown into strings and numbers
    if (nAnzStrings >= SV_MAX_ANZ_INPUT_STRINGS) // too many elements
    {
        return false;                            // Njet, Nope, ...
    }
    if (nAnzNums == 0)                           // no number in input
    {
        if ( nAnzStrings > 0 )
        {
            // Here we may change the original, we don't need it anymore.
            // This saves copies and ToUpper() in GetLogical() and is faster.
            sStrArray[0] = comphelper::string::strip(sStrArray[0], ' ');
            OUString& rStrArray = sStrArray[0];
            nLogical = GetLogical( rStrArray );
            if ( nLogical )
            {
                eScannedType = css::util::NumberFormat::LOGICAL; // !!! it's a BOOLEAN
                nMatchedAllStrings &= ~nMatchedVirgin;
                return true;
            }
            else
            {
                return false;                   // simple text
            }
        }
        else
        {
            return false;                       // simple text
        }
    }

    sal_uInt16 i = 0;                           // mark any symbol
    sal_uInt16 j = 0;                           // mark only numbers

    switch ( nAnzNums )
    {
    case 1 :                                // Exactly 1 number in input
        // nAnzStrings >= 1
        if (GetNextNumber(i,j)) // i=1,0
        {   // Number at start
            if (eSetType == css::util::NumberFormat::FRACTION)  // Fraction 1 = 1/1
            {
                if (i >= nAnzStrings || // no end string nor decimal separator
                    sStrArray[i] == pFormatter->GetNumDecimalSep())
                {
                    eScannedType = css::util::NumberFormat::FRACTION;
                    nMatchedAllStrings &= ~nMatchedVirgin;
                    return true;
                }
            }
        }
        else
        {                                   // Analyze start string
            if (!ScanStartString( sStrArray[i], pFormat ))  // i=0
            {
                return false;               // already an error
            }
            i++;                            // next symbol, i=1
        }
        GetNextNumber(i,j);                 // i=1,2
        if (eSetType == css::util::NumberFormat::FRACTION)  // Fraction -1 = -1/1
        {
            if (nSign && !nNegCheck &&      // Sign +, -
                eScannedType == css::util::NumberFormat::UNDEFINED &&   // not date or currency
                nDecPos == 0 &&             // no previous decimal separator
                (i >= nAnzStrings ||        // no end string nor decimal separator
                 sStrArray[i] == pFormatter->GetNumDecimalSep())
                )
            {
                eScannedType = css::util::NumberFormat::FRACTION;
                nMatchedAllStrings &= ~nMatchedVirgin;
                return true;
            }
        }
        if (i < nAnzStrings && !ScanEndString( sStrArray[i], pFormat ))
        {
            return false;
        }
        break;
    case 2 :                                // Exactly 2 numbers in input
                                            // nAnzStrings >= 3
        if (!GetNextNumber(i,j))            // i=1,0
        {                                   // Analyze start string
            if (!ScanStartString( sStrArray[i], pFormat ))
                return false;               // already an error
            i++;                            // i=1
        }
        GetNextNumber(i,j);                 // i=1,2
        if ( !ScanMidString( sStrArray[i], i, pFormat ) )
        {
            return false;
        }
        i++;                                // next symbol, i=2,3
        GetNextNumber(i,j);                 // i=3,4
        if (i < nAnzStrings && !ScanEndString( sStrArray[i], pFormat ))
        {
            return false;
        }
        if (eSetType == css::util::NumberFormat::FRACTION)  // -1,200. as fraction
        {
            if (!nNegCheck  &&                  // no sign '('
                eScannedType == css::util::NumberFormat::UNDEFINED &&
                (nDecPos == 0 || nDecPos == 3)  // no decimal separator or at end
                )
            {
                eScannedType = css::util::NumberFormat::FRACTION;
                nMatchedAllStrings &= ~nMatchedVirgin;
                return true;
            }
        }
        break;
    case 3 :                                // Exactly 3 numbers in input
                                            // nAnzStrings >= 5
        if (!GetNextNumber(i,j))            // i=1,0
        {                                   // Analyze start string
            if (!ScanStartString( sStrArray[i], pFormat ))
            {
                return false;               // already an error
            }
            i++;                            // i=1
            if (nDecPos == 1)               // decimal separator at start => error
            {
                return false;
            }
        }
        GetNextNumber(i,j);                 // i=1,2
        if ( !ScanMidString( sStrArray[i], i, pFormat ) )
        {
            return false;
        }
        i++;                                // i=2,3
        if (eScannedType == css::util::NumberFormat::SCIENTIFIC)    // E only at end
        {
            return false;
        }
        GetNextNumber(i,j);                 // i=3,4
        if ( !ScanMidString( sStrArray[i], i, pFormat ) )
        {
            return false;
        }
        i++;                                // i=4,5
        GetNextNumber(i,j);                 // i=5,6
        if (i < nAnzStrings && !ScanEndString( sStrArray[i], pFormat ))
        {
            return false;
        }
        if (eSetType == css::util::NumberFormat::FRACTION)  // -1,200,100. as fraction
        {
            if (!nNegCheck  &&                  // no sign '('
                eScannedType == css::util::NumberFormat::UNDEFINED &&
                (nDecPos == 0 || nDecPos == 3)  // no decimal separator or at end
                )
            {
                eScannedType = css::util::NumberFormat::FRACTION;
                nMatchedAllStrings &= ~nMatchedVirgin;
                return true;
            }
        }
        if ( eScannedType == css::util::NumberFormat::FRACTION && nDecPos )
        {
            return false;                   // #36857# not a real fraction
        }
        break;
    default:                                // More than 3 numbers in input
                                            // nAnzStrings >= 7
        if (!GetNextNumber(i,j))            // i=1,0
        {                                   // Analyze startstring
            if (!ScanStartString( sStrArray[i], pFormat ))
                return false;               // already an error
            i++;                            // i=1
            if (nDecPos == 1)               // decimal separator at start => error
                return false;
        }
        GetNextNumber(i,j);                 // i=1,2
        if ( !ScanMidString( sStrArray[i], i, pFormat ) )
        {
            return false;
        }
        i++;                                // i=2,3
        {
            sal_uInt16 nThOld = 10;                 // just not 0 or 1
            while (nThOld != nThousand && j < nAnzNums-1) // Execute at least one time
                                                          // but leave one number.
            {                                             // Loop over group separators
                nThOld = nThousand;
                if (eScannedType == css::util::NumberFormat::SCIENTIFIC)    // E only at end
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
        if (eScannedType == css::util::NumberFormat::DATE ||    // long date or
            eScannedType == css::util::NumberFormat::TIME ||    // long time or
            eScannedType == css::util::NumberFormat::UNDEFINED) // long number
        {
            for (sal_uInt16 k = j; k < nAnzNums-1; k++)
            {
                if (eScannedType == css::util::NumberFormat::SCIENTIFIC)    // E only at endd
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
        if (eSetType == css::util::NumberFormat::FRACTION)  // -1,200,100. as fraction
        {
            if (!nNegCheck  &&                  // no sign '('
                eScannedType == css::util::NumberFormat::UNDEFINED &&
                (nDecPos == 0 || nDecPos == 3)  // no decimal separator or at end
                )
            {
                eScannedType = css::util::NumberFormat::FRACTION;
                nMatchedAllStrings &= ~nMatchedVirgin;
                return true;
            }
        }
        if ( eScannedType == css::util::NumberFormat::FRACTION && nDecPos )
        {
            return false;                       // #36857# not a real fraction
        }
        break;
    }

    if (eScannedType == css::util::NumberFormat::UNDEFINED)
    {
        nMatchedAllStrings &= ~nMatchedVirgin;
        // did match including nMatchedUsedAsReturn
        bool bDidMatch = (nMatchedAllStrings != 0);
        if ( nMatchedAllStrings )
        {
            bool bMatch = pFormat && pFormat->IsNumForStringElementCountEqual(
                               nStringScanNumFor, nAnzStrings, nAnzNums );
            if ( !bMatch )
            {
                nMatchedAllStrings = 0;
            }
        }
        if ( nMatchedAllStrings )
        {
            // A type DEFINED means that no category could be assigned to the
            // overall format because of mixed type subformats. Use the scan
            // matched subformat's type if any.
            short eForType = eSetType;
            if ((eForType == css::util::NumberFormat::UNDEFINED || eForType == css::util::NumberFormat::DEFINED) && pFormat)
                eForType = pFormat->GetNumForInfoScannedType( nStringScanNumFor);
            if (eForType != css::util::NumberFormat::UNDEFINED && eForType != css::util::NumberFormat::DEFINED)
                eScannedType = eForType;
            else
                eScannedType = css::util::NumberFormat::NUMBER;
        }
        else if ( bDidMatch )
        {
            return false;
        }
        else
        {
            eScannedType = css::util::NumberFormat::NUMBER;
            // everything else should have been recognized by now
        }
    }
    else if ( eScannedType == css::util::NumberFormat::DATE )
    {
        // the very relaxed date input checks may interfere with a preset format
        nMatchedAllStrings &= ~nMatchedVirgin;
        bool bWasReturn = ((nMatchedAllStrings & nMatchedUsedAsReturn) != 0);
        if ( nMatchedAllStrings )
        {
            bool bMatch = pFormat && pFormat->IsNumForStringElementCountEqual(
                               nStringScanNumFor, nAnzStrings, nAnzNums );
            if ( !bMatch )
            {
                nMatchedAllStrings = 0;
            }
        }
        if ( nMatchedAllStrings )
        {
            // A type DEFINED means that no category could be assigned to the
            // overall format because of mixed type subformats. Do not override
            // the scanned type in this case. Otherwise in IsNumberFormat() the
            // first numeric particle would be accepted as number.
            short eForType = eSetType;
            if ((eForType == css::util::NumberFormat::UNDEFINED || eForType == css::util::NumberFormat::DEFINED) && pFormat)
                eForType = pFormat->GetNumForInfoScannedType( nStringScanNumFor);
            if (eForType != css::util::NumberFormat::UNDEFINED && eForType != css::util::NumberFormat::DEFINED)
                eScannedType = eForType;
        }
        else if ( bWasReturn )
        {
            return false;
        }
    }
    else
    {
        nMatchedAllStrings = 0; // reset flag to no substrings matched
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
    css::uno::Sequence< css::i18n::CalendarItem2 > xElems = pCal->getMonths();
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
    aUpperCurrSymbol.clear();
    InvalidateDateAcceptancePatterns();
}


void ImpSvNumberInputScan::InvalidateDateAcceptancePatterns()
{
    if (sDateAcceptancePatterns.getLength())
    {
        sDateAcceptancePatterns = css::uno::Sequence< OUString >();
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
bool ImpSvNumberInputScan::IsNumberFormat( const OUString& rString,         // string to be analyzed
                                           short& F_Type,                   // IN: old type, OUT: new type
                                           double& fOutNumber,              // OUT: number if convertible
                                           const SvNumberformat* pFormat )  // maybe a number format to match against
{
    OUString aString;
    bool res; // return value
    sal_uInt16 k;
    eSetType = F_Type; // old type set

    if ( !rString.getLength() )
    {
        res = false;
    }
    else if (rString.getLength() > 308) // arbitrary
    {
        res = false;
    }
    else
    {
        // NoMoreUpperNeeded, all comparisons on UpperCase
        aString = pFormatter->GetCharClass()->uppercase( rString );
        // convert native number to ASCII if necessary
        TransformInput(pFormatter, aString);
        res = IsNumberFormatMain( aString, pFormat );
    }

    if (res)
    {
        if ( nNegCheck ||                             // ')' not found for '('
             (nSign && (eScannedType == css::util::NumberFormat::DATE ||
                        eScannedType == css::util::NumberFormat::DATETIME))) // signed date/datetime
        {
            res = false;
        }
        else
        {                                           // check count of partial number strings
            switch (eScannedType)
            {
            case css::util::NumberFormat::PERCENT:
            case css::util::NumberFormat::CURRENCY:
            case css::util::NumberFormat::NUMBER:
                if (nDecPos == 1)               // .05
                {
                    // matched MidStrings function like group separators
                    if ( nMatchedAllStrings )
                    {
                        nThousand = nAnzNums - 1;
                    }
                    else if ( nAnzNums != 1 )
                    {
                        res = false;
                    }
                }
                else if (nDecPos == 2)          // 1.05
                {
                    // matched MidStrings function like group separators
                    if ( nMatchedAllStrings )
                    {
                        nThousand = nAnzNums - 1;
                    }
                    else if ( nAnzNums != nThousand+2 )
                    {
                        res = false;
                    }
                }
                else                            // 1,100 or 1,100.
                {
                    // matched MidStrings function like group separators
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

            case css::util::NumberFormat::SCIENTIFIC:       // 1.0e-2
                if (nDecPos == 1)               // .05
                {
                    if (nAnzNums != 2)
                    {
                        res = false;
                    }
                }
                else if (nDecPos == 2)          // 1.05
                {
                    if (nAnzNums != nThousand+3)
                    {
                        res = false;
                    }
                }
                else                            // 1,100 or 1,100.
                {
                    if (nAnzNums != nThousand+2)
                    {
                        res = false;
                    }
                }
                break;

            case css::util::NumberFormat::DATE:
                if (nMonth < 0 && nDayOfWeek < 0 && nAnzNums == 3)
                {
                    // If both, short month name and day of week name were
                    // detected, and also numbers for full date, assume that we
                    // have a day of week instead of month name.
                    nMonth = 0;
                    nMonthPos = 0;
                }
                if (nMonth)
                {                               // month name and numbers
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
                        // Even if a date pattern was matched, for abbreviated
                        // pattern like "D.M." an input of "D.M. #" was
                        // accepted because # could had been a time. Here we do
                        // not have a combined date/time input though and #
                        // would be taken as Year in this example, which it is
                        // not. The count of numbers in pattern must match the
                        // count of numbers in input.
                        res = (GetDatePatternNumbers() == nAnzNums)
                            || MayBeIso8601() || nMatchedAllStrings;
                    }
                }
                break;

            case css::util::NumberFormat::TIME:
                if (nDecPos)
                {                               // hundredth seconds included
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

            case css::util::NumberFormat::DATETIME:
                if (nMonth < 0 && nDayOfWeek < 0 && nAnzNums >= 5)
                {
                    // If both, abbreviated month name and day of week name
                    // were detected, and also at least numbers for full date
                    // plus time including minutes, assume that we have a day
                    // of week instead of month name.
                    nMonth = 0;
                    nMonthPos = 0;
                }
                if (nMonth)
                {                               // month name and numbers
                    if (nDecPos)
                    {                           // hundredth seconds included
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
                    {                           // hundredth seconds included
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
            }   // switch
        }   // else
    }   // if (res)

    OUStringBuffer sResString;

    if (res)
    {                                       // we finally have a number
        switch (eScannedType)
        {
        case css::util::NumberFormat::LOGICAL:
            if (nLogical ==  1)
            {
                fOutNumber = 1.0;           // True
            }
            else if (nLogical == -1)
            {
                fOutNumber = 0.0;           // False
            }
            else
            {
                res = false;                // Oops
            }
            break;

        case css::util::NumberFormat::PERCENT:
        case css::util::NumberFormat::CURRENCY:
        case css::util::NumberFormat::NUMBER:
        case css::util::NumberFormat::SCIENTIFIC:
        case css::util::NumberFormat::DEFINED:          // if no category detected handle as number
            if ( nDecPos == 1 )             // . at start
            {
                sResString.append("0.");
            }

            for ( k = 0; k <= nThousand; k++)
            {
                sResString.append(sStrArray[nNums[k]]);  // integer part
            }
            if ( nDecPos == 2 && k < nAnzNums )     // . somewhere
            {
                sResString.append('.');
                sal_uInt16 nStop = (eScannedType == css::util::NumberFormat::SCIENTIFIC ?
                                    nAnzNums-1 : nAnzNums);
                for ( ; k < nStop; k++)
                {
                    sResString.append(sStrArray[nNums[k]]);  // fractional part
                }
            }

            if (eScannedType != css::util::NumberFormat::SCIENTIFIC)
            {
                fOutNumber = StringToDouble(sResString.makeStringAndClear());
            }
            else
            {                                           // append exponent
                sResString.append('E');
                if ( nESign == -1 )
                {
                    sResString.append('-');
                }
                sResString.append(sStrArray[nNums[nAnzNums-1]]);
                rtl_math_ConversionStatus eStatus;
                fOutNumber = ::rtl::math::stringToDouble( sResString.makeStringAndClear(), '.', ',', &eStatus );
                if ( eStatus == rtl_math_ConversionStatus_OutOfRange )
                {
                    F_Type = css::util::NumberFormat::TEXT;         // overflow/underflow -> Text
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

            if (eScannedType == css::util::NumberFormat::PERCENT)
            {
                fOutNumber/= 100.0;
            }
            break;

        case css::util::NumberFormat::FRACTION:
            if (nAnzNums == 1)
            {
                fOutNumber = StringToDouble(sStrArray[nNums[0]]);
            }
            else if (nAnzNums == 2)
            {
                if (nThousand == 1)
                {
                    sResString = sStrArray[nNums[0]];
                    sResString.append(sStrArray[nNums[1]]); // integer part
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
            else // nAnzNums > 2
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

        case css::util::NumberFormat::TIME:
            res = GetTimeRef(fOutNumber, 0, nAnzNums);
            if ( nSign < 0 )
            {
                fOutNumber = -fOutNumber;
            }
            break;

        case css::util::NumberFormat::DATE:
            res = GetDateRef( fOutNumber, k, pFormat );
            break;

        case css::util::NumberFormat::DATETIME:
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

    if (res) // overflow/underflow -> Text
    {
        if (fOutNumber < -DBL_MAX) // -1.7E308
        {
            F_Type = css::util::NumberFormat::TEXT;
            fOutNumber = -DBL_MAX;
            return true;
        }
        else if (fOutNumber >  DBL_MAX) // 1.7E308
        {
            F_Type = css::util::NumberFormat::TEXT;
            fOutNumber = DBL_MAX;
            return true;
        }
    }

    if (!res)
    {
        eScannedType = css::util::NumberFormat::TEXT;
        fOutNumber = 0.0;
    }

    F_Type = eScannedType;
    return res;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
