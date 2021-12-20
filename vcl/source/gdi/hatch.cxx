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

#include <sal/log.hxx>
#include <tools/GenericTypeSerializer.hxx>
#include <tools/helpers.hxx>
#include <tools/line.hxx>
#include <tools/poly.hxx>
#include <tools/stream.hxx>
#include <tools/vcompat.hxx>

#include <vcl/hatch.hxx>
#include <vcl/metaact.hxx>

extern "C" {

static int HatchCmpFnc( const void* p1, const void* p2 )
{
    const tools::Long nX1 = static_cast<Point const *>(p1)->X();
    const tools::Long nX2 = static_cast<Point const *>(p2)->X();
    const tools::Long nY1 = static_cast<Point const *>(p1)->Y();
    const tools::Long nY2 = static_cast<Point const *>(p2)->Y();

    return ( nX1 > nX2 ? 1 : nX1 == nX2 ? nY1 > nY2 ? 1: nY1 == nY2 ? 0 : -1 : -1 );
}

}

ImplHatch::ImplHatch() :
    maColor     ( COL_BLACK ),
    meStyle     ( HatchStyle::Single ),
    mnDistance  ( 1 ),
    mnAngle     ( 0 )
{
}

bool ImplHatch::operator==( const ImplHatch& rImplHatch ) const
{
    return maColor == rImplHatch.maColor &&
        meStyle == rImplHatch.meStyle &&
        mnDistance == rImplHatch.mnDistance &&
        mnAngle == rImplHatch.mnAngle;
}

Hatch::Hatch() = default;

Hatch::Hatch( const Hatch& ) = default;

Hatch::Hatch( HatchStyle eStyle, const Color& rColor,
              tools::Long nDistance, Degree10 nAngle10 )
{
    mpImplHatch->maColor = rColor;
    mpImplHatch->meStyle = eStyle;
    mpImplHatch->mnDistance = nDistance;
    mpImplHatch->mnAngle = nAngle10;
}

Hatch::~Hatch() = default;

Hatch& Hatch::operator=( const Hatch& ) = default;

bool Hatch::operator==( const Hatch& rHatch ) const
{
    return mpImplHatch == rHatch.mpImplHatch;
}


void Hatch::SetColor( const Color& rColor )
{
    mpImplHatch->maColor = rColor;
}

void Hatch::SetDistance( tools::Long nDistance )
{
    mpImplHatch->mnDistance = nDistance;
}

void Hatch::SetAngle( Degree10 nAngle10 )
{
    mpImplHatch->mnAngle = nAngle10;
}

SvStream& ReadHatch( SvStream& rIStm, Hatch& rHatch )
{
    VersionCompatRead aCompat(rIStm);
    sal_uInt16 nTmp16;
    sal_Int32 nTmp32(0);

    rIStm.ReadUInt16(nTmp16);
    rHatch.mpImplHatch->meStyle = static_cast<HatchStyle>(nTmp16);

    tools::GenericTypeSerializer aSerializer(rIStm);
    aSerializer.readColor(rHatch.mpImplHatch->maColor);
    rIStm.ReadInt32(nTmp32);
    rHatch.mpImplHatch->mnDistance = nTmp32;
    rIStm.ReadUInt16(nTmp16);
    rHatch.mpImplHatch->mnAngle = Degree10(nTmp16);

    return rIStm;
}

SvStream& WriteHatch( SvStream& rOStm, const Hatch& rHatch )
{
    VersionCompatWrite aCompat(rOStm, 1);

    rOStm.WriteUInt16( static_cast<sal_uInt16>(rHatch.mpImplHatch->meStyle) );

    tools::GenericTypeSerializer aSerializer(rOStm);
    aSerializer.writeColor(rHatch.mpImplHatch->maColor);
    rOStm.WriteInt32( rHatch.mpImplHatch->mnDistance ).WriteUInt16( rHatch.mpImplHatch->mnAngle.get() );

    return rOStm;
}

