/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <string_view>

#include <test/calc_unoapi_test.hxx>

#include <comphelper/propertyvalue.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/request.hxx>
#include <svx/svdoashp.hxx>
#include <svx/svdomeas.hxx>
#include <svx/svdpage.hxx>
#include <unotools/tempfile.hxx>
#include <vcl/keycodes.hxx>

#include <docsh.hxx>
#include <drwlayer.hxx>
#include <tabvwsh.hxx>
#include <userdat.hxx>

#include <sc.hrc> // defines of slot-IDs

using namespace css;

namespace sc_apitest
{
class ScShapeTest : public CalcUnoApiTest
{
public:
    ScShapeTest();
    void saveAndReload(css::uno::Reference<css::lang::XComponent>& xComponent,
                       const OUString& rFilter);
    void testTdf137033_FlipHori_Resize();
    void testTdf137033_RotShear_ResizeHide();
    void testTdf137033_RotShear_Hide();
// this test has starting failing under OSX
#if !defined MACOSX
    void testTdf137576_LogicRectInDefaultMeasureline();
#endif
    void testTdf137576_LogicRectInNewMeasureline();
    void testMeasurelineHideColSave();
    void testHideColsShow();
    void testTdf138138_MoveCellWithRotatedShape();
    void testLoadVerticalFlip();
    void testTdf117948_CollapseBeforeShape();
    void testTdf137355_UndoHideRows();
    void testTdf115655_HideDetail();
    void testFitToCellSize();
    void testCustomShapeCellAnchoredRotatedShape();

    CPPUNIT_TEST_SUITE(ScShapeTest);
    CPPUNIT_TEST(testTdf137033_FlipHori_Resize);
    CPPUNIT_TEST(testTdf137033_RotShear_ResizeHide);
    CPPUNIT_TEST(testTdf137033_RotShear_Hide);
// this test has starting failing under OSX
#if !defined MACOSX
    CPPUNIT_TEST(testTdf137576_LogicRectInDefaultMeasureline);
#endif
    CPPUNIT_TEST(testTdf137576_LogicRectInNewMeasureline);
    CPPUNIT_TEST(testMeasurelineHideColSave);
    CPPUNIT_TEST(testHideColsShow);
    CPPUNIT_TEST(testTdf138138_MoveCellWithRotatedShape);
    CPPUNIT_TEST(testLoadVerticalFlip);
    CPPUNIT_TEST(testTdf117948_CollapseBeforeShape);
    CPPUNIT_TEST(testTdf137355_UndoHideRows);
    CPPUNIT_TEST(testTdf115655_HideDetail);
    CPPUNIT_TEST(testFitToCellSize);
    CPPUNIT_TEST(testCustomShapeCellAnchoredRotatedShape);
    CPPUNIT_TEST_SUITE_END();
};

ScShapeTest::ScShapeTest()
    : CalcUnoApiTest("sc/qa/unit/data/ods")
{
}

void ScShapeTest::saveAndReload(css::uno::Reference<css::lang::XComponent>& xComponent,
                                const OUString& rFilter)
{
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    css::uno::Sequence<css::beans::PropertyValue> aArgs(1);
    aArgs[0].Name = "FilterName";
    aArgs[0].Value <<= rFilter; // e.g. "calc8"
    css::uno::Reference<css::frame::XStorable> xStorable(xComponent, css::uno::UNO_QUERY_THROW);
    xStorable->storeAsURL(aTempFile.GetURL(), aArgs);
    css::uno::Reference<css::util::XCloseable> xCloseable(xComponent, css::uno::UNO_QUERY_THROW);
    xCloseable->close(true);

    xComponent = loadFromDesktop(aTempFile.GetURL(), "com.sun.star.sheet.SpreadsheetDocument");
}

static void lcl_AssertRectEqualWithTolerance(std::string_view sInfo,
                                             const tools::Rectangle& rExpected,
                                             const tools::Rectangle& rActual,
                                             const sal_Int32 nTolerance)
{
    // Left
    OString sMsg = OString::Concat(sInfo) + " Left expected " + OString::number(rExpected.Left())
                   + " actual " + OString::number(rActual.Left()) + " Tolerance "
                   + OString::number(nTolerance);
    CPPUNIT_ASSERT_MESSAGE(sMsg.getStr(),
                           std::abs(rExpected.Left() - rActual.Left()) <= nTolerance);

    // Top
    sMsg = OString::Concat(sInfo) + " Top expected " + OString::number(rExpected.Top()) + " actual "
           + OString::number(rActual.Top()) + " Tolerance " + OString::number(nTolerance);
    CPPUNIT_ASSERT_MESSAGE(sMsg.getStr(), std::abs(rExpected.Top() - rActual.Top()) <= nTolerance);

    // Width
    sMsg = OString::Concat(sInfo) + " Width expected " + OString::number(rExpected.GetWidth())
           + " actual " + OString::number(rActual.GetWidth()) + " Tolerance "
           + OString::number(nTolerance);
    CPPUNIT_ASSERT_MESSAGE(sMsg.getStr(),
                           std::abs(rExpected.GetWidth() - rActual.GetWidth()) <= nTolerance);

    // Height
    sMsg = OString::Concat(sInfo) + " Height expected " + OString::number(rExpected.GetHeight())
           + " actual " + OString::number(rActual.GetHeight()) + " Tolerance "
           + OString::number(nTolerance);
    CPPUNIT_ASSERT_MESSAGE(sMsg.getStr(),
                           std::abs(rExpected.GetHeight() - rActual.GetHeight()) <= nTolerance);
}

static void lcl_AssertPointEqualWithTolerance(std::string_view sInfo, const Point rExpected,
                                              const Point rActual, const sal_Int32 nTolerance)
{
    // X
    OString sMsg = OString::Concat(sInfo) + " X expected " + OString::number(rExpected.X())
                   + " actual " + OString::number(rActual.X()) + " Tolerance "
                   + OString::number(nTolerance);
    CPPUNIT_ASSERT_MESSAGE(sMsg.getStr(), std::abs(rExpected.X() - rActual.X()) <= nTolerance);
    // Y
    sMsg = OString::Concat(sInfo) + " Y expected " + OString::number(rExpected.Y()) + " actual "
           + OString::number(rActual.Y()) + " Tolerance " + OString::number(nTolerance);
    CPPUNIT_ASSERT_MESSAGE(sMsg.getStr(), std::abs(rExpected.Y() - rActual.Y()) <= nTolerance);
}

void ScShapeTest::testTdf137033_FlipHori_Resize()
{
    // Load a document, which has a rotated custom shape, which is horizontal flipped. Error was, that
    // if such shape was anchored "resize with cell", then after save and reload it was destorted.
    OUString aFileURL;
    createFileURL(u"tdf137033_FlipHoriRotCustomShape.ods", aFileURL);
    uno::Reference<css::lang::XComponent> xComponent = loadFromDesktop(aFileURL);
    CPPUNIT_ASSERT(xComponent.is());

    // Get document
    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);
    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
    ScDocShell* pDocSh = static_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(pDocSh);
    ScDocument& rDoc = pDocSh->GetDocument();

