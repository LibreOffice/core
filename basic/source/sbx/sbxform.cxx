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

#include <sbxform.hxx>
#include <rtl/ustrbuf.hxx>

#include <rtl/character.hxx>
#include <o3tl/sprintf.hxx>
#include <o3tl/string_view.hxx>
#include <string_view>
#include <utility>

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

#define CREATE_1000SEP_CHAR         '@'

#define FORMAT_SEPARATOR            ';'

// predefined formats for the Format$()-command:
constexpr std::u16string_view BASICFORMAT_GENERALNUMBER = u"General Number";
constexpr std::u16string_view BASICFORMAT_CURRENCY = u"Currency";
constexpr std::u16string_view BASICFORMAT_FIXED = u"Fixed";
constexpr std::u16string_view BASICFORMAT_STANDARD = u"Standard";
constexpr std::u16string_view BASICFORMAT_PERCENT = u"Percent";
constexpr std::u16string_view BASICFORMAT_SCIENTIFIC = u"Scientific";
constexpr std::u16string_view BASICFORMAT_YESNO = u"Yes/No";
constexpr std::u16string_view BASICFORMAT_TRUEFALSE = u"True/False";
constexpr std::u16string_view BASICFORMAT_ONOFF = u"On/Off";

// Comment: Visual-Basic has a maximum of 12 positions after the
//          decimal point for floating-point-numbers.
// all format-strings are compatible to Visual-Basic:
constexpr OUString GENERALNUMBER_FORMAT = u"0.############"_ustr;
constexpr OUString FIXED_FORMAT = u"0.00"_ustr;
constexpr OUString STANDARD_FORMAT = u"@0.00"_ustr;
constexpr OUString PERCENT_FORMAT = u"0.00%"_ustr;
constexpr OUString SCIENTIFIC_FORMAT = u"#.00E+00"_ustr;
// Comment: the character @ means that thousand-separators shall
//          be generated. That's a StarBasic 'extension'.


static double get_number_of_digits( double dNumber )
//double floor_log10_fabs( double dNumber )
{
    if( dNumber==0.0 )
        return 0.0; // used to be 1.0, now 0.0 because of #40025;
    else
        return floor( log10( fabs( dNumber ) ) );
}


SbxBasicFormater::SbxBasicFormater( sal_Unicode _cDecPoint, sal_Unicode _cThousandSep,
                      OUString _sOnStrg,
                      OUString _sOffStrg,
                      OUString _sYesStrg,
                      OUString _sNoStrg,
                      OUString _sTrueStrg,
                      OUString _sFalseStrg,
                      OUString _sCurrencyStrg,
                      OUString _sCurrencyFormatStrg )
    : cDecPoint(_cDecPoint)
    , cThousandSep(_cThousandSep)
    , sOnStrg(std::move(_sOnStrg))
    , sOffStrg(std::move(_sOffStrg))
    , sYesStrg(std::move(_sYesStrg))
    , sNoStrg(std::move(_sNoStrg))
    , sTrueStrg(std::move(_sTrueStrg))
    , sFalseStrg(std::move(_sFalseStrg))
    , sCurrencyStrg(std::move(_sCurrencyStrg))
    , sCurrencyFormatStrg(std::move(_sCurrencyFormatStrg))
    , dNum(0.0)
    , nNumExp(0)
    , nExpExp(0)
{
}

// function to output an error-text (for debugging)
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
        sStrg.append(static_cast<sal_Unicode>(nDigit+'0'));
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
    while( nPos >= 0 && ! rtl::isAsciiDigit(sStrg[nPos]))
    {
        nPos--;
    }
    if( nPos==-1 )
    {
        ShiftString( sStrg, 0 );
        sStrg[0] = '1';
        bOverflow = true;
    }
    else
    {
        sal_Unicode c2 = sStrg[nPos];
        if( rtl::isAsciiDigit(c2) )
        {
            if( c2 == '9' )
            {
                sStrg[nPos] = '0';
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
            sStrg[nPos + 1] = '1';
            bOverflow = true;
        }
    }
}

void SbxBasicFormater::StrRoundDigit( OUStringBuffer& sStrg, short nPos )
{
    bool bOverflow;

    StrRoundDigit( sStrg, nPos, bOverflow );
}

void SbxBasicFormater::ParseBack( OUStringBuffer& sStrg, std::u16string_view sFormatStrg,
                                  short nFormatPos )
{
    for( sal_Int32 i = nFormatPos;
         i>0 && sFormatStrg[ i ]  == '#' && sStrg[sStrg.getLength() - 1] == '0';
         i-- )
    {
        sStrg.setLength(sStrg.getLength() - 1 );
    }
}

void SbxBasicFormater::InitScan( double _dNum )
{
    char sBuffer[ MAX_DOUBLE_BUFFER_LENGTH ];

    dNum = _dNum;
    InitExp( get_number_of_digits( dNum ) );
    // maximum of 15 positions behind the decimal point, example: -1.234000000000000E-001
    /*int nCount =*/ o3tl::sprintf( sBuffer,"%+22.15lE",dNum );
    sSciNumStrg = OUString::createFromAscii( sBuffer );
}


