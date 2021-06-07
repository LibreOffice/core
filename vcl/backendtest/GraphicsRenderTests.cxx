/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <test/outputdevice.hxx>
#include <vcl/test/GraphicsRenderTests.hxx>
#include <bitmap/BitmapWriteAccess.hxx>

void GraphicsRenderTests::updateResult(vcl::test::TestResult const result, int atestname)
{
    switch (result)
    {
        case vcl::test::TestResult::Passed:
            m_aPassed.push_back(TestNumber);
            return;
        case vcl::test::TestResult::PassedWithQuirks:
            m_aQuirky.push_back(TestNumber);
            return;
        case vcl::test::TestResult::Failed:
            m_aFailed.push_back(TestNumber);
            return;
    }
}

void GraphicsRenderTests::testDrawRectWithRectangle()
{
    vcl::test::OutputDeviceTestRect aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap);
    UpdateResult(eResult, "testDrawRectWithRectangle");
}

void GraphicsRenderTests::testDrawRectWithPixel()
{
    vcl::test::OutputDeviceTestPixel aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap);
    UpdateResult(eResult, "testDrawRectWithPixel");
}

void GraphicsRenderTests::testDrawRectWithLine()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap);
    UpdateResult(eResult, "testDrawRectWithLine");
}

void GraphicsRenderTests::testDrawRectWithPolygon()
{
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap);
    UpdateResult(eResult, "testDrawRectWithPolygon");
}

void GraphicsRenderTests::testDrawRectWithPolyLine()
{
    vcl::test::OutputDeviceTestPolyLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap);
    UpdateResult(eResult, "testDrawRectWithPolyLine");
}

void GraphicsRenderTests::testDrawRectWithPolyLineB2D()
{
    vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap);
    UpdateResult(eResult, "testDrawRectWithPolyLineB2D");
}

void GraphicsRenderTests::testDrawRectWithPolyPolygon()
{
    vcl::test::OutputDeviceTestPolyPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap);
    UpdateResult(eResult, "testDrawRectWithPolyPolygon");
}

void GraphicsRenderTests::testDrawRectWithPolyPolygonB2D()
{
    vcl::test::OutputDeviceTestPolyPolygonB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap);
    UpdateResult(eResult, "testDrawRectWithPolyPolygonB2D");
}

void GraphicsRenderTests::testDrawRectAAWithRectangle()
{
    vcl::test::OutputDeviceTestRect aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
    UpdateResult(eResult, "testDrawRectAAWithRectangle");
}

void GraphicsRenderTests::testDrawRectAAWithPixel()
{
    vcl::test::OutputDeviceTestPixel aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
    UpdateResult(eResult, "testDrawRectAAWithPixel");
}

void GraphicsRenderTests::testDrawRectAAWithLine()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
    UpdateResult(eResult, "testDrawRectAAWithLine");
}

void GraphicsRenderTests::testDrawRectAAWithPolygon()
{
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
    UpdateResult(eResult, "testDrawRectAAWithPolygon");
}

void GraphicsRenderTests::testDrawRectAAWithPolyLine()
{
    vcl::test::OutputDeviceTestPolyLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
    UpdateResult(eResult, "testDrawRectAAWithPolygon");
}

void GraphicsRenderTests::testDrawRectAAWithPolyLineB2D()
{
    vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
    UpdateResult(eResult, "testDrawRectAAWithPolyLineB2D");
}

void GraphicsRenderTests::testDrawRectAAWithPolyPolygon()
{
    vcl::test::OutputDeviceTestPolyPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
    UpdateResult(eResult, "testDrawRectAAWithPolyPolygon");
}

void GraphicsRenderTests::testDrawRectAAWithPolyPolygonB2D()
{
    vcl::test::OutputDeviceTestPolyPolygonB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
    UpdateResult(eResult, "testDrawRectAAWithPolyPolygonB2D");
}

