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

#include <com/sun/star/frame/DispatchResultState.hpp>
#include <com/sun/star/frame/XDispatchResultListener.hpp>
#include <swmodeltestbase.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <comphelper/dispatchcommand.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/string.hxx>
#include <comphelper/lok.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdview.hxx>
#include <vcl/svapp.hxx>
#include <vcl/virdev.hxx>
#include <editeng/editview.hxx>
#include <editeng/outliner.hxx>
#include <svl/srchitem.hxx>
#include <svl/slstitm.hxx>
#include <drawdoc.hxx>
#include <ndtxt.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <UndoManager.hxx>
#include <cmdid.h>
#include <sfx2/viewsh.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/lokhelper.hxx>
#include <redline.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <vcl/scheduler.hxx>
#include <vcl/vclevent.hxx>
#include <flddat.hxx>

static char const DATA_DIRECTORY[] = "/sw/qa/extras/tiledrendering/data/";

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
    void testRegisterCallback();
    void testPostKeyEvent();
    void testPostMouseEvent();
    void testSetTextSelection();
    void testGetTextSelection();
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
    void testSplitNodeRedlineCallback();
    void testDeleteNodeRedlineCallback();
    void testVisCursorInvalidation();

    CPPUNIT_TEST_SUITE(SwTiledRenderingTest);
    CPPUNIT_TEST(testRegisterCallback);
    CPPUNIT_TEST(testPostKeyEvent);
    CPPUNIT_TEST(testPostMouseEvent);
    CPPUNIT_TEST(testSetTextSelection);
    CPPUNIT_TEST(testGetTextSelection);
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
    CPPUNIT_TEST(testSplitNodeRedlineCallback);
    CPPUNIT_TEST(testDeleteNodeRedlineCallback);
    CPPUNIT_TEST(testVisCursorInvalidation);
    CPPUNIT_TEST_SUITE_END();

private:
    SwXTextDocument* createDoc(const char* pName = nullptr);
    static void callback(int nType, const char* pPayload, void* pData);
    void callbackImpl(int nType, const char* pPayload);
    tools::Rectangle m_aInvalidation;
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
    if (!pName)
        loadURL("private:factory/swriter", nullptr);
    else
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
    tools::Rectangle aTopLeft(0, 0, 256*15, 256*15); // 1 px = 15 twips, assuming 96 DPI.
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
    Scheduler::ProcessEventsToIdle();
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
    Scheduler::ProcessEventsToIdle();
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
    CPPUNIT_ASSERT(!pXTextDocument->getTextSelection("text/html", aUsedFormat).isEmpty());

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
    comphelper::LibreOfficeKit::setActive();

    SwXTextDocument* pXTextDocument = createDoc("2-pages.odt");
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();

    pXTextDocument->setClientVisibleArea(tools::Rectangle(0, 0, 10000, 4000));
    comphelper::dispatchCommand(".uno:BasicShapes.circle", uno::Sequence<beans::PropertyValue>());

    // check that the shape was inserted in the visible area, not outside
    IDocumentDrawModelAccess &rDrawModelAccess = pWrtShell->GetDoc()->getIDocumentDrawModelAccess();
    SdrPage* pPage = rDrawModelAccess.GetDrawModel()->GetPage(0);
    SdrObject* pObject = pPage->GetObj(0);
    CPPUNIT_ASSERT_EQUAL(tools::Rectangle(2736, 868, 7264, 3132), pObject->GetSnapRect());

    // check that it is in the foreground layer
    CPPUNIT_ASSERT_EQUAL(rDrawModelAccess.GetHeavenId().get(), pObject->GetLayer().get());

    comphelper::LibreOfficeKit::setActive(false);
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
    Scheduler::ProcessEventsToIdle();

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

    ViewCallback()
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
          m_bCalled(false)
    {
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
    Scheduler::ProcessEventsToIdle();
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
    Scheduler::ProcessEventsToIdle();
    // Make sure there is a graphic view selection on the first view.
    CPPUNIT_ASSERT(aView1.m_bGraphicViewSelection);

    // Now destroy the second view.
    SfxLokHelper::destroyView(nView2);
    Scheduler::ProcessEventsToIdle();
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
    Scheduler::ProcessEventsToIdle();

    // Assert that both views are invalidated when pressing a key while in text edit.
    aView1.m_bTilesInvalidated = false;
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'y', 0);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 'y', 0);
    Scheduler::ProcessEventsToIdle();

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
    pWrtShell->EndOfSection();
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'c', 0);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 'c', 0);
    Scheduler::ProcessEventsToIdle();
    // ProcessEventsToIdle resets the view; set it again
    SfxLokHelper::setView(nView1);
    SwShellCursor* pShellCursor = pWrtShell->getShellCursor(false);
    CPPUNIT_ASSERT_EQUAL(OUString("Aaa bbb.c"), pShellCursor->GetPoint()->nNode.GetNode().GetTextNode()->GetText());

    // Undo and assert that both views are invalidated.
    aView1.m_bTilesInvalidated = false;
    aView2.m_bTilesInvalidated = false;
    comphelper::dispatchCommand(".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();
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
    pWrtShell2->EndOfSection();
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'c', 0);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 'c', 0);
    Scheduler::ProcessEventsToIdle();
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
    Scheduler::ProcessEventsToIdle();

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
    Scheduler::ProcessEventsToIdle();

    // Make sure that the undo item remembers who created it.
    SwDoc* pDoc = pXTextDocument->GetDocShell()->GetDoc();
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rUndoManager.GetUndoActionCount());
    // This was -1: the view shell id for the undo action wasn't known.
    CPPUNIT_ASSERT_EQUAL(ViewShellId(nView1), rUndoManager.GetUndoAction()->GetViewShellId());

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
    Scheduler::ProcessEventsToIdle();

    // Make sure that the undo item remembers who created it.
    SwDoc* pDoc = pXTextDocument->GetDocShell()->GetDoc();
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), rUndoManager.GetUndoActionCount());
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
    aView2.m_aViewCursor = tools::Rectangle();
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
    ViewCallback aView;
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    pWrtShell->GetSfxViewShell()->registerLibreOfficeKitViewCallback(&ViewCallback::callback, &aView);
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
    SfxItemSet aSet(pWrtShell->GetDoc()->GetAttrPool(), svl::Items<FN_REDLINE_ACCEPT_DIRECT, FN_REDLINE_ACCEPT_DIRECT>{});
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
    pWrtShell->HideCursor();
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

