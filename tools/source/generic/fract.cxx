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
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>

#include <limits.h>
#include <algorithm>
#include <cmath>

#include <boost/rational.hpp>

template< typename T >
static boost::rational< T > rational_FromDouble( double value );

template< typename T >
static void rational_ReduceInaccurate( boost::rational<T>& rRational, size_t nSignificantBits );

struct Fraction::Impl
{
    bool                          okay;
    boost::rational< sal_Int64 >  value;

    Impl()
        : okay(false)
    {
    }
    Impl(const Impl&) = delete;
    Impl& operator=(const Impl&) = delete;
};

Fraction::Fraction() : mpImpl(new Impl)
{
    mpImpl->okay = true;
}

Fraction::Fraction( const Fraction& rFrac ) : mpImpl(new Impl)
{
    mpImpl->okay = rFrac.mpImpl->okay;
    if ( mpImpl->okay )
        mpImpl->value.assign( rFrac.mpImpl->value.numerator(), rFrac.mpImpl->value.denominator() );
}

Fraction::Fraction( Fraction&& rFrac ) : mpImpl(std::move(rFrac.mpImpl))
{
}

// Initialized by setting nNum as nominator and nDen as denominator
// Negative values in the denominator are invalid and cause the
// inversion of both nominator and denominator signs
// in order to return the correct value.
Fraction::Fraction( sal_Int32 nNum, sal_Int32 nDen ) : mpImpl(new Impl)
{
    if ( nDen == 0 )
    {
        mpImpl->okay = false;
        SAL_WARN( "tools.fraction", "'Fraction(" << nNum << ",0)' created fraction with zero denominator" );
        return;
    }
    mpImpl->value.assign( nNum, nDen);
    mpImpl->okay = true;
}

Fraction::Fraction( double dVal ) : mpImpl(new Impl)
{
    try {
        mpImpl->value = rational_FromDouble< sal_Int64 >( dVal );
    } catch ( const boost::bad_rational & )
    {
        // outside of range for underlying integer sal_Int64 type
        mpImpl->okay = false;
        SAL_WARN( "tools.fraction", "'Fraction(" << dVal << ")' can't create fraction" );
        return;
    }

    mpImpl->okay = true;

    if ( HasOutsideOf32bitValue() )
    {
        // outside of range for 32-bit integer type
        mpImpl->okay = false;
        SAL_WARN( "tools.fraction", "'Fraction(" << dVal << ")' created outside of 32-bit fraction" );
    }
}

Fraction::~Fraction()
{
}

bool Fraction::HasOutsideOf32bitValue()
{
    if (  mpImpl->value.numerator()  < std::numeric_limits< sal_Int32 >::min() ||
          mpImpl->value.numerator()  > std::numeric_limits< sal_Int32 >::max() )
    {
        SAL_INFO( "tools.fraction",
                  "numerator " << mpImpl->value.numerator() << " / " << "denominator " << mpImpl->value.denominator()
                      << " ~ numerator is outside of 32-bit" );
        return true;
    }
    if ( mpImpl->value.denominator() < std::numeric_limits< sal_Int32 >::min() ||
         mpImpl->value.denominator() > std::numeric_limits< sal_Int32 >::max() )
    {
        SAL_INFO( "tools.fraction",
                  "numerator " << mpImpl->value.numerator() << " / " << "denominator " << mpImpl->value.denominator()
                      << " ~ denominator is outside of 32-bit" );
        return true;
    }

    return false;
}

Fraction::operator double() const
{
    if ( ! mpImpl->okay )
    {
        SAL_WARN( "tools.fraction", "'operator double' on strange fraction" );
    }

    return boost::rational_cast< double >( mpImpl->value );
}

Fraction& Fraction::operator += ( const Fraction& rVal )
{
    if ( !rVal.mpImpl->okay )
        mpImpl->okay = false;

    if ( !mpImpl->okay )
    {
        SAL_INFO( "tools.fraction", "'operator +=' with too precise fraction" );
        return *this;
    }

    mpImpl->value += rVal.mpImpl->value;

    if ( HasOutsideOf32bitValue() )
    {
        mpImpl->okay = false;
        SAL_INFO( "tools.fraction", "'operator +=' with outside of 32-bit fraction" );
    }

    return *this;
}

