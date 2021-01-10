/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <vcl/gdimtf.hxx>
#include <svx/svdpage.hxx>

#include <wrtsh.hxx>
#include <docsh.hxx>
#include <unotxdoc.hxx>
#include <drawdoc.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentState.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <rootfrm.hxx>

constexpr OUStringLiteral DATA_DIRECTORY = u"/sw/qa/core/layout/data/";

/// Covers sw/source/core/layout/ fixes.
class SwCoreLayoutTest : public SwModelTestBase
{
};

CPPUNIT_TEST_FIXTURE(SwCoreLayoutTest, testTableFlyOverlap)
{
    // Load a document that has an image anchored in the header.
    // It also has a table which has the wrap around the image.
    load(DATA_DIRECTORY, "table-fly-overlap.docx");
    SwTwips nFlyTop = parseDump("//header/txt/anchored/fly/infos/bounds", "top").toInt32();
    SwTwips nFlyHeight = parseDump("//header/txt/anchored/fly/infos/bounds", "height").toInt32();
    SwTwips nFlyBottom = nFlyTop + nFlyHeight;
    SwTwips nTableFrameTop = parseDump("//tab/infos/bounds", "top").toInt32();
    SwTwips nTablePrintTop = parseDump("//tab/infos/prtBounds", "top").toInt32();
    SwTwips nTableTop = nTableFrameTop + nTablePrintTop;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected greater or equal than: 3579
    // - Actual  : 2210
    // i.e. the table's top border overlapped with the image, even if the image's wrap mode was set
    // to parallel.
    CPPUNIT_ASSERT_GREATEREQUAL(nFlyBottom, nTableTop);
}

CPPUNIT_TEST_FIXTURE(SwCoreLayoutTest, testTdf128195)
{
    // Load a document that has two paragraphs in the header.
    // The second paragraph should have its bottom spacing applied.
    load(DATA_DIRECTORY, "tdf128195.docx");
    sal_Int32 nTxtHeight = parseDump("//header/txt[2]/infos/bounds", "height").toInt32();
    sal_Int32 nTxtBottom = parseDump("//header/txt[2]/infos/bounds", "bottom").toInt32();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2269), nTxtHeight);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3529), nTxtBottom);
}

CPPUNIT_TEST_FIXTURE(SwCoreLayoutTest, testBorderCollapseCompat)
{
    // Load a document with a border conflict: top cell has a dotted bottom border, bottom cell has
    // a solid upper border.
    load(DATA_DIRECTORY, "border-collapse-compat.docx");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwDocShell* pShell = pTextDoc->GetDocShell();
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump aDumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(aDumper, *xMetaFile);

    // Make sure the solid border has priority.
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 48
    // i.e. there was no single cell border with width=20, rather there were 48 border parts
    // (forming a dotted border), all with width=40.
    assertXPath(pXmlDoc, "//polyline[@style='solid']", "width", "20");
}

