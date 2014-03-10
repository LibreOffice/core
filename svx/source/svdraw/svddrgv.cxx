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


#include <svx/svddrgv.hxx>
#include "svx/xattr.hxx"
#include <svx/xpoly.hxx>
#include <svx/svdetc.hxx>
#include <svx/svdtrans.hxx>
#include <svx/svdundo.hxx>
#include <svx/svdocapt.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdoedge.hxx>
#include "svx/svdstr.hrc"
#include "svx/svdglob.hxx"
#include "svddrgm1.hxx"
#include <svx/obj3d.hxx>
#include <svx/svdoashp.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <svx/polypolygoneditor.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>

using namespace sdr;



// DragView



void SdrDragView::ImpClearVars()
{
    bFramDrag=false;
    eDragMode=SDRDRAG_MOVE;
    bDragLimit=false;
    bMarkedHitMovesAlways=false;
    eDragHdl=HDL_MOVE;
    pDragHdl=NULL;
    bDragHdl=false;
    bDragSpecial=false;
    mpCurrentSdrDragMethod=NULL;
    bDragStripes=false;
    bMirrRefDragObj=true;
    bDragWithCopy=false;
    pInsPointUndo=NULL;
    bInsGluePoint=false;
    bInsObjPointMode=false;
    bInsGluePointMode=false;
    nDragXorPolyLimit=100;
    nDragXorPointLimit=500;
    bNoDragXorPolys=false;
    bAutoVertexCon=true;
    bAutoCornerCon=false;
    bRubberEdgeDragging=true;
    bDetailedEdgeDragging=true;
    nDetailedEdgeDraggingLimit=10;
    bResizeAtCenter=false;
    bCrookAtCenter=false;
    bMouseHideWhileDraggingPoints=false;

    // init using default
    mbSolidDragging = getOptionsDrawinglayer().IsSolidDragCreate();
}

SdrDragView::SdrDragView(SdrModel* pModel1, OutputDevice* pOut)
:   SdrExchangeView(pModel1,pOut)
{
    ImpClearVars();
}

SdrDragView::~SdrDragView()
{
}

bool SdrDragView::IsAction() const
{
    return (mpCurrentSdrDragMethod || SdrExchangeView::IsAction());
}

void SdrDragView::MovAction(const Point& rPnt)
{
    SdrExchangeView::MovAction(rPnt);
    if (mpCurrentSdrDragMethod)
    {
        MovDragObj(rPnt);
    }
}

void SdrDragView::EndAction()
{
    if (mpCurrentSdrDragMethod)
    {
        EndDragObj(false);
    }
    SdrExchangeView::EndAction();
}

void SdrDragView::BckAction()
{
    SdrExchangeView::BckAction();
    BrkDragObj();
}

void SdrDragView::BrkAction()
{
    SdrExchangeView::BrkAction();
    BrkDragObj();
}

void SdrDragView::TakeActionRect(Rectangle& rRect) const
{
    if (mpCurrentSdrDragMethod)
    {
        rRect=aDragStat.GetActionRect();
        if (rRect.IsEmpty())
        {
            SdrPageView* pPV = GetSdrPageView();

            if(pPV&& pPV->HasMarkedObjPageView())
            {
                // #i95646# is this used..?
                const basegfx::B2DRange aBoundRange(mpCurrentSdrDragMethod->getCurrentRange());
                rRect = Rectangle(
                    basegfx::fround(aBoundRange.getMinX()), basegfx::fround(aBoundRange.getMinY()),
                    basegfx::fround(aBoundRange.getMaxX()), basegfx::fround(aBoundRange.getMaxY()));
            }
        }
        if (rRect.IsEmpty())
        {
            rRect=Rectangle(aDragStat.GetNow(),aDragStat.GetNow());
        }
    }
    else
    {
        SdrExchangeView::TakeActionRect(rRect);
    }
}

