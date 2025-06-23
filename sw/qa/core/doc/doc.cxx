/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/view/XSelectionSupplier.hpp>

#include <comphelper/classids.hxx>
#include <tools/globname.hxx>
#include <svtools/embedhlp.hxx>
#include <editeng/acorrcfg.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/svxacorr.hxx>
#include <vcl/errinf.hxx>
#include <vcl/event.hxx>
#include <editeng/langitem.hxx>
#include <vcl/scheduler.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/scopeguard.hxx>

#include <wrtsh.hxx>
#include <fmtanchr.hxx>
#include <frameformats.hxx>
#include <docsh.hxx>
#include <edtwin.hxx>
#include <view.hxx>
#include <ndtxt.hxx>
#include <swdtflvr.hxx>
#include <unotxdoc.hxx>
#include <UndoManager.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <frmmgr.hxx>
#include <formatflysplit.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <rootfrm.hxx>
#include <pagefrm.hxx>
#include <sortedobjs.hxx>
#include <itabenum.hxx>
#include <redline.hxx>
#include <UndoRedline.hxx>

/// Covers sw/source/core/doc/ fixes.
class SwCoreDocTest : public SwModelTestBase
{
public:
    SwCoreDocTest()
        : SwModelTestBase(u"/sw/qa/core/doc/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(SwCoreDocTest, testMathInsertAnchorType)
{
    // Given an empty document.
    createSwDoc();
    SwDoc* pDoc = getSwDoc();

    // When inserting an a math object.
    SwWrtShell* pShell = getSwDocShell()->GetWrtShell();
    SvGlobalName aGlobalName(SO3_SM_CLASSID);
    pShell->InsertObject(svt::EmbeddedObjectRef(), &aGlobalName);

    // Then the anchor type should be as-char.
    sw::SpzFrameFormats& rFormats = *pDoc->GetSpzFrameFormats();
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
    createSwDoc("textbox-textrotateangle.odt");
    SwDoc* pDoc = getSwDoc();
    sw::SpzFrameFormats& rFrameFormats = *pDoc->GetSpzFrameFormats();
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
    createSwDoc("num-down-indent.docx");
    SwDocShell* pDocShell = getSwDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->Down(/*bSelect=*/false);
    SwEditWin& rEditWin = pDocShell->GetView()->GetEditWin();
    KeyEvent aKeyEvent(0, KEY_TAB);
    rEditWin.KeyInput(aKeyEvent);
    SwTextNode* pTextNode = pWrtShell->GetCursor()->GetPointNode().GetTextNode();

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: \tB
    // - Actual  : B
    // i.e. pressing <tab> at the start of the paragraph did not change the layout.
    CPPUNIT_ASSERT_EQUAL(u"\tB"_ustr, pTextNode->GetText());
    ErrorRegistry::Reset();
}

CPPUNIT_TEST_FIXTURE(SwCoreDocTest, testBulletsOnSpaceOff)
{
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
    pAutoCorrect->GetSwFlags().bSetNumRule = false;
    pAutoCorrect->GetSwFlags().bSetNumRuleAfterSpace = false;

    createSwDoc();
    SwDocShell* pDocShell = getSwDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->Down(/*bSelect=*/false);
    SwEditWin& rEditWin = pDocShell->GetView()->GetEditWin();
    KeyEvent aKeyEvent('-', 0);
    rEditWin.KeyInput(aKeyEvent);
    KeyEvent aKeyEvent2(' ', KEY_SPACE);
    rEditWin.KeyInput(aKeyEvent2);
    KeyEvent aKeyEvent3('a', 0);
    rEditWin.KeyInput(aKeyEvent3);
    SwTextNode* pTextNode = pWrtShell->GetCursor()->GetPointNode().GetTextNode();

    CPPUNIT_ASSERT_EQUAL(u"- a"_ustr, pTextNode->GetText());
    ErrorRegistry::Reset();
}

CPPUNIT_TEST_FIXTURE(SwCoreDocTest, testBulletsOnSpace)
{
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
    pAutoCorrect->GetSwFlags().bSetNumRule = true;
    pAutoCorrect->GetSwFlags().bSetNumRuleAfterSpace = true;

    createSwDoc();
    SwDocShell* pDocShell = getSwDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->Down(/*bSelect=*/false);
    SwEditWin& rEditWin = pDocShell->GetView()->GetEditWin();
    KeyEvent aKeyEvent('-', 0);
    rEditWin.KeyInput(aKeyEvent);
    KeyEvent aKeyEvent2(' ', KEY_SPACE);
    rEditWin.KeyInput(aKeyEvent2);
    KeyEvent aKeyEvent3('a', 0);
    rEditWin.KeyInput(aKeyEvent3);
    SwTextNode* pTextNode = pWrtShell->GetCursor()->GetPointNode().GetTextNode();

    // '- ' was converted into bullet
    CPPUNIT_ASSERT_EQUAL(u"a"_ustr, pTextNode->GetText());
    ErrorRegistry::Reset();
}

CPPUNIT_TEST_FIXTURE(SwCoreDocTest, testLocaleIndependentTemplate)
{
    createSwDoc("locale-independent-template.odt");
    SwDocShell* pDocShell = getSwDocShell();
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
    createSwDoc("textbox-zorder.docx");
    SwDoc* pDoc = getSwDoc();
    sw::SpzFrameFormats& rFormats = *pDoc->GetSpzFrameFormats();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), rFormats.size());
    const sw::SpzFrameFormat* pEllipse = rFormats[2];
    const SdrObject* pEllipseShape = pEllipse->FindRealSdrObject();
    // Make sure we test the right shape.
    CPPUNIT_ASSERT_EQUAL(u"Shape3"_ustr, pEllipseShape->GetName());
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2
    // - Actual  : 1
    // i.e. the ellipse was under the frame of the shape-frame pair, not on top of it.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(2), pEllipseShape->GetOrdNum());
}

