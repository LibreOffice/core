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

#include <unotxdoc.hxx>
#include <docsh.hxx>
#include <wrtsh.hxx>
#include <swdtflvr.hxx>
#include <frameformats.hxx>
#include <fmtcntnt.hxx>

/// Covers sw/source/core/undo/ fixes.
class SwCoreUndoTest : public SwModelTestBase
{
public:
    SwCoreUndoTest()
        : SwModelTestBase(u"/sw/qa/core/undo/data/"_ustr)
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
    aMediaDescriptor[u"FilterName"_ustr] <<= u"Office Open XML Text"_ustr;

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

    selectShape(1);
    rtl::Reference<SwTransferable> pTransfer = new SwTransferable(*pWrtShell);
    pTransfer->Cut();
    auto& rSpzFrameFormats = *pDoc->GetSpzFrameFormats();
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
        pTextDoc->getGraphicObjects()->getByName(u"Image1"_ustr), css::uno::UNO_QUERY_THROW);

    CPPUNIT_ASSERT(pTextDoc->isSetModifiedEnabled());
    CPPUNIT_ASSERT(!pTextDoc->isModified());
    CPPUNIT_ASSERT(!pWrtShell->GetLastUndoInfo(nullptr, nullptr, nullptr));

    // Check that modifications of the geometry mark document dirty, and create an undo

    xImage->setPropertyValue(u"RelativeWidth"_ustr, css::uno::Any(sal_Int16(80)));

    // Without the fix, this would fail
    CPPUNIT_ASSERT(pTextDoc->isModified());
    CPPUNIT_ASSERT(pWrtShell->GetLastUndoInfo(nullptr, nullptr, nullptr));

    pWrtShell->Undo();
    CPPUNIT_ASSERT(!pTextDoc->isModified());
    CPPUNIT_ASSERT(!pWrtShell->GetLastUndoInfo(nullptr, nullptr, nullptr));

    // Check that modifications of anchor mark document dirty, and create an undo

    xImage->setPropertyValue(u"AnchorType"_ustr,
                             css::uno::Any(css::text::TextContentAnchorType_AT_PARAGRAPH));

    CPPUNIT_ASSERT(pTextDoc->isModified());
    CPPUNIT_ASSERT(pWrtShell->GetLastUndoInfo(nullptr, nullptr, nullptr));

    pWrtShell->Undo();
    CPPUNIT_ASSERT(!pTextDoc->isModified());
    CPPUNIT_ASSERT(!pWrtShell->GetLastUndoInfo(nullptr, nullptr, nullptr));

    // Check that setting the same values do not make it dirty and do not add undo

    xImage->setPropertyValue(u"RelativeWidth"_ustr,
                             xImage->getPropertyValue(u"RelativeWidth"_ustr));
    xImage->setPropertyValue(u"AnchorType"_ustr, xImage->getPropertyValue(u"AnchorType"_ustr));

    CPPUNIT_ASSERT(!pTextDoc->isModified());
    CPPUNIT_ASSERT(!pWrtShell->GetLastUndoInfo(nullptr, nullptr, nullptr));
}

CPPUNIT_TEST_FIXTURE(SwCoreUndoTest, testAnchorTypeChangePosition)
{
    // Given a document with a textbox (draw + fly format pair) + an inner image:
    createSwDoc("anchor-type-change-position.docx");
    selectShape(1);
    SwDoc* pDoc = getSwDoc();
    const auto& rFormats = *pDoc->GetSpzFrameFormats();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), rFormats.size());
    Point aOldPos;
    {
        const SwFormatHoriOrient& rHoriOrient = rFormats[0]->GetHoriOrient();
        const SwFormatVertOrient& rVertOrient = rFormats[0]->GetVertOrient();
        aOldPos = Point(rHoriOrient.GetPos(), rVertOrient.GetPos());
    }

    // When changing the anchor type + undo:
    dispatchCommand(mxComponent, u".uno:SetAnchorToChar"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    // Then make sure the old position is also restored:
    const SwFormatHoriOrient& rHoriOrient = rFormats[0]->GetHoriOrient();
    const SwFormatVertOrient& rVertOrient = rFormats[0]->GetVertOrient();
    Point aNewPos(rHoriOrient.GetPos(), rVertOrient.GetPos());
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 789,213
    // - Actual  : 1578,3425
    // i.e. there was a big, unexpected increase in the vertical position after undo.
    CPPUNIT_ASSERT_EQUAL(aOldPos, aNewPos);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
