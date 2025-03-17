/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_features.h>

#include "htmlmodeltestbase.hxx"

#include <memory>

#include <com/sun/star/document/XTypeDetection.hpp>
#include <com/sun/star/table/TableBorder2.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>

#include <officecfg/Office/Common.hxx>

#include <vcl/svapp.hxx>
#include <comphelper/processfactory.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/dibtools.hxx>
#include <editeng/brushitem.hxx>

#include <wrtsh.hxx>
#include <ndtxt.hxx>
#include <docsh.hxx>
#include <unotxdoc.hxx>
#include <formatlinebreak.hxx>
#include <itabenum.hxx>

namespace
{
CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqifEmbedPNGShapeAsOLE)
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
    uno::Sequence<beans::PropertyValue> aStoreProperties = {
        comphelper::makePropertyValue(u"FilterName"_ustr, u"HTML (StarWriter)"_ustr),
        comphelper::makePropertyValue(u"FilterOptions"_ustr, u"xhtmlns=reqif-xhtml"_ustr),
        comphelper::makePropertyValue(u"ExportImagesAsOLE"_ustr, true),
    };
    saveWithParams(aStoreProperties);

    // Then make sure the PNG is embedded with an RTF wrapper:
    xmlDocUniquePtr pXmlDoc = WrapReqifFromTempFile();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: text/rtf
    // - Actual  : image/png
    // i.e. the OLE wrapper around the PNG was missing.
    assertXPath(pXmlDoc, "//reqif-xhtml:p/reqif-xhtml:object", "type", u"text/rtf");
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqifEmbedShapeAsPNG)
{
    // FIXME: the DPI check should be removed when either (1) the test is fixed to work with
    // non-default DPI; or (2) unit tests on Windows are made to use svp VCL plugin.
    if (!IsDefaultDPI())
        return;
    // Given a document with a shape:
    createSwDoc();
    uno::Reference<css::lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(
        xFactory->createInstance(u"com.sun.star.drawing.RectangleShape"_ustr), uno::UNO_QUERY);
    xShape->setSize(awt::Size(10000, 10000));
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    xDrawPageSupplier->getDrawPage()->add(xShape);

    // When exporting to XHTML:
    ExportToReqif();

    // Then make sure the shape is embedded as a PNG:
    xmlDocUniquePtr pXmlDoc = WrapReqifFromTempFile();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: image/png
    // - Actual  : image/x-vclgraphic
    // i.e. the result was invalid ReqIF.
    assertXPath(pXmlDoc, "//reqif-xhtml:p/reqif-xhtml:object", "type", u"image/png");

    // Then check the pixel size of the shape:
    Size aPixelSize(Application::GetDefaultDevice()->LogicToPixel(Size(10000, 10000),
                                                                  MapMode(MapUnit::Map100thMM)));
    // Without the accompanying fix in place, this test would have failed with:
    // - no attribute 'width' exist
    // i.e. shapes had no width.
    assertXPath(pXmlDoc, "//reqif-xhtml:p/reqif-xhtml:object", "width",
                OUString::number(aPixelSize.getWidth()));
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testShapeAsImageHtml)
{
    // Given a document with a shape:
    createSwDoc();
    uno::Reference<css::lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(
        xFactory->createInstance(u"com.sun.star.drawing.RectangleShape"_ustr), uno::UNO_QUERY);
    xShape->setSize(awt::Size(5080, 2540));
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    xDrawPageSupplier->getDrawPage()->add(xShape);

    // When exporting to plain HTML:
    saveAndReload(u"HTML (StarWriter)"_ustr);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected:
    // - Actual  :  />
    // i.e. the output was not well-formed.
    CPPUNIT_ASSERT_EQUAL(u" "_ustr, getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testJson)
{
    // Given a document with a shape:
    createSwDoc();
    uno::Reference<css::lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(
        xFactory->createInstance(u"com.sun.star.drawing.RectangleShape"_ustr), uno::UNO_QUERY);
    xShape->setSize(awt::Size(2540, 2540));
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    xDrawPageSupplier->getDrawPage()->add(xShape);

    // When exporting to HTML, and specifying options as JSON:
    setFilterOptions(u"{\"XhtmlNs\":{\"type\":\"string\", \"value\":\"reqif-xhtml\"},"
                     "\"ShapeDPI\":{\"type\":\"long\",\"value\":\"192\"}}"_ustr);
    save(u"HTML (StarWriter)"_ustr);

    // Then make sure those options are not ignored:
    // Without the accompanying fix in place, this test would have failed, as GetPngPath() expects
    // XML output, but xhtmlns=reqif-xhtml was ignored.
    OUString aPngUrl = GetPngPath();
    SvFileStream aFileStream(aPngUrl, StreamMode::READ);
    GraphicDescriptor aDescriptor(aFileStream, nullptr);
    aDescriptor.Detect(/*bExtendedInfo=*/true);
    // Make sure that the increased DPI is taken into account:
    tools::Long nExpected = 192;
    CPPUNIT_ASSERT_EQUAL(nExpected, aDescriptor.GetSizePixel().getWidth());
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqifEmbedShapeAsPNGCustomDPI)
{
    // FIXME: the DPI check should be removed when either (1) the test is fixed to work with
    // non-default DPI; or (2) unit tests on Windows are made to use svp VCL plugin.
    if (!IsDefaultDPI())
        return;
    // Given a document with a shape:
    createSwDoc();
    uno::Reference<css::lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(
        xFactory->createInstance(u"com.sun.star.drawing.RectangleShape"_ustr), uno::UNO_QUERY);
    xShape->setSize(awt::Size(5080, 2540));
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    xDrawPageSupplier->getDrawPage()->add(xShape);
    sal_Int32 nDPI = 600;

    // When exporting to XHTML:
    uno::Sequence<beans::PropertyValue> aStoreProperties = {
        comphelper::makePropertyValue(u"FilterName"_ustr, u"HTML (StarWriter)"_ustr),
        comphelper::makePropertyValue(u"FilterOptions"_ustr, u"xhtmlns=reqif-xhtml"_ustr),
        comphelper::makePropertyValue(u"ShapeDPI"_ustr, nDPI),
    };
    saveWithParams(aStoreProperties);

    // Then make sure the shape is embedded as a PNG:
    xmlDocUniquePtr pXmlDoc = WrapReqifFromTempFile();
    assertXPath(pXmlDoc, "//reqif-xhtml:p/reqif-xhtml:object", "type", u"image/png");

    // Then check the pixel size of the shape:
    Size aPixelSize(Application::GetDefaultDevice()->LogicToPixel(Size(5080, 2540),
                                                                  MapMode(MapUnit::Map100thMM)));
    tools::Long nPNGWidth = 1200;
    OUString aPngUrl = GetPngPath();
    SvFileStream aFileStream(aPngUrl, StreamMode::READ);
    GraphicDescriptor aDescriptor(aFileStream, nullptr);
    aDescriptor.Detect(/*bExtendedInfo=*/true);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1200
    // - Actual  : 1000
    // i.e. first setting a double DPI didn't result in larger pixel width of the PNG, then it was
    // limited to 1000 pixels (because the pixel limit was 500k).
    CPPUNIT_ASSERT_EQUAL(nPNGWidth, aDescriptor.GetSizePixel().getWidth());

    // Then make sure the shape's logic size (in CSS pixels) don't change:
    assertXPath(pXmlDoc, "//reqif-xhtml:p/reqif-xhtml:object", "width",
                OUString::number(aPixelSize.getWidth()));
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqifOleBmpTransparent)
{
    // Given a document with a transparent image:
    createSwDoc();
    uno::Sequence<beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue(u"FileName"_ustr, createFileURL(u"transparent.png")),
    };
    dispatchCommand(mxComponent, u".uno:InsertGraphic"_ustr, aArgs);

    // When exporting to reqif with ExportImagesAsOLE=true:
    uno::Sequence<beans::PropertyValue> aStoreProperties = {
        comphelper::makePropertyValue(u"FilterName"_ustr, u"HTML (StarWriter)"_ustr),
        comphelper::makePropertyValue(u"FilterOptions"_ustr, u"xhtmlns=reqif-xhtml"_ustr),
        comphelper::makePropertyValue(u"ExportImagesAsOLE"_ustr, true),
    };
    saveWithParams(aStoreProperties);

    // Then make sure the transparent pixel turns into white:
    SvMemoryStream aOle1;
    ParseOle1FromRtfUrl(GetOlePath(), aOle1);
    OLE1Reader aOle1Reader(aOle1);
    SvMemoryStream aBitmapStream(aOle1Reader.m_aNativeData.data(), aOle1Reader.m_aNativeData.size(),
                                 StreamMode::READ);
    Bitmap aBitmap;
    ReadDIB(aBitmap, aBitmapStream, /*bFileHeader=*/true);
    Size aBitmapSize = aBitmap.GetSizePixel();
    BitmapEx aBitmapEx(aBitmap);
    Color nActualColor
        = aBitmapEx.GetPixelColor(aBitmapSize.getWidth() - 1, aBitmapSize.getHeight() - 1);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: Color: R:255 G:255 B:255 A:0
    // - Actual  : Color: R:0 G:0 B:0 A:0
    // i.e. the bitmap without an alpha channel was black, not white.
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, nActualColor);
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testListsHeading)
{
    // Given a document with lh, lh, li, li, lh and lh nodes:
    createSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Insert(u"list 1, header 1"_ustr);
    pWrtShell->SplitNode();
    pWrtShell->Insert(u"list 1, header 2"_ustr);
    pWrtShell->SplitNode();
    pWrtShell->Insert(u"list 2, item 1"_ustr);
    pWrtShell->SplitNode();
    pWrtShell->Insert(u"list 2, item 2"_ustr);
    pWrtShell->SplitNode();
    pWrtShell->Insert(u"list 3, header 1"_ustr);
    pWrtShell->SplitNode();
    pWrtShell->Insert(u"list 3, header 2"_ustr);
    SwDoc* pDoc = pWrtShell->GetDoc();
    pWrtShell->Up(false, 5);
    {
        sal_uInt16 nPos = pDoc->MakeNumRule(pDoc->GetUniqueNumRuleName());
        SwNumRule* pNumRule = pDoc->GetNumRuleTable()[nPos];
        {
            SwNode& rNode = pWrtShell->GetCursor()->GetPoint()->GetNode();
            SwTextNode& rTextNode = *rNode.GetTextNode();
            rTextNode.SetAttr(SwNumRuleItem(pNumRule->GetName()));
            rTextNode.SetCountedInList(false);
        }
        pWrtShell->Down(false, 1);
        {
            SwNode& rNode = pWrtShell->GetCursor()->GetPoint()->GetNode();
            SwTextNode& rTextNode = *rNode.GetTextNode();
            rTextNode.SetAttr(SwNumRuleItem(pNumRule->GetName()));
            rTextNode.SetCountedInList(false);
        }
    }
    pWrtShell->Down(false, 1);
    {
        sal_uInt16 nPos = pDoc->MakeNumRule(pDoc->GetUniqueNumRuleName());
        SwNumRule* pNumRule = pDoc->GetNumRuleTable()[nPos];
        {
            SwNode& rNode = pWrtShell->GetCursor()->GetPoint()->GetNode();
            SwTextNode& rTextNode = *rNode.GetTextNode();
            rTextNode.SetAttr(SwNumRuleItem(pNumRule->GetName()));
        }
        pWrtShell->Down(false, 1);
        {
            SwNode& rNode = pWrtShell->GetCursor()->GetPoint()->GetNode();
            SwTextNode& rTextNode = *rNode.GetTextNode();
            rTextNode.SetAttr(SwNumRuleItem(pNumRule->GetName()));
        }
    }
    pWrtShell->Down(false, 1);
    {
        sal_uInt16 nPos = pDoc->MakeNumRule(pDoc->GetUniqueNumRuleName());
        SwNumRule* pNumRule = pDoc->GetNumRuleTable()[nPos];
        {
            SwNode& rNode = pWrtShell->GetCursor()->GetPoint()->GetNode();
            SwTextNode& rTextNode = *rNode.GetTextNode();
            rTextNode.SetAttr(SwNumRuleItem(pNumRule->GetName()));
            rTextNode.SetCountedInList(false);
        }
        pWrtShell->Down(false, 1);
        {
            SwNode& rNode = pWrtShell->GetCursor()->GetPoint()->GetNode();
            SwTextNode& rTextNode = *rNode.GetTextNode();
            rTextNode.SetAttr(SwNumRuleItem(pNumRule->GetName()));
            rTextNode.SetCountedInList(false);
        }
    }

    // When exporting to ReqIF:
    ExportToReqif();

    // Then make sure the output is valid xhtml:
    xmlDocUniquePtr pXmlDoc = WrapReqifFromTempFile();

    OUString aContent
        = getXPathContent(pXmlDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:ol/"
                                   "reqif-xhtml:li[@style='display: block']/reqif-xhtml:p");
    CPPUNIT_ASSERT_EQUAL(u"list 1, header 1"_ustr, aContent.trim());
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testOleEmfPreviewToHtml)
{
    // Given a document containing an embedded object, with EMF preview:
    createSwDoc("ole2.odt");

    // When exporting to HTML:
    ExportToHTML();

    // Then make sure the <img> tag has matching file extension and data:
    htmlDocUniquePtr pDoc = parseHtml(maTempFile);
    OUString aPath = getXPath(pDoc, "/html/body/p/img", "src");
    // Without the accompanying fix in place, this test would have failed, as aPath was
    // ole_html_3978e5f373402b43.JPG, with EMF data.
    CPPUNIT_ASSERT(aPath.endsWith("gif"));
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testNestedBullets)
{
    // Given a documented with nested lists:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Insert(u"first"_ustr);
    sal_uInt16 nPos = pDoc->MakeNumRule(pDoc->GetUniqueNumRuleName());
    SwNumRule* pNumRule = pDoc->GetNumRuleTable()[nPos];
    {
        SwNode& rNode = pWrtShell->GetCursor()->GetPoint()->GetNode();
        SwTextNode& rTextNode = *rNode.GetTextNode();
        rTextNode.SetAttr(SwNumRuleItem(pNumRule->GetName()));
        rTextNode.SetAttrListLevel(0);
    }
    pWrtShell->SplitNode();
    pWrtShell->Insert(u"second"_ustr);
    {
        SwNode& rNode = pWrtShell->GetCursor()->GetPoint()->GetNode();
        SwTextNode& rTextNode = *rNode.GetTextNode();
        rTextNode.SetAttr(SwNumRuleItem(pNumRule->GetName()));
        rTextNode.SetAttrListLevel(1);
    }

    // When exporting to xhtml:
    ExportToReqif();

    // Then make sure that there is a <li> between the outer and the inner <ol>:
    xmlDocUniquePtr pXmlDoc = WrapReqifFromTempFile();
    // Without the accompanying fix in place, this test would have failed with:
    // - XPath '//reqif-xhtml:ol/reqif-xhtml:li/reqif-xhtml:ol/reqif-xhtml:li/reqif-xhtml:p' not found
    // i.e. the <li> inside the outer <ol> was missing.
    assertXPathContent(
        pXmlDoc, "//reqif-xhtml:ol/reqif-xhtml:li/reqif-xhtml:ol/reqif-xhtml:li/reqif-xhtml:p",
        u"second");
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testTrailingLineBreak)
{
    // Given a document with a trailing line-break:
    createSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Insert(u"test\n"_ustr);

    // When exporting to reqif-xhtml:
    ExportToReqif();

    // Then make sure that we still have a single line-break:
    xmlDocUniquePtr pXmlDoc = WrapReqifFromTempFile();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 2
    // - XPath '//reqif-xhtml:br' number of nodes is incorrect
    assertXPath(pXmlDoc, "//reqif-xhtml:br", 1);

    // Then test the import side:
    // Given an empty document:
    // When importing a <br> from reqif-xhtml:
    ImportFromReqif(maTempFile.GetURL());

    // Then make sure that line-break is not lost:
    pWrtShell = getSwDocShell()->GetWrtShell();
    OUString aActual = pWrtShell->GetCursor()->GetPointNode().GetTextNode()->GetText();
    // Without the accompanying fix in place, this test would have failed, as the trailing
    // line-break was lost.
    CPPUNIT_ASSERT_EQUAL(u"test\n"_ustr, aActual);
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testLeadingTab)
{
    // Given a document with leading tabs:
    createSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Insert(u"\t first"_ustr);
    pWrtShell->SplitNode();
    pWrtShell->Insert(u"\t\t second"_ustr);
    pWrtShell->SplitNode();
    pWrtShell->Insert(u"thi \t rd"_ustr);

    // When exporting to HTML, using LeadingTabWidth=2:
    uno::Sequence<beans::PropertyValue> aStoreProperties = {
        comphelper::makePropertyValue(u"FilterName"_ustr, u"HTML (StarWriter)"_ustr),
        comphelper::makePropertyValue(u"FilterOptions"_ustr, u"xhtmlns=reqif-xhtml"_ustr),
        comphelper::makePropertyValue(u"LeadingTabWidth"_ustr, static_cast<sal_Int32>(2)),
    };
    saveWithParams(aStoreProperties);

    // Then make sure that leading tabs are replaced with 2 nbsps:
    xmlDocUniquePtr pXmlDoc = WrapReqifFromTempFile();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: <nbsp><nbsp><space>first
    // - Actual  : <tab><space>first
    // i.e. the leading tab was not replaced by 2 nbsps.
    assertXPathContent(pXmlDoc, "//reqif-xhtml:p[1]", u"\xa0\xa0 first");
    // Test a leading tab that is not at the start of the paragraph:
    assertXPathContent(pXmlDoc, "//reqif-xhtml:p[2]", u"\xa0\xa0\xa0\xa0 second");
    // Test a tab which is not leading:
    assertXPathContent(pXmlDoc, "//reqif-xhtml:p[3]", u"thi \t rd");
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testLeadingTabHTML)
{
    // Given a document with leading tabs:
    createSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Insert(u"\t test"_ustr);

    // When exporting to plain HTML, using LeadingTabWidth=2:
    uno::Sequence<beans::PropertyValue> aStoreProperties = {
        comphelper::makePropertyValue(u"FilterName"_ustr, u"HTML (StarWriter)"_ustr),
        comphelper::makePropertyValue(u"LeadingTabWidth"_ustr, static_cast<sal_Int32>(2)),
    };
    saveWithParams(aStoreProperties);

    // Then make sure that leading tabs are replaced with 2 nbsps:
    htmlDocUniquePtr pHtmlDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pHtmlDoc);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: <newline><nbsp><nbsp><space>test
    // - Actual  : <newline><tab><space>test
    // i.e. the leading tab was not replaced by 2 nbsps.
    assertXPathContent(pHtmlDoc, "/html/body/p", SAL_NEWLINE_STRING u"\xa0\xa0 test");
}

