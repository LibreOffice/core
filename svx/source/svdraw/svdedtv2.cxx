/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <svx/svdedtv.hxx>
#include <editeng/outliner.hxx>
#include <svx/svdundo.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdovirt.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdpagv.hxx>
#include "svx/svditer.hxx"
#include <svx/svdograf.hxx>
#include <svx/svdoole2.hxx>
#include "svx/svdstr.hrc"
#include "svx/svdglob.hxx"
#include "svdfmtf.hxx"
#include <svx/svdetc.hxx>
#include <sfx2/basedlgs.hxx>
#include <vcl/msgbox.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/eeitem.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>

#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>


#include <svx/svdoashp.hxx>
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>

#include <vector>
using ::std::vector;

SdrObject* SdrEditView::GetMaxToTopObj(SdrObject* /*pObj*/) const
{
  return NULL;
}

SdrObject* SdrEditView::GetMaxToBtmObj(SdrObject* /*pObj*/) const
{
  return NULL;
}

void SdrEditView::ObjOrderChanged(SdrObject* /*pObj*/, sal_uIntPtr /*nOldPos*/, sal_uIntPtr /*nNewPos*/)
{
}

void SdrEditView::MovMarkedToTop()
{
    sal_uIntPtr nAnz=GetMarkedObjectCount();
    if (nAnz!=0)
    {
        const bool bUndo = IsUndoEnabled();

        if( bUndo )
            BegUndo(ImpGetResStr(STR_EditMovToTop),GetDescriptionOfMarkedObjects(),SDRREPFUNC_OBJ_MOVTOTOP);

        SortMarkedObjects();
        sal_uIntPtr nm;
        for (nm=0; nm<nAnz; nm++)
        { 
            GetMarkedObjectByIndex(nm)->GetOrdNum();
        }
        bool bChg=false;
        SdrObjList* pOL0=NULL;
        sal_uIntPtr nNewPos=0;
        for (nm=nAnz; nm>0;)
        {
            nm--;
            SdrMark* pM=GetSdrMarkByIndex(nm);
            SdrObject* pObj=pM->GetMarkedSdrObj();
            SdrObjList* pOL=pObj->GetObjList();
            if (pOL!=pOL0)
            {
                nNewPos=sal_uIntPtr(pOL->GetObjCount()-1);
                pOL0=pOL;
            }
            sal_uIntPtr nNowPos=pObj->GetOrdNumDirect();
            const Rectangle& rBR=pObj->GetCurrentBoundRect();
            sal_uIntPtr nCmpPos=nNowPos+1;
            SdrObject* pMaxObj=GetMaxToTopObj(pObj);
            if (pMaxObj!=NULL)
            {
                sal_uIntPtr nMaxPos=pMaxObj->GetOrdNum();
                if (nMaxPos!=0)
                    nMaxPos--;
                if (nNewPos>nMaxPos)
                    nNewPos=nMaxPos; 
                if (nNewPos<nNowPos)
                    nNewPos=nNowPos; 
            }
            bool bEnd=false;
            while (nCmpPos<nNewPos && !bEnd)
            {
                SdrObject* pCmpObj=pOL->GetObj(nCmpPos);
                if (pCmpObj==NULL)
                {
                    OSL_FAIL("MovMarkedToTop(): Reference object not found.");
                    bEnd=true;
                }
                else if (pCmpObj==pMaxObj)
                {
                    nNewPos=nCmpPos;
                    nNewPos--;
                    bEnd=true;
                }
                else if (rBR.IsOver(pCmpObj->GetCurrentBoundRect()))
                {
                    nNewPos=nCmpPos;
                    bEnd=true;
                }
                else
                {
                    nCmpPos++;
                }
            }
            if (nNowPos!=nNewPos)
            {
                bChg=true;
                pOL->SetObjectOrdNum(nNowPos,nNewPos);
                if( bUndo )
                    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoObjectOrdNum(*pObj,nNowPos,nNewPos));
                ObjOrderChanged(pObj,nNowPos,nNewPos);
            }
            nNewPos--;
        }

        if( bUndo )
            EndUndo();

        if (bChg)
            MarkListHasChanged();
    }
}

void SdrEditView::MovMarkedToBtm()
{
    sal_uIntPtr nAnz=GetMarkedObjectCount();
    if (nAnz!=0)
    {
        const bool bUndo = IsUndoEnabled();

        if( bUndo )
            BegUndo(ImpGetResStr(STR_EditMovToBtm),GetDescriptionOfMarkedObjects(),SDRREPFUNC_OBJ_MOVTOBTM);

        SortMarkedObjects();
        sal_uIntPtr nm;
        for (nm=0; nm<nAnz; nm++)
        { 
            GetMarkedObjectByIndex(nm)->GetOrdNum();
        }

        bool bChg=false;
        SdrObjList* pOL0=NULL;
        sal_uIntPtr nNewPos=0;
        for (nm=0; nm<nAnz; nm++)
        {
            SdrMark* pM=GetSdrMarkByIndex(nm);
            SdrObject* pObj=pM->GetMarkedSdrObj();
            SdrObjList* pOL=pObj->GetObjList();
            if (pOL!=pOL0)
            {
                nNewPos=0;
                pOL0=pOL;
            }
            sal_uIntPtr nNowPos=pObj->GetOrdNumDirect();
            const Rectangle& rBR=pObj->GetCurrentBoundRect();
            sal_uIntPtr nCmpPos=nNowPos; if (nCmpPos>0) nCmpPos--;
            SdrObject* pMaxObj=GetMaxToBtmObj(pObj);
            if (pMaxObj!=NULL)
            {
                sal_uIntPtr nMinPos=pMaxObj->GetOrdNum()+1;
                if (nNewPos<nMinPos)
                    nNewPos=nMinPos; 
                if (nNewPos>nNowPos)
                    nNewPos=nNowPos; 
            }
            bool bEnd=false;
            
            
            
            while (nCmpPos>nNewPos && !bEnd)
            {
                SdrObject* pCmpObj=pOL->GetObj(nCmpPos);
                if (pCmpObj==NULL)
                {
                    OSL_FAIL("MovMarkedToBtm(): Reference object not found.");
                    bEnd=true;
                }
                else if (pCmpObj==pMaxObj)
                {
                    nNewPos=nCmpPos;
                    nNewPos++;
                    bEnd=true;
                }
                else if (rBR.IsOver(pCmpObj->GetCurrentBoundRect()))
                {
                    nNewPos=nCmpPos;
                    bEnd=true;
                }
                else
                {
                    nCmpPos--;
                }
            }
            if (nNowPos!=nNewPos)
            {
                bChg=true;
                pOL->SetObjectOrdNum(nNowPos,nNewPos);
                if( bUndo )
                    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoObjectOrdNum(*pObj,nNowPos,nNewPos));
                ObjOrderChanged(pObj,nNowPos,nNewPos);
            }
            nNewPos++;
        }

        if(bUndo)
            EndUndo();

        if(bChg)
            MarkListHasChanged();
    }
}

void SdrEditView::PutMarkedToTop()
{
    PutMarkedInFrontOfObj(NULL);
}

void SdrEditView::PutMarkedInFrontOfObj(const SdrObject* pRefObj)
{
    sal_uIntPtr nAnz=GetMarkedObjectCount();
    if (nAnz!=0)
    {
        const bool bUndo = IsUndoEnabled();
        if( bUndo )
            BegUndo(ImpGetResStr(STR_EditPutToTop),GetDescriptionOfMarkedObjects(),SDRREPFUNC_OBJ_PUTTOTOP);

        SortMarkedObjects();

        if (pRefObj!=NULL)
        {
            
            
            sal_uIntPtr nRefMark=TryToFindMarkedObject(pRefObj);
            SdrMark aRefMark;
            if (nRefMark!=CONTAINER_ENTRY_NOTFOUND)
            {
                aRefMark=*GetSdrMarkByIndex(nRefMark);
                GetMarkedObjectListWriteAccess().DeleteMark(nRefMark);
            }
            PutMarkedToBtm();
            if (nRefMark!=CONTAINER_ENTRY_NOTFOUND)
            {
                GetMarkedObjectListWriteAccess().InsertEntry(aRefMark);
                SortMarkedObjects();
            }
        }
        sal_uIntPtr nm;
        for (nm=0; nm<nAnz; nm++)
        { 
            GetMarkedObjectByIndex(nm)->GetOrdNum();
        }
        bool bChg=false;
        SdrObjList* pOL0=NULL;
        sal_uIntPtr nNewPos=0;
        for (nm=nAnz; nm>0;)
        {
            nm--;
            SdrMark* pM=GetSdrMarkByIndex(nm);
            SdrObject* pObj=pM->GetMarkedSdrObj();
            if (pObj!=pRefObj)
            {
                SdrObjList* pOL=pObj->GetObjList();
                if (pOL!=pOL0)
                {
                    nNewPos=sal_uIntPtr(pOL->GetObjCount()-1);
                    pOL0=pOL;
                }
                sal_uIntPtr nNowPos=pObj->GetOrdNumDirect();
                SdrObject* pMaxObj=GetMaxToTopObj(pObj);
                if (pMaxObj!=NULL)
                {
                    sal_uIntPtr nMaxOrd=pMaxObj->GetOrdNum(); 
                    if (nMaxOrd>0)
                        nMaxOrd--;
                    if (nNewPos>nMaxOrd)
                        nNewPos=nMaxOrd; 
                    if (nNewPos<nNowPos)
                        nNewPos=nNowPos; 
                }
                if (pRefObj!=NULL)
                {
                    if (pRefObj->GetObjList()==pObj->GetObjList())
                    {
                        sal_uIntPtr nMaxOrd=pRefObj->GetOrdNum(); 
                        if (nNewPos>nMaxOrd)
                            nNewPos=nMaxOrd; 
                        if (nNewPos<nNowPos)
                            nNewPos=nNowPos; 
                    }
                    else
                    {
                        nNewPos=nNowPos; 
                    }
                }
                if (nNowPos!=nNewPos)
                {
                    bChg=true;
                    pOL->SetObjectOrdNum(nNowPos,nNewPos);
                    if( bUndo )
                        AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoObjectOrdNum(*pObj,nNowPos,nNewPos));
                    ObjOrderChanged(pObj,nNowPos,nNewPos);
                }
                nNewPos--;
            } 
        } 

        if( bUndo )
            EndUndo();

        if(bChg)
            MarkListHasChanged();
    }
}

