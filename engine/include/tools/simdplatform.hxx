/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

// Defined on x86/x64 platforms where SIMD-optimized code is compiled and available
// for runtime dispatch via cpuid.
#if defined(__SSE2__) || defined(__x86_64__)                                                       \
    || (defined(_MSC_VER) && (defined(_M_X64) || defined(_M_X86)))
#define LO_X86_SIMD_AVAILABLE
#endif

// Force inlining of SIMD helper functions called in tight loops, ensuring they
// are inlined even in debug builds where the compiler would otherwise skip it.
#if defined(_MSC_VER)
#define LO_FORCE_INLINE __forceinline static
#elif defined(__GNUC__) || defined(__clang__)
#define LO_FORCE_INLINE __attribute__((always_inline)) static inline
#else
#define LO_FORCE_INLINE static inline
#endif

// Symbol export for SIMD functions, which can't include <sal/types.h>
#if defined(_MSC_VER)
#define LO_DLLPUBLIC_EXPORT __declspec(dllexport)
#elif defined(__GNUC__)
#define LO_DLLPUBLIC_EXPORT __attribute__((visibility("default")))
#else
#define LO_DLLPUBLIC_EXPORT
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
