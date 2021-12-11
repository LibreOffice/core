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
#include <vcl/bitmapex.hxx>
#include <bitmap/BitmapWriteAccess.hxx>

#include <vector>

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

Bitmap Benchmark::setupBitmap()
{
    initialSetup(4096, 4096, constBackgroundColor);

    Size aBitmapSize(4095, 4095);
    Bitmap aBitmap(aBitmapSize, vcl::PixelFormat::N24_BPP);

    BitmapScopedWriteAccess aWriteAccess(aBitmap);
    aWriteAccess->Erase(constFillColor);
    aWriteAccess->SetLineColor(constLineColor);

    for (int i = 1; i < 4095; i += 4)
    {
        aWriteAccess->DrawRect(tools::Rectangle(i, i, 4095 - i, 4095 - i));
        aWriteAccess->DrawRect(tools::Rectangle(i + 1, i + 1, 4095 - i - 1, 4095 - i - 1));
    }

    Point aPoint((maVDRectangle.GetWidth() / 2.0) - (aBitmapSize.Width() / 2.0),
                 (maVDRectangle.GetHeight() / 2.0) - (aBitmapSize.Height() / 2.0));

    m_xStart = std::chrono::steady_clock::now();

    mpVirtualDevice->DrawBitmapEx(aPoint, BitmapEx(aBitmap));
    Bitmap rBitmap = mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());

    m_xEnd = std::chrono::steady_clock::now();
    return rBitmap;
}

Bitmap Benchmark::setupBitmapWithAlpha()
{
    initialSetup(4096, 4096, constBackgroundColor);

    Size aBitmapSize(4095, 4095);
    Bitmap aBitmap(aBitmapSize, vcl::PixelFormat::N24_BPP);

    BitmapScopedWriteAccess aWriteAccess(aBitmap);
    aWriteAccess->Erase(constFillColor);
    aWriteAccess->SetLineColor(Color(0xFF, 0xFF, 0x00));

    AlphaMask aAlpha(aBitmapSize);
    AlphaScopedWriteAccess rWriteAccess(aAlpha);
    rWriteAccess->Erase(COL_WHITE);
    rWriteAccess->SetLineColor(Color(0x44, 0x44, 0x44));

    for (int i = 1; i + 4 <= 4095; i += 4)
    {
        aWriteAccess->DrawRect(tools::Rectangle(i, i, 4095 - i, 4095 - i));
        aWriteAccess->DrawRect(tools::Rectangle(i + 1, i + 1, 4095 - i - 1, 4095 - i - 1));
        rWriteAccess->DrawRect(tools::Rectangle(i, i, 4095 - i, 4095 - i));
        rWriteAccess->DrawRect(tools::Rectangle(i + 1, i + 1, 4095 - i - 1, 4095 - i - 1));
    }

    tools::Rectangle aRect(Point(), aBitmapSize);
    Point aPoint((maVDRectangle.GetWidth() / 2.0) - (aRect.GetWidth() / 2.0),
                 (maVDRectangle.GetHeight() / 2.0) - (aRect.GetHeight() / 2.0));

    m_xStart = std::chrono::steady_clock::now();

    mpVirtualDevice->DrawBitmapEx(tools::Rectangle(aPoint, aRect.GetSize()).TopLeft(),
                                  BitmapEx(aBitmap, aAlpha));
    Bitmap rBitmap = mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());

    m_xEnd = std::chrono::steady_clock::now();
    return rBitmap;
}

Bitmap Benchmark::setupScaledBitmap()
{
    initialSetup(4096, 4096, constBackgroundColor);

    Size aBitmapSize(100, 100);
    Bitmap aBitmap(aBitmapSize, vcl::PixelFormat::N24_BPP);

    BitmapScopedWriteAccess aWriteAccess(aBitmap);
    aWriteAccess->Erase(constFillColor);
    aWriteAccess->SetLineColor(constLineColor);

    for (int i = 1; i + 4 <= 100; i += 4)
    {
        aWriteAccess->DrawRect(tools::Rectangle(i, i, 100 - i, 100 - i));
        aWriteAccess->DrawRect(tools::Rectangle(i + 1, i + 1, 100 - i - 1, 100 - i - 1));
    }

    m_xStart = std::chrono::steady_clock::now();

    BitmapEx aBitmapEx(aBitmap);
    aBitmapEx.Scale(Size(4095, 4095), BmpScaleFlag::Fast);

    mpVirtualDevice->DrawBitmapEx(Point(0, 0), aBitmapEx);
    Bitmap rBitmap = mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());

    m_xEnd = std::chrono::steady_clock::now();
    return rBitmap;
}

