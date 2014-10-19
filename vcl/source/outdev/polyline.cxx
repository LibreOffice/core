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
#include <sal/types.h>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dlinegeometry.hxx>

#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>
#include <vcl/settings.hxx>

#include "salgdi.hxx"

bool OutputDevice::drawPolyLine( const basegfx::B2DPolygon& rPolygon,
                                 double fTransparency,
                                 const basegfx::B2DVector& rLineWidth,
                                 basegfx::B2DLineJoin eLineJoin,
                                 com::sun::star::drawing::LineCap eLineCap )
{

    if ( !mpGraphics && !AcquireGraphics() )
        return false;

    bool bRet = false;
    if( (mpGraphics->GetLayout() & SAL_LAYOUT_BIDI_RTL) || IsRTLEnabled() )
    {
        basegfx::B2DPolygon aMirror( mirror( rPolygon ) );
        bRet = mpGraphics->DrawPolyLine( aMirror, fTransparency, rLineWidth, eLineJoin, eLineCap );
    }
    else
        bRet = mpGraphics->DrawPolyLine( rPolygon, fTransparency, rLineWidth, eLineJoin, eLineCap );

    return bRet;
}


void OutputDevice::DrawPolyLine( const Polygon& rPoly )
{

    if( mpMetaFile )
        mpMetaFile->AddAction( new MetaPolyLineAction( rPoly ) );

    sal_uInt16 nPoints = rPoly.GetSize();

    if ( !IsDeviceOutputNecessary() || !mbLineColor || (nPoints < 2) || ImplIsRecordLayout() )
        return;

    // we need a graphics
    if ( !mpGraphics && !AcquireGraphics() )
        return;

    if ( mbInitClipRegion )
        InitClipRegion();

    if ( mbOutputClipped )
        return;

    if ( mbInitLineColor )
        InitLineColor();

    const bool bTryAA( (mnAntialiasing & ANTIALIASING_ENABLE_B2DDRAW) &&
                       mpGraphics->supportsOperation(OutDevSupport_B2DDraw) &&
                       ROP_OVERPAINT == GetRasterOp() &&
                       IsLineColor());

    // use b2dpolygon drawing if possible
    if(bTryAA)
    {
        if ( TryDrawPolyLineDirectNoAACheck( rPoly.getB2DPolygon() ) )
        {
            basegfx::B2DPolygon aB2DPolyLine(rPoly.getB2DPolygon());
            const ::basegfx::B2DHomMatrix aTransform = ImplGetDeviceTransformation();
            const ::basegfx::B2DVector aB2DLineWidth( 1.0, 1.0 );

            // transform the polygon
            aB2DPolyLine.transform( aTransform );

            if(mnAntialiasing & ANTIALIASING_PIXELSNAPHAIRLINE)
            {
                aB2DPolyLine = basegfx::tools::snapPointsOfHorizontalOrVerticalEdges(aB2DPolyLine);
            }

            if(drawPolyLine( aB2DPolyLine, 0.0, aB2DLineWidth, basegfx::B2DLINEJOIN_NONE, css::drawing::LineCap_BUTT))
            {
                return;
            }
        }
    }

    Polygon aPoly = ImplLogicToDevicePixel( rPoly );
    const SalPoint* pPtAry = (const SalPoint*)aPoly.GetConstPointAry();

    // #100127# Forward beziers to sal, if any
    if( aPoly.HasFlags() )
    {
        const sal_uInt8* pFlgAry = aPoly.GetConstFlagAry();
        if( !DrawPolyLineBezier( nPoints, pPtAry, pFlgAry ) )
        {
            aPoly = Polygon::SubdivideBezier(aPoly);
            pPtAry = (const SalPoint*)aPoly.GetConstPointAry();
            mpGraphics->DrawPolyLine( aPoly.GetSize(), pPtAry );
        }
    }
    else
    {
        mpGraphics->DrawPolyLine( nPoints, pPtAry );
    }

    if( mpAlphaVDev )
        mpAlphaVDev->DrawPolyLine( rPoly );
}

bool OutputDevice::DrawPolyLineBezier( sal_uInt32 nPoints, const SalPoint* pPtAry, const sal_uInt8* pFlgAry )
{
    if ( !mpGraphics && !AcquireGraphics() )
        return false;

    bool bResult = false;
    if( (mpGraphics->GetLayout() & SAL_LAYOUT_BIDI_RTL) || IsRTLEnabled() )
    {
        boost::scoped_array<SalPoint> pPtAry2(new SalPoint[nPoints]);
        bool bCopied = mirror( nPoints, pPtAry, pPtAry2.get() );
        bResult = mpGraphics->DrawPolyLineBezier( nPoints, bCopied ? pPtAry2.get() : pPtAry, pFlgAry );
    }
    else
    {
        bResult = mpGraphics->DrawPolyLineBezier( nPoints, pPtAry, pFlgAry );
    }

    return bResult;
}