bool SdrDragView::TakeDragObjAnchorPos(Point& rPos, bool bTR ) const
{
    Rectangle aR;
    TakeActionRect(aR);
    rPos = bTR ? aR.TopRight() : aR.TopLeft();
    if (GetMarkedObjectCount()==1 && IsDragObj() && // only on single selection
        !IsDraggingPoints() && !IsDraggingGluePoints() && // not when moving points
        !mpCurrentSdrDragMethod->ISA(SdrDragMovHdl)) // not when moving handles
    {
        SdrObject* pObj=GetMarkedObjectByIndex(0);
        if (pObj->ISA(SdrCaptionObj))
        {
            Point aPt(((SdrCaptionObj*)pObj)->GetTailPos());
            bool bTail=eDragHdl==HDL_POLY; // drag tail
            sal_Bool bOwn=mpCurrentSdrDragMethod->ISA(SdrDragObjOwn); // specific to object
            if (!bTail)
            { // for bTail, TakeActionRect already does the right thing
                if (bOwn)
                { // bOwn may be MoveTextFrame, ResizeTextFrame, but may not (any more) be DragTail
                    rPos=aPt;
                }
                else
                {
                    // drag the whole Object (Move, Resize, ...)
                    const basegfx::B2DPoint aTransformed(mpCurrentSdrDragMethod->getCurrentTransformation() * basegfx::B2DPoint(aPt.X(), aPt.Y()));
                    rPos.X() = basegfx::fround(aTransformed.getX());
                    rPos.Y() = basegfx::fround(aTransformed.getY());
                }
            }
        }
        return true;
    }
    return false;
}



bool SdrDragView::TakeDragLimit(SdrDragMode /*eMode*/, Rectangle& /*rRect*/) const
{
    return false;
}

