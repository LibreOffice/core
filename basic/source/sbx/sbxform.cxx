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


#include <stdlib.h>

#include <basic/sbxform.hxx>
#include <rtl/ustrbuf.hxx>

/*
TODO: are there any Star-Basic characteristics unconsidered?

        what means: * as placeholder

COMMENT: Visual-Basic treats the following (invalid) format-strings
      as shown:

        ##0##.##0##     --> ##000.000##

      (this class behaves the same way)
*/

#include <stdio.h>
#include <float.h>
#include <math.h>

#define NO_DIGIT_                   -1

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

// Comment: Visual-Basic has a maximum of 12 positions after the
//          decimal point for floating-point-numbers.
// all format-strings are compatible to Visual-Basic:
#define GENERALNUMBER_FORMAT        "0.############"
#define FIXED_FORMAT                "0.00"
#define STANDARD_FORMAT             "@0.00"
#define PERCENT_FORMAT              "0.00%"
#define SCIENTIFIC_FORMAT           "#.00E+00"
// Comment: the character @ means that thousand-separators shall
//          be generated. That's a StarBasic 'extension'.


double get_number_of_digits( double dNumber )
//double floor_log10_fabs( double dNumber )
{
    if( dNumber==0.0 )
        return 0.0; // used to be 1.0, now 0.0 because of #40025;
    else
        return floor( log10( fabs( dNumber ) ) );
}


SbxBasicFormater::SbxBasicFormater( sal_Unicode _cDecPoint, sal_Unicode _cThousandSep,
                      const OUString& _sOnStrg,
                      const OUString& _sOffStrg,
                      const OUString& _sYesStrg,
                      const OUString& _sNoStrg,
                      const OUString& _sTrueStrg,
                      const OUString& _sFalseStrg,
                      const OUString& _sCurrencyStrg,
                      const OUString& _sCurrencyFormatStrg )
    : cDecPoint(_cDecPoint)
    , cThousandSep(_cThousandSep)
    , sOnStrg(_sOnStrg)
    , sOffStrg(_sOffStrg)
    , sYesStrg(_sYesStrg)
    , sNoStrg(_sNoStrg)
    , sTrueStrg(_sTrueStrg)
    , sFalseStrg(_sFalseStrg)
    , sCurrencyStrg(_sCurrencyStrg)
    , sCurrencyFormatStrg(_sCurrencyFormatStrg)
    , dNum(0.0)
    , nNumExp(0)
    , nExpExp(0)
{
}

// function for ouput of a error-text (for debugging)
// displaces all characters of the string, starting from nStartPos
// for one position to larger indexes, i. e. place for a new
// character (which is to be inserted) is created.
// ATTENTION: the string MUST be long enough!
inline void SbxBasicFormater::ShiftString( OUStringBuffer& sStrg, sal_uInt16 nStartPos )
{
    sStrg.remove(nStartPos,1);
}

void SbxBasicFormater::AppendDigit( OUStringBuffer& sStrg, short nDigit )
{
    if( nDigit>=0 && nDigit<=9 )
    {
        sStrg.append((sal_Unicode)(nDigit+ASCII_0));
    }
}

void SbxBasicFormater::LeftShiftDecimalPoint( OUStringBuffer& sStrg )
{
    sal_Int32 nPos = -1;

    for(sal_Int32 i = 0; i < sStrg.getLength(); i++)
    {
        if(sStrg[i] == cDecPoint)
        {
            nPos = i;
            break;
        }
    }
    if( nPos >= 0 )
    {
        sStrg[nPos] = sStrg[nPos - 1];
        sStrg[nPos - 1] = cDecPoint;
    }
}

