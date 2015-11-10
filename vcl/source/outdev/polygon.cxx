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
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <memory>
#include <tools/poly.hxx>
#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>
#include <vcl/window.hxx>

#include "salgdi.hxx"

#define OUTDEV_POLYPOLY_STACKBUF        32

void OutputDevice::DrawPolyPolygon( const tools::PolyPolygon& rPolyPoly )
{
    assert(!is_double_buffered_window());

    if( mpMetaFile )
        mpMetaFile->AddAction( new MetaPolyPolygonAction( rPolyPoly ) );

    sal_uInt16 nPoly = rPolyPoly.Count();

    if ( !IsDeviceOutputNecessary() || (!mbLineColor && !mbFillColor) || !nPoly || ImplIsRecordLayout() )
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

    if ( mbInitFillColor )
        InitFillColor();

    // use b2dpolygon drawing if possible
    if((mnAntialiasing & AntialiasingFlags::EnableB2dDraw) &&
       mpGraphics->supportsOperation(OutDevSupport_B2DDraw) &&
       ROP_OVERPAINT == GetRasterOp() &&
       (IsLineColor() || IsFillColor()))
    {
        const ::basegfx::B2DHomMatrix aTransform = ImplGetDeviceTransformation();
        basegfx::B2DPolyPolygon aB2DPolyPolygon(rPolyPoly.getB2DPolyPolygon());
        bool bSuccess(true);

        // transform the polygon and ensure closed
        aB2DPolyPolygon.transform(aTransform);
        aB2DPolyPolygon.setClosed(true);

        if(IsFillColor())
        {
            bSuccess = mpGraphics->DrawPolyPolygon(aB2DPolyPolygon, 0.0, this);
        }

        if(bSuccess && IsLineColor())
        {
            const ::basegfx::B2DVector aB2DLineWidth( 1.0, 1.0 );

            if(mnAntialiasing & AntialiasingFlags::PixelSnapHairline)
            {
                aB2DPolyPolygon = basegfx::tools::snapPointsOfHorizontalOrVerticalEdges(aB2DPolyPolygon);
            }

            for(sal_uInt32 a(0); bSuccess && a < aB2DPolyPolygon.count(); a++)
            {
                bSuccess = mpGraphics->DrawPolyLine( aB2DPolyPolygon.getB2DPolygon(a),
                                                     0.0,
                                                     aB2DLineWidth,
                                                     basegfx::B2DLineJoin::NONE,
                                                     css::drawing::LineCap_BUTT,
                                                     this);
            }
        }

        if(bSuccess)
        {
            return;
        }
    }

    if ( nPoly == 1 )
    {
        // #100127# Map to DrawPolygon
        tools::Polygon aPoly = rPolyPoly.GetObject( 0 );
        if( aPoly.GetSize() >= 2 )
        {
            GDIMetaFile* pOldMF = mpMetaFile;
            mpMetaFile = nullptr;

            DrawPolygon( aPoly );

            mpMetaFile = pOldMF;
        }
    }
    else
    {
        // #100127# moved real tools::PolyPolygon draw to separate method,
        // have to call recursively, avoiding duplicate
        // ImplLogicToDevicePixel calls
        ImplDrawPolyPolygon( nPoly, ImplLogicToDevicePixel( rPolyPoly ) );
    }
    if( mpAlphaVDev )
        mpAlphaVDev->DrawPolyPolygon( rPolyPoly );
}

void OutputDevice::DrawPolygon( const basegfx::B2DPolygon& rB2DPolygon)
{
    assert(!is_double_buffered_window());

    // AW: Do NOT paint empty polygons
    if(rB2DPolygon.count())
    {
        basegfx::B2DPolyPolygon aPP( rB2DPolygon );
        DrawPolyPolygon( aPP );
    }
}

