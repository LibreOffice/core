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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_basic.hxx"

#include <stdlib.h>

#include <basic/sbxform.hxx>

/*
TODO: are there any Star-Basic characteristics unconsidered?

        what means: * as placeholder

COMMENT: Visual-Basic treats the following (invalid) format-strings
      as shown:

        ##0##.##0##     --> ##000.000##

      (this class behaves the same way)
*/

#include <stdio.h>          // for: sprintf()
#include <float.h>          // for: DBL_DIG, DBL_EPSILON
#include <math.h>           // for: floor(), fabs(), log10(), pow()

//=================================================================
//=========================== DEFINES =============================
//=================================================================

#define _NO_DIGIT                   -1

#define MAX_NO_OF_EXP_DIGITS        5
                    // +4 because of the value range: between -308 and +308
                    // +1 for closing 0
#define MAX_NO_OF_DIGITS            DBL_DIG
#define MAX_DOUBLE_BUFFER_LENGTH    MAX_NO_OF_DIGITS + 9
                    // +1 for leading sign
                    // +1 for digit before the decimal point
                    // +1 for decimal point
                    // +2 for exponent E and exp. leading sign
                    // +3 for the exponent's value
                    // +1 for closing 0

// Defines for the digits:
#define ASCII_0                     '0' // 48
#define ASCII_9                     '9' // 57

#define CREATE_1000SEP_CHAR         '@'

#define FORMAT_SEPARATOR            ';'

// predefined formats for the Format$()-command:
#define BASICFORMAT_GENERALNUMBER   "General Number"
#define BASICFORMAT_CURRENCY        "Currency"
#define BASICFORMAT_FIXED           "Fixed"
#define BASICFORMAT_STANDARD        "Standard"
#define BASICFORMAT_PERCENT         "Percent"
#define BASICFORMAT_SCIENTIFIC      "Scientific"
#define BASICFORMAT_YESNO           "Yes/No"
#define BASICFORMAT_TRUEFALSE       "True/False"
#define BASICFORMAT_ONOFF           "On/Off"

#define EMPTYFORMATSTRING           ""

// Comment: Visual-Basic has a maximum of 12 positions after the
//          decimal point for floating-point-numbers.
// all format-strings are compatible to Visual-Basic:
#define GENERALNUMBER_FORMAT        "0.############"
#define CURRENCY_FORMAT             "@$0.00;@($0.00)"
#define FIXED_FORMAT                "0.00"
#define STANDARD_FORMAT             "@0.00"
#define PERCENT_FORMAT              "0.00%"
#define SCIENTIFIC_FORMAT           "#.00E+00"
// Comment: the character @ means that thousand-separators shall
//          be generated. That's a StarBasic 'extension'.

//=================================================================


double get_number_of_digits( double dNumber )
//double floor_log10_fabs( double dNumber )
{
    if( dNumber==0.0 )
        return 0.0; // used to be 1.0, now 0.0 because of #40025;
    else
        return floor( log10( fabs( dNumber ) ) );
}

//=================================================================
//======================= IMPLEMENTATION ==========================
//=================================================================

SbxBasicFormater::SbxBasicFormater( sal_Unicode _cDecPoint, sal_Unicode _cThousandSep,
                      String _sOnStrg,
                      String _sOffStrg,
                      String _sYesStrg,
                      String _sNoStrg,
                      String _sTrueStrg,
                      String _sFalseStrg,
                      String _sCurrencyStrg,
                      String _sCurrencyFormatStrg )
{
    cDecPoint = _cDecPoint;
    cThousandSep = _cThousandSep;
    sOnStrg = _sOnStrg;
    sOffStrg = _sOffStrg;
    sYesStrg = _sYesStrg;
    sNoStrg = _sNoStrg;
    sTrueStrg = _sTrueStrg;
    sFalseStrg = _sFalseStrg;
    sCurrencyStrg = _sCurrencyStrg;
    sCurrencyFormatStrg = _sCurrencyFormatStrg;
}

// function for ouput of a error-text (for debugging)
// displaces all characters of the string, starting from nStartPos
// for one position to larger indexes, i. e. place for a new
// character (which is to be inserted) is created.
// ATTENTION: the string MUST be long enough!
inline void SbxBasicFormater::ShiftString( String& sStrg, sal_uInt16 nStartPos )
{
    sStrg.Erase( nStartPos,1 );
}

inline void SbxBasicFormater::StrAppendChar( String& sStrg, sal_Unicode ch )
{
    sStrg.Insert( ch );
}

void SbxBasicFormater::AppendDigit( String& sStrg, short nDigit )
{
    if( nDigit>=0 && nDigit<=9 )
        StrAppendChar( sStrg, (sal_Unicode)(nDigit+ASCII_0) );
}

