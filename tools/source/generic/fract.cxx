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
#include <tools/lineend.hxx>
#include <tools/stream.hxx>
#include <o3tl/safeint.hxx>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>

#include <limits.h>
#include <algorithm>
#include <cmath>

#include <boost/math/common_factor_rt.hpp>
#include <boost/rational.hpp>

static boost::rational<sal_Int32> rational_FromDouble(double dVal);

static void rational_ReduceInaccurate(boost::rational<sal_Int32>& rRational, unsigned nSignificantBits);

struct Fraction::Impl
{
    bool                        valid;
    boost::rational<sal_Int32>  value;

    Impl()
        : valid(false)
    {
    }
    Impl(const Impl&) = delete;
    Impl& operator=(const Impl&) = delete;
};

Fraction::Fraction() : mpImpl(new Impl)
{
    mpImpl->valid = true;
}

Fraction::Fraction( const Fraction& rFrac ) : mpImpl(new Impl)
{
    mpImpl->valid = rFrac.mpImpl->valid;
    if (mpImpl->valid)
        mpImpl->value.assign( rFrac.mpImpl->value.numerator(), rFrac.mpImpl->value.denominator() );
}

Fraction::Fraction( Fraction&& rFrac ) : mpImpl(std::move(rFrac.mpImpl))
{
}

// Initialized by setting nNum as nominator and nDen as denominator
// Negative values in the denominator are invalid and cause the
// inversion of both nominator and denominator signs
// in order to return the correct value.
Fraction::Fraction( sal_Int64 nNum, sal_Int64 nDen ) : mpImpl(new Impl)
{
    assert( nNum >= std::numeric_limits<sal_Int32>::min() );
    assert( nNum <= std::numeric_limits<sal_Int32>::max( ));
    assert( nDen >= std::numeric_limits<sal_Int32>::min() );
    assert( nDen <= std::numeric_limits<sal_Int32>::max( ));
    if ( nDen == 0 )
    {
        mpImpl->valid = false;
        SAL_WARN( "tools.fraction", "'Fraction(" << nNum << ",0)' invalid fraction created" );
        return;
    }
    mpImpl->value.assign( nNum, nDen);
    mpImpl->valid = true;
}

/**
 * only here to prevent passing of NaN
 */
Fraction::Fraction( double nNum, double nDen ) : mpImpl(new Impl)
{
    assert( !std::isnan(nNum) );
    assert( !std::isnan(nDen) );
    assert( nNum >= std::numeric_limits<sal_Int32>::min() );
    assert( nNum <= std::numeric_limits<sal_Int32>::max( ));
    assert( nDen >= std::numeric_limits<sal_Int32>::min() );
    assert( nDen <= std::numeric_limits<sal_Int32>::max( ));
    if ( nDen == 0 )
    {
        mpImpl->valid = false;
        SAL_WARN( "tools.fraction", "'Fraction(" << nNum << ",0)' invalid fraction created" );
        return;
    }
    mpImpl->value.assign( sal_Int64(nNum), sal_Int64(nDen));
    mpImpl->valid = true;
}

Fraction::Fraction( double dVal ) : mpImpl(new Impl)
{
    try
    {
        mpImpl->value = rational_FromDouble( dVal );
        if ( HasOverflowValue() )
            throw boost::bad_rational();
        mpImpl->valid = true;
    }
    catch (const boost::bad_rational&)
    {
        mpImpl->valid = false;
        SAL_WARN( "tools.fraction", "'Fraction(" << dVal << ")' invalid fraction created" );
    }
}

Fraction::~Fraction()
{
}

bool Fraction::HasOverflowValue()
{
    //coverity[result_independent_of_operands]
    return mpImpl->value.numerator() < std::numeric_limits<sal_Int32>::min() ||
        mpImpl->value.numerator() > std::numeric_limits<sal_Int32>::max() ||
        mpImpl->value.denominator() < std::numeric_limits<sal_Int32>::min() ||
        mpImpl->value.denominator() > std::numeric_limits<sal_Int32>::max();
}

