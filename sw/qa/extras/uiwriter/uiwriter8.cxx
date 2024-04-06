/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <vcl/filter/PDFiumLibrary.hxx>
#include <vcl/scheduler.hxx>
#include <vcl/TypeSerializer.hxx>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/drawing/GraphicExportFilter.hpp>
#include <IDocumentDrawModelAccess.hxx>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <comphelper/propertysequence.hxx>
#include <boost/property_tree/json_parser.hpp>
#include <frameformats.hxx>
#include <tools/json_writer.hxx>
#include <unotools/streamwrap.hxx>
#include <sfx2/linkmgr.hxx>

#include <wrtsh.hxx>
#include <unotxdoc.hxx>
#include <drawdoc.hxx>
#include <dcontact.hxx>
#include <svx/svdpage.hxx>
#include <ndtxt.hxx>
#include <txtfld.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentLinksAdministration.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <rootfrm.hxx>
#include <redline.hxx>
#include <itabenum.hxx>
#include <officecfg/Office/Common.hxx>

namespace
{
/// 8th set of tests asserting the behavior of Writer user interface shells.
class SwUiWriterTest8 : public SwModelTestBase
{
public:
    SwUiWriterTest8()
        : SwModelTestBase("/sw/qa/extras/uiwriter/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf131684)
{
    createSwDoc("tdf131684.docx");

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    //Use selectAll 3 times in a row
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    dispatchCommand(mxComponent, ".uno:Cut", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xIndexAccess->getCount());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    dispatchCommand(mxComponent, ".uno:Paste", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    // without the fix, it crashes
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    // check that the text frame has the correct upper
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    OUString const sectionId = getXPath(pXmlDoc, "/root/page[1]/body/section[7]"_ostr, "id"_ostr);
    OUString const sectionLower
        = getXPath(pXmlDoc, "/root/page[1]/body/section[7]"_ostr, "lower"_ostr);
    OUString const textId
        = getXPath(pXmlDoc, "/root/page[1]/body/section[7]/txt[1]"_ostr, "id"_ostr);
    OUString const textUpper
        = getXPath(pXmlDoc, "/root/page[1]/body/section[7]/txt[1]"_ostr, "upper"_ostr);
    CPPUNIT_ASSERT_EQUAL(textId, sectionLower);
    CPPUNIT_ASSERT_EQUAL(sectionId, textUpper);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf132420)
{
    createSwDoc("tdf132420.odt");

    CPPUNIT_ASSERT_EQUAL(12, getShapes());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    dispatchCommand(mxComponent, ".uno:Cut", {});
    CPPUNIT_ASSERT_EQUAL(0, getShapes());

    dispatchCommand(mxComponent, ".uno:Undo", {});

    //Without the fix in place, 1 frame and 1 image would be gone and getShapes would return 10
    CPPUNIT_ASSERT_EQUAL(12, getShapes());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf132744)
{
    createSwDoc("tdf132744.odt");
    SwDoc* pDoc = getSwDoc();

    // disable change tracking to cut the table
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);

    CPPUNIT_ASSERT_MESSAGE("redlining should be off",
                           !pDoc->getIDocumentRedlineAccess().IsRedlineOn());

    CPPUNIT_ASSERT_EQUAL(1, getShapes());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    dispatchCommand(mxComponent, ".uno:Cut", {});

    CPPUNIT_ASSERT_EQUAL(0, getShapes());

    dispatchCommand(mxComponent, ".uno:Paste", {});

    //Without the fix in place, the image wouldn't be pasted
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf146622)
{
    createSwDoc("TC-table-del-add.docx");
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());

    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTables->getCount());
    uno::Reference<container::XNameAccess> xTableNames = xTablesSupplier->getTextTables();
    CPPUNIT_ASSERT(xTableNames->hasByName("Table1"));
    uno::Reference<text::XTextTable> xTable1(xTableNames->getByName("Table1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xTable1->getRows()->getCount());

    dispatchCommand(mxComponent, ".uno:DeleteRows", {});

    // This was 3 (deleting the already deleted row with change tracking)
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xTable1->getRows()->getCount());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    dispatchCommand(mxComponent, ".uno:DeleteRows", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xTable1->getRows()->getCount());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    dispatchCommand(mxComponent, ".uno:DeleteRows", {});
    // This was 2 (deleting the already deleted table with change tracking)
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xTable1->getRows()->getCount());

    // check that the first table was deleted with change tracking
    dispatchCommand(mxComponent, ".uno:AcceptAllTrackedChanges", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());

    // Undo AcceptAllTrackedChanges and DeleteRows
    dispatchCommand(mxComponent, ".uno:Undo", {});
    dispatchCommand(mxComponent, ".uno:Undo", {});

    // now only the second table deleted by AcceptAllTrackedChanges
    dispatchCommand(mxComponent, ".uno:AcceptAllTrackedChanges", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTables->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf146962)
{
    // load a 2-row table, set Hide Changes mode and delete the first row with change tracking
    createSwDoc("tdf116789.fodt");
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    // enable redlining
    dispatchCommand(mxComponent, ".uno:TrackChanges", {});
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    // hide changes
    dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
    CPPUNIT_ASSERT(pWrtShell->GetLayout()->IsHideRedlines());

    dispatchCommand(mxComponent, ".uno:DeleteRows", {});

    // Without the fix in place, the deleted row would be visible

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // This was 2
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row"_ostr, 1);

    // check it in Show Changes mode

    dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
    CPPUNIT_ASSERT(!pWrtShell->GetLayout()->IsHideRedlines());

    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    // 2 rows are visible now
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row"_ostr, 2);

    // check it in Hide Changes mode again

    dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
    CPPUNIT_ASSERT(pWrtShell->GetLayout()->IsHideRedlines());

    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    // only a single row is visible again
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row"_ostr, 1);

    // tdf#148227 check Undo of tracked table row deletion

    dispatchCommand(mxComponent, ".uno:Undo", {});
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    // This was 1
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row"_ostr, 2);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf159026)
{
    // load a floating table (tables in DOCX footnotes
    // imported as floating tables in Writer)
    createSwDoc("tdf159026.docx");
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    // enable redlining
    dispatchCommand(mxComponent, ".uno:TrackChanges", {});
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    // hide changes
    dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
    CPPUNIT_ASSERT(pWrtShell->GetLayout()->IsHideRedlines());

    // select table with SelectionSupplier
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<view::XSelectionSupplier> xSelSupplier(xModel->getCurrentController(),
                                                          uno::UNO_QUERY_THROW);
    // select floating table (table in a frame)
    xSelSupplier->select(xIndexAccess->getByIndex(0));

    // delete table with track changes
    dispatchCommand(mxComponent, ".uno:DeleteTable", {});

    // tracked table deletion
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    // hidden table
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//tab"_ostr, 0);

    // delete frame
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess2(xTextFramesSupplier->getTextFrames(),
                                                          uno::UNO_QUERY);
    xSelSupplier->select(xIndexAccess2->getByIndex(0));
    dispatchCommand(mxComponent, ".uno:Delete", {});

    // undo frame deletion
    dispatchCommand(mxComponent, ".uno:Undo", {});

    // undo tracked table deletion

    // This resulted crashing
    dispatchCommand(mxComponent, ".uno:Undo", {});
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf147347)
{
    // load a 2-row table, set Hide Changes mode and delete the table with change tracking
    createSwDoc("tdf116789.fodt");
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    // enable redlining
    dispatchCommand(mxComponent, ".uno:TrackChanges", {});
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    // hide changes
    dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
    CPPUNIT_ASSERT(pWrtShell->GetLayout()->IsHideRedlines());

    dispatchCommand(mxComponent, ".uno:DeleteTable", {});

    // Without the fix in place, the deleted row would be visible

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // This was 1
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row"_ostr, 0);

    // check it in Show Changes mode

    dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
    CPPUNIT_ASSERT(!pWrtShell->GetLayout()->IsHideRedlines());

    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    // 2 rows are visible now
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row"_ostr, 2);

    // check it in Hide Changes mode again

    dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
    CPPUNIT_ASSERT(pWrtShell->GetLayout()->IsHideRedlines());

    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    // no visible row again
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row"_ostr, 0);

    // tdf#148228 check Undo of tracked table deletion

    dispatchCommand(mxComponent, ".uno:Undo", {});
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    // This was 0
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row"_ostr, 2);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf153819)
{
    // copy a table before a deleted table in Hide Changes mode
    createSwDoc("tdf153819.fodt");
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    // hide changes
    CPPUNIT_ASSERT(pWrtShell->GetLayout()->IsHideRedlines());

    dispatchCommand(mxComponent, ".uno:SelectTable", {});
    dispatchCommand(mxComponent, ".uno:Copy", {});
    dispatchCommand(mxComponent, ".uno:GoDown", {});

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, ".uno:Paste", {});

    // FIXME: Show Changes, otherwise ~SwTableNode() would have crashed
    dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf148345)
{
    // load a 2-row table, set Hide Changes mode and delete the first row with change tracking
    createSwDoc("tdf116789.fodt");
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    // enable redlining
    dispatchCommand(mxComponent, ".uno:TrackChanges", {});
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    // hide changes
    dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
    CPPUNIT_ASSERT(pWrtShell->GetLayout()->IsHideRedlines());

    dispatchCommand(mxComponent, ".uno:DeleteRows", {});

    // Without the fix in place, the deleted row would be visible

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // This was 2
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row"_ostr, 1);

    // check it in Show Changes mode

    dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
    CPPUNIT_ASSERT(!pWrtShell->GetLayout()->IsHideRedlines());

    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    // 2 rows are visible now
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row"_ostr, 2);

    // check it in Hide Changes mode again

    dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
    CPPUNIT_ASSERT(pWrtShell->GetLayout()->IsHideRedlines());

    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    // only a single row is visible again
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row"_ostr, 1);

    // tdf#148227 check Reject All of tracked table row deletion

    dispatchCommand(mxComponent, ".uno:RejectAllTrackedChanges", {});
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    // This was 1
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row"_ostr, 2);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf141391)
{
    // table insertion in the first paragraph of the cell
    // overwrites the row content, instead of inserting a nested table

    // load a 2-row table
    createSwDoc("tdf116789.fodt");
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    // select the table, and copy it into at paragraph start of cell "A2"

    dispatchCommand(mxComponent, ".uno:SelectTable", {});
    dispatchCommand(mxComponent, ".uno:Copy", {});
    // remove the selection and positionate the cursor at beginning of A2
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    dispatchCommand(mxComponent, ".uno:Paste", {});

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // 3-row, overwriting cells of the second row and inserting a new row
    // with the 2-row clipboard table content
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row"_ostr, 3);
    assertXPath(pXmlDoc,
                "/root/page[1]/body/tab/row[2]/cell[1]/txt/SwParaPortion/SwLineLayout"_ostr,
                "portion"_ostr, "hello");

    // Undo

    dispatchCommand(mxComponent, ".uno:Undo", {});
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    // 2 rows again, no copied text content
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row"_ostr, 2);
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row[2]/cell[1]/SwParaPortion"_ostr, 0);

    // insert the 2-row table into the second paragraph of cell "A2" as a nested table
    // For this it's enough to positionate the text cursor not in the first paragraph

    // insert some text and an empty paragraph
    pWrtShell->Insert("Some text...");
    pWrtShell->SplitNode();
    Scheduler::ProcessEventsToIdle();
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row"_ostr, 2);
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row[2]/cell[1]/txt"_ostr, 2);
    assertXPath(pXmlDoc,
                "/root/page[1]/body/tab/row[2]/cell[1]/txt[1]/SwParaPortion/SwLineLayout"_ostr,
                "portion"_ostr, "Some text...");
    // the empty paragraph in A2
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row[2]/cell[1]/txt[2]/SwParaPortion"_ostr, 0);

    // insert the table, as a nested one in cell "A2"
    dispatchCommand(mxComponent, ".uno:Paste", {});
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row"_ostr, 2);
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row[2]/cell[1]/tab"_ostr, 1);
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row[2]/cell[1]/tab/row"_ostr, 2);

    // Undo

    dispatchCommand(mxComponent, ".uno:Undo", {});
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    // 2 rows again, no copied text content
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row"_ostr, 2);
    assertXPath(pXmlDoc,
                "/root/page[1]/body/tab/row[2]/cell[1]/txt[1]/SwParaPortion/SwLineLayout"_ostr,
                "portion"_ostr, "Some text...");

    // copy the 2-row table into the first paragraph of cell "A2",
    // but not at paragraph start (changed behaviour)

    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    pWrtShell->Insert("and some text again in the first paragraph to be sure...");
    dispatchCommand(mxComponent, ".uno:Paste", {});

    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();

    // 3-row, overwriting cells of the second row and inserting a new row
    // with the 2-row clipboard table content

    // This was 2 (nested table)
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row"_ostr, 3);
    // This was "Some text..." with a nested table
    assertXPath(pXmlDoc,
                "/root/page[1]/body/tab/row[2]/cell[1]/txt/SwParaPortion/SwLineLayout"_ostr,
                "portion"_ostr, "hello");
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf148791)
{
    // test Paste as Rows Above with centered table alignment

    // load a 2-row table
    createSwDoc("tdf116789.fodt");
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    // select and copy the table, and Paste As Rows Above

    dispatchCommand(mxComponent, ".uno:SelectTable", {});
    dispatchCommand(mxComponent, ".uno:Copy", {});
    // remove the selection and positionate the cursor at beginning of A2
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    pWrtShell->Up(/*bSelect=*/false);
    dispatchCommand(mxComponent, ".uno:PasteRowsBefore", {});

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // Paste as Rows Above results 4-row table with default table alignment
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row"_ostr, 4);
    assertXPath(pXmlDoc,
                "/root/page[1]/body/tab/row[1]/cell[1]/txt/SwParaPortion/SwLineLayout"_ostr,
                "portion"_ostr, "hello");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/tab/row[3]/cell[1]/txt/SwParaPortion/SwLineLayout"_ostr,
                "portion"_ostr, "hello");

    // set table alignment to center, select and copy the table again
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    uno::Reference<text::XTextTable> xTextTable(xIndexAccess->getByIndex(0), uno::UNO_QUERY);

    // Default table alignment
    CPPUNIT_ASSERT_EQUAL(text::HoriOrientation::FULL,
                         getProperty<sal_Int16>(xTextTable, "HoriOrient"));

    //CPPUNIT_ASSERT_EQUAL(OUString(""), getProperty<OUString>(xTextTable, "TableTemplateName"));
    uno::Reference<beans::XPropertySet> xTableProps(xTextTable, uno::UNO_QUERY_THROW);

    xTableProps->setPropertyValue("HoriOrient", uno::Any(text::HoriOrientation::CENTER));

    CPPUNIT_ASSERT_EQUAL(text::HoriOrientation::CENTER,
                         getProperty<sal_Int16>(xTextTable, "HoriOrient"));

    dispatchCommand(mxComponent, ".uno:SelectTable", {});
    dispatchCommand(mxComponent, ".uno:Copy", {});
    // remove the selection and positionate the cursor at beginning of A2
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    pWrtShell->Up(/*bSelect=*/false);
    pWrtShell->Up(/*bSelect=*/false);
    pWrtShell->Up(/*bSelect=*/false);
    dispatchCommand(mxComponent, ".uno:PasteRowsBefore", {});

    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    // This was 5 (inserting only a single row for the 4-row clipboard content, and
    // overwriting 3 existing rows)
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row"_ostr, 8);
    assertXPath(pXmlDoc,
                "/root/page[1]/body/tab/row[1]/cell[1]/txt/SwParaPortion/SwLineLayout"_ostr,
                "portion"_ostr, "hello");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/tab/row[3]/cell[1]/txt/SwParaPortion/SwLineLayout"_ostr,
                "portion"_ostr, "hello");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/tab/row[5]/cell[1]/txt/SwParaPortion/SwLineLayout"_ostr,
                "portion"_ostr, "hello");
    assertXPath(pXmlDoc,
                "/root/page[1]/body/tab/row[7]/cell[1]/txt/SwParaPortion/SwLineLayout"_ostr,
                "portion"_ostr, "hello");