void OutputDevice::DrawPolygon( const tools::Polygon& rPoly )
{
    assert(!is_double_buffered_window());

    if( mpMetaFile )
        mpMetaFile->AddAction( new MetaPolygonAction( rPoly ) );

    sal_uInt16 nPoints = rPoly.GetSize();

    if ( !IsDeviceOutputNecessary() || (!mbLineColor && !mbFillColor) || (nPoints < 2) || ImplIsRecordLayout() )
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

    if ( mbInitFillColor )
        InitFillColor();

    // use b2dpolygon drawing if possible
    if((mnAntialiasing & AntialiasingFlags::EnableB2dDraw) &&
       mpGraphics->supportsOperation(OutDevSupport_B2DDraw) &&
       ROP_OVERPAINT == GetRasterOp() &&
       (IsLineColor() || IsFillColor()))
    {
        const ::basegfx::B2DHomMatrix aTransform = ImplGetDeviceTransformation();
        basegfx::B2DPolygon aB2DPolygon(rPoly.getB2DPolygon());
        bool bSuccess(true);

        // transform the polygon and ensure closed
        aB2DPolygon.transform(aTransform);
        aB2DPolygon.setClosed(true);

        if(IsFillColor())
        {
            bSuccess = mpGraphics->DrawPolyPolygon(basegfx::B2DPolyPolygon(aB2DPolygon), 0.0, this);
        }

        if(bSuccess && IsLineColor())
        {
            const ::basegfx::B2DVector aB2DLineWidth( 1.0, 1.0 );

            if(mnAntialiasing & AntialiasingFlags::PixelSnapHairline)
            {
                aB2DPolygon = basegfx::tools::snapPointsOfHorizontalOrVerticalEdges(aB2DPolygon);
            }

            bSuccess = mpGraphics->DrawPolyLine( aB2DPolygon,
                                                 0.0,
                                                 aB2DLineWidth,
                                                 basegfx::B2DLineJoin::NONE,
                                                 css::drawing::LineCap_BUTT,
                                                 this);
        }

        if(bSuccess)
        {
            return;
        }
    }

    tools::Polygon aPoly = ImplLogicToDevicePixel( rPoly );
    const SalPoint* pPtAry = reinterpret_cast<const SalPoint*>(aPoly.GetConstPointAry());

    // #100127# Forward beziers to sal, if any
    if( aPoly.HasFlags() )
    {
        const sal_uInt8* pFlgAry = aPoly.GetConstFlagAry();
        if( !mpGraphics->DrawPolygonBezier( nPoints, pPtAry, pFlgAry, this ) )
        {
            aPoly = tools::Polygon::SubdivideBezier(aPoly);
            pPtAry = reinterpret_cast<const SalPoint*>(aPoly.GetConstPointAry());
            mpGraphics->DrawPolygon( aPoly.GetSize(), pPtAry, this );
        }
    }
    else
    {
        mpGraphics->DrawPolygon( nPoints, pPtAry, this );
    }
    if( mpAlphaVDev )
        mpAlphaVDev->DrawPolygon( rPoly );
}

// Caution: This method is nearly the same as
// OutputDevice::DrawTransparent( const basegfx::B2DPolyPolygon& rB2DPolyPoly, double fTransparency),
// so when changes are made here do not forget to make changes there, too

void OutputDevice::DrawPolyPolygon( const basegfx::B2DPolyPolygon& rB2DPolyPoly )
{
    assert(!is_double_buffered_window());

    if( mpMetaFile )
        mpMetaFile->AddAction( new MetaPolyPolygonAction( tools::PolyPolygon( rB2DPolyPoly ) ) );

    // call helper
    ImplDrawPolyPolygonWithB2DPolyPolygon(rB2DPolyPoly);
}

