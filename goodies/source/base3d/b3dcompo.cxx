/*************************************************************************
 *
 *  $RCSfile: b3dcompo.cxx,v $
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

#ifndef _B3D_B3DCOMPO_HXX
#include "b3dcompo.hxx"
#endif

#ifndef _B3D_BASE3D_HXX
#include "base3d.hxx"
#endif

#ifndef _B3D_B3DGEOM_HXX
#include "b3dgeom.hxx"
#endif

#ifndef _INC_FLOAT
#include <float.h>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

/*************************************************************************
|*
|* Vergleiche fuer doubles mit bound
|*
\************************************************************************/

#define DOUBLE_EQUAL(a,b) (fabs(a-b) < SMALL_DVALUE)
#define DOUBLE_NOT_EQUAL(a,b) (fabs(a-b) > SMALL_DVALUE)
#define DOUBLE_SMALLER(a,b) ((a + (SMALL_DVALUE / 2.0)) < b)
#define DOUBLE_BIGGER(a,b) ((a - (SMALL_DVALUE / 2.0)) > b)

/*************************************************************************
|*
|* Bucket fuer Kantenliste, vertikaler Teil
|*
\************************************************************************/

BASE3D_IMPL_BUCKET(B3dEdgeList, Bucket)

/*************************************************************************
|*
|* Bucket fuer Kantenliste, horizontaler Teil
|*
\************************************************************************/

BASE3D_IMPL_BUCKET(B3dEdgeEntry, Bucket)

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

B3dComplexPolygon::B3dComplexPolygon()
:   aEntityBuffer(14),      // 16K
    aEdgeList(12),          // 4K
    aEdgeEntry(12)          // 4K
{
    EmptyBuffers();
    bTestForCut = TRUE;
    nHighestEdge = 0L;
    pBase3D = NULL;
    pGeometry = NULL;
    pLastVertex = NULL;
}

/*************************************************************************
|*
|* Gib einen neuen freien Eintrag zurueck
|*
\************************************************************************/

B3dEntity &B3dComplexPolygon::GetFreeEntity()
{
    aEntityBuffer.Append();
    return aEntityBuffer[aEntityBuffer.Count() - 1];
}

/*************************************************************************
|*
|* Ein neuer Punkt ist ausgefuellt
|*
\************************************************************************/

void B3dComplexPolygon::PostAddVertex(B3dEntity &rVertex)
{
    if(pLastVertex)
    {
        if(ArePointsEqual(*pLastVertex, rVertex))
        {
            aEntityBuffer.Remove();
            return;
        }
        if(!nNewPolyStart)
        {
            if(nHighestEdge)
                TestHighestEdge(rVertex);
            else
                nHighestEdge = aEntityBuffer.Count();
        }
    }

    // Zeiger auf letzten hinzugefuegten Punkt setzen
    pLastVertex = &rVertex;
}

/*************************************************************************
|*
|* Testet, ob die neue Edge in allen Freiheitsgraden groesser ist
|* als die momentane
|*
\************************************************************************/

void B3dComplexPolygon::TestHighestEdge(B3dEntity& rVertex)
{
    B3dEntity& rHighest = aEntityBuffer[nHighestEdge - 1];
    if(rVertex.GetX() <= rHighest.GetX())
    {
        if(rVertex.GetX() < rHighest.GetX())
        {
            nHighestEdge = aEntityBuffer.Count();
        }
        else
        {
            if(rVertex.GetY() <= rHighest.GetY())
            {
                if(rVertex.GetY() < rHighest.GetY())
                {
                    nHighestEdge = aEntityBuffer.Count();
                }
                else
                {
                    if(rVertex.GetZ() < rHighest.GetZ())
                    {
                        nHighestEdge = aEntityBuffer.Count();
                    }
                }
            }
        }
    }
}

/*************************************************************************
|*
|* Vergleicht zwei Punkte auf ihren INHALT
|* und fuellt die 2D-Koordinaten aus
|*
\************************************************************************/

BOOL B3dComplexPolygon::ArePointsEqual(B3dEntity& rFirst,
    B3dEntity& rSecond)
{
    // Wenn der Punkt dem letzten gleich ist, gar nicht behandeln
    if(rFirst.Point().GetVector3D() == rSecond.Point().GetVector3D())
        return TRUE;
    return FALSE;
}

/*************************************************************************
|*
|* Alles auf Startzustand, buffer leeren
|*
\************************************************************************/

void B3dComplexPolygon::EmptyBuffers()
{
    aEntityBuffer.Erase();
    nNewPolyStart = 0;
    bOrientationValid = FALSE;
    bNormalValid = FALSE;

    // EdgeList und EdgeEntries leeren
    pEdgeList = NULL;
    aEdgeList.Erase();
    aEdgeEntry.Erase();
}

