/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapixml_test.hxx>

#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterType.hpp>
#include <com/sun/star/drawing/Position3D.hpp>
#include <com/sun/star/drawing/ProjectionMode.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>

#include <comphelper/sequenceashashmap.hxx>
#include <editeng/unoprnms.hxx>
#include <sfx2/viewsh.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdview.hxx>
#include <tools/color.hxx>
#include <vcl/bitmapex.hxx>

using namespace css;

/// Covers tests for scene3d import and export, available since LO 24.8., see tdf#70039
class TestScene3d : public UnoApiXmlTest
{
public:
    TestScene3d()
        : UnoApiXmlTest(u"/oox/qa/unit/data/"_ustr)
    {
    }

protected:
    // get shape with nShapeIndex from page nPageIndex
    uno::Reference<drawing::XShape> getShape(sal_uInt8 nShapeIndex, sal_uInt8 nPageIndex);
    // Converts the shape 0 on page 0 to a bitmap and returns this bitmap.
    // It assumes, that shape 0 on page 0 is the only shape.
    void getShapeAsBitmap(BitmapEx& rBMP, sal_uInt8 nShapeIndex);
};

uno::Reference<drawing::XShape> TestScene3d::getShape(sal_uInt8 nShapeIndex, sal_uInt8 nPageIndex)
{
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent,
                                                                   uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("Could not get XDrawPagesSupplier", xDrawPagesSupplier.is());
    uno::Reference<drawing::XDrawPages> xDrawPages(xDrawPagesSupplier->getDrawPages());
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPages->getByIndex(nPageIndex),
                                                 uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("Could not get xDrawPage", xDrawPage.is());
    uno::Reference<drawing::XShape> xShape(xDrawPage->getByIndex(nShapeIndex), uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("Could not get xShape", xShape.is());
    return xShape;
}

void TestScene3d::getShapeAsBitmap(BitmapEx& rBMP, sal_uInt8 nShapeIndex)
{
    SfxViewShell* pViewShell = SfxViewShell::Current();
    CPPUNIT_ASSERT(pViewShell);
    SdrView* pSdrView = pViewShell->GetDrawView();

    // Mark object and convert it to bitmap
    uno::Reference<drawing::XShape> xShape3D(getShape(nShapeIndex, 0));
    SdrObject* pSdrShape(SdrObject::getSdrObjectFromXShape(xShape3D));
    pSdrView->MarkObj(pSdrShape, pSdrView->GetSdrPageView());
    dispatchCommand(mxComponent, u".uno:ConvertIntoBitmap"_ustr, {});
    pSdrView->UnmarkAll();

    // Get graphic
    uno::Reference<drawing::XShape> xShapeBmp(getShape(nShapeIndex, 0));
    SdrGrafObj* pGrafObj = dynamic_cast<SdrGrafObj*>(SdrObject::getSdrObjectFromXShape(xShapeBmp));
    CPPUNIT_ASSERT_MESSAGE("No image object created", pGrafObj);
    const Graphic& rGraphic = pGrafObj->GetGraphic();
    rBMP = rGraphic.GetBitmapEx();
    CPPUNIT_ASSERT_MESSAGE("No bitmap", !rBMP.IsEmpty());
}

namespace
{
void lcl_AssertColorsApproximateEqual(const ::Color& aExpected, const ::Color& aActual)
{
    // Currently (March 2024), the import of lighting and material is only approximately possible.
    // Thus colors are not identical. When the import will be improved, the tolerances should be
    // reduced. The test uses HSB instead of RGB, because differences in hue are more irritating and
    // should be detected as a priority. That is not possible with GetColorError() method.
    sal_uInt16 nExpH;
    sal_uInt16 nExpS;
    sal_uInt16 nExpB;
    sal_uInt16 nActH;
    sal_uInt16 nActS;
    sal_uInt16 nActB;
    aExpected.RGBtoHSB(nExpH, nExpS, nExpB);
    aActual.RGBtoHSB(nActH, nActS, nActB);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Hue", nExpH, nActH, 2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Saturation", nExpS, nActS, 13);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Brightness", nExpB, nActB, 11);
}
} // end anonymous namespace

