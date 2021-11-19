/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <string>

#include <boost/property_tree/json_parser.hpp>

#include <com/sun/star/frame/DispatchResultState.hpp>
#include <com/sun/star/frame/XDispatchResultListener.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/datatransfer/XTransferable2.hpp>

#include <test/helper/transferable.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <comphelper/dispatchcommand.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/string.hxx>
#include <comphelper/lok.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdview.hxx>
#include <vcl/virdev.hxx>
#include <editeng/editview.hxx>
#include <editeng/outliner.hxx>
#include <editeng/wghtitem.hxx>
#include <svl/srchitem.hxx>
#include <svl/slstitm.hxx>
#include <svl/stritem.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/lokhelper.hxx>
#include <vcl/scheduler.hxx>
#include <vcl/vclevent.hxx>
#include <vcl/bitmapaccess.hxx>
#include <vcl/ITiledRenderable.hxx>
#include <tools/json_writer.hxx>
#include <unotools/mediadescriptor.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>
#include <test/lokcallback.hxx>

#include <drawdoc.hxx>
#include <ndtxt.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <UndoManager.hxx>
#include <cmdid.h>
#include <redline.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <flddat.hxx>
#include <basesh.hxx>
#include <unotxdoc.hxx>
#include <docsh.hxx>
#include <txtfrm.hxx>
#include <rootfrm.hxx>

char const DATA_DIRECTORY[] = "/sw/qa/extras/tiledrendering/data/";

static std::ostream& operator<<(std::ostream& os, ViewShellId id)
{
    os << static_cast<sal_Int32>(id);
    return os;
}

/// Testsuite for the SwXTextDocument methods implementing the vcl::ITiledRenderable interface.
class SwTiledRenderingTest : public SwModelTestBase
{
public:
    SwTiledRenderingTest();
    virtual void setUp() override;
    virtual void tearDown() override;
    void testRegisterCallback();
    void testPostKeyEvent();
    void testPostMouseEvent();
    void testSetTextSelection();
    void testGetTextSelection();
    void testGetTextSelectionLineLimit();
    void testSetGraphicSelection();
    void testResetSelection();
    void testInsertShape();
    void testSearch();
    void testSearchViewArea();
    void testSearchTextFrame();
    void testSearchTextFrameWrapAround();
    void testDocumentSizeChanged();
    void testSearchAll();
    void testSearchAllNotifications();
    void testPageDownInvalidation();
    void testPartHash();
    void testViewCursors();
    void testShapeViewCursors();
    void testMissingInvalidation();
    void testViewCursorVisibility();
    void testViewCursorCleanup();
    void testViewLock();
    void testTextEditViewInvalidations();
    void testUndoInvalidations();
    void testUndoLimiting();
    void testUndoReordering();
    void testUndoReorderingRedo();
    void testUndoReorderingMulti();
    void testUndoShapeLimiting();
    void testUndoDispatch();
    void testUndoRepairDispatch();
    void testShapeTextUndoShells();
    void testShapeTextUndoGroupShells();
    void testTrackChanges();
    void testTrackChangesCallback();
    void testRedlineUpdateCallback();
    void testSetViewGraphicSelection();
    void testCreateViewGraphicSelection();
    void testCreateViewTextSelection();
    void testRedlineColors();
    void testCommentEndTextEdit();
    void testCommentInsert();
    void testCursorPosition();
    void testPaintCallbacks();
    void testUndoRepairResult();
    void testRedoRepairResult();
    void testDisableUndoRepair();
    void testAllTrackedChanges();
    void testDocumentRepair();
    void testPageHeader();
    void testPageFooter();
    void testTdf115088();
    void testRedlineField();
    void testIMESupport();
    void testIMEFormattingAtEndOfParagraph();
    void testSplitNodeRedlineCallback();
    void testDeleteNodeRedlineCallback();
    void testVisCursorInvalidation();
    void testDeselectCustomShape();
    void testSemiTransparent();
    void testHighlightNumbering();
    void testPilcrowRedlining();
    void testClipText();
    void testAnchorTypes();
    void testLanguageStatus();
    void testRedlineNotificationDuringSave();
    void testHyperlink();
    void testFieldmark();
    void testDropDownFormFieldButton();
    void testDropDownFormFieldButtonEditing();
    void testDropDownFormFieldButtonNoSelection();
    void testDropDownFormFieldButtonNoItem();
    void testTablePaintInvalidate();
    void testSpellOnlineRenderParameter();
    void testExtTextInputReadOnly();
    void testBulletDeleteInvalidation();
    void testBulletNoNumInvalidation();
    void testBulletMultiDeleteInvalidation();
    void testCondCollCopy();
    void testMoveShapeHandle();

    CPPUNIT_TEST_SUITE(SwTiledRenderingTest);
    CPPUNIT_TEST(testRegisterCallback);
    CPPUNIT_TEST(testPostKeyEvent);
    CPPUNIT_TEST(testPostMouseEvent);
    CPPUNIT_TEST(testSetTextSelection);
    CPPUNIT_TEST(testGetTextSelection);
    CPPUNIT_TEST(testGetTextSelectionLineLimit);
    CPPUNIT_TEST(testSetGraphicSelection);
    CPPUNIT_TEST(testResetSelection);
    CPPUNIT_TEST(testInsertShape);
    CPPUNIT_TEST(testSearch);
    CPPUNIT_TEST(testSearchViewArea);
    CPPUNIT_TEST(testSearchTextFrame);
    CPPUNIT_TEST(testSearchTextFrameWrapAround);
    CPPUNIT_TEST(testDocumentSizeChanged);
    CPPUNIT_TEST(testSearchAll);
    CPPUNIT_TEST(testSearchAllNotifications);
    CPPUNIT_TEST(testPageDownInvalidation);
    CPPUNIT_TEST(testPartHash);
    CPPUNIT_TEST(testViewCursors);
    CPPUNIT_TEST(testShapeViewCursors);
    CPPUNIT_TEST(testMissingInvalidation);
    CPPUNIT_TEST(testViewCursorVisibility);
    CPPUNIT_TEST(testViewCursorCleanup);
    CPPUNIT_TEST(testViewLock);
    CPPUNIT_TEST(testTextEditViewInvalidations);
    CPPUNIT_TEST(testUndoInvalidations);
    CPPUNIT_TEST(testUndoLimiting);
    CPPUNIT_TEST(testUndoReordering);
    CPPUNIT_TEST(testUndoReorderingRedo);
    CPPUNIT_TEST(testUndoReorderingMulti);
    CPPUNIT_TEST(testUndoShapeLimiting);
    CPPUNIT_TEST(testUndoDispatch);
    CPPUNIT_TEST(testUndoRepairDispatch);
    CPPUNIT_TEST(testShapeTextUndoShells);
    CPPUNIT_TEST(testShapeTextUndoGroupShells);
    CPPUNIT_TEST(testTrackChanges);
    CPPUNIT_TEST(testTrackChangesCallback);
    CPPUNIT_TEST(testRedlineUpdateCallback);
    CPPUNIT_TEST(testSetViewGraphicSelection);
    CPPUNIT_TEST(testCreateViewGraphicSelection);
    CPPUNIT_TEST(testCreateViewTextSelection);
    CPPUNIT_TEST(testRedlineColors);
    CPPUNIT_TEST(testCommentEndTextEdit);
    CPPUNIT_TEST(testCommentInsert);
    CPPUNIT_TEST(testCursorPosition);
    CPPUNIT_TEST(testPaintCallbacks);
    CPPUNIT_TEST(testUndoRepairResult);
    CPPUNIT_TEST(testRedoRepairResult);
    CPPUNIT_TEST(testDisableUndoRepair);
    CPPUNIT_TEST(testAllTrackedChanges);
    CPPUNIT_TEST(testDocumentRepair);
    CPPUNIT_TEST(testPageHeader);
    CPPUNIT_TEST(testPageFooter);
    CPPUNIT_TEST(testTdf115088);
    CPPUNIT_TEST(testRedlineField);
    CPPUNIT_TEST(testIMESupport);
    CPPUNIT_TEST(testIMEFormattingAtEndOfParagraph);
    CPPUNIT_TEST(testSplitNodeRedlineCallback);
    CPPUNIT_TEST(testDeleteNodeRedlineCallback);
    CPPUNIT_TEST(testVisCursorInvalidation);
    CPPUNIT_TEST(testDeselectCustomShape);
    CPPUNIT_TEST(testSemiTransparent);
    CPPUNIT_TEST(testHighlightNumbering);
    CPPUNIT_TEST(testPilcrowRedlining);
    CPPUNIT_TEST(testClipText);
    CPPUNIT_TEST(testAnchorTypes);
    CPPUNIT_TEST(testLanguageStatus);
    CPPUNIT_TEST(testRedlineNotificationDuringSave);
    CPPUNIT_TEST(testHyperlink);
    CPPUNIT_TEST(testFieldmark);
    CPPUNIT_TEST(testDropDownFormFieldButton);
    CPPUNIT_TEST(testDropDownFormFieldButtonEditing);
    CPPUNIT_TEST(testDropDownFormFieldButtonNoSelection);
    CPPUNIT_TEST(testDropDownFormFieldButtonNoItem);
    CPPUNIT_TEST(testTablePaintInvalidate);
    CPPUNIT_TEST(testSpellOnlineRenderParameter);
    CPPUNIT_TEST(testExtTextInputReadOnly);
    CPPUNIT_TEST(testBulletDeleteInvalidation);
    CPPUNIT_TEST(testBulletNoNumInvalidation);
    CPPUNIT_TEST(testBulletMultiDeleteInvalidation);
    CPPUNIT_TEST(testCondCollCopy);
    CPPUNIT_TEST(testMoveShapeHandle);
    CPPUNIT_TEST_SUITE_END();

private:
    SwXTextDocument* createDoc(const char* pName = nullptr);
    void setupLibreOfficeKitViewCallback(SfxViewShell* pViewShell);
    static void callback(int nType, const char* pPayload, void* pData);
    void callbackImpl(int nType, const char* pPayload);
    // First invalidation.
    tools::Rectangle m_aInvalidation;
    /// Union of all invalidations.
    tools::Rectangle m_aInvalidations;
    Size m_aDocumentSize;
    OString m_aTextSelection;
    bool m_bFound;
    std::vector<OString> m_aSearchResultSelection;
    std::vector<int> m_aSearchResultPart;
    int m_nSelectionBeforeSearchResult;
    int m_nSelectionAfterSearchResult;
    int m_nInvalidations;
    int m_nRedlineTableSizeChanged;
    int m_nRedlineTableEntryModified;
    int m_nTrackedChangeIndex;
    OString m_sHyperlinkText;
    OString m_sHyperlinkLink;
    OString m_aFormFieldButton;
    OString m_ShapeSelection;
    TestLokCallbackWrapper m_callbackWrapper;
};

SwTiledRenderingTest::SwTiledRenderingTest()
    : m_bFound(true),
      m_nSelectionBeforeSearchResult(0),
      m_nSelectionAfterSearchResult(0),
      m_nInvalidations(0),
      m_nRedlineTableSizeChanged(0),
      m_nRedlineTableEntryModified(0),
      m_nTrackedChangeIndex(-1),
      m_callbackWrapper(&callback, this)
{
}

void SwTiledRenderingTest::setUp()
{
    SwModelTestBase::setUp();

    comphelper::LibreOfficeKit::setActive(true);
}

void SwTiledRenderingTest::tearDown()
{
    if (mxComponent.is())
    {
        SwXTextDocument* pTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
        if (pTextDocument)
        {
            SwWrtShell* pWrtShell = pTextDocument->GetDocShell()->GetWrtShell();
            if (pWrtShell)
            {
                pWrtShell->GetSfxViewShell()->setLibreOfficeKitViewCallback(nullptr);
            }
        }
        mxComponent->dispose();
        mxComponent.clear();
    }
    m_callbackWrapper.clear();
    comphelper::LibreOfficeKit::setActive(false);

    test::BootstrapFixture::tearDown();
}

SwXTextDocument* SwTiledRenderingTest::createDoc(const char* pName)
{
    if (!pName)
        loadURL("private:factory/swriter", nullptr);
    else
        load(DATA_DIRECTORY, pName);

    SwXTextDocument* pTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDocument);
    pTextDocument->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    return pTextDocument;
}

void SwTiledRenderingTest::setupLibreOfficeKitViewCallback(SfxViewShell* pViewShell)
{
    pViewShell->setLibreOfficeKitViewCallback(&m_callbackWrapper);
    m_callbackWrapper.setLOKViewId(SfxLokHelper::getView(pViewShell));
}

void SwTiledRenderingTest::callback(int nType, const char* pPayload, void* pData)
{
    static_cast<SwTiledRenderingTest*>(pData)->callbackImpl(nType, pPayload);
}

void SwTiledRenderingTest::callbackImpl(int nType, const char* pPayload)
{
    OString aPayload(pPayload);
    switch (nType)
    {
    case LOK_CALLBACK_INVALIDATE_TILES:
    {
        tools::Rectangle aInvalidation;
        uno::Sequence<OUString> aSeq = comphelper::string::convertCommaSeparated(OUString::createFromAscii(pPayload));
        if (OString("EMPTY") == pPayload)
            return;
        CPPUNIT_ASSERT(aSeq.getLength() == 4 || aSeq.getLength() == 5);
        aInvalidation.setX(aSeq[0].toInt32());
        aInvalidation.setY(aSeq[1].toInt32());
        aInvalidation.setWidth(aSeq[2].toInt32());
        aInvalidation.setHeight(aSeq[3].toInt32());
        if (m_aInvalidation.IsEmpty())
        {
            m_aInvalidation = aInvalidation;
        }
        m_aInvalidations.Union(aInvalidation);
        ++m_nInvalidations;
    }
    break;
    case LOK_CALLBACK_DOCUMENT_SIZE_CHANGED:
    {
        uno::Sequence<OUString> aSeq = comphelper::string::convertCommaSeparated(OUString::createFromAscii(pPayload));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), aSeq.getLength());
        m_aDocumentSize.setWidth(aSeq[0].toInt32());
        m_aDocumentSize.setHeight(aSeq[1].toInt32());
    }
    break;
    case LOK_CALLBACK_TEXT_SELECTION:
    {
        m_aTextSelection = pPayload;
        if (m_aSearchResultSelection.empty())
            ++m_nSelectionBeforeSearchResult;
        else
            ++m_nSelectionAfterSearchResult;
    }
    break;
    case LOK_CALLBACK_SEARCH_NOT_FOUND:
    {
        m_bFound = false;
    }
    break;
    case LOK_CALLBACK_SEARCH_RESULT_SELECTION:
    {
        m_aSearchResultSelection.clear();
        boost::property_tree::ptree aTree;
        std::stringstream aStream(pPayload);
        boost::property_tree::read_json(aStream, aTree);
        for (const boost::property_tree::ptree::value_type& rValue : aTree.get_child("searchResultSelection"))
        {
            m_aSearchResultSelection.emplace_back(rValue.second.get<std::string>("rectangles").c_str());
            m_aSearchResultPart.push_back(std::atoi(rValue.second.get<std::string>("part").c_str()));
        }
    }
    break;
    case LOK_CALLBACK_REDLINE_TABLE_SIZE_CHANGED:
    {
        ++m_nRedlineTableSizeChanged;
    }
    break;
    case LOK_CALLBACK_REDLINE_TABLE_ENTRY_MODIFIED:
    {
        ++m_nRedlineTableEntryModified;
    }
    break;
    case LOK_CALLBACK_STATE_CHANGED:
    {
        OString aTrackedChangeIndexPrefix(".uno:TrackedChangeIndex=");
        if (aPayload.startsWith(aTrackedChangeIndexPrefix))
        {
            OString sIndex = aPayload.copy(aTrackedChangeIndexPrefix.getLength());
            if (sIndex.isEmpty())
                m_nTrackedChangeIndex = -1;
            else
                m_nTrackedChangeIndex = sIndex.toInt32();
        }
    }
    break;
    case LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR:
    {
        if (comphelper::LibreOfficeKit::isViewIdForVisCursorInvalidation())
        {
            boost::property_tree::ptree aTree;
            std::stringstream aStream(pPayload);
            boost::property_tree::read_json(aStream, aTree);
            boost::property_tree::ptree &aChild = aTree.get_child("hyperlink");
            m_sHyperlinkText = aChild.get("text", "").c_str();
            m_sHyperlinkLink = aChild.get("link", "").c_str();
        }
    }
    break;
    case LOK_CALLBACK_FORM_FIELD_BUTTON:
    {
        m_aFormFieldButton = OString(pPayload);
    }
    break;
    case LOK_CALLBACK_GRAPHIC_SELECTION:
    {
        m_ShapeSelection = OString(pPayload);
    }
    break;
    }

}

void SwTiledRenderingTest::testRegisterCallback()
{
    SwXTextDocument* pXTextDocument = createDoc("dummy.fodt");
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    setupLibreOfficeKitViewCallback(pWrtShell->GetSfxViewShell());
    // Insert a character at the beginning of the document.
    pWrtShell->Insert("x");
    Scheduler::ProcessEventsToIdle();

    // Check that the top left 256x256px tile would be invalidated.
    CPPUNIT_ASSERT(!m_aInvalidation.IsEmpty());
    tools::Rectangle aTopLeft(0, 0, 256*15, 256*15); // 1 px = 15 twips, assuming 96 DPI.
    CPPUNIT_ASSERT(m_aInvalidation.IsOver(aTopLeft));
}

void SwTiledRenderingTest::testPostKeyEvent()
{
    SwXTextDocument* pXTextDocument = createDoc("dummy.fodt");
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    SwShellCursor* pShellCursor = pWrtShell->getShellCursor(false);
    // Did we manage to go after the first character?
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), pShellCursor->GetPoint()->nContent.GetIndex());

    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'x', 0);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 'x', 0);
    Scheduler::ProcessEventsToIdle();
    // Did we manage to insert the character after the first one?
    CPPUNIT_ASSERT_EQUAL(OUString("Axaa bbb."), pShellCursor->GetPoint()->nNode.GetNode().GetTextNode()->GetText());
}

void SwTiledRenderingTest::testPostMouseEvent()
{
    SwXTextDocument* pXTextDocument = createDoc("dummy.fodt");
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    SwShellCursor* pShellCursor = pWrtShell->getShellCursor(false);
    // Did we manage to go after the first character?
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), pShellCursor->GetPoint()->nContent.GetIndex());

    Point aStart = pShellCursor->GetSttPos();
    aStart.setX(aStart.getX() - 1000);
    pXTextDocument->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONDOWN, aStart.getX(), aStart.getY(), 1, MOUSE_LEFT, 0);
    pXTextDocument->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONUP, aStart.getX(), aStart.getY(), 1, MOUSE_LEFT, 0);
    Scheduler::ProcessEventsToIdle();
    // The new cursor position must be before the first word.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), pShellCursor->GetPoint()->nContent.GetIndex());
}

