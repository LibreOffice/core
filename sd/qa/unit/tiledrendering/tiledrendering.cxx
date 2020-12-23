/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "../sdmodeltestbase.hxx"
#include <app.hrc>
#include <test/bootstrapfixture.hxx>
#include <test/helper/transferable.hxx>
#include <test/xmltesttools.hxx>
#include <boost/property_tree/json_parser.hpp>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <sfx2/lokhelper.hxx>
#include <com/sun/star/frame/Desktop.hpp>
#include <comphelper/dispatchcommand.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/string.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/editids.hrc>
#include <editeng/editobj.hxx>
#include <editeng/editview.hxx>
#include <editeng/numitem.hxx>
#include <editeng/outliner.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/outlobj.hxx>
#include <osl/conditn.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/stritem.hxx>
#include <svl/intitem.hxx>
#include <comphelper/lok.hxx>
#include <svx/svdotable.hxx>
#include <svx/svdoutl.hxx>
#include <unotools/datetime.hxx>
#include <tools/UnitConversion.hxx>

#include <DrawDocShell.hxx>
#include <ViewShellBase.hxx>
#include <ViewShell.hxx>
#include <sdpage.hxx>
#include <unomodel.hxx>
#include <drawdoc.hxx>
#include <undo/undomanager.hxx>
#include <sfx2/request.hxx>
#include <svx/svxids.hrc>
#include <pres.hxx>
#include <navigatr.hxx>
#include <vcl/cursor.hxx>
#include <vcl/scheduler.hxx>
#include <vcl/vclevent.hxx>

#include <chrono>
#include <cstdlib>

using namespace css;

namespace
{
    OUStringLiteral const DATA_DIRECTORY = u"/sd/qa/unit/tiledrendering/data/";
}

static std::ostream& operator<<(std::ostream& os, ViewShellId id)
{
    os << static_cast<sal_Int32>(id);
    return os;
}

class SdTiledRenderingTest : public SdModelTestBase, public XmlTestTools
{
public:
    SdTiledRenderingTest();
    virtual void setUp() override;
    virtual void tearDown() override;

    void testCreateDestroy();
    void testCreateView();
    void testRegisterCallback();
    void testPostKeyEvent();
    void testPostMouseEvent();
    void testSetTextSelection();
    void testGetTextSelection();
    void testSetGraphicSelection();
    void testUndoShells();
    void testResetSelection();
    void testInsertDeletePage();
    void testInsertTable();
    void testPartHash();
    void testResizeTable();
    void testResizeTableColumn();
    void testViewCursors();
    void testViewCursorParts();
    void testCursorViews();
    void testCursorVisibility_SingleClick();
    void testCursorVisibility_DoubleClick();
    void testCursorVisibility_MultiView();
    void testCursorVisibility_Escape();
    void testViewLock();
    void testUndoLimiting();
    void testCreateViewGraphicSelection();
    void testCreateViewTextCursor();
    void testTdf102223();
    void testTdf118354();
    void testPostKeyEventInvalidation();
    void testTdf103083();
    void testTdf104405();
    void testTdf81754();
    void testTdf105502();
    void testCommentCallbacks();
    void testCommentChangeImpress();
    void testCommentChangeDraw();
    void testMultiViewInsertDeletePage();
    void testDisableUndoRepair();
    void testDocumentRepair();
    void testLanguageStatus();
    void testDefaultView();
    void testIMESupport();
    void testTdf115783();
    void testPasteTextOnSlide();
    void testTdf115873();
    void testTdf115873Group();
    void testCutSelectionChange();
    void testRegenerateDiagram();
    void testLanguageAllText();
    void testInsertDeletePageInvalidation();
    void testSpellOnlineRenderParameter();

    CPPUNIT_TEST_SUITE(SdTiledRenderingTest);
    CPPUNIT_TEST(testCreateDestroy);
    CPPUNIT_TEST(testCreateView);
    CPPUNIT_TEST(testRegisterCallback);
    CPPUNIT_TEST(testPostKeyEvent);
    CPPUNIT_TEST(testPostMouseEvent);
    CPPUNIT_TEST(testSetTextSelection);
    CPPUNIT_TEST(testGetTextSelection);
    CPPUNIT_TEST(testSetGraphicSelection);
    CPPUNIT_TEST(testUndoShells);
    CPPUNIT_TEST(testResetSelection);
    CPPUNIT_TEST(testInsertDeletePage);
    CPPUNIT_TEST(testInsertTable);
    CPPUNIT_TEST(testPartHash);
    CPPUNIT_TEST(testResizeTable);
    CPPUNIT_TEST(testResizeTableColumn);
    CPPUNIT_TEST(testViewCursors);
    CPPUNIT_TEST(testViewCursorParts);
    CPPUNIT_TEST(testCursorViews);
    CPPUNIT_TEST(testCursorVisibility_SingleClick);
    CPPUNIT_TEST(testCursorVisibility_DoubleClick);
    CPPUNIT_TEST(testCursorVisibility_MultiView);
    CPPUNIT_TEST(testCursorVisibility_Escape);
    CPPUNIT_TEST(testViewLock);
    CPPUNIT_TEST(testUndoLimiting);
    CPPUNIT_TEST(testCreateViewGraphicSelection);
    CPPUNIT_TEST(testCreateViewTextCursor);
    CPPUNIT_TEST(testTdf102223);
    CPPUNIT_TEST(testTdf118354);
    CPPUNIT_TEST(testPostKeyEventInvalidation);
    CPPUNIT_TEST(testTdf103083);
    CPPUNIT_TEST(testTdf104405);
    CPPUNIT_TEST(testTdf81754);
    CPPUNIT_TEST(testTdf105502);
    CPPUNIT_TEST(testCommentCallbacks);
    CPPUNIT_TEST(testCommentChangeImpress);
    CPPUNIT_TEST(testCommentChangeDraw);
    CPPUNIT_TEST(testMultiViewInsertDeletePage);
    CPPUNIT_TEST(testDisableUndoRepair);
    CPPUNIT_TEST(testDocumentRepair);
    CPPUNIT_TEST(testLanguageStatus);
    CPPUNIT_TEST(testDefaultView);
    CPPUNIT_TEST(testIMESupport);
    CPPUNIT_TEST(testTdf115783);
    CPPUNIT_TEST(testPasteTextOnSlide);
    CPPUNIT_TEST(testTdf115873);
    CPPUNIT_TEST(testTdf115873Group);
    CPPUNIT_TEST(testCutSelectionChange);
    CPPUNIT_TEST(testRegenerateDiagram);
    CPPUNIT_TEST(testLanguageAllText);
    CPPUNIT_TEST(testInsertDeletePageInvalidation);
    CPPUNIT_TEST(testSpellOnlineRenderParameter);

    CPPUNIT_TEST_SUITE_END();

private:
    SdXImpressDocument* createDoc(const char* pName, const uno::Sequence<beans::PropertyValue>& rArguments = uno::Sequence<beans::PropertyValue>());
    static void callback(int nType, const char* pPayload, void* pData);
    void callbackImpl(int nType, const char* pPayload);
    xmlDocUniquePtr parseXmlDump();

    uno::Reference<lang::XComponent> mxComponent;
    ::tools::Rectangle m_aInvalidation;
    std::vector<::tools::Rectangle> m_aSelection;
    bool m_bFound;
    sal_Int32 m_nPart;
    std::vector<OString> m_aSearchResultSelection;
    std::vector<int> m_aSearchResultPart;
    int m_nSelectionBeforeSearchResult;
    int m_nSelectionAfterSearchResult;

    /// For document size changed callback.
    osl::Condition m_aDocumentSizeCondition;
    xmlBufferPtr m_pXmlBuffer;
};

SdTiledRenderingTest::SdTiledRenderingTest()
    : m_bFound(true),
      m_nPart(0),
      m_nSelectionBeforeSearchResult(0),
      m_nSelectionAfterSearchResult(0),
      m_pXmlBuffer(nullptr)
{
}

void SdTiledRenderingTest::setUp()
{
    test::BootstrapFixture::setUp();

    // prevent showing warning message box
    setenv("OOX_NO_SMARTART_WARNING", "1", 1);
    comphelper::LibreOfficeKit::setActive(true);

    mxDesktop.set(css::frame::Desktop::create(comphelper::getComponentContext(getMultiServiceFactory())));
}

void SdTiledRenderingTest::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    if (m_pXmlBuffer)
        xmlBufferFree(m_pXmlBuffer);

    comphelper::LibreOfficeKit::setActive(false);

    test::BootstrapFixture::tearDown();
}

SdXImpressDocument* SdTiledRenderingTest::createDoc(const char* pName, const uno::Sequence<beans::PropertyValue>& rArguments)
{
    if (mxComponent.is())
        mxComponent->dispose();
    mxComponent = loadFromDesktop(m_directories.getURLFromSrc(DATA_DIRECTORY) + OUString::createFromAscii(pName), "com.sun.star.presentation.PresentationDocument");
    SdXImpressDocument* pImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pImpressDocument);
    pImpressDocument->initializeForTiledRendering(rArguments);
    return pImpressDocument;
}

void SdTiledRenderingTest::callback(int nType, const char* pPayload, void* pData)
{
    static_cast<SdTiledRenderingTest*>(pData)->callbackImpl(nType, pPayload);
}

namespace
{

std::vector<OUString> lcl_convertSeparated(const OUString& rString, sal_Unicode nSeparator)
{
    std::vector<OUString> aRet;

    sal_Int32 nIndex = 0;
    do
    {
        OUString aToken = rString.getToken(0, nSeparator, nIndex);
        aToken = aToken.trim();
        if (!aToken.isEmpty())
            aRet.push_back(aToken);
    }
    while (nIndex >= 0);

    return aRet;
}

void lcl_convertRectangle(const OUString& rString, ::tools::Rectangle& rRectangle)
{
    uno::Sequence<OUString> aSeq = comphelper::string::convertCommaSeparated(rString);
    CPPUNIT_ASSERT(aSeq.getLength() == 4 || aSeq.getLength() == 5);
    rRectangle.setX(aSeq[0].toInt32());
    rRectangle.setY(aSeq[1].toInt32());
    rRectangle.setWidth(aSeq[2].toInt32());
    rRectangle.setHeight(aSeq[3].toInt32());
}

} // end anonymous namespace

void SdTiledRenderingTest::callbackImpl(int nType, const char* pPayload)
{
    switch (nType)
    {
    case LOK_CALLBACK_INVALIDATE_TILES:
    {
        OUString aPayload = OUString::createFromAscii(pPayload);
        if (aPayload != "EMPTY" && m_aInvalidation.IsEmpty())
            lcl_convertRectangle(aPayload, m_aInvalidation);
    }
    break;
    case LOK_CALLBACK_TEXT_SELECTION:
    {
        OUString aPayload = OUString::createFromAscii(pPayload);
        m_aSelection.clear();
        for (const OUString& rString : lcl_convertSeparated(aPayload, u';'))
        {
            ::tools::Rectangle aRectangle;
            lcl_convertRectangle(rString, aRectangle);
            m_aSelection.push_back(aRectangle);
        }
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
    case LOK_CALLBACK_DOCUMENT_SIZE_CHANGED:
    {
        m_aDocumentSizeCondition.set();
    }
    break;
    case LOK_CALLBACK_SET_PART:
    {
        OUString aPayload = OUString::createFromAscii(pPayload);
        m_nPart = aPayload.toInt32();
    }
    break;
    case LOK_CALLBACK_SEARCH_RESULT_SELECTION:
    {
        m_aSearchResultSelection.clear();
        m_aSearchResultPart.clear();
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
    }
}

xmlDocUniquePtr SdTiledRenderingTest::parseXmlDump()
{
    if (m_pXmlBuffer)
        xmlBufferFree(m_pXmlBuffer);

    // Create the xml writer.
    m_pXmlBuffer = xmlBufferCreate();
    xmlTextWriterPtr pXmlWriter = xmlNewTextWriterMemory(m_pXmlBuffer, 0);
    xmlTextWriterStartDocument(pXmlWriter, nullptr, nullptr, nullptr);

    // Create the dump.
    SdXImpressDocument* pImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pImpressDocument);
    pImpressDocument->GetDoc()->dumpAsXml(pXmlWriter);

    // Delete the xml writer.
    xmlTextWriterEndDocument(pXmlWriter);
    xmlFreeTextWriter(pXmlWriter);

    return xmlDocUniquePtr(xmlParseMemory(reinterpret_cast<const char*>(xmlBufferContent(m_pXmlBuffer)), xmlBufferLength(m_pXmlBuffer)));
}

void SdTiledRenderingTest::testCreateDestroy()
{
    createDoc("dummy.odp");
    // Nothing to do, the tearDown call should cleanup.
}

void SdTiledRenderingTest::testCreateView()
{
    createDoc("dummy.odp");

    SfxLokHelper::createView();
}

void SdTiledRenderingTest::testRegisterCallback()
{
    SdXImpressDocument* pXImpressDocument = createDoc("dummy.odp");
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    pViewShell->GetViewShellBase().registerLibreOfficeKitViewCallback(&SdTiledRenderingTest::callback, this);

    // Start text edit of the empty title shape.
    SdPage* pActualPage = pViewShell->GetActualPage();
    SdrObject* pObject = pActualPage->GetObj(0);
    SdrView* pView = pViewShell->GetView();
    pView->SdrBeginTextEdit(pObject);
    CPPUNIT_ASSERT(pView->GetTextEditObject());

    // Check that the top left 256x256px tile would be invalidated.
    CPPUNIT_ASSERT(!m_aInvalidation.IsEmpty());
    ::tools::Rectangle aTopLeft(0, 0, 256*15, 256*15); // 1 px = 15 twips, assuming 96 DPI.
    CPPUNIT_ASSERT(m_aInvalidation.IsOver(aTopLeft));
}

void SdTiledRenderingTest::testPostKeyEvent()
{
    SdXImpressDocument* pXImpressDocument = createDoc("dummy.odp");
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    SdPage* pActualPage = pViewShell->GetActualPage();
    SdrObject* pObject = pActualPage->GetObj(0);
    CPPUNIT_ASSERT_EQUAL(OBJ_TITLETEXT, pObject->GetObjIdentifier());
    SdrTextObj* pTextObj = static_cast<SdrTextObj*>(pObject);
    SdrView* pView = pViewShell->GetView();
    pView->MarkObj(pTextObj, pView->GetSdrPageView());
    SfxStringItem aInputString(SID_ATTR_CHAR, "x");
    pViewShell->GetViewFrame()->GetDispatcher()->ExecuteList(SID_ATTR_CHAR,
            SfxCallMode::SYNCHRON, { &aInputString });

    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'x', 0);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 'x', 0);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT(pView->GetTextEditObject());
    EditView& rEditView = pView->GetTextEditOutlinerView()->GetEditView();
    // Did we manage to enter a second character?
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), rEditView.GetSelection().nStartPos);
    ESelection aWordSelection(0, 0, 0, 2); // start para, start char, end para, end char.
    rEditView.SetSelection(aWordSelection);
    // Did we enter the expected character?
    CPPUNIT_ASSERT_EQUAL(OUString("xx"), rEditView.GetSelected());
}

