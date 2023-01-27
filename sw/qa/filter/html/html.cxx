/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>
#include <test/htmltesttools.hxx>

#include <vcl/gdimtf.hxx>

#include <docsh.hxx>
#include <fmtfsize.hxx>
#include <frameformats.hxx>
#include <unotxdoc.hxx>
#include <itabenum.hxx>
#include <wrtsh.hxx>
#include <cellatr.hxx>

namespace
{
/**
 * Covers sw/source/filter/html/ fixes.
 *
 * Note that these tests are meant to be simple: either load a file and assert some result or build
 * a document model with code, export and assert that result.
 *
 * Keep using the various sw_<format>import/export suites for multiple filter calls inside a single
 * test.
 */
class Test : public SwModelTestBase, public HtmlTestTools
{
public:
    Test()
        : SwModelTestBase("/sw/qa/filter/html/data/", "HTML (StarWriter)")
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testEmptyParagraph)
{
    // Given a document with 2 paragraphs, the second is empty:
    setImportFilterOptions("xhtmlns=reqif-xhtml");
    setImportFilterName("HTML (StarWriter)");
    createSwDoc("empty-paragraph.xhtml");

    // Then make sure that the resulting document has a 2nd empty paragraph:
    getParagraph(1, "a");
    // Without the accompanying fix in place, this test would have failed with:
    // An uncaught exception of type com.sun.star.container.NoSuchElementException
    // i.e. the 2nd paragraph was lost.
    getParagraph(2);
}

CPPUNIT_TEST_FIXTURE(Test, testRelativeKeepAspect)
{
    // Given a document with an OLE object, width set to 100%, height is not set:
    setImportFilterOptions("xhtmlns=reqif-xhtml");
    setImportFilterName("HTML (StarWriter)");
    createSwDoc("relative-keep-aspect.xhtml");

    // Then make sure that the aspect ratio of the image is kept:
    auto pTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwDoc* pDoc = pTextDocument->GetDocShell()->GetDoc();
    const SwFrameFormats& rFormats = *pDoc->GetSpzFrameFormats();
    const SwFrameFormat* pFormat = rFormats[0];
    const SwFormatFrameSize& rSize = pFormat->GetFrameSize();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 255
    // - Actual  : 0
    // i.e. the height had a fixed value, not "keep aspect".
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(SwFormatFrameSize::SYNCED),
                         static_cast<int>(rSize.GetHeightPercent()));
}

CPPUNIT_TEST_FIXTURE(Test, testRelativeKeepAspectImage)
{
    // Given a document with an image, width set to 100%, height is not set:
    setImportFilterOptions("xhtmlns=reqif-xhtml");
    setImportFilterName("HTML (StarWriter)");
    createSwDoc("relative-keep-aspect-image.xhtml");

    // Then make sure that the aspect ratio of the image is kept:
    auto pTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwDoc* pDoc = pTextDocument->GetDocShell()->GetDoc();
    const SwFrameFormats& rFormats = *pDoc->GetSpzFrameFormats();
    const SwFrameFormat* pFormat = rFormats[0];
    const SwFormatFrameSize& rSize = pFormat->GetFrameSize();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 255
    // - Actual  : 0
    // i.e. the height had a fixed value, not "keep aspect".
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(SwFormatFrameSize::SYNCED),
                         static_cast<int>(rSize.GetHeightPercent()));
}

CPPUNIT_TEST_FIXTURE(Test, testSvmImageExport)
{
    // Given a document with an image, which has an SVM image data:
    createSwDoc();
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xTextGraphic(
        xFactory->createInstance("com.sun.star.text.TextGraphicObject"), uno::UNO_QUERY);
    xTextGraphic->setPropertyValue("AnchorType",
                                   uno::Any(text::TextContentAnchorType_AS_CHARACTER));
    GDIMetaFile aMetafile;
    Graphic aGraphic(aMetafile);
    xTextGraphic->setPropertyValue("Graphic", uno::Any(aGraphic.GetXGraphic()));
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xBodyText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor(xBodyText->createTextCursor());
    uno::Reference<text::XTextContent> xTextContent(xTextGraphic, uno::UNO_QUERY);
    xBodyText->insertTextContent(xCursor, xTextContent, false);

    // When exporting to reqif:
    setFilterOptions("xhtmlns=reqif-xhtml");
    save("HTML (StarWriter)");

    // Then make sure we only export PNG:
    SvMemoryStream aStream;
    WrapReqifFromTempFile(aStream);
    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aStream);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 2
    // - XPath '//reqif-xhtml:object' number of nodes is incorrect
    // i.e. we wrote both GIF and PNG, not just PNG for SVM images.
    assertXPath(pXmlDoc, "//reqif-xhtml:object", "type", "image/png");
}

