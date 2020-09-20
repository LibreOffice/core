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

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include "test_cpu_runtime_detection_x86_checks.hxx"

/* WARNING: This file is compiled with AVX2 support, don't call
 * any function without checking cpuid to check the CPU can actually
 * handle it.
 */
void CpuRuntimeDetectionX86Checks::checkAVX2()
{
#ifdef LO_AVX2_AVAILABLE
    __m256i a = _mm256_set_epi64x(1, 4, 8, 3);
    __m256i b = _mm256_set_epi64x(2, 1, 1, 5);
    __m256i c = _mm256_xor_si256(a, b);

    sal_Int64 values[4];
    _mm256_storeu_si256(reinterpret_cast<__m256i*>(&values), c);

    CPPUNIT_ASSERT_EQUAL(sal_Int64(6), values[0]);
    CPPUNIT_ASSERT_EQUAL(sal_Int64(9), values[1]);
    CPPUNIT_ASSERT_EQUAL(sal_Int64(5), values[2]);
    CPPUNIT_ASSERT_EQUAL(sal_Int64(3), values[3]);

    __m256i d = _mm256_set_epi64x(3, 5, 1, 0);

    __m256i result = _mm256_cmpeq_epi64(d, c);

    // Compare equals
    sal_Int64 compare[4];
    _mm256_storeu_si256(reinterpret_cast<__m256i*>(&compare), result);

    CPPUNIT_ASSERT_EQUAL(sal_Int64(0), compare[0]);
    CPPUNIT_ASSERT_EQUAL(sal_Int64(0), compare[1]);
    CPPUNIT_ASSERT_EQUAL(sal_Int64(-1), compare[2]);
    CPPUNIT_ASSERT_EQUAL(sal_Int64(-1), compare[3]);
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