void SwTiledRenderingTest::testSetTextSelection()
{
    SwXTextDocument* pXTextDocument = createDoc("dummy.fodt");
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    // Move the cursor into the second word.
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, 5, /*bBasicCall=*/false);
    // Create a selection on the word.
    pWrtShell->SelWrd();
    SwShellCursor* pShellCursor = pWrtShell->getShellCursor(false);
    // Did we indeed manage to select the second word?
    CPPUNIT_ASSERT_EQUAL(OUString("bbb"), pShellCursor->GetText());

    // Now use setTextSelection() to move the start of the selection 1000 twips left.
    Point aStart = pShellCursor->GetSttPos();
    aStart.setX(aStart.getX() - 1000);
    pXTextDocument->setTextSelection(LOK_SETTEXTSELECTION_START, aStart.getX(), aStart.getY());
    // The new selection must include the first word, too -- but not the ending dot.
    CPPUNIT_ASSERT_EQUAL(OUString("Aaa bbb"), pShellCursor->GetText());

    // Next: test that LOK_SETTEXTSELECTION_RESET + LOK_SETTEXTSELECTION_END can be used to create a selection.
    pXTextDocument->setTextSelection(LOK_SETTEXTSELECTION_RESET, aStart.getX(), aStart.getY());
    pXTextDocument->setTextSelection(LOK_SETTEXTSELECTION_END, aStart.getX() + 1000, aStart.getY());
    CPPUNIT_ASSERT_EQUAL(OUString("Aaa b"), pShellCursor->GetText());
}

void SwTiledRenderingTest::testGetTextSelection()
{
    SwXTextDocument* pXTextDocument = createDoc("shape-with-text.fodt");
    // No crash, just empty output for unexpected mime type.
    CPPUNIT_ASSERT_EQUAL(OString(), apitest::helper::transferable::getTextSelection(pXTextDocument->getSelection(), "foo/bar"));

    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    // Move the cursor into the first word.
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, 2, /*bBasicCall=*/false);
    // Create a selection by on the word.
    pWrtShell->SelWrd();

    // Make sure that we selected text from the body text.
    CPPUNIT_ASSERT_EQUAL(OString("Hello"), apitest::helper::transferable::getTextSelection(pXTextDocument->getSelection(), "text/plain;charset=utf-8"));

    // Make sure we produce something for HTML.
    CPPUNIT_ASSERT(!apitest::helper::transferable::getTextSelection(pXTextDocument->getSelection(), "text/html").isEmpty());

    // Now select some shape text and check again.
    SdrPage* pPage = pWrtShell->GetDoc()->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pObject = pPage->GetObj(0);
    SdrView* pView = pWrtShell->GetDrawView();
    pView->SdrBeginTextEdit(pObject);
    CPPUNIT_ASSERT(pView->GetTextEditObject());
    EditView& rEditView = pView->GetTextEditOutlinerView()->GetEditView();
    ESelection aWordSelection(0, 0, 0, 5);
    rEditView.SetSelection(aWordSelection);
    CPPUNIT_ASSERT_EQUAL(OString("Shape"), apitest::helper::transferable::getTextSelection(pXTextDocument->getSelection(), "text/plain;charset=utf-8"));
}

void SwTiledRenderingTest::testGetTextSelectionLineLimit()
{
    const char sOriginalText[] = u8"Estonian employs the Latin script as the basis for its alphabet, which adds the letters ä, ö, ü, and õ, plus the later additions š and ž. The letters c, q, w, x and y are limited to proper names of foreign origin, and f, z, š, and ž appear in loanwords and foreign names only. Ö and Ü are pronounced similarly to their equivalents in Swedish and German. Unlike in standard German but like Swedish (when followed by 'r') and Finnish, Ä is pronounced [æ], as in English mat. The vowels Ä, Ö and Ü are clearly separate phonemes and inherent in Estonian, although the letter shapes come from German. The letter õ denotes /ɤ/, unrounded /o/, or a close-mid back unrounded vowel. It is almost identical to the Bulgarian ъ /ɤ̞/ and the Vietnamese ơ, and is also used to transcribe the Russian ы.";
    const char sExpectedHtml[] = u8"Estonian employs the <a href=\"https://en.wikipedia.org/wiki/Latin_script\">Latin script</a> as the basis for <a href=\"https://en.wikipedia.org/wiki/Estonian_alphabet\">its alphabet</a>, which adds the letters <a href=\"https://en.wikipedia.org/wiki/%C3%84\"><i>ä</i></a>, <a href=\"https://en.wikipedia.org/wiki/%C3%96\"><i>ö</i></a>, <a href=\"https://en.wikipedia.org/wiki/%C3%9C\"><i>ü</i></a>, and <a href=\"https://en.wikipedia.org/wiki/%C3%95\"><i>õ</i></a>, plus the later additions <a href=\"https://en.wikipedia.org/wiki/%C5%A0\"><i>š</i></a> and <a href=\"https://en.wikipedia.org/wiki/%C5%BD\"><i>ž</i></a>. The letters <i>c</i>, <i>q</i>, <i>w</i>, <i>x</i> and <i>y</i> are limited to <a href=\"https://en.wikipedia.org/wiki/Proper_names\">proper names</a> of foreign origin, and <i>f</i>, <i>z</i>, <i>š</i>, and <i>ž</i> appear in loanwords and foreign names only. <i>Ö</i> and <i>Ü</i> are pronounced similarly to their equivalents in Swedish and German. Unlike in standard German but like Swedish (when followed by 'r') and Finnish, <i>Ä</i> is pronounced [æ], as in English <i>mat</i>. The vowels Ä, Ö and Ü are clearly separate <a href=\"https://en.wikipedia.org/wiki/Phonemes\">phonemes</a> and inherent in Estonian, although the letter shapes come from German. The letter <a href=\"https://en.wikipedia.org/wiki/%C3%95\"><i>õ</i></a> denotes /ɤ/, unrounded /o/, or a <a href=\"https://en.wikipedia.org/wiki/Close-mid_back_unrounded_vowel\">close-mid back unrounded vowel</a>. It is almost identical to the <a href=\"https://en.wikipedia.org/wiki/Bulgarian_language\">Bulgarian</a> <a href=\"https://en.wikipedia.org/wiki/%D0%AA\">ъ</a> /ɤ̞/ and the <a href=\"https://en.wikipedia.org/wiki/Vietnamese_language\">Vietnamese</a> <a href=\"https://en.wikipedia.org/wiki/%C6%A0\">ơ</a>, and is also used to transcribe the Russian <a href=\"https://en.wikipedia.org/wiki/%D0%AB\">ы</a>.";

    SwXTextDocument* pXTextDocument = createDoc("estonian.odt");

    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    // Move the cursor into the first word.
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, 2, /*bBasicCall=*/false);
    // Create a selection.
    pWrtShell->SelAll();

    OString sPlainText = apitest::helper::transferable::getTextSelection(pXTextDocument->getSelection(), "text/plain;charset=utf-8");

    CPPUNIT_ASSERT_EQUAL(OString(sOriginalText), sPlainText.trim());

    OString sHtmlText = apitest::helper::transferable::getTextSelection(pXTextDocument->getSelection(), "text/html");

    int nStart = sHtmlText.indexOf(u8"Estonian");

    CPPUNIT_ASSERT(sHtmlText.match(sExpectedHtml, nStart));
}

void SwTiledRenderingTest::testSetGraphicSelection()
{
    SwXTextDocument* pXTextDocument = createDoc("shape.fodt");
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    SdrPage* pPage = pWrtShell->GetDoc()->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pObject = pPage->GetObj(0);
    pWrtShell->SelectObj(Point(), 0, pObject);
    SdrHdlList handleList(nullptr);
    pObject->AddToHdlList(handleList);
    // Make sure the rectangle has 8 handles: at each corner and at the center of each edge.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(8), handleList.GetHdlCount());
    // Take the bottom center one.
    SdrHdl* pHdl = handleList.GetHdl(6);
    CPPUNIT_ASSERT_EQUAL(int(SdrHdlKind::Lower), static_cast<int>(pHdl->GetKind()));
    tools::Rectangle aShapeBefore = pObject->GetSnapRect();
    // Resize.
    pXTextDocument->setGraphicSelection(LOK_SETGRAPHICSELECTION_START, pHdl->GetPos().getX(), pHdl->GetPos().getY());
    pXTextDocument->setGraphicSelection(LOK_SETGRAPHICSELECTION_END, pHdl->GetPos().getX(), pHdl->GetPos().getY() + 1000);
    tools::Rectangle aShapeAfter = pObject->GetSnapRect();
    // Check that a resize happened, but aspect ratio is not kept.
    CPPUNIT_ASSERT_EQUAL(aShapeBefore.getWidth(), aShapeAfter.getWidth());
    CPPUNIT_ASSERT_EQUAL(aShapeBefore.getHeight() + 1000, aShapeAfter.getHeight());
}

void SwTiledRenderingTest::testResetSelection()
{
    SwXTextDocument* pXTextDocument = createDoc("shape.fodt");
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    // Select one character.
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    SwShellCursor* pShellCursor = pWrtShell->getShellCursor(false);
    // We have a text selection.
    CPPUNIT_ASSERT(pShellCursor->HasMark());

    pXTextDocument->resetSelection();
    // We no longer have a text selection.
    CPPUNIT_ASSERT(!pShellCursor->HasMark());

    SdrPage* pPage = pWrtShell->GetDoc()->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pObject = pPage->GetObj(0);
    Point aPoint = pObject->GetSnapRect().Center();
    // Select the shape.
    pWrtShell->EnterSelFrameMode(&aPoint);
    // We have a graphic selection.
    CPPUNIT_ASSERT(pWrtShell->IsSelFrameMode());

    pXTextDocument->resetSelection();
    // We no longer have a graphic selection.
    CPPUNIT_ASSERT(!pWrtShell->IsSelFrameMode());
}

void SwTiledRenderingTest::testInsertShape()
{
    SwXTextDocument* pXTextDocument = createDoc("2-pages.odt");
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();

    pXTextDocument->setClientVisibleArea(tools::Rectangle(0, 0, 10000, 4000));
    comphelper::dispatchCommand(".uno:BasicShapes.circle", uno::Sequence<beans::PropertyValue>());

    // check that the shape was inserted in the visible area, not outside
    IDocumentDrawModelAccess &rDrawModelAccess = pWrtShell->GetDoc()->getIDocumentDrawModelAccess();
    SdrPage* pPage = rDrawModelAccess.GetDrawModel()->GetPage(0);
    SdrObject* pObject = pPage->GetObj(0);
    CPPUNIT_ASSERT_EQUAL(tools::Rectangle(3302, 302, 6698, 3698), pObject->GetSnapRect());

    // check that it is in the foreground layer
    CPPUNIT_ASSERT_EQUAL(rDrawModelAccess.GetHeavenId().get(), pObject->GetLayer().get());
}

static void lcl_search(bool bBackward)
{
    uno::Sequence<beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence(
    {
        {"SearchItem.SearchString", uno::makeAny(OUString("shape"))},
        {"SearchItem.Backward", uno::makeAny(bBackward)}
    }));
    comphelper::dispatchCommand(".uno:ExecuteSearch", aPropertyValues);
}

void SwTiledRenderingTest::testSearch()
{
    SwXTextDocument* pXTextDocument = createDoc("search.odt");
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    setupLibreOfficeKitViewCallback(pWrtShell->GetSfxViewShell());
    std::size_t nNode = pWrtShell->getShellCursor(false)->Start()->nNode.GetNode().GetIndex();

    // First hit, in the second paragraph, before the shape.
    lcl_search(false);
    CPPUNIT_ASSERT(!pWrtShell->GetDrawView()->GetTextEditObject());
    std::size_t nActual = pWrtShell->getShellCursor(false)->Start()->nNode.GetNode().GetIndex();
    CPPUNIT_ASSERT_EQUAL(nNode + 1, nActual);
    /// Make sure we get search result selection for normal find as well, not only find all.
    CPPUNIT_ASSERT(!m_aSearchResultSelection.empty());

    // Next hit, in the shape.
    lcl_search(false);
    CPPUNIT_ASSERT(pWrtShell->GetDrawView()->GetTextEditObject());

    // Next hit, in the shape, still.
    lcl_search(false);
    CPPUNIT_ASSERT(pWrtShell->GetDrawView()->GetTextEditObject());

    // Last hit, in the last paragraph, after the shape.
    lcl_search(false);
    CPPUNIT_ASSERT(!pWrtShell->GetDrawView()->GetTextEditObject());
    nActual = pWrtShell->getShellCursor(false)->Start()->nNode.GetNode().GetIndex();
    CPPUNIT_ASSERT_EQUAL(nNode + 7, nActual);

    // Now change direction and make sure that the first 2 hits are in the shape, but not the 3rd one.
    lcl_search(true);
    CPPUNIT_ASSERT(pWrtShell->GetDrawView()->GetTextEditObject());
    lcl_search(true);
    CPPUNIT_ASSERT(pWrtShell->GetDrawView()->GetTextEditObject());
    lcl_search(true);
    CPPUNIT_ASSERT(!pWrtShell->GetDrawView()->GetTextEditObject());
    nActual = pWrtShell->getShellCursor(false)->Start()->nNode.GetNode().GetIndex();
    CPPUNIT_ASSERT_EQUAL(nNode + 1, nActual);
}

void SwTiledRenderingTest::testSearchViewArea()
{
    SwXTextDocument* pXTextDocument = createDoc("search.odt");
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    // Go to the second page, 1-based.
    pWrtShell->GotoPage(2, false);
    SwShellCursor* pShellCursor = pWrtShell->getShellCursor(false);
    // Get the ~top left corner of the second page.
    Point aPoint = pShellCursor->GetSttPos();

    // Go back to the first page, search while the cursor is there, but the
    // visible area is the second page.
    pWrtShell->GotoPage(1, false);
    uno::Sequence<beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence(
    {
        {"SearchItem.SearchString", uno::makeAny(OUString("Heading"))},
        {"SearchItem.Backward", uno::makeAny(false)},
        {"SearchItem.SearchStartPointX", uno::makeAny(static_cast<sal_Int32>(aPoint.getX()))},
        {"SearchItem.SearchStartPointY", uno::makeAny(static_cast<sal_Int32>(aPoint.getY()))}
    }));
    comphelper::dispatchCommand(".uno:ExecuteSearch", aPropertyValues);
    // This was just "Heading", i.e. SwView::SearchAndWrap() did not search from only the top of the second page.
    CPPUNIT_ASSERT_EQUAL(OUString("Heading on second page"), pShellCursor->GetPoint()->nNode.GetNode().GetTextNode()->GetText());
}

void SwTiledRenderingTest::testSearchTextFrame()
{
    SwXTextDocument* pXTextDocument = createDoc("search.odt");
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    setupLibreOfficeKitViewCallback(pWrtShell->GetSfxViewShell());
    uno::Sequence<beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence(
    {
        {"SearchItem.SearchString", uno::makeAny(OUString("TextFrame"))},
        {"SearchItem.Backward", uno::makeAny(false)},
    }));
    comphelper::dispatchCommand(".uno:ExecuteSearch", aPropertyValues);
    // This was empty: nothing was highlighted after searching for 'TextFrame'.
    CPPUNIT_ASSERT(!m_aTextSelection.isEmpty());
}

void SwTiledRenderingTest::testSearchTextFrameWrapAround()
{
    SwXTextDocument* pXTextDocument = createDoc("search.odt");
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    setupLibreOfficeKitViewCallback(pWrtShell->GetSfxViewShell());
    uno::Sequence<beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence(
    {
        {"SearchItem.SearchString", uno::makeAny(OUString("TextFrame"))},
        {"SearchItem.Backward", uno::makeAny(false)},
    }));
    comphelper::dispatchCommand(".uno:ExecuteSearch", aPropertyValues);
    CPPUNIT_ASSERT(m_bFound);
    comphelper::dispatchCommand(".uno:ExecuteSearch", aPropertyValues);
    // This failed, i.e. the second time 'not found' was reported, instead of wrapping around.
    CPPUNIT_ASSERT(m_bFound);
}

void SwTiledRenderingTest::testDocumentSizeChanged()
{
    // Get the current document size.
    SwXTextDocument* pXTextDocument = createDoc("2-pages.odt");
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    setupLibreOfficeKitViewCallback(pWrtShell->GetSfxViewShell());
    Size aSize = pXTextDocument->getDocumentSize();

    // Delete the second page and see how the size changes.
    pWrtShell->Down(false);
    pWrtShell->DelLeft();
    // Document width should not change, this was 0.
    CPPUNIT_ASSERT_EQUAL(aSize.getWidth(), m_aDocumentSize.getWidth());
    // Document height should be smaller now.
    CPPUNIT_ASSERT(aSize.getHeight() > m_aDocumentSize.getHeight());
}

void SwTiledRenderingTest::testSearchAll()
{
    SwXTextDocument* pXTextDocument = createDoc("search.odt");
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    setupLibreOfficeKitViewCallback(pWrtShell->GetSfxViewShell());
    uno::Sequence<beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence(
    {
        {"SearchItem.SearchString", uno::makeAny(OUString("shape"))},
        {"SearchItem.Backward", uno::makeAny(false)},
        {"SearchItem.Command", uno::makeAny(static_cast<sal_uInt16>(SvxSearchCmd::FIND_ALL))},
    }));
    comphelper::dispatchCommand(".uno:ExecuteSearch", aPropertyValues);
    // This was 0; should be 2 results in the body text.
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(2), m_aSearchResultSelection.size());
    // Writer documents are always a single part.
    CPPUNIT_ASSERT_EQUAL(0, m_aSearchResultPart[0]);
}

void SwTiledRenderingTest::testSearchAllNotifications()
{
    SwXTextDocument* pXTextDocument = createDoc("search.odt");
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    setupLibreOfficeKitViewCallback(pWrtShell->GetSfxViewShell());
    // Reset notification counter before search.
    m_nSelectionBeforeSearchResult = 0;
    uno::Sequence<beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence(
    {
        {"SearchItem.SearchString", uno::makeAny(OUString("shape"))},
        {"SearchItem.Backward", uno::makeAny(false)},
        {"SearchItem.Command", uno::makeAny(static_cast<sal_uInt16>(SvxSearchCmd::FIND_ALL))},
    }));
    comphelper::dispatchCommand(".uno:ExecuteSearch", aPropertyValues);
    Scheduler::ProcessEventsToIdle();

    // This was 5, make sure that we get no notifications about selection changes during search.
    CPPUNIT_ASSERT_EQUAL(0, m_nSelectionBeforeSearchResult);
    // But we do get the selection afterwards.
    CPPUNIT_ASSERT(m_nSelectionAfterSearchResult > 0);
}

void SwTiledRenderingTest::testPageDownInvalidation()
{
    SwXTextDocument* pXTextDocument = createDoc("pagedown-invalidation.odt");
    uno::Sequence<beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence(
    {
        {".uno:HideWhitespace", uno::makeAny(true)},
    }));
    pXTextDocument->initializeForTiledRendering(aPropertyValues);
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    setupLibreOfficeKitViewCallback(pWrtShell->GetSfxViewShell());
    comphelper::dispatchCommand(".uno:PageDown", uno::Sequence<beans::PropertyValue>());

    // This was 2.
    CPPUNIT_ASSERT_EQUAL(0, m_nInvalidations);
}

void SwTiledRenderingTest::testPartHash()
{
    SwXTextDocument* pXTextDocument = createDoc("pagedown-invalidation.odt");
    int nParts = pXTextDocument->getParts();
    for (int it = 0; it < nParts; it++)
    {
        CPPUNIT_ASSERT(!pXTextDocument->getPartHash(it).isEmpty());
    }
}

namespace {

/// A view callback tracks callbacks invoked on one specific view.
class ViewCallback final
{
    SfxViewShell* mpViewShell;
    int mnView;
public:
    bool m_bOwnCursorInvalidated;
    int m_nOwnCursorInvalidatedBy;
    bool m_bOwnCursorAtOrigin;
    tools::Rectangle m_aOwnCursor;
    bool m_bViewCursorInvalidated;
    tools::Rectangle m_aViewCursor;
    bool m_bOwnSelectionSet;
    bool m_bViewSelectionSet;
    OString m_aViewSelection;
    bool m_bTilesInvalidated;
    bool m_bViewCursorVisible;
    bool m_bGraphicViewSelection;
    bool m_bGraphicSelection;
    bool m_bViewLock;
    /// Set if any callback was invoked.
    bool m_bCalled;
    /// Redline table size changed payload
    boost::property_tree::ptree m_aRedlineTableChanged;
    /// Redline table modified payload
    boost::property_tree::ptree m_aRedlineTableModified;
    /// Post-it / annotation payload.
    boost::property_tree::ptree m_aComment;
    TestLokCallbackWrapper m_callbackWrapper;

