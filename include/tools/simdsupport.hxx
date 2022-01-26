/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

// IMPORTANT: Having CPU-specific routines turned out to be a maintenance
// problem, because of various problems such as compilers moving CPU-specific
// code out of #ifdef code into static initialization or our code using C++
// features that caused the compiler to emit code that used CPU-specific
// instructions (even cpuid.hxx isn't safe, see the comment there).
// The only safe usage is using CPU-specific code that's always available,
// such as SSE2-specific code for x86_64. Do not use for anything else
// unless you really know what you are doing (and you check git history
// to learn from past problems).

// Determine the compiler support for SIMD compiler intrinsics.
// This changes from one compiled unit to the other, depending if
// the support has been detected and if the compiled unit contains
// code using intrinsics or not. So we have to (re)set them again
// every time this file has been included.

// In other words... DO NOT ADD "#pragma once" here

#undef LO_SSE2_AVAILABLE
#undef LO_SSSE3_AVAILABLE
#undef LO_AVX_AVAILABLE
#undef LO_AVX2_AVAILABLE
#undef LO_AVX512F_AVAILABLE

#if defined(_MSC_VER) // VISUAL STUDIO COMPILER

// With MSVC using -arch is in fact not necessary for being able
// to use CPU intrinsics, code using AVX512F intrinsics will compile
// even if compiled with -arch:AVX, the -arch option really only affects
// instructions generated for C/C++ code.
#if defined(_M_X64) || defined(_M_X86)
// As such, if we're building for X86 or X64, support for these is always available
// with MSVC2019+.
#define LO_SSE2_AVAILABLE
#define LO_SSSE3_AVAILABLE
#define LO_AVX_AVAILABLE
#define LO_AVX2_AVAILABLE
#define LO_AVX512F_AVAILABLE
#include <intrin.h>
#include <immintrin.h>
#endif

#else // compiler Clang and GCC

#if defined(__SSE2__) || defined(__x86_64__) // SSE2 is required for X64
#define LO_SSE2_AVAILABLE
#include <emmintrin.h>
#endif // defined(__SSE2__)

#if defined(__SSSE3__)
#define LO_SSSE3_AVAILABLE
#include <tmmintrin.h>
#endif // defined(__SSSE3__)

#if defined(__AVX__)
#define LO_AVX_AVAILABLE
#include <immintrin.h>
#endif // defined(__AVX__)

#if defined(__AVX2__)
#define LO_AVX2_AVAILABLE
#include <immintrin.h>
#endif // defined(__AVX2__)

#if defined(__AVX512F__)
#define LO_AVX512F_AVAILABLE
#include <immintrin.h>
#else
#endif // defined(__AVX512F__)

#endif // end compiler Clang and GCC

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
