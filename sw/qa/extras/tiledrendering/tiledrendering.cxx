/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <string>
#include <boost/property_tree/json_parser.hpp>

#include <swmodeltestbase.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <comphelper/dispatchcommand.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/string.hxx>
#include <comphelper/lok.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdview.hxx>
#include <vcl/svapp.hxx>
#include <editeng/editview.hxx>
#include <editeng/outliner.hxx>
#include <svl/srchitem.hxx>
#include <drawdoc.hxx>
#include <ndtxt.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <UndoManager.hxx>
#include <cmdid.h>
#include <sfx2/viewsh.hxx>
#include <sfx2/lokhelper.hxx>

static const char* DATA_DIRECTORY = "/sw/qa/extras/tiledrendering/data/";

/// Testsuite for the SwXTextDocument methods implementing the vcl::ITiledRenderable interface.
class SwTiledRenderingTest : public SwModelTestBase
{
public:
    SwTiledRenderingTest();
    void testRegisterCallback();
    void testPostKeyEvent();
    void testPostMouseEvent();
    void testSetTextSelection();
    void testGetTextSelection();
    void testSetGraphicSelection();
    void testResetSelection();
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

    CPPUNIT_TEST_SUITE(SwTiledRenderingTest);
    CPPUNIT_TEST(testRegisterCallback);
    CPPUNIT_TEST(testPostKeyEvent);
    CPPUNIT_TEST(testPostMouseEvent);
    CPPUNIT_TEST(testSetTextSelection);
    CPPUNIT_TEST(testGetTextSelection);
    CPPUNIT_TEST(testSetGraphicSelection);
    CPPUNIT_TEST(testResetSelection);
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
    CPPUNIT_TEST_SUITE_END();

private:
    SwXTextDocument* createDoc(const char* pName);
    static void callback(int nType, const char* pPayload, void* pData);
    void callbackImpl(int nType, const char* pPayload);
    Rectangle m_aInvalidation;
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
};

SwTiledRenderingTest::SwTiledRenderingTest()
    : m_bFound(true),
      m_nSelectionBeforeSearchResult(0),
      m_nSelectionAfterSearchResult(0),
      m_nInvalidations(0),
      m_nRedlineTableSizeChanged(0),
      m_nRedlineTableEntryModified(0),
      m_nTrackedChangeIndex(-1)
{
}

SwXTextDocument* SwTiledRenderingTest::createDoc(const char* pName)
{
    load(DATA_DIRECTORY, pName);

    SwXTextDocument* pTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDocument);
    pTextDocument->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    return pTextDocument;
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
        if (m_aInvalidation.IsEmpty())
        {
            uno::Sequence<OUString> aSeq = comphelper::string::convertCommaSeparated(OUString::createFromAscii(pPayload));
            if (OString("EMPTY") == pPayload)
                return;
            CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(4), aSeq.getLength());
            m_aInvalidation.setX(aSeq[0].toInt32());
            m_aInvalidation.setY(aSeq[1].toInt32());
            m_aInvalidation.setWidth(aSeq[2].toInt32());
            m_aInvalidation.setHeight(aSeq[3].toInt32());
        }
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
        for (boost::property_tree::ptree::value_type& rValue : aTree.get_child("searchResultSelection"))
        {
            m_aSearchResultSelection.push_back(rValue.second.get<std::string>("rectangles").c_str());
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
    }
}

void SwTiledRenderingTest::testRegisterCallback()
{
    comphelper::LibreOfficeKit::setActive();
    SwXTextDocument* pXTextDocument = createDoc("dummy.fodt");
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    pWrtShell->GetSfxViewShell()->registerLibreOfficeKitViewCallback(&SwTiledRenderingTest::callback, this);
    // Insert a character at the beginning of the document.
    pWrtShell->Insert("x");

    // Check that the top left 256x256px tile would be invalidated.
    CPPUNIT_ASSERT(!m_aInvalidation.IsEmpty());
    Rectangle aTopLeft(0, 0, 256*15, 256*15); // 1 px = 15 twips, assuming 96 DPI.
    CPPUNIT_ASSERT(m_aInvalidation.IsOver(aTopLeft));
    comphelper::LibreOfficeKit::setActive(false);
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
    // Did we manage to insert the character after the first one?
    CPPUNIT_ASSERT_EQUAL(OUString("Axaa bbb."), pShellCursor->GetPoint()->nNode.GetNode().GetTextNode()->GetText());
}

