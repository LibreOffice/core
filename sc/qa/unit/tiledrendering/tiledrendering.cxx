/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>
#include <test/helper/transferable.hxx>
#include <unotest/macros_test.hxx>
#include <test/xmltesttools.hxx>
#include <boost/property_tree/json_parser.hpp>

#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/DispatchHelper.hpp>
#include <com/sun/star/datatransfer/clipboard/CurrentViewClipboard.hpp>
#include <com/sun/star/datatransfer/UnsupportedFlavorException.hpp>
#include <comphelper/dispatchcommand.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
#include <osl/conditn.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/stritem.hxx>

#include <comphelper/lok.hxx>
#include <comphelper/propertyvalue.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/lokhelper.hxx>
#include <svx/svdpage.hxx>
#include <vcl/svapp.hxx>
#include <vcl/scheduler.hxx>
#include <vcl/vclevent.hxx>
#include <vcl/virdev.hxx>
#include <vcl/unohelp2.hxx>
#include <sc.hrc>
#include <comphelper/string.hxx>
#include <tools/json_writer.hxx>
#include <docoptio.hxx>
#include <test/lokcallback.hxx>

#include <chrono>
#include <cstddef>

#include <scitems.hxx>
#include <tabvwsh.hxx>
#include <docsh.hxx>
#include <document.hxx>
#include <docuno.hxx>
#include <drwlayer.hxx>
#include <attrib.hxx>
#include <editutil.hxx>

using namespace css;

static std::ostream& operator<<(std::ostream& os, ViewShellId const & id)
{
    os << static_cast<sal_Int32>(id); return os;
}

namespace
{

char const DATA_DIRECTORY[] = "/sc/qa/unit/tiledrendering/data/";

class ScTiledRenderingTest : public test::BootstrapFixture, public unotest::MacrosTest, public XmlTestTools
{
public:
    ScTiledRenderingTest();
    virtual void setUp() override;
    virtual void tearDown() override;

    void testRowColumnHeaders();
    void testRowColumnSelections();
    void testPartHash();
    void testDocumentSize();
    void testEmptyColumnSelection();
    void testViewCursors();
    void testTextViewSelection();
    void testDocumentSizeChanged();
    void testViewLock();
    void testColRowResize();
    void testUndoShells();
    void testCreateViewGraphicSelection();
    void testTextEditViews();
    void testTextEditViewInvalidations();
    void testGraphicInvalidate();
    void testAutoSum();
    void testHideColRow();
    void testInvalidateOnCopyPasteCells();
    void testInvalidateOnInserRowCol();
    void testCommentCallback();
    void testUndoLimiting();
    void testUndoRepairDispatch();
    void testInsertGraphicInvalidations();
    void testDocumentSizeWithTwoViews();
    void testDisableUndoRepair();
    void testDocumentRepair();
    void testLanguageStatus();
    void testMultiViewCopyPaste();
    void testIMESupport();
    void testFilterDlg();
    void testVbaRangeCopyPaste();
    void testInvalidationLoop();
    void testPageDownInvalidation();
    void testSheetChangeInvalidation();
    void testInsertDeletePageInvalidation();
    void testGetRowColumnHeadersInvalidation();
    void testJumpHorizontallyInvalidation();
    void testJumpToLastRowInvalidation();
    void testSheetGeometryDataInvariance();
    void testSheetGeometryDataCorrectness();
    void testDeleteCellMultilineContent();
    void testFunctionDlg();
    void testSpellOnlineParameter();
    void testSpellOnlineRenderParameter();
    void testPasteIntoWrapTextCell();
    void testSortAscendingDescending();
    void testAutoInputExactMatch();
    void testMoveShapeHandle();
    void testEditCursorBounds();
    void testTextSelectionBounds();
    void testSheetViewDataCrash();

    CPPUNIT_TEST_SUITE(ScTiledRenderingTest);
    CPPUNIT_TEST(testRowColumnHeaders);
    CPPUNIT_TEST(testRowColumnSelections);
    CPPUNIT_TEST(testPartHash);
    CPPUNIT_TEST(testDocumentSize);
    CPPUNIT_TEST(testEmptyColumnSelection);
    CPPUNIT_TEST(testViewCursors);
    CPPUNIT_TEST(testTextViewSelection);
    CPPUNIT_TEST(testDocumentSizeChanged);
    CPPUNIT_TEST(testViewLock);
    CPPUNIT_TEST(testColRowResize);
    CPPUNIT_TEST(testUndoShells);
    CPPUNIT_TEST(testCreateViewGraphicSelection);
    CPPUNIT_TEST(testTextEditViews);
    CPPUNIT_TEST(testTextEditViewInvalidations);
    CPPUNIT_TEST(testGraphicInvalidate);
    CPPUNIT_TEST(testAutoSum);
    CPPUNIT_TEST(testHideColRow);
    CPPUNIT_TEST(testInvalidateOnCopyPasteCells);
    CPPUNIT_TEST(testInvalidateOnInserRowCol);
    CPPUNIT_TEST(testCommentCallback);
    CPPUNIT_TEST(testUndoLimiting);
    CPPUNIT_TEST(testUndoRepairDispatch);
    CPPUNIT_TEST(testInsertGraphicInvalidations);
    CPPUNIT_TEST(testDocumentSizeWithTwoViews);
    CPPUNIT_TEST(testDisableUndoRepair);
    CPPUNIT_TEST(testDocumentRepair);
    CPPUNIT_TEST(testLanguageStatus);
    CPPUNIT_TEST(testMultiViewCopyPaste);
    CPPUNIT_TEST(testIMESupport);
    CPPUNIT_TEST(testFilterDlg);
    CPPUNIT_TEST(testVbaRangeCopyPaste);
    CPPUNIT_TEST(testInvalidationLoop);
    CPPUNIT_TEST(testPageDownInvalidation);
    CPPUNIT_TEST(testSheetChangeInvalidation);
    CPPUNIT_TEST(testInsertDeletePageInvalidation);
    CPPUNIT_TEST(testGetRowColumnHeadersInvalidation);
    CPPUNIT_TEST(testJumpHorizontallyInvalidation);
    CPPUNIT_TEST(testJumpToLastRowInvalidation);
    CPPUNIT_TEST(testSheetGeometryDataInvariance);
    CPPUNIT_TEST(testSheetGeometryDataCorrectness);
    CPPUNIT_TEST(testDeleteCellMultilineContent);
    CPPUNIT_TEST(testFunctionDlg);
    CPPUNIT_TEST(testSpellOnlineParameter);
    CPPUNIT_TEST(testSpellOnlineRenderParameter);
    CPPUNIT_TEST(testPasteIntoWrapTextCell);
    CPPUNIT_TEST(testSortAscendingDescending);
    CPPUNIT_TEST(testAutoInputExactMatch);
    CPPUNIT_TEST(testMoveShapeHandle);
    CPPUNIT_TEST(testEditCursorBounds);
    CPPUNIT_TEST(testTextSelectionBounds);
    CPPUNIT_TEST(testSheetViewDataCrash);
    CPPUNIT_TEST_SUITE_END();

private:
    ScModelObj* createDoc(const char* pName);
    void setupLibreOfficeKitViewCallback(SfxViewShell* pViewShell);
    static void callback(int nType, const char* pPayload, void* pData);
    void callbackImpl(int nType, const char* pPayload);

    /// document size changed callback.
    osl::Condition m_aDocSizeCondition;
    Size m_aDocumentSize;

    uno::Reference<lang::XComponent> mxComponent;
    TestLokCallbackWrapper m_callbackWrapper;
};

ScTiledRenderingTest::ScTiledRenderingTest()
    : m_callbackWrapper(&callback, this)
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
    {
        ScModelObj* pModelObj = static_cast<ScModelObj*>(mxComponent.get());
        ScDocShell* pDocSh = dynamic_cast< ScDocShell* >( pModelObj->GetEmbeddedObject() );
        if (pDocSh)
        {
            ScTabViewShell* pViewShell = pDocSh->GetBestViewShell(false);
            if (pViewShell)
            {
                // The current view is unregistered here, multiple views have to be unregistered
                // in the test function itself.
                pViewShell->setLibreOfficeKitViewCallback(nullptr);
            }
        }
        mxComponent->dispose();
    }
    m_callbackWrapper.clear();
    comphelper::LibreOfficeKit::setActive(false);

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

void ScTiledRenderingTest::setupLibreOfficeKitViewCallback(SfxViewShell* pViewShell)
{
    pViewShell->setLibreOfficeKitViewCallback(&m_callbackWrapper);
    m_callbackWrapper.setLOKViewId(SfxLokHelper::getView(pViewShell));
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

    // Select the 5th row with no modifier
    uno::Sequence<beans::PropertyValue> aArgs( comphelper::InitPropertySequence({
            { "Row", uno::Any(sal_Int32(5 - 1)) },
            { "Modifier", uno::Any(sal_uInt16(0)) }
        }));
    comphelper::dispatchCommand(".uno:SelectRow", aArgs);

    // Check if it is selected
    OString aResult = apitest::helper::transferable::getTextSelection(pModelObj->getSelection(), "text/plain;charset=utf-8");
    OString aExpected("1\t2\t3\t4\t5\t6\t7\t8\t9\t10\t11\t12\t13\t14\t15\t16\t17\t18\t19\t20\t21\n");
    CPPUNIT_ASSERT_EQUAL(aExpected, aResult);

    // Select the 10th row with shift modifier
    aArgs[0].Name = "Row";
    aArgs[0].Value <<= static_cast<sal_Int32>(10 - 1);
    aArgs[1].Name = "Modifier";
    aArgs[1].Value <<= KEY_SHIFT;
    comphelper::dispatchCommand(".uno:SelectRow", aArgs);

    // Check if all the rows from 5th to 10th get selected
    aResult = apitest::helper::transferable::getTextSelection(pModelObj->getSelection(), "text/plain;charset=utf-8");
    aExpected = "1\t2\t3\t4\t5\t6\t7\t8\t9\t10\t11\t12\t13\t14\t15\t16\t17\t18\t19\t20\t21\n2\t3\t4\t5\t6\t7\t8\t9\t10\t11\t12\t13\t14\t15\t16\t17\t18\t19\t20\t21\t22\n3\t4\t5\t6\t7\t8\t9\t10\t11\t12\t13\t14\t15\t16\t17\t18\t19\t20\t21\t22\t23\n4\t5\t6\t7\t8\t9\t10\t11\t12\t13\t14\t15\t16\t17\t18\t19\t20\t21\t22\t23\t24\n5\t6\t7\t8\t9\t10\t11\t12\t13\t14\t15\t16\t17\t18\t19\t20\t21\t22\t23\t24\t25\n6\t7\t8\t9\t10\t11\t12\t13\t14\t15\t16\t17\t18\t19\t20\t21\t22\t23\t24\t25\t26\n";
    CPPUNIT_ASSERT_EQUAL(aExpected, aResult);

    // Select the 10th row with ctrl modifier
    aArgs[0].Name = "Row";
    aArgs[0].Value <<= static_cast<sal_Int32>(13 - 1);
    aArgs[1].Name = "Modifier";
    aArgs[1].Value <<= KEY_MOD1;
    comphelper::dispatchCommand(".uno:SelectRow", aArgs);

    // When we copy this, we don't get anything useful, but we must not crash
    // (used to happen)
    aResult = apitest::helper::transferable::getTextSelection(pModelObj->getSelection(), "text/plain;charset=utf-8");
    CPPUNIT_ASSERT_EQUAL(OString(), aResult);

    // TODO check that we really selected what we wanted here

    // Select Column 5 with ctrl modifier
    aArgs[0].Name = "Col";
    aArgs[0].Value <<= static_cast<sal_Int32>(5 - 1);
    aArgs[1].Name = "Modifier";
    aArgs[1].Value <<= KEY_MOD1;
    comphelper::dispatchCommand(".uno:SelectColumn", aArgs);

    // When we copy this, we don't get anything useful, but we must not crash
    // (used to happen)
    aResult = apitest::helper::transferable::getTextSelection(pModelObj->getSelection(), "text/plain;charset=utf-8");
    CPPUNIT_ASSERT_EQUAL(OString(), aResult);

    // TODO check that we really selected what we wanted here

    // Test for deselection of already selected rows
    // First Deselect Row 13 because copy doesn't work for multiple selections
    aArgs[0].Name = "Row";
    aArgs[0].Value <<= static_cast<sal_Int32>(13 - 1);
    aArgs[1].Name = "Modifier";
    aArgs[1].Value <<= KEY_MOD1;
    comphelper::dispatchCommand(".uno:SelectRow", aArgs);

    // Deselect row 10
    aArgs[0].Name = "Row";
    aArgs[0].Value <<= static_cast<sal_Int32>(10 - 1);
    aArgs[1].Name = "Modifier";
    aArgs[1].Value <<= KEY_MOD1;
    comphelper::dispatchCommand(".uno:SelectRow", aArgs);

    // Click at row 6 holding shift
    aArgs[0].Name = "Row";
    aArgs[0].Value <<= static_cast<sal_Int32>(6 - 1);
    aArgs[1].Name = "Modifier";
    aArgs[1].Value <<= KEY_SHIFT;
    comphelper::dispatchCommand(".uno:SelectRow", aArgs);

    //  only row 5 should remain selected
    aResult = apitest::helper::transferable::getTextSelection(pModelObj->getSelection(), "text/plain;charset=utf-8");
    aExpected = "1\t2\t3\t4\t5\t6\t7\t8\t9\t10\t11\t12\t13\t14\t15\t16\t17\t18\t19\t20\t21\n";
    CPPUNIT_ASSERT_EQUAL(aExpected, aResult);
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
}

void ScTiledRenderingTest::testDocumentSize()
{
    comphelper::LibreOfficeKit::setActive();
    ScModelObj* pModelObj = createDoc("sort-range.ods");
    ScDocShell* pDocSh = dynamic_cast< ScDocShell* >( pModelObj->GetEmbeddedObject() );
    CPPUNIT_ASSERT(pDocSh);

    ScTabViewShell* pViewShell = pDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT(pViewShell);

    setupLibreOfficeKitViewCallback(pViewShell);

    // check initial document size
    Size aDocSize = pModelObj->getDocumentSize();
    CPPUNIT_ASSERT(aDocSize.Width() > 0);
    CPPUNIT_ASSERT(aDocSize.Height() > 0);

    // Set cursor column
    pViewShell->SetCursor(100, 0);
    // 2 seconds
    osl::Condition::Result aResult = m_aDocSizeCondition.wait(std::chrono::seconds(2));
    CPPUNIT_ASSERT_EQUAL(osl::Condition::result_ok, aResult);

    // Set cursor row
    pViewShell->SetCursor(0, 100);
    // 2 seconds
    aResult = m_aDocSizeCondition.wait(std::chrono::seconds(2));
    CPPUNIT_ASSERT_EQUAL(osl::Condition::result_ok, aResult);
}

void ScTiledRenderingTest::testEmptyColumnSelection()
{
    comphelper::LibreOfficeKit::setActive();
    ScModelObj* pModelObj = createDoc("select-row-cols.ods");

    // Select empty column, 1000
    uno::Sequence<beans::PropertyValue> aArgs( comphelper::InitPropertySequence({
                { "Col", uno::Any(sal_Int32(1000 - 1)) },
                { "Modifier", uno::Any(sal_uInt16(0)) }
        }));
    comphelper::dispatchCommand(".uno:SelectColumn", aArgs);

    // should be an empty string
    CPPUNIT_ASSERT_EQUAL(OString(), apitest::helper::transferable::getTextSelection(pModelObj->getSelection(), "text/plain;charset=utf-8"));
}

struct EditCursorMessage final {
    tools::Rectangle m_aRelRect;
    Point m_aRefPoint;

