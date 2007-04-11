/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svdmark.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:21:43 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SVDMARK_HXX
#define _SVDMARK_HXX

#ifndef _CONTNR_HXX //autogen
#include <tools/contnr.hxx>
#endif

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

#ifndef _LIST_HXX //autogen
#include <tools/list.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

#ifndef _SDR_OBJECTUSER_HXX
#include <svx/sdrobjectuser.hxx>
#endif

class Rectangle;
class SdrPage;
class SdrObjList;
class SdrObject;
class SdrPageView;

// Ein Container fuer USHORTs (im Prinzip ein dynamisches Array)
class SdrUShortCont
{
    Container                                           maArray;
    sal_Bool                                            mbSorted;

private:
    void CheckSort(sal_uInt32 nPos);

public:
    SdrUShortCont(sal_uInt16 nBlock, sal_uInt16 nInit, sal_uInt16 nResize)
    :   maArray(nBlock, nInit, nResize),
    mbSorted(sal_True)
    {}

    SdrUShortCont(const SdrUShortCont& rCont)
    :   maArray(rCont.maArray),
        mbSorted(rCont.mbSorted)
    {}

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

    void Insert(sal_uInt16 nElem, sal_uInt32 nPos = CONTAINER_APPEND)
    {
        maArray.Insert((void*)sal_uInt32(nElem),nPos);

        if(mbSorted)
        {
            CheckSort(nPos);
        }
    }

    void Remove(sal_uInt32 nPos)
    {
        maArray.Remove(nPos);
    }

    void Replace(sal_uInt16 nElem, sal_uInt32 nPos)
    {
        maArray.Replace((void*)sal_uInt32(nElem), nPos);

        if(mbSorted)
        {
            CheckSort(nPos);
        }
    }

    sal_uInt16 GetObject(sal_uInt32 nPos) const
    {
        return sal_uInt16(sal_uIntPtr(maArray.GetObject(nPos)));
    }

    sal_uInt32 GetPos(sal_uInt16 nElem) const
    {
        return maArray.GetPos((void*)(sal_uInt32)nElem);
    }

    sal_uInt32 GetCount() const
    {
        return maArray.Count();
    }

    sal_Bool Exist(sal_uInt16 nElem) const
    {
        return (CONTAINER_ENTRY_NOTFOUND != maArray.GetPos((void*)(sal_uInt32)nElem));
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

    sal_uInt32 GetMarkCount() const
    {
        return maList.Count();
    }

    SdrMark* GetMark(sal_uInt32 nNum) const
    {
        return (SdrMark*)(maList.GetObject(nNum));
    }

    sal_uInt32 FindObject(const SdrObject* pObj) const;
    void InsertEntry(const SdrMark& rMark, sal_Bool bChkSort = sal_True);
    void DeleteMark(sal_uInt32 nNum);
    void ReplaceMark(const SdrMark& rNewMark, sal_uInt32 nNum);
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
