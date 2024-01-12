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

#include <cstdlib>
#include <dtoa.h>
#include <float.h>
#include <comphelper/string.hxx>
#include <o3tl/string_view.hxx>
#include <sal/log.hxx>
#include <tools/date.hxx>
#include <rtl/math.hxx>
#include <rtl/character.hxx>
#include <unotools/charclass.hxx>
#include <unotools/calendarwrapper.hxx>
#include <unotools/localedatawrapper.hxx>
#include <com/sun/star/i18n/CalendarFieldIndex.hpp>
#include <com/sun/star/i18n/LocaleCalendar2.hpp>
#include <unotools/digitgroupingiterator.hxx>
#include <comphelper/sequence.hxx>

#include <svl/zforlist.hxx>
#include "zforscan.hxx"
#include <svl/zformat.hxx>

#include <memory>

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

const sal_Unicode cNoBreakSpace = 0xA0;
const sal_Unicode cNarrowNoBreakSpace = 0x202F;
const bool kDefaultEra = true;     // Gregorian CE, positive year

ImpSvNumberInputScan::ImpSvNumberInputScan( SvNumberFormatter* pFormatterP )
        :
        bTextInitialized( false ),
        bScanGenitiveMonths( false ),
        bScanPartitiveMonths( false ),
        eScannedType( SvNumFormatType::UNDEFINED ),
        eSetType( SvNumFormatType::UNDEFINED )
{
    pFormatter = pFormatterP;
    moNullDate.emplace( 30,12,1899 );
    nYear2000 = SvNumberFormatter::GetYear2000Default();
    Reset();
    ChangeIntl();
}


ImpSvNumberInputScan::~ImpSvNumberInputScan()
{
}


void ImpSvNumberInputScan::Reset()
{
    mpFormat     = nullptr;
    nMonth       = 0;
    nMonthPos    = 0;
    nDayOfWeek   = 0;
    nTimePos     = 0;
    nSign        = 0;
    nESign       = 0;
    nDecPos      = 0;
    bNegCheck    = false;
    nStringsCnt  = 0;
    nNumericsCnt = 0;
    nThousand    = 0;
    eScannedType = SvNumFormatType::UNDEFINED;
    nAmPm        = 0;
    nPosThousandString = 0;
    nLogical     = 0;
    mbEraCE        = kDefaultEra;
    nStringScanNumFor = 0;
    nStringScanSign = 0;
    nMatchedAllStrings = nMatchedVirgin;
    nMayBeIso8601 = 0;
    bIso8601Tsep = false;
    nMayBeMonthDate = 0;
    nAcceptedDatePattern = -2;
    nDatePatternStart = 0;
    nDatePatternNumbers = 0;

    for (sal_uInt32 i = 0; i < SV_MAX_COUNT_INPUT_STRINGS; i++)
    {
        IsNum[i] = false;
        nNums[i] = 0;
    }
}

// native number transliteration if necessary
static void TransformInput( SvNumberFormatter const * pFormatter, OUString& rStr )
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
double ImpSvNumberInputScan::StringToDouble( std::u16string_view aStr, bool bForceFraction )
{
    std::unique_ptr<char[]> bufInHeap;
    constexpr int bufOnStackSize = 256;
    char bufOnStack[bufOnStackSize];
    char* buf = bufOnStack;
    const sal_Int32 bufsize = aStr.size() + (bForceFraction ? 2 : 1);
    if (bufsize > bufOnStackSize)
    {
        bufInHeap = std::make_unique<char[]>(bufsize);
        buf = bufInHeap.get();
    }
    char* p = buf;
    if (bForceFraction)
        *p++ = '.';
    for (size_t nPos = 0; nPos < aStr.size(); ++nPos)
    {
        sal_Unicode c = aStr[nPos];
        if (c == '.' || (c >= '0' && c <= '9'))
            *p++ = static_cast<char>(c);
        else
            break;
    }
    *p = '\0';

    return strtod_nolocale(buf, nullptr);
}

namespace {

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

}

