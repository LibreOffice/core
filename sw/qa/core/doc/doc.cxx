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

char const DATA_DIRECTORY[] = "/sw/qa/core/doc/data/";

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
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(RES_DRAWFRMFMT), rFrameFormats[0]->Which());
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(RES_FLYFRMFMT), rFrameFormats[1]->Which());
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
    SfxItemSet aSet(pWrtShell->GetAttrPool(), { { RES_CHRATR_LANGUAGE, RES_CHRATR_LANGUAGE } });
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
    TransferableDataHelper aHelper(pTransfer.get());
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

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