CPPUNIT_TEST_FIXTURE(TestScene3d, test_isometricRightUp)
{
    // Given a document with a scene3d element on a shape. Without the fix, the shape was imported as
    // flat 2D shape without extrusion. This test covers some basic properties.
    loadFromFile(u"Scene3d_isometricRightUp.pptx");
    uno::Reference<drawing::XShape> xShape(getShape(1, 0)); // shape 1 on page 0

    // Prepare property maps
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aGeoPropSeq;
    xShapeProps->getPropertyValue(u"CustomShapeGeometry"_ustr) >>= aGeoPropSeq;
    comphelper::SequenceAsHashMap aGeoPropMap(aGeoPropSeq);
    uno::Sequence<beans::PropertyValue> aExtrusionSeq;
    aGeoPropMap.getValue(u"Extrusion"_ustr) >>= aExtrusionSeq;
    comphelper::SequenceAsHashMap aExtrusionPropMap(aExtrusionSeq);

    // Make sure extrusion is on.
    bool bIsExtruded(false);
    aExtrusionPropMap.getValue(u"Extrusion"_ustr) >>= bIsExtruded;
    CPPUNIT_ASSERT_MESSAGE("error: Extrusion not enabled", bIsExtruded);

    // Make sure the extrusion properties correspond to the camera preset type.
    // projection mode should be PARALLEL
    drawing::ProjectionMode eProjectionMode = drawing::ProjectionMode_PERSPECTIVE;
    aExtrusionPropMap.getValue(u"ProjectionMode"_ustr) >>= eProjectionMode;
    CPPUNIT_ASSERT_EQUAL(drawing::ProjectionMode_PARALLEL, eProjectionMode);

    // For Skew, Origin, RotateAngle and Depth
    drawing::EnhancedCustomShapeParameterPair aParaPair;

    // Should be front projection, which means Skew amount and angle are zero.
    aExtrusionPropMap.getValue(u"Skew"_ustr) >>= aParaPair;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aParaPair.First.Value.get<double>(), 1E-14);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aParaPair.Second.Value.get<double>(), 1E-14);

    // .. and Origin x and y are zero.
    aExtrusionPropMap.getValue(u"Origin"_ustr) >>= aParaPair;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aParaPair.First.Value.get<double>(), 1E-14);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aParaPair.Second.Value.get<double>(), 1E-14);

    // Rotation around x-axis should be -35deg, around y-axis should be -45deg.
    aExtrusionPropMap.getValue(UNO_NAME_MISC_OBJ_ROTATEANGLE) >>= aParaPair;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-35.0, aParaPair.First.Value.get<double>(), 1E-14);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-45.0, aParaPair.Second.Value.get<double>(), 1E-14);

    // Depth should be 36pt = 1270Hmm without shift
    aExtrusionPropMap.getValue(u"Depth"_ustr) >>= aParaPair;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1270.0, aParaPair.First.Value.get<double>(), 1E-14);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aParaPair.Second.Value.get<double>(), 1E-14);

    // This shape does not use an extrusion color, but we enable it in all cases and set
    // its color, because MSO and LO have different defaults.
    bool bIsExtrusionColorEnabled(false);
    aExtrusionPropMap.getValue(u"Color"_ustr) >>= bIsExtrusionColorEnabled;
    CPPUNIT_ASSERT_MESSAGE("error: Extrusion color not enabled", bIsExtrusionColorEnabled);
}

