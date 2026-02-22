/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "htmlmodeltestbase.hxx"

#include <memory>

#include <com/sun/star/document/XEmbeddedObjectSupplier2.hpp>
#include <com/sun/star/io/XActiveDataStreamer.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextEmbeddedObjectsSupplier.hpp>
#include <com/sun/star/packages/zip/ZipFileAccess.hpp>

#include <svtools/rtfkeywd.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/scopeguard.hxx>
#include <sot/storage.hxx>
#include <tools/globname.hxx>
#include <vcl/svapp.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <comphelper/processfactory.hxx>
#include <vcl/dibtools.hxx>

#include <swmodule.hxx>
#include <swdll.hxx>
#include <usrpref.hxx>
#include <wrtsh.hxx>
#include <ndtxt.hxx>
#include <docsh.hxx>
#include <unotxdoc.hxx>

namespace
{
CPPUNIT_TEST_FIXTURE(HtmlExportTest, testFdo81276)
{
    auto verify = [this]() {
        uno::Reference<container::XNameAccess> xPageStyles(getStyles(u"PageStyles"_ustr));
        uno::Reference<beans::XPropertySet> xStyle(xPageStyles->getByName(u"HTML"_ustr),
                                                   uno::UNO_QUERY);
        // some rounding going on here?
        CPPUNIT_ASSERT(abs(sal_Int32(29700) - getProperty<sal_Int32>(xStyle, u"Width"_ustr)) < 10);
        CPPUNIT_ASSERT(abs(sal_Int32(21006) - getProperty<sal_Int32>(xStyle, u"Height"_ustr)) < 10);
        CPPUNIT_ASSERT(abs(sal_Int32(500) - getProperty<sal_Int32>(xStyle, u"LeftMargin"_ustr))
                       < 10);
        CPPUNIT_ASSERT(abs(sal_Int32(500) - getProperty<sal_Int32>(xStyle, u"RightMargin"_ustr))
                       < 10);
        CPPUNIT_ASSERT(abs(sal_Int32(2000) - getProperty<sal_Int32>(xStyle, u"TopMargin"_ustr))
                       < 10);
        CPPUNIT_ASSERT(abs(sal_Int32(500) - getProperty<sal_Int32>(xStyle, u"BottomMargin"_ustr))
                       < 10);
    };
    setImportFilterName(TestFilter::HTML_WRITER);
    createSwDoc("fdo81276.html");
    verify();
    saveAndReload(TestFilter::HTML_WRITER);
    verify();
}

CPPUNIT_TEST_FIXTURE(HtmlExportTest, testFdo62336)
{
    // The problem was essentially a crash during table export as docx/rtf/html
    // If calc-layout is enabled, the crash does not occur, that's why loadFromFile/save is used
    loadFromFile(u"fdo62336.docx");
    save(TestFilter::HTML_WRITER);
}

CPPUNIT_TEST_FIXTURE(HtmlExportTest, testFdo86857)
{
    auto verify = [this]() {
        // problem was that background color on page style was not exported
        uno::Reference<container::XNameAccess> xPageStyles(getStyles(u"PageStyles"_ustr));
        uno::Reference<beans::XPropertySet> xStyle(xPageStyles->getByName(u"HTML"_ustr),
                                                   uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, getProperty<Color>(xStyle, u"BackColor"_ustr));
        // check that table background color works, which still uses RES_BACKGROUND
        uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                        uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
        uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xCell(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(Color(0x66ffff), getProperty<Color>(xCell, u"BackColor"_ustr));
    };

    setImportFilterName(TestFilter::HTML_WRITER);
    createSwDoc("fdo86857.html");
    verify();
    saveAndReload(TestFilter::HTML_WRITER);
    verify();
}

CPPUNIT_TEST_FIXTURE(HtmlExportTest, testCharacterBorder)
{
    // FIXME if padding-top gets exported as inches, not cms, we get rounding errors.
    SwGlobals::ensure(); // make sure that SwModule::get() is not 0
    SwMasterUsrPref* pPref = const_cast<SwMasterUsrPref*>(SwModule::get()->GetUsrPref(false));
    FieldUnit eUnit = pPref->GetMetric();
    pPref->SetMetric(FieldUnit::CM);
    comphelper::ScopeGuard g([pPref, eUnit]() { pPref->SetMetric(eUnit); });

    auto verify = [this]() {
        CPPUNIT_ASSERT_EQUAL(1, getPages());

        uno::Reference<beans::XPropertySet> xRun(getRun(getParagraph(1), 1), uno::UNO_QUERY);
        // Different Border
        {
            CPPUNIT_ASSERT_BORDER_EQUAL(
                table::BorderLine2(0x6666FF, 12, 12, 12, 3, 37),
                getProperty<table::BorderLine2>(xRun, u"CharTopBorder"_ustr));
            CPPUNIT_ASSERT_BORDER_EQUAL(
                table::BorderLine2(0xFF9900, 0, 99, 0, 2, 99),
                getProperty<table::BorderLine2>(xRun, u"CharLeftBorder"_ustr));
            CPPUNIT_ASSERT_BORDER_EQUAL(
                table::BorderLine2(0xFF0000, 0, 169, 0, 1, 169),
                getProperty<table::BorderLine2>(xRun, u"CharBottomBorder"_ustr));
            CPPUNIT_ASSERT_BORDER_EQUAL(
                table::BorderLine2(0x0000FF, 0, 169, 0, 0, 169),
                getProperty<table::BorderLine2>(xRun, u"CharRightBorder"_ustr));
        }

        // Different Padding
        {
            CPPUNIT_ASSERT_EQUAL(sal_Int32(450),
                                 getProperty<sal_Int32>(xRun, u"CharTopBorderDistance"_ustr));
            CPPUNIT_ASSERT_EQUAL(sal_Int32(550),
                                 getProperty<sal_Int32>(xRun, u"CharLeftBorderDistance"_ustr));
            CPPUNIT_ASSERT_EQUAL(sal_Int32(150),
                                 getProperty<sal_Int32>(xRun, u"CharBottomBorderDistance"_ustr));
            CPPUNIT_ASSERT_EQUAL(sal_Int32(250),
                                 getProperty<sal_Int32>(xRun, u"CharRightBorderDistance"_ustr));
        }

        // No shadow
    };
    createSwDoc("charborder.odt");
    verify();
    saveAndReload(TestFilter::HTML_WRITER);
    verify();
}

CPPUNIT_TEST_FIXTURE(HtmlExportTest, testExportOfImages)
{
    createSwDoc("textAndImage.docx");
    save(TestFilter::HTML_WRITER);
    htmlDocUniquePtr pDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/html/body", 1);
    assertXPath(pDoc, "/html/body/p/img", 1);
}

CPPUNIT_TEST_FIXTURE(HtmlExportTest, testExportOfImagesWithSkipImagesEnabled)
{
    createSwDoc("textAndImage.docx");
    save(TestFilter::HTML_WRITER,
         {
             comphelper::makePropertyValue(u"FilterOptions"_ustr, u"SkipImages"_ustr),
         });

    htmlDocUniquePtr pDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/html/body", 1);
    assertXPath(pDoc, "/html/body/p/img", 0);
}

CPPUNIT_TEST_FIXTURE(HtmlExportTest, testSkipImagesEmbedded)
{
    createSwDoc("skipimage-embedded.doc");
    save(TestFilter::HTML_WRITER,
         {
             comphelper::makePropertyValue(u"FilterOptions"_ustr, u"SkipImages"_ustr),
         });

    // Embedded spreadsheet was exported as image, so content was lost. Make
    // sure it's exported as HTML instead.
    htmlDocUniquePtr pDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pDoc);

    // This was 0.
    assertXPath(pDoc, "//table", 1);
    // This was 2, the HTML header was in the document two times.
    assertXPath(pDoc, "//meta[@name='generator']", 1);
    // This was 0, <table> was directly under <p>, which caused errors in the parser.
    assertXPath(pDoc, "//span/table", 1);
}

CPPUNIT_TEST_FIXTURE(HtmlExportTest, testSkipImagesEmbeddedDocument)
{
    createSwDoc("skipimage-embedded-document.docx");
    save(TestFilter::HTML_WRITER,
         {
             comphelper::makePropertyValue(u"FilterOptions"_ustr, u"SkipImages"_ustr),
         });

    // Similar to testSkipImagesEmbedded, but with an embedded Writer object,
    // not a Calc one, and this time OOXML, not WW8.
    htmlDocUniquePtr pDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pDoc);

    // This was 2, the HTML header was in the document two times.
    assertXPath(pDoc, "//meta[@name='generator']", 1);
    // Text of embedded document was missing.
    assertXPathContent(pDoc, "/html/body/p/span/p/span", u"Inner.");
}

