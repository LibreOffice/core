/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

// Declarations only, with no inline code, so a file built with AVX2 can include this header.

#include <tools/simdplatform.hxx>
#include <vcl/dllapi.h>

#include <cstdint>

// Row kernels for scaling premultiplied 32-bit pixels. All 4 channels are treated the same.
// The weights of one destination pixel sum to 1 << constWeightShift, and the results are rounded
// and clamped to 0..255. The AVX2 versions give the same bytes as the scalar ones.
namespace vcl::bitmap::convolution
{
constexpr int32_t constWeightShift = 14;

// Horizontal pass. Destination pixel x is the weighted sum of nWindow source pixels from
// pStarts[x], with weights from pWeights[x * windowWeightCount(nWindow)].
VCL_DLLPUBLIC void convolveRowN32(const uint8_t* pSource, uint8_t* pDestination,
                                  int32_t nDestinationWidth, const int16_t* pWeights,
                                  const int32_t* pStarts, int32_t nWindow);

// The number of weights stored for one destination pixel.
VCL_DLLPUBLIC int32_t windowWeightCount(int32_t nWindow);

// Stores the taps in groups of 4: taps 0 and 1 as a pair repeated 4 times, then taps 2 and 3 the
// same way. Taps past nWindow are zero.
VCL_DLLPUBLIC void spreadWindowWeights(const int16_t* pTaps, int32_t nWindow, int16_t* pWeights);

// Vertical pass. Each destination byte is the sum of the same byte in each source row j times
// pWeights[j].
VCL_DLLPUBLIC void blendRowsN32(const uint8_t* const* pSourceRows, const int16_t* pWeights,
                                int32_t nRowCount, uint8_t* pDestination, int32_t nWidth);

// Averages 2x2 pixel blocks of the 2 rows in pSourceRows.
VCL_DLLPUBLIC void boxHalveRowN32(const uint8_t* const* pSourceRows, uint8_t* pDestination,
                                  int32_t nDestinationWidth);

// Averages 4x4 pixel blocks of the 4 rows in pSourceRows.
VCL_DLLPUBLIC void boxQuarterRowN32(const uint8_t* const* pSourceRows, uint8_t* pDestination,
                                    int32_t nDestinationWidth);

// Converts 24-bit pixels to opaque 32-bit pixels with alpha last.
VCL_DLLPUBLIC void expandRowN24ToN32(const uint8_t* pSource, uint8_t* pDestination, int32_t nWidth,
                                     bool bSwapRedBlue);

// Converts 32-bit pixels with alpha last to 24-bit pixels.
VCL_DLLPUBLIC void packRowN32ToN24(const uint8_t* pSource, uint8_t* pDestination, int32_t nWidth);

#ifdef LO_X86_SIMD_AVAILABLE

// AVX2 versions, for CPUs that support AVX2.

// nWindow is a multiple of 4.
VCL_DLLPUBLIC void convolveRowN32AVX2(const uint8_t* pSource, uint8_t* pDestination,
                                      int32_t nDestinationWidth, const int16_t* pWeights,
                                      const int32_t* pStarts, int32_t nWindow);

VCL_DLLPUBLIC void blendRowsN32AVX2(const uint8_t* const* pSourceRows, const int16_t* pWeights,
                                    int32_t nRowCount, uint8_t* pDestination, int32_t nWidth);

VCL_DLLPUBLIC void boxHalveRowN32AVX2(const uint8_t* const* pSourceRows, uint8_t* pDestination,
                                      int32_t nDestinationWidth);

VCL_DLLPUBLIC void boxQuarterRowN32AVX2(const uint8_t* const* pSourceRows, uint8_t* pDestination,
                                        int32_t nDestinationWidth);

VCL_DLLPUBLIC void expandRowN24ToN32AVX2(const uint8_t* pSource, uint8_t* pDestination,
                                         int32_t nWidth, bool bSwapRedBlue);

VCL_DLLPUBLIC void packRowN32ToN24AVX2(const uint8_t* pSource, uint8_t* pDestination,
                                       int32_t nWidth);

#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
