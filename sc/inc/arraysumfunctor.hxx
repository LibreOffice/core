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

#include <cmath>
#include "kahan.hxx"
#include "arraysumfunctor.hxx"
#include <formula/errorcodes.hxx>

namespace sc::op
{
// Checkout available optimization options.
// Note that it turned out to be problematic to support CPU-specific code
// that's not guaranteed to be available on that specific platform (see
// git history). SSE2 is guaranteed on x86_64 and it is our baseline requirement
// for x86 on Windows, so SSE2 use is hardcoded on those platforms.
// Whenever we raise baseline to e.g. AVX, this may get
// replaced with AVX code (get it from git history).
// Do it similarly with other platforms.
#if defined(X86_64) || (defined(X86) && defined(_WIN32))
#define SC_USE_SSE2 1
KahanSum executeSSE2(size_t& i, size_t nSize, const double* pCurrent);
#else
#define SC_USE_SSE2 0
#endif

/**
  * If no boosts available, Unrolled KahanSum.
  * Most likely to use on android.
  */
static inline KahanSum executeUnrolled(size_t& i, size_t nSize, const double* pCurrent)
{
    size_t nRealSize = nSize - i;
    size_t nUnrolledSize = nRealSize - (nRealSize % 4);

    if (nUnrolledSize > 0)
    {
        KahanSum sum0 = 0.0;
        KahanSum sum1 = 0.0;
        KahanSum sum2 = 0.0;
        KahanSum sum3 = 0.0;

        for (; i + 3 < nUnrolledSize; i += 4)
        {
            sum0 += *pCurrent++;
            sum1 += *pCurrent++;
            sum2 += *pCurrent++;
            sum3 += *pCurrent++;
        }
        // We are using pairwise summation alongside Kahan
        return (sum0 + sum1) + (sum2 + sum3);
    }
    return 0.0;
}

/**
  * This function task is to choose the fastest method available to perform the sum.
  * @param i
  * @param nSize
  * @param pCurrent
  */
static inline KahanSum executeFast(size_t& i, size_t nSize, const double* pCurrent)
{
#if SC_USE_SSE2
    return executeSSE2(i, nSize, pCurrent);
#else
    return executeUnrolled(i, nSize, pCurrent);
#endif
}

/**
  * Performs the sum of an array.
  * Note that align 16 will speed up the process.
  * @param pArray
  * @param nSize
  */
inline KahanSum sumArray(const double* pArray, size_t nSize)
{
    size_t i = 0;
    const double* pCurrent = pArray;
    KahanSum fSum = executeFast(i, nSize, pCurrent);

    // sum rest of the array
    for (; i < nSize; ++i)
        fSum += pArray[i];

    // If the sum is a NaN, some of the terms were empty cells, probably.
    // Re-calculate, carefully
    double fVal = fSum.get();
    if (!std::isfinite(fVal))
    {
        FormulaError nErr = GetDoubleErrorValue(fVal);
        if (nErr == FormulaError::NoValue)
        {
            fSum = 0;
            for (i = 0; i < nSize; i++)
            {
                if (!std::isfinite(pArray[i]))
                {
                    nErr = GetDoubleErrorValue(pArray[i]);
                    if (nErr != FormulaError::NoValue)
                        fSum += pArray[i]; // Let errors encoded as NaNs propagate ???
                }
                else
                    fSum += pArray[i];
            }
        }
    }
    return fSum;
}

} // end namespace sc::op

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
