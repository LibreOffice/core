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

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dlinegeometry.hxx>
#include <tools/debug.hxx>

#include <vcl/lineinfo.hxx>
#include <vcl/metaact.hxx>
#include <vcl/virdev.hxx>

#include <drawmode.hxx>
#include <salgdi.hxx>

#include <cassert>
#include <numeric>

void OutputDevice::SetLineColor()
{

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaLineColorAction( Color(), false ) );

    if ( mbLineColor )
    {
        mbInitLineColor = true;
        mbLineColor = false;
        maLineColor = COL_TRANSPARENT;
    }

    if( mpAlphaVDev )
        mpAlphaVDev->SetLineColor(COL_ALPHA_OPAQUE );
}

void OutputDevice::SetLineColor( const Color& rColor )
{

    Color aColor = vcl::drawmode::GetLineColor(rColor, GetDrawMode(), GetSettings().GetStyleSettings());

    if( mpMetaFile )
        mpMetaFile->AddAction( new MetaLineColorAction( aColor, true ) );

    if( aColor.IsTransparent() )
    {
        if ( mbLineColor )
        {
            mbInitLineColor = true;
            mbLineColor = false;
            maLineColor = COL_TRANSPARENT;
        }
    }
    else
    {
        if( maLineColor != aColor )
        {
            mbInitLineColor = true;
            mbLineColor = true;
            maLineColor = aColor;
        }
    }

    if( mpAlphaVDev )
        mpAlphaVDev->SetLineColor( COL_BLACK );
}

void OutputDevice::InitLineColor()
{
    DBG_TESTSOLARMUTEX();

    if( mbLineColor )
    {
        if( RasterOp::N0 == meRasterOp )
            mpGraphics->SetROPLineColor( SalROPColor::N0 );
        else if( RasterOp::N1 == meRasterOp )
            mpGraphics->SetROPLineColor( SalROPColor::N1 );
        else if( RasterOp::Invert == meRasterOp )
            mpGraphics->SetROPLineColor( SalROPColor::Invert );
        else
            mpGraphics->SetLineColor( maLineColor );
    }
    else
    {
        mpGraphics->SetLineColor();
    }

    mbInitLineColor = false;
}

void OutputDevice::DrawLine( const Point& rStartPt, const Point& rEndPt,
                             const LineInfo& rLineInfo )
{
    assert(!is_double_buffered_window());

    if ( rLineInfo.IsDefault() )
    {
        DrawLine( rStartPt, rEndPt );
        return;
    }

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaLineAction( rStartPt, rEndPt, rLineInfo ) );

    if ( !IsDeviceOutputNecessary() || !mbLineColor || ( LineStyle::NONE == rLineInfo.GetStyle() ) || ImplIsRecordLayout() )
        return;

    if( !mpGraphics && !AcquireGraphics() )
        return;
    assert(mpGraphics);

    if ( mbInitClipRegion )
        InitClipRegion();

    if ( mbOutputClipped )
        return;

    const Point aStartPt( ImplLogicToDevicePixel( rStartPt ) );
    const Point aEndPt( ImplLogicToDevicePixel( rEndPt ) );
    const LineInfo aInfo( ImplLogicToDevicePixel( rLineInfo ) );
    const bool bDashUsed(LineStyle::Dash == aInfo.GetStyle());
    const bool bLineWidthUsed(aInfo.GetWidth() > 1);

    if ( mbInitLineColor )
        InitLineColor();

    if(bDashUsed || bLineWidthUsed)
    {
        basegfx::B2DPolygon aLinePolygon;
        aLinePolygon.append(basegfx::B2DPoint(aStartPt.X(), aStartPt.Y()));
        aLinePolygon.append(basegfx::B2DPoint(aEndPt.X(), aEndPt.Y()));

        drawLine( basegfx::B2DPolyPolygon(aLinePolygon), aInfo );
    }
    else
    {
        mpGraphics->DrawLine( aStartPt.X(), aStartPt.Y(), aEndPt.X(), aEndPt.Y(), *this );
    }

    if( mpAlphaVDev )
        mpAlphaVDev->DrawLine( rStartPt, rEndPt, rLineInfo );
}