    // Get shape
    ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
    CPPUNIT_ASSERT_MESSAGE("No ScDrawLayer", pDrawLayer);
    const SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("No draw page", pPage);
    SdrObjCustomShape* pObj = static_cast<SdrObjCustomShape*>(pPage->GetObj(0));
    CPPUNIT_ASSERT_MESSAGE("Load: custom shape not found", pObj);

    // Verify shape is correctly loaded. Then set shape to "resize with cell".
    tools::Rectangle aSnapRect(pObj->GetSnapRect());
    const tools::Rectangle aExpectRect(Point(4998, 7000), Size(9644, 6723));
    lcl_AssertRectEqualWithTolerance("Load, wrong pos or size: ", aExpectRect, aSnapRect, 1);
    ScDrawLayer::SetCellAnchoredFromPosition(*pObj, rDoc, 0 /*SCTAB*/, true /*bResizeWithCell*/);

    // Save and reload.
    saveAndReload(xComponent, "calc8");
    CPPUNIT_ASSERT(xComponent);

    // Get document
    pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);
    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
    pDocSh = static_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(pDocSh);
    ScDocument& rDoc2 = pDocSh->GetDocument();

    // Get shape
    pDrawLayer = rDoc2.GetDrawLayer();
    CPPUNIT_ASSERT_MESSAGE("No ScDrawLayer", pDrawLayer);
    pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("No draw page", pPage);
    pObj = static_cast<SdrObjCustomShape*>(pPage->GetObj(0));
    CPPUNIT_ASSERT_MESSAGE("Reload: custom shape not found", pObj);

    // Check shape has the original geometry, besides rounding and unit conversion errors
    aSnapRect = pObj->GetSnapRect();
    lcl_AssertRectEqualWithTolerance("Reload, wrong pos or size: ", aExpectRect, aSnapRect, 1);

    pDocSh->DoClose();
}