void Hatch::CalcHatchValues( const tools::Rectangle& rRect, tools::Long nDist, Degree10 nAngle10,
                                    Point& rPt1, Point& rPt2, Size& rInc, Point& rEndPt1, Point const& rRef )
{
    Degree10    nAngle = nAngle10 % 1800_deg10;
    tools::Long    nOffset = 0;

    if( nAngle > 900_deg10 )
        nAngle -= 1800_deg10;

    if( 0_deg10 == nAngle )
    {
        rInc = Size( 0, nDist );
        rPt1 = rRect.TopLeft();
        rPt2 = rRect.TopRight();
        rEndPt1 = rRect.BottomLeft();

        if( rRef.Y() <= rRect.Top() )
            nOffset = ( ( rRect.Top() - rRef.Y() ) % nDist );
        else
            nOffset = ( nDist - ( ( rRef.Y() - rRect.Top() ) % nDist ) );

        rPt1.AdjustY( -nOffset );
        rPt2.AdjustY( -nOffset );
    }
    else if( 900_deg10 == nAngle )
    {
        rInc = Size( nDist, 0 );
        rPt1 = rRect.TopLeft();
        rPt2 = rRect.BottomLeft();
        rEndPt1 = rRect.TopRight();

        if( rRef.X() <= rRect.Left() )
            nOffset = ( rRect.Left() - rRef.X() ) % nDist;
        else
            nOffset = nDist - ( ( rRef.X() - rRect.Left() ) % nDist );

        rPt1.AdjustX( -nOffset );
        rPt2.AdjustX( -nOffset );
    }
    else if( nAngle >= Degree10(-450) && nAngle <= 450_deg10 )
    {
        const double    fAngle = std::abs( toRadians(nAngle) );
        const double    fTan = tan( fAngle );
        const tools::Long      nYOff = FRound( ( rRect.Right() - rRect.Left() ) * fTan );
        tools::Long            nPY;

        nDist = FRound( nDist / cos( fAngle ) );
        rInc = Size( 0, nDist );

        if( nAngle > 0_deg10 )
        {
            rPt1 = rRect.TopLeft();
            rPt2 = Point( rRect.Right(), rRect.Top() - nYOff );
            rEndPt1 = Point( rRect.Left(), rRect.Bottom() + nYOff );
            nPY = FRound( rRef.Y() - ( ( rPt1.X() - rRef.X() ) * fTan ) );
        }
        else
        {
            rPt1 = rRect.TopRight();
            rPt2 = Point( rRect.Left(), rRect.Top() - nYOff );
            rEndPt1 = Point( rRect.Right(), rRect.Bottom() + nYOff );
            nPY = FRound( rRef.Y() + ( ( rPt1.X() - rRef.X() ) * fTan ) );
        }

        if( nPY <= rPt1.Y() )
            nOffset = ( rPt1.Y() - nPY ) % nDist;
        else
            nOffset = nDist - ( ( nPY - rPt1.Y() ) % nDist );

        rPt1.AdjustY( -nOffset );
        rPt2.AdjustY( -nOffset );
    }
    else
    {
        const double fAngle = std::abs( toRadians(nAngle) );
        const double fTan = tan( fAngle );
        const tools::Long   nXOff = FRound( ( rRect.Bottom() - rRect.Top() ) / fTan );
        tools::Long         nPX;

        nDist = FRound( nDist / sin( fAngle ) );
        rInc = Size( nDist, 0 );

        if( nAngle > 0_deg10 )
        {
            rPt1 = rRect.TopLeft();
            rPt2 = Point( rRect.Left() - nXOff, rRect.Bottom() );
            rEndPt1 = Point( rRect.Right() + nXOff, rRect.Top() );
            nPX = FRound( rRef.X() - ( ( rPt1.Y() - rRef.Y() ) / fTan ) );
        }
        else
        {
            rPt1 = rRect.BottomLeft();
            rPt2 = Point( rRect.Left() - nXOff, rRect.Top() );
            rEndPt1 = Point( rRect.Right() + nXOff, rRect.Bottom() );
            nPX = FRound( rRef.X() + ( ( rPt1.Y() - rRef.Y() ) / fTan ) );
        }

        if( nPX <= rPt1.X() )
            nOffset = ( rPt1.X() - nPX ) % nDist;
        else
            nOffset = nDist - ( ( nPX - rPt1.X() ) % nDist );

        rPt1.AdjustX( -nOffset );
        rPt2.AdjustX( -nOffset );
    }
}

