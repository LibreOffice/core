/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <sal/config.h>
#include <sal/log.hxx>

#include <cassert>
#include <cstring>
#include <numeric>
#include <utility>

#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <osl/endian.h>
#include <osl/file.hxx>
#include <sal/types.h>
#include <tools/long.hxx>
#include <vcl/sysdata.hxx>

#include <fontsubset.hxx>
#include <quartz/salbmp.h>
#ifdef MACOSX
#include <quartz/salgdi.h>
#endif
#include <quartz/utils.h>
#ifdef IOS
#include "saldatabasic.hxx"
#endif
#include <sft.hxx>

using namespace vcl;

namespace
{
const basegfx::B2DPoint aHalfPointOfs(0.5, 0.5);

void AddPolygonToPath(CGMutablePathRef xPath, const basegfx::B2DPolygon& rPolygon, bool bClosePath,
                      bool bPixelSnap, bool bLineDraw)
{
    // short circuit if there is nothing to do
    const int nPointCount = rPolygon.count();
    if (nPointCount <= 0)
    {
        return;
    }

    const bool bHasCurves = rPolygon.areControlPointsUsed();
    for (int nPointIdx = 0, nPrevIdx = 0;; nPrevIdx = nPointIdx++)
    {
        int nClosedIdx = nPointIdx;
        if (nPointIdx >= nPointCount)
        {
            // prepare to close last curve segment if needed
            if (bClosePath && (nPointIdx == nPointCount))
            {
                nClosedIdx = 0;
            }
            else
            {
                break;
            }
        }

        basegfx::B2DPoint aPoint = rPolygon.getB2DPoint(nClosedIdx);

        if (bPixelSnap)
        {
            // snap device coordinates to full pixels
            aPoint.setX(basegfx::fround(aPoint.getX()));
            aPoint.setY(basegfx::fround(aPoint.getY()));
        }

        if (bLineDraw)
        {
            aPoint += aHalfPointOfs;
        }
        if (!nPointIdx)
        {
            // first point => just move there
            CGPathMoveToPoint(xPath, nullptr, aPoint.getX(), aPoint.getY());
            continue;
        }

        bool bPendingCurve = false;
        if (bHasCurves)
        {
            bPendingCurve = rPolygon.isNextControlPointUsed(nPrevIdx);
            bPendingCurve |= rPolygon.isPrevControlPointUsed(nClosedIdx);
        }

        if (!bPendingCurve) // line segment
        {
            CGPathAddLineToPoint(xPath, nullptr, aPoint.getX(), aPoint.getY());
        }
        else // cubic bezier segment
        {
            basegfx::B2DPoint aCP1 = rPolygon.getNextControlPoint(nPrevIdx);
            basegfx::B2DPoint aCP2 = rPolygon.getPrevControlPoint(nClosedIdx);
            if (bLineDraw)
            {
                aCP1 += aHalfPointOfs;
                aCP2 += aHalfPointOfs;
            }
            CGPathAddCurveToPoint(xPath, nullptr, aCP1.getX(), aCP1.getY(), aCP2.getX(),
                                  aCP2.getY(), aPoint.getX(), aPoint.getY());
        }
    }

    if (bClosePath)
    {
        CGPathCloseSubpath(xPath);
    }
}

void alignLinePoint(const Point* i_pIn, float& o_fX, float& o_fY)
{
    o_fX = static_cast<float>(i_pIn->getX()) + 0.5;
    o_fY = static_cast<float>(i_pIn->getY()) + 0.5;
}

void getBoundRect(sal_uInt32 nPoints, const Point* pPtAry, tools::Long& rX, tools::Long& rY,
                  tools::Long& rWidth, tools::Long& rHeight)
{
    tools::Long nX1 = pPtAry->getX();
    tools::Long nX2 = nX1;
    tools::Long nY1 = pPtAry->getY();
    tools::Long nY2 = nY1;

    for (sal_uInt32 n = 1; n < nPoints; n++)
    {
        if (pPtAry[n].getX() < nX1)
        {
            nX1 = pPtAry[n].getX();
        }
        else if (pPtAry[n].getX() > nX2)
        {
            nX2 = pPtAry[n].getX();
        }
        if (pPtAry[n].getY() < nY1)
        {
            nY1 = pPtAry[n].getY();
        }
        else if (pPtAry[n].getY() > nY2)
        {
            nY2 = pPtAry[n].getY();
        }
    }
    rX = nX1;
    rY = nY1;
    rWidth = nX2 - nX1 + 1;
    rHeight = nY2 - nY1 + 1;
}

Color ImplGetROPColor(SalROPColor nROPColor)
{
    Color nColor;
    if (nROPColor == SalROPColor::N0)
    {
        nColor = Color(0, 0, 0);
    }
    else
    {
        nColor = Color(255, 255, 255);
    }
    return nColor;
}

void drawPattern50(void*, CGContextRef rContext)
{
    static const CGRect aRects[2] = { { { 0, 0 }, { 2, 2 } }, { { 2, 2 }, { 2, 2 } } };
    CGContextAddRects(rContext, aRects, 2);
    CGContextFillPath(rContext);
}
}

AquaGraphicsBackend::AquaGraphicsBackend(AquaSharedAttributes& rShared)
    : AquaGraphicsBackendBase(rShared)
{
}

AquaGraphicsBackend::~AquaGraphicsBackend() {}

void AquaGraphicsBackend::Init() {}
void AquaGraphicsBackend::freeResources() {}

bool AquaGraphicsBackend::setClipRegion(vcl::Region const& rRegion)
{
    // release old clip path
    mrShared.unsetClipPath();
    mrShared.mxClipPath = CGPathCreateMutable();

    // set current path, either as polypolgon or sequence of rectangles
    RectangleVector aRectangles;
    rRegion.GetRegionRectangles(aRectangles);

    for (const auto& rRect : aRectangles)
    {
        const tools::Long nW(rRect.Right() - rRect.Left() + 1); // uses +1 logic in original

        if (nW)
        {
            const tools::Long nH(rRect.Bottom() - rRect.Top() + 1); // uses +1 logic in original

            if (nH)
            {
                const CGRect aRect = CGRectMake(rRect.Left(), rRect.Top(), nW, nH);
                CGPathAddRect(mrShared.mxClipPath, nullptr, aRect);
            }
        }
    }
    // set the current path as clip region
    if (mrShared.checkContext())
        mrShared.setState();

    return true;
}

