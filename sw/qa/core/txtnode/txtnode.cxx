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
#include <comphelper/lok.hxx>
#include <comphelper/propertyvalue.hxx>
#include <com/sun/star/text/XTextTable.hpp>
#include <sfx2/viewsh.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/scheduler.hxx>
#include <sfx2/lokhelper.hxx>
#include <test/lokcallback.hxx>
#include <editeng/escapementitem.hxx>

#include <IDocumentStatistics.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <fmtanchr.hxx>
#include <frameformats.hxx>
#include <wrtsh.hxx>
#include <unotxdoc.hxx>
#include <docsh.hxx>
#include <formatcontentcontrol.hxx>
#include <view.hxx>
#include <edtwin.hxx>
#include <txatbase.hxx>
#include <ndtxt.hxx>
#include <textcontentcontrol.hxx>
#include <swdtflvr.hxx>
#include <txtrfmrk.hxx>
#include <frmmgr.hxx>
#include <formatflysplit.hxx>
#include <ftnidx.hxx>
#include <rootfrm.hxx>
#include <pagefrm.hxx>
#include <txtfrm.hxx>

/// Covers sw/source/core/txtnode/ fixes.
class SwCoreTxtnodeTest : public SwModelTestBase
{
public:
    SwCoreTxtnodeTest()
        : SwModelTestBase(u"/sw/qa/core/txtnode/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(SwCoreTxtnodeTest, testBtlrCellChinese)
{
    // Load a document with a table cell, with btlr cell direction.  The cell has text which is
    // classified as vertical, i.e. the glyph has the same direction in both the lrtb ("Latin") and
    // tbrl ("Chinese") directions. Make sure that Chinese text is handled the same way in the btlr
    // case as it's handled in the Latin case.
    createSwDoc("btlr-cell-chinese.doc");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwDocShell* pShell = pTextDoc->GetDocShell();
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    assertXPath(pXmlDoc, "//font[1]"_ostr, "orientation"_ostr, u"900"_ustr);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: false
    // - Actual  : true
    // i.e. the glyph was rotated further, so it was upside down.
    assertXPath(pXmlDoc, "//font[1]"_ostr, "vertical"_ostr, u"false"_ustr);
}

CPPUNIT_TEST_FIXTURE(SwCoreTxtnodeTest, testSpecialInsertAfterMergedCells)
{
    // Load a document with a table with bottom right cells merged vertically.
    // SpecialInsert with alt-Enter must work here, too.
    createSwDoc("special-insert-after-merged-cells.fodt");
    SwDoc* pDoc = getSwDoc();
    SwNodeOffset const nNodes(pDoc->GetNodes().Count());
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwDocShell* pShell = pTextDoc->GetDocShell();
    SwWrtShell* pWrtShell = pShell->GetWrtShell();
    // go to the merged cell
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);

    // When pressing alt-Enter on the keyboard:
    SwEditWin& rEditWin = pWrtShell->GetView().GetEditWin();
    vcl::KeyCode aKeyCode(KEY_RETURN, KEY_MOD2);
    KeyEvent aKeyEvent(' ', aKeyCode);
    rEditWin.KeyInput(aKeyEvent);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: nNodes + 1
    // - Actual  : nNodes
    // i.e. new empty paragraph wasn't inserted under the table
    CPPUNIT_ASSERT_EQUAL(nNodes + 1, pDoc->GetNodes().Count());
}

CPPUNIT_TEST_FIXTURE(SwCoreTxtnodeTest, testTextBoxCopyAnchor)
{
    createSwDoc("textbox-copy-anchor.docx");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwDocShell* pShell = pTextDoc->GetDocShell();
    SwWrtShell* pWrtShell = pShell->GetWrtShell();
    SwDoc aClipboard;
    pWrtShell->SelAll();
    pWrtShell->Copy(aClipboard);
    pWrtShell->SttEndDoc(/*bStart=*/false);
    pWrtShell->Paste(aClipboard);

    const auto& rFormats = *pShell->GetDoc()->GetSpzFrameFormats();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 4
    // - Actual  : 6
    // i.e. 2 fly frames were copied twice.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), rFormats.size());

    SwPosition aDrawAnchor1 = *rFormats[0]->GetAnchor().GetContentAnchor();
    SwPosition aFlyAnchor1 = *rFormats[1]->GetAnchor().GetContentAnchor();
    CPPUNIT_ASSERT_EQUAL(aFlyAnchor1.GetNodeIndex(), aDrawAnchor1.GetNodeIndex());
    SwPosition aDrawAnchor2 = *rFormats[2]->GetAnchor().GetContentAnchor();
    SwPosition aFlyAnchor2 = *rFormats[3]->GetAnchor().GetContentAnchor();
    // This also failed, aFlyAnchor2 was wrong, as it got out of sync with aDrawAnchor2.
    CPPUNIT_ASSERT_EQUAL(aFlyAnchor2.GetNodeIndex(), aDrawAnchor2.GetNodeIndex());
}

CPPUNIT_TEST_FIXTURE(SwCoreTxtnodeTest, testTextBoxNodeSplit)
{
    createSwDoc("textbox-node-split.docx");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwDocShell* pShell = pTextDoc->GetDocShell();
    SwWrtShell* pWrtShell = pShell->GetWrtShell();
    pWrtShell->SttEndDoc(/*bStart=*/false);
    // Without the accompanying fix in place, this would have crashed in
    // SwFlyAtContentFrame::SwClientNotify().
    pWrtShell->SplitNode();
}

namespace
{
struct ViewCallback
{
    int m_nInvalidations = 0;