void ScShapeTest::testTdf137033_RotShear_ResizeHide()
{
    // For rotated or sheared shapes anchored "To Cell (resize with cell) hiding rows or columns will
    // not only change size but rotation and shear angle too. Error was, that not the original angles
    // of the full sized shape were written to file but the changed one.

    // Load a document, which has a rotated and sheared shape, anchored to cell with resize.
    OUString aFileURL;
    createFileURL(u"tdf137033_RotShearResizeAnchor.ods", aFileURL);
    uno::Reference<css::lang::XComponent> xComponent = loadFromDesktop(aFileURL);
    CPPUNIT_ASSERT(xComponent.is());

    // Get document
    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);
    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
    ScDocShell* pDocSh = dynamic_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(pDocSh);
    ScDocument& rDoc = pDocSh->GetDocument();

    // Hide rows 4 and 5 (UI number), which are inside the shape and thus change shape geometry
    rDoc.SetRowHidden(3, 4, 0, true);
    rDoc.SetDrawPageSize(0); // trigger recalcpos, otherwise shapes are not changed

    // Get shape
    ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
    CPPUNIT_ASSERT_MESSAGE("No ScDrawLayer", pDrawLayer);
    const SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("No draw page", pPage);
    SdrObject* pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT_MESSAGE("No object found", pObj);

    // Verify hiding has changed shape geometry as expected
    tools::Rectangle aSnapRect(pObj->GetSnapRect());
    tools::Long aRotateAngle(pObj->GetRotateAngle());
    tools::Long aShearAngle(pObj->GetShearAngle());
    // mathematical exact would be Point(3868, 4795), Size(9763, 1909)
    // current values as of LO 7.2
    const tools::Rectangle aExpectRect(Point(3871, 4796), Size(9764, 1910));
    const tools::Long aExpectRotateAngle(20923);
    const tools::Long aExpectShearAngle(-6572);
    CPPUNIT_ASSERT_MESSAGE("Hide rows, shear angle: ", abs(aShearAngle - aExpectShearAngle) <= 1);
    CPPUNIT_ASSERT_MESSAGE("Hide rows, rotate angle: ",
                           abs(aRotateAngle - aExpectRotateAngle) <= 1);
    lcl_AssertRectEqualWithTolerance("Reload: wrong pos or size", aExpectRect, aSnapRect, 1);

    // Save and reload.
    saveAndReload(xComponent, "calc8");
    CPPUNIT_ASSERT(xComponent);

    // Get document
    pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);
    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
    pDocSh = dynamic_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(pDocSh);
    ScDocument& rDoc2 = pDocSh->GetDocument();

    // Get shape
    pDrawLayer = rDoc2.GetDrawLayer();
    CPPUNIT_ASSERT_MESSAGE("No ScDrawLayer", pDrawLayer);
    pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("No draw page", pPage);
    pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT_MESSAGE("No object found", pObj);

    // Check shape has the original geometry, besides heavy rounding and unit conversion errors
    aSnapRect = pObj->GetSnapRect();
    aRotateAngle = pObj->GetRotateAngle();
    aShearAngle = pObj->GetShearAngle();
    CPPUNIT_ASSERT_MESSAGE("Reload, shear angle: ", abs(aShearAngle - aExpectShearAngle) <= 3);
    CPPUNIT_ASSERT_MESSAGE("Reload, rotate angle: ", abs(aRotateAngle - aExpectRotateAngle) <= 3);
    lcl_AssertRectEqualWithTolerance("Reload: wrong pos or size", aExpectRect, aSnapRect, 4);

    pDocSh->DoClose();
}

void ScShapeTest::testTdf137033_RotShear_Hide()
{
    // Hiding row or columns affect cell anchored shape based on their snap rectangle. The first
    // attempt to fix lost position has used the logic rect instead. For rotated or sheared shape it
    // makes a difference.

    // Load a document, which has a rotated and sheared shape, anchored to cell, without resize.
    OUString aFileURL;
    createFileURL(u"tdf137033_RotShearCellAnchor.ods", aFileURL);
    uno::Reference<css::lang::XComponent> xComponent = loadFromDesktop(aFileURL);
    CPPUNIT_ASSERT(xComponent.is());

    // Get document
    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);
    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
    ScDocShell* pDocSh = dynamic_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(pDocSh);
    ScDocument& rDoc = pDocSh->GetDocument();

    // Hide column C, which is left from logic rect, but right from left edge of snap rect
    rDoc.SetColHidden(2, 2, 0, true);
    rDoc.SetDrawPageSize(0); // trigger recalcpos, otherwise shapes are not changed

    // Save and reload.
    saveAndReload(xComponent, "calc8");
    CPPUNIT_ASSERT(xComponent);

    // Get document
    pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);
    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
    pDocSh = dynamic_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(pDocSh);
    ScDocument& rDoc2 = pDocSh->GetDocument();

    // Get shape
    ScDrawLayer* pDrawLayer = rDoc2.GetDrawLayer();
    CPPUNIT_ASSERT_MESSAGE("No ScDrawLayer", pDrawLayer);
    const SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("No draw page", pPage);
    SdrObject* pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT_MESSAGE("No object found", pObj);

    // Check shape is visible. With the old version, the shape was moved to column C and
    // thus hidden on reload.
    CPPUNIT_ASSERT_MESSAGE("Reload: Shape has to be visible", pObj->IsVisible());
    // Verify position and size are unchanged besides rounding and unit conversion errors
    // Values are manually taken from shape before hiding column C.
    const tools::Rectangle aExpectRect(Point(4500, 3500), Size(15143, 5187));
    const tools::Rectangle aSnapRect = pObj->GetSnapRect();
    lcl_AssertRectEqualWithTolerance("Reload: wrong pos and size", aExpectRect, aSnapRect, 1);

    pDocSh->DoClose();
}