bool SdrDragView::BegDragObj(const Point& rPnt, OutputDevice* pOut, SdrHdl* pHdl, short nMinMov, SdrDragMethod* pForcedMeth)
{
    BrkAction();

    bool bRet=false;
    {
        SetDragWithCopy(false);
        //TODO: aAni.Reset();
        mpCurrentSdrDragMethod=NULL;
        bDragSpecial=false;
        bDragLimit=false;
        SdrDragMode eTmpMode=eDragMode;
        if (eTmpMode==SDRDRAG_MOVE && pHdl!=NULL && pHdl->GetKind()!=HDL_MOVE) {
            eTmpMode=SDRDRAG_RESIZE;
        }
        bDragLimit=TakeDragLimit(eTmpMode,aDragLimit);
        bFramDrag=ImpIsFrameHandles();
        if (!bFramDrag &&
            (pMarkedObj==NULL || !pMarkedObj->hasSpecialDrag()) &&
            (pHdl==NULL || pHdl->GetObj()==NULL)) {
            bFramDrag=true;
        }

        Point aPnt(rPnt);
        if(pHdl == NULL
            || pHdl->GetKind() == HDL_MOVE
            || pHdl->GetKind() == HDL_MIRX
            || pHdl->GetKind() == HDL_TRNS
            || pHdl->GetKind() == HDL_GRAD)
        {
            aDragStat.Reset(aPnt);
        }
        else
        {
            aDragStat.Reset(pHdl->GetPos());
        }

        aDragStat.SetView((SdrView*)this);
        aDragStat.SetPageView(pMarkedPV);  // <<-- DragPV has to go here!!!
        aDragStat.SetMinMove(ImpGetMinMovLogic(nMinMov,pOut));
        aDragStat.SetHdl(pHdl);
        aDragStat.NextPoint();
        pDragWin=pOut;
        pDragHdl=pHdl;
        eDragHdl= pHdl==NULL ? HDL_MOVE : pHdl->GetKind();
        bDragHdl=eDragHdl==HDL_REF1 || eDragHdl==HDL_REF2 || eDragHdl==HDL_MIRX;

        // Expand test for HDL_ANCHOR_TR
        bool bNotDraggable = (HDL_ANCHOR == eDragHdl || HDL_ANCHOR_TR == eDragHdl);

        if(pHdl && (pHdl->GetKind() == HDL_SMARTTAG) && pForcedMeth )
        {
            // just use the forced method for smart tags
        }
        else if(bDragHdl)
        {
            mpCurrentSdrDragMethod = new SdrDragMovHdl(*this);
        }
        else if(!bNotDraggable)
        {
            switch (eDragMode)
            {
                case SDRDRAG_ROTATE: case SDRDRAG_SHEAR: case SDRDRAG_DISTORT:
                {
                    switch (eDragHdl)
                    {
                        case HDL_LEFT:  case HDL_RIGHT:
                        case HDL_UPPER: case HDL_LOWER:
                        {
                            // are 3D objects selected?
                            bool b3DObjSelected = false;
                            for(sal_uInt32 a=0;!b3DObjSelected && a<GetMarkedObjectCount();a++)
                            {
                                SdrObject* pObj = GetMarkedObjectByIndex(a);
                                if(pObj && pObj->ISA(E3dObject))
                                    b3DObjSelected = true;
                            }
                            // If yes, allow shear even when !IsShearAllowed,
                            // because 3D objects are limited rotations
                            if (!b3DObjSelected && !IsShearAllowed())
                                return false;
                            mpCurrentSdrDragMethod = new SdrDragShear(*this,eDragMode==SDRDRAG_ROTATE);
                        } break;
                        case HDL_UPLFT: case HDL_UPRGT:
                        case HDL_LWLFT: case HDL_LWRGT:
                        {
                            if (eDragMode==SDRDRAG_SHEAR || eDragMode==SDRDRAG_DISTORT)
                            {
                                if (!IsDistortAllowed(true) && !IsDistortAllowed(false)) return false;
                                mpCurrentSdrDragMethod = new SdrDragDistort(*this);
                            }
                            else
                            {
                                if (!IsRotateAllowed(true)) return false;
                                mpCurrentSdrDragMethod = new SdrDragRotate(*this);
                            }
                        } break;
                        default:
                        {
                            if (IsMarkedHitMovesAlways() && eDragHdl==HDL_MOVE)
                            { // HDL_MOVE is true, even if Obj is hit directly
                                if (!IsMoveAllowed()) return false;
                                mpCurrentSdrDragMethod = new SdrDragMove(*this);
                            }
                            else
                            {
                                if (!IsRotateAllowed(true)) return false;
                                mpCurrentSdrDragMethod = new SdrDragRotate(*this);
                            }
                        }
                    }
                } break;
                case SDRDRAG_MIRROR:
                {
                    if (eDragHdl==HDL_MOVE && IsMarkedHitMovesAlways())
                    {
                        if (!IsMoveAllowed()) return false;
                        mpCurrentSdrDragMethod = new SdrDragMove(*this);
                    }
                    else
                    {
                        if (!IsMirrorAllowed(true,true)) return false;
                        mpCurrentSdrDragMethod = new SdrDragMirror(*this);
                    }
                } break;

                case SDRDRAG_CROP:
                {
                    if (eDragHdl==HDL_MOVE && IsMarkedHitMovesAlways())
                    {
                        if (!IsMoveAllowed())
                            return false;
                        mpCurrentSdrDragMethod = new SdrDragMove(*this);
                    }
                    else
                    {
                        if (!IsCrookAllowed(true) && !IsCrookAllowed(false))
                            return false;
                        mpCurrentSdrDragMethod = new SdrDragCrop(*this);
                    }
                }
                break;

                case SDRDRAG_TRANSPARENCE:
                {
                    if(eDragHdl == HDL_MOVE && IsMarkedHitMovesAlways())
                    {
                        if(!IsMoveAllowed())
                            return false;
                        mpCurrentSdrDragMethod = new SdrDragMove(*this);
                    }
                    else
                    {
                        if(!IsTransparenceAllowed())
                            return false;

                        mpCurrentSdrDragMethod = new SdrDragGradient(*this, false);
                    }
                    break;
                }
                case SDRDRAG_GRADIENT:
                {
                    if(eDragHdl == HDL_MOVE && IsMarkedHitMovesAlways())
                    {
                        if(!IsMoveAllowed())
                            return false;
                        mpCurrentSdrDragMethod = new SdrDragMove(*this);
                    }
                    else
                    {
                        if(!IsGradientAllowed())
                            return false;

                        mpCurrentSdrDragMethod = new SdrDragGradient(*this);
                    }
                    break;
                }

                case SDRDRAG_CROOK :
                {
                    if (eDragHdl==HDL_MOVE && IsMarkedHitMovesAlways())
                    {
                        if (!IsMoveAllowed()) return false;
                        mpCurrentSdrDragMethod = new SdrDragMove(*this);
                    }
                    else
                    {
                        if (!IsCrookAllowed(true) && !IsCrookAllowed(false)) return false;
                        mpCurrentSdrDragMethod = new SdrDragCrook(*this);
                    }
                } break;

                default:
                {
                    // SDRDRAG_MOVE
                    if((eDragHdl == HDL_MOVE) && !IsMoveAllowed())
                    {
                        return false;
                    }
                    else if(eDragHdl == HDL_GLUE)
                    {
                        mpCurrentSdrDragMethod = new SdrDragMove(*this);
                    }
                    else
                    {
                        if(bFramDrag)
                        {
                            if(eDragHdl == HDL_MOVE)
                            {
                                mpCurrentSdrDragMethod = new SdrDragMove(*this);
                            }
                            else
                            {
                                if(!IsResizeAllowed(true))
                                {
                                    return false;
                                }

                                bool bSingleTextObjMark = false;    // SJ: #i100490#
                                if ( GetMarkedObjectCount() == 1 )
                                {
                                    pMarkedObj=GetMarkedObjectByIndex(0);
                                    if ( pMarkedObj &&
                                        pMarkedObj->ISA( SdrTextObj ) &&
                                        static_cast<SdrTextObj*>(pMarkedObj)->IsTextFrame() )
                                        bSingleTextObjMark = true;
                                }
                                if ( bSingleTextObjMark )
                                    mpCurrentSdrDragMethod = new SdrDragObjOwn(*this);
                                else
                                    mpCurrentSdrDragMethod = new SdrDragResize(*this);
                            }
                        }
                        else
                        {
                            if(HDL_MOVE == eDragHdl)
                            {
                                const bool bCustomShapeSelected(1 == GetMarkedObjectCount() && GetMarkedObjectByIndex(0)->ISA(SdrObjCustomShape));

                                if(bCustomShapeSelected)
                                {
                                    mpCurrentSdrDragMethod = new SdrDragMove( *this );
                                }
                            }
                            else if(HDL_POLY == eDragHdl)
                            {
                                const bool bConnectorSelected(1 == GetMarkedObjectCount() && GetMarkedObjectByIndex(0)->ISA(SdrEdgeObj));

                                if(bConnectorSelected)
                                {
                                    // #i97784#
                                    // fallback to old behaviour for connectors (see
                                    // text in task description for more details)
                                }
                                else if(!IsMoveAllowed() || !IsResizeAllowed())
                                {
                                    // #i77187#
                                    // do not allow move of polygon points if object is move or size protected
                                    return false;
                                }
                            }

                            if(!mpCurrentSdrDragMethod)
                            {
                                // fallback to DragSpecial if no interaction defined
                                bDragSpecial = true;
                                mpCurrentSdrDragMethod = new SdrDragObjOwn(*this);
                            }
                        }
                    }
                }
            }
        }
        if (pForcedMeth!=NULL)
        {
            delete mpCurrentSdrDragMethod;
            mpCurrentSdrDragMethod = pForcedMeth;
        }
        aDragStat.SetDragMethod(mpCurrentSdrDragMethod);
        if (mpCurrentSdrDragMethod)
        {
            bRet = mpCurrentSdrDragMethod->BeginSdrDrag();
            if (!bRet)
            {
                if (pHdl==NULL && IS_TYPE(SdrDragObjOwn,mpCurrentSdrDragMethod))
                {
                    // Obj may not Move SpecialDrag, so try with MoveFrameDrag
                    delete mpCurrentSdrDragMethod;
                    mpCurrentSdrDragMethod = 0;
                    bDragSpecial=false;

                    if (!IsMoveAllowed())
                        return false;

                    bFramDrag=true;
                    mpCurrentSdrDragMethod = new SdrDragMove(*this);
                    aDragStat.SetDragMethod(mpCurrentSdrDragMethod);
                    bRet = mpCurrentSdrDragMethod->BeginSdrDrag();
                }
            }
            if (!bRet)
            {
                delete mpCurrentSdrDragMethod;
                mpCurrentSdrDragMethod = 0;
                aDragStat.SetDragMethod(mpCurrentSdrDragMethod);
            }
        }
    }

    return bRet;
}

