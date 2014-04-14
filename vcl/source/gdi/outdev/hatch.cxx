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

#include <tools/debug.hxx>
#include <tools/line.hxx>
#include <tools/poly.hxx>

#include <vcl/gradient.hxx>
#include <vcl/metaact.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/salbtype.hxx>
#include <vcl/hatch.hxx>
#include <vcl/window.hxx>
#include <vcl/virdev.hxx>
#include <vcl/outdev.hxx>
#include <vcl/settings.hxx>

#include "../pdfwriter_impl.hxx"

#include "window.h"
#include "salframe.hxx"
#include "salgdi.hxx"
#include "svdata.hxx"
#include "outdata.hxx"

#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <boost/scoped_array.hpp>
#include <boost/scoped_ptr.hpp>

#define HATCH_MAXPOINTS             1024

extern "C" int SAL_CALL ImplHatchCmpFnc( const void* p1, const void* p2 )
{
    const long nX1 = ( (Point*) p1 )->X();
    const long nX2 = ( (Point*) p2 )->X();
    const long nY1 = ( (Point*) p1 )->Y();
    const long nY2 = ( (Point*) p2 )->Y();

    return ( nX1 > nX2 ? 1 : nX1 == nX2 ? nY1 > nY2 ? 1: nY1 == nY2 ? 0 : -1 : -1 );
}

void OutputDevice::DrawHatch( const PolyPolygon& rPolyPoly, const Hatch& rHatch )
{

    Hatch aHatch( rHatch );

    if ( mnDrawMode & ( DRAWMODE_BLACKLINE | DRAWMODE_WHITELINE |
                        DRAWMODE_GRAYLINE | DRAWMODE_GHOSTEDLINE |
                        DRAWMODE_SETTINGSLINE ) )
    {
        Color aColor( rHatch.GetColor() );

        if ( mnDrawMode & DRAWMODE_BLACKLINE )
            aColor = Color( COL_BLACK );
        else if ( mnDrawMode & DRAWMODE_WHITELINE )
            aColor = Color( COL_WHITE );
        else if ( mnDrawMode & DRAWMODE_GRAYLINE )
        {
            const sal_uInt8 cLum = aColor.GetLuminance();
            aColor = Color( cLum, cLum, cLum );
        }
        else if( mnDrawMode & DRAWMODE_SETTINGSLINE )
        {
            aColor = GetSettings().GetStyleSettings().GetFontColor();
        }

        if ( mnDrawMode & DRAWMODE_GHOSTEDLINE )
        {
            aColor = Color( ( aColor.GetRed() >> 1 ) | 0x80,
                            ( aColor.GetGreen() >> 1 ) | 0x80,
                            ( aColor.GetBlue() >> 1 ) | 0x80);
        }

        aHatch.SetColor( aColor );
    }

    if( mpMetaFile )
        mpMetaFile->AddAction( new MetaHatchAction( rPolyPoly, aHatch ) );

    if( !IsDeviceOutputNecessary() || ImplIsRecordLayout() )
        return;

    if( !mpGraphics && !ImplGetGraphics() )
        return;

    if( mbInitClipRegion )
        ImplInitClipRegion();

    if( mbOutputClipped )
        return;

    if( rPolyPoly.Count() )
    {
        PolyPolygon     aPolyPoly( LogicToPixel( rPolyPoly ) );
        GDIMetaFile*    pOldMetaFile = mpMetaFile;
        bool            bOldMap = mbMap;

        aPolyPoly.Optimize( POLY_OPTIMIZE_NO_SAME );
        aHatch.SetDistance( ImplLogicWidthToDevicePixel( aHatch.GetDistance() ) );

        mpMetaFile = NULL;
        EnableMapMode( false );
        Push( PUSH_LINECOLOR );
        SetLineColor( aHatch.GetColor() );
        ImplInitLineColor();
        ImplDrawHatch( aPolyPoly, aHatch, false );
        Pop();
        EnableMapMode( bOldMap );
        mpMetaFile = pOldMetaFile;
    }

    if( mpAlphaVDev )
        mpAlphaVDev->DrawHatch( rPolyPoly, rHatch );
}

