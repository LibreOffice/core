/*************************************************************************
 *
 *  $RCSfile: b3dgeom.cxx,v $
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

#ifndef _B3D_B3DGEOM_HXX
#include "b3dgeom.hxx"
#endif

#ifndef _B3D_B3DCOMPO_HXX
#include "b3dcompo.hxx"
#endif

#ifndef _B3D_HMATRIX_HXX
#include "hmatrix.hxx"
#endif

#ifndef _B3D_BASE3D_HXX
#include "base3d.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _INC_MATH
#include <math.h>
#endif

/*************************************************************************
|*
|* Bucket fuer Index
|*
\************************************************************************/

BASE3D_IMPL_BUCKET(GeometryIndexValue, Bucket)

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

B3dGeometry::B3dGeometry()
:   pComplexPolygon(NULL),
    aEntityBucket(14),      // 16K
    aIndexBucket(8)         // 256Byte
{
    Reset();
}

/*************************************************************************
|*
|* Ausgangszustand der Variablen herstellen
|*
\************************************************************************/

void B3dGeometry::Reset()
{
    bHintIsComplex = FALSE;
    if(pComplexPolygon)
        delete pComplexPolygon;
    pComplexPolygon = NULL;
}

/*************************************************************************
|*
|* Freien Eintrag zum fuellen holen
|*
\************************************************************************/

B3dEntity& B3dGeometry::GetFreeEntity()
{
    aEntityBucket.Append();
    return aEntityBucket[aEntityBucket.Count() - 1];
}

/*************************************************************************
|*
|* Inhalte loeschen
|*
\************************************************************************/

void B3dGeometry::Erase()
{
    aEntityBucket.Erase();
    aIndexBucket.Erase();
    Reset();
}

/*************************************************************************
|*
|* Inhalte loeschen und Speicher freigeben
|*
\************************************************************************/

void B3dGeometry::Empty()
{
    aEntityBucket.Empty();
    aIndexBucket.Empty();
    Reset();
}

/*************************************************************************
|*
|* Start der Geometriebeschreibung
|*
\************************************************************************/

void B3dGeometry::StartDescription()
{
    Erase();
}

/*************************************************************************
|*
|* Ende der Geometriebeschreibung
|*
\************************************************************************/

void B3dGeometry::EndDescription()
{
    if(pComplexPolygon)
        delete pComplexPolygon;
    pComplexPolygon = NULL;
}

/*************************************************************************
|*
|* Neues Primitiv beginnen
|*
\************************************************************************/

void B3dGeometry::StartObject(BOOL bHintComplex, BOOL bOutl)
{
    // Hint uebernehmen
    bHintIsComplex = bHintComplex;
    bOutline = bOutl;

    // ComplexPolygon anlegen falls nicht vorhanden
    if(bHintIsComplex)
    {
        if(!pComplexPolygon)
            pComplexPolygon = new B3dComplexPolygon;
        pComplexPolygon->StartPrimitive();
    }
    else
    {
        // Direkt neues Polygon beginnen
        StartPolygon();
    }
}

/*************************************************************************
|*
|* Primitiv abschliessen
|*
\************************************************************************/

void B3dGeometry::EndObject()
{
    // Unteren Index holen
    UINT32 nLow = 0L;
    if(aIndexBucket.Count())
        nLow = aIndexBucket[aIndexBucket.Count()-1].GetIndex();

    if(bHintIsComplex)
    {
        pComplexPolygon->EndPrimitive(this);
    }
    else
    {
        // Polygon abschliessen
        EndPolygon();
    }

    // EbenenNormale berechnen und setzen; bei Linien und
    // Punkten wird PlaneNormal auf (0,0,0) gesetzt
    UINT32 nHigh = aIndexBucket[aIndexBucket.Count()-1].GetIndex();
    Vector3D aPlaneNormal = -CalcNormal(nLow, nHigh);
    while(nLow < nHigh)
        aEntityBucket[nLow++].PlaneNormal() = aPlaneNormal;
}

/*************************************************************************
|*
|* Geometrieuebergabe
|*
\************************************************************************/