void SbxBasicFormater::LeftShiftDecimalPoint( String& sStrg )
{
    sal_uInt16 nPos = sStrg.Search( cDecPoint );

    if( nPos!=STRING_NOTFOUND )
    {
        // swap decimal point
        sStrg.SetChar( nPos, sStrg.GetChar( nPos - 1 ) );
        sStrg.SetChar( nPos-1, cDecPoint );
    }
}

// returns a flag if rounding a 9
void SbxBasicFormater::StrRoundDigit( String& sStrg, short nPos, sal_Bool& bOverflow )
{
    if( nPos<0 )
        return;

    bOverflow = sal_False;

    sal_Unicode c = sStrg.GetChar( nPos );
    if( nPos>0 && (c == cDecPoint || c == cThousandSep) )
    {
        StrRoundDigit( sStrg,nPos-1,bOverflow );
        // CHANGE from 9.3.1997: end the method immediately after recursive call!
        return;
    }
    // skip non-digits:
    // COMMENT:
    // in a valid format-string the number's output should be done
    // in one piece, i. e. special characters should ONLY be in
    // front OR behind the number and not right in the middle of
    // the format information for the number
    while( nPos>=0 && (sStrg.GetChar( nPos )<ASCII_0 || sStrg.GetChar( nPos )>ASCII_9) )
        nPos--;

    if( nPos==-1 )
    {
        ShiftString( sStrg,0 );
        sStrg.SetChar( 0, '1' );
        bOverflow = sal_True;
    }
    else
    {
        sal_Unicode c2 = sStrg.GetChar( nPos );
        if( c2 >= ASCII_0 && c2 <= ASCII_9 )
        {
            if( c2 == ASCII_9 )
            {
                sStrg.SetChar( nPos, '0' );
                StrRoundDigit( sStrg,nPos-1,bOverflow );
            }
            else
                sStrg.SetChar( nPos, c2+1 );
        }
        else
        {
            ShiftString( sStrg,nPos+1 );
            sStrg.SetChar( nPos+1, '1' );
            bOverflow = sal_True;
        }
    }
}


void SbxBasicFormater::StrRoundDigit( String& sStrg, short nPos )
{
    sal_Bool bOverflow;

    StrRoundDigit( sStrg,nPos,bOverflow );
}

void SbxBasicFormater::ParseBack( String& sStrg, const String& sFormatStrg,
                                  short nFormatPos )
{
    for( short i=nFormatPos;
         i>0 && sFormatStrg.GetChar( i ) == '#' && sStrg.GetChar( (sStrg.Len()-1) ) == '0';
         i-- )
         { sStrg.Erase( sStrg.Len()-1 ); }
}

#ifdef _with_sprintf


void SbxBasicFormater::InitScan( double _dNum )
{
    char sBuffer[ MAX_DOUBLE_BUFFER_LENGTH ];

    dNum = _dNum;
    InitExp( get_number_of_digits( dNum ) );
    // maximum of 15 positions behind the decimal point, example: -1.234000000000000E-001
    /*int nCount =*/ sprintf( sBuffer,"%+22.15lE",dNum );
    sSciNumStrg.AssignAscii( sBuffer );
}


void SbxBasicFormater::InitExp( double _dNewExp )
{
    char sBuffer[ MAX_DOUBLE_BUFFER_LENGTH ];
    nNumExp = (short)_dNewExp;
    /*int nCount =*/ sprintf( sBuffer,"%+i",nNumExp );
    sNumExpStrg.AssignAscii( sBuffer );
    nExpExp = (short)get_number_of_digits( (double)nNumExp );
}


short SbxBasicFormater::GetDigitAtPosScan( short nPos, sal_Bool& bFoundFirstDigit )
{
    // trying to read a higher digit,
    // e. g. position 4 in 1.234,
    // or to read a digit outside of the
    // number's dissolution (double)
    if( nPos>nNumExp || abs(nNumExp-nPos)>MAX_NO_OF_DIGITS )
        return _NO_DIGIT;
    // determine the index of the position in the number-string:
    // skip the leading sign
    sal_uInt16 no = 1;
    // skip the decimal point if necessary
    if( nPos<nNumExp )
        no++;
    no += nNumExp-nPos;
    // query of the number's first valid digit --> set flag
    if( nPos==nNumExp )
        bFoundFirstDigit = sal_True;
    return (short)(sSciNumStrg.GetChar( no ) - ASCII_0);
}

short SbxBasicFormater::GetDigitAtPosExpScan( short nPos, sal_Bool& bFoundFirstDigit )
{
    if( nPos>nExpExp )
        return -1;

    sal_uInt16 no = 1;
    no += nExpExp-nPos;

    if( nPos==nExpExp )
        bFoundFirstDigit = sal_True;
    return (short)(sNumExpStrg.GetChar( no ) - ASCII_0);
}

