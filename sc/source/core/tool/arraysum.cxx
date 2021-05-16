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
inline void sumAVX64(double& sum, double& err, const double& value)
{
    double t = sum + value;
    if (std::abs(sum) > std::abs(value))
        err += (sum - t) + value;
    else
        err += (value - t) + sum;
    sum = t;
}

//#ifdef LO_AVX512F_AVAILABLE // New processors

/** Kahan sum with AVX512.
  */
inline void sumAVX512(__m512d& sum, __m512d& err, const __m512d& value)
{
    // Temporal parameter
    __m512d t = _mm512_add_pd(sum, value);
    // Absolute value of the total sum
    __m512d asum = _mm512_abs_pd(sum);
    // Absolute value of the value to add
    __m512d avalue = _mm512_abs_pd(value);
    // Comaprate the absolute values sum >= value
    __mmask8 mask = _mm512_cmp_pd_mask(asum, avalue, _CMP_GE_OQ);
    // The following code has this form ( a - t + b)
    // Case 1: a = sum b = value
    // Case 2: a = value b = sum
    __m512d a = _mm512_mask_blend_pd(mask, sum, value);
    __m512d b = _mm512_mask_blend_pd(mask, value, sum);
    err = _mm512_add_pd(err, _mm512_add_pd(_mm512_sub_pd(a, t), b)),
    // Store result
        sum = t;
}

/** Execute Kahan sum with AVX512.
  */
inline KahanSum executeAVX512(size_t& i, size_t nSize, const double* pCurrent)
{
    size_t nRealSize = nSize - i;
    size_t nUnrolledSize = nRealSize - (nRealSize % 8);
    if (nUnrolledSize > 0)
    {
        // Setup sums and errors as 0
        __m512d sum = _mm512_setzero_pd();
        __m512d err = _mm512_setzero_pd();

        // Sum the stuff
        for (; i < nUnrolledSize; i += 8)
        {
            // Kahan sum
            __m512d load = _mm512_load_pd(pCurrent);
            sumAVX512(sum, err, load);
            pCurrent += 8;
        }

        // Store result
        double* sums = new double[8];
        double* errs = new double[8];
        _mm512_store_pd(reinterpret_cast<void*>(sums), sum);
        _mm512_store_pd(reinterpret_cast<void*>(errs), err);

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
        sumAVX64(sums[4], errs[0], sums[6]);
        sumAVX64(sums[0], errs[0], errs[2]);
        sumAVX64(sums[4], errs[0], errs[6]);

        // Thirde Kahan & pairwise summation
        // 0+4 -> 0
        sumAVX64(sums[0], errs[0], sums[4]);
        sumAVX64(sums[0], errs[0], errs[4]);

        // Return final result
        return KahanSum(sums[0], errs[0]);
    }
    else
        return 0.0;
}
//#endif

// #ifdef LO_SSE2_AVAILABLE // Old processors

static const __m128d ANNULATE_SIGN_BIT = _mm_castsi128_pd(_mm_set1_epi64x(0x7FFF'FFFF'FFFF'FFFF));

/** Kahan sum with SSE2.
  */
inline void sumSSE2(__m128d& sum, __m128d& err, const __m128d& value)
{
    // Temporal parameter
    __m128d t = _mm_add_pd(sum, value);
    // Absolute value of the total sum
    __m128d asum = _mm_and_pd(sum, ANNULATE_SIGN_BIT);
    // Absolute value of the value to add
    __m128d avalue = _mm_and_pd(value, ANNULATE_SIGN_BIT);
    // Comaprate the absolute values sum >= value
    __m128d mask = _mm_cmpge_pd(asum, avalue);
    // The following code has this form ( a - t + b)
    // Case 1: a = sum b = value
    // Case 2: a = value b = sum
    __m128d a = _mm_add_pd(_mm_mul_pd(mask, sum), _mm_mul_pd(mask, value));
    __m128d b = _mm_add_pd(_mm_mul_pd(mask, value), _mm_mul_pd(mask, sum));
    err = _mm_add_pd(err, _mm_add_pd(_mm_sub_pd(a, t), b)),
    // Store result
        sum = t;
}

/** Execute Kahan sum with SSE2.
  */
inline KahanSum executeSSE2(size_t& i, size_t nSize, const double* pCurrent)
{
    size_t nRealSize = nSize - i;
    size_t nUnrolledSize = nRealSize - (nRealSize % 8);

    if (nUnrolledSize > 0)
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

        for (; i < nUnrolledSize; i += 8)
        {
            // Kahan sum 1
            __m128d load1 = _mm_load_pd(pCurrent);
            sumSSE2(sum1, err1, load1);
            pCurrent += 2;

            // Kahan sum 2
            __m128d load2 = _mm_load_pd(pCurrent);
            sumSSE2(sum2, err2, load2);
            pCurrent += 2;

            // Kahan sum 3
            __m128d load3 = _mm_load_pd(pCurrent);
            sumSSE2(sum3, err3, load3);
            pCurrent += 2;

            // Kahan sum 1
            __m128d load4 = _mm_load_pd(pCurrent);
            sumSSE2(sum4, err4, load4);
            pCurrent += 2;
        }

        // Now we combine pairwise summation with Kahan summation

        // sum 1 + sum 2
        sumSSE2(sum1, err1, sum2);
        sumSSE2(sum1, err1, err2);

        // sum 3 + sum 4
        sumSSE2(sum3, err3, sum3);
        sumSSE2(sum3, err3, err3);

        // sum 1 + sum 3
        sumSSE2(sum1, err1, sum3);
        sumSSE2(sum1, err1, err3);

        // Store result
        KahanSum ksum1, ksum2;
        (_mm_storeh_pd(ksum1.getSum(), sum1), _mm_storeh_pd(ksum1.getError(), err1));
        (_mm_storel_pd(ksum1.getSum(), sum1), _mm_storel_pd(ksum1.getError(), err1));
        return ksum1 + ksum2;
    }
    return 0.0;
}
//#endif

// Even older processors
/** If no boosts available, Unrolled KahanSum.
  */
KahanSum executeUnrolled(size_t& i, size_t nSize, const double* pCurrent)
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
const static bool hasSSE2 = cpuid::hasSSE2();
const static bool hasFast = hasAVX512 || hasSSE2;

KahanSum executeFast(size_t& i, size_t nSize, const double* pCurrent)
{
    //#ifdef LO_AVX512F_AVAILABLE
    if (hasAVX512)
        return sc::op::executeAVX512(i, nSize, pCurrent);
    //#endif
    //#ifdef LO_SSE2_AVAILABLE
    if (hasSSE2)
        return sc::op::executeSSE2(i, nSize, pCurrent);
    //#endif
    return sc::op::executeUnrolled(i, nSize, pCurrent);
}

} // end namespace sc::op

KahanSum sc::op::sumArray(const double* pArray, size_t nSize)
{
    size_t i = 0;
    KahanSum fSum = 0.0;
    const double* pCurrent = pArray;

    if (hasFast)
    {
        while (i < nSize && !simd::isAligned<double, 16>(pCurrent))
        {
            fSum += *pCurrent++;
            i++;
        }
        if (i < nSize)
        {
            fSum += executeFast(i, nSize, pCurrent);
        }
    }
    else
        fSum = executeUnrolled(i, nSize, pCurrent);

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
