/*************************************************************************
 *
 *  $RCSfile: svdmark.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:24 $
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

class ImpSdrUShortContSorter: public ContainerSorter
{
public:
    ImpSdrUShortContSorter(Container& rNewCont): ContainerSorter(rNewCont) {}
    virtual int Compare(const void* pElem1, const void* pElem2) const;
};

int ImpSdrUShortContSorter::Compare(const void* pElem1, const void* pElem2) const
{
    USHORT n1=USHORT(ULONG(pElem1));
    USHORT n2=USHORT(ULONG(pElem2));
    return n1<n2 ? -1 : n1>n2 ? 1 : 0;
}

void SdrUShortCont::Sort()
{
    ImpSdrUShortContSorter aSort(aArr);
    aSort.DoSort();
    bSorted=TRUE;
    if (bDelDups) {
        ULONG nNum=GetCount();
        if (nNum>1) {
            nNum--;
            USHORT nVal0=GetObject(nNum);
            while (nNum>0) {
                nNum--;
                USHORT nVal1=GetObject(nNum);
                if (nVal1==nVal0) {
                    Remove(nNum);
                }
                nVal0=nVal1;
            }
        }
    }
}

void SdrUShortCont::CheckSort(ULONG nPos)
{
    ULONG nAnz=aArr.Count();
    if (nPos>nAnz) nPos=nAnz;
    USHORT nAktVal=GetObject(nPos);
    if (nPos>0) {
        USHORT nPrevVal=GetObject(nPos-1);
        if (bDelDups) {
            if (nPrevVal>=nAktVal) bSorted=FALSE;
        } else {
            if (nPrevVal>nAktVal) bSorted=FALSE;
        }
    }
    if (nPos<nAnz-1) {
        USHORT nNextVal=GetObject(nPos+1);
        if (bDelDups) {
            if (nNextVal<=nAktVal) bSorted=FALSE;
        } else {
            if (nNextVal<nAktVal) bSorted=FALSE;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrMark& SdrMark::operator=(const SdrMark& rMark)
{
    pObj=rMark.pObj;
    pPageView=rMark.pPageView;
    bCon1=rMark.bCon1;
    bCon2=rMark.bCon2;
    nUser=rMark.nUser;
    if (rMark.pPoints==NULL) {
        if (pPoints!=NULL) {
            delete pPoints;
            pPoints=NULL;
        }
    } else {
        if (pPoints==NULL) {
            pPoints=new SdrUShortCont(*rMark.pPoints);
        } else {
            *pPoints=*rMark.pPoints;
        }
    }
    if (rMark.pLines==NULL) {
        if (pLines!=NULL) {
            delete pLines;
            pLines=NULL;
        }
    } else {
        if (pLines==NULL) {
            pLines=new SdrUShortCont(*rMark.pLines);
        } else {
            *pLines=*rMark.pLines;
        }
    }
    if (rMark.pGluePoints==NULL) {
        if (pGluePoints!=NULL) {
            delete pGluePoints;
            pGluePoints=NULL;
        }
    } else {
        if (pGluePoints==NULL) {
            pGluePoints=new SdrUShortCont(*rMark.pGluePoints);
        } else {
            *pGluePoints=*rMark.pGluePoints;
        }
    }
    return *this;
}

FASTBOOL SdrMark::operator==(const SdrMark& rMark) const
{
    FASTBOOL bRet=pObj==rMark.pObj && pPageView==rMark.pPageView && bCon1==rMark.bCon1 && bCon2==rMark.bCon2 && nUser==rMark.nUser;
    if ((pPoints!=NULL) != (rMark.pPoints!=NULL)) bRet=FALSE;
    if ((pLines !=NULL) != (rMark.pLines !=NULL)) bRet=FALSE;
    if ((pGluePoints!=NULL) != (rMark.pGluePoints!=NULL)) bRet=FALSE;
    if (bRet && pPoints!=NULL && *pPoints!=*rMark.pPoints) bRet=FALSE;
    if (bRet && pLines !=NULL && *pLines !=*rMark.pLines ) bRet=FALSE;
    if (bRet && pGluePoints!=NULL && *pGluePoints!=*rMark.pGluePoints) bRet=FALSE;
    return bRet;
}

SdrPage* SdrMark::GetPage() const
{
    return (pObj!=NULL ? pObj->GetPage() : NULL);
}

SdrObjList* SdrMark::GetObjList() const
{
    return (pObj!=NULL ? pObj->GetObjList() : NULL);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

class ImpSdrMarkListSorter: public ContainerSorter
{
public:
    ImpSdrMarkListSorter(Container& rNewCont): ContainerSorter(rNewCont) {}
    virtual int Compare(const void* pElem1, const void* pElem2) const;
};

int ImpSdrMarkListSorter::Compare(const void* pElem1, const void* pElem2) const
{
    const SdrObject* pObj1=((SdrMark*)pElem1)->GetObj();
    const SdrObject* pObj2=((SdrMark*)pElem2)->GetObj();
    const SdrObjList* pOL1=pObj1!=NULL ? pObj1->GetObjList() : NULL;
    const SdrObjList* pOL2=pObj2!=NULL ? pObj2->GetObjList() : NULL;
    if (pOL1==pOL2) {
        ULONG nObjOrd1=pObj1!=NULL ? pObj1->GetOrdNum() : 0;
        ULONG nObjOrd2=pObj2!=NULL ? pObj2->GetOrdNum() : 0;
        return nObjOrd1<nObjOrd2 ? -1 : 1;
    } else {
        return (long)pOL1<(long)pOL2 ? -1 : 1;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrMarkList::ForceSort()
{
    if (!bSorted) {
        bSorted=TRUE;
        ULONG nAnz=aList.Count();
        if (nAnz>1) {
            ImpSdrMarkListSorter aSort(aList);
            aSort.DoSort();
            // und nun doppelte rauswerfen
            if (aList.Count()>1) {
                SdrMark* pAkt=(SdrMark*)aList.Last();
                SdrMark* pCmp=(SdrMark*)aList.Prev();
                while (pCmp!=NULL) {
                    if (pAkt->GetObj()==pCmp->GetObj()) {
                        // Con1/Con2 Merging
                        if (pCmp->IsCon1()) pAkt->SetCon1(TRUE);
                        if (pCmp->IsCon2()) pAkt->SetCon2(TRUE);
                        // pCmp loeschen.
                        aList.Remove();
                        delete pCmp;
                        // Aktueller Entry im Container ist nun pAkt!
                    } else {
                        pAkt=pCmp;
                    }
                    pCmp=(SdrMark*)aList.Prev();
                }
            }
        }
    }
}

void SdrMarkList::Clear()
{
    for (ULONG i=0; i<GetMarkCount(); i++) {
        SdrMark* pMark=GetMark(i);
        delete pMark;
    }
    aList.Clear();
    SetNameDirty();
}

void SdrMarkList::operator=(const SdrMarkList& rLst)
{
    Clear();
    for (ULONG i=0; i<rLst.GetMarkCount(); i++) {
        SdrMark* pMark=rLst.GetMark(i);
        SdrMark* pNeuMark=new SdrMark(*pMark);
        aList.Insert(pNeuMark,CONTAINER_APPEND);
    }
    aMarkName=rLst.aMarkName;
    bNameOk=rLst.bNameOk;
    aPointName=rLst.aPointName;
    bPointNameOk=rLst.bPointNameOk;
    aGluePointName=rLst.aGluePointName;
    bGluePointNameOk=rLst.bGluePointNameOk;
    bSorted=rLst.bSorted;
}

ULONG SdrMarkList::FindObject(const SdrObject* pObj)
{
    ForceSort();
    if (pObj==NULL || aList.Count()==0) return CONTAINER_ENTRY_NOTFOUND;
    const SdrObjList* pOL=pObj->GetObjList();
    ULONG nObjOrd=pObj->GetOrdNum();
    ULONG nL=0;
    ULONG nR=aList.Count();
    if (GetMark(nL)->GetObj()==pObj) return nL;
    while (nL+1<nR) { // Der Gesuchte befindet sich zwischen, nicht auf den Grenzen!
        ULONG nMid=(nL+nR)/2;
        const SdrObject* pMidObj=GetMark(nMid)->GetObj();
        if (pMidObj==pObj) return nMid; // Hurra, gefunden!
        const SdrObjList* pMidOL=pMidObj!=NULL ? pMidObj->GetObjList() : NULL;
        ULONG nMidObjOrd=pMidObj!=NULL ? pMidObj->GetOrdNum() : 0;
        if (pMidOL==pOL) {
            if (nMidObjOrd<nObjOrd) nL=nMid;
            else if (nMidObjOrd>nObjOrd) nR=nMid;
            else {
                DBG_ASSERT(FALSE,"SdrMarkList::FindObject(): Anderes Objekt mit gleicher OrdNum gefunden!");
                return CONTAINER_ENTRY_NOTFOUND;
            }
        } else if ((long)pMidOL<(long)pOL) nL=nMid;
        else nR=nMid;
    }
    return CONTAINER_ENTRY_NOTFOUND;
}

void SdrMarkList::InsertEntry(const SdrMark& rMark, FASTBOOL bChkSort)
{
    SetNameDirty();
    ULONG nAnz=aList.Count();
    if (!bChkSort || !bSorted || nAnz==0) {
        if (!bChkSort) bSorted=FALSE;
        aList.Insert(new SdrMark(rMark),CONTAINER_APPEND);
    } else {
        SdrMark* pLast=GetMark(ULONG(nAnz-1));
        const SdrObject* pLastObj=pLast->GetObj();
        const SdrObject* pNeuObj=rMark.GetObj();
        if (pLastObj==pNeuObj) { // Aha, den gibt's schon
            // Con1/Con2 Merging
            if (rMark.IsCon1()) pLast->SetCon1(TRUE);
            if (rMark.IsCon2()) pLast->SetCon2(TRUE);
        } else {
            SdrMark* pKopie=new SdrMark(rMark);
            aList.Insert(pKopie,CONTAINER_APPEND);
            // und nun checken, ob die Sortierung noch ok ist
            const SdrObjList* pLastOL=pLastObj!=NULL ? pLastObj->GetObjList() : NULL;
            const SdrObjList* pNeuOL =pNeuObj !=NULL ? pNeuObj ->GetObjList() : NULL;
            if (pLastOL==pNeuOL) {
                ULONG nLastNum=pLastObj!=NULL ? pLastObj->GetOrdNum() : 0;
                ULONG nNeuNum =pNeuObj !=NULL ? pNeuObj ->GetOrdNum() : 0;
                if (nNeuNum<nLastNum) {
                    bSorted=FALSE; // irgendwann muss mal sortiert werden
                }
            } else {
                if ((long)pNeuOL<(long)pLastOL) {
                    bSorted=FALSE; // irgendwann muss mal sortiert werden
                }
            }
        }
    }
    return;
}

void SdrMarkList::DeleteMark(ULONG nNum)
{
    //ForceSort();
    SdrMark* pMark=GetMark(nNum);
    DBG_ASSERT(pMark!=NULL,"DeleteMark: MarkEntry nicht gefunden");
    if (pMark!=NULL) {
        aList.Remove(nNum);
        delete pMark;
        SetNameDirty();
    }
}

void SdrMarkList::ReplaceMark(const SdrMark& rNewMark, ULONG nNum)
{
    SdrMark* pMark=GetMark(nNum);
    DBG_ASSERT(pMark!=NULL,"ReplaceMark: MarkEntry nicht gefunden");
    if (pMark!=NULL) {
        delete pMark;
        SetNameDirty();
        SdrMark* pKopie=new SdrMark(rNewMark);
        aList.Replace(pKopie,nNum);
        bSorted=FALSE;
        // Checken, ob bSorted wirklich gesetzt werden muss
    }
}

void SdrMarkList::Merge(const SdrMarkList& rSrcList, FASTBOOL bReverse)
{
    // Merging ohne ein Sort bei rSrcList zu erzwingen
    if (rSrcList.bSorted) bReverse=FALSE;
    ULONG nAnz=rSrcList.aList.Count();
    if (!bReverse) {
        for (ULONG i=0; i<nAnz; i++) {
            SdrMark* pM=(SdrMark*)(rSrcList.aList.GetObject(i));
            InsertEntry(*pM);
        }
    } else {
        for (ULONG i=nAnz; i>0;) {
            i--;
            SdrMark* pM=(SdrMark*)(rSrcList.aList.GetObject(i));
            InsertEntry(*pM);
        }
    }
}

FASTBOOL SdrMarkList::DeletePageView(const SdrPageView& rPV)
{
    FASTBOOL bChgd=FALSE;
    for (ULONG i=GetMarkCount(); i>0;) {
        i--;
        SdrMark* pMark=GetMark(i);
        if (pMark->GetPageView()==&rPV) {
            aList.Remove(i);
            delete pMark;
            SetNameDirty();
            bChgd=TRUE;
        }
    }
    return bChgd;
}

FASTBOOL SdrMarkList::InsertPageView(const SdrPageView& rPV)
{
    FASTBOOL bChgd=FALSE;
    DeletePageView(rPV); // erstmal alle raus, dann die ganze Seite hinten dran
    SdrObject* pObj;
    const SdrObjList* pOL=rPV.GetObjList();
    ULONG nObjAnz=pOL->GetObjCount();
    for (ULONG nO=0; nO<nObjAnz; nO++) {
        pObj=pOL->GetObj(nO);
        FASTBOOL bDoIt=rPV.IsObjMarkable(pObj);
        if (bDoIt) {
            SdrMark* pM=new SdrMark(pObj,(SdrPageView*)&rPV);
            aList.Insert(pM,CONTAINER_APPEND);
            SetNameDirty();
            bChgd=TRUE;
        }
    }
    return bChgd;
}

const XubString& SdrMarkList::GetMarkDescription() const
{
    UINT32 nAnz(GetMarkCount());

    if(bNameOk && nAnz==1)
    {
        // Bei Einfachselektion nur Textrahmen cachen
        SdrObject* pObj = GetMark(0)->GetObj();
        SdrTextObj* pTextObj = PTR_CAST(SdrTextObj,pObj);

        if(!pTextObj || !pTextObj->IsTextFrame())
        {
            ((SdrMarkList*)(this))->bNameOk = FALSE;
        }
    }

    if(!bNameOk)
    {
        SdrMark* pMark = GetMark(0);
        XubString aNam;

        if(!nAnz)
        {
            ((SdrMarkList*)(this))->aMarkName = ImpGetResStr(STR_ObjNameNoObj);
        }
        else if(nAnz == 1)
        {
            pMark->GetObj()->TakeObjNameSingul(aNam);
        }
        else
        {
            pMark->GetObj()->TakeObjNamePlural(aNam);
            XubString aStr1;
            BOOL bEq(TRUE);

            for(UINT32 i = 1; i < GetMarkCount() && bEq; i++)
            {
                SdrMark* pMark = GetMark(i);
                pMark->GetObj()->TakeObjNamePlural(aStr1);
                bEq = aNam.Equals(aStr1);
            }

            if(!bEq)
                aNam = ImpGetResStr(STR_ObjNamePlural);

            aNam.Insert(sal_Unicode(' '), 0);
            aNam.Insert(UniString::CreateFromInt32(nAnz), 0);
        }

        ((SdrMarkList*)(this))->aMarkName = aNam;
        ((SdrMarkList*)(this))->bNameOk = TRUE;
    }

    return aMarkName;
}

const XubString& SdrMarkList::GetPointMarkDescription(FASTBOOL bGlue) const
{
    FASTBOOL& rNameOk = (FASTBOOL&)(bGlue ? bGluePointNameOk : bPointNameOk);
    XubString& rName = (XubString&)(bGlue ? aGluePointName : aPointName);
    UINT32 nMarkAnz(GetMarkCount());
    UINT32 nMarkPtAnz(0);
    UINT32 nMarkPtObjAnz(0);
    UINT32 n1stMarkNum(0xFFFFFFFF);

    for(UINT32 nMarkNum = 0; nMarkNum < nMarkAnz; nMarkNum++)
    {
        const SdrMark* pMark = GetMark(nMarkNum);
        const SdrUShortCont* pPts = bGlue ? pMark->GetMarkedGluePoints() : pMark->GetMarkedPoints();
        UINT32 nAnz(pPts ? pPts->GetCount() : 0);

        if(nAnz)
        {
            if(n1stMarkNum == 0xFFFFFFFF)
                n1stMarkNum = nMarkNum;

            nMarkPtAnz += nAnz;
            nMarkPtObjAnz++;
        }

        if(nMarkPtObjAnz > 1 && rNameOk)
            // vorzeitige Entscheidung
            return rName;
    }

    if(rNameOk && nMarkPtObjAnz == 1)
    {
        // Bei Einfachselektion nur Textrahmen cachen
        SdrObject* pObj = GetMark(0)->GetObj();
        SdrTextObj* pTextObj = PTR_CAST(SdrTextObj,pObj);

        if(!pTextObj || !pTextObj->IsTextFrame())
        {
            rNameOk = FALSE;
        }
    }

    if(!nMarkPtObjAnz)
    {
        rName.Erase();
        rNameOk = TRUE;
    }
    else if(!rNameOk)
    {
        const SdrMark* pMark = GetMark(n1stMarkNum);
        XubString aNam;

        if(nMarkPtObjAnz == 1)
        {
            pMark->GetObj()->TakeObjNameSingul(aNam);
        }
        else
        {
            pMark->GetObj()->TakeObjNamePlural(aNam);
            XubString aStr1;
            BOOL bEq(TRUE);

            for(UINT32 i = n1stMarkNum + 1; i < GetMarkCount() && bEq; i++)
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
                aNam = ImpGetResStr(STR_ObjNamePlural);

            aNam.Insert(sal_Unicode(' '), 0);
            aNam.Insert(UniString::CreateFromInt32(nMarkPtObjAnz), 0);
        }

        XubString aStr1;

        if(nMarkPtAnz == 1)
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
        rNameOk = TRUE;
    }

    return rName;
}

USHORT SdrMarkList::GetPageViewAnz() const
{
    USHORT nAnz=0;
    SdrPageView* pPV=NULL;
    for (ULONG i=0; i<GetMarkCount(); i++) {
        SdrMark* pMark=GetMark(i);
        if (pMark->GetPageView()!=pPV) {
            pPV=pMark->GetPageView();
            nAnz++;
        }
    }
    return nAnz;
}

SdrPageView* SdrMarkList::GetPageView(USHORT nNum) const
{
  return NULL;
}

FASTBOOL SdrMarkList::TakeBoundRect(SdrPageView* pPV, Rectangle& rRect) const
{
    FASTBOOL bFnd=FALSE;
    Rectangle aR;

    for (ULONG i=0; i<GetMarkCount(); i++) {
        SdrMark* pMark=GetMark(i);
        if (pPV==NULL || pMark->GetPageView()==pPV) {
            aR=pMark->GetObj()->GetBoundRect();
            if (bFnd) {
                rRect.Union(aR);
            } else {
                rRect=aR;
                bFnd=TRUE;
            }
        }
    }
    return bFnd;
}

FASTBOOL SdrMarkList::TakeSnapRect(SdrPageView* pPV, Rectangle& rRect) const
{
    FASTBOOL bFnd=FALSE;

    for (ULONG i=0; i<GetMarkCount(); i++) {
        SdrMark* pMark=GetMark(i);
        if (pPV==NULL || pMark->GetPageView()==pPV) {
            Rectangle aR(pMark->GetObj()->GetSnapRect());
            if (bFnd) {
                rRect.Union(aR);
            } else {
                rRect=aR;
                bFnd=TRUE;
            }
        }
    }
    return bFnd;
}

