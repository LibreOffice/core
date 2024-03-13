/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapixml_test.hxx>
#include <document.hxx>
#include <comphelper/servicehelper.hxx>
#include <Sparkline.hxx>
#include <SparklineGroup.hxx>
#include <docuno.hxx>

using namespace css;

/** Test import, export or roundtrip of sparklines for ODF and OOXML */
class SparklineImportExportTest : public UnoApiXmlTest
{
public:
    SparklineImportExportTest()
        : UnoApiXmlTest("sc/qa/unit/data")
    {
    }

    void testSparklinesRoundtripXLSX();
    void testSparklinesExportODS();
    void testSparklinesRoundtripODS();
    void testNoSparklinesInDocumentXLSX();
    void testSparklinesRoundtripThemeColorsODS();
    void testSparklinesRoundtripThemeColorsOOXML();

    CPPUNIT_TEST_SUITE(SparklineImportExportTest);
    CPPUNIT_TEST(testSparklinesRoundtripXLSX);
    CPPUNIT_TEST(testSparklinesExportODS);
    CPPUNIT_TEST(testSparklinesRoundtripODS);
    CPPUNIT_TEST(testNoSparklinesInDocumentXLSX);
    CPPUNIT_TEST(testSparklinesRoundtripThemeColorsODS);
    CPPUNIT_TEST(testSparklinesRoundtripThemeColorsOOXML);
    CPPUNIT_TEST_SUITE_END();
};

namespace
{
void checkSparklines(ScDocument& rDocument)
{
    // Sparkline at Sheet1:A2
    {
        auto pSparkline = rDocument.GetSparkline(ScAddress(0, 1, 0)); // A2
        CPPUNIT_ASSERT(pSparkline);
        CPPUNIT_ASSERT_EQUAL("{1C5C5DE0-3C09-4CB3-A3EC-9E763301EC82}"_ostr,
                             pSparkline->getSparklineGroup()->getID().getString());

        auto& rAttributes = pSparkline->getSparklineGroup()->getAttributes();
        CPPUNIT_ASSERT_EQUAL(sc::SparklineType::Line, rAttributes.getType());

        CPPUNIT_ASSERT_EQUAL(Color(0x376092), rAttributes.getColorSeries().getFinalColor());
        CPPUNIT_ASSERT_EQUAL(Color(0x00b050), rAttributes.getColorNegative().getFinalColor());
        CPPUNIT_ASSERT_EQUAL(COL_BLACK, rAttributes.getColorAxis().getFinalColor());
        CPPUNIT_ASSERT_EQUAL(COL_BLACK, rAttributes.getColorMarkers().getFinalColor());
        CPPUNIT_ASSERT_EQUAL(Color(0x7030a0), rAttributes.getColorFirst().getFinalColor());
        CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, rAttributes.getColorLast().getFinalColor());
        CPPUNIT_ASSERT_EQUAL(Color(0x92d050), rAttributes.getColorHigh().getFinalColor());
        CPPUNIT_ASSERT_EQUAL(Color(0x00b0f0), rAttributes.getColorLow().getFinalColor());

        CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, rAttributes.getLineWeight(), 1E-2);
        CPPUNIT_ASSERT_EQUAL(false, rAttributes.isDateAxis());
        CPPUNIT_ASSERT_EQUAL(sc::DisplayEmptyCellsAs::Gap, rAttributes.getDisplayEmptyCellsAs());

        CPPUNIT_ASSERT_EQUAL(true, rAttributes.isMarkers());
        CPPUNIT_ASSERT_EQUAL(true, rAttributes.isHigh());
        CPPUNIT_ASSERT_EQUAL(true, rAttributes.isLow());
        CPPUNIT_ASSERT_EQUAL(true, rAttributes.isFirst());
        CPPUNIT_ASSERT_EQUAL(true, rAttributes.isLast());
        CPPUNIT_ASSERT_EQUAL(true, rAttributes.isNegative());
        CPPUNIT_ASSERT_EQUAL(true, rAttributes.shouldDisplayXAxis());
        CPPUNIT_ASSERT_EQUAL(false, rAttributes.shouldDisplayHidden());
        CPPUNIT_ASSERT_EQUAL(false, rAttributes.isRightToLeft());