void SdrDragView::MovDragObj(const Point& rPnt)
{
    if (mpCurrentSdrDragMethod)
    {
        Point aPnt(rPnt);
        ImpLimitToWorkArea(aPnt);
        mpCurrentSdrDragMethod->MoveSdrDrag(aPnt); // this call already makes a Hide()/Show combination
    }
}

bool SdrDragView::EndDragObj(bool bCopy)
{
    bool bRet(false);

    // #i73341# If inserting GluePoint, do not insist on last points being different
    if(mpCurrentSdrDragMethod && aDragStat.IsMinMoved() && (IsInsertGluePoint() || aDragStat.GetNow() != aDragStat.GetPrev()))
    {
        sal_uIntPtr nHdlAnzMerk=0;

        if (bEliminatePolyPoints)
        { // IBM Special
            nHdlAnzMerk=GetMarkablePointCount();
        }

        const bool bUndo = IsUndoEnabled();
        if (IsInsertGluePoint() && bUndo)
        {
            BegUndo(aInsPointUndoStr);
            AddUndo(pInsPointUndo);
        }

        bRet = mpCurrentSdrDragMethod->EndSdrDrag(bCopy);

        if( IsInsertGluePoint() && bUndo)
            EndUndo();

        delete mpCurrentSdrDragMethod;
        mpCurrentSdrDragMethod = 0;

        if (bEliminatePolyPoints)
        { // IBM Special
            if (nHdlAnzMerk!=GetMarkablePointCount())
            {
                UnmarkAllPoints();
            }
        }

        if (bInsPolyPoint)
        {
            SetMarkHandles();
            bInsPolyPoint=false;
            if( bUndo )
            {
                BegUndo(aInsPointUndoStr);
                AddUndo(pInsPointUndo);
                EndUndo();
            }
        }

        eDragHdl=HDL_MOVE;
        pDragHdl=NULL;

        if (!bSomeObjChgdFlag)
        {
            // Obj did not broadcast (e. g. Writer FlyFrames)
            if(!bDragHdl)
            {
                AdjustMarkHdl();
            }
        }
    }
    else
    {
        BrkDragObj();
    }

    bInsPolyPoint=false;
    SetInsertGluePoint(false);

    return bRet;
}