Fraction::operator double() const
{
    if (!mpImpl->valid)
    {
        SAL_WARN( "tools.fraction", "'double()' on invalid fraction" );
        return 0.0;
    }

    return boost::rational_cast<double>(mpImpl->value);
}

// This methods first validates both values.
// If one of the arguments is invalid, the whole operation is invalid.
// After computation detect if result overflows a sal_Int32 value
// which cause the operation to be marked as invalid
Fraction& Fraction::operator += ( const Fraction& rVal )
{
    if ( !rVal.mpImpl->valid )
        mpImpl->valid = false;

    if ( !mpImpl->valid )
    {
        SAL_WARN( "tools.fraction", "'operator +=' with invalid fraction" );
        return *this;
    }

    mpImpl->value += rVal.mpImpl->value;

    if ( HasOverflowValue() )
    {
        mpImpl->valid = false;
        SAL_WARN( "tools.fraction", "'operator +=' detected overflow" );
    }

    return *this;
}

Fraction& Fraction::operator -= ( const Fraction& rVal )
{
    if ( !rVal.mpImpl->valid )
        mpImpl->valid = false;

    if ( !mpImpl->valid )
    {
        SAL_WARN( "tools.fraction", "'operator -=' with invalid fraction" );
        return *this;
    }

    mpImpl->value -= rVal.mpImpl->value;

    if ( HasOverflowValue() )
    {
        mpImpl->valid = false;
        SAL_WARN( "tools.fraction", "'operator -=' detected overflow" );
    }

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
        T gcd1 = boost::math::gcd(i.numerator(), den);
        T gcd2 = boost::math::gcd(num, i.denominator());

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
    if ( !rVal.mpImpl->valid )
        mpImpl->valid = false;

    if ( !mpImpl->valid )
    {
        SAL_WARN( "tools.fraction", "'operator *=' with invalid fraction" );
        return *this;
    }

    bool bFail = checked_multiply_by(mpImpl->value, rVal.mpImpl->value);

    if (bFail || HasOverflowValue())
    {
        mpImpl->valid = false;
    }

    return *this;
}

Fraction& Fraction::operator /= ( const Fraction& rVal )
{
    if ( !rVal.mpImpl->valid )
        mpImpl->valid = false;

    if ( !mpImpl->valid )
    {
        SAL_WARN( "tools.fraction", "'operator /=' with invalid fraction" );
        return *this;
    }

    mpImpl->value /= rVal.mpImpl->value;

    if ( HasOverflowValue() )
    {
        mpImpl->valid = false;
        SAL_WARN( "tools.fraction", "'operator /=' detected overflow" );
    }

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
    if ( !mpImpl->valid )
    {
        SAL_WARN( "tools.fraction", "'ReduceInaccurate' on invalid fraction" );
        return;
    }

    if ( !mpImpl->value.numerator() )
        return;

    rational_ReduceInaccurate(mpImpl->value, nSignificantBits);
}

sal_Int32 Fraction::GetNumerator() const
{
    if ( !mpImpl->valid )
    {
        SAL_WARN( "tools.fraction", "'GetNumerator()' on invalid fraction" );
        return 0;
    }
    return mpImpl->value.numerator();
}

sal_Int32 Fraction::GetDenominator() const
{
    if ( !mpImpl->valid )
    {
        SAL_WARN( "tools.fraction", "'GetDenominator()' on invalid fraction" );
        return -1;
    }
    return mpImpl->value.denominator();
}

Fraction& Fraction::operator=( const Fraction& rFrac )
{
    if (this == &rFrac)
        return *this;

    Fraction tmp(rFrac);
    std::swap(mpImpl, tmp.mpImpl);
    return *this;
}

Fraction& Fraction::operator=( Fraction&& rFrac )
{
    mpImpl = std::move(rFrac.mpImpl);
    return *this;
}

bool Fraction::IsValid() const
{
    return mpImpl->valid;
}

