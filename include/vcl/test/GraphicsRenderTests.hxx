/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */
#pragma once

#include <vcl/bitmap.hxx>
#include <vcl/dllapi.h>
#include <vcl/graphicfilter.hxx>
#include <vcl/test/TestResult.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

#include <unordered_map>
#include <vector>

class VCL_PLUGIN_PUBLIC GraphicsRenderTests
{
    bool m_aStoreResultantBitmap;
    //For storing the results correspondingly to the tests.
    std::vector<OUString> m_aPassed;
    std::vector<OUString> m_aQuirky;
    std::vector<OUString> m_aFailed;
    //For storing Skipped tests.
    std::vector<OUString> m_aSkipped;
    //For storing the resultant bitmap correspondingly to the tests.
    std::unordered_map<OUString, Bitmap> m_aResultantBitmap;
    //For storing the current graphics Backend in use.
    OUString m_aCurGraphicsBackend;

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
    void testDrawBezierWithPolylineB2D();
    void testDrawBezierAAWithPolylineB2D();
    void testDrawBitmap();
    void testDrawTransformedBitmap();
    void testDrawBitmapExWithAlpha();
    void testDrawMask();
    void testDrawBlend();
    void testDrawXor();
    void testClipRectangle();
    void testClipPolygon();
    void testClipPolyPolygon();
    void testClipB2DPolyPolygon();
    void testDrawOutDev();
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
    void updateResult(vcl::test::TestResult const result, OUString atestname);
    void runALLTests();

public:
    std::vector<OUString>& getPassedTests();
    std::vector<OUString>& getQuirkyTests();
    std::vector<OUString>& getFailedTests();
    std::vector<OUString>& getSkippedTests();
    std::unordered_map<OUString, Bitmap>& getResultBitmaps();
    OUString getResultString();
    void run(bool storeResultBitmap = false);
};
