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
    SwWrtShell* pShell = pDoc->GetDocShell()->GetWrtShell();
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
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pDocShell = pDoc->GetDocShell();
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
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pDocShell = pDoc->GetDocShell();
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
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pDocShell = pDoc->GetDocShell();
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
    SwDoc* pDoc = getSwDoc();
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
    SwDoc* pDoc = getSwDoc();

    // When cutting the textbox and pasting it to a new document:
    selectShape(1);
    SwDocShell* pDocShell = pDoc->GetDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    rtl::Reference<SwTransferable> pTransfer = new SwTransferable(*pWrtShell);
    pTransfer->Cut();
    TransferableDataHelper aHelper(pTransfer);
    uno::Reference<lang::XComponent> xDoc2 = loadFromDesktop(
        u"private:factory/swriter"_ustr, u"com.sun.star.text.TextDocument"_ustr, {});
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
    assertXPath(pLayout, "/root/page/body/txt/anchored/fly"_ostr, 1);
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
    SwDocShell* pDocShell = pDoc->GetDocShell();
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
    SwDoc* pDoc = getSwDoc();
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
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
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
        if ((*it)->GetName().indexOf("Copy") == -1)
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
        if (pSpz->GetName().indexOf("Copy") == -1)
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

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