/*************************************************************************
|*
|* Neues Teilpolygon beginnen
|*
\************************************************************************/

void B3dComplexPolygon::StartPrimitive()
{
    // Bisherige Punkte verarbeiten
    if(aEntityBuffer.Count() > nNewPolyStart)
        ComputeLastPolygon();

    // Zeiger auf letzten Punkt loeschen
    pLastVertex = NULL;

    // Hoechten Punkt vergesset
    nHighestEdge = 0L;
}

/*************************************************************************
|*
|* Teilpolygon abschliessen
|*
\************************************************************************/

void B3dComplexPolygon::ComputeLastPolygon(BOOL bIsLast)
{
    // Letzten Punkt mit erstem vergleichen, evtl
    // wegschmeissen
    if(pLastVertex)
    {
        if(ArePointsEqual(aEntityBuffer[nNewPolyStart], *pLastVertex))
        {
            // HighestEdge korrigieren, falls dieser geloescht werden soll
            if(nHighestEdge && nHighestEdge == aEntityBuffer.Count())
                nHighestEdge = nNewPolyStart + 1;

            aEntityBuffer.Remove();
        }
    }

    // Sind noch genug Punkte da?
    if(aEntityBuffer.Count() < nNewPolyStart + 3)
    {
        // Geometrie ausgeben, obwohl zuwenig Punkte fuer ein Polygon
        if(pBase3D)
        {
            pBase3D->StartPrimitive(Base3DPolygon);
            for(UINT32 a=0; a < aEntityBuffer.Count(); a++)
            {
                pBase3D->SetEdgeFlag(aEntityBuffer[a].IsEdgeVisible());
                pBase3D->AddVertex(aEntityBuffer[a]);
            }
            pBase3D->EndPrimitive();
        }
        else if(pGeometry)
        {
            pGeometry->StartComplexPrimitive();
            for(UINT32 a=0; a < aEntityBuffer.Count(); a++)
                pGeometry->AddComplexVertex(aEntityBuffer[a], aEntityBuffer[a].IsEdgeVisible());
            pGeometry->EndComplexPrimitive();
        }
    }
    else
    {
        if(!nNewPolyStart && bIsLast && IsConvexPolygon())
        {
            // Falls das PolyPolygon nur aus einem Polygon besteht
            // und es Konvex ist, ist man fertig.
            // Um die Qualitaet zu verbessern, wird fuer
            // Polygone ab einer gewissen Punktzahl ein
            // abschliessender Mittelpunkt generiert.
            if(pBase3D)
            {
                pBase3D->StartPrimitive(Base3DPolygon);
                if(aEntityBuffer.Count() > 4)
                {
                    B3dEntity aNew;
                    aNew.CalcMiddle(aEntityBuffer[0], aEntityBuffer[aEntityBuffer.Count() / 2]);
                    pBase3D->SetEdgeFlag(FALSE);
                    pBase3D->AddVertex(aNew);
                    for(UINT32 a=0; a < aEntityBuffer.Count(); a++)
                    {
                        pBase3D->SetEdgeFlag(aEntityBuffer[a].IsEdgeVisible());
                        pBase3D->AddVertex(aEntityBuffer[a]);
                    }
                    pBase3D->SetEdgeFlag(FALSE);
                    pBase3D->AddVertex(aEntityBuffer[0]);
                }
                else
                {
                    for(UINT32 a=0; a < aEntityBuffer.Count(); a++)
                    {
                        pBase3D->SetEdgeFlag(aEntityBuffer[a].IsEdgeVisible());
                        pBase3D->AddVertex(aEntityBuffer[a]);
                    }
                }
                pBase3D->EndPrimitive();
            }
            else if(pGeometry)
            {
                pGeometry->StartComplexPrimitive();
                if(aEntityBuffer.Count() > 4)
                {
                    B3dEntity aNew;
                    aNew.CalcMiddle(aEntityBuffer[0], aEntityBuffer[aEntityBuffer.Count() / 2]);
                    pGeometry->AddComplexVertex(aNew, FALSE);
                    for(UINT32 a=0; a < aEntityBuffer.Count(); a++)
                        pGeometry->AddComplexVertex(aEntityBuffer[a], aEntityBuffer[a].IsEdgeVisible());
                    pGeometry->AddComplexVertex(aEntityBuffer[0], FALSE);
                }
                else
                {
                    for(UINT32 a=0; a < aEntityBuffer.Count(); a++)
                        pGeometry->AddComplexVertex(aEntityBuffer[a], aEntityBuffer[a].IsEdgeVisible());
                }
                pGeometry->EndComplexPrimitive();
            }
        }
        else
        {
            if(!bNormalValid)
                ChooseNormal();

            // Einsortieren
            UINT32 nUpperBound = aEntityBuffer.Count();

            // Als Polygon behandeln
            if(GetTestForCut())
            {
                UINT32 a;
                for(a=nNewPolyStart + 1; a < nUpperBound; a++)
                    AddEdgeCut(&aEntityBuffer[a-1], &aEntityBuffer[a]);

                // Polygon schliessen
                AddEdgeCut(&aEntityBuffer[a-1], &aEntityBuffer[nNewPolyStart]);
            }
            else
            {
                UINT32 a;
                for(a=nNewPolyStart + 1; a < nUpperBound; a++)
                    AddEdge(&aEntityBuffer[a-1], &aEntityBuffer[a]);

                // Polygon schliessen
                AddEdge(&aEntityBuffer[a-1], &aEntityBuffer[nNewPolyStart]);
            }

            // Hier setzen, da evtl. bereits neue Punkte
            // durch Schnitte hinzugekommen sind
            nNewPolyStart = aEntityBuffer.Count();
        }
    }
}

