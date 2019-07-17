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

#include <cstdint>
#include <rtl/math.hxx>

#include <tools/simdsupport.hxx>
#include <tools/simd.hxx>
#include <tools/cpuid.hxx>

namespace sc
{

struct ArraySumFunctor
{
private:
    const double* mpArray;
    size_t const mnSize;

public:
    ArraySumFunctor(const double* pArray, size_t nSize)
        : mpArray(pArray)
        , mnSize(nSize)
    {
    }

    double operator() ()
    {
        const static bool hasSSE2 = cpuid::hasSSE2();

        double fSum = 0.0;
        size_t i = 0;
        const double* pCurrent = mpArray;

        if (hasSSE2)
        {
            while ( i < mnSize && !simd::isAligned<double, 16>(pCurrent))
            {
                fSum += *pCurrent++;
                i++;
            }
            if( i < mnSize )
            {
                fSum += executeSSE2(i, pCurrent);
            }
        }
        else
            fSum += executeUnrolled(i, pCurrent);

        // sum rest of the array

        for (; i < mnSize; ++i)
            fSum += mpArray[i];

        // If the sum is a NaN, some of the terms were empty cells, probably.
        // Re-calculate, carefully
        if (!rtl::math::isFinite(fSum))
        {
            sal_uInt32 nErr = reinterpret_cast< sal_math_Double * >(&fSum)->nan_parts.fraction_lo;
            if (nErr & 0xffff0000)
            {
                fSum = 0;
                for (i = 0; i < mnSize; i++)
                {
                    if (!rtl::math::isFinite(mpArray[i]))
                    {
                        nErr = reinterpret_cast< const sal_math_Double * >(&mpArray[i])->nan_parts.fraction_lo;
                        if (!(nErr & 0xffff0000))
                            fSum += mpArray[i]; // Let errors encoded as NaNs propagate ???
                    }
                    else
                        fSum += mpArray[i];
                }
            }
        }
        return fSum;
    }

private:
    double executeSSE2(size_t& i, const double* pCurrent) const
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
        (void) i;
        (void) pCurrent;
        return 0.0;
#endif
    }

    double executeUnrolled(size_t& i, const double* pCurrent) const
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
