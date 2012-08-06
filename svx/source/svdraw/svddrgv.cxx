/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
// DragView
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrDragView::ImpClearVars()
{
    bFramDrag=sal_False;
    eDragMode=SDRDRAG_MOVE;
    bDragLimit=sal_False;
    bMarkedHitMovesAlways=sal_False;
    eDragHdl=HDL_MOVE;
    pDragHdl=NULL;
    bDragHdl=sal_False;
    bDragSpecial=sal_False;
    mpCurrentSdrDragMethod=NULL;
    bDragStripes=sal_False;
    bMirrRefDragObj=sal_True;
    bDragWithCopy=sal_False;
    pInsPointUndo=NULL;
    bInsGluePoint=sal_False;
    bInsObjPointMode=sal_False;
    bInsGluePointMode=sal_False;
    nDragXorPolyLimit=100;
    nDragXorPointLimit=500;
    bNoDragXorPolys=sal_False;
    bAutoVertexCon=sal_True;
    bAutoCornerCon=sal_False;
    bRubberEdgeDragging=sal_True;
    bDetailedEdgeDragging=sal_True;
    nDetailedEdgeDraggingLimit=10;
    bResizeAtCenter=sal_False;
    bCrookAtCenter=sal_False;
    bMouseHideWhileDraggingPoints=sal_False;

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

sal_Bool SdrDragView::IsAction() const
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
        EndDragObj(sal_False);
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

sal_Bool SdrDragView::TakeDragObjAnchorPos(Point& rPos, sal_Bool bTR ) const
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
            sal_Bool bTail=eDragHdl==HDL_POLY; // drag tail
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
        return sal_True;
    }
    return sal_False;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

sal_Bool SdrDragView::TakeDragLimit(SdrDragMode /*eMode*/, Rectangle& /*rRect*/) const
{
    return sal_False;
}

