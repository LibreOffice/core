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

#include <vector>

#include <tools/contnr.hxx>
#include <tools/string.hxx>
#include "svx/svxdllapi.h"
#include <svx/sdrobjectuser.hxx>

#include <set>

class Rectangle;
class SdrPage;
class SdrObjList;
class SdrObject;
class SdrPageView;

// Ein Container fuer USHORTs (im Prinzip ein dynamisches Array)
class SVX_DLLPUBLIC SdrUShortCont : public std::set<sal_uInt16> {};


// Alles was eine View ueber ein markiertes Objekt wissen muss
class SVX_DLLPUBLIC SdrMark : public sdr::ObjectUser
{
protected:
    SdrObject*                                          mpSelectedSdrObject;    // the seleceted object
    SdrPageView*                                        mpPageView;
    SdrUShortCont*                                      mpPoints;     // Markierte Punkte
    SdrUShortCont*                                      mpLines;      // Markierte Linienabschnitte
    SdrUShortCont*                                      mpGluePoints; // Markierte Klebepunkte (deren Id's)
    sal_Bool                                            mbCon1;       // fuer Connectoren
    sal_Bool                                            mbCon2;       // fuer Connectoren
    sal_uInt16                                          mnUser;       // z.B. fuer CopyObjects, mitkopieren von Edges

public:
    explicit SdrMark(SdrObject* pNewObj = 0L, SdrPageView* pNewPageView = 0L);
    SdrMark(const SdrMark& rMark);
    virtual ~SdrMark();

    // derived from ObjectUser
    virtual void ObjectInDestruction(const SdrObject& rObject);

    SdrMark& operator=(const SdrMark& rMark);
    sal_Bool operator==(const SdrMark& rMark) const;
    sal_Bool operator!=(const SdrMark& rMark) const
    {
        return !(operator==(rMark));
    }

    void SetMarkedSdrObj(SdrObject* pNewObj);
    SdrObject* GetMarkedSdrObj() const;

    SdrPageView* GetPageView() const
    {
        return mpPageView;
    }

    void SetPageView(SdrPageView* pNewPageView)
    {
        mpPageView = pNewPageView;
    }

    void SetCon1(sal_Bool bOn)
    {
        mbCon1 = bOn;
    }

    sal_Bool IsCon1() const
    {
        return mbCon1;
    }

    void SetCon2(sal_Bool bOn)
    {
        mbCon2 = bOn;
    }

    sal_Bool IsCon2() const
    {
        return mbCon2;
    }

    void SetUser(sal_uInt16 nVal)
    {
        mnUser = nVal;
    }

    sal_uInt16 GetUser() const
    {
        return mnUser;
    }

    const SdrUShortCont* GetMarkedPoints() const
    {
        return mpPoints;
    }

    const SdrUShortCont* GetMarkedLines() const
    {
        return mpLines;
    }

    const SdrUShortCont* GetMarkedGluePoints() const
    {
        return mpGluePoints;
    }

    SdrUShortCont* GetMarkedPoints()
    {
        return mpPoints;
    }

    SdrUShortCont* GetMarkedLines()
    {
        return mpLines;
    }

    SdrUShortCont* GetMarkedGluePoints()
    {
        return mpGluePoints;
    }

    SdrUShortCont* ForceMarkedPoints()
    {
        if(!mpPoints)
            mpPoints = new SdrUShortCont;

        return mpPoints;
    }

    SdrUShortCont* ForceMarkedLines()
    {
        if(!mpLines)
            mpLines = new SdrUShortCont;

        return mpLines;
    }

    SdrUShortCont* ForceMarkedGluePoints()
    {
        if(!mpGluePoints)
            mpGluePoints = new SdrUShortCont;

        return mpGluePoints;
    }
};

class SVX_DLLPUBLIC SdrMarkList
{
protected:
    Container                                           maList;

    String                                              maMarkName;
    String                                              maPointName;
    String                                              maGluePointName;

