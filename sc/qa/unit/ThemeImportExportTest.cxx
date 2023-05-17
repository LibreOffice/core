/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapixml_test.hxx>

#include <com/sun/star/lang/XComponent.hpp>
#include <comphelper/servicehelper.hxx>
#include <docsh.hxx>

using namespace css;

class ThemeImportExportTest : public UnoApiXmlTest
{
public:
    ThemeImportExportTest()
        : UnoApiXmlTest("sc/qa/unit/data")
    {
    }

    void test();

    CPPUNIT_TEST_SUITE(ThemeImportExportTest);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();
};

void ThemeImportExportTest::test()
{
    loadFromURL(u"xlsx/CalcThemeTest.xlsx");

    save("Calc Office Open XML");

    xmlDocUniquePtr pXmlDoc = parseExport("xl/styles.xml");

    assertXPath(pXmlDoc, "/x:styleSheet", 1);

    // Fonts
    assertXPath(pXmlDoc, "/x:styleSheet/x:fonts/x:font", 6);

    assertXPath(pXmlDoc, "/x:styleSheet/x:fonts/x:font[5]/x:color", "rgb",
                "FFFFC000"); // need to be theme probably

    assertXPath(pXmlDoc, "/x:styleSheet/x:fonts/x:font[6]/x:color", "rgb",
                "FF9C5700"); // need to be theme probably

    // Fills
    assertXPath(pXmlDoc, "/x:styleSheet/x:fills/x:fill", 4);

    assertXPath(pXmlDoc, "/x:styleSheet/x:fills/x:fill[1]/x:patternFill", "patternType", "none");

    assertXPath(pXmlDoc, "/x:styleSheet/x:fills/x:fill[2]/x:patternFill", "patternType", "gray125");

    assertXPath(pXmlDoc, "/x:styleSheet/x:fills/x:fill[3]/x:patternFill", "patternType", "solid");
    assertXPath(pXmlDoc, "/x:styleSheet/x:fills/x:fill[3]/x:patternFill/x:fgColor", "rgb",
                "FFFFEB9C");

    assertXPath(pXmlDoc, "/x:styleSheet/x:fills/x:fill[4]/x:patternFill", "patternType", "solid");
    assertXPath(pXmlDoc, "/x:styleSheet/x:fills/x:fill[4]/x:patternFill/x:fgColor", "theme", "4");
}

CPPUNIT_TEST_SUITE_REGISTRATION(ThemeImportExportTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
