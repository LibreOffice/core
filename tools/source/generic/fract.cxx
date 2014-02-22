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

#include <limits.h>
#include <rtl/ustring.hxx>
#include <tools/debug.hxx>
#include <tools/fract.hxx>
#include <tools/lineend.hxx>
#include <tools/stream.hxx>
#include <tools/bigint.hxx>

/** Compute greates common divisor using Euclidian algorithm

    As the algorithm works on positive values only, the absolute value
    of each parameter is used.

    @param nVal1
    @param nVal2

    @note: If one parameter is {0,1}, GetGGT returns 1.
*/
static long GetGGT( long nVal1, long nVal2 )
{
    nVal1 = std::abs( nVal1 );
    nVal2 = std::abs( nVal2 );

    if ( nVal1 <= 1 || nVal2 <= 1 )
        return 1;

    while ( nVal1 != nVal2 )
    {
        if ( nVal1 > nVal2 )
        {
            nVal1 %= nVal2;
            if ( nVal1 == 0 )
                return nVal2;
        }
        else
        {
            nVal2 %= nVal1;
            if ( nVal2 == 0 )
                return nVal1;
        }
    }
    return nVal1;
}

static void Reduce( BigInt &rVal1, BigInt &rVal2 )
{
    BigInt nA( rVal1 );
    BigInt nB( rVal2 );
    nA.Abs();
    nB.Abs();

    if ( nA.IsOne() || nB.IsOne() || nA.IsZero() || nB.IsZero() )
        return;

    while ( nA != nB )
    {
        if ( nA > nB )
        {
            nA %= nB;
            if ( nA.IsZero() )
            {
                rVal1 /= nB;
                rVal2 /= nB;
                return;
            }
        }
        else
        {
            nB %= nA;
            if ( nB.IsZero() )
            {
                rVal1 /= nA;
                rVal2 /= nA;
                return;
            }
        }
    }

    rVal1 /= nA;
    rVal2 /= nB;
}





Fraction::Fraction( long nNum, long nDen )
{
    nNumerator = nNum;
    nDenominator = nDen;
    if ( nDenominator < 0 )
    {
        nDenominator = -nDenominator;
        nNumerator   = -nNumerator;
    }

    
    long n = GetGGT( nNumerator, nDenominator );
    nNumerator   /= n;
    nDenominator /= n;
}




Fraction::Fraction( double dVal )
{
    long nDen = 1;
    long nMAX = LONG_MAX / 10;

    if ( dVal > LONG_MAX || dVal < LONG_MIN )
    {
        nNumerator   = 0;
        nDenominator = -1;
        return;
    }

    while ( std::abs( (long)dVal ) < nMAX && nDen < nMAX )
    {
        dVal *= 10;
        nDen *= 10;
    }
    nNumerator   = (long)dVal;
    nDenominator = nDen;

    
    long n = GetGGT( nNumerator, nDenominator );
    nNumerator   /= n;
    nDenominator /= n;
}

Fraction::operator double() const
{
    if ( nDenominator > 0 )
        return (double)nNumerator / (double)nDenominator;
    else
        return (double)0;
}







Fraction& Fraction::operator += ( const Fraction& rVal )
{
    if ( !rVal.IsValid() )
    {
        nNumerator   = 0;
        nDenominator = -1;
    }
    if ( !IsValid() )
        return *this;

    
    BigInt nN( nNumerator );
    nN *= BigInt( rVal.nDenominator );
    BigInt nW1Temp( nDenominator );
    nW1Temp *= BigInt( rVal.nNumerator );
    nN += nW1Temp;

    BigInt nD( nDenominator );
    nD *= BigInt( rVal.nDenominator );

    Reduce( nN, nD );

    if ( nN.bIsBig || nD.bIsBig )
    {
        nNumerator   = 0;
        nDenominator = -1;
    }
    else
    {
        nNumerator   = (long)nN,
        nDenominator = (long)nD;
    }

    return *this;
}







Fraction& Fraction::operator -= ( const Fraction& rVal )
{
    if ( !rVal.IsValid() )
    {
        nNumerator   = 0;
        nDenominator = -1;
    }
    if ( !IsValid() )
        return *this;

    
    BigInt nN( nNumerator );
    nN *= BigInt( rVal.nDenominator );
    BigInt nW1Temp( nDenominator );
    nW1Temp *= BigInt( rVal.nNumerator );
    nN -= nW1Temp;

    BigInt nD( nDenominator );
    nD *= BigInt( rVal.nDenominator );

    Reduce( nN, nD );

    if ( nN.bIsBig || nD.bIsBig )
    {
        nNumerator   = 0;
        nDenominator = -1;
    }
    else
    {
        nNumerator   = (long)nN,
        nDenominator = (long)nD;
    }

    return *this;
}







Fraction& Fraction::operator *= ( const Fraction& rVal )
{
    if ( !rVal.IsValid() )
    {
        nNumerator   = 0;
        nDenominator = -1;
    }
    if ( !IsValid() )
        return *this;

    long nGGT1 = GetGGT( nNumerator, rVal.nDenominator );
    long nGGT2 = GetGGT( rVal.nNumerator, nDenominator );
    BigInt nN( nNumerator / nGGT1 );
    nN *= BigInt( rVal.nNumerator / nGGT2 );
    BigInt nD( nDenominator / nGGT2 );
    nD *= BigInt( rVal.nDenominator / nGGT1 );

    if ( nN.bIsBig || nD.bIsBig )
    {
        nNumerator   = 0;
        nDenominator = -1;
    }
    else
    {
        nNumerator   = (long)nN,
        nDenominator = (long)nD;
    }

    return *this;
}







Fraction& Fraction::operator /= ( const Fraction& rVal )
{
    if ( !rVal.IsValid() )
    {
        nNumerator   = 0;
        nDenominator = -1;
    }
    if ( !IsValid() )
        return *this;

    long nGGT1 = GetGGT( nNumerator, rVal.nNumerator );
    long nGGT2 = GetGGT( rVal.nDenominator, nDenominator );
    BigInt nN( nNumerator / nGGT1 );
    nN *= BigInt( rVal.nDenominator / nGGT2 );
    BigInt nD( nDenominator / nGGT2 );
    nD *= BigInt( rVal.nNumerator / nGGT1 );

    if ( nN.bIsBig || nD.bIsBig )
    {
        nNumerator   = 0;
        nDenominator = -1;
    }
    else
    {
        nNumerator   = (long)nN,
        nDenominator = (long)nD;
        if ( nDenominator < 0 )
        {
            nDenominator = -nDenominator;
            nNumerator   = -nNumerator;
        }
    }

    return *this;
}


const char nbits_table[32] =
{
    32,  1, 23,  2, 29, 24, 14,  3,
    30, 27, 25, 18, 20, 15, 10,  4,
    31, 22, 28, 13, 26, 17, 19,  9,
    21, 12, 16,  8, 11,  7,  6,  5
};

static int impl_NumberOfBits( unsigned long nNum )
{
    
    
    
    
    
    const sal_uInt32 nDeBruijn = 0x7DCD629;

    if ( nNum == 0 )
        return 0;

    
    nNum |= ( nNum >>  1 );
    nNum |= ( nNum >>  2 );
    nNum |= ( nNum >>  4 );
    nNum |= ( nNum >>  8 );
    nNum |= ( nNum >> 16 );

    sal_uInt32 nNumber;
    int nBonus = 0;

#if SAL_TYPES_SIZEOFLONG == 4
    nNumber = nNum;
#elif SAL_TYPES_SIZEOFLONG == 8
    nNum |= ( nNum >> 32 );

    if ( nNum & 0x80000000 )
    {
        nNumber = sal_uInt32( nNum >> 32 );
        nBonus = 32;

        if ( nNumber == 0 )
            return 32;
    }
    else
        nNumber = sal_uInt32( nNum & 0xFFFFFFFF );
#else
#error "Unknown size of long!"
#endif

    
    
    
    
    
    
    
    nNumber = nDeBruijn + ( nDeBruijn * nNumber );

    
    return ( nbits_table[nNumber >> 27] ) + nBonus;
}

