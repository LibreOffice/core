/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/chart/XChartDocument.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <comphelper/propertysequence.hxx>
#include <comphelper/configuration.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <vcl/scheduler.hxx>
#include <vcl/settings.hxx>
#include <ndtxt.hxx>
#include <wrtsh.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <UndoManager.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <view.hxx>
#include <fmtcntnt.hxx>
#include <frameformats.hxx>
#include <shellio.hxx>
#include <editeng/fontitem.hxx>
#include <unotxdoc.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <redline.hxx>

/// Second set of tests asserting the behavior of Writer user interface shells.
class SwUiWriterTest5 : public SwModelTestBase
{
public:
    SwUiWriterTest5()
        : SwModelTestBase(u"/sw/qa/extras/uiwriter/data/"_ustr)
    {
    }

protected:
    AllSettings m_aSavedSettings;
};

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testRedlineTableRowDeletionWithExport)
{
    // load a 1-row table, and delete the row with enabled change tracking:
    // now the row is not deleted silently, but keeps the deleted cell contents,
    // and only accepting all of them will result the deletion of the table row.
    createSwDoc("tdf118311.fodt");
    SwDoc* pDoc = getSwDoc();

    // turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // check table
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");

    // delete table row with enabled change tracking
    // (HasTextChangesOnly property of the row will be false)
    dispatchCommand(mxComponent, u".uno:DeleteRows"_ustr, {});

    // Deleted text content with change tracking,
    // but not table deletion
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");

    // Save it and load it back.
    saveAndReload(u"writer8"_ustr);
    pDoc = getSwDoc();

    // accept the deletion of the content of the first cell
    SwEditShell* const pEditShell(pDoc->GetEditShell());
    CPPUNIT_ASSERT(pEditShell);
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(2), pEditShell->GetRedlineCount());
    pEditShell->AcceptRedline(0);

    // table row was still not deleted
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");

    // accept last redline
    pEditShell->AcceptRedline(0);

    // table row (and the 1-row table) was deleted finally
    // (working export/import of HasTextChangesOnly)
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab", 0);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testRedlineTableRowDeletionWithDOCXExport)
{
    // load a 1-row table, and delete the row with enabled change tracking:
    // now the row is not deleted silently, but keeps the deleted cell contents,
    // and only accepting all of them will result the deletion of the table row.
    createSwDoc("tdf118311.fodt");
    SwDoc* pDoc = getSwDoc();

    // turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // check table
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");

    // delete table row with enabled change tracking
    // (HasTextChangesOnly property of the row will be false)
    dispatchCommand(mxComponent, u".uno:DeleteRows"_ustr, {});

    // Deleted text content with change tracking,
    // but not table deletion
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");

    // Save it to a DOCX and load it back.
    // Exporting change tracking of the row wasn't supported.
    // Also Manage Changes for the import.
    saveAndReload(u"Office Open XML Text"_ustr);
    pDoc = getSwDoc();

    // accept the deletion of the content of the first cell
    SwEditShell* const pEditShell(pDoc->GetEditShell());
    CPPUNIT_ASSERT(pEditShell);
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(2), pEditShell->GetRedlineCount());
    pEditShell->AcceptRedline(0);

    // table row was still not deleted
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");

    // accept last redline
    pEditShell->AcceptRedline(0);

    // table row (and the 1-row table) was deleted finally
    // (working export/import of HasTextChangesOnly)
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab", 0);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testRedlineDOCXTableInsertion)
{
    // load a 3-row table inserted with change tracking by text to table conversion
    createSwDoc("TC-table-converttotable.docx");
    SwDoc* pDoc = getSwDoc();

    // check table count (1)
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());

    // reject the text insertions of the table cells (also reject deletion of the tabulated
    // text source of the table, which was used by the tracked text to table conversion)
    SwEditShell* const pEditShell(pDoc->GetEditShell());
    CPPUNIT_ASSERT(pEditShell);
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(10), pEditShell->GetRedlineCount());
    while (pEditShell->GetRedlineCount())
        pEditShell->RejectRedline(0);

    // rejecting all text insertions must undo the table insertion
    // This was 1 (remaining empty table after rejecting all table text insertions)
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xTables->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testRedlineDOCXTableMoveToFrame)
{
    // load a table with tracked drag & drop: Table1 is the moveFrom,
    // Table2 is the moveTo - and framed - table
    createSwDoc("TC-table-DnD-move.docx");
    SwDoc* pDoc = getSwDoc();

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xTableNames = xTextTablesSupplier->getTextTables();
    // check table count (2)
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTables->getCount());

    // accept tracked table moving, remaining table is Table2
    IDocumentRedlineAccess& rIDRA(pDoc->getIDocumentRedlineAccess());
    rIDRA.AcceptAllRedline(true);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
    CPPUNIT_ASSERT(xTableNames->hasByName(u"Table2"_ustr));
    CPPUNIT_ASSERT(!xTableNames->hasByName(u"Table1"_ustr));

    // Undo and reject tracked table moving, remaining table is Table1
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    rIDRA.AcceptAllRedline(false);
    // This was 2 (not deleted Table2 – framed)
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
    CPPUNIT_ASSERT(xTableNames->hasByName(u"Table1"_ustr));
    CPPUNIT_ASSERT(!xTableNames->hasByName(u"Table2"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf157662_AcceptInsertRedlineCutWithDeletion)
{
    createSwDoc("tdf157662_redlineNestedInsertDelete.odt");
    SwDoc* pDoc = getSwDoc();

    // turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    SwEditShell* const pEditShell(pDoc->GetEditShell());
    CPPUNIT_ASSERT(pEditShell);
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(9), pEditShell->GetRedlineCount());

    // Accept the insert that splitted into 3 parts .. accept all 3 of them
    pEditShell->AcceptRedline(6);
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(7), pEditShell->GetRedlineCount());
    // The middle had a delete too, rejecting the delete will remove that redline too.
    pEditShell->RejectRedline(6);
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(6), pEditShell->GetRedlineCount());

    // Accept insert that splitted into 4 parts, but separated to 2-2 parts, with another insert.
    // It will accept only 2 parts, that is not separated. It leave the deletion.
    pEditShell->AcceptRedline(0);
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(5), pEditShell->GetRedlineCount());
    // Accepting the delete will remove that redline.
    // (only that one, as its other half is separated from it with an insert)
    pEditShell->AcceptRedline(0);
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(4), pEditShell->GetRedlineCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf157662_RejectInsertRedlineCutWithDeletion)
{
    createSwDoc("tdf157662_redlineNestedInsertDelete.odt");
    SwDoc* pDoc = getSwDoc();

    // turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    SwEditShell* const pEditShell(pDoc->GetEditShell());
    CPPUNIT_ASSERT(pEditShell);
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(9), pEditShell->GetRedlineCount());

    // Reject the insert that splitted into 3 parts. reject all 3 of them
    // it even remove the deletion, that was on the 2. insert...
    pEditShell->RejectRedline(6);
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(6), pEditShell->GetRedlineCount());

    // Reject insert that splitted into 4 parts, but separated to 2-2 parts, with another insert.
    // It will reject only 2 parts, that is not separated. It remove the deletion.
    pEditShell->RejectRedline(0);
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(4), pEditShell->GetRedlineCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf157663_RedlineMoveRecognition)
{
    createSwDoc("tdf157663_redlineMove.odt");
    SwDoc* pDoc = getSwDoc();

    // turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    SwEditShell* const pEditShell(pDoc->GetEditShell());
    CPPUNIT_ASSERT(pEditShell);

    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(23), pEditShell->GetRedlineCount());

    // Check if move redlines are recognised as moved, during import
    SwRedlineTable& rTable = pDoc->getIDocumentRedlineAccess().GetRedlineTable();
    bool vMovedRedlines[23]
        = { false, true,  true, true,  true, true,  true,  true,  true,  true,  true, true,
            true,  false, true, false, true, false, false, false, false, false, false };
    // 20. and 22. redline is a delete/insert redline with the same text "three".
    // they are not recognised as a move, because 22. redline is not a whole paragraph.
    // Note: delete/insert redlines that are just a part of a paragraph decided to be part of
    // a move, only if it is at least 6 character long and contain a space "" character.
    for (SwRedlineTable::size_type i = 0; i < rTable.size(); i++)
    {
        CPPUNIT_ASSERT_EQUAL(vMovedRedlines[i], rTable[i]->GetMoved() > 0);
    }

    // Check if accepting move redlines accept its pairs as well.
    pEditShell->AcceptRedline(3); // "9 3/4"
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(19), pEditShell->GetRedlineCount());

    pEditShell->AcceptRedline(1); // "sqrt(10)"
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(17), pEditShell->GetRedlineCount());

    pEditShell->AcceptRedline(1); // "four"
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(13), pEditShell->GetRedlineCount());

    pEditShell->AcceptRedline(3); // "six"
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(11), pEditShell->GetRedlineCount());

    pEditShell->AcceptRedline(4); // "sqrt(17)"
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(9), pEditShell->GetRedlineCount());

    // Undo back all the 5 redline accepts
    for (int i = 0; i < 5; i++)
    {
        dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    }
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(23), pEditShell->GetRedlineCount());

    // Check if rejecting redlines reject its pairs as well.
    pEditShell->RejectRedline(3); // "9 3/4"
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(20), pEditShell->GetRedlineCount());

    pEditShell->RejectRedline(2); // "sqrt(10)"
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(18), pEditShell->GetRedlineCount());

    pEditShell->RejectRedline(2); // "four"
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(15), pEditShell->GetRedlineCount());

    pEditShell->RejectRedline(2); // "sqrt(17)"
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(14), pEditShell->GetRedlineCount());

    pEditShell->RejectRedline(2); // "six"
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(12), pEditShell->GetRedlineCount());

    const sal_uInt32 nZeroID = 0;

    // Check if there are no more move redlines
    for (SwRedlineTable::size_type i = 0; i < rTable.size(); i++)
    {
        CPPUNIT_ASSERT_EQUAL(nZeroID, rTable[i]->GetMoved());
    }

    // Check if moving paragraphs generate redline moves

    // move a paragraph that has delete redlines inside of it
    // original text: "Seve ent teen"
    // deleted texts: "e " and " t"
    // moved new text: "Seventeen"
    pEditShell->GotoRedline(6, true);
    pEditShell->UpdateCursor();
    pEditShell->MoveParagraph(SwNodeOffset(1));
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(16), pEditShell->GetRedlineCount());

    sal_uInt32 nMovedID = rTable[6]->GetMoved();
    //moved text from here
    CPPUNIT_ASSERT(nMovedID > 0); // "Sev"
    CPPUNIT_ASSERT_EQUAL(nZeroID, rTable[7]->GetMoved()); // "e " deleted text not moved
    CPPUNIT_ASSERT_EQUAL(nMovedID, rTable[8]->GetMoved()); // "ent"
    CPPUNIT_ASSERT_EQUAL(nZeroID, rTable[9]->GetMoved()); // " t"
    CPPUNIT_ASSERT_EQUAL(nMovedID, rTable[10]->GetMoved()); // "teen"
    // moved text to here
    CPPUNIT_ASSERT_EQUAL(nMovedID, rTable[11]->GetMoved()); // "Seventeen"

    // move paragraph that has an insert redline inside of it
    // original text: "Eigen"
    // inserted text: "hte"
    // moved new text :"Eighteen"
    pEditShell->GotoRedline(12, true);
    pEditShell->UpdateCursor();
    pEditShell->MoveParagraph(SwNodeOffset(-2));
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(19), pEditShell->GetRedlineCount());

    nMovedID = rTable[12]->GetMoved();
    // moved text to here
    CPPUNIT_ASSERT(nMovedID > 0); // "Eighteen"
    // moved text from here
    CPPUNIT_ASSERT_EQUAL(nMovedID, rTable[13]->GetMoved()); // "Eigen"
    CPPUNIT_ASSERT_EQUAL(nMovedID, rTable[14]->GetMoved()); // "hte"
    CPPUNIT_ASSERT_EQUAL(nMovedID, rTable[15]->GetMoved()); // "en"

    //Check if accept work on both side of the redlines made by manual move paragraphs
    pEditShell->AcceptRedline(13); // "Eigen"
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(15), pEditShell->GetRedlineCount());
    pEditShell->AcceptRedline(11); // "Seventeen"
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(10), pEditShell->GetRedlineCount());

    //undo back the last 2 accept
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(19), pEditShell->GetRedlineCount());

    //Check if reject work on both side of the redlines made by manual move paragraphs
    pEditShell->RejectRedline(13); // "Eigen"
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(16), pEditShell->GetRedlineCount());
    pEditShell->RejectRedline(11); // "Seventeen"
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(12), pEditShell->GetRedlineCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf143215)
{
    // load a table with tracked insertion of an empty row
    createSwDoc("TC-table-rowadd.docx");
    SwDoc* pDoc = getSwDoc();

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    // check table count
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());

    // check table row count
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xTable->getRows()->getCount());

    // reject insertion of the empty table row
    IDocumentRedlineAccess& rIDRA(pDoc->getIDocumentRedlineAccess());
    rIDRA.AcceptAllRedline(false);
    // This was 4 (remained empty row)
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTable->getRows()->getCount());

    // Undo and accept insertion of the table row
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    rIDRA.AcceptAllRedline(true);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xTable->getRows()->getCount());

    // delete it with change tracking, and accept the deletion
    dispatchCommand(mxComponent, u".uno:DeleteRows"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xTable->getRows()->getCount());
    rIDRA.AcceptAllRedline(true);
    // This was 4 (remained empty row)
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTable->getRows()->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf150666)
{
    // load a table with tracked insertion of an empty row
    createSwDoc("TC-table-rowadd.docx");

    // check table count
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());

    // check table row count
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xTable->getRows()->getCount());

    // select the second row (tracked table row insertion)
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Down(/*bSelect=*/false);

    // delete it, and accept all tracked changes
    dispatchCommand(mxComponent, u".uno:DeleteRows"_ustr, {});
    dispatchCommand(mxComponent, u".uno:AcceptAllTrackedChanges"_ustr, {});

    // This was 4 (it was not possible to delete only the tracked row insertions)
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTable->getRows()->getCount());

    // insert a new table row with track changes
    dispatchCommand(mxComponent, u".uno:InsertRowsAfter"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xTable->getRows()->getCount());

    // select and delete it
    pWrtShell->Down(/*bSelect=*/false);
    dispatchCommand(mxComponent, u".uno:DeleteRows"_ustr, {});
    dispatchCommand(mxComponent, u".uno:AcceptAllTrackedChanges"_ustr, {});

    // This was 4 (it was not possible to delete own tracked row insertions)
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTable->getRows()->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf150666_regression)
{
    // load a table with tracked insertion of an empty row
    createSwDoc("TC-table-rowadd.docx");

    // check table count
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());

    // check table row count
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xTable->getRows()->getCount());

    // select the second row (tracked table row insertion)
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Down(/*bSelect=*/false);

    // insert a new table row with track changes
    dispatchCommand(mxComponent, u".uno:InsertRowsAfter"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), xTable->getRows()->getCount());

    dispatchCommand(mxComponent, u".uno:RejectAllTrackedChanges"_ustr, {});

    // This was 4 (the inserted table row wasn't tracked)
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTable->getRows()->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf144748)
{
    // load a table with an empty row, and an empty line before the table
    // (to allow the easy selection of the full text with the table)
    createSwDoc("tdf144748.fodt");
    SwDoc* pDoc = getSwDoc();

    // turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    // there is a table in the text
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());

    // delete full text with the table and check Undo

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Delete"_ustr, {});
    // this crashed LibreOffice
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    // redo and check redline usage

    dispatchCommand(mxComponent, u".uno:Redo"_ustr, {});
    SwEditShell* const pEditShell(pDoc->GetEditShell());
    CPPUNIT_ASSERT(pEditShell);
    // This was 2 (bad extra redline for the empty row of the deleted table)
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(1), pEditShell->GetRedlineCount());

    // accept deletion of the text, including the table with the empty row

    IDocumentRedlineAccess& rIDRA(pDoc->getIDocumentRedlineAccess());
    rIDRA.AcceptAllRedline(true);

    // no table left in the text
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xTables->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf147180)
{
    // load a tracked table insertion (single redline)
    createSwDoc("tdf147180.fodt");
    SwDoc* pDoc = getSwDoc();

    // turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    // there is a table in the text
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());

    // insert a character in the first cell with change tracking
    SwWrtShell* const pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Insert(u"x"_ustr);

    // reject all the changes, including table insertion

    IDocumentRedlineAccess& rIDRA(pDoc->getIDocumentRedlineAccess());
    rIDRA.AcceptAllRedline(/*bAccept=*/false);

    // no table left in the text

    // This was 1 (lost tracking of the table after modifying its text content)
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xTables->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf147180_empty_rows)
{
    // load a tracked table insertion (single redline) with empty rows
    createSwDoc("tdf150824.fodt");
    SwDoc* pDoc = getSwDoc();

    // turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    // there is a table in the text
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());

    // insert a character in the first cell with change tracking
    SwWrtShell* const pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Insert(u"x"_ustr);

    // reject all the changes, including table insertion

    IDocumentRedlineAccess& rIDRA(pDoc->getIDocumentRedlineAccess());
    rIDRA.AcceptAllRedline(/*bAccept=*/false);

    // no table left in the text

    // This was 1 (lost tracking of the empty rows after modifying table text content)
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xTables->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testRedlineTableColumnDeletion)
{
    // load a table, and delete the first column with enabled change tracking:
    // now the column is not deleted silently, but keeps the deleted cell content,
    // and only accepting it will result the deletion of the table column.
    createSwDoc("tdf118311.fodt");
    SwDoc* pDoc = getSwDoc();

    // turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // check table
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");
    assertXPath(pXmlDoc, "//page[1]//body/tab/row/cell", 2);

    // delete table column with enabled change tracking
    // (HasTextChangesOnly property of the cell will be false)
    dispatchCommand(mxComponent, u".uno:DeleteColumns"_ustr, {});

    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");
    // This was 1 (deleted cell without change tracking)
    assertXPath(pXmlDoc, "//page[1]//body/tab/row/cell", 2);

    // accept the deletion
    SwEditShell* const pEditShell(pDoc->GetEditShell());
    CPPUNIT_ASSERT(pEditShell);
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(1), pEditShell->GetRedlineCount());
    pEditShell->AcceptRedline(0);

    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");
    // deleted column
    assertXPath(pXmlDoc, "//page[1]//body/tab/row/cell", 1);

    // Undo, and repeat the previous test, but only with deletion of the text content of the cells
    // (HasTextChangesOnly property will be removed by Undo)

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    // first column exists again
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");
    assertXPath(pXmlDoc, "//page[1]//body/tab/row/cell", 2);

    // delete table column with enabled change tracking
    dispatchCommand(mxComponent, u".uno:SelectColumn"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Delete"_ustr, {});

    // Table column still exists
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");
    assertXPath(pXmlDoc, "//page[1]//body/tab/row/cell", 2);

    // accept the deletion of the content of the first cell
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(1), pEditShell->GetRedlineCount());
    pEditShell->AcceptRedline(0);

    // table column was still not deleted
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");
    assertXPath(pXmlDoc, "//page[1]//body/tab/row/cell", 2);

    // Undo, and delete the column without change tracking

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    // table exists again
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");
    assertXPath(pXmlDoc, "//page[1]//body/tab/row/cell", 2);

    // disable change tracking
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);

    CPPUNIT_ASSERT_MESSAGE("redlining should be off",
                           !pDoc->getIDocumentRedlineAccess().IsRedlineOn());

    // delete table column without change tracking
    dispatchCommand(mxComponent, u".uno:DeleteColumns"_ustr, {});

    // the table column was deleted
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");
    assertXPath(pXmlDoc, "//page[1]//body/tab/row/cell", 1);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf156474)
{
    // load a table, and insert a column with change tracking
    createSwDoc("tdf118311.fodt");
    SwDoc* pDoc = getSwDoc();

    // turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);

    // there is a table in the text with two columns
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTextTable->getColumns()->getCount());

    // insert table column with enabled change tracking
    // (HasTextChangesOnly property of the cell will be false)
    dispatchCommand(mxComponent, u".uno:InsertColumnsBefore"_ustr, {});

    // 3 columns
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTextTable->getColumns()->getCount());

    // accept tracked changes: remove HasTextChangesOnly = false of the inserted cells
    dispatchCommand(mxComponent, u".uno:AcceptAllTrackedChanges"_ustr, {});

    // still 3 columns
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTextTable->getColumns()->getCount());

    // delete the text content (dummy character of the previous text change) of the newly
    // inserted cell, and accept tracked changes
    SwWrtShell* const pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    dispatchCommand(mxComponent, u".uno:SwBackspace"_ustr, {});
    dispatchCommand(mxComponent, u".uno:AcceptAllTrackedChanges"_ustr, {});

    // This was 2 columns (not removed HasTextChangesOnly = false resulted column deletion
    // instead of deleting only content of the cell)
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTextTable->getColumns()->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, tdf156475)
{
    // load a table, and insert a row without change tracking,
    // and delete the first column with the empty cell in the second row with change tracking
    createSwDoc("tdf118311.fodt");
    SwDoc* pDoc = getSwDoc();

    // turn off red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be off",
                           !pDoc->getIDocumentRedlineAccess().IsRedlineOn());

    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // insert table row
    dispatchCommand(mxComponent, u".uno:InsertRowsAfter"_ustr, {});

    // check table
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");
    assertXPath(pXmlDoc, "//page[1]//body/tab/row", 2);
    assertXPath(pXmlDoc, "//page[1]//body/tab/row[1]/cell", 2);
    assertXPath(pXmlDoc, "//page[1]//body/tab/row[2]/cell", 2);

    // turn on red-lining
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());

    // delete table column with enabled change tracking
    // (HasTextChangesOnly property of the cell will be false)
    dispatchCommand(mxComponent, u".uno:DeleteColumns"_ustr, {});

    // go down to the empty cell
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Down(/*bSelect=*/false);

    // Without the fix in place, this couldn't work
    dispatchCommand(mxComponent, u".uno:AcceptTrackedChange"_ustr, {});

    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");
    assertXPath(pXmlDoc, "//page[1]//body/tab/row", 2);
    assertXPath(pXmlDoc, "//page[1]//body/tab/row[1]/cell", 1);
    assertXPath(pXmlDoc, "//page[1]//body/tab/row[2]/cell", 1);

    // test Undo/Redo
    for (sal_Int32 i = 0; i < 4; ++i)
    {
        dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    }

    for (sal_Int32 i = 0; i < 4; ++i)
    {
        dispatchCommand(mxComponent, u".uno:Redo"_ustr, {});
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf155747)
{
    // load a table, and delete the first column with enabled change tracking:
    // now the column is not deleted silently, but keeps the deleted cell content,
    // and only accepting it will result the deletion of the table column.
    createSwDoc("tdf118311.fodt");
    SwDoc* pDoc = getSwDoc();

    // turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // delete table column with enabled change tracking
    // (HasTextChangesOnly property of the cell will be false)
    dispatchCommand(mxComponent, u".uno:DeleteColumns"_ustr, {});

    // select table
    dispatchCommand(mxComponent, u".uno:SelectTable"_ustr, {});

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, u".uno:AcceptTrackedChange"_ustr, {});

    // check removed column
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");
    assertXPath(pXmlDoc, "//page[1]//body/tab/row/cell", 1);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf156544)
{
    // load a table, and insert a column without change tracking,
    // and delete the first column with the empty cell in the second row with change tracking
    createSwDoc("tdf118311.fodt");
    SwDoc* pDoc = getSwDoc();

    // turn off red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be off",
                           !pDoc->getIDocumentRedlineAccess().IsRedlineOn());

    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // insert table column without change tracking
    // (HasTextChangesOnly property of the cell will be false)
    dispatchCommand(mxComponent, u".uno:InsertColumnsBefore"_ustr, {});

    // check table
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");
    assertXPath(pXmlDoc, "//page[1]//body/tab/row", 1);
    assertXPath(pXmlDoc, "//page[1]//body/tab/row[1]/cell", 3);

    // turn on red-lining
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());

    // go to the empty column
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);

    // delete table column with enabled change tracking
    // (HasTextChangesOnly property of the cell will be false)
    dispatchCommand(mxComponent, u".uno:DeleteColumns"_ustr, {});

    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");
    assertXPath(pXmlDoc, "//page[1]//body/tab/row", 1);

    // This was 2 (deleted column)
    assertXPath(pXmlDoc, "//page[1]//body/tab/row/cell", 3);

    // accept the deletion of the empty column
    dispatchCommand(mxComponent, u".uno:AcceptTrackedChange"_ustr, {});

    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");
    assertXPath(pXmlDoc, "//page[1]//body/tab/row", 1);
    assertXPath(pXmlDoc, "//page[1]//body/tab/row/cell", 2);

    // test Undo/Redo
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");
    assertXPath(pXmlDoc, "//page[1]//body/tab/row", 1);
    assertXPath(pXmlDoc, "//page[1]//body/tab/row/cell", 3);

    dispatchCommand(mxComponent, u".uno:Redo"_ustr, {});

    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");
    assertXPath(pXmlDoc, "//page[1]//body/tab/row", 1);
    assertXPath(pXmlDoc, "//page[1]//body/tab/row/cell", 2);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf156487)
{
    // load a table, and delete a column in Hide Changes mode
    createSwDoc("tdf118311.fodt");
    SwDoc* pDoc = getSwDoc();

    // turn on red-lining and hide changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());

    CPPUNIT_ASSERT_MESSAGE("redlines shouldn't be visible",
                           !IDocumentRedlineAccess::IsShowChanges(
                               pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // delete table column with enabled change tracking
    // (HasTextChangesOnly property of the cell will be false)
    dispatchCommand(mxComponent, u".uno:DeleteColumns"_ustr, {});

    // Dump the rendering of the first page as an XML file.
    SwDocShell* pShell = getSwDocShell();
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // This would be 2 without hiding the first cell
    assertXPath(pXmlDoc, "/metafile/push/push/push/textarray/text", 1);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf149498)
{
    // load a table, and delete the first column with enabled change tracking:
    // now the column is not deleted silently, but keeps the deleted cell content,
    // and only accepting it will result the deletion of the table column.
    createSwDoc("tdf149498.docx");

    // select table, copy, paste and Undo
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});

    // this would crash due to bookmark over cell boundary
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf150673_RedlineTableColumnDeletionWithExport)
{
    // load a table, and delete the first column with enabled change tracking:
    // now the column is not deleted silently, but keeps the deleted cell contents,
    // and only accepting all of them will result the deletion of the table column.
    createSwDoc("tdf118311.fodt");
    SwDoc* pDoc = getSwDoc();

    // turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // check table
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");

    // delete table column with enabled change tracking
    // (HasTextChangesOnly property of the cell will be false)
    dispatchCommand(mxComponent, u".uno:DeleteColumns"_ustr, {});

    // Deleted text content with change tracking,
    // but not table deletion
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");
    assertXPath(pXmlDoc, "//page[1]//body/tab/row/cell", 2);

    // Save it and load it back.
    saveAndReload(u"writer8"_ustr);
    pDoc = getSwDoc();

    // accept the deletion of the content of the first cell
    SwEditShell* const pEditShell(pDoc->GetEditShell());
    CPPUNIT_ASSERT(pEditShell);
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(1), pEditShell->GetRedlineCount());
    pEditShell->AcceptRedline(0);

    // first table column was deleted finally
    // (working export/import of HasTextChangesOnly)
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");
    assertXPath(pXmlDoc, "//page[1]//body/tab/row/cell", 1);

    // check removing HasTextChangesOnly at acceptance of the deletion

    // Undo, and delete the column without change tracking
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    // table column exists again
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");
    assertXPath(pXmlDoc, "//page[1]//body/tab/row/cell", 2);

    // reject deletion, setting HasTextChangesOnly to TRUE
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(1), pEditShell->GetRedlineCount());
    pEditShell->RejectRedline(0);
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(0), pEditShell->GetRedlineCount());

    // delete table column with enabled change tracking
    dispatchCommand(mxComponent, u".uno:SelectColumn"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Delete"_ustr, {});

    // Table column still exists
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");
    assertXPath(pXmlDoc, "//page[1]//body/tab/row/cell", 2);

    // reject the deletion of the content of the first cell
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(1), pEditShell->GetRedlineCount());
    pEditShell->AcceptRedline(0);

    // table column is still not deleted
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");
    assertXPath(pXmlDoc, "//page[1]//body/tab/row/cell", 2);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testRedlineTableColumnDeletionWithDOCXExport)
{
    // load a 1-row table, and delete the first column with enabled change tracking:
    createSwDoc("tdf118311.fodt");
    SwDoc* pDoc = getSwDoc();

    // turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // check table
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");
    assertXPath(pXmlDoc, "//page[1]//body/tab/row/cell", 2);

    // delete first table column with enabled change tracking
    // (HasTextChangesOnly property of the cell will be false)
    dispatchCommand(mxComponent, u".uno:DeleteColumns"_ustr, {});

    // Deleted text content with change tracking,
    // but not table deletion
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");
    assertXPath(pXmlDoc, "//page[1]//body/tab/row/cell", 2);

    // Save it to a DOCX and load it back.
    // Exporting change tracking of the cell wasn't supported.
    // Also Manage Changes for the import.
    saveAndReload(u"Office Open XML Text"_ustr);
    pDoc = getSwDoc();

    // accept the deletion of the content of the first cell
    SwEditShell* const pEditShell(pDoc->GetEditShell());
    CPPUNIT_ASSERT(pEditShell);
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(1), pEditShell->GetRedlineCount());
    pEditShell->AcceptRedline(0);

    // table column was deleted
    // (working export/import of HasTextChangesOnly of table cells)
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");
    assertXPath(pXmlDoc, "//page[1]//body/tab/row/cell", 1);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf155341_RedlineTableColumnInsertionWithExport)
{
    // load a table, and insert a new column with enabled change tracking
    createSwDoc("tdf118311.fodt");
    SwDoc* pDoc = getSwDoc();

    // turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // check table
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");

    // insert table column with enabled change tracking
    // (HasTextChangesOnly property of the cell will be false)
    dispatchCommand(mxComponent, u".uno:InsertColumnsAfter"_ustr, {});

    // text content with change tracking (dummy redline)
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");
    assertXPath(pXmlDoc, "//page[1]//body/tab/row/cell", 3);

    // Save it and load it back.
    saveAndReload(u"writer8"_ustr);
    pDoc = getSwDoc();

    // reject the insertion of the hidden content of the cell
    SwEditShell* const pEditShell(pDoc->GetEditShell());
    CPPUNIT_ASSERT(pEditShell);
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(1), pEditShell->GetRedlineCount());
    pEditShell->RejectRedline(0);

    // inserted table column was deleted
    // (working export/import of HasTextChangesOnly)
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");
    assertXPath(pXmlDoc, "//page[1]//body/tab/row/cell", 2);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf128335)
{
    // Load the bugdoc, which has 3 textboxes.
    createSwDoc("tdf128335.odt");

    // Select the 3rd textbox.
    SwView* pView = getSwDocShell()->GetView();
    selectShape(1);
    SwXTextDocument* pTextDoc = getSwTextDoc();
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_TAB);
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_TAB);
    Scheduler::ProcessEventsToIdle();

    // Cut it.
    pView->GetViewFrame().GetDispatcher()->Execute(SID_CUT, SfxCallMode::SYNCHRON);

    // Paste it: this makes the 3rd textbox anchored in the 2nd one.
    pView->GetViewFrame().GetDispatcher()->Execute(SID_PASTE, SfxCallMode::SYNCHRON);

    selectShape(1);

    // Cut them.
    // Without the accompanying fix in place, this test would have crashed as the textboxes were
    // deleted in an incorrect order.
    pView->GetViewFrame().GetDispatcher()->Execute(SID_CUT, SfxCallMode::SYNCHRON);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testRedlineTableRowDeletionWithReject)
{
    // load a 1-row table, and delete the row with enabled change tracking:
    // now the row is not deleted silently, but keeps the deleted cell contents,
    // and only accepting all of them will result the deletion of the table row.
    createSwDoc("tdf118311.fodt");
    SwDoc* pDoc = getSwDoc();

    // turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // check table
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");

    // delete table row with enabled change tracking
    // (HasTextChangesOnly property of the row will be false)
    dispatchCommand(mxComponent, u".uno:DeleteRows"_ustr, {});

    // Deleted text content with change tracking,
    // but not table deletion
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");

    // Save it and load it back.
    saveAndReload(u"writer8"_ustr);
    pDoc = getSwDoc();

    // reject the deletion of the content of the first cell
    // HasTextChangesOnly property of the table row will be true
    SwEditShell* const pEditShell(pDoc->GetEditShell());
    CPPUNIT_ASSERT(pEditShell);
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(2), pEditShell->GetRedlineCount());
    pEditShell->RejectRedline(0);

    // Select and delete the content of the first cell
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Delete"_ustr, {});

    // table row was still not deleted
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");

    // accept all redlines
    while (pEditShell->GetRedlineCount())
        pEditShell->AcceptRedline(0);

    // This was table row deletion instead of remaining the empty row
    // (HasTextChangesOnly was false)
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");

    // restore HasTextChangesOnly = false
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    // accept all redlines
    while (pEditShell->GetRedlineCount())
        pEditShell->AcceptRedline(0);

    // table row (and the 1-row table) was deleted finally
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab", 0);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testRedlineTableRowInsertionWithReject)
{
    // load a 1-row table, and insert a row with enabled change tracking
    createSwDoc("tdf118311.fodt");
    SwDoc* pDoc = getSwDoc();

    // turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // check table and its single row
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");
    assertXPath(pXmlDoc, "//page[1]//body/tab/row", 1);

    // insert rows before and after with enabled change tracking
    // (HasTextChangesOnly property of the row will be false, and
    // add dummy characters CH_TXT_TRACKED_DUMMY_CHAR)
    dispatchCommand(mxComponent, u".uno:InsertRowsBefore"_ustr, {});
    dispatchCommand(mxComponent, u".uno:InsertRowsAfter"_ustr, {});

    SwEditShell* const pEditShell(pDoc->GetEditShell());
    CPPUNIT_ASSERT(pEditShell);
    // This was 0 (not tracked row insertion)
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(2), pEditShell->GetRedlineCount());

    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");
    assertXPath(pXmlDoc, "//page[1]//body/tab/row", 3);

    // reject redlines
    pEditShell->RejectRedline(0);
    pEditShell->RejectRedline(0);

    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");
    // This was 3 (not rejected row insertion)
    assertXPath(pXmlDoc, "//page[1]//body/tab/row", 1);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf145089_RedlineTableRowInsertionDOCX)
{
    // load a 1-row table, and insert a row with enabled change tracking
    createSwDoc("tdf118311.fodt");
    SwDoc* pDoc = getSwDoc();

    // turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // check table and its single row
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");
    assertXPath(pXmlDoc, "//page[1]//body/tab/row", 1);

    // insert rows before and after with enabled change tracking
    // (HasTextChangesOnly property of the row will be false, and
    // add dummy characters CH_TXT_TRACKED_DUMMY_CHAR)
    dispatchCommand(mxComponent, u".uno:InsertRowsBefore"_ustr, {});
    dispatchCommand(mxComponent, u".uno:InsertRowsAfter"_ustr, {});

    // save it to DOCX
    saveAndReload(u"Office Open XML Text"_ustr);
    SwViewShell* pViewShell = getSwDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    pViewShell->Reformat();
    pXmlDoc = parseLayoutDump();

    assertXPath(pXmlDoc, "//page[1]//body/tab");
    assertXPath(pXmlDoc, "//page[1]//body/tab/row", 3);

    // reject redlines
    SwDoc* pDOCXDoc(getSwDoc());
    SwEditShell* const pEditShell(pDOCXDoc->GetEditShell());
    CPPUNIT_ASSERT(pEditShell);
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(2), pEditShell->GetRedlineCount());
    pEditShell->RejectRedline(0);
    pEditShell->RejectRedline(0);

    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");

    // This was 3 (not rejected row insertion)
    assertXPath(pXmlDoc, "//page[1]//body/tab/row", 1);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testPasteTrackedTableRow)
{
    // load a 1-row table
    createSwDoc("tdf118311.fodt");
    SwDoc* pDoc = getSwDoc();

    // turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // check table count
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());

    // check table row count
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getRows()->getCount());

    // copy table row and paste it by Paste Special->Rows Above
    dispatchCommand(mxComponent, u".uno:SelectTable"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Escape"_ustr, {});
    dispatchCommand(mxComponent, u".uno:PasteRowsBefore"_ustr, {});

    // 2-row table
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getRows()->getCount());

    // This was 2 (inserted as a nested table in the first cell of the new row)
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());

    // Is it a tracked row insertion? Its rejection results the original 1-row table
    dispatchCommand(mxComponent, u".uno:RejectAllTrackedChanges"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getRows()->getCount());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getRows()->getCount());

    dispatchCommand(mxComponent, u".uno:Redo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getRows()->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testPasteTrackedTableRowInHideChangesMode)
{
    // load a 1-row table
    createSwDoc("tdf118311.fodt");
    SwDoc* pDoc = getSwDoc();

    // turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE("redlines should be invisible",
                           !IDocumentRedlineAccess::IsShowChanges(
                               pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // check table count
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());

    // check table row count
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getRows()->getCount());

    // copy table row and paste it by Paste Special->Rows Above
    dispatchCommand(mxComponent, u".uno:SelectTable"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Escape"_ustr, {});

    // This resulted freezing
    dispatchCommand(mxComponent, u".uno:PasteRowsBefore"_ustr, {});

    // 2-row table
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getRows()->getCount());

    // This was 2 (inserted as a nested table in the first cell of the new row)
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {}); // FIXME Why 3 Undos?
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getRows()->getCount());

    dispatchCommand(mxComponent, u".uno:Redo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getRows()->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf146966)
{
    // load a 4-row table, select more than 1 row and copy them
    // to check insertion of unnecessary empty rows
    createSwDoc("tdf144748.fodt");

    // check table row count
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xTable->getRows()->getCount());

    // copy table row and paste it by Paste Special->Rows Above
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Down(/*bSelect=*/false);
    dispatchCommand(mxComponent, u".uno:SelectTable"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Escape"_ustr, {});
    dispatchCommand(mxComponent, u".uno:PasteRowsBefore"_ustr, {});

    // This was 35 (extra empty rows)
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8), xTable->getRows()->getCount());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {}); // FIXME Why 3 Undos?
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xTable->getRows()->getCount());

    dispatchCommand(mxComponent, u".uno:Redo"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Redo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8), xTable->getRows()->getCount());
    // dispatchCommand(mxComponent, ".uno:Redo", {}); // FIXME assert
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf145091)
{
    // load a deleted table, reject them, and delete only its text and export to DOCX
    createSwDoc("tdf145091.docx");
    SwDoc* pDoc = getSwDoc();

    // turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // reject all redlines
    SwEditShell* const pEditShell(pDoc->GetEditShell());
    CPPUNIT_ASSERT(pEditShell);
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(3), pEditShell->GetRedlineCount());
    while (pEditShell->GetRedlineCount() > 0)
        pEditShell->RejectRedline(0);
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(0), pEditShell->GetRedlineCount());

    // delete only table text, but not table
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Delete"_ustr, {});
    CPPUNIT_ASSERT(pEditShell->GetRedlineCount() > 0);

    // save it to DOCX
    saveAndReload(u"Office Open XML Text"_ustr);
    SwViewShell* pViewShell = getSwDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    pViewShell->Reformat();
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    assertXPath(pXmlDoc, "//page[1]//body/tab");
    assertXPath(pXmlDoc, "//page[1]//body/tab/row", 3);

    // accept all redlines
    dispatchCommand(mxComponent, u".uno:AcceptAllTrackedChanges"_ustr, {});

    pXmlDoc = parseLayoutDump();
    // This was false (deleted table with accepting deletions)
    assertXPath(pXmlDoc, "//page[1]//body/tab");
    assertXPath(pXmlDoc, "//page[1]//body/tab/row", 3);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf128603)
{
    // Load the bugdoc, which has 3 textboxes.
    createSwDoc("tdf128603.odt");
    SwDoc* pDoc = getSwDoc();

    // Select the 3rd textbox.
    SwView* pView = getSwDocShell()->GetView();
    selectShape(1);
    SwXTextDocument* pTextDoc = getSwTextDoc();
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_TAB);
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_TAB);
    Scheduler::ProcessEventsToIdle();

    // Cut it.
    pView->GetViewFrame().GetDispatcher()->Execute(SID_CUT, SfxCallMode::SYNCHRON);

    // Paste it: this makes the 3rd textbox anchored in the 2nd one.
    pView->GetViewFrame().GetDispatcher()->Execute(SID_PASTE, SfxCallMode::SYNCHRON);

    // Undo all of this.
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    rUndoManager.Undo();
    rUndoManager.Undo();

    // Make sure the content indexes still match.
    const auto& rSpzFrameFormats = *pDoc->GetSpzFrameFormats();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(6), rSpzFrameFormats.size());
    const SwNodeIndex* pIndex4 = rSpzFrameFormats[4]->GetContent().GetContentIdx();
    CPPUNIT_ASSERT(pIndex4);
    const SwNodeIndex* pIndex5 = rSpzFrameFormats[5]->GetContent().GetContentIdx();
    CPPUNIT_ASSERT(pIndex5);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 11
    // - Actual  : 14
    // i.e. the shape content index and the frame content index did not match after undo, even if
    // their "other text box format" pointers pointed to each other.
    CPPUNIT_ASSERT_EQUAL(pIndex4->GetIndex(), pIndex5->GetIndex());
}