CPPUNIT_TEST_FIXTURE(HtmlExportTest, testExportImageProperties)
{
    createSwDoc("HTMLImage.odt");
    save(TestFilter::HTML_WRITER);
    htmlDocUniquePtr pDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/html/body", 1);

    assertXPath(pDoc, "/html/body/p/map/area", "shape", u"poly");
    assertXPath(pDoc, "/html/body/p/map/area", "href", u"http://www.microsoft.com/");
    assertXPath(pDoc, "/html/body/p/map/area", "target", u"_self");
    assertXPath(pDoc, "/html/body/p/map/area", "alt", u"microsoft");

    assertXPath(pDoc, "/html/body/p/a", 1);
    assertXPath(pDoc, "/html/body/p/a", "href", u"http://www.google.com/");

    assertXPath(pDoc, "/html/body/p/a/font", 1);
    assertXPath(pDoc, "/html/body/p/a/font", "color", u"#ff0000");

    assertXPath(pDoc, "/html/body/p/a/font/img", 1);
    assertXPath(pDoc, "/html/body/p/a/font/img", "name", u"Text");
    assertXPath(pDoc, "/html/body/p/a/font/img", "alt", u"Four colors");
    assertXPath(pDoc, "/html/body/p/a/font/img", "align", u"middle");

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

    assertXPath(pDoc, "/html/body/p/a/font/img", "usemap", u"#map1");
}

CPPUNIT_TEST_FIXTURE(HtmlExportTest, testExportCheckboxRadioButtonState)
{
    createSwDoc("checkbox-radiobutton.doc");
    save(TestFilter::HTML_WRITER);
    htmlDocUniquePtr pDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/html/body", 1);
    assertXPath(pDoc, "/html/body/p[1]/input", "type", u"checkbox");
    assertXPath(pDoc, "/html/body/p[1]/input", "checked", u"checked");
    assertXPath(pDoc, "/html/body/p[2]/input", "type", u"checkbox");
    assertXPathNoAttribute(pDoc, "/html/body/p[2]/input", "checked");
    assertXPath(pDoc, "/html/body/form/p[1]/input", "type", u"checkbox");
    assertXPath(pDoc, "/html/body/form/p[1]/input", "checked", u"checked");
    assertXPath(pDoc, "/html/body/form/p[2]/input", "type", u"checkbox");
    assertXPathNoAttribute(pDoc, "/html/body/form/p[2]/input", "checked");
    assertXPath(pDoc, "/html/body/form/p[3]/input", "type", u"radio");
    assertXPath(pDoc, "/html/body/form/p[3]/input", "checked", u"checked");
    assertXPath(pDoc, "/html/body/form/p[4]/input", "type", u"radio");
    assertXPathNoAttribute(pDoc, "/html/body/form/p[4]/input", "checked");
}

CPPUNIT_TEST_FIXTURE(HtmlExportTest, testExportUrlEncoding)
{
    createSwDoc("tdf76291.odt");
    save(TestFilter::HTML_WRITER);
    htmlDocUniquePtr pDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pDoc);

    // Test URI encoded hyperlink with Chinese characters
    assertXPath(pDoc, "/html/body/p/a", "href",
                u"http://www.youtube.com/results?search_query=%E7%B2%B5%E8%AA%9Emv&sm=12");
}

CPPUNIT_TEST_FIXTURE(HtmlExportTest, testExportInternalUrl)
{
    createSwDoc("tdf90905.odt");
    save(TestFilter::HTML_WRITER);
    htmlDocUniquePtr pDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pDoc);

    // Internal url should be valid
    assertXPath(pDoc, "/html/body/p[1]/a", "href", u"#0.0.1.Text|outline");
    assertXPath(pDoc, "/html/body/p[2]/a", "href", u"#bookmark");
}

CPPUNIT_TEST_FIXTURE(HtmlExportTest, testExportImageBulletList)
{
    createSwDoc("tdf66822.odt");
    save(TestFilter::HTML_WRITER);
    htmlDocUniquePtr pDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pDoc);

    // Encoded base64 SVG bullet should match and render on browser
    assertXPath(pDoc, "/html/body/ul", 1);
    assertXPath(
        pDoc, "/html/body/ul", "style",
        u"list-style-image: url(data:image/svg+xml;base64,"
        "PD94bWwgdmVyc2lvbj0iMS4wIiBlbmNvZGluZz0idXRmLTgiPz4NCjwhLS0gR2VuZXJhdG9yOiBBZG9iZSBJbGx1c3"
        "RyYXRvciAxMi4wLjEsIFNWRyBFeHBvcnQgUGx1Zy1JbiAuIFNWRyBWZXJzaW9uOiA2LjAwIEJ1aWxkIDUxNDQ4KSAg"
        "LS0+DQo8IURPQ1RZUEUgc3ZnIFBVQkxJQyAiLS8vVzNDLy9EVEQgU1ZHIDEuMS8vRU4iICJodHRwOi8vd3d3LnczLm"
        "9yZy9HcmFwaGljcy9TVkcvMS4xL0RURC9zdmcxMS5kdGQiIFsNCgk8IUVOVElUWSBuc19zdmcgImh0dHA6Ly93d3cu"
        "dzMub3JnLzIwMDAvc3ZnIj4NCgk8IUVOVElUWSBuc194bGluayAiaHR0cDovL3d3dy53My5vcmcvMTk5OS94bGluay"
        "I+DQpdPg0KPHN2ZyAgdmVyc2lvbj0iMS4xIiBpZD0iTGF5ZXJfMSIgeG1sbnM9IiZuc19zdmc7IiB4bWxuczp4bGlu"
        "az0iJm5zX3hsaW5rOyIgd2lkdGg9IjE0LjAwOCIgaGVpZ2h0PSIxNC4wMSINCgkgdmlld0JveD0iMCAwIDE0LjAwOC"
        "AxNC4wMSIgb3ZlcmZsb3c9InZpc2libGUiIGVuYWJsZS1iYWNrZ3JvdW5kPSJuZXcgMCAwIDE0LjAwOCAxNC4wMSIg"
        "eG1sOnNwYWNlPSJwcmVzZXJ2ZSI+DQo8Zz4NCgk8cmFkaWFsR3JhZGllbnQgaWQ9IlhNTElEXzRfIiBjeD0iNy4wMD"
        "Q0IiBjeT0iNy4wMDQ5IiByPSI3LjAwNDQiIGdyYWRpZW50VW5pdHM9InVzZXJTcGFjZU9uVXNlIj4NCgkJPHN0b3Ag"
        "IG9mZnNldD0iMCIgc3R5bGU9InN0b3AtY29sb3I6IzM1REIzNSIvPg0KCQk8c3RvcCAgb2Zmc2V0PSIxIiBzdHlsZT"
        "0ic3RvcC1jb2xvcjojMDBBMDAwIi8+DQoJPC9yYWRpYWxHcmFkaWVudD4NCgk8Y2lyY2xlIGZpbGw9InVybCgjWE1M"
        "SURfNF8pIiBjeD0iNy4wMDQiIGN5PSI3LjAwNSIgcj0iNy4wMDQiLz4NCgk8ZGVmcz4NCgkJPGZpbHRlciBpZD0iQW"
        "RvYmVfT3BhY2l0eU1hc2tGaWx0ZXIiIGZpbHRlclVuaXRzPSJ1c2VyU3BhY2VPblVzZSIgeD0iMy40ODEiIHk9IjAu"
        "NjkzIiB3aWR0aD0iNi45ODgiIGhlaWdodD0iMy44OTMiPg0KCQkJPGZlQ29sb3JNYXRyaXggIHR5cGU9Im1hdHJpeC"
        "IgdmFsdWVzPSIxIDAgMCAwIDAgIDAgMSAwIDAgMCAgMCAwIDEgMCAwICAwIDAgMCAxIDAiLz4NCgkJPC9maWx0ZXI+"
        "DQoJPC9kZWZzPg0KCTxtYXNrIG1hc2tVbml0cz0idXNlclNwYWNlT25Vc2UiIHg9IjMuNDgxIiB5PSIwLjY5MyIgd2"
        "lkdGg9IjYuOTg4IiBoZWlnaHQ9IjMuODkzIiBpZD0iWE1MSURfNV8iPg0KCQk8ZyBmaWx0ZXI9InVybCgjQWRvYmVf"
        "T3BhY2l0eU1hc2tGaWx0ZXIpIj4NCgkJCTxsaW5lYXJHcmFkaWVudCBpZD0iWE1MSURfNl8iIGdyYWRpZW50VW5pdH"
        "M9InVzZXJTcGFjZU9uVXNlIiB4MT0iNy4xMjIxIiB5MT0iMC4xMDMiIHgyPSI3LjEyMjEiIHkyPSI1LjIzNDQiPg0K"
        "CQkJCTxzdG9wICBvZmZzZXQ9IjAiIHN0eWxlPSJzdG9wLWNvbG9yOiNGRkZGRkYiLz4NCgkJCQk8c3RvcCAgb2Zmc2"
        "V0PSIxIiBzdHlsZT0ic3RvcC1jb2xvcjojMDAwMDAwIi8+DQoJCQk8L2xpbmVhckdyYWRpZW50Pg0KCQkJPHJlY3Qg"
        "eD0iMy4xOTkiIHk9IjAuMzM5IiBvcGFjaXR5PSIwLjciIGZpbGw9InVybCgjWE1MSURfNl8pIiB3aWR0aD0iNy44ND"
        "YiIGhlaWdodD0iNC42MDEiLz4NCgkJPC9nPg0KCTwvbWFzaz4NCgk8ZWxsaXBzZSBtYXNrPSJ1cmwoI1hNTElEXzVf"
        "KSIgZmlsbD0iI0ZGRkZGRiIgY3g9IjYuOTc1IiBjeT0iMi42NCIgcng9IjMuNDk0IiByeT0iMS45NDYiLz4NCjwvZz"
        "4NCjwvc3ZnPg0K);");
}