Bitmap Benchmark::setupReducedBitmap()
{
    initialSetup(4096, 4096, constBackgroundColor);

    Size aBitmapSize(8096, 8096);
    Bitmap aBitmap(aBitmapSize, vcl::PixelFormat::N24_BPP);

    BitmapScopedWriteAccess aWriteAccess(aBitmap);
    aWriteAccess->Erase(constFillColor);
    aWriteAccess->SetLineColor(constLineColor);

    for (int i = 1; i + 4 <= 8096; i += 4)
    {
        aWriteAccess->DrawRect(tools::Rectangle(i, i, 8096 - i, 8096 - i));
        aWriteAccess->DrawRect(tools::Rectangle(i + 1, i + 1, 8096 - i - 1, 8096 - i - 1));
    }

    m_xStart = std::chrono::steady_clock::now();

    BitmapEx aBitmapEx(aBitmap);
    aBitmapEx.Scale(Size(4095, 4095), BmpScaleFlag::Fast);

    mpVirtualDevice->DrawBitmapEx(Point(0, 0), aBitmapEx);
    Bitmap rBitmap = mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());

    m_xEnd = std::chrono::steady_clock::now();
    return rBitmap;
}

Bitmap Benchmark::setupRotatedBitmap()
{
    initialSetup(4096, 4096, constBackgroundColor);

    Size aBitmapSize(4095, 4095);
    Bitmap aBitmap(aBitmapSize, vcl::PixelFormat::N24_BPP);

    BitmapScopedWriteAccess aWriteAccess(aBitmap);
    aWriteAccess->Erase(constFillColor);
    aWriteAccess->SetLineColor(constLineColor);

    for (int i = 1; i < 4095; i += 4)
    {
        aWriteAccess->DrawRect(tools::Rectangle(i, i, 4095 - i, 4095 - i));
        aWriteAccess->DrawRect(tools::Rectangle(i + 1, i + 1, 4095 - i - 1, 4095 - i - 1));
    }

    Point aPoint((maVDRectangle.GetWidth() / 2.0) - (aBitmapSize.Width() / 2.0),
                 (maVDRectangle.GetHeight() / 2.0) - (aBitmapSize.Height() / 2.0));

    m_xStart = std::chrono::steady_clock::now();

    BitmapEx aBitmapEx(aBitmap);
    aBitmapEx.Rotate(Degree10(3600), COL_RED);
    mpVirtualDevice->DrawBitmapEx(aPoint, aBitmapEx);
    Bitmap rBitmap = mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());

    m_xEnd = std::chrono::steady_clock::now();
    return rBitmap;
}

Bitmap Benchmark::setupLinearGradient()
{
    initialSetup(4096, 4096, constBackgroundColor);

    std::vector<std::pair<Color, Color>> aGradients
        = { { Color(0x0, 0x9F, 0xFF), Color(0xEC, 0x2F, 0x4B) },
            { Color(0x12, 0xC2, 0xE9), Color(0xC4, 0x71, 0xED) },
            { Color(0xFF, 0x0, 0x99), Color(0x49, 0x32, 0x40) },
            { Color(0xF1, 0x27, 0x11), Color(0xF5, 0xAF, 0x19) },
            { Color(0x83, 0x60, 0xC3), Color(0x2E, 0xBF, 0x91) },
            { Color(0x0, 0x9F, 0xFF), Color(0xEC, 0x2F, 0x4B) },
            { Color(0x65, 0x4E, 0xA3), Color(0xEA, 0xAF, 0xC8) },
            { Color(0xFF, 0x41, 0x6C), Color(0xFF, 0x4B, 0x2B) } };

    m_xStart = std::chrono::steady_clock::now();
    for (int i = 0, nLeftOffset = 0, nRightOffset = 3840; i < 16;
         i++, nLeftOffset += 256, nRightOffset -= 256)
    {
        Gradient aGradient(GradientStyle::Linear, aGradients[i % 8].first,
                           aGradients[i % 8].second);
        aGradient.SetAngle(900_deg10);
        aGradient.SetBorder(50);
        tools::Rectangle aDrawRect(maVDRectangle.Left() + nLeftOffset, maVDRectangle.Top() + 1,
                                   maVDRectangle.Right() - nRightOffset,
                                   maVDRectangle.Bottom() - 1);
        mpVirtualDevice->DrawGradient(aDrawRect, aGradient);
    }
    Bitmap aBitmap = mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
    m_xEnd = std::chrono::steady_clock::now();
    return aBitmap;
}

