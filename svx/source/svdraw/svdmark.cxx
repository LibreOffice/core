/*************************************************************************
 *
 *  $RCSfile: svdmark.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 14:46:24 $
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

////////////////////////////////////////////////////////////////////////////////////////////////////

#include "svdmark.hxx"
#include "svdetc.hxx"
#include "svdobj.hxx"
#include "svdpage.hxx"
#include "svditer.hxx"
#include "svdpagv.hxx"
#include "svdopath.hxx" // zur Abschaltung
#include "svdogrp.hxx"  // des Cache bei
#include "svdorect.hxx" // GetMarkDescription
#include "svdstr.hrc"   // Namen aus der Resource
#include "svdglob.hxx"  // StringCache

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _E3D_OBJ3D_HXX //autogen
#include <obj3d.hxx>
#endif

#ifndef _E3D_SCENE3D_HXX
#include <scene3d.hxx>
#endif

#ifndef _SFXBRDCST_HXX //autogen
#include <svtools/brdcst.hxx>
#endif

#ifndef _SVDOEDGE_HXX
#include <svdoedge.hxx>
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

class ImpSdrUShortContSorter: public ContainerSorter
{
public:
    ImpSdrUShortContSorter(Container& rNewCont)
    :   ContainerSorter(rNewCont)
    {}

    virtual int Compare(const void* pElem1, const void* pElem2) const;
};

int ImpSdrUShortContSorter::Compare(const void* pElem1, const void* pElem2) const
{
    sal_uInt16 n1((sal_uInt16)((sal_uInt32)pElem1));
    sal_uInt16 n2((sal_uInt16)((sal_uInt32)pElem2));

    return ((n1 < n2) ? (-1) : (n1 > n2) ? (1) : (0));
}

void SdrUShortCont::Sort() const
{
    ImpSdrUShortContSorter aSort(*((Container*)(&maArray)));
    aSort.DoSort();
    ((SdrUShortCont*)this)->mbSorted = sal_True;

    sal_uInt32 nNum(GetCount());

    if(nNum > 1)
    {
        nNum--;
        sal_uInt16 nVal0 = GetObject(nNum);

        while(nNum > 0)
        {
            nNum--;
            sal_uInt16 nVal1 = GetObject(nNum);

            if(nVal1 == nVal0)
            {
                ((SdrUShortCont*)this)->Remove(nNum);
            }

            nVal0 = nVal1;
        }
    }
}

void SdrUShortCont::CheckSort(sal_uInt32 nPos)
{
    sal_uInt32 nAnz(maArray.Count());

    if(nPos > nAnz)
        nPos = nAnz;

    sal_uInt16 nAktVal = GetObject(nPos);

    if(nPos > 0)
    {
        sal_uInt16 nPrevVal = GetObject(nPos - 1);

        if(nPrevVal >= nAktVal)
            mbSorted = sal_False;
    }

    if(nPos < nAnz - 1)
    {
        sal_uInt16 nNextVal = GetObject(nPos + 1);

        if(nNextVal <= nAktVal)
            mbSorted = sal_False;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrMark& SdrMark::operator=(const SdrMark& rMark)
{
    mpObj = rMark.mpObj;
    mpPageView = rMark.mpPageView;
    mbCon1 = rMark.mbCon1;
    mbCon2 = rMark.mbCon2;
    mnUser = rMark.mnUser;

    if(!rMark.mpPoints)
    {
        if(mpPoints)
        {
            delete mpPoints;
            mpPoints = 0L;
        }
    }
    else
    {
        if(!mpPoints)
        {
            mpPoints = new SdrUShortCont(*rMark.mpPoints);
        }
        else
        {
            *mpPoints = *rMark.mpPoints;
        }
    }

    if(!rMark.mpLines)
    {
        if(mpLines)
        {
            delete mpLines;
            mpLines = 0L;
        }
    }
    else
    {
        if(!mpLines)
        {
            mpLines = new SdrUShortCont(*rMark.mpLines);
        }
        else
        {
            *mpLines = *rMark.mpLines;
        }
    }

    if(!rMark.mpGluePoints)
    {
        if(mpGluePoints)
        {
            delete mpGluePoints;
            mpGluePoints = 0L;
        }
    }
    else
    {
        if(!mpGluePoints)
        {
            mpGluePoints = new SdrUShortCont(*rMark.mpGluePoints);
        }
        else
        {
            *mpGluePoints = *rMark.mpGluePoints;
        }
    }

    return *this;
}

sal_Bool SdrMark::operator==(const SdrMark& rMark) const
{
    sal_Bool bRet(mpObj == rMark.mpObj && mpPageView == rMark.mpPageView && mbCon1 == rMark.mbCon1 && mbCon2 == rMark.mbCon2 && mnUser == rMark.mnUser);

    if((mpPoints != 0L) != (rMark.mpPoints != 0L))
        bRet = sal_False;

    if((mpLines != 0L) != (rMark.mpLines != 0L))
        bRet = sal_False;

    if((mpGluePoints != 0L) != (rMark.mpGluePoints != 0L))
        bRet = sal_False;

    if(bRet && mpPoints && *mpPoints != *rMark.mpPoints)
        bRet = sal_False;

    if(bRet && mpLines && *mpLines != *rMark.mpLines)
        bRet = sal_False;

    if(bRet && mpGluePoints && *mpGluePoints != *rMark.mpGluePoints)
        bRet = sal_False;

    return bRet;
}

SdrPage* SdrMark::GetPage() const
{
    return (mpObj ? mpObj->GetPage() : 0L);
}

SdrObjList* SdrMark::GetObjList() const
{
    return (mpObj ? mpObj->GetObjList() : 0L);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

class ImpSdrMarkListSorter: public ContainerSorter
{
public:
    ImpSdrMarkListSorter(Container& rNewCont)
    :   ContainerSorter(rNewCont)
    {}

    virtual int Compare(const void* pElem1, const void* pElem2) const;
};

int ImpSdrMarkListSorter::Compare(const void* pElem1, const void* pElem2) const
{
    const SdrObject* pObj1 = ((SdrMark*)pElem1)->GetObj();
    const SdrObject* pObj2 = ((SdrMark*)pElem2)->GetObj();
    const SdrObjList* pOL1 = (pObj1) ? pObj1->GetObjList() : 0L;
    const SdrObjList* pOL2 = (pObj2) ? pObj2->GetObjList() : 0L;

    if(pOL1 == pOL2)
    {
        sal_uInt32 nObjOrd1((pObj1) ? pObj1->GetOrdNum() : 0);
        sal_uInt32 nObjOrd2((pObj2) ? pObj2->GetOrdNum() : 0);

        return (nObjOrd1 < nObjOrd2 ? -1 : 1);
    }
    else
    {
        return ((long)pOL1 < (long)pOL2) ? -1 : 1;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrMarkList::ForceSort() const
{
    if(!mbSorted)
    {
        ((SdrMarkList*)this)->ImpForceSort();
    }
}

void SdrMarkList::ImpForceSort()
{
    if(!mbSorted)
    {
        mbSorted = sal_True;
        sal_uInt32 nAnz = maList.Count();

        if(nAnz > 1)
        {
            ImpSdrMarkListSorter aSort(maList);
            aSort.DoSort();

            // und nun doppelte rauswerfen
            if(maList.Count() > 1)
            {
                SdrMark* pAkt = (SdrMark*)maList.Last();
                SdrMark* pCmp = (SdrMark*)maList.Prev();

                while(pCmp)
                {
                    if(pAkt->GetObj() == pCmp->GetObj())
                    {
                        // Con1/Con2 Merging
                        if(pCmp->IsCon1())
                            pAkt->SetCon1(sal_True);

                        if(pCmp->IsCon2())
                            pAkt->SetCon2(sal_True);

                        // pCmp loeschen.
                        maList.Remove();

                        delete pCmp;
                    }
                    else
                    {
                        pAkt = pCmp;
                    }

                    pCmp = (SdrMark*)maList.Prev();
                }
            }
        }
    }
}

void SdrMarkList::Clear()
{
    for(sal_uInt32 i(0L); i < GetMarkCount(); i++)
    {
        SdrMark* pMark = GetMark(i);
        delete pMark;
    }

    maList.Clear();
    SetNameDirty();
}

void SdrMarkList::operator=(const SdrMarkList& rLst)
{
    Clear();

    for(sal_uInt32 i(0L); i < rLst.GetMarkCount(); i++)
    {
        SdrMark* pMark = rLst.GetMark(i);
        SdrMark* pNeuMark = new SdrMark(*pMark);
        maList.Insert(pNeuMark, CONTAINER_APPEND);
    }

    maMarkName = rLst.maMarkName;
    mbNameOk = rLst.mbNameOk;
    maPointName = rLst.maPointName;
    mbPointNameOk = rLst.mbPointNameOk;
    maGluePointName = rLst.maGluePointName;
    mbGluePointNameOk = rLst.mbGluePointNameOk;
    mbSorted = rLst.mbSorted;
}

sal_uInt32 SdrMarkList::FindObject(const SdrObject* pObj) const
{
    // #109658#
    //
    // Since relying on OrdNums is not allowed for the selection because objects in the
    // selection may not be inserted in a list if they are e.g. modified ATM, i changed
    // this loop to just look if the object pointer is in the selection.
    //
    // Problem is that GetOrdNum() which is const, internally casts to non-const and
    // hardly sets the OrdNum member of the object (nOrdNum) to 0 (ZERO) if the object
    // is not inserted in a object list.
    // Since this may be by purpose and necessary somewhere else i decided that it is
    // less dangerous to change this method then changing SdrObject::GetOrdNum().
    if(pObj && maList.Count())
    {
        for(sal_uInt32 a(0L); a < maList.Count(); a++)
        {
            if(((SdrMark*)(maList.GetObject(a)))->GetObj() == pObj)
            {
                return a;
            }
        }
    }

    return CONTAINER_ENTRY_NOTFOUND;
}

void SdrMarkList::InsertEntry(const SdrMark& rMark, sal_Bool bChkSort)
{
    SetNameDirty();
    sal_uInt32 nAnz(maList.Count());

    if(!bChkSort || !mbSorted || nAnz == 0)
    {
        if(!bChkSort)
            mbSorted = sal_False;

        maList.Insert(new SdrMark(rMark), CONTAINER_APPEND);
    }
    else
    {
        SdrMark* pLast = GetMark(sal_uInt32(nAnz - 1));
        const SdrObject* pLastObj = pLast->GetObj();
        const SdrObject* pNeuObj = rMark.GetObj();

        if(pLastObj == pNeuObj)
        {
            // Aha, den gibt's schon
            // Con1/Con2 Merging
            if(rMark.IsCon1())
                pLast->SetCon1(sal_True);

            if(rMark.IsCon2())
                pLast->SetCon2(sal_True);
        }
        else
        {
            SdrMark* pKopie = new SdrMark(rMark);
            maList.Insert(pKopie, CONTAINER_APPEND);

            // und nun checken, ob die Sortierung noch ok ist
            const SdrObjList* pLastOL = pLastObj!=0L ? pLastObj->GetObjList() : 0L;
            const SdrObjList* pNeuOL = pNeuObj !=0L ? pNeuObj ->GetObjList() : 0L;

            if(pLastOL == pNeuOL)
            {
                sal_uInt32 nLastNum(pLastObj!=0L ? pLastObj->GetOrdNum() : 0);
                sal_uInt32 nNeuNum(pNeuObj !=0L ? pNeuObj ->GetOrdNum() : 0);

                if(nNeuNum < nLastNum)
                {
                    // irgendwann muss mal sortiert werden
                    mbSorted = sal_False;
                }
            }
            else
            {
                if((long)pNeuOL < (long)pLastOL)
                {
                    // irgendwann muss mal sortiert werden
                    mbSorted = sal_False;
                }
            }
        }
    }

    return;
}

void SdrMarkList::DeleteMark(sal_uInt32 nNum)
{
    SdrMark* pMark = GetMark(nNum);
    DBG_ASSERT(pMark!=0L,"DeleteMark: MarkEntry nicht gefunden");

    if(pMark)
    {
        maList.Remove(nNum);
        delete pMark;
        SetNameDirty();
    }
}

void SdrMarkList::ReplaceMark(const SdrMark& rNewMark, sal_uInt32 nNum)
{
    SdrMark* pMark = GetMark(nNum);
    DBG_ASSERT(pMark!=0L,"ReplaceMark: MarkEntry nicht gefunden");

    if(pMark)
    {
        delete pMark;
        SetNameDirty();
        SdrMark* pKopie = new SdrMark(rNewMark);
        maList.Replace(pKopie, nNum);
        mbSorted = sal_False;
    }
}

void SdrMarkList::Merge(const SdrMarkList& rSrcList, sal_Bool bReverse)
{
    sal_uInt32 nAnz(rSrcList.maList.Count());

    if(rSrcList.mbSorted)
    {
        // Merging ohne ein Sort bei rSrcList zu erzwingen
        bReverse = sal_False;
    }

    if(!bReverse)
    {
        for(sal_uInt32 i(0L); i < nAnz; i++)
        {
            SdrMark* pM = (SdrMark*)(rSrcList.maList.GetObject(i));
            InsertEntry(*pM);
        }
    }
    else
    {
        for(sal_uInt32 i(nAnz); i > 0;)
        {
            i--;
            SdrMark* pM = (SdrMark*)(rSrcList.maList.GetObject(i));
            InsertEntry(*pM);
        }
    }
}

sal_Bool SdrMarkList::DeletePageView(const SdrPageView& rPV)
{
    sal_Bool bChgd(sal_False);

    for(sal_uInt32 i(GetMarkCount()); i > 0; )
    {
        i--;
        SdrMark* pMark = GetMark(i);

        if(pMark->GetPageView()==&rPV)
        {
            maList.Remove(i);
            delete pMark;
            SetNameDirty();
            bChgd = sal_True;
        }
    }

    return bChgd;
}

sal_Bool SdrMarkList::InsertPageView(const SdrPageView& rPV)
{
    sal_Bool bChgd(sal_False);
    DeletePageView(rPV); // erstmal alle raus, dann die ganze Seite hinten dran
    SdrObject* pObj;
    const SdrObjList* pOL = rPV.GetObjList();
    sal_uInt32 nObjAnz(pOL->GetObjCount());

    for(sal_uInt32 nO(0L); nO < nObjAnz; nO++)
    {
        pObj = pOL->GetObj(nO);
        sal_Bool bDoIt(rPV.IsObjMarkable(pObj));

        if(bDoIt)
        {
            SdrMark* pM = new SdrMark(pObj, (SdrPageView*)&rPV);
            maList.Insert(pM, CONTAINER_APPEND);
            SetNameDirty();
            bChgd = sal_True;
        }
    }

    return bChgd;
}

const XubString& SdrMarkList::GetMarkDescription() const
{
    sal_uInt32 nAnz(GetMarkCount());

    if(mbNameOk && 1L == nAnz)
    {
        // Bei Einfachselektion nur Textrahmen cachen
        SdrObject* pObj = GetMark(0)->GetObj();
        SdrTextObj* pTextObj = PTR_CAST(SdrTextObj, pObj);

        if(!pTextObj || !pTextObj->IsTextFrame())
        {
            ((SdrMarkList*)(this))->mbNameOk = sal_False;
        }
    }

    if(!mbNameOk)
    {
        SdrMark* pMark = GetMark(0);
        XubString aNam;

        if(!nAnz)
        {
            ((SdrMarkList*)(this))->maMarkName = ImpGetResStr(STR_ObjNameNoObj);
        }
        else if(1L == nAnz)
        {
            pMark->GetObj()->TakeObjNameSingul(aNam);
        }
        else
        {
            pMark->GetObj()->TakeObjNamePlural(aNam);
            XubString aStr1;
            sal_Bool bEq(sal_True);

            for(sal_uInt32 i = 1; i < GetMarkCount() && bEq; i++)
            {
                SdrMark* pMark = GetMark(i);
                pMark->GetObj()->TakeObjNamePlural(aStr1);
                bEq = aNam.Equals(aStr1);
            }

            if(!bEq)
            {
                aNam = ImpGetResStr(STR_ObjNamePlural);
            }

            aNam.Insert(sal_Unicode(' '), 0);
            aNam.Insert(UniString::CreateFromInt32(nAnz), 0);
        }

        ((SdrMarkList*)(this))->maMarkName = aNam;
        ((SdrMarkList*)(this))->mbNameOk = sal_True;
    }

    return maMarkName;
}

const XubString& SdrMarkList::GetPointMarkDescription(sal_Bool bGlue) const
{
    sal_Bool& rNameOk = (sal_Bool&)(bGlue ? mbGluePointNameOk : mbPointNameOk);
    XubString& rName = (XubString&)(bGlue ? maGluePointName : maPointName);
    sal_uInt32 nMarkAnz(GetMarkCount());
    sal_uInt32 nMarkPtAnz(0L);
    sal_uInt32 nMarkPtObjAnz(0L);
    sal_uInt32 n1stMarkNum(0xFFFFFFFF);

    for(sal_uInt32 nMarkNum(0L); nMarkNum < nMarkAnz; nMarkNum++)
    {
        const SdrMark* pMark = GetMark(nMarkNum);
        const SdrUShortCont* pPts = bGlue ? pMark->GetMarkedGluePoints() : pMark->GetMarkedPoints();
        sal_uInt32 nAnz(pPts ? pPts->GetCount() : 0);

        if(nAnz)
        {
            if(n1stMarkNum == 0xFFFFFFFF)
            {
                n1stMarkNum = nMarkNum;
            }

            nMarkPtAnz += nAnz;
            nMarkPtObjAnz++;
        }

        if(nMarkPtObjAnz > 1 && rNameOk)
        {
            // vorzeitige Entscheidung
            return rName;
        }
    }

    if(rNameOk && 1L == nMarkPtObjAnz)
    {
        // Bei Einfachselektion nur Textrahmen cachen
        SdrObject* pObj = GetMark(0)->GetObj();
        SdrTextObj* pTextObj = PTR_CAST(SdrTextObj,pObj);

        if(!pTextObj || !pTextObj->IsTextFrame())
        {
            rNameOk = sal_False;
        }
    }

    if(!nMarkPtObjAnz)
    {
        rName.Erase();
        rNameOk = sal_True;
    }
    else if(!rNameOk)
    {
        const SdrMark* pMark = GetMark(n1stMarkNum);
        XubString aNam;

        if(1L == nMarkPtObjAnz)
        {
            pMark->GetObj()->TakeObjNameSingul(aNam);
        }
        else
        {
            pMark->GetObj()->TakeObjNamePlural(aNam);
            XubString aStr1;
            sal_Bool bEq(sal_True);

            for(sal_uInt32 i(n1stMarkNum + 1L); i < GetMarkCount() && bEq; i++)
            {
                const SdrMark* pMark = GetMark(i);
                const SdrUShortCont* pPts = bGlue ? pMark->GetMarkedGluePoints() : pMark->GetMarkedPoints();

                if(pPts && pPts->GetCount())
                {
                    pMark->GetObj()->TakeObjNamePlural(aStr1);
                    bEq = aNam.Equals(aStr1);
                }
            }

            if(!bEq)
            {
                aNam = ImpGetResStr(STR_ObjNamePlural);
            }

            aNam.Insert(sal_Unicode(' '), 0);
            aNam.Insert(UniString::CreateFromInt32(nMarkPtObjAnz), 0);
        }

        XubString aStr1;

        if(1L == nMarkPtAnz)
        {
            aStr1 = (ImpGetResStr(bGlue ? STR_ViewMarkedGluePoint : STR_ViewMarkedPoint));
        }
        else
        {
            aStr1 = (ImpGetResStr(bGlue ? STR_ViewMarkedGluePoints : STR_ViewMarkedPoints));
            aStr1.SearchAndReplaceAscii("%N", UniString::CreateFromInt32(nMarkPtAnz));
        }

        aStr1.SearchAndReplaceAscii("%O", aNam);
        rName = aStr1;
        rNameOk = sal_True;
    }

    return rName;
}

sal_Bool SdrMarkList::TakeBoundRect(SdrPageView* pPV, Rectangle& rRect) const
{
    sal_Bool bFnd(sal_False);
    Rectangle aR;

    for(sal_uInt32 i(0L); i < GetMarkCount(); i++)
    {
        SdrMark* pMark = GetMark(i);

        if(!pPV || pMark->GetPageView() == pPV)
        {
            aR = pMark->GetObj()->GetCurrentBoundRect();

            if(bFnd)
            {
                rRect.Union(aR);
            }
            else
            {
                rRect = aR;
                bFnd = sal_True;
            }
        }
    }

    return bFnd;
}

sal_Bool SdrMarkList::TakeSnapRect(SdrPageView* pPV, Rectangle& rRect) const
{
    sal_Bool bFnd(sal_False);

    for(sal_uInt32 i(0L); i < GetMarkCount(); i++)
    {
        SdrMark* pMark = GetMark(i);

        if(!pPV || pMark->GetPageView() == pPV)
        {
            Rectangle aR(pMark->GetObj()->GetSnapRect());

            if(bFnd)
            {
                rRect.Union(aR);
            }
            else
            {
                rRect = aR;
                bFnd = sal_True;
            }
        }
    }

    return bFnd;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    ViewSelection::ViewSelection()
    :   mbEdgesOfMarkedNodesDirty(sal_False)
    {
    }

    void ViewSelection::SetEdgesOfMarkedNodesDirty()
    {
        if(!mbEdgesOfMarkedNodesDirty)
        {
            mbEdgesOfMarkedNodesDirty = sal_True;
            maEdgesOfMarkedNodes.Clear();
            maMarkedEdgesOfMarkedNodes.Clear();
            maAllMarkedObjects.Clear();
        }
    }

    const SdrMarkList& ViewSelection::GetEdgesOfMarkedNodes() const
    {
        if(mbEdgesOfMarkedNodesDirty)
        {
            ((ViewSelection*)this)->ImpForceEdgesOfMarkedNodes();
        }

        return maEdgesOfMarkedNodes;
    }

    const SdrMarkList& ViewSelection::GetMarkedEdgesOfMarkedNodes() const
    {
        if(mbEdgesOfMarkedNodesDirty)
        {
            ((ViewSelection*)this)->ImpForceEdgesOfMarkedNodes();
        }

        return maMarkedEdgesOfMarkedNodes;
    }

    const List& ViewSelection::GetAllMarkedObjects() const
    {
        if(mbEdgesOfMarkedNodesDirty)
        {
            ((ViewSelection*)this)->ImpForceEdgesOfMarkedNodes();
        }

        return maAllMarkedObjects;
    }

    void ViewSelection::ImplCollectCompleteSelection(SdrObject* pObj)
    {
        sal_Bool bIsGroup(pObj->IsGroupObject());

        if(bIsGroup && pObj->ISA(E3dObject) && !pObj->ISA(E3dScene))
        {
            bIsGroup = sal_False;
        }

        if(bIsGroup)
        {
            SdrObjList* pList = pObj->GetSubList();

            for(sal_uInt32 a(0L); a < pList->GetObjCount(); a++)
            {
                SdrObject* pObj2 = pList->GetObj(a);
                ImplCollectCompleteSelection(pObj2);
            }
        }

        maAllMarkedObjects.Insert(pObj, LIST_APPEND);
    }

    void ViewSelection::ImpForceEdgesOfMarkedNodes()
    {
        if(mbEdgesOfMarkedNodesDirty)
        {
            const sal_uInt32 nMarkAnz(maMarkedObjectList.GetMarkCount());
            sal_uInt32 a;

            maEdgesOfMarkedNodes.Clear();
            maMarkedEdgesOfMarkedNodes.Clear();
            maAllMarkedObjects.Clear();

            for(a = 0L; a < nMarkAnz; a++)
            {
                SdrObject* pObj = maMarkedObjectList.GetMark(a)->GetObj();
                ImplCollectCompleteSelection(pObj);
            }

            mbEdgesOfMarkedNodesDirty = sal_False;
            maMarkedObjectList.ForceSort();

            for(a = 0L; a < nMarkAnz; a++)
            {
                SdrObject* pNode = maMarkedObjectList.GetMark(a)->GetObj();
                const SfxBroadcaster* pBC = pNode->GetBroadcaster();

                if(pNode->IsNode())
                {
                    if(pBC)
                    {
                        sal_uInt16 nLstAnz(pBC->GetListenerCount());

                        for(sal_uInt16 nl(0); nl < nLstAnz; nl++)
                        {
                            SfxListener* pLst = pBC->GetListener(nl);
                            SdrEdgeObj* pEdge = PTR_CAST(SdrEdgeObj, pLst);

                            if(pEdge && pEdge->IsInserted() && pEdge->GetPage() == pNode->GetPage())
                            {
                                SdrMark aM(pEdge, maMarkedObjectList.GetMark(a)->GetPageView());

                                if(pEdge->GetConnectedNode(sal_True) == pNode)
                                {
                                    aM.SetCon1(sal_True);
                                }

                                if(pEdge->GetConnectedNode(sal_False) == pNode)
                                {
                                    aM.SetCon2(sal_True);
                                }

                                if(CONTAINER_ENTRY_NOTFOUND == maMarkedObjectList.FindObject(pEdge))
                                {
                                    // nachsehen, ob er selbst markiert ist
                                    maEdgesOfMarkedNodes.InsertEntry(aM);
                                }
                                else
                                {
                                    maMarkedEdgesOfMarkedNodes.InsertEntry(aM);
                                }
                            }
                        }
                    }
                }
            }

            maEdgesOfMarkedNodes.ForceSort();
            maMarkedEdgesOfMarkedNodes.ForceSort();
        }
    }
} // end of namespace sdr

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof
