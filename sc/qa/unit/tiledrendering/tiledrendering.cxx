/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/cppunitasserthelper.hxx>
#include <test/unoapixml_test.hxx>
#include <test/helper/transferable.hxx>
#include <cppunit/tools/StringHelper.h>
#include <boost/property_tree/json_parser.hpp>

#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/DispatchHelper.hpp>
#include <com/sun/star/datatransfer/clipboard/LokClipboard.hpp>
#include <com/sun/star/datatransfer/UnsupportedFlavorException.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
#include <osl/conditn.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/stritem.hxx>

#include <comphelper/lok.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/dispatchcommand.hxx>
#include <sfx2/msgpool.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/lokhelper.hxx>
#include <svx/svdpage.hxx>
#include <vcl/scheduler.hxx>
#include <vcl/vclevent.hxx>
#include <vcl/virdev.hxx>
#include <sc.hrc>
#include <comphelper/string.hxx>
#include <tools/json_writer.hxx>
#include <docoptio.hxx>
#include <postit.hxx>
#include <test/lokcallback.hxx>
#include <osl/file.hxx>
#include <unotools/tempfile.hxx>

#include <chrono>
#include <cstddef>

#include <attrib.hxx>
#include <scitems.hxx>
#include <tabvwsh.hxx>
#include <docsh.hxx>
#include <document.hxx>
#include <docuno.hxx>
#include <drwlayer.hxx>
#include <editutil.hxx>
#include <undomanager.hxx>
#include <columnspanset.hxx>

using namespace css;

static std::ostream& operator<<(std::ostream& os, ViewShellId const & id)
{
    os << static_cast<sal_Int32>(id); return os;
}

// for passing data to testInvalidateOnTextEditWithDifferentZoomLevels
struct ColRowZoom
{
    SCCOL col;
    SCROW row;
    int zoom;
};

CPPUNIT_NS_BEGIN
namespace StringHelper
{
// used by CPPUNIT_TEST_PARAMETERIZED for testInvalidateOnTextEditWithDifferentZoomLevels
template<>
inline std::string toString(const ColRowZoom& item)
{
    std::ostringstream ss;
    ss << "zoom level: " << item.zoom << ", "
       << "col: " << item.col << ", "
       << "row: " << item.row;
    return ss.str();
}

}
CPPUNIT_NS_END

namespace
{

class ViewCallback;

class ScTiledRenderingTest : public UnoApiXmlTest
{
public:
    ScTiledRenderingTest();
    virtual void setUp() override;
    virtual void tearDown() override;

    void checkSampleInvalidation(const ViewCallback& rView, bool bFullRow);
    void cellInvalidationHelper(ScModelObj* pModelObj, ScTabViewShell* pView,
                                const ScAddress& rAdr, bool bAddText,
                                bool bFullRow);

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
    void testSheetChangeNoInvalidation();
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
    void testAutoInputStringBlock();
    void testAutoInputExactMatch();
    void testMoveShapeHandle();
    void testEditCursorBounds();
    void testTextSelectionBounds();
    void testSheetViewDataCrash();
    void testTextBoxInsert();
    void testCommentCellCopyPaste();
    void testInvalidEntrySave();
    void testUndoReordering();
    void testUndoReorderingRedo();
    void testUndoReorderingMulti();
    void testGetViewRenderState();
    void testInvalidateOnTextEditWithDifferentZoomLevels(const ColRowZoom& rData);
    void testOpenURL();
    void testInvalidateForSplitPanes();
    void testStatusBarLocale();
    void testLongFirstColumnMouseClick();
    void testSidebarLocale();
    void testNoInvalidateOnSave();
    void testCellMinimalInvalidations();
    void testCellInvalidationDocWithExistingZoom();
    void testInputHandlerSyncedZoom();
    void testOptimalRowHeight();
    void testExtendedAreasDontOverlap();
    void testEditShapeText();
    void testCopyMultiSelection();

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
    CPPUNIT_TEST(testSheetChangeNoInvalidation);
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
    CPPUNIT_TEST(testAutoInputStringBlock);
    CPPUNIT_TEST(testAutoInputExactMatch);
    CPPUNIT_TEST(testMoveShapeHandle);
    CPPUNIT_TEST(testEditCursorBounds);
    CPPUNIT_TEST(testTextSelectionBounds);
    CPPUNIT_TEST(testSheetViewDataCrash);
    CPPUNIT_TEST(testTextBoxInsert);
    CPPUNIT_TEST(testCommentCellCopyPaste);
    CPPUNIT_TEST(testInvalidEntrySave);
    CPPUNIT_TEST(testUndoReordering);
    CPPUNIT_TEST(testUndoReorderingRedo);
    CPPUNIT_TEST(testUndoReorderingMulti);
    CPPUNIT_TEST(testGetViewRenderState);
    CPPUNIT_TEST_PARAMETERIZED(testInvalidateOnTextEditWithDifferentZoomLevels,
                               std::initializer_list<ColRowZoom>
                               {
                                   // zoom level 120%
                                   {0, 999, 1}, {99, 0, 1},
                                   // zoom level 40%
                                   {0, 999, -5}, {99, 0, -5}
                               });
    CPPUNIT_TEST(testOpenURL);
    CPPUNIT_TEST(testInvalidateForSplitPanes);
    CPPUNIT_TEST(testStatusBarLocale);
    CPPUNIT_TEST(testLongFirstColumnMouseClick);
    CPPUNIT_TEST(testSidebarLocale);
    CPPUNIT_TEST(testNoInvalidateOnSave);
    CPPUNIT_TEST(testCellMinimalInvalidations);
    CPPUNIT_TEST(testCellInvalidationDocWithExistingZoom);
    CPPUNIT_TEST(testInputHandlerSyncedZoom);
    CPPUNIT_TEST(testOptimalRowHeight);
    CPPUNIT_TEST(testExtendedAreasDontOverlap);
    CPPUNIT_TEST(testEditShapeText);
    CPPUNIT_TEST(testCopyMultiSelection);
    CPPUNIT_TEST_SUITE_END();

private:
    ScModelObj* createDoc(const char* pName);
    void setupLibreOfficeKitViewCallback(SfxViewShell* pViewShell);
    static void callback(int nType, const char* pPayload, void* pData);
    void callbackImpl(int nType, const char* pPayload);

    /// document size changed callback.
    osl::Condition m_aDocSizeCondition;
    Size m_aDocumentSize;

    TestLokCallbackWrapper m_callbackWrapper;
};

ScTiledRenderingTest::ScTiledRenderingTest()
    : UnoApiXmlTest("/sc/qa/unit/tiledrendering/data/"),
    m_callbackWrapper(&callback, this)
{
}

void ScTiledRenderingTest::setUp()
{
    UnoApiXmlTest::setUp();

    comphelper::LibreOfficeKit::setActive(true);
}

void ScTiledRenderingTest::tearDown()
{
    if (mxComponent.is())
    {
        mxComponent->dispose();
        mxComponent.clear();
    }

    m_callbackWrapper.clear();
    comphelper::LibreOfficeKit::setActive(false);

    UnoApiXmlTest::tearDown();
}

ScModelObj* ScTiledRenderingTest::createDoc(const char* pName)
{
    loadFromURL(OUString::createFromAscii(pName));

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
    rRectangle.SetLeft(aSeq[0].toInt32());
    rRectangle.SetTop(aSeq[1].toInt32());
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
    ScModelObj* pModelObj = createDoc("select-row-cols.ods");

    // Select the 5th row with no modifier
    uno::Sequence<beans::PropertyValue> aArgs( comphelper::InitPropertySequence({
            { "Row", uno::Any(sal_Int32(5 - 1)) },
            { "Modifier", uno::Any(sal_uInt16(0)) }
        }));
    dispatchCommand(mxComponent, ".uno:SelectRow", aArgs);

    // Check if it is selected
    OString aResult = apitest::helper::transferable::getTextSelection(pModelObj->getSelection(), "text/plain;charset=utf-8");
    OString aExpected("1\t2\t3\t4\t5\t6\t7\t8\t9\t10\t11\t12\t13\t14\t15\t16\t17\t18\t19\t20\t21\n");
    CPPUNIT_ASSERT_EQUAL(aExpected, aResult);

    // Select the 10th row with shift modifier
    aArgs = comphelper::InitPropertySequence({ { "Row", uno::Any(static_cast<sal_Int32>(10 - 1)) },
                                               { "Modifier", uno::Any(KEY_SHIFT) } });
    dispatchCommand(mxComponent, ".uno:SelectRow", aArgs);

    // Check if all the rows from 5th to 10th get selected
    aResult = apitest::helper::transferable::getTextSelection(pModelObj->getSelection(), "text/plain;charset=utf-8");
    aExpected = "1\t2\t3\t4\t5\t6\t7\t8\t9\t10\t11\t12\t13\t14\t15\t16\t17\t18\t19\t20\t21\n2\t3\t4\t5\t6\t7\t8\t9\t10\t11\t12\t13\t14\t15\t16\t17\t18\t19\t20\t21\t22\n3\t4\t5\t6\t7\t8\t9\t10\t11\t12\t13\t14\t15\t16\t17\t18\t19\t20\t21\t22\t23\n4\t5\t6\t7\t8\t9\t10\t11\t12\t13\t14\t15\t16\t17\t18\t19\t20\t21\t22\t23\t24\n5\t6\t7\t8\t9\t10\t11\t12\t13\t14\t15\t16\t17\t18\t19\t20\t21\t22\t23\t24\t25\n6\t7\t8\t9\t10\t11\t12\t13\t14\t15\t16\t17\t18\t19\t20\t21\t22\t23\t24\t25\t26\n";
    CPPUNIT_ASSERT_EQUAL(aExpected, aResult);

    // Select the 10th row with ctrl modifier
    aArgs = comphelper::InitPropertySequence({ { "Row", uno::Any(static_cast<sal_Int32>(13 - 1)) },
                                               { "Modifier", uno::Any(KEY_MOD1) } });
    dispatchCommand(mxComponent, ".uno:SelectRow", aArgs);

    // When we copy this, we don't get anything useful, but we must not crash
    // (used to happen)
    aResult = apitest::helper::transferable::getTextSelection(pModelObj->getSelection(), "text/plain;charset=utf-8");
    CPPUNIT_ASSERT_EQUAL(OString(), aResult);

    // TODO check that we really selected what we wanted here

    // Select Column 5 with ctrl modifier
    aArgs = comphelper::InitPropertySequence({ { "Col", uno::Any(static_cast<sal_Int32>(5 - 1)) },
                                               { "Modifier", uno::Any(KEY_MOD1) } });
    dispatchCommand(mxComponent, ".uno:SelectColumn", aArgs);

    // When we copy this, we don't get anything useful, but we must not crash
    // (used to happen)
    aResult = apitest::helper::transferable::getTextSelection(pModelObj->getSelection(), "text/plain;charset=utf-8");
    CPPUNIT_ASSERT_EQUAL(OString(), aResult);

    // TODO check that we really selected what we wanted here

    // Test for deselection of already selected rows
    // First Deselect Row 13 because copy doesn't work for multiple selections
    aArgs = comphelper::InitPropertySequence({ { "Row", uno::Any(static_cast<sal_Int32>(13 - 1)) },
                                               { "Modifier", uno::Any(KEY_MOD1) } });
    dispatchCommand(mxComponent, ".uno:SelectRow", aArgs);

    // Deselect row 10
    aArgs = comphelper::InitPropertySequence({ { "Row", uno::Any(static_cast<sal_Int32>(10 - 1)) },
                                               { "Modifier", uno::Any(KEY_MOD1) } });
    dispatchCommand(mxComponent, ".uno:SelectRow", aArgs);

    // Click at row 6 holding shift
    aArgs = comphelper::InitPropertySequence({ { "Row", uno::Any(static_cast<sal_Int32>(6 - 1)) },
                                               { "Modifier", uno::Any(KEY_SHIFT) } });
    dispatchCommand(mxComponent, ".uno:SelectRow", aArgs);

    //  only row 5 should remain selected
    aResult = apitest::helper::transferable::getTextSelection(pModelObj->getSelection(), "text/plain;charset=utf-8");
    aExpected = "1\t2\t3\t4\t5\t6\t7\t8\t9\t10\t11\t12\t13\t14\t15\t16\t17\t18\t19\t20\t21\n";
    CPPUNIT_ASSERT_EQUAL(aExpected, aResult);
}

void ScTiledRenderingTest::testPartHash()
{
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
    ScModelObj* pModelObj = createDoc("select-row-cols.ods");

    // Select empty column, 1000
    uno::Sequence<beans::PropertyValue> aArgs( comphelper::InitPropertySequence({
                { "Col", uno::Any(sal_Int32(1000 - 1)) },
                { "Modifier", uno::Any(sal_uInt16(0)) }
        }));
    dispatchCommand(mxComponent, ".uno:SelectColumn", aArgs);

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
        std::string aVal;
        boost::property_tree::ptree::const_assoc_iterator it = aTree.find("refpoint");
        if (it != aTree.not_found())
            aVal = aTree.get_child("refpoint").get_value<std::string>();
        else
            return; // happens in testTextBoxInsert test

        uno::Sequence<OUString> aSeq = comphelper::string::convertCommaSeparated(OUString::createFromAscii(aVal.c_str()));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aSeq.getLength());
        m_aRefPoint.setX(aSeq[0].toInt32());
        m_aRefPoint.setY(aSeq[1].toInt32());

