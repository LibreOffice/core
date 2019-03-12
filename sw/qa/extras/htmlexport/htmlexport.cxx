/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <swmodeltestbase.hxx>

#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/document/XEmbeddedObjectSupplier2.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/io/XActiveDataStreamer.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <rtl/byteseq.hxx>

#include <swmodule.hxx>
#include <swdll.hxx>
#include <usrpref.hxx>

#include <test/htmltesttools.hxx>
#include <tools/urlobj.hxx>
#include <svtools/rtfkeywd.hxx>

class HtmlExportTest : public SwModelTestBase, public HtmlTestTools
{
private:
    FieldUnit m_eUnit;

public:
    HtmlExportTest() :
        SwModelTestBase("/sw/qa/extras/htmlexport/data/", "HTML (StarWriter)"),
        m_eUnit(FieldUnit::NONE)
    {}

    /**
     * Wraps a reqif-xhtml fragment into an XHTML file, so an XML parser can
     * parse it.
     */
    void wrapFragment(SvMemoryStream& rStream)
    {
        rStream.WriteCharPtr(
            "<reqif-xhtml:html xmlns:reqif-xhtml=\"http://www.w3.org/1999/xhtml\">\n");
        SvFileStream aFileStream(maTempFile.GetURL(), StreamMode::READ);
        rStream.WriteStream(aFileStream);
        rStream.WriteCharPtr("</reqif-xhtml:html>\n");
        rStream.Seek(0);
    }

private:
    bool mustCalcLayoutOf(const char* filename) override
    {
        return OString(filename) != "fdo62336.docx";
    }

    bool mustTestImportOf(const char* filename) const override
    {
        return OString(filename) != "fdo62336.docx";
    }

    virtual std::unique_ptr<Resetter> preTest(const char* filename) override
    {
        if (getTestName().indexOf("SkipImages") != -1)
            setFilterOptions("SkipImages");
        else if (getTestName().indexOf("EmbedImages") != -1)
            setFilterOptions("EmbedImages");
        else if (getTestName().indexOf("XHTML") != -1)
            setFilterOptions("XHTML");
        else if (getTestName().indexOf("ReqIf") != -1)
        {
            if (OString(filename).endsWith(".xhtml"))
            {
                setImportFilterOptions("xhtmlns=reqif-xhtml");
                // Bypass filter detect.
                setImportFilterName("HTML (StarWriter)");
            }
            // Export options (implies XHTML).
            setFilterOptions("xhtmlns=reqif-xhtml");
        }
        else
            setFilterOptions("");

        if (OString(filename) == "charborder.odt")
        {
            // FIXME if padding-top gets exported as inches, not cms, we get rounding errors.
            SwGlobals::ensure(); // make sure that SW_MOD() is not 0
            std::unique_ptr<Resetter> pResetter(new Resetter(
                [this] () {
                    SwMasterUsrPref* pPref = const_cast<SwMasterUsrPref*>(SW_MOD()->GetUsrPref(false));
                    pPref->SetMetric(this->m_eUnit);
                }));
            SwMasterUsrPref* pPref = const_cast<SwMasterUsrPref*>(SW_MOD()->GetUsrPref(false));
            m_eUnit = pPref->GetMetric();
            pPref->SetMetric(FieldUnit::CM);
            return pResetter;
        }
        return nullptr;
    }

};

#define DECLARE_HTMLEXPORT_ROUNDTRIP_TEST(TestName, filename) DECLARE_SW_ROUNDTRIP_TEST(TestName, filename, nullptr, HtmlExportTest)

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

#define DECLARE_HTMLEXPORT_TEST(TestName, filename) DECLARE_SW_EXPORT_TEST(TestName, filename, nullptr, HtmlExportTest)

DECLARE_HTMLEXPORT_TEST(testExportOfImages, "textAndImage.docx")
{
    htmlDocPtr pDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/html/body", 1);
    assertXPath(pDoc, "/html/body/p/img", 1);
}

DECLARE_HTMLEXPORT_TEST(testExportOfImagesWithSkipImagesEnabled, "textAndImage.docx")
{
    htmlDocPtr pDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/html/body", 1);
    assertXPath(pDoc, "/html/body/p/img", 0);
}

