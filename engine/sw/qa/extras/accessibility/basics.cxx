/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleTextType.hpp>
#include <com/sun/star/accessibility/XAccessibleEditableText.hpp>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <com/sun/star/awt/Key.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/text/XTextRangeCompare.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <COKit/COKit.hxx>
#include <vcl/scheduler.hxx>

#include <comphelper/kit.hxx>
#include <test/a11y/accessibletestbase.hxx>
#include <test/a11y/AccessibilityTools.hxx>

using namespace css;
using namespace ::cpo;

CPPUNIT_TEST_FIXTURE(test::AccessibleTestBase, TestBasicStructure)
{
    loadFromSrc(u"/sw/qa/python/testdocuments/xtextcontent.odt"_ustr);
    auto xContext = getDocumentAccessibleContext();
    CPPUNIT_ASSERT(xContext.is());

    dumpA11YTree(xContext);
    CPPUNIT_ASSERT_EQUAL(
        u"<PARAGRAPH>String1</PARAGRAPH><PARAGRAPH/><PARAGRAPH/><PARAGRAPH/>"
        "<TABLE name=\"Table1-1\" description=\"Table1 on page 1\">"
        "<TABLE_CELL name=\"A1\" description=\"A1\">"
        "<PARAGRAPH>String2</PARAGRAPH>"
        "</TABLE_CELL>"
        "</TABLE>"
        "<PARAGRAPH/>"
        "<TEXT_FRAME name=\"Frame1\"><PARAGRAPH>Frame1</PARAGRAPH></TEXT_FRAME>"
        "<TEXT_FRAME name=\"Frame2\"><PARAGRAPH>Frame2</PARAGRAPH></TEXT_FRAME>"_ustr,
        collectText(xContext));
}

CPPUNIT_TEST_FIXTURE(test::AccessibleTestBase, TestTypeSimple)
{
    load(u"private:factory/swriter"_ustr);
    auto xContext = getDocumentAccessibleContext();
    CPPUNIT_ASSERT(xContext.is());

    documentPostKeyEvent(COKitKeyEventType::DOWN, 'h', 0);
    documentPostKeyEvent(COKitKeyEventType::DOWN, 'e', 0);
    documentPostKeyEvent(COKitKeyEventType::DOWN, 'l', 0);
    documentPostKeyEvent(COKitKeyEventType::DOWN, 'l', 0);
    documentPostKeyEvent(COKitKeyEventType::DOWN, 'o', 0);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(u"<PARAGRAPH>hello</PARAGRAPH>"_ustr, collectText(xContext));
}

CPPUNIT_TEST_FIXTURE(test::AccessibleTestBase, TestTypeMultiPara)
{
    load(u"private:factory/swriter"_ustr);
    auto xContext = getDocumentAccessibleContext();
    CPPUNIT_ASSERT(xContext.is());

    documentPostKeyEvent(COKitKeyEventType::DOWN, 'A', 0);
    documentPostKeyEvent(COKitKeyEventType::DOWN, 0, awt::Key::RETURN);
    documentPostKeyEvent(COKitKeyEventType::DOWN, 'B', 0);
    documentPostKeyEvent(COKitKeyEventType::DOWN, 0, awt::Key::RETURN);
    documentPostKeyEvent(COKitKeyEventType::DOWN, 'C', 0);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(
        u"<PARAGRAPH>A</PARAGRAPH><PARAGRAPH>B</PARAGRAPH><PARAGRAPH>C</PARAGRAPH>"_ustr,
        collectText(xContext));
}

CPPUNIT_TEST_FIXTURE(test::AccessibleTestBase, TestMenuInsertPageNumber)
{
    // The Kit creates no menu bar, the client drawing the menu, so there is nothing to
    // drive these tests through
    if (comphelper::COKit::isActive())
        return;

    load(u"private:factory/swriter"_ustr);
    CPPUNIT_ASSERT(activateMenuItem(u"Insert", u"Field", u"Page Number"));
    CPPUNIT_ASSERT_EQUAL(u"<PARAGRAPH>1</PARAGRAPH>"_ustr, collectText());
}

