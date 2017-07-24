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

#include "ScanlineTools.hxx"

namespace
{

class ScanlineToolsTest : public CppUnit::TestFixture
{
    void testConvertScanlineRGBAtoBGRA();

    CPPUNIT_TEST_SUITE(ScanlineToolsTest);
    CPPUNIT_TEST(testConvertScanlineRGBAtoBGRA);
    CPPUNIT_TEST_SUITE_END();
};

void ScanlineToolsTest::testConvertScanlineRGBAtoBGRA()
{
    sal_uInt8 R = 50;
    sal_uInt8 G = 100;
    sal_uInt8 B = 150;
    sal_uInt8 A = 200;

    sal_uInt8 aPixelSource[]   = { R, G, B, A };
    sal_uInt8 aPixelExpected[] = { B, G, R, A };

    size_t aSize = (256 + 2) * 4;

    std::vector<sal_uInt8> aSource(aSize);
    std::vector<sal_uInt8> aExpected(aSize);

    for (size_t i = 0; i < aSource.size(); ++i)
    {
        aSource[i]   = (aPixelSource[i % 4] + (i / 4));
        aExpected[i] = (aPixelExpected[i % 4] + (i / 4));
    }

    {
        std::vector<sal_uInt8> aDestination(aSize);

        bool bResult = vcl::scanline::swapABCDtoCBAD(aSource.data(), aDestination.data(), aSource.size());
        CPPUNIT_ASSERT(bResult);

        for (size_t i = 0; i < aSource.size(); i += 4)
        {
            CPPUNIT_ASSERT_EQUAL(int(aExpected[i + 0]), int(aDestination[i + 0]));
            CPPUNIT_ASSERT_EQUAL(int(aExpected[i + 1]), int(aDestination[i + 1]));
            CPPUNIT_ASSERT_EQUAL(int(aExpected[i + 2]), int(aDestination[i + 2]));
            CPPUNIT_ASSERT_EQUAL(int(aExpected[i + 3]), int(aDestination[i + 3]));
        }
    }

    if (cpuid::hasSSE2())
    {
        std::vector<sal_uInt8> aDestination(aSize);

        bool bResult = vcl::scanline::swapABCDtoCBAD_SSE2(aSource.data(), aDestination.data(), aSource.size());
        CPPUNIT_ASSERT(bResult);

        for (size_t i = 0; i < aSource.size(); i += 4)
        {
            CPPUNIT_ASSERT_EQUAL(int(aExpected[i + 0]), int(aDestination[i + 0]));
            CPPUNIT_ASSERT_EQUAL(int(aExpected[i + 1]), int(aDestination[i + 1]));
            CPPUNIT_ASSERT_EQUAL(int(aExpected[i + 2]), int(aDestination[i + 2]));
            CPPUNIT_ASSERT_EQUAL(int(aExpected[i + 3]), int(aDestination[i + 3]));
        }
    }

    if (cpuid::hasSSSE3())
    {
        std::vector<sal_uInt8> aDestination(aSize);

        bool bResult = vcl::scanline::swapABCDtoCBAD_SSSE3(aSource.data(), aDestination.data(), aSource.size());
        CPPUNIT_ASSERT(bResult);

        for (size_t i = 0; i < aSource.size(); i += 4)
        {
            CPPUNIT_ASSERT_EQUAL(int(aExpected[i + 0]), int(aDestination[i + 0]));
            CPPUNIT_ASSERT_EQUAL(int(aExpected[i + 1]), int(aDestination[i + 1]));
            CPPUNIT_ASSERT_EQUAL(int(aExpected[i + 2]), int(aDestination[i + 2]));
            CPPUNIT_ASSERT_EQUAL(int(aExpected[i + 3]), int(aDestination[i + 3]));
        }
    }

    if (cpuid::hasAVX2())
    {
        std::vector<sal_uInt8> aDestination(aSize);

        bool bResult = vcl::scanline::swapABCDtoCBAD_AVX2(aSource.data(), aDestination.data(), aSource.size());
        CPPUNIT_ASSERT(bResult);

        for (size_t i = 0; i < aSource.size(); i += 4)
        {
            CPPUNIT_ASSERT_EQUAL(int(aExpected[i + 0]), int(aDestination[i + 0]));
            CPPUNIT_ASSERT_EQUAL(int(aExpected[i + 1]), int(aDestination[i + 1]));
            CPPUNIT_ASSERT_EQUAL(int(aExpected[i + 2]), int(aDestination[i + 2]));
            CPPUNIT_ASSERT_EQUAL(int(aExpected[i + 3]), int(aDestination[i + 3]));
        }
    }
}

} // end anonymous namespace

CPPUNIT_TEST_SUITE_REGISTRATION(ScanlineToolsTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
