/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/types.h>
#include <tools/simdsupport.hxx>

#include <stdlib.h>

#include "test_cpu_runtime_detection_x86_checks.hxx"

/* WARNING: This file is compiled with SSE2 support, don't call
 * any function without checking cpuid to check the CPU can actually
 * handle it, and don't include any headers that contain templates
 * or inline functions, which includes cppunit.
 */
#define CPPUNIT_ASSERT_EQUAL(a, b) ((a) == (b) ? (void)0 : abort())
void CpuRuntimeDetectionX86Checks::checkSSE2()
{
#ifdef LO_SSE2_AVAILABLE
    // Try some SSE2 intrinsics calculation
    __m128i a = _mm_set1_epi32(15);
    __m128i b = _mm_set1_epi32(15);
    __m128i c = _mm_xor_si128(a, b);

    // Check zero
    CPPUNIT_ASSERT_EQUAL(0xFFFF, _mm_movemask_epi8(_mm_cmpeq_epi32(c, _mm_setzero_si128())));
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