CPPUNIT_TEST_FIXTURE(test::AccessibleTestBase, TestMenuInsertPageBreak)
{
    // The Kit creates no menu bar, the client drawing the menu, so there is nothing to
    // drive these tests through
    if (comphelper::COKit::isActive())
        return;

    load(u"private:factory/swriter"_ustr);

    CPPUNIT_ASSERT(activateMenuItem(u"Insert", u"Field", u"Page Number"));
    CPPUNIT_ASSERT(activateMenuItem(u"Insert", u"Page Break"));
    // we need to move focus to the paragraph after the page break to insert the page number there
    documentPostKeyEvent(COKitKeyEventType::DOWN, 0, awt::Key::DOWN);
    CPPUNIT_ASSERT(activateMenuItem(u"Insert", u"Field", u"Page Number"));

    CPPUNIT_ASSERT_EQUAL(u"<PARAGRAPH>1</PARAGRAPH><PARAGRAPH>2</PARAGRAPH>"_ustr, collectText());
}

CPPUNIT_TEST_FIXTURE(test::AccessibleTestBase, TestTextFrameSelectedHeading)
{
    // Given a text frame starting with a heading:
    loadFromSrc(u"/sw/qa/extras/accessibility/testdocuments/heading-in-frame.fodt"_ustr);
    auto xContext = getDocumentAccessibleContext();
    CPPUNIT_ASSERT(xContext.is());

    // When the text of that heading is selected:
    auto xFrame = mxDocument.queryThrow<text::XTextFramesSupplier>()
                      ->getTextFrames()
                      ->getByName(u"Frame1"_ustr)
                      .get<uno::Reference<text::XTextFrame>>();
    auto xHeading = xFrame->getText()
                        .queryThrow<container::XEnumerationAccess>()
                        ->createEnumeration()
                        ->nextElement()
                        .get<uno::Reference<text::XTextRange>>();
    mxDocument.queryThrow<frame::XModel>()
        ->getCurrentController()
        .queryThrow<view::XSelectionSupplier>()
        ->select(uno::Any(xHeading));
    Scheduler::ProcessEventsToIdle();

    // Then the frame reports the heading as its selected child:
    auto xFrameSelection = AccessibilityTools::getAccessibleObjectForRole(
                               xContext, accessibility::AccessibleRole::TEXT_FRAME)
                               .queryThrow<accessibility::XAccessibleSelection>();
    // Without the fix, the frame reported no selected child.
    CPPUNIT_ASSERT_EQUAL(sal_Int64(1), xFrameSelection->getSelectedAccessibleChildCount());
    CPPUNIT_ASSERT_EQUAL(accessibility::AccessibleRole::HEADING,
                         xFrameSelection->getSelectedAccessibleChild(0)
                             ->getAccessibleContext()
                             ->getAccessibleRole());
}

CPPUNIT_TEST_FIXTURE(test::AccessibleTestBase, TestContentControlOffsets)
{
    // Given a content control around "Some notes":
    loadFromSrc(u"/sw/qa/extras/accessibility/testdocuments/content-control.fodt"_ustr);
    auto xText = getDocumentAccessibleContext()
                     ->getAccessibleChild(0)
                     ->getAccessibleContext()
                     .queryThrow<accessibility::XAccessibleText>();
    CPPUNIT_ASSERT_EQUAL(u"Notes: Some notes end."_ustr, xText->getText());
    auto xParagraph = mxDocument.queryThrow<text::XTextDocument>()
                          ->getText()
                          .queryThrow<container::XEnumerationAccess>()
                          ->createEnumeration()
                          ->nextElement()
                          .queryThrow<container::XEnumerationAccess>();
    auto xPortions = xParagraph->createEnumeration();
    uno::Reference<text::XTextRange> xContentControl;
    while (xPortions->hasMoreElements() && !xContentControl)
    {
        auto xPortion = xPortions->nextElement().queryThrow<beans::XPropertySet>();
        if (xPortion->getPropertyValue(u"TextPortionType"_ustr).get<OUString>() == "ContentControl")
            xContentControl
                = xPortion->getPropertyValue(u"ContentControl"_ustr).queryThrow<text::XTextRange>();
    }
    CPPUNIT_ASSERT(xContentControl);

    // When asking for the attribute run at its first character:
    accessibility::TextSegment aRun
        = xText->getTextAtIndex(7, accessibility::AccessibleTextType::ATTRIBUTE_RUN);

    // Then it is its text, not an empty run at the dummy char before it:
    CPPUNIT_ASSERT_EQUAL(u"Some notes"_ustr, aRun.SegmentText);

    // And when selecting its text:
    CPPUNIT_ASSERT(xText->setSelection(7, 17));

    // Then the selection ends where its text does, before the dummy char after it:
    auto xViewCursor = mxDocument.queryThrow<frame::XModel>()
                           ->getCurrentController()
                           .queryThrow<text::XTextViewCursorSupplier>()
                           ->getViewCursor();
    auto xCompare = xViewCursor->getText().queryThrow<text::XTextRangeCompare>();
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), xCompare->compareRegionEnds(xViewCursor, xContentControl));

    // And when selecting it backwards, the selection ends there too:
    CPPUNIT_ASSERT(xText->setSelection(17, 7));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), xCompare->compareRegionEnds(xViewCursor, xContentControl));

    // And when putting the caret before its first character, and typing there:
    CPPUNIT_ASSERT(xText->setCaretPosition(7));
    xViewCursor->getText()->insertString(xViewCursor, u"x"_ustr, /*bAbsorb=*/false);

    // Then the caret was inside the content control:
    CPPUNIT_ASSERT_EQUAL(u"xSome notes"_ustr, xContentControl->getString());

    // And when replacing its text:
    xText.queryThrow<accessibility::XAccessibleEditableText>()->replaceText(7, 18, u"New"_ustr);

    // Then the new text is inside the content control:
    CPPUNIT_ASSERT_EQUAL(u"New"_ustr, xContentControl->getString());
}