// returns a flag if rounding a 9
void SbxBasicFormater::StrRoundDigit( OUStringBuffer& sStrg, short nPos, bool& bOverflow )
{
    if( nPos<0 )
    {
        return;
    }
    bOverflow = false;
    sal_Unicode c = sStrg[nPos];
    if( nPos > 0 && (c == cDecPoint || c == cThousandSep) )
    {
        StrRoundDigit( sStrg, nPos - 1, bOverflow );
        // CHANGE from 9.3.1997: end the method immediately after recursive call!
        return;
    }
    // skip non-digits:
    // COMMENT:
    // in a valid format-string the number's output should be done
    // in one piece, i. e. special characters should ONLY be in
    // front OR behind the number and not right in the middle of
    // the format information for the number
    while( nPos >= 0 && ( sStrg[nPos] < ASCII_0 || sStrg[nPos] > ASCII_9 ))
    {
        nPos--;
    }
    if( nPos==-1 )
    {
        ShiftString( sStrg, 0 );
        sStrg[0] = (sal_Unicode)'1';
        bOverflow = true;
    }
    else
    {
        sal_Unicode c2 = sStrg[nPos];
        if( c2 >= ASCII_0 && c2 <= ASCII_9 )
        {
            if( c2 == ASCII_9 )
            {
                sStrg[nPos] = (sal_Unicode)'0';
                StrRoundDigit( sStrg, nPos - 1, bOverflow );
            }
            else
            {
                sStrg[nPos] = c2 + 1;
            }
        }
        else
        {
            ShiftString( sStrg,nPos+1 );
            sStrg[nPos + 1] = (sal_Unicode)'1';
            bOverflow = true;
        }
    }
}

void SbxBasicFormater::StrRoundDigit( OUStringBuffer& sStrg, short nPos )
{
    bool bOverflow;

    StrRoundDigit( sStrg, nPos, bOverflow );
}

void SbxBasicFormater::ParseBack( OUStringBuffer& sStrg, const OUString& sFormatStrg,
                                  short nFormatPos )
{
    for( sal_Int32 i = nFormatPos;
         i>0 && sFormatStrg[ i ]  == (sal_Unicode)'#' && sStrg[sStrg.getLength() - 1] == (sal_Unicode)'0';
         i-- )
    {
        sStrg.setLength(sStrg.getLength() - 1 );
    }
}

#ifdef with_sprintf_


void SbxBasicFormater::InitScan( double _dNum )
{
    char sBuffer[ MAX_DOUBLE_BUFFER_LENGTH ];

    dNum = _dNum;
    InitExp( get_number_of_digits( dNum ) );
    // maximum of 15 positions behind the decimal point, example: -1.234000000000000E-001
    /*int nCount =*/ sprintf( sBuffer,"%+22.15lE",dNum );
    sSciNumStrg = OUString::createFromAscii( sBuffer );
}


void SbxBasicFormater::InitExp( double _dNewExp )
{
    char sBuffer[ MAX_DOUBLE_BUFFER_LENGTH ];
    nNumExp = (short)_dNewExp;
    /*int nCount =*/ sprintf( sBuffer,"%+i",nNumExp );
    sNumExpStrg = OUString::createFromAscii( sBuffer );
    nExpExp = (short)get_number_of_digits( (double)nNumExp );
}


short SbxBasicFormater::GetDigitAtPosScan( short nPos, bool& bFoundFirstDigit )
{
    // trying to read a higher digit,
    // e. g. position 4 in 1.234,
    // or to read a digit outside of the
    // number's dissolution (double)
    if( nPos>nNumExp || abs(nNumExp-nPos)>MAX_NO_OF_DIGITS )
    {
        return NO_DIGIT_;
    }
    // determine the index of the position in the number-string:
    // skip the leading sign
    sal_uInt16 no = 1;
    // skip the decimal point if necessary
    if( nPos<nNumExp )
        no++;
    no += nNumExp-nPos;
    // query of the number's first valid digit --> set flag
    if( nPos==nNumExp )
        bFoundFirstDigit = true;
    return (short)(sSciNumStrg[ no ] - ASCII_0);
}

short SbxBasicFormater::GetDigitAtPosExpScan( short nPos, bool& bFoundFirstDigit )
{
    if( nPos>nExpExp )
        return -1;

    sal_uInt16 no = 1;
    no += nExpExp-nPos;

    if( nPos==nExpExp )
        bFoundFirstDigit = true;
    return (short)(sNumExpStrg[ no ] - ASCII_0);
}

