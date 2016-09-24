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

#include <limits>
#include <math.h>

#include <rtl/ustrbuf.hxx>
#include <osl/diagnose.h>
#include <tools/bigint.hxx>


#include <string.h>
#include <ctype.h>

static const long MY_MAXLONG  = 0x3fffffff;
static const long MY_MINLONG  = -MY_MAXLONG;
static const long MY_MAXSHORT = 0x00007fff;
static const long MY_MINSHORT = -MY_MAXSHORT;

/*
 * The algorithms for Addition, Subtraction, Multiplication and Division
 * of large numbers originate from SEMINUMERICAL ALGORITHMS by
 * DONALD E. KNUTH in the series The Art of Computer Programming:
 * chapter 4.3.1. The Classical Algorithms.
 */

// TODO: Needs conversion to sal_uInt16/INT16/sal_uInt32/sal_Int32
void BigInt::MakeBigInt( const BigInt& rVal )
{
    if ( rVal.bIsBig )
    {
        memcpy( static_cast<void*>(this), static_cast<const void*>(&rVal), sizeof( BigInt ) );
        while ( nLen > 1 && nNum[nLen-1] == 0 )
            nLen--;
    }
    else
    {
        long nTmp = rVal.nVal;

        nVal   = rVal.nVal;
        bIsBig = true;
        if ( nTmp < 0 )
        {
            bIsNeg = true;
            nTmp = -nTmp;
        }
        else
            bIsNeg = false;

        nNum[0] = (sal_uInt16)(nTmp & 0xffffL);
        nNum[1] = (sal_uInt16)(nTmp >> 16);
        if ( nTmp & 0xffff0000L )
            nLen = 2;
        else
            nLen = 1;
    }
}

void BigInt::Normalize()
{
    if ( bIsBig )
    {
        while ( nLen > 1 && nNum[nLen-1] == 0 )
            nLen--;

        if ( nLen < 3 )
        {
            if ( nLen < 2 )
                nVal = nNum[0];
            else if ( nNum[1] & 0x8000 )
                return;
            else
                nVal = ((long)nNum[1] << 16) + nNum[0];

            bIsBig = false;

            if ( bIsNeg )
                nVal = -nVal;
        }
        // else nVal is undefined !!! W.P.
    }
    // why? nVal is undefined ??? W.P.
    else if ( nVal & 0xFFFF0000L )
        nLen = 2;
    else
        nLen = 1;
}

void BigInt::Mult( const BigInt &rVal, sal_uInt16 nMul )
{
    sal_uInt16 nK = 0;
    for ( int i = 0; i < rVal.nLen; i++ )
    {
        sal_uInt32 nTmp = (sal_uInt32)rVal.nNum[i] * (sal_uInt32)nMul + nK;
        nK            = (sal_uInt16)(nTmp >> 16);
        nNum[i] = (sal_uInt16)nTmp;
    }

    if ( nK )
    {
        nNum[rVal.nLen] = nK;
        nLen = rVal.nLen + 1;
    }
    else
        nLen = rVal.nLen;

    bIsBig = true;
    bIsNeg = rVal.bIsNeg;
}

void BigInt::Div( sal_uInt16 nDiv, sal_uInt16& rRem )
{
    sal_uInt32 nK = 0;
    for ( int i = nLen - 1; i >= 0; i-- )
    {
        sal_uInt32 nTmp = (sal_uInt32)nNum[i] + (nK << 16);
        nNum[i] = (sal_uInt16)(nTmp / nDiv);
        nK            = nTmp % nDiv;
    }
    rRem = (sal_uInt16)nK;

    if ( nNum[nLen-1] == 0 )
        nLen -= 1;
}

bool BigInt::IsLess( const BigInt& rVal ) const
{
    if ( rVal.nLen < nLen)
        return true;
    if ( rVal.nLen > nLen )
        return false;

    int i;
    for ( i = nLen - 1; i > 0 && nNum[i] == rVal.nNum[i]; i-- )
    {
    }
    return rVal.nNum[i] < nNum[i];
}