// this test has starting failing under OSX
#if !defined MACOSX
void ScShapeTest::testTdf137576_LogicRectInDefaultMeasureline()
{
    // Error was, that the empty logical rectangle of a default measure line (Ctrl+Click)
    // resulted in zeros in NonRotatedAnchor and a wrong position when reloading.

    // Load an empty document.
    OUString aFileURL;
    createFileURL(u"ManualColWidthRowHeight.ods", aFileURL);
    uno::Reference<css::lang::XComponent> xComponent = loadFromDesktop(aFileURL);
    CPPUNIT_ASSERT(xComponent.is());

    // Get ScDocShell
    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);
    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
    ScDocShell* pDocSh = dynamic_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT_MESSAGE("No ScDocShell", pDocSh);

    // Create default measureline by SfxRequest that corresponds to Ctrl+Click
    ScTabViewShell* pTabViewShell = pDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT_MESSAGE("No ScTabViewShell", pTabViewShell);
    SfxRequest aReq(pTabViewShell->GetViewFrame(), SID_DRAW_MEASURELINE);
    aReq.SetModifier(KEY_MOD1); // Ctrl
    pTabViewShell->ExecDraw(aReq);

    // Get document and newly created measure line.
    ScDocument& rDoc = pDocSh->GetDocument();
    ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
    CPPUNIT_ASSERT_MESSAGE("No ScDrawLayer", pDrawLayer);
    const SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("No draw page", pPage);
    SdrObject* pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT_MESSAGE("No object found", pObj);

    // Anchor "to Cell (resize with cell)"
    ScDrawLayer::SetCellAnchoredFromPosition(*pObj, rDoc, 0 /*SCTAB*/, true /*bResizeWithCell*/);
    // Deselect shape and switch to object selection type "Cell".
    pTabViewShell->SetDrawShell(false);

    // Hide column A.
    uno::Sequence<beans::PropertyValue> aPropertyValues = {
        comphelper::makePropertyValue("ToPoint", OUString("$A$1")),
    };
    dispatchCommand(xComponent, ".uno:GoToCell", aPropertyValues);
    dispatchCommand(xComponent, ".uno:HideColumn", {});

    // Get current position. I will not use absolute values for comparison, because document is loaded
    // in full screen mode of unknown size and default object is placed in center of window.
    Point aOldPos = pObj->GetRelativePos();

    // Save and reload, get ScDocShell
    saveAndReload(xComponent, "calc8");
    CPPUNIT_ASSERT(xComponent);
    pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);
    CPPUNIT_ASSERT_MESSAGE("Reload: Failed to access document shell", pFoundShell);
    pDocSh = dynamic_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(pDocSh);

    // Get document and object
    ScDocument& rDoc2 = pDocSh->GetDocument();
    pDrawLayer = rDoc2.GetDrawLayer();
    CPPUNIT_ASSERT_MESSAGE("Reload: No ScDrawLayer", pDrawLayer);
    pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("Reload: No draw page", pPage);
    pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT_MESSAGE("Measure line lost", pObj);

    // Assert object position is unchanged, besides Twips<->Hmm inaccuracy.
    Point aNewPos = pObj->GetRelativePos();
    lcl_AssertPointEqualWithTolerance("after reload", aOldPos, aNewPos, 1);

    pDocSh->DoClose();
}
#endif

void ScShapeTest::testTdf137576_LogicRectInNewMeasureline()
{
    // Error was, that a new measure line had no logical rectangle. This resulted in zeros in
    // NonRotatedAnchor. As a result the position was wrong when reloading.

    // Load an empty document
    OUString aFileURL;
    createFileURL(u"ManualColWidthRowHeight.ods", aFileURL);
    uno::Reference<css::lang::XComponent> xComponent = loadFromDesktop(aFileURL);
    CPPUNIT_ASSERT(xComponent.is());

    // Get ScDocShell
    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);
    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
    ScDocShell* pDocSh = dynamic_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(pDocSh);

    // Get SdrPage
    ScDocument& rDoc = pDocSh->GetDocument();
    ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
    CPPUNIT_ASSERT_MESSAGE("No ScDrawLayer", pDrawLayer);
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("No draw page", pPage);

    // Create a new measure line and insert it
    Point aStartPoint(5000, 5500);
    Point aEndPoint(13000, 8000);
    SdrMeasureObj* pObj = new SdrMeasureObj(*pDrawLayer, aStartPoint, aEndPoint);
    CPPUNIT_ASSERT_MESSAGE("Could not create measure line", pObj);
    pPage->InsertObject(pObj);

    // Anchor "to cell (resize with cell)" and examine NonRotatedAnchor
    ScDrawLayer::SetCellAnchoredFromPosition(*pObj, rDoc, 0 /*SCTAB*/, true /*bResizeWithCell*/);
    ScDrawObjData* pNData = ScDrawLayer::GetNonRotatedObjData(pObj);
    CPPUNIT_ASSERT_MESSAGE("Failed to get NonRotatedAnchor", pNData);
    // Without the fix all four values would be zero.
    CPPUNIT_ASSERT(pNData->maStart.Col() == 1 && pNData->maStart.Row() == 2);
    CPPUNIT_ASSERT(pNData->maEnd.Col() == 7 && pNData->maEnd.Row() == 2);

    pDocSh->DoClose();
}

