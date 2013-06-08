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

#include "pdfwriter_impl.hxx"

#include "window.h"
#include "salframe.hxx"
#include "salgdi.hxx"
#include "svdata.hxx"
#include "outdata.hxx"

#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

#define HATCH_MAXPOINTS             1024
#define GRADIENT_DEFAULT_STEPCOUNT  0

extern "C" int SAL_CALL ImplHatchCmpFnc( const void* p1, const void* p2 )
{
    const long nX1 = ( (Point*) p1 )->X();
    const long nX2 = ( (Point*) p2 )->X();
    const long nY1 = ( (Point*) p1 )->Y();
    const long nY2 = ( (Point*) p2 )->Y();

    return ( nX1 > nX2 ? 1 : nX1 == nX2 ? nY1 > nY2 ? 1: nY1 == nY2 ? 0 : -1 : -1 );
}

DBG_NAMEEX( OutputDevice )
DBG_NAMEEX( Gradient )

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
        sal_uInt32*         pPointAry = new sal_uInt32[nCount];
        PCONSTSALPOINT*     pPointAryAry = new PCONSTSALPOINT[nCount];
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
            mpGraphics->DrawPolygon( *pPointAry, *pPointAryAry, this );
        else
            mpGraphics->DrawPolyPolygon( nCount, pPointAry, pPointAryAry, this );

        delete[] pPointAry;
        delete[] pPointAryAry;
    }

    if( pClipPolyPoly )
        delete pPolyPoly;
}

inline sal_uInt8 ImplGetGradientColorValue( long nValue )
{
    if ( nValue < 0 )
        return 0;
    else if ( nValue > 0xFF )
        return 0xFF;
    else
        return (sal_uInt8)nValue;
}