void BigInt::AddLong( BigInt& rB, BigInt& rErg )
{
    if ( bIsNeg == rB.bIsNeg )
    {
        int  i;
        char len;

        // if length of the two values differ, fill remaining positions
        // of the smaller value with zeros.
        if (nLen >= rB.nLen)
        {
            len = nLen;
            for (i = rB.nLen; i < len; i++)
                rB.nNum[i] = 0;
        }
        else
        {
            len = rB.nLen;
            for (i = nLen; i < len; i++)
                nNum[i] = 0;
        }

        // Add numerals, starting from the back
        long k;
        long nZ = 0;
        for (i = 0, k = 0; i < len; i++) {
            nZ = (long)nNum[i] + (long)rB.nNum[i] + k;
            if (nZ & 0xff0000L)
                k = 1;
            else
                k = 0;
            rErg.nNum[i] = (sal_uInt16)(nZ & 0xffffL);
        }
        // If an overflow occurred, add to solution
        if (nZ & 0xff0000L) // or if(k)
        {
            rErg.nNum[i] = 1;
            len++;
        }
        // Set length and sign
        rErg.nLen   = len;
        rErg.bIsNeg = bIsNeg && rB.bIsNeg;
        rErg.bIsBig = true;
    }
    // If one of the values is negative, perform subtraction instead
    else if (bIsNeg)
    {
        bIsNeg = false;
        rB.SubLong(*this, rErg);
        bIsNeg = true;
    }
    else
    {
        rB.bIsNeg = false;
        SubLong(rB, rErg);
        rB.bIsNeg = true;
    }
}

void BigInt::SubLong( BigInt& rB, BigInt& rErg )
{
    if ( bIsNeg == rB.bIsNeg )
    {
        int  i;
        char len;
        long nZ, k;

        // if length of the two values differ, fill remaining positions
        // of the smaller value with zeros.
        if (nLen >= rB.nLen)
        {
            len = nLen;
            for (i = rB.nLen; i < len; i++)
                rB.nNum[i] = 0;
        }
        else
        {
            len = rB.nLen;
            for (i = nLen; i < len; i++)
                nNum[i] = 0;
        }

        if ( IsLess(rB) )
        {
            for (i = 0, k = 0; i < len; i++)
            {
                nZ = (long)nNum[i] - (long)rB.nNum[i] + k;
                if (nZ < 0)
                    k = -1;
                else
                    k = 0;
                rErg.nNum[i] = (sal_uInt16)(nZ & 0xffffL);
            }
            rErg.bIsNeg = bIsNeg;
        }
        else
        {
            for (i = 0, k = 0; i < len; i++)
            {
                nZ = (long)rB.nNum[i] - (long)nNum[i] + k;
                if (nZ < 0)
                    k = -1;
                else
                    k = 0;
                rErg.nNum[i] = (sal_uInt16)(nZ & 0xffffL);
            }
            // if a < b, revert sign
            rErg.bIsNeg = !bIsNeg;
        }
        rErg.nLen   = len;
        rErg.bIsBig = true;
    }
    // If one of the values is negative, perform addition instead
    else if (bIsNeg)
    {
        bIsNeg = false;
        AddLong(rB, rErg);
        bIsNeg = true;
        rErg.bIsNeg = true;
    }
    else
    {
        rB.bIsNeg = false;
        AddLong(rB, rErg);
        rB.bIsNeg = true;
        rErg.bIsNeg = false;
    }
}

void BigInt::MultLong( const BigInt& rB, BigInt& rErg ) const
{
    int    i, j;
    sal_uInt32  nZ, k;

    rErg.bIsNeg = bIsNeg != rB.bIsNeg;
    rErg.bIsBig = true;
    rErg.nLen   = nLen + rB.nLen;

    for (i = 0; i < rErg.nLen; i++)
        rErg.nNum[i] = 0;

    for (j = 0; j < rB.nLen; j++)
    {
        for (i = 0, k = 0; i < nLen; i++)
        {
            nZ = (sal_uInt32)nNum[i] * (sal_uInt32)rB.nNum[j] +
                 (sal_uInt32)rErg.nNum[i + j] + k;
            rErg.nNum[i + j] = (sal_uInt16)(nZ & 0xffffUL);
            k = nZ >> 16;
        }
        rErg.nNum[i + j] = (sal_uInt16)k;
    }
}