sal_Bool SdrDragView::BegDragObj(const Point& rPnt, OutputDevice* pOut, SdrHdl* pHdl, short nMinMov, SdrDragMethod* pForcedMeth)
{
    BrkAction();

    bool bRet=false;
    {
        SetDragWithCopy(sal_False);
        //TODO: aAni.Reset();
        mpCurrentSdrDragMethod=NULL;
        bDragSpecial=sal_False;
        bDragLimit=sal_False;
        SdrDragMode eTmpMode=eDragMode;
        if (eTmpMode==SDRDRAG_MOVE && pHdl!=NULL && pHdl->GetKind()!=HDL_MOVE) {
            eTmpMode=SDRDRAG_RESIZE;
        }
        bDragLimit=TakeDragLimit(eTmpMode,aDragLimit);
        bFramDrag=ImpIsFrameHandles();
        if (!bFramDrag &&
            (pMarkedObj==NULL || !pMarkedObj->hasSpecialDrag()) &&
            (pHdl==NULL || pHdl->GetObj()==NULL)) {
            bFramDrag=sal_True;
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
        sal_Bool bNotDraggable = (HDL_ANCHOR == eDragHdl || HDL_ANCHOR_TR == eDragHdl);

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
                            sal_Bool b3DObjSelected = sal_False;
                            for(sal_uInt32 a=0;!b3DObjSelected && a<GetMarkedObjectCount();a++)
                            {
                                SdrObject* pObj = GetMarkedObjectByIndex(a);
                                if(pObj && pObj->ISA(E3dObject))
                                    b3DObjSelected = sal_True;
                            }
                            // If yes, allow shear even when !IsShearAllowed,
                            // because 3D objects are limited rotations
                            if (!b3DObjSelected && !IsShearAllowed())
                                return sal_False;
                            mpCurrentSdrDragMethod = new SdrDragShear(*this,eDragMode==SDRDRAG_ROTATE);
                        } break;
                        case HDL_UPLFT: case HDL_UPRGT:
                        case HDL_LWLFT: case HDL_LWRGT:
                        {
                            if (eDragMode==SDRDRAG_SHEAR || eDragMode==SDRDRAG_DISTORT)
                            {
                                if (!IsDistortAllowed(sal_True) && !IsDistortAllowed(sal_False)) return sal_False;
                                mpCurrentSdrDragMethod = new SdrDragDistort(*this);
                            }
                            else
                            {
                                if (!IsRotateAllowed(sal_True)) return sal_False;
                                mpCurrentSdrDragMethod = new SdrDragRotate(*this);
                            }
                        } break;
                        default:
                        {
                            if (IsMarkedHitMovesAlways() && eDragHdl==HDL_MOVE)
                            { // HDL_MOVE is true, even if Obj is hit directly
                                if (!IsMoveAllowed()) return sal_False;
                                mpCurrentSdrDragMethod = new SdrDragMove(*this);
                            }
                            else
                            {
                                if (!IsRotateAllowed(sal_True)) return sal_False;
                                mpCurrentSdrDragMethod = new SdrDragRotate(*this);
                            }
                        }
                    }
                } break;
                case SDRDRAG_MIRROR:
                {
                    if (eDragHdl==HDL_MOVE && IsMarkedHitMovesAlways())
                    {
                        if (!IsMoveAllowed()) return sal_False;
                        mpCurrentSdrDragMethod = new SdrDragMove(*this);
                    }
                    else
                    {
                        if (!IsMirrorAllowed(sal_True,sal_True)) return sal_False;
                        mpCurrentSdrDragMethod = new SdrDragMirror(*this);
                    }
                } break;

                case SDRDRAG_CROP:
                {
                    if (eDragHdl==HDL_MOVE && IsMarkedHitMovesAlways())
                    {
                        if (!IsMoveAllowed())
                            return sal_False;
                        mpCurrentSdrDragMethod = new SdrDragMove(*this);
                    }
                    else
                    {
                        if (!IsCrookAllowed(sal_True) && !IsCrookAllowed(sal_False))
                            return sal_False;
                        mpCurrentSdrDragMethod = new SdrDragCrop(*this);
                    }
                }
                break;

                case SDRDRAG_TRANSPARENCE:
                {
                    if(eDragHdl == HDL_MOVE && IsMarkedHitMovesAlways())
                    {
                        if(!IsMoveAllowed())
                            return sal_False;
                        mpCurrentSdrDragMethod = new SdrDragMove(*this);
                    }
                    else
                    {
                        if(!IsTransparenceAllowed())
                            return sal_False;

                        mpCurrentSdrDragMethod = new SdrDragGradient(*this, sal_False);
                    }
                    break;
                }
                case SDRDRAG_GRADIENT:
                {
                    if(eDragHdl == HDL_MOVE && IsMarkedHitMovesAlways())
                    {
                        if(!IsMoveAllowed())
                            return sal_False;
                        mpCurrentSdrDragMethod = new SdrDragMove(*this);
                    }
                    else
                    {
                        if(!IsGradientAllowed())
                            return sal_False;

                        mpCurrentSdrDragMethod = new SdrDragGradient(*this);
                    }
                    break;
                }

                case SDRDRAG_CROOK :
                {
                    if (eDragHdl==HDL_MOVE && IsMarkedHitMovesAlways())
                    {
                        if (!IsMoveAllowed()) return sal_False;
                        mpCurrentSdrDragMethod = new SdrDragMove(*this);
                    }
                    else
                    {
                        if (!IsCrookAllowed(sal_True) && !IsCrookAllowed(sal_False)) return sal_False;
                        mpCurrentSdrDragMethod = new SdrDragCrook(*this);
                    }
                } break;

                default:
                {
                    // SDRDRAG_MOVE
                    if((eDragHdl == HDL_MOVE) && !IsMoveAllowed())
                    {
                        return sal_False;
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
                                if(!IsResizeAllowed(sal_True))
                                {
                                    return sal_False;
                                }

                                sal_Bool bSingleTextObjMark = sal_False;    // SJ: #i100490#
                                if ( GetMarkedObjectCount() == 1 )
                                {
                                    pMarkedObj=GetMarkedObjectByIndex(0);
                                    if ( pMarkedObj &&
                                        pMarkedObj->ISA( SdrTextObj ) &&
                                        static_cast<SdrTextObj*>(pMarkedObj)->IsTextFrame() )
                                        bSingleTextObjMark = sal_True;
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
                                    return sal_False;
                                }
                            }

                            if(!mpCurrentSdrDragMethod)
                            {
                                // fallback to DragSpecial if no interaction defined
                                bDragSpecial = sal_True;
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
                    bDragSpecial=sal_False;

                    if (!IsMoveAllowed())
                        return sal_False;

                    bFramDrag=sal_True;
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

sal_Bool SdrDragView::EndDragObj(sal_Bool bCopy)
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
            bInsPolyPoint=sal_False;
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

    bInsPolyPoint=sal_False;
    SetInsertGluePoint(sal_False);

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
            bInsPolyPoint=sal_False;
        }

        if (IsInsertGluePoint())
        {
            pInsPointUndo->Undo(); // delete inserted glue point again
            delete pInsPointUndo;
            pInsPointUndo=NULL;
            SetInsertGluePoint(sal_False);
        }

        eDragHdl=HDL_MOVE;
        pDragHdl=NULL;
    }
}

sal_Bool SdrDragView::IsInsObjPointPossible() const
{
    return pMarkedObj!=NULL && pMarkedObj->IsPolyObj();
}