        aVal = aTree.get_child("relrect").get_value<std::string>();
        aSeq = comphelper::string::convertCommaSeparated(OUString::createFromAscii(aVal.c_str()));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(4), aSeq.getLength());
        m_aRelRect.SetLeft(aSeq[0].toInt32());
        m_aRelRect.SetTop(aSeq[1].toInt32());
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
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aSeq.getLength());
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
                CPPUNIT_ASSERT_EQUAL(sal_Int32(4), aSeq.getLength());
                tools::Rectangle aRect;
                aRect.SetLeft(aSeq[0].toInt32());
                aRect.SetTop(aSeq[1].toInt32());
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
    std::vector<int> m_aInvalidationsMode;
    bool m_bViewLock;
    OString m_sCellFormula;
    boost::property_tree::ptree m_aCommentCallbackResult;
    EditCursorMessage m_aInvalidateCursorResult;
    TextSelectionMessage m_aTextSelectionResult;
    OString m_sInvalidateHeader;
    OString m_sInvalidateSheetGeometry;
    OString m_aHyperlinkClicked;
    OString m_ShapeSelection;
    std::map<std::string, boost::property_tree::ptree> m_aStateChanges;
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
                m_aCellCursorBounds.SetLeft(aSeq[0].toInt32());
                m_aCellCursorBounds.SetTop(aSeq[1].toInt32());
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
                CPPUNIT_ASSERT(aSeq.getLength() == 4 || aSeq.getLength() == 6);
                tools::Rectangle aInvalidationRect;
                aInvalidationRect.SetLeft(aSeq[0].toInt32());
                aInvalidationRect.SetTop(aSeq[1].toInt32());
                aInvalidationRect.setWidth(aSeq[2].toInt32());
                aInvalidationRect.setHeight(aSeq[3].toInt32());
                m_aInvalidations.push_back(aInvalidationRect);
                if (aSeq.getLength() == 6)
                {
                    m_aInvalidationsParts.push_back(aSeq[4].toInt32());
                    m_aInvalidationsMode.push_back(aSeq[5].toInt32());
                }
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
        case LOK_CALLBACK_HYPERLINK_CLICKED:
        {
            m_aHyperlinkClicked = pPayload;
        }
        break;
        case LOK_CALLBACK_TEXT_SELECTION:
        {
            m_aTextSelectionResult.parseMessage(pPayload);
        }
        break;
        case LOK_CALLBACK_STATE_CHANGED:
        {
            OString aPayload(pPayload);
            if (!aPayload.startsWith("{"))
            {
                break;
            }

            std::stringstream aStream(pPayload);
            boost::property_tree::ptree aTree;
            boost::property_tree::read_json(aStream, aTree);
            std::string aCommandName = aTree.get<std::string>("commandName");
            m_aStateChanges[aCommandName] = aTree;
        }
        break;
        }
    }

    void ClearAllInvalids()
    {
        m_bInvalidateTiles = false;
        m_aInvalidations.clear();
        m_aInvalidationsParts.clear();
        m_aInvalidationsMode.clear();
    }
};

void ScTiledRenderingTest::testViewCursors()
{
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
        comphelper::makePropertyValue(".uno:SpellOnline", uno::Any(!bSet)),
    };
    pModelObj->initializeForTiledRendering(aPropertyValues);

    CPPUNIT_ASSERT_EQUAL(!bSet, pDoc->GetDocOptions().IsAutoSpell());
}

void ScTiledRenderingTest::testTextViewSelection()
{
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

void lcl_extractHandleParameters(std::string_view selection, sal_uInt32& id, sal_uInt32& x, sal_uInt32& y)
{
    OString extraInfo( selection.substr(selection.find("{")) );
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
    ScModelObj* pModelObj = createDoc("shape.ods");
    ViewCallback aView1;
    pModelObj->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONDOWN, /*x=*/ 1,/*y=*/ 1,/*count=*/ 1, /*buttons=*/ 1, /*modifier=*/0);
    pModelObj->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONUP, /*x=*/ 1, /*y=*/ 1, /*count=*/ 1, /*buttons=*/ 1, /*modifier=*/0);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT(!aView1.m_ShapeSelection.isEmpty());
    {
        sal_uInt32 id, x, y;
        lcl_extractHandleParameters(aView1.m_ShapeSelection, id, x ,y);
        sal_uInt32 oldX = x;
        sal_uInt32 oldY = y;
        uno::Sequence<beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence(
        {
            {"HandleNum", uno::Any(id)},
            {"NewPosX", uno::Any(x+1)},
            {"NewPosY", uno::Any(y+1)}
        }));
        dispatchCommand(mxComponent, ".uno:MoveShapeHandle", aPropertyValues);
        Scheduler::ProcessEventsToIdle();
        CPPUNIT_ASSERT(!aView1.m_ShapeSelection.isEmpty());
        lcl_extractHandleParameters(aView1.m_ShapeSelection, id, x ,y);
        CPPUNIT_ASSERT_EQUAL(x-1, oldX);
        CPPUNIT_ASSERT_EQUAL(y-1, oldY);
    }
}

void ScTiledRenderingTest::testColRowResize()
{
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
    dispatchCommand(mxComponent, ".uno:ColumnWidth", aArgs);

    sal_uInt16 nWidth = o3tl::convert(rDoc.GetColWidth(static_cast<SCCOL>(2), static_cast<SCTAB>(0), false), o3tl::Length::twip, o3tl::Length::mm100);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(4001), nWidth);

    // Row 5, Tab 0
    uno::Sequence<beans::PropertyValue> aArgs2( comphelper::InitPropertySequence({
            { "RowHeight", uno::Any(sal_uInt16(2000)) },
            { "Row", uno::Any(sal_Int16(5)) },
        }));
    dispatchCommand(mxComponent, ".uno:RowHeight", aArgs2);

    sal_uInt16 nHeight = o3tl::convert(rDoc.GetRowHeight(static_cast<SCROW>(4), static_cast<SCTAB>(0), false), o3tl::Length::twip, o3tl::Length::mm100);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(2000), nHeight);
}

void ScTiledRenderingTest::testUndoShells()
{
    ScModelObj* pModelObj = createDoc("small.ods");
    // Clear the currently selected cell.
    dispatchCommand(mxComponent, ".uno:ClearContents", {});

    auto pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    CPPUNIT_ASSERT(pDocShell);
    ScDocument& rDoc = pDocShell->GetDocument();
    ScUndoManager* pUndoManager = rDoc.GetUndoManager();
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
    createDoc("small.ods");

    ViewCallback aView;

    uno::Sequence<beans::PropertyValue> aArgs;
    dispatchCommand(mxComponent, ".uno:AutoSum", aArgs);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(aView.m_sCellFormula.startsWith("=SUM("));
}

void ScTiledRenderingTest::testHideColRow()
{
    createDoc("small.ods");
    {
        uno::Sequence<beans::PropertyValue> aArgs( comphelper::InitPropertySequence({
                { "Col", uno::Any(sal_Int32(2 - 1)) },
                { "Modifier", uno::Any(KEY_SHIFT) }
            }));
        dispatchCommand(mxComponent, ".uno:SelectColumn", aArgs);

        uno::Sequence<beans::PropertyValue> aArgs2( comphelper::InitPropertySequence({
                { "Col", uno::Any(sal_Int32(3 - 1)) },
                { "Modifier", uno::Any(sal_uInt16(0)) }
            }));

        dispatchCommand(mxComponent, ".uno:SelectColumn", aArgs2);
        Scheduler::ProcessEventsToIdle();
    }

    SCCOL nOldCurX = ScDocShell::GetViewData()->GetCurX();
    SCROW nOldCurY = ScDocShell::GetViewData()->GetCurY();
    {
        uno::Sequence<beans::PropertyValue> aArgs;
        dispatchCommand(mxComponent, ".uno:HideColumn", aArgs);
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
        dispatchCommand(mxComponent, ".uno:SelectRow", aArgs);

        uno::Sequence<beans::PropertyValue> aArgs2( comphelper::InitPropertySequence({
                { "Row", uno::Any(sal_Int32(7 - 1)) },
                { "Modifier", uno::Any(sal_uInt16(0)) }
            }));
        dispatchCommand(mxComponent, ".uno:SelectRow", aArgs2);
        Scheduler::ProcessEventsToIdle();
    }

    nOldCurX = ScDocShell::GetViewData()->GetCurX();
    nOldCurY = ScDocShell::GetViewData()->GetCurY();
    {
        uno::Sequence<beans::PropertyValue> aArgs;
        dispatchCommand(mxComponent, ".uno:HideRow", aArgs);
        Scheduler::ProcessEventsToIdle();
    }
    nNewCurX = ScDocShell::GetViewData()->GetCurX();
    nNewCurY = ScDocShell::GetViewData()->GetCurY();
    CPPUNIT_ASSERT(nNewCurY > nOldCurY);
    CPPUNIT_ASSERT_EQUAL(nOldCurX, nNewCurX);
}

void ScTiledRenderingTest::testInvalidateOnCopyPasteCells()
{
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
    dispatchCommand(mxComponent, ".uno:Copy", aArgs);

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
    dispatchCommand(mxComponent, ".uno:Paste", aArgs);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(aView.m_bInvalidateTiles);
}

void ScTiledRenderingTest::testInvalidateOnInserRowCol()
{
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
    dispatchCommand(mxComponent, ".uno:InsertRows", aArgs);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(aView.m_bInvalidateTiles);
    CPPUNIT_ASSERT_EQUAL(size_t(1), aView.m_aInvalidations.size());
    CPPUNIT_ASSERT_EQUAL(tools::Rectangle(-75, 51240, 32212230, 63990), aView.m_aInvalidations[0]);

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
    dispatchCommand(mxComponent, ".uno:InsertColumns", aArgs);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(aView.m_bInvalidateTiles);
    CPPUNIT_ASSERT_EQUAL(size_t(1), aView.m_aInvalidations.size());
    CPPUNIT_ASSERT_EQUAL(tools::Rectangle(254925, -15, 32212230, 63990), aView.m_aInvalidations[0]);
}