void SwTiledRenderingTest::testRedlineColors()
{
    // Load a document.
    comphelper::LibreOfficeKit::setActive();
    SwXTextDocument* pXTextDocument = createDoc("dummy.fodt");

    // Turn on track changes, type "zzz" at the end.
    uno::Reference<beans::XPropertySet> xPropertySet(mxComponent, uno::UNO_QUERY);
    xPropertySet->setPropertyValue("RecordChanges", uno::makeAny(true));
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    pWrtShell->EndOfSection();
    pWrtShell->Insert("zzz");

    // Assert that info about exactly one author is returned.
    OUString aInfo = pXTextDocument->getTrackedChangeAuthors();
    std::stringstream aStream(aInfo.toUtf8().getStr());
    boost::property_tree::ptree aTree;
    boost::property_tree::read_json(aStream, aTree);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aTree.get_child("authors").size());

    comphelper::LibreOfficeKit::setActive(false);
}

void SwTiledRenderingTest::testCommentEndTextEdit()
{
    // Create a document, type a character and remember the cursor position.
    comphelper::LibreOfficeKit::setActive();
    SwXTextDocument* pXTextDocument = createDoc();
    ViewCallback aView1;
    SfxViewShell::Current()->registerLibreOfficeKitViewCallback(&ViewCallback::callback, &aView1);
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

    mxComponent->dispose();
    mxComponent.clear();
    comphelper::LibreOfficeKit::setActive(false);
}

void SwTiledRenderingTest::testCursorPosition()
{
    // Load a document and register a callback, should get an own cursor.
    comphelper::LibreOfficeKit::setActive();
    SwXTextDocument* pXTextDocument = createDoc();
    ViewCallback aView1;
    SfxViewShell::Current()->registerLibreOfficeKitViewCallback(&ViewCallback::callback, &aView1);

    // Crete a second view, so the first view gets a collaborative cursor.
    SfxLokHelper::createView();
    pXTextDocument->initializeForTiledRendering({});
    ViewCallback aView2;
    SfxViewShell::Current()->registerLibreOfficeKitViewCallback(&ViewCallback::callback, &aView2);

    // Make sure the two are exactly the same.
    // This failed, own cursor was at '1418, 1418', collaborative cursor was at
    // '1425, 1425', due to pixel alignment.
    CPPUNIT_ASSERT_EQUAL(aView1.m_aOwnCursor.toString(), aView1.m_aViewCursor.toString());

    mxComponent->dispose();
    mxComponent.clear();
    comphelper::LibreOfficeKit::setActive(false);
}