CPPUNIT_TEST_FIXTURE(HtmlExportTest, testClearingBreak)
{
    auto verify = [this]() {
        uno::Reference<container::XEnumerationAccess> xParagraph(getParagraph(1), uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xPortions = xParagraph->createEnumeration();
        uno::Reference<beans::XPropertySet> xPortion;
        OUString aPortionType;
        while (true)
        {
            // Ignore leading comments.
            xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
            xPortion->getPropertyValue(u"TextPortionType"_ustr) >>= aPortionType;
            if (aPortionType != "Annotation")
            {
                break;
            }
        }
        // Skip "foo".
        // Without the accompanying fix in place, this test would have failed with:
        // An uncaught exception of type com.sun.star.container.NoSuchElementException
        // i.e. the first para was just comments + text portion, the clearing break was lost.
        xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
        xPortion->getPropertyValue(u"TextPortionType"_ustr) >>= aPortionType;
        CPPUNIT_ASSERT_EQUAL(u"LineBreak"_ustr, aPortionType);
        uno::Reference<text::XTextContent> xLineBreak;
        xPortion->getPropertyValue(u"LineBreak"_ustr) >>= xLineBreak;
        sal_Int16 eClear{};
        uno::Reference<beans::XPropertySet> xLineBreakProps(xLineBreak, uno::UNO_QUERY);
        xLineBreakProps->getPropertyValue(u"Clear"_ustr) >>= eClear;
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(SwLineBreakClear::ALL), eClear);
    };

    // Given a document with an at-para anchored image + a clearing break:
    // When loading that file:
    createSwWebDoc("clearing-break.html");
    // Then make sure that the clear property of the break is not ignored:
    verify();
    saveAndReload(mpFilter);
    // Make sure that the clear property of the break is not ignored during export:
    verify();
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testTableBackground)
{
    // Given a document with two tables: first stable has a background, second table has a
    // background in its first row:
    createSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    SwInsertTableOptions aInsertTableOptions(SwInsertTableFlags::DefaultBorder,
                                             /*nRowsToRepeat=*/0);
    pWrtShell->InsertTable(aInsertTableOptions, /*nRows=*/1, /*nCols=*/1);
    pWrtShell->MoveTable(GotoPrevTable, fnTableStart);
    SvxBrushItem aBrush(COL_LIGHTRED, RES_BACKGROUND);
    pWrtShell->SetTabBackground(aBrush);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->SplitNode();
    pWrtShell->InsertTable(aInsertTableOptions, /*nRows=*/2, /*nCols=*/1);
    pWrtShell->MoveTable(GotoPrevTable, fnTableStart);
    aBrush.SetColor(COL_LIGHTGREEN);
    pWrtShell->SetRowBackground(aBrush);
    pWrtShell->Down(/*bSelect=*/false);
    // Second row has an explicit transparent background.
    aBrush.SetColor(COL_TRANSPARENT);
    pWrtShell->SetRowBackground(aBrush);

    // When exporting to reqif-xhtml:
    ExportToReqif();

    // Then make sure that CSS markup is used, not HTML one:
    xmlDocUniquePtr pXmlDoc = WrapReqifFromTempFile();
    // Without the accompanying fix in place, this test would have failed with:
    // - XPath '//reqif-xhtml:table[1]' no attribute 'style' exist
    // i.e. HTML markup was used for the table background color.
    assertXPath(pXmlDoc, "//reqif-xhtml:table[1]", "style", u"background: #ff0000");
    assertXPathNoAttribute(pXmlDoc, "//reqif-xhtml:table[1]", "bgcolor");
    assertXPath(pXmlDoc, "//reqif-xhtml:table[2]/reqif-xhtml:tr[1]", "style",
                u"background: #00ff00");
    assertXPathNoAttribute(pXmlDoc, "//reqif-xhtml:table[2]/reqif-xhtml:tr[1]", "bgcolor");
    // Second row has no explicit style, the default is not written.
    assertXPathNoAttribute(pXmlDoc, "//reqif-xhtml:table[2]/reqif-xhtml:tr[2]", "style");
}

