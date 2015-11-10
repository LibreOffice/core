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

#ifndef INCLUDED_SVX_SVDMARK_HXX
#define INCLUDED_SVX_SVDMARK_HXX

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


/**
 * Everything a View needs to know about a selected object
 */
class SVX_DLLPUBLIC SdrMark : public sdr::ObjectUser
{
private:
    void setTime();
protected:
    sal_Int64                                           mnTimeStamp;
    SdrObject*                                          mpSelectedSdrObject; // the selected object
    SdrPageView*                                        mpPageView;
    SdrUShortCont*                                      mpPoints;     // Selected Points
    SdrUShortCont*                                      mpLines;      // Selected Line
    SdrUShortCont*                                      mpGluePoints; // Selected Gluepoints (their Id's)
    bool                                                mbCon1;       // for Connectors
    bool                                                mbCon2;       // for Connectors
    sal_uInt16                                          mnUser;       // E.g. for CopyObjects, also copy Edges

public:
    explicit SdrMark(SdrObject* pNewObj = nullptr, SdrPageView* pNewPageView = nullptr);
    SdrMark(const SdrMark& rMark);
    virtual ~SdrMark();

    // Derived from ObjectUser
    virtual void ObjectInDestruction(const SdrObject& rObject) override;

    SdrMark& operator=(const SdrMark& rMark);
    bool operator==(const SdrMark& rMark) const;
    bool operator!=(const SdrMark& rMark) const
    {
        return !(operator==(rMark));
    }

    void SetMarkedSdrObj(SdrObject* pNewObj);
    SdrObject* GetMarkedSdrObj() const { return mpSelectedSdrObject;}

    SdrPageView* GetPageView() const
    {
        return mpPageView;
    }

    void SetPageView(SdrPageView* pNewPageView)
    {
        mpPageView = pNewPageView;
    }

    void SetCon1(bool bOn)
    {
        mbCon1 = bOn;
    }

    bool IsCon1() const
    {
        return mbCon1;
    }

    void SetCon2(bool bOn)
    {
        mbCon2 = bOn;
    }

    bool IsCon2() const
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

    const SdrUShortCont* GetMarkedGluePoints() const
    {
        return mpGluePoints;
    }

    SdrUShortCont* GetMarkedPoints()
    {
        return mpPoints;
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

    SdrUShortCont* ForceMarkedGluePoints()
    {
        if(!mpGluePoints)
            mpGluePoints = new SdrUShortCont;

        return mpGluePoints;
    }

    sal_Int64 getTimeStamp() const
    {
        return mnTimeStamp;
    }
};

class SVX_DLLPUBLIC SdrMarkList
{
protected:
    std::vector<SdrMark*>                               maList;

    OUString                                            maMarkName;
    OUString                                            maPointName;
    OUString                                            maGluePointName;

    bool                                                mbPointNameOk;
    bool                                                mbGluePointNameOk;
    bool                                                mbNameOk;
    bool                                                mbSorted;

private:
    SVX_DLLPRIVATE bool operator==(const SdrMarkList& rCmpMarkList) const;
    SVX_DLLPRIVATE void ImpForceSort();

private:
    SVX_DLLPRIVATE const OUString& GetPointMarkDescription(bool bGlue) const;

public:
    SdrMarkList()
    :   maList(),
        mbPointNameOk(false),
        mbGluePointNameOk(false),
        mbNameOk(false),
        mbSorted(true)
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
        mbSorted = false;
    }

    size_t GetMarkCount() const
    {
        return maList.size();
    }

    SdrMark* GetMark(size_t nNum) const;
    // returns SAL_MAX_SIZE if not found
    size_t FindObject(const SdrObject* pObj) const;
    void InsertEntry(const SdrMark& rMark, bool bChkSort = true);
    void DeleteMark(size_t nNum);
    void ReplaceMark(const SdrMark& rNewMark, size_t nNum);
    void Merge(const SdrMarkList& rSrcList, bool bReverse = false);
    bool DeletePageView(const SdrPageView& rPV);
    bool InsertPageView(const SdrPageView& rPV);

    void SetNameDirty()
    {
        mbNameOk = false;
        mbPointNameOk = false;
        mbGluePointNameOk = false;
    }

    // A verbal description of selected objects e.g.:
    // "27 Lines", "12 Objects", "Polygon" or even "Not an object"
    const OUString& GetMarkDescription() const;
    const OUString& GetPointMarkDescription() const
    {
        return GetPointMarkDescription(false);
    }

    const OUString& GetGluePointMarkDescription() const
    {
        return GetPointMarkDescription(true);
    }

    // pPage=0L: Selection of everything! Respect Pages
    bool TakeBoundRect(SdrPageView* pPageView, Rectangle& rRect) const;
    bool TakeSnapRect(SdrPageView* pPageView, Rectangle& rRect) const;

    // All Entries are copied!
    void operator=(const SdrMarkList& rLst);
};


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
        bool                        mbEdgesOfMarkedNodesDirty : 1;

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

#endif // INCLUDED_SVX_SVDMARK_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
