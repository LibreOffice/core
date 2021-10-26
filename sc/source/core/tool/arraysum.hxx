/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <math.h>

namespace sc::op
{
// Code must not be shared between different CPU instrinsics flags (e.g. in debug mode the compiler would not
// inline them, and merge the copies, keeping only the one with the most demanding CPU set that's not available otherwise).
// Put everything in a different namespace and additionally try to force inlining.
namespace LO_ARRAYSUM_SPACE
{
#if defined _MSC_VER
#define INLINE __forceinline static
#elif defined __GNUC__
#define INLINE __attribute__((always_inline)) static inline
#else
#define static inline
#endif

/**
  * Performs one step of the Neumanier sum between doubles
  * Overwrites the summand and error
  * @parma sum
  * @param err
  * @param value
  */
INLINE void sumNeumanierNormal(double& sum, double& err, const double& value)
{
    double t = sum + value;
    if (fabs(sum) >= fabs(value))
        err += (sum - t) + value;
    else
        err += (value - t) + sum;
    sum = t;
}

#undef INLINE

} // end namespace LO_ARRAYSUM_SPACE
} // end namespace sc::op

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
