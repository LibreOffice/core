/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: b3dgeom.cxx,v $
 * $Revision: 1.13 $
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
#include <goodies/b3dgeom.hxx>
#include <goodies/b3dcompo.hxx>

//#ifndef _B3D_HMATRIX_HXX
//#include "hmatrix.hxx"
//#endif
#include <goodies/base3d.hxx>
#include <tools/debug.hxx>

#ifndef _INC_MATH
#include <math.h>
#endif
#include <basegfx/matrix/b3dhommatrix.hxx>

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
    bHintIsComplex = sal_False;
    if(pComplexPolygon)
        delete pComplexPolygon;
    pComplexPolygon = NULL;

    // #93136# since #92030# uses bOutline flag now as indication
    // if the filled object is to be drawn, it MUST be initialized now.
    bOutline = sal_False;
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

void B3dGeometry::StartObject(sal_Bool bHintComplex, sal_Bool bOutl)
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
    sal_uInt32 nLow = 0L;
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
    if(aIndexBucket.Count())
    {
        sal_uInt32 nHigh = aIndexBucket[aIndexBucket.Count()-1].GetIndex();
        basegfx::B3DVector aPlaneNormal = -CalcNormal(nLow, nHigh);
        while(nLow < nHigh)
            aEntityBucket[nLow++].PlaneNormal() = aPlaneNormal;
    }
}

/*************************************************************************
|*
|* Geometrieuebergabe
|*
\************************************************************************/

void B3dGeometry::AddEdge(const basegfx::B3DPoint& rPoint)
{
    if(bHintIsComplex)
    {
        B3dEntity& rNew = pComplexPolygon->GetFreeEntity();

        rNew.Reset();
        rNew.Point() = rPoint;
        rNew.PlaneNormal() = basegfx::B3DVector(); // #i68442# Init PlaneNormal
        rNew.SetValid();
        rNew.SetEdgeVisible(sal_True);

        pComplexPolygon->PostAddVertex(rNew);
    }
    else
    {
        B3dEntity& rNew = GetFreeEntity();

        rNew.Reset();
        rNew.Point() = rPoint;
        rNew.PlaneNormal() = basegfx::B3DVector(); // #i68442# Init PlaneNormal
        rNew.SetValid();
        rNew.SetEdgeVisible(sal_True);
    }
}

void B3dGeometry::AddEdge(const basegfx::B3DPoint& rPoint, const basegfx::B3DVector& rNormal)
{
    if(bHintIsComplex)
    {
        B3dEntity& rNew = pComplexPolygon->GetFreeEntity();

        rNew.Reset();
        rNew.Point() = rPoint;
        rNew.PlaneNormal() = basegfx::B3DVector(); // #i68442# Init PlaneNormal
        rNew.SetValid();
        rNew.Normal() = rNormal;
        rNew.SetNormalUsed();
        rNew.SetEdgeVisible(sal_True);

        pComplexPolygon->PostAddVertex(rNew);
    }
    else
    {
        B3dEntity& rNew = GetFreeEntity();

        rNew.Reset();
        rNew.Point() = rPoint;
        rNew.PlaneNormal() = basegfx::B3DVector(); // #i68442# Init PlaneNormal
        rNew.SetValid();
        rNew.Normal() = rNormal;
        rNew.SetNormalUsed();
        rNew.SetEdgeVisible(sal_True);
    }
}

