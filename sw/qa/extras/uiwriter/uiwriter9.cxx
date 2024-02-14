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
#include <com/sun/star/table/TableBorder2.hpp>
#include <com/sun/star/text/XDocumentIndex.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>

#include <comphelper/propertysequence.hxx>
#include <swdtflvr.hxx>
#include <o3tl/string_view.hxx>

#include <view.hxx>
#include <wrtsh.hxx>
#include <unotxdoc.hxx>
#include <ndtxt.hxx>
#include <toxmgr.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <fmtinfmt.hxx>

namespace
{
class SwUiWriterTest9 : public SwModelTestBase
{
public:
    SwUiWriterTest9()
        : SwModelTestBase("/sw/qa/extras/uiwriter/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(SwUiWriterTest9, testTdf158785)
{
    // given a document with a hyperlink surrounded by N-dashes (–www.dordt.edu–)
    createSwDoc("tdf158785_hyperlink.fodt");
    SwDoc& rDoc = *getSwDoc();
    SwWrtShell* pWrtShell = rDoc.GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    // go to the end of the hyperlink
    pWrtShell->SttEndDoc(/*bStart=*/false);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    // get last point that will be part of the hyperlink (current position 1pt wide).
    Point aLogicL(pWrtShell->GetCharRect().Center());
    Point aLogicR(aLogicL);

    // sanity check - we really are right by the hyperlink
    aLogicL.AdjustX(-1);
    SwContentAtPos aContentAtPos(IsAttrAtPos::InetAttr);
    pWrtShell->GetContentAtPos(aLogicL, aContentAtPos);
    CPPUNIT_ASSERT_EQUAL(IsAttrAtPos::InetAttr, aContentAtPos.eContentAtPos);

    // The test: the position of the N-dash should not indicate hyperlink properties
    // cursor pos would NOT be considered part of the hyperlink, but increase for good measure...
    aLogicR.AdjustX(1);
    pWrtShell->GetContentAtPos(aLogicR, aContentAtPos);
    CPPUNIT_ASSERT_EQUAL(IsAttrAtPos::NONE, aContentAtPos.eContentAtPos);

    /*
     * tdf#111969: the beginning of the hyperlink should allow the right-click menu to remove it
     */
    // move cursor (with no selection) to the start of the hyperlink - after the N-dash
    pWrtShell->SttEndDoc(/*bStart=*/true);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    aLogicL = pWrtShell->GetCharRect().Center();
    aLogicR = aLogicL;

    // sanity check - we really are right in front of the hyperlink
    aLogicL.AdjustX(-1);
    aContentAtPos = IsAttrAtPos::InetAttr;
    pWrtShell->GetContentAtPos(aLogicL, aContentAtPos);
    CPPUNIT_ASSERT_EQUAL(IsAttrAtPos::NONE, aContentAtPos.eContentAtPos);
    aLogicR.AdjustX(1);
    aContentAtPos = IsAttrAtPos::InetAttr;
    pWrtShell->GetContentAtPos(aLogicR, aContentAtPos);
    CPPUNIT_ASSERT_EQUAL(IsAttrAtPos::InetAttr, aContentAtPos.eContentAtPos);

    // Remove the hyperlink
    dispatchCommand(mxComponent, ".uno:RemoveHyperlink", {});

    // The test: was the hyperlink actually removed?
    aContentAtPos = IsAttrAtPos::InetAttr;
    pWrtShell->GetContentAtPos(aLogicR, aContentAtPos);
    CPPUNIT_ASSERT_EQUAL(IsAttrAtPos::NONE, aContentAtPos.eContentAtPos);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest9, testTdf111969)
{
    // given a document with a field surrounded by N-dashes (–date–)
    createSwDoc("tdf111969_field.fodt");
    SwDoc& rDoc = *getSwDoc();
    SwWrtShell* pWrtShell = rDoc.GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    // go to the end of the field
    pWrtShell->SttEndDoc(/*bStart=*/false);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    // get last point that will be part of the field (current position 1pt wide).
    Point aLogicL(pWrtShell->GetCharRect().Center());
    Point aLogicR(aLogicL);

    // sanity check - we really are at the right edge of the field
    aLogicR.AdjustX(1);
    SwContentAtPos aContentAtPos(IsAttrAtPos::Field);
    pWrtShell->GetContentAtPos(aLogicR, aContentAtPos);
    CPPUNIT_ASSERT_EQUAL(IsAttrAtPos::NONE, aContentAtPos.eContentAtPos);
    aLogicL.AdjustX(-1);
    aContentAtPos = IsAttrAtPos::Field;
    pWrtShell->GetContentAtPos(aLogicL, aContentAtPos);
    CPPUNIT_ASSERT_EQUAL(IsAttrAtPos::Field, aContentAtPos.eContentAtPos);

    // the test: simulate a right-click of a mouse which sets the cursor and then acts on that pos.
    pWrtShell->SwCursorShell::SetCursor(aLogicL, false, /*Block=*/false, /*FieldInfo=*/true);
    CPPUNIT_ASSERT(pWrtShell->GetCurField(true));

    /*
     * An edge case at the start of a field - don't start the field menu on the first N-dash
     */
    // go to the start of the field
    pWrtShell->SttEndDoc(/*bStart=*/true);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    // get first point that will be part of the field (current position 1pt wide).
    aLogicL = pWrtShell->GetCharRect().Center();
    aLogicR = aLogicL;

    // sanity check - we really are at the left edge of the field
    aLogicR.AdjustX(1);
    aContentAtPos = IsAttrAtPos::Field;
    pWrtShell->GetContentAtPos(aLogicR, aContentAtPos);
    CPPUNIT_ASSERT_EQUAL(IsAttrAtPos::Field, aContentAtPos.eContentAtPos);
    aLogicL.AdjustX(-1);
    aContentAtPos = IsAttrAtPos::Field;
    pWrtShell->GetContentAtPos(aLogicL, aContentAtPos);
    CPPUNIT_ASSERT_EQUAL(IsAttrAtPos::NONE, aContentAtPos.eContentAtPos);

    // the test: simulate a right-click of a mouse (at the end-edge of the N-dash)
    // which sets the cursor and then acts on that pos.
    pWrtShell->SwCursorShell::SetCursor(aLogicL, false, /*Block=*/false, /*FieldInfo=*/true);
    CPPUNIT_ASSERT(!pWrtShell->GetCurField(true));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest9, testTdf111969B)
{
    // given a document with a field surrounded by two N-dashes (––date––)
    createSwDoc("tdf111969_fieldB.fodt");
    SwDoc& rDoc = *getSwDoc();
    SwWrtShell* pWrtShell = rDoc.GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    // go to the start of the field
    pWrtShell->SttEndDoc(/*bStart=*/true);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 2, /*bBasicCall=*/false);
    // get first point that will be part of the field (current position 1pt wide).
    Point aLogicL(pWrtShell->GetCharRect().Center());
    Point aLogicR(aLogicL);

    // sanity check - we really are at the left edge of the field
    aLogicR.AdjustX(1);
    SwContentAtPos aContentAtPos(IsAttrAtPos::Field);
    pWrtShell->GetContentAtPos(aLogicR, aContentAtPos);
    CPPUNIT_ASSERT_EQUAL(IsAttrAtPos::Field, aContentAtPos.eContentAtPos);
    aLogicL.AdjustX(-1);
    aContentAtPos = IsAttrAtPos::Field;
    pWrtShell->GetContentAtPos(aLogicL, aContentAtPos);
    CPPUNIT_ASSERT_EQUAL(IsAttrAtPos::NONE, aContentAtPos.eContentAtPos);

    // the test: simulate a right-click of a mouse (at the end-edge of the second N-dash)
    // which sets the cursor and then acts on that pos.
    pWrtShell->SwCursorShell::SetCursor(aLogicL, false, /*Block=*/false, /*FieldInfo=*/true);
    CPPUNIT_ASSERT(!pWrtShell->GetCurField(true));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest9, testTdf159049)
{
    // The document contains a shape which has a text with a line break. When copying the text to
    // clipboard the line break was missing in the RTF flavor of the clipboard.
    createSwDoc("tdf159049_LineBreakRTFClipboard.fodt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    selectShape(1);

    // Bring shape into text edit mode
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_RETURN);
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_RETURN);
    Scheduler::ProcessEventsToIdle();
    // Copy text
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:Copy", {});

