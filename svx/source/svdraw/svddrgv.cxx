/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svddrgv.cxx,v $
 *
 *  $Revision: 1.25 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 09:49:28 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <svx/svddrgv.hxx>
#include "xattr.hxx"
#include <svx/xpoly.hxx>
#include "svdxout.hxx"
#include <svx/svdetc.hxx>
#include <svx/svdtrans.hxx>
#include <svx/svdundo.hxx>
#include <svx/svdocapt.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdopath.hxx> // wg. Sonderbehandlung in SetDragPolys()
#include <svx/svdoedge.hxx> // wg. Sonderbehandlung in IsInsGluePossible()
#include "svdstr.hrc"   // Namen aus der Resource
#include "svdglob.hxx"  // StringCache
#include "svddrgm1.hxx"
#include <svx/obj3d.hxx>
#ifndef _SVDOASHP_HXX
#include <svx/svdoashp.hxx>
#endif

#ifndef _SDRPAINTWINDOW_HXX
#include <sdrpaintwindow.hxx>
#endif

#ifndef _BGFX_POLYPOLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolygontools.hxx>
#endif
#include <svx/polypolygoneditor.hxx>

using namespace sdr;

////////////////////////////////////////////////////////////////////////////////////////////////////

class ImpSdrDragViewExtraData
{
    // The OverlayObjects for XOR replacement
    ::sdr::overlay::OverlayObjectList               maObjects;

public:
    ImpSdrDragViewExtraData();
    ~ImpSdrDragViewExtraData();

    void CreateAndShowOverlay(const SdrDragView& rView);
    void HideOverlay();
};

ImpSdrDragViewExtraData::ImpSdrDragViewExtraData()
{
}

ImpSdrDragViewExtraData::~ImpSdrDragViewExtraData()
{
    HideOverlay();
}

void ImpSdrDragViewExtraData::CreateAndShowOverlay(const SdrDragView& rView)
{
    // This method replaces DrawDragObj and creates the necessary overlay objects instead.
    // This is only half of the migration, but necessary to get rid of the XOR painting.

    // get DragMethod. All calls to this method test pDragBla.
    SdrDragMethod& rDragMethod = *rView.GetDragMethod();

    // for each PaintWindow and each OverlayManager, create the drag geometry
    for(sal_uInt32 a(0L); a < rView.PaintWindowCount(); a++)
    {
        SdrPaintWindow* pCandidate = rView.GetPaintWindow(a);
        ::sdr::overlay::OverlayManager* pOverlayManager = pCandidate->GetOverlayManager();

        if(pOverlayManager)
        {
            rDragMethod.CreateOverlayGeometry(*pOverlayManager, maObjects);
        }
    }
}

void ImpSdrDragViewExtraData::HideOverlay()
{
    // the clear() call at the list removes all objects from the
    // OverlayManager and deletes them.
    maObjects.clear();
}

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
    pDragBla=NULL;
    bDragStripes=FALSE;
    //HMHbNoDragHdl=TRUE;
    bMirrRefDragObj=TRUE;
    bSolidDragging=FALSE;
    bDragWithCopy=FALSE;
    pInsPointUndo=NULL;
//  bInsAfter=FALSE;
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
}

void SdrDragView::ImpMakeDragAttr()
{
    ImpDelDragAttr();
}

SdrDragView::SdrDragView(SdrModel* pModel1, OutputDevice* pOut)
:   SdrExchangeView(pModel1,pOut),
    mpDragViewExtraData(new ImpSdrDragViewExtraData())
{
    ImpClearVars();
    ImpMakeDragAttr();
}

SdrDragView::~SdrDragView()
{
    delete mpDragViewExtraData;
    ImpDelDragAttr();
}

void SdrDragView::ImpDelDragAttr()
{
}

BOOL SdrDragView::IsAction() const
{
    return (pDragBla || SdrExchangeView::IsAction());
}

void SdrDragView::MovAction(const Point& rPnt)
{
    SdrExchangeView::MovAction(rPnt);
    if (pDragBla!=NULL) {
        MovDragObj(rPnt);
    }
}

