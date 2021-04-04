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
#include <tools/stream.hxx>
#include <o3tl/safeint.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>

#include <algorithm>
#include <cmath>

#include <boost/version.hpp>
#if BOOST_VERSION >= 106700
#include <boost/integer/common_factor_rt.hpp>
#else
#include <boost/math/common_factor_rt.hpp>
#endif
#include <boost/rational.hpp>

#ifdef _MSC_VER
#include <intrin.h>
#endif

static boost::rational<sal_Int32> rational_FromDouble(double dVal);

static void rational_ReduceInaccurate(boost::rational<sal_Int32>& rRational, unsigned nSignificantBits);

static boost::rational<sal_Int32> toRational(sal_Int32 n, sal_Int32 d)
{
    return boost::rational<sal_Int32>(n, d);
}

// Initialized by setting nNum as nominator and nDen as denominator
// Negative values in the denominator are invalid and cause the
// inversion of both nominator and denominator signs
// in order to return the correct value.
Fraction::Fraction( sal_Int64 nNum, sal_Int64 nDen ) : mnNumerator(nNum), mnDenominator(nDen)
{
    assert( nNum >= std::numeric_limits<sal_Int32>::min() );
    assert( nNum <= std::numeric_limits<sal_Int32>::max( ));
    assert( nDen >= std::numeric_limits<sal_Int32>::min() );
    assert( nDen <= std::numeric_limits<sal_Int32>::max( ));
    if ( nDen == 0 )
    {
        mbValid = false;
        SAL_WARN( "tools.fraction", "'Fraction(" << nNum << ",0)' invalid fraction created" );
        return;
    }
    if (nDen == -1 && nNum == std::numeric_limits<sal_Int32>::min())
    {
        mbValid = false;
        SAL_WARN("tools.fraction", "'Fraction(" << nNum << "," << nDen << ")' invalid fraction created");
        return;
    }
}

/**
 * only here to prevent passing of NaN
 */
Fraction::Fraction( double nNum, double nDen ) : mnNumerator(sal_Int64(nNum)), mnDenominator(sal_Int64(nDen))
{
    assert( !std::isnan(nNum) );
    assert( !std::isnan(nDen) );
    assert( nNum >= std::numeric_limits<sal_Int32>::min() );
    assert( nNum <= std::numeric_limits<sal_Int32>::max( ));
    assert( nDen >= std::numeric_limits<sal_Int32>::min() );
    assert( nDen <= std::numeric_limits<sal_Int32>::max( ));
    if ( nDen == 0 )
    {
        mbValid = false;
        SAL_WARN( "tools.fraction", "'Fraction(" << nNum << ",0)' invalid fraction created" );
        return;
    }
}

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

    // https://github.com/boostorg/boost/issues/335 when these are std::numeric_limits<sal_Int32>::min
    if (mnNumerator == mnDenominator)
        return 1.0;

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
    template<typename T> bool checked_multiply_by(boost::rational<T>& i, const boost::rational<T>& r)
    {
        // Protect against self-modification
        T num = r.numerator();
        T den = r.denominator();

        // Avoid overflow and preserve normalization
#if BOOST_VERSION >= 106700
        T gcd1 = boost::integer::gcd(i.numerator(), den);
        T gcd2 = boost::integer::gcd(num, i.denominator());
#else
        T gcd1 = boost::math::gcd(i.numerator(), den);
        T gcd2 = boost::math::gcd(num, i.denominator());
#endif

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
// Otherwise, dVal and denominator are multiplied by 8, until one of them
// is larger than (LONG_MAX / 8).
//
// NOTE: here we use 'sal_Int32' due that only values in sal_Int32 range are valid.
static boost::rational<sal_Int32> rational_FromDouble(double dVal)
{
    if ( dVal > std::numeric_limits<sal_Int32>::max() ||
         dVal < std::numeric_limits<sal_Int32>::min() ||
         std::isnan(dVal) )
        throw boost::bad_rational();

    const sal_Int32 nMAX = std::numeric_limits<sal_Int32>::max() / 8;
    sal_Int32 nDen = 1;
    while ( std::abs( dVal ) < nMAX && nDen < nMAX ) {
        dVal *= 8;
        nDen *= 8;
    }
    return boost::rational<sal_Int32>( sal_Int32(dVal), nDen );
}

/**
 * Find the number of bits required to represent this number, using the CLZ intrinsic
 */
static int impl_NumberOfBits( sal_uInt32 nNum )
{
    if (nNum == 0)
        return 0;
#ifdef _MSC_VER
    unsigned long r = 0;
    _BitScanReverse(&r, nNum);
    return r + 1;
#else
    return 32 - __builtin_clz(nNum);
#endif
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
    const bool bNeg = ( rRational.numerator() < 0 );
    sal_Int32 nMul = bNeg? -rRational.numerator(): rRational.numerator();
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
