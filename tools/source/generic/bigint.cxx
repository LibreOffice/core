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

#include <sal/config.h>

#include <osl/diagnose.h>
#include <tools/bigint.hxx>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>
#include <span>

/**
 * The range in which we can perform add/sub without fear of overflow
 */
const sal_Int32 MY_MAXLONG  = 0x3fffffff;
const sal_Int32 MY_MINLONG  = -MY_MAXLONG;

/*
 * The algorithms for Addition, Subtraction, Multiplication and Division
 * of large numbers originate from SEMINUMERICAL ALGORITHMS by
 * DONALD E. KNUTH in the series The Art of Computer Programming:
 * chapter 4.3.1. The Classical Algorithms.
 */

BigInt BigInt::MakeBig() const
{
    if (IsBig())
    {
        BigInt ret(*this);
        while ( ret.nLen > 1 && ret.nNum[ret.nLen-1] == 0 )
            ret.nLen--;
        return ret;
    }

    BigInt ret;
    if (nVal < 0)
    {
        ret.bIsNeg = true;
        ret.nNum[0] = -static_cast<sal_Int64>(nVal);
    }
    else
    {
        ret.bIsNeg = false;
        ret.nNum[0] = nVal;
    }
    ret.nLen = 1;
    return ret;
}

void BigInt::Normalize()
{
    if (IsBig())
    {
        while ( nLen > 1 && nNum[nLen-1] == 0 )
            nLen--;

        if (nLen < 2)
        {
            static constexpr sal_uInt32 maxForPosInt32 = std::numeric_limits<sal_Int32>::max();
            static constexpr sal_uInt32 maxForNegInt32 = -sal_Int64(std::numeric_limits<sal_Int32>::min());
            sal_uInt32 nNum0 = nNum[0];
            if (bIsNeg && nNum0 <= maxForNegInt32)
            {
                nVal = -sal_Int64(nNum0);
                nLen = 0;
            }
            else if (!bIsNeg && nNum0 <= maxForPosInt32)
            {
                nVal = nNum0;
                nLen = 0;
            }
        }
    }
}

// Normalization in DivLong requires that dividend is multiplied by a number, and the resulting
// value has 1 more 32-bit "digits". 'ret' provides enough room for that. Use of std::span gives
// run-time index checking in debug builds.
static std::span<sal_uInt32> Mult(std::span<const sal_uInt32> aNum, sal_uInt32 nMul, std::span<sal_uInt32> retBuf)
{
    assert(retBuf.size() >= aNum.size());
    sal_uInt64 nK = 0;
    for (size_t i = 0; i < aNum.size(); i++)
    {
        sal_uInt64 nTmp = static_cast<sal_uInt64>(aNum[i]) * nMul + nK;
        nK = nTmp >> 32;
        retBuf[i] = static_cast<sal_uInt32>(nTmp);
    }

    if ( nK )
    {
        assert(retBuf.size() > aNum.size());
        retBuf[aNum.size()] = nK;
        return retBuf.subspan(0, aNum.size() + 1);
    }

    return retBuf.subspan(0, aNum.size());
}

static size_t DivInPlace(std::span<sal_uInt32> aNum, sal_uInt32 nDiv, sal_uInt32& rRem)
{
    assert(aNum.size() > 0);
    sal_uInt64 nK = 0;
    for (int i = aNum.size() - 1; i >= 0; i--)
    {
        sal_uInt64 nTmp = aNum[i] + (nK << 32);
        aNum[i] = nTmp / nDiv;
        nK            = nTmp % nDiv;
    }
    rRem = nK;

    if (aNum[aNum.size() - 1] == 0)
        return aNum.size() - 1;

    return aNum.size();
}

bool BigInt::ABS_IsLessBig(const BigInt& rVal) const
{
    assert(IsBig() && rVal.IsBig());
    if ( rVal.nLen < nLen)
        return false;
    if ( rVal.nLen > nLen )
        return true;

    int i = nLen - 1;
    while (i > 0 && nNum[i] == rVal.nNum[i])
        --i;
    return nNum[i] < rVal.nNum[i];
}

