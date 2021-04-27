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

#include <tools/stream.hxx>
#include <tools/vcompat.hxx>
#include <vcl/lineinfo.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dlinegeometry.hxx>
#include <numeric>


ImplLineInfo::ImplLineInfo()
    : mnWidth(0)
    , mnDashLen(0)
    , mnDotLen(0)
    , mnDistance(0)
    , meLineJoin(basegfx::B2DLineJoin::Round)
    , meLineCap(css::drawing::LineCap_BUTT)
    , meStyle(LineStyle::Solid)
    , mnDashCount(0)
    , mnDotCount(0)
{
}

inline bool ImplLineInfo::operator==( const ImplLineInfo& rB ) const
{
    return(meStyle == rB.meStyle
        && mnWidth == rB.mnWidth
        && mnDashCount == rB.mnDashCount
        && mnDashLen == rB.mnDashLen
        && mnDotCount == rB.mnDotCount
        && mnDotLen == rB.mnDotLen
        && mnDistance == rB.mnDistance
        && meLineJoin == rB.meLineJoin
        && meLineCap == rB.meLineCap);
}


LineInfo::LineInfo( LineStyle eStyle, double nWidth ) : mpImplLineInfo()
{
    mpImplLineInfo->meStyle = eStyle;
    mpImplLineInfo->mnWidth = nWidth;
}

LineInfo::LineInfo( const LineInfo& ) = default;

LineInfo::LineInfo( LineInfo&& ) = default;

LineInfo::~LineInfo() = default;

LineInfo& LineInfo::operator=( const LineInfo& ) = default;

LineInfo& LineInfo::operator=( LineInfo&& ) = default;

bool LineInfo::operator==( const LineInfo& rLineInfo ) const
{
    return mpImplLineInfo == rLineInfo.mpImplLineInfo;
}

void LineInfo::SetStyle( LineStyle eStyle )
{
    mpImplLineInfo->meStyle = eStyle;
}

void LineInfo::SetWidth( double nWidth )
{
    mpImplLineInfo->mnWidth = nWidth;
}

void LineInfo::SetDashCount( sal_uInt16 nDashCount )
{
    mpImplLineInfo->mnDashCount = nDashCount;
}

void LineInfo::SetDashLen( double nDashLen )
{
    mpImplLineInfo->mnDashLen = nDashLen;
}

void LineInfo::SetDotCount( sal_uInt16 nDotCount )
{
    mpImplLineInfo->mnDotCount = nDotCount;
}

void LineInfo::SetDotLen( double nDotLen )
{
    mpImplLineInfo->mnDotLen = nDotLen;
}

void LineInfo::SetDistance( double nDistance )
{
    mpImplLineInfo->mnDistance = nDistance;
}

void LineInfo::SetLineJoin(basegfx::B2DLineJoin eLineJoin)
{
    mpImplLineInfo->meLineJoin = eLineJoin;
}

void LineInfo::SetLineCap(css::drawing::LineCap eLineCap)
{
    mpImplLineInfo->meLineCap = eLineCap;
}

bool LineInfo::IsDefault() const
{
    return( !mpImplLineInfo->mnWidth
        && ( LineStyle::Solid == mpImplLineInfo->meStyle )
        && ( css::drawing::LineCap_BUTT == mpImplLineInfo->meLineCap));
}

SvStream& ReadLineInfo( SvStream& rIStm, LineInfo& rLineInfo )
{
    VersionCompatRead aCompat( rIStm );
    sal_uInt16          nTmp16(0);
    sal_Int32       nTmp32(0);

    rIStm.ReadUInt16( nTmp16 );
    rLineInfo.mpImplLineInfo->meStyle = static_cast<LineStyle>(nTmp16);
    rIStm.ReadInt32( nTmp32 );
    rLineInfo.mpImplLineInfo->mnWidth = nTmp32;

    if( aCompat.GetVersion() >= 2 )
    {
        // version 2
        rIStm.ReadUInt16( rLineInfo.mpImplLineInfo->mnDashCount ).ReadInt32( nTmp32 );
        rLineInfo.mpImplLineInfo->mnDashLen = nTmp32;
        rIStm.ReadUInt16( rLineInfo.mpImplLineInfo->mnDotCount ).ReadInt32( nTmp32 );
        rLineInfo.mpImplLineInfo->mnDotLen = nTmp32;
        rIStm.ReadInt32( nTmp32 );
        rLineInfo.mpImplLineInfo->mnDistance = nTmp32;
    }

    if( aCompat.GetVersion() >= 3 )
    {
        // version 3
        rIStm.ReadUInt16( nTmp16 );
        rLineInfo.mpImplLineInfo->meLineJoin = static_cast<basegfx::B2DLineJoin>(nTmp16);
    }

    if( aCompat.GetVersion() >= 4 )
    {
        // version 4
        rIStm.ReadUInt16( nTmp16 );
        rLineInfo.mpImplLineInfo->meLineCap = static_cast<css::drawing::LineCap>(nTmp16);
    }

    if( aCompat.GetVersion() >= 5 )
    {
        // version 5
        rIStm.ReadDouble( rLineInfo.mpImplLineInfo->mnWidth );
        rIStm.ReadDouble( rLineInfo.mpImplLineInfo->mnDashLen );
        rIStm.ReadDouble( rLineInfo.mpImplLineInfo->mnDotLen );
        rIStm.ReadDouble( rLineInfo.mpImplLineInfo->mnDistance );
    }

    return rIStm;
}

