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

#include <vector>

using namespace std;

class VCL_PLUGIN_PUBLIC GraphicsRenderTests
{
    //For storing the results correspondingly to the tests.
    vector<string> m_aPassed;
    vector<string> m_aQuirky;
    vector<string> m_aFailed;
    //For storing Skipped tests.
    vector<string> m_aSkipped;
    //For storing the current graphics Backend in use.
    string m_aCurGraphicsBackend;

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
    void updateResult(vcl::test::TestResult const result, string atestname);
    void runALLTests();

public:
    void run();
};
