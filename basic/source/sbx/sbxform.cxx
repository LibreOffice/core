/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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





#define _NO_DIGIT                   -1

#define MAX_NO_OF_DIGITS            DBL_DIG
#define MAX_DOUBLE_BUFFER_LENGTH    MAX_NO_OF_DIGITS + 9
                    
                    
                    
                    
                    
                    


#define ASCII_0                     '0' 
#define ASCII_9                     '9' 

#define CREATE_1000SEP_CHAR         '@'

#define FORMAT_SEPARATOR            ';'


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




#define GENERALNUMBER_FORMAT        "0.############"
#define FIXED_FORMAT                "0.00"
#define STANDARD_FORMAT             "@0.00"
#define PERCENT_FORMAT              "0.00%"
#define SCIENTIFIC_FORMAT           "#.00E+00"






double get_number_of_digits( double dNumber )

{
    if( dNumber==0.0 )
        return 0.0; 
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


void SbxBasicFormater::StrRoundDigit( OUStringBuffer& sStrg, short nPos, sal_Bool& bOverflow )
{
    if( nPos<0 )
    {
        return;
    }
    bOverflow = sal_False;
    sal_Unicode c = sStrg[nPos];
    if( nPos > 0 && (c == cDecPoint || c == cThousandSep) )
    {
        StrRoundDigit( sStrg, nPos - 1, bOverflow );
        
        return;
    }
    
    
    
    
    
    
    while( nPos >= 0 && ( sStrg[nPos] < ASCII_0 || sStrg[nPos] > ASCII_9 ))
    {
        nPos--;
    }
    if( nPos==-1 )
    {
        ShiftString( sStrg, 0 );
        sStrg[0] = (sal_Unicode)'1';
        bOverflow = sal_True;
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
            bOverflow = sal_True;
        }
    }
}

