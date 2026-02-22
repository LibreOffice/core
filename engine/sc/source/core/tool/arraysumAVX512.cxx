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

#ifdef LO_AVX512F_AVAILABLE

namespace sc::op
{
/** Kahan sum with AVX512F. */
LO_FORCE_INLINE void sumAVX512(__m512d& sum, __m512d& err, const __m512d& value)
{
    // Use integer AND via _mm512_and_si512 (AVX512F) since _mm512_and_pd requires AVX512DQ
    const __m512i ANNULATE_SIGN_BIT = _mm512_set1_epi64(0x7FFF'FFFF'FFFF'FFFF);
    // Temporal parameter
    __m512d t = _mm512_add_pd(sum, value);
    // Absolute value of the total sum
    __m512d asum
        = _mm512_castsi512_pd(_mm512_and_si512(_mm512_castpd_si512(sum), ANNULATE_SIGN_BIT));
    // Absolute value of the value to add
    __m512d avalue
        = _mm512_castsi512_pd(_mm512_and_si512(_mm512_castpd_si512(value), ANNULATE_SIGN_BIT));
    // Compare the absolute values sum >= value
    __mmask8 mask = _mm512_cmp_pd_mask(asum, avalue, _CMP_GE_OQ);
    // The following code has this form ( a - t + b)
    // Case 1: a = sum b = value
    // Case 2: a = value b = sum
    __m512d a = _mm512_mask_blend_pd(mask, value, sum);
    __m512d b = _mm512_mask_blend_pd(mask, sum, value);
    err = _mm512_add_pd(err, _mm512_add_pd(_mm512_sub_pd(a, t), b));
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

/** Execute Kahan sum with AVX512F. */
LO_DLLPUBLIC_EXPORT void executeAVX512F(size_t& i, size_t nSize, const double* pCurrent,
                                        double& outSum, double& outErr)
{
    // Make sure we don't fall out of bounds.
    // This works by sums of 8 terms.
    // So the 8'th term is i+7
    // If we iterate until nSize won't fall out of bounds
    if (nSize > i + 7)
    {
        // Setup sum and error as 0
        __m512d sum1 = _mm512_setzero_pd();
        __m512d err1 = _mm512_setzero_pd();

        for (; i + 7 < nSize; i += 8)
        {
            // Kahan sum
            __m512d load1 = _mm512_loadu_pd(pCurrent);
            sumAVX512(sum1, err1, load1);
            pCurrent += 8;
        }

        // Store results
        double sums[8];
        double errs[8];
        _mm512_storeu_pd(&sums[0], sum1);
        _mm512_storeu_pd(&errs[0], err1);

        // Pairwise summation of the 8 lanes
        // 0+1 -> 0, 2+3 -> 2, 4+5 -> 4, 6+7 -> 6
        sumNeumaier(sums[0], errs[0], sums[1]);
        sumNeumaier(sums[0], errs[0], errs[1]);
        sumNeumaier(sums[2], errs[2], sums[3]);
        sumNeumaier(sums[2], errs[2], errs[3]);
        sumNeumaier(sums[4], errs[4], sums[5]);
        sumNeumaier(sums[4], errs[4], errs[5]);
        sumNeumaier(sums[6], errs[6], sums[7]);
        sumNeumaier(sums[6], errs[6], errs[7]);

        // 0+2 -> 0, 4+6 -> 4
        sumNeumaier(sums[0], errs[0], sums[2]);
        sumNeumaier(sums[0], errs[0], errs[2]);
        sumNeumaier(sums[4], errs[4], sums[6]);
        sumNeumaier(sums[4], errs[4], errs[6]);

        // 0+4 -> 0
        sumNeumaier(sums[0], errs[0], sums[4]);
        sumNeumaier(sums[0], errs[0], errs[4]);

        // Store result
        outSum = sums[0];
        outErr = errs[0];
    }
}

} // namespace

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