void ScShapeTest::testMeasurelineHideColSave()
{
    // The document contains a SdrMeasureObj anchored "To Cell (resive with cell)" with start in cell
    // D11 and end in cell I5. Error was, that after hiding col A and saving, start and end point
    // position were lost.
    OUString aFileURL;
    createFileURL(u"measurelineHideColSave.ods", aFileURL);
    uno::Reference<css::lang::XComponent> xComponent = loadFromDesktop(aFileURL);
    CPPUNIT_ASSERT(xComponent.is());

    // Get the document model
    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);
    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
    ScDocShell* pDocSh = dynamic_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(pDocSh);

    // Get document and shape
    ScDocument& rDoc = pDocSh->GetDocument();
    ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
    CPPUNIT_ASSERT_MESSAGE("No ScDrawLayer", pDrawLayer);
    const SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("No draw page", pPage);
    SdrObject* pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT_MESSAGE("No object found", pObj);

    // Make sure loading is correct
    Point aStartPoint(7500, 15000); // according UI
    Point aEndPoint(17500, 8000);
    lcl_AssertPointEqualWithTolerance("Load start: ", aStartPoint, pObj->GetPoint(0), 1);
    lcl_AssertPointEqualWithTolerance("Load end: ", aEndPoint, pObj->GetPoint(1), 1);

    // Hide column A
    rDoc.SetColHidden(0, 0, 0, true);
    rDoc.SetDrawPageSize(0); // trigger recalcpos, otherwise shapes are not changed
    // Shape should move by column width, here 3000
    aStartPoint.Move(-3000, 0);
    aEndPoint.Move(-3000, 0);
    lcl_AssertPointEqualWithTolerance("Hide col A: ", aStartPoint, pObj->GetPoint(0), 1);
    lcl_AssertPointEqualWithTolerance("Hide col A: ", aEndPoint, pObj->GetPoint(1), 1);

    // save and reload
    saveAndReload(xComponent, "calc8");
    CPPUNIT_ASSERT(xComponent);

    // Get ScDocShell
    pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);
    CPPUNIT_ASSERT_MESSAGE("Reload: Failed to access document shell", pFoundShell);
    pDocSh = dynamic_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(pDocSh);

    // Get document and object
    ScDocument& rDoc2 = pDocSh->GetDocument();
    pDrawLayer = rDoc2.GetDrawLayer();
    CPPUNIT_ASSERT_MESSAGE("Reload: No ScDrawLayer", pDrawLayer);
    pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("Reload: No draw page", pPage);
    pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT_MESSAGE("Reload: custom shape no longer exists", pObj);

    // Check that start and end point are unchanged besides rounding and unit conversion errors
    lcl_AssertPointEqualWithTolerance("Reload start: ", aStartPoint, pObj->GetPoint(0), 2);
    lcl_AssertPointEqualWithTolerance("Reload end: ", aEndPoint, pObj->GetPoint(1), 2);

    pDocSh->DoClose();
}

void ScShapeTest::testHideColsShow()
{
    // The document contains a shape anchored "To Cell (resize with cell)" with starts in cell C3 and
    //ends in cell D5. Error was, that hiding cols C and D and then show them again extends the shape
    // to column E

    OUString aFileURL;
    createFileURL(u"hideColsShow.ods", aFileURL);
    uno::Reference<css::lang::XComponent> xComponent = loadFromDesktop(aFileURL);
    CPPUNIT_ASSERT(xComponent.is());

    // Get the document model
    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);
    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
    ScDocShell* pDocSh = dynamic_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(pDocSh);

    // Get document and shape
    ScDocument& rDoc = pDocSh->GetDocument();
    ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
    CPPUNIT_ASSERT_MESSAGE("No ScDrawLayer", pDrawLayer);
    const SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("No draw page", pPage);
    SdrObject* pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT_MESSAGE("No object found", pObj);
    CPPUNIT_ASSERT_MESSAGE("Load: Object should be visible", pObj->IsVisible());
    tools::Rectangle aSnapRectOrig(pObj->GetSnapRect());

    // Hide cols C and D.
    uno::Sequence<beans::PropertyValue> aPropertyValues = {
        comphelper::makePropertyValue("ToPoint", OUString("$C$1:$D$1")),
    };
    dispatchCommand(xComponent, ".uno:GoToCell", aPropertyValues);

    ScTabViewShell* pViewShell = pDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT_MESSAGE("No ScTabViewShell", pViewShell);
    pViewShell->GetViewData().GetDispatcher().Execute(FID_COL_HIDE);

    // Check object is invisible
    CPPUNIT_ASSERT_MESSAGE("Hide: Object should be invisible", !pObj->IsVisible());

    // Show cols C and D
    aPropertyValues = {
        comphelper::makePropertyValue("ToPoint", OUString("$C$1:$D$1")),
    };
    dispatchCommand(xComponent, ".uno:GoToCell", aPropertyValues);
    pViewShell->GetViewData().GetDispatcher().Execute(FID_COL_SHOW);

    // Check object is visible and has old size
    CPPUNIT_ASSERT_MESSAGE("Show: Object should be visible", pObj->IsVisible());
    tools::Rectangle aSnapRectShow(pObj->GetSnapRect());
    lcl_AssertRectEqualWithTolerance("Show: Object geometry should not change", aSnapRectOrig,
                                     aSnapRectShow, 1);

    pDocSh->DoClose();
}

