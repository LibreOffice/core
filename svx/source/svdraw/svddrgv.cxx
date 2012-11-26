/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

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
#include <svx/svdlegacy.hxx>

using namespace sdr;

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrDragView::SdrDragView(SdrModel& rModel1, OutputDevice* pOut)
:   SdrExchangeView(rModel1, pOut),
    mpDragHdl(0),
    mpCurrentSdrDragMethod(0),
    mpInsPointUndo(0),
    maDragLimit(),
    maInsPointUndoStr(),
    meDragHdl(HDL_MOVE),
    mbFrameDrag(false),
    mbDragSpecial(false),
    mbMarkedHitMovesAlways(false),
    mbDragLimit(false),
    mbDragHdl(false),
    mbDragStripes(false),
    mbSolidDragging(getOptionsDrawinglayer().IsSolidDragCreate()),
    mbResizeAtCenter(false),
    mbCrookAtCenter(false),
    mbDragWithCopy(false),
    mbInsGluePoint(false),
    mbInsObjPointMode(false),
    mbInsGluePointMode(false),
    mbNoDragXorPolys(false)
{
}

SdrDragView::~SdrDragView()
{
}

bool SdrDragView::IsAction() const
{
    return (GetDragMethod() || SdrExchangeView::IsAction());
}

void SdrDragView::MovAction(const basegfx::B2DPoint& rPnt)
{
    SdrExchangeView::MovAction(rPnt);

    if(GetDragMethod())
    {
        MovDragObj(rPnt);
    }
}