void B3dGeometry::AddEdge(const Vector3D& rPoint)
{
    if(bHintIsComplex)
    {
        B3dEntity& rNew = pComplexPolygon->GetFreeEntity();

        rNew.Reset();
        rNew.Point() = rPoint;
        rNew.SetValid();
        rNew.SetEdgeVisible(TRUE);

        pComplexPolygon->PostAddVertex(rNew);
    }
    else
    {
        B3dEntity& rNew = GetFreeEntity();

        rNew.Reset();
        rNew.Point() = rPoint;
        rNew.SetValid();
        rNew.SetEdgeVisible(TRUE);
    }
}

void B3dGeometry::AddEdge(
    const Vector3D& rPoint,
    const Vector3D& rNormal)
{
    if(bHintIsComplex)
    {
        B3dEntity& rNew = pComplexPolygon->GetFreeEntity();

        rNew.Reset();
        rNew.Point() = rPoint;
        rNew.SetValid();
        rNew.Normal() = rNormal;
        rNew.SetNormalUsed();
        rNew.SetEdgeVisible(TRUE);

        pComplexPolygon->PostAddVertex(rNew);
    }
    else
    {
        B3dEntity& rNew = GetFreeEntity();

        rNew.Reset();
        rNew.Point() = rPoint;
        rNew.SetValid();
        rNew.Normal() = rNormal;
        rNew.SetNormalUsed();
        rNew.SetEdgeVisible(TRUE);
    }
}

void B3dGeometry::AddEdge(
    const Vector3D& rPoint,
    const Vector3D& rNormal,
    const Vector3D& rTexture)
{
    if(bHintIsComplex)
    {
        B3dEntity& rNew = pComplexPolygon->GetFreeEntity();

        rNew.Reset();
        rNew.Point() = rPoint;
        rNew.SetValid();
        rNew.Normal() = rNormal;
        rNew.SetNormalUsed();
        rNew.TexCoor() = rTexture;
        rNew.SetTexCoorUsed();
        rNew.SetEdgeVisible(TRUE);

        pComplexPolygon->PostAddVertex(rNew);
    }
    else
    {
        B3dEntity& rNew = GetFreeEntity();

        rNew.Reset();
        rNew.Point() = rPoint;
        rNew.SetValid();
        rNew.Normal() = rNormal;
        rNew.SetNormalUsed();
        rNew.TexCoor() = rTexture;
        rNew.SetTexCoorUsed();
        rNew.SetEdgeVisible(TRUE);
    }
}

/*************************************************************************
|*
|* Copy-Operator
|*
\************************************************************************/

void B3dGeometry::operator=(const B3dGeometry& rObj)
{
    // Bucket kopieren
    aEntityBucket = rObj.aEntityBucket;
    aIndexBucket = rObj.aIndexBucket;

    // ComplexPolygon nicht kopieren
    pComplexPolygon = NULL;

    // Hint auch nicht
    bHintIsComplex = FALSE;
}

/*************************************************************************
|*
|* Callbacks bei komplexen Primitiven
|*
\************************************************************************/

void B3dGeometry::StartComplexPrimitive()
{
    StartPolygon();
}

void B3dGeometry::EndComplexPrimitive()
{
    EndPolygon();
}

void B3dGeometry::AddComplexVertex(B3dEntity& rNew, BOOL bIsVisible)
{
    // Kopieren
    B3dEntity& rLocal = GetFreeEntity();
    rLocal = rNew;

    // EdgeFlag anpassen
    rLocal.SetEdgeVisible(bIsVisible);
}

/*************************************************************************
|*
|* PolygonStart und -Ende aufzeichnen
|*
\************************************************************************/

void B3dGeometry::StartPolygon()
{
}

void B3dGeometry::EndPolygon()
{
    GeometryIndexValue aNewIndex(aEntityBucket.Count());
    if(bOutline)
        aNewIndex.SetMode(B3D_INDEX_MODE_LINE);
    aIndexBucket.Append(aNewIndex);
}

