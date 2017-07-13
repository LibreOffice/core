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

#if defined(LO_SSE2_AVAILABLE)
#include <emmintrin.h>
#endif

namespace vcl {
namespace scanline {

#if defined(LO_SSE2_AVAILABLE)
bool swapABCDtoCBAD_SSE2(sal_uInt8* pSource, sal_uInt8* pDestination, sal_Int32 nScanlineSize)
{

    sal_Int32 nBlocks = nScanlineSize / 16;

    if (nBlocks > 0)
    {
        __m128i* pSource128      = reinterpret_cast<__m128i*>(pSource);
        __m128i* pDestination128 = reinterpret_cast<__m128i*>(pDestination);

        __m128i agmask = _mm_set1_epi32(0xFF00FF00);

        for (sal_Int32 x = 0; x < nBlocks; ++x, ++pDestination128, ++pSource128)
        {
            // RGBA RGBA RGBA RGBA
            __m128i rgba = _mm_loadu_si128(pSource128);

            // 0G0A 0G0A 0G0A 0G0A
            __m128i ag = _mm_and_si128(agmask, rgba);
            // R0B0 R0B0 R0B0 R0B0
            __m128i rb = _mm_andnot_si128(agmask, rgba);

            // Swap R and B
            // B0R0 B0R0 B0R0 B0R0
            __m128i br = _mm_shufflehi_epi16(_mm_shufflelo_epi16(rb, _MM_SHUFFLE(2, 3, 0, 1)), _MM_SHUFFLE(2, 3, 0, 1));

            //     B0R0 B0R0 B0R0 B0R0
            // or  0G0A 0G0A 0G0A 0G0A
            _mm_storeu_si128(pDestination128, _mm_or_si128(ag, br));
        }
    }

    pSource += nBlocks * 16;
    pDestination += nBlocks * 16;

    for (sal_Int32 i = nBlocks * 16; i < nScanlineSize; i += 4, pSource += 4, pDestination += 4)
    {
        pDestination[0] = pSource[2];
        pDestination[1] = pSource[1];
        pDestination[2] = pSource[0];
        pDestination[3] = pSource[3];
    }
    return true;
}
#else
bool swapABCDtoCBAD_SSE2(sal_uInt8* pSource, sal_uInt8* pDestination, sal_Int32 nScanlineSize)
{
    return false;
}
#endif

}} // end vcl::scanline

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
