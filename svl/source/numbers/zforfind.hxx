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

#ifndef INCLUDED_SVL_SOURCE_NUMBERS_ZFORFIND_HXX
#define INCLUDED_SVL_SOURCE_NUMBERS_ZFORFIND_HXX

#include <com/sun/star/uno/Sequence.hxx>
#include <rtl/ustring.hxx>

class Date;
class SvNumberformat;
class SvNumberFormatter;

#define SV_MAX_ANZ_INPUT_STRINGS  20    // max count of substrings in input scanner

class ImpSvNumberInputScan
{
public:
    explicit ImpSvNumberInputScan( SvNumberFormatter* pFormatter );
    ~ImpSvNumberInputScan();

/*!*/   void ChangeIntl();                      // MUST be called if language changes

    /// set reference date for offset calculation
    void ChangeNullDate( const sal_uInt16 nDay,
                         const sal_uInt16 nMonth,
                         const sal_Int16 nYear );

    /// convert input string to number
    bool IsNumberFormat( const OUString& rString,            /// input string
                         short& F_Type,                      /// format type (in + out)
                         double& fOutNumber,                 /// value determined (out)
                         const SvNumberformat* pFormat);     /// optional a number format to which compare against

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
    OUString* pUpperMonthText;                  //* Array of month names, uppercase
    OUString* pUpperAbbrevMonthText;            //* Array of month names, abbreviated, uppercase
    OUString* pUpperGenitiveMonthText;          //* Array of genitive month names, uppercase
    OUString* pUpperGenitiveAbbrevMonthText;    //* Array of genitive month names, abbreviated, uppercase
    OUString* pUpperPartitiveMonthText;         //* Array of partitive month names, uppercase
    OUString* pUpperPartitiveAbbrevMonthText;   //* Array of partitive month names, abbreviated, uppercase
    OUString* pUpperDayText;                    //* Array of day of week names, uppercase
    OUString* pUpperAbbrevDayText;              //* Array of day of week names, abbreviated, uppercase
    OUString  aUpperCurrSymbol;                 //* Currency symbol, uppercase
    bool    bTextInitialized;                   //* Whether days and months are initialized
    bool    bScanGenitiveMonths;                //* Whether to scan an input for genitive months
    bool    bScanPartitiveMonths;               //* Whether to scan an input for partitive months
    Date* pNullDate;                            //* 30Dec1899
    // Variables for provisional results:
    OUString sStrArray[SV_MAX_ANZ_INPUT_STRINGS]; //* Array of scanned substrings
    bool       IsNum[SV_MAX_ANZ_INPUT_STRINGS]; //* Whether a substring is numeric
    sal_uInt16 nNums[SV_MAX_ANZ_INPUT_STRINGS]; //* Sequence of offsets to numeric strings
    sal_uInt16 nAnzStrings;                     //* Total count of scanned substrings
    sal_uInt16 nAnzNums;                        //* Count of numeric substrings
    bool   bDecSepInDateSeps;                   //* True <=> DecSep in {.,-,/,DateSep}
    sal_uInt8   nMatchedAllStrings;             //* Scan...String() matched all substrings,

    // bit mask of nMatched... constants
    static const sal_uInt8 nMatchedEndString;        // 0x01
    static const sal_uInt8 nMatchedMidString;        // 0x02
    static const sal_uInt8 nMatchedStartString;      // 0x04
    static const sal_uInt8 nMatchedVirgin;           // 0x08
    static const sal_uInt8 nMatchedUsedAsReturn;     // 0x10

    int    nSign;                               // Sign of number
    int    nMonth;                              // Month (1..x) if date
                                                // negative => short format
    short  nMonthPos;                           // 1 = front, 2 = middle
                                                // 3 = end
    int    nDayOfWeek;                          // Temporary (!) day of week (1..7,-1..-7) if date
    sal_uInt16 nTimePos;                        // Index of first time separator (+1)
    short  nDecPos;                             // Index of substring containing "," (+1)
    short  nNegCheck;                           // '( )' for negative
    short  nESign;                              // Sign of exponent
    short  nAmPm;                               // +1 AM, -1 PM, 0 if none
    short  nLogical;                            // -1 => False, 1 => True
    sal_uInt16 nThousand;                       // Count of group (AKA thousand) separators
    sal_uInt16 nPosThousandString;              // Position of concatenated 000,000,000 string
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
    css::uno::Sequence< OUString >  sDateAcceptancePatterns;

