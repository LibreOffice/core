/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <tools/simdsupport.hxx>
#include "test_cpu_runtime_detection_x86_checks.hxx"

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
    void testCpuRuntimeDetection();

    CPPUNIT_TEST_SUITE(CpuRuntimeDetection_AVX2);
    CPPUNIT_TEST(testCpuRuntimeDetection);
    CPPUNIT_TEST_SUITE_END();
};

void CpuRuntimeDetection_AVX2::testCpuRuntimeDetection()
{
    // can only run this function if CPU supports AVX2
    if (cpuid::isCpuInstructionSetSupported(cpuid::InstructionSetFlags::AVX2))
        CpuRuntimeDetectionX86Checks::checkAVX2();
}

CPPUNIT_TEST_SUITE_REGISTRATION(CpuRuntimeDetection_AVX2);

} // end anonymous namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
