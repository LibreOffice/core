/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <stdio.h>
#include <string.h>
#include <svsys.h>
#include <wincomp.hxx>
#include <saldata.hxx>
#include <salgdi.h>
#include <tools/debug.hxx>

#ifndef min
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#endif

#if defined _MSC_VER
#pragma warning(push, 1)
#endif

#include <GdiPlus.h>
#include <GdiPlusEnums.h>
#include <GdiPlusColor.h>

#if defined _MSC_VER
#pragma warning(pop)
#endif

#include <basegfx/polygon/b2dpolygon.hxx>

// -----------------------------------------------------------------------

void impAddB2DPolygonToGDIPlusGraphicsPathReal(Gdiplus::GraphicsPath& rPath, const basegfx::B2DPolygon& rPolygon, bool bNoLineJoin)
{
    sal_uInt32 nCount(rPolygon.count());

    if(nCount)
    {
        const sal_uInt32 nEdgeCount(rPolygon.isClosed() ? nCount : nCount - 1);
        const bool bControls(rPolygon.areControlPointsUsed());
        basegfx::B2DPoint aCurr(rPolygon.getB2DPoint(0));
        Gdiplus::PointF aFCurr(Gdiplus::REAL(aCurr.getX()), Gdiplus::REAL(aCurr.getY()));

        for(sal_uInt32 a(0); a < nEdgeCount; a++)
        {
            const sal_uInt32 nNextIndex((a + 1) % nCount);
            const basegfx::B2DPoint aNext(rPolygon.getB2DPoint(nNextIndex));
            const Gdiplus::PointF aFNext(Gdiplus::REAL(aNext.getX()), Gdiplus::REAL(aNext.getY()));

            if(bControls && (rPolygon.isNextControlPointUsed(a) || rPolygon.isPrevControlPointUsed(nNextIndex)))
            {
                const basegfx::B2DPoint aCa(rPolygon.getNextControlPoint(a));
                const basegfx::B2DPoint aCb(rPolygon.getPrevControlPoint(nNextIndex));

                rPath.AddBezier(
                    aFCurr,
                    Gdiplus::PointF(Gdiplus::REAL(aCa.getX()), Gdiplus::REAL(aCa.getY())),
                    Gdiplus::PointF(Gdiplus::REAL(aCb.getX()), Gdiplus::REAL(aCb.getY())),
                    aFNext);
            }
            else
            {
                rPath.AddLine(aFCurr, aFNext);
            }

            if(a + 1 < nEdgeCount)
            {
                aFCurr = aFNext;

                if(bNoLineJoin)
                {
                    rPath.StartFigure();
                }
            }
        }
    }
}

