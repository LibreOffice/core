/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <bitmap/ScanlineHalfScaler.hxx>

#include <tools/helpers.hxx>
#include <algorithm>
#include <memory>
#include <tools/cpuid.hxx>
#include <tools/simdsupport.hxx>

#if defined(LO_SSE2_AVAILABLE)
#include <emmintrin.h>
#endif

namespace vcl
{
namespace
{
inline sal_uInt32 Avg2x2(sal_uInt32 a, sal_uInt32 b, sal_uInt32 c, sal_uInt32 d)
{
    // Prepare half-adder work
    sal_uInt32 sum = a ^ b ^ c;
    sal_uInt32 carry = (a & b) | (a & c) | (b & c);

    // Before shifting, mask lower order bits of each byte to avoid underflow.
    sal_uInt32 mask = 0xfefefefe;

    // Add d to sum and divide by 2.
    sum = (((sum ^ d) & mask) >> 1) + (sum & d);

    // Sum is now shifted into place relative to carry, add them together.
    return (((sum ^ carry) & mask) >> 1) + (sum & carry);
}
}

void scaleHalfGeneralHorizontal(ScaleContext& rContext, long nStartY, long nEndY)
{
    const long nStartX = 0;
    const long nEndX = rContext.mnTargetW - 1;

    for (long nY = nStartY; nY <= nEndY; nY++)
    {
        Scanline pSourceScanline = rContext.getSourceScanline(nY);
        Scanline pTargetScanline = rContext.getTargetScanline(nY);

        long nTargetX = 0;

        for (long nX = nStartX; nX <= nEndX; nX++)
        {
            long nSourceX = nX * 2;

            BitmapColor aColor0 = rContext.getSourcePixel(pSourceScanline, nSourceX);
            BitmapColor aColor1 = rContext.getSourcePixel(pSourceScanline, nSourceX + 1);

            BitmapColor aColorResult((aColor0.GetRed() + aColor1.GetRed()) / 2,
                                     (aColor0.GetGreen() + aColor1.GetGreen()) / 2,
                                     (aColor0.GetBlue() + aColor1.GetBlue()) / 2);

            rContext.setTargetPixel(pTargetScanline, nTargetX++, aColorResult);
        }
    }
}

void scaleHalfGeneralVertical(ScaleContext& rContext, long nStartY, long nEndY)
{
    const long nStartX = 0;
    const long nEndX = rContext.mnTargetW - 1;

    for (long nY = nStartY; nY <= nEndY; nY++)
    {
        long nSourceY = nY * 2;

        Scanline pSourceScanline0 = rContext.mpSource->GetScanline(nSourceY);
        Scanline pSourceScanline1 = rContext.mpSource->GetScanline(nSourceY + 1);

        Scanline pTargetScanline = rContext.mpTarget->GetScanline(nY);

        long nSourceX = nStartX;
        long nTargetX = 0;

        for (long nX = nStartX; nX <= nEndX; nX++)
        {
            BitmapColor aColor0 = rContext.mpSource->GetPixelFromData(pSourceScanline0, nSourceX);
            BitmapColor aColor1 = rContext.mpSource->GetPixelFromData(pSourceScanline1, nSourceX);

            BitmapColor aColorResult((aColor0.GetRed() + aColor1.GetRed()) / 2,
                                     (aColor0.GetGreen() + aColor1.GetGreen()) / 2,
                                     (aColor0.GetBlue() + aColor1.GetBlue()) / 2);

            rContext.mpTarget->SetPixelOnData(pTargetScanline, nTargetX++, aColorResult);
        }
    }
}

void scaleHalfGeneral(ScaleContext& rContext, long nStartY, long nEndY)
{
    const long nStartX = 0;
    const long nEndX = rContext.mnTargetW - 1;

    long nSourceY = nStartY * 2;

    for (long nY = nStartY; nY <= nEndY; nY++)
    {
        Scanline pSource0 = rContext.mpSource->GetScanline(nSourceY++);
        Scanline pSource1 = rContext.mpSource->GetScanline(nSourceY++);

        Scanline pScanDest = rContext.mpTarget->GetScanline(nY);

        long nTargetX = 0;

        BitmapColor aColor;

        for (long nX = nStartX; nX <= nEndX; nX++)
        {
            long nRed = 0;
            long nGreen = 0;
            long nBlue = 0;
            long nAlpha = 0;

            long nSourceX = nX * 2;

            aColor = rContext.mpSource->GetPixelFromData(pSource0, nSourceX);
            nRed += aColor.GetRed();
            nGreen += aColor.GetGreen();
            nBlue += aColor.GetBlue();
            nAlpha += aColor.GetAlpha();

            aColor = rContext.mpSource->GetPixelFromData(pSource0, nSourceX + 1);
            nRed += aColor.GetRed();
            nGreen += aColor.GetGreen();
            nBlue += aColor.GetBlue();
            nAlpha += aColor.GetAlpha();

            aColor = rContext.mpSource->GetPixelFromData(pSource1, nSourceX);
            nRed += aColor.GetRed();
            nGreen += aColor.GetGreen();
            nBlue += aColor.GetBlue();
            nAlpha += aColor.GetAlpha();

            aColor = rContext.mpSource->GetPixelFromData(pSource1, nSourceX + 1);
            nRed += aColor.GetRed();
            nGreen += aColor.GetGreen();
            nBlue += aColor.GetBlue();
            nAlpha += aColor.GetAlpha();

            BitmapColor aColorResult(ColorAlpha, nRed / 4, nGreen / 4, nBlue / 4, nAlpha / 4);

            rContext.mpTarget->SetPixelOnData(pScanDest, nTargetX++, aColorResult);
        }
    }
}

void scaleQuarterGeneral(ScaleContext& rContext, long nStartY, long nEndY)
{
    const long nStartX = 0;
    const long nEndX = rContext.mnTargetW - 1;

    long nSourceY = nStartY * 4;

    for (long nY = nStartY; nY <= nEndY; nY++)
    {
        Scanline pSource0 = rContext.mpSource->GetScanline(nSourceY++);
        Scanline pSource1 = rContext.mpSource->GetScanline(nSourceY++);
        Scanline pSource2 = rContext.mpSource->GetScanline(nSourceY++);
        Scanline pSource3 = rContext.mpSource->GetScanline(nSourceY++);

        Scanline pScanDest = rContext.mpTarget->GetScanline(nY);

        long nTargetX = 0;

        BitmapColor aColor;

        for (long nX = nStartX; nX <= nEndX; nX++)
        {
            long nRed = 0;
            long nGreen = 0;
            long nBlue = 0;
            long nAlpha = 0;

            long nSourceX = nX * 4;

            aColor = rContext.mpSource->GetPixelFromData(pSource0, nSourceX);
            nRed += aColor.GetRed();
            nGreen += aColor.GetGreen();
            nBlue += aColor.GetBlue();
            nAlpha += aColor.GetAlpha();

            aColor = rContext.mpSource->GetPixelFromData(pSource0, nSourceX + 1);
            nRed += aColor.GetRed();
            nGreen += aColor.GetGreen();
            nBlue += aColor.GetBlue();
            nAlpha += aColor.GetAlpha();

            aColor = rContext.mpSource->GetPixelFromData(pSource0, nSourceX + 2);
            nRed += aColor.GetRed();
            nGreen += aColor.GetGreen();
            nBlue += aColor.GetBlue();
            nAlpha += aColor.GetAlpha();

            aColor = rContext.mpSource->GetPixelFromData(pSource0, nSourceX + 3);
            nRed += aColor.GetRed();
            nGreen += aColor.GetGreen();
            nBlue += aColor.GetBlue();
            nAlpha += aColor.GetAlpha();

            aColor = rContext.mpSource->GetPixelFromData(pSource1, nSourceX);
            nRed += aColor.GetRed();
            nGreen += aColor.GetGreen();
            nBlue += aColor.GetBlue();
            nAlpha += aColor.GetAlpha();

            aColor = rContext.mpSource->GetPixelFromData(pSource1, nSourceX + 1);
            nRed += aColor.GetRed();
            nGreen += aColor.GetGreen();
            nBlue += aColor.GetBlue();
            nAlpha += aColor.GetAlpha();

            aColor = rContext.mpSource->GetPixelFromData(pSource1, nSourceX + 2);
            nRed += aColor.GetRed();
            nGreen += aColor.GetGreen();
            nBlue += aColor.GetBlue();
            nAlpha += aColor.GetAlpha();

            aColor = rContext.mpSource->GetPixelFromData(pSource1, nSourceX + 3);
            nRed += aColor.GetRed();
            nGreen += aColor.GetGreen();
            nBlue += aColor.GetBlue();
            nAlpha += aColor.GetAlpha();

            aColor = rContext.mpSource->GetPixelFromData(pSource2, nSourceX);
            nRed += aColor.GetRed();
            nGreen += aColor.GetGreen();
            nBlue += aColor.GetBlue();
            nAlpha += aColor.GetAlpha();

            aColor = rContext.mpSource->GetPixelFromData(pSource2, nSourceX + 1);
            nRed += aColor.GetRed();
            nGreen += aColor.GetGreen();
            nBlue += aColor.GetBlue();
            nAlpha += aColor.GetAlpha();

            aColor = rContext.mpSource->GetPixelFromData(pSource2, nSourceX + 2);
            nRed += aColor.GetRed();
            nGreen += aColor.GetGreen();
            nBlue += aColor.GetBlue();
            nAlpha += aColor.GetAlpha();

            aColor = rContext.mpSource->GetPixelFromData(pSource2, nSourceX + 3);
            nRed += aColor.GetRed();
            nGreen += aColor.GetGreen();
            nBlue += aColor.GetBlue();
            nAlpha += aColor.GetAlpha();

            aColor = rContext.mpSource->GetPixelFromData(pSource3, nSourceX);
            nRed += aColor.GetRed();
            nGreen += aColor.GetGreen();
            nBlue += aColor.GetBlue();
            nAlpha += aColor.GetAlpha();

            aColor = rContext.mpSource->GetPixelFromData(pSource3, nSourceX + 1);
            nRed += aColor.GetRed();
            nGreen += aColor.GetGreen();
            nBlue += aColor.GetBlue();
            nAlpha += aColor.GetAlpha();

            aColor = rContext.mpSource->GetPixelFromData(pSource3, nSourceX + 2);
            nRed += aColor.GetRed();
            nGreen += aColor.GetGreen();
            nBlue += aColor.GetBlue();
            nAlpha += aColor.GetAlpha();

            aColor = rContext.mpSource->GetPixelFromData(pSource3, nSourceX + 3);
            nRed += aColor.GetRed();
            nGreen += aColor.GetGreen();
            nBlue += aColor.GetBlue();
            nAlpha += aColor.GetAlpha();

            BitmapColor aColorResult(ColorAlpha, nRed / 16, nGreen / 16, nBlue / 16, nAlpha / 16);

            rContext.mpTarget->SetPixelOnData(pScanDest, nTargetX++, aColorResult);
        }
    }
}

void scaleQuarterGeneralHorizontal(ScaleContext& rContext, long nStartY, long nEndY)
{
    const long nStartX = 0;
    const long nEndX = rContext.mnTargetW - 1;

    for (long nY = nStartY; nY <= nEndY; nY++)
    {
        Scanline pSourceScanline = rContext.mpSource->GetScanline(nY);
        Scanline pTargetScanline = rContext.mpTarget->GetScanline(nY);

        long nTargetX = 0;

        for (long nX = nStartX; nX <= nEndX; nX++)
        {
            long nSourceX = nX * 4;

            BitmapColor aColor0 = rContext.mpSource->GetPixelFromData(pSourceScanline, nSourceX);
            BitmapColor aColor1
                = rContext.mpSource->GetPixelFromData(pSourceScanline, nSourceX + 1);
            BitmapColor aColor2
                = rContext.mpSource->GetPixelFromData(pSourceScanline, nSourceX + 2);
            BitmapColor aColor3
                = rContext.mpSource->GetPixelFromData(pSourceScanline, nSourceX + 3);

            BitmapColor aColorResult(
                (aColor0.GetRed() + aColor1.GetRed() + aColor2.GetRed() + aColor3.GetRed()) / 4,
                (aColor0.GetGreen() + aColor1.GetGreen() + aColor2.GetGreen() + aColor3.GetGreen())
                    / 4,
                (aColor0.GetBlue() + aColor1.GetBlue() + aColor2.GetBlue() + aColor3.GetBlue())
                    / 4);

            rContext.mpTarget->SetPixelOnData(pTargetScanline, nTargetX++, aColorResult);
        }
    }
}

void scaleQuarterGeneralVertical(ScaleContext& rContext, long nStartY, long nEndY)
{
    const long nStartX = 0;
    const long nEndX = rContext.mnTargetW - 1;

    for (long nY = nStartY; nY <= nEndY; nY++)
    {
        long nSourceY = nY * 4;

        Scanline pSourceScanline0 = rContext.mpSource->GetScanline(nSourceY);
        Scanline pSourceScanline1 = rContext.mpSource->GetScanline(nSourceY + 1);
        Scanline pSourceScanline2 = rContext.mpSource->GetScanline(nSourceY + 2);
        Scanline pSourceScanline3 = rContext.mpSource->GetScanline(nSourceY + 3);

        Scanline pTargetScanline = rContext.mpTarget->GetScanline(nY);

        long nSourceX = nStartX;
        long nTargetX = 0;

        for (long nX = nStartX; nX <= nEndX; nX++)
        {
            BitmapColor aColor0 = rContext.mpSource->GetPixelFromData(pSourceScanline0, nSourceX);
            BitmapColor aColor1 = rContext.mpSource->GetPixelFromData(pSourceScanline1, nSourceX);
            BitmapColor aColor2 = rContext.mpSource->GetPixelFromData(pSourceScanline2, nSourceX);
            BitmapColor aColor3 = rContext.mpSource->GetPixelFromData(pSourceScanline3, nSourceX);

            BitmapColor aColorResult(
                (aColor0.GetRed() + aColor1.GetRed() + aColor2.GetRed() + aColor3.GetRed()) / 4,
                (aColor0.GetGreen() + aColor1.GetGreen() + aColor2.GetGreen() + aColor3.GetGreen())
                    / 4,
                (aColor0.GetBlue() + aColor1.GetBlue() + aColor2.GetBlue() + aColor3.GetBlue())
                    / 4);

            rContext.mpTarget->SetPixelOnData(pTargetScanline, nTargetX++, aColorResult);
        }
    }
}

void scaleOctalGeneral(ScaleContext& rContext, long nStartY, long nEndY)
{
    const long nStartX = 0;
    const long nEndX = rContext.mnTargetW - 1;

    for (long nY = nStartY; nY <= nEndY; nY++)
    {
        long nSourceY = nY * 8;

        Scanline pSource0 = rContext.mpSource->GetScanline(nSourceY++);
        Scanline pSource1 = rContext.mpSource->GetScanline(nSourceY++);
        Scanline pSource2 = rContext.mpSource->GetScanline(nSourceY++);
        Scanline pSource3 = rContext.mpSource->GetScanline(nSourceY++);
        Scanline pSource4 = rContext.mpSource->GetScanline(nSourceY++);
        Scanline pSource5 = rContext.mpSource->GetScanline(nSourceY++);
        Scanline pSource6 = rContext.mpSource->GetScanline(nSourceY++);
        Scanline pSource7 = rContext.mpSource->GetScanline(nSourceY);

        Scanline pTargetScanline = rContext.mpTarget->GetScanline(nY);

        long nTargetX = 0;

        for (long nX = nStartX; nX <= nEndX; nX++)
        {
            long nRed = 0;
            long nGreen = 0;
            long nBlue = 0;
            long nAlpha = 0;

            long nSourceX = nX * 8;

            for (long i = 0; i < 8; i++)
            {
                BitmapColor aColor = rContext.mpSource->GetPixelFromData(pSource0, nSourceX + i);
                nRed += aColor.GetRed();
                nGreen += aColor.GetGreen();
                nBlue += aColor.GetBlue();
                nAlpha += aColor.GetAlpha();
            }

            for (long i = 0; i < 8; i++)
            {
                BitmapColor aColor = rContext.mpSource->GetPixelFromData(pSource1, nSourceX + i);
                nRed += aColor.GetRed();
                nGreen += aColor.GetGreen();
                nBlue += aColor.GetBlue();
                nAlpha += aColor.GetAlpha();
            }

            for (long i = 0; i < 8; i++)
            {
                BitmapColor aColor = rContext.mpSource->GetPixelFromData(pSource2, nSourceX + i);
                nRed += aColor.GetRed();
                nGreen += aColor.GetGreen();
                nBlue += aColor.GetBlue();
                nAlpha += aColor.GetAlpha();
            }

            for (long i = 0; i < 8; i++)
            {
                BitmapColor aColor = rContext.mpSource->GetPixelFromData(pSource3, nSourceX + i);
                nRed += aColor.GetRed();
                nGreen += aColor.GetGreen();
                nBlue += aColor.GetBlue();
                nAlpha += aColor.GetAlpha();
            }

            for (long i = 0; i < 8; i++)
            {
                BitmapColor aColor = rContext.mpSource->GetPixelFromData(pSource4, nSourceX + i);
                nRed += aColor.GetRed();
                nGreen += aColor.GetGreen();
                nBlue += aColor.GetBlue();
                nAlpha += aColor.GetAlpha();
            }

            for (long i = 0; i < 8; i++)
            {
                BitmapColor aColor = rContext.mpSource->GetPixelFromData(pSource5, nSourceX + i);
                nRed += aColor.GetRed();
                nGreen += aColor.GetGreen();
                nBlue += aColor.GetBlue();
                nAlpha += aColor.GetAlpha();
            }

            for (long i = 0; i < 8; i++)
            {
                BitmapColor aColor = rContext.mpSource->GetPixelFromData(pSource6, nSourceX + i);
                nRed += aColor.GetRed();
                nGreen += aColor.GetGreen();
                nBlue += aColor.GetBlue();
                nAlpha += aColor.GetAlpha();
            }

            for (long i = 0; i < 8; i++)
            {
                BitmapColor aColor = rContext.mpSource->GetPixelFromData(pSource7, nSourceX + i);
                nRed += aColor.GetRed();
                nGreen += aColor.GetGreen();
                nBlue += aColor.GetBlue();
                nAlpha += aColor.GetAlpha();
            }

            BitmapColor aColorResult(nRed / 64, nGreen / 64, nBlue / 64);

            rContext.mpTarget->SetPixelOnData(pTargetScanline, nTargetX++, aColorResult);
        }
    }
}

void scaleOctal32(ScaleContext& rContext, long nStartY, long nEndY)
{
    constexpr int constColorComponents = 4;
    constexpr int constNumberSamples = 4;
    constexpr int constNumberSamplesSquared = (constNumberSamples * constNumberSamples);

    const long nStartX = 0;
    const long nEndX = rContext.mnTargetW - 1;

    for (long nY = nStartY; nY <= nEndY; nY++)
    {
        long nSourceY = nY * constNumberSamples;

        Scanline pSource0 = rContext.mpSource->GetScanline(nSourceY++);
        Scanline pSource1 = rContext.mpSource->GetScanline(nSourceY++);
        Scanline pSource2 = rContext.mpSource->GetScanline(nSourceY++);
        Scanline pSource3 = rContext.mpSource->GetScanline(nSourceY++);
        Scanline pSource4 = rContext.mpSource->GetScanline(nSourceY++);
        Scanline pSource5 = rContext.mpSource->GetScanline(nSourceY++);
        Scanline pSource6 = rContext.mpSource->GetScanline(nSourceY++);
        Scanline pSource7 = rContext.mpSource->GetScanline(nSourceY);

        Scanline pTargetScanline = rContext.mpTarget->GetScanline(nY);

        Scanline pColorPtr;

        for (long nX = nStartX; nX <= nEndX; nX++)
        {
            long nComponent1 = 0;
            long nComponent2 = 0;
            long nComponent3 = 0;
            long nComponent4 = 0;

            long nSourceX = nX * constNumberSamples;

            pColorPtr = pSource0 + constColorComponents * nSourceX;
            for (long i = 0; i < constNumberSamples; i++)
            {
                nComponent1 += *pColorPtr;
                pColorPtr++;
                nComponent2 += *pColorPtr;
                pColorPtr++;
                nComponent3 += *pColorPtr;
                pColorPtr++;
                nComponent4 += *pColorPtr;
                pColorPtr++;
            }

            pColorPtr = pSource1 + constColorComponents * nSourceX;
            for (long i = 0; i < constNumberSamples; i++)
            {
                nComponent1 += *pColorPtr;
                pColorPtr++;
                nComponent2 += *pColorPtr;
                pColorPtr++;
                nComponent3 += *pColorPtr;
                pColorPtr++;
                nComponent4 += *pColorPtr;
                pColorPtr++;
            }

            pColorPtr = pSource2 + constColorComponents * nSourceX;
            for (long i = 0; i < constNumberSamples; i++)
            {
                nComponent1 += *pColorPtr;
                pColorPtr++;
                nComponent2 += *pColorPtr;
                pColorPtr++;
                nComponent3 += *pColorPtr;
                pColorPtr++;
                nComponent4 += *pColorPtr;
                pColorPtr++;
            }

            pColorPtr = pSource3 + constColorComponents * nSourceX;
            for (long i = 0; i < constNumberSamples; i++)
            {
                nComponent1 += *pColorPtr;
                pColorPtr++;
                nComponent2 += *pColorPtr;
                pColorPtr++;
                nComponent3 += *pColorPtr;
                pColorPtr++;
                nComponent4 += *pColorPtr;
                pColorPtr++;
            }

            pColorPtr = pSource4 + constColorComponents * nSourceX;
            for (long i = 0; i < constNumberSamples; i++)
            {
                nComponent1 += *pColorPtr;
                pColorPtr++;
                nComponent2 += *pColorPtr;
                pColorPtr++;
                nComponent3 += *pColorPtr;
                pColorPtr++;
                nComponent4 += *pColorPtr;
                pColorPtr++;
            }

            pColorPtr = pSource5 + constColorComponents * nSourceX;
            for (long i = 0; i < constNumberSamples; i++)
            {
                nComponent1 += *pColorPtr;
                pColorPtr++;
                nComponent2 += *pColorPtr;
                pColorPtr++;
                nComponent3 += *pColorPtr;
                pColorPtr++;
                nComponent4 += *pColorPtr;
                pColorPtr++;
            }

            pColorPtr = pSource6 + constColorComponents * nSourceX;
            for (long i = 0; i < constNumberSamples; i++)
            {
                nComponent1 += *pColorPtr;
                pColorPtr++;
                nComponent2 += *pColorPtr;
                pColorPtr++;
                nComponent3 += *pColorPtr;
                pColorPtr++;
                nComponent4 += *pColorPtr;
                pColorPtr++;
            }

            pColorPtr = pSource7 + constColorComponents * nSourceX;
            for (long i = 0; i < constNumberSamples; i++)
            {
                nComponent1 += *pColorPtr;
                pColorPtr++;
                nComponent2 += *pColorPtr;
                pColorPtr++;
                nComponent3 += *pColorPtr;
                pColorPtr++;
                nComponent4 += *pColorPtr;
                pColorPtr++;
            }

            *pTargetScanline = nComponent1 / constNumberSamplesSquared;
            pTargetScanline++;
            *pTargetScanline = nComponent2 / constNumberSamplesSquared;
            pTargetScanline++;
            *pTargetScanline = nComponent3 / constNumberSamplesSquared;
            pTargetScanline++;
            *pTargetScanline = nComponent4 / constNumberSamplesSquared;
            pTargetScanline++;
        }
    }
}

void scaleQuarter32_1(ScaleContext& rContext, long nStartY, long nEndY)
{
    constexpr int constColorComponents = 4;
    constexpr int constNumberSamples = 4;
    constexpr int constNumberSamplesSquared = (constNumberSamples * constNumberSamples);

    const long nStartX = 0;
    const long nEndX = rContext.mnTargetW - 1;

    for (long nY = nStartY; nY <= nEndY; nY++)
    {
        long nSourceY = nY * constNumberSamples;

        Scanline pSource0 = rContext.mpSource->GetScanline(nSourceY++);
        Scanline pSource1 = rContext.mpSource->GetScanline(nSourceY++);
        Scanline pSource2 = rContext.mpSource->GetScanline(nSourceY++);
        Scanline pSource3 = rContext.mpSource->GetScanline(nSourceY);

        Scanline pTargetScanline = rContext.mpTarget->GetScanline(nY);

        Scanline pColorPtr;

        for (long nX = nStartX; nX <= nEndX; nX++)
        {
            long nComponent1 = 0;
            long nComponent2 = 0;
            long nComponent3 = 0;
            long nComponent4 = 0;

            long nSourceX = nX * constNumberSamples;

            pColorPtr = pSource0 + constColorComponents * nSourceX;
            for (long i = 0; i < constNumberSamples; i++)
            {
                nComponent1 += *pColorPtr;
                pColorPtr++;
                nComponent2 += *pColorPtr;
                pColorPtr++;
                nComponent3 += *pColorPtr;
                pColorPtr++;
                nComponent4 += *pColorPtr;
                pColorPtr++;
            }

            pColorPtr = pSource1 + constColorComponents * nSourceX;
            for (long i = 0; i < constNumberSamples; i++)
            {
                nComponent1 += *pColorPtr;
                pColorPtr++;
                nComponent2 += *pColorPtr;
                pColorPtr++;
                nComponent3 += *pColorPtr;
                pColorPtr++;
                nComponent4 += *pColorPtr;
                pColorPtr++;
            }

            pColorPtr = pSource2 + constColorComponents * nSourceX;
            for (long i = 0; i < constNumberSamples; i++)
            {
                nComponent1 += *pColorPtr;
                pColorPtr++;
                nComponent2 += *pColorPtr;
                pColorPtr++;
                nComponent3 += *pColorPtr;
                pColorPtr++;
                nComponent4 += *pColorPtr;
                pColorPtr++;
            }

            pColorPtr = pSource3 + constColorComponents * nSourceX;
            for (long i = 0; i < constNumberSamples; i++)
            {
                nComponent1 += *pColorPtr;
                pColorPtr++;
                nComponent2 += *pColorPtr;
                pColorPtr++;
                nComponent3 += *pColorPtr;
                pColorPtr++;
                nComponent4 += *pColorPtr;
                pColorPtr++;
            }

            *pTargetScanline = nComponent1 / constNumberSamplesSquared;
            pTargetScanline++;
            *pTargetScanline = nComponent2 / constNumberSamplesSquared;
            pTargetScanline++;
            *pTargetScanline = nComponent3 / constNumberSamplesSquared;
            pTargetScanline++;
            *pTargetScanline = nComponent4 / constNumberSamplesSquared;
            pTargetScanline++;
        }
    }
}

void scaleQuarter32_2(ScaleContext& rContext, long nStartY, long nEndY)
{
    constexpr int constNumberSamples = 4;

    const long nSourceW = rContext.mnSourceW;

    sal_uInt32 nColor00;
    sal_uInt32 nColor01;
    sal_uInt32 nColor10;
    sal_uInt32 nColor11;

    sal_uInt32 nA1;
    sal_uInt32 nA2;
    sal_uInt32 nA3;
    sal_uInt32 nA4;

    for (long nY = nStartY; nY <= nEndY; nY++)
    {
        long nSourceY1 = nY * constNumberSamples;
        long nSourceY2 = nSourceY1 + 1;
        long nSourceY3 = nSourceY2 + 1;
        long nSourceY4 = nSourceY3 + 1;

        sal_uInt32* pSource0
            = reinterpret_cast<sal_uInt32*>(rContext.mpSource->GetScanline(nSourceY1));
        sal_uInt32* pSource1
            = reinterpret_cast<sal_uInt32*>(rContext.mpSource->GetScanline(nSourceY2));
        sal_uInt32* pSource2
            = reinterpret_cast<sal_uInt32*>(rContext.mpSource->GetScanline(nSourceY3));
        sal_uInt32* pSource3
            = reinterpret_cast<sal_uInt32*>(rContext.mpSource->GetScanline(nSourceY4));

        sal_uInt32* pTarget = reinterpret_cast<sal_uInt32*>(rContext.mpTarget->GetScanline(nY));

        for (long nSourceX = 0; nSourceX < nSourceW; nSourceX += constNumberSamples)
        {
            nColor00 = *pSource0;
            pSource0++;
            nColor01 = *pSource0;
            pSource0++;

            nColor10 = *pSource1;
            pSource1++;
            nColor11 = *pSource1;
            pSource1++;

            nA1 = Avg2x2(nColor00, nColor01, nColor10, nColor11);

            nColor00 = *pSource0;
            pSource0++;
            nColor01 = *pSource0;
            pSource0++;

            nColor10 = *pSource1;
            pSource1++;
            nColor11 = *pSource1;
            pSource1++;

            nA2 = Avg2x2(nColor00, nColor01, nColor10, nColor11);

            nColor00 = *pSource2;
            pSource2++;
            nColor01 = *pSource2;
            pSource2++;

            nColor10 = *pSource3;
            pSource3++;
            nColor11 = *pSource3;
            pSource3++;

            nA3 = Avg2x2(nColor00, nColor01, nColor10, nColor11);

            nColor00 = *pSource2;
            pSource2++;
            nColor01 = *pSource2;
            pSource2++;

            nColor10 = *pSource3;
            pSource3++;
            nColor11 = *pSource3;
            pSource3++;

            nA4 = Avg2x2(nColor00, nColor01, nColor10, nColor11);

            *pTarget = Avg2x2(nA1, nA2, nA3, nA4);
            pTarget++;
        }
    }
}

VCL_FORCEINLINE __m128i _mm_not_si128(__m128i arg)
{
    __m128i minusone = _mm_set1_epi32(0xffffffff);
    return _mm_xor_si128(arg, minusone);
}

VCL_FORCEINLINE __m128i avg_sse2_8x2(__m128i* a, __m128i* b, __m128i* c, __m128i* d)
{
#define shuffle_si128(arga, argb, imm)                                                             \
    _mm_castps_si128(_mm_shuffle_ps(_mm_castsi128_ps((arga)), _mm_castsi128_ps((argb)), (imm)));

    __m128i t = shuffle_si128(*a, *b, _MM_SHUFFLE(2, 0, 2, 0));
    *b = shuffle_si128(*a, *b, _MM_SHUFFLE(3, 1, 3, 1));
    *a = t;
    t = shuffle_si128(*c, *d, _MM_SHUFFLE(2, 0, 2, 0));
    *d = shuffle_si128(*c, *d, _MM_SHUFFLE(3, 1, 3, 1));
    *c = t;

#undef shuffle_si128

    __m128i sum = _mm_xor_si128(*a, _mm_xor_si128(*b, *c));

    __m128i carry = _mm_or_si128(_mm_and_si128(*a, *b),
                                 _mm_or_si128(_mm_and_si128(*a, *c), _mm_and_si128(*b, *c)));

    sum = _mm_avg_epu8(_mm_not_si128(sum), _mm_not_si128(*d));

    return _mm_not_si128(_mm_avg_epu8(sum, _mm_not_si128(carry)));
}

void scaleQuarter32_SSE(ScaleContext& rContext, long nStartY, long nEndY)
{
    constexpr int constNumberSamples = 4;

    const long nSourceW = rContext.mnSourceW;

    __m128i nColor00;
    __m128i nColor01;
    __m128i nColor02;
    __m128i nColor03;

    __m128i nColor10;
    __m128i nColor11;
    __m128i nColor12;
    __m128i nColor13;

    __m128i nA1;
    __m128i nA2;
    __m128i nA3;
    __m128i nA4;

    for (long nY = nStartY; nY <= nEndY; nY++)
    {
        long nSourceY1 = nY * constNumberSamples;
        long nSourceY2 = nSourceY1 + 1;
        long nSourceY3 = nSourceY2 + 1;
        long nSourceY4 = nSourceY3 + 1;

        __m128i* pSource0 = reinterpret_cast<__m128i*>(rContext.mpSource->GetScanline(nSourceY1));
        __m128i* pSource1 = reinterpret_cast<__m128i*>(rContext.mpSource->GetScanline(nSourceY2));
        __m128i* pSource2 = reinterpret_cast<__m128i*>(rContext.mpSource->GetScanline(nSourceY3));
        __m128i* pSource3 = reinterpret_cast<__m128i*>(rContext.mpSource->GetScanline(nSourceY4));

        __m128i* pTarget = reinterpret_cast<__m128i*>(rContext.mpTarget->GetScanline(nY));

        for (long nSourceX = 0; nSourceX < nSourceW; nSourceX += constNumberSamples * 4)
        {
            nColor00 = _mm_loadu_si128(pSource0);
            pSource0++;
            nColor01 = _mm_loadu_si128(pSource0);
            pSource0++;
            nColor02 = _mm_loadu_si128(pSource0);
            pSource0++;
            nColor03 = _mm_loadu_si128(pSource0);
            pSource0++;

            nColor10 = _mm_loadu_si128(pSource1);
            pSource1++;
            nColor11 = _mm_loadu_si128(pSource1);
            pSource1++;
            nColor12 = _mm_loadu_si128(pSource1);
            pSource1++;
            nColor13 = _mm_loadu_si128(pSource1);
            pSource1++;

            nA1 = avg_sse2_8x2(&nColor00, &nColor01, &nColor10, &nColor11);
            nA2 = avg_sse2_8x2(&nColor02, &nColor03, &nColor12, &nColor13);

            nColor00 = _mm_loadu_si128(pSource2);
            pSource2++;
            nColor01 = _mm_loadu_si128(pSource2);
            pSource2++;
            nColor02 = _mm_loadu_si128(pSource2);
            pSource2++;
            nColor03 = _mm_loadu_si128(pSource2);
            pSource2++;

            nColor10 = _mm_loadu_si128(pSource3);
            pSource3++;
            nColor11 = _mm_loadu_si128(pSource3);
            pSource3++;
            nColor12 = _mm_loadu_si128(pSource3);
            pSource3++;
            nColor13 = _mm_loadu_si128(pSource3);
            pSource3++;

            nA3 = avg_sse2_8x2(&nColor00, &nColor01, &nColor10, &nColor11);
            nA4 = avg_sse2_8x2(&nColor02, &nColor03, &nColor12, &nColor13);

            _mm_storeu_si128(pTarget, avg_sse2_8x2(&nA1, &nA2, &nA3, &nA4));
            pTarget++;
        }
    }
}

void scaleHalf32_SSE2(ScaleContext& rContext, long nStartY, long nEndY)
{
    constexpr int constNumberSamples = 2;

    const long nSourceW = rContext.mnSourceW;

    __m128i nColor00;
    __m128i nColor01;
    __m128i nColor10;
    __m128i nColor11;

    for (long nY = nStartY; nY <= nEndY; nY++)
    {
        long nSourceY1 = nY * constNumberSamples;
        long nSourceY2 = nSourceY1 + 1;

        __m128i* pSource0 = reinterpret_cast<__m128i*>(rContext.mpSource->GetScanline(nSourceY1));
        __m128i* pSource1 = reinterpret_cast<__m128i*>(rContext.mpSource->GetScanline(nSourceY2));

        __m128i* pTarget = reinterpret_cast<__m128i*>(rContext.mpTarget->GetScanline(nY));

        for (long nSourceX = 0; nSourceX < nSourceW; nSourceX += constNumberSamples * 4)
        {
            nColor00 = _mm_loadu_si128(pSource0);
            pSource0++;
            nColor01 = _mm_loadu_si128(pSource0);
            pSource0++;
            nColor10 = _mm_loadu_si128(pSource1);
            pSource1++;
            nColor11 = _mm_loadu_si128(pSource1);
            pSource1++;

            _mm_storeu_si128(pTarget, avg_sse2_8x2(&nColor00, &nColor01, &nColor10, &nColor11));
            pTarget++;
        }
    }
}

void scaleHalf32_2(ScaleContext& rContext, long nStartY, long nEndY)
{
    constexpr int constNumberSamples = 2;

    const long nSourceW = rContext.mnSourceW;

    sal_uInt32 nColor00;
    sal_uInt32 nColor01;
    sal_uInt32 nColor10;
    sal_uInt32 nColor11;
    for (long nY = nStartY; nY <= nEndY; nY++)
    {
        long nSourceY1 = nY * constNumberSamples;
        long nSourceY2 = nSourceY1 + 1;

        sal_uInt32* pSource0
            = reinterpret_cast<sal_uInt32*>(rContext.mpSource->GetScanline(nSourceY1));
        sal_uInt32* pSource1
            = reinterpret_cast<sal_uInt32*>(rContext.mpSource->GetScanline(nSourceY2));
        sal_uInt32* pTarget = reinterpret_cast<sal_uInt32*>(rContext.mpTarget->GetScanline(nY));

        for (long nSourceX = 0; nSourceX < nSourceW; nSourceX += constNumberSamples)
        {
            nColor00 = *pSource0;
            pSource0++;
            nColor01 = *pSource0;
            pSource0++;
            nColor10 = *pSource1;
            pSource1++;
            nColor11 = *pSource1;
            pSource1++;

            *pTarget = Avg2x2(nColor00, nColor01, nColor10, nColor11);
            pTarget++;
        }
    }
}

void scaleHalf32_1(ScaleContext& rContext, long nStartY, long nEndY)
{
    constexpr int constColorComponents = 4;
    constexpr int constNumberSamples = 2;
    constexpr int constNumberSamplesSquared = (constNumberSamples * constNumberSamples);

    const long nStartX = 0;
    const long nEndX = rContext.mnTargetW - 1;

    long nComponent1 = 0;
    long nComponent2 = 0;
    long nComponent3 = 0;
    long nComponent4 = 0;

    for (long nY = nStartY; nY <= nEndY; nY++)
    {
        long nSourceY = nY * constNumberSamples;

        Scanline pSource0 = rContext.mpSource->GetScanline(nSourceY++);
        Scanline pSource1 = rContext.mpSource->GetScanline(nSourceY);

        Scanline pTargetScanline = rContext.mpTarget->GetScanline(nY);

        Scanline pColorPtr;

        for (long nX = nStartX; nX <= nEndX; nX++)
        {
            nComponent1 = 0;
            nComponent2 = 0;
            nComponent3 = 0;
            nComponent4 = 0;

            long nSourceX = nX * constNumberSamples;

            pColorPtr = pSource0 + constColorComponents * nSourceX;
            for (long i = 0; i < constNumberSamples; i++)
            {
                nComponent1 += *pColorPtr;
                pColorPtr++;
                nComponent2 += *pColorPtr;
                pColorPtr++;
                nComponent3 += *pColorPtr;
                pColorPtr++;
                nComponent4 += *pColorPtr;
                pColorPtr++;
            }

            pColorPtr = pSource1 + constColorComponents * nSourceX;
            for (long i = 0; i < constNumberSamples; i++)
            {
                nComponent1 += *pColorPtr;
                pColorPtr++;
                nComponent2 += *pColorPtr;
                pColorPtr++;
                nComponent3 += *pColorPtr;
                pColorPtr++;
                nComponent4 += *pColorPtr;
                pColorPtr++;
            }

            *pTargetScanline = nComponent1 / constNumberSamplesSquared;
            pTargetScanline++;
            *pTargetScanline = nComponent2 / constNumberSamplesSquared;
            pTargetScanline++;
            *pTargetScanline = nComponent3 / constNumberSamplesSquared;
            pTargetScanline++;
            *pTargetScanline = nComponent4 / constNumberSamplesSquared;
            pTargetScanline++;
        }
    }
}

template <int constColorComponents>
void scaleHalf32or24(ScaleContext& rContext, long nStartY, long nEndY)
{
    constexpr int constNumberSamples = 2;
    constexpr int constNumberSamplesSquared = constNumberSamples * constNumberSamples;

    const long nSourceW = rContext.mnSourceW;

    std::array<long, constColorComponents> aComponents;

    for (long nY = nStartY; nY <= nEndY; nY++)
    {
        long nSourceY1 = nY * constNumberSamples;
        long nSourceY2 = nSourceY1 + 1;

        Scanline pSource0 = rContext.mpSource->GetScanline(nSourceY1);
        Scanline pSource1 = rContext.mpSource->GetScanline(nSourceY2);
        Scanline pTargetScanline = rContext.mpTarget->GetScanline(nY);

        long nSourceX;
        for (nSourceX = 0; nSourceX < nSourceW; nSourceX += constNumberSamples)
        {
            aComponents.fill(0);
            long i;
            long c;

            for (i = 0; i < constNumberSamples; i++)
            {
                for (c = 0; c < constColorComponents; c++)
                {
                    aComponents[c] += *pSource0;
                    pSource0++;
                }
            }

            for (i = 0; i < constNumberSamples; i++)
            {
                for (c = 0; c < constColorComponents; c++)
                {
                    aComponents[c] += *pSource1;
                    pSource1++;
                }
            }

            for (c = 0; c < constColorComponents; c++)
            {
                *pTargetScanline = aComponents[c] / constNumberSamplesSquared;
                pTargetScanline++;
            }
        }
    }
}

void scaleHalf32(ScaleContext& rContext, long nStartY, long nEndY)
{
    scaleHalf32or24<4>(rContext, nStartY, nEndY);
}

void scaleHalf24(ScaleContext& rContext, long nStartY, long nEndY)
{
    scaleHalf32or24<3>(rContext, nStartY, nEndY);
}

void scaleOctal24(ScaleContext& rContext, long nStartY, long nEndY)
{
    constexpr int constColorComponents = 3;
    constexpr int constNumberSamples = 8;
    constexpr int constNumberSamplesSquared = (constNumberSamples * constNumberSamples);

    const long nStartX = 0;
    const long nEndX = rContext.mnTargetW - 1;

    for (long nY = nStartY; nY <= nEndY; nY++)
    {
        long nSourceY = nY * constNumberSamples;

        Scanline pSource0 = rContext.mpSource->GetScanline(nSourceY++);
        Scanline pSource1 = rContext.mpSource->GetScanline(nSourceY++);
        Scanline pSource2 = rContext.mpSource->GetScanline(nSourceY++);
        Scanline pSource3 = rContext.mpSource->GetScanline(nSourceY++);
        Scanline pSource4 = rContext.mpSource->GetScanline(nSourceY++);
        Scanline pSource5 = rContext.mpSource->GetScanline(nSourceY++);
        Scanline pSource6 = rContext.mpSource->GetScanline(nSourceY++);
        Scanline pSource7 = rContext.mpSource->GetScanline(nSourceY);

        Scanline pTargetScanline = rContext.mpTarget->GetScanline(nY);

        Scanline pColorPtr;

        for (long nX = nStartX; nX <= nEndX; nX++)
        {
            long nComponent1 = 0;
            long nComponent2 = 0;
            long nComponent3 = 0;

            long nSourceX = nX * constNumberSamples;

            pColorPtr = pSource0 + constColorComponents * nSourceX;
            for (long i = 0; i < constNumberSamples; i++)
            {
                nComponent1 += *pColorPtr;
                pColorPtr++;
                nComponent2 += *pColorPtr;
                pColorPtr++;
                nComponent3 += *pColorPtr;
                pColorPtr++;
            }

            pColorPtr = pSource1 + constColorComponents * nSourceX;
            for (long i = 0; i < constNumberSamples; i++)
            {
                nComponent1 += *pColorPtr;
                pColorPtr++;
                nComponent2 += *pColorPtr;
                pColorPtr++;
                nComponent3 += *pColorPtr;
                pColorPtr++;
            }

            pColorPtr = pSource2 + constColorComponents * nSourceX;
            for (long i = 0; i < constNumberSamples; i++)
            {
                nComponent1 += *pColorPtr;
                pColorPtr++;
                nComponent2 += *pColorPtr;
                pColorPtr++;
                nComponent3 += *pColorPtr;
                pColorPtr++;
            }

            pColorPtr = pSource3 + constColorComponents * nSourceX;
            for (long i = 0; i < constNumberSamples; i++)
            {
                nComponent1 += *pColorPtr;
                pColorPtr++;
                nComponent2 += *pColorPtr;
                pColorPtr++;
                nComponent3 += *pColorPtr;
                pColorPtr++;
            }

            pColorPtr = pSource4 + constColorComponents * nSourceX;
            for (long i = 0; i < constNumberSamples; i++)
            {
                nComponent1 += *pColorPtr;
                pColorPtr++;
                nComponent2 += *pColorPtr;
                pColorPtr++;
                nComponent3 += *pColorPtr;
                pColorPtr++;
            }

            pColorPtr = pSource5 + constColorComponents * nSourceX;
            for (long i = 0; i < constNumberSamples; i++)
            {
                nComponent1 += *pColorPtr;
                pColorPtr++;
                nComponent2 += *pColorPtr;
                pColorPtr++;
                nComponent3 += *pColorPtr;
                pColorPtr++;
            }

            pColorPtr = pSource6 + constColorComponents * nSourceX;
            for (long i = 0; i < constNumberSamples; i++)
            {
                nComponent1 += *pColorPtr;
                pColorPtr++;
                nComponent2 += *pColorPtr;
                pColorPtr++;
                nComponent3 += *pColorPtr;
                pColorPtr++;
            }

            pColorPtr = pSource7 + constColorComponents * nSourceX;
            for (long i = 0; i < constNumberSamples; i++)
            {
                nComponent1 += *pColorPtr;
                pColorPtr++;
                nComponent2 += *pColorPtr;
                pColorPtr++;
                nComponent3 += *pColorPtr;
                pColorPtr++;
            }

            *pTargetScanline = nComponent1 / constNumberSamplesSquared;
            pTargetScanline++;
            *pTargetScanline = nComponent2 / constNumberSamplesSquared;
            pTargetScanline++;
            *pTargetScanline = nComponent3 / constNumberSamplesSquared;
            pTargetScanline++;
        }
    }
}

void scaleQuarter24(ScaleContext& rContext, long nStartY, long nEndY)
{
    constexpr int constColorComponents = 3;
    constexpr int constNumberSamples = 4;
    constexpr int constNumberSamplesSquared = (constNumberSamples * constNumberSamples);

    const long nStartX = 0;
    const long nEndX = rContext.mnTargetW - 1;

    for (long nY = nStartY; nY <= nEndY; nY++)
    {
        long nSourceY = nY * constNumberSamples;

        Scanline pSource0 = rContext.mpSource->GetScanline(nSourceY++);
        Scanline pSource1 = rContext.mpSource->GetScanline(nSourceY++);
        Scanline pSource2 = rContext.mpSource->GetScanline(nSourceY++);
        Scanline pSource3 = rContext.mpSource->GetScanline(nSourceY++);

        Scanline pTargetScanline = rContext.mpTarget->GetScanline(nY);

        Scanline pColorPtr;

        for (long nX = nStartX; nX <= nEndX; nX++)
        {
            long nComponent1 = 0;
            long nComponent2 = 0;
            long nComponent3 = 0;

            long nSourceX = nX * constNumberSamples;

            pColorPtr = pSource0 + constColorComponents * nSourceX;
            for (long i = 0; i < constNumberSamples; i++)
            {
                nComponent1 += *pColorPtr;
                pColorPtr++;
                nComponent2 += *pColorPtr;
                pColorPtr++;
                nComponent3 += *pColorPtr;
                pColorPtr++;
            }

            pColorPtr = pSource1 + constColorComponents * nSourceX;
            for (long i = 0; i < constNumberSamples; i++)
            {
                nComponent1 += *pColorPtr;
                pColorPtr++;
                nComponent2 += *pColorPtr;
                pColorPtr++;
                nComponent3 += *pColorPtr;
                pColorPtr++;
            }

            pColorPtr = pSource2 + constColorComponents * nSourceX;
            for (long i = 0; i < constNumberSamples; i++)
            {
                nComponent1 += *pColorPtr;
                pColorPtr++;
                nComponent2 += *pColorPtr;
                pColorPtr++;
                nComponent3 += *pColorPtr;
                pColorPtr++;
            }

            pColorPtr = pSource3 + constColorComponents * nSourceX;
            for (long i = 0; i < constNumberSamples; i++)
            {
                nComponent1 += *pColorPtr;
                pColorPtr++;
                nComponent2 += *pColorPtr;
                pColorPtr++;
                nComponent3 += *pColorPtr;
                pColorPtr++;
            }

            *pTargetScanline = nComponent1 / constNumberSamplesSquared;
            pTargetScanline++;
            *pTargetScanline = nComponent2 / constNumberSamplesSquared;
            pTargetScanline++;
            *pTargetScanline = nComponent3 / constNumberSamplesSquared;
            pTargetScanline++;
        }
    }
}

} // namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
