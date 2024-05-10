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

#include <test/unoapi_test.hxx>
#include <rtl/ustring.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <editeng/unoprnms.hxx>
#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>
#include <svl/intitem.hxx>
#include <svx/EnhancedCustomShape2d.hxx>
#include <svx/extrusionbar.hxx>
#include <svx/graphichelper.hxx>
#include <svx/svdoashp.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdview.hxx>
#include <svx/svxids.hrc>
#include <unotools/tempfile.hxx>
#include <vcl/filter/PngImageReader.hxx>
#include <vcl/BitmapReadAccess.hxx>

#include <cppunit/TestAssert.h>

#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegment.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegmentCommand.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>

using namespace ::com::sun::star;

namespace
{
/// Tests for svx/source/customshapes/ code.
class CustomshapesTest : public UnoApiTest
{
public:
    CustomshapesTest()
        : UnoApiTest(u"svx/qa/unit/data/"_ustr)
    {
    }

protected:
    // get shape nShapeIndex from page 0
    uno::Reference<drawing::XShape> getShape(sal_uInt8 nShapeIndex);
    sal_uInt8 countShapes();
    // fX and fY are positions relative to the size of the bitmap of the shape
    // Thus the position is independent from DPI
    Color getColor(uno::Reference<drawing::XShape> xShape, const double& fX, const double& fY);
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

Color CustomshapesTest::getColor(uno::Reference<drawing::XShape> xShape, const double& fX,
                                 const double& fY)
{
    GraphicHelper::SaveShapeAsGraphicToPath(mxComponent, xShape, u"image/png"_ustr,
                                            maTempFile.GetURL());
    SvFileStream aFileStream(maTempFile.GetURL(), StreamMode::READ);
    vcl::PngImageReader aPNGReader(aFileStream);
    Bitmap aBMP = aPNGReader.read().GetBitmap();
    Size aSize = aBMP.GetSizePixel();
    BitmapScopedReadAccess pRead(aBMP);
    return pRead->GetColor(aSize.Height() * fY, aSize.Width() * fX);
}

CPPUNIT_TEST_FIXTURE(CustomshapesTest, testTdf150302)
{
    loadFromFile(u"FontworkSameLetterHeights.fodg");

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong number of shapes", static_cast<sal_uInt8>(2),
                                 countShapes());

    bool bSameHeights = false;
    uno::Reference<drawing::XShape> xShape(getShape(0));
    SdrObjCustomShape* pSdrCustomShape(
        static_cast<SdrObjCustomShape*>(SdrObject::getSdrObjectFromXShape(xShape)));
    const SdrCustomShapeGeometryItem& rGeometryItem(
        pSdrCustomShape->GetMergedItem(SDRATTR_CUSTOMSHAPE_GEOMETRY));
    const css::uno::Any* pAny
        = rGeometryItem.GetPropertyValueByName(u"TextPath"_ustr, u"SameLetterHeights"_ustr);
    if (pAny)
        *pAny >>= bSameHeights;

    CPPUNIT_ASSERT_MESSAGE("Wrong initial value", !bSameHeights);

    // Mark Object
    SfxViewShell* pViewShell = SfxViewShell::Current();
    SdrView* pSdrView = pViewShell->GetDrawView();
    pSdrView->MarkObj(pSdrCustomShape, pSdrView->GetSdrPageView());

    dispatchCommand(mxComponent, u".uno:FontworkSameLetterHeights"_ustr, {});

    const SdrCustomShapeGeometryItem& rGeometryItem1
        = pSdrCustomShape->GetMergedItem(SDRATTR_CUSTOMSHAPE_GEOMETRY);
    pAny = rGeometryItem1.GetPropertyValueByName(u"TextPath"_ustr, u"SameLetterHeights"_ustr);
    if (pAny)
        *pAny >>= bSameHeights;

    CPPUNIT_ASSERT_MESSAGE("Wrong value after toggle", bSameHeights);

    pSdrView->MarkObj(pSdrCustomShape, pSdrView->GetSdrPageView());

    dispatchCommand(mxComponent, u".uno:FontworkSameLetterHeights"_ustr, {});

    const SdrCustomShapeGeometryItem& rGeometryItem2
        = pSdrCustomShape->GetMergedItem(SDRATTR_CUSTOMSHAPE_GEOMETRY);
    pAny = rGeometryItem2.GetPropertyValueByName(u"TextPath"_ustr, u"SameLetterHeights"_ustr);
    if (pAny)
        *pAny >>= bSameHeights;

