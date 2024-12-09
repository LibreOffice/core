/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>
#include <officecfg/Office/Common.hxx>
#include <com/sun/star/document/XEmbeddedObjectSupplier2.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <vcl/scheduler.hxx>

#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/table/TableBorder2.hpp>
#include <com/sun/star/text/XDocumentIndex.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/text/XParagraphCursor.hpp>

#include <com/sun/star/view/XSelectionSupplier.hpp>

#include <comphelper/lok.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/scopeguard.hxx>
#include <comphelper/configuration.hxx>
#include <swdtflvr.hxx>
#include <o3tl/string_view.hxx>
#include <editeng/acorrcfg.hxx>
#include <swacorr.hxx>
#include <sfx2/linkmgr.hxx>

#include <scriptinfo.hxx>
#include <txtfrm.hxx>
#include <edtwin.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <unotxdoc.hxx>
#include <itabenum.hxx>
#include <ndtxt.hxx>
#include <toxmgr.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentLinksAdministration.hxx>
#include <fmtinfmt.hxx>
#include <rootfrm.hxx>
#include <svx/svxids.hrc>
#include <svx/svdview.hxx>
#include <svx/svdmark.hxx>

namespace
{
class SwUiWriterTest9 : public SwModelTestBase
{
public:
    SwUiWriterTest9()
        : SwModelTestBase(u"/sw/qa/extras/uiwriter/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(SwUiWriterTest9, testTdf158785)
{
    // given a document with a hyperlink surrounded by N-dashes (–www.dordt.edu–)
    createSwDoc("tdf158785_hyperlink.fodt");
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
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
    dispatchCommand(mxComponent, u".uno:RemoveHyperlink"_ustr, {});

    // The test: was the hyperlink actually removed?
    aContentAtPos = IsAttrAtPos::InetAttr;
    pWrtShell->GetContentAtPos(aLogicR, aContentAtPos);
    CPPUNIT_ASSERT_EQUAL(IsAttrAtPos::NONE, aContentAtPos.eContentAtPos);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest9, testTdf159377)
{
    createSwDoc();

    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();

    SwInsertTableOptions aTableOptions(SwInsertTableFlags::DefaultBorder, 0);
    pWrtShell->InsertTable(aTableOptions, /*nRows=*/2, /*nCols=*/2);
    pWrtShell->MoveTable(GotoPrevTable, fnTableStart);

    dispatchCommand(mxComponent, u".uno:SelectTable"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});

    pWrtShell->InsertFootnote(u""_ustr);
    CPPUNIT_ASSERT(pWrtShell->IsCursorInFootnote());

    CPPUNIT_ASSERT_EQUAL(SwNodeOffset(28), pDoc->GetNodes().Count());

    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});

    // this pasted the 4 text nodes in the table, but no table nodes
    // as currently tables aren't allowed in footnotes

    CPPUNIT_ASSERT_EQUAL(SwNodeOffset(32), pDoc->GetNodes().Count());

    pWrtShell->Undo();

    CPPUNIT_ASSERT(pWrtShell->IsCursorInFootnote());
    // problem was that this was 29 with an extra text node in the footnote
    CPPUNIT_ASSERT_EQUAL(SwNodeOffset(28), pDoc->GetNodes().Count());

    pWrtShell->Redo();

    CPPUNIT_ASSERT_EQUAL(SwNodeOffset(32), pDoc->GetNodes().Count());

    pWrtShell->Undo();