void OutputDevice::ImplDrawLinearGradient( const Rectangle& rRect,
                                           const Gradient& rGradient,
                                           sal_Bool bMtf, const PolyPolygon* pClipPolyPoly )
{
    // get BoundRect of rotated rectangle
    Rectangle aRect;
    Point     aCenter;
    sal_uInt16    nAngle = rGradient.GetAngle() % 3600;

    rGradient.GetBoundRect( rRect, aRect, aCenter );

    bool bLinear = (rGradient.GetStyle() == GradientStyle_LINEAR);
    double fBorder = rGradient.GetBorder() * aRect.GetHeight() / 100.0;
    if ( !bLinear )
    {
        fBorder /= 2.0;
    }
    Rectangle aMirrorRect = aRect; // used in style axial
    aMirrorRect.Top() = ( aRect.Top() + aRect.Bottom() ) / 2;
    if ( !bLinear )
    {
        aRect.Bottom() = aMirrorRect.Top();
    }

    // Intensitaeten von Start- und Endfarbe ggf. aendern
    long    nFactor;
    Color   aStartCol   = rGradient.GetStartColor();
    Color   aEndCol     = rGradient.GetEndColor();
    long    nStartRed   = aStartCol.GetRed();
    long    nStartGreen = aStartCol.GetGreen();
    long    nStartBlue  = aStartCol.GetBlue();
    long    nEndRed     = aEndCol.GetRed();
    long    nEndGreen   = aEndCol.GetGreen();
    long    nEndBlue    = aEndCol.GetBlue();
            nFactor     = rGradient.GetStartIntensity();
            nStartRed   = (nStartRed   * nFactor) / 100;
            nStartGreen = (nStartGreen * nFactor) / 100;
            nStartBlue  = (nStartBlue  * nFactor) / 100;
            nFactor     = rGradient.GetEndIntensity();
            nEndRed     = (nEndRed   * nFactor) / 100;
            nEndGreen   = (nEndGreen * nFactor) / 100;
            nEndBlue    = (nEndBlue  * nFactor) / 100;

    // gradient style axial has exchanged start and end colors
    if ( !bLinear)
    {
        long nTempColor = nStartRed;
        nStartRed = nEndRed;
        nEndRed = nTempColor;
        nTempColor = nStartGreen;
        nStartGreen = nEndGreen;
        nEndGreen = nTempColor;
        nTempColor = nStartBlue;
        nStartBlue = nEndBlue;
        nEndBlue = nTempColor;
    }

    sal_uInt8   nRed;
    sal_uInt8   nGreen;
    sal_uInt8   nBlue;

    // Create border
    Rectangle aBorderRect = aRect;
    Polygon     aPoly( 4 );
    if (fBorder > 0.0)
    {
        nRed        = (sal_uInt8)nStartRed;
        nGreen      = (sal_uInt8)nStartGreen;
        nBlue       = (sal_uInt8)nStartBlue;
        if ( bMtf )
            mpMetaFile->AddAction( new MetaFillColorAction( Color( nRed, nGreen, nBlue ), sal_True ) );
        else
            mpGraphics->SetFillColor( MAKE_SALCOLOR( nRed, nGreen, nBlue ) );

        aBorderRect.Bottom() = (long)( aBorderRect.Top() + fBorder );
        aRect.Top() = aBorderRect.Bottom();
        aPoly[0] = aBorderRect.TopLeft();
        aPoly[1] = aBorderRect.TopRight();
        aPoly[2] = aBorderRect.BottomRight();
        aPoly[3] = aBorderRect.BottomLeft();
        aPoly.Rotate( aCenter, nAngle );
        if ( bMtf )
            mpMetaFile->AddAction( new MetaPolygonAction( aPoly ) );
        else
            ImplDrawPolygon( aPoly, pClipPolyPoly );
        if ( !bLinear)
        {
            aBorderRect = aMirrorRect;
            aBorderRect.Top() = (long) ( aBorderRect.Bottom() - fBorder );
            aMirrorRect.Bottom() = aBorderRect.Top();
            aPoly[0] = aBorderRect.TopLeft();
            aPoly[1] = aBorderRect.TopRight();
            aPoly[2] = aBorderRect.BottomRight();
            aPoly[3] = aBorderRect.BottomLeft();
            aPoly.Rotate( aCenter, nAngle );
            if ( bMtf )
                mpMetaFile->AddAction( new MetaPolygonAction( aPoly ) );
            else
                ImplDrawPolygon( aPoly, pClipPolyPoly );
        }
    }

    // calculate step count
    long    nStepCount  = rGradient.GetSteps();
    // generate nStepCount, if not passed
    long nMinRect = aRect.GetHeight();
    if ( !nStepCount )
    {
        long nInc = 1;
        if ( meOutDevType != OUTDEV_PRINTER && !bMtf )
        {
            nInc = (nMinRect < 50) ? 2 : 4;
        }
        else
        {
            // Use display-equivalent step size calculation
            nInc = (nMinRect < 800) ? 10 : 20;
        }
        nStepCount = nMinRect / nInc;
    }

    // minimal three steps and maximal as max color steps
    long   nAbsRedSteps   = std::abs( nEndRed   - nStartRed );
    long   nAbsGreenSteps = std::abs( nEndGreen - nStartGreen );
    long   nAbsBlueSteps  = std::abs( nEndBlue  - nStartBlue );
    long   nMaxColorSteps = std::max( nAbsRedSteps , nAbsGreenSteps );
    nMaxColorSteps = std::max( nMaxColorSteps, nAbsBlueSteps );
    long nSteps = std::min( nStepCount, nMaxColorSteps );
    if ( nSteps < 3)
    {
        nSteps = 3;
    }

    double fScanInc = ((double)aRect.GetHeight()) / (double) nSteps;
    double fGradientLine = (double)aRect.Top();
    double fMirrorGradientLine = (double) aMirrorRect.Bottom();

    double fAlpha = 0.0;
    const double fStepsMinus1 = ((double)nSteps) - 1.0;
    double fTempColor;
    if ( !bLinear)
    {
        nSteps -= 1; // draw middle polygons as one polygon after loop to avoid gap
    }
    for ( long i = 0; i < nSteps; i++ )
    {
        // linear interpolation of color
        fAlpha = ((double)i) / fStepsMinus1;
        fTempColor = ((double)nStartRed) * (1.0-fAlpha) + ((double)nEndRed) * fAlpha;
        nRed = ImplGetGradientColorValue((long)fTempColor);
        fTempColor = ((double)nStartGreen) * (1.0-fAlpha) + ((double)nEndGreen) * fAlpha;
        nGreen = ImplGetGradientColorValue((long)fTempColor);
        fTempColor = ((double)nStartBlue) * (1.0-fAlpha) + ((double)nEndBlue) * fAlpha;
        nBlue = ImplGetGradientColorValue((long)fTempColor);
        if ( bMtf )
            mpMetaFile->AddAction( new MetaFillColorAction( Color( nRed, nGreen, nBlue ), sal_True ) );
        else
            mpGraphics->SetFillColor( MAKE_SALCOLOR( nRed, nGreen, nBlue ) );

        // Polygon for this color step
        aRect.Top() = (long)( fGradientLine + ((double) i) * fScanInc );
        aRect.Bottom() = (long)( fGradientLine + ( ((double) i) + 1.0 ) * fScanInc + fScanInc*.1 );
        aPoly[0] = aRect.TopLeft();
        aPoly[1] = aRect.TopRight();
        aPoly[2] = aRect.BottomRight();
        aPoly[3] = aRect.BottomLeft();
        aPoly.Rotate( aCenter, nAngle );
        if ( bMtf )
            mpMetaFile->AddAction( new MetaPolygonAction( aPoly ) );
        else
            ImplDrawPolygon( aPoly, pClipPolyPoly );
        if ( !bLinear )
        {
            aMirrorRect.Bottom() = (long)( fMirrorGradientLine - ((double) i) * fScanInc );
            aMirrorRect.Top() = (long)( fMirrorGradientLine - (((double) i) + 1.0)* fScanInc );
            aPoly[0] = aMirrorRect.TopLeft();
            aPoly[1] = aMirrorRect.TopRight();
            aPoly[2] = aMirrorRect.BottomRight();
            aPoly[3] = aMirrorRect.BottomLeft();
            aPoly.Rotate( aCenter, nAngle );
            if ( bMtf )
                mpMetaFile->AddAction( new MetaPolygonAction( aPoly ) );
            else
                ImplDrawPolygon( aPoly, pClipPolyPoly );
        }
    }
    if ( !bLinear)
    {
        // draw middle polygon with end color
        nRed = ImplGetGradientColorValue(nEndRed);
        nGreen = ImplGetGradientColorValue(nEndGreen);
        nBlue = ImplGetGradientColorValue(nEndBlue);
        if ( bMtf )
            mpMetaFile->AddAction( new MetaFillColorAction( Color( nRed, nGreen, nBlue ), sal_True ) );
        else
            mpGraphics->SetFillColor( MAKE_SALCOLOR( nRed, nGreen, nBlue ) );

        aRect.Top() = (long)( fGradientLine + ((double)nSteps) * fScanInc );
        aRect.Bottom() = (long)( fMirrorGradientLine - ((double) nSteps) * fScanInc );
        aPoly[0] = aRect.TopLeft();
        aPoly[1] = aRect.TopRight();
        aPoly[2] = aRect.BottomRight();
        aPoly[3] = aRect.BottomLeft();
        aPoly.Rotate( aCenter, nAngle );
        if ( bMtf )
            mpMetaFile->AddAction( new MetaPolygonAction( aPoly ) );
        else
            ImplDrawPolygon( aPoly, pClipPolyPoly );
    }
}

