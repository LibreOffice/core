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

#ifndef _SVDMARK_HXX
#define _SVDMARK_HXX

#include <tools/contnr.hxx>

#include <tools/string.hxx>
class Rectangle;
namespace binfilter {

class SdrPage;
class SdrObjList;
class SdrObject;
class SdrPageView;

// Ein Container fuer USHORTs (im Prinzip ein dynamisches Array)
class SdrUShortCont {
    Container aArr;
    FASTBOOL bSorted;
    FASTBOOL bDelDups;
private:
public:
    SdrUShortCont(USHORT nBlock, USHORT nInit, USHORT nResize): aArr(nBlock,nInit,nResize), bSorted(TRUE), bDelDups(TRUE) {}
    SdrUShortCont(const SdrUShortCont& rCont): aArr(rCont.aArr), bSorted(rCont.bSorted), bDelDups(rCont.bDelDups) {}
    SdrUShortCont& operator=(const SdrUShortCont& rCont)     { aArr=rCont.aArr; bSorted=rCont.bSorted; bDelDups=rCont.bDelDups; return *this; }
    FASTBOOL       operator==(const SdrUShortCont& rCont) const { return aArr==rCont.aArr; }
    FASTBOOL       operator!=(const SdrUShortCont& rCont) const { return aArr!=rCont.aArr; }
    void     Clear()                                           { aArr.Clear(); bSorted=TRUE; }
    void     Remove(ULONG nPos)                                { aArr.Remove(nPos); }
    USHORT   GetObject(ULONG nPos) const                       { return USHORT(ULONG(aArr.GetObject(nPos))); }
    ULONG    GetPos(USHORT nElem) const                        { return aArr.GetPos((void*)(ULONG)nElem); }
    ULONG    GetCount() const                                  { return aArr.Count(); }
    FASTBOOL Exist(USHORT nElem) const                         { return aArr.GetPos((void*)(ULONG)nElem)!=CONTAINER_ENTRY_NOTFOUND; }
};

class SdrMark { // Alles was eine View ueber ein markiertes Objekt wissen muss
protected:
    SdrObject*     pObj;
    SdrPageView*   pPageView;
    SdrUShortCont* pPoints;     // Markierte Punkte
    SdrUShortCont* pLines;      // Markierte Linienabschnitte
    SdrUShortCont* pGluePoints; // Markierte Klebepunkte (deren Id's)
    FASTBOOL       bCon1;       // fuer Connectoren
    FASTBOOL       bCon2;       // fuer Connectoren
    USHORT         nUser;       // z.B. fuer CopyObjects, mitkopieren von Edges
public:
    SdrMark(SdrObject* pNewObj=NULL, SdrPageView* pNewPageView=NULL): pObj(pNewObj), pPageView(pNewPageView), pPoints(NULL), pLines(NULL), pGluePoints(NULL), bCon1(FALSE), bCon2(FALSE), nUser(0) {}
    SdrMark(const SdrMark& rMark): pObj(NULL), pPageView(NULL), pPoints(NULL), pLines(NULL), pGluePoints(NULL), bCon1(FALSE), bCon2(FALSE), nUser(0) { *this=rMark; }
    ~SdrMark()                                          { if (pPoints!=NULL) delete pPoints; if (pLines!=NULL) delete pLines; if (pGluePoints!=NULL) delete pGluePoints; }
    SdrObject*   GetObj() const                         { return pObj; }
    SdrPageView* GetPageView() const                    { return pPageView; }
    void         SetObj(SdrObject* pNewObj)             { pObj =pNewObj; }
    void         SetPageView(SdrPageView* pNewPageView) { pPageView=pNewPageView; }
    void         SetCon1(FASTBOOL bOn)                  { bCon1=bOn; }
    FASTBOOL     IsCon1() const                         { return bCon1; }
    void         SetCon2(FASTBOOL bOn)                  { bCon2=bOn; }
    FASTBOOL     IsCon2() const                         { return bCon2; }
    void         SetUser(USHORT nVal)                   { nUser=nVal; }
    USHORT       GetUser() const                        { return nUser; }
    const SdrUShortCont* GetMarkedPoints() const        { return pPoints;     }
    const SdrUShortCont* GetMarkedLines() const         { return pLines;      }
    const SdrUShortCont* GetMarkedGluePoints() const    { return pGluePoints; }
    SdrUShortCont*       GetMarkedPoints()              { return pPoints;     }
    SdrUShortCont*       GetMarkedLines()               { return pLines;      }
    SdrUShortCont*       GetMarkedGluePoints()          { return pGluePoints; }
    SdrUShortCont*       ForceMarkedPoints()            { if (pPoints==NULL) pPoints=new SdrUShortCont(1024,32,32); return pPoints; }
    SdrUShortCont*       ForceMarkedLines()             { if (pLines==NULL) pLines=new SdrUShortCont(1024,32,32); return pLines; }
    SdrUShortCont*       ForceMarkedGluePoints()        { if (pGluePoints==NULL) pGluePoints=new SdrUShortCont(1024,32,32); return pGluePoints; }
};

class SdrMarkList {
protected:
    Container aList;
    String    aMarkName;
    String    aPointName;
    String    aGluePointName;
    FASTBOOL  bPointNameOk;
    FASTBOOL  bGluePointNameOk;
    FASTBOOL  bNameOk;
    FASTBOOL  bSorted;
private:
    // Nicht implementiert:
    FASTBOOL operator==(const SdrMarkList& rCmpMarkList) const;
private:
    const XubString& GetPointMarkDescription(FASTBOOL bGlue) const;
protected:
public:
    SdrMarkList()                        : aList(1024,64,64),bPointNameOk(FALSE),bGluePointNameOk(FALSE),bNameOk(FALSE),bSorted(TRUE) { }
    SdrMarkList(const SdrMarkList& rLst) : aList(1024,64,64)        { *this=rLst; }
    ~SdrMarkList()                                                  { Clear(); }
    void     Clear();
    // Sicherstellen, das die Liste sortiert ist:
    void     ForceSort();
    void     SetUnsorted()                                           { bSorted=FALSE; }
    ULONG    GetMarkCount() const                                    { return aList.Count(); }
    SdrMark* GetMark(ULONG nNum) const                               { return (SdrMark*)(aList.GetObject(nNum)); }