void SbxBasicFormater::InitExp( double _dNewExp )
{
    nNumExp = static_cast<short>(_dNewExp);
    sNumExpStrg = (nNumExp >= 0 ? std::u16string_view(u"+") : std::u16string_view(u""))
        + OUString::number(nNumExp);
    nExpExp = static_cast<short>(get_number_of_digits( static_cast<double>(nNumExp) ));
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
    return static_cast<short>(sSciNumStrg[ no ] - '0');
}

short SbxBasicFormater::GetDigitAtPosExpScan( short nPos, bool& bFoundFirstDigit )
{
    if( nPos>nExpExp )
        return -1;

    sal_uInt16 no = 1;
    no += nExpExp-nPos;

    if( nPos==nExpExp )
        bFoundFirstDigit = true;
    return static_cast<short>(sNumExpStrg[ no ] - '0');
}

// a value for the exponent can be given because the number maybe shall
// not be displayed in a normed way (e. g. 1.2345e-03) but maybe 123.345e-3 !
short SbxBasicFormater::GetDigitAtPosExpScan( double dNewExponent, short nPos,
                                              bool& bFoundFirstDigit )
{
    InitExp( dNewExponent );

    return GetDigitAtPosExpScan( nPos,bFoundFirstDigit );
}

// Copies the respective part of the format-string, if existing, and returns it.
// So a new string is created, which has to be freed by the caller later.
OUString SbxBasicFormater::GetPosFormatString( std::u16string_view sFormatStrg, bool & bFound )
{
    bFound = false;     // default...
    size_t nPos = sFormatStrg.find( FORMAT_SEPARATOR );

    if( nPos != std::u16string_view::npos )
    {
        bFound = true;
        // the format-string for positive numbers is
        // everything before the first ';'
        return OUString(sFormatStrg.substr( 0,nPos ));
    }

    return OUString();
}

// see also GetPosFormatString()
OUString SbxBasicFormater::GetNegFormatString( std::u16string_view sFormatStrg, bool & bFound )
{
    bFound = false;     // default...
    size_t nPos = sFormatStrg.find( FORMAT_SEPARATOR );

    if( nPos != std::u16string_view::npos)
    {
        // the format-string for negative numbers is
        // everything between the first and the second ';'
        std::u16string_view sTempStrg = sFormatStrg.substr( nPos+1 );
        nPos = sTempStrg.find( FORMAT_SEPARATOR );
        bFound = true;
        if( nPos == std::u16string_view::npos )
        {
            return OUString(sTempStrg);
        }
        else
        {
            return OUString(sTempStrg.substr( 0,nPos ));
        }
    }
    return OUString();
}

// see also GetPosFormatString()
OUString SbxBasicFormater::Get0FormatString( std::u16string_view sFormatStrg, bool & bFound )
{
    bFound = false;     // default...
    size_t nPos = sFormatStrg.find( FORMAT_SEPARATOR );

    if( nPos != std::u16string_view::npos )
    {
        // the format string for the zero is
        // everything after the second ';'
        std::u16string_view sTempStrg = sFormatStrg.substr( nPos+1 );
        nPos = sTempStrg.find( FORMAT_SEPARATOR );
        if( nPos != std::u16string_view::npos )
        {
            bFound = true;
            sTempStrg = sTempStrg.substr( nPos+1 );
            nPos = sTempStrg.find( FORMAT_SEPARATOR );
            if( nPos == std::u16string_view::npos )
            {
                return OUString(sTempStrg);
            }
            else
            {
                return OUString(sTempStrg.substr( 0,nPos ));
            }
        }
    }

    return OUString();
}

// see also GetPosFormatString()
OUString SbxBasicFormater::GetNullFormatString( std::u16string_view sFormatStrg, bool & bFound )
{
    bFound = false;     // default...
    size_t nPos = sFormatStrg.find( FORMAT_SEPARATOR );

    if( nPos != std::u16string_view::npos )
    {
        // the format-string for the Null is
        // everything after the third ';'
        std::u16string_view sTempStrg = sFormatStrg.substr( nPos+1 );
        nPos = sTempStrg.find( FORMAT_SEPARATOR );
        if( nPos != std::u16string_view::npos )
        {
            sTempStrg = sTempStrg.substr( nPos+1 );
            nPos = sTempStrg.find( FORMAT_SEPARATOR );
            if( nPos != std::u16string_view::npos )
            {
                bFound = true;
                return OUString(sTempStrg.substr( nPos+1 ));
            }
        }
    }

    return OUString();
}

