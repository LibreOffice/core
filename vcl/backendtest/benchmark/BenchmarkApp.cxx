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

#include <iostream>

#include <test/Benchmarks.hxx>

using namespace css;

static void drawBitmapCentered(tools::Rectangle const& rRect, const Bitmap& aBitmap,
                               vcl::RenderContext& rRenderContext)
{
    tools::Long nWidth = rRect.GetWidth();
    tools::Long nHeight = rRect.GetHeight();

    Size aBitmapSize(aBitmap.GetSizePixel());

    Point aPoint(rRect.TopLeft());

    aPoint.AdjustX((nWidth - aBitmapSize.Width()) / 2);
    aPoint.AdjustY((nHeight - aBitmapSize.Height()) / 2);

    rRenderContext.DrawBitmap(aPoint, aBitmap);
}

static void drawBitmapScaledAndCentered(tools::Rectangle const& rRect, Bitmap aBitmap,
                                        vcl::RenderContext& rRenderContext,
                                        BmpScaleFlag aFlag = BmpScaleFlag::Fast)
{
    tools::Long nWidth = rRect.GetWidth();
    tools::Long nHeight = rRect.GetHeight();

    Size aBitmapSize(aBitmap.GetSizePixel());

    double fWidthHeight = std::min(nWidth, nHeight);
    double fScale = fWidthHeight / aBitmapSize.Width();
    aBitmap.Scale(fScale, fScale, aFlag);

    drawBitmapCentered(rRect, aBitmap, rRenderContext);
}

namespace
{
class BenchMarkWindow : public WorkWindow
{
private:
    static constexpr unsigned char gnNumberOfTests = 4;
    unsigned char mnTest;
    ScopedVclPtr<VirtualDevice> mpVDev;

public:
    BenchMarkWindow()
        : WorkWindow(nullptr, WB_APP | WB_STDWORK)
        , mnTest(10 * gnNumberOfTests)
        , mpVDev(VclPtr<VirtualDevice>::Create())
    {
    }

    virtual ~BenchMarkWindow() override { disposeOnce(); }

    virtual void KeyInput(const KeyEvent& rKEvt) override
    {
        sal_uInt16 nCode = rKEvt.GetKeyCode().GetCode();

        if (nCode == KEY_BACKSPACE)
            mnTest--;
        else if (nCode == KEY_SPACE)
            mnTest++;

        if (nCode != KEY_BACKSPACE && nCode != KEY_SPACE)
            return;

        Invalidate();
    }

    static void updateResults(vcl::RenderContext& rRenderContext, sal_Int64 aTimeElapsed)
    {
        rRenderContext.SetTextColor(COL_YELLOW);
        rRenderContext.DrawText(Point(10, 10), "Time Taken to Render: "
                                                   + OUString::number(float(aTimeElapsed)) + " ms");
    }

    static std::vector<tools::Rectangle> setupRegions(int nPartitionsX, int nPartitionsY,
                                                      int nWidth, int nHeight)
    {
        std::vector<tools::Rectangle> aRegions;

        for (int y = 0; y < nPartitionsY; y++)
        {
            for (int x = 0; x < nPartitionsX; x++)
            {
                tools::Long x1 = x * (nWidth / nPartitionsX);
                tools::Long y1 = y * (nHeight / nPartitionsY);
                tools::Long x2 = (x + 1) * (nWidth / nPartitionsX);
                tools::Long y2 = (y + 1) * (nHeight / nPartitionsY);

                aRegions.emplace_back(x1 + 1, y1 + 1, x2 - 6, y2 - 2);
            }
        }
        return aRegions;
    }

    static void drawMultiplePolygonsWithPolyPolygon(vcl::RenderContext& rRenderContext, int nWidth,
                                                    int nHeight)
    {
        tools::Rectangle aRectangle;
        size_t index = 0;

        std::vector<tools::Rectangle> aRegions = setupRegions(1, 1, nWidth, nHeight);

        aRectangle = aRegions[index++];
        Benchmark aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupMultiplePolygonsWithPolyPolygon();
        drawBitmapScaledAndCentered(aRectangle, aBitmap, rRenderContext);
        updateResults(rRenderContext, aOutDevTest.getElapsedTime());
    }

