/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/style/BreakType.hpp>
#include <com/sun/star/text/ControlCharacter.hpp>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/sequenceashashmap.hxx>

#include <unotxdoc.hxx>
#include <docsh.hxx>
#include <wrtsh.hxx>
#include <swdtflvr.hxx>
#include <frameformats.hxx>
#include <fmtcntnt.hxx>
#include <cntfrm.hxx>
#include <fmthdft.hxx>
#include <fmtpdsc.hxx>
#include <pagefrm.hxx>
#include <rootfrm.hxx>
#include <docstat.hxx>
#include <ndtxt.hxx>
#include <pam.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <redline.hxx>
#include <docary.hxx>

#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>

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
    comphelper::SequenceAsHashMap aMediaDescriptor;
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

namespace
{
/// Put the view cursor into the given cell of the given table.
void lcl_GotoCell(const uno::Reference<lang::XComponent>& xComponent, SwXTextDocument* pTextDoc,
                  const OUString& rTable, const OUString& rCell)
{
    uno::Reference<text::XTextTablesSupplier> xSupplier(xComponent, uno::UNO_QUERY_THROW);
    uno::Reference<text::XTextTable> xTable(xSupplier->getTextTables()->getByName(rTable),
                                            uno::UNO_QUERY_THROW);
    uno::Reference<text::XText> xCell(xTable->getCellByName(rCell), uno::UNO_QUERY_THROW);
    uno::Reference<text::XTextViewCursorSupplier> xCursorSupplier(pTextDoc->getCurrentController(),
                                                                  uno::UNO_QUERY_THROW);
    xCursorSupplier->getViewCursor()->gotoRange(xCell->getStart(), /*bExpand=*/false);
}

/// The text of the given cell of the given table.
OUString lcl_GetCellText(const uno::Reference<lang::XComponent>& xComponent, const OUString& rTable,
                         const OUString& rCell)
{
    uno::Reference<text::XTextTablesSupplier> xSupplier(xComponent, uno::UNO_QUERY_THROW);
    uno::Reference<text::XTextTable> xTable(xSupplier->getTextTables()->getByName(rTable),
                                            uno::UNO_QUERY_THROW);
    uno::Reference<text::XText> xCell(xTable->getCellByName(rCell), uno::UNO_QUERY_THROW);
    return xCell->getString();
}

/// The formula of the given cell of the given table, empty when it holds none.
OUString lcl_GetCellFormula(const uno::Reference<lang::XComponent>& xComponent,
                            const OUString& rTable, const OUString& rCell)
{
    uno::Reference<text::XTextTablesSupplier> xSupplier(xComponent, uno::UNO_QUERY_THROW);
    uno::Reference<text::XTextTable> xTable(xSupplier->getTextTables()->getByName(rTable),
                                            uno::UNO_QUERY_THROW);
    uno::Reference<table::XCell> xCell(xTable->getCellByName(rCell));
    CPPUNIT_ASSERT(xCell.is());
    return xCell->getFormula();
}

/// The tracked changes of the document, as a "<type>:<text>" list.
OUString lcl_GetRedlines(SwDoc* pDoc)
{
    OUString aRet;
    const SwRedlineTable& rTable = pDoc->getIDocumentRedlineAccess().GetRedlineTable();
    for (SwRedlineTable::size_type i = 0; i < rTable.size(); ++i)
    {
        if (!aRet.isEmpty())
            aRet += " ";
        aRet += SwRedlineTypeToOUString(rTable[i]->GetType()) + ":" + rTable[i]->GetText();
    }
    return aRet;
}

/// Every cursor position has to stay inside the text of the node it points at.
void lcl_AssertCursorInText(SwWrtShell* pWrtShell)
{
    for (const SwPaM& rPaM : pWrtShell->GetCursor()->GetRingContainer())
    {
        for (const SwPosition* pPos : { rPaM.GetPoint(), rPaM.GetMark() })
        {
            const SwContentNode* pContentNode = pPos->GetNode().GetContentNode();
            if (!pContentNode)
                continue;
            CPPUNIT_ASSERT_LESSEQUAL(pContentNode->Len(), pPos->GetContentIndex());
        }
    }
}
}