void AquaGraphicsBackend::ResetClipRegion()
{
    // release old path and indicate no clipping
    mrShared.unsetClipPath();

    if (mrShared.checkContext())
    {
        mrShared.setState();
    }
}

sal_uInt16 AquaGraphicsBackend::GetBitCount() const
{
    sal_uInt16 nBits = mrShared.mnBitmapDepth ? mrShared.mnBitmapDepth : 32; //24;
    return nBits;
}

tools::Long AquaGraphicsBackend::GetGraphicsWidth() const
{
    tools::Long width = 0;
    if (mrShared.maContextHolder.isSet()
        && (
#ifndef IOS
               mrShared.mbWindow ||
#endif
               mrShared.mbVirDev))
    {
        width = mrShared.mnWidth;
    }

#ifndef IOS
    if (width == 0)
    {
        if (mrShared.mbWindow && mrShared.mpFrame)
        {
            width = mrShared.mpFrame->maGeometry.nWidth;
        }
    }
#endif
    return width;
}

void AquaGraphicsBackend::SetLineColor()
{
    mrShared.maLineColor.SetAlpha(0.0); // transparent
    if (mrShared.checkContext())
    {
        CGContextSetRGBStrokeColor(mrShared.maContextHolder.get(), mrShared.maLineColor.GetRed(),
                                   mrShared.maLineColor.GetGreen(), mrShared.maLineColor.GetBlue(),
                                   mrShared.maLineColor.GetAlpha());
    }
}

void AquaGraphicsBackend::SetLineColor(Color nColor)
{
    mrShared.maLineColor = RGBAColor(nColor);
    if (mrShared.checkContext())
    {
        CGContextSetRGBStrokeColor(mrShared.maContextHolder.get(), mrShared.maLineColor.GetRed(),
                                   mrShared.maLineColor.GetGreen(), mrShared.maLineColor.GetBlue(),
                                   mrShared.maLineColor.GetAlpha());
    }
}

void AquaGraphicsBackend::SetFillColor()
{
    mrShared.maFillColor.SetAlpha(0.0); // transparent
    if (mrShared.checkContext())
    {
        CGContextSetRGBFillColor(mrShared.maContextHolder.get(), mrShared.maFillColor.GetRed(),
                                 mrShared.maFillColor.GetGreen(), mrShared.maFillColor.GetBlue(),
                                 mrShared.maFillColor.GetAlpha());
    }
}

void AquaGraphicsBackend::SetFillColor(Color nColor)
{
    mrShared.maFillColor = RGBAColor(nColor);
    if (mrShared.checkContext())
    {
        CGContextSetRGBFillColor(mrShared.maContextHolder.get(), mrShared.maFillColor.GetRed(),
                                 mrShared.maFillColor.GetGreen(), mrShared.maFillColor.GetBlue(),
                                 mrShared.maFillColor.GetAlpha());
    }
}

void AquaGraphicsBackend::SetXORMode(bool bSet, bool bInvertOnly)
{
    // return early if XOR mode remains unchanged
    if (mrShared.mbPrinter)
    {
        return;
    }
    if (!bSet && mrShared.mnXorMode == 2)
    {
        CGContextSetBlendMode(mrShared.maContextHolder.get(), kCGBlendModeNormal);
        mrShared.mnXorMode = 0;
        return;
    }
    else if (bSet && bInvertOnly && mrShared.mnXorMode == 0)
    {
        CGContextSetBlendMode(mrShared.maContextHolder.get(), kCGBlendModeDifference);
        mrShared.mnXorMode = 2;
        return;
    }

    if (!mrShared.mpXorEmulation && !bSet)
    {
        return;
    }
    if (mrShared.mpXorEmulation && bSet == mrShared.mpXorEmulation->IsEnabled())
    {
        return;
    }
    if (!mrShared.checkContext())
    {
        return;
    }
    // prepare XOR emulation
    if (!mrShared.mpXorEmulation)
    {
        mrShared.mpXorEmulation = std::make_unique<XorEmulation>();
        mrShared.mpXorEmulation->SetTarget(mrShared.mnWidth, mrShared.mnHeight,
                                           mrShared.mnBitmapDepth, mrShared.maContextHolder.get(),
                                           mrShared.maLayer.get());
    }

    // change the XOR mode
    if (bSet)
    {
        mrShared.mpXorEmulation->Enable();
        mrShared.maContextHolder.set(mrShared.mpXorEmulation->GetMaskContext());
        mrShared.mnXorMode = 1;
    }
    else
    {
        mrShared.mpXorEmulation->UpdateTarget();
        mrShared.mpXorEmulation->Disable();
        mrShared.maContextHolder.set(mrShared.mpXorEmulation->GetTargetContext());
        mrShared.mnXorMode = 0;
    }
}

void AquaGraphicsBackend::SetROPFillColor(SalROPColor nROPColor)
{
    if (!mrShared.mbPrinter)
    {
        SetFillColor(ImplGetROPColor(nROPColor));
    }
}

void AquaGraphicsBackend::SetROPLineColor(SalROPColor nROPColor)
{
    if (!mrShared.mbPrinter)
    {
        SetLineColor(ImplGetROPColor(nROPColor));
    }
}

void AquaGraphicsBackend::drawPixelImpl(tools::Long nX, tools::Long nY, const RGBAColor& rColor)
{
    if (!mrShared.checkContext())
        return;

    // overwrite the fill color
    CGContextSetFillColor(mrShared.maContextHolder.get(), rColor.AsArray());
    // draw 1x1 rect, there is no pixel drawing in Quartz
    const CGRect aDstRect = CGRectMake(nX, nY, 1, 1);
    CGContextFillRect(mrShared.maContextHolder.get(), aDstRect);

    refreshRect(aDstRect);

    // reset the fill color
    CGContextSetFillColor(mrShared.maContextHolder.get(), mrShared.maFillColor.AsArray());
}

void AquaGraphicsBackend::drawPixel(tools::Long nX, tools::Long nY)
{
    // draw pixel with current line color
    drawPixelImpl(nX, nY, mrShared.maLineColor);
}

