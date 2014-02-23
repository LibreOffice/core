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

#ifndef INCLUDED_BASIC_SBXFORM_HXX
#define INCLUDED_BASIC_SBXFORM_HXX


// Implementation class for Basic command: Format$( d,formatStr )

/*
  Grammar of format string (a try):
  -----------------------------------------------

  format_string     := {\special_char} general_format | scientific_format {\special_char} {;format_string}
  general_format    := {#[,]}{0[,]}[.{0}{#}]
  scientific_format := {0}[.{0}{#}](e | E)(+ | -){#}{0}

  percent_char      := '%'
  special_char      := \char | + | - | ( | ) | $ | space_char
  char              := all_ascii_chars
  space_char        := ' '

  {}    repeated multiple times (incl. zero times)
  []    exactly one or zero times
  ()    parenthesis, e.g. (e | E) means e or E times

  Additional predefined formats for the format string:
    "General Number"
    "Currency"
    "Fixed"
    "Standard"
    "Percent"
    "Scientific"
    "Yes/No"
    "True/False"
    "On/Off"

 Note: invalid format string are ignored just as in VisualBasic, the output is
       probably 'undefined'. ASCII letters are outputted directly.

 Constraints in VisualBasic:
    - the exponent (scientific syntax) has a maximum of three digits!

 Constraints of new implementation:
    - the '+' sign is not allowed as wildcard in the mantissa

 TODO:
    - Date formatting
        Wildcards are: 'h', 'm', 's', 'y'
        predefined String-Constants/Commands:
            "AMPM", "Long Date", "Long Time"
*/

/*
  There are two possibilities to get the number of digits of a number:

        a) use sprintf()
        b) use log10() and pow() digit
*/
#define _with_sprintf   // use a)

#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <basic/basicdllapi.h>

class BASIC_DLLPUBLIC SbxBasicFormater {
  public:
    // Constructor takes signs for decimal point, thousand separation sign
    // and necessary resource strings.
    SbxBasicFormater( sal_Unicode _cDecPoint, sal_Unicode _cThousandSep,
                      OUString _sOnStrg,
                      OUString _sOffStrg,
                      OUString _sYesStrg,
                      OUString _sNoStrg,
                      OUString _sTrueStrg,
                      OUString _sFalseStrg,
                      OUString _sCurrencyStrg,
                      OUString _sCurrencyFormatStrg );

    /* Basic command: Format$( number,format-string )

       Parameter:
        dNumber     : number to be formated
        sFormatStrg : the Format-String, e.g. ###0.0###

       Return value:
        String containing the formatted output
    */
    OUString  BasicFormat( double dNumber, OUString sFormatStrg );
    OUString  BasicFormatNull( OUString sFormatStrg );

    static  sal_Bool isBasicFormat( OUString sFormatStrg );

  private:
    BASIC_DLLPRIVATE inline void ShiftString( OUStringBuffer& sStrg, sal_uInt16 nStartPos );
    BASIC_DLLPRIVATE void   AppendDigit( OUStringBuffer& sStrg, short nDigit );
    BASIC_DLLPRIVATE void   LeftShiftDecimalPoint( OUStringBuffer& sStrg );
    BASIC_DLLPRIVATE void   StrRoundDigit( OUStringBuffer& sStrg, short nPos, sal_Bool& bOverflow );
    BASIC_DLLPRIVATE void   StrRoundDigit( OUStringBuffer& sStrg, short nPos );
    BASIC_DLLPRIVATE void   ParseBack( OUStringBuffer& sStrg, const OUString& sFormatStrg,
                                       short nFormatPos );
#ifdef _with_sprintf
    // Methods for string conversion with sprintf():
    BASIC_DLLPRIVATE void   InitScan( double _dNum );
    BASIC_DLLPRIVATE void   InitExp( double _dNewExp );
    BASIC_DLLPRIVATE short  GetDigitAtPosScan( short nPos, sal_Bool& bFoundFirstDigit );
    BASIC_DLLPRIVATE short  GetDigitAtPosExpScan( double dNewExponent, short nPos,
                                                  sal_Bool& bFoundFirstDigit );
    BASIC_DLLPRIVATE short  GetDigitAtPosExpScan( short nPos, sal_Bool& bFoundFirstDigit );
#else
    // Methods for direct 'calculation' with log10() and pow():
    BASIC_DLLPRIVATE short  GetDigitAtPos( double dNumber, short nPos, double& dNextNumber,
                                           sal_Bool& bFoundFirstDigit );
    BASIC_DLLPRIVATE short  RoundDigit( double dNumber );
#endif
    BASIC_DLLPRIVATE OUString GetPosFormatString( const OUString& sFormatStrg, sal_Bool & bFound );
    BASIC_DLLPRIVATE OUString GetNegFormatString( const OUString& sFormatStrg, sal_Bool & bFound );
    BASIC_DLLPRIVATE OUString Get0FormatString( const OUString& sFormatStrg, sal_Bool & bFound );
    BASIC_DLLPRIVATE OUString GetNullFormatString( const OUString& sFormatStrg, sal_Bool & bFound );
    BASIC_DLLPRIVATE short  AnalyseFormatString( const OUString& sFormatStrg,
                                                 short& nNoOfDigitsLeft, short& nNoOfDigitsRight,
                                                 short& nNoOfOptionalDigitsLeft,
                                                 short& nNoOfExponentDigits,
                                                 short& nNoOfOptionalExponentDigits,
                                                 sal_Bool& bPercent, sal_Bool& bCurrency, sal_Bool& bScientific,
                                                 sal_Bool& bGenerateThousandSeparator,
                                                 short& nMultipleThousandSeparators );
    BASIC_DLLPRIVATE void   ScanFormatString( double dNumber, const OUString& sFormatStrg,
                                              OUString& sReturnStrg, sal_Bool bCreateSign );

    //*** Data ***
    sal_Unicode cDecPoint;      // sign for the decimal point
    sal_Unicode cThousandSep;   // sign for thousand delimiter
    // Text for output:
    OUString  sOnStrg;
    OUString  sOffStrg;
    OUString  sYesStrg;
    OUString  sNoStrg;
    OUString  sTrueStrg;
    OUString  sFalseStrg;
    OUString  sCurrencyStrg;
    OUString  sCurrencyFormatStrg;

    //*** temporary data for scan loop ***

    // String containing the number in scientific format
    OUString  sSciNumStrg;
    // String containing the exponent of the number
    OUString  sNumExpStrg;
    double  dNum;           // the number that is scanned
    short   nNumExp;        // the exponent of the number
    short   nExpExp;        // the number of digits in the exponent
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