void SwTiledRenderingTest::testPaintCallbacks()
{
    // Test that paintTile() never results in callbacks, which can cause a
    // paint <-> invalidate loop.

    // Load a document and register a callback for the first view.
    comphelper::LibreOfficeKit::setActive();
    SwXTextDocument* pXTextDocument = createDoc();
    ViewCallback aView1;
    SfxViewShell::Current()->registerLibreOfficeKitViewCallback(&ViewCallback::callback, &aView1);

    // Create a second view and paint a tile on that second view.
    SfxLokHelper::createView();
    int nCanvasWidth = 256;
    int nCanvasHeight = 256;
    std::vector<unsigned char> aBuffer(nCanvasWidth * nCanvasHeight * 4);
    ScopedVclPtrInstance<VirtualDevice> pDevice(nullptr, Size(1, 1), DeviceFormat::DEFAULT);
    pDevice->SetOutputSizePixelScaleOffsetAndBuffer(Size(nCanvasWidth, nCanvasHeight), Fraction(1.0), Point(), aBuffer.data());
    // Make sure that painting a tile in the second view doesn't invoke
    // callbacks on the first view.
    aView1.m_bCalled = false;
    pXTextDocument->paintTile(*pDevice, nCanvasWidth, nCanvasHeight, /*nTilePosX=*/0, /*nTilePosY=*/0, /*nTileWidth=*/3840, /*nTileHeight=*/3840);
    CPPUNIT_ASSERT(!aView1.m_bCalled);

    mxComponent->dispose();
    mxComponent.clear();
    comphelper::LibreOfficeKit::setActive(false);
}

void SwTiledRenderingTest::testUndoRepairResult()
{
    // Load a document and create two views.
    comphelper::LibreOfficeKit::setActive();
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

    mxComponent->dispose();
    mxComponent.clear();
    comphelper::LibreOfficeKit::setActive(false);
}

void SwTiledRenderingTest::testRedoRepairResult()
{
    // Load a document and create two views.
    comphelper::LibreOfficeKit::setActive();
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

    mxComponent->dispose();
    mxComponent.clear();
    comphelper::LibreOfficeKit::setActive(false);
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
    comphelper::LibreOfficeKit::setActive();

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

    mxComponent->dispose();
    mxComponent.clear();
    comphelper::LibreOfficeKit::setActive(false);
}

void SwTiledRenderingTest::testAllTrackedChanges()
{
    // Load a document.
    comphelper::LibreOfficeKit::setActive();
    createDoc("dummy.fodt");

    uno::Reference<beans::XPropertySet> xPropSet(mxComponent, uno::UNO_QUERY);
    xPropSet->setPropertyValue("RecordChanges", uno::makeAny(true));

    // view #1
    SwView* pView1 = dynamic_cast<SwView*>(SfxViewShell::Current());
    CPPUNIT_ASSERT(pView1);
    SwWrtShell* pWrtShell1 = pView1->GetWrtShellPtr();

    // view #2
    SfxLokHelper::createView();
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

    comphelper::LibreOfficeKit::setActive(false);
}

void SwTiledRenderingTest::testDocumentRepair()
{
    comphelper::LibreOfficeKit::setActive();

    // Create two views.
    SwXTextDocument* pXTextDocument = createDoc("dummy.fodt");
    // view #1
    SfxViewShell* pView1 = SfxViewShell::Current();

    // view #2
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

    comphelper::LibreOfficeKit::setActive(false);
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
        CPPUNIT_ASSERT_EQUAL(OUString("Default Style"), aSeq[0]);
    }
    else
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aSeq.getLength());
};

}

void SwTiledRenderingTest::testPageHeader()
{
    comphelper::LibreOfficeKit::setActive();

    createDoc("dummy.fodt");
    SfxViewShell* pViewShell = SfxViewShell::Current();
    // Check Page Header State
    checkPageHeaderOrFooter(pViewShell, FN_INSERT_PAGEHEADER, false);
    // Insert Page Header
    {
        SfxStringItem aStyle(FN_INSERT_PAGEHEADER, "Default Style");
        SfxBoolItem aItem(FN_PARAM_1, true);
        pViewShell->GetDispatcher()->ExecuteList(FN_INSERT_PAGEHEADER, SfxCallMode::API | SfxCallMode::SYNCHRON, {&aStyle, &aItem});
    }
    // Check Page Header State
    checkPageHeaderOrFooter(pViewShell, FN_INSERT_PAGEHEADER, true);

    // Remove Page Header
    {
        SfxStringItem aStyle(FN_INSERT_PAGEHEADER, "Default Style");
        SfxBoolItem aItem(FN_PARAM_1, false);
        pViewShell->GetDispatcher()->ExecuteList(FN_INSERT_PAGEHEADER, SfxCallMode::API | SfxCallMode::SYNCHRON, {&aStyle, &aItem});
    }
    // Check Page Header State
    checkPageHeaderOrFooter(pViewShell, FN_INSERT_PAGEHEADER, false);

    mxComponent->dispose();
    mxComponent.clear();
    comphelper::LibreOfficeKit::setActive(false);
}