DECLARE_HTMLEXPORT_TEST(testSkipImagesEmbedded, "skipimage-embedded.doc")
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

DECLARE_HTMLEXPORT_TEST(testSkipImagesEmbeddedDocument, "skipimage-embedded-document.docx")
{
    // Similar to testSkipImagesEmbedded, but with an embedded Writer object,
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

DECLARE_HTMLEXPORT_TEST(testExportImageBulletList, "tdf66822.odt")
{
    htmlDocPtr pDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pDoc);

    // Encoded base64 SVG bullet should match and render on browser
    assertXPath(pDoc, "/html/body/ul", 1);
    assertXPath(pDoc, "/html/body/ul", "style", "list-style-image: url(data:image/svg+xml;base64,PD94bWwgdmVyc2lvbj0iMS4wIiBlbmNvZGluZz0idXRmLTgiPz4NCjwhLS0gR2VuZXJhdG9yOiBBZG9iZSBJbGx1c3RyYXRvciAxMi4wLjEsIFNWRyBFeHBvcnQgUGx1Zy1JbiAuIFNWRyBWZXJzaW9uOiA2LjAwIEJ1aWxkIDUxNDQ4KSAgLS0+DQo8IURPQ1RZUEUgc3ZnIFBVQkxJQyAiLS8vVzNDLy9EVEQgU1ZHIDEuMS8vRU4iICJodHRwOi8vd3d3LnczLm9yZy9HcmFwaGljcy9TVkcvMS4xL0RURC9zdmcxMS5kdGQiIFsNCgk8IUVOVElUWSBuc19zdmcgImh0dHA6Ly93d3cudzMub3JnLzIwMDAvc3ZnIj4NCgk8IUVOVElUWSBuc194bGluayAiaHR0cDovL3d3dy53My5vcmcvMTk5OS94bGluayI+DQpdPg0KPHN2ZyAgdmVyc2lvbj0iMS4xIiBpZD0iTGF5ZXJfMSIgeG1sbnM9IiZuc19zdmc7IiB4bWxuczp4bGluaz0iJm5zX3hsaW5rOyIgd2lkdGg9IjE0LjAwOCIgaGVpZ2h0PSIxNC4wMSINCgkgdmlld0JveD0iMCAwIDE0LjAwOCAxNC4wMSIgb3ZlcmZsb3c9InZpc2libGUiIGVuYWJsZS1iYWNrZ3JvdW5kPSJuZXcgMCAwIDE0LjAwOCAxNC4wMSIgeG1sOnNwYWNlPSJwcmVzZXJ2ZSI+DQo8Zz4NCgk8cmFkaWFsR3JhZGllbnQgaWQ9IlhNTElEXzRfIiBjeD0iNy4wMDQ0IiBjeT0iNy4wMDQ5IiByPSI3LjAwNDQiIGdyYWRpZW50VW5pdHM9InVzZXJTcGFjZU9uVXNlIj4NCgkJPHN0b3AgIG9mZnNldD0iMCIgc3R5bGU9InN0b3AtY29sb3I6IzM1REIzNSIvPg0KCQk8c3RvcCAgb2Zmc2V0PSIxIiBzdHlsZT0ic3RvcC1jb2xvcjojMDBBMDAwIi8+DQoJPC9yYWRpYWxHcmFkaWVudD4NCgk8Y2lyY2xlIGZpbGw9InVybCgjWE1MSURfNF8pIiBjeD0iNy4wMDQiIGN5PSI3LjAwNSIgcj0iNy4wMDQiLz4NCgk8ZGVmcz4NCgkJPGZpbHRlciBpZD0iQWRvYmVfT3BhY2l0eU1hc2tGaWx0ZXIiIGZpbHRlclVuaXRzPSJ1c2VyU3BhY2VPblVzZSIgeD0iMy40ODEiIHk9IjAuNjkzIiB3aWR0aD0iNi45ODgiIGhlaWdodD0iMy44OTMiPg0KCQkJPGZlQ29sb3JNYXRyaXggIHR5cGU9Im1hdHJpeCIgdmFsdWVzPSIxIDAgMCAwIDAgIDAgMSAwIDAgMCAgMCAwIDEgMCAwICAwIDAgMCAxIDAiLz4NCgkJPC9maWx0ZXI+DQoJPC9kZWZzPg0KCTxtYXNrIG1hc2tVbml0cz0idXNlclNwYWNlT25Vc2UiIHg9IjMuNDgxIiB5PSIwLjY5MyIgd2lkdGg9IjYuOTg4IiBoZWlnaHQ9IjMuODkzIiBpZD0iWE1MSURfNV8iPg0KCQk8ZyBmaWx0ZXI9InVybCgjQWRvYmVfT3BhY2l0eU1hc2tGaWx0ZXIpIj4NCgkJCTxsaW5lYXJHcmFkaWVudCBpZD0iWE1MSURfNl8iIGdyYWRpZW50VW5pdHM9InVzZXJTcGFjZU9uVXNlIiB4MT0iNy4xMjIxIiB5MT0iMC4xMDMiIHgyPSI3LjEyMjEiIHkyPSI1LjIzNDQiPg0KCQkJCTxzdG9wICBvZmZzZXQ9IjAiIHN0eWxlPSJzdG9wLWNvbG9yOiNGRkZGRkYiLz4NCgkJCQk8c3RvcCAgb2Zmc2V0PSIxIiBzdHlsZT0ic3RvcC1jb2xvcjojMDAwMDAwIi8+DQoJCQk8L2xpbmVhckdyYWRpZW50Pg0KCQkJPHJlY3QgeD0iMy4xOTkiIHk9IjAuMzM5IiBvcGFjaXR5PSIwLjciIGZpbGw9InVybCgjWE1MSURfNl8pIiB3aWR0aD0iNy44NDYiIGhlaWdodD0iNC42MDEiLz4NCgkJPC9nPg0KCTwvbWFzaz4NCgk8ZWxsaXBzZSBtYXNrPSJ1cmwoI1hNTElEXzVfKSIgZmlsbD0iI0ZGRkZGRiIgY3g9IjYuOTc1IiBjeT0iMi42NCIgcng9IjMuNDk0IiByeT0iMS45NDYiLz4NCjwvZz4NCjwvc3ZnPg0K);");
}