void SdrDragView::BrkDragObj()
{
    if (mpCurrentSdrDragMethod)
    {
        mpCurrentSdrDragMethod->CancelSdrDrag();

        delete mpCurrentSdrDragMethod;
        mpCurrentSdrDragMethod = 0;

        if (bInsPolyPoint)
        {
            pInsPointUndo->Undo(); // delete inserted point again
            delete pInsPointUndo;
            pInsPointUndo=NULL;
            SetMarkHandles();
            bInsPolyPoint=false;
        }

        if (IsInsertGluePoint())
        {
            pInsPointUndo->Undo(); // delete inserted glue point again
            delete pInsPointUndo;
            pInsPointUndo=NULL;
            SetInsertGluePoint(false);
        }

        eDragHdl=HDL_MOVE;
        pDragHdl=NULL;
    }
}

bool SdrDragView::IsInsObjPointPossible() const
{
    return pMarkedObj!=NULL && pMarkedObj->IsPolyObj();
}

bool SdrDragView::ImpBegInsObjPoint(bool bIdxZwang, sal_uInt32 nIdx, const Point& rPnt, bool bNewObj, OutputDevice* pOut)
{
    bool bRet(false);

    if(pMarkedObj && pMarkedObj->ISA(SdrPathObj))
    {
        SdrPathObj* pMarkedPath = (SdrPathObj*)pMarkedObj;
        BrkAction();
        pInsPointUndo = dynamic_cast< SdrUndoGeoObj* >( GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pMarkedObj) );
        DBG_ASSERT( pInsPointUndo, "svx::SdrDragView::BegInsObjPoint(), could not create correct undo object!" );

        OUString aStr(ImpGetResStr(STR_DragInsertPoint));

        aInsPointUndoStr = aStr.replaceFirst("%1", pMarkedObj->TakeObjNameSingul() );

        Point aPt(rPnt);

        if(bNewObj)
            aPt = GetSnapPos(aPt,pMarkedPV);

        bool bClosed0 = pMarkedPath->IsClosedObj();

        if(bIdxZwang)
        {
            mnInsPointNum = pMarkedPath->NbcInsPoint(nIdx, aPt, bNewObj, true);
        }
        else
        {
            mnInsPointNum = pMarkedPath->NbcInsPointOld(aPt, bNewObj, true);
        }

        if(bClosed0 != pMarkedPath->IsClosedObj())
        {
            // Obj was closed implicitly
            // object changed
            pMarkedPath->SetChanged();
            pMarkedPath->BroadcastObjectChange();
        }

        if(0xffffffff != mnInsPointNum)
        {
            bInsPolyPoint = true;
            UnmarkAllPoints();
            AdjustMarkHdl();

            bRet = BegDragObj(rPnt, pOut, aHdl.GetHdl(mnInsPointNum), 0);

            if (bRet)
            {
                aDragStat.SetMinMoved();
                MovDragObj(rPnt);
            }
        }
        else
        {
            delete pInsPointUndo;
            pInsPointUndo = NULL;
        }
    }

    return bRet;
}