void SdrEditView::PutMarkedToBtm()
{
    PutMarkedBehindObj(NULL);
}

void SdrEditView::PutMarkedBehindObj(const SdrObject* pRefObj)
{
    sal_uIntPtr nAnz=GetMarkedObjectCount();
    if (nAnz!=0)
    {
        const bool bUndo = IsUndoEnabled();

        if( bUndo )
            BegUndo(ImpGetResStr(STR_EditPutToBtm),GetDescriptionOfMarkedObjects(),SDRREPFUNC_OBJ_PUTTOBTM);

        SortMarkedObjects();
        if (pRefObj!=NULL)
        {
            
            
            sal_uIntPtr nRefMark=TryToFindMarkedObject(pRefObj);
            SdrMark aRefMark;
            if (nRefMark!=CONTAINER_ENTRY_NOTFOUND)
            {
                aRefMark=*GetSdrMarkByIndex(nRefMark);
                GetMarkedObjectListWriteAccess().DeleteMark(nRefMark);
            }
            PutMarkedToTop();
            if (nRefMark!=CONTAINER_ENTRY_NOTFOUND)
            {
                GetMarkedObjectListWriteAccess().InsertEntry(aRefMark);
                SortMarkedObjects();
            }
        }
        sal_uIntPtr nm;
        for (nm=0; nm<nAnz; nm++) { 
            GetMarkedObjectByIndex(nm)->GetOrdNum();
        }
        bool bChg=false;
        SdrObjList* pOL0=NULL;
        sal_uIntPtr nNewPos=0;
        for (nm=0; nm<nAnz; nm++) {
            SdrMark* pM=GetSdrMarkByIndex(nm);
            SdrObject* pObj=pM->GetMarkedSdrObj();
            if (pObj!=pRefObj) {
                SdrObjList* pOL=pObj->GetObjList();
                if (pOL!=pOL0) {
                    nNewPos=0;
                    pOL0=pOL;
                }
                sal_uIntPtr nNowPos=pObj->GetOrdNumDirect();
                SdrObject* pMinObj=GetMaxToBtmObj(pObj);
                if (pMinObj!=NULL) {
                    sal_uIntPtr nMinOrd=pMinObj->GetOrdNum()+1; 
                    if (nNewPos<nMinOrd) nNewPos=nMinOrd; 
                    if (nNewPos>nNowPos) nNewPos=nNowPos; 
                }
                if (pRefObj!=NULL) {
                    if (pRefObj->GetObjList()==pObj->GetObjList()) {
                        sal_uIntPtr nMinOrd=pRefObj->GetOrdNum(); 
                        if (nNewPos<nMinOrd) nNewPos=nMinOrd; 
                        if (nNewPos>nNowPos) nNewPos=nNowPos; 
                    } else {
                        nNewPos=nNowPos; 
                    }
                }
                if (nNowPos!=nNewPos) {
                    bChg=true;
                    pOL->SetObjectOrdNum(nNowPos,nNewPos);
                    if( bUndo )
                        AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoObjectOrdNum(*pObj,nNowPos,nNewPos));
                    ObjOrderChanged(pObj,nNowPos,nNewPos);
                }
                nNewPos++;
            } 
        } 

        if(bUndo)
            EndUndo();

        if(bChg)
            MarkListHasChanged();
    }
}

void SdrEditView::ReverseOrderOfMarked()
{
    SortMarkedObjects();
    sal_uIntPtr nMarkAnz=GetMarkedObjectCount();
    if (nMarkAnz>0)
    {
        bool bChg=false;

        bool bUndo = IsUndoEnabled();
        if( bUndo )
            BegUndo(ImpGetResStr(STR_EditRevOrder),GetDescriptionOfMarkedObjects(),SDRREPFUNC_OBJ_REVORDER);

        sal_uIntPtr a=0;
        do {
            
            sal_uIntPtr b=a+1;
            while (b<nMarkAnz && GetSdrPageViewOfMarkedByIndex(b) == GetSdrPageViewOfMarkedByIndex(a)) b++;
            b--;
            SdrObjList* pOL=GetSdrPageViewOfMarkedByIndex(a)->GetObjList();
            sal_uIntPtr c=b;
            if (a<c) { 
                GetMarkedObjectByIndex(a)->GetOrdNum();
            }
            while (a<c) {
                SdrObject* pObj1=GetMarkedObjectByIndex(a);
                SdrObject* pObj2=GetMarkedObjectByIndex(c);
                sal_uIntPtr nOrd1=pObj1->GetOrdNumDirect();
                sal_uIntPtr nOrd2=pObj2->GetOrdNumDirect();
                if( bUndo )
                {
                    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoObjectOrdNum(*pObj1,nOrd1,nOrd2));
                    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoObjectOrdNum(*pObj2,nOrd2-1,nOrd1));
                }
                pOL->SetObjectOrdNum(nOrd1,nOrd2);
                
                pOL->SetObjectOrdNum(nOrd2-1,nOrd1);
                
                a++; c--;
                bChg=true;
            }
            a=b+1;
        } while (a<nMarkAnz);

        if(bUndo)
            EndUndo();

        if(bChg)
            MarkListHasChanged();
    }
}

void SdrEditView::ImpCheckToTopBtmPossible()
{
    sal_uIntPtr nAnz=GetMarkedObjectCount();
    if (nAnz==0)
        return;
    if (nAnz==1)
    { 
        SdrObject* pObj=GetMarkedObjectByIndex(0);
        SdrObjList* pOL=pObj->GetObjList();
        sal_uIntPtr nMax=pOL->GetObjCount();
        sal_uIntPtr nMin=0;
        sal_uIntPtr nObjNum=pObj->GetOrdNum();
        SdrObject* pRestrict=GetMaxToTopObj(pObj);
        if (pRestrict!=NULL) {
            sal_uIntPtr nRestrict=pRestrict->GetOrdNum();
            if (nRestrict<nMax) nMax=nRestrict;
        }
        pRestrict=GetMaxToBtmObj(pObj);
        if (pRestrict!=NULL) {
            sal_uIntPtr nRestrict=pRestrict->GetOrdNum();
            if (nRestrict>nMin) nMin=nRestrict;
        }
        bToTopPossible=nObjNum<sal_uIntPtr(nMax-1);
        bToBtmPossible=nObjNum>nMin;
    } else { 
        sal_uIntPtr nm=0;
        SdrObjList* pOL0=NULL;
        long nPos0=-1;
        while (!bToBtmPossible && nm<nAnz) { 
            SdrObject* pObj=GetMarkedObjectByIndex(nm);
            SdrObjList* pOL=pObj->GetObjList();
            if (pOL!=pOL0) {
                nPos0=-1;
                pOL0=pOL;
            }
            sal_uIntPtr nPos=pObj->GetOrdNum();
            bToBtmPossible=nPos>sal_uIntPtr(nPos0+1);
            nPos0=long(nPos);
            nm++;
        }
        nm=nAnz;
        pOL0=NULL;
        nPos0=0x7FFFFFFF;
        while (!bToTopPossible && nm>0) { 
            nm--;
            SdrObject* pObj=GetMarkedObjectByIndex(nm);
            SdrObjList* pOL=pObj->GetObjList();
            if (pOL!=pOL0) {
                nPos0=pOL->GetObjCount();
                pOL0=pOL;
            }
            sal_uIntPtr nPos=pObj->GetOrdNum();
            bToTopPossible=nPos+1<sal_uIntPtr(nPos0);
            nPos0=nPos;
        }
    }
}