    ViewCallback(SfxViewShell* pViewShell = nullptr, std::function<void(ViewCallback&)> const & rBeforeInstallFunc = {})
        : m_bOwnCursorInvalidated(false),
          m_nOwnCursorInvalidatedBy(-1),
          m_bOwnCursorAtOrigin(false),
          m_bViewCursorInvalidated(false),
          m_bOwnSelectionSet(false),
          m_bViewSelectionSet(false),
          m_bTilesInvalidated(false),
          m_bViewCursorVisible(false),
          m_bGraphicViewSelection(false),
          m_bGraphicSelection(false),
          m_bViewLock(false),
          m_bCalled(false),
          m_callbackWrapper(&callback, this)
    {
        // Because one call-site wants to set the bool fields up before the callback is installed
        if (rBeforeInstallFunc)
            rBeforeInstallFunc(*this);

        mpViewShell = pViewShell ? pViewShell : SfxViewShell::Current();
        mpViewShell->setLibreOfficeKitViewCallback(&m_callbackWrapper);
        mnView = SfxLokHelper::getView();
        m_callbackWrapper.setLOKViewId( mnView );
    }

    ~ViewCallback()
    {
        SfxLokHelper::setView(mnView);
        mpViewShell->setLibreOfficeKitViewCallback(nullptr);
    }

    static void callback(int nType, const char* pPayload, void* pData)
    {
        static_cast<ViewCallback*>(pData)->callbackImpl(nType, pPayload);
    }

    void callbackImpl(int nType, const char* pPayload)
    {
        OString aPayload(pPayload);
        m_bCalled = true;
        switch (nType)
        {
        case LOK_CALLBACK_INVALIDATE_TILES:
        {
            m_bTilesInvalidated = true;
        }
        break;
        case LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR:
        {
            m_bOwnCursorInvalidated = true;

            OString sRect;
            if(comphelper::LibreOfficeKit::isViewIdForVisCursorInvalidation())
            {
                std::stringstream aStream(pPayload);
                boost::property_tree::ptree aTree;
                boost::property_tree::read_json(aStream, aTree);
                sRect = aTree.get_child("rectangle").get_value<std::string>().c_str();
                m_nOwnCursorInvalidatedBy = aTree.get_child("viewId").get_value<int>();
            }
            else
                sRect = aPayload;
            uno::Sequence<OUString> aSeq = comphelper::string::convertCommaSeparated(OUString::fromUtf8(sRect));
            if (OString("EMPTY") == pPayload)
                return;
            CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(4), aSeq.getLength());
            m_aOwnCursor.setX(aSeq[0].toInt32());
            m_aOwnCursor.setY(aSeq[1].toInt32());
            m_aOwnCursor.setWidth(aSeq[2].toInt32());
            m_aOwnCursor.setHeight(aSeq[3].toInt32());
            if (m_aOwnCursor.getX() == 0 && m_aOwnCursor.getY() == 0)
                m_bOwnCursorAtOrigin = true;
        }
        break;
        case LOK_CALLBACK_INVALIDATE_VIEW_CURSOR:
        {
            m_bViewCursorInvalidated = true;
            std::stringstream aStream(pPayload);
            boost::property_tree::ptree aTree;
            boost::property_tree::read_json(aStream, aTree);
            OString aRect = aTree.get_child("rectangle").get_value<std::string>().c_str();

            uno::Sequence<OUString> aSeq = comphelper::string::convertCommaSeparated(OUString::fromUtf8(aRect));
            if (OString("EMPTY") == pPayload)
                return;
            CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(4), aSeq.getLength());
            m_aViewCursor.setX(aSeq[0].toInt32());
            m_aViewCursor.setY(aSeq[1].toInt32());
            m_aViewCursor.setWidth(aSeq[2].toInt32());
            m_aViewCursor.setHeight(aSeq[3].toInt32());
        }
        break;
        case LOK_CALLBACK_TEXT_SELECTION:
        {
            m_bOwnSelectionSet = true;
        }
        break;
        case LOK_CALLBACK_TEXT_VIEW_SELECTION:
        {
            m_bViewSelectionSet = true;
            m_aViewSelection = aPayload;
        }
        break;
        case LOK_CALLBACK_VIEW_CURSOR_VISIBLE:
        {
            std::stringstream aStream(pPayload);
            boost::property_tree::ptree aTree;
            boost::property_tree::read_json(aStream, aTree);
            m_bViewCursorVisible = aTree.get_child("visible").get_value<std::string>() == "true";
        }
        break;
        case LOK_CALLBACK_GRAPHIC_VIEW_SELECTION:
        {
            std::stringstream aStream(pPayload);
            boost::property_tree::ptree aTree;
            boost::property_tree::read_json(aStream, aTree);
            m_bGraphicViewSelection = aTree.get_child("selection").get_value<std::string>() != "EMPTY";
        }
        break;
        case LOK_CALLBACK_GRAPHIC_SELECTION:
        {
            m_bGraphicSelection = aPayload != "EMPTY";
        }
        break;
        case LOK_CALLBACK_VIEW_LOCK:
        {
            std::stringstream aStream(pPayload);
            boost::property_tree::ptree aTree;
            boost::property_tree::read_json(aStream, aTree);
            m_bViewLock = aTree.get_child("rectangle").get_value<std::string>() != "EMPTY";
        }
        break;
        case LOK_CALLBACK_REDLINE_TABLE_SIZE_CHANGED:
        {
            m_aRedlineTableChanged.clear();
            std::stringstream aStream(pPayload);
            boost::property_tree::read_json(aStream, m_aRedlineTableChanged);
            m_aRedlineTableChanged = m_aRedlineTableChanged.get_child("redline");
        }
        break;
        case LOK_CALLBACK_REDLINE_TABLE_ENTRY_MODIFIED:
        {
            m_aRedlineTableModified.clear();
            std::stringstream aStream(pPayload);
            boost::property_tree::read_json(aStream, m_aRedlineTableModified);
            m_aRedlineTableModified = m_aRedlineTableModified.get_child("redline");
        }
        break;
        case LOK_CALLBACK_COMMENT:
        {
            m_aComment.clear();
            std::stringstream aStream(pPayload);
            boost::property_tree::read_json(aStream, m_aComment);
            m_aComment = m_aComment.get_child("comment");
        }
        break;
        }
    }
};

class TestResultListener : public cppu::WeakImplHelper<css::frame::XDispatchResultListener>
{
public:
    sal_uInt32 m_nDocRepair;

    TestResultListener() : m_nDocRepair(0)
    {
    }

    virtual void SAL_CALL dispatchFinished(const css::frame::DispatchResultEvent& rEvent) override
    {
        if (rEvent.State == frame::DispatchResultState::SUCCESS)
        {
            rEvent.Result >>= m_nDocRepair;
        }
    }

    virtual void SAL_CALL disposing(const css::lang::EventObject&) override
    {
    }
};

}

void SwTiledRenderingTest::testMissingInvalidation()
{
    // Create two views.
    SwXTextDocument* pXTextDocument = createDoc("dummy.fodt");
    ViewCallback aView1;
    int nView1 = SfxLokHelper::getView();
    SfxLokHelper::createView();
    ViewCallback aView2;
    int nView2 = SfxLokHelper::getView();

    // First view: put the cursor into the first word.
    SfxLokHelper::setView(nView1);
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, 1, /*bBasicCall=*/false);

    // Second view: select the first word.
    SfxLokHelper::setView(nView2);
    CPPUNIT_ASSERT(pXTextDocument->GetDocShell()->GetWrtShell() != pWrtShell);
    pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    pWrtShell->SelWrd();

    // Now delete the selected word and make sure both views are invalidated.
    Scheduler::ProcessEventsToIdle();
    aView1.m_bTilesInvalidated = false;
    aView2.m_bTilesInvalidated = false;
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::DELETE);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::DELETE);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(aView1.m_bTilesInvalidated);
    CPPUNIT_ASSERT(aView2.m_bTilesInvalidated);
}

void SwTiledRenderingTest::testViewCursors()
{
    SwXTextDocument* pXTextDocument = createDoc("dummy.fodt");
    ViewCallback aView1;
    SfxLokHelper::createView();
    ViewCallback aView2;

    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(aView1.m_bOwnCursorInvalidated);
    CPPUNIT_ASSERT(aView1.m_bViewCursorInvalidated);
    CPPUNIT_ASSERT(aView2.m_bOwnCursorInvalidated);
    // This failed: the cursor position of view1 was only known to view2 once
    // it changed.
    CPPUNIT_ASSERT(aView2.m_bViewCursorInvalidated);

    // Make sure that aView1 gets a view-only selection notification, while
    // aView2 gets a real selection notification.
    aView1.m_bOwnSelectionSet = false;
    aView1.m_bViewSelectionSet = false;
    aView2.m_bOwnSelectionSet = false;
    aView2.m_bViewSelectionSet = false;
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    // Move the cursor into the second word.
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, 5, /*bBasicCall=*/false);
    // Create a selection on the word.
    pWrtShell->SelWrd();
    Scheduler::ProcessEventsToIdle();
    SwShellCursor* pShellCursor = pWrtShell->getShellCursor(false);
    // Did we indeed manage to select the second word?
    CPPUNIT_ASSERT_EQUAL(OUString("bbb"), pShellCursor->GetText());
    CPPUNIT_ASSERT(!aView1.m_bOwnSelectionSet);
    // This failed, aView1 did not get notification about selection changes in
    // aView2.
    CPPUNIT_ASSERT(aView1.m_bViewSelectionSet);
    CPPUNIT_ASSERT(aView2.m_bOwnSelectionSet);
    CPPUNIT_ASSERT(!aView2.m_bViewSelectionSet);
}

void SwTiledRenderingTest::testShapeViewCursors()
{
    // Load a document and create a view, so we have 2 ones.
    SwXTextDocument* pXTextDocument = createDoc("shape.fodt");
    ViewCallback aView1;
    SfxLokHelper::createView();
    pXTextDocument->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ViewCallback aView2;
    SwWrtShell* pWrtShell2 = pXTextDocument->GetDocShell()->GetWrtShell();

    // Start shape text in the second view.
    SdrPage* pPage = pWrtShell2->GetDoc()->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pObject = pPage->GetObj(0);
    SdrView* pView = pWrtShell2->GetDrawView();
    pWrtShell2->GetView().BeginTextEdit(pObject, pView->GetSdrPageView(), pWrtShell2->GetWin());
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'x', 0);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 'x', 0);
    Scheduler::ProcessEventsToIdle();
    // Press a key in the second view, while the first one observes this.
    aView1.m_bViewCursorInvalidated = false;
    aView2.m_bOwnCursorInvalidated = false;
    const tools::Rectangle aLastOwnCursor1 = aView1.m_aOwnCursor;
    const tools::Rectangle aLastViewCursor1 = aView1.m_aViewCursor;
    const tools::Rectangle aLastOwnCursor2 = aView2.m_aOwnCursor;
    const tools::Rectangle aLastViewCursor2 = aView2.m_aViewCursor;

    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'y', 0);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 'y', 0);
    Scheduler::ProcessEventsToIdle();
    // Make sure that aView1 gets a view-only cursor notification, while
    // aView2 gets a real cursor notification.
    CPPUNIT_ASSERT_EQUAL(aView1.m_aOwnCursor, aLastOwnCursor1);
    CPPUNIT_ASSERT(aView1.m_bViewCursorInvalidated && aLastViewCursor1 != aView1.m_aViewCursor);
    CPPUNIT_ASSERT(aView2.m_bOwnCursorInvalidated && aLastOwnCursor2 != aView2.m_aOwnCursor);
    CPPUNIT_ASSERT_EQUAL(aLastViewCursor2, aView2.m_aViewCursor);
}

void SwTiledRenderingTest::testViewCursorVisibility()
{
    // Load a document that has a shape and create two views.
    SwXTextDocument* pXTextDocument = createDoc("shape.fodt");
    ViewCallback aView1;
    SfxLokHelper::createView();
    pXTextDocument->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ViewCallback aView2;
    // This failed, initially the view cursor in the second view wasn't visible.
    CPPUNIT_ASSERT(aView2.m_bViewCursorVisible);

    // Click on the shape in the second view.
    aView1.m_bViewCursorVisible = true;
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    SdrPage* pPage = pWrtShell->GetDoc()->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pObject = pPage->GetObj(0);
    Point aCenter = pObject->GetSnapRect().Center();
    pXTextDocument->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONDOWN, aCenter.getX(), aCenter.getY(), 1, MOUSE_LEFT, 0);
    pXTextDocument->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONUP, aCenter.getX(), aCenter.getY(), 1, MOUSE_LEFT, 0);
    Scheduler::ProcessEventsToIdle();
    // Make sure the "view/text" cursor of the first view gets a notification.
    CPPUNIT_ASSERT(!aView1.m_bViewCursorVisible);
}

void SwTiledRenderingTest::testViewCursorCleanup()
{
    // Load a document that has a shape and create two views.
    SwXTextDocument* pXTextDocument = createDoc("shape.fodt");
    ViewCallback aView1;
    int nView2 = SfxLokHelper::createView();
    pXTextDocument->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    {
        ViewCallback aView2;

        // Click on the shape in the second view.
        SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
        SdrPage* pPage = pWrtShell->GetDoc()->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
        SdrObject* pObject = pPage->GetObj(0);
        Point aCenter = pObject->GetSnapRect().Center();
        aView1.m_bGraphicViewSelection = false;
        pXTextDocument->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONDOWN, aCenter.getX(), aCenter.getY(), 1, MOUSE_LEFT, 0);
        pXTextDocument->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONUP, aCenter.getX(), aCenter.getY(), 1, MOUSE_LEFT, 0);
        Scheduler::ProcessEventsToIdle();
        // Make sure there is a graphic view selection on the first view.
        CPPUNIT_ASSERT(aView1.m_bGraphicViewSelection);
    }
    // Now destroy the second view.
    SfxLokHelper::destroyView(nView2);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1), SfxLokHelper::getViewsCount(0));
    // Make sure that the graphic view selection on the first view is cleaned up.
    CPPUNIT_ASSERT(!aView1.m_bGraphicViewSelection);
}

void SwTiledRenderingTest::testViewLock()
{
    // Load a document that has a shape and create two views.
    SwXTextDocument* pXTextDocument = createDoc("shape.fodt");
    ViewCallback aView1;
    SfxLokHelper::createView();
    pXTextDocument->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ViewCallback aView2;

    // Begin text edit in the second view and assert that the first gets a lock
    // notification.
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    SdrPage* pPage = pWrtShell->GetDoc()->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pObject = pPage->GetObj(0);
    SdrView* pView = pWrtShell->GetDrawView();
    aView1.m_bViewLock = false;
    pWrtShell->GetView().BeginTextEdit(pObject, pView->GetSdrPageView(), pWrtShell->GetWin());
    CPPUNIT_ASSERT(aView1.m_bViewLock);

    // End text edit in the second view, and assert that the lock is removed in
    // the first view.
    pWrtShell->EndTextEdit();
    CPPUNIT_ASSERT(!aView1.m_bViewLock);
}

void SwTiledRenderingTest::testTextEditViewInvalidations()
{
    // Load a document that has a shape and create two views.
    SwXTextDocument* pXTextDocument = createDoc("shape.fodt");
    ViewCallback aView1;
    SfxLokHelper::createView();
    pXTextDocument->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ViewCallback aView2;

    // Begin text edit in the second view.
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    SdrPage* pPage = pWrtShell->GetDoc()->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pObject = pPage->GetObj(0);
    SdrView* pView = pWrtShell->GetDrawView();
    pWrtShell->GetView().BeginTextEdit(pObject, pView->GetSdrPageView(), pWrtShell->GetWin());
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'x', 0);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 'x', 0);
    Scheduler::ProcessEventsToIdle();

    // Assert that both views are invalidated when pressing a key while in text edit.
    aView1.m_bTilesInvalidated = false;
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'y', 0);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 'y', 0);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT(aView1.m_bTilesInvalidated);

    pWrtShell->EndTextEdit();
}

void SwTiledRenderingTest::testUndoInvalidations()
{
    // Load a document and create two views.
    SwXTextDocument* pXTextDocument = createDoc("dummy.fodt");
    ViewCallback aView1;
    int nView1 = SfxLokHelper::getView();
    SfxLokHelper::createView();
    pXTextDocument->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ViewCallback aView2;
    SfxLokHelper::setView(nView1);

    // Insert a character the end of the document.
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    pWrtShell->EndOfSection();
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'c', 0);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 'c', 0);
    Scheduler::ProcessEventsToIdle();
    // ProcessEventsToIdle resets the view; set it again
    SfxLokHelper::setView(nView1);
    SwShellCursor* pShellCursor = pWrtShell->getShellCursor(false);
    CPPUNIT_ASSERT_EQUAL(OUString("Aaa bbb.c"), pShellCursor->GetPoint()->nNode.GetNode().GetTextNode()->GetText());

    // Undo and assert that both views are invalidated.
    Scheduler::ProcessEventsToIdle();
    aView1.m_bTilesInvalidated = false;
    aView2.m_bTilesInvalidated = false;
    comphelper::dispatchCommand(".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(aView1.m_bTilesInvalidated);
    // Undo was dispatched on the first view, this second view was not invalidated.
    CPPUNIT_ASSERT(aView2.m_bTilesInvalidated);
}

void SwTiledRenderingTest::testUndoLimiting()
{
    // Load a document and create two views.
    SwXTextDocument* pXTextDocument = createDoc("dummy.fodt");
    SwWrtShell* pWrtShell1 = pXTextDocument->GetDocShell()->GetWrtShell();
    int nView1 = SfxLokHelper::getView();
    int nView2 = SfxLokHelper::createView();
    pXTextDocument->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());

    // Insert a character the end of the document in the second view.
    SwWrtShell* pWrtShell2 = pXTextDocument->GetDocShell()->GetWrtShell();
    pWrtShell2->EndOfSection();
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'c', 0);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 'c', 0);
    Scheduler::ProcessEventsToIdle();
    SwShellCursor* pShellCursor = pWrtShell2->getShellCursor(false);
    CPPUNIT_ASSERT_EQUAL(OUString("Aaa bbb.c"), pShellCursor->GetPoint()->nNode.GetNode().GetTextNode()->GetText());

    // Assert that the first view can't undo, but the second view can.
    CPPUNIT_ASSERT(!pWrtShell1->GetLastUndoInfo(nullptr, nullptr, &pWrtShell1->GetView()));
    CPPUNIT_ASSERT(pWrtShell2->GetLastUndoInfo(nullptr, nullptr, &pWrtShell2->GetView()));

    SfxLokHelper::setView(nView1);
    SfxViewShell::Current()->setLibreOfficeKitViewCallback(nullptr);
    SfxLokHelper::setView(nView2);
    SfxViewShell::Current()->setLibreOfficeKitViewCallback(nullptr);
}