void OutputDevice::AddHatchActions( const PolyPolygon& rPolyPoly, const Hatch& rHatch,
                                    GDIMetaFile& rMtf )
{

    PolyPolygon aPolyPoly( rPolyPoly );
    aPolyPoly.Optimize( POLY_OPTIMIZE_NO_SAME | POLY_OPTIMIZE_CLOSE );

    if( aPolyPoly.Count() )
    {
        GDIMetaFile* pOldMtf = mpMetaFile;

        mpMetaFile = &rMtf;
        mpMetaFile->AddAction( new MetaPushAction( PUSH_ALL ) );
        mpMetaFile->AddAction( new MetaLineColorAction( rHatch.GetColor(), true ) );
        ImplDrawHatch( aPolyPoly, rHatch, true );
        mpMetaFile->AddAction( new MetaPopAction() );
        mpMetaFile = pOldMtf;
    }
}

void OutputDevice::ImplDrawHatch( const PolyPolygon& rPolyPoly, const Hatch& rHatch, bool bMtf )
{
    if(rPolyPoly.Count())
    {
        // #i115630# ImplDrawHatch does not work with beziers included in the polypolygon, take care of that
        bool bIsCurve(false);

        for(sal_uInt16 a(0); !bIsCurve && a < rPolyPoly.Count(); a++)
        {
            if(rPolyPoly[a].HasFlags())
            {
                bIsCurve = true;
            }
        }

        if(bIsCurve)
        {
            OSL_ENSURE(false, "ImplDrawHatch does *not* support curves, falling back to AdaptiveSubdivide()...");
            PolyPolygon aPolyPoly;

            rPolyPoly.AdaptiveSubdivide(aPolyPoly);
            ImplDrawHatch(aPolyPoly, rHatch, bMtf);
        }
        else
        {
            Rectangle   aRect( rPolyPoly.GetBoundRect() );
            const long  nLogPixelWidth = ImplDevicePixelToLogicWidth( 1 );
            const long  nWidth = ImplDevicePixelToLogicWidth( std::max( ImplLogicWidthToDevicePixel( rHatch.GetDistance() ), 3L ) );
            boost::scoped_array<Point> pPtBuffer(new Point[ HATCH_MAXPOINTS ]);
            Point       aPt1, aPt2, aEndPt1;
            Size        aInc;

            // Single hatch
            aRect.Left() -= nLogPixelWidth; aRect.Top() -= nLogPixelWidth; aRect.Right() += nLogPixelWidth; aRect.Bottom() += nLogPixelWidth;
            ImplCalcHatchValues( aRect, nWidth, rHatch.GetAngle(), aPt1, aPt2, aInc, aEndPt1 );
            do
            {
                ImplDrawHatchLine( Line( aPt1, aPt2 ), rPolyPoly, pPtBuffer.get(), bMtf );
                aPt1.X() += aInc.Width(); aPt1.Y() += aInc.Height();
                aPt2.X() += aInc.Width(); aPt2.Y() += aInc.Height();
            }
            while( ( aPt1.X() <= aEndPt1.X() ) && ( aPt1.Y() <= aEndPt1.Y() ) );

            if( ( rHatch.GetStyle() == HATCH_DOUBLE ) || ( rHatch.GetStyle() == HATCH_TRIPLE ) )
            {
                // Double hatch
                ImplCalcHatchValues( aRect, nWidth, rHatch.GetAngle() + 900, aPt1, aPt2, aInc, aEndPt1 );
                do
                {
                    ImplDrawHatchLine( Line( aPt1, aPt2 ), rPolyPoly, pPtBuffer.get(), bMtf );
                    aPt1.X() += aInc.Width(); aPt1.Y() += aInc.Height();
                    aPt2.X() += aInc.Width(); aPt2.Y() += aInc.Height();
                }
                while( ( aPt1.X() <= aEndPt1.X() ) && ( aPt1.Y() <= aEndPt1.Y() ) );

                if( rHatch.GetStyle() == HATCH_TRIPLE )
                {
                    // Triple hatch
                    ImplCalcHatchValues( aRect, nWidth, rHatch.GetAngle() + 450, aPt1, aPt2, aInc, aEndPt1 );
                    do
                    {
                        ImplDrawHatchLine( Line( aPt1, aPt2 ), rPolyPoly, pPtBuffer.get(), bMtf );
                        aPt1.X() += aInc.Width(); aPt1.Y() += aInc.Height();
                        aPt2.X() += aInc.Width(); aPt2.Y() += aInc.Height();
                    }
                    while( ( aPt1.X() <= aEndPt1.X() ) && ( aPt1.Y() <= aEndPt1.Y() ) );
                }
            }
        }
    }
}