void ScTiledRenderingTest::testCommentCallback()
{
    // Comments callback are emitted only if tiled annotations are off
    comphelper::LibreOfficeKit::setTiledAnnotations(false);

    // FIXME: Hack because previous tests do not destroy ScDocument(with annotations) on exit (?).
    ScPostIt::mnLastPostItId = 1;

    {
        ScModelObj* pModelObj = createDoc("small.ods");
        ViewCallback aView1;
        int nView1 = SfxLokHelper::getView();

        // Create a 2nd view
        SfxLokHelper::createView();
        pModelObj->initializeForTiledRendering({});
        ViewCallback aView2;

        SfxLokHelper::setView(nView1);

        ScTabViewShell* pTabViewShell = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
        if (pTabViewShell)
            pTabViewShell->SetCursor(4, 4);

        // Add a new comment
        uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence(
        {
            {"Text", uno::Any(OUString("Comment"))},
            {"Author", uno::Any(OUString("LOK User1"))},
        }));
        dispatchCommand(mxComponent, ".uno:InsertAnnotation", aArgs);
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
        CPPUNIT_ASSERT_EQUAL(std::string("4 4 4 4"), aView1.m_aCommentCallbackResult.get<std::string>("cellRange"));
        CPPUNIT_ASSERT_EQUAL(std::string("4 4 4 4"), aView2.m_aCommentCallbackResult.get<std::string>("cellRange"));

        // Ensure deleting rows updates comments
        if (pTabViewShell)
            pTabViewShell->SetCursor(2, 2);

        dispatchCommand(mxComponent, ".uno:DeleteRows", {});
        Scheduler::ProcessEventsToIdle();
        CPPUNIT_ASSERT_EQUAL(std::string("4 3 4 3"), aView1.m_aCommentCallbackResult.get<std::string>("cellRange"));
        CPPUNIT_ASSERT_EQUAL(std::string("4 3 4 3"), aView2.m_aCommentCallbackResult.get<std::string>("cellRange"));

        // Ensure deleting columns updates comments
        if (pTabViewShell)
            pTabViewShell->SetCursor(2, 2);

        dispatchCommand(mxComponent, ".uno:DeleteColumns", {});
        Scheduler::ProcessEventsToIdle();
        CPPUNIT_ASSERT_EQUAL(std::string("3 3 3 3"), aView1.m_aCommentCallbackResult.get<std::string>("cellRange"));
        CPPUNIT_ASSERT_EQUAL(std::string("3 3 3 3"), aView2.m_aCommentCallbackResult.get<std::string>("cellRange"));

        std::string aCommentId = aView1.m_aCommentCallbackResult.get<std::string>("id");

        // Edit a comment
        // Select some random cell, we should be able to edit the cell note without
        // selecting the cell
        if (pTabViewShell)
            pTabViewShell->SetCursor(3, 100);
        aArgs = comphelper::InitPropertySequence(
        {
            {"Id", uno::Any(OUString::createFromAscii(aCommentId.c_str()))},
            {"Text", uno::Any(OUString("Edited comment"))},
            {"Author", uno::Any(OUString("LOK User2"))},
        });
        dispatchCommand(mxComponent, ".uno:EditAnnotation", aArgs);
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
        CPPUNIT_ASSERT_EQUAL(std::string("3 3 3 3"), aView1.m_aCommentCallbackResult.get<std::string>("cellRange"));
        CPPUNIT_ASSERT_EQUAL(std::string("3 3 3 3"), aView2.m_aCommentCallbackResult.get<std::string>("cellRange"));

        // Delete the comment
        if (pTabViewShell)
            pTabViewShell->SetCursor(4, 43);
        aArgs = comphelper::InitPropertySequence(
        {
            {"Id", uno::Any(OUString::createFromAscii(aCommentId.c_str()))}
        });
        dispatchCommand(mxComponent, ".uno:DeleteNote", aArgs);
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
    ScModelObj* pModelObj = createDoc("small.ods");
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    ScUndoManager* pUndoManager = pDoc->GetUndoManager();
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
    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();
    // check that undo has not been executed on view #2
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), pUndoManager->GetUndoActionCount());

    // try to execute undo in view #1
    SfxLokHelper::setView(nView1);
    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();
    // check that undo has been executed on view #1
    CPPUNIT_ASSERT_EQUAL(std::size_t(0), pUndoManager->GetUndoActionCount());

    // check that redo action count in not 0
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), pUndoManager->GetRedoActionCount());

    // try to execute redo in view #2
    SfxLokHelper::setView(nView2);
    dispatchCommand(mxComponent, ".uno:Redo", {});
    Scheduler::ProcessEventsToIdle();
    // check that redo has not been executed on view #2
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), pUndoManager->GetRedoActionCount());

    // try to execute redo in view #1
    SfxLokHelper::setView(nView1);
    dispatchCommand(mxComponent, ".uno:Redo", {});
    Scheduler::ProcessEventsToIdle();
    // check that redo has been executed on view #1
    CPPUNIT_ASSERT_EQUAL(std::size_t(0), pUndoManager->GetRedoActionCount());
}

void ScTiledRenderingTest::testUndoRepairDispatch()
{
    ScModelObj* pModelObj = createDoc("small.ods");
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    ScUndoManager* pUndoManager = pDoc->GetUndoManager();
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
    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();
    // check that undo has not been executed on view #2
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), pUndoManager->GetUndoActionCount());

    // try to execute undo in view #2 in repair mode
    SfxLokHelper::setView(nView2);
    uno::Sequence<beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence(
    {
        {"Repair", uno::Any(true)}
    }));
    dispatchCommand(mxComponent, ".uno:Undo", aPropertyValues);
    Scheduler::ProcessEventsToIdle();
    // check that undo has been executed on view #2 in repair mode
    CPPUNIT_ASSERT_EQUAL(std::size_t(0), pUndoManager->GetUndoActionCount());
}

void ScTiledRenderingTest::testInsertGraphicInvalidations()
{
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
    pDevice->SetOutputSizePixelScaleOffsetAndLOKBuffer(Size(nCanvasWidth, nCanvasHeight), Fraction(1.0), Point(), aBuffer.data());
    pModelObj->paintTile(*pDevice, nCanvasWidth, nCanvasHeight, /*nTilePosX=*/0, /*nTilePosY=*/0, /*nTileWidth=*/3840, /*nTileHeight=*/3840);
    Scheduler::ProcessEventsToIdle();

    // insert an image in view and see if both views are invalidated
    aView.m_bInvalidateTiles = false;
    uno::Sequence<beans::PropertyValue> aArgs( comphelper::InitPropertySequence({
            { "FileName", uno::Any(createFileURL(u"smile.png")) }
        }));
    dispatchCommand(mxComponent, ".uno:InsertGraphic", aArgs);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(aView.m_bInvalidateTiles);

    // undo image insertion in view and see if both views are invalidated
    aView.m_bInvalidateTiles = false;
    uno::Sequence<beans::PropertyValue> aArgs2;
    dispatchCommand(mxComponent, ".uno:Undo", aArgs2);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(aView.m_bInvalidateTiles);
}

void ScTiledRenderingTest::testDocumentSizeWithTwoViews()
{
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
    pDevice1->SetOutputSizePixelScaleOffsetAndLOKBuffer(Size(nCanvasWidth, nCanvasHeight), Fraction(1.0), Point(), aBuffer1.data());
    pModelObj->paintTile(*pDevice1, nCanvasWidth, nCanvasHeight, /*nTilePosX=*/0, /*nTilePosY=*/291840, /*nTileWidth=*/3840, /*nTileHeight=*/3840);
    Scheduler::ProcessEventsToIdle();

    // Create a new view
    int nView1 = SfxLokHelper::getView();
    SfxLokHelper::createView();

    std::vector<unsigned char> aBuffer2(nCanvasWidth * nCanvasHeight * 4);
    ScopedVclPtrInstance<VirtualDevice> pDevice2(DeviceFormat::DEFAULT);
    pDevice2->SetOutputSizePixelScaleOffsetAndLOKBuffer(Size(nCanvasWidth, nCanvasHeight), Fraction(1.0), Point(), aBuffer2.data());
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

    // both views have UNDO disabled
    {
        SfxItemSet aSet1(pView1->GetPool(), svl::Items<SID_UNDO, SID_UNDO>);
        SfxItemSet aSet2(pView2->GetPool(), svl::Items<SID_UNDO, SID_UNDO>);
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
    // view1 has UNDO enabled, view2 is in UNDO-repair
    {
        SfxItemSet aSet1(pView1->GetPool(), svl::Items<SID_UNDO, SID_UNDO>);
        SfxItemSet aSet2(pView2->GetPool(), svl::Items<SID_UNDO, SID_UNDO>);
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
    // both views have UNDO enabled
    Scheduler::ProcessEventsToIdle();
    {
        SfxItemSet aSet1(pView1->GetPool(), svl::Items<SID_UNDO, SID_UNDO>);
        SfxItemSet aSet2(pView2->GetPool(), svl::Items<SID_UNDO, SID_UNDO>);
        pView1->GetSlotState(SID_UNDO, nullptr, &aSet1);
        pView2->GetSlotState(SID_UNDO, nullptr, &aSet2);
        CPPUNIT_ASSERT_EQUAL(SfxItemState::SET, aSet1.GetItemState(SID_UNDO));
        CPPUNIT_ASSERT(dynamic_cast< const SfxStringItem* >(aSet1.GetItem(SID_UNDO)));
        CPPUNIT_ASSERT_EQUAL(SfxItemState::SET, aSet2.GetItemState(SID_UNDO));
        CPPUNIT_ASSERT(dynamic_cast< const SfxStringItem* >(aSet2.GetItem(SID_UNDO)));
    }

    SfxViewShell::Current()->setLibreOfficeKitViewCallback(nullptr);
    SfxLokHelper::setView(nView1);
    SfxViewShell::Current()->setLibreOfficeKitViewCallback(nullptr);
}

void ScTiledRenderingTest::testDocumentRepair()
{
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
        std::unique_ptr<SfxBoolItem> pItem1;
        std::unique_ptr<SfxBoolItem> pItem2;
        pView1->GetViewFrame()->GetBindings().QueryState(SID_DOC_REPAIR, pItem1);
        pView2->GetViewFrame()->GetBindings().QueryState(SID_DOC_REPAIR, pItem2);
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
        std::unique_ptr<SfxBoolItem> pItem1;
        std::unique_ptr<SfxBoolItem> pItem2;
        pView1->GetViewFrame()->GetBindings().QueryState(SID_DOC_REPAIR, pItem1);
        pView2->GetViewFrame()->GetBindings().QueryState(SID_DOC_REPAIR, pItem2);
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
    ScModelObj* pModelObj = createDoc("empty.ods");
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    pDoc->SetString(ScAddress(0, 0, 0), "TestCopy1");
    pDoc->SetString(ScAddress(1, 0, 0), "TestCopy2");

    // view #1
    ScTabViewShell* pView1 = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
    CPPUNIT_ASSERT(pView1);
    // emulate clipboard
    pView1->GetViewData().GetActiveWin()->SetClipboard(css::datatransfer::clipboard::LokClipboard::create(comphelper::getProcessComponentContext()));

    // view #2
    int nView1 = SfxLokHelper::getView();
    SfxLokHelper::createView();
    ScTabViewShell* pView2 = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
    // emulate clipboard
    pView2->GetViewData().GetActiveWin()->SetClipboard(css::datatransfer::clipboard::LokClipboard::create(comphelper::getProcessComponentContext()));
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

    // check locking
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
        comphelper::makePropertyValue("Enable", uno::Any(!bSet)),
    };
    dispatchCommand(mxComponent, ".uno:SpellOnline", params);
    CPPUNIT_ASSERT_EQUAL(!bSet, pDoc->GetDocOptions().IsAutoSpell());

    // set the same state as now and we don't expect any change (no-toggle)
    params =
    {
        comphelper::makePropertyValue("Enable", uno::Any(!bSet)),
    };
    dispatchCommand(mxComponent, ".uno:SpellOnline", params);
    CPPUNIT_ASSERT_EQUAL(!bSet, pDoc->GetDocOptions().IsAutoSpell());
}

void ScTiledRenderingTest::testVbaRangeCopyPaste()
{
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

void lcl_typeCharsInCell(const std::string& aStr, SCCOL nCol, SCROW nRow, ScTabViewShell* pView,
    ScModelObj* pModelObj, bool bInEdit = false, bool bCommit = true)
{
    if (!bInEdit)
        pView->SetCursor(nCol, nRow);

    for (const char& cChar : aStr)
    {
        pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, cChar, 0);
        pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, cChar, 0);
        Scheduler::ProcessEventsToIdle();
    }

    if (bCommit)
    {
        pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::RETURN);
        pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::RETURN);
        Scheduler::ProcessEventsToIdle();
    }
}

Bitmap getTile(ScModelObj* pModelObj, int nTilePosX, int nTilePosY, tools::Long nTileWidth, tools::Long nTileHeight)
{
    size_t nCanvasSize = 1024;
    size_t nTileSize = 256;
    std::vector<unsigned char> aPixmap(nCanvasSize * nCanvasSize * 4, 0);
    ScopedVclPtrInstance<VirtualDevice> pDevice(DeviceFormat::DEFAULT);
    pDevice->SetBackground(Wallpaper(COL_TRANSPARENT));
    pDevice->SetOutputSizePixelScaleOffsetAndLOKBuffer(Size(nCanvasSize, nCanvasSize),
            Fraction(1.0), Point(), aPixmap.data());
    pModelObj->paintTile(*pDevice, nCanvasSize, nCanvasSize, nTilePosX, nTilePosY, nTileWidth, nTileHeight);
    pDevice->EnableMapMode(false);
    return pDevice->GetBitmap(Point(0, 0), Size(nTileSize, nTileSize));
}