void SdTiledRenderingTest::testPostMouseEvent()
{
    SdXImpressDocument* pXImpressDocument = createDoc("dummy.odp");
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    SdPage* pActualPage = pViewShell->GetActualPage();
    SdrObject* pObject = pActualPage->GetObj(0);
    CPPUNIT_ASSERT_EQUAL(OBJ_TITLETEXT, pObject->GetObjIdentifier());
    SdrTextObj* pTextObj = static_cast<SdrTextObj*>(pObject);
    SdrView* pView = pViewShell->GetView();
    pView->MarkObj(pTextObj, pView->GetSdrPageView());
    SfxStringItem aInputString(SID_ATTR_CHAR, "x");
    pViewShell->GetViewFrame()->GetDispatcher()->ExecuteList(SID_ATTR_CHAR,
            SfxCallMode::SYNCHRON, { &aInputString });
    CPPUNIT_ASSERT(pView->GetTextEditObject());
    EditView& rEditView = pView->GetTextEditOutlinerView()->GetEditView();
    // Did we manage to go after the first character?
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), rEditView.GetSelection().nStartPos);

    vcl::Cursor* pCursor = rEditView.GetCursor();
    Point aPosition(pCursor->GetPos().getX(), pCursor->GetPos().getY() + pCursor->GetSize().Height() / 2);
    aPosition.setX(aPosition.getX() - 1000);
    pXImpressDocument->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONDOWN,
                                      convertMm100ToTwip(aPosition.getX()), convertMm100ToTwip(aPosition.getY()),
                                      1, MOUSE_LEFT, 0);
    pXImpressDocument->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONUP,
                                      convertMm100ToTwip(aPosition.getX()), convertMm100ToTwip(aPosition.getY()),
                                      1, MOUSE_LEFT, 0);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(pView->GetTextEditObject());
    // The new cursor position must be before the first word.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), rEditView.GetSelection().nStartPos);
}

void SdTiledRenderingTest::testSetTextSelection()
{
    SdXImpressDocument* pXImpressDocument = createDoc("dummy.odp");
    uno::Reference<container::XIndexAccess> xDrawPage(pXImpressDocument->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    xShape->setString("Aaa bbb.");
    // Create a selection on the second word.
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    SdPage* pActualPage = pViewShell->GetActualPage();
    SdrObject* pObject = pActualPage->GetObj(0);
    SdrView* pView = pViewShell->GetView();
    pView->SdrBeginTextEdit(pObject);
    CPPUNIT_ASSERT(pView->GetTextEditObject());
    EditView& rEditView = pView->GetTextEditOutlinerView()->GetEditView();
    ESelection aWordSelection(0, 4, 0, 7);
    rEditView.SetSelection(aWordSelection);
    // Did we indeed manage to select the second word?
    CPPUNIT_ASSERT_EQUAL(OUString("bbb"), rEditView.GetSelected());

    // Now use setTextSelection() to move the end of the selection 1000 twips right.
    vcl::Cursor* pCursor = rEditView.GetCursor();
    Point aEnd = pCursor->GetPos();
    aEnd.setX(aEnd.getX() + 1000);
    pXImpressDocument->setTextSelection(LOK_SETTEXTSELECTION_END, aEnd.getX(), aEnd.getY());
    // The new selection must include the ending dot, too -- but not the first word.
    CPPUNIT_ASSERT_EQUAL(OUString("bbb."), rEditView.GetSelected());
}

void SdTiledRenderingTest::testGetTextSelection()
{
    SdXImpressDocument* pXImpressDocument = createDoc("dummy.odp");
    uno::Reference<container::XIndexAccess> xDrawPage(pXImpressDocument->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    xShape->setString("Shape");
    // Create a selection on the shape text.
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    SdPage* pActualPage = pViewShell->GetActualPage();
    SdrObject* pObject = pActualPage->GetObj(0);
    SdrView* pView = pViewShell->GetView();
    pView->SdrBeginTextEdit(pObject);
    CPPUNIT_ASSERT(pView->GetTextEditObject());
    EditView& rEditView = pView->GetTextEditOutlinerView()->GetEditView();
    ESelection aWordSelection(0, 0, 0, 5);
    rEditView.SetSelection(aWordSelection);
    // Did we indeed manage to copy the selected text?
    CPPUNIT_ASSERT_EQUAL(OString("Shape"), apitest::helper::transferable::getTextSelection(pXImpressDocument->getSelection(), "text/plain;charset=utf-8"));

    // Make sure returned RTF is not empty.
    CPPUNIT_ASSERT(!apitest::helper::transferable::getTextSelection(pXImpressDocument->getSelection(), "text/rtf").isEmpty());
}

void SdTiledRenderingTest::testSetGraphicSelection()
{
    SdXImpressDocument* pXImpressDocument = createDoc("shape.odp");
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    SdPage* pPage = pViewShell->GetActualPage();
    SdrObject* pObject = pPage->GetObj(0);
    SdrHdlList handleList(nullptr);
    pObject->AddToHdlList(handleList);
    // Make sure the rectangle has 8 handles: at each corner and at the center of each edge.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(8), handleList.GetHdlCount());
    // Take the bottom center one.
    SdrHdl* pHdl = handleList.GetHdl(6);
    CPPUNIT_ASSERT_EQUAL(int(SdrHdlKind::Lower), static_cast<int>(pHdl->GetKind()));
    ::tools::Rectangle aShapeBefore = pObject->GetSnapRect();
    // Resize.
    pXImpressDocument->setGraphicSelection(LOK_SETGRAPHICSELECTION_START, convertMm100ToTwip(pHdl->GetPos().getX()), convertMm100ToTwip(pHdl->GetPos().getY()));
    pXImpressDocument->setGraphicSelection(LOK_SETGRAPHICSELECTION_END, convertMm100ToTwip(pHdl->GetPos().getX()), convertMm100ToTwip(pHdl->GetPos().getY() + 1000));

    // Assert that view shell ID tracking works.
    sal_Int32 nView1 = SfxLokHelper::getView();
    SdDrawDocument* pDocument = pXImpressDocument->GetDoc();
    sd::UndoManager* pUndoManager = pDocument->GetUndoManager();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pUndoManager->GetUndoActionCount());
    auto pListAction = dynamic_cast<SfxListUndoAction*>(pUndoManager->GetUndoAction());
    CPPUNIT_ASSERT(pListAction);
    for (size_t i = 0; i < pListAction->maUndoActions.size(); ++i)
        // The second item was -1 here, view shell ID wasn't known.
        CPPUNIT_ASSERT_EQUAL(ViewShellId(nView1), pListAction->GetUndoAction(i)->GetViewShellId());

    ::tools::Rectangle aShapeAfter = pObject->GetSnapRect();
    // Check that a resize happened, but aspect ratio is not kept.
    CPPUNIT_ASSERT_EQUAL(aShapeBefore.getWidth(), aShapeAfter.getWidth());
    CPPUNIT_ASSERT(aShapeBefore.getHeight() < aShapeAfter.getHeight());
}

void SdTiledRenderingTest::testUndoShells()
{
    // Load a document and set the page size.
    SdXImpressDocument* pXImpressDocument = createDoc("shape.odp");
    uno::Sequence<beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence(
    {
        {"AttributePageSize.Width", uno::makeAny(static_cast<sal_Int32>(10000))},
        {"AttributePageSize.Height", uno::makeAny(static_cast<sal_Int32>(10000))},
    }));
    comphelper::dispatchCommand(".uno:AttributePageSize", aPropertyValues);
    Scheduler::ProcessEventsToIdle();

    // Assert that view shell ID tracking works for SdUndoAction subclasses.
    SdDrawDocument* pDocument = pXImpressDocument->GetDoc();
    sd::UndoManager* pUndoManager = pDocument->GetUndoManager();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pUndoManager->GetUndoActionCount());
    sal_Int32 nView1 = SfxLokHelper::getView();
    // This was -1, SdUndoGroup did not track what view shell created it.
    CPPUNIT_ASSERT_EQUAL(ViewShellId(nView1), pUndoManager->GetUndoAction()->GetViewShellId());
}

void SdTiledRenderingTest::testResetSelection()
{
    SdXImpressDocument* pXImpressDocument = createDoc("dummy.odp");
    uno::Reference<container::XIndexAccess> xDrawPage(pXImpressDocument->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    xShape->setString("Aaa bbb.");
    // Create a selection on the second word.
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    SdPage* pActualPage = pViewShell->GetActualPage();
    SdrObject* pObject = pActualPage->GetObj(0);
    SdrView* pView = pViewShell->GetView();
    pView->SdrBeginTextEdit(pObject);
    CPPUNIT_ASSERT(pView->GetTextEditObject());
    EditView& rEditView = pView->GetTextEditOutlinerView()->GetEditView();
    ESelection aWordSelection(0, 4, 0, 7);
    rEditView.SetSelection(aWordSelection);
    // Did we indeed manage to select the second word?
    CPPUNIT_ASSERT_EQUAL(OUString("bbb"), rEditView.GetSelected());

    // Now use resetSelection() to reset the selection.
    pXImpressDocument->resetSelection();
    CPPUNIT_ASSERT(!pView->GetTextEditObject());
}

namespace
{

std::vector<OUString> getCurrentParts(SdXImpressDocument* pDocument)
{
    int parts = pDocument->getParts();
    std::vector<OUString> result;

    result.reserve(parts);
    for (int i = 0; i < parts; i++)
    {
        result.push_back(pDocument->getPartName(i));
    }

    return result;
}

}

void SdTiledRenderingTest::testInsertDeletePage()
{
    SdXImpressDocument* pXImpressDocument = createDoc("insert-delete.odp");
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    pViewShell->GetViewShellBase().registerLibreOfficeKitViewCallback(&SdTiledRenderingTest::callback, this);

    SdDrawDocument* pDoc = pXImpressDocument->GetDocShell()->GetDoc();
    CPPUNIT_ASSERT(pDoc);

    std::vector<OUString> aInserted =
    {
        "Slide 1", "Slide 2", "Slide 3", "Slide 4", "Slide 5",
        "Slide 6", "Slide 7", "Slide 8", "Slide 9", "Slide 10", "Slide 11"
    };

    std::vector<OUString> aDeleted =
    {
        "Slide 1"
    };

    // the document has 1 slide
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(1), pDoc->GetSdPageCount(PageKind::Standard));

    uno::Sequence<beans::PropertyValue> aArgs;

    // Insert slides
    m_aDocumentSizeCondition.reset();
    for (unsigned it = 1; it <= 10; it++)
        comphelper::dispatchCommand(".uno:InsertPage", aArgs);

    osl::Condition::Result aResult = m_aDocumentSizeCondition.wait(std::chrono::seconds(2));
    CPPUNIT_ASSERT_EQUAL(osl::Condition::result_ok, aResult);

    // Verify inserted slides
    std::vector<OUString> aPageList(getCurrentParts(pXImpressDocument));
    CPPUNIT_ASSERT_EQUAL(aPageList.size(), aInserted.size());

    for (auto it1 = aPageList.begin(), it2 = aInserted.begin(); it1 != aPageList.end(); ++it1, ++it2)
    {
        CPPUNIT_ASSERT_EQUAL(*it1, *it2);
    }

    // Delete slides
    m_aDocumentSizeCondition.reset();
    for (unsigned it = 1; it <= 10; it++)
        comphelper::dispatchCommand(".uno:DeletePage", aArgs);

    aResult = m_aDocumentSizeCondition.wait(std::chrono::seconds(2));
    CPPUNIT_ASSERT_EQUAL(osl::Condition::result_ok, aResult);

    // Verify deleted slides
    aPageList = getCurrentParts(pXImpressDocument);
    CPPUNIT_ASSERT_EQUAL(aPageList.size(), aDeleted.size());
    for (auto it1 = aPageList.begin(), it2 = aDeleted.begin(); it1 != aPageList.end(); ++it1, ++it2)
    {
        CPPUNIT_ASSERT_EQUAL(*it1, *it2);
    }

    // Undo deleted slides
    m_aDocumentSizeCondition.reset();
    for (unsigned it = 1; it <= 10; it++)
        comphelper::dispatchCommand(".uno:Undo", aArgs);

    aResult = m_aDocumentSizeCondition.wait(std::chrono::seconds(2));
    CPPUNIT_ASSERT_EQUAL(osl::Condition::result_ok, aResult);

    // Verify inserted slides
    aPageList = getCurrentParts(pXImpressDocument);
    CPPUNIT_ASSERT_EQUAL(aPageList.size(), aInserted.size());
    for (auto it1 = aPageList.begin(), it2 = aInserted.begin(); it1 != aPageList.end(); ++it1, ++it2)
    {
        CPPUNIT_ASSERT_EQUAL(*it1, *it2);
    }

    // Redo deleted slides
    m_aDocumentSizeCondition.reset();
    for (unsigned it = 1; it <= 10; it++)
        comphelper::dispatchCommand(".uno:Redo", aArgs);

    aResult = m_aDocumentSizeCondition.wait(std::chrono::seconds(2));
    CPPUNIT_ASSERT_EQUAL(osl::Condition::result_ok, aResult);

    // Verify deleted slides
    aPageList = getCurrentParts(pXImpressDocument);
    CPPUNIT_ASSERT_EQUAL(aPageList.size(), aDeleted.size());
    for (auto it1 = aPageList.begin(), it2 = aDeleted.begin(); it1 != aPageList.end(); ++it1, ++it2)
    {
        CPPUNIT_ASSERT_EQUAL(*it1, *it2);
    }

    // the document has 1 slide
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(1), pDoc->GetSdPageCount(PageKind::Standard));
}

