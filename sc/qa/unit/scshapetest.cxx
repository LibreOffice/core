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

#include "helper/qahelper.hxx"

#include <comphelper/propertyvalue.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/request.hxx>
#include <svl/intitem.hxx>
#include <svx/svdoashp.hxx>
#include <svx/svdomeas.hxx>
#include <svx/svdorect.hxx>
#include <svx/svdouno.hxx>
#include <svx/svdpage.hxx>
#include <unotools/tempfile.hxx>
#include <vcl/keycodes.hxx>

#include <docsh.hxx>
#include <drwlayer.hxx>
#include <fuconcustomshape.hxx>
#include <fuconuno.hxx>
#include <tabvwsh.hxx>
#include <userdat.hxx>

#include <sc.hrc> // defines of slot-IDs

using namespace css;

class ScShapeTest : public ScModelTestBase
{
public:
    ScShapeTest()
        : ScModelTestBase("sc/qa/unit/data")
    {
    }
};

static SdrPage* lcl_getSdrPageWithAssert(ScDocument& rDoc)
{
    ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
    CPPUNIT_ASSERT_MESSAGE("No ScDrawLayer", pDrawLayer);
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("No draw page", pPage);
    return pPage;
}

static SdrObject* lcl_getSdrObjectWithAssert(ScDocument& rDoc, sal_uInt16 nObjNumber)
{
    ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
    CPPUNIT_ASSERT_MESSAGE("No ScDrawLayer", pDrawLayer);
    const SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("No draw page", pPage);
    SdrObject* pObj = pPage->GetObj(nObjNumber);
    OString sMsg = "no Object " + OString::number(nObjNumber);
    CPPUNIT_ASSERT_MESSAGE(sMsg.getStr(), pObj);
    return pObj;
}

static SdrObject* lcl_getSdrObjectbyName(ScDocument& rDoc, std::u16string_view rName)
{
    ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
    const SdrPage* pPage = pDrawLayer->GetPage(0);
    SdrObject* pObj = pPage->GetObjByName(rName);
    return pObj;
}

CPPUNIT_TEST_FIXTURE(ScShapeTest, testTdf144242_OpenBezier_noSwapWH)
{
    // Shapes, which have rotation incorporated in their points, got erroneously width-height
    // swapped, because they report a rotation. (Rotation was introduced to align text with curve.)

    // Create a spreadsheet document with default row height and col width
    createScDoc();

    // Insert default open Bezier curve
    ScTabViewShell* pTabViewShell = getViewShell();
    SfxRequest aReq(pTabViewShell->GetViewFrame(), SID_DRAW_BEZIER_NOFILL);
    aReq.SetModifier(KEY_MOD1); // Ctrl
    pTabViewShell->ExecDraw(aReq);
    pTabViewShell->SetDrawShell(false);

    // Get document and newly created object
    ScDocument* pDoc = getScDoc();
    SdrObject* pObj = lcl_getSdrObjectWithAssert(*pDoc, 0);

    // Rotate object by 300deg
    pObj->Rotate(pObj->GetSnapRect().Center(), 30000_deg100, sin(toRadians(30000_deg100)),
                 cos(toRadians(30000_deg100)));
    tools::Rectangle aExpectRect(pObj->GetSnapRect());

    // Save, reload and compare
    saveAndReload("Calc Office Open XML");
    pDoc = getScDoc();
    pObj = lcl_getSdrObjectWithAssert(*pDoc, 0);
    tools::Rectangle aSnapRect(pObj->GetSnapRect());
    // Without fix in place width and height were swapped
    CPPUNIT_ASSERT_RECTANGLE_EQUAL_WITH_TOLERANCE(aExpectRect, aSnapRect, 40);
}

CPPUNIT_TEST_FIXTURE(ScShapeTest, testTdf144242_Line_noSwapWH)
{
    // Shapes, which have rotation incorporated in their points, got erroneously width-height
    // swapped, because they report a rotation. (Rotation was introduced to align text with line.)

    // Create a spreadsheet document with default row height and col width
    createScDoc();

    // Insert default line
    ScTabViewShell* pTabViewShell = getViewShell();
    SfxRequest aReq(pTabViewShell->GetViewFrame(), SID_DRAW_LINE);
    aReq.SetModifier(KEY_MOD1); // Ctrl
    pTabViewShell->ExecDraw(aReq);
    pTabViewShell->SetDrawShell(false);

    // Get document and newly created object
    ScDocument* pDoc = getScDoc();
    SdrObject* pObj = lcl_getSdrObjectWithAssert(*pDoc, 0);

    // Rotate object by 300deg
    pObj->Rotate(pObj->GetSnapRect().Center(), 30000_deg100, sin(toRadians(30000_deg100)),
                 cos(toRadians(30000_deg100)));
    tools::Rectangle aExpectRect(pObj->GetSnapRect());

    // Save, reload and compare
    saveAndReload("Calc Office Open XML");
    pDoc = getScDoc();
    pObj = lcl_getSdrObjectWithAssert(*pDoc, 0);
    tools::Rectangle aSnapRect(pObj->GetSnapRect());
    // Without fix in place width and height were swapped
    CPPUNIT_ASSERT_RECTANGLE_EQUAL_WITH_TOLERANCE(aExpectRect, aSnapRect, 40);
}

CPPUNIT_TEST_FIXTURE(ScShapeTest, testTdf143619_validation_circle_pos)
{
    // Load a document, which has validation circle around cell E6.
    createScDoc("ods/tdf143619_validationCirclePos.ods");

    // Get document
    ScDocument* pDoc = getScDoc();

    // Get shape. That is the validation circle.
    SdrObject* pObj = lcl_getSdrObjectWithAssert(*pDoc, 0);

    // Error was, that deleting row and col before E6 does not move circle to D5, but to B3.
    // Delete first row and first column.
    goToCell("$A$1");
    dispatchCommand(mxComponent, ".uno:DeleteRows", {});
    goToCell("$A$1");
    dispatchCommand(mxComponent, ".uno:DeleteColumns", {});

    // Without fix in place the position was (2007, 833)
    Point aPos = pObj->GetSnapRect().TopLeft();
    CPPUNIT_ASSERT_POINT_EQUAL_WITH_TOLERANCE(Point(6523, 1736), aPos, 1);
}

CPPUNIT_TEST_FIXTURE(ScShapeTest, testTdf140252_DragCreateFormControl)
{
    // Error was, that drag-created form controls were initially not on layer 'controls' and thus
    // other shapes could be placed in front of form controls.
    // Load an empty document.
    createScDoc("ods/ManualColWidthRowHeight.ods");

    // Get ScTabViewShell
    ScTabViewShell* pTabViewShell = getViewShell();

    // drag-create a push button as example of form control
    SfxUInt16Item aIdentifierItem(SID_FM_CONTROL_IDENTIFIER,
                                  static_cast<sal_uInt16>(SdrObjKind::FormButton));
    SfxUInt32Item aInventorItem(SID_FM_CONTROL_INVENTOR, sal_uInt32(SdrInventor::FmForm));
    const SfxPoolItem* pArgs[] = { &aIdentifierItem, &aInventorItem, nullptr };
    pTabViewShell->GetViewData().GetDispatcher().Execute(SID_FM_CREATE_CONTROL,
                                                         SfxCallMode::SYNCHRON, pArgs);
    // above includes creation of FuConstUnoControl and call of its Activate() method

    // get FuConstUnoControl
    ScTabView* pTabView = pTabViewShell->GetViewData().GetView();
    CPPUNIT_ASSERT(pTabView);
    FuConstUnoControl* pFuConstUC = static_cast<FuConstUnoControl*>(pTabView->GetDrawFuncPtr());
    CPPUNIT_ASSERT(pFuConstUC);

    // drag-create shape, points are in pixel
    MouseEvent aMouseEvent(Point(50, 100), 1, MouseEventModifiers::NONE, MOUSE_LEFT, 0);
    pFuConstUC->MouseButtonDown(aMouseEvent);
    aMouseEvent = MouseEvent(Point(200, 250), 1, MouseEventModifiers::DRAGMOVE, MOUSE_LEFT, 0);
    pFuConstUC->MouseMove(aMouseEvent);
    aMouseEvent = MouseEvent(Point(200, 250), 1, MouseEventModifiers::NONE, MOUSE_LEFT, 0);
    pFuConstUC->MouseButtonUp(aMouseEvent);
    pFuConstUC->Deactivate();
    pTabViewShell->SetDrawShell(false);

    // Get document and newly created push button.
    ScDocument* pDoc = getScDoc();
    SdrUnoObj* pObj = static_cast<SdrUnoObj*>(lcl_getSdrObjectWithAssert(*pDoc, 0));

    // Without the fix in place, the shape would be on layer SC_LAYER_FRONT (0)
    sal_Int16 nExpectedID = SC_LAYER_CONTROLS.get();
    sal_Int16 nActualID = pObj->GetLayer().get();
    CPPUNIT_ASSERT_EQUAL(nExpectedID, nActualID);
}