void OutputDevice::ImplCalcHatchValues( const Rectangle& rRect, long nDist, sal_uInt16 nAngle10,
                                        Point& rPt1, Point& rPt2, Size& rInc, Point& rEndPt1 )
{
    Point   aRef;
    long    nAngle = nAngle10 % 1800;
    long    nOffset = 0;

    if( nAngle > 900 )
        nAngle -= 1800;

    aRef = ( !IsRefPoint() ? rRect.TopLeft() : GetRefPoint() );

    if( 0 == nAngle )
    {
        rInc = Size( 0, nDist );
        rPt1 = rRect.TopLeft();
        rPt2 = rRect.TopRight();
        rEndPt1 = rRect.BottomLeft();

        if( aRef.Y() <= rRect.Top() )
            nOffset = ( ( rRect.Top() - aRef.Y() ) % nDist );
        else
            nOffset = ( nDist - ( ( aRef.Y() - rRect.Top() ) % nDist ) );

        rPt1.Y() -= nOffset;
        rPt2.Y() -= nOffset;
    }
    else if( 900 == nAngle )
    {
        rInc = Size( nDist, 0 );
        rPt1 = rRect.TopLeft();
        rPt2 = rRect.BottomLeft();
        rEndPt1 = rRect.TopRight();

        if( aRef.X() <= rRect.Left() )
            nOffset = ( rRect.Left() - aRef.X() ) % nDist;
        else
            nOffset = nDist - ( ( aRef.X() - rRect.Left() ) % nDist );

        rPt1.X() -= nOffset;
        rPt2.X() -= nOffset;
    }
    else if( nAngle >= -450 && nAngle <= 450 )
    {
        const double    fAngle = F_PI1800 * labs( nAngle );
        const double    fTan = tan( fAngle );
        const long      nYOff = FRound( ( rRect.Right() - rRect.Left() ) * fTan );
        long            nPY;

        rInc = Size( 0, nDist = FRound( nDist / cos( fAngle ) ) );

        if( nAngle > 0 )
        {
            rPt1 = rRect.TopLeft();
            rPt2 = Point( rRect.Right(), rRect.Top() - nYOff );
            rEndPt1 = Point( rRect.Left(), rRect.Bottom() + nYOff );
            nPY = FRound( aRef.Y() - ( ( rPt1.X() - aRef.X() ) * fTan ) );
        }
        else
        {
            rPt1 = rRect.TopRight();
            rPt2 = Point( rRect.Left(), rRect.Top() - nYOff );
            rEndPt1 = Point( rRect.Right(), rRect.Bottom() + nYOff );
            nPY = FRound( aRef.Y() + ( ( rPt1.X() - aRef.X() ) * fTan ) );
        }

        if( nPY <= rPt1.Y() )
            nOffset = ( rPt1.Y() - nPY ) % nDist;
        else
            nOffset = nDist - ( ( nPY - rPt1.Y() ) % nDist );

        rPt1.Y() -= nOffset;
        rPt2.Y() -= nOffset;
    }
    else
    {
        const double fAngle = F_PI1800 * labs( nAngle );
        const double fTan = tan( fAngle );
        const long   nXOff = FRound( ( rRect.Bottom() - rRect.Top() ) / fTan );
        long         nPX;

        rInc = Size( nDist = FRound( nDist / sin( fAngle ) ), 0 );

        if( nAngle > 0 )
        {
            rPt1 = rRect.TopLeft();
            rPt2 = Point( rRect.Left() - nXOff, rRect.Bottom() );
            rEndPt1 = Point( rRect.Right() + nXOff, rRect.Top() );
            nPX = FRound( aRef.X() - ( ( rPt1.Y() - aRef.Y() ) / fTan ) );
        }
        else
        {
            rPt1 = rRect.BottomLeft();
            rPt2 = Point( rRect.Left() - nXOff, rRect.Top() );
            rEndPt1 = Point( rRect.Right() + nXOff, rRect.Bottom() );
            nPX = FRound( aRef.X() + ( ( rPt1.Y() - aRef.Y() ) / fTan ) );
        }

        if( nPX <= rPt1.X() )
            nOffset = ( rPt1.X() - nPX ) % nDist;
        else
            nOffset = nDist - ( ( nPX - rPt1.X() ) % nDist );

        rPt1.X() -= nOffset;
        rPt2.X() -= nOffset;
    }
}