void SdTiledRenderingTest::testInsertTable()
{
    SdXImpressDocument* pXImpressDocument = createDoc("dummy.odp");

    uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence(
    {
        { "Rows", uno::makeAny(sal_Int32(3)) },
        { "Columns", uno::makeAny(sal_Int32(5)) }
    }));

    comphelper::dispatchCommand(".uno:InsertTable", aArgs);
    Scheduler::ProcessEventsToIdle();

    // get the table
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    SdPage* pActualPage = pViewShell->GetActualPage();
    SdrObject* pObject = pActualPage->GetObj(1);
    CPPUNIT_ASSERT(pObject);

    // check that the table is not in the top left corner
    Point aPos(pObject->GetRelativePos());

    CPPUNIT_ASSERT(aPos.X() != 0);
    CPPUNIT_ASSERT(aPos.Y() != 0);
}

void SdTiledRenderingTest::testPartHash()
{
    SdXImpressDocument* pDoc = createDoc("dummy.odp");

    int nParts = pDoc->getParts();
    for (int it = 0; it < nParts; it++)
    {
        CPPUNIT_ASSERT(!pDoc->getPartHash(it).isEmpty());
    }

    // check part that it does not exists
    CPPUNIT_ASSERT(pDoc->getPartHash(100).isEmpty());
}

void SdTiledRenderingTest::testResizeTable()
{
    // Load the document.
    SdXImpressDocument* pXImpressDocument = createDoc("table.odp");
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    SdPage* pActualPage = pViewShell->GetActualPage();
    SdrObject* pObject = pActualPage->GetObj(0);
    auto pTableObject = dynamic_cast<sdr::table::SdrTableObj*>(pObject);
    CPPUNIT_ASSERT(pTableObject);

    // Select the table by marking it + starting and ending text edit.
    SdrView* pView = pViewShell->GetView();
    pView->MarkObj(pObject, pView->GetSdrPageView());
    pView->SdrBeginTextEdit(pObject);
    pView->SdrEndTextEdit();

    // Remember the original row heights.
    uno::Reference<table::XColumnRowRange> xTable = pTableObject->getTable();
    uno::Reference<container::XIndexAccess> xRows = xTable->getRows();
    uno::Reference<beans::XPropertySet> xRow1(xRows->getByIndex(0), uno::UNO_QUERY);
    sal_Int32 nExpectedRow1 = xRow1->getPropertyValue("Size").get<sal_Int32>();
    uno::Reference<beans::XPropertySet> xRow2(xRows->getByIndex(1), uno::UNO_QUERY);
    sal_Int32 nExpectedRow2 = xRow2->getPropertyValue("Size").get<sal_Int32>();

    // Resize the upper row, decrease its height by 1 cm.
    Point aInnerRowEdge = pObject->GetSnapRect().Center();
    pXImpressDocument->setGraphicSelection(LOK_SETGRAPHICSELECTION_START, convertMm100ToTwip(aInnerRowEdge.getX()), convertMm100ToTwip(aInnerRowEdge.getY()));
    pXImpressDocument->setGraphicSelection(LOK_SETGRAPHICSELECTION_END, convertMm100ToTwip(aInnerRowEdge.getX()), convertMm100ToTwip(aInnerRowEdge.getY() - 1000));

    // Remember the resized row heights.
    sal_Int32 nResizedRow1 = xRow1->getPropertyValue("Size").get<sal_Int32>();
    CPPUNIT_ASSERT(nResizedRow1 < nExpectedRow1);
    sal_Int32 nResizedRow2 = xRow2->getPropertyValue("Size").get<sal_Int32>();
    CPPUNIT_ASSERT_EQUAL(nExpectedRow2, nResizedRow2);

    // Now undo the resize.
    pXImpressDocument->GetDocShell()->GetUndoManager()->Undo();

    // Check the undo result.
    sal_Int32 nActualRow1 = xRow1->getPropertyValue("Size").get<sal_Int32>();
    CPPUNIT_ASSERT_EQUAL(nExpectedRow1, nActualRow1);
    sal_Int32 nActualRow2 = xRow2->getPropertyValue("Size").get<sal_Int32>();
    // Expected was 4000, actual was 4572, i.e. the second row after undo was larger than expected.
    CPPUNIT_ASSERT_EQUAL(nExpectedRow2, nActualRow2);
}

void SdTiledRenderingTest::testResizeTableColumn()
{
    // Load the document.
    SdXImpressDocument* pXImpressDocument = createDoc("table-column.odp");
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    SdPage* pActualPage = pViewShell->GetActualPage();
    SdrObject* pObject = pActualPage->GetObj(0);
    auto pTableObject = dynamic_cast<sdr::table::SdrTableObj*>(pObject);
    CPPUNIT_ASSERT(pTableObject);

    // Select the table by marking it + starting and ending text edit.
    SdrView* pView = pViewShell->GetView();
    pView->MarkObj(pObject, pView->GetSdrPageView());
    pView->SdrBeginTextEdit(pObject);
    pView->SdrEndTextEdit();

    // Remember the original cell widths.
    xmlDocUniquePtr pXmlDoc = parseXmlDump();
    OString aPrefix = "/SdDrawDocument/SdrModel/SdPage/SdrObjList/SdrTableObj/SdrTableObjImpl/TableLayouter/columns/";
    sal_Int32 nExpectedColumn1 = getXPath(pXmlDoc, aPrefix + "TableLayouter_Layout[1]", "size").toInt32();
    sal_Int32 nExpectedColumn2 = getXPath(pXmlDoc, aPrefix + "TableLayouter_Layout[2]", "size").toInt32();
    pXmlDoc = nullptr;

    // Resize the left column, decrease its width by 1 cm.
    Point aInnerRowEdge = pObject->GetSnapRect().Center();
    pXImpressDocument->setGraphicSelection(LOK_SETGRAPHICSELECTION_START, convertMm100ToTwip(aInnerRowEdge.getX()), convertMm100ToTwip(aInnerRowEdge.getY()));
    pXImpressDocument->setGraphicSelection(LOK_SETGRAPHICSELECTION_END, convertMm100ToTwip(aInnerRowEdge.getX() - 1000), convertMm100ToTwip(aInnerRowEdge.getY()));

    // Remember the resized column widths.
    pXmlDoc = parseXmlDump();
    sal_Int32 nResizedColumn1 = getXPath(pXmlDoc, aPrefix + "TableLayouter_Layout[1]", "size").toInt32();
    CPPUNIT_ASSERT(nResizedColumn1 < nExpectedColumn1);
    sal_Int32 nResizedColumn2 = getXPath(pXmlDoc, aPrefix + "TableLayouter_Layout[2]", "size").toInt32();
    CPPUNIT_ASSERT(nResizedColumn2 > nExpectedColumn2);
    pXmlDoc = nullptr;

    // Now undo the resize.
    pXImpressDocument->GetDocShell()->GetUndoManager()->Undo();

    // Check the undo result.
    pXmlDoc = parseXmlDump();
    sal_Int32 nActualColumn1 = getXPath(pXmlDoc, aPrefix + "TableLayouter_Layout[1]", "size").toInt32();
    // Expected was 7049, actual was 6048, i.e. the first column width after undo was 1cm smaller than expected.
    CPPUNIT_ASSERT_EQUAL(nExpectedColumn1, nActualColumn1);
    sal_Int32 nActualColumn2 = getXPath(pXmlDoc, aPrefix + "TableLayouter_Layout[2]", "size").toInt32();
    CPPUNIT_ASSERT_EQUAL(nExpectedColumn2, nActualColumn2);
    pXmlDoc = nullptr;
}

namespace {

/// A view callback tracks callbacks invoked on one specific view.
class ViewCallback final
{
    SfxViewShell* mpViewShell;
    int mnView;
public:
    bool m_bGraphicSelectionInvalidated;
    bool m_bGraphicViewSelectionInvalidated;
    /// Our current part, to be able to decide if a view cursor/selection is relevant for us.
    int m_nPart;
    bool m_bCursorVisibleChanged;
    bool m_bCursorVisible;
    bool m_bViewLock;
    bool m_bTilesInvalidated;
    std::vector<tools::Rectangle> m_aInvalidations;
    std::map<int, bool> m_aViewCursorInvalidations;
    std::map<int, bool> m_aViewCursorVisibilities;
    bool m_bViewSelectionSet;
    boost::property_tree::ptree m_aCommentCallbackResult;

    ViewCallback()
        : m_bGraphicSelectionInvalidated(false),
          m_bGraphicViewSelectionInvalidated(false),
          m_nPart(0),
          m_bCursorVisibleChanged(false),
          m_bCursorVisible(false),
          m_bViewLock(false),
          m_bTilesInvalidated(false),
          m_bViewSelectionSet(false)
    {
        mpViewShell = SfxViewShell::Current();
        mpViewShell->registerLibreOfficeKitViewCallback(&ViewCallback::callback, this);
        mnView = SfxLokHelper::getView();
    }

    ~ViewCallback()
    {
        SfxLokHelper::setView(mnView);
        mpViewShell->registerLibreOfficeKitViewCallback(nullptr, nullptr);
    }

    static void callback(int nType, const char* pPayload, void* pData)
    {
        static_cast<ViewCallback*>(pData)->callbackImpl(nType, pPayload);
    }