        CPPUNIT_ASSERT_EQUAL(false, bool(rAttributes.getManualMax()));
        CPPUNIT_ASSERT_EQUAL(false, bool(rAttributes.getManualMin()));
    }
    // Sparkline at Sheet1:A3
    {
        auto pSparkline = rDocument.GetSparkline(ScAddress(0, 2, 0)); // A3
        CPPUNIT_ASSERT(pSparkline);
        auto& rAttributes = pSparkline->getSparklineGroup()->getAttributes();
        CPPUNIT_ASSERT_EQUAL(sc::SparklineType::Column, rAttributes.getType());

        CPPUNIT_ASSERT_EQUAL(Color(0x376092), rAttributes.getColorSeries().getFinalColor());
        CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, rAttributes.getColorNegative().getFinalColor());
        CPPUNIT_ASSERT_EQUAL(COL_BLACK, rAttributes.getColorAxis().getFinalColor());
        CPPUNIT_ASSERT_EQUAL(Color(0xd00000), rAttributes.getColorMarkers().getFinalColor());
        CPPUNIT_ASSERT_EQUAL(Color(0x92d050), rAttributes.getColorFirst().getFinalColor());
        CPPUNIT_ASSERT_EQUAL(Color(0x00b0f0), rAttributes.getColorLast().getFinalColor());
        CPPUNIT_ASSERT_EQUAL(Color(0x7030a0), rAttributes.getColorHigh().getFinalColor());
        CPPUNIT_ASSERT_EQUAL(Color(0xffc000), rAttributes.getColorLow().getFinalColor());

        CPPUNIT_ASSERT_EQUAL(0.75, rAttributes.getLineWeight());
        CPPUNIT_ASSERT_EQUAL(false, rAttributes.isDateAxis());
        CPPUNIT_ASSERT_EQUAL(sc::DisplayEmptyCellsAs::Gap, rAttributes.getDisplayEmptyCellsAs());

        CPPUNIT_ASSERT_EQUAL(false, rAttributes.isMarkers());
        CPPUNIT_ASSERT_EQUAL(true, rAttributes.isHigh());
        CPPUNIT_ASSERT_EQUAL(true, rAttributes.isLow());
        CPPUNIT_ASSERT_EQUAL(true, rAttributes.isFirst());
        CPPUNIT_ASSERT_EQUAL(true, rAttributes.isLast());
        CPPUNIT_ASSERT_EQUAL(true, rAttributes.isNegative());
        CPPUNIT_ASSERT_EQUAL(false, rAttributes.shouldDisplayXAxis());
        CPPUNIT_ASSERT_EQUAL(false, rAttributes.shouldDisplayHidden());
        CPPUNIT_ASSERT_EQUAL(false, rAttributes.isRightToLeft());

        CPPUNIT_ASSERT_EQUAL(false, bool(rAttributes.getManualMax()));
        CPPUNIT_ASSERT_EQUAL(false, bool(rAttributes.getManualMin()));
    }
    // Sparkline at Sheet2:B1
    {
        auto pSparkline = rDocument.GetSparkline(ScAddress(1, 0, 1)); //B1
        CPPUNIT_ASSERT(pSparkline);
        auto& rAttributes = pSparkline->getSparklineGroup()->getAttributes();
        CPPUNIT_ASSERT_EQUAL(sc::SparklineType::Column, rAttributes.getType());
    }
    // Sparkline at Sheet2:B2
    {
        auto pSparkline = rDocument.GetSparkline(ScAddress(1, 1, 1)); //B2
        CPPUNIT_ASSERT(pSparkline);
        auto& rAttributes = pSparkline->getSparklineGroup()->getAttributes();
        CPPUNIT_ASSERT_EQUAL(sc::SparklineType::Line, rAttributes.getType());
    }
    // Sparkline at Sheet2:B2
    {
        auto pSparkline = rDocument.GetSparkline(ScAddress(1, 1, 1)); //B2
        CPPUNIT_ASSERT(pSparkline);
        auto& rAttributes = pSparkline->getSparklineGroup()->getAttributes();
        CPPUNIT_ASSERT_EQUAL(sc::SparklineType::Line, rAttributes.getType());
    }
    // Sparkline doesn't exists at A4
    {
        auto pSparkline = rDocument.GetSparkline(ScAddress(0, 3, 0)); //A4
        CPPUNIT_ASSERT(!pSparkline);
    }
}
} // end anonymous namespace

