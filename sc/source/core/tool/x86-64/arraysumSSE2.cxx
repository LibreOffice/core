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
#include <tools/simdsupport.hxx>

#include <stdlib.h>

#if SC_USE_SSE2

namespace sc::op
{
/** Kahan sum with SSE2.
  */
static inline void sumSSE2(__m128d& sum, __m128d& err, const __m128d& value)
{
    const __m128d ANNULATE_SIGN_BIT = _mm_castsi128_pd(_mm_set1_epi64x(0x7FFF'FFFF'FFFF'FFFF));
    // Temporal parameter
    __m128d t = _mm_add_pd(sum, value);
    // Absolute value of the total sum
    __m128d asum = _mm_and_pd(sum, ANNULATE_SIGN_BIT);
    // Absolute value of the value to add
    __m128d avalue = _mm_and_pd(value, ANNULATE_SIGN_BIT);
    // Compare the absolute values sum >= value
    __m128d mask = _mm_cmpge_pd(asum, avalue);
    // The following code has this form ( a - t + b)
    // Case 1: a = sum b = value
    // Case 2: a = value b = sum
    __m128d a = _mm_add_pd(_mm_and_pd(mask, sum), _mm_andnot_pd(mask, value));
    __m128d b = _mm_add_pd(_mm_and_pd(mask, value), _mm_andnot_pd(mask, sum));
    err = _mm_add_pd(err, _mm_add_pd(_mm_sub_pd(a, t), b));
    // Store result
    sum = t;
}

/** Execute Kahan sum with SSE2.
  */
KahanSum executeSSE2(size_t& i, size_t nSize, const double* pCurrent)
{
    // Make sure we don't fall out of bounds.
    // This works by sums of 8 terms.
    // So the 8'th term is i+7
    // If we iterate until nSize won't fall out of bounds
    if (nSize > i + 7)
    {
        // Setup sums and errors as 0
        __m128d sum1 = _mm_setzero_pd();
        __m128d err1 = _mm_setzero_pd();
        __m128d sum2 = _mm_setzero_pd();
        __m128d err2 = _mm_setzero_pd();
        __m128d sum3 = _mm_setzero_pd();
        __m128d err3 = _mm_setzero_pd();
        __m128d sum4 = _mm_setzero_pd();
        __m128d err4 = _mm_setzero_pd();

        for (; i + 7 < nSize; i += 8)
        {
            // Kahan sum 1
            __m128d load1 = _mm_loadu_pd(pCurrent);
            sumSSE2(sum1, err1, load1);
            pCurrent += 2;

            // Kahan sum 2
            __m128d load2 = _mm_loadu_pd(pCurrent);
            sumSSE2(sum2, err2, load2);
            pCurrent += 2;

            // Kahan sum 3
            __m128d load3 = _mm_loadu_pd(pCurrent);
            sumSSE2(sum3, err3, load3);
            pCurrent += 2;

            // Kahan sum 4
            __m128d load4 = _mm_loadu_pd(pCurrent);
            sumSSE2(sum4, err4, load4);
            pCurrent += 2;
        }

        // Now we combine pairwise summation with Kahan summation

        // 1+2 3+4 -> 1, 3
        sumSSE2(sum1, err1, sum2);
        sumSSE2(sum1, err1, err2);
        sumSSE2(sum3, err3, sum4);
        sumSSE2(sum3, err3, err4);

        // 1+3 -> 1
        sumSSE2(sum1, err1, sum3);
        sumSSE2(sum1, err1, err3);

        // Store results
        double sums[2];
        double errs[2];
        _mm_storeu_pd(&sums[0], sum1);
        _mm_storeu_pd(&errs[0], err1);

        // First Kahan & pairwise summation
        // 0+1 -> 0
        KahanSum::sumNeumaierNormal(sums[0], errs[0], sums[1]);
        KahanSum::sumNeumaierNormal(sums[0], errs[0], errs[1]);

        // Store result
        return { sums[0], errs[0] };
    }
    return { 0.0, 0.0 };
}

} // namespace

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