/*************************************************************************
|*
|* Orientierung des ersten Polygons ermitteln
|*
\************************************************************************/

void B3dComplexPolygon::ChooseNormal()
{
    if(nHighestEdge)
    {
        UINT32 nHigh = nHighestEdge - 1;
        UINT32 nPrev = (nHigh != 0) ? nHigh - 1 : aEntityBuffer.Count() - 1;
        UINT32 nNext = (nHigh + 1 != aEntityBuffer.Count()) ? nHigh + 1 : nNewPolyStart;

        // Punkt, Vorgaenger und Nachfolger holen
        const Vector3D& rHigh = aEntityBuffer[nHigh].Point().GetVector3D();
        const Vector3D& rPrev = aEntityBuffer[nPrev].Point().GetVector3D();
        const Vector3D& rNext = aEntityBuffer[nNext].Point().GetVector3D();

        // Normale bilden
        aNormal = (rPrev - rHigh)|(rNext - rHigh);
        if(aNormal != Vector3D())
            aNormal.Normalize();
        else
            aNormal = Vector3D(0.0, 0.0, -1.0);
    }
    bNormalValid = TRUE;
}

/*************************************************************************
|*
|* Komplexes Polygon ausgeben
|*
\************************************************************************/

void B3dComplexPolygon::EndPrimitive(Base3D* pB3D)
{
    // Funktionszeiger setzen
    pBase3D = pB3D;

    // Letztes angefangenes Poly verarbeiten
    ComputeLastPolygon(TRUE);

    // Wenn es Kanten gibt
    if(pEdgeList)
    {
        // Dreiecke generieren und ausgeben
        pBase3D->StartPrimitive(Base3DTriangles);
        while(pEdgeList)
            ExtractTriangle();
        pBase3D->EndPrimitive();
    }

    // Buffer leeren
    EmptyBuffers();

    // Zeiger wieder loeschen
    pBase3D = NULL;
}

void B3dComplexPolygon::EndPrimitive(B3dGeometry *pGeom)
{
    // Funktionszeiger setzen
    pGeometry = pGeom;

    // Letztes angefangenes Poly verarbeiten
    ComputeLastPolygon(TRUE);

    // Dreiecke generieren und ausgeben
    while(pEdgeList)
        ExtractTriangle();

    // Buffer leeren
    EmptyBuffers();

    // Zeiger wieder loeschen
    pGeometry = NULL;
}

/*************************************************************************
|*
|* Teste aktuelles Polygon (0..aEntityBuffer.Count()) auf Konvexitaet
|*
\************************************************************************/

BOOL B3dComplexPolygon::IsConvexPolygon()
{
    B3dEntity* pFirst = &aEntityBuffer[aEntityBuffer.Count() - 2];
    B3dEntity* pSecond = &aEntityBuffer[aEntityBuffer.Count() - 1];
    B3dEntity* pThird = &aEntityBuffer[0];
    BOOL bDirection = IsLeft(pSecond, pFirst, pThird);
    BOOL bOrder = CompareOrder(pSecond, pThird);
    UINT16 nDirChanges(0);

    for(UINT32 a = 1; nDirChanges <= 2 && a < aEntityBuffer.Count(); a++)
    {
        pFirst = pSecond;
        pSecond = pThird;
        pThird = &aEntityBuffer[a];

        if(IsLeft(pSecond, pFirst, pThird) != bDirection)
            return FALSE;

        if(CompareOrder(pSecond, pThird) != bOrder)
        {
            nDirChanges++;
            bOrder = !bOrder;
        }
    }
    // Zuviele aenderungen der Ordnung, auf keinen Fall Convex
    if(nDirChanges > 2)
        return FALSE;

    return TRUE;
}