void B3dGeometry::AddEdge(const basegfx::B3DPoint& rPoint, const basegfx::B3DVector& rNormal, const basegfx::B2DPoint& rTexture)
{
    if(bHintIsComplex)
    {
        B3dEntity& rNew = pComplexPolygon->GetFreeEntity();

        rNew.Reset();
        rNew.Point() = rPoint;
        rNew.PlaneNormal() = basegfx::B3DVector(); // #i68442# Init PlaneNormal
        rNew.SetValid();
        rNew.Normal() = rNormal;
        rNew.SetNormalUsed();
        rNew.TexCoor() = rTexture;
        rNew.SetTexCoorUsed();
        rNew.SetEdgeVisible(sal_True);

        pComplexPolygon->PostAddVertex(rNew);
    }
    else
    {
        B3dEntity& rNew = GetFreeEntity();

        rNew.Reset();
        rNew.Point() = rPoint;
        rNew.PlaneNormal() = basegfx::B3DVector(); // #i68442# Init PlaneNormal
        rNew.SetValid();
        rNew.Normal() = rNormal;
        rNew.SetNormalUsed();
        rNew.TexCoor() = rTexture;
        rNew.SetTexCoorUsed();
        rNew.SetEdgeVisible(sal_True);
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
    bHintIsComplex = sal_False;
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

void B3dGeometry::AddComplexVertex(B3dEntity& rNew, sal_Bool bIsVisible)
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

void B3dGeometry::Transform(const basegfx::B3DHomMatrix& rMat)
{
    for(sal_uInt32 a=0;a<aEntityBucket.Count();a++)
        aEntityBucket[a].Transform(rMat);
}

/*************************************************************************
|*
|* Hittest auf Geometrie
|* Liegt der angegebene Schnittpunkt innerhalb eines der Polygone?
|*
\************************************************************************/

sal_Bool B3dGeometry::CheckHit(const basegfx::B3DPoint& rFront, const basegfx::B3DPoint& rBack, sal_uInt16 /*nTol*/)
{
    sal_uInt32 nPolyCounter(0L);
    sal_uInt32 nEntityCounter(0L);
    sal_uInt32 nUpperBound(0L);

    while(nPolyCounter < aIndexBucket.Count())
    {
        // Obergrenze neues Polygon holen
        nUpperBound = aIndexBucket[nPolyCounter++].GetIndex();

        // Hittest fuer momentanes Polygon
        basegfx::B3DPoint aCut;

        if(CheckSinglePolygonHit(nEntityCounter, nUpperBound, rFront, rBack, aCut))
        {
            return sal_True;
        }

        // Auf naechstes Polygon
        nEntityCounter = nUpperBound;
    }

    return sal_False;
}

// #110988# get all cuts of the geometry with the given vector defined by the two positions
void B3dGeometry::GetAllCuts(::std::vector< basegfx::B3DPoint >& rVector, const basegfx::B3DPoint& rFront, const basegfx::B3DPoint& rBack) const
{
    sal_uInt32 nPolyCounter(0L);
    sal_uInt32 nEntityCounter(0L);
    sal_uInt32 nUpperBound;

    while(nPolyCounter < ((B3dGeometry*)this)->aIndexBucket.Count())
    {
        // get upper bound for new polygon
        nUpperBound = ((B3dGeometry*)this)->aIndexBucket[nPolyCounter++].GetIndex();

        // get cut for that polygon
        basegfx::B3DPoint aCut;

        if(CheckSinglePolygonHit(nEntityCounter, nUpperBound, rFront, rBack, aCut))
        {
            rVector.push_back(aCut);
        }

        // Auf naechstes Polygon
        nEntityCounter = nUpperBound;
    }
}

sal_Bool B3dGeometry::CheckSinglePolygonHit(sal_uInt32 nLow, sal_uInt32 nHigh, const basegfx::B3DPoint& rFront, const basegfx::B3DPoint& rBack, basegfx::B3DPoint& rCut) const
{
    if(nLow + 2 < nHigh)
    {
        // calculate cut with plane
        if(GetCutPoint(nLow, rCut, rFront, rBack))
        {
            // cut exists, is it inside the polygon?
            if(IsInside(nLow, nHigh, rCut))
            {
                return sal_True;
            }
        }
    }

    return sal_False;
}

sal_Bool B3dGeometry::GetCutPoint(sal_uInt32 nLow, basegfx::B3DPoint& rCut, const basegfx::B3DPoint& rFront, const basegfx::B3DPoint& rBack) const
{
    sal_Bool bCutValid = sal_False;

    // Normale und Skalar der Ebenengleichung ermitteln
    const basegfx::B3DVector aNormal = ((B3dGeometry*)this)->aEntityBucket[nLow].PlaneNormal();
    const basegfx::B3DVector aPointAsVec = ((B3dGeometry*)this)->aEntityBucket[nLow + 1].Point();
    double fScalar = -(aPointAsVec.scalar(aNormal));
    basegfx::B3DVector aLineVec = rFront - rBack;
    double fZwi = aNormal.scalar(aLineVec);

    if(fabs(fZwi) > SMALL_DVALUE)
    {
        fZwi = (-fScalar - (basegfx::B3DVector(rBack).scalar(aNormal))) / fZwi;
        rCut.setX(rBack.getX() + (aLineVec.getX() * fZwi));
        rCut.setY(rBack.getY() + (aLineVec.getY() * fZwi));
        rCut.setZ(rBack.getZ() + (aLineVec.getZ() * fZwi));

        bCutValid = sal_True;
    }
    return bCutValid;
}

sal_Bool B3dGeometry::IsInside(sal_uInt32 nLow, sal_uInt32 nHigh, const basegfx::B3DPoint& rPnt) const
{
    sal_Bool bInside(sal_False);
    basegfx::B3DRange aVolume;

    // Volume von genau dieser Flaeche feststellen
    for(sal_uInt32 a=nLow;a<nHigh;a++)
        aVolume.expand(((B3dGeometry*)this)->aEntityBucket[a].Point());

    // Hier eigentlich ein aVolume.IsInside(rPnt), doch da hier ein
    // Vergleich mit Epsilon-Umgebung gebraucht wird, vergleiche selbst
    sal_Bool bIsInside =
          (rPnt.getX() + SMALL_DVALUE >= aVolume.getMinX() && rPnt.getX() - SMALL_DVALUE <= aVolume.getMaxX()
        && rPnt.getY() + SMALL_DVALUE >= aVolume.getMinY() && rPnt.getY() - SMALL_DVALUE <= aVolume.getMaxY()
        && rPnt.getZ() + SMALL_DVALUE >= aVolume.getMinZ() && rPnt.getZ() - SMALL_DVALUE <= aVolume.getMaxZ());

    if(bIsInside)
    {
        sal_Bool bInsideXY(sal_False);
        sal_Bool bInsideXZ(sal_False);
        sal_Bool bInsideYZ(sal_False);
        const basegfx::B3DPoint* pPrev = &(((B3dGeometry*)this)->aEntityBucket[nHigh - 1].Point());
        const basegfx::B3DPoint* pActual;
        basegfx::B3DVector aDiffPrev, aDiffActual;

        while(nLow < nHigh)
        {
            // Neuen Punkt holen
            pActual = &(((B3dGeometry*)this)->aEntityBucket[nLow++].Point());

            // Diffs bilden
            aDiffPrev = *pPrev - rPnt;
            aDiffActual = *pActual - rPnt;

            // Ueberschneidung in Y moeglich?
            if((aDiffPrev.getY() > 0.0 && aDiffActual.getY() <= 0.0) || (aDiffActual.getY() > 0.0 && aDiffPrev.getY() <= 0.0))
            {
                // in welchem Bereich liegt X?
                if(aDiffPrev.getX() >= 0.0 && aDiffActual.getX() >= 0.0)
                {
                    // Ueberschneidung
                    bInsideXY = !bInsideXY;
                }
                else if((aDiffPrev.getX() > 0.0 && aDiffActual.getX() <= 0.0) || (aDiffActual.getX() > 0.0 && aDiffPrev.getX() <= 0.0))
                {
                    // eventuell Ueberschneidung
                    // wo liegt die X-Koordinate des Schnitts mit der X-Achse?
                    if(aDiffActual.getY() != aDiffPrev.getY())
                        if(aDiffPrev.getX() - ((aDiffPrev.getY() * (aDiffActual.getX() - aDiffPrev.getX())) / (aDiffActual.getY() - aDiffPrev.getY())) >= 0.0)
                            // Ueberschneidung
                            bInsideXY = !bInsideXY;
                }

                // in welchem Bereich liegt Z?
                if(aDiffPrev.getZ() >= 0.0 && aDiffActual.getZ() >= 0.0)
                {
                    // Ueberschneidung
                    bInsideYZ = !bInsideYZ;
                }
                else if((aDiffPrev.getZ() > 0.0 && aDiffActual.getZ() <= 0.0) || (aDiffActual.getZ() > 0.0 && aDiffPrev.getZ() <= 0.0))
                {
                    // eventuell Ueberschneidung
                    // wo liegt die X-Koordinate des Schnitts mit der X-Achse?
                    if(aDiffActual.getY() != aDiffPrev.getY())
                        if(aDiffPrev.getZ() - ((aDiffPrev.getY() * (aDiffActual.getZ() - aDiffPrev.getZ())) / (aDiffActual.getY() - aDiffPrev.getY())) >= 0.0)
                            // Ueberschneidung
                            bInsideYZ = !bInsideYZ;
                }
            }

            // Ueberschneidung in X moeglich?
            if((aDiffPrev.getX() > 0.0 && aDiffActual.getX() <= 0.0) || (aDiffActual.getX() > 0.0 && aDiffPrev.getX() <= 0.0))
            {
                // in welchem Bereich liegt Z?
                if(aDiffPrev.getZ() >= 0.0 && aDiffActual.getZ() >= 0.0)
                {
                    // Ueberschneidung
                    bInsideXZ = !bInsideXZ;
                }
                else if((aDiffPrev.getZ() > 0.0 && aDiffActual.getZ() <= 0.0) || (aDiffActual.getZ() > 0.0 && aDiffPrev.getZ() <= 0.0))
                {
                    // eventuell Ueberschneidung
                    // wo liegt die X-Koordinate des Schnitts mit der X-Achse?
                    if(aDiffActual.getX() != aDiffPrev.getX())
                        if(aDiffPrev.getZ() - ((aDiffPrev.getX() * (aDiffActual.getZ() - aDiffPrev.getZ())) / (aDiffActual.getX() - aDiffPrev.getX())) >= 0.0)
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

basegfx::B3DRange B3dGeometry::GetBoundVolume() const
{
    basegfx::B3DRange aRetval;

    for(sal_uInt32 a=0;a<((B3dGeometry*)this)->aEntityBucket.Count();a++)
    {
        aRetval.expand(((B3dGeometry*)this)->aEntityBucket[a].Point());
    }

    return aRetval;
}

/*************************************************************************
|*
|* Mittelpunkt liefern
|*
\************************************************************************/

basegfx::B3DPoint B3dGeometry::GetCenter() const
{
    basegfx::B3DRange aVolume = GetBoundVolume();
    return aVolume.getCenter();
}

/*************************************************************************
|*
|* Standard - Normalen generieren
|*
\************************************************************************/

void B3dGeometry::CreateDefaultNormalsSphere()
{
    // Alle Normalen ausgehend vom Zentrum der Geometrie bilden
    basegfx::B3DPoint aCenter = GetCenter();

    for(sal_uInt32 a=0;a<aEntityBucket.Count();a++)
    {
        const basegfx::B3DPoint& aPoint = aEntityBucket[a].Point();
        basegfx::B3DVector aNewNormal = aPoint - aCenter;
        aNewNormal.normalize();
        aEntityBucket[a].Normal() = aNewNormal;
        aEntityBucket[a].SetNormalUsed(sal_True);
    }
}

/*************************************************************************
|*
|* Normale ermitteln fuer einzelnes Polygon
|*
\************************************************************************/

basegfx::B3DVector B3dGeometry::CalcNormal(sal_uInt32 nLow, sal_uInt32 nHigh) const
{
    const basegfx::B3DPoint* pVec1 = NULL;
    const basegfx::B3DPoint* pVec2 = NULL;
    const basegfx::B3DPoint* pVec3 = NULL;
    basegfx::B3DVector aNormal;

    while(nLow < nHigh && !(pVec1 && pVec2 && pVec3))
    {
        if(!pVec1)
        {
            pVec1 = &(((B3dGeometry*)this)->aEntityBucket[nLow++].Point());
        }
        else if(!pVec2)
        {
            pVec2 = &(((B3dGeometry*)this)->aEntityBucket[nLow++].Point());
            if(*pVec2 == *pVec1)
                pVec2 = NULL;
        }
        else if(!pVec3)
        {
            pVec3 = &(((B3dGeometry*)this)->aEntityBucket[nLow++].Point());
            if(*pVec3 == *pVec2 || *pVec3 == *pVec1) // #125865#
                pVec3 = NULL;
        }
    }
    if(pVec1 && pVec2 && pVec3)
    {
        aNormal = basegfx::B3DVector(*pVec2 - *pVec1).getPerpendicular(basegfx::B3DVector(*pVec2 - *pVec3));
        aNormal.normalize();
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
    for(sal_uInt32 a=0;a<aEntityBucket.Count();a++)
        aEntityBucket[a].SetNormalUsed(sal_False);
}

/*************************************************************************
|*
|* Standard - Texturkoordinaten generieren
|*
\************************************************************************/

void B3dGeometry::CreateDefaultTexture(sal_uInt16 nCreateWhat, sal_Bool bUseSphere)
{
    if(nCreateWhat)
    {
        if(bUseSphere)
        {
            // Texturkoordinaten mittels Kugelprojektion ermitteln,
            // dazu das Zentrum der Geometrie benutzen
            // Alle Normalen ausgehend vom Zentrum der Geometrie bilden
            basegfx::B3DPoint aCenter = GetCenter();
            sal_uInt32 nPointCounter = 0;

            for(sal_uInt32 a=0;a<aIndexBucket.Count();a++)
            {
                // Lokales Zentrum der zu behandelnden Flaeche bilden,
                // um zu wissen von welcher Seite sich diese Flaeche
                // dem Winkel F_PI bzw. -F_PI naehert
                basegfx::B3DVector aLocalCenter;
                for(sal_uInt32 b=nPointCounter;b<aIndexBucket[a].GetIndex();b++)
                    aLocalCenter += aEntityBucket[b].Point();
                aLocalCenter /= aIndexBucket[a].GetIndex() - nPointCounter;

                // Vektor vom Mittelpunkt zum lokalen Zentrum bilden
                if(fabs(aLocalCenter.getX()) < SMALL_DVALUE)
                    aLocalCenter.setX(0.0);
                if(fabs(aLocalCenter.getY()) < SMALL_DVALUE)
                    aLocalCenter.setY(0.0);
                if(fabs(aLocalCenter.getZ()) < SMALL_DVALUE)
                    aLocalCenter.setZ(0.0);

                // X,Y fuer das lokale Zentrum bilden
                double fXCenter = atan2(aLocalCenter.getZ(), aLocalCenter.getX());
                double fYCenter = atan2(aLocalCenter.getY(), aLocalCenter.getXZLength());
                fXCenter = 1.0 - ((fXCenter + F_PI) / F_2PI);
                fYCenter = 1.0 - ((fYCenter + F_PI2) / F_PI);

                // Einzelne Punkte behandeln
                sal_uInt32 nRememberPointCounter = nPointCounter;
                while(nPointCounter < aIndexBucket[a].GetIndex())
                {
                    // Vektor vom Mittelpunkt zum Punkt bilden
                    const basegfx::B3DPoint& aPoint = aEntityBucket[nPointCounter].Point();
                    basegfx::B3DVector aDirection = aPoint - aCenter;
                    if(fabs(aDirection.getX()) < SMALL_DVALUE)
                        aDirection.setX(0.0);
                    if(fabs(aDirection.getY()) < SMALL_DVALUE)
                        aDirection.setY(0.0);
                    if(fabs(aDirection.getZ()) < SMALL_DVALUE)
                        aDirection.setZ(0.0);

                    // X,Y fuer Punkt bilden
                    double fXPoint = atan2(aDirection.getZ(), aDirection.getX());
                    double fYPoint = atan2(aDirection.getY(), aDirection.getXZLength());
                    fXPoint = 1.0 - ((fXPoint + F_PI) / F_2PI);
                    fYPoint = 1.0 - ((fYPoint + F_PI2) / F_PI);

                    // X,Y des Punktes korrigieren
                    if(fXPoint > fXCenter + 0.5)
                        fXPoint -= 1.0;
                    if(fXPoint < fXCenter - 0.5)
                        fXPoint += 1.0;

                    // Polarkoordinaten als Texturkoordinaten zuweisen
                    if(nCreateWhat & B3D_CREATE_DEFAULT_X)
                        aEntityBucket[nPointCounter].TexCoor().setX(fXPoint);
                    if(nCreateWhat & B3D_CREATE_DEFAULT_Y)
                        aEntityBucket[nPointCounter].TexCoor().setY(fYPoint);

                    aEntityBucket[nPointCounter++].SetTexCoorUsed(sal_True);
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
                        basegfx::B2DPoint& aCoor = aEntityBucket[nPointCounter].TexCoor();
                        if(fabs(aCoor.getY()) < SMALL_DVALUE || fabs(aCoor.getY() - 1.0) < SMALL_DVALUE)
                        {
                            // Nachfolger finden
                            sal_uInt32 nNextIndex = (nPointCounter + 1 < aIndexBucket[a].GetIndex())
                                ? nPointCounter + 1 : nRememberPointCounter;
                            basegfx::B2DPoint& aNextCoor = aEntityBucket[nNextIndex].TexCoor();

                            // Vorgaenger finden
                            sal_uInt32 nPrevIndex = (nPointCounter && nPointCounter - 1 >= nRememberPointCounter)
                                ? nPointCounter - 1 : aIndexBucket[a].GetIndex() - 1;
                            basegfx::B2DPoint& aPrevCoor = aEntityBucket[nPrevIndex].TexCoor();

                            // Nachfolger testen: Liegt dieser ausserhalb des Pols?
                            if(fabs(aNextCoor.getY()) > SMALL_DVALUE && fabs(aNextCoor.getY() - 1.0) > SMALL_DVALUE)
                            {
                                // falls ja: X-Koordinate uebernehmen
                                aCoor.setX(aNextCoor.getX());
                            }
                            // Vorgaenger testen: Liegt dieser ausserhalb des Pols?
                            else if(fabs(aPrevCoor.getY()) > SMALL_DVALUE && fabs(aPrevCoor.getY() - 1.0) > SMALL_DVALUE)
                            {
                                // falls ja, X-Koordinate uebernehmen
                                aCoor.setX(aPrevCoor.getX());
                            }
                            else
                            {
                                // Weder Vorgaenger noch Nachfolger liegen ausserhalb des Pols.
                                // Uebernimm daher wenigstens den bereits korrigierten X-Wert
                                // des Vorgaengers
                                aCoor.setX(aPrevCoor.getX());
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
            basegfx::B3DRange aVolume = GetBoundVolume();

            for(sal_uInt32 a=0;a<aEntityBucket.Count();a++)
            {
                const basegfx::B3DPoint& aPoint = aEntityBucket[a].Point();

                if(nCreateWhat & B3D_CREATE_DEFAULT_X)
                {
                    if(aVolume.getWidth())
                        aEntityBucket[a].TexCoor().setX((aPoint.getX() - aVolume.getMinX()) / aVolume.getWidth());
                    else
                        aEntityBucket[a].TexCoor().setX(0.0);
                }

                if(nCreateWhat & B3D_CREATE_DEFAULT_Y)
                {
                    if(aVolume.getHeight())
                        aEntityBucket[a].TexCoor().setY(1.0 - ((aPoint.getY() - aVolume.getMinY()) / aVolume.getHeight()));
                    else
                        aEntityBucket[a].TexCoor().setY(1.0);
                }

                aEntityBucket[a].SetTexCoorUsed(sal_True);
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
    for(sal_uInt32 a=0;a<aEntityBucket.Count();a++)
        aEntityBucket[a].SetTexCoorUsed(sal_False);
}

/*************************************************************************
|*
|* Default-Geometrien erstellen
|*
\************************************************************************/

void B3dGeometry::CreateCube(const basegfx::B3DRange& rVolume)
{
    Erase();
    StartDescription();
    basegfx::B3DPoint A(rVolume.getMinX(), rVolume.getMaxY(), rVolume.getMinZ());
    basegfx::B3DPoint B(rVolume.getMaxX(), rVolume.getMaxY(), rVolume.getMinZ());
    basegfx::B3DPoint C(rVolume.getMaxX(), rVolume.getMinY(), rVolume.getMinZ());
    basegfx::B3DPoint D(rVolume.getMinX(), rVolume.getMinY(), rVolume.getMinZ());
    basegfx::B3DPoint E(rVolume.getMinX(), rVolume.getMaxY(), rVolume.getMaxZ());
    basegfx::B3DPoint F(rVolume.getMaxX(), rVolume.getMaxY(), rVolume.getMaxZ());
    basegfx::B3DPoint G(rVolume.getMaxX(), rVolume.getMinY(), rVolume.getMaxZ());
    basegfx::B3DPoint H(rVolume.getMinX(), rVolume.getMinY(), rVolume.getMaxZ());
    StartObject(sal_False);
    AddEdge(A);
    AddEdge(B);
    AddEdge(C);
    AddEdge(D);
    EndObject();
    StartObject(sal_False);
    AddEdge(A);
    AddEdge(E);
    AddEdge(F);
    AddEdge(B);
    EndObject();
    StartObject(sal_False);
    AddEdge(B);
    AddEdge(F);
    AddEdge(G);
    AddEdge(C);
    EndObject();
    StartObject(sal_False);
    AddEdge(C);
    AddEdge(G);
    AddEdge(H);
    AddEdge(D);
    EndObject();
    StartObject(sal_False);
    AddEdge(D);
    AddEdge(H);
    AddEdge(E);
    AddEdge(A);
    EndObject();
    StartObject(sal_False);
    AddEdge(E);
    AddEdge(H);
    AddEdge(G);
    AddEdge(F);
    EndObject();
    EndDescription();
    CreateDefaultNormalsSphere();
    CreateDefaultTexture(B3D_CREATE_DEFAULT_ALL, sal_False);
}

void B3dGeometry::CreateSphere(const basegfx::B3DRange& rVolume, double fX, double fY)
{
    Erase();
    StartDescription();
    basegfx::B3DPoint A,B,C,D;
    double fXInc, fYInc;
    if(fX == 0.0)
        fX = 4.0;
    fXInc = F_2PI / fX;
    if(fY == 0.0)
        fY = 4.0;
    fYInc = F_PI / fY;
    sal_uInt16 nX = (sal_uInt16)fX;
    sal_uInt16 nY = (sal_uInt16)fY;
    fX = 0.0;
    for(sal_uInt16 a=0;a<nX;a++,fX+=fXInc)
    {
        fY = -F_PI2;
        for(sal_uInt16 b=0;b<nY;b++,fY+=fYInc)
        {
            A.setY(sin(fY+fYInc));
            B.setY(A.getY());
            D.setY(sin(fY));
            C.setY(D.getY());
            A.setX(cos(fX) * cos(fY+fYInc));
            D.setX(cos(fX) * cos(fY));
            B.setX(cos(fX+fXInc) * cos(fY+fYInc));
            C.setX(cos(fX+fXInc) * cos(fY));
            A.setZ(sin(fX) * cos(fY+fYInc));
            D.setZ(sin(fX) * cos(fY));
            B.setZ(sin(fX+fXInc) * cos(fY+fYInc));
            C.setZ(sin(fX+fXInc) * cos(fY));
            StartObject(sal_False);
            AddEdge(A);
            AddEdge(B);
            AddEdge(C);
            AddEdge(D);
            EndObject();
        }
    }
    EndDescription();
    CreateDefaultNormalsSphere();
    CreateDefaultTexture(B3D_CREATE_DEFAULT_ALL, sal_True);
    basegfx::B3DHomMatrix aTransform;
    aTransform.translate(1.0, 1.0, 1.0);
    aTransform.scale(
        (rVolume.getMaxX() - rVolume.getMinX())/2.0,
        (rVolume.getMaxY() - rVolume.getMinY())/2.0,
        (rVolume.getMaxZ() - rVolume.getMinZ())/2.0);
    aTransform.translate(rVolume.getMinX(), rVolume.getMinY(), rVolume.getMinZ());
    Transform(aTransform);
}

/*************************************************************************
|*
|* Normalen invertieren
|*
\************************************************************************/

void B3dGeometry::InvertNormals()
{
    for(sal_uInt32 a=0;a<aEntityBucket.Count();a++)
        aEntityBucket[a].Normal() = -aEntityBucket[a].Normal();
}

// eof
