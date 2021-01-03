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

#include <sal/types.h>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dlinegeometry.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/metaact.hxx>
#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>

#include <salgdi.hxx>

void OutputDevice::DrawPolyLine( const tools::Polygon& rPoly )
{
    assert(!is_double_buffered_window());

    if( mpMetaFile )
        mpMetaFile->AddAction( new MetaPolyLineAction( rPoly ) );

    sal_uInt16 nPoints = rPoly.GetSize();

    if ( !IsDeviceOutputNecessary() || !IsOpaqueLineColor() || (nPoints < 2) || ImplIsRecordLayout() )
        return;

    // we need a graphics
    if ( !mpGraphics && !AcquireGraphics() )
        return;

    if ( mbInitClipRegion )
        InitClipRegion();

    if ( mbOutputClipped )
        return;

    if ( IsInitLineColor() )
        InitLineColor();

    // use b2dpolygon drawing if possible
    if(DrawPolyLineDirectInternal(
        basegfx::B2DHomMatrix(),
        rPoly.getB2DPolygon()))
    {
        return;
    }

    const basegfx::B2DPolygon aB2DPolyLine(rPoly.getB2DPolygon());
    const basegfx::B2DHomMatrix aTransform(ImplGetDeviceTransformation());
    const bool bPixelSnapHairline(mnAntialiasing & AntialiasingFlags::PixelSnapHairline);

    mpGraphics->DrawPolyLine(
        aTransform,
        aB2DPolyLine,
        0.0,
        0.0, // tdf#124848 hairline
        nullptr, // MM01
        basegfx::B2DLineJoin::NONE,
        css::drawing::LineCap_BUTT,
        basegfx::deg2rad(15.0) /*default fMiterMinimumAngle, not used*/,
        bPixelSnapHairline,
        *this);

    if( mpAlphaVDev )
        mpAlphaVDev->DrawPolyLine( rPoly );
}

void OutputDevice::DrawPolyLine( const tools::Polygon& rPoly, const LineInfo& rLineInfo )
{
    assert(!is_double_buffered_window());

    if ( rLineInfo.IsDefault() )
    {
        DrawPolyLine( rPoly );
        return;
    }

    // #i101491#
    // Try direct Fallback to B2D-Version of DrawPolyLine
    if(LineStyle::Solid == rLineInfo.GetStyle())
    {
        DrawPolyLine(
            rPoly.getB2DPolygon(),
            static_cast< double >(rLineInfo.GetWidth()),
            rLineInfo.GetLineJoin(),
            rLineInfo.GetLineCap(),
            basegfx::deg2rad(15.0) /* default fMiterMinimumAngle, value not available in LineInfo */);
        return;
    }

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaPolyLineAction( rPoly, rLineInfo ) );

    drawPolyLine(rPoly, rLineInfo);
}