    // FindObject() sucht nach einer Markierung eines SdrObject.
    // Die Methode ist nicht const, da ein ForceSort gerufen wird!
    // Rueckgabewert ist der Index der gefundenen Mark-Entry bzw.
    // CONTAINER_ENTRY_NOTFOUND.
    ULONG FindObject(const SdrObject* pObj);

    void  InsertEntry(const SdrMark& rMark, FASTBOOL bChkSort=TRUE);

    // bReverse kann einen Performancevorteil bringen, wenn sicher ist,
    // dass die SrcList genau verkehrt herum ist.

    // Alle Entrys einer bestimmten PageView rauswerfen inkl delete
    FASTBOOL  DeletePageView (const SdrPageView& rPV);
    // Alle Objekte einer Seiteansicht markieren

    void SetNameDirty()                                              { bNameOk=FALSE; bPointNameOk=FALSE; bGluePointNameOk=FALSE; }

    // Eine verbale Beschreibung der markierten Objekte z.B.:
    // "27 Linien", "12 Objekte", "Polygon" oder auch "Kein Objekt"
    const String& GetGluePointMarkDescription() const                { return GetPointMarkDescription(TRUE); }

    // Schliesslich kann sich eine Mehrfach-Markierung
    // ueber mehrere Seiten erstrecken ...
    // pPage=NULL: Die Markierungen aller! Seiten beruecksichtigen
    FASTBOOL TakeBoundRect(SdrPageView* pPageView, Rectangle& rRect) const;
    FASTBOOL TakeSnapRect(SdrPageView* pPageView, Rectangle& rRect) const;

    // Es werden saemtliche Entries kopiert!
    void operator=(const SdrMarkList& rLst);

};
// Die Klasse kopiert alle eingefuegten MarkEntrys auf den Heap. Die Verwendung
// ist z.B. wie folgt gedacht:
//     InsertEntry(SdrMark(pObj,pPage));

}//end of namespace binfilter
#endif //_SVDMARK_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