    void callbackImpl(int nType, const char* pPayload)
    {
        switch (nType)
        {
        case LOK_CALLBACK_INVALIDATE_TILES:
        {
            m_bTilesInvalidated = true;
            OString text(pPayload);
            if (!text.startsWith("EMPTY"))
            {
                uno::Sequence<OUString> aSeq = comphelper::string::convertCommaSeparated(OUString::createFromAscii(pPayload));
                CPPUNIT_ASSERT(aSeq.getLength() == 4 || aSeq.getLength() == 5);
                tools::Rectangle aInvalidationRect;
                aInvalidationRect.setX(aSeq[0].toInt32());
                aInvalidationRect.setY(aSeq[1].toInt32());
                aInvalidationRect.setWidth(aSeq[2].toInt32());
                aInvalidationRect.setHeight(aSeq[3].toInt32());
                m_aInvalidations.push_back(aInvalidationRect);
            }
        }
        break;
        case LOK_CALLBACK_GRAPHIC_SELECTION:
        {
            m_bGraphicSelectionInvalidated = true;
        }
        break;
        case LOK_CALLBACK_GRAPHIC_VIEW_SELECTION:
        {
            std::stringstream aStream(pPayload);
            boost::property_tree::ptree aTree;
            boost::property_tree::read_json(aStream, aTree);
            if (aTree.get_child("part").get_value<int>() == m_nPart)
                // Ignore callbacks which are for a different part.
                m_bGraphicViewSelectionInvalidated = true;
        }
        break;
        case LOK_CALLBACK_CURSOR_VISIBLE:
        {
            m_bCursorVisibleChanged = true;
            m_bCursorVisible = (OString("true") == pPayload);
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
        case LOK_CALLBACK_INVALIDATE_VIEW_CURSOR:
        {
            std::stringstream aStream(pPayload);
            boost::property_tree::ptree aTree;
            boost::property_tree::read_json(aStream, aTree);
            int nViewId = aTree.get_child("viewId").get_value<int>();
            m_aViewCursorInvalidations[nViewId] = true;
        }
        break;
        case LOK_CALLBACK_VIEW_CURSOR_VISIBLE:
        {
            std::stringstream aStream(pPayload);
            boost::property_tree::ptree aTree;
            boost::property_tree::read_json(aStream, aTree);
            const int nViewId = aTree.get_child("viewId").get_value<int>();
            m_aViewCursorVisibilities[nViewId] = OString("true") == pPayload;
        }
        break;
        case LOK_CALLBACK_TEXT_VIEW_SELECTION:
        {
            m_bViewSelectionSet = true;
        }
        break;
        case LOK_CALLBACK_COMMENT:
        {
            m_aCommentCallbackResult.clear();
            std::stringstream aStream(pPayload);
            boost::property_tree::read_json(aStream, m_aCommentCallbackResult);
            m_aCommentCallbackResult = m_aCommentCallbackResult.get_child("comment");
        }
        break;
        }
    }
};

}

void SdTiledRenderingTest::testViewCursors()
{
    // Create two views.
    SdXImpressDocument* pXImpressDocument = createDoc("shape.odp");
    ViewCallback aView1;
    SfxLokHelper::createView();
    ViewCallback aView2;

    // Select the shape in the second view.
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    SdPage* pActualPage = pViewShell->GetActualPage();
    SdrObject* pObject = pActualPage->GetObj(0);
    SdrView* pView = pViewShell->GetView();
    pView->MarkObj(pObject, pView->GetSdrPageView());
    Scheduler::ProcessEventsToIdle();

    // First view notices that there was a selection change in the other view.
    CPPUNIT_ASSERT(aView1.m_bGraphicViewSelectionInvalidated);
    // Second view notices that there was a selection change in its own view.
    CPPUNIT_ASSERT(aView2.m_bGraphicSelectionInvalidated);
}

void SdTiledRenderingTest::testViewCursorParts()
{
    // Create two views.
    SdXImpressDocument* pXImpressDocument = createDoc("shape.odp");
    ViewCallback aView1;
    SfxLokHelper::createView();
    pXImpressDocument->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ViewCallback aView2;

    // Select the shape in the second view.
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    SdPage* pActualPage = pViewShell->GetActualPage();
    SdrObject* pObject = pActualPage->GetObj(0);
    SdrView* pView = pViewShell->GetView();
    pView->MarkObj(pObject, pView->GetSdrPageView());
    Scheduler::ProcessEventsToIdle();
    // First view notices that there was a selection change in the other view.
    CPPUNIT_ASSERT(aView1.m_bGraphicViewSelectionInvalidated);
    pView->UnmarkAllObj(pView->GetSdrPageView());

    // Now switch to the second part in the second view.
    pXImpressDocument->setPart(1);
    aView2.m_nPart = 1;
    aView1.m_bGraphicViewSelectionInvalidated = false;
    pActualPage = pViewShell->GetActualPage();
    pObject = pActualPage->GetObj(0);
    pView->MarkObj(pObject, pView->GetSdrPageView());
    Scheduler::ProcessEventsToIdle();
    // First view ignores view selection, as it would be for part 1, and it's in part 0.
    // This failed when the "part" was always 0 in the callback.
    CPPUNIT_ASSERT(!aView1.m_bGraphicViewSelectionInvalidated);
}

void SdTiledRenderingTest::testCursorViews()
{
    // Create the first view.
    SdXImpressDocument* pXImpressDocument = createDoc("title-shape.odp");
    int nView1 = SfxLokHelper::getView();
    ViewCallback aView1;

    // Begin text edit on the only object on the slide.
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    SdrView* pView = pViewShell->GetView();
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::TAB);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::TAB);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'x', 0);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 'x', 0);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(pView->IsTextEdit());

    // Make sure that cursor state is not changed just because we create a second view.
    aView1.m_bCursorVisibleChanged = false;
    SfxLokHelper::createView();
    pXImpressDocument->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(!aView1.m_bCursorVisibleChanged);

    // Make sure that typing in the first view causes an invalidation in the
    // second view as well, even if the second view was created after begin
    // text edit in the first view.
    ViewCallback aView2;
    // This failed: the second view didn't get a lock notification, even if the
    // first view already started text edit.
    CPPUNIT_ASSERT(aView2.m_bViewLock);
    SfxLokHelper::setView(nView1);
    aView2.m_bTilesInvalidated = false;
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'x', 0);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 'x', 0);
    Scheduler::ProcessEventsToIdle();
    // This failed: the second view was not invalidated when pressing a key in
    // the first view.
    CPPUNIT_ASSERT(aView2.m_bTilesInvalidated);
}

void SdTiledRenderingTest::testCursorVisibility_SingleClick()
{
    // Single-clicking in a text box enters editing only
    // when it's on the text, even if it's the default text.

    // Load doc.
    SdXImpressDocument* pXImpressDocument = createDoc("dummy.odp");
    ViewCallback aView1;

    // Begin text edit on the only object on the slide.
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    SdPage* pActualPage = pViewShell->GetActualPage();
    SdrObject* pObject1 = pActualPage->GetObj(0);
    CPPUNIT_ASSERT(pObject1 != nullptr);
    CPPUNIT_ASSERT_EQUAL(OBJ_TITLETEXT, pObject1->GetObjIdentifier());
    SdrTextObj* pTextObject = static_cast<SdrTextObj*>(pObject1);

    // Click once outside of the text (in the first quartile) => no editing.
    const ::tools::Rectangle aRect = pTextObject->GetCurrentBoundRect();
    const auto cornerX = convertMm100ToTwip(aRect.getX() + (aRect.getWidth() / 4));
    const auto cornerY = convertMm100ToTwip(aRect.getY() + (aRect.getHeight() / 4));
    pXImpressDocument->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONDOWN,
                                      cornerX, cornerY,
                                      1, MOUSE_LEFT, 0);
    pXImpressDocument->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONUP,
                                      cornerX, cornerY,
                                      1, MOUSE_LEFT, 0);
    Scheduler::ProcessEventsToIdle();

    // No editing.
    CPPUNIT_ASSERT(!pViewShell->GetView()->IsTextEdit());
    CPPUNIT_ASSERT(!aView1.m_bCursorVisible);

    // Click again, now on the text, in the center, to start editing.
    const auto centerX = convertMm100ToTwip(aRect.getX() + (aRect.getWidth() / 2));
    const auto centerY = convertMm100ToTwip(aRect.getY() + (aRect.getHeight() / 2));
    pXImpressDocument->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONDOWN,
                                      centerX, centerY,
                                      1, MOUSE_LEFT, 0);
    pXImpressDocument->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONUP,
                                      centerX, centerY,
                                      1, MOUSE_LEFT, 0);
    Scheduler::ProcessEventsToIdle();

    // We must be in text editing mode and have cursor visible.
    CPPUNIT_ASSERT(pViewShell->GetView()->IsTextEdit());
    CPPUNIT_ASSERT(aView1.m_bCursorVisible);
}


void SdTiledRenderingTest::testCursorVisibility_DoubleClick()
{
    // Double-clicking anywhere in the TextBox should start editing.

    // Create the first view.
    SdXImpressDocument* pXImpressDocument = createDoc("dummy.odp");
    ViewCallback aView1;

    // Begin text edit on the only object on the slide.
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    SdPage* pActualPage = pViewShell->GetActualPage();
    SdrObject* pObject1 = pActualPage->GetObj(0);
    CPPUNIT_ASSERT(pObject1 != nullptr);
    CPPUNIT_ASSERT_EQUAL(OBJ_TITLETEXT, pObject1->GetObjIdentifier());
    SdrTextObj* pTextObject = static_cast<SdrTextObj*>(pObject1);

    // Double-click outside the text to enter edit mode.
    const ::tools::Rectangle aRect = pTextObject->GetCurrentBoundRect();
    const auto cornerX = convertMm100ToTwip(aRect.getX() + (aRect.getWidth() / 4));
    const auto cornerY = convertMm100ToTwip(aRect.getY() + (aRect.getHeight() / 4));
    pXImpressDocument->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONDOWN,
                                      cornerX, cornerY,
                                      2, MOUSE_LEFT, 0);
    pXImpressDocument->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONUP,
                                      cornerX, cornerY,
                                      2, MOUSE_LEFT, 0);
    Scheduler::ProcessEventsToIdle();

    // We must be in text editing mode and have cursor visible.
    CPPUNIT_ASSERT(pViewShell->GetView()->IsTextEdit());
    CPPUNIT_ASSERT(aView1.m_bCursorVisible);
}

void SdTiledRenderingTest::testCursorVisibility_MultiView()
{
    // Create the first view.
    SdXImpressDocument* pXImpressDocument = createDoc("dummy.odp");
    const int nView1 = SfxLokHelper::getView();
    ViewCallback aView1;

    // Begin text edit on the only object on the slide.
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    SdPage* pActualPage = pViewShell->GetActualPage();
    SdrObject* pObject1 = pActualPage->GetObj(0);
    CPPUNIT_ASSERT(pObject1);
    CPPUNIT_ASSERT_EQUAL(OBJ_TITLETEXT, pObject1->GetObjIdentifier());
    SdrTextObj* pTextObject = static_cast<SdrTextObj*>(pObject1);

    // Make sure that cursor state is not changed just because we create a second view.
    SfxLokHelper::createView();
    pXImpressDocument->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    const int nView2 = SfxLokHelper::getView();
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(false, aView1.m_bCursorVisibleChanged);
    CPPUNIT_ASSERT_EQUAL(false, aView1.m_aViewCursorVisibilities[nView2]);

    // Also check that the second view gets the notifications.
    ViewCallback aView2;

    SfxLokHelper::setView(nView1);

    ::tools::Rectangle aRect = pTextObject->GetCurrentBoundRect();
    const auto centerX = convertMm100ToTwip(aRect.getX() + (aRect.getWidth() / 2));
    const auto centerY = convertMm100ToTwip(aRect.getY() + (aRect.getHeight() / 2));
    pXImpressDocument->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONDOWN,
                                      centerX, centerY,
                                      2, MOUSE_LEFT, 0);
    pXImpressDocument->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONUP,
                                      centerX, centerY,
                                      2, MOUSE_LEFT, 0);
    Scheduler::ProcessEventsToIdle();

    // We must be in text editing mode and have cursor visible.
    CPPUNIT_ASSERT(pViewShell->GetView()->IsTextEdit());
    CPPUNIT_ASSERT(aView1.m_bCursorVisible);
    CPPUNIT_ASSERT_EQUAL(false, aView1.m_aViewCursorVisibilities[nView2]);

    CPPUNIT_ASSERT_EQUAL(false, aView2.m_bCursorVisible);
    CPPUNIT_ASSERT_EQUAL(false, aView2.m_aViewCursorVisibilities[nView1]);
    CPPUNIT_ASSERT_EQUAL(false, aView2.m_aViewCursorVisibilities[nView2]);
}

void SdTiledRenderingTest::testCursorVisibility_Escape()
{
    // Load doc.
    SdXImpressDocument* pXImpressDocument = createDoc("dummy.odp");
    ViewCallback aView1;

    // Begin text edit on the only object on the slide.
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    SdPage* pActualPage = pViewShell->GetActualPage();
    SdrObject* pObject1 = pActualPage->GetObj(0);
    CPPUNIT_ASSERT(pObject1 != nullptr);
    CPPUNIT_ASSERT_EQUAL(OBJ_TITLETEXT, pObject1->GetObjIdentifier());
    SdrTextObj* pTextObject = static_cast<SdrTextObj*>(pObject1);

    // Click once on the text to start editing.
    const ::tools::Rectangle aRect = pTextObject->GetCurrentBoundRect();
    const auto centerX = convertMm100ToTwip(aRect.getX() + (aRect.getWidth() / 2));
    const auto centerY = convertMm100ToTwip(aRect.getY() + (aRect.getHeight() / 2));
    pXImpressDocument->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONDOWN,
                                      centerX, centerY,
                                      1, MOUSE_LEFT, 0);
    pXImpressDocument->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONUP,
                                      centerX, centerY,
                                      1, MOUSE_LEFT, 0);
    Scheduler::ProcessEventsToIdle();

    // We must be in text editing mode and have cursor visible.
    CPPUNIT_ASSERT(pViewShell->GetView()->IsTextEdit());
    CPPUNIT_ASSERT(aView1.m_bCursorVisible);

    // End editing by pressing the escape key.
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::ESCAPE);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::ESCAPE);
    Scheduler::ProcessEventsToIdle();

    // We must be in text editing mode and have cursor visible.
    CPPUNIT_ASSERT(!pViewShell->GetView()->IsTextEdit());
    CPPUNIT_ASSERT_EQUAL(false, aView1.m_bCursorVisible);
}

void SdTiledRenderingTest::testViewLock()
{
    // Load a document that has a shape and create two views.
    SdXImpressDocument* pXImpressDocument = createDoc("shape.odp");
    ViewCallback aView1;
    SfxLokHelper::createView();
    pXImpressDocument->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());

    // Begin text edit in the second view and assert that the first gets a lock
    // notification.
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    SdPage* pActualPage = pViewShell->GetActualPage();
    SdrObject* pObject = pActualPage->GetObj(0);
    SdrView* pView = pViewShell->GetView();
    aView1.m_bViewLock = false;
    pView->SdrBeginTextEdit(pObject);
    CPPUNIT_ASSERT(aView1.m_bViewLock);

    // End text edit in the second view, and assert that the lock is removed in
    // the first view.
    pView->SdrEndTextEdit();
    CPPUNIT_ASSERT(!aView1.m_bViewLock);
}

void SdTiledRenderingTest::testUndoLimiting()
{
    // Create the first view.
    SdXImpressDocument* pXImpressDocument = createDoc("title-shape.odp");
    sd::ViewShell* pViewShell1 = pXImpressDocument->GetDocShell()->GetViewShell();
    int nView1 = SfxLokHelper::getView();
    SfxLokHelper::createView();
    sd::ViewShell* pViewShell2 = pXImpressDocument->GetDocShell()->GetViewShell();
    CPPUNIT_ASSERT(pViewShell1 != pViewShell2);

    // Begin text edit on the only object on the slide.
    SfxLokHelper::setView(nView1);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::TAB);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::TAB);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'x', 0);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 'x', 0);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(pViewShell1->GetView()->IsTextEdit());

    // Now check view2 cannot undo actions.
    {
        SfxRequest aReq2(SID_UNDO, SfxCallMode::SLOT, pXImpressDocument->GetDocShell()->GetDoc()->GetPool());
        aReq2.AppendItem(SfxUInt16Item(SID_UNDO, 1));
        pViewShell2->ExecuteSlot(aReq2);
        const SfxUInt32Item* pUInt32Item = dynamic_cast<const SfxUInt32Item*>(aReq2.GetReturnValue());
        CPPUNIT_ASSERT(pUInt32Item);
        CPPUNIT_ASSERT_EQUAL(static_cast< sal_uInt32 >(SID_REPAIRPACKAGE), pUInt32Item->GetValue());
    }

    // Now check view1 can undo action
    {
        SfxRequest aReq1(SID_UNDO, SfxCallMode::SLOT, pXImpressDocument->GetDocShell()->GetDoc()->GetPool());
        aReq1.AppendItem(SfxUInt16Item(SID_UNDO, 1));
        pViewShell1->ExecuteSlot(aReq1);
        CPPUNIT_ASSERT(aReq1.IsDone());
    }

    mxComponent->dispose();
    mxComponent.clear();
}

