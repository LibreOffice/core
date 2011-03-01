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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <svx/svddrgv.hxx>
#include "xattr.hxx"
#include <svx/xpoly.hxx>
#include <svx/svdetc.hxx>
#include <svx/svdtrans.hxx>
#include <svx/svdundo.hxx>
#include <svx/svdocapt.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdoedge.hxx>
#include "svdstr.hrc"
#include "svdglob.hxx"
#include "svddrgm1.hxx"
#include <svx/obj3d.hxx>
#include <svx/svdoashp.hxx>
#include <sdrpaintwindow.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <svx/polypolygoneditor.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>

using namespace sdr;

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  @@@@@  @@@@@   @@@@   @@@@   @@ @@ @@ @@@@@ @@   @@
//  @@  @@ @@  @@ @@  @@ @@  @@  @@ @@ @@ @@    @@   @@
//  @@  @@ @@  @@ @@  @@ @@      @@ @@ @@ @@    @@ @ @@
//  @@  @@ @@@@@  @@@@@@ @@ @@@  @@@@@ @@ @@@@  @@@@@@@
//  @@  @@ @@  @@ @@  @@ @@  @@   @@@  @@ @@    @@@@@@@
//  @@  @@ @@  @@ @@  @@ @@  @@   @@@  @@ @@    @@@ @@@
//  @@@@@  @@  @@ @@  @@  @@@@@    @   @@ @@@@@ @@   @@
//
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrDragView::ImpClearVars()
{
    bFramDrag=FALSE;
    eDragMode=SDRDRAG_MOVE;
    bDragLimit=FALSE;
    bMarkedHitMovesAlways=FALSE;
    eDragHdl=HDL_MOVE;
    pDragHdl=NULL;
    bDragHdl=FALSE;
    bDragSpecial=FALSE;
    mpCurrentSdrDragMethod=NULL;
    bDragStripes=FALSE;
    bMirrRefDragObj=TRUE;
    bDragWithCopy=FALSE;
    pInsPointUndo=NULL;
    bInsGluePoint=FALSE;
    bInsObjPointMode=FALSE;
    bInsGluePointMode=FALSE;
    nDragXorPolyLimit=100;
    nDragXorPointLimit=500;
    bNoDragXorPolys=FALSE;
    bAutoVertexCon=TRUE;
    bAutoCornerCon=FALSE;
    bRubberEdgeDragging=TRUE;
    nRubberEdgeDraggingLimit=100;
    bDetailedEdgeDragging=TRUE;
    nDetailedEdgeDraggingLimit=10;
    bResizeAtCenter=FALSE;
    bCrookAtCenter=FALSE;
    bMouseHideWhileDraggingPoints=FALSE;

    // init using default
    mbSolidDragging = getOptionsDrawinglayer().IsSolidDragCreate();
}

void SdrDragView::ImpMakeDragAttr()
{
    ImpDelDragAttr();
}

SdrDragView::SdrDragView(SdrModel* pModel1, OutputDevice* pOut)
:   SdrExchangeView(pModel1,pOut)
{
    ImpClearVars();
    ImpMakeDragAttr();
}

SdrDragView::~SdrDragView()
{
    ImpDelDragAttr();
}

void SdrDragView::ImpDelDragAttr()
{
}

BOOL SdrDragView::IsAction() const
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
        EndDragObj(FALSE);
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

