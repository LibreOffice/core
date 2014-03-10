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
#include "svx/svdstr.hrc"
#include "svx/svdglob.hxx"
#include <svx/svdpagv.hxx>
#include <svx/svdglue.hxx>
#include <svx/svdtrans.hxx>
#include <svx/svdobj.hxx>



void SdrGlueEditView::ImpClearVars()
{
}

SdrGlueEditView::SdrGlueEditView(SdrModel* pModel1, OutputDevice* pOut):
    SdrPolyEditView(pModel1,pOut)
{
    ImpClearVars();
}

SdrGlueEditView::~SdrGlueEditView()
{
}



void SdrGlueEditView::ImpDoMarkedGluePoints(PGlueDoFunc pDoFunc, bool bConst, const void* p1, const void* p2, const void* p3, const void* p4, const void* p5)
{
    sal_uIntPtr nMarkAnz=GetMarkedObjectCount();
    for (sal_uIntPtr nm=0; nm<nMarkAnz; nm++) {
        SdrMark* pM=GetSdrMarkByIndex(nm);
        SdrObject* pObj=pM->GetMarkedSdrObj();
        const SdrUShortCont* pPts=pM->GetMarkedGluePoints();
        sal_uIntPtr nPtAnz=pPts==NULL ? 0 : pPts->size();
        if (nPtAnz!=0) {
            SdrGluePointList* pGPL=NULL;
            if (bConst) {
                const SdrGluePointList* pConstGPL=pObj->GetGluePointList();
                pGPL=(SdrGluePointList*)pConstGPL;
            } else {
                pGPL=pObj->ForceGluePointList();
            }
            if (pGPL!=NULL)
            {
                if(!bConst && IsUndoEnabled() )
                    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pObj));

                for(SdrUShortCont::const_iterator it = pPts->begin(); it != pPts->end(); ++it)
                {
                    sal_uInt16 nPtId=*it;
                    sal_uInt16 nGlueIdx=pGPL->FindGluePoint(nPtId);
                    if (nGlueIdx!=SDRGLUEPOINT_NOTFOUND)
                    {
                        SdrGluePoint& rGP=(*pGPL)[nGlueIdx];
                        (*pDoFunc)(rGP,pObj,p1,p2,p3,p4,p5);
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
    if (!bConst && nMarkAnz!=0) pMod->SetChanged();
}



static void ImpGetEscDir(SdrGluePoint& rGP, const SdrObject* /*pObj*/, const void* pbFirst, const void* pnThisEsc, const void* pnRet, const void*, const void*)
{
    sal_uInt16& nRet=*(sal_uInt16*)pnRet;
    bool& bFirst=*(bool*)pbFirst;
    if (nRet!=FUZZY) {
        sal_uInt16 nEsc = rGP.GetEscDir();
        bool bOn = (nEsc & *(sal_uInt16*)pnThisEsc) != 0;
        if (bFirst) {
            nRet = bOn ? 1 : 0;
            bFirst = false;
        }
        else if (nRet != (bOn ? 1 : 0)) nRet=FUZZY;
    }
}

TRISTATE SdrGlueEditView::IsMarkedGluePointsEscDir(sal_uInt16 nThisEsc) const
{
    ForceUndirtyMrkPnt();
    bool bFirst=true;
    sal_uInt16 nRet=0;
    ((SdrGlueEditView*)this)->ImpDoMarkedGluePoints(ImpGetEscDir,true,&bFirst,&nThisEsc,&nRet);
    return (TRISTATE)nRet;
}

static void ImpSetEscDir(SdrGluePoint& rGP, const SdrObject* /*pObj*/, const void* pnThisEsc, const void* pbOn, const void*, const void*, const void*)
{
    sal_uInt16 nEsc=rGP.GetEscDir();
    if (*(bool*)pbOn) nEsc|=*(sal_uInt16*)pnThisEsc;
    else nEsc&=~*(sal_uInt16*)pnThisEsc;
    rGP.SetEscDir(nEsc);
}

void SdrGlueEditView::SetMarkedGluePointsEscDir(sal_uInt16 nThisEsc, bool bOn)
{
    ForceUndirtyMrkPnt();
    BegUndo(ImpGetResStr(STR_EditSetGlueEscDir),GetDescriptionOfMarkedGluePoints());
    ImpDoMarkedGluePoints(ImpSetEscDir,false,&nThisEsc,&bOn);
    EndUndo();
}



static void ImpGetPercent(SdrGluePoint& rGP, const SdrObject* /*pObj*/, const void* pbFirst, const void* pnRet, const void*, const void*, const void*)
{
    sal_uInt16& nRet=*(sal_uInt16*)pnRet;
    bool& bFirst=*(bool*)pbFirst;
    if (nRet!=FUZZY) {
        bool bOn=rGP.IsPercent();
        if (bFirst) { nRet=sal_uInt16(bOn); bFirst=false; }
        else if ((nRet!=0)!=bOn) nRet=FUZZY;
    }
}

TRISTATE SdrGlueEditView::IsMarkedGluePointsPercent() const
{
    ForceUndirtyMrkPnt();
    bool bFirst=true;
    sal_uInt16 nRet=sal_True;
    ((SdrGlueEditView*)this)->ImpDoMarkedGluePoints(ImpGetPercent,true,&bFirst,&nRet);
    return (TRISTATE)nRet;
}

static void ImpSetPercent(SdrGluePoint& rGP, const SdrObject* pObj, const void* pbOn, const void*, const void*, const void*, const void*)
{
    Point aPos(rGP.GetAbsolutePos(*pObj));
    rGP.SetPercent(*(bool*)pbOn);
    rGP.SetAbsolutePos(aPos,*pObj);
}

void SdrGlueEditView::SetMarkedGluePointsPercent(bool bOn)
{
    ForceUndirtyMrkPnt();
    BegUndo(ImpGetResStr(STR_EditSetGluePercent),GetDescriptionOfMarkedGluePoints());
    ImpDoMarkedGluePoints(ImpSetPercent,false,&bOn);
    EndUndo();
}



static void ImpGetAlign(SdrGluePoint& rGP, const SdrObject* /*pObj*/, const void* pbFirst, const void* pbDontCare, const void* pbVert, const void* pnRet, const void*)
{
    sal_uInt16& nRet=*(sal_uInt16*)pnRet;
    bool& bFirst=*(bool*)pbFirst;
    bool& bDontCare=*(bool*)pbDontCare;
    bool bVert=*(bool*)pbVert;
    if (!bDontCare) {
        sal_uInt16 nAlg=0;
        if (bVert) {
            nAlg=rGP.GetVertAlign();
        } else {
            nAlg=rGP.GetHorzAlign();
        }
        if (bFirst) { nRet=nAlg; bFirst=false; }
        else if (nRet!=nAlg) {
            if (bVert) {
                nRet=SDRVERTALIGN_DONTCARE;
            } else {
                nRet=SDRHORZALIGN_DONTCARE;
            }
            bDontCare=true;
        }
    }
}

sal_uInt16 SdrGlueEditView::GetMarkedGluePointsAlign(bool bVert) const
{
    ForceUndirtyMrkPnt();
    bool bFirst=true;
    bool bDontCare=false;
    sal_uInt16 nRet=0;
    ((SdrGlueEditView*)this)->ImpDoMarkedGluePoints(ImpGetAlign,true,&bFirst,&bDontCare,&bVert,&nRet);
    return nRet;
}

static void ImpSetAlign(SdrGluePoint& rGP, const SdrObject* pObj, const void* pbVert, const void* pnAlign, const void*, const void*, const void*)
{
    Point aPos(rGP.GetAbsolutePos(*pObj));
    if (*(bool*)pbVert) { // bVert?
        rGP.SetVertAlign(*(sal_uInt16*)pnAlign);
    } else {
        rGP.SetHorzAlign(*(sal_uInt16*)pnAlign);
    }
    rGP.SetAbsolutePos(aPos,*pObj);
}

void SdrGlueEditView::SetMarkedGluePointsAlign(bool bVert, sal_uInt16 nAlign)
{
    ForceUndirtyMrkPnt();
    BegUndo(ImpGetResStr(STR_EditSetGlueAlign),GetDescriptionOfMarkedGluePoints());
    ImpDoMarkedGluePoints(ImpSetAlign,false,&bVert,&nAlign);
    EndUndo();
}

void SdrGlueEditView::DeleteMarkedGluePoints()
{
    BrkAction();
    ForceUndirtyMrkPnt();
    const bool bUndo = IsUndoEnabled();
    if( bUndo )
        BegUndo(ImpGetResStr(STR_EditDelete),GetDescriptionOfMarkedGluePoints(),SDRREPFUNC_OBJ_DELETE);

    sal_uIntPtr nMarkAnz=GetMarkedObjectCount();
    for (sal_uIntPtr nm=0; nm<nMarkAnz; nm++)
    {
        SdrMark* pM=GetSdrMarkByIndex(nm);
        SdrObject* pObj=pM->GetMarkedSdrObj();
        const SdrUShortCont* pPts=pM->GetMarkedGluePoints();
        sal_uIntPtr nPtAnz=pPts==NULL ? 0 : pPts->size();
        if (nPtAnz!=0)
        {
            SdrGluePointList* pGPL=pObj->ForceGluePointList();
            if (pGPL!=NULL)
            {
                if( bUndo )
                    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pObj));

                for(SdrUShortCont::const_iterator it = pPts->begin(); it != pPts->end(); ++it)
                {
                    sal_uInt16 nPtId=*it;
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
    if (nMarkAnz!=0)
        pMod->SetChanged();
}



void SdrGlueEditView::ImpCopyMarkedGluePoints()
{
    const bool bUndo = IsUndoEnabled();

    if( bUndo )
        BegUndo();

    sal_uIntPtr nMarkAnz=GetMarkedObjectCount();
    for (sal_uIntPtr nm=0; nm<nMarkAnz; nm++)
    {
        SdrMark* pM=GetSdrMarkByIndex(nm);
        SdrObject* pObj=pM->GetMarkedSdrObj();
        SdrUShortCont* pPts=pM->GetMarkedGluePoints();
        SdrGluePointList* pGPL=pObj->ForceGluePointList();
        sal_uIntPtr nPtAnz=pPts==NULL ? 0 : pPts->size();
        if (nPtAnz!=0 && pGPL!=NULL)
        {
            if( bUndo )
                AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pObj));

            SdrUShortCont aIdsToErase;
            SdrUShortCont aIdsToInsert;
            for(SdrUShortCont::const_iterator it = pPts->begin(); it != pPts->end(); ++it)
            {
                sal_uInt16 nPtId=*it;
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
            for(SdrUShortCont::const_iterator it = aIdsToErase.begin(); it != aIdsToErase.end(); ++it)
                pPts->erase(*it);
            pPts->insert(aIdsToInsert.begin(), aIdsToInsert.end());

        }
    }
    if( bUndo )
        EndUndo();

    if (nMarkAnz!=0)
        pMod->SetChanged();
}



void SdrGlueEditView::ImpTransformMarkedGluePoints(PGlueTrFunc pTrFunc, const void* p1, const void* p2, const void* p3, const void* p4, const void* p5)
{
    sal_uIntPtr nMarkAnz=GetMarkedObjectCount();
    for (sal_uIntPtr nm=0; nm<nMarkAnz; nm++) {
        SdrMark* pM=GetSdrMarkByIndex(nm);
        SdrObject* pObj=pM->GetMarkedSdrObj();
        const SdrUShortCont* pPts=pM->GetMarkedGluePoints();
        sal_uIntPtr nPtAnz=pPts==NULL ? 0 : pPts->size();
        if (nPtAnz!=0) {
            SdrGluePointList* pGPL=pObj->ForceGluePointList();
            if (pGPL!=NULL)
            {
                if( IsUndoEnabled() )
                    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pObj));

                for(SdrUShortCont::const_iterator it = pPts->begin(); it != pPts->end(); ++it)
                {
                    sal_uInt16 nPtId=*it;
                    sal_uInt16 nGlueIdx=pGPL->FindGluePoint(nPtId);
                    if (nGlueIdx!=SDRGLUEPOINT_NOTFOUND) {
                        SdrGluePoint& rGP=(*pGPL)[nGlueIdx];
                        Point aPos(rGP.GetAbsolutePos(*pObj));
                        (*pTrFunc)(aPos,p1,p2,p3,p4,p5);
                        rGP.SetAbsolutePos(aPos,*pObj);
                    }
                }
                pObj->SetChanged();
                pObj->BroadcastObjectChange();
            }
        }
    }
    if (nMarkAnz!=0) pMod->SetChanged();
}



static void ImpMove(Point& rPt, const void* p1, const void* /*p2*/, const void* /*p3*/, const void* /*p4*/, const void* /*p5*/)
{
    rPt.X()+=((const Size*)p1)->Width();
    rPt.Y()+=((const Size*)p1)->Height();
}

void SdrGlueEditView::MoveMarkedGluePoints(const Size& rSiz, bool bCopy)
{
    ForceUndirtyMrkPnt();
    OUString aStr(ImpGetResStr(STR_EditMove));
    if (bCopy) aStr += ImpGetResStr(STR_EditWithCopy);
    BegUndo(aStr,GetDescriptionOfMarkedGluePoints(),SDRREPFUNC_OBJ_MOVE);
    if (bCopy) ImpCopyMarkedGluePoints();
    ImpTransformMarkedGluePoints(ImpMove,&rSiz);
    EndUndo();
    AdjustMarkHdl();
}



static void ImpResize(Point& rPt, const void* p1, const void* p2, const void* p3, const void* /*p4*/, const void* /*p5*/)
{
    ResizePoint(rPt,*(const Point*)p1,*(const Fraction*)p2,*(const Fraction*)p3);
}

void SdrGlueEditView::ResizeMarkedGluePoints(const Point& rRef, const Fraction& xFact, const Fraction& yFact, bool bCopy)
{
    ForceUndirtyMrkPnt();
    OUString aStr(ImpGetResStr(STR_EditResize));
    if (bCopy) aStr+=ImpGetResStr(STR_EditWithCopy);
    BegUndo(aStr,GetDescriptionOfMarkedGluePoints(),SDRREPFUNC_OBJ_RESIZE);
    if (bCopy) ImpCopyMarkedGluePoints();
    ImpTransformMarkedGluePoints(ImpResize,&rRef,&xFact,&yFact);
    EndUndo();
    AdjustMarkHdl();
}



static void ImpRotate(Point& rPt, const void* p1, const void* /*p2*/, const void* p3, const void* p4, const void* /*p5*/)
{
    RotatePoint(rPt,*(const Point*)p1,*(const double*)p3,*(const double*)p4);
}

void SdrGlueEditView::RotateMarkedGluePoints(const Point& rRef, long nWink, bool bCopy)
{
    ForceUndirtyMrkPnt();
    OUString aStr(ImpGetResStr(STR_EditRotate));
    if (bCopy) aStr+=ImpGetResStr(STR_EditWithCopy);
    BegUndo(aStr,GetDescriptionOfMarkedGluePoints(),SDRREPFUNC_OBJ_ROTATE);
    if (bCopy) ImpCopyMarkedGluePoints();
    double nSin=sin(nWink*nPi180);
    double nCos=cos(nWink*nPi180);
    ImpTransformMarkedGluePoints(ImpRotate,&rRef,&nWink,&nSin,&nCos);
    EndUndo();
    AdjustMarkHdl();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
