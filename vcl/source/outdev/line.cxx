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

#include <cassert>
#include <numeric>

#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>
#include <vcl/window.hxx>

#include <salgdi.hxx>

#include "outdata.hxx"

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dlinegeometry.hxx>

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

    if ( !IsDeviceOutputNecessary() || !mbLineColor || ( LINE_NONE == rLineInfo.GetStyle() ) || ImplIsRecordLayout() )
        return;

    if( !mpGraphics )
    {
        if ( !AcquireGraphics() )
            return;
    }

    if ( mbInitClipRegion )
        InitClipRegion();

    if ( mbOutputClipped )
        return;

    const Point aStartPt( ImplLogicToDevicePixel( rStartPt ) );
    const Point aEndPt( ImplLogicToDevicePixel( rEndPt ) );
    const LineInfo aInfo( ImplLogicToDevicePixel( rLineInfo ) );
    const bool bDashUsed(LINE_DASH == aInfo.GetStyle());
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
        mpGraphics->DrawLine( aStartPt.X(), aStartPt.Y(), aEndPt.X(), aEndPt.Y(), this );
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

    if ( !mpGraphics )
    {
        if ( !AcquireGraphics() )
            return;
    }

    if ( mbInitClipRegion )
        InitClipRegion();

    if ( mbOutputClipped )
        return;

    if ( mbInitLineColor )
        InitLineColor();

    // #i101598# support AA and snap for lines, too
    if((mnAntialiasing & AntialiasingFlags::EnableB2dDraw)
        && mpGraphics->supportsOperation(OutDevSupport_B2DDraw)
        && ROP_OVERPAINT == GetRasterOp()
        && IsLineColor())
    {
        // at least transform with double precision to device coordinates; this will
        // avoid pixel snap of single, appended lines
        const basegfx::B2DHomMatrix aTransform(ImplGetDeviceTransformation());
        const basegfx::B2DVector aB2DLineWidth( 1.0, 1.0 );
        basegfx::B2DPolygon aB2DPolyLine;

        aB2DPolyLine.append(basegfx::B2DPoint(rStartPt.X(), rStartPt.Y()));
        aB2DPolyLine.append(basegfx::B2DPoint(rEndPt.X(), rEndPt.Y()));
        aB2DPolyLine.transform( aTransform );

        if(mnAntialiasing & AntialiasingFlags::PixelSnapHairline)
        {
            aB2DPolyLine = basegfx::tools::snapPointsOfHorizontalOrVerticalEdges(aB2DPolyLine);
        }

        if( mpGraphics->DrawPolyLine(
            aB2DPolyLine,
            0.0,
            aB2DLineWidth,
            basegfx::B2DLineJoin::NONE,
            css::drawing::LineCap_BUTT,
            15.0 * F_PI180, // not used with B2DLineJoin::NONE, but the correct default
            this))
        {
            return;
        }
    }

    const Point aStartPt(ImplLogicToDevicePixel(rStartPt));
    const Point aEndPt(ImplLogicToDevicePixel(rEndPt));

    mpGraphics->DrawLine( aStartPt.X(), aStartPt.Y(), aEndPt.X(), aEndPt.Y(), this );

    if( mpAlphaVDev )
        mpAlphaVDev->DrawLine( rStartPt, rEndPt );
}

void OutputDevice::drawLine( basegfx::B2DPolyPolygon aLinePolyPolygon, const LineInfo& rInfo )
{
    const bool bTryAA((mnAntialiasing & AntialiasingFlags::EnableB2dDraw)
        && mpGraphics->supportsOperation(OutDevSupport_B2DDraw)
        && ROP_OVERPAINT == GetRasterOp()
        && IsLineColor());
    basegfx::B2DPolyPolygon aFillPolyPolygon;
    const bool bDashUsed(LINE_DASH == rInfo.GetStyle());
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

            for(sal_uInt32 c(0); c < aLinePolyPolygon.count(); c++)
            {
                basegfx::B2DPolyPolygon aLineTarget;
                basegfx::tools::applyLineDashing(
                    aLinePolyPolygon.getB2DPolygon(c),
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
            aLinePolyPolygon = basegfx::tools::adaptiveSubdivideByDistance(aLinePolyPolygon, 1.0);
        }

        for(sal_uInt32 a(0); a < aLinePolyPolygon.count(); a++)
        {
            aFillPolyPolygon.append(basegfx::tools::createAreaGeometry(
                aLinePolyPolygon.getB2DPolygon(a),
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
        for(sal_uInt32 a(0); a < aLinePolyPolygon.count(); a++)
        {
            const basegfx::B2DPolygon aCandidate(aLinePolyPolygon.getB2DPolygon(a));
            bool bDone(false);

            if(bTryAA)
            {
                bDone = mpGraphics->DrawPolyLine(
                    aCandidate,
                    0.0,
                    basegfx::B2DVector(1.0,1.0),
                    basegfx::B2DLineJoin::NONE,
                    css::drawing::LineCap_BUTT,
                    15.0 * F_PI180, // not used with B2DLineJoin::NONE, but the correct default
                    this);
            }

            if(!bDone)
            {
                const tools::Polygon aPolygon(aCandidate);
                mpGraphics->DrawPolyLine(aPolygon.GetSize(), reinterpret_cast<const SalPoint*>(aPolygon.GetConstPointAry()), this);
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

        if(bTryAA)
        {
            bDone = mpGraphics->DrawPolyPolygon(aFillPolyPolygon, 0.0, this);
        }

        if(!bDone)
        {
            for(sal_uInt32 a(0); a < aFillPolyPolygon.count(); a++)
            {
                tools::Polygon aPolygon(aFillPolyPolygon.getB2DPolygon(a));

                // need to subdivide, mpGraphics->DrawPolygon ignores curves
                aPolygon.AdaptiveSubdivide(aPolygon);
                mpGraphics->DrawPolygon(aPolygon.GetSize(), reinterpret_cast<const SalPoint*>(aPolygon.GetConstPointAry()), this);
            }
        }

        SetFillColor( aOldFillColor );
        SetLineColor( aOldLineColor );
    }

    mpMetaFile = pOldMetaFile;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
