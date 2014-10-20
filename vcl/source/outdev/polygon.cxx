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
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <boost/scoped_array.hpp>
#include <tools/poly.hxx>
#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>

#include "salgdi.hxx"


#define OUTDEV_POLYPOLY_STACKBUF        32

void OutputDevice::DrawPolyPolygon( const tools::PolyPolygon& rPolyPoly )
{

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
    if((mnAntialiasing & ANTIALIASING_ENABLE_B2DDRAW) &&
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
            bSuccess = drawPolyPolygon(aB2DPolyPolygon, 0.0);
        }

        if(bSuccess && IsLineColor())
        {
            const ::basegfx::B2DVector aB2DLineWidth( 1.0, 1.0 );

            if(mnAntialiasing & ANTIALIASING_PIXELSNAPHAIRLINE)
            {
                aB2DPolyPolygon = basegfx::tools::snapPointsOfHorizontalOrVerticalEdges(aB2DPolyPolygon);
            }

            for(sal_uInt32 a(0); bSuccess && a < aB2DPolyPolygon.count(); a++)
            {
                bSuccess = drawPolyLine( aB2DPolyPolygon.getB2DPolygon(a),
                                         0.0,
                                         aB2DLineWidth,
                                         basegfx::B2DLINEJOIN_NONE,
                                         css::drawing::LineCap_BUTT );
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
        Polygon aPoly = rPolyPoly.GetObject( 0 );
        if( aPoly.GetSize() >= 2 )
        {
            GDIMetaFile* pOldMF = mpMetaFile;
            mpMetaFile = NULL;

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

void OutputDevice::drawPolyPolygon( sal_uInt32 nPoly, const sal_uInt32* pPoints, PCONSTSALPOINT* pPtAry )
{
    if ( !mpGraphics && !AcquireGraphics() )
        return;

    if( (mpGraphics->GetLayout() & SAL_LAYOUT_BIDI_RTL) ||  IsRTLEnabled() )
    {
        // TODO: optimize, reduce new/delete calls
        SalPoint **pPtAry2 = new SalPoint*[nPoly];
        sal_uLong i;
        for(i=0; i<nPoly; i++)
        {
            sal_uLong nPoints = pPoints[i];
            pPtAry2[i] = new SalPoint[ nPoints ];
            mirror( nPoints, pPtAry[i], pPtAry2[i] );
        }

        mpGraphics->DrawPolyPolygon( nPoly, pPoints, (PCONSTSALPOINT*)pPtAry2 );

        for(i=0; i<nPoly; i++)
            delete [] pPtAry2[i];
        delete [] pPtAry2;
    }
    else
    {
        mpGraphics->DrawPolyPolygon( nPoly, pPoints, pPtAry );
    }
}

bool OutputDevice::drawPolyPolygon( const basegfx::B2DPolyPolygon& rPolyPolygon, double fTransparency )
{
    if ( !mpGraphics && !AcquireGraphics() )
        return false;

    bool bRet = false;

    if( (mpGraphics->GetLayout() & SAL_LAYOUT_BIDI_RTL) ||  IsRTLEnabled() )
    {
        basegfx::B2DPolyPolygon aMirror( mirror( rPolyPolygon ) );
        bRet = mpGraphics->DrawPolyPolygon( aMirror, fTransparency );
    }
    else
    {
        bRet = mpGraphics->DrawPolyPolygon( rPolyPolygon, fTransparency );
    }

    return bRet;
}

bool OutputDevice::DrawPolyPolygonBezier( sal_uInt32 nPoly, const sal_uInt32* pPoints,
                                          const SalPoint* const* pPtAry, const sal_uInt8* const* pFlgAry )
{
    if ( !mpGraphics && !AcquireGraphics() )
        return false;

    bool bRet = false;

    if( (mpGraphics->GetLayout() & SAL_LAYOUT_BIDI_RTL) ||  IsRTLEnabled() )
    {
        // TODO: optimize, reduce new/delete calls
        SalPoint **pPtAry2 = new SalPoint*[nPoly];
        sal_uLong i;
        for(i=0; i<nPoly; i++)
        {
            sal_uLong nPoints = pPoints[i];
            pPtAry2[i] = new SalPoint[ nPoints ];
            mirror( nPoints, pPtAry[i], pPtAry2[i] );
        }

        bRet = mpGraphics->DrawPolyPolygonBezier( nPoly, pPoints, (PCONSTSALPOINT*)pPtAry2, pFlgAry );

        for(i=0; i<nPoly; i++)
            delete [] pPtAry2[i];
        delete [] pPtAry2;
    }
    else
    {
        bRet = mpGraphics->DrawPolyPolygonBezier( nPoly, pPoints, pPtAry, pFlgAry );
    }

    return bRet;
}

void OutputDevice::DrawPolygon( const basegfx::B2DPolygon& rB2DPolygon)
{
    // AW: Do NOT paint empty polygons
    if(rB2DPolygon.count())
    {
        basegfx::B2DPolyPolygon aPP( rB2DPolygon );
        DrawPolyPolygon( aPP );
    }
}

void OutputDevice::DrawPolygon( const Polygon& rPoly )
{

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
    if((mnAntialiasing & ANTIALIASING_ENABLE_B2DDRAW) &&
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
            bSuccess = drawPolyPolygon(basegfx::B2DPolyPolygon(aB2DPolygon), 0.0);
        }

        if(bSuccess && IsLineColor())
        {
            const ::basegfx::B2DVector aB2DLineWidth( 1.0, 1.0 );

            if(mnAntialiasing & ANTIALIASING_PIXELSNAPHAIRLINE)
            {
                aB2DPolygon = basegfx::tools::snapPointsOfHorizontalOrVerticalEdges(aB2DPolygon);
            }

            bSuccess = drawPolyLine( aB2DPolygon,
                                     0.0,
                                     aB2DLineWidth,
                                     basegfx::B2DLINEJOIN_NONE,
                                     css::drawing::LineCap_BUTT );
        }

        if(bSuccess)
        {
            return;
        }
    }

    Polygon aPoly = ImplLogicToDevicePixel( rPoly );
    const SalPoint* pPtAry = (const SalPoint*)aPoly.GetConstPointAry();

    // #100127# Forward beziers to sal, if any
    if( aPoly.HasFlags() )
    {
        const sal_uInt8* pFlgAry = aPoly.GetConstFlagAry();
        if( !DrawPolygonBezier( nPoints, pPtAry, pFlgAry ) )
        {
            aPoly = Polygon::SubdivideBezier(aPoly);
            pPtAry = (const SalPoint*)aPoly.GetConstPointAry();
            drawPolygon( aPoly.GetSize(), pPtAry );
        }
    }
    else
    {
        drawPolygon( nPoints, pPtAry );
    }
    if( mpAlphaVDev )
        mpAlphaVDev->DrawPolygon( rPoly );
}

void OutputDevice::drawPolygon( sal_uInt16 nPoints, const SalPoint* pPtAry  )
{
    if ( !mpGraphics && !AcquireGraphics() )
        return;

    if( (mpGraphics->GetLayout() & SAL_LAYOUT_BIDI_RTL) || IsRTLEnabled() )
    {
        boost::scoped_array<SalPoint> pPtAry2(new SalPoint[nPoints]);
        bool bCopied = mirror( nPoints, pPtAry, pPtAry2.get() );

        mpGraphics->DrawPolygon( nPoints, bCopied ? pPtAry2.get() : pPtAry );
    }
}

bool OutputDevice::DrawPolygonBezier( sal_uInt32 nPoints, const SalPoint* pPtAry, const sal_uInt8* pFlgAry )
{
    if ( !mpGraphics && !AcquireGraphics() )
        return false;

    bool bResult = false;

    if( (mpGraphics->GetLayout() & SAL_LAYOUT_BIDI_RTL) || IsRTLEnabled() )
    {
        boost::scoped_array<SalPoint> pPtAry2(new SalPoint[nPoints]);
        bool bCopied = mirror( nPoints, pPtAry, pPtAry2.get() );
        bResult = mpGraphics->DrawPolygonBezier( nPoints, bCopied ? pPtAry2.get() : pPtAry, pFlgAry );
    }
    else
    {
        bResult = mpGraphics->DrawPolygonBezier( nPoints, pPtAry, pFlgAry );
    }

    return bResult;
}

// Caution: This method is nearly the same as
// OutputDevice::DrawTransparent( const basegfx::B2DPolyPolygon& rB2DPolyPoly, double fTransparency),
// so when changes are made here do not forget to make change sthere, too

void OutputDevice::DrawPolyPolygon( const basegfx::B2DPolyPolygon& rB2DPolyPoly )
{

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

    if((mnAntialiasing & ANTIALIASING_ENABLE_B2DDRAW) &&
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
            bSuccess = drawPolyPolygon(aB2DPolyPolygon, 0.0);
        }

        if(bSuccess && IsLineColor())
        {
            const ::basegfx::B2DVector aB2DLineWidth( 1.0, 1.0 );

            if(mnAntialiasing & ANTIALIASING_PIXELSNAPHAIRLINE)
            {
                aB2DPolyPolygon = basegfx::tools::snapPointsOfHorizontalOrVerticalEdges(aB2DPolyPolygon);
            }

            for(sal_uInt32 a(0);bSuccess && a < aB2DPolyPolygon.count(); a++)
            {
                bSuccess = drawPolyLine( aB2DPolyPolygon.getB2DPolygon(a),
                                         0.0,
                                         aB2DLineWidth,
                                         basegfx::B2DLINEJOIN_NONE,
                                         css::drawing::LineCap_BUTT );
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
        pFlagAryAry     = (const sal_uInt8**)aStackAry3;
    }

    do
    {
        const Polygon& rPoly = rPolyPoly.GetObject( i );
        sal_uInt16 nSize = rPoly.GetSize();
        if ( nSize )
        {
            pPointAry[j] = nSize;
            pPointAryAry[j] = (PCONSTSALPOINT)rPoly.GetConstPointAry();
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
            if( !DrawPolygonBezier( *pPointAry, *pPointAryAry, *pFlagAryAry ) )
            {
                Polygon aPoly = Polygon::SubdivideBezier( rPolyPoly.GetObject( last ) );
                drawPolygon( aPoly.GetSize(), (const SalPoint*)aPoly.GetConstPointAry() );
            }
        }
        else
        {
            drawPolygon( *pPointAry, *pPointAryAry );
        }
    }
    else
    {
        // #100127# Forward beziers to sal, if any
        if( bHaveBezier )
        {
            if( !DrawPolyPolygonBezier( j, pPointAry, pPointAryAry, pFlagAryAry ) )
            {
                tools::PolyPolygon aPolyPoly = tools::PolyPolygon::SubdivideBezier( rPolyPoly );
                ImplDrawPolyPolygon( aPolyPoly.Count(), aPolyPoly );
            }
        }
        else
        {
            drawPolyPolygon( j, pPointAry, pPointAryAry );
        }
    }

    if ( pPointAry != aStackAry1 )
    {
        delete[] pPointAry;
        delete[] pPointAryAry;
        delete[] pFlagAryAry;
    }
}

void OutputDevice::ImplDrawPolygon( const Polygon& rPoly, const tools::PolyPolygon* pClipPolyPoly )
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

        const SalPoint* pPtAry = (const SalPoint*)rPoly.GetConstPointAry();
        drawPolygon( nPoints, pPtAry );
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
        pPolyPoly = (tools::PolyPolygon*) &rPolyPoly;
    }
    if( pPolyPoly->Count() == 1 )
    {
        const Polygon rPoly = pPolyPoly->GetObject( 0 );
        sal_uInt16 nSize = rPoly.GetSize();

        if( nSize >= 2 )
        {
            const SalPoint* pPtAry = (const SalPoint*)rPoly.GetConstPointAry();
            drawPolygon( nSize, pPtAry );
        }
    }
    else if( pPolyPoly->Count() )
    {
        sal_uInt16 nCount = pPolyPoly->Count();
        boost::scoped_array<sal_uInt32> pPointAry(new sal_uInt32[nCount]);
        boost::scoped_array<PCONSTSALPOINT> pPointAryAry(new PCONSTSALPOINT[nCount]);
        sal_uInt16 i = 0;
        do
        {
            const Polygon& rPoly = pPolyPoly->GetObject( i );
            sal_uInt16 nSize = rPoly.GetSize();
            if ( nSize )
            {
                pPointAry[i] = nSize;
                pPointAryAry[i] = (PCONSTSALPOINT)rPoly.GetConstPointAry();
                i++;
            }
            else
                nCount--;
        }
        while( i < nCount );

        if( nCount == 1 )
            drawPolygon( pPointAry[0], pPointAryAry[0] );
        else
            drawPolyPolygon( nCount, pPointAry.get(), pPointAryAry.get() );
    }

    if( pClipPolyPoly )
        delete pPolyPoly;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