void AquaGraphicsBackend::drawPixel(tools::Long nX, tools::Long nY, Color nColor)
{
    const RGBAColor aPixelColor(nColor);
    drawPixelImpl(nX, nY, aPixelColor);
}

void AquaGraphicsBackend::drawLine(tools::Long nX1, tools::Long nY1, tools::Long nX2,
                                   tools::Long nY2)
{
    if (nX1 == nX2 && nY1 == nY2)
    {
        // #i109453# platform independent code expects at least one pixel to be drawn
        drawPixel(nX1, nY1);
        return;
    }

    if (!mrShared.checkContext())
        return;

    CGContextBeginPath(mrShared.maContextHolder.get());
    CGContextMoveToPoint(mrShared.maContextHolder.get(), float(nX1) + 0.5, float(nY1) + 0.5);
    CGContextAddLineToPoint(mrShared.maContextHolder.get(), float(nX2) + 0.5, float(nY2) + 0.5);
    CGContextDrawPath(mrShared.maContextHolder.get(), kCGPathStroke);

    tools::Rectangle aRefreshRect(nX1, nY1, nX2, nY2);
    (void)aRefreshRect;
    // Is a call to RefreshRect( aRefreshRect ) missing here?
}

void AquaGraphicsBackend::drawRect(tools::Long nX, tools::Long nY, tools::Long nWidth,
                                   tools::Long nHeight)
{
    if (!mrShared.checkContext())
        return;

    CGRect aRect = CGRectMake(nX, nY, nWidth, nHeight);
    if (mrShared.isPenVisible())
    {
        aRect.origin.x += 0.5;
        aRect.origin.y += 0.5;
        aRect.size.width -= 1;
        aRect.size.height -= 1;
    }

    if (mrShared.isBrushVisible())
    {
        CGContextFillRect(mrShared.maContextHolder.get(), aRect);
    }
    if (mrShared.isPenVisible())
    {
        CGContextStrokeRect(mrShared.maContextHolder.get(), aRect);
    }
    mrShared.refreshRect(nX, nY, nWidth, nHeight);
}

void AquaGraphicsBackend::drawPolyLine(sal_uInt32 nPoints, const Point* pPointArray)
{
    if (nPoints < 1)
        return;

    if (!mrShared.checkContext())
        return;

    tools::Long nX = 0, nY = 0, nWidth = 0, nHeight = 0;
    getBoundRect(nPoints, pPointArray, nX, nY, nWidth, nHeight);

    float fX, fY;
    CGContextBeginPath(mrShared.maContextHolder.get());
    alignLinePoint(pPointArray, fX, fY);
    CGContextMoveToPoint(mrShared.maContextHolder.get(), fX, fY);
    pPointArray++;

    for (sal_uInt32 nPoint = 1; nPoint < nPoints; nPoint++, pPointArray++)
    {
        alignLinePoint(pPointArray, fX, fY);
        CGContextAddLineToPoint(mrShared.maContextHolder.get(), fX, fY);
    }
    CGContextStrokePath(mrShared.maContextHolder.get());

    mrShared.refreshRect(nX, nY, nWidth, nHeight);
}

void AquaGraphicsBackend::drawPolygon(sal_uInt32 nPoints, const Point* pPointArray)
{
    if (nPoints <= 1)
        return;

    if (!mrShared.checkContext())
        return;

    tools::Long nX = 0, nY = 0, nWidth = 0, nHeight = 0;
    getBoundRect(nPoints, pPointArray, nX, nY, nWidth, nHeight);

    CGPathDrawingMode eMode;
    if (mrShared.isBrushVisible() && mrShared.isPenVisible())
    {
        eMode = kCGPathEOFillStroke;
    }
    else if (mrShared.isPenVisible())
    {
        eMode = kCGPathStroke;
    }
    else if (mrShared.isBrushVisible())
    {
        eMode = kCGPathEOFill;
    }
    else
    {
        SAL_WARN("vcl.quartz", "Neither pen nor brush visible");
        return;
    }

    CGContextBeginPath(mrShared.maContextHolder.get());

    if (mrShared.isPenVisible())
    {
        float fX, fY;
        alignLinePoint(pPointArray, fX, fY);
        CGContextMoveToPoint(mrShared.maContextHolder.get(), fX, fY);
        pPointArray++;
        for (sal_uInt32 nPoint = 1; nPoint < nPoints; nPoint++, pPointArray++)
        {
            alignLinePoint(pPointArray, fX, fY);
            CGContextAddLineToPoint(mrShared.maContextHolder.get(), fX, fY);
        }
    }
    else
    {
        CGContextMoveToPoint(mrShared.maContextHolder.get(), pPointArray->getX(),
                             pPointArray->getY());
        pPointArray++;
        for (sal_uInt32 nPoint = 1; nPoint < nPoints; nPoint++, pPointArray++)
        {
            CGContextAddLineToPoint(mrShared.maContextHolder.get(), pPointArray->getX(),
                                    pPointArray->getY());
        }
    }

    CGContextClosePath(mrShared.maContextHolder.get());
    CGContextDrawPath(mrShared.maContextHolder.get(), eMode);

    mrShared.refreshRect(nX, nY, nWidth, nHeight);
}