void SwTiledRenderingTest::testPostMouseEvent()
{
    comphelper::LibreOfficeKit::setActive();
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
    // The new cursor position must be before the first word.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), pShellCursor->GetPoint()->nContent.GetIndex());
    comphelper::LibreOfficeKit::setActive(false);
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
    comphelper::LibreOfficeKit::setActive();

    SwXTextDocument* pXTextDocument = createDoc("shape-with-text.fodt");
    // No crash, just empty output for unexpected mime type.
    OString aUsedFormat;
    CPPUNIT_ASSERT_EQUAL(OString(), pXTextDocument->getTextSelection("foo/bar", aUsedFormat));

    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    // Move the cursor into the first word.
    pWrtShell->Right(CRSR_SKIP_CHARS, /*bSelect=*/false, 2, /*bBasicCall=*/false);
    // Create a selection by on the word.
    pWrtShell->SelWrd();

    // Make sure that we selected text from the body text.
    CPPUNIT_ASSERT_EQUAL(OString("Hello"), pXTextDocument->getTextSelection("text/plain;charset=utf-8", aUsedFormat));

    // Make sure we produce something for HTML.
    CPPUNIT_ASSERT(!OString(pXTextDocument->getTextSelection("text/html", aUsedFormat)).isEmpty());

    // Now select some shape text and check again.
    SdrPage* pPage = pWrtShell->GetDoc()->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pObject = pPage->GetObj(0);
    SdrView* pView = pWrtShell->GetDrawView();
    pView->SdrBeginTextEdit(pObject);
    CPPUNIT_ASSERT(pView->GetTextEditObject());
    EditView& rEditView = pView->GetTextEditOutlinerView()->GetEditView();
    ESelection aWordSelection(0, 0, 0, 5);
    rEditView.SetSelection(aWordSelection);
    CPPUNIT_ASSERT_EQUAL(OString("Shape"), pXTextDocument->getTextSelection("text/plain;charset=utf-8", aUsedFormat));

    comphelper::LibreOfficeKit::setActive(false);
}

void SwTiledRenderingTest::testSetGraphicSelection()
{
    SwXTextDocument* pXTextDocument = createDoc("shape.fodt");
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    SdrPage* pPage = pWrtShell->GetDoc()->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pObject = pPage->GetObj(0);
    pWrtShell->SelectObj(Point(), 0, pObject);
    // Make sure the rectangle has 8 handles: at each corner and at the center of each edge.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(8), pObject->GetHdlCount());
    // Take the bottom center one.
    SdrHdl* pHdl = pObject->GetHdl(6);
    CPPUNIT_ASSERT_EQUAL((int)SdrHdlKind::Lower, (int)pHdl->GetKind());
    Rectangle aShapeBefore = pObject->GetSnapRect();
    // Resize.
    pXTextDocument->setGraphicSelection(LOK_SETGRAPHICSELECTION_START, pHdl->GetPos().getX(), pHdl->GetPos().getY());
    pXTextDocument->setGraphicSelection(LOK_SETGRAPHICSELECTION_END, pHdl->GetPos().getX(), pHdl->GetPos().getY() + 1000);
    Rectangle aShapeAfter = pObject->GetSnapRect();
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

void lcl_search(bool bBackward)
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
    comphelper::LibreOfficeKit::setActive();

    SwXTextDocument* pXTextDocument = createDoc("search.odt");
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    pWrtShell->GetSfxViewShell()->registerLibreOfficeKitViewCallback(&SwTiledRenderingTest::callback, this);
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

    comphelper::LibreOfficeKit::setActive(false);
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
    comphelper::LibreOfficeKit::setActive();

    SwXTextDocument* pXTextDocument = createDoc("search.odt");
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    pWrtShell->GetSfxViewShell()->registerLibreOfficeKitViewCallback(&SwTiledRenderingTest::callback, this);
    uno::Sequence<beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence(
    {
        {"SearchItem.SearchString", uno::makeAny(OUString("TextFrame"))},
        {"SearchItem.Backward", uno::makeAny(false)},
    }));
    comphelper::dispatchCommand(".uno:ExecuteSearch", aPropertyValues);
    // This was empty: nothing was highlighted after searching for 'TextFrame'.
    CPPUNIT_ASSERT(!m_aTextSelection.isEmpty());

    comphelper::LibreOfficeKit::setActive(false);
}

void SwTiledRenderingTest::testSearchTextFrameWrapAround()
{
    SwXTextDocument* pXTextDocument = createDoc("search.odt");
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    pWrtShell->GetSfxViewShell()->registerLibreOfficeKitViewCallback(&SwTiledRenderingTest::callback, this);
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
    comphelper::LibreOfficeKit::setActive();
    // Get the current document size.
    SwXTextDocument* pXTextDocument = createDoc("2-pages.odt");
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    pWrtShell->GetSfxViewShell()->registerLibreOfficeKitViewCallback(&SwTiledRenderingTest::callback, this);
    Size aSize = pXTextDocument->getDocumentSize();

    // Delete the second page and see how the size changes.
    pWrtShell->Down(false);
    pWrtShell->DelLeft();
    // Document width should not change, this was 0.
    CPPUNIT_ASSERT_EQUAL(aSize.getWidth(), m_aDocumentSize.getWidth());
    // Document height should be smaller now.
    CPPUNIT_ASSERT(aSize.getHeight() > m_aDocumentSize.getHeight());
    comphelper::LibreOfficeKit::setActive(false);
}

void SwTiledRenderingTest::testSearchAll()
{
    comphelper::LibreOfficeKit::setActive();

    SwXTextDocument* pXTextDocument = createDoc("search.odt");
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    pWrtShell->GetSfxViewShell()->registerLibreOfficeKitViewCallback(&SwTiledRenderingTest::callback, this);
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

    comphelper::LibreOfficeKit::setActive(false);
}

