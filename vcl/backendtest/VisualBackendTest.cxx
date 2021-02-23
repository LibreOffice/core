/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <math.h>
#include <sal/log.hxx>

#include <comphelper/processfactory.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <vcl/bitmapex.hxx>
#include <vcl/event.hxx>
#include <vcl/gradient.hxx>
#include <vcl/vclmain.hxx>

#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/virdev.hxx>

#include <basegfx/numeric/ftools.hxx>
#include <tools/diagnose_ex.h>

#include <chrono>
#include <iostream>

#include <test/outputdevice.hxx>

using namespace css;

static void drawBitmapCentered(tools::Rectangle const& rRect, const Bitmap& aBitmap,
                        vcl::RenderContext& rRenderContext)
{
    tools::Long nWidth = rRect.GetWidth();
    tools::Long nHeight = rRect.GetHeight();

    Size aBitmapSize(aBitmap.GetSizePixel());

    Point aPoint(rRect.TopLeft());

    aPoint.AdjustX((nWidth  - aBitmapSize.Width())  / 2 );
    aPoint.AdjustY((nHeight - aBitmapSize.Height()) / 2 );

    rRenderContext.DrawBitmap(aPoint, aBitmap);
}

static void drawBitmapScaledAndCentered(tools::Rectangle const & rRect, Bitmap aBitmap, vcl::RenderContext& rRenderContext, BmpScaleFlag aFlag = BmpScaleFlag::Fast)
{
    tools::Long nWidth = rRect.GetWidth();
    tools::Long nHeight = rRect.GetHeight();

    Size aBitmapSize(aBitmap.GetSizePixel());

    double fWidthHeight = std::min(nWidth, nHeight);
    double fScale = fWidthHeight / aBitmapSize.Width();
    aBitmap.Scale(fScale, fScale, aFlag);

    drawBitmapCentered(rRect, aBitmap, rRenderContext);
}

static void drawBackgroundRect(tools::Rectangle const & rRect, Color aColor, vcl::RenderContext& rRenderContext)
{
    rRenderContext.Push(PushFlags::LINECOLOR | PushFlags::FILLCOLOR);
    rRenderContext.SetFillColor(aColor);
    rRenderContext.SetLineColor(aColor);
    rRenderContext.DrawRect(rRect);
    rRenderContext.Pop();
}

static void assertAndSetBackground(vcl::test::TestResult eResult, tools::Rectangle const & rRect, vcl::RenderContext& rRenderContext)
{
    if (eResult == vcl::test::TestResult::Passed)
        drawBackgroundRect(rRect, COL_GREEN, rRenderContext);
    else if (eResult == vcl::test::TestResult::PassedWithQuirks)
        drawBackgroundRect(rRect, COL_YELLOW, rRenderContext);
    else if (eResult == vcl::test::TestResult::Failed)
        drawBackgroundRect(rRect, COL_RED, rRenderContext);
}

namespace {

class VisualBackendTestWindow : public WorkWindow
{
private:
    Timer maUpdateTimer;
    std::vector<std::chrono::high_resolution_clock::time_point> mTimePoints;
    static constexpr unsigned char gnNumberOfTests = 11;
    unsigned char mnTest;
    bool mbAnimate;
    ScopedVclPtr<VirtualDevice> mpVDev;

public:
    VisualBackendTestWindow()
        : WorkWindow(nullptr, WB_APP | WB_STDWORK)
        , mnTest(10 * gnNumberOfTests)
        , mbAnimate(mnTest % gnNumberOfTests == gnNumberOfTests - 1)
        , mpVDev(VclPtr<VirtualDevice>::Create())
    {
        maUpdateTimer.SetInvokeHandler(LINK(this, VisualBackendTestWindow, updateHdl));
        maUpdateTimer.SetPriority(TaskPriority::DEFAULT_IDLE);
        if (mbAnimate)
        {
            maUpdateTimer.SetTimeout(1000.0);
            maUpdateTimer.Start();
        }
    }

    virtual ~VisualBackendTestWindow() override
    {
        disposeOnce();
    }

    DECL_LINK(updateHdl, Timer*, void);

    virtual void KeyInput(const KeyEvent& rKEvt) override
    {
        sal_uInt16 nCode = rKEvt.GetKeyCode().GetCode();

        if (nCode == KEY_BACKSPACE)
            mnTest--;
        else if(nCode == KEY_SPACE)
            mnTest++;

        if (nCode != KEY_BACKSPACE && nCode != KEY_SPACE)
            return;

        if (mnTest % gnNumberOfTests == gnNumberOfTests - 1)
        {
            mbAnimate = true;
            maUpdateTimer.Start();
        }
        else
        {
            mbAnimate = false;
            Invalidate();
        }
    }