/** Inaccurate cancellation for a fraction.

    Clip both nominator and denominator to said number of bits. If
    either of those already have equal or less number of bits used,
    this method does nothing.

    @param nSignificantBits denotes, how many significant binary
    digits to maintain, in both nominator and denominator.

    @example ReduceInaccurate(8) has an error <1% [1/2^(8-1)] - the
    largest error occurs with the following pair of values:

    binary    1000000011111111111111111111111b/1000000000000000000000000000000b
    =         1082130431/1073741824
    = approx. 1.007812499

    A ReduceInaccurate(8) yields 1/1.
*/
void Fraction::ReduceInaccurate( unsigned nSignificantBits )
{
    if ( !nNumerator || !nDenominator )
        return;

    
    const bool bNeg = ( nNumerator < 0 );
    unsigned long nMul = (unsigned long)( bNeg? -nNumerator: nNumerator );
    unsigned long nDiv = (unsigned long)( nDenominator );

    DBG_ASSERT(nSignificantBits<65, "More than 64 bit of significance is overkill!");

    
    const int nMulBitsToLose = std::max( ( impl_NumberOfBits( nMul ) - int( nSignificantBits ) ), 0 );
    const int nDivBitsToLose = std::max( ( impl_NumberOfBits( nDiv ) - int( nSignificantBits ) ), 0 );

    const int nToLose = std::min( nMulBitsToLose, nDivBitsToLose );

    
    nMul >>= nToLose;
    nDiv >>= nToLose;

    if ( !nMul || !nDiv )
    {
        
        OSL_FAIL( "Oops, we reduced too much..." );
        return;
    }

    
    long n1 = GetGGT( nMul, nDiv );
    if ( n1 != 1 )
    {
        nMul /= n1;
        nDiv /= n1;
    }

    nNumerator = bNeg? -long( nMul ): long( nMul );
    nDenominator = nDiv;
}

bool operator == ( const Fraction& rVal1, const Fraction& rVal2 )
{
    if ( !rVal1.IsValid() || !rVal2.IsValid() )
        return false;

    return rVal1.nNumerator == rVal2.nNumerator
           && rVal1.nDenominator == rVal2.nDenominator;
}




bool operator < ( const Fraction& rVal1, const Fraction& rVal2 )
{
    if ( !rVal1.IsValid() || !rVal2.IsValid() )
        return false;

    BigInt nN( rVal1.nNumerator );
    nN *= BigInt( rVal2.nDenominator );
    BigInt nD( rVal1.nDenominator );
    nD *= BigInt( rVal2.nNumerator );

    return nN < nD;
}




bool operator > ( const Fraction& rVal1, const Fraction& rVal2 )
{
    if ( !rVal1.IsValid() || !rVal2.IsValid() )
        return false;

    BigInt nN( rVal1.nNumerator );
    nN *= BigInt( rVal2.nDenominator );
    BigInt nD( rVal1.nDenominator);
    nD *= BigInt( rVal2.nNumerator );

    return nN > nD;
}

SvStream& ReadFraction( SvStream& rIStream, Fraction& rFract )
{
    
    sal_Int32 nTmp(0);
    rIStream.ReadInt32( nTmp );
    rFract.nNumerator = nTmp;
    rIStream.ReadInt32( nTmp );
    rFract.nDenominator = nTmp;
    return rIStream;
}

SvStream& WriteFraction( SvStream& rOStream, const Fraction& rFract )
{
    
    rOStream.WriteInt32( sal::static_int_cast<sal_Int32>(rFract.nNumerator) );
    rOStream.WriteInt32( sal::static_int_cast<sal_Int32>(rFract.nDenominator) );
    return rOStream;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