CPPUNIT_TEST_FIXTURE(TestScene3d, test_legacyObliqueBottomRight)
{
    // The legacy preset camera types correspond to the geometry available in the UI of LibreOffice.
    // They are not available in the UI of MS Office, but if given, user can change some properties.
    // The test includes some of them.

    loadFromFile(u"Scene3d_legacyObliqueBottomRight.pptx");
    uno::Reference<drawing::XShape> xShape(getShape(1, 0)); // shape 1 on page 0

    // Prepare property maps
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aGeoPropSeq;
    xShapeProps->getPropertyValue(u"CustomShapeGeometry"_ustr) >>= aGeoPropSeq;
    comphelper::SequenceAsHashMap aGeoPropMap(aGeoPropSeq);
    uno::Sequence<beans::PropertyValue> aExtrusionSeq;
    aGeoPropMap.getValue(u"Extrusion"_ustr) >>= aExtrusionSeq;
    comphelper::SequenceAsHashMap aExtrusionPropMap(aExtrusionSeq);

    // For Skew, Origin, RotateAngle and Depth
    drawing::EnhancedCustomShapeParameterPair aParaPair;

    // Should be a slanted projection with shifted origin
    aExtrusionPropMap.getValue(u"Skew"_ustr) >>= aParaPair;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(50.0, aParaPair.First.Value.get<double>(), 1E-14);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(135.0, aParaPair.Second.Value.get<double>(), 1E-14);
    aExtrusionPropMap.getValue(u"Origin"_ustr) >>= aParaPair;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, aParaPair.First.Value.get<double>(), 1E-14);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, aParaPair.Second.Value.get<double>(), 1E-14);

    // The shape has a rotation. Rotations on x-axis and y-axis belong to the extrusion rotation,
    // rotation on z-axis belongs to the shape rotation.
    aExtrusionPropMap.getValue(UNO_NAME_MISC_OBJ_ROTATEANGLE) >>= aParaPair;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-3.45117839701884, aParaPair.First.Value.get<double>(), 1E-12);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(9.3912858020435, aParaPair.Second.Value.get<double>(), 1E-12);
    sal_Int32 nZRotate; // unit 1/100 degree
    xShapeProps->getPropertyValue(UNO_NAME_MISC_OBJ_ROTATEANGLE) >>= nZRotate;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2028), nZRotate);

    // Depth should be 36pt = 1270Hmm. The second value corresponds to the 'Distance from ground'
    // setting in the UI of MS Office, only that it is relative to the depth in ODF.
    aExtrusionPropMap.getValue(u"Depth"_ustr) >>= aParaPair;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1270.0, aParaPair.First.Value.get<double>(), 1E-14);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, aParaPair.Second.Value.get<double>(), 1E-14);

    // This shape uses an extrusion color. The extrusion color itself does not belong to the
    // extrusion but to the shape fill properties. The color is not a complex color but still RGB.
    bool bIsExtrusionColorEnabled(false);
    aExtrusionPropMap.getValue(u"Color"_ustr) >>= bIsExtrusionColorEnabled;
    CPPUNIT_ASSERT_MESSAGE("error: Extrusion color is not enabled", bIsExtrusionColorEnabled);
    Color nColor;
    xShapeProps->getPropertyValue(UNO_NAME_FILLCOLOR_2) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0x89c064), nColor);
}

CPPUNIT_TEST_FIXTURE(TestScene3d, test_obliqueTopRight)
{
    // MS Office applies the shape rotation after the camera rotations in case of oblique preset
    // camera types. That needs to be converted to the 'first shape rotation' specification of ODF.
    // That conversion results in angles not available in the UI of LibreOffice, but LO can
    // render them. The shape has got these rotation in the UI of MS Office: 60° shape ,
    // 50° on x-axis, 10° on y-axis and 40° on z-axis.
    loadFromFile(u"Scene3d_obliqueTopRight.pptx");
    uno::Reference<drawing::XShape> xShape(getShape(0, 0)); // shape 0 on page 0

    // Prepare property maps
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aGeoPropSeq;
    xShapeProps->getPropertyValue(u"CustomShapeGeometry"_ustr) >>= aGeoPropSeq;
    comphelper::SequenceAsHashMap aGeoPropMap(aGeoPropSeq);
    uno::Sequence<beans::PropertyValue> aExtrusionSeq;
    aGeoPropMap.getValue(u"Extrusion"_ustr) >>= aExtrusionSeq;
    comphelper::SequenceAsHashMap aExtrusionPropMap(aExtrusionSeq);

    // For Skew and RotateAngle
    drawing::EnhancedCustomShapeParameterPair aParaPair;

    // Should be an oblique projection with non default skew values
    aExtrusionPropMap.getValue(u"Skew"_ustr) >>= aParaPair;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(30.0, aParaPair.First.Value.get<double>(), 1E-14);
    // -135° from preset camera type, minus 60° shape rotation
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-195.0, aParaPair.Second.Value.get<double>(), 1E-14);

    // Because of the needed conversions the resulting angles are very different from
    // those set in the UI of MS Office.
    aExtrusionPropMap.getValue(UNO_NAME_MISC_OBJ_ROTATEANGLE) >>= aParaPair;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-45.7369327638437, aParaPair.First.Value.get<double>(), 1E-12);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(24.9102537477636, aParaPair.Second.Value.get<double>(), 1E-12);
    sal_Int32 nZRotate; // unit 1/100 degree
    xShapeProps->getPropertyValue(UNO_NAME_MISC_OBJ_ROTATEANGLE) >>= nZRotate;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(34597), nZRotate);

    // The test uses BoundRect to verify position and size of the resulting scene.
    // The tolerance 10 is estimated and can be adjusted if required for HiDPI.
    awt::Rectangle aBoundRect;
    xShapeProps->getPropertyValue(UNO_NAME_MISC_OBJ_BOUNDRECT) >>= aBoundRect;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(8325, aBoundRect.Width, 10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(7804, aBoundRect.Height, 10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(8272, aBoundRect.X, 10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3510, aBoundRect.Y, 10);
}

CPPUNIT_TEST_FIXTURE(TestScene3d, test_orthographicFront)
{
    // MS Office users might use the z-rotation instead of the shape rotation without intending an
    // extrusion. As of March 2024 we import the shape as 2D shape in such case, because extrusion
    // looses the stroke, see tdf#159334.
    loadFromFile(u"Scene3d_orthographicFront.pptx");
    uno::Reference<drawing::XShape> xShape(getShape(0, 0)); // shape 0 on page 0

    // Make sure that in case extrusion properties exist, the extrusion is disabled.
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aGeoPropSeq;
    xShapeProps->getPropertyValue(u"CustomShapeGeometry"_ustr) >>= aGeoPropSeq;
    comphelper::SequenceAsHashMap aGeoPropMap(aGeoPropSeq);
    uno::Sequence<beans::PropertyValue> aExtrusionSeq;
    if (aGeoPropMap.getValue(u"Extrusion"_ustr) >>= aExtrusionSeq)
    {
        comphelper::SequenceAsHashMap aExtrusionPropMap(aExtrusionSeq);
        bool bIsExtruded(true);
        aExtrusionPropMap.getValue(u"Extrusion"_ustr) >>= bIsExtruded;
        CPPUNIT_ASSERT_MESSAGE("error: Extrusion is enabled", !bIsExtruded);
    }

    // Make sure the shape is nevertheless rotated.
    sal_Int32 nZRotate; // unit 1/100 degree
    xShapeProps->getPropertyValue(UNO_NAME_MISC_OBJ_ROTATEANGLE) >>= nZRotate;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(9000), nZRotate);
}