void BigInt::AddBig(BigInt& rB, BigInt& rRes)
{
    assert(IsBig() && rB.IsBig());
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
        sal_Int64 k = 0;
        for (i = 0; i < len; i++) {
            sal_Int64 nZ = static_cast<sal_Int64>(nNum[i]) + static_cast<sal_Int64>(rB.nNum[i]) + k;
            if (nZ > sal_Int64(std::numeric_limits<sal_uInt32>::max()))
                k = 1;
            else
                k = 0;
            rRes.nNum[i] = static_cast<sal_uInt32>(nZ);
        }
        // If an overflow occurred, add to solution
        if (k)
        {
            assert(i < MAX_DIGITS);
            rRes.nNum[i] = 1;
            len++;
        }
        // Set length and sign
        rRes.nLen = len;
        rRes.bIsNeg = bIsNeg;
    }
    // If one of the values is negative, perform subtraction instead
    else
    {
        bIsNeg = !bIsNeg;
        rB.SubBig(*this, rRes);
        bIsNeg = !bIsNeg;
    }
}

void BigInt::SubBig(BigInt& rB, BigInt& rRes)
{
    assert(IsBig() && rB.IsBig());
    if ( bIsNeg == rB.bIsNeg )
    {
        char len;

        // if length of the two values differ, fill remaining positions
        // of the smaller value with zeros.
        if (nLen >= rB.nLen)
        {
            len = nLen;
            for (int i = rB.nLen; i < len; i++)
                rB.nNum[i] = 0;
        }
        else
        {
            len = rB.nLen;
            for (int i = nLen; i < len; i++)
                nNum[i] = 0;
        }

        const bool bThisIsLess = ABS_IsLessBig(rB);
        BigInt& rGreater = bThisIsLess ? rB : *this;
        BigInt& rSmaller = bThisIsLess ? *this : rB;

        sal_Int64 k = 0;
        for (int i = 0; i < len; i++)
        {
            sal_Int64 nZ = static_cast<sal_Int64>(rGreater.nNum[i]) - static_cast<sal_Int64>(rSmaller.nNum[i]) + k;
            if (nZ < 0)
                k = -1;
            else
                k = 0;
            rRes.nNum[i] = static_cast<sal_uInt32>(nZ);
        }

        // if a < b, revert sign
        rRes.bIsNeg = bThisIsLess ? !bIsNeg : bIsNeg;
        rRes.nLen   = len;
    }
    // If one of the values is negative, perform addition instead
    else
    {
        bIsNeg = !bIsNeg;
        AddBig(rB, rRes);
        bIsNeg = !bIsNeg;
        rRes.bIsNeg = bIsNeg;
    }
}

void BigInt::MultBig(const BigInt& rB, BigInt& rRes) const
{
    assert(IsBig() && rB.IsBig());

    rRes.bIsNeg = bIsNeg != rB.bIsNeg;
    rRes.nLen = nLen + rB.nLen;
    assert(rRes.nLen <= MAX_DIGITS);

    for (int i = 0; i < rRes.nLen; i++)
        rRes.nNum[i] = 0;

    for (int j = 0; j < rB.nLen; j++)
    {
        sal_uInt64 k = 0;
        int i;
        for (i = 0; i < nLen; i++)
        {
            sal_uInt64 nZ = static_cast<sal_uInt64>(nNum[i]) * static_cast<sal_uInt64>(rB.nNum[j]) +
                 static_cast<sal_uInt64>(rRes.nNum[i + j]) + k;
            rRes.nNum[i + j] = static_cast<sal_uInt32>(nZ);
            k = nZ >> 32;
        }
        rRes.nNum[i + j] = k;
    }
}

