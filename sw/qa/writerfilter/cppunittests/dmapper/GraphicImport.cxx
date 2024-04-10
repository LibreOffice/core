/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>

#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/view/XViewCursor.hpp>
#include <com/sun/star/drawing/PointSequenceSequence.hpp>

#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <officecfg/Office/Common.hxx>

using namespace ::com::sun::star;

namespace
{
/// Tests for sw/source/writerfilter/dmapper/GraphicImport.cxx.
class Test : public UnoApiTest
{
public:
    Test()
        : UnoApiTest("/sw/qa/writerfilter/cppunittests/dmapper/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testTdf143455SmartArtPosition)
{
    loadFromFile(u"tdf143455_SmartArtPosition.docx");
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    uno::Reference<beans::XPropertySet> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    // Without fix in place the group, which represents the SmartArt, was placed at the initializing
    // position 0|0.
    sal_Int32 nHoriPosition = 0;
    xShape->getPropertyValue("HoriOrientPosition") >>= nHoriPosition;
    // The test would have failed with Expected: 2858, Actual: 0
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2858), nHoriPosition);
    sal_Int32 nVertPosition = 0;
    xShape->getPropertyValue("VertOrientPosition") >>= nVertPosition;
    // The test would have failed with Expected: 1588, Actual: 0
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1588), nVertPosition);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf143208wrapTight)
{
    loadFromFile(u"tdf143208_wrapTight.docx");
    // The document has a shape with indentation and contour wrap "wrapTight". Error was, that
    // the corresponding shape property 'ContourOutside=true' was not set.
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    uno::Reference<beans::XPropertySet> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    bool bContourOutside = false;
    xShape->getPropertyValue("ContourOutside") >>= bContourOutside;
    CPPUNIT_ASSERT(bContourOutside);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf142305StrokeGlowMargin)
{
    loadFromFile(u"tdf142305StrokeGlowMargin.docx");
    // The document has an arc with fat stroke and glow. Its bounding rectangle differs much
    // from the snap rectangle. Error was, that the margins were not set in a way, that the shape
    // would render similar to Word.
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    uno::Reference<beans::XPropertySet> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    sal_Int32 nTopMargin = 0;
    xShape->getPropertyValue("TopMargin") >>= nTopMargin;
    // Without fix in place top margin was 0, so that the text comes near to the shape.
    // The test would have failed with Expected: 838, Actual: 0
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(838), nTopMargin);
    sal_Int32 nBottomMargin = 0;
    // Without fix in place bottom margin was >0, so that the text was far from to the shape.
    // The test would have failed with Expected: 0, Actual: 637
    xShape->getPropertyValue("BottomMargin") >>= nBottomMargin;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), nBottomMargin);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf142305SquareWrapMargin)
{
    loadFromFile(u"tdf142305SquareWrapMargin.docx");
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY_THROW);
    uno::Reference<text::XTextViewCursor> xViewCursor(xTextViewCursorSupplier->getViewCursor());
    xViewCursor->gotoStart(/*bExpand=*/false);
    uno::Reference<view::XViewCursor> xCursor(xViewCursor, uno::UNO_QUERY);
    xCursor->goDown(/*nCount=*/10, /*bExpand=*/false);
    xViewCursor->goRight(/*nCount=*/1, /*bExpand=*/true);
    OUString sText = xViewCursor->getString();
    // Without fix in place, wrap was tight to the bounding box and not around the full shape as in
    // Word. That results in different text at start of line, here "u" instead of expected "m".
    CPPUNIT_ASSERT(sText.startsWith("m"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf142304GroupPosition)
{
    loadFromFile(u"tdf142304GroupPosition.docx");
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    uno::Reference<beans::XPropertySet> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    sal_Int32 nVertPosition = 0;
    xShape->getPropertyValue("VertOrientPosition") >>= nVertPosition;
    // Without fix in place the group was shifted left and down
    // The test would have failed with Expected: 2178, Actual: 2521
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2178), nVertPosition);
    sal_Int32 nHoriPosition = 0;
    // The test would have failed with Expected: 4304, Actual: 3874
    xShape->getPropertyValue("HoriOrientPosition") >>= nHoriPosition;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(4304), nHoriPosition);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf141540ChildRotation)
{
    loadFromFile(u"tdf141540ChildRotation.docx");
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    uno::Reference<container::XIndexAccess> xGroup(xDrawPage->getByIndex(0), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xRotatedShape(xGroup->getByIndex(1), uno::UNO_QUERY);
    sal_Int32 nShearAngle = 9000; // initialize with invalid value
    xRotatedShape->getPropertyValue("ShearAngle") >>= nShearAngle;
    // Without fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 2494
    // i.e. the rotated rectangle in the group was sheared, although the group itself is not rotated
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), nShearAngle);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf141540GroupRotation)
{
    loadFromFile(u"tdf141540GroupRotation.docx");
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    uno::Reference<beans::XPropertySet> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    sal_Int32 nShearAngle = 9000; // init with invalid value
    xShape->getPropertyValue("ShearAngle") >>= nShearAngle;
    // Without fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : -3190
    // i.e. the group has got a shearing although MSO does not know shearing at all.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), nShearAngle);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf141540GroupLinePosSize)
{
    loadFromFile(u"tdf141540GroupLinePosSize.docx");
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();

    // Test line
    uno::Reference<drawing::XShape> xLineShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    awt::Point aPosition = xLineShape->getPosition();
    awt::Size aSize = xLineShape->getSize();
    // Without fix in place, you had got Position = (19|6498), Size = 5001 x 2
    // i.e. the line was nearly horizontal instead of vertical
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(5022), aPosition.X);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2963), aPosition.Y);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), aSize.Width);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(7073), aSize.Height);

    // Test group
    uno::Reference<drawing::XShape> xGroupShape(xDrawPage->getByIndex(1), uno::UNO_QUERY);
    aPosition = xGroupShape->getPosition();
    aSize = xGroupShape->getSize();
    // Without fix in place, you had got Position = (11511|3480), Size = 4022 x 4022
    // i.e. the group was erroneously downscaled to unrotated size
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(10679), aPosition.X);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2648), aPosition.Y);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(5687), aSize.Width);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(5687), aSize.Height);
}