/*************************************************************************
|*
|* Eine beliebige Transformation auf die Geometrie anwenden
|*
\************************************************************************/

void B3dGeometry::Transform(const Matrix4D& rMat)
{
    for(UINT32 a=0;a<aEntityBucket.Count();a++)
        aEntityBucket[a].Transform(rMat);
}

/*************************************************************************
|*
|* Hittest auf Geometrie
|* Liegt der angegebene Schnittpunkt innerhalb eines der Polygone?
|*
\************************************************************************/

INT32 B3dGeometry::CheckHit(const Vector3D& rFront, const Vector3D& rBack, USHORT nTol)
{
    UINT32 nPolyCounter = 0;
    UINT32 nEntityCounter = 0;
    UINT32 nUpperBound;
    INT32 nRetval;

    while(nPolyCounter < aIndexBucket.Count())
    {
        // Obergrenze neues Polygon holen
        nUpperBound = aIndexBucket[nPolyCounter++].GetIndex();

        // Hittest fuer momentanes Polygon
        nRetval = CheckSinglePolygonHit(nEntityCounter, nUpperBound, rFront, rBack);
        if(nRetval != -1L)
            return nRetval;

        // Auf naechstes Polygon
        nEntityCounter = nUpperBound;
    }
    return nRetval;
}

INT32 B3dGeometry::CheckSinglePolygonHit(UINT32 nLow, UINT32 nHigh, const Vector3D& rFront, const Vector3D& rBack)
{
    if(nLow + 2 < nHigh)
    {
        // Schnittpunkt berechnen
        Vector3D aCut;
        if(GetCutPoint(nLow, aCut, rFront, rBack))
        {
            // Schnittpunkt existiert, liegt dieser im angegebenen
            // konvexen Polygon?
            if(IsInside(nLow, nHigh, aCut))
            {
                return ((INT32)(aCut.Z() + 0.5));
            }
        }
    }
    return -1L;
}

BOOL B3dGeometry::GetCutPoint(UINT32 nLow, Vector3D& rCut, const Vector3D& rFront, const Vector3D& rBack)
{
    BOOL bCutValid = FALSE;

    // Normale und Skalar der Ebenengleichung ermitteln
    Vector3D aNormal = aEntityBucket[nLow].PlaneNormal();
    double fScalar = -(aEntityBucket[nLow + 1].Point().GetVector3D().Scalar(aNormal));
    Vector3D aLineVec = rFront - rBack;
    double fZwi = aNormal.Scalar(aLineVec);

    if(fabs(fZwi) > SMALL_DVALUE)
    {
        fZwi = (-fScalar - (rBack.Scalar(aNormal))) / fZwi;
//      if(fZwi > SMALL_DVALUE && fZwi < 1.0 - SMALL_DVALUE)
//      {
            rCut.X() = rBack.X() + (aLineVec.X() * fZwi);
            rCut.Y() = rBack.Y() + (aLineVec.Y() * fZwi);
            rCut.Z() = rBack.Z() + (aLineVec.Z() * fZwi);

            bCutValid = TRUE;
//      }
    }
    return bCutValid;
}