void BigInt::DivLong( const BigInt& rB, BigInt& rErg ) const
{
    int    i, j;
    sal_uInt16 nK, nQ, nMult;
    short  nLenB  = rB.nLen;
    short  nLenB1 = rB.nLen - 1;
    BigInt aTmpA, aTmpB;

    nMult = (sal_uInt16)(0x10000L / ((long)rB.nNum[nLenB1] + 1));

    aTmpA.Mult( *this, nMult );
    if ( aTmpA.nLen == nLen )
    {
        aTmpA.nNum[aTmpA.nLen] = 0;
        aTmpA.nLen++;
    }

    aTmpB.Mult( rB, nMult );

    for (j = aTmpA.nLen - 1; j >= nLenB; j--)
    { // guess divisor
        long nTmp = ( (long)aTmpA.nNum[j] << 16 ) + aTmpA.nNum[j - 1];
        if (aTmpA.nNum[j] == aTmpB.nNum[nLenB1])
            nQ = 0xFFFF;
        else
            nQ = (sal_uInt16)(((sal_uInt32)nTmp) / aTmpB.nNum[nLenB1]);

        if ( ((sal_uInt32)aTmpB.nNum[nLenB1 - 1] * nQ) >
            ((((sal_uInt32)nTmp) - aTmpB.nNum[nLenB1] * nQ) << 16) + aTmpA.nNum[j - 2])
            nQ--;
        // Start division
        nK = 0;
        for (i = 0; i < nLenB; i++)
        {
            nTmp = (long)aTmpA.nNum[j - nLenB + i]
                   - ((long)aTmpB.nNum[i] * nQ)
                   - nK;
            aTmpA.nNum[j - nLenB + i] = (sal_uInt16)nTmp;
            nK = (sal_uInt16) (nTmp >> 16);
            if ( nK )
                nK = (sal_uInt16)(0x10000UL - nK);
        }
        unsigned short& rNum( aTmpA.nNum[j - nLenB + i] );
        rNum = rNum - nK;   // MSVC yields a warning on -= here, so don't use it
        if (aTmpA.nNum[j - nLenB + i] == 0)
            rErg.nNum[j - nLenB] = nQ;
        else
        {
            rErg.nNum[j - nLenB] = nQ - 1;
            nK = 0;
            for (i = 0; i < nLenB; i++)
            {
                nTmp = aTmpA.nNum[j - nLenB + i] + aTmpB.nNum[i] + nK;
                aTmpA.nNum[j - nLenB + i] = (sal_uInt16)(nTmp & 0xFFFFL);
                if (nTmp & 0xFFFF0000L)
                    nK = 1;
                else
                    nK = 0;
            }
        }
    }

    rErg.bIsNeg = bIsNeg != rB.bIsNeg;
    rErg.bIsBig = true;
    rErg.nLen   = nLen - rB.nLen + 1;
}

void BigInt::ModLong( const BigInt& rB, BigInt& rErg ) const
{
    short  i, j;
    sal_uInt16 nK, nQ, nMult;
    short  nLenB  = rB.nLen;
    short  nLenB1 = rB.nLen - 1;
    BigInt aTmpA, aTmpB;

    nMult = (sal_uInt16)(0x10000L / ((long)rB.nNum[nLenB1] + 1));

    aTmpA.Mult( *this, nMult);
    if ( aTmpA.nLen == nLen )
    {
        aTmpA.nNum[aTmpA.nLen] = 0;
        aTmpA.nLen++;
    }

    aTmpB.Mult( rB, nMult);

    for (j = aTmpA.nLen - 1; j >= nLenB; j--)
    { // Guess divisor
        long nTmp = ( (long)aTmpA.nNum[j] << 16 ) + aTmpA.nNum[j - 1];
        if (aTmpA.nNum[j] == aTmpB.nNum[nLenB1])
            nQ = 0xFFFF;
        else
            nQ = (sal_uInt16)(((sal_uInt32)nTmp) / aTmpB.nNum[nLenB1]);

        if ( ((sal_uInt32)aTmpB.nNum[nLenB1 - 1] * nQ) >
            ((((sal_uInt32)nTmp) - aTmpB.nNum[nLenB1] * nQ) << 16) + aTmpA.nNum[j - 2])
            nQ--;
        // Start division
        nK = 0;
        for (i = 0; i < nLenB; i++)
        {
            nTmp = (long)aTmpA.nNum[j - nLenB + i]
                   - ((long)aTmpB.nNum[i] * nQ)
                   - nK;
            aTmpA.nNum[j - nLenB + i] = (sal_uInt16)nTmp;
            nK = (sal_uInt16) (nTmp >> 16);
            if ( nK )
                nK = (sal_uInt16)(0x10000UL - nK);
        }
        unsigned short& rNum( aTmpA.nNum[j - nLenB + i] );
        rNum = rNum - nK;
        if (aTmpA.nNum[j - nLenB + i] == 0)
            rErg.nNum[j - nLenB] = nQ;
        else
        {
            rErg.nNum[j - nLenB] = nQ - 1;
            nK = 0;
            for (i = 0; i < nLenB; i++) {
                nTmp = aTmpA.nNum[j - nLenB + i] + aTmpB.nNum[i] + nK;
                aTmpA.nNum[j - nLenB + i] = (sal_uInt16)(nTmp & 0xFFFFL);
                if (nTmp & 0xFFFF0000L)
                    nK = 1;
                else
                    nK = 0;
            }
        }
    }

    rErg = aTmpA;
    rErg.Div( nMult, nQ );
}

bool BigInt::ABS_IsLess( const BigInt& rB ) const
{
    if (bIsBig || rB.bIsBig)
    {
        BigInt nA, nB;
        nA.MakeBigInt( *this );
        nB.MakeBigInt( rB );
        if (nA.nLen == nB.nLen)
        {
            int i;
            for (i = nA.nLen - 1; i > 0 && nA.nNum[i] == nB.nNum[i]; i--)
            {
            }
            return nA.nNum[i] < nB.nNum[i];
        }
        else
            return nA.nLen < nB.nLen;
    }
    if ( nVal < 0 )
        if ( rB.nVal < 0 )
            return nVal > rB.nVal;
        else
            return nVal > -rB.nVal;
    else
        if ( rB.nVal < 0 )
            return nVal < -rB.nVal;
        else
            return nVal < rB.nVal;
}

BigInt::BigInt( const BigInt& rBigInt )
    : nLen(0)
    , bIsNeg(false)
{
    if ( rBigInt.bIsBig )
        memcpy( static_cast<void*>(this), static_cast<const void*>(&rBigInt), sizeof( BigInt ) );
    else
    {
        bIsSet = rBigInt.bIsSet;
        bIsBig = false;
        nVal   = rBigInt.nVal;
    }
}

BigInt::BigInt( const OUString& rString )
    : nLen(0)
{
    bIsSet = true;
    bIsNeg = false;
    bIsBig = false;
    nVal   = 0;

    bool bNeg = false;
    const sal_Unicode* p = rString.getStr();
    if ( *p == '-' )
    {
        bNeg = true;
        p++;
    }
    while( *p >= '0' && *p <= '9' )
    {
        *this *= 10;
        *this += *p - '0';
        p++;
    }
    if ( bIsBig )
        bIsNeg = bNeg;
    else if( bNeg )
        nVal = -nVal;
}

BigInt::BigInt( double nValue )
    : nVal(0)
{
    bIsSet = true;

    if ( nValue < 0 )
    {
        nValue *= -1;
        bIsNeg  = true;
    }
    else
    {
        bIsNeg  = false;
    }

    if ( nValue < 1 )
    {
        bIsBig = false;
        nVal   = 0;
        nLen   = 0;
    }
    else
    {
        bIsBig = true;

        int i=0;

        while ( ( nValue > 65536.0 ) && ( i < MAX_DIGITS ) )
        {
            nNum[i] = (sal_uInt16) fmod( nValue, 65536.0 );
            nValue -= nNum[i];
            nValue /= 65536.0;
            i++;
        }
        if ( i < MAX_DIGITS )
            nNum[i++] = (sal_uInt16) nValue;

        nLen = i;

        if ( i < 3 )
            Normalize();
    }
}

BigInt::BigInt( sal_uInt32 nValue )
    : nVal(0)
{
    bIsSet  = true;
    if ( nValue & 0x80000000UL )
    {
        bIsBig  = true;
        bIsNeg  = false;
        nNum[0] = (sal_uInt16)(nValue & 0xffffUL);
        nNum[1] = (sal_uInt16)(nValue >> 16);
        nLen    = 2;
    }
    else
    {
        bIsBig = false;
        bIsNeg = false;
        nVal   = nValue;
        nLen   = 0;
    }
}

#if SAL_TYPES_SIZEOFLONG < SAL_TYPES_SIZEOFLONGLONG
BigInt::BigInt( long long nValue )
    : nVal(0)
{
    bIsSet = true;
    bIsNeg = nValue < 0;
    nLen = 0;

    if ((nValue >= std::numeric_limits<long>::min()) && (nValue <= std::numeric_limits<long>::max()))
    {
        bIsBig = false;
        nVal   = static_cast<long>(nValue);
    }
    else
    {
        bIsBig  = true;
        unsigned long long nUValue = static_cast<unsigned long long>(bIsNeg ? -nValue : nValue);
        for (int i = 0; (i != sizeof(unsigned long long) / 2) && (nUValue != 0); ++i)
        {
            nNum[i] = static_cast<sal_uInt16>(nUValue & 0xffffUL);
            nUValue = nUValue >> 16;
            ++nLen;
        }
    }
}
#endif

