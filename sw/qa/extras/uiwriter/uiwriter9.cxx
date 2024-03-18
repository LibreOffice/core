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
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <fmtinfmt.hxx>
#include <rootfrm.hxx>

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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest9, testTdf159816)
{
    createSwDoc();

    SwDoc* pDoc = getSwDoc();
    CPPUNIT_ASSERT(pDoc);
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    // Add 5 empty paragraphs
    pWrtShell->SplitNode();
    pWrtShell->SplitNode();
    pWrtShell->SplitNode();
    pWrtShell->SplitNode();
    pWrtShell->SplitNode();

    // Add a bookmark at the very end
    IDocumentMarkAccess& rIDMA(*pDoc->getIDocumentMarkAccess());
    rIDMA.makeMark(*pWrtShell->GetCursor(), "Mark", IDocumentMarkAccess::MarkType::BOOKMARK,
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

CPPUNIT_TEST_FIXTURE(SwUiWriterTest9, testTdf159054_disableOutlineNumbering)
{
    createSwDoc("tdf159054_disableOutlineNumbering.docx");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    const uno::Reference<text::XTextRange> xPara1 = getParagraph(1, "Heading A");
    const uno::Reference<text::XTextRange> xPara2 = getParagraph(2, "Heading B");
    const uno::Reference<text::XTextRange> xPara3 = getParagraph(3, "Heading C");

    CPPUNIT_ASSERT_EQUAL(OUString("A."), getProperty<OUString>(xPara1, "ListLabelString"));
    CPPUNIT_ASSERT_EQUAL(OUString("B."), getProperty<OUString>(xPara2, "ListLabelString"));
    CPPUNIT_ASSERT_EQUAL(OUString("C."), getProperty<OUString>(xPara3, "ListLabelString"));

    // select (at least parts) of the first two paragraphs
    pWrtShell->Down(/*bSelect=*/true, /*nCount=*/1, /*bBasicCall=*/true);

    // on the selection, simulate pressing the toolbar button to toggle OFF numbering
    dispatchCommand(mxComponent, ".uno:DefaultNumbering", {});

    // the selected paragraphs should definitely have the list label removed
    CPPUNIT_ASSERT_EQUAL(OUString(""), getProperty<OUString>(xPara1, "ListLabelString"));
    CPPUNIT_ASSERT_EQUAL(OUString(""), getProperty<OUString>(xPara2, "ListLabelString"));
    // the third paragraph must retain the existing numbering format
    CPPUNIT_ASSERT_EQUAL(OUString("A."), getProperty<OUString>(xPara3, "ListLabelString"));

    // on the selection, simulate pressing the toolbar button to toggle ON numbering again
    dispatchCommand(mxComponent, ".uno:DefaultNumbering", {});

    // the outline numbering format must be re-applied to the first two paragraphs
    CPPUNIT_ASSERT_EQUAL(OUString("A."), getProperty<OUString>(xPara1, "ListLabelString"));
    CPPUNIT_ASSERT_EQUAL(OUString("B."), getProperty<OUString>(xPara2, "ListLabelString"));
    CPPUNIT_ASSERT_EQUAL(OUString("C."), getProperty<OUString>(xPara3, "ListLabelString"));

    // on the selection, simulate a right click - list - No list
    dispatchCommand(mxComponent, ".uno:RemoveBullets", {});

    // the selected paragraphs should definitely have the list label removed
    CPPUNIT_ASSERT_EQUAL(OUString(""), getProperty<OUString>(xPara1, "ListLabelString"));
    CPPUNIT_ASSERT_EQUAL(OUString(""), getProperty<OUString>(xPara2, "ListLabelString"));
    CPPUNIT_ASSERT_EQUAL(OUString("A."), getProperty<OUString>(xPara3, "ListLabelString"));
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