void SdrEditView::ImpCopyAttributes(const SdrObject* pSource, SdrObject* pDest) const
{
    if (pSource!=NULL) {
        SdrObjList* pOL=pSource->GetSubList();
        if (pOL!=NULL && !pSource->Is3DObj()) { 
            SdrObjListIter aIter(*pOL,IM_DEEPNOGROUPS);
            pSource=aIter.Next();
        }
    }

    if(pSource && pDest)
    {
        SfxItemSet aSet(pMod->GetItemPool(),
            SDRATTR_START,              SDRATTR_NOTPERSIST_FIRST-1,
            SDRATTR_NOTPERSIST_LAST+1,  SDRATTR_END,
            EE_ITEMS_START,             EE_ITEMS_END,
            0, 0);

        aSet.Put(pSource->GetMergedItemSet());

        pDest->ClearMergedItem();
        pDest->SetMergedItemSet(aSet);

        pDest->NbcSetLayer(pSource->GetLayer());
        pDest->NbcSetStyleSheet(pSource->GetStyleSheet(), sal_True);
    }
}

sal_Bool SdrEditView::ImpCanConvertForCombine1(const SdrObject* pObj) const
{
    
    sal_Bool bIsLine(sal_False);

    const SdrPathObj* pPath = PTR_CAST(SdrPathObj,pObj);

    if(pPath)
    {
        bIsLine = pPath->IsLine();
    }

    SdrObjTransformInfoRec aInfo;
    pObj->TakeObjInfo(aInfo);

    return (aInfo.bCanConvToPath || aInfo.bCanConvToPoly || bIsLine);
}

sal_Bool SdrEditView::ImpCanConvertForCombine(const SdrObject* pObj) const
{
    SdrObjList* pOL = pObj->GetSubList();

    if(pOL && !pObj->Is3DObj())
    {
        SdrObjListIter aIter(*pOL, IM_DEEPNOGROUPS);

        while(aIter.IsMore())
        {
            SdrObject* pObj1 = aIter.Next();

            
            if(!ImpCanConvertForCombine1(pObj1))
            {
                return sal_False;
            }
        }
    }
    else
    {
        if(!ImpCanConvertForCombine1(pObj))
        {
            return sal_False;
        }
    }

    return sal_True;
}

basegfx::B2DPolyPolygon SdrEditView::ImpGetPolyPolygon1(const SdrObject* pObj, sal_Bool bCombine) const
{
    basegfx::B2DPolyPolygon aRetval;
    SdrPathObj* pPath = PTR_CAST(SdrPathObj, pObj);

    if(bCombine && pPath && !pObj->GetOutlinerParaObject())
    {
        aRetval = pPath->GetPathPoly();
    }
    else
    {
        SdrObject* pConvObj = pObj->ConvertToPolyObj(bCombine, false);

        if(pConvObj)
        {
            SdrObjList* pOL = pConvObj->GetSubList();

            if(pOL)
            {
                SdrObjListIter aIter(*pOL, IM_DEEPNOGROUPS);

                while(aIter.IsMore())
                {
                    SdrObject* pObj1 = aIter.Next();
                    pPath = PTR_CAST(SdrPathObj, pObj1);

                    if(pPath)
                    {
                        aRetval.append(pPath->GetPathPoly());
                    }
                }
            }
            else
            {
                pPath = PTR_CAST(SdrPathObj, pConvObj);

                if(pPath)
                {
                    aRetval = pPath->GetPathPoly();
                }
            }

            SdrObject::Free( pConvObj );
        }
    }

    return aRetval;
}

basegfx::B2DPolyPolygon SdrEditView::ImpGetPolyPolygon(const SdrObject* pObj, sal_Bool bCombine) const
{
    SdrObjList* pOL = pObj->GetSubList();

    if(pOL && !pObj->Is3DObj())
    {
        basegfx::B2DPolyPolygon aRetval;
        SdrObjListIter aIter(*pOL, IM_DEEPNOGROUPS);

        while(aIter.IsMore())
        {
            SdrObject* pObj1 = aIter.Next();
            aRetval.append(ImpGetPolyPolygon1(pObj1, bCombine));
        }

        return aRetval;
    }
    else
    {
        return ImpGetPolyPolygon1(pObj, bCombine);
    }
}

basegfx::B2DPolygon SdrEditView::ImpCombineToSinglePolygon(const basegfx::B2DPolyPolygon& rPolyPolygon) const
{
    const sal_uInt32 nPolyCount(rPolyPolygon.count());

    if(0L == nPolyCount)
    {
        return basegfx::B2DPolygon();
    }
    else if(1L == nPolyCount)
    {
        return rPolyPolygon.getB2DPolygon(0L);
    }
    else
    {
        basegfx::B2DPolygon aRetval(rPolyPolygon.getB2DPolygon(0L));

        for(sal_uInt32 a(1L); a < nPolyCount; a++)
        {
            basegfx::B2DPolygon aCandidate(rPolyPolygon.getB2DPolygon(a));

            if(aRetval.count())
            {
                if(aCandidate.count())
                {
                    const basegfx::B2DPoint aCA(aCandidate.getB2DPoint(0L));
                    const basegfx::B2DPoint aCB(aCandidate.getB2DPoint(aCandidate.count() - 1L));
                    const basegfx::B2DPoint aRA(aRetval.getB2DPoint(0L));
                    const basegfx::B2DPoint aRB(aRetval.getB2DPoint(aRetval.count() - 1L));

                    const double fRACA(basegfx::B2DVector(aCA - aRA).getLength());
                    const double fRACB(basegfx::B2DVector(aCB - aRA).getLength());
                    const double fRBCA(basegfx::B2DVector(aCA - aRB).getLength());
                    const double fRBCB(basegfx::B2DVector(aCB - aRB).getLength());

                    const double fSmallestRA(fRACA < fRACB ? fRACA : fRACB);
                    const double fSmallestRB(fRBCA < fRBCB ? fRBCA : fRBCB);

                    if(fSmallestRA < fSmallestRB)
                    {
                        
                        aRetval.flip();
                    }

                    const double fSmallestCA(fRACA < fRBCA ? fRACA : fRBCA);
                    const double fSmallestCB(fRACB < fRBCB ? fRACB : fRBCB);

                    if(fSmallestCB < fSmallestCA)
                    {
                        
                        aCandidate.flip();
                    }

                    
                    aRetval.append(aCandidate);
                }
            }
            else
            {
                aRetval = aCandidate;
            }
        }

        return aRetval;
    }
}


struct ImpDistributeEntry
{
    SdrObject*                  mpObj;
    sal_Int32                       mnPos;
    sal_Int32                       mnLength;
};

typedef vector< ImpDistributeEntry*> ImpDistributeEntryList;