CPPUNIT_TEST_FIXTURE(HtmlExportTest, testTdf83890)
{
    createSwDoc("tdf83890.odt");
    save(TestFilter::HTML_WRITER);
    htmlDocUniquePtr pDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/html/body/ol[2]/ol", "start", u"2");
}

CPPUNIT_TEST_FIXTURE(HtmlExportTest, testExtbChars)
{
    createSwWebDoc("extb.html");
    save(TestFilter::HTML_WRITER);
    OUString aExpected(u"\U00024b62"_ustr);
    // Assert that UTF8 encoded non-BMP Unicode character is correct
    uno::Reference<text::XTextRange> xTextRange1 = getRun(getParagraph(1), 1);
    CPPUNIT_ASSERT_EQUAL(aExpected, xTextRange1->getString());

    // Assert that non-BMP Unicode in character entity format is correct
    uno::Reference<text::XTextRange> xTextRange2 = getRun(getParagraph(2), 1);
    CPPUNIT_ASSERT_EQUAL(aExpected, xTextRange2->getString());
}

CPPUNIT_TEST_FIXTURE(HtmlExportTest, testNormalImageExport)
{
    createSwDoc("textAndImage.docx");
    save(TestFilter::HTML_WRITER);
    htmlDocUniquePtr pDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/html/body", 1);

    // the HTML export normally does not embed the images
    OUString imgSrc = getXPath(pDoc, "/html/body/p/img", "src");
    CPPUNIT_ASSERT(imgSrc.endsWith(".png"));
}

CPPUNIT_TEST_FIXTURE(HtmlExportTest, testEmbedImagesEnabled)
{
    createSwDoc("textAndImage.docx");
    save(TestFilter::HTML_WRITER,
         {
             comphelper::makePropertyValue(u"FilterOptions"_ustr, u"EmbedImages"_ustr),
         });

    htmlDocUniquePtr pDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/html/body", 1);

    // the HTML export normally does not embed the images, but here the test
    // name triggers setting of the "EmbedImages" filter option, meaning the
    // image will not be a separate PNG, but an embedded base64 encoded
    // version of that
    OUString imgSrc = getXPath(pDoc, "/html/body/p/img", "src");
    CPPUNIT_ASSERT(imgSrc.startsWith("data:image/png;base64,"));
}

CPPUNIT_TEST_FIXTURE(HtmlExportTest, testXHTML)
{
    createSwWebDoc("hello.html");
    save(TestFilter::HTML_WRITER,
         {
             comphelper::makePropertyValue(u"FilterOptions"_ustr, u"XHTML"_ustr),
         });

    OString aExpected("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML"_ostr);
    SvStream* pStream = maTempFile.GetStream(StreamMode::READ);
    CPPUNIT_ASSERT(pStream);
    OString aActual(read_uInt8s_ToOString(*pStream, aExpected.getLength()));
    // This was HTML, not XHTML.
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);

    htmlDocUniquePtr pDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pDoc);
    // This was lang, not xml:lang.
    assertXPath(pDoc, "/html/body", "xml:lang", u"en-US");
}

CPPUNIT_TEST_FIXTURE(HtmlExportTest, testReqIfParagraph)
{
    setImportFilterName(TestFilter::HTML_WRITER);
    createSwDoc("reqif-p.xhtml", {
                                     comphelper::makePropertyValue(u"FilterOptions"_ustr,
                                                                   u"xhtmlns=reqif-xhtml"_ustr),
                                 });
    save(TestFilter::HTML_WRITER,
         {
             comphelper::makePropertyValue(u"FilterOptions"_ustr, u"xhtmlns=reqif-xhtml"_ustr),
         });

    SvStream* pStream = maTempFile.GetStream(StreamMode::READ);
    CPPUNIT_ASSERT(pStream);
    sal_uInt64 nLength = pStream->TellEnd();

    OString aExpected = "<reqif-xhtml:div><reqif-xhtml:p>aaa<reqif-xhtml:br/>\nbbb"
                        "</reqif-xhtml:p>" SAL_NEWLINE_STRING
                        // This was '<table' instead.
                        "<reqif-xhtml:table"_ostr;

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
    CPPUNIT_ASSERT(aStream.indexOf("<reqif-xhtml:span style=\"text-decoration: line-through\"")
                   != -1);

    // This was "<font>" instead of CSS + namespace prefix was missing.
    CPPUNIT_ASSERT(aStream.indexOf("<reqif-xhtml:span style=\"color: #ce181e\"") != -1);

    // This was '<reqif-xhtml:a id="...">': non-unique bookmark name in reqif fragment.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-1), aStream.indexOf("<reqif-xhtml:a id="));
}

CPPUNIT_TEST_FIXTURE(HtmlExportTest, testReqIfOleData)
{
    auto verify = [this]() {
        uno::Reference<text::XTextEmbeddedObjectsSupplier> xSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xObjects(xSupplier->getEmbeddedObjects(),
                                                         uno::UNO_QUERY);
        // This was 0, <object> without URL was ignored.
        // Then this was 0 on export, as data of OLE nodes was ignored.
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), xObjects->getCount());
    };
    setImportFilterName(TestFilter::HTML_WRITER);
    createSwDoc("reqif-ole-data.xhtml", {
                                            comphelper::makePropertyValue(
                                                u"FilterOptions"_ustr, u"xhtmlns=reqif-xhtml"_ustr),
                                        });
    verify();
    saveAndReload(
        TestFilter::HTML_WRITER,
        {
            comphelper::makePropertyValue(u"FilterOptions"_ustr, u"xhtmlns=reqif-xhtml"_ustr),
        });
    verify();
}

CPPUNIT_TEST_FIXTURE(HtmlExportTest, testReqIfOleImg)
{
    // FIXME: the DPI check should be removed when either (1) the test is fixed to work with
    // non-default DPI; or (2) unit tests on Windows are made to use svp VCL plugin.
    if (!IsDefaultDPI())
        return;
    auto verify = [this]() {
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
        CPPUNIT_ASSERT_EQUAL(u"image/png"_ustr, getProperty<OUString>(xGraphic, u"MimeType"_ustr));

        uno::Reference<beans::XPropertySet> xObjectProps(xObject, uno::UNO_QUERY);
        uno::Reference<io::XActiveDataStreamer> xStreamProvider(
            xObjectProps->getPropertyValue(u"EmbeddedObject"_ustr), uno::UNO_QUERY);
        uno::Reference<io::XSeekable> xStream(xStreamProvider->getStream(), uno::UNO_QUERY);
        // This was empty when either import or export handling was missing.
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int64>(37888), xStream->getLength());

        // Check alternate text (it was empty, for export the 'alt' attribute was used).
        CPPUNIT_ASSERT_EQUAL(u"OLE Object"_ustr,
                             getProperty<OUString>(xObject, u"Title"_ustr).trim());
    };

    setImportFilterName(TestFilter::HTML_WRITER);
    createSwDoc("reqif-ole-img.xhtml", {
                                           comphelper::makePropertyValue(
                                               u"FilterOptions"_ustr, u"xhtmlns=reqif-xhtml"_ustr),
                                       });
    verify();
    saveAndReload(
        TestFilter::HTML_WRITER,
        {
            comphelper::makePropertyValue(u"FilterOptions"_ustr, u"xhtmlns=reqif-xhtml"_ustr),
        });
    verify();

    // "type" attribute was missing for the inner <object> element.
    SvStream* pStream = maTempFile.GetStream(StreamMode::READ);
    CPPUNIT_ASSERT(pStream);
    sal_uInt64 nLength = pStream->TellEnd();
    OString aStream(read_uInt8s_ToOString(*pStream, nLength));
    CPPUNIT_ASSERT(aStream.indexOf("type=\"image/png\"") != -1);
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqIfPngImg)
{
    auto verify = [this](bool bExported) {
        uno::Reference<container::XNamed> xShape(getShape(1), uno::UNO_QUERY);
        CPPUNIT_ASSERT(xShape.is());

        if (!bExported)
        {
            // Imported PNG image is not an object.
            CPPUNIT_ASSERT_EQUAL(u"Image1"_ustr, xShape->getName());
            return;
        }

        // All images are exported as objects in ReqIF mode.
        CPPUNIT_ASSERT_EQUAL(u"Object1"_ustr, xShape->getName());

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
    };

    ImportFromReqif(createFileURL(u"reqif-png-img.xhtml"));
    verify(/*bExported=*/false);
    uno::Sequence<beans::PropertyValue> aStoreProperties = {
        comphelper::makePropertyValue(u"FilterOptions"_ustr, u"xhtmlns=reqif-xhtml"_ustr),
        comphelper::makePropertyValue(u"ExportImagesAsOLE"_ustr, true),
    };
    setImportFilterName(TestFilter::HTML_WRITER);
    saveAndReload(TestFilter::HTML_WRITER, aStoreProperties);
    verify(/*bExported=*/true);
}