void BigInt::DivModBig(const BigInt& rB, BigInt* pDiv, BigInt* pMod) const
{
    assert(IsBig() && rB.IsBig());
    assert(nLen >= rB.nLen);

    assert(rB.nNum[rB.nLen - 1] != 0);
    sal_uInt32 nMult = static_cast<sal_uInt32>(0x100000000 / (static_cast<sal_Int64>(rB.nNum[rB.nLen - 1]) + 1));

    sal_uInt32 numBuf[MAX_DIGITS + 1]; // normalized dividend
    auto num = Mult({ nNum, nLen }, nMult, numBuf);
    if (num.size() == nLen)
    {
        num = std::span(numBuf, nLen + 1);
        num[nLen] = 0;
    }

    sal_uInt32 denBuf[MAX_DIGITS + 1]; // normalized divisor
    const auto den = Mult({ rB.nNum, rB.nLen }, nMult, denBuf);
    assert(den.size() == rB.nLen);
    const sal_uInt64 nDenMostSig = den[rB.nLen - 1];
    assert(nDenMostSig >= 0x100000000 / 2);
    const sal_uInt64 nDen2ndSig = rB.nLen > 1 ? den[rB.nLen - 2] : 0;

    BigInt aTmp;
    BigInt& rRes = pDiv ? *pDiv : aTmp;

    for (size_t j = num.size() - 1; j >= den.size(); j--)
    { // guess divisor
        assert(num[j] < nDenMostSig || (num[j] == nDenMostSig && num[j - 1] == 0));
        sal_uInt64 nTmp = ( static_cast<sal_uInt64>(num[j]) << 32 ) + num[j - 1];
        sal_uInt32 nQ;
        if (num[j] == nDenMostSig)
            nQ = 0xFFFFFFFF;
        else
            nQ = static_cast<sal_uInt32>(nTmp / nDenMostSig);

        if (nDen2ndSig && (nDen2ndSig * nQ) > ((nTmp - nDenMostSig * nQ) << 32) + num[j - 2])
            nQ--;
        // Start division
        sal_uInt32 nK = 0;
        size_t i;
        for (i = 0; i < den.size(); i++)
        {
            nTmp = static_cast<sal_uInt64>(num[j - den.size() + i])
                   - (static_cast<sal_uInt64>(den[i]) * nQ)
                   - nK;
            num[j - den.size() + i] = static_cast<sal_uInt32>(nTmp);
            nK = static_cast<sal_uInt32>(nTmp >> 32);
            if ( nK )
                nK = static_cast<sal_uInt32>(0x100000000 - nK);
        }
        sal_uInt32& rNum(num[j - den.size() + i]);
        rNum -= nK;
        if (num[j - den.size() + i] == 0)
            rRes.nNum[j - den.size()] = nQ;
        else
        {
            rRes.nNum[j - den.size()] = nQ - 1;
            nK = 0;
            for (i = 0; i < den.size(); i++)
            {
                nTmp = num[j - den.size() + i] + den[i] + nK;
                num[j - den.size() + i] = static_cast<sal_uInt32>(nTmp & 0xFFFFFFFF);
                if (nTmp > std::numeric_limits<sal_uInt32>::max())
                    nK = 1;
                else
                    nK = 0;
            }
        }
    }

    if (pMod)
    {
        pMod->nLen = DivInPlace(num, nMult, nMult);
        assert(pMod->nLen <= MAX_DIGITS);
        pMod->bIsNeg = bIsNeg;
        std::copy_n(num.begin(), pMod->nLen, pMod->nNum);
        pMod->Normalize();
    }
    if (pDiv) // rRes references pDiv
    {
        pDiv->bIsNeg = bIsNeg != rB.bIsNeg;
        pDiv->nLen = nLen - rB.nLen + 1;
        pDiv->Normalize();
    }
}

BigInt::BigInt( std::u16string_view rString )
    : nLen(0)
{
    bIsNeg = false;
    nVal   = 0;

    bool bNeg = false;
    auto p = rString.begin();
    auto pEnd = rString.end();
    if (p == pEnd)
        return;
    if ( *p == '-' )
    {
        bNeg = true;
        p++;
    }
    if (p == pEnd)
        return;
    while( p != pEnd && *p >= '0' && *p <= '9' )
    {
        *this *= 10;
        *this += *p - '0';
        p++;
    }
    if (IsBig())
        bIsNeg = bNeg;
    else if( bNeg )
        nVal = -nVal;
}