BigInt::operator sal_uIntPtr() const
{
    if ( !bIsBig )
        return (sal_uInt32)nVal;
    else if ( nLen == 2 )
    {
        sal_uInt32 nRet;
        nRet  = ((sal_uInt32)nNum[1]) << 16;
        nRet += nNum[0];
        return nRet;
    }
    return 0;
}

BigInt::operator double() const
{
    if ( !bIsBig )
        return (double) nVal;
    else
    {
        int     i = nLen-1;
        double  nRet = (double) ((sal_uInt32)nNum[i]);

        while ( i )
        {
            nRet *= 65536.0;
            i--;
            nRet += (double) ((sal_uInt32)nNum[i]);
        }

        if ( bIsNeg )
            nRet *= -1;

        return nRet;
    }
}

BigInt& BigInt::operator=( const BigInt& rBigInt )
{
    if (this == &rBigInt)
        return *this;

    if ( rBigInt.bIsBig )
        memcpy( static_cast<void*>(this), static_cast<const void*>(&rBigInt), sizeof( BigInt ) );
    else
    {
        bIsSet = rBigInt.bIsSet;
        bIsBig = false;
        nVal   = rBigInt.nVal;
    }
    return *this;
}

BigInt& BigInt::operator+=( const BigInt& rVal )
{
    if ( !bIsBig && !rVal.bIsBig )
    {
        if( nVal <= MY_MAXLONG && rVal.nVal <= MY_MAXLONG
            && nVal >= MY_MINLONG && rVal.nVal >= MY_MINLONG )
        { // No overflows may occur here
            nVal += rVal.nVal;
            return *this;
        }

        if( (nVal < 0) != (rVal.nVal < 0) )
        { // No overflows may occur here
            nVal += rVal.nVal;
            return *this;
        }
    }

    BigInt aTmp1, aTmp2;
    aTmp1.MakeBigInt( *this );
    aTmp2.MakeBigInt( rVal );
    aTmp1.AddLong( aTmp2, *this );
    Normalize();
    return *this;
}

BigInt& BigInt::operator-=( const BigInt& rVal )
{
    if ( !bIsBig && !rVal.bIsBig )
    {
        if ( nVal <= MY_MAXLONG && rVal.nVal <= MY_MAXLONG &&
             nVal >= MY_MINLONG && rVal.nVal >= MY_MINLONG )
        { // No overflows may occur here
            nVal -= rVal.nVal;
            return *this;
        }

        if ( (nVal < 0) == (rVal.nVal < 0) )
        { // No overflows may occur here
            nVal -= rVal.nVal;
            return *this;
        }
    }

    BigInt aTmp1, aTmp2;
    aTmp1.MakeBigInt( *this );
    aTmp2.MakeBigInt( rVal );
    aTmp1.SubLong( aTmp2, *this );
    Normalize();
    return *this;
}

BigInt& BigInt::operator*=( const BigInt& rVal )
{
    if ( !bIsBig && !rVal.bIsBig
         && nVal <= MY_MAXSHORT && rVal.nVal <= MY_MAXSHORT
         && nVal >= MY_MINSHORT && rVal.nVal >= MY_MINSHORT )
         // TODO: not optimal !!! W.P.
    { // No overflows may occur here
        nVal *= rVal.nVal;
    }
    else
    {
        BigInt aTmp1, aTmp2;
        aTmp1.MakeBigInt( rVal );
        aTmp2.MakeBigInt( *this );
        aTmp1.MultLong(aTmp2, *this);
        Normalize();
    }
    return *this;
}

BigInt& BigInt::operator/=( const BigInt& rVal )
{
    if ( !rVal.bIsBig )
    {
        if ( rVal.nVal == 0 )
        {
            OSL_FAIL( "BigInt::operator/ --> divide by zero" );
            return *this;
        }

        if ( !bIsBig )
        {
            // No overflows may occur here
            nVal /= rVal.nVal;
            return *this;
        }

        if ( rVal.nVal == 1 )
            return *this;

        if ( rVal.nVal == -1 )
        {
            bIsNeg = !bIsNeg;
            return *this;
        }

        if ( rVal.nVal <= (long)0xFFFF && rVal.nVal >= -(long)0xFFFF )
        {
            // Divide BigInt with an sal_uInt16
            sal_uInt16 nTmp;
            if ( rVal.nVal < 0 )
            {
                nTmp = (sal_uInt16) -rVal.nVal;
                bIsNeg = !bIsNeg;
            }
            else
                nTmp = (sal_uInt16) rVal.nVal;

            Div( nTmp, nTmp );
            Normalize();
            return *this;
        }
    }

    if ( ABS_IsLess( rVal ) )
    {
        *this = BigInt( (long)0 );
        return *this;
    }

    // Divide BigInt with BigInt
    BigInt aTmp1, aTmp2;
    aTmp1.MakeBigInt( *this );
    aTmp2.MakeBigInt( rVal );
    aTmp1.DivLong(aTmp2, *this);
    Normalize();
    return *this;
}