CPPUNIT_TEST_FIXTURE(SwCoreDocTest, testTextBoxMakeFlyFrame)
{
    // Given a document with an as-char textbox (as-char draw format + at-char fly format):
    createSwDoc("textbox-makeflyframe.docx");

    // When cutting the textbox and pasting it to a new document:
    selectShape(1);
    SwDocShell* pDocShell = getSwDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    rtl::Reference<SwTransferable> pTransfer = new SwTransferable(*pWrtShell);
    pTransfer->Cut();
    TransferableDataHelper aHelper(pTransfer);
    mxComponent2 = loadFromDesktop(u"private:factory/swriter"_ustr,
                                   u"com.sun.star.text.TextDocument"_ustr, {});
    SwXTextDocument* pTextDoc2 = dynamic_cast<SwXTextDocument*>(mxComponent2.get());
    SwDocShell* pDocShell2 = pTextDoc2->GetDocShell();
    SwWrtShell* pWrtShell2 = pDocShell2->GetWrtShell();
    SwTransferable::Paste(*pWrtShell2, aHelper);

    // Then make sure its fly frame is created.
    xmlDocUniquePtr pLayout = parseLayoutDump(mxComponent2);
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
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    // Make sure no idle is in action, so the ExtTextInput events go to SwEditWin.
    Scheduler::ProcessEventsToIdle();

    // When pressing two keys via IME:
    SwDocShell* pDocShell = getSwDocShell();
    SwEditWin& rEditWin = pDocShell->GetView()->GetEditWin();
    rEditWin.PostExtTextInputEvent(VclEventId::ExtTextInput, u"a"_ustr);
    rEditWin.PostExtTextInputEvent(VclEventId::EndExtTextInput, u""_ustr);
    rEditWin.PostExtTextInputEvent(VclEventId::ExtTextInput, u"b"_ustr);
    rEditWin.PostExtTextInputEvent(VclEventId::EndExtTextInput, u""_ustr);

    // Then make sure that gets grouped together to a single undo action:
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    SwTextNode* pTextNode = pWrtShell->GetCursor()->GetPointNode().GetTextNode();
    CPPUNIT_ASSERT_EQUAL(u"ab"_ustr, pTextNode->GetText());
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
    createSwDoc();
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    uno::Reference<text::XTextContent> xImage(
        xFactory->createInstance(u"com.sun.star.text.TextGraphicObject"_ustr), uno::UNO_QUERY);
    xText->insertTextContent(xCursor, xImage, /*bAbsorb=*/false);
    uno::Reference<beans::XPropertySet> xImageProps(xImage, uno::UNO_QUERY);
    OUString aExpected = u"http://www.example.com"_ustr;
    xImageProps->setPropertyValue(u"HyperLinkURL"_ustr, uno::Any(aExpected));

    // When applying a frame style on it:
    xImageProps->setPropertyValue(u"FrameStyleName"_ustr, uno::Any(u"Frame"_ustr));

    // Then make sure that the hyperlink is not lost:
    auto aActual = getProperty<OUString>(xImageProps, u"HyperLinkURL"_ustr);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: http://www.example.com
    // - Actual  :
    // i.e. the link was lost, even if the frame style dialog doesn't allow specifying a link on
    // frames.
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

CPPUNIT_TEST_FIXTURE(SwCoreDocTest, testContentControlDelete)
{
    // Given a document with a content control:
    createSwDoc();
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertString(xCursor, u"test"_ustr, /*bAbsorb=*/false);
    xCursor->gotoStart(/*bExpand=*/false);
    xCursor->gotoEnd(/*bExpand=*/true);
    uno::Reference<text::XTextContent> xContentControl(
        xMSF->createInstance(u"com.sun.star.text.ContentControl"_ustr), uno::UNO_QUERY);
    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);

    // When deleting the dummy character at the end of the content control:
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->SttEndDoc(/*bStt=*/false);
    pWrtShell->DelLeft();

    // Then make sure that we only enter the content control, to be consistent with the start dummy
    // character:
    SwTextNode* pTextNode = pWrtShell->GetCursor()->GetMark()->GetNode().GetTextNode();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: ^Atest^A
    // - Actual  : ^Atest
    // i.e. the end dummy character got deleted, but not the first one, which is inconsistent.
    CPPUNIT_ASSERT_EQUAL(u"\x0001test\x0001"_ustr, pTextNode->GetText());
}