void OutputDevice::ImplDrawComplexGradient( const Rectangle& rRect,
                                            const Gradient& rGradient,
                                            sal_Bool bMtf, const PolyPolygon* pClipPolyPoly )
{
    // Feststellen ob Ausgabe ueber Polygon oder PolyPolygon
    // Bei Rasteroperationen ungleich Overpaint immer PolyPolygone,
    // da es zu falschen Ergebnissen kommt, wenn man mehrfach uebereinander
    // ausgibt
    // Bei Druckern auch immer PolyPolygone, da nicht alle Drucker
    // das Uebereinanderdrucken von Polygonen koennen
    // Virtuelle Device werden auch ausgeklammert, da einige Treiber
    // ansonsten zu langsam sind
    PolyPolygon*    pPolyPoly;
    Rectangle       aRect;
    Point           aCenter;
    Color           aStartCol( rGradient.GetStartColor() );
    Color           aEndCol( rGradient.GetEndColor() );
    long            nStartRed = ( (long) aStartCol.GetRed() * rGradient.GetStartIntensity() ) / 100;
    long            nStartGreen = ( (long) aStartCol.GetGreen() * rGradient.GetStartIntensity() ) / 100;
    long            nStartBlue = ( (long) aStartCol.GetBlue() * rGradient.GetStartIntensity() ) / 100;
    long            nEndRed = ( (long) aEndCol.GetRed() * rGradient.GetEndIntensity() ) / 100;
    long            nEndGreen = ( (long) aEndCol.GetGreen() * rGradient.GetEndIntensity() ) / 100;
    long            nEndBlue = ( (long) aEndCol.GetBlue() * rGradient.GetEndIntensity() ) / 100;
    long            nRedSteps = nEndRed - nStartRed;
    long            nGreenSteps = nEndGreen - nStartGreen;
    long            nBlueSteps = nEndBlue   - nStartBlue;
    long            nStepCount = rGradient.GetSteps();
    sal_uInt16      nAngle = rGradient.GetAngle() % 3600;

    rGradient.GetBoundRect( rRect, aRect, aCenter );

    if( (meRasterOp != ROP_OVERPAINT) || (meOutDevType != OUTDEV_WINDOW) || bMtf )
        pPolyPoly = new PolyPolygon( 2 );
    else
        pPolyPoly = NULL;

    long nMinRect = std::min( aRect.GetWidth(), aRect.GetHeight() );

    // Anzahl der Schritte berechnen, falls nichts uebergeben wurde
    if( !nStepCount )
    {
        long nInc;

        if ( meOutDevType != OUTDEV_PRINTER && !bMtf )
        {
            nInc = ( nMinRect < 50 ) ? 2 : 4;
        }
        else
        {
            // #105998# Use display-equivalent step size calculation
            nInc = (nMinRect < 800) ? 10 : 20;
        }

        nStepCount = nMinRect / nInc;
    }

    // minimal drei Schritte und maximal die Anzahl der Farbunterschiede
    long nSteps = std::max( nStepCount, 2L );
    long nCalcSteps  = std::abs( nRedSteps );
    long nTempSteps = std::abs( nGreenSteps );
    if ( nTempSteps > nCalcSteps )
        nCalcSteps = nTempSteps;
    nTempSteps = std::abs( nBlueSteps );
    if ( nTempSteps > nCalcSteps )
        nCalcSteps = nTempSteps;
    if ( nCalcSteps < nSteps )
        nSteps = nCalcSteps;
    if ( !nSteps )
        nSteps = 1;

    // Ausgabebegrenzungen und Schrittweite fuer jede Richtung festlegen
    Polygon aPoly;
    double  fScanLeft = aRect.Left();
    double  fScanTop = aRect.Top();
    double  fScanRight = aRect.Right();
    double  fScanBottom = aRect.Bottom();
    double fScanIncX = (double) aRect.GetWidth() / (double) nSteps * 0.5;
    double fScanIncY = (double) aRect.GetHeight() / (double) nSteps * 0.5;

    // all gradients are rendered as nested rectangles which shrink
    // equally in each dimension - except for 'square' gradients
    // which shrink to a central vertex but are not per-se square.
    if( rGradient.GetStyle() != GradientStyle_SQUARE )
    {
        fScanIncY = std::min( fScanIncY, fScanIncX );
        fScanIncX = fScanIncY;
    }
    sal_uInt8   nRed = (sal_uInt8) nStartRed, nGreen = (sal_uInt8) nStartGreen, nBlue = (sal_uInt8) nStartBlue;
    bool    bPaintLastPolygon( false ); // #107349# Paint last polygon only if loop has generated any output

    if( bMtf )
        mpMetaFile->AddAction( new MetaFillColorAction( Color( nRed, nGreen, nBlue ), sal_True ) );
    else
        mpGraphics->SetFillColor( MAKE_SALCOLOR( nRed, nGreen, nBlue ) );

    if( pPolyPoly )
    {
        pPolyPoly->Insert( aPoly = rRect );
        pPolyPoly->Insert( aPoly );
    }
    else
    {
        // extend rect, to avoid missing bounding line
        Rectangle aExtRect( rRect );

        aExtRect.Left() -= 1;
        aExtRect.Top() -= 1;
        aExtRect.Right() += 1;
        aExtRect.Bottom() += 1;

        ImplDrawPolygon( aPoly = aExtRect, pClipPolyPoly );
    }

    // Schleife, um nacheinander die Polygone/PolyPolygone auszugeben
    for( long i = 1; i < nSteps; i++ )
    {
        // neues Polygon berechnen
        aRect.Left() = (long)( fScanLeft += fScanIncX );
        aRect.Top() = (long)( fScanTop += fScanIncY );
        aRect.Right() = (long)( fScanRight -= fScanIncX );
        aRect.Bottom() = (long)( fScanBottom -= fScanIncY );

        if( ( aRect.GetWidth() < 2 ) || ( aRect.GetHeight() < 2 ) )
            break;

        if( rGradient.GetStyle() == GradientStyle_RADIAL || rGradient.GetStyle() == GradientStyle_ELLIPTICAL )
            aPoly = Polygon( aRect.Center(), aRect.GetWidth() >> 1, aRect.GetHeight() >> 1 );
        else
            aPoly = Polygon( aRect );

        aPoly.Rotate( aCenter, nAngle );

        // Farbe entsprechend anpassen
        const long nStepIndex = ( ( pPolyPoly != NULL ) ? i : ( i + 1 ) );
        nRed = ImplGetGradientColorValue( nStartRed + ( ( nRedSteps * nStepIndex ) / nSteps ) );
        nGreen = ImplGetGradientColorValue( nStartGreen + ( ( nGreenSteps * nStepIndex ) / nSteps ) );
        nBlue = ImplGetGradientColorValue( nStartBlue + ( ( nBlueSteps * nStepIndex ) / nSteps ) );

        // entweder langsame PolyPolygon-Ausgaben oder schnelles Polygon-Painting
        if( pPolyPoly )
        {
            bPaintLastPolygon = true; // #107349# Paint last polygon only if loop has generated any output

            pPolyPoly->Replace( pPolyPoly->GetObject( 1 ), 0 );
            pPolyPoly->Replace( aPoly, 1 );

            if( bMtf )
                mpMetaFile->AddAction( new MetaPolyPolygonAction( *pPolyPoly ) );
            else
                ImplDrawPolyPolygon( *pPolyPoly, pClipPolyPoly );

            // #107349# Set fill color _after_ geometry painting:
            // pPolyPoly's geometry is the band from last iteration's
            // aPoly to current iteration's aPoly. The window outdev
            // path (see else below), on the other hand, paints the
            // full aPoly. Thus, here, we're painting the band before
            // the one painted in the window outdev path below. To get
            // matching colors, have to delay color setting here.
            if( bMtf )
                mpMetaFile->AddAction( new MetaFillColorAction( Color( nRed, nGreen, nBlue ), sal_True ) );
            else
                mpGraphics->SetFillColor( MAKE_SALCOLOR( nRed, nGreen, nBlue ) );
        }
        else
        {
            // #107349# Set fill color _before_ geometry painting
            if( bMtf )
                mpMetaFile->AddAction( new MetaFillColorAction( Color( nRed, nGreen, nBlue ), sal_True ) );
            else
                mpGraphics->SetFillColor( MAKE_SALCOLOR( nRed, nGreen, nBlue ) );

            ImplDrawPolygon( aPoly, pClipPolyPoly );
        }
    }

    // Falls PolyPolygon-Ausgabe, muessen wir noch ein letztes inneres Polygon zeichnen
    if( pPolyPoly )
    {
        const Polygon& rPoly = pPolyPoly->GetObject( 1 );

        if( !rPoly.GetBoundRect().IsEmpty() )
        {
            // #107349# Paint last polygon with end color only if loop
            // has generated output. Otherwise, the current
            // (i.e. start) color is taken, to generate _any_ output.
            if( bPaintLastPolygon )
            {
                nRed = ImplGetGradientColorValue( nEndRed );
                nGreen = ImplGetGradientColorValue( nEndGreen );
                nBlue = ImplGetGradientColorValue( nEndBlue );
            }

            if( bMtf )
            {
                mpMetaFile->AddAction( new MetaFillColorAction( Color( nRed, nGreen, nBlue ), sal_True ) );
                mpMetaFile->AddAction( new MetaPolygonAction( rPoly ) );
            }
            else
            {
                mpGraphics->SetFillColor( MAKE_SALCOLOR( nRed, nGreen, nBlue ) );
                   ImplDrawPolygon( rPoly, pClipPolyPoly );
            }
        }

        delete pPolyPoly;
    }
}

void OutputDevice::DrawGradient( const Rectangle& rRect,
                                 const Gradient& rGradient )
{
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );
    DBG_CHKOBJ( &rGradient, Gradient, NULL );

    if ( mnDrawMode & DRAWMODE_NOGRADIENT )
        return;
    else if ( mnDrawMode & ( DRAWMODE_BLACKGRADIENT | DRAWMODE_WHITEGRADIENT | DRAWMODE_SETTINGSGRADIENT) )
    {
        Color aColor;

        if ( mnDrawMode & DRAWMODE_BLACKGRADIENT )
            aColor = Color( COL_BLACK );
        else if ( mnDrawMode & DRAWMODE_WHITEGRADIENT )
            aColor = Color( COL_WHITE );
        else if ( mnDrawMode & DRAWMODE_SETTINGSGRADIENT )
            aColor = GetSettings().GetStyleSettings().GetWindowColor();

        if ( mnDrawMode & DRAWMODE_GHOSTEDGRADIENT )
        {
            aColor = Color( ( aColor.GetRed() >> 1 ) | 0x80,
                            ( aColor.GetGreen() >> 1 ) | 0x80,
                            ( aColor.GetBlue() >> 1 ) | 0x80 );
        }

        Push( PUSH_LINECOLOR | PUSH_FILLCOLOR );
        SetLineColor( aColor );
        SetFillColor( aColor );
        DrawRect( rRect );
        Pop();
        return;
    }

    Gradient aGradient( rGradient );

    if ( mnDrawMode & ( DRAWMODE_GRAYGRADIENT | DRAWMODE_GHOSTEDGRADIENT ) )
    {
        Color aStartCol( aGradient.GetStartColor() );
        Color aEndCol( aGradient.GetEndColor() );

        if ( mnDrawMode & DRAWMODE_GRAYGRADIENT )
        {
            sal_uInt8 cStartLum = aStartCol.GetLuminance(), cEndLum = aEndCol.GetLuminance();
            aStartCol = Color( cStartLum, cStartLum, cStartLum );
            aEndCol = Color( cEndLum, cEndLum, cEndLum );
        }

        if ( mnDrawMode & DRAWMODE_GHOSTEDGRADIENT )
        {
            aStartCol = Color( ( aStartCol.GetRed() >> 1 ) | 0x80,
                               ( aStartCol.GetGreen() >> 1 ) | 0x80,
                               ( aStartCol.GetBlue() >> 1 ) | 0x80 );

            aEndCol = Color( ( aEndCol.GetRed() >> 1 ) | 0x80,
                             ( aEndCol.GetGreen() >> 1 ) | 0x80,
                             ( aEndCol.GetBlue() >> 1 ) | 0x80 );
        }

        aGradient.SetStartColor( aStartCol );
        aGradient.SetEndColor( aEndCol );
    }

    if( mpMetaFile )
        mpMetaFile->AddAction( new MetaGradientAction( rRect, aGradient ) );

    if( !IsDeviceOutputNecessary() || ImplIsRecordLayout() )
        return;

    // Rechteck in Pixel umrechnen
    Rectangle aRect( ImplLogicToDevicePixel( rRect ) );
    aRect.Justify();

    // Wenn Rechteck leer ist, brauchen wir nichts machen
    if ( !aRect.IsEmpty() )
    {
        // Clip Region sichern
        Push( PUSH_CLIPREGION );
        IntersectClipRegion( rRect );

        // because we draw with no border line, we have to expand gradient
        // rect to avoid missing lines on the right and bottom edge
        aRect.Left()--;
        aRect.Top()--;
        aRect.Right()++;
        aRect.Bottom()++;

        // we need a graphics
        if ( !mpGraphics )
        {
            if ( !ImplGetGraphics() )
                return;
        }

        if ( mbInitClipRegion )
            ImplInitClipRegion();

        if ( !mbOutputClipped )
        {
            // Gradienten werden ohne Umrandung gezeichnet
            if ( mbLineColor || mbInitLineColor )
            {
                mpGraphics->SetLineColor();
                mbInitLineColor = sal_True;
            }

            mbInitFillColor = sal_True;

            // calculate step count if necessary
            if ( !aGradient.GetSteps() )
                aGradient.SetSteps( GRADIENT_DEFAULT_STEPCOUNT );

            if( aGradient.GetStyle() == GradientStyle_LINEAR || aGradient.GetStyle() == GradientStyle_AXIAL )
                ImplDrawLinearGradient( aRect, aGradient, sal_False, NULL );
            else
                ImplDrawComplexGradient( aRect, aGradient, sal_False, NULL );
        }

        Pop();
    }

    if( mpAlphaVDev )
    {
        // #i32109#: Make gradient area opaque
        mpAlphaVDev->ImplFillOpaqueRectangle( rRect );
    }
}