    static std::vector<tools::Rectangle> setupRegions(int nPartitionsX, int nPartitionsY, int nWidth, int nHeight)
    {
        std::vector<tools::Rectangle> aRegions;

        for (int y = 0; y < nPartitionsY; y++)
        {
            for (int x = 0; x < nPartitionsX; x++)
            {
                tools::Long x1 =  x    * (nWidth  / nPartitionsX);
                tools::Long y1 =  y    * (nHeight / nPartitionsY);
                tools::Long x2 = (x+1) * (nWidth  / nPartitionsX);
                tools::Long y2 = (y+1) * (nHeight / nPartitionsY);

                aRegions.emplace_back(x1 + 1, y1 + 1, x2 - 6, y2 - 2);
            }
        }
        return aRegions;
    }

    static void testRectangles(vcl::RenderContext& rRenderContext, int nWidth, int nHeight, bool AA)
    {
        tools::Rectangle aRectangle;
        size_t index = 0;

        std::vector<tools::Rectangle> aRegions = setupRegions(4, 2, nWidth, nHeight);

        aRectangle = aRegions[index++];
        {
            vcl::test::OutputDeviceTestRect aOutDevTest;
            Bitmap aBitmap = aOutDevTest.setupRectangle(AA);
            assertAndSetBackground(vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap), aRectangle, rRenderContext);
            drawBitmapScaledAndCentered(aRectangle, aBitmap, rRenderContext);
        }

        aRectangle = aRegions[index++];
        {
            vcl::test::OutputDeviceTestPixel aOutDevTest;
            Bitmap aBitmap = aOutDevTest.setupRectangle(AA);
            assertAndSetBackground(vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap), aRectangle, rRenderContext);
            drawBitmapScaledAndCentered(aRectangle, aBitmap, rRenderContext);
        }

