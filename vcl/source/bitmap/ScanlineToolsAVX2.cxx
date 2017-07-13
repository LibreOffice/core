/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include "ScanlineTools.hxx"

#include <tools/simdsupport.hxx>

#if defined(LO_AVX2_AVAILABLE)
#include <immintrin.h>
#endif

namespace vcl {
namespace scanline {

#if defined(LO_AVX2_AVAILABLE)
bool swapABCDtoCBAD_AVX2(sal_uInt8* pSource, sal_uInt8* pDestination, sal_Int32 nScanlineSize)
{
    __m256i aShuffleMask = _mm256_set_epi8(31, 28, 29, 30, 27, 24, 25, 26,
                                           23, 20, 21, 22, 19, 16, 17, 18,
                                           15, 12, 13, 14, 11,  8,  9, 10,
                                            7,  4,  5,  6,  3,  0,  1,  2);

    sal_Int32 nBlocks = nScanlineSize / 32;

    if (nBlocks > 0)
    {
        __m256i* pSource256      = reinterpret_cast<__m256i*>(pSource);
        __m256i* pDestination256 = reinterpret_cast<__m256i*>(pDestination);

        for (sal_Int32 x = 0; x < nBlocks; ++x, ++pDestination256, ++pSource256)
        {
            _mm256_storeu_si256(pDestination256, _mm256_shuffle_epi8(_mm256_loadu_si256(pSource256), aShuffleMask));
        }
    }

    pSource += nBlocks * 32;
    pDestination += nBlocks * 32;

    for (sal_Int32 i = nBlocks * 32; i < nScanlineSize; i += 4, pSource += 4, pDestination += 4)
    {
        pDestination[0] = pSource[2];
        pDestination[1] = pSource[1];
        pDestination[2] = pSource[0];
        pDestination[3] = pSource[3];
    }
    return true;
}
#else
bool swapABCDtoCBAD_AVX2(sal_uInt8* pSource, sal_uInt8* pDestination, sal_Int32 nScanlineSize)
{
    return false;
}
#endif

}} // end vcl::scanline

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