void OutputDevice::ImplDrawPolyPolygonWithB2DPolyPolygon(const basegfx::B2DPolyPolygon& rB2DPolyPoly)
{
    // Do not paint empty PolyPolygons
    if(!rB2DPolyPoly.count() || !IsDeviceOutputNecessary())
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

    if( mbInitFillColor )
        InitFillColor();

    if((mnAntialiasing & AntialiasingFlags::EnableB2dDraw) &&
       mpGraphics->supportsOperation(OutDevSupport_B2DDraw) &&
       ROP_OVERPAINT == GetRasterOp() &&
       (IsLineColor() || IsFillColor()))
    {
        const basegfx::B2DHomMatrix aTransform(ImplGetDeviceTransformation());
        basegfx::B2DPolyPolygon aB2DPolyPolygon(rB2DPolyPoly);
        bool bSuccess(true);

        // transform the polygon and ensure closed
        aB2DPolyPolygon.transform(aTransform);
        aB2DPolyPolygon.setClosed(true);

        if(IsFillColor())
        {
            bSuccess = mpGraphics->DrawPolyPolygon(aB2DPolyPolygon, 0.0, this);
        }

        if(bSuccess && IsLineColor())
        {
            const ::basegfx::B2DVector aB2DLineWidth( 1.0, 1.0 );

            if(mnAntialiasing & AntialiasingFlags::PixelSnapHairline)
            {
                aB2DPolyPolygon = basegfx::tools::snapPointsOfHorizontalOrVerticalEdges(aB2DPolyPolygon);
            }

            for(sal_uInt32 a(0);bSuccess && a < aB2DPolyPolygon.count(); a++)
            {
                bSuccess = mpGraphics->DrawPolyLine( aB2DPolyPolygon.getB2DPolygon(a),
                                                     0.0,
                                                     aB2DLineWidth,
                                                     basegfx::B2DLineJoin::NONE,
                                                     css::drawing::LineCap_BUTT,
                                                     this);
            }
        }

        if(bSuccess)
        {
            return;
        }
    }

    // fallback to old polygon drawing if needed
    const tools::PolyPolygon aToolsPolyPolygon( rB2DPolyPoly );
    const tools::PolyPolygon aPixelPolyPolygon = ImplLogicToDevicePixel( aToolsPolyPolygon );
    ImplDrawPolyPolygon( aPixelPolyPolygon.Count(), aPixelPolyPolygon );
}

// #100127# Extracted from OutputDevice::DrawPolyPolygon()
void OutputDevice::ImplDrawPolyPolygon( sal_uInt16 nPoly, const tools::PolyPolygon& rPolyPoly )
{
    // AW: This crashes on empty PolyPolygons, avoid that
    if(!nPoly)
        return;

    sal_uInt32 aStackAry1[OUTDEV_POLYPOLY_STACKBUF];
    PCONSTSALPOINT aStackAry2[OUTDEV_POLYPOLY_STACKBUF];
    sal_uInt8* aStackAry3[OUTDEV_POLYPOLY_STACKBUF];
    sal_uInt32* pPointAry;
    PCONSTSALPOINT*     pPointAryAry;
    const sal_uInt8** pFlagAryAry;
    sal_uInt16 i = 0;
    sal_uInt16 j = 0;
    sal_uInt16 last = 0;
    bool bHaveBezier = false;
    if ( nPoly > OUTDEV_POLYPOLY_STACKBUF )
    {
        pPointAry       = new sal_uInt32[nPoly];
        pPointAryAry    = new PCONSTSALPOINT[nPoly];
        pFlagAryAry     = new const sal_uInt8*[nPoly];
    }
    else
    {
        pPointAry       = aStackAry1;
        pPointAryAry    = aStackAry2;
        pFlagAryAry     = const_cast<const sal_uInt8**>(aStackAry3);
    }

    do
    {
        const tools::Polygon& rPoly = rPolyPoly.GetObject( i );
        sal_uInt16 nSize = rPoly.GetSize();
        if ( nSize )
        {
            pPointAry[j] = nSize;
            pPointAryAry[j] = reinterpret_cast<PCONSTSALPOINT>(rPoly.GetConstPointAry());
            pFlagAryAry[j] = rPoly.GetConstFlagAry();
            last = i;

            if( pFlagAryAry[j] )
                bHaveBezier = true;

            ++j;
        }
        ++i;
    }
    while ( i < nPoly );

    if ( j == 1 )
    {
        // #100127# Forward beziers to sal, if any
        if( bHaveBezier )
        {
            if( !mpGraphics->DrawPolygonBezier( *pPointAry, *pPointAryAry, *pFlagAryAry, this ) )
            {
                tools::Polygon aPoly = tools::Polygon::SubdivideBezier( rPolyPoly.GetObject( last ) );
                mpGraphics->DrawPolygon( aPoly.GetSize(), reinterpret_cast<const SalPoint*>(aPoly.GetConstPointAry()), this );
            }
        }
        else
        {
            mpGraphics->DrawPolygon( *pPointAry, *pPointAryAry, this );
        }
    }
    else
    {
        // #100127# Forward beziers to sal, if any
        if( bHaveBezier )
        {
            if( !mpGraphics->DrawPolyPolygonBezier( j, pPointAry, pPointAryAry, pFlagAryAry, this ) )
            {
                tools::PolyPolygon aPolyPoly = tools::PolyPolygon::SubdivideBezier( rPolyPoly );
                ImplDrawPolyPolygon( aPolyPoly.Count(), aPolyPoly );
            }
        }
        else
        {
            mpGraphics->DrawPolyPolygon( j, pPointAry, pPointAryAry, this );
        }
    }

    if ( pPointAry != aStackAry1 )
    {
        delete[] pPointAry;
        delete[] pPointAryAry;
        delete[] pFlagAryAry;
    }
}

void OutputDevice::ImplDrawPolygon( const tools::Polygon& rPoly, const tools::PolyPolygon* pClipPolyPoly )
{
    if( pClipPolyPoly )
    {
        ImplDrawPolyPolygon( rPoly, pClipPolyPoly );
    }
    else
    {
        sal_uInt16 nPoints = rPoly.GetSize();

        if ( nPoints < 2 )
            return;

        const SalPoint* pPtAry = reinterpret_cast<const SalPoint*>(rPoly.GetConstPointAry());
        mpGraphics->DrawPolygon( nPoints, pPtAry, this );
    }
}

void OutputDevice::ImplDrawPolyPolygon( const tools::PolyPolygon& rPolyPoly, const tools::PolyPolygon* pClipPolyPoly )
{
    tools::PolyPolygon* pPolyPoly;

    if( pClipPolyPoly )
    {
        pPolyPoly = new tools::PolyPolygon;
        rPolyPoly.GetIntersection( *pClipPolyPoly, *pPolyPoly );
    }
    else
    {
        pPolyPoly = const_cast<tools::PolyPolygon*>(&rPolyPoly);
    }
    if( pPolyPoly->Count() == 1 )
    {
        const tools::Polygon rPoly = pPolyPoly->GetObject( 0 );
        sal_uInt16 nSize = rPoly.GetSize();

        if( nSize >= 2 )
        {
            const SalPoint* pPtAry = reinterpret_cast<const SalPoint*>(rPoly.GetConstPointAry());
            mpGraphics->DrawPolygon( nSize, pPtAry, this );
        }
    }
    else if( pPolyPoly->Count() )
    {
        sal_uInt16 nCount = pPolyPoly->Count();
        std::unique_ptr<sal_uInt32[]> pPointAry(new sal_uInt32[nCount]);
        std::unique_ptr<PCONSTSALPOINT[]> pPointAryAry(new PCONSTSALPOINT[nCount]);
        sal_uInt16 i = 0;
        do
        {
            const tools::Polygon& rPoly = pPolyPoly->GetObject( i );
            sal_uInt16 nSize = rPoly.GetSize();
            if ( nSize )
            {
                pPointAry[i] = nSize;
                pPointAryAry[i] = reinterpret_cast<PCONSTSALPOINT>(rPoly.GetConstPointAry());
                i++;
            }
            else
                nCount--;
        }
        while( i < nCount );

        if( nCount == 1 )
            mpGraphics->DrawPolygon( pPointAry[0], pPointAryAry[0], this );
        else
            mpGraphics->DrawPolyPolygon( nCount, pPointAry.get(), pPointAryAry.get(), this );
    }

    if( pClipPolyPoly )
        delete pPolyPoly;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
