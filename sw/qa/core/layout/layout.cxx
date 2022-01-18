/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/style/PageStyleLayout.hpp>

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

CPPUNIT_TEST_FIXTURE(SwCoreLayoutTest, testBIRT)
{
    // this looped
    load(DATA_DIRECTORY, "birt_min.odt");
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
    assertXPath(pXmlDoc, "(//anchored/SwAnchoredDrawObject)[1]/bounds", "top", "1671");
    assertXPath(pXmlDoc, "(//anchored/SwAnchoredDrawObject)[1]/bounds", "bottom", "1732");
    assertXPath(pXmlDoc, "(//anchored/SwAnchoredDrawObject)[2]/bounds", "top", "1947");
    assertXPath(pXmlDoc, "(//anchored/SwAnchoredDrawObject)[2]/bounds", "bottom", "2008");
    assertXPath(pXmlDoc, "(//anchored/SwAnchoredDrawObject)[3]/bounds", "top", "3783");
    assertXPath(pXmlDoc, "(//anchored/SwAnchoredDrawObject)[3]/bounds", "bottom", "3844");
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
    assertXPath(pXmlDoc, "//anchored/fly/infos/bounds", "top", "1932");
    assertXPath(pXmlDoc, "//anchored/fly/infos/bounds", "bottom", "7184");
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
    sal_Int32 nFlyLeft = getXPath(pLayout, "//anchored/fly/infos/bounds", "left").toInt32();
    sal_Int32 nFlyTop = getXPath(pLayout, "//anchored/fly/infos/bounds", "top").toInt32();
    sal_Int32 nFlyRight = getXPath(pLayout, "//anchored/fly/infos/bounds", "right").toInt32();
    sal_Int32 nFlyBottom = getXPath(pLayout, "//anchored/fly/infos/bounds", "bottom").toInt32();
    tools::Rectangle aFlyRect(nFlyLeft, nFlyTop, nFlyRight, nFlyBottom);
    // Without the accompanying fix in place, this test would have failed, as aFlyRect was too wide,
    // so it was not inside aShapeRect anymore.
    CPPUNIT_ASSERT(aShapeRect.Contains(aFlyRect));
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

CPPUNIT_TEST_FIXTURE(SwCoreLayoutTest, testGutterMargin)
{
    // Create a document, remember the old left edge of the page print area (the rectangle that is
    // inside margins).
    SwDoc* pDoc = createSwDoc();
    uno::Reference<beans::XPropertySet> xStandard(getStyles("PageStyles")->getByName("Standard"),
                                                  uno::UNO_QUERY);
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    SwFrame* pPage = pLayout->GetLower();
    tools::Long nOldLeft = pPage->getFramePrintArea().Left();

    // Set the gutter margin to 2cm.
    sal_Int32 nGutterMm100 = 2000;
    xStandard->setPropertyValue("GutterMargin", uno::makeAny(nGutterMm100));

    // Verify that the new left edge is larger.
    tools::Long nNewLeft = pPage->getFramePrintArea().Left();
    tools::Long nGutterTwips = o3tl::toTwips(nGutterMm100, o3tl::Length::mm100);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1134
    // - Actual  : 0
    // i.e. the gutter was not added to the left margin.
    CPPUNIT_ASSERT_EQUAL(nGutterTwips, nNewLeft - nOldLeft);
}

CPPUNIT_TEST_FIXTURE(SwCoreLayoutTest, testGutterTopMargin)
{
    // Create a document, remember the old top edge of the page print area (the rectangle that is
    // inside margins).
    SwDoc* pDoc = createSwDoc();
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xSettings(
        xFactory->createInstance("com.sun.star.document.Settings"), uno::UNO_QUERY);
    xSettings->setPropertyValue("GutterAtTop", uno::makeAny(true));
    uno::Reference<beans::XPropertySet> xStandard(getStyles("PageStyles")->getByName("Standard"),
                                                  uno::UNO_QUERY);
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    SwFrame* pPage = pLayout->GetLower();
    tools::Long nOldTop = pPage->getFramePrintArea().Top();

    // Set the gutter margin to 2cm.
    sal_Int32 nGutterMm100 = 2000;
    xStandard->setPropertyValue("GutterMargin", uno::makeAny(nGutterMm100));

    // Verify that the new top edge is larger.
    tools::Long nNewTop = pPage->getFramePrintArea().Top();
    tools::Long nGutterTwips = o3tl::toTwips(nGutterMm100, o3tl::Length::mm100);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1134
    // - Actual  : 0
    // i.e. the gutter was not added to the left margin.
    CPPUNIT_ASSERT_EQUAL(nGutterTwips, nNewTop - nOldTop);
}

CPPUNIT_TEST_FIXTURE(SwCoreLayoutTest, testGutterMirrorMargin)
{
    SwDoc* pDoc = createSwDoc();
    SwDocShell* pDocShell = pDoc->GetDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->InsertPageBreak();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    SwFrame* pPage = pLayout->GetLower();
    tools::Long nOldLeft = pPage->getFramePrintArea().Left();
    SwFrame* pPage2 = pPage->GetNext();
    tools::Long nOldRight = pPage2->getFramePrintArea().Right();

    uno::Reference<beans::XPropertySet> xStandard(getStyles("PageStyles")->getByName("Standard"),
                                                  uno::UNO_QUERY);
    xStandard->setPropertyValue("PageStyleLayout", uno::makeAny(style::PageStyleLayout_MIRRORED));
    sal_Int32 nGutterMm100 = 2000;
    xStandard->setPropertyValue("GutterMargin", uno::makeAny(nGutterMm100));

    tools::Long nNewLeft = pPage->getFramePrintArea().Left();
    tools::Long nGutterTwips = o3tl::toTwips(nGutterMm100, o3tl::Length::mm100);
    CPPUNIT_ASSERT_EQUAL(nGutterTwips, nNewLeft - nOldLeft);
    tools::Long nNewRight = pPage2->getFramePrintArea().Right();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1134
    // - Actual  : 0
    // i.e. the gutter was missing on the second, mirrored page.
    CPPUNIT_ASSERT_EQUAL(nGutterTwips, nOldRight - nNewRight);
}

CPPUNIT_TEST_FIXTURE(SwCoreLayoutTest, testRtlGutterMargin)
{
    // Given a document with a right margin:
    SwDoc* pDoc = createSwDoc();
    uno::Reference<beans::XPropertySet> xStandard(getStyles("PageStyles")->getByName("Standard"),
                                                  uno::UNO_QUERY);
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    SwFrame* pPage = pLayout->GetLower();
    tools::Long nOldRight = pPage->getFramePrintArea().Right();

    // When setting enable RTL gutter mode and setting a gutter margin:
    xStandard->setPropertyValue("RtlGutter", uno::makeAny(true));
    sal_Int32 nGutterMm100 = 2000;
    xStandard->setPropertyValue("GutterMargin", uno::makeAny(nGutterMm100));

    // Then make sure the new right edge of the print area is decreased:
    tools::Long nNewRight = pPage->getFramePrintArea().Right();
    tools::Long nGutterTwips = o3tl::toTwips(nGutterMm100, o3tl::Length::mm100);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1134
    // - Actual  : 0
    // i.e. the gutter was missing on the right side.
    CPPUNIT_ASSERT_EQUAL(nGutterTwips, nOldRight - nNewRight);
}

CPPUNIT_TEST_FIXTURE(SwCoreLayoutTest, testGutterMarginPageBorder)
{
// FIXME this is 3369 on macOS -- calculate this number dynamically?
// FIXME this is random on Windows at the moment (in two subsequent tests without any scaling,
//       the actual values were 6346, 10066) - something broke metafile generation on Windows?
#if !defined(MACOSX) && !defined(_WIN32)
    // Given a document with a non-0 gutter margin.
    SwDoc* pDoc = createSwDoc();
    uno::Reference<beans::XPropertySet> xStandard(getStyles("PageStyles")->getByName("Standard"),
                                                  uno::UNO_QUERY);
    sal_Int32 nGutterMm100 = 2000;
    xStandard->setPropertyValue("GutterMargin", uno::makeAny(nGutterMm100));

    // When setting a left border.
    table::BorderLine2 aBorder;
    aBorder.LineWidth = 2;
    aBorder.OuterLineWidth = 2;
    xStandard->setPropertyValue("LeftBorder", uno::makeAny(aBorder));

    // Then make sure border is at the left edge of the text area.
    SwDocShell* pShell = pDoc->GetDocShell();
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2565
    // - Actual  : 1425
    // Where 2565 is close to the left edge of the text area (2553).
    assertXPath(pXmlDoc, "//polyline[@style='solid']/point[1]", "x", "2565");
#endif
}

CPPUNIT_TEST_FIXTURE(SwCoreLayoutTest, testTdf45908_invoice)
{
    // without the fix, this was hanging (and slowly consuming memory) on fileopen.
    load(DATA_DIRECTORY, "tdf45908_invoice.odt");
}

CPPUNIT_TEST_FIXTURE(SwCoreLayoutTest, testVerticallyMergedCellBorder)
{
    // Given a document with a table: 2 columns, 5 rows. B2 -> B5 is merged:
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "vmerge-cell-border.docx");
    SwDocShell* pShell = pDoc->GetDocShell();

    // When rendering the table:
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();

    // Make sure that B4->B5 has no borders.
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    // Collect vertical positions of all border points.
    xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, "//polyline[@style='solid']/point");
    xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
    std::vector<sal_Int32> aBorderPositions;
    for (int i = 0; i < xmlXPathNodeSetGetLength(pXmlNodes); ++i)
    {
        xmlNodePtr pXmlNode = pXmlNodes->nodeTab[i];
        xmlChar* pValue = xmlGetProp(pXmlNode, BAD_CAST("y"));
        sal_Int32 nValue = OString(reinterpret_cast<char const*>(pValue)).toInt32();
        aBorderPositions.push_back(nValue);
    }
    xmlXPathFreeObject(pXmlObj);
    // Collect top and bottom of the B1->B3 rows.
    xmlDocUniquePtr pLayout = parseLayoutDump();
    pXmlObj = getXPathNode(pLayout, "//tab/row/infos/bounds");
    pXmlNodes = pXmlObj->nodesetval;
    std::vector<sal_Int32> aLayoutPositions;
    for (int i = 0; i < 3; ++i)
    {
        xmlNodePtr pXmlNode = pXmlNodes->nodeTab[i];
        if (i == 0)
        {
            xmlChar* pValue = xmlGetProp(pXmlNode, BAD_CAST("top"));
            sal_Int32 nValue = OString(reinterpret_cast<char const*>(pValue)).toInt32();
            aLayoutPositions.push_back(nValue);
        }
        xmlChar* pValue = xmlGetProp(pXmlNode, BAD_CAST("bottom"));
        sal_Int32 nValue = OString(reinterpret_cast<char const*>(pValue)).toInt32();
        aLayoutPositions.push_back(nValue);
    }
    xmlXPathFreeObject(pXmlObj);
    // Check if any border is outside the B1->B3 range.
    for (const auto nBorderPosition : aBorderPositions)
    {
        bool bFound = false;
        for (const auto nLayoutPosition : aLayoutPositions)
        {
            if (std::abs(nBorderPosition - nLayoutPosition) <= 30)
            {
                bFound = true;
                break;
            }
        }
        std::stringstream ss;
        ss << "Bad vertical position for border point: " << nBorderPosition;
        ss << " Expected positions: ";
        for (size_t i = 0; i < aLayoutPositions.size(); ++i)
        {
            if (i > 0)
            {
                ss << ", ";
            }
            ss << aLayoutPositions[i];
        }

        // Without the accompanying fix in place, this test would have failed with:
        // - Bad vertical position for border point: 5624 Expected positions: 3022, 3540, 4059, 4578
        // i.e. the middle vertical border end was the bottom of B5, not bottom of B3.
        CPPUNIT_ASSERT_MESSAGE(ss.str(), bFound);
    }
}