void SdrEditView::DistributeMarkedObjects()
{
    sal_uInt32 nMark(GetMarkedObjectCount());

    if(nMark > 2)
    {
        SfxItemSet aNewAttr(pMod->GetItemPool());

        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        if(pFact)
        {
            AbstractSvxDistributeDialog *pDlg = pFact->CreateSvxDistributeDialog(NULL, aNewAttr);
            DBG_ASSERT(pDlg, "Dialogdiet fail!");

            sal_uInt16 nResult = pDlg->Execute();

            if(nResult == RET_OK)
            {
                SvxDistributeHorizontal eHor = pDlg->GetDistributeHor();
                SvxDistributeVertical eVer = pDlg->GetDistributeVer();
                ImpDistributeEntryList aEntryList;
                ImpDistributeEntryList::iterator itEntryList;
                sal_uInt32 nFullLength;

                const bool bUndo = IsUndoEnabled();
                if( bUndo )
                    BegUndo();

                if(eHor != SvxDistributeHorizontalNone)
                {
                    
                    nFullLength = 0L;

                    for( sal_uInt32 a = 0; a < nMark; a++ )
                    {
                        SdrMark* pMark = GetSdrMarkByIndex(a);
                        ImpDistributeEntry* pNew = new ImpDistributeEntry;

                        pNew->mpObj = pMark->GetMarkedSdrObj();

                        switch(eHor)
                        {
                            case SvxDistributeHorizontalLeft:
                            {
                                pNew->mnPos = pNew->mpObj->GetSnapRect().Left();
                                break;
                            }
                            case SvxDistributeHorizontalCenter:
                            {
                                pNew->mnPos = (pNew->mpObj->GetSnapRect().Right() + pNew->mpObj->GetSnapRect().Left()) / 2;
                                break;
                            }
                            case SvxDistributeHorizontalDistance:
                            {
                                pNew->mnLength = pNew->mpObj->GetSnapRect().GetWidth() + 1;
                                nFullLength += pNew->mnLength;
                                pNew->mnPos = (pNew->mpObj->GetSnapRect().Right() + pNew->mpObj->GetSnapRect().Left()) / 2;
                                break;
                            }
                            case SvxDistributeHorizontalRight:
                            {
                                pNew->mnPos = pNew->mpObj->GetSnapRect().Right();
                                break;
                            }
                            default: break;
                        }

                        for ( itEntryList = aEntryList.begin();
                              itEntryList < aEntryList.end() && (*itEntryList)->mnPos < pNew->mnPos;
                              ++itEntryList )
                        {};
                        if ( itEntryList < aEntryList.end() )
                            aEntryList.insert( itEntryList, pNew );
                        else
                            aEntryList.push_back( pNew );
                    }

                    if(eHor == SvxDistributeHorizontalDistance)
                    {
                        
                        sal_Int32 nWidth = GetAllMarkedBoundRect().GetWidth() + 1;
                        double fStepWidth = ((double)nWidth - (double)nFullLength) / (double)(aEntryList.size() - 1);
                        double fStepStart = (double)aEntryList[ 0 ]->mnPos;
                        fStepStart += fStepWidth + (double)((aEntryList[ 0 ]->mnLength + aEntryList[ 1 ]->mnLength) / 2);

                        
                        for( size_t i = 1, n = aEntryList.size()-1; i < n; ++i )
                        {
                            ImpDistributeEntry* pCurr = aEntryList[ i    ];
                            ImpDistributeEntry* pNext = aEntryList[ i + 1];
                            sal_Int32 nDelta = (sal_Int32)(fStepStart + 0.5) - pCurr->mnPos;
                            if( bUndo )
                                AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pCurr->mpObj));
                            pCurr->mpObj->Move(Size(nDelta, 0));
                            fStepStart += fStepWidth + (double)((pCurr->mnLength + pNext->mnLength) / 2);
                        }
                    }
                    else
                    {
                        
                        sal_Int32 nWidth = aEntryList[ aEntryList.size() - 1 ]->mnPos - aEntryList[ 0 ]->mnPos;
                        double fStepWidth = (double)nWidth / (double)(aEntryList.size() - 1);
                        double fStepStart = (double)aEntryList[ 0 ]->mnPos;
                        fStepStart += fStepWidth;

                        
                        for( size_t i = 1 ; i < aEntryList.size()-1 ; ++i )
                        {
                            ImpDistributeEntry* pCurr = aEntryList[ i ];
                            sal_Int32 nDelta = (sal_Int32)(fStepStart + 0.5) - pCurr->mnPos;
                            if( bUndo )
                                AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pCurr->mpObj));
                            pCurr->mpObj->Move(Size(nDelta, 0));
                            fStepStart += fStepWidth;
                        }
                    }

                    
                    for ( size_t i = 0, n = aEntryList.size(); i < n; ++i )
                        delete aEntryList[ i ];
                    aEntryList.clear();
                }

                if(eVer != SvxDistributeVerticalNone)
                {
                    
                    nFullLength = 0L;

                    for( sal_uInt32 a = 0; a < nMark; a++ )
                    {
                        SdrMark* pMark = GetSdrMarkByIndex(a);
                        ImpDistributeEntry* pNew = new ImpDistributeEntry;

                        pNew->mpObj = pMark->GetMarkedSdrObj();

                        switch(eVer)
                        {
                            case SvxDistributeVerticalTop:
                            {
                                pNew->mnPos = pNew->mpObj->GetSnapRect().Top();
                                break;
                            }
                            case SvxDistributeVerticalCenter:
                            {
                                pNew->mnPos = (pNew->mpObj->GetSnapRect().Bottom() + pNew->mpObj->GetSnapRect().Top()) / 2;
                                break;
                            }
                            case SvxDistributeVerticalDistance:
                            {
                                pNew->mnLength = pNew->mpObj->GetSnapRect().GetHeight() + 1;
                                nFullLength += pNew->mnLength;
                                pNew->mnPos = (pNew->mpObj->GetSnapRect().Bottom() + pNew->mpObj->GetSnapRect().Top()) / 2;
                                break;
                            }
                            case SvxDistributeVerticalBottom:
                            {
                                pNew->mnPos = pNew->mpObj->GetSnapRect().Bottom();
                                break;
                            }
                            default: break;
                        }

                        for ( itEntryList = aEntryList.begin();
                              itEntryList < aEntryList.end() && (*itEntryList)->mnPos < pNew->mnPos;
                              ++itEntryList )
                        {};
                        if ( itEntryList < aEntryList.end() )
                            aEntryList.insert( itEntryList, pNew );
                        else
                            aEntryList.push_back( pNew );
                    }

                    if(eVer == SvxDistributeVerticalDistance)
                    {
                        
                        sal_Int32 nHeight = GetAllMarkedBoundRect().GetHeight() + 1;
                        double fStepWidth = ((double)nHeight - (double)nFullLength) / (double)(aEntryList.size() - 1);
                        double fStepStart = (double)aEntryList[ 0 ]->mnPos;
                        fStepStart += fStepWidth + (double)((aEntryList[ 0 ]->mnLength + aEntryList[ 1 ]->mnLength) / 2);

                        
                        for( size_t i = 1, n = aEntryList.size()-1; i < n; ++i)
                        {
                            ImpDistributeEntry* pCurr = aEntryList[ i     ];
                            ImpDistributeEntry* pNext = aEntryList[ i + 1 ];
                            sal_Int32 nDelta = (sal_Int32)(fStepStart + 0.5) - pCurr->mnPos;
                            if( bUndo )
                                AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pCurr->mpObj));
                            pCurr->mpObj->Move(Size(0, nDelta));
                            fStepStart += fStepWidth + (double)((pCurr->mnLength + pNext->mnLength) / 2);
                        }
                    }
                    else
                    {
                        
                        sal_Int32 nHeight = aEntryList[ aEntryList.size() - 1 ]->mnPos - aEntryList[ 0 ]->mnPos;
                        double fStepWidth = (double)nHeight / (double)(aEntryList.size() - 1);
                        double fStepStart = (double)aEntryList[ 0 ]->mnPos;
                        fStepStart += fStepWidth;

                        
                        for(size_t i = 1, n = aEntryList.size()-1; i < n; ++i)
                        {
                            ImpDistributeEntry* pCurr = aEntryList[ i ];
                            sal_Int32 nDelta = (sal_Int32)(fStepStart + 0.5) - pCurr->mnPos;
                            if( bUndo )
                                AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pCurr->mpObj));
                            pCurr->mpObj->Move(Size(0, nDelta));
                            fStepStart += fStepWidth;
                        }
                    }

                    
                    for ( size_t i = 0, n = aEntryList.size(); i < n; ++i )
                        delete aEntryList[ i ];
                    aEntryList.clear();
                }

                
                SetUndoComment(ImpGetResStr(STR_DistributeMarkedObjects));

                if( bUndo )
                    EndUndo();
            }

            delete(pDlg);
        }
    }
}