void SbxBasicFormater::StrRoundDigit( OUStringBuffer& sStrg, short nPos )
{
    sal_Bool bOverflow;

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

#ifdef _with_sprintf


void SbxBasicFormater::InitScan( double _dNum )
{
    char sBuffer[ MAX_DOUBLE_BUFFER_LENGTH ];

    dNum = _dNum;
    InitExp( get_number_of_digits( dNum ) );
    
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


short SbxBasicFormater::GetDigitAtPosScan( short nPos, sal_Bool& bFoundFirstDigit )
{
    
    
    
    
    if( nPos>nNumExp || abs(nNumExp-nPos)>MAX_NO_OF_DIGITS )
    {
        return _NO_DIGIT;
    }
    
    
    sal_uInt16 no = 1;
    
    if( nPos<nNumExp )
        no++;
    no += nNumExp-nPos;
    
    if( nPos==nNumExp )
        bFoundFirstDigit = sal_True;
    return (short)(sSciNumStrg[ no ] - ASCII_0);
}

short SbxBasicFormater::GetDigitAtPosExpScan( short nPos, sal_Bool& bFoundFirstDigit )
{
    if( nPos>nExpExp )
        return -1;

    sal_uInt16 no = 1;
    no += nExpExp-nPos;

    if( nPos==nExpExp )
        bFoundFirstDigit = sal_True;
    return (short)(sNumExpStrg[ no ] - ASCII_0);
}



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













short SbxBasicFormater::GetDigitAtPos( double dNumber, short nPos,
                                double& dNextNumber, sal_Bool& bFoundFirstDigit )
{
    double dDigit;
    short  nMaxDigit;

    dNumber = fabs( dNumber );

    nMaxDigit = (short)get_number_of_digits( dNumber );
    
    
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
    
    dNextNumber = dNumber;

    return RoundDigit( dDigit );
}


short SbxBasicFormater::RoundDigit( double dNumber )
{
    if( dNumber<0.0 || dNumber>10.0 )
        return -1;
    short nTempHigh = (short)(dNumber+0.5); 
    return nTempHigh;
}

#endif



OUString SbxBasicFormater::GetPosFormatString( const OUString& sFormatStrg, sal_Bool & bFound )
{
    bFound = sal_False;     
    sal_Int32 nPos = sFormatStrg.indexOf( FORMAT_SEPARATOR );

    if( nPos >= 0 )
    {
        bFound = sal_True;
        
        
        return sFormatStrg.copy( 0,nPos );
    }

    OUString aRetStr;
    aRetStr = OUString::createFromAscii( EMPTYFORMATSTRING );
    return aRetStr;
}


OUString SbxBasicFormater::GetNegFormatString( const OUString& sFormatStrg, sal_Bool & bFound )
{
    bFound = sal_False;     
    sal_Int32 nPos = sFormatStrg.indexOf( FORMAT_SEPARATOR );

    if( nPos >= 0)
    {
        
        
        OUString sTempStrg = sFormatStrg.copy( nPos+1 );
        nPos = sTempStrg.indexOf( FORMAT_SEPARATOR );
        bFound = sal_True;
        if( nPos < 0 )
        {
            return sTempStrg;
        }
        else
        {
            return sTempStrg.copy( 0,nPos );
        }
    }
    OUString aRetStr;
    aRetStr = OUString::createFromAscii( EMPTYFORMATSTRING );
    return aRetStr;
}


OUString SbxBasicFormater::Get0FormatString( const OUString& sFormatStrg, sal_Bool & bFound )
{
    bFound = sal_False;     
    sal_Int32 nPos = sFormatStrg.indexOf( FORMAT_SEPARATOR );

    if( nPos >= 0 )
    {
        
        
        OUString sTempStrg = sFormatStrg.copy( nPos+1 );
        nPos = sTempStrg.indexOf( FORMAT_SEPARATOR );
        if( nPos >= 0 )
        {
            bFound = sal_True;
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

    OUString aRetStr;
    aRetStr = OUString::createFromAscii( EMPTYFORMATSTRING );
    return aRetStr;
}


OUString SbxBasicFormater::GetNullFormatString( const OUString& sFormatStrg, sal_Bool & bFound )
{
    bFound = sal_False;     
    sal_Int32 nPos = sFormatStrg.indexOf( FORMAT_SEPARATOR );

    if( nPos >= 0 )
    {
        
        
        OUString sTempStrg = sFormatStrg.copy( nPos+1 );
        nPos = sTempStrg.indexOf( FORMAT_SEPARATOR );
        if( nPos >= 0 )
        {
            sTempStrg = sTempStrg.copy( nPos+1 );
            nPos = sTempStrg.indexOf( FORMAT_SEPARATOR );
            if( nPos >= 0 )
            {
                bFound = sal_True;
                return sTempStrg.copy( nPos+1 );
            }
        }
    }

    OUString aRetStr;
    aRetStr = OUString::createFromAscii( EMPTYFORMATSTRING );
    return aRetStr;
}


short SbxBasicFormater::AnalyseFormatString( const OUString& sFormatStrg,
                short& nNoOfDigitsLeft, short& nNoOfDigitsRight,
                short& nNoOfOptionalDigitsLeft,
                short& nNoOfExponentDigits, short& nNoOfOptionalExponentDigits,
                sal_Bool& bPercent, sal_Bool& bCurrency, sal_Bool& bScientific,
                sal_Bool& bGenerateThousandSeparator,
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
    bPercent = sal_False;
    bCurrency = sal_False;
    bScientific = sal_False;
    
    
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

                
                
                
                if( c=='#' )
                {
                    nNoOfOptionalDigitsLeft++;
                }
            }
            else if( nState==1 )
            {
                nNoOfDigitsRight++;
            }
            else if( nState==-1 )   
            {
                if( c=='#' )    
                {
                    nNoOfOptionalExponentDigits++;
                    nState = -2;
                }
                nNoOfExponentDigits++;
            }
            else if( nState==-2 )   
            {
                if( c=='0' )
                {
                    
                    return -4;
                }
                nNoOfOptionalExponentDigits++;
                nNoOfExponentDigits++;
            }
            break;
        case '.':
            nState++;
            if( nState>1 )
            {
                return -1;  
            }
            break;
        case '%':
            bPercent = sal_True;
            break;
        case '(':
            bCurrency = sal_True;
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
            
            if( nNoOfDigitsLeft > 0 || nNoOfDigitsRight > 0 )
            {
                nState = -1;   
                bScientific = sal_True;
            }
            break;
            
            
        case '\\':
            
            i++;
            break;
        case CREATE_1000SEP_CHAR:
            bGenerateThousandSeparator = sal_True;
            break;
        }
    }
    return 0;
}



void SbxBasicFormater::ScanFormatString( double dNumber,
                                         const OUString& sFormatStrg, OUString& sReturnStrgFinal,
                                         sal_Bool bCreateSign )
{
    short   /*nErr,*/nNoOfDigitsLeft,nNoOfDigitsRight,nNoOfOptionalDigitsLeft,
        nNoOfExponentDigits,nNoOfOptionalExponentDigits,
        nMultipleThousandSeparators;
    sal_Bool    bPercent,bCurrency,bScientific,bGenerateThousandSeparator;

    OUStringBuffer sReturnStrg = OUStringBuffer();

    
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
    AnalyseFormatString( sFormatStrg, nNoOfDigitsLeft, nNoOfDigitsRight,
                         nNoOfOptionalDigitsLeft, nNoOfExponentDigits,
                         nNoOfOptionalExponentDigits,
                         bPercent, bCurrency, bScientific,
                         bGenerateThousandSeparator, nMultipleThousandSeparators );
    
    if( bPercent )
    {
        dNumber *= 100.0;
    }

        
    if( nMultipleThousandSeparators )
    {
        dNumber /= 1000.0;
    }
    double dExponent;
    short i,nLen;
    short nState,nDigitPos,nExponentPos,nMaxDigit,nMaxExponentDigit;
    sal_Bool bFirstDigit,bFirstExponentDigit,bFoundFirstDigit,
        bIsNegative,bZeroSpaceOn, bSignHappend,bDigitPosNegative;

    bSignHappend = sal_False;
    bFoundFirstDigit = sal_False;
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
        nDigitPos = nNoOfDigitsLeft - 1; 
        
        bDigitPosNegative = (nDigitPos < 0);
    }
    bFirstDigit = sal_True;
    bFirstExponentDigit = sal_True;
    nState = 0; 
    bZeroSpaceOn = 0;


#ifdef _with_sprintf
    InitScan( dNumber );
#endif
    
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
                
                if( bFirstDigit )
                {
                    
                    
                    if( bIsNegative && !bCreateSign && !bSignHappend )
                    {
                        bSignHappend = sal_True;
                        sReturnStrg.append('-');
                    }
                    
                    
                    if( nMaxDigit > nDigitPos )
                    {
                        for( short j = nMaxDigit; j > nDigitPos; j-- )
                        {
                            short nTempDigit;
#ifdef _with_sprintf
                            AppendDigit( sReturnStrg, nTempDigit = GetDigitAtPosScan( j, bFoundFirstDigit ) );
#else
                            AppendDigit( sReturnStrg, nTempDigit = GetDigitAtPos( dNumber, j, dNumber, bFoundFirstDigit ) );
#endif
                            if( nTempDigit!=_NO_DIGIT )
                            {
                                bFirstDigit = sal_False;
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
                    bFirstDigit = sal_False;
                    bZeroSpaceOn = 1;
                    
                    
                    
                    if( bGenerateThousandSeparator && ( c=='0' || nMaxDigit >= nDigitPos ) && nDigitPos > 0 && (nDigitPos % 3 == 0) )
                    {
                        sReturnStrg.append(cThousandSep);
                    }
                }
                else
                {
                    short nTempDigit;
#ifdef _with_sprintf
                    AppendDigit( sReturnStrg, nTempDigit = GetDigitAtPosScan( nDigitPos, bFoundFirstDigit ) );
#else
                    AppendDigit( sReturnStrg, nTempDigit = GetDigitAtPos( dNumber, nDigitPos, dNumber, bFoundFirstDigit ) );
#endif

                    if( nTempDigit != _NO_DIGIT )
                    {
                        bFirstDigit = sal_False;
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
                
                if( bFirstExponentDigit )
                {
                    
                    bFirstExponentDigit = sal_False;
                    if( nMaxExponentDigit > nExponentPos )
                        
                        
                    {
                        for( short j = nMaxExponentDigit; j > nExponentPos; j-- )
                        {
#ifdef _with_sprintf
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
#ifdef _with_sprintf
                    AppendDigit( sReturnStrg, GetDigitAtPosExpScan( dExponent, nExponentPos, bFoundFirstDigit ) );
#else
                    AppendDigit( sReturnStrg, GetDigitAtPos( dExponent, nExponentPos, dExponent, bFoundFirstDigit ) );
#endif
                }
                nExponentPos--;
            }
            break;
        case '.':
            if( bDigitPosNegative ) 
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
            
            ParseBack( sReturnStrg, sFormatStrg, i-1 );
            sReturnStrg.append('%');
            break;
        case 'e':
        case 'E':
            
            {
                
                if( bFirstDigit )
                {
                    
                    
                    

                    
                    sReturnStrg.append(c);
                    break;
                }

                sal_Bool bOverflow = sal_False;
#ifdef _with_sprintf
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
                    
                    LeftShiftDecimalPoint( sReturnStrg );
                    sReturnStrg[sReturnStrg.getLength() - 1] = 0;
                    dExponent += 1.0;
                }
                
                ParseBack( sReturnStrg, sFormatStrg, i-1 );
            }
            
            nState++;
            
            sReturnStrg.append(c);
            
            c = sFormatStrg[ ++i ];
            
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
            
            ParseBack( sReturnStrg, sFormatStrg, i-1 );
            if( bIsNegative )
            {
                sReturnStrg.append(c);
            }
            break;
        case '$':
            
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
            
            
            c = sFormatStrg[ ++i ];
            if( c!=0 )
            {
                sReturnStrg.append(c);
            }
            break;
        case CREATE_1000SEP_CHAR:
            
            
            break;
        default:
            
            if( ( c>='a' && c<='z' ) ||
                ( c>='A' && c<='Z' ) ||
                ( c>='1' && c<='9' ) )
            {
                sReturnStrg.append(c);
            }
        }
    }

    
    if( !bScientific )
    {
#ifdef _with_sprintf
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

OUString SbxBasicFormater::BasicFormatNull( OUString sFormatStrg )
{
    sal_Bool bNullFormatFound;
    OUString sNullFormatStrg = GetNullFormatString( sFormatStrg, bNullFormatFound );

    if( bNullFormatFound )
    {
        return sNullFormatStrg;
    }
    return OUString("null");
}

OUString SbxBasicFormater::BasicFormat( double dNumber, OUString sFormatStrg )
{
    sal_Bool bPosFormatFound,bNegFormatFound,b0FormatFound;

    
    if( sFormatStrg.equalsIgnoreAsciiCase( BASICFORMAT_GENERALNUMBER ) )
    {
        sFormatStrg = OUString::createFromAscii( GENERALNUMBER_FORMAT );
    }
    if( sFormatStrg.equalsIgnoreAsciiCase( BASICFORMAT_CURRENCY ) )
    {
        sFormatStrg = sCurrencyFormatStrg;
    }
    if( sFormatStrg.equalsIgnoreAsciiCase( BASICFORMAT_FIXED ) )
    {
        sFormatStrg = OUString::createFromAscii( FIXED_FORMAT );
    }
    if( sFormatStrg.equalsIgnoreAsciiCase( BASICFORMAT_STANDARD ) )
    {
        sFormatStrg = OUString::createFromAscii( STANDARD_FORMAT );
    }
    if( sFormatStrg.equalsIgnoreAsciiCase( BASICFORMAT_PERCENT ) )
    {
        sFormatStrg = OUString::createFromAscii( PERCENT_FORMAT );
    }
    if( sFormatStrg.equalsIgnoreAsciiCase( BASICFORMAT_SCIENTIFIC ) )
    {
        sFormatStrg = OUString::createFromAscii( SCIENTIFIC_FORMAT );
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
        ScanFormatString( dNumber, sTempStrg, sReturnStrg,/*bCreateSign=*/sal_False );
    }
    else
    {
        if( dNumber<0.0 )
        {
            if( bNegFormatFound )
            {
                if( sNegFormatStrg.isEmpty() && bPosFormatFound )
                {
                    sTempStrg = "-";
                    sTempStrg += sPosFormatStrg;
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
            
            
            ScanFormatString( dNumber, sTempStrg, sReturnStrg,/*bCreateSign=*/bNegFormatFound/*sNegFormatStrg!=EMPTYFORMATSTRING*/ );
        }
        else 
        {
            ScanFormatString( dNumber,
                    (/*sPosFormatStrg!=EMPTYFORMATSTRING*/bPosFormatFound ? sPosFormatStrg : sFormatStrg),
                    sReturnStrg,/*bCreateSign=*/sal_False );
        }
    }
    return sReturnStrg;
}

sal_Bool SbxBasicFormater::isBasicFormat( OUString sFormatStrg )
{
    if( sFormatStrg.equalsIgnoreAsciiCase( BASICFORMAT_GENERALNUMBER ) )
    {
        return sal_True;
    }
    if( sFormatStrg.equalsIgnoreAsciiCase( BASICFORMAT_CURRENCY ) )
    {
        return sal_True;
    }
    if( sFormatStrg.equalsIgnoreAsciiCase( BASICFORMAT_FIXED ) )
    {
        return sal_True;
    }
    if( sFormatStrg.equalsIgnoreAsciiCase( BASICFORMAT_STANDARD ) )
    {
        return sal_True;
    }
    if( sFormatStrg.equalsIgnoreAsciiCase( BASICFORMAT_PERCENT ) )
    {
        return sal_True;
    }
    if( sFormatStrg.equalsIgnoreAsciiCase( BASICFORMAT_SCIENTIFIC ) )
    {
        return sal_True;
    }
    if( sFormatStrg.equalsIgnoreAsciiCase( BASICFORMAT_YESNO ) )
    {
        return sal_True;
    }
    if( sFormatStrg.equalsIgnoreAsciiCase( BASICFORMAT_TRUEFALSE ) )
    {
        return sal_True;
    }
    if( sFormatStrg.equalsIgnoreAsciiCase( BASICFORMAT_ONOFF ) )
    {
        return sal_True;
    }
    return sal_False;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
