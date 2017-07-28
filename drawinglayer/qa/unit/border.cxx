/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <com/sun/star/table/BorderLineStyle.hpp>

#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <drawinglayer/primitive2d/borderlineprimitive2d.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <drawinglayer/processor2d/processorfromoutputdevice.hxx>
#include <rtl/ref.hxx>
#include <test/bootstrapfixture.hxx>
#include <vcl/vclptr.hxx>
#include <vcl/virdev.hxx>
#include <editeng/borderline.hxx>

using namespace com::sun::star;

namespace
{

class DrawinglayerBorderTest : public test::BootstrapFixture
{
public:
    void testDoubleDecompositionSolid();
    void testDoublePixelProcessing();

    CPPUNIT_TEST_SUITE(DrawinglayerBorderTest);
    CPPUNIT_TEST(testDoubleDecompositionSolid);
    CPPUNIT_TEST(testDoublePixelProcessing);
    CPPUNIT_TEST_SUITE_END();
};

void DrawinglayerBorderTest::testDoubleDecompositionSolid()
{
    // Create a border line primitive that's similar to the one from the bugdoc:
    // 1.47 pixels is 0.03cm at 130% zoom and 96 DPI.
    basegfx::B2DPoint aStart(0, 20);
    basegfx::B2DPoint aEnd(100, 20);
    double fLeftWidth = 1.47;
    double const fDistance = 1.47;
    double const fRightWidth = 1.47;
    double const fExtendLeftStart = 0;
    double const fExtendLeftEnd = 0;
    double const fExtendRightStart = 0;
    double const fExtendRightEnd = 0;
    basegfx::BColor aColorRight;
    basegfx::BColor aColorLeft;
    basegfx::BColor aColorGap;
    bool const bHasGapColor = false;
    SvxBorderLineStyle const nStyle = SvxBorderLineStyle::DOUBLE;
    rtl::Reference<drawinglayer::primitive2d::BorderLinePrimitive2D> aBorder(
        new drawinglayer::primitive2d::BorderLinePrimitive2D(
            aStart,
            aEnd,
            drawinglayer::primitive2d::BorderLine(fLeftWidth, aColorLeft, fExtendLeftStart, fExtendLeftEnd),
            drawinglayer::primitive2d::BorderLine(fDistance, aColorGap),
            drawinglayer::primitive2d::BorderLine(fRightWidth, aColorRight, fExtendRightStart, fExtendRightEnd),
            bHasGapColor,
            nStyle));

    // Decompose it into polygons.
    drawinglayer::geometry::ViewInformation2D aView;
    drawinglayer::primitive2d::Primitive2DContainer aContainer;
    aBorder->get2DDecomposition(aContainer, aView);

    // Make sure it results in two borders as it's a double one.
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(2), aContainer.size());

    // Get the inside line, now a PolygonStrokePrimitive2D
    auto pInside = dynamic_cast<const drawinglayer::primitive2d::PolygonStrokePrimitive2D*>(aContainer[0].get());
    CPPUNIT_ASSERT(pInside);

    // Make sure the inside line's height is fLeftWidth.
    const double fLineWidthFromDecompose = pInside->getLineAttribute().getWidth();

    // This was 2.47, i.e. the width of the inner line was 1 unit (in the bugdoc's case: 1 pixel) wider than expected.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(fLeftWidth, fLineWidthFromDecompose, basegfx::fTools::getSmallValue());
}

void DrawinglayerBorderTest::testDoublePixelProcessing()
{
    // Create a pixel processor.
    ScopedVclPtrInstance<VirtualDevice> pDev;
    drawinglayer::geometry::ViewInformation2D aView;
    std::unique_ptr<drawinglayer::processor2d::BaseProcessor2D> pProcessor(drawinglayer::processor2d::createBaseProcessor2DFromOutputDevice(*pDev, aView));
    CPPUNIT_ASSERT(pProcessor);
    GDIMetaFile aMetaFile;
    // Start recording after the processor is created, so we can test the pixel processor.
    aMetaFile.Record(pDev);

    // Create a border line primitive that's similar to the one from the bugdoc:
    // 1.47 pixels is 0.03cm at 130% zoom and 96 DPI.
    basegfx::B2DPoint aStart(0, 20);
    basegfx::B2DPoint aEnd(100, 20);
    double const fLeftWidth = 1.47;
    double const fDistance = 1.47;
    double const fRightWidth = 1.47;
    double const fExtendLeftStart = 0;
    double const fExtendLeftEnd = 0;
    double const fExtendRightStart = 0;
    double const fExtendRightEnd = 0;
    basegfx::BColor aColorRight;
    basegfx::BColor aColorLeft;
    basegfx::BColor aColorGap;
    bool const bHasGapColor = false;
    SvxBorderLineStyle const nStyle = SvxBorderLineStyle::DOUBLE;
    rtl::Reference<drawinglayer::primitive2d::BorderLinePrimitive2D> xBorder(
        new drawinglayer::primitive2d::BorderLinePrimitive2D(
            aStart,
            aEnd,
            drawinglayer::primitive2d::BorderLine(fLeftWidth, aColorLeft, fExtendLeftStart, fExtendLeftEnd),
            drawinglayer::primitive2d::BorderLine(fDistance, aColorGap),
            drawinglayer::primitive2d::BorderLine(fRightWidth, aColorRight, fExtendRightStart, fExtendRightEnd),
            bHasGapColor,
            nStyle));

    drawinglayer::primitive2d::Primitive2DContainer aPrimitives;
    aPrimitives.push_back(drawinglayer::primitive2d::Primitive2DReference(xBorder.get()));

    // Process the primitives.
    pProcessor->process(aPrimitives);

    // Double line now gets decomposed in Metafile to painting four lines
    // with width == 0 in a cross pattern due to real line width being between
    // 1.0 and 2.0. Count created lines
    aMetaFile.Stop();
    aMetaFile.WindStart();
    sal_uInt32 nPolyLineActionCount = 0;

    for (std::size_t nAction = 0; nAction < aMetaFile.GetActionSize(); ++nAction)
    {
        MetaAction* pAction = aMetaFile.GetAction(nAction);

        if (MetaActionType::POLYLINE == pAction->GetType())
        {
            auto pMPLAction = static_cast<MetaPolyLineAction*>(pAction);

            if (0 == pMPLAction->GetLineInfo().GetWidth() && LineStyle::Solid == pMPLAction->GetLineInfo().GetStyle())
            {
                nPolyLineActionCount++;
            }
        }
    }

    // Check if all eight (2x four) simple lines with width == 0 and
    // solid were created
    const sal_uInt32 nExpectedNumPolyLineActions = 8;

    CPPUNIT_ASSERT_EQUAL(nExpectedNumPolyLineActions, nPolyLineActionCount);
}

CPPUNIT_TEST_SUITE_REGISTRATION(DrawinglayerBorderTest);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