void GraphicsRenderTests::testDrawFilledRectWithRectangle()
{
    vcl::test::OutputDeviceTestRect aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupFilledRectangle(false);
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, false);
    UpdateResult(eResult, "testDrawFilledRectWithRectangle");
    aBitmap = aOutDevTest.setupFilledRectangle(true);
    eResult = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, true);
    UpdateResult(eResult, "testDrawFilledRectWithRectangleWithAA");
}

void GraphicsRenderTests::testDrawFilledRectWithPolygon()
{
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupFilledRectangle(false);
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, false);
    UpdateResult(eResult, "testDrawFilledRectWithPolygon");
    aBitmap = aOutDevTest.setupFilledRectangle(true);
    eResult = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, true);
    UpdateResult(eResult, "testDrawFilledRectWithPolygonWithAA");
}

void GraphicsRenderTests::testDrawFilledRectWithPolyPolygon()
{
    vcl::test::OutputDeviceTestPolyPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupFilledRectangle(false);
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, false);
    UpdateResult(eResult, "testDrawFilledRectWithPolyPolygon");
    aBitmap = aOutDevTest.setupFilledRectangle(true);
    eResult = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, true);
    UpdateResult(eResult, "testDrawFilledRectWithPolyPolygonWithAA");
}

void GraphicsRenderTests::testDrawFilledRectWithPolyPolygon2D()
{
    vcl::test::OutputDeviceTestPolyPolygonB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupFilledRectangle(false);
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, false);
    UpdateResult(eResult, "testDrawFilledRectWithPolyPolygon2D");
    aBitmap = aOutDevTest.setupFilledRectangle(true);
    eResult = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, true);
    UpdateResult(eResult, "testDrawFilledRectWithPolyPolygon2DWithAA");
}

void GraphicsRenderTests::testDrawDiamondWithPolygon()
{
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDiamond();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkDiamond(aBitmap);
    UpdateResult(eResult, "testDrawDiamondWithPolygon");
}

void GraphicsRenderTests::testDrawDiamondWithLine()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDiamond();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkDiamond(aBitmap);
    UpdateResult(eResult, "testDrawDiamondWithLine");
}

void GraphicsRenderTests::testDrawDiamondWithPolyline()
{
    vcl::test::OutputDeviceTestPolyLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDiamond();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkDiamond(aBitmap);
    UpdateResult(eResult, "testDrawDiamondWithPolyline");
}

void GraphicsRenderTests::testDrawDiamondWithPolylineB2D()
{
    vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDiamond();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkDiamond(aBitmap);
    UpdateResult(eResult, "testDrawDiamondWithPolylineB2D");
}

void GraphicsRenderTests::testDrawInvertWithRectangle()
{
    vcl::test::OutputDeviceTestRect aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupInvert_NONE();
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestCommon::checkInvertRectangle(aBitmap);
    UpdateResult(eResult, "testDrawInvertWithRectangle");
}

void GraphicsRenderTests::testDrawInvertN50WithRectangle()
{
    vcl::test::OutputDeviceTestRect aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupInvert_N50();
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestCommon::checkInvertN50Rectangle(aBitmap);
    UpdateResult(eResult, "testDrawInvertN50WithRectangle");
}

void GraphicsRenderTests::testDrawInvertTrackFrameWithRectangle()
{
    vcl::test::OutputDeviceTestRect aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupInvert_TrackFrame();
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestCommon::checkInvertTrackFrameRectangle(aBitmap);
    UpdateResult(eResult, "testDrawInvertTrackFrameWithRectangle");
}

void GraphicsRenderTests::testDrawBezierWithPolylineB2D()
{
    vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupBezier();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkBezier(aBitmap);
    UpdateResult(eResult, "testDrawBezierWithPolylineB2D");
}

void GraphicsRenderTests::testDrawBezierAAWithPolylineB2D()
{
    vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupAABezier();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkBezier(aBitmap);
    UpdateResult(eResult, "testDrawBezierAAWithPolylineB2D");
}

void GraphicsRenderTests::testDrawBitmap()
{
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawBitmap();
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestBitmap::checkTransformedBitmap(aBitmap);
    UpdateResult(eResult, "testDrawBitmap");
}