BOOL B3dGeometry::IsInside(UINT32 nLow, UINT32 nHigh, const Vector3D& rPnt)
{
    BOOL bInside(FALSE);
    B3dVolume aVolume;

    // Volume von genau dieser Flaeche feststellen
    for(UINT32 a=nLow;a<nHigh;a++)
        aVolume.Union(aEntityBucket[a].Point().GetVector3D());

    // Hier eigentlich ein aVolume.IsInside(rPnt), doch da hier ein
    // Vergleich mit Epsilon-Umgebung gebraucht wird, vergleiche selbst
    BOOL bIsInside =
        (rPnt.X() + SMALL_DVALUE >= aVolume.MinVec().X() && rPnt.X() - SMALL_DVALUE <= aVolume.MaxVec().X()
        && rPnt.Y() + SMALL_DVALUE >= aVolume.MinVec().Y() && rPnt.Y() - SMALL_DVALUE <= aVolume.MaxVec().Y()
        && rPnt.Z() + SMALL_DVALUE >= aVolume.MinVec().Z() && rPnt.Z() - SMALL_DVALUE <= aVolume.MaxVec().Z());

    if(bIsInside)
    {
        BOOL bInsideXY(FALSE);
        BOOL bInsideXZ(FALSE);
        BOOL bInsideYZ(FALSE);
        const Vector3D* pPrev = &(aEntityBucket[nHigh - 1].Point().GetVector3D());
        const Vector3D* pActual;
        Vector3D aDiffPrev, aDiffActual;

        while(nLow < nHigh)
        {
            // Neuen Punkt holen
            pActual = &(aEntityBucket[nLow++].Point().GetVector3D());

            // Diffs bilden
            aDiffPrev = *pPrev - rPnt;
            aDiffActual = *pActual - rPnt;

            // Ueberschneidung in Y moeglich?
            if((aDiffPrev.Y() > 0.0 && aDiffActual.Y() <= 0.0) || (aDiffActual.Y() > 0.0 && aDiffPrev.Y() <= 0.0))
            {
                // in welchem Bereich liegt X?
                if(aDiffPrev.X() >= 0.0 && aDiffActual.X() >= 0.0)
                {
                    // Ueberschneidung
                    bInsideXY = !bInsideXY;
                }
                else if((aDiffPrev.X() > 0.0 && aDiffActual.X() <= 0.0) || (aDiffActual.X() > 0.0 && aDiffPrev.X() <= 0.0))
                {
                    // eventuell Ueberschneidung
                    // wo liegt die X-Koordinate des Schnitts mit der X-Achse?
                    if(aDiffActual.Y() != aDiffPrev.Y())
                        if(aDiffPrev.X() - ((aDiffPrev.Y() * (aDiffActual.X() - aDiffPrev.X())) / (aDiffActual.Y() - aDiffPrev.Y())) >= 0.0)
                            // Ueberschneidung
                            bInsideXY = !bInsideXY;
                }

                // in welchem Bereich liegt Z?
                if(aDiffPrev.Z() >= 0.0 && aDiffActual.Z() >= 0.0)
                {
                    // Ueberschneidung
                    bInsideYZ = !bInsideYZ;
                }
                else if((aDiffPrev.Z() > 0.0 && aDiffActual.Z() <= 0.0) || (aDiffActual.Z() > 0.0 && aDiffPrev.Z() <= 0.0))
                {
                    // eventuell Ueberschneidung
                    // wo liegt die X-Koordinate des Schnitts mit der X-Achse?
                    if(aDiffActual.Y() != aDiffPrev.Y())
                        if(aDiffPrev.Z() - ((aDiffPrev.Y() * (aDiffActual.Z() - aDiffPrev.Z())) / (aDiffActual.Y() - aDiffPrev.Y())) >= 0.0)
                            // Ueberschneidung
                            bInsideYZ = !bInsideYZ;
                }
            }

            // Ueberschneidung in X moeglich?
            if((aDiffPrev.X() > 0.0 && aDiffActual.X() <= 0.0) || (aDiffActual.X() > 0.0 && aDiffPrev.X() <= 0.0))
            {
                // in welchem Bereich liegt Z?
                if(aDiffPrev.Z() >= 0.0 && aDiffActual.Z() >= 0.0)
                {
                    // Ueberschneidung
                    bInsideXZ = !bInsideXZ;
                }
                else if((aDiffPrev.Z() > 0.0 && aDiffActual.Z() <= 0.0) || (aDiffActual.Z() > 0.0 && aDiffPrev.Z() <= 0.0))
                {
                    // eventuell Ueberschneidung
                    // wo liegt die X-Koordinate des Schnitts mit der X-Achse?
                    if(aDiffActual.X() != aDiffPrev.X())
                        if(aDiffPrev.Z() - ((aDiffPrev.X() * (aDiffActual.Z() - aDiffPrev.Z())) / (aDiffActual.X() - aDiffPrev.X())) >= 0.0)
                            // Ueberschneidung
                            bInsideXZ = !bInsideXZ;
                }
            }

            // Punkt als Vorgaenger merken
            pPrev = pActual;
        }
        // Wahrheitswert bilden
        bInside = bInsideXY || bInsideXZ || bInsideYZ;
    }
    return bInside;
}