CPPUNIT_TEST_FIXTURE(HtmlExportTest, testImageKeepRatio)
{
    // Given a document with an image: width is relative, height is "keep ratio":
    createSwDoc();
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xTextGraphic(
        xFactory->createInstance(u"com.sun.star.text.TextGraphicObject"_ustr), uno::UNO_QUERY);
    xTextGraphic->setPropertyValue(u"AnchorType"_ustr,
                                   uno::Any(text::TextContentAnchorType_AS_CHARACTER));
    xTextGraphic->setPropertyValue(u"RelativeWidth"_ustr, uno::Any(static_cast<sal_Int16>(42)));
    xTextGraphic->setPropertyValue(u"IsSyncHeightToWidth"_ustr, uno::Any(true));
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xBodyText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor(xBodyText->createTextCursor());
    uno::Reference<text::XTextContent> xTextContent(xTextGraphic, uno::UNO_QUERY);
    xBodyText->insertTextContent(xCursor, xTextContent, false);

    // When exporting to HTML:
    save(mpFilter);

    // Then make sure that the width is not a fixed size, that would break on resizing the browser
    // window:
    htmlDocUniquePtr pDoc = parseHtml(maTempFile);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: auto
    // - Actual  : 2
    // i.e. a static (CSS pixel) height was written.
    assertXPath(pDoc, "/html/body/p/img", "height", u"auto");
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testSectionDir)
{
    // Given a document with a section:
    createSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Insert(u"test"_ustr);
    pWrtShell->SelAll();
    SwSectionData aSectionData(SectionType::Content, u"mysect"_ustr);
    pWrtShell->InsertSection(aSectionData);

    // When exporting to (reqif-)xhtml:
    ExportToReqif();

    // Then make sure CSS is used to export the text direction of the section:
    xmlDocUniquePtr pXmlDoc = WrapReqifFromTempFile();
    // Without the accompanying fix in place, this test would have failed with:
    // - XPath '//reqif-xhtml:div[@id='mysect']' no attribute 'style' exist
    // i.e. the dir="ltr" HTML attribute was used instead.
    assertXPath(pXmlDoc, "//reqif-xhtml:div[@id='mysect']", "style", u"dir: ltr");
}

CPPUNIT_TEST_FIXTURE(HtmlExportTest, testTdf114769)
{
    // Create document from scratch since relative urls to filesystem can be replaced
    // by absolute during save/load
    createSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Insert(u"Hyperlink1"_ustr);
    pWrtShell->SplitNode();
    pWrtShell->Insert(u"Hyperlink2"_ustr);
    pWrtShell->SplitNode();
    pWrtShell->Insert(u"Hyperlink3"_ustr);
    pWrtShell->SplitNode();
    pWrtShell->Insert(u"Hyperlink4"_ustr);
    pWrtShell->SplitNode();
    pWrtShell->Insert(u"Hyperlink5"_ustr);
    pWrtShell->SplitNode();

    // Normal external URL
    uno::Reference<beans::XPropertySet> xRun(getRun(getParagraph(1), 1), uno::UNO_QUERY);
    xRun->setPropertyValue(u"HyperLinkURL"_ustr, uno::Any(u"http://libreoffice.org/"_ustr));

    // Bookmark reference
    xRun.set(getRun(getParagraph(2), 1), uno::UNO_QUERY);
    xRun->setPropertyValue(u"HyperLinkURL"_ustr, uno::Any(u"#some_bookmark"_ustr));

    // Filesystem absolute link
    xRun.set(getRun(getParagraph(3), 1), uno::UNO_QUERY);
    xRun->setPropertyValue(u"HyperLinkURL"_ustr, uno::Any(u"C:\\test.txt"_ustr));

    // Filesystem relative link
    xRun.set(getRun(getParagraph(4), 1), uno::UNO_QUERY);
    xRun->setPropertyValue(u"HyperLinkURL"_ustr, uno::Any(u"..\\..\\test.odt"_ustr));

    // Filesystem relative link
    xRun.set(getRun(getParagraph(5), 1), uno::UNO_QUERY);
    xRun->setPropertyValue(u"HyperLinkURL"_ustr, uno::Any(u".\\another.odt"_ustr));

    // Export
    save(mpFilter);

    htmlDocUniquePtr pHtmlDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pHtmlDoc);

    assertXPath(pHtmlDoc, "/html/body/p[1]/a", "href", u"http://libreoffice.org/");
    assertXPath(pHtmlDoc, "/html/body/p[2]/a", "href", u"#some_bookmark");
    assertXPath(pHtmlDoc, "/html/body/p[3]/a", "href", u"C:\\test.txt");
    assertXPath(pHtmlDoc, "/html/body/p[4]/a", "href", u"..\\..\\test.odt");
    assertXPath(pHtmlDoc, "/html/body/p[5]/a", "href", u".\\another.odt");
}