    // tdf#64902 add a test case for nested tables

    // insert a nested table, and copy as paste as rows above the whole table with it
    dispatchCommand(mxComponent, ".uno:PasteNestedTable", {});
    dispatchCommand(mxComponent, ".uno:SelectTable", {});
    dispatchCommand(mxComponent, ".uno:Copy", {});
    // remove the selection and positionate the cursor at beginning of A2
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    // skip 7 table rows plus 4 rows of the nested table
    for (int i = 0; i < 7 + 4; ++i)
        pWrtShell->Up(/*bSelect=*/false);
    dispatchCommand(mxComponent, ".uno:PasteRowsBefore", {});

    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    // rows of the nested table doesn't effect row number of the main table
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row"_ostr, 16);
    // there are two nested tables after the paste
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row/cell/tab"_ostr, 2);

    // tdf#64902 add a test case for repeated table headings

    xTableProps->setPropertyValue("RepeatHeadline", uno::Any(true));
    CPPUNIT_ASSERT(getProperty<bool>(xTextTable, "RepeatHeadline"));

    xTableProps->setPropertyValue("HeaderRowCount", uno::Any(sal_Int32(3)));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), getProperty<sal_Int32>(xTextTable, "HeaderRowCount"));

    dispatchCommand(mxComponent, ".uno:SelectTable", {});
    dispatchCommand(mxComponent, ".uno:Copy", {});
    // remove the selection and positionate the cursor at beginning of A2
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    // skip 15 table rows plus 4 * 2 rows of the nested tables
    for (int i = 0; i < 15 + 4 * 2; ++i)
        pWrtShell->Up(/*bSelect=*/false);
    dispatchCommand(mxComponent, ".uno:PasteRowsBefore", {});

    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    // repeating table header (and its thead/tbody indentation) doesn't effect row number
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row"_ostr, 32);
    // there are two nested tables after the paste
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row/cell/tab"_ostr, 4);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf135014)
{
    createSwDoc();

    uno::Sequence<beans::PropertyValue> aArgs(
        comphelper::InitPropertySequence({ { "KeyModifier", uno::Any(sal_Int32(0)) } }));

    // Toggle Numbering List
    dispatchCommand(mxComponent, ".uno:DefaultBullet", aArgs);

    uno::Sequence<beans::PropertyValue> aArgs2(comphelper::InitPropertySequence(
        { { "Param", uno::Any(OUString("NewNumberingStyle")) },
          { "Family", uno::Any(static_cast<sal_Int16>(SfxStyleFamily::Pseudo)) } }));

    // New Style from selection
    dispatchCommand(mxComponent, ".uno:StyleNewByExample", aArgs2);

    // Without the fix in place, this test would have failed here
    saveAndReload("Office Open XML Text");

    xmlDocUniquePtr pXmlStyles = parseExport("word/styles.xml");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='NewNumberingStyle']/w:qFormat"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf130629)
{
    createSwDoc();
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());

    uno::Sequence<beans::PropertyValue> aArgs(
        comphelper::InitPropertySequence({ { "KeyModifier", uno::Any(KEY_MOD1) } }));

    dispatchCommand(mxComponent, ".uno:BasicShapes.diamond", aArgs);

    CPPUNIT_ASSERT_EQUAL(1, getShapes());

    // Undo twice
    dispatchCommand(mxComponent, ".uno:Undo", {});
    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT_EQUAL(0, getShapes());

    // Shape toolbar is active, use ESC before inserting a new shape
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_ESCAPE);
    Scheduler::ProcessEventsToIdle();

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, ".uno:BasicShapes.diamond", aArgs);

    CPPUNIT_ASSERT_EQUAL(1, getShapes());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf145584)
{
    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    if (!pPDFium)
    {
        return;
    }
    createSwDoc();
    SwDoc* const pDoc = getSwDoc();
    SwWrtShell* const pWrtSh = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtSh);

    pWrtSh->Insert("Hello World");

    // Select 'World'
    pWrtSh->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 5, /*bBasicCall=*/false);

    // Save as PDF.
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "Selection", uno::Any(true) } }));

    uno::Sequence<beans::PropertyValue> aDescriptor(comphelper::InitPropertySequence(
        { { "FilterName", uno::Any(OUString("writer_pdf_Export")) },
          { "FilterData", uno::Any(aFilterData) },
          { "URL", uno::Any(maTempFile.GetURL()) } }));

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, ".uno:ExportToPDF", aDescriptor);

    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);
    CPPUNIT_ASSERT_EQUAL(1, pPdfPage->getObjectCount());
    std::unique_ptr<vcl::pdf::PDFiumTextPage> pPdfTextPage = pPdfPage->getTextPage();
    CPPUNIT_ASSERT(pPdfTextPage);

    std::unique_ptr<vcl::pdf::PDFiumPageObject> pPageObject = pPdfPage->getObject(0);
    OUString sText = pPageObject->getText(pPdfTextPage);
    CPPUNIT_ASSERT_EQUAL(OUString("World"), sText);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf152575)
{
    // FIXME: the DPI check should be removed when either (1) the test is fixed to work with
    // non-default DPI; or (2) unit tests on Windows are made to use svp VCL plugin.
    if (!IsDefaultDPI())
        return;
    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    if (!pPDFium)
        return;

    createSwDoc("152575.fodt");

    // Save as PDF.
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "ExportNotesInMargin", uno::Any(true) } }));

    uno::Sequence<beans::PropertyValue> aDescriptor(comphelper::InitPropertySequence(
        { { "FilterName", uno::Any(OUString("writer_pdf_Export")) },
          { "FilterData", uno::Any(aFilterData) },
          { "URL", uno::Any(maTempFile.GetURL()) } }));

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, ".uno:ExportToPDF", aDescriptor);

    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();
    CPPUNIT_ASSERT_EQUAL(3, pPdfDocument->getPageCount());
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/1);
    CPPUNIT_ASSERT(pPdfPage);
    // Without the fix for tdf#152575 this would be only 42 objects
    CPPUNIT_ASSERT_EQUAL(50, pPdfPage->getObjectCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf140731)
{
    createSwDoc();
    SwDoc* const pDoc = getSwDoc();
    SwWrtShell* const pWrtSh = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtSh);

    pWrtSh->Insert("Lorem");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());

    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_F3);
    Scheduler::ProcessEventsToIdle();

    // generating a big text with ~60k words and several paragraphs
    for (sal_Int32 i = 0; i < 8; ++i)
    {
        dispatchCommand(mxComponent, ".uno:SelectAll", {});

        dispatchCommand(mxComponent, ".uno:Copy", {});

        dispatchCommand(mxComponent, ".uno:Paste", {});

        dispatchCommand(mxComponent, ".uno:Paste", {});
    }

    dispatchCommand(mxComponent, ".uno:GoToStartOfDoc", {});

    // Format->Text operations on small selections (which would generate <~500 redlines)
    // changetracking still working
    dispatchCommand(mxComponent, ".uno:TrackChanges", {});

    SwCursorShell* pShell(pDoc->GetEditShell());

    pShell->SelectTextModel(1, 500);

    dispatchCommand(mxComponent, ".uno:ChangeCaseToTitleCase", {});

    SwEditShell* const pEditShell(pDoc->GetEditShell());
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(120),
                         pEditShell->GetRedlineCount());

    //Removing all the redlines.
    dispatchCommand(mxComponent, ".uno:RejectAllTrackedChanges", {});

    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(0), pEditShell->GetRedlineCount());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    dispatchCommand(mxComponent, ".uno:ChangeCaseToTitleCase", {});

    // Without the fix in place, on big selections writer would freeze. Now it ignores change tracking.
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(0), pEditShell->GetRedlineCount());

    // The patch has no effects on the Format->Text operations
    CPPUNIT_ASSERT(getParagraph(1)->getString().startsWith("Lorem Ipsum Dolor Sit Amet"));

    dispatchCommand(mxComponent, ".uno:ChangeCaseToUpper", {});

    CPPUNIT_ASSERT(getParagraph(1)->getString().startsWith("LOREM IPSUM DOLOR SIT AMET"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf116315)
{
    createSwDoc();
    SwDoc* const pDoc = getSwDoc();
    SwWrtShell* const pWrtSh = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtSh);

    pWrtSh->Insert("This is a test");
    pWrtSh->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 4, /*bBasicCall=*/false);

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    for (sal_Int32 i = 0; i < 5; ++i)
    {
        pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_SHIFT | KEY_F3);
        Scheduler::ProcessEventsToIdle();

        // Title Case
        CPPUNIT_ASSERT_EQUAL(OUString("This is a Test"), getParagraph(1)->getString());

        pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_SHIFT | KEY_F3);
        Scheduler::ProcessEventsToIdle();

        // Upper Case
        CPPUNIT_ASSERT_EQUAL(OUString("This is a TEST"), getParagraph(1)->getString());

        pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_SHIFT | KEY_F3);
        Scheduler::ProcessEventsToIdle();

        // Lower Case
        CPPUNIT_ASSERT_EQUAL(OUString("This is a test"), getParagraph(1)->getString());
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf144364)
{
    createSwDoc();
    SwDoc* const pDoc = getSwDoc();
    SwWrtShell* const pWrtSh = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtSh);

    // expands autotext (via F3)
    pWrtSh->Insert("AR");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_F3);
    Scheduler::ProcessEventsToIdle();

    // was ...'letter of <placeholder:"November 21, 2004":"Click placeholder and overwrite">'
    CPPUNIT_ASSERT_EQUAL(
        OUString("We hereby acknowledge the receipt of your letter of <November 21, 2004>."),
        getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf146248)
{
    createSwDoc("tdf146248.docx");

    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"),
                                                   uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xPageStyle, "HeaderIsOn"));

    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    // Delete the header
    pWrtShell->ChangeHeaderOrFooter(u"Default Page Style", true, false, false);

    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(xPageStyle, "HeaderIsOn"));

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xPageStyle, "HeaderIsOn"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf152964)
{
    createSwDoc();

    dispatchCommand(mxComponent, ".uno:TrackChanges", {});
    dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});

    uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence(
        { { "Rows", uno::Any(sal_Int32(2)) }, { "Columns", uno::Any(sal_Int32(2)) } }));

    dispatchCommand(mxComponent, ".uno:InsertTable", aArgs);

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTextTable->getColumns()->getCount());

    SwDoc* pDoc = getSwDoc();
    SwEditShell* const pEditShell(pDoc->GetEditShell());
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(1), pEditShell->GetRedlineCount());
    dispatchCommand(mxComponent, ".uno:GoDown", {});
    dispatchCommand(mxComponent, ".uno:DeleteRows", {});
    dispatchCommand(mxComponent, ".uno:DeleteRows", {});

    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(2), pEditShell->GetRedlineCount());

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(1), pEditShell->GetRedlineCount());

    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(0), pEditShell->GetRedlineCount());

    dispatchCommand(mxComponent, ".uno:Redo", {});

    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(1), pEditShell->GetRedlineCount());
    dispatchCommand(mxComponent, ".uno:Redo", {});

    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(2), pEditShell->GetRedlineCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf107427)
{
    createSwDoc();

    dispatchCommand(mxComponent,
                    ".uno:InsertPageHeader?PageStyle:string=Default%20Page%20Style&On:bool=true",
                    {});
    uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence(
        { { "Rows", uno::Any(sal_Int32(2)) }, { "Columns", uno::Any(sal_Int32(2)) } }));

    dispatchCommand(mxComponent, ".uno:InsertTable", aArgs);

    xmlDocUniquePtr pLayout = parseLayoutDump();
    assertXPath(pLayout, "/root/page[1]/header/tab/row"_ostr, 2);

    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    // Delete the header
    pWrtShell->ChangeHeaderOrFooter(u"Default Page Style", true, false, false);

    discardDumpedLayout();
    pLayout = parseLayoutDump();
    assertXPath(pLayout, "/root/page[1]/header"_ostr, 0);

    dispatchCommand(mxComponent, ".uno:Undo", {});

    discardDumpedLayout();
    pLayout = parseLayoutDump();
    assertXPath(pLayout, "/root/page[1]/header/tab/row"_ostr, 2);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf141613)
{
    createSwDoc();
    SwDoc* const pDoc = getSwDoc();
    SwWrtShell* const pWrtSh = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtSh);

    pWrtSh->Insert("Test");

    dispatchCommand(mxComponent,
                    ".uno:InsertPageHeader?PageStyle:string=Default%20Page%20Style&On:bool=true",
                    {});

    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"),
                                                   uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xPageStyle, "HeaderIsOn"));
    CPPUNIT_ASSERT_EQUAL(OUString("Test"), getParagraph(1)->getString());

    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(xPageStyle, "HeaderIsOn"));
    CPPUNIT_ASSERT_EQUAL(OUString("Test"), getParagraph(1)->getString());

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(OUString(""), getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf107494)
{
    createSwDoc();

    // Create a graphic object, but don't insert it yet.
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xTextGraphic(
        xFactory->createInstance("com.sun.star.text.TextGraphicObject"), uno::UNO_QUERY);

    uno::Reference<text::XTextContent> xTextContent(xTextGraphic, uno::UNO_QUERY);

    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"),
                                                   uno::UNO_QUERY);

    xPageStyle->setPropertyValue("HeaderIsOn", uno::Any(true));

    uno::Reference<text::XText> xHeader(
        getProperty<uno::Reference<text::XText>>(xPageStyle, "HeaderText"));
    CPPUNIT_ASSERT(xHeader.is());
    uno::Reference<text::XTextCursor> xHeaderCursor(xHeader->createTextCursor());

    xHeader->insertTextContent(xHeaderCursor, xTextContent, false);

    CPPUNIT_ASSERT_EQUAL(1, getShapes());

    xPageStyle->setPropertyValue("HeaderIsOn", uno::Any(false));

    CPPUNIT_ASSERT_EQUAL(0, getShapes());

    xPageStyle->setPropertyValue("FooterIsOn", uno::Any(true));

    uno::Reference<text::XText> xFooter(
        getProperty<uno::Reference<text::XText>>(xPageStyle, "FooterText"));
    CPPUNIT_ASSERT(xFooter.is());
    uno::Reference<text::XTextCursor> xFooterCursor(xFooter->createTextCursor());

    xTextGraphic.set(xFactory->createInstance("com.sun.star.text.TextGraphicObject"),
                     uno::UNO_QUERY);

    xTextContent.set(xTextGraphic, uno::UNO_QUERY);

    xFooter->insertTextContent(xFooterCursor, xTextContent, false);

    CPPUNIT_ASSERT_EQUAL(1, getShapes());

    xPageStyle->setPropertyValue("FooterIsOn", uno::Any(false));

    CPPUNIT_ASSERT_EQUAL(0, getShapes());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf133358)
{
    createSwDoc();
    SwDoc* const pDoc = getSwDoc();
    SwWrtShell* const pWrtSh = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtSh);

    pWrtSh->Insert("Test");

    CPPUNIT_ASSERT_EQUAL(OUString("Test"), getParagraph(1)->getString());

    uno::Reference<beans::XPropertyState> xParagraph(getParagraph(1), uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xParagraph, "ParaLeftMargin"));

    dispatchCommand(mxComponent, ".uno:IncrementIndent", {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1251), getProperty<sal_Int32>(xParagraph, "ParaLeftMargin"));

    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xParagraph, "ParaLeftMargin"));

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, ".uno:Redo", {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1251), getProperty<sal_Int32>(xParagraph, "ParaLeftMargin"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf131771)
{
    createSwDoc();

    uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence(
        { { "Rows", uno::Any(sal_Int32(2)) }, { "Columns", uno::Any(sal_Int32(2)) } }));

    dispatchCommand(mxComponent, ".uno:InsertTable", aArgs);

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    uno::Reference<text::XTextTable> xTextTable(xIndexAccess->getByIndex(0), uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(OUString(""), getProperty<OUString>(xTextTable, "TableTemplateName"));
    uno::Reference<beans::XPropertySet> xTableProps(xTextTable, uno::UNO_QUERY_THROW);
    xTableProps->setPropertyValue("TableTemplateName", uno::Any(OUString("Default Style")));

    CPPUNIT_ASSERT_EQUAL(OUString("Default Style"),
                         getProperty<OUString>(xTextTable, "TableTemplateName"));

    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:Copy", {});
    dispatchCommand(mxComponent, ".uno:GoDown", {});
    dispatchCommand(mxComponent, ".uno:Paste", {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());

    CPPUNIT_ASSERT_EQUAL(OUString("Default Style"),
                         getProperty<OUString>(xTextTable, "TableTemplateName"));

    uno::Reference<text::XTextTable> xTextTable2(xIndexAccess->getByIndex(1), uno::UNO_QUERY);

    // Without the fix in place, this test would have failed with
    // - Expected: Default Style
    // - Actual  :
    CPPUNIT_ASSERT_EQUAL(OUString("Default Style"),
                         getProperty<OUString>(xTextTable2, "TableTemplateName"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf156546)
{
    createSwDoc();

    uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence(
        { { "Rows", uno::Any(sal_Int32(2)) }, { "Columns", uno::Any(sal_Int32(2)) } }));

    dispatchCommand(mxComponent, ".uno:InsertTable", aArgs);

    uno::Reference<text::XTextTablesSupplier> xTableSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTableSupplier->getTextTables(),
                                                    uno::UNO_QUERY);

    // check that table was created and inserted into the document
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:Copy", {});

    // create another document
    createSwDoc();
    dispatchCommand(mxComponent, ".uno:Paste", {});

    uno::Reference<text::XTextTablesSupplier> xTableSupplier2(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables2(xTableSupplier2->getTextTables(),
                                                     uno::UNO_QUERY);

    // check table exists after paste/undo
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables2->getCount());
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xTables2->getCount());

    // without the test, writer freezes on redo table paste into new doc
    dispatchCommand(mxComponent, ".uno:Redo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables2->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf80663)
{
    createSwDoc();

    uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence(
        { { "Rows", uno::Any(sal_Int32(2)) }, { "Columns", uno::Any(sal_Int32(2)) } }));

    dispatchCommand(mxComponent, ".uno:InsertTable", aArgs);

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTextTable->getColumns()->getCount());

    dispatchCommand(mxComponent, ".uno:DeleteRows", {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTextTable->getColumns()->getCount());

    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTextTable->getColumns()->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf130805)
{
    createSwDoc("tdf130805.odt");
    SwDoc* pDoc = getSwDoc();

    const auto& rFrmFormats = *pDoc->GetSpzFrameFormats();
    CPPUNIT_ASSERT(rFrmFormats.size() >= size_t(o3tl::make_unsigned(1)));
    auto pShape = rFrmFormats.front();
    CPPUNIT_ASSERT(pShape);

    SwTextBoxHelper::create(pShape, pShape->FindRealSdrObject());
    auto pTxBxFrm = SwTextBoxHelper::getOtherTextBoxFormat(getShape(1));
    CPPUNIT_ASSERT(pTxBxFrm);

    const SwNode* pTxAnch = pTxBxFrm->GetAnchor().GetAnchorNode();
    const SwNode* pShpAnch = pShape->GetAnchor().GetAnchorNode();
    CPPUNIT_ASSERT(pTxAnch);
    CPPUNIT_ASSERT(pShpAnch);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("The textbox got apart!", pTxAnch->GetIndex(),
                                 pShpAnch->GetIndex());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf107893)
{
    //Open the sample doc
    createSwDoc("tdf107893.odt");
    SwDoc* pDoc = getSwDoc();

    //Get the format of the shape
    const auto& rFrmFormats = *pDoc->GetSpzFrameFormats();
    CPPUNIT_ASSERT(rFrmFormats.size() >= size_t(o3tl::make_unsigned(1)));
    auto pShape = rFrmFormats.front();
    CPPUNIT_ASSERT(pShape);

    //Add a textbox
    SwTextBoxHelper::create(pShape, pShape->FindRealSdrObject());
    SwFrameFormat* pTxBxFrm = SwTextBoxHelper::getOtherTextBoxFormat(getShape(1));
    CPPUNIT_ASSERT(pTxBxFrm);

    //Remove the textbox using Undo
    dispatchCommand(mxComponent, ".uno:Undo", {});

    //Add again
    SwTextBoxHelper::create(pShape, pShape->FindRealSdrObject());
    pTxBxFrm = SwTextBoxHelper::getOtherTextBoxFormat(getShape(1));

    //This was nullptr because of unsuccessful re-adding
    CPPUNIT_ASSERT_MESSAGE("Textbox cannot be readd after Undo!", pTxBxFrm);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf121031)
{
    createSwDoc();

    uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence(
        { { "Rows", uno::Any(sal_Int32(3)) }, { "Columns", uno::Any(sal_Int32(3)) } }));

    dispatchCommand(mxComponent, ".uno:InsertTable", aArgs);

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    dispatchCommand(mxComponent, ".uno:DeleteTable", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xIndexAccess->getCount());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    // Without the fix in place, the table would be hidden
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/tab"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, TestTextBoxCrashAfterLineDel)
{
    // Open the desired file
    createSwDoc("txbx_crash.odt");
    SwDoc* pDoc = getSwDoc();

    // Get the format of the shape
    const auto& rFrmFormats = *pDoc->GetSpzFrameFormats();
    CPPUNIT_ASSERT(rFrmFormats.size() >= size_t(o3tl::make_unsigned(1)));
    auto pShape = rFrmFormats.front();
    CPPUNIT_ASSERT(pShape);

    // Add a textbox
    SwTextBoxHelper::create(pShape, pShape->FindRealSdrObject());
    SwFrameFormat* pTxBxFrm = SwTextBoxHelper::getOtherTextBoxFormat(getShape(1));
    CPPUNIT_ASSERT(pTxBxFrm);

    // remove the last paragraph
    auto xCursor = getParagraph(1)->getText()->createTextCursor();
    xCursor->gotoEnd(false);
    xCursor->goLeft(3, true);

    // This caused crash before, now it should pass with the patch.
    xCursor->setString(OUString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf146356)
{
    createSwDoc("tdf146356.odt");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());

    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_MOD2 | awt::Key::RETURN);
    Scheduler::ProcessEventsToIdle();

    emulateTyping(*pTextDoc, u"Some Text");

    // Without the fix in place, this test would have failed with
    // - Expected: Some Text
    // - Actual  : Table of Contents
    CPPUNIT_ASSERT_EQUAL(OUString("Some Text"), getParagraph(1)->getString());

    // tdf#160095: Without the fix in place, this test would have crashed here
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_MOD2 | awt::Key::RETURN);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(OUString("Some Text"), getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf121546)
{
    createSwDoc("tdf121546.odt");

    CPPUNIT_ASSERT_EQUAL(OUString("xxxxxxxxxxxxxxxxxxxx"), getParagraph(2)->getString());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    dispatchCommand(mxComponent, ".uno:Cut", {});

    CPPUNIT_ASSERT_EQUAL(1, getParagraphs());

    // Create a new document
    createSwDoc();

    dispatchCommand(mxComponent, ".uno:Paste", {});

    CPPUNIT_ASSERT_EQUAL(OUString("xxxxxxxxxxxxxxxxxxxx"), getParagraph(2)->getString());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    dispatchCommand(mxComponent, ".uno:Cut", {});

    CPPUNIT_ASSERT_EQUAL(1, getParagraphs());

    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT_EQUAL(OUString("xxxxxxxxxxxxxxxxxxxx"), getParagraph(2)->getString());

    dispatchCommand(mxComponent, ".uno:Undo", {});

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());

    // Without the fix in place, this test would have crashed here
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_RETURN);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(2, getParagraphs());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf145621)
{
    createSwDoc("tdf145621.odt");

    CPPUNIT_ASSERT_EQUAL(OUString("AAAAAA"), getParagraph(1)->getString());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    dispatchCommand(mxComponent, ".uno:Cut", {});

    CPPUNIT_ASSERT_EQUAL(OUString(""), getParagraph(1)->getString());

    // Without the fix in place, this test would have crashed
    dispatchCommand(mxComponent, ".uno:Paste", {});

    CPPUNIT_ASSERT_EQUAL(OUString("AAAAAA"), getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf134626)
{
    createSwDoc("tdf134626.odt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    CPPUNIT_ASSERT_EQUAL(OUString("Apple"), getParagraph(1)->getString());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    dispatchCommand(mxComponent, ".uno:Copy", {});

    // Create a new document
    createSwDoc();
    pDoc = getSwDoc();
    pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    // Without the fix in place, this test would have crashed here
    for (sal_Int32 i = 0; i < 5; ++i)
    {
        dispatchCommand(mxComponent, ".uno:Paste", {});

        CPPUNIT_ASSERT_EQUAL(OUString("Apple"), getParagraph(1)->getString());

        dispatchCommand(mxComponent, ".uno:Paste", {});

        CPPUNIT_ASSERT_EQUAL(OUString("AppleApple"), getParagraph(1)->getString());

        dispatchCommand(mxComponent, ".uno:Undo", {});

        CPPUNIT_ASSERT_EQUAL(OUString("Apple"), getParagraph(1)->getString());

        dispatchCommand(mxComponent, ".uno:Undo", {});

        CPPUNIT_ASSERT_EQUAL(OUString(""), getParagraph(1)->getString());
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf139566)
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtSh = pDoc->GetDocShell()->GetWrtShell();

    uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence(
        { { "Rows", uno::Any(sal_Int32(1)) }, { "Columns", uno::Any(sal_Int32(1)) } }));

    dispatchCommand(mxComponent, ".uno:InsertTable", aArgs);

    // Move the cursor outside the table
    pWrtSh->Down(/*bSelect=*/false);

    pWrtSh->Insert("Test");

    CPPUNIT_ASSERT_EQUAL(OUString("Test"), getParagraph(2)->getString());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    uno::Reference<frame::XFrames> xFrames = mxDesktop->getFrames();
    sal_Int32 nFrames = xFrames->getCount();

    // Create a second window so the first window looses focus
    dispatchCommand(mxComponent, ".uno:NewWindow", {});

    CPPUNIT_ASSERT_EQUAL(nFrames + 1, xFrames->getCount());

    dispatchCommand(mxComponent, ".uno:CloseWin", {});

    CPPUNIT_ASSERT_EQUAL(nFrames, xFrames->getCount());

    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xSelections(xModel->getCurrentSelection(),
                                                        uno::UNO_QUERY);

    // Without the fix in place, this test would have failed here
    CPPUNIT_ASSERT(xSelections.is());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf96067)
{
    createSwDoc();

    uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence(
        { { "Rows", uno::Any(sal_Int32(3)) }, { "Columns", uno::Any(sal_Int32(3)) } }));

    dispatchCommand(mxComponent, ".uno:InsertTable", aArgs);

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTextTable->getColumns()->getCount());

    dispatchCommand(mxComponent, ".uno:SelectTable", {});
    dispatchCommand(mxComponent, ".uno:InsertRowsBefore", {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTextTable->getColumns()->getCount());

    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTextTable->getColumns()->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf87199)
{
    createSwDoc();

    uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence(
        { { "Rows", uno::Any(sal_Int32(2)) }, { "Columns", uno::Any(sal_Int32(1)) } }));

    dispatchCommand(mxComponent, ".uno:InsertTable", aArgs);

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTextTable->getColumns()->getCount());

    uno::Reference<text::XTextRange> xCellA1(xTextTable->getCellByName("A1"), uno::UNO_QUERY);
    xCellA1->setString("test1");

    uno::Reference<text::XTextRange> xCellA2(xTextTable->getCellByName("A2"), uno::UNO_QUERY);
    xCellA2->setString("test2");

    dispatchCommand(mxComponent, ".uno:EntireColumn", {});
    dispatchCommand(mxComponent, ".uno:MergeCells", {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTextTable->getColumns()->getCount());

    CPPUNIT_ASSERT(xCellA1->getString().endsWith("test2"));

    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTextTable->getColumns()->getCount());

    xCellA1.set(xTextTable->getCellByName("A1"), uno::UNO_QUERY);

    CPPUNIT_ASSERT(xCellA1->getString().endsWith("test1"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf39828)
{
    createSwDoc("tdf39828.fodt");
    SwDoc* pDoc = getSwDoc();

    // show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be off",
                           !pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xIndexAccess->getByIndex(0), uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xTextTable->getRows()->getCount());

    uno::Reference<text::XTextRange> xCellA1(xTextTable->getCellByName("A1"), uno::UNO_QUERY);
    // deleted "1", inserted "2"
    CPPUNIT_ASSERT_EQUAL(OUString("12"), xCellA1->getString());
    uno::Reference<text::XTextRange> xCellA3(xTextTable->getCellByName("A3"), uno::UNO_QUERY);
    // This was 14 (bad sum: 2 + A1, where A1 was 12 instead of the correct 2)
    CPPUNIT_ASSERT_EQUAL(OUString("4"), xCellA3->getString());
    uno::Reference<text::XTextRange> xCellA4(xTextTable->getCellByName("A4"), uno::UNO_QUERY);
    // This was 28 (bad sum: 2 + A1 + A3, where A1 was 12 and A3 was 14)
    CPPUNIT_ASSERT_EQUAL(OUString("8"), xCellA4->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf146573)
{
    createSwDoc("tdf39828.fodt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    // remove redlines, add a footnote, and change the value
    // of the cell with the footnote
    dispatchCommand(mxComponent, ".uno:AcceptAllTrackedChanges", {});
    pWrtShell->Right(SwCursorSkipMode::Cells, /*bSelect=*/false, /*nCount=*/1,
                     /*bBasicCall=*/false);
    dispatchCommand(mxComponent, ".uno:InsertFootnote", {});
    dispatchCommand(mxComponent, ".uno:PageUp", {}); // leave footnote
    pWrtShell->Left(SwCursorSkipMode::Cells, /*bSelect=*/false, /*nCount=*/1, /*bBasicCall=*/false);
    pWrtShell->Left(SwCursorSkipMode::Cells, /*bSelect=*/true, /*nCount=*/1, /*bBasicCall=*/false);
    pWrtShell->Insert("100");

    // trigger recalculation by leaving the cell
    pWrtShell->Down(/*bSelect=*/false, /*nCount=*/1);

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xIndexAccess->getByIndex(0), uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xTextTable->getRows()->getCount());

    uno::Reference<text::XTextRange> xCellA1(xTextTable->getCellByName("A1"), uno::UNO_QUERY);
    // value "100" and footnote index "1"
    CPPUNIT_ASSERT_EQUAL(OUString("1001"), xCellA1->getString());
    uno::Reference<text::XTextRange> xCellA3(xTextTable->getCellByName("A3"), uno::UNO_QUERY);
    // This was 4 (missing recalculation)
    CPPUNIT_ASSERT_EQUAL(OUString("102"), xCellA3->getString());
    uno::Reference<text::XTextRange> xCellA4(xTextTable->getCellByName("A4"), uno::UNO_QUERY);
    // This was 8 (missing recalculation)
    CPPUNIT_ASSERT_EQUAL(OUString("204"), xCellA4->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf157132)
{
    createSwDoc("tdf157132.odt");

    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    // Go to cell A2
    pWrtShell->Down(/*bSelect=*/false, /*nCount=*/1);

    // Select A2 and A3 and copy
    pWrtShell->Down(/*bSelect=*/true, /*nCount=*/1);

    dispatchCommand(mxComponent, ".uno:Copy", {});

    // Go to A4 and paste
    pWrtShell->Down(/*bSelect=*/false, /*nCount=*/1);

    dispatchCommand(mxComponent, ".uno:Paste", {});

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTables->getCount());

    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);

    uno::Reference<text::XTextRange> xCellA2(xTextTable->getCellByName("A2"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("2"), xCellA2->getString());
    uno::Reference<text::XTextRange> xCellA3(xTextTable->getCellByName("A3"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("3"), xCellA3->getString());
    uno::Reference<text::XTextRange> xCellA4(xTextTable->getCellByName("A4"), uno::UNO_QUERY);

    // Without the fix in place, this test would have failed with
    // - Expected: 6
    // - Actual  : 2
    CPPUNIT_ASSERT_EQUAL(OUString("6"), xCellA4->getString());
    uno::Reference<text::XTextRange> xCellA5(xTextTable->getCellByName("A5"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("7"), xCellA5->getString());

    xTextTable.set(xTables->getByIndex(1), uno::UNO_QUERY);

    xCellA2.set(xTextTable->getCellByName("A2"), uno::UNO_QUERY);

    // tdf#158336: Without the fix in place, this test would have failed with
    // - Expected: 2
    // - Actual  : ** Expression is faulty **
    CPPUNIT_ASSERT_EQUAL(OUString("2"), xCellA2->getString());
    xCellA3.set(xTextTable->getCellByName("A3"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("3"), xCellA3->getString());
    xCellA4.set(xTextTable->getCellByName("A4"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("6"), xCellA4->getString());
    xCellA5.set(xTextTable->getCellByName("A5"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("7"), xCellA5->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf147938)
{
    createSwDoc("tdf147938.fodt");

    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->getIDocumentRedlineAccess().GetRedlineTable().size());
    CPPUNIT_ASSERT_EQUAL(OUString("Bar\nbaz "),
                         pDoc->getIDocumentRedlineAccess().GetRedlineTable()[0]->GetText());

    pWrtShell->Down(/*bSelect=*/false, /*nCount=*/1);
    pWrtShell->TableToText('\t');

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->getIDocumentRedlineAccess().GetRedlineTable().size());
    CPPUNIT_ASSERT_EQUAL(OUString("Bar\nbaz "),
                         pDoc->getIDocumentRedlineAccess().GetRedlineTable()[0]->GetText());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    SwInsertTableOptions const opts(SwInsertTableFlags::NONE, 0);
    pWrtShell->TextToTable(opts, '\t', nullptr);

    pWrtShell->Undo();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->getIDocumentRedlineAccess().GetRedlineTable().size());
    CPPUNIT_ASSERT_EQUAL(OUString("Bar\nbaz "),
                         pDoc->getIDocumentRedlineAccess().GetRedlineTable()[0]->GetText());

    pWrtShell->Undo();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->getIDocumentRedlineAccess().GetRedlineTable().size());
    CPPUNIT_ASSERT_EQUAL(OUString("Bar\nbaz "),
                         pDoc->getIDocumentRedlineAccess().GetRedlineTable()[0]->GetText());

    pWrtShell->Redo();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->getIDocumentRedlineAccess().GetRedlineTable().size());
    CPPUNIT_ASSERT_EQUAL(OUString("Bar\nbaz "),
                         pDoc->getIDocumentRedlineAccess().GetRedlineTable()[0]->GetText());

    pWrtShell->Redo();

    pWrtShell->Undo();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->getIDocumentRedlineAccess().GetRedlineTable().size());
    CPPUNIT_ASSERT_EQUAL(OUString("Bar\nbaz "),
                         pDoc->getIDocumentRedlineAccess().GetRedlineTable()[0]->GetText());

    pWrtShell->Undo();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->getIDocumentRedlineAccess().GetRedlineTable().size());
    CPPUNIT_ASSERT_EQUAL(OUString("Bar\nbaz "),
                         pDoc->getIDocumentRedlineAccess().GetRedlineTable()[0]->GetText());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf148799)
{
    // load a document with table formulas with comma delimiter,
    // but with a document language with default point delimiter
    createSwDoc("tdf148799.docx");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    // check formula update

    // put cursor in the first table row
    pWrtShell->Down(/*bSelect=*/false, /*nCount=*/1);

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());

    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), xTextTable->getRows()->getCount());

    // These were "** Expression is faulty **"

    uno::Reference<text::XTextRange> xCellA1(xTextTable->getCellByName("D3"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("2.3"), xCellA1->getString());
    uno::Reference<text::XTextRange> xCellA3(xTextTable->getCellByName("D4"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("2345"), xCellA3->getString());
    uno::Reference<text::XTextRange> xCellA4(xTextTable->getCellByName("D5"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("23684.5"), xCellA4->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf151993)
{
    // load a document with table formulas with comma delimiter
    // (with a document language with default comma delimiter)
    createSwDoc("tdf151993.docx");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    // check formula update

    // put cursor in the first table row
    pWrtShell->Down(/*bSelect=*/false, /*nCount=*/1);

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());

    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTextTable->getRows()->getCount());

    // This was 0
    uno::Reference<text::XTextRange> xCellA1(xTextTable->getCellByName("A2"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("30"), xCellA1->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf148849)
{
    // load a document with a table and an empty paragraph before the table
    createSwDoc("tdf148849.fodt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    // record changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    // hide changes
    dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
    CPPUNIT_ASSERT(pWrtShell->GetLayout()->IsHideRedlines());

    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());

    // put cursor in the first table row
    pWrtShell->Down(/*bSelect=*/false, /*nCount=*/1);

    // delete a table row
    pWrtShell->DeleteRow();

    // check cursor position

    // This was "", because the text cursor jumped to the start of the document
    // after deleting a table row instead of remaining in the next table row
    SwNode& rNode = pWrtShell->GetCursor()->GetPoint()->GetNode();
    CPPUNIT_ASSERT_EQUAL(OUString("Row 2"), rNode.GetTextNode()->GetText());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf150576)
{
    // load a document with a table and an empty paragraph before the table
    createSwDoc("tdf148849.fodt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    // record changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    // hide changes
    dispatchCommand(mxComponent, ".uno:ShowTrackedChanges", {});
    CPPUNIT_ASSERT(pWrtShell->GetLayout()->IsHideRedlines());

    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());

    // Check deletion of the first row, if the second row deleted already

    // put cursor in the second table row
    pWrtShell->Down(/*bSelect=*/false, /*nCount=*/2);
    SwNode& rNode = pWrtShell->GetCursor()->GetPoint()->GetNode();
    CPPUNIT_ASSERT_EQUAL(OUString("Row 2"), rNode.GetTextNode()->GetText());

    // delete the second table row
    pWrtShell->DeleteRow();

    // check cursor position (row 3)
    SwNode& rNode2 = pWrtShell->GetCursor()->GetPoint()->GetNode();
    CPPUNIT_ASSERT_EQUAL(OUString("Row 3"), rNode2.GetTextNode()->GetText());

    // put cursor in the first row
    pWrtShell->Up(/*bSelect=*/false, /*nCount=*/1);
    SwNode& rNode3 = pWrtShell->GetCursor()->GetPoint()->GetNode();
    CPPUNIT_ASSERT_EQUAL(OUString("12"), rNode3.GetTextNode()->GetText());

    // delete the first row
    pWrtShell->DeleteRow();

    // This was empty (cursor jumped in the start of the document instead of
    // the next not deleted row)
    SwNode& rNode4 = pWrtShell->GetCursor()->GetPoint()->GetNode();
    CPPUNIT_ASSERT_EQUAL(OUString("Row 3"), rNode4.GetTextNode()->GetText());

    // Check skipping previous lines

    // restore deleted rows
    dispatchCommand(mxComponent, ".uno:Undo", {});
    dispatchCommand(mxComponent, ".uno:Undo", {});
    SwNode& rNode5 = pWrtShell->GetCursor()->GetPoint()->GetNode();
    CPPUNIT_ASSERT_EQUAL(OUString("Row 2"), rNode5.GetTextNode()->GetText());

    // delete the second row
    pWrtShell->DeleteRow();
    SwNode& rNode7 = pWrtShell->GetCursor()->GetPoint()->GetNode();
    CPPUNIT_ASSERT_EQUAL(OUString("Row 3"), rNode7.GetTextNode()->GetText());

    // delete the third, i.e. last row
    pWrtShell->DeleteRow();
    SwNode& rNode8 = pWrtShell->GetCursor()->GetPoint()->GetNode();

    // This was empty (cursor jumped in the start of the document instead of
    // the previous not deleted row)
    CPPUNIT_ASSERT_EQUAL(OUString("12"), rNode8.GetTextNode()->GetText());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf132603)
{
    createSwDoc();
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());

    uno::Sequence<beans::PropertyValue> aPropertyValues
        = comphelper::InitPropertySequence({ { "Text", uno::Any(OUString("Comment")) } });

    dispatchCommand(mxComponent, ".uno:InsertAnnotation", aPropertyValues);

    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    // Without the fix in place, it would crash here
    dispatchCommand(mxComponent, ".uno:Copy", {});

    tools::JsonWriter aJsonWriter;
    pTextDoc->getPostIts(aJsonWriter);
    OString pChar = aJsonWriter.finishAndGetAsOString();
    std::stringstream aStream((std::string(pChar)));
    boost::property_tree::ptree aTree;
    boost::property_tree::read_json(aStream, aTree);
    for (const boost::property_tree::ptree::value_type& rValue : aTree.get_child("comments"))
    {
        const boost::property_tree::ptree& rComment = rValue.second;

        OString aText(rComment.get<std::string>("text"));
        CPPUNIT_ASSERT_EQUAL("Comment"_ostr, aText);
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf117601)
{
    createSwDoc();
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());

    uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence(
        { { "Rows", uno::Any(sal_Int32(5)) }, { "Columns", uno::Any(sal_Int32(3)) } }));

    dispatchCommand(mxComponent, ".uno:InsertTable", aArgs);

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTextTable->getColumns()->getCount());

    uno::Reference<text::XTextRange> xCellB1(xTextTable->getCellByName("B1"), uno::UNO_QUERY);
    xCellB1->setString("test1");

    uno::Reference<text::XTextRange> xCellB2(xTextTable->getCellByName("B2"), uno::UNO_QUERY);
    xCellB2->setString("test2");

    //go to middle row
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_UP);
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_RIGHT);
    Scheduler::ProcessEventsToIdle();

    dispatchCommand(mxComponent, ".uno:EntireColumn", {});
    dispatchCommand(mxComponent, ".uno:MergeCells", {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTextTable->getColumns()->getCount());

    CPPUNIT_ASSERT(xCellB1->getString().endsWith("test2"));

    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTextTable->getColumns()->getCount());

    CPPUNIT_ASSERT(xCellB1->getString().endsWith("test1"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf138130)
{
    createSwDoc("tdf138130.docx");

    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    uno::Reference<drawing::XShape> xShape = getShape(1);

    awt::Point aPos = xShape->getPosition();

    //select shape and change the anchor
    selectShape(1);

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, ".uno:SetAnchorToPage", {});

    //position has changed
    CPPUNIT_ASSERT(aPos.X < xShape->getPosition().X);
    CPPUNIT_ASSERT(aPos.Y < xShape->getPosition().Y);

    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT_EQUAL(aPos.X, xShape->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(aPos.Y, xShape->getPosition().Y);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf136385)
{
    createSwDoc("tdf136385.odt");

    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    uno::Reference<drawing::XShape> xShape = getShape(1);

    awt::Point aPos = xShape->getPosition();

    //select shape and change the anchor
    selectShape(1);

    dispatchCommand(mxComponent, ".uno:SetAnchorToPage", {});

    //position has changed
    CPPUNIT_ASSERT(aPos.X < xShape->getPosition().X);
    CPPUNIT_ASSERT(aPos.Y < xShape->getPosition().Y);

    dispatchCommand(mxComponent, ".uno:Undo", {});

    //Without the fix in place, this test would have failed with
    //- Expected: 2447
    //- Actual  : 446
    CPPUNIT_ASSERT_EQUAL(aPos.X, xShape->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(aPos.Y, xShape->getPosition().Y);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf145207)
{
    createSwDoc("tdf145207.odt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());

    CPPUNIT_ASSERT_EQUAL(1, getPages());
    CPPUNIT_ASSERT_EQUAL(3, getShapes());

    //select one shape and use the TAB key to iterate over the different shapes
    selectShape(1);

    for (sal_Int32 i = 0; i < 10; ++i)
    {
        // Without the fix in place, this test would have crashed here
        pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
        Scheduler::ProcessEventsToIdle();
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf128782)
{
    createSwDoc("tdf128782.odt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());

    CPPUNIT_ASSERT_EQUAL(2, getShapes());
    uno::Reference<drawing::XShape> xShape1 = getShape(1);
    uno::Reference<drawing::XShape> xShape2 = getShape(2);

    awt::Point aPos[2];
    aPos[0] = xShape1->getPosition();
    aPos[1] = xShape2->getPosition();

    //select shape 2 and move it down
    selectShape(2);

    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_DOWN);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(aPos[0].X, xShape1->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(aPos[0].Y, xShape1->getPosition().Y);
    CPPUNIT_ASSERT_EQUAL(aPos[1].X, xShape2->getPosition().X);
    //Y position in shape 2 has changed
    CPPUNIT_ASSERT(aPos[1].Y < xShape2->getPosition().Y);

    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT_EQUAL(aPos[0].X, xShape1->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(aPos[0].Y, xShape1->getPosition().Y);
    CPPUNIT_ASSERT_EQUAL(aPos[1].X, xShape2->getPosition().X);
    // Shape2 has come back to the original position
    // without the fix in place, it would have failed
    CPPUNIT_ASSERT_EQUAL(aPos[1].Y, xShape2->getPosition().Y);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf135623)
{
    createSwDoc("tdf135623.docx");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());

    CPPUNIT_ASSERT_EQUAL(2, getShapes());
    CPPUNIT_ASSERT_EQUAL(2, getPages());

    uno::Reference<drawing::XShape> xShape1 = getShape(1);
    uno::Reference<drawing::XShape> xShape2 = getShape(2);

    awt::Point aPos[2];
    aPos[0] = xShape1->getPosition();
    aPos[1] = xShape2->getPosition();

    //select shape 1 and move it down
    selectShape(1);

    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_DOWN);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(aPos[0].X, xShape1->getPosition().X);
    //Y position in shape 1 has changed
    CPPUNIT_ASSERT(aPos[0].Y < xShape1->getPosition().Y);
    CPPUNIT_ASSERT_EQUAL(aPos[1].X, xShape2->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(aPos[1].Y, xShape2->getPosition().Y);

    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT_EQUAL(aPos[0].X, xShape1->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(aPos[0].Y, xShape1->getPosition().Y);
    CPPUNIT_ASSERT_EQUAL(aPos[1].X, xShape2->getPosition().X);

    // Without the fix in place, this test would have failed here
    // - Expected: 1351
    // - Actual  : 2233
    CPPUNIT_ASSERT_EQUAL(aPos[1].Y, xShape2->getPosition().Y);

    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf133490)
{
    createSwDoc("tdf133490.odt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());

    CPPUNIT_ASSERT_EQUAL(1, getShapes());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    dispatchCommand(mxComponent, ".uno:Cut", {});

    CPPUNIT_ASSERT_EQUAL(0, getShapes());

    dispatchCommand(mxComponent, ".uno:Paste", {});

    CPPUNIT_ASSERT_EQUAL(1, getShapes());

    dispatchCommand(mxComponent, ".uno:Paste", {});

    CPPUNIT_ASSERT_EQUAL(2, getShapes());

    uno::Reference<drawing::XShape> xShape1 = getShape(1);
    uno::Reference<drawing::XShape> xShape2 = getShape(2);

    awt::Point aPos[2];
    aPos[0] = xShape1->getPosition();
    aPos[1] = xShape2->getPosition();

    //select shape 2 and move it to the right
    selectShape(2);

    for (sal_Int32 i = 0; i < 5; ++i)
    {
        pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_RIGHT);
        Scheduler::ProcessEventsToIdle();
    }

    CPPUNIT_ASSERT_EQUAL(aPos[0].X, xShape1->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(aPos[0].Y, xShape1->getPosition().Y);
    //X position in shape 2 has changed
    CPPUNIT_ASSERT(aPos[1].X < xShape2->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(aPos[1].Y, xShape2->getPosition().Y);

    for (sal_Int32 i = 0; i < 4; ++i)
    {
        dispatchCommand(mxComponent, ".uno:Undo", {});

        // Without the fix in place, undo action would have changed shape1's position
        // and this test would have failed with
        // - Expected: -139
        // - Actual  : 1194
        CPPUNIT_ASSERT_EQUAL(aPos[0].X, xShape1->getPosition().X);
        CPPUNIT_ASSERT_EQUAL(aPos[0].Y, xShape1->getPosition().Y);
        CPPUNIT_ASSERT(aPos[1].X < xShape2->getPosition().X);
        CPPUNIT_ASSERT_EQUAL(aPos[1].Y, xShape2->getPosition().Y);
    }

    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT_EQUAL(aPos[0].X, xShape1->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(aPos[0].Y, xShape1->getPosition().Y);
    // Shape 2 has come back to the original position
    CPPUNIT_ASSERT_EQUAL(aPos[1].X, xShape2->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(aPos[1].Y, xShape2->getPosition().Y);

    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT_EQUAL(1, getShapes());

    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT_EQUAL(0, getShapes());

    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT_EQUAL(1, getShapes());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf132637_protectTrackChanges)
{
    createSwDoc("tdf132637_protectTrackChanges.doc");
    SwDoc* pDoc = getSwDoc();

    // The password should only prevent turning off track changes, not open as read-only
    CPPUNIT_ASSERT(!pDoc->GetDocShell()->IsReadOnly());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf127652)
{
    createSwDoc("tdf127652.odt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* const pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    // get a page cursor
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(),
                                              uno::UNO_QUERY);

    // go to the start of page 4
    xCursor->jumpToPage(4);
    xCursor->jumpToStartOfPage();

    // mark a section that overlaps multiple pages
    pWrtShell->Down(false, 2);
    pWrtShell->Up(true, 5);

    // delete the marked section
    pWrtShell->DelRight();

    // go to the start of page 4
    xCursor->jumpToPage(4);
    xCursor->jumpToStartOfPage();

    // move up to page 3
    pWrtShell->Up(false, 5);

    // check that we are on the third page
    // in the bug one issue was that the cursor was placed incorrectly, so
    // moving up to the previous page would not work any more
    sal_uInt16 assertPage = 3;
    SwCursorShell* pShell(pDoc->GetEditShell());
    sal_uInt16 currentPage = pShell->GetPageNumSeqNonEmpty();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("We are on the wrong page!", assertPage, currentPage);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, AtPageTextBoxCrash)
{
    // Load sample file
    createSwDoc("AtPageTextBoxCrash.odt");
    SwDoc* pDoc = getSwDoc();

    // Get the format of the shape
    const auto& rFrmFormats = *pDoc->GetSpzFrameFormats();
    CPPUNIT_ASSERT(rFrmFormats.size() >= size_t(o3tl::make_unsigned(1)));
    auto pShape = rFrmFormats.front();
    CPPUNIT_ASSERT(pShape);

    // Add a textbox to the shape
    SwTextBoxHelper::create(pShape, pShape->FindRealSdrObject());
    auto pTxBxFrm = SwTextBoxHelper::getOtherTextBoxFormat(getShape(1));
    CPPUNIT_ASSERT(pTxBxFrm);

    // Change its anchor to page
    uno::Reference<beans::XPropertySet> xShpProps(getShape(1), uno::UNO_QUERY_THROW);
    xShpProps->setPropertyValue(
        "AnchorType", uno::Any(text::TextContentAnchorType::TextContentAnchorType_AT_PAGE));

    // The page anchored objects must not have content anchor
    // unless this will lead to crash later, for example on
    // removing the paragraph where it is anchored to...
    CPPUNIT_ASSERT_EQUAL(RndStdIds::FLY_AT_PAGE, pTxBxFrm->GetAnchor().GetAnchorId());
    CPPUNIT_ASSERT(!pTxBxFrm->GetAnchor().GetAnchorNode());

    // Remove the paragraph where the textframe should be anchored
    // before. Now with the patch it must not crash...
    auto xPara = getParagraph(1);
    xPara->getText()->setString(OUString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf135661)
{
    createSwDoc("tdf135661.odt");

    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    uno::Reference<drawing::XShape> xShape(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3424), xShape->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1545), xShape->getPosition().Y);

    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:Cut", {});

    CPPUNIT_ASSERT_EQUAL(0, getShapes());

    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT_EQUAL(1, getShapes());

    xShape.set(getShape(1), uno::UNO_QUERY);

    //Without the fix in place, the shape position would have been 0,0
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3424), xShape->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1545), xShape->getPosition().Y);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf133477)
{
    if (getDefaultDeviceBitCount() < 24)
        return;
    createSwDoc("tdf133477.fodt");

    // Save the shape to a BMP.
    uno::Reference<drawing::XGraphicExportFilter> xGraphicExporter
        = drawing::GraphicExportFilter::create(mxComponentContext);
    uno::Reference<lang::XComponent> xSourceDoc(getShape(1), uno::UNO_QUERY);
    xGraphicExporter->setSourceDocument(xSourceDoc);

    SvMemoryStream aStream;
    uno::Reference<io::XOutputStream> xOutputStream(new utl::OStreamWrapper(aStream));
    uno::Sequence<beans::PropertyValue> aDescriptor(
        comphelper::InitPropertySequence({ { "OutputStream", uno::Any(xOutputStream) },
                                           { "FilterName", uno::Any(OUString("BMP")) } }));
    xGraphicExporter->filter(aDescriptor);
    aStream.Seek(STREAM_SEEK_TO_BEGIN);

    // Read it back and check the color of the first pixel.
    // (Actually check at one-pixel offset, because imprecise shape positioning may
    // result in blending with background for the first pixel).
    Graphic aGraphic;
    TypeSerializer aSerializer(aStream);
    aSerializer.readGraphic(aGraphic);

    BitmapEx aBitmap = aGraphic.GetBitmapEx();
    CPPUNIT_ASSERT_EQUAL(Color(0, 102, 204), aBitmap.GetPixelColor(1, 1));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf137964)
{
    createSwDoc("tdf137964.odt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());

    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    uno::Reference<drawing::XShape> xShape(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3579), xShape->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4090), xShape->getPosition().Y);

    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pObject = pPage->GetObj(1);
    SwContact* pTextBox = static_cast<SwContact*>(pObject->GetUserCall());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(RES_FLYFRMFMT), pTextBox->GetFormat()->Which());

    pWrtShell->SelectObj(Point(), 0, pObject);

    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_SHIFT | KEY_UP);
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_SHIFT | KEY_LEFT);
    Scheduler::ProcessEventsToIdle();

    // Without the fix in place, the shape would have stayed where it was
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2579), xShape->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3090), xShape->getPosition().Y);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf143244)
{
    createSwDoc("tdf143244.odt");

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTextTable->getColumns()->getCount());

    uno::Reference<text::XTextRange> xCell(xTextTable->getCellByName("A1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(Color(0x009353), getProperty<Color>(xCell, "BackColor"));

    xCell.set(xTextTable->getCellByName("A2"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(COL_AUTO, getProperty<Color>(xCell, "BackColor"));

    xCell.set(xTextTable->getCellByName("A3"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(Color(0xdddddd), getProperty<Color>(xCell, "BackColor"));

    xCell.set(xTextTable->getCellByName("A4"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(COL_AUTO, getProperty<Color>(xCell, "BackColor"));

    xCell.set(xTextTable->getCellByName("A5"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(Color(0xdddddd), getProperty<Color>(xCell, "BackColor"));

    xCell.set(xTextTable->getCellByName("A6"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(Color(0xbee3d3), getProperty<Color>(xCell, "BackColor"));

    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:Cut", {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xIndexAccess->getCount());

    dispatchCommand(mxComponent, ".uno:Paste", {});

    xTextTable.set(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTextTable->getColumns()->getCount());

    dispatchCommand(mxComponent, ".uno:GoUp", {});

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    for (sal_Int32 i = 0; i < 6; ++i)
    {
        pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
        Scheduler::ProcessEventsToIdle();
    }

    for (sal_Int32 i = 0; i < 5; ++i)
    {
        dispatchCommand(mxComponent, ".uno:Undo", {});
    }

    xTextTable.set(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTextTable->getColumns()->getCount());

    for (sal_Int32 i = 0; i < 5; ++i)
    {
        dispatchCommand(mxComponent, ".uno:Redo", {});
    }

    xTextTable.set(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(9), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTextTable->getColumns()->getCount());

    xCell.set(xTextTable->getCellByName("A1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(Color(0x009353), getProperty<Color>(xCell, "BackColor"));

    xCell.set(xTextTable->getCellByName("A2"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(COL_AUTO, getProperty<Color>(xCell, "BackColor"));

    xCell.set(xTextTable->getCellByName("A3"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(Color(0xdddddd), getProperty<Color>(xCell, "BackColor"));

    xCell.set(xTextTable->getCellByName("A4"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(COL_AUTO, getProperty<Color>(xCell, "BackColor"));

    xCell.set(xTextTable->getCellByName("A5"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(Color(0xdddddd), getProperty<Color>(xCell, "BackColor"));

    xCell.set(xTextTable->getCellByName("A6"), uno::UNO_QUERY);

    // Without the fix in place, this test would have failed with
    // - Expected: Color: R:255 G:255 B:255 A:255
    // - Actual  : Color: R:190 G:227 B:211 A:0
    CPPUNIT_ASSERT_EQUAL(COL_AUTO, getProperty<Color>(xCell, "BackColor"));

    xCell.set(xTextTable->getCellByName("A7"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(Color(0xdddddd), getProperty<Color>(xCell, "BackColor"));

    xCell.set(xTextTable->getCellByName("A8"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(COL_AUTO, getProperty<Color>(xCell, "BackColor"));

    xCell.set(xTextTable->getCellByName("A9"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(Color(0xbee3d3), getProperty<Color>(xCell, "BackColor"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf136715)
{
    createSwDoc("tdf136715.odt");

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTextTable->getColumns()->getCount());

    uno::Reference<text::XTextRange> xCell(xTextTable->getCellByName("A1"), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xCell->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(xPara, "CharWeight"));

    xCell.set(xTextTable->getCellByName("A2"), uno::UNO_QUERY);
    xParaEnumAccess.set(xCell->getText(), uno::UNO_QUERY);
    xParaEnum.set(xParaEnumAccess->createEnumeration());
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::NORMAL, getProperty<float>(xPara, "CharWeight"));

    xCell.set(xTextTable->getCellByName("A3"), uno::UNO_QUERY);
    xParaEnumAccess.set(xCell->getText(), uno::UNO_QUERY);
    xParaEnum.set(xParaEnumAccess->createEnumeration());
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::NORMAL, getProperty<float>(xPara, "CharWeight"));

    xCell.set(xTextTable->getCellByName("A4"), uno::UNO_QUERY);
    xParaEnumAccess.set(xCell->getText(), uno::UNO_QUERY);
    xParaEnum.set(xParaEnumAccess->createEnumeration());
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(xPara, "CharWeight"));

    dispatchCommand(mxComponent, ".uno:GoDown", {});
    dispatchCommand(mxComponent, ".uno:GoDown", {});
    dispatchCommand(mxComponent, ".uno:LineDownSel", {});
    dispatchCommand(mxComponent, ".uno:DeleteRows", {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTextTable->getColumns()->getCount());

    xCell.set(xTextTable->getCellByName("A1"), uno::UNO_QUERY);
    xParaEnumAccess.set(xCell->getText(), uno::UNO_QUERY);
    xParaEnum.set(xParaEnumAccess->createEnumeration());
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(xPara, "CharWeight"));

    xCell.set(xTextTable->getCellByName("A2"), uno::UNO_QUERY);
    xParaEnumAccess.set(xCell->getText(), uno::UNO_QUERY);
    xParaEnum.set(xParaEnumAccess->createEnumeration());
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(xPara, "CharWeight"));

    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTextTable->getColumns()->getCount());

    xCell.set(xTextTable->getCellByName("A1"), uno::UNO_QUERY);
    xParaEnumAccess.set(xCell->getText(), uno::UNO_QUERY);
    xParaEnum.set(xParaEnumAccess->createEnumeration());
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(xPara, "CharWeight"));

    xCell.set(xTextTable->getCellByName("A2"), uno::UNO_QUERY);
    xParaEnumAccess.set(xCell->getText(), uno::UNO_QUERY);
    xParaEnum.set(xParaEnumAccess->createEnumeration());
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);

    // Without the fix in place, this test would have failed with
    // - Expected: 100
    // - Actual  : 150
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::NORMAL, getProperty<float>(xPara, "CharWeight"));

    xCell.set(xTextTable->getCellByName("A3"), uno::UNO_QUERY);
    xParaEnumAccess.set(xCell->getText(), uno::UNO_QUERY);
    xParaEnum.set(xParaEnumAccess->createEnumeration());
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::NORMAL, getProperty<float>(xPara, "CharWeight"));

    xCell.set(xTextTable->getCellByName("A4"), uno::UNO_QUERY);
    xParaEnumAccess.set(xCell->getText(), uno::UNO_QUERY);
    xParaEnum.set(xParaEnumAccess->createEnumeration());
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(xPara, "CharWeight"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf138897)
{
    createSwDoc("tdf100018-1.odt");

    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:Cut", {});
    dispatchCommand(mxComponent, ".uno:Paste", {});
    // this was crashing
    dispatchCommand(mxComponent, ".uno:Undo", {});
    dispatchCommand(mxComponent, ".uno:Redo", {});
    dispatchCommand(mxComponent, ".uno:Undo", {});
    dispatchCommand(mxComponent, ".uno:Redo", {});
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf136740)
{
    createSwDoc();
    css::uno::Reference<css::lang::XMultiServiceFactory> xFact(mxComponent,
                                                               css::uno::UNO_QUERY_THROW);
    css::uno::Reference<css::beans::XPropertySet> xTextDefaults(
        xFact->createInstance("com.sun.star.text.Defaults"), css::uno::UNO_QUERY_THROW);
    const css::uno::Any aOrig = xTextDefaults->getPropertyValue("TabStopDistance");
    sal_Int32 nDefTab = aOrig.get<sal_Int32>();
    CPPUNIT_ASSERT(nDefTab != 0);

    css::uno::Reference<css::text::XTextRange> const xParagraph(getParagraphOrTable(1),
                                                                css::uno::UNO_QUERY_THROW);
    xParagraph->setString("Foo");

    CPPUNIT_ASSERT_EQUAL(1, getParagraphs());
    CPPUNIT_ASSERT_EQUAL(OUString("Foo"), xParagraph->getString());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:Copy", {});
    dispatchCommand(mxComponent, ".uno:GoToEndOfDoc", {});

    const css::uno::Any aNew(nDefTab * 2);
    xTextDefaults->setPropertyValue("TabStopDistance", aNew);
    // it may become slightly different because of conversions, so get the actual value
    const css::uno::Any aNewCorrected = xTextDefaults->getPropertyValue("TabStopDistance");
    CPPUNIT_ASSERT_DOUBLES_EQUAL(nDefTab * 2, aNewCorrected.get<sal_Int32>(), 1);

    // Paste special as RTF
    const auto aPropertyValues = comphelper::InitPropertySequence(
        { { "SelectedFormat",
            css::uno::Any(static_cast<sal_uInt32>(SotClipboardFormatId::RTF)) } });
    dispatchCommand(mxComponent, ".uno:ClipboardFormatItems", aPropertyValues);

    CPPUNIT_ASSERT_EQUAL(1, getParagraphs());
    CPPUNIT_ASSERT_EQUAL(OUString("FooFoo"), xParagraph->getString());

    // Without the fix in place, this would fail with
    //     equality assertion failed
    //     - Expected: <Any: (long) 2501>
    //     - Actual  : <Any: (long) 1251>
    // i.e., pasting RTF would reset the modified default tab stop distance to hardcoded default
    CPPUNIT_ASSERT_EQUAL(aNewCorrected, xTextDefaults->getPropertyValue("TabStopDistance"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf128106)
{
    createSwDoc("cross_reference_demo_bmk.odt");

    const auto aPropertyValues
        = comphelper::InitPropertySequence({ { "FileName", css::uno::Any(maTempFile.GetURL()) } });
    dispatchCommand(mxComponent, ".uno:NewGlobalDoc", aPropertyValues);

    mxComponent = loadFromDesktop(maTempFile.GetURL());

    SwWrtShell* pWrtShell = getSwDoc()->GetDocShell()->GetWrtShell();
    SwDoc* const pMasterDoc(pWrtShell->GetDoc());
    CPPUNIT_ASSERT_EQUAL(
        size_t(2),
        pMasterDoc->getIDocumentLinksAdministration().GetLinkManager().GetLinks().size());
    // no way to set SwDocShell::m_nUpdateDocMode away from NO_UPDATE ?
    // pMasterDoc->getIDocumentLinksAdministration().UpdateLinks();
    pMasterDoc->getIDocumentLinksAdministration().GetLinkManager().UpdateAllLinks(false, false,
                                                                                  nullptr);
    // note: this has called SwGetRefFieldType::UpdateGetReferences()
    SwFieldType const* const pType(
        pMasterDoc->getIDocumentFieldsAccess().GetSysFieldType(SwFieldIds::GetRef));
    std::vector<SwFormatField*> fields;
    pType->GatherFields(fields);
    CPPUNIT_ASSERT_EQUAL(size_t(6), fields.size());
    std::sort(fields.begin(), fields.end(), [](auto const* const pA, auto const* const pB) {
        SwTextField const* const pHintA(pA->GetTextField());
        SwTextField const* const pHintB(pB->GetTextField());
        // in this document: only 1 field per node
        CPPUNIT_ASSERT(pA == pB || &pHintA->GetTextNode() != &pHintB->GetTextNode());
        return pHintA->GetTextNode().GetIndex() < pHintB->GetTextNode().GetIndex();
    });
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_BOOKMARK), fields[0]->GetField()->GetSubType());
    CPPUNIT_ASSERT_EQUAL(OUString("bookmarkchapter1_text"),
                         static_cast<SwGetRefField const*>(fields[0]->GetField())->GetSetRefName());
    CPPUNIT_ASSERT_EQUAL(OUString("Text"), static_cast<SwGetRefField const*>(fields[0]->GetField())
                                               ->GetExpandedTextOfReferencedTextNode(
                                                   *pWrtShell->GetLayout(), nullptr, nullptr));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_BOOKMARK), fields[1]->GetField()->GetSubType());
    CPPUNIT_ASSERT(
        static_cast<SwGetRefField const*>(fields[1]->GetField())->IsRefToHeadingCrossRefBookmark());
    CPPUNIT_ASSERT_EQUAL(OUString("Chapter 2"),
                         static_cast<SwGetRefField const*>(fields[1]->GetField())->GetPar2());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_BOOKMARK), fields[2]->GetField()->GetSubType());
    CPPUNIT_ASSERT_EQUAL(OUString("Bookmarkchapter1"),
                         static_cast<SwGetRefField const*>(fields[2]->GetField())->GetSetRefName());
    CPPUNIT_ASSERT_EQUAL(OUString("Chapter 1"),
                         static_cast<SwGetRefField const*>(fields[2]->GetField())->GetPar2());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_BOOKMARK), fields[3]->GetField()->GetSubType());
    CPPUNIT_ASSERT_EQUAL(OUString("bookmarkchapter1_text"),
                         static_cast<SwGetRefField const*>(fields[3]->GetField())->GetSetRefName());
    CPPUNIT_ASSERT_EQUAL(OUString("Text"),
                         static_cast<SwGetRefField const*>(fields[3]->GetField())->GetPar2());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_BOOKMARK), fields[4]->GetField()->GetSubType());
    CPPUNIT_ASSERT(
        static_cast<SwGetRefField const*>(fields[4]->GetField())->IsRefToHeadingCrossRefBookmark());
    CPPUNIT_ASSERT_EQUAL(OUString("Chapter 1.1"),
                         static_cast<SwGetRefField const*>(fields[4]->GetField())->GetPar2());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_BOOKMARK), fields[5]->GetField()->GetSubType());
    CPPUNIT_ASSERT(
        static_cast<SwGetRefField const*>(fields[5]->GetField())->IsRefToHeadingCrossRefBookmark());
    CPPUNIT_ASSERT_EQUAL(OUString("Chapter 2"),
                         static_cast<SwGetRefField const*>(fields[5]->GetField())->GetPar2());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf103612)
{
    createSwGlobalDoc("DUMMY.odm");
    SwDoc* pGlobalDoc = getSwDoc();
    CPPUNIT_ASSERT_EQUAL(
        size_t(1),
        pGlobalDoc->getIDocumentLinksAdministration().GetLinkManager().GetLinks().size());
    pGlobalDoc->getIDocumentLinksAdministration().GetLinkManager().UpdateAllLinks(false, false,
                                                                                  nullptr);

    xmlDocUniquePtr pLayout = parseLayoutDump();

    assertXPath(pLayout, "/root/page[1]/body/section[1]/txt[1]/SwParaPortion/SwLineLayout[1]"_ostr,
                "portion"_ostr, "Text before section");
    // the inner section and its content was hidden
    assertXPath(pLayout, "/root/page[1]/body/section[2]/txt[1]/SwParaPortion/SwLineLayout[1]"_ostr,
                "portion"_ostr, "Text inside section before ToC");
    assertXPath(pLayout, "/root/page[1]/body/section[3]/txt[1]/SwParaPortion/SwLineLayout[1]"_ostr,
                "portion"_ostr, "Table of Contents");
    assertXPath(pLayout, "/root/page[1]/body/section[4]/txt[1]/SwParaPortion/SwLineLayout[1]"_ostr,
                "portion"_ostr, "First header*1");
    assertXPath(pLayout, "/root/page[1]/body/section[4]/txt[2]/SwParaPortion/SwLineLayout[1]"_ostr,
                "portion"_ostr, "Second header*1");
    assertXPath(pLayout, "/root/page[1]/body/section[5]/txt[2]/SwParaPortion/SwLineLayout[1]"_ostr,
                "portion"_ostr, "Text inside section after ToC");
    assertXPath(pLayout, "/root/page[1]/body/section[6]/txt[1]/SwParaPortion/SwLineLayout[1]"_ostr,
                "portion"_ostr, "Text after section");
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf97899)
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwPaM* pCursor = pDoc->GetEditShell()->GetCursor();
    IDocumentContentOperations& rIDCO(pDoc->getIDocumentContentOperations());

    // Create an Ordered List
    rIDCO.InsertString(*pCursor, "\ta");
    pWrtShell->SplitNode();
    rIDCO.InsertString(*pCursor, "   b");
    pWrtShell->SplitNode();
    rIDCO.InsertString(*pCursor, "  \t  c");

    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:DefaultNumbering", {});

    // tdf#109285: RemoveLeadingWhiteSpace from all numbered paragraphs
    getParagraph(1, "a");
    getParagraph(2, "b");
    getParagraph(3, "c");

    // Save it as DOCX & load it again
    saveAndReload("Office Open XML Text");
    uno::Reference<container::XIndexAccess> xNumberingRules
        = getProperty<uno::Reference<container::XIndexAccess>>(getParagraph(1), "NumberingRules");
    CPPUNIT_ASSERT(xNumberingRules->getCount());
    uno::Sequence<beans::PropertyValue> aNumbering;
    xNumberingRules->getByIndex(0) >>= aNumbering;
    OUString sCharStyleName;
    for (const auto& prop : aNumbering)
    {
        if (prop.Name == "CharStyleName")
        {
            prop.Value >>= sCharStyleName;
            break;
        }
    }
    CPPUNIT_ASSERT(!sCharStyleName.isEmpty());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf40142)
{
    createSwDoc("tdf40142.odt");
    dispatchCommand(mxComponent, ".uno:UpdateAllIndexes", {});

    xmlDocUniquePtr pLayout = parseLayoutDump();
    // Without the fix in place, this test would have failed with
    // - Expected: 2
    // - Actual  : 4
    assertXPath(pLayout, "/root/page[1]/body/section[2]/txt"_ostr, 2);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf151462)
{
    createSwDoc("tdf151462.odt");
    dispatchCommand(mxComponent, ".uno:UpdateAllIndexes", {});

    xmlDocUniquePtr pLayout = parseLayoutDump();
    // Without the fix in place, there would be just the first index entry
    assertXPath(pLayout,
                "/root/page[1]/body/txt[2]/anchored/fly/section/txt[1]/SwParaPortion/"
                "SwLineLayout[1]/SwLinePortion[1]"_ostr,
                "portion"_ostr, "sub one");
    assertXPath(pLayout,
                "/root/page[1]/body/txt[2]/anchored/fly/section/txt[2]/SwParaPortion/"
                "SwLineLayout[1]/SwLinePortion[1]"_ostr,
                "portion"_ostr, "sub two");
    assertXPath(pLayout,
                "/root/page[1]/body/txt[2]/anchored/fly/section/txt[3]/SwParaPortion/"
                "SwLineLayout[1]/SwLinePortion[1]"_ostr,
                "portion"_ostr, "sub three");

    // Without the fix in place, there would be just the first index entry
    assertXPath(pLayout,
                "/root/page[1]/body/txt[6]/anchored/fly/section/txt[1]/SwParaPortion/"
                "SwLineLayout[1]/SwLinePortion[1]"_ostr,
                "portion"_ostr, "another sub one");
    assertXPath(pLayout,
                "/root/page[1]/body/txt[6]/anchored/fly/section/txt[2]/SwParaPortion/"
                "SwLineLayout[1]/SwLinePortion[1]"_ostr,
                "portion"_ostr, "another sub two");
    assertXPath(pLayout,
                "/root/page[1]/body/txt[6]/anchored/fly/section/txt[3]/SwParaPortion/"
                "SwLineLayout[1]/SwLinePortion[1]"_ostr,
                "portion"_ostr, "another sub three");
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf151801)
{
    Resetter resetter([]() {
        std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::AutoCorrect::SingleQuoteAtStart::set(0, pBatch);
        officecfg::Office::Common::AutoCorrect::SingleQuoteAtEnd::set(0, pBatch);
        return pBatch->commit();
    });
    // Set Single Quotes › and ‹
    std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
        comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::AutoCorrect::SingleQuoteAtStart::set(8250, pBatch);
    officecfg::Office::Common::AutoCorrect::SingleQuoteAtEnd::set(8249, pBatch);
    pBatch->commit();

    createSwDoc("tdf151801.fodt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    // Single starting quote: 'word -> ›word
    emulateTyping(*pTextDoc, u"'word");
    OUString sReplaced(u"\u203Aword"_ustr);
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
    // Single ending quote: ›word' -> ›word‹
    emulateTyping(*pTextDoc, u"'");
    sReplaced += u"\u2039";
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
    // Use apostrophe without preceding starting quote: word' -> word’
    emulateTyping(*pTextDoc, u" word'");
    sReplaced += u" word\u2019";
    CPPUNIT_ASSERT_EQUAL(sReplaced, getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testCursorPositionAfterUndo)
{
    createSwDoc("cursor_position_after_undo.odt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    // switch on "Outline Folding" mode
    dispatchCommand(mxComponent, ".uno:ShowOutlineContentVisibilityButton", {});
    CPPUNIT_ASSERT(pWrtShell->GetViewOptions()->IsShowOutlineContentVisibilityButton());

    // move the cursor to the beginning of the 3rd word in the 3rd paragraph, "tincidunt"
    pWrtShell->FwdPara();
    pWrtShell->FwdPara();
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 16, /*bBasicCall=*/false);

    // select the word
    dispatchCommand(mxComponent, ".uno:SelectWord", {});

    // check the word is select
    SwShellCursor* pShellCursor = pWrtShell->getShellCursor(false);
    CPPUNIT_ASSERT_EQUAL(OUString("tincidunt"), pShellCursor->GetText());

    // remember the cursor position for comparison
    SwPosition aCursorPos(*pWrtShell->GetCursor()->GetPoint());

    // delete the selected word
    pWrtShell->Delete();

    // undo delete
    dispatchCommand(mxComponent, ".uno:Undo", {});

    // without the fix in place, the cursor would have been set to the start of the outline node
    // - Expected: SwPosition (node 11, offset 25)
    // - Actual  : SwPosition (node 9, offset 0)
    CPPUNIT_ASSERT_EQUAL(aCursorPos, *pWrtShell->GetCursor()->GetPoint());

    // switch off "Outline Folding" mode
    dispatchCommand(mxComponent, ".uno:ShowOutlineContentVisibilityButton", {});
    CPPUNIT_ASSERT(!pWrtShell->GetViewOptions()->IsShowOutlineContentVisibilityButton());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf73483)
{
    // Given a document with a first paragraph having a manually set page break with page style
    createSwDoc("pageBreakWithPageStyle.fodt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    CPPUNIT_ASSERT_EQUAL(OUString("Right Page"), pWrtShell->GetCurPageStyle());

    dispatchCommand(mxComponent, ".uno:ResetAttributes", {}); // Ctrl+M "Clear Direct Formatting"
    // Make sure that clearing direct formatting doesn't clear the page style
    CPPUNIT_ASSERT_EQUAL(OUString("Right Page"), pWrtShell->GetCurPageStyle());

    // Make sure that the page break with page style survives ODF save-and-reload
    saveAndReload("writer8");

    xmlDocUniquePtr pXml = parseExport("content.xml");
    CPPUNIT_ASSERT(pXml);
    OUString para_style_name = getXPath(
        pXml, "/office:document-content/office:body/office:text/text:p"_ostr, "style-name"_ostr);
    // Without the fix in place, this would fail
    CPPUNIT_ASSERT(!para_style_name.equalsIgnoreAsciiCase("Standard"));

    OString para_style_path
        = "/office:document-content/office:automatic-styles/style:style[@style:name='"
          + para_style_name.toUtf8() + "']";
    assertXPath(pXml, para_style_path, "family"_ostr, "paragraph");
    // Without the fix in place, the autostyle had no parent
    assertXPath(pXml, para_style_path, "parent-style-name"_ostr, "Standard");
    assertXPath(pXml, para_style_path, "master-page-name"_ostr, "Right_20_Page");
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf62032ApplyStyle)
{
    createSwDoc("tdf62032_apply_style.odt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtSh = pDoc->GetDocShell()->GetWrtShell();

    pWrtSh->Down(/*bSelect=*/false);

    uno::Sequence<beans::PropertyValue> aPropertyValues = comphelper::InitPropertySequence({
        { "Style", uno::Any(OUString("A 2")) },
        { "FamilyName", uno::Any(OUString("ParagraphStyles")) },
    });
    dispatchCommand(mxComponent, ".uno:StyleApply", aPropertyValues);

    // Without the fix in place, it fails with:
    // - Expected: 1.1
    // - Actual  : 2
    CPPUNIT_ASSERT_EQUAL(OUString("1.1"),
                         getProperty<OUString>(getParagraph(2), "ListLabelString").trim());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf156560)
{
    createSwDoc("tdf156560.docx");

    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"),
                                                   uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xPageStyle, "HeaderIsOn"));

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_MOD1 | KEY_PAGEUP);

    // Insert header
    // Without the fix in place, this test would have got SIGABRT here
    dispatchCommand(mxComponent, ".uno:InsertHeader", {});
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf158459)
{
    createSwDoc("tdf158459_tracked_changes_across_nodes.fodt");
    SwDoc* pDoc = getSwDoc();

    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);
    pWrtShell->FwdPara(); // Skip first paragraph
    pWrtShell->EndOfSection(true); // Select everything to the end

    SwDoc aClipboard;
    pWrtShell->Copy(aClipboard); // This must not crash

    pWrtShell->SelAll();
    pWrtShell->Delete();
    pWrtShell->Paste(aClipboard); // Replace everything with the copied stuff

    SwNodes& rNodes = pDoc->GetNodes();
    SwNodeIndex aIdx(rNodes.GetEndOfExtras());
    SwContentNode* pContentNode = SwNodes::GoNext(&aIdx);
    CPPUNIT_ASSERT(pContentNode);
    SwTextNode* pTextNode = pContentNode->GetTextNode();
    CPPUNIT_ASSERT(pTextNode);
    // Check that deleted parts (paragraph break, "c", "e") haven't been pasted
    CPPUNIT_ASSERT_EQUAL(OUString("abdf"), pTextNode->GetText());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf158703)
{
    // Given a document with French text, consisting of a word and several spaces:
    createSwDoc("tdf158703.fodt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    dispatchCommand(mxComponent, ".uno:GoToEndOfDoc", {});

    // Typing ":" after the spaces should start auto-correction, which is expected to
    // remove the spaces, and insert an NBSP instead. It must not crash.
    emulateTyping(*pTextDoc, u":");
    CPPUNIT_ASSERT_EQUAL(u"Foo\u00A0:"_ustr, getParagraph(1)->getString());
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
