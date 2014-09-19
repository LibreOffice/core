/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <tools/debug.hxx>
#include <tools/rational.hxx>
#include <tools/stream.hxx>

// If dVal > LONG_MAX or dVal < LONG_MIN, the rational throws a boost::bad_rational.
// Otherwise, dVal and denominator are multiplied with 10, until one of them
// is larger than (LONG_MAX / 10).
boost::rational<long> rational_FromDouble(double dVal)
{
    long nDen = 1;
    long nMAX = LONG_MAX / 10;

    if ( dVal > LONG_MAX || dVal < LONG_MIN )
    {
        throw boost::bad_rational();
    }

    while ( std::abs( (long)dVal ) < nMAX && nDen < nMAX )
    {
        dVal *= 10;
        nDen *= 10;
    }
    return boost::rational<long>((long) dVal, nDen);
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
void rational_ReduceInaccurate(boost::rational<long>& rRational, unsigned nSignificantBits)
{
    if ( !rRational.numerator() || !rRational.denominator() )
        return;

    // Count with unsigned longs only
    // http://www.boost.org/doc/libs/release/libs/rational/rational.html#Internal%20representation
    const bool bNeg = ( rRational.numerator() < 0 );
    unsigned long nMul = (unsigned long)( bNeg? -rRational.numerator(): rRational.numerator() );
    unsigned long nDiv = (unsigned long)( rRational.denominator() );

    DBG_ASSERT(nSignificantBits<65, "More than 64 bit of significance is overkill!");

    // How much bits can we lose?
    const int nMulBitsToLose = std::max( ( impl_NumberOfBits( nMul ) - int( nSignificantBits ) ), 0 );
    const int nDivBitsToLose = std::max( ( impl_NumberOfBits( nDiv ) - int( nSignificantBits ) ), 0 );

    const int nToLose = std::min( nMulBitsToLose, nDivBitsToLose );

    // Remove the bits
    nMul >>= nToLose;
    nDiv >>= nToLose;

    if ( !nMul || !nDiv )
    {
        // Return without reduction
        OSL_FAIL( "Oops, we reduced too much..." );
        return;
    }

    rRational.assign( bNeg? -long( nMul ): long( nMul ), nDiv );
}

SvStream& ReadFraction(SvStream& rIStream, boost::rational<long>& rRational)
{
    sal_Int32 nTmpNumerator(0), nTmpDenominator(0);
    rIStream.ReadInt32( nTmpNumerator );
    rIStream.ReadInt32( nTmpDenominator );
    // NOTE: this throw boost::bad_rational() if nTmpDenominator is zero
    rRational.assign( nTmpNumerator, nTmpDenominator );
    return rIStream;
}

SvStream& WriteFraction(SvStream& rOStream, const boost::rational<long>& rRational)
{
    //fdo#39428 SvStream no longer supports operator<<(long)
    rOStream.WriteInt32( sal::static_int_cast<sal_Int32>(rRational.numerator()) );
    rOStream.WriteInt32( sal::static_int_cast<sal_Int32>(rRational.denominator()) );
    return rOStream;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