// only care that it doesn't assert/crash
CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testOfz18563)
{
    OUString sURL(m_directories.getURLFromSrc(u"/sw/qa/extras/uiwriter/data/ofz18563.docx"));
    SvFileStream aFileStream(sURL, StreamMode::READ);
    TestImportDOCX(aFileStream);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf143904)
{
    createSwDoc("tdf143904.odt");
    SwDoc* pDoc = getSwDoc();

    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    SwNodeOffset nIndex = pWrtShell->GetCursor()->GetPointNode().GetIndex();

    dispatchCommand(mxComponent, u".uno:InsertRowsAfter"_ustr, {});
    pWrtShell->Down(false);
    pWrtShell->Insert(u"foo"_ustr);

    SwTextNode* pTextNodeA1 = static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex]);
    CPPUNIT_ASSERT(pTextNodeA1->GetText().startsWith("Insert"));
    nIndex = pWrtShell->GetCursor()->GetPointNode().GetIndex();
    SwTextNode* pTextNodeA2 = static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex]);
    CPPUNIT_ASSERT_EQUAL(u"foo"_ustr, pTextNodeA2->GetText());
    CPPUNIT_ASSERT_EQUAL(false, pTextNodeA2->GetSwAttrSet().HasItem(RES_CHRATR_FONT));
    OUString sFontName = pTextNodeA2->GetSwAttrSet().GetItem(RES_CHRATR_FONT)->GetFamilyName();
    CPPUNIT_ASSERT_EQUAL(u"Liberation Serif"_ustr, sFontName);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf90069)
{
    createSwDoc("tdf90069.docx");
    SwDoc* pDoc = getSwDoc();

    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    SwNodeOffset nIndex = pWrtShell->GetCursor()->GetPointNode().GetIndex();

    dispatchCommand(mxComponent, u".uno:InsertRowsAfter"_ustr, {});
    pWrtShell->Down(false);
    pWrtShell->Insert(u"foo"_ustr);

    SwTextNode* pTextNodeA1 = static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex]);
    CPPUNIT_ASSERT(pTextNodeA1->GetText().startsWith("Insert"));
    nIndex = pWrtShell->GetCursor()->GetPointNode().GetIndex();
    SwTextNode* pTextNodeA2 = static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex]);
    CPPUNIT_ASSERT_EQUAL(u"foo"_ustr, pTextNodeA2->GetText());
    CPPUNIT_ASSERT_EQUAL(true, pTextNodeA2->GetSwAttrSet().HasItem(RES_CHRATR_FONT));
    OUString sFontName = pTextNodeA2->GetSwAttrSet().GetItem(RES_CHRATR_FONT)->GetFamilyName();
    CPPUNIT_ASSERT_EQUAL(u"Lohit Devanagari"_ustr, sFontName);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf109266)
{
    // transliteration with redlining
    createSwDoc("lorem.fodt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    SwNodeOffset nIndex = pWrtShell->GetCursor()->GetPointNode().GetIndex();
    SwTextNode* pTextNode = static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex]);

    CPPUNIT_ASSERT_EQUAL(u"Lorem ipsum..."_ustr, pTextNode->GetText());
    CPPUNIT_ASSERT_EQUAL(u"Lorem ipsum..."_ustr, pTextNode->GetRedlineText());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:ChangeCaseToTitleCase"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(u"Lorem Ipsum..."_ustr, pTextNode->GetText());
    CPPUNIT_ASSERT_EQUAL(u"Lorem Ipsum..."_ustr, pTextNode->GetRedlineText());

    //turn on red-lining and show changes
    RedlineFlags const mode(pWrtShell->GetRedlineFlags() | RedlineFlags::On);
    CPPUNIT_ASSERT(mode & (RedlineFlags::ShowDelete | RedlineFlags::ShowInsert));
    pWrtShell->SetRedlineFlags(mode);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:ChangeCaseToTitleCase"_ustr, {});

    // This was "Lorem Ipsum..." (missing redlining)
    CPPUNIT_ASSERT_EQUAL(u"Lorem ipsumIpsum..."_ustr, pTextNode->GetText());
    CPPUNIT_ASSERT_EQUAL(u"Lorem Ipsum..."_ustr, pTextNode->GetRedlineText());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:ChangeCaseToUpper"_ustr, {});

    // This was "LOREM IPSUM..." (missing redlining)
    CPPUNIT_ASSERT_EQUAL(u"Lorem ipsum...LOREM IPSUM..."_ustr, pTextNode->GetText());
    CPPUNIT_ASSERT_EQUAL(u"LOREM IPSUM..."_ustr, pTextNode->GetRedlineText());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:ChangeCaseToLower"_ustr, {});

    // This was "lorem ipsum..." (missing redlining)
    CPPUNIT_ASSERT_EQUAL(u"Lorem ipsum...lorem ipsum..."_ustr, pTextNode->GetText());
    CPPUNIT_ASSERT_EQUAL(u"lorem ipsum..."_ustr, pTextNode->GetRedlineText());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:ChangeCaseToToggleCase"_ustr, {});

    // This was "lOREM IPSUM..." (missing redlining)
    CPPUNIT_ASSERT_EQUAL(u"Lorem ipsum...lOREM IPSUM..."_ustr, pTextNode->GetText());
    CPPUNIT_ASSERT_EQUAL(u"lOREM IPSUM..."_ustr, pTextNode->GetRedlineText());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf129655)
{
    createSwDoc("tdf129655-vtextbox.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//anchored/fly/txt[@WritingMode='Vertical']", 1);
}

