/*
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <skia_opts.hxx>

#if defined __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#endif
#include "src/core/SkCpu.h"
#include "src/core/SkOpts.h"
#if defined __GNUC__
#pragma GCC diagnostic pop
#endif

void SkConvertRGBToRGBA(uint32_t* dest, const uint8_t* src, int count)
{
    SkOpts::RGB_to_RGB1(dest, src, count);
}

void SkConvertGrayToRGBA(uint32_t* dest, const uint8_t* src, int count)
{
    SkOpts::gray_to_RGB1(dest, src, count);
}

void SkConvertRGBAToRGB(uint8_t* dest, const uint32_t* src, int count)
{
    SkLoOpts::RGB1_to_RGB(dest, src, count);
}

void SkConvertRGBAToR(uint8_t* dest, const uint32_t* src, int count)
{
    SkLoOpts::RGB1_to_R(dest, src, count);
}

// The rest is mostly based on Skia's SkOpts.cpp, reduced to only SSSE3 so far.

#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSSE3
    #define SK_OPTS_NS ssse3
#else
    #define SK_OPTS_NS portable
#endif

#include "skia_opts_internal.hxx"

namespace SkLoOpts {
    // Define default function pointer values here...
    // If our global compile options are set high enough, these defaults might even be
    // CPU-specialized, e.g. a typical x86-64 machine might start with SSE2 defaults.
    // They'll still get a chance to be replaced with even better ones, e.g. using SSE4.1.
#define DEFINE_DEFAULT(name) decltype(name) name = SK_OPTS_NS::name
    DEFINE_DEFAULT(RGB1_to_RGB);
    DEFINE_DEFAULT(RGB1_to_R);
#undef DEFINE_DEFAULT

    // Each Init_foo() is defined in its own file.
    void Init_ssse3();

    static void init() {
#if !defined(SK_BUILD_NO_OPTS)
    #if defined(SK_CPU_X86)
        #if SK_CPU_SSE_LEVEL < SK_CPU_SSE_LEVEL_SSSE3
            if (SkCpu::Supports(SkCpu::SSSE3)) { Init_ssse3(); }
        #endif
    #endif
#endif
    }

    void Init() {
        static SkOnce once;
        once(init);
    }
}  // namespace SkLoOpts
