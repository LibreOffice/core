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

namespace sc
{
double ArraySumFunctor::executeSSE2(size_t& i, const double* pCurrent) const
{
#if defined(LO_SSE2_AVAILABLE)
    double fSum = 0.0;
    size_t nRealSize = mnSize - i;
    size_t nUnrolledSize = nRealSize - (nRealSize % 8);

    if (nUnrolledSize > 0)
    {
        __m128d sum1 = _mm_setzero_pd();
        __m128d sum2 = _mm_setzero_pd();
        __m128d sum3 = _mm_setzero_pd();
        __m128d sum4 = _mm_setzero_pd();

        __m128d err1 = _mm_setzero_pd();
        __m128d err2 = _mm_setzero_pd();
        __m128d err3 = _mm_setzero_pd();
        __m128d err4 = _mm_setzero_pd();

        __m128d y, t;

        for (; i < nUnrolledSize; i += 8)
        {
            // Kahan sum 1
            __m128d load1 = _mm_load_pd(pCurrent);
            y = _mm_sub_pd(load1, err1);
            t = _mm_add_pd(sum1, y);
            err1 = _mm_sub_pd(_mm_sub_pd(t, sum1), y);
            sum1 = t;
            pCurrent += 2;

            // Kahan sum 2
            __m128d load2 = _mm_load_pd(pCurrent);
            y = _mm_sub_pd(load2, err2);
            t = _mm_add_pd(sum2, y);
            err2 = _mm_sub_pd(_mm_sub_pd(t, sum2), y);
            sum2 = t;
            pCurrent += 2;

            // Kahan sum 3
            __m128d load3 = _mm_load_pd(pCurrent);
            y = _mm_sub_pd(load3, err3);
            t = _mm_add_pd(sum3, y);
            err3 = _mm_sub_pd(_mm_sub_pd(t, sum3), y);
            sum3 = t;
            pCurrent += 2;

            // Kahan sum 4
            __m128d load4 = _mm_load_pd(pCurrent);
            y = _mm_sub_pd(load4, err4);
            t = _mm_add_pd(sum4, y);
            err4 = _mm_sub_pd(_mm_sub_pd(t, sum4), y);
            sum4 = t;
            pCurrent += 2;
        }

        // Now we combine pairwise summation with Kahan summation

        // sum 1 + sum 2
        y = _mm_sub_pd(sum2, err1);
        t = _mm_add_pd(sum1, y);
        err1 = _mm_sub_pd(_mm_sub_pd(t, sum1), y);
        sum1 = t;

        // sum 3 + sum 4
        y = _mm_sub_pd(sum4, err3);
        t = _mm_add_pd(sum3, y);
        sum3 = t;

        // sum 1 + sum 3
        y = _mm_sub_pd(sum3, err1);
        t = _mm_add_pd(sum1, y);
        sum1 = t;

        double temp;

        _mm_storel_pd(&temp, sum1);
        fSum += temp;

        _mm_storeh_pd(&temp, sum1);
        fSum += temp;
    }
    return fSum;
#else
    (void)i;
    (void)pCurrent;
    return 0.0;
#endif
}
}