void ScTiledRenderingTest::testSheetChangeNoInvalidation()
{
    const bool oldPartInInvalidation = comphelper::LibreOfficeKit::isPartInInvalidation();
    comphelper::LibreOfficeKit::setPartInInvalidation(true);

    ScModelObj* pModelObj = createDoc("two_sheets.ods");
    ScViewData* pViewData = ScDocShell::GetViewData();
    CPPUNIT_ASSERT(pViewData);

    // Set View to initial 100%
    pModelObj->setClientVisibleArea(tools::Rectangle(0, 0, 28050, 10605));
    pModelObj->setClientZoom(256, 256, 1920, 1920);

    ScTabViewShell* pView = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
    CPPUNIT_ASSERT(pView);

    int nView1 = SfxLokHelper::getView();
    ViewCallback aView1;
    CPPUNIT_ASSERT(!lcl_hasEditView(*pViewData));

    SfxLokHelper::setView(nView1);

    aView1.ClearAllInvalids();

    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::PAGEDOWN | KEY_MOD1);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::PAGEDOWN | KEY_MOD1);
    Scheduler::ProcessEventsToIdle();
    // switching sheets should trigger no unnecessary invalidations
    CPPUNIT_ASSERT(!aView1.m_bInvalidateTiles);

    // Get the known columns/rows of this sheet 2 now we have switched to it so
    // it knows what range to broadcast invalidations for if it knows cells need
    // to be redrawn.
    tools::JsonWriter aJsonWriter1;
    pModelObj->getRowColumnHeaders(tools::Rectangle(0, 15, 19650, 5400), aJsonWriter1);
    free(aJsonWriter1.extractData());
    Scheduler::ProcessEventsToIdle();
    aView1.ClearAllInvalids();

    // switching back should also trigger no unnecessary invalidations
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::PAGEUP | KEY_MOD1);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::PAGEUP | KEY_MOD1);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(!aView1.m_bInvalidateTiles);

    // The 2nd sheet has formulas that depend on B1 in the first sheet. So if
    // we change B1 there should be an invalidation in the second sheet for the
    // range that depends on it. Because this is a single user document with no
    // active view on the 2nd sheet this will happen on switching back to sheet 2
    lcl_typeCharsInCell("101", 1, 0, pView, pModelObj); // Type '101' in B1
    aView1.ClearAllInvalids();

    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::PAGEDOWN | KEY_MOD1);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::PAGEDOWN | KEY_MOD1);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(aView1.m_bInvalidateTiles);
    aView1.ClearAllInvalids();

    // Paint it to make it valid again
    getTile(pModelObj, 0, 0, 3840, 3840);

    // switching back to sheet 1 should trigger no unnecessary invalidations
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::PAGEUP | KEY_MOD1);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::PAGEUP | KEY_MOD1);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(!aView1.m_bInvalidateTiles);

    // switching to sheet 2 should trigger no unnecessary invalidations this time
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::PAGEDOWN | KEY_MOD1);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::PAGEDOWN | KEY_MOD1);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(!aView1.m_bInvalidateTiles);

    comphelper::LibreOfficeKit::setPartInInvalidation(oldPartInInvalidation);
}

void ScTiledRenderingTest::testInsertDeletePageInvalidation()
{
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
    dispatchCommand(mxComponent, ".uno:Insert", aArgs);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(aView1.m_bInvalidateTiles);
    CPPUNIT_ASSERT_EQUAL(size_t(2), aView1.m_aInvalidations.size());
    CPPUNIT_ASSERT_EQUAL(tools::Rectangle(0, 0, 1000000000, 1000000000), aView1.m_aInvalidations[0]);
    CPPUNIT_ASSERT_EQUAL(2, pModelObj->getParts());

    // Delete sheet
    aView1.m_bInvalidateTiles = false;
    aView1.m_aInvalidations.clear();
    uno::Sequence<beans::PropertyValue> aArgs2( comphelper::InitPropertySequence({
            { "Index", uno::Any(sal_Int32(1)) }
        }));
    dispatchCommand(mxComponent, ".uno:Remove", aArgs2);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(aView1.m_bInvalidateTiles);
    CPPUNIT_ASSERT_EQUAL(size_t(1), aView1.m_aInvalidations.size());
    CPPUNIT_ASSERT_EQUAL(tools::Rectangle(0, 0, 1000000000, 1000000000), aView1.m_aInvalidations[0]);
    CPPUNIT_ASSERT_EQUAL(1, pModelObj->getParts());
}

void ScTiledRenderingTest::testGetRowColumnHeadersInvalidation()
{
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
    CPPUNIT_ASSERT_EQUAL(tools::Rectangle(Point(26775, 0), Size(22950, 13005)), aView1.m_aInvalidations[0]);

    // Extend area top-to-bottom
    aView1.m_bInvalidateTiles = false;
    aView1.m_aInvalidations.clear();
    tools::JsonWriter aJsonWriter2;
    pModelObj->getRowColumnHeaders(tools::Rectangle(0, 5400, 19650, 9800), aJsonWriter2);
    free(aJsonWriter2.extractData());
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(aView1.m_bInvalidateTiles);
    CPPUNIT_ASSERT_EQUAL(size_t(1), aView1.m_aInvalidations.size());
    CPPUNIT_ASSERT_EQUAL(tools::Rectangle(Point(0, 13005), Size(49725, 6375)), aView1.m_aInvalidations[0]);

    // Extend area left-to-right
    aView1.m_bInvalidateTiles = false;
    aView1.m_aInvalidations.clear();
    tools::JsonWriter aJsonWriter3;
    pModelObj->getRowColumnHeaders(tools::Rectangle(5400, 5400, 25050, 9800), aJsonWriter3);
    free(aJsonWriter3.extractData());
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(aView1.m_bInvalidateTiles);
    CPPUNIT_ASSERT_EQUAL(size_t(1), aView1.m_aInvalidations.size());
    CPPUNIT_ASSERT_EQUAL(tools::Rectangle(Point(49725, 0), Size(25500, 19380)), aView1.m_aInvalidations[0]);
}

void ScTiledRenderingTest::testJumpHorizontallyInvalidation()
{
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
    // 261375 because we limit how far we jump into empty space in online, 267386880 if we don't limit
    CPPUNIT_ASSERT_EQUAL(tools::Rectangle(0, 13005, 26775, 261375), aView1.m_aInvalidations[0]);
}

// We need to ensure that views are not perterbed by rendering (!?) hmm ...
void ScTiledRenderingTest::testRowColumnHeaders()
{
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
    ScModelObj* pModelObj = createDoc("empty.ods");
    ScDocument* pDoc = pModelObj->GetDocument();
    const SheetGeometryData aSGData(
        // cols
        {
            // width spans
            {
                { STD_COL_WIDTH,   20     },
                { 2*STD_COL_WIDTH, 26     },
                { STD_COL_WIDTH,   pDoc->MaxCol() }
            },

            // hidden spans
            {
                { 0, 5      },
                { 1, 12     },
                { 0, pDoc->MaxCol() }
            },

            // filtered spans
            {
                { 0, 50     },
                { 1, 59     },
                { 0, pDoc->MaxCol() }
            }
        },

        // rows
        {
            // height spans
            {
                { 300,  50     },
                { 600,  65     },
                { 300,  pDoc->MaxRow() }
            },

            // hidden spans
            {
                { 1, 100    },
                { 0, 500    },
                { 1, 578    },
                { 0, pDoc->MaxRow() }
            },

            // filtered spans
            {
                { 0, 150    },
                { 1, 159    },
                { 0, pDoc->MaxRow() }
            }
        }
    );

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
    ScModelObj* pModelObj = createDoc("empty.ods");
    ScDocument* pDoc = pModelObj->GetDocument();
    const SheetGeometryData aDefaultSGData(
        // cols
        {
            // width spans
            { { STD_COL_WIDTH, pDoc->MaxCol() } },
            // hidden spans
            { { 0,             pDoc->MaxCol() } },
            // filtered spans
            { { 0,             pDoc->MaxCol() } }
        },
        // rows
        {
            // height spans
            { { ScGlobal::nStdRowHeight, pDoc->MaxRow() } },
            // hidden spans
            { { 0,                       pDoc->MaxRow() } },
            // filtered spans
            { { 0,                       pDoc->MaxRow() } }
        }
    );

    const SheetGeometryData aSGData(
        // cols
        {
            // width spans
            {
                { STD_COL_WIDTH,   20     },
                { 2*STD_COL_WIDTH, 26     },
                { STD_COL_WIDTH,   pDoc->MaxCol() }
            },

            // hidden spans
            {
                { 0, 5      },
                { 1, 12     },
                { 0, pDoc->MaxCol() }
            },

            // filtered spans
            {
                { 0, 50     },
                { 1, 59     },
                { 0, pDoc->MaxCol() }
            }
        },

        // rows
        {
            // height spans
            {
                { 300,  50     },
                { 600,  65     },
                { 300,  pDoc->MaxRow() }
            },

            // hidden spans
            {
                { 1, 100    },
                { 0, 500    },
                { 1, 578    },
                { 0, pDoc->MaxRow() }
            },

            // filtered spans
            {
                { 0, 150    },
                { 1, 159    },
                { 0, pDoc->MaxRow() }
            }
        }
    );

    ScViewData* pViewData = ScDocShell::GetViewData();
    CPPUNIT_ASSERT(pViewData);

    // view #1
    ViewCallback aView1;

    // with the default empty sheet and test the JSON encoding.
    OString aGeomDefaultStr = pModelObj->getSheetGeometryData(/*bColumns*/ true, /*bRows*/ true, /*bSizes*/ true,
            /*bHidden*/ true, /*bFiltered*/ true, /*bGroups*/ true);
    aDefaultSGData.parseTest(aGeomDefaultStr);

    // Apply geometry settings to the sheet and then test the resulting JSON encoding.
    aSGData.setDataToDoc(pDoc);
    OString aGeomStr = pModelObj->getSheetGeometryData(/*bColumns*/ true, /*bRows*/ true, /*bSizes*/ true,
            /*bHidden*/ true, /*bFiltered*/ true, /*bGroups*/ true);
    aSGData.parseTest(aGeomStr);

    SfxViewShell::Current()->setLibreOfficeKitViewCallback(nullptr);
}