void AquaGraphicsBackend::drawPolyPolygon(sal_uInt32 nPolyCount, const sal_uInt32* pPoints,
                                          const Point** ppPtAry)
{
    if (nPolyCount <= 0)
        return;

    if (!mrShared.checkContext())
        return;

    // find bound rect
    tools::Long leftX = 0, topY = 0, maxWidth = 0, maxHeight = 0;

    getBoundRect(pPoints[0], ppPtAry[0], leftX, topY, maxWidth, maxHeight);

    for (sal_uInt32 n = 1; n < nPolyCount; n++)
    {
        tools::Long nX = leftX, nY = topY, nW = maxWidth, nH = maxHeight;
        getBoundRect(pPoints[n], ppPtAry[n], nX, nY, nW, nH);
        if (nX < leftX)
        {
            maxWidth += leftX - nX;
            leftX = nX;
        }
        if (nY < topY)
        {
            maxHeight += topY - nY;
            topY = nY;
        }
        if (nX + nW > leftX + maxWidth)
        {
            maxWidth = nX + nW - leftX;
        }
        if (nY + nH > topY + maxHeight)
        {
            maxHeight = nY + nH - topY;
        }
    }

    // prepare drawing mode
    CGPathDrawingMode eMode;
    if (mrShared.isBrushVisible() && mrShared.isPenVisible())
    {
        eMode = kCGPathEOFillStroke;
    }
    else if (mrShared.isPenVisible())
    {
        eMode = kCGPathStroke;
    }
    else if (mrShared.isBrushVisible())
    {
        eMode = kCGPathEOFill;
    }
    else
    {
        SAL_WARN("vcl.quartz", "Neither pen nor brush visible");
        return;
    }

    // convert to CGPath
    CGContextBeginPath(mrShared.maContextHolder.get());
    if (mrShared.isPenVisible())
    {
        for (sal_uInt32 nPoly = 0; nPoly < nPolyCount; nPoly++)
        {
            const sal_uInt32 nPoints = pPoints[nPoly];
            if (nPoints > 1)
            {
                const Point* pPtAry = ppPtAry[nPoly];
                float fX, fY;

                alignLinePoint(pPtAry, fX, fY);
                CGContextMoveToPoint(mrShared.maContextHolder.get(), fX, fY);
                pPtAry++;

                for (sal_uInt32 nPoint = 1; nPoint < nPoints; nPoint++, pPtAry++)
                {
                    alignLinePoint(pPtAry, fX, fY);
                    CGContextAddLineToPoint(mrShared.maContextHolder.get(), fX, fY);
                }
                CGContextClosePath(mrShared.maContextHolder.get());
            }
        }
    }
    else
    {
        for (sal_uInt32 nPoly = 0; nPoly < nPolyCount; nPoly++)
        {
            const sal_uInt32 nPoints = pPoints[nPoly];
            if (nPoints > 1)
            {
                const Point* pPtAry = ppPtAry[nPoly];
                CGContextMoveToPoint(mrShared.maContextHolder.get(), pPtAry->getX(),
                                     pPtAry->getY());
                pPtAry++;
                for (sal_uInt32 nPoint = 1; nPoint < nPoints; nPoint++, pPtAry++)
                {
                    CGContextAddLineToPoint(mrShared.maContextHolder.get(), pPtAry->getX(),
                                            pPtAry->getY());
                }
                CGContextClosePath(mrShared.maContextHolder.get());
            }
        }
    }

    CGContextDrawPath(mrShared.maContextHolder.get(), eMode);

    mrShared.refreshRect(leftX, topY, maxWidth, maxHeight);
}

bool AquaGraphicsBackend::drawPolyPolygon(const basegfx::B2DHomMatrix& rObjectToDevice,
                                          const basegfx::B2DPolyPolygon& rPolyPolygon,
                                          double fTransparency)
{
#ifdef IOS
    if (!mrShared.maContextHolder.isSet())
        return true;
#endif

    // short circuit if there is nothing to do
    if (rPolyPolygon.count() == 0)
        return true;

    // ignore invisible polygons
    if ((fTransparency >= 1.0) || (fTransparency < 0))
        return true;

    // Fallback: Transform to DeviceCoordinates
    basegfx::B2DPolyPolygon aPolyPolygon(rPolyPolygon);
    aPolyPolygon.transform(rObjectToDevice);

    // setup poly-polygon path
    CGMutablePathRef xPath = CGPathCreateMutable();
    // tdf#120252 Use the correct, already transformed PolyPolygon (as long as
    // the transformation is not used here...)
    for (auto const& rPolygon : std::as_const(aPolyPolygon))
    {
        AddPolygonToPath(xPath, rPolygon, true, !getAntiAlias(), mrShared.isPenVisible());
    }

    const CGRect aRefreshRect = CGPathGetBoundingBox(xPath);
    // #i97317# workaround for Quartz having problems with drawing small polygons
    if (aRefreshRect.size.width > 0.125 || aRefreshRect.size.height > 0.125)
    {
        // prepare drawing mode
        CGPathDrawingMode eMode;
        if (mrShared.isBrushVisible() && mrShared.isPenVisible())
        {
            eMode = kCGPathEOFillStroke;
        }
        else if (mrShared.isPenVisible())
        {
            eMode = kCGPathStroke;
        }
        else if (mrShared.isBrushVisible())
        {
            eMode = kCGPathEOFill;
        }
        else
        {
            SAL_WARN("vcl.quartz", "Neither pen nor brush visible");
            CGPathRelease(xPath);
            return true;
        }

        // use the path to prepare the graphics context
        mrShared.maContextHolder.saveState();
        CGContextBeginPath(mrShared.maContextHolder.get());
        CGContextAddPath(mrShared.maContextHolder.get(), xPath);

        // draw path with antialiased polygon
        CGContextSetShouldAntialias(mrShared.maContextHolder.get(), getAntiAlias());
        CGContextSetAlpha(mrShared.maContextHolder.get(), 1.0 - fTransparency);
        CGContextDrawPath(mrShared.maContextHolder.get(), eMode);
        mrShared.maContextHolder.restoreState();

        // mark modified rectangle as updated
        refreshRect(aRefreshRect);
    }

    CGPathRelease(xPath);

    return true;
}