void SdTiledRenderingTest::testCreateViewGraphicSelection()
{
    // Load a document and register a callback.
    SdXImpressDocument* pXImpressDocument = createDoc("shape.odp");
    ViewCallback aView1;

    // Select the only shape in the document and assert that the graphic selection is changed.
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    SdPage* pActualPage = pViewShell->GetActualPage();
    SdrObject* pObject = pActualPage->GetObj(0);
    SdrView* pView = pViewShell->GetView();
    aView1.m_bGraphicSelectionInvalidated = false;
    pView->MarkObj(pObject, pView->GetSdrPageView());
    CPPUNIT_ASSERT(aView1.m_bGraphicSelectionInvalidated);

    // Now create a new view.
    aView1.m_bGraphicSelectionInvalidated = false;
    SfxLokHelper::createView();
    pXImpressDocument->initializeForTiledRendering({});
    // This failed, creating a new view affected the graphic selection of an
    // existing view.
    CPPUNIT_ASSERT(!aView1.m_bGraphicSelectionInvalidated);

    // Check that when the first view has a shape selected and we register a
    // callback on the second view, then it gets a "graphic view selection".
    ViewCallback aView2;
    // This failed, the created new view had no "view selection" of the first
    // view's selected shape.
    CPPUNIT_ASSERT(aView2.m_bGraphicViewSelectionInvalidated);
}

void SdTiledRenderingTest::testCreateViewTextCursor()
{
    // Load a document and register a callback.
    SdXImpressDocument* pXImpressDocument = createDoc("title-shape.odp");
    ViewCallback aView1;

    // Begin text edit.
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::TAB);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::TAB);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'x', 0);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 'x', 0);
    Scheduler::ProcessEventsToIdle();
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    SdrView* pSdrView = pViewShell->GetView();
    CPPUNIT_ASSERT(pSdrView->IsTextEdit());

    // Create an editeng text selection.
    EditView& rEditView = pSdrView->GetTextEditOutlinerView()->GetEditView();
    // 0th para, 0th char -> 0th para, 1st char.
    ESelection aWordSelection(0, 0, 0, 1);
    rEditView.SetSelection(aWordSelection);

    // Make sure that creating a new view either doesn't affect the previous
    // one, or at least the effect is not visible at the end.
    aView1.m_aViewCursorInvalidations.clear();
    aView1.m_aViewCursorVisibilities.clear();
    SfxLokHelper::createView();
    pXImpressDocument->initializeForTiledRendering({});
    ViewCallback aView2;
    bool bFoundCursor = false;
    for (const auto& rInvalidation : aView1.m_aViewCursorInvalidations)
    {
        auto itVisibility = aView1.m_aViewCursorVisibilities.find(rInvalidation.first);
        // For each cursor invalidation: if there is no visibility or the visibility is true, that's a problem.
        if (itVisibility == aView1.m_aViewCursorVisibilities.end() || itVisibility->second)
        {
            bFoundCursor = true;
            break;
        }
    }
    // This failed: the second view created an unexpected view cursor in the
    // first view.
    CPPUNIT_ASSERT(!bFoundCursor);
    // This failed: the text view selection of the first view wasn't seen by
    // the second view.
    CPPUNIT_ASSERT(aView2.m_bViewSelectionSet);
}

void SdTiledRenderingTest::testTdf102223()
{
    // Load the document.
    SdXImpressDocument* pXImpressDocument = createDoc("tdf102223.odp");
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    SdPage* pActualPage = pViewShell->GetActualPage();
    auto pTableObject = dynamic_cast<sdr::table::SdrTableObj*>(pActualPage->GetObj(2));
    CPPUNIT_ASSERT(pTableObject);
    SdrView* pView = pViewShell->GetView();

    // select contents of cell
    ::tools::Rectangle aRect = pTableObject->GetCurrentBoundRect();
    pXImpressDocument->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONDOWN,
                                      convertMm100ToTwip(aRect.getX() + 2), convertMm100ToTwip(aRect.getY() + 2),
                                      1, MOUSE_LEFT, 0);
    pXImpressDocument->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONUP,
                                      convertMm100ToTwip(aRect.getX() + 2), convertMm100ToTwip(aRect.getY() + 2),
                                      1, MOUSE_LEFT, 0);
    Scheduler::ProcessEventsToIdle();
    pView->SdrBeginTextEdit(pTableObject);
    CPPUNIT_ASSERT(pView->GetTextEditObject());
    EditView& rEditView = pView->GetTextEditOutlinerView()->GetEditView();
    rEditView.SetSelection(ESelection(0, 0, 0, 3)); // start para, start char, end para, end char.
    CPPUNIT_ASSERT_EQUAL(OUString("Red"), rEditView.GetSelected());
    const SvxFontHeightItem& rItem = rEditView.GetAttribs().Get(EE_CHAR_FONTHEIGHT);
    CPPUNIT_ASSERT_EQUAL(int(1411), static_cast<int>(rItem.GetHeight()));

    // cut contents of cell
    uno::Sequence<beans::PropertyValue> aArgs;
    comphelper::dispatchCommand(".uno:Cut", aArgs);

    pView->SdrEndTextEdit(false);
    pView->SdrBeginTextEdit(pTableObject);
    CPPUNIT_ASSERT(pView->GetTextEditObject());
    EditView& rEditView2 = pView->GetTextEditOutlinerView()->GetEditView();
    rEditView2.SetSelection(ESelection(0, 0, 0, 1)); // start para, start char, end para, end char.
    const SvxFontHeightItem& rItem2 = rEditView2.GetAttribs().Get(EE_CHAR_FONTHEIGHT);
    CPPUNIT_ASSERT_EQUAL(int(1411), static_cast<int>(rItem2.GetHeight()));
}

void SdTiledRenderingTest::testTdf118354()
{
    // Load the document.
    SdXImpressDocument* pXImpressDocument = createDoc("tdf118354.odp");

    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    SdPage* pActualPage = pViewShell->GetActualPage();

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), pActualPage->GetObjCount());

    auto pTableObject = dynamic_cast<sdr::table::SdrTableObj*>(pActualPage->GetObj(0));
    CPPUNIT_ASSERT(pTableObject);

    // Without the fix, it would crash here
    ::tools::Rectangle aRect = pTableObject->GetCurrentBoundRect();
    pXImpressDocument->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONDOWN,
                                      convertMm100ToTwip(aRect.getX() + 2), convertMm100ToTwip(aRect.getY() + 2),
                                      1, MOUSE_LEFT, 0);
    pXImpressDocument->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONUP,
                                      convertMm100ToTwip(aRect.getX() + 2), convertMm100ToTwip(aRect.getY() + 2),
                                      1, MOUSE_LEFT, 0);
    Scheduler::ProcessEventsToIdle();

    SdrView* pView = pViewShell->GetView();
    rtl::Reference<sdr::SelectionController> xSelectionController(pView->getSelectionController());
    CPPUNIT_ASSERT(xSelectionController->hasSelectedCells());
}

void SdTiledRenderingTest::testPostKeyEventInvalidation()
{
    // Load a document and begin text edit on the first slide.
    SdXImpressDocument* pXImpressDocument = createDoc("2slides.odp");
    CPPUNIT_ASSERT_EQUAL(0, pXImpressDocument->getPart());
    ViewCallback aView1;
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    SdrView* pView = pViewShell->GetView();
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_TAB);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_F2);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_F2);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(pView->GetTextEditObject());

    // Create a second view and begin text edit there as well, in parallel.
    SfxLokHelper::createView();
    pXImpressDocument->initializeForTiledRendering({});
    ViewCallback aView2;
    pXImpressDocument->setPart(1);
    sd::ViewShell* pViewShell2 = pXImpressDocument->GetDocShell()->GetViewShell();
    SdrView* pView2 = pViewShell2->GetView();
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_TAB);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_F2);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_F2);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(pView2->GetTextEditObject());

    // Now go left with the cursor in the second view and watch for
    // invalidations.
    aView2.m_bTilesInvalidated = false;
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_LEFT);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_LEFT);
    Scheduler::ProcessEventsToIdle();
    // This failed: moving the cursor caused unexpected invalidation.
    CPPUNIT_ASSERT(!aView2.m_bTilesInvalidated);
}

/**
 * tests a cut/paste bug around bullet items in a list
 */
void SdTiledRenderingTest::testTdf103083()
{
    // Load the document.
    SdXImpressDocument* pXImpressDocument = createDoc("tdf103083.fodp");
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    SdPage* pActualPage = pViewShell->GetActualPage();

    SdrObject* pObject1 = pActualPage->GetObj(1);
    CPPUNIT_ASSERT_EQUAL(OBJ_OUTLINETEXT, pObject1->GetObjIdentifier());
    SdrTextObj* pTextObject = static_cast<SdrTextObj*>(pObject1);

    SdrView* pView = pViewShell->GetView();

    // select contents of bullet item
    ::tools::Rectangle aRect = pTextObject->GetCurrentBoundRect();
    pXImpressDocument->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONDOWN,
                                      convertMm100ToTwip(aRect.getX() + 2), convertMm100ToTwip(aRect.getY() + 2),
                                      1, MOUSE_LEFT, 0);
    pXImpressDocument->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONUP,
                                      convertMm100ToTwip(aRect.getX() + 2), convertMm100ToTwip(aRect.getY() + 2),
                                      1, MOUSE_LEFT, 0);
    Scheduler::ProcessEventsToIdle();
    pView->SdrBeginTextEdit(pTextObject);
    CPPUNIT_ASSERT(pView->GetTextEditObject());
    EditView& rEditView = pView->GetTextEditOutlinerView()->GetEditView();
    rEditView.SetSelection(ESelection(2, 0, 2, 33)); // start para, start char, end para, end char.
    CPPUNIT_ASSERT_EQUAL(OUString("They have all the same formatting"), rEditView.GetSelected());
    SdrOutliner* pOutliner = pView->GetTextEditOutliner();
    CPPUNIT_ASSERT_EQUAL(OUString("No-Logo Content~LT~Gliederung 2"),
                         pOutliner->GetStyleSheet(2)->GetName());
    const EditTextObject& aEdit = pTextObject->GetOutlinerParaObject()->GetTextObject();
    const SvxNumBulletItem* pNumFmt = aEdit.GetParaAttribs(2).GetItem(EE_PARA_NUMBULLET);
    SvxNumberFormat aNumFmt(pNumFmt->GetNumRule()->GetLevel(2));

    // cut contents of bullet item
    comphelper::dispatchCommand(".uno:Cut", uno::Sequence<beans::PropertyValue>());

    CPPUNIT_ASSERT(pView->GetTextEditObject());
    EditView& rEditView2 = pView->GetTextEditOutlinerView()->GetEditView();
    rEditView2.SetSelection(ESelection(2, 0, 2, 10)); // start para, start char, end para, end char.
    CPPUNIT_ASSERT_EQUAL(OUString(), rEditView2.GetSelected());

    // paste contents of bullet item
    comphelper::dispatchCommand(".uno:Paste", uno::Sequence<beans::PropertyValue>());

    // send an ESC key to trigger the commit of the edit to the main model
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::ESCAPE);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::ESCAPE);
    Scheduler::ProcessEventsToIdle();

    pView->SdrBeginTextEdit(pTextObject);
    CPPUNIT_ASSERT(pView->GetTextEditObject());
    pOutliner = pView->GetTextEditOutliner();
    EditView& rEditView3 = pView->GetTextEditOutlinerView()->GetEditView();
    rEditView3.SetSelection(ESelection(2, 0, 2, 33)); // start para, start char, end para, end char.
    CPPUNIT_ASSERT_EQUAL(OUString("They have all the same formatting"), rEditView3.GetSelected());
    CPPUNIT_ASSERT_EQUAL(OUString("No-Logo Content~LT~Gliederung 2"),
                         pOutliner->GetStyleSheet(2)->GetName());

    const EditTextObject& aEdit2 = pTextObject->GetOutlinerParaObject()->GetTextObject();
    const SvxNumBulletItem* pNumFmt2 = aEdit2.GetParaAttribs(2).GetItem(EE_PARA_NUMBULLET);
    SvxNumberFormat aNumFmt2(pNumFmt2->GetNumRule()->GetLevel(2));

    bool bEqual(aNumFmt2 == aNumFmt);
    CPPUNIT_ASSERT_MESSAGE("Bullet properties changed after paste", bEqual);
}

/**
 * tests a clone-formatting bug around table cell attributes
 */
