/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef _SVDMARK_HXX
#define _SVDMARK_HXX

#include <rtl/ustring.hxx>
#include <svx/svxdllapi.h>
#include <svx/sdrobjectuser.hxx>
#include <tools/solar.h>

#include <set>
#include <vector>

class Rectangle;
class SdrPage;
class SdrObjList;
class SdrObject;
class SdrPageView;

typedef std::set<sal_uInt16> SdrUShortCont;


// Everything a View needs to know about a selected object
class SVX_DLLPUBLIC SdrMark : public sdr::ObjectUser
{
protected:
    SdrObject*                                          mpSelectedSdrObject; // the selected object
    SdrPageView*                                        mpPageView;
    SdrUShortCont*                                      mpPoints;     // Selected Points
    SdrUShortCont*                                      mpLines;      // Selected Line
    SdrUShortCont*                                      mpGluePoints; // Selected Gluepoints (their Id's)
    sal_Bool                                            mbCon1;       // for Connectors
    sal_Bool                                            mbCon2;       // for Connectors
    sal_uInt16                                          mnUser;       // E.g. for CopyObjects, also copy Edges

public:
    explicit SdrMark(SdrObject* pNewObj = 0L, SdrPageView* pNewPageView = 0L);
    SdrMark(const SdrMark& rMark);
    virtual ~SdrMark();

    // Derived from ObjectUser
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
    std::vector<SdrMark*>                               maList;

    OUString                                            maMarkName;
    OUString                                            maPointName;
    OUString                                            maGluePointName;

    sal_Bool                                            mbPointNameOk;
    sal_Bool                                            mbGluePointNameOk;
    sal_Bool                                            mbNameOk;
    sal_Bool                                            mbSorted;

private:
    SVX_DLLPRIVATE sal_Bool operator==(const SdrMarkList& rCmpMarkList) const;
    SVX_DLLPRIVATE void ImpForceSort();

private:
    SVX_DLLPRIVATE const OUString& GetPointMarkDescription(sal_Bool bGlue) const;

public:
    SdrMarkList()
    :   maList(),
        mbPointNameOk(sal_False),
        mbGluePointNameOk(sal_False),
        mbNameOk(sal_False),
        mbSorted(sal_True)
    {
    }

    SdrMarkList(const SdrMarkList& rLst)
    :   maList()
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
        return maList.size();
    }

    SdrMark* GetMark(sal_uLong nNum) const;
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

    // A verbal description of selected objects e.g.:
    // "27 Lines", "12 Objects", "Polygon" or even "Not an object"
    const OUString& GetMarkDescription() const;
    const OUString& GetPointMarkDescription() const
    {
        return GetPointMarkDescription(sal_False);
    }

    const OUString& GetGluePointMarkDescription() const
    {
        return GetPointMarkDescription(sal_True);
    }

    // pPage=0L: Selection of everything! Respect Pages
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