void ScTiledRenderingTest::testDeleteCellMultilineContent()
{
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
    comphelper::LibreOfficeKit::setCompatFlag(
        comphelper::LibreOfficeKit::Compat::scPrintTwipsMsgs);

    ScModelObj* pModelObj = createDoc("empty.ods");
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();

    // Set Wrap text in A3
    pDoc->ApplyAttr(0, 2, 0, ScLineBreakCell(true));
    const ScLineBreakCell* pItem = pDoc->GetAttr(0, 2, 0, ATTR_LINEBREAK);
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
    dispatchCommand(mxComponent, ".uno:SortAscending", aArgs);

    // check it's sorted
    for (SCROW r = 0; r < 6; ++r)
    {
        CPPUNIT_ASSERT_EQUAL(double(r + 1), pDoc->GetValue(ScAddress(0, r, 0)));
    }

    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(OString("rows"), aView.m_sInvalidateSheetGeometry);

    aView.m_sInvalidateSheetGeometry = "";
    // sort descending
    dispatchCommand(mxComponent, ".uno:SortDescending", aArgs);

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

void ScTiledRenderingTest::testAutoInputStringBlock()
{
    ScModelObj* pModelObj = createDoc("empty.ods");
    CPPUNIT_ASSERT(pModelObj);
    ScTabViewShell* pView = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
    CPPUNIT_ASSERT(pView);
    ScDocument* pDoc = pModelObj->GetDocument();

    pDoc->SetString(ScAddress(0, 3, 0), "ABC");  // A4
    pDoc->SetString(ScAddress(0, 4, 0), "BAC");  // A5
    ScFieldEditEngine& rEE = pDoc->GetEditEngine();
    rEE.SetText("XYZ");
    pDoc->SetEditText(ScAddress(0, 5, 0), rEE.CreateTextObject()); // A6
    pDoc->SetValue(ScAddress(0, 6, 0), 123);
    pDoc->SetString(ScAddress(0, 7, 0), "ZZZ");  // A8

    ScAddress aA1(0, 0, 0);
    lcl_typeCharsInCell("X", aA1.Col(), aA1.Row(), pView, pModelObj); // Type 'X' in A1
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A1 should autocomplete", OUString("XYZ"), pDoc->GetString(aA1));

    ScAddress aA3(0, 2, 0); // Adjacent to the string "superblock" A4:A8
    lcl_typeCharsInCell("X", aA3.Col(), aA3.Row(), pView, pModelObj); // Type 'X' in A3
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A3 should autocomplete", OUString("XYZ"), pDoc->GetString(aA3));

    ScAddress aA9(0, 8, 0); // Adjacent to the string "superblock" A4:A8
    lcl_typeCharsInCell("X", aA9.Col(), aA9.Row(), pView, pModelObj); // Type 'X' in A9
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A9 should autocomplete", OUString("XYZ"), pDoc->GetString(aA9));

    ScAddress aA11(0, 10, 0);
    lcl_typeCharsInCell("X", aA11.Col(), aA11.Row(), pView, pModelObj); // Type 'X' in A11
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A11 should autocomplete", OUString("XYZ"), pDoc->GetString(aA11));
}

void ScTiledRenderingTest::testAutoInputExactMatch()
{
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
    // Should show the partial completion "i".
    CPPUNIT_ASSERT_EQUAL_MESSAGE("1: A8 should have partial completion Si", OUString("Si"), pDoc->GetString(aA8));

    lcl_typeCharsInCell("Si", aA8.Col(), aA8.Row(), pView, pModelObj); // Type "Si" in A8
    // Should not show any suggestions.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("2: A8 should not show suggestions", OUString("Si"), pDoc->GetString(aA8));

    lcl_typeCharsInCell("Sim", aA8.Col(), aA8.Row(), pView, pModelObj); // Type "Sim" in A8
    // Should autocomplete to "Simple" which is the only match.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("3: A8 should autocomplete", OUString("Simple"), pDoc->GetString(aA8));

    lcl_typeCharsInCell("Sin", aA8.Col(), aA8.Row(), pView, pModelObj); // Type "Sin" in A8
    // Should autocomplete to "Sing" which is the only match.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("4: A8 should autocomplete", OUString("Sing"), pDoc->GetString(aA8));

    lcl_typeCharsInCell("C", aA8.Col(), aA8.Row(), pView, pModelObj); // Type "C" in A8
    // Should show the partial completion "as".
    CPPUNIT_ASSERT_EQUAL_MESSAGE("5: A8 should have partial completion Cas", OUString("Cas"), pDoc->GetString(aA8));

    lcl_typeCharsInCell("Cast", aA8.Col(), aA8.Row(), pView, pModelObj); // Type "Cast" in A8
    // Should autocomplete to "Castle" which is the only match.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("6: A8 should autocomplete", OUString("Castle"), pDoc->GetString(aA8));

    lcl_typeCharsInCell("T", aA8.Col(), aA8.Row(), pView, pModelObj); // Type "T" in A8
    // Should autocomplete to "Time" which is the only match.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("7: A8 should autocomplete", OUString("Time"), pDoc->GetString(aA8));
}

void ScTiledRenderingTest::testEditCursorBounds()
{
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
        aCellBounds.Contains(aView.m_aInvalidateCursorResult.getBounds()));

    SfxViewShell::Current()->setLibreOfficeKitViewCallback(nullptr);
}

void ScTiledRenderingTest::testTextSelectionBounds()
{
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
    dispatchCommand(mxComponent, ".uno:Insert", aArgs);
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

void ScTiledRenderingTest::testTextBoxInsert()
{
    createDoc("empty.ods");
    ViewCallback aView1;

    // insert textbox
    uno::Sequence<beans::PropertyValue> aArgs(
        comphelper::InitPropertySequence({
            { "CreateDirectly",  uno::Any(true) }
        }));
    dispatchCommand(mxComponent, ".uno:DrawText", aArgs);
    Scheduler::ProcessEventsToIdle();

    // check if we have textbox selected
    CPPUNIT_ASSERT(!aView1.m_ShapeSelection.isEmpty());
    CPPUNIT_ASSERT(aView1.m_ShapeSelection != "EMPTY");

    Scheduler::ProcessEventsToIdle();
}

void ScTiledRenderingTest::testCommentCellCopyPaste()
{
    // Comments callback are emitted only if tiled annotations are off
    comphelper::LibreOfficeKit::setTiledAnnotations(false);

    // FIXME: Hack because previous tests do not destroy ScDocument(with annotations) on exit (?).
    ScPostIt::mnLastPostItId = 1;

    {
        ScModelObj* pModelObj = createDoc("empty.ods");
        ViewCallback aView;
        int nView = SfxLokHelper::getView();

        SfxLokHelper::setView(nView);

        ScTabViewShell* pTabViewShell = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
        CPPUNIT_ASSERT(pTabViewShell);

        lcl_typeCharsInCell("ABC", 0, 0, pTabViewShell, pModelObj); // Type "ABC" in A1

        pTabViewShell->SetCursor(1, 1);

        // Add a new comment
        uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence(
        {
            {"Text", uno::Any(OUString("LOK Comment Cell B2"))},
            {"Author", uno::Any(OUString("LOK Client"))},
        }));
        dispatchCommand(mxComponent, ".uno:InsertAnnotation", aArgs);
        Scheduler::ProcessEventsToIdle();

        // We received a LOK_CALLBACK_COMMENT callback with comment 'Add' action
        CPPUNIT_ASSERT_EQUAL(std::string("Add"), aView.m_aCommentCallbackResult.get<std::string>("action"));
        CPPUNIT_ASSERT_EQUAL(std::string("1"), aView.m_aCommentCallbackResult.get<std::string>("id"));
        CPPUNIT_ASSERT_EQUAL(std::string("0"), aView.m_aCommentCallbackResult.get<std::string>("tab"));
        CPPUNIT_ASSERT_EQUAL(std::string("LOK Client"), aView.m_aCommentCallbackResult.get<std::string>("author"));
        CPPUNIT_ASSERT_EQUAL(std::string("LOK Comment Cell B2"), aView.m_aCommentCallbackResult.get<std::string>("text"));

        uno::Sequence<beans::PropertyValue> aCopyPasteArgs;

        // We need separate tests for single cell copy-paste and cell-range copy-paste
        // since they hit different code paths in ScColumn methods.

        // Single cell(with comment) copy paste test
        {
            dispatchCommand(mxComponent, ".uno:Copy", aCopyPasteArgs);
            Scheduler::ProcessEventsToIdle();

            pTabViewShell->SetCursor(1, 49);
            Scheduler::ProcessEventsToIdle();
            dispatchCommand(mxComponent, ".uno:Paste", aCopyPasteArgs); // Paste to cell B50
            Scheduler::ProcessEventsToIdle();

            // We received a LOK_CALLBACK_COMMENT callback with comment 'Add' action
            CPPUNIT_ASSERT_EQUAL(std::string("Add"), aView.m_aCommentCallbackResult.get<std::string>("action"));
            // Without the fix the id will be "1".
            CPPUNIT_ASSERT_EQUAL(std::string("2"), aView.m_aCommentCallbackResult.get<std::string>("id"));
            CPPUNIT_ASSERT_EQUAL(std::string("0"), aView.m_aCommentCallbackResult.get<std::string>("tab"));
            CPPUNIT_ASSERT_EQUAL(std::string("LOK Client"), aView.m_aCommentCallbackResult.get<std::string>("author"));
            CPPUNIT_ASSERT_EQUAL(std::string("LOK Comment Cell B2"), aView.m_aCommentCallbackResult.get<std::string>("text"));
        }

        // Cell range (with a comment) copy paste test
        {
            // Select range A1:C3
            pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_HOME | KEY_MOD1);
            pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_HOME | KEY_MOD1);
            pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_DOWN | KEY_SHIFT);
            pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_DOWN | KEY_SHIFT);
            pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_DOWN | KEY_SHIFT);
            pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_DOWN | KEY_SHIFT);
            pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_RIGHT | KEY_SHIFT);
            pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_RIGHT | KEY_SHIFT);
            pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_RIGHT | KEY_SHIFT);
            pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_RIGHT | KEY_SHIFT);
            Scheduler::ProcessEventsToIdle();

            dispatchCommand(mxComponent, ".uno:Copy", aCopyPasteArgs);
            Scheduler::ProcessEventsToIdle();

            pTabViewShell->SetCursor(3, 49);
            Scheduler::ProcessEventsToIdle();
            dispatchCommand(mxComponent, ".uno:Paste", aCopyPasteArgs); // Paste to cell D50
            Scheduler::ProcessEventsToIdle();

            // We received a LOK_CALLBACK_COMMENT callback with comment 'Add' action
            CPPUNIT_ASSERT_EQUAL(std::string("Add"), aView.m_aCommentCallbackResult.get<std::string>("action"));
            // Without the fix the id will be "1".
            CPPUNIT_ASSERT_EQUAL(std::string("3"), aView.m_aCommentCallbackResult.get<std::string>("id"));
            CPPUNIT_ASSERT_EQUAL(std::string("0"), aView.m_aCommentCallbackResult.get<std::string>("tab"));
            CPPUNIT_ASSERT_EQUAL(std::string("LOK Client"), aView.m_aCommentCallbackResult.get<std::string>("author"));
            CPPUNIT_ASSERT_EQUAL(std::string("LOK Comment Cell B2"), aView.m_aCommentCallbackResult.get<std::string>("text"));
        }
    }
    comphelper::LibreOfficeKit::setTiledAnnotations(true);
}

void ScTiledRenderingTest::testInvalidEntrySave()
{
    loadFromURL(u"validity.xlsx");

    // .uno:Save modifies the original file, make a copy first
    saveAndReload("Calc Office Open XML");
    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    const ScDocument* pDoc = pModelObj->GetDocument();
    ViewCallback aView;
    int nView = SfxLokHelper::getView();

    SfxLokHelper::setView(nView);

    ScDocShell* pDocSh = dynamic_cast< ScDocShell* >( pModelObj->GetEmbeddedObject() );
    ScTabViewShell* pTabViewShell = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
    CPPUNIT_ASSERT(pTabViewShell);

    // Type partial date "7/8" of "7/8/2013" that
    // the validation cell at A8 can accept
    lcl_typeCharsInCell("7/8", 0, 7, pTabViewShell, pModelObj,
        false /* bInEdit */, false /* bCommit */); // Type "7/8" in A8

    uno::Sequence<beans::PropertyValue> aArgs;
    dispatchCommand(mxComponent, ".uno:Save", aArgs);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_MESSAGE("Should not be marked modified after save", !pDocSh->IsModified());

    // Complete the date in A8 by appending "/2013" and commit.
    lcl_typeCharsInCell("/2013", 0, 7, pTabViewShell, pModelObj,
        true /* bInEdit */, true /* bCommit */);

    // This would hang if the date entered "7/8/2013" is not acceptable.
    Scheduler::ProcessEventsToIdle();

    // Ensure that the correct date is recorded in the document.
    CPPUNIT_ASSERT_EQUAL(double(41463), pDoc->GetValue(ScAddress(0, 7, 0)));
}

void ScTiledRenderingTest::testUndoReordering()
{
    ScModelObj* pModelObj = createDoc("small.ods");
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    ScUndoManager* pUndoManager = pDoc->GetUndoManager();
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

    // check that undo action count is not 0
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), pUndoManager->GetUndoActionCount());

    // text edit a different cell in view #2
    SfxLokHelper::setView(nView2);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_DOWN);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_DOWN);
    Scheduler::ProcessEventsToIdle();
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_DOWN);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_DOWN);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'x', 0);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 'x', 0);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::RETURN);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::RETURN);
    Scheduler::ProcessEventsToIdle();

    // check that undo action count is not 1
    CPPUNIT_ASSERT_EQUAL(std::size_t(2), pUndoManager->GetUndoActionCount());

    // try to execute undo in view #1
    SfxLokHelper::setView(nView1);
    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();
    // check that undo has been executed on view #1
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), pUndoManager->GetUndoActionCount());

    // try to execute undo in view #2
    SfxLokHelper::setView(nView2);
    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();
    // check that undo has been executed on view #2
    CPPUNIT_ASSERT_EQUAL(std::size_t(0), pUndoManager->GetUndoActionCount());
}

