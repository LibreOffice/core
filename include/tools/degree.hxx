/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <sal/config.h>

#include <basegfx/numeric/ftools.hxx>
#include <sal/types.h>
#include <o3tl/strong_int.hxx>
#include <o3tl/unit_conversion.hxx>
#include <cstdlib>
#include <math.h>
#include <numeric>
#include <utility>

template <int N> struct FractionTag;
// 1/Nth fraction of a degree
template <typename I, int N> using Degree = o3tl::strong_int<I, FractionTag<N>>;

template <typename I, int N> char (&NofDegree(Degree<I, N>))[N]; // helper
// Nof<DegreeN> gives compile-time constant N, needed in templates
template <class D> constexpr int Nof = sizeof(NofDegree(std::declval<D>()));

/** tenths of a Degree, normally rotation */
typedef Degree<sal_Int16, 10> Degree10;

/** custom literal */
constexpr Degree10 operator""_deg10(unsigned long long n) { return Degree10{ n }; }

/** hundredths of a Degree, normally rotation */
typedef Degree<sal_Int32, 100> Degree100;

// Android has trouble calling the correct overload of std::abs
#ifdef ANDROID
inline Degree100 abs(Degree100 x) { return Degree100(std::abs(static_cast<int>(x.get()))); }
#else
inline Degree100 abs(Degree100 x) { return Degree100(std::abs(x.get())); }
#endif

/** custom literal */
constexpr Degree100 operator""_deg100(unsigned long long n) { return Degree100{ n }; }

/** conversion functions */

template <class To, typename IofFrom, int NofFrom> inline To to(Degree<IofFrom, NofFrom> x)
{
    constexpr sal_Int64 m = Nof<To> / std::gcd(Nof<To>, NofFrom);
    constexpr sal_Int64 d = NofFrom / std::gcd(Nof<To>, NofFrom);
    return To{ o3tl::convert(x.get(), m, d) };
}

template <class D> inline double toRadians(D x) { return basegfx::deg2rad<Nof<D>>(x.get()); }
template <class D> inline double toDegrees(D x) { return x.get() / static_cast<double>(Nof<D>); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
