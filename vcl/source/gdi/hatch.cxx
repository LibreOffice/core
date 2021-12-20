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

#include <tools/GenericTypeSerializer.hxx>
#include <tools/helpers.hxx>
#include <tools/stream.hxx>
#include <tools/vcompat.hxx>

#include <vcl/hatch.hxx>

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