void ScTiledRenderingTest::testUndoReorderingRedo()
{
    ScModelObj* pModelObj = createDoc("empty.ods");
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    ScUndoManager* pUndoManager = pDoc->GetUndoManager();
    CPPUNIT_ASSERT(pUndoManager);
    CPPUNIT_ASSERT_EQUAL(std::size_t(0), pUndoManager->GetUndoActionCount());

    // view #1
    int nView1 = SfxLokHelper::getView();
    SfxViewShell* pView1 = SfxViewShell::Current();
    ViewCallback aView1;

    // view #2
    SfxLokHelper::createView();
    int nView2 = SfxLokHelper::getView();
    SfxViewShell* pView2 = SfxViewShell::Current();
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ViewCallback aView2;

    // text edit a cell in view #1
    SfxLokHelper::setView(nView1);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'x', 0);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 'x', 0);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'x', 0);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 'x', 0);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::RETURN);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::RETURN);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), pUndoManager->GetUndoActionCount());

    // text edit another cell in view #1
    SfxLokHelper::setView(nView1);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'y', 0);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 'y', 0);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'y', 0);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 'y', 0);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::RETURN);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::RETURN);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(std::size_t(2), pUndoManager->GetUndoActionCount());
    CPPUNIT_ASSERT_EQUAL(OUString("xx"), pDoc->GetString(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("yy"), pDoc->GetString(ScAddress(0, 1, 0)));

    // text edit a different cell in view #2
    SfxLokHelper::setView(nView2);
    ScTabViewShell* pViewShell2 = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
    pViewShell2->SetCursor(0, 2);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'C', 0);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 'C', 0);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'C', 0);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 'C', 0);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::RETURN);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::RETURN);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(std::size_t(3), pUndoManager->GetUndoActionCount());
    CPPUNIT_ASSERT_EQUAL(OUString("xx"), pDoc->GetString(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("yy"), pDoc->GetString(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("CC"), pDoc->GetString(ScAddress(0, 2, 0)));

    // View 1 presses undo, and the second cell is erased
    SfxLokHelper::setView(nView1);
    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(std::size_t(2), pUndoManager->GetUndoActionCount());
    CPPUNIT_ASSERT_EQUAL(OUString("xx"), pDoc->GetString(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString(""), pDoc->GetString(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("CC"), pDoc->GetString(ScAddress(0, 2, 0)));

    // Verify that the UNDO buttons/actions are still enabled
    {
        SfxItemSet aSet1(pView1->GetPool(), svl::Items<SID_UNDO, SID_UNDO>);
        SfxItemSet aSet2(pView2->GetPool(), svl::Items<SID_UNDO, SID_UNDO>);
        pView1->GetSlotState(SID_UNDO, nullptr, &aSet1);
        pView2->GetSlotState(SID_UNDO, nullptr, &aSet2);
        CPPUNIT_ASSERT_EQUAL(SfxItemState::SET, aSet1.GetItemState(SID_UNDO));
        CPPUNIT_ASSERT(dynamic_cast< const SfxStringItem* >(aSet1.GetItem(SID_UNDO)));
        CPPUNIT_ASSERT_EQUAL(SfxItemState::SET, aSet2.GetItemState(SID_UNDO));
        CPPUNIT_ASSERT(dynamic_cast< const SfxStringItem* >(aSet2.GetItem(SID_UNDO)));
    }

    // View 1 presses undo again, and the first cell is erased
    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), pUndoManager->GetUndoActionCount());
    CPPUNIT_ASSERT_EQUAL(OUString(""), pDoc->GetString(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString(""), pDoc->GetString(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("CC"), pDoc->GetString(ScAddress(0, 2, 0)));
}

void ScTiledRenderingTest::testUndoReorderingMulti()
{
    ScModelObj* pModelObj = createDoc("empty.ods");
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    ScUndoManager* pUndoManager = pDoc->GetUndoManager();
    CPPUNIT_ASSERT(pUndoManager);
    CPPUNIT_ASSERT_EQUAL(std::size_t(0), pUndoManager->GetUndoActionCount());

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
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'x', 0);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 'x', 0);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::RETURN);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::RETURN);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), pUndoManager->GetUndoActionCount());

    // text edit a different cell in view #2
    SfxLokHelper::setView(nView2);
    ScTabViewShell* pView2 = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
    pView2->SetCursor(0, 2);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'C', 0);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 'C', 0);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'C', 0);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 'C', 0);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::RETURN);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::RETURN);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(std::size_t(2), pUndoManager->GetUndoActionCount());
    CPPUNIT_ASSERT_EQUAL(OUString("xx"), pDoc->GetString(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("CC"), pDoc->GetString(ScAddress(0, 2, 0)));

    // and another cell in view #2
    pView2->SetCursor(0, 3);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'D', 0);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 'D', 0);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'D', 0);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 'D', 0);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::RETURN);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::RETURN);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(std::size_t(3), pUndoManager->GetUndoActionCount());
    CPPUNIT_ASSERT_EQUAL(OUString("xx"), pDoc->GetString(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("CC"), pDoc->GetString(ScAddress(0, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("DD"), pDoc->GetString(ScAddress(0, 3, 0)));

    // View 1 presses undo
    SfxLokHelper::setView(nView1);
    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(std::size_t(2), pUndoManager->GetUndoActionCount());
    CPPUNIT_ASSERT_EQUAL(OUString(""), pDoc->GetString(ScAddress(0, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("CC"), pDoc->GetString(ScAddress(0, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("DD"), pDoc->GetString(ScAddress(0, 3, 0)));
}

void ScTiledRenderingTest::testGetViewRenderState()
{
    // Add a pair of schemes, last added is the default
    svtools::EditableColorConfig aColorConfig;
    aColorConfig.AddScheme(u"Dark");
    aColorConfig.AddScheme(u"Light");

    ScModelObj* pModelObj = createDoc("empty.ods");
    int nFirstViewId = SfxLokHelper::getView();
    ViewCallback aView1;

    CPPUNIT_ASSERT_EQUAL(OString(";Default"), pModelObj->getViewRenderState());
    // Create a second view
    SfxLokHelper::createView();
    ViewCallback aView2;
    CPPUNIT_ASSERT_EQUAL(OString(";Default"), pModelObj->getViewRenderState());
    // Set second view to dark scheme
    {
        uno::Sequence<beans::PropertyValue> aPropertyValues = comphelper::InitPropertySequence(
            {
                { "NewTheme", uno::Any(OUString("Dark")) },
            }
        );
        dispatchCommand(mxComponent, ".uno:ChangeTheme", aPropertyValues);
    }
    CPPUNIT_ASSERT_EQUAL(OString(";Dark"), pModelObj->getViewRenderState());

    // Switch back to first view and make sure it's the same
    SfxLokHelper::setView(nFirstViewId);
    CPPUNIT_ASSERT_EQUAL(OString(";Default"), pModelObj->getViewRenderState());
}

/*
 * testInvalidateOnTextEditWithDifferentZoomLevels
 * steps:
 * set view 1 zoom to the passed zoom level
 * in view 1 type a char at the passed cell address
 * store invalidation rectangle
 * exit from in place editing (press esc)
 * create view 2 (keep 100% zoom)
 * go to the same cell address used in view 1
 * type a char into the cell
 * get invalidation rectangle for view 1
 * check if the invalidation rectangle is equal to the one stored previously
*/
void ScTiledRenderingTest::testInvalidateOnTextEditWithDifferentZoomLevels(const ColRowZoom& rData)
{
    ScModelObj* pModelObj = createDoc("empty.ods");
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    OUString sZoomUnoCmd = ".uno:ZoomPlus";
    int nZoomLevel = rData.zoom;
    if (nZoomLevel < 0)
    {
        nZoomLevel = -nZoomLevel;
        sZoomUnoCmd = ".uno:ZoomMinus";
    }

    // view #1
    ViewCallback aView1;
    // set zoom level
    for (int i = 0; i < nZoomLevel; ++i)
        dispatchCommand(mxComponent, sZoomUnoCmd, {});
    Scheduler::ProcessEventsToIdle();

    auto* pTabViewShell1 = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
    CPPUNIT_ASSERT(pTabViewShell1);

    // enable in place editing in view 1
    auto& rInvalidations = aView1.m_aInvalidations;
    pTabViewShell1->SetCursor(rData.col, rData.row);
    Scheduler::ProcessEventsToIdle();
    aView1.m_bInvalidateTiles = false;
    rInvalidations.clear();
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'x', 0);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 'x', 0);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(aView1.m_bInvalidateTiles);
    CPPUNIT_ASSERT(!rInvalidations.empty());
    tools::Rectangle aInvRect1 = rInvalidations[0];

    // end editing
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::ESCAPE);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::ESCAPE);
    Scheduler::ProcessEventsToIdle();

    // view #2
    SfxLokHelper::createView();
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    ViewCallback aView2;
    Scheduler::ProcessEventsToIdle();

    auto* pTabViewShell2 = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
    CPPUNIT_ASSERT(pTabViewShell2);
    pTabViewShell2->SetCursor(rData.col, rData.row);
    Scheduler::ProcessEventsToIdle();

    // text edit in view #2
    aView1.m_bInvalidateTiles = false;
    rInvalidations.clear();
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 'x', 0);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 'x', 0);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(aView1.m_bInvalidateTiles);
    CPPUNIT_ASSERT(!rInvalidations.empty());
    tools::Rectangle aInvRect2 = rInvalidations[0];

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Invalidation rectangle is wrong.", aInvRect1, aInvRect2);
}

void ScTiledRenderingTest::testOpenURL()
{
    // Given a document that has 2 views:
    createDoc("empty.ods");
    int nView1 = SfxLokHelper::getView();
    ViewCallback aView1;
    SfxLokHelper::createView();
    ViewCallback aView2;

    // When clicking on a link in view 2, but switching to view 1 before processing async events:
    ScGlobal::OpenURL(/*aUrl=*/u"http://www.example.com/", /*aTarget=*/u"",
                      /*bIgnoreSettings=*/true);
    SfxLokHelper::setView(nView1);
    Scheduler::ProcessEventsToIdle();

    // Then make sure view 2 gets the callback, not view 1:
    // Without the accompanying fix in place, this test would have failed, view 1 got the hyperlink
    // callback.
    CPPUNIT_ASSERT(aView1.m_aHyperlinkClicked.isEmpty());
    CPPUNIT_ASSERT(!aView2.m_aHyperlinkClicked.isEmpty());
}

void ScTiledRenderingTest::testInvalidateForSplitPanes()
{
    comphelper::LibreOfficeKit::setCompatFlag(
        comphelper::LibreOfficeKit::Compat::scPrintTwipsMsgs);

    ScModelObj* pModelObj = createDoc("split.ods");
    CPPUNIT_ASSERT(pModelObj);
    ScTabViewShell* pView = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
    CPPUNIT_ASSERT(pView);

    // view
    ViewCallback aView;

    // move way over to the right where BP:20 exists, enough so that rows A and B
    // would scroll off the page and not be visible, if they were not frozen
    pModelObj->setClientVisibleArea(tools::Rectangle(73050, 0, 94019, 7034));
    Scheduler::ProcessEventsToIdle();

    ScAddress aBP20(67, 19, 0); // BP:20

    pView->SetCursor(aBP20.Col(), aBP20.Row());
    Scheduler::ProcessEventsToIdle();

    aView.m_bInvalidateTiles = false;
    aView.m_aInvalidations.clear();

    lcl_typeCharsInCell("X", aBP20.Col(), aBP20.Row(), pView, pModelObj); // Type 'X' in A1

    CPPUNIT_ASSERT(aView.m_bInvalidateTiles);

    // missing before fix
    tools::Rectangle aTopLeftPane(0, 500, 3817, 742);
    bool bFoundTopLeftPane =
        std::find(aView.m_aInvalidations.begin(), aView.m_aInvalidations.end(), aTopLeftPane) != aView.m_aInvalidations.end();
    CPPUNIT_ASSERT_MESSAGE("The cell visible in the top left pane should be redrawn", bFoundTopLeftPane);

    // missing before fix
    tools::Rectangle aBottomLeftPane(0, 500, 3817, 3242);
    bool bFoundBottomLeftPane =
        std::find(aView.m_aInvalidations.begin(), aView.m_aInvalidations.end(), aBottomLeftPane) != aView.m_aInvalidations.end();
    CPPUNIT_ASSERT_MESSAGE("The cell visible in the bottom left pane should be redrawn", bFoundBottomLeftPane);
}

