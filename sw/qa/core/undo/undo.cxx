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
#include <cntfrm.hxx>
#include <fmtpdsc.hxx>
#include <pagefrm.hxx>
#include <rootfrm.hxx>

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
    SwDocShell* pDocShell = getSwDocShell();
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
    SwDocShell* pDocShell = getSwDocShell();
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
    SwDocShell* pDocShell = getSwDocShell();
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
    SwDocShell* pDocShell = getSwDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    SwXTextDocument* pTextDoc = getSwTextDoc();
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

CPPUNIT_TEST_FIXTURE(SwCoreUndoTest, testPageDescThatFollowsItself)
{
    createSwDoc();

    SwDoc* pDoc = getSwDoc();

    // Create a custom page desc that has its follow page desc set to itself
    // (that happens by default). This means that, if the current page uses this
    // page desc, then the next page will as well.
    SwPageDesc* pOldPageDesc = pDoc->MakePageDesc(UIName("customPageDesc"), nullptr, true);
    CPPUNIT_ASSERT(pOldPageDesc);
    CPPUNIT_ASSERT_EQUAL(pOldPageDesc, pOldPageDesc->GetFollow());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Redo"_ustr, {});

    // Without the fix, the follow page desc no longer matches, and the next page
    // would not use the correct page desc.
    SwPageDesc* pNewPageDesc = pDoc->FindPageDesc(UIName("customPageDesc"));
    CPPUNIT_ASSERT(pNewPageDesc);
    CPPUNIT_ASSERT_EQUAL(pNewPageDesc, pNewPageDesc->GetFollow());
}

CPPUNIT_TEST_FIXTURE(SwCoreUndoTest, testTdf148703CopyFooterAcrossDocuments)
{
    // A document containing some text to overwrite and no custom page styles.
    createSwDoc("tdf148703-dest.odt");

    // A document containing a custom page style, two paragraphs, and a footer.
    uno::Reference<css::lang::XComponent> xSrcComponent
        = loadFromDesktop(createFileURL(u"tdf148703-src.odt"), OUString(), {});

    dispatchCommand(xSrcComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(xSrcComponent, u".uno:Copy"_ustr, {});

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    // Without the fix, the Undo command crashes with:
    // SwUndoDelete::UndoImpl(sw::UndoRedoContext&): Assertion `pStartNode' failed.
    // That happens because the footer nodes remained in the SwNodes
    // after undo and messed up the index where the SwUndoDelete re-inserts
    // the deleted content.

    dispatchCommand(mxComponent, u".uno:Redo"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    // Clear the redo stack by doing any other operation, to test the whether
    // the destructor works.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertString(xCursor, u"test"_ustr, /*bAbsorb=*/false);
}

static const SwPageDesc* lcl_getLastPagePageDesc(SwDoc& rDoc)
{
    SwRootFrame* pRootFrame = rDoc.GetAllLayouts()[0];
    const SwPageFrame* pPageFrameIter = pRootFrame->GetLastPage();
    const SwContentFrame* pContentFrame = pPageFrameIter->FindFirstBodyContent();
    const SwFormatPageDesc& rFormatPageDesc = pContentFrame->GetPageDescItem();
    const sw::BroadcastingModify* pMod = rFormatPageDesc.GetDefinedIn();

    if (auto pContentNode = dynamic_cast<const SwContentNode*>(pMod))
        return pContentNode->GetAttr(RES_PAGEDESC).GetPageDesc();

    return nullptr;
}

CPPUNIT_TEST_FIXTURE(SwCoreUndoTest, testPageDescDelete)
{
    createSwDoc();

    SwDoc* pDoc = getSwDoc();

    // Create two custom page descs such that a page using customPageDesc1 would be followed
    // by a page containing customPageDesc2, and then set the second page of the document
    // to use customPageDesc2. (the first page just uses the default page desc)

    SwPageDesc* pOldPageDesc2 = pDoc->MakePageDesc(UIName("customPageDesc2"), nullptr, true);
    SwPageDesc* pPageDesc1 = pDoc->MakePageDesc(UIName("customPageDesc1"), nullptr, true);
    pPageDesc1->SetFollow(pOldPageDesc2);

    SwDocShell* pDocShell = getSwDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();

    UIName aName2("customPageDesc2");
    pWrtShell->InsertPageBreak(&aName2);

    CPPUNIT_ASSERT_EQUAL(lcl_getLastPagePageDesc(*pDoc),
                         const_cast<const SwPageDesc*>(pOldPageDesc2));

    pDoc->DelPageDesc(UIName("customPageDesc2"));

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    SwPageDesc* pNewPageDesc2 = pDoc->FindPageDesc(UIName("customPageDesc2"));

    // After deleting customPageDesc2 and undoing that delete, customPageDesc1 should still
    // have customPageDesc2 as its follow.

    CPPUNIT_ASSERT(pNewPageDesc2);
    CPPUNIT_ASSERT_EQUAL(pNewPageDesc2, pPageDesc1->GetFollow());

    // Deleting customPageDesc2 removed it from the second page of the document. Undoing that
    // delete should have re-added it to the second page.

    CPPUNIT_ASSERT_EQUAL(lcl_getLastPagePageDesc(*pDoc),
                         const_cast<const SwPageDesc*>(pNewPageDesc2));

    dispatchCommand(mxComponent, u".uno:Redo"_ustr, {});
}

CPPUNIT_TEST_FIXTURE(SwCoreUndoTest, testPageDescCreate)
{
    createSwDoc();

    SwDoc* pDoc = getSwDoc();

    // Create two custom page descs such that a page using customPageDesc1 would be followed
    // by a page containing customPageDesc2, and then set the second page of the document
    // to use customPageDesc2. (the first page just uses the default page desc)

    SwPageDesc* pOldPageDesc2 = pDoc->MakePageDesc(UIName("customPageDesc2"), nullptr, true);
    SwPageDesc* pOldPageDesc1 = pDoc->MakePageDesc(UIName("customPageDesc1"), nullptr, true);
    pOldPageDesc1->SetFollow(pOldPageDesc2);

    SwDocShell* pDocShell = getSwDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();

    UIName aName2("customPageDesc2");
    pWrtShell->InsertPageBreak(&aName2);

    CPPUNIT_ASSERT_EQUAL(lcl_getLastPagePageDesc(*pDoc),
                         const_cast<const SwPageDesc*>(pOldPageDesc2));

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {}); // undo insert page break
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {}); // undo create customPageDesc1
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {}); // undo create customPageDesc2
    dispatchCommand(mxComponent, u".uno:Redo"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Redo"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Redo"_ustr, {});

    SwPageDesc* pNewPageDesc1 = pDoc->FindPageDesc(UIName("customPageDesc1"));
    SwPageDesc* pNewPageDesc2 = pDoc->FindPageDesc(UIName("customPageDesc2"));

    // After undoing past the creation of both custom page descs, and then redoing to the present,
    // both page descs should exist and customPageDesc2 should still follow customPageDesc1.

    CPPUNIT_ASSERT(pNewPageDesc1);
    CPPUNIT_ASSERT(pNewPageDesc2);
    CPPUNIT_ASSERT_EQUAL(pNewPageDesc2, pNewPageDesc1->GetFollow());

    // Undoing the creation of customPageDesc2 removed it from the second page of the document. Redoing
    // should have re-added it to the second page.

    CPPUNIT_ASSERT_EQUAL(lcl_getLastPagePageDesc(*pDoc),
                         const_cast<const SwPageDesc*>(pNewPageDesc2));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
