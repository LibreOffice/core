/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <osl/diagnose.h>
#include <tools/line.hxx>
#include <tools/helpers.hxx>

#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>
#include <vcl/drawables/HatchDrawable.hxx>

#include <salgdi.hxx>

#define HATCH_MAXPOINTS 1024

namespace vcl
{
bool HatchDrawable::DrawCommand(OutputDevice* pRenderContext) const
{
    if (mbScaffolding)
        return Draw(pRenderContext, maPolyPolygon, maHatch);
    else
        return DrawDecomposedHatchLines(pRenderContext, maPolyPolygon, maHatch);
}

bool HatchDrawable::Draw(OutputDevice* pRenderContext, tools::PolyPolygon const& rPolyPolygon,
                         Hatch const& rHatch) const
{
    Hatch aHatch(rHatch);

    if (pRenderContext->GetDrawMode()
        & (DrawModeFlags::BlackLine | DrawModeFlags::WhiteLine | DrawModeFlags::GrayLine
           | DrawModeFlags::SettingsLine))
    {
        Color aColor(rHatch.GetColor());

        if (pRenderContext->GetDrawMode() & DrawModeFlags::BlackLine)
        {
            aColor = COL_BLACK;
        }
        else if (pRenderContext->GetDrawMode() & DrawModeFlags::WhiteLine)
        {
            aColor = COL_WHITE;
        }
        else if (pRenderContext->GetDrawMode() & DrawModeFlags::GrayLine)
        {
            const sal_uInt8 cLum = aColor.GetLuminance();
            aColor = Color(cLum, cLum, cLum);
        }
        else if (pRenderContext->GetDrawMode() & DrawModeFlags::SettingsLine)
        {
            aColor = pRenderContext->GetSettings().GetStyleSettings().GetFontColor();
        }

        aHatch.SetColor(aColor);
    }

    mpMetaAction = new MetaHatchAction(rPolyPolygon, aHatch);
    AddAction(pRenderContext);

    if (!CanDraw(pRenderContext))
        return false;

    if (!InitClipRegion(pRenderContext))
        return false;

    if (rPolyPolygon.Count())
    {
        tools::PolyPolygon aPolyPoly(pRenderContext->LogicToPixel(rPolyPolygon));
        GDIMetaFile* pOldMetaFile = pRenderContext->GetConnectMetaFile();
        bool bOldMap = pRenderContext->IsMapModeEnabled();

        aPolyPoly.Optimize(PolyOptimizeFlags::NO_SAME);
        aHatch.SetDistance(pRenderContext->ImplLogicWidthToDevicePixel(aHatch.GetDistance()));

        pRenderContext->SetConnectMetaFile(nullptr);
        pRenderContext->EnableMapMode(false);
        pRenderContext->Push(PushFlags::LINECOLOR);
        pRenderContext->SetLineColor(aHatch.GetColor());
        pRenderContext->InitLineColor();
        pRenderContext->DrawHatch(aPolyPoly, aHatch, false);
        pRenderContext->Pop();
        pRenderContext->EnableMapMode(bOldMap);
        pRenderContext->SetConnectMetaFile(pOldMetaFile);
    }

    DrawAlphaVirtDev(pRenderContext);

    return true;
}

bool HatchDrawable::DrawDecomposedHatchLines(OutputDevice* pRenderContext,
                                             tools::PolyPolygon const& rPolyPolygon,
                                             Hatch const& rHatch) const
{
    if (rPolyPolygon.Count())
    {
        // #i115630# DrawHatch does not work with beziers included in the polypolygon, take care of that
        if (rPolyPolygon.HasCurve())
        {
            OSL_ENSURE(
                false,
                "DrawHatch does *not* support curves, falling back to AdaptiveSubdivide()...");
            tools::PolyPolygon aPolyPoly;

            rPolyPolygon.AdaptiveSubdivide(aPolyPoly);
            DrawDecomposedHatchLines(pRenderContext, aPolyPoly, rHatch);
        }
        else
        {
            tools::Rectangle aRect(rPolyPolygon.GetBoundRect());
            const long nLogPixelWidth = pRenderContext->ImplDevicePixelToLogicWidth(1);
            const long nWidth = pRenderContext->ImplDevicePixelToLogicWidth(
                std::max(pRenderContext->ImplLogicWidthToDevicePixel(rHatch.GetDistance()), 3L));

            // Single hatch
            aRect.AdjustLeft(-nLogPixelWidth);
            aRect.AdjustTop(-nLogPixelWidth);
            aRect.AdjustRight(nLogPixelWidth);
            aRect.AdjustBottom(nLogPixelWidth);
            DrawHatchLines(pRenderContext, rPolyPolygon, aRect, nWidth, rHatch.GetAngle());

            if ((rHatch.GetStyle() == HatchStyle::Double)
                || (rHatch.GetStyle() == HatchStyle::Triple))
            {
                // Double hatch
                DrawHatchLines(pRenderContext, rPolyPolygon, aRect, nWidth,
                               rHatch.GetAngle() + 900);

                if (rHatch.GetStyle() == HatchStyle::Triple)
                {
                    // Triple hatch
                    DrawHatchLines(pRenderContext, rPolyPolygon, aRect, nWidth,
                                   rHatch.GetAngle() + 450);
                }
            }
        }
    }

    return true;
}

static Size GetHatchIncrement(long nDist, sal_uInt16 nAngle10)
{
    long nAngle = nAngle10 % 1800;
    double fAngle = 0.0;

    if (nAngle > 900)
        nAngle -= 1800;

    switch (nAngle)
    {
        case 0:
            return Size(0, nDist);

        case 900:
            return Size(nDist, 0);

        case -450:
        case 450:
            fAngle = F_PI1800 * labs(nAngle);
            return Size(0, nDist = FRound(nDist / cos(fAngle)));

        default:
            fAngle = F_PI1800 * labs(nAngle);
            return Size(0, FRound(nDist / cos(fAngle)));
    }
}

static Point GetPt1(tools::Rectangle const& rRect, long nDist, sal_uInt16 nAngle10, Point aRef)
{
    long nAngle = nAngle10 % 1800;
    if (nAngle > 900)
        nAngle -= 1800;

    Point aPt;
    long nOffset = 0;

    double fAngle = 0.0;
    double fTan = 0.0;

    switch (nAngle)
    {
        case 0:
            aPt = rRect.TopLeft();

            if (aRef.Y() <= rRect.Top())
                nOffset = (rRect.Top() - aRef.Y()) % nDist;
            else
                nOffset = nDist - ((aRef.Y() - rRect.Top()) % nDist);

            aPt.AdjustY(-nOffset);

            return aPt;

        case 900:
            aPt = rRect.TopLeft();

            if (aRef.X() <= rRect.Left())
                nOffset = (rRect.Left() - aRef.X()) % nDist;
            else
                nOffset = nDist - ((aRef.X() - rRect.Left()) % nDist);

            aPt.AdjustX(-nOffset);

            return aPt;

        case -450:
        case 450:
            fAngle = F_PI1800 * labs(nAngle);
            fTan = tan(fAngle);

            long nPY;

            if (nAngle > 0)
            {
                aPt = rRect.TopLeft();
                nPY = FRound(aRef.Y() - ((aPt.X() - aRef.X()) * fTan));
            }
            else
            {
                aPt = rRect.TopRight();
                nPY = FRound(aRef.Y() + ((aPt.X() - aRef.X()) * fTan));
            }

            if (nPY <= aPt.Y())
                nOffset = (aPt.Y() - nPY) % nDist;
            else
                nOffset = nDist - ((nPY - aPt.Y()) % nDist);

            aPt.AdjustY(-nOffset);

            return aPt;

        default:
            fAngle = F_PI1800 * labs(nAngle);
            fTan = tan(fAngle);

            long nPX;

            if (nAngle > 0)
            {
                aPt = rRect.TopLeft();
                nPX = FRound(aRef.X() - ((aPt.Y() - aRef.Y()) / fTan));
            }
            else
            {
                aPt = rRect.BottomLeft();
                nPX = FRound(aRef.X() + ((aPt.Y() - aRef.Y()) / fTan));
            }

            if (nPX <= aPt.X())
                nOffset = (aPt.X() - nPX) % nDist;
            else
                nOffset = nDist - ((nPX - aPt.X()) % nDist);

            aPt.AdjustX(-nOffset);

            return aPt;
    }
}

static Point GetPt2(tools::Rectangle const& rRect, long nDist, sal_uInt16 nAngle10, Point aRef)
{
    long nAngle = nAngle10 % 1800;
    if (nAngle > 900)
        nAngle -= 1800;

    long nOffset = 0;

    double fAngle = 0.0;
    double fTan = 0.0;

    Point aPt1;
    Point aPt2;
    long nPX = 0;
    long nPY = 0;
    long nYOff = 0;
    long nXOff = 0;

    switch (nAngle)
    {
        case 0:
            aPt2 = rRect.TopRight();

            if (aRef.Y() <= rRect.Top())
                nOffset = ((rRect.Top() - aRef.Y()) % nDist);
            else
                nOffset = (nDist - ((aRef.Y() - rRect.Top()) % nDist));

            aPt2.AdjustY(-nOffset);

            return aPt2;

        case 900:
            aPt2 = rRect.BottomLeft();

            if (aRef.X() <= rRect.Left())
                nOffset = (rRect.Left() - aRef.X()) % nDist;
            else
                nOffset = nDist - ((aRef.X() - rRect.Left()) % nDist);

            aPt2.AdjustX(-nOffset);

            return aPt2;

        case -450:
        case 450:
            fAngle = F_PI1800 * labs(nAngle);
            fTan = tan(fAngle);
            nYOff = FRound((rRect.Right() - rRect.Left()) * fTan);

            if (nAngle > 0)
            {
                aPt1 = rRect.TopLeft();
                aPt2 = Point(rRect.Right(), rRect.Top() - nYOff);
                nPY = FRound(aRef.Y() - ((aPt1.X() - aRef.X()) * fTan));
            }
            else
            {
                aPt1 = rRect.TopRight();
                aPt2 = Point(rRect.Left(), rRect.Top() - nYOff);
                nPY = FRound(aRef.Y() + ((aPt1.X() - aRef.X()) * fTan));
            }

            if (nPY <= aPt1.Y())
                nOffset = (aPt1.Y() - nPY) % nDist;
            else
                nOffset = nDist - ((nPY - aPt1.Y()) % nDist);

            aPt2.AdjustY(-nOffset);

            return aPt2;

        default:
            fAngle = F_PI1800 * labs(nAngle);
            fTan = tan(fAngle);
            nXOff = FRound((rRect.Bottom() - rRect.Top()) / fTan);

            if (nAngle > 0)
            {
                aPt1 = rRect.TopLeft();
                aPt2 = Point(rRect.Left() - nXOff, rRect.Top());
                nPX = FRound(aRef.Y() - ((aPt1.X() - aRef.X()) * fTan));
            }
            else
            {
                aPt1 = rRect.TopRight();
                aPt2 = Point(rRect.Left() - nXOff, rRect.Bottom());
                nPX = FRound(aRef.Y() + ((aPt1.X() - aRef.X()) * fTan));
            }

            if (nPX <= aPt1.X())
                nOffset = (aPt1.X() - nPX) % nDist;
            else
                nOffset = nDist - ((nPX - aPt1.X()) % nDist);

            aPt2.AdjustX(-nOffset);

            return aPt2;
    }
}

static Point GetEndPt1(tools::Rectangle const& rRect, sal_uInt16 nAngle10)
{
    long nAngle = nAngle10 % 1800;
    const double fAngle = F_PI1800 * labs(nAngle);
    const double fTan = tan(fAngle);
    const long nXOff = FRound((rRect.Bottom() - rRect.Top()) / fTan);
    const long nYOff = FRound((rRect.Right() - rRect.Left()) * fTan);

    if (nAngle > 900)
        nAngle -= 1800;

    switch (nAngle)
    {
        case 0:
            return rRect.BottomLeft();
        case 900:
            return rRect.TopRight();
        case 450:
        case -450:
            return nAngle > 0 ? Point(rRect.Left(), rRect.Bottom() + nYOff)
                              : Point(rRect.Right(), rRect.Bottom() + nYOff);
        default:
            return nAngle > 0 ? Point(rRect.Right() + nXOff, rRect.Top())
                              : Point(rRect.Right() + nXOff, rRect.Bottom());
    }
}

void HatchDrawable::DrawHatchLines(OutputDevice* pRenderContext,
                                   tools::PolyPolygon const& rPolyPolygon,
                                   tools::Rectangle const& rRect, long nDist,
                                   sal_uInt16 nAngle) const
{
    Point aRef = (!pRenderContext->IsRefPoint() ? rRect.TopLeft() : pRenderContext->GetRefPoint());

    Size aInc = GetHatchIncrement(nDist, nAngle);
    Point aPt1 = GetPt1(rRect, nDist, nAngle, aRef);
    Point aPt2 = GetPt2(rRect, nDist, nAngle, aRef);
    Point aEndPt1 = GetEndPt1(rRect, nAngle);

    do
    {
        DrawHatchLine(pRenderContext, tools::Line(aPt1, aPt2), rPolyPolygon);

        aPt1.AdjustX(aInc.Width());
        aPt1.AdjustY(aInc.Height());
        aPt2.AdjustX(aInc.Width());
        aPt2.AdjustY(aInc.Height());
    } while ((aPt1.X() <= aEndPt1.X()) && (aPt1.Y() <= aEndPt1.Y()));
}

struct PointArray
{
    PointArray(long nCountPoints, Point* pPoints)
        : mnCountPoints(nCountPoints)
        , mpPoints(pPoints)
    {
    }

