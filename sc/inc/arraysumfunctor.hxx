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

namespace sc::op
{
/* Checkout available optimization options */
SC_DLLPUBLIC extern const bool hasAVX512F;
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

/* Available methos */
SC_DLLPUBLIC KahanSum executeAVX512F(size_t& i, size_t nSize, const double* pCurrent);
SC_DLLPUBLIC KahanSum executeAVX(size_t& i, size_t nSize, const double* pCurrent);
SC_DLLPUBLIC KahanSum executeSSE2(size_t& i, size_t nSize, const double* pCurrent);

/**
  * This function task is to choose the fastest method available to perform the sum.
  * @param i
  * @param nSize
  * @param pCurrent
  */
static inline KahanSum executeFast(size_t& i, size_t nSize, const double* pCurrent)
{
    if (hasAVX512F)
        return executeAVX512F(i, nSize, pCurrent);
    if (hasAVX)
        return executeAVX(i, nSize, pCurrent);
    if (hasSSE2)
        return executeSSE2(i, nSize, pCurrent);
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

} // end namespace sc::op

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