void OutputDevice::DrawGradient( const PolyPolygon& rPolyPoly,
                                 const Gradient& rGradient )
{
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );
    DBG_CHKOBJ( &rGradient, Gradient, NULL );

    if( mbInitClipRegion )
        ImplInitClipRegion();

    if( mbOutputClipped )
        return;

    if( !mpGraphics )
        if( !ImplGetGraphics() )
            return;

    if( rPolyPoly.Count() && rPolyPoly[ 0 ].GetSize() && !( mnDrawMode & DRAWMODE_NOGRADIENT ) )
    {
        if ( mnDrawMode & ( DRAWMODE_BLACKGRADIENT | DRAWMODE_WHITEGRADIENT | DRAWMODE_SETTINGSGRADIENT) )
        {
            Color aColor;

            if ( mnDrawMode & DRAWMODE_BLACKGRADIENT )
                aColor = Color( COL_BLACK );
            else if ( mnDrawMode & DRAWMODE_WHITEGRADIENT )
                aColor = Color( COL_WHITE );
            else if ( mnDrawMode & DRAWMODE_SETTINGSGRADIENT )
                aColor = GetSettings().GetStyleSettings().GetWindowColor();

            if ( mnDrawMode & DRAWMODE_GHOSTEDGRADIENT )
            {
                aColor = Color( ( aColor.GetRed() >> 1 ) | 0x80,
                                ( aColor.GetGreen() >> 1 ) | 0x80,
                                ( aColor.GetBlue() >> 1 ) | 0x80 );
            }

            Push( PUSH_LINECOLOR | PUSH_FILLCOLOR );
            SetLineColor( aColor );
            SetFillColor( aColor );
            DrawPolyPolygon( rPolyPoly );
            Pop();
            return;
        }

        if( mpMetaFile )
        {
            const Rectangle aRect( rPolyPoly.GetBoundRect() );

            mpMetaFile->AddAction( new MetaCommentAction( "XGRAD_SEQ_BEGIN" ) );
            mpMetaFile->AddAction( new MetaGradientExAction( rPolyPoly, rGradient ) );

            if( OUTDEV_PRINTER == meOutDevType )
            {
                Push( PUSH_CLIPREGION );
                IntersectClipRegion(Region(rPolyPoly));
                DrawGradient( aRect, rGradient );
                Pop();
            }
            else
            {
                const sal_Bool  bOldOutput = IsOutputEnabled();

                EnableOutput( sal_False );
                Push( PUSH_RASTEROP );
                SetRasterOp( ROP_XOR );
                DrawGradient( aRect, rGradient );
                SetFillColor( COL_BLACK );
                SetRasterOp( ROP_0 );
                DrawPolyPolygon( rPolyPoly );
                SetRasterOp( ROP_XOR );
                DrawGradient( aRect, rGradient );
                Pop();
                EnableOutput( bOldOutput );
            }

            mpMetaFile->AddAction( new MetaCommentAction( "XGRAD_SEQ_END" ) );
        }

        if( !IsDeviceOutputNecessary() || ImplIsRecordLayout() )
            return;

        Gradient aGradient( rGradient );

        if ( mnDrawMode & ( DRAWMODE_GRAYGRADIENT | DRAWMODE_GHOSTEDGRADIENT ) )
        {
            Color aStartCol( aGradient.GetStartColor() );
            Color aEndCol( aGradient.GetEndColor() );

            if ( mnDrawMode & DRAWMODE_GRAYGRADIENT )
            {
                sal_uInt8 cStartLum = aStartCol.GetLuminance(), cEndLum = aEndCol.GetLuminance();
                aStartCol = Color( cStartLum, cStartLum, cStartLum );
                aEndCol = Color( cEndLum, cEndLum, cEndLum );
            }

            if ( mnDrawMode & DRAWMODE_GHOSTEDGRADIENT )
            {
                aStartCol = Color( ( aStartCol.GetRed() >> 1 ) | 0x80,
                                   ( aStartCol.GetGreen() >> 1 ) | 0x80,
                                   ( aStartCol.GetBlue() >> 1 ) | 0x80 );

                aEndCol = Color( ( aEndCol.GetRed() >> 1 ) | 0x80,
                                 ( aEndCol.GetGreen() >> 1 ) | 0x80,
                                 ( aEndCol.GetBlue() >> 1 ) | 0x80 );
            }

            aGradient.SetStartColor( aStartCol );
            aGradient.SetEndColor( aEndCol );
        }

        if( OUTDEV_PRINTER == meOutDevType || ImplGetSVData()->maGDIData.mbNoXORClipping )
        {
            const Rectangle aBoundRect( rPolyPoly.GetBoundRect() );

            if( !Rectangle( PixelToLogic( Point() ), GetOutputSize() ).IsEmpty() )
            {
                // Rechteck in Pixel umrechnen
                Rectangle aRect( ImplLogicToDevicePixel( aBoundRect ) );
                aRect.Justify();

                // Wenn Rechteck leer ist, brauchen wir nichts machen
                if ( !aRect.IsEmpty() )
                {
                    if( !mpGraphics && !ImplGetGraphics() )
                        return;

                    if( mbInitClipRegion )
                        ImplInitClipRegion();

                    if( !mbOutputClipped )
                    {
                        PolyPolygon aClipPolyPoly( ImplLogicToDevicePixel( rPolyPoly ) );

                        // Gradienten werden ohne Umrandung gezeichnet
                        if( mbLineColor || mbInitLineColor )
                        {
                            mpGraphics->SetLineColor();
                            mbInitLineColor = sal_True;
                        }

                        mbInitFillColor = sal_True;

                        // calculate step count if necessary
                        if ( !aGradient.GetSteps() )
                            aGradient.SetSteps( GRADIENT_DEFAULT_STEPCOUNT );

                        if( aGradient.GetStyle() == GradientStyle_LINEAR || aGradient.GetStyle() == GradientStyle_AXIAL )
                            ImplDrawLinearGradient( aRect, aGradient, sal_False, &aClipPolyPoly );
                        else
                            ImplDrawComplexGradient( aRect, aGradient, sal_False, &aClipPolyPoly );
                    }
                }
            }
        }
        else
        {
            const PolyPolygon   aPolyPoly( LogicToPixel( rPolyPoly ) );
            const Rectangle     aBoundRect( aPolyPoly.GetBoundRect() );
            Point aPoint;
            Rectangle           aDstRect( aPoint, GetOutputSizePixel() );

            aDstRect.Intersection( aBoundRect );

            if( OUTDEV_WINDOW == meOutDevType )
            {
                const Region aPaintRgn( ( (Window*) this )->GetPaintRegion() );

                if( !aPaintRgn.IsNull() )
                    aDstRect.Intersection( LogicToPixel( aPaintRgn ).GetBoundRect() );
            }

            if( !aDstRect.IsEmpty() )
            {
                VirtualDevice*  pVDev;
                const Size      aDstSize( aDstRect.GetSize() );

                if( HasAlpha() )
                {
                    // #110958# Pay attention to alpha VDevs here, otherwise,
                    // background will be wrong: Temp VDev has to have alpha, too.
                    pVDev = new VirtualDevice( *this, 0, GetAlphaBitCount() > 1 ? 0 : 1 );
                }
                else
                {
                    // nothing special here. Plain VDev
                    pVDev = new VirtualDevice();
                }

                if( pVDev->SetOutputSizePixel( aDstSize) )
                {
                    MapMode         aVDevMap;
                    const sal_Bool      bOldMap = mbMap;

                    EnableMapMode( sal_False );

                    pVDev->DrawOutDev( Point(), aDstSize, aDstRect.TopLeft(), aDstSize, *this );
                    pVDev->SetRasterOp( ROP_XOR );
                    aVDevMap.SetOrigin( Point( -aDstRect.Left(), -aDstRect.Top() ) );
                    pVDev->SetMapMode( aVDevMap );
                    pVDev->DrawGradient( aBoundRect, aGradient );
                    pVDev->SetFillColor( COL_BLACK );
                    pVDev->SetRasterOp( ROP_0 );
                    pVDev->DrawPolyPolygon( aPolyPoly );
                    pVDev->SetRasterOp( ROP_XOR );
                    pVDev->DrawGradient( aBoundRect, aGradient );
                    aVDevMap.SetOrigin( Point() );
                    pVDev->SetMapMode( aVDevMap );
                    DrawOutDev( aDstRect.TopLeft(), aDstSize, Point(), aDstSize, *pVDev );

                    EnableMapMode( bOldMap );
                }

                delete pVDev;
            }
        }
    }

    if( mpAlphaVDev )
        mpAlphaVDev->DrawPolyPolygon( rPolyPoly );
}