    long mnCountPoints;
    Point* mpPoints;
};

extern "C" {
static int HatchCmpFnc(const void* p1, const void* p2)
{
    const long nX1 = static_cast<Point const*>(p1)->X();
    const long nX2 = static_cast<Point const*>(p2)->X();
    const long nY1 = static_cast<Point const*>(p1)->Y();
    const long nY2 = static_cast<Point const*>(p2)->Y();

    return (nX1 > nX2 ? 1 : nX1 == nX2 ? nY1 > nY2 ? 1 : nY1 == nY2 ? 0 : -1 : -1);
}
}

static PointArray GetHatchLinePoints(tools::Line const& rLine,
                                     tools::PolyPolygon const& rPolyPolygon)
{
    double fX, fY;
    long nAdd, nPCounter = 0;
    Point* pPtBuffer(new Point[HATCH_MAXPOINTS]);

    for (long nPoly = 0, nPolyCount = rPolyPolygon.Count(); nPoly < nPolyCount; nPoly++)
    {
        const tools::Polygon& rPoly = rPolyPolygon[static_cast<sal_uInt16>(nPoly)];

        if (rPoly.GetSize() > 1)
        {
            tools::Line aCurSegment(rPoly[0], Point());

            for (long i = 1, nCount = rPoly.GetSize(); i <= nCount; i++)
            {
                aCurSegment.SetEnd(rPoly[static_cast<sal_uInt16>(i % nCount)]);
                nAdd = 0;

                if (rLine.Intersection(aCurSegment, fX, fY))
                {
                    if ((fabs(fX - aCurSegment.GetStart().X()) <= 0.0000001)
                        && (fabs(fY - aCurSegment.GetStart().Y()) <= 0.0000001))
                    {
                        const tools::Line aPrevSegment(
                            rPoly[static_cast<sal_uInt16>((i > 1) ? (i - 2) : (nCount - 1))],
                            aCurSegment.GetStart());
                        const double fPrevDistance = rLine.GetDistance(aPrevSegment.GetStart());
                        const double fCurDistance = rLine.GetDistance(aCurSegment.GetEnd());

                        if ((fPrevDistance <= 0.0 && fCurDistance > 0.0)
                            || (fPrevDistance > 0.0 && fCurDistance < 0.0))
                        {
                            nAdd = 1;
                        }
                    }
                    else if ((fabs(fX - aCurSegment.GetEnd().X()) <= 0.0000001)
                             && (fabs(fY - aCurSegment.GetEnd().Y()) <= 0.0000001))
                    {
                        const tools::Line aNextSegment(
                            aCurSegment.GetEnd(), rPoly[static_cast<sal_uInt16>((i + 1) % nCount)]);

                        if ((fabs(rLine.GetDistance(aNextSegment.GetEnd())) <= 0.0000001)
                            && (rLine.GetDistance(aCurSegment.GetStart()) > 0.0))
                        {
                            nAdd = 1;
                        }
                    }
                    else
                    {
                        nAdd = 1;
                    }

                    if (nAdd)
                        pPtBuffer[nPCounter++] = Point(FRound(fX), FRound(fY));
                }

                aCurSegment.SetStart(aCurSegment.GetEnd());
            }
        }
    }

    if (nPCounter > 1)
    {
        qsort(pPtBuffer, nPCounter, sizeof(Point), HatchCmpFnc);

        if (nPCounter & 1)
            nPCounter--;
    }

    return PointArray(nPCounter, pPtBuffer);
}

void HatchDrawable::DrawHatchLine(OutputDevice* pRenderContext, const tools::Line& rLine,
                                  const tools::PolyPolygon& rPolyPolygon) const
{
    PointArray aPtBuffer = GetHatchLinePoints(rLine, rPolyPolygon);

    if (aPtBuffer.mnCountPoints > 1)
    {
        if (mbMtf)
        {
            for (long i = 0; i < aPtBuffer.mnCountPoints; i += 2)
            {
                GDIMetaFile* pMetaFile = pRenderContext->GetConnectMetaFile();
                pMetaFile->AddAction(
                    new MetaLineAction(aPtBuffer.mpPoints[i], aPtBuffer.mpPoints[i + 1]));
            }
        }
        else
        {
            for (long i = 0; i < aPtBuffer.mnCountPoints; i += 2)
            {
                pRenderContext->DrawHatchLine_DrawLine(aPtBuffer.mpPoints[i],
                                                       aPtBuffer.mpPoints[i + 1]);
            }

            delete[] aPtBuffer.mpPoints;
        }
    }
}

bool HatchDrawable::CanDraw(OutputDevice* pRenderContext) const
{
    if (!pRenderContext->IsDeviceOutputNecessary() || pRenderContext->ImplIsRecordLayout())
        return false;

    return true;
}

} // namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