CPPUNIT_TEST_FIXTURE(ScShapeTest, testTdf134355_DragCreateCustomShape)
{
    // Error was, that drag-created custom shapes were initially on layer "controls", although that
    // layer is exclusively for form controls. Effect was, that other shapes could not be brought in
    // front of custom shapes.
    // Load an empty document.
    createScDoc("ods/ManualColWidthRowHeight.ods");

    // Get ScTabView
    ScTabViewShell* pTabViewShell = getViewShell();
    ScTabView* pTabView = pTabViewShell->GetViewData().GetView();

    // drag-create custom shape
    uno::Sequence<beans::PropertyValue> aPropertyValues = {
        comphelper::makePropertyValue("SymbolShapes", OUString("smiley")),
    };
    dispatchCommand(mxComponent, ".uno:SymbolShapes", aPropertyValues);
    // above includes creation of FuConstCustomShape and call of its Activate() method
    FuConstCustomShape* pFuConstCS = static_cast<FuConstCustomShape*>(pTabView->GetDrawFuncPtr());
    CPPUNIT_ASSERT(pFuConstCS);
    // points are in pixel
    MouseEvent aMouseEvent(Point(50, 100), 1, MouseEventModifiers::NONE, MOUSE_LEFT, 0);
    pFuConstCS->MouseButtonDown(aMouseEvent);
    aMouseEvent = MouseEvent(Point(200, 250), 1, MouseEventModifiers::DRAGMOVE, MOUSE_LEFT, 0);
    pFuConstCS->MouseMove(aMouseEvent);
    aMouseEvent = MouseEvent(Point(200, 250), 1, MouseEventModifiers::NONE, MOUSE_LEFT, 0);
    pFuConstCS->MouseButtonUp(aMouseEvent);
    pFuConstCS->Deactivate();
    pTabViewShell->SetDrawShell(false);

    // Get document and newly created custom shape.
    ScDocument* pDoc = getScDoc();
    SdrObjCustomShape* pObj = static_cast<SdrObjCustomShape*>(lcl_getSdrObjectWithAssert(*pDoc, 0));

    // Without the fix in place, the shape would be on layer SC_LAYER_CONTROLS (3)
    sal_Int16 nExpectedID = SC_LAYER_FRONT.get();
    sal_Int16 nActualID = pObj->GetLayer().get();
    CPPUNIT_ASSERT_EQUAL(nExpectedID, nActualID);
}

CPPUNIT_TEST_FIXTURE(ScShapeTest, testTdf140252_LayerOfControl)
{
    // Error was, that a newly inserted control shape was put on layer
    // "vorne" instead of layer "control".
    // Load an empty document.
    createScDoc("ods/ManualColWidthRowHeight.ods");

    // Create default push button
    SfxUInt16Item aIdentifierItem(SID_FM_CONTROL_IDENTIFIER,
                                  static_cast<sal_uInt16>(SdrObjKind::FormButton));
    SfxUInt32Item aInventorItem(SID_FM_CONTROL_INVENTOR, sal_uInt32(SdrInventor::FmForm));
    const SfxPoolItem* pArgs[] = { &aIdentifierItem, &aInventorItem, nullptr };
    const SfxPoolItem* pInternalArgs[] = { nullptr };
    ScTabViewShell* pTabViewShell = getViewShell();
    pTabViewShell->GetViewData().GetDispatcher().Execute(
        SID_FM_CREATE_CONTROL, SfxCallMode::SYNCHRON, pArgs, KEY_MOD1, pInternalArgs);

    // Get document and newly created push button.
    ScDocument* pDoc = getScDoc();
    SdrObject* pObj = lcl_getSdrObjectWithAssert(*pDoc, 0);

    // Check LayerID of object. Without the fix in place it was 0.
    sal_Int16 nExpectedID = SC_LAYER_CONTROLS.get();
    sal_Int16 nActualID = pObj->GetLayer().get();
    CPPUNIT_ASSERT_EQUAL(nExpectedID, nActualID);
}

CPPUNIT_TEST_FIXTURE(ScShapeTest, testTdf137082_LTR_to_RTL)
{
    // Before the fix for tdf137081 and tdf137082, when flipping sheet from LTR to RTL, page anchored
    // shapes were mirrored, but cell anchored shapes not. This was changed so, that shapes are always
    // mirrored. Graphics are still not mirrored but shifted. This test makes sure a shape is mirrored
    // and an image is not mirrored.

    createScDoc("ods/tdf137082_LTR_arrow_image.ods");

    // Get document
    ScDocument* pDoc = getScDoc();

    // Get objects and their transformation angles
    SdrObject* pObjCS = lcl_getSdrObjectWithAssert(*pDoc, 0);
    const Degree100 nRotateLTR = pObjCS->GetRotateAngle();
    SdrObject* pObjImage = lcl_getSdrObjectWithAssert(*pDoc, 1);
    const Degree100 nShearLTR = pObjImage->GetShearAngle();

    // Switch to RTL
    ScTabViewShell* pViewShell = getViewShell();
    pViewShell->GetViewData().GetDispatcher().Execute(FID_TAB_RTL);

    // Check custom shape is mirrored, image not.
    const Degree100 nShearRTLActual = pObjImage->GetShearAngle();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("image should not be mirrored", nShearLTR.get(),
                                 nShearRTLActual.get());
    const Degree100 nRotateRTLExpected = 36000_deg100 - nRotateLTR;
    const Degree100 nRotateRTLActual = pObjCS->GetRotateAngle();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("custom shape should be mirrored", nRotateRTLExpected.get(),
                                 nRotateRTLActual.get());
}

CPPUNIT_TEST_FIXTURE(ScShapeTest, testTdf137082_RTL_cell_anchored)
{
    // Error was, that cell anchored custom shapes wrote wrong offsets to file and thus were wrong on
    // reloading. The file contains one custom shape with "resize" and another one without.
    createScDoc("ods/tdf137082_RTL_cell_anchored.ods");

    // Get document
    ScDocument* pDoc = getScDoc();

    // Expected values.
    const Point aTopLeftA(-20500, 3500); // shape A without "resize"
    const Point aTopLeftB(-9500, 3500); // shape B with "resize"
    const Size aSize(2278, 5545); // both
    const tools::Rectangle aSnapRectA(aTopLeftA, aSize);
    const tools::Rectangle aSnapRectB(aTopLeftB, aSize);

    // Test reading was correct
    SdrObject* pObj = lcl_getSdrObjectWithAssert(*pDoc, 0);
    CPPUNIT_ASSERT_RECTANGLE_EQUAL_WITH_TOLERANCE(aSnapRectA, pObj->GetSnapRect(), 1);
    pObj = lcl_getSdrObjectWithAssert(*pDoc, 1);
    CPPUNIT_ASSERT_RECTANGLE_EQUAL_WITH_TOLERANCE(aSnapRectB, pObj->GetSnapRect(), 1);

    // Save and reload.
    saveAndReload("calc8");

    // Get document
    pDoc = getScDoc();

    // And test again
    pObj = lcl_getSdrObjectWithAssert(*pDoc, 0);
    CPPUNIT_ASSERT_RECTANGLE_EQUAL_WITH_TOLERANCE(aSnapRectA, pObj->GetSnapRect(), 1);
    pObj = lcl_getSdrObjectWithAssert(*pDoc, 1);
    CPPUNIT_ASSERT_RECTANGLE_EQUAL_WITH_TOLERANCE(aSnapRectB, pObj->GetSnapRect(), 1);
}

