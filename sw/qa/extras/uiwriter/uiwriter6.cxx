/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/drawing/FillStyle.hpp>
#include <swmodeltestbase.hxx>
#include <cntfrm.hxx>
#include <itabenum.hxx>
#include <ndtxt.hxx>
#include <wrtsh.hxx>
#include <drawdoc.hxx>
#include <view.hxx>
#include <com/sun/star/text/XTextColumns.hpp>

#include <svx/svdpage.hxx>
#include <svx/svdview.hxx>
#include <svl/itemiter.hxx>
#include <vcl/filter/PDFiumLibrary.hxx>

#include <dbfld.hxx>
#include <txatbase.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <UndoManager.hxx>
#include <unotools/syslocaleoptions.hxx>

#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <o3tl/cppunittraitshelper.hxx>
#include <swdtflvr.hxx>
#include <comphelper/propertysequence.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <vcl/scheduler.hxx>
#include <config_fonts.h>
#include <test/htmltesttools.hxx>
#include <wrthtml.hxx>
#include <dbmgr.hxx>
#include <rootfrm.hxx>
#include <unotxdoc.hxx>

namespace
{
constexpr OUStringLiteral DATA_DIRECTORY = u"/sw/qa/extras/uiwriter/data/";

sal_Int32 lcl_getAttributeIDFromHints(const SwpHints& hints)
{
    for (size_t i = 0; i < hints.Count(); ++i)
    {
        const SwTextAttr* hint = hints.Get(i);
        if (hint->Which() == RES_TXTATR_AUTOFMT)
        {
            const SwFormatAutoFormat& rFmt = hint->GetAutoFormat();
            SfxItemIter aIter(*rFmt.GetStyleHandle());
            return aIter.GetCurItem()->Which();
        }
    }
    return -1;
}

void emulateTyping(SwXTextDocument& rXTextDocument, const std::u16string_view& rStr)
{
    for (const char16_t c : rStr)
    {
        rXTextDocument.postKeyEvent(LOK_KEYEVENT_KEYINPUT, c, 0);
        rXTextDocument.postKeyEvent(LOK_KEYEVENT_KEYUP, c, 0);
        Scheduler::ProcessEventsToIdle();
    }
}
} //namespace

