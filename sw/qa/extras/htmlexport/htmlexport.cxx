/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <rtl/byteseq.hxx>

#include <swmodule.hxx>
#include <swdll.hxx>
#include <usrpref.hxx>

#include <test/htmltesttools.hxx>

class HtmlExportTest : public SwModelTestBase, public HtmlTestTools
{
private:
    FieldUnit m_eUnit;

public:
    HtmlExportTest() :
        SwModelTestBase("/sw/qa/extras/htmlexport/data/", "HTML (StarWriter)"),
        m_eUnit(FUNIT_NONE)
    {}

private:
    bool mustCalcLayoutOf(const char* filename) override
    {
        return OString(filename) != "fdo62336.docx";
    }

    bool mustTestImportOf(const char* filename) const override
    {
        return OString(filename) != "fdo62336.docx";
    }

    void preTest(const char* filename) override
    {
        if (getTestName().indexOf("SkipImage") != -1)
            setFilterOptions("SkipImages");
        else
            setFilterOptions("");

        if (OString(filename) == "charborder.odt")
        {
            // FIXME if padding-top gets exported as inches, not cms, we get rounding errors.
            SwGlobals::ensure(); // make sure that SW_MOD() is not 0
            SwMasterUsrPref* pPref = const_cast<SwMasterUsrPref*>(SW_MOD()->GetUsrPref(false));
            m_eUnit = pPref->GetMetric();
            pPref->SetMetric(FUNIT_CM);
        }
    }

    void postTest(const char* filename) override
    {
        if (OString(filename) == "charborder.odt")
        {
            SwMasterUsrPref* pPref = const_cast<SwMasterUsrPref*>(SW_MOD()->GetUsrPref(false));
            pPref->SetMetric(m_eUnit);
        }
    }
};

#define DECLARE_HTMLEXPORT_ROUNDTRIP_TEST(TestName, filename) DECLARE_SW_ROUNDTRIP_TEST(TestName, filename, HtmlExportTest)

DECLARE_HTMLEXPORT_ROUNDTRIP_TEST(testFdo81276, "fdo81276.html")
{
    uno::Reference<container::XNameAccess> xPageStyles(getStyles("PageStyles"));
    uno::Reference<beans::XPropertySet> xStyle(xPageStyles->getByName("HTML"), uno::UNO_QUERY);
    // some rounding going on here?
    CPPUNIT_ASSERT(abs(sal_Int32(29700) - getProperty<sal_Int32>(xStyle, "Width")) < 10);
    CPPUNIT_ASSERT(abs(sal_Int32(21006) - getProperty<sal_Int32>(xStyle, "Height")) < 10);
    CPPUNIT_ASSERT(abs(sal_Int32(500) - getProperty<sal_Int32>(xStyle, "LeftMargin")) < 10);
    CPPUNIT_ASSERT(abs(sal_Int32(500) - getProperty<sal_Int32>(xStyle, "RightMargin")) < 10);
    CPPUNIT_ASSERT(abs(sal_Int32(2000) - getProperty<sal_Int32>(xStyle, "TopMargin")) < 10);
    CPPUNIT_ASSERT(abs(sal_Int32(500) - getProperty<sal_Int32>(xStyle, "BottomMargin")) < 10);
}

DECLARE_HTMLEXPORT_ROUNDTRIP_TEST(testFdo62336, "fdo62336.docx")
{
    // The problem was essentially a crash during table export as docx/rtf/html
    // If either of no-calc-layout or no-test-import is enabled, the crash does not occur
}

DECLARE_HTMLEXPORT_ROUNDTRIP_TEST(testFdo86857, "fdo86857.html")
{
    // problem was that background color on page style was not exported
    uno::Reference<container::XNameAccess> xPageStyles(getStyles("PageStyles"));
    uno::Reference<beans::XPropertySet> xStyle(xPageStyles->getByName("HTML"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xff0000), getProperty<sal_Int32>(xStyle, "BackColor"));
    // check that table background color works, which still uses RES_BACKGROUND
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x66ffff), getProperty<sal_Int32>(xCell, "BackColor"));
}