CPPUNIT_TEST_FIXTURE(HtmlExportTest, testReqIfJpgImg)
{
    setImportFilterName(TestFilter::HTML_WRITER);
    createSwDoc("reqif-jpg-img.xhtml", {
                                           comphelper::makePropertyValue(
                                               u"FilterOptions"_ustr, u"xhtmlns=reqif-xhtml"_ustr),
                                       });
    save(TestFilter::HTML_WRITER,
         {
             comphelper::makePropertyValue(u"FilterOptions"_ustr, u"xhtmlns=reqif-xhtml"_ustr),
         });

    SvStream* pStream = maTempFile.GetStream(StreamMode::READ);
    CPPUNIT_ASSERT(pStream);
    sal_uInt64 nLength = pStream->TellEnd();
    OString aStream(read_uInt8s_ToOString(*pStream, nLength));
    // This was image/jpeg, JPG was not converted to PNG in ReqIF mode.
    CPPUNIT_ASSERT(aStream.indexOf("type=\"image/png\"") != -1);
}

CPPUNIT_TEST_FIXTURE(HtmlExportTest, testReqIfTable)
{
    setImportFilterName(TestFilter::HTML_WRITER);
    createSwDoc("reqif-table.xhtml", {
                                         comphelper::makePropertyValue(u"FilterOptions"_ustr,
                                                                       u"xhtmlns=reqif-xhtml"_ustr),
                                     });
    save(TestFilter::HTML_WRITER,
         {
             comphelper::makePropertyValue(u"FilterOptions"_ustr, u"xhtmlns=reqif-xhtml"_ustr),
         });

    xmlDocUniquePtr pDoc = WrapReqifFromTempFile();

    // <div> was missing, so the XHTML fragment wasn't a valid
    // xhtml.BlkStruct.class type anymore.
    assertXPath(pDoc,
                "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:table/reqif-xhtml:tr/reqif-xhtml:th",
                1);
    // Make sure that the cell background is not written using CSS.
    assertXPathNoAttribute(
        pDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:table/reqif-xhtml:tr/reqif-xhtml:th",
        "style");
    // The attribute was present, which is not valid in reqif-xhtml.
    assertXPathNoAttribute(
        pDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:table/reqif-xhtml:tr/reqif-xhtml:th",
        "bgcolor");
}

CPPUNIT_TEST_FIXTURE(HtmlExportTest, testReqIfTable2)
{
    createSwDoc("reqif-table2.odt");
    save(TestFilter::HTML_WRITER,
         {
             comphelper::makePropertyValue(u"FilterOptions"_ustr, u"xhtmlns=reqif-xhtml"_ustr),
         });

    SvStream* pStream = maTempFile.GetStream(StreamMode::READ);
    CPPUNIT_ASSERT(pStream);
    sal_uInt64 nLength = pStream->TellEnd();
    OString aStream(read_uInt8s_ToOString(*pStream, nLength));
    // This failed, <reqif-xhtml:td width="..."> was written.
    CPPUNIT_ASSERT(aStream.indexOf("<reqif-xhtml:td>") != -1);
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqIfTableHeight)
{
    // Given a document with a table in it, with an explicit row height:
    createSwDoc();
    uno::Sequence<beans::PropertyValue> aTableProperties = {
        comphelper::makePropertyValue(u"Rows"_ustr, static_cast<sal_Int32>(1)),
        comphelper::makePropertyValue(u"Columns"_ustr, static_cast<sal_Int32>(1)),
    };
    dispatchCommand(mxComponent, u".uno:InsertTable"_ustr, aTableProperties);
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xRow(xTable->getRows()->getByIndex(0), uno::UNO_QUERY);
    xRow->setPropertyValue(u"Height"_ustr, uno::Any(static_cast<sal_Int32>(1000)));

    // When exporting to reqif-xhtml:
    ExportToReqif();

    // Then make sure that the explicit cell height is omitted from the output:
    xmlDocUniquePtr pDoc = WrapReqifFromTempFile();
    // Without the accompanying fix in place, this test would have failed, explicit height was
    // written, which is not valid reqif-xhtml.
    assertXPathNoAttribute(pDoc, "//reqif-xhtml:td", "height");
}

CPPUNIT_TEST_FIXTURE(HtmlExportTest, testXHTMLUseCSS)
{
    createSwDoc("xhtml-css.odt");
    save(TestFilter::HTML_WRITER,
         {
             comphelper::makePropertyValue(u"FilterOptions"_ustr, u"XHTML"_ustr),
         });

    SvStream* pStream = maTempFile.GetStream(StreamMode::READ);
    CPPUNIT_ASSERT(pStream);
    sal_uInt64 nLength = pStream->TellEnd();
    OString aStream(read_uInt8s_ToOString(*pStream, nLength));
    // This failed, <font face="..."> was written.
    CPPUNIT_ASSERT(aStream.indexOf("<span style=\"font-family:") != -1);
    // This failed, <font size="..."> was written.
    CPPUNIT_ASSERT(aStream.indexOf("<span style=\"font-size:") != -1);
}

