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


#include <svx/svdglev.hxx>
#include <math.h>

#include <svx/svdundo.hxx>
#include <svx/strings.hrc>
#include <svx/dialmgr.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdglue.hxx>
#include <svx/svdtrans.hxx>
#include <svx/svdobj.hxx>

SdrGlueEditView::SdrGlueEditView(
    SdrModel& rSdrModel,
    OutputDevice* pOut)
:   SdrPolyEditView(rSdrModel, pOut)
{
}

SdrGlueEditView::~SdrGlueEditView()
{
}

void SdrGlueEditView::ImpDoMarkedGluePoints(PGlueDoFunc pDoFunc, bool bConst, const void* p1, const void* p2, const void* p3, const void* p4)
{
    const size_t nMarkCount=GetMarkedObjectCount();
    for (size_t nm=0; nm<nMarkCount; ++nm) {
        SdrMark* pM=GetSdrMarkByIndex(nm);
        SdrObject* pObj=pM->GetMarkedSdrObj();
        const SdrUShortCont& rPts = pM->GetMarkedGluePoints();
        if (!rPts.empty())
        {
            SdrGluePointList* pGPL=nullptr;
            if (bConst) {
                const SdrGluePointList* pConstGPL=pObj->GetGluePointList();
                pGPL=const_cast<SdrGluePointList*>(pConstGPL);
            } else {
                pGPL=pObj->ForceGluePointList();
            }
            if (pGPL!=nullptr)
            {
                if(!bConst && IsUndoEnabled() )
                    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pObj));

                for(sal_uInt16 nPtId : rPts)
                {
                    sal_uInt16 nGlueIdx=pGPL->FindGluePoint(nPtId);
                    if (nGlueIdx!=SDRGLUEPOINT_NOTFOUND)
                    {
                        SdrGluePoint& rGP=(*pGPL)[nGlueIdx];
                        (*pDoFunc)(rGP,pObj,p1,p2,p3,p4);
                    }
                }
                if (!bConst)
                {
                    pObj->SetChanged();
                    pObj->BroadcastObjectChange();
                }
            }
        }
    }
    if (!bConst && nMarkCount!=0) mpModel->SetChanged();
}


static void ImpGetEscDir(SdrGluePoint & rGP, const SdrObject* /*pObj*/, const void* pbFirst, const void* pnThisEsc, const void* pnRet, const void*)
{
    TriState& nRet=*const_cast<TriState *>(static_cast<TriState const *>(pnRet));
    if (nRet!=TRISTATE_INDET) {
        SdrEscapeDirection nEsc = rGP.GetEscDir();
        bool bOn = bool(nEsc & *static_cast<SdrEscapeDirection const *>(pnThisEsc));
        bool& bFirst=*const_cast<bool *>(static_cast<bool const *>(pbFirst));
        if (bFirst) {
            nRet = bOn ? TRISTATE_TRUE : TRISTATE_FALSE;
            bFirst = false;
        }
        else if (nRet != (bOn ? TRISTATE_TRUE : TRISTATE_FALSE)) nRet=TRISTATE_INDET;
    }
}

TriState SdrGlueEditView::IsMarkedGluePointsEscDir(SdrEscapeDirection nThisEsc) const
{
    ForceUndirtyMrkPnt();
    bool bFirst=true;
    TriState nRet=TRISTATE_FALSE;
    const_cast<SdrGlueEditView*>(this)->ImpDoMarkedGluePoints(ImpGetEscDir,true,&bFirst,&nThisEsc,&nRet);
    return nRet;
}

static void ImpSetEscDir(SdrGluePoint& rGP, const SdrObject* /*pObj*/, const void* pnThisEsc, const void* pbOn, const void*, const void*)
{
    SdrEscapeDirection nEsc=rGP.GetEscDir();
    if (*static_cast<bool const *>(pbOn))
        nEsc |= *static_cast<SdrEscapeDirection const *>(pnThisEsc);
    else
        nEsc &= ~*static_cast<SdrEscapeDirection const *>(pnThisEsc);
    rGP.SetEscDir(nEsc);
}