CPPUNIT_TEST_FIXTURE(ScShapeTest, testTdf137081_RTL_page_anchored)
{
    // Error was, that page anchored lines and custom shapes were mirrored on opening. The document
    // contains measure line, polyline and transformed custom shape.
    createScDoc("ods/tdf137081_RTL_page_anchored.ods");

    // Get document
    ScDocument* pDoc = getScDoc();

    // Expected values.
    // Measure line
    const Point aStart(-3998, 2490);
    const Point aEnd(-8488, 5490);
    // Polyline
    const Point aFirst(-10010, 2500);
    const Point aSecond(-14032, 5543);
    const Point aThird(-14500, 3500);
    // Custom shape
    const Point aTopLeft(-20500, 4583);

    // Test reading was correct
    SdrObject* pObj = lcl_getSdrObjectWithAssert(*pDoc, 0);
    // Measure line
    CPPUNIT_ASSERT_POINT_EQUAL_WITH_TOLERANCE(aStart, pObj->GetPoint(0), 1);
    CPPUNIT_ASSERT_POINT_EQUAL_WITH_TOLERANCE(aEnd, pObj->GetPoint(1), 1);
    // Polyline
    pObj = lcl_getSdrObjectWithAssert(*pDoc, 1);
    CPPUNIT_ASSERT_POINT_EQUAL_WITH_TOLERANCE(aFirst, pObj->GetPoint(0), 1);
    CPPUNIT_ASSERT_POINT_EQUAL_WITH_TOLERANCE(aSecond, pObj->GetPoint(1), 1);
    CPPUNIT_ASSERT_POINT_EQUAL_WITH_TOLERANCE(aThird, pObj->GetPoint(2), 1);
    //Custom shape
    SdrObjCustomShape* pObjCS
        = static_cast<SdrObjCustomShape*>(lcl_getSdrObjectWithAssert(*pDoc, 2));
    CPPUNIT_ASSERT(!pObjCS->IsMirroredX());
    CPPUNIT_ASSERT_POINT_EQUAL_WITH_TOLERANCE(aTopLeft, pObjCS->GetLogicRect().TopLeft(), 1);

    // Save and reload.
    saveAndReload("calc8");

    // Get document
    pDoc = getScDoc();

    // And test again
    pObj = lcl_getSdrObjectWithAssert(*pDoc, 0);
    // Measure line
    CPPUNIT_ASSERT_POINT_EQUAL_WITH_TOLERANCE(aStart, pObj->GetPoint(0), 1);
    CPPUNIT_ASSERT_POINT_EQUAL_WITH_TOLERANCE(aEnd, pObj->GetPoint(1), 1);
    // Polyline
    pObj = lcl_getSdrObjectWithAssert(*pDoc, 1);
    CPPUNIT_ASSERT_POINT_EQUAL_WITH_TOLERANCE(aFirst, pObj->GetPoint(0), 1);
    CPPUNIT_ASSERT_POINT_EQUAL_WITH_TOLERANCE(aSecond, pObj->GetPoint(1), 1);
    CPPUNIT_ASSERT_POINT_EQUAL_WITH_TOLERANCE(aThird, pObj->GetPoint(2), 1);
    //Custom shape
    pObjCS = static_cast<SdrObjCustomShape*>(lcl_getSdrObjectWithAssert(*pDoc, 2));
    CPPUNIT_ASSERT(!pObjCS->IsMirroredX());
    CPPUNIT_ASSERT_POINT_EQUAL_WITH_TOLERANCE(aTopLeft, pObjCS->GetLogicRect().TopLeft(), 1);
}

CPPUNIT_TEST_FIXTURE(ScShapeTest, testTdf139583_Rotate180deg)
{
    // Load an empty document.
    createScDoc("ods/ManualColWidthRowHeight.ods");

    // Get document and draw page
    ScDocument* pDoc = getScDoc();
    SdrPage* pPage = lcl_getSdrPageWithAssert(*pDoc);

    // Insert Shape
    const tools::Rectangle aRect(Point(3000, 4000), Size(5000, 2000));
    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
    CPPUNIT_ASSERT_MESSAGE("No ScDrawLayer", pDrawLayer);
    rtl::Reference<SdrRectObj> pObj = new SdrRectObj(*pDrawLayer, aRect);
    CPPUNIT_ASSERT_MESSAGE("Could not create rectangle", pObj);
    pPage->InsertObject(pObj.get());

    // Anchor "to cell (resize with cell)" and then rotate it by 180deg around center
    // The order is important here.
    ScDrawLayer::SetCellAnchoredFromPosition(*pObj, *pDoc, 0 /*SCTAB*/, true /*bResizeWithCell*/);
    pObj->Rotate(aRect.Center(), Degree100(18000), 0.0, -1.0);
    pObj.clear();

    // Save and reload.
    saveAndReload("calc8");

    // Get document and object
    pDoc = getScDoc();
    pObj = static_cast<SdrRectObj*>(lcl_getSdrObjectWithAssert(*pDoc, 0));

    //  Without the fix in place, the shape would have nearly zero size.
    CPPUNIT_ASSERT_RECTANGLE_EQUAL_WITH_TOLERANCE(aRect, pObj->GetSnapRect(), 1);
}

CPPUNIT_TEST_FIXTURE(ScShapeTest, testTdf137033_FlipHori_Resize)
{
    // Load a document, which has a rotated custom shape, which is horizontal flipped. Error was, that
    // if such shape was anchored "resize with cell", then after save and reload it was distorted.
    createScDoc("ods/tdf137033_FlipHoriRotCustomShape.ods");

    // Get document and shape
    ScDocument* pDoc = getScDoc();
    SdrObjCustomShape* pObj = static_cast<SdrObjCustomShape*>(lcl_getSdrObjectWithAssert(*pDoc, 0));

    // Verify shape is correctly loaded. Then set shape to "resize with cell".
    tools::Rectangle aSnapRect(pObj->GetSnapRect());
    const tools::Rectangle aExpectRect(Point(4998, 7000), Size(9644, 6723));
    CPPUNIT_ASSERT_RECTANGLE_EQUAL_WITH_TOLERANCE(aExpectRect, aSnapRect, 1);
    ScDrawLayer::SetCellAnchoredFromPosition(*pObj, *pDoc, 0 /*SCTAB*/, true /*bResizeWithCell*/);

    // Save and reload.
    saveAndReload("calc8");

    // Get document and shape
    pDoc = getScDoc();
    pObj = static_cast<SdrObjCustomShape*>(lcl_getSdrObjectWithAssert(*pDoc, 0));

    // Check shape has the original geometry, besides rounding and unit conversion errors
    aSnapRect = pObj->GetSnapRect();
    CPPUNIT_ASSERT_RECTANGLE_EQUAL_WITH_TOLERANCE(aExpectRect, aSnapRect, 1);
}

CPPUNIT_TEST_FIXTURE(ScShapeTest, testTdf137033_RotShear_ResizeHide)
{
    // For rotated or sheared shapes anchored "To Cell (resize with cell) hiding rows or columns will
    // not only change size but rotation and shear angle too. Error was, that not the original angles
    // of the full sized shape were written to file but the changed one.

    // Load a document, which has a rotated and sheared shape, anchored to cell with resize.
    createScDoc("ods/tdf137033_RotShearResizeAnchor.ods");

    // Get document
    ScDocument* pDoc = getScDoc();

    // Hide rows 4 and 5 (UI number), which are inside the shape and thus change shape geometry
    pDoc->SetRowHidden(3, 4, 0, true);
    pDoc->SetDrawPageSize(0); // trigger recalcpos, otherwise shapes are not changed

    // Get shape
    SdrObject* pObj = lcl_getSdrObjectWithAssert(*pDoc, 0);

    // Verify hiding has changed shape geometry as expected
    tools::Rectangle aSnapRect(pObj->GetSnapRect());
    Degree100 aRotateAngle(pObj->GetRotateAngle());
    Degree100 aShearAngle(pObj->GetShearAngle());
    // mathematical exact would be Point(3868, 4795), Size(9763, 1909)
    // current values as of LO 7.2
    const tools::Rectangle aExpectRect(Point(3875, 4796), Size(9760, 1911));
    const Degree100 aExpectRotateAngle(20925_deg100);
    const Degree100 aExpectShearAngle(-6570_deg100);
    CPPUNIT_ASSERT_MESSAGE("Hide rows, shear angle: ",
                           abs(aShearAngle - aExpectShearAngle) <= 1_deg100);
    CPPUNIT_ASSERT_MESSAGE("Hide rows, rotate angle: ",
                           abs(aRotateAngle - aExpectRotateAngle) <= 1_deg100);
    CPPUNIT_ASSERT_RECTANGLE_EQUAL_WITH_TOLERANCE(aExpectRect, aSnapRect, 1);

    // Save and reload.
    saveAndReload("calc8");

    // Get document and shape
    pDoc = getScDoc();
    pObj = lcl_getSdrObjectWithAssert(*pDoc, 0);

    // Check shape has the original geometry, besides heavy rounding and unit conversion errors
    aSnapRect = pObj->GetSnapRect();
    aRotateAngle = pObj->GetRotateAngle();
    aShearAngle = pObj->GetShearAngle();
    CPPUNIT_ASSERT_MESSAGE("Reload, shear angle: ",
                           abs(aShearAngle - aExpectShearAngle) <= 3_deg100);
    CPPUNIT_ASSERT_MESSAGE("Reload, rotate angle: ",
                           abs(aRotateAngle - aExpectRotateAngle) <= 3_deg100);
    CPPUNIT_ASSERT_RECTANGLE_EQUAL_WITH_TOLERANCE(aExpectRect, aSnapRect, 7);
}