DECLARE_HTMLEXPORT_TEST(testTdf83890, "tdf83890.odt")
{
    htmlDocPtr pDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/html/body/ol[2]/ol", "start", "2");
}

DECLARE_HTMLEXPORT_TEST(testExtbChars, "extb.html")
{
    sal_uInt32  nCh = 0x24b62;
    OUString aExpected( &nCh, 1);
    // Assert that UTF8 encoded non-BMP Unicode character is correct
    uno::Reference<text::XTextRange> xTextRange1 = getRun(getParagraph(1), 1);
    CPPUNIT_ASSERT_EQUAL(aExpected, xTextRange1->getString());

    // Assert that non-BMP Unicode in character entity format is correct
    uno::Reference<text::XTextRange> xTextRange2 = getRun(getParagraph(2), 1);
    CPPUNIT_ASSERT_EQUAL(aExpected, xTextRange2->getString());
}

DECLARE_HTMLEXPORT_TEST(testNormalImageExport, "textAndImage.docx")
{
    htmlDocPtr pDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/html/body", 1);
    OUString imgSrc = getXPath(pDoc, "/html/body/p/img", "src");
    CPPUNIT_ASSERT(imgSrc.endsWith(".png"));
}

DECLARE_HTMLEXPORT_TEST(testEmbedImagesEnabled, "textAndImage.docx")
{
    htmlDocPtr pDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/html/body", 1);
    assertXPath(pDoc, "/html/body/p/img", "src", "data:image/png;base64,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");
}

DECLARE_HTMLEXPORT_TEST(testXHTML, "hello.html")
{
    OString aExpected("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML");
    SvStream* pStream = maTempFile.GetStream(StreamMode::READ);
    CPPUNIT_ASSERT(pStream);
    OString aActual(read_uInt8s_ToOString(*pStream, aExpected.getLength()));
    // This was HTML, not XHTML.
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);

    htmlDocPtr pDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pDoc);
    // This was lang, not xml:lang.
    assertXPath(pDoc, "/html/body", "xml:lang", "en-US");
}