void OutputDevice::DrawPolyLine( const basegfx::B2DPolygon& rB2DPolygon,
                                 double fLineWidth,
                                 basegfx::B2DLineJoin eLineJoin,
                                 css::drawing::LineCap eLineCap,
                                 double fMiterMinimumAngle)
{
    assert(!is_double_buffered_window());

    if( mpMetaFile )
    {
        LineInfo aLineInfo;
        if( fLineWidth != 0.0 )
            aLineInfo.SetWidth( static_cast<tools::Long>(fLineWidth+0.5) );

        const tools::Polygon aToolsPolygon( rB2DPolygon );
        mpMetaFile->AddAction( new MetaPolyLineAction( aToolsPolygon, aLineInfo ) );
    }

    // Do not paint empty PolyPolygons
    if(!rB2DPolygon.count() || !IsDeviceOutputNecessary())
        return;

    // we need a graphics
    if( !mpGraphics && !AcquireGraphics() )
        return;

    if( mbInitClipRegion )
        InitClipRegion();

    if( mbOutputClipped )
        return;

    if( IsInitLineColor() )
        InitLineColor();

    // use b2dpolygon drawing if possible
    if(DrawPolyLineDirectInternal(
        basegfx::B2DHomMatrix(),
        rB2DPolygon,
        fLineWidth,
        0.0,
        nullptr, // MM01
        eLineJoin,
        eLineCap,
        fMiterMinimumAngle))
    {
        return;
    }

    // #i101491#
    // no output yet; fallback to geometry decomposition and use filled polygon paint
    // when line is fat and not too complex. ImplDrawPolyPolygonWithB2DPolyPolygon
    // will do internal needed AA checks etc.
    if(fLineWidth >= 2.5 &&
       rB2DPolygon.count() &&
       rB2DPolygon.count() <= 1000)
    {
        const double fHalfLineWidth((fLineWidth * 0.5) + 0.5);
        const basegfx::B2DPolyPolygon aAreaPolyPolygon(
                basegfx::utils::createAreaGeometry( rB2DPolygon,
                                                    fHalfLineWidth,
                                                    eLineJoin,
                                                    eLineCap,
                                                    fMiterMinimumAngle));
        const Color aOldLineColor(GetLineColor());
        const Color aOldFillColor(GetFillColor());

        SetLineColor();
        InitLineColor();
        SetFillColor(aOldLineColor);
        InitFillColor();

        // draw using a loop; else the topology will paint a PolyPolygon
        for(auto const& rPolygon : aAreaPolyPolygon)
        {
            ImplDrawPolyPolygonWithB2DPolyPolygon(
                basegfx::B2DPolyPolygon(rPolygon));
        }

        SetLineColor(aOldLineColor);
        InitLineColor();
        SetFillColor(aOldFillColor);
        InitFillColor();

        // when AA it is necessary to also paint the filled polygon's outline
        // to avoid optical gaps
        for(auto const& rPolygon : aAreaPolyPolygon)
        {
            (void)DrawPolyLineDirectInternal(
                basegfx::B2DHomMatrix(),
                rPolygon);
        }
    }
    else
    {
        // fallback to old polygon drawing if needed
        const tools::Polygon aToolsPolygon( rB2DPolygon );
        LineInfo aLineInfo;
        if( fLineWidth != 0.0 )
            aLineInfo.SetWidth( static_cast<tools::Long>(fLineWidth+0.5) );

        drawPolyLine( aToolsPolygon, aLineInfo );
    }
}

void OutputDevice::drawPolyLine(const tools::Polygon& rPoly, const LineInfo& rLineInfo)
{
    sal_uInt16 nPoints(rPoly.GetSize());

    if ( !IsDeviceOutputNecessary() || !IsOpaqueLineColor() || ( nPoints < 2 ) || ( LineStyle::NONE == rLineInfo.GetStyle() ) || ImplIsRecordLayout() )
        return;

    tools::Polygon aPoly = ImplLogicToDevicePixel( rPoly );

    // we need a graphics
    if ( !mpGraphics && !AcquireGraphics() )
        return;

    if ( mbInitClipRegion )
        InitClipRegion();

    if ( mbOutputClipped )
        return;

    if ( IsInitLineColor() )
        InitLineColor();

    const LineInfo aInfo( ImplLogicToDevicePixel( rLineInfo ) );
    const bool bDashUsed(LineStyle::Dash == aInfo.GetStyle());
    const bool bLineWidthUsed(aInfo.GetWidth() > 1);

    if(bDashUsed || bLineWidthUsed)
    {
        drawLine ( basegfx::B2DPolyPolygon(aPoly.getB2DPolygon()), aInfo );
    }
    else
    {
        // #100127# the subdivision HAS to be done here since only a pointer
        // to an array of points is given to the DrawPolyLine method, there is
        // NO way to find out there that it's a curve.
        if( aPoly.HasFlags() )
        {
            aPoly = tools::Polygon::SubdivideBezier( aPoly );
            nPoints = aPoly.GetSize();
        }

        mpGraphics->DrawPolyLine(nPoints, aPoly.GetPointAry(), *this);
    }

    if( mpAlphaVDev )
        mpAlphaVDev->DrawPolyLine( rPoly, rLineInfo );
}

