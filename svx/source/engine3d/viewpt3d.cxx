/*************************************************************************
 *
 *  $RCSfile: viewpt3d.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:15 $
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

#ifndef _VIEWPT3D_HXX
#include "viewpt3d.hxx"
#endif

#ifndef _VOLUME3D_HXX
#include "volume3d.hxx"
#endif

#ifndef _SVDIO_HXX
#include "svdio.hxx"
#endif

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
    aViewPoint (0, 0, 5000),
    fNearClipDist (0.0),
    fFarClipDist (0.0),
    fWRatio (1.0),
    fHRatio (1.0),
    eProjection(PR_PERSPECTIVE),
    eAspectMapping(AS_NO_MAPPING),
    bTfValid(0),
    // DeviceRect-Groesse < 0 -> ungueltig
    aDeviceRect(Point(0,0), Size(-1,-1))
{
    aViewWin.X = -1; aViewWin.Y = -1;
    aViewWin.W =  2; aViewWin.H = 2;
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
        aViewPoint = aVRP + aVPN * aPRP.Z();

        // auf Einheitsmatrix zuruecksetzen
        aViewTf.Identity();

        // in den Ursprung verschieben
        aViewTf.Translate(-aVRP);

        // fV = Laenge der Projektion von aVPN auf die yz-Ebene:
        fV = aVPN.GetYZLength();

        if ( fV != 0 )
            aViewTf.RotateX(aVPN.Y() / fV, aVPN.Z() / fV);

        aViewTf.RotateY(- aVPN.X(), fV);
        // X- und Y-Koordinaten des View Up Vektors in das (vorlaeufige)
        // View-Koordinatensytem umrechnen
        fXupVp = aViewTf[0][0] * aVUV[0] + aViewTf[0][1] * aVUV[1] + aViewTf[0][2] * aVUV[2];
        fYupVp = aViewTf[1][0] * aVUV[0] + aViewTf[1][1] * aVUV[1] + aViewTf[1][2] * aVUV[2];
        fV = sqrt(fXupVp * fXupVp + fYupVp * fYupVp);
        if ( fV != 0 )
            aViewTf.RotateZ(fXupVp / fV, fYupVp / fV);

        aViewTf = Matrix4D(aViewTf);
        bTfValid = TRUE;
    }
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
|* View-Window genau um das mit rTransform transformierte Volumen legen
|*
\************************************************************************/

void Viewport3D::FitViewToVolume(const Volume3D& rVolume, Matrix4D aTransform)
{
    Vector3D aTfVec;
    Volume3D aFitVol;

    aTransform *= GetViewTransform();
    Vol3DPointIterator aIter(rVolume, &aTransform);

    while ( aIter.Next(aTfVec) )
    {
        DoProjection(aTfVec);
        aFitVol.Union(aTfVec);
    }
    SetViewWindow(aFitVol.MinVec().X(), aFitVol.MinVec().Y(),
                  aFitVol.GetWidth(), aFitVol.GetHeight());
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
    }
    fWRatio = nNewW / aViewWin.W;
    fHRatio = nNewH / aViewWin.H;

    aDeviceRect = rRect;
}

/*************************************************************************
|*
|* Beobachterposition (PRP) in Weltkoordinaten zurueckgeben
|*
\************************************************************************/

const Vector3D& Viewport3D::GetViewPoint()
{
    MakeTransform();

    return aViewPoint;
}

/*************************************************************************
|*
|* Transformationsmatrix zurueckgeben
|*
\************************************************************************/

const Matrix4D& Viewport3D::GetViewTransform()
{
    MakeTransform();

    return aViewTf;
}

/*************************************************************************
|*
|* uebergebenen Punkt transformieren
|*
\************************************************************************/

Vector3D& Viewport3D::Transform(Vector3D& rVec)
{
    MakeTransform();

    rVec *= aViewTf;

    return rVec;
}

/*************************************************************************
|*
|* neuen Punkt transformiert zurueckgeben
|*
\************************************************************************/

Vector3D Viewport3D::Transformed(const Vector3D& rVec)
{
    Vector3D aNewVec = rVec;
    Transform(aNewVec);

    return aNewVec;
}

/*************************************************************************
|*
|* 3D-Punkt auf Viewplane projizieren
|*
\************************************************************************/

Vector3D& Viewport3D::DoProjection(Vector3D& rVec) const
{
    if ( eProjection == PR_PERSPECTIVE )
    {
        double fPrDist = fVPD - aPRP.Z();

        if ( aPRP.Z() == rVec.Z() )
            rVec.X() = rVec.Y() = 0;
        else
        {
            // Das ist die Version fuer beliebigen PRP, wird aber
            // aus Performancegruenden nicht verwendet
/*          double fZDiv = rVec.Z() / fPrDist;
            double fZSub = 1 - fZDiv;
            rVec.X() = (rVec.X() - aPRP.X() * fZDiv) / fZSub;
            rVec.Y() = (rVec.Y() - aPRP.Y() * fZDiv) / fZSub;
*/
            fPrDist /= rVec.Z() - aPRP.Z();
            rVec.X() *= fPrDist;
            rVec.Y() *= fPrDist;
        }
    }
    return rVec;
}