CPPUNIT_TEST_FIXTURE(Test, testGroupShapeRotation)
{
    loadFromFile(u"group-shape-rotation.docx");
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    uno::Reference<beans::XPropertySet> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    sal_Int32 nVertPosition = 0;
    xShape->getPropertyValue("VertOrientPosition") >>= nVertPosition;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1221
    // - Actual  : -2048
    // i.e. the group shape had a so low vertical position that the line shape did not point into
    // it.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1221), nVertPosition);
}

CPPUNIT_TEST_FIXTURE(Test, testDrawShapeInlineEffect)
{
    loadFromFile(u"draw-shape-inline-effect.docx");
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    uno::Reference<beans::XPropertySet> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    sal_Int32 nBottomMargin = 0;
    xShape->getPropertyValue("BottomMargin") >>= nBottomMargin;
    // 273 in mm100 is 98425 EMUs from the file.
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 273
    // - Actual  : 0
    // i.e. the layout result had less pages than expected (compared to Word).
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(273), nBottomMargin);
}

CPPUNIT_TEST_FIXTURE(Test, testInlineAnchoredZOrder)
{
    // Load a document which has two shapes: an inline one and an anchored one. The inline has no
    // explicit ZOrder, the anchored one has, and it's set to a value so it's visible.
    loadFromFile(u"inline-anchored-zorder.docx");
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    uno::Reference<container::XNamed> xOval(xDrawPage->getByIndex(1), uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: Oval 2
    // - Actual  :
    // i.e. the rectangle (with no name) was on top of the oval one, not the other way around.
    CPPUNIT_ASSERT_EQUAL(OUString("Oval 2"), xOval->getName());
}

CPPUNIT_TEST_FIXTURE(Test, testInlineInShapeAnchoredZOrder)
{
    // This document has a textbox shape and then an inline shape inside that.
    // The ZOrder of the inline shape is larger than the hosting textbox, so the image is visible.
    loadFromFile(u"inline-inshape-anchored-zorder.docx");
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    uno::Reference<container::XNamed> xOval(xDrawPage->getByIndex(1), uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: Picture 1
    // - Actual  : Text Box 2
    // i.e. the image was behind the textbox that was hosting it.
    CPPUNIT_ASSERT_EQUAL(OUString("Picture 1"), xOval->getName());
}

CPPUNIT_TEST_FIXTURE(Test, testRelfromhInsidemargin)
{
    loadFromFile(u"relfromh-insidemargin.docx");
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    uno::Reference<beans::XPropertySet> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    sal_Int16 nRelation = 0;
    xShape->getPropertyValue("HoriOrientRelation") >>= nRelation;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 7 (PAGE_FRAME)
    // - Actual  : 0 (FRAME)
    // i.e. the horizontal position was relative to the paragraph area, not to the entire page.
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME, nRelation);
    bool bPageToggle = false;
    xShape->getPropertyValue("PageToggle") >>= bPageToggle;
    CPPUNIT_ASSERT(bPageToggle);
}

CPPUNIT_TEST_FIXTURE(Test, testWrapPolyCrop)
{
    loadFromFile(u"wrap-poly-crop.docx");
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    uno::Reference<beans::XPropertySet> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    drawing::PointSequenceSequence aContour;
    xShape->getPropertyValue("ContourPolyPolygon") >>= aContour;
    auto aPolyPolygon = basegfx::utils::UnoPointSequenceSequenceToB2DPolyPolygon(aContour);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(1), aPolyPolygon.count());
    auto aPolygon = aPolyPolygon.getB2DPolygon(0);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(4), aPolygon.count());

    // Ideally this would be 2352, because the graphic size in mm100, using the graphic's DPI is
    // 10582, the lower 33% of the graphic is cropped, and the wrap polygon covers the middle third
    // of the area vertically. Which means 10582*2/3 = 7054.67 is the cropped height, and the top of
    // the middle third is 2351.55.
    // Then there is a 15 twips shift from the origo, so it's 2351.55 + 26.46 = 2378.01 in mm100.
    //
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2368
    // - Actual  : 3542
    // i.e. the wrap polygon covered a larger-than-correct area, which end the end means 3 lines
    // were wrapping around the image, not only 2 as Word does it.
    CPPUNIT_ASSERT_EQUAL(2368., aPolygon.getB2DPoint(0).getY());
}