/*************************************************************************
|*
|* BoundVolume liefern
|*
\************************************************************************/

B3dVolume B3dGeometry::GetBoundVolume()
{
    B3dVolume aRetval;

    for(UINT32 a=0;a<aEntityBucket.Count();a++)
        aRetval.Union(aEntityBucket[a].Point().GetVector3D());

    return aRetval;
}

/*************************************************************************
|*
|* Mittelpunkt liefern
|*
\************************************************************************/

Vector3D B3dGeometry::GetCenter()
{
    B3dVolume aVolume = GetBoundVolume();
    return (aVolume.MaxVec() + aVolume.MinVec()) / 2.0;
}

/*************************************************************************
|*
|* Standard - Normalen generieren
|*
\************************************************************************/

void B3dGeometry::CreateDefaultNormalsSphere()
{
    // Alle Normalen ausgehend vom Zentrum der Geometrie bilden
    Vector3D aCenter = GetCenter();

    for(UINT32 a=0;a<aEntityBucket.Count();a++)
    {
        const Vector3D& aPoint = aEntityBucket[a].Point().GetVector3D();
        Vector3D aNewNormal = aPoint - aCenter;
        aNewNormal.Normalize();
        aEntityBucket[a].Normal() = aNewNormal;
        aEntityBucket[a].SetNormalUsed(TRUE);
    }
}

/*************************************************************************
|*
|* Normale ermitteln fuer einzelnes Polygon
|*
\************************************************************************/

Vector3D B3dGeometry::CalcNormal(UINT32 nLow, UINT32 nHigh)
{
    const Vector3D* pVec1 = NULL;
    const Vector3D* pVec2 = NULL;
    const Vector3D* pVec3 = NULL;
    Vector3D aNormal;

    while(nLow < nHigh && !(pVec1 && pVec2 && pVec3))
    {
        if(!pVec1)
        {
            pVec1 = &(aEntityBucket[nLow++].Point().GetVector3D());
        }
        else if(!pVec2)
        {
            pVec2 = &(aEntityBucket[nLow++].Point().GetVector3D());
            if(*pVec2 == *pVec1)
                pVec2 = NULL;
        }
        else if(!pVec3)
        {
            pVec3 = &(aEntityBucket[nLow++].Point().GetVector3D());
            if(*pVec3 == *pVec2 || pVec3 == pVec1)
                pVec3 = NULL;
        }
    }
    if(pVec1 && pVec2 && pVec3)
    {
        aNormal = (*pVec2 - *pVec1)|(*pVec2 - *pVec3);
        aNormal.Normalize();
    }
    return aNormal;
}

/*************************************************************************
|*
|* Normaleninformationen ungueltig machen
|*
\************************************************************************/

void B3dGeometry::RemoveNormals()
{
    for(UINT32 a=0;a<aEntityBucket.Count();a++)
        aEntityBucket[a].SetNormalUsed(FALSE);
}

/*************************************************************************
|*
|* Standard - Texturkoordinaten generieren
|*
\************************************************************************/