void OutputDevice::AddGradientActions( const Rectangle& rRect, const Gradient& rGradient,
                                       GDIMetaFile& rMtf )
{
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );
    DBG_CHKOBJ( &rGradient, Gradient, NULL );

    Rectangle aRect( rRect );

    aRect.Justify();

    // Wenn Rechteck leer ist, brauchen wir nichts machen
    if ( !aRect.IsEmpty() )
    {
        Gradient        aGradient( rGradient );
        GDIMetaFile*    pOldMtf = mpMetaFile;

        mpMetaFile = &rMtf;
        mpMetaFile->AddAction( new MetaPushAction( PUSH_ALL ) );
        mpMetaFile->AddAction( new MetaISectRectClipRegionAction( aRect ) );
        mpMetaFile->AddAction( new MetaLineColorAction( Color(), sal_False ) );

        // because we draw with no border line, we have to expand gradient
        // rect to avoid missing lines on the right and bottom edge
        aRect.Left()--;
        aRect.Top()--;
        aRect.Right()++;
        aRect.Bottom()++;

        // calculate step count if necessary
        if ( !aGradient.GetSteps() )
            aGradient.SetSteps( GRADIENT_DEFAULT_STEPCOUNT );

        if( aGradient.GetStyle() == GradientStyle_LINEAR || aGradient.GetStyle() == GradientStyle_AXIAL )
            ImplDrawLinearGradient( aRect, aGradient, sal_True, NULL );
        else
            ImplDrawComplexGradient( aRect, aGradient, sal_True, NULL );

        mpMetaFile->AddAction( new MetaPopAction() );
        mpMetaFile = pOldMtf;
    }
}