CPPUNIT_TEST_FIXTURE(SwCoreDocTest, testCopyBookmarks)
{
    // Given a document with a bookmark in a header that is linked later:
    createSwDoc("copy-bookmarks.docx");
    SwDoc* pDoc = getSwDoc();

    // When checking the # of non-copy bookmarks in the resulting doc model:
    sal_Int32 nActual = 0;
    for (auto it = pDoc->getIDocumentMarkAccess()->getBookmarksBegin();
         it != pDoc->getIDocumentMarkAccess()->getBookmarksEnd(); ++it)
    {
        if ((*it)->GetName().toString().indexOf("Copy") == -1)
        {
            ++nActual;
        }
    }

    // Then make sure we have a single non-copy bookmark, with no duplications:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 2
    // i.e. the 2nd header had a duplicated bookmark without "Copy" in its name.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), nActual);

    // Also, when checking the # of non-copy images in the resulting doc model:
    nActual = 0;
    for (auto pSpz : *pDoc->GetSpzFrameFormats())
    {
        if (pSpz->GetName().toString().indexOf("Copy") == -1)
        {
            ++nActual;
        }
    }

    // Then make sure we have a single non-copy image, with no duplications:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 2
    // i.e. the 2nd header had a duplicated image without "Copy" in its name.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), nActual);
}

CPPUNIT_TEST_FIXTURE(SwCoreDocTest, testLinkedStyleDelete)
{
    // Given a document with linked styles: myparastyle is linked to mycharstyle and vica versa:
    createSwDoc();
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xParaStyle(
        xFactory->createInstance(u"com.sun.star.style.ParagraphStyle"_ustr), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xCharStyle(
        xFactory->createInstance(u"com.sun.star.style.CharacterStyle"_ustr), uno::UNO_QUERY);
    uno::Reference<container::XNameContainer> xParaStyles(getStyles(u"ParagraphStyles"_ustr),
                                                          uno::UNO_QUERY);
    xParaStyles->insertByName(u"myparastyle"_ustr, uno::Any(xParaStyle));
    uno::Reference<container::XNameContainer> xCharStyles(getStyles(u"CharacterStyles"_ustr),
                                                          uno::UNO_QUERY);
    xCharStyles->insertByName(u"mycharstyle"_ustr, uno::Any(xCharStyle));
    xParaStyle->setPropertyValue(u"LinkStyle"_ustr, uno::Any(u"mycharstyle"_ustr));
    xCharStyle->setPropertyValue(u"LinkStyle"_ustr, uno::Any(u"myparastyle"_ustr));

    // When deleting the paragraph style (and only that):
    xParaStyles->removeByName(u"myparastyle"_ustr);

    // Then make sure we don't crash on save:
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue(u"FilterName"_ustr, u"writer8"_ustr),
    };
    xStorable->storeAsURL(maTempFile.GetURL(), aArgs);
}

namespace
{
/// This selection listener calls getAnchor() on selection change, which creates UNO cursors and is
/// invoked in the middle of a bookmark deletion.
struct SelectionChangeListener : public cppu::WeakImplHelper<view::XSelectionChangeListener>
{
    uno::Reference<container::XNameAccess> m_xBookmarks;
    std::vector<uno::Reference<text::XTextRange>> m_aAnchors;

public:
    SelectionChangeListener(const uno::Reference<container::XNameAccess>& xBookmarks);
    // view::XSelectionChangeListener
    void SAL_CALL selectionChanged(const lang::EventObject& rEvent) override;

    // lang::XEventListener
    void SAL_CALL disposing(const lang::EventObject& rSource) override;
};
}

SelectionChangeListener::SelectionChangeListener(
    const uno::Reference<container::XNameAccess>& xBookmarks)
    : m_xBookmarks(xBookmarks)
{
}

void SelectionChangeListener::selectionChanged(const lang::EventObject& /*rEvent*/)
{
    uno::Sequence<OUString> aElementNames = m_xBookmarks->getElementNames();
    for (const auto& rName : aElementNames)
    {
        uno::Reference<text::XTextContent> xTextContent(m_xBookmarks->getByName(rName),
                                                        uno::UNO_QUERY);
        m_aAnchors.push_back(xTextContent->getAnchor());
    }
}

void SelectionChangeListener::disposing(const lang::EventObject& /*rSource*/) {}

CPPUNIT_TEST_FIXTURE(SwCoreDocTest, testBookmarkDeleteListeners)
{
    // Given a document with 2 bookmarks:
    createSwDoc();
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    {
        xText->insertString(xCursor, u"test"_ustr, /*bAbsorb=*/false);
        xCursor->gotoStart(/*bExpand=*/false);
        xCursor->gotoEnd(/*bExpand=*/true);
        uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
        uno::Reference<text::XTextContent> xBookmark(
            xFactory->createInstance(u"com.sun.star.text.Bookmark"_ustr), uno::UNO_QUERY);
        uno::Reference<container::XNamed> xBookmarkNamed(xBookmark, uno::UNO_QUERY);
        xBookmarkNamed->setName(u"mybookmark"_ustr);
        xText->insertTextContent(xCursor, xBookmark, /*bAbsorb=*/true);
    }
    {
        xCursor->gotoEnd(/*bExpand=*/false);
        xText->insertString(xCursor, u"test2"_ustr, /*bAbsorb=*/false);
        xCursor->goLeft(4, /*bExpand=*/true);
        uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
        uno::Reference<text::XTextContent> xBookmark(
            xFactory->createInstance(u"com.sun.star.text.Bookmark"_ustr), uno::UNO_QUERY);
        uno::Reference<container::XNamed> xBookmarkNamed(xBookmark, uno::UNO_QUERY);
        xBookmarkNamed->setName(u"mybookmark2"_ustr);
        xText->insertTextContent(xCursor, xBookmark, /*bAbsorb=*/true);
    }
    uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xBookmarks = xBookmarksSupplier->getBookmarks();

    // When registering a selection listener that creates uno marks:
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<view::XSelectionSupplier> xController(xModel->getCurrentController(),
                                                         uno::UNO_QUERY);
    xController->addSelectionChangeListener(new SelectionChangeListener(xBookmarks));

    // Then make sure that deleting a bookmark doesn't crash:
    uno::Reference<lang::XComponent> xBookmark(xBookmarks->getByName(u"mybookmark2"_ustr),
                                               uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have crashed, an invalidated iterator
    // was used with erase().
    xBookmark->dispose();
}

CPPUNIT_TEST_FIXTURE(SwCoreDocTest, testBookmarkDeleteRedline)
{
    // Given a document with redlines, a mark (annotation mark) inside a redline:
    createSwDoc("bookmark-delete-redline.doc");
    SwDoc* pDoc = getSwDoc();

    // When hiding deletions / showing only inserts, make sure we don't crash:
    // Without the accompanying fix in place, this test would have crashed, equal_range() was used
    // on an unsorted container.
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::ShowInsert);
}

CPPUNIT_TEST_FIXTURE(SwCoreDocTest, testHeaderFooterDelete)
{
    // Given a document with bookmarks in header/footers:
    // When importing that document:
    // Then make sure that we don't crash:
    // Without the accompanying fix in place, this test would have crashed, an invalidated iterator
    // was used in sw::mark::MarkManager::deleteMarks().
    createSwDoc("header-footer-delete.docx");
}

CPPUNIT_TEST_FIXTURE(SwCoreDocTest, testSplitFlyChain)
{
    // Given a document with 2 fly frames, first is allowed to split, second is not:
    createSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    SwFlyFrameAttrMgr aMgr(true, pWrtShell, Frmmgr_Type::TEXT, nullptr);
    RndStdIds eAnchor = RndStdIds::FLY_AT_PARA;
    aMgr.InsertFlyFrame(eAnchor, aMgr.GetPos(), aMgr.GetSize());
    SwDoc* pDoc = getSwDoc();
    auto& rFlys = *pDoc->GetSpzFrameFormats();
    {
        pWrtShell->StartAllAction();
        auto pFly = rFlys[0];
        SwAttrSet aSet(pFly->GetAttrSet());
        aSet.Put(SwFormatFlySplit(true));
        pDoc->SetAttr(aSet, *pFly);
        pWrtShell->EndAllAction();
    }
    pWrtShell->UnSelectFrame();
    pWrtShell->SttEndDoc(/*bStart=*/false);
    aMgr.InsertFlyFrame(eAnchor, aMgr.GetPos(), aMgr.GetSize());
    auto pFly = rFlys[0];
    auto pFly2 = rFlys[1];

    // When checking if chaining is allowed:
    SwChainRet eActual = pDoc->Chainable(*pFly, *pFly2);
    // Then make sure the source is rejected if it is a split fly:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 5 (SwChainRet::SOURCE_CHAINED)
    // - Actual  : 0 (SwChainRet::OK)
    // i.e. the UI allowed chaining for floating tables, which doesn't make sense.
    CPPUNIT_ASSERT_EQUAL(SwChainRet::SOURCE_CHAINED, eActual);

    // Also test the other way around, that should not be OK, either.
    eActual = pDoc->Chainable(*pFly2, *pFly);
    CPPUNIT_ASSERT_EQUAL(SwChainRet::IS_IN_CHAIN, eActual);
}

CPPUNIT_TEST_FIXTURE(SwCoreDocTest, testSplitExpandGlossary)
{
    // Given a document with a split fly (2 pages) and a 'dt' at the end:
    createSwDoc("floating-table-dummy-text.docx");
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->SttEndDoc(/*bStt=*/false);

    // When expanding 'dt' to an actual dummy text:
    dispatchCommand(mxComponent, u".uno:ExpandGlossary"_ustr, {});

    // Then make sure the 2 fly frames stay on the 2 pages:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage1 = dynamic_cast<SwPageFrame*>(pLayout->Lower());
    CPPUNIT_ASSERT(pPage1);
    CPPUNIT_ASSERT(pPage1->GetSortedObjs());
    const SwSortedObjs& rPage1Objs = *pPage1->GetSortedObjs();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 2
    // i.e. both parts of the split fly chain were on page 1.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPage1Objs.size());
    auto pPage2 = dynamic_cast<SwPageFrame*>(pPage1->GetNext());
    CPPUNIT_ASSERT(pPage2);
    CPPUNIT_ASSERT(pPage2->GetSortedObjs());
    const SwSortedObjs& rPage2Objs = *pPage2->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPage2Objs.size());
}