// a value for the exponent can be given because the number maybe shall
// not be displayed in a normed way (e. g. 1.2345e-03) but maybe 123.345e-3 !
short SbxBasicFormater::GetDigitAtPosExpScan( double dNewExponent, short nPos,
                                              bool& bFoundFirstDigit )
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
                                double& dNextNumber, bool& bFoundFirstDigit )
{
    double dDigit;
    short  nMaxDigit;

    dNumber = fabs( dNumber );

    nMaxDigit = (short)get_number_of_digits( dNumber );
    // error only at numbers > 0, i. e. for digits before
    // the decimal point
    if( nMaxDigit<nPos && !bFoundFirstDigit && nPos>=0 )
        return NO_DIGIT_;

    bFoundFirstDigit = true;
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
OUString SbxBasicFormater::GetPosFormatString( const OUString& sFormatStrg, bool & bFound )
{
    bFound = false;     // default...
    sal_Int32 nPos = sFormatStrg.indexOf( FORMAT_SEPARATOR );

    if( nPos >= 0 )
    {
        bFound = true;
        // the format-string for positive numbers is
        // everything before the first ';'
        return sFormatStrg.copy( 0,nPos );
    }

    return OUString();
}

// see also GetPosFormatString()
OUString SbxBasicFormater::GetNegFormatString( const OUString& sFormatStrg, bool & bFound )
{
    bFound = false;     // default...
    sal_Int32 nPos = sFormatStrg.indexOf( FORMAT_SEPARATOR );

    if( nPos >= 0)
    {
        // the format-string for negative numbers is
        // everything between the first and the second ';'
        OUString sTempStrg = sFormatStrg.copy( nPos+1 );
        nPos = sTempStrg.indexOf( FORMAT_SEPARATOR );
        bFound = true;
        if( nPos < 0 )
        {
            return sTempStrg;
        }
        else
        {
            return sTempStrg.copy( 0,nPos );
        }
    }
    return OUString();
}

// see also GetPosFormatString()
OUString SbxBasicFormater::Get0FormatString( const OUString& sFormatStrg, bool & bFound )
{
    bFound = false;     // default...
    sal_Int32 nPos = sFormatStrg.indexOf( FORMAT_SEPARATOR );

    if( nPos >= 0 )
    {
        // the format string for the zero is
        // everything after the second ';'
        OUString sTempStrg = sFormatStrg.copy( nPos+1 );
        nPos = sTempStrg.indexOf( FORMAT_SEPARATOR );
        if( nPos >= 0 )
        {
            bFound = true;
            sTempStrg = sTempStrg.copy( nPos+1 );
            nPos = sTempStrg.indexOf( FORMAT_SEPARATOR );
            if( nPos < 0 )
            {
                return sTempStrg;
            }
            else
            {
                return sTempStrg.copy( 0,nPos );
            }
        }
    }

    return OUString();
}

// see also GetPosFormatString()
OUString SbxBasicFormater::GetNullFormatString( const OUString& sFormatStrg, bool & bFound )
{
    bFound = false;     // default...
    sal_Int32 nPos = sFormatStrg.indexOf( FORMAT_SEPARATOR );

    if( nPos >= 0 )
    {
        // the format-string for the Null is
        // everything after the third ';'
        OUString sTempStrg = sFormatStrg.copy( nPos+1 );
        nPos = sTempStrg.indexOf( FORMAT_SEPARATOR );
        if( nPos >= 0 )
        {
            sTempStrg = sTempStrg.copy( nPos+1 );
            nPos = sTempStrg.indexOf( FORMAT_SEPARATOR );
            if( nPos >= 0 )
            {
                bFound = true;
                return sTempStrg.copy( nPos+1 );
            }
        }
    }

    return OUString();
}

// returns value <> 0 in case of an error
void SbxBasicFormater::AnalyseFormatString( const OUString& sFormatStrg,
                short& nNoOfDigitsLeft, short& nNoOfDigitsRight,
                short& nNoOfOptionalDigitsLeft,
                short& nNoOfExponentDigits, short& nNoOfOptionalExponentDigits,
                bool& bPercent, bool& bCurrency, bool& bScientific,
                bool& bGenerateThousandSeparator,
                short& nMultipleThousandSeparators )
{
    sal_Int32 nLen;
    short nState = 0;

    nLen = sFormatStrg.getLength();
    nNoOfDigitsLeft = 0;
    nNoOfDigitsRight = 0;
    nNoOfOptionalDigitsLeft = 0;
    nNoOfExponentDigits = 0;
    nNoOfOptionalExponentDigits = 0;
    bPercent = false;
    bCurrency = false;
    bScientific = false;
    // from 11.7.97: as soon as a comma (point?) is found in the format string,
    // all three decimal powers are marked (i. e. thousand, million, ...)
    bGenerateThousandSeparator = sFormatStrg.indexOf( ',' ) >= 0;
    nMultipleThousandSeparators = 0;

    for( sal_Int32 i = 0; i < nLen; i++ )
    {
        sal_Unicode c = sFormatStrg[ i ];
        switch( c )
        {
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
                {
                    nNoOfOptionalDigitsLeft++;
                }
            }
            else if( nState==1 )
            {
                nNoOfDigitsRight++;
            }
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
                {
                    // ERROR: 0 after # in the exponent is NOT allowed!!
                    return;
                }
                nNoOfOptionalExponentDigits++;
                nNoOfExponentDigits++;
            }
            break;
        case '.':
            nState++;
            if( nState>1 )
            {
                return;  // ERROR: too many decimal points
            }
            break;
        case '%':
            bPercent = true;
            break;
        case '(':
            bCurrency = true;
            break;
        case ',':
            {
                sal_Unicode ch = sFormatStrg[ i+1 ];

                if( ch!=0 && (ch==',' || ch=='.') )
                {
                    nMultipleThousandSeparators++;
                }
            }
            break;
        case 'e':
        case 'E':
            // #i13821 not when no digits before
            if( nNoOfDigitsLeft > 0 || nNoOfDigitsRight > 0 )
            {
                nState = -1;   // abort counting digits
                bScientific = true;
            }
            break;
            // OWN command-character which turns on
            // the creation of thousand-separators
        case '\\':
            // Ignore next char
            i++;
            break;
        case CREATE_1000SEP_CHAR:
            bGenerateThousandSeparator = true;
            break;
        }
    }
}

