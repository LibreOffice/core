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
#include "precompiled_svx.hxx"
#include <svx/viewpt3d.hxx>
#include <svx/volume3d.hxx>

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

Viewport3D::Viewport3D() :
    aVRP(0, 0, 5),
    aVPN(0, 0, 1),
    aVUV(0, 1, 1),
    aPRP(0, 0, 2),
    fVPD(-3),
    fNearClipDist (0.0),
    fFarClipDist (0.0),
    eProjection(PR_PERSPECTIVE),
    eAspectMapping(AS_NO_MAPPING),
    aDeviceRect(Point(0,0), Size(-1,-1)),
    aViewPoint (0, 0, 5000),
    bTfValid(0),
    fWRatio (1.0),
    fHRatio (1.0)
{
    aViewWin.X = -1; aViewWin.Y = -1;
    aViewWin.W =  2; aViewWin.H = 2;
}

/*************************************************************************
|*
|* ViewWindow (in View-Koordinaten) setzen
|*
\************************************************************************/

void Viewport3D::SetViewWindow(double fX, double fY, double fW, double fH)
{
    aViewWin.X = fX;
    aViewWin.Y = fY;
    if ( fW > 0 )   aViewWin.W = fW;
    else            aViewWin.W = 1.0;
    if ( fH > 0 )   aViewWin.H = fH;
    else            aViewWin.H = 1.0;

    fWRatio = aDeviceRect.GetWidth() / aViewWin.W;
    fHRatio = aDeviceRect.GetHeight() / aViewWin.H;
}

/*************************************************************************
|*
|* ViewWindow zurueckgeben
|*
\************************************************************************/

void Viewport3D::GetViewWindow(double& rX, double& rY,
                               double& rW, double& rH) const
{
    rX = aViewWin.X;
    rY = aViewWin.Y;
    rW = aViewWin.W;
    rH = aViewWin.H;
}

/*************************************************************************
|*
|* Beobachterposition (PRP) in Weltkoordinaten zurueckgeben
|*
\************************************************************************/

const basegfx::B3DPoint& Viewport3D::GetViewPoint()
{
    MakeTransform();

    return aViewPoint;
}

/*************************************************************************
|*
|* Transformationsmatrix zurueckgeben
|*
\************************************************************************/

const basegfx::B3DHomMatrix& Viewport3D::GetViewTransform()
{
    MakeTransform();

    return aViewTf;
}







/*************************************************************************
|*
|* View-Transformationsmatrix berechnen
|*
\************************************************************************/

void Viewport3D::MakeTransform(void)
{
    if ( !bTfValid )
    {
        double fV, fXupVp, fYupVp;
        aViewPoint = aVRP + aVPN * aPRP.getZ();

        // auf Einheitsmatrix zuruecksetzen
        aViewTf.identity();

        // in den Ursprung verschieben
        aViewTf.translate(-aVRP.getX(), -aVRP.getY(), -aVRP.getZ());

        // fV = Laenge der Projektion von aVPN auf die yz-Ebene:
        fV = aVPN.getYZLength();

        if ( fV != 0 )
        {
            basegfx::B3DHomMatrix aTemp;
            const double fSin(aVPN.getY() / fV);
            const double fCos(aVPN.getZ() / fV);
            aTemp.set(2, 2, fCos);
            aTemp.set(1, 1, fCos);
            aTemp.set(2, 1, fSin);
            aTemp.set(1, 2, -fSin);
            aViewTf *= aTemp;
        }

        {
            basegfx::B3DHomMatrix aTemp;
            const double fSin(-aVPN.getX());
            const double fCos(fV);
            aTemp.set(2, 2, fCos);
            aTemp.set(0, 0, fCos);
            aTemp.set(0, 2, fSin);
            aTemp.set(2, 0, -fSin);
            aViewTf *= aTemp;
        }

        // X- und Y-Koordinaten des View Up Vektors in das (vorlaeufige)
        // View-Koordinatensytem umrechnen
        fXupVp = aViewTf.get(0, 0) * aVUV.getX() + aViewTf.get(0, 1) * aVUV.getY() + aViewTf.get(0, 2) * aVUV.getZ();
        fYupVp = aViewTf.get(1, 0) * aVUV.getX() + aViewTf.get(1, 1) * aVUV.getY() + aViewTf.get(1, 2) * aVUV.getZ();
        fV = sqrt(fXupVp * fXupVp + fYupVp * fYupVp);

        if ( fV != 0 )
        {
            basegfx::B3DHomMatrix aTemp;
            const double fSin(fXupVp / fV);
            const double fCos(fYupVp / fV);
            aTemp.set(1, 1, fCos);
            aTemp.set(0, 0, fCos);
            aTemp.set(1, 0, fSin);
            aTemp.set(0, 1, -fSin);
            aViewTf *= aTemp;
        }

        bTfValid = sal_True;
    }
}

/*************************************************************************
|*
|* DeviceWindow des Ausgabegeraetes setzen
|*
\************************************************************************/