void OutputDevice::ImplDrawHatchLine( const Line& rLine, const PolyPolygon& rPolyPoly,
                                      Point* pPtBuffer, bool bMtf )
{
    double  fX, fY;
    long    nAdd, nPCounter = 0;

    for( long nPoly = 0, nPolyCount = rPolyPoly.Count(); nPoly < nPolyCount; nPoly++ )
    {
        const Polygon& rPoly = rPolyPoly[ (sal_uInt16) nPoly ];

        if( rPoly.GetSize() > 1 )
        {
            Line    aCurSegment( rPoly[ 0 ], Point() );

            for( long i = 1, nCount = rPoly.GetSize(); i <= nCount; i++ )
            {
                aCurSegment.SetEnd( rPoly[ (sal_uInt16)( i % nCount ) ] );
                nAdd = 0;

                if( rLine.Intersection( aCurSegment, fX, fY ) )
                {
                    if( ( fabs( fX - aCurSegment.GetStart().X() ) <= 0.0000001 ) &&
                        ( fabs( fY - aCurSegment.GetStart().Y() ) <= 0.0000001 ) )
                    {
                        const Line      aPrevSegment( rPoly[ (sal_uInt16)( ( i > 1 ) ? ( i - 2 ) : ( nCount - 1 ) ) ], aCurSegment.GetStart() );
                        const double    fPrevDistance = rLine.GetDistance( aPrevSegment.GetStart() );
                        const double    fCurDistance = rLine.GetDistance( aCurSegment.GetEnd() );

                        if( ( fPrevDistance <= 0.0 && fCurDistance > 0.0 ) ||
                            ( fPrevDistance > 0.0 && fCurDistance < 0.0 ) )
                        {
                            nAdd = 1;
                        }
                    }
                    else if( ( fabs( fX - aCurSegment.GetEnd().X() ) <= 0.0000001 ) &&
                             ( fabs( fY - aCurSegment.GetEnd().Y() ) <= 0.0000001 ) )
                    {
                        const Line aNextSegment( aCurSegment.GetEnd(), rPoly[ (sal_uInt16)( ( i + 1 ) % nCount ) ] );

                        if( ( fabs( rLine.GetDistance( aNextSegment.GetEnd() ) ) <= 0.0000001 ) &&
                            ( rLine.GetDistance( aCurSegment.GetStart() ) > 0.0 ) )
                        {
                            nAdd = 1;
                        }
                    }
                    else
                        nAdd = 1;

                    if( nAdd )
                        pPtBuffer[ nPCounter++ ] = Point( FRound( fX ), FRound( fY ) );
                }

                aCurSegment.SetStart( aCurSegment.GetEnd() );
            }
        }
    }

    if( nPCounter > 1 )
    {
        qsort( pPtBuffer, nPCounter, sizeof( Point ), ImplHatchCmpFnc );

        if( nPCounter & 1 )
            nPCounter--;

        if( bMtf )
        {
            for( long i = 0; i < nPCounter; i += 2 )
                mpMetaFile->AddAction( new MetaLineAction( pPtBuffer[ i ], pPtBuffer[ i + 1 ] ) );
        }
        else
        {
            for( long i = 0; i < nPCounter; i += 2 )
            {
                if( mpPDFWriter )
                {
                    mpPDFWriter->drawLine( pPtBuffer[ i ], pPtBuffer[ i+1 ] );
                }
                else
                {
                    const Point aPt1( ImplLogicToDevicePixel( pPtBuffer[ i ] ) );
                    const Point aPt2( ImplLogicToDevicePixel( pPtBuffer[ i + 1 ] ) );
                    mpGraphics->DrawLine( aPt1.X(), aPt1.Y(), aPt2.X(), aPt2.Y(), this );
                }
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