void SwTiledRenderingTest::testUndoReordering()
{
    // Create two views and a document of 2 paragraphs.
    SwXTextDocument* pXTextDocument = createDoc();
    SwWrtShell* pWrtShell1 = pXTextDocument->GetDocShell()->GetWrtShell();
    int nView1 = SfxLokHelper::getView();
    int nView2 = SfxLokHelper::createView();
    pXTextDocument->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    SwWrtShell* pWrtShell2 = pXTextDocument->GetDocShell()->GetWrtShell();
    pWrtShell2->SplitNode();
    SfxLokHelper::setView(nView1);
    pWrtShell1->SttEndDoc(/*bStt=*/true);
    SwTextNode* pTextNode1 = pWrtShell1->GetCursor()->GetNode().GetTextNode();
    // View 1 types into the first paragraph.
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'a', 0);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 'a', 0);
    Scheduler::ProcessEventsToIdle();
    SfxLokHelper::setView(nView2);
    pWrtShell2->SttEndDoc(/*bStt=*/false);
    SwTextNode* pTextNode2 = pWrtShell2->GetCursor()->GetNode().GetTextNode();
    // View 2 types into the second paragraph.
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'z', 0);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 'z', 0);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(OUString("a"), pTextNode1->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("z"), pTextNode2->GetText());

    // When view 1 presses undo:
    SfxLokHelper::setView(nView1);
    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    // Then make sure view 1's last undo action is invoked, out of order:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expression: pTextNode1->GetText().isEmpty()
    // i.e. the "a" in the first paragraph was not removed.
    CPPUNIT_ASSERT(pTextNode1->GetText().isEmpty());
    // Last undo action is not invoked, as it belongs to view 2.
    CPPUNIT_ASSERT_EQUAL(OUString("z"), pTextNode2->GetText());
    SfxLokHelper::setView(nView1);
    SfxViewShell::Current()->setLibreOfficeKitViewCallback(nullptr);
    SfxLokHelper::setView(nView2);
    SfxViewShell::Current()->setLibreOfficeKitViewCallback(nullptr);
}

void SwTiledRenderingTest::testUndoReorderingRedo()
{
    // Create two views and a document of 2 paragraphs.
    SwXTextDocument* pXTextDocument = createDoc();
    SwWrtShell* pWrtShell1 = pXTextDocument->GetDocShell()->GetWrtShell();
    int nView1 = SfxLokHelper::getView();
    int nView2 = SfxLokHelper::createView();
    pXTextDocument->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    SwWrtShell* pWrtShell2 = pXTextDocument->GetDocShell()->GetWrtShell();
    pWrtShell2->SplitNode();
    SfxLokHelper::setView(nView1);
    pWrtShell1->SttEndDoc(/*bStt=*/true);
    SwTextNode* pTextNode1 = pWrtShell1->GetCursor()->GetNode().GetTextNode();
    // View 1 types into the first paragraph, twice.
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'f', 0);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 'f', 0);
    Scheduler::ProcessEventsToIdle();
    // Go to the start of the paragraph, to avoid grouping.
    pWrtShell1->SttEndDoc(/*bStt=*/true);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 's', 0);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 's', 0);
    Scheduler::ProcessEventsToIdle();
    SfxLokHelper::setView(nView2);
    pWrtShell2->SttEndDoc(/*bStt=*/false);
    SwTextNode* pTextNode2 = pWrtShell2->GetCursor()->GetNode().GetTextNode();
    // View 2 types into the second paragraph.
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'z', 0);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 'z', 0);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(OUString("sf"), pTextNode1->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("z"), pTextNode2->GetText());

    // When view 1 presses undo, twice:
    SfxLokHelper::setView(nView1);
    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();
    // First just s(econd) is erased:
    CPPUNIT_ASSERT_EQUAL(OUString("f"), pTextNode1->GetText());
    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    // Then make sure view 1's undo actions are invoked, out of order:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expression: pTextNode1->GetText().isEmpty()
    // i.e. out of order undo was executed only once, not twice.
    CPPUNIT_ASSERT(pTextNode1->GetText().isEmpty());
    // The top undo action is not invoked, as it belongs to view 2.
    CPPUNIT_ASSERT_EQUAL(OUString("z"), pTextNode2->GetText());
    SfxLokHelper::setView(nView1);
    SfxViewShell::Current()->setLibreOfficeKitViewCallback(nullptr);
    SfxLokHelper::setView(nView2);
    SfxViewShell::Current()->setLibreOfficeKitViewCallback(nullptr);
}

void SwTiledRenderingTest::testUndoReorderingMulti()
{
    // Create two views and a document of 2 paragraphs.
    SwXTextDocument* pXTextDocument = createDoc();
    SwWrtShell* pWrtShell1 = pXTextDocument->GetDocShell()->GetWrtShell();
    int nView1 = SfxLokHelper::getView();
    int nView2 = SfxLokHelper::createView();
    pXTextDocument->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    SwWrtShell* pWrtShell2 = pXTextDocument->GetDocShell()->GetWrtShell();
    pWrtShell2->SplitNode();
    SfxLokHelper::setView(nView1);
    pWrtShell1->SttEndDoc(/*bStt=*/true);
    SwTextNode* pTextNode1 = pWrtShell1->GetCursor()->GetNode().GetTextNode();
    // View 1 types into the first paragraph.
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'a', 0);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 'a', 0);
    Scheduler::ProcessEventsToIdle();
    SfxLokHelper::setView(nView2);
    pWrtShell2->SttEndDoc(/*bStt=*/false);
    SwTextNode* pTextNode2 = pWrtShell2->GetCursor()->GetNode().GetTextNode();
    // View 2 types into the second paragraph, twice.
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'x', 0);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 'x', 0);
    Scheduler::ProcessEventsToIdle();
    // Go to the start of the paragraph, to avoid grouping.
    pWrtShell2->SttPara();
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'y', 0);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 'y', 0);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(OUString("a"), pTextNode1->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("yx"), pTextNode2->GetText());

    // When view 1 presses undo:
    SfxLokHelper::setView(nView1);
    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    // Then make sure view 1's undo action is invoked, out of order:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expression: pTextNode1->GetText().isEmpty()
    // i.e. out of order undo was not executed, the first paragrph was still "a".
    CPPUNIT_ASSERT(pTextNode1->GetText().isEmpty());
    // The top 2 undo actions are not invoked, as they belong to view 2.
    CPPUNIT_ASSERT_EQUAL(OUString("yx"), pTextNode2->GetText());
    SfxLokHelper::setView(nView1);
    SfxViewShell::Current()->setLibreOfficeKitViewCallback(nullptr);
    SfxLokHelper::setView(nView2);
    SfxViewShell::Current()->setLibreOfficeKitViewCallback(nullptr);
}

void SwTiledRenderingTest::testUndoShapeLimiting()
{
    // Load a document and create a view.
    SwXTextDocument* pXTextDocument = createDoc("shape.fodt");
    SwWrtShell* pWrtShell1 = pXTextDocument->GetDocShell()->GetWrtShell();
    int nView1 = SfxLokHelper::getView();
    int nView2 = SfxLokHelper::createView();
    pXTextDocument->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    SwWrtShell* pWrtShell2 = pXTextDocument->GetDocShell()->GetWrtShell();

    // Start shape text in the second view.
    SdrPage* pPage = pWrtShell2->GetDoc()->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pObject = pPage->GetObj(0);
    SdrView* pView = pWrtShell2->GetDrawView();
    pWrtShell2->GetView().BeginTextEdit(pObject, pView->GetSdrPageView(), pWrtShell2->GetWin());
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'x', 0);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 'x', 0);
    Scheduler::ProcessEventsToIdle();
    pWrtShell2->EndTextEdit();

    // Assert that the first view can't and the second view can undo the insertion.
    SwDoc* pDoc = pXTextDocument->GetDocShell()->GetDoc();
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    rUndoManager.SetView(&pWrtShell1->GetView());
    // This was 1: first view could undo the change of the second view.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), rUndoManager.GetUndoActionCount());
    rUndoManager.SetView(&pWrtShell2->GetView());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rUndoManager.GetUndoActionCount());

    rUndoManager.SetView(nullptr);

    SfxLokHelper::setView(nView1);
    SfxViewShell::Current()->setLibreOfficeKitViewCallback(nullptr);
    SfxLokHelper::setView(nView2);
    SfxViewShell::Current()->setLibreOfficeKitViewCallback(nullptr);
}

void SwTiledRenderingTest::testUndoDispatch()
{
    // Load a document and create two views.
    SwXTextDocument* pXTextDocument = createDoc("dummy.fodt");
    int nView1 = SfxLokHelper::getView();
    SfxLokHelper::createView();
    pXTextDocument->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    int nView2 = SfxLokHelper::getView();

    // Insert a character in the first view.
    SfxLokHelper::setView(nView1);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'c', 0);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 'c', 0);
    Scheduler::ProcessEventsToIdle();

    // Click before the first word in the second view.
    SfxLokHelper::setView(nView2);
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    SwShellCursor* pShellCursor = pWrtShell->getShellCursor(false);
    Point aStart = pShellCursor->GetSttPos();
    aStart.setX(aStart.getX() - 1000);
    pXTextDocument->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONDOWN, aStart.getX(), aStart.getY(), 1, MOUSE_LEFT, 0);
    pXTextDocument->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONUP, aStart.getX(), aStart.getY(), 1, MOUSE_LEFT, 0);
    Scheduler::ProcessEventsToIdle();
    uno::Reference<frame::XDesktop2> xDesktop = frame::Desktop::create(comphelper::getProcessComponentContext());
    uno::Reference<frame::XFrame> xFrame2 = xDesktop->getActiveFrame();

    // Now switch back to the first view, and make sure that the active frame is updated.
    SfxLokHelper::setView(nView1);
    uno::Reference<frame::XFrame> xFrame1 = xDesktop->getActiveFrame();
    // This failed: setView() did not update the active frame.
    CPPUNIT_ASSERT(xFrame1 != xFrame2);

    SfxLokHelper::setView(nView1);
    SfxViewShell::Current()->setLibreOfficeKitViewCallback(nullptr);
    SfxLokHelper::setView(nView2);
    SfxViewShell::Current()->setLibreOfficeKitViewCallback(nullptr);
}

void SwTiledRenderingTest::testUndoRepairDispatch()
{
    // Load a document and create two views.
    SwXTextDocument* pXTextDocument = createDoc("dummy.fodt");
    int nView1 = SfxLokHelper::getView();
    SfxLokHelper::createView();
    pXTextDocument->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    int nView2 = SfxLokHelper::getView();

    // Insert a character in the first view.
    SfxLokHelper::setView(nView1);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'c', 0);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 'c', 0);
    Scheduler::ProcessEventsToIdle();

    // Assert that by default the second view can't undo the action.
    SfxLokHelper::setView(nView2);
    SwDoc* pDoc = pXTextDocument->GetDocShell()->GetDoc();
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rUndoManager.GetUndoActionCount());
    comphelper::dispatchCommand(".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rUndoManager.GetUndoActionCount());

    // But the same is allowed in repair mode.
    SfxLokHelper::setView(nView2);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rUndoManager.GetUndoActionCount());
    uno::Sequence<beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence(
    {
        {"Repair", uno::makeAny(true)}
    }));
    comphelper::dispatchCommand(".uno:Undo", aPropertyValues);
    Scheduler::ProcessEventsToIdle();
    // This was 1: repair mode couldn't undo the action, either.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), rUndoManager.GetUndoActionCount());

    SfxLokHelper::setView(nView1);
    SfxViewShell::Current()->setLibreOfficeKitViewCallback(nullptr);
    SfxLokHelper::setView(nView2);
    SfxViewShell::Current()->setLibreOfficeKitViewCallback(nullptr);
}

void SwTiledRenderingTest::testShapeTextUndoShells()
{
    // Load a document and create a view.
    SwXTextDocument* pXTextDocument = createDoc("shape.fodt");
    sal_Int32 nView1 = SfxLokHelper::getView();

    // Begin text edit.
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    SdrPage* pPage = pWrtShell->GetDoc()->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pObject = pPage->GetObj(0);
    SdrView* pView = pWrtShell->GetDrawView();
    pWrtShell->GetView().BeginTextEdit(pObject, pView->GetSdrPageView(), pWrtShell->GetWin());
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'x', 0);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 'x', 0);
    Scheduler::ProcessEventsToIdle();
    pWrtShell->EndTextEdit();

    // Make sure that the undo item remembers who created it.
    SwDoc* pDoc = pXTextDocument->GetDocShell()->GetDoc();
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    CPPUNIT_ASSERT_EQUAL(size_t(1), rUndoManager.GetUndoActionCount());
    CPPUNIT_ASSERT_EQUAL(OUString("Edit text of Shape 'Shape1'"), rUndoManager.GetUndoActionComment(0));

    // This was -1: the view shell id for the undo action wasn't known.
    CPPUNIT_ASSERT_EQUAL(ViewShellId(nView1), rUndoManager.GetUndoAction()->GetViewShellId());
}

void SwTiledRenderingTest::testShapeTextUndoGroupShells()
{
    // Load a document and create a view.
    SwXTextDocument* pXTextDocument = createDoc("shape.fodt");
    ViewCallback aView1;
    sal_Int32 nView1 = SfxLokHelper::getView();

    // Begin text edit.
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    SdrPage* pPage = pWrtShell->GetDoc()->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pObject = pPage->GetObj(0);
    SdrView* pView = pWrtShell->GetDrawView();
    pWrtShell->GetView().BeginTextEdit(pObject, pView->GetSdrPageView(), pWrtShell->GetWin());
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'x', 0);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 'x', 0);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::BACKSPACE);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::BACKSPACE);
    Scheduler::ProcessEventsToIdle();

    // Make sure that the undo item remembers who created it.
    SwDoc* pDoc = pXTextDocument->GetDocShell()->GetDoc();
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    CPPUNIT_ASSERT_EQUAL(size_t(0), rUndoManager.GetUndoActionCount());

    pWrtShell->EndTextEdit();
    pWrtShell->GetView().BeginTextEdit(pObject, pView->GetSdrPageView(), pWrtShell->GetWin());

    CPPUNIT_ASSERT_EQUAL(size_t(1), rUndoManager.GetUndoActionCount());
    CPPUNIT_ASSERT_EQUAL(OUString("Edit text of Shape 'Shape1'"), rUndoManager.GetUndoActionComment(0));

    // This was -1: the view shell id for the (top) undo list action wasn't known.
    CPPUNIT_ASSERT_EQUAL(ViewShellId(nView1), rUndoManager.GetUndoAction()->GetViewShellId());

    // Create an editeng text selection in the first view.
    EditView& rEditView = pView->GetTextEditOutlinerView()->GetEditView();
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'x', 0);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 'x', 0);
    Scheduler::ProcessEventsToIdle();
    // 0th para, 0th char -> 0th para, 1st char.
    ESelection aWordSelection(0, 0, 0, 1);
    rEditView.SetSelection(aWordSelection);

    // Create a second view, and make sure that the new view sees the same
    // cursor position as the old one.
    SfxLokHelper::createView();
    pXTextDocument->initializeForTiledRendering({});
    ViewCallback aView2;
    // Difference was 935 twips, the new view didn't see the editeng cursor of
    // the old one. The new difference should be <1px, but here we deal with twips.
    CPPUNIT_ASSERT(std::abs(aView1.m_aOwnCursor.Top() - aView2.m_aViewCursor.Top()) < 10);
    // This was false, editeng text selection of the first view wasn't noticed
    // by the second view.
    CPPUNIT_ASSERT(aView2.m_bViewSelectionSet);
    // This was false, the new view wasn't aware of the shape text lock created
    // by the old view.
    CPPUNIT_ASSERT(aView2.m_bViewLock);
}

void SwTiledRenderingTest::testTrackChanges()
{
    // Load a document.
    SwXTextDocument* pXTextDocument = createDoc("dummy.fodt");

    // Turn on track changes, type "zzz" at the end, and move to the start.
    uno::Reference<beans::XPropertySet> xPropertySet(mxComponent, uno::UNO_QUERY);
    xPropertySet->setPropertyValue("RecordChanges", uno::makeAny(true));
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    ViewCallback aView(pWrtShell->GetSfxViewShell());
    pWrtShell->EndOfSection();
    pWrtShell->Insert("zzz");
    pWrtShell->StartOfSection();

    // Get the redline just created
    const SwRedlineTable& rTable = pWrtShell->GetDoc()->getIDocumentRedlineAccess().GetRedlineTable();
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(1), rTable.size());
    SwRangeRedline* pRedline = rTable[0];

    // Reject the change by id, while the cursor does not cover the tracked change.
    uno::Sequence<beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence(
    {
        {"RejectTrackedChange", uno::makeAny(static_cast<sal_uInt16>(pRedline->GetId()))}
    }));
    comphelper::dispatchCommand(".uno:RejectTrackedChange", aPropertyValues);
    Scheduler::ProcessEventsToIdle();

    // Assert that the reject was performed.
    SwShellCursor* pShellCursor = pWrtShell->getShellCursor(false);
    // This was 'Aaa bbb.zzz', the change wasn't rejected.
    CPPUNIT_ASSERT_EQUAL(OUString("Aaa bbb."), pShellCursor->GetPoint()->nNode.GetNode().GetTextNode()->GetText());
}

void SwTiledRenderingTest::testTrackChangesCallback()
{
    // Load a document.
    SwXTextDocument* pXTextDocument = createDoc("dummy.fodt");
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    setupLibreOfficeKitViewCallback(pWrtShell->GetSfxViewShell());

    // Turn on track changes and type "x".
    uno::Reference<beans::XPropertySet> xPropertySet(mxComponent, uno::UNO_QUERY);
    xPropertySet->setPropertyValue("RecordChanges", uno::makeAny(true));
    m_nRedlineTableSizeChanged = 0;
    pWrtShell->Insert("x");

    // Assert that we get exactly one notification about the redline insert.
    // This was 0, as LOK_CALLBACK_REDLINE_TABLE_SIZE_CHANGED wasn't sent.
    CPPUNIT_ASSERT_EQUAL(1, m_nRedlineTableSizeChanged);

    CPPUNIT_ASSERT_EQUAL(-1, m_nTrackedChangeIndex);
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    SfxItemSet aSet(pWrtShell->GetDoc()->GetAttrPool(), svl::Items<FN_REDLINE_ACCEPT_DIRECT, FN_REDLINE_ACCEPT_DIRECT>{});
    SfxVoidItem aItem(FN_REDLINE_ACCEPT_DIRECT);
    aSet.Put(aItem);
    pWrtShell->GetView().GetState(aSet);
    // This failed, LOK_CALLBACK_STATE_CHANGED wasn't sent.
    CPPUNIT_ASSERT_EQUAL(0, m_nTrackedChangeIndex);
}

void SwTiledRenderingTest::testRedlineUpdateCallback()
{
    // Load a document.
    SwXTextDocument* pXTextDocument = createDoc("dummy.fodt");
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    setupLibreOfficeKitViewCallback(pWrtShell->GetSfxViewShell());

    // Turn on track changes, type "xx" and delete the second one.
    uno::Reference<beans::XPropertySet> xPropertySet(mxComponent, uno::UNO_QUERY);
    xPropertySet->setPropertyValue("RecordChanges", uno::makeAny(true));
    pWrtShell->Insert("xx");
    m_nRedlineTableEntryModified = 0;
    pWrtShell->DelLeft();

    // Assert that we get exactly one notification about the redline update.
    // This was 0, as LOK_CALLBACK_REDLINE_TABLE_ENTRY_MODIFIED wasn't sent.
    CPPUNIT_ASSERT_EQUAL(1, m_nRedlineTableEntryModified);

    // Turn off the change tracking mode, make some modification to left of the
    // redline so that its position changes
    xPropertySet->setPropertyValue("RecordChanges", uno::makeAny(false));
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    pWrtShell->Insert("This text is left of the redline");

    // Position of the redline has changed => Modify callback
    CPPUNIT_ASSERT_EQUAL(2, m_nRedlineTableEntryModified);

    pWrtShell->DelLeft();
    // Deletion also emits Modify callback
    CPPUNIT_ASSERT_EQUAL(3, m_nRedlineTableEntryModified);

    // Make changes to the right of the redline => no position change in redline
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, 100/*Go enough right */, /*bBasicCall=*/false);
    pWrtShell->Insert("This text is right of the redline");

    // No Modify callbacks
    CPPUNIT_ASSERT_EQUAL(3, m_nRedlineTableEntryModified);
}

