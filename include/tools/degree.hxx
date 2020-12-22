/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <sal/types.h>
#include <o3tl/strong_int.hxx>
#include <cstdlib>
#include <math.h>

/** tenths of a Degree, normally rotation */
typedef o3tl::strong_int<sal_Int16, struct Degree10Tag> Degree10;

/** custom literal */
constexpr Degree10 operator""_deg10(unsigned long long n) { return Degree10{ n }; }

/** hundredths of a Degree, normally rotation */
typedef o3tl::strong_int<sal_Int32, struct Degree100Tag> Degree100;

// Android has trouble calling the correct overload of std::abs
#ifdef ANDROID
inline Degree100 abs(Degree100 x) { return Degree100(std::abs(static_cast<int>(x.get()))); }
#else
inline Degree100 abs(Degree100 x) { return Degree100(std::abs(x.get())); }
#endif

/** custom literal */
constexpr Degree100 operator""_deg100(unsigned long long n) { return Degree100{ n }; }

/** conversion functions */

inline Degree100 toDegree100(Degree10 x) { return Degree100(x.get() * 10); }
inline double toRadians(Degree10 x) { return x.get() * (M_PI / 1800.0); }
inline double toDegrees(Degree10 x) { return x.get() / 10.0; }

inline Degree10 toDegree10(Degree100 x) { return Degree10((x.get() + 5) / 10); }
inline double toRadians(Degree100 x) { return x.get() * (M_PI / 18000.0); }
inline double toDegrees(Degree100 x) { return x.get() / 100.0; }

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