CPPUNIT_TEST_FIXTURE(SwCoreLayoutTest, testBtlrTableRowSpan)
{
    // Load a document which has a table. The A1 cell has btlr text direction, and the A1..A3 cells
    // are merged.
    load(DATA_DIRECTORY, "btlr-table-row-span.odt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwDocShell* pShell = pTextDoc->GetDocShell();
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump aDumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(aDumper, *xMetaFile);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: USA
    // - Actual  : West
    // i.e. the "USA" text completely disappeared.
    assertXPathContent(pXmlDoc, "//textarray[1]/text", "USA");
}

CPPUNIT_TEST_FIXTURE(SwCoreLayoutTest, testTableFlyOverlapSpacing)
{
    // Load a document that has an image on the right of a table.  The table wraps around the image.
    load(DATA_DIRECTORY, "table-fly-overlap-spacing.docx");
    SwTwips nFlyTop = parseDump("//body/txt/anchored/fly/infos/bounds", "top").toInt32();
    SwTwips nFlyHeight = parseDump("//body/txt/anchored/fly/infos/bounds", "height").toInt32();
    SwTwips nFlyBottom = nFlyTop + nFlyHeight;
    SwTwips nTableFrameTop = parseDump("//tab/infos/bounds", "top").toInt32();
    SwTwips nTablePrintTop = parseDump("//tab/infos/prtBounds", "top").toInt32();
    SwTwips nTableTop = nTableFrameTop + nTablePrintTop;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected greater or equal than: 3993
    // - Actual  : 3993
    // i.e. the table was below the image, not on the left of the image.
    CPPUNIT_ASSERT_LESS(nFlyBottom, nTableTop);
}

CPPUNIT_TEST_FIXTURE(SwCoreLayoutTest, testTablesMoveBackwards)
{
    // Load a document with 1 pages: empty content on first page, then 21 tables on the second page.
    load(DATA_DIRECTORY, "tables-move-backwards.odt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwDocShell* pDocShell = pTextDoc->GetDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();

    // Delete the content on the first page.
    pWrtShell->SttEndDoc(/*bStart=*/true);
    pWrtShell->EndPg(/*bSelect=*/true);
    pWrtShell->DelLeft();

    // Calc the layout and check the number of pages.
    pWrtShell->CalcLayout();
    xmlDocUniquePtr pLayout = parseLayoutDump();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 2
    // i.e. there was an unexpected 2nd page, as only 20 out of 21 tables were moved to the first
    // page.
    assertXPath(pLayout, "//page", 1);
}

CPPUNIT_TEST_FIXTURE(SwCoreLayoutTest, testContinuousEndnotesMoveBackwards)
{
    // Load a document with the ContinuousEndnotes flag turned on.
    load(DATA_DIRECTORY, "continuous-endnotes-move-backwards.doc");
    xmlDocUniquePtr pLayout = parseLayoutDump();
    // We have 2 pages.
    assertXPath(pLayout, "/root/page", 2);
    // No endnote container on page 1.
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 1
    // i.e. there were unexpected endnotes on page 1.
    assertXPath(pLayout, "/root/page[1]/ftncont", 0);
    // All endnotes are in a container on page 2.
    assertXPath(pLayout, "/root/page[2]/ftncont", 1);
}

CPPUNIT_TEST_FIXTURE(SwCoreLayoutTest, testAnchorPositionBasedOnParagraph)
{
    // tdf#134783 check whether position of shape is good if it is anchored to paragraph and
    // the "Don't add space between paragraphs of the same style" option is set
    load(DATA_DIRECTORY, "tdf134783_testAnchorPositionBasedOnParagraph.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "(//SwAnchoredDrawObject)[1]/bounds", "top", "1671");
    assertXPath(pXmlDoc, "(//SwAnchoredDrawObject)[1]/bounds", "bottom", "1732");
    assertXPath(pXmlDoc, "(//SwAnchoredDrawObject)[2]/bounds", "top", "1947");
    assertXPath(pXmlDoc, "(//SwAnchoredDrawObject)[2]/bounds", "bottom", "2008");
    assertXPath(pXmlDoc, "(//SwAnchoredDrawObject)[3]/bounds", "top", "3783");
    assertXPath(pXmlDoc, "(//SwAnchoredDrawObject)[3]/bounds", "bottom", "3844");
}

CPPUNIT_TEST_FIXTURE(SwCoreLayoutTest, testTextBoxStaysInsideShape)
{
    // tdf#135198: check whether text box stays inside shape after moving it upwards
    load(DATA_DIRECTORY, "shape-textbox.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);

    // Without the fix in place, this test would have failed with
    // - Expected: 1932
    // - Actual  : 7476
    assertXPath(pXmlDoc, "//fly/infos/bounds", "top", "1932");
    assertXPath(pXmlDoc, "//fly/infos/bounds", "bottom", "7184");
}

CPPUNIT_TEST_FIXTURE(SwCoreLayoutTest, testTextBoxNotModifiedOnOpen)
{
    // tdf#138050: a freshly opened document containing a shape with a text box
    // should not appear to be modified
    load(DATA_DIRECTORY, "textbox-phantom-change.docx");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();

    // Without the fix in place this test would have shown that the document
    // was modified due to a fix to tdf#135198
    CPPUNIT_ASSERT(!pDoc->getIDocumentState().IsModified());
}

CPPUNIT_TEST_FIXTURE(SwCoreLayoutTest, testTextBoxAutoGrowVertical)
{
    load(DATA_DIRECTORY, "textbox-autogrow-vertical.docx");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pShape = pPage->GetObj(0);
    tools::Rectangle aShapeRect = pShape->GetCurrentBoundRect();

    discardDumpedLayout();
    xmlDocUniquePtr pLayout = parseLayoutDump();
    CPPUNIT_ASSERT(pLayout);
    sal_Int32 nFlyLeft = getXPath(pLayout, "//fly/infos/bounds", "left").toInt32();
    sal_Int32 nFlyTop = getXPath(pLayout, "//fly/infos/bounds", "top").toInt32();
    sal_Int32 nFlyRight = getXPath(pLayout, "//fly/infos/bounds", "right").toInt32();
    sal_Int32 nFlyBottom = getXPath(pLayout, "//fly/infos/bounds", "bottom").toInt32();
    tools::Rectangle aFlyRect(nFlyLeft, nFlyTop, nFlyRight, nFlyBottom);
    // Without the accompanying fix in place, this test would have failed, as aFlyRect was too wide,
    // so it was not inside aShapeRect anymore.
    CPPUNIT_ASSERT(aShapeRect.IsInside(aFlyRect));
}

CPPUNIT_TEST_FIXTURE(SwCoreLayoutTest, testTextboxModification)
{
    // Load a document with a textbox in it: the layout will have to position the shape part.
    load(DATA_DIRECTORY, "textbox-modification.docx");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwDocShell* pDocShell = pTextDoc->GetDocShell();

    // Without the accompanying fix in place, this test would have failed, as the document was
    // marked as modified right after the import.
    CPPUNIT_ASSERT(!pDocShell->IsModified());
}

CPPUNIT_TEST_FIXTURE(SwCoreLayoutTest, testBtlrNestedCell)
{
    // Load a document with a nested table, the inner A1 cell has a btlr text direction.
    load(DATA_DIRECTORY, "btlr-nested-cell.odt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    SwFrame* pPage = pLayout->GetLower();
    SwFrame* pBody = pPage->GetLower();
    SwFrame* pOuterTable = pBody->GetLower()->GetNext();
    SwFrame* pInnerTable = pOuterTable->GetLower()->GetLower()->GetLower();

    // Check the paint area of the only text frame in the cell.
    SwFrame* pTextFrame = pInnerTable->GetLower()->GetLower()->GetLower();
    tools::Long nFrameBottom = pTextFrame->getFrameArea().Bottom();
    SwRect aPaintArea = pTextFrame->GetPaintArea();

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected greater or equal than: 2829
    // - Actual  : 2080
    // i.e. part of the text frame area was not painted, hiding the actual text.
    CPPUNIT_ASSERT_GREATEREQUAL(nFrameBottom, aPaintArea.Bottom());
}

CPPUNIT_TEST_FIXTURE(SwCoreLayoutTest, testKeepwithnextFullheight)
{
    // The document has a heading (keep with next) and a full-page image in the next paragraph, i.e.
    // conflicting requirements.
    // Without the accompanying fix in place, this test would have failed with a layout loop in
    // SwEditShell::CalcLayout().
    load(DATA_DIRECTORY, "keepwithnext-fullheight.fodt");

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);
    // Make sure the document has 2 pages.
    assertXPath(pXmlDoc, "//page", 2);
    // Heading stays on page 1 to avoid a layout loop.
    assertXPathContent(pXmlDoc, "//page[1]/body/txt[2]", "Heading");
    // Image stays on page 2.
    assertXPath(pXmlDoc, "//page[2]/body/txt/anchored/fly", 1);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
