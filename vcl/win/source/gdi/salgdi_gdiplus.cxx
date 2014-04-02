/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <stdio.h>
#include <string.h>
#include <svsys.h>
#include <win/wincomp.hxx>
#include <win/saldata.hxx>
#include <win/salgdi.h>
#include <win/salbmp.h>

#if defined _MSC_VER
#ifndef min
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#endif
#endif

#if defined _MSC_VER
#pragma warning(push, 1)
#endif

#ifdef __MINGW32__
#ifdef GetObject
#undef GetObject
#endif
#define GetObject GetObjectA
#endif

#include <gdiplus.h>
#include <gdiplusenums.h>
#include <gdipluscolor.h>

#ifdef __MINGW32__
#ifdef GetObject
#undef GetObject
#endif
#endif

#if defined _MSC_VER
#pragma warning(pop)
#endif

#include <basegfx/polygon/b2dpolygon.hxx>

void impAddB2DPolygonToGDIPlusGraphicsPathReal(Gdiplus::GpPath *pPath, const basegfx::B2DPolygon& rPolygon, bool bNoLineJoin)
{
    sal_uInt32 nCount(rPolygon.count());

    if(nCount)
    {
        const sal_uInt32 nEdgeCount(rPolygon.isClosed() ? nCount : nCount - 1);
        const bool bControls(rPolygon.areControlPointsUsed());
        basegfx::B2DPoint aCurr(rPolygon.getB2DPoint(0));

        for(sal_uInt32 a(0); a < nEdgeCount; a++)
        {
            const sal_uInt32 nNextIndex((a + 1) % nCount);
            const basegfx::B2DPoint aNext(rPolygon.getB2DPoint(nNextIndex));

            if(bControls && (rPolygon.isNextControlPointUsed(a) || rPolygon.isPrevControlPointUsed(nNextIndex)))
            {
                const basegfx::B2DPoint aCa(rPolygon.getNextControlPoint(a));
                const basegfx::B2DPoint aCb(rPolygon.getPrevControlPoint(nNextIndex));

                Gdiplus::DllExports::GdipAddPathBezier(pPath,
                    aCurr.getX(), aCurr.getY(),
                    aCa.getX(), aCa.getY(),
                    aCb.getX(), aCb.getY(),
                    aNext.getX(), aNext.getY());
            }
            else
            {
                Gdiplus::DllExports::GdipAddPathLine(pPath, aCurr.getX(), aCurr.getY(), aNext.getX(), aNext.getY());
            }

            if(a + 1 < nEdgeCount)
            {
                aCurr = aNext;

                if(bNoLineJoin)
                {
                    Gdiplus::DllExports::GdipStartPathFigure(pPath);
                }
            }
        }
    }
}

void impAddB2DPolygonToGDIPlusGraphicsPathInteger(Gdiplus::GpPath *pPath, const basegfx::B2DPolygon& rPolygon, bool bNoLineJoin)
{
    sal_uInt32 nCount(rPolygon.count());

    if(nCount)
    {
        const sal_uInt32 nEdgeCount(rPolygon.isClosed() ? nCount : nCount - 1);
        const bool bControls(rPolygon.areControlPointsUsed());
        basegfx::B2DPoint aCurr(rPolygon.getB2DPoint(0));

        for(sal_uInt32 a(0); a < nEdgeCount; a++)
        {
            const sal_uInt32 nNextIndex((a + 1) % nCount);
            const basegfx::B2DPoint aNext(rPolygon.getB2DPoint(nNextIndex));

            if(bControls && (rPolygon.isNextControlPointUsed(a) || rPolygon.isPrevControlPointUsed(nNextIndex)))
            {
                const basegfx::B2DPoint aCa(rPolygon.getNextControlPoint(a));
                const basegfx::B2DPoint aCb(rPolygon.getPrevControlPoint(nNextIndex));

                Gdiplus::DllExports::GdipAddPathBezier(
                    pPath,
                    aCurr.getX(), aCurr.getY(),
                    aCa.getX(), aCa.getY(),
                    aCb.getX(), aCb.getY(),
                    aNext.getX(), aNext.getY());
            }
            else
            {
                Gdiplus::DllExports::GdipAddPathLine(pPath, aCurr.getX(), aCurr.getY(), aNext.getX(), aNext.getY());
            }

            if(a + 1 < nEdgeCount)
            {
                aCurr = aNext;

                if(bNoLineJoin)
                {
                    Gdiplus::DllExports::GdipStartPathFigure(pPath);
                }
            }
        }
    }
}

