/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <tools/cpuid.hxx>
#include <tools/simd.hxx>
#include <rtl/ustring.hxx>

namespace
{
class CpuInstructionSetSupport : public CppUnit::TestFixture
{
public:
    void testCpuInstructionSetSupport();

    CPPUNIT_TEST_SUITE(CpuInstructionSetSupport);
    CPPUNIT_TEST(testCpuInstructionSetSupport);
    CPPUNIT_TEST_SUITE_END();
};

void CpuInstructionSetSupport::testCpuInstructionSetSupport()
{
    OUString aString = cpuid::instructionSetSupportedString();

    if (cpuid::isCpuInstructionSetSupported(cpuid::InstructionSetFlags::SSE2))
    {
        CPPUNIT_ASSERT(aString.indexOf("SSE2") >= 0);
    }

    if (cpuid::isCpuInstructionSetSupported(cpuid::InstructionSetFlags::SSSE3))
    {
        CPPUNIT_ASSERT(aString.indexOf("SSSE3") >= 0);
    }

    if (cpuid::isCpuInstructionSetSupported(cpuid::InstructionSetFlags::AVX))
    {
        CPPUNIT_ASSERT(aString.indexOf("AVX") > 0);
    }

    if (cpuid::isCpuInstructionSetSupported(cpuid::InstructionSetFlags::AVX2))
    {
        CPPUNIT_ASSERT(aString.indexOf("AVX2") > 0);
    }

#ifdef LO_SSE2_AVAILABLE
    CPPUNIT_ASSERT_EQUAL(cpuid::hasSSE2(),
                         cpuid::isCpuInstructionSetSupported(cpuid::InstructionSetFlags::SSE2));
#endif

#ifdef LO_SSSE3_AVAILABLE
    CPPUNIT_ASSERT_EQUAL(cpuid::hasSSSE3(),
                         cpuid::isCpuInstructionSetSupported(cpuid::InstructionSetFlags::SSE2));
#endif

#ifdef LO_AVX2_AVAILABLE
    CPPUNIT_ASSERT_EQUAL(cpuid::hasAVX2(),
                         cpuid::isCpuInstructionSetSupported(cpuid::InstructionSetFlags::AVX2));
#endif
}

CPPUNIT_TEST_SUITE_REGISTRATION(CpuInstructionSetSupport);

} // end anonymous namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