void SwTiledRenderingTest::testSearchAllNotifications()
{
    comphelper::LibreOfficeKit::setActive();
    SwXTextDocument* pXTextDocument = createDoc("search.odt");
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    pWrtShell->GetSfxViewShell()->registerLibreOfficeKitViewCallback(&SwTiledRenderingTest::callback, this);
    // Reset notification counter before search.
    m_nSelectionBeforeSearchResult = 0;
    uno::Sequence<beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence(
    {
        {"SearchItem.SearchString", uno::makeAny(OUString("shape"))},
        {"SearchItem.Backward", uno::makeAny(false)},
        {"SearchItem.Command", uno::makeAny(static_cast<sal_uInt16>(SvxSearchCmd::FIND_ALL))},
    }));
    comphelper::dispatchCommand(".uno:ExecuteSearch", aPropertyValues);
    Scheduler::ProcessAllPendingEvents();

    // This was 5, make sure that we get no notifications about selection changes during search.
    CPPUNIT_ASSERT_EQUAL(0, m_nSelectionBeforeSearchResult);
    // But we do get the selection afterwards.
    CPPUNIT_ASSERT(m_nSelectionAfterSearchResult > 0);

    comphelper::LibreOfficeKit::setActive(false);
}

void SwTiledRenderingTest::testPageDownInvalidation()
{
    comphelper::LibreOfficeKit::setActive();

    SwXTextDocument* pXTextDocument = createDoc("pagedown-invalidation.odt");
    uno::Sequence<beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence(
    {
        {".uno:HideWhitespace", uno::makeAny(true)},
    }));
    pXTextDocument->initializeForTiledRendering(aPropertyValues);
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    pWrtShell->GetSfxViewShell()->registerLibreOfficeKitViewCallback(&SwTiledRenderingTest::callback, this);
    comphelper::dispatchCommand(".uno:PageDown", uno::Sequence<beans::PropertyValue>());

    // This was 2.
    CPPUNIT_ASSERT_EQUAL(0, m_nInvalidations);

    comphelper::LibreOfficeKit::setActive(false);
}

void SwTiledRenderingTest::testPartHash()
{
    comphelper::LibreOfficeKit::setActive();

    SwXTextDocument* pXTextDocument = createDoc("pagedown-invalidation.odt");
    int nParts = pXTextDocument->getParts();
    for (int it = 0; it < nParts; it++)
    {
        CPPUNIT_ASSERT(!pXTextDocument->getPartHash(it).isEmpty());
    }

    comphelper::LibreOfficeKit::setActive(false);
}

/// A view callback tracks callbacks invoked on one specific view.
class ViewCallback
{
public:
    bool m_bOwnCursorInvalidated;
    Rectangle m_aOwnCursor;
    bool m_bViewCursorInvalidated;
    Rectangle m_aViewCursor;
    bool m_bOwnSelectionSet;
    bool m_bViewSelectionSet;
    OString m_aViewSelection;
    bool m_bTilesInvalidated;
    bool m_bViewCursorVisible;
    bool m_bGraphicViewSelection;
    bool m_bGraphicSelection;
    bool m_bViewLock;

    ViewCallback()
        : m_bOwnCursorInvalidated(false),
          m_bViewCursorInvalidated(false),
          m_bOwnSelectionSet(false),
          m_bViewSelectionSet(false),
          m_bTilesInvalidated(false),
          m_bViewCursorVisible(false),
          m_bGraphicViewSelection(false),
          m_bGraphicSelection(false),
          m_bViewLock(false)
    {
    }

    static void callback(int nType, const char* pPayload, void* pData)
    {
        static_cast<ViewCallback*>(pData)->callbackImpl(nType, pPayload);
    }

    void callbackImpl(int nType, const char* pPayload)
    {
        OString aPayload(pPayload);
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

            uno::Sequence<OUString> aSeq = comphelper::string::convertCommaSeparated(OUString::fromUtf8(aPayload));
            if (OString("EMPTY") == pPayload)
                return;
            CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(4), aSeq.getLength());
            m_aOwnCursor.setX(aSeq[0].toInt32());
            m_aOwnCursor.setY(aSeq[1].toInt32());
            m_aOwnCursor.setWidth(aSeq[2].toInt32());
            m_aOwnCursor.setHeight(aSeq[3].toInt32());
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
            m_bViewCursorVisible = OString("true") == pPayload;
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
        }
    }
};

void SwTiledRenderingTest::testMissingInvalidation()
{
    comphelper::LibreOfficeKit::setActive();

    // Create two views.
    SwXTextDocument* pXTextDocument = createDoc("dummy.fodt");
    ViewCallback aView1;
    SfxViewShell::Current()->registerLibreOfficeKitViewCallback(&ViewCallback::callback, &aView1);
    int nView1 = SfxLokHelper::getView();
    SfxLokHelper::createView();
    ViewCallback aView2;
    SfxViewShell::Current()->registerLibreOfficeKitViewCallback(&ViewCallback::callback, &aView2);
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
    aView1.m_bTilesInvalidated = false;
    aView2.m_bTilesInvalidated = false;
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::DELETE);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::DELETE);
    Scheduler::ProcessAllPendingEvents();
    CPPUNIT_ASSERT(aView1.m_bTilesInvalidated);
    CPPUNIT_ASSERT(aView2.m_bTilesInvalidated);
    mxComponent->dispose();
    mxComponent.clear();

    comphelper::LibreOfficeKit::setActive(false);
}

