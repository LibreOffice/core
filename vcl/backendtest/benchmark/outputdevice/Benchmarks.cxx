/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <test/Benchmarks.hxx>
#include <basegfx/polygon/WaveLine.hxx>
#include <vcl/lineinfo.hxx>

const Color Benchmark::constBackgroundColor(COL_LIGHTGRAY);
const Color Benchmark::constLineColor(COL_LIGHTBLUE);
const Color Benchmark::constFillColor(COL_BLUE);

void Benchmark::initialSetup(tools::Long nWidth, tools::Long nHeight, Color aColor)
{
    mpVirtualDevice = VclPtr<VirtualDevice>::Create(DeviceFormat::DEFAULT);
    maVDRectangle = tools::Rectangle(Point(), Size(nWidth, nHeight));
    mpVirtualDevice->SetOutputSizePixel(maVDRectangle.GetSize());
    mpVirtualDevice->SetAntialiasing(AntialiasingFlags::NONE);
    mpVirtualDevice->SetBackground(Wallpaper(aColor));
    mpVirtualDevice->Erase();
}

sal_Int64 Benchmark::getElapsedTime()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(m_xEnd - m_xStart).count();
}

Bitmap Benchmark::setupWavelines()
{
    initialSetup(4096, 4096, constBackgroundColor);

    mpVirtualDevice->SetLineColor(constLineColor);
    mpVirtualDevice->SetFillColor();

    m_xStart = std::chrono::steady_clock::now();
    for (int i = 1; i <= 4094; i += 2)
    {
        const basegfx::B2DRectangle aWaveLineRectangle(1, i, 4096, i + 2);
        const basegfx::B2DPolygon aWaveLinePolygon
            = basegfx::createWaveLinePolygon(aWaveLineRectangle);
        mpVirtualDevice->DrawPolyLine(aWaveLinePolygon);
    }
    Bitmap aBitmap = mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
    m_xEnd = std::chrono::steady_clock::now();
    return aBitmap;
}

Bitmap Benchmark::setupGrid()
{
    initialSetup(4096, 4096, constBackgroundColor);

    mpVirtualDevice->SetLineColor(constLineColor);
    mpVirtualDevice->SetFillColor();

    m_xStart = std::chrono::steady_clock::now();
    for (int i = 1; i <= 4096; i += 2)
    {
        mpVirtualDevice->DrawLine(Point(1, i), Point(4096, i));
        mpVirtualDevice->DrawLine(Point(i, 1), Point(i, 4096));
    }
    Bitmap aBitmap = mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
    m_xEnd = std::chrono::steady_clock::now();
    return aBitmap;
}

Bitmap Benchmark::setupGridWithDottedLine()
{
    initialSetup(4096, 4096, constBackgroundColor);

    mpVirtualDevice->SetLineColor(constLineColor);
    mpVirtualDevice->SetFillColor();

    m_xStart = std::chrono::steady_clock::now();
    for (int i = 1; i <= 4096; i += 2)
    {
        LineInfo aLineInfo(LineStyle::Dash, 1);
        aLineInfo.SetDashCount(10);
        aLineInfo.SetDashLen(1);
        aLineInfo.SetDotCount(10);
        aLineInfo.SetDotLen(1);
        aLineInfo.SetDistance(1);
        aLineInfo.SetLineJoin(basegfx::B2DLineJoin::Bevel);
        aLineInfo.SetLineCap(css::drawing::LineCap_BUTT);
        mpVirtualDevice->DrawLine(Point(1, i), Point(4096, i), aLineInfo);
        mpVirtualDevice->DrawLine(Point(i, 1), Point(i, 4096), aLineInfo);
    }
    Bitmap aBitmap = mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
    m_xEnd = std::chrono::steady_clock::now();
    return aBitmap;
}

Bitmap Benchmark::setupMultiplePolygonsWithPolyPolygon()
{
    initialSetup(4096, 4096, constBackgroundColor);

    mpVirtualDevice->SetLineColor(constLineColor);
    mpVirtualDevice->SetFillColor();

    tools::PolyPolygon aPolyPolygon(4);

    for (int nOffset = 1; nOffset <= 4096; nOffset += 4)
    {
        tools::Polygon aPolygon1(4);
        aPolygon1.SetPoint(Point(maVDRectangle.Left() + nOffset, maVDRectangle.Top() + nOffset), 0);
        aPolygon1.SetPoint(Point(maVDRectangle.Right() - nOffset, maVDRectangle.Top() + nOffset),
                           1);
        aPolygon1.SetPoint(Point(maVDRectangle.Right() - nOffset, maVDRectangle.Bottom() - nOffset),
                           2);
        aPolygon1.SetPoint(Point(maVDRectangle.Left() + nOffset, maVDRectangle.Bottom() - nOffset),
                           3);
        aPolyPolygon.Insert(aPolygon1);
    }

    m_xStart = std::chrono::steady_clock::now();
    mpVirtualDevice->DrawPolyPolygon(aPolyPolygon);
    Bitmap aBitmap = mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
    m_xEnd = std::chrono::steady_clock::now();
    return aBitmap;
}
