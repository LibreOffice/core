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
#include <rtl/ustring.hxx>
#include <editeng/unoprnms.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <svx/EnhancedCustomShape2d.hxx>
#include <svx/svdoashp.hxx>
#include <svx/svdopath.hxx>
#include <svx/unoapi.hxx>

#include <cppunit/TestAssert.h>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/frame/Desktop.hpp>

using namespace ::com::sun::star;

namespace
{
const OUString sDataDirectory("svx/qa/unit/data/");

/// Tests for svx/source/customshapes/ code.
class CustomshapesTest : public test::BootstrapFixture, public unotest::MacrosTest
{
    uno::Reference<lang::XComponent> mxComponent;

private:
    // get shape nShapeIndex from page 0
    uno::Reference<drawing::XShape> getShape(sal_uInt8 nShapeIndex);

public:
    virtual void setUp() override
    {
        test::BootstrapFixture::setUp();
        mxDesktop.set(frame::Desktop::create(m_xContext));
    }

    virtual void tearDown() override
    {
        if (mxComponent.is())
        {
            mxComponent->dispose();
        }
        test::BootstrapFixture::tearDown();
    }

    void testViewBoxLeftTop();
    void testAccuracyCommandX();
    void testToggleCommandXY();
    void testMultipleMoveTo();
    void testWidthOrientationCommandU();
    void testHalfEllipseVML();
    void testLargeSwingAngleVML();
    void testTdf121845_two_commands_U();

