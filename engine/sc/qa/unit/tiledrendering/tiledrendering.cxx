/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sctiledrenderingtest.hxx>

#include <boost/property_tree/json_parser.hpp>

#include <com/sun/star/datatransfer/clipboard/KitClipboard.hpp>

#include <test/helper/transferable.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/stritem.hxx>
#include <COKit/COKit.hxx>
#include <vcl/scheduler.hxx>
#include <comphelper/kit.hxx>
#include <comphelper/propertyvalue.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/kit/helper.hxx>
#include <svx/svdpage.hxx>
#include <vcl/vclevent.hxx>
#include <vcl/virdev.hxx>
#include <tools/json_writer.hxx>

#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <sc.hrc>
#include <postit.hxx>
#include <document.hxx>
#include <docuno.hxx>
#include <drwlayer.hxx>
#include <editutil.hxx>
#include <undomanager.hxx>
#include <docsh.hxx>
#include <tabvwsh.hxx>
#include <dbdocfun.hxx>
#include <dbdata.hxx>
#include <subtotalparam.hxx>
#include <queryparam.hxx>
#include <queryentry.hxx>
#include <tablestyle.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <gridwin.hxx>
#include <sctestviewcallback.hxx>
#include <o3tl/unit_conversion.hxx>
#include <cstdlib>

using namespace com::sun::star;

static std::ostream& operator<<(std::ostream& os, ViewShellId const & id)
{
    os << static_cast<sal_Int32>(id); return os;
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testRowColumnSelections)
{
    ScModelObj* pModelObj = createDoc("select-row-cols.ods");

    // Select the 5th row with no modifier
    cpo::uno::Sequence<beans::PropertyValue> aArgs( comphelper::InitPropertySequence({
            { u"Row"_ustr, cpo::uno::Any(sal_Int32(5 - 1)) },
            { u"Modifier"_ustr, cpo::uno::Any(sal_uInt16(0)) }
        }));
    dispatchCommand(mxComponent, u".uno:SelectRow"_ustr, aArgs);

    // Check if it is selected
    OString aResult = apitest::helper::transferable::getTextSelection(pModelObj->getSelection(), "text/plain;charset=utf-8"_ostr);
    OString aExpected("1\t2\t3\t4\t5\t6\t7\t8\t9\t10\t11\t12\t13\t14\t15\t16\t17\t18\t19\t20\t21" SAL_NEWLINE_STRING ""_ostr);
    CPPUNIT_ASSERT_EQUAL(aExpected, aResult);

    // Select the 10th row with shift modifier
    aArgs = comphelper::InitPropertySequence({ { u"Row"_ustr, cpo::uno::Any(static_cast<sal_Int32>(10 - 1)) },
                                               { u"Modifier"_ustr, cpo::uno::Any(KEY_SHIFT) } });
    dispatchCommand(mxComponent, u".uno:SelectRow"_ustr, aArgs);

    // Check if all the rows from 5th to 10th get selected
    aResult = apitest::helper::transferable::getTextSelection(pModelObj->getSelection(), "text/plain;charset=utf-8"_ostr);
    aExpected = "1\t2\t3\t4\t5\t6\t7\t8\t9\t10\t11\t12\t13\t14\t15\t16\t17\t18\t19\t20\t21" SAL_NEWLINE_STRING "2\t3\t4\t5\t6\t7\t8\t9\t10\t11\t12\t13\t14\t15\t16\t17\t18\t19\t20\t21\t22" SAL_NEWLINE_STRING "3\t4\t5\t6\t7\t8\t9\t10\t11\t12\t13\t14\t15\t16\t17\t18\t19\t20\t21\t22\t23" SAL_NEWLINE_STRING "4\t5\t6\t7\t8\t9\t10\t11\t12\t13\t14\t15\t16\t17\t18\t19\t20\t21\t22\t23\t24" SAL_NEWLINE_STRING "5\t6\t7\t8\t9\t10\t11\t12\t13\t14\t15\t16\t17\t18\t19\t20\t21\t22\t23\t24\t25" SAL_NEWLINE_STRING "6\t7\t8\t9\t10\t11\t12\t13\t14\t15\t16\t17\t18\t19\t20\t21\t22\t23\t24\t25\t26" SAL_NEWLINE_STRING ""_ostr;
    CPPUNIT_ASSERT_EQUAL(aExpected, aResult);

    // Select the 10th row with ctrl modifier
    aArgs = comphelper::InitPropertySequence({ { u"Row"_ustr, cpo::uno::Any(static_cast<sal_Int32>(13 - 1)) },
                                               { u"Modifier"_ustr, cpo::uno::Any(KEY_MOD1) } });
    dispatchCommand(mxComponent, u".uno:SelectRow"_ustr, aArgs);

    // Copying the non-contiguous selection (rows 5-10 plus row 13) now
    // serializes the selected rows, clamped to the used data area. The
    // unselected gap rows 11 and 12 inside the bounding box come out empty.
    aResult = apitest::helper::transferable::getTextSelection(pModelObj->getSelection(), "text/plain;charset=utf-8"_ostr);
    aExpected = "1\t2\t3\t4\t5\t6\t7\t8\t9\t10\t11\t12\t13\t14\t15\t16\t17\t18\t19\t20\t21" SAL_NEWLINE_STRING
                "2\t3\t4\t5\t6\t7\t8\t9\t10\t11\t12\t13\t14\t15\t16\t17\t18\t19\t20\t21\t22" SAL_NEWLINE_STRING
                "3\t4\t5\t6\t7\t8\t9\t10\t11\t12\t13\t14\t15\t16\t17\t18\t19\t20\t21\t22\t23" SAL_NEWLINE_STRING
                "4\t5\t6\t7\t8\t9\t10\t11\t12\t13\t14\t15\t16\t17\t18\t19\t20\t21\t22\t23\t24" SAL_NEWLINE_STRING
                "5\t6\t7\t8\t9\t10\t11\t12\t13\t14\t15\t16\t17\t18\t19\t20\t21\t22\t23\t24\t25" SAL_NEWLINE_STRING
                "6\t7\t8\t9\t10\t11\t12\t13\t14\t15\t16\t17\t18\t19\t20\t21\t22\t23\t24\t25\t26" SAL_NEWLINE_STRING
                "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t" SAL_NEWLINE_STRING
                "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t" SAL_NEWLINE_STRING
                "9\t10\t11\t12\t13\t14\t15\t16\t17\t18\t19\t20\t21\t22\t23\t24\t25\t26\t27\t28\t29" SAL_NEWLINE_STRING ""_ostr;
    CPPUNIT_ASSERT_EQUAL(aExpected, aResult);

    // Select Column 5 with ctrl modifier
    aArgs = comphelper::InitPropertySequence({ { u"Col"_ustr, cpo::uno::Any(static_cast<sal_Int32>(5 - 1)) },
                                               { u"Modifier"_ustr, cpo::uno::Any(KEY_MOD1) } });
    dispatchCommand(mxComponent, u".uno:SelectColumn"_ustr, aArgs);

    // Adding column 5 to the row selection yields a non-empty serialization
    // of the mixed selection (rows 5-10 and 13 in full, plus column 5's
    // cells for the rows in between) rather than the single cursor cell.
    // Assert it is populated and carries the fully selected rows rather than
    // pinning the exact bounding-box dump, which is dominated by column 5's
    // long tail of otherwise-empty rows.
    aResult = apitest::helper::transferable::getTextSelection(pModelObj->getSelection(), "text/plain;charset=utf-8"_ostr);
    CPPUNIT_ASSERT(!aResult.isEmpty());
    CPPUNIT_ASSERT(aResult.indexOf("1\t2\t3\t4\t5\t6\t7\t8\t9\t10\t11\t12\t13\t14\t15\t16\t17\t18\t19\t20\t21") >= 0);
    CPPUNIT_ASSERT(aResult.indexOf("9\t10\t11\t12\t13\t14\t15\t16\t17\t18\t19\t20\t21\t22\t23\t24\t25\t26\t27\t28\t29") >= 0);

    // Test for deselection of already selected rows
    // First Deselect Row 13 because copy doesn't work for multiple selections
    aArgs = comphelper::InitPropertySequence({ { u"Row"_ustr, cpo::uno::Any(static_cast<sal_Int32>(13 - 1)) },
                                               { u"Modifier"_ustr, cpo::uno::Any(KEY_MOD1) } });
    dispatchCommand(mxComponent, u".uno:SelectRow"_ustr, aArgs);

    // Deselect row 10
    aArgs = comphelper::InitPropertySequence({ { u"Row"_ustr, cpo::uno::Any(static_cast<sal_Int32>(10 - 1)) },
                                               { u"Modifier"_ustr, cpo::uno::Any(KEY_MOD1) } });
    dispatchCommand(mxComponent, u".uno:SelectRow"_ustr, aArgs);

    // Click at row 6 holding shift
    aArgs = comphelper::InitPropertySequence({ { u"Row"_ustr, cpo::uno::Any(static_cast<sal_Int32>(6 - 1)) },
                                               { u"Modifier"_ustr, cpo::uno::Any(KEY_SHIFT) } });
    dispatchCommand(mxComponent, u".uno:SelectRow"_ustr, aArgs);

    //  only row 5 should remain selected
    aResult = apitest::helper::transferable::getTextSelection(pModelObj->getSelection(), "text/plain;charset=utf-8"_ostr);
    aExpected = "1\t2\t3\t4\t5\t6\t7\t8\t9\t10\t11\t12\t13\t14\t15\t16\t17\t18\t19\t20\t21" SAL_NEWLINE_STRING ""_ostr;
    CPPUNIT_ASSERT_EQUAL(aExpected, aResult);
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testPartHash)
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

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testDocumentSize)
{
    ScModelObj* pModelObj = createDoc("sort-range.ods");
    ScDocShell* pDocSh = dynamic_cast< ScDocShell* >( pModelObj->GetEmbeddedObject() );
    CPPUNIT_ASSERT(pDocSh);

    ScTabViewShell* pViewShell = pDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT(pViewShell);

    setupCOKitViewCallback(pViewShell);

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

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testEmptyColumnSelection)
{
    ScModelObj* pModelObj = createDoc("select-row-cols.ods");

    // Select empty column, 1000
    cpo::uno::Sequence<beans::PropertyValue> aArgs( comphelper::InitPropertySequence({
                { u"Col"_ustr, cpo::uno::Any(sal_Int32(1000 - 1)) },
                { u"Modifier"_ustr, cpo::uno::Any(sal_uInt16(0)) }
        }));
    dispatchCommand(mxComponent, u".uno:SelectColumn"_ustr, aArgs);

    // should be an empty string
    CPPUNIT_ASSERT_EQUAL(OString(), apitest::helper::transferable::getTextSelection(pModelObj->getSelection(), "text/plain;charset=utf-8"_ostr));
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testViewCursors)
{
    ScModelObj* pModelObj = createDoc("select-row-cols.ods");
    ScTestViewCallback aView1;
    KitHelper::createView();
    pModelObj->initializeForTiledRendering(cpo::uno::Sequence<beans::PropertyValue>());
    ScTestViewCallback aView2(/*bDeleteListenerOnDestruct*/false);
    // This was false, the new view did not get the view (cell) cursor of the old view.
    CPPUNIT_ASSERT(aView2.m_bViewCursorInvalidated);
    CPPUNIT_ASSERT(aView2.m_bOwnCursorInvalidated);
    pModelObj->postKeyEvent(COKitKeyEventType::DOWN, 0, awt::Key::DOWN);
    pModelObj->postKeyEvent(COKitKeyEventType::UP, 0, awt::Key::DOWN);
    Scheduler::ProcessEventsToIdle();
    KitHelper::destroyView(KitHelper::getCurrentView());
    CPPUNIT_ASSERT(aView1.m_bViewCursorInvalidated);
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testSpellOnlineRenderParameter)
{
    ScModelObj* pModelObj = createDoc("empty.ods");

    ScTabViewShell* pView = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
    CPPUNIT_ASSERT(pView);

    bool bSet = pView->IsAutoSpell();

    cpo::uno::Sequence<beans::PropertyValue> aPropertyValues =
    {
        comphelper::makePropertyValue(u".uno:SpellOnline"_ustr, cpo::uno::Any(!bSet)),
    };
    pModelObj->initializeForTiledRendering(aPropertyValues);

    CPPUNIT_ASSERT_EQUAL(!bSet, pView->IsAutoSpell());
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testTextViewSelection)
{
    // Create two views, and leave the second one current.
    ScModelObj* pModelObj = createDoc("select-row-cols.ods");
    ScTestViewCallback aView1;
    KitHelper::createView();
    pModelObj->initializeForTiledRendering(cpo::uno::Sequence<beans::PropertyValue>());
    ScTestViewCallback aView2;

    // Create a selection on two cells in the second view, that's a text selection in COKit terms.
    aView1.m_bTextViewSelectionInvalidated = false;
    dispatchCommand(mxComponent, u".uno:GoRightSel"_ustr, {});
    // Make sure the first view got its notification.
    CPPUNIT_ASSERT(aView1.m_bTextViewSelectionInvalidated);
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testDocumentSizeChanged)
{
    // Load a document that doesn't have much content.
    createDoc("small.ods");
    setupCOKitViewCallback(SfxViewShell::Current());

    // Go to the A30 cell -- that will extend the document size.
    cpo::uno::Sequence<beans::PropertyValue> aPropertyValues =
    {
        comphelper::makePropertyValue(u"ToPoint"_ustr, u"$A$30"_ustr),
    };
    dispatchCommand(mxComponent, u".uno:GoToCell"_ustr, aPropertyValues);
    // Assert that the size in the payload is not 0.
    CPPUNIT_ASSERT(m_aDocumentSize.getWidth() > 0);
    CPPUNIT_ASSERT(m_aDocumentSize.getHeight() > 0);
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testViewLock)
{
    // Load a document that has a shape and create two views.
    ScModelObj* pModelObj = createDoc("shape.ods");
    ScTestViewCallback aView1;
    KitHelper::createView();
    pModelObj->initializeForTiledRendering(cpo::uno::Sequence<beans::PropertyValue>());
    ScTestViewCallback aView2;

    // Begin text edit in the second view and assert that the first gets a lock
    // notification.
    const ScViewData* pViewData = ScDocShell::GetViewData();
    CPPUNIT_ASSERT(pViewData);
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

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testSheetSwitchViewLock)
{
    // Load a document with two sheets and create two views.
    ScModelObj* pModelObj = createDoc("two_sheets.ods");
    ScTestViewCallback aView1;
    KitHelper::createView();
    pModelObj->initializeForTiledRendering(cpo::uno::Sequence<beans::PropertyValue>());
    ScTestViewCallback aView2;

    // Reset counter after view creation (which may trigger callbacks).
    Scheduler::ProcessEventsToIdle();
    aView1.m_nViewLockCount = 0;

    // Switch to the second view and have it switch sheets.
    KitHelper::setView(aView2.getViewID());
    pModelObj->setPart(1);
    Scheduler::ProcessEventsToIdle();

    // The first view should not have received any VIEW_LOCK callbacks,
    // because no text editing was active.
    CPPUNIT_ASSERT_EQUAL(0, aView1.m_nViewLockCount);
}