CPPUNIT_TEST_FIXTURE(HtmlExportTest, testReqIfList)
{
    setImportFilterName(TestFilter::HTML_WRITER);
    createSwDoc("reqif-list.xhtml", {
                                        comphelper::makePropertyValue(u"FilterOptions"_ustr,
                                                                      u"xhtmlns=reqif-xhtml"_ustr),
                                    });
    save(TestFilter::HTML_WRITER,
         {
             comphelper::makePropertyValue(u"FilterOptions"_ustr, u"xhtmlns=reqif-xhtml"_ustr),
         });

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

CPPUNIT_TEST_FIXTURE(HtmlExportTest, testReqIfOle2)
{
    auto verify = [this]() {
        uno::Reference<text::XTextEmbeddedObjectsSupplier> xSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xObjects(xSupplier->getEmbeddedObjects(),
                                                         uno::UNO_QUERY);
        uno::Reference<document::XEmbeddedObjectSupplier2> xObject(xObjects->getByIndex(0),
                                                                   uno::UNO_QUERY);
        uno::Reference<io::XActiveDataStreamer> xEmbeddedObject(
            xObject->getExtendedControlOverEmbeddedObject(), uno::UNO_QUERY);
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
    };

    setImportFilterName(TestFilter::HTML_WRITER);
    createSwDoc("reqif-ole2.xhtml", {
                                        comphelper::makePropertyValue(u"FilterOptions"_ustr,
                                                                      u"xhtmlns=reqif-xhtml"_ustr),
                                    });
    verify();
    saveAndReload(
        TestFilter::HTML_WRITER,
        {
            comphelper::makePropertyValue(u"FilterOptions"_ustr, u"xhtmlns=reqif-xhtml"_ustr),
        });
    verify();

    // Check that the replacement graphic is exported at RTF level.
    xmlDocUniquePtr pDoc = WrapReqifFromTempFile();
    // Get the path of the RTF data.
    OUString aOlePath = getXPath(
        pDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:p/reqif-xhtml:object", "data");
    OUString aOleSuffix(u".ole"_ustr);
    CPPUNIT_ASSERT(aOlePath.endsWith(aOleSuffix));
    INetURLObject aUrl(maTempFile.GetURL());
    aUrl.setBase(aOlePath.subView(0, aOlePath.getLength() - aOleSuffix.getLength()));
    aUrl.setExtension(u"ole");
    OUString aOleUrl = aUrl.GetMainURL(INetURLObject::DecodeMechanism::NONE);

    // Search for \result in the RTF data.
    SvFileStream aOleStream(aOleUrl, StreamMode::READ);
    CPPUNIT_ASSERT(aOleStream.IsOpen());
    OString aOleString(read_uInt8s_ToOString(aOleStream, aOleStream.TellEnd()));
    // Without the accompanying fix in place, this test would have failed,
    // replacement graphic was missing at RTF level.
    CPPUNIT_ASSERT(aOleString.indexOf(OOO_STRING_SVTOOLS_RTF_RESULT) != -1);
}

CPPUNIT_TEST_FIXTURE(HtmlExportTest, testReqIfOle2Odg)
{
    auto verify = [this]() {
        uno::Reference<text::XTextEmbeddedObjectsSupplier> xSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xObjects(xSupplier->getEmbeddedObjects(),
                                                         uno::UNO_QUERY);
        uno::Reference<document::XEmbeddedObjectSupplier> xTextEmbeddedObject(
            xObjects->getByIndex(0), uno::UNO_QUERY);
        uno::Reference<lang::XServiceInfo> xObject(xTextEmbeddedObject->getEmbeddedObject(),
                                                   uno::UNO_QUERY);
        // This failed, both import and export failed to handle OLE2 that contains
        // just ODF.
        CPPUNIT_ASSERT(xObject.is());
        CPPUNIT_ASSERT(xObject->supportsService(u"com.sun.star.drawing.DrawingDocument"_ustr));
    };
    setImportFilterName(TestFilter::HTML_WRITER);
    createSwDoc("reqif-ole-odg.xhtml", {
                                           comphelper::makePropertyValue(
                                               u"FilterOptions"_ustr, u"xhtmlns=reqif-xhtml"_ustr),
                                       });
    verify();
    saveAndReload(
        TestFilter::HTML_WRITER,
        {
            comphelper::makePropertyValue(u"FilterOptions"_ustr, u"xhtmlns=reqif-xhtml"_ustr),
        });
    verify();
}

CPPUNIT_TEST_FIXTURE(HtmlExportTest, testList)
{
    createSwWebDoc("list.html");
    save(TestFilter::HTML_WRITER);
    SvStream* pStream = maTempFile.GetStream(StreamMode::READ);
    CPPUNIT_ASSERT(pStream);
    sal_uInt64 nLength = pStream->TellEnd();
    OString aStream(read_uInt8s_ToOString(*pStream, nLength));
    // This failed, it was <li/>, i.e. list item was closed before content
    // started.
    CPPUNIT_ASSERT(aStream.indexOf("<li>") != -1);
}

CPPUNIT_TEST_FIXTURE(HtmlExportTest, testTransparentImage)
{
    createSwDoc("transparent-image.odt");
    save(TestFilter::HTML_WRITER);
    htmlDocUniquePtr pDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pDoc);

    OUString aSource = getXPath(pDoc, "/html/body/p/img", "src");
    OUString aMessage = "src attribute is: " + aSource;
    // This was a jpeg, transparency was lost.
    CPPUNIT_ASSERT_MESSAGE(aMessage.toUtf8().getStr(), aSource.endsWith(".gif"));
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testTransparentImageReqIf)
{
    createSwDoc("transparent-image.odt");
    uno::Sequence<beans::PropertyValue> aStoreProperties = {
        comphelper::makePropertyValue(u"FilterOptions"_ustr, u"xhtmlns=reqif-xhtml"_ustr),
        comphelper::makePropertyValue(u"ExportImagesAsOLE"_ustr, true),
    };
    save(TestFilter::HTML_WRITER, aStoreProperties);
    xmlDocUniquePtr pDoc = WrapReqifFromTempFile();

    OUString aSource = getXPath(
        pDoc,
        "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:p/reqif-xhtml:object/reqif-xhtml:object",
        "data");
    OUString aMessage = "src attribute is: " + aSource;
    // This was GIF, when the intention was to force PNG.
    CPPUNIT_ASSERT_MESSAGE(aMessage.toUtf8().getStr(), aSource.endsWith(".png"));
}

CPPUNIT_TEST_FIXTURE(HtmlExportTest, testOleNodataReqIf)
{
    createSwDoc("reqif-ole-nodata.odt");
    save(TestFilter::HTML_WRITER,
         {
             comphelper::makePropertyValue(u"FilterOptions"_ustr, u"xhtmlns=reqif-xhtml"_ustr),
         });

    // This failed, io::IOException was thrown during the filter() call.
    xmlDocUniquePtr pDoc = WrapReqifFromTempFile();

    // Make sure the native <object> element has the required data attribute.
    OUString aSource = getXPath(
        pDoc,
        "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:p/reqif-xhtml:object/reqif-xhtml:object",
        "data");
    CPPUNIT_ASSERT(!aSource.isEmpty());
}

CPPUNIT_TEST_FIXTURE(HtmlExportTest, testNoLangReqIf)
{
    createSwDoc("reqif-no-lang.odt");
    save(TestFilter::HTML_WRITER,
         {
             comphelper::makePropertyValue(u"FilterOptions"_ustr, u"xhtmlns=reqif-xhtml"_ustr),
         });

    xmlDocUniquePtr pDoc = WrapReqifFromTempFile();

    // Make sure that xml:lang is not written in ReqIF mode.
    assertXPathNoAttribute(pDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:h1", "lang");
}

CPPUNIT_TEST_FIXTURE(HtmlExportTest, testFieldShade)
{
    createSwDoc("field-shade.odt");
    save(TestFilter::HTML_WRITER);
    htmlDocUniquePtr pDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pDoc);

    // Without the accompanying fix in place, this test would have failed with 'Expected: 1; Actual:
    // 0', i.e. shading for the field was lost.
    assertXPath(pDoc, "/html/body/p[1]/span", "style", u"background: #c0c0c0");

    // Check that field shading is written only in case there is no user-defined span background.
    assertXPath(pDoc, "/html/body/p[2]/span", "style", u"background: #ff0000");
    // Without the accompanying fix in place, this test would have failed with 'Expected: 0; Actual:
    // 1', i.e there was an inner span hiding the wanted background color.
    assertXPath(pDoc, "/html/body/p[2]/span/span", 0);
}

CPPUNIT_TEST_FIXTURE(HtmlExportTest, testTdf132739)
{
    createSwDoc("tdf132739.odt");
    save(TestFilter::HTML_WRITER);
    htmlDocUniquePtr pDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pDoc);

    // Without the fix in place, this test would have failed with
    // - Expected: background: #5983b0; border: 1px solid #333333; padding: 0.04in
    // - Actual  : background: #5983b0
    assertXPath(pDoc, "/html/body/table/tr[1]/td", "style",
                u"background: #5983b0; border: 1px solid #333333; padding: 0.04in");
}

CPPUNIT_TEST_FIXTURE(HtmlExportTest, testFieldShadeReqIf)
{
    createSwDoc("field-shade-reqif.odt");
    save(TestFilter::HTML_WRITER,
         {
             comphelper::makePropertyValue(u"FilterOptions"_ustr, u"xhtmlns=reqif-xhtml"_ustr),
         });

    htmlDocUniquePtr pDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pDoc);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 2
    // i.e. the ReqIF subset of xhtml had a background color and a page number field, resulting in
    // an invalid ReqIF-XHTML.
    assertXPath(pDoc, "/html/body/div/p[1]/span", 0);
    assertXPath(pDoc, "/html/body/div/p[1]/sdfield", 0);
}

