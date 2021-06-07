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
#include <vcl/GraphicsRenderTests.hxx>
#include <bitmap/BitmapWriteAccess.hxx>
#include <officecfg/Office/Common.hxx>
#include <officecfg/Setup.hxx>
#include <unotools/configmgr.hxx>

namespace
{
void UpdateResult(vcl::test::TestResult const result, GraphicsRenderTests* TestObject,
                  int TestNumber)
{
    switch (result)
    {
        case vcl::test::TestResult::Passed:
            TestObject->Passed.push_back(TestNumber);
            return;
        case vcl::test::TestResult::PassedWithQuirks:
            TestObject->Quirky.push_back(TestNumber);
            return;
        case vcl::test::TestResult::Failed:
            TestObject->Failed.push_back(TestNumber);
            return;
    }
}
}

void GraphicsRenderTests::testDrawRectWithRectangle()
{
    vcl::test::OutputDeviceTestRect aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap);
    UpdateResult(eResult, this, 1);
}

void GraphicsRenderTests::testDrawRectWithPixel()
{
    vcl::test::OutputDeviceTestPixel aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap);
    UpdateResult(eResult, this, 2);
}

void GraphicsRenderTests::testDrawRectWithLine()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap);
    UpdateResult(eResult, this, 3);
}

void GraphicsRenderTests::testDrawRectWithPolygon()
{
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap);
    UpdateResult(eResult, this, 4);
}

void GraphicsRenderTests::testDrawRectWithPolyLine()
{
    vcl::test::OutputDeviceTestPolyLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap);
    UpdateResult(eResult, this, 5);
}

void GraphicsRenderTests::testDrawRectWithPolyLineB2D()
{
    vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap);
    UpdateResult(eResult, this, 6);
}

void GraphicsRenderTests::testDrawRectWithPolyPolygon()
{
    vcl::test::OutputDeviceTestPolyPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap);
    UpdateResult(eResult, this, 7);
}

void GraphicsRenderTests::testDrawRectWithPolyPolygonB2D()
{
    vcl::test::OutputDeviceTestPolyPolygonB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(false);
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap);
    UpdateResult(eResult, this, 8);
}

void GraphicsRenderTests::testDrawRectAAWithRectangle()
{
    vcl::test::OutputDeviceTestRect aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
    UpdateResult(eResult, this, 9);
}

void GraphicsRenderTests::testDrawRectAAWithPixel()
{
    vcl::test::OutputDeviceTestPixel aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
    UpdateResult(eResult, this, 10);
}

void GraphicsRenderTests::testDrawRectAAWithLine()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
    UpdateResult(eResult, this, 11);
}

void GraphicsRenderTests::testDrawRectAAWithPolygon()
{
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
    UpdateResult(eResult, this, 12);
}

void GraphicsRenderTests::testDrawRectAAWithPolyLine()
{
    vcl::test::OutputDeviceTestPolyLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
    UpdateResult(eResult, this, 13);
}

void GraphicsRenderTests::testDrawRectAAWithPolyLineB2D()
{
    vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
    UpdateResult(eResult, this, 14);
}

void GraphicsRenderTests::testDrawRectAAWithPolyPolygon()
{
    vcl::test::OutputDeviceTestPolyPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
    UpdateResult(eResult, this, 15);
}

void GraphicsRenderTests::testDrawRectAAWithPolyPolygonB2D()
{
    vcl::test::OutputDeviceTestPolyPolygonB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRectangle(true);
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkRectangleAA(aBitmap);
    UpdateResult(eResult, this, 16);
}

void GraphicsRenderTests::testDrawFilledRectWithRectangle()
{
    vcl::test::OutputDeviceTestRect aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupFilledRectangle(false);
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, false);
    UpdateResult(eResult, this, 17);
    aBitmap = aOutDevTest.setupFilledRectangle(true);
    eResult = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, true);
    UpdateResult(eResult, this, 18);
}

void GraphicsRenderTests::testDrawFilledRectWithPolygon()
{
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupFilledRectangle(false);
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, false);
    UpdateResult(eResult, this, 19);
    aBitmap = aOutDevTest.setupFilledRectangle(true);
    eResult = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, true);
    UpdateResult(eResult, this, 20);
}

