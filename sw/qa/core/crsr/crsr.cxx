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
#include <ndtxt.hxx>
#include <formatcontentcontrol.hxx>

/// Covers sw/source/core/crsr/ fixes.
class SwCoreCrsrTest : public SwModelTestBase
{
public:
    SwCoreCrsrTest()
        : SwModelTestBase(u"/sw/qa/core/crsr/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(SwCoreCrsrTest, testFindReplace)
{
    createSwDoc();

    // Given: a document with two "foo" in it, the second followed by a formatted soft hyphen.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    // Create a document which has 2 lines: first line has foo in it, second line has the same, but
    // followed by a formatted soft hyphen.
    xText->insertString(xText->getEnd(), u"foo xxx"_ustr, /*bAbsorb=*/false);
    xText->insertControlCharacter(xText->getEnd(), text::ControlCharacter::PARAGRAPH_BREAK,
                                  /*bAbsorb=*/false);
    xText->insertString(xText->getEnd(), u"foo xxx \xad after"_ustr, /*bAbsorb=*/false);
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursor> xViewCursor = xTextViewCursorSupplier->getViewCursor();
    xViewCursor->gotoEnd(/*bExpand=*/false);
    xViewCursor->goLeft(/*nCount=*/6, /*bExpand=*/false);
    xViewCursor->goLeft(/*nCount=*/1, /*bExpand=*/true);
    uno::Reference<beans::XPropertySet> xViewCursorProps(xViewCursor, uno::UNO_QUERY);
    xViewCursorProps->setPropertyValue(u"CharWeight"_ustr, uno::Any(awt::FontWeight::BOLD));
    xViewCursor->gotoStart(/*bExpand=*/false);

    // When: doing search & replace 3 times.
    uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence({
        { "SearchItem.SearchString", uno::Any(u"foo"_ustr) },
        { "SearchItem.ReplaceString", uno::Any(u"bar"_ustr) },
        { "SearchItem.Command", uno::Any(static_cast<sal_Int16>(SvxSearchCmd::REPLACE)) },
    }));
    // Find the first foo.
    dispatchCommand(mxComponent, u".uno:ExecuteSearch"_ustr, aArgs);
    // Replace the first foo.
    dispatchCommand(mxComponent, u".uno:ExecuteSearch"_ustr, aArgs);
    // Replace the second foo.
    dispatchCommand(mxComponent, u".uno:ExecuteSearch"_ustr, aArgs);

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
    CPPUNIT_ASSERT_EQUAL(u"bar"_ustr, aActualStart);
}

CPPUNIT_TEST_FIXTURE(SwCoreCrsrTest, testSelAllStartsWithTable)
{
    createSwDoc("sel-all-starts-with-table.odt");
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

CPPUNIT_TEST_FIXTURE(SwCoreCrsrTest, testContentControlLineBreak)
{
    // Given a document with a (rich text) content control:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertString(xCursor, u"test"_ustr, /*bAbsorb=*/false);
    xCursor->gotoStart(/*bExpand=*/false);
    xCursor->gotoEnd(/*bExpand=*/true);
    uno::Reference<text::XTextContent> xContentControl(
        xMSF->createInstance(u"com.sun.star.text.ContentControl"_ustr), uno::UNO_QUERY);
    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);

    // When pressing "enter" in the middle of that content control:
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->SttEndDoc(/*bStt=*/true);
    // Go after "t".
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 2, /*bBasicCall=*/false);
    dispatchCommand(mxComponent, u".uno:InsertPara"_ustr, {});

    // Then make sure that we only insert a line break, not a new paragraph:
    SwTextNode* pTextNode = pWrtShell->GetCursor()->GetMark()->GetNode().GetTextNode();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: t\nest
    // - Actual  : est
    // i.e. a new paragraph was inserted, which is not allowed for inline content controls.
    CPPUNIT_ASSERT_EQUAL(u"t\nest"_ustr, pTextNode->GetExpandText(pWrtShell->GetLayout()));
}

CPPUNIT_TEST_FIXTURE(SwCoreCrsrTest, testContentControlReadOnly)
{
    // Given a document with a checkbox content control:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertString(xCursor, u"☐"_ustr, /*bAbsorb=*/false);
    xCursor->gotoStart(/*bExpand=*/false);
    xCursor->gotoEnd(/*bExpand=*/true);
    uno::Reference<text::XTextContent> xContentControl(
        xMSF->createInstance(u"com.sun.star.text.ContentControl"_ustr), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    xContentControlProps->setPropertyValue(u"Checkbox"_ustr, uno::Any(true));
    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);

    // When entering the content control:
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->SttEndDoc(/*bStt=*/true);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);

    // Then make sure that the cursor is read-only:
    // Without the accompanying fix in place, this test would have failed, it was possible to type
    // into the checkbox content control, just to loose the typed content on the next click.
    CPPUNIT_ASSERT(pWrtShell->HasReadonlySel());
}

CPPUNIT_TEST_FIXTURE(SwCoreCrsrTest, testTdf135451)
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    // Insert narrow no-break space and move the cursor right before it
    pWrtShell->Insert(u"a" + OUStringChar(CHAR_NNBSP) + "b");
    pWrtShell->EndPara(/*bSelect=*/false);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    pWrtShell->GoPrevWord();
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: a
    // - Actual  : CHAR_NNBSP
    // i.e., the cursor did not move over the narrow no-break space (CHAR_NNBSP)
    CPPUNIT_ASSERT_EQUAL(u"a"_ustr, pWrtShell->GetSelText());
}

CPPUNIT_TEST_FIXTURE(SwCoreCrsrTest, testDropdownContentControl)
{
    // Given a document with a dropdown content control:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->InsertContentControl(SwContentControlType::DROP_DOWN_LIST);

    // When entering the content control:
    pWrtShell->SttEndDoc(/*bStt=*/true);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);

    // Then make sure that the cursor is read-only:
    // Without the accompanying fix in place, this test would have failed, it was possible to type
    // into the drop-down content control, providing content that is not one of the list items.
    CPPUNIT_ASSERT(pWrtShell->HasReadonlySel());
}

CPPUNIT_TEST_FIXTURE(SwCoreCrsrTest, testContentControlProtectedSection)
{
    // Given a document with a date content control in a protected section:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->InsertContentControl(SwContentControlType::DATE);
    pWrtShell->SelAll();
    OUString aSectionName = pWrtShell->GetUniqueSectionName();
    SwSectionData aSection(SectionType::Content, aSectionName);
    aSection.SetProtectFlag(true);
    pWrtShell->InsertSection(aSection);

    // When entering the content control:
    pWrtShell->SttEndDoc(/*bStt=*/true);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);

    // Then make sure that the cursor is read-only:
    // Without the accompanying fix in place, this test would have failed, it was not possible to
    // pick a date in a protected section (the new value was inserted, but the placeholder was not
    // removed).
    CPPUNIT_ASSERT(!pWrtShell->HasReadonlySel());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
