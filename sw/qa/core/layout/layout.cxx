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
#include <comphelper/classids.hxx>
#include <tools/globname.hxx>
#include <svtools/embedhlp.hxx>

#include <wrtsh.hxx>
#include <fmtanchr.hxx>

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
    xmlDocPtr pXmlDoc = dumpAndParse(aDumper, *xMetaFile);

    // Make sure the solid border has priority.
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 48
    // i.e. there was no single cell border with width=20, rather there were 48 border parts
    // (forming a dotted border), all with width=40.
    assertXPath(pXmlDoc, "//polyline[@style='solid']", "width", "20");
}

CPPUNIT_TEST_FIXTURE(SwCoreLayoutTest, testGutterMargin)
{
    // Create a document, remember the old left edge of the page print area (the rectangle that is
    // inside margins).
    loadURL("private:factory/swriter", nullptr);
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    uno::Reference<beans::XPropertySet> xStandard(getStyles("PageStyles")->getByName("Standard"),
                                                  uno::UNO_QUERY);
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    SwFrame* pPage = pLayout->GetLower();
    long nOldLeft = pPage->getFramePrintArea().Left();

    // Set the gutter margin to 2cm.
    sal_Int32 nGutterMm100 = 2000;
    xStandard->setPropertyValue("GutterMargin", uno::makeAny(nGutterMm100));

    // Verify that the new left edge is larger.
    long nNewLeft = pPage->getFramePrintArea().Left();
    long nGutterTwips = convertMm100ToTwip(nGutterMm100);
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
    loadURL("private:factory/swriter", nullptr);
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xSettings(
        xFactory->createInstance("com.sun.star.document.Settings"), uno::UNO_QUERY);
    xSettings->setPropertyValue("GutterAtTop", uno::makeAny(true));
    uno::Reference<beans::XPropertySet> xStandard(getStyles("PageStyles")->getByName("Standard"),
                                                  uno::UNO_QUERY);
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    SwFrame* pPage = pLayout->GetLower();
    long nOldTop = pPage->getFramePrintArea().Top();

    // Set the gutter margin to 2cm.
    sal_Int32 nGutterMm100 = 2000;
    xStandard->setPropertyValue("GutterMargin", uno::makeAny(nGutterMm100));

    // Verify that the new top edge is larger.
    long nNewTop = pPage->getFramePrintArea().Top();
    long nGutterTwips = convertMm100ToTwip(nGutterMm100);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1134
    // - Actual  : 0
    // i.e. the gutter was not added to the left margin.
    CPPUNIT_ASSERT_EQUAL(nGutterTwips, nNewTop - nOldTop);
}

CPPUNIT_TEST_FIXTURE(SwCoreLayoutTest, testGutterMirrorMargin)
{
    loadURL("private:factory/swriter", nullptr);
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    SwDocShell* pDocShell = pDoc->GetDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->InsertPageBreak();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    SwFrame* pPage = pLayout->GetLower();
    long nOldLeft = pPage->getFramePrintArea().Left();
    SwFrame* pPage2 = pPage->GetNext();
    long nOldRight = pPage2->getFramePrintArea().Right();

    uno::Reference<beans::XPropertySet> xStandard(getStyles("PageStyles")->getByName("Standard"),
                                                  uno::UNO_QUERY);
    xStandard->setPropertyValue("PageStyleLayout", uno::makeAny(style::PageStyleLayout_MIRRORED));
    sal_Int32 nGutterMm100 = 2000;
    xStandard->setPropertyValue("GutterMargin", uno::makeAny(nGutterMm100));

    long nNewLeft = pPage->getFramePrintArea().Left();
    long nGutterTwips = convertMm100ToTwip(nGutterMm100);
    CPPUNIT_ASSERT_EQUAL(nGutterTwips, nNewLeft - nOldLeft);
    long nNewRight = pPage2->getFramePrintArea().Right();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1134
    // - Actual  : 0
    // i.e. the gutter was missing on the second, mirrored page.
    CPPUNIT_ASSERT_EQUAL(nGutterTwips, nOldRight - nNewRight);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
