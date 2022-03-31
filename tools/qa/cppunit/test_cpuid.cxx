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
#include <tools/cpuid.hxx>
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
}

CPPUNIT_TEST_SUITE_REGISTRATION(CpuInstructionSetSupport);

} // end anonymous namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