// the flag bCreateSign says that at the mantissa a leading sign
// shall be created
void SbxBasicFormater::ScanFormatString( double dNumber,
                                         const OUString& sFormatStrg, OUString& sReturnStrgFinal,
                                         bool bCreateSign )
{
    short   /*nErr,*/nNoOfDigitsLeft,nNoOfDigitsRight,nNoOfOptionalDigitsLeft,
        nNoOfExponentDigits,nNoOfOptionalExponentDigits,
        nMultipleThousandSeparators;
    bool    bPercent,bCurrency,bScientific,bGenerateThousandSeparator;

    OUStringBuffer sReturnStrg = OUStringBuffer();

    // analyse the format-string, i. e. determine the following values:
    /*
            - number of digits before decimal point
            - number of digits after decimal point
            - optional digits before decimal point
            - number of digits in the exponent
            - optional digits in the exponent
            - percent-character found?
            - () for negative leading sign?
            - exponential-notation?
            - shall thousand-separators be generated?
            - is a percent-character being found? --> dNumber *= 100.0;
            - are there thousand-separators in a row?
                ,, or ,. --> dNumber /= 1000.0;
            - other errors? multiple decimal points, E's, etc.
        --> errors are simply ignored at the moment
    */
    AnalyseFormatString( sFormatStrg, nNoOfDigitsLeft, nNoOfDigitsRight,
                         nNoOfOptionalDigitsLeft, nNoOfExponentDigits,
                         nNoOfOptionalExponentDigits,
                         bPercent, bCurrency, bScientific,
                         bGenerateThousandSeparator, nMultipleThousandSeparators );
    // special handling for special characters
    if( bPercent )
    {
        dNumber *= 100.0;
    }
// TODO: this condition (,, or ,.) is NOT Visual-Basic compatible!
        // Question: shall this stay here (requirements)?
    if( nMultipleThousandSeparators )
    {
        dNumber /= 1000.0;
    }
    double dExponent;
    short i,nLen;
    short nState,nDigitPos,nExponentPos,nMaxDigit,nMaxExponentDigit;
    bool bFirstDigit,bFirstExponentDigit,bFoundFirstDigit,
        bIsNegative,bZeroSpaceOn, bSignHappend,bDigitPosNegative;

    bSignHappend = false;
    bFoundFirstDigit = false;
    bIsNegative = dNumber < 0.0;
    nLen = sFormatStrg.getLength();
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
        nExponentPos = nNoOfExponentDigits - 1 - nNoOfOptionalExponentDigits;
    }
    else
    {
        nDigitPos = nNoOfDigitsLeft - 1; // counting starts at 0, 10^0
        // no exponent-data is needed here!
        bDigitPosNegative = (nDigitPos < 0);
    }
    bFirstDigit = true;
    bFirstExponentDigit = true;
    nState = 0; // 0 --> mantissa; 1 --> exponent
    bZeroSpaceOn = false;


#ifdef with_sprintf_
    InitScan( dNumber );