    void clear()
    {
        m_aRelRect.SetEmpty();
        m_aRefPoint = Point(-1, -1);
    }

    bool empty()
    {
        return m_aRelRect.IsEmpty() &&
            m_aRefPoint.X() == -1 &&
            m_aRefPoint.Y() == -1;
    }

    void parseMessage(const char* pMessage)
    {
        clear();
        if (!pMessage || !comphelper::LibreOfficeKit::isCompatFlagSet(
            comphelper::LibreOfficeKit::Compat::scPrintTwipsMsgs) ||
            !comphelper::LibreOfficeKit::isViewIdForVisCursorInvalidation())
            return;

        std::stringstream aStream(pMessage);
        boost::property_tree::ptree aTree;
        boost::property_tree::read_json(aStream, aTree);
        std::string aVal = aTree.get_child("refpoint").get_value<std::string>();

        uno::Sequence<OUString> aSeq = comphelper::string::convertCommaSeparated(OUString::createFromAscii(aVal.c_str()));
        CPPUNIT_ASSERT_EQUAL(2, aSeq.getLength());
        m_aRefPoint.setX(aSeq[0].toInt32());
        m_aRefPoint.setY(aSeq[1].toInt32());

        aVal = aTree.get_child("relrect").get_value<std::string>();
        aSeq = comphelper::string::convertCommaSeparated(OUString::createFromAscii(aVal.c_str()));
        CPPUNIT_ASSERT_EQUAL(4, aSeq.getLength());
        m_aRelRect.setX(aSeq[0].toInt32());
        m_aRelRect.setY(aSeq[1].toInt32());
        m_aRelRect.setWidth(aSeq[2].toInt32());
        m_aRelRect.setHeight(aSeq[3].toInt32());
    }

    tools::Rectangle getBounds()
    {
        tools::Rectangle aBounds = m_aRelRect;
        aBounds.Move(m_aRefPoint.X(), m_aRefPoint.Y());
        return aBounds;
    }
};

struct TextSelectionMessage
{
    std::vector<tools::Rectangle> m_aRelRects;
    Point m_aRefPoint;

    void clear() {
        m_aRefPoint.setX(0);
        m_aRefPoint.setY(0);
        m_aRelRects.clear();
    }

    bool empty() {
        return m_aRelRects.empty();
    }

    void parseMessage(const char* pMessage)
    {
        clear();
        if (!pMessage)
            return;

        std::string aStr(pMessage);
        if (aStr.find(",") == std::string::npos)
            return;

        size_t nRefDelimStart = aStr.find("::");
        std::string aRectListString = (nRefDelimStart == std::string::npos) ? aStr : aStr.substr(0, nRefDelimStart);
        std::string aRefPointString = (nRefDelimStart == std::string::npos) ?
            std::string("0, 0") :
            aStr.substr(nRefDelimStart + 2, aStr.length() - 2 - nRefDelimStart);
        uno::Sequence<OUString> aSeq = comphelper::string::convertCommaSeparated(OUString::createFromAscii(aRefPointString.c_str()));
        CPPUNIT_ASSERT_EQUAL(2, aSeq.getLength());
        m_aRefPoint.setX(aSeq[0].toInt32());
        m_aRefPoint.setY(aSeq[1].toInt32());

        size_t nStart = 0;
        size_t nEnd = aRectListString.find(";");
        if (nEnd == std::string::npos)
            nEnd = aRectListString.length();
        do
        {
            std::string aRectString = aRectListString.substr(nStart, nEnd - nStart);
            {
                aSeq = comphelper::string::convertCommaSeparated(OUString::createFromAscii(aRectString.c_str()));
                CPPUNIT_ASSERT_EQUAL(4, aSeq.getLength());
                tools::Rectangle aRect;
                aRect.setX(aSeq[0].toInt32());
                aRect.setY(aSeq[1].toInt32());
                aRect.setWidth(aSeq[2].toInt32());
                aRect.setHeight(aSeq[3].toInt32());

                m_aRelRects.push_back(aRect);
            }

            nStart = nEnd + 1;
            nEnd = aRectListString.find(";", nStart);
        }
        while(nEnd != std::string::npos);
    }

    tools::Rectangle getBounds(size_t nIndex)
    {
        if (nIndex >= m_aRelRects.size())
            return tools::Rectangle();

        tools::Rectangle aBounds = m_aRelRects[nIndex];
        aBounds.Move(m_aRefPoint.X(), m_aRefPoint.Y());
        return aBounds;
    }

};

/// A view callback tracks callbacks invoked on one specific view.
class ViewCallback final
{
    SfxViewShell* mpViewShell;
    int mnView;
public:
    bool m_bOwnCursorInvalidated;
    bool m_bViewCursorInvalidated;
    bool m_bTextViewSelectionInvalidated;
    bool m_bGraphicSelection;
    bool m_bGraphicViewSelection;
    bool m_bFullInvalidateTiles;
    bool m_bInvalidateTiles;
    std::vector<tools::Rectangle> m_aInvalidations;
    tools::Rectangle m_aCellCursorBounds;
    std::vector<int> m_aInvalidationsParts;
    bool m_bViewLock;
    OString m_sCellFormula;
    boost::property_tree::ptree m_aCommentCallbackResult;
    EditCursorMessage m_aInvalidateCursorResult;
    TextSelectionMessage m_aTextSelectionResult;
    OString m_sInvalidateHeader;
    OString m_sInvalidateSheetGeometry;
    OString m_ShapeSelection;
    TestLokCallbackWrapper m_callbackWrapper;

    ViewCallback(bool bDeleteListenerOnDestruct=true)
        : m_bOwnCursorInvalidated(false),
          m_bViewCursorInvalidated(false),
          m_bTextViewSelectionInvalidated(false),
          m_bGraphicSelection(false),
          m_bGraphicViewSelection(false),
          m_bFullInvalidateTiles(false),
          m_bInvalidateTiles(false),
          m_bViewLock(false),
          m_callbackWrapper(&callback, this)
    {
        mpViewShell = SfxViewShell::Current();
        mpViewShell->setLibreOfficeKitViewCallback(&m_callbackWrapper);
        mnView = SfxLokHelper::getView();
        m_callbackWrapper.setLOKViewId( mnView );
        if (!bDeleteListenerOnDestruct)
            mpViewShell = nullptr;
    }

    ~ViewCallback()
    {
        if (mpViewShell)
        {
            SfxLokHelper::setView(mnView);
            mpViewShell->setLibreOfficeKitViewCallback(nullptr);
        }
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
            uno::Sequence<OUString> aSeq = comphelper::string::convertCommaSeparated(OUString::createFromAscii(pPayload));
            m_aCellCursorBounds = tools::Rectangle();
            if (aSeq.getLength() == 6) {
                m_aCellCursorBounds.setX(aSeq[0].toInt32());
                m_aCellCursorBounds.setY(aSeq[1].toInt32());
                m_aCellCursorBounds.setWidth(aSeq[2].toInt32());
                m_aCellCursorBounds.setHeight(aSeq[3].toInt32());
            }
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
        case LOK_CALLBACK_GRAPHIC_SELECTION:
        {
            m_bGraphicSelection = true;
            m_ShapeSelection = OString(pPayload);
        }
        break;
        case LOK_CALLBACK_GRAPHIC_VIEW_SELECTION:
        {
            m_bGraphicViewSelection = true;
        }
        break;
        case LOK_CALLBACK_INVALIDATE_TILES:
        {
            OString text(pPayload);
            if (text.startsWith("EMPTY"))
            {
                m_bFullInvalidateTiles = true;
            }
            else
            {
                uno::Sequence<OUString> aSeq = comphelper::string::convertCommaSeparated(OUString::createFromAscii(pPayload));
                CPPUNIT_ASSERT(aSeq.getLength() == 4 || aSeq.getLength() == 5);
                tools::Rectangle aInvalidationRect;
                aInvalidationRect.setX(aSeq[0].toInt32());
                aInvalidationRect.setY(aSeq[1].toInt32());
                aInvalidationRect.setWidth(aSeq[2].toInt32());
                aInvalidationRect.setHeight(aSeq[3].toInt32());
                m_aInvalidations.push_back(aInvalidationRect);
                if (aSeq.getLength() == 5)
                    m_aInvalidationsParts.push_back(aSeq[4].toInt32());
                m_bInvalidateTiles = true;
            }
        }
        break;
        case LOK_CALLBACK_CELL_FORMULA:
        {
            m_sCellFormula = pPayload;
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
        case LOK_CALLBACK_INVALIDATE_HEADER:
        {
            m_sInvalidateHeader = pPayload;
        }
        break;
        case LOK_CALLBACK_INVALIDATE_SHEET_GEOMETRY:
        {
            m_sInvalidateSheetGeometry = pPayload;
        }
        break;
        case LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR:
        {
            m_aInvalidateCursorResult.parseMessage(pPayload);
        }
        break;
        case LOK_CALLBACK_TEXT_SELECTION:
        {
            m_aTextSelectionResult.parseMessage(pPayload);
        }
        }
    }
};


void ScTiledRenderingTest::testViewCursors()
{
    comphelper::LibreOfficeKit::setActive();

    ScModelObj* pModelObj = createDoc("select-row-cols.ods");
    ViewCallback aView1;
    SfxLokHelper::createView();
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ViewCallback aView2(/*bDeleteListenerOnDestruct*/false);
    // This was false, the new view did not get the view (cell) cursor of the old view.
    CPPUNIT_ASSERT(aView2.m_bViewCursorInvalidated);
    CPPUNIT_ASSERT(aView2.m_bOwnCursorInvalidated);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::DOWN);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::DOWN);
    Scheduler::ProcessEventsToIdle();
    SfxLokHelper::destroyView(SfxLokHelper::getView());
    CPPUNIT_ASSERT(aView1.m_bViewCursorInvalidated);
}

void ScTiledRenderingTest::testSpellOnlineRenderParameter()
{
    ScModelObj* pModelObj = createDoc("empty.ods");
    ScDocument* pDoc = pModelObj->GetDocument();
    bool bSet = pDoc->GetDocOptions().IsAutoSpell();

    uno::Sequence<beans::PropertyValue> aPropertyValues =
    {
        comphelper::makePropertyValue(".uno:SpellOnline", uno::makeAny(!bSet)),
    };
    pModelObj->initializeForTiledRendering(aPropertyValues);

    CPPUNIT_ASSERT_EQUAL(!bSet, pDoc->GetDocOptions().IsAutoSpell());
}

void ScTiledRenderingTest::testTextViewSelection()
{
    comphelper::LibreOfficeKit::setActive();

    // Create two views, and leave the second one current.
    ScModelObj* pModelObj = createDoc("select-row-cols.ods");
    ViewCallback aView1;
    SfxLokHelper::createView();
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ViewCallback aView2;

    // Create a selection on two cells in the second view, that's a text selection in LOK terms.
    aView1.m_bTextViewSelectionInvalidated = false;
    dispatchCommand(mxComponent, ".uno:GoRightSel", {});
    Scheduler::ProcessEventsToIdle();
    // Make sure the first view got its notification.
    CPPUNIT_ASSERT(aView1.m_bTextViewSelectionInvalidated);
}

void ScTiledRenderingTest::testDocumentSizeChanged()
{
    comphelper::LibreOfficeKit::setActive();

    // Load a document that doesn't have much content.
    createDoc("small.ods");
    setupLibreOfficeKitViewCallback(SfxViewShell::Current());

    // Go to the A30 cell -- that will extend the document size.
    uno::Sequence<beans::PropertyValue> aPropertyValues =
    {
        comphelper::makePropertyValue("ToPoint", OUString("$A$30")),
    };
    dispatchCommand(mxComponent, ".uno:GoToCell", aPropertyValues);
    Scheduler::ProcessEventsToIdle();
    // Assert that the size in the payload is not 0.
    CPPUNIT_ASSERT(m_aDocumentSize.getWidth() > 0);
    CPPUNIT_ASSERT(m_aDocumentSize.getHeight() > 0);
}