CPPUNIT_TEST_FIXTURE(TestScene3d, test_perspectiveContrastingLeft)
{
    // The file contains a shape with the preset camera perspectiveContrastingLeft.
    // Such shape cannot be produced in the UI of LibreOffice, but it can be rendered.
    loadFromFile(u"Scene3d_perspectiveContrastingLeft.pptx");
    uno::Reference<drawing::XShape> xShape(getShape(0, 0)); // shape 0 on page 0

    // Prepare property maps
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aGeoPropSeq;
    xShapeProps->getPropertyValue(u"CustomShapeGeometry"_ustr) >>= aGeoPropSeq;
    comphelper::SequenceAsHashMap aGeoPropMap(aGeoPropSeq);
    uno::Sequence<beans::PropertyValue> aExtrusionSeq;
    aGeoPropMap.getValue(u"Extrusion"_ustr) >>= aExtrusionSeq;
    comphelper::SequenceAsHashMap aExtrusionPropMap(aExtrusionSeq);

    // projection mode should be PERSPECTIVE
    drawing::ProjectionMode eProjectionMode = drawing::ProjectionMode_PARALLEL;
    aExtrusionPropMap.getValue(u"ProjectionMode"_ustr) >>= eProjectionMode;
    CPPUNIT_ASSERT_EQUAL(drawing::ProjectionMode_PERSPECTIVE, eProjectionMode);

    // A perspective projection needs a viewPoint. MS Office has it on the z-axis.
    drawing::Position3D aViewPoint;
    aExtrusionPropMap.getValue(u"ViewPoint"_ustr) >>= aViewPoint;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aViewPoint.PositionX, 1E-12);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aViewPoint.PositionY, 1E-12);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(38451.0, aViewPoint.PositionZ, 1E-12);

    // Check rotation angles
    drawing::EnhancedCustomShapeParameterPair aParaPair;
    aExtrusionPropMap.getValue(UNO_NAME_MISC_OBJ_ROTATEANGLE) >>= aParaPair;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-6.94093344831102, aParaPair.First.Value.get<double>(), 1E-12);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(44.4431265782766, aParaPair.Second.Value.get<double>(), 1E-12);
    sal_Int32 nZRotate; // unit 1/100 degree
    xShapeProps->getPropertyValue(UNO_NAME_MISC_OBJ_ROTATEANGLE) >>= nZRotate;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(35504), nZRotate);

    // Use BoundRect as workaround, because there is no easy way to test the rendered line geometry.
    // The tolerance 10 is estimated and can be adjusted if required for HiDPI.
    awt::Rectangle aBoundRect;
    xShapeProps->getPropertyValue(UNO_NAME_MISC_OBJ_BOUNDRECT) >>= aBoundRect;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6614, aBoundRect.Width, 10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(8577, aBoundRect.Height, 10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5725, aBoundRect.X, 10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4213, aBoundRect.Y, 10);
}

