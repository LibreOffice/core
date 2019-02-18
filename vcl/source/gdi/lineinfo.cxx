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


LineInfo::LineInfo( LineStyle eStyle, sal_Int32 nWidth ) : mpImplLineInfo()
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

void LineInfo::SetWidth( sal_Int32 nWidth )
{
    mpImplLineInfo->mnWidth = nWidth;
}

void LineInfo::SetDashCount( sal_uInt16 nDashCount )
{
    mpImplLineInfo->mnDashCount = nDashCount;
}

void LineInfo::SetDashLen( sal_Int32 nDashLen )
{
    mpImplLineInfo->mnDashLen = nDashLen;
}

void LineInfo::SetDotCount( sal_uInt16 nDotCount )
{
    mpImplLineInfo->mnDotCount = nDotCount;
}

void LineInfo::SetDotLen( sal_Int32 nDotLen )
{
    mpImplLineInfo->mnDotLen = nDotLen;
}

void LineInfo::SetDistance( sal_Int32 nDistance )
{
    mpImplLineInfo->mnDistance = nDistance;
}

void LineInfo::SetLineJoin(basegfx::B2DLineJoin eLineJoin)
{

    if(eLineJoin != mpImplLineInfo->meLineJoin)
    {
        mpImplLineInfo->meLineJoin = eLineJoin;
    }
}

void LineInfo::SetLineCap(css::drawing::LineCap eLineCap)
{
    if(eLineCap != mpImplLineInfo->meLineCap)
    {
        mpImplLineInfo->meLineCap = eLineCap;
    }
}

bool LineInfo::IsDefault() const
{
    return( !mpImplLineInfo->mnWidth
        && ( LineStyle::Solid == mpImplLineInfo->meStyle )
        && ( css::drawing::LineCap_BUTT == mpImplLineInfo->meLineCap));
}

SvStream& ReadLineInfo( SvStream& rIStm, LineInfo& rLineInfo )
{
    VersionCompat   aCompat( rIStm, StreamMode::READ );
    sal_uInt16          nTmp16(0);
    sal_Int32       nTmp32(0);

    rIStm.ReadUInt16( nTmp16 ); rLineInfo.mpImplLineInfo->meStyle = static_cast<LineStyle>(nTmp16);
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
        rIStm.ReadUInt16( nTmp16 ); rLineInfo.mpImplLineInfo->meLineJoin = static_cast<basegfx::B2DLineJoin>(nTmp16);
    }

    if( aCompat.GetVersion() >= 4 )
    {
        // version 4
        rIStm.ReadUInt16( nTmp16 ); rLineInfo.mpImplLineInfo->meLineCap = static_cast<css::drawing::LineCap>(nTmp16);
    }

    return rIStm;
}

SvStream& WriteLineInfo( SvStream& rOStm, const LineInfo& rLineInfo )
{
    VersionCompat aCompat( rOStm, StreamMode::WRITE, 4 );

    // version 1
    rOStm.WriteUInt16( static_cast<sal_uInt16>(rLineInfo.mpImplLineInfo->meStyle) )
         .WriteInt32( rLineInfo.mpImplLineInfo->mnWidth );

    // since version2
    rOStm.WriteUInt16( rLineInfo.mpImplLineInfo->mnDashCount )
         .WriteInt32( rLineInfo.mpImplLineInfo->mnDashLen );
    rOStm.WriteUInt16( rLineInfo.mpImplLineInfo->mnDotCount )
         .WriteInt32( rLineInfo.mpImplLineInfo->mnDotLen );
    rOStm.WriteInt32( rLineInfo.mpImplLineInfo->mnDistance );

    // since version3
    rOStm.WriteUInt16( static_cast<sal_uInt16>(rLineInfo.mpImplLineInfo->meLineJoin) );

    // since version4
    rOStm.WriteUInt16( static_cast<sal_uInt16>(rLineInfo.mpImplLineInfo->meLineCap) );

    return rOStm;
}

void LineInfo::applyToB2DPolyPolygon(
    basegfx::B2DPolyPolygon& io_rLinePolyPolygon,
    basegfx::B2DPolyPolygon& o_rFillPolyPolygon) const
{
    o_rFillPolyPolygon.clear();

    if(io_rLinePolyPolygon.count())
    {
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
                    basegfx::B2DPolyPolygon aLineTraget;
                    basegfx::utils::applyLineDashing(
                        rPolygon,
                        fDotDashArray,
                        &aLineTraget);
                    aResult.append(aLineTraget);
                }

                io_rLinePolyPolygon = aResult;
            }
        }

        if(GetWidth() > 1 && io_rLinePolyPolygon.count())
        {
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
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