void ScShapeTest::testTdf138138_MoveCellWithRotatedShape()
{
    // The document contains a 90deg rotated, cell-anchored rectangle in column D. Insert 2 columns
    // after column B, save and reload. The shape was not correctly moved to column F.
    OUString aFileURL;
    createFileURL(u"tdf138138_MoveCellWithRotatedShape.ods", aFileURL);
    uno::Reference<css::lang::XComponent> xComponent = loadFromDesktop(aFileURL);
    CPPUNIT_ASSERT(xComponent.is());

    // Get ScDocShell
    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);
    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
    ScDocShell* pDocSh = dynamic_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(pDocSh);

    // Get document and object
    ScDocument& rDoc = pDocSh->GetDocument();
    ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
    CPPUNIT_ASSERT_MESSAGE("Load: No ScDrawLayer", pDrawLayer);
    const SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("Load: No draw page", pPage);
    SdrObject* pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT_MESSAGE("Load: custom shape not found", pObj);

    // Check anchor and position of shape. The expected values are taken from UI.
    tools::Rectangle aSnapRect = pObj->GetSnapRect();
    tools::Rectangle aExpectedRect(Point(10000, 3000), Size(1000, 7500));
    lcl_AssertRectEqualWithTolerance("Load original: ", aExpectedRect, aSnapRect, 1);

    // Insert two columns after column B
    uno::Sequence<beans::PropertyValue> aPropertyValues = {
        comphelper::makePropertyValue("ToPoint", OUString("$A$1:$B$1")),
    };
    dispatchCommand(xComponent, ".uno:GoToCell", aPropertyValues);
    ScTabViewShell* pViewShell = pDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT_MESSAGE("No ScTabViewShell", pViewShell);
    pViewShell->GetViewData().GetDispatcher().Execute(FID_INS_COLUMNS_AFTER);
    aExpectedRect = tools::Rectangle(Point(16000, 3000), Size(1000, 7500)); // col width 3000
    aSnapRect = pObj->GetSnapRect();
    lcl_AssertRectEqualWithTolerance("Shift: Wrong after insert of columns ", aExpectedRect,
                                     aSnapRect, 1);

    // Save and reload
    saveAndReload(xComponent, "calc8");
    CPPUNIT_ASSERT(xComponent);

    // Get ScDocShell
    pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);
    CPPUNIT_ASSERT_MESSAGE("Reload: Failed to access document shell", pFoundShell);
    pDocSh = dynamic_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(pDocSh);

    // Get document and object
    ScDocument& rDoc2 = pDocSh->GetDocument();
    pDrawLayer = rDoc2.GetDrawLayer();
    CPPUNIT_ASSERT_MESSAGE("Reload: No ScDrawLayer", pDrawLayer);
    pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("Reload: No draw page", pPage);
    pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT_MESSAGE("Reload: custom shape no longer exists", pObj);

    // Assert objects size is unchanged, position is shifted.
    aSnapRect = pObj->GetSnapRect();
    lcl_AssertRectEqualWithTolerance("Reload: Shape geometry has changed.", aExpectedRect,
                                     aSnapRect, 1);

    pDocSh->DoClose();
}

void ScShapeTest::testLoadVerticalFlip()
{
    // The document has a cell anchored custom shape with vertical flip. Error was, that the
    // flip was lost on loading.
    OUString aFileURL;
    createFileURL(u"loadVerticalFlip.ods", aFileURL);
    uno::Reference<css::lang::XComponent> xComponent = loadFromDesktop(aFileURL);
    CPPUNIT_ASSERT(xComponent.is());

    // Get the document model
    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);
    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
    ScDocShell* pDocSh = dynamic_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(pDocSh);

    // Get the shape and check that it is flipped
    ScDocument& rDoc = pDocSh->GetDocument();
    ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
    CPPUNIT_ASSERT(pDrawLayer);
    const SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT(pPage);
    SdrObjCustomShape* pObj = dynamic_cast<SdrObjCustomShape*>(pPage->GetObj(0));
    CPPUNIT_ASSERT(pObj);
    CPPUNIT_ASSERT_MESSAGE("Load: Object should be vertically flipped", pObj->IsMirroredY());

    pDocSh->DoClose();
}

void ScShapeTest::testTdf117948_CollapseBeforeShape()
{
    // The document contains a column group left from the image. The group is expanded. Collapse the
    // group, save and reload. The original error was, that the line was on wrong position after reload.
    // After the fix for 'resize with cell', the custom shape had wrong position and size too.
    OUString aFileURL;
    createFileURL(u"tdf117948_CollapseBeforeShape.ods", aFileURL);
    uno::Reference<css::lang::XComponent> xComponent = loadFromDesktop(aFileURL);
    CPPUNIT_ASSERT(xComponent.is());

    // Get ScDocShell
    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);
    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
    ScDocShell* pDocSh = dynamic_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(pDocSh);

    // Get document and objects
    ScDocument& rDoc = pDocSh->GetDocument();
    ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
    CPPUNIT_ASSERT_MESSAGE("Load: No ScDrawLayer", pDrawLayer);
    const SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("Load: No draw page", pPage);
    SdrObject* pObj0 = pPage->GetObj(0);
    CPPUNIT_ASSERT_MESSAGE("Load: custom shape not found", pObj0);
    SdrObject* pObj1 = pPage->GetObj(1);
    CPPUNIT_ASSERT_MESSAGE("Load: Vertical line not found", pObj1);

    // Collapse the group
    ScTabViewShell* pViewShell = pDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT_MESSAGE("Load: No ScTabViewShell", pViewShell);
    pViewShell->GetViewData().SetCurX(1);
    pViewShell->GetViewData().SetCurY(0);
    pViewShell->GetViewData().GetDispatcher().Execute(SID_OUTLINE_HIDE);

    // Check anchor and position of shape. The expected values are taken from UI before saving.
    tools::Rectangle aSnapRect0Collapse = pObj0->GetSnapRect();
    tools::Rectangle aExpectedRect0(Point(4672, 1334), Size(1787, 1723));
    lcl_AssertRectEqualWithTolerance("Collapse: Custom shape", aExpectedRect0, aSnapRect0Collapse,
                                     1);
    tools::Rectangle aSnapRect1Collapse = pObj1->GetSnapRect();
    tools::Rectangle aExpectedRect1(Point(5647, 4172), Size(21, 3441));
    lcl_AssertRectEqualWithTolerance("Collapse: Line", aExpectedRect1, aSnapRect1Collapse, 1);

    // Save and reload
    saveAndReload(xComponent, "calc8");
    CPPUNIT_ASSERT(xComponent);

    // Get ScDocShell
    pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);
    CPPUNIT_ASSERT_MESSAGE("Reload: Failed to access document shell", pFoundShell);
    pDocSh = dynamic_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(pDocSh);

    // Get document and objects
    ScDocument& rDoc2 = pDocSh->GetDocument();
    pDrawLayer = rDoc2.GetDrawLayer();
    CPPUNIT_ASSERT_MESSAGE("Reload: No ScDrawLayer", pDrawLayer);
    pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("Reload: No draw page", pPage);
    pObj0 = pPage->GetObj(0);
    CPPUNIT_ASSERT_MESSAGE("Reload: custom shape no longer exists", pObj0);
    pObj1 = pPage->GetObj(1);
    CPPUNIT_ASSERT_MESSAGE("Reload: custom shape no longer exists", pObj1);

    // Assert objects size and position are not changed. Actual values differ a little bit
    // because of cumulated Twips-Hmm conversion errors.
    tools::Rectangle aSnapRect0Reload = pObj0->GetSnapRect();
    lcl_AssertRectEqualWithTolerance("Reload: Custom shape geometry has changed.", aExpectedRect0,
                                     aSnapRect0Reload, 2);

    tools::Rectangle aSnapRect1Reload = pObj1->GetSnapRect();
    lcl_AssertRectEqualWithTolerance("Reload: Line geometry has changed.", aExpectedRect1,
                                     aSnapRect1Reload, 2);

    pDocSh->DoClose();
}