bool WinSalGraphics::drawPolyPolygon( const ::basegfx::B2DPolyPolygon& rPolyPolygon, double fTransparency)
{
    const sal_uInt32 nCount(rPolyPolygon.count());

    if(mbBrush && nCount && (fTransparency >= 0.0 && fTransparency < 1.0))
    {
        Gdiplus::GpGraphics *pGraphics = NULL;
        Gdiplus::DllExports::GdipCreateFromHDC(getHDC(), &pGraphics);
        const sal_uInt8 aTrans((sal_uInt8)255 - (sal_uInt8)basegfx::fround(fTransparency * 255.0));
        Gdiplus::Color aTestColor(aTrans, SALCOLOR_RED(maFillColor), SALCOLOR_GREEN(maFillColor), SALCOLOR_BLUE(maFillColor));
        Gdiplus::GpSolidFill *pTestBrush;
        Gdiplus::DllExports::GdipCreateSolidFill(aTestColor.GetValue(), &pTestBrush);
        Gdiplus::GpPath *pPath = NULL;
        Gdiplus::DllExports::GdipCreatePath(Gdiplus::FillModeAlternate, &pPath);

        for(sal_uInt32 a(0); a < nCount; a++)
        {
            if(0 != a)
            {
                Gdiplus::DllExports::GdipStartPathFigure(pPath); // #i101491# not needed for first run
            }

            impAddB2DPolygonToGDIPlusGraphicsPathReal(pPath, rPolyPolygon.getB2DPolygon(a), false);
            Gdiplus::DllExports::GdipClosePathFigure(pPath);
        }

        if(getAntiAliasB2DDraw())
        {
            Gdiplus::DllExports::GdipSetSmoothingMode(pGraphics, Gdiplus::SmoothingModeAntiAlias);
        }
        else
        {
            Gdiplus::DllExports::GdipSetSmoothingMode(pGraphics, Gdiplus::SmoothingModeNone);
        }

        if(mbPrinter)
        {
            // #i121591#
            // Normally GdiPlus should not be used for printing at all since printers cannot
            // print transparent filled polygon geometry and normally this does not happen
            // since OutputDevice::RemoveTransparenciesFromMetaFile is used as preparation
            // and no transparent parts should remain for printing. But this can be overridden
            // by the user and thus happens. This call can only come (currently) from
            // OutputDevice::DrawTransparent, see comments there with the same TaskID.
            // If it is used, the mapping for the printer is wrong and needs to be corrected. I
            // checked that there is *no* transformation set and estimated that a stable factor
            // dependent of the printer's DPI is used. Create and set a transformation here to
            // correct this.
            Gdiplus::REAL aDpiX;
            Gdiplus::DllExports::GdipGetDpiX(pGraphics, &aDpiX);
            Gdiplus::REAL aDpiY;
            Gdiplus::DllExports::GdipGetDpiY(pGraphics, &aDpiY);

            Gdiplus::DllExports::GdipResetWorldTransform(pGraphics);
            Gdiplus::DllExports::GdipScaleWorldTransform(pGraphics, Gdiplus::REAL(100.0) / aDpiX, Gdiplus::REAL(100.0) / aDpiY, Gdiplus::MatrixOrderAppend);
        }

        Gdiplus::DllExports::GdipFillPath(pGraphics, pTestBrush, pPath);

        Gdiplus::DllExports::GdipDeletePath(pPath);
        Gdiplus::DllExports::GdipDeleteGraphics(pGraphics);
    }

     return true;
}