/*************************************************************************
|*
|* Lexikografische Ordnung der beiden Punkte
|*
\************************************************************************/

BOOL B3dComplexPolygon::CompareOrder(B3dEntity* pFirst, B3dEntity* pSecond)
{
    if(pFirst->GetX() < pSecond->GetX())
        return FALSE;
    if(pFirst->GetX() > pSecond->GetX())
        return TRUE;
    if(pFirst->GetY() < pSecond->GetY())
        return FALSE;
    return TRUE;
}

/*************************************************************************
|*
|* Teste, ob die Punkte der Kante getauscht werden muessen
|*
\************************************************************************/

BOOL B3dComplexPolygon::DoSwap(B3dEntity* pStart, B3dEntity* pEnd)
{
    if(DOUBLE_EQUAL(pStart->GetY(), pEnd->GetY()))
    {
        if(pStart->GetX() > pEnd->GetX())
            return TRUE;
    }
    else
    {
        if(pStart->GetY() > pEnd->GetY())
            return TRUE;
    }
    return FALSE;
}

/*************************************************************************
|*
|* Kante nInd1, nInd2 zu Kantenliste hinzufuegen
|*
\************************************************************************/

B3dEdgeEntry* B3dComplexPolygon::AddEdge(B3dEntity* pStart, B3dEntity* pEnd)
{
    if(DoSwap(pStart, pEnd))
        return InsertEdge(GetList(pEnd), pStart, TRUE);
    return InsertEdge(GetList(pStart), pEnd, TRUE);
}

/*************************************************************************
|*
|* Einen Listeneintrag suchen oder neu anlegen
|* Liefert immer einen Listeneintrag zurueck
|*
\************************************************************************/

B3dEdgeList* B3dComplexPolygon::GetList(B3dEntity* pStart)
{
    B3dEdgeList* pList = pEdgeList;
    B3dEdgeList* pLast = NULL;

    while(pList && pList->GetStart() != pStart && DoSwap(pStart, pList->GetStart()))
    {
        pLast = pList;
        pList = pList->GetDown();
    }

    if(pList)
    {
        if(pList->GetStart() != pStart)
        {
            if(DOUBLE_NOT_EQUAL(pStart->GetX(), pList->GetXPos()) || DOUBLE_NOT_EQUAL(pStart->GetY(), pList->GetYPos()))
            {
                // Auf jeden Fall ein neuer Eintrag
                aEdgeList.Append();
                B3dEdgeList* pNewList = &aEdgeList[aEdgeList.Count() - 1];
                pNewList->Reset();
                pNewList->SetStart(pStart);

                // vor pList einhaengen
                // pLast KANN NULL SEIN!
                pNewList->SetDown(pList);
                pList->SetParent(pNewList);
                if(pLast)
                {
                    pNewList->SetParent(pLast);
                    pLast->SetDown(pNewList);
                }
                else
                {
                    pEdgeList = pNewList;
                }

                // Returnwert setzen
                pList = pNewList;
            }
            else
            {
                // pList->GetStart() != pStart, aber
                // die Koordinaten sind praktisch identisch!
                // Gib diese Liste zurueck, d.h.
                // tue gar nichts
            }
        }
    }
    else
    {
        // pLast->GetYPos() < pStart->GetY(),
        // Hinten anhaengen
        aEdgeList.Append();
        pList = &aEdgeList[aEdgeList.Count() - 1];
        pList->Reset();
        pList->SetStart(pStart);
        if(pLast)
        {
            pList->SetParent(pLast);
            pLast->SetDown(pList);
        }
        else
        {
            pEdgeList = pList;
        }
    }
    return pList;
}

/*************************************************************************
|*
|* Eine Kante in eine Kantenliste einsortieren
|* Die Kante wird dabei neu erzeugt
|*
\************************************************************************/