CPPUNIT_TEST_FIXTURE(TestScene3d, test_legacyPerspectiveTopRight)
{
    // The file contains a shape with the preset camera legacyPerspectiveTopLeft. These kind of
    // camera corresponds directly to the extrusions available in the UI of LibreOffice. The
    // non-frontal view is not done by rotation but by shifting the view point horizontal and
    // vertical. That is tested here.
    loadFromFile(u"Scene3d_legacyPerspectiveTopRight.pptx");
    uno::Reference<drawing::XShape> xShape(getShape(0, 0)); // shape 0 on page 0

    // Prepare property maps
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aGeoPropSeq;
    xShapeProps->getPropertyValue(u"CustomShapeGeometry"_ustr) >>= aGeoPropSeq;
    comphelper::SequenceAsHashMap aGeoPropMap(aGeoPropSeq);
    uno::Sequence<beans::PropertyValue> aExtrusionSeq;
    aGeoPropMap.getValue(u"Extrusion"_ustr) >>= aExtrusionSeq;
    comphelper::SequenceAsHashMap aExtrusionPropMap(aExtrusionSeq);

    // shifted view point
    drawing::Position3D aViewPoint;
    aExtrusionPropMap.getValue(u"ViewPoint"_ustr) >>= aViewPoint;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3472.0, aViewPoint.PositionX, 1E-12);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-3472.0, aViewPoint.PositionY, 1E-12);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(25000.0, aViewPoint.PositionZ, 1E-12);

    // Use BoundRect as workaround, because there is no easy way to test the rendered line geometry.
    // The tolerance 10 is estimated and can be adjusted if required for HiDPI.
    awt::Rectangle aBoundRect;
    xShapeProps->getPropertyValue(UNO_NAME_MISC_OBJ_BOUNDRECT) >>= aBoundRect;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(7004, aBoundRect.Width, 10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(7004, aBoundRect.Height, 10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6292, aBoundRect.X, 10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3138, aBoundRect.Y, 10);
}

CPPUNIT_TEST_FIXTURE(TestScene3d, test_lightRig_modernCamera)
{
    // The modern camera 'orthographicFront' looks at the shape. The scene is lit by lightRig 'twoPt'.
    // A modern camera moves around the shape. The lightRig has a fixed position in regard to the
    // shape. This is the same shape as in test_lightRig_legacyCamera but with a modern camera.
    // The test assumes rendering with ShadeMode_FLAT.
    loadFromFile(u"Scene3d_lightRig_modernCamera.pptx");
    BitmapEx aBMP;
    getShapeAsBitmap(aBMP, 0);

    // Size in pixel depends on dpi. Thus calculate positions relative to size.
    // Color in center
    sal_Int32 nX = 0.5 * aBMP.GetSizePixel().Width();
    sal_Int32 nY = 0.5 * aBMP.GetSizePixel().Height();
    lcl_AssertColorsApproximateEqual(::Color(247, 225, 211), aBMP.GetPixelColor(nX, nY));
    // Color left
    nX = 0.046122 * aBMP.GetSizePixel().Width();
    nY = 0.5 * aBMP.GetSizePixel().Height();
    lcl_AssertColorsApproximateEqual(::Color(0, 103, 47), aBMP.GetPixelColor(nX, nY));
}

CPPUNIT_TEST_FIXTURE(TestScene3d, test_lightRig_legacyCamera)
{
    // The legacy camera 'legacyObliqueFront' looks at the shape. The scene is lit by lightRig
    // 'twoPt'. A legacy camera is fix, instead the shape is rotated. The lightRig has a fixed
    // position in regard to the camera, but the shape receives various lighting when rotated.
    // This is the same shape as in test_lightRig_modernCamera but with a legacy camera.
    // The test assumes rendering with ShadeMode_FLAT.
    loadFromFile(u"Scene3d_lightRig_legacyCamera.pptx");
    BitmapEx aBMP;
    getShapeAsBitmap(aBMP, 0);

    // Size in pixel depends on dpi. Thus calculate positions relative to size.
    // Color in center
    sal_Int32 nX = 0.5 * aBMP.GetSizePixel().Width();
    sal_Int32 nY = 0.5 * aBMP.GetSizePixel().Height();
    lcl_AssertColorsApproximateEqual(::Color(94, 86, 80), aBMP.GetPixelColor(nX, nY));
    // Color left
    nX = 0.046122 * aBMP.GetSizePixel().Width();
    nY = 0.5 * aBMP.GetSizePixel().Height();
    lcl_AssertColorsApproximateEqual(::Color(5, 185, 87), aBMP.GetPixelColor(nX, nY));
}

