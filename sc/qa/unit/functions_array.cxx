/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "functions_test.hxx"

class ArrayFunctionsTest : public FunctionsTest
{
public:
    ArrayFunctionsTest();

    void testArrayFormulasFODS();
    void testDubiousArrayFormulasFODS();

    CPPUNIT_TEST_SUITE(ArrayFunctionsTest);
    CPPUNIT_TEST(testArrayFormulasFODS);
    CPPUNIT_TEST(testDubiousArrayFormulasFODS);
    CPPUNIT_TEST_SUITE_END();

};

void ArrayFunctionsTest::testArrayFormulasFODS()
{
    OUString aDirectoryURL = m_directories.getURLFromSrc(u"/sc/qa/unit/data/functions/array/fods/");
    recursiveScan(test::pass, u"OpenDocument Spreadsheet Flat XML"_ustr, aDirectoryURL,
            u"com.sun.star.comp.filter.OdfFlatXml,,com.sun.star.comp.Calc.XMLOasisImporter,com.sun.star.comp.Calc.XMLOasisExporter,,,true"_ustr,
            FODS_FORMAT_TYPE, SotClipboardFormatId::NONE, 0, false);
}

void ArrayFunctionsTest::testDubiousArrayFormulasFODS()
{
    //TODO: sc/qa/unit/data/functions/array/dubious/fods/linest.fods produces widely different
    // values when built with -ffp-contract enabled (-ffp-contract=on default on Clang 14,
    // -ffp-contract=fast default when building with optimizations on GCC) on at least aarch64
    // and ppc64le. Thus limit the check only to platforms with consistent results.
#if defined X86_64
    OUString aDirectoryURL
        = m_directories.getURLFromSrc(u"/sc/qa/unit/data/functions/array/dubious/fods/");
    recursiveScan(test::pass, u"OpenDocument Spreadsheet Flat XML"_ustr, aDirectoryURL,
            u"com.sun.star.comp.filter.OdfFlatXml,,com.sun.star.comp.Calc.XMLOasisImporter,com.sun.star.comp.Calc.XMLOasisExporter,,,true"_ustr,
            FODS_FORMAT_TYPE, SotClipboardFormatId::NONE, 0, false);
#endif
}

ArrayFunctionsTest::ArrayFunctionsTest():
    FunctionsTest()
{
}

CPPUNIT_TEST_SUITE_REGISTRATION(ArrayFunctionsTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