void ScTiledRenderingTest::testViewLock()
{
    comphelper::LibreOfficeKit::setActive();

    // Load a document that has a shape and create two views.
    ScModelObj* pModelObj = createDoc("shape.ods");
    ViewCallback aView1;
    SfxLokHelper::createView();
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ViewCallback aView2;

    // Begin text edit in the second view and assert that the first gets a lock
    // notification.
    const ScViewData* pViewData = ScDocShell::GetViewData();
    ScTabViewShell* pViewShell = pViewData->GetViewShell();
    CPPUNIT_ASSERT(pViewShell);
    SdrModel* pDrawModel = pViewData->GetDocument().GetDrawLayer();
    SdrPage* pDrawPage = pDrawModel->GetPage(0);
    SdrObject* pObject = pDrawPage->GetObj(0);
    SdrView* pView = pViewShell->GetScDrawView();
    aView1.m_bViewLock = false;
    pView->SdrBeginTextEdit(pObject);
    CPPUNIT_ASSERT(aView1.m_bViewLock);

    // End text edit in the second view, and assert that the lock is removed in
    // the first view.
    pView->SdrEndTextEdit();
    CPPUNIT_ASSERT(!aView1.m_bViewLock);
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

void ScTiledRenderingTest::testMoveShapeHandle()
{
    comphelper::LibreOfficeKit::setActive();
    ScModelObj* pModelObj = createDoc("shape.ods");
    ViewCallback aView1;
    pModelObj->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONDOWN, /*x=*/ 1,/*y=*/ 1,/*count=*/ 1, /*buttons=*/ 1, /*modifier=*/0);
    pModelObj->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONUP, /*x=*/ 1, /*y=*/ 1, /*count=*/ 1, /*buttons=*/ 1, /*modifier=*/0);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT(!aView1.m_ShapeSelection.isEmpty());
    {
        int id, x, y;
        lcl_extractHandleParameters(aView1.m_ShapeSelection, id, x ,y);
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
        CPPUNIT_ASSERT(!aView1.m_ShapeSelection.isEmpty());
        lcl_extractHandleParameters(aView1.m_ShapeSelection, id, x ,y);
        CPPUNIT_ASSERT_EQUAL(x-1, oldX);
        CPPUNIT_ASSERT_EQUAL(y-1, oldY);
    }
}

void ScTiledRenderingTest::testColRowResize()
{
    comphelper::LibreOfficeKit::setActive();
    ScModelObj* pModelObj = createDoc("sort-range.ods");
    ScDocShell* pDocSh = dynamic_cast< ScDocShell* >( pModelObj->GetEmbeddedObject() );
    CPPUNIT_ASSERT(pDocSh);

    ScTabViewShell* pViewShell = pDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT(pViewShell);

    setupLibreOfficeKitViewCallback(pViewShell);

    ScDocument& rDoc = pDocSh->GetDocument();

    // Col 3, Tab 0
    uno::Sequence<beans::PropertyValue> aArgs( comphelper::InitPropertySequence({
            { "ColumnWidth", uno::Any(sal_uInt16(4000)) }, // 4cm
            { "Column", uno::Any(sal_Int16(3)) }
        }));
    comphelper::dispatchCommand(".uno:ColumnWidth", aArgs);

    sal_uInt16 nWidth = rDoc.GetColWidth(static_cast<SCCOL>(2), static_cast<SCTAB>(0), false) * HMM_PER_TWIPS;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(4000), nWidth);

    // Row 5, Tab 0
    uno::Sequence<beans::PropertyValue> aArgs2( comphelper::InitPropertySequence({
            { "RowHeight", uno::Any(sal_uInt16(2000)) },
            { "Row", uno::Any(sal_Int16(5)) },
        }));
    comphelper::dispatchCommand(".uno:RowHeight", aArgs2);

    sal_uInt16 nHeight = rDoc.GetRowHeight(static_cast<SCROW>(4), static_cast<SCTAB>(0), false) * HMM_PER_TWIPS;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(2000), nHeight);
}

void ScTiledRenderingTest::testUndoShells()
{
    comphelper::LibreOfficeKit::setActive();
    ScModelObj* pModelObj = createDoc("small.ods");
    // Clear the currently selected cell.
    comphelper::dispatchCommand(".uno:ClearContents", {});

    auto pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    CPPUNIT_ASSERT(pDocShell);
    ScDocument& rDoc = pDocShell->GetDocument();
    SfxUndoManager* pUndoManager = rDoc.GetUndoManager();
    CPPUNIT_ASSERT(pUndoManager);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pUndoManager->GetUndoActionCount());
    sal_Int32 nView1 = SfxLokHelper::getView();
    // This was -1: ScSimpleUndo did not remember what view shell created it.
    CPPUNIT_ASSERT_EQUAL(ViewShellId(nView1), pUndoManager->GetUndoAction()->GetViewShellId());
}

bool lcl_hasEditView(const ScViewData& rViewData)
{
    bool bResult = false;
    for (unsigned int i=0; i<4; i++)
    {
        bResult = rViewData.HasEditView( static_cast<ScSplitPos>(i) );
        if (bResult) break;
    }
    return bResult;
}

void ScTiledRenderingTest::testTextEditViews()
{
    comphelper::LibreOfficeKit::setActive();

    ScModelObj* pModelObj = createDoc("small.ods");
    CPPUNIT_ASSERT(pModelObj);
    ScViewData* pViewData = ScDocShell::GetViewData();
    CPPUNIT_ASSERT(pViewData);

    // view #1
    ViewCallback aView1;
    CPPUNIT_ASSERT(!lcl_hasEditView(*pViewData));

    // text edit a cell in view #1
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'x', 0);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 'x', 0);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(lcl_hasEditView(*pViewData));

    // view #2
    SfxLokHelper::createView();
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ViewCallback aView2;

    // move cell cursor i view #2
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::DOWN);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::DOWN);
    Scheduler::ProcessEventsToIdle();

    // check that text edit view in view #1 has not be killed
    CPPUNIT_ASSERT(lcl_hasEditView(*pViewData));
}

void ScTiledRenderingTest::testTextEditViewInvalidations()
{
    comphelper::LibreOfficeKit::setActive();

    ScModelObj* pModelObj = createDoc("small.ods");
    CPPUNIT_ASSERT(pModelObj);
    ScViewData* pViewData = ScDocShell::GetViewData();
    CPPUNIT_ASSERT(pViewData);

    // view #1
    int nView1 = SfxLokHelper::getView();
    ViewCallback aView1;
    CPPUNIT_ASSERT(!lcl_hasEditView(*pViewData));

    // view #2
    SfxLokHelper::createView();
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ViewCallback aView2;

    // text edit a cell in view #1
    SfxLokHelper::setView(nView1);
    aView2.m_bInvalidateTiles = false;
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'x', 0);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 'x', 0);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(lcl_hasEditView(*pViewData));
    CPPUNIT_ASSERT(aView2.m_bInvalidateTiles);

    // text edit a cell in view #1 until
    // we can be sure we are out of the initial tile
    for (int i = 0; i < 40; ++i)
    {
        pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'x', 0);
        pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 'x', 0);
    }
    Scheduler::ProcessEventsToIdle();

    // text edit a cell in view #1 inside the new tile and
    // check that view #2 receive a tile invalidate message
    aView2.m_bInvalidateTiles = false;
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'x', 0);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 'x', 0);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(aView2.m_bInvalidateTiles);

    // view #3
    SfxLokHelper::createView();
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ViewCallback aView3;

    // text edit a cell in view #1
    SfxLokHelper::setView(nView1);
    aView3.m_bInvalidateTiles = false;
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'y', 0);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 'y', 0);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(aView3.m_bInvalidateTiles);
}

void ScTiledRenderingTest::testCreateViewGraphicSelection()
{
    // Load a document
    comphelper::LibreOfficeKit::setActive();

    // Load a document that has a shape and create two views.
    ScModelObj* pModelObj = createDoc("shape.ods");
    ViewCallback aView1;

    // Mark the graphic in the first view.
    const ScViewData* pViewData = ScDocShell::GetViewData();
    ScTabViewShell* pViewShell = pViewData->GetViewShell();
    CPPUNIT_ASSERT(pViewShell);
    SdrModel* pDrawModel = pViewData->GetDocument().GetDrawLayer();
    SdrPage* pDrawPage = pDrawModel->GetPage(0);
    SdrObject* pObject = pDrawPage->GetObj(0);
    SdrView* pView = pViewShell->GetScDrawView();
    aView1.m_bGraphicSelection = false;
    aView1.m_bGraphicViewSelection = false;
    pView->MarkObj(pObject, pView->GetSdrPageView());
    CPPUNIT_ASSERT(aView1.m_bGraphicSelection);

    // Create a second view.
    int nView1 = SfxLokHelper::getView();
    SfxLokHelper::createView();
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ViewCallback aView2;
    CPPUNIT_ASSERT(aView2.m_bGraphicViewSelection);
    CPPUNIT_ASSERT(aView1.m_bGraphicViewSelection);

    SfxLokHelper::setView(nView1);
    SfxViewShell::Current()->setLibreOfficeKitViewCallback(nullptr);
}

void ScTiledRenderingTest::testGraphicInvalidate()
{
    // Load a document
    comphelper::LibreOfficeKit::setActive();

    // Load a document that has a shape and create two views.
    ScModelObj* pModelObj = createDoc("shape.ods");
    ViewCallback aView;

    // Click to select graphic
    aView.m_bGraphicSelection = false;
    pModelObj->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONDOWN, /*x=*/ 1,/*y=*/ 1,/*count=*/ 1, /*buttons=*/ 1, /*modifier=*/0);
    pModelObj->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONUP, /*x=*/ 1, /*y=*/ 1, /*count=*/ 1, /*buttons=*/ 1, /*modifier=*/0);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(aView.m_bGraphicSelection);

    // Drag Drop graphic
    aView.m_bGraphicSelection = false;
    pModelObj->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONDOWN, /*x=*/ 1,/*y=*/ 1,/*count=*/ 1, /*buttons=*/ 1, /*modifier=*/0);
    pModelObj->postMouseEvent(LOK_MOUSEEVENT_MOUSEMOVE, /*x=*/ 1,/*y=*/ 10,/*count=*/ 1, /*buttons=*/ 1, /*modifier=*/0);
    pModelObj->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONUP, /*x=*/ 1, /*y=*/ 10, /*count=*/ 1, /*buttons=*/ 1, /*modifier=*/0);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(!aView.m_bFullInvalidateTiles);

    // Check again
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(!aView.m_bFullInvalidateTiles);
}

void ScTiledRenderingTest::testAutoSum()
{
     // Load a document
    comphelper::LibreOfficeKit::setActive();
    createDoc("small.ods");

    ViewCallback aView;

    uno::Sequence<beans::PropertyValue> aArgs;
    comphelper::dispatchCommand(".uno:AutoSum", aArgs);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(aView.m_sCellFormula.startsWith("=SUM("));
}

void ScTiledRenderingTest::testHideColRow()
{
    // Load a document
    comphelper::LibreOfficeKit::setActive();
    createDoc("small.ods");
    {
        uno::Sequence<beans::PropertyValue> aArgs( comphelper::InitPropertySequence({
                { "Col", uno::Any(sal_Int32(2 - 1)) },
                { "Modifier", uno::Any(KEY_SHIFT) }
            }));
        comphelper::dispatchCommand(".uno:SelectColumn", aArgs);

        uno::Sequence<beans::PropertyValue> aArgs2( comphelper::InitPropertySequence({
                { "Col", uno::Any(sal_Int32(3 - 1)) },
                { "Modifier", uno::Any(sal_uInt16(0)) }
            }));

        comphelper::dispatchCommand(".uno:SelectColumn", aArgs2);
        Scheduler::ProcessEventsToIdle();
    }

    SCCOL nOldCurX = ScDocShell::GetViewData()->GetCurX();
    SCROW nOldCurY = ScDocShell::GetViewData()->GetCurY();
    {
        uno::Sequence<beans::PropertyValue> aArgs;
        comphelper::dispatchCommand(".uno:HideColumn", aArgs);
        Scheduler::ProcessEventsToIdle();
    }

    SCCOL nNewCurX = ScDocShell::GetViewData()->GetCurX();
    SCROW nNewCurY = ScDocShell::GetViewData()->GetCurY();
    CPPUNIT_ASSERT(nNewCurX > nOldCurX);
    CPPUNIT_ASSERT_EQUAL(nOldCurY, nNewCurY);
    {
        uno::Sequence<beans::PropertyValue> aArgs( comphelper::InitPropertySequence({
                { "Row", uno::Any(sal_Int32(6 - 1)) },
                { "Modifier", uno::Any(KEY_SHIFT) }
            }));
        comphelper::dispatchCommand(".uno:SelectRow", aArgs);

        uno::Sequence<beans::PropertyValue> aArgs2( comphelper::InitPropertySequence({
                { "Row", uno::Any(sal_Int32(7 - 1)) },
                { "Modifier", uno::Any(sal_uInt16(0)) }
            }));
        comphelper::dispatchCommand(".uno:SelectRow", aArgs2);
        Scheduler::ProcessEventsToIdle();
    }

    nOldCurX = ScDocShell::GetViewData()->GetCurX();
    nOldCurY = ScDocShell::GetViewData()->GetCurY();
    {
        uno::Sequence<beans::PropertyValue> aArgs;
        comphelper::dispatchCommand(".uno:HideRow", aArgs);
        Scheduler::ProcessEventsToIdle();
    }
    nNewCurX = ScDocShell::GetViewData()->GetCurX();
    nNewCurY = ScDocShell::GetViewData()->GetCurY();
    CPPUNIT_ASSERT(nNewCurY > nOldCurY);
    CPPUNIT_ASSERT_EQUAL(nOldCurX, nNewCurX);
}

void ScTiledRenderingTest::testInvalidateOnCopyPasteCells()
{
    // Load a document
    comphelper::LibreOfficeKit::setActive();
    ScModelObj* pModelObj = createDoc("small.ods");
    CPPUNIT_ASSERT(pModelObj);

    // view
    ViewCallback aView;

    uno::Sequence<beans::PropertyValue> aArgs;
    // select and copy cells
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_HOME | KEY_MOD1);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_HOME | KEY_MOD1);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_DOWN | KEY_SHIFT);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_DOWN | KEY_SHIFT);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_RIGHT | KEY_SHIFT);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_RIGHT | KEY_SHIFT);
    Scheduler::ProcessEventsToIdle();
    comphelper::dispatchCommand(".uno:Copy", aArgs);

    // move to destination cell
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_DOWN);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_DOWN);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_DOWN | KEY_MOD1);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_DOWN | KEY_MOD1);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_UP);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_UP);
    Scheduler::ProcessEventsToIdle();

    // paste cells
    aView.m_bInvalidateTiles = false;
    comphelper::dispatchCommand(".uno:Paste", aArgs);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(aView.m_bInvalidateTiles);
}