    /** If input matched a date acceptance pattern that starts at input
        particle sStrArray[nDatePatternStart].

        @see IsAcceptedDatePattern()
     */
    sal_uInt16  nDatePatternStart;

    /** Count of numbers that matched the accepted pattern, if any, else 0.

        @see GetDatePatternNumbers()
     */
    sal_uInt16  nDatePatternNumbers;

    // Copy assignment is forbidden and not implemented.
    ImpSvNumberInputScan (const ImpSvNumberInputScan &) = delete;
    ImpSvNumberInputScan & operator= (const ImpSvNumberInputScan &) = delete;

    void Reset();                               // Reset all variables before start of analysis

    void InitText();                            // Init of months and days of week

    // Convert string to double.
    // Only simple unsigned floating point values without any error detection,
    // decimal separator has to be '.'
    // If bForceFraction==true the string is taken to be the fractional part
    // of 0.1234 without the leading 0. (thus being just "1234").
    static double StringToDouble( const OUString& rStr,
                                  bool bForceFraction = false );

    // Next number/string symbol
    static bool NextNumberStringSymbol( const sal_Unicode*& pStr,
                                        OUString& rSymbol );

    // Concatenate ,000,23 blocks
    // in input to 000123
    bool SkipThousands( const sal_Unicode*& pStr, OUString& rSymbol );

    // Divide numbers/strings into
    // arrays and variables above.
    // Leading blanks and blanks
    // after numbers are thrown away
    void NumberStringDivision( const OUString& rString );


    /** Whether rString contains word (!) rWhat at nPos.
        rWhat will not be matched if it is a substring of a word.
     */
    bool StringContainsWord( const OUString& rWhat,
                             const OUString& rString,
                             sal_Int32 nPos );

    // optimized substring versions

    // Whether rString contains rWhat at nPos
    static inline bool StringContains( const OUString& rWhat,
                                       const OUString& rString,
                                       sal_Int32 nPos )
        {
            if (rWhat.isEmpty() || rString.getLength() <= nPos)
            {
                return false;
            }
            // mostly used with one character
            if ( rWhat[ 0 ] != rString[ nPos ] )
            {
                return false;
            }
            return StringContainsImpl( rWhat, rString, nPos );
        }

    // Whether pString contains rWhat at nPos
    static inline bool StringPtrContains( const OUString& rWhat,
                                          const sal_Unicode* pString,
                                          sal_Int32 nPos ) // nPos MUST be a valid offset from pString
        {
            // mostly used with one character
            if ( rWhat[ 0 ] != pString[ nPos ] )
            {
                return false;
            }
            return StringPtrContainsImpl( rWhat, pString, nPos );
        }

    //! DO NOT use directly
    static bool StringContainsImpl( const OUString& rWhat,
                                    const OUString& rString,
                                    sal_Int32 nPos );
    //! DO NOT use directly
    static bool StringPtrContainsImpl( const OUString& rWhat,
                                       const sal_Unicode* pString,
                                       sal_Int32 nPos );

    // Skip a special character
    static inline bool SkipChar( sal_Unicode c,
                                 const OUString& rString,
                                 sal_Int32& nPos );

    // Skip blank
    static inline void SkipBlanks( const OUString& rString,
                                   sal_Int32& nPos );

    // Jump over rWhat in rString at nPos
    static inline bool SkipString( const OUString& rWhat,
                                   const OUString& rString,
                                   sal_Int32& nPos );

    // Recognizes exactly ,111 as group separator
    inline bool GetThousandSep( const OUString& rString,
                                sal_Int32& nPos,
                                sal_uInt16 nStringPos );
    // Get boolean value
    short GetLogical( const OUString& rString );

    // Get month and advance string position
    short GetMonth( const OUString& rString,
                    sal_Int32& nPos );