bool ImpSvNumberInputScan::NextNumberStringSymbol( const sal_Unicode*& pStr,
                                                   OUString& rSymbol )
{
    bool isNumber = false;
    sal_Unicode cToken;
    ScanState eState = SsStart;
    const sal_Unicode* pHere = pStr;
    sal_Int32 nChars = 0;

    for (;;)
    {
        cToken = *pHere;
        if (cToken == 0 || eState == SsStop)
            break;
        pHere++;
        switch (eState)
        {
        case SsStart:
            if ( rtl::isAsciiDigit( cToken ) )
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
            if ( rtl::isAsciiDigit( cToken ) )
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
            if ( !rtl::isAsciiDigit( cToken ) )
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


// FIXME: should be grouping; it is only used though in case nStringsCnt is
// near SV_MAX_COUNT_INPUT_STRINGS, in NumberStringDivision().

bool ImpSvNumberInputScan::SkipThousands( const sal_Unicode*& pStr,
                                          OUString& rSymbol ) const
{
    bool res = false;
    OUStringBuffer sBuff(rSymbol);
    sal_Unicode cToken;
    const OUString& rThSep = pFormatter->GetNumThousandSep();
    const sal_Unicode* pHere = pStr;
    ScanState eState = SsStart;
    sal_Int32 nCounter = 0; // counts 3 digits

    for (;;)
    {
        cToken = *pHere;
        if (cToken == 0 || eState == SsStop)
            break;
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
            if ( rtl::isAsciiDigit( cToken ) )
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
    while ( pStr < pEnd && nStringsCnt < SV_MAX_COUNT_INPUT_STRINGS )
    {
        if ( NextNumberStringSymbol( pStr, sStrArray[nStringsCnt] ) )
        {   // Number
            IsNum[nStringsCnt] = true;
            nNums[nNumericsCnt] = nStringsCnt;
            nNumericsCnt++;
            if (nStringsCnt >= SV_MAX_COUNT_INPUT_STRINGS - 7 &&
                nPosThousandString == 0) // Only once
            {
                if ( SkipThousands( pStr, sStrArray[nStringsCnt] ) )
                {
                    nPosThousandString = nStringsCnt;
                }
            }
        }
        else
        {
            IsNum[nStringsCnt] = false;
        }
        nStringsCnt++;
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
                                               const OUString& rString, sal_Int32 nPos ) const
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
        rString.iterateCodePoints( &nIndex);
        if (nPos+1 < nIndex)
            return true;    // Surrogate, assume these to be new words.

        const sal_Int32 nType = pFormatter->GetCharClass()->getCharacterType( rString, nPos);
        using namespace ::com::sun::star::i18n;

        if ((nType & (KCharacterType::UPPER | KCharacterType::LOWER | KCharacterType::DIGIT)) != 0)
            return false;   // Alpha or numeric is not word gap.

        if (nType & KCharacterType::LETTER)
            return true;    // Letter other than alpha is new word. (Is it?)

        return true;        // Catch all remaining as gap until we know better.
    }

    return false;
}


/**
 * Skips the supplied char
 */
inline bool ImpSvNumberInputScan::SkipChar( sal_Unicode c, std::u16string_view rString,
                                            sal_Int32& nPos )
{
    if ((nPos < static_cast<sal_Int32>(rString.size())) && (rString[nPos] == c))
    {
        nPos++;
        return true;
    }
    return false;
}


/**
 * Skips blanks
 */
inline bool ImpSvNumberInputScan::SkipBlanks( const OUString& rString,
                                              sal_Int32& nPos )
{
    sal_Int32 nHere = nPos;
    if ( nPos < rString.getLength() )
    {
        const sal_Unicode* p = rString.getStr() + nPos;
        while ( *p == ' ' || *p == cNoBreakSpace || *p == cNarrowNoBreakSpace )
        {
            nPos++;
            p++;
        }
    }
    return nHere < nPos;
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
inline bool ImpSvNumberInputScan::GetThousandSep( std::u16string_view rString,
                                                  sal_Int32& nPos,
                                                  sal_uInt16 nStringPos ) const
{
    const OUString& rSep = pFormatter->GetNumThousandSep();
    // Is it an ordinary space instead of a no-break space?
    bool bSpaceBreak = (rSep[0] == cNoBreakSpace || rSep[0] == cNarrowNoBreakSpace) &&
        rString[0] == u' ' &&
        rSep.getLength() == 1 && rString.size() == 1;
    if (!((rString == rSep || bSpaceBreak) &&      // nothing else
           nStringPos < nStringsCnt - 1 &&         // safety first!
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
short ImpSvNumberInputScan::GetLogical( std::u16string_view rString ) const
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
            else if (i == 2 && pFormatter->GetLanguageTag().getLanguage() == "de")
            {
                if (pUpperAbbrevMonthText[i] == u"M\u00C4R" && StringContainsWord( "MRZ", rString, nPos))
                {   // Accept MRZ for MÄR
                    nPos = nPos + 3;
                    res = sal::static_int_cast< short >(-(i+1)); // negative
                    break;  // for
                }
                else if (pUpperAbbrevMonthText[i] == "MRZ" && StringContainsWord( u"M\u00C4R", rString, nPos))
                {   // And vice versa, accept MÄR for MRZ
                    nPos = nPos + 3;
                    res = sal::static_int_cast< short >(-(i+1)); // negative
                    break;  // for
                }
            }
            else if (i == 8)
            {
                // This assumes the weirdness is applicable to all locales.
                // It is the case for at least en-* and de-* locales.
                if (pUpperAbbrevMonthText[i] == "SEPT" && StringContainsWord( "SEP", rString, nPos))
                {   // #102136# The correct English form of month September abbreviated is
                    // SEPT, but almost every data contains SEP instead.
                    nPos = nPos + 3;
                    res = sal::static_int_cast< short >(-(i+1)); // negative
                    break;  // for
                }
                else if (pUpperAbbrevMonthText[i] == "SEP" && StringContainsWord( "SEPT", rString, nPos))
                {   // And vice versa, accept SEPT for SEP
                    nPos = nPos + 4;
                    res = sal::static_int_cast< short >(-(i+1)); // negative
                    break;  // for
                }
            }
        }
        if (!res)
        {
            // Brutal hack for German locales that know "Januar" or "Jänner".
            /* TODO: add alternative month names to locale data? if there are
             * more languages... */
            const LanguageTag& rLanguageTag = pFormatter->GetLanguageTag();
            if (rLanguageTag.getLanguage() == "de")
            {
                if (rLanguageTag.getCountry() == "AT")
                {
                    // Locale data has Jänner/Jän
                    assert(pUpperMonthText[0] == u"J\u00C4NNER");
                    if (StringContainsWord( "JANUAR", rString, nPos))
                    {
                        nPos += 6;
                        res = 1;
                    }
                    else if (StringContainsWord( "JAN", rString, nPos))
                    {
                        nPos += 3;
                        res = -1;
                    }
                }
                else
                {
                    // Locale data has Januar/Jan
                    assert(pUpperMonthText[0] == "JANUAR");
                    if (StringContainsWord( u"J\u00C4NNER", rString, nPos))
                    {
                        nPos += 6;
                        res = 1;
                    }
                    else if (StringContainsWord( u"J\u00C4N", rString, nPos))
                    {
                        nPos += 3;
                        res = -1;
                    }
                }
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
bool ImpSvNumberInputScan::GetCurrency( const OUString& rString, sal_Int32& nPos )
{
    if ( rString.getLength() > nPos )
    {
        if ( !aUpperCurrSymbol.getLength() )
        {   // If no format specified the currency of the currently active locale.
            LanguageType eLang = (mpFormat ? mpFormat->GetLanguage() :
                    pFormatter->GetLocaleData()->getLanguageTag().getLanguageType());
            aUpperCurrSymbol = pFormatter->GetCharClass()->uppercase(
                SvNumberFormatter::GetCurrencyEntry( eLang ).GetSymbol() );
        }
        if ( StringContains( aUpperCurrSymbol, rString, nPos ) )
        {
            nPos = nPos + aUpperCurrSymbol.getLength();
            return true;
        }
        if ( mpFormat )
        {
            OUString aSymbol, aExtension;
            if ( mpFormat->GetNewCurrencySymbol( aSymbol, aExtension ) )
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
inline bool ImpSvNumberInputScan::GetDecSep( std::u16string_view rString, sal_Int32& nPos ) const
{
    if ( static_cast<sal_Int32>(rString.size()) > nPos )
    {
        const OUString& rSep = pFormatter->GetNumDecimalSep();
        if ( o3tl::starts_with(rString.substr(nPos), rSep) )
        {
            nPos = nPos + rSep.getLength();
            return true;
        }
        const OUString& rSepAlt = pFormatter->GetNumDecimalSepAlt();
        if ( !rSepAlt.isEmpty() && o3tl::starts_with(rString.substr(nPos), rSepAlt) )
        {
            nPos = nPos + rSepAlt.getLength();
            return true;
        }
    }
    return false;
}


/**
 * Reading a hundredth seconds separator
 */
inline bool ImpSvNumberInputScan::GetTime100SecSep( std::u16string_view rString, sal_Int32& nPos ) const
{
    if ( static_cast<sal_Int32>(rString.size()) > nPos )
    {
        if (bIso8601Tsep)
        {
            // ISO 8601 specifies both '.' dot and ',' comma as fractional
            // separator.
            if (rString[nPos] == '.' || rString[nPos] == ',')
            {
                ++nPos;
                return true;
            }
        }
        // Even in an otherwise ISO 8601 string be lenient and accept the
        // locale defined separator.
        const OUString& rSep = pFormatter->GetLocaleData()->getTime100SecSep();
        if ( o3tl::starts_with(rString.substr(nPos), rSep))
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
 * u'−'   => -1
 *  '('   => -1, bNegCheck = 1
 * else =>  0
 */
int ImpSvNumberInputScan::GetSign( std::u16string_view rString, sal_Int32& nPos )
{
    if (static_cast<sal_Int32>(rString.size()) > nPos)
        switch (rString[ nPos ])
        {
        case '+':
            nPos++;
            return 1;
        case '(': // '(' similar to '-' ?!?
            bNegCheck = true;
            [[fallthrough]];
        case '-':
        // tdf#117037 - unicode minus (0x2212)
        case u'−':
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
short ImpSvNumberInputScan::GetESign( std::u16string_view rString, sal_Int32& nPos )
{
    if (static_cast<sal_Int32>(rString.size()) > nPos)
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
inline bool ImpSvNumberInputScan::GetNextNumber( sal_uInt16& i, sal_uInt16& j ) const
{
    if ( i < nStringsCnt && IsNum[i] )
    {
        j++;
        i++;
        return true;
    }
    return false;
}


bool ImpSvNumberInputScan::GetTimeRef( double& fOutNumber,
                                       sal_uInt16 nIndex, // j-value of the first numeric time part of input, default 0
                                       sal_uInt16 nCnt,   // count of numeric time parts
                                       SvNumInputOptions eInputOptions
                                     ) const
{
    bool bRet = true;
    sal_Int32 nHour;
    sal_Int32 nMinute = 0;
    sal_Int32 nSecond = 0;
    double fSecond100 = 0.0;
    sal_uInt16 nStartIndex = nIndex;

    if (nDecPos == 2 && (nCnt == 3 || nCnt == 2)) // 20:45.5 or 45.5
    {
        nHour = 0;
    }
    else if (mpFormat && nDecPos == 0 && nCnt == 2 && mpFormat->IsMinuteSecondFormat())
    {
        // Input on MM:SS format, instead of doing HH:MM:00
        nHour = 0;
    }
    else if (nIndex - nStartIndex < nCnt)
    {
        const OUString& rValStr = sStrArray[nNums[nIndex++]];
        nHour = rValStr.toInt32();
        if (nHour == 0 && rValStr != "0" && rValStr != "00")
            bRet = false;   // overflow -> Text
    }
    else
    {
        nHour = 0;
        bRet = false;
        SAL_WARN( "svl.numbers", "ImpSvNumberInputScan::GetTimeRef: bad number index");
    }

    // 0:123 or 0:0:123 or 0:123:59 is valid
    bool bAllowDuration = (nHour == 0 && !nAmPm);

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

    if (nDecPos == 2 && nCnt == 2) // 45.5
    {
        nMinute = 0;
    }
    else if (nIndex - nStartIndex < nCnt)
    {
        const OUString& rValStr = sStrArray[nNums[nIndex++]];
        nMinute = rValStr.toInt32();
        if (nMinute == 0 && rValStr != "0" && rValStr != "00")
            bRet = false;   // overflow -> Text
        if (!(eInputOptions & SvNumInputOptions::LAX_TIME) && !bAllowDuration
                && nIndex > 1 && nMinute > 59)
            bRet = false;   // 1:60 or 1:123 is invalid, 123:1 or 0:123 is valid
        if (bAllowDuration)
            bAllowDuration = (nMinute == 0);
    }
    if (nIndex - nStartIndex < nCnt)
    {
        const OUString& rValStr = sStrArray[nNums[nIndex++]];
        nSecond = rValStr.toInt32();
        if (nSecond == 0 && rValStr != "0" && rValStr != "00")
            bRet = false;   // overflow -> Text
        if (!(eInputOptions & SvNumInputOptions::LAX_TIME) && !bAllowDuration
                && nIndex > 1 && nSecond > 59 && !(nHour == 23 && nMinute == 59 && nSecond == 60))
            bRet = false;   // 1:60 or 1:123 or 1:1:123 is invalid, 123:1 or 123:1:1 or 0:0:123 is valid, or leap second
    }
    if (nIndex - nStartIndex < nCnt)
    {
        fSecond100 = StringToDouble( sStrArray[nNums[nIndex]], true );
    }
    fOutNumber = (static_cast<double>(nHour)*3600 +
                  static_cast<double>(nMinute)*60 +
                  static_cast<double>(nSecond) +
                  fSecond100)/86400.0;
    return bRet;
}


sal_uInt16 ImpSvNumberInputScan::ImplGetDay( sal_uInt16 nIndex ) const
{
    sal_uInt16 nRes = 0;

    if (sStrArray[nNums[nIndex]].getLength() <= 2)
    {
        sal_uInt16 nNum = static_cast<sal_uInt16>(sStrArray[nNums[nIndex]].toInt32());
        if (nNum <= 31)
        {
            nRes = nNum;
        }
    }

    return nRes;
}


sal_uInt16 ImpSvNumberInputScan::ImplGetMonth( sal_uInt16 nIndex ) const
{
    // Preset invalid month number
    sal_uInt16 nRes = pFormatter->GetCalendar()->getNumberOfMonthsInYear();

    if (sStrArray[nNums[nIndex]].getLength() <= 2)
    {
        sal_uInt16 nNum = static_cast<sal_uInt16>(sStrArray[nNums[nIndex]].toInt32());
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
    // 16-bit integer year width can have 5 digits, allow for one additional
    // leading zero as convention.
    if (nLen <= 6)
    {
        nYear = static_cast<sal_uInt16>(sStrArray[nNums[nIndex]].toInt32());
        // A year in another, not Gregorian CE era is never expanded.
        // A year < 100 entered with at least 3 digits with leading 0 is taken
        // as is without expansion.
        if (mbEraCE == kDefaultEra && nYear < 100 && nLen < 3)
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
        sal_Int32 nLen = ((nNumericsCnt >= 1 && nNums[0] < nStringsCnt) ? sStrArray[nNums[0]].getLength() : 0);
        if (nLen)
        {
            sal_Int32 n;
            if (nNumericsCnt >= 3 && nNums[2] < nStringsCnt &&
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


bool ImpSvNumberInputScan::CanForceToIso8601( DateOrder eDateOrder )
{
    int nCanForceToIso8601 = 0;
    if (!MayBeIso8601())
    {
        return false;
    }
    else if (nMayBeIso8601 >= 3)
    {
        return true;    // at least 3 digits in year
    }
    else
    {
        if (eDateOrder == DateOrder::Invalid)
        {
            // As if any of the cases below can be applied, but only if a
            // locale dependent date pattern was not matched.
            if ((GetDatePatternNumbers() == nNumericsCnt) && IsDatePatternNumberOfType(0,'Y'))
                return false;
            eDateOrder = GetDateOrder();
        }

        nCanForceToIso8601 = 1;
    }

    sal_Int32 n;
    switch (eDateOrder)
    {
        case DateOrder::DMY:               // "day" value out of range => ISO 8601 year
            n = sStrArray[nNums[0]].toInt32();
            if (n < 1 || n > 31)
            {
                nCanForceToIso8601 = 2;
            }
        break;
        case DateOrder::MDY:               // "month" value out of range => ISO 8601 year
            n = sStrArray[nNums[0]].toInt32();
            if (n < 1 || n > 12)
            {
                nCanForceToIso8601 = 2;
            }
        break;
        case DateOrder::YMD:               // always possible
            nCanForceToIso8601 = 2;
        break;
        default: break;
    }
    return nCanForceToIso8601 > 1;
}


bool ImpSvNumberInputScan::IsAcceptableIso8601()
{
    if (mpFormat && (mpFormat->GetType() & SvNumFormatType::DATE))
    {
        switch (pFormatter->GetEvalDateFormat())
        {
            case NF_EVALDATEFORMAT_INTL:
                return CanForceToIso8601( GetDateOrder());
            case NF_EVALDATEFORMAT_FORMAT:
                return CanForceToIso8601( mpFormat->GetDateOrder());
            default:
                return CanForceToIso8601( GetDateOrder()) || CanForceToIso8601( mpFormat->GetDateOrder());
        }
    }
    return CanForceToIso8601( GetDateOrder());
}


bool ImpSvNumberInputScan::MayBeMonthDate()
{
    if (nMayBeMonthDate == 0)
    {
        nMayBeMonthDate = 1;
        if (nNumericsCnt >= 2 && nNums[1] < nStringsCnt)
        {
            // "-Jan-"
            const OUString& rM = sStrArray[ nNums[ 0 ] + 1 ];
            if (rM.getLength() >= 3 && rM[0] == '-' && rM[ rM.getLength() - 1] == '-')
            {
                // Check year length assuming at least 3 digits (including
                // leading zero). Two digit years 1..31 are out of luck here
                // and may be taken as day of month.
                bool bYear1 = (sStrArray[nNums[0]].getLength() >= 3);
                bool bYear2 = (sStrArray[nNums[1]].getLength() >= 3);
                sal_Int32 n;
                bool bDay1 = !bYear1;
                if (bDay1)
                {
                    n = sStrArray[nNums[0]].toInt32();
                    bDay1 = n >= 1 && n <= 31;
                }
                bool bDay2 = !bYear2;
                if (bDay2)
                {
                    n = sStrArray[nNums[1]].toInt32();
                    bDay2 = n >= 1 && n <= 31;
                }

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


/** If a string is a separator plus '-' minus sign preceding a 'Y' year in
    a date pattern at position nPat.
 */
static bool lcl_IsSignedYearSep( std::u16string_view rStr, std::u16string_view rPat, sal_Int32 nPat )
{
    bool bOk = false;
    sal_Int32 nLen = rStr.size();
    if (nLen > 1 && rStr[nLen-1] == '-')
    {
        --nLen;
        if (nPat + nLen < static_cast<sal_Int32>(rPat.size()) && rPat[nPat+nLen] == 'Y')
        {
            // Signed year is possible.
            bOk = (rPat.find( rStr.substr( 0, nLen), nPat) == static_cast<size_t>(nPat));
        }
    }
    return bOk;
}


/** Length of separator usually is 1 but theoretically could be anything. */
static sal_Int32 lcl_getPatternSeparatorLength( std::u16string_view rPat, sal_Int32 nPat )
{
    sal_Int32 nSep = nPat;
    sal_Unicode c;
    while (nSep < static_cast<sal_Int32>(rPat.size()) && (c = rPat[nSep]) != 'D' && c != 'M' && c != 'Y')
        ++nSep;
    return nSep - nPat;
}


bool ImpSvNumberInputScan::IsAcceptedDatePattern( sal_uInt16 nStartPatternAt )
{
    if (nAcceptedDatePattern >= -1)
    {
        return (nAcceptedDatePattern >= 0);
    }
    if (!nNumericsCnt)
    {
        nAcceptedDatePattern = -1;
    }
    else if (!sDateAcceptancePatterns.hasElements())
    {
        // The current locale is the format's locale, if a format is present.
        const NfEvalDateFormat eEDF = pFormatter->GetEvalDateFormat();
        if (!mpFormat || eEDF == NF_EVALDATEFORMAT_FORMAT || mpFormat->GetLanguage() == pFormatter->GetLanguage())
        {
            sDateAcceptancePatterns = pFormatter->GetLocaleData()->getDateAcceptancePatterns();
        }
        else
        {
            OnDemandLocaleDataWrapper& xLocaleData = pFormatter->GetOnDemandLocaleDataWrapper(
                    SvNumberFormatter::InputScannerPrivateAccess());
            const LanguageTag aSaveLocale( xLocaleData->getLanguageTag() );
            assert(mpFormat->GetLanguage() == aSaveLocale.getLanguageType());   // prerequisite
            // Obtain formatter's locale's (e.g. system) patterns.
            xLocaleData.changeLocale( LanguageTag( pFormatter->GetLanguage()));
            const css::uno::Sequence<OUString> aLocalePatterns( xLocaleData->getDateAcceptancePatterns());
            // Reset to format's locale.
            xLocaleData.changeLocale( aSaveLocale);
            // When concatenating don't care about duplicates, combining
            // weeding those out reallocs yet another time and probably doesn't
            // take less time than looping over two additional patterns below...
            switch (eEDF)
            {
                case NF_EVALDATEFORMAT_FORMAT:
                    assert(!"shouldn't reach here");
                break;
                case NF_EVALDATEFORMAT_INTL:
                    sDateAcceptancePatterns = aLocalePatterns;
                break;
                case NF_EVALDATEFORMAT_INTL_FORMAT:
                    sDateAcceptancePatterns = comphelper::concatSequences(
                            aLocalePatterns,
                            xLocaleData->getDateAcceptancePatterns());
                break;
                case NF_EVALDATEFORMAT_FORMAT_INTL:
                    sDateAcceptancePatterns = comphelper::concatSequences(
                            xLocaleData->getDateAcceptancePatterns(),
                            aLocalePatterns);
                break;
            }
        }
        SAL_WARN_IF( !sDateAcceptancePatterns.hasElements(), "svl.numbers", "ImpSvNumberInputScan::IsAcceptedDatePattern: no date acceptance patterns");
        nAcceptedDatePattern = (sDateAcceptancePatterns.hasElements() ? -2 : -1);
    }

    if (nAcceptedDatePattern == -1)
    {
        return false;
    }
    nDatePatternStart = nStartPatternAt; // remember start particle

    const sal_Int32 nMonthsInYear = pFormatter->GetCalendar()->getNumberOfMonthsInYear();

    for (sal_Int32 nPattern=0; nPattern < sDateAcceptancePatterns.getLength(); ++nPattern)
    {
        const OUString& rPat = sDateAcceptancePatterns[nPattern];
        if (rPat.getLength() == 3)
        {
            // Ignore a pattern that would match numeric input with decimal
            // separator. It may had been read from configuration or resulted
            // from the locales' patterns concatenation above.
            if (    rPat[1] == pFormatter->GetLocaleData()->getNumDecimalSep().toChar()
                 || rPat[1] == pFormatter->GetLocaleData()->getNumDecimalSepAlt().toChar())
            {
                SAL_WARN("svl.numbers", "ignoring date acceptance pattern with decimal separator ambiguity: " << rPat);
                continue;   // for, next pattern
            }
        }
        sal_uInt16 nNext = nDatePatternStart;
        nDatePatternNumbers = 0;
        bool bOk = true;
        sal_Int32 nPat = 0;
        for ( ; nPat < rPat.getLength() && bOk && nNext < nStringsCnt; ++nPat, ++nNext)
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
                    const sal_Int32 nSepLen = lcl_getPatternSeparatorLength( rPat, nPat);
                    // Non-numeric input must match separator exactly to be
                    // accepted as such.
                    const sal_Int32 nLen = sStrArray[nNext].getLength();
                    bOk = (nLen == nSepLen && rPat.indexOf( sStrArray[nNext], nPat) == nPat);
                    if (bOk)
                    {
                        nPat += nLen - 1;
                    }
                    else if ((bOk = lcl_IsSignedYearSep( sStrArray[nNext], rPat, nPat)))
                    {
                        nPat += nLen - 2;
                    }
                    else if (nPat + nLen > rPat.getLength() && sStrArray[nNext][ nLen - 1 ] == ' ')
                    {
                        using namespace comphelper::string;
                        // Trailing blanks in input.
                        OUStringBuffer aBuf(sStrArray[nNext]);
                        aBuf.stripEnd();
                        // Expand again in case of pattern "M. D. " and
                        // input "M. D.  ", maybe fetched far, but...
                        padToLength(aBuf, rPat.getLength() - nPat, ' ');
                        bOk = (rPat.indexOf( aBuf, nPat) == nPat);
                        if (bOk)
                        {
                            nPat += aBuf.getLength() - 1;
                        }
                    }
                }
                break;
            }
        }
        if (bOk)
        {
            // Check for trailing characters mismatch.
            if (nNext < nStringsCnt)
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
                                    c = rPat[--nPatCheck];
                                } while (c != 'Y' && c != 'M' && c != 'D' && nPatCheck > 0);
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
                    // No blank and a following number is not.
                    const bool bBlanks = SkipBlanks( sStrArray[nCheck], nPos);
                    if (nPos == sStrArray[nCheck].getLength() && (bBlanks || !IsNum[nNext]))
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


bool ImpSvNumberInputScan::SkipDatePatternSeparator( sal_uInt16 nParticle, sal_Int32 & rPos, bool & rSignedYear )
{
    // If not initialized yet start with first number, if any.
    if (!IsAcceptedDatePattern( nNumericsCnt ? nNums[0] : 0 ))
    {
        return false;
    }
    if (nParticle < nDatePatternStart || nParticle >= nStringsCnt || IsNum[nParticle])
    {
        return false;
    }
    sal_uInt16 nNext = nDatePatternStart;
    const OUString& rPat = sDateAcceptancePatterns[nAcceptedDatePattern];
    for (sal_Int32 nPat = 0; nPat < rPat.getLength() && nNext < nStringsCnt; ++nPat, ++nNext)
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
                const sal_Int32 nSepLen = lcl_getPatternSeparatorLength( rPat, nPat);
                const sal_Int32 nLen = sStrArray[nNext].getLength();
                bool bOk = (nLen == nSepLen && rPat.indexOf( sStrArray[nNext], nPat) == nPat);
                if (!bOk)
                {
                    bOk = lcl_IsSignedYearSep( sStrArray[nNext], rPat, nPat);
                    if (bOk)
                        rSignedYear = true;
                }
                if (!bOk && (nPat + nLen > rPat.getLength() && sStrArray[nNext][ nLen - 1 ] == ' '))
                {
                    // The same ugly trailing blanks check as in
                    // IsAcceptedDatePattern().
                    using namespace comphelper::string;
                    OUStringBuffer aBuf(sStrArray[nNext]);
                    aBuf.stripEnd();
                    padToLength(aBuf, rPat.getLength() - nPat, ' ');
                    bOk = (rPat.indexOf(aBuf, nPat) == nPat);
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
    if (!IsAcceptedDatePattern( nNumericsCnt ? nNums[0] : 0 ))
    {
        return 0;
    }
    return nDatePatternNumbers;
}


bool ImpSvNumberInputScan::IsDatePatternNumberOfType( sal_uInt16 nNumber, sal_Unicode cType )
{
    if (GetDatePatternNumbers() <= nNumber)
        return false;

    sal_uInt16 nNum = 0;
    const OUString& rPat = sDateAcceptancePatterns[nAcceptedDatePattern];
    for (sal_Int32 nPat = 0; nPat < rPat.getLength(); ++nPat)
    {
        switch (rPat[nPat])
        {
            case 'Y':
            case 'M':
            case 'D':
                if (nNum == nNumber)
                    return rPat[nPat] == cType;
                ++nNum;
            break;
        }
    }
    return false;
}


sal_uInt32 ImpSvNumberInputScan::GetDatePatternOrder()
{
    // If not initialized yet start with first number, if any.
    if (!IsAcceptedDatePattern( nNumericsCnt ? nNums[0] : 0 ))
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


DateOrder ImpSvNumberInputScan::GetDateOrder( bool bFromFormatIfNoPattern )
{
    sal_uInt32 nOrder = GetDatePatternOrder();
    if (!nOrder)
    {
        if (bFromFormatIfNoPattern && mpFormat)
            return mpFormat->GetDateOrder();
        else
            return pFormatter->GetLocaleData()->getDateOrder();
    }
    switch ((nOrder & 0xff0000) >> 16)
    {
    case 'Y':
        if ((((nOrder & 0xff00) >> 8) == 'M') && ((nOrder & 0xff) == 'D'))
        {
            return DateOrder::YMD;
        }
        break;
    case 'M':
        if ((((nOrder & 0xff00) >> 8) == 'D') && ((nOrder & 0xff) == 'Y'))
        {
            return DateOrder::MDY;
        }
        break;
    case 'D':
        if ((((nOrder & 0xff00) >> 8) == 'M') && ((nOrder & 0xff) == 'Y'))
        {
            return DateOrder::DMY;
        }
        break;
    default:
    case 0:
        switch ((nOrder & 0xff00) >> 8)
        {
        case 'Y':
            switch (nOrder & 0xff)
            {
            case 'M':
                return DateOrder::YMD;
            }
            break;
        case 'M':
            switch (nOrder & 0xff)
            {
            case 'Y':
                return DateOrder::DMY;
            case 'D':
                return DateOrder::MDY;
            }
            break;
        case 'D':
            switch (nOrder & 0xff)
            {
            case 'Y':
                return DateOrder::MDY;
            case 'M':
                return DateOrder::DMY;
            }
            break;
        default:
        case 0:
            switch (nOrder & 0xff)
            {
            case 'Y':
                return DateOrder::YMD;
            case 'M':
                return DateOrder::MDY;
            case 'D':
                return DateOrder::DMY;
            }
            break;
        }
    }
    SAL_WARN( "svl.numbers", "ImpSvNumberInputScan::GetDateOrder: undefined, falling back to locale's default");
    return pFormatter->GetLocaleData()->getDateOrder();
}

LongDateOrder ImpSvNumberInputScan::GetMiddleMonthLongDateOrder( bool bFormatTurn,
                                                                 const LocaleDataWrapper* pLoc,
                                                                 DateOrder eDateOrder )
{
    if (MayBeMonthDate())
        return (nMayBeMonthDate == 2) ? LongDateOrder::DMY : LongDateOrder::YMD;

    LongDateOrder eLDO;
    const sal_uInt32 nExactDateOrder = (bFormatTurn ? mpFormat->GetExactDateOrder() : 0);
    if (!nExactDateOrder)
        eLDO = pLoc->getLongDateOrder();
    else if ((((nExactDateOrder >> 16) & 0xff) == 'Y') && ((nExactDateOrder & 0xff) == 'D'))
        eLDO = LongDateOrder::YMD;
    else if ((((nExactDateOrder >> 16) & 0xff) == 'D') && ((nExactDateOrder & 0xff) == 'Y'))
        eLDO = LongDateOrder::DMY;
    else
        eLDO = pLoc->getLongDateOrder();
    if (eLDO != LongDateOrder::YMD && eLDO != LongDateOrder::DMY)
    {
        switch (eDateOrder)
        {
            case DateOrder::YMD:
                eLDO = LongDateOrder::YMD;
            break;
            case DateOrder::DMY:
                eLDO = LongDateOrder::DMY;
            break;
            default:
                ;   // nothing, not a date
        }
    }
    else if (eLDO == LongDateOrder::DMY && eDateOrder == DateOrder::YMD)
    {
        // Check possible order and maybe switch.
        if (!ImplGetDay(0) && ImplGetDay(1))
            eLDO = LongDateOrder::YMD;
    }
    else if (eLDO == LongDateOrder::YMD && eDateOrder == DateOrder::DMY)
    {
        // Check possible order and maybe switch.
        if (!ImplGetDay(1) && ImplGetDay(0))
            eLDO = LongDateOrder::DMY;
    }
    return eLDO;
}

bool ImpSvNumberInputScan::GetDateRef( double& fDays, sal_uInt16& nCounter )
{
    using namespace ::com::sun::star::i18n;
    NfEvalDateFormat eEDF;
    int nFormatOrder;
    if ( mpFormat && (mpFormat->GetType() & SvNumFormatType::DATE) )
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
        DateOrder DateFmt;
        bool bFormatTurn;
        switch ( eEDF )
        {
        case NF_EVALDATEFORMAT_INTL :
            bFormatTurn = false;
            DateFmt = GetDateOrder();
            break;
        case NF_EVALDATEFORMAT_FORMAT :
            bFormatTurn = true;
            DateFmt = mpFormat->GetDateOrder();
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
                DateFmt = mpFormat->GetDateOrder();
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
                // Even if the format pattern is to be preferred, the input may
                // have matched a pattern of the current locale, which then
                // again is to be preferred. Both date orders can be different
                // so we need to obtain the actual match. For example ISO
                // YYYY-MM-DD format vs locale's DD.MM.YY input.
                // If no pattern was matched, obtain from format.
                // Note that patterns may have been constructed from the
                // format's locale and prepended to the current locale's
                // patterns, it doesn't necessarily mean a current locale's
                // pattern was matched, but may if the format's locale's
                // patterns didn't match, which were tried first.
                DateFmt = GetDateOrder(true);
            }
            break;
        default:
            SAL_WARN( "svl.numbers", "ImpSvNumberInputScan::GetDateRef: unknown NfEvalDateFormat" );
            DateFmt = DateOrder::YMD;
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
            if ( mpFormat->IsOtherCalendar( nStringScanNumFor ) )
            {
                mpFormat->SwitchToOtherCalendar( aOrgCalendar, fOrgDateTime );
            }
            else
            {
                mpFormat->SwitchToSpecifiedCalendar( aOrgCalendar, fOrgDateTime,
                                                    nStringScanNumFor );
            }
#endif
        }

        res = true;
        nCounter = 0;
        // For incomplete dates, always assume first day of month if not specified.
        pCal->setValue( CalendarFieldIndex::DAY_OF_MONTH, 1 );

        switch (nNumericsCnt) // count of numbers in string
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
                                         mpFormat->GetExactDateOrder() :
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
                case DateOrder::MDY:
                case DateOrder::YMD:
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
                case DateOrder::DMY:
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
                case DateOrder::DMY:
                    pCal->setValue( CalendarFieldIndex::DAY_OF_MONTH, ImplGetDay(0) );
                    break;
                case DateOrder::YMD:
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
                                              mpFormat->GetExactDateOrder() :
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
                    case DateOrder::MDY:
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
                    case DateOrder::DMY:
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
                    case DateOrder::YMD:
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
                sal_uInt32 nExactDateOrder = (bFormatTurn ? mpFormat->GetExactDateOrder() : 0);
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
                const LongDateOrder eLDO = GetMiddleMonthLongDateOrder( bFormatTurn, pLoc, DateFmt);
                switch (eLDO)
                {
                case LongDateOrder::DMY:
                    pCal->setValue( CalendarFieldIndex::DAY_OF_MONTH, ImplGetDay(0) );
                    pCal->setValue( CalendarFieldIndex::YEAR, ImplGetYear(1) );
                    break;
                case LongDateOrder::YMD:
                    pCal->setValue( CalendarFieldIndex::DAY_OF_MONTH, ImplGetDay(1) );
                    pCal->setValue( CalendarFieldIndex::YEAR, ImplGetYear(0) );
                    break;
                default:
                    res = false;
                    break;
                }
                break;
            }
            case 3:             // month at the end (94 10 Jan)
                if (pLoc->getLongDateOrder() != LongDateOrder::YDM)
                    res = false;
                else
                {
                    pCal->setValue( CalendarFieldIndex::DAY_OF_MONTH, ImplGetDay(1) );
                    pCal->setValue( CalendarFieldIndex::MONTH, std::abs(nMonth)-1 );
                    pCal->setValue( CalendarFieldIndex::YEAR, ImplGetYear(0) );
                }
                break;
            default:
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
                    for ( sal_uInt16 j = 0; j < nNumericsCnt; j++ )
                    {
                        if ( nNums[j] == nTimePos - 2 )
                        {
                            nCounter = j;
                            break; // for
                        }
                    }
                }
                // ISO 8601 yyyy-mm-dd forced recognition
                DateOrder eDF = (CanForceToIso8601( DateFmt) ? DateOrder::YMD : DateFmt);
                switch (eDF)
                {
                case DateOrder::MDY:
                    pCal->setValue( CalendarFieldIndex::DAY_OF_MONTH, ImplGetDay(1) );
                    pCal->setValue( CalendarFieldIndex::MONTH, ImplGetMonth(0) );
                    if ( nCounter > 2 )
                        pCal->setValue( CalendarFieldIndex::YEAR, ImplGetYear(2) );
                    break;
                case DateOrder::DMY:
                    pCal->setValue( CalendarFieldIndex::DAY_OF_MONTH, ImplGetDay(0) );
                    pCal->setValue( CalendarFieldIndex::MONTH, ImplGetMonth(1) );
                    if ( nCounter > 2 )
                        pCal->setValue( CalendarFieldIndex::YEAR, ImplGetYear(2) );
                    break;
                case DateOrder::YMD:
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
            {
                nCounter = 2;
                // The input is valid as MDY in almost any
                // constellation, there is no date order (M)YD except if
                // set in a format applied.
                pCal->setValue( CalendarFieldIndex::MONTH, std::abs(nMonth)-1 );
                sal_uInt32 nExactDateOrder = (bFormatTurn ? mpFormat->GetExactDateOrder() : 0);
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
            case 2:             // month in the middle (10 Jan 94 8:23)
            {
                nCounter = 2;
                pCal->setValue( CalendarFieldIndex::MONTH, std::abs(nMonth)-1 );
                const LongDateOrder eLDO = GetMiddleMonthLongDateOrder( bFormatTurn, pLoc, DateFmt);
                switch (eLDO)
                {
                case LongDateOrder::DMY:
                    pCal->setValue( CalendarFieldIndex::DAY_OF_MONTH, ImplGetDay(0) );
                    pCal->setValue( CalendarFieldIndex::YEAR, ImplGetYear(1) );
                    break;
                case LongDateOrder::YMD:
                    pCal->setValue( CalendarFieldIndex::DAY_OF_MONTH, ImplGetDay(1) );
                    pCal->setValue( CalendarFieldIndex::YEAR, ImplGetYear(0) );
                    break;
                default:
                    res = false;
                    break;
                }
                break;
            }
            case 3:            // month at the end (94 10 Jan 8:23)
                nCounter = 2;
                if (pLoc->getLongDateOrder() != LongDateOrder::YDM)
                    res = false;
                else
                {
                    pCal->setValue( CalendarFieldIndex::DAY_OF_MONTH, ImplGetDay(1) );
                    pCal->setValue( CalendarFieldIndex::MONTH, std::abs(nMonth)-1 );
                    pCal->setValue( CalendarFieldIndex::YEAR, ImplGetYear(0) );
                }
                break;
            default:
                nCounter = 2;
                res = false;
                break;
            }   // switch (nMonthPos)
            break;
        }   // switch (nNumericsCnt)

        if (mbEraCE != kDefaultEra)
            pCal->setValue( CalendarFieldIndex::ERA, mbEraCE ? 1 : 0);

        if ( res && pCal->isValid() )
        {
            double fDiff = DateTime(*moNullDate) - pCal->getEpochStart();
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
bool ImpSvNumberInputScan::ScanStartString( const OUString& rString )
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
        if ( ScanStringNumFor( rString, nPos, 0, true ) )
        {
            nMatchedAllStrings |= nMatchedStartString;
        }
        else
        {
            nMatchedAllStrings = 0;
        }
    }

    // Bail out early for just a sign.
    if (nSign && nPos == rString.getLength())
        return true;

    const sal_Int32 nStartBlanks = nPos;
    if ( GetDecSep(rString, nPos) )                 // decimal separator in start string
    {
        if (SkipBlanks(rString, nPos))
            nPos = nStartBlanks;                    // `. 2` not a decimal separator
        else
            nDecPos = 1;                            // leading decimal separator
    }
    else if ( GetCurrency(rString, nPos) )          // currency (DM 1)?
    {
        eScannedType = SvNumFormatType::CURRENCY;       // !!! it IS currency !!!
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
            if (SkipBlanks(rString, nPos))
            {
                nPos = nStartBlanks;                // `DM . 2` not a decimal separator
                eScannedType = SvNumFormatType::UNDEFINED;  // !!! it is NOT currency !!!
            }
            else
                nDecPos = 1;                        // leading decimal separator
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
            if (nPos < rString.getLength() || (nStringsCnt >= 4 && nNumericsCnt >= 2))
            {
                eScannedType = SvNumFormatType::DATE;   // !!! it IS a date !!!
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
                eScannedType = SvNumFormatType::DATE;       // !!! it IS a date !!!
                if ( nPos < rString.getLength() )
                {
                    if ( nTempDayOfWeek < 0 )
                    {
                        // abbreviated
                        if ( rString[ nPos ] == '.' )
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
                        if (nPos < rString.getLength() || (nStringsCnt >= 4 && nNumericsCnt >= 2))
                        {
                            nMonth = nTempMonth;
                            nMonthPos = 1; // month at the beginning
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
        // Skip one trailing '-' or '/' character to recognize June-2007
        if (nMonth && nPos + 1 == rString.getLength())
        {
            SkipChar('-', rString, nPos) || SkipChar('/', rString, nPos);
        }
    }

    if (nPos < rString.getLength()) // not everything consumed
    {
        // Does input StartString equal StartString of format?
        // This time with sign detection!
        if ( !ScanStringNumFor( rString, nPos, 0 ) )
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
bool ImpSvNumberInputScan::ScanMidString( const OUString& rString, sal_uInt16 nStringPos, sal_uInt16 nCurNumCount )
{
    sal_Int32 nPos = 0;
    SvNumFormatType eOldScannedType = eScannedType;

    if ( nMatchedAllStrings )
    {   // Match against format in any case, so later on for a "1-2-3-4" input
        // we may distinguish between a y-m-d (or similar) date and a 0-0-0-0
        // format.
        if ( ScanStringNumFor( rString, 0, nStringPos ) )
        {
            nMatchedAllStrings |= nMatchedMidString;
        }
        else
        {
            nMatchedAllStrings = 0;
        }
    }

    const sal_Int32 nStartBlanks = nPos;
    const bool bBlanks = SkipBlanks(rString, nPos);
    if (GetDecSep(rString, nPos))                   // decimal separator?
    {
        if (nDecPos == 1 || nDecPos == 3)           // .12.4 or 1.E2.1
        {
            return MatchedReturn();
        }
        else if (nDecPos == 2)                      // . dup: 12.4.
        {
            bool bSignedYear = false;
            if (bDecSepInDateSeps ||                // . also date separator
                SkipDatePatternSeparator( nStringPos, nPos, bSignedYear))
            {
                if ( eScannedType != SvNumFormatType::UNDEFINED &&
                     eScannedType != SvNumFormatType::DATE &&
                     eScannedType != SvNumFormatType::DATETIME)  // already another type
                {
                    return MatchedReturn();
                }
                if (eScannedType == SvNumFormatType::UNDEFINED)
                {
                    eScannedType = SvNumFormatType::DATE; // !!! it IS a date
                }
                SkipBlanks(rString, nPos);
            }
            else
            {
                return MatchedReturn();
            }
        }
        else if (bBlanks)
        {
            // `1 .2` or `1 . 2` not a decimal separator, reset
            nPos = nStartBlanks;
        }
        else if (SkipBlanks(rString, nPos))
        {
            // `1. 2` not a decimal separator, reset
            nPos = nStartBlanks;
        }
        else
        {
            nDecPos = 2;                            // . in mid string
        }
    }
    else if ( (eScannedType & SvNumFormatType::TIME) &&
              GetTime100SecSep( rString, nPos ) )
    {                                               // hundredth seconds separator
        if ( nDecPos )
        {
            return MatchedReturn();
        }
        nDecPos = 2;                                // . in mid string

        // If this is exactly an ISO 8601 fractional seconds separator, bail
        // out early to not get confused by later checks for group separator or
        // other.
        if (bIso8601Tsep && nPos == rString.getLength() &&
                eScannedType == SvNumFormatType::DATETIME && (rString == "." || rString == ","))
            return true;

        SkipBlanks(rString, nPos);
    }

    if (SkipChar('/', rString, nPos))               // fraction?
    {
        if ( eScannedType != SvNumFormatType::UNDEFINED &&  // already another type
             eScannedType != SvNumFormatType::DATE)         // except date
        {
            return MatchedReturn();                         // => jan/31/1994
        }
        else if (eScannedType != SvNumFormatType::DATE &&   // analyzed no date until now
                 (eSetType == SvNumFormatType::FRACTION ||  // and preset was fraction
                  (nNumericsCnt == 3 &&                     // or 3 numbers
                   (nStringPos == 3 ||                      // and 4th string particle
                    (nStringPos == 4 && nSign)) &&          // or 5th if signed
                   sStrArray[nStringPos-2].indexOf('/') == -1)))  // and not 23/11/1999
                                                                  // that was not accepted as date yet
        {
            SkipBlanks(rString, nPos);
            if (nPos == rString.getLength())
            {
                eScannedType = SvNumFormatType::FRACTION;   // !!! it IS a fraction (so far)
                if (eSetType == SvNumFormatType::FRACTION &&
                    nNumericsCnt == 2 &&
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
        if ( eScannedType != SvNumFormatType::UNDEFINED &&   // already another type
             eScannedType != SvNumFormatType::CURRENCY)      // except currency
        {
            return MatchedReturn();
        }
        nThousand++;
    }

    const LocaleDataWrapper* pLoc = pFormatter->GetLocaleData();
    bool bSignedYear = false;
    bool bDate = SkipDatePatternSeparator( nStringPos, nPos, bSignedYear);   // 12/31  31.12.  12/31/1999  31.12.1999
    if (!bDate)
    {
        const OUString& rDate = pFormatter->GetDateSep();
        SkipBlanks(rString, nPos);
        bDate = SkipString( rDate, rString, nPos);      // 10.  10-  10/
    }
    if (!bDate && nStringPos == 1 && mpFormat && (mpFormat->GetType() & SvNumFormatType::DATE))
    {
        // If a DMY format was given and a mid string starts with a literal
        // ". " dot+space and could contain a following month name and ends
        // with a space or LongDateMonthSeparator, like it's scanned in
        // `14". AUG "18`, then it may be a date as well. Regardless whether
        // defined such by the locale or not.
        // This *could* check for presence of ". "MMM or ". "MMMM in the actual
        // format code for further restriction to match only if present, but..

        const sal_uInt32 nExactDateOrder = mpFormat->GetExactDateOrder();
        // Exactly DMY.
        if (((nExactDateOrder & 0xff) == 'Y') && (((nExactDateOrder >> 8) & 0xff) == 'M')
                && (((nExactDateOrder >> 16) & 0xff) == 'D'))
        {
            const sal_Int32 nTmpPos = nPos;
            if (SkipChar('.', rString, nPos) && SkipBlanks(rString, nPos) && nPos + 2 < rString.getLength()
                    && (rString.endsWith(" ") || rString.endsWith( pLoc->getLongDateMonthSep())))
                bDate = true;
            else
                nPos = nTmpPos;
        }
    }
    if (bDate || ((MayBeIso8601() || MayBeMonthDate()) &&    // 1999-12-31  31-Dec-1999
                  SkipChar( '-', rString, nPos)))
    {
        if ( eScannedType != SvNumFormatType::UNDEFINED &&  // already another type
             eScannedType != SvNumFormatType::DATE)       // except date
        {
            return MatchedReturn();
        }
        SkipBlanks(rString, nPos);
        eScannedType = SvNumFormatType::DATE;           // !!! it IS a date
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
        if (bSignedYear)
        {
            if (mbEraCE != kDefaultEra)               // signed year twice?
                return MatchedReturn();

            mbEraCE = false;  // BCE
        }
    }

    const sal_Int32 nMonthStart = nPos;
    short nTempMonth = GetMonth(rString, nPos);     // month in the middle (10 Jan 94) or at the end (94 10 Jan)
    if (nTempMonth)
    {
        if (nMonth != 0)                            // month dup
        {
            return MatchedReturn();
        }
        if ( eScannedType != SvNumFormatType::UNDEFINED &&  // already another type
             eScannedType != SvNumFormatType::DATE)         // except date
        {
            return MatchedReturn();
        }
        if (nMonthStart > 0 && nPos < rString.getLength())  // 10Jan or Jan94 without separator are not dates
        {
            eScannedType = SvNumFormatType::DATE;       // !!! it IS a date
            nMonth = nTempMonth;
            if (nCurNumCount <= 1)
                nMonthPos = 2;                      // month in the middle
            else
                nMonthPos = 3;                      // month at the end
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
        if (eScannedType != SvNumFormatType::UNDEFINED) // already another type
        {
            return MatchedReturn();
        }
        else
        {
            SkipBlanks(rString, nPos);
            eScannedType = SvNumFormatType::SCIENTIFIC; // !!! it IS scientific
            if ( nThousand+2 == nNumericsCnt && nDecPos == 2 ) // special case 1.E2
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
                if ( eScannedType != SvNumFormatType::DATE &&    // already another type than date
                     eScannedType != SvNumFormatType::DATETIME)  // or date time
                {
                    return MatchedReturn();
                }
                if (eScannedType == SvNumFormatType::DATE)
                {
                    nDecPos = 0;                    // reset for time transition
                }
            }
            else
            {
                return MatchedReturn();
            }
        }
        if ((eScannedType == SvNumFormatType::DATE ||        // already date type
             eScannedType == SvNumFormatType::DATETIME) &&   // or date time
            nNumericsCnt > 3)                                // and more than 3 numbers? (31.Dez.94 8:23)
        {
            SkipBlanks(rString, nPos);
            eScannedType = SvNumFormatType::DATETIME;   // !!! it IS date with time
        }
        else if ( eScannedType != SvNumFormatType::UNDEFINED &&  // already another type
                  eScannedType != SvNumFormatType::TIME)         // except time
        {
            return MatchedReturn();
        }
        else
        {
            SkipBlanks(rString, nPos);
            eScannedType = SvNumFormatType::TIME;       // !!! it IS a time
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
        case SvNumFormatType::DATE:
            if (nMonthPos == 1 && pLoc->getLongDateOrder() == LongDateOrder::MDY)
            {
                // #68232# recognize long date separators like ", " in "September 5, 1999"
                if (SkipString( pLoc->getLongDateDaySep(), rString, nPos ))
                {
                    SkipBlanks( rString, nPos );
                }
            }
            else if (nPos == 0 && rString.getLength() == 1 && MayBeIso8601())
            {
                if (    (nStringPos == 5 && rString[0] == 'T') ||
                        (nStringPos == 6 && rString[0] == 'T' && sStrArray[0] == "-"))
                {
                    // ISO 8601 combined date and time, yyyy-mm-ddThh:mm or -yyyy-mm-ddThh:mm
                    ++nPos;
                    bIso8601Tsep = true;
                }
                else if (nStringPos == 7 && rString[0] == ':')
                {
                    // ISO 8601 combined date and time, the time part; we reach
                    // here if the locale's separator is not ':' so it couldn't
                    // be detected above in the time block.
                    if (nNumericsCnt >= 5)
                        eScannedType = SvNumFormatType::DATETIME;
                    ++nPos;
                }
            }
            break;
        case SvNumFormatType::DATETIME:
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
                    if (nStringPos == nStringsCnt - 2 ||
                        nStringPos == nStringsCnt - 4)
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
                        nStringPos == nStringsCnt - 2)
                    {
                        ++nPos;     // yyyy-mm-ddThh:mm[:ss]+xx:yy
                    }
                    break;
                }
            }
#endif
            break;
        default: break;
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
bool ImpSvNumberInputScan::ScanEndString( const OUString& rString )
{
    sal_Int32 nPos = 0;

    if ( nMatchedAllStrings )
    {   // Match against format in any case, so later on for a "1-2-3-4" input
        // we may distinguish between a y-m-d (or similar) date and a 0-0-0-0
        // format.
        if ( ScanStringNumFor( rString, 0, 0xFFFF ) )
        {
            nMatchedAllStrings |= nMatchedEndString;
        }
        else
        {
            nMatchedAllStrings = 0;
        }
    }

    const sal_Int32 nStartBlanks = nPos;
    const bool bBlanks = SkipBlanks(rString, nPos);
    if (GetDecSep(rString, nPos))                   // decimal separator?
    {
        if (nDecPos == 1 || nDecPos == 3)           // .12.4 or 12.E4.
        {
            return MatchedReturn();
        }
        else if (nDecPos == 2)                      // . dup: 12.4.
        {
            bool bSignedYear = false;
            if (bDecSepInDateSeps ||                // . also date separator
                SkipDatePatternSeparator( nStringsCnt-1, nPos, bSignedYear))
            {
                if ( eScannedType != SvNumFormatType::UNDEFINED &&
                     eScannedType != SvNumFormatType::DATE &&
                     eScannedType != SvNumFormatType::DATETIME)  // already another type
                {
                    return MatchedReturn();
                }
                if (eScannedType == SvNumFormatType::UNDEFINED)
                {
                    eScannedType = SvNumFormatType::DATE;   // !!! it IS a date
                }
                SkipBlanks(rString, nPos);
            }
            else
            {
                return MatchedReturn();
            }
        }
        else if (bBlanks)
        {
            // not a decimal separator, reset
            nPos = nStartBlanks;
        }
        else
        {
            nDecPos = 3;                            // . in end string
            SkipBlanks(rString, nPos);
        }
    }

    bool bSignDetectedHere = false;
    if ( nSign == 0  &&                             // conflict - not signed
         eScannedType != SvNumFormatType::DATE)         // and not date
                                                    //!? catch time too?
    {                                               // not signed yet
        nSign = GetSign(rString, nPos);             // 1- DM
        if (bNegCheck)                              // '(' as sign
        {
            return MatchedReturn();
        }
        if (nSign)
        {
            bSignDetectedHere = true;
        }
    }

    SkipBlanks(rString, nPos);
    if (bNegCheck && SkipChar(')', rString, nPos))  // skip ')' if appropriate
    {
        bNegCheck = false;
        SkipBlanks(rString, nPos);
    }

    if ( GetCurrency(rString, nPos) )               // currency symbol?
    {
        if (eScannedType != SvNumFormatType::UNDEFINED) // currency dup
        {
            return MatchedReturn();
        }
        else
        {
            SkipBlanks(rString, nPos);
            eScannedType = SvNumFormatType::CURRENCY;
        }                                           // behind currency a '-' is allowed
        if (nSign == 0)                             // not signed yet
        {
            nSign = GetSign(rString, nPos);         // DM -
            SkipBlanks(rString, nPos);
            if (bNegCheck)                          // 3 DM (
            {
                return MatchedReturn();
            }
        }
        if ( bNegCheck && eScannedType == SvNumFormatType::CURRENCY &&
             SkipChar(')', rString, nPos) )
        {
            bNegCheck = false;                          // ')' skipped
            SkipBlanks(rString, nPos);              // only if currency
        }
    }

    if ( SkipChar('%', rString, nPos) )             // 1%
    {
        if (eScannedType != SvNumFormatType::UNDEFINED) // already another type
        {
            return MatchedReturn();
        }
        SkipBlanks(rString, nPos);
        eScannedType = SvNumFormatType::PERCENT;
    }

    const LocaleDataWrapper* pLoc = pFormatter->GetLocaleData();
    const OUString& rTime = pLoc->getTimeSep();
    if ( SkipString(rTime, rString, nPos) )         // 10:
    {
        if (nDecPos)                                // already , => error
        {
            return MatchedReturn();
        }
        if (eScannedType == SvNumFormatType::DATE && nNumericsCnt > 2) // 31.Dez.94 8:
        {
            SkipBlanks(rString, nPos);
            eScannedType = SvNumFormatType::DATETIME;
        }
        else if (eScannedType != SvNumFormatType::UNDEFINED &&
                 eScannedType != SvNumFormatType::TIME) // already another type
        {
            return MatchedReturn();
        }
        else
        {
            SkipBlanks(rString, nPos);
            eScannedType = SvNumFormatType::TIME;
        }
        if ( !nTimePos )
        {
            nTimePos = nStringsCnt;
        }
    }

    bool bSignedYear = false;
    bool bDate = SkipDatePatternSeparator( nStringsCnt-1, nPos, bSignedYear);   // 12/31  31.12.  12/31/1999  31.12.1999
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
        if (eScannedType != SvNumFormatType::UNDEFINED &&
            eScannedType != SvNumFormatType::DATE)          // already another type
        {
            return MatchedReturn();
        }
        else
        {
            SkipBlanks(rString, nPos);
            eScannedType = SvNumFormatType::DATE;
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
        if (eScannedType != SvNumFormatType::UNDEFINED &&
            eScannedType != SvNumFormatType::DATE)      // already another type
        {
            return MatchedReturn();
        }
        if (nMonthStart > 0)                        // 10Jan without separator is not a date
        {
            eScannedType = SvNumFormatType::DATE;
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
        if (eScannedType != SvNumFormatType::UNDEFINED &&
            eScannedType != SvNumFormatType::TIME &&
            eScannedType != SvNumFormatType::DATETIME)  // already another type
        {
            return MatchedReturn();
        }
        else
        {
            // If not already scanned as time, 6.78am does not result in 6
            // seconds and 78 hundredths in the morning. Keep as suffix.
            if (eScannedType != SvNumFormatType::TIME && nDecPos == 2 && nNumericsCnt == 2)
            {
                nPos = nOrigPos; // rewind am/pm
            }
            else
            {
                SkipBlanks(rString, nPos);
                if ( eScannedType != SvNumFormatType::DATETIME )
                {
                    eScannedType = SvNumFormatType::TIME;
                }
            }
        }
    }

    if ( bNegCheck && SkipChar(')', rString, nPos) )
    {
        if (eScannedType == SvNumFormatType::CURRENCY)  // only if currency
        {
            bNegCheck = false;                          // skip ')'
            SkipBlanks(rString, nPos);
        }
        else
        {
            return MatchedReturn();
        }
    }

    if ( nPos < rString.getLength() &&
         (eScannedType == SvNumFormatType::DATE ||
          eScannedType == SvNumFormatType::DATETIME) )
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
                    if ( rString[ nPos ] == '.' )
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
    if (nPos == 0 && eScannedType == SvNumFormatType::DATETIME &&
        rString.getLength() == 1 && rString[ 0 ] == 'Z' && MayBeIso8601())
    {
        // ISO 8601 timezone UTC yyyy-mm-ddThh:mmZ
        ++nPos;
    }
#endif

    if (nPos < rString.getLength()) // everything consumed?
    {
        // does input EndString equal EndString in Format?
        if ( !ScanStringNumFor( rString, nPos, 0xFFFF ) )
        {
            return false;
        }
    }

    return true;
}


bool ImpSvNumberInputScan::ScanStringNumFor( const OUString& rString,       // String to scan
                                             sal_Int32 nPos,                // Position until which was consumed
                                             sal_uInt16 nString,            // Substring of format, 0xFFFF => last
                                             bool bDontDetectNegation)      // Suppress sign detection
{
    if ( !mpFormat )
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
            pStr = mpFormat->GetNumForString( nSub, nString, true );
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
             !bFirst && (nSign < 0) && mpFormat->IsSecondSubformatRealNegative() )
        {
            // simply negated twice? --1
            aString = aString.replaceAll(" ", "");
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
              mpFormat->IsSecondSubformatRealNegative() )
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
                else if ( mpFormat->IsNegativeWithoutSign() )
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
    mpFormat = pFormat;
    NumberStringDivision( rString );             // breakdown into strings and numbers
    if (nStringsCnt >= SV_MAX_COUNT_INPUT_STRINGS) // too many elements
    {
        return false;                            // Njet, Nope, ...
    }
    if (nNumericsCnt == 0)                           // no number in input
    {
        if ( nStringsCnt > 0 )
        {
            // Here we may change the original, we don't need it anymore.
            // This saves copies and ToUpper() in GetLogical() and is faster.
            sStrArray[0] = comphelper::string::strip(sStrArray[0], ' ');
            OUString& rStrArray = sStrArray[0];
            nLogical = GetLogical( rStrArray );
            if ( nLogical )
            {
                eScannedType = SvNumFormatType::LOGICAL; // !!! it's a BOOLEAN
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

    switch ( nNumericsCnt )
    {
    case 1 :                                // Exactly 1 number in input
        // nStringsCnt >= 1
        if (GetNextNumber(i,j)) // i=1,0
        {   // Number at start
            if (eSetType == SvNumFormatType::FRACTION)  // Fraction 1 = 1/1
            {
                if (i >= nStringsCnt || // no end string nor decimal separator
                    pFormatter->IsDecimalSep( sStrArray[i]))
                {
                    eScannedType = SvNumFormatType::FRACTION;
                    nMatchedAllStrings &= ~nMatchedVirgin;
                    return true;
                }
            }
        }
        else
        {                                   // Analyze start string
            if (!ScanStartString( sStrArray[i] ))  // i=0
            {
                return false;               // already an error
            }
            i++;                            // next symbol, i=1
        }
        GetNextNumber(i,j);                 // i=1,2
        if (eSetType == SvNumFormatType::FRACTION)  // Fraction -1 = -1/1
        {
            if (nSign && !bNegCheck &&      // Sign +, -
                eScannedType == SvNumFormatType::UNDEFINED &&   // not date or currency
                nDecPos == 0 &&             // no previous decimal separator
                (i >= nStringsCnt ||        // no end string nor decimal separator
                 pFormatter->IsDecimalSep( sStrArray[i]))
                )
            {
                eScannedType = SvNumFormatType::FRACTION;
                nMatchedAllStrings &= ~nMatchedVirgin;
                return true;
            }
        }
        if (i < nStringsCnt && !ScanEndString( sStrArray[i] ))
        {
            return false;
        }
        break;
    case 2 :                                // Exactly 2 numbers in input
                                            // nStringsCnt >= 3
        if (!GetNextNumber(i,j))            // i=1,0
        {                                   // Analyze start string
            if (!ScanStartString( sStrArray[i] ))
                return false;               // already an error
            i++;                            // i=1
        }
        GetNextNumber(i,j);                 // i=1,2
        if ( !ScanMidString( sStrArray[i], i, j ) )
        {
            return false;
        }
        i++;                                // next symbol, i=2,3
        GetNextNumber(i,j);                 // i=3,4
        if (i < nStringsCnt && !ScanEndString( sStrArray[i] ))
        {
            return false;
        }
        if (eSetType == SvNumFormatType::FRACTION)  // -1,200. as fraction
        {
            if (!bNegCheck  &&                  // no sign '('
                eScannedType == SvNumFormatType::UNDEFINED &&
                (nDecPos == 0 || nDecPos == 3)  // no decimal separator or at end
                )
            {
                eScannedType = SvNumFormatType::FRACTION;
                nMatchedAllStrings &= ~nMatchedVirgin;
                return true;
            }
        }
        break;
    case 3 :                                // Exactly 3 numbers in input
                                            // nStringsCnt >= 5
        if (!GetNextNumber(i,j))            // i=1,0
        {                                   // Analyze start string
            if (!ScanStartString( sStrArray[i] ))
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
        if ( !ScanMidString( sStrArray[i], i, j ) )
        {
            return false;
        }
        i++;                                // i=2,3
        if (eScannedType == SvNumFormatType::SCIENTIFIC)    // E only at end
        {
            return false;
        }
        GetNextNumber(i,j);                 // i=3,4
        if ( !ScanMidString( sStrArray[i], i, j ) )
        {
            return false;
        }
        i++;                                // i=4,5
        GetNextNumber(i,j);                 // i=5,6
        if (i < nStringsCnt && !ScanEndString( sStrArray[i] ))
        {
            return false;
        }
        if (eSetType == SvNumFormatType::FRACTION)  // -1,200,100. as fraction
        {
            if (!bNegCheck  &&                  // no sign '('
                eScannedType == SvNumFormatType::UNDEFINED &&
                (nDecPos == 0 || nDecPos == 3)  // no decimal separator or at end
                )
            {
                eScannedType = SvNumFormatType::FRACTION;
                nMatchedAllStrings &= ~nMatchedVirgin;
                return true;
            }
        }
        if ( eScannedType == SvNumFormatType::FRACTION && nDecPos )
        {
            return false;                   // #36857# not a real fraction
        }
        break;
    default:                                // More than 3 numbers in input
                                            // nStringsCnt >= 7
        if (!GetNextNumber(i,j))            // i=1,0
        {                                   // Analyze startstring
            if (!ScanStartString( sStrArray[i] ))
                return false;               // already an error
            i++;                            // i=1
            if (nDecPos == 1)               // decimal separator at start => error
                return false;
        }
        GetNextNumber(i,j);                 // i=1,2
        if ( !ScanMidString( sStrArray[i], i, j ) )
        {
            return false;
        }
        i++;                                // i=2,3
        {
            sal_uInt16 nThOld = 10;                 // just not 0 or 1
            while (nThOld != nThousand && j < nNumericsCnt-1) // Execute at least one time
                                                          // but leave one number.
            {                                             // Loop over group separators
                nThOld = nThousand;
                if (eScannedType == SvNumFormatType::SCIENTIFIC)    // E only at end
                {
                    return false;
                }
                GetNextNumber(i,j);
                if ( i < nStringsCnt && !ScanMidString( sStrArray[i], i, j ) )
                {
                    return false;
                }
                i++;
            }
        }
        if (eScannedType == SvNumFormatType::DATE ||    // long date or
            eScannedType == SvNumFormatType::TIME ||    // long time or
            eScannedType == SvNumFormatType::UNDEFINED) // long number
        {
            for (sal_uInt16 k = j; k < nNumericsCnt-1; k++)
            {
                if (eScannedType == SvNumFormatType::SCIENTIFIC)    // E only at endd
                {
                    return false;
                }
                GetNextNumber(i,j);
                if ( i < nStringsCnt && !ScanMidString( sStrArray[i], i, j ) )
                {
                    return false;
                }
                i++;
            }
        }
        GetNextNumber(i,j);
        if (i < nStringsCnt && !ScanEndString( sStrArray[i] ))
        {
            return false;
        }
        if (eSetType == SvNumFormatType::FRACTION)  // -1,200,100. as fraction
        {
            if (!bNegCheck  &&                  // no sign '('
                eScannedType == SvNumFormatType::UNDEFINED &&
                (nDecPos == 0 || nDecPos == 3)  // no decimal separator or at end
                )
            {
                eScannedType = SvNumFormatType::FRACTION;
                nMatchedAllStrings &= ~nMatchedVirgin;
                return true;
            }
        }
        if ( eScannedType == SvNumFormatType::FRACTION && nDecPos )
        {
            return false;                       // #36857# not a real fraction
        }
        break;
    }

    if (eScannedType == SvNumFormatType::UNDEFINED)
    {
        nMatchedAllStrings &= ~nMatchedVirgin;
        // did match including nMatchedUsedAsReturn
        bool bDidMatch = (nMatchedAllStrings != 0);
        if ( nMatchedAllStrings )
        {
            bool bMatch = mpFormat && mpFormat->IsNumForStringElementCountEqual(
                               nStringScanNumFor, nStringsCnt, nNumericsCnt );
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
            SvNumFormatType eForType = eSetType;
            if ((eForType == SvNumFormatType::UNDEFINED || eForType == SvNumFormatType::DEFINED) && mpFormat)
                eForType = mpFormat->GetNumForInfoScannedType( nStringScanNumFor);
            if (eForType != SvNumFormatType::UNDEFINED && eForType != SvNumFormatType::DEFINED)
                eScannedType = eForType;
            else
                eScannedType = SvNumFormatType::NUMBER;
        }
        else if ( bDidMatch )
        {
            // Accept a plain fractional number like 123.45 as there may be a
            // decimal separator also present as literal like in a 0"."0 weirdo
            // format.
            if (nDecPos != 2 || nNumericsCnt != 2)
                return false;
            eScannedType = SvNumFormatType::NUMBER;
        }
        else
        {
            eScannedType = SvNumFormatType::NUMBER;
            // everything else should have been recognized by now
        }
    }
    else if ( eScannedType == SvNumFormatType::DATE )
    {
        // the very relaxed date input checks may interfere with a preset format
        nMatchedAllStrings &= ~nMatchedVirgin;
        bool bWasReturn = ((nMatchedAllStrings & nMatchedUsedAsReturn) != 0);
        if ( nMatchedAllStrings )
        {
            bool bMatch = mpFormat && mpFormat->IsNumForStringElementCountEqual(
                               nStringScanNumFor, nStringsCnt, nNumericsCnt );
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
            SvNumFormatType eForType = eSetType;
            if ((eForType == SvNumFormatType::UNDEFINED || eForType == SvNumFormatType::DEFINED) && mpFormat)
                eForType = mpFormat->GetNumForInfoScannedType( nStringScanNumFor);
            if (eForType != SvNumFormatType::UNDEFINED && eForType != SvNumFormatType::DEFINED)
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

    pUpperMonthText.reset();
    pUpperAbbrevMonthText.reset();
    css::uno::Sequence< css::i18n::CalendarItem2 > xElems = pCal->getMonths();
    nElems = xElems.getLength();
    pUpperMonthText.reset( new OUString[nElems] );
    pUpperAbbrevMonthText.reset( new OUString[nElems] );
    for ( j = 0; j < nElems; j++ )
    {
        pUpperMonthText[j] = pChrCls->uppercase( xElems[j].FullName );
        pUpperAbbrevMonthText[j] = pChrCls->uppercase( xElems[j].AbbrevName );
    }

    pUpperGenitiveMonthText.reset();
    pUpperGenitiveAbbrevMonthText.reset();
    xElems = pCal->getGenitiveMonths();
    bScanGenitiveMonths = (nElems != xElems.getLength());
    nElems = xElems.getLength();
    pUpperGenitiveMonthText.reset( new OUString[nElems] );
    pUpperGenitiveAbbrevMonthText.reset( new OUString[nElems] );
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

    pUpperPartitiveMonthText.reset();
    pUpperPartitiveAbbrevMonthText.reset();
    xElems = pCal->getPartitiveMonths();
    bScanPartitiveMonths = (nElems != xElems.getLength());
    nElems = xElems.getLength();
    pUpperPartitiveMonthText.reset( new OUString[nElems] );
    pUpperPartitiveAbbrevMonthText.reset( new OUString[nElems] );
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

    pUpperDayText.reset();
    pUpperAbbrevDayText.reset();
    xElems = pCal->getDays();
    nElems = xElems.getLength();
    pUpperDayText.reset( new OUString[nElems] );
    pUpperAbbrevDayText.reset( new OUString[nElems] );
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
    bDecSepInDateSeps = ( cDecSep == '-' ||
                          cDecSep == pFormatter->GetDateSep()[0] );
    if (!bDecSepInDateSeps)
    {
        sal_Unicode cDecSepAlt = pFormatter->GetNumDecimalSepAlt().toChar();
        bDecSepInDateSeps = cDecSepAlt && (cDecSepAlt == '-' || cDecSepAlt == pFormatter->GetDateSep()[0]);
    }
    bTextInitialized = false;
    aUpperCurrSymbol.clear();
    InvalidateDateAcceptancePatterns();
}


void ImpSvNumberInputScan::InvalidateDateAcceptancePatterns()
{
    if (sDateAcceptancePatterns.hasElements())
    {
        sDateAcceptancePatterns = css::uno::Sequence< OUString >();
    }
}


void ImpSvNumberInputScan::ChangeNullDate( const sal_uInt16 Day,
                                           const sal_uInt16 Month,
                                           const sal_Int16 Year )
{
    moNullDate = Date(Day, Month, Year);
}


/**
 * Does rString represent a number (also date, time et al)
 */
bool ImpSvNumberInputScan::IsNumberFormat( const OUString& rString,         // string to be analyzed
                                           SvNumFormatType& F_Type,         // IN: old type, OUT: new type
                                           double& fOutNumber,              // OUT: number if convertible
                                           const SvNumberformat* pFormat,   // maybe a number format to match against
                                           SvNumInputOptions eInputOptions )
{
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
        OUString aString = pFormatter->GetCharClass()->uppercase( rString );
        // convert native number to ASCII if necessary
        TransformInput(pFormatter, aString);
        res = IsNumberFormatMain( aString, pFormat );
    }

    if (res)
    {
        // Accept signed date only for ISO date with at least four digits in
        // year to not have an input of -M-D-Y arbitrarily recognized. The
        // final order is only determined in GetDateRef().
        // Also accept for Y/M/D date pattern match, i.e. if the first number
        // is year.
        // Accept only if the year immediately follows the sign character with
        // no space in between.
        if (nSign && (eScannedType == SvNumFormatType::DATE ||
                      eScannedType == SvNumFormatType::DATETIME) && mbEraCE == kDefaultEra &&
                (IsDatePatternNumberOfType(0,'Y') || (MayBeIso8601() && sStrArray[nNums[0]].getLength() >= 4)))
        {
            const sal_Unicode c = sStrArray[0][sStrArray[0].getLength()-1];
            if (c == '-' || c == '+')
            {
                // A '+' sign doesn't change the era.
                if (nSign < 0)
                    mbEraCE = false;  // BCE
                nSign = 0;
            }
        }
        if ( bNegCheck ||                             // ')' not found for '('
             (nSign && (eScannedType == SvNumFormatType::DATE ||
                        eScannedType == SvNumFormatType::DATETIME))) // signed date/datetime
        {
            res = false;
        }
        else
        {                                           // check count of partial number strings
            switch (eScannedType)
            {
            case SvNumFormatType::PERCENT:
            case SvNumFormatType::CURRENCY:
            case SvNumFormatType::NUMBER:
                if (nDecPos == 1)               // .05
                {
                    // Matched MidStrings function like group separators, but
                    // there can't be an integer part numeric input, so
                    // effectively 0 thousands groups.
                    if ( nMatchedAllStrings )
                    {
                        nThousand = 0;
                    }
                    else if ( nNumericsCnt != 1 )
                    {
                        res = false;
                    }
                }
                else if (nDecPos == 2)          // 1.05
                {
                    // Matched MidStrings function like group separators, but
                    // let a decimal separator override a literal separator
                    // string; like 0"." with input 123.45
                    if ( nMatchedAllStrings )
                    {
                        if (nNumericsCnt == 2)
                            nThousand = 0;
                        else
                        {
                            // Assume that if there was a decimal separator
                            // matching also a literal string then it was the
                            // last. We could find the last possible match to
                            // support literals in fractions, but really..
                            nThousand = nNumericsCnt - 1;
                        }
                    }
                    else if ( nNumericsCnt != nThousand+2 )
                    {
                        res = false;
                    }
                }
                else                            // 1,100 or 1,100.
                {
                    // matched MidStrings function like group separators
                    if ( nMatchedAllStrings )
                    {
                        nThousand = nNumericsCnt - 1;
                    }
                    else if ( nNumericsCnt != nThousand+1 )
                    {
                        res = false;
                    }
                }
                break;

            case SvNumFormatType::SCIENTIFIC:       // 1.0e-2
                if (nDecPos == 1)               // .05
                {
                    if (nNumericsCnt != 2)
                    {
                        res = false;
                    }
                }
                else if (nDecPos == 2)          // 1.05
                {
                    if (nNumericsCnt != nThousand+3)
                    {
                        res = false;
                    }
                }
                else                            // 1,100 or 1,100.
                {
                    if (nNumericsCnt != nThousand+2)
                    {
                        res = false;
                    }
                }
                break;

            case SvNumFormatType::DATE:
                if (nMonth < 0 && nDayOfWeek < 0 && nNumericsCnt == 3)
                {
                    // If both, short month name and day of week name were
                    // detected, and also numbers for full date, assume that we
                    // have a day of week instead of month name.
                    nMonth = 0;
                    nMonthPos = 0;
                }
                if (nMonth)
                {                               // month name and numbers
                    if (nNumericsCnt > 2)
                    {
                        res = false;
                    }
                }
                else
                {
                    if (nNumericsCnt > 3)
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
                        res = (GetDatePatternNumbers() == nNumericsCnt)
                            || IsAcceptableIso8601() || nMatchedAllStrings;
                    }
                }
                break;

            case SvNumFormatType::TIME:
                if (nDecPos)
                {                               // hundredth seconds included
                    if (nNumericsCnt > 4)
                    {
                        res = false;
                    }
                }
                else
                {
                    if (nNumericsCnt > 3)
                    {
                        res = false;
                    }
                }
                break;

            case SvNumFormatType::DATETIME:
                if (nMonth < 0 && nDayOfWeek < 0 && nNumericsCnt >= 5)
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
                        if (nNumericsCnt > 6)
                        {
                            res = false;
                        }
                    }
                    else
                    {
                        if (nNumericsCnt > 5)
                        {
                            res = false;
                        }
                    }
                }
                else
                {
                    if (nDecPos)
                    {                           // hundredth seconds included
                        if (nNumericsCnt > 7)
                        {
                            res = false;
                        }
                    }
                    else
                    {
                        if (nNumericsCnt > 6)
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
        case SvNumFormatType::LOGICAL:
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

        case SvNumFormatType::PERCENT:
        case SvNumFormatType::CURRENCY:
        case SvNumFormatType::NUMBER:
        case SvNumFormatType::SCIENTIFIC:
        case SvNumFormatType::DEFINED:          // if no category detected handle as number
            if ( nDecPos == 1 )             // . at start
            {
                sResString.append("0.");
            }

            for ( k = 0; k <= nThousand; k++)
            {
                sResString.append(sStrArray[nNums[k]]);  // integer part
            }
            if ( nDecPos == 2 && k < nNumericsCnt )     // . somewhere
            {
                sResString.append('.');
                sal_uInt16 nStop = (eScannedType == SvNumFormatType::SCIENTIFIC ?
                                    nNumericsCnt-1 : nNumericsCnt);
                for ( ; k < nStop; k++)
                {
                    sResString.append(sStrArray[nNums[k]]);  // fractional part
                }
            }

            if (eScannedType != SvNumFormatType::SCIENTIFIC)
            {
                fOutNumber = StringToDouble(sResString);
            }
            else
            {                                           // append exponent
                sResString.append('E');
                if ( nESign == -1 )
                {
                    sResString.append('-');
                }
                sResString.append(sStrArray[nNums[nNumericsCnt-1]]);
                rtl_math_ConversionStatus eStatus;
                fOutNumber = ::rtl::math::stringToDouble( sResString, '.', ',', &eStatus );
                if ( eStatus == rtl_math_ConversionStatus_OutOfRange )
                {
                    F_Type = SvNumFormatType::TEXT;         // overflow/underflow -> Text
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

            if (eScannedType == SvNumFormatType::PERCENT)
            {
                fOutNumber/= 100.0;
            }
            break;

        case SvNumFormatType::FRACTION:
            if (nNumericsCnt == 1)
            {
                fOutNumber = StringToDouble(sStrArray[nNums[0]]);
            }
            else if (nNumericsCnt == 2)
            {
                if (nThousand == 1)
                {
                    sResString = sStrArray[nNums[0]];
                    sResString.append(sStrArray[nNums[1]]); // integer part
                    fOutNumber = StringToDouble(sResString);
                }
                else
                {
                    double fNumerator = StringToDouble(sStrArray[nNums[0]]);
                    double fDenominator = StringToDouble(sStrArray[nNums[1]]);
                    if (fDenominator != 0.0)
                    {
                        fOutNumber = fNumerator/fDenominator;
                    }
                    else
                    {
                        res = false;
                    }
                }
            }
            else // nNumericsCnt > 2
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
                fOutNumber = StringToDouble(sResString);

                if (k == nNumericsCnt-2)
                {
                    double fNumerator = StringToDouble(sStrArray[nNums[k]]);
                    double fDenominator = StringToDouble(sStrArray[nNums[k + 1]]);
                    if (fDenominator != 0.0)
                    {
                        fOutNumber += fNumerator/fDenominator;
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

        case SvNumFormatType::TIME:
            res = GetTimeRef(fOutNumber, 0, nNumericsCnt, eInputOptions);
            if ( nSign < 0 )
            {
                fOutNumber = -fOutNumber;
            }
            break;

        case SvNumFormatType::DATE:
            res = GetDateRef( fOutNumber, k );
            break;

        case SvNumFormatType::DATETIME:
            res = GetDateRef( fOutNumber, k );
            if ( res )
            {
                double fTime;
                res = GetTimeRef( fTime, k, nNumericsCnt - k, eInputOptions);
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
            F_Type = SvNumFormatType::TEXT;
            fOutNumber = -DBL_MAX;
            return true;
        }
        else if (fOutNumber >  DBL_MAX) // 1.7E308
        {
            F_Type = SvNumFormatType::TEXT;
            fOutNumber = DBL_MAX;
            return true;
        }
    }

    if (!res)
    {
        eScannedType = SvNumFormatType::TEXT;
        fOutNumber = 0.0;
    }

    F_Type = eScannedType;
    return res;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