void OutputDevice::drawPolyLine( sal_uInt32 nPoints, const SalPoint* pPtAry )
{
    if ( !mpGraphics && !AcquireGraphics() )
        return;

    if( (mpGraphics->GetLayout() & SAL_LAYOUT_BIDI_RTL) || IsRTLEnabled() )
    {
        boost::scoped_array<SalPoint> pPtAry2(new SalPoint[nPoints]);
        bool bCopied = mirror( nPoints, pPtAry, pPtAry2.get() );
        mpGraphics->DrawPolyLine( nPoints, bCopied ? pPtAry2.get() : pPtAry );
    }
    else
    {
        mpGraphics->DrawPolyLine( nPoints, pPtAry );
    }
}

void OutputDevice::DrawPolyLine( const Polygon& rPoly, const LineInfo& rLineInfo )
{

    if ( rLineInfo.IsDefault() )
    {
        DrawPolyLine( rPoly );
        return;
    }

    // #i101491#
    // Try direct Fallback to B2D-Version of DrawPolyLine
    if((mnAntialiasing & ANTIALIASING_ENABLE_B2DDRAW) &&
       LINE_SOLID == rLineInfo.GetStyle())
    {
        DrawPolyLine( rPoly.getB2DPolygon(), (double)rLineInfo.GetWidth(), rLineInfo.GetLineJoin(), rLineInfo.GetLineCap());
        return;
    }

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaPolyLineAction( rPoly, rLineInfo ) );

    DrawPolyLineWithLineInfo(rPoly, rLineInfo);
}

void OutputDevice::DrawPolyLine( const basegfx::B2DPolygon& rB2DPolygon,
                                 double fLineWidth,
                                 basegfx::B2DLineJoin eLineJoin,
                                 css::drawing::LineCap eLineCap)
{
    if( mpMetaFile )
    {
        LineInfo aLineInfo;
        if( fLineWidth != 0.0 )
            aLineInfo.SetWidth( static_cast<long>(fLineWidth+0.5) );

        const Polygon aToolsPolygon( rB2DPolygon );
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

    if( mbInitLineColor )
        InitLineColor();

    const bool bTryAA((mnAntialiasing & ANTIALIASING_ENABLE_B2DDRAW) &&
                      mpGraphics->supportsOperation(OutDevSupport_B2DDraw) &&
                      ROP_OVERPAINT == GetRasterOp() &&
                      IsLineColor());

    // use b2dpolygon drawing if possible
    if(bTryAA)
    {
        if ( TryDrawPolyLineDirectNoAACheck(rB2DPolygon, fLineWidth, 0.0, eLineJoin, eLineCap) )
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
                basegfx::tools::createAreaGeometry( rB2DPolygon,
                                                    fHalfLineWidth,
                                                    eLineJoin,
                                                    eLineCap));
        const Color aOldLineColor(maLineColor);
        const Color aOldFillColor(maFillColor);

        SetLineColor();
        InitLineColor();
        SetFillColor(aOldLineColor);
        InitFillColor();

        // draw usig a loop; else the topology will paint a PolyPolygon
        for(sal_uInt32 a(0); a < aAreaPolyPolygon.count(); a++)
        {
            ImplDrawPolyPolygonWithB2DPolyPolygon(
                basegfx::B2DPolyPolygon(aAreaPolyPolygon.getB2DPolygon(a)));
        }

        SetLineColor(aOldLineColor);
        InitLineColor();
        SetFillColor(aOldFillColor);
        InitFillColor();

        if(bTryAA)
        {
            // when AA it is necessary to also paint the filled polygon's outline
            // to avoid optical gaps
            for(sal_uInt32 a(0); a < aAreaPolyPolygon.count(); a++)
            {
                TryDrawPolyLineDirectNoAACheck(aAreaPolyPolygon.getB2DPolygon(a));
            }
        }
    }
    else
    {
        // fallback to old polygon drawing if needed
        const Polygon aToolsPolygon( rB2DPolygon );
        LineInfo aLineInfo;
        if( fLineWidth != 0.0 )
            aLineInfo.SetWidth( static_cast<long>(fLineWidth+0.5) );

        DrawPolyLineWithLineInfo( aToolsPolygon, aLineInfo );
    }
}