    CPPUNIT_ASSERT_EQUAL(SwNodeOffset(28), pDoc->GetNodes().Count());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest9, testPasteTableInMiddleOfParagraph)
{
    createSwDoc();

    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();

    SwInsertTableOptions aTableOptions(SwInsertTableFlags::DefaultBorder, 0);
    pWrtShell->InsertTable(aTableOptions, /*nRows=*/2, /*nCols=*/2);
    pWrtShell->MoveTable(GotoPrevTable, fnTableStart);

    dispatchCommand(mxComponent, u".uno:SelectTable"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});

    pWrtShell->Undo();

    pWrtShell->Insert(u"AB"_ustr);

    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);

    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});

    pWrtShell->Undo();

    // the problem was that the A was missing
    CPPUNIT_ASSERT_EQUAL(OUString("AB"),
                         pWrtShell->GetCursor()->GetPointNode().GetTextNode()->GetText());

    pWrtShell->Redo();
    pWrtShell->Undo();
    CPPUNIT_ASSERT_EQUAL(OUString("AB"),
                         pWrtShell->GetCursor()->GetPointNode().GetTextNode()->GetText());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest9, testTdf111969)
{
    // given a document with a field surrounded by N-dashes (–date–)
    createSwDoc("tdf111969_field.fodt");
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
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
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
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
    SwXTextDocument* pTextDoc = getSwTextDoc();
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_RETURN);
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_RETURN);
    Scheduler::ProcessEventsToIdle();
    // Copy text
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});

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
    dispatchCommand(mxComponent, u".uno:ClipboardFormatItems"_ustr, aArgs);
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
    dispatchCommand(mxComponent, u".uno:ClipboardFormatItems"_ustr, aArgs);

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
    CPPUNIT_ASSERT_EQUAL(u"Landscape"_ustr, getProperty<OUString>(xCursor, u"PageStyleName"_ustr));
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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest9, testTdf159816)
{
    createSwDoc();

    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    // Add 5 empty paragraphs
    pWrtShell->SplitNode();
    pWrtShell->SplitNode();
    pWrtShell->SplitNode();
    pWrtShell->SplitNode();
    pWrtShell->SplitNode();

    // Add a bookmark at the very end
    IDocumentMarkAccess& rIDMA(*pDoc->getIDocumentMarkAccess());
    rIDMA.makeMark(*pWrtShell->GetCursor(), u"Mark"_ustr, IDocumentMarkAccess::MarkType::BOOKMARK,
                   sw::mark::InsertMode::New);

    // Get coordinates of the end point in the document
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    SwFrame* pPage = pLayout->Lower();
    SwFrame* pBody = pPage->GetLower();
    SwFrame* pLastPara = pBody->GetLower()->GetNext()->GetNext()->GetNext()->GetNext()->GetNext();
    Point ptTo = pLastPara->getFrameArea().BottomRight();

    pWrtShell->SelAll();

    // Drag-n-drop to its own end
    rtl::Reference<SwTransferable> xTransfer = new SwTransferable(*pWrtShell);
    // Without the fix, this would crash: either in CopyFlyInFlyImpl (tdf#159813):
    // Assertion failed: !pCopiedPaM || pCopiedPaM->End()->GetNode() == rRg.aEnd.GetNode()
    // or in BigPtrArray::operator[] (tdf#159816):
    // Assertion failed: idx < m_nSize
    xTransfer->PrivateDrop(*pWrtShell, ptTo, /*bMove=*/true, /*bXSelection=*/true);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest9, testTdf34804)
{
    createSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();

    // Simulate a keyboard shortcut to SID_ATTR_CHAR_COLOR2 (which must use the shared button color)
    dispatchCommand(mxComponent, u".uno:FontColor"_ustr, {});
    pWrtShell->Insert(u"New World!"_ustr);

    const uno::Reference<text::XTextRange> xRun = getRun(getParagraph(1, "New World!"), 1);
    // (This test assumes that nothing in the unit tests has modified the app's recent font color)
    // COL_DEFAULT_FONT is the default red color for the fontColor button on the toolbar.
    CPPUNIT_ASSERT_EQUAL(COL_DEFAULT_FONT, getProperty<Color>(xRun, u"CharColor"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest9, testTdf139631)
{
    // Unit test for tdf#139631
    // Test to see if preceding space is cut when cutting a word with track changes (redline) on
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();

    pWrtShell->Insert(u"New World!\""_ustr);
    // Assert that the string, New World!", is inserted correctly into the document
    CPPUNIT_ASSERT_EQUAL(u"New World!\""_ustr, getParagraph(1)->getString());

    // Enable redline
    dispatchCommand(mxComponent, u".uno:TrackChanges"_ustr, {});
    CPPUNIT_ASSERT(pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    // Hide redline changes
    dispatchCommand(mxComponent, u".uno:ShowTrackedChanges"_ustr, {});
    CPPUNIT_ASSERT(pWrtShell->GetLayout()->IsHideRedlines());

    pWrtShell->Left(SwCursorSkipMode::Chars, false, 2, false);
    // Select and cut "World" from string
    pWrtShell->Left(SwCursorSkipMode::Chars, true, 5, false);
    dispatchCommand(mxComponent, u".uno:Cut"_ustr, {});
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    pXmlDoc = parseLayoutDump();
    // Verifies that the leading space before "World" was also cut
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion",
                "portion", u"New!\"");

    // Reset to initial string
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion",
                "portion", u"New World!\"");

    pWrtShell->EndPara(false);
    pWrtShell->Left(SwCursorSkipMode::Chars, false, 1, false);
    // Replace ! with .
    pWrtShell->Left(SwCursorSkipMode::Chars, true, 1, false);
    pWrtShell->Delete();
    pWrtShell->Insert(u"."_ustr);
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion",
                "portion", u"New World.\"");

    pWrtShell->Left(SwCursorSkipMode::Chars, false, 1, false);
    // Select and cut "World" from string
    pWrtShell->Left(SwCursorSkipMode::Chars, true, 5, false);
    dispatchCommand(mxComponent, u".uno:Cut"_ustr, {});

    pXmlDoc = parseLayoutDump();
    // Without the test in place, the leading space before "World" is not also cut.
    // Expected: New."
    // Actual: New ."
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion",
                "portion", u"New.\"");
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest9, testTdf165351)
{
    createSwDoc("tdf165351.fodt");

    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();

    // Move the cursor into the fly frame
    pWrtShell->GotoFly(u"Frame1"_ustr, FLYCNTTYPE_FRM, false);
    pWrtShell->EndOfSection();
    pWrtShell->GoNextCell(/*bAppendLine=*/true);
    pWrtShell->Undo();
    // getting this item crashed
    SfxItemSet temp{ pDoc->GetAttrPool(), svl::Items<SID_RULER_LR_MIN_MAX, SID_RULER_LR_MIN_MAX> };
    pWrtShell->GetView().StateTabWin(temp);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest9, testTdf151710)
{
    createSwDoc();

    // Check that the particular setting is turned on by default
    const SwViewOption* pVwOpt = getSwDocShell()->GetWrtShell()->GetViewOptions();
    CPPUNIT_ASSERT(pVwOpt);
    CPPUNIT_ASSERT(pVwOpt->IsEncloseWithCharactersOn());

    // Localized quotation marks
    SvxAutoCorrect* pACorr = SvxAutoCorrCfg::Get().GetAutoCorrect();
    CPPUNIT_ASSERT(pACorr);
    LanguageType eLang = Application::GetSettings().GetLanguageTag().getLanguageType();
    OUString sStartSingleQuote{ pACorr->GetQuote('\'', true, eLang) };
    OUString sEndSingleQuote{ pACorr->GetQuote('\'', false, eLang) };
    OUString sStartDoubleQuote{ pACorr->GetQuote('\"', true, eLang) };
    OUString sEndDoubleQuote{ pACorr->GetQuote('\"', false, eLang) };

    // Insert some text to work with
    uno::Sequence<beans::PropertyValue> aArgsInsert(
        comphelper::InitPropertySequence({ { "Text", uno::Any(u"abcd"_ustr) } }));
    dispatchCommand(mxComponent, u".uno:InsertText"_ustr, aArgsInsert);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"abcd"_ustr, xTextDocument->getText()->getString());

    // Successfully enclose the text; afterwards the selection should exist with the new
    // enclosed text
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    SwXTextDocument* pTextDoc = getSwTextDoc();
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, '(', 0);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(u"(abcd)"_ustr, xTextDocument->getText()->getString());

    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, '[', 0);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(u"[(abcd)]"_ustr, xTextDocument->getText()->getString());

    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, '{', 0);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(u"{[(abcd)]}"_ustr, xTextDocument->getText()->getString());

    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, '\'', 0);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(OUString(sStartSingleQuote + "{[(abcd)]}" + sEndSingleQuote),
                         xTextDocument->getText()->getString());

    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, '\"', 0);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(OUString(sStartDoubleQuote + sStartSingleQuote + "{[(abcd)]}"
                                  + sEndSingleQuote + sEndDoubleQuote),
                         xTextDocument->getText()->getString());

    // Disable the setting and check that enclosing doesn't happen anymore
    const_cast<SwViewOption*>(pVwOpt)->SetEncloseWithCharactersOn(false);
    CPPUNIT_ASSERT(!pVwOpt->IsEncloseWithCharactersOn());

    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, '(', 0);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(u"("_ustr, xTextDocument->getText()->getString());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, '[', 0);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(u"["_ustr, xTextDocument->getText()->getString());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, '{', 0);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(u"{"_ustr, xTextDocument->getText()->getString());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, '\'', 0);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(sStartSingleQuote, xTextDocument->getText()->getString());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, '\"', 0);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(sStartDoubleQuote, xTextDocument->getText()->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest9, testTdf159054_disableOutlineNumbering)
{
    createSwDoc("tdf159054_disableOutlineNumbering.docx");
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();

    const uno::Reference<text::XTextRange> xPara1 = getParagraph(1, u"Heading A"_ustr);
    const uno::Reference<text::XTextRange> xPara2 = getParagraph(2, u"Heading B"_ustr);
    const uno::Reference<text::XTextRange> xPara3 = getParagraph(3, u"Heading C"_ustr);

    CPPUNIT_ASSERT_EQUAL(u"A."_ustr, getProperty<OUString>(xPara1, u"ListLabelString"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"B."_ustr, getProperty<OUString>(xPara2, u"ListLabelString"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"C."_ustr, getProperty<OUString>(xPara3, u"ListLabelString"_ustr));

    // select (at least parts) of the first two paragraphs
    pWrtShell->Down(/*bSelect=*/true, /*nCount=*/1, /*bBasicCall=*/true);

    // on the selection, simulate pressing the toolbar button to toggle OFF numbering
    dispatchCommand(mxComponent, u".uno:DefaultNumbering"_ustr, {});

    // the selected paragraphs should definitely have the list label removed
    CPPUNIT_ASSERT_EQUAL(u""_ustr, getProperty<OUString>(xPara1, u"ListLabelString"_ustr));
    CPPUNIT_ASSERT_EQUAL(u""_ustr, getProperty<OUString>(xPara2, u"ListLabelString"_ustr));
    // the third paragraph must retain the existing numbering format
    CPPUNIT_ASSERT_EQUAL(u"A."_ustr, getProperty<OUString>(xPara3, u"ListLabelString"_ustr));

    // on the selection, simulate pressing the toolbar button to toggle ON numbering again
    dispatchCommand(mxComponent, u".uno:DefaultNumbering"_ustr, {});

    // the outline numbering format must be re-applied to the first two paragraphs
    CPPUNIT_ASSERT_EQUAL(u"A."_ustr, getProperty<OUString>(xPara1, u"ListLabelString"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"B."_ustr, getProperty<OUString>(xPara2, u"ListLabelString"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"C."_ustr, getProperty<OUString>(xPara3, u"ListLabelString"_ustr));

    // on the selection, simulate a right click - list - No list
    dispatchCommand(mxComponent, u".uno:RemoveBullets"_ustr, {});

    // the selected paragraphs should definitely have the list label removed
    CPPUNIT_ASSERT_EQUAL(u""_ustr, getProperty<OUString>(xPara1, u"ListLabelString"_ustr));
    CPPUNIT_ASSERT_EQUAL(u""_ustr, getProperty<OUString>(xPara2, u"ListLabelString"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"A."_ustr, getProperty<OUString>(xPara3, u"ListLabelString"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest9, testTdf158375_dde_disable)
{
    std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
        comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Security::Scripting::DisableActiveContent::set(true, pBatch);
    pBatch->commit();
    comphelper::ScopeGuard g([] {
        std::shared_ptr<comphelper::ConfigurationChanges> _pBatch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Security::Scripting::DisableActiveContent::set(false, _pBatch);
        _pBatch->commit();
    });

    createSwDoc();
    SwDoc* pDoc = getSwDoc();

    // force the AppName to enable DDE, it is not there for test runs
    Application::SetAppName(u"soffice"_ustr);

    // temp copy for the file that will be used as a reference for DDE link
    // this file includes a section named "Section1" with text inside
    createTempCopy(u"tdf158375_dde_reference.fodt");

    comphelper::EmbeddedObjectContainer& rEmbeddedObjectContainer
        = getSwDocShell()->getEmbeddedObjectContainer();
    rEmbeddedObjectContainer.setUserAllowsLinkUpdate(true);

    // create a section with DDE link
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xTextSectionProps(
        xFactory->createInstance(u"com.sun.star.text.TextSection"_ustr), uno::UNO_QUERY);

    uno::Sequence<OUString> aNames{ u"DDECommandFile"_ustr, u"DDECommandType"_ustr,
                                    u"DDECommandElement"_ustr, u"IsAutomaticUpdate"_ustr,
                                    u"IsProtected"_ustr };
    uno::Sequence<uno::Any> aValues{ uno::Any(u"soffice"_ustr), uno::Any(maTempFile.GetURL()),
                                     uno::Any(u"Section1"_ustr), uno::Any(true), uno::Any(true) };
    uno::Reference<beans::XMultiPropertySet> rMultiPropSet(xTextSectionProps, uno::UNO_QUERY);
    rMultiPropSet->setPropertyValues(aNames, aValues);

    // insert the TextSection with DDE link
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xTextRange = xTextDocument->getText();
    uno::Reference<text::XText> xText = xTextRange->getText();
    uno::Reference<text::XParagraphCursor> xCursor(xText->createTextCursor(), uno::UNO_QUERY);
    xText->insertTextContent(
        xCursor, uno::Reference<text::XTextContent>(xTextSectionProps, uno::UNO_QUERY), false);

    CPPUNIT_ASSERT_EQUAL(
        size_t(1), pDoc->getIDocumentLinksAdministration().GetLinkManager().GetLinks().size());

    pDoc->getIDocumentLinksAdministration().GetLinkManager().UpdateAllLinks(false, false, nullptr,
                                                                            u""_ustr);

    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xSections(xTextSectionsSupplier->getTextSections(),
                                                      uno::UNO_QUERY);
    uno::Reference<text::XTextSection> xSection(xSections->getByIndex(0), uno::UNO_QUERY);

    // make sure there's no text in the section after UpdateAllLinks, since
    // DisableActiveContent disables DDE links.
    CPPUNIT_ASSERT_EQUAL(u""_ustr, xSection->getAnchor()->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest9, testTdf158375_ole_object_disable)
{
    std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
        comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Security::Scripting::DisableActiveContent::set(true, pBatch);
    pBatch->commit();
    comphelper::ScopeGuard g([] {
        std::shared_ptr<comphelper::ConfigurationChanges> _pBatch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Security::Scripting::DisableActiveContent::set(false, _pBatch);
        _pBatch->commit();
    });

    // Enable LOK mode, otherwise OCommonEmbeddedObject::SwitchStateTo_Impl() will throw when it
    // finds out that the test runs headless.
    comphelper::LibreOfficeKit::setActive();

    // Load a document with a Draw doc in it.
    createSwDoc("ole-save-while-edit.odt");
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();

    selectShape(1);

    // attempt to edit the OLE object.
    pWrtShell->LaunchOLEObj();

    // it shouldn't switch because the current configuration
    // (DisableActiveContent) prohibits OLE objects changing to states other
    // then LOADED
    auto xShape = getShape(1);
    uno::Reference<document::XEmbeddedObjectSupplier2> xEmbedSupplier(xShape, uno::UNO_QUERY);
    auto xEmbeddedObj = xEmbedSupplier->getExtendedControlOverEmbeddedObject();
    CPPUNIT_ASSERT_EQUAL(embed::EmbedStates::LOADED, xEmbeddedObj->getCurrentState());

    // Dispose the document while LOK is still active to avoid leaks.
    mxComponent->dispose();
    mxComponent.clear();
    comphelper::LibreOfficeKit::setActive(false);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest9, testTdf146190)
{
    // Given a document with a number rule at the start of a paragraph and two drawing objects:
    createSwDoc("tdf146190.odt");
    SwDocShell* pDocShell = getSwDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();

    const SdrMarkList& rMrkList = pWrtShell->GetDrawView()->GetMarkedObjectList();

    // Assert the current cursor position has a number rule and is at the start of a paragraph:
    pWrtShell->SttEndDoc(/*bStt=*/true);
    CPPUNIT_ASSERT(pWrtShell->GetNumRuleAtCurrCursorPos());
    CPPUNIT_ASSERT(pWrtShell->IsSttOfPara());

    // Then go to "Shape 1" drawing object using the GotoDrawingObject function:
    pWrtShell->GotoDrawingObject(u"Shape 1");
    CPPUNIT_ASSERT_EQUAL(u"Shape 1"_ustr, rMrkList.GetMark(0)->GetMarkedSdrObj()->GetName());

    // Move to the next drawing object by Tab key press:
    SwXTextDocument* pTextDoc = getSwTextDoc();
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
    Scheduler::ProcessEventsToIdle();
    // Without the fix in place, this test would have failed with:
    // equality assertion failed
    // - Expected: Shape 2
    // - Actual  : Shape 1
    // i.e. Tab did not move to the next drawing object
    CPPUNIT_ASSERT_EQUAL(u"Shape 2"_ustr, rMrkList.GetMark(0)->GetMarkedSdrObj()->GetName());

    // Tab key press should now select 'Shape 1':
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(u"Shape 1"_ustr, rMrkList.GetMark(0)->GetMarkedSdrObj()->GetName());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest9, testTdf160898)
{
    // Given a document with a 1-cell table in another 1-cell table:
    createSwDoc("table-in-table.fodt");
    SwDocShell* pDocShell = getSwDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();

    // Move to the normally hidden paragraph inside the outer table cell, following the inner table
    pWrtShell->Down(false, 2);
    // Without the fix, this would crash:
    pWrtShell->SelAll();
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest9, testTdf164949)
{
    createSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();

    SwInsertTableOptions aTableOptions(SwInsertTableFlags::DefaultBorder, 0);
    pWrtShell->InsertTable(aTableOptions, /*nRows=*/2, /*nCols=*/2);
    pWrtShell->MoveTable(GotoPrevTable, fnTableStart);

    dispatchCommand(mxComponent, u".uno:SelectTable"_ustr, {});

    uno::Sequence aArgs{ comphelper::makePropertyValue(u"PersistentCopy"_ustr, uno::Any(false)) };

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, u".uno:FormatPaintbrush"_ustr, aArgs);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest9, testParagraphStyleCloneFormatting)
{
    createSwDoc();
    emulateTyping(u"First Line");

    SwXTextDocument* pTextDoc = getSwTextDoc();
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_RETURN);
    pTextDoc->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_RETURN);
    Scheduler::ProcessEventsToIdle();

    emulateTyping(u"Second Line");

    CPPUNIT_ASSERT_EQUAL(u"Standard"_ustr,
                         getProperty<OUString>(getParagraph(1), u"ParaStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Standard"_ustr,
                         getProperty<OUString>(getParagraph(2), u"ParaStyleName"_ustr));

    uno::Sequence<beans::PropertyValue> aPropertyValues = comphelper::InitPropertySequence({
        { "Style", uno::Any(u"Heading 1"_ustr) },
        { "FamilyName", uno::Any(u"ParagraphStyles"_ustr) },
    });
    dispatchCommand(mxComponent, u".uno:StyleApply"_ustr, aPropertyValues);

    CPPUNIT_ASSERT_EQUAL(u"Standard"_ustr,
                         getProperty<OUString>(getParagraph(1), u"ParaStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Heading 1"_ustr,
                         getProperty<OUString>(getParagraph(2), u"ParaStyleName"_ustr));

    uno::Sequence aArgs{ comphelper::makePropertyValue(u"PersistentCopy"_ustr, uno::Any(false)) };
    dispatchCommand(mxComponent, u".uno:FormatPaintbrush"_ustr, aArgs);

    // Disable map mode, so that it's possible to send mouse event coordinates
    // directly in twips.
    SwEditWin& rEditWin = getSwDocShell()->GetView()->GetEditWin();
    rEditWin.EnableMapMode(false);

    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);
    SwShellCursor* pShellCursor = pWrtShell->getShellCursor(false);

    // move to first line
    pWrtShell->Up(/*bSelect=*/false, 1);
    Point aPoint = pShellCursor->GetSttPos();

    // click on first line
    pTextDoc->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONDOWN, aPoint.getX(), aPoint.getY(), 1,
                             MOUSE_LEFT, 0);
    pTextDoc->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONUP, aPoint.getX(), aPoint.getY(), 1,
                             MOUSE_LEFT, 0);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(u"Heading 1"_ustr,
                         getProperty<OUString>(getParagraph(1), u"ParaStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Heading 1"_ustr,
                         getProperty<OUString>(getParagraph(2), u"ParaStyleName"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest9, testTdf122756)
{
    createSwDoc("tdf122756.odt");

    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);

    uno::Reference<text::XTextRange> xCellA1(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"€ 100,00"_ustr, xCellA1->getString());

    uno::Reference<text::XTextRange> xCellA2(xTable->getCellByName(u"A2"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"100"_ustr, xCellA2->getString());

    // Cursor is already on cell A1
    uno::Sequence aArgs{ comphelper::makePropertyValue(u"PersistentCopy"_ustr, uno::Any(false)) };
    dispatchCommand(mxComponent, u".uno:FormatPaintbrush"_ustr, aArgs);

    // Disable map mode, so that it's possible to send mouse event coordinates
    // directly in twips.
    SwEditWin& rEditWin = getSwDocShell()->GetView()->GetEditWin();
    rEditWin.EnableMapMode(false);

    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);
    SwShellCursor* pShellCursor = pWrtShell->getShellCursor(false);

    // move to cell A2
    pWrtShell->Down(/*bSelect=*/false, 1);
    Point aPoint = pShellCursor->GetSttPos();

    // click on cell A2
    SwXTextDocument* pTextDoc = getSwTextDoc();
    pTextDoc->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONDOWN, aPoint.getX(), aPoint.getY(), 1,
                             MOUSE_LEFT, 0);
    pTextDoc->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONUP, aPoint.getX(), aPoint.getY(), 1,
                             MOUSE_LEFT, 0);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(u"€ 100,00"_ustr, xCellA1->getString());

    // Without the fix in place, this test would have failed with
    // - Expected: € 100,00
    // - Actual  : 100
    CPPUNIT_ASSERT_EQUAL(u"€ 100,00"_ustr, xCellA2->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest9, testTdf161172)
{
    // Given a paragraph manually made a member of a list:
    createSwDoc("tdf161172.fodt");
    auto para = getParagraph(1);

    // Check initial state: the first paragraph has "No_list" para style, "Num_1" numbering style,
    // numbering level 0, and "Num1_lvl1_1" numbering label.
    CPPUNIT_ASSERT_EQUAL(u"No_list"_ustr, getProperty<OUString>(para, u"ParaStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Num_1"_ustr, getProperty<OUString>(para, u"NumberingStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Num1_lvl1_1"_ustr, getProperty<OUString>(para, u"ListLabelString"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), getProperty<sal_Int16>(para, u"NumberingLevel"_ustr));

    // Assign "Num_1_lvl2" paragraph style to the first paragraph. The style is associated with
    // "Num_1" numbering style, level 1.
    dispatchCommand(mxComponent, u".uno:StyleApply"_ustr,
                    { comphelper::makePropertyValue(u"FamilyName"_ustr, u"ParagraphStyles"_ustr),
                      comphelper::makePropertyValue(u"Style"_ustr, u"Num_1_lvl2"_ustr) });

    // Check that the respective properties got correctly applied
    CPPUNIT_ASSERT_EQUAL(u"Num_1_lvl2"_ustr, getProperty<OUString>(para, u"ParaStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Num_1"_ustr, getProperty<OUString>(para, u"NumberingStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Num1_lvl2_1"_ustr, getProperty<OUString>(para, u"ListLabelString"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), getProperty<sal_Int16>(para, u"NumberingLevel"_ustr));

    // Undo
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    // Check that the numbering properties got correctly restored
    CPPUNIT_ASSERT_EQUAL(u"No_list"_ustr, getProperty<OUString>(para, u"ParaStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Num_1"_ustr, getProperty<OUString>(para, u"NumberingStyleName"_ustr));
    // Without the fix, this would fail with
    // - Expected: Num1_lvl1_1
    // - Actual  : Num1_lvl2_1
    CPPUNIT_ASSERT_EQUAL(u"Num1_lvl1_1"_ustr, getProperty<OUString>(para, u"ListLabelString"_ustr));
    // Without the fix, this would fail with
    // - Expected: 0
    // - Actual  : 1
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), getProperty<sal_Int16>(para, u"NumberingLevel"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest9, testTdf144752)
{
    // Undoing/redoing a replacement must select the new text
    createSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    emulateTyping(u"Some Text");
    CPPUNIT_ASSERT(!pWrtShell->HasSelection());
    // Select "Text", and replace with "Word"
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect*/ true, 4, /*bBasicCall*/ false);
    pWrtShell->Replace(u"Word"_ustr, false);
    pWrtShell->EndOfSection();
    CPPUNIT_ASSERT(!pWrtShell->HasSelection());

    // Undo and check, that the "Text" is selected
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    // Without the fix, this would fail
    CPPUNIT_ASSERT(pWrtShell->HasSelection());
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, pWrtShell->GetSelText());

    // Redo and check, that the "Word" is selected
    dispatchCommand(mxComponent, u".uno:Redo"_ustr, {});
    CPPUNIT_ASSERT(pWrtShell->HasSelection());
    CPPUNIT_ASSERT_EQUAL(u"Word"_ustr, pWrtShell->GetSelText());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest9, testTdf162326_Paragraph)
{
    createSwDoc("tdf162326.odt");
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD,
                         getProperty<float>(getRun(getParagraph(1), 1), u"CharWeight"_ustr));
    CPPUNIT_ASSERT_EQUAL(
        awt::FontSlant_ITALIC,
        getProperty<awt::FontSlant>(getRun(getParagraph(2), 2), u"CharPosture"_ustr));
    CPPUNIT_ASSERT_EQUAL(short(1),
                         getProperty<short>(getRun(getParagraph(3), 2), u"CharUnderline"_ustr));

    pWrtShell->Down(/*bSelect=*/true, 3);

    dispatchCommand(mxComponent, u".uno:StyleApply"_ustr,
                    { comphelper::makePropertyValue(u"FamilyName"_ustr, u"ParagraphStyles"_ustr),
                      comphelper::makePropertyValue(u"Style"_ustr, u"Footnote"_ustr),
                      comphelper::makePropertyValue(u"KeyModifier"_ustr, uno::Any(KEY_MOD1)) });

    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::NORMAL,
                         getProperty<float>(getRun(getParagraph(1), 1), u"CharWeight"_ustr));
    CPPUNIT_ASSERT_THROW(getRun(getParagraph(2), 2), css::container::NoSuchElementException);
    CPPUNIT_ASSERT_THROW(getRun(getParagraph(3), 2), css::container::NoSuchElementException);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest9, testTdf162326_Character)
{
    createSwDoc("tdf162326.odt");
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD,
                         getProperty<float>(getRun(getParagraph(1), 1), u"CharWeight"_ustr));
    CPPUNIT_ASSERT_EQUAL(
        awt::FontSlant_ITALIC,
        getProperty<awt::FontSlant>(getRun(getParagraph(2), 2), u"CharPosture"_ustr));
    CPPUNIT_ASSERT_EQUAL(short(1),
                         getProperty<short>(getRun(getParagraph(3), 2), u"CharUnderline"_ustr));

    pWrtShell->Down(/*bSelect=*/true, 3);

    //add Ctrl/MOD_1
    dispatchCommand(mxComponent, u".uno:StyleApply"_ustr,
                    { comphelper::makePropertyValue(u"FamilyName"_ustr, u"CharacterStyles"_ustr),
                      comphelper::makePropertyValue(u"Style"_ustr, u"Definition"_ustr),
                      comphelper::makePropertyValue(u"KeyModifier"_ustr, uno::Any(KEY_MOD1)) });

    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::NORMAL,
                         getProperty<float>(getRun(getParagraph(1), 1), u"CharWeight"_ustr));
    CPPUNIT_ASSERT_THROW(getRun(getParagraph(2), 2), css::container::NoSuchElementException);
    //last runs are not changed because the selection ends at the beginning of that paragraph
    CPPUNIT_ASSERT_EQUAL(short(1),
                         getProperty<short>(getRun(getParagraph(3), 2), u"CharUnderline"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest9, testTdf162326_List)
{
    createSwDoc("tdf162326_list.odt");
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XParagraphCursor> xParaCursor(xTextDocument->getText()->createTextCursor(),
                                                       uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(u"A)"_ustr, getProperty<OUString>(xParaCursor, u"ListLabelString"_ustr));

    dispatchCommand(mxComponent, u".uno:StyleApply"_ustr,
                    { comphelper::makePropertyValue(u"FamilyName"_ustr, u"ParagraphStyles"_ustr),
                      comphelper::makePropertyValue(u"Style"_ustr, u"Footnote"_ustr) });

    //hard list attribute unchanged
    CPPUNIT_ASSERT_EQUAL(u"A)"_ustr, getProperty<OUString>(xParaCursor, u"ListLabelString"_ustr));

    dispatchCommand(mxComponent, u".uno:StyleApply"_ustr,
                    { comphelper::makePropertyValue(u"FamilyName"_ustr, u"ParagraphStyles"_ustr),
                      comphelper::makePropertyValue(u"Style"_ustr, u"Footnote"_ustr),
                      comphelper::makePropertyValue(u"KeyModifier"_ustr, uno::Any(KEY_MOD1)) });

    //list replaced by para style list setting
    CPPUNIT_ASSERT_EQUAL(u"1."_ustr, getProperty<OUString>(xParaCursor, u"ListLabelString"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest9, testTdf163340)
{
    createSwDoc("tdf163340.odt");
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<view::XSelectionSupplier> xSelSupplier(xModel->getCurrentController(),
                                                          uno::UNO_QUERY_THROW);
    uno::Reference<text::XParagraphCursor> xParaCursor(xTextDocument->getText()->createTextCursor(),
                                                       uno::UNO_QUERY);

    for (int i = 0; i < 14; i++)
        xParaCursor->gotoNextParagraph(false);
    xParaCursor->gotoEndOfParagraph(true);
    xSelSupplier->select(uno::Any(xParaCursor));

    CPPUNIT_ASSERT_EQUAL(u"A."_ustr, getProperty<OUString>(xParaCursor, u"ListLabelString"_ustr));
    dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});

    xParaCursor = uno::Reference<text::XParagraphCursor>(xText->createTextCursor(), uno::UNO_QUERY);
    for (int i = 0; i < 3; i++)
        xParaCursor->gotoNextParagraph(false);
    xParaCursor->gotoEndOfParagraph(true);
    CPPUNIT_ASSERT_EQUAL(u"1."_ustr, getProperty<OUString>(xParaCursor, u"ListLabelString"_ustr));
    xSelSupplier->select(uno::Any(xParaCursor));
    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(u"A."_ustr, getProperty<OUString>(xParaCursor, u"ListLabelString"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest9, testTdf163340_2)
{
    //selects and copies a single paragraph with a list (bullets)
    //and pastes it into an empty paragraph with a different list (numbers)
    //checks that the resulting paragraph keeps that different list
    createSwDoc("tdf163340_2.odt");
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<view::XSelectionSupplier> xSelSupplier(xModel->getCurrentController(),
                                                          uno::UNO_QUERY_THROW);
    uno::Reference<text::XParagraphCursor> xParaCursor(xTextDocument->getText()->createTextCursor(),
                                                       uno::UNO_QUERY);

    for (int i = 0; i < 2; i++)
        xParaCursor->gotoNextParagraph(false);
    xParaCursor->gotoEndOfParagraph(true);
    xSelSupplier->select(uno::Any(xParaCursor));

    xParaCursor = uno::Reference<text::XParagraphCursor>(xText->createTextCursor(), uno::UNO_QUERY);
    for (int i = 0; i < 10; i++)
        xParaCursor->gotoNextParagraph(false);
    xParaCursor->gotoEndOfParagraph(true);
    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(u"5."_ustr, getProperty<OUString>(xParaCursor, u"ListLabelString"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest9, testTdf159023)
{
    createSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();

    SwInsertTableOptions aTableOptions(SwInsertTableFlags::DefaultBorder, 0);
    pWrtShell->InsertTable(aTableOptions, /*nRows=*/2, /*nCols=*/2);
    pWrtShell->MoveTable(GotoPrevTable, fnTableStart);

    dispatchCommand(mxComponent, u".uno:SelectTable"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});

    pWrtShell->InsertFootnote(u""_ustr);
    CPPUNIT_ASSERT(pWrtShell->IsCursorInFootnote());

    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});
    dispatchCommand(mxComponent, u".uno:GoLeft"_ustr, {});
    dispatchCommand(mxComponent, u".uno:GoLeft"_ustr, {});

    // Without the fix in place, this test would have crashed here
    CPPUNIT_ASSERT(pWrtShell->IsCursorInFootnote());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest9, testTdf162195)
{
    // Given a document, which has some index entries in a hidden section
    createSwDoc("IndexElementsInHiddenSections.fodt");

    auto xIndexSupplier(mxComponent.queryThrow<css::text::XDocumentIndexesSupplier>());
    auto xIndexes = xIndexSupplier->getDocumentIndexes();
    CPPUNIT_ASSERT(xIndexes);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexes->getCount()); // A ToC and a table index

    auto xToC(xIndexes->getByIndex(0).queryThrow<css::text::XDocumentIndex>());
    xToC->update();
    // Without the fix, all the elements from the hidden section appeared in the index
    CPPUNIT_ASSERT_EQUAL(u"Table of Contents" SAL_NEWLINE_STRING "Section Visible\t1"_ustr,
                         xToC->getAnchor()->getString());

    auto xTables(xIndexes->getByIndex(1).queryThrow<css::text::XDocumentIndex>());
    xTables->update();
    // Without the fix, all the elements from the hidden section appeared in the index
    CPPUNIT_ASSERT_EQUAL(u"Index of Tables" SAL_NEWLINE_STRING "Table1\t1"_ustr,
                         xTables->getAnchor()->getString());

    // Show the hidden section
    auto xTextSectionsSupplier = mxComponent.queryThrow<css::text::XTextSectionsSupplier>();
    auto xSections = xTextSectionsSupplier->getTextSections();
    CPPUNIT_ASSERT(xSections);
    auto xSection
        = xSections->getByName(u"Section Hidden"_ustr).queryThrow<css::beans::XPropertySet>();
    xSection->setPropertyValue(u"IsVisible"_ustr, css::uno::Any(true));

    xToC->update();
    CPPUNIT_ASSERT_EQUAL(u"Table of Contents" SAL_NEWLINE_STRING
                         "Section Visible\t1" SAL_NEWLINE_STRING
                         "Section Hidden\t1" SAL_NEWLINE_STRING "entry\t1" SAL_NEWLINE_STRING
                         "CustomTOCStyle paragraph\t1"_ustr,
                         xToC->getAnchor()->getString());

    xTables->update();
    CPPUNIT_ASSERT_EQUAL(u"Index of Tables" SAL_NEWLINE_STRING "Table1\t1" SAL_NEWLINE_STRING
                         "Table2\t1"_ustr,
                         xTables->getAnchor()->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest9, testTdf164140)
{
    createSwDoc("tdf164140.fodt");
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();

    SwTextFrame& pTextFrame
        = dynamic_cast<SwTextFrame&>(*pWrtShell->GetLayout()->GetLower()->GetLower()->GetLower());
    const SwScriptInfo* pSI = pTextFrame.GetScriptInfo();

    // Prior to editing, there should be no kashida
    auto stBeforeKashida = pSI->GetKashidaPositions();
    CPPUNIT_ASSERT_EQUAL(size_t(0), stBeforeKashida.size());

    // Insert text at the beginning of the document
    pWrtShell->Insert(u"A"_ustr);

    // After editing, there should still be no room for kashida
    // Without the fix, this will be non-zero
    auto stAfterKashida = pSI->GetKashidaPositions();
    CPPUNIT_ASSERT_EQUAL(size_t(0), stAfterKashida.size());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest9, testTdf71583)
{
    //create a document, multiple pages, some restart page counting
    // insert page count in section fields and check the calculated
    // values

    auto insertParagraphAndBreak = [](sal_Int32& nParagraph, SwWrtShell& rWrtSh,
                                      uno::Reference<lang::XComponent> xComponent,
                                      const sal_uInt16 nPages) {
        for (sal_uInt16 nPage = 0; nPage < nPages; ++nPage)
        {
            rWrtSh.Insert(u"Paragraph "_ustr);
            rWrtSh.Insert(OUString::number(nParagraph));
            ++nParagraph;
            auto xModel(xComponent.queryThrow<frame::XModel>());
            auto xFactory(xComponent.queryThrow<lang::XMultiServiceFactory>());
            auto xTextViewCursorSupplier(
                xModel->getCurrentController().queryThrow<text::XTextViewCursorSupplier>());
            auto xTextRangeCursor(
                xTextViewCursorSupplier->getViewCursor().queryThrow<text::XTextRange>());

            uno::Reference<text::XTextField> xTextField(
                xFactory->createInstance(u"com.sun.star.text.TextField.PageCountRange"_ustr),
                uno::UNO_QUERY);
            xTextRangeCursor->getText()->insertTextContent(xTextRangeCursor, xTextField, false);

            rWrtSh.SttEndDoc(false);
            rWrtSh.SplitNode();
            if (nPage < nPages - 1)
                rWrtSh.InsertPageBreak();
            else
            {
                std::optional<sal_uInt16> oPageNumber = 1;
                OUString sPageStyle(u"Default Page Style"_ustr);
                rWrtSh.InsertPageBreak(&sPageStyle, oPageNumber);
            }
        }
    };

    auto checkDocument = [this]() {
        auto checkFieldContent = [this](uno::Reference<text::XText>& xBodyText, sal_uInt16 nPara,
                                        rtl::OUString sSymbol) {
            uno::Reference<text::XTextRange> xPara(getParagraphOfText(nPara, xBodyText));
            const uno::Reference<text::XTextRange> xRun = getRun(xPara, 2);
            uno::Reference<text::XTextField> xTextField
                = getProperty<uno::Reference<text::XTextField>>(xRun, u"TextField"_ustr);
            OUString sPresentation = xTextField->getPresentation(false);
            CPPUNIT_ASSERT_EQUAL(sSymbol, sPresentation);
        };

        auto xModel(mxComponent.queryThrow<text::XTextDocument>());
        uno::Reference<text::XText> xBodyText = xModel->getText();
        OUString sCompare = u"B"_ustr;

        for (sal_uInt16 nPara = 1; nPara < 25; nPara += 2)
        {
            if (nPara == 5)
                sCompare = u"D"_ustr;
            if (nPara == 13)
                sCompare = u"F"_ustr;
            checkFieldContent(xBodyText, nPara, sCompare);
        }
    };
    createSwDoc();
    SwWrtShell* pWrtShell = nullptr;
    {
        pWrtShell = getSwDocShell()->GetWrtShell();
        CPPUNIT_ASSERT(pWrtShell);

        sal_Int32 nParagraph = 0;
        for (int i = 1; i <= 3; ++i)
            insertParagraphAndBreak(nParagraph, *pWrtShell, mxComponent, i * 2);
        checkDocument();
    }

    {
        saveAndReload(u"writer8"_ustr);
        checkDocument();
    }
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
