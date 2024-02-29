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

#include <tools/fract.hxx>
#include <tools/debug.hxx>
#include <o3tl/hash_combine.hxx>
#include <o3tl/safeint.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>

#include <algorithm>
#include <bit>
#include <cmath>
#include <numeric>

#include <boost/rational.hpp>

static boost::rational<sal_Int32> rational_FromDouble(double dVal);
static void rational_ReduceInaccurate(boost::rational<sal_Int32>& rRational, unsigned nSignificantBits);
// Find the number of bits required to represent this number
static int impl_NumberOfBits(sal_uInt32 nNum) { return 32 - std::countl_zero(nNum); }

static boost::rational<sal_Int32> toRational(sal_Int32 n, sal_Int32 d)
{
    // https://github.com/boostorg/boost/issues/335 when these are std::numeric_limits<sal_Int32>::min
    if (n == d)
        return 1;
    // tdf#144319 avoid boost::bad_rational e.g. if numerator=-476741369, denominator=-2147483648
    if (d < -std::numeric_limits<sal_Int32>::max())
        return 0;
    return boost::rational<sal_Int32>(n, d);
}

static constexpr bool isOutOfRange(sal_Int64 nNum)
{
    return nNum < std::numeric_limits<sal_Int32>::min()
            || nNum > std::numeric_limits<sal_Int32>::max();
}

Fraction::Fraction( sal_Int64 nNum, sal_Int64 nDen ) : mnNumerator(nNum), mnDenominator(nDen)
{
    if ( isOutOfRange(nNum) || isOutOfRange(nDen) )
    {
        // tdf#143200
        if (const auto gcd = std::gcd(nNum, nDen); gcd > 1)
        {
            nNum /= gcd;
            nDen /= gcd;
        }
        SAL_WARN_IF(isOutOfRange(nNum) || isOutOfRange(nDen),
                    "tools.fraction", "values outside of range we can represent, doing reduction, which will reduce precision");
        while (isOutOfRange(nNum) || isOutOfRange(nDen))
        {
            nNum /= 2;
            nDen /= 2;
        }
        mnNumerator = nNum;
        mnDenominator = nDen;
    }
    if ( mnDenominator == 0 )
    {
        mbValid = false;
        SAL_WARN( "tools.fraction", "'Fraction(" << nNum << ",0)' invalid fraction created" );
        return;
    }
    else if ((nDen == -1 && nNum == std::numeric_limits<sal_Int32>::min()) ||
            (nNum == -1 && nDen == std::numeric_limits<sal_Int32>::min()))
    {
        mbValid = false;
        SAL_WARN("tools.fraction", "'Fraction(" << nNum << "," << nDen << ")' invalid fraction created");
        return;
    }
}

/**
 * only here to prevent passing of NaN
 */
Fraction::Fraction( double nNum, double nDen )
    : Fraction(sal_Int64(nNum), sal_Int64(nDen))
{}

Fraction::Fraction( double dVal )
{
    try
    {
        boost::rational<sal_Int32> v = rational_FromDouble( dVal );
        mnNumerator = v.numerator();
        mnDenominator = v.denominator();
    }
    catch (const boost::bad_rational&)
    {
        mbValid = false;
        SAL_WARN( "tools.fraction", "'Fraction(" << dVal << ")' invalid fraction created" );
    }
}

Fraction::operator double() const
{
    if (!mbValid)
    {
        SAL_WARN( "tools.fraction", "'double()' on invalid fraction" );
        return 0.0;
    }

    return boost::rational_cast<double>(toRational(mnNumerator, mnDenominator));
}

