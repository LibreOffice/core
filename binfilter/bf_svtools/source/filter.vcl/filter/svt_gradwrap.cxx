/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove


#include <math.h>
#include <svgrad.hxx>
#include <svbmpacc.hxx>
#include <gradwrap.hxx>

namespace binfilter
{

// -------------------
// - GradientWrapper -
// -------------------

GradientWrapper::GradientWrapper(const Link& rDrawPolyRecordHdl,
                                 const Link& rDrawPolyPolyRecordHdl,
                                 const Link& rSetFillInBrushRecordHdl) :
            aDrawPolyRecordHdl		(rDrawPolyRecordHdl),
            aDrawPolyPolyRecordHdl	(rDrawPolyPolyRecordHdl),
            aSetFillInBrushRecordHdl(rSetFillInBrushRecordHdl)
{
}

// ------------------------------------------------------------------------

GradientWrapper::~GradientWrapper()
{
}

// ------------------------------------------------------------------------

void GradientWrapper::WriteLinearGradient(const Rectangle& rRect,
                                          const Gradient& rGradient)
{
    USHORT nStepCount = 100;

    Rectangle aRect = rRect;
    aRect.Left()--;
    aRect.Top()--;
    aRect.Right()++;
    aRect.Bottom()++;

    // rotiertes BoundRect ausrechnen
    double  fAngle  = (rGradient.GetAngle() % 3600) * F_PI1800;
    double  fWidth  = aRect.GetWidth();
    double  fHeight = aRect.GetHeight();
    double  fDX     = fWidth  * fabs( cos( fAngle ) ) +
                      fHeight * fabs( sin( fAngle ) );
    double  fDY     = fHeight * fabs( cos( fAngle ) ) +
                      fWidth  * fabs( sin( fAngle ) );
            fDX = (fDX - fWidth)  * 0.5 + 0.5;
            fDY = (fDY - fHeight) * 0.5 + 0.5;
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

    // Anzahl der Schritte berechnen, falls nichts uebergeben wurde
    if ( !nStepCount )
    {
        long nInc = ((nMinRect >> 9) + 1) << 3;

        if ( !nInc )
            nInc = 1;

        nStepCount = (USHORT)(nMinRect / nInc);
    }
    // minimal drei Schritte
    long nSteps = Max( nStepCount, (USHORT)3 );

    // Falls axialer Farbverlauf, muss die Schrittanzahl ungerade sein
    if ( !bLinear && !(nSteps & 1) )
        nSteps++;

    // Berechnung ueber Double-Addition wegen Genauigkeit
    double fScanLine = aRect.Top();
    double fScanInc  = (double)aRect.GetHeight() / (double)nSteps;

    // Intensitaeten von Start- und Endfarbe ggf. aendern und
    // Farbschrittweiten berechnen
    long            nFactor;
    const Color&    rStartCol   = rGradient.GetStartColor();
    const Color&    rEndCol     = rGradient.GetEndColor();
    long            nRed        = rStartCol.GetRed();
    long            nGreen      = rStartCol.GetGreen();
    long            nBlue       = rStartCol.GetBlue();
    long            nEndRed     = rEndCol.GetRed();
    long            nEndGreen   = rEndCol.GetGreen();
    long            nEndBlue    = rEndCol.GetBlue();
    nFactor                     = rGradient.GetStartIntensity();
    nRed                        = (nRed   * nFactor) / 100;
    nGreen                      = (nGreen * nFactor) / 100;
    nBlue                       = (nBlue  * nFactor) / 100;
    nFactor                     = rGradient.GetEndIntensity();
    nEndRed                     = (nEndRed   * nFactor) / 100;
    nEndGreen                   = (nEndGreen * nFactor) / 100;
    nEndBlue                    = (nEndBlue  * nFactor) / 100;
    long            nStepRed    = (nEndRed   - nRed)   / nSteps;
    long            nStepGreen  = (nEndGreen - nGreen) / nSteps;
    long            nStepBlue   = (nEndBlue  - nBlue)  / nSteps;
    long            nSteps2;

    if ( bLinear )
    {
        // Um 1 erhoeht, um die Border innerhalb der Schleife
        // zeichnen zu koennen
        nSteps2 = nSteps + 1;
    }
    else
    {
        nStepRed   <<= 1;
        nStepGreen <<= 1;
        nStepBlue  <<= 1;
        nRed   = nEndRed;
        nGreen = nEndGreen;
        nBlue  = nEndBlue;

        // Um 2 erhoeht, um die Border innerhalb der Schleife
        // zeichnen zu koennen
        nSteps2 = nSteps + 2;
    }
    Color aCol( (BYTE) nRed, (BYTE) nGreen, (BYTE) nBlue );

    // GDI-Objekte sichern und setzen
    aSetFillInBrushRecordHdl.Call(&aCol);

    // Startpolygon erzeugen (== Borderpolygon)
    Polygon     aPoly( 4 );
    Polygon     aTempPoly( 2 );
    aPoly[0] = aFullRect.TopLeft();
    aPoly[1] = aFullRect.TopRight();
    aPoly[2] = aRect.TopRight();
    aPoly[3] = aRect.TopLeft();
    aPoly.Rotate( aCenter, rGradient.GetAngle() );

    // Schleife, um rotierten Verlauf zu fuellen
    for ( long i = 0; i < nSteps2; i++ )
    {
        Polygon aTempPoly = aPoly;
        aTempPoly.Clip( rRect );
        aDrawPolyRecordHdl.Call(&aTempPoly);
        aTempPoly.SetSize( 2 );

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
        aTempPoly.Rotate( aCenter, rGradient.GetAngle() );

        aPoly[0] = aPoly[3];
        aPoly[1] = aPoly[2];
        aPoly[2] = aTempPoly[1];
        aPoly[3] = aTempPoly[0];

        // Farbintensitaeten aendern...
        // fuer lineare FV
        if ( bLinear )
        {
            nRed   += nStepRed;
            nGreen += nStepGreen;
            nBlue  += nStepBlue;
        }
        // fuer radiale FV
        else
        {
            if ( i <= (nSteps >> 1) )
            {
                nRed   -= nStepRed;
                nGreen -= nStepGreen;
                nBlue  -= nStepBlue;
            }
            // genau die Mitte und hoeher
            else
            {
                nRed   += nStepRed;
                nGreen += nStepGreen;
                nBlue  += nStepBlue;
            }
        }

        nRed    = MinMax( nRed,   0, 255 );
        nGreen  = MinMax( nGreen, 0, 255 );
        nBlue   = MinMax( nBlue,  0, 255 );

        // fuer lineare FV ganz normale Bestimmung der Farbe
        if ( bLinear || (i <= nSteps) )
        {
            aCol = Color( (BYTE) nRed, (BYTE) nGreen, (BYTE) nBlue );
        }
        // fuer axiale FV muss die letzte Farbe der ersten
        // Farbe entsprechen
        else
        {
            aCol = Color( (BYTE) nEndRed, (BYTE) nEndGreen, (BYTE) nEndBlue );
        }

        aSetFillInBrushRecordHdl.Call(&aCol);
    }
}

// ------------------------------------------------------------------------

void GradientWrapper::WriteRadialGradient(const Rectangle& rRect,
                                          const Gradient& rGradient)
{
    USHORT  	nStepCount = 100;
    Rectangle   aClipRect = rRect;
    Rectangle   aRect   = rRect;
    long        nZWidth = aRect.GetWidth() * (long)rGradient.GetOfsX() / 100;
    long        nZHeight= aRect.GetHeight() * (long)rGradient.GetOfsY() / 100;
    Size        aSize   = aRect.GetSize();
    Point       aCenter( aRect.Left() + nZWidth, aRect.Top() + nZHeight );

    // Radien-Berechnung fuer Kreisausgabe (Kreis schliesst Rechteck ein)
    if ( rGradient.GetStyle() == GRADIENT_RADIAL )
    {
        aSize.Width()   = (long)(0.5 + sqrt((double)aSize.Width()*(double)aSize.Width() +
                                            (double)aSize.Height()*(double)aSize.Height()));
        aSize.Height()  = aSize.Width();
    }
    // Radien-Berechnung fuer Ellipse
    else
    {
        aSize.Width()   = (long)(0.5 + (double)aSize.Width()  * 1.4142);
        aSize.Height()  = (long)(0.5 + (double)aSize.Height() * 1.4142);
    }

    long nBorderX   = (long)rGradient.GetBorder() * aSize.Width()  / 100;
    long nBorderY   = (long)rGradient.GetBorder() * aSize.Height() / 100;
    aSize.Width()  -= nBorderX;
    aSize.Height() -= nBorderY;
    aRect.Left()    = aCenter.X() - (aSize.Width()  >> 1);
    aRect.Top()     = aCenter.Y() - (aSize.Height() >> 1);
    aRect.SetSize( aSize );

    long nMinRect = Min( aRect.GetWidth(), aRect.GetHeight() );

    // Anzahl der Schritte berechnen, falls nichts uebergeben wurde
    if ( !nStepCount )
    {
        long nInc = ((nMinRect >> 9) + 1) << 3;

        if ( !nInc )
            nInc = 1;

        nStepCount = (USHORT)(nMinRect / nInc);
    }
    // minimal drei Schritte
    long nSteps = Max( nStepCount, (USHORT)3 );

    // Ausgabebegrenzungen und Schrittweite fuer jede Richtung festlegen
    double fScanLeft   = aRect.Left();
    double fScanTop    = aRect.Top();
    double fScanRight  = aRect.Right();
    double fScanBottom = aRect.Bottom();
    double fScanInc    = (double)nMinRect / (double)nSteps * 0.5;

    // Intensitaeten von Start- und Endfarbe ggf. aendern und
    // Farbschrittweiten berechnen
    long            nFactor;
    const Color&    rStartCol   = rGradient.GetStartColor();
    const Color&    rEndCol     = rGradient.GetEndColor();
    long            nRed        = rStartCol.GetRed();
    long            nGreen      = rStartCol.GetGreen();
    long            nBlue       = rStartCol.GetBlue();
    long            nEndRed     = rEndCol.GetRed();
    long            nEndGreen   = rEndCol.GetGreen();
    long            nEndBlue    = rEndCol.GetBlue();
    nFactor                     = rGradient.GetStartIntensity();
    nRed                        = (nRed   * nFactor) / 100;
    nGreen                      = (nGreen * nFactor) / 100;
    nBlue                       = (nBlue  * nFactor) / 100;
    nFactor                     = rGradient.GetEndIntensity();
    nEndRed                     = (nEndRed   * nFactor) / 100;
    nEndGreen                   = (nEndGreen * nFactor) / 100;
    nEndBlue                    = (nEndBlue  * nFactor) / 100;
    long            nStepRed    = (nEndRed   - nRed)   / nSteps;
    long            nStepGreen  = (nEndGreen - nGreen) / nSteps;
    long            nStepBlue   = (nEndBlue  - nBlue)  / nSteps;
    Color           aCol( (BYTE) nRed, (BYTE) nGreen, (BYTE) nBlue );

    // GDI-Objekte sichern und setzen
    aSetFillInBrushRecordHdl.Call(&aCol);

    // Recteck erstmal ausgeben
    PolyPolygon aPolyPoly( 2 );
    Polygon     aPoly( rRect );

    aPolyPoly.Insert( aPoly );
    aPoly = Polygon( aRect );
    aPoly.Rotate( aCenter, rGradient.GetAngle() );
    aPolyPoly.Insert( aPoly );

    // erstes Polygon zeichnen (entspricht Rechteck)
    PolyPolygon aTempPolyPoly = aPolyPoly;
    aTempPolyPoly.Clip( aClipRect );
    aDrawPolyPolyRecordHdl.Call(&aTempPolyPoly);

    for ( long i = 0; i < nSteps; i++ )
    {
        Color aCol( (BYTE) nRed, (BYTE) nGreen, (BYTE) nBlue );
        aSetFillInBrushRecordHdl.Call(&aCol);

        // neues Polygon berechnen
        aRect.Left()    = (long)(fScanLeft   += fScanInc);
        aRect.Top()     = (long)(fScanTop    += fScanInc);
        aRect.Right()   = (long)(fScanRight  -= fScanInc);
        aRect.Bottom()  = (long)(fScanBottom -= fScanInc);

        if ( (aRect.GetWidth() < 2) || (aRect.GetHeight() < 2) )
            break;

        aPoly = Polygon( aRect.Center(),
                         aRect.GetWidth() >> 1, aRect.GetHeight() >> 1 );
        aPoly.Rotate( aCenter, rGradient.GetAngle() );

        aPolyPoly.Replace( aPolyPoly.GetObject( 1 ), 0 );
        aPolyPoly.Replace( aPoly, 1 );

        PolyPolygon aTempPolyPoly = aPolyPoly;
        aTempPolyPoly.Clip( aClipRect );
        aDrawPolyPolyRecordHdl.Call(&aTempPolyPoly);

        // Farbe entsprechend anpassen
        nRed   += nStepRed;
        nGreen += nStepGreen;
        nBlue  += nStepBlue;

        nRed    = MinMax( nRed,   0, 0xFF );
        nGreen  = MinMax( nGreen, 0, 0xFF );
        nBlue   = MinMax( nBlue,  0, 0xFF );
    }

    // Falls PolyPolygon-Ausgabe, muessen wir noch ein letztes
    // inneres Polygon zeichnen
    aCol = Color( (BYTE) nRed, (BYTE) nGreen, (BYTE) nBlue );
    aSetFillInBrushRecordHdl.Call(&aCol);

    aPoly = aPolyPoly.GetObject( 1 );
    if ( !aPoly.GetBoundRect().IsEmpty() )
    {
        aPoly.Clip( aClipRect );
        aDrawPolyRecordHdl.Call(&aPoly);
    }
}

// ------------------------------------------------------------------------

void GradientWrapper::WriteRectGradient(const Rectangle& rRect,
                                        const Gradient& rGradient)
{
    USHORT  	nStepCount = 100;
    Rectangle   aClipRect = rRect;
    Rectangle 	aRect = rRect;

    aRect.Left()--;
    aRect.Top()--;
    aRect.Right()++;
    aRect.Bottom()++;

    // rotiertes BoundRect ausrechnen
    double  fAngle  = (rGradient.GetAngle() % 3600) * F_PI1800;
    double  fWidth  = aRect.GetWidth();
    double  fHeight = aRect.GetHeight();
    double  fDX     = fWidth  * fabs( cos( fAngle ) ) +
                      fHeight * fabs( sin( fAngle ) );
    double  fDY     = fHeight * fabs( cos( fAngle ) ) +
                      fWidth  * fabs( sin( fAngle ) );
            fDX = (fDX - fWidth)  * 0.5 + 0.5;
            fDY = (fDY - fHeight) * 0.5 + 0.5;
    aRect.Left()   -= (long)fDX;
    aRect.Right()  += (long)fDX;
    aRect.Top()    -= (long)fDY;
    aRect.Bottom() += (long)fDY;

    // Quadratisch machen, wenn angefordert;
    Size aSize = aRect.GetSize();
    if ( rGradient.GetStyle() == GRADIENT_SQUARE )
    {
        if ( aSize.Width() > aSize.Height() )
            aSize.Height() = aSize.Width();
        else
            aSize.Width() = aSize.Height();
    }

    // neue Mittelpunkte berechnen
    long    nZWidth     = aRect.GetWidth()  * (long)rGradient.GetOfsX() / 100;
    long    nZHeight    = aRect.GetHeight() * (long)rGradient.GetOfsY() / 100;
    long    nBorderX    = (long)rGradient.GetBorder() * aSize.Width()  / 100;
    long    nBorderY    = (long)rGradient.GetBorder() * aSize.Height() / 100;
    Point   aCenter( aRect.Left() + nZWidth, aRect.Top() + nZHeight );

    // Rand beruecksichtigen
    aSize.Width()   -= nBorderX;
    aSize.Height()  -= nBorderY;

    // Ausgaberechteck neu setzen
    aRect.Left() = aCenter.X() - (aSize.Width() >> 1);
    aRect.Top()  = aCenter.Y() - (aSize.Height() >> 1);
    aRect.SetSize( aSize );

    long nMinRect = Min( aRect.GetWidth(), aRect.GetHeight() );

    // Anzahl der Schritte berechnen, falls nichts uebergeben wurde
    if ( !nStepCount )
    {
        long nInc = ((nMinRect >> 9) + 1) << 3;

        if ( !nInc )
            nInc = 1;

        nStepCount = (USHORT)(nMinRect / nInc);
    }
    // minimal drei Schritte
    long nSteps = Max( nStepCount, (USHORT)3 );

    // Ausgabebegrenzungen und Schrittweite fuer jede Richtung festlegen
    double fScanLeft   = aRect.Left();
    double fScanTop    = aRect.Top();
    double fScanRight  = aRect.Right();
    double fScanBottom = aRect.Bottom();
    double fScanInc    = (double)nMinRect / (double)nSteps * 0.5;

    // Intensitaeten von Start- und Endfarbe ggf. aendern und
    // Farbschrittweiten berechnen
    long            nFactor;
    const Color&    rStartCol   = rGradient.GetStartColor();
    const Color&    rEndCol     = rGradient.GetEndColor();
    long            nRed        = rStartCol.GetRed();
    long            nGreen      = rStartCol.GetGreen();
    long            nBlue       = rStartCol.GetBlue();
    long            nEndRed     = rEndCol.GetRed();
    long            nEndGreen   = rEndCol.GetGreen();
    long            nEndBlue    = rEndCol.GetBlue();
    nFactor                     = rGradient.GetStartIntensity();
    nRed                        = (nRed   * nFactor) / 100;
    nGreen                      = (nGreen * nFactor) / 100;
    nBlue                       = (nBlue  * nFactor) / 100;
    nFactor                     = rGradient.GetEndIntensity();
    nEndRed                     = (nEndRed   * nFactor) / 100;
    nEndGreen                   = (nEndGreen * nFactor) / 100;
    nEndBlue                    = (nEndBlue  * nFactor) / 100;
    long            nStepRed    = (nEndRed   - nRed)   / nSteps;
    long            nStepGreen  = (nEndGreen - nGreen) / nSteps;
    long            nStepBlue   = (nEndBlue  - nBlue)  / nSteps;
    Color           aCol( (BYTE) nRed, (BYTE) nGreen, (BYTE) nBlue );

    // GDI-Objekte sichern und setzen
    aSetFillInBrushRecordHdl.Call(&aCol);

    // Recteck erstmal ausgeben
    PolyPolygon aPolyPoly( 2 );
    Polygon     aPoly( rRect );

    aPolyPoly.Insert( aPoly );
    aPoly = Polygon( aRect );
    aPoly.Rotate( aCenter, rGradient.GetAngle() );
    aPolyPoly.Insert( aPoly );

    PolyPolygon aTempPolyPoly = aPolyPoly;
    aTempPolyPoly.Clip( aClipRect );
    aDrawPolyPolyRecordHdl.Call(&aTempPolyPoly);

    // Schleife, um nacheinander die Polygone/PolyPolygone auszugeben
    for ( long i = 0; i < nSteps; i++ )
    {
        Color aCol( (BYTE) nRed, (BYTE) nGreen, (BYTE) nBlue );
        aSetFillInBrushRecordHdl.Call(&aCol);

        // neues Polygon berechnen
        aRect.Left()    = (long)(fScanLeft  += fScanInc);
        aRect.Top()     = (long)(fScanTop   += fScanInc);
        aRect.Right()   = (long)(fScanRight -= fScanInc);
        aRect.Bottom()  = (long)(fScanBottom-= fScanInc);

        if ( (aRect.GetWidth() < 2) || (aRect.GetHeight() < 2) )
            break;

        aPoly = Polygon( aRect );
        aPoly.Rotate( aCenter, rGradient.GetAngle() );

        aPolyPoly.Replace( aPolyPoly.GetObject( 1 ), 0 );
        aPolyPoly.Replace( aPoly, 1 );

        PolyPolygon aTempPolyPoly = aPolyPoly;
        aTempPolyPoly.Clip( aClipRect );
        aDrawPolyPolyRecordHdl.Call(&aTempPolyPoly);

        // Farben aendern
        nRed   += nStepRed;
        nGreen += nStepGreen;
        nBlue  += nStepBlue;

        nRed    = MinMax( nRed,   0, 0xFF );
        nGreen  = MinMax( nGreen, 0, 0xFF );
        nBlue   = MinMax( nBlue,  0, 0xFF );
    }

    aCol = Color( (BYTE) nRed, (BYTE) nGreen, (BYTE) nBlue );
    aSetFillInBrushRecordHdl.Call(&aCol);

    aPoly = aPolyPoly.GetObject( 1 );
    if ( !aPoly.GetBoundRect().IsEmpty() )
    {
        aPoly.Clip( aClipRect );
        aDrawPolyRecordHdl.Call(&aPoly);
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