void B3dGeometry::CreateDefaultTexture(UINT16 nCreateWhat, BOOL bUseSphere)
{
    if(nCreateWhat)
    {
        if(bUseSphere)
        {
            // Texturkoordinaten mittels Kugelprojektion ermitteln,
            // dazu das Zentrum der Geometrie benutzen
            // Alle Normalen ausgehend vom Zentrum der Geometrie bilden
            Vector3D aCenter = GetCenter();
            UINT32 nPointCounter = 0;

            for(UINT32 a=0;a<aIndexBucket.Count();a++)
            {
                // Lokales Zentrum der zu behandelnden Flaeche bilden,
                // um zu wissen von welcher Seite sich diese Flaeche
                // dem Winkel F_PI bzw. -F_PI naehert
                Vector3D aLocalCenter;
                for(UINT32 b=nPointCounter;b<aIndexBucket[a].GetIndex();b++)
                    aLocalCenter += aEntityBucket[b].Point().GetVector3D();
                aLocalCenter /= aIndexBucket[a].GetIndex() - nPointCounter;

                // Vektor vom Mittelpunkt zum lokalen Zentrum bilden
                aLocalCenter = aLocalCenter - aCenter;
                if(fabs(aLocalCenter.X()) < SMALL_DVALUE)
                    aLocalCenter.X() = 0.0;
                if(fabs(aLocalCenter.Y()) < SMALL_DVALUE)
                    aLocalCenter.Y() = 0.0;
                if(fabs(aLocalCenter.Z()) < SMALL_DVALUE)
                    aLocalCenter.Z() = 0.0;

                // X,Y fuer das lokale Zentrum bilden
                double fXCenter = atan2(aLocalCenter.Z(), aLocalCenter.X());
                double fYCenter = atan2(aLocalCenter.Y(), aLocalCenter.GetXZLength());
                fXCenter = 1.0 - ((fXCenter + F_PI) / F_2PI);
                fYCenter = 1.0 - ((fYCenter + F_PI2) / F_PI);

                // Einzelne Punkte behandeln
                UINT32 nRememberPointCounter = nPointCounter;
                while(nPointCounter < aIndexBucket[a].GetIndex())
                {
                    // Vektor vom Mittelpunkt zum Punkt bilden
                    const Vector3D& aPoint = aEntityBucket[nPointCounter].Point().GetVector3D();
                    Vector3D aDirection = aPoint - aCenter;
                    if(fabs(aDirection.X()) < SMALL_DVALUE)
                        aDirection.X() = 0.0;
                    if(fabs(aDirection.Y()) < SMALL_DVALUE)
                        aDirection.Y() = 0.0;
                    if(fabs(aDirection.Z()) < SMALL_DVALUE)
                        aDirection.Z() = 0.0;

                    // X,Y fuer Punkt bilden
                    double fXPoint = atan2(aDirection.Z(), aDirection.X());
                    double fYPoint = atan2(aDirection.Y(), aDirection.GetXZLength());
                    fXPoint = 1.0 - ((fXPoint + F_PI) / F_2PI);
                    fYPoint = 1.0 - ((fYPoint + F_PI2) / F_PI);

                    // X,Y des Punktes korrigieren
                    if(fXPoint > fXCenter + 0.5)
                        fXPoint -= 1.0;
                    if(fXPoint < fXCenter - 0.5)
                        fXPoint += 1.0;

                    // Polarkoordinaten als Texturkoordinaten zuweisen
                    if(nCreateWhat & B3D_CREATE_DEFAULT_X)
                        aEntityBucket[nPointCounter].TexCoor().X() = fXPoint;
                    if(nCreateWhat & B3D_CREATE_DEFAULT_Y)
                        aEntityBucket[nPointCounter].TexCoor().Y() = fYPoint;
                    if(nCreateWhat & B3D_CREATE_DEFAULT_Z)
                        aEntityBucket[nPointCounter].TexCoor().Z() = 0.0;

                    aEntityBucket[nPointCounter++].SetTexCoorUsed(TRUE);
                }

                // Punkte korrigieren, die direkt in den Polarregionen liegen. Deren
                // X-Koordinate kann nicht korrekt sein. Die korrekte X-Koordinate
                // ist diejenige des Punktes, der in den Pol hinein oder aus diesem heraus
                // fuehrt, auf der Kugel also direkt darauf zu.
                if(nCreateWhat & B3D_CREATE_DEFAULT_X)
                {
                    nPointCounter = nRememberPointCounter;
                    while(nPointCounter < aIndexBucket[a].GetIndex())
                    {
                        Vector3D& aCoor = aEntityBucket[nPointCounter].TexCoor();
                        if(fabs(aCoor.Y()) < SMALL_DVALUE || fabs(aCoor.Y() - 1.0) < SMALL_DVALUE)
                        {
                            // Nachfolger finden
                            UINT32 nNextIndex = (nPointCounter + 1 < aIndexBucket[a].GetIndex())
                                ? nPointCounter + 1 : nRememberPointCounter;
                            Vector3D& aNextCoor = aEntityBucket[nNextIndex].TexCoor();

                            // Vorgaenger finden
                            UINT32 nPrevIndex = (nPointCounter && nPointCounter - 1 >= nRememberPointCounter)
                                ? nPointCounter - 1 : aIndexBucket[a].GetIndex() - 1;
                            Vector3D& aPrevCoor = aEntityBucket[nPrevIndex].TexCoor();

                            // Nachfolger testen: Liegt dieser ausserhalb des Pols?
                            if(fabs(aNextCoor.Y()) > SMALL_DVALUE && fabs(aNextCoor.Y() - 1.0) > SMALL_DVALUE)
                            {
                                // falls ja: X-Koordinate uebernehmen
                                aCoor.X() = aNextCoor.X();
                            }
                            // Vorgaenger testen: Liegt dieser ausserhalb des Pols?
                            else if(fabs(aPrevCoor.Y()) > SMALL_DVALUE && fabs(aPrevCoor.Y() - 1.0) > SMALL_DVALUE)
                            {
                                // falls ja, X-Koordinate uebernehmen
                                aCoor.X() = aPrevCoor.X();
                            }
                            else
                            {
                                // Weder Vorgaenger noch Nachfolger liegen ausserhalb des Pols.
                                // Uebernimm daher wenigstens den bereits korrigierten X-Wert
                                // des Vorgaengers
                                aCoor.X() = aPrevCoor.X();
                            }
                        }
                        // naechster Punkt
                        nPointCounter++;
                    }
                }
            }
        }
        else
        {
            // Texturkoordinaten als Parallelprojektion auf X,Y,Z - Koordinaten
            // im Bereich 1.0 bis 0.0 der Geometrie abstellen
            // Gesamtabmessungen holen
            B3dVolume aVolume = GetBoundVolume();

            for(UINT32 a=0;a<aEntityBucket.Count();a++)
            {
                const Vector3D& aPoint = aEntityBucket[a].Point().GetVector3D();

                if(nCreateWhat & B3D_CREATE_DEFAULT_X)
                {
                    if(aVolume.GetWidth())
                        aEntityBucket[a].TexCoor().X() = (aPoint.X() - aVolume.MinVec().X()) / aVolume.GetWidth();
                    else
                        aEntityBucket[a].TexCoor().X() = 0.0;
                }

                if(nCreateWhat & B3D_CREATE_DEFAULT_Y)
                {
                    if(aVolume.GetHeight())
                        aEntityBucket[a].TexCoor().Y() = 1.0 - ((aPoint.Y() - aVolume.MinVec().Y()) / aVolume.GetHeight());
                    else
                        aEntityBucket[a].TexCoor().Y() = 1.0;
                }

                if(nCreateWhat & B3D_CREATE_DEFAULT_Z)
                    aEntityBucket[a].TexCoor().Z() = 0.0;

                aEntityBucket[a].SetTexCoorUsed(TRUE);
            }
        }
    }
}