B3dEdgeEntry* B3dComplexPolygon::InsertEdge(B3dEdgeList* pList,
    B3dEntity* pEnd, BOOL bEdgeVisible)
{
    B3dEdgeEntry* pEntry = pList->GetEntries();

    // Immer ein neuer Eintrag
    aEdgeEntry.Append();
    B3dEdgeEntry* pNewEntry = &aEdgeEntry[aEdgeEntry.Count() - 1];
    pNewEntry->Reset();
    pNewEntry->SetEnd(pEnd);
    pNewEntry->SetParent(pList);
    pNewEntry->SetEdgeVisible(bEdgeVisible);

    if(pEntry)
    {
        B3dEdgeEntry* pLast = NULL;
        double fSlant = GetSlant(pNewEntry);
        while(pEntry
            && GetSlant(pEntry) < fSlant)
        {
            pLast = pEntry;
            pEntry = pEntry->GetRight();
        }

        if(pEntry)
        {
            // GetSlant(pEntry) < fSlant
            // GetSlant(pLast) >= fSlant
            // Neuen Eintrag hinter pLast einfuegen
            // pLast KANN NULL SEIN!
            pNewEntry->SetRight(pEntry);
            if(pLast)
            {
                pLast->SetRight(pNewEntry);
            }
            else
            {
                pList->SetEntries(pNewEntry);
            }
        }
        else
        {
            // GetSlant(pEntry) >= fSlant
            // Neuen Eintrag am Ende anhaengen
            pLast->SetRight(pNewEntry);
        }
    }
    else
    {
        pList->SetEntries(pNewEntry);
    }
    // Returnwert
    return pNewEntry;
}

/*************************************************************************
|*
|* Steigung der Kante liefern
|*
\************************************************************************/

double B3dComplexPolygon::GetSlant(B3dEdgeEntry* pEdge)
{
    double fDivisor = pEdge->GetYPos() - pEdge->GetParent()->GetYPos();
    if(fabs(fDivisor) < SMALL_DVALUE)
        return DBL_MAX;
    return (pEdge->GetXPos() - pEdge->GetParent()->GetXPos()) / fDivisor;
}

/*************************************************************************
|*
|* Auf Schnitt mit einer vorhandenen Kante testen
|*
\************************************************************************/

void B3dComplexPolygon::TestForCut(B3dEdgeEntry* pEntry)
{
    // pEntry: die bereits eingefuegte neue Kante, die mit allen
    // aelteren Kanten geschnitten werden soll
    B3dEdgeList* pList = pEdgeList;

    while(pList && DOUBLE_SMALLER(pList->GetYPos(), pEntry->GetYPos()))
    {
        // nur in Gruppen mit anderem Startpunkt suchen
        if(pList != pEntry->GetParent())
        {
            B3dEdgeEntry* pTestEntry = pList->GetEntries();

            while(pTestEntry)
            {
                if(DOUBLE_BIGGER(pTestEntry->GetYPos(), pEntry->GetParent()->GetYPos()))
                {
                    // es existiert eine vertikale Bereichsueberschneidung
                    // Min/Max fuer pEntry holen
                    double fXMin = pEntry->GetXPos();
                    double fXMax = pEntry->GetParent()->GetXPos();
                    if(fXMin > fXMax)
                    {
                        double fSwap = fXMin;
                        fXMin = fXMax;
                        fXMax = fSwap;
                    }

                    // Min/Max in X fuer Kandidat holen
                    double fTestXMin = pTestEntry->GetXPos();
                    double fTestXMax = pList->GetXPos();
                    if(fTestXMin > fTestXMax)
                    {
                        double fSwap = fTestXMin;
                        fTestXMin = fTestXMax;
                        fTestXMax = fSwap;
                    }

                    if(fTestXMin < fXMax && fTestXMax > fXMin)
                    {
                        // es existiert eine horizontale Bereichsueberschneidung
                        // ein Schnitt ist moeglich
                        double fCut = FindCut(pEntry, pTestEntry);

                        if(fCut != 0.0)
                        {
                            // Schnitt existiert! fCut ist aus dem Parameterbereich
                            // der ersten Kante, also pEntry. Neuen Punkt erzeugen.
                            B3dEntity& rNew = GetFreeEntity();
                            rNew.CalcInBetween(*pEntry->GetParent()->GetStart(), *pEntry->GetEnd(), fCut);

                            // Neuen Punkt und neue von diesem ausgehende Kanten erzeugen
                            B3dEdgeList* pNewPointList = GetList(&rNew);
                            B3dEdgeEntry* pEntry2 = InsertEdge(pNewPointList, pEntry->GetEnd(), pEntry->IsEdgeVisible());
                            InsertEdge(pNewPointList, pTestEntry->GetEnd(), pTestEntry->IsEdgeVisible());

                            // Beteiligte Entries kuerzen
                            pEntry->SetEnd(&rNew);
                            pTestEntry->SetEnd(&rNew);

                            // Das neue Ende von pEntry kann weitere Linien
                            // schneiden, also geht der test mit diesem weiter
                            TestForCut(pEntry2);

                            // Test mit gekuerztem pEntry fortsetzen
                        }
                    }
                }

                // naechster Entry
                pTestEntry = pTestEntry->GetRight();
            }
        }

        // naechste Liste
        pList = pList->GetDown();
    }
}