class SwUiWriterTest6 : public SwModelTestBase, public HtmlTestTools
{
};

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testTdf116640)
{
    createSwDoc();

    uno::Sequence<beans::PropertyValue> aArgs(
        comphelper::InitPropertySequence({ { "Columns", uno::Any(sal_Int32(2)) } }));

    dispatchCommand(mxComponent, ".uno:InsertSection", aArgs);
    Scheduler::ProcessEventsToIdle();

    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xSections(xTextSectionsSupplier->getTextSections(),
                                                      uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xTextSection(xSections->getByIndex(0), uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSections->getCount());

    uno::Reference<text::XTextColumns> xTextColumns
        = getProperty<uno::Reference<text::XTextColumns>>(xTextSection, "TextColumns");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), xTextColumns->getColumnCount());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xSections->getCount());

    dispatchCommand(mxComponent, ".uno:Redo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSections->getCount());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xSections->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testTdf108524)
{
    createSwDoc(DATA_DIRECTORY, "tdf108524.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // In total we expect two cells containing a section.
    assertXPath(pXmlDoc, "/root/page/body/tab/row/cell/section", 2);

    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row/cell/section", 1);
    // This was 0, section wasn't split, instead it was only on the first page
    // and it was cut off.
    assertXPath(pXmlDoc, "/root/page[2]/body/tab/row/cell/section", 1);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testLinesInSectionInTable)
{
    // This is similar to testTdf108524(), but the page boundary now is not in
    // the middle of a multi-line paragraph: the section only contains oneliner
    // paragraphs instead.
    createSwDoc(DATA_DIRECTORY, "lines-in-section-in-table.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // In total we expect two cells containing a section.
    assertXPath(pXmlDoc, "/root/page/body/tab/row/cell/section", 2);

    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row/cell/section", 1);
    // This was 0, section wasn't split, instead it was only on the first page
    // and it was cut off.
    assertXPath(pXmlDoc, "/root/page[2]/body/tab/row/cell/section", 1);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testLinesMoveBackwardsInSectionInTable)
{
#if HAVE_MORE_FONTS
    // Assert that paragraph "4" is on page 1 and "5" is on page 2.
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "lines-in-section-in-table.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page", 2);
    SwNodeOffset nPara4Node(
        getXPath(pXmlDoc, "/root/page[1]/body/tab/row/cell[1]/section/txt[last()]", "txtNodeIndex")
            .toUInt32());
    CPPUNIT_ASSERT_EQUAL(OUString("4"), pDoc->GetNodes()[nPara4Node]->GetTextNode()->GetText());
    SwNodeOffset nPara5Node(
        getXPath(pXmlDoc, "/root/page[2]/body/tab/row/cell[1]/section/txt[1]", "txtNodeIndex")
            .toUInt32());
    CPPUNIT_ASSERT_EQUAL(OUString("5"), pDoc->GetNodes()[nPara5Node]->GetTextNode()->GetText());

    // Remove paragraph "4".
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    while (pWrtShell->GetCursor()->GetNode().GetIndex() < nPara4Node)
        pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->EndPara();
    pWrtShell->Up(/*bSelect=*/true);
    pWrtShell->DelLeft();

    // Assert that paragraph "5" is now moved back to page 1 and is the last paragraph there.
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    SwNodeOffset nPage1LastNode(
        getXPath(pXmlDoc, "/root/page[1]/body/tab/row/cell[1]/section/txt[last()]", "txtNodeIndex")
            .toUInt32());
    // This was "3", paragraph "4" was deleted, but "5" was not moved backwards from page 2.
    CPPUNIT_ASSERT_EQUAL(OUString("5"), pDoc->GetNodes()[nPage1LastNode]->GetTextNode()->GetText());
#endif
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testTableInSection)
{
#if HAVE_MORE_FONTS
    // The document has a section, containing a table that spans over 2 pages.
    createSwDoc(DATA_DIRECTORY, "table-in-sect.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // In total we expect 4 cells.
    assertXPath(pXmlDoc, "/root/page/body/section/tab/row/cell", 4);

    // Assert that on both pages the section contains 2 cells.
    assertXPath(pXmlDoc, "/root/page[1]/body/section/tab/row/cell", 2);
    assertXPath(pXmlDoc, "/root/page[2]/body/section/tab/row/cell", 2);
#endif
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testTableInNestedSection)
{
#if HAVE_MORE_FONTS
    // The document has a nested section, containing a table that spans over 2 pages.
    // This crashed the layout.
    createSwDoc(DATA_DIRECTORY, "rhbz739252-3.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // Make sure the table is inside a section and spans over 2 pages.
    assertXPath(pXmlDoc, "//page[1]//section/tab", 1);
    assertXPath(pXmlDoc, "//page[2]//section/tab", 1);
#endif
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testTdf112741)
{
#if HAVE_MORE_FONTS
    createSwDoc(DATA_DIRECTORY, "tdf112741.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // This was 5 pages.
    assertXPath(pXmlDoc, "//page", 4);
    assertXPath(pXmlDoc, "//page[1]/body/tab/row/cell/tab/row/cell/section", 1);
    assertXPath(pXmlDoc, "//page[2]/body/tab/row/cell/tab/row/cell/section", 1);
    // This failed, 3rd page contained no sections.
    assertXPath(pXmlDoc, "//page[3]/body/tab/row/cell/tab/row/cell/section", 1);
    assertXPath(pXmlDoc, "//page[4]/body/tab/row/cell/tab/row/cell/section", 1);
#endif
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testTdf112860)
{
#if HAVE_MORE_FONTS
    // The document has a split section inside a nested table, and also a table
    // in the footer.
    // This crashed the layout.
    createSwDoc(DATA_DIRECTORY, "tdf112860.fodt");
#endif
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testTdf113287)
{
#if HAVE_MORE_FONTS
    createSwDoc(DATA_DIRECTORY, "tdf113287.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page", 2);
    sal_uInt32 nCellTop
        = getXPath(pXmlDoc, "//page[2]/body/tab/row/cell[1]/infos/bounds", "top").toUInt32();
    sal_uInt32 nSectionTop
        = getXPath(pXmlDoc, "//page[2]/body/tab/row/cell[1]/section/infos/bounds", "top")
              .toUInt32();
    // Make sure section frame is inside the cell frame.
    // Expected greater than 4593, was only 3714.
    CPPUNIT_ASSERT_GREATER(nCellTop, nSectionTop);
#endif
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testTdf113445)
{
#if HAVE_MORE_FONTS
    // Force multiple-page view.
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf113445.fodt");
    SwDocShell* pDocShell = pDoc->GetDocShell();
    SwView* pView = pDocShell->GetView();
    pView->SetViewLayout(/*nColumns=*/2, /*bBookMode=*/false);
    calcLayout();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page", 2);
    sal_uInt32 nPage1Left = getXPath(pXmlDoc, "//page[1]/infos/bounds", "left").toUInt32();
    sal_uInt32 nPage2Left = getXPath(pXmlDoc, "//page[2]/infos/bounds", "left").toUInt32();
    // Make sure that page 2 is on the right hand side of page 1, not below it.
    CPPUNIT_ASSERT_GREATER(nPage1Left, nPage2Left);

    // Insert a new paragraph at the start of the document.
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->StartOfSection();
    pWrtShell->SplitNode();
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();

    // Make sure that Table2:C5 and Table2:D5 has its section frame inside the cell frame.
    sal_uInt32 nCell3Top
        = getXPath(pXmlDoc, "//page[2]/body/tab/row/cell/tab/row[4]/cell[3]/infos/bounds", "top")
              .toUInt32();
    sal_uInt32 nSection3Top
        = getXPath(pXmlDoc, "//page[2]/body/tab/row/cell/tab/row[4]/cell[3]/section/infos/bounds",
                   "top")
              .toUInt32();
    CPPUNIT_ASSERT_GREATER(nCell3Top, nSection3Top);
    sal_uInt32 nCell4Top
        = getXPath(pXmlDoc, "//page[2]/body/tab/row/cell/tab/row[4]/cell[4]/infos/bounds", "top")
              .toUInt32();
    sal_uInt32 nSection4Top
        = getXPath(pXmlDoc, "//page[2]/body/tab/row/cell/tab/row[4]/cell[4]/section/infos/bounds",
                   "top")
              .toUInt32();
    CPPUNIT_ASSERT_GREATER(nCell4Top, nSection4Top);
    // Also check if the two cells in the same row have the same top position.
    // This was 4818, expected only 1672.
    CPPUNIT_ASSERT_EQUAL(nCell3Top, nCell4Top);
#endif
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testTdf113686)
{
#if HAVE_MORE_FONTS
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf113686.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page", 2);
    SwNodeOffset nPage1LastNode(
        getXPath(pXmlDoc, "/root/page[1]/body/tab/row/cell[1]/tab/row/cell[1]/txt[last()]",
                 "txtNodeIndex")
            .toUInt32());
    CPPUNIT_ASSERT_EQUAL(OUString("Table2:A1-P10"),
                         pDoc->GetNodes()[nPage1LastNode]->GetTextNode()->GetText());
    SwNodeOffset nPage2FirstNode(
        getXPath(pXmlDoc, "/root/page[2]/body/tab/row/cell[1]/section/txt[1]", "txtNodeIndex")
            .toUInt32());
    CPPUNIT_ASSERT_EQUAL(OUString("Table1:A1"),
                         pDoc->GetNodes()[nPage2FirstNode]->GetTextNode()->GetText());

    // Remove page 2.
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    while (pWrtShell->GetCursor()->Start()->nNode.GetIndex() < nPage1LastNode)
        pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->EndPara();
    for (int i = 0; i < 3; ++i)
        pWrtShell->Up(/*bSelect=*/true);
    pWrtShell->DelLeft();

    // Assert that the second page is removed.
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    // This was still 2, content from 2nd page was not moved.
    assertXPath(pXmlDoc, "/root/page", 1);
#endif
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testTableInSectionInTable)
{
#if HAVE_MORE_FONTS
    // The document has a table, containing a section, containing a nested
    // table.
    // This crashed the layout.
    createSwDoc(DATA_DIRECTORY, "i95698.odt");
#endif
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testSectionInTableInTable)
{
#if HAVE_MORE_FONTS
    // The document has a nested table, containing a multi-line section at a
    // page boundary.
    // This crashed the layout later in SwFrame::IsFootnoteAllowed().
    createSwDoc(DATA_DIRECTORY, "tdf112109.fodt");
#endif
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testSectionInTableInTable2)
{
#if HAVE_MORE_FONTS
    createSwDoc(DATA_DIRECTORY, "split-section-in-nested-table.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_uInt32 nSection1
        = getXPath(pXmlDoc, "//page[1]//body/tab/row/cell/tab/row/cell/section", "id").toUInt32();
    sal_uInt32 nSection1Follow
        = getXPath(pXmlDoc, "//page[1]//body/tab/row/cell/tab/row/cell/section", "follow")
              .toUInt32();
    // This failed, the section wasn't split inside a nested table.
    sal_uInt32 nSection2
        = getXPath(pXmlDoc, "//page[2]//body/tab/row/cell/tab/row/cell/section", "id").toUInt32();
    sal_uInt32 nSection2Precede
        = getXPath(pXmlDoc, "//page[2]//body/tab/row/cell/tab/row/cell/section", "precede")
              .toUInt32();

    // Make sure that the first's follow and the second's precede is correct.
    CPPUNIT_ASSERT_EQUAL(nSection2, nSection1Follow);
    CPPUNIT_ASSERT_EQUAL(nSection1, nSection2Precede);
#endif
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testSectionInTableInTable3)
{
#if HAVE_MORE_FONTS
    createSwDoc(DATA_DIRECTORY, "tdf113153.fodt");

    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<container::XNamed> xTable(xTables->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Table16"), xTable->getName());

    uno::Reference<text::XTextTable> xRowSupplier(xTable, uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xRows = xRowSupplier->getRows();
    uno::Reference<beans::XPropertySet> xRow(xRows->getByIndex(1), uno::UNO_QUERY);
    xRow->setPropertyValue("IsSplitAllowed", uno::Any(true));
    // This never returned.
    calcLayout();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_uInt32 nTable1 = getXPath(pXmlDoc, "//page[1]//body/tab", "id").toUInt32();
    sal_uInt32 nTable1Follow = getXPath(pXmlDoc, "//page[1]//body/tab", "follow").toUInt32();
    sal_uInt32 nTable2 = getXPath(pXmlDoc, "//page[2]//body/tab", "id").toUInt32();
    sal_uInt32 nTable2Precede = getXPath(pXmlDoc, "//page[2]//body/tab", "precede").toUInt32();
    sal_uInt32 nTable2Follow = getXPath(pXmlDoc, "//page[2]//body/tab", "follow").toUInt32();
    sal_uInt32 nTable3 = getXPath(pXmlDoc, "//page[3]//body/tab", "id").toUInt32();
    sal_uInt32 nTable3Precede = getXPath(pXmlDoc, "//page[3]//body/tab", "precede").toUInt32();

    // Make sure the outer table frames are linked together properly.
    CPPUNIT_ASSERT_EQUAL(nTable2, nTable1Follow);
    CPPUNIT_ASSERT_EQUAL(nTable1, nTable2Precede);
    CPPUNIT_ASSERT_EQUAL(nTable3, nTable2Follow);
    CPPUNIT_ASSERT_EQUAL(nTable2, nTable3Precede);
#endif
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testSectionInTableInTable4)
{
#if HAVE_MORE_FONTS
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf113520.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page", 3);
    SwNodeOffset nPage1LastNode(
        getXPath(pXmlDoc, "/root/page[1]/body/tab/row/cell[1]/tab/row/cell[1]/section/txt[last()]",
                 "txtNodeIndex")
            .toUInt32());
    CPPUNIT_ASSERT_EQUAL(OUString("Section1:P10"),
                         pDoc->GetNodes()[nPage1LastNode]->GetTextNode()->GetText());
    SwNodeOffset nPage3FirstNode(
        getXPath(pXmlDoc, "/root/page[3]/body/tab/row/cell[1]/tab/row/cell[1]/section/txt[1]",
                 "txtNodeIndex")
            .toUInt32());
    CPPUNIT_ASSERT_EQUAL(OUString("Section1:P23"),
                         pDoc->GetNodes()[nPage3FirstNode]->GetTextNode()->GetText());

    // Remove page 2.
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    while (pWrtShell->GetCursor()->Start()->nNode.GetIndex() < nPage1LastNode)
        pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->EndPara();
    while (pWrtShell->GetCursor()->End()->nNode.GetIndex() < nPage3FirstNode)
        pWrtShell->Down(/*bSelect=*/true);
    pWrtShell->EndPara(/*bSelect=*/true);
    pWrtShell->DelLeft();

    // Assert that the page is removed.
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    // This was 3, page 2 was emptied, but it wasn't removed.
    assertXPath(pXmlDoc, "/root/page", 2);

    // Make sure the outer table frames are linked together properly.
    sal_uInt32 nTable1 = getXPath(pXmlDoc, "//page[1]//body/tab", "id").toUInt32();
    sal_uInt32 nTable1Follow = getXPath(pXmlDoc, "//page[1]//body/tab", "follow").toUInt32();
    sal_uInt32 nTable2 = getXPath(pXmlDoc, "//page[2]//body/tab", "id").toUInt32();
    sal_uInt32 nTable2Precede = getXPath(pXmlDoc, "//page[2]//body/tab", "precede").toUInt32();
    CPPUNIT_ASSERT_EQUAL(nTable2, nTable1Follow);
    CPPUNIT_ASSERT_EQUAL(nTable1, nTable2Precede);
#endif
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testTdf112160)
{
#if HAVE_MORE_FONTS
    // Assert that the A2 cell is on page 1.
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf112160.fodt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    SwNodeOffset nA2CellNode(getXPath(pXmlDoc,
                                      "/root/page[1]/body/tab/row[2]/cell[1]/section/txt[last()]",
                                      "txtNodeIndex")
                                 .toUInt32());
    CPPUNIT_ASSERT_EQUAL(OUString("Table1.A2"),
                         pDoc->GetNodes()[nA2CellNode]->GetTextNode()->GetText());

    // Append a new paragraph to the end of the A2 cell.
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    while (pWrtShell->GetCursor()->GetNode().GetIndex() < nA2CellNode)
        pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->EndPara();
    pWrtShell->SplitNode();

    // Assert that after A2 got extended, D2 stays on page 1.
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    sal_uInt32 nD2CellNode
        = getXPath(pXmlDoc, "/root/page[1]/body/tab/row[2]/cell[last()]/section/txt[last()]",
                   "txtNodeIndex")
              .toUInt32();
    // This was Table1.C2, Table1.D2 was moved to the next page, unexpected.
    CPPUNIT_ASSERT_EQUAL(OUString("Table1.D2"),
                         pDoc->GetNodes()[SwNodeOffset(nD2CellNode)]->GetTextNode()->GetText());
#endif
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testTdf114536)
{
    // This crashed in SwTextFormatter::MergeCharacterBorder() due to a
    // use after free.
    createSwDoc(DATA_DIRECTORY, "tdf114536.odt");
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testParagraphOfTextRange)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "paragraph-of-text-range.odt");

    // Enter the table.
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Down(/*bSelect=*/false);
    CPPUNIT_ASSERT(pWrtShell->IsCursorInTable());
    // Enter the section.
    pWrtShell->Down(/*bSelect=*/false);
    CPPUNIT_ASSERT(pWrtShell->IsDirectlyInSection());

    // Assert that we get the right paragraph object.
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xController(xModel->getCurrentController(),
                                                              uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xViewCursor = xController->getViewCursor();
    // This failed as there were no TextParagraph property.
    auto xParagraph
        = getProperty<uno::Reference<text::XTextRange>>(xViewCursor->getStart(), "TextParagraph");
    CPPUNIT_ASSERT_EQUAL(OUString("In section"), xParagraph->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testTdf99689TableOfContents)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf99689.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->GotoNextTOXBase();
    const SwTOXBase* pTOXBase = pWrtShell->GetCurTOX();
    pWrtShell->UpdateTableOf(*pTOXBase);
    SwCursorShell* pShell(pDoc->GetEditShell());
    SwTextNode* pTitleNode = pShell->GetCursor()->GetNode().GetTextNode();
    SwNodeIndex aIdx(*pTitleNode);
    // skip the title
    pDoc->GetNodes().GoNext(&aIdx);

    // skip the first header. No attributes there.
    // next node should contain superscript
    SwTextNode* pNext = static_cast<SwTextNode*>(pDoc->GetNodes().GoNext(&aIdx));
    CPPUNIT_ASSERT(pNext->HasHints());
    sal_uInt16 nAttrType = lcl_getAttributeIDFromHints(pNext->GetSwpHints());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(RES_CHRATR_ESCAPEMENT), nAttrType);

    // next node should contain subscript
    pNext = static_cast<SwTextNode*>(pDoc->GetNodes().GoNext(&aIdx));
    CPPUNIT_ASSERT(pNext->HasHints());
    nAttrType = lcl_getAttributeIDFromHints(pNext->GetSwpHints());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(RES_CHRATR_ESCAPEMENT), nAttrType);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testTdf99689TableOfFigures)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf99689_figures.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->GotoNextTOXBase();
    const SwTOXBase* pTOXBase = pWrtShell->GetCurTOX();
    pWrtShell->UpdateTableOf(*pTOXBase);
    SwCursorShell* pShell(pDoc->GetEditShell());
    SwTextNode* pTitleNode = pShell->GetCursor()->GetNode().GetTextNode();
    SwNodeIndex aIdx(*pTitleNode);

    // skip the title
    // next node should contain subscript
    SwTextNode* pNext = static_cast<SwTextNode*>(pDoc->GetNodes().GoNext(&aIdx));
    CPPUNIT_ASSERT(pNext->HasHints());
    sal_uInt16 nAttrType = lcl_getAttributeIDFromHints(pNext->GetSwpHints());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(RES_CHRATR_ESCAPEMENT), nAttrType);

    // next node should contain superscript
    pNext = static_cast<SwTextNode*>(pDoc->GetNodes().GoNext(&aIdx));
    CPPUNIT_ASSERT(pNext->HasHints());
    nAttrType = lcl_getAttributeIDFromHints(pNext->GetSwpHints());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(RES_CHRATR_ESCAPEMENT), nAttrType);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testTdf99689TableOfTables)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf99689_tables.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->GotoNextTOXBase();
    const SwTOXBase* pTOXBase = pWrtShell->GetCurTOX();
    pWrtShell->UpdateTableOf(*pTOXBase);
    SwCursorShell* pShell(pDoc->GetEditShell());
    SwTextNode* pTitleNode = pShell->GetCursor()->GetNode().GetTextNode();
    SwNodeIndex aIdx(*pTitleNode);

    // skip the title
    // next node should contain superscript
    SwTextNode* pNext = static_cast<SwTextNode*>(pDoc->GetNodes().GoNext(&aIdx));
    CPPUNIT_ASSERT(pNext->HasHints());
    sal_uInt16 nAttrType = lcl_getAttributeIDFromHints(pNext->GetSwpHints());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(RES_CHRATR_ESCAPEMENT), nAttrType);

    // next node should contain subscript
    pNext = static_cast<SwTextNode*>(pDoc->GetNodes().GoNext(&aIdx));
    CPPUNIT_ASSERT(pNext->HasHints());
    nAttrType = lcl_getAttributeIDFromHints(pNext->GetSwpHints());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(RES_CHRATR_ESCAPEMENT), nAttrType);
}