// a value for the exponent can be given because the number maybe shall
// not be displayed in a normed way (e. g. 1.2345e-03) but maybe 123.345e-3 !
short SbxBasicFormater::GetDigitAtPosExpScan( double dNewExponent, short nPos,
                                              sal_Bool& bFoundFirstDigit )
{
    InitExp( dNewExponent );

    return GetDigitAtPosExpScan( nPos,bFoundFirstDigit );
}

#else

/* Problems with the following method:

TODO: an 'intelligent' peek-parser might be needed to detect rounding
      mistakes at double-numbers - e. g. for  0.00115 #.#e-000

  problem with: format( 0.3345 ,  "0.000" )
  problem with: format( 0.00115 , "0.0000" )

*/
// returns the digit at the given '10 system'-position,
// i. e. positive nPos for positions before the decimal
// point and negative for positions after.
// nPos==0 means first position after the decimalpoint, so 10^0.
// returns 0..9 for valid digits and -1 for not existing,
// i. e. if the passed number is too small
// (e. g. position 5 of dNumber=123).
// Furthermore in dNextNumber the number shorted by leading
// positions (till nPos) is returned, e. g.
//   GetDigitAtPos( 3434.565 , 2 , dNewNumber ) --> dNewNumber = 434.565
// In bFoundFirstDigit a flag is set if a digit has been found,
// this is used to prevent 'errors' on parsing 202
// ATTENTION: apparently there are sometimes still problems with rounding mistakes!
short SbxBasicFormater::GetDigitAtPos( double dNumber, short nPos,
                                double& dNextNumber, sal_Bool& bFoundFirstDigit )
{
    double dDigit;
    short  nMaxDigit;

    dNumber = fabs( dNumber );

    nMaxDigit = (short)get_number_of_digits( dNumber );
    // error only at numbers > 0, i. e. for digits before
    // the decimal point
    if( nMaxDigit<nPos && !bFoundFirstDigit && nPos>=0 )
        return _NO_DIGIT;

    bFoundFirstDigit = sal_True;
    for( short i=nMaxDigit; i>=nPos; i-- )
    {
        double dI = (double)i;
        double dTemp1 = pow( 10.0,dI );

        dDigit = floor( pow( 10.0,log10( fabs( dNumber ) )-dI ) );
        dNumber -= dTemp1 * dDigit;
    }
    // for optimized loop run
    dNextNumber = dNumber;

    return RoundDigit( dDigit );
}


short SbxBasicFormater::RoundDigit( double dNumber )
{
    if( dNumber<0.0 || dNumber>10.0 )
        return -1;
    short nTempHigh = (short)(dNumber+0.5); // maybe floor( )
    return nTempHigh;
}

#endif

// Copies the respective part of the format-string, if existing, and returns it.
// So a new string is created, which has to be freed by the caller later.
String SbxBasicFormater::GetPosFormatString( const String& sFormatStrg, sal_Bool & bFound )
{
    bFound = sal_False;     // default...
    sal_uInt16 nPos = sFormatStrg.Search( FORMAT_SEPARATOR );

    if( nPos!=STRING_NOTFOUND )
    {
        bFound = sal_True;
        // the format-string for positive numbers is
        // everything before the first ';'
        return sFormatStrg.Copy( 0,nPos );
    }

    String aRetStr;
    aRetStr.AssignAscii( EMPTYFORMATSTRING );
    return aRetStr;
}

// see also GetPosFormatString()
String SbxBasicFormater::GetNegFormatString( const String& sFormatStrg, sal_Bool & bFound )
{
    bFound = sal_False;     // default...
    sal_uInt16 nPos = sFormatStrg.Search( FORMAT_SEPARATOR );

    if( nPos!=STRING_NOTFOUND )
    {
        // the format-string for negative numbers is
        // everything between the first and the second ';'
        String sTempStrg = sFormatStrg.Copy( nPos+1 );
        nPos = sTempStrg.Search( FORMAT_SEPARATOR );
        bFound = sal_True;
        if( nPos==STRING_NOTFOUND )
            return sTempStrg;
        else
            return sTempStrg.Copy( 0,nPos );
    }
    String aRetStr;
    aRetStr.AssignAscii( EMPTYFORMATSTRING );
    return aRetStr;
}