void GraphicsRenderTests::testDrawFilledRectWithPolyPolygon()
{
    vcl::test::OutputDeviceTestPolyPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupFilledRectangle(false);
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, false);
    UpdateResult(eResult, this, 21);
    aBitmap = aOutDevTest.setupFilledRectangle(true);
    eResult = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, true);
    UpdateResult(eResult, this, 22);
}

void GraphicsRenderTests::testDrawFilledRectWithPolyPolygon2D()
{
    vcl::test::OutputDeviceTestPolyPolygonB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupFilledRectangle(false);
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, false);
    UpdateResult(eResult, this, 23);
    aBitmap = aOutDevTest.setupFilledRectangle(true);
    eResult = vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, true);
    UpdateResult(eResult, this, 24);
}

void GraphicsRenderTests::testDrawDiamondWithPolygon()
{
    vcl::test::OutputDeviceTestPolygon aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDiamond();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkDiamond(aBitmap);
    UpdateResult(eResult, this, 25);
}

void GraphicsRenderTests::testDrawDiamondWithLine()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDiamond();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkDiamond(aBitmap);
    UpdateResult(eResult, this, 26);
}

void GraphicsRenderTests::testDrawDiamondWithPolyline()
{
    vcl::test::OutputDeviceTestPolyLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDiamond();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkDiamond(aBitmap);
    UpdateResult(eResult, this, 27);
}

void GraphicsRenderTests::testDrawDiamondWithPolylineB2D()
{
    vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDiamond();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkDiamond(aBitmap);
    UpdateResult(eResult, this, 28);
}

void GraphicsRenderTests::testDrawInvertWithRectangle()
{
    vcl::test::OutputDeviceTestRect aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupInvert_NONE();
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestCommon::checkInvertRectangle(aBitmap);
    UpdateResult(eResult, this, 29);
}

void GraphicsRenderTests::testDrawInvertN50WithRectangle()
{
    vcl::test::OutputDeviceTestRect aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupInvert_N50();
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestCommon::checkInvertN50Rectangle(aBitmap);
    UpdateResult(eResult, this, 30);
}

void GraphicsRenderTests::testDrawInvertTrackFrameWithRectangle()
{
    vcl::test::OutputDeviceTestRect aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupInvert_TrackFrame();
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestCommon::checkInvertTrackFrameRectangle(aBitmap);
    UpdateResult(eResult, this, 31);
}

void GraphicsRenderTests::testDrawBezierWithPolylineB2D()
{
    vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupBezier();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkBezier(aBitmap);
    UpdateResult(eResult, this, 32);
}

void GraphicsRenderTests::testDrawBezierAAWithPolylineB2D()
{
    vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupAABezier();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestCommon::checkBezier(aBitmap);
    UpdateResult(eResult, this, 33);
}

void GraphicsRenderTests::testDrawBitmap()
{
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawBitmap();
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestBitmap::checkTransformedBitmap(aBitmap);
    UpdateResult(eResult, this, 34);
}

void GraphicsRenderTests::testDrawTransformedBitmap()
{
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawTransformedBitmap();
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestBitmap::checkTransformedBitmap(aBitmap);
    UpdateResult(eResult, this, 35);
}

void GraphicsRenderTests::testDrawBitmapExWithAlpha()
{
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawBitmapExWithAlpha();
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestBitmap::checkBitmapExWithAlpha(aBitmap);
    UpdateResult(eResult, this, 36);
}

void GraphicsRenderTests::testDrawMask()
{
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawMask();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestBitmap::checkMask(aBitmap);
    UpdateResult(eResult, this, 37);
}

void GraphicsRenderTests::testDrawBlend()
{
    vcl::test::OutputDeviceTestBitmap aOutDevTest;
    BitmapEx aBitmapEx = aOutDevTest.setupDrawBlend();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestBitmap::checkBlend(aBitmapEx);
    UpdateResult(eResult, this, 38);
}

void GraphicsRenderTests::testDrawXor()
{
    vcl::test::OutputDeviceTestAnotherOutDev aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupXOR();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestAnotherOutDev::checkXOR(aBitmap);
    UpdateResult(eResult, this, 39);
}

void GraphicsRenderTests::testClipRectangle()
{
    vcl::test::OutputDeviceTestClip aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupClipRectangle();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestClip::checkClip(aBitmap);
    UpdateResult(eResult, this, 40);
}

