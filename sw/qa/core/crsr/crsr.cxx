/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/text/ControlCharacter.hpp>
#include <com/sun/star/view/XLineCursor.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/text/XTextViewCursor.hpp>

#include <comphelper/propertysequence.hxx>
#include <svl/srchitem.hxx>
#include <vcl/scheduler.hxx>

#include <docsh.hxx>
#include <unotxdoc.hxx>
#include <wrtsh.hxx>

constexpr OUStringLiteral DATA_DIRECTORY = u"/sw/qa/core/crsr/data/";

/// Covers sw/source/core/crsr/ fixes.
class SwCoreCrsrTest : public SwModelTestBase
{
};

CPPUNIT_TEST_FIXTURE(SwCoreCrsrTest, testFindReplace)
{
    loadURL("private:factory/swriter", nullptr);

    // Given: a document with two "foo" in it, the second followed by a formatted soft hyphen.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    // Create a document which has 2 lines: first line has foo in it, second line has the same, but
    // followed by a formatted soft hyphen.
    xText->insertString(xText->getEnd(), "foo xxx", /*bAbsorb=*/false);
    xText->insertControlCharacter(xText->getEnd(), text::ControlCharacter::PARAGRAPH_BREAK,
                                  /*bAbsorb=*/false);
    xText->insertString(xText->getEnd(), OUString(u"foo xxx \xad after"), /*bAbsorb=*/false);
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursor> xViewCursor = xTextViewCursorSupplier->getViewCursor();
    xViewCursor->gotoEnd(/*bExpand=*/false);
    xViewCursor->goLeft(/*nCount=*/6, /*bExpand=*/false);
    xViewCursor->goLeft(/*nCount=*/1, /*bExpand=*/true);
    uno::Reference<beans::XPropertySet> xViewCursorProps(xViewCursor, uno::UNO_QUERY);
    xViewCursorProps->setPropertyValue("CharWeight", uno::makeAny(awt::FontWeight::BOLD));
    xViewCursor->gotoStart(/*bExpand=*/false);

    // When: doing search & replace 3 times.
    uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence({
        { "SearchItem.SearchString", uno::makeAny(OUString("foo")) },
        { "SearchItem.ReplaceString", uno::makeAny(OUString("bar")) },
        { "SearchItem.Command", uno::makeAny(static_cast<sal_Int16>(SvxSearchCmd::REPLACE)) },
    }));
    // Find the first foo.
    dispatchCommand(mxComponent, ".uno:ExecuteSearch", aArgs);
    // Replace the first foo.
    dispatchCommand(mxComponent, ".uno:ExecuteSearch", aArgs);
    // Replace the second foo.
    dispatchCommand(mxComponent, ".uno:ExecuteSearch", aArgs);

    // Then: the second "foo" should be replaced as well.
    xViewCursor->gotoEnd(/*bExpand=*/false);
    uno::Reference<view::XLineCursor> xLineCursor(xViewCursor, uno::UNO_QUERY);
    xLineCursor->gotoStartOfLine(/*bExpand=*/true);
    OUString aActual = xViewCursor->getString();
    CPPUNIT_ASSERT_GREATEREQUAL(static_cast<sal_Int32>(3), aActual.getLength());
    OUString aActualStart = aActual.copy(0, 3);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: bar
    // - Actual  : foo
    // i.e. the foo on the second line was not replaced.
    CPPUNIT_ASSERT_EQUAL(OUString("bar"), aActualStart);
}

CPPUNIT_TEST_FIXTURE(SwCoreCrsrTest, testSelAllStartsWithTable)
{
    load(DATA_DIRECTORY, "sel-all-starts-with-table.odt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwDocShell* pDocShell = pTextDoc->GetDocShell();
    SwDoc* pDoc = pDocShell->GetDoc();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pDoc->GetTableFrameFormatCount(/*bUsed=*/true));

    pWrtShell->SelAll();
    pWrtShell->SelAll();
    Scheduler::ProcessEventsToIdle();
    pWrtShell->DelLeft();

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 1
    // i.e. the table selection was lost and the table was not deleted.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), pDoc->GetTableFrameFormatCount(/*bUsed=*/true));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
