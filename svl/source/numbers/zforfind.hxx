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

#ifndef _ZFORFIND_HXX
#define _ZFORFIND_HXX

#include <tools/string.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <rtl/ustring.hxx>

class Date;
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
            const sal_uInt16 nDay,
            const sal_uInt16 nMonth,
            const sal_uInt16 nYear );

    /// convert input string to number
    bool IsNumberFormat(
            const String& rString,              /// input string
            short& F_Type,                      /// format type (in + out)
            double& fOutNumber,                 /// value determined (out)
            const SvNumberformat* pFormat = NULL    /// optional a number format to which compare against
            );

    /// after IsNumberFormat: get decimal position
    short   GetDecPos() const { return nDecPos; }
    /// after IsNumberFormat: get count of numeric substrings in input string
    sal_uInt16  GetAnzNums() const { return nAnzNums; }

    /// set threshold of two-digit year input
    void    SetYear2000( sal_uInt16 nVal ) { nYear2000 = nVal; }
    /// get threshold of two-digit year input
    sal_uInt16  GetYear2000() const { return nYear2000; }

    /** Whether input can be forced to ISO 8601 format.

        Depends on locale's date separator and a specific date format order.

        @param eDateFormat
            Evaluated only on first call during one scan process, subsequent
            calls return state of nCanForceToIso8601!

        @see nCanForceToIso8601
     */
    bool CanForceToIso8601( DateFormat eDateFormat );

    void InvalidateDateAcceptancePatterns();

private:
    SvNumberFormatter*  pFormatter;
    String* pUpperMonthText;                    // Array of month names, uppercase
    String* pUpperAbbrevMonthText;              // Array of month names, abbreviated, uppercase
    String* pUpperGenitiveMonthText;            // Array of genitive month names, uppercase
    String* pUpperGenitiveAbbrevMonthText;      // Array of genitive month names, abbreviated, uppercase
    String* pUpperPartitiveMonthText;           // Array of partitive month names, uppercase
    String* pUpperPartitiveAbbrevMonthText;     // Array of partitive month names, abbreviated, uppercase
    String* pUpperDayText;                      // Array of day of week names, uppercase
    String* pUpperAbbrevDayText;                // Array of day of week names, abbreviated, uppercase
    String  aUpperCurrSymbol;                   // Currency symbol, uppercase
    bool    bTextInitialized;                   // Whether days and months are initialized
    bool    bScanGenitiveMonths;                // Whether to scan an input for genitive months
    bool    bScanPartitiveMonths;               // Whether to scan an input for partitive months
    Date* pNullDate;                            // 30Dec1899
                                                // Variables for provisional results:
    String sStrArray[SV_MAX_ANZ_INPUT_STRINGS]; // Array of scanned substrings
    bool       IsNum[SV_MAX_ANZ_INPUT_STRINGS]; // Whether a substring is numeric
    sal_uInt16 nNums[SV_MAX_ANZ_INPUT_STRINGS]; // Sequence of offsets to numeric strings
    sal_uInt16 nAnzStrings;                     // Total count of scanned substrings
    sal_uInt16 nAnzNums;                        // Count of numeric substrings
    bool   bDecSepInDateSeps;                   // True <=> DecSep in {.,-,/,DateSep}
    sal_uInt8   nMatchedAllStrings;             // Scan...String() matched all substrings,
                                                // bit mask of nMatched... constants

    static const sal_uInt8 nMatchedEndString;        // 0x01
    static const sal_uInt8 nMatchedMidString;        // 0x02
    static const sal_uInt8 nMatchedStartString;      // 0x04
    static const sal_uInt8 nMatchedVirgin;           // 0x08
    static const sal_uInt8 nMatchedUsedAsReturn;     // 0x10

    int    nSign;                               // Sign of number
    short  nMonth;                              // Month (1..x) if date
                                                // negative => short format
    short  nMonthPos;                           // 1 = front, 2 = middle
                                                // 3 = end
    sal_uInt16 nTimePos;                        // Index of first time separator (+1)
    short  nDecPos;                             // Index of substring containing "," (+1)
    short  nNegCheck;                           // '( )' for negative
    short  nESign;                              // Sign of exponent
    short  nAmPm;                               // +1 AM, -1 PM, 0 if none
    short  nLogical;                            // -1 => False, 1 => True
    sal_uInt16 nThousand;                       // Count of group (AKA thousand) separators
    sal_uInt16 nPosThousandString;              // Position of concatenaded 000,000,000 string
    short  eScannedType;                        // Scanned type
    short  eSetType;                            // Preset Type

    sal_uInt16 nStringScanNumFor;               // Fixed strings recognized in
                                                // pFormat->NumFor[nNumForStringScan]
    short  nStringScanSign;                     // Sign resulting of FixString
    sal_uInt16 nYear2000;                       // Two-digit threshold
                                                // Year as 20xx
                                                // default 18
                                                // number <= nYear2000 => 20xx
                                                // number >  nYear2000 => 19xx
    sal_uInt16  nTimezonePos;                   // Index of timezone separator (+1)

    /** State of ISO 8601 detection.

        0:= don't know yet
        1:= no
        2:= yes, <=2 digits in year
        3:= yes,   3 digits in year
        4:= yes, >=4 digits in year

        @see MayBeIso8601()
     */
    sal_uInt8    nMayBeIso8601;

    /** State of ISO 8601 can be forced.

        0:= don't know yet
        1:= no
        2:= yes

        @see CanForceToIso8601()
     */
    sal_uInt8   nCanForceToIso8601;

    /** State of dd-month-yy or yy-month-dd detection, with month name.

        0:= don't know yet
        1:= no
        2:= yes, dd-month-yy
        3:= yes, yy-month-dd

        @see MayBeMonthDate()
     */
    sal_uInt8   nMayBeMonthDate;

    /** Input matched this locale dependent date acceptance pattern.
        -2 if not checked yet, -1 if no match, >=0 matched pattern.

        @see IsAcceptedDatePattern()
     */
    sal_Int32   nAcceptedDatePattern;
    com::sun::star::uno::Sequence< rtl::OUString >  sDateAcceptancePatterns;

    /** If input matched a date acceptance pattern that starts at input
        particle sStrArray[nDatePatternStart].

        @see IsAcceptedDatePattern()
     */
    sal_uInt16  nDatePatternStart;