void SdTiledRenderingTest::testTdf104405()
{
    // Load the document.
    SdXImpressDocument* pXImpressDocument = createDoc("tdf104405.fodp");
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    SdPage* pActualPage = pViewShell->GetActualPage();
    SdrObject* pObject = pActualPage->GetObj(2);
    auto pTableObject = dynamic_cast<sdr::table::SdrTableObj*>(pObject);
    CPPUNIT_ASSERT(pTableObject);

    // select the middle cell
    SdrView* pView = pViewShell->GetView();
    pView->MarkObj(pTableObject, pView->GetSdrPageView());
    pTableObject->setActiveCell(sdr::table::CellPos(2,1));
    pView->SdrBeginTextEdit(pTableObject);
    EditView& rEditView = pView->GetTextEditOutlinerView()->GetEditView();
    rEditView.SetSelection(ESelection(0, 0, 0, 3)); // start para, start char, end para, end char.

    // trigger the clone-formatting/paintbrush command to copy formatting contents of cell
    uno::Sequence< beans::PropertyValue > aArgs(1);
    aArgs[0].Name  = "PersistentCopy";
    aArgs[0].Value <<= true;
    comphelper::dispatchCommand(".uno:FormatPaintbrush", aArgs);

    Scheduler::ProcessEventsToIdle();

    // now click on the table
    pView->MarkObj(pTableObject, pView->GetSdrPageView());
    pTableObject->setActiveCell(sdr::table::CellPos(0,0));
    pView->SdrEndTextEdit(false);
    pView->SdrBeginTextEdit(pTableObject);
    EditView& rEditView2 = pView->GetTextEditOutlinerView()->GetEditView();
    rEditView2.SetSelection(ESelection(0, 0, 0, 3)); // start para, start char, end para, end char.
    ::tools::Rectangle aRect = pTableObject->GetCurrentBoundRect();
    pXImpressDocument->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONDOWN,
                                      convertMm100ToTwip(aRect.getX()), convertMm100ToTwip(aRect.getY()),
                                      1, MOUSE_LEFT, 0);
    pXImpressDocument->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONUP,
                                      convertMm100ToTwip(aRect.getX()), convertMm100ToTwip(aRect.getY()),
                                      1, MOUSE_LEFT, 0);

    Scheduler::ProcessEventsToIdle();

    // check that the first cell has acquired the resulting vertical style
    xmlDocUniquePtr pXmlDoc = parseXmlDump();
    // the following name has a compiler-dependent part
    CPPUNIT_ASSERT_EQUAL(
        OUString("2"),
        getXPath(
            pXmlDoc,
            "/SdDrawDocument/SdrModel/SdPage/SdrObjList/SdrTableObj/SdrTableObjImpl"
                "/TableModel/Cell[1]/DefaultProperties/SfxItemSet/SdrTextVertAdjustItem",
            "value"));
}

void SdTiledRenderingTest::testTdf81754()
{
    SdXImpressDocument* pXImpressDocument = createDoc("tdf81754.pptx");
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    SdPage* pActualPage = pViewShell->GetActualPage();
    SdrObject* pObject = pActualPage->GetObj(1);

    SdrTextObj* pTextObj = static_cast<SdrTextObj*>(pObject);
    SdrView* pView = pViewShell->GetView();
    pView->MarkObj(pTextObj, pView->GetSdrPageView());
    SfxStringItem aInputString(SID_ATTR_CHAR, "x");
    pViewShell->GetViewFrame()->GetDispatcher()->ExecuteList(SID_ATTR_CHAR,
            SfxCallMode::SYNCHRON, { &aInputString });

    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'x', 0);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 'x', 0);
    Scheduler::ProcessEventsToIdle();

    // now save, reload, and assert that we did not lose the edit
    ::sd::DrawDocShellRef xDocShRef = saveAndReload(pXImpressDocument->GetDocShell(), PPTX);

    const SdrPage* pPage = GetPage(1, xDocShRef);
    SdrTextObj* pTextObject = dynamic_cast<SdrTextObj*>(pPage->GetObj(1));
    CPPUNIT_ASSERT(pTextObject);

    OutlinerParaObject* pOutlinerParagraphObject = pTextObject->GetOutlinerParaObject();
    const EditTextObject& aEdit = pOutlinerParagraphObject->GetTextObject();

    CPPUNIT_ASSERT_EQUAL(OUString("Somethingxx"), aEdit.GetText(0));

    xDocShRef->DoClose();
}

void SdTiledRenderingTest::testTdf105502()
{
    // Load the document.
    SdXImpressDocument* pXImpressDocument = createDoc("tdf105502.odp");
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    sd::Window* pWindow = pViewShell->GetActiveWindow();
    CPPUNIT_ASSERT(pWindow);
    SdPage* pActualPage = pViewShell->GetActualPage();
    SdrObject* pObject = pActualPage->GetObj(0);
    auto pTableObject = dynamic_cast<sdr::table::SdrTableObj*>(pObject);
    CPPUNIT_ASSERT(pTableObject);

    // Select the first row.
    sd::View* pView = pViewShell->GetView();
    pView->MarkObj(pObject, pView->GetSdrPageView());
    pView->SdrBeginTextEdit(pObject);
    rtl::Reference<sdr::SelectionController> xSelectionController(pView->getSelectionController());
    CPPUNIT_ASSERT(xSelectionController.is());
    SfxRequest aRequest(pViewShell->GetViewFrame(), SID_TABLE_SELECT_ROW);
    xSelectionController->Execute(aRequest);

    // Assert that the A1:B1 selection succeeded.
    CPPUNIT_ASSERT(xSelectionController->hasSelectedCells());
    sdr::table::CellPos aFirstCell;
    sdr::table::CellPos aLastCell;
    xSelectionController->getSelectedCells(aFirstCell, aLastCell);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), aFirstCell.mnCol);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), aFirstCell.mnRow);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), aLastCell.mnCol);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), aLastCell.mnRow);

    // Grow font size for the selection.
    comphelper::dispatchCommand(".uno:Grow", {});
    Scheduler::ProcessEventsToIdle();

    // Assert that the selected A1 has now a larger font than the unselected
    // A2.
    xmlDocUniquePtr pXmlDoc = parseXmlDump();
    sal_Int32 nA1Height = getXPath(pXmlDoc, "//Cell[1]/SdrText/OutlinerParaObject/EditTextObject/ContentInfo/SfxItemSet/SvxFontHeightItem[1]", "height").toInt32();
    sal_Int32 nA2Height = getXPath(pXmlDoc, "//Cell[3]/SdrText/OutlinerParaObject/EditTextObject/ContentInfo/attribs[1]/SvxFontHeightItem", "height").toInt32();
    // This failed when FuText::ChangeFontSize() never did "continue" in the
    // text loop, instead of doing so depending on what IsInSelection() returns.
    CPPUNIT_ASSERT(nA1Height > nA2Height);

    // Check that selection remains the same
    CPPUNIT_ASSERT(xSelectionController->hasSelectedCells());
    xSelectionController->getSelectedCells(aFirstCell, aLastCell);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), aFirstCell.mnCol);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), aFirstCell.mnRow);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), aLastCell.mnCol);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), aLastCell.mnRow);
}

void SdTiledRenderingTest::testCommentCallbacks()
{
    // Load the document.
    // Set the tiled annotations off
    comphelper::LibreOfficeKit::setTiledAnnotations(false);

    SdXImpressDocument* pXImpressDocument = createDoc("dummy.odp", comphelper::InitPropertySequence(
    {
        {".uno:Author", uno::makeAny(OUString("LOK User1"))},
    }));
    ViewCallback aView1;
    int nView1 = SfxLokHelper::getView();

    SfxLokHelper::createView();
    uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence(
    {
        {".uno:Author", uno::makeAny(OUString("LOK User2"))},
    }));
    pXImpressDocument->initializeForTiledRendering(aArgs);
    ViewCallback aView2;
    int nView2 = SfxLokHelper::getView();

    SfxLokHelper::setView(nView1);

    // Add a new comment
    aArgs = comphelper::InitPropertySequence(
    {
        {"Text", uno::makeAny(OUString("Comment"))},
    });
    comphelper::dispatchCommand(".uno:InsertAnnotation", aArgs);
    Scheduler::ProcessEventsToIdle();

    // We received a LOK_CALLBACK_COMMENT callback with comment 'Add' action
    CPPUNIT_ASSERT_EQUAL(std::string("Add"), aView1.m_aCommentCallbackResult.get<std::string>("action"));
    CPPUNIT_ASSERT_EQUAL(std::string("Add"), aView2.m_aCommentCallbackResult.get<std::string>("action"));
    int nComment1 = aView1.m_aCommentCallbackResult.get<int>("id");
    CPPUNIT_ASSERT_EQUAL(nComment1, aView2.m_aCommentCallbackResult.get<int>("id"));
    css::util::DateTime aDateTime;
    OUString aDateTimeString = OUString::createFromAscii(aView1.m_aCommentCallbackResult.get<std::string>("dateTime").c_str());
    CPPUNIT_ASSERT(utl::ISO8601parseDateTime(aDateTimeString, aDateTime));
    CPPUNIT_ASSERT_EQUAL(std::string("LOK User1"), aView1.m_aCommentCallbackResult.get<std::string>("author"));
    CPPUNIT_ASSERT_EQUAL(std::string("LOK User1"), aView2.m_aCommentCallbackResult.get<std::string>("author"));
    CPPUNIT_ASSERT_EQUAL(std::string("Comment"), aView1.m_aCommentCallbackResult.get<std::string>("text"));
    CPPUNIT_ASSERT_EQUAL(std::string("Comment"), aView2.m_aCommentCallbackResult.get<std::string>("text"));
    CPPUNIT_ASSERT(!aView1.m_aCommentCallbackResult.get<std::string>("parthash").empty());
    CPPUNIT_ASSERT(!aView2.m_aCommentCallbackResult.get<std::string>("parthash").empty());

    // Reply to a just added comment
    SfxLokHelper::setView(nView2);
    aArgs = comphelper::InitPropertySequence(
    {
        {"Id", uno::makeAny(OUString::number(nComment1))},
        {"Text", uno::makeAny(OUString("Reply to comment"))},
    });
    comphelper::dispatchCommand(".uno:ReplyToAnnotation", aArgs);
    Scheduler::ProcessEventsToIdle();

    // We received a LOK_CALLBACK_COMMENT callback with comment 'Modify' action
    CPPUNIT_ASSERT_EQUAL(std::string("Modify"), aView1.m_aCommentCallbackResult.get<std::string>("action"));
    CPPUNIT_ASSERT_EQUAL(std::string("Modify"), aView2.m_aCommentCallbackResult.get<std::string>("action"));
    CPPUNIT_ASSERT_EQUAL(nComment1, aView1.m_aCommentCallbackResult.get<int>("id"));
    CPPUNIT_ASSERT_EQUAL(nComment1, aView2.m_aCommentCallbackResult.get<int>("id"));
    CPPUNIT_ASSERT_EQUAL(std::string("LOK User2"), aView1.m_aCommentCallbackResult.get<std::string>("author"));
    CPPUNIT_ASSERT_EQUAL(std::string("LOK User2"), aView2.m_aCommentCallbackResult.get<std::string>("author"));
    OUString aReplyTextView1 = OUString::createFromAscii(aView1.m_aCommentCallbackResult.get<std::string>("text").c_str());
    OUString aReplyTextView2 = OUString::createFromAscii(aView2.m_aCommentCallbackResult.get<std::string>("text").c_str());
    CPPUNIT_ASSERT(aReplyTextView1.startsWith("Reply to LOK User1"));
    CPPUNIT_ASSERT(aReplyTextView1.endsWith("Reply to comment"));
    CPPUNIT_ASSERT(aReplyTextView2.startsWith("Reply to LOK User1"));
    CPPUNIT_ASSERT(aReplyTextView2.endsWith("Reply to comment"));
    CPPUNIT_ASSERT(!aView1.m_aCommentCallbackResult.get<std::string>("parthash").empty());
    CPPUNIT_ASSERT(!aView2.m_aCommentCallbackResult.get<std::string>("parthash").empty());

    // Edit this annotation now
    aArgs = comphelper::InitPropertySequence(
    {
        {"Id", uno::makeAny(OUString::number(nComment1))},
        {"Text", uno::makeAny(OUString("Edited comment"))},
    });
    comphelper::dispatchCommand(".uno:EditAnnotation", aArgs);
    Scheduler::ProcessEventsToIdle();

    // We received a LOK_CALLBACK_COMMENT callback with comment 'Modify' action
    CPPUNIT_ASSERT_EQUAL(std::string("Modify"), aView1.m_aCommentCallbackResult.get<std::string>("action"));
    CPPUNIT_ASSERT_EQUAL(std::string("Modify"), aView2.m_aCommentCallbackResult.get<std::string>("action"));
    CPPUNIT_ASSERT_EQUAL(nComment1, aView1.m_aCommentCallbackResult.get<int>("id"));
    CPPUNIT_ASSERT_EQUAL(nComment1, aView2.m_aCommentCallbackResult.get<int>("id"));
    CPPUNIT_ASSERT(!aView1.m_aCommentCallbackResult.get<std::string>("parthash").empty());
    CPPUNIT_ASSERT(!aView2.m_aCommentCallbackResult.get<std::string>("parthash").empty());
    CPPUNIT_ASSERT_EQUAL(std::string("Edited comment"), aView1.m_aCommentCallbackResult.get<std::string>("text"));
    CPPUNIT_ASSERT_EQUAL(std::string("Edited comment"), aView2.m_aCommentCallbackResult.get<std::string>("text"));

    // Delete the comment
    aArgs = comphelper::InitPropertySequence(
    {
        {"Id", uno::makeAny(OUString::number(nComment1))},
    });
    comphelper::dispatchCommand(".uno:DeleteAnnotation", aArgs);
    Scheduler::ProcessEventsToIdle();

    // We received a LOK_CALLBACK_COMMENT callback with comment 'Remove' action
    CPPUNIT_ASSERT_EQUAL(std::string("Remove"), aView1.m_aCommentCallbackResult.get<std::string>("action"));
    CPPUNIT_ASSERT_EQUAL(std::string("Remove"), aView2.m_aCommentCallbackResult.get<std::string>("action"));
    CPPUNIT_ASSERT_EQUAL(nComment1, aView1.m_aCommentCallbackResult.get<int>("id"));
    CPPUNIT_ASSERT_EQUAL(nComment1, aView2.m_aCommentCallbackResult.get<int>("id"));

    comphelper::LibreOfficeKit::setTiledAnnotations(true);
}