BOOL SdrDragView::TakeDragObjAnchorPos(Point& rPos, BOOL bTR ) const
{
    Rectangle aR;
    TakeActionRect(aR);
    rPos = bTR ? aR.TopRight() : aR.TopLeft();
    if (GetMarkedObjectCount()==1 && IsDragObj() && // nur bei Einzelselektion
        !IsDraggingPoints() && !IsDraggingGluePoints() && // nicht beim Punkteschieben
        !mpCurrentSdrDragMethod->ISA(SdrDragMovHdl)) // nicht beim Handlesschieben
    {
        SdrObject* pObj=GetMarkedObjectByIndex(0);
        if (pObj->ISA(SdrCaptionObj))
        {
            Point aPt(((SdrCaptionObj*)pObj)->GetTailPos());
            BOOL bTail=eDragHdl==HDL_POLY; // Schwanz wird gedraggt (nicht so ganz feine Abfrage hier)
            BOOL bOwn=mpCurrentSdrDragMethod->ISA(SdrDragObjOwn); // Objektspeziefisch
            if (!bTail)
            { // bei bTail liefert TakeActionRect schon das richtige
                if (bOwn)
                { // bOwn kann sein MoveTextFrame, ResizeTextFrame aber eben nicht mehr DragTail
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
        return TRUE;
    }
    return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL SdrDragView::TakeDragLimit(SdrDragMode /*eMode*/, Rectangle& /*rRect*/) const
{
    return FALSE;
}

BOOL SdrDragView::BegDragObj(const Point& rPnt, OutputDevice* pOut, SdrHdl* pHdl, short nMinMov, SdrDragMethod* pForcedMeth)
{
    BrkAction();

    bool bRet=false;
    {
        SetDragWithCopy(FALSE);
        //ForceEdgesOfMarkedNodes();
        //TODO: aAni.Reset();
        mpCurrentSdrDragMethod=NULL;
        bDragSpecial=FALSE;
        bDragLimit=FALSE;
        SdrDragMode eTmpMode=eDragMode;
        if (eTmpMode==SDRDRAG_MOVE && pHdl!=NULL && pHdl->GetKind()!=HDL_MOVE) {
            eTmpMode=SDRDRAG_RESIZE;
        }
        bDragLimit=TakeDragLimit(eTmpMode,aDragLimit);
        bFramDrag=ImpIsFrameHandles();
        if (!bFramDrag &&
            (pMarkedObj==NULL || !pMarkedObj->hasSpecialDrag()) &&
            (pHdl==NULL || pHdl->GetObj()==NULL)) {
            bFramDrag=TRUE;
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
        aDragStat.SetPageView(pMarkedPV);  // <<-- hier muss die DragPV rein!!!
        aDragStat.SetMinMove(ImpGetMinMovLogic(nMinMov,pOut));
        aDragStat.SetHdl(pHdl);
        aDragStat.NextPoint();
        pDragWin=pOut;
        pDragHdl=pHdl;
        eDragHdl= pHdl==NULL ? HDL_MOVE : pHdl->GetKind();
        bDragHdl=eDragHdl==HDL_REF1 || eDragHdl==HDL_REF2 || eDragHdl==HDL_MIRX;

        // #103894# Expand test for HDL_ANCHOR_TR
        BOOL bNotDraggable = (HDL_ANCHOR == eDragHdl || HDL_ANCHOR_TR == eDragHdl);

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
                            // Sind 3D-Objekte selektiert?
                            BOOL b3DObjSelected = FALSE;
                            for(UINT32 a=0;!b3DObjSelected && a<GetMarkedObjectCount();a++)
                            {
                                SdrObject* pObj = GetMarkedObjectByIndex(a);
                                if(pObj && pObj->ISA(E3dObject))
                                    b3DObjSelected = TRUE;
                            }
                            // Falls ja, Shear auch bei !IsShearAllowed zulassen,
                            // da es sich bei 3D-Objekten um eingeschraenkte
                            // Rotationen handelt
                            if (!b3DObjSelected && !IsShearAllowed())
                                return FALSE;
                            mpCurrentSdrDragMethod = new SdrDragShear(*this,eDragMode==SDRDRAG_ROTATE);
                        } break;
                        case HDL_UPLFT: case HDL_UPRGT:
                        case HDL_LWLFT: case HDL_LWRGT:
                        {
                            if (eDragMode==SDRDRAG_SHEAR || eDragMode==SDRDRAG_DISTORT)
                            {
                                if (!IsDistortAllowed(TRUE) && !IsDistortAllowed(FALSE)) return FALSE;
                                mpCurrentSdrDragMethod = new SdrDragDistort(*this);
                            }
                            else
                            {
                                if (!IsRotateAllowed(TRUE)) return FALSE;
                                mpCurrentSdrDragMethod = new SdrDragRotate(*this);
                            }
                        } break;
                        default:
                        {
                            if (IsMarkedHitMovesAlways() && eDragHdl==HDL_MOVE)
                            { // HDL_MOVE ist auch wenn Obj direkt getroffen
                                if (!IsMoveAllowed()) return FALSE;
                                mpCurrentSdrDragMethod = new SdrDragMove(*this);
                            }
                            else
                            {
                                if (!IsRotateAllowed(TRUE)) return FALSE;
                                mpCurrentSdrDragMethod = new SdrDragRotate(*this);
                            }
                        }
                    }
                } break;
                case SDRDRAG_MIRROR:
                {
                    if (eDragHdl==HDL_MOVE && IsMarkedHitMovesAlways())
                    {
                        if (!IsMoveAllowed()) return FALSE;
                        mpCurrentSdrDragMethod = new SdrDragMove(*this);
                    }
                    else
                    {
                        if (!IsMirrorAllowed(TRUE,TRUE)) return FALSE;
                        mpCurrentSdrDragMethod = new SdrDragMirror(*this);
                    }
                } break;

                case SDRDRAG_CROP:
                {
                    if (eDragHdl==HDL_MOVE && IsMarkedHitMovesAlways())
                    {
                        if (!IsMoveAllowed())
                            return FALSE;
                        mpCurrentSdrDragMethod = new SdrDragMove(*this);
                    }
                    else
                    {
                        if (!IsCrookAllowed(TRUE) && !IsCrookAllowed(FALSE))
                            return FALSE;
                        mpCurrentSdrDragMethod = new SdrDragCrop(*this);
                    }
                }
                break;

                case SDRDRAG_TRANSPARENCE:
                {
                    if(eDragHdl == HDL_MOVE && IsMarkedHitMovesAlways())
                    {
                        if(!IsMoveAllowed())
                            return FALSE;
                        mpCurrentSdrDragMethod = new SdrDragMove(*this);
                    }
                    else
                    {
                        if(!IsTransparenceAllowed())
                            return FALSE;

                        mpCurrentSdrDragMethod = new SdrDragGradient(*this, FALSE);
                    }
                    break;
                }
                case SDRDRAG_GRADIENT:
                {
                    if(eDragHdl == HDL_MOVE && IsMarkedHitMovesAlways())
                    {
                        if(!IsMoveAllowed())
                            return FALSE;
                        mpCurrentSdrDragMethod = new SdrDragMove(*this);
                    }
                    else
                    {
                        if(!IsGradientAllowed())
                            return FALSE;

                        mpCurrentSdrDragMethod = new SdrDragGradient(*this);
                    }
                    break;
                }

                case SDRDRAG_CROOK :
                {
                    if (eDragHdl==HDL_MOVE && IsMarkedHitMovesAlways())
                    {
                        if (!IsMoveAllowed()) return FALSE;
                        mpCurrentSdrDragMethod = new SdrDragMove(*this);
                    }
                    else
                    {
                        if (!IsCrookAllowed(TRUE) && !IsCrookAllowed(FALSE)) return FALSE;
                        mpCurrentSdrDragMethod = new SdrDragCrook(*this);
                    }
                } break;

                default:
                {
                    // SDRDRAG_MOVE
                    if((eDragHdl == HDL_MOVE) && !IsMoveAllowed())
                    {
                        return FALSE;
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
                                if(!IsResizeAllowed(TRUE))
                                {
                                    return FALSE;
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
                                    return FALSE;
                                }
                            }

                            if(!mpCurrentSdrDragMethod)
                            {
                                // fallback to DragSpecial if no interaction defined
                                bDragSpecial = TRUE;
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
                    // Aha, Obj kann nicht Move SpecialDrag, also MoveFrameDrag versuchen
                    delete mpCurrentSdrDragMethod;
                    mpCurrentSdrDragMethod = 0;
                    bDragSpecial=FALSE;

                    if (!IsMoveAllowed())
                        return FALSE;

                    bFramDrag=TRUE;
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

BOOL SdrDragView::EndDragObj(BOOL bCopy)
{
    bool bRet(false);

    // #i73341# If insert GluePoint, do not insist on last points being different
    if(mpCurrentSdrDragMethod && aDragStat.IsMinMoved() && (IsInsertGluePoint() || aDragStat.GetNow() != aDragStat.GetPrev()))
    {
        ULONG nHdlAnzMerk=0;

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
            bInsPolyPoint=FALSE;
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
            // Aha, Obj hat nicht gebroadcastet (z.B. Writer FlyFrames)
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

    bInsPolyPoint=FALSE;
    SetInsertGluePoint(FALSE);

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
            pInsPointUndo->Undo(); // Den eingefuegten Punkt wieder raus
            delete pInsPointUndo;
            pInsPointUndo=NULL;
            SetMarkHandles();
            bInsPolyPoint=FALSE;
        }

        if (IsInsertGluePoint())
        {
            pInsPointUndo->Undo(); // Den eingefuegten Klebepunkt wieder raus
            delete pInsPointUndo;
            pInsPointUndo=NULL;
            SetInsertGluePoint(FALSE);
        }

        eDragHdl=HDL_MOVE;
        pDragHdl=NULL;
    }
}

BOOL SdrDragView::IsInsObjPointPossible() const
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

        sal_Bool bClosed0(pMarkedPath->IsClosedObj());

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
            // Obj was closed implicit
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

BOOL SdrDragView::EndInsObjPoint(SdrCreateCmd eCmd)
{
    if(IsInsObjPoint())
    {
        sal_uInt32 nNextPnt(mnInsPointNum);
        Point aPnt(aDragStat.GetNow());
        BOOL bOk=EndDragObj(FALSE);
        if (bOk==TRUE && eCmd!=SDRCREATE_FORCEEND)
        {
            // Ret=True bedeutet: Action ist vorbei.
            bOk=!(ImpBegInsObjPoint(sal_True, nNextPnt, aPnt, eCmd == SDRCREATE_NEXTOBJECT, pDragWin));
        }

        return bOk;
    } else return FALSE;
}

BOOL SdrDragView::IsInsGluePointPossible() const
{
    BOOL bRet=FALSE;
    if (IsInsGluePointMode() && AreObjectsMarked())
    {
        if (GetMarkedObjectCount()==1)
        {
            // FALSE liefern, wenn 1 Objekt und dieses ein Verbinder ist.
            const SdrObject* pObj=GetMarkedObjectByIndex(0);
            if (!HAS_BASE(SdrEdgeObj,pObj))
            {
               bRet=TRUE;
            }
        }
        else
        {
            bRet=TRUE;
        }
    }
    return bRet;
}

BOOL SdrDragView::BegInsGluePoint(const Point& rPnt)
{
    BOOL bRet=FALSE;
    SdrObject* pObj;
    SdrPageView* pPV;
    ULONG nMarkNum;
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
            USHORT nGlueIdx=pGPL->Insert(SdrGluePoint());
            SdrGluePoint& rGP=(*pGPL)[nGlueIdx];
            USHORT nGlueId=rGP.GetId();
            rGP.SetAbsolutePos(rPnt,*pObj);

            SdrHdl* pHdl=NULL;
            if (MarkGluePoint(pObj,nGlueId,pPV))
            {
                pHdl=GetGluePointHdl(pObj,nGlueId);
            }
            if (pHdl!=NULL && pHdl->GetKind()==HDL_GLUE && pHdl->GetObj()==pObj && pHdl->GetObjHdlNum()==nGlueId)
            {
                SetInsertGluePoint(TRUE);
                bRet=BegDragObj(rPnt,NULL,pHdl,0);
                if (bRet)
                {
                    aDragStat.SetMinMoved();
                    MovDragObj(rPnt);
                }
                else
                {
                    SetInsertGluePoint(FALSE);
                    delete pInsPointUndo;
                    pInsPointUndo=NULL;
                }
            }
            else
            {
                OSL_FAIL("BegInsGluePoint(): GluePoint-Handle nicht gefunden");
            }
        }
        else
        {
            // Keine Klebepunkte moeglich bei diesem Objekt (z.B. Edge)
            SetInsertGluePoint(FALSE);
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
            sdr::overlay::OverlayManager* pOverlayManager = pCandidate->GetOverlayManager();

            if(pOverlayManager)
            {
                mpCurrentSdrDragMethod->CreateOverlayGeometry(*pOverlayManager);

                // #i101679# Force changed overlay to be shown
                pOverlayManager->flush();
            }
        }

        aDragStat.SetShown(TRUE);
    }
}

void SdrDragView::HideDragObj()
{
    if(mpCurrentSdrDragMethod && aDragStat.IsShown())
    {
        mpCurrentSdrDragMethod->destroyOverlayGeometry();
        aDragStat.SetShown(FALSE);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrDragView::SetNoDragXorPolys(BOOL bOn)
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

void SdrDragView::SetDragStripes(BOOL bOn)
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

BOOL SdrDragView::IsOrthoDesired() const
{
    if(mpCurrentSdrDragMethod && (IS_TYPE(SdrDragObjOwn, mpCurrentSdrDragMethod) || IS_TYPE(SdrDragResize, mpCurrentSdrDragMethod)))
    {
        return bOrthoDesiredOnMarked;
    }

    return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrDragView::SetRubberEdgeDragging(BOOL bOn)
{
    if (bOn!=IsRubberEdgeDragging())
    {
        ULONG nAnz = GetEdgesOfMarkedNodes().GetMarkCount();
        BOOL bShowHide=nAnz!=0 && IsDragObj() &&
                 (nRubberEdgeDraggingLimit>=nAnz);
        if (bShowHide)
            HideDragObj();
        bRubberEdgeDragging=bOn;
        if (bShowHide)
            ShowDragObj();
    }
}

void SdrDragView::SetRubberEdgeDraggingLimit(USHORT nEdgeObjAnz)
{
    if (nEdgeObjAnz!=nRubberEdgeDraggingLimit)
    {
        ULONG nAnz = GetEdgesOfMarkedNodes().GetMarkCount();
        BOOL bShowHide=IsRubberEdgeDragging() && nAnz!=0 && IsDragObj() &&
                 (nEdgeObjAnz>=nAnz)!=(nRubberEdgeDraggingLimit>=nAnz);
        if (bShowHide)
            HideDragObj();
        nRubberEdgeDraggingLimit=nEdgeObjAnz;
        if (bShowHide)
            ShowDragObj();
    }
}

void SdrDragView::SetDetailedEdgeDragging(BOOL bOn)
{
    if (bOn!=IsDetailedEdgeDragging())
    {
        ULONG nAnz = GetEdgesOfMarkedNodes().GetMarkCount();
        BOOL bShowHide=nAnz!=0 && IsDragObj() &&
                 (nDetailedEdgeDraggingLimit>=nAnz);
        if (bShowHide)
            HideDragObj();
        bDetailedEdgeDragging=bOn;
        if (bShowHide)
            ShowDragObj();
    }
}

void SdrDragView::SetDetailedEdgeDraggingLimit(USHORT nEdgeObjAnz)
{
    if (nEdgeObjAnz!=nDetailedEdgeDraggingLimit)
    {
        ULONG nAnz = GetEdgesOfMarkedNodes().GetMarkCount();
        BOOL bShowHide=IsDetailedEdgeDragging() && nAnz!=0 && IsDragObj() &&
                 (nEdgeObjAnz>=nAnz)!=(nDetailedEdgeDraggingLimit>=nAnz);
        if (bShowHide)
            HideDragObj();
        nDetailedEdgeDraggingLimit=nEdgeObjAnz;
        if (bShowHide)
            ShowDragObj();
    }
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

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