/*************************************************************************
|*
|* 3D-Punkt auf Geraetekoordinaten mappen
|*
\************************************************************************/

Vector3D Viewport3D::MapToDevice(const Vector3D& rVec) const
{
    Vector3D aRetval;

    // Y-Koordinate subtrahieren, da die Device-Y-Achse von oben
    // nach unten verlaeuft
    aRetval.X() = (double)aDeviceRect.Left() + ((rVec.X() - aViewWin.X) * fWRatio);
    aRetval.Y() = (double)aDeviceRect.Bottom() - ((rVec.Y() - aViewWin.Y) * fHRatio);
    aRetval.Z() = rVec.Z();

    return aRetval;
}

/*************************************************************************
|*
|* 3D-Punkt auf Viewplane projizieren und auf Geraetekoordinaten mappen
|*
\************************************************************************/

Vector3D Viewport3D::ProjectAndMap(Vector3D aVec) const
{
    DoProjection(aVec);
    return MapToDevice(aVec);
}

/*************************************************************************
|*
|* Device-Werte fuer Breite und Hoehe des Vektors in View-Werte
|* wandeln, abhaengig von der Projektion
|*
\************************************************************************/

Vector3D Viewport3D::MapSizeToView(const Vector3D& rVec)
{
    Vector3D aVec(rVec);
    // zuerst von Device in View umrechnen
    aVec.X() /= fWRatio;
    aVec.Y() /= fHRatio;

    // dann die Perspektivtransformation umkehren
/*  if ( eProjection == PR_PERSPECTIVE )
    {
        double fPrDist = fVPD - aPRP.Z();

        if ( aPRP.Z() != aVec.Z() )
        {
            fPrDist /= aVec.Z() - aPRP.Z();
            aVec.X() /= fPrDist;
            aVec.Y() /= fPrDist;
        }
    }
*/  return aVec;
}

/*************************************************************************
|*
|* View Reference Point setzen
|*
\************************************************************************/

void Viewport3D::SetVRP(const Vector3D& rNewVRP)
{
    aVRP = rNewVRP;
    bTfValid = FALSE;
}

/*************************************************************************
|*
|* View Plane Normal setzen
|*
\************************************************************************/

void Viewport3D::SetVPN(const Vector3D& rNewVPN)
{
    aVPN = rNewVPN;
    aVPN.Normalize();
    bTfValid = FALSE;
}

/*************************************************************************
|*
|* View Up Vector setzen
|*
\************************************************************************/

void Viewport3D::SetVUV(const Vector3D& rNewVUV)
{
    aVUV = rNewVUV;
    bTfValid = FALSE;
}

/*************************************************************************
|*
|* Center Of Projection setzen
|*
\************************************************************************/

void Viewport3D::SetPRP(const Vector3D& rNewPRP)
{
    aPRP = rNewPRP;
    aPRP.X() = 0;
    aPRP.Y() = 0;
    bTfValid = FALSE;
}

/*************************************************************************
|*
|* View Plane Distance setzen
|*
\************************************************************************/

void Viewport3D::SetVPD(double fNewVPD)
{
    fVPD = fNewVPD;
    bTfValid = FALSE;
}

/*************************************************************************
|*
|* Abstand der vorderen Clippingebene setzen
|*
\************************************************************************/

void Viewport3D::SetNearClipDist(double fNewNCD)
{
    fNearClipDist = fNewNCD;
    bTfValid = FALSE;
}

/*************************************************************************
|*
|* Abstand der hinteren Clippingebene setzen
|*
\************************************************************************/

void Viewport3D::SetFarClipDist(double fNewFCD)
{
    fFarClipDist = fNewFCD;
    bTfValid = FALSE;
}

/*************************************************************************
|*
|* Stream-Out-Operator fuer Viewport3D (Version 3.1)
|*
\************************************************************************/

void Viewport3D::WriteData31(SvStream& rOut) const
{
    rOut << aVRP;
    rOut << aVPN;
    rOut << aVUV;
    rOut << aPRP;
    rOut << fVPD;
    rOut << fNearClipDist;
    rOut << fFarClipDist;
    rOut << UINT16(eProjection);
    rOut << UINT16(eAspectMapping);
    rOut << aDeviceRect;
    rOut << aViewWin.X;
    rOut << aViewWin.Y;
    rOut << aViewWin.W;
    rOut << aViewWin.H;
}

/*************************************************************************
|*
|* Stream-Out-Operator fuer Viewport3D ab File-Revision 13
|* implementiert zum Hauptupdate 355 am 4.2.97 FG (freigegeben ab 356)
|*
\************************************************************************/