/*************************************************************************
|*
|* Berechne den Schnitt zwischen den beiden Kanten und gib den
|* Schnittpunkt im Parameterbereich der 1. Kante zurueck
|*
\************************************************************************/

double B3dComplexPolygon::FindCut(B3dEdgeEntry* pEdge1, B3dEdgeEntry* pEdge2)
{
    double fRetval = 0.0;
    double fDeltaEdge2Y = pEdge2->GetYPos() - pEdge2->GetParent()->GetYPos();
    double fDeltaEdge2X = pEdge2->GetXPos() - pEdge2->GetParent()->GetXPos();
    double fDeltaEdge1X = pEdge1->GetXPos() - pEdge1->GetParent()->GetXPos();
    double fDeltaEdge1Y = pEdge1->GetYPos() - pEdge1->GetParent()->GetYPos();

    // Dynamische Grenze fuer parallelitaet berechnen
    double fSmallValue = fabs((fDeltaEdge2Y + fDeltaEdge2X + fDeltaEdge1X + fDeltaEdge1Y) * (SMALL_DVALUE / 4.0));
    double fZwi = (fDeltaEdge1X * fDeltaEdge2Y) - (fDeltaEdge1Y * fDeltaEdge2X);

    if(fabs(fZwi) > fSmallValue)
    {
        fZwi = (fDeltaEdge2Y * (pEdge2->GetParent()->GetXPos() - pEdge1->GetParent()->GetXPos())
            + fDeltaEdge2X * (pEdge1->GetParent()->GetYPos() - pEdge2->GetParent()->GetYPos())) / fZwi;

        // Im Parameterbereich der ersten Kante (ohne Punkte) ?
        if(fZwi > fSmallValue && fZwi < 1.0 - fSmallValue)
        {
            // Schnitt liegt im Parameterbereich der ersten
            // Linie, aber auch in dem der zweiten?
            if(fabs(fDeltaEdge2X) > fSmallValue && fabs(fDeltaEdge2X) > fabs(fDeltaEdge2Y))
            {
                fDeltaEdge2Y = (pEdge1->GetParent()->GetXPos() + fZwi
                    * fDeltaEdge1X - pEdge2->GetParent()->GetXPos()) / fDeltaEdge2X;

                // Parameterbereich der zweiten schliesst Start/Ende mit ein!
                if(fDeltaEdge2Y > -fSmallValue && fDeltaEdge2Y < 1.0 + fSmallValue)
                {
                    // Ja. Zuweisen.
                    fRetval = fZwi;
                }
            }
            else if(fabs(fDeltaEdge2Y) > fSmallValue)
            {
                fDeltaEdge2X = (pEdge1->GetParent()->GetYPos() + fZwi
                    * fDeltaEdge1Y - pEdge2->GetParent()->GetYPos()) / fDeltaEdge2Y;

                // Parameterbereich der zweiten schliesst Start/Ende mit ein!
                if(fDeltaEdge2X > -fSmallValue && fDeltaEdge2X < 1.0 + fSmallValue)
                {
                    // Ja. Zuweisen.
                    fRetval = fZwi;
                }
            }
        }
    }
    return fRetval;
}

/*************************************************************************
|*
|* Testet, ob die angegebene Kante schon existiert
|* Ja: Entfernen
|* Nein: Einfuegen
|*
\************************************************************************/

BOOL B3dComplexPolygon::SwitchEdgeExistance(B3dEntity* pStart,
    B3dEntity* pEnd)
{
    if(DoSwap(pStart, pEnd))
    {
        B3dEntity* pZwi = pStart;
        pStart = pEnd;
        pEnd = pZwi;
    }

    if(pEdgeList)
    {
        // Suchen
        B3dEdgeList* pList = pEdgeList;
        while(pList && pList->GetStart() != pStart)
            pList = pList->GetDown();

        if(pList && pList->GetStart() == pStart)
        {
            // Liste gefunden, Eintrag mit Endpunkt
            // pEnd finden
            B3dEdgeEntry* pEntry = pList->GetEntries();
            B3dEdgeEntry* pLeft = NULL;

            while(pEntry)
            {
                if(pEntry->GetEnd() == pEnd)
                {
                    // Kante existiert, austragen
                    // Liste ist pList
                    // Links ist pLeft
                    if(pLeft)
                    {
                        pLeft->SetRight(pEntry->GetRight());
                    }
                    else
                    {
                        if(pEntry->GetRight())
                            pList->SetEntries(pEntry->GetRight());
                        else
                            RemoveEdgeList(pList);
                    }
                    // fertig
                    return TRUE;
                }

                // naechste Kante
                pLeft = pEntry;
                pEntry = pEntry->GetRight();
            }

            // Liste existiert, aber der EdgeEintrag nicht.
            // Fuege diesen hinzu
            InsertEdge(pList, pEnd, FALSE);

            // fertig
            return FALSE;
        }
    }
    // Liste und Eintrag existieren nicht
    // Erzeuge beides
    InsertEdge(GetList(pStart), pEnd, FALSE);

    return FALSE;
}