void Hatch::GenerateHatchLinePoints(tools::Line const& rLine, tools::PolyPolygon const& rPolyPoly, tools::Long& nPCounter, Point* pPtBuffer)
{
    double fX, fY;
    tools::Long nAdd;

    for (tools::Long nPoly = 0, nPolyCount = rPolyPoly.Count(); nPoly < nPolyCount; nPoly++)
    {
        tools::Polygon const& rPoly = rPolyPoly[static_cast<sal_uInt16>(nPoly)];

        if (rPoly.GetSize() > 1)
        {
            tools::Line aCurSegment(rPoly[0], Point());

            for (tools::Long i = 1, nCount = rPoly.GetSize(); i <= nCount; i++)
            {
                aCurSegment.SetEnd(rPoly[static_cast<sal_uInt16>(i % nCount)]);
                nAdd = 0;

                if (rLine.Intersection(aCurSegment, fX, fY))
                {
                    if ((fabs(fX - aCurSegment.GetStart().X() ) <= 0.0000001) &&
                        (fabs(fY - aCurSegment.GetStart().Y() ) <= 0.0000001))
                    {
                        const tools::Line aPrevSegment( rPoly[ static_cast<sal_uInt16>( ( i > 1 ) ? ( i - 2 ) : ( nCount - 1 ) ) ], aCurSegment.GetStart() );
                        const double fPrevDistance = rLine.GetDistance( aPrevSegment.GetStart() );
                        const double fCurDistance = rLine.GetDistance( aCurSegment.GetEnd() );

                        if ((fPrevDistance <= 0.0 && fCurDistance > 0.0) ||
                            (fPrevDistance > 0.0 && fCurDistance < 0.0))
                        {
                            nAdd = 1;
                        }
                    }
                    else if ((fabs(fX - aCurSegment.GetEnd().X()) <= 0.0000001) &&
                             (fabs(fY - aCurSegment.GetEnd().Y()) <= 0.0000001))
                    {
                        const tools::Line aNextSegment(aCurSegment.GetEnd(), rPoly[static_cast<sal_uInt16>((i + 1) % nCount)]);

                        if ((fabs(rLine.GetDistance(aNextSegment.GetEnd())) <= 0.0000001) &&
                            (rLine.GetDistance(aCurSegment.GetStart()) > 0.0))
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
}

void Hatch::AddHatchActions(tools::PolyPolygon const& rPolyPoly,
                                   tools::Long nLogPixelWidth, tools::Long nWidth, Point const& rRef,
                                   GDIMetaFile& rMtf) const
{

    tools::PolyPolygon aPolyPoly( rPolyPoly );
    aPolyPoly.Optimize( PolyOptimizeFlags::NO_SAME | PolyOptimizeFlags::CLOSE );

    if( aPolyPoly.Count() )
    {
        AddHatchLinesToMetafile(aPolyPoly, nLogPixelWidth, nWidth, rRef, rMtf);
        rMtf.AddAction(new MetaPopAction());
    }
}

const size_t HATCH_MAXPOINTS = 1024;

void Hatch::AddHatchLinesToMetafile(tools::PolyPolygon const& rPolyPoly,
                                    tools::Long nLogPixelWidth, tools::Long nWidth, Point const& rRef,
                                    GDIMetaFile& rMtf) const
{
    if (!rPolyPoly.Count())
        return;

    // #i115630# DrawHatch does not work with beziers included in the polypolygon, take care of that
    bool bIsCurve = false;

    for (sal_uInt16 a = 0; !bIsCurve && a < rPolyPoly.Count(); a++)
    {
        if (rPolyPoly[a].HasFlags())
            bIsCurve = true;
    }

    if (bIsCurve)
    {
        SAL_WARN("vcl.gdi", "DrawHatch does *not* support curves, falling back to AdaptiveSubdivide()...");
        tools::PolyPolygon aPolyPoly;

        rPolyPoly.AdaptiveSubdivide(aPolyPoly);
        AddHatchLinesToMetafile(aPolyPoly, nLogPixelWidth, nWidth, rRef, rMtf);
    }
    else
    {
        tools::Rectangle aRect(rPolyPoly.GetBoundRect());
        std::unique_ptr<Point[]> pPtBuffer(new Point[HATCH_MAXPOINTS]);
        Point aPt1, aPt2, aEndPt1;
        Size aInc;

        // Single hatch
        aRect.AdjustLeft(-nLogPixelWidth);
        aRect.AdjustTop(-nLogPixelWidth);
        aRect.AdjustRight(nLogPixelWidth);
        aRect.AdjustBottom(nLogPixelWidth);

        Hatch::CalcHatchValues(aRect, nWidth, GetAngle(), aPt1, aPt2, aInc, aEndPt1, rRef);

        do
        {
            Hatch::AddHatchLines(tools::Line(aPt1, aPt2), rPolyPoly, pPtBuffer.get(), rMtf);

            aPt1.AdjustX(aInc.Width());
            aPt1.AdjustY(aInc.Height());

            aPt2.AdjustX(aInc.Width());
            aPt2.AdjustY(aInc.Height());
        }
        while((aPt1.X() <= aEndPt1.X()) && (aPt1.Y() <= aEndPt1.Y()));

        if ((GetStyle() == HatchStyle::Double) || (GetStyle() == HatchStyle::Triple))
        {
            // Double hatch
            Hatch::CalcHatchValues(aRect, nWidth, GetAngle() + 900_deg10, aPt1, aPt2, aInc, aEndPt1, rRef);

            do
            {
                Hatch::AddHatchLines(tools::Line(aPt1, aPt2), rPolyPoly, pPtBuffer.get(), rMtf);

                aPt1.AdjustX(aInc.Width());
                aPt1.AdjustY(aInc.Height());

                aPt2.AdjustX(aInc.Width());
                aPt2.AdjustY(aInc.Height());
            }
            while ((aPt1.X() <= aEndPt1.X()) && (aPt1.Y() <= aEndPt1.Y()));

            if (GetStyle() == HatchStyle::Triple)
            {
                // Triple hatch
                Hatch::CalcHatchValues(aRect, nWidth, GetAngle() + 450_deg10, aPt1, aPt2, aInc, aEndPt1, rRef);

                do
                {
                    Hatch::AddHatchLines(tools::Line(aPt1, aPt2), rPolyPoly, pPtBuffer.get(), rMtf);

                    aPt1.AdjustX(aInc.Width());
                    aPt1.AdjustY(aInc.Height());

                    aPt2.AdjustX(aInc.Width());
                    aPt2.AdjustY(aInc.Height());
                }
                while ((aPt1.X() <= aEndPt1.X()) && (aPt1.Y() <= aEndPt1.Y()));
            }
        }
    }
}

void Hatch::AddHatchLines(tools::Line const& rLine, tools::PolyPolygon const& rPolyPoly,
                                  Point* pPtBuffer, GDIMetaFile& rMtf) const
{
    tools::Long nPCounter = 0;

    Hatch::GenerateHatchLinePoints(rLine, rPolyPoly, nPCounter, pPtBuffer);

    if (nPCounter <= 1)
        return;

    for (tools::Long i = 0; i < nPCounter; i += 2)
    {
        rMtf.AddAction(new MetaLineAction(pPtBuffer[i], pPtBuffer[i + 1]));
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