void SparklineImportExportTest::testSparklinesRoundtripXLSX()
{
    loadFromFile(u"xlsx/Sparklines.xlsx");
    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    CPPUNIT_ASSERT(pModelObj);

    checkSparklines(*pModelObj->GetDocument());

    saveAndReload("Calc Office Open XML");
    pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    CPPUNIT_ASSERT(pModelObj);

    checkSparklines(*pModelObj->GetDocument());
}

void SparklineImportExportTest::testSparklinesExportODS()
{
    // Load the document containing sparklines
    loadFromFile(u"xlsx/Sparklines.xlsx");

    // Save as ODS and check content.xml with XPath
    save("calc8");
    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");

    // We have 3 sparkline groups = 3 tables that contain sparklines
    assertXPath(pXmlDoc, "//table:table/calcext:sparkline-groups"_ostr, 3);

    // Check the number of sparkline groups in table[1]
    assertXPath(pXmlDoc, "//table:table[1]/calcext:sparkline-groups/calcext:sparkline-group"_ostr,
                2);
    // Check the number of sparkline groups in table[2]
    assertXPath(pXmlDoc, "//table:table[2]/calcext:sparkline-groups/calcext:sparkline-group"_ostr,
                2);
    // Check the number of sparkline groups in table[3]
    assertXPath(pXmlDoc, "//table:table[3]/calcext:sparkline-groups/calcext:sparkline-group"_ostr,
                3);

    // Check table[1] - sparkline-group[1]
    OString aSparklineGroupPath
        = "//table:table[1]/calcext:sparkline-groups/calcext:sparkline-group[1]"_ostr;
    assertXPath(pXmlDoc, aSparklineGroupPath, "type"_ostr, "line");
    assertXPath(pXmlDoc, aSparklineGroupPath, "line-width"_ostr, "1pt");
    assertXPath(pXmlDoc, aSparklineGroupPath, "display-empty-cells-as"_ostr, "gap");
    assertXPath(pXmlDoc, aSparklineGroupPath, "markers"_ostr, "true");
    assertXPath(pXmlDoc, aSparklineGroupPath, "high"_ostr, "true");
    assertXPath(pXmlDoc, aSparklineGroupPath, "low"_ostr, "true");
    assertXPath(pXmlDoc, aSparklineGroupPath, "first"_ostr, "true");
    assertXPath(pXmlDoc, aSparklineGroupPath, "last"_ostr, "true");
    assertXPath(pXmlDoc, aSparklineGroupPath, "negative"_ostr, "true");
    assertXPath(pXmlDoc, aSparklineGroupPath, "display-x-axis"_ostr, "true");
    assertXPath(pXmlDoc, aSparklineGroupPath, "min-axis-type"_ostr, "individual");
    assertXPath(pXmlDoc, aSparklineGroupPath, "max-axis-type"_ostr, "individual");
    assertXPath(pXmlDoc, aSparklineGroupPath, "color-series"_ostr, "#376092");
    assertXPath(pXmlDoc, aSparklineGroupPath, "color-negative"_ostr, "#00b050");
    assertXPath(pXmlDoc, aSparklineGroupPath, "color-axis"_ostr, "#000000");
    assertXPath(pXmlDoc, aSparklineGroupPath, "color-markers"_ostr, "#000000");
    assertXPath(pXmlDoc, aSparklineGroupPath, "color-first"_ostr, "#7030a0");
    assertXPath(pXmlDoc, aSparklineGroupPath, "color-last"_ostr, "#ff0000");
    assertXPath(pXmlDoc, aSparklineGroupPath, "color-high"_ostr, "#92d050");
    assertXPath(pXmlDoc, aSparklineGroupPath, "color-low"_ostr, "#00b0f0");

    assertXPath(pXmlDoc, aSparklineGroupPath + "/calcext:sparklines/calcext:sparkline", 1);
    assertXPath(pXmlDoc, aSparklineGroupPath + "/calcext:sparklines/calcext:sparkline[1]",
                "cell-address"_ostr, "Sheet1.A2");
}