void OutputDevice::DrawPolyLineWithLineInfo(const Polygon& rPoly, const LineInfo& rLineInfo)
{
    sal_uInt16 nPoints(rPoly.GetSize());

    if ( !IsDeviceOutputNecessary() || !mbLineColor || ( nPoints < 2 ) || ( LINE_NONE == rLineInfo.GetStyle() ) || ImplIsRecordLayout() )
        return;

    Polygon aPoly = ImplLogicToDevicePixel( rPoly );

    // we need a graphics
    if ( !mpGraphics && !AcquireGraphics() )
        return;

    if ( mbInitClipRegion )
        InitClipRegion();

    if ( mbOutputClipped )
        return;

    if ( mbInitLineColor )
        InitLineColor();

    const LineInfo aInfo( ImplLogicToDevicePixel( rLineInfo ) );
    const bool bDashUsed(LINE_DASH == aInfo.GetStyle());
    const bool bLineWidthUsed(aInfo.GetWidth() > 1);

    if(bDashUsed || bLineWidthUsed)
    {
        PaintLineGeometryWithEvtlExpand(aInfo, basegfx::B2DPolyPolygon(aPoly.getB2DPolygon()));
    }
    else
    {
        // #100127# the subdivision HAS to be done here since only a pointer
        // to an array of points is given to the DrawPolyLine method, there is
        // NO way to find out there that it's a curve.
        if( aPoly.HasFlags() )
        {
            aPoly = Polygon::SubdivideBezier( aPoly );
            nPoints = aPoly.GetSize();
        }

        mpGraphics->DrawPolyLine(nPoints, (const SalPoint*)aPoly.GetConstPointAry() );
    }

    if( mpAlphaVDev )
        mpAlphaVDev->DrawPolyLine( rPoly, rLineInfo );
}

bool OutputDevice::TryDrawPolyLineDirectNoAACheck( const basegfx::B2DPolygon& rB2DPolygon,
                                              double fLineWidth,
                                              double fTransparency,
                                              basegfx::B2DLineJoin eLineJoin,
                                              css::drawing::LineCap eLineCap)
{
    const basegfx::B2DHomMatrix aTransform = ImplGetDeviceTransformation();
    basegfx::B2DVector aB2DLineWidth(1.0, 1.0);

    // transform the line width if used
    if( fLineWidth != 0.0 )
    {
        aB2DLineWidth = aTransform * ::basegfx::B2DVector( fLineWidth, 0.0 );
    }

    // transform the polygon
    basegfx::B2DPolygon aB2DPolygon(rB2DPolygon);
    aB2DPolygon.transform(aTransform);

    if((mnAntialiasing & ANTIALIASING_PIXELSNAPHAIRLINE) &&
       aB2DPolygon.count() < 1000)
    {
        // #i98289#, #i101491#
        // better to remove doubles on device coordinates. Also assume from a given amount
        // of points that the single edges are not long enough to smooth
        aB2DPolygon.removeDoublePoints();
        aB2DPolygon = basegfx::tools::snapPointsOfHorizontalOrVerticalEdges(aB2DPolygon);
    }

    // draw the polyline
    return drawPolyLine( aB2DPolygon, fTransparency, aB2DLineWidth, eLineJoin, eLineCap );
}

bool OutputDevice::TryDrawPolyLineDirect( const basegfx::B2DPolygon& rB2DPolygon,
                                          double fLineWidth,
                                          double fTransparency,
                                          basegfx::B2DLineJoin eLineJoin,
                                          css::drawing::LineCap eLineCap)
{
    // AW: Do NOT paint empty PolyPolygons
    if(!rB2DPolygon.count())
        return true;

    // we need a graphics
    if( !mpGraphics && !AcquireGraphics() )
        return false;

    if( mbInitClipRegion )
        InitClipRegion();

    if( mbOutputClipped )
        return true;

    if( mbInitLineColor )
        InitLineColor();

    const bool bTryAA((mnAntialiasing & ANTIALIASING_ENABLE_B2DDRAW) &&
                      mpGraphics->supportsOperation(OutDevSupport_B2DDraw) &&
                      ROP_OVERPAINT == GetRasterOp() &&
                      IsLineColor());

    if(bTryAA)
    {
        if(TryDrawPolyLineDirectNoAACheck(rB2DPolygon, fLineWidth, fTransparency, eLineJoin, eLineCap))
        {
            // worked, add metafile action (if recorded) and return true
            if( mpMetaFile )
            {
                LineInfo aLineInfo;
                if( fLineWidth != 0.0 )
                    aLineInfo.SetWidth( static_cast<long>(fLineWidth+0.5) );

                const Polygon aToolsPolygon( rB2DPolygon );
                mpMetaFile->AddAction( new MetaPolyLineAction( aToolsPolygon, aLineInfo ) );
            }
            return true;
        }
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