void GraphicsRenderTests::testDrawTransformedBitmap()
{
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawTransformedBitmap();
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestBitmap::checkTransformedBitmap(aBitmap);
    UpdateResult(eResult, "testDrawTransformedBitmap");
}

void GraphicsRenderTests::testDrawBitmapExWithAlpha()
{
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawBitmapExWithAlpha();
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestBitmap::checkBitmapExWithAlpha(aBitmap);
    UpdateResult(eResult, "testDrawBitmapExWithAlpha");
}

void GraphicsRenderTests::testDrawMask()
{
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawMask();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestBitmap::checkMask(aBitmap);
    UpdateResult(eResult, "testDrawMask");
}

void GraphicsRenderTests::testDrawBlend()
{
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    BitmapEx aBitmapEx = aOutDevTest.setupDrawBlend();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestBitmap::checkBlend(aBitmapEx);
    UpdateResult(eResult, "testDrawBlend");
}

void GraphicsRenderTests::testDrawXor()
{
    vcl::test::OutputDeviceTestAnotherOutDev aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupXOR();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestAnotherOutDev::checkXOR(aBitmap);
    UpdateResult(eResult, "testDrawXor");
}

void GraphicsRenderTests::testClipRectangle()
{
    vcl::test::OutputDeviceTestClip aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupClipRectangle();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestClip::checkClip(aBitmap);
    UpdateResult(eResult, "testClipRectangle");
}

void GraphicsRenderTests::testClipPolygon()
{
    vcl::test::OutputDeviceTestClip aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupClipPolygon();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestClip::checkClip(aBitmap);
    UpdateResult(eResult, "testClipPolygon");
}

void GraphicsRenderTests::testClipPolyPolygon()
{
    vcl::test::OutputDeviceTestClip aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupClipPolyPolygon();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestClip::checkClip(aBitmap);
    UpdateResult(eResult, "testClipPolyPolygon");
}

void GraphicsRenderTests::testClipB2DPolyPolygon()
{
    vcl::test::OutputDeviceTestClip aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupClipB2DPolyPolygon();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestClip::checkClip(aBitmap);
    UpdateResult(eResult, "testClipB2DPolyPolygon");
}

void GraphicsRenderTests::testDrawOutDev()
{
    vcl::test::OutputDeviceTestAnotherOutDev aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawOutDev();
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestAnotherOutDev::checkDrawOutDev(aBitmap);
    UpdateResult(eResult, "testDrawOutDev");
}

void GraphicsRenderTests::testDashedLine()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDashedLine();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestLine::checkDashedLine(aBitmap);
    UpdateResult(eResult, "testDashedLine");
}

void GraphicsRenderTests::testLinearGradient()
{
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLinearGradient();
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestGradient::checkLinearGradient(aBitmap);
    UpdateResult(eResult, "testLinearGradient");
}

void GraphicsRenderTests::testLinearGradientAngled()
{
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLinearGradientAngled();
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestGradient::checkLinearGradientAngled(aBitmap);
    UpdateResult(eResult, "testLinearGradientAngled");
}

void GraphicsRenderTests::testLinearGradientBorder()
{
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLinearGradientBorder();
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestGradient::checkLinearGradientBorder(aBitmap);
    UpdateResult(eResult, "testLinearGradientBorder");
}

void GraphicsRenderTests::testLinearGradientIntensity()
{
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLinearGradientIntensity();
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestGradient::checkLinearGradientIntensity(aBitmap);
    UpdateResult(eResult, "testLinearGradientIntensity");
}

void GraphicsRenderTests::testLinearGradientSteps()
{
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLinearGradientSteps();
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestGradient::checkLinearGradientSteps(aBitmap);
    UpdateResult(eResult, "testLinearGradientSteps");
}

void GraphicsRenderTests::testAxialGradient()
{
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupAxialGradient();
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestGradient::checkAxialGradient(aBitmap);
    UpdateResult(eResult, "testAxialGradient");
}

void GraphicsRenderTests::testRadialGradient()
{
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRadialGradient();
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestGradient::checkRadialGradient(aBitmap);
    UpdateResult(eResult, "testRadialGradient");
}