void SwTiledRenderingTest::testViewCursors()
{
    comphelper::LibreOfficeKit::setActive();

    SwXTextDocument* pXTextDocument = createDoc("dummy.fodt");
    ViewCallback aView1;
    SfxViewShell::Current()->registerLibreOfficeKitViewCallback(&ViewCallback::callback, &aView1);
    SfxLokHelper::createView();
    ViewCallback aView2;
    SfxViewShell::Current()->registerLibreOfficeKitViewCallback(&ViewCallback::callback, &aView2);
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
    SwShellCursor* pShellCursor = pWrtShell->getShellCursor(false);
    // Did we indeed manage to select the second word?
    CPPUNIT_ASSERT_EQUAL(OUString("bbb"), pShellCursor->GetText());
    CPPUNIT_ASSERT(!aView1.m_bOwnSelectionSet);
    // This failed, aView1 did not get notification about selection changes in
    // aView2.
    CPPUNIT_ASSERT(aView1.m_bViewSelectionSet);
    CPPUNIT_ASSERT(aView2.m_bOwnSelectionSet);
    CPPUNIT_ASSERT(!aView2.m_bViewSelectionSet);
    mxComponent->dispose();
    mxComponent.clear();

    comphelper::LibreOfficeKit::setActive(false);
}

void SwTiledRenderingTest::testShapeViewCursors()
{
    comphelper::LibreOfficeKit::setActive();

    // Load a document and create a view, so we have 2 ones.
    SwXTextDocument* pXTextDocument = createDoc("shape.fodt");
    ViewCallback aView1;
    SfxViewShell::Current()->registerLibreOfficeKitViewCallback(&ViewCallback::callback, &aView1);
    SfxLokHelper::createView();
    ViewCallback aView2;
    pXTextDocument->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    SfxViewShell::Current()->registerLibreOfficeKitViewCallback(&ViewCallback::callback, &aView2);
    SwWrtShell* pWrtShell2 = pXTextDocument->GetDocShell()->GetWrtShell();

    // Start shape text in the second view.
    SdrPage* pPage = pWrtShell2->GetDoc()->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pObject = pPage->GetObj(0);
    SdrView* pView = pWrtShell2->GetDrawView();
    pWrtShell2->GetView().BeginTextEdit(pObject, pView->GetSdrPageView(), pWrtShell2->GetWin());
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'x', 0);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 'x', 0);

    // Press a key in the second view, while the first one observes this.
    aView1.m_bOwnCursorInvalidated = false;
    aView1.m_bViewCursorInvalidated = false;
    aView2.m_bOwnCursorInvalidated = false;
    aView2.m_bViewCursorInvalidated = false;
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'y', 0);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 'y', 0);
    // Make sure that aView1 gets a view-only cursor notification, while
    // aView2 gets a real cursor notification.
    CPPUNIT_ASSERT(!aView1.m_bOwnCursorInvalidated);
    CPPUNIT_ASSERT(aView1.m_bViewCursorInvalidated);
    CPPUNIT_ASSERT(aView2.m_bOwnCursorInvalidated);
    CPPUNIT_ASSERT(!aView2.m_bViewCursorInvalidated);
    mxComponent->dispose();
    mxComponent.clear();

    comphelper::LibreOfficeKit::setActive(false);
}

void SwTiledRenderingTest::testViewCursorVisibility()
{
    comphelper::LibreOfficeKit::setActive();

    // Load a document that has a shape and create two views.
    SwXTextDocument* pXTextDocument = createDoc("shape.fodt");
    ViewCallback aView1;
    SfxViewShell::Current()->registerLibreOfficeKitViewCallback(&ViewCallback::callback, &aView1);
    SfxLokHelper::createView();
    pXTextDocument->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ViewCallback aView2;
    SfxViewShell::Current()->registerLibreOfficeKitViewCallback(&ViewCallback::callback, &aView2);

    // Click on the shape in the second view.
    aView1.m_bViewCursorVisible = true;
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    SdrPage* pPage = pWrtShell->GetDoc()->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pObject = pPage->GetObj(0);
    Point aCenter = pObject->GetSnapRect().Center();
    pXTextDocument->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONDOWN, aCenter.getX(), aCenter.getY(), 1, MOUSE_LEFT, 0);
    pXTextDocument->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONUP, aCenter.getX(), aCenter.getY(), 1, MOUSE_LEFT, 0);
    Scheduler::ProcessAllPendingEvents();
    // Make sure the "view/text" cursor of the first view gets a notification.
    CPPUNIT_ASSERT(!aView1.m_bViewCursorVisible);
    mxComponent->dispose();
    mxComponent.clear();

    comphelper::LibreOfficeKit::setActive(false);
}