/*************************************************************************
|*
|* Texturinformationen ungueltig machen
|*
\************************************************************************/

void B3dGeometry::RemoveTexture()
{
    for(UINT32 a=0;a<aEntityBucket.Count();a++)
        aEntityBucket[a].SetTexCoorUsed(FALSE);
}

/*************************************************************************
|*
|* Default-Geometrien erstellen
|*
\************************************************************************/

void B3dGeometry::CreateCube(const B3dVolume& rVolume)
{
    Erase();
    StartDescription();
    Vector3D A(rVolume.MinVec().X(), rVolume.MaxVec().Y(), rVolume.MinVec().Z());
    Vector3D B(rVolume.MaxVec().X(), rVolume.MaxVec().Y(), rVolume.MinVec().Z());
    Vector3D C(rVolume.MaxVec().X(), rVolume.MinVec().Y(), rVolume.MinVec().Z());
    Vector3D D(rVolume.MinVec().X(), rVolume.MinVec().Y(), rVolume.MinVec().Z());
    Vector3D E(rVolume.MinVec().X(), rVolume.MaxVec().Y(), rVolume.MaxVec().Z());
    Vector3D F(rVolume.MaxVec().X(), rVolume.MaxVec().Y(), rVolume.MaxVec().Z());
    Vector3D G(rVolume.MaxVec().X(), rVolume.MinVec().Y(), rVolume.MaxVec().Z());
    Vector3D H(rVolume.MinVec().X(), rVolume.MinVec().Y(), rVolume.MaxVec().Z());
    StartObject(FALSE);
    AddEdge(A);
    AddEdge(B);
    AddEdge(C);
    AddEdge(D);
    EndObject();
    StartObject(FALSE);
    AddEdge(A);
    AddEdge(E);
    AddEdge(F);
    AddEdge(B);
    EndObject();
    StartObject(FALSE);
    AddEdge(B);
    AddEdge(F);
    AddEdge(G);
    AddEdge(C);
    EndObject();
    StartObject(FALSE);
    AddEdge(C);
    AddEdge(G);
    AddEdge(H);
    AddEdge(D);
    EndObject();
    StartObject(FALSE);
    AddEdge(D);
    AddEdge(H);
    AddEdge(E);
    AddEdge(A);
    EndObject();
    StartObject(FALSE);
    AddEdge(E);
    AddEdge(H);
    AddEdge(G);
    AddEdge(F);
    EndObject();
    EndDescription();
    CreateDefaultNormalsSphere();
    CreateDefaultTexture(B3D_CREATE_DEFAULT_ALL, FALSE);
}

