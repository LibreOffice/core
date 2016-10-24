/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>
#include <unotest/macros_test.hxx>
#include <test/xmltesttools.hxx>
#include <boost/property_tree/json_parser.hpp>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <com/sun/star/frame/Desktop.hpp>
#include <comphelper/dispatchcommand.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/string.hxx>
#include <editeng/editids.hrc>
#include <editeng/editview.hxx>
#include <editeng/editobj.hxx>
#include <editeng/outliner.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/outlobj.hxx>
#include <osl/conditn.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/srchitem.hxx>
#include <comphelper/lok.hxx>
#include <svx/svdotable.hxx>
#include <svx/svdoutl.hxx>

#include <DrawDocShell.hxx>
#include <ViewShell.hxx>
#include <sdpage.hxx>
#include <unomodel.hxx>
#include <drawdoc.hxx>

using namespace css;

#if !defined(_WIN32) && !defined(MACOSX)
static const char* DATA_DIRECTORY = "/sd/qa/unit/tiledrendering/data/";
#endif

class SdTiledRenderingTest : public test::BootstrapFixture, public unotest::MacrosTest, public XmlTestTools
{
public:
    SdTiledRenderingTest();
    virtual void setUp() override;
    virtual void tearDown() override;

#if !defined(_WIN32) && !defined(MACOSX)
    void testRegisterCallback();
    void testPostKeyEvent();
    void testPostMouseEvent();
    void testSetTextSelection();
    void testGetTextSelection();
    void testSetGraphicSelection();
    void testResetSelection();
    void testSearch();
    void testSearchAll();
    void testSearchAllSelections();
    void testSearchAllNotifications();
    void testSearchAllFollowedBySearch();
    void testInsertDeletePage();
    void testInsertTable();
    void testPartHash();
    void testResizeTable();
    void testResizeTableColumn();
    void testTdf102223();
    void testTdf103083();
#endif

    CPPUNIT_TEST_SUITE(SdTiledRenderingTest);
#if !defined(_WIN32) && !defined(MACOSX)
    CPPUNIT_TEST(testRegisterCallback);
    CPPUNIT_TEST(testPostKeyEvent);
    CPPUNIT_TEST(testPostMouseEvent);
    CPPUNIT_TEST(testSetTextSelection);
    CPPUNIT_TEST(testGetTextSelection);
    CPPUNIT_TEST(testSetGraphicSelection);
    CPPUNIT_TEST(testResetSelection);
    CPPUNIT_TEST(testSearch);
    CPPUNIT_TEST(testSearchAll);
    CPPUNIT_TEST(testSearchAllSelections);
    CPPUNIT_TEST(testSearchAllNotifications);
    CPPUNIT_TEST(testSearchAllFollowedBySearch);
    CPPUNIT_TEST(testInsertDeletePage);
    CPPUNIT_TEST(testInsertTable);
    CPPUNIT_TEST(testPartHash);
    CPPUNIT_TEST(testResizeTable);
    CPPUNIT_TEST(testResizeTableColumn);
    CPPUNIT_TEST(testTdf102223);
    CPPUNIT_TEST(testTdf103083);
#endif
    CPPUNIT_TEST_SUITE_END();

private:
#if !defined(_WIN32) && !defined(MACOSX)
    SdXImpressDocument* createDoc(const char* pName);
    static void callback(int nType, const char* pPayload, void* pData);
    void callbackImpl(int nType, const char* pPayload);
    xmlDocPtr parseXmlDump();
#endif

    uno::Reference<lang::XComponent> mxComponent;
#if !defined(_WIN32) && !defined(MACOSX)
    Rectangle m_aInvalidation;
    std::vector<Rectangle> m_aSelection;
    bool m_bFound;
    sal_Int32 m_nPart;
    std::vector<OString> m_aSearchResultSelection;
    std::vector<int> m_aSearchResultPart;
    int m_nSelectionBeforeSearchResult;
    int m_nSelectionAfterSearchResult;

    /// For document size changed callback.
    osl::Condition m_aDocumentSizeCondition;
    xmlBufferPtr m_pXmlBuffer;
#endif
};