void impAddB2DPolygonToGDIPlusGraphicsPathInteger(Gdiplus::GraphicsPath& rPath, const basegfx::B2DPolygon& rPolygon, bool bNoLineJoin)
{
    sal_uInt32 nCount(rPolygon.count());

    if(nCount)
    {
        const sal_uInt32 nEdgeCount(rPolygon.isClosed() ? nCount : nCount - 1);
        const bool bControls(rPolygon.areControlPointsUsed());
        basegfx::B2DPoint aCurr(rPolygon.getB2DPoint(0));
        Gdiplus::Point aICurr(INT(aCurr.getX()), INT(aCurr.getY()));

        for(sal_uInt32 a(0); a < nEdgeCount; a++)
        {
            const sal_uInt32 nNextIndex((a + 1) % nCount);
            const basegfx::B2DPoint aNext(rPolygon.getB2DPoint(nNextIndex));
            const Gdiplus::Point aINext(INT(aNext.getX()), INT(aNext.getY()));

            if(bControls && (rPolygon.isNextControlPointUsed(a) || rPolygon.isPrevControlPointUsed(nNextIndex)))
            {
                const basegfx::B2DPoint aCa(rPolygon.getNextControlPoint(a));
                const basegfx::B2DPoint aCb(rPolygon.getPrevControlPoint(nNextIndex));

                rPath.AddBezier(
                    aICurr,
                    Gdiplus::Point(INT(aCa.getX()), INT(aCa.getY())),
                    Gdiplus::Point(INT(aCb.getX()), INT(aCb.getY())),
                    aINext);
            }
            else
            {
                rPath.AddLine(aICurr, aINext);
            }

            if(a + 1 < nEdgeCount)
            {
                aICurr = aINext;

                if(bNoLineJoin)
                {
                    rPath.StartFigure();
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
        Gdiplus::Graphics aGraphics(mhDC);
        const sal_uInt8 aTrans((sal_uInt8)255 - (sal_uInt8)basegfx::fround(fTransparency * 255.0));
        Gdiplus::Color aTestColor(aTrans, SALCOLOR_RED(maFillColor), SALCOLOR_GREEN(maFillColor), SALCOLOR_BLUE(maFillColor));
        Gdiplus::SolidBrush aTestBrush(aTestColor);
        Gdiplus::GraphicsPath aPath;

        for(sal_uInt32 a(0); a < nCount; a++)
        {
            if(0 != a)
            {
                aPath.StartFigure(); // #i101491# not needed for first run
            }

            impAddB2DPolygonToGDIPlusGraphicsPathReal(aPath, rPolyPolygon.getB2DPolygon(a), false);
            aPath.CloseFigure();
        }

        if(getAntiAliasB2DDraw())
        {
            aGraphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
        }
        else
        {
            aGraphics.SetSmoothingMode(Gdiplus::SmoothingModeNone);
        }

        aGraphics.FillPath(&aTestBrush, &aPath);
    }

     return true;
}

bool WinSalGraphics::drawPolyLine( const basegfx::B2DPolygon& rPolygon, double fTransparency, const basegfx::B2DVector& rLineWidths, basegfx::B2DLineJoin eLineJoin )
{
    const sal_uInt32 nCount(rPolygon.count());

    if(mbPen && nCount)
    {
        Gdiplus::Graphics aGraphics(mhDC);
        const sal_uInt8 aTrans = (sal_uInt8)basegfx::fround( 255 * (1.0 - fTransparency) );
        Gdiplus::Color aTestColor(aTrans, SALCOLOR_RED(maLineColor), SALCOLOR_GREEN(maLineColor), SALCOLOR_BLUE(maLineColor));
        Gdiplus::Pen aTestPen(aTestColor, Gdiplus::REAL(rLineWidths.getX()));
        Gdiplus::GraphicsPath aPath;
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
                aTestPen.SetLineJoin(Gdiplus::LineJoinBevel);
                break;
            }
            case basegfx::B2DLINEJOIN_MIDDLE :
            case basegfx::B2DLINEJOIN_MITER :
            {
                const Gdiplus::REAL aMiterLimit(15.0);
                aTestPen.SetMiterLimit(aMiterLimit);
                aTestPen.SetLineJoin(Gdiplus::LineJoinMiter);
                break;
            }
            case basegfx::B2DLINEJOIN_ROUND :
            {
                aTestPen.SetLineJoin(Gdiplus::LineJoinRound);
                break;
            }
        }

        if(nCount > 250 && basegfx::fTools::more(rLineWidths.getX(), 1.5))
        {
            impAddB2DPolygonToGDIPlusGraphicsPathInteger(aPath, rPolygon, bNoLineJoin);
        }
        else
        {
            impAddB2DPolygonToGDIPlusGraphicsPathReal(aPath, rPolygon, bNoLineJoin);
        }

        if(rPolygon.isClosed() && !bNoLineJoin)
        {
            // #i101491# needed to create the correct line joins
            aPath.CloseFigure();
        }

        if(getAntiAliasB2DDraw())
        {
            aGraphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
        }
        else
        {
            aGraphics.SetSmoothingMode(Gdiplus::SmoothingModeNone);
        }

        aGraphics.DrawPath(&aTestPen, &aPath);
    }

    return true;
}

// -----------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