void SdrEditView::MergeMarkedObjects(SdrMergeMode eMode)
{
    
    if(AreObjectsMarked())
    {
        SdrMarkList aRemove;
        SortMarkedObjects();

        const bool bUndo = IsUndoEnabled();

        if( bUndo )
            BegUndo();

        sal_uInt32 nInsPos=0xFFFFFFFF;
        const SdrObject* pAttrObj = NULL;
        basegfx::B2DPolyPolygon aMergePolyPolygonA;
        basegfx::B2DPolyPolygon aMergePolyPolygonB;

        SdrObjList* pInsOL = NULL;
        SdrPageView* pInsPV = NULL;
        bool bFirstObjectComplete(false);

        
        
        
        
        
        ConvertMarkedToPathObj(sal_True);
        OSL_ENSURE(AreObjectsMarked(), "no more objects selected after preparations (!)");

        for(sal_uInt32 a=0;a<GetMarkedObjectCount();a++)
        {
            SdrMark* pM = GetSdrMarkByIndex(a);
            SdrObject* pObj = pM->GetMarkedSdrObj();

            if(ImpCanConvertForCombine(pObj))
            {
                if(!pAttrObj)
                    pAttrObj = pObj;

                nInsPos = pObj->GetOrdNum() + 1;
                pInsPV = pM->GetPageView();
                pInsOL = pObj->GetObjList();

                
                
                SdrObjListIter aIter(*pObj, IM_DEEPWITHGROUPS);

                while(aIter.IsMore())
                {
                    SdrObject* pCandidate = aIter.Next();
                    SdrPathObj* pPathObj = PTR_CAST(SdrPathObj, pCandidate);
                    if(pPathObj)
                    {
                        basegfx::B2DPolyPolygon aTmpPoly(pPathObj->GetPathPoly());

                        
                        
                        
                        aTmpPoly = basegfx::tools::simplifyCurveSegments(aTmpPoly);

                        
                        
                        aTmpPoly = basegfx::tools::prepareForPolygonOperation(aTmpPoly);

                        if(!bFirstObjectComplete)
                        {
                            
                            
                            if(aMergePolyPolygonA.count())
                            {
                                aMergePolyPolygonA = basegfx::tools::solvePolygonOperationOr(aMergePolyPolygonA, aTmpPoly);
                            }
                            else
                            {
                                aMergePolyPolygonA = aTmpPoly;
                            }
                        }
                        else
                        {
                            if(aMergePolyPolygonB.count())
                            {
                                
                                
                                
                                aMergePolyPolygonB = basegfx::tools::solvePolygonOperationOr(aMergePolyPolygonB, aTmpPoly);
                            }
                            else
                            {
                                aMergePolyPolygonB = aTmpPoly;
                            }
                        }
                    }
                }

                
                if(!bFirstObjectComplete && aMergePolyPolygonA.count())
                {
                    bFirstObjectComplete = true;
                }

                
                aRemove.InsertEntry(SdrMark(pObj, pM->GetPageView()));
            }
        }

        switch(eMode)
        {
            case SDR_MERGE_MERGE:
            {
                
                static bool bTestXOR(false);
                if(bTestXOR)
                {
                    aMergePolyPolygonA = basegfx::tools::solvePolygonOperationXor(aMergePolyPolygonA, aMergePolyPolygonB);
                }
                else
                {
                    aMergePolyPolygonA = basegfx::tools::solvePolygonOperationOr(aMergePolyPolygonA, aMergePolyPolygonB);
                }
                break;
            }
            case SDR_MERGE_SUBSTRACT:
            {
                
                aMergePolyPolygonA = basegfx::tools::solvePolygonOperationDiff(aMergePolyPolygonA, aMergePolyPolygonB);
                break;
            }
            case SDR_MERGE_INTERSECT:
            {
                
                aMergePolyPolygonA = basegfx::tools::solvePolygonOperationAnd(aMergePolyPolygonA, aMergePolyPolygonB);
                break;
            }
        }

        
        if(pInsOL)
        {
            SdrPathObj* pPath = new SdrPathObj(OBJ_PATHFILL, aMergePolyPolygonA);
            ImpCopyAttributes(pAttrObj, pPath);
            SdrInsertReason aReason(SDRREASON_VIEWCALL, pAttrObj);
            pInsOL->InsertObject(pPath, nInsPos, &aReason);
            if( bUndo )
                AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoNewObject(*pPath));
            MarkObj(pPath, pInsPV, sal_False, sal_True);
        }

        aRemove.ForceSort();
        switch(eMode)
        {
            case SDR_MERGE_MERGE:
            {
                SetUndoComment(
                    ImpGetResStr(STR_EditMergeMergePoly),
                    aRemove.GetMarkDescription());
                break;
            }
            case SDR_MERGE_SUBSTRACT:
            {
                SetUndoComment(
                    ImpGetResStr(STR_EditMergeSubstractPoly),
                    aRemove.GetMarkDescription());
                break;
            }
            case SDR_MERGE_INTERSECT:
            {
                SetUndoComment(
                    ImpGetResStr(STR_EditMergeIntersectPoly),
                    aRemove.GetMarkDescription());
                break;
            }
        }
        DeleteMarkedList(aRemove);

        if( bUndo )
            EndUndo();
    }
}

void SdrEditView::CombineMarkedObjects(sal_Bool bNoPolyPoly)
{
    
    

    bool bUndo = IsUndoEnabled();

    
    if( bUndo )
        BegUndo("", "", bNoPolyPoly ? SDRREPFUNC_OBJ_COMBINE_ONEPOLY : SDRREPFUNC_OBJ_COMBINE_POLYPOLY);

    
    
    

    
    
    
    

    
    
    
    
    
    

    
    
    
    
    
    
    
    
    ConvertMarkedToPathObj(sal_False /* bLineToArea */);

    
    basegfx::B2DPolyPolygon aPolyPolygon;
    SdrObjList* pAktOL = 0L;
    SdrMarkList aRemoveMerker;

    SortMarkedObjects();
    sal_uInt32 nInsPos(0xFFFFFFFF);
    SdrObjList* pInsOL = 0L;
    SdrPageView* pInsPV = 0L;
    const sal_uInt32 nAnz(GetMarkedObjectCount());
    const SdrObject* pAttrObj = 0L;

    for(sal_uInt32 a(nAnz); a > 0L; )
    {
        a--;
        SdrMark* pM = GetSdrMarkByIndex(a);
        SdrObject* pObj = pM->GetMarkedSdrObj();
        SdrObjList* pThisOL = pObj->GetObjList();

        if(pAktOL != pThisOL)
        {
            pAktOL = pThisOL;
        }

        if(ImpCanConvertForCombine(pObj))
        {
            
            pAttrObj = pObj;

            
            
            
            basegfx::B2DPolyPolygon aTmpPoly(basegfx::tools::simplifyCurveSegments(ImpGetPolyPolygon(pObj, sal_True)));
            aPolyPolygon.insert(0L, aTmpPoly);

            if(!pInsOL)
            {
                nInsPos = pObj->GetOrdNum() + 1L;
                pInsPV = pM->GetPageView();
                pInsOL = pObj->GetObjList();
            }

            aRemoveMerker.InsertEntry(SdrMark(pObj, pM->GetPageView()));
        }
    }

    if(bNoPolyPoly)
    {
        basegfx::B2DPolygon aCombinedPolygon(ImpCombineToSinglePolygon(aPolyPolygon));
        aPolyPolygon.clear();
        aPolyPolygon.append(aCombinedPolygon);
    }

    const sal_uInt32 nPolyCount(aPolyPolygon.count());

    if(nPolyCount)
    {
        SdrObjKind eKind = OBJ_PATHFILL;

        if(nPolyCount > 1L)
        {
            aPolyPolygon.setClosed(true);
        }
        else
        {
            
            const basegfx::B2DPolygon aPolygon(aPolyPolygon.getB2DPolygon(0L));
            const sal_uInt32 nPointCount(aPolygon.count());

            if(nPointCount <= 2L)
            {
                eKind = OBJ_PATHLINE;
            }
            else
            {
                if(!aPolygon.isClosed())
                {
                    const basegfx::B2DPoint aPointA(aPolygon.getB2DPoint(0L));
                    const basegfx::B2DPoint aPointB(aPolygon.getB2DPoint(nPointCount - 1L));
                    const double fDistance(basegfx::B2DVector(aPointB - aPointA).getLength());
                    const double fJoinTolerance(10.0);

                    if(fDistance < fJoinTolerance)
                    {
                        aPolyPolygon.setClosed(true);
                    }
                    else
                    {
                        eKind = OBJ_PATHLINE;
                    }
                }
            }
        }

        SdrPathObj* pPath = new SdrPathObj(eKind,aPolyPolygon);

        
        ImpCopyAttributes(pAttrObj, pPath);

        
        const XLineStyle eLineStyle = ((const XLineStyleItem&)pAttrObj->GetMergedItem(XATTR_LINESTYLE)).GetValue();
        const XFillStyle eFillStyle = ((const XFillStyleItem&)pAttrObj->GetMergedItem(XATTR_FILLSTYLE)).GetValue();

        
        bool bIsClosedPathObj(pAttrObj->ISA(SdrPathObj) && ((SdrPathObj*)pAttrObj)->IsClosed());

        if(XLINE_NONE == eLineStyle && (XFILL_NONE == eFillStyle || !bIsClosedPathObj))
        {
            pPath->SetMergedItem(XLineStyleItem(XLINE_SOLID));
        }

        SdrInsertReason aReason(SDRREASON_VIEWCALL,pAttrObj);
        pInsOL->InsertObject(pPath,nInsPos,&aReason);
        if( bUndo )
            AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoNewObject(*pPath));

        
        
        
        
        UnmarkAllObj(pInsPV);
        MarkObj(pPath, pInsPV, sal_False, sal_True);
    }

    
    aRemoveMerker.ForceSort(); 
    if( bUndo )
        SetUndoComment(ImpGetResStr(bNoPolyPoly?STR_EditCombine_OnePoly:STR_EditCombine_PolyPoly),aRemoveMerker.GetMarkDescription());

    
    DeleteMarkedList(aRemoveMerker);
    if( bUndo )
        EndUndo();
}