void ScTiledRenderingTest::testInvalidateOnInserRowCol()
{
    // Load a document
    comphelper::LibreOfficeKit::setActive();
    ScModelObj* pModelObj = createDoc("small.ods");
    CPPUNIT_ASSERT(pModelObj);

    // view
    ViewCallback aView;

    uno::Sequence<beans::PropertyValue> aArgs;
    // move downward
    for (int i = 0; i < 200; ++i)
    {
        pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_DOWN);
        pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_DOWN);
    }
    Scheduler::ProcessEventsToIdle();

    // insert row
    aView.m_bInvalidateTiles = false;
    aView.m_aInvalidations.clear();
    comphelper::dispatchCommand(".uno:InsertRows", aArgs);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(aView.m_bInvalidateTiles);
    CPPUNIT_ASSERT_EQUAL(size_t(2), aView.m_aInvalidations.size());
    CPPUNIT_ASSERT_EQUAL(tools::Rectangle(-75, 50985, 32212230, 63990), aView.m_aInvalidations[0]);

    // move on the right
    for (int i = 0; i < 200; ++i)
    {
        pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_RIGHT);
        pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_RIGHT);
    }
    Scheduler::ProcessEventsToIdle();

    // insert column
    aView.m_bInvalidateTiles = false;
    aView.m_aInvalidations.clear();
    comphelper::dispatchCommand(".uno:InsertColumns", aArgs);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(aView.m_bInvalidateTiles);
    CPPUNIT_ASSERT_EQUAL(size_t(2), aView.m_aInvalidations.size());
    CPPUNIT_ASSERT_EQUAL(tools::Rectangle(253650, -15, 32212230, 63990), aView.m_aInvalidations[0]);
}

void ScTiledRenderingTest::testCommentCallback()
{
    // Load a document
    comphelper::LibreOfficeKit::setActive();
    // Comments callback are emitted only if tiled annotations are off
    comphelper::LibreOfficeKit::setTiledAnnotations(false);

    {
        ScModelObj* pModelObj = createDoc("small.ods");
        ViewCallback aView1;
        int nView1 = SfxLokHelper::getView();

        // Create a 2nd view
        SfxLokHelper::createView();
        pModelObj->initializeForTiledRendering({});
        ViewCallback aView2;

        SfxLokHelper::setView(nView1);

        // Add a new comment
        uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence(
        {
            {"Text", uno::makeAny(OUString("Comment"))},
            {"Author", uno::makeAny(OUString("LOK User1"))},
        }));
        comphelper::dispatchCommand(".uno:InsertAnnotation", aArgs);
        Scheduler::ProcessEventsToIdle();

        // We received a LOK_CALLBACK_COMMENT callback with comment 'Add' action
        CPPUNIT_ASSERT_EQUAL(std::string("Add"), aView1.m_aCommentCallbackResult.get<std::string>("action"));
        CPPUNIT_ASSERT_EQUAL(std::string("Add"), aView2.m_aCommentCallbackResult.get<std::string>("action"));
        CPPUNIT_ASSERT_EQUAL(std::string("1"), aView1.m_aCommentCallbackResult.get<std::string>("id"));
        CPPUNIT_ASSERT_EQUAL(std::string("1"), aView2.m_aCommentCallbackResult.get<std::string>("id"));
        CPPUNIT_ASSERT_EQUAL(std::string("0"), aView1.m_aCommentCallbackResult.get<std::string>("tab"));
        CPPUNIT_ASSERT_EQUAL(std::string("0"), aView2.m_aCommentCallbackResult.get<std::string>("tab"));
        CPPUNIT_ASSERT_EQUAL(std::string("LOK User1"), aView1.m_aCommentCallbackResult.get<std::string>("author"));
        CPPUNIT_ASSERT_EQUAL(std::string("LOK User1"), aView2.m_aCommentCallbackResult.get<std::string>("author"));
        CPPUNIT_ASSERT_EQUAL(std::string("Comment"), aView1.m_aCommentCallbackResult.get<std::string>("text"));
        CPPUNIT_ASSERT_EQUAL(std::string("Comment"), aView2.m_aCommentCallbackResult.get<std::string>("text"));
        CPPUNIT_ASSERT_EQUAL(std::string("0, 255, 1274, 254"), aView1.m_aCommentCallbackResult.get<std::string>("cellPos"));
        CPPUNIT_ASSERT_EQUAL(std::string("0, 255, 1274, 254"), aView2.m_aCommentCallbackResult.get<std::string>("cellPos"));

        std::string aCommentId = aView1.m_aCommentCallbackResult.get<std::string>("id");

        // Edit a comment
        // Select some random cell, we should be able to edit the cell note without
        // selecting the cell
        ScTabViewShell* pTabViewShell = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
        if (pTabViewShell)
            pTabViewShell->SetCursor(3, 100);
        aArgs = comphelper::InitPropertySequence(
        {
            {"Id", uno::makeAny(OUString::createFromAscii(aCommentId.c_str()))},
            {"Text", uno::makeAny(OUString("Edited comment"))},
            {"Author", uno::makeAny(OUString("LOK User2"))},
        });
        comphelper::dispatchCommand(".uno:EditAnnotation", aArgs);
        Scheduler::ProcessEventsToIdle();

        // We received a LOK_CALLBACK_COMMENT callback with comment 'Modify' action
        CPPUNIT_ASSERT_EQUAL(std::string("Modify"), aView1.m_aCommentCallbackResult.get<std::string>("action"));
        CPPUNIT_ASSERT_EQUAL(std::string("Modify"), aView2.m_aCommentCallbackResult.get<std::string>("action"));
        CPPUNIT_ASSERT_EQUAL(aCommentId, aView1.m_aCommentCallbackResult.get<std::string>("id"));
        CPPUNIT_ASSERT_EQUAL(aCommentId, aView2.m_aCommentCallbackResult.get<std::string>("id"));
        CPPUNIT_ASSERT_EQUAL(std::string("LOK User2"), aView1.m_aCommentCallbackResult.get<std::string>("author"));
        CPPUNIT_ASSERT_EQUAL(std::string("LOK User2"), aView2.m_aCommentCallbackResult.get<std::string>("author"));
        CPPUNIT_ASSERT_EQUAL(std::string("Edited comment"), aView1.m_aCommentCallbackResult.get<std::string>("text"));
        CPPUNIT_ASSERT_EQUAL(std::string("Edited comment"), aView2.m_aCommentCallbackResult.get<std::string>("text"));
        CPPUNIT_ASSERT_EQUAL(std::string("0, 255, 1274, 254"), aView1.m_aCommentCallbackResult.get<std::string>("cellPos"));
        CPPUNIT_ASSERT_EQUAL(std::string("0, 255, 1274, 254"), aView2.m_aCommentCallbackResult.get<std::string>("cellPos"));

        // Delete the comment
        if (pTabViewShell)
            pTabViewShell->SetCursor(4, 43);
        aArgs = comphelper::InitPropertySequence(
        {
            {"Id", uno::makeAny(OUString::createFromAscii(aCommentId.c_str()))}
        });
        comphelper::dispatchCommand(".uno:DeleteNote", aArgs);
        Scheduler::ProcessEventsToIdle();

        // We received a LOK_CALLBACK_COMMENT callback with comment 'Remove' action
        CPPUNIT_ASSERT_EQUAL(std::string("Remove"), aView1.m_aCommentCallbackResult.get<std::string>("action"));
        CPPUNIT_ASSERT_EQUAL(std::string("Remove"), aView2.m_aCommentCallbackResult.get<std::string>("action"));
        CPPUNIT_ASSERT_EQUAL(aCommentId, aView1.m_aCommentCallbackResult.get<std::string>("id"));
        CPPUNIT_ASSERT_EQUAL(aCommentId, aView2.m_aCommentCallbackResult.get<std::string>("id"));
    }
    comphelper::LibreOfficeKit::setTiledAnnotations(true);
}

void ScTiledRenderingTest::testUndoLimiting()
{
    comphelper::LibreOfficeKit::setActive();

    ScModelObj* pModelObj = createDoc("small.ods");
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    SfxUndoManager* pUndoManager = pDoc->GetUndoManager();
    CPPUNIT_ASSERT(pUndoManager);

    // view #1
    int nView1 = SfxLokHelper::getView();
    ViewCallback aView1;

    // view #2
    SfxLokHelper::createView();
    int nView2 = SfxLokHelper::getView();
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ViewCallback aView2;

    // text edit a cell in view #1
    SfxLokHelper::setView(nView1);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'x', 0);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 'x', 0);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::RETURN);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::RETURN);
    Scheduler::ProcessEventsToIdle();

    // check that undo action count in not 0
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), pUndoManager->GetUndoActionCount());

    // try to execute undo in view #2
    SfxLokHelper::setView(nView2);
    comphelper::dispatchCommand(".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();
    // check that undo has not been executed on view #2
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), pUndoManager->GetUndoActionCount());

    // try to execute undo in view #1
    SfxLokHelper::setView(nView1);
    comphelper::dispatchCommand(".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();
    // check that undo has been executed on view #1
    CPPUNIT_ASSERT_EQUAL(std::size_t(0), pUndoManager->GetUndoActionCount());

    // check that redo action count in not 0
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), pUndoManager->GetRedoActionCount());

    // try to execute redo in view #2
    SfxLokHelper::setView(nView2);
    comphelper::dispatchCommand(".uno:Redo", {});
    Scheduler::ProcessEventsToIdle();
    // check that redo has not been executed on view #2
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), pUndoManager->GetRedoActionCount());

    // try to execute redo in view #1
    SfxLokHelper::setView(nView1);
    comphelper::dispatchCommand(".uno:Redo", {});
    Scheduler::ProcessEventsToIdle();
    // check that redo has been executed on view #1
    CPPUNIT_ASSERT_EQUAL(std::size_t(0), pUndoManager->GetRedoActionCount());
}

void ScTiledRenderingTest::testUndoRepairDispatch()
{
    comphelper::LibreOfficeKit::setActive();

    ScModelObj* pModelObj = createDoc("small.ods");
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    SfxUndoManager* pUndoManager = pDoc->GetUndoManager();
    CPPUNIT_ASSERT(pUndoManager);

    // view #1
    int nView1 = SfxLokHelper::getView();
    ViewCallback aView1;

    // view #2
    SfxLokHelper::createView();
    int nView2 = SfxLokHelper::getView();
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ViewCallback aView2;

    // text edit a cell in view #1
    SfxLokHelper::setView(nView1);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'x', 0);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 'x', 0);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::RETURN);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::RETURN);
    Scheduler::ProcessEventsToIdle();

    // check that undo action count in not 0
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), pUndoManager->GetUndoActionCount());

    // try to execute undo in view #2
    SfxLokHelper::setView(nView2);
    comphelper::dispatchCommand(".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();
    // check that undo has not been executed on view #2
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), pUndoManager->GetUndoActionCount());

    // try to execute undo in view #2 in repair mode
    SfxLokHelper::setView(nView2);
    uno::Sequence<beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence(
    {
        {"Repair", uno::makeAny(true)}
    }));
    comphelper::dispatchCommand(".uno:Undo", aPropertyValues);
    Scheduler::ProcessEventsToIdle();
    // check that undo has been executed on view #2 in repair mode
    CPPUNIT_ASSERT_EQUAL(std::size_t(0), pUndoManager->GetUndoActionCount());
}

void ScTiledRenderingTest::testInsertGraphicInvalidations()
{
    comphelper::LibreOfficeKit::setActive();

    ScModelObj* pModelObj = createDoc("small.ods");
    CPPUNIT_ASSERT(pModelObj);
    ScViewData* pViewData = ScDocShell::GetViewData();
    CPPUNIT_ASSERT(pViewData);

    // view
    ViewCallback aView;

    // we need to paint a tile in the view for triggering the tile invalidation solution
    int nCanvasWidth = 256;
    int nCanvasHeight = 256;
    std::vector<unsigned char> aBuffer(nCanvasWidth * nCanvasHeight * 4);
    ScopedVclPtrInstance<VirtualDevice> pDevice(DeviceFormat::DEFAULT);
    pDevice->SetOutputSizePixelScaleOffsetAndBuffer(Size(nCanvasWidth, nCanvasHeight), Fraction(1.0), Point(), aBuffer.data());
    pModelObj->paintTile(*pDevice, nCanvasWidth, nCanvasHeight, /*nTilePosX=*/0, /*nTilePosY=*/0, /*nTileWidth=*/3840, /*nTileHeight=*/3840);
    Scheduler::ProcessEventsToIdle();

    // insert an image in view and see if both views are invalidated
    aView.m_bInvalidateTiles = false;
    uno::Sequence<beans::PropertyValue> aArgs( comphelper::InitPropertySequence({
            { "FileName", uno::Any(m_directories.getURLFromSrc(DATA_DIRECTORY) + "smile.png") }
        }));
    comphelper::dispatchCommand(".uno:InsertGraphic", aArgs);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(aView.m_bInvalidateTiles);

    // undo image insertion in view and see if both views are invalidated
    aView.m_bInvalidateTiles = false;
    uno::Sequence<beans::PropertyValue> aArgs2;
    comphelper::dispatchCommand(".uno:Undo", aArgs2);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(aView.m_bInvalidateTiles);
}

void ScTiledRenderingTest::testDocumentSizeWithTwoViews()
{
    comphelper::LibreOfficeKit::setActive();

    // Open a document that has the cursor far away & paint a tile
    ScModelObj* pModelObj = createDoc("cursor-away.ods");

    // Set the visible area, and press page down
    pModelObj->setClientVisibleArea(tools::Rectangle(750, 1861, 20583, 6997));
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::PAGEDOWN);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::PAGEDOWN);
    Scheduler::ProcessEventsToIdle();

    int nCanvasWidth = 256;
    int nCanvasHeight = 256;
    std::vector<unsigned char> aBuffer1(nCanvasWidth * nCanvasHeight * 4);
    ScopedVclPtrInstance<VirtualDevice> pDevice1(DeviceFormat::DEFAULT);
    pDevice1->SetOutputSizePixelScaleOffsetAndBuffer(Size(nCanvasWidth, nCanvasHeight), Fraction(1.0), Point(), aBuffer1.data());
    pModelObj->paintTile(*pDevice1, nCanvasWidth, nCanvasHeight, /*nTilePosX=*/0, /*nTilePosY=*/291840, /*nTileWidth=*/3840, /*nTileHeight=*/3840);
    Scheduler::ProcessEventsToIdle();

    // Create a new view
    int nView1 = SfxLokHelper::getView();
    SfxLokHelper::createView();

    std::vector<unsigned char> aBuffer2(nCanvasWidth * nCanvasHeight * 4);
    ScopedVclPtrInstance<VirtualDevice> pDevice2(DeviceFormat::DEFAULT);
    pDevice2->SetOutputSizePixelScaleOffsetAndBuffer(Size(nCanvasWidth, nCanvasHeight), Fraction(1.0), Point(), aBuffer2.data());
    pModelObj->paintTile(*pDevice2, nCanvasWidth, nCanvasHeight, /*nTilePosX=*/0, /*nTilePosY=*/291840, /*nTileWidth=*/3840, /*nTileHeight=*/3840);
    Scheduler::ProcessEventsToIdle();

    // Check that the tiles actually have the same content
    for (size_t i = 0; i < aBuffer1.size(); ++i)
        CPPUNIT_ASSERT_EQUAL(aBuffer1[i], aBuffer2[i]);

    SfxViewShell::Current()->setLibreOfficeKitViewCallback(nullptr);
    SfxLokHelper::setView(nView1);
    SfxViewShell::Current()->setLibreOfficeKitViewCallback(nullptr);
}

