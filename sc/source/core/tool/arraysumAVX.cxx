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
#include <sal/log.hxx>
#include <tools/simd.hxx>
#include <tools/simdsupport.hxx>

namespace sc::op
{
#ifdef LO_AVX_AVAILABLE // Old processors

const __m256d ANNULATE_SIGN_BIT = _mm256_castsi256_pd(_mm256_set1_epi64x(0x7FFF'FFFF'FFFF'FFFF));

/** Kahan sum with AVX.
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
    __m256d a = _mm256_add_pd(_mm256_and_pd(mask, sum), _mm256_andnot_pd(mask, value));
    __m256d b = _mm256_add_pd(_mm256_and_pd(mask, value), _mm256_andnot_pd(mask, sum));
    err = _mm256_add_pd(err, _mm256_add_pd(_mm256_sub_pd(a, t), b));
    // Store result
    sum = t;
}

#endif

/** Execute Kahan sum with AVX.
  */
KahanSum executeAVX(size_t& i, size_t nSize, const double* pCurrent)
{
#ifdef LO_AVX_AVAILABLE
    // Make sure we don't fall out of bounds.
    // This works by sums of 8 terms.
    // So the 8'th term is i+7
    // If we iterate untill nSize won't fall out of bounds
    if (nSize > i + 7)
    {
        // Setup sums and errors as 0
        __m256d sum1 = _mm256_setzero_pd();
        __m256d err1 = _mm256_setzero_pd();
        __m256d sum2 = _mm256_setzero_pd();
        __m256d err2 = _mm256_setzero_pd();

        for (; i + 7 < nSize; i += 8)
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

        // sum 1 + sum 2 -> sum 1
        sumAVX(sum1, err1, sum2);
        sumAVX(sum1, err1, err2);

        // Store results
        double sums[4];
        double errs[4];
        _mm256_storeu_pd(&sums[0], sum1);
        _mm256_storeu_pd(&errs[0], err1);

        // First Kahan & pairwise summation
        // 0+1 1+2 -> 0, 2
        sumNeumanierNormal(sums[0], errs[0], sums[1]);
        sumNeumanierNormal(sums[2], errs[2], sums[3]);
        sumNeumanierNormal(sums[0], errs[0], errs[1]);
        sumNeumanierNormal(sums[2], errs[2], errs[3]);

        // 0+2 -> 0
        sumNeumanierNormal(sums[0], errs[0], sums[2]);
        sumNeumanierNormal(sums[0], errs[0], errs[2]);

        // Store result
        return KahanSum(sums[0], errs[0]);
    }
    return 0.0;
#else
    SAL_WARN("sc", "Failed to use AVX");
    (void)i;
    (void)nSize;
    (void)pCurrent;
    return 0.0;
#endif
}

} // end namespace sc::op

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