bool SdrDragView::EndInsObjPoint(SdrCreateCmd eCmd)
{
    if(IsInsObjPoint())
    {
        sal_uInt32 nNextPnt(mnInsPointNum);
        Point aPnt(aDragStat.GetNow());
        bool bOk=EndDragObj(false);
        if (bOk && eCmd!=SDRCREATE_FORCEEND)
        {
            // Ret=True means: Action is over.
            bOk=!(ImpBegInsObjPoint(true, nNextPnt, aPnt, eCmd == SDRCREATE_NEXTOBJECT, pDragWin));
        }

        return bOk;
    } else return false;
}

bool SdrDragView::IsInsGluePointPossible() const
{
    bool bRet=false;
    if (IsInsGluePointMode() && AreObjectsMarked())
    {
        if (GetMarkedObjectCount()==1)
        {
            // return sal_False, if only 1 object which is a connector.
            const SdrObject* pObj=GetMarkedObjectByIndex(0);
            if (!HAS_BASE(SdrEdgeObj,pObj))
            {
               bRet=true;
            }
        }
        else
        {
            bRet=true;
        }
    }
    return bRet;
}

bool SdrDragView::BegInsGluePoint(const Point& rPnt)
{
    bool bRet=false;
    SdrObject* pObj;
    SdrPageView* pPV;
    sal_uIntPtr nMarkNum;
    if (PickMarkedObj(rPnt,pObj,pPV,&nMarkNum,SDRSEARCH_PASS2BOUND))
    {
        BrkAction();
        UnmarkAllGluePoints();
        pInsPointUndo= dynamic_cast< SdrUndoGeoObj* >( GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pObj) );
        DBG_ASSERT( pInsPointUndo, "svx::SdrDragView::BegInsObjPoint(), could not create correct undo object!" );
        OUString aStr(ImpGetResStr(STR_DragInsertGluePoint));

        aInsPointUndoStr = aStr.replaceFirst("%1", pObj->TakeObjNameSingul() );

        SdrGluePointList* pGPL=pObj->ForceGluePointList();
        if (pGPL!=NULL)
        {
            sal_uInt16 nGlueIdx=pGPL->Insert(SdrGluePoint());
            SdrGluePoint& rGP=(*pGPL)[nGlueIdx];
            sal_uInt16 nGlueId=rGP.GetId();
            rGP.SetAbsolutePos(rPnt,*pObj);

            SdrHdl* pHdl=NULL;
            if (MarkGluePoint(pObj,nGlueId,pPV))
            {
                pHdl=GetGluePointHdl(pObj,nGlueId);
            }
            if (pHdl!=NULL && pHdl->GetKind()==HDL_GLUE && pHdl->GetObj()==pObj && pHdl->GetObjHdlNum()==nGlueId)
            {
                SetInsertGluePoint(true);
                bRet=BegDragObj(rPnt,NULL,pHdl,0);
                if (bRet)
                {
                    aDragStat.SetMinMoved();
                    MovDragObj(rPnt);
                }
                else
                {
                    SetInsertGluePoint(false);
                    delete pInsPointUndo;
                    pInsPointUndo=NULL;
                }
            }
            else
            {
                OSL_FAIL("BegInsGluePoint(): GluePoint handle not found.");
            }
        }
        else
        {
            // no glue points possible for this object (e. g. Edge)
            SetInsertGluePoint(false);
            delete pInsPointUndo;
            pInsPointUndo=NULL;
        }
    }

    return bRet;
}