void SdrDragView::EndAction()
{
    if (pDragBla!=NULL)
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
    if (pDragBla!=NULL) {
        rRect=aDragStat.GetActionRect();
        if (rRect.IsEmpty()) {
            BOOL b1st=TRUE;
            SdrPageView* pPV = GetSdrPageView();

            if(pPV)
            {
                if (pPV->HasMarkedObjPageView())
                {
                    const basegfx::B2DRange aBoundRange(basegfx::tools::getRange(basegfx::tools::adaptiveSubdivideByAngle(pPV->getDragPoly())));
                    const Rectangle aR(FRound(aBoundRange.getMinX()), FRound(aBoundRange.getMinY()), FRound(aBoundRange.getMaxX()), FRound(aBoundRange.getMaxY()));

                    if (b1st) {
                        b1st=FALSE;
                        rRect=aR;
                    } else {
                        rRect.Union(aR);
                    }
                }
            }
        }
        if (rRect.IsEmpty()) {
            rRect=Rectangle(aDragStat.GetNow(),aDragStat.GetNow());
        }
    } else {
        SdrExchangeView::TakeActionRect(rRect);
    }
}

void SdrDragView::SetDragPolys(bool bReset)
{
    SdrPageView* pPV = GetSdrPageView();
    ULONG nMarkAnz=GetMarkedObjectCount();

    if(bReset)
    {
        if(pPV)
        {
            pPV->setDragPoly(basegfx::B2DPolyPolygon());
        }
    }
    else if(IsDraggingPoints() || IsDraggingGluePoints())
    {
        BOOL bGlue=IsDraggingGluePoints();

        if(pPV)
        {
            pPV->setDragPoly0(basegfx::B2DPolyPolygon());
            if (pPV->HasMarkedObjPageView()) {
                for (ULONG nm=0; nm<nMarkAnz; nm++) {
                    SdrMark* pM=GetSdrMarkByIndex(nm);
                    if (pM->GetPageView()==pPV) {
                        const SdrUShortCont* pPts=bGlue ? pM->GetMarkedGluePoints() : pM->GetMarkedPoints();
                        if (pPts!=NULL && pPts->GetCount()!=0) {
                            const SdrObject* pObj=pM->GetMarkedSdrObj();
                            const SdrPathObj* pPath=bGlue ? NULL : PTR_CAST(SdrPathObj,pObj);
                            const basegfx::B2DPolyPolygon aPathXPP = (pPath) ? pPath->GetPathPoly() : basegfx::B2DPolyPolygon();
                            const SdrGluePointList* pGPL=bGlue ? pObj->GetGluePointList() : NULL;
                            const sal_uInt32 nPtAnz(pPts->GetCount());
                            basegfx::B2DPolygon aPolygon;

                            for(sal_uInt32 nPtNum(0L); nPtNum < nPtAnz; nPtNum++)
                            {
                                sal_uInt16 nObjPt(pPts->GetObject(nPtNum));

                                if(bGlue)
                                {
                                    if(pGPL)
                                    {
                                        sal_uInt16 nGlueNum(pGPL->FindGluePoint(nObjPt));

                                        if(SDRGLUEPOINT_NOTFOUND != nGlueNum)
                                        {
                                            const Point aPoint((*pGPL)[nGlueNum].GetAbsolutePos(*pObj));
                                            aPolygon.append(basegfx::B2DPoint(aPoint.X(), aPoint.Y()));
                                        }
                                    }
                                }
                                else
                                {
                                    if(pPath && aPathXPP.count())
                                    {
                                        sal_uInt32 nPolyNum, nPointNum;

                                        if(PolyPolygonEditor::GetRelativePolyPoint(aPathXPP, nObjPt, nPolyNum, nPointNum))
                                        {
                                            aPolygon.append(aPathXPP.getB2DPolygon(nPolyNum).getB2DPoint(nPointNum));
                                        }
                                    }
                                }
                            }

                            basegfx::B2DPolyPolygon aDragPoly(pPV->getDragPoly0());
                            aDragPoly.append(aPolygon);
                            pPV->setDragPoly0(aDragPoly);
                        }
                    }
                }
            }
            pPV->setDragPoly(pPV->getDragPoly0());
        }
        return;
    }
    else
    {
        Rectangle aRect;
        XPolygon aEmptyPoly(0); // Lerres XPoly fuer Separate
        ULONG nMaxObj=nDragXorPolyLimit;
        ULONG nMaxPnt=nDragXorPointLimit;
        BOOL bNoPoly = IsNoDragXorPolys() || GetMarkedObjectCount()>nMaxObj;
        BOOL bBrk=FALSE;
        ULONG nPolyCnt=0; // Max nDragXorPolyLimit Polys
        ULONG nPntCnt=0;  // Max 5*nDragXorPolyLimit Punkte

        if(!bNoPoly)
        {
            if(pPV)
            {
                if(pPV->HasMarkedObjPageView())
                {
                    pPV->setDragPoly0(basegfx::B2DPolyPolygon());
                    basegfx::B2DPolyPolygon aDazuPP;

                    for(ULONG nm=0; nm<nMarkAnz && !bBrk; nm++)
                    {
                        SdrMark* pM=GetSdrMarkByIndex(nm);

                        if(pM->GetPageView()==pPV)
                        {
                            aDazuPP = pM->GetMarkedSdrObj()->TakeXorPoly(sal_False);
                            const sal_uInt32 nDazuPolyAnz(aDazuPP.count());
                            nPolyCnt += nDazuPolyAnz;

                            for(sal_uInt32 i(0L); i < nDazuPolyAnz; i++)
                            {
                                nPntCnt += aDazuPP.getB2DPolygon(i).count();
                            }

                            if(nPolyCnt > nMaxObj || nPntCnt > nMaxPnt)
                            {
                                bBrk = TRUE;
                                bNoPoly = TRUE;
                            }

                            if(!bBrk)
                            {
                                basegfx::B2DPolyPolygon aPolyPoygon(pPV->getDragPoly0());
                                aPolyPoygon.append(aDazuPP);
                                pPV->setDragPoly0(aPolyPoygon);
                            }
                        }
                    }

                    pPV->setDragPoly(pPV->getDragPoly0());
                }
            }
        }

        if(bNoPoly)
        {
            if(pPV)
            {
                if (pPV->HasMarkedObjPageView())
                {
                    const Rectangle aR(pPV->MarkSnap());
                    const basegfx::B2DRange aNewRectangle(aR.Left(), aR.Top(), aR.Right(), aR.Bottom());
                    basegfx::B2DPolygon aNewPolygon(basegfx::tools::createPolygonFromRect(aNewRectangle));
                    aNewPolygon = basegfx::tools::expandToCurve(aNewPolygon);
                    pPV->setDragPoly0(basegfx::B2DPolyPolygon(aNewPolygon));
                    pPV->setDragPoly(pPV->getDragPoly0());
                }
            }
        }
    }
}