// This methods first validates both values.
// If one of the arguments is invalid, the whole operation is invalid.
// After computation detect if result overflows a sal_Int32 value
// which cause the operation to be marked as invalid
Fraction& Fraction::operator += ( const Fraction& rVal )
{
    if ( !rVal.mbValid )
        mbValid = false;

    if ( !mbValid )
    {
        SAL_WARN( "tools.fraction", "'operator +=' with invalid fraction" );
        return *this;
    }

    boost::rational<sal_Int32> a = toRational(mnNumerator, mnDenominator);
    a += toRational(rVal.mnNumerator, rVal.mnDenominator);
    mnNumerator = a.numerator();
    mnDenominator = a.denominator();

    return *this;
}

Fraction& Fraction::operator -= ( const Fraction& rVal )
{
    if ( !rVal.mbValid )
        mbValid = false;

    if ( !mbValid )
    {
        SAL_WARN( "tools.fraction", "'operator -=' with invalid fraction" );
        return *this;
    }

    boost::rational<sal_Int32> a = toRational(mnNumerator, mnDenominator);
    a -= toRational(rVal.mnNumerator, rVal.mnDenominator);
    mnNumerator = a.numerator();
    mnDenominator = a.denominator();

    return *this;
}

namespace
{
    bool checked_multiply_by(boost::rational<sal_Int32>& i, const boost::rational<sal_Int32>& r)
    {
        // Protect against self-modification
        sal_Int32 num = r.numerator();
        sal_Int32 den = r.denominator();

        // Fast-path if the number of bits in input is < the number of bits in the output, overflow cannot happen
        // This is considerably faster than repeated std::gcd() operations
        if ((impl_NumberOfBits(std::abs(i.numerator())) + impl_NumberOfBits(std::abs(r.numerator()))) < 32 &&
            (impl_NumberOfBits(std::abs(i.denominator())) + impl_NumberOfBits(std::abs(r.denominator()))) < 32)
        {
            i *= r;
            return false;
        }

        // Avoid overflow and preserve normalization
        sal_Int32 gcd1 = std::gcd(i.numerator(), den);
        sal_Int32 gcd2 = std::gcd(num, i.denominator());

        if (!gcd1 || !gcd2)
            return true;

        bool fail = false;
        fail |= o3tl::checked_multiply(i.numerator() / gcd1, num / gcd2, num);
        fail |= o3tl::checked_multiply(i.denominator() / gcd2, den / gcd1, den);

        if (!fail)
            i.assign(num, den);

        return fail;
    }
}

Fraction& Fraction::operator *= ( const Fraction& rVal )
{
    if ( !rVal.mbValid )
        mbValid = false;

    if ( !mbValid )
    {
        SAL_WARN( "tools.fraction", "'operator *=' with invalid fraction" );
        return *this;
    }

    boost::rational<sal_Int32> a = toRational(mnNumerator, mnDenominator);
    boost::rational<sal_Int32> b = toRational(rVal.mnNumerator, rVal.mnDenominator);
    bool bFail = checked_multiply_by(a, b);
    mnNumerator = a.numerator();
    mnDenominator = a.denominator();

    if (bFail)
    {
        mbValid = false;
    }

    return *this;
}

Fraction& Fraction::operator /= ( const Fraction& rVal )
{
    if ( !rVal.mbValid )
        mbValid = false;

    if ( !mbValid )
    {
        SAL_WARN( "tools.fraction", "'operator /=' with invalid fraction" );
        return *this;
    }

    boost::rational<sal_Int32> a = toRational(mnNumerator, mnDenominator);
    a /= toRational(rVal.mnNumerator, rVal.mnDenominator);
    mnNumerator = a.numerator();
    mnDenominator = a.denominator();

    return *this;
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
    if ( !mbValid )
    {
        SAL_WARN( "tools.fraction", "'ReduceInaccurate' on invalid fraction" );
        return;
    }

    if ( !mnNumerator )
        return;

    auto a = toRational(mnNumerator, mnDenominator);
    rational_ReduceInaccurate(a, nSignificantBits);
    mnNumerator = a.numerator();
    mnDenominator = a.denominator();
}