bool AquaGraphicsBackend::drawPolyLine(const basegfx::B2DHomMatrix& rObjectToDevice,
                                       const basegfx::B2DPolygon& rPolyLine, double fTransparency,
                                       double fLineWidth,
                                       const std::vector<double>* pStroke, // MM01
                                       basegfx::B2DLineJoin eLineJoin,
                                       css::drawing::LineCap eLineCap, double fMiterMinimumAngle,
                                       bool bPixelSnapHairline)
{
    // MM01 check done for simple reasons
    if (!rPolyLine.count() || fTransparency < 0.0 || fTransparency > 1.0)
    {
        return true;
    }

#ifdef IOS
    if (!mrShared.checkContext())
        return false;
#endif

    // tdf#124848 get correct LineWidth in discrete coordinates,
    if (fLineWidth == 0) // hairline
        fLineWidth = 1.0;
    else // Adjust line width for object-to-device scale.
        fLineWidth = (rObjectToDevice * basegfx::B2DVector(fLineWidth, 0)).getLength();

    // #i101491# Aqua does not support B2DLineJoin::NONE; return false to use
    // the fallback (own geometry preparation)
    // #i104886# linejoin-mode and thus the above only applies to "fat" lines
    if ((basegfx::B2DLineJoin::NONE == eLineJoin) && (fLineWidth > 1.3))
        return false;

    // MM01 need to do line dashing as fallback stuff here now
    const double fDotDashLength(
        nullptr != pStroke ? std::accumulate(pStroke->begin(), pStroke->end(), 0.0) : 0.0);
    const bool bStrokeUsed(0.0 != fDotDashLength);
    assert(!bStrokeUsed || (bStrokeUsed && pStroke));
    basegfx::B2DPolyPolygon aPolyPolygonLine;

    if (bStrokeUsed)
    {
        // apply LineStyle
        basegfx::utils::applyLineDashing(rPolyLine, // source
                                         *pStroke, // pattern
                                         &aPolyPolygonLine, // target for lines
                                         nullptr, // target for gaps
                                         fDotDashLength); // full length if available
    }
    else
    {
        // no line dashing, just copy
        aPolyPolygonLine.append(rPolyLine);
    }

    // Transform to DeviceCoordinates, get DeviceLineWidth, execute PixelSnapHairline
    aPolyPolygonLine.transform(rObjectToDevice);
    if (bPixelSnapHairline)
    {
        aPolyPolygonLine = basegfx::utils::snapPointsOfHorizontalOrVerticalEdges(aPolyPolygonLine);
    }

    // setup line attributes
    CGLineJoin aCGLineJoin = kCGLineJoinMiter;
    switch (eLineJoin)
    {
        case basegfx::B2DLineJoin::NONE:
            aCGLineJoin = /*TODO?*/ kCGLineJoinMiter;
            break;
        case basegfx::B2DLineJoin::Bevel:
            aCGLineJoin = kCGLineJoinBevel;
            break;
        case basegfx::B2DLineJoin::Miter:
            aCGLineJoin = kCGLineJoinMiter;
            break;
        case basegfx::B2DLineJoin::Round:
            aCGLineJoin = kCGLineJoinRound;
            break;
    }
    // convert miter minimum angle to miter limit
    CGFloat fCGMiterLimit = 1.0 / sin(fMiterMinimumAngle / 2.0);
    // setup cap attribute
    CGLineCap aCGLineCap(kCGLineCapButt);

    switch (eLineCap)
    {
        default: // css::drawing::LineCap_BUTT:
        {
            aCGLineCap = kCGLineCapButt;
            break;
        }
        case css::drawing::LineCap_ROUND:
        {
            aCGLineCap = kCGLineCapRound;
            break;
        }
        case css::drawing::LineCap_SQUARE:
        {
            aCGLineCap = kCGLineCapSquare;
            break;
        }
    }

    // setup poly-polygon path
    CGMutablePathRef xPath = CGPathCreateMutable();

    // MM01 todo - I assume that this is OKAY to be done in one run for quartz
    // but this NEEDS to be checked/verified
    for (sal_uInt32 a(0); a < aPolyPolygonLine.count(); a++)
    {
        const basegfx::B2DPolygon aPolyLine(aPolyPolygonLine.getB2DPolygon(a));
        AddPolygonToPath(xPath, aPolyLine, aPolyLine.isClosed(), !getAntiAlias(), true);
    }

    const CGRect aRefreshRect = CGPathGetBoundingBox(xPath);
    // #i97317# workaround for Quartz having problems with drawing small polygons
    if ((aRefreshRect.size.width > 0.125) || (aRefreshRect.size.height > 0.125))
    {
        // use the path to prepare the graphics context
        mrShared.maContextHolder.saveState();
        CGContextBeginPath(mrShared.maContextHolder.get());
        CGContextAddPath(mrShared.maContextHolder.get(), xPath);
        // draw path with antialiased line
        CGContextSetShouldAntialias(mrShared.maContextHolder.get(), getAntiAlias());
        CGContextSetAlpha(mrShared.maContextHolder.get(), 1.0 - fTransparency);
        CGContextSetLineJoin(mrShared.maContextHolder.get(), aCGLineJoin);
        CGContextSetLineCap(mrShared.maContextHolder.get(), aCGLineCap);
        CGContextSetLineWidth(mrShared.maContextHolder.get(), fLineWidth);
        CGContextSetMiterLimit(mrShared.maContextHolder.get(), fCGMiterLimit);
        CGContextDrawPath(mrShared.maContextHolder.get(), kCGPathStroke);
        mrShared.maContextHolder.restoreState();

        // mark modified rectangle as updated
        refreshRect(aRefreshRect);
    }

    CGPathRelease(xPath);

    return true;
}

bool AquaGraphicsBackend::drawPolyLineBezier(sal_uInt32 /*nPoints*/, const Point* /*pPointArray*/,
                                             const PolyFlags* /*pFlagArray*/)
{
    return false;
}

bool AquaGraphicsBackend::drawPolygonBezier(sal_uInt32 /*nPoints*/, const Point* /*pPointArray*/,
                                            const PolyFlags* /*pFlagArray*/)
{
    return false;
}

bool AquaGraphicsBackend::drawPolyPolygonBezier(sal_uInt32 /*nPoly*/, const sal_uInt32* /*pPoints*/,
                                                const Point* const* /*pPointArray*/,
                                                const PolyFlags* const* /*pFlagArray*/)
{
    return false;
}

void AquaGraphicsBackend::drawBitmap(const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap)
{
    if (!mrShared.checkContext())
        return;

    const QuartzSalBitmap& rBitmap = static_cast<const QuartzSalBitmap&>(rSalBitmap);
    CGImageRef xImage = rBitmap.CreateCroppedImage(
        static_cast<int>(rPosAry.mnSrcX), static_cast<int>(rPosAry.mnSrcY),
        static_cast<int>(rPosAry.mnSrcWidth), static_cast<int>(rPosAry.mnSrcHeight));
    if (!xImage)
        return;

    const CGRect aDstRect
        = CGRectMake(rPosAry.mnDestX, rPosAry.mnDestY, rPosAry.mnDestWidth, rPosAry.mnDestHeight);
    CGContextDrawImage(mrShared.maContextHolder.get(), aDstRect, xImage);

    CGImageRelease(xImage);
    refreshRect(aDstRect);
}