        aRectangle = aRegions[index++];
        {
            vcl::test::OutputDeviceTestLine aOutDevTest;
            Bitmap aBitmap = aOutDevTest.setupRectangle(AA);
            assertAndSetBackground(vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap), aRectangle, rRenderContext);
            drawBitmapScaledAndCentered(aRectangle, aBitmap, rRenderContext);
        }

        aRectangle = aRegions[index++];
        {
            vcl::test::OutputDeviceTestPolygon aOutDevTest;
            Bitmap aBitmap = aOutDevTest.setupRectangle(AA);
            assertAndSetBackground(vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap), aRectangle, rRenderContext);
            drawBitmapScaledAndCentered(aRectangle, aBitmap, rRenderContext);
        }

        aRectangle = aRegions[index++];
        {
            vcl::test::OutputDeviceTestPolyLine aOutDevTest;
            Bitmap aBitmap = aOutDevTest.setupRectangle(AA);
            assertAndSetBackground(vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap), aRectangle, rRenderContext);
            drawBitmapScaledAndCentered(aRectangle, aBitmap, rRenderContext);
        }

        aRectangle = aRegions[index++];
        {
            vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
            Bitmap aBitmap = aOutDevTest.setupRectangle(AA);
            assertAndSetBackground(vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap), aRectangle, rRenderContext);
            drawBitmapScaledAndCentered(aRectangle, aBitmap, rRenderContext);
        }

        aRectangle = aRegions[index++];
        {
            vcl::test::OutputDeviceTestPolyPolygon aOutDevTest;
            Bitmap aBitmap = aOutDevTest.setupRectangle(AA);
            assertAndSetBackground(vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap), aRectangle, rRenderContext);
            drawBitmapScaledAndCentered(aRectangle, aBitmap, rRenderContext);
        }

        aRectangle = aRegions[index++];
        {
            vcl::test::OutputDeviceTestPolyPolygonB2D aOutDevTest;
            Bitmap aBitmap = aOutDevTest.setupRectangle(AA);
            assertAndSetBackground(vcl::test::OutputDeviceTestCommon::checkRectangle(aBitmap), aRectangle, rRenderContext);
            drawBitmapScaledAndCentered(aRectangle, aBitmap, rRenderContext);
        }
    }

    static void testFilledRectangles(vcl::RenderContext& rRenderContext, int nWidth, int nHeight)
    {
        tools::Rectangle aRectangle;
        size_t index = 0;

        std::vector<tools::Rectangle> aRegions = setupRegions(4, 2, nWidth, nHeight);

        aRectangle = aRegions[index++];
        {
            vcl::test::OutputDeviceTestRect aOutDevTest;
            Bitmap aBitmap = aOutDevTest.setupFilledRectangle(false);
            assertAndSetBackground(vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, false), aRectangle, rRenderContext);
            drawBitmapScaledAndCentered(aRectangle, aBitmap, rRenderContext);
        }

        aRectangle = aRegions[index++];
        {
            vcl::test::OutputDeviceTestPolygon aOutDevTest;
            Bitmap aBitmap = aOutDevTest.setupFilledRectangle(false);
            assertAndSetBackground(vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, false), aRectangle, rRenderContext);
            drawBitmapScaledAndCentered(aRectangle, aBitmap, rRenderContext);
        }

        aRectangle = aRegions[index++];
        {
            vcl::test::OutputDeviceTestPolyPolygon aOutDevTest;
            Bitmap aBitmap = aOutDevTest.setupFilledRectangle(false);
            assertAndSetBackground(vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, false), aRectangle, rRenderContext);
            drawBitmapScaledAndCentered(aRectangle, aBitmap, rRenderContext);
        }

        aRectangle = aRegions[index++];
        {
            vcl::test::OutputDeviceTestPolyPolygonB2D aOutDevTest;
            Bitmap aBitmap = aOutDevTest.setupFilledRectangle(false);
            assertAndSetBackground(vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, false), aRectangle, rRenderContext);
            drawBitmapScaledAndCentered(aRectangle, aBitmap, rRenderContext);
        }

        aRectangle = aRegions[index++];
        {
            vcl::test::OutputDeviceTestRect aOutDevTest;
            Bitmap aBitmap = aOutDevTest.setupFilledRectangle(true);
            assertAndSetBackground(vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, true), aRectangle, rRenderContext);
            drawBitmapScaledAndCentered(aRectangle, aBitmap, rRenderContext);
        }

        aRectangle = aRegions[index++];
        {
            vcl::test::OutputDeviceTestPolygon aOutDevTest;
            Bitmap aBitmap = aOutDevTest.setupFilledRectangle(true);
            assertAndSetBackground(vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, true), aRectangle, rRenderContext);
            drawBitmapScaledAndCentered(aRectangle, aBitmap, rRenderContext);
        }

        aRectangle = aRegions[index++];
        {
            vcl::test::OutputDeviceTestPolyPolygon aOutDevTest;
            Bitmap aBitmap = aOutDevTest.setupFilledRectangle(true);
            assertAndSetBackground(vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, true), aRectangle, rRenderContext);
            drawBitmapScaledAndCentered(aRectangle, aBitmap, rRenderContext);
        }

        aRectangle = aRegions[index++];
        {
            vcl::test::OutputDeviceTestPolyPolygonB2D aOutDevTest;
            Bitmap aBitmap = aOutDevTest.setupFilledRectangle(true);
            assertAndSetBackground(vcl::test::OutputDeviceTestCommon::checkFilledRectangle(aBitmap, true), aRectangle, rRenderContext);
            drawBitmapScaledAndCentered(aRectangle, aBitmap, rRenderContext);
        }
    }

    static void testDiamondsAndBezier(vcl::RenderContext& rRenderContext, int nWidth, int nHeight)
    {
        tools::Rectangle aRectangle;
        size_t index = 0;

        std::vector<tools::Rectangle> aRegions = setupRegions(3, 2, nWidth, nHeight);

        aRectangle = aRegions[index++];
        {
            vcl::test::OutputDeviceTestPolygon aOutDevTest;
            Bitmap aBitmap = aOutDevTest.setupDiamond();
            assertAndSetBackground(vcl::test::OutputDeviceTestCommon::checkDiamond(aBitmap), aRectangle, rRenderContext);
            drawBitmapScaledAndCentered(aRectangle, aBitmap, rRenderContext);
        }
        aRectangle = aRegions[index++];
        {
            vcl::test::OutputDeviceTestLine aOutDevTest;
            Bitmap aBitmap = aOutDevTest.setupDiamond();
            assertAndSetBackground(vcl::test::OutputDeviceTestCommon::checkDiamond(aBitmap), aRectangle, rRenderContext);
            drawBitmapScaledAndCentered(aRectangle, aBitmap, rRenderContext);
        }
        aRectangle = aRegions[index++];
        {
            vcl::test::OutputDeviceTestPolyLine aOutDevTest;
            Bitmap aBitmap = aOutDevTest.setupDiamond();
            assertAndSetBackground(vcl::test::OutputDeviceTestCommon::checkDiamond(aBitmap), aRectangle, rRenderContext);
            drawBitmapScaledAndCentered(aRectangle, aBitmap, rRenderContext);
        }
        aRectangle = aRegions[index++];
        {
            vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
            Bitmap aBitmap = aOutDevTest.setupDiamond();
            assertAndSetBackground(vcl::test::OutputDeviceTestCommon::checkDiamond(aBitmap), aRectangle, rRenderContext);
            drawBitmapScaledAndCentered(aRectangle, aBitmap, rRenderContext);
        }

        aRectangle = aRegions[index++];
        {
            vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
            Bitmap aBitmap = aOutDevTest.setupBezier();
            assertAndSetBackground(vcl::test::OutputDeviceTestCommon::checkBezier(aBitmap), aRectangle, rRenderContext);
            drawBitmapScaledAndCentered(aRectangle, aBitmap, rRenderContext);
        }
        aRectangle = aRegions[index++];
        {
            vcl::test::OutputDeviceTestPolyLineB2D aOutDevTest;
            Bitmap aBitmap = aOutDevTest.setupAABezier();
            assertAndSetBackground(vcl::test::OutputDeviceTestCommon::checkBezier(aBitmap), aRectangle, rRenderContext);
            drawBitmapScaledAndCentered(aRectangle, aBitmap, rRenderContext);
        }
    }

    static void testLines(vcl::RenderContext& rRenderContext, int nWidth, int nHeight)
    {
        tools::Rectangle aRectangle;
        size_t index = 0;

        std::vector<tools::Rectangle> aRegions = setupRegions(3, 2, nWidth, nHeight);

        aRectangle = aRegions[index++];
        {
            vcl::test::OutputDeviceTestLine aOutDevTest;
            Bitmap aBitmap = aOutDevTest.setupLines();
            assertAndSetBackground(vcl::test::OutputDeviceTestCommon::checkLines(aBitmap), aRectangle, rRenderContext);
            drawBitmapScaledAndCentered(aRectangle, aBitmap, rRenderContext);
        }
        aRectangle = aRegions[index++];
        {
            vcl::test::OutputDeviceTestPolyLine aOutDevTest;
            Bitmap aBitmap = aOutDevTest.setupLines();
            assertAndSetBackground(vcl::test::OutputDeviceTestCommon::checkLines(aBitmap), aRectangle, rRenderContext);
            drawBitmapScaledAndCentered(aRectangle, aBitmap, rRenderContext);
        }
        aRectangle = aRegions[index++];
        {
            vcl::test::OutputDeviceTestPolygon aOutDevTest;
            Bitmap aBitmap = aOutDevTest.setupLines();
            assertAndSetBackground(vcl::test::OutputDeviceTestCommon::checkLines(aBitmap), aRectangle, rRenderContext);
            drawBitmapScaledAndCentered(aRectangle, aBitmap, rRenderContext);
        }
        aRectangle = aRegions[index++];
        {
            vcl::test::OutputDeviceTestLine aOutDevTest;
            Bitmap aBitmap = aOutDevTest.setupAALines();
            assertAndSetBackground(vcl::test::OutputDeviceTestCommon::checkAALines(aBitmap), aRectangle, rRenderContext);
            drawBitmapScaledAndCentered(aRectangle, aBitmap, rRenderContext);
        }
        aRectangle = aRegions[index++];
        {
            vcl::test::OutputDeviceTestPolyLine aOutDevTest;
            Bitmap aBitmap = aOutDevTest.setupAALines();
            assertAndSetBackground(vcl::test::OutputDeviceTestCommon::checkAALines(aBitmap), aRectangle, rRenderContext);
            drawBitmapScaledAndCentered(aRectangle, aBitmap, rRenderContext);
        }
        aRectangle = aRegions[index++];
        {
            vcl::test::OutputDeviceTestPolygon aOutDevTest;
            Bitmap aBitmap = aOutDevTest.setupAALines();
            assertAndSetBackground(vcl::test::OutputDeviceTestCommon::checkAALines(aBitmap), aRectangle, rRenderContext);
            drawBitmapScaledAndCentered(aRectangle, aBitmap, rRenderContext);
        }
    }

    static void testBitmaps(vcl::RenderContext& rRenderContext, int nWidth, int nHeight)
    {
        tools::Rectangle aRectangle;
        size_t index = 0;

        std::vector<tools::Rectangle> aRegions = setupRegions(3, 2, nWidth, nHeight);

        aRectangle = aRegions[index++];
        {
            vcl::test::OutputDeviceTestBitmap aOutDevTest;
            Bitmap aBitmap = aOutDevTest.setupDrawBitmap();
            assertAndSetBackground(vcl::test::OutputDeviceTestBitmap::checkTransformedBitmap(aBitmap), aRectangle, rRenderContext);
            drawBitmapScaledAndCentered(aRectangle, aBitmap, rRenderContext);
        }
        aRectangle = aRegions[index++];
        {
            vcl::test::OutputDeviceTestBitmap aOutDevTest;
            Bitmap aBitmap = aOutDevTest.setupDrawTransformedBitmap();
            assertAndSetBackground(vcl::test::OutputDeviceTestBitmap::checkTransformedBitmap(aBitmap), aRectangle, rRenderContext);
            drawBitmapScaledAndCentered(aRectangle, aBitmap, rRenderContext);
        }
        aRectangle = aRegions[index++];
        {
            vcl::test::OutputDeviceTestBitmap aOutDevTest;
            Bitmap aBitmap = aOutDevTest.setupDrawBitmapExWithAlpha();
            assertAndSetBackground(vcl::test::OutputDeviceTestBitmap::checkBitmapExWithAlpha(aBitmap), aRectangle, rRenderContext);
            drawBitmapScaledAndCentered(aRectangle, aBitmap, rRenderContext);
        }
        aRectangle = aRegions[index++];
        {
            vcl::test::OutputDeviceTestBitmap aOutDevTest;
            Bitmap aBitmap = aOutDevTest.setupDrawMask();
            assertAndSetBackground(vcl::test::OutputDeviceTestBitmap::checkMask(aBitmap), aRectangle, rRenderContext);
            drawBitmapScaledAndCentered(aRectangle, aBitmap, rRenderContext);
        }
        aRectangle = aRegions[index++];
        {
            vcl::test::OutputDeviceTestBitmap aOutDevTest;
            BitmapEx aBitmap = aOutDevTest.setupDrawBlend();
            assertAndSetBackground(vcl::test::OutputDeviceTestBitmap::checkBlend(aBitmap), aRectangle, rRenderContext);
            drawBitmapScaledAndCentered(aRectangle, aBitmap.GetBitmap(), rRenderContext);
        }
    }

    static void testInvert(vcl::RenderContext& rRenderContext, int nWidth, int nHeight)
    {
        tools::Rectangle aRectangle;
        size_t index = 0;

        std::vector<tools::Rectangle> aRegions = setupRegions(2, 2, nWidth, nHeight);

        aRectangle = aRegions[index++];
        {
            vcl::test::OutputDeviceTestRect aOutDevTest;
            Bitmap aBitmap = aOutDevTest.setupInvert_NONE();
            assertAndSetBackground(vcl::test::OutputDeviceTestRect::checkInvertRectangle(aBitmap), aRectangle, rRenderContext);
            drawBitmapScaledAndCentered(aRectangle, aBitmap, rRenderContext);
        }
        aRectangle = aRegions[index++];
        {
            vcl::test::OutputDeviceTestRect aOutDevTest;
            Bitmap aBitmap = aOutDevTest.setupInvert_N50();
            assertAndSetBackground(vcl::test::OutputDeviceTestRect::checkInvertN50Rectangle(aBitmap), aRectangle, rRenderContext);
            drawBitmapScaledAndCentered(aRectangle, aBitmap, rRenderContext);
        }
        aRectangle = aRegions[index++];
        {
            vcl::test::OutputDeviceTestRect aOutDevTest;
            Bitmap aBitmap = aOutDevTest.setupInvert_TrackFrame();
            assertAndSetBackground(vcl::test::OutputDeviceTestRect::checkInvertTrackFrameRectangle(aBitmap), aRectangle, rRenderContext);
            drawBitmapScaledAndCentered(aRectangle, aBitmap, rRenderContext);
        }
        aRectangle = aRegions[index++];
        {
            vcl::test::OutputDeviceTestAnotherOutDev aOutDevTest;
            Bitmap aBitmap = aOutDevTest.setupXOR();
            assertAndSetBackground(vcl::test::OutputDeviceTestAnotherOutDev::checkXOR(aBitmap), aRectangle, rRenderContext);
            drawBitmapScaledAndCentered(aRectangle, aBitmap, rRenderContext);
        }
    }

    static void testClip(vcl::RenderContext& rRenderContext, int nWidth, int nHeight)
    {
        tools::Rectangle aRectangle;
        size_t index = 0;

        std::vector<tools::Rectangle> aRegions = setupRegions(2, 2, nWidth, nHeight);

        aRectangle = aRegions[index++];
        {
            vcl::test::OutputDeviceTestClip aOutDevTest;
            Bitmap aBitmap = aOutDevTest.setupClipRectangle();
            assertAndSetBackground(vcl::test::OutputDeviceTestClip::checkClip(aBitmap), aRectangle, rRenderContext);
            drawBitmapScaledAndCentered(aRectangle, aBitmap, rRenderContext);
        }
        aRectangle = aRegions[index++];
        {
            vcl::test::OutputDeviceTestClip aOutDevTest;
            Bitmap aBitmap = aOutDevTest.setupClipPolygon();
            assertAndSetBackground(vcl::test::OutputDeviceTestClip::checkClip(aBitmap), aRectangle, rRenderContext);
            drawBitmapScaledAndCentered(aRectangle, aBitmap, rRenderContext);
        }
        aRectangle = aRegions[index++];
        {
            vcl::test::OutputDeviceTestClip aOutDevTest;
            Bitmap aBitmap = aOutDevTest.setupClipPolyPolygon();
            assertAndSetBackground(vcl::test::OutputDeviceTestClip::checkClip(aBitmap), aRectangle, rRenderContext);
            drawBitmapScaledAndCentered(aRectangle, aBitmap, rRenderContext);
        }
        aRectangle = aRegions[index++];
        {
            vcl::test::OutputDeviceTestClip aOutDevTest;
            Bitmap aBitmap = aOutDevTest.setupClipB2DPolyPolygon();
            assertAndSetBackground(vcl::test::OutputDeviceTestClip::checkClip(aBitmap), aRectangle, rRenderContext);
            drawBitmapScaledAndCentered(aRectangle, aBitmap, rRenderContext);
        }
    }

    static void testGradients(vcl::RenderContext& rRenderContext, int nWidth, int nHeight)
    {
        tools::Rectangle aRectangle;
        size_t index = 0;

        std::vector<tools::Rectangle> aRegions = setupRegions(4, 2, nWidth, nHeight);

        aRectangle = aRegions[index++];
        {
            vcl::test::OutputDeviceTestGradient aOutDevTest;
            Bitmap aBitmap = aOutDevTest.setupLinearGradient();
            assertAndSetBackground(vcl::test::OutputDeviceTestGradient::checkLinearGradient(aBitmap), aRectangle, rRenderContext);
            drawBitmapScaledAndCentered(aRectangle, aBitmap, rRenderContext);
        }
        aRectangle = aRegions[index++];
        {
            vcl::test::OutputDeviceTestGradient aOutDevTest;
            Bitmap aBitmap = aOutDevTest.setupLinearGradientAngled();
            assertAndSetBackground(vcl::test::OutputDeviceTestGradient::checkLinearGradientAngled(aBitmap), aRectangle, rRenderContext);
            drawBitmapScaledAndCentered(aRectangle, aBitmap, rRenderContext);
        }
        aRectangle = aRegions[index++];
        {
            vcl::test::OutputDeviceTestGradient aOutDevTest;
            Bitmap aBitmap = aOutDevTest.setupLinearGradientBorder();
            assertAndSetBackground(vcl::test::OutputDeviceTestGradient::checkLinearGradientBorder(aBitmap), aRectangle, rRenderContext);
            drawBitmapScaledAndCentered(aRectangle, aBitmap, rRenderContext);
        }
        aRectangle = aRegions[index++];
        {
            vcl::test::OutputDeviceTestGradient aOutDevTest;
            Bitmap aBitmap = aOutDevTest.setupLinearGradientIntensity();
            assertAndSetBackground(vcl::test::OutputDeviceTestGradient::checkLinearGradientIntensity(aBitmap), aRectangle, rRenderContext);
            drawBitmapScaledAndCentered(aRectangle, aBitmap, rRenderContext);
        }
        aRectangle = aRegions[index++];
        {
            vcl::test::OutputDeviceTestGradient aOutDevTest;
            Bitmap aBitmap = aOutDevTest.setupLinearGradientSteps();
            assertAndSetBackground(vcl::test::OutputDeviceTestGradient::checkLinearGradientSteps(aBitmap), aRectangle, rRenderContext);
            drawBitmapScaledAndCentered(aRectangle, aBitmap, rRenderContext);
        }
        aRectangle = aRegions[index++];
        {
            vcl::test::OutputDeviceTestGradient aOutDevTest;
            Bitmap aBitmap = aOutDevTest.setupAxialGradient();
            assertAndSetBackground(vcl::test::OutputDeviceTestGradient::checkAxialGradient(aBitmap), aRectangle, rRenderContext);
            drawBitmapScaledAndCentered(aRectangle, aBitmap, rRenderContext);
        }
        aRectangle = aRegions[index++];
        {
            vcl::test::OutputDeviceTestGradient aOutDevTest;
            Bitmap aBitmap = aOutDevTest.setupRadialGradient();
            assertAndSetBackground(vcl::test::OutputDeviceTestGradient::checkRadialGradient(aBitmap), aRectangle, rRenderContext);
            drawBitmapScaledAndCentered(aRectangle, aBitmap, rRenderContext);
        }
        aRectangle = aRegions[index++];
        {
            vcl::test::OutputDeviceTestGradient aOutDevTest;
            Bitmap aBitmap = aOutDevTest.setupRadialGradientOfs();
            assertAndSetBackground(vcl::test::OutputDeviceTestGradient::checkRadialGradientOfs(aBitmap), aRectangle, rRenderContext);
            drawBitmapScaledAndCentered(aRectangle, aBitmap, rRenderContext);
        }
    }

    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& /*rRect*/) override
    {
        if (mnTest % gnNumberOfTests == gnNumberOfTests - 1)
        {
            rRenderContext.SetBackground(Wallpaper(COL_GREEN));

            static size_t nTimeIndex = 0;
            static const size_t constSamplesFPS = 120;
            double fps = 0.0;

            if (mTimePoints.size() < constSamplesFPS)
            {
                mTimePoints.push_back(std::chrono::high_resolution_clock::now());
                nTimeIndex++;
            }
            else
            {
                size_t current = nTimeIndex % constSamplesFPS;
                mTimePoints[current] = std::chrono::high_resolution_clock::now();
                size_t last = (nTimeIndex + 1) % constSamplesFPS;
                auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(mTimePoints[current] - mTimePoints[last]).count();
                fps = constSamplesFPS * 1000.0 / ms;
                nTimeIndex++;
            }

            double fTime = 0.5 + std::sin(nTimeIndex / 100.0) / 2.0;

            Size aSizePixel = GetSizePixel();

            mpVDev->SetAntialiasing(AntialiasingFlags::Enable | AntialiasingFlags::PixelSnapHairline);
            mpVDev->SetOutputSizePixel(aSizePixel);
            mpVDev->SetBackground(Wallpaper(COL_LIGHTGRAY));
            mpVDev->Erase();
            mpVDev->SetFillColor(COL_LIGHTRED);
            mpVDev->SetLineColor(COL_LIGHTBLUE);

            basegfx::B2DPolyPolygon polyPolygon;

            for (int b=10; b<14; b++)
            {
                basegfx::B2DPolygon polygon;
                for (double a=0.0; a<360.0; a+=0.5)
                {
                    double x = std::sin(basegfx::deg2rad(a)) * (b+1) * 20;
                    double y = std::cos(basegfx::deg2rad(a)) * (b+1) * 20;
                    polygon.append(basegfx::B2DPoint(x + 200 + 500 * fTime, y + 200 + 500 * fTime));
                }
                polygon.setClosed(true);
                polyPolygon.append(polygon);
            }

            mpVDev->DrawPolyPolygon(polyPolygon);

            tools::Rectangle aGradientRect(Point(200, 200), Size(200 + fTime * 300, 200 + fTime * 300));
            mpVDev->DrawGradient(aGradientRect, Gradient(GradientStyle::Linear, COL_YELLOW, COL_BLUE));

            rRenderContext.DrawOutDev(Point(), mpVDev->GetOutputSizePixel(),
                                      Point(), mpVDev->GetOutputSizePixel(),
                                      *mpVDev);
            rRenderContext.SetTextColor(COL_LIGHTRED);
            rRenderContext.DrawText(Point(10, 10), "FPS: " + OUString::number(int(fps)));
            return;
        }

        rRenderContext.SetBackground(Wallpaper(COL_GREEN));

        Size aSize = GetOutputSizePixel();

        tools::Long nWidth = aSize.Width();
        tools::Long nHeight = aSize.Height();

        if (mnTest % gnNumberOfTests == 0)
        {
            testRectangles(rRenderContext, nWidth, nHeight, false);
        }
        else if (mnTest % gnNumberOfTests == 1)
        {
            testRectangles(rRenderContext, nWidth, nHeight, true);
        }
        else if (mnTest % gnNumberOfTests == 2)
        {
            testFilledRectangles(rRenderContext, nWidth, nHeight);
        }
        else if (mnTest % gnNumberOfTests == 3)
        {
            testDiamondsAndBezier(rRenderContext, nWidth, nHeight);
        }
        else if (mnTest % gnNumberOfTests == 4)
        {
            testLines(rRenderContext, nWidth, nHeight);
        }
        else if (mnTest % gnNumberOfTests == 5)
        {
            testBitmaps(rRenderContext, nWidth, nHeight);
        }
        else if (mnTest % gnNumberOfTests == 6)
        {
            testInvert(rRenderContext, nWidth, nHeight);
        }
        else if (mnTest % gnNumberOfTests == 7)
        {
            testClip(rRenderContext, nWidth, nHeight);
        }
        else if (mnTest % gnNumberOfTests == 8)
        {
            testGradients(rRenderContext, nWidth, nHeight);
        }
        else if (mnTest % gnNumberOfTests == 9)
        {
            std::vector<tools::Rectangle> aRegions = setupRegions(2, 1, nWidth, nHeight);
            size_t index = 0;

            tools::Rectangle aRectangle = aRegions[index++];
            {
                vcl::test::OutputDeviceTestAnotherOutDev aOutDevTest;
                Bitmap aBitmap = aOutDevTest.setupDrawOutDev();
                assertAndSetBackground(vcl::test::OutputDeviceTestAnotherOutDev::checkDrawOutDev(aBitmap), aRectangle, rRenderContext);
                drawBitmapScaledAndCentered(aRectangle, aBitmap, rRenderContext);
            }
            aRectangle = aRegions[index++];
            {
                vcl::test::OutputDeviceTestLine aOutDevTest;
                Bitmap aBitmap = aOutDevTest.setupDashedLine();
                assertAndSetBackground(vcl::test::OutputDeviceTestLine::checkDashedLine(aBitmap), aRectangle, rRenderContext);
                drawBitmapScaledAndCentered(aRectangle, aBitmap, rRenderContext);
            }
        }
    }
};

}

