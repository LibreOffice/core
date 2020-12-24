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
#include <vcl/scheduler.hxx>
#include <vcl/TypeSerializer.hxx>
#include <com/sun/star/drawing/GraphicExportFilter.hpp>
#include <IDocumentDrawModelAccess.hxx>
#include <com/sun/star/text/TextContentAnchorType.hpp>
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
#include <unotools/streamwrap.hxx>

#include <wrtsh.hxx>
#include <unotxdoc.hxx>
#include <docsh.hxx>
#include <drawdoc.hxx>
#include <dcontact.hxx>
#include <svx/svdpage.hxx>
#include <ndtxt.hxx>
#include <IDocumentRedlineAccess.hxx>

namespace
{
char const DATA_DIRECTORY[] = "/sw/qa/extras/uiwriter/data3/";
} // namespace

/// Third set of tests asserting the behavior of Writer user interface shells.
class SwUiWriterTest3 : public SwModelTestBase
{
};

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf129382)
{
    load(DATA_DIRECTORY, "tdf129382.docx");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    CPPUNIT_ASSERT_EQUAL(8, getShapes());
    CPPUNIT_ASSERT_EQUAL(2, getPages());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    rtl::Reference<SwTransferable> xTransfer = new SwTransferable(*pWrtShell);
    xTransfer->Cut();

    CPPUNIT_ASSERT_EQUAL(3, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    TransferableDataHelper aHelper(xTransfer.get());
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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf134227)
{
    load(DATA_DIRECTORY, "tdf134227.docx");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    CPPUNIT_ASSERT_EQUAL(4, getShapes());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    // Without the fix in place, it would have crashed here
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    rtl::Reference<SwTransferable> xTransfer = new SwTransferable(*pWrtShell);
    xTransfer->Cut();

    CPPUNIT_ASSERT_EQUAL(0, getShapes());

    TransferableDataHelper aHelper(xTransfer.get());
    SwTransferable::Paste(*pWrtShell, aHelper);

    CPPUNIT_ASSERT_EQUAL(4, getShapes());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(0, getShapes());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(4, getShapes());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf135412)
{
    load(DATA_DIRECTORY, "tdf135412.docx");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    CPPUNIT_ASSERT_EQUAL(4, getShapes());
    uno::Reference<text::XTextRange> xShape(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("X"), xShape->getString());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    rtl::Reference<SwTransferable> xTransfer = new SwTransferable(*pWrtShell);
    xTransfer->Cut();

    CPPUNIT_ASSERT_EQUAL(0, getShapes());

    TransferableDataHelper aHelper(xTransfer.get());
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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf132911)
{
    load(DATA_DIRECTORY, "tdf132911.odt");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(4, getShapes());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    rtl::Reference<SwTransferable> xTransfer = new SwTransferable(*pWrtShell);
    xTransfer->Cut();
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(0, getShapes());

    TransferableDataHelper aHelper(xTransfer.get());
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

    //FIXME: tdf#135247
    //dispatchCommand(mxComponent, ".uno:Undo", {});
    //Scheduler::ProcessEventsToIdle();
    //CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    //CPPUNIT_ASSERT_EQUAL(4, getShapes());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf61154)
{
    load(DATA_DIRECTORY, "tdf61154.fodt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();

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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf100691)
{
    load(DATA_DIRECTORY, "tdf100691.fodt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();

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
    load(DATA_DIRECTORY, "tdf134404.odt");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

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
    load(DATA_DIRECTORY, "tdf112342.docx");

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
    load(DATA_DIRECTORY, "tdf132321.odt");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

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
    load(DATA_DIRECTORY, "tdf135056.odt");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    CPPUNIT_ASSERT_EQUAL(sal_uInt16(1), pWrtShell->GetTOXCount());

    const SwTOXBase* pTOX = pWrtShell->GetTOX(0);
    CPPUNIT_ASSERT(pTOX);

    //Without the fix in place, it would have hung here
    pWrtShell->DeleteTOX(*pTOX, true);

    CPPUNIT_ASSERT_EQUAL(sal_uInt16(0), pWrtShell->GetTOXCount());

    dispatchCommand(mxComponent, ".uno:Undo", {});

    CPPUNIT_ASSERT_EQUAL(sal_uInt16(1), pWrtShell->GetTOXCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf126626)
{
    load(DATA_DIRECTORY, "tdf126626.docx");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    CPPUNIT_ASSERT_EQUAL(2, getShapes());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    rtl::Reference<SwTransferable> xTransfer = new SwTransferable(*pWrtShell);
    xTransfer->Copy();

    CPPUNIT_ASSERT_EQUAL(2, getShapes());

    TransferableDataHelper aHelper(xTransfer.get());
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
    load(DATA_DIRECTORY, "tdf133967.odt");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
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
    load(DATA_DIRECTORY, "tdf132187.odt");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    CPPUNIT_ASSERT_EQUAL(1, getPages());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    rtl::Reference<SwTransferable> xTransfer = new SwTransferable(*pWrtShell);
    xTransfer->Copy();

    dispatchCommand(mxComponent, ".uno:GoToEndOfDoc", {});

    TransferableDataHelper aHelper(xTransfer.get());
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
    load(DATA_DIRECTORY, "tdf135733.odt");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    CPPUNIT_ASSERT_EQUAL(2, getShapes());

    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();

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

    TransferableDataHelper aHelper(xTransfer.get());
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
    load(DATA_DIRECTORY, "tdf128739.docx");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    CPPUNIT_ASSERT_EQUAL(OUString("Fehler: Verweis nicht gefunden"), getParagraph(1)->getString());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    rtl::Reference<SwTransferable> xTransfer = new SwTransferable(*pWrtShell);
    xTransfer->Cut();

    CPPUNIT_ASSERT_EQUAL(OUString(""), getParagraph(1)->getString());

    TransferableDataHelper aHelper(xTransfer.get());
    SwTransferable::Paste(*pWrtShell, aHelper);
    CPPUNIT_ASSERT_EQUAL(OUString("Fehler: Verweis nicht gefunden"), getParagraph(1)->getString());

    // without the fix, it crashes
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(OUString(""), getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf124722)
{
    load(DATA_DIRECTORY, "tdf124722.rtf");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    CPPUNIT_ASSERT_EQUAL(22, getPages());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    rtl::Reference<SwTransferable> xTransfer = new SwTransferable(*pWrtShell);
    xTransfer->Copy();

    CPPUNIT_ASSERT_EQUAL(22, getPages());

    TransferableDataHelper aHelper(xTransfer.get());
    SwTransferable::Paste(*pWrtShell, aHelper);
    CPPUNIT_ASSERT_EQUAL(43, getPages());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(22, getPages());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf125261)
{
    load(DATA_DIRECTORY, "tdf125261.odt");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

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
    load(DATA_DIRECTORY, "tdf133990.odt");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

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
    load(DATA_DIRECTORY, "tdf126504.odt");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    //Use selectAll 2 times in a row
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    rtl::Reference<SwTransferable> xTransfer = new SwTransferable(*pWrtShell);
    xTransfer->Copy();

    dispatchCommand(mxComponent, ".uno:GoToEndOfPage", {});
    Scheduler::ProcessEventsToIdle();

    TransferableDataHelper aHelper(xTransfer.get());
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
    load(DATA_DIRECTORY, "tdf133982.docx");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());

    //Use selectAll 3 times in a row
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    rtl::Reference<SwTransferable> xTransfer = new SwTransferable(*pWrtShell);
    //Without the fix in place, it would have crashed here
    xTransfer->Cut();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xIndexAccess->getCount());

    TransferableDataHelper aHelper(xTransfer.get());
    SwTransferable::Paste(*pWrtShell, aHelper);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf134253)
{
    load(DATA_DIRECTORY, "tdf134253.odt");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(6, getPages());

    //Use selectAll 3 times in a row
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    rtl::Reference<SwTransferable> xTransfer = new SwTransferable(*pWrtShell);
    xTransfer->Copy();
    TransferableDataHelper aHelper(xTransfer.get());
    SwTransferable::Paste(*pWrtShell, aHelper);

    //Without the fix in place, it would have crashed here
    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(6, getPages());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf76636)
{
    load(DATA_DIRECTORY, "tdf76636.doc");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

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
    load(DATA_DIRECTORY, "tdf76636.doc");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf132725)
{
    load(DATA_DIRECTORY, "tdf132725.odt");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

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
    load(DATA_DIRECTORY, "tdf126340.odt");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    dispatchCommand(mxComponent, ".uno:GoDown", {});
    // without the fix, it crashes
    dispatchCommand(mxComponent, ".uno:Delete", {});
    CPPUNIT_ASSERT_EQUAL(OUString("foo"), getParagraph(1)->getString());
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(OUString("foo"), getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf124397)
{
    load(DATA_DIRECTORY, "tdf124397.docx");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf107975)
{
    // This test also covers tdf#117185 tdf#110442

    load(DATA_DIRECTORY, "tdf107975.odt");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    uno::Reference<text::XTextGraphicObjectsSupplier> xTextGraphicObjectsSupplier(mxComponent,
                                                                                  uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(
        xTextGraphicObjectsSupplier->getGraphicObjects(), uno::UNO_QUERY);

    uno::Reference<drawing::XShape> xShape(xIndexAccess->getByIndex(0), uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_CHARACTER,
                         getProperty<text::TextContentAnchorType>(xShape, "AnchorType"));

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    rtl::Reference<SwTransferable> xTransfer = new SwTransferable(*pWrtShell);
    xTransfer->Copy();

    //Position the mouse cursor (caret) after "ABC" below the blue image
    dispatchCommand(mxComponent, ".uno:GoRight", {});
    {
        TransferableDataHelper aHelper(xTransfer.get());
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
        TransferableDataHelper aHelper(xTransfer.get());
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
    load(DATA_DIRECTORY, "tdf134021.docx");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf130746)
{
    load(DATA_DIRECTORY, "tdf130746.odt");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

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
    load(DATA_DIRECTORY, "tdf129805.docx");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    CPPUNIT_ASSERT_EQUAL(OUString("x"), getParagraph(1)->getString());
    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    rtl::Reference<SwTransferable> xTransfer = new SwTransferable(*pWrtShell);
    // without the fix in place, it would crash here
    xTransfer->Cut();
    CPPUNIT_ASSERT_EQUAL(OUString(""), getParagraph(1)->getString());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(OUString("x"), getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf130685)
{
    load(DATA_DIRECTORY, "tdf130685.odt");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    CPPUNIT_ASSERT_EQUAL(2, getPages());
    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    rtl::Reference<SwTransferable> xTransfer = new SwTransferable(*pWrtShell);
    xTransfer->Cut();
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(1, getPages());

    TransferableDataHelper aHelper(xTransfer.get());
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
    load(DATA_DIRECTORY, "tdf132944.odt");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

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
    load(DATA_DIRECTORY, "tdf104649.docx");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

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
    load(DATA_DIRECTORY, "tdf134931.odt");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    rtl::Reference<SwTransferable> xTransfer = new SwTransferable(*pWrtShell);
    xTransfer->Copy();
    Scheduler::ProcessEventsToIdle();

    dispatchCommand(mxComponent, ".uno:GoDown", {});

    TransferableDataHelper aHelper(xTransfer.get());
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
    load(DATA_DIRECTORY, "tdf130680.odt");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(23), xIndexAccess->getCount());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    rtl::Reference<SwDoc> xClpDoc(new SwDoc());
    xClpDoc->SetClipBoard(true);

    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    rtl::Reference<SwTransferable> xTransfer = new SwTransferable(*pWrtShell);
    // without the fix, it crashes
    xTransfer->Cut();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xIndexAccess->getCount());

    TransferableDataHelper aHelper(xTransfer.get());
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
    load(DATA_DIRECTORY, "tdf131684.docx");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    //Use selectAll 3 times in a row
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    rtl::Reference<SwTransferable> xTransfer = new SwTransferable(*pWrtShell);
    xTransfer->Cut();
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xIndexAccess->getCount());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());

    TransferableDataHelper aHelper(xTransfer.get());
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
    load(DATA_DIRECTORY, "tdf132420.odt");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

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
    load(DATA_DIRECTORY, "tdf132744.odt");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    CPPUNIT_ASSERT_EQUAL(1, getShapes());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    Scheduler::ProcessEventsToIdle();

    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    rtl::Reference<SwTransferable> xTransfer = new SwTransferable(*pWrtShell);
    xTransfer->Cut();
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(0, getShapes());

    TransferableDataHelper aHelper(xTransfer.get());
    SwTransferable::Paste(*pWrtShell, aHelper);
    Scheduler::ProcessEventsToIdle();

    //Without the fix in place, the image wouldn't be pasted
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf133358)
{
    mxComponent = loadFromDesktop("private:factory/swriter", "com.sun.star.text.TextDocument");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    SwWrtShell* pWrtSh = pTextDoc->GetDocShell()->GetWrtShell();
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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf80663)
{
    mxComponent = loadFromDesktop("private:factory/swriter", "com.sun.star.text.TextDocument");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

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
    load(DATA_DIRECTORY, "tdf130805.odt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    SwWrtShell* pWrtSh = pTextDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtSh);

    const SwFrameFormats& rFrmFormats = *pWrtSh->GetDoc()->GetSpzFrameFormats();
    CPPUNIT_ASSERT(rFrmFormats.size() >= size_t(o3tl::make_unsigned(1)));
    auto pShape = rFrmFormats.front();
    CPPUNIT_ASSERT(pShape);

    SwTextBoxHelper::create(pShape);
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
    load(DATA_DIRECTORY, "tdf107893.odt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    //Get the Writer shell
    SwWrtShell* pWrtSh = pTextDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtSh);

    //Get the format of the shape
    const SwFrameFormats& rFrmFormats = *pWrtSh->GetDoc()->GetSpzFrameFormats();
    CPPUNIT_ASSERT(rFrmFormats.size() >= size_t(o3tl::make_unsigned(1)));
    SwFrameFormat* pShape = rFrmFormats.front();
    CPPUNIT_ASSERT(pShape);

    //Add a textbox
    SwTextBoxHelper::create(pShape);
    SwFrameFormat* pTxBxFrm = SwTextBoxHelper::getOtherTextBoxFormat(getShape(1));
    CPPUNIT_ASSERT(pTxBxFrm);

    //Remove the textbox using Undo
    dispatchCommand(mxComponent, ".uno:Undo", {});

    //Add again
    SwTextBoxHelper::create(pShape);
    pTxBxFrm = SwTextBoxHelper::getOtherTextBoxFormat(getShape(1));

    //This was nullptr because of unsuccessful re-adding
    CPPUNIT_ASSERT_MESSAGE("Textbox cannot be readd after Undo!", pTxBxFrm);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf121031)
{
    mxComponent = loadFromDesktop("private:factory/swriter", "com.sun.star.text.TextDocument");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

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
    load(DATA_DIRECTORY, "txbx_crash.odt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    // Get the Writer shell
    SwWrtShell* pWrtSh = pTextDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtSh);

    // Get the format of the shape
    const SwFrameFormats& rFrmFormats = *pWrtSh->GetDoc()->GetSpzFrameFormats();
    CPPUNIT_ASSERT(rFrmFormats.size() >= size_t(o3tl::make_unsigned(1)));
    SwFrameFormat* pShape = rFrmFormats.front();
    CPPUNIT_ASSERT(pShape);

    // Add a textbox
    SwTextBoxHelper::create(pShape);
    SwFrameFormat* pTxBxFrm = SwTextBoxHelper::getOtherTextBoxFormat(getShape(1));
    CPPUNIT_ASSERT(pTxBxFrm);

    // remove the last paragraph
    auto xCursor = getParagraph(1)->getText()->createTextCursor();
    xCursor->gotoEnd(false);
    xCursor->goLeft(3, true);

    // This caused crash before, now it should pass with the patch.
    xCursor->setString(OUString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf134626)
{
    load(DATA_DIRECTORY, "tdf134626.odt");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    CPPUNIT_ASSERT_EQUAL(OUString("Apple"), getParagraph(1)->getString());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    rtl::Reference<SwTransferable> xTransfer = new SwTransferable(*pWrtShell);
    xTransfer->Copy();
    Scheduler::ProcessEventsToIdle();
    TransferableDataHelper aHelper(xTransfer.get());

    // Create a new document
    mxComponent = loadFromDesktop("private:factory/swriter", "com.sun.star.text.TextDocument");

    pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();

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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf96067)
{
    mxComponent = loadFromDesktop("private:factory/swriter", "com.sun.star.text.TextDocument");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

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
    mxComponent = loadFromDesktop("private:factory/swriter", "com.sun.star.text.TextDocument");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf132603)
{
    mxComponent = loadFromDesktop("private:factory/swriter", "com.sun.star.text.TextDocument");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

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
    mxComponent = loadFromDesktop("private:factory/swriter", "com.sun.star.text.TextDocument");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

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
    load(DATA_DIRECTORY, "tdf138130.docx");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

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
    load(DATA_DIRECTORY, "tdf136385.odt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf128782)
{
    load(DATA_DIRECTORY, "tdf128782.odt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

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
    load(DATA_DIRECTORY, "tdf135623.docx");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

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
    load(DATA_DIRECTORY, "tdf133490.odt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    CPPUNIT_ASSERT_EQUAL(1, getShapes());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});

    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    rtl::Reference<SwTransferable> xTransfer = new SwTransferable(*pWrtShell);
    xTransfer->Cut();
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(0, getShapes());

    TransferableDataHelper aHelper(xTransfer.get());
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
    load(DATA_DIRECTORY, "tdf132637_protectTrackChanges.doc");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    // The password should only prevent turning off track changes, not open as read-only
    CPPUNIT_ASSERT(!pTextDoc->GetDocShell()->IsReadOnly());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf127652)
{
    load(DATA_DIRECTORY, "tdf127652.odt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    SwCursorShell* pShell = pTextDoc->GetDocShell()->GetWrtShell();

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
    sal_uInt16 currentPage = pShell->GetPageNumSeqNonEmpty();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("We are on the wrong page!", assertPage, currentPage);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, AtPageTextBoxCrash)
{
    // Load sample file
    load(DATA_DIRECTORY, "AtPageTextBoxCrash.odt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    // Get the Writer-Shell for later use
    SwWrtShell* pWrtSh = pTextDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtSh);

    // Get the format of the shape
    const SwFrameFormats& rFrmFormats = *pWrtSh->GetDoc()->GetSpzFrameFormats();
    CPPUNIT_ASSERT(rFrmFormats.size() >= size_t(o3tl::make_unsigned(1)));
    auto pShape = rFrmFormats.front();
    CPPUNIT_ASSERT(pShape);

    // Add a textbox to the shape
    SwTextBoxHelper::create(pShape);
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
    load(DATA_DIRECTORY, "tdf135661.odt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

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
    load(DATA_DIRECTORY, "tdf133477.fodt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

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
    Graphic aGraphic;
    TypeSerializer aSerializer(aStream);
    aSerializer.readGraphic(aGraphic);

    BitmapEx aBitmap = aGraphic.GetBitmapEx();
    CPPUNIT_ASSERT_EQUAL(Color(0, 102, 204), aBitmap.GetPixelColor(0, 0));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf137964)
{
    load(DATA_DIRECTORY, "tdf137964.odt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    uno::Reference<drawing::XShape> xShape(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3579), xShape->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4090), xShape->getPosition().Y);

    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf138897)
{
    load(DATA_DIRECTORY, "tdf100018-1.odt");

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

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