sal_Bool SdrEditView::ImpCanDismantle(const basegfx::B2DPolyPolygon& rPpolyPolygon, sal_Bool bMakeLines) const
{
    sal_Bool bCan(sal_False);
    const sal_uInt32 nPolygonCount(rPpolyPolygon.count());

    if(nPolygonCount >= 2L)
    {
        
        bCan = sal_True;
    }
    else if(bMakeLines && 1L == nPolygonCount)
    {
        
        const basegfx::B2DPolygon aPolygon(rPpolyPolygon.getB2DPolygon(0L));
        const sal_uInt32 nPointCount(aPolygon.count());

        if(nPointCount > 2L)
        {
            bCan = sal_True;
        }
    }

    return bCan;
}

sal_Bool SdrEditView::ImpCanDismantle(const SdrObject* pObj, sal_Bool bMakeLines) const
{
    bool bOtherObjs(false);    
    bool bMin1PolyPoly(false); 
    SdrObjList* pOL = pObj->GetSubList();

    if(pOL)
    {
        
        SdrObjListIter aIter(*pOL, IM_DEEPNOGROUPS);

        while(aIter.IsMore() && !bOtherObjs)
        {
            const SdrObject* pObj1 = aIter.Next();
            const SdrPathObj* pPath = PTR_CAST(SdrPathObj, pObj1);

            if(pPath)
            {
                if(ImpCanDismantle(pPath->GetPathPoly(), bMakeLines))
                {
                    bMin1PolyPoly = true;
                }

                SdrObjTransformInfoRec aInfo;
                pObj1->TakeObjInfo(aInfo);

                if(!aInfo.bCanConvToPath)
                {
                    
                    bOtherObjs = true;
                }
            }
            else
            {
                bOtherObjs = true;
            }
        }
    }
    else
    {
        const SdrPathObj* pPath = PTR_CAST(SdrPathObj, pObj);
        const SdrObjCustomShape* pCustomShape = PTR_CAST(SdrObjCustomShape, pObj);

        
        if(pPath)
        {
            if(ImpCanDismantle(pPath->GetPathPoly(),bMakeLines))
            {
                bMin1PolyPoly = true;
            }

            SdrObjTransformInfoRec aInfo;
            pObj->TakeObjInfo(aInfo);

            
            if(!(aInfo.bCanConvToPath || aInfo.bCanConvToPoly) && !pPath->IsLine())
            {
                
                bOtherObjs = true;
            }
        }
        else if(pCustomShape)
        {
            if(bMakeLines)
            {
                
                bMin1PolyPoly = true;
            }
        }
        else
        {
            bOtherObjs = true;
        }
    }
    return bMin1PolyPoly && !bOtherObjs;
}

void SdrEditView::ImpDismantleOneObject(const SdrObject* pObj, SdrObjList& rOL, sal_uIntPtr& rPos, SdrPageView* pPV, sal_Bool bMakeLines)
{
    const SdrPathObj* pSrcPath = PTR_CAST(SdrPathObj, pObj);
    const SdrObjCustomShape* pCustomShape = PTR_CAST(SdrObjCustomShape, pObj);

    const bool bUndo = IsUndoEnabled();

    if(pSrcPath)
    {
        
        SdrObject* pLast = 0; 
        const basegfx::B2DPolyPolygon& rPolyPolygon(pSrcPath->GetPathPoly());
        const sal_uInt32 nPolyCount(rPolyPolygon.count());

        for(sal_uInt32 a(0); a < nPolyCount; a++)
        {
            const basegfx::B2DPolygon& rCandidate(rPolyPolygon.getB2DPolygon(a));
            const sal_uInt32 nPointCount(rCandidate.count());

            if(!bMakeLines || nPointCount < 2)
            {
                SdrPathObj* pPath = new SdrPathObj((SdrObjKind)pSrcPath->GetObjIdentifier(), basegfx::B2DPolyPolygon(rCandidate));
                ImpCopyAttributes(pSrcPath, pPath);
                pLast = pPath;
                SdrInsertReason aReason(SDRREASON_VIEWCALL, pSrcPath);
                rOL.InsertObject(pPath, rPos, &aReason);
                if( bUndo )
                    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoNewObject(*pPath, true));
                MarkObj(pPath, pPV, sal_False, sal_True);
                rPos++;
            }
            else
            {
                const sal_uInt32 nLoopCount(rCandidate.isClosed() ? nPointCount : nPointCount - 1);

                for(sal_uInt32 b(0); b < nLoopCount; b++)
                {
                    SdrObjKind eKind(OBJ_PLIN);
                    basegfx::B2DPolygon aNewPolygon;
                    const sal_uInt32 nNextIndex((b + 1) % nPointCount);

                    aNewPolygon.append(rCandidate.getB2DPoint(b));

                    if(rCandidate.areControlPointsUsed())
                    {
                        aNewPolygon.appendBezierSegment(
                            rCandidate.getNextControlPoint(b),
                            rCandidate.getPrevControlPoint(nNextIndex),
                            rCandidate.getB2DPoint(nNextIndex));
                        eKind = OBJ_PATHLINE;
                    }
                    else
                    {
                        aNewPolygon.append(rCandidate.getB2DPoint(nNextIndex));
                    }

                    SdrPathObj* pPath = new SdrPathObj(eKind, basegfx::B2DPolyPolygon(aNewPolygon));
                    ImpCopyAttributes(pSrcPath, pPath);
                    pLast = pPath;
                    SdrInsertReason aReason(SDRREASON_VIEWCALL, pSrcPath);
                    rOL.InsertObject(pPath, rPos, &aReason);
                    if( bUndo )
                        AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoNewObject(*pPath, true));
                    MarkObj(pPath, pPV, sal_False, sal_True);
                    rPos++;
                }
            }
        }

        if(pLast && pSrcPath->GetOutlinerParaObject())
        {
            pLast->SetOutlinerParaObject(new OutlinerParaObject(*pSrcPath->GetOutlinerParaObject()));
        }
    }
    else if(pCustomShape)
    {
        if(bMakeLines)
        {
            
            const SdrObject* pReplacement = pCustomShape->GetSdrObjectFromCustomShape();

            if(pReplacement)
            {
                SdrObject* pCandidate = pReplacement->Clone();
                DBG_ASSERT(pCandidate, "SdrEditView::ImpDismantleOneObject: Could not clone SdrObject (!)");
                pCandidate->SetModel(pCustomShape->GetModel());

                if(((SdrShadowItem&)pCustomShape->GetMergedItem(SDRATTR_SHADOW)).GetValue())
                {
                    if(pReplacement->ISA(SdrObjGroup))
                    {
                        pCandidate->SetMergedItem(SdrShadowItem(sal_True));
                    }
                }

                SdrInsertReason aReason(SDRREASON_VIEWCALL, pCustomShape);
                rOL.InsertObject(pCandidate, rPos, &aReason);
                if( bUndo )
                    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoNewObject(*pCandidate, true));
                MarkObj(pCandidate, pPV, sal_False, sal_True);

                if(pCustomShape->HasText() && !pCustomShape->IsTextPath())
                {
                    
                    SdrTextObj* pTextObj = (SdrTextObj*)SdrObjFactory::MakeNewObject(
                        pCustomShape->GetObjInventor(), OBJ_TEXT, 0L, pCustomShape->GetModel());

                    
                    OutlinerParaObject* pParaObj = pCustomShape->GetOutlinerParaObject();
                    if(pParaObj)
                    {
                        pTextObj->NbcSetOutlinerParaObject(new OutlinerParaObject(*pParaObj));
                    }

                    
                    SfxItemSet aTargetItemSet(pCustomShape->GetMergedItemSet());

                    
                    aTargetItemSet.Put(XLineStyleItem(XLINE_NONE));
                    aTargetItemSet.Put(XFillStyleItem(XFILL_NONE));

                    
                    Rectangle aTextBounds = pCustomShape->GetSnapRect();
                    if(pCustomShape->GetTextBounds(aTextBounds))
                    {
                        pTextObj->SetSnapRect(aTextBounds);
                    }

                    
                    const GeoStat& rSourceGeo = pCustomShape->GetGeoStat();
                    if(rSourceGeo.nDrehWink)
                    {
                        pTextObj->NbcRotate(
                            pCustomShape->GetSnapRect().Center(), rSourceGeo.nDrehWink,
                            rSourceGeo.nSin, rSourceGeo.nCos);
                    }

                    
                    pTextObj->SetMergedItemSet(aTargetItemSet);

                    
                    rOL.InsertObject(pTextObj, rPos + 1, &aReason);
                    if( bUndo )
                        AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoNewObject(*pTextObj, true));
                    MarkObj(pTextObj, pPV, sal_False, sal_True);
                }
            }
        }
    }
}