void SdrGlueEditView::SetMarkedGluePointsEscDir(SdrEscapeDirection nThisEsc, bool bOn)
{
    ForceUndirtyMrkPnt();
    BegUndo(SvxResId(STR_EditSetGlueEscDir),GetDescriptionOfMarkedGluePoints());
    ImpDoMarkedGluePoints(ImpSetEscDir,false,&nThisEsc,&bOn);
    EndUndo();
}


static void ImpGetPercent(SdrGluePoint & rGP, const SdrObject* /*pObj*/, const void* pbFirst, const void* pnRet, const void*, const void*)
{
    TriState& nRet=*const_cast<TriState *>(static_cast<TriState const *>(pnRet));
    if (nRet!=TRISTATE_INDET) {
        bool bOn=rGP.IsPercent();
        bool& bFirst=*const_cast<bool *>(static_cast<bool const *>(pbFirst));
        if (bFirst) {
            nRet = bOn ? TRISTATE_TRUE : TRISTATE_FALSE;
            bFirst = false;
        }
        else if ((nRet!=TRISTATE_FALSE)!=bOn)
            nRet=TRISTATE_INDET;
    }
}

TriState SdrGlueEditView::IsMarkedGluePointsPercent() const
{
    ForceUndirtyMrkPnt();
    bool bFirst=true;
    TriState nRet = TRISTATE_TRUE;
    const_cast<SdrGlueEditView*>(this)->ImpDoMarkedGluePoints(ImpGetPercent,true,&bFirst,&nRet);
    return nRet;
}

static void ImpSetPercent(SdrGluePoint& rGP, const SdrObject* pObj, const void* pbOn, const void*, const void*, const void*)
{
    Point aPos(rGP.GetAbsolutePos(*pObj));
    rGP.SetPercent(*static_cast<bool const *>(pbOn));
    rGP.SetAbsolutePos(aPos,*pObj);
}

void SdrGlueEditView::SetMarkedGluePointsPercent(bool bOn)
{
    ForceUndirtyMrkPnt();
    BegUndo(SvxResId(STR_EditSetGluePercent),GetDescriptionOfMarkedGluePoints());
    ImpDoMarkedGluePoints(ImpSetPercent,false,&bOn);
    EndUndo();
}


static void ImpGetAlign(SdrGluePoint & rGP, const SdrObject* /*pObj*/, const void* pbFirst, const void* pbDontCare, const void* pbVert, const void* pnRet)
{
    SdrAlign& nRet=*const_cast<SdrAlign *>(static_cast<SdrAlign const *>(pnRet));
    bool& bDontCare=*const_cast<bool *>(static_cast<bool const *>(pbDontCare));
    bool bVert=*static_cast<bool const *>(pbVert);
    if (!bDontCare) {
        SdrAlign nAlg=SdrAlign::NONE;
        if (bVert) {
            nAlg=rGP.GetVertAlign();
        } else {
            nAlg=rGP.GetHorzAlign();
        }
        bool& bFirst=*const_cast<bool *>(static_cast<bool const *>(pbFirst));
        if (bFirst) { nRet=nAlg; bFirst=false; }
        else if (nRet!=nAlg) {
            if (bVert) {
                nRet=SdrAlign::VERT_DONTCARE;
            } else {
                nRet=SdrAlign::HORZ_DONTCARE;
            }
            bDontCare=true;
        }
    }
}

SdrAlign SdrGlueEditView::GetMarkedGluePointsAlign(bool bVert) const
{
    ForceUndirtyMrkPnt();
    bool bFirst=true;
    bool bDontCare=false;
    SdrAlign nRet=SdrAlign::NONE;
    const_cast<SdrGlueEditView*>(this)->ImpDoMarkedGluePoints(ImpGetAlign,true,&bFirst,&bDontCare,&bVert,&nRet);
    return nRet;
}

static void ImpSetAlign(SdrGluePoint& rGP, const SdrObject* pObj, const void* pbVert, const void* pnAlign, const void*, const void*)
{
    Point aPos(rGP.GetAbsolutePos(*pObj));
    if (*static_cast<bool const *>(pbVert)) { // bVert?
        rGP.SetVertAlign(*static_cast<SdrAlign const *>(pnAlign));
    } else {
        rGP.SetHorzAlign(*static_cast<SdrAlign const *>(pnAlign));
    }
    rGP.SetAbsolutePos(aPos,*pObj);
}