BigInt& BigInt::operator%=( const BigInt& rVal )
{
    if ( !rVal.bIsBig )
    {
        if ( rVal.nVal == 0 )
        {
            OSL_FAIL( "BigInt::operator/ --> divide by zero" );
            return *this;
        }

        if ( !bIsBig )
        {
            // No overflows may occur here
            nVal %= rVal.nVal;
            return *this;
        }

        if ( rVal.nVal <= (long)0xFFFF && rVal.nVal >= -(long)0xFFFF )
        {
            // Divide Bigint by short
            sal_uInt16 nTmp;
            if ( rVal.nVal < 0 )
            {
                nTmp = (sal_uInt16) -rVal.nVal;
                bIsNeg = !bIsNeg;
            }
            else
                nTmp = (sal_uInt16) rVal.nVal;

            Div( nTmp, nTmp );
            *this = BigInt( (long)nTmp );
            return *this;
        }
    }

    if ( ABS_IsLess( rVal ) )
        return *this;

    // Divide BigInt with BigInt
    BigInt aTmp1, aTmp2;
    aTmp1.MakeBigInt( *this );
    aTmp2.MakeBigInt( rVal );
    aTmp1.ModLong(aTmp2, *this);
    Normalize();
    return *this;
}

bool operator==( const BigInt& rVal1, const BigInt& rVal2 )
{
    if ( rVal1.bIsBig || rVal2.bIsBig )
    {
        BigInt nA, nB;
        nA.MakeBigInt( rVal1 );
        nB.MakeBigInt( rVal2 );
        if ( nA.bIsNeg == nB.bIsNeg )
        {
            if ( nA.nLen == nB.nLen )
            {
                int i;
                for ( i = nA.nLen - 1; i > 0 && nA.nNum[i] == nB.nNum[i]; i-- )
                {
                }

                return nA.nNum[i] == nB.nNum[i];
            }
            return false;
        }
        return false;
    }
    return rVal1.nVal == rVal2.nVal;
}

bool operator<( const BigInt& rVal1, const BigInt& rVal2 )
{
    if ( rVal1.bIsBig || rVal2.bIsBig )
    {
        BigInt nA, nB;
        nA.MakeBigInt( rVal1 );
        nB.MakeBigInt( rVal2 );
        if ( nA.bIsNeg == nB.bIsNeg )
        {
            if ( nA.nLen == nB.nLen )
            {
                int i;
                for ( i = nA.nLen - 1; i > 0 && nA.nNum[i] == nB.nNum[i]; i-- )
                {
                }

                if ( nA.bIsNeg )
                    return nA.nNum[i] > nB.nNum[i];
                else
                    return nA.nNum[i] < nB.nNum[i];
            }
            if ( nA.bIsNeg )
                return nA.nLen > nB.nLen;
            else
                return nA.nLen < nB.nLen;
        }
        return !nB.bIsNeg;
    }
    return rVal1.nVal < rVal2.nVal;
}

bool operator >(const BigInt& rVal1, const BigInt& rVal2 )
{
    if ( rVal1.bIsBig || rVal2.bIsBig )
    {
        BigInt nA, nB;
        nA.MakeBigInt( rVal1 );
        nB.MakeBigInt( rVal2 );
        if ( nA.bIsNeg == nB.bIsNeg )
        {
            if ( nA.nLen == nB.nLen )
            {
                int i;
                for ( i = nA.nLen - 1; i > 0 && nA.nNum[i] == nB.nNum[i]; i-- )
                {
                }

                if ( nA.bIsNeg )
                    return nA.nNum[i] < nB.nNum[i];
                else
                    return nA.nNum[i] > nB.nNum[i];
            }
            if ( nA.bIsNeg )
                return nA.nLen < nB.nLen;
            else
                return nA.nLen > nB.nLen;
        }
        return !nA.bIsNeg;
    }

    return rVal1.nVal > rVal2.nVal;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