    // Get day of week and advance string position
    int GetDayOfWeek( const OUString& rString,
                      sal_Int32& nPos );

    // Get currency symbol and advance string position
    bool GetCurrency( const OUString& rString,
                      sal_Int32& nPos,
                      const SvNumberformat* pFormat = nullptr ); // optional number format to match against

    // Get symbol AM or PM and advance string position
    bool GetTimeAmPm( const OUString& rString,
                      sal_Int32& nPos );

    // Get decimal separator and advance string position
    inline bool GetDecSep( const OUString& rString,
                           sal_Int32& nPos );

    // Get hundredth seconds separator and advance string position
    inline bool GetTime100SecSep( const OUString& rString,
                                  sal_Int32& nPos );

    // Get sign  and advance string position
    // Including special case '('
    int GetSign( const OUString& rString,
                 sal_Int32& nPos );

    // Get sign of exponent and advance string position
    static short GetESign( const OUString& rString,
                           sal_Int32& nPos );

    // Get next number as array offset
    inline bool GetNextNumber( sal_uInt16& i,
                               sal_uInt16& j );

    /** Converts time -> double (only decimals)

        @return TRUE if time, FALSE if not (e.g. hours >12 with AM/PM)
     */
    bool GetTimeRef( double& fOutNumber,                     // result as double
                     sal_uInt16 nIndex,                      // Index of hour in input
                     sal_uInt16 nAnz );                      // Count of time substrings in input
    sal_uInt16 ImplGetDay  ( sal_uInt16 nIndex );   // Day input, 0 if no match
    sal_uInt16 ImplGetMonth( sal_uInt16 nIndex );   // Month input, zero based return, NumberOfMonths if no match
    sal_uInt16 ImplGetYear ( sal_uInt16 nIndex );   // Year input, 0 if no match

    // Conversion of date to number
    bool GetDateRef( double& fDays,                          // OUT: days diff to null date
                     sal_uInt16& nCounter,                   // Count of date substrings
                     const SvNumberformat* pFormat = nullptr ); // optional number format to match against

    // Analyze start of string
    bool ScanStartString( const OUString& rString,
                          const SvNumberformat* pFormat = nullptr );

    // Analyze middle substring
    bool ScanMidString( const OUString& rString,
                        sal_uInt16 nStringPos,
                        const SvNumberformat* pFormat = nullptr );


    // Analyze end of string
    bool ScanEndString( const OUString& rString,
                        const SvNumberformat* pFormat = nullptr );

    // Compare rString to substring of array indexed by nString
    // nString == 0xFFFF => last substring
    bool ScanStringNumFor( const OUString& rString,
                           sal_Int32 nPos,
                           const SvNumberformat* pFormat,
                           sal_uInt16 nString,
                           bool bDontDetectNegation = false );

    // if nMatchedAllStrings set nMatchedUsedAsReturn and return true,
    // else do nothing and return false
    bool MatchedReturn();

    //! Be sure that the string to be analyzed is already converted to upper
    //! case and if it contained native number digits that they are already
    //! converted to ASCII.

    // Main analyzing function
    bool IsNumberFormatMain( const OUString& rString,
                             const SvNumberformat* pFormat = nullptr);    // optional number format to match against

    static inline bool MyIsdigit( sal_Unicode c );

    /** Whether input matches locale dependent date acceptance pattern.

        @param nStartPatternAt
               The pattern matching starts at input particle
               sStrArray[nStartPatternAt].

        NOTE: once called the result is remembered, subsequent calls with
        different parameters do not check for a match and do not lead to a
        different result.
     */
    bool IsAcceptedDatePattern( sal_uInt16 nStartPatternAt );

    /** Sets (not advances!) rPos to sStrArray[nParticle].getLength() if string
        matches separator in pattern at nParticle.

        @returns TRUE if separator matched.
     */
    bool SkipDatePatternSeparator( sal_uInt16 nParticle, sal_Int32 & rPos );

    /** Returns count of numbers in accepted date pattern.
     */
    sal_uInt16 GetDatePatternNumbers();

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
};

#endif // INCLUDED_SVL_SOURCE_NUMBERS_ZFORFIND_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
