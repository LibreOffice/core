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

#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/drawing/PointSequenceSequence.hpp>

#include <basegfx/polygon/b2dpolypolygontools.hxx>

using namespace ::com::sun::star;

namespace
{
/// Tests for writerfilter/source/dmapper/GraphicImport.cxx.
class Test : public test::BootstrapFixture, public unotest::MacrosTest
{
private:
    uno::Reference<lang::XComponent> mxComponent;

public:
    void setUp() override;
    void tearDown() override;
    uno::Reference<lang::XComponent>& getComponent() { return mxComponent; }
};

void Test::setUp()
{
    test::BootstrapFixture::setUp();

    mxDesktop.set(frame::Desktop::create(mxComponentContext));
}

void Test::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

char const DATA_DIRECTORY[] = "/writerfilter/qa/cppunittests/dmapper/data/";

CPPUNIT_TEST_FIXTURE(Test, testTdf141540ChildRotation)
{
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf141540ChildRotation.docx";
    getComponent() = loadFromDesktop(aURL);
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(getComponent(), uno::UNO_QUERY);
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
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf141540GroupRotation.docx";
    getComponent() = loadFromDesktop(aURL);
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(getComponent(), uno::UNO_QUERY);
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
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf141540GroupLinePosSize.docx";
    getComponent() = loadFromDesktop(aURL);
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(getComponent(), uno::UNO_QUERY);
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
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "group-shape-rotation.docx";
    getComponent() = loadFromDesktop(aURL);
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(getComponent(), uno::UNO_QUERY);
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
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "draw-shape-inline-effect.docx";
    getComponent() = loadFromDesktop(aURL);
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(getComponent(), uno::UNO_QUERY);
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
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "inline-anchored-zorder.docx";
    getComponent() = loadFromDesktop(aURL);
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(getComponent(), uno::UNO_QUERY);
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
    OUString aURL
        = m_directories.getURLFromSrc(DATA_DIRECTORY) + "inline-inshape-anchored-zorder.docx";
    getComponent() = loadFromDesktop(aURL);
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(getComponent(), uno::UNO_QUERY);
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
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "relfromh-insidemargin.docx";
    getComponent() = loadFromDesktop(aURL);
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(getComponent(), uno::UNO_QUERY);
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
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "wrap-poly-crop.docx";
    getComponent() = loadFromDesktop(aURL);
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(getComponent(), uno::UNO_QUERY);
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
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "textbox-textline.docx";
    getComponent() = loadFromDesktop(aURL);
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(getComponent(), uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    uno::Reference<beans::XPropertySet> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    sal_Int16 nActualRelation{};
    CPPUNIT_ASSERT(xShape->getPropertyValue("VertOrientRelation") >>= nActualRelation);
    sal_Int32 nActualPosition{};
    CPPUNIT_ASSERT(xShape->getPropertyValue("VertOrientPosition") >>= nActualPosition);

    sal_Int16 nExpectedRelation = text::RelOrientation::TEXT_LINE;
    CPPUNIT_ASSERT_EQUAL(nExpectedRelation, nActualRelation);
    // 0 on this branch, but sw/qa/extras/ooxmlexport/data/tdf97371.docx shows that negative values
    // work fine in general, don't bother:
    // sal_Int32 nExpectedPosition = -2;
    // CPPUNIT_ASSERT_EQUAL(nExpectedPosition, nActualPosition);
}

CPPUNIT_TEST_FIXTURE(Test, testTextboxTextlineTop)
{
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "textbox-textline-top.docx";
    getComponent() = loadFromDesktop(aURL);
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(getComponent(), uno::UNO_QUERY);
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
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