void SdrEditView::DismantleMarkedObjects(sal_Bool bMakeLines)
{
    
    SdrMarkList aRemoveMerker;

    SortMarkedObjects();

    const bool bUndo = IsUndoEnabled();

    if( bUndo )
    {
        
        BegUndo("", "", bMakeLines ? SDRREPFUNC_OBJ_DISMANTLE_LINES : SDRREPFUNC_OBJ_DISMANTLE_POLYS);
    }

    sal_uIntPtr nm;
    sal_uIntPtr nAnz=GetMarkedObjectCount();
    SdrObjList* pOL0=NULL;
    for (nm=nAnz; nm>0;) {
        nm--;
        SdrMark* pM=GetSdrMarkByIndex(nm);
        SdrObject* pObj=pM->GetMarkedSdrObj();
        SdrPageView* pPV=pM->GetPageView();
        SdrObjList* pOL=pObj->GetObjList();
        if (pOL!=pOL0) { pOL0=pOL; pObj->GetOrdNum(); } 
        if (ImpCanDismantle(pObj,bMakeLines)) {
            aRemoveMerker.InsertEntry(SdrMark(pObj,pM->GetPageView()));
            sal_uIntPtr nPos0=pObj->GetOrdNumDirect();
            sal_uIntPtr nPos=nPos0+1;
            SdrObjList* pSubList=pObj->GetSubList();
            if (pSubList!=NULL && !pObj->Is3DObj()) {
                SdrObjListIter aIter(*pSubList,IM_DEEPNOGROUPS);
                while (aIter.IsMore()) {
                    const SdrObject* pObj1=aIter.Next();
                    ImpDismantleOneObject(pObj1,*pOL,nPos,pPV,bMakeLines);
                }
            } else {
                ImpDismantleOneObject(pObj,*pOL,nPos,pPV,bMakeLines);
            }
            if( bUndo )
                AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoDeleteObject(*pObj,true));
            pOL->RemoveObject(nPos0);

            if( !bUndo )
                SdrObject::Free(pObj);
        }
    }

    if( bUndo )
    {
        
        SetUndoComment(ImpGetResStr(bMakeLines?STR_EditDismantle_Lines:STR_EditDismantle_Polys),aRemoveMerker.GetMarkDescription());
        
        EndUndo();
    }
}





void SdrEditView::GroupMarked(const SdrObject* pUserGrp)
{
    if (AreObjectsMarked())
    {
        SortMarkedObjects();

        const bool bUndo = IsUndoEnabled();
        if( bUndo )
        {
            BegUndo(ImpGetResStr(STR_EditGroup),GetDescriptionOfMarkedObjects(),SDRREPFUNC_OBJ_GROUP);

            const sal_uIntPtr nAnz = GetMarkedObjectCount();
            for(sal_uIntPtr nm = nAnz; nm>0; )
            {
                
                nm--;
                SdrMark* pM=GetSdrMarkByIndex(nm);
                SdrObject* pObj = pM->GetMarkedSdrObj();
                    std::vector< SdrUndoAction* > vConnectorUndoActions( CreateConnectorUndo( *pObj ) );
                    AddUndoActions( vConnectorUndoActions );
                    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoRemoveObject( *pObj ));
            }
        }

        SdrMarkList aNewMark;
        SdrPageView* pPV = GetSdrPageView();

        if(pPV)
        {
            SdrObjList* pAktLst=pPV->GetObjList();
            SdrObjList* pSrcLst=pAktLst;
            SdrObjList* pSrcLst0=pSrcLst;
            SdrPage*    pPage=pPV->GetPage();
            
            if (pSrcLst->IsObjOrdNumsDirty())
                pSrcLst->RecalcObjOrdNums();
            SdrObject*  pGrp=NULL;
            SdrObject*  pRefObj=NULL; 
            SdrObject*  pRefObj1=NULL; 
            SdrObjList* pDstLst=NULL;
            
            
            sal_uIntPtr       nInsPos=pSrcLst->GetObjCount();
            bool        bNeedInsPos=true;
            for (sal_uIntPtr nm=GetMarkedObjectCount(); nm>0;)
            {
                nm--;
                SdrMark* pM=GetSdrMarkByIndex(nm);
                if (pM->GetPageView()==pPV)
                {
                    if (pGrp==NULL)
                    {
                        if (pUserGrp!=NULL)
                            pGrp=pUserGrp->Clone();
                        if (pGrp==NULL)
                            pGrp=new SdrObjGroup;
                        pDstLst=pGrp->GetSubList();
                        DBG_ASSERT(pDstLst!=NULL,"Alleged group object doesn't return object list.");
                    }
                    SdrObject* pObj=pM->GetMarkedSdrObj();
                    pSrcLst=pObj->GetObjList();
                    if (pSrcLst!=pSrcLst0)
                    {
                        if (pSrcLst->IsObjOrdNumsDirty())
                            pSrcLst->RecalcObjOrdNums();
                    }
                    bool bForeignList=pSrcLst!=pAktLst;
                    bool bGrouped=pSrcLst!=pPage;
                    if (!bForeignList && bNeedInsPos)
                    {
                        nInsPos=pObj->GetOrdNum(); 
                        nInsPos++;
                        bNeedInsPos=false;
                    }
                    pSrcLst->RemoveObject(pObj->GetOrdNumDirect());
                    if (!bForeignList)
                        nInsPos--; 
                    SdrInsertReason aReason(SDRREASON_VIEWCALL);
                    pDstLst->InsertObject(pObj,0,&aReason);
                    GetMarkedObjectListWriteAccess().DeleteMark(nm);
                    if (pRefObj1==NULL)
                        pRefObj1=pObj; 
                    if (!bGrouped)
                    {
                        if (pRefObj==NULL)
                            pRefObj=pObj; 
                    }
                    pSrcLst0=pSrcLst;
                }
            }
            if (pRefObj==NULL)
                pRefObj=pRefObj1;
            if (pGrp!=NULL)
            {
                aNewMark.InsertEntry(SdrMark(pGrp,pPV));
                sal_uIntPtr nAnz=pDstLst->GetObjCount();
                SdrInsertReason aReason(SDRREASON_VIEWCALL,pRefObj);
                pAktLst->InsertObject(pGrp,nInsPos,&aReason);
                if( bUndo )
                {
                    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoNewObject(*pGrp,true)); 
                    for (sal_uIntPtr no=0; no<nAnz; no++)
                    {
                        AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoInsertObject(*pDstLst->GetObj(no)));
                    }
                }
            }
        }
        GetMarkedObjectListWriteAccess().Merge(aNewMark);
        MarkListHasChanged();

        if( bUndo )
            EndUndo();
    }
}





void SdrEditView::UnGroupMarked()
{
    SdrMarkList aNewMark;

    const bool bUndo = IsUndoEnabled();
    if( bUndo )
        BegUndo("", "", SDRREPFUNC_OBJ_UNGROUP);

    sal_uIntPtr nCount=0;
    OUString aName1;
    OUString aName;
    bool bNameOk=false;
    for (sal_uIntPtr nm=GetMarkedObjectCount(); nm>0;) {
        nm--;
        SdrMark* pM=GetSdrMarkByIndex(nm);
        SdrObject* pGrp=pM->GetMarkedSdrObj();
        SdrObjList* pSrcLst=pGrp->GetSubList();
        if (pSrcLst!=NULL) {
            nCount++;
            if (nCount==1) {
                aName = pGrp->TakeObjNameSingul();  
                aName1 = pGrp->TakeObjNamePlural(); 
                bNameOk=true;
            } else {
                if (nCount==2) aName=aName1; 
                if (bNameOk) {
                    OUString aStr(pGrp->TakeObjNamePlural()); 

                    if (aStr != aName)
                        bNameOk = false;
                }
            }
            sal_uIntPtr nDstCnt=pGrp->GetOrdNum();
            SdrObjList* pDstLst=pM->GetPageView()->GetObjList();

            
            
            
            sal_uIntPtr nAnz=pSrcLst->GetObjCount();
            sal_uIntPtr no;

            if( bUndo )
            {
                for (no=nAnz; no>0;)
                {
                    no--;
                    SdrObject* pObj=pSrcLst->GetObj(no);
                    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoRemoveObject(*pObj));
                }
            }
            for (no=0; no<nAnz; no++)
            {
                SdrObject* pObj=pSrcLst->RemoveObject(0);
                SdrInsertReason aReason(SDRREASON_VIEWCALL,pGrp);
                pDstLst->InsertObject(pObj,nDstCnt,&aReason);
                if( bUndo )
                    AddUndo( GetModel()->GetSdrUndoFactory().CreateUndoInsertObject(*pObj,true));
                nDstCnt++;
                
                
                aNewMark.InsertEntry(SdrMark(pObj,pM->GetPageView()),sal_False);
            }

            if( bUndo )
            {
                
                
                
                
                AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoDeleteObject(*pGrp));
            }
            pDstLst->RemoveObject(nDstCnt);

            if( !bUndo )
                SdrObject::Free(pGrp);

            GetMarkedObjectListWriteAccess().DeleteMark(nm);
        }
    }
    if (nCount!=0)
    {
        if (!bNameOk)
            aName=ImpGetResStr(STR_ObjNamePluralGRUP); 
        SetUndoComment(ImpGetResStr(STR_EditUngroup),aName);
    }

    if( bUndo )
        EndUndo();

    if (nCount!=0)
    {
        GetMarkedObjectListWriteAccess().Merge(aNewMark,sal_True); 
        MarkListHasChanged();
    }
}