/*************************************************************************
|*
|* Entferne die Kante aus der Kantenliste. Tue alles weitere,
|* um die Struktur weiter aufzuloesen
|*
\************************************************************************/

void B3dComplexPolygon::RemoveFirstEdge(B3dEdgeList* pList)
{
    if(pList->GetEntries()->GetRight())
        pList->SetEntries(pList->GetEntries()->GetRight());
    else
        RemoveEdgeList(pList);
}

/*************************************************************************
|*
|* Entferne die Kantenliste. Tue alles weitere,
|* um die Struktur weiter aufzuloesen
|*
\************************************************************************/

void B3dComplexPolygon::RemoveEdgeList(B3dEdgeList* pList)
{
    if(pList->GetDown())
        pList->GetDown()->SetParent(pList->GetParent());
    if(pList->GetParent())
        pList->GetParent()->SetDown(pList->GetDown());
    else
    {
        // Es gibt keinen parent mehr
        pEdgeList = pList->GetDown();
    }
}

/*************************************************************************
|*
|* Extrahiere das naechste Dreieck aus der Kantenliste
|* und zeichne es
|*
\************************************************************************/

void B3dComplexPolygon::ExtractTriangle()
{
    B3dEdgeEntry* pLeft = pEdgeList->GetEntries();
    B3dEdgeEntry* pRight = pLeft->GetRight();

    if(!pRight)
    {
//      DBG_ASSERT(0, "AW: Einzelne Kante als Startpunkt!");
        RemoveFirstEdge(pEdgeList);
        return;
    }

    B3dEdgeList* pList = FindStartInTriangle();
    BOOL bNotAllAligned = (fabs(GetSlant(pLeft) - GetSlant(pRight)) > SMALL_DVALUE);
    BOOL bStartIsEdgePoint = FALSE;
    if(pList)
    {
        const Vector3D& rListStart = pList->GetStart()->Point().GetVector3D();
        if((rListStart - pEdgeList->GetStart()->Point().GetVector3D()).GetLength() < SMALL_DVALUE)
            bStartIsEdgePoint = TRUE;
        else if((rListStart - pLeft->GetEnd()->Point().GetVector3D()).GetLength() < SMALL_DVALUE)
            bStartIsEdgePoint = TRUE;
        else if((rListStart - pRight->GetEnd()->Point().GetVector3D()).GetLength() < SMALL_DVALUE)
            bStartIsEdgePoint = TRUE;
    }

    if(pList && bNotAllAligned && !bStartIsEdgePoint)
    {
        // Zerlegen in 2 Teildreiecke
        // Erstes Teildreieck
        InsertEdge(pEdgeList, pList->GetStart(), FALSE);
        ExtractTriangle();

        // Zweites Teildreieck
        InsertEdge(pEdgeList, pList->GetStart(), FALSE);
        ExtractTriangle();
    }
    else
    {
        B3dEntity* pEntLeft = pLeft->GetEnd();
        B3dEntity* pEntRight = pRight->GetEnd();
        B3dEntity* pEntTop = pEdgeList->GetStart();
        BOOL bLeftVisible = pLeft->IsEdgeVisible();
        BOOL bRightVisible = pRight->IsEdgeVisible();

        RemoveFirstEdge(pEdgeList);
        RemoveFirstEdge(pEdgeList);

        if(pEntLeft != pEntRight)
        {
            // Merken, ob die Abschlusslinie existiert hat oder nicht
            BOOL bDidEdgeExist = SwitchEdgeExistance(pEntLeft, pEntRight);

            if(DOUBLE_NOT_EQUAL(pEntLeft->GetY(), pEntTop->GetY())
                || DOUBLE_NOT_EQUAL(pEntRight->GetY(), pEntTop->GetY()))
            {
                if(!bOrientationValid)
                {
                    // Anhand des ersten Dreiecks entscheiden,
                    // in welcher Orientierung die Dreiecke
                    // auszugeben sind
                    Vector3D aTmpNormal =
                        (pEntLeft->Point().GetVector3D() - pEntTop->Point().GetVector3D())
                        |(pEntRight->Point().GetVector3D() - pEntTop->Point().GetVector3D());

                    bOrientation = (aNormal.Scalar(aTmpNormal) > 0.0) ? TRUE : FALSE;
                    bOrientationValid = TRUE;
                }

                // Dreieck ausgeben
                if(pBase3D)
                {
                    if(bOrientation)
                    {
                        // Rechtsrum
                        pBase3D->SetEdgeFlag(bRightVisible);
                        pBase3D->AddVertex(*pEntTop);
                        pBase3D->SetEdgeFlag(bDidEdgeExist);
                        pBase3D->AddVertex(*pEntRight);
                        pBase3D->SetEdgeFlag(bLeftVisible);
                        pBase3D->AddVertex(*pEntLeft);
                    }
                    else
                    {
                        // Linksrum
                        pBase3D->SetEdgeFlag(bLeftVisible);
                        pBase3D->AddVertex(*pEntTop);
                        pBase3D->SetEdgeFlag(bDidEdgeExist);
                        pBase3D->AddVertex(*pEntLeft);
                        pBase3D->SetEdgeFlag(bRightVisible);
                        pBase3D->AddVertex(*pEntRight);
                    }
                }
                else if(pGeometry)
                {
                    pGeometry->StartComplexPrimitive();
                    if(bOrientation)
                    {
                        // Rechtsrum
                        pGeometry->AddComplexVertex(*pEntTop, bRightVisible);
                        pGeometry->AddComplexVertex(*pEntRight, bDidEdgeExist);
                        pGeometry->AddComplexVertex(*pEntLeft, bLeftVisible);
                    }
                    else
                    {
                        // Linksrum
                        pGeometry->AddComplexVertex(*pEntTop, bLeftVisible);
                        pGeometry->AddComplexVertex(*pEntLeft, bDidEdgeExist);
                        pGeometry->AddComplexVertex(*pEntRight, bRightVisible);
                    }
                    pGeometry->EndComplexPrimitive();
                }
            }
        }
    }
}