bool WinSalGraphics::drawPolyLine(
    const basegfx::B2DPolygon& rPolygon,
    double fTransparency,
    const basegfx::B2DVector& rLineWidths,
    basegfx::B2DLineJoin eLineJoin,
    com::sun::star::drawing::LineCap eLineCap)
{
    const sal_uInt32 nCount(rPolygon.count());

    if(mbPen && nCount)
    {
        Gdiplus::GpGraphics *pGraphics = NULL;
        Gdiplus::DllExports::GdipCreateFromHDC(getHDC(), &pGraphics);
        const sal_uInt8 aTrans = (sal_uInt8)basegfx::fround( 255 * (1.0 - fTransparency) );
        Gdiplus::Color aTestColor(aTrans, SALCOLOR_RED(maLineColor), SALCOLOR_GREEN(maLineColor), SALCOLOR_BLUE(maLineColor));
        Gdiplus::GpPen *pTestPen = NULL;
        Gdiplus::DllExports::GdipCreatePen1(aTestColor.GetValue(), Gdiplus::REAL(rLineWidths.getX()), Gdiplus::UnitWorld, &pTestPen);
        Gdiplus::GpPath *pPath;
        Gdiplus::DllExports::GdipCreatePath(Gdiplus::FillModeAlternate, &pPath);
        bool bNoLineJoin(false);

        switch(eLineJoin)
        {
            default : // basegfx::B2DLINEJOIN_NONE :
            {
                if(basegfx::fTools::more(rLineWidths.getX(), 0.0))
                {
                    bNoLineJoin = true;
                }
                break;
            }
            case basegfx::B2DLINEJOIN_BEVEL :
            {
                Gdiplus::DllExports::GdipSetPenLineJoin(pTestPen, Gdiplus::LineJoinBevel);
                break;
            }
            case basegfx::B2DLINEJOIN_MIDDLE :
            case basegfx::B2DLINEJOIN_MITER :
            {
                const Gdiplus::REAL aMiterLimit(15.0);
                Gdiplus::DllExports::GdipSetPenMiterLimit(pTestPen, aMiterLimit);
                Gdiplus::DllExports::GdipSetPenLineJoin(pTestPen, Gdiplus::LineJoinMiter);
                break;
            }
            case basegfx::B2DLINEJOIN_ROUND :
            {
                Gdiplus::DllExports::GdipSetPenLineJoin(pTestPen, Gdiplus::LineJoinRound);
                break;
            }
        }

        switch(eLineCap)
        {
            default: /*com::sun::star::drawing::LineCap_BUTT*/
            {
                // nothing to do
                break;
            }
            case com::sun::star::drawing::LineCap_ROUND:
            {
                Gdiplus::DllExports::GdipSetPenStartCap(pTestPen, Gdiplus::LineCapRound);
                Gdiplus::DllExports::GdipSetPenEndCap(pTestPen, Gdiplus::LineCapRound);
                break;
            }
            case com::sun::star::drawing::LineCap_SQUARE:
            {
                Gdiplus::DllExports::GdipSetPenStartCap(pTestPen, Gdiplus::LineCapSquare);
                Gdiplus::DllExports::GdipSetPenEndCap(pTestPen, Gdiplus::LineCapSquare);
                break;
            }
        }

        if(nCount > 250 && basegfx::fTools::more(rLineWidths.getX(), 1.5))
        {
            impAddB2DPolygonToGDIPlusGraphicsPathInteger(pPath, rPolygon, bNoLineJoin);
        }
        else
        {
            impAddB2DPolygonToGDIPlusGraphicsPathReal(pPath, rPolygon, bNoLineJoin);
        }

        if(rPolygon.isClosed() && !bNoLineJoin)
        {
            // #i101491# needed to create the correct line joins
            Gdiplus::DllExports::GdipClosePathFigure(pPath);
        }

        if(getAntiAliasB2DDraw())
        {
            Gdiplus::DllExports::GdipSetSmoothingMode(pGraphics, Gdiplus::SmoothingModeAntiAlias);
        }
        else
        {
            Gdiplus::DllExports::GdipSetSmoothingMode(pGraphics, Gdiplus::SmoothingModeNone);
        }

        Gdiplus::DllExports::GdipDrawPath(pGraphics, pTestPen, pPath);

        Gdiplus::DllExports::GdipDeletePath(pPath);
        Gdiplus::DllExports::GdipDeletePen(pTestPen);
        Gdiplus::DllExports::GdipDeleteGraphics(pGraphics);
    }

    return true;
}