void OutputDevice::DrawHatch( const PolyPolygon& rPolyPoly, const Hatch& rHatch )
{
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

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
        sal_Bool            bOldMap = mbMap;

        aPolyPoly.Optimize( POLY_OPTIMIZE_NO_SAME );
        aHatch.SetDistance( ImplLogicWidthToDevicePixel( aHatch.GetDistance() ) );

        mpMetaFile = NULL;
        EnableMapMode( sal_False );
        Push( PUSH_LINECOLOR );
        SetLineColor( aHatch.GetColor() );
        ImplInitLineColor();
        ImplDrawHatch( aPolyPoly, aHatch, sal_False );
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
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    PolyPolygon aPolyPoly( rPolyPoly );
    aPolyPoly.Optimize( POLY_OPTIMIZE_NO_SAME | POLY_OPTIMIZE_CLOSE );

    if( aPolyPoly.Count() )
    {
        GDIMetaFile* pOldMtf = mpMetaFile;

        mpMetaFile = &rMtf;
        mpMetaFile->AddAction( new MetaPushAction( PUSH_ALL ) );
        mpMetaFile->AddAction( new MetaLineColorAction( rHatch.GetColor(), sal_True ) );
        ImplDrawHatch( aPolyPoly, rHatch, sal_True );
        mpMetaFile->AddAction( new MetaPopAction() );
        mpMetaFile = pOldMtf;
    }
}

