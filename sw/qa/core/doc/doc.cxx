/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <comphelper/classids.hxx>
#include <tools/globname.hxx>
#include <svtools/embedhlp.hxx>
#include <editeng/frmdiritem.hxx>
#include <vcl/errinf.hxx>
#include <vcl/event.hxx>
#include <editeng/langitem.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <vcl/scheduler.hxx>

#include <wrtsh.hxx>
#include <fmtanchr.hxx>
#include <frameformats.hxx>
#include <docsh.hxx>
#include <edtwin.hxx>
#include <view.hxx>
#include <ndtxt.hxx>
#include <swdtflvr.hxx>
#include <cmdid.h>
#include <unotxdoc.hxx>
#include <UndoManager.hxx>

constexpr OUStringLiteral DATA_DIRECTORY = u"/sw/qa/core/doc/data/";

/// Covers sw/source/core/doc/ fixes.
class SwCoreDocTest : public SwModelTestBase
{
};

CPPUNIT_TEST_FIXTURE(SwCoreDocTest, testMathInsertAnchorType)
{
    // Given an empty document.
    SwDoc* pDoc = createSwDoc();

    // When inserting an a math object.
    SwWrtShell* pShell = pDoc->GetDocShell()->GetWrtShell();
    SvGlobalName aGlobalName(SO3_SM_CLASSID);
    pShell->InsertObject(svt::EmbeddedObjectRef(), &aGlobalName);

    // Then the anchor type should be as-char.
    SwFrameFormats& rFormats = *pDoc->GetSpzFrameFormats();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rFormats.size());
    const SwFrameFormat& rFormat = *rFormats[0];
    const SwFormatAnchor& rAnchor = rFormat.GetAnchor();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 4
    // i.e. the anchor type was at-char, not as-char.
    CPPUNIT_ASSERT_EQUAL(RndStdIds::FLY_AS_CHAR, rAnchor.GetAnchorId());
    ErrorRegistry::Reset();
}

CPPUNIT_TEST_FIXTURE(SwCoreDocTest, testTextboxTextRotateAngle)
{
    // Check the writing direction of the only TextFrame in the document.
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "textbox-textrotateangle.odt");
    SwFrameFormats& rFrameFormats = *pDoc->GetSpzFrameFormats();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), rFrameFormats.size());
    CPPUNIT_ASSERT_EQUAL(o3tl::narrowing<sal_uInt16>(RES_DRAWFRMFMT), rFrameFormats[0]->Which());
    CPPUNIT_ASSERT_EQUAL(o3tl::narrowing<sal_uInt16>(RES_FLYFRMFMT), rFrameFormats[1]->Which());
    SvxFrameDirection eActual = rFrameFormats[1]->GetAttrSet().GetItem(RES_FRAMEDIR)->GetValue();

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 5 (btlr)
    // - Actual  : 0 (lrtb)
    // i.e. the writing direction was in the ODT file, but it was lost on import in the textbox
    // case.
    CPPUNIT_ASSERT_EQUAL(SvxFrameDirection::Vertical_LR_BT, eActual);
    ErrorRegistry::Reset();
}

CPPUNIT_TEST_FIXTURE(SwCoreDocTest, testNumDownIndent)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "num-down-indent.docx");
    SwDocShell* pDocShell = pDoc->GetDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->Down(/*bSelect=*/false);
    SwEditWin& rEditWin = pDocShell->GetView()->GetEditWin();
    KeyEvent aKeyEvent(0, KEY_TAB);
    rEditWin.KeyInput(aKeyEvent);
    SwTextNode* pTextNode = pWrtShell->GetCursor()->GetNode().GetTextNode();

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: \tB
    // - Actual  : B
    // i.e. pressing <tab> at the start of the paragraph did not change the layout.
    CPPUNIT_ASSERT_EQUAL(OUString("\tB"), pTextNode->GetText());
    ErrorRegistry::Reset();
}

CPPUNIT_TEST_FIXTURE(SwCoreDocTest, testLocaleIndependentTemplate)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "locale-independent-template.odt");
    SwDocShell* pDocShell = pDoc->GetDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    SfxItemSet aSet(pWrtShell->GetAttrPool(), svl::Items<RES_CHRATR_LANGUAGE, RES_CHRATR_LANGUAGE>);
    pWrtShell->GetCurAttr(aSet);
    const SvxLanguageItem* pItem = aSet.GetItem(RES_CHRATR_LANGUAGE);
    CPPUNIT_ASSERT(pItem);
    LanguageType eLang = pItem->GetValue();

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1033 (LANGUAGE_ENGLISH_US)
    // - Actual  : 1023 (LANGUAGE_DONTKNOW)
    // i.e. the status bar and the format -> character dialog didn't fall back to the UI locale when
    // an explicit language was not set for the document.
    CPPUNIT_ASSERT_EQUAL(LANGUAGE_ENGLISH_US, eLang);
    ErrorRegistry::Reset();
}

CPPUNIT_TEST_FIXTURE(SwCoreDocTest, testTextBoxZOrder)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "textbox-zorder.docx");
    SwFrameFormats& rFormats = *pDoc->GetSpzFrameFormats();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), rFormats.size());
    const SwFrameFormat* pEllipse = rFormats[2];
    const SdrObject* pEllipseShape = pEllipse->FindRealSdrObject();
    // Make sure we test the right shape.
    CPPUNIT_ASSERT_EQUAL(OUString("Shape3"), pEllipseShape->GetName());
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2
    // - Actual  : 1
    // i.e. the ellipse was under the frame of the shape-frame pair, not on top of it.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(2), pEllipseShape->GetOrdNum());
}

CPPUNIT_TEST_FIXTURE(SwCoreDocTest, testTextBoxMakeFlyFrame)
{
    // Given a document with an as-char textbox (as-char draw format + at-char fly format):
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "textbox-makeflyframe.docx");

    // When cutting the textbox and pasting it to a new document:
    SwView* pView = pDoc->GetDocShell()->GetView();
    pView->GetViewFrame()->GetDispatcher()->Execute(FN_CNTNT_TO_NEXT_FRAME, SfxCallMode::SYNCHRON);
    pView->StopShellTimer();
    SwDocShell* pDocShell = pDoc->GetDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    rtl::Reference<SwTransferable> pTransfer = new SwTransferable(*pWrtShell);
    pTransfer->Cut();
    TransferableDataHelper aHelper(pTransfer);
    uno::Reference<lang::XComponent> xDoc2
        = loadFromDesktop("private:factory/swriter", "com.sun.star.text.TextDocument", {});
    SwXTextDocument* pTextDoc2 = dynamic_cast<SwXTextDocument*>(xDoc2.get());
    SwDocShell* pDocShell2 = pTextDoc2->GetDocShell();
    SwWrtShell* pWrtShell2 = pDocShell2->GetWrtShell();
    SwTransferable::Paste(*pWrtShell2, aHelper);

    // Then make sure its fly frame is created.
    mxComponent->dispose();
    mxComponent = xDoc2;
    xmlDocUniquePtr pLayout = parseLayoutDump();
    // Without the accompanying fix in place, this test would have failed, because the first text
    // frame in the body frame had an SwAnchoredDrawObject anchored to it, but not a fly frame, so
    // a blank square was painted, not the image.
    assertXPath(pLayout, "/root/page/body/txt/anchored/fly", 1);
}

CPPUNIT_TEST_FIXTURE(SwCoreDocTest, testIMEGrouping)
{
// TODO figure out why the ext text input in this test code reaches the wrong window on
// non-headless.
#if !defined MACOSX && !defined _WIN32
    // Given an empty document:
    SwDoc* pDoc = createSwDoc();
    // Make sure no idle is in action, so the ExtTextInput events go to SwEditWin.
    Scheduler::ProcessEventsToIdle();

    // When pressing two keys via IME:
    SwDocShell* pDocShell = pDoc->GetDocShell();
    SwEditWin& rEditWin = pDocShell->GetView()->GetEditWin();
    rEditWin.PostExtTextInputEvent(VclEventId::ExtTextInput, "a");
    rEditWin.PostExtTextInputEvent(VclEventId::EndExtTextInput, "");
    rEditWin.PostExtTextInputEvent(VclEventId::ExtTextInput, "b");
    rEditWin.PostExtTextInputEvent(VclEventId::EndExtTextInput, "");

    // Then make sure that gets grouped together to a single undo action:
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    SwTextNode* pTextNode = pWrtShell->GetCursor()->GetNode().GetTextNode();
    CPPUNIT_ASSERT_EQUAL(OUString("ab"), pTextNode->GetText());
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 2
    // i.e. 2 subsequent IME events got their own undo actions.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pDoc->GetUndoManager().GetUndoActionCount());
#endif
}

CPPUNIT_TEST_FIXTURE(SwCoreDocTest, testImageHyperlinkStyle)
{
    // Given a document with an image with a hyperlink:
    loadURL("private:factory/swriter", nullptr);
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    uno::Reference<text::XTextContent> xImage(
        xFactory->createInstance("com.sun.star.text.TextGraphicObject"), uno::UNO_QUERY);
    xText->insertTextContent(xCursor, xImage, /*bAbsorb=*/false);
    uno::Reference<beans::XPropertySet> xImageProps(xImage, uno::UNO_QUERY);
    OUString aExpected = "http://www.example.com";
    xImageProps->setPropertyValue("HyperLinkURL", uno::Any(aExpected));

    // When applying a frame style on it:
    xImageProps->setPropertyValue("FrameStyleName", uno::Any(OUString("Frame")));

    // Then make sure that the hyperlink is not lost:
    auto aActual = getProperty<OUString>(xImageProps, "HyperLinkURL");
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: http://www.example.com
    // - Actual  :
    // i.e. the link was lost, even if the frame style dialog doesn't allow specifying a link on
    // frames.
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
