/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "helper/qahelper.hxx"
#include "helper/xpath.hxx"

#include <com/sun/star/lang/XComponent.hpp>
#include <docsh.hxx>
#include <Sparkline.hxx>
#include <SparklineGroup.hxx>

using namespace css;

/** Test import, export or roundtrip of sparklines for ODF and OOXML */
class SparklineImportExportTest : public ScBootstrapFixture, public XmlTestTools
{
private:
    uno::Reference<uno::XInterface> m_xCalcComponent;

public:
    SparklineImportExportTest()
        : ScBootstrapFixture("sc/qa/unit/data")
    {
    }

    virtual void setUp() override
    {
        test::BootstrapFixture::setUp();

        // This is a bit of a fudge, we do this to ensure that ScGlobals::ensure,
        // which is a private symbol to us, gets called
        m_xCalcComponent = getMultiServiceFactory()->createInstance(
            "com.sun.star.comp.Calc.SpreadsheetDocument");
        CPPUNIT_ASSERT_MESSAGE("no calc component!", m_xCalcComponent.is());
    }

    virtual void tearDown() override
    {
        uno::Reference<lang::XComponent>(m_xCalcComponent, uno::UNO_QUERY_THROW)->dispose();
        test::BootstrapFixture::tearDown();
    }

    virtual void registerNamespaces(xmlXPathContextPtr& pXmlXPathContextPtr) override
    {
        XmlTestTools::registerOOXMLNamespaces(pXmlXPathContextPtr);
        XmlTestTools::registerODFNamespaces(pXmlXPathContextPtr);
    }

    void testSparklinesRoundtripXLSX();
    void testSparklinesExportODS();
    void testSparklinesRoundtripODS();
    void testNoSparklinesInDocumentXLSX();

    CPPUNIT_TEST_SUITE(SparklineImportExportTest);
    CPPUNIT_TEST(testSparklinesRoundtripXLSX);
    CPPUNIT_TEST(testSparklinesExportODS);
    CPPUNIT_TEST(testSparklinesRoundtripODS);
    CPPUNIT_TEST(testNoSparklinesInDocumentXLSX);
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
        CPPUNIT_ASSERT_EQUAL(OString("{1C5C5DE0-3C09-4CB3-A3EC-9E763301EC82}"),
                             pSparkline->getSparklineGroup()->getID().getString());

        auto& rAttributes = pSparkline->getSparklineGroup()->getAttributes();
        CPPUNIT_ASSERT_EQUAL(sc::SparklineType::Line, rAttributes.getType());

        CPPUNIT_ASSERT_EQUAL(Color(0x376092), rAttributes.getColorSeries());
        CPPUNIT_ASSERT_EQUAL(Color(0x00b050), rAttributes.getColorNegative());
        CPPUNIT_ASSERT_EQUAL(Color(0x000000), rAttributes.getColorAxis());
        CPPUNIT_ASSERT_EQUAL(Color(0x000000), rAttributes.getColorMarkers());
        CPPUNIT_ASSERT_EQUAL(Color(0x7030a0), rAttributes.getColorFirst());
        CPPUNIT_ASSERT_EQUAL(Color(0xff0000), rAttributes.getColorLast());
        CPPUNIT_ASSERT_EQUAL(Color(0x92d050), rAttributes.getColorHigh());
        CPPUNIT_ASSERT_EQUAL(Color(0x00b0f0), rAttributes.getColorLow());

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

        CPPUNIT_ASSERT_EQUAL(Color(0x376092), rAttributes.getColorSeries());
        CPPUNIT_ASSERT_EQUAL(Color(0xff0000), rAttributes.getColorNegative());
        CPPUNIT_ASSERT_EQUAL(Color(0x000000), rAttributes.getColorAxis());
        CPPUNIT_ASSERT_EQUAL(Color(0xd00000), rAttributes.getColorMarkers());
        CPPUNIT_ASSERT_EQUAL(Color(0x92d050), rAttributes.getColorFirst());
        CPPUNIT_ASSERT_EQUAL(Color(0x00b0f0), rAttributes.getColorLast());
        CPPUNIT_ASSERT_EQUAL(Color(0x7030a0), rAttributes.getColorHigh());
        CPPUNIT_ASSERT_EQUAL(Color(0xffc000), rAttributes.getColorLow());

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
    ScDocShellRef xDocSh = loadDoc(u"Sparklines.", FORMAT_XLSX);

    checkSparklines(xDocSh->GetDocument());

    xDocSh = saveAndReload(*xDocSh, FORMAT_XLSX);

    checkSparklines(xDocSh->GetDocument());

    xDocSh->DoClose();
}

