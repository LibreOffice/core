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

#if defined(LO_SSSE3_AVAILABLE)
#include <tmmintrin.h>
#endif

namespace vcl {
namespace scanline {

#if defined(LO_SSSE3_AVAILABLE)
bool swapABCDtoCBAD_SSSE3(sal_uInt8* pSource, sal_uInt8* pDestination, sal_Int32 nScanlineSize)
{
    __m128i aShuffleMask = _mm_set_epi8(15, 12, 13, 14, 11, 8, 9, 10, 7, 4, 5, 6, 3, 0, 1, 2);

    sal_Int32 nBlocks = nScanlineSize / 16;

    if (nBlocks > 0)
    {
        __m128i* pSource128      = reinterpret_cast<__m128i*>(pSource);
        __m128i* pDestination128 = reinterpret_cast<__m128i*>(pDestination);

        for (sal_Int32 x = 0; x < nBlocks; ++x, ++pDestination128, ++pSource128)
        {
            // _mm_lddqu_si128 - faster unaligned load with ssse3
            _mm_storeu_si128(pDestination128, _mm_shuffle_epi8(_mm_lddqu_si128(pSource128), aShuffleMask));
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
bool swapABCDtoCBAD_SSSE3(sal_uInt8* pSource, sal_uInt8* pDestination, sal_Int32 nScanlineSize)
{
    return false;
}
#endif
}} // end vcl::scanline

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