#endif
    // scanning the format-string:
    sal_Unicode cForce = 0;
    for( i = 0; i < nLen; i++ )
    {
        sal_Unicode c;
        if( cForce )
        {
            c = cForce;
            cForce = 0;
        }
        else
        {
            c = sFormatStrg[ i ];
        }
        switch( c )
        {
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
                        bSignHappend = true;
                        sReturnStrg.append('-');
                    }
                    // output redundant positions, i. e. those which
                    // are undocumented by the format-string
                    if( nMaxDigit > nDigitPos )
                    {
                        for( short j = nMaxDigit; j > nDigitPos; j-- )
                        {
                            short nTempDigit;
#ifdef with_sprintf_
                            AppendDigit( sReturnStrg, nTempDigit = GetDigitAtPosScan( j, bFoundFirstDigit ) );
#else
                            AppendDigit( sReturnStrg, nTempDigit = GetDigitAtPos( dNumber, j, dNumber, bFoundFirstDigit ) );
#endif
                            if( nTempDigit != NO_DIGIT_ )
                            {
                                bFirstDigit = false;
                            }
                            if( bGenerateThousandSeparator && ( c=='0' || nMaxDigit >= nDigitPos ) && j > 0 && (j % 3 == 0) )
                            {
                                sReturnStrg.append(cThousandSep );
                            }
                        }
                    }
                }

                if( nMaxDigit<nDigitPos && ( c=='0' || bZeroSpaceOn ) )
                {
                    AppendDigit( sReturnStrg, 0 );
                    bFirstDigit = false;
                    bZeroSpaceOn = true;
                    // Remark: in Visual-Basic the first 0 turns on the 0 for
                    //         all the following # (up to the decimal point),
                    //         this behaviour is simulated here with the flag.
                    if( bGenerateThousandSeparator && ( c=='0' || nMaxDigit >= nDigitPos ) && nDigitPos > 0 && (nDigitPos % 3 == 0) )
                    {
                        sReturnStrg.append(cThousandSep);
                    }
                }
                else
                {
                    short nTempDigit;
#ifdef with_sprintf_
                    AppendDigit( sReturnStrg, nTempDigit = GetDigitAtPosScan( nDigitPos, bFoundFirstDigit ) );
#else
                    AppendDigit( sReturnStrg, nTempDigit = GetDigitAtPos( dNumber, nDigitPos, dNumber, bFoundFirstDigit ) );
#endif

                    if( nTempDigit != NO_DIGIT_ )
                    {
                        bFirstDigit = false;
                    }
                    if( bGenerateThousandSeparator && ( c=='0' || nMaxDigit>=nDigitPos ) && nDigitPos>0 && (nDigitPos % 3 == 0) )
                    {
                        sReturnStrg.append(cThousandSep);
                    }
                }
                nDigitPos--;
            }
            else
            {
                // handling the exponent
                if( bFirstExponentDigit )
                {
                    // leading sign has been given out at e/E already
                    bFirstExponentDigit = false;
                    if( nMaxExponentDigit > nExponentPos )
                        // output redundant positions, i. e. those which
                        // are undocumented by the format-string
                    {
                        for( short j = nMaxExponentDigit; j > nExponentPos; j-- )
                        {
#ifdef with_sprintf_
                            AppendDigit( sReturnStrg, GetDigitAtPosExpScan( dExponent, j, bFoundFirstDigit ) );
#else
                            AppendDigit( sReturnStrg,GetDigitAtPos( dExponent, j, dExponent, bFoundFirstDigit ) );
#endif
                        }
                    }
                }

                if( nMaxExponentDigit < nExponentPos && c=='0' )
                {
                    AppendDigit( sReturnStrg, 0 );
                }
                else
                {
#ifdef with_sprintf_
                    AppendDigit( sReturnStrg, GetDigitAtPosExpScan( dExponent, nExponentPos, bFoundFirstDigit ) );
#else
                    AppendDigit( sReturnStrg, GetDigitAtPos( dExponent, nExponentPos, dExponent, bFoundFirstDigit ) );
#endif
                }
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
            sReturnStrg.append(cDecPoint);
            break;
        case '%':
            // maybe remove redundant 0s, e. g. 4.500e4 in 0.0##e-00
            ParseBack( sReturnStrg, sFormatStrg, i-1 );
            sReturnStrg.append('%');
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
                    sReturnStrg.append(c);
                    break;
                }

                bool bOverflow = false;