void GraphicsRenderTests::testClipPolygon()
{
    vcl::test::OutputDeviceTestClip aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupClipPolygon();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestClip::checkClip(aBitmap);
    UpdateResult(eResult, this, 41);
}

void GraphicsRenderTests::testClipPolyPolygon()
{
    vcl::test::OutputDeviceTestClip aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupClipPolyPolygon();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestClip::checkClip(aBitmap);
    UpdateResult(eResult, this, 42);
}

void GraphicsRenderTests::testClipB2DPolyPolygon()
{
    vcl::test::OutputDeviceTestClip aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupClipB2DPolyPolygon();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestClip::checkClip(aBitmap);
    UpdateResult(eResult, this, 43);
}

void GraphicsRenderTests::testDrawOutDev()
{
    vcl::test::OutputDeviceTestAnotherOutDev aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDrawOutDev();
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestAnotherOutDev::checkDrawOutDev(aBitmap);
    UpdateResult(eResult, this, 44);
}

void GraphicsRenderTests::testDashedLine()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupDashedLine();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestLine::checkDashedLine(aBitmap);
    UpdateResult(eResult, this, 45);
}

void GraphicsRenderTests::testLinearGradient()
{
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLinearGradient();
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestGradient::checkLinearGradient(aBitmap);
    UpdateResult(eResult, this, 46);
}

void GraphicsRenderTests::testLinearGradientAngled()
{
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLinearGradientAngled();
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestGradient::checkLinearGradientAngled(aBitmap);
    UpdateResult(eResult, this, 47);
}

void GraphicsRenderTests::testLinearGradientBorder()
{
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLinearGradientBorder();
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestGradient::checkLinearGradientBorder(aBitmap);
    UpdateResult(eResult, this, 48);
}

void GraphicsRenderTests::testLinearGradientIntensity()
{
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLinearGradientIntensity();
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestGradient::checkLinearGradientIntensity(aBitmap);
    UpdateResult(eResult, this, 49);
}

void GraphicsRenderTests::testLinearGradientSteps()
{
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLinearGradientSteps();
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestGradient::checkLinearGradientSteps(aBitmap);
    UpdateResult(eResult, this, 50);
}

void GraphicsRenderTests::testAxialGradient()
{
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupAxialGradient();
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestGradient::checkAxialGradient(aBitmap);
    UpdateResult(eResult, this, 51);
}

void GraphicsRenderTests::testRadialGradient()
{
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRadialGradient();
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestGradient::checkRadialGradient(aBitmap);
    UpdateResult(eResult, this, 52);
}

void GraphicsRenderTests::testRadialGradientOfs()
{
    vcl::test::OutputDeviceTestGradient aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupRadialGradientOfs();
    vcl::test::TestResult eResult
        = vcl::test::OutputDeviceTestGradient::checkRadialGradientOfs(aBitmap);
    UpdateResult(eResult, this, 53);
}

void GraphicsRenderTests::testLineJoinBevel()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLineJoinBevel();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestLine::checkLineJoinBevel(aBitmap);
    UpdateResult(eResult, this, 54);
}

void GraphicsRenderTests::testLineJoinRound()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLineJoinRound();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestLine::checkLineJoinRound(aBitmap);
    UpdateResult(eResult, this, 55);
}

void GraphicsRenderTests::testLineJoinMiter()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLineJoinMiter();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestLine::checkLineJoinMiter(aBitmap);
    UpdateResult(eResult, this, 56);
}

void GraphicsRenderTests::testLineJoinNone()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLineJoinNone();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestLine::checkLineJoinNone(aBitmap);
    UpdateResult(eResult, this, 57);
}

void GraphicsRenderTests::testLineCapRound()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLineCapRound();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestLine::checkLineCapRound(aBitmap);
    UpdateResult(eResult, this, 58);
}

void GraphicsRenderTests::testLineCapSquare()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLineCapSquare();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestLine::checkLineCapSquare(aBitmap);
    UpdateResult(eResult, this, 59);
}

void GraphicsRenderTests::testLineCapButt()
{
    vcl::test::OutputDeviceTestLine aOutDevTest;
    Bitmap aBitmap = aOutDevTest.setupLineCapButt();
    vcl::test::TestResult eResult = vcl::test::OutputDeviceTestLine::checkLineCapButt(aBitmap);
    UpdateResult(eResult, this, 60);
}

