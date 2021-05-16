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
#include "scdllapi.h"
#include <tools/cpuid.hxx>
#include <formula/errorcodes.hxx>

/* TODO Remove this once GCC updated and AVX512 can work. */
#ifdef __GNUC__
#if __GNUC__ < 9
#ifdef LO_AVX512F_AVAILABLE
#define HAS_LO_AVX512F_AVAILABLE
#undef LO_AVX512F_AVAILABLE
#endif
#endif
#endif

namespace sc::op
{
/* Checkout available optimization options */
const bool hasAVX512F = cpuid::hasAVX512F();
const bool hasAVX = cpuid::hasAVX();
const bool hasSSE2 = cpuid::hasSSE2();

/**
  * Performs one step of the Neumanier sum between doubles
  * Overwrites the summand and error
  * @parma sum
  * @param err
  * @param value
  */
inline void sumNeumanierNormal(double& sum, double& err, const double& value)
{
    double t = sum + value;
    if (std::abs(sum) >= std::abs(value))
        err += (sum - t) + value;
    else
        err += (value - t) + sum;
    sum = t;
}

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

        for (; i < nUnrolledSize; i += 4)
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

/* Available methos */
KahanSum executeAVX512F(size_t& i, size_t nSize, const double* pCurrent);
KahanSum executeAVX(size_t& i, size_t nSize, const double* pCurrent);
KahanSum executeSSE2(size_t& i, size_t nSize, const double* pCurrent);

/**
  * This function task is to choose the fastest method available to perform the sum.
  * @param i
  * @param nSize
  * @param pCurrent
  */
static inline KahanSum executeFast(size_t& i, size_t nSize, const double* pCurrent)
{
#ifdef LO_AVX512F_AVAILABLE
    if (hasAVX512F)
        return executeAVX512F(i, nSize, pCurrent);
#endif
#ifdef LO_AVX_AVAILABLE
    if (hasAVX)
        return executeAVX(i, nSize, pCurrent);
#endif
#ifdef LO_SSE2_AVAILABLE
    if (hasSSE2)
        return executeSSE2(i, nSize, pCurrent);
#endif
    return executeUnrolled(i, nSize, pCurrent);
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

#include <cassert>
/**
  * Check it works fine.
  */
static inline void test_sums()
{
    // Needs exactly 9 terms
    double summands[9] = { 0, 1, 2, 3, 4, 10, 20, 2, -1 };
    double* pCurrent = summands;
    bool bOK = true;
    size_t i = 0;
#ifdef LO_AVX512F_AVAILABLE
    if (hasAVX512F)
        bOK &= executeAVX512F(i, 9, pCurrent).get() == 42.0;
#endif
#ifdef LO_AVX_AVAILABLE
    if (hasAVX)
        bOK &= executeAVX(i, 9, pCurrent).get() == 42.0;
#endif
#ifdef LO_SSE2_AVAILABLE
    if (hasSSE2)
        bOK &= executeSSE2(i, 9, pCurrent).get() == 42.0;
#endif
    bOK &= executeUnrolled(i, 9, pCurrent).get() == 42.0;
    assert(bOK);
}

} // end namespace sc::op

/* TODO Remove this once GCC updated and AVX512 can work. */
#ifdef __GNUC__
#if __GNUC__ < 9
#ifdef HAS_LO_AVX512F_AVAILABLE
#define LO_AVX512F_AVAILABLE
#undef HAS_LO_AVX512F_AVAILABLE
#endif
#endif
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