void SdTiledRenderingTest::testCommentChangeImpress()
{
    uno::Sequence<beans::PropertyValue> aArgs;

    // Load the document.
    // Set the tiled annotations off
    comphelper::LibreOfficeKit::setTiledAnnotations(false);

    createDoc("dummy.odp", comphelper::InitPropertySequence(
    {
        {".uno:Author", uno::makeAny(OUString("LOK User1"))},
    }));

    ViewCallback aView1;

    // Add a new comment
    aArgs = comphelper::InitPropertySequence(
    {
        {"Text", uno::makeAny(OUString("Comment"))},
    });
    comphelper::dispatchCommand(".uno:InsertAnnotation", aArgs);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(std::string("Add"), aView1.m_aCommentCallbackResult.get<std::string>("action"));

    int nComment1 = aView1.m_aCommentCallbackResult.get<int>("id");

    CPPUNIT_ASSERT(!aView1.m_aCommentCallbackResult.get<std::string>("parthash").empty());
    CPPUNIT_ASSERT_EQUAL(std::string("Comment"), aView1.m_aCommentCallbackResult.get<std::string>("text"));
    CPPUNIT_ASSERT_EQUAL(std::string("0, 0, 0, 0"), aView1.m_aCommentCallbackResult.get<std::string>("rectangle"));

    // Edit this annotation now
    aArgs = comphelper::InitPropertySequence(
    {
        {"Id", uno::makeAny(OUString::number(nComment1))},
        {"PositionX", uno::makeAny(sal_Int32(10))},
        {"PositionY", uno::makeAny(sal_Int32(20))}
    });
    comphelper::dispatchCommand(".uno:EditAnnotation", aArgs);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(std::string("Modify"), aView1.m_aCommentCallbackResult.get<std::string>("action"));
    CPPUNIT_ASSERT_EQUAL(std::string("Comment"), aView1.m_aCommentCallbackResult.get<std::string>("text"));
    CPPUNIT_ASSERT_EQUAL(std::string("10, 20, 0, 0"), aView1.m_aCommentCallbackResult.get<std::string>("rectangle"));

    comphelper::LibreOfficeKit::setTiledAnnotations(true);
}

void SdTiledRenderingTest::testCommentChangeDraw()
{
    uno::Sequence<beans::PropertyValue> aArgs;

    // Load the document.
    // Set the tiled annotations off
    comphelper::LibreOfficeKit::setTiledAnnotations(false);

    createDoc("dummy.odg", comphelper::InitPropertySequence(
    {
        {".uno:Author", uno::makeAny(OUString("LOK User1"))},
    }));

    ViewCallback aView1;

    // Add a new comment
    aArgs = comphelper::InitPropertySequence(
    {
        {"Text", uno::makeAny(OUString("Comment"))},
    });
    comphelper::dispatchCommand(".uno:InsertAnnotation", aArgs);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(std::string("Add"), aView1.m_aCommentCallbackResult.get<std::string>("action"));

    int nComment1 = aView1.m_aCommentCallbackResult.get<int>("id");

    CPPUNIT_ASSERT(!aView1.m_aCommentCallbackResult.get<std::string>("parthash").empty());
    CPPUNIT_ASSERT_EQUAL(std::string("Comment"), aView1.m_aCommentCallbackResult.get<std::string>("text"));
    CPPUNIT_ASSERT_EQUAL(std::string("0, 0, 0, 0"), aView1.m_aCommentCallbackResult.get<std::string>("rectangle"));

    // Edit this annotation now
    aArgs = comphelper::InitPropertySequence(
    {
        {"Id", uno::makeAny(OUString::number(nComment1))},
        {"PositionX", uno::makeAny(sal_Int32(10))},
        {"PositionY", uno::makeAny(sal_Int32(20))}
    });
    comphelper::dispatchCommand(".uno:EditAnnotation", aArgs);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(std::string("Modify"), aView1.m_aCommentCallbackResult.get<std::string>("action"));
    CPPUNIT_ASSERT_EQUAL(std::string("Comment"), aView1.m_aCommentCallbackResult.get<std::string>("text"));
    CPPUNIT_ASSERT_EQUAL(std::string("10, 20, 0, 0"), aView1.m_aCommentCallbackResult.get<std::string>("rectangle"));

    comphelper::LibreOfficeKit::setTiledAnnotations(true);
}

void SdTiledRenderingTest::testMultiViewInsertDeletePage()
{
    // Load the document.
    SdXImpressDocument* pXImpressDocument = createDoc("dummy.odp");
    ViewCallback aView1;
    int nView1 = SfxLokHelper::getView();
    uno::Sequence<beans::PropertyValue> aArgs;
    SdDrawDocument* pDoc = pXImpressDocument->GetDocShell()->GetDoc();

    // Create second view
    SfxLokHelper::createView();
    pXImpressDocument->initializeForTiledRendering(aArgs);
    ViewCallback aView2;
    int nView2 = SfxLokHelper::getView();

    // the document has 8 slides
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(8), pDoc->GetSdPageCount(PageKind::Standard));

    // Switch to 5th page in 2nd view
    pXImpressDocument->setPart(4);

    // Insert slide in 1st view
    SfxLokHelper::setView(nView1);
    comphelper::dispatchCommand(".uno:InsertPage", aArgs);
    Scheduler::ProcessEventsToIdle();

    // See if the current slide number changed in 2nd view too
    SfxLokHelper::setView(nView2);
    CPPUNIT_ASSERT_EQUAL(5, pXImpressDocument->getPart());

    // Delete the page in 1st view now
    SfxLokHelper::setView(nView1);
    comphelper::dispatchCommand(".uno:DeletePage", aArgs);
    Scheduler::ProcessEventsToIdle();

    // See if current slide number changed in 2nd view too
    SfxLokHelper::setView(nView2);
    CPPUNIT_ASSERT_EQUAL(4, pXImpressDocument->getPart());
}

void SdTiledRenderingTest::testDisableUndoRepair()
{
    // Load the document.
    SdXImpressDocument* pXImpressDocument = createDoc("dummy.odp");
    SfxViewShell* pView1 = SfxViewShell::Current();
    int nView1 = SfxLokHelper::getView();
    SfxLokHelper::createView();
    SfxViewShell* pView2 = SfxViewShell::Current();
    int nView2 = SfxLokHelper::getView();
    {
        std::unique_ptr<SfxPoolItem> pItem1;
        std::unique_ptr<SfxPoolItem> pItem2;
        CPPUNIT_ASSERT_EQUAL(SfxItemState::DISABLED, pView1->GetViewFrame()->GetBindings().QueryState(SID_UNDO, pItem1));
        CPPUNIT_ASSERT_EQUAL(SfxItemState::DISABLED, pView2->GetViewFrame()->GetBindings().QueryState(SID_UNDO, pItem2));
    }

    // Insert a character in the first view.
    SfxLokHelper::setView(nView1);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::TAB);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::TAB);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'h', 0);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 'h', 0);
    Scheduler::ProcessEventsToIdle();
    {
        std::unique_ptr<SfxPoolItem> xItem1;
        std::unique_ptr<SfxPoolItem> xItem2;
        pView1->GetViewFrame()->GetBindings().QueryState(SID_UNDO, xItem1);
        pView2->GetViewFrame()->GetBindings().QueryState(SID_UNDO, xItem2);
        CPPUNIT_ASSERT(!dynamic_cast< const SfxUInt32Item* >(xItem1.get()));
        const SfxUInt32Item* pUInt32Item = dynamic_cast<const SfxUInt32Item*>(xItem2.get());
        CPPUNIT_ASSERT(pUInt32Item);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(SID_REPAIRPACKAGE), pUInt32Item->GetValue());
    }

    // Insert a character in the second view.
    SfxLokHelper::setView(nView2);
    pXImpressDocument->setPart(1);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::TAB);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::TAB);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'c', 0);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 'c', 0);
    Scheduler::ProcessEventsToIdle();
    {
        std::unique_ptr<SfxPoolItem> xItem1;
        std::unique_ptr<SfxPoolItem> xItem2;
        pView1->GetViewFrame()->GetBindings().QueryState(SID_UNDO, xItem1);
        pView2->GetViewFrame()->GetBindings().QueryState(SID_UNDO, xItem2);
        CPPUNIT_ASSERT(!dynamic_cast< const SfxUInt32Item* >(xItem2.get()));
        const SfxUInt32Item* pUInt32Item = dynamic_cast<const SfxUInt32Item*>(xItem1.get());
        CPPUNIT_ASSERT(pUInt32Item);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(SID_REPAIRPACKAGE), pUInt32Item->GetValue());
    }
}

void SdTiledRenderingTest::testDocumentRepair()
{
    // Create two views.
    SdXImpressDocument* pXImpressDocument = createDoc("dummy.odp");
    CPPUNIT_ASSERT(pXImpressDocument);

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
    pXImpressDocument->setPart(1);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::TAB);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::TAB);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'c', 0);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 'c', 0);
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
}

void SdTiledRenderingTest::testLanguageStatus()
{
    // Load the document.
    createDoc("dummy.odp");
    SfxViewShell* pView1 = SfxViewShell::Current();
    SfxLokHelper::createView();
    SfxViewShell* pView2 = SfxViewShell::Current();
    {
        std::unique_ptr<SfxPoolItem> xItem1;
        std::unique_ptr<SfxPoolItem> xItem2;
        pView1->GetViewFrame()->GetBindings().QueryState(SID_LANGUAGE_STATUS, xItem1);
        pView2->GetViewFrame()->GetBindings().QueryState(SID_LANGUAGE_STATUS, xItem2);
        auto pStringItem = dynamic_cast<const SfxStringItem*>(xItem1.get());
        CPPUNIT_ASSERT(pStringItem);

        CPPUNIT_ASSERT_EQUAL(OUString("English (USA);en-US"), pStringItem->GetValue());

        CPPUNIT_ASSERT(dynamic_cast< const SfxStringItem* >(xItem2.get()));
    }
}

void SdTiledRenderingTest::testLanguageAllText()
{
    // Load the document, which has a single shape, with Hungarian text.
    createDoc("language-all-text.odp");

    // Set the language to English for all text.
    uno::Sequence<beans::PropertyValue> aArgs = comphelper::InitPropertySequence({
        { "Language", uno::makeAny(OUString("Default_English (USA)")) },
    });
    comphelper::dispatchCommand(".uno:LanguageStatus", aArgs);
    Scheduler::ProcessEventsToIdle();

    // Assert that the shape text language was changed.
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                             uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShape(xPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xRun(
        getRunFromParagraph(0, getParagraphFromShape(0, xShape)), uno::UNO_QUERY);
    lang::Locale aLocale;
    xRun->getPropertyValue("CharLocale") >>= aLocale;
    // Without the accompanying fix in place, this test would have failed with 'Expected: en;
    // Actual: hu', as the shape text language was not set.
    CPPUNIT_ASSERT_EQUAL(OUString("en"), aLocale.Language);
}

void SdTiledRenderingTest::testDefaultView()
{
    // Load the document with notes view.
    SdXImpressDocument* pXImpressDocument = createDoc("notes-view.odp");
    sd::ViewShell* pView = pXImpressDocument->GetDocShell()->GetViewShell();
    {
        std::unique_ptr<SfxPoolItem> xItem1;
        std::unique_ptr<SfxPoolItem> xItem2;
        pView->GetViewFrame()->GetBindings().QueryState(SID_NORMAL_MULTI_PANE_GUI, xItem1);
        pView->GetViewFrame()->GetBindings().QueryState(SID_NOTES_MODE, xItem2);
        const SfxBoolItem* pImpressView = dynamic_cast< const SfxBoolItem* >(xItem1.get());
        const SfxBoolItem* pNotesView = dynamic_cast< const SfxBoolItem* >(xItem2.get());
        CPPUNIT_ASSERT(pImpressView);
        CPPUNIT_ASSERT(pNotesView);
        CPPUNIT_ASSERT_EQUAL(true, pImpressView->GetValue());
        CPPUNIT_ASSERT_EQUAL(false, pNotesView->GetValue());
    }
}

void SdTiledRenderingTest::testIMESupport()
{
    // Load the document with notes view.
    SdXImpressDocument* pXImpressDocument = createDoc("dummy.odp");
    VclPtr<vcl::Window> pDocWindow = pXImpressDocument->getDocWindow();
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    SdrObject* pObject = pViewShell->GetActualPage()->GetObj(0);
    SdrTextObj* pTextObj = static_cast<SdrTextObj*>(pObject);
    SdrView* pView = pViewShell->GetView();
    pView->MarkObj(pTextObj, pView->GetSdrPageView());
    SfxStringItem aInputString(SID_ATTR_CHAR, "x");
    pViewShell->GetViewFrame()->GetDispatcher()->ExecuteList(SID_ATTR_CHAR,
                                                             SfxCallMode::SYNCHRON, { &aInputString });

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

    // the cursor should be at position 3rd
    EditView& rEditView = pView->GetTextEditOutlinerView()->GetEditView();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3), rEditView.GetSelection().nStartPos);

    ESelection aWordSelection(0, 0, 0, 3); // start para, start char, end para, end char.
    rEditView.SetSelection(aWordSelection);
    // content contains only the last IME composition, not all
    CPPUNIT_ASSERT_EQUAL(OUString("x" + aInputs[aInputs.size() - 1]), rEditView.GetSelected());
}