void GraphicsRenderTests::testDrawTransformedBitmapExAlpha()
{
// TODO: This test is not executed for macOS unless bitmap scaling is implemented
#ifndef MACOSX
    bool ResultFlag = true;
    ScopedVclPtrInstance<VirtualDevice> device;
    device->SetOutputSizePixel(Size(16, 16));
    device->SetBackground(Wallpaper(COL_WHITE));
    device->Erase();
    Bitmap aBitmap(Size(16, 16), vcl::PixelFormat::N24_BPP);
    {
        // Fill the top left quarter with black.
        BitmapScopedWriteAccess pWriteAccess(aBitmap);
        pWriteAccess->Erase(COL_WHITE);
        for (int i = 0; i < 8; ++i)
            for (int j = 0; j < 8; ++j)
                pWriteAccess->SetPixel(j, i, COL_BLACK);
    }
    BitmapEx aBitmapEx(aBitmap);
    basegfx::B2DHomMatrix aMatrix;
    // Draw with no transformation, only alpha change.
    aMatrix.scale(16, 16);
    device->DrawTransformedBitmapEx(aMatrix, aBitmapEx, 0.5);
    BitmapEx result = device->GetBitmapEx(Point(0, 0), Size(16, 16));
    ResultFlag &= (Color(0x80, 0x80, 0x80) == result.GetPixelColor(0, 0));
    ResultFlag &= (COL_WHITE == result.GetPixelColor(15, 15));
    // Draw rotated and move to the bottom-left corner.
    device->Erase();
    aMatrix.identity();
    aMatrix.scale(16, 16);
    aMatrix.rotate(M_PI / 2);
    aMatrix.translate(8, 8);
    device->DrawTransformedBitmapEx(aMatrix, aBitmapEx, 0.5);
    result = device->GetBitmap(Point(0, 0), Size(16, 16));
    ResultFlag &= (COL_WHITE == result.GetPixelColor(0, 0));
    ResultFlag &= (Color(0x80, 0x80, 0x80) == result.GetPixelColor(0, 15));
    if (ResultFlag)
    {
        Passed.push_back(61);
    }
    else
    {
        Failed.push_back(61);
    }
#endif
}

// Test SalGraphics::blendBitmap() and blendAlphaBitmap() calls.
void GraphicsRenderTests::testDrawBlendExtended()
{
// TODO: This test is not executed for macOS unless bitmap scaling is implemented
#ifndef MACOSX
    bool ResultFlag = true;
    // Create virtual device with alpha.
    ScopedVclPtr<VirtualDevice> device
        = VclPtr<VirtualDevice>::Create(DeviceFormat::DEFAULT, DeviceFormat::DEFAULT);
    device->SetOutputSizePixel(Size(10, 10));
    device->SetBackground(Wallpaper(COL_WHITE));
    device->Erase();
    Bitmap bitmap(Size(5, 5), vcl::PixelFormat::N24_BPP);
    bitmap.Erase(COL_BLUE);
    // No alpha, this will actually call SalGraphics::DrawBitmap(), but still check
    // the alpha of the device is handled correctly.
    device->DrawBitmapEx(Point(2, 2), BitmapEx(bitmap));
    ResultFlag &= (COL_BLUE == device->GetPixel(Point(2, 2)));
    ResultFlag &= (COL_BLUE == device->GetPixel(Point(6, 6)));
    // Check pixels outside of the bitmap aren't affected.
    ResultFlag &= (COL_WHITE == device->GetPixel(Point(1, 1)));
    ResultFlag &= (COL_WHITE == device->GetPixel(Point(7, 7)));

    device->Erase();
    AlphaMask alpha(Size(5, 5));
    alpha.Erase(0); // opaque
    device->DrawBitmapEx(Point(2, 2), BitmapEx(bitmap, alpha));
    ResultFlag &= (COL_BLUE == device->GetPixel(Point(2, 2)));
    ResultFlag &= (COL_BLUE == device->GetPixel(Point(6, 6)));

    device->Erase();
    alpha.Erase(255); // transparent
    device->DrawBitmapEx(Point(2, 2), BitmapEx(bitmap, alpha));
    ResultFlag &= (COL_WHITE == device->GetPixel(Point(2, 2)));
    ResultFlag &= (COL_WHITE == device->GetPixel(Point(6, 6)));

    // Skia optimizes bitmaps that have just been Erase()-ed, so explicitly
    // set some pixels in the alpha to avoid this and have an actual bitmap
    // as the alpha mask.
    device->Erase();
    alpha.Erase(255); // transparent
    BitmapWriteAccess* alphaWrite = alpha.AcquireAlphaWriteAccess();
    alphaWrite->SetPixelIndex(0, 0, 0); // opaque
    alpha.ReleaseAccess(alphaWrite);
    device->DrawBitmapEx(Point(2, 2), BitmapEx(bitmap, alpha));
    ResultFlag &= (COL_BLUE == device->GetPixel(Point(2, 2)));
    ResultFlag &= (COL_WHITE == device->GetPixel(Point(6, 6)));
    if (ResultFlag)
    {
        Passed.push_back(62);
    }
    else
    {
        Failed.push_back(62);
    }
#endif
}