#ifdef with_sprintf_
                short nNextDigit = GetDigitAtPosScan( nDigitPos, bFoundFirstDigit );
#else
                short nNextDigit = GetDigitAtPos( dNumber, nDigitPos, dNumber, bFoundFirstDigit );
#endif
                if( nNextDigit>=5 )
                {
                    StrRoundDigit( sReturnStrg, sReturnStrg.getLength() - 1, bOverflow );
                }
                if( bOverflow )
                {
                    // a leading 9 has been rounded
                    LeftShiftDecimalPoint( sReturnStrg );
                    sReturnStrg[sReturnStrg.getLength() - 1] = 0;
                    dExponent += 1.0;
                }
                // maybe remove redundant 0s, e. g. 4.500e4 in 0.0##e-00
                ParseBack( sReturnStrg, sFormatStrg, i-1 );
            }
            // change the scanner's condition
            nState++;
            // output exponent character
            sReturnStrg.append(c);
            // i++; // MANIPULATION of the loop-variable!
            c = sFormatStrg[ ++i ];
            // output leading sign / exponent
            if( c != 0 )
            {
                if( c == '-' )
                {
                    if( dExponent < 0.0 )
                    {
                        sReturnStrg.append('-');
                    }
                }
                else if( c == '+' )
                {
                    if( dExponent < 0.0 )
                    {
                        sReturnStrg.append('-');
                    }
                    else
                    {
                        sReturnStrg.append('+');
                    }
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
            ParseBack( sReturnStrg, sFormatStrg, i-1 );
            if( bIsNegative )
            {
                sReturnStrg.append(c);
            }
            break;
        case '$':
            // append the string for the currency:
            sReturnStrg.append(sCurrencyStrg);
            break;
        case ' ':
        case '-':
        case '+':
            ParseBack( sReturnStrg, sFormatStrg, i-1 );
            sReturnStrg.append(c);
            break;
        case '\\':
            ParseBack( sReturnStrg, sFormatStrg, i-1 );
            // special character found, output next
            // character directly (if existing)
            c = sFormatStrg[ ++i ];
            if( c!=0 )
            {
                sReturnStrg.append(c);
            }
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
            {
                sReturnStrg.append(c);
            }
        }
    }

    // scan completed - rounding necessary?
    if( !bScientific )
    {
#ifdef with_sprintf_
        short nNextDigit = GetDigitAtPosScan( nDigitPos, bFoundFirstDigit );
#else
        short nNextDigit = GetDigitAtPos( dNumber, nDigitPos, dNumber, bFoundFirstDigit );
#endif
        if( nNextDigit>=5 )
        {
            StrRoundDigit( sReturnStrg, sReturnStrg.getLength() - 1 );
        }
    }

    if( nNoOfDigitsRight>0 )
    {
        ParseBack( sReturnStrg, sFormatStrg, sFormatStrg.getLength()-1 );
    }
    sReturnStrgFinal = sReturnStrg.makeStringAndClear();
}

OUString SbxBasicFormater::BasicFormatNull( const OUString& sFormatStrg )
{
    bool bNullFormatFound;
    OUString sNullFormatStrg = GetNullFormatString( sFormatStrg, bNullFormatFound );

    if( bNullFormatFound )
    {
        return sNullFormatStrg;
    }
    return OUString("null");
}