CPPUNIT_TEST_FIXTURE(test::AccessibleTestBase, TestAdjacentContentControlOffsets)
{
    // Given two adjacent content controls around "x" and "y", and an empty one:
    loadFromSrc(u"/sw/qa/extras/accessibility/testdocuments/content-control.fodt"_ustr);
    auto xText = getDocumentAccessibleContext()
                     ->getAccessibleChild(1)
                     ->getAccessibleContext()
                     .queryThrow<accessibility::XAccessibleText>();
    CPPUNIT_ASSERT_EQUAL(u"Two: xy and end."_ustr, xText->getText());

    // When asking for the attribute runs at "y" and at the empty one:
    // Then they are the text there, not empty runs at the dummy chars:
    CPPUNIT_ASSERT_EQUAL(
        u"y"_ustr,
        xText->getTextAtIndex(6, accessibility::AccessibleTextType::ATTRIBUTE_RUN).SegmentText);
    CPPUNIT_ASSERT_EQUAL(
        u" end."_ustr,
        xText->getTextAtIndex(11, accessibility::AccessibleTextType::ATTRIBUTE_RUN).SegmentText);

    // And when putting the caret before "y", and typing there:
    CPPUNIT_ASSERT(xText->setCaretPosition(6));
    auto xViewCursor = mxDocument.queryThrow<frame::XModel>()
                           ->getCurrentController()
                           .queryThrow<text::XTextViewCursorSupplier>()
                           ->getViewCursor();
    xViewCursor->getText()->insertString(xViewCursor, u"z"_ustr, /*bAbsorb=*/false);

    // Then the caret was inside the second content control, not between the two:
    CPPUNIT_ASSERT_EQUAL(u"Two: xzy and end."_ustr, xText->getText());
    auto xParagraphs = mxDocument.queryThrow<text::XTextDocument>()
                           ->getText()
                           .queryThrow<container::XEnumerationAccess>()
                           ->createEnumeration();
    xParagraphs->nextElement();
    auto xPortions = xParagraphs->nextElement()
                         .queryThrow<container::XEnumerationAccess>()
                         ->createEnumeration();
    std::vector<OUString> aContents;
    while (xPortions->hasMoreElements())
    {
        auto xPortion = xPortions->nextElement().queryThrow<beans::XPropertySet>();
        if (xPortion->getPropertyValue(u"TextPortionType"_ustr).get<OUString>() == "ContentControl")
            aContents.push_back(xPortion->getPropertyValue(u"ContentControl"_ustr)
                                    .queryThrow<text::XTextRange>()
                                    ->getString());
    }
    CPPUNIT_ASSERT_EQUAL(size_t(3), aContents.size());
    CPPUNIT_ASSERT_EQUAL(u"x"_ustr, aContents[0]);
    CPPUNIT_ASSERT_EQUAL(u"zy"_ustr, aContents[1]);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