void GraphicsRenderTests::testDrawAlphaBitmapMirrored()
{
// TODO: This test is not executed for macOS unless bitmap scaling is implemented
#ifndef MACOSX
    bool ResultFlag = true;
    // Normal virtual device.
    ScopedVclPtr<VirtualDevice> device = VclPtr<VirtualDevice>::Create(DeviceFormat::DEFAULT);
    // Virtual device with alpha.
    ScopedVclPtr<VirtualDevice> alphaDevice
        = VclPtr<VirtualDevice>::Create(DeviceFormat::DEFAULT, DeviceFormat::DEFAULT);
    device->SetOutputSizePixel(Size(20, 20));
    device->SetBackground(Wallpaper(COL_BLACK));
    device->Erase();
    alphaDevice->SetOutputSizePixel(Size(20, 20));
    alphaDevice->SetBackground(Wallpaper(COL_BLACK));
    alphaDevice->Erase();
    Bitmap bitmap(Size(4, 4), vcl::PixelFormat::N24_BPP);
    AlphaMask alpha(Size(4, 4));
    bitmap.Erase(COL_LIGHTBLUE);
    {
        BitmapScopedWriteAccess writeAccess(bitmap);
        writeAccess->SetPixel(3, 3, COL_LIGHTRED);
    }
    // alpha 127 will make COL_LIGHTRED -> COL_RED and the same for blue
    alpha.Erase(127);
    // Normal device.
    device->DrawBitmapEx(Point(5, 5), Size(-4, -4), BitmapEx(bitmap));
    device->DrawBitmapEx(Point(15, 15), Size(4, 4), BitmapEx(bitmap));
    ResultFlag &= (COL_LIGHTRED == device->GetPixel(Point(18, 18)));
    ResultFlag &= (COL_LIGHTBLUE == device->GetPixel(Point(17, 18)));
    ResultFlag &= (COL_LIGHTRED == device->GetPixel(Point(2, 2)));
    ResultFlag &= (COL_LIGHTBLUE == device->GetPixel(Point(3, 2)));
    device->Erase();
    device->DrawBitmapEx(Point(5, 5), Size(-4, -4), BitmapEx(bitmap, alpha));
    device->DrawBitmapEx(Point(15, 15), Size(4, 4), BitmapEx(bitmap, alpha));
    ResultFlag &= (COL_RED == device->GetPixel(Point(18, 18)));
    ResultFlag &= (COL_BLUE == device->GetPixel(Point(17, 18)));
    ResultFlag &= (COL_RED == device->GetPixel(Point(2, 2)));
    ResultFlag &= (COL_BLUE == device->GetPixel(Point(3, 2)));
    device->Erase();
    // Now with alpha device.
    alphaDevice->DrawBitmapEx(Point(5, 5), Size(-4, -4), BitmapEx(bitmap));
    alphaDevice->DrawBitmapEx(Point(15, 15), Size(4, 4), BitmapEx(bitmap));
    ResultFlag &= (COL_LIGHTRED == alphaDevice->GetPixel(Point(18, 18)));
    ResultFlag &= (COL_LIGHTBLUE == alphaDevice->GetPixel(Point(17, 18)));
    ResultFlag &= (COL_LIGHTRED == alphaDevice->GetPixel(Point(2, 2)));
    ResultFlag &= (COL_LIGHTBLUE == alphaDevice->GetPixel(Point(3, 2)));
    alphaDevice->Erase();
    alphaDevice->DrawBitmapEx(Point(5, 5), Size(-4, -4), BitmapEx(bitmap, alpha));
    alphaDevice->DrawBitmapEx(Point(15, 15), Size(4, 4), BitmapEx(bitmap, alpha));
    ResultFlag &= (COL_RED == alphaDevice->GetPixel(Point(18, 18)));
    ResultFlag &= (COL_BLUE == alphaDevice->GetPixel(Point(17, 18)));
    ResultFlag &= (COL_RED == alphaDevice->GetPixel(Point(2, 2)));
    ResultFlag &= (COL_BLUE == alphaDevice->GetPixel(Point(3, 2)));
    alphaDevice->Erase();
    if (ResultFlag)
    {
        Passed.push_back(63);
    }
    else
    {
        Failed.push_back(63);
    }
#endif
}