// see also GetPosFormatString()
String SbxBasicFormater::Get0FormatString( const String& sFormatStrg, sal_Bool & bFound )
{
    bFound = sal_False;     // default...
    sal_uInt16 nPos = sFormatStrg.Search( FORMAT_SEPARATOR );

    if( nPos!=STRING_NOTFOUND )
    {
        // the format string for the zero is
        // everything after the second ';'
        String sTempStrg = sFormatStrg.Copy( nPos+1 );
        nPos = sTempStrg.Search( FORMAT_SEPARATOR );
        if( nPos!=STRING_NOTFOUND )
        {
            bFound = sal_True;
            sTempStrg = sTempStrg.Copy( nPos+1 );
            nPos = sTempStrg.Search( FORMAT_SEPARATOR );
            if( nPos==STRING_NOTFOUND )
                return sTempStrg;
            else
                return sTempStrg.Copy( 0,nPos );
        }
    }

    String aRetStr;
    aRetStr.AssignAscii( EMPTYFORMATSTRING );
    return aRetStr;
}

// see also GetPosFormatString()
String SbxBasicFormater::GetNullFormatString( const String& sFormatStrg, sal_Bool & bFound )
{
    bFound = sal_False;     // default...
    sal_uInt16 nPos = sFormatStrg.Search( FORMAT_SEPARATOR );

    if( nPos!=STRING_NOTFOUND )
    {
        // the format-string for the Null is
        // everything after the third ';'
        String sTempStrg = sFormatStrg.Copy( nPos+1 );
        nPos = sTempStrg.Search( FORMAT_SEPARATOR );
        if( nPos!=STRING_NOTFOUND )
        {
            sTempStrg = sTempStrg.Copy( nPos+1 );
            nPos = sTempStrg.Search( FORMAT_SEPARATOR );
            if( nPos!=STRING_NOTFOUND )
            {
                bFound = sal_True;
                return sTempStrg.Copy( nPos+1 );
            }
        }
    }

    String aRetStr;
    aRetStr.AssignAscii( EMPTYFORMATSTRING );
    return aRetStr;
}


// returns value <> 0 in case of an error
short SbxBasicFormater::AnalyseFormatString( const String& sFormatStrg,
                short& nNoOfDigitsLeft, short& nNoOfDigitsRight,
                short& nNoOfOptionalDigitsLeft,
                short& nNoOfExponentDigits, short& nNoOfOptionalExponentDigits,
                sal_Bool& bPercent, sal_Bool& bCurrency, sal_Bool& bScientific,
                sal_Bool& bGenerateThousandSeparator,
                short& nMultipleThousandSeparators )
{
    sal_uInt16 nLen;
    short nState = 0;

    nLen = sFormatStrg.Len();
    nNoOfDigitsLeft = 0;
    nNoOfDigitsRight = 0;
    nNoOfOptionalDigitsLeft = 0;
    nNoOfExponentDigits = 0;
    nNoOfOptionalExponentDigits = 0;
    bPercent = sal_False;
    bCurrency = sal_False;
    bScientific = sal_False;
    // from 11.7.97: as soon as a comma (point?) is found in the format string,
    // all three decimal powers are marked (i. e. thousand, million, ...)
    bGenerateThousandSeparator = sFormatStrg.Search( ',' ) != STRING_NOTFOUND;
    nMultipleThousandSeparators = 0;

    for( sal_uInt16 i=0; i<nLen; i++ )
    {
        sal_Unicode c = sFormatStrg.GetChar( i );
        switch( c ) {
            case '#':
            case '0':
                if( nState==0 )
                {
                    nNoOfDigitsLeft++;
// TODO  here maybe better error inspection of the mantissa for valid syntax (see grammar)h
                    // ATTENTION: 'undefined' behaviour if # and 0 are combined!
                    // REMARK: #-placeholders are actually useless for
                    // scientific display before the decimal point!
                    if( c=='#' )
                        nNoOfOptionalDigitsLeft++;
                }
                else if( nState==1 )
                    nNoOfDigitsRight++;
                else if( nState==-1 )   // search 0 in the exponent
                {
                    if( c=='#' )    // # switches on the condition
                    {
                        nNoOfOptionalExponentDigits++;
                        nState = -2;
                    }
                    nNoOfExponentDigits++;
                }
                else if( nState==-2 )   // search # in the exponent
                {
                    if( c=='0' )
                        // ERROR: 0 after # in the exponent is NOT allowed!!
                        return -4;
                    nNoOfOptionalExponentDigits++;
                    nNoOfExponentDigits++;
                }
                break;
            case '.':
                nState++;
                if( nState>1 )
                    return -1;  // ERROR: too many decimal points
                break;
            case '%':
                bPercent = sal_True;
                break;
            case '(':
                bCurrency = sal_True;
                break;
            case ',':
            {
                sal_Unicode ch = sFormatStrg.GetChar( i+1 );

                if( ch!=0 && (ch==',' || ch=='.') )
                    nMultipleThousandSeparators++;
            }   break;
            case 'e':
            case 'E':
                // #i13821 not when no digits before
                if( nNoOfDigitsLeft > 0 || nNoOfDigitsRight > 0 )
                {
                     nState = -1;   // abort counting digits
                    bScientific = sal_True;
                }
                break;
            // OWN command-character which turns on
            // the creation of thousand-separators
            case '\\':
                // Ignore next char
                i++;
                break;
            case CREATE_1000SEP_CHAR:
                bGenerateThousandSeparator = sal_True;
                break;
        }
    }
    return 0;
}

