/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SVDMARK_HXX
#define _SVDMARK_HXX

#include <tools/contnr.hxx>
#include <tools/string.hxx>
#include <tools/list.hxx>
#include "svx/svxdllapi.h"
#include <svx/sdrobjectuser.hxx>

#include <set>

class Rectangle;
class SdrPage;
class SdrObjList;
class SdrObject;
class SdrPageView;

// Ein Container fuer USHORTs (im Prinzip ein dynamisches Array)
class SVX_DLLPUBLIC SdrUShortCont
{
    Container                                           maArray;
    sal_Bool                                            mbSorted;

private:
    void CheckSort(sal_uLong nPos);

public:
    SdrUShortCont(sal_uInt16 nBlock, sal_uInt16 nInit, sal_uInt16 nResize)
    :   maArray(nBlock, nInit, nResize),
    mbSorted(sal_True)
    {}

    SdrUShortCont(const SdrUShortCont& rCont)
    :   maArray(rCont.maArray),
        mbSorted(rCont.mbSorted)
    {}

    /** helper to migrate to stl containers */
    std::set< sal_uInt16 > getContainer();

    SdrUShortCont& operator=(const SdrUShortCont& rCont)
    {
        maArray = rCont.maArray;
        mbSorted = rCont.mbSorted;
        return *this;
    }

    sal_Bool operator==(const SdrUShortCont& rCont) const
    {
        return maArray == rCont.maArray;
    }

    sal_Bool operator!=(const SdrUShortCont& rCont) const
    {
        return maArray != rCont.maArray;
    }

    void Clear()
    {
        maArray.Clear();
        mbSorted = sal_True;
    }

    void Sort() const;

    void ForceSort() const
    {
        if(!mbSorted)
        {
            Sort();
        }
    }

    void Insert(sal_uInt16 nElem, sal_uLong nPos = CONTAINER_APPEND)
    {
        maArray.Insert((void*)sal_uLong(nElem),nPos);

        if(mbSorted)
        {
            CheckSort(nPos);
        }
    }

    void Remove(sal_uLong nPos)
    {
        maArray.Remove(nPos);
    }

    void Replace(sal_uInt16 nElem, sal_uLong nPos)
    {
        maArray.Replace((void*)sal_uLong(nElem), nPos);

        if(mbSorted)
        {
            CheckSort(nPos);
        }
    }

    sal_uInt16 GetObject(sal_uLong nPos) const
    {
        return sal_uInt16(sal_uIntPtr(maArray.GetObject(nPos)));
    }

    sal_uLong GetPos(sal_uInt16 nElem) const
    {
        return maArray.GetPos((void*)(sal_uLong)nElem);
    }

    sal_uLong GetCount() const
    {
        return maArray.Count();
    }

    sal_Bool Exist(sal_uInt16 nElem) const
    {
        return (CONTAINER_ENTRY_NOTFOUND != maArray.GetPos((void*)(sal_uLong)nElem));
    }
};

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
    SdrMark(SdrObject* pNewObj = 0L, SdrPageView* pNewPageView = 0L);
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

    SdrPage* GetPage() const;
    SdrObjList* GetObjList() const;
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
            mpPoints = new SdrUShortCont(1024, 32, 32);

        return mpPoints;
    }

    SdrUShortCont* ForceMarkedLines()
    {
        if(!mpLines)
            mpLines = new SdrUShortCont(1024, 32, 32);

        return mpLines;
    }

    SdrUShortCont* ForceMarkedGluePoints()
    {
        if(!mpGluePoints)
            mpGluePoints = new SdrUShortCont(1024, 32, 32);

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
        List                        maAllMarkedObjects;

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
        const List& GetAllMarkedObjects() const;

        SdrMarkList& GetMarkedObjectListWriteAccess()
        {
            return maMarkedObjectList;
        }
    };
} // end of namespace sdr

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SVDMARK_HXX
// eof
