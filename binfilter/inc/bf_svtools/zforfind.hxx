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

#ifndef _ZFORFIND_HXX
#define _ZFORFIND_HXX

#include <tools/string.hxx>

class Date;

namespace binfilter
{

class SvNumberformat;
class SvNumberFormatter;

#define SV_MAX_ANZ_INPUT_STRINGS  20    // max count of substrings in input scanner

class ImpSvNumberInputScan
{
public:
    ImpSvNumberInputScan( SvNumberFormatter* pFormatter );
    ~ImpSvNumberInputScan();

/*!*/   void ChangeIntl();                      // MUST be called if language changes

    /// set reference date for offset calculation
    void ChangeNullDate(
            const USHORT nDay,
            const USHORT nMonth,
            const USHORT nYear );

    /// convert input string to number
    BOOL IsNumberFormat(
            const String& rString,              /// input string
            short& F_Type,                      /// format type (in + out)
            double& fOutNumber,                 /// value determined (out)
            const SvNumberformat* pFormat = NULL    /// optional a number format to which compare against
            );

    /// after IsNumberFormat: get decimal position
    short   GetDecPos() const { return nDecPos; }
    /// after IsNumberFormat: get count of numeric substrings in input string
    USHORT  GetAnzNums() const { return nAnzNums; }

    /// set threshold of two-digit year input
    void    SetYear2000( USHORT nVal ) { nYear2000 = nVal; }
    /// get threshold of two-digit year input
    USHORT  GetYear2000() const { return nYear2000; }

private:
    SvNumberFormatter*  pFormatter;
    String* pUpperMonthText;                    // Array of month names, uppercase
    String* pUpperAbbrevMonthText;              // Array of month names, abbreviated, uppercase
    String* pUpperDayText;                      // Array of day of week names, uppercase
    String* pUpperAbbrevDayText;                // Array of day of week names, abbreviated, uppercase
    String  aUpperCurrSymbol;                   // Currency symbol, uppercase
    BOOL    bTextInitialized;                   // Whether days and months are initialized
    Date* pNullDate;                            // 30Dec1899
                                                // Variables for provisional results:
    String sStrArray[SV_MAX_ANZ_INPUT_STRINGS]; // Array of scanned substrings
    BOOL   IsNum[SV_MAX_ANZ_INPUT_STRINGS];     // Whether a substring is numeric
    USHORT nNums[SV_MAX_ANZ_INPUT_STRINGS];     // Sequence of offsets to numeric strings
    USHORT nAnzStrings;                         // Total count of scanned substrings
    USHORT nAnzNums;                            // Count of numeric substrings
    BOOL   bDecSepInDateSeps;                   // True <=> DecSep in {.,-,/,DateSep}
    BYTE   nMatchedAllStrings;                  // Scan...String() matched all substrings,
                                                // bit mask of nMatched... constants

    static const BYTE nMatchedEndString;        // 0x01
    static const BYTE nMatchedMidString;        // 0x02
    static const BYTE nMatchedStartString;      // 0x04
    static const BYTE nMatchedVirgin;           // 0x08
    static const BYTE nMatchedUsedAsReturn;     // 0x10

    int    nSign;                               // Sign of number
    short  nMonth;                              // Month (1..x) if date
                                                // negative => short format
    short  nMonthPos;                           // 1 = front, 2 = middle
                                                // 3 = end
    USHORT nTimePos;                            // Index of first time separator (+1)
    short  nDecPos;                             // Index of substring containing "," (+1)
    short  nNegCheck;                           // '( )' for negative
    short  nESign;                              // Sign of exponent
    short  nAmPm;                               // +1 AM, -1 PM, 0 if none
    short  nLogical;                            // -1 => False, 1 => True
    USHORT nThousand;                           // Count of group (AKA thousand) separators
    USHORT nPosThousandString;                  // Position of concatenaded 000,000,000 string
    short  eScannedType;                        // Scanned type
    short  eSetType;                            // Preset Type

    USHORT nStringScanNumFor;                   // Fixed strings recognized in
                                                // pFormat->NumFor[nNumForStringScan]
    short  nStringScanSign;                     // Sign resulting of FixString
    USHORT nYear2000;                           // Two-digit threshold
                                                // Year as 20xx
                                                // default 18
                                                // number <= nYear2000 => 20xx
                                                // number >  nYear2000 => 19xx
    USHORT  nTimezonePos;                       // Index of timezone separator (+1)
    BYTE    nMayBeIso8601;                      // 0:=dontknowyet, 1:=yes, 2:=no

#ifdef _ZFORFIND_CXX        // methods private to implementation
    void Reset();                               // Reset all variables before start of analysis

    void InitText();                            // Init of months and days of week

    // Convert string to double.
    // Only simple unsigned floating point values without any error detection,
    // decimal separator has to be '.'
    // If bForceFraction==TRUE the string is taken to be the fractional part
    // of 0.1234 without the leading 0. (thus being just "1234").
    double StringToDouble(
            const String& rStr,
            BOOL bForceFraction = FALSE );

    BOOL NextNumberStringSymbol(                // Next number/string symbol
            const sal_Unicode*& pStr,
            String& rSymbol );

    BOOL SkipThousands(                         // Concatenate ,000,23 blocks
            const sal_Unicode*& pStr,           // in input to 000123
            String& rSymbol );

    void NumberStringDivision(                  // Divide numbers/strings into
            const String& rString );            // arrays and variables above.
                                                // Leading blanks and blanks
                                                // after numbers are thrown away


