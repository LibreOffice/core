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

#ifndef _B3D_B3DCOMPO_HXX
#define _B3D_B3DCOMPO_HXX

#include "b3dentty.hxx"

namespace binfilter {

/*************************************************************************
|*
|* Klasse fuer Kantenliste, horizontaler Teil
|*
\************************************************************************/

class B3dEdgeList;
//class Base3D;
class B3dGeometry;

class B3dEdgeEntry
{
private:
    // Verzeigerung
    B3dEdgeList*			pParent;
    B3dEdgeEntry*			pRight;
    B3dEntity*				pEnd;

    // Ist diese Kante Teil der urspruenglichen Geometrie
    // oder als Stuetzkante hinzugefuegt?
    unsigned				bEdgeVisible		: 1;

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

SV_DECL_VARARR(B3dEdgeEntryBucketMemArr, char*, 32, 32)
class B3dEdgeEntryBucket {
private:
B3dEdgeEntryBucketMemArr	aMemArray;
    UINT32			nMask;
    UINT32			nCount;
    INT16			nFreeMemArray;
    INT16			nActMemArray;
    UINT16			nFreeEntry;
    UINT16			nShift;
    UINT16			nBlockShift;
    UINT16			nEntriesPerArray;
    UINT16			nSlotSize;
    UINT16			nNext;
    UINT16			nMemArray;
public:
    B3dEdgeEntryBucket(UINT16 TheSize);
    /* Zu verwendende Groesse der Speicherarrays setzen */
    /* Bitte NUR verwenden, falls sich der Leerkonstruktor */
    /* nicht vermeiden laesst! Nicht nachtraeglich anwenden!  */
    void InitializeSize(UINT16 TheSize);
    /* Destruktor */
    ~B3dEdgeEntryBucket();
    /* Anhaengen und kopieren */
    BOOL Append(B3dEdgeEntry& rVec)
        { if(CareForSpace()) return ImplAppend(rVec); return FALSE; }
    /* nur neuen Eintrag anhaengen, ohne ausfuellen */
    BOOL Append()
        { if(CareForSpace()) return ImplAppend(); return FALSE; }
    /* leeren und Speicher freigeben */
    void Empty();
    /* leeren aber Speicher behalten */
    void Erase();
    B3dEdgeEntry& operator[] (UINT32 nPos);
    UINT32 Count() { return nCount; }
    UINT32 GetNumAllocated() { return aMemArray.Count() * nEntriesPerArray; }
    void operator=(const B3dEdgeEntryBucket&);
    UINT16 GetBlockShift() { return nBlockShift; }
    UINT16 GetSlotSize() { return nSlotSize; }
private:
    BOOL CareForSpace()
        { if(nFreeEntry == nEntriesPerArray)
        return ImplCareForSpace(); return TRUE; }
    BOOL ImplCareForSpace();
    /* Anhaengen und kopieren */
    BOOL ImplAppend(B3dEdgeEntry& rVec);
    /* nur neuen Eintrag anhaengen, ohne ausfuellen */
    BOOL ImplAppend();
};


/*************************************************************************
|*
|* Klasse fuer Kantenliste, vertikaler Teil
|*
\************************************************************************/

class B3dEdgeList
{
private:
    // Verzeigerung
    B3dEdgeList*			pParent;
    B3dEdgeList*			pDown;
    B3dEdgeEntry*			pEntries;
    B3dEntity*				pStart;

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

SV_DECL_VARARR(B3dEdgeListBucketMemArr, char*, 32, 32)
class B3dEdgeListBucket {
private:
B3dEdgeListBucketMemArr	aMemArray;
    UINT32			nMask;
    UINT32			nCount;
    INT16			nFreeMemArray;
    INT16			nActMemArray;
    UINT16			nFreeEntry;
    UINT16			nShift;
    UINT16			nBlockShift;
    UINT16			nEntriesPerArray;
    UINT16			nSlotSize;
    UINT16			nNext;
    UINT16			nMemArray;
public:
    B3dEdgeListBucket(UINT16 TheSize);
    /* Zu verwendende Groesse der Speicherarrays setzen */
    /* Bitte NUR verwenden, falls sich der Leerkonstruktor */
    /* nicht vermeiden laesst! Nicht nachtraeglich anwenden!  */
    void InitializeSize(UINT16 TheSize);
    /* Destruktor */
    ~B3dEdgeListBucket();
    /* Anhaengen und kopieren */
    BOOL Append(B3dEdgeList& rVec)
        { if(CareForSpace()) return ImplAppend(rVec); return FALSE; }
    /* nur neuen Eintrag anhaengen, ohne ausfuellen */
    BOOL Append()
        { if(CareForSpace()) return ImplAppend(); return FALSE; }
    /* leeren und Speicher freigeben */
    void Empty();
    /* leeren aber Speicher behalten */
    void Erase();
    B3dEdgeList& operator[] (UINT32 nPos);
    UINT32 Count() { return nCount; }
    UINT32 GetNumAllocated() { return aMemArray.Count() * nEntriesPerArray; }
    void operator=(const B3dEdgeListBucket&);
    UINT16 GetBlockShift() { return nBlockShift; }
    UINT16 GetSlotSize() { return nSlotSize; }
private:
    BOOL CareForSpace()
        { if(nFreeEntry == nEntriesPerArray)
        return ImplCareForSpace(); return TRUE; }
    BOOL ImplCareForSpace();
    /* Anhaengen und kopieren */
    BOOL ImplAppend(B3dEdgeList& rVec);
    /* nur neuen Eintrag anhaengen, ohne ausfuellen */
    BOOL ImplAppend();
};

/*************************************************************************
|*
|* Klasse fuer komplexe Polygone (PolyPolygone) und deren Triangulierung
|*
\************************************************************************/

class B3dComplexPolygon
{
private:
    // Buffer fuer Punkte
    B3dEntityBucket			aEntityBuffer;

    // Startpunkt der EdgeList
    B3dEdgeList*			pEdgeList;

    // Y-Richtung der Kantenliste
    B3dEdgeListBucket		aEdgeList;

    // X-Eintraege der Kantenliste
    B3dEdgeEntryBucket		aEdgeEntry;

    // Startpunkt neues Polygon
    ULONG					nNewPolyStart;

    // Index der in allen Belangen hoechsten Ecke
    // um 1 erhoeht!
    ULONG					nHighestEdge;

    // Zeiger auf den zuletzt hinzugefuegten Punkt
    B3dEntity*				pLastVertex;

    // Normale des bearbeiteten Polygons
    Vector3D				aNormal;

    // Zeiger auf Base3D und B3dGeometry
//	Base3D					*pBase3D;
    B3dGeometry				*pGeometry;

    // Flags
    unsigned				bOrientationValid	: 1;
    unsigned				bNormalValid		: 1;
    unsigned				bTestForCut			: 1;
    unsigned				bOrientation		: 1;

public:
    B3dComplexPolygon();

    void StartPrimitive();
//	void EndPrimitive(Base3D *pB3d);
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
}//end of namespace binfilter

#endif          // _B3D_B3DCOMPO_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
