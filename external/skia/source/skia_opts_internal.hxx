/*
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKIA_OPTS_INTERNAL_H
#define SKIA_OPTS_INTERNAL_H

#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSSE3
    #include <immintrin.h>
#endif

namespace SK_OPTS_NS {

static void RGB1_to_RGB_portable(uint8_t dst[], const uint32_t* src, int count) {
    for (int i = 0; i < count; i++) {
        dst[0] = src[i] >> 0;
        dst[1] = src[i] >> 8;
        dst[2] = src[i] >> 16;
        dst += 3;
    }
}
static void RGB1_to_gray_fast_portable(uint8_t dst[], const uint32_t* src, int count) {
    for (int i = 0; i < count; i++) {
        dst[i] = src[i] & 0xFF;
    }
}

#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSSE3
inline void RGB1_to_RGB(uint8_t dst[], const uint32_t* src, int count) {
        const uint8_t X = 0xFF; // Used a placeholder.  The value of X is irrelevant.
        __m128i pack = _mm_setr_epi8(0,1,2, 4,5,6, 8,9,10, 12,13,14, X,X,X,X);

// Storing 4 pixels should store 12 bytes, but here it stores 16, so test count >= 6
// in order to not overrun the output buffer.
        while (count >= 6) {
            __m128i rgba = _mm_loadu_si128((const __m128i*) src);

            __m128i rgb = _mm_shuffle_epi8(rgba, pack);

            // Store 4 pixels.
            _mm_storeu_si128((__m128i*) dst, rgb);

            src += 4;
            dst += 4*3;
            count -= 4;
        }
        RGB1_to_RGB_portable(dst, src, count);
}

inline void RGB1_to_gray_fast(uint8_t dst[], const uint32_t* src, int count) {
        const uint8_t X = 0xFF; // Used a placeholder.  The value of X is irrelevant.
        __m128i pack = _mm_setr_epi8(0,4,8,12, X,X,X,X,X,X,X,X,X,X,X,X);

// Storing 4 pixels should store 4 bytes, but here it stores 16, so test count >= 16
// in order to not overrun the output buffer.
        while (count >= 16) {
            __m128i rgba = _mm_loadu_si128((const __m128i*) src);

            __m128i rgb = _mm_shuffle_epi8(rgba, pack);

            // Store 4 pixels.
            _mm_storeu_si128((__m128i*) dst, rgb);

            src += 4;
            dst += 4;
            count -= 4;
        }
        RGB1_to_gray_fast_portable(dst, src, count);
}

#else
inline void RGB1_to_RGB(uint8_t dst[], const uint32_t* src, int count) {
    RGB1_to_RGB_portable(dst, src, count);
}
inline void RGB1_to_gray_fast(uint8_t dst[], const uint32_t* src, int count) {
    RGB1_to_gray_fast_portable(dst, src, count);
}
#endif

} // namespace

#endif
