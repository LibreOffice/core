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
#include <com/sun/star/frame/DispatchHelper.hpp>
#include <comphelper/dispatchcommand.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/string.hxx>
#include <editeng/editids.hrc>
#include <editeng/editview.hxx>
#include <editeng/outliner.hxx>
#include <osl/conditn.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/srchitem.hxx>

#include <comphelper/lok.hxx>
#include <comphelper/propertyvalue.hxx>
#include <sfx2/lokhelper.hxx>
#include <svx/svdpage.hxx>

#include <tabvwsh.hxx>
#include <docsh.hxx>
#include <document.hxx>
#include <docuno.hxx>

using namespace css;

namespace
{

const char* DATA_DIRECTORY = "/sc/qa/unit/tiledrendering/data/";

class ScTiledRenderingTest : public test::BootstrapFixture, public unotest::MacrosTest, public XmlTestTools
{
public:
    ScTiledRenderingTest();
    virtual void setUp() SAL_OVERRIDE;
    virtual void tearDown() SAL_OVERRIDE;

    void testRowColumnSelections();
    void testSortAscendingDescending();
    void testPartHash();
    void testDocumentSize();
    void testEmptyColumnSelection();
    void testViewCursors();
    void testTextViewSelection();
    void testDocumentSizeChanged();
    void testViewLock();

    CPPUNIT_TEST_SUITE(ScTiledRenderingTest);
    CPPUNIT_TEST(testRowColumnSelections);
    CPPUNIT_TEST(testSortAscendingDescending);
    CPPUNIT_TEST(testPartHash);
    CPPUNIT_TEST(testDocumentSize);
    CPPUNIT_TEST(testEmptyColumnSelection);
    CPPUNIT_TEST(testViewCursors);
    CPPUNIT_TEST(testTextViewSelection);
    CPPUNIT_TEST(testDocumentSizeChanged);
    CPPUNIT_TEST(testViewLock);
    CPPUNIT_TEST_SUITE_END();

private:
    ScModelObj* createDoc(const char* pName);
    static void callback(int nType, const char* pPayload, void* pData);
    void callbackImpl(int nType, const char* pPayload);

    /// document size changed callback.
    osl::Condition m_aDocSizeCondition;
    Size m_aDocumentSize;