void ScTiledRenderingTest::testDisableUndoRepair()
{
    comphelper::LibreOfficeKit::setActive();
    ScModelObj* pModelObj = createDoc("cursor-away.ods");
    CPPUNIT_ASSERT(pModelObj);

    // view #1
    int nView1 = SfxLokHelper::getView();
    SfxViewShell* pView1 = SfxViewShell::Current();

    // view #2
    SfxLokHelper::createView();
    int nView2 = SfxLokHelper::getView();
    SfxViewShell* pView2 = SfxViewShell::Current();
    CPPUNIT_ASSERT(pView1 != pView2);
    {
        SfxItemSet aSet1(pView1->GetPool(), svl::Items<SID_UNDO, SID_UNDO>{});
        SfxItemSet aSet2(pView2->GetPool(), svl::Items<SID_UNDO, SID_UNDO>{});
        pView1->GetSlotState(SID_UNDO, nullptr, &aSet1);
        pView2->GetSlotState(SID_UNDO, nullptr, &aSet2);
        CPPUNIT_ASSERT_EQUAL(SfxItemState::DISABLED, aSet1.GetItemState(SID_UNDO));
        CPPUNIT_ASSERT_EQUAL(SfxItemState::DISABLED, aSet2.GetItemState(SID_UNDO));
    }

    // text edit a cell in view #1
    SfxLokHelper::setView(nView1);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'h', 0);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 'h', 0);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::RETURN);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::RETURN);
    Scheduler::ProcessEventsToIdle();
    {
        SfxItemSet aSet1(pView1->GetPool(), svl::Items<SID_UNDO, SID_UNDO>{});
        SfxItemSet aSet2(pView2->GetPool(), svl::Items<SID_UNDO, SID_UNDO>{});
        pView1->GetSlotState(SID_UNDO, nullptr, &aSet1);
        pView2->GetSlotState(SID_UNDO, nullptr, &aSet2);
        CPPUNIT_ASSERT_EQUAL(SfxItemState::SET, aSet1.GetItemState(SID_UNDO));
        CPPUNIT_ASSERT(dynamic_cast< const SfxStringItem* >(aSet1.GetItem(SID_UNDO)));
        CPPUNIT_ASSERT_EQUAL(SfxItemState::SET, aSet2.GetItemState(SID_UNDO));
        CPPUNIT_ASSERT(dynamic_cast< const SfxUInt32Item* >(aSet2.GetItem(SID_UNDO)));
        const SfxUInt32Item* pUInt32Item = dynamic_cast<const SfxUInt32Item*>(aSet2.GetItem(SID_UNDO));
        CPPUNIT_ASSERT(pUInt32Item);
        CPPUNIT_ASSERT_EQUAL(static_cast< sal_uInt32 >(SID_REPAIRPACKAGE), pUInt32Item->GetValue());
    }

    // text edit a cell in view #2
    SfxLokHelper::setView(nView2);
    pModelObj->setPart(1);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'c', 0);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 'c', 0);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::RETURN);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::RETURN);
    Scheduler::ProcessEventsToIdle();
    {
        SfxItemSet aSet1(pView1->GetPool(), svl::Items<SID_UNDO, SID_UNDO>{});
        SfxItemSet aSet2(pView2->GetPool(), svl::Items<SID_UNDO, SID_UNDO>{});
        pView1->GetSlotState(SID_UNDO, nullptr, &aSet1);
        pView2->GetSlotState(SID_UNDO, nullptr, &aSet2);
        CPPUNIT_ASSERT_EQUAL(SfxItemState::SET, aSet1.GetItemState(SID_UNDO));
        const SfxUInt32Item* pUInt32Item = dynamic_cast<const SfxUInt32Item*>(aSet1.GetItem(SID_UNDO));
        CPPUNIT_ASSERT(pUInt32Item);
        CPPUNIT_ASSERT_EQUAL(static_cast< sal_uInt32 >(SID_REPAIRPACKAGE), pUInt32Item->GetValue());
        CPPUNIT_ASSERT_EQUAL(SfxItemState::SET, aSet2.GetItemState(SID_UNDO));
        CPPUNIT_ASSERT(dynamic_cast< const SfxStringItem* >(aSet2.GetItem(SID_UNDO)));
    }

    SfxViewShell::Current()->setLibreOfficeKitViewCallback(nullptr);
    SfxLokHelper::setView(nView1);
    SfxViewShell::Current()->setLibreOfficeKitViewCallback(nullptr);
}

void ScTiledRenderingTest::testDocumentRepair()
{
    comphelper::LibreOfficeKit::setActive();

    // Create two views.
    ScModelObj* pModelObj = createDoc("cursor-away.ods");
    CPPUNIT_ASSERT(pModelObj);

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
        const SfxBoolItem* pItem1 = dynamic_cast< const SfxBoolItem* >(xItem1.get());
        const SfxBoolItem* pItem2 = dynamic_cast< const SfxBoolItem* >(xItem2.get());
        CPPUNIT_ASSERT(pItem1);
        CPPUNIT_ASSERT(pItem2);
        CPPUNIT_ASSERT_EQUAL(false, pItem1->GetValue());
        CPPUNIT_ASSERT_EQUAL(false, pItem2->GetValue());
    }

    // Insert a character in the second view.
    SfxLokHelper::setView(nView2);
    pModelObj->setPart(1);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'c', 0);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 'c', 0);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::RETURN);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::RETURN);
    Scheduler::ProcessEventsToIdle();
    {
        std::unique_ptr<SfxPoolItem> xItem1;
        std::unique_ptr<SfxPoolItem> xItem2;
        pView1->GetViewFrame()->GetBindings().QueryState(SID_DOC_REPAIR, xItem1);
        pView2->GetViewFrame()->GetBindings().QueryState(SID_DOC_REPAIR, xItem2);
        const SfxBoolItem* pItem1 = dynamic_cast< const SfxBoolItem* >(xItem1.get());
        const SfxBoolItem* pItem2 = dynamic_cast< const SfxBoolItem* >(xItem2.get());
        CPPUNIT_ASSERT(pItem1);
        CPPUNIT_ASSERT(pItem2);
        CPPUNIT_ASSERT_EQUAL(true, pItem1->GetValue());
        CPPUNIT_ASSERT_EQUAL(true, pItem2->GetValue());
    }

    SfxViewShell::Current()->setLibreOfficeKitViewCallback(nullptr);
    SfxLokHelper::setView(nView1);
    SfxViewShell::Current()->setLibreOfficeKitViewCallback(nullptr);
}

void ScTiledRenderingTest::testLanguageStatus()
{
    comphelper::LibreOfficeKit::setActive();
    ScModelObj* pModelObj = createDoc("small.ods");
    CPPUNIT_ASSERT(pModelObj);
    ScDocShell* pDocSh = dynamic_cast< ScDocShell* >( pModelObj->GetEmbeddedObject() );
    CPPUNIT_ASSERT(pDocSh);

    // view #1
    SfxViewShell* pView1 = SfxViewShell::Current();

    // view #2
    int nView1 = SfxLokHelper::getView();
    SfxLokHelper::createView();
    SfxViewShell* pView2 = SfxViewShell::Current();
    CPPUNIT_ASSERT(pView1 != pView2);
    {
        std::unique_ptr<SfxPoolItem> xItem1;
        std::unique_ptr<SfxPoolItem> xItem2;
        pView1->GetViewFrame()->GetBindings().QueryState(SID_LANGUAGE_STATUS, xItem1);
        pView2->GetViewFrame()->GetBindings().QueryState(SID_LANGUAGE_STATUS, xItem2);
        const SfxStringItem* pItem1 = dynamic_cast<const SfxStringItem*>(xItem1.get());
        const SfxStringItem* pItem2 = dynamic_cast<const SfxStringItem*>(xItem2.get());
        CPPUNIT_ASSERT(pItem1);
        CPPUNIT_ASSERT(pItem2);
        CPPUNIT_ASSERT(!pItem1->GetValue().isEmpty());
        CPPUNIT_ASSERT(!pItem2->GetValue().isEmpty());
    }

    {
        SfxStringItem aLangString(SID_LANGUAGE_STATUS, "Default_Spanish (Bolivia)");
        pView1->GetViewFrame()->GetDispatcher()->ExecuteList(SID_LANGUAGE_STATUS,
            SfxCallMode::SYNCHRON, { &aLangString });
    }

    {
        std::unique_ptr<SfxPoolItem> xItem1;
        std::unique_ptr<SfxPoolItem> xItem2;
        pView1->GetViewFrame()->GetBindings().QueryState(SID_LANGUAGE_STATUS, xItem1);
        pView2->GetViewFrame()->GetBindings().QueryState(SID_LANGUAGE_STATUS, xItem2);
        const SfxStringItem* pItem1 = dynamic_cast<const SfxStringItem*>(xItem1.get());
        const SfxStringItem* pItem2 = dynamic_cast<const SfxStringItem*>(xItem2.get());
        CPPUNIT_ASSERT(pItem1);
        CPPUNIT_ASSERT(pItem2);
        const OUString aLangBolivia("Spanish (Bolivia);es-BO");
        CPPUNIT_ASSERT_EQUAL(aLangBolivia, pItem1->GetValue());
        CPPUNIT_ASSERT_EQUAL(aLangBolivia, pItem2->GetValue());
    }

    SfxViewShell::Current()->setLibreOfficeKitViewCallback(nullptr);
    SfxLokHelper::setView(nView1);
    SfxViewShell::Current()->setLibreOfficeKitViewCallback(nullptr);
}

void ScTiledRenderingTest::testMultiViewCopyPaste()
{
    comphelper::LibreOfficeKit::setActive();

    ScModelObj* pModelObj = createDoc("empty.ods");
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    pDoc->SetString(ScAddress(0, 0, 0), "TestCopy1");
    pDoc->SetString(ScAddress(1, 0, 0), "TestCopy2");

    // view #1
    ScTabViewShell* pView1 = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
    CPPUNIT_ASSERT(pView1);
    // emulate clipboard
    pView1->GetViewData().GetActiveWin()->SetClipboard(css::datatransfer::clipboard::CurrentViewClipboard::create(comphelper::getProcessComponentContext()));

    // view #2
    int nView1 = SfxLokHelper::getView();
    SfxLokHelper::createView();
    ScTabViewShell* pView2 = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
    // emulate clipboard
    pView2->GetViewData().GetActiveWin()->SetClipboard(css::datatransfer::clipboard::CurrentViewClipboard::create(comphelper::getProcessComponentContext()));
    CPPUNIT_ASSERT(pView2);
    CPPUNIT_ASSERT(pView1 != pView2);
    CPPUNIT_ASSERT(pView1->GetViewData().GetActiveWin()->GetClipboard() != pView2->GetViewData().GetActiveWin()->GetClipboard());

    // copy text view 1
    pView1->SetCursor(0, 0);
    pView1->GetViewFrame()->GetBindings().Execute(SID_COPY);

    // copy text view 2
    pView2->SetCursor(1, 0);
    pView2->GetViewFrame()->GetBindings().Execute(SID_COPY);

     // paste text view 1
    pView1->SetCursor(0, 1);
    pView1->GetViewFrame()->GetBindings().Execute(SID_PASTE);

    // paste text view 2
    pView2->SetCursor(1, 1);
    pView2->GetViewFrame()->GetBindings().Execute(SID_PASTE);

    CPPUNIT_ASSERT_EQUAL(OUString("TestCopy1"), pDoc->GetString(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("TestCopy2"), pDoc->GetString(ScAddress(1, 1, 0)));

    SfxViewShell::Current()->setLibreOfficeKitViewCallback(nullptr);
    SfxLokHelper::setView(nView1);
    SfxViewShell::Current()->setLibreOfficeKitViewCallback(nullptr);
}

void ScTiledRenderingTest::testIMESupport()
{
    comphelper::LibreOfficeKit::setActive();

    ScModelObj* pModelObj = createDoc("empty.ods");
    VclPtr<vcl::Window> pDocWindow = pModelObj->getDocWindow();
    ScDocument* pDoc = pModelObj->GetDocument();

    ScTabViewShell* pView = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
    CPPUNIT_ASSERT(pView);

    pView->SetCursor(0, 0);
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

    // commit the string to the cell
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::RETURN);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::RETURN);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(aInputs[aInputs.size() - 1], pDoc->GetString(ScAddress(0, 0, 0)));
}

void ScTiledRenderingTest::testFilterDlg()
{
    comphelper::LibreOfficeKit::setActive();

    createDoc("empty.ods");

    // view #1
    SfxViewShell* pView1 = SfxViewShell::Current();
    int nView1 = SfxLokHelper::getView();

    // view #2
    SfxLokHelper::createView();
    SfxViewShell* pView2 = SfxViewShell::Current();
    CPPUNIT_ASSERT(pView1 != pView2);
    {
        pView2->GetViewFrame()->GetDispatcher()->Execute(SID_FILTER,
            SfxCallMode::SLOT|SfxCallMode::RECORD);
    }

    Scheduler::ProcessEventsToIdle();
    SfxChildWindow* pRefWindow = pView2->GetViewFrame()->GetChildWindow(SID_FILTER);
    CPPUNIT_ASSERT(pRefWindow);

    // switch to view 1
    SfxLokHelper::setView(nView1);
    CPPUNIT_ASSERT_EQUAL(true, pView2->GetViewFrame()->GetDispatcher()->IsLocked());
    CPPUNIT_ASSERT_EQUAL(false, pView1->GetViewFrame()->GetDispatcher()->IsLocked());

    pRefWindow->GetController()->response(RET_CANCEL);

    CPPUNIT_ASSERT_EQUAL(false, pView2->GetViewFrame()->GetDispatcher()->IsLocked());
    CPPUNIT_ASSERT_EQUAL(false, pView1->GetViewFrame()->GetDispatcher()->IsLocked());

    SfxViewShell::Current()->setLibreOfficeKitViewCallback(nullptr);
    SfxLokHelper::setView(nView1);
    SfxViewShell::Current()->setLibreOfficeKitViewCallback(nullptr);
}

