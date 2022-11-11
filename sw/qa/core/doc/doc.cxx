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
#include <editeng/frmdiritem.hxx>
#include <vcl/errinf.hxx>
#include <vcl/event.hxx>
#include <editeng/langitem.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
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
#include <cmdid.h>
#include <unotxdoc.hxx>
#include <UndoManager.hxx>
#include <IDocumentRedlineAccess.hxx>

/// Covers sw/source/core/doc/ fixes.
class SwCoreDocTest : public SwModelTestBase
{
public:
    SwCoreDocTest()
        : SwModelTestBase("/sw/qa/core/doc/data/")
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
    createSwDoc("textbox-textrotateangle.odt");
    SwDoc* pDoc = getSwDoc();
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
    CPPUNIT_ASSERT_EQUAL(OUString("\tB"), pTextNode->GetText());
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
    createSwDoc("textbox-makeflyframe.docx");
    SwDoc* pDoc = getSwDoc();

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
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
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
    SwTextNode* pTextNode = pWrtShell->GetCursor()->GetPointNode().GetTextNode();
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
    createSwDoc();
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

CPPUNIT_TEST_FIXTURE(SwCoreDocTest, testContentControlDelete)
{
    // Given a document with a content control:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertString(xCursor, "test", /*bAbsorb=*/false);
    xCursor->gotoStart(/*bExpand=*/false);
    xCursor->gotoEnd(/*bExpand=*/true);
    uno::Reference<text::XTextContent> xContentControl(
        xMSF->createInstance("com.sun.star.text.ContentControl"), uno::UNO_QUERY);
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
    CPPUNIT_ASSERT_EQUAL(OUString("\x0001test\x0001"), pTextNode->GetText());
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
    SwFrameFormats& rFrameFormats = *pDoc->GetSpzFrameFormats();
    for (size_t i = 0; i < rFrameFormats.size(); ++i)
    {
        if (rFrameFormats[i]->GetName().indexOf("Copy") == -1)
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
        xFactory->createInstance("com.sun.star.style.ParagraphStyle"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xCharStyle(
        xFactory->createInstance("com.sun.star.style.CharacterStyle"), uno::UNO_QUERY);
    uno::Reference<container::XNameContainer> xParaStyles(getStyles("ParagraphStyles"),
                                                          uno::UNO_QUERY);
    xParaStyles->insertByName("myparastyle", uno::Any(xParaStyle));
    uno::Reference<container::XNameContainer> xCharStyles(getStyles("CharacterStyles"),
                                                          uno::UNO_QUERY);
    xCharStyles->insertByName("mycharstyle", uno::Any(xCharStyle));
    xParaStyle->setPropertyValue("LinkStyle", uno::Any(OUString("mycharstyle")));
    xCharStyle->setPropertyValue("LinkStyle", uno::Any(OUString("myparastyle")));

    // When deleting the paragraph style (and only that):
    xParaStyles->removeByName("myparastyle");

    // Then make sure we don't crash on save:
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue("FilterName", OUString("writer8")),
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
        xText->insertString(xCursor, "test", /*bAbsorb=*/false);
        xCursor->gotoStart(/*bExpand=*/false);
        xCursor->gotoEnd(/*bExpand=*/true);
        uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
        uno::Reference<text::XTextContent> xBookmark(
            xFactory->createInstance("com.sun.star.text.Bookmark"), uno::UNO_QUERY);
        uno::Reference<container::XNamed> xBookmarkNamed(xBookmark, uno::UNO_QUERY);
        xBookmarkNamed->setName("mybookmark");
        xText->insertTextContent(xCursor, xBookmark, /*bAbsorb=*/true);
    }
    {
        xCursor->gotoEnd(/*bExpand=*/false);
        xText->insertString(xCursor, "test2", /*bAbsorb=*/false);
        xCursor->goLeft(4, /*bExpand=*/true);
        uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
        uno::Reference<text::XTextContent> xBookmark(
            xFactory->createInstance("com.sun.star.text.Bookmark"), uno::UNO_QUERY);
        uno::Reference<container::XNamed> xBookmarkNamed(xBookmark, uno::UNO_QUERY);
        xBookmarkNamed->setName("mybookmark2");
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
    uno::Reference<lang::XComponent> xBookmark(xBookmarks->getByName("mybookmark2"),
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

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