void paintToGdiPlus(
    Gdiplus::Graphics& rGraphics,
    const SalTwoRect& rTR,
    Gdiplus::Bitmap& rBitmap)
{
    // only parts of source are used
    Gdiplus::PointF aDestPoints[3];
    Gdiplus::ImageAttributes aAttributes;

    // define target region as paralellogram
    aDestPoints[0].X = Gdiplus::REAL(rTR.mnDestX);
    aDestPoints[0].Y = Gdiplus::REAL(rTR.mnDestY);
    aDestPoints[1].X = Gdiplus::REAL(rTR.mnDestX + rTR.mnDestWidth);
    aDestPoints[1].Y = Gdiplus::REAL(rTR.mnDestY);
    aDestPoints[2].X = Gdiplus::REAL(rTR.mnDestX);
    aDestPoints[2].Y = Gdiplus::REAL(rTR.mnDestY + rTR.mnDestHeight);

    aAttributes.SetWrapMode(Gdiplus::WrapModeTileFlipXY);

    rGraphics.DrawImage(
        &rBitmap,
        aDestPoints,
        3,
        Gdiplus::REAL(rTR.mnSrcX),
        Gdiplus::REAL(rTR.mnSrcY),
        Gdiplus::REAL(rTR.mnSrcWidth),
        Gdiplus::REAL(rTR.mnSrcHeight),
        Gdiplus::UnitPixel,
        &aAttributes,
        0,
        0);
}

void setInterpolationMode(
    Gdiplus::Graphics& rGraphics,
    const long& rSrcWidth,
    const long& rDestWidth,
    const long& rSrcHeight,
    const long& rDestHeight)
{
    const bool bSameWidth(rSrcWidth == rDestWidth);
    const bool bSameHeight(rSrcHeight == rDestHeight);

    if(bSameWidth && bSameHeight)
    {
#ifdef __MINGW32__
        //Gdiplus::InterpolationModeInvalid is missing on mingw
        rGraphics.SetInterpolationMode(Gdiplus::InterpolationModeDefault);
#else
        rGraphics.SetInterpolationMode(Gdiplus::InterpolationModeInvalid);
#endif
    }
    else if(rDestWidth > rSrcWidth && rDestHeight > rSrcHeight)
    {
        rGraphics.SetInterpolationMode(Gdiplus::InterpolationModeDefault);
    }
    else if(rDestWidth < rSrcWidth && rDestHeight < rSrcHeight)
    {
        rGraphics.SetInterpolationMode(Gdiplus::InterpolationModeBicubic);
    }
    else
    {
        rGraphics.SetInterpolationMode(Gdiplus::InterpolationModeDefault);
    }
}

bool WinSalGraphics::tryDrawBitmapGdiPlus(const SalTwoRect& rTR, const SalBitmap& rSrcBitmap)
{
    if(rTR.mnSrcWidth && rTR.mnSrcHeight && rTR.mnDestWidth && rTR.mnDestHeight)
    {
        const WinSalBitmap& rSalBitmap = static_cast< const WinSalBitmap& >(rSrcBitmap);
        GdiPlusBmpPtr aARGB(rSalBitmap.ImplGetGdiPlusBitmap());

        if(aARGB.get())
        {
            Gdiplus::Graphics aGraphics(getHDC());

            setInterpolationMode(
                aGraphics,
                rTR.mnSrcWidth,
                rTR.mnDestWidth,
                rTR.mnSrcHeight,
                rTR.mnDestHeight);

            paintToGdiPlus(
                aGraphics,
                rTR,
                *aARGB.get());

            return true;
        }
    }

    return false;
}

