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
#include <editeng/lrspitem.hxx>
#include <sfx2/linkmgr.hxx>

#include <wrtsh.hxx>
#include <UndoManager.hxx>
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
        : SwModelTestBase(u"/sw/qa/extras/uiwriter/data/"_ustr)
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
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    dispatchCommand(mxComponent, u".uno:Cut"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xIndexAccess->getCount());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    // without the fix, it crashes
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    // check that the text frame has the correct upper
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    OUString const sectionId = getXPath(pXmlDoc, "/root/page[1]/body/section[7]", "id");
    OUString const sectionLower = getXPath(pXmlDoc, "/root/page[1]/body/section[7]", "lower");
    OUString const textId = getXPath(pXmlDoc, "/root/page[1]/body/section[7]/txt[1]", "id");
    OUString const textUpper = getXPath(pXmlDoc, "/root/page[1]/body/section[7]/txt[1]", "upper");
    CPPUNIT_ASSERT_EQUAL(textId, sectionLower);
    CPPUNIT_ASSERT_EQUAL(sectionId, textUpper);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf132420)
{
    createSwDoc("tdf132420.odt");

    CPPUNIT_ASSERT_EQUAL(12, getShapes());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    dispatchCommand(mxComponent, u".uno:Cut"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(0, getShapes());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

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

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    dispatchCommand(mxComponent, u".uno:Cut"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(0, getShapes());

    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});

    //Without the fix in place, the image wouldn't be pasted
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf146622)
{
    createSwDoc("TC-table-del-add.docx");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());

    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTables->getCount());
    uno::Reference<container::XNameAccess> xTableNames = xTablesSupplier->getTextTables();
    CPPUNIT_ASSERT(xTableNames->hasByName(u"Table1"_ustr));
    uno::Reference<text::XTextTable> xTable1(xTableNames->getByName(u"Table1"_ustr),
                                             uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xTable1->getRows()->getCount());

    dispatchCommand(mxComponent, u".uno:DeleteRows"_ustr, {});

    // This was 3 (deleting the already deleted row with change tracking)
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xTable1->getRows()->getCount());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    dispatchCommand(mxComponent, u".uno:DeleteRows"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xTable1->getRows()->getCount());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    dispatchCommand(mxComponent, u".uno:DeleteRows"_ustr, {});
    // This was 2 (deleting the already deleted table with change tracking)
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xTable1->getRows()->getCount());

    // check that the first table was deleted with change tracking
    dispatchCommand(mxComponent, u".uno:AcceptAllTrackedChanges"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());

    // Undo AcceptAllTrackedChanges and DeleteRows
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    // now only the second table deleted by AcceptAllTrackedChanges
    dispatchCommand(mxComponent, u".uno:AcceptAllTrackedChanges"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTables->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf146962)
{
    // load a 2-row table, set Hide Changes mode and delete the first row with change tracking
    createSwDoc("tdf116789.fodt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    // enable redlining
    dispatchCommand(mxComponent, u".uno:TrackChanges"_ustr, {});
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    // hide changes
    dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});
    CPPUNIT_ASSERT(pWrtShell->GetLayout()->IsHideRedlines());

    dispatchCommand(mxComponent, u".uno:DeleteRows"_ustr, {});

    // Without the fix in place, the deleted row would be visible

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // This was 2
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row", 1);

    // check it in Show Changes mode

    dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});
    CPPUNIT_ASSERT(!pWrtShell->GetLayout()->IsHideRedlines());

    pXmlDoc = parseLayoutDump();
    // 2 rows are visible now
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row", 2);

    // check it in Hide Changes mode again

    dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});
    CPPUNIT_ASSERT(pWrtShell->GetLayout()->IsHideRedlines());

    pXmlDoc = parseLayoutDump();
    // only a single row is visible again
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row", 1);

    // tdf#148227 check Undo of tracked table row deletion

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    pXmlDoc = parseLayoutDump();
    // This was 1
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row", 2);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf159026)
{
    // load a floating table (tables in DOCX footnotes
    // imported as floating tables in Writer)
    createSwDoc("tdf159026.docx");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    // enable redlining
    dispatchCommand(mxComponent, u".uno:TrackChanges"_ustr, {});
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    // hide changes
    dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});
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
    dispatchCommand(mxComponent, u".uno:DeleteTable"_ustr, {});

    // tracked table deletion
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    // hidden table
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//tab", 0);

    // delete frame
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess2(xTextFramesSupplier->getTextFrames(),
                                                          uno::UNO_QUERY);
    xSelSupplier->select(xIndexAccess2->getByIndex(0));
    dispatchCommand(mxComponent, u".uno:Delete"_ustr, {});

    // undo frame deletion
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    // undo tracked table deletion

    // This resulted crashing
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf147347)
{
    // load a 2-row table, set Hide Changes mode and delete the table with change tracking
    createSwDoc("tdf116789.fodt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    // enable redlining
    dispatchCommand(mxComponent, u".uno:TrackChanges"_ustr, {});
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    // hide changes
    dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});
    CPPUNIT_ASSERT(pWrtShell->GetLayout()->IsHideRedlines());

    dispatchCommand(mxComponent, u".uno:DeleteTable"_ustr, {});

    // Without the fix in place, the deleted row would be visible

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // This was 1
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row", 0);

    // check it in Show Changes mode

    dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});
    CPPUNIT_ASSERT(!pWrtShell->GetLayout()->IsHideRedlines());

    pXmlDoc = parseLayoutDump();
    // 2 rows are visible now
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row", 2);

    // check it in Hide Changes mode again

    dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});
    CPPUNIT_ASSERT(pWrtShell->GetLayout()->IsHideRedlines());

    pXmlDoc = parseLayoutDump();
    // no visible row again
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row", 0);

    // tdf#148228 check Undo of tracked table deletion

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    pXmlDoc = parseLayoutDump();
    // This was 0
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row", 2);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf153819)
{
    // copy a table before a deleted table in Hide Changes mode
    createSwDoc("tdf153819.fodt");
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    // hide changes
    CPPUNIT_ASSERT(pWrtShell->GetLayout()->IsHideRedlines());

    dispatchCommand(mxComponent, u".uno:SelectTable"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});
    dispatchCommand(mxComponent, u".uno:GoDown"_ustr, {});

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});

    // FIXME: Show Changes, otherwise ~SwTableNode() would have crashed
    dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf148345)
{
    // load a 2-row table, set Hide Changes mode and delete the first row with change tracking
    createSwDoc("tdf116789.fodt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    // enable redlining
    dispatchCommand(mxComponent, u".uno:TrackChanges"_ustr, {});
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    // hide changes
    dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});
    CPPUNIT_ASSERT(pWrtShell->GetLayout()->IsHideRedlines());

    dispatchCommand(mxComponent, u".uno:DeleteRows"_ustr, {});

    // Without the fix in place, the deleted row would be visible

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // This was 2
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row", 1);

    // check it in Show Changes mode

    dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});
    CPPUNIT_ASSERT(!pWrtShell->GetLayout()->IsHideRedlines());

    pXmlDoc = parseLayoutDump();
    // 2 rows are visible now
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row", 2);

    // check it in Hide Changes mode again

    dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});
    CPPUNIT_ASSERT(pWrtShell->GetLayout()->IsHideRedlines());

    pXmlDoc = parseLayoutDump();
    // only a single row is visible again
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row", 1);

    // tdf#148227 check Reject All of tracked table row deletion

    dispatchCommand(mxComponent, u".uno:RejectAllTrackedChanges"_ustr, {});
    pXmlDoc = parseLayoutDump();
    // This was 1
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row", 2);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf141391)
{
    // table insertion in the first paragraph of the cell
    // overwrites the row content, instead of inserting a nested table

    // load a 2-row table
    createSwDoc("tdf116789.fodt");
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    // select the table, and copy it into at paragraph start of cell "A2"

    dispatchCommand(mxComponent, u".uno:SelectTable"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});
    // remove the selection and positionate the cursor at beginning of A2
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // 3-row, overwriting cells of the second row and inserting a new row
    // with the 2-row clipboard table content
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row", 3);
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row[2]/cell[1]/txt/SwParaPortion/SwLineLayout",
                "portion", u"hello");

    // Undo

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    pXmlDoc = parseLayoutDump();
    // 2 rows again, no copied text content
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row", 2);
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row[2]/cell[1]/SwParaPortion", 0);

    // insert the 2-row table into the second paragraph of cell "A2" as a nested table
    // For this it's enough to positionate the text cursor not in the first paragraph

    // insert some text and an empty paragraph
    pWrtShell->Insert(u"Some text..."_ustr);
    pWrtShell->SplitNode();
    Scheduler::ProcessEventsToIdle();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row", 2);
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row[2]/cell[1]/txt", 2);
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row[2]/cell[1]/txt[1]/SwParaPortion/SwLineLayout",
                "portion", u"Some text...");
    // the empty paragraph in A2
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row[2]/cell[1]/txt[2]/SwParaPortion", 0);

    // insert the table, as a nested one in cell "A2"
    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row", 2);
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row[2]/cell[1]/tab", 1);
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row[2]/cell[1]/tab/row", 2);

    // Undo

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    pXmlDoc = parseLayoutDump();
    // 2 rows again, no copied text content
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row", 2);
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row[2]/cell[1]/txt[1]/SwParaPortion/SwLineLayout",
                "portion", u"Some text...");

    // copy the 2-row table into the first paragraph of cell "A2",
    // but not at paragraph start (changed behaviour)

    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    pWrtShell->Insert(u"and some text again in the first paragraph to be sure..."_ustr);
    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});

    pXmlDoc = parseLayoutDump();

    // 3-row, overwriting cells of the second row and inserting a new row
    // with the 2-row clipboard table content

    // This was 2 (nested table)
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row", 3);
    // This was "Some text..." with a nested table
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row[2]/cell[1]/txt/SwParaPortion/SwLineLayout",
                "portion", u"hello");
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf148791)
{
    // test Paste as Rows Above with centered table alignment

    // load a 2-row table
    createSwDoc("tdf116789.fodt");
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    // select and copy the table, and Paste As Rows Above

    dispatchCommand(mxComponent, u".uno:SelectTable"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});
    // remove the selection and positionate the cursor at beginning of A2
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    pWrtShell->Up(/*bSelect=*/false);
    dispatchCommand(mxComponent, u".uno:PasteRowsBefore"_ustr, {});

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // Paste as Rows Above results 4-row table with default table alignment
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row", 4);
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row[1]/cell[1]/txt/SwParaPortion/SwLineLayout",
                "portion", u"hello");
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row[3]/cell[1]/txt/SwParaPortion/SwLineLayout",
                "portion", u"hello");

    // set table alignment to center, select and copy the table again
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    uno::Reference<text::XTextTable> xTextTable(xIndexAccess->getByIndex(0), uno::UNO_QUERY);

    // Default table alignment
    CPPUNIT_ASSERT_EQUAL(text::HoriOrientation::FULL,
                         getProperty<sal_Int16>(xTextTable, u"HoriOrient"_ustr));

    //CPPUNIT_ASSERT_EQUAL(OUString(""), getProperty<OUString>(xTextTable, "TableTemplateName"));
    uno::Reference<beans::XPropertySet> xTableProps(xTextTable, uno::UNO_QUERY_THROW);

    xTableProps->setPropertyValue(u"HoriOrient"_ustr, uno::Any(text::HoriOrientation::CENTER));

    CPPUNIT_ASSERT_EQUAL(text::HoriOrientation::CENTER,
                         getProperty<sal_Int16>(xTextTable, u"HoriOrient"_ustr));

    dispatchCommand(mxComponent, u".uno:SelectTable"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});
    // remove the selection and positionate the cursor at beginning of A2
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    pWrtShell->Up(/*bSelect=*/false);
    pWrtShell->Up(/*bSelect=*/false);
    pWrtShell->Up(/*bSelect=*/false);
    dispatchCommand(mxComponent, u".uno:PasteRowsBefore"_ustr, {});

    pXmlDoc = parseLayoutDump();
    // This was 5 (inserting only a single row for the 4-row clipboard content, and
    // overwriting 3 existing rows)
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row", 8);
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row[1]/cell[1]/txt/SwParaPortion/SwLineLayout",
                "portion", u"hello");
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row[3]/cell[1]/txt/SwParaPortion/SwLineLayout",
                "portion", u"hello");
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row[5]/cell[1]/txt/SwParaPortion/SwLineLayout",
                "portion", u"hello");
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row[7]/cell[1]/txt/SwParaPortion/SwLineLayout",
                "portion", u"hello");

    // tdf#64902 add a test case for nested tables

    // insert a nested table, and copy as paste as rows above the whole table with it
    dispatchCommand(mxComponent, u".uno:PasteNestedTable"_ustr, {});
    dispatchCommand(mxComponent, u".uno:SelectTable"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});
    // remove the selection and positionate the cursor at beginning of A2
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    // skip 7 table rows plus 4 rows of the nested table
    for (int i = 0; i < 7 + 4; ++i)
        pWrtShell->Up(/*bSelect=*/false);
    dispatchCommand(mxComponent, u".uno:PasteRowsBefore"_ustr, {});

    pXmlDoc = parseLayoutDump();
    // rows of the nested table doesn't effect row number of the main table
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row", 16);
    // there are two nested tables after the paste
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row/cell/tab", 2);

    // tdf#64902 add a test case for repeated table headings

    xTableProps->setPropertyValue(u"RepeatHeadline"_ustr, uno::Any(true));
    CPPUNIT_ASSERT(getProperty<bool>(xTextTable, u"RepeatHeadline"_ustr));

    xTableProps->setPropertyValue(u"HeaderRowCount"_ustr, uno::Any(sal_Int32(3)));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), getProperty<sal_Int32>(xTextTable, u"HeaderRowCount"_ustr));

    dispatchCommand(mxComponent, u".uno:SelectTable"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});
    // remove the selection and positionate the cursor at beginning of A2
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    // skip 15 table rows plus 4 * 2 rows of the nested tables
    for (int i = 0; i < 15 + 4 * 2; ++i)
        pWrtShell->Up(/*bSelect=*/false);
    dispatchCommand(mxComponent, u".uno:PasteRowsBefore"_ustr, {});

    pXmlDoc = parseLayoutDump();
    // repeating table header (and its thead/tbody indentation) doesn't effect row number
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row", 32);
    // there are two nested tables after the paste
    assertXPath(pXmlDoc, "/root/page[1]/body/tab/row/cell/tab", 4);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf135014)
{
    createSwDoc();

    uno::Sequence<beans::PropertyValue> aArgs(
        comphelper::InitPropertySequence({ { "KeyModifier", uno::Any(sal_Int32(0)) } }));

    // Toggle Numbering List
    dispatchCommand(mxComponent, u".uno:DefaultBullet"_ustr, aArgs);

    uno::Sequence<beans::PropertyValue> aArgs2(comphelper::InitPropertySequence(
        { { "Param", uno::Any(u"NewNumberingStyle"_ustr) },
          { "Family", uno::Any(static_cast<sal_Int16>(SfxStyleFamily::Pseudo)) } }));

    // New Style from selection
    dispatchCommand(mxComponent, u".uno:StyleNewByExample"_ustr, aArgs2);

    // Without the fix in place, this test would have failed here
    saveAndReload(u"Office Open XML Text"_ustr);

    xmlDocUniquePtr pXmlStyles = parseExport(u"word/styles.xml"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='NewNumberingStyle']/w:qFormat", 1);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf130629)
{
    createSwDoc();

    uno::Sequence<beans::PropertyValue> aArgs(
        comphelper::InitPropertySequence({ { "KeyModifier", uno::Any(KEY_MOD1) } }));

    dispatchCommand(mxComponent, u".uno:BasicShapes.diamond"_ustr, aArgs);

    CPPUNIT_ASSERT_EQUAL(1, getShapes());

    // Undo twice
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(0, getShapes());

    // Shape toolbar is active, use ESC before inserting a new shape
    SwXTextDocument* pTextDoc = getSwTextDoc();
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_ESCAPE);
    Scheduler::ProcessEventsToIdle();

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, u".uno:BasicShapes.diamond"_ustr, aArgs);

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
    SwWrtShell* const pWrtSh = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtSh);

    pWrtSh->Insert(u"Hello World"_ustr);

    // Select 'World'
    pWrtSh->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 5, /*bBasicCall=*/false);

    // Save as PDF.
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "Selection", uno::Any(true) } }));

    uno::Sequence<beans::PropertyValue> aDescriptor(
        comphelper::InitPropertySequence({ { "FilterName", uno::Any(u"writer_pdf_Export"_ustr) },
                                           { "FilterData", uno::Any(aFilterData) },
                                           { "URL", uno::Any(maTempFile.GetURL()) } }));

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, u".uno:ExportToPDF"_ustr, aDescriptor);

    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);
    CPPUNIT_ASSERT_EQUAL(1, pPdfPage->getObjectCount());
    std::unique_ptr<vcl::pdf::PDFiumTextPage> pPdfTextPage = pPdfPage->getTextPage();
    CPPUNIT_ASSERT(pPdfTextPage);

    std::unique_ptr<vcl::pdf::PDFiumPageObject> pPageObject = pPdfPage->getObject(0);
    OUString sText = pPageObject->getText(pPdfTextPage);
    CPPUNIT_ASSERT_EQUAL(u"World"_ustr, sText);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf131728)
{
    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    if (!pPDFium)
    {
        return;
    }
    createSwDoc("tdf131728.docx");
    SwDoc* const pDoc = getSwDoc();
    SwWrtShell* const pWrtSh = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtSh);

    // Save as PDF.
    uno::Sequence<beans::PropertyValue> aFilterData(comphelper::InitPropertySequence(
        { { "ExportBookmarksToPDFDestination", uno::Any(true) } }));

    uno::Sequence<beans::PropertyValue> aDescriptor(
        comphelper::InitPropertySequence({ { "FilterName", uno::Any(u"writer_pdf_Export"_ustr) },
                                           { "FilterData", uno::Any(aFilterData) },
                                           { "URL", uno::Any(maTempFile.GetURL()) } }));

    dispatchCommand(mxComponent, u".uno:ExportToPDF"_ustr, aDescriptor);

    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());

    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);

    // Without the fix in place, this test would have bad order
    // (starting with the outlines of text frames)
    CPPUNIT_ASSERT_EQUAL(u"Article 1. Definitions\n"
                         " Apple\n"
                         " Bread\n"
                         " Cable\n"
                         " Cable\n" // ???
                         "Article 2. Three style separators in one line!\n"
                         " Heading 2\n"
                         " Heading 2 Again\n"_ustr,
                         pPdfDocument->getBookmarks());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf95239)
{
    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    if (!pPDFium)
    {
        return;
    }
    createSwDoc("tdf95239.fodt");
    SwDoc* const pDoc = getSwDoc();
    SwWrtShell* const pWrtSh = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtSh);

    // Save as PDF.
    uno::Sequence<beans::PropertyValue> aFilterData(comphelper::InitPropertySequence(
        { { "ExportBookmarksToPDFDestination", uno::Any(true) } }));

    uno::Sequence<beans::PropertyValue> aDescriptor(
        comphelper::InitPropertySequence({ { "FilterName", uno::Any(u"writer_pdf_Export"_ustr) },
                                           { "FilterData", uno::Any(aFilterData) },
                                           { "URL", uno::Any(maTempFile.GetURL()) } }));

    dispatchCommand(mxComponent, u".uno:ExportToPDF"_ustr, aDescriptor);

    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();
    CPPUNIT_ASSERT_EQUAL(2, pPdfDocument->getPageCount());

    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);

    // Without the fix in place, this test would have bad order
    // (starting with the outlines of text frames)
    CPPUNIT_ASSERT_EQUAL(u"H1\n"
                         " H2\n"
                         "  H3\n"
                         "   Lorem\n"
                         "    Vestibulum\n"
                         "    Integer\n"
                         "   Aliquam\n"
                         "    Donec\n"
                         "    Praesent\n"
                         "  H3\n"
                         "   Lorem\n"
                         "    Vestibulum\n"
                         "    Integer\n"
                         "   Aliquam\n"
                         "    Donec\n"
                         "    Praesent\n"
                         "H1\n"
                         " H2\n"
                         "  H3\n"
                         "   Lorem\n"
                         "    Vestibulum\n"
                         "    Integer\n"
                         "   Aliquam\n"
                         "    Donec\n"
                         "    Praesent\n"_ustr,
                         pPdfDocument->getBookmarks());
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

    uno::Sequence<beans::PropertyValue> aDescriptor(
        comphelper::InitPropertySequence({ { "FilterName", uno::Any(u"writer_pdf_Export"_ustr) },
                                           { "FilterData", uno::Any(aFilterData) },
                                           { "URL", uno::Any(maTempFile.GetURL()) } }));

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, u".uno:ExportToPDF"_ustr, aDescriptor);

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
    SwWrtShell* const pWrtSh = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtSh);

    pWrtSh->Insert(u"Lorem"_ustr);

    SwXTextDocument* pTextDoc = getSwTextDoc();
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_F3);
    Scheduler::ProcessEventsToIdle();

    // generating a big text with ~60k words and several paragraphs
    for (sal_Int32 i = 0; i < 8; ++i)
    {
        dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

        dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});

        dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});

        dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});
    }

    dispatchCommand(mxComponent, u".uno:GoToStartOfDoc"_ustr, {});

    // Format->Text operations on small selections (which would generate <~500 redlines)
    // changetracking still working
    dispatchCommand(mxComponent, u".uno:TrackChanges"_ustr, {});

    SwCursorShell* pShell(pDoc->GetEditShell());
    CPPUNIT_ASSERT(pShell);

    pShell->SelectTextModel(1, 500);

    dispatchCommand(mxComponent, u".uno:ChangeCaseToTitleCase"_ustr, {});

    SwEditShell* const pEditShell(pDoc->GetEditShell());
    CPPUNIT_ASSERT(pEditShell);
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(120),
                         pEditShell->GetRedlineCount());

    //Removing all the redlines.
    dispatchCommand(mxComponent, u".uno:RejectAllTrackedChanges"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(0), pEditShell->GetRedlineCount());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    dispatchCommand(mxComponent, u".uno:ChangeCaseToTitleCase"_ustr, {});

    // Without the fix in place, on big selections writer would freeze. Now it ignores change tracking.
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(0), pEditShell->GetRedlineCount());

    // The patch has no effects on the Format->Text operations
    CPPUNIT_ASSERT(getParagraph(1)->getString().startsWith("Lorem Ipsum Dolor Sit Amet"));

    dispatchCommand(mxComponent, u".uno:ChangeCaseToUpper"_ustr, {});

    CPPUNIT_ASSERT(getParagraph(1)->getString().startsWith("LOREM IPSUM DOLOR SIT AMET"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf116315)
{
    createSwDoc();
    SwWrtShell* const pWrtSh = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtSh);

    pWrtSh->Insert(u"This is a test"_ustr);
    pWrtSh->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 4, /*bBasicCall=*/false);

    SwXTextDocument* pTextDoc = getSwTextDoc();
    for (sal_Int32 i = 0; i < 5; ++i)
    {
        pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_SHIFT | KEY_F3);
        Scheduler::ProcessEventsToIdle();

        // Title Case
        CPPUNIT_ASSERT_EQUAL(u"This is a Test"_ustr, getParagraph(1)->getString());

        pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_SHIFT | KEY_F3);
        Scheduler::ProcessEventsToIdle();

        // Upper Case
        CPPUNIT_ASSERT_EQUAL(u"This is a TEST"_ustr, getParagraph(1)->getString());

        pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_SHIFT | KEY_F3);
        Scheduler::ProcessEventsToIdle();

        // Lower Case
        CPPUNIT_ASSERT_EQUAL(u"This is a test"_ustr, getParagraph(1)->getString());
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testInsertAutoTextIntoListFromParaStyle)
{
    createSwDoc("stylewithlistandindents.fodt");
    SwWrtShell* const pWrtShell = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    pWrtShell->FwdPara();
    pWrtShell->EndPara(/*bSelect=*/false);
    // expands autotext (via F3)
    pWrtShell->Insert(u" jacr"_ustr);

    SwXTextDocument* pTextDoc = getSwTextDoc();
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_F3);
    Scheduler::ProcessEventsToIdle();

    pWrtShell->SttEndDoc(/*bStt=*/true);
    pWrtShell->FwdPara();

    SwNumRule* pNumRule;
    SvxTextLeftMarginItem const* pTextLeftMargin;
    SvxFirstLineIndentItem const* pFirstLineIndent;

    {
        SwTextNode& rNode{ *pWrtShell->GetCursor()->GetPoint()->GetNode().GetTextNode() };
        // numrule from paragraph style, but not from direct formatting
        auto pSet{ rNode.GetpSwAttrSet() };
        CPPUNIT_ASSERT(pSet);
        // list id was set
        CPPUNIT_ASSERT_EQUAL(SfxItemState::SET, pSet->GetItemState(RES_PARATR_LIST_ID, false));
        // the numrule is set on the paragraph style, not on the paragraph
        CPPUNIT_ASSERT_EQUAL(SfxItemState::DEFAULT, pSet->GetItemState(RES_PARATR_NUMRULE, false));
        CPPUNIT_ASSERT_EQUAL(SfxItemState::SET, pSet->GetItemState(RES_PARATR_NUMRULE, true));
        CPPUNIT_ASSERT_EQUAL(SfxItemState::DEFAULT,
                             pSet->GetItemState(RES_MARGIN_FIRSTLINE, false));
        CPPUNIT_ASSERT_EQUAL(SfxItemState::SET, pSet->GetItemState(RES_MARGIN_FIRSTLINE, true));
        CPPUNIT_ASSERT_EQUAL(SfxItemState::DEFAULT, pSet->GetItemState(RES_MARGIN_TEXTLEFT, false));
        CPPUNIT_ASSERT_EQUAL(SfxItemState::SET, pSet->GetItemState(RES_MARGIN_TEXTLEFT, true));
        CPPUNIT_ASSERT_EQUAL(u"ListAndIndents"_ustr, rNode.GetTextColl()->GetName());
        CPPUNIT_ASSERT_EQUAL(u"Item We confirm receipt of your application material."_ustr,
                             rNode.GetText());
        pNumRule = rNode.GetNumRule();
        pTextLeftMargin = &rNode.GetAttr(RES_MARGIN_TEXTLEFT);
        pFirstLineIndent = &rNode.GetAttr(RES_MARGIN_FIRSTLINE);
    }

    pWrtShell->FwdPara();

    {
        SwTextNode& rNode{ *pWrtShell->GetCursor()->GetPoint()->GetNode().GetTextNode() };
        auto pSet{ rNode.GetpSwAttrSet() };
        CPPUNIT_ASSERT(pSet);
        // list id was set
        CPPUNIT_ASSERT_EQUAL(SfxItemState::SET, pSet->GetItemState(RES_PARATR_LIST_ID, false));
        // middle paragraph was pasted - has numrule and indents applied directly
        CPPUNIT_ASSERT_EQUAL(SfxItemState::SET, pSet->GetItemState(RES_PARATR_NUMRULE, false));
        CPPUNIT_ASSERT_EQUAL(SfxItemState::SET, pSet->GetItemState(RES_MARGIN_FIRSTLINE, false));
        CPPUNIT_ASSERT_EQUAL(SfxItemState::SET, pSet->GetItemState(RES_MARGIN_TEXTLEFT, false));
        CPPUNIT_ASSERT_EQUAL(u"Default Paragraph Style"_ustr, rNode.GetTextColl()->GetName());
        CPPUNIT_ASSERT(rNode.GetText().startsWith("As more applicants applied"));
        CPPUNIT_ASSERT_EQUAL(pNumRule, rNode.GetNumRule());
        CPPUNIT_ASSERT_EQUAL(pTextLeftMargin->ResolveTextLeft({}),
                             rNode.GetAttr(RES_MARGIN_TEXTLEFT).ResolveTextLeft({}));
        CPPUNIT_ASSERT_EQUAL(pFirstLineIndent->ResolveTextFirstLineOffset({}),
                             rNode.GetAttr(RES_MARGIN_FIRSTLINE).ResolveTextFirstLineOffset({}));
    }

    pWrtShell->FwdPara();

    {
        SwTextNode& rNode{ *pWrtShell->GetCursor()->GetPoint()->GetNode().GetTextNode() };
        // numrule from paragraph style, but not from direct formatting
        auto pSet{ rNode.GetpSwAttrSet() };
        CPPUNIT_ASSERT(pSet);
        // list id was set
        CPPUNIT_ASSERT_EQUAL(SfxItemState::SET, pSet->GetItemState(RES_PARATR_LIST_ID, false));
        // the numrule is set on the paragraph style, not on the paragraph
        CPPUNIT_ASSERT_EQUAL(SfxItemState::DEFAULT, pSet->GetItemState(RES_PARATR_NUMRULE, false));
        CPPUNIT_ASSERT_EQUAL(SfxItemState::SET, pSet->GetItemState(RES_PARATR_NUMRULE, true));
        CPPUNIT_ASSERT_EQUAL(SfxItemState::DEFAULT,
                             pSet->GetItemState(RES_MARGIN_FIRSTLINE, false));
        CPPUNIT_ASSERT_EQUAL(SfxItemState::SET, pSet->GetItemState(RES_MARGIN_FIRSTLINE, true));
        CPPUNIT_ASSERT_EQUAL(SfxItemState::DEFAULT, pSet->GetItemState(RES_MARGIN_TEXTLEFT, false));
        CPPUNIT_ASSERT_EQUAL(SfxItemState::SET, pSet->GetItemState(RES_MARGIN_TEXTLEFT, true));
        CPPUNIT_ASSERT_EQUAL(u"ListAndIndents"_ustr, rNode.GetTextColl()->GetName());
        CPPUNIT_ASSERT(rNode.GetText().endsWith("as soon as we have come to a decision."));
        CPPUNIT_ASSERT_EQUAL(pNumRule, rNode.GetNumRule());
        CPPUNIT_ASSERT_EQUAL(pTextLeftMargin->ResolveTextLeft({}),
                             rNode.GetAttr(RES_MARGIN_TEXTLEFT).ResolveTextLeft({}));
        CPPUNIT_ASSERT_EQUAL(pFirstLineIndent->ResolveTextFirstLineOffset({}),
                             rNode.GetAttr(RES_MARGIN_FIRSTLINE).ResolveTextFirstLineOffset({}));
    }

    pWrtShell->FwdPara();

    {
        SwTextNode& rNode{ *pWrtShell->GetCursor()->GetPoint()->GetNode().GetTextNode() };
        // numrule from paragraph style, but not from direct formatting
        auto pSet{ rNode.GetpSwAttrSet() };
        CPPUNIT_ASSERT(pSet);
        // list id was set
        CPPUNIT_ASSERT_EQUAL(SfxItemState::SET, pSet->GetItemState(RES_PARATR_LIST_ID, false));
        // the numrule is set on the paragraph style, not on the paragraph
        CPPUNIT_ASSERT_EQUAL(SfxItemState::DEFAULT, pSet->GetItemState(RES_PARATR_NUMRULE, false));
        CPPUNIT_ASSERT_EQUAL(SfxItemState::SET, pSet->GetItemState(RES_PARATR_NUMRULE, true));
        CPPUNIT_ASSERT_EQUAL(SfxItemState::DEFAULT,
                             pSet->GetItemState(RES_MARGIN_FIRSTLINE, false));
        CPPUNIT_ASSERT_EQUAL(SfxItemState::SET, pSet->GetItemState(RES_MARGIN_FIRSTLINE, true));
        CPPUNIT_ASSERT_EQUAL(SfxItemState::DEFAULT, pSet->GetItemState(RES_MARGIN_TEXTLEFT, false));
        CPPUNIT_ASSERT_EQUAL(SfxItemState::SET, pSet->GetItemState(RES_MARGIN_TEXTLEFT, true));
        CPPUNIT_ASSERT_EQUAL(u"ListAndIndents"_ustr, rNode.GetTextColl()->GetName());
        CPPUNIT_ASSERT_EQUAL(u"more"_ustr, rNode.GetText()); // pre-existing list item
        CPPUNIT_ASSERT_EQUAL(pNumRule, rNode.GetNumRule());
        CPPUNIT_ASSERT_EQUAL(pTextLeftMargin->ResolveTextLeft({}),
                             rNode.GetAttr(RES_MARGIN_TEXTLEFT).ResolveTextLeft({}));
        CPPUNIT_ASSERT_EQUAL(pFirstLineIndent->ResolveTextFirstLineOffset({}),
                             rNode.GetAttr(RES_MARGIN_FIRSTLINE).ResolveTextFirstLineOffset({}));
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf144364)
{
    createSwDoc();
    SwWrtShell* const pWrtSh = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtSh);

    // expands autotext (via F3)
    pWrtSh->Insert(u"AR"_ustr);

    SwXTextDocument* pTextDoc = getSwTextDoc();
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_F3);
    Scheduler::ProcessEventsToIdle();

    // was ...'letter of <placeholder:"November 21, 2004":"Click placeholder and overwrite">'
    CPPUNIT_ASSERT_EQUAL(
        u"We hereby acknowledge the receipt of your letter of <November 21, 2004>."_ustr,
        getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf146248)
{
    createSwDoc("tdf146248.docx");

    uno::Reference<beans::XPropertySet> xPageStyle(
        getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xPageStyle, u"HeaderIsOn"_ustr));

    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();

    // Delete the header
    pWrtShell->ChangeHeaderOrFooter(u"Default Page Style", true, false, false);

    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(xPageStyle, u"HeaderIsOn"_ustr));

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xPageStyle, u"HeaderIsOn"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf161741)
{
    // Redo of header change causes LO to crash
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();

    uno::Reference<beans::XPropertySet> xPageStyle(
        getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);

    // sanity checks: verify baseline status
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(xPageStyle, u"HeaderIsOn"_ustr));
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), rUndoManager.GetUndoActionCount());

    // Create a header
    pWrtShell->ChangeHeaderOrFooter(u"Default Page Style", /*header*/ true, /*on*/ true, false);
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xPageStyle, u"HeaderIsOn"_ustr));

    // create an additional non-header undo point
    pWrtShell->Insert(u"crash_test"_ustr); // three undo points
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), rUndoManager.GetUndoActionCount());

    // undo all the changes in one pass
    uno::Sequence<beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence({
        { "Undo", uno::Any(sal_Int32(4)) },
    }));
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, aPropertyValues); // undo all 4 actions
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(xPageStyle, u"HeaderIsOn"_ustr));
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), rUndoManager.GetUndoActionCount());

    // Crash avoided by clearing the entire redo stack. This redo request will do nothing.
    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, u".uno:Redo"_ustr, {}); // redo first (Header) change
    // Since Redo is "cleared", the redo did nothing, thus the Header remains off
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(xPageStyle, u"HeaderIsOn"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf152964)
{
    createSwDoc();

    dispatchCommand(mxComponent, u".uno:TrackChanges"_ustr, {});
    dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});

    uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence(
        { { "Rows", uno::Any(sal_Int32(2)) }, { "Columns", uno::Any(sal_Int32(2)) } }));

    dispatchCommand(mxComponent, u".uno:InsertTable"_ustr, aArgs);

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTextTable->getColumns()->getCount());

    SwDoc* pDoc = getSwDoc();
    SwEditShell* const pEditShell(pDoc->GetEditShell());
    CPPUNIT_ASSERT(pEditShell);
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(1), pEditShell->GetRedlineCount());
    dispatchCommand(mxComponent, u".uno:GoDown"_ustr, {});
    dispatchCommand(mxComponent, u".uno:DeleteRows"_ustr, {});
    dispatchCommand(mxComponent, u".uno:DeleteRows"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(2), pEditShell->GetRedlineCount());

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(1), pEditShell->GetRedlineCount());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(0), pEditShell->GetRedlineCount());

    dispatchCommand(mxComponent, u".uno:Redo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(1), pEditShell->GetRedlineCount());
    dispatchCommand(mxComponent, u".uno:Redo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(2), pEditShell->GetRedlineCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf107427)
{
    createSwDoc();

    dispatchCommand(
        mxComponent,
        u".uno:InsertPageHeader?PageStyle:string=Default%20Page%20Style&On:bool=true"_ustr, {});
    uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence(
        { { "Rows", uno::Any(sal_Int32(2)) }, { "Columns", uno::Any(sal_Int32(2)) } }));

    dispatchCommand(mxComponent, u".uno:InsertTable"_ustr, aArgs);

    xmlDocUniquePtr pLayout = parseLayoutDump();
    assertXPath(pLayout, "/root/page[1]/header/tab/row", 2);

    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    // Delete the header
    pWrtShell->ChangeHeaderOrFooter(u"Default Page Style", true, false, false);

    pLayout = parseLayoutDump();
    assertXPath(pLayout, "/root/page[1]/header", 0);

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    pLayout = parseLayoutDump();
    assertXPath(pLayout, "/root/page[1]/header/tab/row", 2);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf141613)
{
    createSwDoc();
    SwWrtShell* const pWrtSh = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtSh);

    pWrtSh->Insert(u"Test"_ustr);

    dispatchCommand(
        mxComponent,
        u".uno:InsertPageHeader?PageStyle:string=Default%20Page%20Style&On:bool=true"_ustr, {});

    uno::Reference<beans::XPropertySet> xPageStyle(
        getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xPageStyle, u"HeaderIsOn"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Test"_ustr, getParagraph(1)->getString());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(xPageStyle, u"HeaderIsOn"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Test"_ustr, getParagraph(1)->getString());

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(u""_ustr, getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf107494)
{
    createSwDoc();

    // Create a graphic object, but don't insert it yet.
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xTextGraphic(
        xFactory->createInstance(u"com.sun.star.text.TextGraphicObject"_ustr), uno::UNO_QUERY);

    uno::Reference<text::XTextContent> xTextContent(xTextGraphic, uno::UNO_QUERY);

    uno::Reference<beans::XPropertySet> xPageStyle(
        getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);

    xPageStyle->setPropertyValue(u"HeaderIsOn"_ustr, uno::Any(true));

    uno::Reference<text::XText> xHeader(
        getProperty<uno::Reference<text::XText>>(xPageStyle, u"HeaderText"_ustr));
    CPPUNIT_ASSERT(xHeader.is());
    uno::Reference<text::XTextCursor> xHeaderCursor(xHeader->createTextCursor());

    xHeader->insertTextContent(xHeaderCursor, xTextContent, false);

    CPPUNIT_ASSERT_EQUAL(1, getShapes());

    xPageStyle->setPropertyValue(u"HeaderIsOn"_ustr, uno::Any(false));

    CPPUNIT_ASSERT_EQUAL(0, getShapes());

    xPageStyle->setPropertyValue(u"FooterIsOn"_ustr, uno::Any(true));

    uno::Reference<text::XText> xFooter(
        getProperty<uno::Reference<text::XText>>(xPageStyle, u"FooterText"_ustr));
    CPPUNIT_ASSERT(xFooter.is());
    uno::Reference<text::XTextCursor> xFooterCursor(xFooter->createTextCursor());

    xTextGraphic.set(xFactory->createInstance(u"com.sun.star.text.TextGraphicObject"_ustr),
                     uno::UNO_QUERY);

    xTextContent.set(xTextGraphic, uno::UNO_QUERY);

    xFooter->insertTextContent(xFooterCursor, xTextContent, false);

    CPPUNIT_ASSERT_EQUAL(1, getShapes());

    xPageStyle->setPropertyValue(u"FooterIsOn"_ustr, uno::Any(false));

    CPPUNIT_ASSERT_EQUAL(0, getShapes());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf133358)
{
    createSwDoc();
    SwWrtShell* const pWrtSh = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtSh);

    pWrtSh->Insert(u"Test"_ustr);

    CPPUNIT_ASSERT_EQUAL(u"Test"_ustr, getParagraph(1)->getString());

    uno::Reference<beans::XPropertyState> xParagraph(getParagraph(1), uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xParagraph, u"ParaLeftMargin"_ustr));

    dispatchCommand(mxComponent, u".uno:IncrementIndent"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1251),
                         getProperty<sal_Int32>(xParagraph, u"ParaLeftMargin"_ustr));

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xParagraph, u"ParaLeftMargin"_ustr));

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, u".uno:Redo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1251),
                         getProperty<sal_Int32>(xParagraph, u"ParaLeftMargin"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf131771)
{
    createSwDoc();

    uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence(
        { { "Rows", uno::Any(sal_Int32(2)) }, { "Columns", uno::Any(sal_Int32(2)) } }));

    dispatchCommand(mxComponent, u".uno:InsertTable"_ustr, aArgs);

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    uno::Reference<text::XTextTable> xTextTable(xIndexAccess->getByIndex(0), uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(u""_ustr, getProperty<OUString>(xTextTable, u"TableTemplateName"_ustr));
    uno::Reference<beans::XPropertySet> xTableProps(xTextTable, uno::UNO_QUERY_THROW);
    xTableProps->setPropertyValue(u"TableTemplateName"_ustr, uno::Any(u"Default Style"_ustr));

    CPPUNIT_ASSERT_EQUAL(u"Default Style"_ustr,
                         getProperty<OUString>(xTextTable, u"TableTemplateName"_ustr));

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});
    dispatchCommand(mxComponent, u".uno:GoDown"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());

    CPPUNIT_ASSERT_EQUAL(u"Default Style"_ustr,
                         getProperty<OUString>(xTextTable, u"TableTemplateName"_ustr));

    uno::Reference<text::XTextTable> xTextTable2(xIndexAccess->getByIndex(1), uno::UNO_QUERY);

    // Without the fix in place, this test would have failed with
    // - Expected: Default Style
    // - Actual  :
    CPPUNIT_ASSERT_EQUAL(u"Default Style"_ustr,
                         getProperty<OUString>(xTextTable2, u"TableTemplateName"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf156546)
{
    createSwDoc();

    uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence(
        { { "Rows", uno::Any(sal_Int32(2)) }, { "Columns", uno::Any(sal_Int32(2)) } }));

    dispatchCommand(mxComponent, u".uno:InsertTable"_ustr, aArgs);

    uno::Reference<text::XTextTablesSupplier> xTableSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTableSupplier->getTextTables(),
                                                    uno::UNO_QUERY);

    // check that table was created and inserted into the document
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});

    // create another document
    createSwDoc();
    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});

    uno::Reference<text::XTextTablesSupplier> xTableSupplier2(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables2(xTableSupplier2->getTextTables(),
                                                     uno::UNO_QUERY);

    // check table exists after paste/undo
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables2->getCount());
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xTables2->getCount());

    // without the test, writer freezes on redo table paste into new doc
    dispatchCommand(mxComponent, u".uno:Redo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables2->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf80663)
{
    createSwDoc();

    uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence(
        { { "Rows", uno::Any(sal_Int32(2)) }, { "Columns", uno::Any(sal_Int32(2)) } }));

    dispatchCommand(mxComponent, u".uno:InsertTable"_ustr, aArgs);

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTextTable->getColumns()->getCount());

    dispatchCommand(mxComponent, u".uno:DeleteRows"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTextTable->getColumns()->getCount());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

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
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

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

    dispatchCommand(mxComponent, u".uno:InsertTable"_ustr, aArgs);

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    dispatchCommand(mxComponent, u".uno:DeleteTable"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xIndexAccess->getCount());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    // Without the fix in place, the table would be hidden
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/tab", 1);
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

    SwXTextDocument* pTextDoc = getSwTextDoc();
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_MOD2 | awt::Key::RETURN);
    Scheduler::ProcessEventsToIdle();

    emulateTyping(u"Some Text");

    // Without the fix in place, this test would have failed with
    // - Expected: Some Text
    // - Actual  : Table of Contents
    CPPUNIT_ASSERT_EQUAL(u"Some Text"_ustr, getParagraph(1)->getString());

    // tdf#160095: Without the fix in place, this test would have crashed here
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_MOD2 | awt::Key::RETURN);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(u"Some Text"_ustr, getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf121546)
{
    createSwDoc("tdf121546.odt");

    CPPUNIT_ASSERT_EQUAL(u"xxxxxxxxxxxxxxxxxxxx"_ustr, getParagraph(2)->getString());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    dispatchCommand(mxComponent, u".uno:Cut"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(1, getParagraphs());

    // Create a new document
    createSwDoc();

    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(u"xxxxxxxxxxxxxxxxxxxx"_ustr, getParagraph(2)->getString());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    dispatchCommand(mxComponent, u".uno:Cut"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(1, getParagraphs());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(u"xxxxxxxxxxxxxxxxxxxx"_ustr, getParagraph(2)->getString());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    // Without the fix in place, this test would have crashed here
    SwXTextDocument* pTextDoc = getSwTextDoc();
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_RETURN);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(2, getParagraphs());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf145621)
{
    createSwDoc("tdf145621.odt");

    CPPUNIT_ASSERT_EQUAL(u"AAAAAA"_ustr, getParagraph(1)->getString());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    dispatchCommand(mxComponent, u".uno:Cut"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(u""_ustr, getParagraph(1)->getString());

    // Without the fix in place, this test would have crashed
    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(u"AAAAAA"_ustr, getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf134626)
{
    createSwDoc("tdf134626.odt");
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();

    CPPUNIT_ASSERT_EQUAL(u"Apple"_ustr, getParagraph(1)->getString());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});

    // Create a new document
    createSwDoc();
    pWrtShell = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    // Without the fix in place, this test would have crashed here
    for (sal_Int32 i = 0; i < 5; ++i)
    {
        dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});

        CPPUNIT_ASSERT_EQUAL(u"Apple"_ustr, getParagraph(1)->getString());

        dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});

        CPPUNIT_ASSERT_EQUAL(u"AppleApple"_ustr, getParagraph(1)->getString());

        dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

        CPPUNIT_ASSERT_EQUAL(u"Apple"_ustr, getParagraph(1)->getString());

        dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

        CPPUNIT_ASSERT_EQUAL(u""_ustr, getParagraph(1)->getString());
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf139566)
{
    createSwDoc();
    SwWrtShell* pWrtSh = getSwDocShell()->GetWrtShell();

    uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence(
        { { "Rows", uno::Any(sal_Int32(1)) }, { "Columns", uno::Any(sal_Int32(1)) } }));

    dispatchCommand(mxComponent, u".uno:InsertTable"_ustr, aArgs);

    // Move the cursor outside the table
    pWrtSh->Down(/*bSelect=*/false);

    pWrtSh->Insert(u"Test"_ustr);

    CPPUNIT_ASSERT_EQUAL(u"Test"_ustr, getParagraph(2)->getString());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    uno::Reference<frame::XFrames> xFrames = mxDesktop->getFrames();
    sal_Int32 nFrames = xFrames->getCount();

    // Create a second window so the first window looses focus
    dispatchCommand(mxComponent, u".uno:NewWindow"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(nFrames + 1, xFrames->getCount());

    dispatchCommand(mxComponent, u".uno:CloseWin"_ustr, {});

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

    dispatchCommand(mxComponent, u".uno:InsertTable"_ustr, aArgs);

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTextTable->getColumns()->getCount());

    dispatchCommand(mxComponent, u".uno:SelectTable"_ustr, {});
    dispatchCommand(mxComponent, u".uno:InsertRowsBefore"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTextTable->getColumns()->getCount());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTextTable->getColumns()->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf87199)
{
    createSwDoc();

    uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence(
        { { "Rows", uno::Any(sal_Int32(2)) }, { "Columns", uno::Any(sal_Int32(1)) } }));

    dispatchCommand(mxComponent, u".uno:InsertTable"_ustr, aArgs);

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTextTable->getColumns()->getCount());

    uno::Reference<text::XTextRange> xCellA1(xTextTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
    xCellA1->setString(u"test1"_ustr);

    uno::Reference<text::XTextRange> xCellA2(xTextTable->getCellByName(u"A2"_ustr), uno::UNO_QUERY);
    xCellA2->setString(u"test2"_ustr);

    dispatchCommand(mxComponent, u".uno:EntireColumn"_ustr, {});
    dispatchCommand(mxComponent, u".uno:MergeCells"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTextTable->getColumns()->getCount());

    CPPUNIT_ASSERT(xCellA1->getString().endsWith("test2"));

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTextTable->getColumns()->getCount());

    xCellA1.set(xTextTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);

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

    uno::Reference<text::XTextRange> xCellA1(xTextTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
    // deleted "1", inserted "2"
    CPPUNIT_ASSERT_EQUAL(u"12"_ustr, xCellA1->getString());
    uno::Reference<text::XTextRange> xCellA3(xTextTable->getCellByName(u"A3"_ustr), uno::UNO_QUERY);
    // This was 14 (bad sum: 2 + A1, where A1 was 12 instead of the correct 2)
    CPPUNIT_ASSERT_EQUAL(u"4"_ustr, xCellA3->getString());
    uno::Reference<text::XTextRange> xCellA4(xTextTable->getCellByName(u"A4"_ustr), uno::UNO_QUERY);
    // This was 28 (bad sum: 2 + A1 + A3, where A1 was 12 and A3 was 14)
    CPPUNIT_ASSERT_EQUAL(u"8"_ustr, xCellA4->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf146573)
{
    createSwDoc("tdf39828.fodt");
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();

    // remove redlines, add a footnote, and change the value
    // of the cell with the footnote
    dispatchCommand(mxComponent, u".uno:AcceptAllTrackedChanges"_ustr, {});
    pWrtShell->Right(SwCursorSkipMode::Cells, /*bSelect=*/false, /*nCount=*/1,
                     /*bBasicCall=*/false);
    dispatchCommand(mxComponent, u".uno:InsertFootnote"_ustr, {});
    dispatchCommand(mxComponent, u".uno:PageUp"_ustr, {}); // leave footnote
    pWrtShell->Left(SwCursorSkipMode::Cells, /*bSelect=*/false, /*nCount=*/1, /*bBasicCall=*/false);
    pWrtShell->Left(SwCursorSkipMode::Cells, /*bSelect=*/true, /*nCount=*/1, /*bBasicCall=*/false);
    pWrtShell->Insert(u"100"_ustr);

    // trigger recalculation by leaving the cell
    pWrtShell->Down(/*bSelect=*/false, /*nCount=*/1);

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xIndexAccess->getByIndex(0), uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xTextTable->getRows()->getCount());

    uno::Reference<text::XTextRange> xCellA1(xTextTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
    // value "100" and footnote index "1"
    CPPUNIT_ASSERT_EQUAL(u"1001"_ustr, xCellA1->getString());
    uno::Reference<text::XTextRange> xCellA3(xTextTable->getCellByName(u"A3"_ustr), uno::UNO_QUERY);
    // This was 4 (missing recalculation)
    CPPUNIT_ASSERT_EQUAL(u"102"_ustr, xCellA3->getString());
    uno::Reference<text::XTextRange> xCellA4(xTextTable->getCellByName(u"A4"_ustr), uno::UNO_QUERY);
    // This was 8 (missing recalculation)
    CPPUNIT_ASSERT_EQUAL(u"204"_ustr, xCellA4->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf157132)
{
    createSwDoc("tdf157132.odt");

    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();

    // Go to cell A2
    pWrtShell->Down(/*bSelect=*/false, /*nCount=*/1);

    // Select A2 and A3 and copy
    pWrtShell->Down(/*bSelect=*/true, /*nCount=*/1);

    dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});

    // Go to A4 and paste
    pWrtShell->Down(/*bSelect=*/false, /*nCount=*/1);

    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTables->getCount());

    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);

    uno::Reference<text::XTextRange> xCellA2(xTextTable->getCellByName(u"A2"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"2"_ustr, xCellA2->getString());
    uno::Reference<text::XTextRange> xCellA3(xTextTable->getCellByName(u"A3"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"3"_ustr, xCellA3->getString());
    uno::Reference<text::XTextRange> xCellA4(xTextTable->getCellByName(u"A4"_ustr), uno::UNO_QUERY);

    // Without the fix in place, this test would have failed with
    // - Expected: 6
    // - Actual  : 2
    CPPUNIT_ASSERT_EQUAL(u"6"_ustr, xCellA4->getString());
    uno::Reference<text::XTextRange> xCellA5(xTextTable->getCellByName(u"A5"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"7"_ustr, xCellA5->getString());

    xTextTable.set(xTables->getByIndex(1), uno::UNO_QUERY);

    xCellA2.set(xTextTable->getCellByName(u"A2"_ustr), uno::UNO_QUERY);

    // tdf#158336: Without the fix in place, this test would have failed with
    // - Expected: 2
    // - Actual  : ** Expression is faulty **
    CPPUNIT_ASSERT_EQUAL(u"2"_ustr, xCellA2->getString());
    xCellA3.set(xTextTable->getCellByName(u"A3"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"3"_ustr, xCellA3->getString());
    xCellA4.set(xTextTable->getCellByName(u"A4"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"6"_ustr, xCellA4->getString());
    xCellA5.set(xTextTable->getCellByName(u"A5"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"7"_ustr, xCellA5->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf147938)
{
    createSwDoc("tdf147938.fodt");

    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->getIDocumentRedlineAccess().GetRedlineTable().size());
    CPPUNIT_ASSERT_EQUAL(u"Bar\nbaz "_ustr,
                         pDoc->getIDocumentRedlineAccess().GetRedlineTable()[0]->GetText());

    pWrtShell->Down(/*bSelect=*/false, /*nCount=*/1);
    pWrtShell->TableToText('\t');

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->getIDocumentRedlineAccess().GetRedlineTable().size());
    CPPUNIT_ASSERT_EQUAL(u"Bar\nbaz "_ustr,
                         pDoc->getIDocumentRedlineAccess().GetRedlineTable()[0]->GetText());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    SwInsertTableOptions const opts(SwInsertTableFlags::NONE, 0);
    pWrtShell->TextToTable(opts, '\t', nullptr);

    pWrtShell->Undo();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->getIDocumentRedlineAccess().GetRedlineTable().size());
    CPPUNIT_ASSERT_EQUAL(u"Bar\nbaz "_ustr,
                         pDoc->getIDocumentRedlineAccess().GetRedlineTable()[0]->GetText());

    pWrtShell->Undo();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->getIDocumentRedlineAccess().GetRedlineTable().size());
    CPPUNIT_ASSERT_EQUAL(u"Bar\nbaz "_ustr,
                         pDoc->getIDocumentRedlineAccess().GetRedlineTable()[0]->GetText());

    pWrtShell->Redo();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->getIDocumentRedlineAccess().GetRedlineTable().size());
    CPPUNIT_ASSERT_EQUAL(u"Bar\nbaz "_ustr,
                         pDoc->getIDocumentRedlineAccess().GetRedlineTable()[0]->GetText());

    pWrtShell->Redo();

    pWrtShell->Undo();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->getIDocumentRedlineAccess().GetRedlineTable().size());
    CPPUNIT_ASSERT_EQUAL(u"Bar\nbaz "_ustr,
                         pDoc->getIDocumentRedlineAccess().GetRedlineTable()[0]->GetText());

    pWrtShell->Undo();

    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->getIDocumentRedlineAccess().GetRedlineTable().size());
    CPPUNIT_ASSERT_EQUAL(u"Bar\nbaz "_ustr,
                         pDoc->getIDocumentRedlineAccess().GetRedlineTable()[0]->GetText());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf148799)
{
    // load a document with table formulas with comma delimiter,
    // but with a document language with default point delimiter
    createSwDoc("tdf148799.docx");
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();

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

    uno::Reference<text::XTextRange> xCellA1(xTextTable->getCellByName(u"D3"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"2.3"_ustr, xCellA1->getString());
    uno::Reference<text::XTextRange> xCellA3(xTextTable->getCellByName(u"D4"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"2345"_ustr, xCellA3->getString());
    uno::Reference<text::XTextRange> xCellA4(xTextTable->getCellByName(u"D5"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"23684.5"_ustr, xCellA4->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf151993)
{
    // load a document with table formulas with comma delimiter
    // (with a document language with default comma delimiter)
    createSwDoc("tdf151993.docx");
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();

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
    uno::Reference<text::XTextRange> xCellA1(xTextTable->getCellByName(u"A2"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"30"_ustr, xCellA1->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf148849)
{
    // load a document with a table and an empty paragraph before the table
    createSwDoc("tdf148849.fodt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();

    // record changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    // hide changes
    dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});
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
    CPPUNIT_ASSERT_EQUAL(u"Row 2"_ustr, rNode.GetTextNode()->GetText());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf150576)
{
    // load a document with a table and an empty paragraph before the table
    createSwDoc("tdf148849.fodt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();

    // record changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    // hide changes
    dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});
    CPPUNIT_ASSERT(pWrtShell->GetLayout()->IsHideRedlines());

    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());

    // Check deletion of the first row, if the second row deleted already

    // put cursor in the second table row
    pWrtShell->Down(/*bSelect=*/false, /*nCount=*/2);
    SwNode& rNode = pWrtShell->GetCursor()->GetPoint()->GetNode();
    CPPUNIT_ASSERT_EQUAL(u"Row 2"_ustr, rNode.GetTextNode()->GetText());

    // delete the second table row
    pWrtShell->DeleteRow();

    // check cursor position (row 3)
    SwNode& rNode2 = pWrtShell->GetCursor()->GetPoint()->GetNode();
    CPPUNIT_ASSERT_EQUAL(u"Row 3"_ustr, rNode2.GetTextNode()->GetText());

    // put cursor in the first row
    pWrtShell->Up(/*bSelect=*/false, /*nCount=*/1);
    SwNode& rNode3 = pWrtShell->GetCursor()->GetPoint()->GetNode();
    CPPUNIT_ASSERT_EQUAL(u"12"_ustr, rNode3.GetTextNode()->GetText());

    // delete the first row
    pWrtShell->DeleteRow();

    // This was empty (cursor jumped in the start of the document instead of
    // the next not deleted row)
    SwNode& rNode4 = pWrtShell->GetCursor()->GetPoint()->GetNode();
    CPPUNIT_ASSERT_EQUAL(u"Row 3"_ustr, rNode4.GetTextNode()->GetText());

    // Check skipping previous lines

    // restore deleted rows
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    SwNode& rNode5 = pWrtShell->GetCursor()->GetPoint()->GetNode();
    CPPUNIT_ASSERT_EQUAL(u"Row 2"_ustr, rNode5.GetTextNode()->GetText());

    // delete the second row
    pWrtShell->DeleteRow();
    SwNode& rNode7 = pWrtShell->GetCursor()->GetPoint()->GetNode();
    CPPUNIT_ASSERT_EQUAL(u"Row 3"_ustr, rNode7.GetTextNode()->GetText());

    // delete the third, i.e. last row
    pWrtShell->DeleteRow();
    SwNode& rNode8 = pWrtShell->GetCursor()->GetPoint()->GetNode();

    // This was empty (cursor jumped in the start of the document instead of
    // the previous not deleted row)
    CPPUNIT_ASSERT_EQUAL(u"12"_ustr, rNode8.GetTextNode()->GetText());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf132603)
{
    createSwDoc();

    uno::Sequence<beans::PropertyValue> aPropertyValues
        = comphelper::InitPropertySequence({ { "Text", uno::Any(u"Comment"_ustr) } });

    dispatchCommand(mxComponent, u".uno:InsertAnnotation"_ustr, aPropertyValues);

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    // Without the fix in place, it would crash here
    dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});

    tools::JsonWriter aJsonWriter;
    SwXTextDocument* pTextDoc = getSwTextDoc();
    pTextDoc->getPostIts(aJsonWriter);
    OString pChar = aJsonWriter.finishAndGetAsOString();
    std::stringstream aStream((std::string(pChar)));
    boost::property_tree::ptree aTree;
    boost::property_tree::read_json(aStream, aTree);
    for (const boost::property_tree::ptree::value_type& rValue : aTree.get_child("comments"))
    {
        const boost::property_tree::ptree& rComment = rValue.second;

        OString aText(rComment.get<std::string>("html"));
        CPPUNIT_ASSERT_EQUAL("<div>Comment</div>"_ostr, aText);
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf117601)
{
    createSwDoc();

    uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence(
        { { "Rows", uno::Any(sal_Int32(5)) }, { "Columns", uno::Any(sal_Int32(3)) } }));

    dispatchCommand(mxComponent, u".uno:InsertTable"_ustr, aArgs);

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTextTable->getColumns()->getCount());

    uno::Reference<text::XTextRange> xCellB1(xTextTable->getCellByName(u"B1"_ustr), uno::UNO_QUERY);
    xCellB1->setString(u"test1"_ustr);

    uno::Reference<text::XTextRange> xCellB2(xTextTable->getCellByName(u"B2"_ustr), uno::UNO_QUERY);
    xCellB2->setString(u"test2"_ustr);

    //go to middle row
    SwXTextDocument* pTextDoc = getSwTextDoc();
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_UP);
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_RIGHT);
    Scheduler::ProcessEventsToIdle();

    dispatchCommand(mxComponent, u".uno:EntireColumn"_ustr, {});
    dispatchCommand(mxComponent, u".uno:MergeCells"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTextTable->getColumns()->getCount());

    CPPUNIT_ASSERT(xCellB1->getString().endsWith("test2"));

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

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
    dispatchCommand(mxComponent, u".uno:SetAnchorToPage"_ustr, {});

    //position has changed
    CPPUNIT_ASSERT(aPos.X < xShape->getPosition().X);
    CPPUNIT_ASSERT(aPos.Y < xShape->getPosition().Y);

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

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

    dispatchCommand(mxComponent, u".uno:SetAnchorToPage"_ustr, {});

    //position has changed
    CPPUNIT_ASSERT(aPos.X < xShape->getPosition().X);
    CPPUNIT_ASSERT(aPos.Y < xShape->getPosition().Y);

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    //Without the fix in place, this test would have failed with
    //- Expected: 2447
    //- Actual  : 446
    CPPUNIT_ASSERT_EQUAL(aPos.X, xShape->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(aPos.Y, xShape->getPosition().Y);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf145207)
{
    createSwDoc("tdf145207.odt");

    CPPUNIT_ASSERT_EQUAL(1, getPages());
    CPPUNIT_ASSERT_EQUAL(3, getShapes());

    //select one shape and use the TAB key to iterate over the different shapes
    selectShape(1);

    SwXTextDocument* pTextDoc = getSwTextDoc();
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

    CPPUNIT_ASSERT_EQUAL(2, getShapes());
    uno::Reference<drawing::XShape> xShape1 = getShape(1);
    uno::Reference<drawing::XShape> xShape2 = getShape(2);

    awt::Point aPos[2];
    aPos[0] = xShape1->getPosition();
    aPos[1] = xShape2->getPosition();

    //select shape 2 and move it down
    selectShape(2);

    SwXTextDocument* pTextDoc = getSwTextDoc();
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_DOWN);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(aPos[0].X, xShape1->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(aPos[0].Y, xShape1->getPosition().Y);
    CPPUNIT_ASSERT_EQUAL(aPos[1].X, xShape2->getPosition().X);
    //Y position in shape 2 has changed
    CPPUNIT_ASSERT(aPos[1].Y < xShape2->getPosition().Y);

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

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

    CPPUNIT_ASSERT_EQUAL(2, getShapes());
    CPPUNIT_ASSERT_EQUAL(2, getPages());

    uno::Reference<drawing::XShape> xShape1 = getShape(1);
    uno::Reference<drawing::XShape> xShape2 = getShape(2);

    awt::Point aPos[2];
    aPos[0] = xShape1->getPosition();
    aPos[1] = xShape2->getPosition();

    //select shape 1 and move it down
    selectShape(1);

    SwXTextDocument* pTextDoc = getSwTextDoc();
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_DOWN);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(aPos[0].X, xShape1->getPosition().X);
    //Y position in shape 1 has changed
    CPPUNIT_ASSERT(aPos[0].Y < xShape1->getPosition().Y);
    CPPUNIT_ASSERT_EQUAL(aPos[1].X, xShape2->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(aPos[1].Y, xShape2->getPosition().Y);

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

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

    CPPUNIT_ASSERT_EQUAL(1, getShapes());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    dispatchCommand(mxComponent, u".uno:Cut"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(0, getShapes());

    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(1, getShapes());

    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(2, getShapes());

    uno::Reference<drawing::XShape> xShape1 = getShape(1);
    uno::Reference<drawing::XShape> xShape2 = getShape(2);

    awt::Point aPos[2];
    aPos[0] = xShape1->getPosition();
    aPos[1] = xShape2->getPosition();

    //select shape 2 and move it to the right
    selectShape(2);

    SwXTextDocument* pTextDoc = getSwTextDoc();
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
        dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

        // Without the fix in place, undo action would have changed shape1's position
        // and this test would have failed with
        // - Expected: -139
        // - Actual  : 1194
        CPPUNIT_ASSERT_EQUAL(aPos[0].X, xShape1->getPosition().X);
        CPPUNIT_ASSERT_EQUAL(aPos[0].Y, xShape1->getPosition().Y);
        CPPUNIT_ASSERT(aPos[1].X < xShape2->getPosition().X);
        CPPUNIT_ASSERT_EQUAL(aPos[1].Y, xShape2->getPosition().Y);
    }

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(aPos[0].X, xShape1->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(aPos[0].Y, xShape1->getPosition().Y);
    // Shape 2 has come back to the original position
    CPPUNIT_ASSERT_EQUAL(aPos[1].X, xShape2->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(aPos[1].Y, xShape2->getPosition().Y);

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(1, getShapes());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(0, getShapes());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(1, getShapes());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf132637_protectTrackChanges)
{
    createSwDoc("tdf132637_protectTrackChanges.doc");

    // The password should only prevent turning off track changes, not open as read-only
    CPPUNIT_ASSERT(!getSwDocShell()->IsReadOnly());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf127652)
{
    createSwDoc("tdf127652.odt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* const pWrtShell = getSwDocShell()->GetWrtShell();

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
    CPPUNIT_ASSERT(pShell);
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
        u"AnchorType"_ustr, uno::Any(text::TextContentAnchorType::TextContentAnchorType_AT_PAGE));

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

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Cut"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(0, getShapes());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

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
        = drawing::GraphicExportFilter::create(m_xContext);
    uno::Reference<lang::XComponent> xSourceDoc(getShape(1), uno::UNO_QUERY);
    xGraphicExporter->setSourceDocument(xSourceDoc);

    SvMemoryStream aStream;
    uno::Reference<io::XOutputStream> xOutputStream(new utl::OStreamWrapper(aStream));
    uno::Sequence<beans::PropertyValue> aDescriptor(comphelper::InitPropertySequence(
        { { "OutputStream", uno::Any(xOutputStream) }, { "FilterName", uno::Any(u"BMP"_ustr) } }));
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
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();

    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    uno::Reference<drawing::XShape> xShape(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3579), xShape->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4090), xShape->getPosition().Y);

    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pObject = pPage->GetObj(1);
    SwContact* pTextBox = static_cast<SwContact*>(pObject->GetUserCall());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(RES_FLYFRMFMT), pTextBox->GetFormat()->Which());

    pWrtShell->SelectObj(Point(), 0, pObject);

    SwXTextDocument* pTextDoc = getSwTextDoc();
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

    uno::Reference<text::XTextRange> xCell(xTextTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(Color(0x009353), getProperty<Color>(xCell, u"BackColor"_ustr));

    xCell.set(xTextTable->getCellByName(u"A2"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(COL_AUTO, getProperty<Color>(xCell, u"BackColor"_ustr));

    xCell.set(xTextTable->getCellByName(u"A3"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(Color(0xdddddd), getProperty<Color>(xCell, u"BackColor"_ustr));

    xCell.set(xTextTable->getCellByName(u"A4"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(COL_AUTO, getProperty<Color>(xCell, u"BackColor"_ustr));

    xCell.set(xTextTable->getCellByName(u"A5"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(Color(0xdddddd), getProperty<Color>(xCell, u"BackColor"_ustr));

    xCell.set(xTextTable->getCellByName(u"A6"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(Color(0xbee3d3), getProperty<Color>(xCell, u"BackColor"_ustr));

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Cut"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xIndexAccess->getCount());

    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});

    xTextTable.set(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTextTable->getColumns()->getCount());

    dispatchCommand(mxComponent, u".uno:GoUp"_ustr, {});

    SwXTextDocument* pTextDoc = getSwTextDoc();
    for (sal_Int32 i = 0; i < 6; ++i)
    {
        pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
        Scheduler::ProcessEventsToIdle();
    }

    for (sal_Int32 i = 0; i < 5; ++i)
    {
        dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    }

    xTextTable.set(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTextTable->getColumns()->getCount());

    for (sal_Int32 i = 0; i < 5; ++i)
    {
        dispatchCommand(mxComponent, u".uno:Redo"_ustr, {});
    }

    xTextTable.set(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(9), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTextTable->getColumns()->getCount());

    xCell.set(xTextTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(Color(0x009353), getProperty<Color>(xCell, u"BackColor"_ustr));

    xCell.set(xTextTable->getCellByName(u"A2"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(COL_AUTO, getProperty<Color>(xCell, u"BackColor"_ustr));

    xCell.set(xTextTable->getCellByName(u"A3"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(Color(0xdddddd), getProperty<Color>(xCell, u"BackColor"_ustr));

    xCell.set(xTextTable->getCellByName(u"A4"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(COL_AUTO, getProperty<Color>(xCell, u"BackColor"_ustr));

    xCell.set(xTextTable->getCellByName(u"A5"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(Color(0xdddddd), getProperty<Color>(xCell, u"BackColor"_ustr));

    xCell.set(xTextTable->getCellByName(u"A6"_ustr), uno::UNO_QUERY);

    // Without the fix in place, this test would have failed with
    // - Expected: Color: R:255 G:255 B:255 A:255
    // - Actual  : Color: R:190 G:227 B:211 A:0
    CPPUNIT_ASSERT_EQUAL(COL_AUTO, getProperty<Color>(xCell, u"BackColor"_ustr));

    xCell.set(xTextTable->getCellByName(u"A7"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(Color(0xdddddd), getProperty<Color>(xCell, u"BackColor"_ustr));

    xCell.set(xTextTable->getCellByName(u"A8"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(COL_AUTO, getProperty<Color>(xCell, u"BackColor"_ustr));

    xCell.set(xTextTable->getCellByName(u"A9"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(Color(0xbee3d3), getProperty<Color>(xCell, u"BackColor"_ustr));
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

    uno::Reference<text::XTextRange> xCell(xTextTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xCell->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(xPara, u"CharWeight"_ustr));

    xCell.set(xTextTable->getCellByName(u"A2"_ustr), uno::UNO_QUERY);
    xParaEnumAccess.set(xCell->getText(), uno::UNO_QUERY);
    xParaEnum.set(xParaEnumAccess->createEnumeration());
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::NORMAL, getProperty<float>(xPara, u"CharWeight"_ustr));

    xCell.set(xTextTable->getCellByName(u"A3"_ustr), uno::UNO_QUERY);
    xParaEnumAccess.set(xCell->getText(), uno::UNO_QUERY);
    xParaEnum.set(xParaEnumAccess->createEnumeration());
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::NORMAL, getProperty<float>(xPara, u"CharWeight"_ustr));

    xCell.set(xTextTable->getCellByName(u"A4"_ustr), uno::UNO_QUERY);
    xParaEnumAccess.set(xCell->getText(), uno::UNO_QUERY);
    xParaEnum.set(xParaEnumAccess->createEnumeration());
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(xPara, u"CharWeight"_ustr));

    dispatchCommand(mxComponent, u".uno:GoDown"_ustr, {});
    dispatchCommand(mxComponent, u".uno:GoDown"_ustr, {});
    dispatchCommand(mxComponent, u".uno:LineDownSel"_ustr, {});
    dispatchCommand(mxComponent, u".uno:DeleteRows"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTextTable->getColumns()->getCount());

    xCell.set(xTextTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
    xParaEnumAccess.set(xCell->getText(), uno::UNO_QUERY);
    xParaEnum.set(xParaEnumAccess->createEnumeration());
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(xPara, u"CharWeight"_ustr));

    xCell.set(xTextTable->getCellByName(u"A2"_ustr), uno::UNO_QUERY);
    xParaEnumAccess.set(xCell->getText(), uno::UNO_QUERY);
    xParaEnum.set(xParaEnumAccess->createEnumeration());
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(xPara, u"CharWeight"_ustr));

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTextTable->getColumns()->getCount());

    xCell.set(xTextTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
    xParaEnumAccess.set(xCell->getText(), uno::UNO_QUERY);
    xParaEnum.set(xParaEnumAccess->createEnumeration());
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(xPara, u"CharWeight"_ustr));

    xCell.set(xTextTable->getCellByName(u"A2"_ustr), uno::UNO_QUERY);
    xParaEnumAccess.set(xCell->getText(), uno::UNO_QUERY);
    xParaEnum.set(xParaEnumAccess->createEnumeration());
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);

    // Without the fix in place, this test would have failed with
    // - Expected: 100
    // - Actual  : 150
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::NORMAL, getProperty<float>(xPara, u"CharWeight"_ustr));

    xCell.set(xTextTable->getCellByName(u"A3"_ustr), uno::UNO_QUERY);
    xParaEnumAccess.set(xCell->getText(), uno::UNO_QUERY);
    xParaEnum.set(xParaEnumAccess->createEnumeration());
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::NORMAL, getProperty<float>(xPara, u"CharWeight"_ustr));

    xCell.set(xTextTable->getCellByName(u"A4"_ustr), uno::UNO_QUERY);
    xParaEnumAccess.set(xCell->getText(), uno::UNO_QUERY);
    xParaEnum.set(xParaEnumAccess->createEnumeration());
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(xPara, u"CharWeight"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf138897)
{
    createSwDoc("tdf100018-1.odt");

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Cut"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});
    // this was crashing
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Redo"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Redo"_ustr, {});
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf136740)
{
    createSwDoc();
    css::uno::Reference<css::lang::XMultiServiceFactory> xFact(mxComponent,
                                                               css::uno::UNO_QUERY_THROW);
    css::uno::Reference<css::beans::XPropertySet> xTextDefaults(
        xFact->createInstance(u"com.sun.star.text.Defaults"_ustr), css::uno::UNO_QUERY_THROW);
    const css::uno::Any aOrig = xTextDefaults->getPropertyValue(u"TabStopDistance"_ustr);
    sal_Int32 nDefTab = aOrig.get<sal_Int32>();
    CPPUNIT_ASSERT(nDefTab != 0);

    css::uno::Reference<css::text::XTextRange> const xParagraph(getParagraphOrTable(1),
                                                                css::uno::UNO_QUERY_THROW);
    xParagraph->setString(u"Foo"_ustr);

    CPPUNIT_ASSERT_EQUAL(1, getParagraphs());
    CPPUNIT_ASSERT_EQUAL(u"Foo"_ustr, xParagraph->getString());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});
    dispatchCommand(mxComponent, u".uno:GoToEndOfDoc"_ustr, {});

    const css::uno::Any aNew(nDefTab * 2);
    xTextDefaults->setPropertyValue(u"TabStopDistance"_ustr, aNew);
    // it may become slightly different because of conversions, so get the actual value
    const css::uno::Any aNewCorrected = xTextDefaults->getPropertyValue(u"TabStopDistance"_ustr);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(nDefTab * 2, aNewCorrected.get<sal_Int32>(), 1);

    // Paste special as RTF
    const auto aPropertyValues = comphelper::InitPropertySequence(
        { { "SelectedFormat",
            css::uno::Any(static_cast<sal_uInt32>(SotClipboardFormatId::RTF)) } });
    dispatchCommand(mxComponent, u".uno:ClipboardFormatItems"_ustr, aPropertyValues);

    CPPUNIT_ASSERT_EQUAL(1, getParagraphs());
    CPPUNIT_ASSERT_EQUAL(u"FooFoo"_ustr, xParagraph->getString());

    // Without the fix in place, this would fail with
    //     equality assertion failed
    //     - Expected: <Any: (long) 2501>
    //     - Actual  : <Any: (long) 1251>
    // i.e., pasting RTF would reset the modified default tab stop distance to hardcoded default
    CPPUNIT_ASSERT_EQUAL(aNewCorrected, xTextDefaults->getPropertyValue(u"TabStopDistance"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf128106)
{
    createSwDoc("cross_reference_demo_bmk.odt");

    const auto aPropertyValues
        = comphelper::InitPropertySequence({ { "FileName", css::uno::Any(maTempFile.GetURL()) } });
    dispatchCommand(mxComponent, u".uno:NewGlobalDoc"_ustr, aPropertyValues);

    // Use loadFromDesktop instead of loadFromURL to avoid calling mxComponent->dispose()
    // Otherwise it fails with 'DeInitVCL: some top Windows are still alive'
    mxComponent = loadFromDesktop(maTempFile.GetURL());

    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    SwDoc* const pMasterDoc(pWrtShell->GetDoc());
    CPPUNIT_ASSERT_EQUAL(
        size_t(2),
        pMasterDoc->getIDocumentLinksAdministration().GetLinkManager().GetLinks().size());
    // no way to set SwDocShell::m_nUpdateDocMode away from NO_UPDATE ?
    // pMasterDoc->getIDocumentLinksAdministration().UpdateLinks();
    pMasterDoc->getIDocumentLinksAdministration().GetLinkManager().UpdateAllLinks(
        false, false, nullptr, u""_ustr);
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
    CPPUNIT_ASSERT_EQUAL(u"bookmarkchapter1_text"_ustr,
                         static_cast<SwGetRefField const*>(fields[0]->GetField())->GetSetRefName());
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr,
                         static_cast<SwGetRefField const*>(fields[0]->GetField())
                             ->GetExpandedTextOfReferencedTextNode(*pWrtShell->GetLayout()));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_BOOKMARK), fields[1]->GetField()->GetSubType());
    CPPUNIT_ASSERT(
        static_cast<SwGetRefField const*>(fields[1]->GetField())->IsRefToHeadingCrossRefBookmark());
    CPPUNIT_ASSERT_EQUAL(u"Chapter 2"_ustr,
                         static_cast<SwGetRefField const*>(fields[1]->GetField())->GetPar2());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_BOOKMARK), fields[2]->GetField()->GetSubType());
    CPPUNIT_ASSERT_EQUAL(u"Bookmarkchapter1"_ustr,
                         static_cast<SwGetRefField const*>(fields[2]->GetField())->GetSetRefName());
    CPPUNIT_ASSERT_EQUAL(u"Chapter 1"_ustr,
                         static_cast<SwGetRefField const*>(fields[2]->GetField())->GetPar2());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_BOOKMARK), fields[3]->GetField()->GetSubType());
    CPPUNIT_ASSERT_EQUAL(u"bookmarkchapter1_text"_ustr,
                         static_cast<SwGetRefField const*>(fields[3]->GetField())->GetSetRefName());
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr,
                         static_cast<SwGetRefField const*>(fields[3]->GetField())->GetPar2());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_BOOKMARK), fields[4]->GetField()->GetSubType());
    CPPUNIT_ASSERT(
        static_cast<SwGetRefField const*>(fields[4]->GetField())->IsRefToHeadingCrossRefBookmark());
    CPPUNIT_ASSERT_EQUAL(u"Chapter 1.1"_ustr,
                         static_cast<SwGetRefField const*>(fields[4]->GetField())->GetPar2());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(REF_BOOKMARK), fields[5]->GetField()->GetSubType());
    CPPUNIT_ASSERT(
        static_cast<SwGetRefField const*>(fields[5]->GetField())->IsRefToHeadingCrossRefBookmark());
    CPPUNIT_ASSERT_EQUAL(u"Chapter 2"_ustr,
                         static_cast<SwGetRefField const*>(fields[5]->GetField())->GetPar2());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf103612)
{
    createSwGlobalDoc("DUMMY.odm");
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT_EQUAL(
        size_t(1), pDoc->getIDocumentLinksAdministration().GetLinkManager().GetLinks().size());
    pDoc->getIDocumentLinksAdministration().GetLinkManager().UpdateAllLinks(false, false, nullptr,
                                                                            u""_ustr);

    xmlDocUniquePtr pLayout = parseLayoutDump();

    assertXPath(pLayout, "/root/page[1]/body/section[1]/txt[1]/SwParaPortion/SwLineLayout[1]",
                "portion", u"Text before section");
    // the inner section and its content was hidden
    assertXPath(pLayout, "/root/page[1]/body/section[2]/txt[1]/SwParaPortion/SwLineLayout[1]",
                "portion", u"Text inside section before ToC");
    assertXPath(pLayout, "/root/page[1]/body/section[3]/txt[1]/SwParaPortion/SwLineLayout[1]",
                "portion", u"Table of Contents");
    assertXPath(pLayout, "/root/page[1]/body/section[4]/txt[1]/SwParaPortion/SwLineLayout[1]",
                "portion", u"First header*1");
    assertXPath(pLayout, "/root/page[1]/body/section[4]/txt[2]/SwParaPortion/SwLineLayout[1]",
                "portion", u"Second header*1");
    assertXPath(pLayout, "/root/page[1]/body/section[5]/txt[2]/SwParaPortion/SwLineLayout[1]",
                "portion", u"Text inside section after ToC");
    assertXPath(pLayout, "/root/page[1]/body/section[6]/txt[1]/SwParaPortion/SwLineLayout[1]",
                "portion", u"Text after section");
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf121119)
{
    createSwGlobalDoc("tdf121119.odm");
    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT_EQUAL(
        size_t(2), pDoc->getIDocumentLinksAdministration().GetLinkManager().GetLinks().size());
    pDoc->getIDocumentLinksAdministration().GetLinkManager().UpdateAllLinks(false, false, nullptr,
                                                                            u""_ustr);

    uno::Reference<text::XTextGraphicObjectsSupplier> xTextGraphicObjectsSupplier(mxComponent,
                                                                                  uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(
        xTextGraphicObjectsSupplier->getGraphicObjects(), uno::UNO_QUERY);

    // This was 0 (missing images anchored at page)
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xIndexAccess->getCount());

    uno::Reference<drawing::XShape> xShape(xIndexAccess->getByIndex(0), uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_PAGE,
                         getProperty<text::TextContentAnchorType>(xShape, u"AnchorType"_ustr));

    xmlDocUniquePtr pLayout = parseLayoutDump();
    // check page numbers of the objects anchored at page
    assertXPath(pLayout, "/root/page[2]/anchored/fly/SwAnchoredObject", 1);
    assertXPath(pLayout, "/root/page[4]/anchored/fly/SwAnchoredObject", 1);
    assertXPath(pLayout, "/root/page[7]/anchored/fly/SwAnchoredObject", 1);
    assertXPath(pLayout, "/root/page[9]/anchored/fly/SwAnchoredObject", 1);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf97899)
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    SwCursorShell* pShell(pDoc->GetEditShell());
    CPPUNIT_ASSERT(pShell);
    SwPaM* pCursor = pShell->GetCursor();

    IDocumentContentOperations& rIDCO(pDoc->getIDocumentContentOperations());

    // Create an Ordered List
    rIDCO.InsertString(*pCursor, u"\ta"_ustr);
    pWrtShell->SplitNode();
    rIDCO.InsertString(*pCursor, u"   b"_ustr);
    pWrtShell->SplitNode();
    rIDCO.InsertString(*pCursor, u"  \t  c"_ustr);

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:DefaultNumbering"_ustr, {});

    // tdf#109285: RemoveLeadingWhiteSpace from all numbered paragraphs
    getParagraph(1, u"a"_ustr);
    getParagraph(2, u"b"_ustr);
    getParagraph(3, u"c"_ustr);

    // Save it as DOCX & load it again
    saveAndReload(u"Office Open XML Text"_ustr);
    uno::Reference<container::XIndexAccess> xNumberingRules
        = getProperty<uno::Reference<container::XIndexAccess>>(getParagraph(1),
                                                               u"NumberingRules"_ustr);
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
    dispatchCommand(mxComponent, u".uno:UpdateAllIndexes"_ustr, {});

    xmlDocUniquePtr pLayout = parseLayoutDump();
    // Without the fix in place, this test would have failed with
    // - Expected: 2
    // - Actual  : 4
    assertXPath(pLayout, "/root/page[1]/body/section[2]/txt", 2);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf151462)
{
    createSwDoc("tdf151462.odt");
    dispatchCommand(mxComponent, u".uno:UpdateAllIndexes"_ustr, {});

    xmlDocUniquePtr pLayout = parseLayoutDump();
    // Without the fix in place, there would be just the first index entry
    assertXPath(pLayout,
                "/root/page[1]/body/txt[2]/anchored/fly/section/txt[1]/SwParaPortion/"
                "SwLineLayout[1]/SwLinePortion[1]",
                "portion", u"sub one");
    assertXPath(pLayout,
                "/root/page[1]/body/txt[2]/anchored/fly/section/txt[2]/SwParaPortion/"
                "SwLineLayout[1]/SwLinePortion[1]",
                "portion", u"sub two");
    assertXPath(pLayout,
                "/root/page[1]/body/txt[2]/anchored/fly/section/txt[3]/SwParaPortion/"
                "SwLineLayout[1]/SwLinePortion[1]",
                "portion", u"sub three");

    // Without the fix in place, there would be just the first index entry
    assertXPath(pLayout,
                "/root/page[1]/body/txt[6]/anchored/fly/section/txt[1]/SwParaPortion/"
                "SwLineLayout[1]/SwLinePortion[1]",
                "portion", u"another sub one");
    assertXPath(pLayout,
                "/root/page[1]/body/txt[6]/anchored/fly/section/txt[2]/SwParaPortion/"
                "SwLineLayout[1]/SwLinePortion[1]",
                "portion", u"another sub two");
    assertXPath(pLayout,
                "/root/page[1]/body/txt[6]/anchored/fly/section/txt[3]/SwParaPortion/"
                "SwLineLayout[1]/SwLinePortion[1]",
                "portion", u"another sub three");
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf153636)
{
    createSwDoc("tdf153636.odt");
    dispatchCommand(mxComponent, u".uno:UpdateAllIndexes"_ustr, {});
    saveAndReload(u"writer8"_ustr);

    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    for (int i = 1; i <= 3; i += 2)
    {
        const OUString frameStyleName
            = getXPath(pXmlDoc,
                       "/office:document-content/office:body/office:text/"
                       "text:user-index[@text:name='User-Defined1']/text:index-body/text:p["
                           + OString::number(i) + "]",
                       "style-name");
        const OUString tableStyleName
            = getXPath(pXmlDoc,
                       "/office:document-content/office:body/office:text/"
                       "text:user-index[@text:name='User-Defined1']/text:index-body/text:p["
                           + OString::number(i + 1) + "]",
                       "style-name");

        // Without the fix in place, the frame and table indentation would differ
        CPPUNIT_ASSERT_EQUAL(frameStyleName, tableStyleName);
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf157129)
{
    // Unit test for tdf#157129
    // Test to see if cursor moves to the end after paste

    // First document containing test data
    createSwDoc("tdf157129.doc");
    CPPUNIT_ASSERT_EQUAL(5, getParagraphs());
    // Copy data from first document
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});

    // Create a new document
    createSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();

    CPPUNIT_ASSERT_EQUAL(1, getParagraphs());

    // Paste data from first document
    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(5, getParagraphs());

    // Save cursor position after paste occurs
    SwPosition aCursorPosPaste(*pWrtShell->GetCursor()->GetPoint());

    // Move cursor position to the end
    pWrtShell->SttEndDoc(false); //bStart = false

    // Save cursor position at end
    SwPosition aCursorPosEnd(*pWrtShell->GetCursor()->GetPoint());

    // Assert the cursor position after paste is at the end
    // Without the test in place, the cursor position is at the beginning of the document
    // - Expected : SwPosition (node 18, offset 0)
    // - Actual : SwPosition (node 6, offset 0)
    CPPUNIT_ASSERT_EQUAL(aCursorPosEnd, aCursorPosPaste);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testCursorPositionAfterUndo)
{
    createSwDoc("cursor_position_after_undo.odt");
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();

    // switch on "Outline Folding" mode
    dispatchCommand(mxComponent, u".uno:ShowOutlineContentVisibilityButton"_ustr, {});
    CPPUNIT_ASSERT(pWrtShell->GetViewOptions()->IsShowOutlineContentVisibilityButton());

    // move the cursor to the beginning of the 3rd word in the 3rd paragraph, "tincidunt"
    pWrtShell->FwdPara();
    pWrtShell->FwdPara();
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 16, /*bBasicCall=*/false);

    // select the word
    dispatchCommand(mxComponent, u".uno:SelectWord"_ustr, {});

    // check the word is select
    SwShellCursor* pShellCursor = pWrtShell->getShellCursor(false);
    CPPUNIT_ASSERT_EQUAL(u"tincidunt"_ustr, pShellCursor->GetText());

    // remember the cursor position for comparison
    SwPosition aCursorPos(*pWrtShell->GetCursor()->GetPoint());

    // delete the selected word
    pWrtShell->Delete();

    // undo delete
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    // without the fix in place, the cursor would have been set to the start of the outline node
    // - Expected: SwPosition (node 11, offset 25)
    // - Actual  : SwPosition (node 9, offset 0)
    CPPUNIT_ASSERT_EQUAL(aCursorPos, *pWrtShell->GetCursor()->GetPoint());

    // switch off "Outline Folding" mode
    dispatchCommand(mxComponent, u".uno:ShowOutlineContentVisibilityButton"_ustr, {});
    CPPUNIT_ASSERT(!pWrtShell->GetViewOptions()->IsShowOutlineContentVisibilityButton());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf73483)
{
    // Given a document with a first paragraph having a manually set page break with page style
    createSwDoc("pageBreakWithPageStyle.fodt");
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();

    CPPUNIT_ASSERT_EQUAL(u"Right Page"_ustr, pWrtShell->GetCurPageStyle());

    dispatchCommand(mxComponent, u".uno:ResetAttributes"_ustr,
                    {}); // Ctrl+M "Clear Direct Formatting"
    // Make sure that clearing direct formatting doesn't clear the page style
    CPPUNIT_ASSERT_EQUAL(u"Right Page"_ustr, pWrtShell->GetCurPageStyle());

    // Make sure that the page break with page style survives ODF save-and-reload
    saveAndReload(u"writer8"_ustr);

    xmlDocUniquePtr pXml = parseExport(u"content.xml"_ustr);
    CPPUNIT_ASSERT(pXml);
    OUString para_style_name
        = getXPath(pXml, "/office:document-content/office:body/office:text/text:p", "style-name");
    // Without the fix in place, this would fail
    CPPUNIT_ASSERT(!para_style_name.equalsIgnoreAsciiCase("Standard"));

    OString para_style_path
        = "/office:document-content/office:automatic-styles/style:style[@style:name='"
          + para_style_name.toUtf8() + "']";
    assertXPath(pXml, para_style_path, "family", u"paragraph");
    // Without the fix in place, the autostyle had no parent
    assertXPath(pXml, para_style_path, "parent-style-name", u"Standard");
    assertXPath(pXml, para_style_path, "master-page-name", u"Right_20_Page");
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf62032ApplyStyle)
{
    createSwDoc("tdf62032_apply_style.odt");
    SwWrtShell* pWrtSh = getSwDocShell()->GetWrtShell();

    pWrtSh->Down(/*bSelect=*/false);

    uno::Sequence<beans::PropertyValue> aPropertyValues = comphelper::InitPropertySequence({
        { "Style", uno::Any(u"A 2"_ustr) },
        { "FamilyName", uno::Any(u"ParagraphStyles"_ustr) },
    });
    dispatchCommand(mxComponent, u".uno:StyleApply"_ustr, aPropertyValues);

    // Without the fix in place, it fails with:
    // - Expected: 1.1
    // - Actual  : 2
    CPPUNIT_ASSERT_EQUAL(u"1.1"_ustr,
                         getProperty<OUString>(getParagraph(2), u"ListLabelString"_ustr).trim());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf156560)
{
    createSwDoc("tdf156560.docx");

    uno::Reference<beans::XPropertySet> xPageStyle(
        getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xPageStyle, u"HeaderIsOn"_ustr));

    SwXTextDocument* pTextDoc = getSwTextDoc();
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_MOD1 | KEY_PAGEUP);

    // Insert header
    // Without the fix in place, this test would have got SIGABRT here
    dispatchCommand(mxComponent, u".uno:InsertHeader"_ustr, {});
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest8, testTdf158459)
{
    createSwDoc("tdf158459_tracked_changes_across_nodes.fodt");
    SwDoc* pDoc = getSwDoc();

    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
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
    CPPUNIT_ASSERT_EQUAL(u"abdf"_ustr, pTextNode->GetText());
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