sal_Bool SdrDragView::ImpBegInsObjPoint(sal_Bool bIdxZwang, sal_uInt32 nIdx, const Point& rPnt, sal_Bool bNewObj, OutputDevice* pOut)
{
    sal_Bool bRet(sal_False);

    if(pMarkedObj && pMarkedObj->ISA(SdrPathObj))
    {
        SdrPathObj* pMarkedPath = (SdrPathObj*)pMarkedObj;
        BrkAction();
        pInsPointUndo = dynamic_cast< SdrUndoGeoObj* >( GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pMarkedObj) );
        DBG_ASSERT( pInsPointUndo, "svx::SdrDragView::BegInsObjPoint(), could not create correct undo object!" );

        XubString aStr(ImpGetResStr(STR_DragInsertPoint));
        XubString aName;
        pMarkedObj->TakeObjNameSingul(aName);
        xub_StrLen nPos(aStr.SearchAscii("%1"));

        if(STRING_NOTFOUND != nPos)
        {
            aStr.Erase(nPos, 2);
            aStr.Insert(aName, nPos);
        }

        aInsPointUndoStr = aStr;
        Point aPt(rPnt);

        if(bNewObj)
            aPt = GetSnapPos(aPt,pMarkedPV);

        bool bClosed0 = pMarkedPath->IsClosedObj();

        if(bIdxZwang)
        {
            mnInsPointNum = pMarkedPath->NbcInsPoint(nIdx, aPt, bNewObj, sal_True);
        }
        else
        {
            mnInsPointNum = pMarkedPath->NbcInsPointOld(aPt, bNewObj, sal_True);
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
            bInsPolyPoint = sal_True;
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

sal_Bool SdrDragView::EndInsObjPoint(SdrCreateCmd eCmd)
{
    if(IsInsObjPoint())
    {
        sal_uInt32 nNextPnt(mnInsPointNum);
        Point aPnt(aDragStat.GetNow());
        sal_Bool bOk=EndDragObj(sal_False);
        if (bOk==sal_True && eCmd!=SDRCREATE_FORCEEND)
        {
            // Ret=True means: Action is over.
            bOk=!(ImpBegInsObjPoint(sal_True, nNextPnt, aPnt, eCmd == SDRCREATE_NEXTOBJECT, pDragWin));
        }

        return bOk;
    } else return sal_False;
}

sal_Bool SdrDragView::IsInsGluePointPossible() const
{
    sal_Bool bRet=sal_False;
    if (IsInsGluePointMode() && AreObjectsMarked())
    {
        if (GetMarkedObjectCount()==1)
        {
            // return sal_False, if only 1 object which is a connector.
            const SdrObject* pObj=GetMarkedObjectByIndex(0);
            if (!HAS_BASE(SdrEdgeObj,pObj))
            {
               bRet=sal_True;
            }
        }
        else
        {
            bRet=sal_True;
        }
    }
    return bRet;
}

sal_Bool SdrDragView::BegInsGluePoint(const Point& rPnt)
{
    sal_Bool bRet=sal_False;
    SdrObject* pObj;
    SdrPageView* pPV;
    sal_uIntPtr nMarkNum;
    if (PickMarkedObj(rPnt,pObj,pPV,&nMarkNum,SDRSEARCH_PASS2BOUND))
    {
        BrkAction();
        UnmarkAllGluePoints();
        pInsPointUndo= dynamic_cast< SdrUndoGeoObj* >( GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pObj) );
        DBG_ASSERT( pInsPointUndo, "svx::SdrDragView::BegInsObjPoint(), could not create correct undo object!" );
        XubString aStr(ImpGetResStr(STR_DragInsertGluePoint));
        XubString aName; pObj->TakeObjNameSingul(aName);

        aStr.SearchAndReplaceAscii("%1", aName);

        aInsPointUndoStr=aStr;
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
                SetInsertGluePoint(sal_True);
                bRet=BegDragObj(rPnt,NULL,pHdl,0);
                if (bRet)
                {
                    aDragStat.SetMinMoved();
                    MovDragObj(rPnt);
                }
                else
                {
                    SetInsertGluePoint(sal_False);
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
            SetInsertGluePoint(sal_False);
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

        aDragStat.SetShown(sal_True);
    }
}

void SdrDragView::HideDragObj()
{
    if(mpCurrentSdrDragMethod && aDragStat.IsShown())
    {
        mpCurrentSdrDragMethod->destroyOverlayGeometry();
        aDragStat.SetShown(sal_False);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrDragView::SetNoDragXorPolys(sal_Bool bOn)
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

void SdrDragView::SetDragStripes(sal_Bool bOn)
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

sal_Bool SdrDragView::IsOrthoDesired() const
{
    if(mpCurrentSdrDragMethod && (IS_TYPE(SdrDragObjOwn, mpCurrentSdrDragMethod) || IS_TYPE(SdrDragResize, mpCurrentSdrDragMethod)))
    {
        return bOrthoDesiredOnMarked;
    }

    return sal_False;
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