// tdf#112448: Fix: take correct line height
//
// When line metrics is not calculated we need to call CalcRealHeight()
// before usage of the Height() and GetRealHeight().
CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testTdf112448)
{
    createSwDoc(DATA_DIRECTORY, "tdf112448.odt");

    // check actual number of line breaks in the paragraph
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page/body/txt/LineBreak", 2);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testTdf113790)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf113790.docx");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    // Create the clipboard document.
    SwDoc aClipboard;
    aClipboard.SetClipBoard(true);

    // Go to fourth line - to "ABCD" bulleted list item
    pWrtShell->Down(/*bSelect=*/false, 4);
    pWrtShell->SelPara(nullptr);
    CPPUNIT_ASSERT_EQUAL(OUString("ABCD"), pWrtShell->GetSelText());
    pWrtShell->Copy(aClipboard);

    // Go down to next-to-last (empty) line above "Title3"
    pWrtShell->Down(/*bSelect=*/false, 4);
    pWrtShell->Paste(aClipboard);

    // Save it as DOCX & load it again
    reload("Office Open XML Text", "tdf113790.docx");
    CPPUNIT_ASSERT(dynamic_cast<SwXTextDocument*>(mxComponent.get()));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testTdf108048)
{
    createSwDoc();

    uno::Sequence<beans::PropertyValue> aPropertyValues = comphelper::InitPropertySequence({
        { "Kind", uno::Any(sal_Int16(3)) },
        { "TemplateName", uno::Any(OUString("Default Page Style")) },
        { "PageNumber", uno::Any(sal_uInt16(6)) }, // Even number to avoid auto-inserted blank page
        { "PageNumberFilled", uno::Any(true) },
    });
    dispatchCommand(mxComponent, ".uno:InsertBreak", aPropertyValues);
    CPPUNIT_ASSERT_EQUAL(2, getParagraphs());
    CPPUNIT_ASSERT_EQUAL(2, getPages());

    // The inserted page must have page number set to 6
    uno::Reference<text::XTextRange> xPara = getParagraph(2);
    sal_uInt16 nPageNumber = getProperty<sal_uInt16>(xPara, "PageNumberOffset");
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(6), nPageNumber);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testTdf113481)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf113481-IVS.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    // One backspace should completely remove the CJK ideograph variation sequence
    pWrtShell->EndPara();
    // Before: U+8FBA U+E0102. After: empty
    pWrtShell->DelLeft();
    const uno::Reference<text::XTextRange> xPara1 = getParagraph(1);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xPara1->getString().getLength());

    // In case that weak script is treated as CJK script, remove one character.
    pWrtShell->Down(false);
    pWrtShell->EndPara();
    // Before: U+4E2D U+2205 U+FE00. After: U+4E2D U+2205
    if (pWrtShell->GetScriptType() == SvtScriptType::ASIAN)
    {
        pWrtShell->DelLeft();
        const uno::Reference<text::XTextRange> xPara2 = getParagraph(2);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xPara2->getString().getLength());
        CPPUNIT_ASSERT_EQUAL(u'\x2205', xPara2->getString()[1]);
    }

    // Characters of other scripts, remove one character.
    pWrtShell->Down(false);
    pWrtShell->EndPara();
    // Before: U+1820 U+180B. After: U+1820
    pWrtShell->DelLeft();
    const uno::Reference<text::XTextRange> xPara3 = getParagraph(3);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xPara3->getString().getLength());
    CPPUNIT_ASSERT_EQUAL(u'\x1820', xPara3->getString()[0]);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testTdf115013)
{
    const OUString sColumnName("Name with spaces, \"quotes\" and \\backslashes");

    utl::TempFile aTempDir(nullptr, true);
    aTempDir.EnableKillingFile();
    const OUString aWorkDir = aTempDir.GetURL();

    //create new writer document
    SwDoc* pDoc = createSwDoc();

    {
        // Load and register data source
        const OUString aDataSourceURI(m_directories.getURLFromSrc(DATA_DIRECTORY)
                                      + "datasource.ods");
        OUString sDataSource = SwDBManager::LoadAndRegisterDataSource(aDataSourceURI, &aWorkDir);
        CPPUNIT_ASSERT(!sDataSource.isEmpty());

        // Insert a new field type for the mailmerge field
        SwDBData aDBData;
        aDBData.sDataSource = sDataSource;
        aDBData.sCommand = "Sheet1";
        SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
        CPPUNIT_ASSERT(pWrtShell);
        SwDBFieldType* pFieldType = static_cast<SwDBFieldType*>(
            pWrtShell->InsertFieldType(SwDBFieldType(pDoc, sColumnName, aDBData)));
        CPPUNIT_ASSERT(pFieldType);

        // Insert the field into document
        SwDBField aField(pFieldType);
        pWrtShell->InsertField2(aField);
    }
    // Save it as DOCX & load it again
    reload("Office Open XML Text", "mm-field.docx");

    auto pXTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXTextDocument);
    pDoc = pXTextDocument->GetDocShell()->GetDoc();
    CPPUNIT_ASSERT(pDoc);
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);
    SwPaM* pCursor = pDoc->GetEditShell()->GetCursor();
    CPPUNIT_ASSERT(pCursor);

    // Get the field at the beginning of the document
    SwDBField* pField = dynamic_cast<SwDBField*>(SwCursorShell::GetFieldAtCursor(pCursor, true));
    CPPUNIT_ASSERT(pField);
    OUString sColumn = static_cast<SwDBFieldType*>(pField->GetTyp())->GetColumnName();
    // The column name must come correct after round trip
    CPPUNIT_ASSERT_EQUAL(sColumnName, sColumn);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testTdf115065)
{
    // In the document, the tables have table style assigned
    // Source table (first one) has two rows;
    // destination (second one) has only one row
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf115065.odt");
    CPPUNIT_ASSERT(pDoc);
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    pWrtShell->GotoTable("Table2");
    SwRect aRect = pWrtShell->GetCurrFrame()->getFrameArea();
    // Destination point is the middle of the first cell of second table
    Point ptTo(aRect.Left() + aRect.Width() / 2, aRect.Top() + aRect.Height() / 2);

    pWrtShell->GotoTable("Table1");
    aRect = pWrtShell->GetCurrFrame()->getFrameArea();
    // Source point is the middle of the first cell of first table
    Point ptFrom(aRect.Left() + aRect.Width() / 2, aRect.Top() + aRect.Height() / 2);

    pWrtShell->SelTableCol();
    // The copy operation (or closing document after that) segfaulted
    pWrtShell->Copy(*pWrtShell, ptFrom, ptTo);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testTdf84806_MovingMultipleTableRows)
{
    // Moving of multiple table rows.
    // Source table (first one) has two rows;
    // destination (second one) has only one row
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf115065.odt");
    CPPUNIT_ASSERT(pDoc);
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTables->getCount());
    uno::Reference<container::XNameAccess> xTableNames = xTablesSupplier->getTextTables();
    CPPUNIT_ASSERT(xTableNames->hasByName("Table1"));
    CPPUNIT_ASSERT(xTableNames->hasByName("Table2"));
    uno::Reference<text::XTextTable> xTable1(xTableNames->getByName("Table1"), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable2(xTableNames->getByName("Table2"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable1->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable2->getRows()->getCount());

    // without redlining
    CPPUNIT_ASSERT_MESSAGE("redlining should be off",
                           !pDoc->getIDocumentRedlineAccess().IsRedlineOn());

    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();

    pWrtShell->GotoTable("Table2");
    SwRect aRect = pWrtShell->GetCurrFrame()->getFrameArea();
    // Destination point is the middle of the first cell of second table
    Point ptTo(aRect.Left() + aRect.Width() / 2, aRect.Top() + aRect.Height() / 2);

    // Move rows of the first table into the second table
    pWrtShell->GotoTable("Table1");
    pWrtShell->SelTable();
    rtl::Reference<SwTransferable> xTransfer = new SwTransferable(*pWrtShell);
    xTransfer->PrivateDrop(*pWrtShell, ptTo, /*bMove=*/true, /*bXSelection=*/true);

    // This was 2 tables
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTable2->getRows()->getCount());

    // Undo results 2 tables
    rUndoManager.Undo();
    uno::Reference<container::XIndexAccess> xTables2(xTablesSupplier->getTextTables(),
                                                     uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTables2->getCount());
    uno::Reference<text::XTextTable> xTable1b(xTableNames->getByName("Table1"), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable2b(xTableNames->getByName("Table2"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable1b->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable2b->getRows()->getCount());

    // FIXME assert with Redo()
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testTdf147181_TrackedMovingOfMultipleTableRows)
{
    // Tracked moving of multiple table rows.
    // Source table (first one) has two rows;
    // destination (second one) has only one row
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf115065.odt");
    CPPUNIT_ASSERT(pDoc);
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTables->getCount());
    uno::Reference<container::XNameAccess> xTableNames = xTablesSupplier->getTextTables();
    CPPUNIT_ASSERT(xTableNames->hasByName("Table1"));
    CPPUNIT_ASSERT(xTableNames->hasByName("Table2"));
    uno::Reference<text::XTextTable> xTable1(xTableNames->getByName("Table1"), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable2(xTableNames->getByName("Table2"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable1->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable2->getRows()->getCount());

    // FIXME: doesn't work with empty rows, yet
    pWrtShell->Insert("x");
    pWrtShell->Down(false);
    pWrtShell->Insert("x");

    // enable redlining
    dispatchCommand(mxComponent, ".uno:TrackChanges", {});
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());

    // show changes
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();

    pWrtShell->GotoTable("Table2");
    SwRect aRect = pWrtShell->GetCurrFrame()->getFrameArea();
    // Destination point is the middle of the first cell of second table
    Point ptTo(aRect.Left() + aRect.Width() / 2, aRect.Top() + aRect.Height() / 2);

    // Move rows of the first table into the second table
    pWrtShell->GotoTable("Table1");
    pWrtShell->SelTable();
    rtl::Reference<SwTransferable> xTransfer = new SwTransferable(*pWrtShell);
    xTransfer->PrivateDrop(*pWrtShell, ptTo, /*bMove=*/true, /*bXSelection=*/true);

    // still 2 tables, but the second one has got 3 rows
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable1->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTable2->getRows()->getCount());

    // accept changes results 1 table (removing moved table)
    dispatchCommand(mxComponent, ".uno:AcceptAllTrackedChanges", {});
    Scheduler::ProcessEventsToIdle();
    uno::Reference<container::XIndexAccess> xTables2(xTablesSupplier->getTextTables(),
                                                     uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables2->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTable2->getRows()->getCount());

    // Undo results 2 tables
    rUndoManager.Undo();
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTables2->getCount());
    uno::Reference<text::XTextTable> xTable1b(xTableNames->getByName("Table1"), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable2b(xTableNames->getByName("Table2"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable1b->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable2b->getRows()->getCount());

    // reject changes results 2 table again, with the original row counts
    dispatchCommand(mxComponent, ".uno:RejectAllTrackedChanges", {});
    uno::Reference<container::XIndexAccess> xTables3(xTablesSupplier->getTextTables(),
                                                     uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTables3->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable1b->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable2b->getRows()->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testTdf115132)
{
    SwDoc* pDoc = createSwDoc();
    CPPUNIT_ASSERT(pDoc);
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    std::vector<OUString> vTestTableNames;

    // Create an empty paragraph that will separate first table from the rest
    pWrtShell->SplitNode();
    pWrtShell->StartOfSection();
    // Create a table at the start of document body
    SwInsertTableOptions TableOpt(SwInsertTableFlags::DefaultBorder, 0);
    const SwTable* pTable = &pWrtShell->InsertTable(TableOpt, 2, 3);
    const SwTableFormat* pFormat = pTable->GetFrameFormat();
    CPPUNIT_ASSERT(pFormat);
    vTestTableNames.push_back(pFormat->GetName());
    pWrtShell->EndOfSection();
    // Create a table after a paragraph
    pTable = &pWrtShell->InsertTable(TableOpt, 2, 3);
    pFormat = pTable->GetFrameFormat();
    CPPUNIT_ASSERT(pFormat);
    vTestTableNames.push_back(pFormat->GetName());
    // Create a table immediately after the previous
    pTable = &pWrtShell->InsertTable(TableOpt, 2, 3);
    pFormat = pTable->GetFrameFormat();
    CPPUNIT_ASSERT(pFormat);
    vTestTableNames.push_back(pFormat->GetName());
    // Create a nested table in the middle of last row
    pWrtShell->GotoTable(vTestTableNames.back());
    for (int i = 0; i < 4; ++i)
        pWrtShell->GoNextCell(false);
    pTable = &pWrtShell->InsertTable(TableOpt, 2, 3);
    pFormat = pTable->GetFrameFormat();
    CPPUNIT_ASSERT(pFormat);
    vTestTableNames.push_back(pFormat->GetName());

    // Now check that in any cell in all tables we don't go out of a cell
    // using Delete or Backspace. We test cases when a table is the first node;
    // when we are in a first/middle/last cell in a row; when there's a paragraph
    // before/after this cell; when there's another table before/after this cell;
    // in nested table.
    for (const auto& rTableName : vTestTableNames)
    {
        pWrtShell->GotoTable(rTableName);
        do
        {
            const SwStartNode* pNd = pWrtShell->GetCursor()->GetNode().FindTableBoxStartNode();
            pWrtShell->DelRight();
            CPPUNIT_ASSERT_EQUAL(pNd, pWrtShell->GetCursor()->GetNode().FindTableBoxStartNode());
            pWrtShell->DelLeft();
            CPPUNIT_ASSERT_EQUAL(pNd, pWrtShell->GetCursor()->GetNode().FindTableBoxStartNode());
        } while (pWrtShell->GoNextCell(false));
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testXDrawPagesSupplier)
{
    createSwDoc();
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("XDrawPagesSupplier interface is unavailable", xDrawPagesSupplier.is());
    uno::Reference<drawing::XDrawPages> xDrawPages = xDrawPagesSupplier->getDrawPages();
    CPPUNIT_ASSERT(xDrawPages.is());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("There must be only a single DrawPage in Writer documents",
                                 sal_Int32(1), xDrawPages->getCount());
    uno::Any aDrawPage = xDrawPages->getByIndex(0);
    uno::Reference<drawing::XDrawPage> xDrawPageFromXDrawPages(aDrawPage, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDrawPageFromXDrawPages.is());

    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "The DrawPage accessed using XDrawPages must be the same as using XDrawPageSupplier",
        xDrawPage.get(), xDrawPageFromXDrawPages.get());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testTdf116403)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf116403-considerborders.odt");
    // Check that before ToX update, the tab stop position is the old one
    uno::Reference<text::XTextRange> xParagraph = getParagraph(2, "1\t1");
    auto aTabs = getProperty<uno::Sequence<style::TabStop>>(xParagraph, "ParaTabStops");
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), aTabs.getLength());
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(17000), aTabs[0].Position);

    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    const SwTOXBase* pTOX = pWrtShell->GetTOX(0);
    CPPUNIT_ASSERT(pTOX);
    pWrtShell->UpdateTableOf(*pTOX);

    xParagraph = getParagraph(2, "1\t1");
    aTabs = getProperty<uno::Sequence<style::TabStop>>(xParagraph, "ParaTabStops");
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), aTabs.getLength());
    // This was still 17000, refreshing ToX didn't take borders spacings and widths into account
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Page borders must be considered for right-aligned tabstop",
                                 static_cast<sal_Int32>(17000 - 2 * 500 - 2 * 1),
                                 aTabs[0].Position);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testHtmlCopyImages)
{
    // Load a document with an image.
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "image.odt");

    // Trigger the copy part of HTML copy&paste.
    WriterRef xWrt = new SwHTMLWriter(/*rBaseURL=*/OUString());
    CPPUNIT_ASSERT(xWrt.is());

    xWrt->m_bWriteClipboardDoc = true;
    xWrt->m_bWriteOnlyFirstTable = false;
    xWrt->SetShowProgress(false);
    {
        SvFileStream aStream(maTempFile.GetURL(), StreamMode::WRITE | StreamMode::TRUNC);
        SwWriter aWrt(aStream, *pDoc);
        aWrt.Write(xWrt);
    }
    htmlDocUniquePtr pHtmlDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pHtmlDoc);

    // This failed, image was lost during HTML copy.
    OUString aImage = getXPath(pHtmlDoc, "/html/body/p/img", "src");
    // Also make sure that the image is not embedded (e.g. Word doesn't handle
    // embedded images).
    CPPUNIT_ASSERT(aImage.startsWith("file:///"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testTdf116789)
{
    createSwDoc(DATA_DIRECTORY, "tdf116789.fodt");
    uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText1;
    uno::Reference<text::XText> xText2;
    {
        uno::Reference<text::XTextContent> xBookmark(
            xBookmarksSupplier->getBookmarks()->getByName("Bookmark 1"), uno::UNO_QUERY);
        xText1 = xBookmark->getAnchor()->getText();
    }
    {
        uno::Reference<text::XTextContent> xBookmark(
            xBookmarksSupplier->getBookmarks()->getByName("Bookmark 1"), uno::UNO_QUERY);
        xText2 = xBookmark->getAnchor()->getText();
    }
    // This failed, we got two different SwXCell for the same bookmark anchor text.
    CPPUNIT_ASSERT_EQUAL(xText1, xText2);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testTdf91801)
{
    // Tests calculation with several user field variables without prior user fields
    createSwDoc(DATA_DIRECTORY, "tdf91801.fodt");
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);
    uno::Reference<table::XCell> xCell(xTable->getCellByName("A1"));
    CPPUNIT_ASSERT_EQUAL(555.0, xCell->getValue());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testTdf51223)
{
    SwDoc* pDoc = createSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    SwNodeOffset nIndex = pWrtShell->GetCursor()->GetNode().GetIndex();
    pWrtShell->Insert("i");
    pWrtShell->SplitNode(true);
    CPPUNIT_ASSERT_EQUAL(OUString("I"),
                         static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex])->GetText());
    rUndoManager.Undo();
    CPPUNIT_ASSERT_EQUAL(OUString("i"),
                         static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex])->GetText());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testFontEmbedding)
{
#if HAVE_MORE_FONTS && !defined(MACOSX)
    createSwDoc(DATA_DIRECTORY, "testFontEmbedding.odt");

    OString aContentBaseXpath("/office:document-content/office:font-face-decls");
    OString aSettingsBaseXpath("/office:document-settings/office:settings/config:config-item-set");

    xmlDocUniquePtr pXmlDoc;
    uno::Sequence<beans::PropertyValue> aDescriptor;
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();

    // Get document settings
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xProps(
        xFactory->createInstance("com.sun.star.document.Settings"), uno::UNO_QUERY_THROW);

    // Check font embedding state
    CPPUNIT_ASSERT_EQUAL(false, xProps->getPropertyValue("EmbedFonts").get<bool>());
    CPPUNIT_ASSERT_EQUAL(false, xProps->getPropertyValue("EmbedOnlyUsedFonts").get<bool>());
    // Font scripts should be enabled by default, however this has no effect unless "EmbedOnlyUsedFonts" is enabled
    CPPUNIT_ASSERT_EQUAL(true, xProps->getPropertyValue("EmbedLatinScriptFonts").get<bool>());
    CPPUNIT_ASSERT_EQUAL(true, xProps->getPropertyValue("EmbedAsianScriptFonts").get<bool>());
    CPPUNIT_ASSERT_EQUAL(true, xProps->getPropertyValue("EmbedComplexScriptFonts").get<bool>());

    // CASE 1 - no font embedding enabled

    // Save the document
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    xStorable->storeToURL(aTempFile.GetURL(), aDescriptor);
    CPPUNIT_ASSERT(aTempFile.IsValid());

    // Check setting - No font embedding should be enabled
    pXmlDoc = parseExportInternal(aTempFile.GetURL(), "settings.xml");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPathContent(
        pXmlDoc, aSettingsBaseXpath + "/config:config-item[@config:name='EmbedFonts']", "false");

    // Check content - No font-face-src nodes should be present
    pXmlDoc = parseExportInternal(aTempFile.GetURL(), "content.xml");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, aContentBaseXpath + "/style:font-face", 6);
    assertXPath(pXmlDoc, aContentBaseXpath + "/style:font-face[@style:name='Liberation Sans']");
    assertXPath(
        pXmlDoc,
        aContentBaseXpath + "/style:font-face[@style:name='Liberation Sans']/svg:font-face-src", 0);
    assertXPath(pXmlDoc, aContentBaseXpath + "/style:font-face[@style:name='Liberation Sans1']");
    assertXPath(pXmlDoc,
                aContentBaseXpath
                    + "/style:font-face[@style:name='Liberation Sans1']/svg:font-face-src",
                0);
    assertXPath(pXmlDoc, aContentBaseXpath + "/style:font-face[@style:name='Liberation Serif']");
    assertXPath(pXmlDoc,
                aContentBaseXpath
                    + "/style:font-face[@style:name='Liberation Serif']/svg:font-face-src",
                0);
    assertXPath(pXmlDoc, aContentBaseXpath + "/style:font-face[@style:name='Liberation Serif1']");
    assertXPath(pXmlDoc,
                aContentBaseXpath
                    + "/style:font-face[@style:name='Liberation Serif1']/svg:font-face-src",
                0);
    assertXPath(pXmlDoc, aContentBaseXpath + "/style:font-face[@style:name='Carlito']");
    assertXPath(pXmlDoc,
                aContentBaseXpath + "/style:font-face[@style:name='Carlito']/svg:font-face-src", 0);
    assertXPath(pXmlDoc, aContentBaseXpath + "/style:font-face[@style:name='Caladea']");
    assertXPath(pXmlDoc,
                aContentBaseXpath + "/style:font-face[@style:name='Caladea']/svg:font-face-src", 0);

    // CASE 2 - font embedding enabled, but embed used fonts disabled

    // Enable font embedding, disable embedding used font only
    xProps->setPropertyValue("EmbedFonts", uno::Any(true));
    xProps->setPropertyValue("EmbedOnlyUsedFonts", uno::Any(false));

    // Save the document again
    xStorable->storeToURL(aTempFile.GetURL(), aDescriptor);
    CPPUNIT_ASSERT(aTempFile.IsValid());

    // Check setting - font embedding should be enabled + embed only used fonts and scripts
    pXmlDoc = parseExportInternal(aTempFile.GetURL(), "settings.xml");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPathContent(
        pXmlDoc, aSettingsBaseXpath + "/config:config-item[@config:name='EmbedFonts']", "true");
    assertXPathContent(
        pXmlDoc, aSettingsBaseXpath + "/config:config-item[@config:name='EmbedOnlyUsedFonts']",
        "false");
    assertXPathContent(
        pXmlDoc, aSettingsBaseXpath + "/config:config-item[@config:name='EmbedLatinScriptFonts']",
        "true");
    assertXPathContent(
        pXmlDoc, aSettingsBaseXpath + "/config:config-item[@config:name='EmbedAsianScriptFonts']",
        "true");
    assertXPathContent(
        pXmlDoc, aSettingsBaseXpath + "/config:config-item[@config:name='EmbedComplexScriptFonts']",
        "true");

    // Check content - font-face-src should be present only for "Liberation Sans" fonts

    pXmlDoc = parseExportInternal(aTempFile.GetURL(), "content.xml");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, aContentBaseXpath + "/style:font-face", 6);
    assertXPath(pXmlDoc, aContentBaseXpath + "/style:font-face[@style:name='Liberation Sans']");
    assertXPath(
        pXmlDoc,
        aContentBaseXpath + "/style:font-face[@style:name='Liberation Sans']/svg:font-face-src", 1);
    assertXPath(pXmlDoc, aContentBaseXpath + "/style:font-face[@style:name='Liberation Sans1']");
    assertXPath(pXmlDoc,
                aContentBaseXpath
                    + "/style:font-face[@style:name='Liberation Sans1']/svg:font-face-src",
                1);
    assertXPath(pXmlDoc, aContentBaseXpath + "/style:font-face[@style:name='Liberation Serif']");
    assertXPath(pXmlDoc,
                aContentBaseXpath
                    + "/style:font-face[@style:name='Liberation Serif']/svg:font-face-src",
                1);
    assertXPath(pXmlDoc, aContentBaseXpath + "/style:font-face[@style:name='Liberation Serif1']");
    assertXPath(pXmlDoc,
                aContentBaseXpath
                    + "/style:font-face[@style:name='Liberation Serif1']/svg:font-face-src",
                1);
    assertXPath(pXmlDoc, aContentBaseXpath + "/style:font-face[@style:name='Carlito']");
    assertXPath(pXmlDoc,
                aContentBaseXpath + "/style:font-face[@style:name='Carlito']/svg:font-face-src", 1);
    assertXPath(pXmlDoc, aContentBaseXpath + "/style:font-face[@style:name='Caladea']");
    assertXPath(pXmlDoc,
                aContentBaseXpath + "/style:font-face[@style:name='Caladea']/svg:font-face-src", 1);

    // CASE 3 - font embedding enabled, embed only used fonts enabled

    // Enable font embedding and setting to embed used fonts only
    xProps->setPropertyValue("EmbedFonts", uno::Any(true));
    xProps->setPropertyValue("EmbedOnlyUsedFonts", uno::Any(true));
    xProps->setPropertyValue("EmbedLatinScriptFonts", uno::Any(true));
    xProps->setPropertyValue("EmbedAsianScriptFonts", uno::Any(true));
    xProps->setPropertyValue("EmbedComplexScriptFonts", uno::Any(true));

    // Save the document again
    xStorable->storeToURL(aTempFile.GetURL(), aDescriptor);
    CPPUNIT_ASSERT(aTempFile.IsValid());

    // Check setting - font embedding should be enabled + embed only used fonts and scripts
    pXmlDoc = parseExportInternal(aTempFile.GetURL(), "settings.xml");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPathContent(
        pXmlDoc, aSettingsBaseXpath + "/config:config-item[@config:name='EmbedFonts']", "true");
    assertXPathContent(
        pXmlDoc, aSettingsBaseXpath + "/config:config-item[@config:name='EmbedOnlyUsedFonts']",
        "true");
    assertXPathContent(
        pXmlDoc, aSettingsBaseXpath + "/config:config-item[@config:name='EmbedLatinScriptFonts']",
        "true");
    assertXPathContent(
        pXmlDoc, aSettingsBaseXpath + "/config:config-item[@config:name='EmbedAsianScriptFonts']",
        "true");
    assertXPathContent(
        pXmlDoc, aSettingsBaseXpath + "/config:config-item[@config:name='EmbedComplexScriptFonts']",
        "true");

    // Check content - font-face-src should be present only for "Liberation Sans" fonts

    pXmlDoc = parseExportInternal(aTempFile.GetURL(), "content.xml");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, aContentBaseXpath + "/style:font-face", 6);
    assertXPath(pXmlDoc, aContentBaseXpath + "/style:font-face[@style:name='Liberation Sans']");
    assertXPath(
        pXmlDoc,
        aContentBaseXpath + "/style:font-face[@style:name='Liberation Sans']/svg:font-face-src", 0);
    assertXPath(pXmlDoc, aContentBaseXpath + "/style:font-face[@style:name='Liberation Sans1']");
    assertXPath(pXmlDoc,
                aContentBaseXpath
                    + "/style:font-face[@style:name='Liberation Sans1']/svg:font-face-src",
                0);
    assertXPath(pXmlDoc, aContentBaseXpath + "/style:font-face[@style:name='Liberation Serif']");
    assertXPath(pXmlDoc,
                aContentBaseXpath
                    + "/style:font-face[@style:name='Liberation Serif']/svg:font-face-src",
                1);
    assertXPath(pXmlDoc, aContentBaseXpath + "/style:font-face[@style:name='Liberation Serif1']");
    assertXPath(pXmlDoc,
                aContentBaseXpath
                    + "/style:font-face[@style:name='Liberation Serif1']/svg:font-face-src",
                1);
    assertXPath(pXmlDoc, aContentBaseXpath + "/style:font-face[@style:name='Carlito']");
    assertXPath(pXmlDoc,
                aContentBaseXpath + "/style:font-face[@style:name='Carlito']/svg:font-face-src", 1);
    assertXPath(pXmlDoc, aContentBaseXpath + "/style:font-face[@style:name='Caladea']");
    assertXPath(pXmlDoc,
                aContentBaseXpath + "/style:font-face[@style:name='Caladea']/svg:font-face-src", 0);