CPPUNIT_TEST_FIXTURE(HtmlExportTest, testTdf126879)
{
    createSwDoc("tdf126879.odt");
    save(TestFilter::HTML_WRITER);
    static constexpr OString aExpected("<!DOCTYPE html>"_ostr);
    SvStream* pStream = maTempFile.GetStream(StreamMode::READ);
    CPPUNIT_ASSERT(pStream);
    const OString aActual(read_uInt8s_ToOString(*pStream, aExpected.getLength()));
    // Without the fix in place, this test would have failed with
    // - Expected: <!DOCTYPE html>
    // - Actual  : <!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testBlockQuoteReqIf)
{
    // Build a document model that uses the Quotations paragraph style.
    createSwDoc();
    uno::Reference<beans::XPropertySet> xParagraph(getParagraph(1), uno::UNO_QUERY);
    xParagraph->setPropertyValue(u"ParaStyleName"_ustr, uno::Any(u"Quotations"_ustr));

    // Export it.
    ExportToReqif();
    xmlDocUniquePtr pDoc = WrapReqifFromTempFile();

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // i.e. <blackquote> had character (direct) children, which is invalid xhtml.
    assertXPath(pDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:blockquote/reqif-xhtml:p", 1);
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testRTFOLEMimeType)
{
    // Import a document with an embedded object.
    OUString aType(u"test/rtf"_ustr);
    ImportFromReqif(createFileURL(u"reqif-ole-data.xhtml"));

    // Export it.
    uno::Sequence<beans::PropertyValue> aStoreProperties = {
        comphelper::makePropertyValue(u"FilterOptions"_ustr, u"xhtmlns=reqif-xhtml"_ustr),
        comphelper::makePropertyValue(u"RTFOLEMimeType"_ustr, aType),
    };
    save(TestFilter::HTML_WRITER, aStoreProperties);
    xmlDocUniquePtr pDoc = WrapReqifFromTempFile();

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: test/rtf
    // - Actual  : text/rtf
    // i.e. the MIME type was always text/rtf, not taking the store parameter into account.
    assertXPath(pDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:p/reqif-xhtml:object", "type",
                aType);
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testChinese)
{
    // Load a document with Chinese text in it.
    createSwDoc("reqif-chinese.odt");

    // Export it.
    ExportToReqif();
    // Without the accompanying fix in place, this would have failed as the output was not
    // well-formed.
    WrapReqifFromTempFile();
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqifComment)
{
    // Create a document with a comment in it.
    createSwDoc();
    uno::Sequence<beans::PropertyValue> aPropertyValues = comphelper::InitPropertySequence({
        { "Text", uno::Any(u"some text"_ustr) },
        { "Author", uno::Any(u"me"_ustr) },
    });
    dispatchCommand(mxComponent, u".uno:InsertAnnotation"_ustr, aPropertyValues);

    // Export it.
    ExportToReqif();
    // Without the accompanying fix in place, this would have failed as the output was not
    // well-formed.
    WrapReqifFromTempFile();
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqifFontNameSize)
{
    // Create a document with a custom font name and size in it.
    createSwDoc();
    uno::Reference<beans::XPropertySet> xParagraph(getParagraph(1), uno::UNO_QUERY);
    xParagraph->setPropertyValue(u"CharFontName"_ustr, uno::Any(u"Liberation Serif"_ustr));
    float fCharHeight = 14.0;
    xParagraph->setPropertyValue(u"CharHeight"_ustr, uno::Any(fCharHeight));
    sal_Int32 nCharColor = 0xff0000;
    xParagraph->setPropertyValue(u"CharColor"_ustr, uno::Any(nCharColor));
    uno::Reference<text::XTextRange> xTextRange(xParagraph, uno::UNO_QUERY);
    xTextRange->setString(u"x"_ustr);

    // Export it.
    ExportToReqif();
    xmlDocUniquePtr pDoc = WrapReqifFromTempFile();

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 3
    // i.e. font name and size was written, even if that's not relevant for ReqIF.
    assertXPath(pDoc, "//reqif-xhtml:span", 1);
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqifParagraphAlignment)
{
    // Create a document with an explicitly aligned paragraph.
    createSwDoc();
    uno::Reference<beans::XPropertySet> xParagraph(getParagraph(1), uno::UNO_QUERY);
    xParagraph->setPropertyValue(u"ParaAdjust"_ustr,
                                 uno::Any(static_cast<sal_Int16>(style::ParagraphAdjust_RIGHT)));

    // Export it.
    ExportToReqif();
    xmlDocUniquePtr pDoc = WrapReqifFromTempFile();

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected:
    // - Actual  : right
    // i.e. the <reqif-xhtml:p align="..."> markup was used, which is invalid.
    assertXPathNoAttribute(pDoc, "//reqif-xhtml:p", "align");
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqifOle1PDF)
{
    // Save to reqif-xhtml.
    createSwDoc("pdf-ole.odt");

    setImportFilterName(TestFilter::HTML_WRITER);
    saveAndReload(
        TestFilter::HTML_WRITER,
        {
            comphelper::makePropertyValue(u"FilterOptions"_ustr, u"xhtmlns=reqif-xhtml"_ustr),
        });
    OUString aRtfUrl = GetOlePath();
    SvMemoryStream aOle1;
    ParseOle1FromRtfUrl(aRtfUrl, aOle1);

    // Check the content of the ole1 data.
    OLE1Reader aOle1Reader(aOle1);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 39405
    // - Actual  : 43008
    // i.e. we did not work with the Ole10Native stream, rather created an OLE1 wrapper around the
    // OLE1-in-OLE2 data, resulting in additional size.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(0x99ed), aOle1Reader.m_nNativeDataSize);

    // Now check the ODT result.
    save(TestFilter::ODT);
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess
        = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory),
                                                      maTempFile.GetURL());
    uno::Reference<io::XInputStream> xInputStream(xNameAccess->getByName(u"Object 2"_ustr),
                                                  uno::UNO_QUERY);
    std::unique_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(xInputStream, true));
    rtl::Reference<SotStorage> pStorage = new SotStorage(*pStream);
    rtl::Reference<SotStorageStream> pOleNative = pStorage->OpenSotStream(u"\1Ole10Native"_ustr);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 39409
    // - Actual  : 0
    // i.e. we didn't handle the case when the ole1 payload was not an ole2 container. Note how the
    // expected value is the same as nData above + 4 bytes, since this data is length-prefixed.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(39409), pOleNative->GetSize());
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqifOle1Paint)
{
    // Load the bug document, which has OLE1 data in it, which is not a wrapper around OLE2 data.
    ImportFromReqif(createFileURL(u"paint-ole.xhtml"));

    // Save it as ODT to inspect the result of the OLE1 -> OLE2 conversion.
    save(TestFilter::ODT);
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess
        = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory),
                                                      maTempFile.GetURL());
    uno::Reference<io::XInputStream> xInputStream(xNameAccess->getByName(u"Object 2"_ustr),
                                                  uno::UNO_QUERY);
    std::unique_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(xInputStream, true));
    rtl::Reference<SotStorage> pStorage = new SotStorage(*pStream);
    // Check the clsid of the root stream of the OLE2 storage.
    SvGlobalName aActual = pStorage->GetClassName();
    SvGlobalName aExpected(0x0003000A, 0, 0, 0xc0, 0, 0, 0, 0, 0, 0, 0x46);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0003000A-0000-0000-c000-000000000046
    // - Actual  : 0003000C-0000-0000-c000-000000000046
    // i.e. the "Package" clsid was used on the OLE2 storage unconditionally, even for an mspaint
    // case, which has its own clsid.
    CPPUNIT_ASSERT_EQUAL(aExpected.GetHexName(), aActual.GetHexName());

    ExportToReqif();
    OUString aRtfUrl = GetOlePath();
    SvMemoryStream aOle1;
    ParseOle1FromRtfUrl(aRtfUrl, aOle1);

    // Check the content of the ole1 data.
    // Skip ObjectHeader, see [MS-OLEDS] 2.2.4.
    aOle1.Seek(0);
    sal_uInt32 nData;
    aOle1.ReadUInt32(nData); // OLEVersion
    aOle1.ReadUInt32(nData); // FormatID
    aOle1.ReadUInt32(nData); // ClassName
    CPPUNIT_ASSERT(nData);
    OString aClassName = read_uInt8s_ToOString(aOle1, nData - 1);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: PBrush
    // - Actual  : Package
    // i.e. a hardcoded class name was written.
    CPPUNIT_ASSERT_EQUAL("PBrush"_ostr, aClassName);
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqifOle1PaintBitmapFormat)
{
    // Given a document with a 8bpp bitmap:
    createSwDoc("paint-ole-bitmap-format.odt");

    // When exporting to reqif-xhtml with ExportImagesAsOLE enabled:
    uno::Sequence<beans::PropertyValue> aStoreProperties = {
        comphelper::makePropertyValue(u"FilterOptions"_ustr, u"xhtmlns=reqif-xhtml"_ustr),
        comphelper::makePropertyValue(u"ExportImagesAsOLE"_ustr, true),
    };
    save(TestFilter::HTML_WRITER, aStoreProperties);

    // Then make sure the resulting bitmap is 24bpp:
    OUString aRtfUrl = GetOlePath();
    SvMemoryStream aOle1;
    ParseOle1FromRtfUrl(aRtfUrl, aOle1);
    OLE1Reader aOle1Reader(aOle1);
    Bitmap aBitmap;
    SvMemoryStream aMemory;
    aMemory.WriteBytes(aOle1Reader.m_aNativeData.data(), aOle1Reader.m_aNativeData.size());
    aMemory.Seek(0);
    CPPUNIT_ASSERT(ReadDIB(aBitmap, aMemory, true));
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 24
    // - Actual  : 8
    // i.e. it was not a pixel format ms paint could handle in OLE mode.
    CPPUNIT_ASSERT_EQUAL(vcl::PixelFormat::N24_BPP, aBitmap.getPixelFormat());
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testMultiParaListItem)
{
    // Create a document with 3 list items: A, B&C and D.
    createSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Insert(u"A"_ustr);
    SwDoc* pDoc = pWrtShell->GetDoc();
    {
        // Enable numbering.
        sal_uInt16 nPos = pDoc->MakeNumRule(pDoc->GetUniqueNumRuleName());
        SwNumRule* pNumRule = pDoc->GetNumRuleTable()[nPos];
        SwNode& rNode = pWrtShell->GetCursor()->GetPoint()->GetNode();
        SwTextNode& rTextNode = *rNode.GetTextNode();
        rTextNode.SetAttr(SwNumRuleItem(pNumRule->GetName()));
    }
    pWrtShell->SplitNode();
    pWrtShell->Insert(u"B"_ustr);
    pWrtShell->SplitNode();
    pWrtShell->Insert(u"C"_ustr);
    {
        // C is in the same list item as B.
        SwNode& rNode = pWrtShell->GetCursor()->GetPoint()->GetNode();
        SwTextNode& rTextNode = *rNode.GetTextNode();
        rTextNode.SetCountedInList(false);
    }
    pWrtShell->SplitNode();
    pWrtShell->Insert(u"D"_ustr);

    ExportToReqif();

    xmlDocUniquePtr pXmlDoc = WrapReqifFromTempFile();
    assertXPathContent(pXmlDoc, "//reqif-xhtml:ol/reqif-xhtml:li[1]/reqif-xhtml:p", u"A");
    assertXPathContent(pXmlDoc, "//reqif-xhtml:ol/reqif-xhtml:li[2]/reqif-xhtml:p[1]", u"B");
    // Without the accompanying fix in place, this test would have failed with:
    // XPath '//reqif-xhtml:ol/reqif-xhtml:li[2]/reqif-xhtml:p[2]' not found
    // i.e. </li> was written before "C", not after "C", so "C" was not in the 2nd list item.
    assertXPathContent(pXmlDoc, "//reqif-xhtml:ol/reqif-xhtml:li[2]/reqif-xhtml:p[2]", u"C");
    assertXPathContent(pXmlDoc, "//reqif-xhtml:ol/reqif-xhtml:li[3]/reqif-xhtml:p", u"D");
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testUnderlineNone)
{
    // Create a document with a single paragraph: its underlying is set to an explicit 'none' value.
    createSwDoc();
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    xText->insertString(xText->getEnd(), u"x"_ustr, /*bAbsorb=*/false);
    uno::Reference<beans::XPropertySet> xParagraph(getParagraph(1), uno::UNO_QUERY);
    xParagraph->setPropertyValue(u"CharUnderline"_ustr,
                                 uno::Any(sal_Int16(awt::FontUnderline::NONE)));

    // Export to reqif-xhtml.
    ExportToReqif();

    // Make sure that the paragraph has no explicit style, because "text-decoration: none" is
    // filtered out.
    xmlDocUniquePtr pXmlDoc = WrapReqifFromTempFile();
    assertXPathNoAttribute(pXmlDoc, "//reqif-xhtml:div/reqif-xhtml:p", "style");
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqifOle1PresDataNoOle2)
{
    // Save to reqif-xhtml.
    createSwDoc("no-ole2-pres-data.odt");
    ExportToReqif();
    OUString aRtfUrl = GetOlePath();
    SvMemoryStream aOle1;
    ParseOle1FromRtfUrl(aRtfUrl, aOle1);

    // Check the content of the ole1 data.
    // Without the accompanying fix in place, this test would have failed as there was no
    // presentation data after the native data in the OLE1 container. The result was not editable in
    // Word.
    OLE1Reader aOle1Reader(aOle1);
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqifOle1PresDataWmfOnly)
{
    // Save to reqif-xhtml.
    createSwDoc("ole1-pres-data-wmf.odt");
    ExportToReqif();
    OUString aRtfUrl = GetOlePath();
    SvMemoryStream aOle1;
    ParseOle1FromRtfUrl(aRtfUrl, aOle1);

    OLE1Reader aOle1Reader(aOle1);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 135660
    // - Actual  : 272376
    // i.e. we wrote some additional EMF data into the WMF output, which broke Word.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(135660), aOle1Reader.m_nPresentationDataSize);
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqifAscharObjsize)
{
    // Given a document with an as-char anchored embedded object:
    createSwDoc("reqif-aschar-objsize.odt");

    // When exporting to reqif-xhtml:
    ExportToReqif();

    // Then make sure that the RTF snippet has the correct aspect ratio:
    OUString aRtfUrl = GetOlePath();
    SvMemoryStream aRtf;
    HtmlExportTest::wrapRtfFragment(aRtfUrl, aRtf);
    tools::SvRef<TestReqIfRtfReader> xReader(new TestReqIfRtfReader(aRtf));
    CPPUNIT_ASSERT(xReader->CallParser() != SvParserState::Error);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 7344
    // - Actual  : 2836
    // i.e. the aspect ratio was 1:1, while the PNG aspect ratio was correctly not 1:1.
    CPPUNIT_ASSERT_EQUAL(static_cast<tools::Long>(7344), xReader->GetObjw());
    CPPUNIT_ASSERT_EQUAL(static_cast<tools::Long>(4116), xReader->GetObjh());
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqifObjdataPresentationDataSize)
{
    // Given a document with an OLE2 embedded object, containing a preview:
    createSwDoc("reqif-objdata-presentationdatasize.odt");

    // When exporting to ReqIF:
    ExportToReqif();

    // Then make sure that the PresentationDataSize in the RTF's objdata blob is correct:
    OUString aRtfUrl = GetOlePath();
    SvMemoryStream aOle1;
    ParseOle1FromRtfUrl(aRtfUrl, aOle1);
    OLE1Reader aOle1Reader(aOle1);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 565994
    // - Actual  : 330240 (Linux)
    // - Actual  : 566034 (Windows, when Word is installed)
    // because PresentationData was taken from the OLE2 stream but its size was taken from RTF.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(565994), aOle1Reader.m_nPresentationDataSize);
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testListHeading)
{
    // Given a document with a list heading:
    createSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Insert(u"list header"_ustr);
    SwDoc* pDoc = pWrtShell->GetDoc();
    sal_uInt16 nPos = pDoc->MakeNumRule(pDoc->GetUniqueNumRuleName());
    SwNumRule* pNumRule = pDoc->GetNumRuleTable()[nPos];
    SwNode& rNode = pWrtShell->GetCursor()->GetPoint()->GetNode();
    SwTextNode& rTextNode = *rNode.GetTextNode();
    rTextNode.SetAttr(SwNumRuleItem(pNumRule->GetName()));
    rTextNode.SetCountedInList(false);

    // When exporting to ReqIF:
    ExportToReqif();

    // Then make sure the output is valid xhtml:
    xmlDocUniquePtr pXmlDoc = WrapReqifFromTempFile();

    // Without the accompanying fix in place, this test would have failed:
    // - expected: <div><ol><li style="display: block"><p>...</p></li></ol></div>
    // - actual  : <div><ol><p>...</p></li></ol></div>
    // because a </li> but no <li> is not well-formed and <ol> with a non-li children is invalid.
    OUString aContent
        = getXPathContent(pXmlDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:ol/"
                                   "reqif-xhtml:li[@style='display: block']/reqif-xhtml:p");
    CPPUNIT_ASSERT_EQUAL(u"list header"_ustr, aContent.trim());
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testPartiallyNumberedList)
{
    // Given a document with a list, first para is numbered, second is not:
    createSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Insert(u"list header"_ustr);
    SwDoc* pDoc = pWrtShell->GetDoc();
    sal_uInt16 nPos = pDoc->MakeNumRule(pDoc->GetUniqueNumRuleName());
    SwNumRule* pNumRule = pDoc->GetNumRuleTable()[nPos];
    {
        SwNode& rNode = pWrtShell->GetCursor()->GetPoint()->GetNode();
        SwTextNode& rTextNode = *rNode.GetTextNode();
        rTextNode.SetAttr(SwNumRuleItem(pNumRule->GetName()));
    }
    pWrtShell->Insert2(u"numbered"_ustr);
    pWrtShell->SplitNode();
    pWrtShell->Insert2(u"not numbered"_ustr);
    {
        SwNode& rNode = pWrtShell->GetCursor()->GetPoint()->GetNode();
        SwTextNode& rTextNode = *rNode.GetTextNode();
        rTextNode.SetAttr(SwNumRuleItem(pNumRule->GetName()));
        rTextNode.SetCountedInList(false);
    }

    // When exporting to ReqIF:
    ExportToReqif();

    // Then make sure the output is well-formed xhtml:
    xmlDocUniquePtr pXmlDoc = WrapReqifFromTempFile();
    // Without the accompanying fix in place, this test would have failed:
    // - expected: <li><p>...</p><p>...</p></li>
    // - actual  : <li><p>...</p><p>...</p>
    // because a <li> without a matching </li> is not well-formed, and the </li> was omitted because
    // the second para was not numbered.
    assertXPath(pXmlDoc,
                "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:ol/reqif-xhtml:li/reqif-xhtml:p", 2);
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testPartiallyNumberedListHTML)
{
    // Given a document with a list, first para is numbered, second is not:
    createSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Insert(u"list header"_ustr);
    SwDoc* pDoc = pWrtShell->GetDoc();
    sal_uInt16 nPos = pDoc->MakeNumRule(pDoc->GetUniqueNumRuleName());
    SwNumRule* pNumRule = pDoc->GetNumRuleTable()[nPos];
    {
        SwNode& rNode = pWrtShell->GetCursor()->GetPoint()->nNode.GetNode();
        SwTextNode& rTextNode = *rNode.GetTextNode();
        rTextNode.SetAttr(SwNumRuleItem(pNumRule->GetName()));
    }
    pWrtShell->Insert2(u"numbered"_ustr);
    pWrtShell->SplitNode();
    pWrtShell->Insert2(u"not numbered"_ustr);
    {
        SwNode& rNode = pWrtShell->GetCursor()->GetPoint()->nNode.GetNode();
        SwTextNode& rTextNode = *rNode.GetTextNode();
        rTextNode.SetAttr(SwNumRuleItem(pNumRule->GetName()));
        rTextNode.SetCountedInList(false);
    }

    // When exporting to HTML:
    save(TestFilter::HTML_WRITER);

    xmlDocUniquePtr pXmlDoc = parseXml(maTempFile);
    CPPUNIT_ASSERT(pXmlDoc); // if we have missing closing marks - parse error

    // Without the accompanying fix in place, this test would have failed:
    // - expected: <li><p>...</p><p>...</p></li>
    // - actual  : <li><p>...</p><p>...</p>
    // because a <li> without a matching </li> is not well-formed, and the </li> was omitted because
    // the second para was not numbered.

    assertXPath(pXmlDoc, "/html/body/ol/li/p", 2);
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testListHeaderAndItem)
{
    // Given a document with a list, first para is not numbered, but the second is:
    createSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Insert(u"not numbered"_ustr);
    SwDoc* pDoc = pWrtShell->GetDoc();
    sal_uInt16 nPos = pDoc->MakeNumRule(pDoc->GetUniqueNumRuleName());
    SwNumRule* pNumRule = pDoc->GetNumRuleTable()[nPos];
    {
        SwNode& rNode = pWrtShell->GetCursor()->GetPoint()->GetNode();
        SwTextNode& rTextNode = *rNode.GetTextNode();
        rTextNode.SetAttr(SwNumRuleItem(pNumRule->GetName()));
        rTextNode.SetCountedInList(false);
    }
    pWrtShell->SplitNode();
    pWrtShell->Insert2(u"numbered"_ustr);
    {
        SwNode& rNode = pWrtShell->GetCursor()->GetPoint()->GetNode();
        SwTextNode& rTextNode = *rNode.GetTextNode();
        rTextNode.SetAttr(SwNumRuleItem(pNumRule->GetName()));
    }

    // When exporting to ReqIF:
    ExportToReqif();

    // Then make sure the output is well-formed xhtml:
    // Without the accompanying fix in place, this would have failed:
    // Entity: line 3: parser error : Opening and ending tag mismatch: ol line 3 and li
    // <reqif-xhtml:ol><reqif-xhtml:p>not numbered</reqif-xhtml:p></reqif-xhtml:li>
    xmlDocUniquePtr pXmlDoc = WrapReqifFromTempFile();
    // Make sure that in case the list has a header and an item, then both are wrapped in an <li>
    // element.
    assertXPath(pXmlDoc,
                "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:ol/reqif-xhtml:li/reqif-xhtml:p", 2);
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testBlockQuoteNoMargin)
{
    // Given a document with some text, para style set to Quotations, no bottom margin:
    createSwDoc();
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    xText->insertString(xText->getEnd(), u"string"_ustr, /*bAbsorb=*/false);
    uno::Reference<beans::XPropertySet> xQuotations(
        getStyles(u"ParagraphStyles"_ustr)->getByName(u"Quotations"_ustr), uno::UNO_QUERY);
    xQuotations->setPropertyValue(u"ParaBottomMargin"_ustr, uno::Any(static_cast<sal_Int32>(0)));
    uno::Reference<beans::XPropertySet> xParagraph(getParagraph(1), uno::UNO_QUERY);
    xParagraph->setPropertyValue(u"ParaStyleName"_ustr, uno::Any(u"Quotations"_ustr));

    // When exporting to XHTML:
    ExportToReqif();

    // Then make sure the output is valid xhtml:
    xmlDocUniquePtr pXmlDoc = WrapReqifFromTempFile();
    // Without the accompanying fix in place, this test would have failed:
    // - expected: <blockquote><p>...</p></blockquote>
    // - actual  : <blockquote>...</blockquote>
    // i.e. <blockquote> is can't have character children, but it had.
    assertXPathContent(pXmlDoc,
                       "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:blockquote/reqif-xhtml:p",
                       u"string");
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqifImageToOle)
{
    // Given a document with an image:
    createSwDoc();
    uno::Sequence<beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue(u"FileName"_ustr, createFileURL(u"ole2.png")),
    };
    dispatchCommand(mxComponent, u".uno:InsertGraphic"_ustr, aArgs);

    // When exporting to XHTML:
    uno::Sequence<beans::PropertyValue> aStoreProperties = {
        comphelper::makePropertyValue(u"FilterOptions"_ustr, u"xhtmlns=reqif-xhtml"_ustr),
        comphelper::makePropertyValue(u"ExportImagesAsOLE"_ustr, true),
    };
    save(TestFilter::HTML_WRITER, aStoreProperties);

    // Then make sure we export that PNG as WMF in ReqIF mode:
    OUString aRtfUrl = GetOlePath();
    SvMemoryStream aRtf;
    HtmlExportTest::wrapRtfFragment(aRtfUrl, aRtf);
    tools::SvRef<TestReqIfRtfReader> xReader(new TestReqIfRtfReader(aRtf));
    CPPUNIT_ASSERT(xReader->CallParser() != SvParserState::Error);
    // Without the accompanying fix in place, this test would have failed:
    // - Expected: 8
    // - Actual  : 0
    // i.e. the image was exported as PNG, not as WMF (with a version).
    CPPUNIT_ASSERT_EQUAL(8, xReader->getWmetafile());

    // Make sure that the native data byte array is not empty.
    SvMemoryStream aOle1;
    CPPUNIT_ASSERT(xReader->WriteObjectData(aOle1));
    // Without the accompanying fix in place, this test would have failed, as aOle1 was empty.
    OLE1Reader aOle1Reader(aOle1);
    CPPUNIT_ASSERT(aOle1Reader.m_nNativeDataSize);

    // Make sure that the presentation data byte array is not empty.
    // Without the accompanying fix in place, this test would have failed, as aOle1 only contained
    // the native data.
    CPPUNIT_ASSERT(aOle1Reader.m_nPresentationDataSize);
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqifEmbedPNGDirectly)
{
    // Given a document with an image:
    createSwDoc();
    uno::Sequence<beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue(u"FileName"_ustr, createFileURL(u"ole2.png")),
    };
    dispatchCommand(mxComponent, u".uno:InsertGraphic"_ustr, aArgs);

    // When exporting to XHTML:
    ExportToReqif();

    // Then make sure the PNG is embedded directly, without an RTF wrapper:
    xmlDocUniquePtr pXmlDoc = WrapReqifFromTempFile();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: image/png
    // - Actual  : text/rtf
    // i.e. even PNG was wrapped in an RTF.
    assertXPath(pXmlDoc, "//reqif-xhtml:p/reqif-xhtml:object", "type", u"image/png");
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqifEmbedJPGDirectly)
{
    // Given a document with an image:
    createSwDoc();
    uno::Sequence<beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue(u"FileName"_ustr, createFileURL(u"reqif-ole-img.jpg")),
    };
    dispatchCommand(mxComponent, u".uno:InsertGraphic"_ustr, aArgs);

    // When exporting to XHTML:
    ExportToReqif();

    // Then make sure the JPG is embedded directly, without an RTF wrapper:
    xmlDocUniquePtr pXmlDoc = WrapReqifFromTempFile();
    assertXPath(pXmlDoc, "//reqif-xhtml:p/reqif-xhtml:object", "type", u"image/jpeg");
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: image/jpeg
    // - Actual  : image/png
    // i.e. first the original JPG data was lost, then the inner PNG fallback was missing.
    assertXPath(pXmlDoc, "//reqif-xhtml:p/reqif-xhtml:object/reqif-xhtml:object", "type",
                u"image/png");
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqifEmbedPNGShapeDirectly)
{
    // Given a document with an image shape:
    createSwDoc();
    uno::Reference<css::lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(
        xFactory->createInstance(u"com.sun.star.drawing.GraphicObjectShape"_ustr), uno::UNO_QUERY);
    xShape->setSize(awt::Size(10000, 10000));
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    xShapeProps->setPropertyValue(u"GraphicURL"_ustr, uno::Any(createFileURL(u"ole2.png")));
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    xDrawPageSupplier->getDrawPage()->add(xShape);

    // When exporting to XHTML:
    ExportToReqif();

    // Then make sure the PNG is embedded directly, without an RTF wrapper:
    xmlDocUniquePtr pXmlDoc = WrapReqifFromTempFile();
    // Without the accompanying fix in place, this test would have failed with:
    // - no attribute 'type' exist
    // i.e. the PNG was exported as GIF, without an explicit type.
    assertXPath(pXmlDoc, "//reqif-xhtml:p/reqif-xhtml:object", "type", u"image/png");
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqifEmbedJPGShapeDirectly)
{
    // Given a document with an image:
    createSwDoc();
    uno::Reference<css::lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(
        xFactory->createInstance(u"com.sun.star.drawing.GraphicObjectShape"_ustr), uno::UNO_QUERY);
    xShape->setSize(awt::Size(10000, 10000));
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    xShapeProps->setPropertyValue(u"GraphicURL"_ustr,
                                  uno::Any(createFileURL(u"reqif-ole-img.jpg")));
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    xDrawPageSupplier->getDrawPage()->add(xShape);

    // When exporting to XHTML:
    ExportToReqif();

    // Then make sure the JPG is embedded directly, without an RTF wrapper:
    xmlDocUniquePtr pXmlDoc = WrapReqifFromTempFile();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: image/jpeg
    // - Actual  : image/png
    // i.e. first the original JPG data was lost, then the inner PNG fallback was missing.
    assertXPath(pXmlDoc, "//reqif-xhtml:p/reqif-xhtml:object", "type", u"image/jpeg");
    assertXPath(pXmlDoc, "//reqif-xhtml:p/reqif-xhtml:object/reqif-xhtml:object", "type",
                u"image/png");
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqifNoTypeInUL)
{
    // Given a document with an unordered list:
    createSwDoc("ul_with_disc.fodt");

    // When exporting to XHTML:
    ExportToReqif();

    // Check that 'ul' element has no 'type' attribute
    xmlDocUniquePtr pXmlDoc = WrapReqifFromTempFile();
    // Without the accompanying fix in place, this test would have failed
    assertXPathNoAttribute(pXmlDoc, "//reqif-xhtml:ul/reqif-xhtml:li/reqif-xhtml:ul", "type");
}
} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