BOOL SdrDragView::TakeDragObjAnchorPos(Point& rPos, BOOL bTR ) const
{
    Rectangle aR;
    TakeActionRect(aR);
    rPos = bTR ? aR.TopRight() : aR.TopLeft();
    if (GetMarkedObjectCount()==1 && IsDragObj() && // nur bei Einzelselektion
        !IsDraggingPoints() && !IsDraggingGluePoints() && // nicht beim Punkteschieben
        !pDragBla->ISA(SdrDragMovHdl)) // nicht beim Handlesschieben
    {
        SdrObject* pObj=GetMarkedObjectByIndex(0);
        if (pObj->ISA(SdrCaptionObj)) {
            Point aPt(((SdrCaptionObj*)pObj)->GetTailPos());
            BOOL bTail=eDragHdl==HDL_POLY; // Schwanz wird gedraggt (nicht so ganz feine Abfrage hier)
            BOOL bOwn=pDragBla->ISA(SdrDragObjOwn); // Objektspeziefisch
            if (!bTail) { // bei bTail liefert TakeActionRect schon das richtige
                if (bOwn) { // bOwn kann sein MoveTextFrame, ResizeTextFrame aber eben nicht mehr DragTail
                    rPos=aPt;
                } else {
                    // hier nun dragging des gesamten Objekts (Move, Resize, ...)
                    pDragBla->MovPoint(aPt); // ,Point()); //GetSdrPageViewOfMarkedByIndex(0)->GetOffset());
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
        pDragBla=NULL;
        bDragSpecial=FALSE;
        bDragLimit=FALSE;
        SdrDragMode eTmpMode=eDragMode;
        if (eTmpMode==SDRDRAG_MOVE && pHdl!=NULL && pHdl->GetKind()!=HDL_MOVE) {
            eTmpMode=SDRDRAG_RESIZE;
        }
        bDragLimit=TakeDragLimit(eTmpMode,aDragLimit);
        bFramDrag=ImpIsFrameHandles();
        if (!bFramDrag &&
            (pMarkedObj==NULL || !pMarkedObj->HasSpecialDrag()) &&
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

        if(bDragHdl)
        {
            pDragBla = new SdrDragMovHdl(*this);
        }
        else if(!bNotDraggable)
        {
            switch (eDragMode) {
                case SDRDRAG_ROTATE: case SDRDRAG_SHEAR: case SDRDRAG_DISTORT: {
                    switch (eDragHdl) {
                        case HDL_LEFT:  case HDL_RIGHT:
                        case HDL_UPPER: case HDL_LOWER: {
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
                            pDragBla=new SdrDragShear(*this,eDragMode==SDRDRAG_ROTATE);
                        } break;
                        case HDL_UPLFT: case HDL_UPRGT:
                        case HDL_LWLFT: case HDL_LWRGT: {
                            if (eDragMode==SDRDRAG_SHEAR || eDragMode==SDRDRAG_DISTORT) {
                                if (!IsDistortAllowed(TRUE) && !IsDistortAllowed(FALSE)) return FALSE;
                                pDragBla=new SdrDragDistort(*this);
                            } else {
                                if (!IsRotateAllowed(TRUE)) return FALSE;
                                pDragBla=new SdrDragRotate(*this);
                            }
                        } break;
                        default: {
                            if (IsMarkedHitMovesAlways() && eDragHdl==HDL_MOVE) { // HDL_MOVE ist auch wenn Obj direkt getroffen
                                if (!IsMoveAllowed()) return FALSE;
                                pDragBla=new SdrDragMove(*this);
                            } else {
                                if (!IsRotateAllowed(TRUE)) return FALSE;
                                pDragBla=new SdrDragRotate(*this);
                            }
                        }
                    }
                } break;
                case SDRDRAG_MIRROR: {
                    if (eDragHdl==HDL_MOVE && IsMarkedHitMovesAlways()) {
                        if (!IsMoveAllowed()) return FALSE;
                        pDragBla=new SdrDragMove(*this);
                    } else {
                        if (!IsMirrorAllowed(TRUE,TRUE)) return FALSE;
                        pDragBla=new SdrDragMirror(*this);
                    }
                } break;

                case SDRDRAG_CROP:
                {
                    if (eDragHdl==HDL_MOVE && IsMarkedHitMovesAlways())
                    {
                        if (!IsMoveAllowed())
                            return FALSE;
                        pDragBla=new SdrDragMove(*this);
                    }
                    else
                    {
                        if (!IsCrookAllowed(TRUE) && !IsCrookAllowed(FALSE))
                            return FALSE;
                        pDragBla=new SdrDragCrop(*this);
                    }
                }
                break;

                case SDRDRAG_TRANSPARENCE:
                {
                    if(eDragHdl == HDL_MOVE && IsMarkedHitMovesAlways())
                    {
                        if(!IsMoveAllowed())
                            return FALSE;
                        pDragBla = new SdrDragMove(*this);
                    }
                    else
                    {
                        if(!IsTransparenceAllowed())
                            return FALSE;

                        pDragBla = new SdrDragGradient(*this, FALSE);
                    }
                    break;
                }
                case SDRDRAG_GRADIENT:
                {
                    if(eDragHdl == HDL_MOVE && IsMarkedHitMovesAlways())
                    {
                        if(!IsMoveAllowed())
                            return FALSE;
                        pDragBla = new SdrDragMove(*this);
                    }
                    else
                    {
                        if(!IsGradientAllowed())
                            return FALSE;

                        pDragBla = new SdrDragGradient(*this);
                    }
                    break;
                }

                case SDRDRAG_CROOK : {
                    if (eDragHdl==HDL_MOVE && IsMarkedHitMovesAlways()) {
                        if (!IsMoveAllowed()) return FALSE;
                        pDragBla=new SdrDragMove(*this);
                    } else {
                        if (!IsCrookAllowed(TRUE) && !IsCrookAllowed(FALSE)) return FALSE;
                        pDragBla=new SdrDragCrook(*this);
                    }
                } break;

                default:
                {   // SDRDRAG_MOVE
                    if ( ( eDragHdl == HDL_MOVE ) && !IsMoveAllowed() )
                        return FALSE;
                    else if ( eDragHdl==HDL_GLUE )
                        pDragBla = new SdrDragMove(*this);
                    else
                    {
                        if ( bFramDrag )
                        {
                            if ( eDragHdl == HDL_MOVE )
                                pDragBla=new SdrDragMove(*this);
                            else
                            {
                                if (!IsResizeAllowed(TRUE)) return FALSE;
                                pDragBla=new SdrDragResize(*this);
                            }
                        }
                        else
                        {
                            if ( ( eDragHdl == HDL_MOVE ) && ( GetMarkedObjectCount() == 1 )
                                && GetMarkedObjectByIndex( 0 )->ISA( SdrObjCustomShape ) )
                                pDragBla = new SdrDragMove( *this );
                            else
                            {
                                bDragSpecial=TRUE;
                                pDragBla=new SdrDragObjOwn(*this);
                            }
                        }
                    }
                }
            }
        }
        if (pForcedMeth!=NULL) {
            delete pDragBla;
            pDragBla=pForcedMeth;
        }
        aDragStat.SetDragMethod(pDragBla);
        if (pDragBla!=NULL) {
            bRet=pDragBla->Beg();
            if (!bRet) {
                if (pHdl==NULL && IS_TYPE(SdrDragObjOwn,pDragBla)) {
                    // Aha, Obj kann nicht Move SpecialDrag, also MoveFrameDrag versuchen
                    delete pDragBla;
                    pDragBla=NULL;
                    bDragSpecial=FALSE;
                    if (!IsMoveAllowed()) return FALSE;
                    bFramDrag=TRUE;
                    pDragBla=new SdrDragMove(*this);
                    aDragStat.SetDragMethod(pDragBla);
                    bRet=pDragBla->Beg();
                }
            }
            if (!bRet) {
                delete pDragBla;
                pDragBla=NULL;
                aDragStat.SetDragMethod(pDragBla);
            }
        }
    }

    return bRet;
}

void SdrDragView::MovDragObj(const Point& rPnt)
{
    if (pDragBla!=NULL)
    {
        Point aPnt(rPnt);
        ImpLimitToWorkArea(aPnt);
        pDragBla->Mov(aPnt); // this call already makes a Hide()/Show combination
//SDO       if (/*HMHIsDragHdlHide() &&*/ aDragStat.IsMinMoved() && !bDragHdl /*HMH&& IsMarkHdlShown()*/) {
//SDO           //HMHBOOL bLeaveRefs=IS_TYPE(SdrDragMirror,pDragBla) || IS_TYPE(SdrDragRotate,pDragBla);
//SDO           BOOL bFlag=IsSolidMarkHdl() && aDragStat.IsShown();
//SDO           if (bFlag) HideDragObj();
//SDO           //HMHHideMarkHdl(bLeaveRefs);
//SDO           if (bFlag) ShowDragObj();
//SDO       }
    }
}

BOOL SdrDragView::EndDragObj(BOOL bCopy)
{
    bool bRet(false);

    // #i73341# If insert GluePoint, do not insist on last points being different
    if(pDragBla && aDragStat.IsMinMoved() && (IsInsertGluePoint() || aDragStat.GetNow() != aDragStat.GetPrev()))
    {
        ULONG nHdlAnzMerk=0;
        if (bEliminatePolyPoints) { // IBM Special
            nHdlAnzMerk=GetMarkablePointCount();
        }
        if (IsInsertGluePoint()) {
            BegUndo(aInsPointUndoStr);
            AddUndo(pInsPointUndo);
        }
        bRet=pDragBla->End(bCopy);
        if (IsInsertGluePoint()) EndUndo();
        delete pDragBla;
        pDragBla=NULL;
        if (bEliminatePolyPoints) { // IBM Special
            if (nHdlAnzMerk!=GetMarkablePointCount()) {
                UnmarkAllPoints();
            }
        }
        if (bInsPolyPoint) {
            //HMHBOOL bVis=IsMarkHdlShown();
            //HMHif (bVis) HideMarkHdl();
            SetMarkHandles();
            bInsPolyPoint=FALSE;
            //HMHif (bVis) ShowMarkHdl();
            BegUndo(aInsPointUndoStr);
            AddUndo(pInsPointUndo);
            EndUndo();
        }

        eDragHdl=HDL_MOVE;
        pDragHdl=NULL;

        if (!bSomeObjChgdFlag)
        {
            // Aha, Obj hat nicht gebroadcastet (z.B. Writer FlyFrames)
            if(/*HMHIsDragHdlHide() &&*/ !bDragHdl && !IS_TYPE(SdrDragMirror,pDragBla) && !IS_TYPE(SdrDragRotate,pDragBla))
            {
                AdjustMarkHdl();
                //HMHShowMarkHdl();
            }
        }
        SetDragPolys(true);
    } else {
        BrkDragObj();
    }
    bInsPolyPoint=FALSE;
    SetInsertGluePoint(FALSE);

    return bRet;
}

void SdrDragView::BrkDragObj()
{
    if (pDragBla!=NULL) {
        pDragBla->Brk();
        delete pDragBla;
        pDragBla=NULL;
        if (bInsPolyPoint) {
            //HMHBOOL bVis=IsMarkHdlShown();
            //HMHif (bVis) HideMarkHdl();
            pInsPointUndo->Undo(); // Den eingefuegten Punkt wieder raus
            delete pInsPointUndo;
            pInsPointUndo=NULL;
            SetMarkHandles();
            bInsPolyPoint=FALSE;
            //HMHif (bVis) ShowMarkHdl();
        }
        if (IsInsertGluePoint()) {
            pInsPointUndo->Undo(); // Den eingefuegten Klebepunkt wieder raus
            delete pInsPointUndo;
            pInsPointUndo=NULL;
            SetInsertGluePoint(FALSE);
        }
//HMH       if (IsDragHdlHide() && !bDragHdl &&
//HMH           !IS_TYPE(SdrDragMirror,pDragBla) && !IS_TYPE(SdrDragRotate,pDragBla))
//HMH       {
//HMH           ShowMarkHdl();
//HMH       }
        eDragHdl=HDL_MOVE;
        pDragHdl=NULL;
        SetDragPolys(true);
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
        Point aPt(rPnt); // - pMarkedPV->GetOffset());

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
            //HMHsal_Bool bVis(IsMarkHdlShown());

            //HMHif(bVis)
            //HMH   HideMarkHdl();

            bInsPolyPoint = sal_True;
            UnmarkAllPoints();
            AdjustMarkHdl();

            //HMHif(bVis)
            //HMH   ShowMarkHdl();

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
        if (bOk==TRUE && eCmd!=SDRCREATE_FORCEEND) {
            // Ret=True bedeutet: Action ist vorbei.
            bOk=!(ImpBegInsObjPoint(sal_True, nNextPnt, aPnt, eCmd == SDRCREATE_NEXTOBJECT, pDragWin));
        }

        return bOk;
    } else return FALSE;
}

BOOL SdrDragView::IsInsGluePointPossible() const
{
    BOOL bRet=FALSE;
    if (IsInsGluePointMode() && AreObjectsMarked()) {
        if (GetMarkedObjectCount()==1) {
            // FALSE liefern, wenn 1 Objekt und dieses ein Verbinder ist.
            const SdrObject* pObj=GetMarkedObjectByIndex(0);
            if (!HAS_BASE(SdrEdgeObj,pObj)) {
               bRet=TRUE;
            }
        } else {
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
    if (PickMarkedObj(rPnt,pObj,pPV,&nMarkNum,SDRSEARCH_PASS2BOUND)) {
        BrkAction();
        UnmarkAllGluePoints();
        //SdrMark* pM=GetSdrMarkByIndex(nMarkNum);
        pInsPointUndo= dynamic_cast< SdrUndoGeoObj* >( GetModel()->GetSdrUndoFactory().CreateUndoGeoObject(*pObj) );
        DBG_ASSERT( pInsPointUndo, "svx::SdrDragView::BegInsObjPoint(), could not create correct undo object!" );
        XubString aStr(ImpGetResStr(STR_DragInsertGluePoint));
        XubString aName; pObj->TakeObjNameSingul(aName);

        aStr.SearchAndReplaceAscii("%1", aName);

        aInsPointUndoStr=aStr;
        SdrGluePointList* pGPL=pObj->ForceGluePointList();
        if (pGPL!=NULL) {
            USHORT nGlueIdx=pGPL->Insert(SdrGluePoint());
            SdrGluePoint& rGP=(*pGPL)[nGlueIdx];
            USHORT nGlueId=rGP.GetId();
            rGP.SetAbsolutePos(rPnt,*pObj);

            SdrHdl* pHdl=NULL;
            if (MarkGluePoint(pObj,nGlueId,pPV)) {
                pHdl=GetGluePointHdl(pObj,nGlueId);
            }
            if (pHdl!=NULL && pHdl->GetKind()==HDL_GLUE && pHdl->GetObj()==pObj && pHdl->GetObjHdlNum()==nGlueId) {
                SetInsertGluePoint(TRUE);
                bRet=BegDragObj(rPnt,NULL,pHdl,0);
                if (bRet) {
                    aDragStat.SetMinMoved();
                    MovDragObj(rPnt);
                } else {
                    SetInsertGluePoint(FALSE);
                    delete pInsPointUndo;
                    pInsPointUndo=NULL;
                }
            } else {
                DBG_ERROR("BegInsGluePoint(): GluePoint-Handle nicht gefunden");
            }
        } else {
            // Keine Klebepunkte moeglich bei diesem Objekt (z.B. Edge)
            SetInsertGluePoint(FALSE);
            delete pInsPointUndo;
            pInsPointUndo=NULL;
        }
    }

    return bRet;
}

BOOL SdrDragView::IsMoveOnlyDragObj(BOOL bAskRTTI) const
{
    bool bRet=false;
    if (pDragBla!=NULL && !IsDraggingPoints() && !IsDraggingGluePoints()) {
        if (bAskRTTI) {
            bRet=IS_TYPE(SdrDragMove,pDragBla);
        } else {
            bRet=pDragBla->IsMoveOnly();
        }
    }
    return bRet;
}

void SdrDragView::ShowDragObj()
{
    if(pDragBla && !aDragStat.IsShown())
    {
        // for migration from XOR, replace DrawDragObj here to create
        // overlay objects instead.
        if(pDragBla)
        {
            mpDragViewExtraData->CreateAndShowOverlay(*this);
        }

        aDragStat.SetShown(TRUE);
    }
}

void SdrDragView::HideDragObj()
{
    if(pDragBla && aDragStat.IsShown())
    {
        // for migration from XOR, replace DrawDragObj here to create
        // overlay objects instead.
        mpDragViewExtraData->HideOverlay();

        //DrawDragObj(pOut, FALSE);
        aDragStat.SetShown(FALSE);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrDragView::SetNoDragXorPolys(BOOL bOn)
{
    if (IsNoDragXorPolys()!=bOn) {
        BOOL bDragging=pDragBla!=NULL;
        BOOL bShown=bDragging && aDragStat.IsShown();
        if (bShown) HideDragObj();
        bNoDragXorPolys=bOn;
        if (bDragging) {
            SetDragPolys();
            pDragBla->MovAllPoints(); // die gedraggten Polys neu berechnen
        }
        if (bShown) ShowDragObj();
    }
}

void SdrDragView::SetDragStripes(BOOL bOn)
{
    if (pDragBla!=NULL && aDragStat.IsShown()) {
        HideDragObj();
        bDragStripes=bOn;
        ShowDragObj();
    } else {
        bDragStripes=bOn;
    }
}

//HMHvoid SdrDragView::SetDragHdlHide(BOOL bOn)
//HMH{
//HMH   bNoDragHdl=bOn;
//HMH   if (pDragBla!=NULL && !bDragHdl && !IS_TYPE(SdrDragMirror,pDragBla) && !IS_TYPE(SdrDragRotate,pDragBla))
//HMH   {
//HMH       if (bOn) HideMarkHdl();
//HMH       else ShowMarkHdl();
//HMH   }
//HMH}

BOOL SdrDragView::IsOrthoDesired() const
{
    if (pDragBla!=NULL && (IS_TYPE(SdrDragObjOwn,pDragBla) || IS_TYPE(SdrDragResize,pDragBla))) {
        return bOrthoDesiredOnMarked;
    }
    return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrDragView::SetRubberEdgeDragging(BOOL bOn)
{
    if (bOn!=IsRubberEdgeDragging()) {
        ULONG nAnz = GetEdgesOfMarkedNodes().GetMarkCount();
        BOOL bShowHide=nAnz!=0 && IsDragObj() &&
                 (nRubberEdgeDraggingLimit>=nAnz);
        if (bShowHide) HideDragObj();
        bRubberEdgeDragging=bOn;
        if (bShowHide) ShowDragObj();
    }
}

void SdrDragView::SetRubberEdgeDraggingLimit(USHORT nEdgeObjAnz)
{
    if (nEdgeObjAnz!=nRubberEdgeDraggingLimit) {
        ULONG nAnz = GetEdgesOfMarkedNodes().GetMarkCount();
        BOOL bShowHide=IsRubberEdgeDragging() && nAnz!=0 && IsDragObj() &&
                 (nEdgeObjAnz>=nAnz)!=(nRubberEdgeDraggingLimit>=nAnz);
        if (bShowHide) HideDragObj();
        nRubberEdgeDraggingLimit=nEdgeObjAnz;
        if (bShowHide) ShowDragObj();
    }
}

void SdrDragView::SetDetailedEdgeDragging(BOOL bOn)
{
    if (bOn!=IsDetailedEdgeDragging()) {
        ULONG nAnz = GetEdgesOfMarkedNodes().GetMarkCount();
        BOOL bShowHide=nAnz!=0 && IsDragObj() &&
                 (nDetailedEdgeDraggingLimit>=nAnz);
        if (bShowHide) HideDragObj();
        bDetailedEdgeDragging=bOn;
        if (bShowHide) ShowDragObj();
    }
}

void SdrDragView::SetDetailedEdgeDraggingLimit(USHORT nEdgeObjAnz)
{
    if (nEdgeObjAnz!=nDetailedEdgeDraggingLimit) {
        ULONG nAnz = GetEdgesOfMarkedNodes().GetMarkCount();
        BOOL bShowHide=IsDetailedEdgeDragging() && nAnz!=0 && IsDragObj() &&
                 (nEdgeObjAnz>=nAnz)!=(nDetailedEdgeDraggingLimit>=nAnz);
        if (bShowHide) HideDragObj();
        nDetailedEdgeDraggingLimit=nEdgeObjAnz;
        if (bShowHide) ShowDragObj();
    }
}

void SdrDragView::SetMarkHandles()
{
//  DBG_ASSERT( (pDragBla == 0) && (pDragHdl == 0), "svx::SdrDragView::SetMarkHandles(), who calls me during drag operation?" );

    if( pDragHdl )
        pDragHdl = 0;

    SdrExchangeView::SetMarkHandles();
}

// eof