CPPUNIT_TEST_FIXTURE(SwCoreDocTest, testSplitFlyInsertUndo)
{
    // Given a document with an inline table, which is then turned into a floating one:
    createSwDoc();
    SwDoc* pDoc = getSwDocShell()->GetDoc();
    sw::FrameFormats<sw::SpzFrameFormat*>& rFlyFormats = *pDoc->GetSpzFrameFormats();
    CPPUNIT_ASSERT(rFlyFormats.empty());
    // Insert a table:
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    SwInsertTableOptions aTableOptions(SwInsertTableFlags::DefaultBorder, 0);
    pWrtShell->InsertTable(aTableOptions, /*nRows=*/2, /*nCols=*/1);
    pWrtShell->MoveTable(GotoPrevTable, fnTableStart);
    pWrtShell->GoPrevCell();
    pWrtShell->Insert(u"A1"_ustr);
    pWrtShell->GoNextCell();
    pWrtShell->Insert(u"A2"_ustr);
    // Select cell:
    pWrtShell->SelAll();
    // Select table:
    pWrtShell->SelAll();
    // Wrap the table in a text frame:
    SwFlyFrameAttrMgr aMgr(true, pWrtShell, Frmmgr_Type::TEXT, nullptr);
    pWrtShell->StartAllAction();
    aMgr.InsertFlyFrame(RndStdIds::FLY_AT_PARA, aMgr.GetPos(), aMgr.GetSize());
    pWrtShell->EndAllAction();
    CPPUNIT_ASSERT(!rFlyFormats.empty());

    // When undoing the conversion to floating table:
    pDoc->GetUndoManager().Undo();

    // Then make sure we don't have a frame anymore:
    // Without the accompanying fix in place, this test would have failed, there was no undo action
    // to execute.
    CPPUNIT_ASSERT(rFlyFormats.empty());

    // And when redoing the conversion:
    pDoc->GetUndoManager().Redo();

    // Then make sure we again have a frame:
    CPPUNIT_ASSERT(!rFlyFormats.empty());
}

CPPUNIT_TEST_FIXTURE(SwCoreDocTest, testVirtPageNumReset)
{
    createSwDoc("tdf160843.odt");
    auto pWrtShell = getSwDocShell()->GetWrtShell();
    sal_uInt16 nPhys = 0;
    sal_uInt16 nVirt = 0;

    pWrtShell->GotoPage(1, false);
    pWrtShell->GetPageNum(nPhys, nVirt, true, false);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(1), nPhys);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(1), nVirt);

    pWrtShell->GotoPage(3, false);
    pWrtShell->GetPageNum(nPhys, nVirt, true, false);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(3), nPhys);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(1), nVirt);

    pWrtShell->GotoPage(5, false);
    pWrtShell->GetPageNum(nPhys, nVirt, true, false);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(5), nPhys);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(1), nVirt);
}

CPPUNIT_TEST_FIXTURE(SwCoreDocTest, testDefaultPageDescUsed)
{
    createSwDoc();
    auto pDoc = getSwDocShell()->GetDoc();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pDoc->GetPageDescCnt());
    const SwPageDesc& rPageDesc = pDoc->GetPageDesc(0);
    CPPUNIT_ASSERT(rPageDesc.IsUsed());
}

CPPUNIT_TEST_FIXTURE(SwCoreDocTest, testTextBoxWordWrap)
{
    // Given a document with a shape in the header that extends horizontally when there is enough
    // content:
    createSwDoc("text-box-word-wrap.docx");

    // When checking the layout size of the shape:
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    auto nFlyHeight = getXPath(pXmlDoc, "//anchored/fly/infos/bounds", "height").toInt32();

    // Then make sure it has a small height, hosting just one line:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 773
    // - Actual  : 5183
    // i.e. the shape had new lines for each character instead of 1 line.
    CPPUNIT_ASSERT_LESS(static_cast<sal_Int32>(1000), nFlyHeight);
}

CPPUNIT_TEST_FIXTURE(SwCoreDocTest, testAtCharImageCopy)
{
    // Given a document with 2 pages, one draw object on both pages:
    createSwDoc("at-char-image-copy.odt");
    SwWrtShell* pWrtShell1 = getSwDocShell()->GetWrtShell();
    pWrtShell1->SelAll();
    rtl::Reference<SwTransferable> xTransfer = new SwTransferable(*pWrtShell1);
    xTransfer->Copy();
    // Don't use createSwDoc(), UnoApiTest::loadWithParams() would dispose the first document.
    mxComponent2 = loadFromDesktop(u"private:factory/swriter"_ustr);

    // When copying the body text from that document to a new one:
    auto pXTextDocument2 = dynamic_cast<SwXTextDocument*>(mxComponent2.get());
    SwDocShell* pDocShell2 = pXTextDocument2->GetDocShell();
    SwWrtShell* pWrtShell2 = pDocShell2->GetWrtShell();
    TransferableDataHelper aHelper(xTransfer);
    SwTransferable::Paste(*pWrtShell2, aHelper);

    // Then make sure the new document also has the 2 images on the 2 pages:
    SwDoc* pDoc2 = pDocShell2->GetDoc();
    SwRootFrame* pLayout2 = pDoc2->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage1 = pLayout2->GetLower()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage1);
    CPPUNIT_ASSERT(pPage1->GetSortedObjs());
    const SwSortedObjs& rPage1Objs = *pPage1->GetSortedObjs();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 2
    // i.e. both images went to page 1.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPage1Objs.size());
    auto pPage2 = pPage1->GetNext()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage2);
    CPPUNIT_ASSERT(pPage2->GetSortedObjs());
    const SwSortedObjs& rPage2Objs = *pPage2->GetSortedObjs();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPage2Objs.size());
}