CPPUNIT_TEST_FIXTURE(HtmlExportTest, testTdf153923)
{
    createSwDoc("TableWithIndent.fodt");
    save(mpFilter);

    // Parse it as XML (strict!)
    xmlDocUniquePtr pDoc = parseXml(maTempFile);
    // Without the fix in place, this would fail
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/html/body//dl", 3);
    // The 'dd' tag was not closed
    assertXPath(pDoc, "/html/body//dd", 3);
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testTdf153923_ReqIF)
{
    createSwDoc("TableWithIndent.fodt");
    ExportToReqif();

    xmlDocUniquePtr pDoc = WrapReqifFromTempFile();

    assertXPath(pDoc, "//reqif-xhtml:table");
    // There should be no 'dd' or 'dl' tags, used as a hack for table indentation
    assertXPath(pDoc, "//reqif-xhtml:dl", 0);
    assertXPath(pDoc, "//reqif-xhtml:dd", 0);
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqIfTransparentTifImg)
{
    // reqIf export must keep the TIF encoding of the image
    createSwDoc("reqif-transparent-tif-img.odt");
    ExportToReqif();

    xmlDocUniquePtr pXmlDoc = WrapReqifFromTempFile();
    assertXPath(pXmlDoc, "//reqif-xhtml:p/reqif-xhtml:object[1]", "type", u"image/tiff");
    OUString imageName = getXPath(pXmlDoc, "//reqif-xhtml:p/reqif-xhtml:object[1]", "data");
    // Without the accompanying fix in place, this test would have failed,
    // ending with .gif, because XOutFlags::UseGifIfSensible flag combined
    // with the transparent image would result in GIF export
    CPPUNIT_ASSERT(imageName.endsWith(".tif"));

    INetURLObject aURL(maTempFile.GetURL());
    aURL.setName(imageName);
    GraphicDescriptor aDescriptor(aURL);
    aDescriptor.Detect();
    CPPUNIT_ASSERT_EQUAL(GraphicFileFormat::TIF, aDescriptor.GetFileFormat());
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testTdf155387)
{
    createSwDoc("sub_li_and_ctd.fodt");
    ExportToReqif();

    // Without the fix in place, this would fail
    xmlDocUniquePtr pDoc = WrapReqifFromTempFile();

    // Single top-level list
    assertXPath(pDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:ul");
    // Single top-level item
    assertXPath(pDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:ul/reqif-xhtml:li");
    // 4 top-level paragraphs in the item
    assertXPath(pDoc,
                "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:ul/reqif-xhtml:li/reqif-xhtml:p", 4);
    // 2 sublists in the item
    assertXPath(
        pDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:ul/reqif-xhtml:li/reqif-xhtml:ul", 2);
    // 2 items in the first sublist
    assertXPath(pDoc,
                "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:ul/reqif-xhtml:li/reqif-xhtml:ul[1]/"
                "reqif-xhtml:li",
                2);
    // Check the last (most nested) subitem's text
    assertXPathContent(
        pDoc,
        "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:ul/reqif-xhtml:li/reqif-xhtml:ul[2]/"
        "reqif-xhtml:li/reqif-xhtml:ul/reqif-xhtml:li/reqif-xhtml:p",
        u"l3");
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testTdf155496)
{
    createSwDoc("listItemSubheader.fodt");
    ExportToReqif();

    // Without the fix in place, this would fail
    xmlDocUniquePtr pDoc = WrapReqifFromTempFile();

    // Two top-level lists
    assertXPath(pDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:ul", 2);
    // Single top-level item
    assertXPath(pDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:ul[1]/reqif-xhtml:li");
    // One top-level paragraph in the item
    assertXPath(pDoc,
                "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:ul[1]/reqif-xhtml:li/reqif-xhtml:p");
    // One sublist in the item
    assertXPath(
        pDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:ul[1]/reqif-xhtml:li/reqif-xhtml:ul");
    // One item in the sublist
    assertXPath(pDoc,
                "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:ul[1]/reqif-xhtml:li/reqif-xhtml:ul/"
                "reqif-xhtml:li");
    // Check its text
    OUString aContent = getXPathContent(
        pDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:ul[1]/reqif-xhtml:li/reqif-xhtml:ul/"
              "reqif-xhtml:li/reqif-xhtml:p");
    CPPUNIT_ASSERT_EQUAL(u"list 1 item 1\n\t\tsub-header"_ustr, aContent.trim());
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqIF_RightAlignedTable)
{
    createSwDoc("tableRight.fodt");
    ExportToReqif();

    xmlDocUniquePtr pDoc = WrapReqifFromTempFile();

    // No 'align' attribute must be present in 'div'
    assertXPathNoAttribute(pDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:div", "align");
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqIF_ListsWithNumFormat)
{
    createSwDoc("listsWithNumFormat.fodt");
    ExportToReqif();

    xmlDocUniquePtr pDoc = WrapReqifFromTempFile();

    // No 'type' attribute must be present in 'ol'
    assertXPathNoAttribute(pDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:ol[1]", "type");
    assertXPathNoAttribute(pDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:ol[2]", "type");
    assertXPathNoAttribute(pDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:ol[3]", "type");
    assertXPathNoAttribute(pDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:ol[4]", "type");
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testTdf155871)
{
    createSwDoc("tdf155871.fodt");
    ExportToReqif();

    // Without the fix in place, this would fail
    xmlDocUniquePtr pDoc = WrapReqifFromTempFile();
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqIF_ListsNoStartAttribute)
{
    createSwDoc("twoListsWithSameStyle.fodt");
    ExportToReqif();

    xmlDocUniquePtr pDoc = WrapReqifFromTempFile();

    // No 'start' attribute must be present in 'ol'
    assertXPath(pDoc, "//reqif-xhtml:ol[@start]", 0);
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqIF_FrameTextAsObjectAltText)
{
    createSwDoc("frameWithText.fodt");
    ExportToReqif();

    xmlDocUniquePtr pDoc = WrapReqifFromTempFile();

    // Without the fix, this would fail with
    // - Expected: Some text in frame & <foo>
    // - Actual  : Frame1
    // i.e., frame name was used as the object element content, not frame text
    assertXPathContent(pDoc,
                       "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:p[2]/reqif-xhtml:object",
                       u"Some text in frame & <foo>");
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testSingleOleExport)
{
    // Given a document containing an embedded OLE object:
    createSwDoc("ole2.odt");

    // Create a selection for that object:
    auto xDrawPageSupplier(mxComponent.queryThrow<css::drawing::XDrawPageSupplier>());
    auto xDrawPage(xDrawPageSupplier->getDrawPage());
    auto xModel(mxComponent.queryThrow<css::frame::XModel>());
    auto xController(xModel->getCurrentController().queryThrow<css::view::XSelectionSupplier>());
    xController->select(xDrawPage->getByIndex(0));

    // Store only the selection
    css::uno::Sequence<css::beans::PropertyValue> aStoreProperties = {
        comphelper::makePropertyValue(u"FilterName"_ustr, u"HTML (StarWriter)"_ustr),
        comphelper::makePropertyValue(u"FilterOptions"_ustr, u"xhtmlns=reqif-xhtml"_ustr),
        comphelper::makePropertyValue(u"RTFOLEMimeType"_ustr, u"text/rtf"_ustr),
        comphelper::makePropertyValue(u"SelectionOnly"_ustr, true),
    };
    saveWithParams(aStoreProperties);

    xmlDocUniquePtr pXmlDoc = WrapReqifFromTempFile();

    // The root element must be reqif-xhtml:object
    assertXPath(pXmlDoc, "/reqif-xhtml:html/reqif-xhtml:object", "type", u"text/rtf");
    // It has no children
    assertXPathChildren(pXmlDoc, "/reqif-xhtml:html/reqif-xhtml:object", 0);
    // And the content is empty
    assertXPathContent(pXmlDoc, "/reqif-xhtml:html/reqif-xhtml:object", u"");

    OUString aRtfData = getXPath(pXmlDoc, "/reqif-xhtml:html/reqif-xhtml:object", "data");
    INetURLObject aUrl(maTempFile.GetURL());
    aUrl.setName(aRtfData);
    SvMemoryStream aRtf;
    HtmlExportTest::wrapRtfFragment(aUrl.GetMainURL(INetURLObject::DecodeMechanism::NONE), aRtf);
    tools::SvRef<TestReqIfRtfReader> xReader(new TestReqIfRtfReader(aRtf));
    // The RTF OLE exports correctly
    CPPUNIT_ASSERT(xReader->CallParser() != SvParserState::Error);
    CPPUNIT_ASSERT_EQUAL(tools::Long(9358), xReader->GetObjw());
    CPPUNIT_ASSERT_EQUAL(tools::Long(450), xReader->GetObjh());
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqIF_Tdf156602)
{
    createSwDoc("NestingInA1.fodt");
    ExportToReqif();

    xmlDocUniquePtr pDoc = WrapReqifFromTempFile();

    // The outer table must be kept in the document where the outer table is the first element,
    // and its A1 starts with a nested table

    // Only two sub-elements must be inside the div: an outer table and a trailing paragraph
    assertXPathChildren(pDoc, "/reqif-xhtml:html/reqif-xhtml:div", 2);
    // The outer table must have exactly two rows
    assertXPath(pDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:table/reqif-xhtml:tr", 2);
    // First outer table cell must have two sub-elements: an inner table and a trailing paragraph
    assertXPathChildren(
        pDoc,
        "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:table/reqif-xhtml:tr[1]/reqif-xhtml:td[1]",
        2);
    // The inner table must have exactly two rows
    assertXPath(
        pDoc,
        "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:table/reqif-xhtml:tr[1]/reqif-xhtml:td[1]/"
        "reqif-xhtml:table/reqif-xhtml:tr",
        2);
    // Check all the elements' content
    assertXPathContent(
        pDoc,
        "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:table/reqif-xhtml:tr[1]/reqif-xhtml:td[1]/"
        "reqif-xhtml:table/reqif-xhtml:tr[1]/reqif-xhtml:td[1]/reqif-xhtml:p",
        u"Inner.A1");
    assertXPathContent(
        pDoc,
        "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:table/reqif-xhtml:tr[1]/reqif-xhtml:td[1]/"
        "reqif-xhtml:table/reqif-xhtml:tr[1]/reqif-xhtml:td[2]/reqif-xhtml:p",
        u"Inner.B1");
    assertXPathContent(
        pDoc,
        "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:table/reqif-xhtml:tr[1]/reqif-xhtml:td[1]/"
        "reqif-xhtml:table/reqif-xhtml:tr[2]/reqif-xhtml:td[1]/reqif-xhtml:p",
        u"Inner.A2");
    assertXPathContent(
        pDoc,
        "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:table/reqif-xhtml:tr[1]/reqif-xhtml:td[1]/"
        "reqif-xhtml:table/reqif-xhtml:tr[2]/reqif-xhtml:td[2]/reqif-xhtml:p",
        u"Inner.B2");
    assertXPathContent(
        pDoc,
        "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:table/reqif-xhtml:tr[1]/reqif-xhtml:td[1]/"
        "reqif-xhtml:p",
        u"Outer.A1");
    assertXPathContent(
        pDoc,
        "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:table/reqif-xhtml:tr[1]/reqif-xhtml:td[2]/"
        "reqif-xhtml:p",
        u"Outer.B1");
    assertXPathContent(
        pDoc,
        "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:table/reqif-xhtml:tr[2]/reqif-xhtml:td[1]/"
        "reqif-xhtml:p",
        u"Outer.A2");
    assertXPathContent(
        pDoc,
        "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:table/reqif-xhtml:tr[2]/reqif-xhtml:td[2]/"
        "reqif-xhtml:p",
        u"Outer.B2");
    assertXPathContent(pDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:p", u"Following text");
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testTdf156647_CellPaddingRoundtrip)
{
    // Given a document with a table with cell padding:
    createSwDoc("table_cell_padding.fodt");
    {
        auto xTable = getParagraphOrTable(1);
        auto aTableBorder = getProperty<css::table::TableBorder2>(xTable, u"TableBorder2"_ustr);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(1270), aTableBorder.Distance);
        CPPUNIT_ASSERT(aTableBorder.IsDistanceValid);
    }
    // When exporting to reqif-xhtml:
    ExportToReqif();
    // Make sure that we export it:
    xmlDocUniquePtr pXmlDoc = WrapReqifFromTempFile();
    assertXPath(pXmlDoc, "//reqif-xhtml:table", "cellpadding", u"48"); // px
    // Now import it
    ImportFromReqif(maTempFile.GetURL());
    // Then make sure that padding is not lost:
    {
        auto xTable = getParagraphOrTable(1);
        auto aTableBorder = getProperty<css::table::TableBorder2>(xTable, u"TableBorder2"_ustr);
        // Without the accompanying fix in place, this test would have failed:
        // - Expected: 1270
        // - Actual  : 97
        // as the padding was lost, and the default 55 twip padding was used.
        CPPUNIT_ASSERT_EQUAL(sal_Int16(1270), aTableBorder.Distance);
        CPPUNIT_ASSERT(aTableBorder.IsDistanceValid);
    }
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testTdf157643_WideHBorder)
{
    // Given a document with a table with a wide border between its two rows:
    createSwDoc("table_with_wide_horizontal_border.fodt");
    // When exporting to reqif-xhtml:
    ExportToReqif();
    // Make sure that there's no extra tr's:
    xmlDocUniquePtr pXmlDoc = WrapReqifFromTempFile();
    assertXPath(pXmlDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:table/reqif-xhtml:tr", 2);
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testHTML_PreserveSpaces)
{
    // Given a document with leading, trailing, and repeating intermediate spaces:
    createSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    static constexpr OUString paraText = u"\t test  \t more  text \t"_ustr;
    pWrtShell->Insert(paraText);

    // When exporting to plain HTML, using PreserveSpaces:
    saveWithParams({
        comphelper::makePropertyValue(u"FilterName"_ustr, u"HTML (StarWriter)"_ustr),
        comphelper::makePropertyValue(u"PreserveSpaces"_ustr, true),
    });

    // Then make sure that "white-space: pre-wrap" is written into the paragraph's style:
    htmlDocUniquePtr pHtmlDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pHtmlDoc);
    const OUString style = getXPath(pHtmlDoc, "/html/body/p", "style");
    CPPUNIT_ASSERT(style.indexOf("white-space: pre-wrap") >= 0);
    // Also check that the paragraph text is correct, without modifications in whitespace
    assertXPathContent(pHtmlDoc, "/html/body/p", paraText);

    // Test import

    setImportFilterName(u"HTML (StarWriter)"_ustr);
    loadFromURL(maTempFile.GetURL());
    CPPUNIT_ASSERT_EQUAL(paraText, getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqIF_PreserveSpaces)
{
    // Given a document with leading, trailing, and repeating intermediate spaces:
    createSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    static constexpr OUString paraText = u"\t test  \t more  text \t"_ustr;
    pWrtShell->Insert(paraText);

    // When exporting to ReqIF, using PreserveSpaces:
    saveWithParams({
        comphelper::makePropertyValue(u"FilterName"_ustr, u"HTML (StarWriter)"_ustr),
        comphelper::makePropertyValue(u"FilterOptions"_ustr, u"xhtmlns=reqif-xhtml"_ustr),
        comphelper::makePropertyValue(u"PreserveSpaces"_ustr, true),
    });

    // Then make sure that xml:space="preserve" attribute exists in the paragraph element:
    xmlDocUniquePtr pXmlDoc = WrapReqifFromTempFile();
    assertXPath(pXmlDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:p", "space", u"preserve");
    // Also check that the paragraph text is correct, without modifications in whitespace
    assertXPathContent(pXmlDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:p", paraText);

    // Test import

    setImportFilterOptions(u"xhtmlns=reqif-xhtml"_ustr);
    setImportFilterName(u"HTML (StarWriter)"_ustr);
    loadFromURL(maTempFile.GetURL());
    CPPUNIT_ASSERT_EQUAL(paraText, getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testHTML_NoPreserveSpaces)
{
    // Test cases where "PreserveSpaces" should not introduce respective markup

    const auto assertXPath_NoWhiteSpaceInStyle
        = [this](const xmlDocUniquePtr& pDoc, const char* pXPath) {
              xmlXPathObjectPtr pXmlObj = getXPathNode(pDoc, pXPath);
              xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
              CPPUNIT_ASSERT_EQUAL_MESSAGE(pXPath, 1, xmlXPathNodeSetGetLength(pXmlNodes));
              CPPUNIT_ASSERT(pXmlNodes);
              xmlNodePtr pXmlNode = pXmlNodes->nodeTab[0];
              if (xmlChar* prop = xmlGetProp(pXmlNode, BAD_CAST("style")))
              {
                  OUString style = OUString::fromUtf8(reinterpret_cast<const char*>(prop));
                  CPPUNIT_ASSERT_MESSAGE(pXPath, style.indexOf("white-space:") < 0);
              }
              xmlXPathFreeObject(pXmlObj);
          };
    const auto assertXPath_HasWhiteSpaceInStyle
        = [this](const xmlDocUniquePtr& pDoc, const char* pXPath) {
              const OUString style = getXPath(pDoc, pXPath, "style");
              CPPUNIT_ASSERT_MESSAGE(pXPath, style.indexOf("white-space: pre-wrap") >= 0);
          };

    createSwDoc("test_no_space_preserve.fodt");

    // Export to plain HTML, using PreserveSpaces:
    saveWithParams({
        comphelper::makePropertyValue(u"FilterName"_ustr, u"HTML (StarWriter)"_ustr),
        comphelper::makePropertyValue(u"PreserveSpaces"_ustr, true),
    });

    htmlDocUniquePtr pHtmlDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pHtmlDoc);

    // No whitespace preservation, where no leading / trailing / double whitespace
    assertXPath_NoWhiteSpaceInStyle(pHtmlDoc, "/html/body/p[1]");
    // Whitespace preserved for a leading space
    assertXPath_HasWhiteSpaceInStyle(pHtmlDoc, "/html/body/p[2]");
    // Whitespace preserved for a trailing space
    assertXPath_HasWhiteSpaceInStyle(pHtmlDoc, "/html/body/p[3]");
    // Whitespace preserved for a double space
    assertXPath_HasWhiteSpaceInStyle(pHtmlDoc, "/html/body/p[4]");
    // No whitespace preservation for leading / trailing breaks
    assertXPath_NoWhiteSpaceInStyle(pHtmlDoc, "/html/body/p[5]");
    // Whitespace preserved for a leading break + space
    assertXPath_HasWhiteSpaceInStyle(pHtmlDoc, "/html/body/p[6]");
    // Whitespace preserved for a trailing space + break
    assertXPath_HasWhiteSpaceInStyle(pHtmlDoc, "/html/body/p[7]");
    // No whitespace preservation for a middle break
    assertXPath_NoWhiteSpaceInStyle(pHtmlDoc, "/html/body/p[8]");
    // Whitespace preserved for a middle space + break
    assertXPath_HasWhiteSpaceInStyle(pHtmlDoc, "/html/body/p[9]");
    // Whitespace preserved for a middle break + space
    assertXPath_HasWhiteSpaceInStyle(pHtmlDoc, "/html/body/p[10]");
    // No whitespace preservation for a trailing space and SVG
    assertXPath_NoWhiteSpaceInStyle(pHtmlDoc, "/html/body/p[11]");

    // Test import

    setImportFilterName(u"HTML (StarWriter)"_ustr);
    loadFromURL(maTempFile.GetURL());

    CPPUNIT_ASSERT_EQUAL(u"No special spaces"_ustr, getParagraph(1)->getString());
    CPPUNIT_ASSERT_EQUAL(u" Leading space"_ustr, getParagraph(2)->getString());
    CPPUNIT_ASSERT_EQUAL(u"Trailing space "_ustr, getParagraph(3)->getString());
    CPPUNIT_ASSERT_EQUAL(u"Double  space"_ustr, getParagraph(4)->getString());
    // Trailing break is removed in SwHTMLParser::AppendTextNode, and replaced with para spacing
    CPPUNIT_ASSERT_EQUAL(u"\nLeading/trailing breaks"_ustr, getParagraph(5)->getString());
    CPPUNIT_ASSERT_EQUAL(u"\n Leading break + space"_ustr, getParagraph(6)->getString());
    // Trailing break is removed in SwHTMLParser::AppendTextNode, and replaced with para spacing
    CPPUNIT_ASSERT_EQUAL(u"Trailing space + break "_ustr, getParagraph(7)->getString());
    CPPUNIT_ASSERT_EQUAL(u"Middle\nbreak"_ustr, getParagraph(8)->getString());
    CPPUNIT_ASSERT_EQUAL(u"Middle space \n+ break"_ustr, getParagraph(9)->getString());
    CPPUNIT_ASSERT_EQUAL(u"Middle break\n + space"_ustr, getParagraph(10)->getString());
    // The SVG is replaced by a space in SwXParagraph::getString()
    CPPUNIT_ASSERT_EQUAL(u"Trailing space and SVG  "_ustr, getParagraph(11)->getString());
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqIF_NoPreserveSpaces)
{
    // Test cases where "PreserveSpaces" should not introduce respective markup

    createSwDoc("test_no_space_preserve.fodt");

    // Export to ReqIF, using PreserveSpaces:
    saveWithParams({
        comphelper::makePropertyValue(u"FilterName"_ustr, u"HTML (StarWriter)"_ustr),
        comphelper::makePropertyValue(u"FilterOptions"_ustr, u"xhtmlns=reqif-xhtml"_ustr),
        comphelper::makePropertyValue(u"PreserveSpaces"_ustr, true),
    });

    xmlDocUniquePtr pXmlDoc = WrapReqifFromTempFile();

    // No whitespace preservation, where no leading / trailing / double whitespace
    assertXPathNoAttribute(pXmlDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:p[1]", "space");
    // Whitespace preserved for a leading space
    assertXPath(pXmlDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:p[2]", "space",
                u"preserve");
    // Whitespace preserved for a trailing space
    assertXPath(pXmlDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:p[3]", "space",
                u"preserve");
    // Whitespace preserved for a double space
    assertXPath(pXmlDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:p[4]", "space",
                u"preserve");
    // No whitespace preservation for leading / trailing breaks
    assertXPathNoAttribute(pXmlDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:p[5]", "space");
    // Whitespace preserved for a leading break + space
    assertXPath(pXmlDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:p[6]", "space",
                u"preserve");
    // No whitespace preservation for a trailing space + break
    assertXPathNoAttribute(pXmlDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:p[7]", "space");
    // No whitespace preservation for a middle break
    assertXPathNoAttribute(pXmlDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:p[8]", "space");
    // No whitespace preservation for a middle space + break
    assertXPathNoAttribute(pXmlDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:p[9]", "space");
    // Whitespace preserved for a middle break + space
    assertXPath(pXmlDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:p[10]", "space",
                u"preserve");
    // No whitespace preservation for a trailing space and SVG
    assertXPathNoAttribute(pXmlDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:p[11]", "space");

    // Test import

    setImportFilterOptions(u"xhtmlns=reqif-xhtml"_ustr);
    setImportFilterName(u"HTML (StarWriter)"_ustr);
    loadFromURL(maTempFile.GetURL());

    CPPUNIT_ASSERT_EQUAL(u"No special spaces"_ustr, getParagraph(1)->getString());
    CPPUNIT_ASSERT_EQUAL(u" Leading space"_ustr, getParagraph(2)->getString());
    CPPUNIT_ASSERT_EQUAL(u"Trailing space "_ustr, getParagraph(3)->getString());
    CPPUNIT_ASSERT_EQUAL(u"Double  space"_ustr, getParagraph(4)->getString());
    CPPUNIT_ASSERT_EQUAL(u"\nLeading/trailing breaks\n"_ustr, getParagraph(5)->getString());
    CPPUNIT_ASSERT_EQUAL(u"\n Leading break + space"_ustr, getParagraph(6)->getString());
    CPPUNIT_ASSERT_EQUAL(u"Trailing space + break \n"_ustr, getParagraph(7)->getString());
    CPPUNIT_ASSERT_EQUAL(u"Middle\nbreak"_ustr, getParagraph(8)->getString());
    CPPUNIT_ASSERT_EQUAL(u"Middle space \n+ break"_ustr, getParagraph(9)->getString());
    CPPUNIT_ASSERT_EQUAL(u"Middle break\n + space"_ustr, getParagraph(10)->getString());
    // The SVG is replaced by a space in SwXParagraph::getString()
    CPPUNIT_ASSERT_EQUAL(u"Trailing space and SVG  "_ustr, getParagraph(11)->getString());
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqIF_ExportFormulasAsPDF)
{
    // Given a document with a formula:
    createSwDoc("embedded_formula.fodt");

    // When exporting to reqif with ExportFormulasAsPDF=true:
    uno::Sequence<beans::PropertyValue> aStoreProperties = {
        comphelper::makePropertyValue(u"FilterName"_ustr, u"HTML (StarWriter)"_ustr),
        comphelper::makePropertyValue(u"FilterOptions"_ustr, u"xhtmlns=reqif-xhtml"_ustr),
        comphelper::makePropertyValue(u"ExportFormulasAsPDF"_ustr, true),
    };
    saveWithParams(aStoreProperties);

#if HAVE_FEATURE_PDFIMPORT
    // Make sure that the formula is exported as PDF:
    xmlDocUniquePtr pXmlDoc = WrapReqifFromTempFile();
    assertXPath(pXmlDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:p[2]/reqif-xhtml:object",
                "type", u"application/pdf");

    css::uno::Sequence<css::beans::PropertyValue> descr{
        comphelper::makePropertyValue(u"URL"_ustr, GetObjectPath(u".pdf"_ustr)),
    };

    uno::Reference<lang::XMultiServiceFactory> xFactory(
        comphelper::getProcessComponentContext()->getServiceManager(), uno::UNO_QUERY_THROW);
    uno::Reference<document::XTypeDetection> xTypeDetection(
        xFactory->createInstance(u"com.sun.star.document.TypeDetection"_ustr),
        uno::UNO_QUERY_THROW);

    CPPUNIT_ASSERT_EQUAL(u"pdf_Portable_Document_Format"_ustr,
                         xTypeDetection->queryTypeByDescriptor(descr, true));
#endif
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqIF_NoBrClearForImageWrap)
{
    // Given a document with a paragraph-anchored image with "none" wrap:
    createSwDoc("image_anchored_to_paragraph_no_wrap.fodt");
    // When exporting to reqif:
    ExportToReqif();
    // Make sure that there's no 'br' elements in the 'object' (used to represent the wrapping
    // in HTML export, using 'clear' attribute):
    xmlDocUniquePtr pXmlDoc = WrapReqifFromTempFile();
    assertXPath(pXmlDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:p/reqif-xhtml:object");
    assertXPath(pXmlDoc,
                "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:p/reqif-xhtml:object/reqif-xhtml:br",
                0);
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqIF_Tdf160017_spanClosingOrder)
{
    // Given a document with a paragraph having explicit font color and character border properties:
    createSwDoc("char_border_and_font_color.fodt");
    // When exporting to reqif:
    ExportToReqif();
    // Without the fix, this would fail, because there was an extra closing </reqif-xhtml:span>
    WrapReqifFromTempFile();
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testHTML_Tdf160017_spanClosingOrder)
{
    // Given a document with a paragraph having explicit font color and character border properties:
    createSwDoc("char_border_and_font_color.fodt");
    // When exporting to HTML:
    ExportToHTML();
    // Parse it as XML (strict!)
    // Without the fix, this would fail, because span and font elements closed in wrong order
    CPPUNIT_ASSERT(parseXml(maTempFile));
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testHTML_Tdf160390)
{
    // This document must not hang infinitely on HTML export
    createSwDoc("tdf160390.fodt");
    ExportToHTML();
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testHTML_160867)
{
    // Given a document with an image with hyperlink, and text with hyperlink, both in a frame:
    createSwDoc("tdf160867_image_with_link.fodt");
    // When exporting to HTML:
    ExportToHTML();
    // Parse it as XML (strict!)
    xmlDocUniquePtr pDoc = parseXml(maTempFile);
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/html/body/p", 2);

    // Test export of image and text hyperlinks in the image map.
    // Without the fix, the test would fail with
    // - Expected: 1
    // - Actual  : 0
    // - In <>, XPath '/html/body/p[2]/map' number of nodes is incorrect
    const OUString mapName = getXPath(pDoc, "/html/body/p[2]/map", "name");
    assertXPath(pDoc, "/html/body/p[2]/map/area[1]", "shape", u"rect");
    CPPUNIT_ASSERT(getXPath(pDoc, "/html/body/p[2]/map/area[1]", "href").endsWith("foo/bar"));
    assertXPath(pDoc, "/html/body/p[2]/map/area[2]", "shape", u"rect");
    CPPUNIT_ASSERT(getXPath(pDoc, "/html/body/p[2]/map/area[2]", "href").endsWith("baz"));
    assertXPath(pDoc, "/html/body/p[2]/img", "usemap", Concat2View("#" + mapName));
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqIF_160867)
{
    // Given a document with an image with hyperlink, and text with hyperlink, both in a frame:
    createSwDoc("tdf160867_image_with_link.fodt");
    // When exporting to reqif:
    ExportToReqif();
    // For now, we don't (yet) output the whole map in ReqIF case.
    // Make sure that the first hyperlink from the objects in the frame is output as an <a> element
    // around the whole image of the frame.
    xmlDocUniquePtr pXmlDoc = WrapReqifFromTempFile();
    assertXPath(pXmlDoc, "//reqif-xhtml:p[2]/reqif-xhtml:a/reqif-xhtml:object");
    CPPUNIT_ASSERT(
        getXPath(pXmlDoc, "//reqif-xhtml:p[2]/reqif-xhtml:a", "href").endsWith("foo/bar"));
    // There must be no 'target' attribute
    assertXPathNoAttribute(pXmlDoc, "//reqif-xhtml:p[2]/reqif-xhtml:a", "target");
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testHTML_161979)
{
    // Given a document with two embedded metafiles:
    createSwDoc("tdf161979_metafile.fodt");
    ExportToHTML();
    xmlDocUniquePtr pDoc = parseXml(maTempFile);
    CPPUNIT_ASSERT(pDoc);
    // First image: it has no EMF+ actions, and didn't use canvas rendering before the fix;
    // yet, it didn't export correctly.
    OUString imgName = getXPath(pDoc, "/html/body/p[2]/img", "src");
    CPPUNIT_ASSERT(imgName.endsWith(".gif"));
    INetURLObject aUrl(maTempFile.GetURL());
    aUrl.setName(imgName);
    Graphic graphic;
    CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE, GraphicFilter().ImportGraphic(graphic, aUrl));

    // Check that only ~4% of pixels are not transparent (before the fix, it was completely black)
    BitmapEx bitmap = graphic.GetBitmapEx();
    Size size = bitmap.GetSizePixel();
    int numNonTransparent = 0;
    for (tools::Long y = 0; y < size.Height(); ++y)
        for (tools::Long x = 0; x < size.Width(); ++x)
            if (bitmap.GetPixelColor(x, y) != COL_TRANSPARENT)
                ++numNonTransparent;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.04, numNonTransparent / double(size.Height() * size.Width()),
                                 0.01);

    // Second image: it consists of EMF+ records (no EMF fallback). It used canvas rendering
    // before the fix; it also didn't export correctly.
    imgName = getXPath(pDoc, "/html/body/p[4]/img", "src");
    CPPUNIT_ASSERT(imgName.endsWith(".gif"));
    aUrl.SetURL(maTempFile.GetURL());
    aUrl.setName(imgName);
    graphic.Clear();
    CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE, GraphicFilter().ImportGraphic(graphic, aUrl));

    // Check that some pixels are transparent (before the fix, it was completely black)
    bitmap = graphic.GetBitmapEx();
    size = bitmap.GetSizePixel();
    numNonTransparent = 0;
    for (tools::Long y = 0; y < size.Height(); ++y)
        for (tools::Long x = 0; x < size.Width(); ++x)
            if (bitmap.GetPixelColor(x, y) != COL_TRANSPARENT)
                ++numNonTransparent;
    CPPUNIT_ASSERT(numNonTransparent > 0);
    CPPUNIT_ASSERT(numNonTransparent < size.Height() * size.Width());
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqIF_exportAbsoluteURLs_ownRelative)
{
    auto pBatch(comphelper::ConfigurationChanges::create());
    Resetter resetter([
        bInternetPreviousValue = officecfg::Office::Common::Save::URL::Internet::get(),
        bFileSystemPreviousValue = officecfg::Office::Common::Save::URL::FileSystem::get(), pBatch
    ]() {
        officecfg::Office::Common::Save::URL::Internet::set(bInternetPreviousValue, pBatch);
        officecfg::Office::Common::Save::URL::FileSystem::set(bFileSystemPreviousValue, pBatch);
        return pBatch->commit();
    });
    // Set saving absolute URLs
    officecfg::Office::Common::Save::URL::Internet::set(false, pBatch);
    officecfg::Office::Common::Save::URL::FileSystem::set(false, pBatch);
    pBatch->commit();

    createSwDoc("URLs.odt");
    // Export to ReqIF, using absolute URLs
    saveWithParams({
        comphelper::makePropertyValue(u"FilterName"_ustr, u"HTML (StarWriter)"_ustr),
        comphelper::makePropertyValue(u"FilterOptions"_ustr, u"xhtmlns=reqif-xhtml"_ustr),
        comphelper::makePropertyValue(u"ExportImagesAsOLE"_ustr, true),
        comphelper::makePropertyValue(u"RelativeOwnObjectURL"_ustr, true),
    });
    xmlDocUniquePtr pXmlDoc = WrapReqifFromTempFile();

    // HTTP URL: must be absolute
    assertXPath(pXmlDoc, "//reqif-xhtml:p[1]/reqif-xhtml:a", "href", u"http://www.example.org/");
    // file URL: must be absolute
    assertXPath(pXmlDoc, "//reqif-xhtml:p[2]/reqif-xhtml:a", "href",
                createFileURL(u"NonExistingPath/NonExistingFile.html"));
    // form URL: must be absolute
    assertXPath(pXmlDoc, "//reqif-xhtml:form", "action", u"https://www.example.org/submit");
    // linked image exported as object: generated, must be relative
    OUString url = getXPath(pXmlDoc, "//reqif-xhtml:p[3]/reqif-xhtml:object", "data");
    CPPUNIT_ASSERT(!url.startsWith("file:"));
    CPPUNIT_ASSERT(url.endsWith(".ole"));
    // its original image URL: must be absolute
    assertXPath(pXmlDoc, "//reqif-xhtml:p[3]/reqif-xhtml:object/reqif-xhtml:object", "data",
                createFileURL(u"external.png"));
    // embedded image exported as object: generated, must be relative
    url = getXPath(pXmlDoc, "//reqif-xhtml:p[4]/reqif-xhtml:object", "data");
    CPPUNIT_ASSERT(!url.startsWith("file:"));
    CPPUNIT_ASSERT(url.endsWith(".ole"));
    // its image URL: generated, must be relative
    url = getXPath(pXmlDoc, "//reqif-xhtml:p[4]/reqif-xhtml:object/reqif-xhtml:object", "data");
    CPPUNIT_ASSERT(!url.startsWith("file:"));
    CPPUNIT_ASSERT(url.endsWith(".png"));
    // unordered list with image bullet - it gets embedded as base64 data
    OUString style = getXPath(pXmlDoc, "//reqif-xhtml:ul", "style");
    CPPUNIT_ASSERT(style.indexOf("list-style-image: url(data:image/png;base64,") != -1);
    // an as-char frame, exported as a whole to an object, must be relative
    url = getXPath(pXmlDoc, "//reqif-xhtml:p[5]/reqif-xhtml:object", "data");
    CPPUNIT_ASSERT(!url.startsWith("file:"));
    CPPUNIT_ASSERT(url.endsWith(".ole"));
    // its file hyperlink must be absolute
    assertXPath(pXmlDoc, "//reqif-xhtml:p[5]/reqif-xhtml:object/reqif-xhtml:a", "href",
                createFileURL(u"foo/bar"));
    // its image URL: generated, must be relative
    url = getXPath(
        pXmlDoc, "//reqif-xhtml:p[5]/reqif-xhtml:object/reqif-xhtml:a/reqif-xhtml:object", "data");
    CPPUNIT_ASSERT(!url.startsWith("file:"));
    CPPUNIT_ASSERT(url.endsWith(".png"));
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqIF_exportRelativeURLs)
{
    CPPUNIT_ASSERT(officecfg::Office::Common::Save::URL::Internet::get());
    CPPUNIT_ASSERT(officecfg::Office::Common::Save::URL::FileSystem::get());

    createSwDoc("URLs.odt");
    // Export to ReqIF, using relative URLs (the default)
    saveWithParams({
        comphelper::makePropertyValue(u"FilterName"_ustr, u"HTML (StarWriter)"_ustr),
        comphelper::makePropertyValue(u"FilterOptions"_ustr, u"xhtmlns=reqif-xhtml"_ustr),
        comphelper::makePropertyValue(u"ExportImagesAsOLE"_ustr, true),
    });
    xmlDocUniquePtr pXmlDoc = WrapReqifFromTempFile();

    // HTTP URL: must be absolute
    assertXPath(pXmlDoc, "//reqif-xhtml:p[1]/reqif-xhtml:a", "href", u"http://www.example.org/");
    // file URL: must be relative
    OUString url = getXPath(pXmlDoc, "//reqif-xhtml:p[2]/reqif-xhtml:a", "href");
    CPPUNIT_ASSERT(!url.startsWith("file:"));
    CPPUNIT_ASSERT(url.endsWith("NonExistingPath/NonExistingFile.html"));
    // form URL: must be absolute
    assertXPath(pXmlDoc, "//reqif-xhtml:form", "action", u"https://www.example.org/submit");
    // linked image exported as object: generated, must be relative
    url = getXPath(pXmlDoc, "//reqif-xhtml:p[3]/reqif-xhtml:object", "data");
    CPPUNIT_ASSERT(!url.startsWith("file:"));
    CPPUNIT_ASSERT(url.endsWith(".ole"));
    // its original image URL: must be relative
    url = getXPath(pXmlDoc, "//reqif-xhtml:p[3]/reqif-xhtml:object/reqif-xhtml:object", "data");
    CPPUNIT_ASSERT(!url.startsWith("file:"));
    CPPUNIT_ASSERT(url.endsWith("external.png"));
    // embedded image exported as object: generated, must be relative
    url = getXPath(pXmlDoc, "//reqif-xhtml:p[4]/reqif-xhtml:object", "data");
    CPPUNIT_ASSERT(!url.startsWith("file:"));
    CPPUNIT_ASSERT(url.endsWith(".ole"));
    // its image URL: generated, must be relative
    url = getXPath(pXmlDoc, "//reqif-xhtml:p[4]/reqif-xhtml:object/reqif-xhtml:object", "data");
    CPPUNIT_ASSERT(!url.startsWith("file:"));
    CPPUNIT_ASSERT(url.endsWith(".png"));
    // unordered list with image bullet - it gets embedded as base64 data
    OUString style = getXPath(pXmlDoc, "//reqif-xhtml:ul", "style");
    CPPUNIT_ASSERT(style.indexOf("list-style-image: url(data:image/png;base64,") != -1);
    // an as-char frame, exported as a whole to an object, must be relative
    url = getXPath(pXmlDoc, "//reqif-xhtml:p[5]/reqif-xhtml:object", "data");
    CPPUNIT_ASSERT(!url.startsWith("file:"));
    CPPUNIT_ASSERT(url.endsWith(".ole"));
    // its file hyperlink must be relative
    url = getXPath(pXmlDoc, "//reqif-xhtml:p[5]/reqif-xhtml:object/reqif-xhtml:a", "href");
    CPPUNIT_ASSERT(!url.startsWith("file:"));
    CPPUNIT_ASSERT(url.endsWith("foo/bar"));
    // its image URL: generated, must be relative
    url = getXPath(
        pXmlDoc, "//reqif-xhtml:p[5]/reqif-xhtml:object/reqif-xhtml:a/reqif-xhtml:object", "data");
    CPPUNIT_ASSERT(!url.startsWith("file:"));
    CPPUNIT_ASSERT(url.endsWith(".png"));
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testHTML_exportAbsoluteURLs_ownRelative)
{
    auto pBatch(comphelper::ConfigurationChanges::create());
    Resetter resetter([
        bInternetPreviousValue = officecfg::Office::Common::Save::URL::Internet::get(),
        bFileSystemPreviousValue = officecfg::Office::Common::Save::URL::FileSystem::get(), pBatch
    ]() {
        officecfg::Office::Common::Save::URL::Internet::set(bInternetPreviousValue, pBatch);
        officecfg::Office::Common::Save::URL::FileSystem::set(bFileSystemPreviousValue, pBatch);
        return pBatch->commit();
    });
    // Set saving absolute URLs
    officecfg::Office::Common::Save::URL::Internet::set(false, pBatch);
    officecfg::Office::Common::Save::URL::FileSystem::set(false, pBatch);
    pBatch->commit();

    createSwDoc("URLs.odt");
    // Export to HTML, using absolute URLs
    saveWithParams({
        comphelper::makePropertyValue(u"FilterName"_ustr, u"HTML (StarWriter)"_ustr),
        comphelper::makePropertyValue(u"RelativeOwnObjectURL"_ustr, true),
    });
    htmlDocUniquePtr pHtmlDoc = parseHtml(maTempFile);

    // HTTP URL: must be absolute
    assertXPath(pHtmlDoc, "//p[1]/a", "href", u"http://www.example.org/");
    // file URL: must be absolute
    assertXPath(pHtmlDoc, "//p[2]/a", "href",
                createFileURL(u"NonExistingPath/NonExistingFile.html"));
    // form URL: must be absolute
    assertXPath(pHtmlDoc, "//form", "action", u"https://www.example.org/submit");
    // linked image: must be absolute
    assertXPath(pHtmlDoc, "//p[3]/img", "src", createFileURL(u"external.png"));
    // embedded image: generated, must be relative
    OUString url = getXPath(pHtmlDoc, "//p[4]/img", "src");
    CPPUNIT_ASSERT(!url.startsWith("file:"));
    CPPUNIT_ASSERT(url.endsWith(".png"));
    // unordered list with image bullet - it gets embedded as base64 data
    OUString style = getXPath(pHtmlDoc, "//ul", "style");
    CPPUNIT_ASSERT(style.indexOf("list-style-image: url(data:image/png;base64,") != -1);
    // image-in-frame file hyperlink must be absolute; FIXME: HTMLOutFuncs::Out_ImageMap
    // assertXPath(pHtmlDoc, "//p[5]/map/area", "href", createFileURL(u"foo/bar"));
    // its image URL: generated, must be relative
    url = getXPath(pHtmlDoc, "//p[5]/img", "src");
    CPPUNIT_ASSERT(!url.startsWith("file:"));
    CPPUNIT_ASSERT(url.endsWith(".gif"));
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testHTML_exportRelativeURLs)
{
    CPPUNIT_ASSERT(officecfg::Office::Common::Save::URL::Internet::get());
    CPPUNIT_ASSERT(officecfg::Office::Common::Save::URL::FileSystem::get());

    createSwDoc("URLs.odt");
    // Export to HTML, using relative URLs (the default)
    ExportToHTML();
    htmlDocUniquePtr pHtmlDoc = parseHtml(maTempFile);

    // HTTP URL: must be absolute
    assertXPath(pHtmlDoc, "//p[1]/a", "href", u"http://www.example.org/");
    // file URL: must be relative
    OUString url = getXPath(pHtmlDoc, "//p[2]/a", "href");
    CPPUNIT_ASSERT(!url.startsWith("file:"));
    CPPUNIT_ASSERT(url.endsWith("NonExistingPath/NonExistingFile.html"));
    // form URL: must be absolute
    assertXPath(pHtmlDoc, "//form", "action", u"https://www.example.org/submit");
    // linked image: must be relative
    url = getXPath(pHtmlDoc, "//p[3]/img", "src");
    CPPUNIT_ASSERT(!url.startsWith("file:"));
    CPPUNIT_ASSERT(url.endsWith("external.png"));
    // embedded image: generated, must be relative
    url = getXPath(pHtmlDoc, "//p[4]/img", "src");
    CPPUNIT_ASSERT(!url.startsWith("file:"));
    CPPUNIT_ASSERT(url.endsWith(".png"));
    // unordered list with image bullet - it gets embedded as base64 data
    OUString style = getXPath(pHtmlDoc, "//ul", "style");
    CPPUNIT_ASSERT(style.indexOf("list-style-image: url(data:image/png;base64,") != -1);
    // image-in-frame file hyperlink must be relative
    url = getXPath(pHtmlDoc, "//p[5]/map/area", "href");
    CPPUNIT_ASSERT(!url.startsWith("file:"));
    CPPUNIT_ASSERT(url.endsWith("foo/bar"));
    // its image URL: generated, must be relative
    url = getXPath(pHtmlDoc, "//p[5]/img", "src");
    CPPUNIT_ASSERT(!url.startsWith("file:"));
    CPPUNIT_ASSERT(url.endsWith(".gif"));
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqIF_162282)
{
    // Given a document with an embedded metafile:
    createSwDoc("tdf162282.odt");
    ExportToReqif();
    xmlDocUniquePtr pDoc = WrapReqifFromTempFile();

    // Check that the exported EMF is exactly the same as in the ODF package
    assertXPath(pDoc, "//reqif-xhtml:p/reqif-xhtml:object", "type", u"image/x-emf");
    OUString imgName = getXPath(pDoc, "//reqif-xhtml:p/reqif-xhtml:object", "data");
    CPPUNIT_ASSERT(imgName.endsWith(".emf"));
    INetURLObject aUrl(maTempFile.GetURL());
    aUrl.setName(imgName);
    SvFileStream aEmfStream(aUrl.GetMainURL(INetURLObject::DecodeMechanism::NONE),
                            StreamMode::READ);

    // without the fix, this would fail with
    // - Expected: 220
    // - Actual  : 111260
    CPPUNIT_ASSERT_EQUAL(sal_uInt64(220), aEmfStream.TellEnd());

    css::uno::Sequence<sal_uInt8> emfData(220);
    aEmfStream.ReadBytes(emfData.getArray(), emfData.getLength());

    const css::uno::Sequence<sal_uInt8> correctData{
        0x01, 0x00, 0x00, 0x00, 0x6C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0xF4, 0x01, 0x00, 0x00, 0xF4, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x88, 0x13, 0x00, 0x00, 0x88, 0x13, 0x00, 0x00, 0x20, 0x45, 0x4D, 0x46, 0x00,
        0x00, 0x01, 0x00, 0xDC, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x07, 0x00,
        0x00, 0x38, 0x04, 0x00, 0x00, 0xC0, 0x00, 0x00, 0x00, 0x6C, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF4, 0x01, 0x00, 0x00, 0xF4,
        0x01, 0x00, 0x00, 0x39, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
        0x2D, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x32, 0x00, 0x00, 0x00, 0x96, 0x00, 0x00,
        0x00, 0xFA, 0x00, 0x00, 0x00, 0x2C, 0x01, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x14, 0x00,
        0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x2F, 0x00, 0x00, 0x00, 0x28,
        0x00, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x2C, 0x01, 0x00, 0x00,
        0x2C, 0x01, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x2C, 0x01, 0x00,
        0x00, 0xC8, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00,
    };
    CPPUNIT_ASSERT_EQUAL(correctData, emfData);
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testHTML_162426)
{
    // Given a document with an image with style:wrap="none":
    createSwDoc("tdf162426_image_with_wrap_none.fodt");
    // Before the fix, an assertion failed in HtmlWriter::attribute when exporting to HTML :
    ExportToHTML();

    xmlDocUniquePtr pDoc = parseXml(maTempFile);
    CPPUNIT_ASSERT(pDoc);

    // Before the fix, the 'border' attribute was written after the 'img' tag was already closed,
    // so without the assertion, this would fail with
    // - In <>, XPath '/html/body/p/img' no attribute 'border' exist
    assertXPath(pDoc, "/html/body/p/img", "border", u"0");
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testHTML_163873)
{
    // Given a document with an image with style:wrap="none":
    createSwDoc("tdf131728.docx");
    // Before the fix, an assertion failed in HtmlWriter::attribute when exporting to HTML :
    ExportToHTML();

    xmlDocUniquePtr pDoc = parseXml(maTempFile);
    CPPUNIT_ASSERT(pDoc);

    // Before the fix, inline headings weren't inline
    assertXPath(pDoc, "/html/body/p[5]/span/h2", "style", u"display:inline;");
    assertXPath(pDoc, "/html/body/p[6]/span/h2", "style", u"display:inline;");
    assertXPath(pDoc, "/html/body/p[7]/span/h2", "style", u"display:inline;");
    assertXPath(pDoc, "/html/body/p[11]/span/h2", "style", u"display:inline;");
    assertXPath(pDoc, "/html/body/p[14]/span/h2", "style", u"display:inline;");
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqifNoTargetInA)
{
    // Given a document with a link with target:
    createSwDoc("link_with_target.fodt");

    // When exporting to XHTML:
    ExportToReqif();

    // Check that 'a' element has no 'target' attribute
    xmlDocUniquePtr pXmlDoc = WrapReqifFromTempFile();
    // Without the accompanying fix in place, this test would have failed
    assertXPathNoAttribute(pXmlDoc, "//reqif-xhtml:a", "target");
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqifNoObjectBorderExport)
{
    // Given a document with an image with black border
    createSwDoc("img_with_border.fodt");

    // When exporting to XHTML:
    ExportToReqif();

    // Check that there's no 'font' element that used to be exported, containing the border color:
    xmlDocUniquePtr pXmlDoc = WrapReqifFromTempFile();
    // Without the accompanying fix in place, this test would have failed
    assertXPath(pXmlDoc, "//reqif-xhtml:font", 0);
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