    static void callback(int nType, const char* pPayload, void* pData);
    void callbackImpl(int nType, const char* pPayload);
};

void ViewCallback::callback(int nType, const char* pPayload, void* pData)
{
    static_cast<ViewCallback*>(pData)->callbackImpl(nType, pPayload);
}

void ViewCallback::callbackImpl(int nType, const char* /*pPayload*/)
{
    switch (nType)
    {
        case LOK_CALLBACK_INVALIDATE_TILES:
        {
            ++m_nInvalidations;
        }
        break;
    }
}
}

CPPUNIT_TEST_FIXTURE(SwCoreTxtnodeTest, testTitleFieldInvalidate)
{
    // Set up LOK to track invalidations.
    comphelper::LibreOfficeKit::setActive(true);

    // Given a document with a title field:
    createSwDoc("title-field-invalidate.fodt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    pTextDoc->initializeForTiledRendering({});
    SwDocShell* pShell = pTextDoc->GetDocShell();
    SwDoc* pDoc = pShell->GetDoc();
    SwWrtShell* pWrtShell = pShell->GetWrtShell();
    pWrtShell->SttEndDoc(/*bStt=*/false);
    ViewCallback aCallback;
    TestLokCallbackWrapper aCallbackWrapper(&ViewCallback::callback, &aCallback);
    pWrtShell->GetSfxViewShell()->setLibreOfficeKitViewCallback(&aCallbackWrapper);
    aCallbackWrapper.setLOKViewId(SfxLokHelper::getView(pWrtShell->GetSfxViewShell()));
    Scheduler::ProcessEventsToIdle();
    aCallback.m_nInvalidations = 0;

    // When typing to the document:
    pWrtShell->Insert(u"x"_ustr);
    pWrtShell->GetSfxViewShell()->flushPendingLOKInvalidateTiles();

    // Then make sure that only the text frame at the cursor is invalidated:
    pDoc->getIDocumentStatistics().GetUpdatedDocStat(/*bCompleteAsync=*/true, /*bFields=*/false);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 2
    // i.e. the footer was also invalidated on each keypress.
    CPPUNIT_ASSERT_EQUAL(1, aCallback.m_nInvalidations);

    // Tear down LOK.
    pWrtShell->GetSfxViewShell()->setLibreOfficeKitViewCallback(nullptr);
    mxComponent->dispose();
    mxComponent.clear();
    comphelper::LibreOfficeKit::setActive(false);
}

CPPUNIT_TEST_FIXTURE(SwCoreTxtnodeTest, testFlyAnchorUndo)
{
    // Given a document with a fly frame, anchored after the last char of the document:
    createSwDoc("fly-anchor-undo.odt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwDocShell* pShell = pTextDoc->GetDocShell();
    SwDoc* pDoc = pShell->GetDoc();
    const auto& rSpz = *pDoc->GetSpzFrameFormats();
    sal_Int32 nExpected = rSpz[0]->GetAnchor().GetAnchorContentOffset();

    // When deleting that last character and undoing it:
    SwWrtShell* pWrtShell = pShell->GetWrtShell();
    pWrtShell->SttEndDoc(/*bStt=*/false);
    pWrtShell->DelLeft();
    pWrtShell->Undo();

    // Then make sure the anchor position after the undo is the same as the original:
    sal_Int32 nActual = rSpz[0]->GetAnchor().GetAnchorContentOffset();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 3
    // - Actual  : 2
    // i.e. the anchor position was left unchanged by the undo.
    CPPUNIT_ASSERT_EQUAL(nExpected, nActual);
}

CPPUNIT_TEST_FIXTURE(SwCoreTxtnodeTest, testSplitNodeSuperscriptCopy)
{
    // Given a document with superscript text at the end of a paragraph:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Insert(u"1st"_ustr);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 2, /*bBasicCall=*/false);
    SfxItemSetFixed<RES_CHRATR_ESCAPEMENT, RES_CHRATR_ESCAPEMENT> aSet(pWrtShell->GetAttrPool());
    SvxEscapementItem aItem(SvxEscapement::Superscript, RES_CHRATR_ESCAPEMENT);
    aSet.Put(aItem);
    pWrtShell->SetAttrSet(aSet);

    // When hitting enter at the end of the paragraph:
    pWrtShell->SttEndDoc(/*bStt=*/false);
    pWrtShell->SplitNode(/*bAutoFormat=*/true);

    // Then make sure that the superscript formatting doesn't appear on the next paragraph:
    aSet.ClearItem(RES_CHRATR_ESCAPEMENT);
    pWrtShell->GetCurAttr(aSet);
    // Without the accompanying fix in place, this test would have failed, the unexpected
    // superscript appeared in the next paragraph.
    CPPUNIT_ASSERT(!aSet.HasItem(RES_CHRATR_ESCAPEMENT));
}

/* FIXME: behavior change reverted due to regression;
 * see sw/source/core/txtnode/atrref.cxx
 *CPPUNIT_TEST_FIXTURE(SwCoreTxtnodeTest, testDontExpandRefmark)
 *{
 *    // Given a document with a refmark:
 *    createSwDoc();
 *
 *    uno::Sequence<css::beans::PropertyValue> aArgs = {
 *        comphelper::makePropertyValue("TypeName", uno::Any(OUString("SetRef"))),
 *        comphelper::makePropertyValue(
 *            "Name", uno::Any(OUString("ZOTERO_ITEM CSL_CITATION {} RNDpyJknp173F"))),
 *        comphelper::makePropertyValue("Content", uno::Any(OUString("foo"))),
 *    };
 *    dispatchCommand(mxComponent, ".uno:InsertField", aArgs);
 *
 *    SwDoc* pDoc = getSwDoc();
 *    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
 *    SwPosition& rCursor = *pWrtShell->GetCursor()->GetPoint();
 *    SwTextNode* pTextNode = rCursor.GetNode().GetTextNode();
 *    std::vector<SwTextAttr*> aAttrs
 *        = pTextNode->GetTextAttrsAt(rCursor.GetContentIndex(), RES_TXTATR_REFMARK);
 *
 *    auto& rRefmark = const_cast<SwFormatRefMark&>(aAttrs[0]->GetRefMark());
 *    auto pTextRefMark = const_cast<SwTextRefMark*>(rRefmark.GetTextRefMark());
 *
 *    // When typing after the refmark...
 *    pWrtShell->SttEndDoc(true);
 *    pWrtShell->Right(SwCursorSkipMode::Chars, false, 3, false);
 *    pWrtShell->Insert(" bar");
 *
 *    // and skipping back to insert a comma after the refmark
 *    pWrtShell->Left(SwCursorSkipMode::Chars, false, 4, false);
 *    pWrtShell->Insert(",");
 *
 *    // Without the accompanying fix in place, this test would have failed with:
 *    // - Expected: 3
 *    // - Actual  : 4
 *    // i.e. the reference mark expanded
 *    CPPUNIT_ASSERT_EQUAL(3, static_cast<int>(*pTextRefMark->End()));
 *}
 */

CPPUNIT_TEST_FIXTURE(SwCoreTxtnodeTest, testInsertDropDownContentControlTwice)
{
    // Given an already selected dropdown content control:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->InsertContentControl(SwContentControlType::DROP_DOWN_LIST);

    // When trying to insert an inner one, make sure that we don't crash:
    pWrtShell->InsertContentControl(SwContentControlType::DROP_DOWN_LIST);
}

CPPUNIT_TEST_FIXTURE(SwCoreTxtnodeTest, testCheckboxContentControlKeyboard)
{
    // Given an already selected checkbox content control:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->InsertContentControl(SwContentControlType::CHECKBOX);
    SwEditWin& rEditWin = pWrtShell->GetView().GetEditWin();

    // When pressing space on the keyboard:
    KeyEvent aKeyEvent(' ', KEY_SPACE);
    rEditWin.KeyInput(aKeyEvent);

    // Then make sure the state is toggled:
    SwTextNode* pTextNode = pWrtShell->GetCursor()->GetPointNode().GetTextNode();
    SwTextAttr* pAttr = pTextNode->GetTextAttrForCharAt(0, RES_TXTATR_CONTENTCONTROL);
    auto pTextContentControl = static_txtattr_cast<SwTextContentControl*>(pAttr);
    auto& rFormatContentControl
        = static_cast<SwFormatContentControl&>(pTextContentControl->GetAttr());
    std::shared_ptr<SwContentControl> pContentControl = rFormatContentControl.GetContentControl();
    // Without the accompanying fix in place, this test would have failed, because the state
    // remained unchanged.
    CPPUNIT_ASSERT(pContentControl->GetChecked());
}

CPPUNIT_TEST_FIXTURE(SwCoreTxtnodeTest, testDropdownContentControlKeyboard)
{
    // Given an already selected dropdown content control:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->InsertContentControl(SwContentControlType::DROP_DOWN_LIST);

    // When checking if alt-down should open a popup:
    SwTextContentControl* pTextContentControl = pWrtShell->CursorInsideContentControl();
    auto& rFormatContentControl
        = static_cast<SwFormatContentControl&>(pTextContentControl->GetAttr());
    std::shared_ptr<SwContentControl> pContentControl = rFormatContentControl.GetContentControl();
    vcl::KeyCode aKeyCode(KEY_DOWN, KEY_MOD2);
    bool bShouldOpen = pContentControl->ShouldOpenPopup(aKeyCode);

    // Then make sure that the answer is yes for dropdowns:
    // Without the accompanying fix in place, this test would have failed, the dropdown popup was
    // mouse-only.
    CPPUNIT_ASSERT(bShouldOpen);
}

CPPUNIT_TEST_FIXTURE(SwCoreTxtnodeTest, testPictureContentControlKeyboard)
{
    // Given an already selected picture content control:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->InsertContentControl(SwContentControlType::PICTURE);
    pWrtShell->GotoObj(/*bNext=*/true, GotoObjFlags::Any);

    // When checking if enter should trigger the file picker:
    const SwFrameFormat* pFlyFormat = pWrtShell->GetFlyFrameFormat();
    const SwFormatAnchor& rFormatAnchor = pFlyFormat->GetAnchor();
    SwNode* pAnchorNode = rFormatAnchor.GetAnchorNode();
    SwTextNode* pTextNode = pAnchorNode->GetTextNode();
    SwTextAttr* pAttr
        = pTextNode->GetTextAttrAt(rFormatAnchor.GetAnchorContentOffset(),
                                   RES_TXTATR_CONTENTCONTROL, ::sw::GetTextAttrMode::Parent);
    auto pTextContentControl = static_txtattr_cast<SwTextContentControl*>(pAttr);
    auto& rFormatContentControl
        = static_cast<SwFormatContentControl&>(pTextContentControl->GetAttr());
    std::shared_ptr<SwContentControl> pContentControl = rFormatContentControl.GetContentControl();
    bool bIsInteracting = pContentControl->IsInteractingCharacter('\r');

    // Then make sure that the answer is yes for pictures:
    // Without the accompanying fix in place, this test would have failed, the picture replacement
    // file-picker was mouse-only.
    CPPUNIT_ASSERT(bIsInteracting);
}

CPPUNIT_TEST_FIXTURE(SwCoreTxtnodeTest, testDateContentControlKeyboard)
{
    // Given an already selected date content control:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->InsertContentControl(SwContentControlType::DATE);

    // When checking if alt-down should open a popup:
    SwTextContentControl* pTextContentControl = pWrtShell->CursorInsideContentControl();
    auto& rFormatContentControl
        = static_cast<SwFormatContentControl&>(pTextContentControl->GetAttr());
    std::shared_ptr<SwContentControl> pContentControl = rFormatContentControl.GetContentControl();
    vcl::KeyCode aKeyCode(KEY_DOWN, KEY_MOD2);
    bool bShouldOpen = pContentControl->ShouldOpenPopup(aKeyCode);

    // Then make sure that the answer is yes for date:
    // Without the accompanying fix in place, this test would have failed, the date popup was
    // mouse-only.
    CPPUNIT_ASSERT(bShouldOpen);
}

CPPUNIT_TEST_FIXTURE(SwCoreTxtnodeTest, testContentControlCopy)
{
    // Given a document with a content control:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->InsertContentControl(SwContentControlType::CHECKBOX);

    // When copying that content control:
    pWrtShell->SelAll();
    rtl::Reference<SwTransferable> xTransfer = new SwTransferable(*pWrtShell);
    xTransfer->Copy();
    // Kill the selection, go to the end of the document:
    pWrtShell->EndOfSection();
    TransferableDataHelper aHelper(xTransfer);
    SwTransferable::Paste(*pWrtShell, aHelper);

    // Then make sure that the copy is also a checkbox:
    SwContentControlManager& rManager = pDoc->GetContentControlManager();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), rManager.GetCount());
    const SwFormatContentControl& rFormat1 = rManager.Get(0)->GetContentControl();
    CPPUNIT_ASSERT_EQUAL(SwContentControlType::CHECKBOX, rFormat1.GetContentControl()->GetType());
    const SwFormatContentControl& rFormat2 = rManager.Get(1)->GetContentControl();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1 (CHECKBOX)
    // - Actual  : 0 (RICH_TEXT)
    // i.e. properties were not copied from the source to the destination content control.
    CPPUNIT_ASSERT_EQUAL(SwContentControlType::CHECKBOX, rFormat2.GetContentControl()->GetType());
}

CPPUNIT_TEST_FIXTURE(SwCoreTxtnodeTest, testTdf157287)
{
    createSwDoc("tdf157287.odt");
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    auto xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    uno::Reference<text::XTextField> xField(xFields->nextElement(), uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(u"30"_ustr, xField->getPresentation(false));

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xIndexAccess->getByIndex(0), uno::UNO_QUERY);

    uno::Reference<text::XTextRange> xCellA1(xTextTable->getCellByName(u"B1"_ustr), uno::UNO_QUERY);
    xCellA1->setString(u"100"_ustr);

    dispatchCommand(mxComponent, u".uno:UpdateFields"_ustr, {});

    // Without the fix in place, this test would have failed with
    // - Expected: 120
    // - Actual  :
    CPPUNIT_ASSERT_EQUAL(u"120"_ustr, xField->getPresentation(false));
}

CPPUNIT_TEST_FIXTURE(SwCoreTxtnodeTest, testFlySplitFootnote)
{
    // Given a document with a split fly (to host a table):
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    SwFlyFrameAttrMgr aMgr(true, pWrtShell, Frmmgr_Type::TEXT, nullptr);
    RndStdIds eAnchor = RndStdIds::FLY_AT_PARA;
    pWrtShell->StartAllAction();
    aMgr.InsertFlyFrame(eAnchor, aMgr.GetPos(), aMgr.GetSize());
    pWrtShell->EndAllAction();
    pWrtShell->StartAllAction();
    sw::FrameFormats<sw::SpzFrameFormat*>& rFlys = *pDoc->GetSpzFrameFormats();
    sw::SpzFrameFormat* pFly = rFlys[0];
    SwAttrSet aSet(pFly->GetAttrSet());
    aSet.Put(SwFormatFlySplit(true));
    pDoc->SetAttr(aSet, *pFly);
    pWrtShell->EndAllAction();
    pWrtShell->UnSelectFrame();
    pWrtShell->LeaveSelFrameMode();
    pWrtShell->GetView().AttrChangedNotify(nullptr);
    pWrtShell->MoveSection(GoCurrSection, fnSectionEnd);

    // When inserting a footnote:
    pWrtShell->InsertFootnote(OUString());

    // Then make sure the footnote gets inserted to the doc model.
    // Without the accompanying fix in place, this test would have failed, insert code refused to
    // have footnotes in all fly frames.
    CPPUNIT_ASSERT(!pDoc->GetFootnoteIdxs().empty());
}

CPPUNIT_TEST_FIXTURE(SwCoreTxtnodeTest, testSplitFlyAnchorSplit)
{
    // Given a document with a 2 pages long floating table:
    createSwDoc("floattable-anchor-split.docx");

    // When splitting the "AB" anchor text into "A" (remains as anchor text) and "B" (new text node
    // after it):
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->SttEndDoc(/*bStt=*/false);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    // Without the accompanying fix in place, this test would have failed with a layout loop.
    pWrtShell->SplitNode();

    // Then make sure the resulting layout is what we want:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage1 = pLayout->Lower()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage1);
    // Page 1 has the master fly:
    CPPUNIT_ASSERT(pPage1->GetSortedObjs());
    auto pPage2 = pPage1->GetNext()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage2);
    // Page 2 has the follow fly:
    CPPUNIT_ASSERT(pPage2->GetSortedObjs());
    // Anchor text is now just "A":
    auto pText1 = pPage2->FindFirstBodyContent()->DynCastTextFrame();
    CPPUNIT_ASSERT_EQUAL(u"A"_ustr, pText1->GetText());
    // New text frame is just "B":
    auto pText2 = pText1->GetNext()->DynCastTextFrame();
    CPPUNIT_ASSERT_EQUAL(u"B"_ustr, pText2->GetText());

