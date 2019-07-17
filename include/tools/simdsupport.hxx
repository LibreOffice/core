/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

// Determine the compiler support for SIMD compiler intrinsics.
// This changes from one compiled unit to the other, depending if
// the support has been detected and if the compiled unit contains
// code using intrinsics or not. So we have to (re)set them again
// every time this file has been included.

#undef LO_SSE2_AVAILABLE
#undef LO_SSSE3_AVAILABLE
#undef LO_AVX_AVAILABLE
#undef LO_AVX2_AVAILABLE

#if defined(_MSC_VER) // VISUAL STUDIO COMPILER

// SSE2 is required for X64
#if (defined(_M_X64) || defined(_M_IX86_FP) && _M_IX86_FP >= 2)
#define LO_SSE2_AVAILABLE
#endif // end SSE2

// compiled with /arch:AVX
#if defined(__AVX__)
#ifndef LO_SSE2_AVAILABLE
#define LO_SSE2_AVAILABLE
#endif
#define LO_SSSE3_AVAILABLE
#define LO_AVX_AVAILABLE
#endif // defined(__AVX__)

// compiled with /arch:AVX2
#if defined(__AVX2__)
#define LO_AVX2_AVAILABLE
#endif // defined(__AVX2__)

#else // compiler Clang and GCC

#if defined(__SSE2__) || defined(__x86_64__) // SSE2 is required for X64
#define LO_SSE2_AVAILABLE
#endif // defined(__SSE2__)

#if defined(__SSSE3__)
#define LO_SSSE3_AVAILABLE
#endif // defined(__SSSE3__)

#if defined(__AVX__)
#define LO_AVX_AVAILABLE
#endif // defined(__AVX__)

#if defined(__AVX2__)
#define LO_AVX2_AVAILABLE
#endif // defined(__AVX2__)

#endif // end compiler Clang and GCC

// If we detect any SIMD intrinsics, include the headers automatically
#if defined(LO_SSE2_AVAILABLE)
#include <emmintrin.h>
#elif defined(LO_SSSE3_AVAILABLE)
#include <tmmintrin.h>
#elif defined(LO_AVX_AVAILABLE) || defined(LO_AVX2_AVAILABLE)
#include <immintrin.h>
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