SdTiledRenderingTest::SdTiledRenderingTest()
#if !defined(_WIN32) && !defined(MACOSX)
    : m_bFound(true),
      m_nPart(0),
      m_nSelectionBeforeSearchResult(0),
      m_nSelectionAfterSearchResult(0),
      m_pXmlBuffer(nullptr)
#endif
{
}

void SdTiledRenderingTest::setUp()
{
    test::BootstrapFixture::setUp();

    mxDesktop.set(css::frame::Desktop::create(comphelper::getComponentContext(getMultiServiceFactory())));
}

void SdTiledRenderingTest::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

#if !defined(_WIN32) && !defined(MACOSX)
    if (m_pXmlBuffer)
        xmlBufferFree(m_pXmlBuffer);
#endif

    test::BootstrapFixture::tearDown();
}

#if !defined(_WIN32) && !defined(MACOSX)
SdXImpressDocument* SdTiledRenderingTest::createDoc(const char* pName)
{
    if (mxComponent.is())
        mxComponent->dispose();
    mxComponent = loadFromDesktop(m_directories.getURLFromSrc(DATA_DIRECTORY) + OUString::createFromAscii(pName), "com.sun.star.presentation.PresentationDocument");
    SdXImpressDocument* pImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pImpressDocument);
    pImpressDocument->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    return pImpressDocument;
}

void SdTiledRenderingTest::callback(int nType, const char* pPayload, void* pData)
{
    static_cast<SdTiledRenderingTest*>(pData)->callbackImpl(nType, pPayload);
}

static std::vector<OUString> lcl_convertSeparated(const OUString& rString, sal_Unicode nSeparator)
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

static void lcl_convertRectangle(const OUString& rString, Rectangle& rRectangle)
{
    uno::Sequence<OUString> aSeq = comphelper::string::convertCommaSeparated(rString);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(4), aSeq.getLength());
    rRectangle.setX(aSeq[0].toInt32());
    rRectangle.setY(aSeq[1].toInt32());
    rRectangle.setWidth(aSeq[2].toInt32());
    rRectangle.setHeight(aSeq[3].toInt32());
}

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
        for (const OUString& rString : lcl_convertSeparated(aPayload, static_cast<sal_Unicode>(';')))
        {
            Rectangle aRectangle;
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
        for (boost::property_tree::ptree::value_type& rValue : aTree.get_child("searchResultSelection"))
        {
            m_aSearchResultSelection.push_back(rValue.second.get<std::string>("rectangles").c_str());
            m_aSearchResultPart.push_back(std::atoi(rValue.second.get<std::string>("part").c_str()));
        }
    }
    break;
    }
}

xmlDocPtr SdTiledRenderingTest::parseXmlDump()
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

    return xmlParseMemory(reinterpret_cast<const char*>(xmlBufferContent(m_pXmlBuffer)), xmlBufferLength(m_pXmlBuffer));
}

void SdTiledRenderingTest::testRegisterCallback()
{
    SdXImpressDocument* pXImpressDocument = createDoc("dummy.odp");
    pXImpressDocument->registerCallback(&SdTiledRenderingTest::callback, this);
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();

    // Start text edit of the empty title shape.
    SdPage* pActualPage = pViewShell->GetActualPage();
    SdrObject* pObject = pActualPage->GetObj(0);
    SdrView* pView = pViewShell->GetView();
    pView->SdrBeginTextEdit(pObject);
    CPPUNIT_ASSERT(pView->GetTextEditObject());

    // Check that the top left 256x256px tile would be invalidated.
    CPPUNIT_ASSERT(!m_aInvalidation.IsEmpty());
    Rectangle aTopLeft(0, 0, 256*15, 256*15); // 1 px = 15 twips, assuming 96 DPI.
    CPPUNIT_ASSERT(m_aInvalidation.IsOver(aTopLeft));
}

