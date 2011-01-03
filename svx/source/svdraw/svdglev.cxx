/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <svx/svdglev.hxx>
#include <math.h>

#include <svx/svdundo.hxx>
#include "svx/svdstr.hrc"   // Namen aus der Resource
#include "svx/svdglob.hxx"  // StringCache
#include <svx/svdpagv.hxx>
#include <svx/svdglue.hxx>
#include <svx/svdtrans.hxx>
#include <svx/svdobj.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrGlueEditView::ImpDoMarkedGluePoints(PGlueDoFunc pDoFunc, BOOL bConst, const void* p1, const void* p2, const void* p3, const void* p4, const void* p5)
{
    ULONG nMarkAnz=GetMarkedObjectCount();
    for (ULONG nm=0; nm<nMarkAnz; nm++) {
        SdrMark* pM=GetSdrMarkByIndex(nm);
        SdrObject* pObj=pM->GetMarkedSdrObj();
        const SdrUShortCont* pPts=pM->GetMarkedGluePoints();
        ULONG nPtAnz=pPts==NULL ? 0 : pPts->GetCount();
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

                for (ULONG nPtNum=0; nPtNum<nPtAnz; nPtNum++)
                {
                    USHORT nPtId=pPts->GetObject(nPtNum);
                    USHORT nGlueIdx=pGPL->FindGluePoint(nPtId);
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

////////////////////////////////////////////////////////////////////////////////////////////////////

static void ImpGetEscDir(SdrGluePoint& rGP, const SdrObject* /*pObj*/, const void* pbFirst, const void* pnThisEsc, const void* pnRet, const void*, const void*)
{
    USHORT& nRet=*(USHORT*)pnRet;
    BOOL& bFirst=*(BOOL*)pbFirst;
    if (nRet!=FUZZY) {
        USHORT nEsc=rGP.GetEscDir();
        BOOL bOn=(nEsc & *(USHORT*)pnThisEsc)!=0;
        if (bFirst) { nRet=bOn; bFirst=FALSE; }
        else if (nRet!=bOn) nRet=FUZZY;
    }
}

TRISTATE SdrGlueEditView::IsMarkedGluePointsEscDir(USHORT nThisEsc) const
{
    ForceUndirtyMrkPnt();
    BOOL bFirst=TRUE;
    USHORT nRet=FALSE;
    ((SdrGlueEditView*)this)->ImpDoMarkedGluePoints(ImpGetEscDir,TRUE,&bFirst,&nThisEsc,&nRet);
    return (TRISTATE)nRet;
}

static void ImpSetEscDir(SdrGluePoint& rGP, const SdrObject* /*pObj*/, const void* pnThisEsc, const void* pbOn, const void*, const void*, const void*)
{
    USHORT nEsc=rGP.GetEscDir();
    if (*(BOOL*)pbOn) nEsc|=*(USHORT*)pnThisEsc;
    else nEsc&=~*(USHORT*)pnThisEsc;
    rGP.SetEscDir(nEsc);
}

void SdrGlueEditView::SetMarkedGluePointsEscDir(USHORT nThisEsc, BOOL bOn)
{
    ForceUndirtyMrkPnt();
    BegUndo(ImpGetResStr(STR_EditSetGlueEscDir),GetDescriptionOfMarkedGluePoints());
    ImpDoMarkedGluePoints(ImpSetEscDir,FALSE,&nThisEsc,&bOn);
    EndUndo();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

static void ImpGetPercent(SdrGluePoint& rGP, const SdrObject* /*pObj*/, const void* pbFirst, const void* pnRet, const void*, const void*, const void*)
{
    USHORT& nRet=*(USHORT*)pnRet;
    BOOL& bFirst=*(BOOL*)pbFirst;
    if (nRet!=FUZZY) {
        bool bOn=rGP.IsPercent();
        if (bFirst) { nRet=bOn; bFirst=FALSE; }
        else if ((nRet!=0)!=bOn) nRet=FUZZY;
    }
}

TRISTATE SdrGlueEditView::IsMarkedGluePointsPercent() const
{
    ForceUndirtyMrkPnt();
    BOOL bFirst=TRUE;
    USHORT nRet=TRUE;
    ((SdrGlueEditView*)this)->ImpDoMarkedGluePoints(ImpGetPercent,TRUE,&bFirst,&nRet);
    return (TRISTATE)nRet;
}

static void ImpSetPercent(SdrGluePoint& rGP, const SdrObject* pObj, const void* pbOn, const void*, const void*, const void*, const void*)
{
    Point aPos(rGP.GetAbsolutePos(*pObj));
    rGP.SetPercent(*(BOOL*)pbOn);
    rGP.SetAbsolutePos(aPos,*pObj);
}

void SdrGlueEditView::SetMarkedGluePointsPercent(BOOL bOn)
{
    ForceUndirtyMrkPnt();
    BegUndo(ImpGetResStr(STR_EditSetGluePercent),GetDescriptionOfMarkedGluePoints());
    ImpDoMarkedGluePoints(ImpSetPercent,FALSE,&bOn);
    EndUndo();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

static void ImpGetAlign(SdrGluePoint& rGP, const SdrObject* /*pObj*/, const void* pbFirst, const void* pbDontCare, const void* pbVert, const void* pnRet, const void*)
{
    USHORT& nRet=*(USHORT*)pnRet;
    BOOL& bFirst=*(BOOL*)pbFirst;
    BOOL& bDontCare=*(BOOL*)pbDontCare;
    BOOL bVert=*(BOOL*)pbVert;
    if (!bDontCare) {
        USHORT nAlg=0;
        if (bVert) {
            nAlg=rGP.GetVertAlign();
        } else {
            nAlg=rGP.GetHorzAlign();
        }
        if (bFirst) { nRet=nAlg; bFirst=FALSE; }
        else if (nRet!=nAlg) {
            if (bVert) {
                nRet=SDRVERTALIGN_DONTCARE;
            } else {
                nRet=SDRHORZALIGN_DONTCARE;
            }
            bDontCare=TRUE;
        }
    }
}

USHORT SdrGlueEditView::GetMarkedGluePointsAlign(BOOL bVert) const
{
    ForceUndirtyMrkPnt();
    BOOL bFirst=TRUE;
    BOOL bDontCare=FALSE;
    USHORT nRet=0;
    ((SdrGlueEditView*)this)->ImpDoMarkedGluePoints(ImpGetAlign,TRUE,&bFirst,&bDontCare,&bVert,&nRet);
    return nRet;
}

static void ImpSetAlign(SdrGluePoint& rGP, const SdrObject* pObj, const void* pbVert, const void* pnAlign, const void*, const void*, const void*)
{
    Point aPos(rGP.GetAbsolutePos(*pObj));
    if (*(BOOL*)pbVert) { // bVert?
        rGP.SetVertAlign(*(USHORT*)pnAlign);
    } else {
        rGP.SetHorzAlign(*(USHORT*)pnAlign);
    }
    rGP.SetAbsolutePos(aPos,*pObj);
}

void SdrGlueEditView::SetMarkedGluePointsAlign(BOOL bVert, USHORT nAlign)
{
    ForceUndirtyMrkPnt();
    BegUndo(ImpGetResStr(STR_EditSetGlueAlign),GetDescriptionOfMarkedGluePoints());
    ImpDoMarkedGluePoints(ImpSetAlign,FALSE,&bVert,&nAlign);
    EndUndo();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL SdrGlueEditView::IsDeleteMarkedGluePointsPossible() const
{
    return HasMarkedGluePoints();
}

void SdrGlueEditView::DeleteMarkedGluePoints()
{
    BrkAction();
    ForceUndirtyMrkPnt();
    const bool bUndo = IsUndoEnabled();
    if( bUndo )
        BegUndo(ImpGetResStr(STR_EditDelete),GetDescriptionOfMarkedGluePoints(),SDRREPFUNC_OBJ_DELETE);

    ULONG nMarkAnz=GetMarkedObjectCount();
    for (ULONG nm=0; nm<nMarkAnz; nm++)
    {
        SdrMark* pM=GetSdrMarkByIndex(nm);
        SdrObject* pObj=pM->GetMarkedSdrObj();
        const SdrUShortCont* pPts=pM->GetMarkedGluePoints();
        ULONG nPtAnz=pPts==NULL ? 0 : pPts->GetCount();
        if (nPtAnz!=0)
        {
            SdrGluePointList* pGPL=pObj->ForceGluePointList();
            if (pGPL!=NULL)
            {
                if( bUndo )
                    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pObj));

                for (ULONG nPtNum=0; nPtNum<nPtAnz; nPtNum++)
                {
                    USHORT nPtId=pPts->GetObject(nPtNum);
                    USHORT nGlueIdx=pGPL->FindGluePoint(nPtId);
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

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrGlueEditView::ImpCopyMarkedGluePoints()
{
    const bool bUndo = IsUndoEnabled();

    if( bUndo )
        BegUndo();

    ULONG nMarkAnz=GetMarkedObjectCount();
    for (ULONG nm=0; nm<nMarkAnz; nm++)
    {
        SdrMark* pM=GetSdrMarkByIndex(nm);
        SdrObject* pObj=pM->GetMarkedSdrObj();
        SdrUShortCont* pPts=pM->GetMarkedGluePoints();
        SdrGluePointList* pGPL=pObj->ForceGluePointList();
        ULONG nPtAnz=pPts==NULL ? 0 : pPts->GetCount();
        if (nPtAnz!=0 && pGPL!=NULL)
        {
            if( bUndo )
                AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pObj));

            for (ULONG nPtNum=0; nPtNum<nPtAnz; nPtNum++)
            {
                USHORT nPtId=pPts->GetObject(nPtNum);
                USHORT nGlueIdx=pGPL->FindGluePoint(nPtId);
                if (nGlueIdx!=SDRGLUEPOINT_NOTFOUND)
                {
                    SdrGluePoint aNewGP((*pGPL)[nGlueIdx]);  // GluePoint klonen
                    USHORT nNewIdx=pGPL->Insert(aNewGP);     // und einfuegen
                    USHORT nNewId=(*pGPL)[nNewIdx].GetId();  // Id des neuen GluePoints ermitteln
                    pPts->Replace(nNewId,nPtNum);            // und diesen markieren (anstelle des alten)
                }
            }
        }
    }
    if( bUndo )
        EndUndo();

    if (nMarkAnz!=0)
        pMod->SetChanged();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrGlueEditView::ImpTransformMarkedGluePoints(PGlueTrFunc pTrFunc, const void* p1, const void* p2, const void* p3, const void* p4, const void* p5)
{
    ULONG nMarkAnz=GetMarkedObjectCount();
    for (ULONG nm=0; nm<nMarkAnz; nm++) {
        SdrMark* pM=GetSdrMarkByIndex(nm);
        SdrObject* pObj=pM->GetMarkedSdrObj();
        const SdrUShortCont* pPts=pM->GetMarkedGluePoints();
        ULONG nPtAnz=pPts==NULL ? 0 : pPts->GetCount();
        if (nPtAnz!=0) {
            SdrGluePointList* pGPL=pObj->ForceGluePointList();
            if (pGPL!=NULL)
            {
                if( IsUndoEnabled() )
                    AddUndo(GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pObj));

                for (ULONG nPtNum=0; nPtNum<nPtAnz; nPtNum++) {
                    USHORT nPtId=pPts->GetObject(nPtNum);
                    USHORT nGlueIdx=pGPL->FindGluePoint(nPtId);
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

////////////////////////////////////////////////////////////////////////////////////////////////////

static void ImpMove(Point& rPt, const void* p1, const void* /*p2*/, const void* /*p3*/, const void* /*p4*/, const void* /*p5*/)
{
    rPt.X()+=((const Size*)p1)->Width();
    rPt.Y()+=((const Size*)p1)->Height();
}

void SdrGlueEditView::MoveMarkedGluePoints(const Size& rSiz, bool bCopy)
{
    ForceUndirtyMrkPnt();
    XubString aStr(ImpGetResStr(STR_EditMove));
    if (bCopy) aStr+=ImpGetResStr(STR_EditWithCopy);
    BegUndo(aStr,GetDescriptionOfMarkedGluePoints(),SDRREPFUNC_OBJ_MOVE);
    if (bCopy) ImpCopyMarkedGluePoints();
    ImpTransformMarkedGluePoints(ImpMove,&rSiz);
    EndUndo();
    AdjustMarkHdl();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

static void ImpResize(Point& rPt, const void* p1, const void* p2, const void* p3, const void* /*p4*/, const void* /*p5*/)
{
    ResizePoint(rPt,*(const Point*)p1,*(const Fraction*)p2,*(const Fraction*)p3);
}

void SdrGlueEditView::ResizeMarkedGluePoints(const Point& rRef, const Fraction& xFact, const Fraction& yFact, bool bCopy)
{
    ForceUndirtyMrkPnt();
    XubString aStr(ImpGetResStr(STR_EditResize));
    if (bCopy) aStr+=ImpGetResStr(STR_EditWithCopy);
    BegUndo(aStr,GetDescriptionOfMarkedGluePoints(),SDRREPFUNC_OBJ_RESIZE);
    if (bCopy) ImpCopyMarkedGluePoints();
    ImpTransformMarkedGluePoints(ImpResize,&rRef,&xFact,&yFact);
    EndUndo();
    AdjustMarkHdl();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

static void ImpRotate(Point& rPt, const void* p1, const void* /*p2*/, const void* p3, const void* p4, const void* /*p5*/)
{
    RotatePoint(rPt,*(const Point*)p1,*(const double*)p3,*(const double*)p4);
}

void SdrGlueEditView::RotateMarkedGluePoints(const Point& rRef, long nWink, bool bCopy)
{
    ForceUndirtyMrkPnt();
    XubString aStr(ImpGetResStr(STR_EditRotate));
    if (bCopy) aStr+=ImpGetResStr(STR_EditWithCopy);
    BegUndo(aStr,GetDescriptionOfMarkedGluePoints(),SDRREPFUNC_OBJ_ROTATE);
    if (bCopy) ImpCopyMarkedGluePoints();
    double nSin=sin(nWink*nPi180);
    double nCos=cos(nWink*nPi180);
    ImpTransformMarkedGluePoints(ImpRotate,&rRef,&nWink,&nSin,&nCos);
    EndUndo();
    AdjustMarkHdl();
}