CPPUNIT_TEST_FIXTURE(SwCoreLayoutTest, testCrashRemoveFromLayout)
{
    load(DATA_DIRECTORY, "tdf122894-4.doc");
}

CPPUNIT_TEST_FIXTURE(SwCoreLayoutTest, testLinkedBullet)
{
    // Given a document with a graphic bullet, where the image is a linked one:
    load(DATA_DIRECTORY, "linked-bullet.odt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwDocShell* pShell = pTextDoc->GetDocShell();

    // When rendering that document:
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();

    // Then make sure the render result contains exactly one bitmap:
    MetafileXmlDump aDumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(aDumper, *xMetaFile);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // i.e. the bullet's bitmap was lost.
    assertXPath(pXmlDoc, "//bmpexscale", 1);
}

CPPUNIT_TEST_FIXTURE(SwCoreLayoutTest, testInnerCellBorderIntersect)
{
    // Given a table with both outer and inner borders:
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "inner-border.docx");
    SwDocShell* pShell = pDoc->GetDocShell();

    // When rendering table borders:
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();

    // Then make sure that inner and outer borders don't overlap in Word compatibility mode,
    // and inner borders are reduced to prevent an overlap:
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    // Collect start/end (vertical) positions of horizontal borders.
    xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, "//polyline[@style='solid']/point");
    xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
    std::vector<std::pair<sal_Int32, sal_Int32>> aBorderStartEnds;
    for (int i = 0; i < xmlXPathNodeSetGetLength(pXmlNodes); i += 2)
    {
        xmlNodePtr pStart = pXmlNodes->nodeTab[i];
        xmlNodePtr pEnd = pXmlNodes->nodeTab[i + 1];
        xmlChar* pStartY = xmlGetProp(pStart, BAD_CAST("y"));
        xmlChar* pEndY = xmlGetProp(pEnd, BAD_CAST("y"));
        sal_Int32 nStartY = OString(reinterpret_cast<char const*>(pStartY)).toInt32();
        sal_Int32 nEndY = OString(reinterpret_cast<char const*>(pEndY)).toInt32();
        if (nStartY != nEndY)
        {
            // Vertical border.
            continue;
        }
        xmlChar* pStartX = xmlGetProp(pStart, BAD_CAST("x"));
        xmlChar* pEndX = xmlGetProp(pEnd, BAD_CAST("x"));
        sal_Int32 nStartX = OString(reinterpret_cast<char const*>(pStartX)).toInt32();
        sal_Int32 nEndX = OString(reinterpret_cast<char const*>(pEndX)).toInt32();
        aBorderStartEnds.emplace_back(nStartX, nEndX);
    }
    xmlXPathFreeObject(pXmlObj);
    // We have 3 lines: top, middle and bottom. The top and the bottom one is a full line, since
    // it's an outer border. The middle one has increased start and decreased end to avoid an
    // overlap.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), aBorderStartEnds.size());
    CPPUNIT_ASSERT_EQUAL(aBorderStartEnds[0].first, aBorderStartEnds[2].first);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected greater than: 1724
    // - Actual  : 1724
    // i.e. the middle line's start was the same as the top line start, while what we want is a
    // larger X position, so the start of the middle line doesn't overlap with the thick vertical
    // outer border on the left of the table.
    CPPUNIT_ASSERT_GREATER(aBorderStartEnds[0].first, aBorderStartEnds[1].first);
    CPPUNIT_ASSERT_EQUAL(aBorderStartEnds[0].second, aBorderStartEnds[2].second);
    CPPUNIT_ASSERT_LESS(aBorderStartEnds[0].second, aBorderStartEnds[1].second);
}