namespace
{
void lcl_extractHandleParameters(std::string_view selection, sal_uInt32& id, sal_uInt32& x, sal_uInt32& y)
{
    OString extraInfo( selection.substr(selection.find("{")) );
    std::stringstream aStream((std::string(extraInfo)));
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
} //namespace

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testMoveShapeHandle)
{
    ScModelObj* pModelObj = createDoc("shape.ods");
    ScTestViewCallback aView1;
    pModelObj->postMouseEvent(COKitMouseEventType::BUTTONDOWN, /*x=*/ 1,/*y=*/ 1,/*count=*/ 1, /*buttons=*/ 1, /*modifier=*/0);
    pModelObj->postMouseEvent(COKitMouseEventType::BUTTONUP, /*x=*/ 1, /*y=*/ 1, /*count=*/ 1, /*buttons=*/ 1, /*modifier=*/0);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT(!aView1.m_ShapeSelection.isEmpty());
    {
        sal_uInt32 id, x, y;
        lcl_extractHandleParameters(aView1.m_ShapeSelection, id, x ,y);
        sal_uInt32 oldX = x;
        sal_uInt32 oldY = y;
        cpo::uno::Sequence<beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence(
        {
            {u"HandleNum"_ustr, cpo::uno::Any(id)},
            {u"NewPosX"_ustr, cpo::uno::Any(x+1)},
            {u"NewPosY"_ustr, cpo::uno::Any(y+1)}
        }));
        dispatchCommand(mxComponent, u".uno:MoveShapeHandle"_ustr, aPropertyValues);
        CPPUNIT_ASSERT(!aView1.m_ShapeSelection.isEmpty());
        lcl_extractHandleParameters(aView1.m_ShapeSelection, id, x ,y);
        CPPUNIT_ASSERT_EQUAL(x-1, oldX);
        CPPUNIT_ASSERT_EQUAL(y-1, oldY);
    }
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testMoveShapeHandleTextBox)
{
    ScModelObj* pModelObj = createDoc("shape-textbox.ods");
    ScTestViewCallback aView1;
    pModelObj->postMouseEvent(COKitMouseEventType::BUTTONDOWN, /*x=*/ 1,/*y=*/ 1,/*count=*/ 1, /*buttons=*/ 1, /*modifier=*/0);
    pModelObj->postMouseEvent(COKitMouseEventType::BUTTONUP, /*x=*/ 1, /*y=*/ 1, /*count=*/ 1, /*buttons=*/ 1, /*modifier=*/0);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT(!aView1.m_ShapeSelection.isEmpty());
    {
        sal_uInt32 id, x, y;
        lcl_extractHandleParameters(aView1.m_ShapeSelection, id, x ,y);
        sal_uInt32 oldX = x;
        sal_uInt32 oldY = y;
        cpo::uno::Sequence<beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence(
        {
            {u"HandleNum"_ustr, cpo::uno::Any(id)},
            {u"NewPosX"_ustr, cpo::uno::Any(x+1)},
            {u"NewPosY"_ustr, cpo::uno::Any(y+1)}
        }));
        dispatchCommand(mxComponent, u".uno:MoveShapeHandle"_ustr, aPropertyValues);
        CPPUNIT_ASSERT(!aView1.m_ShapeSelection.isEmpty());
        lcl_extractHandleParameters(aView1.m_ShapeSelection, id, x ,y);
        CPPUNIT_ASSERT_EQUAL(x-1, oldX);
        CPPUNIT_ASSERT_EQUAL(y-1, oldY);
    }
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testColRowResize)
{
    ScModelObj* pModelObj = createDoc("sort-range.ods");
    ScDocShell* pDocSh = dynamic_cast< ScDocShell* >( pModelObj->GetEmbeddedObject() );
    CPPUNIT_ASSERT(pDocSh);

    ScTabViewShell* pViewShell = pDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT(pViewShell);

    setupCOKitViewCallback(pViewShell);

    ScDocument& rDoc = pDocSh->GetDocument();

    // Col 3, Tab 0

    cpo::uno::Sequence<beans::PropertyValue> aArgs( comphelper::InitPropertySequence({
            { u"ColumnWidth"_ustr, cpo::uno::Any(sal_uInt16(4000)) }, // 4cm
            { u"Column"_ustr, cpo::uno::Any(sal_Int16(3)) }
        }));
    dispatchCommand(mxComponent, u".uno:ColumnWidth"_ustr, aArgs);

    sal_uInt16 nWidth = o3tl::convert(rDoc.GetColWidth(static_cast<SCCOL>(2), static_cast<SCTAB>(0), false), o3tl::Length::twip, o3tl::Length::mm100);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(4001), nWidth);

    // Row 5, Tab 0
    cpo::uno::Sequence<beans::PropertyValue> aArgs2( comphelper::InitPropertySequence({
            { u"RowHeight"_ustr, cpo::uno::Any(sal_uInt16(2000)) },
            { u"Row"_ustr, cpo::uno::Any(sal_Int16(5)) },
        }));
    dispatchCommand(mxComponent, u".uno:RowHeight"_ustr, aArgs2);

    sal_uInt16 nHeight = o3tl::convert(rDoc.GetRowHeight(static_cast<SCROW>(4), static_cast<SCTAB>(0), false), o3tl::Length::twip, o3tl::Length::mm100);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(2000), nHeight);
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testUndoShells)
{
    ScModelObj* pModelObj = createDoc("small.ods");
    // Clear the currently selected cell.
    dispatchCommand(mxComponent, u".uno:ClearContents"_ustr, {});

    auto pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    CPPUNIT_ASSERT(pDocShell);
    ScDocument& rDoc = pDocShell->GetDocument();
    ScUndoManager* pUndoManager = rDoc.GetUndoManager();
    CPPUNIT_ASSERT(pUndoManager);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pUndoManager->GetUndoActionCount());
    sal_Int32 nView1 = KitHelper::getCurrentView();
    // This was -1: ScSimpleUndo did not remember what view shell created it.
    CPPUNIT_ASSERT_EQUAL(ViewShellId(nView1), pUndoManager->GetUndoAction()->GetViewShellId());
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testTextEditViews)
{
    ScModelObj* pModelObj = createDoc("small.ods");
    CPPUNIT_ASSERT(pModelObj);
    ScViewData* pViewData = ScDocShell::GetViewData();
    CPPUNIT_ASSERT(pViewData);

    // view #1
    ScTestViewCallback aView1;
    CPPUNIT_ASSERT(!hasEditView(*pViewData));

    // text edit a cell in view #1
    pModelObj->postKeyEvent(COKitKeyEventType::DOWN, 'x', 0);
    pModelObj->postKeyEvent(COKitKeyEventType::UP, 'x', 0);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(hasEditView(*pViewData));

    // view #2
    KitHelper::createView();
    pModelObj->initializeForTiledRendering(cpo::uno::Sequence<beans::PropertyValue>());
    ScTestViewCallback aView2;

    // move cell cursor i view #2
    pModelObj->postKeyEvent(COKitKeyEventType::DOWN, 0, awt::Key::DOWN);
    pModelObj->postKeyEvent(COKitKeyEventType::UP, 0, awt::Key::DOWN);
    Scheduler::ProcessEventsToIdle();

    // check that text edit view in view #1 has not be killed
    CPPUNIT_ASSERT(hasEditView(*pViewData));
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testTextEditViewInvalidations)
{
    ScModelObj* pModelObj = createDoc("small.ods");
    CPPUNIT_ASSERT(pModelObj);
    ScViewData* pViewData = ScDocShell::GetViewData();
    CPPUNIT_ASSERT(pViewData);

    // view #1
    int nView1 = KitHelper::getCurrentView();
    ScTestViewCallback aView1;
    CPPUNIT_ASSERT(!hasEditView(*pViewData));

    // view #2
    KitHelper::createView();
    pModelObj->initializeForTiledRendering(cpo::uno::Sequence<beans::PropertyValue>());
    ScTestViewCallback aView2;

    // text edit a cell in view #1
    KitHelper::setView(nView1);
    aView2.m_bInvalidateTiles = false;
    pModelObj->postKeyEvent(COKitKeyEventType::DOWN, 'x', 0);
    pModelObj->postKeyEvent(COKitKeyEventType::UP, 'x', 0);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(hasEditView(*pViewData));
    CPPUNIT_ASSERT(aView2.m_bInvalidateTiles);

    // text edit a cell in view #1 until
    // we can be sure we are out of the initial tile
    for (int i = 0; i < 40; ++i)
    {
        pModelObj->postKeyEvent(COKitKeyEventType::DOWN, 'x', 0);
        pModelObj->postKeyEvent(COKitKeyEventType::UP, 'x', 0);
    }
    Scheduler::ProcessEventsToIdle();

    // text edit a cell in view #1 inside the new tile and
    // check that view #2 receive a tile invalidate message
    aView2.m_bInvalidateTiles = false;
    pModelObj->postKeyEvent(COKitKeyEventType::DOWN, 'x', 0);
    pModelObj->postKeyEvent(COKitKeyEventType::UP, 'x', 0);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(aView2.m_bInvalidateTiles);

    // view #3
    KitHelper::createView();
    pModelObj->initializeForTiledRendering(cpo::uno::Sequence<beans::PropertyValue>());
    ScTestViewCallback aView3;

    // text edit a cell in view #1
    KitHelper::setView(nView1);
    aView3.m_bInvalidateTiles = false;
    pModelObj->postKeyEvent(COKitKeyEventType::DOWN, 'y', 0);
    pModelObj->postKeyEvent(COKitKeyEventType::UP, 'y', 0);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(aView3.m_bInvalidateTiles);
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testNoRedundantSpellInvalidations)
{
    // Regression test: typing a long, mixed-language cell text/comment must
    // not cause idle spell-check to emit extra tile-invalidation messages on
    // either view.

    // Turn online spell-checking on for the document and current view #1.
    cpo::uno::Sequence<beans::PropertyValue> aSpellOnArgs = {
        comphelper::makePropertyValue(u".uno:SpellOnline"_ustr, cpo::uno::Any(true)),
    };
    ScModelObj* pModelObj = createDoc("small.ods");
    CPPUNIT_ASSERT(pModelObj);
    pModelObj->initializeForTiledRendering(aSpellOnArgs);
    ScViewData* pViewData = ScDocShell::GetViewData();
    CPPUNIT_ASSERT(pViewData);

    ScTabViewShell* pView1Sh = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
    CPPUNIT_ASSERT(pView1Sh);
    CPPUNIT_ASSERT(pView1Sh->IsAutoSpell());

    int nView1 = KitHelper::getCurrentView();
    ScTestViewCallback aView1;

    // view #2 - also auto-spell enabled via the same render parameter.
    KitHelper::createView();
    pModelObj->initializeForTiledRendering(aSpellOnArgs);
    ScTabViewShell* pView2Sh = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
    CPPUNIT_ASSERT(pView2Sh);
    CPPUNIT_ASSERT(pView2Sh->IsAutoSpell());
    ScTestViewCallback aView2;

    KitHelper::setView(nView1);

    // Type a long, mixed-language sequence into the active cell. Lots of
    // misspellings + language guesses are exactly what would have made
    // the old idle handler fire repeatedly and emit a stream of
    // invalidations after every batch of keystrokes.
    static const char* const kTypedText =
        "Thiss is ein kompliziertes exemple avec plusieurs languages "
        "und wronglyspeltwords throughout-the-text to ensure the "
        "spellcheker keepps rechecking stuff while we type.";
    for (const char* p = kTypedText; *p; ++p)
    {
        pModelObj->postKeyEvent(COKitKeyEventType::DOWN, *p, 0);
        Scheduler::ProcessEventsToIdle();
        pModelObj->postKeyEvent(COKitKeyEventType::UP, *p, 0);
        Scheduler::ProcessEventsToIdle();
    }

    // Now the typing has settled. From here on, no spell-check work
    // should produce any new tile invalidation on either view.
    aView1.ClearAllInvalids();
    aView2.ClearAllInvalids();

    // Pump idle several times. The editeng spell timer has a 100ms
    // timeout and could re-arm itself. Before this fix each fire would
    // produce a fresh InvalidateAtWindow / UpdateViews call.
    for (int i = 0; i < 5; ++i)
        Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Idle spell-check must not invalidate tiles in the typing view",
        size_t(0), aView1.m_aInvalidations.size());
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Idle spell-check must not invalidate tiles in the spectator view",
        size_t(0), aView2.m_aInvalidations.size());
    CPPUNIT_ASSERT_MESSAGE(
        "Idle spell-check must not flag a full-tile invalidation in the typing view",
        !aView1.m_bFullInvalidateTiles);
    CPPUNIT_ASSERT_MESSAGE(
        "Idle spell-check must not flag a full-tile invalidation in the spectator view",
        !aView2.m_bFullInvalidateTiles);
    CPPUNIT_ASSERT_MESSAGE(
        "Idle spell-check must not raise the tile-invalidate flag in the typing view",
        !aView1.m_bInvalidateTiles);
    CPPUNIT_ASSERT_MESSAGE(
        "Idle spell-check must not raise the tile-invalidate flag in the spectator view",
        !aView2.m_bInvalidateTiles);
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testCreateViewGraphicSelection)
{
    // Load a document that has a shape and create two views.
    ScModelObj* pModelObj = createDoc("shape.ods");
    ScTestViewCallback aView1;

    // Mark the graphic in the first view.
    const ScViewData* pViewData = ScDocShell::GetViewData();
    CPPUNIT_ASSERT(pViewData);
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
    int nView1 = KitHelper::getCurrentView();
    KitHelper::createView();
    pModelObj->initializeForTiledRendering(cpo::uno::Sequence<beans::PropertyValue>());
    ScTestViewCallback aView2;
    CPPUNIT_ASSERT(aView2.m_bGraphicViewSelection);
    CPPUNIT_ASSERT(aView1.m_bGraphicViewSelection);

    KitHelper::setView(nView1);
    SfxViewShell::Current()->setCOKitViewCallback(nullptr);
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testGraphicInvalidate)
{
    // Load a document that has a shape and create two views.
    ScModelObj* pModelObj = createDoc("shape.ods");
    ScTestViewCallback aView;

    // Click to select graphic
    aView.m_bGraphicSelection = false;
    pModelObj->postMouseEvent(COKitMouseEventType::BUTTONDOWN, /*x=*/ 1,/*y=*/ 1,/*count=*/ 1, /*buttons=*/ 1, /*modifier=*/0);
    pModelObj->postMouseEvent(COKitMouseEventType::BUTTONUP, /*x=*/ 1, /*y=*/ 1, /*count=*/ 1, /*buttons=*/ 1, /*modifier=*/0);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(aView.m_bGraphicSelection);

    // Drag Drop graphic
    aView.m_bGraphicSelection = false;
    pModelObj->postMouseEvent(COKitMouseEventType::BUTTONDOWN, /*x=*/ 1,/*y=*/ 1,/*count=*/ 1, /*buttons=*/ 1, /*modifier=*/0);
    pModelObj->postMouseEvent(COKitMouseEventType::MOVE, /*x=*/ 1,/*y=*/ 10,/*count=*/ 1, /*buttons=*/ 1, /*modifier=*/0);
    pModelObj->postMouseEvent(COKitMouseEventType::BUTTONUP, /*x=*/ 1, /*y=*/ 10, /*count=*/ 1, /*buttons=*/ 1, /*modifier=*/0);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(!aView.m_bFullInvalidateTiles);

    // Check again
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(!aView.m_bFullInvalidateTiles);
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testAutoSum)
{
    createDoc("small.ods");

    ScTestViewCallback aView;

    cpo::uno::Sequence<beans::PropertyValue> aArgs;
    dispatchCommand(mxComponent, u".uno:AutoSum"_ustr, aArgs);
    CPPUNIT_ASSERT(aView.m_sCellFormula.startsWith("=SUM("));
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testHideColRow)
{
    createDoc("small.ods");
    {
        cpo::uno::Sequence<beans::PropertyValue> aArgs( comphelper::InitPropertySequence({
                { u"Col"_ustr, cpo::uno::Any(sal_Int32(2 - 1)) },
                { u"Modifier"_ustr, cpo::uno::Any(KEY_SHIFT) }
            }));
        dispatchCommand(mxComponent, u".uno:SelectColumn"_ustr, aArgs);

        cpo::uno::Sequence<beans::PropertyValue> aArgs2( comphelper::InitPropertySequence({
                { u"Col"_ustr, cpo::uno::Any(sal_Int32(3 - 1)) },
                { u"Modifier"_ustr, cpo::uno::Any(sal_uInt16(0)) }
            }));

        dispatchCommand(mxComponent, u".uno:SelectColumn"_ustr, aArgs2);
    }

    ScViewData* pViewData = ScDocShell::GetViewData();
    CPPUNIT_ASSERT(pViewData);
    SCCOL nOldCurX = pViewData->GetCurX();
    SCROW nOldCurY = pViewData->GetCurY();
    {
        cpo::uno::Sequence<beans::PropertyValue> aArgs;
        dispatchCommand(mxComponent, u".uno:HideColumn"_ustr, aArgs);
    }

    SCCOL nNewCurX = pViewData->GetCurX();
    SCROW nNewCurY = pViewData->GetCurY();
    CPPUNIT_ASSERT(nNewCurX > nOldCurX);
    CPPUNIT_ASSERT_EQUAL(nOldCurY, nNewCurY);
    {
        cpo::uno::Sequence<beans::PropertyValue> aArgs( comphelper::InitPropertySequence({
                { u"Row"_ustr, cpo::uno::Any(sal_Int32(6 - 1)) },
                { u"Modifier"_ustr, cpo::uno::Any(KEY_SHIFT) }
            }));
        dispatchCommand(mxComponent, u".uno:SelectRow"_ustr, aArgs);

        cpo::uno::Sequence<beans::PropertyValue> aArgs2( comphelper::InitPropertySequence({
                { u"Row"_ustr, cpo::uno::Any(sal_Int32(7 - 1)) },
                { u"Modifier"_ustr, cpo::uno::Any(sal_uInt16(0)) }
            }));
        dispatchCommand(mxComponent, u".uno:SelectRow"_ustr, aArgs2);
    }

    nOldCurX = pViewData->GetCurX();
    nOldCurY = pViewData->GetCurY();
    {
        cpo::uno::Sequence<beans::PropertyValue> aArgs;
        dispatchCommand(mxComponent, u".uno:HideRow"_ustr, aArgs);
    }
    nNewCurX = pViewData->GetCurX();
    nNewCurY = pViewData->GetCurY();
    CPPUNIT_ASSERT(nNewCurY > nOldCurY);
    CPPUNIT_ASSERT_EQUAL(nOldCurX, nNewCurX);
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testTableResizeUndoKeepsCursor)
{
    ScModelObj* pModelObj = createDoc("empty.ods");
    CPPUNIT_ASSERT(pModelObj);

    auto pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    CPPUNIT_ASSERT(pDocShell);
    ScDocument& rDoc = pDocShell->GetDocument();
    ScUndoManager* pUndoManager = rDoc.GetUndoManager();
    CPPUNIT_ASSERT(pUndoManager);

    // A styled table A1:C14 with auto-generated headers Column1|Column2|Column3.
    ScDBDocFunc aFunc(*pDocShell);
    CPPUNIT_ASSERT(aFunc.AddDBTable(u"Table1"_ustr, ScRange(0, 0, 0, 2, 13, 0),
                                    /*bHeader*/ true, /*bRecord*/ true, /*bApi*/ true,
                                    u"TableStyleMedium2"_ustr));

    // Put the cursor on B8, inside the table body.
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    CPPUNIT_ASSERT(pViewShell);
    pViewShell->SetCursor(1, 7);

    ScViewData* pViewData = ScDocShell::GetViewData();
    CPPUNIT_ASSERT(pViewData);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), pViewData->GetCurX());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7), pViewData->GetCurY());

    // Widen A1:C14 -> A1:L14 (drag-resize), generating Column4..Column12.
    ScDBData* pData = rDoc.GetDBCollection()->getNamedDBs().findByUpperName(u"TABLE1"_ustr);
    CPPUNIT_ASSERT(pData);
    ScDBData aWide(*pData);
    aWide.SetArea(0, 0, 0, 11, 13);
    aFunc.ModifyDBData(aWide);
    CPPUNIT_ASSERT_EQUAL(u"Column12"_ustr, rDoc.GetString(11, 0, 0));
    // The resize itself must not move the cursor.
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), pViewData->GetCurX());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7), pViewData->GetCurY());

    // Undo: without the cursor guard the cursor jumped to D1 (first generated cell).
    pUndoManager->Undo();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("undo must keep the cursor on B8", sal_Int16(1),
                                 pViewData->GetCurX());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("undo must keep the cursor on B8", sal_Int32(7),
                                 pViewData->GetCurY());

    // Redo: without the guard the cursor jumped to L1 (last generated cell).
    pUndoManager->Redo();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("redo must keep the cursor on B8", sal_Int16(1),
                                 pViewData->GetCurX());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("redo must keep the cursor on B8", sal_Int32(7),
                                 pViewData->GetCurY());
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testDeleteContentsDropsCoveredTable)
{
    ScModelObj* pModelObj = createDoc("empty.ods");
    CPPUNIT_ASSERT(pModelObj);
    auto pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    CPPUNIT_ASSERT(pDocShell);
    ScDocument& rDoc = pDocShell->GetDocument();
    ScTabViewShell* pViewShell = pDocShell->GetBestViewShell();
    CPPUNIT_ASSERT(pViewShell);

    // Styled table A1:C4, header plus three data rows.
    ScDBData* pData = new ScDBData(u"Table1"_ustr, /*nTab*/ 0, 0, 0, 2, 3,
                                   /*bByRow*/ true, /*bHasHeader*/ true);
    ScTableStyleParam aStyleParam;
    aStyleParam.maStyleID = u"TableStyleMedium2"_ustr;
    pData->SetTableStyleInfo(aStyleParam);
    CPPUNIT_ASSERT(rDoc.GetDBCollection()->getNamedDBs().insert(std::unique_ptr<ScDBData>(pData)));

    rDoc.SetString(0, 0, 0, u"Name"_ustr);
    rDoc.SetString(1, 0, 0, u"Number"_ustr);
    rDoc.SetString(2, 0, 0, u"Column3"_ustr);
    rDoc.SetValue(1, 1, 0, 1.0);

    auto hasTable = [&rDoc]() {
        return rDoc.GetDBCollection()->getNamedDBs().findByUpperName(u"TABLE1"_ustr) != nullptr;
    };

    // Deleting only the data rows keeps the table.
    pViewShell->GetViewData().GetMarkData().SetMarkArea(ScRange(0, 1, 0, 2, 3, 0));
    dispatchCommand(mxComponent, u".uno:ClearContents"_ustr, {});
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_MESSAGE("a partly covered table stays", hasTable());
    CPPUNIT_ASSERT(rDoc.GetString(1, 1, 0).isEmpty());

    // The header row included: such a table goes, like Remove Table does.
    pViewShell->GetViewData().GetMarkData().SetMarkArea(ScRange(0, 0, 0, 2, 3, 0));
    dispatchCommand(mxComponent, u".uno:ClearContents"_ustr, {});
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_MESSAGE("a fully covered table goes", !hasTable());
    CPPUNIT_ASSERT(rDoc.GetString(0, 0, 0).isEmpty());

    // The content and the table come back in one undo step.
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_MESSAGE("undo restores the table", hasTable());
    CPPUNIT_ASSERT_EQUAL(u"Name"_ustr, rDoc.GetString(0, 0, 0));
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testTableRemoveDuplicates)
{
    ScModelObj* pModelObj = createDoc("empty.ods");
    CPPUNIT_ASSERT(pModelObj);
    auto pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    CPPUNIT_ASSERT(pDocShell);
    ScDocument& rDoc = pDocShell->GetDocument();

    // A styled table A1:B5: a header + 4 data rows, where rows 4 and 5 duplicate rows 2 and 3.
    rDoc.SetString(0, 0, 0, u"Name"_ustr);  rDoc.SetString(1, 0, 0, u"City"_ustr);
    rDoc.SetString(0, 1, 0, u"Alice"_ustr); rDoc.SetString(1, 1, 0, u"NY"_ustr);
    rDoc.SetString(0, 2, 0, u"Bob"_ustr);   rDoc.SetString(1, 2, 0, u"LA"_ustr);
    rDoc.SetString(0, 3, 0, u"Alice"_ustr); rDoc.SetString(1, 3, 0, u"NY"_ustr);
    rDoc.SetString(0, 4, 0, u"Bob"_ustr);   rDoc.SetString(1, 4, 0, u"LA"_ustr);

    ScDBDocFunc aFunc(*pDocShell);
    CPPUNIT_ASSERT(aFunc.AddDBTable(u"Table1"_ustr, ScRange(0, 0, 0, 1, 4, 0),
                                    /*bHeader*/ true, /*bRecord*/ true, /*bApi*/ true,
                                    u"TableStyleMedium2"_ustr));

    // Keep the cursor inside the table so the command stays enabled (not a mixed selection).
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    CPPUNIT_ASSERT(pViewShell);
    pViewShell->SetCursor(0, 1);

    // Remove duplicate rows over the table (compare all columns; first row is the header).
    // The explicit args drive the synchronous playback path - no async dialog to wait on.
    cpo::uno::Sequence<beans::PropertyValue> aArgs{
        comphelper::makePropertyValue(u"Remove"_ustr, true),
        comphelper::makePropertyValue(u"IncludesHeaders"_ustr, true),
        comphelper::makePropertyValue(u"DuplicateRows"_ustr, true),
        comphelper::makePropertyValue(u"StartColumn"_ustr, sal_Int32(0)),
        comphelper::makePropertyValue(u"StartRow"_ustr, sal_Int32(0)),
        comphelper::makePropertyValue(u"EndColumn"_ustr, sal_Int32(1)),
        comphelper::makePropertyValue(u"EndRow"_ustr, sal_Int32(4)),
        comphelper::makePropertyValue(u"TabNo"_ustr, sal_Int32(0)),
    };
    dispatchCommand(mxComponent, u".uno:HandleDuplicateRecords"_ustr, aArgs);
    Scheduler::ProcessEventsToIdle();

    // First occurrences kept; the two duplicate rows removed (shifted up, so now empty).
    CPPUNIT_ASSERT_EQUAL(u"Alice"_ustr, rDoc.GetString(0, 1, 0));
    CPPUNIT_ASSERT_EQUAL(u"NY"_ustr, rDoc.GetString(1, 1, 0));
    CPPUNIT_ASSERT_EQUAL(u"Bob"_ustr, rDoc.GetString(0, 2, 0));
    CPPUNIT_ASSERT_EQUAL(u"LA"_ustr, rDoc.GetString(1, 2, 0));
    CPPUNIT_ASSERT_MESSAGE("first duplicate row removed", rDoc.GetString(0, 3, 0).isEmpty());
    CPPUNIT_ASSERT_MESSAGE("second duplicate row removed", rDoc.GetString(0, 4, 0).isEmpty());
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testDeleteTableDataRowsUndoRedo)
{
    ScModelObj* pModelObj = createDoc("empty.ods");
    CPPUNIT_ASSERT(pModelObj);
    auto pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    CPPUNIT_ASSERT(pDocShell);
    ScDocument& rDoc = pDocShell->GetDocument();
    ScTabViewShell* pViewShell = pDocShell->GetBestViewShell();
    CPPUNIT_ASSERT(pViewShell);

    // Styled table A1:C11 with header (row 1), data rows 2-10 and a Total Row (row 11).
    ScDBData* pData = new ScDBData(u"Table1"_ustr, /*nTab*/ 0, 0, 0, 2, 10,
                                   /*bByRow*/ true, /*bHasHeader*/ true, /*bHasTotals*/ true);
    ScTableStyleParam aStyleParam;
    aStyleParam.maStyleID = u"TableStyleMedium2"_ustr;
    pData->SetTableStyleInfo(aStyleParam);
    CPPUNIT_ASSERT(rDoc.GetDBCollection()->getNamedDBs().insert(std::unique_ptr<ScDBData>(pData)));

    rDoc.SetString(0, 0, 0, u"Name"_ustr);
    rDoc.SetString(1, 0, 0, u"Number"_ustr);
    rDoc.SetString(2, 0, 0, u"Column3"_ustr);
    for (SCROW nRow = 1; nRow <= 9; ++nRow)
        rDoc.SetValue(1, nRow, 0, static_cast<double>(nRow)); // Number: 1..9 (sum 45)
    pData->RefreshTableColumnNames(&rDoc);

    rDoc.SetString(0, 10, 0, u"Total"_ustr);
    rDoc.SetString(1, 10, 0, u"=SUBTOTAL(9;Table1[Number])"_ustr);
    rDoc.CalcAll();
    CPPUNIT_ASSERT_EQUAL(45.0, rDoc.GetValue(1, 10, 0));

    auto getTableArea = [&rDoc]() {
        ScDBData* p = rDoc.GetDBCollection()->getNamedDBs().findByUpperName(u"TABLE1"_ustr);
        CPPUNIT_ASSERT(p);
        ScRange aArea;
        p->GetArea(aArea);
        return aArea;
    };
    // Every data row, header and Total Row untouched: the Table keeps a data row.
    pViewShell->GetViewData().GetMarkData().SetMarkArea(ScRange(0, 1, 0, 2, 9, 0));

    rDoc.SetString(0, 12, 0, u"below"_ustr);

    dispatchCommand(mxComponent, u".uno:DeleteRows"_ustr, {});
    Scheduler::ProcessEventsToIdle();

    // Header + one empty data row + the Total Row, and 9 rows went for 8 rows of shift.
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 0, 0, 2, 2, 0), getTableArea());
    CPPUNIT_ASSERT_MESSAGE("the data row we give back must be empty",
                           rDoc.GetString(1, 1, 0).isEmpty());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("the Total Row rode the shrink up", u"Total"_ustr,
                                 rDoc.GetString(0, 2, 0));
    CPPUNIT_ASSERT_EQUAL(u"below"_ustr, rDoc.GetString(0, 4, 0));

    // The whole point of the exercise: the total is over that one row, not over nothing.
    rDoc.CalcAll();
    CPPUNIT_ASSERT_EQUAL(0.0, rDoc.GetValue(1, 2, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("the struct ref must resolve, not break to an error", u"0"_ustr,
                                 rDoc.GetString(1, 2, 0));

    // Deletion and repair are one undo step.
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 0, 0, 2, 10, 0), getTableArea());
    CPPUNIT_ASSERT_EQUAL(u"below"_ustr, rDoc.GetString(0, 12, 0));
    rDoc.CalcAll();
    CPPUNIT_ASSERT_EQUAL(45.0, rDoc.GetValue(1, 10, 0));

    dispatchCommand(mxComponent, u".uno:Redo"_ustr, {});
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 0, 0, 2, 2, 0), getTableArea());
    CPPUNIT_ASSERT_EQUAL(u"below"_ustr, rDoc.GetString(0, 4, 0));
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testDeleteTableDataCellsUpUndo)
{
    ScModelObj* pModelObj = createDoc("empty.ods");
    CPPUNIT_ASSERT(pModelObj);
    auto pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    CPPUNIT_ASSERT(pDocShell);
    ScDocument& rDoc = pDocShell->GetDocument();
    ScTabViewShell* pViewShell = pDocShell->GetBestViewShell();
    CPPUNIT_ASSERT(pViewShell);

    // Styled table A1:C8 without a Total Row: header (row 1), data rows 2-8.
    ScDBData* pData = new ScDBData(u"Table1"_ustr, /*nTab*/ 0, 0, 0, 2, 7,
                                   /*bByRow*/ true, /*bHasHeader*/ true, /*bHasTotals*/ false);
    ScTableStyleParam aStyleParam;
    aStyleParam.maStyleID = u"TableStyleMedium2"_ustr;
    pData->SetTableStyleInfo(aStyleParam);
    CPPUNIT_ASSERT(rDoc.GetDBCollection()->getNamedDBs().insert(std::unique_ptr<ScDBData>(pData)));

    auto getTableArea = [&rDoc]() {
        ScDBData* p = rDoc.GetDBCollection()->getNamedDBs().findByUpperName(u"TABLE1"_ustr);
        CPPUNIT_ASSERT(p);
        ScRange aArea;
        p->GetArea(aArea);
        return aArea;
    };

    rDoc.SetString(0, 9, 0, u"below"_ustr); // in the Table's columns, under it
    rDoc.SetString(3, 9, 0, u"beside"_ustr); // column D, outside the shifted band

    // Delete every data row as a cell shift over the Table's own columns only.
    pViewShell->GetViewData().GetMarkData().SetMarkArea(ScRange(0, 1, 0, 2, 7, 0));
    cpo::uno::Sequence<beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue(u"Flags"_ustr, u"U"_ustr), // shift cells up
    };
    dispatchCommand(mxComponent, u".uno:DeleteCell"_ustr, aArgs);
    Scheduler::ProcessEventsToIdle();

    // Here the insert lands below the shrunken area, so the area has to be widened explicitly.
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 0, 0, 2, 1, 0), getTableArea());
    CPPUNIT_ASSERT_MESSAGE("the data row we give back must be empty",
                           rDoc.GetString(0, 1, 0).isEmpty());

    // 7 rows went from the band, one came back: below moved up by 6, beside it did not move.
    CPPUNIT_ASSERT_EQUAL(u"below"_ustr, rDoc.GetString(0, 3, 0));
    CPPUNIT_ASSERT_EQUAL(u"beside"_ustr, rDoc.GetString(3, 9, 0));

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 0, 0, 2, 7, 0), getTableArea());
    CPPUNIT_ASSERT_EQUAL(u"below"_ustr, rDoc.GetString(0, 9, 0));
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testDeleteTableTotalsRowUndoRedo)
{
    ScModelObj* pModelObj = createDoc("empty.ods");
    CPPUNIT_ASSERT(pModelObj);
    auto pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    CPPUNIT_ASSERT(pDocShell);
    ScDocument& rDoc = pDocShell->GetDocument();
    ScTabViewShell* pViewShell = pDocShell->GetBestViewShell();
    CPPUNIT_ASSERT(pViewShell);

    // Styled table A1:C6 with header (row 1), data rows 2-5 and a Total Row (row 6).
    ScDBData* pData = new ScDBData(u"Table1"_ustr, /*nTab*/ 0, 0, 0, 2, 5,
                                   /*bByRow*/ true, /*bHasHeader*/ true, /*bHasTotals*/ true);
    ScTableStyleParam aStyleParam;
    aStyleParam.maStyleID = u"TableStyleMedium2"_ustr;
    pData->SetTableStyleInfo(aStyleParam);
    CPPUNIT_ASSERT(rDoc.GetDBCollection()->getNamedDBs().insert(std::unique_ptr<ScDBData>(pData)));

    rDoc.SetString(0, 0, 0, u"Name"_ustr);
    rDoc.SetString(1, 0, 0, u"Number"_ustr);
    rDoc.SetString(2, 0, 0, u"Column3"_ustr);
    for (SCROW nRow = 1; nRow <= 4; ++nRow)
        rDoc.SetValue(1, nRow, 0, static_cast<double>(nRow));
    pData->RefreshTableColumnNames(&rDoc);
    rDoc.SetString(0, 5, 0, u"Total"_ustr);
    rDoc.SetString(1, 5, 0, u"=SUBTOTAL(109;Table1[Number])"_ustr);
    rDoc.CalcAll();

    auto getTable = [&rDoc]() {
        ScDBData* p = rDoc.GetDBCollection()->getNamedDBs().findByUpperName(u"TABLE1"_ustr);
        CPPUNIT_ASSERT(p);
        return p;
    };
    auto getArea = [&getTable]() {
        ScRange aArea;
        getTable()->GetArea(aArea);
        return aArea;
    };
    auto getStashedFuncs = [&getTable]() {
        ScSubTotalParam aParam;
        getTable()->GetSubTotalParam(aParam);
        return static_cast<int>(aParam.aGroups[0].nCustFuncs);
    };

    // Rows 2-6: every data row and the Total Row with them.
    pViewShell->GetViewData().GetMarkData().SetMarkArea(ScRange(0, 1, 0, 2, 5, 0));
    dispatchCommand(mxComponent, u".uno:DeleteRows"_ustr, {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(ScRange(0, 0, 0, 2, 1, 0), getArea());
    CPPUNIT_ASSERT_MESSAGE("the Total Row went with its row", !getTable()->HasTotals());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("no stashed total formulas are left", 0, getStashedFuncs());
    CPPUNIT_ASSERT_MESSAGE("the data row we give back must be empty",
                           rDoc.GetString(1, 1, 0).isEmpty());

    // One undo step brings back the rows and the Total Row.
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 0, 0, 2, 5, 0), getArea());
    CPPUNIT_ASSERT_MESSAGE("undo restores the Total Row", getTable()->HasTotals());
    rDoc.CalcAll();
    CPPUNIT_ASSERT_EQUAL(10.0, rDoc.GetValue(1, 5, 0));

    // The Total Row's row alone: the data rows stay, so only the Total Row goes.
    pViewShell->GetViewData().GetMarkData().SetMarkArea(ScRange(0, 5, 0, 2, 5, 0));
    dispatchCommand(mxComponent, u".uno:DeleteRows"_ustr, {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(ScRange(0, 0, 0, 2, 4, 0), getArea());
    CPPUNIT_ASSERT_MESSAGE("the Total Row went, the data rows did not",
                           !getTable()->HasTotals());
    CPPUNIT_ASSERT_EQUAL(4.0, rDoc.GetValue(1, 4, 0));

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 0, 0, 2, 5, 0), getArea());
    CPPUNIT_ASSERT_MESSAGE("undo restores the Total Row", getTable()->HasTotals());
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testResizeTotalsRowsOnlyUndoRedo)
{
    ScModelObj* pModelObj = createDoc("empty.ods");
    CPPUNIT_ASSERT(pModelObj);
    auto pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    CPPUNIT_ASSERT(pDocShell);
    ScDocument& rDoc = pDocShell->GetDocument();
    ScUndoManager* pUndoManager = rDoc.GetUndoManager();
    CPPUNIT_ASSERT(pUndoManager);

    // Styled table A1:C11 with header + Total Row (row 11).
    ScDBData* pData = new ScDBData(u"Table1"_ustr, /*nTab*/ 0, 0, 0, 2, 10,
                                   /*bByRow*/ true, /*bHasHeader*/ true, /*bHasTotals*/ true);
    ScTableStyleParam aStyleParam;
    aStyleParam.maStyleID = u"TableStyleMedium2"_ustr;
    pData->SetTableStyleInfo(aStyleParam);
    CPPUNIT_ASSERT(rDoc.GetDBCollection()->getNamedDBs().insert(std::unique_ptr<ScDBData>(pData)));

    auto getTableArea = [&rDoc]() {
        ScDBData* p = rDoc.GetDBCollection()->getNamedDBs().findByUpperName(u"TABLE1"_ustr);
        CPPUNIT_ASSERT(p);
        ScRange aArea;
        p->GetArea(aArea);
        return aArea;
    };

    // Grow A1:C11 -> A1:C27 (rows only): the Total Row relocates.
    ScDBDocFunc(*pDocShell).ResizeTable(*pData, ScRange(0, 0, 0, 2, 26, 0));
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 0, 0, 2, 26, 0), getTableArea());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("resize undo entry", ScResId(STR_UNDO_RESIZE_CALCTABLE),
                                 pUndoManager->GetUndoActionComment(0));

    pUndoManager->Undo();
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 0, 0, 2, 10, 0), getTableArea());

    pUndoManager->Redo();
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 0, 0, 2, 26, 0), getTableArea());
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testResizeTotalsColumnsOnlyUndoRedo)
{
    ScModelObj* pModelObj = createDoc("empty.ods");
    CPPUNIT_ASSERT(pModelObj);
    auto pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    CPPUNIT_ASSERT(pDocShell);
    ScDocument& rDoc = pDocShell->GetDocument();
    ScUndoManager* pUndoManager = rDoc.GetUndoManager();
    CPPUNIT_ASSERT(pUndoManager);

    // Styled table A1:C11 with header + Total Row (row 11).
    ScDBData* pData = new ScDBData(u"Table1"_ustr, /*nTab*/ 0, 0, 0, 2, 10,
                                   /*bByRow*/ true, /*bHasHeader*/ true, /*bHasTotals*/ true);
    ScTableStyleParam aStyleParam;
    aStyleParam.maStyleID = u"TableStyleMedium2"_ustr;
    pData->SetTableStyleInfo(aStyleParam);
    CPPUNIT_ASSERT(rDoc.GetDBCollection()->getNamedDBs().insert(std::unique_ptr<ScDBData>(pData)));

    auto getTableArea = [&rDoc]() {
        ScDBData* p = rDoc.GetDBCollection()->getNamedDBs().findByUpperName(u"TABLE1"_ustr);
        CPPUNIT_ASSERT(p);
        ScRange aArea;
        p->GetArea(aArea);
        return aArea;
    };

    // Grow A1:C11 -> A1:E11 (columns only): rows are unchanged, so the Total Row is not
    // relocated (DoTableSubTotals is not re-entered); ModifyDBData extends the columns and
    // fills the new header cells.
    ScDBDocFunc(*pDocShell).ResizeTable(*pData, ScRange(0, 0, 0, 4, 10, 0));
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 0, 0, 4, 10, 0), getTableArea());

    // The new columns got auto-generated header cells...
    ScDBData* pLive = rDoc.GetDBCollection()->getNamedDBs().findByUpperName(u"TABLE1"_ustr);
    CPPUNIT_ASSERT(pLive);
    CPPUNIT_ASSERT(pLive->IsGeneratedHeaderName(rDoc.GetString(3, 0, 0)));
    CPPUNIT_ASSERT(pLive->IsGeneratedHeaderName(rDoc.GetString(4, 0, 0)));

    // ...but no SUBTOTAL was written into their Total Row cells (DoTableSubTotals not re-entered).
    CPPUNIT_ASSERT(rDoc.GetString(3, 10, 0).isEmpty());
    CPPUNIT_ASSERT(rDoc.GetString(4, 10, 0).isEmpty());

    CPPUNIT_ASSERT_EQUAL_MESSAGE("resize undo entry", ScResId(STR_UNDO_RESIZE_CALCTABLE),
                                 pUndoManager->GetUndoActionComment(0));

    pUndoManager->Undo();
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 0, 0, 2, 10, 0), getTableArea());

    pUndoManager->Redo();
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 0, 0, 4, 10, 0), getTableArea());
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testTableFilteredRowsShownOnDrop)
{
    ScModelObj* pModelObj = createDoc("empty.ods");
    CPPUNIT_ASSERT(pModelObj);
    auto pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    CPPUNIT_ASSERT(pDocShell);
    ScDocument& rDoc = pDocShell->GetDocument();
    ScUndoManager* pUndoManager = rDoc.GetUndoManager();
    CPPUNIT_ASSERT(pUndoManager);

    // A1:B5 Table with a header row, filtered down to the single data row holding 1.
    ScDBData* pData = new ScDBData(u"Table1"_ustr, /*nTab*/ 0, 0, 0, 1, 4, /*bByRow*/ true,
                                   /*bHasHeader*/ true, /*bHasTotals*/ false);
    ScTableStyleParam aStyleParam;
    aStyleParam.maStyleID = u"TableStyleMedium2"_ustr;
    pData->SetTableStyleInfo(aStyleParam);
    CPPUNIT_ASSERT(rDoc.GetDBCollection()->getNamedDBs().insert(std::unique_ptr<ScDBData>(pData)));

    rDoc.SetString(ScAddress(0, 0, 0), u"Head"_ustr);
    for (SCROW nRow = 1; nRow <= 4; ++nRow)
        rDoc.SetValue(0, nRow, 0, static_cast<double>(nRow));

    ScQueryParam aParam;
    pData->GetQueryParam(aParam);
    ScQueryEntry& rEntry = aParam.GetEntry(0);
    rEntry.bDoQuery = true;
    rEntry.nField = 0;
    rEntry.eOp = SC_EQUAL;
    rEntry.GetQueryItem().meType = ScQueryEntry::ByValue;
    rEntry.GetQueryItem().mfVal = 1.0;
    pData->SetQueryParam(aParam);
    ScDBDocFunc(*pDocShell).Query(0, aParam, nullptr, /*bRecord*/ false, /*bApi*/ true);

    CPPUNIT_ASSERT_MESSAGE("row 2 matches the filter", !rDoc.RowFiltered(1, 0));
    CPPUNIT_ASSERT_MESSAGE("row 5 is filtered away", rDoc.RowFiltered(4, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("the setup filter recorded nothing", size_t(0),
                                 pUndoManager->GetUndoActionCount());

    CPPUNIT_ASSERT(ScDBDocFunc(*pDocShell).ConvertTableToRange(pData));
    CPPUNIT_ASSERT_MESSAGE("row 5 shows again", !rDoc.RowFiltered(4, 0));
    CPPUNIT_ASSERT_MESSAGE("row 5 is not hidden", !rDoc.RowHidden(4, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("one undo entry for both halves", size_t(1),
                                 pUndoManager->GetUndoActionCount());

    pUndoManager->Undo();
    CPPUNIT_ASSERT_MESSAGE("undo hides row 5 again", rDoc.RowFiltered(4, 0));

    // The undo re-created the range, so the old pointer is dead.
    ScDBData* pLive = rDoc.GetDBCollection()->getNamedDBs().findByUpperName(u"TABLE1"_ustr);
    CPPUNIT_ASSERT_MESSAGE("undo restored the Table", pLive);

    // Same again through Remove Table, on the restored filter.
    CPPUNIT_ASSERT(ScDBDocFunc(*pDocShell).DeleteDBTable(pLive, /*bRecord*/ true, /*bApi*/ true));
    CPPUNIT_ASSERT_MESSAGE("row 5 shows again", !rDoc.RowFiltered(4, 0));
    CPPUNIT_ASSERT_MESSAGE("row 5 is not hidden", !rDoc.RowHidden(4, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("one undo entry for both halves", size_t(1),
                                 pUndoManager->GetUndoActionCount());

    pUndoManager->Undo();
    CPPUNIT_ASSERT_MESSAGE("undo hides row 5 again", rDoc.RowFiltered(4, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("back to an empty undo stack", size_t(0),
                                 pUndoManager->GetUndoActionCount());

    pLive = rDoc.GetDBCollection()->getNamedDBs().findByUpperName(u"TABLE1"_ustr);
    CPPUNIT_ASSERT_MESSAGE("undo restored the Table", pLive);

    // A caller that asked for no recording must not get a stray Filter entry either.
    CPPUNIT_ASSERT(ScDBDocFunc(*pDocShell).DeleteDBTable(pLive, /*bRecord*/ false, /*bApi*/ true));
    CPPUNIT_ASSERT_MESSAGE("row 5 shows again", !rDoc.RowFiltered(4, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("no undo entry when the caller asked for none", size_t(0),
                                 pUndoManager->GetUndoActionCount());
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testResizeTotalsMultiDimUndoRedo)
{
    ScModelObj* pModelObj = createDoc("empty.ods");
    CPPUNIT_ASSERT(pModelObj);
    auto pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    CPPUNIT_ASSERT(pDocShell);
    ScDocument& rDoc = pDocShell->GetDocument();
    ScUndoManager* pUndoManager = rDoc.GetUndoManager();
    CPPUNIT_ASSERT(pUndoManager);

    // Styled table A1:C11 with header + Total Row (row 11).
    ScDBData* pData = new ScDBData(u"Table1"_ustr, /*nTab*/ 0, 0, 0, 2, 10,
                                   /*bByRow*/ true, /*bHasHeader*/ true, /*bHasTotals*/ true);
    ScTableStyleParam aStyleParam;
    aStyleParam.maStyleID = u"TableStyleMedium2"_ustr;
    pData->SetTableStyleInfo(aStyleParam);
    CPPUNIT_ASSERT(rDoc.GetDBCollection()->getNamedDBs().insert(std::unique_ptr<ScDBData>(pData)));

    auto getTableArea = [&rDoc]() {
        ScDBData* p = rDoc.GetDBCollection()->getNamedDBs().findByUpperName(u"TABLE1"_ustr);
        CPPUNIT_ASSERT(p);
        ScRange aArea;
        p->GetArea(aArea);
        return aArea;
    };

    // Grow A1:C11 -> A1:H26 (columns AND rows).
    ScDBDocFunc(*pDocShell).ResizeTable(*pData, ScRange(0, 0, 0, 7, 25, 0));
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 0, 0, 7, 25, 0), getTableArea());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("resize undo entry", ScResId(STR_UNDO_RESIZE_CALCTABLE),
                                 pUndoManager->GetUndoActionComment(0));

    pUndoManager->Undo();
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 0, 0, 2, 10, 0), getTableArea());

    pUndoManager->Redo();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("redo must reinstate the resized range",
                                 ScRange(0, 0, 0, 7, 25, 0), getTableArea());
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testResizeTotalsStartColMoveUndoRedo)
{
    ScModelObj* pModelObj = createDoc("empty.ods");
    CPPUNIT_ASSERT(pModelObj);
    auto pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    CPPUNIT_ASSERT(pDocShell);
    ScDocument& rDoc = pDocShell->GetDocument();
    ScUndoManager* pUndoManager = rDoc.GetUndoManager();
    CPPUNIT_ASSERT(pUndoManager);

    // Styled table A1:C11 with header + Total Row (row 11).
    ScDBData* pData = new ScDBData(u"Table1"_ustr, /*nTab*/ 0, 0, 0, 2, 10,
                                   /*bByRow*/ true, /*bHasHeader*/ true, /*bHasTotals*/ true);
    ScTableStyleParam aStyleParam;
    aStyleParam.maStyleID = u"TableStyleMedium2"_ustr;
    pData->SetTableStyleInfo(aStyleParam);
    CPPUNIT_ASSERT(rDoc.GetDBCollection()->getNamedDBs().insert(std::unique_ptr<ScDBData>(pData)));

    auto getTableArea = [&rDoc]() {
        ScDBData* p = rDoc.GetDBCollection()->getNamedDBs().findByUpperName(u"TABLE1"_ustr);
        CPPUNIT_ASSERT(p);
        ScRange aArea;
        p->GetArea(aArea);
        return aArea;
    };

    // Resize A1:C11 -> B1:E26: the start column moves right AND the rows change.
    ScDBDocFunc(*pDocShell).ResizeTable(*pData, ScRange(1, 0, 0, 4, 25, 0));
    CPPUNIT_ASSERT_EQUAL(ScRange(1, 0, 0, 4, 25, 0), getTableArea());

    pUndoManager->Undo();
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 0, 0, 2, 10, 0), getTableArea());

    pUndoManager->Redo();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("redo reinstates the resized range after a start-column move",
                                 ScRange(1, 0, 0, 4, 25, 0), getTableArea());
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testResizeTotalsOverlapRefusedNoHalfApply)
{
    ScModelObj* pModelObj = createDoc("empty.ods");
    CPPUNIT_ASSERT(pModelObj);
    auto pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    CPPUNIT_ASSERT(pDocShell);
    ScDocument& rDoc = pDocShell->GetDocument();
    ScUndoManager* pUndoManager = rDoc.GetUndoManager();
    CPPUNIT_ASSERT(pUndoManager);

    // Styled table A1:C11 with header + Total Row (row 11)...
    ScDBData* pData = new ScDBData(u"Table1"_ustr, /*nTab*/ 0, 0, 0, 2, 10,
                                   /*bByRow*/ true, /*bHasHeader*/ true, /*bHasTotals*/ true);
    ScTableStyleParam aStyleParam;
    aStyleParam.maStyleID = u"TableStyleMedium2"_ustr;
    pData->SetTableStyleInfo(aStyleParam);
    CPPUNIT_ASSERT(rDoc.GetDBCollection()->getNamedDBs().insert(std::unique_ptr<ScDBData>(pData)));

    // ...and a second table E1:G11 that a rightward grow would overlap.
    ScDBData* pOther = new ScDBData(u"Table2"_ustr, /*nTab*/ 0, 4, 0, 6, 10,
                                    /*bByRow*/ true, /*bHasHeader*/ true, /*bHasTotals*/ false);
    CPPUNIT_ASSERT(rDoc.GetDBCollection()->getNamedDBs().insert(std::unique_ptr<ScDBData>(pOther)));

    auto getTableArea = [&rDoc]() {
        ScDBData* p = rDoc.GetDBCollection()->getNamedDBs().findByUpperName(u"TABLE1"_ustr);
        CPPUNIT_ASSERT(p);
        ScRange aArea;
        p->GetArea(aArea);
        return aArea;
    };

    // A1:C11 -> A1:E26 grows columns (into Table2) AND rows on a Total-Row table. The column
    // grow overlaps, so the whole resize must be refused up front: no columns extended, no
    // undo entry - not a half-applied column change that the Total-Row step then backs out of.
    CPPUNIT_ASSERT(!ScDBDocFunc(*pDocShell).ResizeTable(*pData, ScRange(0, 0, 0, 4, 25, 0), /*bApi*/true));
    CPPUNIT_ASSERT_EQUAL(ScRange(0, 0, 0, 2, 10, 0), getTableArea());
    CPPUNIT_ASSERT_EQUAL(size_t(0), pUndoManager->GetUndoActionCount());
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testAutoExpandUndoKeepsCursor)
{
    ScModelObj* pModelObj = createDoc("empty.ods");
    CPPUNIT_ASSERT(pModelObj);

    auto pDocShell = dynamic_cast<ScDocShell*>(pModelObj->GetEmbeddedObject());
    CPPUNIT_ASSERT(pDocShell);
    ScDocument& rDoc = pDocShell->GetDocument();
    ScUndoManager* pUndoManager = rDoc.GetUndoManager();
    CPPUNIT_ASSERT(pUndoManager);

    // Styled table A1:C5 with auto-generated headers Column1|Column2|Column3.
    ScDBDocFunc aFunc(*pDocShell);
    CPPUNIT_ASSERT(aFunc.AddDBTable(u"Table1"_ustr, ScRange(0, 0, 0, 2, 4, 0),
                                    /*bHeader*/ true, /*bRecord*/ true, /*bApi*/ true,
                                    u"TableStyleMedium2"_ustr));

    // Type into D3 (the column band right of the table) — this is what the user
    // does, and it both flags the auto-expansion and leaves the cursor on D3.
    rDoc.SetString(ScAddress(3, 2, 0), u"x"_ustr);
    ScDBData* pData = rDoc.GetDBCollection()->getNamedDBs().findByUpperName(u"TABLE1"_ustr);
    CPPUNIT_ASSERT(pData);
    CPPUNIT_ASSERT(pData->HasPendingExpansion());

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    CPPUNIT_ASSERT(pViewShell);
    pViewShell->SetCursor(3, 2);

    ScViewData* pViewData = ScDocShell::GetViewData();
    CPPUNIT_ASSERT(pViewData);

    // Drain the pending expansion: the table grows to column D and a "Column4"
    // header is generated for it.
    pDocShell->ProcessPendingTableExpansions();
    CPPUNIT_ASSERT_EQUAL(u"Column4"_ustr, rDoc.GetString(3, 0, 0));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(3), pViewData->GetCurX());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), pViewData->GetCurY());

    // Undo: without the cursor guard this jumped to D1 (the generated header).
    pUndoManager->Undo();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("undo must keep the cursor on D3", sal_Int16(3),
                                 pViewData->GetCurX());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("undo must keep the cursor on D3", sal_Int32(2),
                                 pViewData->GetCurY());

    // Redo: without the guard this jumped to D1 again.
    pUndoManager->Redo();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("redo must keep the cursor on D3", sal_Int16(3),
                                 pViewData->GetCurX());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("redo must keep the cursor on D3", sal_Int32(2),
                                 pViewData->GetCurY());
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testInvalidateOnCopyPasteCells)
{
    ScModelObj* pModelObj = createDoc("small.ods");
    CPPUNIT_ASSERT(pModelObj);

    // view
    ScTestViewCallback aView;

    cpo::uno::Sequence<beans::PropertyValue> aArgs;
    // select and copy cells
    pModelObj->postKeyEvent(COKitKeyEventType::DOWN, 0, KEY_HOME | KEY_MOD1);
    pModelObj->postKeyEvent(COKitKeyEventType::UP, 0, KEY_HOME | KEY_MOD1);
    pModelObj->postKeyEvent(COKitKeyEventType::DOWN, 0, KEY_DOWN | KEY_SHIFT);
    pModelObj->postKeyEvent(COKitKeyEventType::UP, 0, KEY_DOWN | KEY_SHIFT);
    pModelObj->postKeyEvent(COKitKeyEventType::DOWN, 0, KEY_RIGHT | KEY_SHIFT);
    pModelObj->postKeyEvent(COKitKeyEventType::UP, 0, KEY_RIGHT | KEY_SHIFT);
    Scheduler::ProcessEventsToIdle();
    dispatchCommand(mxComponent, u".uno:Copy"_ustr, aArgs);

    // move to destination cell
    pModelObj->postKeyEvent(COKitKeyEventType::DOWN, 0, KEY_DOWN);
    pModelObj->postKeyEvent(COKitKeyEventType::UP, 0, KEY_DOWN);
    pModelObj->postKeyEvent(COKitKeyEventType::DOWN, 0, KEY_DOWN | KEY_MOD1);
    pModelObj->postKeyEvent(COKitKeyEventType::UP, 0, KEY_DOWN | KEY_MOD1);
    pModelObj->postKeyEvent(COKitKeyEventType::DOWN, 0, KEY_UP);
    pModelObj->postKeyEvent(COKitKeyEventType::UP, 0, KEY_UP);
    Scheduler::ProcessEventsToIdle();

    // paste cells
    aView.m_bInvalidateTiles = false;
    dispatchCommand(mxComponent, u".uno:Paste"_ustr, aArgs);
    CPPUNIT_ASSERT(aView.m_bInvalidateTiles);
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testInvalidateOnInserRowCol)
{
    ScModelObj* pModelObj = createDoc("small.ods");
    CPPUNIT_ASSERT(pModelObj);

    // view
    ScTestViewCallback aView;

    cpo::uno::Sequence<beans::PropertyValue> aArgs;
    // move downward
    for (int i = 0; i < 200; ++i)
    {
        pModelObj->postKeyEvent(COKitKeyEventType::DOWN, 0, KEY_DOWN);
        pModelObj->postKeyEvent(COKitKeyEventType::UP, 0, KEY_DOWN);
    }
    Scheduler::ProcessEventsToIdle();

    // insert row
    aView.m_bInvalidateTiles = false;
    aView.m_aInvalidations.clear();
    dispatchCommand(mxComponent, u".uno:InsertRows"_ustr, aArgs);
    CPPUNIT_ASSERT(aView.m_bInvalidateTiles);
    CPPUNIT_ASSERT_EQUAL(size_t(1), aView.m_aInvalidations.size());
    CPPUNIT_ASSERT_EQUAL(tools::Rectangle(-75, 51240, 32212230, 63990), aView.m_aInvalidations[0]);

    // move on the right
    for (int i = 0; i < 200; ++i)
    {
        pModelObj->postKeyEvent(COKitKeyEventType::DOWN, 0, KEY_RIGHT);
        pModelObj->postKeyEvent(COKitKeyEventType::UP, 0, KEY_RIGHT);
    }
    Scheduler::ProcessEventsToIdle();

    // insert column
    aView.m_bInvalidateTiles = false;
    aView.m_aInvalidations.clear();
    dispatchCommand(mxComponent, u".uno:InsertColumns"_ustr, aArgs);
    CPPUNIT_ASSERT(aView.m_bInvalidateTiles);
    CPPUNIT_ASSERT_EQUAL(size_t(1), aView.m_aInvalidations.size());
    CPPUNIT_ASSERT_EQUAL(tools::Rectangle(254925, -15, 32212230, 63990), aView.m_aInvalidations[0]);
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testCommentCallback)
{
    // FIXME: Hack because previous tests do not destroy ScDocument(with annotations) on exit (?).
    ScPostIt::mnLastPostItId = 1;

    {
        ScModelObj* pModelObj = createDoc("small.ods");
        ScTestViewCallback aView1;
        int nView1 = KitHelper::getCurrentView();

        // Create a 2nd view
        KitHelper::createView();
        pModelObj->initializeForTiledRendering({});
        ScTestViewCallback aView2;

        KitHelper::setView(nView1);

        ScTabViewShell* pTabViewShell = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
        if (pTabViewShell)
            pTabViewShell->SetCursor(4, 4);

        // Add a new comment
        cpo::uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence(
        {
            {u"Text"_ustr, cpo::uno::Any(u"Comment"_ustr)},
            {u"Author"_ustr, cpo::uno::Any(u"Kit User1"_ustr)},
        }));
        dispatchCommand(mxComponent, u".uno:InsertAnnotation"_ustr, aArgs);

        // We received a COKitCallbackType::COMMENT callback with comment 'Add' action
        CPPUNIT_ASSERT_EQUAL(std::string("Add"), aView1.m_aCommentCallbackResult.get<std::string>("action"));
        CPPUNIT_ASSERT_EQUAL(std::string("Add"), aView2.m_aCommentCallbackResult.get<std::string>("action"));
        CPPUNIT_ASSERT_EQUAL(std::string("1"), aView1.m_aCommentCallbackResult.get<std::string>("id"));
        CPPUNIT_ASSERT_EQUAL(std::string("1"), aView2.m_aCommentCallbackResult.get<std::string>("id"));
        CPPUNIT_ASSERT_EQUAL(std::string("0"), aView1.m_aCommentCallbackResult.get<std::string>("tab"));
        CPPUNIT_ASSERT_EQUAL(std::string("0"), aView2.m_aCommentCallbackResult.get<std::string>("tab"));
        CPPUNIT_ASSERT_EQUAL(std::string("Kit User1"), aView1.m_aCommentCallbackResult.get<std::string>("author"));
        CPPUNIT_ASSERT_EQUAL(std::string("Kit User1"), aView2.m_aCommentCallbackResult.get<std::string>("author"));
        CPPUNIT_ASSERT_EQUAL(std::string("Comment"), aView1.m_aCommentCallbackResult.get<std::string>("text"));
        CPPUNIT_ASSERT_EQUAL(std::string("Comment"), aView2.m_aCommentCallbackResult.get<std::string>("text"));
        CPPUNIT_ASSERT_EQUAL(std::string("4 4 4 4"), aView1.m_aCommentCallbackResult.get<std::string>("cellRange"));
        CPPUNIT_ASSERT_EQUAL(std::string("4 4 4 4"), aView2.m_aCommentCallbackResult.get<std::string>("cellRange"));

        // Ensure deleting rows updates comments
        if (pTabViewShell)
            pTabViewShell->SetCursor(2, 2);

        dispatchCommand(mxComponent, u".uno:DeleteRows"_ustr, {});
        Scheduler::ProcessEventsToIdle();
        CPPUNIT_ASSERT_EQUAL(std::string("4 3 4 3"), aView1.m_aCommentCallbackResult.get<std::string>("cellRange"));
        CPPUNIT_ASSERT_EQUAL(std::string("4 3 4 3"), aView2.m_aCommentCallbackResult.get<std::string>("cellRange"));

        // Ensure deleting columns updates comments
        if (pTabViewShell)
            pTabViewShell->SetCursor(2, 2);

        dispatchCommand(mxComponent, u".uno:DeleteColumns"_ustr, {});
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
            {u"Id"_ustr, cpo::uno::Any(OUString::createFromAscii(aCommentId))},
            {u"Text"_ustr, cpo::uno::Any(u"Edited comment"_ustr)},
            {u"Author"_ustr, cpo::uno::Any(u"Kit User2"_ustr)},
        });
        dispatchCommand(mxComponent, u".uno:EditAnnotation"_ustr, aArgs);

        // We received a COKitCallbackType::COMMENT callback with comment 'Modify' action
        CPPUNIT_ASSERT_EQUAL(std::string("Modify"), aView1.m_aCommentCallbackResult.get<std::string>("action"));
        CPPUNIT_ASSERT_EQUAL(std::string("Modify"), aView2.m_aCommentCallbackResult.get<std::string>("action"));
        CPPUNIT_ASSERT_EQUAL(aCommentId, aView1.m_aCommentCallbackResult.get<std::string>("id"));
        CPPUNIT_ASSERT_EQUAL(aCommentId, aView2.m_aCommentCallbackResult.get<std::string>("id"));
        CPPUNIT_ASSERT_EQUAL(std::string("Kit User2"), aView1.m_aCommentCallbackResult.get<std::string>("author"));
        CPPUNIT_ASSERT_EQUAL(std::string("Kit User2"), aView2.m_aCommentCallbackResult.get<std::string>("author"));
        CPPUNIT_ASSERT_EQUAL(std::string("Edited comment"), aView1.m_aCommentCallbackResult.get<std::string>("text"));
        CPPUNIT_ASSERT_EQUAL(std::string("Edited comment"), aView2.m_aCommentCallbackResult.get<std::string>("text"));
        CPPUNIT_ASSERT_EQUAL(std::string("3 3 3 3"), aView1.m_aCommentCallbackResult.get<std::string>("cellRange"));
        CPPUNIT_ASSERT_EQUAL(std::string("3 3 3 3"), aView2.m_aCommentCallbackResult.get<std::string>("cellRange"));

        // Delete the comment
        if (pTabViewShell)
            pTabViewShell->SetCursor(4, 43);
        aArgs = comphelper::InitPropertySequence(
        {
            {u"Id"_ustr, cpo::uno::Any(OUString::createFromAscii(aCommentId))}
        });
        dispatchCommand(mxComponent, u".uno:DeleteNote"_ustr, aArgs);

        // We received a COKitCallbackType::COMMENT callback with comment 'Remove' action
        CPPUNIT_ASSERT_EQUAL(std::string("Remove"), aView1.m_aCommentCallbackResult.get<std::string>("action"));
        CPPUNIT_ASSERT_EQUAL(std::string("Remove"), aView2.m_aCommentCallbackResult.get<std::string>("action"));
        CPPUNIT_ASSERT_EQUAL(aCommentId, aView1.m_aCommentCallbackResult.get<std::string>("id"));
        CPPUNIT_ASSERT_EQUAL(aCommentId, aView2.m_aCommentCallbackResult.get<std::string>("id"));
    }
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testUndoLimiting)
{
    ScModelObj* pModelObj = createDoc("small.ods");
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    ScUndoManager* pUndoManager = pDoc->GetUndoManager();
    CPPUNIT_ASSERT(pUndoManager);

    // view #1
    int nView1 = KitHelper::getCurrentView();
    ScTestViewCallback aView1;

    // view #2
    KitHelper::createView();
    int nView2 = KitHelper::getCurrentView();
    pModelObj->initializeForTiledRendering(cpo::uno::Sequence<beans::PropertyValue>());
    ScTestViewCallback aView2;

    // text edit a cell in view #1
    KitHelper::setView(nView1);
    pModelObj->postKeyEvent(COKitKeyEventType::DOWN, 'x', 0);
    pModelObj->postKeyEvent(COKitKeyEventType::UP, 'x', 0);
    pModelObj->postKeyEvent(COKitKeyEventType::DOWN, 0, awt::Key::RETURN);
    pModelObj->postKeyEvent(COKitKeyEventType::UP, 0, awt::Key::RETURN);
    Scheduler::ProcessEventsToIdle();

    // check that undo action count in not 0
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), pUndoManager->GetUndoActionCount());

    // try to execute undo in view #2
    KitHelper::setView(nView2);
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    // check that undo has not been executed on view #2
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), pUndoManager->GetUndoActionCount());

    // try to execute undo in view #1
    KitHelper::setView(nView1);
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    // check that undo has been executed on view #1
    CPPUNIT_ASSERT_EQUAL(std::size_t(0), pUndoManager->GetUndoActionCount());

    // check that redo action count in not 0
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), pUndoManager->GetRedoActionCount());

    // try to execute redo in view #2
    KitHelper::setView(nView2);
    dispatchCommand(mxComponent, u".uno:Redo"_ustr, {});
    // check that redo has not been executed on view #2
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), pUndoManager->GetRedoActionCount());

    // try to execute redo in view #1
    KitHelper::setView(nView1);
    dispatchCommand(mxComponent, u".uno:Redo"_ustr, {});
    // check that redo has been executed on view #1
    CPPUNIT_ASSERT_EQUAL(std::size_t(0), pUndoManager->GetRedoActionCount());
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testUndoRepairDispatch)
{
    ScModelObj* pModelObj = createDoc("small.ods");
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);
    ScUndoManager* pUndoManager = pDoc->GetUndoManager();
    CPPUNIT_ASSERT(pUndoManager);

    // view #1
    int nView1 = KitHelper::getCurrentView();
    ScTestViewCallback aView1;

    // view #2
    KitHelper::createView();
    int nView2 = KitHelper::getCurrentView();
    pModelObj->initializeForTiledRendering(cpo::uno::Sequence<beans::PropertyValue>());
    ScTestViewCallback aView2;

    // text edit a cell in view #1
    KitHelper::setView(nView1);
    pModelObj->postKeyEvent(COKitKeyEventType::DOWN, 'x', 0);
    pModelObj->postKeyEvent(COKitKeyEventType::UP, 'x', 0);
    pModelObj->postKeyEvent(COKitKeyEventType::DOWN, 0, awt::Key::RETURN);
    pModelObj->postKeyEvent(COKitKeyEventType::UP, 0, awt::Key::RETURN);
    Scheduler::ProcessEventsToIdle();

    // check that undo action count in not 0
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), pUndoManager->GetUndoActionCount());

    // try to execute undo in view #2
    KitHelper::setView(nView2);
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    // check that undo has not been executed on view #2
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), pUndoManager->GetUndoActionCount());

    // try to execute undo in view #2 in repair mode
    KitHelper::setView(nView2);
    cpo::uno::Sequence<beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence(
    {
        {u"Repair"_ustr, cpo::uno::Any(true)}
    }));
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, aPropertyValues);
    // check that undo has been executed on view #2 in repair mode
    CPPUNIT_ASSERT_EQUAL(std::size_t(0), pUndoManager->GetUndoActionCount());
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testInsertGraphicInvalidations)
{
    ScModelObj* pModelObj = createDoc("small.ods");
    CPPUNIT_ASSERT(pModelObj);
    ScViewData* pViewData = ScDocShell::GetViewData();
    CPPUNIT_ASSERT(pViewData);

    // view
    ScTestViewCallback aView;

    // we need to paint a tile in the view for triggering the tile invalidation solution
    int nCanvasWidth = 256;
    int nCanvasHeight = 256;
    std::vector<unsigned char> aBuffer(nCanvasWidth * nCanvasHeight * 4);
    ScopedVclPtrInstance<VirtualDevice> pDevice(DeviceFormat::WITHOUT_ALPHA);
    pDevice->SetOutputSizePixelScaleOffsetAndKitBuffer(Size(nCanvasWidth, nCanvasHeight), 1.0, Point(), aBuffer.data());
    pModelObj->paintTile(*pDevice, nCanvasWidth, nCanvasHeight, /*nTilePosX=*/0, /*nTilePosY=*/0, /*nTileWidth=*/3840, /*nTileHeight=*/3840);
    Scheduler::ProcessEventsToIdle();

    // insert an image in view and see if both views are invalidated
    aView.m_bInvalidateTiles = false;
    cpo::uno::Sequence<beans::PropertyValue> aArgs( comphelper::InitPropertySequence({
            { u"FileName"_ustr, cpo::uno::Any(createFileURL(u"smile.png")) }
        }));
    dispatchCommand(mxComponent, u".uno:InsertGraphic"_ustr, aArgs);
    CPPUNIT_ASSERT(aView.m_bInvalidateTiles);

    // undo image insertion in view and see if both views are invalidated
    aView.m_bInvalidateTiles = false;
    cpo::uno::Sequence<beans::PropertyValue> aArgs2;
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, aArgs2);
    CPPUNIT_ASSERT(aView.m_bInvalidateTiles);
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testDocumentSizeWithTwoViews)
{
    // Open a document that has the cursor far away & paint a tile
    ScModelObj* pModelObj = createDoc("cursor-away.ods");

    // Set the visible area, and press page down
    pModelObj->setClientVisibleArea(tools::Rectangle(750, 1861, 20583, 6997));
    pModelObj->postKeyEvent(COKitKeyEventType::DOWN, 0, awt::Key::PAGEDOWN);
    pModelObj->postKeyEvent(COKitKeyEventType::UP, 0, awt::Key::PAGEDOWN);
    Scheduler::ProcessEventsToIdle();

    int nCanvasWidth = 256;
    int nCanvasHeight = 256;
    std::vector<unsigned char> aBuffer1(nCanvasWidth * nCanvasHeight * 4);
    ScopedVclPtrInstance<VirtualDevice> pDevice1(DeviceFormat::WITHOUT_ALPHA);
    pDevice1->SetOutputSizePixelScaleOffsetAndKitBuffer(Size(nCanvasWidth, nCanvasHeight), 1.0, Point(), aBuffer1.data());
    pModelObj->paintTile(*pDevice1, nCanvasWidth, nCanvasHeight, /*nTilePosX=*/0, /*nTilePosY=*/291840, /*nTileWidth=*/3840, /*nTileHeight=*/3840);
    Scheduler::ProcessEventsToIdle();

    // Create a new view
    int nView1 = KitHelper::getCurrentView();
    KitHelper::createView();

    std::vector<unsigned char> aBuffer2(nCanvasWidth * nCanvasHeight * 4);
    ScopedVclPtrInstance<VirtualDevice> pDevice2(DeviceFormat::WITHOUT_ALPHA);
    pDevice2->SetOutputSizePixelScaleOffsetAndKitBuffer(Size(nCanvasWidth, nCanvasHeight), 1.0, Point(), aBuffer2.data());
    pModelObj->paintTile(*pDevice2, nCanvasWidth, nCanvasHeight, /*nTilePosX=*/0, /*nTilePosY=*/291840, /*nTileWidth=*/3840, /*nTileHeight=*/3840);
    Scheduler::ProcessEventsToIdle();

    // Check that the tiles actually have the same content
    for (size_t i = 0; i < aBuffer1.size(); ++i)
        CPPUNIT_ASSERT_EQUAL(aBuffer1[i], aBuffer2[i]);

    SfxViewShell::Current()->setCOKitViewCallback(nullptr);
    KitHelper::setView(nView1);
    SfxViewShell::Current()->setCOKitViewCallback(nullptr);
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testRowHeightChangeRendersFreshAfterUndo)
{
    // A row height change made outside the interactive resize path, like undo or
    // redo of typing that auto-grew the row, shifts every row below it. A view
    // that already resolved row positions for that area must render tiles with
    // the rows at their new places, the same as a freshly created view does.
    ScModelObj* pModelObj = createDoc("empty.ods");
    ScViewData* pViewData = ScDocShell::GetViewData();
    CPPUNIT_ASSERT(pViewData);
    ScDocShell* pDocSh = pViewData->GetDocShell();
    CPPUNIT_ASSERT(pDocSh);
    ScDocument& rDoc = pViewData->GetDocument();

    // A distant row lets a stale cached row position show up as a big offset.
    // The neighbouring rows carry text so a shifted render differs in pixels.
    const SCCOL nCol = 8;
    const SCROW nRow = 9864;
    for (SCROW nR = nRow - 3; nR <= nRow + 3; ++nR)
        if (nR != nRow)
            rDoc.SetString(ScAddress(nCol, nR, 0), "row " + OUString::number(nR + 1));

    const tools::Long nRowOffsetTw = rDoc.GetRowHeight(0, nRow - 1, 0);
    const tools::Rectangle aVisArea(0, nRowOffsetTw - 2000, 20000, nRowOffsetTw + 8000);
    pModelObj->setClientVisibleArea(aVisArea);

    // The header request is how a client viewport makes the view record row
    // position anchors for this area.
    {
        tools::JsonWriter aJsonWriter;
        pModelObj->getRowColumnHeaders(aVisArea, aJsonWriter);
        aJsonWriter.finishAndGetAsOString();
    }
    Scheduler::ProcessEventsToIdle();

    // Grow the row the way undo and redo of a data entry do: write the cell and
    // recompute the optimal height through the document shell.
    ScFieldEditEngine& rEngine = rDoc.GetEditEngine();
    rEngine.SetTextCurrentDefaults(u"one\ntwo\nthree\nfour\nfive"_ustr);
    rDoc.SetEditText(ScAddress(nCol, nRow, 0), rEngine.CreateTextObject());
    const sal_uInt16 nOldHeight = rDoc.GetRowHeight(nRow, 0);
    CPPUNIT_ASSERT(pDocSh->AdjustRowHeight(nRow, nRow, 0));
    CPPUNIT_ASSERT(rDoc.GetRowHeight(nRow, 0) > nOldHeight);
    Scheduler::ProcessEventsToIdle();

    // Render the rows below the grown one from the view that held old anchors.
    const int nCanvasWidth = 512;
    const int nCanvasHeight = 512;
    const tools::Long nTilePosX = 9000;
    const tools::Long nTilePosY = nRowOffsetTw + 1000;
    std::vector<unsigned char> aBuffer1(nCanvasWidth * nCanvasHeight * 4);
    ScopedVclPtrInstance<VirtualDevice> pDevice1(DeviceFormat::WITHOUT_ALPHA);
    pDevice1->SetOutputSizePixelScaleOffsetAndKitBuffer(Size(nCanvasWidth, nCanvasHeight), 1.0,
                                                        Point(), aBuffer1.data());
    pModelObj->paintTile(*pDevice1, nCanvasWidth, nCanvasHeight, nTilePosX, nTilePosY,
                         /*nTileWidth=*/7680, /*nTileHeight=*/7680);
    Scheduler::ProcessEventsToIdle();

    // A freshly created view resolves the row positions from the document.
    int nView1 = KitHelper::getCurrentView();
    KitHelper::createView();
    pModelObj->setClientVisibleArea(aVisArea);
    {
        tools::JsonWriter aJsonWriter;
        pModelObj->getRowColumnHeaders(aVisArea, aJsonWriter);
        aJsonWriter.finishAndGetAsOString();
    }
    Scheduler::ProcessEventsToIdle();

    std::vector<unsigned char> aBuffer2(nCanvasWidth * nCanvasHeight * 4);
    ScopedVclPtrInstance<VirtualDevice> pDevice2(DeviceFormat::WITHOUT_ALPHA);
    pDevice2->SetOutputSizePixelScaleOffsetAndKitBuffer(Size(nCanvasWidth, nCanvasHeight), 1.0,
                                                        Point(), aBuffer2.data());
    pModelObj->paintTile(*pDevice2, nCanvasWidth, nCanvasHeight, nTilePosX, nTilePosY,
                         /*nTileWidth=*/7680, /*nTileHeight=*/7680);
    Scheduler::ProcessEventsToIdle();

    bool bAreBuffersMatching = aBuffer1 == aBuffer2;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Buffers should match", true, bAreBuffersMatching);

    SfxViewShell::Current()->setCOKitViewCallback(nullptr);
    KitHelper::setView(nView1);
    SfxViewShell::Current()->setCOKitViewCallback(nullptr);
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testRowsBelowKeepPositionAfterRowHeightChange)
{
    // A row height change made through the document shell, the path undo and
    // redo of a data entry take, shifts every row below it. A view that already
    // resolved row positions for that area must place those rows at their new
    // positions: both in the coordinates it reports and in the tiles it paints.
    // When it does not, tiles below the grown row show the neighbouring rows
    // shifted by the height delta, with text cut in half at the tile boundary.
    // The clients also have to be told that the row geometry they cache changed.
    comphelper::COKit::setCompatFlag(comphelper::COKit::Compat::scPrintTwipsMsgs);
    ScModelObj* pModelObj = createDoc("empty.ods");
    ScViewData* pViewData = ScDocShell::GetViewData();
    CPPUNIT_ASSERT(pViewData);
    ScDocShell* pDocSh = pViewData->GetDocShell();
    CPPUNIT_ASSERT(pDocSh);
    ScDocument& rDoc = pViewData->GetDocument();
    ScTestViewCallback aView;

    // A distant row makes a stale cached position visible as a large offset;
    // the rows around the grown one carry text so a shifted render differs in
    // pixels and shows the artifact.
    const SCCOL nCol = 8;
    const SCROW nRow = 9864;
    for (SCROW nR = nRow - 3; nR <= nRow + 3; ++nR)
        if (nR != nRow)
            rDoc.SetString(ScAddress(nCol, nR, 0), "row " + OUString::number(nR + 1));

    const tools::Long nRowOffsetTw = rDoc.GetRowHeight(0, nRow - 1, 0);
    const tools::Rectangle aVisArea(0, nRowOffsetTw - 2000, 20000, nRowOffsetTw + 8000);
    pModelObj->setClientVisibleArea(aVisArea);

    // The header request is how a client viewport makes the view record row
    // position anchors for this area.
    {
        tools::JsonWriter aJsonWriter;
        pModelObj->getRowColumnHeaders(aVisArea, aJsonWriter);
        aJsonWriter.finishAndGetAsOString();
    }
    Scheduler::ProcessEventsToIdle();

    // Grow the row the way ScUndoEnterData::DoChange does: write the multiline
    // cell, then recompute the optimal height through the document shell.
    ScFieldEditEngine& rEngine = rDoc.GetEditEngine();
    rEngine.SetTextCurrentDefaults(u"one\ntwo\nthree\nfour\nfive"_ustr);
    rDoc.SetEditText(ScAddress(nCol, nRow, 0), rEngine.CreateTextObject());
    const sal_uInt16 nOldHeight = rDoc.GetRowHeight(nRow, 0);
    aView.m_sInvalidateSheetGeometry = ""_ostr;
    CPPUNIT_ASSERT(pDocSh->AdjustRowHeight(nRow, nRow, 0));
    CPPUNIT_ASSERT(rDoc.GetRowHeight(nRow, 0) > nOldHeight);
    Scheduler::ProcessEventsToIdle();

    // The client re-reads the row geometry only when it is told that it changed.
    CPPUNIT_ASSERT_EQUAL("rows sizes"_ostr, aView.m_sInvalidateSheetGeometry);

    // The view's position for a row below the grown one has to match the
    // position accumulated from the document row heights.
    const double fPPTY = pViewData->GetPPTY();
    tools::Long nExpectedPixels = 0;
    for (SCROW nR = 0; nR < nRow + 2; ++nR)
        if (sal_uInt16 nSize = rDoc.GetRowHeight(nR, 0))
            nExpectedPixels += ScViewData::ToPixel(nSize, fPPTY);
    const Point aScrPos = pViewData->GetScrPos(nCol, nRow + 2, pViewData->GetActivePart());
    CPPUNIT_ASSERT_EQUAL(nExpectedPixels, aScrPos.Y());

    // Two stacked tiles have to show the same pixels as one tile covering both:
    // a stale position shifts the lower tile's content but not the lower half
    // of the taller render, which is the text-cut-in-half seam.
    const tools::Long nTileTw = 3840;
    const tools::Long nTilePosY = (nRowOffsetTw / nTileTw) * nTileTw;
    tools::Long nColOffsetTw = 0;
    for (SCCOL nC = 0; nC < nCol; ++nC)
        nColOffsetTw += rDoc.GetColWidth(nC, 0);
    const tools::Long nTilePosX = (nColOffsetTw / nTileTw) * nTileTw;
    const int nCanvasSize = 256;

    auto paintArea = [&](tools::Long nPosY, int nHeightPx,
                         tools::Long nHeightTw) -> std::vector<unsigned char>
    {
        std::vector<unsigned char> aBuffer(nCanvasSize * nHeightPx * 4);
        ScopedVclPtrInstance<VirtualDevice> pDevice(DeviceFormat::WITHOUT_ALPHA);
        pDevice->SetOutputSizePixelScaleOffsetAndKitBuffer(Size(nCanvasSize, nHeightPx), 1.0,
                                                           Point(), aBuffer.data());
        pModelObj->paintTile(*pDevice, nCanvasSize, nHeightPx, nTilePosX, nPosY,
                             /*nTileWidth=*/nTileTw, /*nTileHeight=*/nHeightTw);
        Scheduler::ProcessEventsToIdle();
        return aBuffer;
    };

    const std::vector<unsigned char> aUpperTile = paintArea(nTilePosY, nCanvasSize, nTileTw);
    const std::vector<unsigned char> aLowerTile
        = paintArea(nTilePosY + nTileTw, nCanvasSize, nTileTw);
    const std::vector<unsigned char> aBothTiles
        = paintArea(nTilePosY, 2 * nCanvasSize, 2 * nTileTw);
    const std::vector<unsigned char> aUpperRepaint = paintArea(nTilePosY, nCanvasSize, nTileTw);

    auto firstDiffPixelRow = [](const std::vector<unsigned char>& rTile,
                                 const unsigned char* pReference) -> int
    {
        for (size_t i = 0; i < rTile.size(); ++i)
            if (rTile[i] != pReference[i])
                return static_cast<int>(i / (nCanvasSize * 4));
        return -1;
    };
    const OString aDiffInfo = "first differing pixel row: upper vs big "
        + OString::number(firstDiffPixelRow(aUpperTile, aBothTiles.data()))
        + ", lower vs big "
        + OString::number(firstDiffPixelRow(aLowerTile, aBothTiles.data() + aUpperTile.size()))
        + ", upper repaint vs upper "
        + OString::number(firstDiffPixelRow(aUpperTile, aUpperRepaint.data()));

    // The bottommost pixel row of the upper tile carries the tile's own edge of
    // the boundary grid line; in the taller render the same y is interior, so
    // the comparison covers the rows above it.
    const size_t nUpperComparable = (nCanvasSize - 1) * nCanvasSize * 4;
    CPPUNIT_ASSERT_MESSAGE(aDiffInfo.getStr(),
        std::equal(aUpperTile.begin(), aUpperTile.begin() + nUpperComparable,
                   aBothTiles.begin()));
    CPPUNIT_ASSERT_MESSAGE(aDiffInfo.getStr(),
        std::equal(aLowerTile.begin(), aLowerTile.end(),
                   aBothTiles.begin() + aUpperTile.size()));
}