    CPPUNIT_TEST_SUITE(CustomshapesTest);
    CPPUNIT_TEST(testViewBoxLeftTop);
    CPPUNIT_TEST(testAccuracyCommandX);
    CPPUNIT_TEST(testToggleCommandXY);
    CPPUNIT_TEST(testMultipleMoveTo);
    CPPUNIT_TEST(testWidthOrientationCommandU);
    CPPUNIT_TEST(testHalfEllipseVML);
    CPPUNIT_TEST(testLargeSwingAngleVML);
    CPPUNIT_TEST(testTdf121845_two_commands_U);
    CPPUNIT_TEST_SUITE_END();
};

uno::Reference<drawing::XShape> CustomshapesTest::getShape(sal_uInt8 nShapeIndex)
{
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent,
                                                                   uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("Could not get XDrawPagesSupplier", xDrawPagesSupplier.is());
    uno::Reference<drawing::XDrawPages> xDrawPages(xDrawPagesSupplier->getDrawPages());
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPages->getByIndex(0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("Could not get xDrawPage", xDrawPage.is());
    uno::Reference<drawing::XShape> xShape(xDrawPage->getByIndex(nShapeIndex), uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("Could not get xShape", xShape.is());
    return xShape;
}

void CustomshapesTest::testViewBoxLeftTop()
{
    // tdf#121890 formula values "left" and "top" are wrongly calculated
    // Load a document with two custom shapes of type "non-primitive"
    OUString aURL
        = m_directories.getURLFromSrc(sDataDirectory) + "viewBox_positive_twolines_strict.odp";
    mxComponent = loadFromDesktop(aURL, "com.sun.star.comp.presentation.PresentationDocument");
    CPPUNIT_ASSERT_MESSAGE("Could not load document", mxComponent.is());
    // Get the shape "leftright". Error was, that the identifier "left" was always set to zero, thus
    // the path was outside the frame rectangle for a viewBox having a positive "left" value.
    uno::Reference<drawing::XShape> xShapeLR(getShape(0));
    uno::Reference<beans::XPropertySet> xShapeLRProps(xShapeLR, uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("Could not get the shape 'leftright' properties", xShapeLRProps.is());
    awt::Rectangle aFrameRectLR;
    xShapeLRProps->getPropertyValue(UNO_NAME_MISC_OBJ_FRAMERECT) >>= aFrameRectLR;
    awt::Rectangle aBoundRectLR;
    xShapeLRProps->getPropertyValue(UNO_NAME_MISC_OBJ_BOUNDRECT) >>= aBoundRectLR;
    // difference should be zero, but allow some rounding errors
    CPPUNIT_ASSERT_LESS(static_cast<long>(3), labs(aFrameRectLR.X - aBoundRectLR.X));

    // Get the shape "topbottom". Error was, that the identifier "top" was always set to zero, thus
    // the path was outside the frame rectangle for a viewBox having a positive "top" value.
    uno::Reference<drawing::XShape> xShapeTB(getShape(1));
    uno::Reference<beans::XPropertySet> xShapeTBProps(xShapeTB, uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("Could not get the shape 'topbottom' properties", xShapeTBProps.is());
    awt::Rectangle aFrameRectTB;
    xShapeTBProps->getPropertyValue(UNO_NAME_MISC_OBJ_FRAMERECT) >>= aFrameRectTB;
    awt::Rectangle aBoundRectTB;
    xShapeTBProps->getPropertyValue(UNO_NAME_MISC_OBJ_BOUNDRECT) >>= aBoundRectTB;
    // difference should be zero, but allow some rounding errors
    CPPUNIT_ASSERT_LESS(static_cast<long>(3), labs(aFrameRectTB.Y - aBoundRectTB.Y));
}

void CustomshapesTest::testAccuracyCommandX()
{
    // 121761 Increase accuracy of quarter circles drawn by command X or Y
    // The loaded document has a quarter circle with radius 10000 (unit 1/100 mm)
    // which is rotated by 45deg. The test considers the segment.
    OUString aURL
        = m_directories.getURLFromSrc(sDataDirectory) + "tdf121761_Accuracy_command_X.odp";
    mxComponent = loadFromDesktop(aURL, "com.sun.star.comp.presentation.PresentationDocument");
    CPPUNIT_ASSERT_MESSAGE("Could not load document", mxComponent.is());
    // Get the shape "arc_45deg_rotated". Error was, that a Bezier curve with bad parameters
    // was used, thus the segment height was obviously smaller than for a true circle.
    // Math: segment height approx 10000 * ( 1 - sqrt(0.5)) + line width
    uno::Reference<drawing::XShape> xShape(getShape(0));
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("Could not get the shape properties", xShapeProps.is());
    awt::Rectangle aBoundRect;
    xShapeProps->getPropertyValue(UNO_NAME_MISC_OBJ_BOUNDRECT) >>= aBoundRect;
    double fHeight = static_cast<double>(aBoundRect.Height);
    // The tolerance is a guess, might be smaller.
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("segment height out of tolerance", 2942.0, fHeight, 8.0);
}

void CustomshapesTest::testToggleCommandXY()
{
    // 121952 Toggle x- and y-direction if command X has several parameters
    // The loaded document has a shape with command X and two parameter placed on a diagonal.
    // The radius of the quarter circles are both 10000 (unit 1/100 mm).
    // The shape is rotated by 45deg, so you get two segments, one up and one down.
    OUString aURL
        = m_directories.getURLFromSrc(sDataDirectory) + "tdf121952_Toggle_direction_command_X.odp";
    mxComponent = loadFromDesktop(aURL, "com.sun.star.comp.presentation.PresentationDocument");
    CPPUNIT_ASSERT_MESSAGE("Could not load document", mxComponent.is());
    // Error was, that the second segment was drawn with same direction as first one. If drawn
    // correctly, the bounding box height of the segments together is about twice the single
    // segment height. Math: segment height approx 10000 * ( 1 - sqrt(0.5)) + line width
    uno::Reference<drawing::XShape> xShape(getShape(0));
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("Could not get the shape properties", xShapeProps.is());
    awt::Rectangle aBoundRect;
    xShapeProps->getPropertyValue(UNO_NAME_MISC_OBJ_BOUNDRECT) >>= aBoundRect;
    double fHeight = static_cast<double>(aBoundRect.Height);
    // The tolerance is a guess, might be smaller.
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("segment height out of tolerance", 5871.0, fHeight, 16.0);
}

void CustomshapesTest::testMultipleMoveTo()
{
    // tdf122964 Multiple moveTo has to be treated as lineTo in draw:enhanced-path
    // Load a document with path "M 0 0 5 10 10 0 N"
    OUString aURL = m_directories.getURLFromSrc(sDataDirectory) + "tdf122964_MultipleMoveTo.odg";
    mxComponent = loadFromDesktop(aURL, "com.sun.star.comp.drawing.DrawingDocument");
    CPPUNIT_ASSERT_MESSAGE("Could not load document", mxComponent.is());
    // Error was, that the second and further parameter pairs were treated as moveTo,
    // and so the generated path was empty, resulting in zero width and height of the
    // bounding box. It has to be treated same as "M 0 0 L 5 10 10 0 N".
    uno::Reference<drawing::XShape> xShape(getShape(0));
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("Could not get the shape properties", xShapeProps.is());
    awt::Rectangle aBoundRect;
    xShapeProps->getPropertyValue(UNO_NAME_MISC_OBJ_BOUNDRECT) >>= aBoundRect;
    bool bIsZero(aBoundRect.Height == 0 && aBoundRect.Width == 0);
    CPPUNIT_ASSERT_MESSAGE("Path is empty", !bIsZero);
}

void CustomshapesTest::testWidthOrientationCommandU()
{
    // tdf121845 custom shape with command U (angleellipse) is  wrongly drawn
    // Load a document with path "M 750 0 L 750 500 250 500 250 0 U 500 0 500 500 0 180 N"
    // in viewBox="0 0 1000 500" and width="10cm", height="5cm".
    const OUString sFileName("tdf121845_WidthOrientation_command_U.odg");
    const OUString sURL = m_directories.getURLFromSrc(sDataDirectory) + sFileName;
    mxComponent = loadFromDesktop(sURL, "com.sun.star.comp.drawing.DrawingDocument");
    CPPUNIT_ASSERT_MESSAGE("Could not load document", mxComponent.is());
    // Error was, that the width and height of the ellipse was halved and that the ellipse
    // was not drawn clockwise but counter clockwise.
    uno::Reference<drawing::XShape> xShape(getShape(0));
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("Could not get the shape properties", xShapeProps.is());
    awt::Rectangle aBoundRect;
    xShapeProps->getPropertyValue(UNO_NAME_MISC_OBJ_BOUNDRECT) >>= aBoundRect;
    const double fWidth = static_cast<double>(aBoundRect.Width);
    // Need some tolerance for line width
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("wrong width", 10000.0, fWidth, 40.0);
    const double fHeight = static_cast<double>(aBoundRect.Height);
    // Wrong orientation draws segment above the top of the viewBox and so increases 'Height'.
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("wrong orientation", 5000.0, fHeight, 40.0);
}

void CustomshapesTest::testHalfEllipseVML()
{
    // tdf121845 custom shape with command U (angleellipse) is  wrongly drawn
    // Load a document which was converted from VML to doc by Word. It had a VML
    // path="m750,al500,,500,500,,-11796480e" resulting in a lower half circle.
    const OUString sFileName("tdf121845_HalfEllipseVML.doc");
    const OUString sURL = m_directories.getURLFromSrc(sDataDirectory) + sFileName;
    mxComponent = loadFromDesktop(sURL, "com.sun.star.comp.text.TextDocument");
    CPPUNIT_ASSERT_MESSAGE("Could not load document", mxComponent.is());
    // Error was, that a full circle instead of the half circle was draw.
    uno::Reference<drawing::XShape> xShape(getShape(0));
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("Could not get the shape properties", xShapeProps.is());
    awt::Rectangle aBoundRect;
    xShapeProps->getPropertyValue(UNO_NAME_MISC_OBJ_BOUNDRECT) >>= aBoundRect;
    const double fDiff2HmW = static_cast<double>(2 * aBoundRect.Height - aBoundRect.Width);
    // Need some tolerance for line width
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("not a half circle", 0.0, fDiff2HmW, 40.0);
}

void CustomshapesTest::testLargeSwingAngleVML()
{
    // tdf121845 custom shape with command U (angleellipse) is  wrongly drawn
    // Load a document which was converted from VML to doc by Word. It had a VML
    // path="al50,50,45,45,2621440,31457280e" resulting in a full circle plus 120 deg segment.
    const OUString sFileName("tdf121845_start40_swing480.doc");
    const OUString sURL = m_directories.getURLFromSrc(sDataDirectory) + sFileName;
    mxComponent = loadFromDesktop(sURL, "com.sun.star.comp.text.TextDocument");
    CPPUNIT_ASSERT_MESSAGE("Could not load document", mxComponent.is());
    // Error was, that only the 120 deg segment was drawn.
    uno::Reference<drawing::XShape> xShape(getShape(0));
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("Could not get the shape properties", xShapeProps.is());
    awt::Rectangle aBoundRect;
    xShapeProps->getPropertyValue(UNO_NAME_MISC_OBJ_BOUNDRECT) >>= aBoundRect;
    const double fDiffWmH = static_cast<double>(aBoundRect.Width - aBoundRect.Height);
    // Need some tolerance for line width
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Full circle plus segment expected", 0.0, fDiffWmH, 10.0);
}
void CustomshapesTest::testTdf121845_two_commands_U()
{
    // tdf121845 custom shape with command U (angleellipse) is  wrongly drawn
    // Load a document with path "U 950 250 200 200 90 180 250 250 200 200 180 270 N"
    // Error was, that the second ellipse segment was interpreted as command T and
    // thus a line from first to second segment was drawn.
    const OUString sFileName("tdf121845_Two_commands_U.odg");
    OUString sURL = m_directories.getURLFromSrc(sDataDirectory) + sFileName;
    mxComponent = loadFromDesktop(sURL, "com.sun.star.comp.drawing.DrawingDocument");
    CPPUNIT_ASSERT_MESSAGE("Could not load document", mxComponent.is());
    uno::Reference<drawing::XShape> xShape(getShape(0));
    // In case no line is drawn, two polygons are generated; with line only one polygon
    SdrObjCustomShape& rSdrObjCustomShape(
        static_cast<SdrObjCustomShape&>(*GetSdrObjectFromXShape(xShape)));
    EnhancedCustomShape2d aCustomShape2d(rSdrObjCustomShape);
    SdrPathObj* pPathObj = static_cast<SdrPathObj*>(aCustomShape2d.CreateLineGeometry());
    CPPUNIT_ASSERT_MESSAGE("Could not convert to SdrPathObj", pPathObj);
    const basegfx::B2DPolyPolygon aPolyPolygon(pPathObj->GetPathPoly());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("count polygons", static_cast<sal_uInt32>(2),
                                 aPolyPolygon.count());
}

CPPUNIT_TEST_SUITE_REGISTRATION(CustomshapesTest);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
