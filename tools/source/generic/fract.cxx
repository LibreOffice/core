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

#include <algorithm>
#include <cmath>

#include <limits.h>
#include <rtl/ustring.hxx>
#include <tools/debug.hxx>
#include <tools/fract.hxx>
#include <tools/lineend.hxx>
#include <tools/stream.hxx>

template<typename T>
static boost::rational<T> rational_FromDouble(double dVal);

template<typename T>
static void rational_ReduceInaccurate(boost::rational<T>& rRational, unsigned nSignificantBits);

// Initialized by setting nNum as nominator and nDen as denominator
// Negative values in the denominator are invalid and cause the
// inversion of both nominator and denominator signs
// in order to return the correct value.
Fraction::Fraction( long nNum, long nDen )
{
    if ( nDen == 0 ) {
        valid = false;
        SAL_WARN( "tools.fraction", "'Fraction(" + std::to_string(nNum) + ",0)' invalid fraction created" );
        return;
    }
    value.assign( nNum, nDen);
    valid = true;
}

Fraction::Fraction( double dVal )
{
    try {
        value = rational_FromDouble<sal_Int64>( dVal );
        if ( HasOverflowValue() )
            throw boost::bad_rational();
        valid = true;
    } catch(const boost::bad_rational& unused) {
        valid = false;
        SAL_WARN( "tools.fraction", "'Fraction(" + std::to_string(dVal) + ")' invalid fraction created" );
    }
}

bool Fraction::HasOverflowValue()
{
    return value.numerator() < std::numeric_limits<long>::min() ||
        value.numerator() > std::numeric_limits<long>::max() ||
        value.denominator() < std::numeric_limits<long>::min() ||
        value.denominator() > std::numeric_limits<long>::max();
}

Fraction::operator double() const
{
    if ( !valid ) {
        SAL_WARN( "tools.fraction", "'double()' on invalid fraction" );
        return 0.0;
    }

    return boost::rational_cast<double>(value);
}

// This methods first validates both values.
// If one of the arguments is invalid, the whole operation is invalid.
// After computation detect if result overflows a long value
// which cause the operation to be marked as invalid
Fraction& Fraction::operator += ( const Fraction& rVal )
{
    if ( !rVal.valid )
        valid = false;
    if ( !valid ) {
        SAL_WARN( "tools.fraction", "'operator +=' with invalid fraction" );
        return *this;
    }

    value += rVal.value;

    if ( HasOverflowValue() ) {
        valid = false;
        SAL_WARN( "tools.fraction", "'operator +=' detected overflow" );
    }

    return *this;
}

Fraction& Fraction::operator -= ( const Fraction& rVal )
{
    if ( !rVal.valid )
        valid = false;
    if ( !valid ) {
        SAL_WARN( "tools.fraction", "'operator -=' with invalid fraction" );
        return *this;
    }

    value -= rVal.value;

    if ( HasOverflowValue() ) {
        valid = false;
        SAL_WARN( "tools.fraction", "'operator -=' detected overflow" );
    }

    return *this;
}

Fraction& Fraction::operator *= ( const Fraction& rVal )
{
    if ( !rVal.valid )
        valid = false;
    if ( !valid ) {
        SAL_WARN( "tools.fraction", "'operator *=' with invalid fraction" );
        return *this;
    }

    value *= rVal.value;

    if ( HasOverflowValue() ) {
        valid = false;
        SAL_WARN( "tools.fraction", "'operator *=' detected overflow" );
    }

    return *this;
}

Fraction& Fraction::operator /= ( const Fraction& rVal )
{
    if ( !rVal.valid )
        valid = false;
    if ( !valid ) {
        SAL_WARN( "tools.fraction", "'operator /=' with invalid fraction" );
        return *this;
    }

    value /= rVal.value;

    if ( HasOverflowValue() ) {
        valid = false;
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
    if ( !valid ) {
        SAL_WARN( "tools.fraction", "'ReduceInaccurate' on invalid fraction" );
        return;
    }
    if ( !value.numerator() )
        return;

    rational_ReduceInaccurate(value, nSignificantBits);
}

bool operator == ( const Fraction& rVal1, const Fraction& rVal2 )
{
    if ( !rVal1.valid || !rVal2.valid ) {
        SAL_WARN( "tools.fraction", "'operator ==' with an invalid fraction" );
        return false;
    }

    return rVal1.value == rVal2.value;
}

bool operator < ( const Fraction& rVal1, const Fraction& rVal2 )
{
    if ( !rVal1.valid || !rVal2.valid ) {
        SAL_WARN( "tools.fraction", "'operator <' with an invalid fraction" );
        return false;
    }

    return rVal1.value < rVal2.value;
}

bool operator > ( const Fraction& rVal1, const Fraction& rVal2 )
{
    if ( !rVal1.valid || !rVal2.valid ) {
        SAL_WARN( "tools.fraction", "'operator >' with an invalid fraction" );
        return false;
    }

    return rVal1.value > rVal2.value;
}

SvStream& ReadFraction( SvStream& rIStream, Fraction& rFract )
{
    sal_Int32 num(0), den(0);
    rIStream.ReadInt32( num );
    rIStream.ReadInt32( den );
    if ( den <= 0 ) {
        SAL_WARN( "tools.fraction", "'ReadFraction()' read an invalid fraction" );
        rFract.valid = false;
    } else {
        rFract.value.assign( num, den );
        rFract.valid = true;
    }
    return rIStream;
}

SvStream& WriteFraction( SvStream& rOStream, const Fraction& rFract )
{
    if ( !rFract.valid ) {
        SAL_WARN( "tools.fraction", "'WriteFraction()' write an invalid fraction" );
        rOStream.WriteInt32( 0 );
        rOStream.WriteInt32( -1 );
    } else {
        rOStream.WriteInt32( rFract.value.numerator() );
        rOStream.WriteInt32( rFract.value.denominator() );
    }
    return rOStream;
}

// If dVal > LONG_MAX or dVal < LONG_MIN, the rational throws a boost::bad_rational.
// Otherwise, dVal and denominator are multiplied by 10, until one of them
// is larger than (LONG_MAX / 10).
//
// NOTE: here we use 'long' due that only values in long range are valid.
template<typename T>
static boost::rational<T> rational_FromDouble(double dVal)
{
    if ( dVal > std::numeric_limits<long>::max() ||
            dVal < std::numeric_limits<long>::min() )
        throw boost::bad_rational();

    const long nMAX = std::numeric_limits<long>::max() / 10;
    long nDen = 1;
    while ( std::abs( dVal ) < nMAX && nDen < nMAX ) {
        dVal *= 10;
        nDen *= 10;
    }
    return boost::rational<T>( long(dVal), nDen );
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
template<typename T>
static void rational_ReduceInaccurate(boost::rational<T>& rRational, unsigned nSignificantBits)
{
    if ( !rRational )
        return;

    // http://www.boost.org/doc/libs/release/libs/rational/rational.html#Internal%20representation
    const bool bNeg = ( rRational.numerator() < 0 );
    T nMul = bNeg? -rRational.numerator(): rRational.numerator();
    T nDiv = rRational.denominator();

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

    rRational.assign( bNeg? -T( nMul ): T( nMul ), nDiv );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
