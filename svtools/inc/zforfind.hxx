/*************************************************************************
 *
 *  $RCSfile: zforfind.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: er $ $Date: 2000-10-17 18:44:42 $
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

#ifndef _ZFORFIND_HXX
#define _ZFORFIND_HXX

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

class International;
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
    String aUpperMonthText[12];                 // The 12 months
    String aUpperAbbrevMonthText[12];           // The 12 monaths, abbreviated
    String aUpperDayText[7];                    // The 7 days of week
    String aUpperAbbrevDayText[7];              // The 7 days of week, abbreviated
    String aUpperCurrSymbol;                    // Currency symbol
    BOOL    bTextInitialized;                   // Whether days and months are initialized
    Date* pNullDate;                            // "1.1.1900"
                                                // Variables for provisional results:
    String sStrArray[SV_MAX_ANZ_INPUT_STRINGS]; // Array of scanned substrings
    BOOL   IsNum[SV_MAX_ANZ_INPUT_STRINGS];     // Whether a substring is numeric
    USHORT nNums[SV_MAX_ANZ_INPUT_STRINGS];     // Sequence of offsets to numeric strings
    USHORT nAnzStrings;                         // Total count of scanned substrings
    USHORT nAnzNums;                            // Count of numeric substrings
    BOOL   bDecSepInDateSeps;                   // True <=> DecSep in {.,-,/,DateSep}

    short  nSign;                               // Sign of number
    short  nMonth;                              // Month (1..12) if date
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

#ifdef _ZFORFIND_CXX        // methods private to implementation
    void Reset();                               // Reset all variables before start of analysis

    void InitText();                            // Init of months and days of week

    double StringToDouble(                      // Convert string to double
            const String& rStr );

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
                    if ( rWhat.GetChar(0) != rString.GetChar(0) )
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
    short GetDayOfWeek(                         // Get day of week and advance string position
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
    short GetSign(                              // Get sign  and advance string position
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
    USHORT ImplGetDay  ( USHORT nIndex );       // Day: input or current
    USHORT ImplGetMonth( USHORT nIndex );       // Month: input or current
    USHORT ImplGetYear ( USHORT nIndex );       // Year: input or current
    BOOL GetDateRef(                            // Conversion of date to class Date
            Date& aDt,
            USHORT& nCounter,                   // Count of date substrings
            const SvNumberformat* pFormat = NULL ); // optional number format to match against

    BOOL ScanStartString(                       // Analyze start of string
            const String& rString,
            const SvNumberformat* pFormat = NULL );
    BOOL ScanMidString(                         // Analyze middle substring
            const String& rString,
            USHORT nStringPos );
    BOOL ScanEndString(                         // Analyze end of string
            const String& rString,
            const SvNumberformat* pFormat = NULL );

    // Compare rString to substring of array indexed by nString
    // nString == 0xFFFF => last substring
    BOOL ScanStringNumFor(
            const String& rString,
            xub_StrLen nPos,
            const SvNumberformat* pFormat,
            USHORT nString );

    BOOL IsNumberFormatMain(                    // Main anlyzing function
            const String& rString,
            double& fOutNumber,                 // return value if string is numeric
            const SvNumberformat* pFormat = NULL    // optional number format to match against
            );


#endif  // _ZFORFIND_CXX
};



#endif  // _ZFORFIND_HXX