void SdrGlueEditView::SetMarkedGluePointsAlign(bool bVert, SdrAlign nAlign)
{
    ForceUndirtyMrkPnt();
    BegUndo(SvxResId(STR_EditSetGlueAlign),GetDescriptionOfMarkedGluePoints());
    ImpDoMarkedGluePoints(ImpSetAlign,false,&bVert,&nAlign);
    EndUndo();
}

void SdrGlueEditView::DeleteMarkedGluePoints()
{
    BrkAction();
    ForceUndirtyMrkPnt();
    const bool bUndo = IsUndoEnabled();
    if( bUndo )
        BegUndo(SvxResId(STR_EditDelete),GetDescriptionOfMarkedGluePoints(),SdrRepeatFunc::Delete);

    const size_t nMarkCount=GetMarkedObjectCount();
    for (size_t nm=0; nm<nMarkCount; ++nm)
    {
        SdrMark* pM=GetSdrMarkByIndex(nm);
        SdrObject* pObj=pM->GetMarkedSdrObj();
        const SdrUShortCont& rPts = pM->GetMarkedGluePoints();
        if (!rPts.empty())
        {
            SdrGluePointList* pGPL=pObj->ForceGluePointList();
            if (pGPL!=nullptr)
            {
                if( bUndo )
                    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pObj));

                for(sal_uInt16 nPtId : rPts)
                {
                    sal_uInt16 nGlueIdx=pGPL->FindGluePoint(nPtId);
                    if (nGlueIdx!=SDRGLUEPOINT_NOTFOUND)
                    {
                        pGPL->Delete(nGlueIdx);
                    }
                }
                pObj->SetChanged();
                pObj->BroadcastObjectChange();
            }
        }
    }
    if( bUndo )
        EndUndo();
    UnmarkAllGluePoints();
    if (nMarkCount!=0)
        mpModel->SetChanged();
}


void SdrGlueEditView::ImpCopyMarkedGluePoints()
{
    const bool bUndo = IsUndoEnabled();

    if( bUndo )
        BegUndo();

    const size_t nMarkCount=GetMarkedObjectCount();
    for (size_t nm=0; nm<nMarkCount; ++nm)
    {
        SdrMark* pM=GetSdrMarkByIndex(nm);
        SdrObject* pObj=pM->GetMarkedSdrObj();
        SdrUShortCont& rPts = pM->GetMarkedGluePoints();
        SdrGluePointList* pGPL=pObj->ForceGluePointList();
        if (!rPts.empty() && pGPL!=nullptr)
        {
            if( bUndo )
                AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pObj));

            SdrUShortCont aIdsToErase;
            SdrUShortCont aIdsToInsert;
            for(sal_uInt16 nPtId : rPts)
            {
                sal_uInt16 nGlueIdx=pGPL->FindGluePoint(nPtId);
                if (nGlueIdx!=SDRGLUEPOINT_NOTFOUND)
                {
                    SdrGluePoint aNewGP((*pGPL)[nGlueIdx]);  // clone GluePoint
                    sal_uInt16 nNewIdx=pGPL->Insert(aNewGP);     // and insert it
                    sal_uInt16 nNewId=(*pGPL)[nNewIdx].GetId();  // retrieve ID of new GluePoints
                    aIdsToErase.insert(nPtId);               // select it (instead of the old one)
                    aIdsToInsert.insert(nNewId);
                }
            }
            for(const auto& rId : aIdsToErase)
                rPts.erase(rId);
            rPts.insert(aIdsToInsert.begin(), aIdsToInsert.end());
        }
    }
    if( bUndo )
        EndUndo();

    if (nMarkCount!=0)
        mpModel->SetChanged();
}