Fraction::operator sal_Int32() const
{
    if ( !mpImpl->valid )
    {
        SAL_WARN( "tools.fraction", "'operator sal_Int32()' on invalid fraction" );
        return 0;
    }
    return boost::rational_cast<sal_Int32>(mpImpl->value);
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
    if ( !rVal1.mpImpl->valid || !rVal2.mpImpl->valid )
    {
        SAL_WARN( "tools.fraction", "'operator ==' with an invalid fraction" );
        return false;
    }

    return rVal1.mpImpl->value == rVal2.mpImpl->value;
}

bool operator < ( const Fraction& rVal1, const Fraction& rVal2 )
{
    if ( !rVal1.mpImpl->valid || !rVal2.mpImpl->valid )
    {
        SAL_WARN( "tools.fraction", "'operator <' with an invalid fraction" );
        return false;
    }

    return rVal1.mpImpl->value < rVal2.mpImpl->value;
}

bool operator > ( const Fraction& rVal1, const Fraction& rVal2 )
{
    if ( !rVal1.mpImpl->valid || !rVal2.mpImpl->valid )
    {
        SAL_WARN( "tools.fraction", "'operator >' with an invalid fraction" );
        return false;
    }

    return rVal1.mpImpl->value > rVal2.mpImpl->value;
}

SvStream& ReadFraction( SvStream& rIStream, Fraction const & rFract )
{
    sal_Int32 num(0), den(0);
    rIStream.ReadInt32( num );
    rIStream.ReadInt32( den );
    if ( den <= 0 )
    {
        SAL_WARN( "tools.fraction", "'ReadFraction()' read an invalid fraction" );
        rFract.mpImpl->valid = false;
    }
    else
    {
        rFract.mpImpl->value.assign( num, den );
        rFract.mpImpl->valid = true;
    }
    return rIStream;
}

SvStream& WriteFraction( SvStream& rOStream, const Fraction& rFract )
{
    if ( !rFract.mpImpl->valid )
    {
        SAL_WARN( "tools.fraction", "'WriteFraction()' write an invalid fraction" );
        rOStream.WriteInt32( 0 );
        rOStream.WriteInt32( -1 );
    } else {
        rOStream.WriteInt32( rFract.mpImpl->value.numerator() );
        rOStream.WriteInt32( rFract.mpImpl->value.denominator() );
    }
    return rOStream;
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

// Similar to clz_table that can be googled
const char nbits_table[32] =
{
    32,  1, 23,  2, 29, 24, 14,  3,
    30, 27, 25, 18, 20, 15, 10,  4,
    31, 22, 28, 13, 26, 17, 19,  9,
    21, 12, 16,  8, 11,  7,  6,  5
};

static int impl_NumberOfBits( sal_uInt32 nNum )
{
    // http://en.wikipedia.org/wiki/De_Bruijn_sequence
    // background paper: Using de Bruijn Sequences to Index a 1 in a
    // Computer Word (1998) Charles E. Leiserson,
    // Harald Prokop, Keith H. Randall
    // (e.g. http://citeseer.ist.psu.edu/leiserson98using.html)
    const sal_uInt32 nDeBruijn = 0x7DCD629;

    if ( nNum == 0 )
        return 0;

    // Get it to form like 0000001111111111b
    nNum |= ( nNum >>  1 );
    nNum |= ( nNum >>  2 );
    nNum |= ( nNum >>  4 );
    nNum |= ( nNum >>  8 );
    nNum |= ( nNum >> 16 );

    sal_uInt32 nNumber;
    int nBonus;

    nNumber = nNum;
    nBonus = 0;

    // De facto shift left of nDeBruijn using multiplication (nNumber
    // is all ones from topmost bit, thus nDeBruijn + (nDeBruijn *
    // nNumber) => nDeBruijn * (nNumber+1) clears all those bits to
    // zero, sets the next bit to one, and thus effectively shift-left
    // nDeBruijn by lg2(nNumber+1). This generates a distinct 5bit
    // sequence in the msb for each distinct position of the last
    // leading 0 bit - that's the property of a de Bruijn number.
    nNumber = nDeBruijn + ( nDeBruijn * nNumber );

    // 5-bit window indexes the result
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