sal_Int32 Fraction::GetNumerator() const
{
    if ( !mbValid )
    {
        SAL_WARN( "tools.fraction", "'GetNumerator()' on invalid fraction" );
        return 0;
    }
    return mnNumerator;
}

sal_Int32 Fraction::GetDenominator() const
{
    if ( !mbValid )
    {
        SAL_WARN( "tools.fraction", "'GetDenominator()' on invalid fraction" );
        return -1;
    }
    return mnDenominator;
}

Fraction::operator sal_Int32() const
{
    if ( !mbValid )
    {
        SAL_WARN( "tools.fraction", "'operator sal_Int32()' on invalid fraction" );
        return 0;
    }
    return boost::rational_cast<sal_Int32>(toRational(mnNumerator, mnDenominator));
}

Fraction operator+( const Fraction& rVal1, const Fraction& rVal2 )
{
    Fraction aErg( rVal1 );
    aErg += rVal2;
    return aErg;
}

Fraction operator-( const Fraction& rVal1, const Fraction& rVal2 )
{
    Fraction aErg( rVal1 );
    aErg -= rVal2;
    return aErg;
}

Fraction operator*( const Fraction& rVal1, const Fraction& rVal2 )
{
    Fraction aErg( rVal1 );
    aErg *= rVal2;
    return aErg;
}

Fraction operator/( const Fraction& rVal1, const Fraction& rVal2 )
{
    Fraction aErg( rVal1 );
    aErg /= rVal2;
    return aErg;
}

bool operator !=( const Fraction& rVal1, const Fraction& rVal2 )
{
    return !(rVal1 == rVal2);
}

bool operator <=( const Fraction& rVal1, const Fraction& rVal2 )
{
    return !(rVal1 > rVal2);
}

bool operator >=( const Fraction& rVal1, const Fraction& rVal2 )
{
    return !(rVal1 < rVal2);
}

bool operator == ( const Fraction& rVal1, const Fraction& rVal2 )
{
    if ( !rVal1.mbValid || !rVal2.mbValid )
    {
        SAL_WARN( "tools.fraction", "'operator ==' with an invalid fraction" );
        return false;
    }

    return toRational(rVal1.mnNumerator, rVal1.mnDenominator) == toRational(rVal2.mnNumerator, rVal2.mnDenominator);
}

bool operator < ( const Fraction& rVal1, const Fraction& rVal2 )
{
    if ( !rVal1.mbValid || !rVal2.mbValid )
    {
        SAL_WARN( "tools.fraction", "'operator <' with an invalid fraction" );
        return false;
    }

    return toRational(rVal1.mnNumerator, rVal1.mnDenominator) < toRational(rVal2.mnNumerator, rVal2.mnDenominator);
}

bool operator > ( const Fraction& rVal1, const Fraction& rVal2 )
{
    if ( !rVal1.mbValid || !rVal2.mbValid )
    {
        SAL_WARN( "tools.fraction", "'operator >' with an invalid fraction" );
        return false;
    }

    return toRational(rVal1.mnNumerator, rVal1.mnDenominator) > toRational(rVal2.mnNumerator, rVal2.mnDenominator);
}