void GraphicsRenderTests::testRadialGradientOfs()
{
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRadialGradientOfs();
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestGradient::checkRadialGradientOfs(aBitmap);
    UpdateResult(eResult, "testRadialGradientOfs");
}

void GraphicsRenderTests::testLineJoinBevel()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLineJoinBevel();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestLine::checkLineJoinBevel(aBitmap);
    UpdateResult(eResult, "testLineJoinBevel");
}

void GraphicsRenderTests::testLineJoinRound()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLineJoinRound();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestLine::checkLineJoinRound(aBitmap);
    UpdateResult(eResult, "testLineJoinRound");
}

void GraphicsRenderTests::testLineJoinMiter()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLineJoinMiter();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestLine::checkLineJoinMiter(aBitmap);
    UpdateResult(eResult, "testLineJoinMiter");
}

void GraphicsRenderTests::testLineJoinNone()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLineJoinNone();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestLine::checkLineJoinNone(aBitmap);
    UpdateResult(eResult, "testLineJoinNone");
}

void GraphicsRenderTests::testLineCapRound()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLineCapRound();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestLine::checkLineCapRound(aBitmap);
    UpdateResult(eResult, "testLineCapRound");
}

void GraphicsRenderTests::testLineCapSquare()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLineCapSquare();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestLine::checkLineCapSquare(aBitmap);
    UpdateResult(eResult, "testLineCapSquare");
}

void GraphicsRenderTests::testLineCapButt()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLineCapButt();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestLine::checkLineCapButt(aBitmap);
    UpdateResult(eResult, "testLineCapButt");
}

void GraphicsRenderTests::RunALLTests()
{
    testDrawRectWithRectangle();
    testDrawRectWithPixel();
    testDrawRectWithLine();
    testDrawRectWithPolygon();
    testDrawRectWithPolyLine();
    testDrawRectWithPolyLineB2D();
    testDrawRectWithPolyPolygon();
    testDrawRectWithPolyPolygonB2D();
    testDrawRectAAWithRectangle();
    testDrawRectAAWithPixel();
    testDrawRectAAWithLine();
    testDrawRectAAWithPolygon();
    testDrawRectAAWithPolyLine();
    testDrawRectAAWithPolyLineB2D();
    testDrawRectAAWithPolyPolygon();
    testDrawRectAAWithPolyPolygonB2D();
    testDrawFilledRectWithRectangle();
    testDrawFilledRectWithPolygon();
    testDrawFilledRectWithPolyPolygon();
    testDrawFilledRectWithPolyPolygon2D();
    testDrawDiamondWithPolygon();
    testDrawDiamondWithLine();
    testDrawDiamondWithPolyline();
    testDrawDiamondWithPolylineB2D();
    testDrawInvertWithRectangle();
    testDrawInvertN50WithRectangle();
    testDrawInvertTrackFrameWithRectangle();
    testDrawBezierWithPolylineB2D();
    testDrawBezierAAWithPolylineB2D();
    testDrawBitmap();
    testDrawTransformedBitmap();
    testDrawBitmapExWithAlpha();
    testDrawMask();
    testDrawBlend();
    testDrawXor();
    testClipRectangle();
    testClipPolygon();
    testClipPolyPolygon();
    testClipB2DPolyPolygon();
    testDrawOutDev();
    testDashedLine();
    testLinearGradient();
    testLinearGradientAngled();
    testLinearGradientBorder();
    testLinearGradientIntensity();
    testLinearGradientSteps();
    testAxialGradient();
    testRadialGradient();
    testRadialGradientOfs();
    testLineJoinBevel();
    testLineJoinRound();
    testLineJoinMiter();
    testLineJoinNone();
    testLineCapRound();
    testLineCapSquare();
    testLineCapButt();
    testDrawTransformedBitmapExAlpha();
    testErase();
    testDrawBlendExtended();
    testDrawAlphaBitmapMirrored();
    testTdf124848();
    testTdf136171();
}

void GraphicsRenderTests::run() { RunALLTests(); }
