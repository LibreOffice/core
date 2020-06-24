/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <tools/simdsupport.hxx>

#ifdef LO_SSE2_AVAILABLE

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <tools/cpuid.hxx>

namespace
{
class CpuRuntimeDetection_SSE2 : public CppUnit::TestFixture
{
public:
    void checkSSE2();
    void testCpuRuntimeDetection();

    CPPUNIT_TEST_SUITE(CpuRuntimeDetection_SSE2);
    CPPUNIT_TEST(testCpuRuntimeDetection);
    CPPUNIT_TEST_SUITE_END();
};

void CpuRuntimeDetection_SSE2::testCpuRuntimeDetection()
{
    // can only run if this function if CPU supports SSE2
    if (cpuid::isCpuInstructionSetSupported(cpuid::InstructionSetFlags::SSE2))
        checkSSE2();
}

void CpuRuntimeDetection_SSE2::checkSSE2()
{
    // Try some SSE2 intrinsics calculation
    __m128i a = _mm_set1_epi32(15);
    __m128i b = _mm_set1_epi32(15);
    __m128i c = _mm_xor_si128(a, b);

    // Check zero
    CPPUNIT_ASSERT_EQUAL(0xFFFF, _mm_movemask_epi8(_mm_cmpeq_epi32(c, _mm_setzero_si128())));
}

CPPUNIT_TEST_SUITE_REGISTRATION(CpuRuntimeDetection_SSE2);

} // end anonymous namespace

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