CPPUNIT_TEST_FIXTURE(ScShapeTest, testTdf137033_RotShear_Hide)
{
    // Hiding row or columns affect cell anchored shape based on their snap rectangle. The first
    // attempt to fix lost position has used the logic rect instead. For rotated or sheared shape it
    // makes a difference.

    // Load a document, which has a rotated and sheared shape, anchored to cell, without resize.
    createScDoc("ods/tdf137033_RotShearCellAnchor.ods");

    // Get document
    ScDocument* pDoc = getScDoc();

    // Hide column C, which is left from logic rect, but right from left edge of snap rect
    pDoc->SetColHidden(2, 2, 0, true);
    pDoc->SetDrawPageSize(0); // trigger recalcpos, otherwise shapes are not changed

    // Save and reload.
    saveAndReload("calc8");

    // Get document and shape
    pDoc = getScDoc();
    SdrObject* pObj = lcl_getSdrObjectWithAssert(*pDoc, 0);

    // Check shape is visible. With the old version, the shape was moved to column C and
    // thus hidden on reload.
    CPPUNIT_ASSERT_MESSAGE("Reload: Shape has to be visible", pObj->IsVisible());
    // Verify position and size are unchanged besides rounding and unit conversion errors
    // Values are manually taken from shape before hiding column C.
    const tools::Rectangle aExpectRect(Point(4500, 3500), Size(15143, 5187));
    const tools::Rectangle aSnapRect = pObj->GetSnapRect();
    CPPUNIT_ASSERT_RECTANGLE_EQUAL_WITH_TOLERANCE(aExpectRect, aSnapRect, 1);
}

CPPUNIT_TEST_FIXTURE(ScShapeTest, testTdf137576_LogicRectInDefaultMeasureline)
{
    // Error was, that the empty logical rectangle of a default measure line (Ctrl+Click)
    // resulted in zeros in NonRotatedAnchor and a wrong position when reloading.

    // Load an empty document.
    createScDoc("ods/ManualColWidthRowHeight.ods");

    // Create default measureline by SfxRequest that corresponds to Ctrl+Click
    ScTabViewShell* pTabViewShell = getViewShell();
    SfxRequest aReq(pTabViewShell->GetViewFrame(), SID_DRAW_MEASURELINE);
    aReq.SetModifier(KEY_MOD1); // Ctrl
    pTabViewShell->ExecDraw(aReq);

    // Get document and newly created measure line.
    ScDocument* pDoc = getScDoc();
    SdrObject* pObj = lcl_getSdrObjectWithAssert(*pDoc, 0);

    // Anchor "to Cell (resize with cell)"
    ScDrawLayer::SetCellAnchoredFromPosition(*pObj, *pDoc, 0 /*SCTAB*/, true /*bResizeWithCell*/);
    // Deselect shape and switch to object selection type "Cell".
    pTabViewShell->SetDrawShell(false);

    // Hide column A.
    goToCell("$A$1");
    dispatchCommand(mxComponent, ".uno:HideColumn", {});

    // Get current position. I will not use absolute values for comparison, because document is loaded
    // in full screen mode of unknown size and default object is placed in center of window.
    Point aOldPos = pObj->GetRelativePos();

    // Save and reload, get ScDocShell
    saveAndReload("calc8");

    // Get document and object
    pDoc = getScDoc();
    pObj = lcl_getSdrObjectWithAssert(*pDoc, 0);

    // Assert object position is unchanged, besides Twips<->Hmm inaccuracy.
    Point aNewPos = pObj->GetRelativePos();
    CPPUNIT_ASSERT_POINT_EQUAL_WITH_TOLERANCE(aOldPos, aNewPos, 1);
}

CPPUNIT_TEST_FIXTURE(ScShapeTest, testTdf137576_LogicRectInNewMeasureline)
{
    // Error was, that a new measure line had no logical rectangle. This resulted in zeros in
    // NonRotatedAnchor. As a result the position was wrong when reloading.

    // Load an empty document
    createScDoc("ods/ManualColWidthRowHeight.ods");

    // Get document and draw page
    ScDocument* pDoc = getScDoc();
    SdrPage* pPage = lcl_getSdrPageWithAssert(*pDoc);

    // Create a new measure line and insert it
    Point aStartPoint(5000, 5500);
    Point aEndPoint(13000, 8000);
    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
    CPPUNIT_ASSERT_MESSAGE("No ScDrawLayer", pDrawLayer);
    rtl::Reference<SdrMeasureObj> pObj = new SdrMeasureObj(*pDrawLayer, aStartPoint, aEndPoint);
    CPPUNIT_ASSERT_MESSAGE("Could not create measure line", pObj);
    pPage->InsertObject(pObj.get());

    // Anchor "to cell (resize with cell)" and examine NonRotatedAnchor
    ScDrawLayer::SetCellAnchoredFromPosition(*pObj, *pDoc, 0 /*SCTAB*/, true /*bResizeWithCell*/);
    ScDrawObjData* pNData = ScDrawLayer::GetNonRotatedObjData(pObj.get());
    CPPUNIT_ASSERT_MESSAGE("Failed to get NonRotatedAnchor", pNData);
    // Without the fix all four values would be zero.
    CPPUNIT_ASSERT_EQUAL(SCCOL(1), pNData->maStart.Col());
    CPPUNIT_ASSERT_EQUAL(SCROW(2), pNData->maStart.Row());
    CPPUNIT_ASSERT_EQUAL(SCCOL(7), pNData->maEnd.Col());
    CPPUNIT_ASSERT_EQUAL(SCROW(2), pNData->maEnd.Row());

    pObj.clear();
}

CPPUNIT_TEST_FIXTURE(ScShapeTest, testMeasurelineHideColSave)
{
    // The document contains a SdrMeasureObj anchored "To Cell (resize with cell)" with start in cell
    // D11 and end in cell I5. Error was, that after hiding col A and saving, start and end point
    // position were lost.
    createScDoc("ods/measurelineHideColSave.ods");

    // Get document and shape
    ScDocument* pDoc = getScDoc();
    SdrObject* pObj = lcl_getSdrObjectWithAssert(*pDoc, 0);

    // Make sure loading is correct
    Point aStartPoint(7500, 15000); // according UI
    Point aEndPoint(17500, 8000);
    CPPUNIT_ASSERT_POINT_EQUAL_WITH_TOLERANCE(aStartPoint, pObj->GetPoint(0), 1);
    CPPUNIT_ASSERT_POINT_EQUAL_WITH_TOLERANCE(aEndPoint, pObj->GetPoint(1), 1);

    // Hide column A
    pDoc->SetColHidden(0, 0, 0, true);
    pDoc->SetDrawPageSize(0); // trigger recalcpos, otherwise shapes are not changed
    // Shape should move by column width, here 3000
    aStartPoint.Move(-3000, 0);
    aEndPoint.Move(-3000, 0);
    CPPUNIT_ASSERT_POINT_EQUAL_WITH_TOLERANCE(aStartPoint, pObj->GetPoint(0), 1);
    CPPUNIT_ASSERT_POINT_EQUAL_WITH_TOLERANCE(aEndPoint, pObj->GetPoint(1), 1);

    // save and reload
    saveAndReload("calc8");

    // Get document and shape
    pDoc = getScDoc();
    pObj = lcl_getSdrObjectWithAssert(*pDoc, 0);

    // Check that start and end point are unchanged besides rounding and unit conversion errors
    CPPUNIT_ASSERT_POINT_EQUAL_WITH_TOLERANCE(aStartPoint, pObj->GetPoint(0), 2);
    CPPUNIT_ASSERT_POINT_EQUAL_WITH_TOLERANCE(aEndPoint, pObj->GetPoint(1), 2);
}

