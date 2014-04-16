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

#include <tools/poly.hxx>

#include <vcl/hatch.hxx>
#include <vcl/virdev.hxx>
#include <vcl/outdev.hxx>

#include "salgdi.hxx"

#include <basegfx/vector/b2dvector.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>

#include <boost/scoped_array.hpp>

#define OUTDEV_POLYPOLY_STACKBUF        32

void OutputDevice::DrawPolyPolygon( const PolyPolygon& rPolyPoly )
{

    if( mpMetaFile )
        mpMetaFile->AddAction( new MetaPolyPolygonAction( rPolyPoly ) );

    sal_uInt16 nPoly = rPolyPoly.Count();

    if ( !IsDeviceOutputNecessary() || (!mbLineColor && !mbFillColor) || !nPoly || ImplIsRecordLayout() )
        return;

    // we need a graphics
    if ( !mpGraphics )
        if ( !ImplGetGraphics() )
            return;

    if ( mbInitClipRegion )
        ImplInitClipRegion();
    if ( mbOutputClipped )
        return;

    if ( mbInitLineColor )
        ImplInitLineColor();
    if ( mbInitFillColor )
        ImplInitFillColor();

    // use b2dpolygon drawing if possible
    if((mnAntialiasing & ANTIALIASING_ENABLE_B2DDRAW)
        && mpGraphics->supportsOperation(OutDevSupport_B2DDraw)
        && ROP_OVERPAINT == GetRasterOp()
        && (IsLineColor() || IsFillColor()))
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

            if(mnAntialiasing & ANTIALIASING_PIXELSNAPHAIRLINE)
            {
                aB2DPolyPolygon = basegfx::tools::snapPointsOfHorizontalOrVerticalEdges(aB2DPolyPolygon);
            }

            for(sal_uInt32 a(0); bSuccess && a < aB2DPolyPolygon.count(); a++)
            {
                bSuccess = mpGraphics->DrawPolyLine(
                    aB2DPolyPolygon.getB2DPolygon(a),
                    0.0,
                    aB2DLineWidth,
                    basegfx::B2DLINEJOIN_NONE,
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
        // #100127# moved real PolyPolygon draw to separate method,
        // have to call recursively, avoiding duplicate
        // ImplLogicToDevicePixel calls
        ImplDrawPolyPolygon( nPoly, ImplLogicToDevicePixel( rPolyPoly ) );
    }
    if( mpAlphaVDev )
        mpAlphaVDev->DrawPolyPolygon( rPolyPoly );
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
    if ( !mpGraphics )
        if ( !ImplGetGraphics() )
            return;

    if ( mbInitClipRegion )
        ImplInitClipRegion();
    if ( mbOutputClipped )
        return;

    if ( mbInitLineColor )
        ImplInitLineColor();
    if ( mbInitFillColor )
        ImplInitFillColor();

    // use b2dpolygon drawing if possible
    if((mnAntialiasing & ANTIALIASING_ENABLE_B2DDRAW)
        && mpGraphics->supportsOperation(OutDevSupport_B2DDraw)
        && ROP_OVERPAINT == GetRasterOp()
        && (IsLineColor() || IsFillColor()))
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

            if(mnAntialiasing & ANTIALIASING_PIXELSNAPHAIRLINE)
            {
                aB2DPolygon = basegfx::tools::snapPointsOfHorizontalOrVerticalEdges(aB2DPolygon);
            }

            bSuccess = mpGraphics->DrawPolyLine(
                aB2DPolygon,
                0.0,
                aB2DLineWidth,
                basegfx::B2DLINEJOIN_NONE,
                css::drawing::LineCap_BUTT,
                this);
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
        if( !mpGraphics->DrawPolygonBezier( nPoints, pPtAry, pFlgAry, this ) )
        {
            aPoly = Polygon::SubdivideBezier(aPoly);
            pPtAry = (const SalPoint*)aPoly.GetConstPointAry();
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
// so when changes are made here do not forget to make change sthere, too

void OutputDevice::DrawPolyPolygon( const basegfx::B2DPolyPolygon& rB2DPolyPoly )
{

    if( mpMetaFile )
        mpMetaFile->AddAction( new MetaPolyPolygonAction( PolyPolygon( rB2DPolyPoly ) ) );

    // call helper
    ImplDrawPolyPolygonWithB2DPolyPolygon(rB2DPolyPoly);
}

void OutputDevice::ImplDrawPolyPolygonWithB2DPolyPolygon(const basegfx::B2DPolyPolygon& rB2DPolyPoly)
{
    // Do not paint empty PolyPolygons
    if(!rB2DPolyPoly.count() || !IsDeviceOutputNecessary())
        return;

    // we need a graphics
    if( !mpGraphics )
        if( !ImplGetGraphics() )
            return;

    if( mbInitClipRegion )
        ImplInitClipRegion();
    if( mbOutputClipped )
        return;

    if( mbInitLineColor )
        ImplInitLineColor();
    if( mbInitFillColor )
        ImplInitFillColor();

    if((mnAntialiasing & ANTIALIASING_ENABLE_B2DDRAW)
        && mpGraphics->supportsOperation(OutDevSupport_B2DDraw)
        && ROP_OVERPAINT == GetRasterOp()
        && (IsLineColor() || IsFillColor()))
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

            if(mnAntialiasing & ANTIALIASING_PIXELSNAPHAIRLINE)
            {
                aB2DPolyPolygon = basegfx::tools::snapPointsOfHorizontalOrVerticalEdges(aB2DPolyPolygon);
            }

            for(sal_uInt32 a(0);bSuccess && a < aB2DPolyPolygon.count(); a++)
            {
                bSuccess = mpGraphics->DrawPolyLine(
                    aB2DPolyPolygon.getB2DPolygon(a),
                    0.0,
                    aB2DLineWidth,
                    basegfx::B2DLINEJOIN_NONE,
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
    const PolyPolygon aToolsPolyPolygon( rB2DPolyPoly );
    const PolyPolygon aPixelPolyPolygon = ImplLogicToDevicePixel( aToolsPolyPolygon );
    ImplDrawPolyPolygon( aPixelPolyPolygon.Count(), aPixelPolyPolygon );
}

// #100127# Extracted from OutputDevice::DrawPolyPolygon()
void OutputDevice::ImplDrawPolyPolygon( sal_uInt16 nPoly, const PolyPolygon& rPolyPoly )
{
    // AW: This crashes on empty PolyPolygons, avoid that
    if(!nPoly)
        return;

    sal_uInt32          aStackAry1[OUTDEV_POLYPOLY_STACKBUF];
    PCONSTSALPOINT      aStackAry2[OUTDEV_POLYPOLY_STACKBUF];
    sal_uInt8*              aStackAry3[OUTDEV_POLYPOLY_STACKBUF];
    sal_uInt32*         pPointAry;
    PCONSTSALPOINT*     pPointAryAry;
    const sal_uInt8**       pFlagAryAry;
    sal_uInt16              i = 0, j = 0, last = 0;
    bool                bHaveBezier = false;
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
        const Polygon&  rPoly = rPolyPoly.GetObject( i );
        sal_uInt16          nSize = rPoly.GetSize();
        if ( nSize )
        {
            pPointAry[j]    = nSize;
            pPointAryAry[j] = (PCONSTSALPOINT)rPoly.GetConstPointAry();
            pFlagAryAry[j]  = rPoly.GetConstFlagAry();
            last            = i;

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
                Polygon aPoly = Polygon::SubdivideBezier( rPolyPoly.GetObject( last ) );
                mpGraphics->DrawPolygon( aPoly.GetSize(), (const SalPoint*)aPoly.GetConstPointAry(), this );
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
                PolyPolygon aPolyPoly = PolyPolygon::SubdivideBezier( rPolyPoly );
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

void OutputDevice::ImplDrawPolygon( const Polygon& rPoly, const PolyPolygon* pClipPolyPoly )
{
    if( pClipPolyPoly )
        ImplDrawPolyPolygon( rPoly, pClipPolyPoly );
    else
    {
        sal_uInt16 nPoints = rPoly.GetSize();

        if ( nPoints < 2 )
            return;

        const SalPoint* pPtAry = (const SalPoint*)rPoly.GetConstPointAry();
        mpGraphics->DrawPolygon( nPoints, pPtAry, this );
    }
}

void OutputDevice::ImplDrawPolyPolygon( const PolyPolygon& rPolyPoly, const PolyPolygon* pClipPolyPoly )
{
    PolyPolygon* pPolyPoly;

    if( pClipPolyPoly )
    {
        pPolyPoly = new PolyPolygon;
        rPolyPoly.GetIntersection( *pClipPolyPoly, *pPolyPoly );
    }
    else
        pPolyPoly = (PolyPolygon*) &rPolyPoly;

    if( pPolyPoly->Count() == 1 )
    {
        const Polygon   rPoly = pPolyPoly->GetObject( 0 );
        sal_uInt16          nSize = rPoly.GetSize();

        if( nSize >= 2 )
        {
            const SalPoint* pPtAry = (const SalPoint*)rPoly.GetConstPointAry();
            mpGraphics->DrawPolygon( nSize, pPtAry, this );
        }
    }
    else if( pPolyPoly->Count() )
    {
        sal_uInt16              nCount = pPolyPoly->Count();
        boost::scoped_array<sal_uInt32> pPointAry(new sal_uInt32[nCount]);
        boost::scoped_array<PCONSTSALPOINT> pPointAryAry(new PCONSTSALPOINT[nCount]);
        sal_uInt16              i = 0;
        do
        {
            const Polygon&  rPoly = pPolyPoly->GetObject( i );
            sal_uInt16          nSize = rPoly.GetSize();
            if ( nSize )
            {
                pPointAry[i]    = nSize;
                pPointAryAry[i] = (PCONSTSALPOINT)rPoly.GetConstPointAry();
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

void OutputDevice::DrawGrid( const Rectangle& rRect, const Size& rDist, sal_uLong nFlags )
{

    Rectangle aDstRect( PixelToLogic( Point() ), GetOutputSize() );
    aDstRect.Intersection( rRect );

    if( aDstRect.IsEmpty() || ImplIsRecordLayout() )
        return;

    if( !mpGraphics && !ImplGetGraphics() )
        return;

    if( mbInitClipRegion )
        ImplInitClipRegion();

    if( mbOutputClipped )
        return;

    const long  nDistX = std::max( rDist.Width(), 1L );
    const long  nDistY = std::max( rDist.Height(), 1L );
    long        nX = ( rRect.Left() >= aDstRect.Left() ) ? rRect.Left() : ( rRect.Left() + ( ( aDstRect.Left() - rRect.Left() ) / nDistX ) * nDistX );
    long        nY = ( rRect.Top() >= aDstRect.Top() ) ? rRect.Top() : ( rRect.Top() + ( ( aDstRect.Top() - rRect.Top() ) / nDistY ) * nDistY );
    const long  nRight = aDstRect.Right();
    const long  nBottom = aDstRect.Bottom();
    const long  nStartX = ImplLogicXToDevicePixel( nX );
    const long  nEndX = ImplLogicXToDevicePixel( nRight );
    const long  nStartY = ImplLogicYToDevicePixel( nY );
    const long  nEndY = ImplLogicYToDevicePixel( nBottom );
    long        nHorzCount = 0L;
    long        nVertCount = 0L;

    ::com::sun::star::uno::Sequence< sal_Int32 > aVertBuf;
    ::com::sun::star::uno::Sequence< sal_Int32 > aHorzBuf;

    if( ( nFlags & GRID_DOTS ) || ( nFlags & GRID_HORZLINES ) )
    {
        aVertBuf.realloc( aDstRect.GetHeight() / nDistY + 2L );
        aVertBuf[ nVertCount++ ] = nStartY;
        while( ( nY += nDistY ) <= nBottom )
            aVertBuf[ nVertCount++ ] = ImplLogicYToDevicePixel( nY );
    }

    if( ( nFlags & GRID_DOTS ) || ( nFlags & GRID_VERTLINES ) )
    {
        aHorzBuf.realloc( aDstRect.GetWidth() / nDistX + 2L );
        aHorzBuf[ nHorzCount++ ] = nStartX;
        while( ( nX += nDistX ) <= nRight )
            aHorzBuf[ nHorzCount++ ] = ImplLogicXToDevicePixel( nX );
    }

    if( mbInitLineColor )
        ImplInitLineColor();

    if( mbInitFillColor )
        ImplInitFillColor();

    const bool bOldMap = mbMap;
    EnableMapMode( false );

    if( nFlags & GRID_DOTS )
    {
        for( long i = 0L; i < nVertCount; i++ )
            for( long j = 0L, Y = aVertBuf[ i ]; j < nHorzCount; j++ )
                mpGraphics->DrawPixel( aHorzBuf[ j ], Y, this );
    }
    else
    {
        if( nFlags & GRID_HORZLINES )
        {
            for( long i = 0L; i < nVertCount; i++ )
            {
                nY = aVertBuf[ i ];
                mpGraphics->DrawLine( nStartX, nY, nEndX, nY, this );
            }
        }

        if( nFlags & GRID_VERTLINES )
        {
            for( long i = 0L; i < nHorzCount; i++ )
            {
                nX = aHorzBuf[ i ];
                mpGraphics->DrawLine( nX, nStartY, nX, nEndY, this );
            }
        }
    }

    EnableMapMode( bOldMap );

    if( mpAlphaVDev )
        mpAlphaVDev->DrawGrid( rRect, rDist, nFlags );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