#ifdef _ZFORFIND_CXX        // methods private to implementation
    void Reset();                               // Reset all variables before start of analysis

    void InitText();                            // Init of months and days of week

    // Convert string to double.
    // Only simple unsigned floating point values without any error detection,
    // decimal separator has to be '.'
    // If bForceFraction==true the string is taken to be the fractional part
    // of 0.1234 without the leading 0. (thus being just "1234").
    double StringToDouble(
            const String& rStr,
            bool bForceFraction = false );

    bool NextNumberStringSymbol(                // Next number/string symbol
            const sal_Unicode*& pStr,
            String& rSymbol );

    bool SkipThousands(                         // Concatenate ,000,23 blocks
            const sal_Unicode*& pStr,           // in input to 000123
            String& rSymbol );

    void NumberStringDivision(                  // Divide numbers/strings into
            const String& rString );            // arrays and variables above.
                                                // Leading blanks and blanks
                                                // after numbers are thrown away


                                                // optimized substring versions

    static inline bool StringContains(          // Whether rString contains rWhat at nPos
            const String& rWhat,
            const String& rString,
            xub_StrLen nPos )
                {   // mostly used with one character
                    if ( rWhat.GetChar(0) != rString.GetChar(nPos) )
                        return false;
                    return StringContainsImpl( rWhat, rString, nPos );
                }
    static inline bool StringPtrContains(       // Whether pString contains rWhat at nPos
            const String& rWhat,
            const sal_Unicode* pString,
            xub_StrLen nPos )                   // nPos MUST be a valid offset from pString
                {   // mostly used with one character
                    if ( rWhat.GetChar(0) != *(pString+nPos) )
                        return false;
                    return StringPtrContainsImpl( rWhat, pString, nPos );
                }
    static bool StringContainsImpl(             //! DO NOT use directly
            const String& rWhat,
            const String& rString,
            xub_StrLen nPos );
    static bool StringPtrContainsImpl(          //! DO NOT use directly
            const String& rWhat,
            const sal_Unicode* pString,
            xub_StrLen nPos );


    static inline bool SkipChar(                // Skip a special character
            sal_Unicode c,
            const String& rString,
            xub_StrLen& nPos );
    static inline void SkipBlanks(              // Skip blank
            const String& rString,
            xub_StrLen& nPos );
    static inline bool SkipString(              // Jump over rWhat in rString at nPos
            const String& rWhat,
            const String& rString,
            xub_StrLen& nPos );

    inline bool GetThousandSep(                 // Recognizes exactly ,111 as group separator
            const String& rString,
            xub_StrLen& nPos,
            sal_uInt16 nStringPos );
    short GetLogical(                           // Get boolean value
            const String& rString );
    short GetMonth(                             // Get month and advance string position
            const String& rString,
            xub_StrLen& nPos );
    int GetDayOfWeek(                           // Get day of week and advance string position
            const String& rString,
            xub_StrLen& nPos );
    bool GetCurrency(                           // Get currency symbol and advance string position
            const String& rString,
            xub_StrLen& nPos,
            const SvNumberformat* pFormat = NULL ); // optional number format to match against
    bool GetTimeAmPm(                           // Get symbol AM or PM and advance string position
            const String& rString,
            xub_StrLen& nPos );
    inline bool GetDecSep(                      // Get decimal separator and advance string position
            const String& rString,
            xub_StrLen& nPos );
    inline bool GetTime100SecSep(               // Get hundredth seconds separator and advance string position
            const String& rString,
            xub_StrLen& nPos );
    int GetSign(                                // Get sign  and advance string position
            const String& rString,              // Including special case '('
            xub_StrLen& nPos );
    short GetESign(                             // Get sign of exponent and advance string position
            const String& rString,
            xub_StrLen& nPos );

    inline bool GetNextNumber(                  // Get next number as array offset
            sal_uInt16& i,
            sal_uInt16& j );

    /** Converts time -> double (only decimals)

        @return TRUE if time, FALSE if not (e.g. hours >12 with AM/PM)
     */
    bool GetTimeRef(
            double& fOutNumber,                     // result as double
            sal_uInt16 nIndex,                      // Index of hour in input
            sal_uInt16 nAnz );                      // Count of time substrings in input
    sal_uInt16 ImplGetDay  ( sal_uInt16 nIndex );   // Day input, 0 if no match
    sal_uInt16 ImplGetMonth( sal_uInt16 nIndex );   // Month input, zero based return, NumberOfMonths if no match
    sal_uInt16 ImplGetYear ( sal_uInt16 nIndex );   // Year input, 0 if no match
    bool GetDateRef(                                // Conversion of date to number
            double& fDays,                          // OUT: days diff to null date
            sal_uInt16& nCounter,                   // Count of date substrings
            const SvNumberformat* pFormat = NULL ); // optional number format to match against

    bool ScanStartString(                       // Analyze start of string
            const String& rString,
            const SvNumberformat* pFormat = NULL );
    bool ScanMidString(                         // Analyze middle substring
            const String& rString,
            sal_uInt16 nStringPos,
            const SvNumberformat* pFormat = NULL );
    bool ScanEndString(                         // Analyze end of string
            const String& rString,
            const SvNumberformat* pFormat = NULL );

    // Compare rString to substring of array indexed by nString
    // nString == 0xFFFF => last substring
    bool ScanStringNumFor(
            const String& rString,
            xub_StrLen nPos,
            const SvNumberformat* pFormat,
            sal_uInt16 nString,
            bool bDontDetectNegation = false );

    // if nMatchedAllStrings set nMatchedUsedAsReturn and return true,
    // else do nothing and return false
    bool MatchedReturn();

    //! Be sure that the string to be analyzed is already converted to upper
    //! case and if it contained native humber digits that they are already
    //! converted to ASCII.
    bool IsNumberFormatMain(                    // Main anlyzing function
            const String& rString,
            const SvNumberformat* pFormat = NULL    // optional number format to match against
            );

    static inline bool MyIsdigit( sal_Unicode c );

    // native number transliteration if necessary
    void TransformInput( String& rString );

    /** Whether input matches locale dependent date acceptance pattern.

        @param nStartPatternAt
               The pattern matching starts at input particle
               sStrArray[nStartPatternAt].

        NOTE: once called the result is remembered, subsequent calls with
        different parameters do not check for a match and do not lead to a
        different result.
     */
    bool IsAcceptedDatePattern( sal_uInt16 nStartPatternAt );

    /** Sets (not advances!) rPos to sStrArray[nParticle].Len() if string
        matches separator in pattern at nParticle.

        @returns TRUE if separator matched.
     */
    bool SkipDatePatternSeparator( sal_uInt16 nParticle, xub_StrLen & rPos );

    /** Obtain order of accepted date pattern coded as, for example,
        ('D'<<16)|('M'<<8)|'Y'
    */
    sal_uInt32 GetDatePatternOrder();

    /** Obtain date format order, from accepted date pattern if available or
        otherwise the locale's default order.
     */
    DateFormat GetDateOrder();

    /** Whether input may be an ISO 8601 date format, yyyy-mm-dd...

        Checks if input has at least 3 numbers for yyyy-mm-dd and the separator
        is '-', and 1<=mm<=12 and 1<=dd<=31.

        @see nMayBeIso8601
     */
    bool MayBeIso8601();

    /** Whether input may be a dd-month-yy format, with month name, not
        number.

        @see nMayBeMonthDate
     */
    bool MayBeMonthDate();

#endif  // _ZFORFIND_CXX
};



#endif  // _ZFORFIND_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