void SwTiledRenderingTest::testViewCursorCleanup()
{
    comphelper::LibreOfficeKit::setActive();

    // Load a document that has a shape and create two views.
    SwXTextDocument* pXTextDocument = createDoc("shape.fodt");
    ViewCallback aView1;
    SfxViewShell::Current()->registerLibreOfficeKitViewCallback(&ViewCallback::callback, &aView1);
    int nView2 = SfxLokHelper::createView();
    pXTextDocument->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ViewCallback aView2;
    SfxViewShell::Current()->registerLibreOfficeKitViewCallback(&ViewCallback::callback, &aView2);

    // Click on the shape in the second view.
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    SdrPage* pPage = pWrtShell->GetDoc()->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pObject = pPage->GetObj(0);
    Point aCenter = pObject->GetSnapRect().Center();
    aView1.m_bGraphicViewSelection = false;
    pXTextDocument->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONDOWN, aCenter.getX(), aCenter.getY(), 1, MOUSE_LEFT, 0);
    pXTextDocument->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONUP, aCenter.getX(), aCenter.getY(), 1, MOUSE_LEFT, 0);
    Scheduler::ProcessAllPendingEvents();
    // Make sure there is a graphic view selection on the first view.
    CPPUNIT_ASSERT(aView1.m_bGraphicViewSelection);

    // Now destroy the second view.
    SfxLokHelper::destroyView(nView2);
    Scheduler::ProcessAllPendingEvents();
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1), SfxLokHelper::getViewsCount());
    // Make sure that the graphic view selection on the first view is cleaned up.
    CPPUNIT_ASSERT(!aView1.m_bGraphicViewSelection);
    mxComponent->dispose();
    mxComponent.clear();

    comphelper::LibreOfficeKit::setActive(false);
}

void SwTiledRenderingTest::testViewLock()
{
    comphelper::LibreOfficeKit::setActive();

    // Load a document that has a shape and create two views.
    SwXTextDocument* pXTextDocument = createDoc("shape.fodt");
    ViewCallback aView1;
    SfxViewShell::Current()->registerLibreOfficeKitViewCallback(&ViewCallback::callback, &aView1);
    SfxLokHelper::createView();
    pXTextDocument->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ViewCallback aView2;
    SfxViewShell::Current()->registerLibreOfficeKitViewCallback(&ViewCallback::callback, &aView2);

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

    mxComponent->dispose();
    mxComponent.clear();

    comphelper::LibreOfficeKit::setActive(false);
}

void SwTiledRenderingTest::testTextEditViewInvalidations()
{
    // Load a document that has a shape and create two views.
    comphelper::LibreOfficeKit::setActive();
    SwXTextDocument* pXTextDocument = createDoc("shape.fodt");
    ViewCallback aView1;
    SfxViewShell::Current()->registerLibreOfficeKitViewCallback(&ViewCallback::callback, &aView1);
    SfxLokHelper::createView();
    pXTextDocument->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ViewCallback aView2;
    SfxViewShell::Current()->registerLibreOfficeKitViewCallback(&ViewCallback::callback, &aView2);

    // Begin text edit in the second view.
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    SdrPage* pPage = pWrtShell->GetDoc()->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pObject = pPage->GetObj(0);
    SdrView* pView = pWrtShell->GetDrawView();
    pWrtShell->GetView().BeginTextEdit(pObject, pView->GetSdrPageView(), pWrtShell->GetWin());
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'x', 0);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 'x', 0);

    // Assert that both views are invalidated when pressing a key while in text edit.
    aView1.m_bTilesInvalidated = false;
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'y', 0);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 'y', 0);
    CPPUNIT_ASSERT(aView1.m_bTilesInvalidated);

    pWrtShell->EndTextEdit();
    mxComponent->dispose();
    mxComponent.clear();
    comphelper::LibreOfficeKit::setActive(false);
}

void SwTiledRenderingTest::testUndoInvalidations()
{
    // Load a document and create two views.
    comphelper::LibreOfficeKit::setActive();
    SwXTextDocument* pXTextDocument = createDoc("dummy.fodt");
    ViewCallback aView1;
    SfxViewShell::Current()->registerLibreOfficeKitViewCallback(&ViewCallback::callback, &aView1);
    int nView1 = SfxLokHelper::getView();
    SfxLokHelper::createView();
    pXTextDocument->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ViewCallback aView2;
    SfxViewShell::Current()->registerLibreOfficeKitViewCallback(&ViewCallback::callback, &aView2);
    SfxLokHelper::setView(nView1);

    // Insert a character the end of the document.
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    pWrtShell->EndDoc();
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'c', 0);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 'c', 0);
    SwShellCursor* pShellCursor = pWrtShell->getShellCursor(false);
    CPPUNIT_ASSERT_EQUAL(OUString("Aaa bbb.c"), pShellCursor->GetPoint()->nNode.GetNode().GetTextNode()->GetText());

    // Undo and assert that both views are invalidated.
    aView1.m_bTilesInvalidated = false;
    aView2.m_bTilesInvalidated = false;
    comphelper::dispatchCommand(".uno:Undo", {});
    Scheduler::ProcessAllPendingEvents();
    CPPUNIT_ASSERT(aView1.m_bTilesInvalidated);
    // Undo was dispatched on the first view, this second view was not invalidated.
    CPPUNIT_ASSERT(aView2.m_bTilesInvalidated);

    mxComponent->dispose();
    mxComponent.clear();
    comphelper::LibreOfficeKit::setActive(false);
}

