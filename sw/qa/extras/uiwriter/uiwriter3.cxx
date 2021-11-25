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
#include <com/sun/star/table/TableBorder2.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <comphelper/propertysequence.hxx>
#include <boost/property_tree/json_parser.hpp>
#include <fmtanchr.hxx>
#include <frameformats.hxx>
#include <swdtflvr.hxx>
#include <textboxhelper.hxx>
#include <o3tl/safeint.hxx>
#include <tools/json_writer.hxx>
#include <unotools/mediadescriptor.hxx>
#include <unotools/streamwrap.hxx>
#include <sfx2/linkmgr.hxx>

#include <fmtinfmt.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <unotxdoc.hxx>
#include <docsh.hxx>
#include <drawdoc.hxx>
#include <dcontact.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdview.hxx>
#include <ndtxt.hxx>
#include <txtfld.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentLinksAdministration.hxx>
#include <IDocumentRedlineAccess.hxx>

namespace
{
constexpr OUStringLiteral DATA_DIRECTORY = u"/sw/qa/extras/uiwriter/data/";
} // namespace

/// Third set of tests asserting the behavior of Writer user interface shells.
class SwUiWriterTest3 : public SwModelTestBase
{
};

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf145731)
{
    createSwDoc(DATA_DIRECTORY, "tdf145731.odt");

    CPPUNIT_ASSERT_EQUAL(9, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    Scheduler::ProcessEventsToIdle();

    dispatchCommand(mxComponent, ".uno:Cut", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(0, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, ".uno:Paste", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(9, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf129382)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf129382.docx");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    CPPUNIT_ASSERT_EQUAL(8, getShapes());
    CPPUNIT_ASSERT_EQUAL(2, getPages());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    rtl::Reference<SwTransferable> xTransfer = new SwTransferable(*pWrtShell);
    xTransfer->Cut();

    CPPUNIT_ASSERT_EQUAL(3, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    TransferableDataHelper aHelper(xTransfer);
    SwTransferable::Paste(*pWrtShell, aHelper);

    CPPUNIT_ASSERT_EQUAL(8, getShapes());
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(3, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    // without the fix, it crashes
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(8, getShapes());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf135662)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf135662.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    CPPUNIT_ASSERT_EQUAL(2, getShapes());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    rtl::Reference<SwTransferable> xTransfer = new SwTransferable(*pWrtShell);
    xTransfer->Cut();

    CPPUNIT_ASSERT_EQUAL(0, getShapes());

    TransferableDataHelper aHelper(xTransfer);
    SwTransferable::Paste(*pWrtShell, aHelper);

    // Without the fix in place, this test would have failed with
    // - Expected: 2
    // - Actual  : 1
    CPPUNIT_ASSERT_EQUAL(2, getShapes());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf134227)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf134227.docx");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    CPPUNIT_ASSERT_EQUAL(4, getShapes());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    // Without the fix in place, it would have crashed here
    rtl::Reference<SwTransferable> xTransfer = new SwTransferable(*pWrtShell);
    xTransfer->Cut();

    CPPUNIT_ASSERT_EQUAL(0, getShapes());

    TransferableDataHelper aHelper(xTransfer);
    SwTransferable::Paste(*pWrtShell, aHelper);

    CPPUNIT_ASSERT_EQUAL(4, getShapes());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(0, getShapes());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(4, getShapes());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf135412)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf135412.docx");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    CPPUNIT_ASSERT_EQUAL(4, getShapes());
    uno::Reference<text::XTextRange> xShape(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("X"), xShape->getString());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    rtl::Reference<SwTransferable> xTransfer = new SwTransferable(*pWrtShell);
    xTransfer->Cut();

    CPPUNIT_ASSERT_EQUAL(0, getShapes());

    TransferableDataHelper aHelper(xTransfer);
    SwTransferable::Paste(*pWrtShell, aHelper);

    CPPUNIT_ASSERT_EQUAL(4, getShapes());

    // Without the fix in place, the text in the shape wouldn't be pasted
    xShape.set(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("X"), xShape->getString());

    //Without the fix in place, it would have crashed here
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(0, getShapes());
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(4, getShapes());
    xShape.set(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("X"), xShape->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf138482)
{
    createSwDoc(DATA_DIRECTORY, "tdf138482.docx");

    CPPUNIT_ASSERT_EQUAL(2, getShapes());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    Scheduler::ProcessEventsToIdle();

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, ".uno:Delete", {});
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(0, getShapes());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(2, getShapes());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf132911)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf132911.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(4, getShapes());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    rtl::Reference<SwTransferable> xTransfer = new SwTransferable(*pWrtShell);
    xTransfer->Cut();
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(0, getShapes());

    TransferableDataHelper aHelper(xTransfer);
    SwTransferable::Paste(*pWrtShell, aHelper);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(4, getShapes());

    // Without the fix in place, it would have crashed here
    SwTransferable::Paste(*pWrtShell, aHelper);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(8, getShapes());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(4, getShapes());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(0, getShapes());

    SwTransferable::Paste(*pWrtShell, aHelper);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(4, getShapes());

    SwTransferable::Paste(*pWrtShell, aHelper);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(8, getShapes());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(4, getShapes());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(0, getShapes());

    //tdf#135247: Without the fix in place, this would have crashed
    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(4, getShapes());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf61154)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf61154.fodt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    pWrtShell->GotoNextTOXBase();

    // show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be off",
                           !pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    const SwTOXBase* pTOXBase = pWrtShell->GetCurTOX();
    pWrtShell->UpdateTableOf(*pTOXBase);
    SwCursorShell* pShell(pDoc->GetEditShell());
    SwTextNode* pTitleNode = pShell->GetCursor()->GetNode().GetTextNode();
    SwNodeIndex aIdx(*pTitleNode);

    // table of contents node shouldn't contain tracked deletion
    // This was "Text InsertedDeleted\t1"
    SwTextNode* pNext = static_cast<SwTextNode*>(pDoc->GetNodes().GoNext(&aIdx));
    CPPUNIT_ASSERT_EQUAL(OUString("Text Inserted\t1"), pNext->GetText());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf124904)
{
    // don't show deletions in referenced text,
    // (except if the full text is deleted)
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf124904.fodt");

    // show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // remove the first word "Heading" (with change tracking) to update the referenced text
    dispatchCommand(mxComponent, ".uno:DelToEndOfWord", {});
    Scheduler::ProcessEventsToIdle();

    // This was "Reference to Heading of document file"
    CPPUNIT_ASSERT_EQUAL(OUString("Reference to of example document "),
                         getParagraph(2)->getString());

    // don't hide the wholly deleted referenced text
    dispatchCommand(mxComponent, ".uno:DelToEndOfLine", {});
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(OUString("Reference to Heading of example document file"),
                         getParagraph(2)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf100691)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf100691.fodt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    pWrtShell->GotoNextTOXBase();

    const SwTOXBase* pTOXBase = pWrtShell->GetCurTOX();
    pWrtShell->UpdateTableOf(*pTOXBase);
    SwCursorShell* pShell(pDoc->GetEditShell());
    SwTextNode* pTitleNode = pShell->GetCursor()->GetNode().GetTextNode();
    SwNodeIndex aIdx(*pTitleNode);

    // table of contents node shouldn't contain invisible text
    // This was "Text Hidden\t1"
    SwTextNode* pNext = static_cast<SwTextNode*>(pDoc->GetNodes().GoNext(&aIdx));
    CPPUNIT_ASSERT_EQUAL(OUString("Text\t1"), pNext->GetText());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf134404)
{
    createSwDoc(DATA_DIRECTORY, "tdf134404.odt");

    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    dispatchCommand(mxComponent, ".uno:GoToEndOfPage", {});
    Scheduler::ProcessEventsToIdle();
    dispatchCommand(mxComponent, ".uno:InsertPagebreak", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(2, getPages());

    dispatchCommand(mxComponent, ".uno:SwBackspace", {});
    Scheduler::ProcessEventsToIdle();

    // Without the fix in place, the image would have been deleted
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf112342)
{
    createSwDoc(DATA_DIRECTORY, "tdf112342.docx");

    //Get the last para
    uno::Reference<text::XTextRange> xPara = getParagraph(3);
    auto xCur = xPara->getText()->createTextCursor();
    //Go to the end of it
    xCur->gotoEnd(false);
    //And let's remove the last 2 chars (the last para with its char).
    xCur->goLeft(2, true);
    xCur->setString("");

    //If the second paragraph on the second page, this will be passed.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Page break does not match", 2, getPages());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf132321)
{
    createSwDoc(DATA_DIRECTORY, "tdf132321.odt");

    CPPUNIT_ASSERT_EQUAL(1, getShapes());

    dispatchCommand(mxComponent, ".uno:GoToEndOfPage", {});
    Scheduler::ProcessEventsToIdle();
    dispatchCommand(mxComponent, ".uno:SwBackspace", {});
    Scheduler::ProcessEventsToIdle();

    // Without the fix in place, the button form would have also been deleted
    CPPUNIT_ASSERT_EQUAL(1, getShapes());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf135056)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf135056.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    CPPUNIT_ASSERT_EQUAL(sal_uInt16(1), pWrtShell->GetTOXCount());

    const SwTOXBase* pTOX = pWrtShell->GetTOX(0);
    CPPUNIT_ASSERT(pTOX);

    //Without the fix in place, it would have hung here
    pWrtShell->DeleteTOX(*pTOX, true);

    CPPUNIT_ASSERT_EQUAL(sal_uInt16(0), pWrtShell->GetTOXCount());

    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT_EQUAL(sal_uInt16(1), pWrtShell->GetTOXCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf132597)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf132597.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    rtl::Reference<SwTransferable> xTransfer = new SwTransferable(*pWrtShell);
    xTransfer->Copy();

    // Paste special as RTF
    uno::Sequence<beans::PropertyValue> aPropertyValues = comphelper::InitPropertySequence(
        { { "SelectedFormat", uno::makeAny(static_cast<sal_uInt32>(SotClipboardFormatId::RTF)) } });

    dispatchCommand(mxComponent, ".uno:ClipboardFormatItems", aPropertyValues);
    Scheduler::ProcessEventsToIdle();

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf144840)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf144840.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    rtl::Reference<SwTransferable> xTransfer = new SwTransferable(*pWrtShell);
    xTransfer->Cut();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xIndexAccess->getCount());

    // Paste special as RTF
    uno::Sequence<beans::PropertyValue> aPropertyValues = comphelper::InitPropertySequence(
        { { "SelectedFormat", uno::makeAny(static_cast<sal_uInt32>(SotClipboardFormatId::RTF)) } });

    dispatchCommand(mxComponent, ".uno:ClipboardFormatItems", aPropertyValues);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    uno::Reference<text::XTextTable> xTextTable(xIndexAccess->getByIndex(0), uno::UNO_QUERY);

    table::TableBorder2 tableBorder = getProperty<table::TableBorder2>(xTextTable, "TableBorder2");

    // Without the fix in place, this test would have failed with
    // - Expected: 4
    // - Actual  : 0
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(4), tableBorder.TopLine.LineWidth);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(4), tableBorder.LeftLine.LineWidth);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(4), tableBorder.RightLine.LineWidth);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(4), tableBorder.BottomLine.LineWidth);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf131963)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf131963.docx");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    CPPUNIT_ASSERT_EQUAL(11, getPages());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    rtl::Reference<SwTransferable> xTransfer = new SwTransferable(*pWrtShell);
    xTransfer->Copy();

    // Paste special as RTF
    uno::Sequence<beans::PropertyValue> aPropertyValues = comphelper::InitPropertySequence(
        { { "SelectedFormat", uno::makeAny(static_cast<sal_uInt32>(SotClipboardFormatId::RTF)) } });

    dispatchCommand(mxComponent, ".uno:ClipboardFormatItems", aPropertyValues);
    Scheduler::ProcessEventsToIdle();

    // Without the fix in place, this test would have crashed here

    // tdf#133169: without the fix in place, it would have been 2 instead of 11
    CPPUNIT_ASSERT_EQUAL(11, getPages());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf132596)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf132596.docx");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    CPPUNIT_ASSERT_EQUAL(2, getPages());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    rtl::Reference<SwTransferable> xTransfer = new SwTransferable(*pWrtShell);
    xTransfer->Copy();

    // Paste special as RTF
    uno::Sequence<beans::PropertyValue> aPropertyValues = comphelper::InitPropertySequence(
        { { "SelectedFormat", uno::makeAny(static_cast<sal_uInt32>(SotClipboardFormatId::RTF)) } });

    dispatchCommand(mxComponent, ".uno:ClipboardFormatItems", aPropertyValues);
    Scheduler::ProcessEventsToIdle();

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf126626)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf126626.docx");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    CPPUNIT_ASSERT_EQUAL(2, getShapes());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    rtl::Reference<SwTransferable> xTransfer = new SwTransferable(*pWrtShell);
    xTransfer->Copy();

    CPPUNIT_ASSERT_EQUAL(2, getShapes());

    TransferableDataHelper aHelper(xTransfer);
    SwTransferable::Paste(*pWrtShell, aHelper);
    CPPUNIT_ASSERT_EQUAL(2, getShapes());

    SwTransferable::Paste(*pWrtShell, aHelper);
    CPPUNIT_ASSERT_EQUAL(4, getShapes());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(2, getShapes());

    // without the fix, it crashes
    SwTransferable::Paste(*pWrtShell, aHelper);
    CPPUNIT_ASSERT_EQUAL(4, getShapes());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf133967)
{
    createSwDoc(DATA_DIRECTORY, "tdf133967.odt");

    CPPUNIT_ASSERT_EQUAL(6, getPages());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    dispatchCommand(mxComponent, ".uno:Cut", {});
    Scheduler::ProcessEventsToIdle();

    for (sal_Int32 i = 0; i < 10; ++i)
    {
        dispatchCommand(mxComponent, ".uno:Undo", {});
        Scheduler::ProcessEventsToIdle();

        dispatchCommand(mxComponent, ".uno:Redo", {});
        Scheduler::ProcessEventsToIdle();
    }

    // Without the fix in place, this test would have failed with:
    //- Expected: 1
    //- Actual  : 45
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf132187)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf132187.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    CPPUNIT_ASSERT_EQUAL(1, getPages());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    rtl::Reference<SwTransferable> xTransfer = new SwTransferable(*pWrtShell);
    xTransfer->Copy();

    dispatchCommand(mxComponent, ".uno:GoToEndOfDoc", {});

    TransferableDataHelper aHelper(xTransfer);
    for (sal_Int32 i = 0; i < 10; ++i)
    {
        SwTransferable::Paste(*pWrtShell, aHelper);
        Scheduler::ProcessEventsToIdle();
    }

    //without the fix in place, this test would fail with:
    //- Expected: 1
    //- Actual  : 70

    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf135733)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf135733.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    CPPUNIT_ASSERT_EQUAL(2, getShapes());

    //Move the cursor inside the table
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);

    //Select first column
    pWrtShell->Down(/*bSelect=*/true);
    pWrtShell->Down(/*bSelect=*/true);
    pWrtShell->Down(/*bSelect=*/true);

    rtl::Reference<SwTransferable> xTransfer = new SwTransferable(*pWrtShell);
    xTransfer->Cut();

    pWrtShell->SttPg(/*bSelect=*/false);

    TransferableDataHelper aHelper(xTransfer);
    SwTransferable::Paste(*pWrtShell, aHelper);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    CPPUNIT_ASSERT_EQUAL(2, getShapes());

    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    CPPUNIT_ASSERT_EQUAL(0, getShapes());

    // without the fix, it crashes
    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    CPPUNIT_ASSERT_EQUAL(2, getShapes());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf128739)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf128739.docx");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    CPPUNIT_ASSERT_EQUAL(OUString("Fehler: Verweis nicht gefunden"), getParagraph(1)->getString());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    rtl::Reference<SwTransferable> xTransfer = new SwTransferable(*pWrtShell);
    xTransfer->Cut();

    CPPUNIT_ASSERT_EQUAL(OUString(""), getParagraph(1)->getString());

    TransferableDataHelper aHelper(xTransfer);
    SwTransferable::Paste(*pWrtShell, aHelper);
    CPPUNIT_ASSERT_EQUAL(OUString("Fehler: Verweis nicht gefunden"), getParagraph(1)->getString());

    // without the fix, it crashes
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(OUString(""), getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf124722)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf124722.rtf");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    CPPUNIT_ASSERT_EQUAL(22, getPages());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    rtl::Reference<SwTransferable> xTransfer = new SwTransferable(*pWrtShell);
    xTransfer->Copy();

    CPPUNIT_ASSERT_EQUAL(22, getPages());

    TransferableDataHelper aHelper(xTransfer);
    SwTransferable::Paste(*pWrtShell, aHelper);
    CPPUNIT_ASSERT_EQUAL(43, getPages());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(22, getPages());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testToxmarkLinks)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "udindex3.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwView& rView(*pDoc->GetDocShell()->GetView());

    // update indexes
    for (auto i = pWrtShell->GetTOXCount(); 0 < i;)
    {
        --i;
        pWrtShell->UpdateTableOf(*pWrtShell->GetTOX(i));
    }

    // click on the links...
    {
        OUString const tmp("Table of Contents");
        pWrtShell->GotoNextTOXBase(&tmp);
    }

    { // ToC toxmark
        pWrtShell->Down(false);
        SfxItemSet aSet(pWrtShell->GetAttrPool(),
                        svl::Items<RES_TXTATR_INETFMT, RES_TXTATR_INETFMT>);
        pWrtShell->GetCurAttr(aSet);
        CPPUNIT_ASSERT(aSet.HasItem(RES_TXTATR_INETFMT));
        pWrtShell->Push();
        OUString const url(aSet.GetItem<SwFormatINetFormat>(RES_TXTATR_INETFMT)->GetValue());
        CPPUNIT_ASSERT_EQUAL(OUString("#1%19the%20tocmark%19C%7Ctoxmark"), url);
        rView.JumpToSwMark(url.subView(1)); // SfxApplication::OpenDocExec_Impl eats the "#"
        CPPUNIT_ASSERT_EQUAL(OUString(OUStringChar(CH_TXTATR_INWORD) + "tocmark"),
                             pWrtShell->GetCursor()->GetNode().GetTextNode()->GetText());
        pWrtShell->Pop(SwCursorShell::PopMode::DeleteCurrent);
    }

    { // ToC heading
        pWrtShell->Down(false);
        SfxItemSet aSet(pWrtShell->GetAttrPool(),
                        svl::Items<RES_TXTATR_INETFMT, RES_TXTATR_INETFMT>);
        pWrtShell->GetCurAttr(aSet);
        CPPUNIT_ASSERT(aSet.HasItem(RES_TXTATR_INETFMT));
        pWrtShell->Push();
        OUString const url(aSet.GetItem<SwFormatINetFormat>(RES_TXTATR_INETFMT)->GetValue());
        CPPUNIT_ASSERT_EQUAL(OUString("#__RefHeading___Toc105_706348105"), url);
        rView.JumpToSwMark(url.subView(1));
        CPPUNIT_ASSERT_EQUAL(OUString("foo"),
                             pWrtShell->GetCursor()->GetNode().GetTextNode()->GetText());
        pWrtShell->Pop(SwCursorShell::PopMode::DeleteCurrent);
    }

    {
        OUString const tmp("User-Defined1");
        pWrtShell->GotoNextTOXBase(&tmp);
    }

    { // UD1 toxmark 1
        pWrtShell->Down(false);
        SfxItemSet aSet(pWrtShell->GetAttrPool(),
                        svl::Items<RES_TXTATR_INETFMT, RES_TXTATR_INETFMT>);
        pWrtShell->GetCurAttr(aSet);
        CPPUNIT_ASSERT(aSet.HasItem(RES_TXTATR_INETFMT));
        pWrtShell->Push();
        OUString const url(aSet.GetItem<SwFormatINetFormat>(RES_TXTATR_INETFMT)->GetValue());
        CPPUNIT_ASSERT_EQUAL(OUString("#1%19the%20udmark%19UUser-Defined%7Ctoxmark"), url);
        rView.JumpToSwMark(url.subView(1));
        CPPUNIT_ASSERT_EQUAL(OUString(OUStringChar(CH_TXTATR_INWORD) + "udmark the first"),
                             pWrtShell->GetCursor()->GetNode().GetTextNode()->GetText());
        pWrtShell->Pop(SwCursorShell::PopMode::DeleteCurrent);
    }

    { // UD1 toxmark 2 (with same text)
        pWrtShell->Down(false);
        SfxItemSet aSet(pWrtShell->GetAttrPool(),
                        svl::Items<RES_TXTATR_INETFMT, RES_TXTATR_INETFMT>);
        pWrtShell->GetCurAttr(aSet);
        CPPUNIT_ASSERT(aSet.HasItem(RES_TXTATR_INETFMT));
        pWrtShell->Push();
        OUString const url(aSet.GetItem<SwFormatINetFormat>(RES_TXTATR_INETFMT)->GetValue());
        CPPUNIT_ASSERT_EQUAL(OUString("#2%19the%20udmark%19UUser-Defined%7Ctoxmark"), url);
        rView.JumpToSwMark(url.subView(1));
        CPPUNIT_ASSERT_EQUAL(OUString(OUStringChar(CH_TXTATR_INWORD) + "udmark the 2nd"),
                             pWrtShell->GetCursor()->GetNode().GetTextNode()->GetText());
        pWrtShell->Pop(SwCursorShell::PopMode::DeleteCurrent);
    }

    { // UD heading
        pWrtShell->Down(false);
        SfxItemSet aSet(pWrtShell->GetAttrPool(),
                        svl::Items<RES_TXTATR_INETFMT, RES_TXTATR_INETFMT>);
        pWrtShell->GetCurAttr(aSet);
        CPPUNIT_ASSERT(aSet.HasItem(RES_TXTATR_INETFMT));
        pWrtShell->Push();
        OUString const url(aSet.GetItem<SwFormatINetFormat>(RES_TXTATR_INETFMT)->GetValue());
        CPPUNIT_ASSERT_EQUAL(OUString("#__RefHeading___Toc105_706348105"), url);
        rView.JumpToSwMark(url.subView(1));
        CPPUNIT_ASSERT_EQUAL(OUString("foo"),
                             pWrtShell->GetCursor()->GetNode().GetTextNode()->GetText());
        pWrtShell->Pop(SwCursorShell::PopMode::DeleteCurrent);
    }

    {
        OUString const tmp("NewUD!|1");
        pWrtShell->GotoNextTOXBase(&tmp);
    }

    { // UD2 toxmark, with same text as those in other UD
        pWrtShell->Down(false);
        SfxItemSet aSet(pWrtShell->GetAttrPool(),
                        svl::Items<RES_TXTATR_INETFMT, RES_TXTATR_INETFMT>);
        pWrtShell->GetCurAttr(aSet);
        CPPUNIT_ASSERT(aSet.HasItem(RES_TXTATR_INETFMT));
        pWrtShell->Push();
        OUString const url(aSet.GetItem<SwFormatINetFormat>(RES_TXTATR_INETFMT)->GetValue());
        CPPUNIT_ASSERT_EQUAL(OUString("#1%19the%20udmark%19UNewUD!%7C%7Ctoxmark"), url);
        rView.JumpToSwMark(url.subView(1));
        CPPUNIT_ASSERT_EQUAL(OUString("the udmark"),
                             pWrtShell->GetCursor()->GetNode().GetTextNode()->GetText());
        pWrtShell->Pop(SwCursorShell::PopMode::DeleteCurrent);
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf139922)
{
    SwDoc* const pDoc = createSwDoc();
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());

    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_RETURN);
    Scheduler::ProcessEventsToIdle();

    SwWrtShell* const pWrtSh = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtSh);

    pWrtSh->Insert("this _is_ a SEntence. this _is_ a SEntence.");

    CPPUNIT_ASSERT_EQUAL(OUString("this _is_ a SEntence. this _is_ a SEntence."),
                         getParagraph(2)->getString());

    //apply autocorrect StartAutoCorrect
    dispatchCommand(mxComponent, ".uno:AutoFormatApply", {});
    Scheduler::ProcessEventsToIdle();

    // Without the fix in place, this test would have failed with
    // - Expected: This is a Sentence. This is a Sentence.
    // - Actual  : this is a Sentence. This is a Sentence.
    CPPUNIT_ASSERT_EQUAL(OUString("This is a Sentence. This is a Sentence."),
                         getParagraph(2)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf125261)
{
    createSwDoc(DATA_DIRECTORY, "tdf125261.odt");

    CPPUNIT_ASSERT_EQUAL(OUString("https://www.example.com/"),
                         getProperty<OUString>(getRun(getParagraph(1), 1), "HyperLinkURL"));
    //apply autocorrect StartAutoCorrect
    dispatchCommand(mxComponent, ".uno:AutoFormatApply", {});
    CPPUNIT_ASSERT_EQUAL(OUString("https://www.example.com/"),
                         getProperty<OUString>(getRun(getParagraph(1), 1), "HyperLinkURL"));
    // without the fix, it hangs
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(OUString("https://www.example.com/"),
                         getProperty<OUString>(getRun(getParagraph(1), 1), "HyperLinkURL"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf133990)
{
    createSwDoc(DATA_DIRECTORY, "tdf133990.odt");

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    //Use selectAll 3 times in a row
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    dispatchCommand(mxComponent, ".uno:Delete", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xIndexAccess->getCount());

    //Without the fix in place, it would have crashed here
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf126504)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf126504.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    //Use selectAll 2 times in a row
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    rtl::Reference<SwTransferable> xTransfer = new SwTransferable(*pWrtShell);
    xTransfer->Copy();

    dispatchCommand(mxComponent, ".uno:GoToEndOfPage", {});
    Scheduler::ProcessEventsToIdle();

    TransferableDataHelper aHelper(xTransfer);
    SwTransferable::Paste(*pWrtShell, aHelper);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    dispatchCommand(mxComponent, ".uno:Redo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    //Without the fix in place, it would have crashed here
    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf133982)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf133982.docx");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());

    //Use selectAll 3 times in a row
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    rtl::Reference<SwTransferable> xTransfer = new SwTransferable(*pWrtShell);
    //Without the fix in place, it would have crashed here
    xTransfer->Cut();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xIndexAccess->getCount());

    TransferableDataHelper aHelper(xTransfer);
    SwTransferable::Paste(*pWrtShell, aHelper);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf134253)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf134253.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(6, getPages());

    //Use selectAll 3 times in a row
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    rtl::Reference<SwTransferable> xTransfer = new SwTransferable(*pWrtShell);
    xTransfer->Copy();
    TransferableDataHelper aHelper(xTransfer);
    SwTransferable::Paste(*pWrtShell, aHelper);

    //Without the fix in place, it would have crashed here
    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(6, getPages());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, TestAsCharTextBox)
{
    // Related tickets:
    // tdf#138598 Replace vertical alignment of As_char textboxes in footer
    // tdf#140158 Remove horizontal positioning of As_char textboxes, because
    // the anchor moving does the same for it.

    createSwDoc(DATA_DIRECTORY, "AsCharTxBxTest.docx");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());

    // Add 3x tab to the doc
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
    Scheduler::ProcessEventsToIdle();

    auto pExportDump = parseLayoutDump();
    CPPUNIT_ASSERT(pExportDump);

    // Check if the texbox fallen apart due to the tabs
    const double nLeftSideOfShape1
        = getXPath(pExportDump, "/root/page/body/txt/anchored/SwAnchoredDrawObject/bounds", "left")
              .toDouble();
    const double nLeftSideOfTxBx1
        = getXPath(pExportDump, "/root/page/body/txt/anchored/fly/infos/bounds", "left").toDouble();

    CPPUNIT_ASSERT(nLeftSideOfShape1 < nLeftSideOfTxBx1);

    // Another test is for the tdf#138598: Check footer textbox
    const double nLeftSideOfShape2
        = getXPath(pExportDump, "/root/page[2]/footer/txt/anchored/SwAnchoredDrawObject/bounds",
                   "left")
              .toDouble();
    const double nLeftSideOfTxBx2
        = getXPath(pExportDump, "/root/page[2]/footer/txt/anchored/fly/infos/bounds", "left")
              .toDouble();

    CPPUNIT_ASSERT(nLeftSideOfShape2 < nLeftSideOfTxBx2);

    const double nTopSideOfShape2
        = getXPath(pExportDump, "/root/page[2]/footer/txt/anchored/SwAnchoredDrawObject/bounds",
                   "top")
              .toDouble();
    const double nTopSideOfTxBx2
        = getXPath(pExportDump, "/root/page[2]/footer/txt/anchored/fly/infos/bounds", "top")
              .toDouble();

    CPPUNIT_ASSERT(nTopSideOfShape2 < nTopSideOfTxBx2);
    // Without the fix in place the two texboxes has been fallen apart, and  asserts will broken.
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf140975)
{
    // Load the bugdoc
    createSwDoc(DATA_DIRECTORY, "tdf140975.docx");

    // Set the Anchor of the shape to As_Char
    dispatchCommand(mxComponent, ".uno:JumpToNextFrame", {});
    Scheduler::ProcessEventsToIdle();
    dispatchCommand(mxComponent, ".uno:SetAnchorToChar", {});
    Scheduler::ProcessEventsToIdle();

    // Get the layout of the textbox
    auto pExportDump = parseLayoutDump();
    CPPUNIT_ASSERT(pExportDump);

    const sal_Int32 nShpTop
        = getXPath(pExportDump, "/root/page/body/txt[4]/anchored/SwAnchoredDrawObject/bounds",
                   "top")
              .toInt32();
    const sal_Int32 nFrmTop
        = getXPath(pExportDump, "/root/page/body/txt[4]/anchored/fly/infos/bounds", "top")
              .toInt32();

    // Without the fix in place, the frame has less value for Top than
    // the shape. This means the frame is outside from the shape.
    CPPUNIT_ASSERT_GREATER(nShpTop, nFrmTop);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf76636)
{
    createSwDoc(DATA_DIRECTORY, "tdf76636.doc");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), xTextTable->getColumns()->getCount());

    //go to middle row
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_DOWN);
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_DOWN);
    Scheduler::ProcessEventsToIdle();

    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
    Scheduler::ProcessEventsToIdle();

    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_LEFT | KEY_SHIFT);
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_LEFT | KEY_SHIFT);
    Scheduler::ProcessEventsToIdle();

    dispatchCommand(mxComponent, ".uno:MergeCells", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xTextTable->getColumns()->getCount());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), xTextTable->getColumns()->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf76636_2)
{
    createSwDoc(DATA_DIRECTORY, "tdf76636.doc");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), xTextTable->getColumns()->getCount());

    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_DOWN);
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_DOWN);
    Scheduler::ProcessEventsToIdle();

    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
    Scheduler::ProcessEventsToIdle();

    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_RIGHT | KEY_SHIFT);
    Scheduler::ProcessEventsToIdle();

    dispatchCommand(mxComponent, ".uno:MergeCells", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), xTextTable->getColumns()->getCount());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), xTextTable->getColumns()->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf143574)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf143574.odt");

    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<drawing::XShapeDescriptor> xShapeDescriptor(xGroup, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("com.sun.star.drawing.GroupShape"),
                         xShapeDescriptor->getShapeType());

    dispatchCommand(mxComponent, ".uno:JumpToNextFrame", {});
    Scheduler::ProcessEventsToIdle();

    dispatchCommand(mxComponent, ".uno:EnterGroup", {});
    Scheduler::ProcessEventsToIdle();

    SdrPage* pDrawPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pObjGroup = pDrawPage->GetObj(0);
    SdrObject* pObjShape = pObjGroup->GetSubList()->GetObj(0);
    CPPUNIT_ASSERT_EQUAL(OUString("Alakzat1"), pObjShape->GetName());

    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->SelectObj(Point(), 0, pObjShape);

    // Make sure that one shape is selected.
    const SdrMarkList& rMarkList = pWrtShell->GetDrawView()->GetMarkedObjectList();
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rMarkList.GetMarkCount());

    // At this point Writer crashed here before the fix.
    dispatchCommand(mxComponent, ".uno:AddTextBox", {});
    Scheduler::ProcessEventsToIdle();

    uno::Reference<beans::XPropertySet> xShapeProps(xGroup->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(true, xShapeProps->getPropertyValue("TextBox").get<bool>());

    dispatchCommand(mxComponent, ".uno:RemoveTextBox", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(false, xShapeProps->getPropertyValue("TextBox").get<bool>());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf140828)
{
    createSwDoc(DATA_DIRECTORY, "tdf140828.docx");

    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    uno::Reference<drawing::XShape> xShp = getShape(1);
    CPPUNIT_ASSERT(xShp);

    uno::Reference<beans::XPropertySet> ShpProps(xShp, uno::UNO_QUERY_THROW);
    dispatchCommand(mxComponent, ".uno:JumpToNextFrame", {});
    Scheduler::ProcessEventsToIdle();

    dispatchCommand(mxComponent, ".uno:SetAnchorAtChar", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT(ShpProps->getPropertyValue("AnchorType").get<text::TextContentAnchorType>()
                   != text::TextContentAnchorType::TextContentAnchorType_AS_CHARACTER);

    uno::Reference<text::XTextFrame> xTxBx(SwTextBoxHelper::getUnoTextFrame(xShp));
    CPPUNIT_ASSERT(xTxBx);

    uno::Reference<beans::XPropertySet> TxBxProps(xTxBx, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("top left image"), xTxBx->getText()->getString());

    CPPUNIT_ASSERT_MESSAGE("Bad Relative Orientation and Position!",
                           TxBxProps->getPropertyValue("HoriOrientRelation").get<sal_Int16>()
                               != text::RelOrientation::CHAR);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf132725)
{
    createSwDoc(DATA_DIRECTORY, "tdf132725.odt");

    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(OUString("AA"), getParagraph(1)->getString());

    dispatchCommand(mxComponent, ".uno:GoToEndOfPara", {});
    Scheduler::ProcessEventsToIdle();

    dispatchCommand(mxComponent, ".uno:SwBackspace", {});
    dispatchCommand(mxComponent, ".uno:SwBackspace", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(0, getShapes());
    CPPUNIT_ASSERT_EQUAL(OUString(""), getParagraph(1)->getString());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(OUString("AA"), getParagraph(1)->getString());

    dispatchCommand(mxComponent, ".uno:Redo", {});
    dispatchCommand(mxComponent, ".uno:Redo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(0, getShapes());
    CPPUNIT_ASSERT_EQUAL(OUString(""), getParagraph(1)->getString());

    //Without the fix in place, it would crash here
    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(OUString("A"), getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf126340)
{
    createSwDoc(DATA_DIRECTORY, "tdf126340.odt");

    dispatchCommand(mxComponent, ".uno:GoDown", {});
    // without the fix, it crashes
    dispatchCommand(mxComponent, ".uno:Delete", {});
    CPPUNIT_ASSERT_EQUAL(OUString("foo"), getParagraph(1)->getString());
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(OUString("foo"), getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf124397)
{
    createSwDoc(DATA_DIRECTORY, "tdf124397.docx");

    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(),
                                                         uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:Delete", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xIndexAccess->getCount());
    // Without the fix in place, it would crash here
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf108124)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf108124.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    uno::Reference<text::XTextGraphicObjectsSupplier> xTextGraphicObjectsSupplier(mxComponent,
                                                                                  uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(
        xTextGraphicObjectsSupplier->getGraphicObjects(), uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    rtl::Reference<SwTransferable> xTransfer = new SwTransferable(*pWrtShell);
    xTransfer->Copy();

    TransferableDataHelper aHelper(xTransfer);
    SwTransferable::Paste(*pWrtShell, aHelper);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());

    uno::Reference<drawing::XShape> xOldShape1(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xOldShape2(xIndexAccess->getByIndex(1), uno::UNO_QUERY);

    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());

    uno::Reference<drawing::XShape> xNewShape1(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xNewShape2(xIndexAccess->getByIndex(1), uno::UNO_QUERY);

    // there should be 2 different objects now but they have the same names,
    // so rely on the object identity for testing...
    CPPUNIT_ASSERT(xOldShape1.get() != xNewShape1.get());
    CPPUNIT_ASSERT(xOldShape1.get() != xNewShape2.get());
    CPPUNIT_ASSERT(xOldShape2.get() != xNewShape1.get());
    CPPUNIT_ASSERT(xOldShape2.get() != xNewShape2.get());

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, ".uno:Redo", {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf107975)
{
    // This test also covers tdf#117185 tdf#110442

    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf107975.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());

    uno::Reference<text::XTextGraphicObjectsSupplier> xTextGraphicObjectsSupplier(mxComponent,
                                                                                  uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(
        xTextGraphicObjectsSupplier->getGraphicObjects(), uno::UNO_QUERY);

    uno::Reference<drawing::XShape> xShape(xIndexAccess->getByIndex(0), uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_CHARACTER,
                         getProperty<text::TextContentAnchorType>(xShape, "AnchorType"));

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    rtl::Reference<SwTransferable> xTransfer = new SwTransferable(*pWrtShell);
    xTransfer->Copy();

    //Position the mouse cursor (caret) after "ABC" below the blue image
    dispatchCommand(mxComponent, ".uno:GoRight", {});
    {
        TransferableDataHelper aHelper(xTransfer);
        SwTransferable::Paste(*pWrtShell, aHelper);
    }

    // without the fix, it crashes
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(OUString("ABC"), getParagraph(1)->getString());
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    dispatchCommand(mxComponent, ".uno:Redo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    dispatchCommand(mxComponent, ".uno:Redo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    //try again with anchor at start of doc which is another special case
    xShape.set(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextContent> xShapeContent(xShape, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> const xStart = pTextDoc->getText()->getStart();
    xShapeContent->attach(xStart);

    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_CHARACTER,
                         getProperty<text::TextContentAnchorType>(xShape, "AnchorType"));

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    xTransfer.set(new SwTransferable(*pWrtShell));
    xTransfer->Copy();

    //Position the mouse cursor (caret) after "ABC" below the blue image
    dispatchCommand(mxComponent, ".uno:GoRight", {});
    {
        TransferableDataHelper aHelper(xTransfer);
        SwTransferable::Paste(*pWrtShell, aHelper);
    }

    // without the fix, it crashes
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(OUString("ABC"), getParagraph(1)->getString());
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    dispatchCommand(mxComponent, ".uno:Redo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    dispatchCommand(mxComponent, ".uno:Redo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf134021)
{
    createSwDoc(DATA_DIRECTORY, "tdf134021.docx");

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(12, getPages());

    dispatchCommand(mxComponent, ".uno:JumpToNextTable", {});

    dispatchCommand(mxComponent, ".uno:DeleteTable", {});
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    // Without the fix in place, it would have crashed here
    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(12, getPages());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf136778)
{
    createSwDoc(DATA_DIRECTORY, "tdf136778.docx");

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    dispatchCommand(mxComponent, ".uno:JumpToNextTable", {});

    dispatchCommand(mxComponent, ".uno:DeleteTable", {});
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    // Without the fix in place, it would have crashed here
    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf123285)
{
    createSwDoc(DATA_DIRECTORY, "tdf123285.odt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());

    CPPUNIT_ASSERT_EQUAL(true,
                         getParagraph(1)->getString().endsWith(
                             ".  Here is a short sentence demonstrating this very peculiar bug"
                             ".  Here is a short sentence demonstrating this very peculiar bug."));

    dispatchCommand(mxComponent, ".uno:GoToEndOfPage", {});
    Scheduler::ProcessEventsToIdle();

    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_RETURN);
    Scheduler::ProcessEventsToIdle();

    // Without the fix in place, the last "sentence" would have been changed to " entence"
    CPPUNIT_ASSERT_EQUAL(true,
                         getParagraph(1)->getString().endsWith(
                             ". Here is a short sentence demonstrating this very peculiar bug"
                             ".  Here is a short sentence demonstrating this very peculiar bug."));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf130746)
{
    createSwDoc(DATA_DIRECTORY, "tdf130746.odt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    dispatchCommand(mxComponent, ".uno:JumpToNextTable", {});

    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(),
                                              uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), xCursor->getPage());

    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_RETURN);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), xCursor->getPage());

    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_UP | KEY_MOD2);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), xCursor->getPage());

    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), xCursor->getPage());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf129805)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf129805.docx");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    CPPUNIT_ASSERT_EQUAL(OUString("x"), getParagraph(1)->getString());
    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    rtl::Reference<SwTransferable> xTransfer = new SwTransferable(*pWrtShell);
    // without the fix in place, it would crash here
    xTransfer->Cut();
    CPPUNIT_ASSERT_EQUAL(OUString(""), getParagraph(1)->getString());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(OUString("x"), getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf130685)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf130685.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    CPPUNIT_ASSERT_EQUAL(2, getPages());
    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    rtl::Reference<SwTransferable> xTransfer = new SwTransferable(*pWrtShell);
    xTransfer->Cut();
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(1, getPages());

    TransferableDataHelper aHelper(xTransfer);
    SwTransferable::Paste(*pWrtShell, aHelper);
    SwTransferable::Paste(*pWrtShell, aHelper);

    // Without fix in place, this test would have failed with:
    //- Expected: 2
    //- Actual  : 4
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(2, getPages());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf132944)
{
    createSwDoc(DATA_DIRECTORY, "tdf132944.odt");

    CPPUNIT_ASSERT_EQUAL(1, getPages());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:Delete", {});
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    // Without the fix in place, the document would have had 2 pages
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf104649)
{
    createSwDoc(DATA_DIRECTORY, "tdf104649.docx");

    CPPUNIT_ASSERT_EQUAL(OUString("Test"), getParagraph(1)->getString());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:Delete", {});
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(OUString("Test"), getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf134931)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf134931.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    rtl::Reference<SwTransferable> xTransfer = new SwTransferable(*pWrtShell);
    xTransfer->Copy();
    Scheduler::ProcessEventsToIdle();

    dispatchCommand(mxComponent, ".uno:GoDown", {});

    TransferableDataHelper aHelper(xTransfer);
    for (sal_Int32 i = 0; i < 10; ++i)
    {
        SwTransferable::Paste(*pWrtShell, aHelper);
        Scheduler::ProcessEventsToIdle();
    }

    CPPUNIT_ASSERT_EQUAL(sal_Int32(11), xIndexAccess->getCount());

    // Without the fix in place, this test would have failed with:
    // - Expected: 4
    // - Actual  : 1
    // Because the tables are pasted but not displayed

    CPPUNIT_ASSERT_EQUAL(4, getPages());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf130680)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf130680.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(23), xIndexAccess->getCount());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    rtl::Reference<SwDoc> xClpDoc(new SwDoc());
    xClpDoc->SetClipBoard(true);

    rtl::Reference<SwTransferable> xTransfer = new SwTransferable(*pWrtShell);
    // without the fix, it crashes
    xTransfer->Cut();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xIndexAccess->getCount());

    TransferableDataHelper aHelper(xTransfer);
    SwTransferable::Paste(*pWrtShell, aHelper);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    xClpDoc.clear();

    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xIndexAccess->getCount());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(23), xIndexAccess->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf131684)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf131684.docx");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    //Use selectAll 3 times in a row
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    rtl::Reference<SwTransferable> xTransfer = new SwTransferable(*pWrtShell);
    xTransfer->Cut();
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xIndexAccess->getCount());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    TransferableDataHelper aHelper(xTransfer);
    SwTransferable::Paste(*pWrtShell, aHelper);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    // without the fix, it crashes
    dispatchCommand(mxComponent, ".uno:Undo", {});
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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf132420)
{
    createSwDoc(DATA_DIRECTORY, "tdf132420.odt");

    CPPUNIT_ASSERT_EQUAL(12, getShapes());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    Scheduler::ProcessEventsToIdle();

    dispatchCommand(mxComponent, ".uno:Cut", {});
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(0, getShapes());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    //Without the fix in place, 1 frame and 1 image would be gone and getShapes would return 10
    CPPUNIT_ASSERT_EQUAL(12, getShapes());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf132744)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf132744.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    // disable change tracking to cut the table
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);

    CPPUNIT_ASSERT_MESSAGE("redlining should be off",
                           !pDoc->getIDocumentRedlineAccess().IsRedlineOn());

    CPPUNIT_ASSERT_EQUAL(1, getShapes());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    Scheduler::ProcessEventsToIdle();

    rtl::Reference<SwTransferable> xTransfer = new SwTransferable(*pWrtShell);
    xTransfer->Cut();
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(0, getShapes());

    TransferableDataHelper aHelper(xTransfer);
    SwTransferable::Paste(*pWrtShell, aHelper);
    Scheduler::ProcessEventsToIdle();

    //Without the fix in place, the image wouldn't be pasted
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf135014)
{
    createSwDoc();

    uno::Sequence<beans::PropertyValue> aArgs(
        comphelper::InitPropertySequence({ { "KeyModifier", uno::makeAny(sal_Int32(0)) } }));

    // Toggle Numbering List
    dispatchCommand(mxComponent, ".uno:DefaultBullet", aArgs);
    Scheduler::ProcessEventsToIdle();

    uno::Sequence<beans::PropertyValue> aArgs2(comphelper::InitPropertySequence(
        { { "Param", uno::makeAny(OUString("NewNumberingStyle")) },
          { "Family", uno::makeAny(static_cast<sal_Int16>(SfxStyleFamily::Pseudo)) } }));

    // New Style from selection
    dispatchCommand(mxComponent, ".uno:StyleNewByExample", aArgs2);
    Scheduler::ProcessEventsToIdle();

    // Without the fix in place, this test would have failed here
    reload("Office Open XML Text", "tdf135014.docx");

    xmlDocUniquePtr pXmlStyles = parseExport("word/styles.xml");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='NewNumberingStyle']/w:qFormat", 1);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf130629)
{
    createSwDoc();
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());

    uno::Sequence<beans::PropertyValue> aArgs(
        comphelper::InitPropertySequence({ { "KeyModifier", uno::makeAny(KEY_MOD1) } }));

    dispatchCommand(mxComponent, ".uno:BasicShapes.diamond", aArgs);
    Scheduler::ProcessEventsToIdle();

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
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(1, getShapes());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf145584)
{
    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    if (!pPDFium)
    {
        return;
    }
    SwDoc* const pDoc = createSwDoc();
    SwWrtShell* const pWrtSh = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtSh);

    pWrtSh->Insert("Hello World");

    // Select 'World'
    pWrtSh->Left(CRSR_SKIP_CHARS, /*bSelect=*/true, 5, /*bBasicCall=*/false);

    // Save as PDF.
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "Selection", uno::Any(true) } }));

    uno::Sequence<beans::PropertyValue> aDescriptor(comphelper::InitPropertySequence(
        { { "FilterName", uno::Any(OUString("writer_pdf_Export")) },
          { "FilterData", uno::Any(aFilterData) },
          { "URL", uno::Any(maTempFile.GetURL()) } }));

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, ".uno:ExportToPDF", aDescriptor);

    // Parse the export result.
    SvFileStream aFile(maTempFile.GetURL(), StreamMode::READ);
    SvMemoryStream aMemory;
    aMemory.WriteStream(aFile);
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = pPDFium->openDocument(aMemory.GetData(), aMemory.GetSize());
    CPPUNIT_ASSERT(pPdfDocument);
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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf116315)
{
    SwDoc* const pDoc = createSwDoc();
    SwWrtShell* const pWrtSh = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtSh);

    pWrtSh->Insert("This is a test");
    pWrtSh->Left(CRSR_SKIP_CHARS, /*bSelect=*/true, 4, /*bBasicCall=*/false);

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    for (sal_Int32 i = 0; i < 5; ++i)
    {
        pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_SHIFT | KEY_F3);
        Scheduler::ProcessEventsToIdle();

        // Title Case
        CPPUNIT_ASSERT_EQUAL(OUString("This is a Test"), getParagraph(1)->getString());

        pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_SHIFT | KEY_F3);
        Scheduler::ProcessEventsToIdle();

        // Sentence Case
        // Without the fix in place, this test would have failed with
        // - Expected: This is a Test
        // - Actual  : This is a TEST
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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf141613)
{
    SwDoc* const pDoc = createSwDoc();
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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf133358)
{
    SwDoc* const pDoc = createSwDoc();
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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf131771)
{
    createSwDoc();

    uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence(
        { { "Rows", uno::makeAny(sal_Int32(2)) }, { "Columns", uno::makeAny(sal_Int32(2)) } }));

    dispatchCommand(mxComponent, ".uno:InsertTable", aArgs);
    Scheduler::ProcessEventsToIdle();

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    uno::Reference<text::XTextTable> xTextTable(xIndexAccess->getByIndex(0), uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(OUString(""), getProperty<OUString>(xTextTable, "TableTemplateName"));
    uno::Reference<beans::XPropertySet> xTableProps(xTextTable, uno::UNO_QUERY_THROW);
    xTableProps->setPropertyValue("TableTemplateName", uno::makeAny(OUString("Default Style")));

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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf80663)
{
    createSwDoc();

    uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence(
        { { "Rows", uno::makeAny(sal_Int32(2)) }, { "Columns", uno::makeAny(sal_Int32(2)) } }));

    dispatchCommand(mxComponent, ".uno:InsertTable", aArgs);
    Scheduler::ProcessEventsToIdle();

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTextTable->getColumns()->getCount());

    dispatchCommand(mxComponent, ".uno:DeleteRows", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTextTable->getColumns()->getCount());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTextTable->getColumns()->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf130805)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf130805.odt");

    const SwFrameFormats& rFrmFormats = *pDoc->GetSpzFrameFormats();
    CPPUNIT_ASSERT(rFrmFormats.size() >= size_t(o3tl::make_unsigned(1)));
    auto pShape = rFrmFormats.front();
    CPPUNIT_ASSERT(pShape);

    SwTextBoxHelper::create(pShape, pShape->FindRealSdrObject());
    auto pTxBxFrm = SwTextBoxHelper::getOtherTextBoxFormat(getShape(1));
    CPPUNIT_ASSERT(pTxBxFrm);

    auto pTxAnch = pTxBxFrm->GetAnchor().GetContentAnchor();
    auto pShpAnch = pShape->GetAnchor().GetContentAnchor();
    CPPUNIT_ASSERT(pTxAnch);
    CPPUNIT_ASSERT(pShpAnch);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("The textbox got apart!", pTxAnch->nNode, pShpAnch->nNode);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf107893)
{
    //Open the sample doc
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf107893.odt");

    //Get the format of the shape
    const SwFrameFormats& rFrmFormats = *pDoc->GetSpzFrameFormats();
    CPPUNIT_ASSERT(rFrmFormats.size() >= size_t(o3tl::make_unsigned(1)));
    SwFrameFormat* pShape = rFrmFormats.front();
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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf121031)
{
    createSwDoc();

    uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence(
        { { "Rows", uno::makeAny(sal_Int32(3)) }, { "Columns", uno::makeAny(sal_Int32(3)) } }));

    dispatchCommand(mxComponent, ".uno:InsertTable", aArgs);
    Scheduler::ProcessEventsToIdle();

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    dispatchCommand(mxComponent, ".uno:DeleteTable", {});
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xIndexAccess->getCount());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    // Without the fix in place, the table would be hidden
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/tab", 1);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, TestTextBoxCrashAfterLineDel)
{
    // Open the desired file
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "txbx_crash.odt");

    // Get the format of the shape
    const SwFrameFormats& rFrmFormats = *pDoc->GetSpzFrameFormats();
    CPPUNIT_ASSERT(rFrmFormats.size() >= size_t(o3tl::make_unsigned(1)));
    SwFrameFormat* pShape = rFrmFormats.front();
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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf121546)
{
    createSwDoc(DATA_DIRECTORY, "tdf121546.odt");

    CPPUNIT_ASSERT_EQUAL(OUString("xxxxxxxxxxxxxxxxxxxx"), getParagraph(2)->getString());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    Scheduler::ProcessEventsToIdle();

    dispatchCommand(mxComponent, ".uno:Cut", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(1, getParagraphs());

    // Create a new document
    createSwDoc();

    dispatchCommand(mxComponent, ".uno:Paste", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(OUString("xxxxxxxxxxxxxxxxxxxx"), getParagraph(2)->getString());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    Scheduler::ProcessEventsToIdle();

    dispatchCommand(mxComponent, ".uno:Cut", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(1, getParagraphs());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(OUString("xxxxxxxxxxxxxxxxxxxx"), getParagraph(2)->getString());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());

    // Without the fix in place, this test would have crashed here
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_RETURN);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(2, getParagraphs());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf145621)
{
    createSwDoc(DATA_DIRECTORY, "tdf145621.odt");

    CPPUNIT_ASSERT_EQUAL(OUString("AAAAAA"), getParagraph(1)->getString());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    dispatchCommand(mxComponent, ".uno:Cut", {});

    CPPUNIT_ASSERT_EQUAL(OUString(""), getParagraph(1)->getString());

    // Without the fix in place, this test would have crashed
    dispatchCommand(mxComponent, ".uno:Paste", {});

    CPPUNIT_ASSERT_EQUAL(OUString("AAAAAA"), getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf134626)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf134626.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    CPPUNIT_ASSERT_EQUAL(OUString("Apple"), getParagraph(1)->getString());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    rtl::Reference<SwTransferable> xTransfer = new SwTransferable(*pWrtShell);
    xTransfer->Copy();
    Scheduler::ProcessEventsToIdle();
    TransferableDataHelper aHelper(xTransfer);

    // Create a new document
    pDoc = createSwDoc();
    pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    // Without the fix in place, this test would have crashed here
    for (sal_Int32 i = 0; i < 5; ++i)
    {
        SwTransferable::Paste(*pWrtShell, aHelper);
        Scheduler::ProcessEventsToIdle();

        CPPUNIT_ASSERT_EQUAL(OUString("Apple"), getParagraph(1)->getString());

        SwTransferable::Paste(*pWrtShell, aHelper);
        Scheduler::ProcessEventsToIdle();

        CPPUNIT_ASSERT_EQUAL(OUString("AppleApple"), getParagraph(1)->getString());

        dispatchCommand(mxComponent, ".uno:Undo", {});
        Scheduler::ProcessEventsToIdle();

        CPPUNIT_ASSERT_EQUAL(OUString("Apple"), getParagraph(1)->getString());

        dispatchCommand(mxComponent, ".uno:Undo", {});
        Scheduler::ProcessEventsToIdle();

        CPPUNIT_ASSERT_EQUAL(OUString(""), getParagraph(1)->getString());
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf139566)
{
    SwDoc* pDoc = createSwDoc();
    SwWrtShell* pWrtSh = pDoc->GetDocShell()->GetWrtShell();

    uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence(
        { { "Rows", uno::makeAny(sal_Int32(1)) }, { "Columns", uno::makeAny(sal_Int32(1)) } }));

    dispatchCommand(mxComponent, ".uno:InsertTable", aArgs);
    Scheduler::ProcessEventsToIdle();

    // Move the cursor outside the table
    pWrtSh->Down(/*bSelect=*/false);

    pWrtSh->Insert("Test");

    CPPUNIT_ASSERT_EQUAL(OUString("Test"), getParagraph(2)->getString());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    Scheduler::ProcessEventsToIdle();

    uno::Reference<frame::XFrames> xFrames = mxDesktop->getFrames();
    sal_Int32 nFrames = xFrames->getCount();

    // Create a second window so the first window looses focus
    dispatchCommand(mxComponent, ".uno:NewWindow", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(nFrames + 1, xFrames->getCount());

    dispatchCommand(mxComponent, ".uno:CloseWin", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(nFrames, xFrames->getCount());

    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xSelections(xModel->getCurrentSelection(),
                                                        uno::UNO_QUERY);

    // Without the fix in place, this test would have failed here
    CPPUNIT_ASSERT(xSelections.is());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf96067)
{
    createSwDoc();

    uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence(
        { { "Rows", uno::makeAny(sal_Int32(3)) }, { "Columns", uno::makeAny(sal_Int32(3)) } }));

    dispatchCommand(mxComponent, ".uno:InsertTable", aArgs);
    Scheduler::ProcessEventsToIdle();

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTextTable->getColumns()->getCount());

    dispatchCommand(mxComponent, ".uno:SelectTable", {});
    dispatchCommand(mxComponent, ".uno:InsertRowsBefore", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTextTable->getColumns()->getCount());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTextTable->getColumns()->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf87199)
{
    createSwDoc();

    uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence(
        { { "Rows", uno::makeAny(sal_Int32(2)) }, { "Columns", uno::makeAny(sal_Int32(1)) } }));

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
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTextTable->getColumns()->getCount());

    CPPUNIT_ASSERT(xCellA1->getString().endsWith("test2"));

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTextTable->getColumns()->getCount());

    xCellA1.set(xTextTable->getCellByName("A1"), uno::UNO_QUERY);

    CPPUNIT_ASSERT(xCellA1->getString().endsWith("test1"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf39828)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf39828.fodt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    pWrtShell->GotoNextTOXBase();

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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf132603)
{
    createSwDoc();
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());

    uno::Sequence<beans::PropertyValue> aPropertyValues
        = comphelper::InitPropertySequence({ { "Text", uno::makeAny(OUString("Comment")) } });

    dispatchCommand(mxComponent, ".uno:InsertAnnotation", aPropertyValues);
    Scheduler::ProcessEventsToIdle();

    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    Scheduler::ProcessEventsToIdle();

    // Without the fix in place, it would crash here
    dispatchCommand(mxComponent, ".uno:Copy", {});
    Scheduler::ProcessEventsToIdle();

    tools::JsonWriter aJsonWriter;
    pTextDoc->getPostIts(aJsonWriter);
    char* pChar = aJsonWriter.extractData();
    std::stringstream aStream(pChar);
    free(pChar);
    boost::property_tree::ptree aTree;
    boost::property_tree::read_json(aStream, aTree);
    for (const boost::property_tree::ptree::value_type& rValue : aTree.get_child("comments"))
    {
        const boost::property_tree::ptree& rComment = rValue.second;
        OString aText(rComment.get<std::string>("text").c_str());
        CPPUNIT_ASSERT_EQUAL(OString("Comment"), aText);
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf117601)
{
    createSwDoc();
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());

    uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence(
        { { "Rows", uno::makeAny(sal_Int32(5)) }, { "Columns", uno::makeAny(sal_Int32(3)) } }));

    dispatchCommand(mxComponent, ".uno:InsertTable", aArgs);
    Scheduler::ProcessEventsToIdle();

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
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTextTable->getColumns()->getCount());

    CPPUNIT_ASSERT(xCellB1->getString().endsWith("test2"));

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTextTable->getColumns()->getCount());

    CPPUNIT_ASSERT(xCellB1->getString().endsWith("test1"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf138130)
{
    createSwDoc(DATA_DIRECTORY, "tdf138130.docx");

    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    uno::Reference<drawing::XShape> xShape = getShape(1);

    awt::Point aPos = xShape->getPosition();

    //select shape and change the anchor
    dispatchCommand(mxComponent, ".uno:JumpToNextFrame", {});
    Scheduler::ProcessEventsToIdle();

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, ".uno:SetAnchorToPage", {});
    Scheduler::ProcessEventsToIdle();

    //position has changed
    CPPUNIT_ASSERT(aPos.X < xShape->getPosition().X);
    CPPUNIT_ASSERT(aPos.Y < xShape->getPosition().Y);

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(aPos.X, xShape->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(aPos.Y, xShape->getPosition().Y);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf136385)
{
    createSwDoc(DATA_DIRECTORY, "tdf136385.odt");

    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    uno::Reference<drawing::XShape> xShape = getShape(1);

    awt::Point aPos = xShape->getPosition();

    //select shape and change the anchor
    dispatchCommand(mxComponent, ".uno:JumpToNextFrame", {});
    Scheduler::ProcessEventsToIdle();

    dispatchCommand(mxComponent, ".uno:SetAnchorToPage", {});
    Scheduler::ProcessEventsToIdle();

    //position has changed
    CPPUNIT_ASSERT(aPos.X < xShape->getPosition().X);
    CPPUNIT_ASSERT(aPos.Y < xShape->getPosition().Y);

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    //Without the fix in place, this test would have failed with
    //- Expected: 2447
    //- Actual  : 446
    CPPUNIT_ASSERT_EQUAL(aPos.X, xShape->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(aPos.Y, xShape->getPosition().Y);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf145207)
{
    createSwDoc(DATA_DIRECTORY, "tdf145207.odt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());

    CPPUNIT_ASSERT_EQUAL(1, getPages());
    CPPUNIT_ASSERT_EQUAL(3, getShapes());

    //select one shape and use the TAB key to iterate over the different shapes
    dispatchCommand(mxComponent, ".uno:JumpToNextFrame", {});
    Scheduler::ProcessEventsToIdle();

    for (sal_Int32 i = 0; i < 10; ++i)
    {
        // Without the fix in place, this test would have crashed here
        pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
        Scheduler::ProcessEventsToIdle();
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf128782)
{
    createSwDoc(DATA_DIRECTORY, "tdf128782.odt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());

    CPPUNIT_ASSERT_EQUAL(2, getShapes());
    uno::Reference<drawing::XShape> xShape1 = getShape(1);
    uno::Reference<drawing::XShape> xShape2 = getShape(2);

    awt::Point aPos[2];
    aPos[0] = xShape1->getPosition();
    aPos[1] = xShape2->getPosition();

    //select shape 2 and move it down
    dispatchCommand(mxComponent, ".uno:JumpToNextFrame", {});
    dispatchCommand(mxComponent, ".uno:JumpToNextFrame", {});
    Scheduler::ProcessEventsToIdle();

    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_DOWN);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(aPos[0].X, xShape1->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(aPos[0].Y, xShape1->getPosition().Y);
    CPPUNIT_ASSERT_EQUAL(aPos[1].X, xShape2->getPosition().X);
    //Y position in shape 2 has changed
    CPPUNIT_ASSERT(aPos[1].Y < xShape2->getPosition().Y);

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(aPos[0].X, xShape1->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(aPos[0].Y, xShape1->getPosition().Y);
    CPPUNIT_ASSERT_EQUAL(aPos[1].X, xShape2->getPosition().X);
    // Shape2 has come back to the original position
    // without the fix in place, it would have failed
    CPPUNIT_ASSERT_EQUAL(aPos[1].Y, xShape2->getPosition().Y);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf135623)
{
    createSwDoc(DATA_DIRECTORY, "tdf135623.docx");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());

    CPPUNIT_ASSERT_EQUAL(2, getShapes());
    CPPUNIT_ASSERT_EQUAL(2, getPages());

    uno::Reference<drawing::XShape> xShape1 = getShape(1);
    uno::Reference<drawing::XShape> xShape2 = getShape(2);

    awt::Point aPos[2];
    aPos[0] = xShape1->getPosition();
    aPos[1] = xShape2->getPosition();

    //select shape 1 and move it down
    dispatchCommand(mxComponent, ".uno:JumpToNextFrame", {});
    Scheduler::ProcessEventsToIdle();

    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_DOWN);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(aPos[0].X, xShape1->getPosition().X);
    //Y position in shape 1 has changed
    CPPUNIT_ASSERT(aPos[0].Y < xShape1->getPosition().Y);
    CPPUNIT_ASSERT_EQUAL(aPos[1].X, xShape2->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(aPos[1].Y, xShape2->getPosition().Y);

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(aPos[0].X, xShape1->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(aPos[0].Y, xShape1->getPosition().Y);
    CPPUNIT_ASSERT_EQUAL(aPos[1].X, xShape2->getPosition().X);

    // Without the fix in place, this test would have failed here
    // - Expected: 1351
    // - Actual  : 2233
    CPPUNIT_ASSERT_EQUAL(aPos[1].Y, xShape2->getPosition().Y);

    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf133490)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf133490.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());

    CPPUNIT_ASSERT_EQUAL(1, getShapes());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    rtl::Reference<SwTransferable> xTransfer = new SwTransferable(*pWrtShell);
    xTransfer->Cut();
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(0, getShapes());

    TransferableDataHelper aHelper(xTransfer);
    SwTransferable::Paste(*pWrtShell, aHelper);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(1, getShapes());

    SwTransferable::Paste(*pWrtShell, aHelper);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(2, getShapes());

    uno::Reference<drawing::XShape> xShape1 = getShape(1);
    uno::Reference<drawing::XShape> xShape2 = getShape(2);

    awt::Point aPos[2];
    aPos[0] = xShape1->getPosition();
    aPos[1] = xShape2->getPosition();

    //select shape 2 and move it to the right
    dispatchCommand(mxComponent, ".uno:JumpToNextFrame", {});
    dispatchCommand(mxComponent, ".uno:JumpToNextFrame", {});
    Scheduler::ProcessEventsToIdle();

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
        Scheduler::ProcessEventsToIdle();

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
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(aPos[0].X, xShape1->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(aPos[0].Y, xShape1->getPosition().Y);
    // Shape 2 has come back to the original position
    CPPUNIT_ASSERT_EQUAL(aPos[1].X, xShape2->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(aPos[1].Y, xShape2->getPosition().Y);

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(1, getShapes());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(0, getShapes());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(1, getShapes());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf132637_protectTrackChanges)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf132637_protectTrackChanges.doc");

    // The password should only prevent turning off track changes, not open as read-only
    CPPUNIT_ASSERT(!pDoc->GetDocShell()->IsReadOnly());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf127652)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf127652.odt");
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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, AtPageTextBoxCrash)
{
    // Load sample file
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "AtPageTextBoxCrash.odt");

    // Get the format of the shape
    const SwFrameFormats& rFrmFormats = *pDoc->GetSpzFrameFormats();
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
        "AnchorType", uno::makeAny(text::TextContentAnchorType::TextContentAnchorType_AT_PAGE));

    // The page anchored objects must not have content anchor
    // unless this will lead to crash later, for example on
    // removing the paragraph where it is anchored to...
    CPPUNIT_ASSERT_EQUAL(RndStdIds::FLY_AT_PAGE, pTxBxFrm->GetAnchor().GetAnchorId());
    CPPUNIT_ASSERT(!pTxBxFrm->GetAnchor().GetContentAnchor());

    // Remove the paragraph where the textframe should be anchored
    // before. Now with the patch it must not crash...
    auto xPara = getParagraph(1);
    xPara->getText()->setString(OUString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf135661)
{
    createSwDoc(DATA_DIRECTORY, "tdf135661.odt");

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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf133477)
{
    if (getDefaultDeviceBitCount() < 24)
        return;
    createSwDoc(DATA_DIRECTORY, "tdf133477.fodt");

    // Save the shape to a BMP.
    uno::Reference<drawing::XGraphicExportFilter> xGraphicExporter
        = drawing::GraphicExportFilter::create(mxComponentContext);
    uno::Reference<lang::XComponent> xSourceDoc(getShape(1), uno::UNO_QUERY);
    xGraphicExporter->setSourceDocument(xSourceDoc);

    SvMemoryStream aStream;
    uno::Reference<io::XOutputStream> xOutputStream(new utl::OStreamWrapper(aStream));
    uno::Sequence<beans::PropertyValue> aDescriptor(
        comphelper::InitPropertySequence({ { "OutputStream", uno::makeAny(xOutputStream) },
                                           { "FilterName", uno::makeAny(OUString("BMP")) } }));
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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf137964)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf137964.odt");
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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf143244)
{
    createSwDoc(DATA_DIRECTORY, "tdf143244.odt");

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
        Scheduler::ProcessEventsToIdle();
    }

    xTextTable.set(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTextTable->getColumns()->getCount());

    for (sal_Int32 i = 0; i < 5; ++i)
    {
        dispatchCommand(mxComponent, ".uno:Redo", {});
        Scheduler::ProcessEventsToIdle();
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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf136715)
{
    createSwDoc(DATA_DIRECTORY, "tdf136715.odt");

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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf138897)
{
    createSwDoc(DATA_DIRECTORY, "tdf100018-1.odt");

    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:Cut", {});
    dispatchCommand(mxComponent, ".uno:Paste", {});
    // this was crashing
    dispatchCommand(mxComponent, ".uno:Undo", {});
    dispatchCommand(mxComponent, ".uno:Redo", {});
    dispatchCommand(mxComponent, ".uno:Undo", {});
    dispatchCommand(mxComponent, ".uno:Redo", {});
    Scheduler::ProcessEventsToIdle();
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf136740)
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
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(1, getParagraphs());
    CPPUNIT_ASSERT_EQUAL(OUString("FooFoo"), xParagraph->getString());

    // Without the fix in place, this would fail with
    //     equality assertion failed
    //     - Expected: <Any: (long) 2501>
    //     - Actual  : <Any: (long) 1251>
    // i.e., pasting RTF would reset the modified default tab stop distance to hardcoded default
    CPPUNIT_ASSERT_EQUAL(aNewCorrected, xTextDefaults->getPropertyValue("TabStopDistance"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf128106)
{
    SwWrtShell* pWrtShell
        = createSwDoc(DATA_DIRECTORY, "cross_reference_demo_bmk.odt")->GetDocShell()->GetWrtShell();

    utl::TempFile tempDir(nullptr, true);

    const auto aPropertyValues = comphelper::InitPropertySequence(
        { { "FileName", css::uno::Any(tempDir.GetURL() + "/test.odm") } });
    dispatchCommand(mxComponent, ".uno:NewGlobalDoc", aPropertyValues);

    // new document now!
    mxComponent.set(pWrtShell->GetDoc()->GetDocShell()->GetModel());
    CPPUNIT_ASSERT(mxComponent.is());

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
    CPPUNIT_ASSERT_EQUAL(OUString("Text"),
                         static_cast<SwGetRefField const*>(fields[0]->GetField())
                             ->GetExpandedTextOfReferencedTextNode(*pWrtShell->GetLayout()));
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

    tempDir.EnableKillingFile();
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf103612)
{
    SwDoc* const pGlobalDoc = createSwDoc(DATA_DIRECTORY, "DUMMY.odm");
    CPPUNIT_ASSERT_EQUAL(
        size_t(1),
        pGlobalDoc->getIDocumentLinksAdministration().GetLinkManager().GetLinks().size());
    pGlobalDoc->getIDocumentLinksAdministration().GetLinkManager().UpdateAllLinks(false, false,
                                                                                  nullptr);

    xmlDocUniquePtr pLayout = parseLayoutDump();

    assertXPath(pLayout, "/root/page[1]/body/section[1]/txt[1]/LineBreak[1]", "Line",
                "Text before section");
    // the inner section and its content was hidden
    assertXPath(pLayout, "/root/page[1]/body/section[2]/txt[1]/LineBreak[1]", "Line",
                "Text inside section before ToC");
    assertXPath(pLayout, "/root/page[1]/body/section[3]/txt[1]/LineBreak[1]", "Line",
                "Table of Contents");
    assertXPath(pLayout, "/root/page[1]/body/section[4]/txt[1]/LineBreak[1]", "Line",
                "First header*1");
    assertXPath(pLayout, "/root/page[1]/body/section[4]/txt[2]/LineBreak[1]", "Line",
                "Second header*1");
    assertXPath(pLayout, "/root/page[1]/body/section[5]/txt[2]/LineBreak[1]", "Line",
                "Text inside section after ToC");
    assertXPath(pLayout, "/root/page[1]/body/section[6]/txt[1]/LineBreak[1]", "Line",
                "Text after section");
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf97899)
{
    SwDoc* pDoc = createSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwPaM* pCursor = pDoc->GetEditShell()->GetCursor();
    IDocumentContentOperations& rIDCO(pDoc->getIDocumentContentOperations());

    // Create an Ordered List
    rIDCO.InsertString(*pCursor, "a");
    pWrtShell->SplitNode();
    rIDCO.InsertString(*pCursor, "b");
    pWrtShell->SplitNode();
    rIDCO.InsertString(*pCursor, "c");

    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:DefaultNumbering", {});

    // Save it as DOCX & load it again
    reload("Office Open XML Text", "tdf97899-tmp.docx");
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

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