void SdTiledRenderingTest::testPostKeyEvent()
{
    SdXImpressDocument* pXImpressDocument = createDoc("dummy.odp");
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    SdPage* pActualPage = pViewShell->GetActualPage();
    SdrObject* pObject = pActualPage->GetObj(0);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(OBJ_TITLETEXT), pObject->GetObjIdentifier());
    SdrTextObj* pTextObj = static_cast<SdrTextObj*>(pObject);
    SdrView* pView = pViewShell->GetView();
    pView->MarkObj(pTextObj, pView->GetSdrPageView());
    SfxStringItem aInputString(SID_ATTR_CHAR, "x");
    pViewShell->GetViewFrame()->GetDispatcher()->ExecuteList(SID_ATTR_CHAR,
            SfxCallMode::SYNCHRON, { &aInputString });

    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'x', 0);
    pXImpressDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 'x', 0);

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
    comphelper::LibreOfficeKit::setActive();
    SdXImpressDocument* pXImpressDocument = createDoc("dummy.odp");
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    SdPage* pActualPage = pViewShell->GetActualPage();
    SdrObject* pObject = pActualPage->GetObj(0);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(OBJ_TITLETEXT), pObject->GetObjIdentifier());
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
    Point aPosition = pCursor->GetPos();
    aPosition.setX(aPosition.getX() - 1000);
    pXImpressDocument->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONDOWN, convertMm100ToTwip(aPosition.getX()), convertMm100ToTwip(aPosition.getY()), 1);
    pXImpressDocument->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONUP, convertMm100ToTwip(aPosition.getX()), convertMm100ToTwip(aPosition.getY()), 1);
    CPPUNIT_ASSERT(pView->GetTextEditObject());
    // The new cursor position must be before the first word.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), rEditView.GetSelection().nStartPos);
    comphelper::LibreOfficeKit::setActive(false);
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
    OString aUsedFormat;
    CPPUNIT_ASSERT_EQUAL(OString("Shape"), pXImpressDocument->getTextSelection("text/plain;charset=utf-8", aUsedFormat));

    // Make sure returned RTF is not empty.
    CPPUNIT_ASSERT(!OString(pXImpressDocument->getTextSelection("text/rtf", aUsedFormat)).isEmpty());
}

void SdTiledRenderingTest::testSetGraphicSelection()
{
    SdXImpressDocument* pXImpressDocument = createDoc("shape.odp");
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    SdPage* pPage = pViewShell->GetActualPage();
    SdrObject* pObject = pPage->GetObj(0);
    // Make sure the rectangle has 8 handles: at each corner and at the center of each edge.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(8), pObject->GetHdlCount());
    // Take the bottom center one.
    SdrHdl* pHdl = pObject->GetHdl(6);
    CPPUNIT_ASSERT_EQUAL(HDL_LOWER, pHdl->GetKind());
    Rectangle aShapeBefore = pObject->GetSnapRect();
    // Resize.
    pXImpressDocument->setGraphicSelection(LOK_SETGRAPHICSELECTION_START, convertMm100ToTwip(pHdl->GetPos().getX()), convertMm100ToTwip(pHdl->GetPos().getY()));
    pXImpressDocument->setGraphicSelection(LOK_SETGRAPHICSELECTION_END, convertMm100ToTwip(pHdl->GetPos().getX()), convertMm100ToTwip(pHdl->GetPos().getY() + 1000));
    Rectangle aShapeAfter = pObject->GetSnapRect();
    // Check that a resize happened, but aspect ratio is not kept.
    CPPUNIT_ASSERT_EQUAL(aShapeBefore.getWidth(), aShapeAfter.getWidth());
    CPPUNIT_ASSERT(aShapeBefore.getHeight() < aShapeAfter.getHeight());
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

static void lcl_search(const OUString& rKey, bool bFindAll = false)
{
    uno::Sequence<beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence(
    {
        {"SearchItem.SearchString", uno::makeAny(rKey)},
        {"SearchItem.Backward", uno::makeAny(false)},
        {"SearchItem.Command", uno::makeAny(static_cast<sal_uInt16>(bFindAll ? SvxSearchCmd::FIND_ALL : SvxSearchCmd::FIND))},
    }));
    comphelper::dispatchCommand(".uno:ExecuteSearch", aPropertyValues);
}