void SdTiledRenderingTest::testTdf115783()
{
    // Load the document.
    SdXImpressDocument* pXImpressDocument = createDoc("tdf115783.fodp");
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    SdPage* pActualPage = pViewShell->GetActualPage();
    SdrObject* pObject = pActualPage->GetObj(0);
    auto pTableObject = dynamic_cast<sdr::table::SdrTableObj*>(pObject);
    CPPUNIT_ASSERT(pTableObject);
    SdrView* pView = pViewShell->GetView();
    pView->MarkObj(pTableObject, pView->GetSdrPageView());

    // Create a cell selection and set font height.
    // Go to the end of the B1 cell.
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_LEFT);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_LEFT);
    // Create a B1->C1 cell selection.
    const int nShiftRight = KEY_SHIFT + KEY_RIGHT;
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, nShiftRight);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, nShiftRight);
    uno::Sequence<beans::PropertyValue> aArgs = comphelper::InitPropertySequence({
        { "FontHeight.Height", uno::makeAny(static_cast<float>(12)) },
    });
    comphelper::dispatchCommand(".uno:FontHeight", aArgs);
    Scheduler::ProcessEventsToIdle();

    // Create a text selection on the B1 cell.
    pTableObject->setActiveCell(sdr::table::CellPos(1, 0));
    pView->SdrBeginTextEdit(pTableObject);
    EditView& rEditView = pView->GetTextEditOutlinerView()->GetEditView();
    // Start para, start char, end para, end char.
    rEditView.SetSelection(ESelection(0, 0, 0, 5));
    CPPUNIT_ASSERT_EQUAL(OUString("hello"), rEditView.GetSelected());

    // Copy selection, paste at the start of the cell.
    aArgs = {};
    comphelper::dispatchCommand(".uno:Copy", aArgs);
    Scheduler::ProcessEventsToIdle();
    rEditView.SetSelection(ESelection(0, 0, 0, 0));
    aArgs = {};
    comphelper::dispatchCommand(".uno:Paste", aArgs);
    Scheduler::ProcessEventsToIdle();
    pView->SdrEndTextEdit();

    // And now verify that the cell has the correct font size.
    uno::Reference<table::XCellRange> xTable = pTableObject->getTable();
    CPPUNIT_ASSERT(xTable.is());
    uno::Reference<text::XTextRange> xCell(xTable->getCellByPosition(1, 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xCell.is());
    uno::Reference<container::XEnumerationAccess> xText(xCell->getText(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xText.is());
    uno::Reference<container::XEnumerationAccess> xParagraph(
        xText->createEnumeration()->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xParagraph.is());
    uno::Reference<text::XTextRange> xPortion(xParagraph->createEnumeration()->nextElement(),
                                              uno::UNO_QUERY);
    CPPUNIT_ASSERT(xPortion.is());
    // This failed, it was only "hello" as the paragraph had 2 portions: a
    // "hello" with 12pt size and a "hello" with 18pt.
    CPPUNIT_ASSERT_EQUAL(OUString("hellohello"), xPortion->getString());
    uno::Reference<beans::XPropertySet> xPropertySet(xPortion, uno::UNO_QUERY);
    int nHeight = xPropertySet->getPropertyValue("CharHeight").get<float>();
    // Make sure that the single font size for the cell is the expected one.
    CPPUNIT_ASSERT_EQUAL(12, nHeight);
}

void SdTiledRenderingTest::testPasteTextOnSlide()
{
    // Load the document.
    SdXImpressDocument* pXImpressDocument = createDoc("paste_text_onslide.odp");
    CPPUNIT_ASSERT(pXImpressDocument);

    // select second text object
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::TAB);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::TAB);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::TAB);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::TAB);
    Scheduler::ProcessEventsToIdle();

    // step into text editing
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, '1', 0);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, '1', 0);
    Scheduler::ProcessEventsToIdle();

    // select full text
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_LEFT | KEY_SHIFT);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_LEFT | KEY_SHIFT);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_LEFT | KEY_SHIFT);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_LEFT | KEY_SHIFT);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_LEFT | KEY_SHIFT);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_LEFT | KEY_SHIFT);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_LEFT | KEY_SHIFT);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_LEFT | KEY_SHIFT);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_LEFT | KEY_SHIFT);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_LEFT | KEY_SHIFT);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_LEFT | KEY_SHIFT);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_LEFT | KEY_SHIFT);
    Scheduler::ProcessEventsToIdle();

    // Copy some text
    comphelper::dispatchCommand(".uno:Copy", uno::Sequence<beans::PropertyValue>());
    Scheduler::ProcessEventsToIdle();

    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::ESCAPE);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::ESCAPE);
    Scheduler::ProcessEventsToIdle();

    // Paste onto the slide
    comphelper::dispatchCommand(".uno:Paste", uno::Sequence<beans::PropertyValue>());
    Scheduler::ProcessEventsToIdle();

    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::ESCAPE);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::ESCAPE);
    Scheduler::ProcessEventsToIdle();

    // Check the position of the newly added text shape, created for pasted text
    SdPage* pActualPage = pXImpressDocument->GetDocShell()->GetViewShell()->GetActualPage();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), pActualPage->GetObjCount());
    SdrObject* pObject = pActualPage->GetObj(2);
    CPPUNIT_ASSERT(pObject);
    SdrTextObj* pTextObj = dynamic_cast<SdrTextObj*>(pObject);
    CPPUNIT_ASSERT(pTextObj);
    CPPUNIT_ASSERT_EQUAL(OBJ_TEXT, pTextObj->GetObjIdentifier());
    // This test is unreliable: it gives alternating results for the following coordinates.
    // As a compromise, instead of disabling it altogether, we allow for both sets of values.
    const Point aPos = pTextObj->GetLastBoundRect().TopLeft();
    if (aPos.getX() < 10000)
    {
        // We get this with 'make CppunitTest_sd_tiledrendering'
        CPPUNIT_ASSERT_DOUBLES_EQUAL(static_cast<tools::Long>(6739), aPos.getX(), 100);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(static_cast<tools::Long>(6822), aPos.getY(), 100);
    }
    else
    {
        // We get this with 'make check'
        CPPUNIT_ASSERT_DOUBLES_EQUAL(static_cast<tools::Long>(12990), aPos.getX(), 100);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(static_cast<tools::Long>(7393), aPos.getY(), 100);
    }
}

void SdTiledRenderingTest::testTdf115873()
{
    // Initialize the navigator.
    SdXImpressDocument* pXImpressDocument = createDoc("tdf115873.fodp");
    SfxViewShell* pViewShell = SfxViewShell::Current();
    CPPUNIT_ASSERT(pViewShell);
    SfxBindings& rBindings = pViewShell->GetViewFrame()->GetBindings();
    ScopedVclPtrInstance<SdNavigatorWin> pNavigator(nullptr, &rBindings);
    pNavigator->InitTreeLB(pXImpressDocument->GetDoc());
    pNavigator->Show();
    SdPageObjsTLV& rObjects = pNavigator->GetObjects();
    rObjects.SelectEntry(u"Slide 1");
    rObjects.Select();
    sd::ViewShell* pSdViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    SdrView* pSdrView = pSdViewShell->GetView();
    pSdrView->UnmarkAllObj(pSdrView->GetSdrPageView());

    // Make sure that no shapes are selected.
    const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), rMarkList.GetMarkCount());

    // Single-click with the mouse.
    MouseEvent aMouseEvent(Point(0, 0), /*nClicks=*/1, MouseEventModifiers::NONE, MOUSE_LEFT);
    rObjects.MousePressHdl(aMouseEvent);
    rObjects.SelectEntry(u"Rectangle");
    rObjects.Select();
    rObjects.MouseReleaseHdl(aMouseEvent);
    Scheduler::ProcessEventsToIdle();
    // This failed, single-click did not result in a shape selection (only
    // double-click did).
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rMarkList.GetMarkCount());
}

void SdTiledRenderingTest::testTdf115873Group()
{
    // Initialize the navigator.
    SdXImpressDocument* pXImpressDocument = createDoc("tdf115873-group.fodp");
    SfxViewShell* pViewShell = SfxViewShell::Current();
    CPPUNIT_ASSERT(pViewShell);
    SfxBindings& rBindings = pViewShell->GetViewFrame()->GetBindings();
    ScopedVclPtrInstance<SdNavigatorWin> pNavigator(nullptr, &rBindings);
    pNavigator->InitTreeLB(pXImpressDocument->GetDoc());
    SdPageObjsTLV& rObjects = pNavigator->GetObjects();
    // This failed, Fill() and IsEqualToDoc() were out of sync for group
    // shapes.
    CPPUNIT_ASSERT(rObjects.IsEqualToDoc(pXImpressDocument->GetDoc()));
}

void SdTiledRenderingTest::testCutSelectionChange()
{
    // Load the document.
    SdXImpressDocument* pXImpressDocument = createDoc("cut_selection_change.odp");
    CPPUNIT_ASSERT(pXImpressDocument);

    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    pViewShell->GetViewShellBase().registerLibreOfficeKitViewCallback(&SdTiledRenderingTest::callback, this);
    Scheduler::ProcessEventsToIdle();

    // Select first text object
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::TAB);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::TAB);
    Scheduler::ProcessEventsToIdle();

    // step into text editing
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, '1', 0);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, '1', 0);
    Scheduler::ProcessEventsToIdle();

    // select some text
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_LEFT | KEY_SHIFT);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_LEFT | KEY_SHIFT);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_LEFT | KEY_SHIFT);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_LEFT | KEY_SHIFT);
    Scheduler::ProcessEventsToIdle();

    // Check that we have a selection before cutting
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1), m_aSelection.size());

    // Cut the selected text
    comphelper::dispatchCommand(".uno:Cut", uno::Sequence<beans::PropertyValue>());
    Scheduler::ProcessEventsToIdle();

    // Selection is removed
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(0), m_aSelection.size());
}

void SdTiledRenderingTest::testRegenerateDiagram()
{
    // Load the document.
    SdXImpressDocument* pXImpressDocument = createDoc("regenerate-diagram.pptx");
    CPPUNIT_ASSERT(pXImpressDocument);

    SdPage* pActualPage = pXImpressDocument->GetDocShell()->GetViewShell()->GetActualPage();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), pActualPage->GetObj(0)->GetSubList()->GetObjCount());

    // select diagram
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::TAB);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::TAB);
    Scheduler::ProcessEventsToIdle();

    // enter group
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::F3);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::F3);
    Scheduler::ProcessEventsToIdle();

    // select shape and delete
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::TAB);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::TAB);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::TAB);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::TAB);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::DELETE);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::DELETE);
    Scheduler::ProcessEventsToIdle();

    // exit group
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::TAB);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::TAB);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_MOD1 | awt::Key::F3);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_MOD1 | awt::Key::F3);
    Scheduler::ProcessEventsToIdle();

    // select diagram
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::TAB);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::TAB);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), pActualPage->GetObj(0)->GetSubList()->GetObjCount());

    // regenerate diagram
    comphelper::dispatchCommand(".uno:RegenerateDiagram", uno::Sequence<beans::PropertyValue>());
    Scheduler::ProcessEventsToIdle();

    // diagram content (child shape count) should be the same as in the beginning
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), pActualPage->GetObj(0)->GetSubList()->GetObjCount());
}

void SdTiledRenderingTest::testInsertDeletePageInvalidation()
{
    // Load the document.
    SdXImpressDocument* pXImpressDocument = createDoc("dummy.odp");
    ViewCallback aView1;
    CPPUNIT_ASSERT_EQUAL(8, pXImpressDocument->getParts());

    // Insert slide
    aView1.m_bTilesInvalidated = false;
    aView1.m_aInvalidations.clear();
    comphelper::dispatchCommand(".uno:InsertPage", uno::Sequence<beans::PropertyValue>());
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(aView1.m_bTilesInvalidated);
    CPPUNIT_ASSERT_EQUAL(9, pXImpressDocument->getParts());
    CPPUNIT_ASSERT_EQUAL(size_t(9), aView1.m_aInvalidations.size());

    // Delete slide
    aView1.m_bTilesInvalidated = false;
    aView1.m_aInvalidations.clear();
    comphelper::dispatchCommand(".uno:DeletePage", uno::Sequence<beans::PropertyValue>());
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(aView1.m_bTilesInvalidated);
    CPPUNIT_ASSERT_EQUAL(8, pXImpressDocument->getParts());
    CPPUNIT_ASSERT_EQUAL(size_t(8), aView1.m_aInvalidations.size());
}

void SdTiledRenderingTest::testSpellOnlineRenderParameter()
{
    // Load the document.
    SdXImpressDocument* pXImpressDocument = createDoc("dummy.odp");
    bool bSet = pXImpressDocument->GetDoc()->GetOnlineSpell();

    uno::Sequence<beans::PropertyValue> aPropertyValues =
    {
        comphelper::InitPropertySequence({ { ".uno:SpellOnline", uno::makeAny(!bSet) } }),
    };
    pXImpressDocument->initializeForTiledRendering(aPropertyValues);
    CPPUNIT_ASSERT_EQUAL(!bSet, pXImpressDocument->GetDoc()->GetOnlineSpell());
}

CPPUNIT_TEST_SUITE_REGISTRATION(SdTiledRenderingTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