void AquaGraphicsBackend::drawBitmap(const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap,
                                     const SalBitmap& rTransparentBitmap)
{
    if (!mrShared.checkContext())
        return;

    const QuartzSalBitmap& rBitmap = static_cast<const QuartzSalBitmap&>(rSalBitmap);
    const QuartzSalBitmap& rMask = static_cast<const QuartzSalBitmap&>(rTransparentBitmap);

    CGImageRef xMaskedImage(rBitmap.CreateWithMask(rMask, rPosAry.mnSrcX, rPosAry.mnSrcY,
                                                   rPosAry.mnSrcWidth, rPosAry.mnSrcHeight));
    if (!xMaskedImage)
        return;

    const CGRect aDstRect
        = CGRectMake(rPosAry.mnDestX, rPosAry.mnDestY, rPosAry.mnDestWidth, rPosAry.mnDestHeight);
    CGContextDrawImage(mrShared.maContextHolder.get(), aDstRect, xMaskedImage);
    CGImageRelease(xMaskedImage);
    refreshRect(aDstRect);
}

void AquaGraphicsBackend::drawMask(const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap,
                                   Color nMaskColor)
{
    if (!mrShared.checkContext())
        return;

    const QuartzSalBitmap& rBitmap = static_cast<const QuartzSalBitmap&>(rSalBitmap);
    CGImageRef xImage = rBitmap.CreateColorMask(rPosAry.mnSrcX, rPosAry.mnSrcY, rPosAry.mnSrcWidth,
                                                rPosAry.mnSrcHeight, nMaskColor);
    if (!xImage)
        return;

    const CGRect aDstRect
        = CGRectMake(rPosAry.mnDestX, rPosAry.mnDestY, rPosAry.mnDestWidth, rPosAry.mnDestHeight);
    CGContextDrawImage(mrShared.maContextHolder.get(), aDstRect, xImage);
    CGImageRelease(xImage);
    refreshRect(aDstRect);
}

std::shared_ptr<SalBitmap> AquaGraphicsBackend::getBitmap(tools::Long nX, tools::Long nY,
                                                          tools::Long nDX, tools::Long nDY)
{
    SAL_WARN_IF(!mrShared.maLayer.isSet(), "vcl.quartz",
                "AquaSalGraphics::getBitmap() with no layer this=" << this);

    mrShared.applyXorContext();

    std::shared_ptr<QuartzSalBitmap> pBitmap = std::make_shared<QuartzSalBitmap>();
    if (!pBitmap->Create(mrShared.maLayer, mrShared.mnBitmapDepth, nX, nY, nDX, nDY,
                         mrShared.isFlipped()))
    {
        pBitmap = nullptr;
    }
    return pBitmap;
}

Color AquaGraphicsBackend::getPixel(tools::Long nX, tools::Long nY)
{
    // return default value on printers or when out of bounds
    if (!mrShared.maLayer.isSet() || (nX < 0) || (nX >= mrShared.mnWidth) || (nY < 0)
        || (nY >= mrShared.mnHeight))
    {
        return COL_BLACK;
    }

        // prepare creation of matching a CGBitmapContext
#if defined OSL_BIGENDIAN
    struct
    {
        unsigned char b, g, r, a;
    } aPixel;
#else
    struct
    {
        unsigned char a, r, g, b;
    } aPixel;
#endif

    // create a one-pixel bitmap context
    // TODO: is it worth to cache it?
    CGContextRef xOnePixelContext = CGBitmapContextCreate(
        &aPixel, 1, 1, 8, 32, GetSalData()->mxRGBSpace,
        uint32_t(kCGImageAlphaNoneSkipFirst) | uint32_t(kCGBitmapByteOrder32Big));

    // update this graphics layer
    mrShared.applyXorContext();

    // copy the requested pixel into the bitmap context
    if (mrShared.isFlipped())
    {
        nY = mrShared.mnHeight - nY;
    }
    const CGPoint aCGPoint = CGPointMake(-nX, -nY);
    CGContextDrawLayerAtPoint(xOnePixelContext, aCGPoint, mrShared.maLayer.get());

    CGContextRelease(xOnePixelContext);

    Color nColor(aPixel.r, aPixel.g, aPixel.b);
    return nColor;
}

void AquaSalGraphics::GetResolution(sal_Int32& rDPIX, sal_Int32& rDPIY)
{
#ifndef IOS
    if (!mnRealDPIY)
    {
        initResolution((maShared.mbWindow && maShared.mpFrame) ? maShared.mpFrame->getNSWindow()
                                                               : nil);
    }

    rDPIX = mnRealDPIX;
    rDPIY = mnRealDPIY;
#else
    // This *must* be 96 or else the iOS app will behave very badly (tiles are scaled wrongly and
    // don't match each others at their boundaries, and other issues). But *why* it must be 96 I
    // have no idea. The commit that changed it to 96 from (the arbitrary) 200 did not say. If you
    // know where else 96 is explicitly or implicitly hard-coded, please modify this comment.

    // Follow-up: It might be this: in 'online', loleaflet/src/map/Map.js:
    // 15 = 1440 twips-per-inch / 96 dpi.
    // Chosen to match previous hardcoded value of 3840 for
    // the current tile pixel size of 256.
    rDPIX = rDPIY = 96;
#endif
}

void AquaGraphicsBackend::pattern50Fill()
{
    static const CGFloat aFillCol[4] = { 1, 1, 1, 1 };
    static const CGPatternCallbacks aCallback = { 0, &drawPattern50, nullptr };
    static const CGColorSpaceRef mxP50Space = CGColorSpaceCreatePattern(GetSalData()->mxRGBSpace);
    static const CGPatternRef mxP50Pattern
        = CGPatternCreate(nullptr, CGRectMake(0, 0, 4, 4), CGAffineTransformIdentity, 4, 4,
                          kCGPatternTilingConstantSpacing, false, &aCallback);
    SAL_WARN_IF(!mrShared.maContextHolder.get(), "vcl.quartz", "maContextHolder.get() is NULL");
    CGContextSetFillColorSpace(mrShared.maContextHolder.get(), mxP50Space);
    CGContextSetFillPattern(mrShared.maContextHolder.get(), mxP50Pattern, aFillCol);
    CGContextFillPath(mrShared.maContextHolder.get());
}