void SwTiledRenderingTest::testSetViewGraphicSelection()
{
    // Load a document.
    SwXTextDocument* pXTextDocument = createDoc("frame.odt");
    int nView1 = SfxLokHelper::getView();
    ViewCallback aView1;
    // Create a second view, and switch back to the first view.
    SfxLokHelper::createView();
    pXTextDocument->initializeForTiledRendering({});
    SfxLokHelper::setView(nView1);

    // Mark the textframe in the first view.
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    SdrPage* pPage = pWrtShell->GetDoc()->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pObject = pPage->GetObj(0);
    SdrView* pView = pWrtShell->GetDrawView();
    pView->MarkObj(pObject, pView->GetSdrPageView());
    CPPUNIT_ASSERT(aView1.m_bGraphicSelection);

    // Now start to switch to the second view (part of setView()).
    pWrtShell->ShellLoseFocus();
    // This failed, mark handles were hidden in the first view.
    CPPUNIT_ASSERT(!pView->areMarkHandlesHidden());
}

void SwTiledRenderingTest::testCreateViewGraphicSelection()
{
    // Load a document.
    SwXTextDocument* pXTextDocument = createDoc("frame.odt");
    ViewCallback aView1;

    // Mark the textframe in the first view.
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    SdrPage* pPage = pWrtShell->GetDoc()->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pObject = pPage->GetObj(0);
    SdrView* pView = pWrtShell->GetDrawView();
    aView1.m_bGraphicSelection = true;
    pView->MarkObj(pObject, pView->GetSdrPageView());
    pWrtShell->HideCursor();
    CPPUNIT_ASSERT(aView1.m_bGraphicSelection);

    // Create a second view.
    SfxLokHelper::createView();
    // This was false, creating a second view cleared the selection of the
    // first one.
    CPPUNIT_ASSERT(aView1.m_bGraphicSelection);

    // Make sure that the hidden text cursor isn't visible in the second view, either.
    ViewCallback aView2(SfxViewShell::Current(),
        [](ViewCallback& rView) { rView.m_bViewCursorVisible = true; });
    // This was true, the second view didn't get the visibility of the text
    // cursor of the first view.
    CPPUNIT_ASSERT(!aView2.m_bViewCursorVisible);
    // This was false, the second view didn't get the graphic selection of the
    // first view.
    CPPUNIT_ASSERT(aView2.m_bGraphicViewSelection);
}

void SwTiledRenderingTest::testCreateViewTextSelection()
{
    // Load a document.
    SwXTextDocument* pXTextDocument = createDoc("dummy.fodt");
    ViewCallback aView1;

    // Create a text selection:
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    // Move the cursor into the second word.
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, 5, /*bBasicCall=*/false);
    // Create a selection on the word.
    pWrtShell->SelWrd();
    SwShellCursor* pShellCursor = pWrtShell->getShellCursor(false);
    // Did we indeed manage to select the second word?
    CPPUNIT_ASSERT_EQUAL(OUString("bbb"), pShellCursor->GetText());

    // Create a second view.
    SfxLokHelper::createView();

    // Make sure that the text selection is visible in the second view.
    ViewCallback aView2;
    // This failed, the second view didn't get the text selection of the first view.
    CPPUNIT_ASSERT(!aView2.m_aViewSelection.isEmpty());
}

void SwTiledRenderingTest::testRedlineColors()
{
    // Load a document.
    SwXTextDocument* pXTextDocument = createDoc("dummy.fodt");

    // Turn on track changes, type "zzz" at the end.
    uno::Reference<beans::XPropertySet> xPropertySet(mxComponent, uno::UNO_QUERY);
    xPropertySet->setPropertyValue("RecordChanges", uno::makeAny(true));
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    pWrtShell->EndOfSection();
    pWrtShell->Insert("zzz");

    // Assert that info about exactly one author is returned.
    tools::JsonWriter aJsonWriter;
    pXTextDocument->getTrackedChangeAuthors(aJsonWriter);
    std::stringstream aStream(aJsonWriter.extractAsOString().getStr());
    boost::property_tree::ptree aTree;
    boost::property_tree::read_json(aStream, aTree);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aTree.get_child("authors").size());
}

void SwTiledRenderingTest::testCommentEndTextEdit()
{
    // Create a document, type a character and remember the cursor position.
    SwXTextDocument* pXTextDocument = createDoc();
    ViewCallback aView1;
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'x', 0);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 'x', 0);
    Scheduler::ProcessEventsToIdle();
    tools::Rectangle aBodyCursor = aView1.m_aOwnCursor;

    // Create a comment and type a character there as well.
    const int nCtrlAltC = KEY_MOD1 + KEY_MOD2 + 512 + 'c' - 'a';
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'c', nCtrlAltC);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 'c', nCtrlAltC);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'x', 0);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 'x', 0);
    Scheduler::ProcessEventsToIdle();
    // End comment text edit by clicking in the body text area, and assert that
    // no unexpected cursor callbacks are emitted at origin (top left corner of
    // the document).
    aView1.m_bOwnCursorAtOrigin = false;
    pXTextDocument->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONDOWN, aBodyCursor.getX(), aBodyCursor.getY(), 1, MOUSE_LEFT, 0);
    pXTextDocument->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONUP, aBodyCursor.getX(), aBodyCursor.getY(), 1, MOUSE_LEFT, 0);
    Scheduler::ProcessEventsToIdle();
    // This failed, the cursor was at 0, 0 at some point during end text edit
    // of the comment.
    CPPUNIT_ASSERT(!aView1.m_bOwnCursorAtOrigin);

    // Hit enter and expect invalidation.
    Scheduler::ProcessEventsToIdle();
    aView1.m_bTilesInvalidated = false;
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_RETURN);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_RETURN);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(aView1.m_bTilesInvalidated);
}

void SwTiledRenderingTest::testCommentInsert()
{
    // Load a document with an as-char image in it.
    comphelper::LibreOfficeKit::setTiledAnnotations(false);
    SwXTextDocument* pXTextDocument = createDoc("image-comment.odt");
    SwDoc* pDoc = pXTextDocument->GetDocShell()->GetDoc();
    SwView* pView = pDoc->GetDocShell()->GetView();

    // Select the image.
    pView->GetViewFrame()->GetDispatcher()->Execute(FN_CNTNT_TO_NEXT_FRAME, SfxCallMode::SYNCHRON);
    // Make sure SwTextShell is replaced with SwDrawShell right now, not after 120 ms, as set in the
    // SwView ctor.
    pView->StopShellTimer();

    // Add a comment.
    uno::Reference<frame::XFrame> xFrame = pView->GetViewFrame()->GetFrame().GetFrameInterface();
    uno::Sequence<beans::PropertyValue> aPropertyValues = comphelper::InitPropertySequence(
    {
        {"Text", uno::makeAny(OUString("some text"))},
        {"Author", uno::makeAny(OUString("me"))},
    });
    ViewCallback aView;
    comphelper::dispatchCommand(".uno:InsertAnnotation", xFrame, aPropertyValues);
    Scheduler::ProcessEventsToIdle();
    OString aAnchorPos(aView.m_aComment.get_child("anchorPos").get_value<std::string>().c_str());
    // Without the accompanying fix in place, this test would have failed with
    // - Expected: 1418, 1418, 0, 0
    // - Actual  : 1418, 1418, 1024, 1024
    // i.e. the anchor position was a non-empty rectangle.
    CPPUNIT_ASSERT_EQUAL(OString("1418, 1418, 0, 0"), aAnchorPos);
    comphelper::LibreOfficeKit::setTiledAnnotations(true);
}

void SwTiledRenderingTest::testCursorPosition()
{
    // Load a document and register a callback, should get an own cursor.
    SwXTextDocument* pXTextDocument = createDoc();
    ViewCallback aView1;

    // Crete a second view, so the first view gets a collaborative cursor.
    SfxLokHelper::createView();
    pXTextDocument->initializeForTiledRendering({});
    ViewCallback aView2;

    // Make sure the two are exactly the same.
    // This failed, own cursor was at '1418, 1418', collaborative cursor was at
    // '1425, 1425', due to pixel alignment.
    CPPUNIT_ASSERT_EQUAL(aView1.m_aOwnCursor.toString(), aView1.m_aViewCursor.toString());
}

void SwTiledRenderingTest::testPaintCallbacks()
{
    // Test that paintTile() never results in callbacks, which can cause a
    // paint <-> invalidate loop.

    // Load a document and register a callback for the first view.
    SwXTextDocument* pXTextDocument = createDoc();
    ViewCallback aView1;

    // Create a second view and paint a tile on that second view.
    SfxLokHelper::createView();
    int nCanvasWidth = 256;
    int nCanvasHeight = 256;
    std::vector<unsigned char> aBuffer(nCanvasWidth * nCanvasHeight * 4);
    ScopedVclPtrInstance<VirtualDevice> pDevice(DeviceFormat::DEFAULT);
    pDevice->SetOutputSizePixelScaleOffsetAndBuffer(Size(nCanvasWidth, nCanvasHeight), Fraction(1.0), Point(), aBuffer.data());
    // Make sure that painting a tile in the second view doesn't invoke
    // callbacks on the first view.
    aView1.m_bCalled = false;
    pXTextDocument->paintTile(*pDevice, nCanvasWidth, nCanvasHeight, /*nTilePosX=*/0, /*nTilePosY=*/0, /*nTileWidth=*/3840, /*nTileHeight=*/3840);
    CPPUNIT_ASSERT(!aView1.m_bCalled);
}

void SwTiledRenderingTest::testUndoRepairResult()
{
    // Load a document and create two views.
    SwXTextDocument* pXTextDocument = createDoc("dummy.fodt");
    int nView1 = SfxLokHelper::getView();
    SfxLokHelper::createView();
    TestResultListener* pResult2 = new TestResultListener();
    css::uno::Reference< css::frame::XDispatchResultListener > xListener(static_cast< css::frame::XDispatchResultListener* >(pResult2), css::uno::UNO_QUERY);
    pXTextDocument->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    int nView2 = SfxLokHelper::getView();

    // Insert a character in the second view.
    SfxLokHelper::setView(nView2);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'b', 0);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 'b', 0);
    Scheduler::ProcessEventsToIdle();

    // Insert a character in the first view.
    SfxLokHelper::setView(nView1);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'a', 0);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 'a', 0);
    Scheduler::ProcessEventsToIdle();

    // Assert that by default the second view can't undo the action.
    SfxLokHelper::setView(nView2);
    comphelper::dispatchCommand(".uno:Undo", {}, xListener);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(SID_REPAIRPACKAGE), pResult2->m_nDocRepair);

    SfxLokHelper::setView(nView1);
    SfxViewShell::Current()->setLibreOfficeKitViewCallback(nullptr);
    SfxLokHelper::setView(nView2);
    SfxViewShell::Current()->setLibreOfficeKitViewCallback(nullptr);
}

void SwTiledRenderingTest::testRedoRepairResult()
{
    // Load a document and create two views.
    SwXTextDocument* pXTextDocument = createDoc("dummy.fodt");
    int nView1 = SfxLokHelper::getView();
    SfxLokHelper::createView();
    TestResultListener* pResult2 = new TestResultListener();
    css::uno::Reference< css::frame::XDispatchResultListener > xListener(static_cast< css::frame::XDispatchResultListener* >(pResult2), css::uno::UNO_QUERY);
    pXTextDocument->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    int nView2 = SfxLokHelper::getView();

    // Insert a character in the second view.
    SfxLokHelper::setView(nView2);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'b', 0);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 'b', 0);
    Scheduler::ProcessEventsToIdle();

    // Insert a character in the first view.
    SfxLokHelper::setView(nView1);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'a', 0);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 'a', 0);
    Scheduler::ProcessEventsToIdle();

    comphelper::dispatchCommand(".uno:Undo", {}, xListener);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(0), pResult2->m_nDocRepair);

    // Assert that by default the second view can't redo the action.
    SfxLokHelper::setView(nView2);
    comphelper::dispatchCommand(".uno:Redo", {}, xListener);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(SID_REPAIRPACKAGE), pResult2->m_nDocRepair);

    SfxLokHelper::setView(nView1);
    SfxViewShell::Current()->setLibreOfficeKitViewCallback(nullptr);
    SfxLokHelper::setView(nView2);
    SfxViewShell::Current()->setLibreOfficeKitViewCallback(nullptr);
}

namespace {

void checkUndoRepairStates(SwXTextDocument* pXTextDocument, SwView* pView1, SwView* pView2)
{
    SfxItemSet aItemSet1(pXTextDocument->GetDocShell()->GetDoc()->GetAttrPool(), svl::Items<SID_UNDO, SID_UNDO>{});
    SfxItemSet aItemSet2(pXTextDocument->GetDocShell()->GetDoc()->GetAttrPool(), svl::Items<SID_UNDO, SID_UNDO>{});
    // first view, undo enabled
    pView1->GetState(aItemSet1);
    CPPUNIT_ASSERT_EQUAL(SfxItemState::SET, aItemSet1.GetItemState(SID_UNDO));
    const SfxUInt32Item *pUnsetItem = dynamic_cast<const SfxUInt32Item*>(aItemSet1.GetItem(SID_UNDO));
    CPPUNIT_ASSERT(!pUnsetItem);
    // second view, undo conflict
    pView2->GetState(aItemSet2);
    CPPUNIT_ASSERT_EQUAL(SfxItemState::SET, aItemSet2.GetItemState(SID_UNDO));
    const SfxUInt32Item* pUInt32Item = dynamic_cast<const SfxUInt32Item*>(aItemSet2.GetItem(SID_UNDO));
    CPPUNIT_ASSERT(pUInt32Item);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(SID_REPAIRPACKAGE), pUInt32Item->GetValue());
};

}

void SwTiledRenderingTest::testDisableUndoRepair()
{
    // Create two views.
    SwXTextDocument* pXTextDocument = createDoc("dummy.fodt");
    ViewCallback aView1;
    SwView* pView1 = dynamic_cast<SwView*>(SfxViewShell::Current());
    CPPUNIT_ASSERT(pView1);
    int nView1 = SfxLokHelper::getView();
    SfxLokHelper::createView();
    ViewCallback aView2;
    SwView* pView2 = dynamic_cast<SwView*>(SfxViewShell::Current());
    CPPUNIT_ASSERT(pView2);
    int nView2 = SfxLokHelper::getView();

    {
        SfxItemSet aItemSet1(pXTextDocument->GetDocShell()->GetDoc()->GetAttrPool(), svl::Items<SID_UNDO, SID_UNDO>{});
        SfxItemSet aItemSet2(pXTextDocument->GetDocShell()->GetDoc()->GetAttrPool(), svl::Items<SID_UNDO, SID_UNDO>{});
        pView1->GetState(aItemSet1);
        CPPUNIT_ASSERT_EQUAL(SfxItemState::DISABLED, aItemSet1.GetItemState(SID_UNDO));
        pView2->GetState(aItemSet2);
        CPPUNIT_ASSERT_EQUAL(SfxItemState::DISABLED, aItemSet2.GetItemState(SID_UNDO));
    }

    // Insert a character in the first view.
    SfxLokHelper::setView(nView1);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'k', 0);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 'k', 0);
    Scheduler::ProcessEventsToIdle();
    checkUndoRepairStates(pXTextDocument, pView1, pView2);

    // Insert a character in the second view.
    SfxLokHelper::setView(nView2);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'u', 0);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 'u', 0);
    Scheduler::ProcessEventsToIdle();
    {
        SfxItemSet aItemSet1(pXTextDocument->GetDocShell()->GetDoc()->GetAttrPool(), svl::Items<SID_UNDO, SID_UNDO>{});
        SfxItemSet aItemSet2(pXTextDocument->GetDocShell()->GetDoc()->GetAttrPool(), svl::Items<SID_UNDO, SID_UNDO>{});
        // second view, undo enabled
        pView2->GetState(aItemSet2);
        CPPUNIT_ASSERT_EQUAL(SfxItemState::SET, aItemSet2.GetItemState(SID_UNDO));
        const SfxUInt32Item *pUnsetItem = dynamic_cast<const SfxUInt32Item*>(aItemSet2.GetItem(SID_UNDO));
        CPPUNIT_ASSERT(!pUnsetItem);
        // first view, undo conflict
        pView1->GetState(aItemSet1);
        CPPUNIT_ASSERT_EQUAL(SfxItemState::SET, aItemSet1.GetItemState(SID_UNDO));
        const SfxUInt32Item* pUInt32Item = dynamic_cast<const SfxUInt32Item*>(aItemSet1.GetItem(SID_UNDO));
        CPPUNIT_ASSERT(pUInt32Item);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(SID_REPAIRPACKAGE), pUInt32Item->GetValue());
    }

    // Insert a character in the first view.
    SfxLokHelper::setView(nView1);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'l', 0);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 'l', 0);
    Scheduler::ProcessEventsToIdle();
    checkUndoRepairStates(pXTextDocument, pView1, pView2);
}

void SwTiledRenderingTest::testAllTrackedChanges()
{
    // Load a document.
    createDoc("dummy.fodt");

    uno::Reference<beans::XPropertySet> xPropSet(mxComponent, uno::UNO_QUERY);
    xPropSet->setPropertyValue("RecordChanges", uno::makeAny(true));

    // view #1
    SwView* pView1 = dynamic_cast<SwView*>(SfxViewShell::Current());
    CPPUNIT_ASSERT(pView1);
    SwWrtShell* pWrtShell1 = pView1->GetWrtShellPtr();

    // view #2
    int nView1 = SfxLokHelper::getView();
    int nView2 = SfxLokHelper::createView();
    SwView* pView2 = dynamic_cast<SwView*>(SfxViewShell::Current());
    CPPUNIT_ASSERT(pView2 && pView1 != pView2);
    SwWrtShell* pWrtShell2 = pView2->GetWrtShellPtr();
    // Insert text and reject all
    {
        pWrtShell1->StartOfSection();
        pWrtShell1->Insert("hxx");

        pWrtShell2->EndOfSection();
        pWrtShell2->Insert("cxx");
    }

    // Get the redline
    const SwRedlineTable& rTable = pWrtShell2->GetDoc()->getIDocumentRedlineAccess().GetRedlineTable();
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(2), rTable.size());
    {
        SfxVoidItem aItem(FN_REDLINE_REJECT_ALL);
        pView1->GetViewFrame()->GetDispatcher()->ExecuteList(FN_REDLINE_REJECT_ALL,
            SfxCallMode::SYNCHRON, { &aItem });
    }

    // The reject all was performed.
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(0), rTable.size());
    {
        SwShellCursor* pShellCursor = pWrtShell1->getShellCursor(false);
        CPPUNIT_ASSERT_EQUAL(OUString("Aaa bbb."), pShellCursor->GetPoint()->nNode.GetNode().GetTextNode()->GetText());
    }

    // Insert text and accept all
    {
        pWrtShell1->StartOfSection();
        pWrtShell1->Insert("hyy");

        pWrtShell2->EndOfSection();
        pWrtShell2->Insert("cyy");
    }

    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(2), rTable.size());
    {
        SfxVoidItem aItem(FN_REDLINE_ACCEPT_ALL);
        pView1->GetViewFrame()->GetDispatcher()->ExecuteList(FN_REDLINE_ACCEPT_ALL,
            SfxCallMode::SYNCHRON, { &aItem });
    }

    // The accept all was performed
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(0), rTable.size());
    {
        SwShellCursor* pShellCursor = pWrtShell2->getShellCursor(false);
        CPPUNIT_ASSERT_EQUAL(OUString("hyyAaa bbb.cyy"), pShellCursor->GetPoint()->nNode.GetNode().GetTextNode()->GetText());
    }

    SfxLokHelper::setView(nView1);
    SfxViewShell::Current()->setLibreOfficeKitViewCallback(nullptr);
    SfxLokHelper::setView(nView2);
    SfxViewShell::Current()->setLibreOfficeKitViewCallback(nullptr);
}

