/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * libfixmath is Copyright (c) 2011-2021 Flatmush <Flatmush@gmail.com>,
 * Petteri Aimonen <Petteri.Aimonen@gmail.com>, & libfixmath AUTHORS
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <tools/fix16.hxx>

#include <bit>

const fix16_t fix16_minimum = 0x80000000; /*!< the minimum value of fix16_t */
const fix16_t fix16_overflow = 0x80000000; /*!< the value used to indicate overflows */

static inline uint32_t fix_abs(fix16_t in)
{
    if (in == fix16_minimum)
    {
        // minimum negative number has same representation as
        // its absolute value in unsigned
        return 0x80000000;
    }
    else
    {
        return (in >= 0) ? in : -in;
    }
}

/* 64-bit implementation for fix16_mul. Fastest version for e.g. ARM Cortex M3.
 * Performs a 32*32 -> 64bit multiplication. The middle 32 bits are the result,
 * bottom 16 bits are used for rounding, and upper 16 bits are used for overflow
 * detection.
 */

fix16_t fix16_mul(fix16_t inArg0, fix16_t inArg1)
{
    int64_t product = static_cast<int64_t>(inArg0) * inArg1;

    // The upper 17 bits should all be the same (the sign).
    uint32_t upper = (product >> 47);

    if (product < 0)
    {
        if (~upper)
            return fix16_overflow;

        // This adjustment is required in order to round -1/2 correctly
        product--;
    }
    else
    {
        if (upper)
            return fix16_overflow;
    }

    fix16_t result = product >> 16;
    result += (product & 0x8000) >> 15;

    return result;
}

/* 32-bit implementation of fix16_div. Fastest version for e.g. ARM Cortex M3.
 * Performs 32-bit divisions repeatedly to reduce the remainder. For this to
 * be efficient, the processor has to have 32-bit hardware division.
 */
fix16_t fix16_div(fix16_t a, fix16_t b)
{
    // This uses a hardware 32/32 bit division multiple times, until we have
    // computed all the bits in (a<<17)/b. Usually this takes 1-3 iterations.

    if (b == 0)
        return fix16_minimum;

    uint32_t remainder = fix_abs(a);
    uint32_t divider = fix_abs(b);
    uint64_t quotient = 0;
    int bit_pos = 17;

    // Kick-start the division a bit.
    // This improves speed in the worst-case scenarios where N and D are large
    // It gets a lower estimate for the result by N/(D >> 17 + 1).
    if (divider & 0xFFF00000)
    {
        uint32_t shifted_div = (divider >> 17) + 1;
        quotient = remainder / shifted_div;
        uint64_t tmp = (quotient * static_cast<uint64_t>(divider)) >> 17;
        remainder -= static_cast<uint32_t>(tmp);
    }

    // If the divider is divisible by 2^n, take advantage of it.
    while (!(divider & 0xF) && bit_pos >= 4)
    {
        divider >>= 4;
        bit_pos -= 4;
    }

    while (remainder && bit_pos >= 0)
    {
        // Shift remainder as much as we can without overflowing
        int shift = std::countl_zero(remainder);
        if (shift > bit_pos)
            shift = bit_pos;
        remainder <<= shift;
        bit_pos -= shift;

        uint32_t div = remainder / divider;
        remainder = remainder % divider;
        quotient += static_cast<uint64_t>(div) << bit_pos;

        if (div & ~(0xFFFFFFFF >> bit_pos))
            return fix16_overflow;

        remainder <<= 1;
        bit_pos--;
    }

    // Quotient is always positive so rounding is easy
    quotient++;

    fix16_t result = quotient >> 1;

    // Figure out the sign of the result
    if ((a ^ b) & 0x80000000)
    {
        if (result == fix16_minimum)
            return fix16_overflow;

        result = -result;
    }

    return result;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