void AquaGraphicsBackend::invert(tools::Long nX, tools::Long nY, tools::Long nWidth,
                                 tools::Long nHeight, SalInvert nFlags)
{
    if (mrShared.checkContext())
    {
        CGRect aCGRect = CGRectMake(nX, nY, nWidth, nHeight);
        mrShared.maContextHolder.saveState();
        if (nFlags & SalInvert::TrackFrame)
        {
            const CGFloat dashLengths[2] = { 4.0, 4.0 }; // for drawing dashed line
            CGContextSetBlendMode(mrShared.maContextHolder.get(), kCGBlendModeDifference);
            CGContextSetRGBStrokeColor(mrShared.maContextHolder.get(), 1.0, 1.0, 1.0, 1.0);
            CGContextSetLineDash(mrShared.maContextHolder.get(), 0, dashLengths, 2);
            CGContextSetLineWidth(mrShared.maContextHolder.get(), 2.0);
            CGContextStrokeRect(mrShared.maContextHolder.get(), aCGRect);
        }
        else if (nFlags & SalInvert::N50)
        {
            //CGContextSetAllowsAntialiasing( maContextHolder.get(), false );
            CGContextSetBlendMode(mrShared.maContextHolder.get(), kCGBlendModeDifference);
            CGContextAddRect(mrShared.maContextHolder.get(), aCGRect);
            pattern50Fill();
        }
        else // just invert
        {
            CGContextSetBlendMode(mrShared.maContextHolder.get(), kCGBlendModeDifference);
            CGContextSetRGBFillColor(mrShared.maContextHolder.get(), 1.0, 1.0, 1.0, 1.0);
            CGContextFillRect(mrShared.maContextHolder.get(), aCGRect);
        }
        mrShared.maContextHolder.restoreState();
        refreshRect(aCGRect);
    }
}

namespace
{
CGPoint* makeCGptArray(sal_uInt32 nPoints, const Point* pPtAry)
{
    CGPoint* CGpoints = new CGPoint[nPoints];
    for (sal_uLong i = 0; i < nPoints; i++)
    {
        CGpoints[i].x = pPtAry[i].getX();
        CGpoints[i].y = pPtAry[i].getY();
    }
    return CGpoints;
}

} // end anonymous ns

void AquaGraphicsBackend::invert(sal_uInt32 nPoints, const Point* pPtAry, SalInvert nSalFlags)
{
    if (mrShared.checkContext())
    {
        mrShared.maContextHolder.saveState();
        CGPoint* CGpoints = makeCGptArray(nPoints, pPtAry);
        CGContextAddLines(mrShared.maContextHolder.get(), CGpoints, nPoints);
        if (nSalFlags & SalInvert::TrackFrame)
        {
            const CGFloat dashLengths[2] = { 4.0, 4.0 }; // for drawing dashed line
            CGContextSetBlendMode(mrShared.maContextHolder.get(), kCGBlendModeDifference);
            CGContextSetRGBStrokeColor(mrShared.maContextHolder.get(), 1.0, 1.0, 1.0, 1.0);
            CGContextSetLineDash(mrShared.maContextHolder.get(), 0, dashLengths, 2);
            CGContextSetLineWidth(mrShared.maContextHolder.get(), 2.0);
            CGContextStrokePath(mrShared.maContextHolder.get());
        }
        else if (nSalFlags & SalInvert::N50)
        {
            CGContextSetBlendMode(mrShared.maContextHolder.get(), kCGBlendModeDifference);
            pattern50Fill();
        }
        else // just invert
        {
            CGContextSetBlendMode(mrShared.maContextHolder.get(), kCGBlendModeDifference);
            CGContextSetRGBFillColor(mrShared.maContextHolder.get(), 1.0, 1.0, 1.0, 1.0);
            CGContextFillPath(mrShared.maContextHolder.get());
        }
        const CGRect aRefreshRect = CGContextGetClipBoundingBox(mrShared.maContextHolder.get());
        mrShared.maContextHolder.restoreState();
        delete[] CGpoints;
        refreshRect(aRefreshRect);
    }
}

#ifndef IOS
bool AquaGraphicsBackend::drawEPS(tools::Long nX, tools::Long nY, tools::Long nWidth,
                                  tools::Long nHeight, void* pEpsData, sal_uInt32 nByteCount)
{
    // convert the raw data to an NSImageRef
    NSData* xNSData = [NSData dataWithBytes:pEpsData length:static_cast<int>(nByteCount)];
    NSImageRep* xEpsImage = [NSEPSImageRep imageRepWithData:xNSData];
    if (!xEpsImage)
    {
        return false;
    }
    // get the target context
    if (!mrShared.checkContext())
    {
        return false;
    }
    // NOTE: flip drawing, else the nsimage would be drawn upside down
    mrShared.maContextHolder.saveState();
    //  CGContextTranslateCTM( maContextHolder.get(), 0, +mnHeight );
    CGContextScaleCTM(mrShared.maContextHolder.get(), +1, -1);
    nY = /*mnHeight*/ -(nY + nHeight);

    // prepare the target context
    NSGraphicsContext* pOrigNSCtx = [NSGraphicsContext currentContext];
    [pOrigNSCtx retain];

    // create new context
    NSGraphicsContext* pDrawNSCtx =
        [NSGraphicsContext graphicsContextWithCGContext:mrShared.maContextHolder.get()
                                                flipped:mrShared.isFlipped()];
    // set it, setCurrentContext also releases the previously set one
    [NSGraphicsContext setCurrentContext:pDrawNSCtx];

    // draw the EPS
    const NSRect aDstRect = NSMakeRect(nX, nY, nWidth, nHeight);
    const bool bOK = [xEpsImage drawInRect:aDstRect];

    // restore the NSGraphicsContext
    [NSGraphicsContext setCurrentContext:pOrigNSCtx];
    [pOrigNSCtx release]; // restore the original retain count

    mrShared.maContextHolder.restoreState();
    // mark the destination rectangle as updated
    refreshRect(aDstRect);

    return bOK;
}
#else
bool AquaGraphicsBackend::drawEPS(tools::Long /*nX*/, tools::Long /*nY*/, tools::Long /*nWidth*/,
                                  tools::Long /*nHeight*/, void* /*pEpsData*/,
                                  sal_uInt32 /*nByteCount*/)
{
    return false;
}
#endif