void ScShapeTest::testTdf137355_UndoHideRows()
{
    // The document contains a shape anchored "To Cell" with start in cell C3 and end in cell D6.
    // Error was, that hiding rows 3 to 6 and undo that action "lost" the shape.
    // Actually it was not lost but hidden.
    OUString aFileURL;
    createFileURL(u"tdf137355_UndoHideRows.ods", aFileURL);
    uno::Reference<css::lang::XComponent> xComponent = loadFromDesktop(aFileURL);
    CPPUNIT_ASSERT(xComponent.is());

    // Get the document model
    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);
    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
    ScDocShell* pDocSh = dynamic_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(pDocSh);

    // Get document and shape
    ScDocument& rDoc = pDocSh->GetDocument();
    ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
    CPPUNIT_ASSERT_MESSAGE("No ScDrawLayer", pDrawLayer);
    const SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("No draw page", pPage);
    SdrObject* pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT_MESSAGE("No object found", pObj);
    CPPUNIT_ASSERT_MESSAGE("Load: Object should be visible", pObj->IsVisible());
    tools::Rectangle aSnapRectOrig(pObj->GetSnapRect());

    // Hide rows 3 to 6 in UI. [Note: Simple rDoc.SetRowHidden(2,5,0,true) does not work, because it
    // does not produce the needed undo items.]
    uno::Sequence<beans::PropertyValue> aPropertyValues = {
        comphelper::makePropertyValue("ToPoint", OUString("$A$3:$A$6")),
    };
    dispatchCommand(xComponent, ".uno:GoToCell", aPropertyValues);

    ScTabViewShell* pViewShell = pDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT_MESSAGE("No ScTabViewShell", pViewShell);
    pViewShell->GetViewData().GetDispatcher().Execute(FID_ROW_HIDE);

    // Check object is invisible
    CPPUNIT_ASSERT_MESSAGE("Hide: Object should be invisible", !pObj->IsVisible());

    // Undo
    pViewShell->GetViewData().GetDispatcher().Execute(SID_UNDO);

    // Check object is visible and has old size
    CPPUNIT_ASSERT_MESSAGE("Undo: Object should exist", pObj);
    CPPUNIT_ASSERT_MESSAGE("Undo: Object should be visible", pObj->IsVisible());
    tools::Rectangle aSnapRectUndo(pObj->GetSnapRect());
    lcl_AssertRectEqualWithTolerance("Undo: Object geometry should not change", aSnapRectOrig,
                                     aSnapRectUndo, 1);

    pDocSh->DoClose();
}