void SwTiledRenderingTest::testPageFooter()
{
    comphelper::LibreOfficeKit::setActive();

    createDoc("dummy.fodt");
    SfxViewShell* pViewShell = SfxViewShell::Current();
    // Check Page Footer State
    checkPageHeaderOrFooter(pViewShell, FN_INSERT_PAGEFOOTER, false);
    // Insert Page Footer
    {
        SfxStringItem aPageStyle(FN_INSERT_PAGEFOOTER, "Default Style");
        SfxBoolItem aItem(FN_PARAM_1, true);
        pViewShell->GetDispatcher()->ExecuteList(FN_INSERT_PAGEFOOTER, SfxCallMode::API | SfxCallMode::SYNCHRON, {&aPageStyle, &aItem});
    }
    // Check Page Footer State
    checkPageHeaderOrFooter(pViewShell, FN_INSERT_PAGEFOOTER, true);

    // Remove Page Footer
    {
        SfxStringItem aPageStyle(FN_INSERT_PAGEFOOTER, "Default Style");
        SfxBoolItem aItem(FN_PARAM_1, false);
        pViewShell->GetDispatcher()->ExecuteList(FN_INSERT_PAGEFOOTER, SfxCallMode::API | SfxCallMode::SYNCHRON, {&aPageStyle, &aItem});
    }
    // Check Footer State
    checkPageHeaderOrFooter(pViewShell, FN_INSERT_PAGEFOOTER, false);

    mxComponent->dispose();
    mxComponent.clear();
    comphelper::LibreOfficeKit::setActive(false);
}

void SwTiledRenderingTest::testTdf115088()
{
    comphelper::LibreOfficeKit::setActive();

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
    comphelper::LibreOfficeKit::setActive();
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

    comphelper::LibreOfficeKit::setActive(false);
}

void SwTiledRenderingTest::testIMESupport()
{
    comphelper::LibreOfficeKit::setActive();
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
    CPPUNIT_ASSERT_EQUAL(aInputs[aInputs.size() - 1].concat("Aaa bbb."), pShellCursor->GetPoint()->nNode.GetNode().GetTextNode()->GetText());

    comphelper::LibreOfficeKit::setActive(false);
}

void SwTiledRenderingTest::testSplitNodeRedlineCallback()
{
    // Load a document.
    comphelper::LibreOfficeKit::setActive();
    SwXTextDocument* pXTextDocument = createDoc("splitnode_redline_callback.fodt");
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    pWrtShell->GetSfxViewShell()->registerLibreOfficeKitViewCallback(&SwTiledRenderingTest::callback, this);

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

    comphelper::LibreOfficeKit::setActive(false);
}

void SwTiledRenderingTest::testDeleteNodeRedlineCallback()
{
    // Load a document.
    comphelper::LibreOfficeKit::setActive();
    SwXTextDocument* pXTextDocument = createDoc("removenode_redline_callback.fodt");
    SwWrtShell* pWrtShell = pXTextDocument->GetDocShell()->GetWrtShell();
    pWrtShell->GetSfxViewShell()->registerLibreOfficeKitViewCallback(&SwTiledRenderingTest::callback, this);

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

    comphelper::LibreOfficeKit::setActive(false);
}


void SwTiledRenderingTest::testVisCursorInvalidation()
{
    comphelper::LibreOfficeKit::setActive();

    SwXTextDocument* pXTextDocument = createDoc("dummy.fodt");
    ViewCallback aView1;
    SfxViewShell::Current()->registerLibreOfficeKitViewCallback(&ViewCallback::callback, &aView1);
    int nView1 = SfxLokHelper::getView();

    SfxLokHelper::createView();
    int nView2 = SfxLokHelper::getView();
    ViewCallback aView2;
    SfxViewShell::Current()->registerLibreOfficeKitViewCallback(&ViewCallback::callback, &aView2);
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

    // Do the same as before, but set the related compatibility flag first
    SfxLokHelper::setView(nView2);

    comphelper::LibreOfficeKit::setViewIdForVisCursorInvalidation(true);

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

    comphelper::LibreOfficeKit::setViewIdForVisCursorInvalidation(false);

    mxComponent->dispose();
    mxComponent.clear();
    comphelper::LibreOfficeKit::setActive(false);
}

CPPUNIT_TEST_SUITE_REGISTRATION(SwTiledRenderingTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
