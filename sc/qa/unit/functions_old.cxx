/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "functions_test.hxx"

class FunctionsTestOld : public FunctionsTest
{
public:
    FunctionsTestOld();

    void testFormulasFODS();

    CPPUNIT_TEST_SUITE(FunctionsTestOld);
    CPPUNIT_TEST(testFormulasFODS);
    CPPUNIT_TEST_SUITE_END();

};

FunctionsTestOld::FunctionsTestOld():
    FunctionsTest()
{
}

void FunctionsTestOld::testFormulasFODS()
{
    OUString aDirectoryURL = m_directories.getURLFromSrc(u"/sc/qa/unit/data/functions/fods/");
    recursiveScan(test::pass, u"OpenDocument Spreadsheet Flat XML"_ustr, aDirectoryURL,
            u"com.sun.star.comp.filter.OdfFlatXml,,com.sun.star.comp.Calc.XMLOasisImporter,com.sun.star.comp.Calc.XMLOasisExporter,,,true"_ustr,
            FODS_FORMAT_TYPE, SotClipboardFormatId::NONE, 0, false);
}

CPPUNIT_TEST_SUITE_REGISTRATION(FunctionsTestOld);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
