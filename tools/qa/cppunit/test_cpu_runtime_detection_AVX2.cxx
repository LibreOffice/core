/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <tools/simdsupport.hxx>

#ifdef LO_AVX2_AVAILABLE

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <tools/cpuid.hxx>

namespace
{
class CpuRuntimeDetection_AVX2 : public CppUnit::TestFixture
{
public:
    void checkAVX2();
    void testCpuRuntimeDetection();

    CPPUNIT_TEST_SUITE(CpuRuntimeDetection_AVX2);
    CPPUNIT_TEST(testCpuRuntimeDetection);
    CPPUNIT_TEST_SUITE_END();
};

void CpuRuntimeDetection_AVX2::testCpuRuntimeDetection()
{
    // can only run if this function if CPU supports AVX2
    if (cpuid::isCpuInstructionSetSupported(cpuid::InstructionSetFlags::AVX2))
        checkAVX2();
}

void CpuRuntimeDetection_AVX2::checkAVX2()
{
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
}

CPPUNIT_TEST_SUITE_REGISTRATION(CpuRuntimeDetection_AVX2);

} // end anonymous namespace

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
