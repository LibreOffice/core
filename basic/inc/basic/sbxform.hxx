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

#ifndef _SBXFORM_HXX
#define _SBXFORM_HXX

//====================================================================
// Implementation class for Basic command: Format$( d,formatStr )
//====================================================================
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

#include <tools/string.hxx>

class SbxBasicFormater {
  public:
    // Constructor takes signs for decimal point, thousand separation sign
    // and necessary resource strings.
    SbxBasicFormater( sal_Unicode _cDecPoint, sal_Unicode _cThousandSep,
                      String _sOnStrg,
                      String _sOffStrg,
                      String _sYesStrg,
                      String _sNoStrg,
                      String _sTrueStrg,
                      String _sFalseStrg,
                      String _sCurrencyStrg,
                      String _sCurrencyFormatStrg );

    /* Basic command: Format$( number,format-string )

       Parameter:
        dNumber     : number to be formated
        sFormatStrg : the Format-String, e.g. ###0.0###

       Return value:
        String containing the formatted output
    */
    String  BasicFormat( double dNumber, String sFormatStrg );
    String  BasicFormatNull( String sFormatStrg );

    static  sal_Bool isBasicFormat( String sFormatStrg );

  private:
    //*** some helper methods ***
    //void  ShowError( char *sErrMsg );
    inline void ShiftString( String& sStrg, sal_uInt16 nStartPos );
    inline void StrAppendChar( String& sStrg, sal_Unicode ch );
    void    AppendDigit( String& sStrg, short nDigit );
    void    LeftShiftDecimalPoint( String& sStrg );
    void    StrRoundDigit( String& sStrg, short nPos, sal_Bool& bOverflow );
    void    StrRoundDigit( String& sStrg, short nPos );
    void    ParseBack( String& sStrg, const String& sFormatStrg,
                short nFormatPos );
#ifdef _with_sprintf
    // Methods for string conversion with sprintf():
    void    InitScan( double _dNum );
    void    InitExp( double _dNewExp );
    short   GetDigitAtPosScan( short nPos, sal_Bool& bFoundFirstDigit );
    short   GetDigitAtPosExpScan( double dNewExponent, short nPos,
                sal_Bool& bFoundFirstDigit );
    short   GetDigitAtPosExpScan( short nPos, sal_Bool& bFoundFirstDigit );
#else
    // Methods for direct 'calculation' with log10() and pow():
    short   GetDigitAtPos( double dNumber, short nPos, double& dNextNumber,
                sal_Bool& bFoundFirstDigit );
    short   RoundDigit( double dNumber );
#endif
    String  GetPosFormatString( const String& sFormatStrg, sal_Bool & bFound );
    String  GetNegFormatString( const String& sFormatStrg, sal_Bool & bFound );
    String  Get0FormatString( const String& sFormatStrg, sal_Bool & bFound );
    String  GetNullFormatString( const String& sFormatStrg, sal_Bool & bFound );
    short   AnalyseFormatString( const String& sFormatStrg,
                short& nNoOfDigitsLeft, short& nNoOfDigitsRight,
                short& nNoOfOptionalDigitsLeft,
                short& nNoOfExponentDigits,
                short& nNoOfOptionalExponentDigits,
                sal_Bool& bPercent, sal_Bool& bCurrency, sal_Bool& bScientific,
                sal_Bool& bGenerateThousandSeparator,
                short& nMultipleThousandSeparators );
    void    ScanFormatString( double dNumber, const String& sFormatStrg,
                String& sReturnStrg, sal_Bool bCreateSign );

    //*** Data ***
    sal_Unicode cDecPoint;      // sign for the decimal point
    sal_Unicode cThousandSep;   // sign for thousand delimiter
    // Text for output:
    String  sOnStrg;
    String  sOffStrg;
    String  sYesStrg;
    String  sNoStrg;
    String  sTrueStrg;
    String  sFalseStrg;
    String  sCurrencyStrg;
    String  sCurrencyFormatStrg;

    //*** temporary data for scan loop ***
    //-----------------------------------------------
    // String containing the number in scientific format
    String  sSciNumStrg;
    // String containing the exponent of the number
    String  sNumExpStrg;
    double  dNum;           // the number that is scanned
    short   nNumExp;        // the exponent of the number
    short   nExpExp;        // the number of digits in the exponent
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