void SdrDragView::ShowDragObj()
{
    if(mpCurrentSdrDragMethod && !aDragStat.IsShown())
    {
        for(sal_uInt32 a(0); a < PaintWindowCount(); a++)
        {
            SdrPaintWindow* pCandidate = GetPaintWindow(a);
            rtl::Reference<sdr::overlay::OverlayManager> xOverlayManager = pCandidate->GetOverlayManager();

            if (xOverlayManager.is())
            {
                mpCurrentSdrDragMethod->CreateOverlayGeometry(*xOverlayManager);

                // #i101679# Force changed overlay to be shown
                xOverlayManager->flush();
            }
        }

        aDragStat.SetShown(true);
    }
}

void SdrDragView::HideDragObj()
{
    if(mpCurrentSdrDragMethod && aDragStat.IsShown())
    {
        mpCurrentSdrDragMethod->destroyOverlayGeometry();
        aDragStat.SetShown(false);
    }
}



void SdrDragView::SetNoDragXorPolys(bool bOn)
{
    if (IsNoDragXorPolys()!=bOn)
    {
        const bool bDragging(mpCurrentSdrDragMethod);
        const bool bShown(bDragging && aDragStat.IsShown());

        if(bShown)
        {
            HideDragObj();
        }

        bNoDragXorPolys = bOn;

        if(bDragging)
        {
            // force recreation of drag content
            mpCurrentSdrDragMethod->resetSdrDragEntries();
        }

        if(bShown)
        {
            ShowDragObj();
        }
    }
}

void SdrDragView::SetDragStripes(bool bOn)
{
    if (mpCurrentSdrDragMethod && aDragStat.IsShown())
    {
        HideDragObj();
        bDragStripes=bOn;
        ShowDragObj();
    }
    else
    {
        bDragStripes=bOn;
    }
}

bool SdrDragView::IsOrthoDesired() const
{
    if(mpCurrentSdrDragMethod && (IS_TYPE(SdrDragObjOwn, mpCurrentSdrDragMethod) || IS_TYPE(SdrDragResize, mpCurrentSdrDragMethod)))
    {
        return bOrthoDesiredOnMarked;
    }

    return false;
}

void SdrDragView::SetMarkHandles()
{
    if( pDragHdl )
        pDragHdl = 0;

    SdrExchangeView::SetMarkHandles();
}

void SdrDragView::SetSolidDragging(bool bOn)
{
    if((bool)mbSolidDragging != bOn)
    {
        mbSolidDragging = bOn;
    }
}

bool SdrDragView::IsSolidDragging() const
{
    // allow each user to disable by having a local setting, but using AND for
    // checking allowance
    return mbSolidDragging && getOptionsDrawinglayer().IsSolidDragCreate();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