#endif
}

// Unit test for fix inconsistent bookmark behavior around at-char/as-char anchored frames
//
// We have a placeholder character in the sw doc model for as-char anchored frames,
// so it's possible to have a bookmark before/after the frame or a non-collapsed bookmark
// which covers the frame. The same is not true for at-char anchored frames,
// where the anchor points to a doc model position, but there is no placeholder character.
// If a bookmark is created covering the start and end of the anchor of the frame,
// internally we create a collapsed bookmark which has the same position as the anchor of the frame.
// When this doc model is handled by SwXParagraph::createEnumeration(),
// first the frame and then the bookmark is appended to the text portion enumeration,
// so your bookmark around the frame is turned into a collapsed bookmark after the frame.
// (The same happens when we roundtrip an ODT document representing this doc model.)
//
// Fix the problem by inserting collapsed bookmarks with affected anchor positions
// (same position is the anchor for an at-char frame) into the enumeration in two stages:
// first the start of them before frames and then the end of them + other bookmarks.
// This way UNO API users get their non-collapsed bookmarks around at-char anchored frames,
// similar to as-char ones.
CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testInconsistentBookmark)
{
    // create test document with text and bookmark
    {
        SwDoc* pDoc(createSwDoc(DATA_DIRECTORY, "testInconsistentBookmark.ott"));
        IDocumentMarkAccess& rIDMA(*pDoc->getIDocumentMarkAccess());
        SwNodeIndex aIdx(pDoc->GetNodes().GetEndOfContent(), -1);
        SwCursor aPaM(SwPosition(aIdx), nullptr);
        aPaM.SetMark();
        aPaM.MovePara(GoCurrPara, fnParaStart);
        aPaM.MovePara(GoCurrPara, fnParaEnd);
        rIDMA.makeMark(aPaM, "Mark", IDocumentMarkAccess::MarkType::BOOKMARK,
                       ::sw::mark::InsertMode::New);
        aPaM.Exchange();
        aPaM.DeleteMark();
    }

    // save document and verify the bookmark scoup
    {
        // save document
        utl::TempFile aTempFile;
        save("writer8", aTempFile);

        // load only content.xml
        if (xmlDocUniquePtr pXmlDoc = parseExportInternal(aTempFile.GetURL(), "content.xml"))
        {
            const OString aPath("/office:document-content/office:body/office:text/text:p");

            const int pos1 = getXPathPosition(pXmlDoc, aPath, "bookmark-start");
            const int pos2 = getXPathPosition(pXmlDoc, aPath, "control");
            const int pos3 = getXPathPosition(pXmlDoc, aPath, "bookmark-end");

            CPPUNIT_ASSERT_GREATER(pos1, pos2);
            CPPUNIT_ASSERT_GREATER(pos2, pos3);
        }
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testSpellOnlineParameter)
{
    SwDoc* pDoc = createSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    const SwViewOption* pOpt = pWrtShell->GetViewOptions();
    bool bSet = pOpt->IsOnlineSpell();

    uno::Sequence<beans::PropertyValue> params
        = comphelper::InitPropertySequence({ { "Enable", uno::Any(!bSet) } });
    dispatchCommand(mxComponent, ".uno:SpellOnline", params);
    CPPUNIT_ASSERT_EQUAL(!bSet, pOpt->IsOnlineSpell());

    // set the same state as now and we don't expect any change (no-toggle)
    params = comphelper::InitPropertySequence({ { "Enable", uno::Any(!bSet) } });
    dispatchCommand(mxComponent, ".uno:SpellOnline", params);
    CPPUNIT_ASSERT_EQUAL(!bSet, pOpt->IsOnlineSpell());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testRedlineAutoCorrect)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "redline-autocorrect.fodt");

    dispatchCommand(mxComponent, ".uno:GoToEndOfDoc", {});

    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);
    SwXTextDocument* pXTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXTextDocument);

    // show tracked deletion with enabled change tracking
    RedlineFlags const nMode(pWrtShell->GetRedlineFlags() | RedlineFlags::On);
    CPPUNIT_ASSERT(nMode & (RedlineFlags::ShowDelete | RedlineFlags::ShowInsert));
    pWrtShell->SetRedlineFlags(nMode);
    CPPUNIT_ASSERT(nMode & RedlineFlags::ShowDelete);

    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());

    emulateTyping(*pXTextDocument, u" ");

    // tdf#83419 This was "Ts " removing the deletion of "t" silently by sentence capitalization
    OUString sReplaced("ts ");
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());

    // hide delete redlines
    pWrtShell->SetRedlineFlags(nMode & ~RedlineFlags::ShowDelete);

    // repeat it with not visible redlining
    dispatchCommand(mxComponent, ".uno:Undo", {});

    emulateTyping(*pXTextDocument, u" ");

    sReplaced = "S ";
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());

    // show delete redlines
    pWrtShell->SetRedlineFlags(nMode);

    // This still keep the tracked deletion, capitalize only the visible text "s"
    // with tracked deletion of the original character
    sReplaced = "tsS ";
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());

    // repeat it with visible redlining and word auto replacement of "tset"
    dispatchCommand(mxComponent, ".uno:Undo", {});
    dispatchCommand(mxComponent, ".uno:Undo", {});

    emulateTyping(*pXTextDocument, u"et ");
    // This was "Ttest" removing the tracked deletion silently.
    // Don't replace, if a redline starts or ends within the text.
    sReplaced = "tset ";
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());

    // Otherwise replace it
    emulateTyping(*pXTextDocument, u"tset ");
    sReplaced = "tset test ";
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());

    // Including capitalization
    emulateTyping(*pXTextDocument, u"end. word ");
    sReplaced = "tset test end. Word ";
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());

    // tracked deletions after the correction point doesn't affect autocorrect
    dispatchCommand(mxComponent, ".uno:GoToStartOfDoc", {});
    emulateTyping(*pXTextDocument, u"a ");
    sReplaced = "A tset test end. Word ";
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testRedlineAutoCorrect2)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "redline-autocorrect2.fodt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);
    SwXTextDocument* pXTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXTextDocument);

    dispatchCommand(mxComponent, ".uno:GoToEndOfDoc", {});

    // show tracked deletion
    RedlineFlags const nMode(pWrtShell->GetRedlineFlags() | RedlineFlags::On);
    CPPUNIT_ASSERT(nMode & (RedlineFlags::ShowDelete | RedlineFlags::ShowInsert));
    pWrtShell->SetRedlineFlags(nMode);
    CPPUNIT_ASSERT(nMode & RedlineFlags::ShowDelete);

    emulateTyping(*pXTextDocument, u"... ");

    // This was "LoremLorem,…," (duplicating the deleted comma, but without deletion)
    // Don't replace, if a redline starts or ends within the text.
    OUString sReplaced = "Lorem,... ";
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());

    // Continue it:
    emulateTyping(*pXTextDocument, u"Lorem,... ");
    sReplaced = u"Lorem,... Lorem,… ";
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testEmojiAutoCorrect)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "redline-autocorrect2.fodt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);
    SwXTextDocument* pXTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXTextDocument);

    // Emoji replacement (:snowman: -> ☃)

    // without change tracking
    CPPUNIT_ASSERT(!(pWrtShell->GetRedlineFlags() & RedlineFlags::On));
    emulateTyping(*pXTextDocument, u":snowman:");
    OUString sReplaced = u"☃Lorem,";
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());

    // with change tracking (showing redlines)
    RedlineFlags const nMode(pWrtShell->GetRedlineFlags() | RedlineFlags::On);
    CPPUNIT_ASSERT(nMode & (RedlineFlags::ShowDelete | RedlineFlags::ShowInsert));
    pWrtShell->SetRedlineFlags(nMode);
    CPPUNIT_ASSERT(nMode & RedlineFlags::On);
    CPPUNIT_ASSERT(nMode & RedlineFlags::ShowDelete);

    emulateTyping(*pXTextDocument, u":snowman:");
    sReplaced = u"☃☃Lorem,";

    // tdf#140674 This was ":snowman:" instead of autocorrect
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testTdf108423)
{
    SwDoc* pDoc = createSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);
    SwXTextDocument* pXTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXTextDocument);
    // testing autocorrect of i' -> I' on start of first paragraph
    emulateTyping(*pXTextDocument, u"i'");
    // The word "i" should be capitalized due to autocorrect, followed by a typographical apostrophe
    OUString sIApostrophe(u"I\u2019");
    CPPUNIT_ASSERT_EQUAL(sIApostrophe, getParagraph(1)->getString());
    emulateTyping(*pXTextDocument, u" i'");
    OUString sText(sIApostrophe + u" " + sIApostrophe);
    CPPUNIT_ASSERT_EQUAL(sText, getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testTdf106164)
{
    SwDoc* pDoc = createSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);
    SwXTextDocument* pXTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXTextDocument);
    // testing autocorrect of we're -> We're on start of first paragraph
    emulateTyping(*pXTextDocument, u"we're ");
    CPPUNIT_ASSERT_EQUAL(OUString(u"We\u2019re "), getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testTdf54409)
{
    SwDoc* pDoc = createSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);
    SwXTextDocument* pXTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXTextDocument);
    // testing autocorrect of "tset -> "test with typographical double quotation mark U+201C
    emulateTyping(*pXTextDocument, u"\"test ");
    OUString sReplaced(u"\u201Ctest ");
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
    // testing autocorrect of test" -> test" with typographical double quotation mark U+201D
    emulateTyping(*pXTextDocument, u"and tset\" ");
    OUString sReplaced2(sReplaced + u"and test\u201D ");
    CPPUNIT_ASSERT_EQUAL(sReplaced2, getParagraph(1)->getString());
    // testing autocorrect of "tset" -> "test" with typographical double quotation mark U+201C and U+201D
    emulateTyping(*pXTextDocument, u"\"tset\" ");
    OUString sReplaced3(sReplaced2 + u"\u201Ctest\u201D ");
    CPPUNIT_ASSERT_EQUAL(sReplaced3, getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testTdf38394)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf38394.fodt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);
    SwXTextDocument* pXTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXTextDocument);
    // testing autocorrect of French l'" -> l'« (instead of l'»)
    emulateTyping(*pXTextDocument, u"l'\"");
    OUString sReplaced(u"l\u2019« ");
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
    // tdf#132301 autocorrect of qu'«
    emulateTyping(*pXTextDocument, u" qu'\"");
    sReplaced += u" qu\u2019« ";
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testTdf59666)
{
    SwDoc* pDoc = createSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);
    SwXTextDocument* pXTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXTextDocument);
    // testing missing autocorrect of single Greek letters
    emulateTyping(*pXTextDocument, u"π ");
    CPPUNIT_ASSERT_EQUAL(OUString(u"\u03C0 "), getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testTdf133524)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf133524.fodt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);
    SwXTextDocument* pXTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXTextDocument);
    // 1. Testing autocorrect of >> and <<
    // Example: »word«
    emulateTyping(*pXTextDocument, u">>");
    OUString sReplaced(u"»");
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
    // <<
    emulateTyping(*pXTextDocument, u"word<<");
    sReplaced += u"word«";
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
    // 2. Testing autocorrect of " to >> and << inside „...”
    // Example: „Sentence and »word«.”
    // opening primary level quote
    emulateTyping(*pXTextDocument, u" \"");
    sReplaced += u" „";
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
    // opening second level quote
    emulateTyping(*pXTextDocument, u"Sentence and \"");
    sReplaced += u"Sentence and »";
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
    // closing second level quote
    emulateTyping(*pXTextDocument, u"word\"");
    sReplaced += u"word«";
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
    // closing primary level quote
    emulateTyping(*pXTextDocument, u".\"");
    sReplaced += u".”";
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
    // tdf#134940 avoid premature replacement of "--" in "-->"
    emulateTyping(*pXTextDocument, u" -->");
    OUString sReplaced2(sReplaced + u" -->");
    // This was "–>" instead of "-->"
    CPPUNIT_ASSERT_EQUAL(sReplaced2, getParagraph(1)->getString());
    emulateTyping(*pXTextDocument, u" ");
    sReplaced += u" → ";
    // This was "–>" instead of "→"
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());

    // tdf#83037
    emulateTyping(*pXTextDocument, u"-> ");
    sReplaced += u"→ ";
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
    emulateTyping(*pXTextDocument, u"<- ");
    sReplaced += u"← ";
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
    emulateTyping(*pXTextDocument, u"<-- ");
    sReplaced += u"← ";
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
    emulateTyping(*pXTextDocument, u"<--> ");
    sReplaced += u"↔ ";
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testTdf133524_Romanian)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf133524_ro.fodt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);
    SwXTextDocument* pXTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXTextDocument);
    // 1. Testing autocorrect of " to << and >> inside „...”
    // Example: „Sentence and «word».”
    // opening primary level quote
    emulateTyping(*pXTextDocument, u"\"");
    OUString sReplaced(u"„");
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
    // opening second level quote
    emulateTyping(*pXTextDocument, u"Sentence and \"");
    sReplaced += u"Sentence and «";
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
    // closing second level quote
    emulateTyping(*pXTextDocument, u"word\"");
    sReplaced += u"word»";
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
    // closing primary level quote
    emulateTyping(*pXTextDocument, u".\"");
    sReplaced += u".”";
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
    // 2. Testing recognition of closing double quotation mark ”
    emulateTyping(*pXTextDocument, u" \"");
    sReplaced += u" „";
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
    // 3. Testing recognition of alternative closing double quotation mark “
    emulateTyping(*pXTextDocument, u"Alternative.“ \"");
    sReplaced += u"Alternative.“ „";
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testTdf128860)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf128860.fodt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);
    SwXTextDocument* pXTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXTextDocument);
    // Second level ending quote: ‚word' -> ,word‘
    emulateTyping(*pXTextDocument, u",word'");
    OUString sReplaced(u",word\u2019");
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
    // Us apostrophe without preceding starting quote: word' -> word’
    emulateTyping(*pXTextDocument, u" word'");
    sReplaced += u" word\u2019";
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
    // But only after letters: word.' -> word.‘
    emulateTyping(*pXTextDocument, u" word.'");
    sReplaced += u" word.‘";
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testTdf123786)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf123786.fodt");

    // On Windows, it will detect that system input language is en-US (despite "typing" e.g. Cyrillic characters),
    // and will change Russian into English (US); in the latter language,
    // the replacement from single quote will not become “, but ’.
    SvtSysLocaleOptions aOptions;
    aOptions.SetIgnoreLanguageChange(true);
    aOptions.Commit();

    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);
    SwXTextDocument* pXTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXTextDocument);
    // Second level ending quote: „word' -> „word“
    emulateTyping(*pXTextDocument, u"„слово'");
    OUString sReplaced(u"„слово“");
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
    // Us apostrophe without preceding starting quote: word' -> word’
    emulateTyping(*pXTextDocument, u" слово'");
    sReplaced += u" слово’";
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
    // But only after letters: word.' -> word.“
    emulateTyping(*pXTextDocument, u" слово.'");
    sReplaced += u" слово.“";
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testTdf133589)
{
    // Hungarian test document with right-to-left paragraph setting
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf133589.fodt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);
    SwXTextDocument* pXTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXTextDocument);
    // translitere words to Old Hungarian
    emulateTyping(*pXTextDocument, u"székely ");
    OUString sReplaced(u"𐳥𐳋𐳓𐳉𐳗 ");
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
    // disambiguate consonants: asszony -> asz|szony
    emulateTyping(*pXTextDocument, u"asszony ");
    sReplaced += u"𐳀𐳥𐳥𐳛𐳚 ";
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
    // disambiguate consonants: kosszarv -> kos|szarv
    // (add explicit ZWSP temporarily for consonant disambiguation, because the requested
    // hu_HU hyphenation dictionary isn't installed on all testing platform)
    // pWrtShell->Insert(u"kosszarv");
    emulateTyping(*pXTextDocument, u"kos\u200Bszarv ");
    sReplaced += u"𐳓𐳛𐳤𐳥𐳀𐳢𐳮 ";
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
    // transliterate numbers to Old Hungarian
    emulateTyping(*pXTextDocument, u"2020 ");
    sReplaced += u"𐳺𐳺𐳿𐳼𐳼 ";
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());

    // tdf#147546 transliterate punctuation marks

    // question mark
    emulateTyping(*pXTextDocument, u"Kérdőjel?");
    sReplaced += u"𐲓𐳋𐳢𐳇𐳟𐳒𐳉𐳖";
    OUString sReplaced2(sReplaced + "?");
    CPPUNIT_ASSERT_EQUAL(sReplaced2, getParagraph(1)->getString());
    emulateTyping(*pXTextDocument, u" ");
    sReplaced += u"⸮ ";
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
    // comma
    emulateTyping(*pXTextDocument, u"Vessző,");
    sReplaced += u"𐲮𐳉𐳥𐳥𐳟";
    sReplaced2 = sReplaced + ",";
    CPPUNIT_ASSERT_EQUAL(sReplaced2, getParagraph(1)->getString());
    emulateTyping(*pXTextDocument, u" ");
    sReplaced += u"⹁ ";
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
    // semicolon
    emulateTyping(*pXTextDocument, u"pontosvessző;");
    sReplaced += u"𐳠𐳛𐳙𐳦𐳛𐳤𐳮𐳉𐳥𐳥𐳟";
    sReplaced2 = sReplaced + ";";
    CPPUNIT_ASSERT_EQUAL(sReplaced2, getParagraph(1)->getString());
    emulateTyping(*pXTextDocument, u" ");
    sReplaced += u"⁏ ";
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
    // quotation marks
    emulateTyping(*pXTextDocument, u"„idézőjel” ");
    sReplaced += u"⹂𐳐𐳇𐳋𐳯𐳟𐳒𐳉𐳖‟ ";
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testAutoCorr)
{
    SwDoc* pDoc = createSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);
    SwXTextDocument* pXTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXTextDocument);

    //Normal AutoCorrect
    emulateTyping(*pXTextDocument, u"tset ");
    CPPUNIT_ASSERT_EQUAL(OUString("Test "), getParagraph(1)->getString());

    //AutoCorrect with change style to bolt
    emulateTyping(*pXTextDocument, u"Bolt ");
    const uno::Reference<text::XTextRange> xRun = getRun(getParagraph(1), 2);
    CPPUNIT_ASSERT_EQUAL(OUString("Bolt"), xRun->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("Arial"), getProperty<OUString>(xRun, "CharFontName"));

    //AutoCorrect inserts Table with 2 rows and 3 columns
    emulateTyping(*pXTextDocument, u"4xx ");
    const uno::Reference<text::XTextTable> xTable(getParagraphOrTable(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTable->getColumns()->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testTdf130274)
{
    SwDoc* const pDoc(createSwDoc());
    SwWrtShell* const pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);
    SwXTextDocument* pXTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXTextDocument);

    CPPUNIT_ASSERT(!pWrtShell->GetLayout()->IsHideRedlines());
    CPPUNIT_ASSERT(
        !IDocumentRedlineAccess::IsRedlineOn(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // "tset" may be replaced by the AutoCorrect in the test profile
    emulateTyping(*pXTextDocument, u"tset");
    // select from left to right
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/false, 4, /*bBasicCall=*/false);
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, 4, /*bBasicCall=*/false);

    pWrtShell->SetRedlineFlags(pWrtShell->GetRedlineFlags() | RedlineFlags::On);
    // this would crash in AutoCorrect
    emulateTyping(*pXTextDocument, u".");

    CPPUNIT_ASSERT(!pDoc->getIDocumentRedlineAccess().GetRedlineTable().empty());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testTdf83260)
{
    SwDoc* const pDoc(createSwDoc(DATA_DIRECTORY, "tdf83260-1.odt"));
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);
    SwXTextDocument* pXTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXTextDocument);

    // enabled but not shown
    CPPUNIT_ASSERT(pWrtShell->GetLayout()->IsHideRedlines());