CPPUNIT_TEST_FIXTURE(SwCoreDocTest, testEditListAutofmt)
{
    // Given a document with a number portion in para 2, para marker is formatted to be red:
    createSwDoc("edit-list-autofmt.docx");
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->EndPara();

    // When changing that red to be black:
    uno::Sequence<beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue("Color.Color", static_cast<sal_Int32>(COL_BLACK)),
    };
    dispatchCommand(mxComponent, ".uno:Color", aArgs);

    // Then make sure the layout turns into red:
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 00000000
    // - Actual  : 00ff0000
    // i.e. clicking on the toolbar button didn't result in a layout update because the doc model
    // became inconsistent.
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout/SwFieldPortion/SwFont",
                "color", u"00000000");
}

CPPUNIT_TEST_FIXTURE(SwCoreDocTest, testInsThenDelRejectUndo)
{
    // Given a document with an outer insert redline and an inner delete redline:
    createSwDoc("ins-then-del.docx");

    // When rejecting the insert, undo, then re-rejecting:
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->RejectRedline(0);
    SwDoc* pDoc = getSwDocShell()->GetDoc();
    IDocumentRedlineAccess& rIDRA = pDoc->getIDocumentRedlineAccess();
    SwRedlineTable& rRedlines = rIDRA.GetRedlineTable();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), rRedlines.size());
    pWrtShell->Undo();
    pWrtShell->SttEndDoc(/*bStt=*/true);
    pWrtShell->RejectRedline(0);

    // Then make sure that the reject of insert also gets rid of the delete on top of it:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 2
    // i.e. initially the doc had no redlines after insert, but undo + doing it again resulted in
    // redlines, which is inconsistent.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), rRedlines.size());

    // And given a reset state, matching the one after import:
    pWrtShell->Undo();
    {
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), rRedlines.size());
        CPPUNIT_ASSERT_EQUAL(RedlineType::Insert, rRedlines[0]->GetType());
        const SwRedlineData& rRedlineData1 = rRedlines[1]->GetRedlineData(0);
        CPPUNIT_ASSERT_EQUAL(RedlineType::Delete, rRedlineData1.GetType());
        CPPUNIT_ASSERT(rRedlineData1.Next());
        const SwRedlineData& rInnerRedlineData = *rRedlineData1.Next();
        CPPUNIT_ASSERT_EQUAL(RedlineType::Insert, rInnerRedlineData.GetType());
        CPPUNIT_ASSERT_EQUAL(RedlineType::Insert, rRedlines[2]->GetType());
    }

    // When rejecting the insert-then-delete + undo:
    SwCursor* pCursor = pWrtShell->GetCursor();
    pCursor->DeleteMark();
    pWrtShell->SttEndDoc(/*bStt=*/true);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 4, /*bBasicCall=*/false);
    SwRedlineTable::size_type nRedline{};
    rRedlines.FindAtPosition(*pCursor->Start(), nRedline);
    // A redline is found.
    CPPUNIT_ASSERT_LESS(rRedlines.size(), nRedline);
    pWrtShell->RejectRedline(nRedline);
    pWrtShell->Undo();

    // Then make sure that the restored redline has 2 redlines datas: delete and insert:
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), rRedlines.size());
    CPPUNIT_ASSERT_EQUAL(RedlineType::Insert, rRedlines[0]->GetType());
    const SwRedlineData& rRedlineData1 = rRedlines[1]->GetRedlineData(0);
    CPPUNIT_ASSERT_EQUAL(RedlineType::Delete, rRedlineData1.GetType());
    // The insert "under" the delete was lost.
    CPPUNIT_ASSERT(rRedlineData1.Next());
    const SwRedlineData& rInnerRedlineData = *rRedlineData1.Next();
    CPPUNIT_ASSERT_EQUAL(RedlineType::Insert, rInnerRedlineData.GetType());
    CPPUNIT_ASSERT_EQUAL(RedlineType::Insert, rRedlines[2]->GetType());

    // And when rejecting the "ins" part of ins-then-del:
    pWrtShell->RejectRedline(0);

    // Then make sure "reject" (and no accept) was created on the undo stack:
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    int nAccepts = 0;
    auto pListUndoAction = dynamic_cast<SfxListUndoAction*>(rUndoManager.GetUndoAction());
    if (pListUndoAction)
    {
        for (const auto& rMarkedAction : pListUndoAction->maUndoActions)
        {
            auto pUndo = dynamic_cast<SwUndoRedline*>(rMarkedAction.pAction.get());
            if (!pUndo)
            {
                continue;
            }

            if (pUndo->GetUserId() == SwUndoId::ACCEPT_REDLINE)
            {
                ++nAccepts;
            }
        }
    }
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 1
    // i.e. an "accept" undo action was created by RejectRedline().
    CPPUNIT_ASSERT_EQUAL(0, nAccepts);
}