CPPUNIT_TEST_FIXTURE(Test, testTextboxTextline)
{
    // Load a document with a shape with a textbox.
    // The shape's vertical relation is <wp:positionV relativeFrom="line">.
    loadFromFile(u"textbox-textline.docx");
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    uno::Reference<beans::XPropertySet> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    sal_Int16 nActualRelation{};
    CPPUNIT_ASSERT(xShape->getPropertyValue("VertOrientRelation") >>= nActualRelation);
    sal_Int32 nActualPosition{};
    CPPUNIT_ASSERT(xShape->getPropertyValue("VertOrientPosition") >>= nActualPosition);

    sal_Int16 nExpectedRelation = text::RelOrientation::TEXT_LINE;
    CPPUNIT_ASSERT_EQUAL(nExpectedRelation, nActualRelation);
    sal_Int32 nExpectedPosition = -2;
    CPPUNIT_ASSERT_EQUAL(nExpectedPosition, nActualPosition);
}

CPPUNIT_TEST_FIXTURE(Test, testTextboxTextlineTop)
{
    loadFromFile(u"textbox-textline-top.docx");
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    uno::Reference<beans::XPropertySet> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    sal_Int16 nActualRelation{};
    CPPUNIT_ASSERT(xShape->getPropertyValue("VertOrientRelation") >>= nActualRelation);
    sal_Int16 nExpectedRelation = text::RelOrientation::TEXT_LINE;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 9 (TEXT_LINE)
    // - Actual  : 0 (FRAME)
    // i.e. the anchor point for the positioning was wrong, resulting in overlapping textboxes.
    CPPUNIT_ASSERT_EQUAL(nExpectedRelation, nActualRelation);

    sal_Int16 nActualOrient{};
    CPPUNIT_ASSERT(xShape->getPropertyValue("VertOrient") >>= nActualOrient);
    sal_Int16 nExpectedOrient = text::VertOrientation::BOTTOM;
    CPPUNIT_ASSERT_EQUAL(nExpectedOrient, nActualOrient);
}