Bitmap Benchmark::setupAxialGradient()
{
    initialSetup(4096, 4096, constBackgroundColor);

    std::vector<std::pair<Color, Color>> aGradients
        = { { Color(0x0, 0x9F, 0xFF), Color(0xEC, 0x2F, 0x4B) },
            { Color(0x12, 0xC2, 0xE9), Color(0xC4, 0x71, 0xED) },
            { Color(0xFF, 0x0, 0x99), Color(0x49, 0x32, 0x40) },
            { Color(0xF1, 0x27, 0x11), Color(0xF5, 0xAF, 0x19) },
            { Color(0x83, 0x60, 0xC3), Color(0x2E, 0xBF, 0x91) },
            { Color(0x0, 0x9F, 0xFF), Color(0xEC, 0x2F, 0x4B) },
            { Color(0x65, 0x4E, 0xA3), Color(0xEA, 0xAF, 0xC8) },
            { Color(0xFF, 0x41, 0x6C), Color(0xFF, 0x4B, 0x2B) } };

    m_xStart = std::chrono::steady_clock::now();
    for (int i = 0, nLeftOffset = 0, nRightOffset = 3840; i < 16;
         i++, nLeftOffset += 256, nRightOffset -= 256)
    {
        Gradient aGradient(GradientStyle::Axial, aGradients[i % 8].first, aGradients[i % 8].second);
        aGradient.SetAngle(900_deg10);
        aGradient.SetBorder(50);
        tools::Rectangle aDrawRect(maVDRectangle.Left() + nLeftOffset, maVDRectangle.Top() + 1,
                                   maVDRectangle.Right() - nRightOffset,
                                   maVDRectangle.Bottom() - 1);
        mpVirtualDevice->DrawGradient(aDrawRect, aGradient);
    }
    Bitmap aBitmap = mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
    m_xEnd = std::chrono::steady_clock::now();
    return aBitmap;
}

Bitmap Benchmark::setupRadialGradient()
{
    initialSetup(4096, 4096, constBackgroundColor);

    std::vector<std::pair<Color, Color>> aGradients
        = { { Color(0x0, 0x9F, 0xFF), Color(0xEC, 0x2F, 0x4B) },
            { Color(0x12, 0xC2, 0xE9), Color(0xC4, 0x71, 0xED) },
            { Color(0xFF, 0x0, 0x99), Color(0x49, 0x32, 0x40) },
            { Color(0xF1, 0x27, 0x11), Color(0xF5, 0xAF, 0x19) },
            { Color(0x83, 0x60, 0xC3), Color(0x2E, 0xBF, 0x91) },
            { Color(0x0, 0x9F, 0xFF), Color(0xEC, 0x2F, 0x4B) },
            { Color(0x65, 0x4E, 0xA3), Color(0xEA, 0xAF, 0xC8) },
            { Color(0xFF, 0x41, 0x6C), Color(0xFF, 0x4B, 0x2B) } };

    m_xStart = std::chrono::steady_clock::now();
    for (int i = 0, nLeftOffset = 0, nRightOffset = 3840; i < 16;
         i++, nLeftOffset += 256, nRightOffset -= 256)
    {
        Gradient aGradient(GradientStyle::Radial, aGradients[i % 8].first,
                           aGradients[i % 8].second);
        aGradient.SetAngle(900_deg10);
        aGradient.SetBorder(50);
        tools::Rectangle aDrawRect(maVDRectangle.Left() + nLeftOffset, maVDRectangle.Top() + 1,
                                   maVDRectangle.Right() - nRightOffset,
                                   maVDRectangle.Bottom() - 1);
        mpVirtualDevice->DrawGradient(aDrawRect, aGradient);
    }
    Bitmap aBitmap = mpVirtualDevice->GetBitmap(maVDRectangle.TopLeft(), maVDRectangle.GetSize());
    m_xEnd = std::chrono::steady_clock::now();
    return aBitmap;
}
