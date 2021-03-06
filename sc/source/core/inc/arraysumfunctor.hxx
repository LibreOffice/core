/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <cstdint>
#include <cmath>

#include <sal/mathconf.h>
#include <sal/types.h>
#include <tools/simd.hxx>
#include <tools/cpuid.hxx>

namespace sc
{
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

    double operator()()
    {
        const static bool hasSSE2 = cpuid::hasSSE2();

        double fSum = 0.0;
        size_t i = 0;
        const double* pCurrent = mpArray;

        if (hasSSE2)
        {
            while (i < mnSize && !simd::isAligned<double, 16>(pCurrent))
            {
                fSum += *pCurrent++;
                i++;
            }
            if (i < mnSize)
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
        if (!std::isfinite(fSum))
        {
            sal_uInt32 nErr = reinterpret_cast<sal_math_Double*>(&fSum)->nan_parts.fraction_lo;
            if (nErr & 0xffff0000)
            {
                fSum = 0;
                for (i = 0; i < mnSize; i++)
                {
                    if (!std::isfinite(mpArray[i]))
                    {
                        nErr = reinterpret_cast<const sal_math_Double*>(&mpArray[i])
                                   ->nan_parts.fraction_lo;
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
    double executeSSE2(size_t& i, const double* pCurrent) const;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