void Viewport3D::WriteData(SvStream& rOut) const
{
    if (rOut.GetVersion() < 3560)
    {
        Viewport3D::WriteData31(rOut);
        return;
    }
    SdrDownCompat aCompat(rOut, STREAM_WRITE);
#ifdef DBG_UTIL
    aCompat.SetID("Viewport3D");
#endif

    rOut << aVRP;
    rOut << aVPN;
    rOut << aVUV;
    rOut << aPRP;
    rOut << fVPD;
    rOut << fNearClipDist;
    rOut << fFarClipDist;
    rOut << UINT16(eProjection);
    rOut << UINT16(eAspectMapping);
    rOut << aDeviceRect;
    rOut << aViewWin.X;
    rOut << aViewWin.Y;
    rOut << aViewWin.W;
    rOut << aViewWin.H;
}


/*************************************************************************
|*
|* Stream-In-Operator fuer Viewport3D fuer die Version 3.1
|*
\************************************************************************/

void Viewport3D::ReadData31(SvStream& rIn)
{
    UINT16  nTmp16;

    rIn >> aVRP;
    rIn >> aVPN;
    rIn >> aVUV;
    rIn >> aPRP;
    rIn >> fVPD;
    rIn >> fNearClipDist;
    rIn >> fFarClipDist;
    rIn >> nTmp16; eProjection = ProjectionType(nTmp16);
    rIn >> nTmp16; eAspectMapping = AspectMapType(nTmp16);
    rIn >> aDeviceRect;
    rIn >> aViewWin.X;
    rIn >> aViewWin.Y;
    rIn >> aViewWin.W;
    rIn >> aViewWin.H;

    // es gibt einige Docs mit zweifelhaften werten, daher wird die Dist auf 0 gesetzt
    if ((fNearClipDist <= 1e-100) || (fNearClipDist >= 1e100)) fNearClipDist = 0.0;
    if ((fFarClipDist <= 1e-100) || (fFarClipDist >= 1e100)) fFarClipDist = 0.0;

    fWRatio = aDeviceRect.GetWidth() / aViewWin.W;
    fHRatio = aDeviceRect.GetHeight() / aViewWin.H;

    bTfValid = FALSE;
}

/*************************************************************************
|*
|* ReadData-Methode fuer Viewport3D, gueltig fuer Filerevisionen ab 13
|* eingecheckt ab Revision 355 am 4.2.97 (FG) (freigegeben ab 356)
|*
\************************************************************************/

void Viewport3D::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
{
    if ((rHead.GetVersion() < 13) || (rIn.GetVersion() < 3560))
    {
        Viewport3D::ReadData31 (rIn);
        return;
    }
    SdrDownCompat aCompat(rIn, STREAM_READ);
#ifdef DBG_UTIL
    aCompat.SetID("Viewport3D");
#endif

    UINT16  nTmp16;

    rIn >> aVRP;
    rIn >> aVPN;
    rIn >> aVUV;
    rIn >> aPRP;
    rIn >> fVPD;
    rIn >> fNearClipDist;
    rIn >> fFarClipDist;
    rIn >> nTmp16; eProjection = ProjectionType(nTmp16);
    rIn >> nTmp16; eAspectMapping = AspectMapType(nTmp16);
    rIn >> aDeviceRect;
    rIn >> aViewWin.X;
    rIn >> aViewWin.Y;
    rIn >> aViewWin.W;
    rIn >> aViewWin.H;

    // es gibt einige Docs mit zweifelhaften werten, daher wird die Dist auf 0 gesetzt
    if ((fNearClipDist <= 1e-100) || (fNearClipDist >= 1e100)) fNearClipDist = 0.0;
    if ((fFarClipDist <= 1e-100) || (fFarClipDist >= 1e100)) fFarClipDist = 0.0;

    fWRatio = aDeviceRect.GetWidth() / aViewWin.W;
    fHRatio = aDeviceRect.GetHeight() / aViewWin.H;

    bTfValid = FALSE;
}


// FG: eigentlich sollten die Stream-Funktionen in der 3D-Engine nicht verwendet werden
//     sondern WriteData und ReadData, da man insbesondere bei ReadData einen
//     zusaetzlichen Parameter hat der die File-Revision enthaelt.

/*************************************************************************
|*
|* Stream-Out-Operator fuer Viewport3D
|*
\************************************************************************/

SvStream& operator<<(SvStream& rOStream, const Viewport3D& rViewpt)
{
    rViewpt.WriteData31(rOStream);
    return rOStream;
}

/*************************************************************************
|*
|* Stream-In-Operator fuer Viewport3D
|*
\************************************************************************/

SvStream& operator>>(SvStream& rIStream, Viewport3D& rViewpt)
{
    rViewpt.ReadData31(rIStream);
    return rIStream;
}