bool WinSalGraphics::drawAlphaBitmap(
    const SalTwoRect& rTR,
    const SalBitmap& rSrcBitmap,
    const SalBitmap& rAlphaBmp)
{
    if(rTR.mnSrcWidth && rTR.mnSrcHeight && rTR.mnDestWidth && rTR.mnDestHeight)
    {
        const WinSalBitmap& rSalBitmap = static_cast< const WinSalBitmap& >(rSrcBitmap);
        const WinSalBitmap& rSalAlpha = static_cast< const WinSalBitmap& >(rAlphaBmp);
        GdiPlusBmpPtr aARGB(rSalBitmap.ImplGetGdiPlusBitmap(&rSalAlpha));

        if(aARGB.get())
        {
            Gdiplus::Graphics aGraphics(getHDC());

            setInterpolationMode(
                aGraphics,
                rTR.mnSrcWidth,
                rTR.mnDestWidth,
                rTR.mnSrcHeight,
                rTR.mnDestHeight);

            paintToGdiPlus(
                aGraphics,
                rTR,
                *aARGB.get());

            return true;
        }
    }

    return false;
}

bool WinSalGraphics::drawTransformedBitmap(
    const basegfx::B2DPoint& rNull,
    const basegfx::B2DPoint& rX,
    const basegfx::B2DPoint& rY,
    const SalBitmap& rSourceBitmap,
    const SalBitmap* pAlphaBitmap)
{
    const WinSalBitmap& rSalBitmap = static_cast< const WinSalBitmap& >(rSourceBitmap);
    const WinSalBitmap* pSalAlpha = static_cast< const WinSalBitmap* >(pAlphaBitmap);
    GdiPlusBmpPtr aARGB(rSalBitmap.ImplGetGdiPlusBitmap(pSalAlpha));

    if(aARGB.get())
    {
        const long nSrcWidth(aARGB->GetWidth());
        const long nSrcHeight(aARGB->GetHeight());

        if(nSrcWidth && nSrcHeight)
        {
            const long nDestWidth(basegfx::fround(basegfx::B2DVector(rX - rNull).getLength()));
            const long nDestHeight(basegfx::fround(basegfx::B2DVector(rY - rNull).getLength()));

            if(nDestWidth && nDestHeight)
            {
                Gdiplus::Graphics aGraphics(getHDC());
                Gdiplus::PointF aDestPoints[3];
                Gdiplus::ImageAttributes aAttributes;

                setInterpolationMode(
                    aGraphics,
                    nSrcWidth,
                    nDestWidth,
                    nSrcHeight,
                    nDestHeight);

                // this mode is only capable of drawing the whole bitmap to a paralellogram
                aDestPoints[0].X = Gdiplus::REAL(rNull.getX());
                aDestPoints[0].Y = Gdiplus::REAL(rNull.getY());
                aDestPoints[1].X = Gdiplus::REAL(rX.getX());
                aDestPoints[1].Y = Gdiplus::REAL(rX.getY());
                aDestPoints[2].X = Gdiplus::REAL(rY.getX());
                aDestPoints[2].Y = Gdiplus::REAL(rY.getY());

                aAttributes.SetWrapMode(Gdiplus::WrapModeTileFlipXY);

                aGraphics.DrawImage(
                    aARGB.get(),
                    aDestPoints,
                    3,
                    Gdiplus::REAL(0.0),
                    Gdiplus::REAL(0.0),
                    Gdiplus::REAL(nSrcWidth),
                    Gdiplus::REAL(nSrcHeight),
                    Gdiplus::UnitPixel,
                    &aAttributes,
                    0,
                    0);
            }
        }

        return true;
    }

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