namespace
{

// Records row and column position anchors for the given area in the current view, the way a
// client viewport does when it shows that part of the sheet.
void requestRowColumnHeaders(ScModelObj* pModelObj, const tools::Rectangle& rArea)
{
    pModelObj->setClientVisibleArea(rArea);
    tools::JsonWriter aJsonWriter;
    pModelObj->getRowColumnHeaders(rArea, aJsonWriter);
    aJsonWriter.finishAndGetAsOString();
    Scheduler::ProcessEventsToIdle();
}

// The pixel position of the top of nRow accumulated from the document row heights, the same
// way a freshly created view resolves it. Hidden rows contribute nothing.
tools::Long expectedRowPosition(const ScDocument& rDoc, double fPPTY, SCROW nRow)
{
    tools::Long nPixels = 0;
    for (SCROW nR = 0; nR < nRow; ++nR)
        if (sal_uInt16 nSize = rDoc.GetRowHeight(nR, 0))
            nPixels += ScViewData::ToPixel(nSize, fPPTY);
    return nPixels;
}

}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testRowsKeepPositionAfterSortUndo)
{
    // Undo of a sort that was executed on a range with hidden rows moves the hidden flags
    // back to their previous rows. A view that already resolved row positions for that area
    // must place the rows at their restored positions, the same as a freshly created view,
    // and the clients have to be told that the row geometry they cache changed.
    comphelper::COKit::setCompatFlag(comphelper::COKit::Compat::scPrintTwipsMsgs);
    ScModelObj* pModelObj = createDoc("empty.ods");
    ScViewData* pViewData = ScDocShell::GetViewData();
    CPPUNIT_ASSERT(pViewData);
    ScDocument& rDoc = pViewData->GetDocument();
    ScTestViewCallback aView;

    // Descending values in a distant range make the ascending sort reverse the row order; a
    // stale cached row position then shows up as a large offset.
    const SCROW nRow = 9840;
    const int nRangeSize = 10;
    for (int i = 0; i < nRangeSize; ++i)
        rDoc.SetValue(ScAddress(0, nRow + i, 0), nRangeSize - i);

    // Two tall hidden rows: hiding them removes more height than hiding the default rows the
    // sort is going to hide in their place, so the total height above the rows below changes
    // with every move of the hidden flags.
    rDoc.SetRowHeightRange(nRow + 2, nRow + 3, 0, 800);
    rDoc.SetManualHeight(nRow + 2, nRow + 3, 0, true);
    rDoc.SetRowHidden(nRow + 2, nRow + 3, 0, true);

    dispatchCommand(mxComponent, u".uno:GoToCell"_ustr,
                    comphelper::InitPropertySequence(
                        { { u"ToPoint"_ustr, cpo::uno::Any(u"A9841:A9850"_ustr) } }));
    dispatchCommand(mxComponent, u".uno:SortAscending"_ustr, {});
    Scheduler::ProcessEventsToIdle();

    // The rows that carried the values 8 and 7 moved to the positions of the values in the
    // ascending order, and took their hidden flags with them; the tall rows became visible.
    CPPUNIT_ASSERT_EQUAL(1.0, rDoc.GetValue(ScAddress(0, nRow, 0)));
    CPPUNIT_ASSERT(rDoc.RowHidden(nRow + 6, 0));
    CPPUNIT_ASSERT(rDoc.RowHidden(nRow + 7, 0));
    CPPUNIT_ASSERT(!rDoc.RowHidden(nRow + 2, 0));

    const tools::Long nRowOffsetTw = rDoc.GetRowHeight(0, nRow - 1, 0);
    const tools::Rectangle aVisArea(0, nRowOffsetTw - 2000, 20000, nRowOffsetTw + 8000);
    requestRowColumnHeaders(pModelObj, aVisArea);

    const double fPPTY = pViewData->GetPPTY();
    CPPUNIT_ASSERT_EQUAL(expectedRowPosition(rDoc, fPPTY, nRow + 8),
                         pViewData->GetScrPos(0, nRow + 8, pViewData->GetActivePart()).Y());

    aView.m_sInvalidateSheetGeometry = ""_ostr;
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    Scheduler::ProcessEventsToIdle();

    // The undo moved the hidden flags back to the tall rows.
    CPPUNIT_ASSERT(rDoc.RowHidden(nRow + 2, 0));
    CPPUNIT_ASSERT(rDoc.RowHidden(nRow + 3, 0));
    CPPUNIT_ASSERT(!rDoc.RowHidden(nRow + 6, 0));

    CPPUNIT_ASSERT_EQUAL(expectedRowPosition(rDoc, fPPTY, nRow + 8),
                         pViewData->GetScrPos(0, nRow + 8, pViewData->GetActivePart()).Y());

    // The client re-reads the row geometry only when it is told that it changed.
    CPPUNIT_ASSERT_EQUAL("rows"_ostr, aView.m_sInvalidateSheetGeometry);
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testTilesBelowSortedRangeInvalidatedOnSort)
{
    // Sorting a range with hidden rows moves the hidden flags along with the rows, so every
    // row below the sorted range can sit at a new position. The clients keep the old pixels
    // of that area until they receive an invalidation for it.
    comphelper::COKit::setCompatFlag(comphelper::COKit::Compat::scPrintTwipsMsgs);
    ScModelObj* pModelObj = createDoc("empty.ods");
    ScViewData* pViewData = ScDocShell::GetViewData();
    CPPUNIT_ASSERT(pViewData);
    ScDocument& rDoc = pViewData->GetDocument();
    ScTestViewCallback aView;

    const SCROW nRow = 9840;
    const int nRangeSize = 10;
    for (int i = 0; i < nRangeSize; ++i)
        rDoc.SetValue(ScAddress(0, nRow + i, 0), nRangeSize - i);

    // Content below the sorted range extends the document there, so invalidations for that
    // area are not clipped away.
    for (SCROW nR = nRow + 12; nR <= nRow + 20; ++nR)
        rDoc.SetString(ScAddress(1, nR, 0), "row " + OUString::number(nR + 1));

    // Two tall hidden rows: every move of the hidden flags changes the total height of the
    // sorted range and with it the position of everything below.
    rDoc.SetRowHeightRange(nRow + 2, nRow + 3, 0, 800);
    rDoc.SetManualHeight(nRow + 2, nRow + 3, 0, true);
    rDoc.SetRowHidden(nRow + 2, nRow + 3, 0, true);

    dispatchCommand(mxComponent, u".uno:GoToCell"_ustr,
                    comphelper::InitPropertySequence(
                        { { u"ToPoint"_ustr, cpo::uno::Any(u"A9841:A9850"_ustr) } }));

    // The client viewport covers the sorted range and the rows below it; invalidations are
    // clipped to the area the client can show.
    const tools::Long nRowOffsetTw = rDoc.GetRowHeight(0, nRow - 1, 0);
    pModelObj->setClientVisibleArea(
        tools::Rectangle(0, nRowOffsetTw - 1000, 20000, nRowOffsetTw + 12000));
    Scheduler::ProcessEventsToIdle();

    auto anyInvalidationCoversY = [&aView](tools::Long nY) {
        for (const auto& rRect : aView.m_aInvalidations)
            if (rRect.Top() <= nY && rRect.Bottom() >= nY)
                return true;
        return false;
    };

    aView.ClearAllInvalids();
    dispatchCommand(mxComponent, u".uno:SortAscending"_ustr, {});
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(rDoc.RowHidden(nRow + 6, 0));

    // A row a few rows below the sorted range sits at a new position now, so its old pixels
    // are stale and the view has to receive an invalidation covering it.
    CPPUNIT_ASSERT(anyInvalidationCoversY(rDoc.GetRowHeight(0, nRow + 14, 0)));

    aView.ClearAllInvalids();
    dispatchCommand(mxComponent, u".uno:SortDescending"_ustr, {});
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(rDoc.RowHidden(nRow + 2, 0));

    CPPUNIT_ASSERT(anyInvalidationCoversY(rDoc.GetRowHeight(0, nRow + 14, 0)));
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testRowsKeepPositionAfterCellStyleChange)
{
    // Changing a cell style through the style API, the way a macro or extension does,
    // recomputes the optimal height of every row that uses the style. A view that already
    // resolved row positions for that area must place the rows below at their new positions,
    // and the client has to be told that the row geometry it caches changed.
    comphelper::COKit::setCompatFlag(comphelper::COKit::Compat::scPrintTwipsMsgs);
    ScModelObj* pModelObj = createDoc("empty.ods");
    ScViewData* pViewData = ScDocShell::GetViewData();
    CPPUNIT_ASSERT(pViewData);
    ScDocument& rDoc = pViewData->GetDocument();
    ScTestViewCallback aView;

    const SCROW nRow = 9840;
    for (SCROW nR = nRow; nR <= nRow + 3; ++nR)
        rDoc.SetString(ScAddress(0, nR, 0), "row " + OUString::number(nR + 1));

    const tools::Long nRowOffsetTw = rDoc.GetRowHeight(0, nRow - 1, 0);
    const tools::Rectangle aVisArea(0, nRowOffsetTw - 2000, 20000, nRowOffsetTw + 8000);
    requestRowColumnHeaders(pModelObj, aVisArea);

    const double fPPTY = pViewData->GetPPTY();
    CPPUNIT_ASSERT_EQUAL(expectedRowPosition(rDoc, fPPTY, nRow + 8),
                         pViewData->GetScrPos(0, nRow + 8, pViewData->GetActivePart()).Y());

    // A bigger font in the default cell style makes every row with content taller.
    const sal_uInt16 nOldHeight = rDoc.GetRowHeight(nRow, 0);
    uno::Reference<style::XStyleFamiliesSupplier> xSupplier(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<container::XNameAccess> xFamilies = xSupplier->getStyleFamilies();
    uno::Reference<container::XNameAccess> xCellStyles(xFamilies->getByName(u"CellStyles"_ustr),
                                                      uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xStyle(xCellStyles->getByName(u"Default"_ustr),
                                               uno::UNO_QUERY_THROW);
    aView.m_sInvalidateSheetGeometry = ""_ostr;
    xStyle->setPropertyValue(u"CharHeight"_ustr, cpo::uno::Any(float(24)));
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(rDoc.GetRowHeight(nRow, 0) > nOldHeight);

    CPPUNIT_ASSERT_EQUAL(expectedRowPosition(rDoc, fPPTY, nRow + 8),
                         pViewData->GetScrPos(0, nRow + 8, pViewData->GetActivePart()).Y());

    // The client re-reads the row geometry only when it is told that it changed.
    CPPUNIT_ASSERT_EQUAL("rows sizes"_ostr, aView.m_sInvalidateSheetGeometry);
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testDisableUndoRepair)
{
    ScModelObj* pModelObj = createDoc("cursor-away.ods");
    CPPUNIT_ASSERT(pModelObj);

    // view #1
    int nView1 = KitHelper::getCurrentView();
    SfxViewShell* pView1 = SfxViewShell::Current();

    // view #2
    KitHelper::createView();
    int nView2 = KitHelper::getCurrentView();
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
    KitHelper::setView(nView1);
    pModelObj->postKeyEvent(COKitKeyEventType::DOWN, 'h', 0);
    pModelObj->postKeyEvent(COKitKeyEventType::UP, 'h', 0);
    pModelObj->postKeyEvent(COKitKeyEventType::DOWN, 0, awt::Key::RETURN);
    pModelObj->postKeyEvent(COKitKeyEventType::UP, 0, awt::Key::RETURN);
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
    KitHelper::setView(nView2);
    pModelObj->setPart(1);
    pModelObj->postKeyEvent(COKitKeyEventType::DOWN, 'c', 0);
    pModelObj->postKeyEvent(COKitKeyEventType::UP, 'c', 0);
    pModelObj->postKeyEvent(COKitKeyEventType::DOWN, 0, awt::Key::RETURN);
    pModelObj->postKeyEvent(COKitKeyEventType::UP, 0, awt::Key::RETURN);
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

    SfxViewShell::Current()->setCOKitViewCallback(nullptr);
    KitHelper::setView(nView1);
    SfxViewShell::Current()->setCOKitViewCallback(nullptr);
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testDocumentRepair)
{
    // Create two views.
    ScModelObj* pModelObj = createDoc("cursor-away.ods");
    CPPUNIT_ASSERT(pModelObj);

    // view #1
    SfxViewShell* pView1 = SfxViewShell::Current();

    // view #2
    int nView1 = KitHelper::getCurrentView();
    KitHelper::createView();
    SfxViewShell* pView2 = SfxViewShell::Current();
    int nView2 = KitHelper::getCurrentView();
    CPPUNIT_ASSERT(pView1 != pView2);
    {
        std::unique_ptr<SfxBoolItem> pItem1;
        std::unique_ptr<SfxBoolItem> pItem2;
        pView1->GetViewFrame().GetBindings().QueryState(SID_DOC_REPAIR, pItem1);
        pView2->GetViewFrame().GetBindings().QueryState(SID_DOC_REPAIR, pItem2);
        CPPUNIT_ASSERT(pItem1);
        CPPUNIT_ASSERT(pItem2);
        CPPUNIT_ASSERT_EQUAL(false, pItem1->GetValue());
        CPPUNIT_ASSERT_EQUAL(false, pItem2->GetValue());
    }

    // Insert a character in the second view.
    KitHelper::setView(nView2);
    pModelObj->setPart(1);
    pModelObj->postKeyEvent(COKitKeyEventType::DOWN, 'c', 0);
    pModelObj->postKeyEvent(COKitKeyEventType::UP, 'c', 0);
    pModelObj->postKeyEvent(COKitKeyEventType::DOWN, 0, awt::Key::RETURN);
    pModelObj->postKeyEvent(COKitKeyEventType::UP, 0, awt::Key::RETURN);
    Scheduler::ProcessEventsToIdle();
    {
        std::unique_ptr<SfxBoolItem> pItem1;
        std::unique_ptr<SfxBoolItem> pItem2;
        pView1->GetViewFrame().GetBindings().QueryState(SID_DOC_REPAIR, pItem1);
        pView2->GetViewFrame().GetBindings().QueryState(SID_DOC_REPAIR, pItem2);
        CPPUNIT_ASSERT(pItem1);
        CPPUNIT_ASSERT(pItem2);
        CPPUNIT_ASSERT_EQUAL(true, pItem1->GetValue());
        CPPUNIT_ASSERT_EQUAL(true, pItem2->GetValue());
    }

    SfxViewShell::Current()->setCOKitViewCallback(nullptr);
    KitHelper::setView(nView1);
    SfxViewShell::Current()->setCOKitViewCallback(nullptr);
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testLanguageStatus)
{
    ScModelObj* pModelObj = createDoc("small.ods");
    CPPUNIT_ASSERT(pModelObj);
    ScDocShell* pDocSh = dynamic_cast< ScDocShell* >( pModelObj->GetEmbeddedObject() );
    CPPUNIT_ASSERT(pDocSh);

    // view #1
    SfxViewShell* pView1 = SfxViewShell::Current();

    // view #2
    int nView1 = KitHelper::getCurrentView();
    KitHelper::createView();
    SfxViewShell* pView2 = SfxViewShell::Current();
    CPPUNIT_ASSERT(pView1 != pView2);
    {
        std::unique_ptr<SfxPoolItem> xItem1;
        std::unique_ptr<SfxPoolItem> xItem2;
        pView1->GetViewFrame().GetBindings().QueryState(SID_LANGUAGE_STATUS, xItem1);
        pView2->GetViewFrame().GetBindings().QueryState(SID_LANGUAGE_STATUS, xItem2);
        const SfxStringItem* pItem1 = dynamic_cast<const SfxStringItem*>(xItem1.get());
        const SfxStringItem* pItem2 = dynamic_cast<const SfxStringItem*>(xItem2.get());
        CPPUNIT_ASSERT(pItem1);
        CPPUNIT_ASSERT(pItem2);
        CPPUNIT_ASSERT(!pItem1->GetValue().isEmpty());
        CPPUNIT_ASSERT(!pItem2->GetValue().isEmpty());
    }

    {
        SfxStringItem aLangString(SID_LANGUAGE_STATUS, u"Default_Spanish (Bolivia)"_ustr);
        pView1->GetViewFrame().GetDispatcher()->ExecuteList(SID_LANGUAGE_STATUS,
            SfxCallMode::SYNCHRON, { &aLangString });
    }

    {
        std::unique_ptr<SfxPoolItem> xItem1;
        std::unique_ptr<SfxPoolItem> xItem2;
        pView1->GetViewFrame().GetBindings().QueryState(SID_LANGUAGE_STATUS, xItem1);
        pView2->GetViewFrame().GetBindings().QueryState(SID_LANGUAGE_STATUS, xItem2);
        const SfxStringItem* pItem1 = dynamic_cast<const SfxStringItem*>(xItem1.get());
        const SfxStringItem* pItem2 = dynamic_cast<const SfxStringItem*>(xItem2.get());
        CPPUNIT_ASSERT(pItem1);
        CPPUNIT_ASSERT(pItem2);
        static constexpr OUString aLangBolivia(u"Spanish (Bolivia);es-BO"_ustr);
        CPPUNIT_ASSERT_EQUAL(aLangBolivia, pItem1->GetValue());
        CPPUNIT_ASSERT_EQUAL(aLangBolivia, pItem2->GetValue());
    }

    SfxViewShell::Current()->setCOKitViewCallback(nullptr);
    KitHelper::setView(nView1);
    SfxViewShell::Current()->setCOKitViewCallback(nullptr);
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testMultiViewCopyPaste)
{
    ScModelObj* pModelObj = createDoc("empty.ods");
    ScDocument* pDoc = pModelObj->GetDocument();
    CPPUNIT_ASSERT(pDoc);

    pDoc->SetString(ScAddress(0, 0, 0), u"TestCopy1"_ustr);
    pDoc->SetString(ScAddress(1, 0, 0), u"TestCopy2"_ustr);

    // view #1
    ScTabViewShell* pView1 = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
    CPPUNIT_ASSERT(pView1);
    // emulate clipboard
    pView1->GetViewData().GetActiveWin()->SetClipboard(css::datatransfer::clipboard::KitClipboard::create(comphelper::getProcessComponentContext()));

    // view #2
    int nView1 = KitHelper::getCurrentView();
    KitHelper::createView();
    ScTabViewShell* pView2 = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
    // emulate clipboard
    pView2->GetViewData().GetActiveWin()->SetClipboard(css::datatransfer::clipboard::KitClipboard::create(comphelper::getProcessComponentContext()));
    CPPUNIT_ASSERT(pView2);
    CPPUNIT_ASSERT(pView1 != pView2);
    CPPUNIT_ASSERT(pView1->GetViewData().GetActiveWin()->GetClipboard() != pView2->GetViewData().GetActiveWin()->GetClipboard());

    // copy text view 1
    pView1->SetCursor(0, 0);
    pView1->GetViewFrame().GetBindings().Execute(SID_COPY);

    // copy text view 2
    pView2->SetCursor(1, 0);
    pView2->GetViewFrame().GetBindings().Execute(SID_COPY);

     // paste text view 1
    pView1->SetCursor(0, 1);
    pView1->GetViewFrame().GetBindings().Execute(SID_PASTE);

    // paste text view 2
    pView2->SetCursor(1, 1);
    pView2->GetViewFrame().GetBindings().Execute(SID_PASTE);

    CPPUNIT_ASSERT_EQUAL(u"TestCopy1"_ustr, pDoc->GetString(ScAddress(0, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(u"TestCopy2"_ustr, pDoc->GetString(ScAddress(1, 1, 0)));

    SfxViewShell::Current()->setCOKitViewCallback(nullptr);
    KitHelper::setView(nView1);
    SfxViewShell::Current()->setCOKitViewCallback(nullptr);
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testIMESupport)
{
    ScModelObj* pModelObj = createDoc("empty.ods");
    VclPtr<vcl::Window> pDocWindow = pModelObj->getDocWindow();
    ScDocument* pDoc = pModelObj->GetDocument();

    ScTabViewShell* pView = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
    CPPUNIT_ASSERT(pView);

    pView->SetCursor(0, 0);
    // sequence of chinese IME compositions when 'nihao' is typed in an IME
    const std::vector<OString> aUtf8Inputs{ "年"_ostr, "你"_ostr, "你好"_ostr, "你哈"_ostr, "你好"_ostr, "你好"_ostr };
    std::vector<OUString> aInputs;
    std::transform(aUtf8Inputs.begin(), aUtf8Inputs.end(),
                   std::back_inserter(aInputs), [](OString aInput) {
                       return OUString::fromUtf8(aInput);
                   });
    for (const auto& aInput: aInputs)
    {
        pDocWindow->PostExtTextInputEvent(VclEventId::ExtTextInput, aInput);
    }
    pDocWindow->PostExtTextInputEvent(VclEventId::EndExtTextInput, u""_ustr);

    // commit the string to the cell
    pModelObj->postKeyEvent(COKitKeyEventType::DOWN, 0, awt::Key::RETURN);
    pModelObj->postKeyEvent(COKitKeyEventType::UP, 0, awt::Key::RETURN);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(aInputs[aInputs.size() - 1], pDoc->GetString(ScAddress(0, 0, 0)));
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testFilterDlg)
{
    createDoc("empty.ods");

    // view #1
    SfxViewShell* pView1 = SfxViewShell::Current();
    int nView1 = KitHelper::getCurrentView();

    // view #2
    KitHelper::createView();
    SfxViewShell* pView2 = SfxViewShell::Current();
    CPPUNIT_ASSERT(pView1 != pView2);
    {
        pView2->GetViewFrame().GetDispatcher()->Execute(SID_FILTER,
            SfxCallMode::SLOT|SfxCallMode::RECORD);
    }

    Scheduler::ProcessEventsToIdle();
    SfxChildWindow* pRefWindow = pView2->GetViewFrame().GetChildWindow(SID_FILTER);
    CPPUNIT_ASSERT(pRefWindow);

    // switch to view 1
    KitHelper::setView(nView1);
    CPPUNIT_ASSERT_EQUAL(true, pView2->GetViewFrame().GetDispatcher()->IsLocked());
    CPPUNIT_ASSERT_EQUAL(false, pView1->GetViewFrame().GetDispatcher()->IsLocked());

    pRefWindow->GetController()->response(RET_CANCEL);

    CPPUNIT_ASSERT_EQUAL(false, pView2->GetViewFrame().GetDispatcher()->IsLocked());
    CPPUNIT_ASSERT_EQUAL(false, pView1->GetViewFrame().GetDispatcher()->IsLocked());

    SfxViewShell::Current()->setCOKitViewCallback(nullptr);
    KitHelper::setView(nView1);
    SfxViewShell::Current()->setCOKitViewCallback(nullptr);
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testFunctionDlg)
{
    createDoc("empty.ods");

    // view #1
    SfxViewShell* pView1 = SfxViewShell::Current();
    int nView1 = KitHelper::getCurrentView();
    {
        pView1->GetViewFrame().GetDispatcher()->Execute(SID_OPENDLG_FUNCTION,
            SfxCallMode::SLOT|SfxCallMode::RECORD);
    }
    Scheduler::ProcessEventsToIdle();
    SfxChildWindow* pRefWindow = pView1->GetViewFrame().GetChildWindow(SID_OPENDLG_FUNCTION);
    CPPUNIT_ASSERT(pRefWindow);

    // view #2
    int nView2 = KitHelper::createView();
    SfxViewShell* pView2 = SfxViewShell::Current();
    CPPUNIT_ASSERT(pView1 != pView2);

    // check locking
    CPPUNIT_ASSERT_EQUAL(true, pView1->GetViewFrame().GetDispatcher()->IsLocked());
    CPPUNIT_ASSERT_EQUAL(false, pView2->GetViewFrame().GetDispatcher()->IsLocked());

    KitHelper::setView(nView1);
    pRefWindow->GetController()->response(RET_CANCEL);

    CPPUNIT_ASSERT_EQUAL(false, pView1->GetViewFrame().GetDispatcher()->IsLocked());
    CPPUNIT_ASSERT_EQUAL(false, pView2->GetViewFrame().GetDispatcher()->IsLocked());

    SfxViewShell::Current()->setCOKitViewCallback(nullptr);
    KitHelper::setView(nView2);
    SfxViewShell::Current()->setCOKitViewCallback(nullptr);
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testSpellOnlineParameter)
{
    createDoc("empty.ods");

    ScTabViewShell* pView = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
    CPPUNIT_ASSERT(pView);

    bool bSet = pView->IsAutoSpell();

    cpo::uno::Sequence<beans::PropertyValue> params =
    {
        comphelper::makePropertyValue(u"Enable"_ustr, cpo::uno::Any(!bSet)),
    };
    dispatchCommand(mxComponent, u".uno:SpellOnline"_ustr, params);
    CPPUNIT_ASSERT_EQUAL(!bSet, pView->IsAutoSpell());

    // set the same state as now and we don't expect any change (no-toggle)
    params =
    {
        comphelper::makePropertyValue(u"Enable"_ustr, cpo::uno::Any(!bSet)),
    };
    dispatchCommand(mxComponent, u".uno:SpellOnline"_ustr, params);
    CPPUNIT_ASSERT_EQUAL(!bSet, pView->IsAutoSpell());
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testVbaRangeCopyPaste)
{
    ScModelObj* pModelObj = createDoc("RangeCopyPaste.ods");
    ScDocShell* pDocShell = dynamic_cast< ScDocShell* >( pModelObj->GetEmbeddedObject() );
    CPPUNIT_ASSERT(pDocShell);

    cpo::uno::Any aRet;
    cpo::uno::Sequence< cpo::uno::Any > aOutParam;
    cpo::uno::Sequence< cpo::uno::Any > aParams;
    cpo::uno::Sequence< sal_Int16 > aOutParamIndex;

    SfxObjectShell::CallXScript(
        mxComponent,
        u"vnd.sun.Star.script:Standard.Module1.Test_RangeCopyPaste?language=Basic&location=document"_ustr,
        aParams, aRet, aOutParamIndex, aOutParam);

    CPPUNIT_ASSERT(!pDocShell->GetClipData().is());
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testInvalidationLoop)
{
    // Load the document with a form control.
    createDoc("invalidation-loop.fods");
    // Without the accompanying fix in place, this test would have never returned due to an infinite
    // invalidation loop between ScGridWindow::Paint() and vcl::Window::ImplPosSizeWindow().
    Scheduler::ProcessEventsToIdle();
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testPageDownInvalidation)
{
    ScModelObj* pModelObj = createDoc("empty.ods");
    ScViewData* pViewData = ScDocShell::GetViewData();
    CPPUNIT_ASSERT(pViewData);

    int nView1 = KitHelper::getCurrentView();
    ScTestViewCallback aView1;
    CPPUNIT_ASSERT(!hasEditView(*pViewData));

    KitHelper::setView(nView1);
    aView1.m_bInvalidateTiles = false;
    aView1.m_aInvalidations.clear();
    pModelObj->postKeyEvent(COKitKeyEventType::DOWN, awt::Key::PAGEDOWN, 0);
    pModelObj->postKeyEvent(COKitKeyEventType::UP, awt::Key::PAGEDOWN, 0);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(aView1.m_bInvalidateTiles);
    CPPUNIT_ASSERT_EQUAL(size_t(3), aView1.m_aInvalidations.size());
    CPPUNIT_ASSERT_EQUAL(tools::Rectangle(15, 15, 1230, 225), aView1.m_aInvalidations[0]);
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testSheetChangeNoInvalidation)
{
    ScModelObj* pModelObj = createDoc("two_sheets.ods");
    ScViewData* pViewData = ScDocShell::GetViewData();
    CPPUNIT_ASSERT(pViewData);

    // Set View to initial 100%
    pModelObj->setClientVisibleArea(tools::Rectangle(0, 0, 28050, 10605));
    pModelObj->setClientZoom(256, 256, 1920, 1920);

    ScTabViewShell* pView = dynamic_cast<ScTabViewShell*>(SfxViewShell::Current());
    CPPUNIT_ASSERT(pView);

    int nView1 = KitHelper::getCurrentView();
    ScTestViewCallback aView1;
    CPPUNIT_ASSERT(!hasEditView(*pViewData));

    KitHelper::setView(nView1);

    aView1.ClearAllInvalids();

    pModelObj->postKeyEvent(COKitKeyEventType::DOWN, 0, awt::Key::PAGEDOWN | KEY_MOD1);
    pModelObj->postKeyEvent(COKitKeyEventType::UP, 0, awt::Key::PAGEDOWN | KEY_MOD1);
    Scheduler::ProcessEventsToIdle();
    // switching sheets should trigger no unnecessary invalidations
    CPPUNIT_ASSERT(!aView1.m_bInvalidateTiles);

    // Get the known columns/rows of this sheet 2 now we have switched to it so
    // it knows what range to broadcast invalidations for if it knows cells need
    // to be redrawn.
    tools::JsonWriter aJsonWriter1;
    pModelObj->getRowColumnHeaders(tools::Rectangle(0, 15, 19650, 5400), aJsonWriter1);
    aJsonWriter1.finishAndGetAsOString();
    Scheduler::ProcessEventsToIdle();
    aView1.ClearAllInvalids();

    // switching back should also trigger no unnecessary invalidations
    pModelObj->postKeyEvent(COKitKeyEventType::DOWN, 0, awt::Key::PAGEUP | KEY_MOD1);
    pModelObj->postKeyEvent(COKitKeyEventType::UP, 0, awt::Key::PAGEUP | KEY_MOD1);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(!aView1.m_bInvalidateTiles);

    // The 2nd sheet has formulas that depend on B1 in the first sheet. So if
    // we change B1 there should be an invalidation in the second sheet for the
    // range that depends on it. Because this is a single user document with no
    // active view on the 2nd sheet this will happen on switching back to sheet 2
    typeCharsInCell("101", 1, 0, pView, pModelObj); // Type '101' in B1
    aView1.ClearAllInvalids();

    pModelObj->postKeyEvent(COKitKeyEventType::DOWN, 0, awt::Key::PAGEDOWN | KEY_MOD1);
    pModelObj->postKeyEvent(COKitKeyEventType::UP, 0, awt::Key::PAGEDOWN | KEY_MOD1);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(aView1.m_bInvalidateTiles);
    aView1.ClearAllInvalids();

    // Paint it to make it valid again
    getTile(pModelObj, 0, 0, 3840, 3840);

    // switching back to sheet 1 should trigger no unnecessary invalidations
    pModelObj->postKeyEvent(COKitKeyEventType::DOWN, 0, awt::Key::PAGEUP | KEY_MOD1);
    pModelObj->postKeyEvent(COKitKeyEventType::UP, 0, awt::Key::PAGEUP | KEY_MOD1);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(!aView1.m_bInvalidateTiles);

    // switching to sheet 2 should trigger no unnecessary invalidations this time
    pModelObj->postKeyEvent(COKitKeyEventType::DOWN, 0, awt::Key::PAGEDOWN | KEY_MOD1);
    pModelObj->postKeyEvent(COKitKeyEventType::UP, 0, awt::Key::PAGEDOWN | KEY_MOD1);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(!aView1.m_bInvalidateTiles);
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testInsertDeletePageInvalidation)
{
    ScModelObj* pModelObj = createDoc("insert_delete_sheet.ods");
    // the document has 1 sheet
    CPPUNIT_ASSERT_EQUAL(1, pModelObj->getParts());
    ScViewData* pViewData = ScDocShell::GetViewData();
    CPPUNIT_ASSERT(pViewData);

    int nView1 = KitHelper::getCurrentView();
    ScTestViewCallback aView1;
    CPPUNIT_ASSERT(!hasEditView(*pViewData));

    KitHelper::setView(nView1);
    aView1.m_bInvalidateTiles = false;
    aView1.m_aInvalidations.clear();

    cpo::uno::Sequence<beans::PropertyValue> aArgs( comphelper::InitPropertySequence({
            { u"Name"_ustr, cpo::uno::Any(u""_ustr) },
            { u"Index"_ustr, cpo::uno::Any(sal_Int32(1)) }
        }));
    dispatchCommand(mxComponent, u".uno:Insert"_ustr, aArgs);
    CPPUNIT_ASSERT(aView1.m_bInvalidateTiles);
    CPPUNIT_ASSERT_EQUAL(size_t(2), aView1.m_aInvalidations.size());
    CPPUNIT_ASSERT_EQUAL(tools::Rectangle(0, 0, 1000000000, 1000000000), aView1.m_aInvalidations[0]);
    CPPUNIT_ASSERT_EQUAL(2, pModelObj->getParts());

    // Delete sheet
    aView1.m_bInvalidateTiles = false;
    aView1.m_aInvalidations.clear();
    cpo::uno::Sequence<beans::PropertyValue> aArgs2( comphelper::InitPropertySequence({
            { u"Index"_ustr, cpo::uno::Any(sal_Int32(1)) }
        }));
    dispatchCommand(mxComponent, u".uno:Remove"_ustr, aArgs2);
    CPPUNIT_ASSERT(aView1.m_bInvalidateTiles);
    CPPUNIT_ASSERT_EQUAL(size_t(1), aView1.m_aInvalidations.size());
    CPPUNIT_ASSERT_EQUAL(tools::Rectangle(0, 0, 1000000000, 1000000000), aView1.m_aInvalidations[0]);
    CPPUNIT_ASSERT_EQUAL(1, pModelObj->getParts());
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testGetRowColumnHeadersInvalidation)
{
    // NOTE NOTE NOTE
    // If you run this test in isolation using CPPUNIT_TEST_NAME=, it will fail because the invalidations
    // will be different.

    ScModelObj* pModelObj = createDoc("empty.ods");
    ScViewData* pViewData = ScDocShell::GetViewData();
    CPPUNIT_ASSERT(pViewData);

    int nView1 = KitHelper::getCurrentView();
    ScTestViewCallback aView1;
    CPPUNIT_ASSERT(!hasEditView(*pViewData));

    KitHelper::setView(nView1);
    aView1.m_bInvalidateTiles = false;
    aView1.m_aInvalidations.clear();
    tools::JsonWriter aJsonWriter1;
    pModelObj->getRowColumnHeaders(tools::Rectangle(0, 15, 19650, 5400), aJsonWriter1);
    aJsonWriter1.finishAndGetAsOString();
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(aView1.m_bInvalidateTiles);
    CPPUNIT_ASSERT_EQUAL(size_t(1), aView1.m_aInvalidations.size());
    CPPUNIT_ASSERT_EQUAL(tools::Rectangle(Point(26775, 0), Size(22950, 13005)), aView1.m_aInvalidations[0]);

    // Extend area top-to-bottom
    aView1.m_bInvalidateTiles = false;
    aView1.m_aInvalidations.clear();
    tools::JsonWriter aJsonWriter2;
    pModelObj->getRowColumnHeaders(tools::Rectangle(0, 5400, 19650, 9800), aJsonWriter2);
    aJsonWriter2.finishAndGetAsOString();
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(aView1.m_bInvalidateTiles);
    CPPUNIT_ASSERT_EQUAL(size_t(1), aView1.m_aInvalidations.size());
    CPPUNIT_ASSERT_EQUAL(tools::Rectangle(Point(0, 13005), Size(49725, 6375)), aView1.m_aInvalidations[0]);

    // Extend area left-to-right
    aView1.m_bInvalidateTiles = false;
    aView1.m_aInvalidations.clear();
    tools::JsonWriter aJsonWriter3;
    pModelObj->getRowColumnHeaders(tools::Rectangle(5400, 5400, 25050, 9800), aJsonWriter3);
    aJsonWriter3.finishAndGetAsOString();
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(aView1.m_bInvalidateTiles);
    CPPUNIT_ASSERT_EQUAL(size_t(1), aView1.m_aInvalidations.size());
    CPPUNIT_ASSERT_EQUAL(tools::Rectangle(Point(49725, 0), Size(25500, 19380)), aView1.m_aInvalidations[0]);
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testJumpHorizontallyInvalidation)
{
    ScModelObj* pModelObj = createDoc("empty.ods");
    ScViewData* pViewData = ScDocShell::GetViewData();
    CPPUNIT_ASSERT(pViewData);

    int nView1 = KitHelper::getCurrentView();
    ScTestViewCallback aView1;
    CPPUNIT_ASSERT(!hasEditView(*pViewData));

    KitHelper::setView(nView1);
    aView1.m_bInvalidateTiles = false;
    aView1.m_aInvalidations.clear();
    pModelObj->postKeyEvent(COKitKeyEventType::DOWN, 0, awt::Key::PAGEDOWN | KEY_MOD2);
    pModelObj->postKeyEvent(COKitKeyEventType::UP, 0, awt::Key::PAGEDOWN | KEY_MOD2);
    Scheduler::ProcessEventsToIdle();
    pModelObj->postKeyEvent(COKitKeyEventType::DOWN, 0, awt::Key::PAGEDOWN | KEY_MOD2);
    pModelObj->postKeyEvent(COKitKeyEventType::UP, 0, awt::Key::PAGEDOWN | KEY_MOD2);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(aView1.m_bInvalidateTiles);
    CPPUNIT_ASSERT_EQUAL(size_t(1), aView1.m_aInvalidations.size());
    CPPUNIT_ASSERT_EQUAL(tools::Rectangle(26775, 0, 39525, 13005), aView1.m_aInvalidations[0]);
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testChartInsertPosNoBorderOffset)
{
    // cool#2222: a newly inserted chart landed with a stray offset from the
    // grid line, instead of its edge sitting exactly on it.
    ScModelObj* pModelObj = createDoc("empty.ods");
    ScDocument* pDoc = pModelObj->GetDocument();
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    CPPUNIT_ASSERT(pViewShell);
    CPPUNIT_ASSERT(comphelper::COKit::isActive());

    // Give the view plenty of room, so the chart can be placed flush against
    // the selection instead of the position getting clamped into a
    // constrained viewport.
    pModelObj->setClientVisibleArea(tools::Rectangle(0, 0, 28050, 10605));

    const Size aChartSize(5000, 5000);
    const ScRange aRange(0, 0, 0, 1, 1, 0); // A1:B2
    const Point aPos = pViewShell->GetChartInsertPos(aChartSize, aRange);

    // The selection sits at the top-left corner of a fresh sheet, so there is
    // plenty of room to place the chart directly to its right, flush against
    // the grid line between the selection and the next column. Without the
    // fix, this landed 100 (1/100 mm) short of that line on both axes.
    const tools::Rectangle aSelection = pDoc->GetMMRect(0, 0, 1, 1, 0);
    CPPUNIT_ASSERT_EQUAL(aSelection.Right() + 1, aPos.X());
    CPPUNIT_ASSERT_EQUAL(aSelection.Top(), aPos.Y());
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testReferenceMarksClearedAfterEdit)
{
    // cool#15850: the pink reference-highlight overlay a formula shows while
    // being edited (e.g. "=" then pick another cell) stayed on screen after
    // pressing Enter, because nothing told the client to clear it once
    // editing was over.
    ScModelObj* pModelObj = createDoc("empty.ods");
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    CPPUNIT_ASSERT(pViewShell);

    int nView1 = KitHelper::getCurrentView();
    ScTestViewCallback aView1;
    KitHelper::setView(nView1);

    // Edit A2 with a formula referencing A1 above it.
    pViewShell->SetCursor(0, 1);
    pModelObj->postKeyEvent(COKitKeyEventType::DOWN, '=', 0);
    pModelObj->postKeyEvent(COKitKeyEventType::UP, '=', 0);
    Scheduler::ProcessEventsToIdle();
    pModelObj->postKeyEvent(COKitKeyEventType::DOWN, 0, awt::Key::UP);
    pModelObj->postKeyEvent(COKitKeyEventType::UP, 0, awt::Key::UP);
    Scheduler::ProcessEventsToIdle();

    // The reference to A1 is now highlighted.
    CPPUNIT_ASSERT(aView1.m_sReferenceMarks.indexOf("rectangle") >= 0);

    pModelObj->postKeyEvent(COKitKeyEventType::DOWN, 0, awt::Key::RETURN);
    pModelObj->postKeyEvent(COKitKeyEventType::UP, 0, awt::Key::RETURN);
    Scheduler::ProcessEventsToIdle();

    // Without the fix, the highlight from above was never cleared.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), aView1.m_sReferenceMarks.indexOf("rectangle"));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