    CPPUNIT_ASSERT_MESSAGE("Wrong value after toggle 2", !bSameHeights);
}

CPPUNIT_TEST_FIXTURE(CustomshapesTest, testTdf147409_GeomItemHash)
{
    loadFromFile(u"tdf147409_GeomItemHash.odg");
    uno::Reference<drawing::XShape> xShape(getShape(0));
    SdrObjCustomShape* pSdrCustomShape(
        static_cast<SdrObjCustomShape*>(SdrObject::getSdrObjectFromXShape(xShape)));

    // Mark Object
    SfxViewShell* pViewShell = SfxViewShell::Current();
    SdrView* pSdrView = pViewShell->GetDrawView();
    pSdrView->MarkObj(pSdrCustomShape, pSdrView->GetSdrPageView());

    // Apply FontworkSameLetterHeights toggle
    // Without patch a debug build fails assert in SfxItemPool::DirectPutItemInPoolImpl and so crashes.
    dispatchCommand(mxComponent, u".uno:FontworkSameLetterHeights"_ustr, {});
}

CPPUNIT_TEST_FIXTURE(CustomshapesTest, testTdf146866_GeomItemHash)
{
    loadFromFile(u"tdf147409_GeomItemHash.odg");
    uno::Reference<drawing::XShape> xShape(getShape(0));
    SdrObjCustomShape* pSdrCustomShape(
        static_cast<SdrObjCustomShape*>(SdrObject::getSdrObjectFromXShape(xShape)));

    // Mark Object
    SfxViewShell* pViewShell = SfxViewShell::Current();
    SdrView* pSdrView = pViewShell->GetDrawView();
    pSdrView->MarkObj(pSdrCustomShape, pSdrView->GetSdrPageView());

    // Apply extrusion toggle
    // Without patch a debug build fails assert in SfxItemPool::DirectPutItemInPoolImpl and so crashes.
    dispatchCommand(mxComponent, u".uno:ExtrusionToggle"_ustr, {});
}

CPPUNIT_TEST_FIXTURE(CustomshapesTest, testTdf145700_3D_NonUI)
{
    // The document contains first light soft, no ambient color, no second light and shininess 6.
    // Such settings are not available in the UI. It tests the actual color, not the geometry.
    // Load document
    loadFromFile(u"tdf145700_3D_NonUI.doc");

    // Generate bitmap from shape
    uno::Reference<drawing::XShape> xShape = getShape(0);
    GraphicHelper::SaveShapeAsGraphicToPath(mxComponent, xShape, u"image/png"_ustr,
                                            maTempFile.GetURL());

    // Read bitmap and test color
    // The expected values are taken from an image generated by Word
    // Without the changed methods the colors were in range RGB(17,11,17) to RGB(87,55,89).
    SvFileStream aFileStream(maTempFile.GetURL(), StreamMode::READ);
    vcl::PngImageReader aPNGReader(aFileStream);
    BitmapEx aBMPEx = aPNGReader.read();
    Bitmap aBMP = aBMPEx.GetBitmap();
    BitmapScopedReadAccess pRead(aBMP);
    Size aSize = aBMP.GetSizePixel();
    // GetColor(Y,X)
    Color aActualColor = pRead->GetColor(aSize.Height() / 2, aSize.Width() * 0.125);
    Color aExpectedColor(107, 67, 109);
    sal_uInt16 nColorDistance = aExpectedColor.GetColorError(aActualColor);
    CPPUNIT_ASSERT_LESS(sal_uInt16(6), nColorDistance);
    // The current solution for soft light still can be improved. nColorDistance is high.
    aActualColor = pRead->GetColor(aSize.Height() / 2, aSize.Width() * 0.45);
    aExpectedColor = Color(179, 113, 183);
    nColorDistance = aExpectedColor.GetColorError(aActualColor);
    CPPUNIT_ASSERT_LESS(sal_uInt16(54), nColorDistance);
    // This point tests whether shininess is read and used. With default shininess it would be white.
    aActualColor = pRead->GetColor(aSize.Height() / 2, aSize.Width() * 0.72);
    aExpectedColor = Color(255, 231, 255);
    nColorDistance = aExpectedColor.GetColorError(aActualColor);
    CPPUNIT_ASSERT_LESS(sal_uInt16(14), nColorDistance);
}

CPPUNIT_TEST_FIXTURE(CustomshapesTest, testTdf145700_3D_FrontLightDim)
{
    // This tests the actual color, not the geometry.
    // Load document
    loadFromFile(u"tdf145700_3D_FrontLightDim.doc");

    // Generate bitmap from shape
    uno::Reference<drawing::XShape> xShape = getShape(0);
    GraphicHelper::SaveShapeAsGraphicToPath(mxComponent, xShape, u"image/png"_ustr,
                                            maTempFile.GetURL());

    // Read bitmap and test color
    // The expected values are taken from an image generated by Word
    // Without the changed methods the nColorDistance was 476 and 173 respectively.
    SvFileStream aFileStream(maTempFile.GetURL(), StreamMode::READ);
    vcl::PngImageReader aPNGReader(aFileStream);
    BitmapEx aBMPEx = aPNGReader.read();
    Bitmap aBMP = aBMPEx.GetBitmap();
    BitmapScopedReadAccess pRead(aBMP);
    Size aSize = aBMP.GetSizePixel();
    // GetColor(Y,X)
    Color aActualColor = pRead->GetColor(aSize.Height() / 2, aSize.Width() * 0.4);
    Color aExpectedColor(240, 224, 229);
    sal_uInt16 nColorDistance = aExpectedColor.GetColorError(aActualColor);
    CPPUNIT_ASSERT_LESS(sal_uInt16(9), nColorDistance);
    aActualColor = pRead->GetColor(aSize.Height() / 2, aSize.Width() * 0.9);
    aExpectedColor = Color(96, 90, 92);
    nColorDistance = aExpectedColor.GetColorError(aActualColor);
    CPPUNIT_ASSERT_LESS(sal_uInt16(6), nColorDistance);
}

CPPUNIT_TEST_FIXTURE(CustomshapesTest, testTdf145700_3D_FirstLightHarsh)
{
    // Load document
    loadFromFile(u"tdf145700_3D_FirstLightHarsh.doc");

    // Generate bitmap from shape
    uno::Reference<drawing::XShape> xShape = getShape(0);
    GraphicHelper::SaveShapeAsGraphicToPath(mxComponent, xShape, u"image/png"_ustr,
                                            maTempFile.GetURL());

    // Read bitmap and test color in center
    SvFileStream aFileStream(maTempFile.GetURL(), StreamMode::READ);
    vcl::PngImageReader aPNGReader(aFileStream);
    BitmapEx aBMPEx = aPNGReader.read();
    Bitmap aBMP = aBMPEx.GetBitmap();
    BitmapScopedReadAccess pRead(aBMP);
    Size aSize = aBMP.GetSizePixel();
    // GetColor(Y,X)
    const Color aActualColor = pRead->GetColor(aSize.Height() / 2, aSize.Width() / 2);
    const Color aExpectedColor(211, 247, 255); // from image generated by Word
    sal_uInt16 nColorDistance = aExpectedColor.GetColorError(aActualColor);
    CPPUNIT_ASSERT_LESS(sal_uInt16(3), nColorDistance);
}

CPPUNIT_TEST_FIXTURE(CustomshapesTest, testTdf145956_Origin_Relative_BoundRect)
{
    // The ViewPoint is relative to point Origin. The coordinates of point Origin are fractions of
    // the actual (2D) bounding rectangle of the shape, including rotation around z-axis and flip.
    // Error (among others) was, that the unrotated snap rectangle was used.

    // Load document
    loadFromFile(u"tdf145956_Origin.odp");

    // The shape is extruded with 10cm. viewpoint="(0cm 0cm 25cm)", origin="0 0".
    uno::Reference<drawing::XShape> xShape(getShape(0));
    uno::Reference<beans::XPropertySet> xPropSet(xShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("Could not get the properties", xPropSet.is());
    awt::Rectangle aBoundRect;
    xPropSet->getPropertyValue(UNO_NAME_MISC_OBJ_BOUNDRECT) >>= aBoundRect;
    sal_Int32 nActualTop = aBoundRect.Y;

    // Without the fix it would have failed with top = 9462.
    // The tolerance 10 is estimated and can be adjusted if required for HiDPI.
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("top", 10448, nActualTop, 10);
}

CPPUNIT_TEST_FIXTURE(CustomshapesTest, testTdf145904_Extrusion_CenterZ_odt)
{
    // The Z-component of the extrusion rotation center specifies the position in Hmm.
    // Error (among others) was, that the value was interpreted as Twips.

    // Load document
    loadFromFile(u"tdf145904_center_Zminus2000.odt");

    // The shape is extruded and tilt left 60deg. The rotation center is at -2000Hmm on the z-axis.
    // That is a position behind the back face of the extruded shape.
    uno::Reference<drawing::XShape> xShape(getShape(0));
    uno::Reference<beans::XPropertySet> xPropSet(xShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("Could not get the properties", xPropSet.is());
    awt::Rectangle aBoundRect;
    xPropSet->getPropertyValue(UNO_NAME_MISC_OBJ_BOUNDRECT) >>= aBoundRect;
    awt::Point aAnchorPosition;
    xPropSet->getPropertyValue(u"AnchorPosition"_ustr) >>= aAnchorPosition;
    sal_Int32 nActualLeft = aBoundRect.X - aAnchorPosition.X;

    // Without the fix it would have failed with left = 7731.
    // The tolerance 10 is estimated and can be adjusted if required for HiDPI.
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("left", 3501, nActualLeft, 10);
}

CPPUNIT_TEST_FIXTURE(CustomshapesTest, testTdf145904_Extrusion_CenterY_odt)
{
    // The X- and Y-component of the extrusion rotation center specify the position as fraction of
    // shape size. Error was, that the relative fraction was handled as absolute value in Hmm.

    // Load document
    loadFromFile(u"tdf145904_center_Y0dot25.odt");

    // The shape is extruded and tilt down 90deg. The rotation center is in the middle between shape
    // center and bottom shape edge. The bottom edge of the projected solid has roughly the
    // y-coordinate of the rotation center.
    uno::Reference<drawing::XShape> xShape(getShape(0));
    uno::Reference<beans::XPropertySet> xPropSet(xShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("Could not get the properties", xPropSet.is());
    awt::Rectangle aBoundRect;
    xPropSet->getPropertyValue(UNO_NAME_MISC_OBJ_BOUNDRECT) >>= aBoundRect;
    awt::Point aAnchorPosition;
    xPropSet->getPropertyValue(u"AnchorPosition"_ustr) >>= aAnchorPosition;
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
    loadFromFile(u"tdf145904_center_Y0dot25.doc");

    // The shape is extruded and tilt down 90deg. The rotation center is in the middle between shape
    // center and bottom shape edge. The bottom edge of the projected solid has roughly the
    // y-coordinate of the center.
    uno::Reference<drawing::XShape> xShape(getShape(0));
    uno::Reference<beans::XPropertySet> xPropSet(xShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("Could not get the properties", xPropSet.is());
    awt::Rectangle aBoundRect;
    xPropSet->getPropertyValue(UNO_NAME_MISC_OBJ_BOUNDRECT) >>= aBoundRect;
    awt::Point aAnchorPosition;
    xPropSet->getPropertyValue(u"AnchorPosition"_ustr) >>= aAnchorPosition;
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
    loadFromFile(u"tdf145245_ExtrusionPosition.odp");

    // The tolerance 40 is estimated and can be adjusted if required for HiDPI.
    {
        // First shape has extrusion behind the shape.
        uno::Reference<drawing::XShape> xShape0(getShape(0));
        SdrObjCustomShape& rSdrCustomShape(
            static_cast<SdrObjCustomShape&>(*SdrObject::getSdrObjectFromXShape(xShape0)));
        tools::Rectangle aBoundRect(rSdrCustomShape.GetCurrentBoundRect());
        tools::Rectangle aExpected(Point(1000, 1000), Size(6002, 5001));
        CPPUNIT_ASSERT_RECTANGLE_EQUAL_WITH_TOLERANCE(aExpected, aBoundRect, 40);
    }
    {
        // Second shape has half of extrusion behind the shape.
        uno::Reference<drawing::XShape> xShape(getShape(1));
        SdrObjCustomShape& rSdrCustomShape(
            static_cast<SdrObjCustomShape&>(*SdrObject::getSdrObjectFromXShape(xShape)));
        // Without the fix the height was 1 instead of 5001.
        tools::Rectangle aBoundRect(rSdrCustomShape.GetCurrentBoundRect());
        tools::Rectangle aExpected(Point(9000, 3500), Size(6002, 5001));
        CPPUNIT_ASSERT_RECTANGLE_EQUAL_WITH_TOLERANCE(aExpected, aBoundRect, 40);
    }
    {
        // Third shape has extrusion before the shape.
        uno::Reference<drawing::XShape> xShape(getShape(2));
        SdrObjCustomShape& rSdrCustomShape(
            static_cast<SdrObjCustomShape&>(*SdrObject::getSdrObjectFromXShape(xShape)));
        // Without the fix the y-coordinate was 1000 instead of 6000.
        tools::Rectangle aBoundRect(rSdrCustomShape.GetCurrentBoundRect());
        tools::Rectangle aExpected(Point(18000, 6000), Size(6002, 5001));
        CPPUNIT_ASSERT_RECTANGLE_EQUAL_WITH_TOLERANCE(aExpected, aBoundRect, 40);
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

    loadFromFile(u"tdf144988_Fontwork_FontSize.odp");
    uno::Reference<drawing::XShape> xShape(getShape(0));
    SdrObjCustomShape& rSdrCustomShape(
        static_cast<SdrObjCustomShape&>(*SdrObject::getSdrObjectFromXShape(xShape)));

    // Without the fix in place left|top, width x height was 1279|1279, 2815 x 2448.
    // The expected values 1501|1777, 3947 x 1446 are only valid for 96dpi.
    tools::Rectangle aBoundRect(rSdrCustomShape.GetCurrentBoundRect());
    tools::Rectangle aExpected(Point(1501, 1777), Size(3947, 1446));
    CPPUNIT_ASSERT_RECTANGLE_EQUAL_WITH_TOLERANCE(aExpected, aBoundRect, 5);
}

CPPUNIT_TEST_FIXTURE(CustomshapesTest, testTdf145111_anchor_in_Fontwork)
{
    // tdf#145004 In case ScaleX is true in property TextPath, SDRTEXTVERTADJUST is
    // evaluated and should shift the Fontwork text. That did not work for
    // 'Top-Left' and 'Bottom-Left'.

    // Load document
    loadFromFile(u"tdf145111_TL_BL_Fontwork.odp");

    {
        // First shape has anchor set to Top-Left, which shifts Fontwork text down.
        uno::Reference<drawing::XShape> xShape(getShape(0));
        SdrObjCustomShape& rSdrCustomShape(
            static_cast<SdrObjCustomShape&>(*SdrObject::getSdrObjectFromXShape(xShape)));

        // Without the fix in place top was 2295, but should be 2900.
        tools::Rectangle aBoundRect(rSdrCustomShape.GetCurrentBoundRect());
        CPPUNIT_ASSERT_DOUBLES_EQUAL(tools::Long(2900), aBoundRect.Top(), 5);
    }
    {
        // Second shape has anchor set to Bottom-Left, which shifts Fontwork text up.
        uno::Reference<drawing::XShape> xShape(getShape(1));
        SdrObjCustomShape& rSdrCustomShape(
            static_cast<SdrObjCustomShape&>(*SdrObject::getSdrObjectFromXShape(xShape)));

        // Without the fix in place top was 10294, but should be 9500.
        tools::Rectangle aBoundRect(rSdrCustomShape.GetCurrentBoundRect());
        CPPUNIT_ASSERT_DOUBLES_EQUAL(tools::Long(9500), aBoundRect.Top(), 5);
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
    loadFromFile(u"tdf145004_gap_by_ScaleX.pptx");
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
    loadFromFile(u"tdf141021_ExtrusionNorth.odp");
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
    loadFromFile(u"tdf138945_resizeRotatedShape.odg");
    uno::Reference<drawing::XShape> xShape(getShape(0));

    // Change height and mirror vertical
    {
        SdrObjCustomShape& rSdrShape(
            static_cast<SdrObjCustomShape&>(*SdrObject::getSdrObjectFromXShape(xShape)));
        rSdrShape.NbcResize(rSdrShape.GetRelativePos(), Fraction(1.0), Fraction(-0.5));
        tools::Rectangle aSnapRect(rSdrShape.GetSnapRect());
        tools::Rectangle aBoundRect(rSdrShape.GetCurrentBoundRect());
        CPPUNIT_ASSERT_RECTANGLE_EQUAL_WITH_TOLERANCE(aSnapRect, aBoundRect, 3);
    }

    // Change height
    {
        SdrObjCustomShape& rSdrShape(
            static_cast<SdrObjCustomShape&>(*SdrObject::getSdrObjectFromXShape(xShape)));
        rSdrShape.NbcResize(rSdrShape.GetRelativePos(), Fraction(1.0), Fraction(2.0));
        tools::Rectangle aSnapRect(rSdrShape.GetSnapRect());
        tools::Rectangle aBoundRect(rSdrShape.GetCurrentBoundRect());
        CPPUNIT_ASSERT_RECTANGLE_EQUAL_WITH_TOLERANCE(aSnapRect, aBoundRect, 3);
    }

    // Change width
    {
        SdrObjCustomShape& rSdrShape(
            static_cast<SdrObjCustomShape&>(*SdrObject::getSdrObjectFromXShape(xShape)));
        rSdrShape.NbcResize(rSdrShape.GetRelativePos(), Fraction(2.0), Fraction(1.0));
        tools::Rectangle aSnapRect(rSdrShape.GetSnapRect());
        tools::Rectangle aBoundRect(rSdrShape.GetCurrentBoundRect());
        CPPUNIT_ASSERT_RECTANGLE_EQUAL_WITH_TOLERANCE(aSnapRect, aBoundRect, 3);
    }

    // Change width and mirror horizontal
    {
        SdrObjCustomShape& rSdrShape(
            static_cast<SdrObjCustomShape&>(*SdrObject::getSdrObjectFromXShape(xShape)));
        rSdrShape.NbcResize(rSdrShape.GetRelativePos(), Fraction(-0.5), Fraction(1.0));
        tools::Rectangle aSnapRect(rSdrShape.GetSnapRect());
        tools::Rectangle aBoundRect(rSdrShape.GetCurrentBoundRect());
        CPPUNIT_ASSERT_RECTANGLE_EQUAL_WITH_TOLERANCE(aSnapRect, aBoundRect, 3);
    }
}

CPPUNIT_TEST_FIXTURE(CustomshapesTest, testViewBoxLeftTop)
{
    // tdf#121890 formula values "left" and "top" are wrongly calculated
    // Load a document with two custom shapes of type "non-primitive"
    loadFromFile(u"viewBox_positive_twolines_strict.odp");
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
    loadFromFile(u"tdf121761_Accuracy_command_X.odp");
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
    loadFromFile(u"tdf121952_Toggle_direction_command_X.odp");
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
    loadFromFile(u"tdf122964_MultipleMoveTo.odg");
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
    loadFromFile(u"tdf121845_WidthOrientation_command_U.odg");
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
    loadFromFile(u"tdf121845_HalfEllipseVML.doc");
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
    loadFromFile(u"tdf121845_start40_swing480.doc");
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
    loadFromFile(u"tdf121845_Two_commands_U.odg");
    uno::Reference<drawing::XShape> xShape(getShape(0));
    // In case no line is drawn, two polygons are generated; with line only one polygon
    SdrObjCustomShape& rSdrObjCustomShape(
        static_cast<SdrObjCustomShape&>(*SdrObject::getSdrObjectFromXShape(xShape)));
    EnhancedCustomShape2d aCustomShape2d(rSdrObjCustomShape);
    rtl::Reference<SdrPathObj> pPathObj(
        static_cast<SdrPathObj*>(aCustomShape2d.CreateLineGeometry().get()));
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
    loadFromFile(u"tdf124212_handle_position.odg");
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
    loadFromFile(u"tdf124029_Arc_position.doc");
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
    loadFromFile(u"tdf124740_HandleInOOXMLUserShape.pptx");
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
    loadFromFile(u"tdf115813_HandleMovementOOXMLPresetShapes.pptx");

    // values in vector InteractionsHandles are in 1/100 mm and refer to page
    for (sal_uInt8 i = 0; i < countShapes(); i++)
    {
        uno::Reference<drawing::XShape> xShape(getShape(i));
        SdrObjCustomShape& rSdrObjCustomShape(
            static_cast<SdrObjCustomShape&>(*SdrObject::getSdrObjectFromXShape(xShape)));
        OUString sShapeType(u"non-primitive"_ustr); // default for ODF
        const SdrCustomShapeGeometryItem& rGeometryItem(
            rSdrObjCustomShape.GetMergedItem(SDRATTR_CUSTOMSHAPE_GEOMETRY));
        const uno::Any* pAny = rGeometryItem.GetPropertyValueByName(u"Type"_ustr);
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
                OUString sError
                    = OUString::number(i) + " " + sShapeType + ": " + OUString::number(j) + " X "
                      + OUString::number(nDesiredX) + "|" + OUString::number(nObservedX) + " Y "
                      + OUString::number(nDesiredY) + "|" + OUString::number(nObservedY);
                CPPUNIT_FAIL(sError.toUtf8().getStr());
            }
        }
    }
}

CPPUNIT_TEST_FIXTURE(CustomshapesTest, testQuadraticCurveTo)
{
    // tdf125782 command Q (quadraticcurveto) uses wrong 'current point'.
    // When converting to cubic Bezier curve, this had resulted in a wrong first control point.
    // The quadraticcurveto segment starts in shape center in the test file. The first control
    // point should produce a horizontal tangent in the start point.
    loadFromFile(u"tdf125782_QuadraticCurveTo.odg");
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
    loadFromFile(u"tdf126512_OOXMLHandleMovementInODF.odp");

    for (sal_uInt8 i = 0; i < countShapes(); i++)
    {
        uno::Reference<drawing::XShape> xShape(getShape(i));
        SdrObjCustomShape& rSdrObjCustomShape(
            static_cast<SdrObjCustomShape&>(*SdrObject::getSdrObjectFromXShape(xShape)));
        OUString sShapeType(u"non-primitive"_ustr); // only to initialize, value not used here
        const SdrCustomShapeGeometryItem& rGeometryItem(
            rSdrObjCustomShape.GetMergedItem(SDRATTR_CUSTOMSHAPE_GEOMETRY));
        const uno::Any* pAny = rGeometryItem.GetPropertyValueByName(u"Type"_ustr);
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
                OUString sError
                    = OUString::number(i) + " " + sShapeType + "  " + OUString::number(j);
                CPPUNIT_FAIL(sError.toUtf8().getStr());
            }
        }
    }
}

CPPUNIT_TEST_FIXTURE(CustomshapesTest, testTdf127785_Mirror)
{
    // The document contains two shapes, one with horizontal flip, the other with vertical
    // flip. They are diamonds, so their text frame is symmetric to the center of the shape.
    // The shapes have not stroke and no fill, so that the bounding box surrounds the text
    // and therefore equals approximately the text frame.
    // Error was, that because of wrong calculation, the flipped shapes do not use the
    // text frame but the frame rectangle for their text.
    loadFromFile(u"tdf127785_Mirror.odp");

    uno::Reference<drawing::XShape> xShapeV(getShape(0));
    uno::Reference<beans::XPropertySet> xShapeVProps(xShapeV, uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("Could not get the shape properties", xShapeVProps.is());
    awt::Rectangle aBoundRectV;
    xShapeVProps->getPropertyValue(UNO_NAME_MISC_OBJ_BOUNDRECT) >>= aBoundRectV;
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Flip vertical wrong size.", 8000.0, aBoundRectV.Height,
                                         10.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Flip vertical wrong size.", 8000.0, aBoundRectV.Width,
                                         10.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Flip vertical wrong position.", 1000.0, aBoundRectV.X,
                                         10.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Flip vertical wrong position.", 2000.0, aBoundRectV.Y,
                                         10.0);

    uno::Reference<drawing::XShape> xShapeH(getShape(1));
    uno::Reference<beans::XPropertySet> xShapeHProps(xShapeH, uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("Could not get the shape properties", xShapeHProps.is());
    awt::Rectangle aBoundRectH;
    xShapeHProps->getPropertyValue(UNO_NAME_MISC_OBJ_BOUNDRECT) >>= aBoundRectH;
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Flip horizontal wrong size.", 8000.0, aBoundRectH.Height,
                                         10.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Flip horizontal wrong size.", 8000.0, aBoundRectH.Width,
                                         10.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Flip horizontal wrong position.", 13000.0, aBoundRectH.X,
                                         10.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Flip horizontal wrong position.", 2000.0, aBoundRectH.Y,
                                         10.0);
}

CPPUNIT_TEST_FIXTURE(CustomshapesTest, testTdf126060_3D_Z_Rotation)
{
    // The document contains one textbox with inside overflowed text
    // and the text has 3D z rotation. When we open the document we
    // should see the text vertically and rotated from text bound center not text box.

    loadFromFile(u"tdf126060_3D_Z_Rotation.pptx");

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
    loadFromFile(u"tdf127785_asymmetricTextBoxFlipV.odg");

    uno::Reference<drawing::XShape> xShape(getShape(0));
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("Could not get the shape properties", xShapeProps.is());
    awt::Rectangle aBoundRect;
    xShapeProps->getPropertyValue(UNO_NAME_MISC_OBJ_BOUNDRECT) >>= aBoundRect;

    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("wrong left", 9000.0, aBoundRect.X, 10.0);
    const double nRight = aBoundRect.X + aBoundRect.Width - 1;
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("wrong right", 19000.0, nRight, 10.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("wrong top", 3000.0, aBoundRect.Y, 10.0);
    const double nBottom = aBoundRect.Y + aBoundRect.Height - 1;
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("wrong bottom", 18000.0, nBottom, 10.0);
}

CPPUNIT_TEST_FIXTURE(CustomshapesTest, testTdf127785_TextRotateAngle)
{
    // The document contains a shapes with vertical flip and a text frame with own
    // rotate angle. The shape has not stroke and no fill, so that the bounding box
    // surrounds the text and therefore equals approximately the text frame.
    // Error was, that the compensation for the 180° rotation added for vertical
    // flip were not made to the text box position but to the text matrix.
    loadFromFile(u"tdf127785_TextRotateAngle.odp");

    uno::Reference<drawing::XShape> xShape(getShape(0));
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("Could not get the shape properties", xShapeProps.is());
    awt::Rectangle aBoundRect;
    xShapeProps->getPropertyValue(UNO_NAME_MISC_OBJ_BOUNDRECT) >>= aBoundRect;

    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("wrong left", 2000.0, aBoundRect.X, 10.0);
    const double nRight = aBoundRect.X + aBoundRect.Width - 1;
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("wrong right", 14000.0, nRight, 10.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("wrong top", 3000.0, aBoundRect.Y, 10.0);
    const double nBottom = aBoundRect.Y + aBoundRect.Height - 1;
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("wrong bottom", 9000.0, nBottom, 10.0);
}

CPPUNIT_TEST_FIXTURE(CustomshapesTest, testTdf128413_tbrlOnOff)
{
    // The document contains a rotated shape with text. The error was, that switching
    // tb-rl writing-mode on, changed the shape size and position.

    loadFromFile(u"tdf128413_tbrl_OnOff.odp");
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
    loadFromFile(u"tdf129532_MatrixFlipV.odg");

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
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Incorrect width", aBoundRect0.Width, aBoundRect1.Width);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Incorrect height", aBoundRect0.Height, aBoundRect1.Height);
}

CPPUNIT_TEST_FIXTURE(CustomshapesTest, testTdf103474_commandT_CaseZeroHeight)
{
    // tdf103474 custom shape with command T to create quarter ellipses in a bracket,
    // corner case where the ellipse has zero height.
    // Error was, that the calculation of the circle angle from the ellipse
    // angle results in a wrong angle for the case 180° and height zero.
    loadFromFile(u"tdf103474_commandT_CaseZeroHeight.odp");
    uno::Reference<drawing::XShape> xShape(getShape(0));
    // The end points of the straight line segment should have the same x-coordinate of left
    // of shape, and different y-coordinates, one top and the other bottom of the shape.
    SdrObjCustomShape& rSdrObjCustomShape(
        static_cast<SdrObjCustomShape&>(*SdrObject::getSdrObjectFromXShape(xShape)));
    EnhancedCustomShape2d aCustomShape2d(rSdrObjCustomShape);
    rtl::Reference<SdrPathObj> pPathObj(
        static_cast<SdrPathObj*>(aCustomShape2d.CreateLineGeometry().get()));
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
    loadFromFile(u"tdf103474_commandG_CaseZeroHeight.odp");
    uno::Reference<drawing::XShape> xShape(getShape(0));
    // The end points of the straight line segment should have the same x-coordinate of left
    // of shape, and different y-coordinates, one top and the other bottom of the shape.
    SdrObjCustomShape& rSdrObjCustomShape(
        static_cast<SdrObjCustomShape&>(*SdrObject::getSdrObjectFromXShape(xShape)));
    EnhancedCustomShape2d aCustomShape2d(rSdrObjCustomShape);
    rtl::Reference<SdrPathObj> pPathObj(
        static_cast<SdrPathObj*>(aCustomShape2d.CreateLineGeometry().get()));
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
    loadFromFile(u"tdf122323_swingAngle_larger360deg.pptx");
    uno::Reference<drawing::XShape> xShape(getShape(0));
    SdrObjCustomShape& rSdrObjCustomShape(
        static_cast<SdrObjCustomShape&>(*SdrObject::getSdrObjectFromXShape(xShape)));
    EnhancedCustomShape2d aCustomShape2d(rSdrObjCustomShape);
    rtl::Reference<SdrPathObj> pPathObj(
        static_cast<SdrPathObj*>(aCustomShape2d.CreateLineGeometry().get()));
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
    loadFromFile(u"tdf141268.odp");
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
    loadFromFile(u"tdf136176_rot30_flip.odg");

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
    saveAndReload(u"draw8"_ustr);

    // Expected values of point 4 of the shape polygon
    const OString sTestCase[] = { "FlipH"_ostr, "FlipV"_ostr, "FlipHV"_ostr };
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

CPPUNIT_TEST_FIXTURE(CustomshapesTest, testTdf148501_OctagonBevel)
{
    // The document contains a shape "Octagon Bevel". It should use shadings 40%, 20%, -20%, -40%
    // from left-top to bottom-right. The test examines actual color, not the geometry.
    // Load document
    loadFromFile(u"tdf148501_OctagonBevel.odp");

    // Generate bitmap from shape
    uno::Reference<drawing::XShape> xShape = getShape(0);
    GraphicHelper::SaveShapeAsGraphicToPath(mxComponent, xShape, u"image/png"_ustr,
                                            maTempFile.GetURL());

    // Read bitmap and test color
    // expected in order top-left, top, top-right, right, bottom-right:
    // RGB(165|195|266), RGB(139|176|217), RGB(91|127|166), RGB(68|95|124), RGB(68|95|124)
    // Without applied patch the colors were:
    // RGB(193|214,236), RGB(193|214,236), RGB(80|111|145), RGB(23|32|41), RGB(193|214|236)
    // So we test segments top, right and bottom-right.
    SvFileStream aFileStream(maTempFile.GetURL(), StreamMode::READ);
    vcl::PngImageReader aPNGReader(aFileStream);
    BitmapEx aBMPEx = aPNGReader.read();
    Bitmap aBMP = aBMPEx.GetBitmap();
    BitmapScopedReadAccess pRead(aBMP);
    Size aSize = aBMP.GetSizePixel();

    // GetColor(Y,X). The chosen threshold for the ColorDistance can be adapted if necessary.
    Color aActualColor = pRead->GetColor(aSize.Height() * 0.17, aSize.Width() * 0.5); // top
    Color aExpectedColor(139, 176, 217);
    sal_uInt16 nColorDistance = aExpectedColor.GetColorError(aActualColor);
    CPPUNIT_ASSERT_LESS(sal_uInt16(6), nColorDistance);
    aActualColor = pRead->GetColor(aSize.Height() * 0.5, aSize.Width() * 0.83); // right
    aExpectedColor = Color(68, 95, 124); // same for right and bottom-right
    nColorDistance = aExpectedColor.GetColorError(aActualColor);
    CPPUNIT_ASSERT_LESS(sal_uInt16(6), nColorDistance);
    aActualColor = pRead->GetColor(aSize.Height() * 0.75, aSize.Width() * 0.75); // bottom-right
    nColorDistance = aExpectedColor.GetColorError(aActualColor);
    CPPUNIT_ASSERT_LESS(sal_uInt16(6), nColorDistance);
}

bool lcl_getShapeSegments(uno::Sequence<drawing::EnhancedCustomShapeSegment>& rSegments,
                          const uno::Reference<drawing::XShape>& xShape)
{
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY_THROW);
    uno::Any anotherAny = xShapeProps->getPropertyValue(u"CustomShapeGeometry"_ustr);
    uno::Sequence<beans::PropertyValue> aCustomShapeGeometry;
    if (!(anotherAny >>= aCustomShapeGeometry))
        return false;
    uno::Sequence<beans::PropertyValue> aPathProps;
    for (beans::PropertyValue const& rProp : aCustomShapeGeometry)
    {
        if (rProp.Name == "Path")
        {
            rProp.Value >>= aPathProps;
            break;
        }
    }

    for (beans::PropertyValue const& rProp : aPathProps)
    {
        if (rProp.Name == "Segments")
        {
            rProp.Value >>= rSegments;
            break;
        }
    }
    if (rSegments.getLength() > 1)
        return true;
    else
        return false;
}

CPPUNIT_TEST_FIXTURE(CustomshapesTest, testTdf148714_CurvedArrows)
{
    // Error was, that the line between 1. and 2. arc was missing.
    loadFromFile(u"tdf148714_CurvedArrows.ppt");

    for (sal_Int32 nShapeIndex = 0; nShapeIndex < 4; nShapeIndex++)
    {
        uno::Reference<drawing::XShape> xShape(getShape(nShapeIndex));
        uno::Sequence<drawing::EnhancedCustomShapeSegment> aSegments;
        CPPUNIT_ASSERT(lcl_getShapeSegments(aSegments, xShape));

        if (nShapeIndex == 0 || nShapeIndex == 3)
        {
            // curvedDownArrow or curvedLeftArrow. Segments should start with VW. Without fix it was
            // V with count 2, which means VV.
            CPPUNIT_ASSERT_EQUAL(
                sal_Int16(drawing::EnhancedCustomShapeSegmentCommand::CLOCKWISEARC),
                aSegments[0].Command);
            CPPUNIT_ASSERT_EQUAL(sal_Int16(1), aSegments[0].Count);
            CPPUNIT_ASSERT_EQUAL(
                sal_Int16(drawing::EnhancedCustomShapeSegmentCommand::CLOCKWISEARCTO),
                aSegments[1].Command);
            CPPUNIT_ASSERT_EQUAL(sal_Int16(1), aSegments[1].Count);
        }
        else
        {
            // curvedUpArrow or curvedRightArrow. Segments should start with BA. Without fix is was
            // B with count 2, which means BB.
            CPPUNIT_ASSERT_EQUAL(sal_Int16(drawing::EnhancedCustomShapeSegmentCommand::ARC),
                                 aSegments[0].Command);
            CPPUNIT_ASSERT_EQUAL(sal_Int16(1), aSegments[0].Count);
            CPPUNIT_ASSERT_EQUAL(sal_Int16(drawing::EnhancedCustomShapeSegmentCommand::ARCTO),
                                 aSegments[1].Command);
            CPPUNIT_ASSERT_EQUAL(sal_Int16(1), aSegments[1].Count);
        }
    }
}

CPPUNIT_TEST_FIXTURE(CustomshapesTest, testTdf148707_two_commands_B_V)
{
    // tdf148707 custom shape with multiple command B or multiple command V were drawn with a line
    // between the arcs as if the second command was a A or W respectively.
    // The test document has a shape with path "V 0 0 50 100 0 50 25 0 50 0 100 100 75 0 100 50 N"
    // and a shape with path "B 0 0 50 100 0 50 25 100 50 0 100 100 75 100 100 50 N".
    loadFromFile(u"tdf148707_two_commands_B_V.odp");
    for (sal_uInt8 i = 0; i < 2; i++)
    {
        uno::Reference<drawing::XShape> xShape(getShape(i));
        // In case no line is drawn, two polygons are generated; with line only one polygon
        SdrObjCustomShape& rSdrObjCustomShape(
            static_cast<SdrObjCustomShape&>(*SdrObject::getSdrObjectFromXShape(xShape)));
        EnhancedCustomShape2d aCustomShape2d(rSdrObjCustomShape);
        rtl::Reference<SdrPathObj> pPathObj(
            static_cast<SdrPathObj*>(aCustomShape2d.CreateLineGeometry().get()));
        CPPUNIT_ASSERT_MESSAGE("Could not convert to SdrPathObj", pPathObj);
        const basegfx::B2DPolyPolygon aPolyPolygon(pPathObj->GetPathPoly());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("count polygons", sal_uInt32(2), aPolyPolygon.count());
    }
}

bool lcl_getShapeCoordinates(uno::Sequence<drawing::EnhancedCustomShapeParameterPair>& rCoordinates,
                             const uno::Reference<drawing::XShape>& xShape)
{
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY_THROW);
    uno::Any anotherAny = xShapeProps->getPropertyValue(u"CustomShapeGeometry"_ustr);
    uno::Sequence<beans::PropertyValue> aCustomShapeGeometry;
    if (!(anotherAny >>= aCustomShapeGeometry))
        return false;
    uno::Sequence<beans::PropertyValue> aPathProps;
    for (beans::PropertyValue const& rProp : aCustomShapeGeometry)
    {
        if (rProp.Name == "Path")
        {
            rProp.Value >>= aPathProps;
            break;
        }
    }

    for (beans::PropertyValue const& rProp : aPathProps)
    {
        if (rProp.Name == "Coordinates")
        {
            rProp.Value >>= rCoordinates;
            break;
        }
    }
    if (rCoordinates.getLength() > 0)
        return true;
    else
        return false;
}

CPPUNIT_TEST_FIXTURE(CustomshapesTest, testTdf153000_MS0_SPT_25_31)
{
    // The shapes MSO_SPT=25 to MSO_SPT=31 are currently rendered as rectangle. They should be
    // rendered same way as in Word. More info in bug 153000.
    loadFromFile(u"tdf153000_WordArt_type_25_to_31.docx");
    // The wrong rendering becomes visible in properties "Coordinates" and "Segments". To simplify
    // the test we do not compare the values themselves but only the amount of values.
    // Without fix there were always 5 pairs in "Coordinates" and "Segments" did not exist.
    sal_Int32 aExpected[] = { 8, 5, 14, 8, 8, 14, 4 };
    for (sal_uInt8 i = 0; i < 7; i++)
    {
        uno::Reference<drawing::XShape> xShape(getShape(i));
        uno::Sequence<drawing::EnhancedCustomShapeSegment> aSegments;
        CPPUNIT_ASSERT(lcl_getShapeSegments(aSegments, xShape));
        uno::Sequence<drawing::EnhancedCustomShapeParameterPair> aCoordinates;
        CPPUNIT_ASSERT(lcl_getShapeCoordinates(aCoordinates, xShape));
        CPPUNIT_ASSERT_EQUAL(aExpected[i], aCoordinates.getLength());
    }
}

CPPUNIT_TEST_FIXTURE(CustomshapesTest, testTdf160421_3D_FlipLight)
{
    // The document contains (0)an extruded 'rectangle' custom shape which is illuminated with front
    // light, (1) this shape vertically flipped and (2) this shape horizontally flipped.
    // When the shape is flipped vertically or horizontally, the light direction should not
    // change. MS Office behaves in this way for ppt and pptx and it is meaningful as flipping is
    // applied to the shape, not to the scene.

    // Load document.
    loadFromFile(u"tdf160421_3D_FlipLight.odp");

    // Get color from untransformed shape (0).
    uno::Reference<drawing::XShape> xShape = getShape(0);
    Color aNormalColor = getColor(xShape, 0.6, 0.6);

    // Test that color from vertically flipped shape (1) is same as normal color. Without the fix
    // it was only build from ambient light and thus much darker.
    xShape = getShape(1);
    sal_uInt16 nColorDistance = aNormalColor.GetColorError(getColor(xShape, 0.6, 0.6));
    CPPUNIT_ASSERT_LESS(sal_uInt16(6), nColorDistance);

    // Same for horizontally flipped shape (2)
    xShape = getShape(2);
    nColorDistance = aNormalColor.GetColorError(getColor(xShape, 0.6, 0.6));
    CPPUNIT_ASSERT_LESS(sal_uInt16(6), nColorDistance);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