void ScTiledRenderingTest::testFunctionDlg()
{
    comphelper::LibreOfficeKit::setActive();

    createDoc("empty.ods");

    // view #1
    SfxViewShell* pView1 = SfxViewShell::Current();
    int nView1 = SfxLokHelper::getView();
    {
        pView1->GetViewFrame()->GetDispatcher()->Execute(SID_OPENDLG_FUNCTION,
            SfxCallMode::SLOT|SfxCallMode::RECORD);
    }
    Scheduler::ProcessEventsToIdle();
    SfxChildWindow* pRefWindow = pView1->GetViewFrame()->GetChildWindow(SID_OPENDLG_FUNCTION);
    CPPUNIT_ASSERT(pRefWindow);

    // view #2
    int nView2 = SfxLokHelper::createView();
    SfxViewShell* pView2 = SfxViewShell::Current();
    CPPUNIT_ASSERT(pView1 != pView2);

    // check loking
    CPPUNIT_ASSERT_EQUAL(true, pView1->GetViewFrame()->GetDispatcher()->IsLocked());
    CPPUNIT_ASSERT_EQUAL(false, pView2->GetViewFrame()->GetDispatcher()->IsLocked());

    SfxLokHelper::setView(nView1);
    pRefWindow->GetController()->response(RET_CANCEL);

    CPPUNIT_ASSERT_EQUAL(false, pView1->GetViewFrame()->GetDispatcher()->IsLocked());
    CPPUNIT_ASSERT_EQUAL(false, pView2->GetViewFrame()->GetDispatcher()->IsLocked());

    SfxViewShell::Current()->setLibreOfficeKitViewCallback(nullptr);
    SfxLokHelper::setView(nView2);
    SfxViewShell::Current()->setLibreOfficeKitViewCallback(nullptr);
}

void ScTiledRenderingTest::testSpellOnlineParameter()
{
    ScModelObj* pModelObj = createDoc("empty.ods");
    ScDocument* pDoc = pModelObj->GetDocument();
    bool bSet = pDoc->GetDocOptions().IsAutoSpell();

    uno::Sequence<beans::PropertyValue> params =
    {
        comphelper::makePropertyValue("Enable", uno::makeAny(!bSet)),
    };
    dispatchCommand(mxComponent, ".uno:SpellOnline", params);
    CPPUNIT_ASSERT_EQUAL(!bSet, pDoc->GetDocOptions().IsAutoSpell());

    // set the same state as now and we don't expect any change (no-toggle)
    params =
    {
        comphelper::makePropertyValue("Enable", uno::makeAny(!bSet)),
    };
    dispatchCommand(mxComponent, ".uno:SpellOnline", params);
    CPPUNIT_ASSERT_EQUAL(!bSet, pDoc->GetDocOptions().IsAutoSpell());
}

void ScTiledRenderingTest::testVbaRangeCopyPaste()
{
    comphelper::LibreOfficeKit::setActive();
    ScModelObj* pModelObj = createDoc("RangeCopyPaste.ods");
    ScDocShell* pDocShell = dynamic_cast< ScDocShell* >( pModelObj->GetEmbeddedObject() );
    CPPUNIT_ASSERT(pDocShell);

    uno::Any aRet;
    uno::Sequence< uno::Any > aOutParam;
    uno::Sequence< uno::Any > aParams;
    uno::Sequence< sal_Int16 > aOutParamIndex;

    SfxObjectShell::CallXScript(
        mxComponent,
        "vnd.sun.Star.script:Standard.Module1.Test_RangeCopyPaste?language=Basic&location=document",
        aParams, aRet, aOutParamIndex, aOutParam);

    CPPUNIT_ASSERT(!pDocShell->GetClipData().is());
}

void ScTiledRenderingTest::testInvalidationLoop()
{
    // Load the document with a form control.
    createDoc("invalidation-loop.fods");
    // Without the accompanying fix in place, this test would have never returned due to an infinite
    // invalidation loop between ScGridWindow::Paint() and vcl::Window::ImplPosSizeWindow().
    Scheduler::ProcessEventsToIdle();
}

void ScTiledRenderingTest::testPageDownInvalidation()
{
    comphelper::LibreOfficeKit::setActive();

    ScModelObj* pModelObj = createDoc("empty.ods");
    ScViewData* pViewData = ScDocShell::GetViewData();
    CPPUNIT_ASSERT(pViewData);

    int nView1 = SfxLokHelper::getView();
    ViewCallback aView1;
    CPPUNIT_ASSERT(!lcl_hasEditView(*pViewData));

    SfxLokHelper::setView(nView1);
    aView1.m_bInvalidateTiles = false;
    aView1.m_aInvalidations.clear();
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, awt::Key::PAGEDOWN, 0);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, awt::Key::PAGEDOWN, 0);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(aView1.m_bInvalidateTiles);
    CPPUNIT_ASSERT_EQUAL(size_t(3), aView1.m_aInvalidations.size());
    CPPUNIT_ASSERT_EQUAL(tools::Rectangle(15, 15, 1230, 225), aView1.m_aInvalidations[0]);
}

void ScTiledRenderingTest::testSheetChangeInvalidation()
{
    comphelper::LibreOfficeKit::setActive();
    const bool oldPartInInvalidation = comphelper::LibreOfficeKit::isPartInInvalidation();
    comphelper::LibreOfficeKit::setPartInInvalidation(true);

    ScModelObj* pModelObj = createDoc("two_sheets.ods");
    ScViewData* pViewData = ScDocShell::GetViewData();
    CPPUNIT_ASSERT(pViewData);

    int nView1 = SfxLokHelper::getView();
    ViewCallback aView1;
    CPPUNIT_ASSERT(!lcl_hasEditView(*pViewData));

    SfxLokHelper::setView(nView1);
    aView1.m_bInvalidateTiles = false;
    aView1.m_aInvalidations.clear();
    aView1.m_aInvalidationsParts.clear();
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::PAGEDOWN | KEY_MOD1);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::PAGEDOWN | KEY_MOD1);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(aView1.m_bInvalidateTiles);
    CPPUNIT_ASSERT_EQUAL(size_t(2), aView1.m_aInvalidations.size());
    CPPUNIT_ASSERT_EQUAL(tools::Rectangle(0, 0, 1310720, 268435456), aView1.m_aInvalidations[0]);
    CPPUNIT_ASSERT_EQUAL(tools::Rectangle(0, 0, 1000000000, 1000000000), aView1.m_aInvalidations[1]);
    CPPUNIT_ASSERT_EQUAL(size_t(2), aView1.m_aInvalidationsParts.size());
    CPPUNIT_ASSERT_EQUAL(pModelObj->getPart(), aView1.m_aInvalidationsParts[0]);
    CPPUNIT_ASSERT_EQUAL(pModelObj->getPart(), aView1.m_aInvalidationsParts[1]);
    comphelper::LibreOfficeKit::setPartInInvalidation(oldPartInInvalidation);
}

void ScTiledRenderingTest::testInsertDeletePageInvalidation()
{
    comphelper::LibreOfficeKit::setActive();

    ScModelObj* pModelObj = createDoc("insert_delete_sheet.ods");
    // the document has 1 sheet
    CPPUNIT_ASSERT_EQUAL(1, pModelObj->getParts());
    ScViewData* pViewData = ScDocShell::GetViewData();
    CPPUNIT_ASSERT(pViewData);

    int nView1 = SfxLokHelper::getView();
    ViewCallback aView1;
    CPPUNIT_ASSERT(!lcl_hasEditView(*pViewData));

    SfxLokHelper::setView(nView1);
    aView1.m_bInvalidateTiles = false;
    aView1.m_aInvalidations.clear();

    uno::Sequence<beans::PropertyValue> aArgs( comphelper::InitPropertySequence({
            { "Name", uno::Any(OUString("")) },
            { "Index", uno::Any(sal_Int32(1)) }
        }));
    comphelper::dispatchCommand(".uno:Insert", aArgs);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(aView1.m_bInvalidateTiles);
    CPPUNIT_ASSERT_EQUAL(size_t(6), aView1.m_aInvalidations.size());
    CPPUNIT_ASSERT_EQUAL(tools::Rectangle(0, 0, 1000000000, 1000000000), aView1.m_aInvalidations[0]);
    CPPUNIT_ASSERT_EQUAL(2, pModelObj->getParts());

    // Delete sheet
    aView1.m_bInvalidateTiles = false;
    aView1.m_aInvalidations.clear();
    uno::Sequence<beans::PropertyValue> aArgs2( comphelper::InitPropertySequence({
            { "Index", uno::Any(sal_Int32(1)) }
        }));
    comphelper::dispatchCommand(".uno:Remove", aArgs2);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(aView1.m_bInvalidateTiles);
    CPPUNIT_ASSERT_EQUAL(size_t(5), aView1.m_aInvalidations.size());
    CPPUNIT_ASSERT_EQUAL(tools::Rectangle(0, 0, 1000000000, 1000000000), aView1.m_aInvalidations[0]);
    CPPUNIT_ASSERT_EQUAL(1, pModelObj->getParts());
}

void ScTiledRenderingTest::testGetRowColumnHeadersInvalidation()
{
    comphelper::LibreOfficeKit::setActive();

    ScModelObj* pModelObj = createDoc("empty.ods");
    ScViewData* pViewData = ScDocShell::GetViewData();
    CPPUNIT_ASSERT(pViewData);

    int nView1 = SfxLokHelper::getView();
    ViewCallback aView1;
    CPPUNIT_ASSERT(!lcl_hasEditView(*pViewData));

    SfxLokHelper::setView(nView1);
    aView1.m_bInvalidateTiles = false;
    aView1.m_aInvalidations.clear();
    tools::JsonWriter aJsonWriter1;
    pModelObj->getRowColumnHeaders(tools::Rectangle(0, 15, 19650, 5400), aJsonWriter1);
    free(aJsonWriter1.extractData());
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(aView1.m_bInvalidateTiles);
    CPPUNIT_ASSERT_EQUAL(size_t(1), aView1.m_aInvalidations.size());
    CPPUNIT_ASSERT_EQUAL(tools::Rectangle(26775, 0, 49725, 13005), aView1.m_aInvalidations[0]);

    // Extend area top-to-bottom
    aView1.m_bInvalidateTiles = false;
    aView1.m_aInvalidations.clear();
    tools::JsonWriter aJsonWriter2;
    pModelObj->getRowColumnHeaders(tools::Rectangle(0, 5400, 19650, 9800), aJsonWriter2);
    free(aJsonWriter2.extractData());
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(aView1.m_bInvalidateTiles);
    CPPUNIT_ASSERT_EQUAL(size_t(1), aView1.m_aInvalidations.size());
    CPPUNIT_ASSERT_EQUAL(tools::Rectangle(0, 13005, 49725, 19380), aView1.m_aInvalidations[0]);

    // Extend area left-to-right
    aView1.m_bInvalidateTiles = false;
    aView1.m_aInvalidations.clear();
    tools::JsonWriter aJsonWriter3;
    pModelObj->getRowColumnHeaders(tools::Rectangle(5400, 5400, 25050, 9800), aJsonWriter3);
    free(aJsonWriter3.extractData());
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(aView1.m_bInvalidateTiles);
    CPPUNIT_ASSERT_EQUAL(size_t(1), aView1.m_aInvalidations.size());
    CPPUNIT_ASSERT_EQUAL(tools::Rectangle(49725, 0, 75225, 19380), aView1.m_aInvalidations[0]);
}

void ScTiledRenderingTest::testJumpHorizontallyInvalidation()
{
    comphelper::LibreOfficeKit::setActive();

    ScModelObj* pModelObj = createDoc("empty.ods");
    ScViewData* pViewData = ScDocShell::GetViewData();
    CPPUNIT_ASSERT(pViewData);

    int nView1 = SfxLokHelper::getView();
    ViewCallback aView1;
    CPPUNIT_ASSERT(!lcl_hasEditView(*pViewData));

    SfxLokHelper::setView(nView1);
    aView1.m_bInvalidateTiles = false;
    aView1.m_aInvalidations.clear();
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::PAGEDOWN | KEY_MOD2);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::PAGEDOWN | KEY_MOD2);
    Scheduler::ProcessEventsToIdle();
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::PAGEDOWN | KEY_MOD2);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::PAGEDOWN | KEY_MOD2);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(aView1.m_bInvalidateTiles);
    CPPUNIT_ASSERT_EQUAL(size_t(1), aView1.m_aInvalidations.size());
    CPPUNIT_ASSERT_EQUAL(tools::Rectangle(26775, 0, 39525, 13005), aView1.m_aInvalidations[0]);
}

void ScTiledRenderingTest::testJumpToLastRowInvalidation()
{
    comphelper::LibreOfficeKit::setActive();

    ScModelObj* pModelObj = createDoc("empty.ods");
    ScViewData* pViewData = ScDocShell::GetViewData();
    CPPUNIT_ASSERT(pViewData);

    int nView1 = SfxLokHelper::getView();
    ViewCallback aView1;
    CPPUNIT_ASSERT(!lcl_hasEditView(*pViewData));

    SfxLokHelper::setView(nView1);
    aView1.m_bInvalidateTiles = false;
    aView1.m_aInvalidations.clear();
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::DOWN | KEY_MOD1);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::DOWN | KEY_MOD1);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(aView1.m_bInvalidateTiles);
    CPPUNIT_ASSERT_EQUAL(size_t(1), aView1.m_aInvalidations.size());
    CPPUNIT_ASSERT_EQUAL(tools::Rectangle(0, 13005, 26775, 127500255), aView1.m_aInvalidations[0]);
}