void SdTiledRenderingTest::testSearch()
{
    comphelper::LibreOfficeKit::setActive();
    SdXImpressDocument* pXImpressDocument = createDoc("dummy.odp");
    pXImpressDocument->registerCallback(&SdTiledRenderingTest::callback, this);
    uno::Reference<container::XIndexAccess> xDrawPage(pXImpressDocument->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    xShape->setString("Aaa bbb.");

    lcl_search("bbb");

    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    SdrView* pView = pViewShell->GetView();
    EditView& rEditView = pView->GetTextEditOutlinerView()->GetEditView();
    // Did we indeed manage to select the second word?
    CPPUNIT_ASSERT_EQUAL(OUString("bbb"), rEditView.GetSelected());

    // Did the selection callback fire?
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1), m_aSelection.size());

    // Search for something on the second slide, and make sure that the set-part callback fired.
    lcl_search("bbb");
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), m_nPart);
    CPPUNIT_ASSERT_EQUAL(true, m_bFound);
    // This was 0; should be 1 match for "find".
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1), m_aSearchResultSelection.size());
    // Result is on the second slide.
    CPPUNIT_ASSERT_EQUAL(1, m_aSearchResultPart[0]);

    // This should trigger the not-found callback.
    lcl_search("ccc");
    CPPUNIT_ASSERT_EQUAL(false, m_bFound);
    comphelper::LibreOfficeKit::setActive(false);
}

void SdTiledRenderingTest::testSearchAll()
{
    comphelper::LibreOfficeKit::setActive();
    SdXImpressDocument* pXImpressDocument = createDoc("search-all.odp");
    pXImpressDocument->registerCallback(&SdTiledRenderingTest::callback, this);

    lcl_search("match", /*bFindAll=*/true);

    OString aUsedFormat;
    // This was empty: find-all did not highlight the first match.
    CPPUNIT_ASSERT_EQUAL(OString("match"), pXImpressDocument->getTextSelection("text/plain;charset=utf-8", aUsedFormat));

    // We're on the first slide, search for something on the second slide and make sure we get a SET_PART.
    m_nPart = 0;
    lcl_search("second", /*bFindAll=*/true);
    // This was 0: no SET_PART was emitted.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), m_nPart);
    comphelper::LibreOfficeKit::setActive(false);
}

void SdTiledRenderingTest::testSearchAllSelections()
{
    comphelper::LibreOfficeKit::setActive();
    SdXImpressDocument* pXImpressDocument = createDoc("search-all.odp");
    pXImpressDocument->registerCallback(&SdTiledRenderingTest::callback, this);

    lcl_search("third", /*bFindAll=*/true);
    // Make sure this is found on the 3rd slide.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), m_nPart);
    // This was 1: only the first match was highlighted.
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(2), m_aSelection.size());
    comphelper::LibreOfficeKit::setActive(false);
}

void SdTiledRenderingTest::testSearchAllNotifications()
{
    comphelper::LibreOfficeKit::setActive();
    SdXImpressDocument* pXImpressDocument = createDoc("search-all.odp");
    pXImpressDocument->registerCallback(&SdTiledRenderingTest::callback, this);

    lcl_search("third", /*bFindAll=*/true);
    // Make sure that we get no notifications about selection changes during search.
    CPPUNIT_ASSERT_EQUAL(0, m_nSelectionBeforeSearchResult);
    // But we do get the selection of the first hit.
    CPPUNIT_ASSERT(m_nSelectionAfterSearchResult > 0);
    comphelper::LibreOfficeKit::setActive(false);
}

void SdTiledRenderingTest::testSearchAllFollowedBySearch()
{
    comphelper::LibreOfficeKit::setActive();
    SdXImpressDocument* pXImpressDocument = createDoc("search-all.odp");
    pXImpressDocument->registerCallback(&SdTiledRenderingTest::callback, this);

    lcl_search("third", /*bFindAll=*/true);
    lcl_search("match" /*,bFindAll=false*/);

    OString aUsedFormat;
    // This used to give wrong result: 'search' after 'search all' still
    // returned 'third'
    CPPUNIT_ASSERT_EQUAL(OString("match"), pXImpressDocument->getTextSelection("text/plain;charset=utf-8", aUsedFormat));
}

