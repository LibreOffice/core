/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#define LO_ARRAYSUM_SPACE AVX512
#include "arraysum.hxx"

#include <arraysumfunctorinternal.hxx>

#include <tools/simdsupport.hxx>

#include <stdlib.h>

namespace sc::op
{
#ifdef LO_AVX512F_AVAILABLE

bool hasAVX512FCode() { return true; }

using namespace AVX512;

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
    // Compare the absolute values sum >= value
    __mmask8 mask = _mm512_cmp_pd_mask(avalue, asum, _CMP_GE_OQ);
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
KahanSumSimple executeAVX512F(size_t& i, size_t nSize, const double* pCurrent)
{
    // Make sure we don't fall out of bounds.
    // This works by sums of 8 terms.
    // So the 8'th term is i+7
    // If we iterate until nSize won't fall out of bounds
    if (nSize > i + 7)
    {
        // Setup sums and errors as 0
        __m512d sum = _mm512_setzero_pd();
        __m512d err = _mm512_setzero_pd();

        // Sum the stuff
        for (; i + 7 < nSize; i += 8)
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
        _mm512_storeu_pd(&sums[0], sum);
        _mm512_storeu_pd(&errs[0], err);

        // First Kahan & pairwise summation
        // 0+1 1+2 3+4 4+5 6+7 -> 0, 2, 4, 6
        sumNeumanierNormal(sums[0], errs[0], sums[1]);
        sumNeumanierNormal(sums[2], errs[2], sums[3]);
        sumNeumanierNormal(sums[4], errs[4], sums[5]);
        sumNeumanierNormal(sums[6], errs[6], sums[7]);
        sumNeumanierNormal(sums[0], errs[0], errs[1]);
        sumNeumanierNormal(sums[2], errs[2], errs[3]);
        sumNeumanierNormal(sums[4], errs[4], errs[5]);
        sumNeumanierNormal(sums[6], errs[6], errs[7]);

        // Second Kahan & pairwise summation
        // 0+2 4+6 -> 0, 4
        sumNeumanierNormal(sums[0], errs[0], sums[2]);
        sumNeumanierNormal(sums[4], errs[4], sums[6]);
        sumNeumanierNormal(sums[0], errs[0], errs[2]);
        sumNeumanierNormal(sums[4], errs[4], errs[6]);

        // Third Kahan & pairwise summation
        // 0+4 -> 0
        sumNeumanierNormal(sums[0], errs[0], sums[4]);
        sumNeumanierNormal(sums[0], errs[0], errs[4]);

        // Return final result
        return { sums[0], errs[0] };
    }
    return { 0.0, 0.0 };
}

#else // LO_AVX512F_AVAILABLE

bool hasAVX512FCode() { return false; }

KahanSumSimple executeAVX512F(size_t&, size_t, const double*) { abort(); }

#endif

} // end namespace sc::op

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