BigInt::BigInt( double nValue )
    : nVal(0)
{
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
        nVal   = 0;
        nLen   = 0;
    }
    else
    {
        int i=0;

        while ( ( nValue > 0x100000000 ) && ( i < MAX_DIGITS ) )
        {
            nNum[i] = static_cast<sal_uInt32>(fmod( nValue, 0x100000000 ));
            nValue -= nNum[i];
            nValue /= 0x100000000;
            i++;
        }
        if ( i < MAX_DIGITS )
            nNum[i++] = static_cast<sal_uInt32>(nValue);

        nLen = i;

        if ( i < 2 )
            Normalize();
    }
}

BigInt::BigInt( sal_uInt32 nValue )
    : nVal(0)
    , bIsNeg(false)
{
    if ( nValue & 0x80000000U )
    {
        nNum[0] = nValue;
        nLen = 1;
    }
    else
    {
        nVal   = nValue;
        nLen   = 0;
    }
}

BigInt::BigInt( sal_Int64 nValue )
    : nVal(0)
{
    bIsNeg = nValue < 0;
    nLen = 0;

    if ((nValue >= SAL_MIN_INT32) && (nValue <= SAL_MAX_INT32))
    {
        nVal = static_cast<sal_Int32>(nValue);
    }
    else
    {
        for (sal_uInt64 n = static_cast<sal_uInt64>(
                 (bIsNeg && nValue != std::numeric_limits<sal_Int64>::min()) ? -nValue : nValue);
             n != 0; n >>= 32)
            nNum[nLen++] = static_cast<sal_uInt32>(n);
    }
}

