/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "functions_test.hxx"
#include <arraysumfunctor.hxx>

class StatisticalFunctionsTest : public FunctionsTest
{
public:
    StatisticalFunctionsTest();

    void testStatisticalFormulasFODS();
    void testIntrinsicSums();

    CPPUNIT_TEST_SUITE(StatisticalFunctionsTest);
    CPPUNIT_TEST(testStatisticalFormulasFODS);
    CPPUNIT_TEST(testIntrinsicSums);
    CPPUNIT_TEST_SUITE_END();

};

void StatisticalFunctionsTest::testStatisticalFormulasFODS()
{
    OUString aDirectoryURL = m_directories.getURLFromSrc(u"/sc/qa/unit/data/functions/statistical/fods/");
    recursiveScan(test::pass, u"OpenDocument Spreadsheet Flat XML"_ustr, aDirectoryURL,
            u"com.sun.star.comp.filter.OdfFlatXml,,com.sun.star.comp.Calc.XMLOasisImporter,com.sun.star.comp.Calc.XMLOasisExporter,,,true"_ustr,
            FODS_FORMAT_TYPE, SotClipboardFormatId::NONE, 0, false);
}

StatisticalFunctionsTest::StatisticalFunctionsTest():
    FunctionsTest()
{
}

void StatisticalFunctionsTest::testIntrinsicSums()
{
    // Checkout SSE2, AVX and AVX512 operations
    // Needs exactly 9 terms
    double summands[9] = { 0, 1, 2, 3, 4, 10, 20, 2, -1 };
    double* pCurrent = summands;
    size_t i = 0;
#ifdef LO_X86_SIMD_AVAILABLE
    double fSum, fErr;
    if (cpuid::hasAVX512F())
    {
        fSum = 0.0; fErr = 0.0;
        sc::op::executeAVX512F(i, 9, pCurrent, fSum, fErr);
        CPPUNIT_ASSERT_EQUAL(42.0, KahanSum(fSum, fErr).get());
    }
    i = 0;
    if (cpuid::hasAVX())
    {
        fSum = 0.0; fErr = 0.0;
        sc::op::executeAVX(i, 9, pCurrent, fSum, fErr);
        CPPUNIT_ASSERT_EQUAL(42.0, KahanSum(fSum, fErr).get());
    }
    i = 0;
    fSum = 0.0; fErr = 0.0;
    sc::op::executeSSE2(i, 9, pCurrent, fSum, fErr);
    CPPUNIT_ASSERT_EQUAL(42.0, KahanSum(fSum, fErr).get());
    i = 0;
#endif
    CPPUNIT_ASSERT_EQUAL(42.0, sc::op::executeUnrolled(i, 9, pCurrent).get());
}

CPPUNIT_TEST_SUITE_REGISTRATION(StatisticalFunctionsTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