SdrObject* SdrEditView::ImpConvertOneObj(SdrObject* pObj, sal_Bool bPath, sal_Bool bLineToArea)
{
    SdrObject* pNewObj = pObj->ConvertToPolyObj(bPath, bLineToArea);
    if (pNewObj!=NULL)
    {
        SdrObjList* pOL=pObj->GetObjList();
        DBG_ASSERT(pOL!=NULL,"ConvertTo: Object doesn't return object list");
        if (pOL!=NULL)
        {
            const bool bUndo = IsUndoEnabled();
            if( bUndo )
                AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoReplaceObject(*pObj,*pNewObj));

            pOL->ReplaceObject(pNewObj,pObj->GetOrdNum());

            if( !bUndo )
                SdrObject::Free(pObj);
        }
    }
    return pNewObj;
}

void SdrEditView::ImpConvertTo(sal_Bool bPath, sal_Bool bLineToArea)
{
    bool bMrkChg=false;
    if (AreObjectsMarked()) {
        sal_uIntPtr nMarkAnz=GetMarkedObjectCount();
        sal_uInt16 nDscrID=0;
        if(bLineToArea)
        {
            if(nMarkAnz == 1)
                nDscrID = STR_EditConvToContour;
            else
                nDscrID = STR_EditConvToContours;

            BegUndo(ImpGetResStr(nDscrID), GetDescriptionOfMarkedObjects());
        }
        else
        {
            if (bPath) {
                if (nMarkAnz==1) nDscrID=STR_EditConvToCurve;
                else nDscrID=STR_EditConvToCurves;
                BegUndo(ImpGetResStr(nDscrID),GetDescriptionOfMarkedObjects(),SDRREPFUNC_OBJ_CONVERTTOPATH);
            } else {
                if (nMarkAnz==1) nDscrID=STR_EditConvToPoly;
                else nDscrID=STR_EditConvToPolys;
                BegUndo(ImpGetResStr(nDscrID),GetDescriptionOfMarkedObjects(),SDRREPFUNC_OBJ_CONVERTTOPOLY);
            }
        }
        for (sal_uIntPtr nm=nMarkAnz; nm>0;) {
            nm--;
            SdrMark* pM=GetSdrMarkByIndex(nm);
            SdrObject* pObj=pM->GetMarkedSdrObj();
            SdrPageView* pPV=pM->GetPageView();
            if (pObj->IsGroupObject() && !pObj->Is3DObj()) {
                SdrObject* pGrp=pObj;
                SdrObjListIter aIter(*pGrp,IM_DEEPNOGROUPS);
                while (aIter.IsMore()) {
                    pObj=aIter.Next();
                    ImpConvertOneObj(pObj,bPath,bLineToArea);
                }
            } else {
                SdrObject* pNewObj=ImpConvertOneObj(pObj,bPath,bLineToArea);
                if (pNewObj!=NULL) {
                    bMrkChg=true;
                    GetMarkedObjectListWriteAccess().ReplaceMark(SdrMark(pNewObj,pPV),nm);
                }
            }
        }
        EndUndo();
        if (bMrkChg) AdjustMarkHdl();
        if (bMrkChg) MarkListHasChanged();
    }
}

void SdrEditView::ConvertMarkedToPathObj(sal_Bool bLineToArea)
{
    ImpConvertTo(sal_True, bLineToArea);
}

void SdrEditView::ConvertMarkedToPolyObj(sal_Bool bLineToArea)
{
    ImpConvertTo(sal_False, bLineToArea);
}





void SdrEditView::DoImportMarkedMtf(SvdProgressInfo *pProgrInfo)
{
    const bool bUndo = IsUndoEnabled();

    if( bUndo )
        BegUndo("", "", SDRREPFUNC_OBJ_IMPORTMTF);

    SortMarkedObjects();
    SdrMarkList aForTheDescription;
    SdrMarkList aNewMarked;
    sal_uIntPtr nAnz=GetMarkedObjectCount();

    for (sal_uIntPtr nm=nAnz; nm>0;)
    { 
        
        if( pProgrInfo != NULL )
        {
            pProgrInfo->SetNextObject();
            if(!pProgrInfo->ReportActions(0))
                break;
        }

        nm--;
        SdrMark*     pM=GetSdrMarkByIndex(nm);
        SdrObject*   pObj=pM->GetMarkedSdrObj();
        SdrPageView* pPV=pM->GetPageView();
        SdrObjList*  pOL=pObj->GetObjList();
        sal_uIntPtr        nInsPos=pObj->GetOrdNum()+1;
        SdrGrafObj*  pGraf=PTR_CAST(SdrGrafObj,pObj);
        SdrOle2Obj*  pOle2=PTR_CAST(SdrOle2Obj,pObj);
        sal_uIntPtr        nInsAnz=0;
        Rectangle aLogicRect;

        if(pGraf && (pGraf->HasGDIMetaFile() || pGraf->isEmbeddedSvg()))
        {
            GDIMetaFile aMetaFile;

            if(pGraf->HasGDIMetaFile())
            {
                aMetaFile = pGraf->GetTransformedGraphic(SDRGRAFOBJ_TRANSFORMATTR_COLOR|SDRGRAFOBJ_TRANSFORMATTR_MIRROR).GetGDIMetaFile();
            }
            else if(pGraf->isEmbeddedSvg())
            {
                aMetaFile = pGraf->getMetafileFromEmbeddedSvg();
            }

            if(aMetaFile.GetActionSize())
            {
                aLogicRect = pGraf->GetLogicRect();
                ImpSdrGDIMetaFileImport aFilter(*pMod, pObj->GetLayer(), aLogicRect);
                nInsAnz = aFilter.DoImport(aMetaFile, *pOL, nInsPos, pProgrInfo);
            }
        }
        if ( pOle2!=NULL && pOle2->GetGraphic() )
        {
            aLogicRect = pOle2->GetLogicRect();
            ImpSdrGDIMetaFileImport aFilter(*pMod, pObj->GetLayer(), aLogicRect);
            nInsAnz = aFilter.DoImport(pOle2->GetGraphic()->GetGDIMetaFile(), *pOL, nInsPos, pProgrInfo);
        }
        if (nInsAnz!=0)
        {
            
            GeoStat aGeoStat(pGraf ? pGraf->GetGeoStat() : pOle2->GetGeoStat());
            sal_uIntPtr nObj=nInsPos;

            if(aGeoStat.nShearWink)
            {
                aGeoStat.RecalcTan();
            }

            if(aGeoStat.nDrehWink)
            {
                aGeoStat.RecalcSinCos();
            }

            for (sal_uIntPtr i=0; i<nInsAnz; i++)
            {
                if( bUndo )
                    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoNewObject(*pOL->GetObj(nObj)));

                
                SdrObject* pCandidate = pOL->GetObj(nObj);

                
                if(aGeoStat.nShearWink)
                {
                    pCandidate->NbcShear(aLogicRect.TopLeft(), aGeoStat.nShearWink, aGeoStat.nTan, false);
                }

                if(aGeoStat.nDrehWink)
                {
                    pCandidate->NbcRotate(aLogicRect.TopLeft(), aGeoStat.nDrehWink, aGeoStat.nSin, aGeoStat.nCos);
                }

                SdrMark aNewMark(pCandidate, pPV);
                aNewMarked.InsertEntry(aNewMark);

                nObj++;
            }
            aForTheDescription.InsertEntry(*pM);

            if( bUndo )
                AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoDeleteObject(*pObj));

            
            GetMarkedObjectListWriteAccess().DeleteMark(TryToFindMarkedObject(pObj));
            pOL->RemoveObject(nInsPos-1);

            if( !bUndo )
                SdrObject::Free(pObj);
        }
    }

    if(aNewMarked.GetMarkCount())
    {
        
        for(sal_uIntPtr a(0); a < aNewMarked.GetMarkCount(); a++)
        {
            GetMarkedObjectListWriteAccess().InsertEntry(*aNewMarked.GetMark(a));
        }

        SortMarkedObjects();
    }

    if( bUndo )
    {
        SetUndoComment(ImpGetResStr(STR_EditImportMtf),aForTheDescription.GetMarkDescription());
        EndUndo();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
