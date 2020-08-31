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
#include <com/sun/star/drawing/GraphicExportFilter.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <comphelper/propertysequence.hxx>
#include <boost/property_tree/json_parser.hpp>
#include <frameformats.hxx>
#include <textboxhelper.hxx>
#include <fmtanchr.hxx>
#include <o3tl/safeint.hxx>
#include <tools/json_writer.hxx>
#include <unotools/streamwrap.hxx>

#include <wrtsh.hxx>
#include <unotxdoc.hxx>
#include <docsh.hxx>

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
    dispatchCommand(mxComponent, ".uno:Cut", {});
    CPPUNIT_ASSERT_EQUAL(3, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    dispatchCommand(mxComponent, ".uno:Paste", {});
    CPPUNIT_ASSERT_EQUAL(8, getShapes());
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(3, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    // without the fix, it crashes
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(8, getShapes());
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
    dispatchCommand(mxComponent, ".uno:Cut", {});
    CPPUNIT_ASSERT_EQUAL(0, getShapes());
    dispatchCommand(mxComponent, ".uno:Paste", {});
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
    dispatchCommand(mxComponent, ".uno:Cut", {});
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(0, getShapes());

    dispatchCommand(mxComponent, ".uno:Paste", {});
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(4, getShapes());

    // Without the fix in place, it would have crashed here
    dispatchCommand(mxComponent, ".uno:Paste", {});
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

    dispatchCommand(mxComponent, ".uno:Paste", {});
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(4, getShapes());

    dispatchCommand(mxComponent, ".uno:Paste", {});
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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf126626)
{
    load(DATA_DIRECTORY, "tdf126626.docx");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    CPPUNIT_ASSERT_EQUAL(2, getShapes());
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:Copy", {});
    CPPUNIT_ASSERT_EQUAL(2, getShapes());
    dispatchCommand(mxComponent, ".uno:Paste", {});
    CPPUNIT_ASSERT_EQUAL(2, getShapes());
    dispatchCommand(mxComponent, ".uno:Paste", {});
    CPPUNIT_ASSERT_EQUAL(4, getShapes());
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(2, getShapes());

    // without the fix, it crashes
    dispatchCommand(mxComponent, ".uno:Paste", {});
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
    dispatchCommand(mxComponent, ".uno:Copy", {});
    dispatchCommand(mxComponent, ".uno:GoToEndOfDoc", {});

    for (sal_Int32 i = 0; i < 10; ++i)
    {
        dispatchCommand(mxComponent, ".uno:Paste", {});
        Scheduler::ProcessEventsToIdle();
    }

    //without the fix in place, this test would fail with:
    //- Expected: 1
    //- Actual  : 70

    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest3, testTdf128739)
{
    load(DATA_DIRECTORY, "tdf128739.docx");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    CPPUNIT_ASSERT_EQUAL(OUString("Fehler: Verweis nicht gefunden"), getParagraph(1)->getString());
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:Cut", {});
    CPPUNIT_ASSERT_EQUAL(OUString(""), getParagraph(1)->getString());
    dispatchCommand(mxComponent, ".uno:Paste", {});
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
    dispatchCommand(mxComponent, ".uno:Copy", {});
    CPPUNIT_ASSERT_EQUAL(22, getPages());
    dispatchCommand(mxComponent, ".uno:Paste", {});
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

    dispatchCommand(mxComponent, ".uno:Copy", {});
    dispatchCommand(mxComponent, ".uno:GoToEndOfPage", {});
    Scheduler::ProcessEventsToIdle();

    dispatchCommand(mxComponent, ".uno:Paste", {});
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

    //Without the fix in place, it would have crashed here
    dispatchCommand(mxComponent, ".uno:Cut", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xIndexAccess->getCount());

    dispatchCommand(mxComponent, ".uno:Paste", {});
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

    dispatchCommand(mxComponent, ".uno:Copy", {});
    dispatchCommand(mxComponent, ".uno:Paste", {});

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
    dispatchCommand(mxComponent, ".uno:Copy", {});

    //Position the mouse cursor (caret) after "ABC" below the blue image
    dispatchCommand(mxComponent, ".uno:GoRight", {});
    dispatchCommand(mxComponent, ".uno:Paste", {});

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
    dispatchCommand(mxComponent, ".uno:Copy", {});

    //Position the mouse cursor (caret) after "ABC" below the blue image
    dispatchCommand(mxComponent, ".uno:GoRight", {});
    dispatchCommand(mxComponent, ".uno:Paste", {});

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
    // without the fix in place, it would crash here
    dispatchCommand(mxComponent, ".uno:Cut", {});
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
    dispatchCommand(mxComponent, ".uno:Cut", {});
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    dispatchCommand(mxComponent, ".uno:Paste", {});
    dispatchCommand(mxComponent, ".uno:Paste", {});

    // Without fix in place, this test would have failed with:
    //- Expected: 2
    //- Actual  : 4
    CPPUNIT_ASSERT_EQUAL(2, getPages());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    dispatchCommand(mxComponent, ".uno:Undo", {});
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
    dispatchCommand(mxComponent, ".uno:Copy", {});
    Scheduler::ProcessEventsToIdle();

    dispatchCommand(mxComponent, ".uno:GoDown", {});

    for (sal_Int32 i = 0; i < 10; ++i)
    {
        dispatchCommand(mxComponent, ".uno:Paste", {});
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

    // without the fix, it crashes
    dispatchCommand(mxComponent, ".uno:Cut", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xIndexAccess->getCount());
    dispatchCommand(mxComponent, ".uno:Paste", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
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

    dispatchCommand(mxComponent, ".uno:Cut", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(0, getShapes());

    dispatchCommand(mxComponent, ".uno:Paste", {});
    Scheduler::ProcessEventsToIdle();

    //Without the fix in place, the image wouldn't be pasted
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
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
    //CPPUNIT_ASSERT_EQUAL_MESSAGE("", xShp->getPosition().Y, xShp2->getPosition().Y);
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
    CPPUNIT_ASSERT(aPos[1].Y != xShape2->getPosition().Y);

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(aPos[0].X, xShape1->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(aPos[0].Y, xShape1->getPosition().Y);
    CPPUNIT_ASSERT_EQUAL(aPos[1].X, xShape2->getPosition().X);
    // Shape2 has come back to the original position
    // without the fix in place, it would have failed
    CPPUNIT_ASSERT_EQUAL(aPos[1].Y, xShape2->getPosition().Y);
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
    ReadGraphic(aStream, aGraphic);
    BitmapEx aBitmap = aGraphic.GetBitmapEx();
    CPPUNIT_ASSERT_EQUAL(Color(0, 102, 204), aBitmap.GetPixelColor(0, 0));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