    uno::Reference<lang::XComponent> mxComponent;
};

ScTiledRenderingTest::ScTiledRenderingTest()
{
}

void ScTiledRenderingTest::setUp()
{
    test::BootstrapFixture::setUp();

    mxDesktop.set(css::frame::Desktop::create(comphelper::getComponentContext(getMultiServiceFactory())));
}

void ScTiledRenderingTest::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

ScModelObj* ScTiledRenderingTest::createDoc(const char* pName)
{
    if (mxComponent.is())
        mxComponent->dispose();
    mxComponent = loadFromDesktop(m_directories.getURLFromSrc(DATA_DIRECTORY) + OUString::createFromAscii(pName), "com.sun.star.sheet.SpreadsheetDocument");
    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    return pModelObj;
}

void ScTiledRenderingTest::callback(int nType, const char* pPayload, void* pData)
{
    static_cast<ScTiledRenderingTest*>(pData)->callbackImpl(nType, pPayload);
}

/* TODO when needed...
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
*/

void ScTiledRenderingTest::callbackImpl(int nType, const char* pPayload)
{
    switch (nType)
    {
    case LOK_CALLBACK_DOCUMENT_SIZE_CHANGED:
    {
        OString aPayload(pPayload);
        sal_Int32 nIndex = 0;
        OString aToken = aPayload.getToken(0, ',', nIndex);
        m_aDocumentSize.setWidth(aToken.toInt32());
        aToken = aPayload.getToken(0, ',', nIndex);
        m_aDocumentSize.setHeight(aToken.toInt32());
        m_aDocSizeCondition.set();
    }
    break;
    }
}

void ScTiledRenderingTest::testRowColumnSelections()
{
    comphelper::LibreOfficeKit::setActive();
    ScModelObj* pModelObj = createDoc("select-row-cols.ods");
    uno::Sequence<beans::PropertyValue> aArgs(2);

    // Select the 5th row with no modifier
    aArgs[0].Name = OUString::fromUtf8("Row");
    aArgs[0].Value <<= static_cast<sal_Int32>(5 - 1);
    aArgs[1].Name = OUString::fromUtf8("Modifier");
    aArgs[1].Value <<= static_cast<sal_uInt16>(0);
    comphelper::dispatchCommand(".uno:SelectRow", aArgs);

    // Check if it is selected
    OString aUsedMimeType;
    OString aResult = pModelObj->getTextSelection("text/plain;charset=utf-8", aUsedMimeType);
    OString aExpected("1\t2\t3\t4\t5\t6\t7\t8\t9\t10\t11\t12\t13\t14\t15\t16\t17\t18\t19\t20\t21\n");
    CPPUNIT_ASSERT_EQUAL(aExpected, aResult);

    // Select the 10th row with shift modifier
    aArgs[0].Name = OUString::fromUtf8("Row");
    aArgs[0].Value <<= static_cast<sal_Int32>(10 - 1);
    aArgs[1].Name = OUString::fromUtf8("Modifier");
    aArgs[1].Value <<= static_cast<sal_uInt16>(KEY_SHIFT);
    comphelper::dispatchCommand(".uno:SelectRow", aArgs);

    // Check if all the rows from 5th to 10th get selected
    aResult = pModelObj->getTextSelection("text/plain;charset=utf-8", aUsedMimeType);
    aExpected = "1\t2\t3\t4\t5\t6\t7\t8\t9\t10\t11\t12\t13\t14\t15\t16\t17\t18\t19\t20\t21\n2\t3\t4\t5\t6\t7\t8\t9\t10\t11\t12\t13\t14\t15\t16\t17\t18\t19\t20\t21\t22\n3\t4\t5\t6\t7\t8\t9\t10\t11\t12\t13\t14\t15\t16\t17\t18\t19\t20\t21\t22\t23\n4\t5\t6\t7\t8\t9\t10\t11\t12\t13\t14\t15\t16\t17\t18\t19\t20\t21\t22\t23\t24\n5\t6\t7\t8\t9\t10\t11\t12\t13\t14\t15\t16\t17\t18\t19\t20\t21\t22\t23\t24\t25\n6\t7\t8\t9\t10\t11\t12\t13\t14\t15\t16\t17\t18\t19\t20\t21\t22\t23\t24\t25\t26\n";
    CPPUNIT_ASSERT_EQUAL(aExpected, aResult);

    // Select the 10th row with ctrl modifier
    aArgs[0].Name = OUString::fromUtf8("Row");
    aArgs[0].Value <<= static_cast<sal_Int32>(13 - 1);
    aArgs[1].Name = OUString::fromUtf8("Modifier");
    aArgs[1].Value <<= static_cast<sal_uInt16>(KEY_MOD1);
    comphelper::dispatchCommand(".uno:SelectRow", aArgs);

    // When we copy this, we don't get anything useful, but we must not crash
    // (used to happen)
    aResult = pModelObj->getTextSelection("text/plain;charset=utf-8", aUsedMimeType);
    CPPUNIT_ASSERT_EQUAL(OString(), aResult);

    // TODO check that we really selected what we wanted here

    // Select Column 5 with ctrl modifier
    aArgs[0].Name = OUString::fromUtf8("Col");
    aArgs[0].Value <<= static_cast<sal_Int32>(5 - 1);
    aArgs[1].Name = OUString::fromUtf8("Modifier");
    aArgs[1].Value <<= static_cast<sal_uInt16>(KEY_MOD1);
    comphelper::dispatchCommand(".uno:SelectColumn", aArgs);

    // When we copy this, we don't get anything useful, but we must not crash
    // (used to happen)
    aResult = pModelObj->getTextSelection("text/plain;charset=utf-8", aUsedMimeType);
    CPPUNIT_ASSERT_EQUAL(OString(), aResult);

    // TODO check that we really selected what we wanted here

    // Test for deselection of already selected rows
    // First Deselect Row 13 because copy doesn't work for multiple selections
    aArgs[0].Name = OUString::fromUtf8("Row");
    aArgs[0].Value <<= static_cast<sal_Int32>(13 - 1);
    aArgs[1].Name = OUString::fromUtf8("Modifier");
    aArgs[1].Value <<= static_cast<sal_uInt16>(KEY_MOD1);
    comphelper::dispatchCommand(".uno:SelectRow", aArgs);

    // Deselect row 10
    aArgs[0].Name = OUString::fromUtf8("Row");
    aArgs[0].Value <<= static_cast<sal_Int32>(10 - 1);
    aArgs[1].Name = OUString::fromUtf8("Modifier");
    aArgs[1].Value <<= static_cast<sal_uInt16>(KEY_MOD1);
    comphelper::dispatchCommand(".uno:SelectRow", aArgs);

    // Click at row 6 holding shift
    aArgs[0].Name = OUString::fromUtf8("Row");
    aArgs[0].Value <<= static_cast<sal_Int32>(6 - 1);
    aArgs[1].Name = OUString::fromUtf8("Modifier");
    aArgs[1].Value <<= static_cast<sal_uInt16>(KEY_SHIFT);
    comphelper::dispatchCommand(".uno:SelectRow", aArgs);

    //  only row 5 should remain selected
    aResult = pModelObj->getTextSelection("text/plain;charset=utf-8", aUsedMimeType);
    aExpected = "1\t2\t3\t4\t5\t6\t7\t8\t9\t10\t11\t12\t13\t14\t15\t16\t17\t18\t19\t20\t21\n";
    CPPUNIT_ASSERT_EQUAL(aExpected, aResult);

    comphelper::LibreOfficeKit::setActive(false);
}

void ScTiledRenderingTest::testSortAscendingDescending()
{
    comphelper::LibreOfficeKit::setActive();
    ScModelObj* pModelObj = createDoc("sort-range.ods");
    ScDocument* pDoc = pModelObj->GetDocument();

    // select the values in the first column
    pModelObj->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONDOWN, 551, 129, 1, MOUSE_LEFT, 0);
    pModelObj->postMouseEvent(LOK_MOUSEEVENT_MOUSEMOVE, 820, 1336, 1, MOUSE_LEFT, 0);
    pModelObj->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONUP, 820, 1359, 1, MOUSE_LEFT, 0);

    // sort ascending
    uno::Sequence<beans::PropertyValue> aArgs;
    comphelper::dispatchCommand(".uno:SortAscending", aArgs);

    // check it's sorted
    for (SCROW r = 0; r < 6; ++r)
    {
        CPPUNIT_ASSERT_EQUAL(double(r + 1), pDoc->GetValue(ScAddress(0, r, 0)));
    }

    // sort descending
    comphelper::dispatchCommand(".uno:SortDescending", aArgs);

    // check it's sorted
    for (SCROW r = 0; r < 6; ++r)
    {
        CPPUNIT_ASSERT_EQUAL(double(6 - r), pDoc->GetValue(ScAddress(0, r, 0)));
    }

    // nothing else was sorted
    CPPUNIT_ASSERT_EQUAL(double(1), pDoc->GetValue(ScAddress(1, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(double(3), pDoc->GetValue(ScAddress(1, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(double(2), pDoc->GetValue(ScAddress(1, 2, 0)));

    comphelper::LibreOfficeKit::setActive(false);
}

void ScTiledRenderingTest::testPartHash()
{
    comphelper::LibreOfficeKit::setActive();
    ScModelObj* pModelObj = createDoc("sort-range.ods");

    int nParts = pModelObj->getParts();
    for (int it = 0; it < nParts; it++)
    {
        CPPUNIT_ASSERT(!pModelObj->getPartHash(it).isEmpty());
    }

    // check part that it does not exists
    CPPUNIT_ASSERT(pModelObj->getPartHash(100).isEmpty());
    comphelper::LibreOfficeKit::setActive(false);
}

void ScTiledRenderingTest::testDocumentSize()
{
    comphelper::LibreOfficeKit::setActive();
    ScModelObj* pModelObj = createDoc("sort-range.ods");
    ScDocShell* pDocSh = dynamic_cast< ScDocShell* >( pModelObj->GetEmbeddedObject() );
    CPPUNIT_ASSERT(pDocSh);

    ScTabViewShell* pViewShell = pDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT(pViewShell);

    pViewShell->registerLibreOfficeKitViewCallback(&ScTiledRenderingTest::callback, this);

    // check initial document size
    Size aDocSize = pModelObj->getDocumentSize();
    CPPUNIT_ASSERT(aDocSize.Width() > 0);
    CPPUNIT_ASSERT(aDocSize.Height() > 0);

    // Set cursor column
    pViewShell->SetCursor(100, 0);
    // 2 seconds
    TimeValue aTime = { 2 , 0 };
    osl::Condition::Result aResult = m_aDocSizeCondition.wait(aTime);
    CPPUNIT_ASSERT_EQUAL(aResult, osl::Condition::result_ok);

    // Set cursor row
    pViewShell->SetCursor(0, 100);
    // 2 seconds
    aTime = { 2 , 0 };
    aResult = m_aDocSizeCondition.wait(aTime);
    CPPUNIT_ASSERT_EQUAL(aResult, osl::Condition::result_ok);

    comphelper::LibreOfficeKit::setActive(false);
}

void ScTiledRenderingTest::testEmptyColumnSelection()
{
    comphelper::LibreOfficeKit::setActive();
    ScModelObj* pModelObj = createDoc("select-row-cols.ods");
    uno::Sequence<beans::PropertyValue> aArgs(2);

    // Select empty column, 1000
    aArgs[0].Name = OUString::fromUtf8("Col");
    aArgs[0].Value <<= static_cast<sal_Int32>(1000 - 1);
    aArgs[1].Name = OUString::fromUtf8("Modifier");
    aArgs[1].Value <<= static_cast<sal_uInt16>(0);
    comphelper::dispatchCommand(".uno:SelectColumn", aArgs);

    // Get plain selection
    OString aUsedMimeType;
    OString aResult = pModelObj->getTextSelection("text/plain;charset=utf-8", aUsedMimeType);
    // should be an empty string
    CPPUNIT_ASSERT_EQUAL(OString(), aResult);

    comphelper::LibreOfficeKit::setActive(false);
}

class ViewCallback
{
public:
    bool m_bOwnCursorInvalidated;
    bool m_bViewCursorInvalidated;
    bool m_bTextViewSelectionInvalidated;
    bool m_bViewLock;

    ViewCallback()
        : m_bOwnCursorInvalidated(false),
          m_bViewCursorInvalidated(false),
          m_bTextViewSelectionInvalidated(false),
          m_bViewLock(false)
    {
    }

    static void callback(int nType, const char* pPayload, void* pData)
    {
        static_cast<ViewCallback*>(pData)->callbackImpl(nType, pPayload);
    }

    void callbackImpl(int nType, const char* pPayload)
    {
        switch (nType)
        {
        case LOK_CALLBACK_CELL_CURSOR:
        {
            m_bOwnCursorInvalidated = true;
        }
        break;
        case LOK_CALLBACK_CELL_VIEW_CURSOR:
        {
            m_bViewCursorInvalidated = true;
        }
        break;
        case LOK_CALLBACK_TEXT_VIEW_SELECTION:
        {
            m_bTextViewSelectionInvalidated = true;
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


void ScTiledRenderingTest::testViewCursors()
{
    comphelper::LibreOfficeKit::setActive();

    ScModelObj* pModelObj = createDoc("select-row-cols.ods");
    ViewCallback aView1;
    SfxViewShell::Current()->registerLibreOfficeKitViewCallback(&ViewCallback::callback, &aView1);
    SfxLokHelper::createView();
    ViewCallback aView2;
    SfxViewShell::Current()->registerLibreOfficeKitViewCallback(&ViewCallback::callback, &aView2);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::DOWN);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::DOWN);
    Scheduler::ProcessEventsToIdle();
    SfxLokHelper::destroyView(SfxLokHelper::getView());
    CPPUNIT_ASSERT(aView1.m_bViewCursorInvalidated);
    mxComponent->dispose();
    mxComponent.clear();

    comphelper::LibreOfficeKit::setActive(false);
}

void lcl_dispatchCommand(const uno::Reference<lang::XComponent>& xComponent, const OUString& rCommand, const uno::Sequence<beans::PropertyValue>& rArguments)
{
    uno::Reference<frame::XController> xController = uno::Reference<frame::XModel>(xComponent, uno::UNO_QUERY)->getCurrentController();
    CPPUNIT_ASSERT(xController.is());
    uno::Reference<frame::XDispatchProvider> xFrame(xController->getFrame(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xFrame.is());

    uno::Reference<uno::XComponentContext> xContext = ::comphelper::getProcessComponentContext();
    uno::Reference<frame::XDispatchHelper> xDispatchHelper(frame::DispatchHelper::create(xContext));
    CPPUNIT_ASSERT(xDispatchHelper.is());

    xDispatchHelper->executeDispatch(xFrame, rCommand, OUString(), 0, rArguments);
}

void ScTiledRenderingTest::testTextViewSelection()
{
    comphelper::LibreOfficeKit::setActive();

    // Create two views, and leave the second one current.
    ScModelObj* pModelObj = createDoc("select-row-cols.ods");
    ViewCallback aView1;
    SfxViewShell::Current()->registerLibreOfficeKitViewCallback(&ViewCallback::callback, &aView1);
    SfxLokHelper::createView();
    ViewCallback aView2;
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    SfxViewShell::Current()->registerLibreOfficeKitViewCallback(&ViewCallback::callback, &aView2);

    // Create a selection on two cells in the second view, that's a text selection in LOK terms.
    aView1.m_bTextViewSelectionInvalidated = false;
    lcl_dispatchCommand(mxComponent, ".uno:GoRightSel", {});
    Scheduler::ProcessEventsToIdle();
    // Make sure the first view got its notification.
    CPPUNIT_ASSERT(aView1.m_bTextViewSelectionInvalidated);

    mxComponent->dispose();
    mxComponent.clear();
    comphelper::LibreOfficeKit::setActive(false);
}

void ScTiledRenderingTest::testDocumentSizeChanged()
{
    comphelper::LibreOfficeKit::setActive();

    // Load a document that doesn't have much content.
    createDoc("small.ods");
    SfxViewShell::Current()->registerLibreOfficeKitViewCallback(&ScTiledRenderingTest::callback, this);

    // Go to the A30 cell -- that will extend the document size.
    uno::Sequence<beans::PropertyValue> aPropertyValues =
    {
        comphelper::makePropertyValue("ToPoint", OUString("$A$30")),
    };
    lcl_dispatchCommand(mxComponent, ".uno:GoToCell", aPropertyValues);
    Scheduler::ProcessEventsToIdle();
    // Assert that the size in the payload is not 0.
    CPPUNIT_ASSERT(m_aDocumentSize.getWidth() > 0);
    CPPUNIT_ASSERT(m_aDocumentSize.getHeight() > 0);

    mxComponent->dispose();
    mxComponent.clear();
    comphelper::LibreOfficeKit::setActive(false);
}

void ScTiledRenderingTest::testViewLock()
{
    comphelper::LibreOfficeKit::setActive();

    // Load a document that has a shape and create two views.
    ScModelObj* pModelObj = createDoc("shape.ods");
    ViewCallback aView1;
    SfxViewShell::Current()->registerLibreOfficeKitViewCallback(&ViewCallback::callback, &aView1);
    SfxLokHelper::createView();
    ViewCallback aView2;
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    SfxViewShell::Current()->registerLibreOfficeKitViewCallback(&ViewCallback::callback, &aView2);

    // Begin text edit in the second view and assert that the first gets a lock
    // notification.
    const ScViewData* pViewData = ScDocShell::GetViewData();
    ScTabViewShell* pViewShell = pViewData->GetViewShell();
    CPPUNIT_ASSERT(pViewShell);
    SdrModel* pDrawModel = pViewData->GetDocument()->GetDrawLayer();
    SdrPage* pDrawPage = pDrawModel->GetPage(0);
    SdrObject* pObject = pDrawPage->GetObj(0);
    SdrView* pView = pViewShell->GetSdrView();
    aView1.m_bViewLock = false;
    pView->SdrBeginTextEdit(pObject);
    CPPUNIT_ASSERT(aView1.m_bViewLock);

    // End text edit in the second view, and assert that the lock is removed in
    // the first view.
    pView->SdrEndTextEdit();
    CPPUNIT_ASSERT(!aView1.m_bViewLock);

    mxComponent->dispose();
    mxComponent.clear();
    comphelper::LibreOfficeKit::setActive(false);
}

}

CPPUNIT_TEST_SUITE_REGISTRATION(ScTiledRenderingTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