CPPUNIT_TEST_FIXTURE(ScShapeTest, testHideColsShow)
{
    // The document contains a shape anchored "To Cell (resize with cell)" with starts in cell C3 and
    //ends in cell D5. Error was, that hiding cols C and D and then show them again extends the shape
    // to column E

    createScDoc("ods/hideColsShow.ods");

    // Get document and shape
    ScDocument* pDoc = getScDoc();
    SdrObjCustomShape* pObj = static_cast<SdrObjCustomShape*>(lcl_getSdrObjectWithAssert(*pDoc, 0));

    CPPUNIT_ASSERT_MESSAGE("Load: Object should be visible", pObj->IsVisible());
    tools::Rectangle aSnapRectOrig(pObj->GetSnapRect());

    // Hide cols C and D.
    goToCell("$C$1:$D$1");

    ScTabViewShell* pViewShell = getViewShell();
    pViewShell->GetViewData().GetDispatcher().Execute(FID_COL_HIDE);

    // Check object is invisible
    CPPUNIT_ASSERT_MESSAGE("Hide: Object should be invisible", !pObj->IsVisible());

    // Show cols C and D
    goToCell("$C$1:$D$1");
    pViewShell->GetViewData().GetDispatcher().Execute(FID_COL_SHOW);

    // Check object is visible and has old size
    CPPUNIT_ASSERT_MESSAGE("Show: Object should be visible", pObj->IsVisible());
    tools::Rectangle aSnapRectShow(pObj->GetSnapRect());
    CPPUNIT_ASSERT_RECTANGLE_EQUAL_WITH_TOLERANCE(aSnapRectOrig, aSnapRectShow, 1);
}

CPPUNIT_TEST_FIXTURE(ScShapeTest, testFormSizeWithHiddenCol)
{
    // The document contains a form (Listbox) shape anchored "To Cell (resize with cell)" with starts in cell B5 and
    // ends in cell D5. The error was the form shape was resized if there was hidden col/row.

    createScDoc("ods/tdf154005.ods");

    // Get document and shape
    ScDocument* pDoc = getScDoc();
    SdrUnoObj* pObj = static_cast<SdrUnoObj*>(lcl_getSdrObjectWithAssert(*pDoc, 0));

    // Check Position and Size
    pDoc->SetDrawPageSize(0); // trigger recalcpos
    tools::Rectangle aRect(2432, 3981, 4932, 4631); // expected snap rect from values in file
    const tools::Rectangle& rShapeRect(pObj->GetSnapRect());
    CPPUNIT_ASSERT_RECTANGLE_EQUAL_WITH_TOLERANCE(aRect, rShapeRect, 1);

    // Check anchor
    ScDrawObjData* pData = ScDrawLayer::GetObjData(pObj);
    CPPUNIT_ASSERT_MESSAGE("expected object meta data", pData);

    const OUString sActual("start col " + OUString::number(pData->maStart.Col()) + " row "
                           + OUString::number(pData->maStart.Row()) + " end col "
                           + OUString::number(pData->maEnd.Col()) + " row "
                           + OUString::number(pData->maEnd.Row()));
    CPPUNIT_ASSERT_EQUAL(OUString("start col 1 row 4 end col 3 row 4"), sActual);
}

CPPUNIT_TEST_FIXTURE(ScShapeTest, testTdf138138_MoveCellWithRotatedShape)
{
    // The document contains a 90deg rotated, cell-anchored rectangle in column D. Insert 2 columns
    // after column B, save and reload. The shape was not correctly moved to column F.
    createScDoc("ods/tdf138138_MoveCellWithRotatedShape.ods");

    // Get document and shape
    ScDocument* pDoc = getScDoc();
    SdrObject* pObj = lcl_getSdrObjectWithAssert(*pDoc, 0);

    // Check anchor and position of shape. The expected values are taken from UI.
    tools::Rectangle aSnapRect = pObj->GetSnapRect();
    tools::Rectangle aExpectedRect(Point(10000, 3000), Size(1000, 7500));
    CPPUNIT_ASSERT_RECTANGLE_EQUAL_WITH_TOLERANCE(aExpectedRect, aSnapRect, 1);

    // Insert two columns after column B
    goToCell("$A$1:$B$1");

    ScTabViewShell* pViewShell = getViewShell();
    pViewShell->GetViewData().GetDispatcher().Execute(FID_INS_COLUMNS_AFTER);
    aExpectedRect = tools::Rectangle(Point(16000, 3000), Size(1000, 7500)); // col width 3000
    aSnapRect = pObj->GetSnapRect();
    CPPUNIT_ASSERT_RECTANGLE_EQUAL_WITH_TOLERANCE(aExpectedRect, aSnapRect, 1);

    // Save and reload
    saveAndReload("calc8");

    // Get document and shape
    pDoc = getScDoc();
    pObj = lcl_getSdrObjectWithAssert(*pDoc, 0);

    // Assert objects size is unchanged, position is shifted.
    aSnapRect = pObj->GetSnapRect();
    CPPUNIT_ASSERT_RECTANGLE_EQUAL_WITH_TOLERANCE(aExpectedRect, aSnapRect, 1);
}

CPPUNIT_TEST_FIXTURE(ScShapeTest, testLoadVerticalFlip)
{
    // The document has a cell anchored custom shape with vertical flip. Error was, that the
    // flip was lost on loading.
    createScDoc("ods/loadVerticalFlip.ods");

    // Get document and shape
    ScDocument* pDoc = getScDoc();
    SdrObjCustomShape* pObj = static_cast<SdrObjCustomShape*>(lcl_getSdrObjectWithAssert(*pDoc, 0));

    // Check that shape is flipped
    CPPUNIT_ASSERT_MESSAGE("Load: Object should be vertically flipped", pObj->IsMirroredY());
}

CPPUNIT_TEST_FIXTURE(ScShapeTest, testTdf117948_CollapseBeforeShape)
{
    // The document contains a column group left from the image. The group is expanded. Collapse the
    // group, save and reload. The original error was, that the line was on wrong position after reload.
    // After the fix for 'resize with cell', the custom shape had wrong position and size too.
    createScDoc("ods/tdf117948_CollapseBeforeShape.ods");

    // Get document and objects
    ScDocument* pDoc = getScDoc();
    SdrObject* pObj0 = lcl_getSdrObjectWithAssert(*pDoc, 0);
    SdrObject* pObj1 = lcl_getSdrObjectWithAssert(*pDoc, 1);

    // Collapse the group
    ScTabViewShell* pViewShell = getViewShell();
    pViewShell->GetViewData().SetCurX(1);
    pViewShell->GetViewData().SetCurY(0);
    pViewShell->GetViewData().GetDispatcher().Execute(SID_OUTLINE_HIDE);

    // Check anchor and position of shape. The expected values are taken from UI before saving.
    tools::Rectangle aSnapRect0Collapse = pObj0->GetSnapRect();
    tools::Rectangle aExpectedRect0(Point(4672, 1334), Size(1787, 1723));
    CPPUNIT_ASSERT_RECTANGLE_EQUAL_WITH_TOLERANCE(aExpectedRect0, aSnapRect0Collapse, 1);
    tools::Rectangle aSnapRect1Collapse = pObj1->GetSnapRect();
    tools::Rectangle aExpectedRect1(Point(5647, 4172), Size(21, 3441));
    CPPUNIT_ASSERT_RECTANGLE_EQUAL_WITH_TOLERANCE(aExpectedRect1, aSnapRect1Collapse, 1);

    // Save and reload
    saveAndReload("calc8");

    // Get document and objects
    pDoc = getScDoc();
    pObj0 = lcl_getSdrObjectWithAssert(*pDoc, 0);
    pObj1 = lcl_getSdrObjectWithAssert(*pDoc, 1);

    // Assert objects size and position are not changed. Actual values differ a little bit
    // because of cumulated Twips-Hmm conversion errors.
    tools::Rectangle aSnapRect0Reload = pObj0->GetSnapRect();
    CPPUNIT_ASSERT_RECTANGLE_EQUAL_WITH_TOLERANCE(aExpectedRect0, aSnapRect0Reload, 2);

    tools::Rectangle aSnapRect1Reload = pObj1->GetSnapRect();
    CPPUNIT_ASSERT_RECTANGLE_EQUAL_WITH_TOLERANCE(aExpectedRect1, aSnapRect1Reload, 2);
}

CPPUNIT_TEST_FIXTURE(ScShapeTest, testTdf137355_UndoHideRows)
{
    // The document contains a shape anchored "To Cell" with start in cell C3 and end in cell D6.
    // Error was, that hiding rows 3 to 6 and undo that action "lost" the shape.
    // Actually it was not lost but hidden.
    createScDoc("ods/tdf137355_UndoHideRows.ods");

    // Get document and shape
    ScDocument* pDoc = getScDoc();
    SdrObject* pObj = lcl_getSdrObjectWithAssert(*pDoc, 0);

    CPPUNIT_ASSERT_MESSAGE("Load: Object should be visible", pObj->IsVisible());
    tools::Rectangle aSnapRectOrig(pObj->GetSnapRect());

    // Hide rows 3 to 6 in UI. [Note: Simple pDoc->SetRowHidden(2,5,0,true) does not work, because it
    // does not produce the needed undo items.]
    goToCell("$A$3:$A$6");
    ScTabViewShell* pViewShell = getViewShell();
    pViewShell->GetViewData().GetDispatcher().Execute(FID_ROW_HIDE);

    // Check object is invisible
    CPPUNIT_ASSERT_MESSAGE("Hide: Object should be invisible", !pObj->IsVisible());

    // Undo
    pViewShell->GetViewData().GetDispatcher().Execute(SID_UNDO);

    // Check object is visible and has old size
    CPPUNIT_ASSERT_MESSAGE("Undo: Object should exist", pObj);
    CPPUNIT_ASSERT_MESSAGE("Undo: Object should be visible", pObj->IsVisible());
    tools::Rectangle aSnapRectUndo(pObj->GetSnapRect());
    CPPUNIT_ASSERT_RECTANGLE_EQUAL_WITH_TOLERANCE(aSnapRectOrig, aSnapRectUndo, 1);
}

