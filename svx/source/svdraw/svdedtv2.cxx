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

#include <svx/svdedtv.hxx>
#include <editeng/outliner.hxx>
#include <svx/svdundo.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdovirt.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svditer.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdoole2.hxx>
#include <svx/dialmgr.hxx>
#include "svdfmtf.hxx"
#include "svdpdf.hxx"
#include <svx/svdetc.hxx>
#include <sfx2/basedlgs.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/eeitem.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <svx/svxdlg.hxx>
#include <svx/strings.hrc>
#include <svx/svdoashp.hxx>
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>
#include <sal/log.hxx>
#include <memory>
#include <vector>

using ::std::vector;
using namespace com::sun::star;

SdrObject* SdrEditView::GetMaxToTopObj(SdrObject* /*pObj*/) const
{
  return nullptr;
}

SdrObject* SdrEditView::GetMaxToBtmObj(SdrObject* /*pObj*/) const
{
  return nullptr;
}

void SdrEditView::ObjOrderChanged(SdrObject* /*pObj*/, size_t /*nOldPos*/, size_t /*nNewPos*/)
{
}

void SdrEditView::MovMarkedToTop()
{
    const size_t nCount=GetMarkedObjectCount();
    if (nCount!=0)
    {
        const bool bUndo = IsUndoEnabled();

        if( bUndo )
            BegUndo(SvxResId(STR_EditMovToTop),GetDescriptionOfMarkedObjects(),SdrRepeatFunc::MoveToTop);

        SortMarkedObjects();
        for (size_t nm=0; nm<nCount; ++nm)
        { // All Ordnums have to be correct!
            GetMarkedObjectByIndex(nm)->GetOrdNum();
        }
        bool bChg=false;
        SdrObjList* pOL0=nullptr;
        size_t nNewPos=0;
        for (size_t nm=nCount; nm>0;)
        {
            --nm;
            SdrMark* pM=GetSdrMarkByIndex(nm);
            SdrObject* pObj=pM->GetMarkedSdrObj();
            SdrObjList* pOL=pObj->getParentSdrObjListFromSdrObject();
            if (pOL!=pOL0)
            {
                nNewPos = pOL->GetObjCount()-1;
                pOL0=pOL;
            }
            const size_t nNowPos = pObj->GetOrdNumDirect();
            const tools::Rectangle& rBR=pObj->GetCurrentBoundRect();
            size_t nCmpPos = nNowPos+1;
            SdrObject* pMaxObj=GetMaxToTopObj(pObj);
            if (pMaxObj!=nullptr)
            {
                size_t nMaxPos=pMaxObj->GetOrdNum();
                if (nMaxPos!=0)
                    nMaxPos--;
                if (nNewPos>nMaxPos)
                    nNewPos=nMaxPos; // neither go faster...
                if (nNewPos<nNowPos)
                    nNewPos=nNowPos; // nor go in the other direction
            }
            bool bEnd=false;
            while (nCmpPos<nNewPos && !bEnd)
            {
                SdrObject* pCmpObj=pOL->GetObj(nCmpPos);
                if (pCmpObj==nullptr)
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
    const size_t nCount=GetMarkedObjectCount();
    if (nCount!=0)
    {
        const bool bUndo = IsUndoEnabled();

        if( bUndo )
            BegUndo(SvxResId(STR_EditMovToBtm),GetDescriptionOfMarkedObjects(),SdrRepeatFunc::MoveToBottom);

        SortMarkedObjects();
        for (size_t nm=0; nm<nCount; ++nm)
        { // All Ordnums have to be correct!
            GetMarkedObjectByIndex(nm)->GetOrdNum();
        }

        bool bChg=false;
        SdrObjList* pOL0=nullptr;
        size_t nNewPos=0;
        for (size_t nm=0; nm<nCount; ++nm)
        {
            SdrMark* pM=GetSdrMarkByIndex(nm);
            SdrObject* pObj=pM->GetMarkedSdrObj();
            SdrObjList* pOL=pObj->getParentSdrObjListFromSdrObject();
            if (pOL!=pOL0)
            {
                nNewPos=0;
                pOL0=pOL;
            }
            const size_t nNowPos = pObj->GetOrdNumDirect();
            const tools::Rectangle& rBR=pObj->GetCurrentBoundRect();
            size_t nCmpPos = nNowPos;
            if (nCmpPos>0)
                --nCmpPos;
            SdrObject* pMaxObj=GetMaxToBtmObj(pObj);
            if (pMaxObj!=nullptr)
            {
                const size_t nMinPos=pMaxObj->GetOrdNum()+1;
                if (nNewPos<nMinPos)
                    nNewPos=nMinPos; // neither go faster...
                if (nNewPos>nNowPos)
                    nNewPos=nNowPos; // nor go in the other direction
            }
            bool bEnd=false;
            // nNewPos in this case is the "maximum" position
            // the object may reach without going faster than the object before
            // it (multiple selection).
            while (nCmpPos>nNewPos && !bEnd)
            {
                SdrObject* pCmpObj=pOL->GetObj(nCmpPos);
                if (pCmpObj==nullptr)
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
    PutMarkedInFrontOfObj(nullptr);
}

void SdrEditView::PutMarkedInFrontOfObj(const SdrObject* pRefObj)
{
    const size_t nCount=GetMarkedObjectCount();
    if (nCount==0)
        return;

    const bool bUndo = IsUndoEnabled();
    if( bUndo )
        BegUndo(SvxResId(STR_EditPutToTop),GetDescriptionOfMarkedObjects(),SdrRepeatFunc::PutToTop);

    SortMarkedObjects();

    if (pRefObj!=nullptr)
    {
        // Make "in front of the object" work, even if the
        // selected objects are already in front of the other object
        const size_t nRefMark=TryToFindMarkedObject(pRefObj);
        SdrMark aRefMark;
        if (nRefMark!=SAL_MAX_SIZE)
        {
            aRefMark=*GetSdrMarkByIndex(nRefMark);
            GetMarkedObjectListWriteAccess().DeleteMark(nRefMark);
        }
        PutMarkedToBtm();
        if (nRefMark!=SAL_MAX_SIZE)
        {
            GetMarkedObjectListWriteAccess().InsertEntry(aRefMark);
            SortMarkedObjects();
        }
    }
    for (size_t nm=0; nm<nCount; ++nm)
    { // All Ordnums have to be correct!
        GetMarkedObjectByIndex(nm)->GetOrdNum();
    }
    bool bChg=false;
    SdrObjList* pOL0=nullptr;
    size_t nNewPos=0;
    for (size_t nm=nCount; nm>0;)
    {
        --nm;
        SdrMark* pM=GetSdrMarkByIndex(nm);
        SdrObject* pObj=pM->GetMarkedSdrObj();
        if (pObj!=pRefObj)
        {
            SdrObjList* pOL=pObj->getParentSdrObjListFromSdrObject();
            if (pOL!=pOL0)
            {
                nNewPos=pOL->GetObjCount()-1;
                pOL0=pOL;
            }
            const size_t nNowPos=pObj->GetOrdNumDirect();
            SdrObject* pMaxObj=GetMaxToTopObj(pObj);
            if (pMaxObj!=nullptr)
            {
                size_t nMaxOrd=pMaxObj->GetOrdNum(); // sadly doesn't work any other way
                if (nMaxOrd>0)
                    nMaxOrd--;
                if (nNewPos>nMaxOrd)
                    nNewPos=nMaxOrd; // neither go faster...
                if (nNewPos<nNowPos)
                    nNewPos=nNowPos; // nor go into the other direction
            }
            if (pRefObj!=nullptr)
            {
                if (pRefObj->getParentSdrObjListFromSdrObject()==pObj->getParentSdrObjListFromSdrObject())
                {
                    const size_t nMaxOrd=pRefObj->GetOrdNum(); // sadly doesn't work any other way
                    if (nNewPos>nMaxOrd)
                        nNewPos=nMaxOrd; // neither go faster...
                    if (nNewPos<nNowPos)
                        nNewPos=nNowPos; // nor go into the other direction
                }
                else
                {
                    nNewPos=nNowPos; // different PageView, so don't change
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
        } // if (pObj!=pRefObj)
    } // for loop over all selected objects

    if( bUndo )
        EndUndo();

    if(bChg)
        MarkListHasChanged();
}

void SdrEditView::PutMarkedToBtm()
{
    PutMarkedBehindObj(nullptr);
}

void SdrEditView::PutMarkedBehindObj(const SdrObject* pRefObj)
{
    const size_t nCount=GetMarkedObjectCount();
    if (nCount==0)
        return;

    const bool bUndo = IsUndoEnabled();

    if( bUndo )
        BegUndo(SvxResId(STR_EditPutToBtm),GetDescriptionOfMarkedObjects(),SdrRepeatFunc::PutToBottom);

    SortMarkedObjects();
    if (pRefObj!=nullptr)
    {
        // Make "behind the object" work, even if the
        // selected objects are already behind the other object
        const size_t nRefMark=TryToFindMarkedObject(pRefObj);
        SdrMark aRefMark;
        if (nRefMark!=SAL_MAX_SIZE)
        {
            aRefMark=*GetSdrMarkByIndex(nRefMark);
            GetMarkedObjectListWriteAccess().DeleteMark(nRefMark);
        }
        PutMarkedToTop();
        if (nRefMark!=SAL_MAX_SIZE)
        {
            GetMarkedObjectListWriteAccess().InsertEntry(aRefMark);
            SortMarkedObjects();
        }
    }
    for (size_t nm=0; nm<nCount; ++nm) { // All Ordnums have to be correct!
        GetMarkedObjectByIndex(nm)->GetOrdNum();
    }
    bool bChg=false;
    SdrObjList* pOL0=nullptr;
    size_t nNewPos=0;
    for (size_t nm=0; nm<nCount; ++nm) {
        SdrMark* pM=GetSdrMarkByIndex(nm);
        SdrObject* pObj=pM->GetMarkedSdrObj();
        if (pObj!=pRefObj) {
            SdrObjList* pOL=pObj->getParentSdrObjListFromSdrObject();
            if (pOL!=pOL0) {
                nNewPos=0;
                pOL0=pOL;
            }
            const size_t nNowPos=pObj->GetOrdNumDirect();
            SdrObject* pMinObj=GetMaxToBtmObj(pObj);
            if (pMinObj!=nullptr) {
                const size_t nMinOrd=pMinObj->GetOrdNum()+1; // sadly doesn't work any differently
                if (nNewPos<nMinOrd) nNewPos=nMinOrd; // neither go faster...
                if (nNewPos>nNowPos) nNewPos=nNowPos; // nor go into the other direction
            }
            if (pRefObj!=nullptr) {
                if (pRefObj->getParentSdrObjListFromSdrObject()==pObj->getParentSdrObjListFromSdrObject()) {
                    const size_t nMinOrd=pRefObj->GetOrdNum(); // sadly doesn't work any differently
                    if (nNewPos<nMinOrd) nNewPos=nMinOrd; // neither go faster...
                    if (nNewPos>nNowPos) nNewPos=nNowPos; // nor go into the other direction
                } else {
                    nNewPos=nNowPos; // different PageView, so don't change
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
        } // if (pObj!=pRefObj)
    } // for loop over all selected objects

    if(bUndo)
        EndUndo();

    if(bChg)
        MarkListHasChanged();

}

void SdrEditView::ReverseOrderOfMarked()
{
    SortMarkedObjects();
    const size_t nMarkCount=GetMarkedObjectCount();
    if (nMarkCount>0)
    {
        bool bChg=false;

        bool bUndo = IsUndoEnabled();
        if( bUndo )
            BegUndo(SvxResId(STR_EditRevOrder),GetDescriptionOfMarkedObjects(),SdrRepeatFunc::ReverseOrder);

        size_t a=0;
        do {
            // take into account selection across multiple PageViews
            size_t b=a+1;
            while (b<nMarkCount && GetSdrPageViewOfMarkedByIndex(b) == GetSdrPageViewOfMarkedByIndex(a)) ++b;
            --b;
            SdrObjList* pOL=GetSdrPageViewOfMarkedByIndex(a)->GetObjList();
            size_t c=b;
            if (a<c) { // make sure OrdNums aren't dirty
                GetMarkedObjectByIndex(a)->GetOrdNum();
            }
            while (a<c) {
                SdrObject* pObj1=GetMarkedObjectByIndex(a);
                SdrObject* pObj2=GetMarkedObjectByIndex(c);
                const size_t nOrd1=pObj1->GetOrdNumDirect();
                const size_t nOrd2=pObj2->GetOrdNumDirect();
                if( bUndo )
                {
                    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoObjectOrdNum(*pObj1,nOrd1,nOrd2));
                    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoObjectOrdNum(*pObj2,nOrd2-1,nOrd1));
                }
                pOL->SetObjectOrdNum(nOrd1,nOrd2);
                // Obj 2 has moved forward by one position, so now nOrd2-1
                pOL->SetObjectOrdNum(nOrd2-1,nOrd1);
                // use Replace instead of SetOrdNum for performance reasons (recalculation of Ordnums)
                ++a;
                --c;
                bChg=true;
            }
            a=b+1;
        } while (a<nMarkCount);

        if(bUndo)
            EndUndo();

        if(bChg)
            MarkListHasChanged();
    }
}

void SdrEditView::ImpCheckToTopBtmPossible()
{
    const size_t nCount=GetMarkedObjectCount();
    if (nCount==0)
        return;
    if (nCount==1)
    { // special-casing for single selection
        SdrObject* pObj=GetMarkedObjectByIndex(0);
        SdrObjList* pOL=pObj->getParentSdrObjListFromSdrObject();
        SAL_WARN_IF(!pOL, "svx", "Object somehow has no ObjList");
        size_t nMax = pOL ? pOL->GetObjCount() : 0;
        size_t nMin = 0;
        const size_t nObjNum=pObj->GetOrdNum();
        SdrObject* pRestrict=GetMaxToTopObj(pObj);
        if (pRestrict!=nullptr) {
            const size_t nRestrict=pRestrict->GetOrdNum();
            if (nRestrict<nMax) nMax=nRestrict;
        }
        pRestrict=GetMaxToBtmObj(pObj);
        if (pRestrict!=nullptr) {
            const size_t nRestrict=pRestrict->GetOrdNum();
            if (nRestrict>nMin) nMin=nRestrict;
        }
        m_bToTopPossible=nObjNum<nMax-1;
        m_bToBtmPossible=nObjNum>nMin;
    } else { // multiple selection
        SdrObjList* pOL0=nullptr;
        size_t nPos0 = 0;
        for (size_t nm = 0; !m_bToBtmPossible && nm<nCount; ++nm) { // check 'send to background'
            SdrObject* pObj=GetMarkedObjectByIndex(nm);
            SdrObjList* pOL=pObj->getParentSdrObjListFromSdrObject();
            if (pOL!=pOL0) {
                nPos0 = 0;
                pOL0=pOL;
            }
            const size_t nPos = pObj->GetOrdNum();
            m_bToBtmPossible = nPos && (nPos-1 > nPos0);
            nPos0 = nPos;
        }

        pOL0=nullptr;
        nPos0 = SAL_MAX_SIZE;
        for (size_t nm=nCount; !m_bToTopPossible && nm>0; ) { // check 'bring to front'
            --nm;
            SdrObject* pObj=GetMarkedObjectByIndex(nm);
            SdrObjList* pOL=pObj->getParentSdrObjListFromSdrObject();
            if (pOL!=pOL0) {
                nPos0=pOL->GetObjCount();
                pOL0=pOL;
            }
            const size_t nPos = pObj->GetOrdNum();
            m_bToTopPossible = nPos+1 < nPos0;
            nPos0=nPos;
        }
    }
}


// Combine


void SdrEditView::ImpCopyAttributes(const SdrObject* pSource, SdrObject* pDest) const
{
    if (pSource!=nullptr) {
        SdrObjList* pOL=pSource->GetSubList();
        if (pOL!=nullptr && !pSource->Is3DObj()) { // get first non-group object from group
            SdrObjListIter aIter(pOL,SdrIterMode::DeepNoGroups);
            pSource=aIter.Next();
        }
    }

    if(pSource && pDest)
    {
        SfxItemSet aSet(mpModel->GetItemPool(),
            svl::Items<SDRATTR_START,              SDRATTR_NOTPERSIST_FIRST-1,
            SDRATTR_NOTPERSIST_LAST+1,  SDRATTR_END,
            EE_ITEMS_START,             EE_ITEMS_END>{});

        aSet.Put(pSource->GetMergedItemSet());

        pDest->ClearMergedItem();
        pDest->SetMergedItemSet(aSet);

        pDest->NbcSetLayer(pSource->GetLayer());
        pDest->NbcSetStyleSheet(pSource->GetStyleSheet(), true);
    }
}

bool SdrEditView::ImpCanConvertForCombine1(const SdrObject* pObj)
{
    // new condition IsLine() to be able to combine simple Lines
    bool bIsLine(false);

    const SdrPathObj* pPath = dynamic_cast< const SdrPathObj*>( pObj );

    if(pPath)
    {
        bIsLine = pPath->IsLine();
    }

    SdrObjTransformInfoRec aInfo;
    pObj->TakeObjInfo(aInfo);

    return (aInfo.bCanConvToPath || aInfo.bCanConvToPoly || bIsLine);
}

bool SdrEditView::ImpCanConvertForCombine(const SdrObject* pObj)
{
    SdrObjList* pOL = pObj->GetSubList();

    if(pOL && !pObj->Is3DObj())
    {
        SdrObjListIter aIter(pOL, SdrIterMode::DeepNoGroups);

        while(aIter.IsMore())
        {
            SdrObject* pObj1 = aIter.Next();

            // all members of a group have to be convertible
            if(!ImpCanConvertForCombine1(pObj1))
            {
                return false;
            }
        }
    }
    else
    {
        if(!ImpCanConvertForCombine1(pObj))
        {
            return false;
        }
    }

    return true;
}

basegfx::B2DPolyPolygon SdrEditView::ImpGetPolyPolygon1(const SdrObject* pObj)
{
    basegfx::B2DPolyPolygon aRetval;
    const SdrPathObj* pPath = dynamic_cast<const SdrPathObj*>( pObj );

    if(pPath && !pObj->GetOutlinerParaObject())
    {
        aRetval = pPath->GetPathPoly();
    }
    else
    {
        SdrObject* pConvObj = pObj->ConvertToPolyObj(true/*bCombine*/, false);

        if(pConvObj)
        {
            SdrObjList* pOL = pConvObj->GetSubList();

            if(pOL)
            {
                SdrObjListIter aIter(pOL, SdrIterMode::DeepNoGroups);

                while(aIter.IsMore())
                {
                    SdrObject* pObj1 = aIter.Next();
                    pPath = dynamic_cast<SdrPathObj*>( pObj1 );

                    if(pPath)
                    {
                        aRetval.append(pPath->GetPathPoly());
                    }
                }
            }
            else
            {
                pPath = dynamic_cast<SdrPathObj*>( pConvObj );

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

basegfx::B2DPolyPolygon SdrEditView::ImpGetPolyPolygon(const SdrObject* pObj)
{
    SdrObjList* pOL = pObj->GetSubList();

    if(pOL && !pObj->Is3DObj())
    {
        basegfx::B2DPolyPolygon aRetval;
        SdrObjListIter aIter(pOL, SdrIterMode::DeepNoGroups);

        while(aIter.IsMore())
        {
            SdrObject* pObj1 = aIter.Next();
            aRetval.append(ImpGetPolyPolygon1(pObj1));
        }

        return aRetval;
    }
    else
    {
        return ImpGetPolyPolygon1(pObj);
    }
}

basegfx::B2DPolygon SdrEditView::ImpCombineToSinglePolygon(const basegfx::B2DPolyPolygon& rPolyPolygon)
{
    const sal_uInt32 nPolyCount(rPolyPolygon.count());

    if(0 == nPolyCount)
    {
        return basegfx::B2DPolygon();
    }
    else if(1 == nPolyCount)
    {
        return rPolyPolygon.getB2DPolygon(0);
    }
    else
    {
        basegfx::B2DPolygon aRetval(rPolyPolygon.getB2DPolygon(0));

        for(sal_uInt32 a(1); a < nPolyCount; a++)
        {
            basegfx::B2DPolygon aCandidate(rPolyPolygon.getB2DPolygon(a));

            if(aRetval.count())
            {
                if(aCandidate.count())
                {
                    const basegfx::B2DPoint aCA(aCandidate.getB2DPoint(0));
                    const basegfx::B2DPoint aCB(aCandidate.getB2DPoint(aCandidate.count() - 1));
                    const basegfx::B2DPoint aRA(aRetval.getB2DPoint(0));
                    const basegfx::B2DPoint aRB(aRetval.getB2DPoint(aRetval.count() - 1));

                    const double fRACA(basegfx::B2DVector(aCA - aRA).getLength());
                    const double fRACB(basegfx::B2DVector(aCB - aRA).getLength());
                    const double fRBCA(basegfx::B2DVector(aCA - aRB).getLength());
                    const double fRBCB(basegfx::B2DVector(aCB - aRB).getLength());

                    const double fSmallestRA(std::min(fRACA, fRACB));
                    const double fSmallestRB(std::min(fRBCA, fRBCB));

                    if(fSmallestRA < fSmallestRB)
                    {
                        // flip result
                        aRetval.flip();
                    }

                    const double fSmallestCA(std::min(fRACA, fRBCA));
                    const double fSmallestCB(std::min(fRACB, fRBCB));

                    if(fSmallestCB < fSmallestCA)
                    {
                        // flip candidate
                        aCandidate.flip();
                    }

                    // append candidate to retval
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

// for distribution dialog function
struct ImpDistributeEntry
{
    SdrObject*                  mpObj;
    sal_Int32                       mnPos;
    sal_Int32                       mnLength;
};

typedef vector<ImpDistributeEntry> ImpDistributeEntryList;

void SdrEditView::DistributeMarkedObjects(weld::Window* pParent)
{
    const size_t nMark(GetMarkedObjectCount());

    if(nMark > 2)
    {
        SfxItemSet aNewAttr(mpModel->GetItemPool());

        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        ScopedVclPtr<AbstractSvxDistributeDialog> pDlg(pFact->CreateSvxDistributeDialog(pParent, aNewAttr));

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

            if(eHor != SvxDistributeHorizontal::NONE)
            {
                // build sorted entry list
                nFullLength = 0;

                for( size_t a = 0; a < nMark; ++a )
                {
                    SdrMark* pMark = GetSdrMarkByIndex(a);
                    ImpDistributeEntry aNew;

                    aNew.mpObj = pMark->GetMarkedSdrObj();

                    switch(eHor)
                    {
                        case SvxDistributeHorizontal::Left:
                        {
                            aNew.mnPos = aNew.mpObj->GetSnapRect().Left();
                            break;
                        }
                        case SvxDistributeHorizontal::Center:
                        {
                            aNew.mnPos = (aNew.mpObj->GetSnapRect().Right() + aNew.mpObj->GetSnapRect().Left()) / 2;
                            break;
                        }
                        case SvxDistributeHorizontal::Distance:
                        {
                            aNew.mnLength = aNew.mpObj->GetSnapRect().GetWidth() + 1;
                            nFullLength += aNew.mnLength;
                            aNew.mnPos = (aNew.mpObj->GetSnapRect().Right() + aNew.mpObj->GetSnapRect().Left()) / 2;
                            break;
                        }
                        case SvxDistributeHorizontal::Right:
                        {
                            aNew.mnPos = aNew.mpObj->GetSnapRect().Right();
                            break;
                        }
                        default: break;
                    }

                    itEntryList = std::find_if(aEntryList.begin(), aEntryList.end(),
                        [&aNew](const ImpDistributeEntry& rEntry) { return rEntry.mnPos >= aNew.mnPos; });
                    if ( itEntryList < aEntryList.end() )
                        aEntryList.insert( itEntryList, aNew );
                    else
                        aEntryList.push_back( aNew );
                }

                if(eHor == SvxDistributeHorizontal::Distance)
                {
                    // calculate room in-between
                    sal_Int32 nWidth = GetAllMarkedBoundRect().GetWidth() + 1;
                    double fStepWidth = (static_cast<double>(nWidth) - static_cast<double>(nFullLength)) / static_cast<double>(aEntryList.size() - 1);
                    double fStepStart = static_cast<double>(aEntryList[ 0 ].mnPos);
                    fStepStart += fStepWidth + static_cast<double>((aEntryList[ 0 ].mnLength + aEntryList[ 1 ].mnLength) / 2);

                    // move entries 1..n-1
                    for( size_t i = 1, n = aEntryList.size()-1; i < n; ++i )
                    {
                        ImpDistributeEntry& rCurr = aEntryList[ i    ];
                        ImpDistributeEntry& rNext = aEntryList[ i + 1];
                        sal_Int32 nDelta = static_cast<sal_Int32>(fStepStart + 0.5) - rCurr.mnPos;
                        if( bUndo )
                            AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*rCurr.mpObj));
                        rCurr.mpObj->Move(Size(nDelta, 0));
                        fStepStart += fStepWidth + static_cast<double>((rCurr.mnLength + rNext.mnLength) / 2);
                    }
                }
                else
                {
                    // calculate distances
                    sal_Int32 nWidth = aEntryList[ aEntryList.size() - 1 ].mnPos - aEntryList[ 0 ].mnPos;
                    double fStepWidth = static_cast<double>(nWidth) / static_cast<double>(aEntryList.size() - 1);
                    double fStepStart = static_cast<double>(aEntryList[ 0 ].mnPos);
                    fStepStart += fStepWidth;

                    // move entries 1..n-1
                    for( size_t i = 1 ; i < aEntryList.size()-1 ; ++i )
                    {
                        ImpDistributeEntry& rCurr = aEntryList[ i ];
                        sal_Int32 nDelta = static_cast<sal_Int32>(fStepStart + 0.5) - rCurr.mnPos;
                        if( bUndo )
                            AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*rCurr.mpObj));
                        rCurr.mpObj->Move(Size(nDelta, 0));
                        fStepStart += fStepWidth;
                    }
                }

                // clear list
                aEntryList.clear();
            }

            if(eVer != SvxDistributeVertical::NONE)
            {
                // build sorted entry list
                nFullLength = 0;

                for( size_t a = 0; a < nMark; ++a )
                {
                    SdrMark* pMark = GetSdrMarkByIndex(a);
                    ImpDistributeEntry aNew;

                    aNew.mpObj = pMark->GetMarkedSdrObj();

                    switch(eVer)
                    {
                        case SvxDistributeVertical::Top:
                        {
                            aNew.mnPos = aNew.mpObj->GetSnapRect().Top();
                            break;
                        }
                        case SvxDistributeVertical::Center:
                        {
                            aNew.mnPos = (aNew.mpObj->GetSnapRect().Bottom() + aNew.mpObj->GetSnapRect().Top()) / 2;
                            break;
                        }
                        case SvxDistributeVertical::Distance:
                        {
                            aNew.mnLength = aNew.mpObj->GetSnapRect().GetHeight() + 1;
                            nFullLength += aNew.mnLength;
                            aNew.mnPos = (aNew.mpObj->GetSnapRect().Bottom() + aNew.mpObj->GetSnapRect().Top()) / 2;
                            break;
                        }
                        case SvxDistributeVertical::Bottom:
                        {
                            aNew.mnPos = aNew.mpObj->GetSnapRect().Bottom();
                            break;
                        }
                        default: break;
                    }

                    itEntryList = std::find_if(aEntryList.begin(), aEntryList.end(),
                        [&aNew](const ImpDistributeEntry& rEntry) { return rEntry.mnPos >= aNew.mnPos; });
                    if ( itEntryList < aEntryList.end() )
                        aEntryList.insert( itEntryList, aNew );
                    else
                        aEntryList.push_back( aNew );
                }

                if(eVer == SvxDistributeVertical::Distance)
                {
                    // calculate room in-between
                    sal_Int32 nHeight = GetAllMarkedBoundRect().GetHeight() + 1;
                    double fStepWidth = (static_cast<double>(nHeight) - static_cast<double>(nFullLength)) / static_cast<double>(aEntryList.size() - 1);
                    double fStepStart = static_cast<double>(aEntryList[ 0 ].mnPos);
                    fStepStart += fStepWidth + static_cast<double>((aEntryList[ 0 ].mnLength + aEntryList[ 1 ].mnLength) / 2);

                    // move entries 1..n-1
                    for( size_t i = 1, n = aEntryList.size()-1; i < n; ++i)
                    {
                        ImpDistributeEntry& rCurr = aEntryList[ i     ];
                        ImpDistributeEntry& rNext = aEntryList[ i + 1 ];
                        sal_Int32 nDelta = static_cast<sal_Int32>(fStepStart + 0.5) - rCurr.mnPos;
                        if( bUndo )
                            AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*rCurr.mpObj));
                        rCurr.mpObj->Move(Size(0, nDelta));
                        fStepStart += fStepWidth + static_cast<double>((rCurr.mnLength + rNext.mnLength) / 2);
                    }
                }
                else
                {
                    // calculate distances
                    sal_Int32 nHeight = aEntryList[ aEntryList.size() - 1 ].mnPos - aEntryList[ 0 ].mnPos;
                    double fStepWidth = static_cast<double>(nHeight) / static_cast<double>(aEntryList.size() - 1);
                    double fStepStart = static_cast<double>(aEntryList[ 0 ].mnPos);
                    fStepStart += fStepWidth;

                    // move entries 1..n-1
                    for(size_t i = 1, n = aEntryList.size()-1; i < n; ++i)
                    {
                        ImpDistributeEntry& rCurr = aEntryList[ i ];
                        sal_Int32 nDelta = static_cast<sal_Int32>(fStepStart + 0.5) - rCurr.mnPos;
                        if( bUndo )
                            AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*rCurr.mpObj));
                        rCurr.mpObj->Move(Size(0, nDelta));
                        fStepStart += fStepWidth;
                    }
                }

                // clear list
                aEntryList.clear();
            }

            // UNDO-Comment and end of UNDO
            mpModel->SetUndoComment(SvxResId(STR_DistributeMarkedObjects));

            if( bUndo )
                EndUndo();
        }
    }
}

void SdrEditView::MergeMarkedObjects(SdrMergeMode eMode)
{
    // #i73441# check content
    if(!AreObjectsMarked())
        return;

    SdrMarkList aRemove;
    SortMarkedObjects();

    const bool bUndo = IsUndoEnabled();

    if( bUndo )
        BegUndo();

    size_t nInsPos = SAL_MAX_SIZE;
    const SdrObject* pAttrObj = nullptr;
    basegfx::B2DPolyPolygon aMergePolyPolygonA;
    basegfx::B2DPolyPolygon aMergePolyPolygonB;

    SdrObjList* pInsOL = nullptr;
    SdrPageView* pInsPV = nullptr;
    bool bFirstObjectComplete(false);

    // make sure selected objects are contour objects
    // since now basegfx::utils::adaptiveSubdivide() is used, it is no longer
    // necessary to use ConvertMarkedToPolyObj which will subdivide curves using the old
    // mechanisms. In a next step the polygon clipper will even be able to clip curves...
    // ConvertMarkedToPolyObj(true);
    ConvertMarkedToPathObj(true);
    OSL_ENSURE(AreObjectsMarked(), "no more objects selected after preparations (!)");

    for(size_t a=0; a<GetMarkedObjectCount(); ++a)
    {
        SdrMark* pM = GetSdrMarkByIndex(a);
        SdrObject* pObj = pM->GetMarkedSdrObj();

        if(ImpCanConvertForCombine(pObj))
        {
            if(!pAttrObj)
                pAttrObj = pObj;

            nInsPos = pObj->GetOrdNum() + 1;
            pInsPV = pM->GetPageView();
            pInsOL = pObj->getParentSdrObjListFromSdrObject();

            // #i76891# use single iteration from SJ here which works on SdrObjects and takes
            // groups into account by itself
            SdrObjListIter aIter(*pObj, SdrIterMode::DeepWithGroups);

            while(aIter.IsMore())
            {
                SdrObject* pCandidate = aIter.Next();
                SdrPathObj* pPathObj = dynamic_cast<SdrPathObj*>( pCandidate );
                if(pPathObj)
                {
                    basegfx::B2DPolyPolygon aTmpPoly(pPathObj->GetPathPoly());

                    // #i76891# unfortunately ConvertMarkedToPathObj has converted all
                    // involved polygon data to curve segments, even if not necessary.
                    // It is better to try to reduce to more simple polygons.
                    aTmpPoly = basegfx::utils::simplifyCurveSegments(aTmpPoly);

                    // for each part polygon as preparation, remove self-intersections
                    // correct orientations and get rid of possible neutral polygons.
                    aTmpPoly = basegfx::utils::prepareForPolygonOperation(aTmpPoly);

                    if(!bFirstObjectComplete)
                    {
                        // #i111987# Also need to collect ORed source shape when more than
                        // a single polygon is involved
                        if(aMergePolyPolygonA.count())
                        {
                            aMergePolyPolygonA = basegfx::utils::solvePolygonOperationOr(aMergePolyPolygonA, aTmpPoly);
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
                            // to topologically correctly collect the 2nd polygon
                            // group it is necessary to OR the parts (each is seen as
                            // XOR-FillRule polygon and they are drawn over each-other)
                            aMergePolyPolygonB = basegfx::utils::solvePolygonOperationOr(aMergePolyPolygonB, aTmpPoly);
                        }
                        else
                        {
                            aMergePolyPolygonB = aTmpPoly;
                        }
                    }
                }
            }

            // was there something added to the first polygon?
            if(!bFirstObjectComplete && aMergePolyPolygonA.count())
            {
                bFirstObjectComplete = true;
            }

            // move object to temporary delete list
            aRemove.InsertEntry(SdrMark(pObj, pM->GetPageView()));
        }
    }

    switch(eMode)
    {
        case SdrMergeMode::Merge:
        {
            // merge all contained parts (OR)
            aMergePolyPolygonA = basegfx::utils::solvePolygonOperationOr(aMergePolyPolygonA, aMergePolyPolygonB);
            break;
        }
        case SdrMergeMode::Subtract:
        {
            // Subtract B from A
            aMergePolyPolygonA = basegfx::utils::solvePolygonOperationDiff(aMergePolyPolygonA, aMergePolyPolygonB);
            break;
        }
        case SdrMergeMode::Intersect:
        {
            // AND B and A
            aMergePolyPolygonA = basegfx::utils::solvePolygonOperationAnd(aMergePolyPolygonA, aMergePolyPolygonB);
            break;
        }
    }

    // #i73441# check insert list before taking actions
    if(pInsOL)
    {
        SdrPathObj* pPath = new SdrPathObj(pAttrObj->getSdrModelFromSdrObject(), OBJ_PATHFILL, aMergePolyPolygonA);
        ImpCopyAttributes(pAttrObj, pPath);
        pInsOL->InsertObject(pPath, nInsPos);
        if( bUndo )
            AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoNewObject(*pPath));

        // #i124760# To have a correct selection with only the new object it is necessary to
        // unmark all objects first. If not doing so, there may remain invalid pointers to objects
        // TTTT:Not needed for aw080 (!)
        UnmarkAllObj(pInsPV);

        MarkObj(pPath, pInsPV, false, true);
    }

    aRemove.ForceSort();
    switch(eMode)
    {
        case SdrMergeMode::Merge:
        {
            SetUndoComment(
                SvxResId(STR_EditMergeMergePoly),
                aRemove.GetMarkDescription());
            break;
        }
        case SdrMergeMode::Subtract:
        {
            SetUndoComment(
                SvxResId(STR_EditMergeSubstractPoly),
                aRemove.GetMarkDescription());
            break;
        }
        case SdrMergeMode::Intersect:
        {
            SetUndoComment(
                SvxResId(STR_EditMergeIntersectPoly),
                aRemove.GetMarkDescription());
            break;
        }
    }
    DeleteMarkedList(aRemove);

    if( bUndo )
        EndUndo();
}

void SdrEditView::EqualizeMarkedObjects(bool bWidth)
{
    const SdrMarkList& rMarkList = GetMarkedObjectList();
    size_t nMarked = rMarkList.GetMarkCount();

    if (nMarked < 2)
        return;

    size_t nLastSelected = 0;
    sal_Int64 nLastSelectedTime = rMarkList.GetMark(0)->getTimeStamp();
    for (size_t a = 1; a < nMarked; ++a)
    {
        sal_Int64 nCandidateTime = rMarkList.GetMark(a)->getTimeStamp();
        if (nCandidateTime > nLastSelectedTime)
        {
            nLastSelectedTime = nCandidateTime;
            nLastSelected = a;
        }
    }

    SdrObject* pLastSelectedObj = rMarkList.GetMark(nLastSelected)->GetMarkedSdrObj();
    Size aLastRectSize(pLastSelectedObj->GetLogicRect().GetSize());

    const bool bUndo = IsUndoEnabled();

    if (bUndo)
        BegUndo();

    for (size_t a = 0; a < nMarked; ++a)
    {
        if (a == nLastSelected)
            continue;
        SdrMark* pM = rMarkList.GetMark(a);
        SdrObject* pObj = pM->GetMarkedSdrObj();
        tools::Rectangle aLogicRect(pObj->GetLogicRect());
        Size aLogicRectSize(aLogicRect.GetSize());
        if (bWidth)
            aLogicRectSize.setWidth( aLastRectSize.Width() );
        else
            aLogicRectSize.setHeight( aLastRectSize.Height() );
        aLogicRect.SetSize(aLogicRectSize);
        if (bUndo)
            AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pObj));
        pObj->SetLogicRect(aLogicRect);
    }

    SetUndoComment(
        SvxResId(bWidth ? STR_EqualizeWidthMarkedObjects : STR_EqualizeHeightMarkedObjects),
        rMarkList.GetMarkDescription());

    if (bUndo)
        EndUndo();
}

void SdrEditView::CombineMarkedObjects(bool bNoPolyPoly)
{
    // #105899# Start of Combine-Undo put to front, else ConvertMarkedToPolyObj would
    // create a 2nd Undo-action and Undo-Comment.

    bool bUndo = IsUndoEnabled();

    // Undo-String will be set later
    if( bUndo )
        BegUndo("", "", bNoPolyPoly ? SdrRepeatFunc::CombineOnePoly : SdrRepeatFunc::CombinePolyPoly);

    // #105899# First, guarantee that all objects are converted to polyobjects,
    // especially for SdrGrafObj with bitmap filling this is necessary to not
    // lose the bitmap filling.

    // #i12392#
    // ConvertMarkedToPolyObj was too strong here, it will lose quality and
    // information when curve objects are combined. This can be replaced by
    // using ConvertMarkedToPathObj without changing the previous fix.

    // #i21250#
    // Instead of simply passing true as LineToArea, use bNoPolyPoly as info
    // if this command is a 'Combine' or a 'Connect' command. On Connect it's true.
    // To not concert line segments with a set line width to polygons in that case,
    // use this info. Do not convert LineToArea on Connect commands.
    // ConvertMarkedToPathObj(!bNoPolyPoly);

    // This is used for Combine and Connect. In no case it is necessary to force
    // the content to curve, but it is also not good to force to polygons. Thus,
    // curve is the less information losing one. Remember: This place is not
    // used for merge.
    // LineToArea is never necessary, both commands are able to take over the
    // set line style and to display it correctly. Thus, i will use a
    // ConvertMarkedToPathObj with a false in any case. Only drawback is that
    // simple polygons will be changed to curves, but with no information loss.
    ConvertMarkedToPathObj(false /* bLineToArea */);

    // continue as before
    basegfx::B2DPolyPolygon aPolyPolygon;
    SdrObjList* pCurrentOL = nullptr;
    SdrMarkList aRemoveBuffer;

    SortMarkedObjects();
    size_t nInsPos = SAL_MAX_SIZE;
    SdrObjList* pInsOL = nullptr;
    SdrPageView* pInsPV = nullptr;
    const SdrObject* pAttrObj = nullptr;

    for(size_t a = GetMarkedObjectCount(); a; )
    {
        --a;
        SdrMark* pM = GetSdrMarkByIndex(a);
        SdrObject* pObj = pM->GetMarkedSdrObj();
        SdrObjList* pThisOL = pObj->getParentSdrObjListFromSdrObject();

        if(pCurrentOL != pThisOL)
        {
            pCurrentOL = pThisOL;
        }

        if(ImpCanConvertForCombine(pObj))
        {
            // remember objects to be able to copy attributes
            pAttrObj = pObj;

            // unfortunately ConvertMarkedToPathObj has converted all
            // involved polygon data to curve segments, even if not necessary.
            // It is better to try to reduce to more simple polygons.
            basegfx::B2DPolyPolygon aTmpPoly(basegfx::utils::simplifyCurveSegments(ImpGetPolyPolygon(pObj)));
            aPolyPolygon.insert(0L, aTmpPoly);

            if(!pInsOL)
            {
                nInsPos = pObj->GetOrdNum() + 1;
                pInsPV = pM->GetPageView();
                pInsOL = pObj->getParentSdrObjListFromSdrObject();
            }

            aRemoveBuffer.InsertEntry(SdrMark(pObj, pM->GetPageView()));
        }
    }

    if(bNoPolyPoly)
    {
        basegfx::B2DPolygon aCombinedPolygon(ImpCombineToSinglePolygon(aPolyPolygon));
        aPolyPolygon.clear();
        aPolyPolygon.append(aCombinedPolygon);
    }

    const sal_uInt32 nPolyCount(aPolyPolygon.count());

    if (nPolyCount && pAttrObj)
    {
        SdrObjKind eKind = OBJ_PATHFILL;

        if(nPolyCount > 1)
        {
            aPolyPolygon.setClosed(true);
        }
        else
        {
            // check for Polyline
            const basegfx::B2DPolygon aPolygon(aPolyPolygon.getB2DPolygon(0));
            const sal_uInt32 nPointCount(aPolygon.count());

            if(nPointCount <= 2)
            {
                eKind = OBJ_PATHLINE;
            }
            else
            {
                if(!aPolygon.isClosed())
                {
                    const basegfx::B2DPoint aPointA(aPolygon.getB2DPoint(0));
                    const basegfx::B2DPoint aPointB(aPolygon.getB2DPoint(nPointCount - 1));
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

        SdrPathObj* pPath = new SdrPathObj(pAttrObj->getSdrModelFromSdrObject(), eKind, aPolyPolygon);

        // attributes of the lowest object
        ImpCopyAttributes(pAttrObj, pPath);

        // If LineStyle of pAttrObj is drawing::LineStyle_NONE force to drawing::LineStyle_SOLID to make visible.
        const drawing::LineStyle eLineStyle = pAttrObj->GetMergedItem(XATTR_LINESTYLE).GetValue();
        const drawing::FillStyle eFillStyle = pAttrObj->GetMergedItem(XATTR_FILLSTYLE).GetValue();

        // Take fill style/closed state of pAttrObj in account when deciding to change the line style
        bool bIsClosedPathObj(dynamic_cast<const SdrPathObj*>( pAttrObj) != nullptr && static_cast<const SdrPathObj*>(pAttrObj)->IsClosed());

        if(drawing::LineStyle_NONE == eLineStyle && (drawing::FillStyle_NONE == eFillStyle || !bIsClosedPathObj))
        {
            pPath->SetMergedItem(XLineStyleItem(drawing::LineStyle_SOLID));
        }

        pInsOL->InsertObject(pPath,nInsPos);
        if( bUndo )
            AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoNewObject(*pPath));

        // Here was a severe error: Without UnmarkAllObj, the new object was marked
        // additionally to the two ones which are deleted below. As long as those are
        // in the UNDO there is no problem, but as soon as they get deleted, the
        // MarkList will contain deleted objects -> GPF.
        UnmarkAllObj(pInsPV);
        MarkObj(pPath, pInsPV, false, true);
    }

    // build an UndoComment from the objects actually used
    aRemoveBuffer.ForceSort(); // important for remove (see below)
    if( bUndo )
        SetUndoComment(SvxResId(bNoPolyPoly?STR_EditCombine_OnePoly:STR_EditCombine_PolyPoly),aRemoveBuffer.GetMarkDescription());

    // remove objects actually used from the list
    DeleteMarkedList(aRemoveBuffer);
    if( bUndo )
        EndUndo();
}


// Dismantle


bool SdrEditView::ImpCanDismantle(const basegfx::B2DPolyPolygon& rPpolyPolygon, bool bMakeLines)
{
    bool bCan(false);
    const sal_uInt32 nPolygonCount(rPpolyPolygon.count());

    if(nPolygonCount >= 2)
    {
        // #i69172# dismantle makes sense with 2 or more polygons in a polyPolygon
        bCan = true;
    }
    else if(bMakeLines && 1 == nPolygonCount)
    {
        // #i69172# ..or with at least 2 edges (curves or lines)
        const basegfx::B2DPolygon& aPolygon(rPpolyPolygon.getB2DPolygon(0));
        const sal_uInt32 nPointCount(aPolygon.count());

        if(nPointCount > 2)
        {
            bCan = true;
        }
    }

    return bCan;
}

bool SdrEditView::ImpCanDismantle(const SdrObject* pObj, bool bMakeLines)
{
    bool bOtherObjs(false);    // true=objects other than PathObj's existent
    bool bMin1PolyPoly(false); // true=at least 1 tools::PolyPolygon with more than one Polygon existent
    SdrObjList* pOL = pObj->GetSubList();

    if(pOL)
    {
        // group object -- check all members if they're PathObjs
        SdrObjListIter aIter(pOL, SdrIterMode::DeepNoGroups);

        while(aIter.IsMore() && !bOtherObjs)
        {
            const SdrObject* pObj1 = aIter.Next();
            const SdrPathObj* pPath = dynamic_cast<const SdrPathObj*>( pObj1 );

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
                    // happens e. g. in the case of FontWork
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
        const SdrPathObj* pPath = dynamic_cast<const SdrPathObj*>(pObj);
        const SdrObjCustomShape* pCustomShape = dynamic_cast<const SdrObjCustomShape*>(pObj);

        // #i37011#
        if(pPath)
        {
            if(ImpCanDismantle(pPath->GetPathPoly(),bMakeLines))
            {
                bMin1PolyPoly = true;
            }

            SdrObjTransformInfoRec aInfo;
            pObj->TakeObjInfo(aInfo);

            // new condition IsLine() to be able to break simple Lines
            if(!(aInfo.bCanConvToPath || aInfo.bCanConvToPoly) && !pPath->IsLine())
            {
                // happens e. g. in the case of FontWork
                bOtherObjs = true;
            }
        }
        else if(pCustomShape)
        {
            if(bMakeLines)
            {
                // allow break command
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

void SdrEditView::ImpDismantleOneObject(const SdrObject* pObj, SdrObjList& rOL, size_t& rPos, SdrPageView* pPV, bool bMakeLines)
{
    const SdrPathObj* pSrcPath = dynamic_cast<const SdrPathObj*>( pObj );
    const SdrObjCustomShape* pCustomShape = dynamic_cast<const SdrObjCustomShape*>( pObj );

    const bool bUndo = IsUndoEnabled();

    if(pSrcPath)
    {
        // #i74631# redesigned due to XpolyPolygon removal and explicit constructors
        SdrObject* pLast = nullptr; // to be able to apply OutlinerParaObject
        const basegfx::B2DPolyPolygon& rPolyPolygon(pSrcPath->GetPathPoly());
        const sal_uInt32 nPolyCount(rPolyPolygon.count());

        for(sal_uInt32 a(0); a < nPolyCount; a++)
        {
            const basegfx::B2DPolygon& rCandidate(rPolyPolygon.getB2DPolygon(a));
            const sal_uInt32 nPointCount(rCandidate.count());

            if(!bMakeLines || nPointCount < 2)
            {
                SdrPathObj* pPath = new SdrPathObj(
                    pSrcPath->getSdrModelFromSdrObject(),
                    static_cast<SdrObjKind>(pSrcPath->GetObjIdentifier()),
                    basegfx::B2DPolyPolygon(rCandidate));
                ImpCopyAttributes(pSrcPath, pPath);
                pLast = pPath;
                rOL.InsertObject(pPath, rPos);
                if( bUndo )
                    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoNewObject(*pPath, true));
                MarkObj(pPath, pPV, false, true);
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

                    SdrPathObj* pPath = new SdrPathObj(
                        pSrcPath->getSdrModelFromSdrObject(),
                        eKind,
                        basegfx::B2DPolyPolygon(aNewPolygon));
                    ImpCopyAttributes(pSrcPath, pPath);
                    pLast = pPath;
                    rOL.InsertObject(pPath, rPos);
                    if( bUndo )
                        AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoNewObject(*pPath, true));
                    MarkObj(pPath, pPV, false, true);
                    rPos++;
                }
            }
        }

        if(pLast && pSrcPath->GetOutlinerParaObject())
        {
            pLast->SetOutlinerParaObject(std::make_unique<OutlinerParaObject>(*pSrcPath->GetOutlinerParaObject()));
        }
    }
    else if(pCustomShape)
    {
        if(bMakeLines)
        {
            // break up custom shape
            const SdrObject* pReplacement = pCustomShape->GetSdrObjectFromCustomShape();

            if(pReplacement)
            {
                SdrObject* pCandidate(pReplacement->CloneSdrObject(pReplacement->getSdrModelFromSdrObject()));
                DBG_ASSERT(pCandidate, "SdrEditView::ImpDismantleOneObject: Could not clone SdrObject (!)");

                if(pCustomShape->GetMergedItem(SDRATTR_SHADOW).GetValue())
                {
                    if(dynamic_cast<const SdrObjGroup*>( pReplacement) !=  nullptr)
                    {
                        pCandidate->SetMergedItem(makeSdrShadowItem(true));
                    }
                }

                rOL.InsertObject(pCandidate, rPos);
                if( bUndo )
                    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoNewObject(*pCandidate, true));
                MarkObj(pCandidate, pPV, false, true);

                if(pCustomShape->HasText() && !pCustomShape->IsTextPath())
                {
                    // #i37011# also create a text object and add at rPos + 1
                    SdrObject* pTextObj = SdrObjFactory::MakeNewObject(
                        pCustomShape->getSdrModelFromSdrObject(),
                        pCustomShape->GetObjInventor(),
                        OBJ_TEXT);

                    // Copy text content
                    OutlinerParaObject* pParaObj = pCustomShape->GetOutlinerParaObject();
                    if(pParaObj)
                    {
                        pTextObj->NbcSetOutlinerParaObject(std::make_unique<OutlinerParaObject>(*pParaObj));
                    }

                    // copy all attributes
                    SfxItemSet aTargetItemSet(pCustomShape->GetMergedItemSet());

                    // clear fill and line style
                    aTargetItemSet.Put(XLineStyleItem(drawing::LineStyle_NONE));
                    aTargetItemSet.Put(XFillStyleItem(drawing::FillStyle_NONE));

                    // get the text bounds and set at text object
                    tools::Rectangle aTextBounds = pCustomShape->GetSnapRect();
                    if(pCustomShape->GetTextBounds(aTextBounds))
                    {
                        pTextObj->SetSnapRect(aTextBounds);
                    }

                    // if rotated, copy GeoStat, too.
                    const GeoStat& rSourceGeo = pCustomShape->GetGeoStat();
                    if(rSourceGeo.nRotationAngle)
                    {
                        pTextObj->NbcRotate(
                            pCustomShape->GetSnapRect().Center(), rSourceGeo.nRotationAngle,
                            rSourceGeo.nSin, rSourceGeo.nCos);
                    }

                    // set modified ItemSet at text object
                    pTextObj->SetMergedItemSet(aTargetItemSet);

                    // insert object
                    rOL.InsertObject(pTextObj, rPos + 1);
                    if( bUndo )
                        AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoNewObject(*pTextObj, true));
                    MarkObj(pTextObj, pPV, false, true);
                }
            }
        }
    }
}

void SdrEditView::DismantleMarkedObjects(bool bMakeLines)
{
    // temporary MarkList
    SdrMarkList aRemoveBuffer;

    SortMarkedObjects();

    const bool bUndo = IsUndoEnabled();

    if( bUndo )
    {
        // comment is constructed later
        BegUndo("", "", bMakeLines ? SdrRepeatFunc::DismantleLines : SdrRepeatFunc::DismantlePolys);
    }

    SdrObjList* pOL0=nullptr;
    for (size_t nm=GetMarkedObjectCount(); nm>0;) {
        --nm;
        SdrMark* pM=GetSdrMarkByIndex(nm);
        SdrObject* pObj=pM->GetMarkedSdrObj();
        SdrPageView* pPV=pM->GetPageView();
        SdrObjList* pOL=pObj->getParentSdrObjListFromSdrObject();
        if (pOL!=pOL0) { pOL0=pOL; pObj->GetOrdNum(); } // make sure OrdNums are correct!
        if (ImpCanDismantle(pObj,bMakeLines)) {
            aRemoveBuffer.InsertEntry(SdrMark(pObj,pM->GetPageView()));
            const size_t nPos0=pObj->GetOrdNumDirect();
            size_t nPos=nPos0+1;
            SdrObjList* pSubList=pObj->GetSubList();
            if (pSubList!=nullptr && !pObj->Is3DObj()) {
                SdrObjListIter aIter(pSubList,SdrIterMode::DeepNoGroups);
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
        // construct UndoComment from objects actually used
        SetUndoComment(SvxResId(bMakeLines?STR_EditDismantle_Lines:STR_EditDismantle_Polys),aRemoveBuffer.GetMarkDescription());
        // remove objects actually used from the list
        EndUndo();
    }
}


// Group


void SdrEditView::GroupMarked()
{
    if (AreObjectsMarked())
    {
        SortMarkedObjects();

        const bool bUndo = IsUndoEnabled();
        if( bUndo )
        {
            BegUndo(SvxResId(STR_EditGroup),GetDescriptionOfMarkedObjects(),SdrRepeatFunc::Group);

            for(size_t nm = GetMarkedObjectCount(); nm>0; )
            {
                // add UndoActions for all affected objects
                --nm;
                SdrMark* pM=GetSdrMarkByIndex(nm);
                SdrObject* pObj = pM->GetMarkedSdrObj();
                AddUndoActions( CreateConnectorUndo( *pObj ) );
                AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoRemoveObject( *pObj ));
            }
        }

        SdrMarkList aNewMark;
        SdrPageView* pPV = GetSdrPageView();

        if(pPV)
        {
            SdrObjList* pCurrentLst=pPV->GetObjList();
            SdrObjList* pSrcLst=pCurrentLst;
            SdrObjList* pSrcLst0=pSrcLst;
            // make sure OrdNums are correct
            if (pSrcLst->IsObjOrdNumsDirty())
                pSrcLst->RecalcObjOrdNums();
            SdrObject*  pGrp=nullptr;
            SdrObjList* pDstLst=nullptr;
            // if all selected objects come from foreign object lists.
            // the group object is the last one in the list.
            size_t      nInsPos=pSrcLst->GetObjCount();
            bool        bNeedInsPos=true;
            for (size_t nm=GetMarkedObjectCount(); nm>0;)
            {
                --nm;
                SdrMark* pM=GetSdrMarkByIndex(nm);
                if (pM->GetPageView()==pPV)
                {
                    SdrObject* pObj=pM->GetMarkedSdrObj();
                    if (nullptr==pGrp)
                    {
                        pGrp = new SdrObjGroup(pObj->getSdrModelFromSdrObject());
                        pDstLst=pGrp->GetSubList();
                        DBG_ASSERT(pDstLst!=nullptr,"Alleged group object doesn't return object list.");
                    }
                    pSrcLst=pObj->getParentSdrObjListFromSdrObject();
                    if (pSrcLst!=pSrcLst0)
                    {
                        if (pSrcLst->IsObjOrdNumsDirty())
                            pSrcLst->RecalcObjOrdNums();
                    }
                    bool bForeignList=pSrcLst!=pCurrentLst;
                    if (!bForeignList && bNeedInsPos)
                    {
                        nInsPos=pObj->GetOrdNum(); // this way, all ObjOrdNum of the page are set
                        nInsPos++;
                        bNeedInsPos=false;
                    }
                    pSrcLst->RemoveObject(pObj->GetOrdNumDirect());
                    if (!bForeignList)
                        nInsPos--; // correct InsertPos
                    pDstLst->InsertObject(pObj,0);
                    GetMarkedObjectListWriteAccess().DeleteMark(nm);
                    pSrcLst0=pSrcLst;
                }
            }
            if (pGrp!=nullptr)
            {
                aNewMark.InsertEntry(SdrMark(pGrp,pPV));
                const size_t nCount=pDstLst->GetObjCount();
                pCurrentLst->InsertObject(pGrp,nInsPos);
                if( bUndo )
                {
                    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoNewObject(*pGrp,true)); // no recalculation!
                    for (size_t no=0; no<nCount; ++no)
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


// Ungroup


void SdrEditView::UnGroupMarked()
{
    SdrMarkList aNewMark;

    const bool bUndo = IsUndoEnabled();
    if( bUndo )
        BegUndo("", "", SdrRepeatFunc::Ungroup);

    size_t nCount=0;
    OUString aName1;
    OUString aName;
    bool bNameOk=false;
    for (size_t nm=GetMarkedObjectCount(); nm>0;) {
        --nm;
        SdrMark* pM=GetSdrMarkByIndex(nm);
        SdrObject* pGrp=pM->GetMarkedSdrObj();
        SdrObjList* pSrcLst=pGrp->GetSubList();
        if (pSrcLst!=nullptr) {
            nCount++;
            if (nCount==1) {
                aName = pGrp->TakeObjNameSingul();  // retrieve name of group
                aName1 = pGrp->TakeObjNamePlural(); // retrieve name of group
                bNameOk=true;
            } else {
                if (nCount==2) aName=aName1; // set plural name
                if (bNameOk) {
                    OUString aStr(pGrp->TakeObjNamePlural()); // retrieve name of group

                    if (aStr != aName)
                        bNameOk = false;
                }
            }
            size_t nDstCnt=pGrp->GetOrdNum();
            SdrObjList* pDstLst=pM->GetPageView()->GetObjList();

            // FIRST move contained objects to parent of group, so that
            // the contained objects are NOT migrated to the UNDO-ItemPool
            // when AddUndo(new SdrUndoDelObj(*pGrp)) is called.
            const size_t nObjCount=pSrcLst->GetObjCount();

            if( bUndo )
            {
                for (size_t no=nObjCount; no>0;)
                {
                    no--;
                    SdrObject* pObj=pSrcLst->GetObj(no);
                    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoRemoveObject(*pObj));
                }
            }
            for (size_t no=0; no<nObjCount; ++no)
            {
                SdrObject* pObj=pSrcLst->RemoveObject(0);
                pDstLst->InsertObject(pObj,nDstCnt);
                if( bUndo )
                    AddUndo( GetModel()->GetSdrUndoFactory().CreateUndoInsertObject(*pObj,true));
                nDstCnt++;
                // No SortCheck when inserting into MarkList, because that would
                // provoke a RecalcOrdNums() each time because of pObj->GetOrdNum():
                aNewMark.InsertEntry(SdrMark(pObj,pM->GetPageView()),false);
            }

            if( bUndo )
            {
                // Now it is safe to add the delete-UNDO which triggers the
                // MigrateItemPool now only for itself, not for the sub-objects.
                // nDstCnt is right, because previous inserts move group
                // object deeper and increase nDstCnt.
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
            aName=SvxResId(STR_ObjNamePluralGRUP); // Use the term "Group Objects," if different objects are grouped.
        SetUndoComment(SvxResId(STR_EditUngroup),aName);
    }

    if( bUndo )
        EndUndo();

    if (nCount!=0)
    {
        GetMarkedObjectListWriteAccess().Merge(aNewMark,true); // Because of the sorting above, aNewMark is reversed
        MarkListHasChanged();
    }
}


// ConvertToPoly


SdrObject* SdrEditView::ImpConvertOneObj(SdrObject* pObj, bool bPath, bool bLineToArea)
{
    SdrObject* pNewObj = pObj->ConvertToPolyObj(bPath, bLineToArea);
    if (pNewObj!=nullptr)
    {
        SdrObjList* pOL=pObj->getParentSdrObjListFromSdrObject();
        DBG_ASSERT(pOL!=nullptr,"ConvertTo: Object doesn't return object list");
        if (pOL!=nullptr)
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

void SdrEditView::ImpConvertTo(bool bPath, bool bLineToArea)
{
    if (AreObjectsMarked()) {
        bool bMrkChg = false;
        const size_t nMarkCount=GetMarkedObjectCount();
        const char* pDscrID = nullptr;
        if(bLineToArea)
        {
            if(nMarkCount == 1)
                pDscrID = STR_EditConvToContour;
            else
                pDscrID = STR_EditConvToContours;

            BegUndo(SvxResId(pDscrID), GetDescriptionOfMarkedObjects());
        }
        else
        {
            if (bPath) {
                if (nMarkCount==1) pDscrID=STR_EditConvToCurve;
                else pDscrID=STR_EditConvToCurves;
                BegUndo(SvxResId(pDscrID),GetDescriptionOfMarkedObjects(),SdrRepeatFunc::ConvertToPath);
            } else {
                if (nMarkCount==1) pDscrID=STR_EditConvToPoly;
                else pDscrID=STR_EditConvToPolys;
                BegUndo(SvxResId(pDscrID),GetDescriptionOfMarkedObjects(),SdrRepeatFunc::ConvertToPoly);
            }
        }
        for (size_t nm=nMarkCount; nm>0;) {
            --nm;
            SdrMark* pM=GetSdrMarkByIndex(nm);
            SdrObject* pObj=pM->GetMarkedSdrObj();
            SdrPageView* pPV=pM->GetPageView();
            if (pObj->IsGroupObject() && !pObj->Is3DObj()) {
                SdrObject* pGrp=pObj;
                SdrObjListIter aIter(*pGrp, SdrIterMode::DeepNoGroups);
                while (aIter.IsMore()) {
                    pObj=aIter.Next();
                    ImpConvertOneObj(pObj,bPath,bLineToArea);
                }
            } else {
                SdrObject* pNewObj=ImpConvertOneObj(pObj,bPath,bLineToArea);
                if (pNewObj!=nullptr) {
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

void SdrEditView::ConvertMarkedToPathObj(bool bLineToArea)
{
    ImpConvertTo(true, bLineToArea);
}

void SdrEditView::ConvertMarkedToPolyObj()
{
    ImpConvertTo(false, false/*bLineToArea*/);
}

namespace
{
    GDIMetaFile GetMetaFile(SdrGrafObj const * pGraf)
    {
        if (pGraf->HasGDIMetaFile())
            return pGraf->GetTransformedGraphic(SdrGrafObjTransformsAttrs::MIRROR).GetGDIMetaFile();
        assert(pGraf->isEmbeddedVectorGraphicData());
        return pGraf->getMetafileFromEmbeddedVectorGraphicData();
    }
}

// Metafile Import
void SdrEditView::DoImportMarkedMtf(SvdProgressInfo *pProgrInfo)
{
    const bool bUndo = IsUndoEnabled();

    if( bUndo )
        BegUndo("", "", SdrRepeatFunc::ImportMtf);

    SortMarkedObjects();
    SdrMarkList aForTheDescription;
    SdrMarkList aNewMarked;
    for (size_t nm =GetMarkedObjectCount(); nm > 0; )
    {
        // create Undo objects for all new objects
        // check for cancellation between the metafiles
        if (pProgrInfo != nullptr)
        {
            pProgrInfo->SetNextObject();
            if (!pProgrInfo->ReportActions(0))
                break;
        }

        --nm;
        SdrMark*     pM=GetSdrMarkByIndex(nm);
        SdrObject*   pObj=pM->GetMarkedSdrObj();
        SdrPageView* pPV=pM->GetPageView();
        SdrObjList*  pOL=pObj->getParentSdrObjListFromSdrObject();
        const size_t nInsPos=pObj->GetOrdNum()+1;
        sal_uIntPtr      nInsCnt=0;
        tools::Rectangle aLogicRect;

        SdrGrafObj*  pGraf = dynamic_cast<SdrGrafObj*>( pObj );
        if (pGraf != nullptr)
        {
            if (pGraf->HasGDIMetaFile() || pGraf->isEmbeddedVectorGraphicData())
            {
                GDIMetaFile aMetaFile(GetMetaFile(pGraf));
                if (aMetaFile.GetActionSize())
                {
                    aLogicRect = pGraf->GetLogicRect();
                    ImpSdrGDIMetaFileImport aFilter(*mpModel, pObj->GetLayer(), aLogicRect);
                    nInsCnt = aFilter.DoImport(aMetaFile, *pOL, nInsPos, pProgrInfo);
                }
            }
            else if (pGraf->isEmbeddedPdfData())
            {
#if HAVE_FEATURE_PDFIUM
                aLogicRect = pGraf->GetLogicRect();
                ImpSdrPdfImport aFilter(*mpModel, pObj->GetLayer(), aLogicRect, pGraf->getEmbeddedPdfData());
                if (pGraf->getEmbeddedPageNumber() < aFilter.GetPageCount())
                {
                    nInsCnt = aFilter.DoImport(*pOL, nInsPos, pGraf->getEmbeddedPageNumber(), pProgrInfo);
                }
#endif // HAVE_FEATURE_PDFIUM
            }
        }

        SdrOle2Obj* pOle2 = dynamic_cast<SdrOle2Obj*>(pObj);
        if (pOle2 != nullptr && pOle2->GetGraphic())
        {
            aLogicRect = pOle2->GetLogicRect();
            ImpSdrGDIMetaFileImport aFilter(*mpModel, pObj->GetLayer(), aLogicRect);
            nInsCnt = aFilter.DoImport(pOle2->GetGraphic()->GetGDIMetaFile(), *pOL, nInsPos, pProgrInfo);
        }

        if (nInsCnt != 0)
        {
            // transformation
            GeoStat aGeoStat(pGraf ? pGraf->GetGeoStat() : pOle2->GetGeoStat());
            size_t nObj = nInsPos;

            if (aGeoStat.nShearAngle)
                aGeoStat.RecalcTan();

            if (aGeoStat.nRotationAngle)
                aGeoStat.RecalcSinCos();

            for (sal_uIntPtr i = 0; i < nInsCnt; i++)
            {
                if (bUndo)
                    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoNewObject(*pOL->GetObj(nObj)));

                // update new MarkList
                SdrObject* pCandidate = pOL->GetObj(nObj);

                // apply original transformation
                if (aGeoStat.nShearAngle)
                    pCandidate->NbcShear(aLogicRect.TopLeft(), aGeoStat.nShearAngle, aGeoStat.nTan, false);

                if (aGeoStat.nRotationAngle)
                    pCandidate->NbcRotate(aLogicRect.TopLeft(), aGeoStat.nRotationAngle, aGeoStat.nSin, aGeoStat.nCos);

                SdrMark aNewMark(pCandidate, pPV);
                aNewMarked.InsertEntry(aNewMark);

                nObj++;
            }

            aForTheDescription.InsertEntry(*pM);

            if (bUndo)
                AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoDeleteObject(*pObj));

            // remove object from selection and delete
            GetMarkedObjectListWriteAccess().DeleteMark(TryToFindMarkedObject(pObj));
            pOL->RemoveObject(nInsPos-1);

            if (!bUndo)
                SdrObject::Free(pObj);
        }
    }

    if (aNewMarked.GetMarkCount())
    {
        // create new selection
        for (size_t a = 0; a < aNewMarked.GetMarkCount(); ++a)
        {
            GetMarkedObjectListWriteAccess().InsertEntry(*aNewMarked.GetMark(a));
        }

        SortMarkedObjects();
    }

    if (bUndo)
    {
        SetUndoComment(SvxResId(STR_EditImportMtf),aForTheDescription.GetMarkDescription());
        EndUndo();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