// We need to ensure that views are not perterbed by rendering (!?) hmm ...
void ScTiledRenderingTest::testRowColumnHeaders()
{
    comphelper::LibreOfficeKit::setActive();

    ScModelObj* pModelObj = createDoc("empty.ods");
    ScViewData* pViewData = ScDocShell::GetViewData();
    CPPUNIT_ASSERT(pViewData);

    // view #1
    ViewCallback aView1;
    int nView1 = SfxLokHelper::getView();
    CPPUNIT_ASSERT(!lcl_hasEditView(*pViewData));

    // view #2
    SfxLokHelper::createView();
    int nView2 = SfxLokHelper::getView();
    ViewCallback aView2;
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());

    // ViewRowColumnHeaders test
    SfxLokHelper::setView(nView1);
    tools::JsonWriter aJsonWriter1;
    pModelObj->getRowColumnHeaders(tools::Rectangle(65,723,10410,4695), aJsonWriter1);
    OString aHeaders1 = aJsonWriter1.extractAsOString();

    SfxLokHelper::setView(nView2);
    // 50% zoom
    pModelObj->setClientVisibleArea(tools::Rectangle(0, 0, 22474, 47333));
    pModelObj->setClientZoom(256, 256, 6636, 6636);
    tools::JsonWriter aJsonWriter2;
    pModelObj->getRowColumnHeaders(tools::Rectangle(65,723,10410,4695), aJsonWriter2);
    OString aHeaders2 = aJsonWriter2.extractAsOString();

    // Check vs. view #1
    SfxLokHelper::setView(nView1);
    tools::JsonWriter aJsonWriter3;
    pModelObj->getRowColumnHeaders(tools::Rectangle(65,723,10410,4695), aJsonWriter3);
    OString aHeaders1_2 = aJsonWriter3.extractAsOString();
    CPPUNIT_ASSERT_EQUAL(aHeaders1, aHeaders1_2);

    // Check vs. view #2
    SfxLokHelper::setView(nView2);
    tools::JsonWriter aJsonWriter4;
    pModelObj->getRowColumnHeaders(tools::Rectangle(65,723,10410,4695), aJsonWriter4);
    OString aHeaders2_2 = aJsonWriter4.extractAsOString();
    CPPUNIT_ASSERT_EQUAL(aHeaders2, aHeaders2_2);

    SfxLokHelper::setView(nView1);
    SfxViewShell::Current()->setLibreOfficeKitViewCallback(nullptr);
    SfxLokHelper::setView(nView2);
    SfxViewShell::Current()->setLibreOfficeKitViewCallback(nullptr);
}

// Helper structs for setup and testing of ScModelObj::getSheetGeometryData()
struct SpanEntry
{
    size_t nVal;
    SCCOLROW nEnd;
};

struct SheetDimData
{
    typedef std::vector<SpanEntry> SpanList;
    SpanList aSizes;
    SpanList aHidden;
    SpanList aFiltered;
    // TODO: Add group info too to test.

    void setDataToDoc(ScDocument* pDoc, bool bCol) const
    {
        SCCOLROW nStart = 0;
        // sizes
        for (const auto& rSpan : aSizes)
        {
            if (bCol)
            {
                for (SCCOLROW nIdx = nStart; nIdx <= rSpan.nEnd; ++nIdx)
                    pDoc->SetColWidthOnly(nIdx, 0, rSpan.nVal);
            }
            else
                pDoc->SetRowHeightOnly(nStart, rSpan.nEnd, 0, rSpan.nVal);

            nStart = rSpan.nEnd + 1;
        }

        nStart = 0;
        // hidden
        for (const auto& rSpan : aHidden)
        {
            if (bCol)
                pDoc->SetColHidden(nStart, rSpan.nEnd, 0, !!rSpan.nVal);
            else
                pDoc->SetRowHidden(nStart, rSpan.nEnd, 0, !!rSpan.nVal);

            nStart = rSpan.nEnd + 1;
        }

        // There is no ScDocument interface to set ScTable::mpFilteredCols
        // It seems ScTable::mpFilteredCols is not really used !?
        if (bCol)
            return;

        nStart = 0;
        // filtered
        for (const auto& rSpan : aFiltered)
        {
            pDoc->SetRowFiltered(nStart, rSpan.nEnd, 0, !!rSpan.nVal);
            nStart = rSpan.nEnd + 1;
        }
    }

    void testPropertyTree(const boost::property_tree::ptree& rTree, bool bCol) const
    {
        struct SpanListWithKey
        {
            OString aKey;
            const SpanList& rSpanList;
        };

        const SpanListWithKey aPairList[] = {
            { "sizes",    aSizes    },
            { "hidden",   aHidden   },
            { "filtered", aFiltered }
        };

        for (const auto& rEntry : aPairList)
        {
            // There is no ScDocument interface to set ScTable::mpFilteredCols
            // It seems ScTable::mpFilteredCols is not really used !?
            if (bCol && rEntry.aKey == "filtered")
                continue;

            bool bBooleanValue = rEntry.aKey != "sizes";
            OString aExpectedEncoding;
            bool bFirst = true;
            for (const auto& rSpan : rEntry.rSpanList)
            {
                size_t nVal = rSpan.nVal;
                if (bBooleanValue && bFirst)
                    nVal = static_cast<size_t>(!!nVal);
                if (!bBooleanValue || bFirst)
                    aExpectedEncoding += OString::number(nVal) + ":";
                aExpectedEncoding += OString::number(rSpan.nEnd) + " ";
                bFirst = false;
            }

            // Get the tree's value for the property key ("sizes"/"hidden"/"filtered").
            OString aTreeValue = rTree.get<std::string>(rEntry.aKey.getStr()).c_str();

            CPPUNIT_ASSERT_EQUAL(aExpectedEncoding, aTreeValue);
        }
    }
};

class SheetGeometryData
{
    SheetDimData aCols;
    SheetDimData aRows;

public:

    SheetGeometryData()
    {
        aCols = {
            // width spans
            { { STD_COL_WIDTH, MAXCOL } },
            // hidden spans
            { { 0,             MAXCOL } },
            // filtered spans
            { { 0,             MAXCOL } }
        };

        aRows = {
            // height spans
            { { ScGlobal::nStdRowHeight, MAXROW } },
            // hidden spans
            { { 0,                       MAXROW } },
            // filtered spans
            { { 0,                       MAXROW } }
        };
    }

    SheetGeometryData(const SheetDimData& rCols, const SheetDimData& rRows) :
        aCols(rCols), aRows(rRows)
    {}

    void setDataToDoc(ScDocument* pDoc) const
    {
        aCols.setDataToDoc(pDoc, true);
        aRows.setDataToDoc(pDoc, false);
    }

    void parseTest(const OString& rJSON) const
    {
        // Assumes all flags passed to getSheetGeometryData() are true.
        boost::property_tree::ptree aTree;
        std::stringstream aStream(rJSON.getStr());
        boost::property_tree::read_json(aStream, aTree);

        CPPUNIT_ASSERT_EQUAL(OString(".uno:SheetGeometryData"), OString(aTree.get<std::string>("commandName").c_str()));

        aCols.testPropertyTree(aTree.get_child("columns"), true);
        aRows.testPropertyTree(aTree.get_child("rows"), false);
    }
};

// getSheetGeometryData() should return the exact same message
// irrespective of client zoom and view-area. Switching views
// should also not alter it.
void ScTiledRenderingTest::testSheetGeometryDataInvariance()
{
    comphelper::LibreOfficeKit::setActive();

    ScModelObj* pModelObj = createDoc("empty.ods");
    const SheetGeometryData aSGData(
        // cols
        {
            // width spans
            {
                { STD_COL_WIDTH,   20     },
                { 2*STD_COL_WIDTH, 26     },
                { STD_COL_WIDTH,   MAXCOL }
            },

            // hidden spans
            {
                { 0, 5      },
                { 1, 12     },
                { 0, MAXCOL }
            },

            // filtered spans
            {
                { 0, 50     },
                { 1, 59     },
                { 0, MAXCOL }
            }
        },

        // rows
        {
            // height spans
            {
                { 300,  50     },
                { 600,  65     },
                { 300,  MAXROW }
            },

            // hidden spans
            {
                { 1, 100    },
                { 0, 500    },
                { 1, 578    },
                { 0, MAXROW }
            },

            // filtered spans
            {
                { 0, 150    },
                { 1, 159    },
                { 0, MAXROW }
            }
        }
    );

    ScDocument* pDoc = pModelObj->GetDocument();

    ScViewData* pViewData = ScDocShell::GetViewData();
    CPPUNIT_ASSERT(pViewData);

    // view #1
    ViewCallback aView1;
    int nView1 = SfxLokHelper::getView();

    // view #2
    SfxLokHelper::createView();
    int nView2 = SfxLokHelper::getView();
    ViewCallback aView2;
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());

    // Try with the default empty document once (nIdx = 0) and then with sheet geometry settings (nIdx = 1)
    for (size_t nIdx = 0; nIdx < 2; ++nIdx)
    {
        if (nIdx)
            aSGData.setDataToDoc(pDoc);

        SfxLokHelper::setView(nView1);
        OString aGeomStr1 = pModelObj->getSheetGeometryData(/*bColumns*/ true, /*bRows*/ true, /*bSizes*/ true,
                /*bHidden*/ true, /*bFiltered*/ true, /*bGroups*/ true);

        SfxLokHelper::setView(nView2);
        pModelObj->setClientVisibleArea(tools::Rectangle(0, 0, 22474, 47333));
        pModelObj->setClientZoom(256, 256, 6636, 6636);
        OString aGeomStr2 = pModelObj->getSheetGeometryData(/*bColumns*/ true, /*bRows*/ true, /*bSizes*/ true,
                /*bHidden*/ true, /*bFiltered*/ true, /*bGroups*/ true);

        // Check vs. view #1
        SfxLokHelper::setView(nView1);
        OString aGeomStr1_2 = pModelObj->getSheetGeometryData(/*bColumns*/ true, /*bRows*/ true, /*bSizes*/ true,
                /*bHidden*/ true, /*bFiltered*/ true, /*bGroups*/ true);
        CPPUNIT_ASSERT_EQUAL(aGeomStr1, aGeomStr1_2);

        // Check vs. view #2
        SfxLokHelper::setView(nView2);
        OString aGeomStr2_2 = pModelObj->getSheetGeometryData(/*bColumns*/ true, /*bRows*/ true, /*bSizes*/ true,
                /*bHidden*/ true, /*bFiltered*/ true, /*bGroups*/ true);
        CPPUNIT_ASSERT_EQUAL(aGeomStr2, aGeomStr2_2);
    }

    SfxLokHelper::setView(nView1);
    SfxViewShell::Current()->setLibreOfficeKitViewCallback(nullptr);
    SfxLokHelper::setView(nView2);
    SfxViewShell::Current()->setLibreOfficeKitViewCallback(nullptr);
}

void ScTiledRenderingTest::testSheetGeometryDataCorrectness()
{
    comphelper::LibreOfficeKit::setActive();

    ScModelObj* pModelObj = createDoc("empty.ods");
    const SheetGeometryData aSGData(
        // cols
        {
            // width spans
            {
                { STD_COL_WIDTH,   20     },
                { 2*STD_COL_WIDTH, 26     },
                { STD_COL_WIDTH,   MAXCOL }
            },

            // hidden spans
            {
                { 0, 5      },
                { 1, 12     },
                { 0, MAXCOL }
            },

            // filtered spans
            {
                { 0, 50     },
                { 1, 59     },
                { 0, MAXCOL }
            }
        },

        // rows
        {
            // height spans
            {
                { 300,  50     },
                { 600,  65     },
                { 300,  MAXROW }
            },

            // hidden spans
            {
                { 1, 100    },
                { 0, 500    },
                { 1, 578    },
                { 0, MAXROW }
            },

            // filtered spans
            {
                { 0, 150    },
                { 1, 159    },
                { 0, MAXROW }
            }
        }
    );

    ScDocument* pDoc = pModelObj->GetDocument();

    ScViewData* pViewData = ScDocShell::GetViewData();
    CPPUNIT_ASSERT(pViewData);

    // view #1
    ViewCallback aView1;

    // with the default empty sheet and test the JSON encoding.
    OString aGeomDefaultStr = pModelObj->getSheetGeometryData(/*bColumns*/ true, /*bRows*/ true, /*bSizes*/ true,
            /*bHidden*/ true, /*bFiltered*/ true, /*bGroups*/ true);
    SheetGeometryData().parseTest(aGeomDefaultStr);

    // Apply geometry settings to the sheet and then test the resulting JSON encoding.
    aSGData.setDataToDoc(pDoc);
    OString aGeomStr = pModelObj->getSheetGeometryData(/*bColumns*/ true, /*bRows*/ true, /*bSizes*/ true,
            /*bHidden*/ true, /*bFiltered*/ true, /*bGroups*/ true);
    aSGData.parseTest(aGeomStr);

    SfxViewShell::Current()->setLibreOfficeKitViewCallback(nullptr);
}

void ScTiledRenderingTest::testDeleteCellMultilineContent()
{
    comphelper::LibreOfficeKit::setActive();

    ScModelObj* pModelObj = createDoc("multiline.ods");
    CPPUNIT_ASSERT(pModelObj);
    ScViewData* pViewData = ScDocShell::GetViewData();
    CPPUNIT_ASSERT(pViewData);
    ScDocShell* pDocSh = dynamic_cast< ScDocShell* >( pModelObj->GetEmbeddedObject() );
    CPPUNIT_ASSERT(pDocSh);

    // view #1
    ViewCallback aView1;
    CPPUNIT_ASSERT(!lcl_hasEditView(*pViewData));

    aView1.m_sInvalidateHeader = "";
    ScDocument& rDoc = pDocSh->GetDocument();
    sal_uInt16 nRow1Height = rDoc.GetRowHeight(static_cast<SCROW>(0), static_cast<SCTAB>(0), false);

    // delete multiline cell content in view #1
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::DOWN);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::DOWN);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::DELETE);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::DELETE);
    Scheduler::ProcessEventsToIdle();

    // check if the row header has been invalidated and if the involved row is of the expected height
    CPPUNIT_ASSERT_EQUAL(OString("row"), aView1.m_sInvalidateHeader);
    sal_uInt16 nRow2Height = rDoc.GetRowHeight(static_cast<SCROW>(0), static_cast<SCTAB>(0), false);
    CPPUNIT_ASSERT_EQUAL(nRow1Height, nRow2Height);
    SfxViewShell::Current()->setLibreOfficeKitViewCallback(nullptr);
}