CPPUNIT_TEST_FIXTURE(SwCoreUndoTest, testTdf169795CopyCellWithFormula)
{
    // Given a document with two tables and change recording enabled, where the cell to copy
    // holds a formula and the cell to copy it over does not:
    createSwDoc("tdf169795.odt");
    SwDocShell* pDocShell = getSwDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    SwXTextDocument* pTextDoc = getSwTextDoc();
    SwDoc* pDoc = pWrtShell->GetDoc();
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowInsert
                                                      | RedlineFlags::ShowDelete);
    CPPUNIT_ASSERT_EQUAL(OUString(), lcl_GetCellFormula(mxComponent, u"Table1"_ustr, u"D2"_ustr));

    // When copying that whole cell, "$100.00", over "$10.00":
    lcl_GotoCell(mxComponent, pTextDoc, u"Table2"_ustr, u"D3"_ustr);
    pWrtShell->SelTableBox();
    rtl::Reference<SwTransferable> pTransfer = new SwTransferable(*pWrtShell);
    pTransfer->Copy();
    TransferableDataHelper aHelper(pTransfer);
    lcl_GotoCell(mxComponent, pTextDoc, u"Table1"_ustr, u"D2"_ustr);
    pWrtShell->SelTableBox();
    SwTransferable::Paste(*pWrtShell, aHelper);

    // Then no change is recorded over the cell: what it shows is worked out from the formula in
    // its new place, not text that somebody wrote, so there is nothing to accept or reject.
    // Without the accompanying fix in place, the copy was recorded as a change over text that
    // the recalculation then rewrote, and the records left over that text described text that
    // was no longer there.
    CPPUNIT_ASSERT_EQUAL(OUString(), lcl_GetRedlines(pDoc));
    CPPUNIT_ASSERT(!lcl_GetCellFormula(mxComponent, u"Table1"_ustr, u"D2"_ustr).isEmpty());
    CPPUNIT_ASSERT_EQUAL(u"$10.00"_ustr, lcl_GetCellText(mxComponent, u"Table1"_ustr, u"D2"_ustr));

    // And undo takes the formula away again, leaving the old value behind.
    pWrtShell->Undo();
    CPPUNIT_ASSERT_EQUAL(OUString(), lcl_GetRedlines(pDoc));
    CPPUNIT_ASSERT_EQUAL(OUString(), lcl_GetCellFormula(mxComponent, u"Table1"_ustr, u"D2"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"$10.00"_ustr, lcl_GetCellText(mxComponent, u"Table1"_ustr, u"D2"_ustr));

    // The cursor has to stay inside the text it points at. Without the fix, undo left it on the
    // range of the pasted text, which the undo removed again, so it pointed past the end of the
    // cell text: content index 11 of a 6 character long node.
    lcl_AssertCursorInText(pWrtShell);

    // ... and counting words over that selection must not run out of the text either; this is
    // what the status bar does, and what crashed in SwScanner.
    SwDocStat aDocStat;
    pWrtShell->CountWords(aDocStat);

    // Redo puts the formula back, again without recording a change, and again with a cursor that
    // describes something. Without the fix, redo went down the route the copy had not taken.
    pWrtShell->Redo();
    CPPUNIT_ASSERT_EQUAL(OUString(), lcl_GetRedlines(pDoc));
    CPPUNIT_ASSERT(!lcl_GetCellFormula(mxComponent, u"Table1"_ustr, u"D2"_ustr).isEmpty());
    lcl_AssertCursorInText(pWrtShell);
    pWrtShell->CountWords(aDocStat);
}