CPPUNIT_TEST_FIXTURE(TestScene3d, test_lightRig_default)
{
    // The scene uses the modern camera 'isometricOffAxis1Top' and the lightRig 'harsh'. Here the
    // unrotated lightRig is tested. Since rig 'harsh' has only two lights and the direction of the
    // second light is against view direction, the colors are same in LibreOffice and MS Office.
    // The test assumes rendering with ShadeMode_FLAT.
    loadFromFile(u"Scene3d_lightRig_default.pptx");
    BitmapEx aBMP;
    getShapeAsBitmap(aBMP, 0);

    // Size in pixel depends on dpi. Thus calculate positions relative to size.
    // Front color
    sal_Int32 nX = 0.93811 * aBMP.GetSizePixel().Width();
    sal_Int32 nY = 0.49904 * aBMP.GetSizePixel().Height();
    lcl_AssertColorsApproximateEqual(::Color(165, 187, 150), aBMP.GetPixelColor(nX, nY));
    // Left color
    nX = 0.078176 * aBMP.GetSizePixel().Width();
    nY = 0.49904 * aBMP.GetSizePixel().Height();
    lcl_AssertColorsApproximateEqual(::Color(255, 189, 74), aBMP.GetPixelColor(nX, nY));
    // Top color
    nX = 0.48860 * aBMP.GetSizePixel().Width();
    nY = 0.069098 * aBMP.GetSizePixel().Height();
    lcl_AssertColorsApproximateEqual(::Color(189, 100, 39), aBMP.GetPixelColor(nX, nY));
}

CPPUNIT_TEST_FIXTURE(TestScene3d, test_lightRig_dir_rotation)
{
    // The scene uses the modern camera 'isometricOffAxis1Top' and the lightRig 'harsh'. The rig is
    // rotated around the z-axis by attribute 'dir'. Since rig 'harsh' has only two lights and the
    // direction of the second light is against the view direction, colors are same in LibreOffice
    // and MSO. The test assumes rendering with ShadeMode_FLAT.
    loadFromFile(u"Scene3d_lightRig_dir_rotation.pptx");
    BitmapEx aBMP;
    getShapeAsBitmap(aBMP, 0);

    // Size in pixel depends on dpi. Thus calculate positions relative to size.
    // Front color
    sal_Int32 nX = 0.93811 * aBMP.GetSizePixel().Width();
    sal_Int32 nY = 0.49904 * aBMP.GetSizePixel().Height();
    lcl_AssertColorsApproximateEqual(::Color(165, 187, 150), aBMP.GetPixelColor(nX, nY));
    // Left color
    nX = 0.078176 * aBMP.GetSizePixel().Width();
    nY = 0.49904 * aBMP.GetSizePixel().Height();
    lcl_AssertColorsApproximateEqual(::Color(206, 108, 42), aBMP.GetPixelColor(nX, nY));
    // Top color
    nX = 0.48860 * aBMP.GetSizePixel().Width();
    nY = 0.069098 * aBMP.GetSizePixel().Height();
    lcl_AssertColorsApproximateEqual(::Color(255, 189, 74), aBMP.GetPixelColor(nX, nY));
}

CPPUNIT_TEST_FIXTURE(TestScene3d, test_lightRig_rot_rotation)
{
    // The scene uses the modern camera 'isometricOffAxis1Top' and the lightRig 'harsh'. The rig is
    // rotated around x- and y-axis by element 'rot'.
    // The test assumes rendering with ShadeMode_FLAT.
    loadFromFile(u"Scene3d_lightRig_rot_rotation.pptx");
    BitmapEx aBMP;
    getShapeAsBitmap(aBMP, 0);

    // Size in pixel depends on dpi. Thus calculate positions relative to size.
    // Front color, same as in MS Office
    sal_Int32 nX = 0.93811 * aBMP.GetSizePixel().Width();
    sal_Int32 nY = 0.49904 * aBMP.GetSizePixel().Height();
    lcl_AssertColorsApproximateEqual(::Color(88, 100, 80), aBMP.GetPixelColor(nX, nY));
    // Left color
    nX = 0.078176 * aBMP.GetSizePixel().Width();
    nY = 0.49904 * aBMP.GetSizePixel().Height();
    lcl_AssertColorsApproximateEqual(::Color(255, 214, 99), aBMP.GetPixelColor(nX, nY));
    // Top color
    nX = 0.48860 * aBMP.GetSizePixel().Width();
    nY = 0.069098 * aBMP.GetSizePixel().Height();
    lcl_AssertColorsApproximateEqual(::Color(106, 56, 22), aBMP.GetPixelColor(nX, nY));
}