DECLARE_HTMLEXPORT_TEST(testReqIfParagraph, "reqif-p.xhtml")
{
    SvStream* pStream = maTempFile.GetStream(StreamMode::READ);
    CPPUNIT_ASSERT(pStream);
    sal_uInt64 nLength = pStream->TellEnd();

    OString aExpected("<reqif-xhtml:div><reqif-xhtml:p>aaa<reqif-xhtml:br/>\nbbb"
                      "</reqif-xhtml:p>" SAL_NEWLINE_STRING);

    // This was '<table' instead.
    aExpected += "<reqif-xhtml:table";

    OString aStream(read_uInt8s_ToOString(*pStream, nLength));
    pStream->Seek(0);
    OString aActual(read_uInt8s_ToOString(*pStream, aExpected.getLength()));
    // This was a HTML header, like '<!DOCTYPE html ...'.
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);

    // This was "<a", was not found.
    CPPUNIT_ASSERT(aStream.indexOf("<reqif-xhtml:a") != -1);

    // This was "<u>" instead of CSS.
    CPPUNIT_ASSERT(aStream.indexOf("<reqif-xhtml:span style=\"text-decoration: underline\"") != -1);

    // This was <strong>, namespace prefix was missing.
    CPPUNIT_ASSERT(aStream.indexOf("<reqif-xhtml:strong>") != -1);

    // This was "<strike>" instead of CSS.
    CPPUNIT_ASSERT(aStream.indexOf("<reqif-xhtml:span style=\"text-decoration: line-through\"") != -1);

    // This was "<font>" instead of CSS + namespace prefix was missing.
    CPPUNIT_ASSERT(aStream.indexOf("<reqif-xhtml:span style=\"color: #ce181e\"") != -1);

    // This was '<a name="Bookmark 1"': missing namespace prefix, wrong
    // attribute name, wrong attribute value.
    CPPUNIT_ASSERT(aStream.indexOf("<reqif-xhtml:a id=\"Bookmark_1\"></reqif-xhtml:a>") != -1);
}

DECLARE_HTMLEXPORT_ROUNDTRIP_TEST(testReqIfOleData, "reqif-ole-data.xhtml")
{
    uno::Reference<text::XTextEmbeddedObjectsSupplier> xSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xObjects(xSupplier->getEmbeddedObjects(),
                                                     uno::UNO_QUERY);
    // This was 0, <object> without URL was ignored.
    // Then this was 0 on export, as data of OLE nodes was ignored.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), xObjects->getCount());
}

DECLARE_HTMLEXPORT_ROUNDTRIP_TEST(testReqIfOleImg, "reqif-ole-img.xhtml")
{
    uno::Reference<text::XTextEmbeddedObjectsSupplier> xSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xObjects(xSupplier->getEmbeddedObjects(),
                                                     uno::UNO_QUERY);
    uno::Reference<document::XEmbeddedObjectSupplier2> xObject(xObjects->getByIndex(0),
                                                               uno::UNO_QUERY);
    // This failed, OLE object had no replacement image.
    // And then it also failed when the export lost the replacement image.
    uno::Reference<graphic::XGraphic> xGraphic = xObject->getReplacementGraphic();
    // This failed when query and fragment of file:// URLs were not ignored.
    CPPUNIT_ASSERT(xGraphic.is());

    uno::Reference<drawing::XShape> xShape(xObject, uno::UNO_QUERY);
    OutputDevice* pDevice = Application::GetDefaultDevice();
    Size aPixel(64, 64);
    // Expected to be 1693.
    Size aLogic(pDevice->PixelToLogic(aPixel, MapMode(MapUnit::Map100thMM)));
    awt::Size aSize = xShape->getSize();
    // This was only 1247, size was not set explicitly.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(aLogic.getWidth()), aSize.Width);

    // Check mime/media types.
    CPPUNIT_ASSERT_EQUAL(OUString("image/png"), getProperty<OUString>(xGraphic, "MimeType"));

    uno::Reference<document::XStorageBasedDocument> xStorageProvider(mxComponent, uno::UNO_QUERY);
    uno::Reference<embed::XStorage> xStorage = xStorageProvider->getDocumentStorage();
    auto aStreamName = getProperty<OUString>(xObject, "StreamName");
    uno::Reference<io::XStream> xStream
        = xStorage->openStreamElement(aStreamName, embed::ElementModes::READ);
    // This was empty when either import or export handling was missing.
    CPPUNIT_ASSERT_EQUAL(OUString("text/rtf"), getProperty<OUString>(xStream, "MediaType"));

    // Check alternate text (it was empty, for export the 'alt' attribute was used).
    CPPUNIT_ASSERT_EQUAL(OUString("OLE Object"), getProperty<OUString>(xObject, "Title").trim());

    if (!mbExported)
        return;

    // "type" attribute was missing for the inner <object> element.
    SvStream* pStream = maTempFile.GetStream(StreamMode::READ);
    CPPUNIT_ASSERT(pStream);
    sal_uInt64 nLength = pStream->TellEnd();
    OString aStream(read_uInt8s_ToOString(*pStream, nLength));
    CPPUNIT_ASSERT(aStream.indexOf("type=\"image/png\"") != -1);
}