void Viewport3D::SetDeviceWindow(const Rectangle& rRect)
{
    long nNewW = rRect.GetWidth();
    long nNewH = rRect.GetHeight();
    long nOldW = aDeviceRect.GetWidth();
    long nOldH = aDeviceRect.GetHeight();

    switch ( eAspectMapping )
    {
        double  fRatio, fTmp;

        // Mapping, ohne die reale Groesse der Objekte im Device-Window
        // zu aendern
        case AS_HOLD_SIZE:
            // Wenn Device ungueltig (w, h = -1), zunaechst
            // View mit AsHoldX anpassen
            if ( nOldW > 0 && nOldH > 0 )
            {
                fRatio = (double) nNewW / nOldW;
                aViewWin.X *= fRatio;
                aViewWin.W *= fRatio;
                fRatio = (double) nNewH / nOldH;
                aViewWin.Y *= fRatio;
                aViewWin.H *= fRatio;
                break;
            }
        case AS_HOLD_X:
            // View-Hoehe an -Breite anpassen
            fRatio = (double) nNewH / nNewW;
            fTmp = aViewWin.H;
            aViewWin.H = aViewWin.W * fRatio;
            aViewWin.Y = aViewWin.Y * aViewWin.H / fTmp;
            break;

        case AS_HOLD_Y:
            // View-Breite an -Hoehe anpassen
            fRatio = (double) nNewW / nNewH;
            fTmp = aViewWin.W;
            aViewWin.W = aViewWin.H * fRatio;
            aViewWin.X = aViewWin.X * aViewWin.W / fTmp;
            break;
        default: break;
    }
    fWRatio = nNewW / aViewWin.W;
    fHRatio = nNewH / aViewWin.H;

    aDeviceRect = rRect;
}










/*************************************************************************
|*
|* 3D-Punkt auf Viewplane projizieren
|*
\************************************************************************/

basegfx::B3DPoint Viewport3D::DoProjection(const basegfx::B3DPoint& rVec) const
{
    basegfx::B3DPoint aVec(rVec);

    if ( eProjection == PR_PERSPECTIVE )
    {
        double fPrDist = fVPD - aPRP.getZ();

        if ( aPRP.getZ() == rVec.getZ() )
        {
            aVec.setX(0.0);
            aVec.setY(0.0);
        }
        else
        {
            // Das ist die Version fuer beliebigen PRP, wird aber
            // aus Performancegruenden nicht verwendet
            fPrDist /= aVec.getZ() - aPRP.getZ();
            aVec.setX(aVec.getX() * fPrDist);
            aVec.setY(aVec.getY() * fPrDist);
        }
    }

    return aVec;
}

/*************************************************************************
|*
|* 3D-Punkt auf Geraetekoordinaten mappen
|*
\************************************************************************/

basegfx::B3DPoint Viewport3D::MapToDevice(const basegfx::B3DPoint& rVec) const
{
    basegfx::B3DPoint aRetval;

    // Y-Koordinate subtrahieren, da die Device-Y-Achse von oben
    // nach unten verlaeuft
    aRetval.setX((double)aDeviceRect.Left() + ((rVec.getX() - aViewWin.X) * fWRatio));
    aRetval.setY((double)aDeviceRect.Bottom() - ((rVec.getY() - aViewWin.Y) * fHRatio));
    aRetval.setZ(rVec.getZ());

    return aRetval;
}

/*************************************************************************
|*
|* View Reference Point setzen
|*
\************************************************************************/

void Viewport3D::SetVRP(const basegfx::B3DPoint& rNewVRP)
{
    aVRP = rNewVRP;
    bTfValid = sal_False;
}

/*************************************************************************
|*
|* View Plane Normal setzen
|*
\************************************************************************/

void Viewport3D::SetVPN(const basegfx::B3DVector& rNewVPN)
{
    aVPN = rNewVPN;
    aVPN.normalize();
    bTfValid = sal_False;
}

/*************************************************************************
|*
|* View Up Vector setzen
|*
\************************************************************************/

void Viewport3D::SetVUV(const basegfx::B3DVector& rNewVUV)
{
    aVUV = rNewVUV;
    bTfValid = sal_False;
}

/*************************************************************************
|*
|* Center Of Projection setzen
|*
\************************************************************************/

void Viewport3D::SetPRP(const basegfx::B3DPoint& rNewPRP)
{
    aPRP = rNewPRP;
    aPRP.setX(0.0);
    aPRP.setY(0.0);
    bTfValid = sal_False;
}

/*************************************************************************
|*
|* View Plane Distance setzen
|*
\************************************************************************/

void Viewport3D::SetVPD(double fNewVPD)
{
    fVPD = fNewVPD;
    bTfValid = sal_False;
}

/*************************************************************************
|*
|* Abstand der vorderen Clippingebene setzen
|*
\************************************************************************/

void Viewport3D::SetNearClipDist(double fNewNCD)
{
    fNearClipDist = fNewNCD;
    bTfValid = sal_False;
}

/*************************************************************************
|*
|* Abstand der hinteren Clippingebene setzen
|*
\************************************************************************/

void Viewport3D::SetFarClipDist(double fNewFCD)
{
    fFarClipDist = fNewFCD;
    bTfValid = sal_False;
}

// eof