namespace
{

std::vector<OUString> getCurrentParts(SdXImpressDocument* pDocument)
{
    int parts = pDocument->getParts();
    std::vector<OUString> result;

    for (int i = 0; i < parts; i++)
    {
        result.push_back(pDocument->getPartName(i));
    }

    return result;
}

}

void SdTiledRenderingTest::testInsertDeletePage()
{
    comphelper::LibreOfficeKit::setActive();
    SdXImpressDocument* pXImpressDocument = createDoc("insert-delete.odp");
    pXImpressDocument->registerCallback(&SdTiledRenderingTest::callback, this);

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
    CPPUNIT_ASSERT(pDoc->GetSdPageCount(PK_STANDARD) == 1);

    uno::Sequence<beans::PropertyValue> aArgs;

    // Insert slides
    m_aDocumentSizeCondition.reset();
    for (unsigned it = 1; it <= 10; it++)
        comphelper::dispatchCommand(".uno:InsertPage", aArgs);

    TimeValue aTimeValue = { 2 , 0 }; // 2 seconds max
    osl::Condition::Result aResult = m_aDocumentSizeCondition.wait(aTimeValue);
    CPPUNIT_ASSERT_EQUAL(aResult, osl::Condition::result_ok);

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

    aResult = m_aDocumentSizeCondition.wait(aTimeValue);
    CPPUNIT_ASSERT_EQUAL(aResult, osl::Condition::result_ok);

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

    aResult = m_aDocumentSizeCondition.wait(aTimeValue);
    CPPUNIT_ASSERT_EQUAL(aResult, osl::Condition::result_ok);

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

    aResult = m_aDocumentSizeCondition.wait(aTimeValue);
    CPPUNIT_ASSERT_EQUAL(aResult, osl::Condition::result_ok);

    // Verify deleted slides
    aPageList = getCurrentParts(pXImpressDocument);
    CPPUNIT_ASSERT_EQUAL(aPageList.size(), aDeleted.size());
    for (auto it1 = aPageList.begin(), it2 = aDeleted.begin(); it1 != aPageList.end(); ++it1, ++it2)
    {
        CPPUNIT_ASSERT_EQUAL(*it1, *it2);
    }

    // the document has 1 slide
    CPPUNIT_ASSERT(pDoc->GetSdPageCount(PK_STANDARD) == 1);

    comphelper::LibreOfficeKit::setActive(false);
}

void SdTiledRenderingTest::testInsertTable()
{
    comphelper::LibreOfficeKit::setActive();
    SdXImpressDocument* pXImpressDocument = createDoc("dummy.odp");

    uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence(
    {
        { "Rows", uno::makeAny(sal_Int32(3)) },
        { "Columns", uno::makeAny(sal_Int32(5)) }
    }
    ));

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

    comphelper::LibreOfficeKit::setActive(false);
}

void SdTiledRenderingTest::testPartHash()
{
    comphelper::LibreOfficeKit::setActive();
    SdXImpressDocument* pDoc = createDoc("dummy.odp");

    int nParts = pDoc->getParts();
    for (int it = 0; it < nParts; it++)
    {
        CPPUNIT_ASSERT(!pDoc->getPartHash(it).isEmpty());
    }

    // check part that it does not exists
    CPPUNIT_ASSERT(pDoc->getPartHash(100).isEmpty());
    comphelper::LibreOfficeKit::setActive(false);
}