IMPL_LINK_NOARG(VisualBackendTestWindow, updateHdl, Timer *, void)
{
    if (mbAnimate)
    {
        maUpdateTimer.SetTimeout(1.0);
        maUpdateTimer.Start();
        Invalidate();
    }
}

namespace {

class VisualBackendTestApp : public Application
{

public:
    VisualBackendTestApp()
    {}

    virtual int Main() override
    {
        try
        {
            ScopedVclPtrInstance<VisualBackendTestWindow> aMainWindow;

            aMainWindow->SetText("VCL Test");
            aMainWindow->Show();

            Application::Execute();
        }
        catch (const css::uno::Exception&)
        {
            DBG_UNHANDLED_EXCEPTION("vcl.app", "Fatal");
            return 1;
        }
        catch (const std::exception& rException)
        {
            SAL_WARN("vcl.app", "Fatal exception: " << rException.what());
            return 1;
        }
        return 0;
    }

protected:
    void Init() override
    {
        try
        {
            uno::Reference<uno::XComponentContext> xComponentContext = ::cppu::defaultBootstrap_InitialComponentContext();
            uno::Reference<lang::XMultiServiceFactory> xMSF(xComponentContext->getServiceManager(), uno::UNO_QUERY);

            if (!xMSF.is())
                Application::Abort("Bootstrap failure - no service manager");

            comphelper::setProcessServiceFactory(xMSF);
        }
        catch (const uno::Exception &e)
        {
            Application::Abort("Bootstrap exception " + e.Message);
        }
    }

    void DeInit() override
    {
        comphelper::setProcessServiceFactory(nullptr);
    }
};

}

void vclmain::createApplication()
{
    static VisualBackendTestApp aApplication;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