CPPUNIT_TEST_FIXTURE(ScShapeTest, testTdf152081_UndoHideColsWithNotes)
{
    createScDoc("ods/tdf152081_UndoHideColsWithNotes.ods");

    // Get document and shape
    ScDocument* pDoc = getScDoc();
    SdrObject* pObj = lcl_getSdrObjectWithAssert(*pDoc, 0);

    CPPUNIT_ASSERT_MESSAGE("Load: Note object should be visible", pObj->IsVisible());

    // Hide B column
    goToCell("$B$2:$B$2");
    ScTabViewShell* pViewShell = getViewShell();
    pViewShell->GetViewData().GetDispatcher().Execute(FID_COL_HIDE);

    // Check object is invisible
    CPPUNIT_ASSERT_MESSAGE("Hide: Note object should be invisible", !pObj->IsVisible());

    // Undo
    pViewShell->GetViewData().GetDispatcher().Execute(SID_UNDO);

    // Check object is visible
    CPPUNIT_ASSERT_MESSAGE("Undo: Note object should exist", pObj);
    CPPUNIT_ASSERT_MESSAGE("Undo: Note object should be visible", pObj->IsVisible());
}

CPPUNIT_TEST_FIXTURE(ScShapeTest, testTdf115655_HideDetail)
{
    // The document contains an image inside a cell anchored "To Cell (resize with cell)". The cell
    // belongs to a group. On loading the group is expanded.
    // Error was, that after collapsing the group, save and reload, and expanding the group, the image
    // was "lost". Actually is was resized to zero height.
    createScDoc("ods/tdf115655_HideDetail.ods");

    // Get document and image
    ScDocument* pDoc = getScDoc();
    SdrObject* pObj = lcl_getSdrObjectWithAssert(*pDoc, 0);

    // Get image size
    tools::Rectangle aSnapRectOrig = pObj->GetSnapRect();

    // Collapse the group
    ScTabViewShell* pViewShell = getViewShell();
    pViewShell->GetViewData().SetCurX(0);
    pViewShell->GetViewData().SetCurY(1);
    pViewShell->GetViewData().GetDispatcher().Execute(SID_OUTLINE_HIDE);
    CPPUNIT_ASSERT_MESSAGE("Collapse: Image should not be visible", !pObj->IsVisible());

    // Save and reload
    saveAndReload("calc8");

    // Get document and image
    pDoc = getScDoc();
    pObj = lcl_getSdrObjectWithAssert(*pDoc, 0);

    // Expand the group
    pViewShell = getViewShell();
    pViewShell->GetViewData().SetCurX(0);
    pViewShell->GetViewData().SetCurY(1);
    pViewShell->GetViewData().GetDispatcher().Execute(SID_OUTLINE_SHOW);
    CPPUNIT_ASSERT_MESSAGE("Expand: Image should be visible", pObj->IsVisible());

    // Assert image size is not changed
    tools::Rectangle aSnapRectReload = pObj->GetSnapRect();
    CPPUNIT_ASSERT_RECTANGLE_EQUAL_WITH_TOLERANCE(aSnapRectOrig, aSnapRectReload, 1);
}

CPPUNIT_TEST_FIXTURE(ScShapeTest, testFitToCellSize)
{
    // The document has a cell anchored custom shape. Applying
    // FitToCellSize should resize and position the shape so,
    // that it fits into its anchor cell. That did not happened.
    createScDoc("ods/tdf119191_FitToCellSize.ods");

    // Get document and shape
    ScDocument* pDoc = getScDoc();
    SdrObjCustomShape* pObj
        = dynamic_cast<SdrObjCustomShape*>(lcl_getSdrObjectWithAssert(*pDoc, 0));

    // Get the draw view of the document
    ScTabViewShell* pViewShell = getViewShell();
    ScDrawView* pDrawView = pViewShell->GetViewData().GetScDrawView();
    CPPUNIT_ASSERT(pDrawView);

    // Select the shape
    pDrawView->MarkNextObj();
    CPPUNIT_ASSERT(pDrawView->AreObjectsMarked());

    // Fit selected shape into cell
    pViewShell->GetViewData().GetDispatcher().Execute(SID_FITCELLSIZE);

    const tools::Rectangle& rShapeRect(pObj->GetSnapRect());
    const tools::Rectangle aCellRect = pDoc->GetMMRect(1, 1, 1, 1, 0);
    CPPUNIT_ASSERT_RECTANGLE_EQUAL_WITH_TOLERANCE(aCellRect, rShapeRect, 2);
}

CPPUNIT_TEST_FIXTURE(ScShapeTest, testCustomShapeCellAnchoredRotatedShape)
{
    // The example doc contains a cell anchored custom shape that is rotated
    // and sheared. Error was, that the shape lost position and size on
    // loading.
    createScDoc("ods/tdf119191_transformedShape.ods");

    // Get document and shape
    ScDocument* pDoc = getScDoc();
    SdrObjCustomShape* pObj
        = dynamic_cast<SdrObjCustomShape*>(lcl_getSdrObjectWithAssert(*pDoc, 0));

    // Check Position and Size
    pDoc->SetDrawPageSize(0); // trigger recalcpos
    tools::Rectangle aRect(2400, 751, 5772, 3694); // expected snap rect from values in file
    const tools::Rectangle& rShapeRect(pObj->GetSnapRect());
    CPPUNIT_ASSERT_RECTANGLE_EQUAL_WITH_TOLERANCE(aRect, rShapeRect, 1);

    // Check anchor
    ScDrawObjData* pData = ScDrawLayer::GetObjData(pObj);
    CPPUNIT_ASSERT_MESSAGE("expected object meta data", pData);

    const OUString sActual("start col " + OUString::number(pData->maStart.Col()) + " row "
                           + OUString::number(pData->maStart.Row()) + " end col "
                           + OUString::number(pData->maEnd.Col()) + " row "
                           + OUString::number(pData->maEnd.Row()));
    CPPUNIT_ASSERT_EQUAL(OUString("start col 1 row 1 end col 2 row 8"), sActual);
}

CPPUNIT_TEST_FIXTURE(ScShapeTest, testLargeAnchorOffset)
{
    // The example doc contains a resize-with-cell-anchored measure line
    // with a large vertical offset that shifts the start point onto the
    // next cell below.
    createScDoc("ods/LargeAnchorOffset.ods");

    ScDocument* pDoc = getScDoc();
    SdrObject* pObj = lcl_getSdrObjectWithAssert(*pDoc, 0);

    const Point aOldPos = pObj->GetRelativePos();
    // Just to check that it imports correctly
    CPPUNIT_ASSERT_POINT_EQUAL_WITH_TOLERANCE(Point(9504, 9089), aOldPos, 1);

    saveAndReload("calc8");

    pDoc = getScDoc();
    pObj = lcl_getSdrObjectWithAssert(*pDoc, 0);

    // Without the fix, this would fail:
    //   Test name: sc_apitest::ScShapeTest::testLargeAnchorOffset
    //   assertion failed
    //   - Expression: std::abs(rExpected.Y() - rActual.Y()) <= nTolerance
    //   - after reload Y expected 9089 actual 9643 Tolerance 1
    const Point aNewPos = pObj->GetRelativePos();
    CPPUNIT_ASSERT_POINT_EQUAL_WITH_TOLERANCE(aOldPos, aNewPos, 1);
}