void B3dGeometry::CreateSphere(const B3dVolume& rVolume, double fX, double fY)
{
    Erase();
    StartDescription();
    Vector3D A,B,C,D;
    double fXInc, fYInc;
    if(fX == 0.0)
        fX = 4.0;
    fXInc = F_2PI / fX;
    if(fY == 0.0)
        fY = 4.0;
    fYInc = F_PI / fY;
    UINT16 nX = (UINT16)fX;
    UINT16 nY = (UINT16)fY;
    fX = 0.0;
    for(UINT16 a=0;a<nX;a++,fX+=fXInc)
    {
        fY = -F_PI2;
        for(UINT16 b=0;b<nY;b++,fY+=fYInc)
        {
            A.Y() = B.Y() = sin(fY+fYInc);
            D.Y() = C.Y() = sin(fY);
            A.X() = cos(fX) * cos(fY+fYInc);
            D.X() = cos(fX) * cos(fY);
            B.X() = cos(fX+fXInc) * cos(fY+fYInc);
            C.X() = cos(fX+fXInc) * cos(fY);
            A.Z() = sin(fX) * cos(fY+fYInc);
            D.Z() = sin(fX) * cos(fY);
            B.Z() = sin(fX+fXInc) * cos(fY+fYInc);
            C.Z() = sin(fX+fXInc) * cos(fY);
            StartObject(FALSE);
            AddEdge(A);
            AddEdge(B);
            AddEdge(C);
            AddEdge(D);
            EndObject();
        }
    }
    EndDescription();
    CreateDefaultNormalsSphere();
    CreateDefaultTexture(B3D_CREATE_DEFAULT_ALL, TRUE);
    Matrix4D aTransform;
    aTransform.Translate(Vector3D(1.0, 1.0, 1.0));
    aTransform.Scale(
        (rVolume.MaxVec().X() - rVolume.MinVec().X())/2.0,
        (rVolume.MaxVec().Y() - rVolume.MinVec().Y())/2.0,
        (rVolume.MaxVec().Z() - rVolume.MinVec().Z())/2.0);
    aTransform.Translate(rVolume.MinVec());
    Transform(aTransform);
}

/*************************************************************************
|*
|* Normalen invertieren
|*
\************************************************************************/

void B3dGeometry::InvertNormals()
{
    for(UINT32 a=0;a<aEntityBucket.Count();a++)
        aEntityBucket[a].Normal() = -aEntityBucket[a].Normal();
}