void SwTiledRenderingTest::testUndoLimiting()
{
    // Load a document and create two views.
    comphelper::LibreOfficeKit::setActive();
    SwXTextDocument* pXTextDocument = createDoc("dummy.fodt");
    SwWrtShell* pWrtShell1 = pXTextDocument->GetDocShell()->GetWrtShell();
    SfxLokHelper::createView();
    pXTextDocument->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());

    // Insert a character the end of the document in the second view.
    SwWrtShell* pWrtShell2 = pXTextDocument->GetDocShell()->GetWrtShell();
    pWrtShell2->EndDoc();
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'c', 0);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 'c', 0);
    SwShellCursor* pShellCursor = pWrtShell2->getShellCursor(false);
    CPPUNIT_ASSERT_EQUAL(OUString("Aaa bbb.c"), pShellCursor->GetPoint()->nNode.GetNode().GetTextNode()->GetText());

    // Assert that the first view can't undo, but the second view can.
    CPPUNIT_ASSERT(!pWrtShell1->GetLastUndoInfo(nullptr, nullptr, &pWrtShell1->GetView()));
    CPPUNIT_ASSERT(pWrtShell2->GetLastUndoInfo(nullptr, nullptr, &pWrtShell2->GetView()));

    comphelper::LibreOfficeKit::setActive(false);
}

void SwTiledRenderingTest::testUndoShapeLimiting()
{
    // Load a document and create a view.
    comphelper::LibreOfficeKit::setActive();
    SwXTextDocument* pXTextDocument = createDoc("shape.fodt");
    SwWrtShell* pWrtShell1 = pXTextDocument->GetDocShell()->GetWrtShell();
    SfxLokHelper::createView();
    pXTextDocument->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    SwWrtShell* pWrtShell2 = pXTextDocument->GetDocShell()->GetWrtShell();

    // Start shape text in the second view.
    SdrPage* pPage = pWrtShell2->GetDoc()->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pObject = pPage->GetObj(0);
    SdrView* pView = pWrtShell2->GetDrawView();
    pWrtShell2->GetView().BeginTextEdit(pObject, pView->GetSdrPageView(), pWrtShell2->GetWin());
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'x', 0);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 'x', 0);

    // Assert that the first view can't and the second view can undo the insertion.
    SwDoc* pDoc = pXTextDocument->GetDocShell()->GetDoc();
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    rUndoManager.SetView(&pWrtShell1->GetView());
    // This was 1: first view could undo the change of the second view.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), rUndoManager.GetUndoActionCount());
    rUndoManager.SetView(&pWrtShell2->GetView());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rUndoManager.GetUndoActionCount());

    pWrtShell2->EndTextEdit();
    rUndoManager.SetView(nullptr);
    comphelper::LibreOfficeKit::setActive(false);
}

void SwTiledRenderingTest::testUndoDispatch()
{
    // Load a document and create two views.
    comphelper::LibreOfficeKit::setActive();
    SwXTextDocument* pXTextDocument = createDoc("dummy.fodt");
    int nView1 = SfxLokHelper::getView();
    SfxLokHelper::createView();
    pXTextDocument->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    int nView2 = SfxLokHelper::getView();

    // Insert a character in the first view.
    SfxLokHelper::setView(nView1);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'c', 0);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 'c', 0);

    // Click before the first word in the second view.
    SfxLokHelper::setView(nView2);
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    SwShellCursor* pShellCursor = pWrtShell->getShellCursor(false);
    Point aStart = pShellCursor->GetSttPos();
    aStart.setX(aStart.getX() - 1000);
    pXTextDocument->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONDOWN, aStart.getX(), aStart.getY(), 1, MOUSE_LEFT, 0);
    pXTextDocument->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONUP, aStart.getX(), aStart.getY(), 1, MOUSE_LEFT, 0);
    uno::Reference<frame::XDesktop2> xDesktop = frame::Desktop::create(comphelper::getProcessComponentContext());
    uno::Reference<frame::XFrame> xFrame2 = xDesktop->getActiveFrame();

    // Now switch back to the first view, and make sure that the active frame is updated.
    SfxLokHelper::setView(nView1);
    uno::Reference<frame::XFrame> xFrame1 = xDesktop->getActiveFrame();
    // This failed: setView() did not update the active frame.
    CPPUNIT_ASSERT(xFrame1 != xFrame2);

    comphelper::LibreOfficeKit::setActive(false);
}