OUString SbxBasicFormater::BasicFormat( double dNumber, const OUString& _sFormatStrg )
{
    bool bPosFormatFound,bNegFormatFound,b0FormatFound;
    OUString sFormatStrg = _sFormatStrg;

    // analyse format-string concerning predefined formats:
    if( sFormatStrg.equalsIgnoreAsciiCase( BASICFORMAT_GENERALNUMBER ) )
    {
        sFormatStrg = GENERALNUMBER_FORMAT;
    }
    if( sFormatStrg.equalsIgnoreAsciiCase( BASICFORMAT_CURRENCY ) )
    {
        sFormatStrg = sCurrencyFormatStrg;
    }
    if( sFormatStrg.equalsIgnoreAsciiCase( BASICFORMAT_FIXED ) )
    {
        sFormatStrg = FIXED_FORMAT;
    }
    if( sFormatStrg.equalsIgnoreAsciiCase( BASICFORMAT_STANDARD ) )
    {
        sFormatStrg = STANDARD_FORMAT;
    }
    if( sFormatStrg.equalsIgnoreAsciiCase( BASICFORMAT_PERCENT ) )
    {
        sFormatStrg = PERCENT_FORMAT;
    }
    if( sFormatStrg.equalsIgnoreAsciiCase( BASICFORMAT_SCIENTIFIC ) )
    {
        sFormatStrg = SCIENTIFIC_FORMAT;
    }
    if( sFormatStrg.equalsIgnoreAsciiCase( BASICFORMAT_YESNO ) )
    {
        return ( dNumber==0.0 ) ? sNoStrg : sYesStrg ;
    }
    if( sFormatStrg.equalsIgnoreAsciiCase( BASICFORMAT_TRUEFALSE ) )
    {
        return ( dNumber==0.0 ) ? sFalseStrg : sTrueStrg ;
    }
    if( sFormatStrg.equalsIgnoreAsciiCase( BASICFORMAT_ONOFF ) )
    {
        return ( dNumber==0.0 ) ? sOffStrg : sOnStrg ;
    }

    // analyse format-string concerning ';', i. e. format-strings for
    // positive-, negative- and 0-values
    OUString sPosFormatStrg = GetPosFormatString( sFormatStrg, bPosFormatFound );
    OUString sNegFormatStrg = GetNegFormatString( sFormatStrg, bNegFormatFound );
    OUString s0FormatStrg = Get0FormatString( sFormatStrg, b0FormatFound );

    OUString sReturnStrg;
    OUString sTempStrg;

    if( dNumber==0.0 )
    {
        sTempStrg = sFormatStrg;
        if( b0FormatFound )
        {
            if( s0FormatStrg.isEmpty() && bPosFormatFound )
            {
                sTempStrg = sPosFormatStrg;
            }
            else
            {
                sTempStrg = s0FormatStrg;
            }
        }
        else if( bPosFormatFound )
        {
            sTempStrg = sPosFormatStrg;
        }
        ScanFormatString( dNumber, sTempStrg, sReturnStrg,/*bCreateSign=*/false );
    }
    else
    {
        if( dNumber<0.0 )
        {
            if( bNegFormatFound )
            {
                if( sNegFormatStrg.isEmpty() && bPosFormatFound )
                {
                    sTempStrg = "-" + sPosFormatStrg;
                }
                else
                {
                    sTempStrg = sNegFormatStrg;
                }
            }
            else
            {
                sTempStrg = sFormatStrg;
            }
            // if NO format-string especially for negative
            // values is given, output the leading sign
            ScanFormatString( dNumber, sTempStrg, sReturnStrg,/*bCreateSign=*/bNegFormatFound/*sNegFormatStrg!=EMPTYFORMATSTRING*/ );
        }
        else // if( dNumber>0.0 )
        {
            ScanFormatString( dNumber,
                    (/*sPosFormatStrg!=EMPTYFORMATSTRING*/bPosFormatFound ? sPosFormatStrg : sFormatStrg),
                    sReturnStrg,/*bCreateSign=*/false );
        }
    }
    return sReturnStrg;
}

bool SbxBasicFormater::isBasicFormat( const OUString& sFormatStrg )
{
    if( sFormatStrg.equalsIgnoreAsciiCase( BASICFORMAT_GENERALNUMBER ) )
    {
        return true;
    }
    if( sFormatStrg.equalsIgnoreAsciiCase( BASICFORMAT_CURRENCY ) )
    {
        return true;
    }
    if( sFormatStrg.equalsIgnoreAsciiCase( BASICFORMAT_FIXED ) )
    {
        return true;
    }
    if( sFormatStrg.equalsIgnoreAsciiCase( BASICFORMAT_STANDARD ) )
    {
        return true;
    }
    if( sFormatStrg.equalsIgnoreAsciiCase( BASICFORMAT_PERCENT ) )
    {
        return true;
    }
    if( sFormatStrg.equalsIgnoreAsciiCase( BASICFORMAT_SCIENTIFIC ) )
    {
        return true;
    }
    if( sFormatStrg.equalsIgnoreAsciiCase( BASICFORMAT_YESNO ) )
    {
        return true;
    }
    if( sFormatStrg.equalsIgnoreAsciiCase( BASICFORMAT_TRUEFALSE ) )
    {
        return true;
    }
    if( sFormatStrg.equalsIgnoreAsciiCase( BASICFORMAT_ONOFF ) )
    {
        return true;
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