CPPUNIT_TEST_FIXTURE(TestScene3d, test_material_highlight)
{
    // The file contains six shapes with same geometry and fill and line color. The scenes use the
    // camera 'orthographicFront' and the lightRig 'twoPt'. The test looks at an area of highlight
    // and at an area outside the highlight.

    loadFromFile(u"Scene3d_material_highlight.pptx");

    BitmapEx aBMP;
    getShapeAsBitmap(aBMP, 0); // material legacyPlastic
    sal_Int32 nX = 0.75 * aBMP.GetSizePixel().Width();
    sal_Int32 nYhigh = 0.25 * aBMP.GetSizePixel().Height();
    sal_Int32 nYsoft = 0.75 * aBMP.GetSizePixel().Height();
    lcl_AssertColorsApproximateEqual(::Color(255, 255, 255), aBMP.GetPixelColor(nX, nYhigh));
    lcl_AssertColorsApproximateEqual(::Color(130, 95, 70), aBMP.GetPixelColor(nX, nYsoft));

    // same geometry, thus nX, nYhigh and nYsoft unchanged
    getShapeAsBitmap(aBMP, 1); // material warmMatte
    lcl_AssertColorsApproximateEqual(::Color(253, 200, 164), aBMP.GetPixelColor(nX, nYhigh));
    lcl_AssertColorsApproximateEqual(::Color(132, 96, 71), aBMP.GetPixelColor(nX, nYsoft));

    getShapeAsBitmap(aBMP, 2); // material metal
    lcl_AssertColorsApproximateEqual(::Color(255, 255, 255), aBMP.GetPixelColor(nX, nYhigh));
    lcl_AssertColorsApproximateEqual(::Color(132, 96, 71), aBMP.GetPixelColor(nX, nYsoft));

    getShapeAsBitmap(aBMP, 3); // material matte
    lcl_AssertColorsApproximateEqual(::Color(190, 138, 102), aBMP.GetPixelColor(nX, nYhigh));
    lcl_AssertColorsApproximateEqual(::Color(130, 95, 70), aBMP.GetPixelColor(nX, nYsoft));

    getShapeAsBitmap(aBMP, 4); // material dkEdge
    lcl_AssertColorsApproximateEqual(::Color(255, 255, 255), aBMP.GetPixelColor(nX, nYhigh));
    lcl_AssertColorsApproximateEqual(::Color(115, 84, 62), aBMP.GetPixelColor(nX, nYsoft));

    getShapeAsBitmap(aBMP, 5); // material legacyMetal
    lcl_AssertColorsApproximateEqual(::Color(255, 255, 220), aBMP.GetPixelColor(nX, nYhigh));
    lcl_AssertColorsApproximateEqual(::Color(86, 63, 46), aBMP.GetPixelColor(nX, nYsoft));
}

CPPUNIT_TEST_FIXTURE(TestScene3d, test_material_wireframe)
{
    // Given a document with a shape in 3D mode with material legacyWireframe.
    // It uses a projection "Oblique: Top Left".
    loadFromFile(u"Scene3d_material_wireframe.pptx");
    uno::Reference<drawing::XShape> xShape(getShape(0, 0));

    // Make sure the export to ODF has the corresponding attributes.
    save(u"impress8"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);

    assertXPath(pXmlDoc,
                "/office:document-content/office:body/office:presentation/draw:page/"
                "draw:custom-shape/draw:enhanced-geometry"_ostr,
                "extrusion-origin"_ostr, u"-0.5 -0.5"_ustr);
    assertXPath(pXmlDoc,
                "/office:document-content/office:body/office:presentation/draw:page/"
                "draw:custom-shape/draw:enhanced-geometry"_ostr,
                "extrusion-skew"_ostr, u"30 -45"_ustr);
    assertXPath(pXmlDoc,
                "/office:document-content/office:body/office:presentation/draw:page/"
                "draw:custom-shape/draw:enhanced-geometry"_ostr,
                "projection"_ostr, u"parallel"_ustr);
    assertXPath(pXmlDoc,
                "/office:document-content/office:body/office:presentation/draw:page/"
                "draw:custom-shape/draw:enhanced-geometry"_ostr,
                "shade-mode"_ostr, u"draft"_ustr);
}