CPPUNIT_TEST_FIXTURE(SwCoreDocTest, testInsThenFormat)
{
    // Given a document with <ins>A<format>B</format>C</ins> style redlines:
    // When importing that document:
    createSwDoc("ins-then-format.docx");

    // Then make sure that both the insert and the format on top of it is in the model:
    SwDocShell* pDocShell = getSwDocShell();
    SwDoc* pDoc = pDocShell->GetDoc();
    IDocumentRedlineAccess& rIDRA = pDoc->getIDocumentRedlineAccess();
    SwRedlineTable& rRedlines = rIDRA.GetRedlineTable();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 3
    // - Actual  : 1
    // i.e. a single insert redline was created, format redline was lost on import.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), rRedlines.size());
    CPPUNIT_ASSERT_EQUAL(RedlineType::Insert, rRedlines[0]->GetType());
    const SwRedlineData& rRedlineData1 = rRedlines[1]->GetRedlineData(0);
    CPPUNIT_ASSERT_EQUAL(RedlineType::Format, rRedlineData1.GetType());
    CPPUNIT_ASSERT(rRedlineData1.Next());
    const SwRedlineData& rInnerRedlineData = *rRedlineData1.Next();
    CPPUNIT_ASSERT_EQUAL(RedlineType::Insert, rInnerRedlineData.GetType());
    CPPUNIT_ASSERT_EQUAL(RedlineType::Insert, rRedlines[2]->GetType());

    // And when accepting the insert:
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->AcceptRedline(0);

    // Then make sure only the format on top of insert remains:
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rRedlines.size());
    CPPUNIT_ASSERT_EQUAL(RedlineType::Format, rRedlines[0]->GetType());
    const SwRedlineData& rRedlineData = rRedlines[0]->GetRedlineData(0);
    // Without the accompanying fix in place, this test would have failed, the insert under format
    // was not accepted.
    CPPUNIT_ASSERT(!rRedlineData.Next());

    // And when rejecting the insert:
    pWrtShell->Undo();
    SwTextNode* pTextNode = pWrtShell->GetCursor()->GetPointNode().GetTextNode();
    CPPUNIT_ASSERT_EQUAL(u"AAABBBCCC"_ustr, pTextNode->GetText());
    pWrtShell->RejectRedline(0);

    // Then make sure no redlines and no content remain:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 1
    // i.e. a format-on-insert redline remained.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), rRedlines.size());
    // Also make sure the text of the format-on-insert redline is removed.
    CPPUNIT_ASSERT(pTextNode->GetText().isEmpty());

    // And when accepting the format-on-insert:
    pWrtShell->Undo();
    CPPUNIT_ASSERT_EQUAL(u"AAABBBCCC"_ustr, pTextNode->GetText());
    SwPaM* pCursor = pWrtShell->GetCursor();
    pCursor->DeleteMark();
    pWrtShell->SttEndDoc(/*bStt=*/true);
    // Move inside "BBB".
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 4, /*bBasicCall=*/false);
    SwRedlineTable::size_type nRedline{};
    rRedlines.FindAtPosition(*pCursor->Start(), nRedline);
    pWrtShell->AcceptRedline(nRedline);

    // Then make sure the format is kept and the insert is accepted:
    pWrtShell->SttEndDoc(/*bStt=*/true);
    // Move inside "BBB".
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 4, /*bBasicCall=*/false);
    // Search from start in the table again, for a redline that covers the cursor position.
    nRedline = 0;
    const SwRangeRedline* pRedline = rRedlines.FindAtPosition(*pCursor->Start(), nRedline);
    // Without the accompanying fix in place, this test would have failed, there was no redline for
    // "BBB".
    CPPUNIT_ASSERT(pRedline);
    CPPUNIT_ASSERT_EQUAL(RedlineType::Format, pRedline->GetType());
    CPPUNIT_ASSERT(!pRedline->GetRedlineData().Next());

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 3
    // i.e. the insert redlines before/after BBB were not accepted.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rRedlines.size());

    // And when rejecting the format-on-insert:
    pWrtShell->Undo();
    CPPUNIT_ASSERT_EQUAL(u"AAABBBCCC"_ustr, pTextNode->GetText());
    // Undo() creates a new cursor.
    pCursor = pWrtShell->GetCursor();
    pCursor->DeleteMark();
    pWrtShell->SttEndDoc(/*bStt=*/true);
    // Move inside "BBB".
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 4, /*bBasicCall=*/false);
    nRedline = 0;
    rRedlines.FindAtPosition(*pCursor->Start(), nRedline);
    // A redline is found.
    CPPUNIT_ASSERT_LESS(rRedlines.size(), nRedline);
    pWrtShell->RejectRedline(nRedline);

    // Then make sure the format-on-insert is rejected, i.e. neither the format-on-insert BBB, nor
    // the surrounding AAA and CCC inserts are in the text anymore:
    // Without the accompanying fix in place, this test would have failed, the text was AAABBBCCC,
    // just the format of BBB was dropped.
    CPPUNIT_ASSERT(pTextNode->GetText().isEmpty());
}