DECLARE_HTMLEXPORT_ROUNDTRIP_TEST(testReqIfPngImg, "reqif-png-img.xhtml")
{
    uno::Reference<container::XNamed> xShape(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xShape.is());

    if (!mbExported)
    {
        // Imported PNG image is not an object.
        CPPUNIT_ASSERT_EQUAL(OUString("Image1"), xShape->getName());
        return;
    }

    // All images are exported as objects in ReqIF mode.
    CPPUNIT_ASSERT_EQUAL(OUString("Object1"), xShape->getName());

    // This was <img>, not <object>, which is not valid in the reqif-xhtml
    // subset.
    SvStream* pStream = maTempFile.GetStream(StreamMode::READ);
    CPPUNIT_ASSERT(pStream);
    sal_uInt64 nLength = pStream->TellEnd();
    OString aStream(read_uInt8s_ToOString(*pStream, nLength));
    CPPUNIT_ASSERT(aStream.indexOf("<reqif-xhtml:object") != -1);

    // Make sure that both RTF and PNG versions are written.
    CPPUNIT_ASSERT(aStream.indexOf("text/rtf") != -1);
    // This failed when images with a query in their file:// URL failed to
    // import.
    CPPUNIT_ASSERT(aStream.indexOf("image/png") != -1);
}

DECLARE_HTMLEXPORT_TEST(testReqIfJpgImg, "reqif-jpg-img.xhtml")
{
    SvStream* pStream = maTempFile.GetStream(StreamMode::READ);
    CPPUNIT_ASSERT(pStream);
    sal_uInt64 nLength = pStream->TellEnd();
    OString aStream(read_uInt8s_ToOString(*pStream, nLength));
    // This was image/jpeg, JPG was not converted to PNG in ReqIF mode.
    CPPUNIT_ASSERT(aStream.indexOf("type=\"image/png\"") != -1);
}

DECLARE_HTMLEXPORT_TEST(testReqIfTable, "reqif-table.xhtml")
{
    htmlDocPtr pDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pDoc);

    // <div> was missing, so the XHTML fragment wasn't a valid
    // xhtml.BlkStruct.class type anymore.
    assertXPath(pDoc, "/html/body/div/table/tr/th", 1);
    // The attribute was present to contain "background" and "border", which is
    // ignored in reqif-xhtml.
    assertXPathNoAttribute(pDoc, "/html/body/div/table/tr/th", "style");
    // The attribute was present, which is not valid in reqif-xhtml.
    assertXPathNoAttribute(pDoc, "/html/body/div/table/tr/th", "bgcolor");
}

DECLARE_HTMLEXPORT_TEST(testReqIfTable2, "reqif-table2.odt")
{
    SvStream* pStream = maTempFile.GetStream(StreamMode::READ);
    CPPUNIT_ASSERT(pStream);
    sal_uInt64 nLength = pStream->TellEnd();
    OString aStream(read_uInt8s_ToOString(*pStream, nLength));
    // This failed, <reqif-xhtml:td width="..."> was written.
    CPPUNIT_ASSERT(aStream.indexOf("<reqif-xhtml:td>") != -1);
}

