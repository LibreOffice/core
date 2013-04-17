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

#if defined _MSC_VER
#pragma warning(push, 1)
#endif

#include <gdiplus.h>
#include <gdiplusenums.h>
#include <gdipluscolor.h>

#if defined _MSC_VER
#pragma warning(pop)
#endif

#include <basegfx/polygon/b2dpolygon.hxx>

// -----------------------------------------------------------------------

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
        Gdiplus::DllExports::GdipCreateFromHDC(mhDC, &pGraphics);
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
        Gdiplus::DllExports::GdipCreateFromHDC(mhDC, &pGraphics);
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

// -----------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