void SdrDragView::EndAction()
{
    if (GetDragMethod())
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

basegfx::B2DRange SdrDragView::TakeActionRange() const
{
    if(GetDragMethod())
    {
        basegfx::B2DRange aRetval(GetDragStat().GetActionRange());

        if(aRetval.isEmpty())
        {
            // #i95646# is this used..?
            aRetval = GetDragMethod()->getCurrentRange();
        }

        if(aRetval.isEmpty())
        {
            aRetval = basegfx::B2DRange(GetDragStat().GetNow(), GetDragStat().GetNow());
        }

        return aRetval;
    }
    else
    {
        return SdrExchangeView::TakeActionRange();
    }
}

bool SdrDragView::TakeDragObjAnchorPos(basegfx::B2DPoint& rPos, bool bTR ) const
{
    const basegfx::B2DRange aActionRange(TakeActionRange());
    const basegfx::B2DPoint aTopLeft(bTR ? aActionRange.getMaxX() : aActionRange.getMinX(), aActionRange.getMinY());
    SdrObject* pSelected = getSelectedIfSingle();

    rPos = aTopLeft;

    if(pSelected
        && IsDragObj()
        && !IsDraggingPoints()
        && !IsDraggingGluePoints())
    {
        if(!dynamic_cast< SdrDragMovHdl* >(GetDragMethod()))
        {
            const SdrCaptionObj* pCaptionObj = dynamic_cast< const SdrCaptionObj* >(pSelected);

            if(pCaptionObj)
            {
                basegfx::B2DPoint aPt(pCaptionObj->GetTailPos());

                if(!HDL_POLY == GetDragHdlKind())
                {
                    if(dynamic_cast< SdrDragObjOwn* >(GetDragMethod()))
                    {
                        rPos=aPt;
                    }
                    else
                    {
                        // drag the whole Object (Move, Resize, ...)
                        rPos = GetDragMethod()->getCurrentTransformation() * aPt;
                    }
                }
            }

            return true;
        }
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool SdrDragView::TakeDragLimit(SdrDragMode /*eMode*/, basegfx::B2DRange& /*rRange*/) const
{
    return false;
}

bool SdrDragView::BegDragObj(const basegfx::B2DPoint& rPnt, const SdrHdl* pHdl, double fMinMovLogic, SdrDragMethod* pForcedMeth)
{
    bool bRetval(false);

    BrkAction();
    SetDragWithCopy(false);
    mpCurrentSdrDragMethod = 0;
    mbDragSpecial = false;
    mbDragLimit = false;
    SdrDragMode eTmpMode(GetDragMode());

    if(SDRDRAG_MOVE == eTmpMode && pHdl && HDL_MOVE != pHdl->GetKind())
    {
            eTmpMode=SDRDRAG_RESIZE;
    }

    mbDragLimit = TakeDragLimit(eTmpMode, maDragLimit);
    mbFrameDrag =ImpIsFrameHandles();

    if(!mbFrameDrag)
    {
        const SdrObject* pMarkedObject = getSelectedIfSingle();

        if((!pMarkedObject || !pMarkedObject->hasSpecialDrag())
            && (!pHdl || !pHdl->GetObj()))
        {
            mbFrameDrag = true;
        }
    }

    const basegfx::B2DPoint aPnt(rPnt);

    if(!pHdl
        || HDL_MOVE == pHdl->GetKind()
        || HDL_MIRX == pHdl->GetKind()
        || HDL_TRNS == pHdl->GetKind()
        || HDL_GRAD == pHdl->GetKind())
    {
        GetDragStat().Reset(aPnt);
    }
    else
    {
        GetDragStat().Reset(pHdl->getPosition());
    }

    GetDragStat().SetMinMove(fMinMovLogic);
    GetDragStat().SetActiveHdl(const_cast< SdrHdl* >(pHdl));
    GetDragStat().NextPoint();

    mpDragHdl = const_cast< SdrHdl* >(pHdl);
    meDragHdl = pHdl ? pHdl->GetKind() : HDL_MOVE;
    mbDragHdl = (HDL_REF1 == GetDragHdlKind() || HDL_REF2 == GetDragHdlKind() || HDL_MIRX == GetDragHdlKind());

    // #103894# Expand test for HDL_ANCHOR_TR
    const bool bNotDraggable = (HDL_ANCHOR == GetDragHdlKind() || HDL_ANCHOR_TR == GetDragHdlKind());
    SdrView* pSdrView = static_cast< SdrView* >(this);

    if(pHdl && (HDL_SMARTTAG == pHdl->GetKind()) && pForcedMeth)
    {
        // just use the forced method for smart tags
    }
    else if(mbDragHdl)
    {
        mpCurrentSdrDragMethod = new SdrDragMovHdl(*pSdrView);
    }
    else if(!bNotDraggable)
    {
        switch(GetDragMode())
        {
            case SDRDRAG_ROTATE:
            case SDRDRAG_SHEAR:
            case SDRDRAG_DISTORT:
            {
                switch(GetDragHdlKind())
                {
                    case HDL_LEFT:
                    case HDL_RIGHT:
                    case HDL_UPPER:
                    case HDL_LOWER:
                    {
                        // Sind 3D-Objekte selektiert?
                        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());
                        bool b3DObjSelected(false);

                        for(sal_uInt32 a(0); !b3DObjSelected && a < aSelection.size(); a++)
                        {
                            if(dynamic_cast< E3dObject* >(aSelection[a]))
                            {
                                b3DObjSelected = true;
                            }
                        }

                        // Falls ja, Shear auch bei !IsShearAllowed zulassen,
                        // da es sich bei 3D-Objekten um eingeschraenkte
                        // Rotationen handelt
                        if (!b3DObjSelected && !IsShearAllowed())
                        {
                            return false;
                        }

                        mpCurrentSdrDragMethod = new SdrDragShear(*pSdrView);
                        break;
                    }

                    case HDL_UPLFT:
                    case HDL_UPRGT:
                    case HDL_LWLFT:
                    case HDL_LWRGT:
                    {
                        if(SDRDRAG_SHEAR == GetDragMode() || SDRDRAG_DISTORT == GetDragMode())
                        {
                            if(!IsDistortAllowed(true) && !IsDistortAllowed(false))
                            {
                                return false;
                            }

                            mpCurrentSdrDragMethod = new SdrDragDistort(*pSdrView);
                        }
                        else
                        {
                            if(!IsRotateAllowed(true))
                            {
                                return false;
                            }

                            mpCurrentSdrDragMethod = new SdrDragRotate(*pSdrView);
                        }
                        break;
                    }

                    default:
                    {
                        if(IsMarkedHitMovesAlways() && HDL_MOVE == GetDragHdlKind())
                        {
                            // HDL_MOVE ist auch wenn Obj direkt getroffen
                            if(!IsMoveAllowed())
                            {
                                return false;
                            }

                            mpCurrentSdrDragMethod = new SdrDragMove(*pSdrView);
                        }
                        else
                        {
                            if(!IsRotateAllowed(true))
                            {
                                return false;
                            }

                            mpCurrentSdrDragMethod = new SdrDragRotate(*pSdrView);
                        }
                    }
                }
                break;
            }

            case SDRDRAG_MIRROR:
            {
                if(HDL_MOVE == GetDragHdlKind() && IsMarkedHitMovesAlways())
                {
                    if(!IsMoveAllowed())
                    {
                        return false;
                    }

                    mpCurrentSdrDragMethod = new SdrDragMove(*pSdrView);
                }
                else
                {
                    if(!IsMirrorAllowed(true, true))
                    {
                        return false;
                    }

                    mpCurrentSdrDragMethod = new SdrDragMirror(*pSdrView);
                }
                break;
            }

            case SDRDRAG_CROP:
            {
                if(HDL_MOVE == GetDragHdlKind() && IsMarkedHitMovesAlways())
                {
                    if (!IsMoveAllowed())
                    {
                        return false;
                    }

                    mpCurrentSdrDragMethod = new SdrDragMove(*pSdrView);
                }
                else
                {
                    if(!IsCrookAllowed(true) && !IsCrookAllowed(false))
                    {
                        return false;
                    }

                    mpCurrentSdrDragMethod = new SdrDragCrop(*pSdrView);
                }
                break;
            }

            case SDRDRAG_TRANSPARENCE:
            {
                if(HDL_MOVE == GetDragHdlKind()  && IsMarkedHitMovesAlways())
                {
                    if(!IsMoveAllowed())
                    {
                        return false;
                    }

                    mpCurrentSdrDragMethod = new SdrDragMove(*pSdrView);
                }
                else
                {
                    if(!IsTransparenceAllowed())
                {
                    return false;
                }

                mpCurrentSdrDragMethod = new SdrDragGradient(*pSdrView, false);
                }
                break;
            }

            case SDRDRAG_GRADIENT:
            {
                if(HDL_MOVE == GetDragHdlKind() && IsMarkedHitMovesAlways())
                {
                    if(!IsMoveAllowed())
                    {
                        return false;
                    }

                    mpCurrentSdrDragMethod = new SdrDragMove(*pSdrView);
                }
                else
                {
                    if(!IsGradientAllowed())
                    {
                        return false;
                    }

                    mpCurrentSdrDragMethod = new SdrDragGradient(*pSdrView);
                }
                break;
            }

            case SDRDRAG_CROOK :
            {
                if(HDL_MOVE == GetDragHdlKind() && IsMarkedHitMovesAlways())
                {
                    if(!IsMoveAllowed())
                    {
                        return false;
                    }

                    mpCurrentSdrDragMethod = new SdrDragMove(*pSdrView);
                }
                else
                {
                    if(!IsCrookAllowed(true) && !IsCrookAllowed(false))
                    {
                        return false;
                    }

                    mpCurrentSdrDragMethod = new SdrDragCrook(*pSdrView);
                }
                break;
            }

            default:
            {
                // SDRDRAG_MOVE
                if((HDL_MOVE == GetDragHdlKind()) && !IsMoveAllowed())
                {
                    return false;
                }
                else if(HDL_GLUE == GetDragHdlKind())
                {
                    mpCurrentSdrDragMethod = new SdrDragMove(*pSdrView);
                }
                else
                {
                    if(mbFrameDrag)
                    {
                        if(HDL_MOVE == GetDragHdlKind())
                        {
                            mpCurrentSdrDragMethod = new SdrDragMove(*pSdrView);
                        }
                        else
                        {
                            if(!IsResizeAllowed(true))
                            {
                                return false;
                            }

                            bool bSingleTextObjMark(false); // SJ: #i100490#
                            const SdrObject* pMarkedObject = getSelectedIfSingle();

                            if(pMarkedObject)
                            {
                                const SdrTextObj* pMarkedText = dynamic_cast< const SdrTextObj* >(pMarkedObject);

                                if(pMarkedText && pMarkedText->IsTextFrame())
                                {
                                    bSingleTextObjMark = true;
                                }
                            }

                            if ( bSingleTextObjMark )
                            {
                                mpCurrentSdrDragMethod = new SdrDragObjOwn(*pSdrView);
                            }
                            else
                            {
                                mpCurrentSdrDragMethod = new SdrDragResize(*pSdrView);
                            }
                        }
                    }
                    else
                    {
                        if(HDL_MOVE == GetDragHdlKind())
                        {
                            const SdrObject* pMarkedObject = getSelectedIfSingle();
                            const bool bCustomShapeSelected(dynamic_cast< const SdrObjCustomShape* >(pMarkedObject));

                            if(bCustomShapeSelected)
                            {
                                mpCurrentSdrDragMethod = new SdrDragMove( *pSdrView );
                            }
                        }
                        else if(HDL_POLY == GetDragHdlKind())
                        {
                            const SdrObject* pMarkedObject = getSelectedIfSingle();
                            const bool bConnectorSelected(pMarkedObject && pMarkedObject->IsSdrEdgeObj());

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

                        if(!GetDragMethod())
                        {
                            // fallback to DragSpecial if no interaction defined
                            mbDragSpecial = true;
                            mpCurrentSdrDragMethod = new SdrDragObjOwn(*pSdrView);
                        }
                    }
                }
            }
        }
    }

    if(pForcedMeth)
    {
        delete mpCurrentSdrDragMethod;

        mpCurrentSdrDragMethod = pForcedMeth;
    }

    GetDragStat().SetDragMethod(GetDragMethod());

    if(GetDragMethod())
    {
        bRetval = GetDragMethod()->BeginSdrDrag();

        if(!bRetval)
        {
            if(!pHdl && dynamic_cast< SdrDragObjOwn* >(GetDragMethod()))
            {
                // Aha, Obj kann nicht Move SpecialDrag, also MoveFrameDrag versuchen
                delete mpCurrentSdrDragMethod;
                mpCurrentSdrDragMethod = 0;
                mbDragSpecial = false;

                if (!IsMoveAllowed())
                {
                    return false;
                }

                mbFrameDrag = true;
                mpCurrentSdrDragMethod = new SdrDragMove(*pSdrView);
                GetDragStat().SetDragMethod(GetDragMethod());
                bRetval = GetDragMethod()->BeginSdrDrag();
            }
        }

        if(!bRetval)
        {
            delete mpCurrentSdrDragMethod;
            mpCurrentSdrDragMethod = 0;
            GetDragStat().SetDragMethod(0);
        }
    }

    return bRetval;
}

void SdrDragView::MovDragObj(const basegfx::B2DPoint& rPnt)
{
    if (GetDragMethod())
    {
        const basegfx::B2DPoint aPnt(ImpLimitToWorkArea(rPnt));

        GetDragMethod()->MoveSdrDrag(aPnt);
    }
}

bool SdrDragView::EndDragObj(bool bCopy)
{
    bool bRetval(false);

    // #i73341# If insert GluePoint, do not insist on last points being different
    if(GetDragMethod() && GetDragStat().IsMinMoved() && (IsInsertGluePoint() || GetDragStat().GetNow() != GetDragStat().GetPrev()))
    {
        sal_uInt32 nHdlAnzMerk(0);

        if(IsEliminatePolyPoints())
        {
            // IBM Special
            nHdlAnzMerk=GetMarkablePointCount();
        }

        const bool bUndo = IsUndoEnabled();

        if (IsInsertGluePoint() && bUndo)
        {
            BegUndo(maInsPointUndoStr);
            AddUndo(mpInsPointUndo);
        }

        bRetval = GetDragMethod()->EndSdrDrag(bCopy);

        if( IsInsertGluePoint() && bUndo)
        {
            EndUndo();
        }

        delete mpCurrentSdrDragMethod;
        mpCurrentSdrDragMethod = 0;

        if(IsEliminatePolyPoints())
        {
            // IBM Special
            if (nHdlAnzMerk!=GetMarkablePointCount())
            {
                MarkPoints(0, true); // unmarkall
            }
        }

        if(mbInsPolyPoint)
        {
            SetMarkHandles();
            mbInsPolyPoint = false;

            if( bUndo )
            {
                BegUndo(maInsPointUndoStr);
                AddUndo(mpInsPointUndo);
                EndUndo();
            }
        }

        meDragHdl = HDL_MOVE;
        mpDragHdl = 0;
    }
    else
    {
        BrkDragObj();
    }

    mbInsPolyPoint = false;
    SetInsertGluePoint(false);

    return bRetval;
}

void SdrDragView::BrkDragObj()
{
    if(GetDragMethod())
    {
        GetDragMethod()->CancelSdrDrag();
        delete mpCurrentSdrDragMethod;
        mpCurrentSdrDragMethod = 0;

        if(mbInsPolyPoint)
        {
            mpInsPointUndo->Undo(); // Den eingefuegten Punkt wieder raus
            delete mpInsPointUndo;
            mpInsPointUndo = 0;
            SetMarkHandles();
            mbInsPolyPoint = false;
        }

        if (IsInsertGluePoint())
        {
            mpInsPointUndo->Undo(); // Den eingefuegten Klebepunkt wieder raus
            delete mpInsPointUndo;
            mpInsPointUndo = 0;
            SetInsertGluePoint(false);
        }

        meDragHdl = HDL_MOVE;
        mpDragHdl = 0;
    }
}

bool SdrDragView::IsInsObjPointPossible() const
{
    const SdrObject* pMarkedObject = getSelectedIfSingle();

    return pMarkedObject && pMarkedObject->IsPolygonObject();
}

bool SdrDragView::ImpBegInsObjPoint(bool bIdxZwang, sal_uInt32 /*nIdx*/, const basegfx::B2DPoint& rPnt, bool bNewObj)
{
    bool bRetval(false);
    SdrPathObj* pMarkedPath = dynamic_cast< SdrPathObj* >(getSelectedIfSingle());

    if(pMarkedPath)
    {
        BrkAction();
        mpInsPointUndo = dynamic_cast< SdrUndoGeoObj* >( getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoGeoObject(*pMarkedPath) );
        DBG_ASSERT( mpInsPointUndo, "svx::SdrDragView::BegInsObjPoint(), could not create correct undo object!" );

        XubString aStr(ImpGetResStr(STR_DragInsertPoint));
        XubString aName;
        pMarkedPath->TakeObjNameSingul(aName);
        xub_StrLen nPos(aStr.SearchAscii("%1"));

        if(STRING_NOTFOUND != nPos)
        {
            aStr.Erase(nPos, 2);
            aStr.Insert(aName, nPos);
        }

        maInsPointUndoStr = aStr;
        basegfx::B2DPoint aPt(rPnt);

        if(bNewObj)
        {
            aPt = GetSnapPos(aPt);
        }

        bool bClosed0(pMarkedPath->IsClosedObj());

        if(bIdxZwang)
        {
            const Point aOldPoint(basegfx::fround(aPt.getX()), basegfx::fround(aPt.getY()));
            mnInsPointNum = pMarkedPath->InsPoint(aOldPoint, bNewObj);
        }
        else
        {
            const Point aOldPoint(basegfx::fround(aPt.getX()), basegfx::fround(aPt.getY()));
            mnInsPointNum = pMarkedPath->InsPointOld(aOldPoint, bNewObj);
        }

        if(bClosed0 != pMarkedPath->IsClosedObj())
        {
            // Obj was closed implicit, object changed
            const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*pMarkedPath);
            pMarkedPath->SetChanged();
        }

        if(0xffffffff != mnInsPointNum)
        {
            mbInsPolyPoint = true;
            MarkPoints(0, true); // unmarkall
            SetMarkHandles();

            bRetval = BegDragObj(rPnt, maViewHandleList.GetHdlByIndex(mnInsPointNum), 0.0);

            if (bRetval)
            {
                GetDragStat().SetMinMoved();
                MovDragObj(rPnt);
            }
        }
        else
        {
            delete mpInsPointUndo;
            mpInsPointUndo = 0;
        }
    }

    return bRetval;
}

bool SdrDragView::EndInsObjPoint(SdrCreateCmd eCmd)
{
    if(IsInsObjPoint())
    {
        sal_uInt32 nNextPnt(mnInsPointNum);
        basegfx::B2DPoint aPnt(GetDragStat().GetNow());
        bool bOk(EndDragObj(false));

        if(bOk && SDRCREATE_FORCEEND != eCmd)
        {
            // Ret=True bedeutet: Action ist vorbei.
            bOk = !ImpBegInsObjPoint(true, nNextPnt, aPnt, eCmd == SDRCREATE_NEXTOBJECT);
        }

        return bOk;
    }
    else
    {
        return false;
    }
}

bool SdrDragView::IsInsGluePointPossible() const
{
    bool bRetval(false);

    if(IsInsGluePointMode() && areSdrObjectsSelected())
    {
        const SdrObject* pMarkedObject = getSelectedIfSingle();

        if(pMarkedObject)
        {
            if(!pMarkedObject->IsSdrEdgeObj())
            {
               bRetval = true;
            }
        }
        else
        {
            bRetval = true;
        }
    }

    return bRetval;
}

bool SdrDragView::BegInsGluePoint(const basegfx::B2DPoint& rPnt)
{
    bool bRetval(false);
    SdrObject* pObj = 0;
    sal_uInt32 nMarkNum;

    if(PickMarkedObj(rPnt, pObj, &nMarkNum, SDRSEARCH_PASS2BOUND))
    {
        BrkAction();
        MarkGluePoints(0, true);
        mpInsPointUndo = dynamic_cast< SdrUndoGeoObj* >( getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoGeoObject(*pObj) );
        DBG_ASSERT( mpInsPointUndo, "svx::SdrDragView::BegInsObjPoint(), could not create correct undo object!" );
        XubString aStr(ImpGetResStr(STR_DragInsertGluePoint));
        XubString aName;

        pObj->TakeObjNameSingul(aName);
        aStr.SearchAndReplaceAscii("%1", aName);
        maInsPointUndoStr = aStr;
        SdrGluePointList* pGPL=pObj->ForceGluePointList();

        if(pGPL)
        {
            const sal_uInt32 nGlueIdx(pGPL->Insert(SdrGluePoint()));
            SdrGluePoint& rGP=(*pGPL)[nGlueIdx];
            const sal_uInt32 nGlueId(rGP.GetId());
            SdrHdl* pHdl = 0;

            rGP.SetAbsolutePos(rPnt, sdr::legacy::GetSnapRange(*pObj));

            if(MarkGluePoint(pObj, nGlueId))
            {
                pHdl=GetGluePointHdl(pObj,nGlueId);
            }

            if(pHdl && HDL_GLUE == pHdl->GetKind() && pHdl->GetObj() == pObj && pHdl->GetObjHdlNum() == nGlueId)
            {
                SetInsertGluePoint(true);
                bRetval = BegDragObj(rPnt, pHdl, 0.0);

                if(bRetval)
                {
                    GetDragStat().SetMinMoved();
                    MovDragObj(rPnt);
                }
                else
                {
                    SetInsertGluePoint(false);
                    delete mpInsPointUndo;
                    mpInsPointUndo = 0;
                }
            }
            else
            {
                DBG_ERROR("BegInsGluePoint(): GluePoint-Handle nicht gefunden");
            }
        }
        else
        {
            // Keine Klebepunkte moeglich bei diesem Objekt (z.B. Edge)
            SetInsertGluePoint(false);
            delete mpInsPointUndo;
            mpInsPointUndo = 0;
        }
    }

    return bRetval;
}

void SdrDragView::ShowDragObj()
{
    if(GetDragMethod() && !GetDragStat().IsShown())
    {
        for(sal_uInt32 a(0); a < PaintWindowCount(); a++)
        {
            SdrPaintWindow* pCandidate = GetPaintWindow(a);
            sdr::overlay::OverlayManager* pOverlayManager = pCandidate->GetOverlayManager();

            if(pOverlayManager)
            {
                GetDragMethod()->CreateOverlayGeometry(*pOverlayManager);

                // #i101679# Force changed overlay to be shown
                pOverlayManager->flush();
            }
        }

        GetDragStat().SetShown(true);
    }
}

void SdrDragView::HideDragObj()
{
    if(GetDragMethod() && GetDragStat().IsShown())
    {
        GetDragMethod()->destroyOverlayGeometry();
        GetDragStat().SetShown(false);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrDragView::SetNoDragXorPolys(bool bOn)
{
    if (IsNoDragXorPolys()!=bOn)
    {
            HideDragObj();
        mbNoDragXorPolys = bOn;

        if(GetDragMethod())
        {
            // force recreation of drag content
            GetDragMethod()->resetSdrDragEntries();
        }

        ShowDragObj();
    }
}

void SdrDragView::SetDragStripes(bool bOn)
{
    if(mbDragStripes != bOn)
    {
        if(GetDragMethod() && GetDragStat().IsShown())
        {
            HideDragObj();
            mbDragStripes = bOn;
            ShowDragObj();
        }
        else
        {
            mbDragStripes = bOn;
        }
    }
}

bool SdrDragView::IsOrthoDesired() const
{
    if(GetDragMethod() && (dynamic_cast< SdrDragObjOwn* >(GetDragMethod()) || dynamic_cast< SdrDragResize* >(GetDragMethod())))
    {
        return mbOrthoDesiredOnMarked;
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrDragView::SetMarkHandles()
{
    if(GetDragHdl())
    {
        mpDragHdl = 0;
    }

    SdrExchangeView::SetMarkHandles();
}

bool SdrDragView::IsSolidDragging() const
{
    // allow each user to disable by having a local setting, but using AND for
    // checking allowance
    return mbSolidDragging && getOptionsDrawinglayer().IsSolidDragCreate();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof
