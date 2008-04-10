/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: b3dcommn.cxx,v $
 * $Revision: 1.11 $
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
#include "precompiled_goodies.hxx"
#ifndef GCC
#endif
#include "b3dcommn.hxx"
#include "b3dtrans.hxx"
#include <tools/debug.hxx>
#include <vcl/outdev.hxx>

/*************************************************************************
|*
|* Bucket fuer Index
|*
\************************************************************************/

BASE3D_IMPL_BUCKET(sal_uInt32, Bucket)

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

void Base3DCommon::SetLightGroup(B3dLightGroup* pSet, sal_Bool bSetGlobal)
{
    // call parent
    Base3D::SetLightGroup(pSet, bSetGlobal);

    if(GetLightGroup())
    {
        basegfx::B3DHomMatrix aOldObjectTrans;
        basegfx::B3DHomMatrix aEmptyTrans;

        if(GetTransformationSet() && bSetGlobal)
        {
            aOldObjectTrans = GetTransformationSet()->GetObjectTrans();
            GetTransformationSet()->SetObjectTrans(aEmptyTrans);
        }

        for(sal_uInt16 i=0;i<BASE3D_MAX_NUMBER_LIGHTS;i++)
        {
            B3dLight& rLight = GetLightGroup()->GetLightObject((Base3DLightNumber)(Base3DLight0 + i));

            if(rLight.IsDirectionalSource())
            {
                basegfx::B3DPoint aPosition(rLight.GetPosition());

                if(GetTransformationSet())
                {
                    aPosition = GetTransformationSet()->InvTransObjectToEye(aPosition);
                }

                rLight.SetPositionEye(aPosition);
            }
            else
            {
                basegfx::B3DPoint aPosition(rLight.GetPosition());

                if(GetTransformationSet())
                {
                    aPosition = GetTransformationSet()->ObjectToEyeCoor(aPosition);
                }

                rLight.SetPositionEye(aPosition);

                basegfx::B3DVector aDirection(rLight.GetSpotDirection());

                if(GetTransformationSet())
                {
                    aDirection = GetTransformationSet()->InvTransObjectToEye(aDirection);
                }

                aDirection.normalize();
                rLight.SetSpotDirectionEye(aDirection);
            }
        }

        if(GetTransformationSet() && bSetGlobal)
        {
            GetTransformationSet()->SetObjectTrans(aOldObjectTrans);
        }
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
            {
                rEntity.Normal() = pSet->InvTransObjectToEye(rEntity.PlaneNormal());
            }
            else
            {
                rEntity.Normal() = pSet->InvTransObjectToEye(rEntity.Normal());
            }
        }

        // Texturkoordinaten transformieren anhand der Texturmatrix
        if(rEntity.IsTexCoorUsed())
        {
            rEntity.TexCoor() = pSet->TransTextureCoor(rEntity.TexCoor());
        }
    }

    // Aktuelle Farbe in diesen B3dEntity eintragen
    rEntity.Color() = GetColor();

    // Jetzt Topologie beachten und evtl. ein Primitiv ausspucken
    sal_uInt32 aCount = aBuffers.Count();
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

                sal_Bool bZwi = rEnt2.IsEdgeVisible();
                rEnt2.SetEdgeVisible(sal_False);
                Create3DTriangle(0, 1, 2);
                rEnt2.SetEdgeVisible(bZwi);

                bZwi = rEnt0.IsEdgeVisible();
                rEnt0.SetEdgeVisible(sal_False);
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
                sal_Bool bZwi = rEnt.IsEdgeVisible();
                rEnt.SetEdgeVisible(sal_False);
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
                sal_Bool bZwi = rEnt.IsEdgeVisible();
                rEnt.SetEdgeVisible(sal_False);
                Create3DTriangle(0, aCount - 2, aCount - 1);
                rEnt.SetEdgeVisible(bZwi);
                // Ab jetzt nie wieder eine Kante vom 1. Punkt (0)
                // ausgehend generieren
                if(aCount == 3)
                    aBuffers[0].SetEdgeVisible(sal_False);
            }
            break;

        }
        case Base3DComplexPolygon:
        case Base3DComplexPolygonCut:
            break;  // -Wall not handled.
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
    sal_uInt32 aCount = aBuffers.Count();
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
        default:
            break;  // -Wall multiple values not handled.
    }
}

/*************************************************************************
|*
|* Funktion fuer Primitiv Punkt
|* Clipping und Ausgabe, falls noch was ueber ist
|*
\************************************************************************/

void Base3DCommon::Create3DPoint(sal_uInt32 nInd)
{
    bLastPrimitiveRejected = sal_True;
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

void Base3DCommon::Create3DPointClipped(sal_uInt32 nInd)
{
    // einige Beleuchtungsdinge koennen hier schon geklaert
    // werden
    if(GetLightGroup() && GetLightGroup()->IsLightingEnabled())
    {
        B3dEntity& rEnt = aBuffers[nInd];
        if(rEnt.IsNormalUsed() && GetLightGroup())
        {
            // Beleuchtungsmodell loesen, Normale loeschen
            SolveColorModel(rEnt.Color(), rEnt.Normal(), rEnt.Point());
        }
        rEnt.SetNormalUsed(sal_False);
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
        sal_uInt32 nNew1 = aBuffers.Count();
        aBuffers.Append(rEnt1);
        B3dEntity& rNew1 = aBuffers[nNew1];

        sal_uInt32 nNew2 = aBuffers.Count();
        aBuffers.Append(rEnt1);
        B3dEntity& rNew2 = aBuffers[nNew2];

        sal_uInt32 nNew3 = aBuffers.Count();
        aBuffers.Append(rEnt1);
        B3dEntity& rNew3 = aBuffers[nNew3];

        // Schleife drehen
        Base3DRenderMode eRenderMode = GetRenderMode();
        SetRenderMode(Base3DRenderFill);
        sal_Bool bPolyOffset = GetPolygonOffset(Base3DPolygonOffsetFill);
        SetPolygonOffset(Base3DPolygonOffsetFill, sal_True);

        for(double fWink=0.0;fWink<F_2PI-(F_2PI/24.0);fWink+=F_2PI/12.0)
        {
            rNew2.Point().setX(rNew1.Point().getX() + (cos(fWink) * fRadius));
            rNew2.Point().setY(rNew1.Point().getY() + (sin(fWink) * fRadius));

            rNew3.Point().setX(rNew1.Point().getX() + (cos(fWink+(F_2PI/12.0)) * fRadius));
            rNew3.Point().setY(rNew1.Point().getY() + (sin(fWink+(F_2PI/12.0)) * fRadius));

            // Dreieck Zeichnen
            Create3DTriangle(nNew1, nNew3, nNew2);
        }

        SetRenderMode(eRenderMode);
        SetPolygonOffset(Base3DPolygonOffsetFill, bPolyOffset);

        bLastPrimitiveRejected = sal_False;
    }
    else
    {
        Clipped3DPoint(nInd);
        bLastPrimitiveRejected = sal_False;
    }
}

/*************************************************************************
|*
|* Funktion fuer Primitiv Linie
|* Clipping und Ausgabe, falls noch was ueber ist
|*
\************************************************************************/

void Base3DCommon::Create3DLine(sal_uInt32 nInd1, sal_uInt32 nInd2)
{
    bLastPrimitiveRejected = sal_True;
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

void Base3DCommon::Create3DLineClipped(sal_uInt32 nInd1, sal_uInt32 nInd2)
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
                basegfx::B3DVector aNormal = rEnt1.Normal() + rEnt2.Normal();
                aNormal.normalize();
                basegfx::B3DPoint aPoint = (rEnt1.Point() + rEnt2.Point()) / 2.0;
                SolveColorModel(rEnt1.Color(), aNormal, aPoint);
                rEnt2.Color() = rEnt1.Color();
            }
            rEnt1.SetNormalUsed(sal_False);
            rEnt2.SetNormalUsed(sal_False);
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
            sal_uInt32 nNew1 = aBuffers.Count();
            aBuffers.Append(rEnt1);
            B3dEntity& rNew1 = aBuffers[nNew1];

            sal_uInt32 nNew2 = aBuffers.Count();
            aBuffers.Append(rEnt1);
            B3dEntity& rNew2 = aBuffers[nNew2];

            sal_uInt32 nNew3 = aBuffers.Count();
            aBuffers.Append(rEnt2);
            B3dEntity& rNew3 = aBuffers[nNew3];

            sal_uInt32 nNew4 = aBuffers.Count();
            aBuffers.Append(rEnt2);
            B3dEntity& rNew4 = aBuffers[nNew4];

            // Berechnen
            basegfx::B3DVector aEntVector = rEnt2.Point() - rEnt1.Point();
            basegfx::B3DVector aTurned(-aEntVector.getY(), aEntVector.getX(), 0.0);
            aTurned.normalize();

            // Logische Koordinaten nach Pixel
            Point aPnt((long)(GetLineWidth() + 0.5), 0);
            double fFac = ((double)((
                GetOutputDevice()->PixelToLogic(aPnt).X() -
                GetOutputDevice()->PixelToLogic(Point()).X()) + 0.5)) / 2.0;

            // Aufmuliplizieren
            aTurned *= fFac;

            rNew1.Point().setX(rNew1.Point().getX() + aTurned.getX());
            rNew1.Point().setY(rNew1.Point().getY() + aTurned.getY());
            rNew2.Point().setX(rNew2.Point().getX() - aTurned.getX());
            rNew2.Point().setY(rNew2.Point().getY() - aTurned.getY());

            rNew3.Point().setX(rNew3.Point().getX() + aTurned.getX());
            rNew3.Point().setY(rNew3.Point().getY() + aTurned.getY());
            rNew4.Point().setX(rNew4.Point().getX() - aTurned.getX());
            rNew4.Point().setY(rNew4.Point().getY() - aTurned.getY());

            // Ausgeben
            Base3DRenderMode eRenderMode = GetRenderMode();
            SetRenderMode(Base3DRenderFill);
            sal_Bool bPolyOffset = GetPolygonOffset(Base3DPolygonOffsetFill);
            SetPolygonOffset(Base3DPolygonOffsetFill, sal_True);

            Create3DTriangle(nNew2, nNew1, nNew3);
            Create3DTriangle(nNew2, nNew3, nNew4);

            SetRenderMode(eRenderMode);
            SetPolygonOffset(Base3DPolygonOffsetFill, bPolyOffset);

            bLastPrimitiveRejected = sal_False;
        }
        else
        {
            // Linie ausgeben
            Clipped3DLine(nInd1,nInd2);
            bLastPrimitiveRejected = sal_False;
        }
    }
}

