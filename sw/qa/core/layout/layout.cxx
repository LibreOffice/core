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

#include <wrtsh.hxx>

static char const DATA_DIRECTORY[] = "/sw/qa/core/layout/data/";

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