void SdrGlueEditView::ImpTransformMarkedGluePoints(PGlueTrFunc pTrFunc, const void* p1, const void* p2, const void* p3, const void* p4)
{
    const size_t nMarkCount=GetMarkedObjectCount();
    for (size_t nm=0; nm<nMarkCount; ++nm) {
        SdrMark* pM=GetSdrMarkByIndex(nm);
        SdrObject* pObj=pM->GetMarkedSdrObj();
        const SdrUShortCont& rPts = pM->GetMarkedGluePoints();
        if (!rPts.empty()) {
            SdrGluePointList* pGPL=pObj->ForceGluePointList();
            if (pGPL!=nullptr)
            {
                if( IsUndoEnabled() )
                    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pObj));

                for(sal_uInt16 nPtId : rPts)
                {
                    sal_uInt16 nGlueIdx=pGPL->FindGluePoint(nPtId);
                    if (nGlueIdx!=SDRGLUEPOINT_NOTFOUND) {
                        SdrGluePoint& rGP=(*pGPL)[nGlueIdx];
                        Point aPos(rGP.GetAbsolutePos(*pObj));
                        (*pTrFunc)(aPos,p1,p2,p3,p4);
                        rGP.SetAbsolutePos(aPos,*pObj);
                    }
                }
                pObj->SetChanged();
                pObj->BroadcastObjectChange();
            }
        }
    }
    if (nMarkCount!=0) mpModel->SetChanged();
}


static void ImpMove(Point& rPt, const void* p1, const void* /*p2*/, const void* /*p3*/, const void* /*p4*/)
{
    rPt.AdjustX(static_cast<const Size*>(p1)->Width() );
    rPt.AdjustY(static_cast<const Size*>(p1)->Height() );
}

void SdrGlueEditView::MoveMarkedGluePoints(const Size& rSiz, bool bCopy)
{
    ForceUndirtyMrkPnt();
    OUString aStr(SvxResId(STR_EditMove));
    if (bCopy) aStr += SvxResId(STR_EditWithCopy);
    BegUndo(aStr,GetDescriptionOfMarkedGluePoints(),SdrRepeatFunc::Move);
    if (bCopy) ImpCopyMarkedGluePoints();
    ImpTransformMarkedGluePoints(ImpMove,&rSiz);
    EndUndo();
    AdjustMarkHdl();
}


static void ImpResize(Point& rPt, const void* p1, const void* p2, const void* p3, const void* /*p4*/)
{
    ResizePoint(rPt,*static_cast<const Point*>(p1),*static_cast<const Fraction*>(p2),*static_cast<const Fraction*>(p3));
}

void SdrGlueEditView::ResizeMarkedGluePoints(const Point& rRef, const Fraction& xFact, const Fraction& yFact, bool bCopy)
{
    ForceUndirtyMrkPnt();
    OUString aStr(SvxResId(STR_EditResize));
    if (bCopy) aStr+=SvxResId(STR_EditWithCopy);
    BegUndo(aStr,GetDescriptionOfMarkedGluePoints(),SdrRepeatFunc::Resize);
    if (bCopy) ImpCopyMarkedGluePoints();
    ImpTransformMarkedGluePoints(ImpResize,&rRef,&xFact,&yFact);
    EndUndo();
    AdjustMarkHdl();
}


static void ImpRotate(Point& rPt, const void* p1, const void* /*p2*/, const void* p3, const void* p4)
{
    RotatePoint(rPt,*static_cast<const Point*>(p1),*static_cast<const double*>(p3),*static_cast<const double*>(p4));
}

void SdrGlueEditView::RotateMarkedGluePoints(const Point& rRef, long nAngle, bool bCopy)
{
    ForceUndirtyMrkPnt();
    OUString aStr(SvxResId(STR_EditRotate));
    if (bCopy) aStr+=SvxResId(STR_EditWithCopy);
    BegUndo(aStr,GetDescriptionOfMarkedGluePoints(),SdrRepeatFunc::Rotate);
    if (bCopy) ImpCopyMarkedGluePoints();
    double nSin = sin(nAngle * F_PI18000);
    double nCos = cos(nAngle * F_PI18000);
    ImpTransformMarkedGluePoints(ImpRotate,&rRef,&nAngle,&nSin,&nCos);
    EndUndo();
    AdjustMarkHdl();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