/*************************************************************************
|*
|* Funktion fuer Primitiv Dreieck
|* Clipping und Ausgabe, falls noch was ueber ist
|*
\************************************************************************/

void Base3DCommon::Create3DTriangle(sal_uInt32 nInd1, sal_uInt32 nInd2, sal_uInt32 nInd3)
{
    bLastPrimitiveRejected = sal_True;

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
    const basegfx::B3DPoint& rPnt1 = aBuffers[nInd1].Point();
    const basegfx::B3DPoint& rPnt2 = aBuffers[nInd2].Point();
    const basegfx::B3DPoint& rPnt3 = aBuffers[nInd3].Point();
    basegfx::B3DVector aNormal = basegfx::B3DVector(rPnt2 - rPnt3).getPerpendicular(basegfx::B3DVector(rPnt2 - rPnt1));
    aNormal.normalize();

    if(GetCullMode() != Base3DCullNone)
    {
        // Normale ermitteln, eventuell gar nicht zeichnen
        if(GetCullMode() == Base3DCullFront)
        {
            if(aNormal.getZ() > 0.0)
                return;
        }
        else
        {
            if(aNormal.getZ() < 0.0)
                return;
        }
    }

    // allgemeines Polygon vorbereiten
    sal_uInt32Bucket aEdgeIndex(8);
    aEdgeIndex.Append(nInd1);
    aEdgeIndex.Append(nInd2);
    aEdgeIndex.Append(nInd3);

    // Alte Buffergroesse merken
    ULONG nCount = aBuffers.Count();

    // Dreieck an Einheitswuerfel clippen
    if(Clip3DPolygon(aEdgeIndex))
    {
        // #93184# set flag for polygon normal direction
        bNormalPointsAway = sal_Bool(aNormal.getZ() < 0.0);

        sal_uInt32 nNumPoints = aEdgeIndex.Count();

        // einige Beleuchtungsdinge koennen hier schon geklaert
        // werden
        if(GetLightGroup() && GetLightGroup()->IsLightingEnabled() && nNumPoints != 0)
        {
            if(GetShadeModel() == Base3DFlat)
            {
                B3dEntity& rEntity = aBuffers[nInd1];
                B3dColor aColFlatMode;

                // #63505#
                aNormal.setX(-aNormal.getX());
                aNormal.setY(-aNormal.getY());

                SolveColorModel(aColFlatMode, aNormal, rEntity.Point());

                // Vorberechnete Farbe in Eckpunkten setzen
                for(sal_uInt32 i=0;i<nNumPoints;i++)
                {
                    B3dEntity& rEnt = aBuffers[aEdgeIndex[i]];
                    rEnt.Color() = aColFlatMode;
                    rEnt.SetNormalUsed(sal_False);
                }
            }
        }
        else
        {
            if(GetShadeModel() == Base3DFlat && nNumPoints != 0)
            {
                sal_uInt16 aRed(0), aGreen(0), aBlue(0), aAlpha(0);
                sal_uInt32 i;
                for(i=0;i<nNumPoints;i++)
                {
                    B3dEntity& rEnt = aBuffers[aEdgeIndex[i]];
                    aRed = aRed + rEnt.Color().GetRed();
                    aGreen = aGreen + rEnt.Color().GetGreen();
                    aBlue = aBlue + rEnt.Color().GetBlue();
                    aAlpha = aAlpha + rEnt.Color().GetTransparency();
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
        if(aNormal.getZ() < 0.0
            && (GetLightGroup() && GetLightGroup()->GetModelTwoSide()))
            eMat = Base3DMaterialBack;
        Base3DRenderMode eMode = GetRenderMode(eMat);

        if(eMode == Base3DRenderPoint)
        {
            // Als Punktmenge ausgeben
            for(sal_uInt32 i=0;i<nNumPoints;i++)
            {
                Create3DPointClipped(aEdgeIndex[i]);
            }
        }
        else if(eMode == Base3DRenderLine)
        {
            // Als Linien ausgeben
            sal_uInt32 i2, i3;
            for(sal_uInt32 i=0;i<nNumPoints;i++)
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
                for(sal_uInt32 i=2;i < nNumPoints; i++)
                {
                    Clipped3DTriangle(
                        aEdgeIndex[0],
                        aEdgeIndex[i-1],
                        aEdgeIndex[i]);
                    bLastPrimitiveRejected = sal_False;
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

sal_Bool Base3DCommon::Clip3DPoint(sal_uInt32 nInd)
{
    return (!(sal_Bool)GetClipFlags(nInd));
}

/*************************************************************************
|*
|* Vergleicht, ob die beiden Entities geometrisch gleich sind
|*
\************************************************************************/

sal_Bool Base3DCommon::AreEqual(sal_uInt32 nInd1, sal_uInt32 nInd2)
{
    const basegfx::B3DPoint& rPnt1 = aBuffers[nInd1].Point();
    const basegfx::B3DPoint& rPnt2 = aBuffers[nInd2].Point();

    return rPnt1.equal(rPnt2);
}

/*************************************************************************
|*
|* Linie am canonical view volume clippen
|*
\************************************************************************/

sal_Bool Base3DCommon::Clip3DLine(sal_uInt32& nInd1,sal_uInt32& nInd2)
{
    sal_uInt16 nFlag0, nFlag1;
    do
    {
        nFlag0 = GetClipFlags(nInd1);
        nFlag1 = GetClipFlags(nInd2);

        // Beide Endpunkte drin?
        if(!(nFlag0 | nFlag1))
            return sal_True;

        // Linie komplett draussen?
        if(nFlag0 & nFlag1)
            return sal_False;

        // Es muss geclippt werden, bereite einen neuen Punkt vor
        sal_uInt32 nNewIndex = aBuffers.Count();
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
    return sal_True;
}

/*************************************************************************
|*
|* ClipFlags eines Punktes ermitteln und zurueckgeben
|*
\************************************************************************/

sal_uInt16 Base3DCommon::GetClipFlags(sal_uInt32 nInd)
{
    sal_uInt16 nRetval(0);
    basegfx::B3DPoint& rPoint = aBuffers[nInd].Point();

    if(rPoint.getX() < -(1.0 + SMALL_DVALUE))
        nRetval |= CLIPFLAG_LEFT;
    if(rPoint.getX() >  1.0 + SMALL_DVALUE)
        nRetval |= CLIPFLAG_RIGHT;

    if(rPoint.getY() < -(1.0 + SMALL_DVALUE))
        nRetval |= CLIPFLAG_BOTTOM;
    if(rPoint.getY() >  1.0 + SMALL_DVALUE)
        nRetval |= CLIPFLAG_TOP;

    if(rPoint.getZ() < -(1.0 + SMALL_DVALUE))
        nRetval |= CLIPFLAG_FRONT;
    if(rPoint.getZ() >  1.0 + SMALL_DVALUE)
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

sal_Bool Base3DCommon::Clip3DPolygon(sal_uInt32Bucket& rEdgeIndex)
{
    sal_uInt32 i;
    sal_uInt16 nAllFlagsOr, nAllFlagsAnd;

    do
    {
        // ClipFlags holen
        nAllFlagsOr = 0;
        nAllFlagsAnd = CLIPFLAG_ALL;

        for(i=0; i < rEdgeIndex.Count(); i++)
        {
            sal_uInt16 nFlag = GetClipFlags(rEdgeIndex[i]);
            nAllFlagsOr |= nFlag;
            nAllFlagsAnd &= nFlag;
        }

        // Alle Endpunkte drin?
        if(!nAllFlagsOr)
            return sal_True;

        // Dreieck komplett draussen?
        if(nAllFlagsAnd)
            return sal_False;

        if(nAllFlagsOr & (CLIPFLAG_FRONT|CLIPFLAG_BACK))
        {
            // clippen in Z
            if(nAllFlagsOr & CLIPFLAG_FRONT)
            {
                ClipPoly(rEdgeIndex, 2, sal_True);
            }
            else
            {
                ClipPoly(rEdgeIndex, 2, sal_False);
            }
        }
        else if(nAllFlagsOr & (CLIPFLAG_LEFT|CLIPFLAG_RIGHT))
        {
            // clippen in X
            if(nAllFlagsOr & CLIPFLAG_LEFT)
            {
                ClipPoly(rEdgeIndex, 0, sal_True);
            }
            else
            {
                ClipPoly(rEdgeIndex, 0, sal_False);
            }
        }
        else
        {
            // clippen in Y
            if(nAllFlagsOr & CLIPFLAG_BOTTOM)
            {
                ClipPoly(rEdgeIndex, 1, sal_True);
            }
            else
            {
                ClipPoly(rEdgeIndex, 1, sal_False);
            }
        }
    } while(nAllFlagsOr);
    return sal_True;
}

/*************************************************************************
|*
|* Testen, ob die Entitaet nInd innerhalb des canonical view volume liegt
|*
\************************************************************************/

sal_Bool Base3DCommon::IsInside(sal_uInt32 nInd, sal_uInt32 nDim, sal_Bool bLow)
{
    B3dEntity& aEntity = aBuffers[nInd];

    // #110270#
    // Add SMALL_DVALUE to this tests
    if(bLow)
    {
        if(aEntity.Point()[nDim] < -(1.0 + SMALL_DVALUE))
            return sal_False;
    }
    else
    {
        if(aEntity.Point()[nDim] > (1.0 + SMALL_DVALUE))
            return sal_False;
    }
    return sal_True;
}

/*************************************************************************
|*
|* Macht einen Clippingdurchgang eines Polygons in der angegebenen
|* Dimension. Es werden eventuell neue Punkte erzeugt.
|*
\************************************************************************/

void Base3DCommon::ClipPoly(sal_uInt32Bucket& rEdgeIndex, sal_uInt16 nDim, sal_Bool bLow)
{
    sal_uInt32 nNumEdges = rEdgeIndex.Count();
    sal_uInt32 nCurrentInd = rEdgeIndex[0];
    sal_Bool bCurrentInside = IsInside(nCurrentInd, nDim, bLow);
    sal_uInt32 nNextInd;
    sal_Bool bNextInside;
    sal_uInt32Bucket aEdgeIndex(8);

    for(sal_uInt32 i=0;i<nNumEdges;i++)
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
                sal_uInt32 nNewIndex = aBuffers.Count();
                aBuffers.Append();

                // Schnittpunkt berechnen
                if(bLow)
                    CalcNewPoint(nNewIndex, nCurrentInd, nNextInd, nDim, -1.0);
                else
                    CalcNewPoint(nNewIndex, nNextInd, nCurrentInd, nDim, 1.0);

                // EdgeFlag behandeln, beim Verlassen zuruecksetzen
                if(aBuffers[nCurrentInd].IsEdgeVisible())
                    aBuffers[nNewIndex].SetEdgeVisible(sal_False);

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
                sal_uInt32 nNewIndex = aBuffers.Count();
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

void Base3DCommon::CalcNewPoint(sal_uInt32 nNew, sal_uInt32 nHigh, sal_uInt32 nLow,
    sal_uInt16 nDim, double fBound)
{
    B3dEntity& aNew = aBuffers[nNew];
    B3dEntity& aHigh = aBuffers[nHigh];
    B3dEntity& aLow = aBuffers[nLow];
    double fFactor(1.0);
    aNew.Reset();

    if(aLow.Point()[nDim] != aHigh.Point()[nDim])
    {
        // #i27159#
        // Still numerical problems here around 0.0 and 1.0. I decided to
        // go the numerically more cautios way and avoid values which get
        // a little bit below 0.0 or above 1.0 here. The original line was:
        // fFactor = (fBound - aHigh.Point()[nDim]) / (aLow.Point()[nDim] - aHigh.Point()[nDim]);
        const double fFactorA(fBound - aHigh.Point()[nDim]);

        if(fabs(fFactorA) < SMALL_DVALUE)
        {
            fFactor = 0.0;
        }
        else
        {
            const double fFactorB(aLow.Point()[nDim] - aHigh.Point()[nDim]);

            if(fabs(fFactorA - fFactorB) < SMALL_DVALUE)
            {
                fFactor = 1.0;
            }
            else
            {
                fFactor = fFactorA / fFactorB;
            }
        }
    }

#ifdef DBG_UTIL
        if(fFactor > 1.0 || fFactor < 0.0)
            DBG_ERROR("Wrong clipping factor (out of range)!");
#endif

    if(fFactor == 0.0)
    {
        // #110270#
        // If fFactor is 0.0, copy the point
        aNew.Copy(aHigh);
    }
    else if(fFactor == 1.0)
    {
        // #110270#
        // If fFactor is 1.0, copy the point
        aNew.Copy(aLow);
    }
    else
    {
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
}

/*************************************************************************
|*
|* Beleuchtungsmodell (ColorModel) in einem Punkt loesen
|* Dabei das Ergebnis in rCol ablegen, d.h. wirklich modifizieren
|*
\************************************************************************/

void Base3DCommon::SolveColorModel(B3dColor& rCol, basegfx::B3DVector& rVec, const basegfx::B3DPoint& rPnt)
{
    if(GetLightGroup() && GetLightGroup()->IsLightingEnabled())
    {
        B3dMaterial& rMat = GetMaterialObject();
        sal_Bool bDoSolve(sal_True);

        // Welches Material? Zeigt der Vektor vom Betrachter
        // weg oder auf diesen?
        if(rVec.getZ() < 0.0)
        {
            // Rueckseite, soll diese dargestellt werden?
            if(GetLightGroup() && GetLightGroup()->GetModelTwoSide())
            {
                rMat = GetMaterialObject(Base3DMaterialBack);
            }
            else
            {
                bDoSolve = sal_False;
            }
        }
        if(bDoSolve)
            rCol = SolveColorModel(rMat, rVec, rPnt);
    }
}

B3dColor Base3DCommon::SolveColorModel(B3dMaterial& rMat, basegfx::B3DVector& rVec, const basegfx::B3DPoint& rPnt)
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
        basegfx::B3DPoint aPnt(rPnt);
        aPnt = GetTransformationSet()->DeviceToEyeCoor(aPnt);

        // Falls die Normale vom Betrachter weg zeigt und das Beleuchtungs-
        // modell doppelseitig ist, Normale umdrehen
        basegfx::B3DVector aVec(rVec);
        // #93184# use flag from polygon normal direction to switch local normal eventually
        if(bNormalPointsAway && GetLightGroup()->GetModelTwoSide())
            aVec = -rVec;

        // Die einzelnen Lichtquellen einbeziehen
        for(sal_uInt16 i=Base3DLight0; i <= Base3DLight7; i++)
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

B3dColor Base3DCommon::SolveColorModel(B3dLight& rLight, B3dMaterial& rMat, basegfx::B3DVector& rVec, const basegfx::B3DPoint& rPnt)
{
    B3dColor aRetval(255, 0, 0, 0);
    if(rLight.IsEnabled())
    {
        // Faktor mit Attenuation 1.0 initialisieren, falls
        // IsDirectionalSource() == sal_True
        double fFac = 1.0;
        basegfx::B3DVector aLightToVertex;
        sal_Bool bLightToVertex(sal_False);

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
                bLightToVertex = sal_True;
                double fLen = aLightToVertex.getLength();
                aLightToVertex.normalize();

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
                    aLightToVertex.normalize();
                    bLightToVertex = sal_True;
                }
                double fCosAngle = aLightToVertex.scalar(rLight.GetSpotDirection());
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
                    aLightToVertex.normalize();
                }
                double fCosFac = aLightToVertex.scalar(rVec);

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
                            aLightToVertex.setZ(aLightToVertex.getZ() + 1.0);
                        }
                        else
                        {
                            // vector Vertex to Viewpoint berechnen in
                            // Augkoordinaten, ist 0 - rPnt
                            aLightToVertex -= rPnt;
                        }
                        aLightToVertex.normalize();
                        fCosFac = aLightToVertex.scalar(rVec);
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

// eof