    // Also test that the new follow anchor text frame still has a fly portion, otherwise the anchor
    // text and the floating table would overlap:
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    OUString aPortionType
        = getXPath(pXmlDoc, "//page[2]/body/txt[1]/SwParaPortion/SwLineLayout[1]/child::*[1]"_ostr,
                   "type"_ostr);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: PortionType::Fly
    // - Actual  : PortionType::Para
    // i.e. the fly portion was missing, text overlapped.
    CPPUNIT_ASSERT_EQUAL(u"PortionType::Fly"_ustr, aPortionType);
}

CPPUNIT_TEST_FIXTURE(SwCoreTxtnodeTest, testPlainContentControlCopy)
{
    // Given a document with a plain text content control, all text selected and copied to the
    // clipboard:
    createSwDoc("plain-content-control-copy.docx");
    SwDocShell* pDocShell = getSwDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->SelAll();
    {
        rtl::Reference<SwTransferable> xTransfer = new SwTransferable(*pWrtShell);
        xTransfer->Copy();
    }

    // When closing that document, then make sure we don't crash on shutdown:
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<util::XCloseable> xFrame(xModel->getCurrentController()->getFrame(),
                                            uno::UNO_QUERY);
    // Without the accompanying fix in place, this resulted in an assertion failure, a char style
    // still had clients by the time it was deleted.
    xFrame->close(false);
    mxComponent.clear();
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