void SparklineImportExportTest::testSparklinesExportODS()
{
    // Load the document containing sparklines
    ScDocShellRef xDocSh = loadDoc(u"Sparklines.", FORMAT_XLSX);

    // Save as ODS and check content.xml with XPath
    std::shared_ptr<utl::TempFile> pXPathFile = ScBootstrapFixture::exportTo(*xDocSh, FORMAT_ODS);
    xmlDocUniquePtr pXmlDoc = XPathHelper::parseExport(pXPathFile, m_xSFactory, "content.xml");

    // We have 3 sparkline groups = 3 tables that contain sparklines
    assertXPath(pXmlDoc, "//table:table/calcext:sparkline-groups", 3);

    // Check the number of sparkline groups in table[1]
    assertXPath(pXmlDoc, "//table:table[1]/calcext:sparkline-groups/calcext:sparkline-group", 2);
    // Check the number of sparkline groups in table[2]
    assertXPath(pXmlDoc, "//table:table[2]/calcext:sparkline-groups/calcext:sparkline-group", 2);
    // Check the number of sparkline groups in table[3]
    assertXPath(pXmlDoc, "//table:table[3]/calcext:sparkline-groups/calcext:sparkline-group", 3);

    // Check table[1] - sparkline-group[1]
    OString aSparklineGroupPath
        = "//table:table[1]/calcext:sparkline-groups/calcext:sparkline-group[1]";
    assertXPath(pXmlDoc, aSparklineGroupPath, "type", "line");
    assertXPath(pXmlDoc, aSparklineGroupPath, "line-width", "1pt");
    assertXPath(pXmlDoc, aSparklineGroupPath, "display-empty-cells-as", "gap");
    assertXPath(pXmlDoc, aSparklineGroupPath, "markers", "true");
    assertXPath(pXmlDoc, aSparklineGroupPath, "high", "true");
    assertXPath(pXmlDoc, aSparklineGroupPath, "low", "true");
    assertXPath(pXmlDoc, aSparklineGroupPath, "first", "true");
    assertXPath(pXmlDoc, aSparklineGroupPath, "last", "true");
    assertXPath(pXmlDoc, aSparklineGroupPath, "negative", "true");
    assertXPath(pXmlDoc, aSparklineGroupPath, "display-x-axis", "true");
    assertXPath(pXmlDoc, aSparklineGroupPath, "min-axis-type", "individual");
    assertXPath(pXmlDoc, aSparklineGroupPath, "max-axis-type", "individual");
    assertXPath(pXmlDoc, aSparklineGroupPath, "color-series", "#376092");
    assertXPath(pXmlDoc, aSparklineGroupPath, "color-negative", "#00b050");
    assertXPath(pXmlDoc, aSparklineGroupPath, "color-axis", "#000000");
    assertXPath(pXmlDoc, aSparklineGroupPath, "color-markers", "#000000");
    assertXPath(pXmlDoc, aSparklineGroupPath, "color-first", "#7030a0");
    assertXPath(pXmlDoc, aSparklineGroupPath, "color-last", "#ff0000");
    assertXPath(pXmlDoc, aSparklineGroupPath, "color-high", "#92d050");
    assertXPath(pXmlDoc, aSparklineGroupPath, "color-low", "#00b0f0");

    assertXPath(pXmlDoc, aSparklineGroupPath + "/calcext:sparklines/calcext:sparkline", 1);
    assertXPath(pXmlDoc, aSparklineGroupPath + "/calcext:sparklines/calcext:sparkline[1]",
                "cell-address", "Sheet1.A2");
}

void SparklineImportExportTest::testSparklinesRoundtripODS()
{
    ScDocShellRef xDocSh = loadDoc(u"Sparklines.", FORMAT_XLSX);

    checkSparklines(xDocSh->GetDocument());

    // Trigger export and import of sparklines
    xDocSh = saveAndReload(*xDocSh, FORMAT_ODS);

    checkSparklines(xDocSh->GetDocument());

    xDocSh->DoClose();
}

void SparklineImportExportTest::testNoSparklinesInDocumentXLSX()
{
    // tdf#148835
    // Check no sparkline elements are written when there is none in the document

    // Load the document containing NO sparklines
    ScDocShellRef xDocSh = loadDoc(u"empty.", FORMAT_XLSX);

    std::shared_ptr<utl::TempFile> pXPathFile = ScBootstrapFixture::exportTo(*xDocSh, FORMAT_XLSX);
    xmlDocUniquePtr pXmlDoc
        = XPathHelper::parseExport(pXPathFile, m_xSFactory, "xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/x:worksheet", 1);
    assertXPath(pXmlDoc, "/x:worksheet/x:extLst/x:ext/x14:sparklineGroups", 0);
    assertXPath(pXmlDoc, "/x:worksheet/x:extLst/x:ext", 0);
    assertXPath(pXmlDoc, "/x:worksheet/x:extLst", 0);
}

CPPUNIT_TEST_SUITE_REGISTRATION(SparklineImportExportTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
