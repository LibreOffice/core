/*
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKIA_OPTS_H
#define SKIA_OPTS_H

#include <include/core/SkTypes.h>

SK_API void SkConvertRGBToRGBA(uint32_t* dest, const uint8_t* src, int count);

SK_API void SkConvertGrayToRGBA(uint32_t* dest, const uint8_t* src, int count);

SK_API void SkConvertRGBAToRGB(uint8_t* dest, const uint32_t* src, int count);

SK_API void SkConvertRGBAToGrayFast(uint8_t* dest, const uint32_t* src, int count);

namespace SkLoOpts
{
SK_API void Init();

typedef void (*Swizzle_u8_8888)(uint8_t*, const uint32_t*, int);
extern Swizzle_u8_8888 RGB1_to_RGB,     // i.e. remove an (opaque) alpha
                       RGB1_to_gray_fast;    // i.e. copy one channel to the result
}

#endif