CPPUNIT_TEST_FIXTURE(Test, testTableCellFloatValueType)
{
    // Given a document with a single table cell, its cell value is set to double:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwInsertTableOptions aTableOptions(SwInsertTableFlags::DefaultBorder, 0);
    pWrtShell->InsertTable(aTableOptions, 1, 1);
    pWrtShell->MoveTable(GotoPrevTable, fnTableStart);
    SwTableNode* pTableNode = pWrtShell->GetCursor()->GetPointNode().FindTableNode();
    SwTable& rTable = pTableNode->GetTable();
    auto pBox = const_cast<SwTableBox*>(rTable.GetTableBox("A1"));
    SwFrameFormat* pBoxFormat = pBox->ClaimFrameFormat();
    SwAttrSet aSet(pBoxFormat->GetAttrSet());
    SwTableBoxValue aBoxValue(42.0);
    aSet.Put(aBoxValue);
    pBoxFormat->GetDoc()->SetAttr(aSet, *pBoxFormat);

    // When exporting to XHTML:
    setFilterOptions("xhtmlns=reqif-xhtml");
    save("HTML (StarWriter)");

    // Then make sure that the sdval attribute is omitted, which is not in the XHTML spec:
    SvMemoryStream aStream;
    WrapReqifFromTempFile(aStream);
    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aStream);
    // Without the accompanying fix in place, this test would have failed with:
    // - XPath '//reqif-xhtml:td' unexpected 'sdval' attribute
    // i.e. sdval was written in XHTML mode.
    assertXPathNoAttribute(pXmlDoc, "//reqif-xhtml:td", "sdval");
    assertXPathNoAttribute(pXmlDoc, "//reqif-xhtml:td", "sdnum");
}

CPPUNIT_TEST_FIXTURE(Test, testTableRowSpanInAllCells)
{
    // Given a document with a 2x2 table, A1:A2 and B1:B2 is merged:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwInsertTableOptions aTableOptions(SwInsertTableFlags::DefaultBorder, 0);
    pWrtShell->InsertTable(aTableOptions, /*nRows=*/2, /*nCols=*/2);
    pWrtShell->MoveTable(GotoPrevTable, fnTableStart);
    SwTableNode* pTableNode = pWrtShell->GetCursor()->GetPointNode().FindTableNode();
    SwTable& rTable = pTableNode->GetTable();
    auto pBox = const_cast<SwTableBox*>(rTable.GetTableBox("A1"));
    pBox->setRowSpan(2);
    pBox = const_cast<SwTableBox*>(rTable.GetTableBox("B1"));
    pBox->setRowSpan(2);
    pBox = const_cast<SwTableBox*>(rTable.GetTableBox("A2"));
    pBox->setRowSpan(-1);
    pBox = const_cast<SwTableBox*>(rTable.GetTableBox("B2"));
    pBox->setRowSpan(-1);

    // When exporting to HTML:
    save("HTML (StarWriter)");

    // Then make sure that the output is simplified to valid HTML, by omitting the rowspan attribute
    // & the empty <tr> element:
    htmlDocUniquePtr pHtmlDoc = parseHtml(maTempFile);
    // Without the accompanying fix in place, this test would have failed with:
    // - XPath '//tr[1]/td[1]' unexpected 'rowspan' attribute
    // i.e. a combination of rowspan + empty <tr> was emitted.
    assertXPathNoAttribute(pHtmlDoc, "//tr[1]/td[1]", "rowspan");
    assertXPath(pHtmlDoc, "//tr", 1);
}

CPPUNIT_TEST_FIXTURE(Test, testCenteredTableCSSExport)
{
    // Given a document with a centered table:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwInsertTableOptions aTableOptions(SwInsertTableFlags::NONE, 0);
    pWrtShell->InsertTable(aTableOptions, 1, 1);
    pWrtShell->MoveTable(GotoPrevTable, fnTableStart);
    SfxItemSetFixed<RES_FRMATR_BEGIN, RES_FRMATR_END - 1> aSet(pWrtShell->GetAttrPool());
    SwFormatHoriOrient aHoriOrientItem(/*nX=*/0, text::HoriOrientation::CENTER);
    aSet.Put(aHoriOrientItem);
    pWrtShell->SetTableAttr(aSet);

    // When exporting to XHTML:
    setFilterOptions("xhtmlns=reqif-xhtml");
    save("HTML (StarWriter)");

    // Then make sure that CSS is used to horizontally position the table:
    SvMemoryStream aStream;
    WrapReqifFromTempFile(aStream);
    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aStream);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 1
    // i.e <center> was used to position the table, not CSS.
    assertXPath(pXmlDoc, "//reqif-xhtml:center", 0);
    assertXPath(pXmlDoc, "//reqif-xhtml:table", "style", "margin-left: auto; margin-right: auto");
}

CPPUNIT_TEST_FIXTURE(Test, testCenteredTableCSSImport)
{
    // Given an XHTML file with a centered (with inline CSS) table, when importing that document:
    setImportFilterOptions("xhtmlns=reqif-xhtml");
    setImportFilterName("HTML (StarWriter)");
    createSwDoc("centered-table.xhtml");

    // Then make sure that the table is centered:
    SwDoc* pDoc = getSwDoc();
    const SwFrameFormats& rTableFormats = *pDoc->GetTableFrameFormats();
    const SwFrameFormat* pTableFormat = rTableFormats[0];
    sal_Int16 eHoriOrient = pTableFormat->GetHoriOrient().GetHoriOrient();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2 (CENTER)
    // - Actual  : 3 (LEFT)
    // i.e. the table alignment was lost on import.
    CPPUNIT_ASSERT_EQUAL(text::HoriOrientation::CENTER, eHoriOrient);
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