DECLARE_HTMLEXPORT_TEST(testReqIfWellFormed, "reqif.odt")
{
    SvStream* pStream = maTempFile.GetStream(StreamMode::READ);
    CPPUNIT_ASSERT(pStream);
    sal_uInt64 nLength = pStream->TellEnd();
    OString aStream(read_uInt8s_ToOString(*pStream, nLength));
    // This failed, <font face="..."> was written.
    CPPUNIT_ASSERT(aStream.indexOf("<reqif-xhtml:span style=\"font-family:") != -1);
    // This failed, <font size="..."> was written.
    CPPUNIT_ASSERT(aStream.indexOf("<reqif-xhtml:span style=\"font-size:") != -1);
}

DECLARE_HTMLEXPORT_TEST(testReqIfList, "reqif-list.xhtml")
{
    SvStream* pStream = maTempFile.GetStream(StreamMode::READ);
    CPPUNIT_ASSERT(pStream);
    sal_uInt64 nLength = pStream->TellEnd();
    OString aStream(read_uInt8s_ToOString(*pStream, nLength));
    // This failed, <ul> was written.
    CPPUNIT_ASSERT(aStream.indexOf("<reqif-xhtml:ul>") != -1);

    // This failed, the 'style' attribute was written, even if the input did
    // not had one.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-1), aStream.indexOf(" style=\""));

    // This failed <li> was only opened, not closed.
    CPPUNIT_ASSERT(aStream.indexOf("</reqif-xhtml:li>") != -1);
}

DECLARE_HTMLEXPORT_ROUNDTRIP_TEST(testReqIfOle2, "reqif-ole2.xhtml")
{
    uno::Reference<text::XTextEmbeddedObjectsSupplier> xSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xObjects(xSupplier->getEmbeddedObjects(),
                                                     uno::UNO_QUERY);
    uno::Reference<document::XEmbeddedObjectSupplier2> xObject(xObjects->getByIndex(0),
                                                               uno::UNO_QUERY);
    uno::Reference<io::XActiveDataStreamer> xEmbeddedObject(xObject->getExtendedControlOverEmbeddedObject(), uno::UNO_QUERY);
    // This failed, the "RTF fragment" native data was loaded as-is, we had no
    // filter to handle it, so nothing happened on double-click.
    CPPUNIT_ASSERT(xEmbeddedObject.is());
    uno::Reference<io::XSeekable> xStream(xEmbeddedObject->getStream(), uno::UNO_QUERY);
    // This was 38375, msfilter::rtfutil::ExtractOLE2FromObjdata() wrote
    // everything after the OLE1 header into the OLE2 stream, while the
    // Presentation field after the OLE2 data doesn't belong there.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int64>(37888), xStream->getLength());
    // Finally the export also failed as it tried to open the stream from the
    // document storage, but the embedded object already opened it, so an
    // exception of type com.sun.star.io.IOException was thrown.

    if (mbExported)
    {
        // Check that the replacement graphic is exported at RTF level.
        SvMemoryStream aStream;
        wrapFragment(aStream);
        xmlDocPtr pDoc = parseXmlStream(&aStream);
        CPPUNIT_ASSERT(pDoc);
        // Get the path of the RTF data.
        OUString aOlePath = getXPath(
            pDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:p/reqif-xhtml:object", "data");
        OUString aOleSuffix(".ole");
        CPPUNIT_ASSERT(aOlePath.endsWith(aOleSuffix));
        INetURLObject aUrl(maTempFile.GetURL());
        aUrl.setBase(aOlePath.copy(0, aOlePath.getLength() - aOleSuffix.getLength()));
        aUrl.setExtension("ole");
        OUString aOleUrl = aUrl.GetMainURL(INetURLObject::DecodeMechanism::NONE);

        // Search for \result in the RTF data.
        SvFileStream aOleStream(aOleUrl, StreamMode::READ);
        CPPUNIT_ASSERT(aOleStream.IsOpen());
        OString aOleString(read_uInt8s_ToOString(aOleStream, aOleStream.TellEnd()));
        // Without the accompanying fix in place, this test would have failed,
        // replacement graphic was missing at RTF level.
        CPPUNIT_ASSERT(aOleString.indexOf(OOO_STRING_SVTOOLS_RTF_RESULT) != -1);
    }
}

DECLARE_HTMLEXPORT_ROUNDTRIP_TEST(testReqIfOle2Odg, "reqif-ole-odg.xhtml")
{
    uno::Reference<text::XTextEmbeddedObjectsSupplier> xSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xObjects(xSupplier->getEmbeddedObjects(),
                                                     uno::UNO_QUERY);
    uno::Reference<document::XEmbeddedObjectSupplier> xTextEmbeddedObject(xObjects->getByIndex(0),
                                                                          uno::UNO_QUERY);
    uno::Reference<lang::XServiceInfo> xObject(xTextEmbeddedObject->getEmbeddedObject(),
                                               uno::UNO_QUERY);
    // This failed, both import and export failed to handle OLE2 that contains
    // just ODF.
    CPPUNIT_ASSERT(xObject.is());
    CPPUNIT_ASSERT(xObject->supportsService("com.sun.star.drawing.DrawingDocument"));
}

DECLARE_HTMLEXPORT_TEST(testList, "list.html")
{
    SvStream* pStream = maTempFile.GetStream(StreamMode::READ);
    CPPUNIT_ASSERT(pStream);
    sal_uInt64 nLength = pStream->TellEnd();
    OString aStream(read_uInt8s_ToOString(*pStream, nLength));
    // This failed, it was <li/>, i.e. list item was closed before content
    // started.
    CPPUNIT_ASSERT(aStream.indexOf("<li>") != -1);
}

DECLARE_HTMLEXPORT_TEST(testTransparentImage, "transparent-image.odt")
{
    htmlDocPtr pDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pDoc);

    OUString aSource = getXPath(pDoc, "/html/body/p/img", "src");
    OUString aMessage = "src attribute is: " + aSource;
    // This was a jpeg, transparency was lost.
    CPPUNIT_ASSERT_MESSAGE(aMessage.toUtf8().getStr(), aSource.endsWith(".gif"));
}