void SwTiledRenderingTest::testDocumentRepair()
{
    // Create two views.
    SwXTextDocument* pXTextDocument = createDoc("dummy.fodt");
    // view #1
    SfxViewShell* pView1 = SfxViewShell::Current();

    // view #2
    int nView1 = SfxLokHelper::getView();
    SfxLokHelper::createView();
    SfxViewShell* pView2 = SfxViewShell::Current();
    int nView2 = SfxLokHelper::getView();
    CPPUNIT_ASSERT(pView1 != pView2);
    {
        std::unique_ptr<SfxPoolItem> xItem1;
        std::unique_ptr<SfxPoolItem> xItem2;
        pView1->GetViewFrame()->GetBindings().QueryState(SID_DOC_REPAIR, xItem1);
        pView2->GetViewFrame()->GetBindings().QueryState(SID_DOC_REPAIR, xItem2);
        const SfxBoolItem* pItem1 = dynamic_cast<const SfxBoolItem*>(xItem1.get());
        const SfxBoolItem* pItem2 = dynamic_cast<const SfxBoolItem*>(xItem2.get());
        CPPUNIT_ASSERT(pItem1);
        CPPUNIT_ASSERT(pItem2);
        CPPUNIT_ASSERT_EQUAL(false, pItem1->GetValue());
        CPPUNIT_ASSERT_EQUAL(false, pItem2->GetValue());
    }

    // Insert a character in the second view.
    SfxLokHelper::setView(nView2);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'u', 0);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 'u', 0);
    Scheduler::ProcessEventsToIdle();
    {
        std::unique_ptr<SfxPoolItem> xItem1;
        std::unique_ptr<SfxPoolItem> xItem2;
        pView1->GetViewFrame()->GetBindings().QueryState(SID_DOC_REPAIR, xItem1);
        pView2->GetViewFrame()->GetBindings().QueryState(SID_DOC_REPAIR, xItem2);
        const SfxBoolItem* pItem1 = dynamic_cast<const SfxBoolItem*>(xItem1.get());
        const SfxBoolItem* pItem2 = dynamic_cast<const SfxBoolItem*>(xItem2.get());
        CPPUNIT_ASSERT(pItem1);
        CPPUNIT_ASSERT(pItem2);
        CPPUNIT_ASSERT_EQUAL(true, pItem1->GetValue());
        CPPUNIT_ASSERT_EQUAL(true, pItem2->GetValue());
    }

    SfxLokHelper::setView(nView1);
    SfxViewShell::Current()->setLibreOfficeKitViewCallback(nullptr);
    SfxLokHelper::setView(nView2);
    SfxViewShell::Current()->setLibreOfficeKitViewCallback(nullptr);
}

namespace {

void checkPageHeaderOrFooter(const SfxViewShell* pViewShell, sal_uInt16 nWhich, bool bValue)
{
    uno::Sequence<OUString> aSeq;
    const SfxPoolItem* pState = nullptr;
    pViewShell->GetDispatcher()->QueryState(nWhich, pState);
    const SfxStringListItem* pListItem = dynamic_cast<const SfxStringListItem*>(pState);
    CPPUNIT_ASSERT(pListItem);
    pListItem->GetStringList(aSeq);
    if (bValue)
    {
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aSeq.getLength());
        CPPUNIT_ASSERT_EQUAL(OUString("Default Page Style"), aSeq[0]);
    }
    else
        CPPUNIT_ASSERT(!aSeq.hasElements());
};

}

void SwTiledRenderingTest::testPageHeader()
{
    createDoc("dummy.fodt");
    SfxViewShell* pViewShell = SfxViewShell::Current();
    // Check Page Header State
    checkPageHeaderOrFooter(pViewShell, FN_INSERT_PAGEHEADER, false);
    // Insert Page Header
    {
        SfxStringItem aStyle(FN_INSERT_PAGEHEADER, "Default Page Style");
        SfxBoolItem aItem(FN_PARAM_1, true);
        pViewShell->GetDispatcher()->ExecuteList(FN_INSERT_PAGEHEADER, SfxCallMode::API | SfxCallMode::SYNCHRON, {&aStyle, &aItem});
    }
    // Check Page Header State
    checkPageHeaderOrFooter(pViewShell, FN_INSERT_PAGEHEADER, true);

    // Remove Page Header
    {
        SfxStringItem aStyle(FN_INSERT_PAGEHEADER, "Default Page Style");
        SfxBoolItem aItem(FN_PARAM_1, false);
        pViewShell->GetDispatcher()->ExecuteList(FN_INSERT_PAGEHEADER, SfxCallMode::API | SfxCallMode::SYNCHRON, {&aStyle, &aItem});
    }
    // Check Page Header State
    checkPageHeaderOrFooter(pViewShell, FN_INSERT_PAGEHEADER, false);
}

void SwTiledRenderingTest::testPageFooter()
{
    createDoc("dummy.fodt");
    SfxViewShell* pViewShell = SfxViewShell::Current();
    // Check Page Footer State
    checkPageHeaderOrFooter(pViewShell, FN_INSERT_PAGEFOOTER, false);
    // Insert Page Footer
    {
        SfxStringItem aPageStyle(FN_INSERT_PAGEFOOTER, "Default Page Style");
        SfxBoolItem aItem(FN_PARAM_1, true);
        pViewShell->GetDispatcher()->ExecuteList(FN_INSERT_PAGEFOOTER, SfxCallMode::API | SfxCallMode::SYNCHRON, {&aPageStyle, &aItem});
    }
    // Check Page Footer State
    checkPageHeaderOrFooter(pViewShell, FN_INSERT_PAGEFOOTER, true);

    // Remove Page Footer
    {
        SfxStringItem aPageStyle(FN_INSERT_PAGEFOOTER, "Default Page Style");
        SfxBoolItem aItem(FN_PARAM_1, false);
        pViewShell->GetDispatcher()->ExecuteList(FN_INSERT_PAGEFOOTER, SfxCallMode::API | SfxCallMode::SYNCHRON, {&aPageStyle, &aItem});
    }
    // Check Footer State
    checkPageHeaderOrFooter(pViewShell, FN_INSERT_PAGEFOOTER, false);
}

void SwTiledRenderingTest::testTdf115088()
{
    // We have three lines in the test document and we try to copy the second and third line
    // To the beginning of the document
    SwXTextDocument* pXTextDocument = createDoc("tdf115088.odt");

    // Select and copy second and third line
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_HOME | KEY_MOD1);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_HOME | KEY_MOD1);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_DOWN);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_DOWN);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_DOWN | KEY_SHIFT);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_DOWN | KEY_SHIFT);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_RIGHT | KEY_SHIFT);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_RIGHT | KEY_SHIFT);
    Scheduler::ProcessEventsToIdle();
    comphelper::dispatchCommand(".uno:Copy", uno::Sequence<beans::PropertyValue>());

    // Move cursor to the beginning of the first line and paste
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_HOME | KEY_MOD1);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_HOME | KEY_MOD1);
    Scheduler::ProcessEventsToIdle();
    comphelper::dispatchCommand(".uno:PasteUnformatted", uno::Sequence<beans::PropertyValue>());
    Scheduler::ProcessEventsToIdle();

    // Check the resulting text in the document. (it was 1Text\n1\n1\n1)
    CPPUNIT_ASSERT_EQUAL(OUString("1\n1Text\n1\n1"), pXTextDocument->getText()->getString());

    mxComponent->dispose();
    mxComponent.clear();
    comphelper::LibreOfficeKit::setActive(false);
}

void SwTiledRenderingTest::testRedlineField()
{
    // Load a document.
    SwXTextDocument* pXTextDocument = createDoc("dummy.fodt");
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();

    // Turn on track changes and type "x".
    uno::Reference<beans::XPropertySet> xPropertySet(mxComponent, uno::UNO_QUERY);
    xPropertySet->setPropertyValue("RecordChanges", uno::makeAny(true));

    SwDateTimeField aDate(static_cast<SwDateTimeFieldType*>(pWrtShell->GetFieldType(0, SwFieldIds::DateTime)));
    //aDate->SetDateTime(::DateTime(::DateTime::SYSTEM));
    pWrtShell->Insert(aDate);

    // Get the redline just created
    const SwRedlineTable& rTable = pWrtShell->GetDoc()->getIDocumentRedlineAccess().GetRedlineTable();
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(1), rTable.size());
    SwRangeRedline* pRedline = rTable[0];
    CPPUNIT_ASSERT(pRedline->GetDescr().indexOf(aDate.GetFieldName())!= -1);
}

void SwTiledRenderingTest::testIMESupport()
{
    SwXTextDocument* pXTextDocument = createDoc("dummy.fodt");
    VclPtr<vcl::Window> pDocWindow = pXTextDocument->getDocWindow();

    SwView* pView = dynamic_cast<SwView*>(SfxViewShell::Current());
    assert(pView);
    SwWrtShell* pWrtShell = pView->GetWrtShellPtr();

    // sequence of chinese IME compositions when 'nihao' is typed in an IME
    const std::vector<OString> aUtf8Inputs{ "年", "你", "你好", "你哈", "你好", "你好" };
    std::vector<OUString> aInputs;
    std::transform(aUtf8Inputs.begin(), aUtf8Inputs.end(),
                   std::back_inserter(aInputs), [](OString aInput) {
                       return OUString::fromUtf8(aInput);
                   });
    for (const auto& aInput: aInputs)
    {
        pDocWindow->PostExtTextInputEvent(VclEventId::ExtTextInput, aInput);
    }
    pDocWindow->PostExtTextInputEvent(VclEventId::EndExtTextInput, "");

    // the cursor should be at position 2nd
    SwShellCursor* pShellCursor = pWrtShell->getShellCursor(false);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), pShellCursor->GetPoint()->nContent.GetIndex());

    // content contains only the last IME composition, not all
    CPPUNIT_ASSERT_EQUAL(OUString(aInputs[aInputs.size() - 1] + "Aaa bbb."), pShellCursor->GetPoint()->nNode.GetNode().GetTextNode()->GetText());
}

void SwTiledRenderingTest::testIMEFormattingAtEndOfParagraph()
{
    comphelper::LibreOfficeKit::setActive();
    SwXTextDocument* pXTextDocument = createDoc("dummy.fodt");
    VclPtr<vcl::Window> pDocWindow = pXTextDocument->getDocWindow();

    SwView* pView = dynamic_cast<SwView*>(SfxViewShell::Current());
    assert(pView);
    SwWrtShell* pWrtShell = pView->GetWrtShellPtr();

    // delete all characters

    for (int i = 0; i < 9; i++)
    {
        pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_DELETE);
        pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_DELETE);
    }

    Scheduler::ProcessEventsToIdle();

    pDocWindow->PostExtTextInputEvent(VclEventId::ExtTextInput, "a");
    pDocWindow->PostExtTextInputEvent(VclEventId::EndExtTextInput, "");

    // status: "a"

    comphelper::dispatchCommand(".uno:Bold", uno::Sequence<beans::PropertyValue>());
    Scheduler::ProcessEventsToIdle();

    pDocWindow->PostExtTextInputEvent(VclEventId::ExtTextInput, "b");
    pDocWindow->PostExtTextInputEvent(VclEventId::EndExtTextInput, "");

    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_RETURN);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_RETURN);
    Scheduler::ProcessEventsToIdle();

    // status: "a<bold>b</bold>\n"

    pDocWindow->PostExtTextInputEvent(VclEventId::ExtTextInput, "a");
    pDocWindow->PostExtTextInputEvent(VclEventId::EndExtTextInput, "");

    std::unique_ptr<SfxPoolItem> pItem;
    pView->GetViewFrame()->GetBindings().QueryState(SID_ATTR_CHAR_WEIGHT, pItem);
    auto pWeightItem = dynamic_cast<SvxWeightItem*>(pItem.get());
    CPPUNIT_ASSERT(pWeightItem);

    CPPUNIT_ASSERT_EQUAL(FontWeight::WEIGHT_BOLD, pWeightItem->GetWeight());

    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_RETURN);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_RETURN);
    Scheduler::ProcessEventsToIdle();

    // status: "a<bold>b</bold>\n
    //          <bold>a</bold>\n"

    comphelper::dispatchCommand(".uno:Bold", uno::Sequence<beans::PropertyValue>());
    Scheduler::ProcessEventsToIdle();

    pDocWindow->PostExtTextInputEvent(VclEventId::ExtTextInput, "b");
    pDocWindow->PostExtTextInputEvent(VclEventId::EndExtTextInput, "");

    pView->GetViewFrame()->GetBindings().QueryState(SID_ATTR_CHAR_WEIGHT, pItem);
    auto pWeightItem2 = dynamic_cast<SvxWeightItem*>(pItem.get());
    CPPUNIT_ASSERT(pWeightItem2);

    CPPUNIT_ASSERT_EQUAL(FontWeight::WEIGHT_NORMAL, pWeightItem2->GetWeight());

    // status: "a<bold>b</bold>\n
    //          <bold>a</bold>\n"
    //          b"

    comphelper::dispatchCommand(".uno:Bold", uno::Sequence<beans::PropertyValue>());
    Scheduler::ProcessEventsToIdle();

    pDocWindow->PostExtTextInputEvent(VclEventId::ExtTextInput, "a");
    pDocWindow->PostExtTextInputEvent(VclEventId::EndExtTextInput, "");

    pView->GetViewFrame()->GetBindings().QueryState(SID_ATTR_CHAR_WEIGHT, pItem);
    auto pWeightItem3 = dynamic_cast<SvxWeightItem*>(pItem.get());
    CPPUNIT_ASSERT(pWeightItem3);

    CPPUNIT_ASSERT_EQUAL(FontWeight::WEIGHT_BOLD, pWeightItem3->GetWeight());

    comphelper::dispatchCommand(".uno:Bold", uno::Sequence<beans::PropertyValue>());
    Scheduler::ProcessEventsToIdle();

    pDocWindow->PostExtTextInputEvent(VclEventId::ExtTextInput, "b");
    pDocWindow->PostExtTextInputEvent(VclEventId::EndExtTextInput, "");

    pView->GetViewFrame()->GetBindings().QueryState(SID_ATTR_CHAR_WEIGHT, pItem);
    auto pWeightItem4 = dynamic_cast<SvxWeightItem*>(pItem.get());
    CPPUNIT_ASSERT(pWeightItem4);

    CPPUNIT_ASSERT_EQUAL(FontWeight::WEIGHT_NORMAL, pWeightItem4->GetWeight());

    // status: "a<bold>b</bold>\n
    //          <bold>a</bold>\n"
    //          b<bold>a</bold>b"

    // the cursor should be at position 3nd
    SwShellCursor* pShellCursor = pWrtShell->getShellCursor(false);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3), pShellCursor->GetPoint()->nContent.GetIndex());

    // check the content
    CPPUNIT_ASSERT_EQUAL(OUString("bab"), pShellCursor->GetPoint()->nNode.GetNode().GetTextNode()->GetText());
}

void SwTiledRenderingTest::testSplitNodeRedlineCallback()
{
    // Load a document.
    SwXTextDocument* pXTextDocument = createDoc("splitnode_redline_callback.fodt");
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    setupLibreOfficeKitViewCallback(pWrtShell->GetSfxViewShell());

    // 1. test case
    // Move cursor between the two tracked changes
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_DOWN);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_DOWN);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_DOWN);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_DOWN);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_DOWN);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_DOWN);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_DOWN);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_DOWN);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_DOWN);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_DOWN);
    Scheduler::ProcessEventsToIdle();

    // Add a new line
    m_nRedlineTableEntryModified = 0;
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_RETURN);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_RETURN);
    Scheduler::ProcessEventsToIdle();

    // Assert that we get a notification about redline modification
    // The redline after the inserted node gets a different vertical position
    CPPUNIT_ASSERT_EQUAL(1, m_nRedlineTableEntryModified);

    // 2. test case
    // Move cursor back to the first line, so adding new line will affect both tracked changes
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_HOME | KEY_MOD1);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_HOME | KEY_MOD1);
    Scheduler::ProcessEventsToIdle();

    // Add a new line
    m_nRedlineTableEntryModified = 0;
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_RETURN);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_RETURN);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(2, m_nRedlineTableEntryModified);

    // 3. test case
    // Move cursor to the end of the document, so adding a new line won't affect any tracked changes
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_END | KEY_MOD1);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_END | KEY_MOD1);
    Scheduler::ProcessEventsToIdle();

    // Add a new line
    m_nRedlineTableEntryModified = 0;
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_RETURN);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_RETURN);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(0, m_nRedlineTableEntryModified);
}

void SwTiledRenderingTest::testDeleteNodeRedlineCallback()
{
    // Load a document.
    SwXTextDocument* pXTextDocument = createDoc("removenode_redline_callback.fodt");
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    setupLibreOfficeKitViewCallback(pWrtShell->GetSfxViewShell());

    // 1. test case
    // Move cursor between the two tracked changes
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_DOWN);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_DOWN);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_DOWN);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_DOWN);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_DOWN);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_DOWN);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_DOWN);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_DOWN);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_DOWN);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_DOWN);
    Scheduler::ProcessEventsToIdle();

    // Remove one (empty) line
    m_nRedlineTableEntryModified = 0;
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_DELETE);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_DELETE);
    Scheduler::ProcessEventsToIdle();

    // Assert that we get a notification about redline modification
    // The redline after the removed node gets a different vertical position
    CPPUNIT_ASSERT_EQUAL(1, m_nRedlineTableEntryModified);

    // 2. test case
    // Move cursor back to the first line, so removing one line will affect both tracked changes
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_HOME | KEY_MOD1);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_HOME | KEY_MOD1);
    Scheduler::ProcessEventsToIdle();

    // Remove a new line
    m_nRedlineTableEntryModified = 0;
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_DELETE);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_DELETE);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(2, m_nRedlineTableEntryModified);

    // 3. test case
    // Move cursor to the end of the document, so removing one line won't affect any tracked changes
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_END | KEY_MOD1);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_END | KEY_MOD1);
    Scheduler::ProcessEventsToIdle();

    // Remove a line
    m_nRedlineTableEntryModified = 0;
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_BACKSPACE);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_BACKSPACE);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(0, m_nRedlineTableEntryModified);
}