void SdTiledRenderingTest::testResizeTable()
{
    // Load the document.
    comphelper::LibreOfficeKit::setActive();
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
    uno::Reference<table::XColumnRowRange> xTable(pTableObject->getTable(), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xRows(xTable->getRows(), uno::UNO_QUERY);
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
    comphelper::LibreOfficeKit::setActive(false);
}

void SdTiledRenderingTest::testResizeTableColumn()
{
    // Load the document.
    comphelper::LibreOfficeKit::setActive();
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
    xmlDocPtr pXmlDoc = parseXmlDump();
    OString aPrefix = "/sdDrawDocument/sdrModel/sdPage/sdrObjList/sdrTableObj/sdrTableObjImpl/tableLayouter/columns/";
    sal_Int32 nExpectedColumn1 = getXPath(pXmlDoc, aPrefix + "layout[1]", "size").toInt32();
    sal_Int32 nExpectedColumn2 = getXPath(pXmlDoc, aPrefix + "layout[2]", "size").toInt32();
    xmlFreeDoc(pXmlDoc);
    pXmlDoc = nullptr;

    // Resize the left column, decrease its width by 1 cm.
    Point aInnerRowEdge = pObject->GetSnapRect().Center();
    pXImpressDocument->setGraphicSelection(LOK_SETGRAPHICSELECTION_START, convertMm100ToTwip(aInnerRowEdge.getX()), convertMm100ToTwip(aInnerRowEdge.getY()));
    pXImpressDocument->setGraphicSelection(LOK_SETGRAPHICSELECTION_END, convertMm100ToTwip(aInnerRowEdge.getX() - 1000), convertMm100ToTwip(aInnerRowEdge.getY()));

    // Remember the resized column widths.
    pXmlDoc = parseXmlDump();
    sal_Int32 nResizedColumn1 = getXPath(pXmlDoc, aPrefix + "layout[1]", "size").toInt32();
    CPPUNIT_ASSERT(nResizedColumn1 < nExpectedColumn1);
    sal_Int32 nResizedColumn2 = getXPath(pXmlDoc, aPrefix + "layout[2]", "size").toInt32();
    CPPUNIT_ASSERT(nResizedColumn2 > nExpectedColumn2);
    xmlFreeDoc(pXmlDoc);
    pXmlDoc = nullptr;

    // Now undo the resize.
    pXImpressDocument->GetDocShell()->GetUndoManager()->Undo();

    // Check the undo result.
    pXmlDoc = parseXmlDump();
    sal_Int32 nActualColumn1 = getXPath(pXmlDoc, aPrefix + "layout[1]", "size").toInt32();
    // Expected was 7049, actual was 6048, i.e. the first column width after undo was 1cm smaller than expected.
    CPPUNIT_ASSERT_EQUAL(nExpectedColumn1, nActualColumn1);
    sal_Int32 nActualColumn2 = getXPath(pXmlDoc, aPrefix + "layout[2]", "size").toInt32();
    CPPUNIT_ASSERT_EQUAL(nExpectedColumn2, nActualColumn2);
    xmlFreeDoc(pXmlDoc);
    pXmlDoc = nullptr;
    comphelper::LibreOfficeKit::setActive(false);
}

void SdTiledRenderingTest::testTdf102223()
{
    // Load the document.
    comphelper::LibreOfficeKit::setActive();
    SdXImpressDocument* pXImpressDocument = createDoc("tdf102223.odp");
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    SdPage* pActualPage = pViewShell->GetActualPage();
    auto pTableObject = dynamic_cast<sdr::table::SdrTableObj*>(pActualPage->GetObj(2));
    CPPUNIT_ASSERT(pTableObject);
    SdrView* pView = pViewShell->GetView();

    // select contents of cell
    Rectangle aRect = pTableObject->GetCurrentBoundRect();
    pXImpressDocument->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONDOWN,
                                      convertMm100ToTwip(aRect.getX() + 2), convertMm100ToTwip(aRect.getY() + 2),
                                      1, MOUSE_LEFT, 0);
    pXImpressDocument->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONUP,
                                      convertMm100ToTwip(aRect.getX() + 2), convertMm100ToTwip(aRect.getY() + 2),
                                      1, MOUSE_LEFT, 0);
    pView->SdrBeginTextEdit(pTableObject);
    CPPUNIT_ASSERT(pView->GetTextEditObject());
    EditView& rEditView = pView->GetTextEditOutlinerView()->GetEditView();
    rEditView.SetSelection(ESelection(0, 0, 0, 3)); // start para, start char, end para, end char.
    CPPUNIT_ASSERT_EQUAL(OUString("Red"), rEditView.GetSelected());
    const SvxFontHeightItem& rItem = static_cast<const SvxFontHeightItem&>(rEditView.GetAttribs().Get(EE_CHAR_FONTHEIGHT));
    CPPUNIT_ASSERT_EQUAL((int)1411, (int)rItem.GetHeight());

    // cut contents of cell
    uno::Sequence<beans::PropertyValue> aArgs;
    comphelper::dispatchCommand(".uno:Cut", aArgs);

    pView->SdrBeginTextEdit(pTableObject);
    CPPUNIT_ASSERT(pView->GetTextEditObject());
    EditView& rEditView2 = pView->GetTextEditOutlinerView()->GetEditView();
    rEditView2.SetSelection(ESelection(0, 0, 0, 1)); // start para, start char, end para, end char.
    const SvxFontHeightItem& rItem2 = static_cast<const SvxFontHeightItem&>(rEditView2.GetAttribs().Get(EE_CHAR_FONTHEIGHT));
    CPPUNIT_ASSERT_EQUAL((int)1411, (int)rItem2.GetHeight());

    comphelper::LibreOfficeKit::setActive(false);
}