CPPUNIT_TEST_FIXTURE(ScShapeTest, testTdf139083_copy_without_resize)
{
    // Load a document, which has a shape anchored to cell B2, but without 'resize with cell'.
    // When the range B2:B3 is copied and pasted to D5, then the copied shape should keep its size.
    createScDoc("ods/tdf139083_copy_without_resize.ods");

    // Get document
    ScDocument* pDoc = getScDoc();

    // Copy cells B2:B3. They have row height 2cm and column width 3cm.
    goToCell("$B$2:$B$3");
    dispatchCommand(mxComponent, ".uno:Copy", {});

    // Paste to D5. There are row height 0.5cm and column width 1cm.
    goToCell("$D$5");
    dispatchCommand(mxComponent, ".uno:Paste", {});

    // Make sure original and pasted shape have the same size.
    // Size of original shape is 2001x3002, without fix size of pasted shape was 668x750.
    SdrObject* pObjOrig = lcl_getSdrObjectWithAssert(*pDoc, 0); // original shape
    SdrObject* pObjPasted = lcl_getSdrObjectWithAssert(*pDoc, 1); // pasted shape
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2001, pObjOrig->GetSnapRect().GetWidth(), 1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3002, pObjOrig->GetSnapRect().GetHeight(), 1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2001, pObjPasted->GetSnapRect().GetWidth(), 1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3002, pObjPasted->GetSnapRect().GetHeight(), 1);
}

CPPUNIT_TEST_FIXTURE(ScShapeTest, testTdf155093_double_names)
{
    // Load a document, which has a shape in range B6:C14 with name "myArrow". When the range was
    // copied and pasted, then the copied shape got the same name and thus was not accessible with
    // Navigator.
    createScDoc("ods/tdf155093_double_names.ods");
    ScDocument* pDoc = getScDoc();

    // Copy and paste
    goToCell("$B$6:$C$14");
    dispatchCommand(mxComponent, ".uno:Copy", {});
    goToCell("$D$16");
    dispatchCommand(mxComponent, ".uno:Paste", {});

    // Make sure original and pasted shape have different names.
    SdrObject* pObjOrig = lcl_getSdrObjectWithAssert(*pDoc, 0); // original shape
    SdrObject* pObjPasted = lcl_getSdrObjectWithAssert(*pDoc, 1); // pasted shape
    CPPUNIT_ASSERT(pObjOrig->GetName() != pObjPasted->GetName());
}

CPPUNIT_TEST_FIXTURE(ScShapeTest, testTdf155095_shape_collapsed_group)
{
    // Load a document, which has a shape in range C9:C16, anchored 'To cell (resize with cell)'.
    // The rows 11 to 14 are in a collapsed group. So the shape effectively spans 4 rows. When
    // copying the range B5:C19 and pasting it to B22, the group is expanded and the shape should
    // increase its height so that it spans 8 rows.
    createScDoc("ods/tdf155095_shape_over_collapsed_group.ods");
    ScDocument* pDoc = getScDoc();

    // Copy and paste
    goToCell("$B$5:$C$19");
    dispatchCommand(mxComponent, ".uno:Copy", {});
    goToCell("$B$22");
    dispatchCommand(mxComponent, ".uno:Paste", {});

    // Make sure the shape has the correct size and spans C26:C33
    SdrObject* pObj = lcl_getSdrObjectWithAssert(*pDoc, 1); // pasted shape
    // Without fix the shape had position(6708,11564) and size(407,2013).
    tools::Rectangle aExpectedRect(tools::Rectangle(Point(6708, 10743), Size(407, 3473)));
    tools::Rectangle aSnapRect(pObj->GetSnapRect());
    CPPUNIT_ASSERT_RECTANGLE_EQUAL_WITH_TOLERANCE(aExpectedRect, aSnapRect, 1);

    // Without fix the shape spans C28:C32
    ScDrawObjData* pObjData = ScDrawLayer::GetObjData(pObj);
    ScAddress aExpectedStart(SCCOL(2), SCROW(25), SCTAB(0)); // zero based
    ScAddress aExpectedEnd(SCCOL(2), SCROW(32), SCTAB(0));
    CPPUNIT_ASSERT_EQUAL(aExpectedStart, (*pObjData).maStart);
    CPPUNIT_ASSERT_EQUAL(aExpectedEnd, (*pObjData).maEnd);
}

CPPUNIT_TEST_FIXTURE(ScShapeTest, testTdf155094_paste_transposed)
{
    // Load a document, which has a page anchored shape "Red" in C4, a cell anchored shape "Green" in
    // D4 and a cell anchored shape "Blue" with 'resize with cell' in E4. The range C3:E5 is copied
    // and pasted with 'Transpose all' to cell K6. The pasted content had these errors:
    // Pasted shape "Red" was missing.
    // Pasted shape "Green" was resized although 'resize with cell' was not set.
    // Pasted shape "Blue" was in cell K5 instead of L8.
    // The behavior of paste transposed is changed since LO 7.6 so that no shape is resized.
    createScDoc("ods/tdf155094_paste_transposed.ods");
    ScDocument* pDoc = getScDoc();

    // Copy and paste
    goToCell("$C$3:$E$5");
    dispatchCommand(mxComponent, ".uno:Copy", {});
    goToCell("$K$6");
    uno::Sequence<beans::PropertyValue> aPropertyValues
        = { comphelper::makePropertyValue("Flags", OUString("A")),
            comphelper::makePropertyValue("FormulaCommand", sal_uInt16(0)),
            comphelper::makePropertyValue("SkipEmptyCells", false),
            comphelper::makePropertyValue("Transpose", true),
            comphelper::makePropertyValue("AsLink", false),
            comphelper::makePropertyValue("MoveMode", sal_uInt16(4)) };
    dispatchCommand(mxComponent, ".uno:InsertContents", aPropertyValues);

    // Without fix there had been only 5 object.
    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
    CPPUNIT_ASSERT_MESSAGE("No ScDrawLayer", pDrawLayer);
    const SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("No draw page", pPage);
    CPPUNIT_ASSERT_EQUAL(size_t(6), pPage->GetObjCount());

    // Without fix pasted object had position(7972, 8616) and size(1805×801).
    SdrObject* pObjGreen = lcl_getSdrObjectWithAssert(*pDoc, 4); // pasted shape "Green"
    tools::Rectangle aExpectedRect(tools::Rectangle(Point(12489, 12609), Size(800, 800)));
    tools::Rectangle aSnapRect(pObjGreen->GetSnapRect());
    CPPUNIT_ASSERT_RECTANGLE_EQUAL_WITH_TOLERANCE(aExpectedRect, aSnapRect, 1);

    // Without fix the pasted object was at wrong position(10230,8616).
    // Since LO 7.6 the pasted object has the same size as the original shape "Blue".
    SdrObject* pObjBlue = lcl_getSdrObjectWithAssert(*pDoc, 5);
    aExpectedRect = tools::Rectangle(Point(12489, 14612), Size(800, 800));
    aSnapRect = pObjBlue->GetSnapRect();
    CPPUNIT_ASSERT_RECTANGLE_EQUAL_WITH_TOLERANCE(aExpectedRect, aSnapRect, 1);
}

CPPUNIT_TEST_FIXTURE(ScShapeTest, testTdf155091_paste_duplicates)
{
    // Load a document, which has a shape in range C6:C16, anchored 'To cell (resize with cell)'.
    // The rows 6 to 9 are filtered. When copying the range B5:C19 and paste it to B23, the
    // shape was pasted twice.
    createScDoc("ods/tdf155091_paste_duplicates.ods");
    ScDocument* pDoc = getScDoc();

    // Copy and paste
    goToCell("$B$5:$C$19");
    dispatchCommand(mxComponent, ".uno:Copy", {});
    goToCell("$B$23");
    dispatchCommand(mxComponent, ".uno:Paste", {});

    // Make sure there is no third object but only original and pasted one.
    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
    CPPUNIT_ASSERT_MESSAGE("No ScDrawLayer", pDrawLayer);
    const SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("No draw page", pPage);
    CPPUNIT_ASSERT_EQUAL(size_t(2), pPage->GetObjCount());
}

CPPUNIT_TEST_FIXTURE(ScShapeTest, testTdf125938_anchor_after_copy_paste)
{
    // Load a document, which has an image in cell $sheet1.$B$3, anchored to cell. When the range
    // A3:C3 was copied and pasted to D9:D11 in sheet2, the image was displayed in cell D10, but
    // its anchor was in B3.
    createScDoc("ods/tdf125938_anchor_after_copy_paste.ods");
    ScDocument* pDoc = getScDoc();

    // Copy and paste
    goToCell("$Sheet1.$A$3:$C$3");
    dispatchCommand(mxComponent, ".uno:Copy", {});
    goToCell("$Sheet2.$D$9");
    dispatchCommand(mxComponent, ".uno:Paste", {});

    // Get pasted shape
    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
    const SdrPage* pPage = pDrawLayer->GetPage(1);
    SdrObject* pObj = pPage->GetObj(0);

    // Make sure object is anchored to E9
    ScDrawObjData* pObjData = ScDrawLayer::GetObjData(pObj);
    ScAddress aExpectedAddress(SCCOL(4), SCROW(8), SCTAB(1)); // zero based
    CPPUNIT_ASSERT_EQUAL(aExpectedAddress, (*pObjData).maStart);
}