bool OutputDevice::DrawPolyLineDirect(
    const basegfx::B2DHomMatrix& rObjectTransform,
    const basegfx::B2DPolygon& rB2DPolygon,
    double fLineWidth,
    double fTransparency,
    const std::vector< double >* pStroke, // MM01
    basegfx::B2DLineJoin eLineJoin,
    css::drawing::LineCap eLineCap,
    double fMiterMinimumAngle)
{
    assert(!is_double_buffered_window());

    // AW: Do NOT paint empty PolyPolygons
    if (!rB2DPolygon.count())
        return true;

    // we need a graphics
    if (!mpGraphics && !AcquireGraphics())
        return false;

    if (mbInitClipRegion)
        InitClipRegion();

    if (mbOutputClipped)
        return true;

    if (IsInitLineColor())
        InitLineColor();

    if(DrawPolyLineDirectInternal(rObjectTransform, rB2DPolygon, fLineWidth, fTransparency,
        pStroke, eLineJoin, eLineCap, fMiterMinimumAngle))
    {
        // Worked, add metafile action (if recorded). This is done only here,
        // because this function is public, other OutDev functions already add metafile
        // actions, so they call the internal function directly.
        if( mpMetaFile )
        {
            LineInfo aLineInfo;
            if( fLineWidth != 0.0 )
                aLineInfo.SetWidth( static_cast<tools::Long>(fLineWidth+0.5) );
            // Transport known information, might be needed
            aLineInfo.SetLineJoin(eLineJoin);
            aLineInfo.SetLineCap(eLineCap);
            // MiterMinimumAngle does not exist yet in LineInfo
            const tools::Polygon aToolsPolygon( rB2DPolygon );
            mpMetaFile->AddAction( new MetaPolyLineAction( aToolsPolygon, aLineInfo ) );
        }
        return true;
    }
    return false;
}

bool OutputDevice::DrawPolyLineDirectInternal(
    const basegfx::B2DHomMatrix& rObjectTransform,
    const basegfx::B2DPolygon& rB2DPolygon,
    double fLineWidth,
    double fTransparency,
    const std::vector< double >* pStroke, // MM01
    basegfx::B2DLineJoin eLineJoin,
    css::drawing::LineCap eLineCap,
    double fMiterMinimumAngle)
{
    const bool bTryB2d(mpGraphics->supportsOperation(OutDevSupportType::B2DDraw) &&
                      RasterOp::OverPaint == GetRasterOp() &&
                      IsOpaqueLineColor());

    if(bTryB2d)
    {
        // combine rObjectTransform with WorldToDevice
        const basegfx::B2DHomMatrix aTransform(ImplGetDeviceTransformation() * rObjectTransform);
        const bool bPixelSnapHairline((mnAntialiasing & AntialiasingFlags::PixelSnapHairline) && rB2DPolygon.count() < 1000);

        const double fAdjustedTransparency = mpAlphaVDev ? 0 : fTransparency;
        // draw the polyline
        mpGraphics->DrawPolyLine(
            aTransform,
            rB2DPolygon,
            fAdjustedTransparency,
            fLineWidth, // tdf#124848 use LineWidth direct, do not try to solve for zero-case (aka hairline)
            pStroke, // MM01
            eLineJoin,
            eLineCap,
            fMiterMinimumAngle,
            bPixelSnapHairline,
            *this);

        if (mpAlphaVDev)
            mpAlphaVDev->DrawPolyLineDirect(rObjectTransform, rB2DPolygon, fLineWidth,
                                            fTransparency, pStroke, eLineJoin, eLineCap,
                                            fMiterMinimumAngle);

        return true;
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