// the flag bCreateSign says that at the mantissa a leading sign
// shall be created
void SbxBasicFormater::ScanFormatString( double dNumber,
                                const String& sFormatStrg, String& sReturnStrg,
                                sal_Bool bCreateSign )
{
    short   /*nErr,*/nNoOfDigitsLeft,nNoOfDigitsRight,nNoOfOptionalDigitsLeft,
            nNoOfExponentDigits,nNoOfOptionalExponentDigits,
            nMultipleThousandSeparators;
    sal_Bool    bPercent,bCurrency,bScientific,bGenerateThousandSeparator;

    sReturnStrg = String();

    // analyse the format-string, i. e. determine the following values:
    /*
            - number of digits before decimal point
            - number of digits after decimal point
            - optional digits before decimal point
            - number of digits in the exponent
            - optional digits in the exponent
            - percent-character found?
            - () for negative leading sign?
            - exponetial-notation?
            - shall thousand-separators be generated?
            - is a percent-character being found? --> dNumber *= 100.0;
            - are there thousand-separators in a row?
                ,, or ,. --> dNumber /= 1000.0;
            - other errors? multiple decimal points, E's, etc.
        --> errors are simply ignored at the moment
    */
    AnalyseFormatString( sFormatStrg,nNoOfDigitsLeft,nNoOfDigitsRight,
                    nNoOfOptionalDigitsLeft,nNoOfExponentDigits,
                    nNoOfOptionalExponentDigits,
                    bPercent,bCurrency,bScientific,bGenerateThousandSeparator,
                    nMultipleThousandSeparators );
        // special handling for special characters
        if( bPercent )
            dNumber *= 100.0;
// TODO: this condition (,, or ,.) is NOT Visual-Basic compatible!
        // Question: shall this stay here (requirements)?
        if( nMultipleThousandSeparators )
            dNumber /= 1000.0;

        double dExponent;
        short i,nLen;
        short nState,nDigitPos,nExponentPos,nMaxDigit,nMaxExponentDigit;
        sal_Bool bFirstDigit,bFirstExponentDigit,bFoundFirstDigit,
             bIsNegative,bZeroSpaceOn, bSignHappend,bDigitPosNegative;

        bSignHappend = sal_False;
        bFoundFirstDigit = sal_False;
        bIsNegative = dNumber<0.0;
        nLen = sFormatStrg.Len();
        dExponent = get_number_of_digits( dNumber );
        nExponentPos = 0;
        nMaxExponentDigit = 0;
        nMaxDigit = (short)dExponent;
        bDigitPosNegative = false;
        if( bScientific )
        {
            dExponent = dExponent - (double)(nNoOfDigitsLeft-1);
            nDigitPos = nMaxDigit;
            nMaxExponentDigit = (short)get_number_of_digits( dExponent );
            nExponentPos = nNoOfExponentDigits-1 - nNoOfOptionalExponentDigits;
        }
        else
        {
            nDigitPos = nNoOfDigitsLeft-1; // counting starts at 0, 10^0
            // no exponent-data is needed here!
            bDigitPosNegative = (nDigitPos < 0);
        }
        bFirstDigit = sal_True;
        bFirstExponentDigit = sal_True;
        nState = 0; // 0 --> mantissa; 1 --> exponent
        bZeroSpaceOn = 0;


#ifdef _with_sprintf
        InitScan( dNumber );
#endif
        // scanning the format-string:
        sal_Unicode cForce = 0;
        for( i=0; i<nLen; i++ )
        {
            sal_Unicode c;
            if( cForce )
            {
                c = cForce;
                cForce = 0;
            }
            else
            {
                c = sFormatStrg.GetChar( i );
            }
            switch( c ) {
                case '0':
                case '#':
                    if( nState==0 )
                    {
                    // handling of the mantissa
                        if( bFirstDigit )
                        {
                            // remark: at bCurrency the negative
                            //         leading sign shall be shown with ()
                            if( bIsNegative && !bCreateSign && !bSignHappend )
                            {
                                bSignHappend = sal_True;
                                StrAppendChar( sReturnStrg,'-' );
                            }
                            // output redundant positions, i. e. those which
                            // are undocumented by the format-string
                            if( nMaxDigit>nDigitPos )
                            {
                                for( short j=nMaxDigit; j>nDigitPos; j-- )
                                {
                                    short nTempDigit;
#ifdef _with_sprintf
                                    AppendDigit( sReturnStrg,nTempDigit = GetDigitAtPosScan( j,bFoundFirstDigit ) );
#else
                                    AppendDigit( sReturnStrg,nTempDigit = GetDigitAtPos( dNumber,j,dNumber,bFoundFirstDigit ) );
#endif

                                    if( nTempDigit!=_NO_DIGIT )
                                        bFirstDigit = sal_False;

                                    if( bGenerateThousandSeparator && ( c=='0' || nMaxDigit>=nDigitPos ) && j>0 && (j % 3 == 0) )
                                        StrAppendChar( sReturnStrg,cThousandSep );
                                }
                            }
                        }

                        if( nMaxDigit<nDigitPos && ( c=='0' || bZeroSpaceOn ) )
                        {
                            AppendDigit( sReturnStrg,0 );

                            bFirstDigit = sal_False;
                            bZeroSpaceOn = 1;
                            // Remark: in Visual-Basic the first 0 turns on the 0 for
                            //         all the following # (up to the decimal point),
                            //         this behaviour is simulated here with the flag.

                            if( bGenerateThousandSeparator && ( c=='0' || nMaxDigit>=nDigitPos ) && nDigitPos>0 && (nDigitPos % 3 == 0) )
                                StrAppendChar( sReturnStrg,cThousandSep );
                        }
                        else
                        {
                            short nTempDigit;
#ifdef _with_sprintf
                            AppendDigit( sReturnStrg,nTempDigit = GetDigitAtPosScan( nDigitPos,bFoundFirstDigit ) );
#else
                            AppendDigit( sReturnStrg,nTempDigit = GetDigitAtPos( dNumber,nDigitPos,dNumber,bFoundFirstDigit ) );
#endif

                            if( nTempDigit!=_NO_DIGIT )
                                bFirstDigit = sal_False;

                            if( bGenerateThousandSeparator && ( c=='0' || nMaxDigit>=nDigitPos ) && nDigitPos>0 && (nDigitPos % 3 == 0) )
                                StrAppendChar( sReturnStrg,cThousandSep );
                        }

                        nDigitPos--;
                    }
                    else
                    {
                    // handling the exponent
                        if( bFirstExponentDigit )
                        {
                            // leading sign has been given out at e/E already
                            bFirstExponentDigit = sal_False;
                            if( nMaxExponentDigit>nExponentPos )
                            // output redundant positions, i. e. those which
                            // are undocumented by the format-string
                            {
                                for( short j=nMaxExponentDigit; j>nExponentPos; j-- )
                                {
#ifdef _with_sprintf
                                    AppendDigit( sReturnStrg,GetDigitAtPosExpScan( dExponent,j,bFoundFirstDigit ) );
#else
                                    AppendDigit( sReturnStrg,GetDigitAtPos( dExponent,j,dExponent,bFoundFirstDigit ) );
#endif
                                }
                            }
                        }

                        if( nMaxExponentDigit<nExponentPos && c=='0' )
                            AppendDigit( sReturnStrg,0 );
                        else
#ifdef _with_sprintf
                            AppendDigit( sReturnStrg,GetDigitAtPosExpScan( dExponent,nExponentPos,bFoundFirstDigit ) );
#else
                            AppendDigit( sReturnStrg,GetDigitAtPos( dExponent,nExponentPos,dExponent,bFoundFirstDigit ) );
#endif
                        nExponentPos--;
                    }
                    break;
                case '.':
                    if( bDigitPosNegative ) // #i13821: If no digits before .
                    {
                        bDigitPosNegative = false;
                        nDigitPos = 0;
                        cForce = '#';
                        i-=2;
                        break;
                    }
                    StrAppendChar( sReturnStrg,cDecPoint );
                    break;
                case '%':
                    // maybe remove redundant 0s, e. g. 4.500e4 in 0.0##e-00
                    ParseBack( sReturnStrg,sFormatStrg,i-1 );
                    sReturnStrg.Insert('%');
                    break;
                case 'e':
                case 'E':
                    // does mantissa have to be rounded, before the exponent is displayed?
                    {
                        // is there a mantissa at all?
                        if( bFirstDigit )
                        {
                            // apparently not, i. e. invalid format string, e. g. E000.00
                            // so ignore these e and E characters
                            // maybe output an error (like in Visual Basic)?

                            // #i13821: VB 6 behaviour
                            StrAppendChar( sReturnStrg,c );
                            break;
                        }

                        sal_Bool bOverflow = sal_False;
#ifdef _with_sprintf
                        short nNextDigit = GetDigitAtPosScan( nDigitPos,bFoundFirstDigit );
#else
                        short nNextDigit = GetDigitAtPos( dNumber,nDigitPos,dNumber,bFoundFirstDigit );
#endif
                        if( nNextDigit>=5 )
                            StrRoundDigit( sReturnStrg,sReturnStrg.Len()-1,bOverflow );
                        if( bOverflow )
                        {
                            // a leading 9 has been rounded
                            LeftShiftDecimalPoint( sReturnStrg );
                            sReturnStrg.SetChar( sReturnStrg.Len()-1 , 0 );
                            dExponent += 1.0;
                        }
                        // maybe remove redundant 0s, e. g. 4.500e4 in 0.0##e-00
                        ParseBack( sReturnStrg,sFormatStrg,i-1 );
                    }
                    // change the scanner's condition
                    nState++;
                    // output exponent character
                    StrAppendChar( sReturnStrg,c );
                    // i++; // MANIPULATION of the loop-variable!
                    c = sFormatStrg.GetChar( ++i );
                    // output leading sign / exponent
                    if( c!=0 )
                    {
                        if( c=='-' )
                        {
                            if( dExponent<0.0 )
                                StrAppendChar( sReturnStrg,'-' );
                        }
                        else if( c=='+' )
                        {
                            if( dExponent<0.0 )
                                StrAppendChar( sReturnStrg,'-' );
                            else
                                StrAppendChar( sReturnStrg,'+' );
                        }
                    }
                    break;
                case ',':
                    break;
                case ';':
                    break;
                case '(':
                case ')':
                    // maybe remove redundant 0s, e. g. 4.500e4 in 0.0##e-00
                    ParseBack( sReturnStrg,sFormatStrg,i-1 );
                    if( bIsNegative )
                        StrAppendChar( sReturnStrg,c );
                    break;
                case '$':
                    // append the string for the currency:
                    sReturnStrg += sCurrencyStrg;
                    break;
                case ' ':
                case '-':
                case '+':
                    ParseBack( sReturnStrg,sFormatStrg,i-1 );
                    StrAppendChar( sReturnStrg,c );
                    break;
                case '\\':
                    ParseBack( sReturnStrg,sFormatStrg,i-1 );
                    // special character found, output next
                    // character directly (if existing)
                    c = sFormatStrg.GetChar( ++i );
                    if( c!=0 )
                        StrAppendChar( sReturnStrg,c );
                    break;
                case CREATE_1000SEP_CHAR:
                    // ignore here, action has already been
                    // executed in AnalyseFormatString
                    break;
                default:
                    // output characters and digits, too (like in Visual-Basic)
                    if( ( c>='a' && c<='z' ) ||
                        ( c>='A' && c<='Z' ) ||
                        ( c>='1' && c<='9' ) )
                        StrAppendChar( sReturnStrg,c );
            }
        }

        // scan completed - rounding necessary?
        if( !bScientific )
        {
#ifdef _with_sprintf
            short nNextDigit = GetDigitAtPosScan( nDigitPos,bFoundFirstDigit );
#else
            short nNextDigit = GetDigitAtPos( dNumber,nDigitPos,dNumber,bFoundFirstDigit );
#endif
            if( nNextDigit>=5 )
                StrRoundDigit( sReturnStrg,sReturnStrg.Len()-1 );
        }

        if( nNoOfDigitsRight>0 )
            ParseBack( sReturnStrg,sFormatStrg,sFormatStrg.Len()-1 );
}