void GraphicsRenderTests::testErase()
{
    bool ResultFlag = true;
    {
        // Create normal virtual device (no alpha).
        ScopedVclPtr<VirtualDevice> device = VclPtr<VirtualDevice>::Create(DeviceFormat::DEFAULT);
        device->SetOutputSizePixel(Size(10, 10));
        // Erase with white, check it's white.
        device->SetBackground(Wallpaper(COL_WHITE));
        device->Erase();
        ResultFlag &= (COL_WHITE == device->GetPixel(Point(0, 0)));
        ResultFlag &= (COL_WHITE == device->GetPixel(Point(9, 9)));
        ResultFlag &= (COL_WHITE == device->GetPixel(Point(5, 5)));
        // Erase with black, check it's black.
        device->SetBackground(Wallpaper(COL_BLACK));
        device->Erase();
        ResultFlag &= (COL_BLACK == device->GetPixel(Point(0, 0)));
        ResultFlag &= (COL_BLACK == device->GetPixel(Point(9, 9)));
        ResultFlag &= (COL_BLACK == device->GetPixel(Point(5, 5)));
        // Erase with cyan, check it's cyan.
        device->SetBackground(Wallpaper(COL_CYAN));
        device->Erase();
        ResultFlag &= (COL_CYAN == device->GetPixel(Point(0, 0)));
        ResultFlag &= (COL_CYAN == device->GetPixel(Point(9, 9)));
        ResultFlag &= (COL_CYAN == device->GetPixel(Point(5, 5)));
    }
    {
        // Create virtual device with alpha.
        ScopedVclPtr<VirtualDevice> device
            = VclPtr<VirtualDevice>::Create(DeviceFormat::DEFAULT, DeviceFormat::DEFAULT);
        device->SetOutputSizePixel(Size(10, 10));
        // Erase with white, check it's white.
        device->SetBackground(Wallpaper(COL_WHITE));
        device->Erase();
        ResultFlag &= (COL_WHITE == device->GetPixel(Point(0, 0)));
        ResultFlag &= (COL_WHITE == device->GetPixel(Point(9, 9)));
        ResultFlag &= (COL_WHITE == device->GetPixel(Point(5, 5)));
        // Erase with black, check it's black.
        device->SetBackground(Wallpaper(COL_BLACK));
        device->Erase();
        ResultFlag &= (COL_BLACK == device->GetPixel(Point(0, 0)));
        ResultFlag &= (COL_BLACK == device->GetPixel(Point(9, 9)));
        ResultFlag &= (COL_BLACK == device->GetPixel(Point(5, 5)));
        // Erase with cyan, check it's cyan.
        device->SetBackground(Wallpaper(COL_CYAN));
        device->Erase();
        ResultFlag &= (COL_CYAN == device->GetPixel(Point(0, 0)));
        ResultFlag &= (COL_CYAN == device->GetPixel(Point(9, 9)));
        ResultFlag &= (COL_CYAN == device->GetPixel(Point(5, 5)));
        // Erase with transparent, check it's transparent.
        device->SetBackground(Wallpaper(COL_TRANSPARENT));
        device->Erase();
        ResultFlag &= (sal_uInt8(0) == device->GetPixel(Point(0, 0)).GetAlpha());
        ResultFlag &= (sal_uInt8(0) == device->GetPixel(Point(9, 9)).GetAlpha());
        ResultFlag &= (sal_uInt8(0) == device->GetPixel(Point(5, 5)).GetAlpha());
    }
    if (ResultFlag)
    {
        Passed.push_back(64);
    }
    else
    {
        Failed.push_back(64);
    }
}