// If dVal > LONG_MAX or dVal < LONG_MIN, the rational throws a boost::bad_rational.
// Otherwise, dVal and denominator are multiplied by 10, until one of them
// is larger than (LONG_MAX / 10).
//
// NOTE: here we use 'sal_Int32' due that only values in sal_Int32 range are valid.
static boost::rational<sal_Int32> rational_FromDouble(double dVal)
{
    if ( dVal > std::numeric_limits<sal_Int32>::max() ||
         dVal < std::numeric_limits<sal_Int32>::min() ||
         std::isnan(dVal) )
        throw boost::bad_rational();

    const sal_Int32 nMAX = std::numeric_limits<sal_Int32>::max() / 10;
    sal_Int32 nDen = 1;
    while ( std::abs( dVal ) < nMAX && nDen < nMAX ) {
        dVal *= 10;
        nDen *= 10;
    }
    return boost::rational<sal_Int32>( sal_Int32(dVal), nDen );
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
static void rational_ReduceInaccurate(boost::rational<sal_Int32>& rRational, unsigned nSignificantBits)
{
    if ( !rRational )
        return;

    // http://www.boost.org/doc/libs/release/libs/rational/rational.html#Internal%20representation
    sal_Int32 nMul = rRational.numerator();
    if (nMul == std::numeric_limits<sal_Int32>::min())
    {
        // ofz#32973 Integer-overflow
        return;
    }
    const bool bNeg = nMul < 0;
    if (bNeg)
        nMul = -nMul;
    sal_Int32 nDiv = rRational.denominator();

    DBG_ASSERT(nSignificantBits<65, "More than 64 bit of significance is overkill!");

    // How much bits can we lose?
    const int nMulBitsToLose = std::max( ( impl_NumberOfBits( nMul ) - int( nSignificantBits ) ), 0 );
    const int nDivBitsToLose = std::max( ( impl_NumberOfBits( nDiv ) - int( nSignificantBits ) ), 0 );

    const int nToLose = std::min( nMulBitsToLose, nDivBitsToLose );

    // Remove the bits
    nMul >>= nToLose;
    nDiv >>= nToLose;

    if ( !nMul || !nDiv ) {
        // Return without reduction
        OSL_FAIL( "Oops, we reduced too much..." );
        return;
    }

    rRational.assign( bNeg ? -nMul : nMul, nDiv );
}

size_t Fraction::GetHashValue() const
{
    size_t hash = 0;
    o3tl::hash_combine( hash, mnNumerator );
    o3tl::hash_combine( hash, mnDenominator );
    o3tl::hash_combine( hash, mbValid );
    return hash;
}

Fraction Fraction::MakeFraction( tools::Long nN1, tools::Long nN2, tools::Long nD1, tools::Long nD2 )
{
    if( nD1 == 0 || nD2 == 0 ) //under these bad circumstances the following while loop will be endless
    {
        SAL_WARN("tools.fraction", "Invalid parameter for ImplMakeFraction");
        return Fraction( 1, 1 );
    }

    tools::Long i = 1;

    if ( nN1 < 0 ) { i = -i; nN1 = -nN1; }
    if ( nN2 < 0 ) { i = -i; nN2 = -nN2; }
    if ( nD1 < 0 ) { i = -i; nD1 = -nD1; }
    if ( nD2 < 0 ) { i = -i; nD2 = -nD2; }
    // all positive; i sign

    assert( nN1 >= std::numeric_limits<sal_Int32>::min() );
    assert( nN1 <= std::numeric_limits<sal_Int32>::max( ));
    assert( nD1 >= std::numeric_limits<sal_Int32>::min() );
    assert( nD1 <= std::numeric_limits<sal_Int32>::max( ));
    assert( nN2 >= std::numeric_limits<sal_Int32>::min() );
    assert( nN2 <= std::numeric_limits<sal_Int32>::max( ));
    assert( nD2 >= std::numeric_limits<sal_Int32>::min() );
    assert( nD2 <= std::numeric_limits<sal_Int32>::max( ));

    boost::rational<sal_Int32> a = toRational(i*nN1, nD1);
    boost::rational<sal_Int32> b = toRational(nN2, nD2);
    bool bFail = checked_multiply_by(a, b);

    while ( bFail ) {
        if ( nN1 > nN2 )
            nN1 = (nN1 + 1) / 2;
        else
            nN2 = (nN2 + 1) / 2;
        if ( nD1 > nD2 )
            nD1 = (nD1 + 1) / 2;
        else
            nD2 = (nD2 + 1) / 2;

        a = toRational(i*nN1, nD1);
        b = toRational(nN2, nD2);
        bFail = checked_multiply_by(a, b);
    }

    return Fraction(a.numerator(), a.denominator());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