void ScShapeTest::testTdf115655_HideDetail()
{
    // The document contains an image inside a cell anchored "To Cell (resize with cell)". The cell
    // belongs to a group. On loading the group is expanded.
    // Error was, that after collapsing the group, save and reload, and expanding the group, the image
    // was "lost". Actually is was resized to zero height.
    OUString aFileURL;
    createFileURL(u"tdf115655_HideDetail.ods", aFileURL);
    uno::Reference<css::lang::XComponent> xComponent = loadFromDesktop(aFileURL);
    CPPUNIT_ASSERT(xComponent.is());

    // Get ScDocShell
    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);
    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
    ScDocShell* pDocSh = dynamic_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(pDocSh);

    // Get document and image
    ScDocument& rDoc = pDocSh->GetDocument();
    ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
    CPPUNIT_ASSERT_MESSAGE("Load: No ScDrawLayer", pDrawLayer);
    const SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("Load: No draw page", pPage);
    SdrObject* pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT_MESSAGE("Load: No object found", pObj);

    // Get image size
    tools::Rectangle aSnapRectOrig = pObj->GetSnapRect();

    // Collapse the group
    ScTabViewShell* pViewShell = pDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT_MESSAGE("Load: No ScTabViewShell", pViewShell);
    pViewShell->GetViewData().SetCurX(0);
    pViewShell->GetViewData().SetCurY(1);
    pViewShell->GetViewData().GetDispatcher().Execute(SID_OUTLINE_HIDE);
    CPPUNIT_ASSERT_MESSAGE("Collapse: Image should not be visible", !pObj->IsVisible());

    // Save and reload
    saveAndReload(xComponent, "calc8");
    CPPUNIT_ASSERT(xComponent);

    // Get ScDocShell
    pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);
    CPPUNIT_ASSERT_MESSAGE("Reload: Failed to access document shell", pFoundShell);
    pDocSh = dynamic_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(pDocSh);

    // Get document and image
    ScDocument& rDoc2 = pDocSh->GetDocument();
    pDrawLayer = rDoc2.GetDrawLayer();
    CPPUNIT_ASSERT_MESSAGE("Reload: No ScDrawLayer", pDrawLayer);
    pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("Reload: No draw page", pPage);
    pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT_MESSAGE("Reload: Image no longer exists", pObj);

    // Expand the group
    pViewShell = pDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT_MESSAGE("Reload: No ScTabViewShell", pViewShell);
    pViewShell->GetViewData().SetCurX(0);
    pViewShell->GetViewData().SetCurY(1);
    pViewShell->GetViewData().GetDispatcher().Execute(SID_OUTLINE_SHOW);
    CPPUNIT_ASSERT_MESSAGE("Expand: Image should be visible", pObj->IsVisible());

    // Assert image size is not changed
    tools::Rectangle aSnapRectReload = pObj->GetSnapRect();
    lcl_AssertRectEqualWithTolerance("Reload: Object geometry has changed.", aSnapRectOrig,
                                     aSnapRectReload, 1);

    pDocSh->DoClose();
}

void ScShapeTest::testFitToCellSize()
{
    // The document has a cell anchored custom shape. Applying
    // FitToCellSize should resize and position the shape so,
    // that it fits into its anchor cell. That did not happened.
    OUString aFileURL;
    createFileURL(u"tdf119191_FitToCellSize.ods", aFileURL);
    uno::Reference<css::lang::XComponent> xComponent = loadFromDesktop(aFileURL);
    CPPUNIT_ASSERT(xComponent.is());

    // Get the document model
    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);
    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);

    ScDocShell* pDocSh = dynamic_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(pDocSh);

    // Get the shape
    ScDocument& rDoc = pDocSh->GetDocument();
    ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
    CPPUNIT_ASSERT(pDrawLayer);

    const SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT(pPage);

    SdrObjCustomShape* pObj = dynamic_cast<SdrObjCustomShape*>(pPage->GetObj(0));
    CPPUNIT_ASSERT(pObj);

    // Get the document controller
    ScTabViewShell* pViewShell = pDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT(pViewShell);

    // Get the draw view of the document
    ScDrawView* pDrawView = pViewShell->GetViewData().GetScDrawView();
    CPPUNIT_ASSERT(pDrawView);

    // Select the shape
    pDrawView->MarkNextObj();
    CPPUNIT_ASSERT(pDrawView->AreObjectsMarked());

    // Fit selected shape into cell
    pViewShell->GetViewData().GetDispatcher().Execute(SID_FITCELLSIZE);

    const tools::Rectangle& rShapeRect(pObj->GetSnapRect());
    const tools::Rectangle aCellRect = rDoc.GetMMRect(1, 1, 1, 1, 0);
    lcl_AssertRectEqualWithTolerance("Cell and SnapRect should be equal", aCellRect, rShapeRect, 1);

    pDocSh->DoClose();
}

void ScShapeTest::testCustomShapeCellAnchoredRotatedShape()
{
    // The example doc contains a cell anchored custom shape that is rotated
    // and sheared. Error was, that the shape lost position and size on
    // loading.
    OUString aFileURL;
    createFileURL(u"tdf119191_transformedShape.ods", aFileURL);
    uno::Reference<css::lang::XComponent> xComponent = loadFromDesktop(aFileURL);
    CPPUNIT_ASSERT(xComponent.is());

    // Get the document model
    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);
    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);

    ScDocShell* pDocSh = dynamic_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(pDocSh);

    // Get the shape
    ScDocument& rDoc = pDocSh->GetDocument();
    ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
    CPPUNIT_ASSERT(pDrawLayer);

    const SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT(pPage);

    SdrObjCustomShape* pObj = dynamic_cast<SdrObjCustomShape*>(pPage->GetObj(0));
    CPPUNIT_ASSERT(pObj);

    // Check Position and Size
    rDoc.SetDrawPageSize(0); // trigger recalcpos
    tools::Rectangle aRect(2400, 751, 5772, 3694); // expected snap rect from values in file
    const tools::Rectangle& rShapeRect(pObj->GetSnapRect());
    lcl_AssertRectEqualWithTolerance("Load: wrong pos and size", aRect, rShapeRect, 1);

    // Check anchor
    ScDrawObjData* pData = ScDrawLayer::GetObjData(pObj);
    CPPUNIT_ASSERT_MESSAGE("expected object meta data", pData);

    const OUString sActual("start col " + OUString::number(pData->maStart.Col()) + " row "
                           + OUString::number(pData->maStart.Row()) + " end col "
                           + OUString::number(pData->maEnd.Col()) + " row "
                           + OUString::number(pData->maEnd.Row()));
    CPPUNIT_ASSERT_EQUAL(OUString("start col 1 row 1 end col 2 row 8"), sActual);

    pDocSh->DoClose();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScShapeTest);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