String SbxBasicFormater::BasicFormatNull( String sFormatStrg )
{
    sal_Bool bNullFormatFound;
    String sNullFormatStrg = GetNullFormatString( sFormatStrg,bNullFormatFound );

    if( bNullFormatFound )
        return sNullFormatStrg;
    String aRetStr;
    aRetStr.AssignAscii( "null" );
    return aRetStr;
}

String SbxBasicFormater::BasicFormat( double dNumber, String sFormatStrg )
{
    sal_Bool bPosFormatFound,bNegFormatFound,b0FormatFound;

    // analyse format-string concerning predefined formats:
    if( sFormatStrg.EqualsIgnoreCaseAscii( BASICFORMAT_GENERALNUMBER ) )
        sFormatStrg.AssignAscii( GENERALNUMBER_FORMAT );
    if( sFormatStrg.EqualsIgnoreCaseAscii( BASICFORMAT_CURRENCY ) )
        sFormatStrg = sCurrencyFormatStrg; // old: CURRENCY_FORMAT;
    if( sFormatStrg.EqualsIgnoreCaseAscii( BASICFORMAT_FIXED ) )
        sFormatStrg.AssignAscii( FIXED_FORMAT );
    if( sFormatStrg.EqualsIgnoreCaseAscii( BASICFORMAT_STANDARD ) )
        sFormatStrg.AssignAscii( STANDARD_FORMAT );
    if( sFormatStrg.EqualsIgnoreCaseAscii( BASICFORMAT_PERCENT ) )
        sFormatStrg.AssignAscii( PERCENT_FORMAT );
    if( sFormatStrg.EqualsIgnoreCaseAscii( BASICFORMAT_SCIENTIFIC ) )
        sFormatStrg.AssignAscii( SCIENTIFIC_FORMAT );
    if( sFormatStrg.EqualsIgnoreCaseAscii( BASICFORMAT_YESNO ) )
        return ( dNumber==0.0 ) ? sNoStrg : sYesStrg ;
    if( sFormatStrg.EqualsIgnoreCaseAscii( BASICFORMAT_TRUEFALSE ) )
        return ( dNumber==0.0 ) ? sFalseStrg : sTrueStrg ;
    if( sFormatStrg.EqualsIgnoreCaseAscii( BASICFORMAT_ONOFF ) )
        return ( dNumber==0.0 ) ? sOffStrg : sOnStrg ;

    // analyse format-string concerning ';', i. e. format-strings for
    // positive-, negative- and 0-values
    String sPosFormatStrg = GetPosFormatString( sFormatStrg, bPosFormatFound );
    String sNegFormatStrg = GetNegFormatString( sFormatStrg, bNegFormatFound );
    String s0FormatStrg = Get0FormatString( sFormatStrg, b0FormatFound );

    String sReturnStrg;
    String sTempStrg;

    if( dNumber==0.0 )
    {
        sTempStrg = sFormatStrg;
        if( b0FormatFound )
        {
            if( s0FormatStrg.Len() == 0 && bPosFormatFound )
                sTempStrg = sPosFormatStrg;
            else
                sTempStrg = s0FormatStrg;
        }
        else if( bPosFormatFound )
        {
            sTempStrg = sPosFormatStrg;
        }
        ScanFormatString( dNumber, sTempStrg, sReturnStrg,/*bCreateSign=*/sal_False );
    }
    else
    {
        if( dNumber<0.0 )
        {
            if( bNegFormatFound )
            {
                if( sNegFormatStrg.Len() == 0 && bPosFormatFound )
                {
                    sTempStrg = String::CreateFromAscii("-");
                    sTempStrg += sPosFormatStrg;
                }
                else
                    sTempStrg = sNegFormatStrg;
           }
            else
                sTempStrg = sFormatStrg;
            // if NO format-string especially for negative
            // values is given, output the leading sign
            ScanFormatString( dNumber, sTempStrg, sReturnStrg,/*bCreateSign=*/bNegFormatFound/*sNegFormatStrg!=EMPTYFORMATSTRING*/ );
        }
        else // if( dNumber>0.0 )
        {
            ScanFormatString( dNumber,
                    (/*sPosFormatStrg!=EMPTYFORMATSTRING*/bPosFormatFound ? sPosFormatStrg : sFormatStrg),
                    sReturnStrg,/*bCreateSign=*/sal_False );
        }
    }
    return sReturnStrg;
}