void SwTiledRenderingTest::testVisCursorInvalidation()
{
    SwXTextDocument* pXTextDocument = createDoc("dummy.fodt");
    ViewCallback aView1;
    int nView1 = SfxLokHelper::getView();

    SfxLokHelper::createView();
    int nView2 = SfxLokHelper::getView();
    ViewCallback aView2;
    Scheduler::ProcessEventsToIdle();

    // Move visible cursor in the first view
    SfxLokHelper::setView(nView1);
    Scheduler::ProcessEventsToIdle();

    aView1.m_bOwnCursorInvalidated = false;
    aView1.m_bViewCursorInvalidated = false;
    aView2.m_bOwnCursorInvalidated = false;
    aView2.m_bViewCursorInvalidated = false;

    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_RIGHT);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_RIGHT);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT(!aView1.m_bViewCursorInvalidated);
    CPPUNIT_ASSERT(aView1.m_bOwnCursorInvalidated);
    CPPUNIT_ASSERT(aView2.m_bViewCursorInvalidated);
    CPPUNIT_ASSERT(!aView2.m_bOwnCursorInvalidated);

    // Insert text in the second view which moves the other view's cursor too
    SfxLokHelper::setView(nView2);

    Scheduler::ProcessEventsToIdle();
    aView1.m_bOwnCursorInvalidated = false;
    aView1.m_bViewCursorInvalidated = false;
    aView2.m_bOwnCursorInvalidated = false;
    aView2.m_bViewCursorInvalidated = false;

    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'x', 0);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 'x', 0);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT(aView1.m_bViewCursorInvalidated);
    CPPUNIT_ASSERT(aView1.m_bOwnCursorInvalidated);
    CPPUNIT_ASSERT(aView2.m_bViewCursorInvalidated);
    CPPUNIT_ASSERT(aView2.m_bOwnCursorInvalidated);
    // Check that views have correct location for the other's cursor.
    CPPUNIT_ASSERT_EQUAL(aView1.m_aOwnCursor, aView2.m_aViewCursor);
    CPPUNIT_ASSERT_EQUAL(aView2.m_aOwnCursor, aView1.m_aViewCursor);
    // Their cursors should be on the same line, first view's more to the right.
    CPPUNIT_ASSERT_EQUAL(aView1.m_aOwnCursor.getY(), aView2.m_aOwnCursor.getY());
    CPPUNIT_ASSERT_GREATER(aView2.m_aOwnCursor.getX(), aView1.m_aOwnCursor.getX());

    // Do the same as before, but set the related compatibility flag first
    SfxLokHelper::setView(nView2);

    comphelper::LibreOfficeKit::setViewIdForVisCursorInvalidation(true);

    Scheduler::ProcessEventsToIdle();
    aView1.m_bOwnCursorInvalidated = false;
    aView1.m_bViewCursorInvalidated = false;
    aView2.m_bOwnCursorInvalidated = false;
    aView2.m_bViewCursorInvalidated = false;

    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'x', 0);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 'x', 0);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT(aView1.m_bViewCursorInvalidated);
    CPPUNIT_ASSERT(aView1.m_bOwnCursorInvalidated);
    CPPUNIT_ASSERT_EQUAL(nView2, aView1.m_nOwnCursorInvalidatedBy);
    CPPUNIT_ASSERT(aView2.m_bViewCursorInvalidated);
    CPPUNIT_ASSERT(aView2.m_bOwnCursorInvalidated);
    CPPUNIT_ASSERT_EQUAL(nView2, aView2.m_nOwnCursorInvalidatedBy);
    CPPUNIT_ASSERT_EQUAL(aView1.m_aOwnCursor, aView2.m_aViewCursor);
    CPPUNIT_ASSERT_EQUAL(aView2.m_aOwnCursor, aView1.m_aViewCursor);
    // Their cursors should be on the same line, first view's more to the right.
    CPPUNIT_ASSERT_EQUAL(aView1.m_aOwnCursor.getY(), aView2.m_aOwnCursor.getY());
    CPPUNIT_ASSERT_GREATER(aView2.m_aOwnCursor.getX(), aView1.m_aOwnCursor.getX());

    comphelper::LibreOfficeKit::setViewIdForVisCursorInvalidation(false);
}

void SwTiledRenderingTest::testDeselectCustomShape()
{
    SwXTextDocument* pXTextDocument = createDoc("dummy.fodt");
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    SwShellCursor* pShellCursor = pWrtShell->getShellCursor(false);
    Point aStart = pShellCursor->GetSttPos();
    aStart.setX(aStart.getX() - 1000);
    aStart.setY(aStart.getY() - 1000);

    comphelper::dispatchCommand(".uno:BasicShapes.hexagon", uno::Sequence<beans::PropertyValue>());
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pWrtShell->GetDrawView()->GetMarkedObjectList().GetMarkCount());

    pXTextDocument->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONDOWN, aStart.getX(), aStart.getY(), 1, MOUSE_LEFT, 0);
    pXTextDocument->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONUP, aStart.getX(), aStart.getY(), 1, MOUSE_LEFT, 0);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), pWrtShell->GetDrawView()->GetMarkedObjectList().GetMarkCount());
}

void SwTiledRenderingTest::testSemiTransparent()
{
    // Load a document where the top left tile contains a semi-transparent rectangle shape.
    SwXTextDocument* pXTextDocument = createDoc("semi-transparent.odt");

    // Render a larger area, and then get the color of the bottom right corner of our tile.
    size_t nCanvasWidth = 1024;
    size_t nCanvasHeight = 512;
    size_t nTileSize = 256;
    std::vector<unsigned char> aPixmap(nCanvasWidth * nCanvasHeight * 4, 0);
    ScopedVclPtrInstance<VirtualDevice> pDevice(DeviceFormat::DEFAULT);
    pDevice->SetBackground(Wallpaper(COL_TRANSPARENT));
    pDevice->SetOutputSizePixelScaleOffsetAndBuffer(Size(nCanvasWidth, nCanvasHeight),
                                                    Fraction(1.0), Point(), aPixmap.data());
    pXTextDocument->paintTile(*pDevice, nCanvasWidth, nCanvasHeight, /*nTilePosX=*/0,
                              /*nTilePosY=*/0, /*nTileWidth=*/15360, /*nTileHeight=*/7680);
    pDevice->EnableMapMode(false);
    Bitmap aBitmap = pDevice->GetBitmap(Point(0, 0), Size(nTileSize, nTileSize));
    Bitmap::ScopedReadAccess pAccess(aBitmap);
    Color aColor(pAccess->GetPixel(255, 255));

    // Without the accompanying fix in place, this test would have failed with 'Expected greater or
    // equal than: 190; Actual: 159'. This means the semi-transparent gray rectangle was darker than
    // expected, as it was painted twice.
    CPPUNIT_ASSERT_GREATEREQUAL(190, static_cast<int>(aColor.R));
    CPPUNIT_ASSERT_GREATEREQUAL(190, static_cast<int>(aColor.G));
    CPPUNIT_ASSERT_GREATEREQUAL(190, static_cast<int>(aColor.B));
}

void SwTiledRenderingTest::testHighlightNumbering()
{
    // Load a document where the top left tile contains a semi-transparent rectangle shape.
    SwXTextDocument* pXTextDocument = createDoc("tdf114799.docx");

    // Render a larger area, and then get the color of the bottom right corner of our tile.
    size_t nCanvasWidth = 1024;
    size_t nCanvasHeight = 512;
    size_t nTileSize = 256;
    std::vector<unsigned char> aPixmap(nCanvasWidth * nCanvasHeight * 4, 0);
    ScopedVclPtrInstance<VirtualDevice> pDevice(DeviceFormat::DEFAULT);
    pDevice->SetBackground(Wallpaper(COL_TRANSPARENT));
    pDevice->SetOutputSizePixelScaleOffsetAndBuffer(Size(nCanvasWidth, nCanvasHeight),
                                                    Fraction(1.0), Point(), aPixmap.data());
    pXTextDocument->paintTile(*pDevice, nCanvasWidth, nCanvasHeight, /*nTilePosX=*/0,
                              /*nTilePosY=*/0, /*nTileWidth=*/15360, /*nTileHeight=*/7680);
    pDevice->EnableMapMode(false);
    Bitmap aBitmap = pDevice->GetBitmap(Point(0, 0), Size(nTileSize, nTileSize));
    Bitmap::ScopedReadAccess pAccess(aBitmap);

    // Yellow highlighting over numbering
    Color aColor(pAccess->GetPixel(103, 148));
    CPPUNIT_ASSERT_EQUAL(COL_YELLOW, aColor);
}

void SwTiledRenderingTest::testPilcrowRedlining()
{
    // Load a document where the top left tile contains
    // paragraph and line break symbols with redlining.
    SwXTextDocument* pXTextDocument = createDoc("pilcrow-redlining.fodt");

    // show non printing characters, including pilcrow and
    // line break symbols with redlining
    comphelper::dispatchCommand(".uno:ControlCodes", {});

    // Render a larger area, and then get the color of the bottom right corner of our tile.
    size_t nCanvasWidth = 2048;
    size_t nCanvasHeight = 1024;
    size_t nTileSize = 512;
    std::vector<unsigned char> aPixmap(nCanvasWidth * nCanvasHeight * 4, 0);
    ScopedVclPtrInstance<VirtualDevice> pDevice(DeviceFormat::DEFAULT);
    pDevice->SetBackground(Wallpaper(COL_TRANSPARENT));
    pDevice->SetOutputSizePixelScaleOffsetAndBuffer(Size(nCanvasWidth, nCanvasHeight),
                                                    Fraction(1.0), Point(), aPixmap.data());
    pXTextDocument->paintTile(*pDevice, nCanvasWidth, nCanvasHeight, /*nTilePosX=*/0,
                              /*nTilePosY=*/0, /*nTileWidth=*/15360, /*nTileHeight=*/7680);
    pDevice->EnableMapMode(false);
    Bitmap aBitmap = pDevice->GetBitmap(Point(100, 100), Size(nTileSize, nTileSize));
    Bitmap::ScopedReadAccess pAccess(aBitmap);

    const char* aTexts[] = {
            "Insert paragraph break",
            "Insert paragraph break (empty line)",
            "Delete paragraph break",
            "Delete paragraph break (empty line)",
            "Insert line break",
            "Insert line break (empty line)",
            "Delete line break",
            "Delete line break (empty line)"
    };

    // Check redlining (strikeout and underline) over the paragraph and line break symbols
    for (int nLine = 0; nLine < 8; ++nLine)
    {
        bool bHasRedlineColor = false;
        for (int i = 0; i < 36 && !bHasRedlineColor; ++i)
        {
            int nY = 96 + nLine * 36 + i;
            for (sal_uInt32 j = 0; j < nTileSize - 1; ++j)
            {
                Color aColor(pAccess->GetPixel(nY, j));
                Color aColor2(pAccess->GetPixel(nY+1, j));
                Color aColor3(pAccess->GetPixel(nY, j+1));
                Color aColor4(pAccess->GetPixel(nY+1, j+1));
                // 4-pixel same color square sign strikeout or underline of redlining
                // if its color is not white, black or non-printing character color
                if ( aColor == aColor2 && aColor == aColor3 && aColor == aColor4 &&
                    aColor != COL_WHITE && aColor != COL_BLACK &&
                    aColor != NON_PRINTING_CHARACTER_COLOR )
                {
                    bHasRedlineColor = true;
                    break;
                }
            }
        }

        CPPUNIT_ASSERT_MESSAGE(aTexts[nLine], bHasRedlineColor);
    }

    comphelper::dispatchCommand(".uno:ControlCodes", {});
}

void SwTiledRenderingTest::testClipText()
{
    // Load a document where the top left tile contains table text with
    // too small line height, but with top and bottom paragraph margins,
    // avoiding of clipping top and bottom parts of the characters.
    SwXTextDocument* pXTextDocument = createDoc("tdf117448.fodt");

    // Render a larger area, and then get the top and bottom of the text in that tile
    size_t nCanvasWidth = 1024;
    size_t nCanvasHeight = 512;
    size_t nTileSize = 256;
    std::vector<unsigned char> aPixmap(nCanvasWidth * nCanvasHeight * 4, 0);
    ScopedVclPtrInstance<VirtualDevice> pDevice(DeviceFormat::DEFAULT);
    pDevice->SetBackground(Wallpaper(COL_TRANSPARENT));
    pDevice->SetOutputSizePixelScaleOffsetAndBuffer(Size(nCanvasWidth, nCanvasHeight),
                                                    Fraction(1.0), Point(), aPixmap.data());
    pXTextDocument->paintTile(*pDevice, nCanvasWidth, nCanvasHeight, /*nTilePosX=*/0,
                              /*nTilePosY=*/0, /*nTileWidth=*/15360, /*nTileHeight=*/7680);
    pDevice->EnableMapMode(false);
    Bitmap aBitmap = pDevice->GetBitmap(Point(0, 0), Size(nTileSize, nTileSize));
    Bitmap::ScopedReadAccess pAccess(aBitmap);

    // check top margin, it's not white completely (i.e. showing top of letter "T")
    bool bClipTop = true;
    for (int i = 0; i < 150; i++)
    {
        Color aTopTextColor(pAccess->GetPixel(98, 98 + i));
        if (aTopTextColor.R < 255)
        {
            bClipTop = false;
            break;
        }
    }
    CPPUNIT_ASSERT(!bClipTop);
    // switch off because of false alarm on some platform, maybe related to font replacements
#if 0
    // check bottom margin, it's not white completely (i.e. showing bottom of letter "g")
    bool bClipBottom = true;
    for (int i = 0; i < 150; i++)
    {
        Color aBottomTextColor(pAccess->GetPixel(110, 98 + i));
        if (aBottomTextColor.R < 255)
        {
            bClipBottom = false;
            break;
        }
    }
    CPPUNIT_ASSERT(!bClipBottom);
#endif
}

void SwTiledRenderingTest::testAnchorTypes()
{
    SwXTextDocument* pXTextDocument = createDoc("shape.fodt");
    SwDoc* pDoc = pXTextDocument->GetDocShell()->GetDoc();
    SwView* pView = pXTextDocument->GetDocShell()->GetView();
    pView->GetViewFrame()->GetDispatcher()->Execute(FN_CNTNT_TO_NEXT_FRAME, SfxCallMode::SYNCHRON);
    SfxItemSet aSet(pDoc->GetAttrPool(), svl::Items<FN_TOOL_ANCHOR_PAGE, FN_TOOL_ANCHOR_PAGE>{});
    SfxBoolItem aItem(FN_TOOL_ANCHOR_PAGE);
    aSet.Put(aItem);
    auto pShell = dynamic_cast<SwBaseShell*>(pView->GetCurShell());
    pShell->GetState(aSet);
    // Without the accompanying fix in place, this test would have failed, setting the anchor type
    // to other than as/at-char was possible.
    CPPUNIT_ASSERT(!aSet.HasItem(FN_TOOL_ANCHOR_PAGE));
}

void SwTiledRenderingTest::testLanguageStatus()
{
    SwXTextDocument* pXTextDocument = createDoc("dummy.fodt");
    SwView* pView = pXTextDocument->GetDocShell()->GetView();
    std::unique_ptr<SfxPoolItem> pItem;
    pView->GetViewFrame()->GetBindings().QueryState(SID_LANGUAGE_STATUS, pItem);
    auto pStringListItem = dynamic_cast<SfxStringListItem*>(pItem.get());
    CPPUNIT_ASSERT(pStringListItem);

    uno::Sequence< OUString > aList;
    pStringListItem->GetStringList(aList);
    CPPUNIT_ASSERT_EQUAL(OUString("English (USA);en-US"), aList[0]);
}

void SwTiledRenderingTest::testRedlineNotificationDuringSave()
{
    // Load a document with redlines which are hidden at a layout level.
    // It's an empty document, just settings.xml and content.xml are custom.
    SwXTextDocument* pXTextDocument = createDoc("redline-notification-during-save.odt");
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    setupLibreOfficeKitViewCallback(pWrtShell->GetSfxViewShell());

    // Save the document.
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("writer8");
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have never returned due to an infinite
    // loop while sending not needed LOK notifications for redline changes during save.
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
}

void SwTiledRenderingTest::testHyperlink()
{
    comphelper::LibreOfficeKit::setViewIdForVisCursorInvalidation(true);
    SwXTextDocument* pXTextDocument = createDoc("hyperlink.odt");
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    setupLibreOfficeKitViewCallback(pWrtShell->GetSfxViewShell());
    m_callbackWrapper.setLOKViewId(SfxLokHelper::getView(pWrtShell->GetSfxViewShell()));
    SwShellCursor* pShellCursor = pWrtShell->getShellCursor(false);

    Point aStart = pShellCursor->GetSttPos();
    aStart.setX(aStart.getX() + 1800);
    pXTextDocument->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONDOWN, aStart.getX(), aStart.getY(), 1,
                                   MOUSE_LEFT, 0);
    pXTextDocument->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONUP, aStart.getX(), aStart.getY(), 1,
                                   MOUSE_LEFT, 0);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(OString("hyperlink"), m_sHyperlinkText);
    CPPUNIT_ASSERT_EQUAL(OString("http://example.com/"), m_sHyperlinkLink);
}

void SwTiledRenderingTest::testFieldmark()
{
    // Without the accompanying fix in place, this crashed on load.
    createDoc("fieldmark.docx");
}

void SwTiledRenderingTest::testDropDownFormFieldButton()
{
    SwXTextDocument* pXTextDocument = createDoc("drop_down_form_field.odt");
    pXTextDocument->setClientVisibleArea(tools::Rectangle(0, 0, 10000, 4000));

    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    setupLibreOfficeKitViewCallback(pWrtShell->GetSfxViewShell());

    // Move the cursor to trigger displaying of the field button.
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    CPPUNIT_ASSERT(m_aFormFieldButton.isEmpty());

    // Do a tile rendering to trigger the button message with a valid text area
    size_t nCanvasWidth = 1024;
    size_t nCanvasHeight = 512;
    std::vector<unsigned char> aPixmap(nCanvasWidth * nCanvasHeight * 4, 0);
    ScopedVclPtrInstance<VirtualDevice> pDevice(DeviceFormat::DEFAULT);
    pDevice->SetBackground(Wallpaper(COL_TRANSPARENT));
    pDevice->SetOutputSizePixelScaleOffsetAndBuffer(Size(nCanvasWidth, nCanvasHeight),
                                                    Fraction(1.0), Point(), aPixmap.data());
    pXTextDocument->paintTile(*pDevice, nCanvasWidth, nCanvasHeight, /*nTilePosX=*/0,
                              /*nTilePosY=*/0, /*nTileWidth=*/10000, /*nTileHeight=*/4000);

    CPPUNIT_ASSERT(!m_aFormFieldButton.isEmpty());
    {
        std::stringstream aStream(m_aFormFieldButton.getStr());
        boost::property_tree::ptree aTree;
        boost::property_tree::read_json(aStream, aTree);

        OString sAction = aTree.get_child("action").get_value<std::string>().c_str();
        CPPUNIT_ASSERT_EQUAL(OString("show"), sAction);

        OString sType = aTree.get_child("type").get_value<std::string>().c_str();
        CPPUNIT_ASSERT_EQUAL(OString("drop-down"), sType);

        OString sTextArea = aTree.get_child("textArea").get_value<std::string>().c_str();
        CPPUNIT_ASSERT_EQUAL(OString("1538, 1418, 1026, 275"), sTextArea);

        boost::property_tree::ptree aItems = aTree.get_child("params").get_child("items");
        CPPUNIT_ASSERT_EQUAL(size_t(6), aItems.size());

        OStringBuffer aItemList;
        for (auto &item : aItems)
        {
            aItemList.append(item.second.get_value<std::string>().c_str());
            aItemList.append(";");
        }
        CPPUNIT_ASSERT_EQUAL(OString("2019/2020;2020/2021;2021/2022;2022/2023;2023/2024;2024/2025;"), aItemList.toString());

        OString sSelected = aTree.get_child("params").get_child("selected").get_value<std::string>().c_str();
        CPPUNIT_ASSERT_EQUAL(OString("1"), sSelected);

        OString sPlaceholder = aTree.get_child("params").get_child("placeholderText").get_value<std::string>().c_str();
        CPPUNIT_ASSERT_EQUAL(OString("No Item specified"), sPlaceholder);
    }

    // Move the cursor back so the button becomes hidden.
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/false, 1, /*bBasicCall=*/false);

    CPPUNIT_ASSERT(!m_aFormFieldButton.isEmpty());
    {
        std::stringstream aStream(m_aFormFieldButton.getStr());
        boost::property_tree::ptree aTree;
        boost::property_tree::read_json(aStream, aTree);

        OString sAction = aTree.get_child("action").get_value<std::string>().c_str();
        CPPUNIT_ASSERT_EQUAL(OString("hide"), sAction);

        OString sType = aTree.get_child("type").get_value<std::string>().c_str();
        CPPUNIT_ASSERT_EQUAL(OString("drop-down"), sType);
    }
}

