/*************************************************************************
 *
 *  $RCSfile: gradwrap.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:59 $
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

#include <math.h>
#include <svgrad.hxx>
#include <svbmpacc.hxx>
#include <gradwrap.hxx>

// -------------------
// - GradientWrapper -
// -------------------

GradientWrapper::GradientWrapper(const Link& rDrawPolyRecordHdl,
                                 const Link& rDrawPolyPolyRecordHdl,
                                 const Link& rSetFillInBrushRecordHdl) :
            aDrawPolyRecordHdl      (rDrawPolyRecordHdl),
            aDrawPolyPolyRecordHdl  (rDrawPolyPolyRecordHdl),
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
    Brush aBrush( aCol );
    aSetFillInBrushRecordHdl.Call(&aBrush);

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

        aBrush.SetColor(aCol);
        aSetFillInBrushRecordHdl.Call(&aBrush);
    }
}

// ------------------------------------------------------------------------

void GradientWrapper::WriteRadialGradient(const Rectangle& rRect,
                                          const Gradient& rGradient)
{
    USHORT      nStepCount = 100;
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
    Brush aBrush( aCol );
    aSetFillInBrushRecordHdl.Call(&aBrush);

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
        aBrush.SetColor( aCol );
        aSetFillInBrushRecordHdl.Call(&aBrush);

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

#ifndef VCL
        nRed    = MinMax( nRed,   0, 0xFFFF );
        nGreen  = MinMax( nGreen, 0, 0xFFFF );
        nBlue   = MinMax( nBlue,  0, 0xFFFF );
#else
        nRed    = MinMax( nRed,   0, 0xFF );
        nGreen  = MinMax( nGreen, 0, 0xFF );
        nBlue   = MinMax( nBlue,  0, 0xFF );
#endif
    }

    // Falls PolyPolygon-Ausgabe, muessen wir noch ein letztes
    // inneres Polygon zeichnen
    aBrush.SetColor( Color( (BYTE) nRed, (BYTE) nGreen, (BYTE) nBlue ) );
    aSetFillInBrushRecordHdl.Call(&aBrush);

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
    USHORT      nStepCount = 100;
    Rectangle   aClipRect = rRect;
    Rectangle   aRect = rRect;

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
    Brush aBrush( aCol );
    aSetFillInBrushRecordHdl.Call(&aBrush);

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
        aBrush.SetColor( aCol );
        aSetFillInBrushRecordHdl.Call(&aBrush);

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

#ifndef VCL
        nRed    = MinMax( nRed,   0, 65535 );
        nGreen  = MinMax( nGreen, 0, 65535 );
        nBlue   = MinMax( nBlue,  0, 65535 );
#else
        nRed    = MinMax( nRed,   0, 0xFF );
        nGreen  = MinMax( nGreen, 0, 0xFF );
        nBlue   = MinMax( nBlue,  0, 0xFF );
#endif
    }

    aBrush.SetColor( Color( (BYTE) nRed, (BYTE) nGreen, (BYTE) nBlue ) );
    aSetFillInBrushRecordHdl.Call(&aBrush);

    aPoly = aPolyPoly.GetObject( 1 );
    if ( !aPoly.GetBoundRect().IsEmpty() )
    {
        aPoly.Clip( aClipRect );
        aDrawPolyRecordHdl.Call(&aPoly);
    }
}