void OutputDevice::DrawLine( const Point& rStartPt, const Point& rEndPt )
{
    assert(!is_double_buffered_window());

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaLineAction( rStartPt, rEndPt ) );

    if ( !IsDeviceOutputNecessary() || !mbLineColor || ImplIsRecordLayout() )
        return;

    if ( !mpGraphics && !AcquireGraphics() )
        return;
    assert(mpGraphics);

    if ( mbInitClipRegion )
        InitClipRegion();

    if ( mbOutputClipped )
        return;

    if ( mbInitLineColor )
        InitLineColor();

    bool bDrawn = false;

    // #i101598# support AA and snap for lines, too
    if( mpGraphics->supportsOperation(OutDevSupportType::B2DDraw)
        && RasterOp::OverPaint == GetRasterOp()
        && IsLineColor())
    {
        // at least transform with double precision to device coordinates; this will
        // avoid pixel snap of single, appended lines
        const basegfx::B2DHomMatrix aTransform(ImplGetDeviceTransformation());
        basegfx::B2DPolygon aB2DPolyLine;

        aB2DPolyLine.append(basegfx::B2DPoint(rStartPt.X(), rStartPt.Y()));
        aB2DPolyLine.append(basegfx::B2DPoint(rEndPt.X(), rEndPt.Y()));
        aB2DPolyLine.transform( aTransform );

        const bool bPixelSnapHairline(mnAntialiasing & AntialiasingFlags::PixelSnapHairline);

        bDrawn = mpGraphics->DrawPolyLine(
            basegfx::B2DHomMatrix(),
            aB2DPolyLine,
            0.0,
            0.0, // tdf#124848 hairline
            nullptr, // MM01
            basegfx::B2DLineJoin::NONE,
            css::drawing::LineCap_BUTT,
            basegfx::deg2rad(15.0), // not used with B2DLineJoin::NONE, but the correct default
            bPixelSnapHairline,
            *this);
    }
    if(!bDrawn)
    {
        const Point aStartPt(ImplLogicToDevicePixel(rStartPt));
        const Point aEndPt(ImplLogicToDevicePixel(rEndPt));
        mpGraphics->DrawLine( aStartPt.X(), aStartPt.Y(), aEndPt.X(), aEndPt.Y(), *this );
    }

    if( mpAlphaVDev )
        mpAlphaVDev->DrawLine( rStartPt, rEndPt );
}

