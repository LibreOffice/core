/*************************************************************************
 *
 *  $RCSfile: b3dcommn.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:30:10 $
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

#pragma hdrstop

#ifndef _B3D_B3DCOMMN_HXX
#include "b3dcommn.hxx"
#endif

#ifndef _B3D_B3DTRANS_HXX
#include "b3dtrans.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _SV_OUTDEV_HXX
#include <vcl/outdev.hxx>
#endif

/*************************************************************************
|*
|* Bucket fuer Index
|*
\************************************************************************/

BASE3D_IMPL_BUCKET(UINT32, Bucket)

/*************************************************************************
|*
|* Konstruktor Base3DCommon
|*
\************************************************************************/

Base3DCommon::Base3DCommon(OutputDevice* pOutDev)
:   Base3D(pOutDev),
    aBuffers(12)        // 4K
{
}

/*************************************************************************
|*
|* Destruktor Base3DCommon
|*
\************************************************************************/

Base3DCommon::~Base3DCommon()
{
}

/*************************************************************************
|*
|* Start der Szenenbeschreibung:
|*
\************************************************************************/

void Base3DCommon::StartScene()
{
}

/*************************************************************************
|*
|* Ende der Szenenbeschreibung:
|*
\************************************************************************/

void Base3DCommon::EndScene()
{
}

/*************************************************************************
|*
|* Neuen freien Eintrag fuer naechste geometrische Daten liefern
|*
\************************************************************************/

B3dEntity& Base3DCommon::ImplGetFreeEntity()
{
    aBuffers.Append();
    return (aBuffers[aBuffers.Count() - 1]);
}

/*************************************************************************
|*
|* Beleuchtung setzen/lesen
|*
\************************************************************************/

void Base3DCommon::SetLightGroup(B3dLightGroup* pSet, BOOL bSetGlobal)
{
    // call parent
    Base3D::SetLightGroup(pSet, bSetGlobal);

    if(GetLightGroup())
    {
        Matrix4D aOldObjectTrans;
        Matrix4D aEmptyTrans;
        if(GetTransformationSet() && bSetGlobal)
        {
            aOldObjectTrans = GetTransformationSet()->GetObjectTrans();
            GetTransformationSet()->SetObjectTrans(aEmptyTrans);
        }

        for(UINT16 i=0;i<BASE3D_MAX_NUMBER_LIGHTS;i++)
        {
            B3dLight& rLight = GetLightGroup()->
                GetLightObject((Base3DLightNumber)(Base3DLight0 + i));
            if(rLight.IsDirectionalSource())
            {
                Vector3D aDirection = rLight.GetPosition();
                if(GetTransformationSet())
                    aDirection = GetTransformationSet()->InvTransObjectToEye(aDirection);
                aDirection.Normalize();
                rLight.SetPositionEye(aDirection);
            }
            else
            {
                Vector3D aPosition = rLight.GetPosition();
                if(GetTransformationSet())
                    aPosition = GetTransformationSet()->ObjectToEyeCoor(aPosition);
                rLight.SetPositionEye(aPosition);

                Vector3D aDirection = rLight.GetSpotDirection();
                if(GetTransformationSet())
                    aDirection = GetTransformationSet()->InvTransObjectToEye(aDirection);
                aDirection.Normalize();
                rLight.SetSpotDirectionEye(aDirection);
            }
        }

        if(GetTransformationSet() && bSetGlobal)
            GetTransformationSet()->SetObjectTrans(aOldObjectTrans);
    }
}

/*************************************************************************
|*
|* entsprechend der impliziten Topologie auf den neuen Punkt reagieren
|*
\************************************************************************/

void Base3DCommon::ImplPostAddVertex(B3dEntity& rEntity)
{
    B3dTransformationSet* pSet = GetTransformationSet();
    if(pSet)
    {
        // Positionen transformieren in ClippingCoordinates
        rEntity.Point() *= pSet->GetObjectToDevice();

        // Normalen transformieren in EyeCoordinates
        if((GetLightGroup() && GetLightGroup()->IsLightingEnabled()) && rEntity.IsNormalUsed())
        {
            if(GetForceFlat() || GetShadeModel() == Base3DFlat)
                rEntity.Normal() = pSet->InvTransObjectToEye(rEntity.PlaneNormal());
            else
                rEntity.Normal() = pSet->InvTransObjectToEye(rEntity.Normal());
        }

        // Texturkoordinaten transformieren anhand der Texturmatrix
        if(rEntity.IsTexCoorUsed())
        {
            // Multiplikation mittels Point4D, um die Translationen zu
            // beruecksichtigen. Die Koordinaten werden bei GetVector3D()
            // homogenisiert.
            rEntity.TexCoor() = pSet->TransTextureCoor(rEntity.TexCoor());
        }
    }

    // Aktuelle Farbe in diesen B3dEntity eintragen
    rEntity.Color() = GetColor();

    // Jetzt Topologie beachten und evtl. ein Primitiv ausspucken
    UINT32 aCount = aBuffers.Count();
    switch(GetObjectMode())
    {
        case Base3DPoints:
        {
            Create3DPoint(0);
            aBuffers.Erase();
            break;
        }
        case Base3DLines:
        {
            if(aCount == 2)
            {
                Create3DLine(0, 1);
                aBuffers.Erase();
            }
            break;
        }
        case Base3DLineLoop:
        case Base3DLineStrip:
        {
            if(aCount > 1)
                Create3DLine(aCount - 2, aCount - 1);
            break;
        }
        case Base3DTriangles:
        {
            if(aCount == 3)
            {
                Create3DTriangle(0, 1, 2);
                aBuffers.Erase();
            }
            break;
        }
        case Base3DTriangleStrip:
        {
            if(aCount > 2)
            {
                if(aCount % 2)
                    Create3DTriangle(aCount - 3, aCount - 2, aCount - 1);
                else
                    Create3DTriangle(aCount - 2, aCount - 3, aCount - 1);
            }
            break;
        }
        case Base3DTriangleFan:
        {
            if(aCount > 2)
                Create3DTriangle(0, aCount - 2, aCount - 1);
            break;
        }
        case Base3DQuads:
        {
            if(aCount == 4)
            {
                // Spezielle Behandlung, da die inneren Kanten
                // eines Quads NICHT sichtbar sein sollen
                B3dEntity& rEnt2 = aBuffers[2];
                B3dEntity& rEnt0 = aBuffers[0];

                BOOL bZwi = rEnt2.IsEdgeVisible();
                rEnt2.SetEdgeVisible(FALSE);
                Create3DTriangle(0, 1, 2);
                rEnt2.SetEdgeVisible(bZwi);

                bZwi = rEnt0.IsEdgeVisible();
                rEnt0.SetEdgeVisible(FALSE);
                Create3DTriangle(0, 2, 3);
                rEnt0.SetEdgeVisible(bZwi);

                aBuffers.Erase();
            }
            break;
        }
        case Base3DQuadStrip:
        {
            if(aCount > 2)
            {
                B3dEntity& rEnt = aBuffers[aCount - 2];
                BOOL bZwi = rEnt.IsEdgeVisible();
                rEnt.SetEdgeVisible(FALSE);
                if(aCount % 2)
                    Create3DTriangle(aCount - 2, aCount - 1, aCount - 3);
                else
                    Create3DTriangle(aCount - 3, aCount - 1, aCount - 2);
                rEnt.SetEdgeVisible(bZwi);
            }
            break;
        }
        case Base3DPolygon:
        {
            if(aCount > 2)
            {
                B3dEntity& rEnt = aBuffers[aCount - 1];
                BOOL bZwi = rEnt.IsEdgeVisible();
                rEnt.SetEdgeVisible(FALSE);
                Create3DTriangle(0, aCount - 2, aCount - 1);
                rEnt.SetEdgeVisible(bZwi);
                // Ab jetzt nie wieder eine Kante vom 1. Punkt (0)
                // ausgehend generieren
                if(aCount == 3)
                    aBuffers[0].SetEdgeVisible(FALSE);
            }
            break;
        }
    }
}

/*************************************************************************
|*
|* Ein neues Primitiv, leere die jetzigen buffer
|*
\************************************************************************/

void Base3DCommon::ImplStartPrimitive()
{
    // Buffer leeren
    aBuffers.Erase();
}

/*************************************************************************
|*
|* Primitiv abgeschlossen
|*
\************************************************************************/

void Base3DCommon::ImplEndPrimitive()
{
    // Topologie beachten und evtl. ein Primitiv ausspucken
    UINT32 aCount = aBuffers.Count();
    switch(GetObjectMode())
    {
        case Base3DLineLoop:
        {
            if(aCount > 2)
                Create3DLine(aCount - 1, 0);
            break;
        }
        case Base3DPolygon:
        {
            // Letzte schliessende Linie erzeugen, falls
            // es um das Erzeugen von Linien ging und
            // das letzte Primitiv auch wirklich zur Ausgabe
            // gelangt ist
            if(GetRenderMode() == Base3DRenderLine
                && !WasLastPrimitiveRejected())
            {
                if(aCount > 2)
                    Create3DLine(aCount - 1, 0);
            }
            break;
        }
    }
}

/*************************************************************************
|*
|* Funktion fuer Primitiv Punkt
|* Clipping und Ausgabe, falls noch was ueber ist
|*
\************************************************************************/

void Base3DCommon::Create3DPoint(UINT32 nInd)
{
    bLastPrimitiveRejected = TRUE;
    if(GetRenderMode() != Base3DRenderNone)
    {
        // Sicherstellen, dass die Koordinaten in
        // ClipCoordinates vorliegen
        aBuffers[nInd].To3DCoor(GetTransformationSet());

        // Punkt an Einheitswuerfel clippen
        if(Clip3DPoint(nInd))
            Create3DPointClipped(nInd);
    }
}

void Base3DCommon::Create3DPointClipped(UINT32 nInd)
{
    // einige Beleuchtungsdinge koennen hier schon geklaert
    // werden
    if(GetLightGroup() && GetLightGroup()->IsLightingEnabled())
    {
        B3dEntity& rEnt = aBuffers[nInd];
        if(rEnt.IsNormalUsed() && GetLightGroup())
        {
            // Beleuchtungsmodell loesen, Normale loeschen
            SolveColorModel(rEnt.Color(), rEnt.Normal(), rEnt.Point().GetVector3D());
        }
        rEnt.SetNormalUsed(FALSE);
    }

    // Punkt wird dargestellt, weiterreichen
    if(GetPointSize() != 1.0)
    {
        // Punkt als Kreis mit dem Durchmesser GetPointSize() ausgeben
        // Hole den Original-Punkt
        B3dEntity& rEnt1 = aBuffers[nInd];

        // Umrechnen auf DeviceCoor
        rEnt1.ToDeviceCoor(GetTransformationSet());

        // Radius holen
        // Logische Koordinaten nach Pixel
        Point aPnt((long)(GetLineWidth() + 0.5), 0);
        double fRadius = ((double)((
            GetOutputDevice()->PixelToLogic(aPnt).X() -
            GetOutputDevice()->PixelToLogic(Point()).X()) + 0.5)) / 2.0;

        // Bereite neue Punkte vor
        // Hole die neuen Punkte
        UINT32 nNew1 = aBuffers.Count();
        aBuffers.Append(rEnt1);
        B3dEntity& rNew1 = aBuffers[nNew1];

        UINT32 nNew2 = aBuffers.Count();
        aBuffers.Append(rEnt1);
        B3dEntity& rNew2 = aBuffers[nNew2];

        UINT32 nNew3 = aBuffers.Count();
        aBuffers.Append(rEnt1);
        B3dEntity& rNew3 = aBuffers[nNew3];

        // Schleife drehen
        Base3DRenderMode eRenderMode = GetRenderMode();
        SetRenderMode(Base3DRenderFill);
        BOOL bPolyOffset = GetPolygonOffset(Base3DPolygonOffsetFill);
        SetPolygonOffset(Base3DPolygonOffsetFill, TRUE);

        for(double fWink=0.0;fWink<F_2PI-(F_2PI/24.0);fWink+=F_2PI/12.0)
        {
            rNew2.Point().X() = rNew1.Point().X() + (cos(fWink) * fRadius);
            rNew2.Point().Y() = rNew1.Point().Y() + (sin(fWink) * fRadius);

            rNew3.Point().X() = rNew1.Point().X() + (cos(fWink+(F_2PI/12.0)) * fRadius);
            rNew3.Point().Y() = rNew1.Point().Y() + (sin(fWink+(F_2PI/12.0)) * fRadius);

            // Dreieck Zeichnen
            Create3DTriangle(nNew1, nNew3, nNew2);
        }

        SetRenderMode(eRenderMode);
        SetPolygonOffset(Base3DPolygonOffsetFill, bPolyOffset);

        bLastPrimitiveRejected = FALSE;
    }
    else
    {
        Clipped3DPoint(nInd);
        bLastPrimitiveRejected = FALSE;
    }
}

/*************************************************************************
|*
|* Funktion fuer Primitiv Linie
|* Clipping und Ausgabe, falls noch was ueber ist
|*
\************************************************************************/

void Base3DCommon::Create3DLine(UINT32 nInd1, UINT32 nInd2)
{
    bLastPrimitiveRejected = TRUE;
    if(GetRenderMode() != Base3DRenderNone)
    {
        // Sicherstellen, dass die Koordinaten in
        // ClipCoordinates vorliegen
        aBuffers[nInd1].To3DCoor(GetTransformationSet());
        aBuffers[nInd2].To3DCoor(GetTransformationSet());

        if(AreEqual(nInd1, nInd2))
            return;

        // Linie an Einheitswuerfel clippen, dabei kann eine
        // der Indexvariablen (werden als Referenz uegergeben)
        // veraendert werden

        // Alte Buffergroesse merken
        ULONG nCount = aBuffers.Count();

        if(Clip3DLine(nInd1, nInd2))
        {
            // EdgeFlag beachten
            if(aBuffers[nInd1].IsEdgeVisible())
                Create3DLineClipped(nInd1, nInd2);
        }

        // Alte Buffergroesse wiederherstellen, um fortgesetzte
        // Primitive nicht zu zerstoeren
        while(aBuffers.Count() > nCount)
            aBuffers.Remove();
    }
}

void Base3DCommon::Create3DLineClipped(UINT32 nInd1, UINT32 nInd2)
{
    // einige Beleuchtungsdinge koennen hier schon geklaert
    // werden
    if(GetLightGroup() && GetLightGroup()->IsLightingEnabled())
    {
        if(GetShadeModel() == Base3DFlat)
        {
            // Beleuchtuungsmodell fuer gemittelte Normale
            // loesen, Normalen loeschen
            B3dEntity& rEnt1 = aBuffers[nInd1];
            B3dEntity& rEnt2 = aBuffers[nInd2];
            if(rEnt1.IsNormalUsed() && rEnt2.IsNormalUsed() && GetLightGroup())
            {
                Vector3D aNormal = rEnt1.Normal() + rEnt2.Normal();
                aNormal.Normalize();
                Vector3D aPoint = (rEnt1.Point().GetVector3D() + rEnt2.Point().GetVector3D()) / 2.0;
                SolveColorModel(rEnt1.Color(), aNormal, aPoint);
                rEnt2.Color() = rEnt1.Color();
            }
            rEnt1.SetNormalUsed(FALSE);
            rEnt2.SetNormalUsed(FALSE);
        }
    }
    else
    {
        if(GetShadeModel() == Base3DFlat)
        {
            B3dEntity& rEnt1 = aBuffers[nInd1];
            B3dEntity& rEnt2 = aBuffers[nInd2];
            B3dColor aCol;
            aCol.CalcMiddle(rEnt1.Color(), rEnt2.Color());
            rEnt1.Color() = aCol;
            rEnt2.Color() = aCol;
        }
    }

    if(GetRenderMode() == Base3DRenderPoint)
    {
        // Als Punkte ausgeben
        Create3DPointClipped(nInd1);
        Create3DPointClipped(nInd2);
    }
    else
    {
        if(GetLineWidth() != 1.0)
        {
            // Linie als Polygon mit der Breite GetLineWidth() ausgeben
            // Hole die Original-Punkte
            B3dEntity& rEnt1 = aBuffers[nInd1];
            B3dEntity& rEnt2 = aBuffers[nInd2];

            // Umrechnen auf DeviceCoor
            rEnt1.ToDeviceCoor(GetTransformationSet());
            rEnt2.ToDeviceCoor(GetTransformationSet());

            // Bereite neue Punkte vor
            // Hole die neuen Punkte
            UINT32 nNew1 = aBuffers.Count();
            aBuffers.Append(rEnt1);
            B3dEntity& rNew1 = aBuffers[nNew1];

            UINT32 nNew2 = aBuffers.Count();
            aBuffers.Append(rEnt1);
            B3dEntity& rNew2 = aBuffers[nNew2];

            UINT32 nNew3 = aBuffers.Count();
            aBuffers.Append(rEnt2);
            B3dEntity& rNew3 = aBuffers[nNew3];

            UINT32 nNew4 = aBuffers.Count();
            aBuffers.Append(rEnt2);
            B3dEntity& rNew4 = aBuffers[nNew4];

            // Berechnen
            Vector3D aEntVector = rEnt2.Point().GetVector3D() - rEnt1.Point().GetVector3D();
            Vector3D aTurned(-aEntVector.Y(), aEntVector.X(), 0.0);
            aTurned.Normalize();

            // Logische Koordinaten nach Pixel
            Point aPnt((long)(GetLineWidth() + 0.5), 0);
            double fFac = ((double)((
                GetOutputDevice()->PixelToLogic(aPnt).X() -
                GetOutputDevice()->PixelToLogic(Point()).X()) + 0.5)) / 2.0;

            // Aufmuliplizieren
            aTurned *= fFac;

            rNew1.Point().X() += aTurned.X();
            rNew1.Point().Y() += aTurned.Y();
            rNew2.Point().X() -= aTurned.X();
            rNew2.Point().Y() -= aTurned.Y();

            rNew3.Point().X() += aTurned.X();
            rNew3.Point().Y() += aTurned.Y();
            rNew4.Point().X() -= aTurned.X();
            rNew4.Point().Y() -= aTurned.Y();

            // Ausgeben
            Base3DRenderMode eRenderMode = GetRenderMode();
            SetRenderMode(Base3DRenderFill);
            BOOL bPolyOffset = GetPolygonOffset(Base3DPolygonOffsetFill);
            SetPolygonOffset(Base3DPolygonOffsetFill, TRUE);

            Create3DTriangle(nNew2, nNew1, nNew3);
            Create3DTriangle(nNew2, nNew3, nNew4);

            SetRenderMode(eRenderMode);
            SetPolygonOffset(Base3DPolygonOffsetFill, bPolyOffset);

            bLastPrimitiveRejected = FALSE;
        }
        else
        {
            // Linie ausgeben
            Clipped3DLine(nInd1,nInd2);
            bLastPrimitiveRejected = FALSE;
        }
    }
}

/*************************************************************************
|*
|* Funktion fuer Primitiv Dreieck
|* Clipping und Ausgabe, falls noch was ueber ist
|*
\************************************************************************/

void Base3DCommon::Create3DTriangle(UINT32 nInd1, UINT32 nInd2, UINT32 nInd3)
{
    bLastPrimitiveRejected = TRUE;

    // Sicherstellen, dass die Koordinaten in
    // ClipCoordinates vorliegen
    aBuffers[nInd1].To3DCoor(GetTransformationSet());
    aBuffers[nInd2].To3DCoor(GetTransformationSet());
    aBuffers[nInd3].To3DCoor(GetTransformationSet());

    if(AreEqual(nInd1, nInd2)
        || AreEqual(nInd1, nInd3)
        || AreEqual(nInd2, nInd3))
        return;

    // Normale vorbereiten, die echte Ebenennormale der Geometrie
    // im Device-Koordinatensystem. Verzerrungen durch perspektivische
    // Projektion sind somit bereits beruecksichtigt.
    const Vector3D& rPnt1 = aBuffers[nInd1].Point().GetVector3D();
    const Vector3D& rPnt2 = aBuffers[nInd2].Point().GetVector3D();
    const Vector3D& rPnt3 = aBuffers[nInd3].Point().GetVector3D();
    Vector3D aNormal = (rPnt2 - rPnt3)|(rPnt2 - rPnt1);
    aNormal.Normalize();

    if(GetCullMode() != Base3DCullNone)
    {
        // Normale ermitteln, eventuell gar nicht zeichnen
        if(GetCullMode() == Base3DCullFront)
        {
            if(aNormal.Z() > 0.0)
                return;
        }
        else
        {
            if(aNormal.Z() < 0.0)
                return;
        }
    }

    // allgemeines Polygon vorbereiten
    UINT32Bucket aEdgeIndex(8);
    aEdgeIndex.Append(nInd1);
    aEdgeIndex.Append(nInd2);
    aEdgeIndex.Append(nInd3);

    // Alte Buffergroesse merken
    ULONG nCount = aBuffers.Count();

    // Dreieck an Einheitswuerfel clippen
    if(Clip3DPolygon(aEdgeIndex))
    {
        UINT32 nNumPoints = aEdgeIndex.Count();

        // einige Beleuchtungsdinge koennen hier schon geklaert
        // werden
        if(GetLightGroup() && GetLightGroup()->IsLightingEnabled() && nNumPoints != 0)
        {
            if(GetShadeModel() == Base3DFlat)
            {
                B3dEntity& rEnt = aBuffers[nInd1];
                B3dColor aColFlatMode;

                // #63505#
                aNormal.X() = -aNormal.X();
                aNormal.Y() = -aNormal.Y();

                SolveColorModel(aColFlatMode, aNormal, rEnt.Point().GetVector3D());

                // Vorberechnete Farbe in Eckpunkten setzen
                for(UINT32 i=0;i<nNumPoints;i++)
                {
                    B3dEntity& rEnt = aBuffers[aEdgeIndex[i]];
                    rEnt.Color() = aColFlatMode;
                    rEnt.SetNormalUsed(FALSE);
                }
            }
        }
        else
        {
            if(GetShadeModel() == Base3DFlat && nNumPoints != 0)
            {
                UINT16 aRed(0), aGreen(0), aBlue(0), aAlpha(0);
                UINT32 i;
                for(i=0;i<nNumPoints;i++)
                {
                    B3dEntity& rEnt = aBuffers[aEdgeIndex[i]];
                    aRed += (UINT16)rEnt.Color().GetRed();
                    aGreen += (UINT16)rEnt.Color().GetGreen();
                    aBlue += (UINT16)rEnt.Color().GetBlue();
                    aAlpha += (UINT16)rEnt.Color().GetTransparency();
                }
                B3dColor aCol((UINT8)(aAlpha / nNumPoints),
                    (UINT8)(aRed / nNumPoints),
                    (UINT8)(aGreen / nNumPoints),
                    (UINT8)(aBlue / nNumPoints));
                for(i=0;i<nNumPoints;i++)
                {
                    aBuffers[aEdgeIndex[i]].Color() = aCol;
                }
            }
        }

        // Ausgeben, je nach Modus
        Base3DMaterialMode eMat = Base3DMaterialFront;
        if(aNormal.Z() < 0.0
            && (GetLightGroup() && GetLightGroup()->GetModelTwoSide()))
            eMat = Base3DMaterialBack;
        Base3DRenderMode eMode = GetRenderMode(eMat);

        if(eMode == Base3DRenderPoint)
        {
            // Als Punktmenge ausgeben
            for(UINT32 i=0;i<nNumPoints;i++)
            {
                Create3DPointClipped(aEdgeIndex[i]);
            }
        }
        else if(eMode == Base3DRenderLine)
        {
            // Als Linien ausgeben
            UINT32 i2, i3;
            for(UINT32 i=0;i<nNumPoints;i++)
            {
                i2 = i+1;
                if(i2 == nNumPoints)
                    i2 = 0;
                i3 = aEdgeIndex[i];
                i2 = aEdgeIndex[i2];

                // EdgeFlag beachten
                if(aBuffers[i3].IsEdgeVisible())
                    Create3DLineClipped(i3, i2);
            }
        }
        else
        {
            // after clipping the triangle can be a multi-sided, but
            // convex polygon. Render it by dividing it into triangles
            // again. Cause' it is clipped now, no more taking care is
            // necessary. Just give hints which flags are truly edges
            // to allow further computations.
            if(nNumPoints > 2)
            {
                for(UINT32 i=2;i < nNumPoints; i++)
                {
                    Clipped3DTriangle(
                        aEdgeIndex[0],
                        aEdgeIndex[i-1],
                        aEdgeIndex[i]);
                    bLastPrimitiveRejected = FALSE;
                }
            }
        }
    }
    // Alte Buffergroesse wiederherstellen, um fortgesetzte
    // Primitive nicht zu zerstoeren
    while(aBuffers.Count() > nCount)
        aBuffers.Remove();
}

/*************************************************************************
|*
|* Punkt am canonical view volume clippen
|*
\************************************************************************/

BOOL Base3DCommon::Clip3DPoint(UINT32 nInd)
{
    return (!(BOOL)GetClipFlags(nInd));
}

/*************************************************************************
|*
|* Vergleicht, ob die beiden Entities geometrisch gleich sind
|*
\************************************************************************/

BOOL Base3DCommon::AreEqual(UINT32 nInd1, UINT32 nInd2)
{
    const Vector3D& rVec1 = aBuffers[nInd1].Point().GetVector3D();
    const Vector3D& rVec2 = aBuffers[nInd2].Point().GetVector3D();

    if(fabs(rVec1.X() - rVec2.X()) < SMALL_DVALUE)
        if(fabs(rVec1.Y() - rVec2.Y()) < SMALL_DVALUE)
            if(fabs(rVec1.Z() - rVec2.Z()) < SMALL_DVALUE)
                return TRUE;
    return FALSE;
}

/*************************************************************************
|*
|* Linie am canonical view volume clippen
|*
\************************************************************************/

BOOL Base3DCommon::Clip3DLine(UINT32& nInd1,UINT32& nInd2)
{
    UINT16 nFlag0, nFlag1;
    do
    {
        nFlag0 = GetClipFlags(nInd1);
        nFlag1 = GetClipFlags(nInd2);

        // Beide Endpunkte drin?
        if(!(nFlag0 | nFlag1))
            return TRUE;

        // Linie komplett draussen?
        if(nFlag0 & nFlag1)
            return FALSE;

        // Es muss geclippt werden, bereite einen neuen Punkt vor
        UINT32 nNewIndex = aBuffers.Count();
        aBuffers.Append();

        if((nFlag0 | nFlag1) & (CLIPFLAG_FRONT | CLIPFLAG_BACK))
        {
            // clippen in Z
            if(nFlag0 & (CLIPFLAG_FRONT | CLIPFLAG_BACK))
            {
                if(nFlag0 & CLIPFLAG_FRONT)
                    CalcNewPoint(nNewIndex, nInd2, nInd1, 2, -1.0);
                else
                    CalcNewPoint(nNewIndex, nInd1, nInd2, 2, 1.0);
                nInd1 = nNewIndex;
            }
            else
            {
                if(nFlag1 & CLIPFLAG_FRONT)
                    CalcNewPoint(nNewIndex, nInd1, nInd2, 2, -1.0);
                else
                    CalcNewPoint(nNewIndex, nInd2, nInd1, 2, 1.0);
                nInd2 = nNewIndex;
            }
        }
        else if((nFlag0 | nFlag1) & (CLIPFLAG_LEFT | CLIPFLAG_RIGHT))
        {
            // clippen in X
            if(nFlag0 & (CLIPFLAG_LEFT | CLIPFLAG_RIGHT))
            {
                if(nFlag0 & CLIPFLAG_LEFT)
                    CalcNewPoint(nNewIndex, nInd2, nInd1, 0, -1.0);
                else
                    CalcNewPoint(nNewIndex, nInd1, nInd2, 0, 1.0);
                nInd1 = nNewIndex;
            }
            else
            {
                if(nFlag1 & CLIPFLAG_LEFT)
                    CalcNewPoint(nNewIndex, nInd1, nInd2, 0, -1.0);
                else
                    CalcNewPoint(nNewIndex, nInd2, nInd1, 0, 1.0);
                nInd2 = nNewIndex;
            }
        }
        else
        {
            // clippen in Y
            if(nFlag0 & (CLIPFLAG_BOTTOM | CLIPFLAG_TOP))
            {
                if(nFlag0 & CLIPFLAG_BOTTOM)
                    CalcNewPoint(nNewIndex, nInd2, nInd1, 1, -1.0);
                else
                    CalcNewPoint(nNewIndex, nInd1, nInd2, 1, 1.0);
                nInd1 = nNewIndex;
            }
            else
            {
                if(nFlag1 & CLIPFLAG_BOTTOM)
                    CalcNewPoint(nNewIndex, nInd1, nInd2, 1, -1.0);
                else
                    CalcNewPoint(nNewIndex, nInd2, nInd1, 1, 1.0);
                nInd2 = nNewIndex;
            }
        }
    } while(nFlag0 | nFlag1);
    return TRUE;
}

/*************************************************************************
|*
|* ClipFlags eines Punktes ermitteln und zurueckgeben
|*
\************************************************************************/

UINT16 Base3DCommon::GetClipFlags(UINT32 nInd)
{
    UINT16 nRetval(0);
    Point4D& rPoint = aBuffers[nInd].Point();
    rPoint.Homogenize();

    if(rPoint[0] < -(1.0 + SMALL_DVALUE))
        nRetval |= CLIPFLAG_LEFT;
    if(rPoint[0] >  1.0 + SMALL_DVALUE)
        nRetval |= CLIPFLAG_RIGHT;

    if(rPoint[1] < -(1.0 + SMALL_DVALUE))
        nRetval |= CLIPFLAG_BOTTOM;
    if(rPoint[1] >  1.0 + SMALL_DVALUE)
        nRetval |= CLIPFLAG_TOP;

    if(rPoint[2] < -(1.0 + SMALL_DVALUE))
        nRetval |= CLIPFLAG_FRONT;
    if(rPoint[2] >  1.0 + SMALL_DVALUE)
        nRetval |= CLIPFLAG_BACK;

    return nRetval;
}

/*************************************************************************
|*
|* Dreieck am canonical view volume clippen
|* Ergebnis steht indirekt ueber eine Indizierungstabelle in
|* aEdgeIndex[nEdgeDestination][0 .. aEdgeIndexFree[nEdgeDestination]]
|*
\************************************************************************/

BOOL Base3DCommon::Clip3DPolygon(UINT32Bucket& rEdgeIndex)
{
    UINT32 i;
    UINT16 nAllFlagsOr, nAllFlagsAnd;

    do
    {
        // ClipFlags holen
        nAllFlagsOr = 0;
        nAllFlagsAnd = CLIPFLAG_ALL;

        for(i=0; i < rEdgeIndex.Count(); i++)
        {
            UINT16 nFlag = GetClipFlags(rEdgeIndex[i]);
            nAllFlagsOr |= nFlag;
            nAllFlagsAnd &= nFlag;
        }

        // Alle Endpunkte drin?
        if(!nAllFlagsOr)
            return TRUE;

        // Dreieck komplett draussen?
        if(nAllFlagsAnd)
            return FALSE;

        if(nAllFlagsOr & (CLIPFLAG_FRONT|CLIPFLAG_BACK))
        {
            // clippen in Z
            if(nAllFlagsOr & CLIPFLAG_FRONT)
            {
                ClipPoly(rEdgeIndex, 2, TRUE);
            }
            else
            {
                ClipPoly(rEdgeIndex, 2, FALSE);
            }
        }
        else if(nAllFlagsOr & (CLIPFLAG_LEFT|CLIPFLAG_RIGHT))
        {
            // clippen in X
            if(nAllFlagsOr & CLIPFLAG_LEFT)
            {
                ClipPoly(rEdgeIndex, 0, TRUE);
            }
            else
            {
                ClipPoly(rEdgeIndex, 0, FALSE);
            }
        }
        else
        {
            // clippen in Y
            if(nAllFlagsOr & CLIPFLAG_BOTTOM)
            {
                ClipPoly(rEdgeIndex, 1, TRUE);
            }
            else
            {
                ClipPoly(rEdgeIndex, 1, FALSE);
            }
        }
    } while(nAllFlagsOr);
    return TRUE;
}

/*************************************************************************
|*
|* Testen, ob die Entitaet nInd innerhalb des canonical view volume liegt
|*
\************************************************************************/

BOOL Base3DCommon::IsInside(UINT32 nInd, UINT32 nDim, BOOL bLow)
{
    B3dEntity& aEntity = aBuffers[nInd];
    if(bLow)
    {
        if(aEntity.Point()[nDim] < -1.0)
            return FALSE;
    }
    else
    {
        if(aEntity.Point()[nDim] > 1.0)
            return FALSE;
    }
    return TRUE;
}

/*************************************************************************
|*
|* Macht einen Clippingdurchgang eines Polygons in der angegebenen
|* Dimension. Es werden eventuell neue Punkte erzeugt.
|*
\************************************************************************/

void Base3DCommon::ClipPoly(UINT32Bucket& rEdgeIndex, UINT16 nDim, BOOL bLow)
{
    UINT32 nNumEdges = rEdgeIndex.Count();
    UINT32 nCurrentInd = rEdgeIndex[0];
    BOOL bCurrentInside = IsInside(nCurrentInd, nDim, bLow);
    UINT32 nNextInd;
    BOOL bNextInside;
    UINT32Bucket aEdgeIndex(8);

    for(UINT32 i=0;i<nNumEdges;i++)
    {
        // hole naechsten Eckpunkt
        nNextInd = i+1;
        if(nNextInd == nNumEdges)
            nNextInd = 0;
        nNextInd = rEdgeIndex[nNextInd];
        bNextInside = IsInside(nNextInd, nDim, bLow);

        // behandle die Kante nCurrentInd, nNextInd in der Dimension
        // nDim gegen die Grenze -1.0
        if(bCurrentInside)
        {
            // aktuellen Punkt hinzufuegen
            aEdgeIndex.Append(nCurrentInd);
            if(!bNextInside)
            {
                // drin -> draussen
                // Platz fuer Schnittpunkt allokieren
                UINT32 nNewIndex = aBuffers.Count();
                aBuffers.Append();

                // Schnittpunkt berechnen
                if(bLow)
                    CalcNewPoint(nNewIndex, nCurrentInd, nNextInd, nDim, -1.0);
                else
                    CalcNewPoint(nNewIndex, nNextInd, nCurrentInd, nDim, 1.0);

                // EdgeFlag behandeln, beim Verlassen zuruecksetzen
                if(aBuffers[nCurrentInd].IsEdgeVisible())
                    aBuffers[nNewIndex].SetEdgeVisible(FALSE);

                // Schnittpunkt hinzufuegen
                aEdgeIndex.Append(nNewIndex);
            }
        }
        else
        {
            if(bNextInside)
            {
                // draussen -> drin
                // Platz fuer Schnittpunkt allokieren
                UINT32 nNewIndex = aBuffers.Count();
                aBuffers.Append();

                // Schnittpunkt berechnen
                if(bLow)
                    CalcNewPoint(nNewIndex, nNextInd, nCurrentInd, nDim, -1.0);
                else
                    CalcNewPoint(nNewIndex, nCurrentInd, nNextInd, nDim, 1.0);

                // EdgeFlag behandeln, selber Wert wie Vorgaenger
                aBuffers[nNewIndex].SetEdgeVisible(aBuffers[nCurrentInd].IsEdgeVisible());

                // Schnittpunkt hinzufuegen
                aEdgeIndex.Append(nNewIndex);
            }
        }

        // bereite naechste Kante vor
        nCurrentInd = nNextInd;
        bCurrentInside = bNextInside;
    }

    // Indices kopieren
    rEdgeIndex = aEdgeIndex;
}

/*************************************************************************
|*
|* Die Entitaet nNew muss berechnet werden in der angegebenen
|* Dimension an der Grenze fBound.
|*
\************************************************************************/

void Base3DCommon::CalcNewPoint(UINT32 nNew, UINT32 nHigh, UINT32 nLow,
    UINT16 nDim, double fBound)
{
    B3dEntity& aNew = aBuffers[nNew];
    B3dEntity& aHigh = aBuffers[nHigh];
    B3dEntity& aLow = aBuffers[nLow];
    aNew.Reset();

    double fFactor = 1.0;
    if(aLow.Point()[nDim] != aHigh.Point()[nDim])
        fFactor = (fBound - aHigh.Point()[nDim])
            / (aLow.Point()[nDim] - aHigh.Point()[nDim]);

#ifdef DBG_UTIL
    if(fFactor > 1.0 || fFactor < 0.0)
        DBG_ERROR("Wrong clipping factor (out of range)!");
    if(fFactor == 1.0 || fFactor == 0.0)
        DBG_ERROR("Wrong clipping factor (on boundary)!");
#endif

    // Neuen Punkt berechnen, aber Reihenfolge der
    // Punkte aufrecht erhalten um die Sichtbarkeit
    // der Kanten zu retten
    aLow.ForceEqualBase(GetTransformationSet(), aHigh);
    if(fBound < 0.0)
        aNew.CalcInBetween(aLow, aHigh, 1.0 - fFactor);
    else
        aNew.CalcInBetween(aHigh, aLow, fFactor);

    // WICHTIG fuer die Numerik beim Clippen: Die betroffene
    // Koordinate wirklich auf fBound setzen, nicht berechnen.
    // Beim Berechnen koennen nur wieder Ungenauigkeiten auftreten,
    // die bei der Bestimmung der Clipping-Grenzen zu
    // Endlosschleifen fuehren koennen.
    aNew.Point()[nDim] = fBound;
}

/*************************************************************************
|*
|* Beleuchtungsmodell (ColorModel) in einem Punkt loesen
|* Dabei das Ergebnis in rCol ablegen, d.h. wirklich modifizieren
|*
\************************************************************************/

void Base3DCommon::SolveColorModel(B3dColor& rCol, Vector3D& rVec,
    const Vector3D& rPnt)
{
    if(GetLightGroup() && GetLightGroup()->IsLightingEnabled())
    {
        B3dMaterial& rMat = GetMaterialObject();
        BOOL bDoSolve(TRUE);

        // Welches Material? Zeigt der Vektor vom Betrachter
        // weg oder auf diesen?
        if(rVec.Z() < 0.0)
        {
            // Rueckseite, soll diese dargestellt werden?
            if(GetLightGroup() && GetLightGroup()->GetModelTwoSide())
            {
                rMat = GetMaterialObject(Base3DMaterialBack);
            }
            else
            {
                bDoSolve = FALSE;
            }
        }
        if(bDoSolve)
            rCol = SolveColorModel(rMat, rVec, rPnt);
    }
}

B3dColor Base3DCommon::SolveColorModel(B3dMaterial& rMat, Vector3D& rVec, const Vector3D& rPnt)
{
    // Material emission einbeziehen
    B3dColor aNew = rMat.GetMaterial(Base3DMaterialEmission);

    // global ambient light beachten
    aNew += (B3dColor)rMat.GetMaterial(Base3DMaterialAmbient)
        * (B3dColor)GetLightGroup()->GetGlobalAmbientLight();

    if(GetTransformationSet())
    {
        // Die Punktkoordinate liegt in ClipCoordinates vor, wird
        // aber zur Farbberechnung in EyeCoordinates benoetigt.
        // Fuehre eine Ruecktransformation durch.
        Vector3D aPnt(rPnt);
        aPnt = GetTransformationSet()->DeviceToEyeCoor(aPnt);

        // Falls die Normale vom Betrachter weg zeigt und das Beleuchtungs-
        // modell doppelseitig ist, Normale umdrehen
        Vector3D aVec(rVec);
        if(rVec.Z() < 0.0 && GetLightGroup()->GetModelTwoSide())
            aVec = -rVec;

        // Die einzelnen Lichtquellen einbeziehen
        for(UINT16 i=Base3DLight0; i <= Base3DLight7; i++)
        {
            if(GetLightGroup()->IsEnabled((Base3DLightNumber)i))
            {
                aNew += SolveColorModel(
                    GetLightGroup()->GetLightObject((Base3DLightNumber)i),
                    rMat, aVec, aPnt);
            }
        }

        // Transparenz aus der Diffuse (=Objekt-) Farbe uebernehmen
        aNew.SetTransparency(rMat.GetMaterial(Base3DMaterialDiffuse).GetTransparency());
    }
    return aNew;
}

/*************************************************************************
|*
|* Beleuchtungsmodell (ColorModel) fuer eine Lichtquelle loesen
|*
\************************************************************************/

B3dColor Base3DCommon::SolveColorModel(B3dLight& rLight, B3dMaterial& rMat,
    Vector3D& rVec, const Vector3D& rPnt)
{
    B3dColor aRetval(255, 0, 0, 0);
    if(rLight.IsEnabled())
    {
        // Faktor mit Attenuation 1.0 initialisieren, falls
        // IsDirectionalSource() == TRUE
        double fFac = 1.0;
        Vector3D aLightToVertex;
        BOOL bLightToVertex(FALSE);

        if(!rLight.IsDirectionalSource())
        {
            // positional light
            // echten attenuation Faktor ermitteln
            fFac = rLight.GetConstantAttenuation();
            if(rLight.IsLinearOrQuadratic())
            {
                // jetzt wird die Entfernung zwischen Lichtposition
                // und Punkt benoetigt
                aLightToVertex = rPnt - rLight.GetPositionEye();
                bLightToVertex = TRUE;
                double fLen = aLightToVertex.GetLength();
                aLightToVertex.Normalize();

                fFac += rLight.GetLinearAttenuation() * fLen;
                fFac += rLight.GetQuadraticAttenuation() * fLen * fLen;
            }
            // Kehrwert bilden
            if(fFac != 1.0 && fFac != 0.0)
                fFac = 1.0 / fFac;

            if(rLight.IsSpot())
            {
                // Spotlight it immer ein positional light
                // Spotlight effect, wird auf den Faktor multipliziert
                if(!bLightToVertex)
                {
                    aLightToVertex = rPnt - rLight.GetPositionEye();
                    aLightToVertex.Normalize();
                    bLightToVertex = TRUE;
                }
                double fCosAngle = aLightToVertex.Scalar(rLight.GetSpotDirection());
                // innerhalb des konus?
                if(fCosAngle <= rLight.GetCosSpotCutoff())
                {
                    if(fCosAngle > 0.000001)
                    {
                        if(rLight.GetSpotExponent() != 0.0)
                            fCosAngle = pow(fCosAngle, rLight.GetSpotExponent());
                        fFac *= fCosAngle;
                    }
                    else
                    {
                        fFac = 0.0;
                    }
                }
                else
                {
                    // Ausserhalb des Konus, keine Beleuchtung
                    fFac = 0.0;
                }
            }
        }

        // falls es etwas zu beleuchten gibt...
        if(fFac != 0.0)
        {
            // Ambient term
            if(rLight.IsAmbient())
            {
                aRetval += (B3dColor)rLight.GetIntensity(Base3DMaterialAmbient)
                    * (B3dColor)rMat.GetMaterial(Base3DMaterialAmbient);
            }

            if(rLight.IsDiffuse() || rLight.IsSpecular())
            {
                if(bLightToVertex)
                {
                    // Falls hier schon berechnet, handelt es sich
                    // auch um ein positional light
                    aLightToVertex = -aLightToVertex;
                }
                else
                {
                    if(rLight.IsDirectionalSource())
                    {
                        // Vektor direkt nehmen
                        aLightToVertex = rLight.GetPosition();
                    }
                    else
                    {
                        // Umgerechnete Lichtposition nehmen
                        aLightToVertex = rLight.GetPositionEye();
                        // Betrachtete Position abziehen -> Einheitsvektor
                        // vom Punkt zur Lichtquelle
                        aLightToVertex -= rPnt;
                    }
                    aLightToVertex.Normalize();
                }
                double fCosFac = aLightToVertex.Scalar(rVec);

                if(fCosFac > 0.000001)
                {
                    if(rLight.IsDiffuse())
                    {
                        // Diffuse term
                        aRetval += (B3dColor)rLight.GetIntensity(Base3DMaterialDiffuse)
                            * (B3dColor)rMat.GetMaterial(Base3DMaterialDiffuse)
                            * fCosFac;
                    }
                    if(rLight.IsSpecular())
                    {
                        // Specular term
                        if(GetLightGroup()->GetLocalViewer())
                        {
                            // use vector 0,0,1
                            aLightToVertex.Z() += 1.0;
                        }
                        else
                        {
                            // vector Vertex to Viewpoint berechnen in
                            // Augkoordinaten, ist 0 - rPnt
                            aLightToVertex -= rPnt;
                        }
                        aLightToVertex.Normalize();
                        fCosFac = aLightToVertex.Scalar(rVec);
                        if(fCosFac > 0.000001)
                        {
                            if(rMat.GetShininess())
                                fCosFac = pow(fCosFac, rMat.GetShininess());
                            aRetval += (B3dColor)rLight.GetIntensity(Base3DMaterialSpecular)
                                * (B3dColor)rMat.GetMaterial(Base3DMaterialSpecular)
                                * fCosFac;
                        }
                    }
                }
            }

            // jetzt fFac aufrechnen
            if(fFac != 1.0)
                aRetval *= fFac;
        }
    }
    return aRetval;
}