void SwTiledRenderingTest::testUndoRepairDispatch()
{
    // Load a document and create two views.
    comphelper::LibreOfficeKit::setActive();
    SwXTextDocument* pXTextDocument = createDoc("dummy.fodt");
    int nView1 = SfxLokHelper::getView();
    SfxLokHelper::createView();
    pXTextDocument->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    int nView2 = SfxLokHelper::getView();

    // Insert a character in the first view.
    SfxLokHelper::setView(nView1);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'c', 0);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 'c', 0);

    // Assert that by default the second view can't undo the action.
    SfxLokHelper::setView(nView2);
    SwDoc* pDoc = pXTextDocument->GetDocShell()->GetDoc();
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rUndoManager.GetUndoActionCount());
    comphelper::dispatchCommand(".uno:Undo", {});
    Scheduler::ProcessAllPendingEvents();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rUndoManager.GetUndoActionCount());

    // But the same is allowed in repair mode.
    SfxLokHelper::setView(nView2);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rUndoManager.GetUndoActionCount());
    uno::Sequence<beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence(
    {
        {"Repair", uno::makeAny(true)}
    }));
    comphelper::dispatchCommand(".uno:Undo", aPropertyValues);
    Scheduler::ProcessAllPendingEvents();
    // This was 1: repair mode couldn't undo the action, either.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), rUndoManager.GetUndoActionCount());

    comphelper::LibreOfficeKit::setActive(false);
}

void SwTiledRenderingTest::testShapeTextUndoShells()
{
    // Load a document and create a view.
    comphelper::LibreOfficeKit::setActive();
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

    // Make sure that the undo item remembers who created it.
    SwDoc* pDoc = pXTextDocument->GetDocShell()->GetDoc();
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rUndoManager.GetUndoActionCount());
    // This was -1: the view shell id for the undo action wasn't known.
    CPPUNIT_ASSERT_EQUAL(nView1, rUndoManager.GetUndoAction()->GetViewShellId());

    mxComponent->dispose();
    mxComponent.clear();
    comphelper::LibreOfficeKit::setActive(false);
}

void SwTiledRenderingTest::testShapeTextUndoGroupShells()
{
    // Load a document and create a view.
    comphelper::LibreOfficeKit::setActive();
    SwXTextDocument* pXTextDocument = createDoc("shape.fodt");
    ViewCallback aView1;
    SfxViewShell::Current()->registerLibreOfficeKitViewCallback(&ViewCallback::callback, &aView1);
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

    // Make sure that the undo item remembers who created it.
    SwDoc* pDoc = pXTextDocument->GetDocShell()->GetDoc();
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), rUndoManager.GetUndoActionCount());
    // This was -1: the view shell id for the (top) undo list action wasn't known.
    CPPUNIT_ASSERT_EQUAL(nView1, rUndoManager.GetUndoAction()->GetViewShellId());

    // Create an editeng text selection in the first view.
    EditView& rEditView = pView->GetTextEditOutlinerView()->GetEditView();
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'x', 0);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 'x', 0);
    // 0th para, 0th char -> 0th para, 1st char.
    ESelection aWordSelection(0, 0, 0, 1);
    rEditView.SetSelection(aWordSelection);

    // Create a second view, and make sure that the new view sees the same
    // cursor position as the old one.
    SfxLokHelper::createView();
    pXTextDocument->initializeForTiledRendering({});
    ViewCallback aView2;
    aView2.m_aViewCursor = Rectangle();
    aView2.m_bViewSelectionSet = false;
    aView2.m_bViewLock = false;
    SfxViewShell::Current()->registerLibreOfficeKitViewCallback(&ViewCallback::callback, &aView2);
    // Difference was 935 twips, the new view didn't see the editeng cursor of
    // the old one. The new difference should be <1px, but here we deal with twips.
    CPPUNIT_ASSERT(std::abs(aView1.m_aOwnCursor.Top() - aView2.m_aViewCursor.Top()) < 10);
    // This was false, editeng text selection of the first view wasn't noticed
    // by the second view.
    CPPUNIT_ASSERT(aView2.m_bViewSelectionSet);
    // This was false, the new view wasn't aware of the shape text lock created
    // by the old view.
    CPPUNIT_ASSERT(aView2.m_bViewLock);

    mxComponent->dispose();
    mxComponent.clear();
    comphelper::LibreOfficeKit::setActive(false);
}

void SwTiledRenderingTest::testTrackChanges()
{
    // Load a document.
    comphelper::LibreOfficeKit::setActive();
    SwXTextDocument* pXTextDocument = createDoc("dummy.fodt");

    // Turn on trak changes, type "zzz" at the end, and move to the start.
    uno::Reference<beans::XPropertySet> xPropertySet(mxComponent, uno::UNO_QUERY);
    xPropertySet->setPropertyValue("RecordChanges", uno::makeAny(true));
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    pWrtShell->EndDoc();
    pWrtShell->Insert("zzz");
    pWrtShell->SttDoc();

    // Reject the change by index, while the cursor does not cover the tracked change.
    uno::Sequence<beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence(
    {
        {"RejectTrackedChange", uno::makeAny(static_cast<sal_uInt16>(0))}
    }));
    comphelper::dispatchCommand(".uno:RejectTrackedChange", aPropertyValues);
    Scheduler::ProcessAllPendingEvents();

    // Assert that the reject was performed.
    SwShellCursor* pShellCursor = pWrtShell->getShellCursor(false);
    // This was 'Aaa bbb.zzz', the change wasn't rejected.
    CPPUNIT_ASSERT_EQUAL(OUString("Aaa bbb."), pShellCursor->GetPoint()->nNode.GetNode().GetTextNode()->GetText());

    comphelper::LibreOfficeKit::setActive(false);
}