void SparklineImportExportTest::testSparklinesRoundtripODS()
{
    loadFromFile(u"xlsx/Sparklines.xlsx");
    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    CPPUNIT_ASSERT(pModelObj);

    checkSparklines(*pModelObj->GetDocument());

    // Trigger export and import of sparklines
    saveAndReload("calc8");
    pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    CPPUNIT_ASSERT(pModelObj);

    checkSparklines(*pModelObj->GetDocument());
}

void SparklineImportExportTest::testNoSparklinesInDocumentXLSX()
{
    // tdf#148835
    // Check no sparkline elements are written when there are none in the document

    // Load the document containing NO sparklines
    loadFromFile(u"xlsx/empty.xlsx");

    save("Calc Office Open XML");
    xmlDocUniquePtr pXmlDoc = parseExport("xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/x:worksheet"_ostr, 1);
    assertXPath(pXmlDoc, "/x:worksheet/x:extLst/x:ext/x14:sparklineGroups"_ostr, 0);
    assertXPath(pXmlDoc, "/x:worksheet/x:extLst/x:ext"_ostr, 0);
    assertXPath(pXmlDoc, "/x:worksheet/x:extLst"_ostr, 0);
}

namespace
{
void checkSparklineThemeColors(ScDocument& rDocument)
{
    auto pSparkline = rDocument.GetSparkline(ScAddress(0, 1, 0)); // A2
    CPPUNIT_ASSERT(pSparkline);
    CPPUNIT_ASSERT_EQUAL("{1C5C5DE0-3C09-4CB3-A3EC-9E763301EC82}"_ostr,
                         pSparkline->getSparklineGroup()->getID().getString());

    auto& rAttributes = pSparkline->getSparklineGroup()->getAttributes();
    CPPUNIT_ASSERT_EQUAL(sc::SparklineType::Column, rAttributes.getType());

    CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent3,
                         rAttributes.getColorSeries().getThemeColorType());
    CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent6,
                         rAttributes.getColorNegative().getThemeColorType());
    CPPUNIT_ASSERT_EQUAL(model::ColorType::Unused, rAttributes.getColorAxis().getType());
    CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Light1,
                         rAttributes.getColorMarkers().getThemeColorType());
    CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent5,
                         rAttributes.getColorFirst().getThemeColorType());
    CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent2,
                         rAttributes.getColorLast().getThemeColorType());
    CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent1,
                         rAttributes.getColorHigh().getThemeColorType());
    CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent4,
                         rAttributes.getColorLow().getThemeColorType());
}
} // end anonymous namespace

void SparklineImportExportTest::testSparklinesRoundtripThemeColorsODS()
{
    loadFromFile(u"fods/Sparklines.fods");

    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    CPPUNIT_ASSERT(pModelObj);
    checkSparklineThemeColors(*pModelObj->GetDocument());

    saveAndReload("calc8");

    pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    CPPUNIT_ASSERT(pModelObj);
    checkSparklineThemeColors(*pModelObj->GetDocument());
}

void SparklineImportExportTest::testSparklinesRoundtripThemeColorsOOXML()
{
    loadFromFile(u"fods/Sparklines.fods");
    saveAndReload("Calc Office Open XML");

    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    CPPUNIT_ASSERT(pModelObj);
    checkSparklineThemeColors(*pModelObj->GetDocument());
}

CPPUNIT_TEST_SUITE_REGISTRATION(SparklineImportExportTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