void ScTiledRenderingTest::testStatusBarLocale()
{
    // Given 2 views, the second's locale is set to German:
    createDoc("empty.ods");
    int nView1 = SfxLokHelper::getView();
    ViewCallback aView1;
    SfxLokHelper::createView();
    ViewCallback aView2;
    SfxViewShell* pView2 = SfxViewShell::Current();
    pView2->SetLOKLocale("de-DE");
    {
        SfxViewFrame* pFrame = pView2->GetViewFrame();
        SfxSlotPool& rSlotPool = SfxSlotPool::GetSlotPool(pFrame);
        uno::Reference<util::XURLTransformer> xParser(util::URLTransformer::create(m_xContext));
        util::URL aCommandURL;
        aCommandURL.Complete = ".uno:RowColSelCount";
        xParser->parseStrict(aCommandURL);
        const SfxSlot* pSlot = rSlotPool.GetUnoSlot(aCommandURL.Path);
        pFrame->GetBindings().GetDispatch(pSlot, aCommandURL, false);
    }
    aView2.m_aStateChanges.clear();

    // When creating a cell selection in the 2nd view and processing jobs with the 1st view set to
    // active:
    comphelper::dispatchCommand(".uno:GoDownSel", {});
    SfxLokHelper::setView(nView1);
    pView2->GetViewFrame()->GetBindings().GetTimer().Invoke();
    // Once more to hit the pImpl->bMsgDirty = false case in SfxBindings::NextJob_Impl().
    pView2->GetViewFrame()->GetBindings().GetTimer().Invoke();

    // Then make sure that the locale is taken into account while producing the state changed
    // callback:
    auto it = aView2.m_aStateChanges.find(".uno:RowColSelCount");
    CPPUNIT_ASSERT(it != aView2.m_aStateChanges.end());
    std::string aLocale = it->second.get<std::string>("locale");
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: de-DE
    // - Actual  : en-US
    // i.e. the 2nd view got its callback with the locale of the first view, which is buggy.
    CPPUNIT_ASSERT_EQUAL(std::string("de-DE"), aLocale);
}

void ScTiledRenderingTest::testLongFirstColumnMouseClick()
{
    // Document has a long first column. We want to mouse-click on the column and
    // check the selection changed to this column.

    // The issue we want to reproduce is that the click on a cell in the first column that is
    // very long (longer than ~800px default size of GridWindow) triggers a code-path where the cell
    // selected is the neighbouring cell even when we clicked on the area of the first cell.

    ScModelObj* pModelObj = createDoc("DocumentWithLongFirstColumn.ods");
    CPPUNIT_ASSERT(pModelObj);
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());

    // Fetch current view data
    ScViewData* pViewData = ScDocShell::GetViewData();
    CPPUNIT_ASSERT(pViewData);
    double nPPTX = pViewData->GetPPTX();
    double nPPTY = pViewData->GetPPTX();

    // Set click position

    // Left side of the first cell
    int leftCellSideX = 1 / nPPTX; // convert pixels to logical units

    // Right side of the first cell. First cell is long so click somewhere more than 800px (default of GridWindow size).
    int rightCellSideX = 1000 / nPPTX; // convert pixels to logical units

    // Vettical position - doesn't matter - select the first row
    int y = 1 / nPPTY;

    // Setup view #1
    ViewCallback aView1;
    // Set client rect to 2000 x 2000 pixels
    pModelObj->setClientVisibleArea(tools::Rectangle(0, 0, 2000 / nPPTX, 2000 / nPPTY));
    Scheduler::ProcessEventsToIdle();

    // Click at on the left side of A1 cell
    pModelObj->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONDOWN, leftCellSideX, y, /*count=*/ 1, /*buttons=*/ 1, /*modifier=*/0);
    pModelObj->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONUP, leftCellSideX, y, /*count=*/ 1, /*buttons=*/ 1, /*modifier=*/0);
    Scheduler::ProcessEventsToIdle();

    // Check the A1 cell is selected in view #1
    CPPUNIT_ASSERT_EQUAL(SCCOL(0), ScDocShell::GetViewData()->GetCurX());
    CPPUNIT_ASSERT_EQUAL(SCROW(0), ScDocShell::GetViewData()->GetCurY());

    // Click at on the right side of A1 cell
    pModelObj->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONDOWN, rightCellSideX, y, /*count=*/ 1, /*buttons=*/ 1, /*modifier=*/0);
    pModelObj->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONUP, rightCellSideX, y, /*count=*/ 1, /*buttons=*/ 1, /*modifier=*/0);
    Scheduler::ProcessEventsToIdle();

    // Check the A1 cell is selected in view #1
    CPPUNIT_ASSERT_EQUAL(SCCOL(0), ScDocShell::GetViewData()->GetCurX());
    CPPUNIT_ASSERT_EQUAL(SCROW(0), ScDocShell::GetViewData()->GetCurY());

    // Try to check the same scenario in a new view

    // Setup view #2
    SfxLokHelper::createView();
    int nView2 = SfxLokHelper::getView();
    ViewCallback aView2;
    // Set client rect to 2000 x 2000 pixels
    pModelObj->setClientVisibleArea(tools::Rectangle(0, 0, 2000 / nPPTX, 2000 / nPPTY));

    // Lets make sure we are in view #2
    SfxLokHelper::setView(nView2);
    Scheduler::ProcessEventsToIdle();

    // Click at on the left side of A1 cell
    pModelObj->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONDOWN, leftCellSideX, y, /*count=*/ 1, /*buttons=*/ 1, /*modifier=*/0);
    pModelObj->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONUP, leftCellSideX, y, /*count=*/ 1, /*buttons=*/ 1, /*modifier=*/0);
    Scheduler::ProcessEventsToIdle();

    // Check the A1 cell is selected in view #2
    CPPUNIT_ASSERT_EQUAL(SCCOL(0), ScDocShell::GetViewData()->GetCurX());
    CPPUNIT_ASSERT_EQUAL(SCROW(0), ScDocShell::GetViewData()->GetCurY());

    // Click at on the right side of A1 cell
    pModelObj->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONDOWN, rightCellSideX, y, /*count=*/ 1, /*buttons=*/ 1, /*modifier=*/0);
    pModelObj->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONUP, rightCellSideX, y, /*count=*/ 1, /*buttons=*/ 1, /*modifier=*/0);
    Scheduler::ProcessEventsToIdle();

    // Check the A1 cell is selected in view #2
    CPPUNIT_ASSERT_EQUAL(SCCOL(0), ScDocShell::GetViewData()->GetCurX());
    CPPUNIT_ASSERT_EQUAL(SCROW(0), ScDocShell::GetViewData()->GetCurY());
}

void ScTiledRenderingTest::testSidebarLocale()
{
    ScModelObj* pModelObj = createDoc("chart.ods");
    int nView1 = SfxLokHelper::getView();
    ViewCallback aView1;
    SfxViewShell* pView1 = SfxViewShell::Current();
    pView1->SetLOKLocale("en-US");
    SfxLokHelper::createView();
    ViewCallback aView2;
    SfxViewShell* pView2 = SfxViewShell::Current();
    pView2->SetLOKLocale("de-DE");
    TestLokCallbackWrapper::InitializeSidebar();
    Scheduler::ProcessEventsToIdle();
    aView2.m_aStateChanges.clear();

    pModelObj->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONDOWN, /*x=*/1,/*y=*/1,/*count=*/2, /*buttons=*/1, /*modifier=*/0);
    pModelObj->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONUP, /*x=*/1, /*y=*/1, /*count=*/2, /*buttons=*/1, /*modifier=*/0);
    SfxLokHelper::setView(nView1);
    Scheduler::ProcessEventsToIdle();

    auto it = aView2.m_aStateChanges.find(".uno:Sidebar");
    CPPUNIT_ASSERT(it != aView2.m_aStateChanges.end());
    std::string aLocale = it->second.get<std::string>("locale");
    CPPUNIT_ASSERT_EQUAL(std::string("de-DE"), aLocale);
}

// Saving shouldn't trigger an invalidation
void ScTiledRenderingTest::testNoInvalidateOnSave()
{
    comphelper::LibreOfficeKit::setCompatFlag(
        comphelper::LibreOfficeKit::Compat::scPrintTwipsMsgs);

    loadFromURL(u"invalidate-on-save.ods");

    // .uno:Save modifies the original file, make a copy first
    saveAndReload("calc8");
    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    CPPUNIT_ASSERT(pModelObj);
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());

    ScTabViewShell* pView = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
    CPPUNIT_ASSERT(pView);

    Scheduler::ProcessEventsToIdle();

    // track invalidations
    ViewCallback aView;

    uno::Sequence<beans::PropertyValue> aArgs;
    dispatchCommand(mxComponent, ".uno:Save", aArgs);

    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT(!aView.m_bInvalidateTiles);
}

void ScTiledRenderingTest::checkSampleInvalidation(const ViewCallback& rView, bool bFullRow)
{
    // we expect invalidations, but that isn't really important
    CPPUNIT_ASSERT(rView.m_bInvalidateTiles);
    tools::Rectangle aInvalidation;
    for (const auto& rRect : rView.m_aInvalidations)
        aInvalidation.Union(rRect);
    if (!bFullRow)
    {
        // What matters is that we expect that the invalidation does not extend all the
        // way to the max right of the sheet.
        // Here we originally got 32212306 and now ~5056 for a single cell case
        CPPUNIT_ASSERT_LESSEQUAL(tools::Long(8000), aInvalidation.GetWidth());
    }
    else
    {
        // We expect RTL to continue to invalidate the entire row
        // from 0 to end of sheet (see ScDocShell::PostPaint, 'Extend to whole rows'),
        // which is different to the adjusted LTR case which
        // invalidated the row from left of edited cell to right of end
        // of sheet.
        CPPUNIT_ASSERT_LESSEQUAL(tools::Long(0), aInvalidation.Left());
        CPPUNIT_ASSERT_EQUAL(tools::Long(32212230), aInvalidation.Right());
    }
}

void ScTiledRenderingTest::cellInvalidationHelper(ScModelObj* pModelObj, ScTabViewShell* pView, const ScAddress& rAdr,
                                                  bool bAddText, bool bFullRow)
{
    // view
    ViewCallback aView;

    if (bAddText)
    {
        // Type "Hello World" in D8, process events to idle and don't commit yet
        lcl_typeCharsInCell("Hello World", rAdr.Col(), rAdr.Row(), pView, pModelObj, false, false);

        aView.m_bInvalidateTiles = false;
        aView.m_aInvalidations.clear();

        // commit text and process events to idle
        lcl_typeCharsInCell("", rAdr.Col(), rAdr.Row(), pView, pModelObj, true, true);
    }
    else // DeleteText
    {
        pView->SetCursor(rAdr.Col(), rAdr.Row());
        pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::DELETE);
        pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::DELETE);
        Scheduler::ProcessEventsToIdle();
    }

    checkSampleInvalidation(aView, bFullRow);
}

void ScTiledRenderingTest::testCellMinimalInvalidations()
{
    ScAddress aA8(0, 7, 0);
    ScAddress aD4(3, 7, 0);
    ScAddress aD13(3, 12, 0);
    ScAddress aD17(3, 16, 0);

    ScModelObj* pModelObj = createDoc("cell-invalidations.ods");
    ScTabViewShell* pView = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
    CPPUNIT_ASSERT(pModelObj && pView);

    // Changed: Minimized invalidations (bFullRow: false)

    // Common case, LTR, default cell formatting
    cellInvalidationHelper(pModelObj, pView, aA8, true, false);
    cellInvalidationHelper(pModelObj, pView, aD4, true, false);
    // Left-aligned merged cells
    cellInvalidationHelper(pModelObj, pView, aD17, true, false);
    // Delete single cell text case
    cellInvalidationHelper(pModelObj, pView, aA8, false, false);
    // Paste into a single cell
    {
        pView->SetCursor(aD4.Col(), aD4.Row());
        uno::Sequence<beans::PropertyValue> aArgs;
        dispatchCommand(mxComponent, ".uno:Copy", aArgs);
        pView->SetCursor(aA8.Col(), aA8.Row());
        Scheduler::ProcessEventsToIdle();

        ViewCallback aView;
        dispatchCommand(mxComponent, ".uno:Paste", aArgs);
        Scheduler::ProcessEventsToIdle();

        checkSampleInvalidation(aView, false);
    }

    // Unchanged: Non-minimized invalidations (bFullRow: true)

    // Centered merged cells;
    cellInvalidationHelper(pModelObj, pView, aD13, true, true);

    // switch to RTL sheet
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::PAGEDOWN | KEY_MOD1);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::PAGEDOWN | KEY_MOD1);
    Scheduler::ProcessEventsToIdle();

    cellInvalidationHelper(pModelObj, pView, aA8, true, true);
    cellInvalidationHelper(pModelObj, pView, aD4, true, true);
    // Delete Text
    cellInvalidationHelper(pModelObj, pView, aA8, false, true);
}