CPPUNIT_TEST_FIXTURE(SwCoreDocTest, testDelThenFormat)
{
    // Given a document with <del>A<format>B</format>C</del> style redlines:
    // When importing that document:
    createSwDoc("del-then-format.docx");

    // When accepting the delete:
    SwDocShell* pDocShell = getSwDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->AcceptRedline(0);

    // Then make sure no redlines and no content remain:
    SwDoc* pDoc = pDocShell->GetDoc();
    IDocumentRedlineAccess& rIDRA = pDoc->getIDocumentRedlineAccess();
    SwRedlineTable& rRedlines = rIDRA.GetRedlineTable();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 1
    // i.e. the delete-then-format redline remained in the document.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), rRedlines.size());
    SwTextNode* pTextNode = pWrtShell->GetCursor()->GetPointNode().GetTextNode();
    // Also make sure the text of the format-on-delete redline is removed.
    CPPUNIT_ASSERT(pTextNode->GetText().isEmpty());

    // And when rejecting the delete:
    pWrtShell->Undo();
    CPPUNIT_ASSERT_EQUAL(u"AAABBBCCC"_ustr, pTextNode->GetText());
    pWrtShell->RejectRedline(0);

    // Then make sure the delete goes away, but the text node string and the format redline stays:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // i.e. the format redline on top of the delete was lost.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rRedlines.size());
    CPPUNIT_ASSERT_EQUAL(RedlineType::Format, rRedlines[0]->GetType());
    const SwRedlineData& rRedlineData = rRedlines[0]->GetRedlineData();
    CPPUNIT_ASSERT(!rRedlineData.Next());
    // This was AAACCC.
    CPPUNIT_ASSERT_EQUAL(u"AAABBBCCC"_ustr, pTextNode->GetText());

    // And when accepting the delete with the cursor inside BBB:
    pWrtShell->Undo();
    CPPUNIT_ASSERT_EQUAL(u"AAABBBCCC"_ustr, pTextNode->GetText());
    pWrtShell->KillPams();
    SwPaM* pCursor = pWrtShell->GetCursor();
    pCursor->DeleteMark();
    pWrtShell->SttEndDoc(/*bStt=*/true);
    // Move inside "BBB".
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 4, /*bBasicCall=*/false);
    SwRedlineTable::size_type nRedline{};
    rRedlines.FindAtPosition(*pCursor->Start(), nRedline);
    CPPUNIT_ASSERT_LESS(rRedlines.size(), nRedline);
    pWrtShell->AcceptRedline(nRedline);

    // Then make sure the format-on-delete is accepted, i.e. neither the format-on-delete BBB, nor
    // the surrounding AAA and CCC deletes are in the text anymore:
    // Without the accompanying fix in place, this test would have failed, the text was AAABBBCCC,
    // just the format of BBB was dropped.
    CPPUNIT_ASSERT(pTextNode->GetText().isEmpty());

    // And when rejecting the delete with the cursor inside BBB:
    pWrtShell->Undo();
    CPPUNIT_ASSERT_EQUAL(u"AAABBBCCC"_ustr, pTextNode->GetText());
    // Undo() creates a new cursor.
    pCursor = pWrtShell->GetCursor();
    pCursor->DeleteMark();
    pWrtShell->SttEndDoc(/*bStt=*/true);
    // Move inside "BBB".
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 4, /*bBasicCall=*/false);
    nRedline = 0;
    const SwRangeRedline* pRedline = rRedlines.FindAtPosition(*pCursor->Start(), nRedline);
    // A redline is found.
    CPPUNIT_ASSERT_LESS(rRedlines.size(), nRedline);
    pWrtShell->RejectRedline(nRedline);

    // Then make sure the format-on-delete is rejected, i.e. the delete part is gone but the format
    // part is kept:
    nRedline = 0;
    pRedline = rRedlines.FindAtPosition(*pCursor->Start(), nRedline);
    // Without the accompanying fix in place, this test would have failed, the redline over BBB was
    // gone completely.
    CPPUNIT_ASSERT(pRedline);
    CPPUNIT_ASSERT_EQUAL(RedlineType::Format, pRedline->GetType());
    CPPUNIT_ASSERT(!pRedline->GetRedlineData().Next());
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 3
    // i.e. the surrounding delete redlines were not combined on reject.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rRedlines.size());

    // Reset to the state after file load:
    pWrtShell->Undo();
    // And when we do a reject for the first "delete" part, undo, redo:
    pWrtShell->RejectRedline(0);
    pWrtShell->Undo();
    pWrtShell->Redo();

    // Then make sure get a single format redline, matching the state right after reject:
    {
        // Without the accompanying fix in place, this test would have failed with:
        // - Expected: 1
        // - Actual  : 0
        // i.e. the format redline was lost on redo.
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rRedlines.size());
        const SwRedlineData& rRedlineData1 = rRedlines[0]->GetRedlineData(0);
        CPPUNIT_ASSERT_EQUAL(RedlineType::Format, rRedlineData1.GetType());
        CPPUNIT_ASSERT(!rRedlineData1.Next());
    }
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
