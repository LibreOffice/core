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

#include <vcl/BitmapWriteAccess.hxx>

#if defined(_MSC_VER)
#define VCL_FORCEINLINE __forceinline
#elif defined(__GNUC__)
#define VCL_FORCEINLINE __attribute__((always_inline)) inline
#else
#define VCL_FORCEINLINE inline
#endif

namespace vcl
{
struct VCL_DLLPUBLIC ScaleContext
{
    BitmapReadAccess const* const mpSource;
    BitmapWriteAccess* mpTarget;
    long mnSourceW;
    long mnSourceH;
    long mnTargetW;
    long mnTargetH;
    bool mbHMirr;
    bool mbVMirr;

    VCL_FORCEINLINE Scanline getSourceScanline(long y) { return mpSource->GetScanline(y); }

    VCL_FORCEINLINE Scanline getTargetScanline(long y) { return mpTarget->GetScanline(y); }

    VCL_FORCEINLINE BitmapColor getSourcePixel(Scanline const& pScanline, long x)
    {
        return mpSource->GetPixelFromData(pScanline, x);
    }

    VCL_FORCEINLINE void setTargetPixel(Scanline const& pScanline, long x, BitmapColor& rColor)
    {
        return mpTarget->SetPixelOnData(pScanline, x, rColor);
    }

    ScaleContext(BitmapReadAccess* pSource, BitmapWriteAccess* pTarget, long nSourceW,
                 long nTargetW, long nSourceH, long nTargetH, bool bHMirr, bool bVMirr)
        : mpSource(pSource)
        , mpTarget(pTarget)
        , mnSourceW(nSourceW)
        , mnSourceH(nSourceH)
        , mnTargetW(nTargetW)
        , mnTargetH(nTargetH)
        , mbHMirr(bHMirr)
        , mbVMirr(bVMirr)
    {
    }
};

void scaleHalfGeneralHorizontal(ScaleContext& rContext, long nStartY, long nEndY);
void scaleHalfGeneralVertical(ScaleContext& rContext, long nStartY, long nEndY);
void scaleHalfGeneral(ScaleContext& rContext, long nStartY, long nEndY);
void scaleQuarterGeneral(ScaleContext& rContext, long nStartY, long nEndY);
void scaleQuarterGeneralHorizontal(ScaleContext& rContext, long nStartY, long nEndY);
void scaleQuarterGeneralVertical(ScaleContext& rContext, long nStartY, long nEndY);
void scaleOctalGeneral(ScaleContext& rContext, long nStartY, long nEndY);

void scaleOctal32(ScaleContext& rContext, long nStartY, long nEndY);
void scaleQuarter32_1(ScaleContext& rContext, long nStartY, long nEndY);
void scaleQuarter32_2(ScaleContext& rContext, long nStartY, long nEndY);
void scaleQuarter32_SSE(ScaleContext& rContext, long nStartY, long nEndY);
void scaleHalf32_SSE2(ScaleContext& rContext, long nStartY, long nEndY);
void scaleHalf32_2(ScaleContext& rContext, long nStartY, long nEndY);
void scaleHalf32_1(ScaleContext& rContext, long nStartY, long nEndY);

void scaleHalf24(ScaleContext& rContext, long nStartY, long nEndY);
void scaleHalf32(ScaleContext& rContext, long nStartY, long nEndY);

void scaleOctal24(ScaleContext& rContext, long nStartY, long nEndY);
void scaleQuarter24(ScaleContext& rContext, long nStartY, long nEndY);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