void ScTiledRenderingTest::testCellInvalidationDocWithExistingZoom()
{
    ScAddress aB7(1, 6, 0);
    ScopedVclPtrInstance<VirtualDevice> xDevice(DeviceFormat::DEFAULT);

    ScModelObj* pModelObj = createDoc("cell-invalidations-200zoom-settings.ods");
    ScTabViewShell* pView = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
    CPPUNIT_ASSERT(pModelObj && pView);

    // Set View #1 to initial 100% and generate a paint
    pModelObj->setClientVisibleArea(tools::Rectangle(0, 0, 19845, 6405));
    pModelObj->setClientZoom(256, 256, 1536, 1536);
    pModelObj->paintTile(*xDevice, 3328, 512, 0, 0, 19968, 3072);

    Scheduler::ProcessEventsToIdle();

    int nView1 = SfxLokHelper::getView();
    // register to track View #1 invalidations
    ViewCallback aView1;

    // Create a View #2
    SfxLokHelper::createView();
    int nView2 = SfxLokHelper::getView();
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    // register to track View #1 invalidations
    ViewCallback aView2;

    // Set View #2 to initial 100% and generate a paint
    pModelObj->setClientVisibleArea(tools::Rectangle(0, 0, 19845, 6405));
    pModelObj->setClientZoom(256, 256, 1536, 1536);
    pModelObj->paintTile(*xDevice, 3328, 512, 0, 0, 19968, 3072);

    // Set View #1 to 50% zoom and generate a paint
    SfxLokHelper::setView(nView1);
    pModelObj->setClientVisibleArea(tools::Rectangle(0, 0, 41150, 13250));
    pModelObj->setClientZoom(256, 256, 3185, 3185);
    pModelObj->paintTile(*xDevice, 3328, 512, 0, 0, 41405, 6370);

    Scheduler::ProcessEventsToIdle();

    // Set View #2 to 200% zoom and generate a paint
    SfxLokHelper::setView(nView2);
    pModelObj->setClientVisibleArea(tools::Rectangle(0, 0, 9574, 3090));
    pModelObj->setClientZoom(256, 256, 741, 741);
    pModelObj->paintTile(*xDevice, 3328, 512, 0, 0, 19968, 3072);

    Scheduler::ProcessEventsToIdle();
    aView1.m_bInvalidateTiles = false;
    aView1.m_aInvalidations.clear();
    aView2.m_bInvalidateTiles = false;
    aView2.m_aInvalidations.clear();

    ScTabViewShell* pView2 = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
    CPPUNIT_ASSERT(pView2);
    pView2->SetCursor(aB7.Col(), aB7.Row());

    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::DELETE);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::DELETE);
    Scheduler::ProcessEventsToIdle();

    // The problem tested for here is with two views at different zooms then a
    // single cell invalidation resulted in the same rectangle reported as two
    // different invalidations rectangles of different scales. While we should
    // get the same invalidation rectangle reported.
    //
    // (B7 is a good choice to use in the real world to see the effect, to both
    // avoid getting the two rects combined into one bigger one, or to have the
    // two separated by so much space the 2nd is off-screen and not seen
    CPPUNIT_ASSERT_EQUAL(size_t(1), aView1.m_aInvalidations.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), aView2.m_aInvalidations.size());

    // That they don't exactly match doesn't matter, we're not checking rounding issues,
    // what matters is that they are not utterly different rectangles
    // Without fix result is originally:
    // Comparing invalidation rectangles Width expected 6214742 actual 26716502 Tolerance 50
    CppUnit::AssertRectEqualWithTolerance("Comparing invalidation rectangles",
                                          aView2.m_aInvalidations[0],
                                          aView1.m_aInvalidations[0],
                                          50);
}

void ScTiledRenderingTest::testInputHandlerSyncedZoom()
{
    ScModelObj* pModelObj = createDoc("cell-edit-300zoom-settings.ods");

    // Set View #1 to initial 150%
    pModelObj->setClientVisibleArea(tools::Rectangle(0, 0, 17933, 4853));
    // Before the fix, this zoom would leave the EditEngine reference device
    // at the zoom level stored in the document, so normal rendering and
    // editing rendering happened with different MapModes
    pModelObj->setClientZoom(256, 256, 1333, 1333);

    ScTabViewShell* pView1 = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
    CPPUNIT_ASSERT(pView1);
    pView1->SetCursor(0, 4); // A5

    Scheduler::ProcessEventsToIdle();

    // Activate edit mode in that A5 cell
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::F2);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::F2);
    Scheduler::ProcessEventsToIdle();

    const ScViewData* pViewData1 = ScDocShell::GetViewData();
    CPPUNIT_ASSERT(pViewData1);

    // Get that active EditView
    EditView* pEditView1 = pViewData1->GetEditView(SC_SPLIT_BOTTOMLEFT);
    CPPUNIT_ASSERT(pEditView1);
    EditEngine* pEditEngine1 = pEditView1->GetEditEngine();
    CPPUNIT_ASSERT(pEditEngine1);
    // These must match, if they don't then text will have a different width in edit and view modes
    CPPUNIT_ASSERT_EQUAL_MESSAGE("EditEngine Ref Dev Zoom and ViewData Zoom should match",
                                 pViewData1->GetZoomX(), pEditEngine1->GetRefMapMode().GetScaleX());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("EditEngine Ref Dev Zoom and ViewData Zoom should match",
                                 pViewData1->GetZoomY(), pEditEngine1->GetRefMapMode().GetScaleY());

    // Create a View #2
    SfxLokHelper::createView();
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());

    // Set View #2 to the same zoom as View #1
    pModelObj->setClientVisibleArea(tools::Rectangle(0, 0, 17933, 4853));
    pModelObj->setClientZoom(256, 256, 1333, 1333);

    ScTabViewShell* pView2 = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
    CPPUNIT_ASSERT(pView2);
    pView2->SetCursor(0, 5); // A6

    Scheduler::ProcessEventsToIdle();

    // Activate edit mode in that A6 cell
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::F2);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::F2);
    Scheduler::ProcessEventsToIdle();

    const ScViewData* pViewData2 = ScDocShell::GetViewData();
    CPPUNIT_ASSERT(pViewData2);

    // Get the View #2 EditView
    EditView* pEditView2 = pViewData2->GetEditView(SC_SPLIT_BOTTOMLEFT);
    CPPUNIT_ASSERT(pEditView2);
    EditEngine* pEditEngine2 = pEditView2->GetEditEngine();
    CPPUNIT_ASSERT(pEditEngine2);
    CPPUNIT_ASSERT(pEditEngine1 != pEditEngine2);
    // Before the fix, these had different settings, resulting in the text
    // dancing for the second user as they toggle in and out of edit mode, but
    // each user should have the same settings.
    CPPUNIT_ASSERT_EQUAL(pEditEngine1->GetControlWord(), pEditEngine2->GetControlWord());
}

void ScTiledRenderingTest::testOptimalRowHeight()
{
    ScModelObj* pModelObj = createDoc("rowheight.ods");
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    ScTabViewShell* pView = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
    CPPUNIT_ASSERT(pView);

    pModelObj->setClientVisibleArea(tools::Rectangle(0, 82545, 22290, 7380));
    pModelObj->setClientZoom(256, 256, 3072, 3072);
    Scheduler::ProcessEventsToIdle();

    constexpr SCROW nRow = 305;
    pView->SetCursor(0, nRow);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Before setOptimalHeight: Row#306 height is invalid!", sal_uInt16(300), pDoc->GetRowHeight(nRow, 0));

    std::vector<sc::ColRowSpan> aRowArr(1, sc::ColRowSpan(nRow, nRow));
    pView->SetWidthOrHeight(false, aRowArr, SC_SIZE_OPTIMAL, 0);

    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("After setOptimalHeight: Row#306 height is invalid!", sal_uInt16(504), pDoc->GetRowHeight(nRow, 0));
}

// if we extend the tiled area to the right and bottom we want two resulting area
// that don't overlap. If they overlap that typically creates an unnecessary full
// screen invalidation.
void ScTiledRenderingTest::testExtendedAreasDontOverlap()
{
    ScModelObj* pModelObj = createDoc("empty.ods");
    ScTabViewShell* pView = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
    CPPUNIT_ASSERT(pModelObj && pView);

    Scheduler::ProcessEventsToIdle();

    // register to track View #1 invalidations
    ViewCallback aView1;

    // extend to the right and bottom
    pModelObj->setClientVisibleArea(tools::Rectangle(0, 0, 39750, 12780));

    Scheduler::ProcessEventsToIdle();

    // we should get two rectangles for the two new areas
    CPPUNIT_ASSERT_EQUAL(size_t(2), aView1.m_aInvalidations.size());

    // And those should not overlap, otherwise they would merge to form
    // a mega rectangle, which defeats the purpose of creating two rects
    // in the first place.
    CPPUNIT_ASSERT_MESSAGE("Invalidations should not overlap",
        !aView1.m_aInvalidations[0].Overlaps(aView1.m_aInvalidations[1]));

    // But they should be adjacent
    CPPUNIT_ASSERT_EQUAL(aView1.m_aInvalidations[0].Top() +
                         aView1.m_aInvalidations[0].GetSize().Height(),
                         aView1.m_aInvalidations[1].Top());
}

// Ensure that editing a shape not in the topleft tile has its text shown inside the shape
// center while editing
void ScTiledRenderingTest::testEditShapeText()
{
    ScModelObj* pModelObj = createDoc("edit-shape-text.ods");

    // Set View to initial 100%
    pModelObj->setClientVisibleArea(tools::Rectangle(0, 0, 28050, 10605));
    pModelObj->setClientZoom(256, 256, 1920, 1920);

    ScTabViewShell* pView = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
    CPPUNIT_ASSERT(pView);

    const bool bShapeSelected = pView->SelectObject(u"Shape 1");
    CPPUNIT_ASSERT(bShapeSelected);

    CPPUNIT_ASSERT(ScDocShell::GetViewData()->GetScDrawView()->AreObjectsMarked());

    Scheduler::ProcessEventsToIdle();

    // Enter editing mode, shape start with no text
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::F2);
    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, awt::Key::F2);

    Scheduler::ProcessEventsToIdle();

    // Grab a snapshot of the center of the shape
    Bitmap aBitmapBefore = getTile(pModelObj, 4096, 3584, 15360, 7680);

    // reuse this to type into the active shape edit
    lcl_typeCharsInCell("MMMMMMM", 0, 0, pView, pModelObj, true, false);

    // Grab a new snapshot of the center of the shape
    Bitmap aBitmapAfter = getTile(pModelObj, 4096, 3584, 15360, 7680);

    // Without the fix, the text is not inside this tile and the before and
    // after are the same.
    CPPUNIT_ASSERT_MESSAGE("Text is not visible", aBitmapBefore != aBitmapAfter);
}

void ScTiledRenderingTest::testCopyMultiSelection()
{
    // Given a document with A1 and A3 as selected cells:
    ScModelObj* pModelObj = createDoc("multi-selection.ods");
    ViewCallback aView1;
    // Get the center of A3:
    uno::Sequence<beans::PropertyValue> aPropertyValues = {
        comphelper::makePropertyValue("ToPoint", OUString("$A$3")),
    };
    dispatchCommand(mxComponent, ".uno:GoToCell", aPropertyValues);
    Point aPoint = aView1.m_aCellCursorBounds.Center();
    // Go to A1:
    aPropertyValues = {
        comphelper::makePropertyValue("ToPoint", OUString("$A$1")),
    };
    dispatchCommand(mxComponent, ".uno:GoToCell", aPropertyValues);
    // Ctrl-click on A3:
    int nCtrl = KEY_MOD1;
    pModelObj->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONDOWN, aPoint.getX(), aPoint.getY(), 1,
                              MOUSE_LEFT, nCtrl);

    // When getting the selection:
    uno::Reference<datatransfer::XTransferable> xTransferable = pModelObj->getSelection();

    // Make sure we get A1+A3 instead of an error:
    CPPUNIT_ASSERT(xTransferable.is());
}

}

CPPUNIT_TEST_SUITE_REGISTRATION(ScTiledRenderingTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