void GraphicsRenderTests::testTdf124848()
{
// TODO: This unit test is not executed for macOS unless bitmap scaling is implemented
#ifndef MACOSX
    bool ResultFlag = true;
    ScopedVclPtr<VirtualDevice> device = VclPtr<VirtualDevice>::Create(DeviceFormat::DEFAULT);
    device->SetOutputSizePixel(Size(100, 100));
    device->SetBackground(Wallpaper(COL_WHITE));
    device->Erase();
    device->SetAntialiasing(AntialiasingFlags::Enable);
    device->SetLineColor(COL_BLACK);
    basegfx::B2DHomMatrix matrix;
    // DrawPolyLine() would apply the whole matrix to the line width, making it negative
    // in case of a larger rotation.
    matrix.rotate(M_PI); //180 degrees
    matrix.translate(100, 100);
    ResultFlag
        &= (device->DrawPolyLineDirect(matrix, basegfx::B2DPolygon{ { 50, 50 }, { 50, 100 } }, 100,
                                       0, nullptr, basegfx::B2DLineJoin::Miter));
    // 100px wide line should fill the entire width of the upper half
    ResultFlag &= (COL_BLACK == device->GetPixel(Point(2, 2)));

    // Also check hairline.
    device->Erase();
    ResultFlag
        &= (device->DrawPolyLineDirect(matrix, basegfx::B2DPolygon{ { 50, 50 }, { 50, 100 } }, 0, 0,
                                       nullptr, basegfx::B2DLineJoin::Miter));
    // 1px wide
    ResultFlag &= (COL_BLACK == device->GetPixel(Point(50, 20)));
    ResultFlag &= (COL_WHITE == device->GetPixel(Point(49, 20)));
    ResultFlag &= (COL_WHITE == device->GetPixel(Point(51, 20)));
    if (ResultFlag)
    {
        Passed.push_back(65);
    }
    else
    {
        Failed.push_back(65);
    }
#endif
}

void GraphicsRenderTests::testTdf136171()
{
// TODO: Following tests are not executed for macOS unless bitmap scaling is implemented
#ifndef MACOSX
    bool ResultFlag = true;
    // Create virtual device with alpha.
    ScopedVclPtr<VirtualDevice> device
        = VclPtr<VirtualDevice>::Create(DeviceFormat::DEFAULT, DeviceFormat::DEFAULT);
    device->SetOutputSizePixel(Size(10, 10));
    device->SetBackground(Wallpaper(COL_WHITE));
    device->Erase();
    Bitmap bitmap(Size(10, 10), vcl::PixelFormat::N24_BPP);
    bitmap.Erase(COL_BLUE);
    basegfx::B2DHomMatrix matrix;
    matrix.scale(bitmap.GetSizePixel().Width(),
                 bitmap.GetSizePixel().Height()); // draw as 10x10
    // Draw a blue bitmap to the device. The bug was that there was no alpha, but OutputDevice::DrawTransformBitmapExDirect()
    // supplied a fully opaque alpha done with Erase() on the alpha bitmap, and Skia backend didn't handle such alpha correctly.
    device->DrawTransformedBitmapEx(matrix, BitmapEx(bitmap));
    // The whole virtual device content now should be blue.
    ResultFlag &= (COL_BLUE == device->GetPixel(Point(0, 0)));
    ResultFlag &= (COL_BLUE == device->GetPixel(Point(9, 0)));
    ResultFlag &= (COL_BLUE == device->GetPixel(Point(0, 9)));
    ResultFlag &= (COL_BLUE == device->GetPixel(Point(9, 9)));
    ResultFlag &= (COL_BLUE == device->GetPixel(Point(4, 4)));
    if (ResultFlag)
    {
        Passed.push_back(66);
    }
    else
    {
        Failed.push_back(66);
    }
#endif
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

void GraphicsRenderTests::RunVclTests()
{
    OUString sSetupVersion = utl::ConfigManager::getProductVersion();
    sal_Int32 iCurrent
        = sSetupVersion.getToken(0, '.').toInt32() * 10 + sSetupVersion.getToken(1, '.').toInt32();
    OUString sLastVersion = officecfg::Setup::Product::ooSetupLastVersion::get().value_or("0.0");
    sal_Int32 iLast
        = sLastVersion.getToken(0, '.').toInt32() * 10 + sLastVersion.getToken(1, '.').toInt32();
    if (iCurrent > iLast)
    {
        RunALLTests();
    }
}