                                                // optimized substring versions

    static inline BOOL StringContains(          // Whether rString contains rWhat at nPos
            const String& rWhat,
            const String& rString,
            xub_StrLen nPos )
                {   // mostly used with one character
                    if ( rWhat.GetChar(0) != rString.GetChar(nPos) )
                        return FALSE;
                    return StringContainsImpl( rWhat, rString, nPos );
                }
    static inline BOOL StringPtrContains(       // Whether pString contains rWhat at nPos
            const String& rWhat,
            const sal_Unicode* pString,
            xub_StrLen nPos )                   // nPos MUST be a valid offset from pString
                {   // mostly used with one character
                    if ( rWhat.GetChar(0) != *(pString+nPos) )
                        return FALSE;
                    return StringPtrContainsImpl( rWhat, pString, nPos );
                }
    static BOOL StringContainsImpl(             //! DO NOT use directly
            const String& rWhat,
            const String& rString,
            xub_StrLen nPos );
    static BOOL StringPtrContainsImpl(          //! DO NOT use directly
            const String& rWhat,
            const sal_Unicode* pString,
            xub_StrLen nPos );


    static inline BOOL SkipChar(                // Skip a special character
            sal_Unicode c,
            const String& rString,
            xub_StrLen& nPos );
    static inline void SkipBlanks(              // Skip blank
            const String& rString,
            xub_StrLen& nPos );
    static inline BOOL SkipString(              // Jump over rWhat in rString at nPos
            const String& rWhat,
            const String& rString,
            xub_StrLen& nPos );

    inline BOOL GetThousandSep(                 // Recognizes exactly ,111 as group separator
            const String& rString,
            xub_StrLen& nPos,
            USHORT nStringPos );
    short GetLogical(                           // Get boolean value
            const String& rString );
    short GetMonth(                             // Get month and advance string position
            const String& rString,
            xub_StrLen& nPos );
    int GetDayOfWeek(                           // Get day of week and advance string position
            const String& rString,
            xub_StrLen& nPos );
    BOOL GetCurrency(                           // Get currency symbol and advance string position
            const String& rString,
            xub_StrLen& nPos,
            const SvNumberformat* pFormat = NULL ); // optional number format to match against
    BOOL GetTimeAmPm(                           // Get symbol AM or PM and advance string position
            const String& rString,
            xub_StrLen& nPos );
    inline BOOL GetDecSep(                      // Get decimal separator and advance string position
            const String& rString,
            xub_StrLen& nPos );
    inline BOOL GetTime100SecSep(               // Get hundredth seconds separator and advance string position
            const String& rString,
            xub_StrLen& nPos );
    int GetSign(                                // Get sign  and advance string position
            const String& rString,              // Including special case '('
            xub_StrLen& nPos );
    short GetESign(                             // Get sign of exponent and advance string position
            const String& rString,
            xub_StrLen& nPos );

    inline BOOL GetNextNumber(                  // Get next number as array offset
            USHORT& i,
            USHORT& j );

    void GetTimeRef(                            // Converts time -> double (only decimals)
            double& fOutNumber,                 // result as double
            USHORT nIndex,                      // Index of hour in input
            USHORT nAnz );                      // Count of time substrings in input
    USHORT ImplGetDay  ( USHORT nIndex );       // Day input, 0 if no match
    USHORT ImplGetMonth( USHORT nIndex );       // Month input, zero based return, NumberOfMonths if no match
    USHORT ImplGetYear ( USHORT nIndex );       // Year input, 0 if no match
    BOOL GetDateRef(                            // Conversion of date to number
            double& fDays,                      // OUT: days diff to null date
            USHORT& nCounter,                   // Count of date substrings
            const SvNumberformat* pFormat = NULL ); // optional number format to match against

    BOOL ScanStartString(                       // Analyze start of string
            const String& rString,
            const SvNumberformat* pFormat = NULL );
    BOOL ScanMidString(                         // Analyze middle substring
            const String& rString,
            USHORT nStringPos,
            const SvNumberformat* pFormat = NULL );
    BOOL ScanEndString(                         // Analyze end of string
            const String& rString,
            const SvNumberformat* pFormat = NULL );

    // Whether input may be a ISO 8601 date format, yyyy-mm-dd...
    // checks if at least 3 numbers and first number>31
    bool MayBeIso8601();

    // Compare rString to substring of array indexed by nString
    // nString == 0xFFFF => last substring
    BOOL ScanStringNumFor(
            const String& rString,
            xub_StrLen nPos,
            const SvNumberformat* pFormat,
            USHORT nString,
            BOOL bDontDetectNegation = FALSE );

    // if nMatchedAllStrings set nMatchedUsedAsReturn and return TRUE,
    // else do nothing and return FALSE
    BOOL MatchedReturn();

    //! Be sure that the string to be analyzed is already converted to upper
    //! case and if it contained native humber digits that they are already
    //! converted to ASCII.
    BOOL IsNumberFormatMain(                    // Main anlyzing function
            const String& rString,
            double& fOutNumber,                 // return value if string is numeric
            const SvNumberformat* pFormat = NULL    // optional number format to match against
            );

    static inline BOOL MyIsdigit( sal_Unicode c );

    // native number transliteration if necessary
    void TransformInput( String& rString );

#endif  // _ZFORFIND_CXX
};


}

#endif  // _ZFORFIND_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
