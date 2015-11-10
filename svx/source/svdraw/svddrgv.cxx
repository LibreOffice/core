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
#include <svx/svdview.hxx>
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
#include "svdglob.hxx"
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
    mbFramDrag=false;
    meDragMode=SDRDRAG_MOVE;
    mbDragLimit=false;
    mbMarkedHitMovesAlways=false;
    meDragHdl=HDL_MOVE;
    mpDragHdl=nullptr;
    mbDragHdl=false;
    mbDragSpecial=false;
    mpCurrentSdrDragMethod=nullptr;
    mbDragStripes=false;
    mbMirrRefDragObj=true;
    mbDragWithCopy=false;
    mpInsPointUndo=nullptr;
    mbInsGluePoint=false;
    mbInsObjPointMode=false;
    mbInsGluePointMode=false;
    mnDragXorPolyLimit=100;
    mnDragXorPointLimit=500;
    mbNoDragXorPolys=false;
    mbAutoVertexCon=true;
    mbAutoCornerCon=false;
    mbRubberEdgeDragging=true;
    mbDetailedEdgeDragging=true;
    mnDetailedEdgeDraggingLimit=10;
    mbResizeAtCenter=false;
    mbCrookAtCenter=false;
    mbMouseHideWhileDraggingPoints=false;

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
        EndDragObj();
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
        rRect=maDragStat.GetActionRect();
        if (rRect.IsEmpty())
        {
            SdrPageView* pPV = GetSdrPageView();

            if(pPV&& pPV->HasMarkedObjPageView())
            {
                // #i95646# is this used..?
                const basegfx::B2DRange aBoundRange(mpCurrentSdrDragMethod->getCurrentRange());
                if (aBoundRange.isEmpty())
                {
                    rRect.SetEmpty();
                }
                else
                {
                    rRect = Rectangle(
                        basegfx::fround(aBoundRange.getMinX()), basegfx::fround(aBoundRange.getMinY()),
                        basegfx::fround(aBoundRange.getMaxX()), basegfx::fround(aBoundRange.getMaxY()));
                }
            }
        }
        if (rRect.IsEmpty())
        {
            rRect=Rectangle(maDragStat.GetNow(),maDragStat.GetNow());
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
        dynamic_cast<const SdrDragMovHdl*>( mpCurrentSdrDragMethod) ==  nullptr) // not when moving handles
    {
        SdrObject* pObj=GetMarkedObjectByIndex(0);
        if (dynamic_cast<const SdrCaptionObj*>( pObj) !=  nullptr)
        {
            Point aPt(static_cast<SdrCaptionObj*>(pObj)->GetTailPos());
            bool bTail=meDragHdl==HDL_POLY; // drag tail
            bool bOwn=dynamic_cast<const SdrDragObjOwn*>( mpCurrentSdrDragMethod) !=  nullptr; // specific to object
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
        mpCurrentSdrDragMethod=nullptr;
        mbDragSpecial=false;
        mbDragLimit=false;
        SdrDragMode eTmpMode=meDragMode;
        if (eTmpMode==SDRDRAG_MOVE && pHdl!=nullptr && pHdl->GetKind()!=HDL_MOVE) {
            eTmpMode=SDRDRAG_RESIZE;
        }
        mbDragLimit=TakeDragLimit(eTmpMode,maDragLimit);
        mbFramDrag=ImpIsFrameHandles();
        if (!mbFramDrag &&
            (mpMarkedObj==nullptr || !mpMarkedObj->hasSpecialDrag()) &&
            (pHdl==nullptr || pHdl->GetObj()==nullptr)) {
            mbFramDrag=true;
        }

        Point aPnt(rPnt);
        if(pHdl == nullptr
            || pHdl->GetKind() == HDL_MOVE
            || pHdl->GetKind() == HDL_MIRX
            || pHdl->GetKind() == HDL_TRNS
            || pHdl->GetKind() == HDL_GRAD)
        {
            maDragStat.Reset(aPnt);
        }
        else
        {
            maDragStat.Reset(pHdl->GetPos());
        }

        maDragStat.SetView(static_cast<SdrView*>(this));
        maDragStat.SetPageView(mpMarkedPV);  // <<-- DragPV has to go here!!!
        maDragStat.SetMinMove(ImpGetMinMovLogic(nMinMov,pOut));
        maDragStat.SetHdl(pHdl);
        maDragStat.NextPoint();
        mpDragWin=pOut;
        mpDragHdl=pHdl;
        meDragHdl= pHdl==nullptr ? HDL_MOVE : pHdl->GetKind();
        mbDragHdl=meDragHdl==HDL_REF1 || meDragHdl==HDL_REF2 || meDragHdl==HDL_MIRX;

        // Expand test for HDL_ANCHOR_TR
        bool bNotDraggable = (HDL_ANCHOR == meDragHdl || HDL_ANCHOR_TR == meDragHdl);

        if(pHdl && (pHdl->GetKind() == HDL_SMARTTAG) && pForcedMeth )
        {
            // just use the forced method for smart tags
        }
        else if(mbDragHdl)
        {
            mpCurrentSdrDragMethod = new SdrDragMovHdl(*this);
        }
        else if(!bNotDraggable)
        {
            switch (meDragMode)
            {
                case SDRDRAG_ROTATE: case SDRDRAG_SHEAR: case SDRDRAG_DISTORT:
                {
                    switch (meDragHdl)
                    {
                        case HDL_LEFT:  case HDL_RIGHT:
                        case HDL_UPPER: case HDL_LOWER:
                        {
                            // are 3D objects selected?
                            bool b3DObjSelected = false;
                            for(size_t a=0; !b3DObjSelected && a<GetMarkedObjectCount(); ++a)
                            {
                                SdrObject* pObj = GetMarkedObjectByIndex(a);
                                if(pObj && dynamic_cast< const E3dObject* >(pObj) !=  nullptr)
                                    b3DObjSelected = true;
                            }
                            // If yes, allow shear even when !IsShearAllowed,
                            // because 3D objects are limited rotations
                            if (!b3DObjSelected && !IsShearAllowed())
                                return false;
                            mpCurrentSdrDragMethod = new SdrDragShear(*this,meDragMode==SDRDRAG_ROTATE);
                        } break;
                        case HDL_UPLFT: case HDL_UPRGT:
                        case HDL_LWLFT: case HDL_LWRGT:
                        {
                            if (meDragMode==SDRDRAG_SHEAR || meDragMode==SDRDRAG_DISTORT)
                            {
                                if (!IsDistortAllowed(true) && !IsDistortAllowed()) return false;
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
                            if (IsMarkedHitMovesAlways() && meDragHdl==HDL_MOVE)
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
                    if (meDragHdl==HDL_MOVE && IsMarkedHitMovesAlways())
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
                    if (meDragHdl==HDL_MOVE && IsMarkedHitMovesAlways())
                    {
                        if (!IsMoveAllowed())
                            return false;
                        mpCurrentSdrDragMethod = new SdrDragMove(*this);
                    }
                    else
                    {
                        if (!IsCropAllowed())
                            return false;
                        mpCurrentSdrDragMethod = new SdrDragCrop(*this);
                    }
                }
                break;

                case SDRDRAG_TRANSPARENCE:
                {
                    if(meDragHdl == HDL_MOVE && IsMarkedHitMovesAlways())
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
                    if(meDragHdl == HDL_MOVE && IsMarkedHitMovesAlways())
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
                    if (meDragHdl==HDL_MOVE && IsMarkedHitMovesAlways())
                    {
                        if (!IsMoveAllowed()) return false;
                        mpCurrentSdrDragMethod = new SdrDragMove(*this);
                    }
                    else
                    {
                        if (!IsCrookAllowed(true) && !IsCrookAllowed()) return false;
                        mpCurrentSdrDragMethod = new SdrDragCrook(*this);
                    }
                } break;

                default:
                {
                    // SDRDRAG_MOVE
                    if((meDragHdl == HDL_MOVE) && !IsMoveAllowed())
                    {
                        return false;
                    }
                    else if(meDragHdl == HDL_GLUE)
                    {
                        mpCurrentSdrDragMethod = new SdrDragMove(*this);
                    }
                    else
                    {
                        if(mbFramDrag)
                        {
                            if(meDragHdl == HDL_MOVE)
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
                                    mpMarkedObj=GetMarkedObjectByIndex(0);
                                    if ( mpMarkedObj &&
                                        dynamic_cast<const SdrTextObj*>( mpMarkedObj) !=  nullptr &&
                                        static_cast<SdrTextObj*>(mpMarkedObj)->IsTextFrame() )
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
                            if(HDL_MOVE == meDragHdl)
                            {
                                const bool bCustomShapeSelected(1 == GetMarkedObjectCount() && dynamic_cast<const SdrObjCustomShape*>(GetMarkedObjectByIndex(0)) != nullptr);

                                if(bCustomShapeSelected)
                                {
                                    mpCurrentSdrDragMethod = new SdrDragMove( *this );
                                }
                            }
                            else if(HDL_POLY == meDragHdl)
                            {
                                const bool bConnectorSelected(1 == GetMarkedObjectCount() && dynamic_cast<const SdrEdgeObj*>(GetMarkedObjectByIndex(0)) != nullptr);

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
                                mbDragSpecial = true;
                                mpCurrentSdrDragMethod = new SdrDragObjOwn(*this);
                            }
                        }
                    }
                }
            }
        }
        if (pForcedMeth!=nullptr)
        {
            delete mpCurrentSdrDragMethod;
            mpCurrentSdrDragMethod = pForcedMeth;
        }
        maDragStat.SetDragMethod(mpCurrentSdrDragMethod);
        if (mpCurrentSdrDragMethod)
        {
            bRet = mpCurrentSdrDragMethod->BeginSdrDrag();
            if (!bRet)
            {
                if (pHdl==nullptr && dynamic_cast< const SdrDragObjOwn* >(mpCurrentSdrDragMethod) !=  nullptr)
                {
                    // Obj may not Move SpecialDrag, so try with MoveFrameDrag
                    delete mpCurrentSdrDragMethod;
                    mpCurrentSdrDragMethod = nullptr;
                    mbDragSpecial=false;

                    if (!IsMoveAllowed())
                        return false;

                    mbFramDrag=true;
                    mpCurrentSdrDragMethod = new SdrDragMove(*this);
                    maDragStat.SetDragMethod(mpCurrentSdrDragMethod);
                    bRet = mpCurrentSdrDragMethod->BeginSdrDrag();
                }
            }
            if (!bRet)
            {
                delete mpCurrentSdrDragMethod;
                mpCurrentSdrDragMethod = nullptr;
                maDragStat.SetDragMethod(mpCurrentSdrDragMethod);
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
    if(mpCurrentSdrDragMethod && maDragStat.IsMinMoved() && (IsInsertGluePoint() || maDragStat.GetNow() != maDragStat.GetPrev()))
    {
        sal_uIntPtr nSavedHdlCount=0;

        if (bEliminatePolyPoints)
        {
            nSavedHdlCount=GetMarkablePointCount();
        }

        const bool bUndo = IsUndoEnabled();
        if (IsInsertGluePoint() && bUndo)
        {
            BegUndo(maInsPointUndoStr);
            AddUndo(mpInsPointUndo);
        }

        bRet = mpCurrentSdrDragMethod->EndSdrDrag(bCopy);

        if( IsInsertGluePoint() && bUndo)
            EndUndo();

        delete mpCurrentSdrDragMethod;
        mpCurrentSdrDragMethod = nullptr;

        if (bEliminatePolyPoints)
        {
            if (nSavedHdlCount!=GetMarkablePointCount())
            {
                UnmarkAllPoints();
            }
        }

        if (mbInsPolyPoint)
        {
            SetMarkHandles();
            mbInsPolyPoint=false;
            if( bUndo )
            {
                BegUndo(maInsPointUndoStr);
                AddUndo(mpInsPointUndo);
                EndUndo();
            }
        }

        meDragHdl=HDL_MOVE;
        mpDragHdl=nullptr;

        if (!mbSomeObjChgdFlag)
        {
            // Obj did not broadcast (e. g. Writer FlyFrames)
            if(!mbDragHdl)
            {
                AdjustMarkHdl();
            }
        }
    }
    else
    {
        BrkDragObj();
    }

    mbInsPolyPoint=false;
    SetInsertGluePoint(false);

    return bRet;
}

void SdrDragView::BrkDragObj()
{
    if (mpCurrentSdrDragMethod)
    {
        mpCurrentSdrDragMethod->CancelSdrDrag();

        delete mpCurrentSdrDragMethod;
        mpCurrentSdrDragMethod = nullptr;

        if (mbInsPolyPoint)
        {
            mpInsPointUndo->Undo(); // delete inserted point again
            delete mpInsPointUndo;
            mpInsPointUndo=nullptr;
            SetMarkHandles();
            mbInsPolyPoint=false;
        }

        if (IsInsertGluePoint())
        {
            mpInsPointUndo->Undo(); // delete inserted glue point again
            delete mpInsPointUndo;
            mpInsPointUndo=nullptr;
            SetInsertGluePoint(false);
        }

        meDragHdl=HDL_MOVE;
        mpDragHdl=nullptr;
    }
}

bool SdrDragView::IsInsObjPointPossible() const
{
    return mpMarkedObj!=nullptr && mpMarkedObj->IsPolyObj();
}

bool SdrDragView::ImpBegInsObjPoint(bool bIdxZwang, sal_uInt32 nIdx, const Point& rPnt, bool bNewObj, OutputDevice* pOut)
{
    bool bRet(false);

    if(mpMarkedObj && dynamic_cast<const SdrPathObj*>( mpMarkedObj) !=  nullptr)
    {
        SdrPathObj* pMarkedPath = static_cast<SdrPathObj*>(mpMarkedObj);
        BrkAction();
        mpInsPointUndo = dynamic_cast< SdrUndoGeoObj* >( GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*mpMarkedObj) );
        DBG_ASSERT( mpInsPointUndo, "svx::SdrDragView::BegInsObjPoint(), could not create correct undo object!" );

        OUString aStr(ImpGetResStr(STR_DragInsertPoint));

        maInsPointUndoStr = aStr.replaceFirst("%1", mpMarkedObj->TakeObjNameSingul() );

        Point aPt(rPnt);

        if(bNewObj)
            aPt = GetSnapPos(aPt,mpMarkedPV);

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
            mbInsPolyPoint = true;
            UnmarkAllPoints();
            AdjustMarkHdl();

            bRet = BegDragObj(rPnt, pOut, maHdlList.GetHdl(mnInsPointNum), 0);

            if (bRet)
            {
                maDragStat.SetMinMoved();
                MovDragObj(rPnt);
            }
        }
        else
        {
            delete mpInsPointUndo;
            mpInsPointUndo = nullptr;
        }
    }

    return bRet;
}

bool SdrDragView::EndInsObjPoint(SdrCreateCmd eCmd)
{
    if(IsInsObjPoint())
    {
        sal_uInt32 nNextPnt(mnInsPointNum);
        Point aPnt(maDragStat.GetNow());
        bool bOk=EndDragObj();
        if (bOk && eCmd!=SDRCREATE_FORCEEND)
        {
            // Ret=True means: Action is over.
            bOk=!(ImpBegInsObjPoint(true, nNextPnt, aPnt, eCmd == SDRCREATE_NEXTOBJECT, mpDragWin));
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
    if (PickMarkedObj(rPnt,pObj,pPV,SdrSearchOptions::PASS2BOUND))
    {
        BrkAction();
        UnmarkAllGluePoints();
        mpInsPointUndo= dynamic_cast< SdrUndoGeoObj* >( GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pObj) );
        DBG_ASSERT( mpInsPointUndo, "svx::SdrDragView::BegInsObjPoint(), could not create correct undo object!" );
        OUString aStr(ImpGetResStr(STR_DragInsertGluePoint));

        maInsPointUndoStr = aStr.replaceFirst("%1", pObj->TakeObjNameSingul() );

        SdrGluePointList* pGPL=pObj->ForceGluePointList();
        if (pGPL!=nullptr)
        {
            sal_uInt16 nGlueIdx=pGPL->Insert(SdrGluePoint());
            SdrGluePoint& rGP=(*pGPL)[nGlueIdx];
            sal_uInt16 nGlueId=rGP.GetId();
            rGP.SetAbsolutePos(rPnt,*pObj);

            SdrHdl* pHdl=nullptr;
            if (MarkGluePoint(pObj,nGlueId,pPV))
            {
                pHdl=GetGluePointHdl(pObj,nGlueId);
            }
            if (pHdl!=nullptr && pHdl->GetKind()==HDL_GLUE && pHdl->GetObj()==pObj && pHdl->GetObjHdlNum()==nGlueId)
            {
                SetInsertGluePoint(true);
                bRet=BegDragObj(rPnt,nullptr,pHdl,0);
                if (bRet)
                {
                    maDragStat.SetMinMoved();
                    MovDragObj(rPnt);
                }
                else
                {
                    SetInsertGluePoint(false);
                    delete mpInsPointUndo;
                    mpInsPointUndo=nullptr;
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
            delete mpInsPointUndo;
            mpInsPointUndo=nullptr;
        }
    }

    return bRet;
}

void SdrDragView::ShowDragObj()
{
    if(mpCurrentSdrDragMethod && !maDragStat.IsShown())
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

        maDragStat.SetShown(true);
    }
}

void SdrDragView::HideDragObj()
{
    if(mpCurrentSdrDragMethod && maDragStat.IsShown())
    {
        mpCurrentSdrDragMethod->destroyOverlayGeometry();
        maDragStat.SetShown(false);
    }
}



void SdrDragView::SetNoDragXorPolys(bool bOn)
{
    if (IsNoDragXorPolys()!=bOn)
    {
        const bool bDragging(mpCurrentSdrDragMethod);
        const bool bShown(bDragging && maDragStat.IsShown());

        if(bShown)
        {
            HideDragObj();
        }

        mbNoDragXorPolys = bOn;

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
    if (mpCurrentSdrDragMethod && maDragStat.IsShown())
    {
        HideDragObj();
        mbDragStripes=bOn;
        ShowDragObj();
    }
    else
    {
        mbDragStripes=bOn;
    }
}

bool SdrDragView::IsOrthoDesired() const
{
    if(mpCurrentSdrDragMethod && (dynamic_cast< const SdrDragObjOwn* >( mpCurrentSdrDragMethod) !=  nullptr
                                                || dynamic_cast< const SdrDragResize* >(mpCurrentSdrDragMethod) !=  nullptr))
    {
        return bOrthoDesiredOnMarked;
    }

    return false;
}

void SdrDragView::SetMarkHandles()
{
    if( mpDragHdl )
        mpDragHdl = nullptr;

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
