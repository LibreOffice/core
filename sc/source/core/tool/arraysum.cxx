/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */
#include <arraysumfunctor.hxx>
#include <formula/errorcodes.hxx>

#include <cmath>

#include <tools/simd.hxx>
#include <tools/cpuid.hxx>
#include <tools/simdsupport.hxx>

//AVX512VL + AVX512F + KNCNI

namespace sc::op
{
/** Kahan sum with double values.
  */
static inline void sumAVX64(double& sum, double& err, const double& value)
{
    double t = sum + value;
    if (std::abs(sum) >= std::abs(value))
        err += (sum - t) + value;
    else
        err += (value - t) + sum;
    sum = t;
}

#ifdef LO_AVX512F_AVAILABLE // New processors

/** Kahan sum with AVX512.
  */
static inline void sumAVX512(__m512d& sum, __m512d& err, const __m512d& value)
{
    // Temporal parameter
    __m512d t = _mm512_add_pd(sum, value);
    // Absolute value of the total sum
    __m512d asum = _mm512_abs_pd(sum);
    // Absolute value of the value to add
    __m512d avalue = _mm512_abs_pd(value);
    // Comaprate the absolute values sum >= value
    __mmask8 mask = _mm512_cmplt_pd_mask(avalue, asum);
    // The following code has this form ( a - t + b)
    // Case 1: a = sum b = value
    // Case 2: a = value b = sum
    __m512d a = _mm512_mask_blend_pd(mask, sum, value);
    __m512d b = _mm512_mask_blend_pd(mask, value, sum);
    err = _mm512_add_pd(err, _mm512_add_pd(_mm512_sub_pd(a, t), b));
    // Store result
    sum = t;
}

/** Execute Kahan sum with AVX512.
  */
static inline KahanSum executeAVX512(size_t& i, size_t nSize, const double* pCurrent)
{
    if (nSize > i + 8)
    {
        // Make sure we don't fall out of bounds.
        nSize -= (nSize - i) % 8;

        // Setup sums and errors as 0
        __m512d sum = _mm512_setzero_pd();
        __m512d err = _mm512_setzero_pd();

        // Sum the stuff
        for (; i < nSize; i += 8)
        {
            // Kahan sum
            __m512d load = _mm512_loadu_pd(pCurrent);
            sumAVX512(sum, err, load);
            pCurrent += 8;
        }

        // Store result
        static_assert(sizeof(double) == 8);
        double sums[8];
        double errs[8];
        _mm512_storeu_pd(reinterpret_cast<void*>(&sums[0]), sum);
        _mm512_storeu_pd(reinterpret_cast<void*>(&errs[0]), err);

        // First Kahan & pairwise summation
        // 0+1 1+2 3+4 4+5 6+7 -> 0, 2, 4, 6
        sumAVX64(sums[0], errs[0], sums[1]);
        sumAVX64(sums[2], errs[2], sums[3]);
        sumAVX64(sums[4], errs[4], sums[5]);
        sumAVX64(sums[6], errs[6], sums[7]);
        sumAVX64(sums[0], errs[0], errs[1]);
        sumAVX64(sums[2], errs[2], errs[3]);
        sumAVX64(sums[4], errs[4], errs[5]);
        sumAVX64(sums[6], errs[6], errs[7]);

        // Second Kahan & pairwise summation
        // 0+2 4+6 -> 0, 4
        sumAVX64(sums[0], errs[0], sums[2]);
        sumAVX64(sums[4], errs[4], sums[6]);
        sumAVX64(sums[0], errs[0], errs[2]);
        sumAVX64(sums[4], errs[4], errs[6]);

        // Third Kahan & pairwise summation
        // 0+4 -> 0
        sumAVX64(sums[0], errs[0], sums[4]);
        sumAVX64(sums[0], errs[0], errs[4]);

        // Return final result
        return KahanSum(sums[0], errs[0]);
    }
    else
        return 0.0;
}
#endif

#ifdef LO_AVX_AVAILABLE // Old processors

static const __m256d ANNULATE_SIGN_BIT
    = _mm256_castsi256_pd(_mm256_set1_epi64x(0x7FFF'FFFF'FFFF'FFFF));

/** Kahan sum with SSE4.2.
  */
static inline void sumAVX(__m256d& sum, __m256d& err, const __m256d& value)
{
    // Temporal parameter
    __m256d t = _mm256_add_pd(sum, value);
    // Absolute value of the total sum
    __m256d asum = _mm256_and_pd(sum, ANNULATE_SIGN_BIT);
    // Absolute value of the value to add
    __m256d avalue = _mm256_and_pd(value, ANNULATE_SIGN_BIT);
    // Comaprate the absolute values sum >= value
    __m256d mask = _mm256_cmp_pd(asum, avalue, _CMP_GE_OQ);
    // The following code has this form ( a - t + b)
    // Case 1: a = sum b = value
    // Case 2: a = value b = sum
    __m256d a = _mm256_add_pd(_mm256_mul_pd(mask, sum), _mm256_mul_pd(mask, value));
    __m256d b = _mm256_add_pd(_mm256_mul_pd(mask, value), _mm256_mul_pd(mask, sum));
    err = _mm256_add_pd(err, _mm256_add_pd(_mm256_sub_pd(a, t), b));
    // Store result
    sum = t;
}

/** Execute Kahan sum with AVX.
  */
static inline KahanSum executeAVX(size_t& i, size_t nSize, const double* pCurrent)
{
    if (nSize > i + 8)
    {
        // Make sure we don't fall out of bounds.
        nSize -= (nSize - i) % 8;

        // Setup sums and errors as 0
        __m256d sum1 = _mm256_setzero_pd();
        __m256d err1 = _mm256_setzero_pd();
        __m256d sum2 = _mm256_setzero_pd();
        __m256d err2 = _mm256_setzero_pd();

        for (; i < nSize; i += 8)
        {
            // Kahan sum 1
            __m256d load1 = _mm256_loadu_pd(pCurrent);
            sumAVX(sum1, err1, load1);
            pCurrent += 4;

            // Kahan sum 2
            __m256d load2 = _mm256_loadu_pd(pCurrent);
            sumAVX(sum2, err2, load2);
            pCurrent += 4;
        }

        // Now we combine pairwise summation with Kahan summation

        // sum 1 + sum 2
        sumAVX(sum1, err1, sum2);
        sumAVX(sum1, err1, err2);

        // Store results
        double sums[4];
        double errs[4];
        _mm256_storeu_pd(&sums[0], sum1);
        _mm256_storeu_pd(&errs[0], err1);

        // First Kahan & pairwise summation
        // 0+1 1+2 -> 0, 2
        sumAVX64(sums[0], errs[0], sums[1]);
        sumAVX64(sums[2], errs[2], sums[3]);
        sumAVX64(sums[0], errs[0], errs[1]);
        sumAVX64(sums[2], errs[2], errs[3]);

        // 0+2 0
        sumAVX64(sums[0], errs[0], sums[2]);
        sumAVX64(sums[0], errs[0], errs[2]);

        // Store result
        return KahanSum(sums[0], errs[0]);
    }
    return 0.0;
}
#endif

// Even older processors
/** If no boosts available, Unrolled KahanSum.
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

const static bool hasAVX512 = cpuid::hasAVX512();
const static bool hasAVX = cpuid::hasAVX();

KahanSum executeFast(size_t& i, size_t nSize, const double* pCurrent)
{
#ifdef LO_AVX512F_AVAILABLE
    if (hasAVX512)
        return sc::op::executeAVX512(i, nSize, pCurrent);
#endif
#ifdef LO_AVX_AVAILABLE
    if (hasAVX)
        return sc::op::executeAVX(i, nSize, pCurrent);
#endif
    return sc::op::executeUnrolled(i, nSize, pCurrent);
}

} // end namespace sc::op

KahanSum sc::op::sumArray(const double* pArray, size_t nSize)
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