sal_Bool SbxBasicFormater::isBasicFormat( String sFormatStrg )
{
    if( sFormatStrg.EqualsIgnoreCaseAscii( BASICFORMAT_GENERALNUMBER ) )
        return sal_True;
    if( sFormatStrg.EqualsIgnoreCaseAscii( BASICFORMAT_CURRENCY ) )
        return sal_True;
    if( sFormatStrg.EqualsIgnoreCaseAscii( BASICFORMAT_FIXED ) )
        return sal_True;
    if( sFormatStrg.EqualsIgnoreCaseAscii( BASICFORMAT_STANDARD ) )
        return sal_True;
    if( sFormatStrg.EqualsIgnoreCaseAscii( BASICFORMAT_PERCENT ) )
        return sal_True;
    if( sFormatStrg.EqualsIgnoreCaseAscii( BASICFORMAT_SCIENTIFIC ) )
        return sal_True;
    if( sFormatStrg.EqualsIgnoreCaseAscii( BASICFORMAT_YESNO ) )
        return sal_True;
    if( sFormatStrg.EqualsIgnoreCaseAscii( BASICFORMAT_TRUEFALSE ) )
        return sal_True;
    if( sFormatStrg.EqualsIgnoreCaseAscii( BASICFORMAT_ONOFF ) )
        return sal_True;
    return sal_False;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