void OutputDevice::ImplDrawHatch( const PolyPolygon& rPolyPoly, const Hatch& rHatch, sal_Bool bMtf )
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
            Point*      pPtBuffer = new Point[ HATCH_MAXPOINTS ];
            Point       aPt1, aPt2, aEndPt1;
            Size        aInc;

            // Single hatch
            aRect.Left() -= nLogPixelWidth; aRect.Top() -= nLogPixelWidth; aRect.Right() += nLogPixelWidth; aRect.Bottom() += nLogPixelWidth;
            ImplCalcHatchValues( aRect, nWidth, rHatch.GetAngle(), aPt1, aPt2, aInc, aEndPt1 );
            do
            {
                ImplDrawHatchLine( Line( aPt1, aPt2 ), rPolyPoly, pPtBuffer, bMtf );
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
                    ImplDrawHatchLine( Line( aPt1, aPt2 ), rPolyPoly, pPtBuffer, bMtf );
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
                        ImplDrawHatchLine( Line( aPt1, aPt2 ), rPolyPoly, pPtBuffer, bMtf );
                        aPt1.X() += aInc.Width(); aPt1.Y() += aInc.Height();
                        aPt2.X() += aInc.Width(); aPt2.Y() += aInc.Height();
                    }
                    while( ( aPt1.X() <= aEndPt1.X() ) && ( aPt1.Y() <= aEndPt1.Y() ) );
                }
            }

            delete[] pPtBuffer;
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
                                      Point* pPtBuffer, sal_Bool bMtf )
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