CPPUNIT_TEST_FIXTURE(Test, testLayoutInCellWrapnoneColumn)
{
    // Given a file with a table, then a shape anchored inside the cell:
    loadFromFile(u"layout-in-cell-wrapnone-column.docx");

    // Then make sure the shape can leave the cell:
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    uno::Reference<beans::XPropertySet> xShape(xDrawPage->getByIndex(1), uno::UNO_QUERY);
    uno::Reference<container::XNamed> xNamedShape(xShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Text Box 1"), xNamedShape->getName());
    bool bFollowingTextFlow = true;
    // Without the accompanying fix in place, this test would have failed, the shape was not allowed
    // to leave the cell, leading to incorrect layout.
    CPPUNIT_ASSERT(xShape->getPropertyValue("IsFollowingTextFlow") >>= bFollowingTextFlow);
    CPPUNIT_ASSERT(!bFollowingTextFlow);
}

CPPUNIT_TEST_FIXTURE(Test, testLayoutInCellOfHraphics)
{
    // Given a file with a table, then a shape anchored inside the cell:
    loadFromFile(u"layout-in-cell-2.docx");

    // Then make sure the cell obeys the layoutInCell:
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    uno::Reference<beans::XPropertySet> xShape(xDrawPage->getByIndex(1), uno::UNO_QUERY);
    bool bFollowingTextFlow = false;
    CPPUNIT_ASSERT(xShape->getPropertyValue("IsFollowingTextFlow") >>= bFollowingTextFlow);
    CPPUNIT_ASSERT(bFollowingTextFlow);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf149840SmartArtBackground)
{
    // Make sure SmartArt is loaded as group shape
    bool bUseGroup = officecfg::Office::Common::Filter::Microsoft::Import::SmartArtToShapes::get();
    if (!bUseGroup)
    {
        std::shared_ptr<comphelper::ConfigurationChanges> pChange(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Filter::Microsoft::Import::SmartArtToShapes::set(true, pChange);
        pChange->commit();
    }

    loadFromFile(u"tdf149840_SmartArtBackground.docx");
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    uno::Reference<container::XIndexAccess> xGroup(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3), xGroup->getCount());

    // The first shape in the group, which represents the SmartArt, corresponds to the background of
    // the diagram. Without fix in place it has width and height zero, which does not only result in
    // not visible background but in wrong sizes of the diagram shapes too.
    uno::Reference<drawing::XShape> xBackgroundShape(xGroup->getByIndex(0), uno::UNO_QUERY);
    awt::Size aBackgroundSize = xBackgroundShape->getSize();
    // Tolerances are for rounding inaccuracies.
    // The test would have failed with Expected: 9560x5036, Actual: 2x2
    CPPUNIT_ASSERT_DOUBLES_EQUAL(static_cast<sal_Int32>(9560), aBackgroundSize.Width, 1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(static_cast<sal_Int32>(5036), aBackgroundSize.Height, 1);

    uno::Reference<drawing::XShape> xShapeOne(xGroup->getByIndex(1), uno::UNO_QUERY);
    awt::Size aShapeOneSize = xShapeOne->getSize();
    // The test would have failed with Expected: 3282x3709, Actual: 3972x3709
    CPPUNIT_ASSERT_DOUBLES_EQUAL(static_cast<sal_Int32>(3282), aShapeOneSize.Width, 1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(static_cast<sal_Int32>(3709), aShapeOneSize.Height, 1);

    uno::Reference<drawing::XShape> xShapeTwo(xGroup->getByIndex(2), uno::UNO_QUERY);
    awt::Size aShapeTwoSize = xShapeTwo->getSize();
    // The test would have failed with Expected: 2404x5226, Actual: 2910x5226
    CPPUNIT_ASSERT_DOUBLES_EQUAL(static_cast<sal_Int32>(2404), aShapeTwoSize.Width, 1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(static_cast<sal_Int32>(5226), aShapeTwoSize.Height, 1);

    if (!bUseGroup)
    {
        std::shared_ptr<comphelper::ConfigurationChanges> pChange(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Filter::Microsoft::Import::SmartArtToShapes::set(false, pChange);
        pChange->commit();
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