BigInt::operator double() const
{
    if (!IsBig())
        return static_cast<double>(nVal);
    else
    {
        int     i = nLen-1;
        double  nRet = static_cast<double>(nNum[i]);

        while ( i )
        {
            nRet *= 0x100000000;
            i--;
            nRet += static_cast<double>(nNum[i]);
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

    if (rBigInt.IsBig())
        memcpy( static_cast<void*>(this), static_cast<const void*>(&rBigInt), sizeof( BigInt ) );
    else
    {
        nLen = 0;
        nVal = rBigInt.nVal;
    }
    return *this;
}

BigInt& BigInt::operator+=( const BigInt& rVal )
{
    if (!IsBig() && !rVal.IsBig())
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

    BigInt aTmp2 = rVal.MakeBig();
    MakeBig().AddBig(aTmp2, *this);
    Normalize();
    return *this;
}

BigInt& BigInt::operator-=( const BigInt& rVal )
{
    if (!IsBig() && !rVal.IsBig())
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

    BigInt aTmp2 = rVal.MakeBig();
    MakeBig().SubBig(aTmp2, *this);
    Normalize();
    return *this;
}

BigInt& BigInt::operator*=( const BigInt& rVal )
{
    static const sal_Int32 MY_MAXSHORT = 0x00007fff;
    static const sal_Int32 MY_MINSHORT = -MY_MAXSHORT;

    if (!IsBig() && !rVal.IsBig()
         && nVal <= MY_MAXSHORT && rVal.nVal <= MY_MAXSHORT
         && nVal >= MY_MINSHORT && rVal.nVal >= MY_MINSHORT )
         // TODO: not optimal !!! W.P.
    { // No overflows may occur here
        nVal *= rVal.nVal;
    }
    else
    {
        rVal.MakeBig().MultBig(MakeBig(), *this);
        Normalize();
    }
    return *this;
}

void BigInt::DivMod(const BigInt& rVal, BigInt* pDiv, BigInt* pMod) const
{
    assert(pDiv || pMod); // Avoid useless calls
    if (!rVal.IsBig())
    {
        if ( rVal.nVal == 0 )
        {
            OSL_FAIL( "BigInt::operator/ --> divide by zero" );
            return;
        }

        if (rVal.nVal == 1)
        {
            if (pDiv)
            {
                *pDiv = *this;
                pDiv->Normalize();
            }
            if (pMod)
                *pMod = 0;
            return;
        }

        if (!IsBig())
        {
            // No overflows may occur here
            const sal_Int32 nDiv = nVal / rVal.nVal; // Compilers usually optimize adjacent
            const sal_Int32 nMod = nVal % rVal.nVal; // / and % into a single instruction
            if (pDiv)
                *pDiv = nDiv;
            if (pMod)
                *pMod = nMod;
            return;
        }

        if ( rVal.nVal == -1 )
        {
            if (pDiv)
            {
                *pDiv = *this;
                pDiv->bIsNeg = !bIsNeg;
                pDiv->Normalize();
            }
            if (pMod)
                *pMod = 0;
            return;
        }

        // Divide BigInt with an sal_uInt32
        sal_uInt32 nTmp;
        bool bNegate;
        if ( rVal.nVal < 0 )
        {
            nTmp = static_cast<sal_uInt32>(-rVal.nVal);
            bNegate = true;
        }
        else
        {
            nTmp = static_cast<sal_uInt32>(rVal.nVal);
            bNegate = false;
        }

        BigInt aTmp;
        BigInt& rDiv = pDiv ? *pDiv : aTmp;
        rDiv = *this;
        rDiv.nLen = DivInPlace({ rDiv.nNum, rDiv.nLen }, nTmp, nTmp);
        if (pDiv)
        {
            if (bNegate)
                pDiv->bIsNeg = !pDiv->bIsNeg;
            pDiv->Normalize();
        }
        if (pMod)
        {
            *pMod = BigInt(nTmp);
        }
        return;
    }

    BigInt tmpA = MakeBig(), tmpB = rVal.MakeBig();
    if (tmpA.ABS_IsLessBig(tmpB))
    {
        // First store *this to *pMod, then nullify *pDiv, to handle 'pDiv == this' case
        if (pMod)
        {
            *pMod = *this;
            pMod->Normalize();
        }
        if (pDiv)
            *pDiv = 0;
        return;
    }

    // Divide BigInt with BigInt
    tmpA.DivModBig(tmpB, pDiv, pMod);
}

BigInt& BigInt::operator/=( const BigInt& rVal )
{
    DivMod(rVal, this, nullptr);
    return *this;
}

BigInt& BigInt::operator%=( const BigInt& rVal )
{
    DivMod(rVal, nullptr, this);
    return *this;
}

bool operator==( const BigInt& rVal1, const BigInt& rVal2 )
{
    if (!rVal1.IsBig() && !rVal2.IsBig())
        return rVal1.nVal == rVal2.nVal;

    BigInt nA = rVal1.MakeBig(), nB = rVal2.MakeBig();
    return nA.bIsNeg == nB.bIsNeg && nA.nLen == nB.nLen
           && std::equal(nA.nNum, nA.nNum + nA.nLen, nB.nNum);
}

std::strong_ordering operator<=>(const BigInt& rVal1, const BigInt& rVal2)
{
    if (!rVal1.IsBig() && !rVal2.IsBig())
        return rVal1.nVal <=> rVal2.nVal;

    BigInt nA = rVal1.MakeBig(), nB = rVal2.MakeBig();
    if (nA.bIsNeg != nB.bIsNeg)
        return nB.bIsNeg ? std::strong_ordering::less : std::strong_ordering::greater;
    if (nA.nLen < nB.nLen)
        return nB.bIsNeg ? std::strong_ordering::greater : std::strong_ordering::less;
    if (nA.nLen > nB.nLen)
        return nB.bIsNeg ? std::strong_ordering::less : std::strong_ordering::greater;
    int i = nA.nLen - 1;
    while (i > 0 && nA.nNum[i] == nB.nNum[i])
        --i;
    return nB.bIsNeg ? (nB.nNum[i] <=> nA.nNum[i]) : (nA.nNum[i] <=> nB.nNum[i]);
}

tools::Long BigInt::Scale( tools::Long nVal, tools::Long nMul, tools::Long nDiv )
{
    BigInt aVal( nVal );

    aVal *= nMul;

    if ( aVal.IsNeg() != ( nDiv < 0 ) )
        aVal -= nDiv / 2; // for correct rounding
    else
        aVal += nDiv / 2; // for correct rounding

    aVal /= nDiv;

    return tools::Long( aVal );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
