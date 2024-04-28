/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */
#pragma once

#include <utility>
#include <vcl/bitmap.hxx>
#include <vcl/dllapi.h>
#include <vcl/test/TestResult.hxx>

#include <vector>

class VCL_PLUGIN_PUBLIC VclTestResult
{
    OUString m_aTestName;
    //For storing the result of the test.
    OUString m_aTestStatus;
    //For storing the resultant bitmap correspondingly to the test.
    Bitmap m_aResultantBitmap;

public:
    VclTestResult(OUString atestName, OUString atestStatus, Bitmap atestBitmap)
        : m_aTestName(std::move(atestName))
        , m_aTestStatus(std::move(atestStatus))
        , m_aResultantBitmap(atestBitmap)
    {
    }
    const OUString& getTestName() const { return m_aTestName; }
    OUString getStatus(bool bLocalize = false);
    const Bitmap& getBitmap() const { return m_aResultantBitmap; }
};

class SAL_DLLPUBLIC_RTTI GraphicsRenderTests
{
    bool m_aStoreResultantBitmap;

    //For storing the test's info
    std::vector<VclTestResult> m_aTestResult;
    //For storing the current graphics Backend in use.
    OUString m_aCurGraphicsBackend;
    //Location where the results should be stored.
    OUString m_aUserInstallPath;

    void testDrawRectWithRectangle();
    void testDrawRectWithPixel();
    void testDrawRectWithLine();
    void testDrawRectWithPolygon();
    void testDrawRectWithPolyLine();
    void testDrawRectWithPolyLineB2D();
    void testDrawRectWithPolyPolygon();
    void testDrawRectWithPolyPolygonB2D();
    void testDrawRectAAWithRectangle();
    void testDrawRectAAWithPixel();
    void testDrawRectAAWithLine();
    void testDrawRectAAWithPolygon();
    void testDrawRectAAWithPolyLine();
    void testDrawRectAAWithPolyLineB2D();
    void testDrawRectAAWithPolyPolygon();
    void testDrawRectAAWithPolyPolygonB2D();
    void testDrawFilledRectWithRectangle();
    void testDrawFilledRectWithPolygon();
    void testDrawFilledRectWithPolyPolygon();
    void testDrawFilledRectWithPolyPolygon2D();
    void testDrawDiamondWithPolygon();
    void testDrawDiamondWithLine();
    void testDrawDiamondWithPolyline();
    void testDrawDiamondWithPolylineB2D();
    void testDrawInvertWithRectangle();
    void testDrawInvertN50WithRectangle();
    void testDrawInvertTrackFrameWithRectangle();
    void testDrawDropShapeWithPolyline();
    void testDrawDropShapeAAWithPolyline();
    void testDrawBezierWithPolylineB2D();
    void testDrawBezierAAWithPolylineB2D();
    void testDrawDropShapeWithPolygon();
    void testDrawDropShapeAAWithPolygon();
    void testDrawBitmap24bpp();
    void testDrawTransformedBitmap24bpp();
    void testComplexDrawTransformedBitmap24bpp();
    void testDrawBitmapExWithAlpha24bpp();
    void testDrawMask24bpp();
    void testDrawBlend24bpp();
    void testDrawBitmap32bpp();
    void testDrawTransformedBitmap32bpp();
    void testDrawBitmapExWithAlpha32bpp();
    void testDrawMask32bpp();
    void testDrawBlend32bpp();
    void testDrawBitmap8bppGreyScale();
    void testDrawTransformedBitmap8bppGreyScale();
    void testDrawXor();
    void testClipRectangle();
    void testClipPolygon();
    void testClipPolyPolygon();
    void testClipB2DPolyPolygon();
    void testDrawOutDev();
    void testDrawOutDevScaledClipped();
    void testDrawOutDevSelf();
    void testDashedLine();
    void testLinearGradient();
    void testLinearGradientAngled();
    void testLinearGradientBorder();
    void testLinearGradientIntensity();
    void testLinearGradientSteps();
    void testAxialGradient();
    void testRadialGradient();
    void testRadialGradientOfs();
    void testLineJoinBevel();
    void testLineJoinRound();
    void testLineJoinMiter();
    void testLineJoinNone();
    void testLineCapRound();
    void testLineCapSquare();
    void testLineCapButt();
    void testHalfEllipseWithPolyLine();
    void testHalfEllipseAAWithPolyLine();
    void testHalfEllipseWithPolyLineB2D();
    void testHalfEllipseAAWithPolyLineB2D();
    void testHalfEllipseWithPolygon();
    void testHalfEllipseAAWithPolygon();
    void testClosedBezierWithPolyline();
    void testClosedBezierWithPolygon();
    void testFilledAsymmetricalDropShape();
    void testTextDrawing();
    void testEvenOddRuleInIntersectingRectsWithPolyPolygon();
    void testEvenOddRuleInIntersectingRectsWithPolyPolygonB2D();
    void testDrawRectangleOnSize1028WithRect();
    void testDrawRectangleOnSize4096WithRect();
    void testDrawRectangleOnSize1028WithPixel();
    void testDrawRectangleOnSize4096WithPixel();
    void testDrawRectangleOnSize1028WithLine();
    void testDrawRectangleOnSize4096WithLine();
    void testDrawRectangleOnSize1028WithPolygon();
    void testDrawRectangleOnSize4096WithPolygon();
    void testDrawRectangleOnSize1028WithPolyLine();
    void testDrawRectangleOnSize4096WithPolyLine();
    void testDrawRectangleOnSize1028WithPolyLineB2D();
    void testDrawRectangleOnSize4096WithPolyLineB2D();
    void testDrawRectangleOnSize1028WithPolyPolygon();
    void testDrawRectangleOnSize4096WithPolyPolygon();
    void testDrawRectangleOnSize1028WithPolyPolygonB2D();
    void testDrawRectangleOnSize4096WithPolygonPolygonB2D();
    void testDrawOpenPolygonWithPolyLine();
    void testDrawOpenPolygonWithPolyLineB2D();
    void testDrawOpenPolygonWithPolygon();
    void testDrawOpenPolygonWithPolyPolygon();
    void testDrawOpenPolygonWithPolyPolygonB2D();
    void testDrawOpenBezierWithPolyLine();
    void testDrawOpenBezierWithPolyLineB2D();
    static OUString returnTestStatus(vcl::test::TestResult const result);
    void runALLTests();
    void appendTestResult(const OUString& rTestName, const OUString& rTestStatus,
                          const Bitmap& rTestBitmap = Bitmap());

public:
    VCL_PLUGIN_PUBLIC std::vector<VclTestResult>& getTestResults();
    VCL_PLUGIN_PUBLIC OUString getResultString(bool bLocalize = false);
    VCL_PLUGIN_PUBLIC void run(bool storeResultBitmap = false);

    GraphicsRenderTests()
        : m_aStoreResultantBitmap(false)
    {
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