bool AquaGraphicsBackend::blendBitmap(const SalTwoRect& /*rPosAry*/, const SalBitmap& /*rBitmap*/)
{
    return false;
}

bool AquaGraphicsBackend::blendAlphaBitmap(const SalTwoRect& /*rPosAry*/,
                                           const SalBitmap& /*rSrcBitmap*/,
                                           const SalBitmap& /*rMaskBitmap*/,
                                           const SalBitmap& /*rAlphaBitmap*/)
{
    return false;
}

bool AquaGraphicsBackend::drawAlphaBitmap(const SalTwoRect& rTR, const SalBitmap& rSrcBitmap,
                                          const SalBitmap& rAlphaBmp)
{
    // An image mask can't have a depth > 8 bits (should be 1 to 8 bits)
    if (rAlphaBmp.GetBitCount() > 8)
        return false;

    // are these two tests really necessary? (see vcl/unx/source/gdi/salgdi2.cxx)
    // horizontal/vertical mirroring not implemented yet
    if (rTR.mnDestWidth < 0 || rTR.mnDestHeight < 0)
        return false;

    const QuartzSalBitmap& rSrcSalBmp = static_cast<const QuartzSalBitmap&>(rSrcBitmap);
    const QuartzSalBitmap& rMaskSalBmp = static_cast<const QuartzSalBitmap&>(rAlphaBmp);
    CGImageRef xMaskedImage = rSrcSalBmp.CreateWithMask(rMaskSalBmp, rTR.mnSrcX, rTR.mnSrcY,
                                                        rTR.mnSrcWidth, rTR.mnSrcHeight);
    if (!xMaskedImage)
        return false;

    if (mrShared.checkContext())
    {
        const CGRect aDstRect
            = CGRectMake(rTR.mnDestX, rTR.mnDestY, rTR.mnDestWidth, rTR.mnDestHeight);
        CGContextDrawImage(mrShared.maContextHolder.get(), aDstRect, xMaskedImage);
        refreshRect(aDstRect);
    }

    CGImageRelease(xMaskedImage);

    return true;
}

bool AquaGraphicsBackend::drawTransformedBitmap(const basegfx::B2DPoint& rNull,
                                                const basegfx::B2DPoint& rX,
                                                const basegfx::B2DPoint& rY,
                                                const SalBitmap& rSrcBitmap,
                                                const SalBitmap* pAlphaBmp, double fAlpha)
{
    if (!mrShared.checkContext())
        return true;

    if (fAlpha != 1.0)
        return false;

    // get the Quartz image
    CGImageRef xImage = nullptr;
    const Size aSize = rSrcBitmap.GetSize();
    const QuartzSalBitmap& rSrcSalBmp = static_cast<const QuartzSalBitmap&>(rSrcBitmap);
    const QuartzSalBitmap* pMaskSalBmp = static_cast<const QuartzSalBitmap*>(pAlphaBmp);

    if (!pMaskSalBmp)
        xImage = rSrcSalBmp.CreateCroppedImage(0, 0, int(aSize.Width()), int(aSize.Height()));
    else
        xImage = rSrcSalBmp.CreateWithMask(*pMaskSalBmp, 0, 0, int(aSize.Width()),
                                           int(aSize.Height()));

    if (!xImage)
        return false;

    // setup the image transformation
    // using the rNull,rX,rY points as destinations for the (0,0),(0,Width),(Height,0) source points
    mrShared.maContextHolder.saveState();
    const basegfx::B2DVector aXRel = rX - rNull;
    const basegfx::B2DVector aYRel = rY - rNull;
    const CGAffineTransform aCGMat = CGAffineTransformMake(
        aXRel.getX() / aSize.Width(), aXRel.getY() / aSize.Width(), aYRel.getX() / aSize.Height(),
        aYRel.getY() / aSize.Height(), rNull.getX(), rNull.getY());

    CGContextConcatCTM(mrShared.maContextHolder.get(), aCGMat);

    // draw the transformed image
    const CGRect aSrcRect = CGRectMake(0, 0, aSize.Width(), aSize.Height());
    CGContextDrawImage(mrShared.maContextHolder.get(), aSrcRect, xImage);

    CGImageRelease(xImage);

    // restore the Quartz graphics state
    mrShared.maContextHolder.restoreState();

    // mark the destination as painted
    const CGRect aDstRect = CGRectApplyAffineTransform(aSrcRect, aCGMat);
    refreshRect(aDstRect);

    return true;
}

bool AquaGraphicsBackend::hasFastDrawTransformedBitmap() const { return false; }

bool AquaGraphicsBackend::drawAlphaRect(tools::Long nX, tools::Long nY, tools::Long nWidth,
                                        tools::Long nHeight, sal_uInt8 nTransparency)
{
    if (!mrShared.checkContext())
        return true;

    // save the current state
    mrShared.maContextHolder.saveState();
    CGContextSetAlpha(mrShared.maContextHolder.get(), (100 - nTransparency) * (1.0 / 100));

    CGRect aRect = CGRectMake(nX, nY, nWidth - 1, nHeight - 1);
    if (mrShared.isPenVisible())
    {
        aRect.origin.x += 0.5;
        aRect.origin.y += 0.5;
    }

    CGContextBeginPath(mrShared.maContextHolder.get());
    CGContextAddRect(mrShared.maContextHolder.get(), aRect);
    CGContextDrawPath(mrShared.maContextHolder.get(), kCGPathFill);

    mrShared.maContextHolder.restoreState();
    refreshRect(aRect);

    return true;
}

bool AquaGraphicsBackend::drawGradient(const tools::PolyPolygon& /*rPolygon*/,
                                       const Gradient& /*rGradient*/)
{
    return false;
}

bool AquaGraphicsBackend::implDrawGradient(basegfx::B2DPolyPolygon const& /*rPolyPolygon*/,
                                           SalGradient const& /*rGradient*/)
{
    return false;
}

bool AquaGraphicsBackend::supportsOperation(OutDevSupportType eType) const
{
    switch (eType)
    {
        case OutDevSupportType::TransparentRect:
        case OutDevSupportType::B2DDraw:
            return true;
        default:
            break;
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