DECLARE_HTMLEXPORT_ROUNDTRIP_TEST(testCharacterBorder, "charborder.odt")
{

    uno::Reference<beans::XPropertySet> xRun(getRun(getParagraph(1),1), uno::UNO_QUERY);
    // Different Border
    {
        CPPUNIT_ASSERT_BORDER_EQUAL(table::BorderLine2(0x6666FF,12,12,12,3,37), getProperty<table::BorderLine2>(xRun,"CharTopBorder"));
        CPPUNIT_ASSERT_BORDER_EQUAL(table::BorderLine2(0xFF9900,0,99,0,2,99), getProperty<table::BorderLine2>(xRun,"CharLeftBorder"));
        CPPUNIT_ASSERT_BORDER_EQUAL(table::BorderLine2(0xFF0000,0,169,0,1,169), getProperty<table::BorderLine2>(xRun,"CharBottomBorder"));
        CPPUNIT_ASSERT_BORDER_EQUAL(table::BorderLine2(0x0000FF,0,169,0,0,169), getProperty<table::BorderLine2>(xRun,"CharRightBorder"));
    }

    // Different Padding
    {
        CPPUNIT_ASSERT_EQUAL(sal_Int32(450), getProperty<sal_Int32>(xRun,"CharTopBorderDistance"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(550), getProperty<sal_Int32>(xRun,"CharLeftBorderDistance"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(150), getProperty<sal_Int32>(xRun,"CharBottomBorderDistance"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(250), getProperty<sal_Int32>(xRun,"CharRightBorderDistance"));
    }

    // No shadow
}

#define DECLARE_HTMLEXPORT_TEST(TestName, filename) DECLARE_SW_EXPORT_TEST(TestName, filename, HtmlExportTest)

DECLARE_HTMLEXPORT_TEST(testExportOfImages, "textAndImage.docx")
{
    htmlDocPtr pDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/html/body", 1);
    assertXPath(pDoc, "/html/body/p/img", 1);
}

DECLARE_HTMLEXPORT_TEST(testExportOfImagesWithSkipImageEnabled, "textAndImage.docx")
{
    htmlDocPtr pDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/html/body", 1);
    assertXPath(pDoc, "/html/body/p/img", 0);
}

DECLARE_HTMLEXPORT_TEST(testSkipImageEmbedded, "skipimage-embedded.doc")
{
    // Embedded spreadsheet was exported as image, so content was lost. Make
    // sure it's exported as HTML instead.
    htmlDocPtr pDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pDoc);

    // This was 0.
    assertXPath(pDoc, "//table", 1);
    // This was 2, the HTML header was in the document two times.
    assertXPath(pDoc, "//meta[@name='generator']", 1);
    // This was 0, <table> was directly under <p>, which caused errors in the parser.
    assertXPath(pDoc, "//span/table", 1);
}

DECLARE_HTMLEXPORT_TEST(testSkipImageEmbeddedDocument, "skipimage-embedded-document.docx")
{
    // Similar to testSkipImageEmbedded, but with an embedded Writer object,
    // not a Calc one, and this time OOXML, not WW8.
    htmlDocPtr pDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pDoc);

    // This was 2, the HTML header was in the document two times.
    assertXPath(pDoc, "//meta[@name='generator']", 1);
    // Text of embedded document was missing.
    assertXPathContent(pDoc, "/html/body/p/span/p/span", "Inner.");
}

DECLARE_HTMLEXPORT_TEST(testExportImageProperties, "HTMLImage.odt")
{
    htmlDocPtr pDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/html/body", 1);

    assertXPath(pDoc, "/html/body/p/map/area", "shape", "poly");
    assertXPath(pDoc, "/html/body/p/map/area", "href", "http://www.microsoft.com/");
    assertXPath(pDoc, "/html/body/p/map/area", "target", "_self");
    assertXPath(pDoc, "/html/body/p/map/area", "alt", "microsoft");

    assertXPath(pDoc, "/html/body/p/a", 1);
    assertXPath(pDoc, "/html/body/p/a", "href", "http://www.google.com/");

    assertXPath(pDoc, "/html/body/p/a/font", 1);
    assertXPath(pDoc, "/html/body/p/a/font", "color", "#ff0000");

    assertXPath(pDoc, "/html/body/p/a/font/img", 1);
    assertXPath(pDoc, "/html/body/p/a/font/img", "name", "Text");
    assertXPath(pDoc, "/html/body/p/a/font/img", "alt", "Four colors");
    assertXPath(pDoc, "/html/body/p/a/font/img", "align", "middle");

    // Probably the DPI in OSX is different and Twip -> Pixel conversion produces
    // different results - so disable OSX for now.
    //
    // It would make sense to switch to use CSS and use "real world" units instead
    // i.e. (style="margin: 0cm 1.5cm; width: 1cm; height: 1cm")

#if 0 // disabled as it depends that the system DPI is set to 96
    assertXPath(pDoc, "/html/body/p/a/font/img", "hspace", "38");
    assertXPath(pDoc, "/html/body/p/a/font/img", "vspace", "19");
    assertXPath(pDoc, "/html/body/p/a/font/img", "width", "222");
    assertXPath(pDoc, "/html/body/p/a/font/img", "height", "222");
    assertXPath(pDoc, "/html/body/p/a/font/img", "border", "3");
#endif

    assertXPath(pDoc, "/html/body/p/a/font/img", "usemap", "#map1");
}

DECLARE_HTMLEXPORT_TEST(testExportCheckboxRadioButtonState, "checkbox-radiobutton.doc")
{
    htmlDocPtr pDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/html/body", 1);
    assertXPath(pDoc, "/html/body/p[1]/input", "type", "checkbox");
    assertXPath(pDoc, "/html/body/p[1]/input", "checked", "checked");
    assertXPath(pDoc, "/html/body/p[2]/input", "type", "checkbox");
    assertXPathNoAttribute(pDoc, "/html/body/p[2]/input", "checked");
    assertXPath(pDoc, "/html/body/form/p[1]/input", "type", "checkbox");
    assertXPath(pDoc, "/html/body/form/p[1]/input", "checked", "checked");
    assertXPath(pDoc, "/html/body/form/p[2]/input", "type", "checkbox");
    assertXPathNoAttribute(pDoc, "/html/body/form/p[2]/input", "checked");
    assertXPath(pDoc, "/html/body/form/p[3]/input", "type", "radio");
    assertXPath(pDoc, "/html/body/form/p[3]/input", "checked", "checked");
    assertXPath(pDoc, "/html/body/form/p[4]/input", "type", "radio");
    assertXPathNoAttribute(pDoc, "/html/body/form/p[4]/input", "checked");
}

DECLARE_HTMLEXPORT_TEST(testExportUrlEncoding, "tdf76291.odt")
{
    htmlDocPtr pDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pDoc);

    // Test URI encoded hyperlink with Chinese characters
    assertXPath(pDoc, "/html/body/p/a", "href", "http://www.youtube.com/results?search_query=%E7%B2%B5%E8%AA%9Emv&sm=12");
}

DECLARE_HTMLEXPORT_TEST(testExportInternalUrl, "tdf90905.odt")
{
    htmlDocPtr pDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pDoc);

    // Internal url should be valid
    assertXPath(pDoc, "/html/body/p[1]/a", "href", "#0.0.1.Text|outline");
    assertXPath(pDoc, "/html/body/p[2]/a", "href", "#bookmark");
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
