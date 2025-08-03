/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <o3tl/safeint.hxx>
#include <sal/types.h>

#include <array>
#include <cassert>
#include <numeric>
#include <utility>
#include <type_traits>

namespace o3tl
{
// Length units
enum class Length
{
    mm100 = 0, // 1/100th mm
    mm10, // 1/10 mm, corresponds to MapUnit::Map10thMM
    mm, // millimeter
    cm, // centimeter
    m, // meter
    km, // kilometer
    emu, // English Metric Unit: 1/360000 cm, 1/914400 in
    twip, // "Twentieth of a point" aka "dxa": 1/20 pt
    pt, // Point: 1/72 in
    pc, // Pica: 1/6 in, corresponds to FieldUnit::PICA and MeasureUnit::PICA
    in1000, // 1/1000 in, corresponds to MapUnit::Map1000thInch
    in100, // 1/100 in, corresponds to MapUnit::Map100thInch
    in10, // 1/10 in, corresponds to MapUnit::Map10thInch
    in, // inch
    ft, // foot
    mi, // mile
    master, // PPT Master Unit: 1/576 in
    px, // "pixel" unit: 15 twip (96 ppi), corresponds to MeasureUnit::PIXEL
    ch, // "char" unit: 210 twip (14 px), corresponds to FieldUnit::CHAR
    line, // "line" unit: 312 twip, corresponds to FieldUnit::LINE
    count, // <== add new units above this last entry
    invalid = -1
};

// If other categories of units would be needed (like time), a separate scoped enum
// should be created, respective conversion array prepared in detail namespace, and
// respective md(NewUnit, NewUnit) overload introduced, which would allow using
// o3tl::convert(), o3tl::convertSaturate() and o3tl::getConversionMulDiv() with the
// new category in a type-safe way, without mixing unrelated units.

namespace detail
{
// Common utilities

// A special function to avoid compiler warning comparing signed and unsigned values
template <typename I> constexpr bool isBetween(I n, sal_Int64 min, sal_Int64 max)
{
    assert(max > 0 && min < 0);
    if constexpr (std::is_signed_v<I>)
        return n >= min && n <= max;
    else
        return n <= sal_uInt64(max);
}

// Ensure correct rounding for both positive and negative integers
template <typename I, std::enable_if_t<std::is_integral_v<I>, int> = 0>
constexpr sal_Int64 MulDiv(I n, sal_Int64 m, sal_Int64 d)
{
    assert(m > 0 && d > 0);
    assert(isBetween(n, (SAL_MIN_INT64 + d / 2) / m, (SAL_MAX_INT64 - d / 2) / m));
    // coverity[dead_error_line] - suppress warning for template
    return (n >= 0 ? (n * m + d / 2) : (n * m - d / 2)) / d;
}
template <typename F, std::enable_if_t<std::is_floating_point_v<F>, int> = 0>
constexpr double MulDiv(F f, sal_Int64 m, sal_Int64 d)
{
    assert(m > 0 && d > 0);
    return f * (double(m) / d);
}

template <typename I, std::enable_if_t<std::is_integral_v<I>, int> = 0>
constexpr sal_Int64 MulDiv(I n, sal_Int64 m, sal_Int64 d, bool& bOverflow, sal_Int64 nDefault)
{
    if (!isBetween(n, (SAL_MIN_INT64 + d / 2) / m, (SAL_MAX_INT64 - d / 2) / m))
    {
        bOverflow = true;
        return nDefault;
    }
    bOverflow = false;
    return MulDiv(n, m, d);
}

template <typename I, std::enable_if_t<std::is_integral_v<I>, int> = 0>
constexpr sal_Int64 MulDivSaturate(I n, sal_Int64 m, sal_Int64 d)
{
    if (sal_Int64 d_2 = d / 2; !isBetween(n, (SAL_MIN_INT64 + d_2) / m, (SAL_MAX_INT64 - d_2) / m))
    {
        if (n >= 0)
        {
            if (m > d && std::make_unsigned_t<I>(n) > sal_uInt64(SAL_MAX_INT64 / m * d - d_2))
                return SAL_MAX_INT64; // saturate
            // coverity[ tainted_data_return : FALSE ] version 2023.12.2
            return saturating_add<sal_uInt64>(n, d_2) / d * m; // divide before multiplication
        }
        else if constexpr (std::is_signed_v<I>) // n < 0; don't compile for unsigned n
        {
            if (m > d && n < SAL_MIN_INT64 / m * d + d_2)
                return SAL_MIN_INT64; // saturate
            return saturating_sub<sal_Int64>(n, d_2) / d * m; // divide before multiplication
        }
    }
    return MulDiv(n, m, d);
}

template <class M, class N> constexpr std::common_type_t<M, N> asserting_gcd(M m, N n)
{
    auto ret = std::gcd(m, n);
    assert(ret != 0);
    return ret;
}

// Packs integral multiplier and divisor for conversion from one unit to another
struct m_and_d
{
    sal_Int64 m; // multiplier
    sal_Int64 d; // divisor
    constexpr m_and_d(sal_Int64 _m, sal_Int64 _d)
        : m(_m / asserting_gcd(_m, _d)) // make sure to use smallest quotients here because
        , d(_d / asserting_gcd(_m, _d)) // they will be multiplied when building final table
    {
        assert(_m > 0 && _d > 0);
    }
};

// Resulting static array N x N of all quotients to convert between all units. The
// quotients are minimal to allow largest range of converted numbers without overflow.
// Maybe o3tl::enumarray could be used here, but it's not constexpr yet.
template <int N> constexpr auto prepareMDArray(const m_and_d (&mdBase)[N])
{
    std::array<std::array<sal_Int64, N>, N> a{};
    for (int i = 0; i < N; ++i)
    {
        a[i][i] = 1;
        for (int j = 0; j < i; ++j)
        {
            assert(mdBase[i].m < SAL_MAX_INT64 / mdBase[j].d);
            assert(mdBase[i].d < SAL_MAX_INT64 / mdBase[j].m);
            const sal_Int64 m = mdBase[i].m * mdBase[j].d, d = mdBase[i].d * mdBase[j].m;
            const sal_Int64 g = asserting_gcd(m, d);
            a[i][j] = m / g;
            a[j][i] = d / g;
        }
    }
    return a;
}

// A generic template used for fundamental arithmetic types
template <typename U> constexpr sal_Int64 md(U i, U /*j*/) { return i; }

// Length units implementation

// Array of conversion quotients for mm, used to build final conversion table. Entries
// are { multiplier, divider } to convert respective unit *to* mm. Order of elements
// corresponds to order in o3tl::Length enum (Length::count and Length::invalid omitted).
constexpr m_and_d mdBaseLen[] = {
    { 1, 100 }, // mm100 => mm
    { 1, 10 }, // mm10 => mm
    { 1, 1 }, // mm => mm
    { 10, 1 }, // cm => mm
    { 1000, 1 }, // m => mm
    { 1000000, 1 }, // km => mm
    { 1, 36000 }, // emu => mm
    { 254, 10 * 1440 }, // twip => mm
    { 254, 10 * 72 }, // pt => mm
    { 254, 10 * 6 }, // pc => mm
    { 254, 10000 }, // in1000 => mm
    { 254, 1000 }, // in100 => mm
    { 254, 100 }, // in10 => mm
    { 254, 10 }, // in => mm
    { 254 * 12, 10 }, // ft => mm
    { 254 * 12 * 5280, 10 }, // mi => mm
    { 254, 10 * 576 }, // master => mm
    { 254 * 15, 10 * 1440 }, // px => mm
    { 254 * 210, 10 * 1440 }, // ch => mm
    { 254 * 312, 10 * 1440 }, // line => mm
};
static_assert(std::size(mdBaseLen) == static_cast<int>(Length::count),
              "mdBaseL must have an entry for each unit in o3tl::Length");

// The resulting multipliers and divisors array
constexpr auto aLengthMDArray = prepareMDArray(mdBaseLen);

// an overload taking Length
constexpr sal_Int64 md(Length i, Length j)
{
    const int nI = static_cast<int>(i), nJ = static_cast<int>(j);
    assert(nI >= 0 && o3tl::make_unsigned(nI) < aLengthMDArray.size());
    assert(nJ >= 0 && o3tl::make_unsigned(nJ) < aLengthMDArray.size());
    return aLengthMDArray[nI][nJ];
}

// here might go overloads of md() taking other units ...
}

// Unchecked conversion. Takes a number value, multiplier and divisor
template <typename N> constexpr auto convert(N n, sal_Int64 mul, sal_Int64 div)
{
    return detail::MulDiv(n, mul, div);
}

// Unchecked conversion. Takes a number value and units defined in this header
template <typename N, typename U> constexpr auto convert(N n, U from, U to)
{
    return convert(n, detail::md(from, to), detail::md(to, from));
}

// Convert to twips - for convenience as we do this a lot
template <typename N> constexpr auto toTwips(N number, Length from)
{
    return convert(number, from, Length::twip);
}

// Returns nDefault if intermediate multiplication overflows sal_Int64 (only for integral types).
// On return, bOverflow indicates if overflow happened. nDefault is returned when overflow occurs.
template <typename N, typename U>
constexpr auto convert(N n, U from, U to, bool& bOverflow, sal_Int64 nDefault = 0)
{
    return detail::MulDiv(n, detail::md(from, to), detail::md(to, from), bOverflow, nDefault);
}

// Conversion with saturation (only for integral types). For too large input returns SAL_MAX_INT64.
// When intermediate multiplication would overflow, but the end result is in sal_Int64 range, the
// precision is decreased because of inversion of multiplication and division.
template <typename N, typename U> constexpr auto convertSaturate(N n, U from, U to)
{
    return detail::MulDivSaturate(n, detail::md(from, to), detail::md(to, from));
}

// Conversion with saturation (only for integral types), optimized for return types smaller than
// sal_Int64. In this case, it's easier to clamp input values to known bounds, than to do some
// preprocessing to handle too large input values, just to clamp the result anyway. Use it like:
//
//     sal_Int32 n = convertNarrowing<sal_Int32, o3tl::Length::mm100, o3tl::Length::emu>(m);
template <typename Out, auto from, auto to, typename N,
          std::enable_if_t<
              std::is_integral_v<N> && std::is_integral_v<Out> && sizeof(Out) < sizeof(sal_Int64),
              int> = 0>
constexpr Out convertNarrowing(N n)
{
    constexpr sal_Int64 nMin = convertSaturate(std::numeric_limits<Out>::min(), to, from);
    constexpr sal_Int64 nMax = convertSaturate(std::numeric_limits<Out>::max(), to, from);
    if (static_cast<sal_Int64>(n) > nMax)
        return std::numeric_limits<Out>::max();
    if (static_cast<sal_Int64>(n) < nMin)
        return std::numeric_limits<Out>::min();
    return convert(n, from, to);
}

// Return a pair { multiplier, divisor } for a given conversion
template <typename U> constexpr std::pair<sal_Int64, sal_Int64> getConversionMulDiv(U from, U to)
{
    return { detail::md(from, to), detail::md(to, from) };
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