void SwTiledRenderingTest::testDropDownFormFieldButtonEditing()
{
    SwXTextDocument* pXTextDocument = createDoc("drop_down_form_field2.odt");
    pXTextDocument->setClientVisibleArea(tools::Rectangle(0, 0, 10000, 4000));

    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    setupLibreOfficeKitViewCallback(pWrtShell->GetSfxViewShell());

    // Move the cursor to trigger displaying of the field button.
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    CPPUNIT_ASSERT(m_aFormFieldButton.isEmpty());

    // Do a tile rendering to trigger the button message with a valid text area
    size_t nCanvasWidth = 1024;
    size_t nCanvasHeight = 512;
    std::vector<unsigned char> aPixmap(nCanvasWidth * nCanvasHeight * 4, 0);
    ScopedVclPtrInstance<VirtualDevice> pDevice(DeviceFormat::DEFAULT);
    pDevice->SetBackground(Wallpaper(COL_TRANSPARENT));
    pDevice->SetOutputSizePixelScaleOffsetAndBuffer(Size(nCanvasWidth, nCanvasHeight),
                                                    Fraction(1.0), Point(), aPixmap.data());
    pXTextDocument->paintTile(*pDevice, nCanvasWidth, nCanvasHeight, /*nTilePosX=*/0,
                              /*nTilePosY=*/0, /*nTileWidth=*/10000, /*nTileHeight=*/4000);

    // The item with the index '1' is selected by default
    CPPUNIT_ASSERT(!m_aFormFieldButton.isEmpty());
    {
        std::stringstream aStream(m_aFormFieldButton.getStr());
        boost::property_tree::ptree aTree;
        boost::property_tree::read_json(aStream, aTree);

        OString sSelected = aTree.get_child("params").get_child("selected").get_value<std::string>().c_str();
        CPPUNIT_ASSERT_EQUAL(OString("1"), sSelected);
    }
    m_aFormFieldButton = "";

    // Trigger a form field event to select a different item.
    vcl::ITiledRenderable::StringMap aArguments;
    aArguments["type"] = "drop-down";
    aArguments["cmd"] = "selected";
    aArguments["data"] = "3";
    pXTextDocument->executeFromFieldEvent(aArguments);

    // Do a tile rendering to trigger the button message.
    pXTextDocument->paintTile(*pDevice, nCanvasWidth, nCanvasHeight, /*nTilePosX=*/0,
                              /*nTilePosY=*/0, /*nTileWidth=*/10000, /*nTileHeight=*/4000);

    CPPUNIT_ASSERT(!m_aFormFieldButton.isEmpty());
    {
        std::stringstream aStream(m_aFormFieldButton.getStr());
        boost::property_tree::ptree aTree;
        boost::property_tree::read_json(aStream, aTree);

        OString sSelected = aTree.get_child("params").get_child("selected").get_value<std::string>().c_str();
        CPPUNIT_ASSERT_EQUAL(OString("3"), sSelected);
    }
}

void SwTiledRenderingTest::testDropDownFormFieldButtonNoSelection()
{
    SwXTextDocument* pXTextDocument = createDoc("drop_down_form_field_noselection.odt");
    pXTextDocument->setClientVisibleArea(tools::Rectangle(0, 0, 10000, 4000));

    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    setupLibreOfficeKitViewCallback(pWrtShell->GetSfxViewShell());

    // Move the cursor to trigger displaying of the field button.
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    CPPUNIT_ASSERT(m_aFormFieldButton.isEmpty());

    // Do a tile rendering to trigger the button message with a valid text area
    size_t nCanvasWidth = 1024;
    size_t nCanvasHeight = 512;
    std::vector<unsigned char> aPixmap(nCanvasWidth * nCanvasHeight * 4, 0);
    ScopedVclPtrInstance<VirtualDevice> pDevice(DeviceFormat::DEFAULT);
    pDevice->SetBackground(Wallpaper(COL_TRANSPARENT));
    pDevice->SetOutputSizePixelScaleOffsetAndBuffer(Size(nCanvasWidth, nCanvasHeight),
                                                    Fraction(1.0), Point(), aPixmap.data());
    pXTextDocument->paintTile(*pDevice, nCanvasWidth, nCanvasHeight, /*nTilePosX=*/0,
                              /*nTilePosY=*/0, /*nTileWidth=*/10000, /*nTileHeight=*/4000);

    // None of the items is selected
    CPPUNIT_ASSERT(!m_aFormFieldButton.isEmpty());
    {
        std::stringstream aStream(m_aFormFieldButton.getStr());
        boost::property_tree::ptree aTree;
        boost::property_tree::read_json(aStream, aTree);

        OString sSelected = aTree.get_child("params").get_child("selected").get_value<std::string>().c_str();
        CPPUNIT_ASSERT_EQUAL(OString("-1"), sSelected);
    }
}

static void lcl_extractHandleParameters(const OString& selection, int& id, int& x, int& y)
{
    OString extraInfo = selection.copy(selection.indexOf("{"));
    std::stringstream aStream(extraInfo.getStr());
    boost::property_tree::ptree aTree;
    boost::property_tree::read_json(aStream, aTree);
    boost::property_tree::ptree
        handle0 = aTree
            .get_child("handles")
            .get_child("kinds")
            .get_child("rectangle")
            .get_child("1")
            .begin()->second;
    id = handle0.get_child("id").get_value<int>();
    x = handle0.get_child("point").get_child("x").get_value<int>();
    y = handle0.get_child("point").get_child("y").get_value<int>();
}

void SwTiledRenderingTest::testMoveShapeHandle()
{
    comphelper::LibreOfficeKit::setActive();
    SwXTextDocument* pXTextDocument = createDoc("shape.fodt");

    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    setupLibreOfficeKitViewCallback(pWrtShell->GetSfxViewShell());
    SdrPage* pPage = pWrtShell->GetDoc()->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pObject = pPage->GetObj(0);
    pWrtShell->SelectObj(Point(), 0, pObject);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT(!m_ShapeSelection.isEmpty());
    {
        int id, x, y;
        lcl_extractHandleParameters(m_ShapeSelection, id, x ,y);
        int oldX = x;
        int oldY = y;
        uno::Sequence<beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence(
        {
            {"HandleNum", uno::makeAny(id)},
            {"NewPosX", uno::makeAny(x+1)},
            {"NewPosY", uno::makeAny(y+1)}
        }));
        comphelper::dispatchCommand(".uno:MoveShapeHandle", aPropertyValues);
        Scheduler::ProcessEventsToIdle();
        CPPUNIT_ASSERT(!m_ShapeSelection.isEmpty());
        lcl_extractHandleParameters(m_ShapeSelection, id, x ,y);
        CPPUNIT_ASSERT_EQUAL(x-1, oldX);
        CPPUNIT_ASSERT_EQUAL(y-1, oldY);
    }
}

void SwTiledRenderingTest::testDropDownFormFieldButtonNoItem()
{
    SwXTextDocument* pXTextDocument = createDoc("drop_down_form_field_noitem.odt");
    pXTextDocument->setClientVisibleArea(tools::Rectangle(0, 0, 10000, 4000));

    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    setupLibreOfficeKitViewCallback(pWrtShell->GetSfxViewShell());

    // Move the cursor to trigger displaying of the field button.
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    CPPUNIT_ASSERT(m_aFormFieldButton.isEmpty());

    // Do a tile rendering to trigger the button message with a valid text area
    size_t nCanvasWidth = 1024;
    size_t nCanvasHeight = 512;
    std::vector<unsigned char> aPixmap(nCanvasWidth * nCanvasHeight * 4, 0);
    ScopedVclPtrInstance<VirtualDevice> pDevice(DeviceFormat::DEFAULT);
    pDevice->SetBackground(Wallpaper(COL_TRANSPARENT));
    pDevice->SetOutputSizePixelScaleOffsetAndBuffer(Size(nCanvasWidth, nCanvasHeight),
                                                    Fraction(1.0), Point(), aPixmap.data());
    pXTextDocument->paintTile(*pDevice, nCanvasWidth, nCanvasHeight, /*nTilePosX=*/0,
                              /*nTilePosY=*/0, /*nTileWidth=*/10000, /*nTileHeight=*/4000);

    // There is not item specified for the field
    CPPUNIT_ASSERT(!m_aFormFieldButton.isEmpty());
    {
        std::stringstream aStream(m_aFormFieldButton.getStr());
        boost::property_tree::ptree aTree;
        boost::property_tree::read_json(aStream, aTree);

        boost::property_tree::ptree aItems = aTree.get_child("params").get_child("items");
        CPPUNIT_ASSERT_EQUAL(size_t(0), aItems.size());

        OString sSelected = aTree.get_child("params").get_child("selected").get_value<std::string>().c_str();
        CPPUNIT_ASSERT_EQUAL(OString("-1"), sSelected);
    }
}

void SwTiledRenderingTest::testTablePaintInvalidate()
{
    // Load a document with a table in it.
    SwXTextDocument* pXTextDocument = createDoc("table-paint-invalidate.odt");
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    setupLibreOfficeKitViewCallback(pWrtShell->GetSfxViewShell());
    // Enter the table.
    pWrtShell->Down(/*bSelect=*/false);
    Scheduler::ProcessEventsToIdle();
    m_nInvalidations = 0;

    // Paint a tile.
    size_t nCanvasWidth = 256;
    size_t nCanvasHeight = 256;
    std::vector<unsigned char> aPixmap(nCanvasWidth * nCanvasHeight * 4, 0);
    ScopedVclPtrInstance<VirtualDevice> pDevice(DeviceFormat::DEFAULT);
    pDevice->SetBackground(Wallpaper(COL_TRANSPARENT));
    pDevice->SetOutputSizePixelScaleOffsetAndBuffer(Size(nCanvasWidth, nCanvasHeight),
                                                    Fraction(1.0), Point(), aPixmap.data());
    pXTextDocument->paintTile(*pDevice, nCanvasWidth, nCanvasHeight, m_aInvalidation.getX(),
                              m_aInvalidation.getY(), /*nTileWidth=*/1000,
                              /*nTileHeight=*/1000);
    Scheduler::ProcessEventsToIdle();

    // Without the accompanying fix in place, this test would have failed with
    // - Expected: 0
    // - Actual  : 5
    // i.e. paint generated an invalidation, which caused a loop.
    CPPUNIT_ASSERT_EQUAL(0, m_nInvalidations);
}

void SwTiledRenderingTest::testSpellOnlineRenderParameter()
{
    SwXTextDocument* pXTextDocument = createDoc("dummy.fodt");
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    const SwViewOption* pOpt = pWrtShell->GetViewOptions();
    bool bSet = pOpt->IsOnlineSpell();

    uno::Sequence<beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence(
    {
        {".uno:SpellOnline", uno::makeAny(!bSet)},
    }));
    pXTextDocument->initializeForTiledRendering(aPropertyValues);
    CPPUNIT_ASSERT_EQUAL(!bSet, pOpt->IsOnlineSpell());
}

void SwTiledRenderingTest::testExtTextInputReadOnly()
{
    // Create a document with a protected section + a normal paragraph after it.
    SwXTextDocument* pXTextDocument = createDoc();
    uno::Reference<text::XTextViewCursorSupplier> xController(
        pXTextDocument->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursor> xCursor = xController->getViewCursor();
    uno::Reference<text::XText> xText = xCursor->getText();
    uno::Reference<text::XTextContent> xSection(
        pXTextDocument->createInstance("com.sun.star.text.TextSection"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xSectionProps(xSection, uno::UNO_QUERY);
    xSectionProps->setPropertyValue("IsProtected", uno::Any(true));
    xText->insertTextContent(xCursor, xSection, /*bAbsorb=*/true);

    // First paragraph is the protected section, is it empty?
    VclPtr<vcl::Window> pEditWin = pXTextDocument->getDocWindow();
    CPPUNIT_ASSERT(pEditWin);
    CPPUNIT_ASSERT(getParagraph(1)->getString().isEmpty());

    // Try to type into the protected section, is it still empty?
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    pWrtShell->SttEndDoc(/*bStt=*/true);
    SfxLokHelper::postExtTextEventAsync(pEditWin, LOK_EXT_TEXTINPUT, "x");
    SfxLokHelper::postExtTextEventAsync(pEditWin, LOK_EXT_TEXTINPUT_END, "x");
    Scheduler::ProcessEventsToIdle();
    // Without the accompanying fix in place, this test would have failed, as it was possible to
    // type into the protected section.
    CPPUNIT_ASSERT(getParagraph(1)->getString().isEmpty());

    // Second paragraph is a normal paragraph, is it empty?
    pWrtShell->Down(/*bSelect=*/false);
    CPPUNIT_ASSERT(getParagraph(2)->getString().isEmpty());

    // Try to type into the protected section, does it have the typed content?
    SfxLokHelper::postExtTextEventAsync(pEditWin, LOK_EXT_TEXTINPUT, "x");
    SfxLokHelper::postExtTextEventAsync(pEditWin, LOK_EXT_TEXTINPUT_END, "x");
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(OUString("x"), getParagraph(2)->getString());
}

void SwTiledRenderingTest::testBulletDeleteInvalidation()
{
    // Given a document with 3 paragraphs: first 2 is bulleted, the last is not.
    SwXTextDocument* pXTextDocument = createDoc();
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    pWrtShell->SplitNode();
    pWrtShell->Up(/*bSelect=*/false);
    pWrtShell->StartAllAction();
    pWrtShell->BulletOn();
    pWrtShell->EndAllAction();
    pWrtShell->Insert2("a");
    pWrtShell->SplitNode();
    pWrtShell->Insert2("b");
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->GetLayout()->PaintSwFrame(*pWrtShell->GetOut(),
                                         pWrtShell->GetLayout()->getFrameArea());
    Scheduler::ProcessEventsToIdle();
    setupLibreOfficeKitViewCallback(pWrtShell->GetSfxViewShell());
    m_aInvalidations = tools::Rectangle();

    // When pressing backspace in the last paragraph.
    pWrtShell->DelLeft();

    // Then the first paragraph should not be invalidated.
    SwRootFrame* pRoot = pWrtShell->GetLayout();
    SwFrame* pPage = pRoot->GetLower();
    SwFrame* pBody = pPage->GetLower();
    SwFrame* pFirstText = pBody->GetLower();
    tools::Rectangle aFirstTextRect = pFirstText->getFrameArea().SVRect();
    CPPUNIT_ASSERT(!aFirstTextRect.IsOver(m_aInvalidations));
}

void SwTiledRenderingTest::testBulletNoNumInvalidation()
{
    // Given a document with 3 paragraphs: all are bulleted.
    SwXTextDocument* pXTextDocument = createDoc();
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    pWrtShell->StartAllAction();
    pWrtShell->BulletOn();
    pWrtShell->EndAllAction();
    pWrtShell->Insert2("a");
    pWrtShell->SplitNode();
    pWrtShell->Insert2("b");
    pWrtShell->SplitNode();
    pWrtShell->GetLayout()->PaintSwFrame(*pWrtShell->GetOut(),
                                         pWrtShell->GetLayout()->getFrameArea());
    Scheduler::ProcessEventsToIdle();
    setupLibreOfficeKitViewCallback(pWrtShell->GetSfxViewShell());
    m_aInvalidations = tools::Rectangle();

    // When pressing backspace in the last paragraph to turn bullets off.
    pWrtShell->StartAllAction();
    pWrtShell->NumOrNoNum(/*bDelete=*/false);
    pWrtShell->EndAllAction();

    // Then the first paragraph should not be invalidated.
    SwRootFrame* pRoot = pWrtShell->GetLayout();
    SwFrame* pPage = pRoot->GetLower();
    SwFrame* pBody = pPage->GetLower();
    SwFrame* pFirstText = pBody->GetLower();
    tools::Rectangle aFirstTextRect = pFirstText->getFrameArea().SVRect();
    CPPUNIT_ASSERT(!aFirstTextRect.IsOver(m_aInvalidations));
}

void SwTiledRenderingTest::testBulletMultiDeleteInvalidation()
{
    // Given a document with 5 paragraphs: all are bulleted.
    SwXTextDocument* pXTextDocument = createDoc();
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    pWrtShell->StartAllAction();
    pWrtShell->BulletOn();
    pWrtShell->EndAllAction();
    // There is alredy an initial text node, so type 5 times, but split 4 times.
    for (int i = 0; i < 4; ++i)
    {
        pWrtShell->Insert2("a");
        pWrtShell->SplitNode();
    }
    pWrtShell->Insert2("a");
    // Go to the end of the 4th para.
    pWrtShell->Up(/*bSelect=*/false);
    pWrtShell->GetLayout()->PaintSwFrame(*pWrtShell->GetOut(),
                                         pWrtShell->GetLayout()->getFrameArea());
    Scheduler::ProcessEventsToIdle();
    setupLibreOfficeKitViewCallback(pWrtShell->GetSfxViewShell());
    m_aInvalidations = tools::Rectangle();

    // When selecting and deleting several bullets: select till the end of the 2nd para and delete.
    pWrtShell->Up(/*bSelect=*/true, /*nCount=*/2);
    pWrtShell->DelRight();

    // Then the first paragraph should not be invalidated.
    SwRootFrame* pRoot = pWrtShell->GetLayout();
    SwFrame* pPage = pRoot->GetLower();
    SwFrame* pBody = pPage->GetLower();
    SwFrame* pFirstText = pBody->GetLower();
    tools::Rectangle aFirstTextRect = pFirstText->getFrameArea().SVRect();
    CPPUNIT_ASSERT(!aFirstTextRect.IsOver(m_aInvalidations));
}

void SwTiledRenderingTest::testCondCollCopy()
{
    // Given a document with a custom Text Body cond style:
    SwXTextDocument* pXTextDocument = createDoc("cond-coll-copy.odt");
    uno::Sequence<beans::PropertyValue> aPropertyValues
        = { comphelper::makePropertyValue("Style", OUString("Text Body")),
            comphelper::makePropertyValue("FamilyName", OUString("ParagraphStyles")) };
    dispatchCommand(mxComponent, ".uno:StyleApply", aPropertyValues);
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    pWrtShell->SelAll();

    // When getting the text selection, then make sure it doesn't crash:
    uno::Reference<datatransfer::XTransferable2> xTransferable(pXTextDocument->getSelection(),
                                                               css::uno::UNO_QUERY);
    datatransfer::DataFlavor aFlavor;
    aFlavor.MimeType = "text/plain;charset=utf-16";
    aFlavor.DataType = cppu::UnoType<OUString>::get();
    CPPUNIT_ASSERT(xTransferable->isDataFlavorSupported(aFlavor));
    // Without the accompanying fix in place, this test would have crashed.
    xTransferable->getTransferData(aFlavor);
}

CPPUNIT_TEST_SUITE_REGISTRATION(SwTiledRenderingTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
