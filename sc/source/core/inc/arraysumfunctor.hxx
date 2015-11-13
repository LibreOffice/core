/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_SC_SOURCE_CORE_INC_ARRAYSUMFUNCTOR_HXX
#define INCLUDED_SC_SOURCE_CORE_INC_ARRAYSUMFUNCTOR_HXX

#include <tools/cpuid.hxx>

#if defined(__LO_SSE2_AVAILABLE__)
#include <emmintrin.h>
#endif

namespace sc
{

template<typename T, unsigned int N>
inline bool isAligned(const T* pointer)
{
    return 0 == (uintptr_t(pointer) % N);
}

struct ArraySumFunctor
{
private:
    const double* mpArray;
    size_t mnSize;

public:
    ArraySumFunctor(const double* pArray, size_t nSize)
        : mpArray(pArray)
        , mnSize(nSize)
    {
    }

    double operator() ()
    {
        static bool hasSSE2 = tools::cpuid::hasSSE2();

        double fSum = 0.0;
        size_t i = 0;
        const double* pCurrent = mpArray;

        if (hasSSE2)
        {
            while (!isAligned<double, 16>(pCurrent))
            {
                fSum += *pCurrent++;
                i++;
            }
            fSum += executeSSE2(i, pCurrent);
        }
        else
            fSum += executeUnrolled(i, pCurrent);

        // sum rest of the array

        for (; i < mnSize; ++i)
            fSum += mpArray[i];

        return fSum;
    }

private:
    inline double executeSSE2(size_t& i, const double* pCurrent) const
    {
#if defined(__LO_SSE2_AVAILABLE__)
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
        (void) i;
        (void) pCurrent;
        return 0.0;
#endif
    }

    inline double executeUnrolled(size_t& i, const double* pCurrent) const
    {
        size_t nRealSize = mnSize - i;
        size_t nUnrolledSize = nRealSize - (nRealSize % 4);

        if (nUnrolledSize > 0)
        {
            double sum0 = 0.0;
            double sum1 = 0.0;
            double sum2 = 0.0;
            double sum3 = 0.0;

            for (; i < nUnrolledSize; i += 4)
            {
                sum0 += *pCurrent++;
                sum1 += *pCurrent++;
                sum2 += *pCurrent++;
                sum3 += *pCurrent++;
            }
            return sum0 + sum1 + sum2 + sum3;
        }
        return 0.0;
    }
};

} // end namespace sc

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