void ScTiledRenderingTest::testPasteIntoWrapTextCell()
{
    comphelper::LibreOfficeKit::setActive();
    comphelper::LibreOfficeKit::setCompatFlag(
        comphelper::LibreOfficeKit::Compat::scPrintTwipsMsgs);

    ScModelObj* pModelObj = createDoc("empty.ods");
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();

    // Set Wrap text in A3
    pDoc->ApplyAttr(0, 2, 0, ScLineBreakCell(true));
    const SfxBoolItem* pItem = pDoc->GetAttr(0, 2, 0, ATTR_LINEBREAK);
    CPPUNIT_ASSERT(pItem->GetValue());

    ScViewData* pViewData = ScDocShell::GetViewData();
    CPPUNIT_ASSERT(pViewData);

    ViewCallback aView;
    CPPUNIT_ASSERT(!lcl_hasEditView(*pViewData));

    ScTabViewShell* pView = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
    CPPUNIT_ASSERT(pView);

    // create source text in A1
    OUString sCopyContent("Very long text to copy");
    pDoc->SetString(0, 0, 0, sCopyContent);

    // copy A1
    pView->SetCursor(0, 0);
    Scheduler::ProcessEventsToIdle();
    pView->GetViewFrame()->GetBindings().Execute(SID_COPY);
    Scheduler::ProcessEventsToIdle();

    // verify clipboard
    uno::Reference<datatransfer::clipboard::XClipboard> xClipboard1 = pView->GetViewData().GetActiveWin()->GetClipboard();
    uno::Reference< datatransfer::XTransferable > xDataObj =
        xClipboard1->getContents();
    datatransfer::DataFlavor aFlavor;
    SotExchange::GetFormatDataFlavor(SotClipboardFormatId::STRING, aFlavor);
    uno::Any aData = xDataObj->getTransferData(aFlavor);
    OUString aTmpText;
    aData >>= aTmpText;
    CPPUNIT_ASSERT_EQUAL(sCopyContent, aTmpText.trim());

    // Go to A2 and paste.
    pView->SetCursor(0, 1);
    Scheduler::ProcessEventsToIdle();
    aView.m_sInvalidateSheetGeometry = "";
    pView->GetViewFrame()->GetBindings().Execute(SID_PASTE);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(sCopyContent, pDoc->GetString(0, 1, 0));
    CPPUNIT_ASSERT_EQUAL(OString("rows sizes"), aView.m_sInvalidateSheetGeometry);

    // create new source text in A2
    OUString sCopyContent2("Very long text to copy 2");
    pDoc->SetString(0, 1, 0, sCopyContent2);
    Scheduler::ProcessEventsToIdle();

    // cut from A2
    pView->GetViewFrame()->GetBindings().Execute(SID_CUT);
    Scheduler::ProcessEventsToIdle();

    // verify clipboard
    uno::Reference<datatransfer::clipboard::XClipboard> xClipboard2
        = pView->GetViewData().GetActiveWin()->GetClipboard();
    xDataObj = xClipboard2->getContents();
    SotExchange::GetFormatDataFlavor(SotClipboardFormatId::STRING, aFlavor);
    aData = xDataObj->getTransferData(aFlavor);
    aData >>= aTmpText;
    CPPUNIT_ASSERT_EQUAL(xClipboard1, xClipboard2);
    CPPUNIT_ASSERT_EQUAL(sCopyContent2, aTmpText.trim());

    // Go to A3 and paste.
    pView->SetCursor(0, 2);
    Scheduler::ProcessEventsToIdle();
    aView.m_sInvalidateSheetGeometry = "";
    pView->GetViewFrame()->GetBindings().Execute(SID_PASTE);
    Scheduler::ProcessEventsToIdle();

    // SG invalidations for all
    CPPUNIT_ASSERT_EQUAL(sCopyContent2, pDoc->GetString(0, 1, 0));
    CPPUNIT_ASSERT_EQUAL(OString("all"), aView.m_sInvalidateSheetGeometry);

    SfxViewShell::Current()->setLibreOfficeKitViewCallback(nullptr);
}

void ScTiledRenderingTest::testSortAscendingDescending()
{
    comphelper::LibreOfficeKit::setActive();
    comphelper::LibreOfficeKit::setCompatFlag(
        comphelper::LibreOfficeKit::Compat::scPrintTwipsMsgs);
    ScModelObj* pModelObj = createDoc("sort-range.ods");
    ScDocument* pDoc = pModelObj->GetDocument();

    ViewCallback aView;

    // select the values in the first column
    pModelObj->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONDOWN, 551, 129, 1, MOUSE_LEFT, 0);
    pModelObj->postMouseEvent(LOK_MOUSEEVENT_MOUSEMOVE, 820, 1336, 1, MOUSE_LEFT, 0);
    pModelObj->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONUP, 820, 1359, 1, MOUSE_LEFT, 0);
    Scheduler::ProcessEventsToIdle();
    aView.m_sInvalidateSheetGeometry = "";

    // sort ascending
    uno::Sequence<beans::PropertyValue> aArgs;
    comphelper::dispatchCommand(".uno:SortAscending", aArgs);

    // check it's sorted
    for (SCROW r = 0; r < 6; ++r)
    {
        CPPUNIT_ASSERT_EQUAL(double(r + 1), pDoc->GetValue(ScAddress(0, r, 0)));
    }

    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(OString("rows"), aView.m_sInvalidateSheetGeometry);

    aView.m_sInvalidateSheetGeometry = "";
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

    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(OString("rows"), aView.m_sInvalidateSheetGeometry);
}

void lcl_typeCharsInCell(const std::string& aStr, SCCOL nCol, SCROW nRow, ScTabViewShell* pView, ScModelObj* pModelObj)
{
    pView->SetCursor(nCol, nRow);
    for (const char& cChar : aStr)
    {
        pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, cChar, 0);
        pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, cChar, 0);
        Scheduler::ProcessEventsToIdle();
    }
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::RETURN);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::RETURN);
    Scheduler::ProcessEventsToIdle();
}

void ScTiledRenderingTest::testAutoInputExactMatch()
{
    comphelper::LibreOfficeKit::setActive();

    ScModelObj* pModelObj = createDoc("empty.ods");
    CPPUNIT_ASSERT(pModelObj);
    ScTabViewShell* pView = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
    CPPUNIT_ASSERT(pView);
    ScDocument* pDoc = pModelObj->GetDocument();

    pDoc->SetString(ScAddress(0, 1, 0), "Simple");  // A2
    pDoc->SetString(ScAddress(0, 2, 0), "Simple");  // A3
    pDoc->SetString(ScAddress(0, 3, 0), "Sing");  // A4
    ScFieldEditEngine& rEE = pDoc->GetEditEngine();
    rEE.SetText("Case");
    pDoc->SetEditText(ScAddress(0, 4, 0), rEE.CreateTextObject()); // A5
    pDoc->SetString(ScAddress(0, 5, 0), "Time");  // A6
    pDoc->SetString(ScAddress(0, 6, 0), "Castle");  // A7

    ScAddress aA8(0, 7, 0);
    lcl_typeCharsInCell("S", aA8.Col(), aA8.Row(), pView, pModelObj); // Type "S" in A8
    // Should not autocomplete as there are multiple matches starting with "S".
    CPPUNIT_ASSERT_EQUAL_MESSAGE("1: A8 should have just S (should not autocomplete)", OUString("S"), pDoc->GetString(aA8));

    lcl_typeCharsInCell("Si", aA8.Col(), aA8.Row(), pView, pModelObj); // Type "Si" in A8
    // Should not autocomplete as there are multiple matches starting with "Si".
    CPPUNIT_ASSERT_EQUAL_MESSAGE("2: A8 should not autocomplete", OUString("Si"), pDoc->GetString(aA8));

    lcl_typeCharsInCell("Sim", aA8.Col(), aA8.Row(), pView, pModelObj); // Type "Sim" in A8
    // Should autocomplete to "Simple" which is the only match.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("3: A8 should autocomplete", OUString("Simple"), pDoc->GetString(aA8));

    lcl_typeCharsInCell("Sin", aA8.Col(), aA8.Row(), pView, pModelObj); // Type "Sin" in A8
    // Should autocomplete to "Sing" which is the only match.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("4: A8 should autocomplete", OUString("Sing"), pDoc->GetString(aA8));

    lcl_typeCharsInCell("Cas", aA8.Col(), aA8.Row(), pView, pModelObj); // Type "Cas" in A8
    // Should not autocomplete as there are multiple matches starting with "Cas".
    CPPUNIT_ASSERT_EQUAL_MESSAGE("5: A8 should not autocomplete", OUString("Cas"), pDoc->GetString(aA8));

    lcl_typeCharsInCell("Cast", aA8.Col(), aA8.Row(), pView, pModelObj); // Type "Cast" in A8
    // Should autocomplete to "Castle" which is the only match.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("6: A8 should autocomplete", OUString("Castle"), pDoc->GetString(aA8));

    lcl_typeCharsInCell("T", aA8.Col(), aA8.Row(), pView, pModelObj); // Type "T" in A8
    // Should autocomplete to "Time" which is the only match.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("7: A8 should autocomplete", OUString("Time"), pDoc->GetString(aA8));
}

void ScTiledRenderingTest::testEditCursorBounds()
{
    comphelper::LibreOfficeKit::setActive();
    comphelper::LibreOfficeKit::setCompatFlag(
        comphelper::LibreOfficeKit::Compat::scPrintTwipsMsgs);
    ScModelObj* pModelObj = createDoc("empty.ods");
    ScDocument* pDoc = pModelObj->GetDocument();

    ViewCallback aView;
    ScTabViewShell* pView = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
    CPPUNIT_ASSERT(pView);
    comphelper::LibreOfficeKit::setViewIdForVisCursorInvalidation(true);

    // ~170% zoom.
    pModelObj->setClientZoom(256, 256, 2222, 2222);
    pModelObj->setClientVisibleArea(tools::Rectangle(7725, 379832, 16240, 6449));
    Scheduler::ProcessEventsToIdle();

    constexpr SCCOL nCol = 5;
    constexpr SCROW nRow = 2048;
    pDoc->SetValue(ScAddress(nCol, nRow, 0), 123);

    aView.m_bOwnCursorInvalidated = false;
    // Obtain the cell bounds via cursor.
    pView->SetCursor(nCol, nRow);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT(aView.m_bOwnCursorInvalidated);
    CPPUNIT_ASSERT(!aView.m_aCellCursorBounds.IsEmpty());
    tools::Rectangle aCellBounds(aView.m_aCellCursorBounds);

    aView.m_aInvalidateCursorResult.clear();
    // Enter edit mode in the same cell.
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::F2);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::F2);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT(!aView.m_aInvalidateCursorResult.empty());
    CPPUNIT_ASSERT_MESSAGE("Edit cursor must be in cell bounds!",
        aCellBounds.IsInside(aView.m_aInvalidateCursorResult.getBounds()));

    SfxViewShell::Current()->setLibreOfficeKitViewCallback(nullptr);
}

void ScTiledRenderingTest::testTextSelectionBounds()
{
    comphelper::LibreOfficeKit::setActive();
    comphelper::LibreOfficeKit::setCompatFlag(
        comphelper::LibreOfficeKit::Compat::scPrintTwipsMsgs);
    ScModelObj* pModelObj = createDoc("empty.ods");
    ScDocument* pDoc = pModelObj->GetDocument();

    ViewCallback aView;
    ScTabViewShell* pView = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
    CPPUNIT_ASSERT(pView);
    comphelper::LibreOfficeKit::setViewIdForVisCursorInvalidation(true);

    // ~170% zoom.
    pModelObj->setClientZoom(256, 256, 2222, 2222);
    pModelObj->setClientVisibleArea(tools::Rectangle(7725, 379832, 16240, 6449));
    Scheduler::ProcessEventsToIdle();

    constexpr SCCOL nCol = 5;
    constexpr SCROW nRow = 2048;
    pDoc->SetValue(ScAddress(nCol, nRow, 0), 123);

    aView.m_bOwnCursorInvalidated = false;
    // Obtain the cell bounds via cursor.
    pView->SetCursor(nCol, nRow);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT(aView.m_bOwnCursorInvalidated);
    CPPUNIT_ASSERT(!aView.m_aCellCursorBounds.IsEmpty());
    tools::Rectangle aCellBounds(aView.m_aCellCursorBounds);

    aView.m_aTextSelectionResult.clear();
    // Enter edit mode in the same cell and select all text.
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::F2);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::F2);
    Scheduler::ProcessEventsToIdle();

    // CTRL + A
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_MOD1 | awt::Key::A);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_MOD1 | awt::Key::A);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT(!aView.m_aTextSelectionResult.empty());
    CPPUNIT_ASSERT_MESSAGE("Text selections must be in cell bounds!",
        !aCellBounds.Intersection(aView.m_aTextSelectionResult.getBounds(0)).IsEmpty());

    SfxViewShell::Current()->setLibreOfficeKitViewCallback(nullptr);
}

void ScTiledRenderingTest::testSheetViewDataCrash()
{
    comphelper::LibreOfficeKit::setActive();

    ScModelObj* pModelObj = createDoc("empty.ods");

    // view #1
    int nView1 = SfxLokHelper::getView();
    SfxLokHelper::setView(nView1);

    // Imitate online while creating a new sheet on empty.ods.
    uno::Sequence<beans::PropertyValue> aArgs(
        comphelper::InitPropertySequence({
            { "Name",  uno::Any(OUString("NewSheet")) },
            { "Index", uno::Any(sal_Int32(2)) }
        }));
    comphelper::dispatchCommand(".uno:Insert", aArgs);
    Scheduler::ProcessEventsToIdle();
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::PAGEDOWN | KEY_MOD1);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::PAGEDOWN | KEY_MOD1);
    Scheduler::ProcessEventsToIdle();
    ScTabViewShell* pView1 = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
    CPPUNIT_ASSERT(pView1);

    // view #2
    SfxLokHelper::createView();
    ScTabViewShell* pView2 = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
    CPPUNIT_ASSERT(pView2);
    Scheduler::ProcessEventsToIdle();

    SfxLokHelper::setView(nView1);
    // Delete a range.
    pView1->SetCursor(1, 1);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::DOWN | KEY_SHIFT);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::DOWN | KEY_SHIFT);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::DELETE);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::DELETE);
    // It will crash at this point without the fix.
    Scheduler::ProcessEventsToIdle();
}

}

CPPUNIT_TEST_SUITE_REGISTRATION(ScTiledRenderingTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
