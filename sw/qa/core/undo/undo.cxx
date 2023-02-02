/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <unotools/mediadescriptor.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>

#include <unotxdoc.hxx>
#include <docsh.hxx>
#include <wrtsh.hxx>
#include <swdtflvr.hxx>
#include <frameformats.hxx>
#include <fmtcntnt.hxx>
#include <view.hxx>
#include <cmdid.h>

/// Covers sw/source/core/undo/ fixes.
class SwCoreUndoTest : public SwModelTestBase
{
public:
    SwCoreUndoTest()
        : SwModelTestBase("/sw/qa/core/undo/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(SwCoreUndoTest, testTextboxCutSave)
{
    // Load the document and select all.
    createSwDoc("textbox-cut-save.docx");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwDocShell* pDocShell = pTextDoc->GetDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->SelAll();

    // Cut.
    rtl::Reference<SwTransferable> pTransfer = new SwTransferable(*pWrtShell);
    pTransfer->Cut();

    // Undo.
    pWrtShell->Undo();

    // Save.
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("Office Open XML Text");

    // Without the accompanying fix in place, this test would have failed with:
    // void sax_fastparser::FastSaxSerializer::endDocument(): Assertion `mbMarkStackEmpty && maMarkStack.empty()' failed.
    // i.e. failed to save because we tried to write not-well-formed XML.
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
}

CPPUNIT_TEST_FIXTURE(SwCoreUndoTest, testTextboxCutUndo)
{
    createSwDoc("textbox-cut-undo.docx");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwDocShell* pDocShell = pTextDoc->GetDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    SwDoc* pDoc = pDocShell->GetDoc();
    SwView* pView = pDoc->GetDocShell()->GetView();

    pView->GetViewFrame()->GetDispatcher()->Execute(FN_CNTNT_TO_NEXT_FRAME, SfxCallMode::SYNCHRON);
    pView->StopShellTimer();
    rtl::Reference<SwTransferable> pTransfer = new SwTransferable(*pWrtShell);
    pTransfer->Cut();
    SwFrameFormats& rSpzFrameFormats = *pDoc->GetSpzFrameFormats();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), rSpzFrameFormats.size());

    pWrtShell->Undo();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), rSpzFrameFormats.size());

    const SwNodeIndex* pIndex1 = rSpzFrameFormats[0]->GetContent().GetContentIdx();
    const SwNodeIndex* pIndex2 = rSpzFrameFormats[1]->GetContent().GetContentIdx();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 5
    // - Actual  : 8
    // i.e. the draw frame format had a wrong node index in its content.
    CPPUNIT_ASSERT_EQUAL(pIndex1->GetIndex(), pIndex2->GetIndex());
}

CPPUNIT_TEST_FIXTURE(SwCoreUndoTest, testTableCopyRedline)
{
    // Given a document with two table cells and redlining enabled:
    createSwDoc("table-copy-redline.odt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwDocShell* pDocShell = pTextDoc->GetDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();

    // When doing select-all, copy, paste and undo:
    pWrtShell->SelAll();
    rtl::Reference<SwTransferable> pTransfer = new SwTransferable(*pWrtShell);
    pTransfer->Copy();
    TransferableDataHelper aHelper(pTransfer);
    SwTransferable::Paste(*pWrtShell, aHelper);

    // Without the accompanying fix in place, this test would have crashed.
    pWrtShell->Undo();
}

CPPUNIT_TEST_FIXTURE(SwCoreUndoTest, testImagePropsCreateUndoAndModifyDoc)
{
    createSwDoc("image-as-character.odt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwDocShell* pDocShell = pTextDoc->GetDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    css::uno::Reference<css::beans::XPropertySet> xImage(
        pTextDoc->getGraphicObjects()->getByName("Image1"), css::uno::UNO_QUERY_THROW);

    CPPUNIT_ASSERT(pTextDoc->isSetModifiedEnabled());
    CPPUNIT_ASSERT(!pTextDoc->isModified());
    CPPUNIT_ASSERT(!pWrtShell->GetLastUndoInfo(nullptr, nullptr, nullptr));

    // Check that modifications of the geometry mark document dirty, and create an undo

    xImage->setPropertyValue("RelativeWidth", css::uno::Any(sal_Int16(80)));

    // Without the fix, this would fail
    CPPUNIT_ASSERT(pTextDoc->isModified());
    CPPUNIT_ASSERT(pWrtShell->GetLastUndoInfo(nullptr, nullptr, nullptr));

    pWrtShell->Undo();
    CPPUNIT_ASSERT(!pTextDoc->isModified());
    CPPUNIT_ASSERT(!pWrtShell->GetLastUndoInfo(nullptr, nullptr, nullptr));

    // Check that modifications of anchor mark document dirty, and create an undo

    xImage->setPropertyValue("AnchorType",
                             css::uno::Any(css::text::TextContentAnchorType_AT_PARAGRAPH));

    CPPUNIT_ASSERT(pTextDoc->isModified());
    CPPUNIT_ASSERT(pWrtShell->GetLastUndoInfo(nullptr, nullptr, nullptr));

    pWrtShell->Undo();
    CPPUNIT_ASSERT(!pTextDoc->isModified());
    CPPUNIT_ASSERT(!pWrtShell->GetLastUndoInfo(nullptr, nullptr, nullptr));

    // Check that setting the same values do not make it dirty and do not add undo

    xImage->setPropertyValue("RelativeWidth", xImage->getPropertyValue("RelativeWidth"));
    xImage->setPropertyValue("AnchorType", xImage->getPropertyValue("AnchorType"));

    CPPUNIT_ASSERT(!pTextDoc->isModified());
    CPPUNIT_ASSERT(!pWrtShell->GetLastUndoInfo(nullptr, nullptr, nullptr));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
