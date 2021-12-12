/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <cstdlib>

#include <test/bootstrapfixture.hxx>
#include <unotest/macros_test.hxx>
#include <rtl/ustring.hxx>
#include <editeng/unoprnms.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>
#include <svl/intitem.hxx>
#include <svx/EnhancedCustomShape2d.hxx>
#include <svx/extrusionbar.hxx>
#include <svx/svdoashp.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdview.hxx>
#include <svx/svxids.hrc>
#include <svx/unoapi.hxx>
#include <unotools/mediadescriptor.hxx>
#include <unotools/tempfile.hxx>

#include <cppunit/TestAssert.h>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XStorable.hpp>

using namespace ::com::sun::star;

namespace
{
constexpr OUStringLiteral sDataDirectory(u"svx/qa/unit/data/");

/// Tests for svx/source/customshapes/ code.
class CustomshapesTest : public test::BootstrapFixture, public unotest::MacrosTest
{
protected:
    uno::Reference<lang::XComponent> mxComponent;
    // get shape nShapeIndex from page 0
    uno::Reference<drawing::XShape> getShape(sal_uInt8 nShapeIndex);
    sal_uInt8 countShapes();

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

sal_uInt8 CustomshapesTest::countShapes()
{
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent,
                                                                   uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("Could not get XDrawPagesSupplier", xDrawPagesSupplier.is());
    uno::Reference<drawing::XDrawPages> xDrawPages(xDrawPagesSupplier->getDrawPages());
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPages->getByIndex(0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("Could not get xDrawPage", xDrawPage.is());
    return xDrawPage->getCount();
}

void lcl_AssertRectEqualWithTolerance(std::string_view sInfo, const tools::Rectangle& rExpected,
                                      const tools::Rectangle& rActual, const sal_Int32 nTolerance)
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

CPPUNIT_TEST_FIXTURE(CustomshapesTest, testTdf145904_Extrusion_CenterY_odt)
{
    // The X- and Y-component of the extrusion rotation center specify the position as fraction of
    // shape size. Error was, that the relative fraction was handled as absolute value in Hmm.

    // Load document
    OUString aURL = m_directories.getURLFromSrc(sDataDirectory) + "tdf145904_center_Y0dot25.odt";
    mxComponent = loadFromDesktop(aURL, "com.sun.star.text.TextDocument");

    // The shape is extruded and tilt down 90deg. The rotation center is in the middle between shape
    // center and bottom shape edge. The bottom edge of the projected solid has roughly the
    // y-coordinate of the rotation center.
    uno::Reference<drawing::XShape> xShape(getShape(0));
    uno::Reference<beans::XPropertySet> xPropSet(xShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("Could not get the properties", xPropSet.is());
    awt::Rectangle aBoundRect;
    xPropSet->getPropertyValue(UNO_NAME_MISC_OBJ_BOUNDRECT) >>= aBoundRect;
    awt::Point aAnchorPosition;
    xPropSet->getPropertyValue("AnchorPosition") >>= aAnchorPosition;
    sal_Int32 nActualTop = aBoundRect.Y - aAnchorPosition.Y;

    // Without the fix it would have failed with top = 2252.
    // The tolerance 10 is estimated and can be adjusted if required for HiDPI.
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("top", 3208, nActualTop, 10);
}

CPPUNIT_TEST_FIXTURE(CustomshapesTest, testTdf145904_Extrusion_CenterY_doc)
{
    // The X- and Y-component of the extrusion rotation center specify the position as fraction of
    // shape size. Error was, that the relative fraction was handled as absolute value in EMU.

    // Load document
    OUString aURL = m_directories.getURLFromSrc(sDataDirectory) + "tdf145904_center_Y0dot25.doc";
    mxComponent = loadFromDesktop(aURL, "com.sun.star.text.TextDocument");

    // The shape is extruded and tilt down 90deg. The rotation center is in the middle between shape
    // center and bottom shape edge. The bottom edge of the projected solid has roughly the
    // y-coordinate of the center.
    uno::Reference<drawing::XShape> xShape(getShape(0));
    uno::Reference<beans::XPropertySet> xPropSet(xShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("Could not get the properties", xPropSet.is());
    awt::Rectangle aBoundRect;
    xPropSet->getPropertyValue(UNO_NAME_MISC_OBJ_BOUNDRECT) >>= aBoundRect;
    awt::Point aAnchorPosition;
    xPropSet->getPropertyValue("AnchorPosition") >>= aAnchorPosition;
    sal_Int32 nActualTop = aBoundRect.Y - aAnchorPosition.Y;

    // Without the fix it would have failed with top = 2330
    // The tolerance 10 is estimated and can be adjusted if required for HiDPI.
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("top", 3208, nActualTop, 10);
}

CPPUNIT_TEST_FIXTURE(CustomshapesTest, testTdf145245_ExtrusionPosition)
{
    // The second parameter of the extrusion-depth property specifies how much of the extrusion
    // lies before the shape. The file contains three shapes which have the values 0, 0.5 and 1.
    // They are rotated around the x-axis so that the extrusion becomes visible. The extrusion
    // depth itself is 5cm. Y-coordinate of shape is 6cm.

    // Load document
    OUString aURL = m_directories.getURLFromSrc(sDataDirectory) + "tdf145245_ExtrusionPosition.odp";
    mxComponent = loadFromDesktop(aURL, "com.sun.star.comp.presentation.PresentationDocument");

    // The tolerance 40 is estimated and can be adjusted if required for HiDPI.
    {
        // First shape has extrusion behind the shape.
        uno::Reference<drawing::XShape> xShape0(getShape(0));
        SdrObjCustomShape& rSdrCustomShape(
            static_cast<SdrObjCustomShape&>(*SdrObject::getSdrObjectFromXShape(xShape0)));
        tools::Rectangle aBoundRect(rSdrCustomShape.GetCurrentBoundRect());
        tools::Rectangle aExpected(Point(1000, 1000), Size(6002, 5001));
        lcl_AssertRectEqualWithTolerance("Pos 0.0 extrusion", aExpected, aBoundRect, 40);
    }
    {
        // Second shape has half of extrusion behind the shape.
        uno::Reference<drawing::XShape> xShape(getShape(1));
        SdrObjCustomShape& rSdrCustomShape(
            static_cast<SdrObjCustomShape&>(*SdrObject::getSdrObjectFromXShape(xShape)));
        // Without the fix the height was 1 instead of 5001.
        tools::Rectangle aBoundRect(rSdrCustomShape.GetCurrentBoundRect());
        tools::Rectangle aExpected(Point(9000, 3500), Size(6002, 5001));
        lcl_AssertRectEqualWithTolerance("Pos 0.5 extrusion", aExpected, aBoundRect, 40);
    }
    {
        // Third shape has extrusion before the shape.
        uno::Reference<drawing::XShape> xShape(getShape(2));
        SdrObjCustomShape& rSdrCustomShape(
            static_cast<SdrObjCustomShape&>(*SdrObject::getSdrObjectFromXShape(xShape)));
        // Without the fix the y-coordinate was 1000 instead of 6000.
        tools::Rectangle aBoundRect(rSdrCustomShape.GetCurrentBoundRect());
        tools::Rectangle aExpected(Point(18000, 6000), Size(6002, 5001));
        lcl_AssertRectEqualWithTolerance("Pos 1.0 extrusion", aExpected, aBoundRect, 40);
    }
}

CPPUNIT_TEST_FIXTURE(CustomshapesTest, testTdf145111_Fontwork_rendering_font_size)
{
    // The tested position and height depend on dpi.
    if (!IsDefaultDPI())
        return;

    // tdf#144988 In case ScaleX is true in property TextPath, the rendering font size should be
    // reduced in case any of the paragraphs would be longer as its sub-path. That was wrong, if
    // the first paragraph was too long and the second would fit. It resulted in wrong position
    // and height and overlapping characters.

    OUString aURL = m_directories.getURLFromSrc(sDataDirectory) + "tdf144988_Fontwork_FontSize.odp";
    mxComponent = loadFromDesktop(aURL, "com.sun.star.comp.presentation.PresentationDocument");
    uno::Reference<drawing::XShape> xShape(getShape(0));
    SdrObjCustomShape& rSdrCustomShape(
        static_cast<SdrObjCustomShape&>(*SdrObject::getSdrObjectFromXShape(xShape)));

    // Without the fix in place left|top, width x height was 1279|1279, 2815 x 2448.
    // The expected values 1501|1777, 3941 x 1446 are only valid for 96dpi.
    tools::Rectangle aBoundRect(rSdrCustomShape.GetCurrentBoundRect());
    tools::Rectangle aExpected(Point(1501, 1777), Size(3941, 1446));
    lcl_AssertRectEqualWithTolerance("Wrong text rendering", aExpected, aBoundRect, 5);
}

CPPUNIT_TEST_FIXTURE(CustomshapesTest, testTdf145111_anchor_in_Fontwork)
{
    // The tested positions depend on dpi.
    if (!IsDefaultDPI())
        return;

    // tdf#145004 In case ScaleX is true in property TextPath, SDRTEXTVERTADJUST is
    // evaluated and should shift the Fontwork text. That did not work for
    // 'Top-Left' and 'Bottom-Left'.

    // Load document
    OUString aURL = m_directories.getURLFromSrc(sDataDirectory) + "tdf145111_TL_BL_Fontwork.odp";
    mxComponent = loadFromDesktop(aURL, "com.sun.star.comp.presentation.PresentationDocument");

    {
        // First shape has anchor set to Top-Left, which shifts Fontwork text down.
        uno::Reference<drawing::XShape> xShape(getShape(0));
        SdrObjCustomShape& rSdrCustomShape(
            static_cast<SdrObjCustomShape&>(*SdrObject::getSdrObjectFromXShape(xShape)));

        // Without the fix in place top was 2295, but should be 2916 for 96dpi.
        // Was 2184, should be 2886 for 120dpi.
        tools::Rectangle aBoundRect(rSdrCustomShape.GetCurrentBoundRect());
        CPPUNIT_ASSERT_DOUBLES_EQUAL(tools::Long(2916), aBoundRect.Top(), 5);
    }
    {
        // Second shape has anchor set to Bottom-Left, which shifts Fontwork text up.
        uno::Reference<drawing::XShape> xShape(getShape(1));
        SdrObjCustomShape& rSdrCustomShape(
            static_cast<SdrObjCustomShape&>(*SdrObject::getSdrObjectFromXShape(xShape)));

        // Without the fix in place top was 10294, but should be 9519 for 96dpi.
        // Was 10184, should be 9481 for 120dpi.
        tools::Rectangle aBoundRect(rSdrCustomShape.GetCurrentBoundRect());
        CPPUNIT_ASSERT_DOUBLES_EQUAL(tools::Long(9519), aBoundRect.Top(), 5);
    }
}

CPPUNIT_TEST_FIXTURE(CustomshapesTest, testTdf145004_gap_by_ScaleX)
{
    if (!IsDefaultDPI())
        return;
    // tdf#145004 In case property ScaleX=true was set in property 'TextPath' an additional
    // padding was added to the scaling factor. That results in a gap at start or/and end of
    // the text. Such gap should not be there.

    // Load document and get shape. It is a custom shape from pptx import of a WordArt of
    // kind 'Follow Path'.
    OUString aURL = m_directories.getURLFromSrc(sDataDirectory) + "tdf145004_gap_by_ScaleX.pptx";
    mxComponent = loadFromDesktop(aURL, "com.sun.star.comp.presentation.PresentationDocument");
    uno::Reference<drawing::XShape> xShape(getShape(0));
    SdrObjCustomShape& rSdrCustomShape(
        static_cast<SdrObjCustomShape&>(*SdrObject::getSdrObjectFromXShape(xShape)));

    // Verify width. Without the fix in place the width was 8231, but should be 8496 for 96dpi.
    // Was 8328, should be 8527 for 120dpi.
    tools::Rectangle aBoundRect(rSdrCustomShape.GetCurrentBoundRect());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(tools::Long(8496), aBoundRect.GetWidth(), 5);
}

CPPUNIT_TEST_FIXTURE(CustomshapesTest, testTdf141021ExtrusionNorth)
{
    // tdf#141021 Setting extrusion direction in projection method 'perspective' to
    // 'Extrusion North' had used a wrong origin for the ViewPoint and thus the
    // side faces were wrong calculated.

    // Load document and get shape. It is a custom shape in 3D mode.
    OUString aURL = m_directories.getURLFromSrc(sDataDirectory) + "tdf141021_ExtrusionNorth.odp";
    mxComponent = loadFromDesktop(aURL, "com.sun.star.comp.presentation.PresentationDocument");
    uno::Reference<drawing::XShape> xShape(getShape(0));
    SdrObjCustomShape& rSdrCustomShape(
        static_cast<SdrObjCustomShape&>(*SdrObject::getSdrObjectFromXShape(xShape)));

    // Mark Object
    SfxViewShell* pViewShell = SfxViewShell::Current();
    SdrView* pSdrView = pViewShell->GetDrawView();
    pSdrView->MarkObj(&rSdrCustomShape, pSdrView->GetSdrPageView());

    // Set direction
    SfxRequest aReq(pViewShell->GetViewFrame(), SID_EXTRUSION_DIRECTION);
    SfxInt32Item aItem(SID_EXTRUSION_DIRECTION, 90);
    aReq.AppendItem(aItem);
    svx::ExtrusionBar::execute(pSdrView, aReq, SfxViewFrame::Current()->GetBindings());

    // Verify height. Without the fix in place the height would 4001.
    tools::Rectangle aBoundRect(rSdrCustomShape.GetCurrentBoundRect());
    CPPUNIT_ASSERT_EQUAL(tools::Long(5895), aBoundRect.GetHeight());
}

CPPUNIT_TEST_FIXTURE(CustomshapesTest, testResizeRotatedShape)
{
    // tdf#138945 Setting width or height for a rotated or sheared shape in the Position&Size dialog
    // had resulted in a mismatch of handle position and shape outline. That becomes visible in object
    // properties as mismatch of frame rectangle and bound rectangle.
    // Problem was, that fObjectRotation was not updated.

    // Load document and get shape. It is a rectangle custom shape with 45° shear and 330° rotation.
    OUString aURL
        = m_directories.getURLFromSrc(sDataDirectory) + "tdf138945_resizeRotatedShape.odg";
    mxComponent = loadFromDesktop(aURL, "com.sun.star.comp.presentation.PresentationDocument");
    uno::Reference<drawing::XShape> xShape(getShape(0));

    // Change height and mirror vertical
    {
        SdrObjCustomShape& rSdrShape(
            static_cast<SdrObjCustomShape&>(*SdrObject::getSdrObjectFromXShape(xShape)));
        rSdrShape.NbcResize(rSdrShape.GetRelativePos(), Fraction(1.0), Fraction(-0.5));
        tools::Rectangle aSnapRect(rSdrShape.GetSnapRect());
        tools::Rectangle aBoundRect(rSdrShape.GetCurrentBoundRect());
        lcl_AssertRectEqualWithTolerance("height changed, mirror vert", aSnapRect, aBoundRect, 3);
    }

    // Change height
    {
        SdrObjCustomShape& rSdrShape(
            static_cast<SdrObjCustomShape&>(*SdrObject::getSdrObjectFromXShape(xShape)));
        rSdrShape.NbcResize(rSdrShape.GetRelativePos(), Fraction(1.0), Fraction(2.0));
        tools::Rectangle aSnapRect(rSdrShape.GetSnapRect());
        tools::Rectangle aBoundRect(rSdrShape.GetCurrentBoundRect());
        lcl_AssertRectEqualWithTolerance("height changed", aSnapRect, aBoundRect, 3);
    }

    // Change width
    {
        SdrObjCustomShape& rSdrShape(
            static_cast<SdrObjCustomShape&>(*SdrObject::getSdrObjectFromXShape(xShape)));
        rSdrShape.NbcResize(rSdrShape.GetRelativePos(), Fraction(2.0), Fraction(1.0));
        tools::Rectangle aSnapRect(rSdrShape.GetSnapRect());
        tools::Rectangle aBoundRect(rSdrShape.GetCurrentBoundRect());
        lcl_AssertRectEqualWithTolerance("width changed", aSnapRect, aBoundRect, 3);
    }

    // Change width and mirror horizontal
    {
        SdrObjCustomShape& rSdrShape(
            static_cast<SdrObjCustomShape&>(*SdrObject::getSdrObjectFromXShape(xShape)));
        rSdrShape.NbcResize(rSdrShape.GetRelativePos(), Fraction(-0.5), Fraction(1.0));
        tools::Rectangle aSnapRect(rSdrShape.GetSnapRect());
        tools::Rectangle aBoundRect(rSdrShape.GetCurrentBoundRect());
        lcl_AssertRectEqualWithTolerance("width changed, mirror hori", aSnapRect, aBoundRect, 3);
    }
}

CPPUNIT_TEST_FIXTURE(CustomshapesTest, testViewBoxLeftTop)
{
    // tdf#121890 formula values "left" and "top" are wrongly calculated
    // Load a document with two custom shapes of type "non-primitive"
    OUString aURL
        = m_directories.getURLFromSrc(sDataDirectory) + "viewBox_positive_twolines_strict.odp";
    mxComponent = loadFromDesktop(aURL, "com.sun.star.comp.presentation.PresentationDocument");
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
    CPPUNIT_ASSERT_LESS(sal_Int32(3), std::abs(aFrameRectLR.X - aBoundRectLR.X));

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
    CPPUNIT_ASSERT_LESS(sal_Int32(3), std::abs(aFrameRectTB.Y - aBoundRectTB.Y));
}

CPPUNIT_TEST_FIXTURE(CustomshapesTest, testAccuracyCommandX)
{
    // 121761 Increase accuracy of quarter circles drawn by command X or Y
    // The loaded document has a quarter circle with radius 10000 (unit 1/100 mm)
    // which is rotated by 45deg. The test considers the segment.
    OUString aURL
        = m_directories.getURLFromSrc(sDataDirectory) + "tdf121761_Accuracy_command_X.odp";
    mxComponent = loadFromDesktop(aURL, "com.sun.star.comp.presentation.PresentationDocument");
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

CPPUNIT_TEST_FIXTURE(CustomshapesTest, testToggleCommandXY)
{
    // 121952 Toggle x- and y-direction if command X has several parameters
    // The loaded document has a shape with command X and two parameter placed on a diagonal.
    // The radius of the quarter circles are both 10000 (unit 1/100 mm).
    // The shape is rotated by 45deg, so you get two segments, one up and one down.
    OUString aURL
        = m_directories.getURLFromSrc(sDataDirectory) + "tdf121952_Toggle_direction_command_X.odp";
    mxComponent = loadFromDesktop(aURL, "com.sun.star.comp.presentation.PresentationDocument");
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

CPPUNIT_TEST_FIXTURE(CustomshapesTest, testMultipleMoveTo)
{
    // tdf122964 Multiple moveTo has to be treated as lineTo in draw:enhanced-path
    // Load a document with path "M 0 0 5 10 10 0 N"
    OUString aURL = m_directories.getURLFromSrc(sDataDirectory) + "tdf122964_MultipleMoveTo.odg";
    mxComponent = loadFromDesktop(aURL, "com.sun.star.comp.drawing.DrawingDocument");
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

CPPUNIT_TEST_FIXTURE(CustomshapesTest, testWidthOrientationCommandU)
{
    // tdf121845 custom shape with command U (angleellipse) is  wrongly drawn
    // Load a document with path "M 750 0 L 750 500 250 500 250 0 U 500 0 500 500 0 180 N"
    // in viewBox="0 0 1000 500" and width="10cm", height="5cm".
    const OUString sURL
        = m_directories.getURLFromSrc(sDataDirectory) + "tdf121845_WidthOrientation_command_U.odg";
    mxComponent = loadFromDesktop(sURL, "com.sun.star.comp.drawing.DrawingDocument");
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

CPPUNIT_TEST_FIXTURE(CustomshapesTest, testHalfEllipseVML)
{
    // tdf121845 custom shape with command U (angleellipse) is  wrongly drawn
    // Load a document which was converted from VML to doc by Word. It had a VML
    // path="m750,al500,,500,500,,-11796480e" resulting in a lower half circle.
    const OUString sURL
        = m_directories.getURLFromSrc(sDataDirectory) + "tdf121845_HalfEllipseVML.doc";
    mxComponent = loadFromDesktop(sURL, "com.sun.star.comp.text.TextDocument");
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

CPPUNIT_TEST_FIXTURE(CustomshapesTest, testLargeSwingAngleVML)
{
    // tdf121845 custom shape with command U (angleellipse) is  wrongly drawn
    // Load a document which was converted from VML to doc by Word. It had a VML
    // path="al50,50,45,45,2621440,31457280e" resulting in a full circle plus 120 deg segment.
    const OUString sURL
        = m_directories.getURLFromSrc(sDataDirectory) + "tdf121845_start40_swing480.doc";
    mxComponent = loadFromDesktop(sURL, "com.sun.star.comp.text.TextDocument");
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
CPPUNIT_TEST_FIXTURE(CustomshapesTest, testTdf121845_two_commands_U)
{
    // tdf121845 custom shape with command U (angleellipse) is  wrongly drawn
    // Load a document with path "U 950 250 200 200 90 180 250 250 200 200 180 270 N"
    // Error was, that the second ellipse segment was interpreted as command T and
    // thus a line from first to second segment was drawn.
    OUString sURL = m_directories.getURLFromSrc(sDataDirectory) + "tdf121845_Two_commands_U.odg";
    mxComponent = loadFromDesktop(sURL, "com.sun.star.comp.drawing.DrawingDocument");
    uno::Reference<drawing::XShape> xShape(getShape(0));
    // In case no line is drawn, two polygons are generated; with line only one polygon
    SdrObjCustomShape& rSdrObjCustomShape(
        static_cast<SdrObjCustomShape&>(*SdrObject::getSdrObjectFromXShape(xShape)));
    EnhancedCustomShape2d aCustomShape2d(rSdrObjCustomShape);
    SdrPathObjUniquePtr pPathObj(
        static_cast<SdrPathObj*>(aCustomShape2d.CreateLineGeometry().release()));
    CPPUNIT_ASSERT_MESSAGE("Could not convert to SdrPathObj", pPathObj);
    const basegfx::B2DPolyPolygon aPolyPolygon(pPathObj->GetPathPoly());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("count polygons", static_cast<sal_uInt32>(2),
                                 aPolyPolygon.count());
}

CPPUNIT_TEST_FIXTURE(CustomshapesTest, testTdf124212_handle_position)
{
    // tdf124212 Adjustment handle reacts wrongly, if custom shape has a non
    // default viewBox. Load a document with svg:viewBox="10800 0 10800 21600"
    // Error was, that moving the controller results in a handle position that
    // does not reflect the movement.
    OUString sURL = m_directories.getURLFromSrc(sDataDirectory) + "tdf124212_handle_position.odg";
    mxComponent = loadFromDesktop(sURL, "com.sun.star.comp.drawing.DrawingDocument");
    uno::Reference<drawing::XShape> xShape(getShape(0));
    // The shape has one, horizontal adjust handle.
    SdrObjCustomShape& rSdrObjCustomShape(
        static_cast<SdrObjCustomShape&>(*SdrObject::getSdrObjectFromXShape(xShape)));
    EnhancedCustomShape2d aCustomShape2d(rSdrObjCustomShape);
    Point aInitialPosition;
    aCustomShape2d.GetHandlePosition(0, aInitialPosition);
    css::awt::Point aDesiredPosition(aInitialPosition.X() + 1000, aInitialPosition.Y());
    aCustomShape2d.SetHandleControllerPosition(0, aDesiredPosition);
    Point aObservedPosition;
    aCustomShape2d.GetHandlePosition(0, aObservedPosition);
    sal_Int32 nDesiredX(aDesiredPosition.X); // awt::Point
    sal_Int32 nObservedX(aObservedPosition.X()); // tools::Point
    CPPUNIT_ASSERT_EQUAL_MESSAGE("handle X coordinate", nDesiredX, nObservedX);
}

CPPUNIT_TEST_FIXTURE(CustomshapesTest, testTdf124029_arc_position)
{
    // tdf121029 MS binary custom shape mso_sptArc has wrong position
    // MS uses the sector for position reference. Error was, that
    // LibreOffice has used the underlying ellipse.
    OUString sURL = m_directories.getURLFromSrc(sDataDirectory) + "tdf124029_Arc_position.doc";
    mxComponent = loadFromDesktop(sURL, "com.sun.star.comp.text.TextDocument");
    uno::Reference<drawing::XShape> xShape(getShape(0));
    // The visual wrong position is due to a wrong shape width.
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("Could not get the shape properties", xShapeProps.is());
    awt::Rectangle aFrameRect;
    xShapeProps->getPropertyValue(UNO_NAME_MISC_OBJ_FRAMERECT) >>= aFrameRect;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("shape width", static_cast<sal_uInt32>(1610),
                                 static_cast<sal_uInt32>(aFrameRect.Width));
}

CPPUNIT_TEST_FIXTURE(CustomshapesTest, testTdf124740_handle_path_coordsystem)
{
    // tdf124740 OOXML shape with handle and w and h attribute on path has wrong
    // handle position
    // The handle position was scaled erroneously twice.
    OUString sURL
        = m_directories.getURLFromSrc(sDataDirectory) + "tdf124740_HandleInOOXMLUserShape.pptx";
    mxComponent = loadFromDesktop(sURL, "com.sun.star.comp.drawing.DrawingDocument");
    uno::Reference<drawing::XShape> xShape(getShape(0));
    // The shape has one, horizontal adjust handle. It is about 1/5 of 10cm from left
    // shape edge, shape is 6cm from left . That results in a position
    // of 8cm from left page edge, which is 8000 in 1/100 mm unit.
    SdrObjCustomShape& rSdrObjCustomShape(
        static_cast<SdrObjCustomShape&>(*SdrObject::getSdrObjectFromXShape(xShape)));
    EnhancedCustomShape2d aCustomShape2d(rSdrObjCustomShape);
    Point aPosition;
    aCustomShape2d.GetHandlePosition(0, aPosition);
    double fX(aPosition.X());
    // tolerance for rounding to integer
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("handle X coordinate", 8000.0, fX, 2.0);
}

CPPUNIT_TEST_FIXTURE(CustomshapesTest, testTdf115813_OOXML_XY_handle)
{
    // The test covers all preset shapes with handles. Only these ones are
    // excluded: arc, blockArc, chord, circularArrow, gear6, gear9, mathNotEqual, pie,
    // leftCircularArrow, leftRightCircularArrow, swooshArrow.
    // Connectors are included as ordinary shapes to prevent converting.
    // Error was, that the handle movement and the changes to the shape did not follow
    // the mouse movement.
    OUString sURL = m_directories.getURLFromSrc(sDataDirectory)
                    + "tdf115813_HandleMovementOOXMLPresetShapes.pptx";
    mxComponent = loadFromDesktop(sURL, "com.sun.star.comp.drawing.DrawingDocument");

    OUString sErrors;
    // values in vector InteractionsHandles are in 1/100 mm and refer to page
    for (sal_uInt8 i = 0; i < countShapes(); i++)
    {
        uno::Reference<drawing::XShape> xShape(getShape(i));
        SdrObjCustomShape& rSdrObjCustomShape(
            static_cast<SdrObjCustomShape&>(*SdrObject::getSdrObjectFromXShape(xShape)));
        OUString sShapeType("non-primitive"); // default for ODF
        const SdrCustomShapeGeometryItem& rGeometryItem(
            rSdrObjCustomShape.GetMergedItem(SDRATTR_CUSTOMSHAPE_GEOMETRY));
        const uno::Any* pAny = rGeometryItem.GetPropertyValueByName("Type");
        if (pAny)
            *pAny >>= sShapeType;

        sal_uInt8 nHandlesCount = rSdrObjCustomShape.GetInteractionHandles().size();
        for (sal_uInt8 j = 0; j < nHandlesCount; j++)
        {
            css::awt::Point aInitialPosition(
                rSdrObjCustomShape.GetInteractionHandles()[j].aPosition);
            // The handles are initialized in the test document, so that if the handle is moveable in
            // that direction at all, then it can move at least with an amount of 100.
            Point aDesiredPosition(aInitialPosition.X + 100, aInitialPosition.Y + 100);
            rSdrObjCustomShape.DragMoveCustomShapeHdl(aDesiredPosition, j, false);
            css::awt::Point aObservedPosition(
                rSdrObjCustomShape.GetInteractionHandles()[j].aPosition);
            sal_Int32 nDesiredX(aDesiredPosition.X()); // tools::Point
            sal_Int32 nDesiredY(aDesiredPosition.Y());
            sal_Int32 nObservedX(aObservedPosition.X); // css::awt::Point
            sal_Int32 nObservedY(aObservedPosition.Y);
            // If a handle only moves in one direction, the difference is 100 for the other direction.
            // There exists some rounding differences, therefore '<= 1' instead of '== 0'.
            // The condition has the form '!(good cases)'.
            if (!((abs(nDesiredX - nObservedX) <= 1 && abs(nDesiredY - nObservedY) == 100)
                  || (abs(nDesiredX - nObservedX) == 100 && abs(nDesiredY - nObservedY) <= 1)
                  || (abs(nDesiredX - nObservedX) <= 1 && abs(nDesiredY - nObservedY) <= 1)))
            {
                sErrors += "\n" +
                           //sErrors += OUString(sal_Unicode(10));
                           OUString::number(i) + " " + sShapeType + ": " + OUString::number(j)
                           + " X " + OUString::number(nDesiredX) + "|"
                           + OUString::number(nObservedX) + " Y " + OUString::number(nDesiredY)
                           + "|" + OUString::number(nObservedY);
            }
        }
    }
    CPPUNIT_ASSERT_EQUAL(OUString(), sErrors);
}

CPPUNIT_TEST_FIXTURE(CustomshapesTest, testQuadraticCurveTo)
{
    // tdf125782 command Q (quadraticcurveto) uses wrong 'current point'.
    // When converting to cubic Bezier curve, this had resulted in a wrong first control point.
    // The quadraticcurveto segment starts in shape center in the test file. The first control
    // point should produce a horizontal tangent in the start point.
    const OUString sURL
        = m_directories.getURLFromSrc(sDataDirectory) + "tdf125782_QuadraticCurveTo.odg";
    mxComponent = loadFromDesktop(sURL, "com.sun.star.comp.drawing.DrawingDocument");
    uno::Reference<drawing::XShape> xShape(getShape(0));
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("Could not get the shape properties", xShapeProps.is());
    awt::Rectangle aBoundRect;
    xShapeProps->getPropertyValue(UNO_NAME_MISC_OBJ_BOUNDRECT) >>= aBoundRect;
    const double fHeight = static_cast<double>(aBoundRect.Height);
    //Add some tolerance
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("bad height of quadraticcurveto", 3004, fHeight, 10.0);
}

CPPUNIT_TEST_FIXTURE(CustomshapesTest, testTdf126512_OOXML_handle_in_ODP)
{
    // The test covers all preset shapes with handles. Connectors are included as ordinary
    // shapes to prevent converting. The file was created in PowerPoint 365 and then
    // opened and exported to ODF format by LibreOffice.
    // Error was, that for shapes, which were originally imported from OOXML, the handles
    // could not be moved at all.
    OUString sURL
        = m_directories.getURLFromSrc(sDataDirectory) + "tdf126512_OOXMLHandleMovementInODF.odp";
    mxComponent = loadFromDesktop(sURL, "com.sun.star.comp.drawing.DrawingDocument");

    OUString sErrors; // sErrors collects shape type and handle index for failing cases
    for (sal_uInt8 i = 0; i < countShapes(); i++)
    {
        uno::Reference<drawing::XShape> xShape(getShape(i));
        SdrObjCustomShape& rSdrObjCustomShape(
            static_cast<SdrObjCustomShape&>(*SdrObject::getSdrObjectFromXShape(xShape)));
        OUString sShapeType("non-primitive"); // only to initialize, value not used here
        const SdrCustomShapeGeometryItem& rGeometryItem(
            rSdrObjCustomShape.GetMergedItem(SDRATTR_CUSTOMSHAPE_GEOMETRY));
        const uno::Any* pAny = rGeometryItem.GetPropertyValueByName("Type");
        if (pAny)
            *pAny >>= sShapeType;

        sal_uInt8 nHandlesCount = rSdrObjCustomShape.GetInteractionHandles().size();
        for (sal_uInt8 j = 0; j < nHandlesCount; j++)
        {
            css::awt::Point aInitialPosition(
                rSdrObjCustomShape.GetInteractionHandles()[j].aPosition);
            // The handles are initialized in the test document, so that if the handle is moveable
            // in that direction at all, then it can move at least with an amount of 100.
            Point aDesiredPosition(aInitialPosition.X + 100, aInitialPosition.Y + 100);
            rSdrObjCustomShape.DragMoveCustomShapeHdl(aDesiredPosition, j, false);
            css::awt::Point aObservedPosition(
                rSdrObjCustomShape.GetInteractionHandles()[j].aPosition);
            if (aInitialPosition.X == aObservedPosition.X
                && aInitialPosition.Y == aObservedPosition.Y)
            {
                sErrors
                    += "\n" + OUString::number(i) + " " + sShapeType + "  " + OUString::number(j);
            }
        }
    }
    CPPUNIT_ASSERT_EQUAL(OUString(), sErrors);
}

CPPUNIT_TEST_FIXTURE(CustomshapesTest, testTdf127785_Mirror)
{
    // The document contains two shapes, one with horizontal flip, the other with vertical
    // flip. They are diamonds, so their text frame is symmetric to the center of the shape.
    // The shapes have not stroke and no fill, so that the bounding box surrounds the text
    // and therefore equals approximately the text frame.
    // Error was, that because of wrong calculation, the flipped shapes do not use the
    // text frame but the frame rectangle for their text.
    OUString sURL = m_directories.getURLFromSrc(sDataDirectory) + "tdf127785_Mirror.odp";
    mxComponent = loadFromDesktop(sURL, "com.sun.star.comp.drawing.DrawingDocument");
    OUString sErrors; // sErrors collects the errors and should be empty in case all is OK.

    uno::Reference<drawing::XShape> xShapeV(getShape(0));
    uno::Reference<beans::XPropertySet> xShapeVProps(xShapeV, uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("Could not get the shape properties", xShapeVProps.is());
    awt::Rectangle aBoundRectV;
    xShapeVProps->getPropertyValue(UNO_NAME_MISC_OBJ_BOUNDRECT) >>= aBoundRectV;
    const sal_Int32 nHeightV = aBoundRectV.Height;
    const sal_Int32 nWidthV = aBoundRectV.Width;
    const sal_Int32 nLeftV = aBoundRectV.X;
    const sal_Int32 nTopV = aBoundRectV.Y;
    if (abs(nHeightV - 8000) > 10 || abs(nWidthV - 8000) > 10)
        sErrors += "Flip vertical wrong size.";
    if (abs(nLeftV - 1000) > 10 || abs(nTopV - 2000) > 10)
        sErrors += " Flip vertical wrong position.";

    uno::Reference<drawing::XShape> xShapeH(getShape(1));
    uno::Reference<beans::XPropertySet> xShapeHProps(xShapeH, uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("Could not get the shape properties", xShapeHProps.is());
    awt::Rectangle aBoundRectH;
    xShapeHProps->getPropertyValue(UNO_NAME_MISC_OBJ_BOUNDRECT) >>= aBoundRectH;
    const sal_Int32 nHeightH = aBoundRectH.Height;
    const sal_Int32 nWidthH = aBoundRectH.Width;
    const sal_Int32 nLeftH = aBoundRectH.X;
    const sal_Int32 nTopH = aBoundRectH.Y;
    if (abs(nHeightH - 8000) > 10 || abs(nWidthH - 8000) > 10)
        sErrors += " Flip horizontal wrong size.";
    if (abs(nLeftH - 13000) > 10 || abs(nTopH - 2000) > 10)
        sErrors += " Flip horizontal wrong position.";

    CPPUNIT_ASSERT_EQUAL(OUString(), sErrors);
}

CPPUNIT_TEST_FIXTURE(CustomshapesTest, testTdf126060_3D_Z_Rotation)
{
    // The document contains one textbox with inside overflowed text
    // and the text has 3D z rotation. When we open the document we
    // should see the text vertically and rotated from text bound center not text box.

    OUString sURL = m_directories.getURLFromSrc(sDataDirectory) + "tdf126060_3D_Z_Rotation.pptx";
    mxComponent = loadFromDesktop(sURL, "com.sun.star.comp.drawing.DrawingDocument");

    uno::Reference<drawing::XShape> xShape(getShape(0));
    SdrObjCustomShape& rSdrObjCustomShape(
        static_cast<SdrObjCustomShape&>(*SdrObject::getSdrObjectFromXShape(xShape)));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong text camera Z rotation", 90.0,
                                 rSdrObjCustomShape.GetCameraZRotation());

    basegfx::B2DHomMatrix aObjectTransform;
    basegfx::B2DPolyPolygon aObjectPolyPolygon;
    rSdrObjCustomShape.TRGetBaseGeometry(aObjectTransform, aObjectPolyPolygon);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong transformation at 0,0 position", 1492.0,
                                 aObjectTransform.get(0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong transformation at 0,1 position", 0.0,
                                 aObjectTransform.get(0, 1));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong transformation at 0,2 position", 1129.0,
                                 aObjectTransform.get(0, 2));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong transformation at 1,0 position", 0.0,
                                 aObjectTransform.get(1, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong transformation at 1,1 position", 2500.0,
                                 aObjectTransform.get(1, 1));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong transformation at 1,2 position", 5846.0,
                                 aObjectTransform.get(1, 2));
}

CPPUNIT_TEST_FIXTURE(CustomshapesTest, testTdf127785_Asymmetric)
{
    // The document contains a shapes with vertical flip and text frame asymmetrical
    // to shape. The shape has not stroke and no fill, so that the bounding box surrounds
    // the text and therefore equals approximately the text frame.
    // Error was, that the 180deg text rotation was not compensated for the position of
    // the flipped text box.
    OUString sURL
        = m_directories.getURLFromSrc(sDataDirectory) + "tdf127785_asymmetricTextBoxFlipV.odg";
    mxComponent = loadFromDesktop(sURL, "com.sun.star.comp.drawing.DrawingDocument");
    OUString sErrors; // sErrors collects the errors and should be empty in case all is OK.

    uno::Reference<drawing::XShape> xShape(getShape(0));
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("Could not get the shape properties", xShapeProps.is());
    awt::Rectangle aBoundRect;
    xShapeProps->getPropertyValue(UNO_NAME_MISC_OBJ_BOUNDRECT) >>= aBoundRect;
    const sal_Int32 nLeft = aBoundRect.X;
    const sal_Int32 nTop = aBoundRect.Y;
    const sal_Int32 nRight = aBoundRect.X + aBoundRect.Width - 1;
    const sal_Int32 nBottom = aBoundRect.Y + aBoundRect.Height - 1;
    if (abs(nLeft - 9000) > 10)
        sErrors += "wrong left";
    if (abs(nRight - 19000) > 10)
        sErrors += " wrong right";
    if (abs(nTop - 3000) > 10)
        sErrors += " wrong top";
    if (abs(nBottom - 18000) > 10)
        sErrors += " wrong bottom";

    CPPUNIT_ASSERT_EQUAL(OUString(), sErrors);
}

CPPUNIT_TEST_FIXTURE(CustomshapesTest, testTdf127785_TextRotateAngle)
{
    // The document contains a shapes with vertical flip and a text frame with own
    // rotate angle. The shape has not stroke and no fill, so that the bounding box
    // surrounds the text and therefore equals approximately the text frame.
    // Error was, that the compensation for the 180° rotation added for vertical
    // flip were not made to the text box position but to the text matrix.
    OUString sURL = m_directories.getURLFromSrc(sDataDirectory) + "tdf127785_TextRotateAngle.odp";
    mxComponent = loadFromDesktop(sURL, "com.sun.star.comp.drawing.DrawingDocument");
    OUString sErrors; // sErrors collects the errors and should be empty in case all is OK.

    uno::Reference<drawing::XShape> xShape(getShape(0));
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("Could not get the shape properties", xShapeProps.is());
    awt::Rectangle aBoundRect;
    xShapeProps->getPropertyValue(UNO_NAME_MISC_OBJ_BOUNDRECT) >>= aBoundRect;
    const sal_Int32 nLeft = aBoundRect.X;
    const sal_Int32 nTop = aBoundRect.Y;
    const sal_Int32 nRight = aBoundRect.X + aBoundRect.Width - 1;
    const sal_Int32 nBottom = aBoundRect.Y + aBoundRect.Height - 1;
    if (abs(nLeft - 2000) > 10)
        sErrors += "wrong left";
    if (abs(nRight - 14000) > 10)
        sErrors += " wrong right";
    if (abs(nTop - 3000) > 10)
        sErrors += " wrong top";
    if (abs(nBottom - 9000) > 10)
        sErrors += " wrong bottom";

    CPPUNIT_ASSERT_EQUAL(OUString(), sErrors);
}

CPPUNIT_TEST_FIXTURE(CustomshapesTest, testTdf128413_tbrlOnOff)
{
    // The document contains a rotated shape with text. The error was, that switching
    // tb-rl writing-mode on, changed the shape size and position.

    OUString sURL = m_directories.getURLFromSrc(sDataDirectory) + "tdf128413_tbrl_OnOff.odp";
    mxComponent = loadFromDesktop(sURL, "com.sun.star.comp.drawing.DrawingDocument");
    uno::Reference<drawing::XShape> xShape(getShape(0));
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("Could not get the shape properties", xShapeProps.is());
    awt::Rectangle aOrigRect;
    xShapeProps->getPropertyValue(UNO_NAME_MISC_OBJ_FRAMERECT) >>= aOrigRect;

    SdrObjCustomShape& rSdrObjCustomShape(
        static_cast<SdrObjCustomShape&>(*SdrObject::getSdrObjectFromXShape(xShape)));
    rSdrObjCustomShape.SetVerticalWriting(true);

    awt::Rectangle aObservedRect;
    xShapeProps->getPropertyValue(UNO_NAME_MISC_OBJ_FRAMERECT) >>= aObservedRect;

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Shape has wrong width", aOrigRect.Width, aObservedRect.Width);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Shape has wrong height", aOrigRect.Height, aObservedRect.Height);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Shape has wrong X position", aOrigRect.X, aObservedRect.X);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Shape has wrong Y position", aOrigRect.Y, aObservedRect.Y);
}

CPPUNIT_TEST_FIXTURE(CustomshapesTest, testTdf129532_MatrixFlipV)
{
    // The document contains two rotated shapes with the same geometry. For one of them
    // "matrix(1 0 0 -1 0cm 0cm)" was manually added to the value of the draw:transform
    // attribute. That should result in mirroring on the x-axis. Error was, that the lines
    // which are drawn on the shape rectangle were mirrored, but not the rectangle itself.
    // The rectangle was only shifted.
    OUString sURL = m_directories.getURLFromSrc(sDataDirectory) + "tdf129532_MatrixFlipV.odg";
    mxComponent = loadFromDesktop(sURL, "com.sun.star.comp.drawing.DrawingDocument");
    OUString sErrors; // sErrors collects the errors and should be empty in case all is OK.

    uno::Reference<drawing::XShape> xShape0(getShape(0));
    uno::Reference<beans::XPropertySet> xShape0Props(xShape0, uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("Could not get the shape properties", xShape0Props.is());
    awt::Rectangle aBoundRect0;
    xShape0Props->getPropertyValue(UNO_NAME_MISC_OBJ_BOUNDRECT) >>= aBoundRect0;

    uno::Reference<drawing::XShape> xShape1(getShape(1));
    uno::Reference<beans::XPropertySet> xShape1Props(xShape1, uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("Could not get the shape properties", xShape1Props.is());
    awt::Rectangle aBoundRect1;
    xShape1Props->getPropertyValue(UNO_NAME_MISC_OBJ_BOUNDRECT) >>= aBoundRect1;

    // The size of the two BoundRect rectangles are the same in case of correct
    // vertical mirroring.
    if (aBoundRect0.Width != aBoundRect1.Width)
    {
        sErrors += "\n Width expected: " + OUString::number(aBoundRect1.Width)
                   + " actual: " + OUString::number(aBoundRect0.Width);
    }
    if (aBoundRect0.Height != aBoundRect1.Height)
    {
        sErrors += "\n Height expected: " + OUString::number(aBoundRect1.Height)
                   + " actual: " + OUString::number(aBoundRect0.Height);
    }
    CPPUNIT_ASSERT_EQUAL(OUString(), sErrors);
}

CPPUNIT_TEST_FIXTURE(CustomshapesTest, testTdf103474_commandT_CaseZeroHeight)
{
    // tdf103474 custom shape with command T to create quarter ellipses in a bracket,
    // corner case where the ellipse has zero height.
    // Error was, that the calculation of the circle angle from the ellipse
    // angle results in a wrong angle for the case 180° and height zero.
    OUString sURL
        = m_directories.getURLFromSrc(sDataDirectory) + "tdf103474_commandT_CaseZeroHeight.odp";
    mxComponent = loadFromDesktop(sURL, "com.sun.star.comp.presentation.PresentationDocument");
    uno::Reference<drawing::XShape> xShape(getShape(0));
    // The end points of the straight line segment should have the same x-coordinate of left
    // of shape, and different y-coordinates, one top and the other bottom of the shape.
    SdrObjCustomShape& rSdrObjCustomShape(
        static_cast<SdrObjCustomShape&>(*SdrObject::getSdrObjectFromXShape(xShape)));
    EnhancedCustomShape2d aCustomShape2d(rSdrObjCustomShape);
    SdrPathObjUniquePtr pPathObj(
        static_cast<SdrPathObj*>(aCustomShape2d.CreateLineGeometry().release()));
    CPPUNIT_ASSERT_MESSAGE("Could not convert to SdrPathObj", pPathObj);
    const basegfx::B2DPolyPolygon aPolyPolygon(pPathObj->GetPathPoly());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("count polygons", static_cast<sal_uInt32>(1),
                                 aPolyPolygon.count());
    const basegfx::B2DPolygon aPolygon(aPolyPolygon.getB2DPolygon(0));
    // Get the middle points of the polygon. They are the endpoints of the
    // straight line segment regardless of the quarter ellipse parts, because
    // the shape is symmetric.
    const basegfx::B2DPoint aStart(aPolygon.getB2DPoint(aPolygon.count() / 2 - 1));
    const basegfx::B2DPoint aEnd(aPolygon.getB2DPoint(aPolygon.count() / 2));
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("aStart x-coordinate", 13999.0, aStart.getX(), 1.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("aEnd x-coordinate", 13999.0, aEnd.getX(), 1.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("aStart y-coordinate", 9999.0, aStart.getY(), 1.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("aEnd y-coordinate", 1999.0, aEnd.getY(), 1.0);
}

CPPUNIT_TEST_FIXTURE(CustomshapesTest, testTdf103474_commandG_CaseZeroHeight)
{
    // Some as above, but with shape with command G.
    OUString sURL
        = m_directories.getURLFromSrc(sDataDirectory) + "tdf103474_commandG_CaseZeroHeight.odp";
    mxComponent = loadFromDesktop(sURL, "com.sun.star.comp.presentation.PresentationDocument");
    uno::Reference<drawing::XShape> xShape(getShape(0));
    // The end points of the straight line segment should have the same x-coordinate of left
    // of shape, and different y-coordinates, one top and the other bottom of the shape.
    SdrObjCustomShape& rSdrObjCustomShape(
        static_cast<SdrObjCustomShape&>(*SdrObject::getSdrObjectFromXShape(xShape)));
    EnhancedCustomShape2d aCustomShape2d(rSdrObjCustomShape);
    SdrPathObjUniquePtr pPathObj(
        static_cast<SdrPathObj*>(aCustomShape2d.CreateLineGeometry().release()));
    CPPUNIT_ASSERT_MESSAGE("Could not convert to SdrPathObj", pPathObj);
    const basegfx::B2DPolyPolygon aPolyPolygon(pPathObj->GetPathPoly());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("count polygons", static_cast<sal_uInt32>(1),
                                 aPolyPolygon.count());
    const basegfx::B2DPolygon aPolygon(aPolyPolygon.getB2DPolygon(0));
    // Get the middle points of the polygon. They are the endpoints of the
    // straight line segment regardless of the quarter ellipse parts, because
    // the shape is symmetric.
    const basegfx::B2DPoint aStart(aPolygon.getB2DPoint(aPolygon.count() / 2 - 1));
    const basegfx::B2DPoint aEnd(aPolygon.getB2DPoint(aPolygon.count() / 2));
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("aStart x-coordinate", 1999.0, aStart.getX(), 1.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("aEnd x-coordinate", 1999.0, aEnd.getX(), 1.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("aStart y-coordinate", 9999.0, aStart.getY(), 1.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("aEnd y-coordinate", 1999.0, aEnd.getY(), 1.0);
}

CPPUNIT_TEST_FIXTURE(CustomshapesTest, testTdf122323_largeSwingAngle)
{
    // SwingAngles are clamped to [-360;360] in MS Office. Error was, that LO calculated
    // the end angle and used it modulo 360, no full ellipse was drawn.
    OUString sURL
        = m_directories.getURLFromSrc(sDataDirectory) + "tdf122323_swingAngle_larger360deg.pptx";
    mxComponent = loadFromDesktop(sURL, "com.sun.star.comp.presentation.PresentationDocument");
    uno::Reference<drawing::XShape> xShape(getShape(0));
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    SdrObjCustomShape& rSdrObjCustomShape(
        static_cast<SdrObjCustomShape&>(*SdrObject::getSdrObjectFromXShape(xShape)));
    EnhancedCustomShape2d aCustomShape2d(rSdrObjCustomShape);
    SdrPathObjUniquePtr pPathObj(
        static_cast<SdrPathObj*>(aCustomShape2d.CreateLineGeometry().release()));
    CPPUNIT_ASSERT_MESSAGE("Could not convert to SdrPathObj", pPathObj);
    const basegfx::B2DPolyPolygon aPolyPolygon(pPathObj->GetPathPoly());
    const basegfx::B2DPolygon aPolygon(aPolyPolygon.getB2DPolygon(0));
    const basegfx::B2DPoint aStart(aPolygon.getB2DPoint(0));
    // last point comes from line to center, therefore -2 instead of -1
    const basegfx::B2DPoint aEnd(aPolygon.getB2DPoint(aPolygon.count() - 2));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Start <> End", aStart, aEnd);
}

CPPUNIT_TEST_FIXTURE(CustomshapesTest, testTdf141268)
{
    OUString sURL = m_directories.getURLFromSrc(sDataDirectory) + "tdf141268.odp";
    mxComponent = loadFromDesktop(sURL, "com.sun.star.comp.presentation.PresentationDocument");
    uno::Reference<drawing::XShape> xShape(getShape(0));
    SdrObjCustomShape& rSdrCustomShape(
        static_cast<SdrObjCustomShape&>(*SdrObject::getSdrObjectFromXShape(xShape)));

    // Check left/bottom of bound rect. Without fix it would be left=6722, bottom=9483.
    tools::Rectangle aBoundRect(rSdrCustomShape.GetCurrentBoundRect());
    CPPUNIT_ASSERT_EQUAL(tools::Long(7620), aBoundRect.Left());
    CPPUNIT_ASSERT_EQUAL(tools::Long(8584), aBoundRect.Bottom());
}

CPPUNIT_TEST_FIXTURE(CustomshapesTest, testTdf136176)
{
    // Error was, that fObjectRotation was not correctly updated after shearing.
    // The problem becomes visible after save and reload.
    OUString sURL = m_directories.getURLFromSrc(sDataDirectory) + "tdf136176_rot30_flip.odg";
    mxComponent = loadFromDesktop(sURL, "com.sun.star.comp.drawing.DrawingDocument");

    for (sal_uInt16 i = 0; i < 3; i++)
    {
        // get shape
        uno::Reference<drawing::XShape> xShape(getShape(i));
        SdrObjCustomShape& rSdrObjCustomShape(
            static_cast<SdrObjCustomShape&>(*SdrObject::getSdrObjectFromXShape(xShape)));
        // apply shearing 20deg
        const Point aCenter = rSdrObjCustomShape.GetSnapRect().Center();
        rSdrObjCustomShape.Shear(aCenter, 2000_deg100, tan(basegfx::deg2rad(20.0)), false);
    }

    // Save and reload
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("draw8");
    xStorable->storeToURL(aTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
    mxComponent->dispose();
    mxComponent = loadFromDesktop(aTempFile.GetURL());

    // Expected values of point 4 of the shape polygon
    const OString sTestCase[] = { "FlipH", "FlipV", "FlipHV" };
    const double fX[] = { 14981.0, 3849.0, 15214.0 };
    const double fY[] = { 9366.0, 16464.0, 23463.0 };

    // Verify correct positions
    for (sal_uInt16 i = 0; i < 3; i++)
    {
        // Get shape
        const uno::Reference<drawing::XShape> xShape(getShape(i));
        const SdrObjCustomShape& rSdrObjCustomShape(
            static_cast<SdrObjCustomShape&>(*SdrObject::getSdrObjectFromXShape(xShape)));
        // Create polygon from shape and examine point 4 of the polygon
        const basegfx::B2DPolyPolygon aLineGeometry = rSdrObjCustomShape.GetLineGeometry(false);
        const basegfx::B2DPoint aPoint(aLineGeometry.getB2DPolygon(0).getB2DPoint(4));
        // Allow some tolerance for rounding errors
        if (fabs(aPoint.getX() - fX[i]) > 2.0 || fabs(aPoint.getY() - fY[i]) > 2.0)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sTestCase[i].getStr(), aPoint,
                                         basegfx::B2DPoint(fX[i], fY[i]));
        }
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
