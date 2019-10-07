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

        for (; i < nUnrolledSize; i += 8)
        {
            __m128d load1 = _mm_load_pd(pCurrent);
            sum1 = _mm_add_pd(sum1, load1);
            pCurrent += 2;

            __m128d load2 = _mm_load_pd(pCurrent);
            sum2 = _mm_add_pd(sum2, load2);
            pCurrent += 2;

            __m128d load3 = _mm_load_pd(pCurrent);
            sum3 = _mm_add_pd(sum3, load3);
            pCurrent += 2;

            __m128d load4 = _mm_load_pd(pCurrent);
            sum4 = _mm_add_pd(sum4, load4);
            pCurrent += 2;
        }
        sum1 = _mm_add_pd(_mm_add_pd(sum1, sum2), _mm_add_pd(sum3, sum4));

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