// returns value <> 0 in case of an error
void SbxBasicFormater::AnalyseFormatString( std::u16string_view sFormatStrg,
                short& nNoOfDigitsLeft, short& nNoOfDigitsRight,
                short& nNoOfOptionalDigitsLeft,
                short& nNoOfExponentDigits, short& nNoOfOptionalExponentDigits,
                bool& bPercent, bool& bCurrency, bool& bScientific,
                bool& bGenerateThousandSeparator,
                short& nMultipleThousandSeparators )
{
    sal_Int32 nLen;
    short nState = 0;

    nLen = sFormatStrg.size();
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
    bGenerateThousandSeparator = sFormatStrg.find( ',' ) != std::u16string_view::npos;
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
                                         std::u16string_view sFormatStrg, OUString& sReturnStrgFinal,
                                         bool bCreateSign )
{
    short   /*nErr,*/nNoOfDigitsLeft,nNoOfDigitsRight,nNoOfOptionalDigitsLeft,
        nNoOfExponentDigits,nNoOfOptionalExponentDigits,
        nMultipleThousandSeparators;
    bool    bPercent,bCurrency,bScientific,bGenerateThousandSeparator;

    OUStringBuffer sReturnStrg(32);

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
    nLen = sFormatStrg.size();
    dExponent = get_number_of_digits( dNumber );
    nExponentPos = 0;
    nMaxExponentDigit = 0;
    nMaxDigit = static_cast<short>(dExponent);
    bDigitPosNegative = false;
    if( bScientific )
    {
        dExponent = dExponent - static_cast<double>(nNoOfDigitsLeft-1);
        nDigitPos = nMaxDigit;
        nMaxExponentDigit = static_cast<short>(get_number_of_digits( dExponent ));
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


    InitScan( dNumber );
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
                            short nTempDigit = GetDigitAtPosScan( j, bFoundFirstDigit );
                            AppendDigit( sReturnStrg, nTempDigit );
                            if( nTempDigit != NO_DIGIT_ )
                            {
                                bFirstDigit = false;
                            }
                            if( bGenerateThousandSeparator && c=='0' && j > 0 && (j % 3 == 0) )
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
                    if (bGenerateThousandSeparator && c == '0' && nDigitPos > 0 && (nDigitPos % 3 == 0))
                    {
                        sReturnStrg.append(cThousandSep);
                    }
                }
                else
                {
                    short nTempDigit = GetDigitAtPosScan( nDigitPos, bFoundFirstDigit ) ;
                    AppendDigit( sReturnStrg, nTempDigit );

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
                            AppendDigit( sReturnStrg, GetDigitAtPosExpScan( dExponent, j, bFoundFirstDigit ) );
                        }
                    }
                }

                if( nMaxExponentDigit < nExponentPos && c=='0' )
                {
                    AppendDigit( sReturnStrg, 0 );
                }
                else
                {
                    AppendDigit( sReturnStrg, GetDigitAtPosExpScan( dExponent, nExponentPos, bFoundFirstDigit ) );
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
                short nNextDigit = GetDigitAtPosScan( nDigitPos, bFoundFirstDigit );
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
        short nNextDigit = GetDigitAtPosScan( nDigitPos, bFoundFirstDigit );
        if( nNextDigit>=5 )
        {
            StrRoundDigit( sReturnStrg, sReturnStrg.getLength() - 1 );
        }
    }

    if( nNoOfDigitsRight>0 )
    {
        ParseBack( sReturnStrg, sFormatStrg, sFormatStrg.size()-1 );
    }
    sReturnStrgFinal = sReturnStrg.makeStringAndClear();
}

OUString SbxBasicFormater::BasicFormatNull( std::u16string_view sFormatStrg )
{
    bool bNullFormatFound;
    OUString sNullFormatStrg = GetNullFormatString( sFormatStrg, bNullFormatFound );

    if( bNullFormatFound )
    {
        return sNullFormatStrg;
    }
    return u"null"_ustr;
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

bool SbxBasicFormater::isBasicFormat( std::u16string_view sFormatStrg )
{
    if( o3tl::equalsIgnoreAsciiCase( sFormatStrg, BASICFORMAT_GENERALNUMBER ) )
    {
        return true;
    }
    if( o3tl::equalsIgnoreAsciiCase( sFormatStrg, BASICFORMAT_CURRENCY ) )
    {
        return true;
    }
    if( o3tl::equalsIgnoreAsciiCase( sFormatStrg, BASICFORMAT_FIXED ) )
    {
        return true;
    }
    if( o3tl::equalsIgnoreAsciiCase( sFormatStrg, BASICFORMAT_STANDARD ) )
    {
        return true;
    }
    if( o3tl::equalsIgnoreAsciiCase( sFormatStrg, BASICFORMAT_PERCENT ) )
    {
        return true;
    }
    if( o3tl::equalsIgnoreAsciiCase( sFormatStrg, BASICFORMAT_SCIENTIFIC ) )
    {
        return true;
    }
    if( o3tl::equalsIgnoreAsciiCase( sFormatStrg, BASICFORMAT_YESNO ) )
    {
        return true;
    }
    if( o3tl::equalsIgnoreAsciiCase( sFormatStrg, BASICFORMAT_TRUEFALSE ) )
    {
        return true;
    }
    if( o3tl::equalsIgnoreAsciiCase( sFormatStrg, BASICFORMAT_ONOFF ) )
    {
        return true;
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