Fraction& Fraction::operator -= ( const Fraction& rVal )
{
    if ( !rVal.mpImpl->okay )
        mpImpl->okay = false;

    if ( !mpImpl->okay )
    {
        SAL_INFO( "tools.fraction", "'operator -=' with too precise fraction" );
        return *this;
    }

    mpImpl->value -= rVal.mpImpl->value;

    if ( HasOutsideOf32bitValue() )
    {
        mpImpl->okay = false;
        SAL_INFO( "tools.fraction", "'operator -=' with outside of 32-bit fraction" );
    }

    return *this;
}

Fraction& Fraction::operator *= ( const Fraction& rVal )
{
    if ( !rVal.mpImpl->okay )
        mpImpl->okay = false;

    if ( !mpImpl->okay )
    {
        SAL_INFO( "tools.fraction", "'operator *=' with too precise fraction" );
        return *this;
    }

    mpImpl->value *= rVal.mpImpl->value;

    if ( HasOutsideOf32bitValue() )
    {
        mpImpl->okay = false;
        SAL_INFO( "tools.fraction", "'operator *=' with outside of 32-bit fraction" );
    }

    return *this;
}

Fraction& Fraction::operator /= ( const Fraction& rVal )
{
    if ( !rVal.mpImpl->okay )
        mpImpl->okay = false;

    if ( !mpImpl->okay )
    {
        SAL_INFO( "tools.fraction", "'operator /=' with too precise fraction" );
        return *this;
    }

    mpImpl->value /= rVal.mpImpl->value;

    if ( HasOutsideOf32bitValue() )
    {
        mpImpl->okay = false;
        SAL_INFO( "tools.fraction", "'operator /=' with outside of 32-bit fraction" );
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
void Fraction::ReduceInaccurate( size_t nSignificantBits )
{
    if ( !mpImpl->value.numerator() )
        return;

    rational_ReduceInaccurate( mpImpl->value, nSignificantBits );
}

sal_Int64 Fraction::GetNumerator() const
{
    return mpImpl->value.numerator();
}

sal_Int64 Fraction::GetDenominator() const
{
    return mpImpl->value.denominator();
}

Fraction& Fraction::operator = ( const Fraction& rFrac )
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

bool Fraction::IsOkay() const
{
    return mpImpl->okay;
}

Fraction operator + ( const Fraction& rVal1, const Fraction& rVal2 )
{
    Fraction aErg( rVal1 );
    aErg += rVal2;
    return aErg;
}

Fraction operator - ( const Fraction& rVal1, const Fraction& rVal2 )
{
    Fraction aErg( rVal1 );
    aErg -= rVal2;
    return aErg;
}

Fraction operator * ( const Fraction& rVal1, const Fraction& rVal2 )
{
    Fraction aErg( rVal1 );
    aErg *= rVal2;
    return aErg;
}

Fraction operator / ( const Fraction& rVal1, const Fraction& rVal2 )
{
    Fraction aErg( rVal1 );
    aErg /= rVal2;
    return aErg;
}

bool operator != ( const Fraction& rVal1, const Fraction& rVal2 )
{
    return !(rVal1 == rVal2);
}

bool operator <= ( const Fraction& rVal1, const Fraction& rVal2 )
{
    return !(rVal1 > rVal2);
}

bool operator >= ( const Fraction& rVal1, const Fraction& rVal2 )
{
    return !(rVal1 < rVal2);
}

bool operator == ( const Fraction& rVal1, const Fraction& rVal2 )
{
    if ( !rVal1.mpImpl->okay || !rVal2.mpImpl->okay )
    {
        SAL_INFO( "tools.fraction", "'operator ==' with too precise fraction" );
        return false;
    }

    return rVal1.mpImpl->value == rVal2.mpImpl->value;
}

bool operator < ( const Fraction& rVal1, const Fraction& rVal2 )
{
    if ( !rVal1.mpImpl->okay || !rVal2.mpImpl->okay )
    {
        SAL_INFO( "tools.fraction", "'operator <' with too precise fraction" );
        return false;
    }

    return rVal1.mpImpl->value < rVal2.mpImpl->value;
}

bool operator > ( const Fraction& rVal1, const Fraction& rVal2 )
{
    if ( !rVal1.mpImpl->okay || !rVal2.mpImpl->okay )
    {
        SAL_INFO( "tools.fraction", "'operator >' with too precise fraction" );
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
        SAL_WARN( "tools.fraction", "'ReadFraction()' read fraction with zero or negative denominator" );
        rFract.mpImpl->okay = false;
    }
    else
    {
        rFract.mpImpl->value.assign( num, den );
        rFract.mpImpl->okay = true;
    }
    return rIStream;
}

SvStream& WriteFraction( SvStream& rOStream, const Fraction& rFract )
{
    if ( !rFract.mpImpl->okay )
    {
        SAL_WARN( "tools.fraction", "'WriteFraction()' with too precise fraction" );
        rOStream.WriteInt32( 0 );
        rOStream.WriteInt32( -1 );
    } else {
#if OSL_DEBUG_LEVEL > 0
        // can only write 32 bits, so check that no data is lost
        boost::rational< sal_Int64 > copy( rFract.mpImpl->value );
        rational_ReduceInaccurate( copy, 32 );
        assert( copy == rFract.mpImpl->value && "data loss in WriteFraction" );
#endif
        rOStream.WriteInt32( rFract.mpImpl->value.numerator() );
        rOStream.WriteInt32( rFract.mpImpl->value.denominator() );
    }
    return rOStream;
}

template< typename T >
static boost::rational< T > rational_FromDouble( double value )
{
    if ( value > std::numeric_limits< T >::max() ||
            value < std::numeric_limits< T >::min() )
        throw boost::bad_rational( "value is outside of range for underlying integer type" );

    // multiply value and denominator by 10 until one of them is larger than one tenth of max32
    const sal_Int32 tenthOfMax32 = std::numeric_limits< sal_Int32 >::max() / 10;
    sal_Int32 denominator = 1;
    while ( std::abs( value ) < tenthOfMax32 && denominator < tenthOfMax32 ) {
        value *= 10;
        denominator *= 10;
    }

    return boost::rational< T >( T(value), T(denominator) );
}

// Similar to clz_table that can be googled
const char nbits_table[32] =
{
    32,  1, 23,  2, 29, 24, 14,  3,
    30, 27, 25, 18, 20, 15, 10,  4,
    31, 22, 28, 13, 26, 17, 19,  9,
    21, 12, 16,  8, 11,  7,  6,  5
};

static int impl_NumberOfBits( unsigned long nNum )
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
template< typename T >
static void rational_ReduceInaccurate( boost::rational< T > & rRational, size_t nSignificantBits )
{
    if ( !rRational )
        return;

    // http://www.boost.org/doc/libs/release/libs/rational/rational.html#Internal%20representation
    const bool bNeg = ( rRational.numerator() < 0 );
    T nMul = bNeg ? -rRational.numerator() : rRational.numerator();
    T nDiv = rRational.denominator();

    DBG_ASSERT( nSignificantBits < 65, "More than 64 bit of significance is overkill" );

    // How much bits can we lose?
    const size_t nMulBitsToLose = std::max( ( impl_NumberOfBits( nMul ) - signed( nSignificantBits ) ), 0 );
    const size_t nDivBitsToLose = std::max( ( impl_NumberOfBits( nDiv ) - signed( nSignificantBits ) ), 0 );

    const size_t nToLose = std::min( nMulBitsToLose, nDivBitsToLose );

    // Remove the bits
    nMul >>= nToLose;
    nDiv >>= nToLose;

    if ( nMul == 0 || nDiv == 0 ) {
        // return without reduction
        OSL_FAIL( "Oops, reduced too much..." );
        return;
    }

    rRational.assign( bNeg ? -T( nMul ) : T( nMul ), nDiv );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
