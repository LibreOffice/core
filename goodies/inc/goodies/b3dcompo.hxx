/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: b3dcompo.hxx,v $
 * $Revision: 1.3 $
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

#ifndef _B3D_B3DCOMPO_HXX
#define _B3D_B3DCOMPO_HXX

#include <goodies/b3dentty.hxx>

/*************************************************************************
|*
|* Klasse fuer Kantenliste, horizontaler Teil
|*
\************************************************************************/

class B3dEdgeList;
class Base3D;
class B3dGeometry;

class B3dEdgeEntry
{
private:
    // Verzeigerung
    B3dEdgeList*            pParent;
    B3dEdgeEntry*           pRight;
    B3dEntity*              pEnd;

    // Ist diese Kante Teil der urspruenglichen Geometrie
    // oder als Stuetzkante hinzugefuegt?
    unsigned                bEdgeVisible        : 1;

public:
    B3dEdgeEntry() {}

    void Reset()
    {
        pEnd = NULL;
        pParent = NULL;
        pRight = NULL;
    }

    void SetEnd(B3dEntity* pNew) { pEnd = pNew; }
    B3dEntity* GetEnd() { return pEnd; }
    double GetXPos() { return pEnd->GetX(); }
    double GetYPos() { return pEnd->GetY(); }

    void SetParent(B3dEdgeList* pNew) { pParent = pNew; }
    void SetRight(B3dEdgeEntry* pNew) { pRight = pNew; }

    B3dEdgeList* GetParent() { return pParent; }
    B3dEdgeEntry* GetRight() { return pRight; }

    void SetEdgeVisible(BOOL bNew) { bEdgeVisible = bNew; }
    BOOL IsEdgeVisible() { return bEdgeVisible; }

protected:
};

/*************************************************************************
|*
|* Bucket fuer Kantenliste, horizontaler Teil
|*
\************************************************************************/

BASE3D_DECL_BUCKET(B3dEdgeEntry, Bucket)

/*************************************************************************
|*
|* Klasse fuer Kantenliste, vertikaler Teil
|*
\************************************************************************/

class B3dEdgeList
{
private:
    // Verzeigerung
    B3dEdgeList*            pParent;
    B3dEdgeList*            pDown;
    B3dEdgeEntry*           pEntries;
    B3dEntity*              pStart;

public:
    B3dEdgeList() {}

    void Reset()
    {
        pParent = pDown = NULL;
        pEntries = NULL;
        pStart = NULL;
    }

    void SetParent(B3dEdgeList* pNew) { pParent = pNew; }
    void SetDown(B3dEdgeList* pNew) { pDown = pNew; }
    void SetEntries(B3dEdgeEntry* pNew) { pEntries = pNew; }
    void SetStart(B3dEntity* pNew) { pStart = pNew; }

    B3dEdgeList* GetParent() { return pParent; }
    B3dEdgeList* GetDown() { return pDown; }
    B3dEdgeEntry* GetEntries() { return pEntries; }
    B3dEntity* GetStart() { return pStart; }

    double GetXPos() { return pStart->GetX(); }
    double GetYPos() { return pStart->GetY(); }

protected:
};

/*************************************************************************
|*
|* Bucket fuer Kantenliste, vertikaler Teil
|*
\************************************************************************/

BASE3D_DECL_BUCKET(B3dEdgeList, Bucket)

/*************************************************************************
|*
|* Klasse fuer komplexe Polygone (PolyPolygone) und deren Triangulierung
|*
\************************************************************************/

class B3dComplexPolygon
{
private:
    // Buffer fuer Punkte
    B3dEntityBucket         aEntityBuffer;

    // Startpunkt der EdgeList
    B3dEdgeList*            pEdgeList;

    // Y-Richtung der Kantenliste
    B3dEdgeListBucket       aEdgeList;

    // X-Eintraege der Kantenliste
    B3dEdgeEntryBucket      aEdgeEntry;

    // Startpunkt neues Polygon
    ULONG                   nNewPolyStart;

    // Index der in allen Belangen hoechsten Ecke
    // um 1 erhoeht!
    ULONG                   nHighestEdge;

    // Zeiger auf den zuletzt hinzugefuegten Punkt
    B3dEntity*              pLastVertex;

    // Normale des bearbeiteten Polygons
    basegfx::B3DVector  aNormal;

    // Zeiger auf Base3D und B3dGeometry
    Base3D                  *pBase3D;
    B3dGeometry             *pGeometry;

    // Flags
    unsigned                bOrientationValid   : 1;
    unsigned                bNormalValid        : 1;
    unsigned                bTestForCut         : 1;
    unsigned                bOrientation        : 1;

public:
    B3dComplexPolygon();

    void StartPrimitive();
    void EndPrimitive(Base3D *pB3d);
    void EndPrimitive(B3dGeometry *pGeom);
    void PostAddVertex(B3dEntity& rVertex);
    B3dEntity &GetFreeEntity();

    void EmptyBuffers();

    // Schnitte testen an/aus
    void SetTestForCut(BOOL bNew) { bTestForCut = bNew; }

protected:
    BOOL GetTestForCut() { return bTestForCut; }
    void ComputeLastPolygon(BOOL bIsLast=FALSE);
    B3dEdgeEntry* AddEdge(B3dEntity* pPoint1, B3dEntity* pPoint2);
    void TestForCut(B3dEdgeEntry* pEntry);
    void AddEdgeCut(B3dEntity* pStart, B3dEntity* pEnd)
        { TestForCut(AddEdge(pStart, pEnd)); }
    B3dEdgeList* GetList(B3dEntity* pStart);
    B3dEdgeEntry* InsertEdge(B3dEdgeList* pList, B3dEntity* pEnd, BOOL bEdgeVisible);
    double GetSlant(B3dEdgeEntry* pEdge);
    double FindCut(B3dEdgeEntry* pNewEntry, B3dEdgeEntry* pEntry);
    BOOL SwitchEdgeExistance(B3dEntity* pStart, B3dEntity* pEnd);
    void RemoveFirstEdge(B3dEdgeList* pList);
    void RemoveEdgeList(B3dEdgeList* pList);
    void ExtractTriangle();
    B3dEdgeList* FindStartInTriangle();
    BOOL ArePointsEqual(B3dEntity& rFirst, B3dEntity& rSecond);
    BOOL IsConvexPolygon();
    BOOL DoSwap(B3dEntity* pStart, B3dEntity* pEnd);
    BOOL IsLeft(B3dEntity* pTop, B3dEntity* pDirection,
        B3dEntity* pPoint);
    void ChooseNormal();
    BOOL CompareOrder(B3dEntity* pFirst, B3dEntity* pSecond);
    void TestHighestEdge(B3dEntity& rVertex);
};

#endif          // _B3D_B3DCOMPO_HXX