/**
 * tests a cut/paste bug around bullet items in a list
 */
void SdTiledRenderingTest::testTdf103083()
{
    // Load the document.
    comphelper::LibreOfficeKit::setActive();
    SdXImpressDocument* pXImpressDocument = createDoc("tdf103083.fodp");
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    SdPage* pActualPage = pViewShell->GetActualPage();

    SdrObject* pObject1 = pActualPage->GetObj(1);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(OBJ_OUTLINETEXT), pObject1->GetObjIdentifier());
    SdrTextObj* pTextObject = static_cast<SdrTextObj*>(pObject1);

    SdrView* pView = pViewShell->GetView();

    // select contents of bullet item
    Rectangle aRect = pTextObject->GetCurrentBoundRect();
    pXImpressDocument->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONDOWN,
                                      convertMm100ToTwip(aRect.getX() + 2), convertMm100ToTwip(aRect.getY() + 2),
                                      1, MOUSE_LEFT, 0);
    pXImpressDocument->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONUP,
                                      convertMm100ToTwip(aRect.getX() + 2), convertMm100ToTwip(aRect.getY() + 2),
                                      1, MOUSE_LEFT, 0);
    pView->SdrBeginTextEdit(pTextObject);
    CPPUNIT_ASSERT(pView->GetTextEditObject());
    EditView& rEditView = pView->GetTextEditOutlinerView()->GetEditView();
    rEditView.SetSelection(ESelection(2, 0, 2, 33)); // start para, start char, end para, end char.
    CPPUNIT_ASSERT_EQUAL(OUString("They have all the same formatting"), rEditView.GetSelected());
    SdrOutliner* pOutliner = pView->GetTextEditOutliner();
    CPPUNIT_ASSERT_EQUAL(OUString("No-Logo Content~LT~Gliederung 2"),
                         pOutliner->GetStyleSheet(2)->GetName());
    const SfxItemSet& rParagraphItemSet1 = pTextObject->GetOutlinerParaObject()->GetTextObject().GetParaAttribs(2);
    CPPUNIT_ASSERT_EQUAL((sal_uInt16)3, rParagraphItemSet1.Count());

    // cut contents of bullet item
    comphelper::dispatchCommand(".uno:Cut", uno::Sequence<beans::PropertyValue>());

    CPPUNIT_ASSERT(pView->GetTextEditObject());
    EditView& rEditView2 = pView->GetTextEditOutlinerView()->GetEditView();
    rEditView2.SetSelection(ESelection(2, 0, 2, 10)); // start para, start char, end para, end char.
    CPPUNIT_ASSERT_EQUAL(OUString(""), rEditView2.GetSelected());

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

    const SfxItemSet& rParagraphItemSet2 = pTextObject->GetOutlinerParaObject()->GetTextObject().GetParaAttribs(2);
    CPPUNIT_ASSERT_EQUAL((sal_uInt16)3, rParagraphItemSet2.Count());

    comphelper::LibreOfficeKit::setActive(false);
}

#endif

CPPUNIT_TEST_SUITE_REGISTRATION(SdTiledRenderingTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