static uno::Reference<text::XTextRange> getAssociatedTextRange(uno::Any object)
{
    // possible cases:
    // 1. a container of other objects - e.g. selection of 0 to n text portions, or 1 to n drawing objects
    try
    {
        uno::Reference<container::XIndexAccess> xIndexAccess(object, uno::UNO_QUERY_THROW);
        if (xIndexAccess.is() && xIndexAccess->getCount() > 0)
        {
            for (int i = 0; i < xIndexAccess->getCount(); ++i)
            {
                uno::Reference<text::XTextRange> xRange
                    = getAssociatedTextRange(xIndexAccess->getByIndex(i));
                if (xRange.is())
                    return xRange;
            }
        }
    }
    catch (const uno::Exception&)
    {
    }

    // 2. another TextContent, having an anchor we can use
    try
    {
        uno::Reference<text::XTextContent> xTextContent(object, uno::UNO_QUERY_THROW);
        if (xTextContent.is())
        {
            uno::Reference<text::XTextRange> xRange = xTextContent->getAnchor();
            if (xRange.is())
                return xRange;
        }
    }
    catch (const uno::Exception&)
    {
    }

    // an object which supports XTextRange directly
    try
    {
        uno::Reference<text::XTextRange> xRange(object, uno::UNO_QUERY_THROW);
        if (xRange.is())
            return xRange;
    }
    catch (const uno::Exception&)
    {
    }

    return nullptr;
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf123218)
{
    struct ReverseXAxisOrientationDoughnutChart
        : public comphelper::ConfigurationProperty<ReverseXAxisOrientationDoughnutChart, bool>
    {
        static OUString path()
        {
            return u"/org.openoffice.Office.Compatibility/View/ReverseXAxisOrientationDoughnutChart"_ustr;
        }
        ~ReverseXAxisOrientationDoughnutChart() = delete;
    };

    struct ClockwisePieChartDirection
        : public comphelper::ConfigurationProperty<ClockwisePieChartDirection, bool>
    {
        static OUString path()
        {
            return u"/org.openoffice.Office.Compatibility/View/ClockwisePieChartDirection"_ustr;
        }
        ~ClockwisePieChartDirection() = delete;
    };
    auto batch = comphelper::ConfigurationChanges::create();

    ReverseXAxisOrientationDoughnutChart::set(false, batch);
    ClockwisePieChartDirection::set(true, batch);
    batch->commit();

    createSwDoc();

    // create an OLE shape in the document
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xMSF);
    uno::Reference<beans::XPropertySet> xShapeProps(
        xMSF->createInstance(u"com.sun.star.text.TextEmbeddedObject"_ustr), uno::UNO_QUERY);
    xShapeProps->setPropertyValue(u"CLSID"_ustr,
                                  uno::Any(u"12dcae26-281f-416f-a234-c3086127382e"_ustr));
    uno::Reference<drawing::XShape> xShape(xShapeProps, uno::UNO_QUERY_THROW);
    xShape->setSize(awt::Size(16000, 9000));
    uno::Reference<text::XTextContent> chartTextContent(xShapeProps, uno::UNO_QUERY_THROW);
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<view::XSelectionSupplier> xSelSupplier(xModel->getCurrentController(),
                                                          uno::UNO_QUERY_THROW);
    uno::Any aSelection = xSelSupplier->getSelection();
    uno::Reference<text::XTextRange> xTextRange = getAssociatedTextRange(aSelection);
    CPPUNIT_ASSERT(xTextRange);
    xTextRange->getText()->insertTextContent(xTextRange, chartTextContent, false);

    // insert a doughnut chart
    uno::Reference<frame::XModel> xDocModel;
    xShapeProps->getPropertyValue(u"Model"_ustr) >>= xDocModel;
    CPPUNIT_ASSERT(xDocModel);
    uno::Reference<chart::XChartDocument> xChartDoc(xDocModel, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xChartDoc);
    uno::Reference<lang::XMultiServiceFactory> xChartMSF(xChartDoc, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xChartMSF);
    uno::Reference<chart::XDiagram> xDiagram(
        xChartMSF->createInstance(u"com.sun.star.chart.DonutDiagram"_ustr), uno::UNO_QUERY);
    xChartDoc->setDiagram(xDiagram);

    // test primary X axis Orientation value
    uno::Reference<chart2::XChartDocument> xChartDoc2(xChartDoc, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xChartDoc2);
    uno::Reference<chart2::XCoordinateSystemContainer> xCooSysContainer(
        xChartDoc2->getFirstDiagram(), uno::UNO_QUERY_THROW);
    uno::Sequence<uno::Reference<chart2::XCoordinateSystem>> xCooSysSequence
        = xCooSysContainer->getCoordinateSystems();
    uno::Reference<chart2::XCoordinateSystem> xCoord = xCooSysSequence[0];
    CPPUNIT_ASSERT(xCoord.is());
    uno::Reference<chart2::XAxis> xAxis = xCoord->getAxisByDimension(0, 0);
    CPPUNIT_ASSERT(xAxis.is());
    chart2::ScaleData aScaleData = xAxis->getScaleData();
    CPPUNIT_ASSERT_EQUAL(chart2::AxisOrientation_MATHEMATICAL, aScaleData.Orientation);

    // tdf#108059 test primary Y axis Orientation value
    uno::Reference<chart2::XAxis> xYAxis = xCoord->getAxisByDimension(1, 0);
    CPPUNIT_ASSERT(xYAxis.is());
    aScaleData = xYAxis->getScaleData();
    CPPUNIT_ASSERT_EQUAL(chart2::AxisOrientation_REVERSE, aScaleData.Orientation);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf93747)
{
    createSwDoc();
    SwWrtShell* pWrtSh = getSwDocShell()->GetWrtShell();

    uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence(
        { { "Rows", uno::Any(sal_Int32(2)) }, { "Columns", uno::Any(sal_Int32(2)) } }));

    dispatchCommand(mxComponent, u".uno:InsertTable"_ustr, aArgs);

    pWrtSh->Insert(u"Col1"_ustr);

    // Move the cursor to B1
    pWrtSh->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);

    pWrtSh->Insert(u"Col2"_ustr);

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTextTable->getColumns()->getCount());

    uno::Reference<text::XTextRange> xCellA1(xTextTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Col1"_ustr, xCellA1->getString());

    uno::Reference<text::XTextRange> xCellB1(xTextTable->getCellByName(u"B1"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Col2"_ustr, xCellB1->getString());

    // Select backwards B1 and A1
    pWrtSh->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 5, /*bBasicCall=*/false);

    // Just select the whole B1
    pWrtSh->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);

    uno::Sequence<beans::PropertyValue> aPropertyValues = comphelper::InitPropertySequence({
        { "Style", uno::Any(u"Heading 1"_ustr) },
        { "FamilyName", uno::Any(u"ParagraphStyles"_ustr) },
    });
    dispatchCommand(mxComponent, u".uno:StyleApply"_ustr, aPropertyValues);

    // Without the fix in place, this test would have failed with
    // - Expected: Table Contents
    // - Actual  : Heading 1
    CPPUNIT_ASSERT_EQUAL(
        u"Table Contents"_ustr,
        getProperty<OUString>(getParagraphOfText(1, xCellA1->getText()), u"ParaStyleName"_ustr));

    CPPUNIT_ASSERT_EQUAL(
        u"Heading 1"_ustr,
        getProperty<OUString>(getParagraphOfText(1, xCellB1->getText()), u"ParaStyleName"_ustr));

    // Now select A1 again
    pWrtSh->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);

    dispatchCommand(mxComponent, u".uno:StyleApply"_ustr, aPropertyValues);

    CPPUNIT_ASSERT_EQUAL(
        u"Heading 1"_ustr,
        getProperty<OUString>(getParagraphOfText(1, xCellA1->getText()), u"ParaStyleName"_ustr));

    CPPUNIT_ASSERT_EQUAL(
        u"Heading 1"_ustr,
        getProperty<OUString>(getParagraphOfText(1, xCellB1->getText()), u"ParaStyleName"_ustr));

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(
        u"Table Contents"_ustr,
        getProperty<OUString>(getParagraphOfText(1, xCellA1->getText()), u"ParaStyleName"_ustr));

    CPPUNIT_ASSERT_EQUAL(
        u"Heading 1"_ustr,
        getProperty<OUString>(getParagraphOfText(1, xCellB1->getText()), u"ParaStyleName"_ustr));

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(
        u"Table Contents"_ustr,
        getProperty<OUString>(getParagraphOfText(1, xCellA1->getText()), u"ParaStyleName"_ustr));

    CPPUNIT_ASSERT_EQUAL(
        u"Table Contents"_ustr,
        getProperty<OUString>(getParagraphOfText(1, xCellB1->getText()), u"ParaStyleName"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf145151)
{
    createSwDoc();
    SwWrtShell* pWrtSh = getSwDocShell()->GetWrtShell();

    uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence(
        { { "Rows", uno::Any(sal_Int32(2)) }, { "Columns", uno::Any(sal_Int32(2)) } }));

    dispatchCommand(mxComponent, u".uno:InsertTable"_ustr, aArgs);

    pWrtSh->Insert(u"Col1"_ustr);

    // Move the cursor to B1
    pWrtSh->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);

    pWrtSh->Insert(u"Col2"_ustr);

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTextTable->getColumns()->getCount());

    uno::Reference<text::XTextRange> xCellA1(xTextTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Col1"_ustr, xCellA1->getString());

    uno::Reference<text::XTextRange> xCellB1(xTextTable->getCellByName(u"B1"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Col2"_ustr, xCellB1->getString());

    // Select backwards B1 and A1 (select "2loC<cell>" which ends up selecting both cells)
    pWrtSh->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 5, /*bBasicCall=*/false);

    // Just select the whole B1
    pWrtSh->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);

    dispatchCommand(mxComponent, u".uno:DefaultNumbering"_ustr, {});

    // B1 should now have a numbering style, but A1 should not be affected.
    OUString sNumStyleB1 = getProperty<OUString>(getParagraphOfText(1, xCellB1->getText()),
                                                 u"NumberingStyleName"_ustr);
    CPPUNIT_ASSERT(!sNumStyleB1.isEmpty());
    CPPUNIT_ASSERT_MESSAGE(
        "Only cell B1 was selected. A1 should not have any numbering.",
        getProperty<OUString>(getParagraphOfText(1, xCellA1->getText()), u"NumberingStyleName"_ustr)
            .isEmpty());

    // Toggle it back off
    dispatchCommand(mxComponent, u".uno:DefaultNumbering"_ustr, {});

    CPPUNIT_ASSERT_MESSAGE(
        "Cell B1 must be able to toggle numbering on and off.",
        getProperty<OUString>(getParagraphOfText(1, xCellB1->getText()), u"NumberingStyleName"_ustr)
            .isEmpty());

    // Now test removing numbering/bullets
    // Add A1 to the current B1 selection
    pWrtSh->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);

    // Toggle on bullet numbering
    dispatchCommand(mxComponent, u".uno:DefaultBullet"_ustr, {});

    // sanity check - both cells have bullets turned on.
    OUString sNumStyleA1 = getProperty<OUString>(getParagraphOfText(1, xCellA1->getText()),
                                                 u"NumberingStyleName"_ustr);
    CPPUNIT_ASSERT(!sNumStyleA1.isEmpty());
    CPPUNIT_ASSERT_EQUAL(sNumStyleA1,
                         getProperty<OUString>(getParagraphOfText(1, xCellB1->getText()),
                                               u"NumberingStyleName"_ustr));
    CPPUNIT_ASSERT(sNumStyleA1 != sNumStyleB1); // therefore B1 changed from numbering to bullets

    // Just select cell B1
    pWrtSh->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);

    // Toggle off bullet numbering
    dispatchCommand(mxComponent, u".uno:DefaultBullet"_ustr, {});

    // B1 should now have removed all numbering, while A1 should still have the bullet.
    CPPUNIT_ASSERT(
        getProperty<OUString>(getParagraphOfText(1, xCellB1->getText()), u"NumberingStyleName"_ustr)
            .isEmpty());
    CPPUNIT_ASSERT_MESSAGE("Only cell B1 was selected. A1 should still have bullets turned on.",
                           !getProperty<OUString>(getParagraphOfText(1, xCellA1->getText()),
                                                  u"NumberingStyleName"_ustr)
                                .isEmpty());

    // Toggle it back on
    dispatchCommand(mxComponent, u".uno:DefaultBullet"_ustr, {});

    CPPUNIT_ASSERT(!getProperty<OUString>(getParagraphOfText(1, xCellB1->getText()),
                                          u"NumberingStyleName"_ustr)
                        .isEmpty());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf126735)
{
    createSwDoc("tdf39721.fodt");
    SwDoc* pDoc = getSwDoc();

    //turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // check next selected tracked change
    dispatchCommand(mxComponent, u".uno:NextTrackedChange"_ustr, {});
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<view::XSelectionSupplier> xSelSupplier(xModel->getCurrentController(),
                                                          uno::UNO_QUERY_THROW);
    uno::Any aSelection = xSelSupplier->getSelection();
    uno::Reference<text::XTextRange> xTextRange = getAssociatedTextRange(aSelection);
    CPPUNIT_ASSERT(xTextRange);
    CPPUNIT_ASSERT_EQUAL(u" ipsu"_ustr, xTextRange->getString());

    // check next selected tracked change
    dispatchCommand(mxComponent, u".uno:NextTrackedChange"_ustr, {});
    aSelection = xSelSupplier->getSelection();
    xTextRange = getAssociatedTextRange(aSelection);
    CPPUNIT_ASSERT(xTextRange);
    CPPUNIT_ASSERT_EQUAL(u"or "_ustr, xTextRange->getString());

    // check next selected tracked change at the end of the document:
    // select the first tracked change of the document
    dispatchCommand(mxComponent, u".uno:NextTrackedChange"_ustr, {});
    aSelection = xSelSupplier->getSelection();
    xTextRange = getAssociatedTextRange(aSelection);
    CPPUNIT_ASSERT(xTextRange);
    // This was empty (collapsing at the end of the last tracked change)
    CPPUNIT_ASSERT_EQUAL(u" ipsu"_ustr, xTextRange->getString());

    // check the previous tracked change at the start of the document:
    // select the last tracked change of the document
    dispatchCommand(mxComponent, u".uno:PreviousTrackedChange"_ustr, {});
    aSelection = xSelSupplier->getSelection();
    xTextRange = getAssociatedTextRange(aSelection);
    CPPUNIT_ASSERT(xTextRange);
    // This was empty (collapsing at the start of the last tracked change)
    CPPUNIT_ASSERT_EQUAL(u"or "_ustr, xTextRange->getString());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