void SwTiledRenderingTest::testTrackChangesCallback()
{
    // Load a document.
    comphelper::LibreOfficeKit::setActive();
    SwXTextDocument* pXTextDocument = createDoc("dummy.fodt");
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    pWrtShell->GetSfxViewShell()->registerLibreOfficeKitViewCallback(&SwTiledRenderingTest::callback, this);

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
    SfxItemSet aSet(pWrtShell->GetDoc()->GetAttrPool(), FN_REDLINE_ACCEPT_DIRECT, FN_REDLINE_ACCEPT_DIRECT);
    SfxVoidItem aItem(FN_REDLINE_ACCEPT_DIRECT);
    aSet.Put(aItem);
    pWrtShell->GetView().GetState(aSet);
    // This failed, LOK_CALLBACK_STATE_CHANGED wasn't sent.
    CPPUNIT_ASSERT_EQUAL(0, m_nTrackedChangeIndex);

    comphelper::LibreOfficeKit::setActive(false);
}

void SwTiledRenderingTest::testRedlineUpdateCallback()
{
    // Load a document.
    comphelper::LibreOfficeKit::setActive();
    SwXTextDocument* pXTextDocument = createDoc("dummy.fodt");
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    pWrtShell->GetSfxViewShell()->registerLibreOfficeKitViewCallback(&SwTiledRenderingTest::callback, this);

    // Turn on track changes, type "xx" and delete the second one.
    uno::Reference<beans::XPropertySet> xPropertySet(mxComponent, uno::UNO_QUERY);
    xPropertySet->setPropertyValue("RecordChanges", uno::makeAny(true));
    pWrtShell->Insert("xx");
    m_nRedlineTableEntryModified = 0;
    pWrtShell->DelLeft();

    // Assert that we get exactly one notification about the redline update.
    // This was 0, as LOK_CALLBACK_REDLINE_TABLE_ENTRY_MODIFIED wasn't sent.
    CPPUNIT_ASSERT_EQUAL(1, m_nRedlineTableEntryModified);

    comphelper::LibreOfficeKit::setActive(false);
}

void SwTiledRenderingTest::testSetViewGraphicSelection()
{
    // Load a document.
    comphelper::LibreOfficeKit::setActive();
    SwXTextDocument* pXTextDocument = createDoc("frame.odt");
    int nView1 = SfxLokHelper::getView();
    ViewCallback aView1;
    SfxViewShell::Current()->registerLibreOfficeKitViewCallback(&ViewCallback::callback, &aView1);
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

    mxComponent->dispose();
    mxComponent.clear();
    comphelper::LibreOfficeKit::setActive(false);
}

void SwTiledRenderingTest::testCreateViewGraphicSelection()
{
    // Load a document.
    comphelper::LibreOfficeKit::setActive();
    SwXTextDocument* pXTextDocument = createDoc("frame.odt");
    ViewCallback aView1;
    SfxViewShell::Current()->registerLibreOfficeKitViewCallback(&ViewCallback::callback, &aView1);

    // Mark the textframe in the first view.
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    SdrPage* pPage = pWrtShell->GetDoc()->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pObject = pPage->GetObj(0);
    SdrView* pView = pWrtShell->GetDrawView();
    aView1.m_bGraphicSelection = true;
    pView->MarkObj(pObject, pView->GetSdrPageView());
    CPPUNIT_ASSERT(aView1.m_bGraphicSelection);

    // Create a second view.
    SfxLokHelper::createView();
    // This was false, creating a second view cleared the selection of the
    // first one.
    CPPUNIT_ASSERT(aView1.m_bGraphicSelection);

    // Make sure that the hidden text cursor isn't visible in the second view, either.
    ViewCallback aView2;
    aView2.m_bViewCursorVisible = true;
    aView2.m_bGraphicViewSelection = false;
    SfxViewShell::Current()->registerLibreOfficeKitViewCallback(&ViewCallback::callback, &aView2);
    // This was true, the second view didn't get the visibility of the text
    // cursor of the first view.
    CPPUNIT_ASSERT(!aView2.m_bViewCursorVisible);
    // This was false, the second view didn't get the graphic selection of the
    // first view.
    CPPUNIT_ASSERT(aView2.m_bGraphicViewSelection);

    mxComponent->dispose();
    mxComponent.clear();
    comphelper::LibreOfficeKit::setActive(false);
}

void SwTiledRenderingTest::testCreateViewTextSelection()
{
    // Load a document.
    comphelper::LibreOfficeKit::setActive();
    SwXTextDocument* pXTextDocument = createDoc("dummy.fodt");
    ViewCallback aView1;
    SfxViewShell::Current()->registerLibreOfficeKitViewCallback(&ViewCallback::callback, &aView1);

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
    aView2.m_bViewSelectionSet = true;
    SfxViewShell::Current()->registerLibreOfficeKitViewCallback(&ViewCallback::callback, &aView2);
    // This failed, the second view didn't get the text selection of the first view.
    CPPUNIT_ASSERT(!aView2.m_aViewSelection.isEmpty());

    mxComponent->dispose();
    mxComponent.clear();
    comphelper::LibreOfficeKit::setActive(false);
}

CPPUNIT_TEST_SUITE_REGISTRATION(SwTiledRenderingTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