#if 0
    CPPUNIT_ASSERT(IDocumentRedlineAccess::IsHideChanges(
            pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));
#endif
    CPPUNIT_ASSERT(
        IDocumentRedlineAccess::IsRedlineOn(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));
    CPPUNIT_ASSERT(!pDoc->getIDocumentRedlineAccess().GetRedlineTable().empty());

    // the document contains redlines that are combined with CompressRedlines()
    // if that happens during AutoCorrect then indexes in Undo are off -> crash
    emulateTyping(*pXTextDocument, u"tset ");
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    auto const nActions(rUndoManager.GetUndoActionCount());
    for (auto i = nActions; 0 < i; --i)
    {
        rUndoManager.Undo();
    }
    // check that every text node has a layout frame
    for (SwNodeOffset i(0); i < pDoc->GetNodes().Count(); ++i)
    {
        if (SwTextNode const* const pNode = pDoc->GetNodes()[i]->GetTextNode())
        {
            CPPUNIT_ASSERT(pNode->getLayoutFrame(nullptr, nullptr, nullptr));
        }
    }
    for (auto i = nActions; 0 < i; --i)
    {
        rUndoManager.Redo();
    }
    for (SwNodeOffset i(0); i < pDoc->GetNodes().Count(); ++i)
    {
        if (SwTextNode const* const pNode = pDoc->GetNodes()[i]->GetTextNode())
        {
            CPPUNIT_ASSERT(pNode->getLayoutFrame(nullptr, nullptr, nullptr));
        }
    }
    for (auto i = nActions; 0 < i; --i)
    {
        rUndoManager.Undo();
    }
    for (SwNodeOffset i(0); i < pDoc->GetNodes().Count(); ++i)
    {
        if (SwTextNode const* const pNode = pDoc->GetNodes()[i]->GetTextNode())
        {
            CPPUNIT_ASSERT(pNode->getLayoutFrame(nullptr, nullptr, nullptr));
        }
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testTdf74363)
{
    SwDoc* pDoc = createSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);
    SwXTextDocument* pXTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXTextDocument);
    //testing autocorrect of initial capitals on start of first paragraph
    //Inserting one all-lowercase word into the first paragraph
    emulateTyping(*pXTextDocument, u"testing ");
    //The word should be capitalized due to autocorrect
    CPPUNIT_ASSERT_EQUAL(OUString("Testing "), getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testTdf143176)
{
    // Hungarian test document with right-to-left paragraph setting
    createSwDoc(DATA_DIRECTORY, "tdf143176.fodt");

    // transliterate the document to Old Hungarian (note: it only works
    // with right-to-left text direction and Default Paragraph Style)
    dispatchCommand(mxComponent, ".uno:AutoFormatApply", {});

    // This was the original "Lorem ipsum..."
    CPPUNIT_ASSERT_EQUAL(OUString(u"𐲖𐳛𐳢𐳉𐳘 𐳐𐳠𐳤𐳪𐳘 𐳇𐳛𐳖𐳛𐳢 "
                                  u"𐳤𐳐𐳦 𐳀𐳘𐳉𐳦⹁"),
                         getParagraph(1)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString(u"𐳄𐳛𐳙𐳤𐳉𐳄𐳦𐳉𐳦𐳪𐳢 "
                                  u"𐳀𐳇𐳐𐳠𐳐𐳤𐳄𐳐𐳙𐳍 𐳉𐳖𐳐𐳦."),
                         getParagraph(2)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testInsertLongDateFormat)
{
    // only for Hungarian, yet
    createSwDoc(DATA_DIRECTORY, "tdf133524.fodt");
    dispatchCommand(mxComponent, ".uno:InsertDateField", {});
    // Make sure that the document starts with a field now, and its expanded string value contains space
    const uno::Reference<text::XTextRange> xField = getRun(getParagraph(1), 1);
    CPPUNIT_ASSERT_EQUAL(OUString("TextField"), getProperty<OUString>(xField, "TextPortionType"));
    // the date format was "YYYY-MM-DD", but now "YYYY. MMM DD."
    CPPUNIT_ASSERT(xField->getString().indexOf(" ") > -1);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testTdf129270)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf129270.odt");
    CPPUNIT_ASSERT(pDoc);
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);
    SwXTextDocument* pXTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXTextDocument);

    // Go to document end
    pWrtShell->SttEndDoc(/*bStt=*/false);

    // Press enter
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_RETURN);
    Scheduler::ProcessEventsToIdle();

    // Numbering for previous outline should remain the same "2"
    CPPUNIT_ASSERT_EQUAL(OUString("2"), getProperty<OUString>(getParagraph(4), "ListLabelString"));

    // Numbering for newly created outline should be "2.1"
    CPPUNIT_ASSERT_EQUAL(OUString("2.1"),
                         getProperty<OUString>(getParagraph(5), "ListLabelString"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testInsertPdf)
{
    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    if (!pPdfium)
    {
        return;
    }

    createSwDoc();

    // insert the PDF into the document
    uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence(
        { { "FileName",
            uno::Any(m_directories.getURLFromSrc(DATA_DIRECTORY) + "hello-world.pdf") } }));
    dispatchCommand(mxComponent, ".uno:InsertGraphic", aArgs);

    // Save and load cycle
    reload("writer8", "testInsertPdf.odt");

    uno::Reference<drawing::XShape> xShape = getShape(1);
    // Assert that we have a replacement graphics
    auto xReplacementGraphic
        = getProperty<uno::Reference<graphic::XGraphic>>(xShape, "ReplacementGraphic");
    CPPUNIT_ASSERT(xReplacementGraphic.is());

    auto xGraphic = getProperty<uno::Reference<graphic::XGraphic>>(xShape, "Graphic");
    CPPUNIT_ASSERT(xGraphic.is());
    // Assert that the graphic is a PDF
    CPPUNIT_ASSERT_EQUAL(OUString("application/pdf"), getProperty<OUString>(xGraphic, "MimeType"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testTdf143760WrapContourToOff)
{
    // Actually, this is an ooxmlexport test. It is here because here is a ready environment
    // to change a shape by dispatchCommand.
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf143760_ContourToWrapOff.docx");
    CPPUNIT_ASSERT(pDoc);
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(getShape(1), "SurroundContour"));

    // Mark the object
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pObject = pPage->GetObj(0);
    CPPUNIT_ASSERT(pObject);
    SdrView* pView = pWrtShell->GetDrawView();
    pView->MarkObj(pObject, pView->GetSdrPageView());

    // Set "wrap off"
    dispatchCommand(mxComponent, ".uno:WrapOff", {});
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(getShape(1), "SurroundContour"));

    // Without fix this had failed, because the shape was written to file with contour.
    reload("Office Open XML Text", "tdf143760_ContourToWrapOff.docx");
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(getShape(1), "SurroundContour"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testHatchFill)
{
    createSwDoc();

    // Add a rectangle shape to the document.
    uno::Reference<css::lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(
        xFactory->createInstance("com.sun.star.drawing.RectangleShape"), uno::UNO_QUERY);
    xShape->setSize(awt::Size(10000, 10000));
    xShape->setPosition(awt::Point(1000, 1000));
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    xShapeProps->setPropertyValue("FillStyle", uno::Any(drawing::FillStyle_HATCH));
    xShapeProps->setPropertyValue("FillHatchName", uno::Any(OUString("Black 0 Degrees")));
    xShapeProps->setPropertyValue("FillBackground", uno::Any(false));
    xShapeProps->setPropertyValue("FillTransparence", uno::Any(sal_Int32(30)));
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    xDrawPage->add(xShape);

    // Save it as DOCX and load it again.
    reload("Office Open XML Text", "hatchFill.docx");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());

    // tdf#127989 Without fix this had failed, because the background of the hatch was not set as 'no background'.
    CPPUNIT_ASSERT(!getProperty<bool>(getShape(1), "FillBackground"));

    // tdf#146822 Without fix this had failed, because the transparency value of the hatch was not exported.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(30), getProperty<sal_Int32>(getShape(1), "FillTransparence"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testNestedGroupTextBoxCopyCrash)
{
    createSwDoc(DATA_DIRECTORY, "tdf149550.docx");

    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    Scheduler::ProcessEventsToIdle();
    dispatchCommand(mxComponent, ".uno:Copy", {});
    Scheduler::ProcessEventsToIdle();
    // This crashed here before the fix.
    SwXTextDocument* pXTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXTextDocument);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_ESCAPE);
    Scheduler::ProcessEventsToIdle();
    dispatchCommand(mxComponent, ".uno:Paste", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_MESSAGE("Where is the doc, it crashed, isn't it?!", mxComponent);

    auto pLayout = parseLayoutDump();
    // There must be 2 textboxes!
    assertXPath(pLayout, "/root/page/body/txt/anchored/fly[2]");
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest6, testCaptionShape)
{
    createSwDoc();

    // Add a caption shape to the document.
    uno::Reference<css::lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(
        xFactory->createInstance("com.sun.star.drawing.CaptionShape"), uno::UNO_QUERY);
    xShape->setSize(awt::Size(10000, 10000));
    xShape->setPosition(awt::Point(1000, 1000));
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    xDrawPage->add(xShape);

    // Save it as DOCX and load it again.
    reload("Office Open XML Text", "captionshape.docx");

    // Without fix in place, the shape was lost on export.
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