CPPUNIT_TEST_FIXTURE(ScShapeTest, testTdf154821_shape_in_group)
{
    // The document contains a shape in A7, a group spanning rows 2 to 4 and a second group spanning
    // rows 6 to 10. Error was, that when the document was saved with collapsed groups, the shape
    // lost its position.
    createScDoc("ods/tdf154821_shape_in_group.ods");

    // Get snap rectangle before collapse and save
    ScDocument* pDoc = getScDoc();
    SdrObject* pObj = lcl_getSdrObjectWithAssert(*pDoc, 0);
    tools::Rectangle aRectOrig = pObj->GetSnapRect();

    // Collapse the lower group
    ScTabViewShell* pViewShell = getViewShell();
    pViewShell->GetViewData().SetCurX(0);
    pViewShell->GetViewData().SetCurY(5);
    pViewShell->GetViewData().GetDispatcher().Execute(SID_OUTLINE_HIDE);
    // Collapse the upper group
    pViewShell->GetViewData().SetCurX(0);
    pViewShell->GetViewData().SetCurY(1);
    pViewShell->GetViewData().GetDispatcher().Execute(SID_OUTLINE_HIDE);

    saveAndReload("calc8");

    // Expand the lower group
    pViewShell = getViewShell();
    pViewShell->GetViewData().SetCurX(0);
    pViewShell->GetViewData().SetCurY(5);
    pViewShell->GetViewData().GetDispatcher().Execute(SID_OUTLINE_SHOW);
    // Expand the upper group
    pViewShell = getViewShell();
    pViewShell->GetViewData().SetCurX(0);
    pViewShell->GetViewData().SetCurY(1);
    pViewShell->GetViewData().GetDispatcher().Execute(SID_OUTLINE_SHOW);

    // Verify shape position is not changed besides rounding errors from twips<->mm
    pDoc = getScDoc();
    pObj = lcl_getSdrObjectWithAssert(*pDoc, 0);
    tools::Rectangle aRectReload = pObj->GetSnapRect();
    CPPUNIT_ASSERT_RECTANGLE_EQUAL_WITH_TOLERANCE(aRectOrig, aRectReload, 1);
}

CPPUNIT_TEST_FIXTURE(ScShapeTest, testTdf160003_copy_page_anchored)
{
    // Load a document, which has a chart anchored to page on sheet2. Copy&paste to other document
    // had lost the chart object.
    createScDoc("ods/tdf160003_page_anchored_object.ods");

    // copy range with chart
    goToCell("$Sheet2.$A$1:$L$24");
    dispatchCommand(mxComponent, ".uno:Copy", {});

    // close document and create new one
    createScDoc();

    // paste clipboard
    goToCell("$Sheet1.$A$1");
    dispatchCommand(mxComponent, ".uno:Paste", {});

    // Make sure the chart object exists.
    ScDocument* pDoc = getScDoc();
    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
    const SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pPage->GetObjCount());
}

CPPUNIT_TEST_FIXTURE(ScShapeTest, testTdf160369_groupshape)
{
    // The document contains a group spanning range C5:F12. It is currently anchored to page to
    // make sure its position does not change. When the group was anchored 'To Cell' and rows or
    // columns were hidden before the group, saving changed the anchor position and anchor
    // offset. This happened both with using 'resize with cell' and not.
    createScDoc("ods/tdf160369_groupshape.ods");

    // Get document and group object
    ScDocument* pDoc = getScDoc();
    SdrObject* pObj = lcl_getSdrObjectWithAssert(*pDoc, 0);

    // Anchor group 'To Cell (resize with cell)' to prepare the test.
    ScDrawLayer::SetCellAnchoredFromPosition(*pObj, *pDoc, 0 /*SCTAB*/, true /*bResizeWithCell*/);

    // Hide rows 3 and 4 (UI number), which are before the group
    // Hide column D, which is inside the group
    pDoc->SetRowHidden(2, 3, 0, true);
    pDoc->SetDrawPageSize(0); // trigger recalcpos, otherwise shapes are not changed
    pDoc->SetColHidden(3, 3, 0, true);
    pDoc->SetDrawPageSize(0);

    // Get geometry of the group
    ScDrawObjData* pObjData = ScDrawLayer::GetObjData(pObj);
    ScAddress aOrigStart = (*pObjData).maStart;
    ScAddress aOrigEnd = (*pObjData).maEnd;
    tools::Rectangle aOrigRect = pObj->GetSnapRect();

    // Save document but do not reload. Saving alone had already caused the error.
    save("calc8");

    // Get geometry of the group again
    ScDrawObjData* pAfterObjData = ScDrawLayer::GetObjData(pObj);
    ScAddress aAfterStart = (*pAfterObjData).maStart;
    ScAddress aAfterEnd = (*pAfterObjData).maEnd;
    tools::Rectangle aAfterRect = pObj->GetSnapRect();

    // verify Orig equals After
    CPPUNIT_ASSERT_EQUAL(aOrigStart, aAfterStart);
    CPPUNIT_ASSERT_EQUAL(aOrigEnd, aAfterEnd);
    CPPUNIT_ASSERT_RECTANGLE_EQUAL_WITH_TOLERANCE(aOrigRect, aAfterRect, 1);

    // The same but with saveAndReload.
    createScDoc("ods/tdf160369_groupshape.ods");
    pDoc = getScDoc();
    pObj = lcl_getSdrObjectWithAssert(*pDoc, 0);
    ScDrawLayer::SetCellAnchoredFromPosition(*pObj, *pDoc, 0 /*SCTAB*/, true /*bResizeWithCell*/);
    pDoc->SetRowHidden(2, 3, 0, true);
    pDoc->SetDrawPageSize(0); // trigger recalcpos, otherwise shapes are not changed
    pDoc->SetColHidden(3, 3, 0, true);
    pDoc->SetDrawPageSize(0);

    saveAndReload("calc8");

    // Verify geometry is same as before save
    pDoc = getScDoc();
    pObj = lcl_getSdrObjectWithAssert(*pDoc, 0);
    pAfterObjData = ScDrawLayer::GetObjData(pObj);
    aAfterStart = (*pAfterObjData).maStart;
    aAfterEnd = (*pAfterObjData).maEnd;
    aAfterRect = pObj->GetSnapRect();

    // verify Orig equals After
    CPPUNIT_ASSERT_EQUAL(aOrigStart, aAfterStart);
    CPPUNIT_ASSERT_EQUAL(aOrigEnd, aAfterEnd);
    CPPUNIT_ASSERT_RECTANGLE_EQUAL_WITH_TOLERANCE(aOrigRect, aAfterRect, 1);
}

CPPUNIT_TEST_FIXTURE(ScShapeTest, testTdf160329_sortWithHiddenRows)
{
    // Load a document, which has images anchored to cell and rows hidden
    createScDoc("ods/tdf160329_sortWithHiddenRows.ods");
    ScDocument* pDoc = getScDoc();

    // Sort the rows
    uno::Sequence<beans::PropertyValue> aArgs1
        = { comphelper::makePropertyValue("DbName", u"myRange"_ustr) };
    dispatchCommand(mxComponent, ".uno:SelectDB", aArgs1);
    uno::Sequence<beans::PropertyValue> aArgs2
        = { comphelper::makePropertyValue("ByRows", true),
            comphelper::makePropertyValue("HasHeader", true),
            comphelper::makePropertyValue("Col1", sal_Int32(1)),
            comphelper::makePropertyValue("Ascending1", false),
            comphelper::makePropertyValue("IncludeImages", true) };
    dispatchCommand(mxComponent, ".uno:DataSort", aArgs2);

    // Make sure objects are on correct position
    SdrObject* pObj = lcl_getSdrObjectbyName(*pDoc, std::u16string_view(u"ImageD"));
    Point aPos = pObj->GetSnapRect().TopLeft();
    // The position was (3000|2899) without fix.
    CPPUNIT_ASSERT_POINT_EQUAL_WITH_TOLERANCE(Point(3000, 5898), aPos, 1);
    pObj = lcl_getSdrObjectbyName(*pDoc, std::u16string_view(u"ImageE"));
    aPos = pObj->GetSnapRect().TopLeft();
    // The position was (2600|2499) without fix.
    CPPUNIT_ASSERT_POINT_EQUAL_WITH_TOLERANCE(Point(2600, 4399), aPos, 1);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
