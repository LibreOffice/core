/*************************************************************************
 *
 *  $RCSfile: outdev4.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: kz $ $Date: 2003-08-25 13:53:57 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#define _SV_OUTDEV_CXX

#include <math.h>

#ifndef REMOTE_APPSERVER
#ifndef _SV_SVSYS_HXX
#include <svsys.h>
#endif
#endif
#ifndef REMOTE_APPSERVER
#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif
#else
#ifndef _SV_RMOUTDEV_HXX
#include <rmoutdev.hxx>
#endif
#endif
#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SV_SVDATA_HXX
#include <svdata.hxx>
#endif
#ifndef _SV_GRADIENT_HXX
#include <gradient.hxx>
#endif
#ifndef _SV_METAACT_HXX
#include <metaact.hxx>
#endif
#ifndef _SV_GDIMTF_HXX
#include <gdimtf.hxx>
#endif
#ifndef _SV_OUTDATA_HXX
#include <outdata.hxx>
#endif
#ifndef _POLY_HXX
#include <tools/poly.hxx>
#endif
#ifndef _SV_SALBTYPE_HXX
#include <salbtype.hxx>
#endif
#ifndef _SV_LINE_HXX
#include <line.hxx>
#endif
#ifndef _SV_HATCH_HXX
#include <hatch.hxx>
#endif
#ifndef _SV_WINDOW_HXX
#include <window.hxx>
#endif
#ifndef _SV_VIRDEV_HXX
#include <virdev.hxx>
#endif
#ifndef _SV_OUTDEV_HXX
#include <outdev.hxx>
#endif

#include "pdfwriter_impl.hxx"

// -----------
// - Defines -
// -----------

#define HATCH_MAXPOINTS             1024
#define GRADIENT_DEFAULT_STEPCOUNT  0

// ----------------
// - Cmp-Function -
// ----------------

extern "C" int __LOADONCALLAPI ImplHatchCmpFnc( const void* p1, const void* p2 )
{
    const long nX1 = ( (Point*) p1 )->X();
    const long nX2 = ( (Point*) p2 )->X();
    const long nY1 = ( (Point*) p1 )->Y();
    const long nY2 = ( (Point*) p2 )->Y();

    return ( nX1 > nX2 ? 1 : nX1 == nX2 ? nY1 > nY2 ? 1: nY1 == nY2 ? 0 : -1 : -1 );
}

// =======================================================================

DBG_NAMEEX( OutputDevice );
DBG_NAMEEX( Gradient );

// =======================================================================

#ifndef REMOTE_APPSERVER

void OutputDevice::ImplDrawPolygon( const Polygon& rPoly, const PolyPolygon* pClipPolyPoly )
{
    if( pClipPolyPoly )
        ImplDrawPolyPolygon( rPoly, pClipPolyPoly );
    else
    {
        USHORT nPoints = rPoly.GetSize();

        if ( nPoints < 2 )
            return;

        const SalPoint* pPtAry = (const SalPoint*)rPoly.GetConstPointAry();
        mpGraphics->DrawPolygon( nPoints, pPtAry, this );
    }
}

// -----------------------------------------------------------------------

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
        USHORT          nSize = rPoly.GetSize();

        if( nSize >= 2 )
        {
            const SalPoint* pPtAry = (const SalPoint*)rPoly.GetConstPointAry();
            mpGraphics->DrawPolygon( nSize, pPtAry, this );
        }
    }
    else if( pPolyPoly->Count() )
    {
        USHORT              nCount = pPolyPoly->Count();
        ULONG*              pPointAry = new ULONG[nCount];
        PCONSTSALPOINT*     pPointAryAry = new PCONSTSALPOINT[nCount];
        USHORT              i = 0;
        do
        {
            const Polygon&  rPoly = pPolyPoly->GetObject( i );
            USHORT          nSize = rPoly.GetSize();
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

#endif

// -----------------------------------------------------------------------

inline UINT8 ImplGetGradientColorValue( long nValue )
{
    if ( nValue < 0 )
        return 0;
    else if ( nValue > 0xFF )
        return 0xFF;
    else
        return (UINT8)nValue;
}

// -----------------------------------------------------------------------

void OutputDevice::ImplDrawLinearGradient( const Rectangle& rRect,
                                           const Gradient& rGradient,
                                           BOOL bMtf, const PolyPolygon* pClipPolyPoly )
{
    // rotiertes BoundRect ausrechnen
    Rectangle aRect = rRect;
    aRect.Left()--;
    aRect.Top()--;
    aRect.Right()++;
    aRect.Bottom()++;
    USHORT  nAngle = rGradient.GetAngle() % 3600;
    double  fAngle  = nAngle * F_PI1800;
    double  fWidth  = aRect.GetWidth();
    double  fHeight = aRect.GetHeight();
    double  fDX     = fWidth  * fabs( cos( fAngle ) ) +
                      fHeight * fabs( sin( fAngle ) );
    double  fDY     = fHeight * fabs( cos( fAngle ) ) +
                      fWidth  * fabs( sin( fAngle ) );
            fDX     = (fDX - fWidth)  * 0.5 + 0.5;
            fDY     = (fDY - fHeight) * 0.5 + 0.5;
    aRect.Left()   -= (long)fDX;
    aRect.Right()  += (long)fDX;
    aRect.Top()    -= (long)fDY;
    aRect.Bottom() += (long)fDY;

    // Rand berechnen und Rechteck neu setzen
    Point       aCenter = rRect.Center();
    Rectangle   aFullRect = aRect;
    long        nBorder = (long)rGradient.GetBorder() * aRect.GetHeight() / 100;
    BOOL        bLinear;

    // Rand berechnen und Rechteck neu setzen fuer linearen Farbverlauf
    if ( rGradient.GetStyle() == GRADIENT_LINEAR )
    {
        bLinear = TRUE;
        aRect.Top() += nBorder;
    }
    // Rand berechnen und Rechteck neu setzen fuer axiale Farbverlauf
    else
    {
        bLinear = FALSE;
        nBorder >>= 1;

        aRect.Top()    += nBorder;
        aRect.Bottom() -= nBorder;
    }

    // Top darf nicht groesser als Bottom sein
    aRect.Top() = Min( aRect.Top(), (long)(aRect.Bottom() - 1) );

    long nMinRect = aRect.GetHeight();

    // Intensitaeten von Start- und Endfarbe ggf. aendern und
    // Farbschrittweiten berechnen
    long            nFactor;
    Color           aStartCol   = rGradient.GetStartColor();
    Color           aEndCol     = rGradient.GetEndColor();
    long            nStartRed   = aStartCol.GetRed();
    long            nStartGreen = aStartCol.GetGreen();
    long            nStartBlue  = aStartCol.GetBlue();
    long            nEndRed     = aEndCol.GetRed();
    long            nEndGreen   = aEndCol.GetGreen();
    long            nEndBlue    = aEndCol.GetBlue();
                    nFactor     = rGradient.GetStartIntensity();
                    nStartRed   = (nStartRed   * nFactor) / 100;
                    nStartGreen = (nStartGreen * nFactor) / 100;
                    nStartBlue  = (nStartBlue  * nFactor) / 100;
                    nFactor     = rGradient.GetEndIntensity();
                    nEndRed     = (nEndRed   * nFactor) / 100;
                    nEndGreen   = (nEndGreen * nFactor) / 100;
                    nEndBlue    = (nEndBlue  * nFactor) / 100;
    long            nRedSteps   = nEndRed   - nStartRed;
    long            nGreenSteps = nEndGreen - nStartGreen;
    long            nBlueSteps  = nEndBlue  - nStartBlue;
    long            nStepCount = rGradient.GetSteps();

    // Bei nicht linearen Farbverlaeufen haben wir nur die halben Steps
    // pro Farbe
    if ( !bLinear )
    {
        nRedSteps   <<= 1;
        nGreenSteps <<= 1;
        nBlueSteps  <<= 1;
    }

    // Anzahl der Schritte berechnen, falls nichts uebergeben wurde
    if ( !nStepCount )
    {
        long nInc;

        if ( meOutDevType != OUTDEV_PRINTER && !bMtf )
        {
            nInc = (nMinRect < 50) ? 2 : 4;
        }
        else
        {
            // #105998# Use display-equivalent step size calculation
            nInc = (nMinRect < 800) ? 10 : 20;
        }

        if ( !nInc )
            nInc = 1;

        nStepCount = nMinRect / nInc;
    }
    // minimal drei Schritte und maximal die Anzahl der Farbunterschiede
    long nSteps = Max( nStepCount, 2L );
    long nCalcSteps  = Abs( nRedSteps );
    long nTempSteps = Abs( nGreenSteps );
    if ( nTempSteps > nCalcSteps )
        nCalcSteps = nTempSteps;
    nTempSteps = Abs( nBlueSteps );
    if ( nTempSteps > nCalcSteps )
        nCalcSteps = nTempSteps;
    if ( nCalcSteps < nSteps )
        nSteps = nCalcSteps;
    if ( !nSteps )
        nSteps = 1;

    // Falls axialer Farbverlauf, muss die Schrittanzahl ungerade sein
    if ( !bLinear && !(nSteps & 1) )
        nSteps++;

    // Berechnung ueber Double-Addition wegen Genauigkeit
    double fScanLine = aRect.Top();
    double fScanInc  = (double)aRect.GetHeight() / (double)nSteps;

    // Startfarbe berechnen und setzen
    UINT8   nRed;
    UINT8   nGreen;
    UINT8   nBlue;
    long    nSteps2;
    long    nStepsHalf;
    if ( bLinear )
    {
        // Um 1 erhoeht, um die Border innerhalb der Schleife
        // zeichnen zu koennen
        nSteps2     = nSteps + 1;
        nRed        = (UINT8)nStartRed;
        nGreen      = (UINT8)nStartGreen;
        nBlue       = (UINT8)nStartBlue;
    }
    else
    {
        // Um 2 erhoeht, um die Border innerhalb der Schleife
        // zeichnen zu koennen
        nSteps2     = nSteps + 2;
        nRed        = (UINT8)nEndRed;
        nGreen      = (UINT8)nEndGreen;
        nBlue       = (UINT8)nEndBlue;
        nStepsHalf  = nSteps >> 1;
    }

    if ( bMtf )
        mpMetaFile->AddAction( new MetaFillColorAction( Color( nRed, nGreen, nBlue ), TRUE ) );
#ifndef REMOTE_APPSERVER
    else
        mpGraphics->SetFillColor( MAKE_SALCOLOR( nRed, nGreen, nBlue ) );
#endif

    // Startpolygon erzeugen (== Borderpolygon)
    Polygon     aPoly( 4 );
    Polygon     aTempPoly( 2 );
    aPoly[0] = aFullRect.TopLeft();
    aPoly[1] = aFullRect.TopRight();
    aPoly[2] = aRect.TopRight();
    aPoly[3] = aRect.TopLeft();
    aPoly.Rotate( aCenter, nAngle );

    // Schleife, um rotierten Verlauf zu fuellen
    for ( long i = 0; i < nSteps2; i++ )
    {
        // berechnetesPolygon ausgeben
        if ( bMtf )
            mpMetaFile->AddAction( new MetaPolygonAction( aPoly ) );
#ifndef REMOTE_APPSERVER
        else
            ImplDrawPolygon( aPoly, pClipPolyPoly );
#endif

        // neues Polygon berechnen
        aRect.Top() = (long)(fScanLine += fScanInc);

        // unteren Rand komplett fuellen
        if ( i == nSteps )
        {
            aTempPoly[0] = aFullRect.BottomLeft();
            aTempPoly[1] = aFullRect.BottomRight();
        }
        else
        {
            aTempPoly[0] = aRect.TopLeft();
            aTempPoly[1] = aRect.TopRight();
        }
        aTempPoly.Rotate( aCenter, nAngle );

        aPoly[0] = aPoly[3];
        aPoly[1] = aPoly[2];
        aPoly[2] = aTempPoly[1];
        aPoly[3] = aTempPoly[0];

        // Farbintensitaeten aendern...
        // fuer lineare FV
        if ( bLinear )
        {
            nRed    = ImplGetGradientColorValue( nStartRed+((nRedSteps*i)/nSteps2) );
            nGreen  = ImplGetGradientColorValue( nStartGreen+((nGreenSteps*i)/nSteps2) );
            nBlue   = ImplGetGradientColorValue( nStartBlue+((nBlueSteps*i)/nSteps2) );
        }
        // fuer radiale FV
        else
        {
            // fuer axiale FV muss die letzte Farbe der ersten
            // Farbe entsprechen
            // #107350# Setting end color one step earlier, as the
            // last time we get here, we drop out of the loop later
            // on.
            if ( i >= nSteps )
            {
                nRed    = (UINT8)nEndRed;
                nGreen  = (UINT8)nEndGreen;
                nBlue   = (UINT8)nEndBlue;
            }
            else
            {
                if ( i <= nStepsHalf )
                {
                    nRed    = ImplGetGradientColorValue( nEndRed-((nRedSteps*i)/nSteps2) );
                    nGreen  = ImplGetGradientColorValue( nEndGreen-((nGreenSteps*i)/nSteps2) );
                    nBlue   = ImplGetGradientColorValue( nEndBlue-((nBlueSteps*i)/nSteps2) );
                }
                // genau die Mitte und hoeher
                else
                {
                    long i2 = i - nStepsHalf;
                    nRed    = ImplGetGradientColorValue( nStartRed+((nRedSteps*i2)/nSteps2) );
                    nGreen  = ImplGetGradientColorValue( nStartGreen+((nGreenSteps*i2)/nSteps2) );
                    nBlue   = ImplGetGradientColorValue( nStartBlue+((nBlueSteps*i2)/nSteps2) );
                }
            }
        }

        if ( bMtf )
            mpMetaFile->AddAction( new MetaFillColorAction( Color( nRed, nGreen, nBlue ), TRUE ) );
#ifndef REMOTE_APPSERVER
        else
            mpGraphics->SetFillColor( MAKE_SALCOLOR( nRed, nGreen, nBlue ) );
#endif
    }
}

// -----------------------------------------------------------------------

void OutputDevice::ImplDrawComplexGradient( const Rectangle& rRect,
                                            const Gradient& rGradient,
                                            BOOL bMtf, const PolyPolygon* pClipPolyPoly )
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
    Rectangle       aRect( rRect );
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
    USHORT          nAngle = rGradient.GetAngle() % 3600;

    if( (meRasterOp != ROP_OVERPAINT) || (meOutDevType != OUTDEV_WINDOW) || bMtf )
        pPolyPoly = new PolyPolygon( 2 );
    else
        pPolyPoly = NULL;

    if( rGradient.GetStyle() == GRADIENT_SQUARE || rGradient.GetStyle() == GRADIENT_RECT )
    {
        const double    fAngle  = nAngle * F_PI1800;
        const double    fWidth  = aRect.GetWidth();
        const double    fHeight = aRect.GetHeight();
        double          fDX = fWidth  * fabs( cos( fAngle ) ) + fHeight * fabs( sin( fAngle ) );
        double          fDY = fHeight * fabs( cos( fAngle ) ) + fWidth  * fabs( sin( fAngle ) );

        fDX = ( fDX - fWidth ) * 0.5 + 0.5;
        fDY = ( fDY - fHeight ) * 0.5 + 0.5;

        aRect.Left() -= (long) fDX;
        aRect.Right() += (long) fDX;
        aRect.Top() -= (long) fDY;
        aRect.Bottom() += (long) fDY;
    }

    Size aSize( aRect.GetSize() );

    if( rGradient.GetStyle() == GRADIENT_RADIAL )
    {
        // Radien-Berechnung fuer Kreis
        aSize.Width() = (long)(0.5 + sqrt((double)aSize.Width()*(double)aSize.Width() + (double)aSize.Height()*(double)aSize.Height()));
        aSize.Height() = aSize.Width();
    }
    else if( rGradient.GetStyle() == GRADIENT_ELLIPTICAL )
    {
        // Radien-Berechnung fuer Ellipse
        aSize.Width() = (long)( 0.5 + (double) aSize.Width()  * 1.4142 );
        aSize.Height() = (long)( 0.5 + (double) aSize.Height() * 1.4142 );
    }
    else if( rGradient.GetStyle() == GRADIENT_SQUARE )
    {
        if ( aSize.Width() > aSize.Height() )
            aSize.Height() = aSize.Width();
        else
            aSize.Width() = aSize.Height();
    }

    // neue Mittelpunkte berechnen
    long    nZWidth = aRect.GetWidth()  * (long) rGradient.GetOfsX() / 100;
    long    nZHeight = aRect.GetHeight() * (long) rGradient.GetOfsY() / 100;
    long    nBorderX = (long) rGradient.GetBorder() * aSize.Width()  / 100;
    long    nBorderY = (long) rGradient.GetBorder() * aSize.Height() / 100;
    Point   aCenter( aRect.Left() + nZWidth, aRect.Top() + nZHeight );

    // Rand beruecksichtigen
    aSize.Width() -= nBorderX;
    aSize.Height() -= nBorderY;

    // Ausgaberechteck neu setzen
    aRect.Left() = aCenter.X() - ( aSize.Width() >> 1 );
    aRect.Top() = aCenter.Y() - ( aSize.Height() >> 1 );

    aRect.SetSize( aSize );
    long nMinRect = Min( aRect.GetWidth(), aRect.GetHeight() );

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

        if( !nInc )
            nInc = 1;

        nStepCount = nMinRect / nInc;
    }

    // minimal drei Schritte und maximal die Anzahl der Farbunterschiede
    long nSteps = Max( nStepCount, 2L );
    long nCalcSteps  = Abs( nRedSteps );
    long nTempSteps = Abs( nGreenSteps );
    if ( nTempSteps > nCalcSteps )
        nCalcSteps = nTempSteps;
    nTempSteps = Abs( nBlueSteps );
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
    double  fScanInc = (double) nMinRect / (double) nSteps * 0.5;
    UINT8   nRed = (UINT8) nStartRed, nGreen = (UINT8) nStartGreen, nBlue = (UINT8) nStartBlue;
    bool    bPaintLastPolygon( false ); // #107349# Paint last polygon only if loop has generated any output

    if( bMtf )
        mpMetaFile->AddAction( new MetaFillColorAction( Color( nRed, nGreen, nBlue ), TRUE ) );
#ifndef REMOTE_APPSERVER
    else
        mpGraphics->SetFillColor( MAKE_SALCOLOR( nRed, nGreen, nBlue ) );
#endif

    if( pPolyPoly )
    {
        pPolyPoly->Insert( aPoly = rRect );
        pPolyPoly->Insert( aPoly );
    }
#ifndef REMOTE_APPSERVER
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
#endif

    // Schleife, um nacheinander die Polygone/PolyPolygone auszugeben
    for( long i = 1; i < nSteps; i++ )
    {
        // neues Polygon berechnen
        aRect.Left() = (long)( fScanLeft += fScanInc );
        aRect.Top() = (long)( fScanTop += fScanInc );
        aRect.Right() = (long)( fScanRight -= fScanInc );
        aRect.Bottom() = (long)( fScanBottom -= fScanInc );

        if( ( aRect.GetWidth() < 2 ) || ( aRect.GetHeight() < 2 ) )
            break;

        if( rGradient.GetStyle() == GRADIENT_RADIAL || rGradient.GetStyle() == GRADIENT_ELLIPTICAL )
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
#ifndef REMOTE_APPSERVER
            else
                ImplDrawPolyPolygon( *pPolyPoly, pClipPolyPoly );
#endif

            // #107349# Set fill color _after_ geometry painting:
            // pPolyPoly's geometry is the band from last iteration's
            // aPoly to current iteration's aPoly. The window outdev
            // path (see else below), on the other hand, paints the
            // full aPoly. Thus, here, we're painting the band before
            // the one painted in the window outdev path below. To get
            // matching colors, have to delay color setting here.
            if( bMtf )
                mpMetaFile->AddAction( new MetaFillColorAction( Color( nRed, nGreen, nBlue ), TRUE ) );
            else
                mpGraphics->SetFillColor( MAKE_SALCOLOR( nRed, nGreen, nBlue ) );
        }
#ifndef REMOTE_APPSERVER
        else
        {
            // #107349# Set fill color _before_ geometry painting
            if( bMtf )
                mpMetaFile->AddAction( new MetaFillColorAction( Color( nRed, nGreen, nBlue ), TRUE ) );
            else
                mpGraphics->SetFillColor( MAKE_SALCOLOR( nRed, nGreen, nBlue ) );

            ImplDrawPolygon( aPoly, pClipPolyPoly );
        }
#endif
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
                mpMetaFile->AddAction( new MetaFillColorAction( Color( nRed, nGreen, nBlue ), TRUE ) );
                mpMetaFile->AddAction( new MetaPolygonAction( rPoly ) );
            }
#ifndef REMOTE_APPSERVER
            else
            {
                mpGraphics->SetFillColor( MAKE_SALCOLOR( nRed, nGreen, nBlue ) );
                   ImplDrawPolygon( rPoly, pClipPolyPoly );
            }
#endif
        }

        delete pPolyPoly;
    }
}

// -----------------------------------------------------------------------

void OutputDevice::DrawGradient( const Rectangle& rRect,
                                 const Gradient& rGradient )
{
    DBG_TRACE( "OutputDevice::DrawGradient()" );
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
            BYTE cStartLum = aStartCol.GetLuminance(), cEndLum = aEndCol.GetLuminance();
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
#ifndef REMOTE_APPSERVER
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
                mbInitLineColor = TRUE;
            }

            mbInitFillColor = TRUE;

            // calculate step count if neccessary
            if ( !aGradient.GetSteps() )
                aGradient.SetSteps( GRADIENT_DEFAULT_STEPCOUNT );

            if( aGradient.GetStyle() == GRADIENT_LINEAR || aGradient.GetStyle() == GRADIENT_AXIAL )
                ImplDrawLinearGradient( aRect, aGradient, FALSE, NULL );
            else
                ImplDrawComplexGradient( aRect, aGradient, FALSE, NULL );
        }

        Pop();
#else
        ImplServerGraphics* pGraphics = ImplGetServerGraphics();
        if ( pGraphics )
            pGraphics->DrawGradient( aRect, aGradient );
#endif
    }
}

// -----------------------------------------------------------------------

void OutputDevice::DrawGradient( const PolyPolygon& rPolyPoly,
                                 const Gradient& rGradient )
{
    DBG_TRACE( "OutputDevice::DrawGradient()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );
    DBG_CHKOBJ( &rGradient, Gradient, NULL );

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
                IntersectClipRegion( rPolyPoly );
                DrawGradient( aRect, rGradient );
                Pop();
            }
            else
            {
                const BOOL  bOldOutput = IsOutputEnabled();

                EnableOutput( FALSE );
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
                BYTE cStartLum = aStartCol.GetLuminance(), cEndLum = aEndCol.GetLuminance();
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

#ifndef REMOTE_APPSERVER
        if( OUTDEV_PRINTER == meOutDevType )
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
                            mbInitLineColor = TRUE;
                        }

                        mbInitFillColor = TRUE;

                        // calculate step count if neccessary
                        if ( !aGradient.GetSteps() )
                            aGradient.SetSteps( GRADIENT_DEFAULT_STEPCOUNT );

                        if( aGradient.GetStyle() == GRADIENT_LINEAR || aGradient.GetStyle() == GRADIENT_AXIAL )
                            ImplDrawLinearGradient( aRect, aGradient, FALSE, &aClipPolyPoly );
                        else
                            ImplDrawComplexGradient( aRect, aGradient, FALSE, &aClipPolyPoly );
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
                VirtualDevice   aVDev;
                const Size      aDstSize( aDstRect.GetSize() );

                if( aVDev.SetOutputSizePixel( aDstSize) )
                {
                    MapMode         aVDevMap;
                    const RasterOp  eOldROP = GetRasterOp();
                    const BOOL      bOldMap = mbMap;

                    mbMap = FALSE;

                    aVDev.DrawOutDev( Point(), aDstSize, aDstRect.TopLeft(), aDstSize, *this );
                    aVDev.SetRasterOp( ROP_XOR );
                    aVDevMap.SetOrigin( Point( -aDstRect.Left(), -aDstRect.Top() ) );
                    aVDev.SetMapMode( aVDevMap );
                    aVDev.DrawGradient( aBoundRect, aGradient );
                    aVDev.SetFillColor( COL_BLACK );
                    aVDev.SetRasterOp( ROP_0 );
                    aVDev.DrawPolyPolygon( aPolyPoly );
                    aVDev.SetRasterOp( ROP_XOR );
                    aVDev.DrawGradient( aBoundRect, aGradient );
                    aVDevMap.SetOrigin( Point() );
                    aVDev.SetMapMode( aVDevMap );
                    DrawOutDev( aDstRect.TopLeft(), aDstSize, Point(), aDstSize, aVDev );

                    mbMap = bOldMap;
                }
            }
        }
#else
        ImplServerGraphics* pGraphics = ImplGetServerGraphics();
        if ( pGraphics )
            pGraphics->DrawGradient( ImplLogicToDevicePixel( rPolyPoly ), aGradient );
#endif
    }
}

// -----------------------------------------------------------------------

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
        mpMetaFile->AddAction( new MetaLineColorAction( Color(), FALSE ) );

        // because we draw with no border line, we have to expand gradient
        // rect to avoid missing lines on the right and bottom edge
        aRect.Left()--;
        aRect.Top()--;
        aRect.Right()++;
        aRect.Bottom()++;

        // calculate step count if neccessary
        if ( !aGradient.GetSteps() )
            aGradient.SetSteps( GRADIENT_DEFAULT_STEPCOUNT );

        if( aGradient.GetStyle() == GRADIENT_LINEAR || aGradient.GetStyle() == GRADIENT_AXIAL )
            ImplDrawLinearGradient( aRect, aGradient, TRUE, NULL );
        else
            ImplDrawComplexGradient( aRect, aGradient, TRUE, NULL );

        mpMetaFile->AddAction( new MetaPopAction() );
        mpMetaFile = pOldMtf;
    }
}

// -----------------------------------------------------------------------

void OutputDevice::DrawHatch( const PolyPolygon& rPolyPoly, const Hatch& rHatch )
{
    DBG_TRACE( "OutputDevice::DrawHatch()" );
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
            const UINT8 cLum = aColor.GetLuminance();
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

#ifndef REMOTE_APPSERVER
    if( !mpGraphics && !ImplGetGraphics() )
        return;

    if( mbInitClipRegion )
        ImplInitClipRegion();

    if( mbOutputClipped )
        return;
#endif

    if( rPolyPoly.Count() )
    {
#ifndef REMOTE_APPSERVER
        PolyPolygon     aPolyPoly( LogicToPixel( rPolyPoly ) );
        GDIMetaFile*    pOldMetaFile = mpMetaFile;
        BOOL            bOldMap = mbMap;

        aPolyPoly.Optimize( POLY_OPTIMIZE_NO_SAME );
        aHatch.SetDistance( ImplLogicWidthToDevicePixel( aHatch.GetDistance() ) );

        mpMetaFile = NULL;
        mbMap = FALSE;
        Push( PUSH_LINECOLOR );
        SetLineColor( aHatch.GetColor() );
        ImplInitLineColor();
        ImplDrawHatch( aPolyPoly, aHatch, FALSE );
        Pop();
        mbMap = bOldMap;
        mpMetaFile = pOldMetaFile;
#else
        ImplServerGraphics* pGraphics = ImplGetServerGraphics();
        if ( pGraphics )
        {
            PolyPolygon aPolyPoly( ImplLogicToDevicePixel( rPolyPoly ) );

            aPolyPoly.Optimize( POLY_OPTIMIZE_NO_SAME );
            aHatch.SetDistance( ImplLogicWidthToDevicePixel( aHatch.GetDistance() ) );
            pGraphics->DrawHatch( aPolyPoly, aHatch );
        }
#endif
    }
}

// -----------------------------------------------------------------------

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
        mpMetaFile->AddAction( new MetaLineColorAction( rHatch.GetColor(), TRUE ) );
        ImplDrawHatch( aPolyPoly, rHatch, TRUE );
        mpMetaFile->AddAction( new MetaPopAction() );
        mpMetaFile = pOldMtf;
    }
}

// -----------------------------------------------------------------------

void OutputDevice::ImplDrawHatch( const PolyPolygon& rPolyPoly, const Hatch& rHatch, BOOL bMtf )
{
    Rectangle   aRect( rPolyPoly.GetBoundRect() );
    const long  nLogPixelWidth = ImplDevicePixelToLogicWidth( 1 );
    const long  nWidth = ImplDevicePixelToLogicWidth( Max( ImplLogicWidthToDevicePixel( rHatch.GetDistance() ), 3L ) );
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

// -----------------------------------------------------------------------

void OutputDevice::ImplCalcHatchValues( const Rectangle& rRect, long nDist, USHORT nAngle10,
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

// ------------------------------------------------------------------------

void OutputDevice::ImplDrawHatchLine( const Line& rLine, const PolyPolygon& rPolyPoly,
                                      Point* pPtBuffer, BOOL bMtf )
{
    double  fX, fY;
    long    nAdd, nPCounter = 0;

    for( long nPoly = 0, nPolyCount = rPolyPoly.Count(); nPoly < nPolyCount; nPoly++ )
    {
        const Polygon& rPoly = rPolyPoly[ (USHORT) nPoly ];

        if( rPoly.GetSize() > 1 )
        {
            Point   aIntersection;
            Line    aCurSegment( rPoly[ 0 ], Point() );

            for( long i = 1, nCount = rPoly.GetSize(); i <= nCount; i++ )
            {
                aCurSegment.SetEnd( rPoly[ (USHORT)( i % nCount ) ] );
                nAdd = 0;

                if( rLine.Intersection( aCurSegment, fX, fY ) )
                {
                    if( ( fabs( fX - aCurSegment.GetStart().X() ) <= 0.0000001 ) &&
                        ( fabs( fY - aCurSegment.GetStart().Y() ) <= 0.0000001 ) )
                    {
                        const Line      aPrevSegment( rPoly[ (USHORT)( ( i > 1 ) ? ( i - 2 ) : ( nCount - 1 ) ) ], aCurSegment.GetStart() );
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
                        const Line aNextSegment( aCurSegment.GetEnd(), rPoly[ (USHORT)( ( i + 1 ) % nCount ) ] );

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
#ifndef REMOTE_APPSERVER
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
#endif
        }
    }
}
