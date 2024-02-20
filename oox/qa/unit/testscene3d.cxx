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
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterType.hpp>
#include <com/sun/star/drawing/Position3D.hpp>
#include <com/sun/star/drawing/ProjectionMode.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <comphelper/sequenceashashmap.hxx>
#include <editeng/unoprnms.hxx>

using namespace css;

/// Covers tests for scene3d import and export, available since LO 24.8., see tdf#70039
class TestScene3d : public UnoApiXmlTest
{
public:
    TestScene3d()
        : UnoApiXmlTest("/oox/qa/unit/data/")
    {
    }

protected:
    // get shape with nShapeIndex from page nPageIndex
    uno::Reference<drawing::XShape> getShape(sal_uInt8 nShapeIndex, sal_uInt8 nPageIndex);
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

    // This shape does not use an extrusion color.
    bool bIsExtrusionColorEnabled(true);
    aExtrusionPropMap.getValue(u"Color"_ustr) >>= bIsExtrusionColorEnabled;
    CPPUNIT_ASSERT_MESSAGE("error: Extrusion color enabled", !bIsExtrusionColorEnabled);
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
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