DECLARE_HTMLEXPORT_TEST(testTransparentImageReqIf, "transparent-image.odt")
{
    SvMemoryStream aStream;
    wrapFragment(aStream);
    xmlDocPtr pDoc = parseXmlStream(&aStream);
    CPPUNIT_ASSERT(pDoc);

    OUString aSource = getXPath(
        pDoc,
        "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:p/reqif-xhtml:object/reqif-xhtml:object",
        "data");
    OUString aMessage = "src attribute is: " + aSource;
    // This was GIF, when the intention was to force PNG.
    CPPUNIT_ASSERT_MESSAGE(aMessage.toUtf8().getStr(), aSource.endsWith(".png"));
}

DECLARE_HTMLEXPORT_TEST(testOleNodataReqIf, "reqif-ole-nodata.odt")
{
    // This failed, io::IOException was thrown during the filter() call.
    SvMemoryStream aStream;
    wrapFragment(aStream);
    xmlDocPtr pDoc = parseXmlStream(&aStream);
    CPPUNIT_ASSERT(pDoc);

    // Make sure the native <object> element has the required data attribute.
    OUString aSource = getXPath(
        pDoc,
        "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:p/reqif-xhtml:object/reqif-xhtml:object",
        "data");
    CPPUNIT_ASSERT(!aSource.isEmpty());
}

DECLARE_HTMLEXPORT_TEST(testNoLangReqIf, "reqif-no-lang.odt")
{
    SvMemoryStream aStream;
    wrapFragment(aStream);
    xmlDocPtr pDoc = parseXmlStream(&aStream);
    CPPUNIT_ASSERT(pDoc);

    // Make sure that xml:lang is not written in ReqIF mode.
    assertXPathNoAttribute(pDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:h1", "lang");
}

DECLARE_HTMLEXPORT_TEST(testFieldShade, "field-shade.odt")
{
    htmlDocPtr pDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pDoc);

    // Without the accompanying fix in place, this test would have failed with 'Expected: 1; Actual:
    // 0', i.e. shading for the field was lost.
    assertXPath(pDoc, "/html/body/p[1]/span", "style", "background: #c0c0c0");

    // Check that field shading is written only in case there is no user-defined span background.
    assertXPath(pDoc, "/html/body/p[2]/span", "style", "background: #ff0000");
    // Without the accompanying fix in place, this test would have failed with 'Expected: 0; Actual:
    // 1', i.e there was an inner span hiding the wanted background color.
    assertXPath(pDoc, "/html/body/p[2]/span/span", 0);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