SvStream& WriteLineInfo( SvStream& rOStm, const LineInfo& rLineInfo )
{
    VersionCompatWrite aCompat( rOStm, 5 );

    // version 1
    rOStm.WriteUInt16( static_cast<sal_uInt16>(rLineInfo.mpImplLineInfo->meStyle) )
         .WriteInt32( basegfx::fround( rLineInfo.mpImplLineInfo->mnWidth ));

    // since version2
    rOStm.WriteUInt16( rLineInfo.mpImplLineInfo->mnDashCount )
         .WriteInt32( basegfx::fround( rLineInfo.mpImplLineInfo->mnDashLen ));
    rOStm.WriteUInt16( rLineInfo.mpImplLineInfo->mnDotCount )
         .WriteInt32( basegfx::fround( rLineInfo.mpImplLineInfo->mnDotLen ));
    rOStm.WriteInt32( basegfx::fround( rLineInfo.mpImplLineInfo->mnDistance ));

    // since version3
    rOStm.WriteUInt16( static_cast<sal_uInt16>(rLineInfo.mpImplLineInfo->meLineJoin) );

    // since version4
    rOStm.WriteUInt16( static_cast<sal_uInt16>(rLineInfo.mpImplLineInfo->meLineCap) );

    // since version5
    rOStm.WriteDouble( rLineInfo.mpImplLineInfo->mnWidth );
    rOStm.WriteDouble( rLineInfo.mpImplLineInfo->mnDashLen );
    rOStm.WriteDouble( rLineInfo.mpImplLineInfo->mnDotLen );
    rOStm.WriteDouble( rLineInfo.mpImplLineInfo->mnDistance );

    return rOStm;
}

void LineInfo::applyToB2DPolyPolygon(
    basegfx::B2DPolyPolygon& io_rLinePolyPolygon,
    basegfx::B2DPolyPolygon& o_rFillPolyPolygon) const
{
    o_rFillPolyPolygon.clear();

    if(!io_rLinePolyPolygon.count())
        return;

    if(LineStyle::Dash == GetStyle())
    {
        ::std::vector< double > fDotDashArray;
        const double fDashLen(GetDashLen());
        const double fDotLen(GetDotLen());
        const double fDistance(GetDistance());

        for(sal_uInt16 a(0); a < GetDashCount(); a++)
        {
            fDotDashArray.push_back(fDashLen);
            fDotDashArray.push_back(fDistance);
        }

        for(sal_uInt16 b(0); b < GetDotCount(); b++)
        {
            fDotDashArray.push_back(fDotLen);
            fDotDashArray.push_back(fDistance);
        }

        const double fAccumulated(::std::accumulate(fDotDashArray.begin(), fDotDashArray.end(), 0.0));

        if(fAccumulated > 0.0)
        {
            basegfx::B2DPolyPolygon aResult;

            for(auto const& rPolygon : io_rLinePolyPolygon)
            {
                basegfx::B2DPolyPolygon aLineTarget;
                basegfx::utils::applyLineDashing(
                    rPolygon,
                    fDotDashArray,
                    &aLineTarget);
                aResult.append(aLineTarget);
            }

            io_rLinePolyPolygon = aResult;
        }
    }

    if(!(GetWidth() > 1 && io_rLinePolyPolygon.count()))
        return;

    const double fHalfLineWidth((GetWidth() * 0.5) + 0.5);

    for(auto const& rPolygon : io_rLinePolyPolygon)
    {
        o_rFillPolyPolygon.append(basegfx::utils::createAreaGeometry(
            rPolygon,
            fHalfLineWidth,
            GetLineJoin(),
            GetLineCap()));
    }

    io_rLinePolyPolygon.clear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