CPPUNIT_TEST_FIXTURE(SwCoreUndoTest, testTdf169795NumFormatCellPasteKeepsTrackedChanges)
{
    // Given a document with number cells and change recording enabled:
    createSwDoc("tdf169795.odt");
    SwDocShell* pDocShell = getSwDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    SwXTextDocument* pTextDoc = getSwTextDoc();
    SwDoc* pDoc = pWrtShell->GetDoc();
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowInsert
                                                      | RedlineFlags::ShowDelete);

    // When copying one currency cell over another one:
    lcl_GotoCell(mxComponent, pTextDoc, u"Table1"_ustr, u"C3"_ustr);
    pWrtShell->SelTableBox();
    rtl::Reference<SwTransferable> pTransfer = new SwTransferable(*pWrtShell);
    pTransfer->Copy();
    TransferableDataHelper aHelper(pTransfer);
    lcl_GotoCell(mxComponent, pTextDoc, u"Table1"_ustr, u"C2"_ustr);
    pWrtShell->SelTableBox();
    SwTransferable::Paste(*pWrtShell, aHelper);

    // Then the old value has to be kept as a deletion.
    // Without the accompanying fix in place, the number format of the box regenerated the text
    // of the cell right after the copy, which dropped the deletion: only the insertion was left,
    // and the old value was gone for good.
    OUString aExpected(u"Insert:$2.00 Delete:$1.00"_ustr);
    CPPUNIT_ASSERT_EQUAL(aExpected, lcl_GetRedlines(pDoc));

    // Leaving the cell must not drop it either, that is where the cell is checked against its
    // number format.
    pWrtShell->EndAllTableBoxEdit();
    CPPUNIT_ASSERT_EQUAL(aExpected, lcl_GetRedlines(pDoc));

    // And undo and redo have to be symmetric: undo leaves the old value alone,
    pWrtShell->Undo();
    CPPUNIT_ASSERT_EQUAL(OUString(), lcl_GetRedlines(pDoc));
    CPPUNIT_ASSERT_EQUAL(u"$1.00"_ustr, lcl_GetCellText(mxComponent, u"Table1"_ustr, u"C2"_ustr));

    // ... and redo puts the very same pair of changes back. Without the fix, redo produced two
    // empty redlines instead, which piled up over further undo and redo rounds.
    pWrtShell->Redo();
    CPPUNIT_ASSERT_EQUAL(aExpected, lcl_GetRedlines(pDoc));
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

CPPUNIT_TEST_FIXTURE(SwCoreUndoTest, testTdf171068CopyPageDescWithStashedHeaderAcrossDocuments)
{
    createSwDoc();
    SwDoc* pSrcDocument = getSwDoc();
    uno::Reference<lang::XComponent> xSrcComponent = mxComponent;
    mxComponent.clear();

    createSwDoc();
    uno::Reference<lang::XComponent> xDstComponent = mxComponent;
    mxComponent.clear();

    // Build the source document, which will be a two page document with one paragraph
    // on each page, separated by a page break that sets the page style of the second
    // page to a custom page Style "SourceStyle", which includes a stashed header;

    // Create the page style and attach the stashed header
    {
        SwPageDesc* pSrcPageDesc = pSrcDocument->MakePageDesc(UIName(u"SourceStyle"_ustr));
        pSrcPageDesc->ChgFirstShare(false);
        CPPUNIT_ASSERT(!pSrcPageDesc->IsFirstShared());

        SwFrameFormat& rFirstMaster = pSrcPageDesc->GetFirstMaster();
        rFirstMaster.SetFormatAttr(SwFormatHeader(true));

        pSrcPageDesc->StashFrameFormat(pSrcPageDesc->GetFirstMaster(), true, false, true);
        pSrcDocument->ChgPageDesc(UIName(u"SourceStyle"_ustr), *pSrcPageDesc);
        CPPUNIT_ASSERT(pSrcPageDesc->HasStashedFormat(true, false, true));
    }

    // Insert the paragraphs and the page break which sets the page style for page 2
    {
        uno::Reference<text::XTextDocument> xTextDoc(xSrcComponent, uno::UNO_QUERY);
        uno::Reference<text::XText> xText = xTextDoc->getText();
        uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();

        xText->insertString(xCursor, u"First page"_ustr, false);
        xText->insertControlCharacter(xCursor, css::text::ControlCharacter::PARAGRAPH_BREAK, false);

        uno::Reference<beans::XPropertySet> xProps(xCursor, uno::UNO_QUERY);
        xProps->setPropertyValue(u"BreakType"_ustr, uno::Any(style::BreakType_PAGE_BEFORE));
        xProps->setPropertyValue(u"PageDescName"_ustr, uno::Any(u"SourceStyle"_ustr));

        xText->insertString(xCursor, u"Second page"_ustr, false);
    }

    dispatchCommand(xSrcComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(xSrcComponent, u".uno:Copy"_ustr, {});

    dispatchCommand(xDstComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(xDstComponent, u".uno:Paste"_ustr, {});
    dispatchCommand(xDstComponent, u".uno:Undo"_ustr, {});

    // Without the fix, undoing the copy of the page desc failed
    // because the page desc could not be located on the dest
    // document, because the frame format was not named the same
    // as the page desc.

    // Also, the stashed frame format was not correctly saved
    // to the undo nodes, which led to corruption of the
    // node offsets similar to tdf#148703.

    dispatchCommand(xDstComponent, u".uno:Redo"_ustr, {});
    dispatchCommand(xDstComponent, u".uno:Undo"_ustr, {});

    xSrcComponent->dispose();
    xDstComponent->dispose();
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
