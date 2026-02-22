/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <arraysum.hxx>
#include <tools/simdsupport.hxx>

#include <math.h>

#ifdef LO_AVX_AVAILABLE

namespace sc::op
{
/** Kahan sum with AVX. */
LO_FORCE_INLINE void sumAVX(__m256d& sum, __m256d& err, const __m256d& value)
{
    const __m256d ANNULATE_SIGN_BIT
        = _mm256_castsi256_pd(_mm256_set1_epi64x(0x7FFF'FFFF'FFFF'FFFF));
    // Temporal parameter
    __m256d t = _mm256_add_pd(sum, value);
    // Absolute value of the total sum
    __m256d asum = _mm256_and_pd(sum, ANNULATE_SIGN_BIT);
    // Absolute value of the value to add
    __m256d avalue = _mm256_and_pd(value, ANNULATE_SIGN_BIT);
    // Compare the absolute values sum >= value
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

/** Performs one step of Neumaier summation. */
LO_FORCE_INLINE void sumNeumaier(double& sum, double& err, double value)
{
    double t = sum + value;
    if (fabs(sum) >= fabs(value))
        err += (sum - t) + value;
    else
        err += (value - t) + sum;
    sum = t;
}

/** Execute Kahan sum with AVX. */
LO_DLLPUBLIC_EXPORT void executeAVX(size_t& i, size_t nSize, const double* pCurrent, double& outSum,
                                    double& outErr)
{
    // Make sure we don't fall out of bounds.
    // This works by sums of 8 terms.
    // So the 8'th term is i+7
    // If we iterate until nSize won't fall out of bounds
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

        // 1+2 -> 1
        sumAVX(sum1, err1, sum2);
        sumAVX(sum1, err1, err2);

        // Store results
        double sums[4];
        double errs[4];
        _mm256_storeu_pd(&sums[0], sum1);
        _mm256_storeu_pd(&errs[0], err1);

        // Pairwise summation of the 4 lanes
        // 0+1 -> 0
        sumNeumaier(sums[0], errs[0], sums[1]);
        sumNeumaier(sums[0], errs[0], errs[1]);
        // 2+3 -> 2
        sumNeumaier(sums[2], errs[2], sums[3]);
        sumNeumaier(sums[2], errs[2], errs[3]);
        // 0+2 -> 0
        sumNeumaier(sums[0], errs[0], sums[2]);
        sumNeumaier(sums[0], errs[0], errs[2]);

        // Store result
        outSum = sums[0];
        outErr = errs[0];
    }
}

} // end namespace sc::op

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