void OutputDevice::drawLine( basegfx::B2DPolyPolygon aLinePolyPolygon, const LineInfo& rInfo )
{
    const bool bTryB2d(mpGraphics->supportsOperation(OutDevSupportType::B2DDraw)
        && RasterOp::OverPaint == GetRasterOp()
        && IsLineColor());
    basegfx::B2DPolyPolygon aFillPolyPolygon;
    const bool bDashUsed(LineStyle::Dash == rInfo.GetStyle());
    const bool bLineWidthUsed(rInfo.GetWidth() > 1);

    if(bDashUsed && aLinePolyPolygon.count())
    {
        ::std::vector< double > fDotDashArray;
        const double fDashLen(rInfo.GetDashLen());
        const double fDotLen(rInfo.GetDotLen());
        const double fDistance(rInfo.GetDistance());

        for(sal_uInt16 a(0); a < rInfo.GetDashCount(); a++)
        {
            fDotDashArray.push_back(fDashLen);
            fDotDashArray.push_back(fDistance);
        }

        for(sal_uInt16 b(0); b < rInfo.GetDotCount(); b++)
        {
            fDotDashArray.push_back(fDotLen);
            fDotDashArray.push_back(fDistance);
        }

        const double fAccumulated(::std::accumulate(fDotDashArray.begin(), fDotDashArray.end(), 0.0));

        if(fAccumulated > 0.0)
        {
            basegfx::B2DPolyPolygon aResult;

            for(auto const& rPolygon : std::as_const(aLinePolyPolygon))
            {
                basegfx::B2DPolyPolygon aLineTarget;
                basegfx::utils::applyLineDashing(
                    rPolygon,
                    fDotDashArray,
                    &aLineTarget);
                aResult.append(aLineTarget);
            }

            aLinePolyPolygon = aResult;
        }
    }

    if(bLineWidthUsed && aLinePolyPolygon.count())
    {
        const double fHalfLineWidth((rInfo.GetWidth() * 0.5) + 0.5);

        if(aLinePolyPolygon.areControlPointsUsed())
        {
            // #i110768# When area geometry has to be created, do not
            // use the fallback bezier decomposition inside createAreaGeometry,
            // but one that is at least as good as ImplSubdivideBezier was.
            // There, Polygon::AdaptiveSubdivide was used with default parameter
            // 1.0 as quality index.
            aLinePolyPolygon = basegfx::utils::adaptiveSubdivideByDistance(aLinePolyPolygon, 1.0);
        }

        for(auto const& rPolygon : std::as_const(aLinePolyPolygon))
        {
            aFillPolyPolygon.append(basegfx::utils::createAreaGeometry(
                rPolygon,
                fHalfLineWidth,
                rInfo.GetLineJoin(),
                rInfo.GetLineCap()));
        }

        aLinePolyPolygon.clear();
    }

    GDIMetaFile* pOldMetaFile = mpMetaFile;
    mpMetaFile = nullptr;

    if(aLinePolyPolygon.count())
    {
        for(auto const& rB2DPolygon : std::as_const(aLinePolyPolygon))
        {
            const bool bPixelSnapHairline(mnAntialiasing & AntialiasingFlags::PixelSnapHairline);
            bool bDone(false);

            if(bTryB2d)
            {
                bDone = mpGraphics->DrawPolyLine(
                    basegfx::B2DHomMatrix(),
                    rB2DPolygon,
                    0.0,
                    0.0, // tdf#124848 hairline
                    nullptr, // MM01
                    basegfx::B2DLineJoin::NONE,
                    css::drawing::LineCap_BUTT,
                    basegfx::deg2rad(15.0), // not used with B2DLineJoin::NONE, but the correct default
                    bPixelSnapHairline,
                    *this);
            }

            if(!bDone)
            {
                tools::Polygon aPolygon(rB2DPolygon);
                mpGraphics->DrawPolyLine(
                    aPolygon.GetSize(),
                    aPolygon.GetPointAry(),
                    *this);
            }
        }
    }

    if(aFillPolyPolygon.count())
    {
        const Color     aOldLineColor( maLineColor );
        const Color     aOldFillColor( maFillColor );

        SetLineColor();
        InitLineColor();
        SetFillColor( aOldLineColor );
        InitFillColor();

        bool bDone(false);

        if(bTryB2d)
        {
            bDone = mpGraphics->DrawPolyPolygon(
                basegfx::B2DHomMatrix(),
                aFillPolyPolygon,
                0.0,
                *this);
        }

        if(!bDone)
        {
            for(auto const& rB2DPolygon : std::as_const(aFillPolyPolygon))
            {
                tools::Polygon aPolygon(rB2DPolygon);

                // need to subdivide, mpGraphics->DrawPolygon ignores curves
                aPolygon.AdaptiveSubdivide(aPolygon);
                mpGraphics->DrawPolygon(aPolygon.GetSize(), aPolygon.GetConstPointAry(), *this);
            }
        }

        SetFillColor( aOldFillColor );
        SetLineColor( aOldLineColor );
    }

    mpMetaFile = pOldMetaFile;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