CPPUNIT_TEST_FIXTURE(SwCoreLayoutTest, testDoubleBorderVertical)
{
    // Given a table with a left and right double border, outer is thick, inner is thin:
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "double-border-vertical.docx");
    SwDocShell* pShell = pDoc->GetDocShell();

    // When rendering that document:
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();

    // Then make sure the left border is thick+thin and the right border is thin+thick (from left to
    // right):
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    // Collect widths of vertical lines.
    xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, "//polyline[@style='solid']/point");
    xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
    // Horizontal position -> width.
    std::map<sal_Int32, sal_Int32> aBorderWidths;
    for (int i = 0; i < xmlXPathNodeSetGetLength(pXmlNodes); i += 2)
    {
        xmlNodePtr pStart = pXmlNodes->nodeTab[i];
        xmlNodePtr pEnd = pXmlNodes->nodeTab[i + 1];
        xmlChar* pStartX = xmlGetProp(pStart, BAD_CAST("x"));
        xmlChar* pEndX = xmlGetProp(pEnd, BAD_CAST("x"));
        sal_Int32 nStartX = OString(reinterpret_cast<char const*>(pStartX)).toInt32();
        sal_Int32 nEndX = OString(reinterpret_cast<char const*>(pEndX)).toInt32();
        if (nStartX != nEndX)
        {
            // Horizontal border.
            continue;
        }
        xmlChar* pWidth = xmlGetProp(pStart->parent, BAD_CAST("width"));
        sal_Int32 nWidth = OString(reinterpret_cast<char const*>(pWidth)).toInt32();
        aBorderWidths[nStartX] = nWidth;
    }
    xmlXPathFreeObject(pXmlObj);
    std::vector<sal_Int32> aBorderWidthVec;
    std::transform(aBorderWidths.begin(), aBorderWidths.end(), std::back_inserter(aBorderWidthVec),
                   [](const std::pair<sal_Int32, sal_Int32>& rPair) { return rPair.second; });
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), aBorderWidthVec.size());
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected greater than: 120
    // - Actual  : 60
    // i.e. the left border was thin+thick, not thick+thin.
    CPPUNIT_ASSERT_GREATER(aBorderWidthVec[1], aBorderWidthVec[0]);
    CPPUNIT_ASSERT_GREATER(aBorderWidthVec[2], aBorderWidthVec[3]);
}