CPPUNIT_TEST_FIXTURE(TestScene3d, test_cropped_image)
{
    // The file contains an image that has been cropped to shape in PowerPoint. The image is in 3D
    // mode with perspective camera perspectiveContrastingRightFacing and perspective angle 120°. The
    // extrusion is 76200 EMU deep with extrusion color #00B050. As a cropped image, it was imported
    // in earlier LO versions as custom shape with bitmap fill, but without any 3D.
    loadFromFile(u"Scene3d_cropped_image.pptx");
    uno::Reference<drawing::XShape> xShape(getShape(0, 0)); // shape 0 on page 0

    // Prepare property maps
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aGeoPropSeq;
    xShapeProps->getPropertyValue(u"CustomShapeGeometry"_ustr) >>= aGeoPropSeq;
    comphelper::SequenceAsHashMap aGeoPropMap(aGeoPropSeq);
    uno::Sequence<beans::PropertyValue> aExtrusionSeq;
    aGeoPropMap.getValue(u"Extrusion"_ustr) >>= aExtrusionSeq;
    comphelper::SequenceAsHashMap aExtrusionPropMap(aExtrusionSeq);

    // Check that extrusion is on and has the correct geometry.
    bool bIsExtruded(false);
    aExtrusionPropMap.getValue(u"Extrusion"_ustr) >>= bIsExtruded;
    CPPUNIT_ASSERT_MESSAGE("error: Extrusion is disabled", bIsExtruded);

    drawing::ProjectionMode eProjectionMode = drawing::ProjectionMode_PARALLEL;
    aExtrusionPropMap.getValue(u"ProjectionMode"_ustr) >>= eProjectionMode;
    CPPUNIT_ASSERT_EQUAL(drawing::ProjectionMode_PERSPECTIVE, eProjectionMode);

    drawing::Position3D aViewPoint;
    aExtrusionPropMap.getValue(u"ViewPoint"_ustr) >>= aViewPoint;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aViewPoint.PositionX, 1E-12);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, aViewPoint.PositionY, 1E-12);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(9223.7479, aViewPoint.PositionZ, 1E-5);

    // Check shape rotation angles
    drawing::EnhancedCustomShapeParameterPair aParaPair;
    aExtrusionPropMap.getValue(UNO_NAME_MISC_OBJ_ROTATEANGLE) >>= aParaPair;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-6.94093344831102, aParaPair.First.Value.get<double>(), 1E-12);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-44.4431265782766, aParaPair.Second.Value.get<double>(), 1E-12);
    sal_Int32 nZRotate; // unit 1/100 degree
    xShapeProps->getPropertyValue(UNO_NAME_MISC_OBJ_ROTATEANGLE) >>= nZRotate;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(496), nZRotate);

    // Check extrusion depth and color
    aExtrusionPropMap.getValue(u"Depth"_ustr) >>= aParaPair;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(212.0, aParaPair.First.Value.get<double>(), 1E-12);
    Color nColor;
    xShapeProps->getPropertyValue(UNO_NAME_FILLCOLOR_2) >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0x00b050), nColor);
}

CPPUNIT_TEST_FIXTURE(TestScene3d, test_pureImage)
{
    // Given a document with a scene3d element on an image. Because it would lose image properties
    // it is currently (March 2024) not imported as custom shape. But the z-rotation is evaluated
    // as users might have used this instead of shape rotation, for example.
    loadFromFile(u"Scene3d_pureImage.pptx");
    uno::Reference<drawing::XShape> xShape(getShape(0, 0)); // shape 0 on page 0

    // Make sure it is an image.
    CPPUNIT_ASSERT_EQUAL(u"com.sun.star.drawing.GraphicObjectShape"_ustr, xShape->getShapeType());

    // Make sure the image is rotated.
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    sal_Int32 nZRotate; // unit 1/100 degree
    xShapeProps->getPropertyValue(UNO_NAME_MISC_OBJ_ROTATEANGLE) >>= nZRotate;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(27000), nZRotate);
}

CPPUNIT_TEST_FIXTURE(TestScene3d, test_shape_rotation)
{
    // Given a document with a shape in 3D mode with shape rotation rot="300000".
    loadFromFile(u"Scene3d_shape_rotation.pptx");
    uno::Reference<drawing::XShape> xShape(getShape(0, 0));

    // Make sure that the immediate export to pptx has the same shape rotation
    save(u"Impress Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc, "//p:spPr/a:xfrm"_ostr, "rot"_ostr, u"300000"_ustr);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