/*************************************************************************
|*
|* Suche nach einem fremden Startpunkt innerhalb des zu zeichnenden
|* naechsten Dreiecks
|*
\************************************************************************/

B3dEdgeList* B3dComplexPolygon::FindStartInTriangle()
{
    B3dEdgeList* pList = pEdgeList->GetDown();
    if(pList)
    {
        B3dEdgeEntry* pLeft = pEdgeList->GetEntries();
        B3dEdgeEntry* pRight = pLeft->GetRight();

        double fYMax = pLeft->GetYPos();
        double fZwi = pRight->GetYPos();
        if(fZwi > fYMax)
            fYMax = fZwi;

        if(pList->GetYPos() <= fYMax)
        {
            B3dEntity* pTop = pEdgeList->GetStart();
            double fXMin = pLeft->GetXPos();
            double fXMax = pRight->GetXPos();
            if(fXMin > fXMax)
            {
                fZwi = fXMin;
                fXMin = fXMax;
                fXMax = fZwi;
            }

            double fXTop = pTop->GetX();
            if(fXMin > fXTop)
                fXMin = fXTop;
            if(fXMax < fXTop)
                fXMax = fXTop;

            while(pList
                && pList->GetYPos() <= fYMax)
            {
                if(pList->GetXPos() > fXMin && pList->GetXPos() < fXMax)
                {
                    if(pList->GetStart() != pLeft->GetEnd()
                        && pList->GetStart() != pRight->GetEnd())
                    {
                        if(IsLeft(pTop, pLeft->GetEnd(), pList->GetStart()))
                        {
                            if(DOUBLE_NOT_EQUAL(pList->GetXPos(), pLeft->GetXPos())
                                || DOUBLE_NOT_EQUAL(pList->GetYPos(), pLeft->GetYPos()))
                            {
                                if(IsLeft(pRight->GetEnd(), pTop, pList->GetStart()))
                                {
                                    if(DOUBLE_NOT_EQUAL(pList->GetXPos(), pRight->GetXPos())
                                        || DOUBLE_NOT_EQUAL(pList->GetYPos(), pRight->GetYPos()))
                                    {
                                        if(IsLeft(pLeft->GetEnd(), pRight->GetEnd(),
                                            pList->GetStart()))
                                        {
                                            return pList;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                // naechste Liste
                pList = pList->GetDown();
            }
        }
    }
    return NULL;
}

/*************************************************************************
|*
|* Testen, auf welcher Seite pPoint von der Linie pTop, pDirection liegt
|*
\************************************************************************/

BOOL B3dComplexPolygon::IsLeft(B3dEntity* pTop, B3dEntity* pDirection,
    B3dEntity* pPoint)
{
    double fDirX = pDirection->GetX() - pTop->GetX();
    double fDirY = pDirection->GetY() - pTop->GetY();
    double fPntX = pPoint->GetX() - pTop->GetX();
    double fPntY = pPoint->GetY() - pTop->GetY();

    return ((fDirX * fPntY - fDirY * fPntX) <= 0.0);
}