CPPUNIT_TEST_FIXTURE(SwCoreLayoutTest, testDoubleBorderHorizontal)
{
    // Given a table with a top and bottom double border, outer is thin, inner is thick:
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "double-border-horizontal.docx");
    SwDocShell* pShell = pDoc->GetDocShell();

    // When rendering table borders:
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();

    // Then make sure the top border is thin+thick and the bottom border is thick+thin (from top to
    // bottom):
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    // Collect widths of horizontal lines.
    xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, "//polyline[@style='solid']/point");
    xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
    // Vertical position -> width.
    std::map<sal_Int32, sal_Int32> aBorderWidths;
    for (int i = 0; i < xmlXPathNodeSetGetLength(pXmlNodes); i += 2)
    {
        xmlNodePtr pStart = pXmlNodes->nodeTab[i];
        xmlNodePtr pEnd = pXmlNodes->nodeTab[i + 1];
        xmlChar* pStartY = xmlGetProp(pStart, BAD_CAST("y"));
        xmlChar* pEndY = xmlGetProp(pEnd, BAD_CAST("y"));
        sal_Int32 nStartY = OString(reinterpret_cast<char const*>(pStartY)).toInt32();
        sal_Int32 nEndY = OString(reinterpret_cast<char const*>(pEndY)).toInt32();
        if (nStartY != nEndY)
        {
            // Vertical border.
            continue;
        }
        xmlChar* pWidth = xmlGetProp(pStart->parent, BAD_CAST("width"));
        sal_Int32 nWidth = OString(reinterpret_cast<char const*>(pWidth)).toInt32();
        aBorderWidths[nStartY] = nWidth;
    }
    xmlXPathFreeObject(pXmlObj);
    std::vector<sal_Int32> aBorderWidthVec;
    std::transform(aBorderWidths.begin(), aBorderWidths.end(), std::back_inserter(aBorderWidthVec),
                   [](const std::pair<sal_Int32, sal_Int32>& rPair) { return rPair.second; });
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), aBorderWidthVec.size());
    CPPUNIT_ASSERT_GREATER(aBorderWidthVec[0], aBorderWidthVec[1]);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected greater than: 120
    // - Actual  : 60
    // i.e. the bottom border was thin+thick, not thick+thin.
    CPPUNIT_ASSERT_GREATER(aBorderWidthVec[3], aBorderWidthVec[2]);
}

CPPUNIT_TEST_FIXTURE(SwCoreLayoutTest, testParaBorderInCellClip)
{
    // Given a document which has outside-cell borders defined, which should not be visible:
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "para-border-in-cell-clip.docx");
    SwDocShell* pShell = pDoc->GetDocShell();

    // When rendering those borders:
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();

    // Then make sure that we have clipping setup for both paragraphs inside the table cell:
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2
    // - Actual  : 0
    // - XPath '//clipregion/polygon' number of nodes is incorrect
    // i.e. there was no clipping applied, leading to unexpected left/right borders.
    assertXPath(pXmlDoc, "//clipregion/polygon", 2);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