    sal_Bool                                            mbPointNameOk;
    sal_Bool                                            mbGluePointNameOk;
    sal_Bool                                            mbNameOk;
    sal_Bool                                            mbSorted;

private:
    SVX_DLLPRIVATE sal_Bool operator==(const SdrMarkList& rCmpMarkList) const;
    SVX_DLLPRIVATE void ImpForceSort();

private:
    SVX_DLLPRIVATE const XubString& GetPointMarkDescription(sal_Bool bGlue) const;

public:
    SdrMarkList()
    :   maList(1024, 64, 64),
        mbPointNameOk(sal_False),
        mbGluePointNameOk(sal_False),
        mbNameOk(sal_False),
        mbSorted(sal_True)
    {
    }

    SdrMarkList(const SdrMarkList& rLst)
    :   maList(1024, 64, 64)
    {
        *this = rLst;
    }

    ~SdrMarkList()
    {
        Clear();
    }

    void Clear();
    void ForceSort() const;
    void SetUnsorted()
    {
        mbSorted = sal_False;
    }

    sal_uLong GetMarkCount() const
    {
        return maList.Count();
    }

    SdrMark* GetMark(sal_uLong nNum) const
    {
        return (SdrMark*)(maList.GetObject(nNum));
    }

    sal_uLong FindObject(const SdrObject* pObj) const;
    void InsertEntry(const SdrMark& rMark, sal_Bool bChkSort = sal_True);
    void DeleteMark(sal_uLong nNum);
    void ReplaceMark(const SdrMark& rNewMark, sal_uLong nNum);
    void Merge(const SdrMarkList& rSrcList, sal_Bool bReverse = sal_False);
    sal_Bool DeletePageView(const SdrPageView& rPV);
    sal_Bool InsertPageView(const SdrPageView& rPV);

    void SetNameDirty()
    {
        mbNameOk = sal_False;
        mbPointNameOk = sal_False;
        mbGluePointNameOk = sal_False;
    }

    // Eine verbale Beschreibung der markierten Objekte z.B.:
    // "27 Linien", "12 Objekte", "Polygon" oder auch "Kein Objekt"
    const String& GetMarkDescription() const;
    const String& GetPointMarkDescription() const
    {
        return GetPointMarkDescription(sal_False);
    }

    const String& GetGluePointMarkDescription() const
    {
        return GetPointMarkDescription(sal_True);
    }

    // pPage=0L: Die Markierungen aller! Seiten beruecksichtigen
    sal_Bool TakeBoundRect(SdrPageView* pPageView, Rectangle& rRect) const;
    sal_Bool TakeSnapRect(SdrPageView* pPageView, Rectangle& rRect) const;

    // Es werden saemtliche Entries kopiert!
    void operator=(const SdrMarkList& rLst);
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// migrate selections

namespace sdr
{
    class SVX_DLLPUBLIC ViewSelection
    {
        SdrMarkList                 maMarkedObjectList;
        SdrMarkList                 maEdgesOfMarkedNodes;
        SdrMarkList                 maMarkedEdgesOfMarkedNodes;
        std::vector<SdrObject*>     maAllMarkedObjects;

        // bitfield
        unsigned                    mbEdgesOfMarkedNodesDirty : 1;

        SVX_DLLPRIVATE void ImpForceEdgesOfMarkedNodes();
        SVX_DLLPRIVATE void ImplCollectCompleteSelection(SdrObject* pObj);

    public:
        ViewSelection();

        void SetEdgesOfMarkedNodesDirty();

        const SdrMarkList& GetMarkedObjectList() const
        {
            return maMarkedObjectList;
        }

        const SdrMarkList& GetEdgesOfMarkedNodes() const;
        const SdrMarkList& GetMarkedEdgesOfMarkedNodes() const;
        const std::vector<SdrObject*>& GetAllMarkedObjects() const;

        SdrMarkList& GetMarkedObjectListWriteAccess()
        {
            return maMarkedObjectList;
        }
    };
} // end of namespace sdr

#endif //_SVDMARK_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