    // Deactivate text edit mode ...
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_ESCAPE);
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_ESCAPE);
    Scheduler::ProcessEventsToIdle();
    // ... and deselect shape.
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_ESCAPE);
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_ESCAPE);
    Scheduler::ProcessEventsToIdle();

    // Paste special as RTF
    uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence(
        { { "SelectedFormat", uno::Any(static_cast<sal_uInt32>(SotClipboardFormatId::RTF)) } }));
    dispatchCommand(mxComponent, ".uno:ClipboardFormatItems", aArgs);
    // Without fix Actual was "Abreakhere", the line break \n was missing.
    CPPUNIT_ASSERT_EQUAL(u"Abreak\nhere"_ustr, getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest9, testTdf135083)
{
    createSwDoc("tdf135083-simple-text-plus-list.fodt");

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});

    // Paste special as RTF
    uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence(
        { { u"SelectedFormat"_ustr,
            uno::Any(static_cast<sal_uInt32>(SotClipboardFormatId::RTF)) } }));
    dispatchCommand(mxComponent, ".uno:ClipboardFormatItems", aArgs);

    auto xLastPara = getParagraph(3);
    CPPUNIT_ASSERT_EQUAL(u"dolor"_ustr, xLastPara->getString());
    // Without the fix in place, the last paragraph would loose its settings. ListId would be empty.
    CPPUNIT_ASSERT(!getProperty<OUString>(xLastPara, u"ListId"_ustr).isEmpty());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest9, testHiddenSectionsAroundPageBreak)
{
    createSwDoc("hiddenSectionsAroundPageBreak.fodt");

    CPPUNIT_ASSERT_EQUAL(1, getPages());

    auto xModel(mxComponent.queryThrow<frame::XModel>());
    auto xTextViewCursorSupplier(
        xModel->getCurrentController().queryThrow<text::XTextViewCursorSupplier>());
    auto xCursor(xTextViewCursorSupplier->getViewCursor().queryThrow<text::XPageCursor>());

    // Make sure that the page style is set correctly
    xCursor->jumpToFirstPage();
    CPPUNIT_ASSERT_EQUAL(u"Landscape"_ustr, getProperty<OUString>(xCursor, "PageStyleName"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest9, testTdf159565)
{
    // Given a document with a hidden section in the beginning, additionally containing a frame
    createSwDoc("FrameInHiddenSection.fodt");

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});

    // Check that the selection covers the whole visible text
    auto xModel(mxComponent.queryThrow<css::frame::XModel>());
    auto xSelSupplier(xModel->getCurrentController().queryThrow<css::view::XSelectionSupplier>());
    auto xSelections(xSelSupplier->getSelection().queryThrow<css::container::XIndexAccess>());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSelections->getCount());
    auto xSelection(xSelections->getByIndex(0).queryThrow<css::text::XTextRange>());

    // Without the fix, this would fail - there was no selection
    CPPUNIT_ASSERT_EQUAL(u"" SAL_NEWLINE_STRING SAL_NEWLINE_STRING "ipsum"_ustr,
                         xSelection->getString());
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