    static void drawWavelines(vcl::RenderContext& rRenderContext, int nWidth, int nHeight)
    {
        tools::Rectangle aRectangle;
        size_t index = 0;

        std::vector<tools::Rectangle> aRegions = setupRegions(1, 1, nWidth, nHeight);
        aRectangle = aRegions[index++];
        Benchmark aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupWavelines();
        drawBitmapScaledAndCentered(aRectangle, aBitmap, rRenderContext);
        updateResults(rRenderContext, aOutDevTest.getElapsedTime());
    }

    static void drawGrid(vcl::RenderContext& rRenderContext, int nWidth, int nHeight)
    {
        tools::Rectangle aRectangle;
        size_t index = 0;

        std::vector<tools::Rectangle> aRegions = setupRegions(1, 1, nWidth, nHeight);
        aRectangle = aRegions[index++];
        Benchmark aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupGrid();
        drawBitmapScaledAndCentered(aRectangle, aBitmap, rRenderContext);
        updateResults(rRenderContext, aOutDevTest.getElapsedTime());
    }

    static void drawGridWithDottedLines(vcl::RenderContext& rRenderContext, int nWidth, int nHeight)
    {
        tools::Rectangle aRectangle;
        size_t index = 0;

        std::vector<tools::Rectangle> aRegions = setupRegions(1, 1, nWidth, nHeight);
        aRectangle = aRegions[index++];
        Benchmark aOutDevTest;
        Bitmap aBitmap = aOutDevTest.setupGridWithDottedLine();
        drawBitmapScaledAndCentered(aRectangle, aBitmap, rRenderContext);
        updateResults(rRenderContext, aOutDevTest.getElapsedTime());
    }

    virtual void Paint(vcl::RenderContext& rRenderContext,
                       const tools::Rectangle& /*rRect*/) override
    {
        rRenderContext.SetBackground(Wallpaper(Color(0x36, 0x45, 0X4F)));

        Size aSize = GetOutputSizePixel();

        tools::Long nWidth = aSize.Width();
        tools::Long nHeight = aSize.Height();

        if (mnTest % gnNumberOfTests == 0)
        {
            drawMultiplePolygonsWithPolyPolygon(rRenderContext, nWidth, nHeight);
        }
        else if (mnTest % gnNumberOfTests == 1)
        {
            drawWavelines(rRenderContext, nWidth, nHeight);
        }
        else if (mnTest % gnNumberOfTests == 2)
        {
            drawGrid(rRenderContext, nWidth, nHeight);
        }
        else if (mnTest % gnNumberOfTests == 3)
        {
            drawGridWithDottedLines(rRenderContext, nWidth, nHeight);
        }
    }
};
}

namespace
{
class BenchMarkApp : public Application
{
public:
    BenchMarkApp() {}

    virtual int Main() override
    {
        try
        {
            ScopedVclPtrInstance<BenchMarkWindow> aMainWindow;

            aMainWindow->SetText("Benchmarks");
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
            uno::Reference<uno::XComponentContext> xComponentContext
                = ::cppu::defaultBootstrap_InitialComponentContext();
            uno::Reference<lang::XMultiServiceFactory> xMSF(xComponentContext->getServiceManager(),
                                                            uno::UNO_QUERY);

            if (!xMSF.is())
                Application::Abort("Bootstrap failure - no service manager");

            comphelper::setProcessServiceFactory(xMSF);
        }
        catch (const uno::Exception& e)
        {
            Application::Abort("Bootstrap exception " + e.Message);
        }
    }

    void DeInit() override { comphelper::setProcessServiceFactory(nullptr); }
};
}

void vclmain::createApplication() { static BenchMarkApp aApplication; }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
