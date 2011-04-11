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

#include <svx/svdmrkv.hxx>
#include <svx/svdetc.hxx>
#include <svx/svdoedge.hxx>
#include "svx/svdglob.hxx"
#include "svx/svditext.hxx"
#include <svx/svdview.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdpage.hxx>
#include "svddrgm1.hxx"

#ifdef DBG_UTIL
#include <svdibrow.hxx>
#endif

#include <svx/svdoole2.hxx>
#include <svx/xgrad.hxx>
#include <svx/xflgrit.hxx>
#include "gradtrns.hxx"
#include <svx/xflftrit.hxx>
#include <svx/dialmgr.hxx>
#include "svx/svdstr.hrc"
#include <svx/svdundo.hxx>
#include <svx/svdopath.hxx>
#include <svx/scene3d.hxx>
#include <svx/svdovirt.hxx>
#include <svx/sdr/overlay/overlayrollingrectangle.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <svx/sdrpagewindow.hxx>
#include <svx/sdrhittesthelper.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////
// predefines

class SdrUnoControlList;

////////////////////////////////////////////////////////////////////////////////////////////////////
// #114409#-3 Migrate Marking of Objects, Points and GluePoints

class ImplMarkingOverlay
{
    // The OverlayObjects
    ::sdr::overlay::OverlayObjectList               maObjects;

    // The remembered second position in logical coodinates
    basegfx::B2DPoint                               maSecondPosition;

    // bitfield
    // A flag to remember if the action is for unmarking.
    unsigned                                        mbUnmarking : 1;

public:
    ImplMarkingOverlay(const SdrPaintView& rView, const basegfx::B2DPoint& rStartPos, sal_Bool bUnmarking = sal_False);
    ~ImplMarkingOverlay();

    void SetSecondPosition(const basegfx::B2DPoint& rNewPosition);
    sal_Bool IsUnmarking() const { return mbUnmarking; }
};

ImplMarkingOverlay::ImplMarkingOverlay(const SdrPaintView& rView, const basegfx::B2DPoint& rStartPos, sal_Bool bUnmarking)
:   maSecondPosition(rStartPos),
    mbUnmarking(bUnmarking)
{
    for(sal_uInt32 a(0L); a < rView.PaintWindowCount(); a++)
    {
        SdrPaintWindow* pCandidate = rView.GetPaintWindow(a);
        ::sdr::overlay::OverlayManager* pTargetOverlay = pCandidate->GetOverlayManager();

        if(pTargetOverlay)
        {
            ::sdr::overlay::OverlayRollingRectangleStriped* pNew = new ::sdr::overlay::OverlayRollingRectangleStriped(
                rStartPos, rStartPos, false);
            pTargetOverlay->add(*pNew);
            maObjects.append(*pNew);
        }
    }
}

ImplMarkingOverlay::~ImplMarkingOverlay()
{
    // The OverlayObjects are cleared using the destructor of OverlayObjectList.
    // That destructor calls clear() at the list which removes all objects from the
    // OverlayManager and deletes them.
}

void ImplMarkingOverlay::SetSecondPosition(const basegfx::B2DPoint& rNewPosition)
{
    if(rNewPosition != maSecondPosition)
    {
        // apply to OverlayObjects
        for(sal_uInt32 a(0L); a < maObjects.count(); a++)
        {
            ::sdr::overlay::OverlayRollingRectangleStriped& rCandidate = (::sdr::overlay::OverlayRollingRectangleStriped&)maObjects.getOverlayObject(a);
            rCandidate.setSecondPosition(rNewPosition);
        }

        // remember new position
        maSecondPosition = rNewPosition;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  @@   @@  @@@@  @@@@@  @@  @@  @@ @@ @@ @@@@@ @@   @@
//  @@@ @@@ @@  @@ @@  @@ @@  @@  @@ @@ @@ @@    @@   @@
//  @@@@@@@ @@  @@ @@  @@ @@ @@   @@ @@ @@ @@    @@ @ @@
//  @@@@@@@ @@@@@@ @@@@@  @@@@    @@@@@ @@ @@@@  @@@@@@@
//  @@ @ @@ @@  @@ @@  @@ @@ @@    @@@  @@ @@    @@@@@@@
//  @@   @@ @@  @@ @@  @@ @@  @@   @@@  @@ @@    @@@ @@@
//  @@   @@ @@  @@ @@  @@ @@  @@    @   @@ @@@@@ @@   @@
//
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrMarkView::ImpClearVars()
{
    eDragMode=SDRDRAG_MOVE;
    //HMHbHdlShown=sal_False;
    bRefHdlShownOnly=sal_False;
    eEditMode=SDREDITMODE_EDIT;
    eEditMode0=SDREDITMODE_EDIT;
    bDesignMode=sal_False;
    pMarkedObj=NULL;
    pMarkedPV=NULL;
    bForceFrameHandles=sal_False;
    bPlusHdlAlways=sal_False;
    nFrameHandlesLimit=50;
    bInsPolyPoint=sal_False;
    mnInsPointNum = 0L;
    bMarkedObjRectDirty=sal_False;
    bMarkedPointsRectsDirty=sal_False;
    mbMarkHandlesHidden = false;
    bMrkPntDirty=sal_False;
    bMarkHdlWhenTextEdit=sal_False;
    bMarkableObjCountDirty=sal_False; // noch nicht implementiert
    nMarkableObjCount=0;          // noch nicht implementiert

    // #114409#-3 Migrate selections
    BrkMarkObj();
    BrkMarkPoints();
    BrkMarkGluePoints();
}

SdrMarkView::SdrMarkView(SdrModel* pModel1, OutputDevice* pOut)
:   SdrSnapView(pModel1,pOut),
    mpMarkObjOverlay(0L),
    mpMarkPointsOverlay(0L),
    mpMarkGluePointsOverlay(0L),
    aHdl(this),
    mpSdrViewSelection(new sdr::ViewSelection())
{
    ImpClearVars();
    StartListening(*pModel1);
}

SdrMarkView::~SdrMarkView()
{
    // #114409#-3 Migrate selections
    BrkMarkObj();
    BrkMarkPoints();
    BrkMarkGluePoints();
    delete mpSdrViewSelection;
}

void SdrMarkView::Notify(SfxBroadcaster& rBC, const SfxHint& rHint)
{
    SdrHint* pSdrHint=PTR_CAST(SdrHint,&rHint);
    if (pSdrHint!=NULL)
    {
        SdrHintKind eKind=pSdrHint->GetKind();

        if (eKind==HINT_OBJCHG || eKind==HINT_OBJINSERTED || eKind==HINT_OBJREMOVED)
        {
            bMarkedObjRectDirty=sal_True;
            bMarkedPointsRectsDirty=sal_True;
        }
/* removed for now since this breaks existing code who iterates over the mark list and sequentially replaces objects
        if( eKind==HINT_OBJREMOVED && IsObjMarked( const_cast<SdrObject*>(pSdrHint->GetObject()) ) )
        {
            MarkObj( const_cast<SdrObject*>(pSdrHint->GetObject()), GetSdrPageView(), sal_True );
        }
*/
    }
    SdrSnapView::Notify(rBC,rHint);
}

void SdrMarkView::ModelHasChanged()
{
    SdrPaintView::ModelHasChanged();
    GetMarkedObjectListWriteAccess().SetNameDirty();
    bMarkedObjRectDirty=sal_True;
    bMarkedPointsRectsDirty=sal_True;
    // Es sind beispielsweise Obj markiert und maMarkedObjectListist Sorted.
    // In einer anderen View 2 wird die ObjOrder veraendert
    // (z.B. MovToTop()). Dann ist Neusortieren der MarkList erforderlich.
    GetMarkedObjectListWriteAccess().SetUnsorted();
    SortMarkedObjects();
    bMrkPntDirty=sal_True;
    UndirtyMrkPnt();
    SdrView* pV=(SdrView*)this;
    if (pV!=NULL && !pV->IsDragObj() && !pV->IsInsObjPoint()) { // an dieser Stelle habe ich ein ziemliches Problem !!!
        AdjustMarkHdl();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

sal_Bool SdrMarkView::IsAction() const
{
    return SdrSnapView::IsAction() || IsMarkObj() || IsMarkPoints() || IsMarkGluePoints();
}

void SdrMarkView::MovAction(const Point& rPnt)
{
    SdrSnapView::MovAction(rPnt);

    if(IsMarkObj())
    {
        MovMarkObj(rPnt);
    }
    else if(IsMarkPoints())
    {
        MovMarkPoints(rPnt);
    }
    else if(IsMarkGluePoints())
    {
        MovMarkGluePoints(rPnt);
    }
}

void SdrMarkView::EndAction()
{
    if(IsMarkObj())
    {
        EndMarkObj();
    }
    else if(IsMarkPoints())
    {
        EndMarkPoints();
    }
    else if(IsMarkGluePoints())
    {
        EndMarkGluePoints();
    }

    SdrSnapView::EndAction();
}

void SdrMarkView::BckAction()
{
    SdrSnapView::BckAction();
    BrkMarkObj();
    BrkMarkPoints();
    BrkMarkGluePoints();
}

void SdrMarkView::BrkAction()
{
    SdrSnapView::BrkAction();
    BrkMarkObj();
    BrkMarkPoints();
    BrkMarkGluePoints();
}

void SdrMarkView::TakeActionRect(Rectangle& rRect) const
{
    if(IsMarkObj() || IsMarkPoints() || IsMarkGluePoints())
    {
        rRect = Rectangle(aDragStat.GetStart(), aDragStat.GetNow());
    }
    else
    {
        SdrSnapView::TakeActionRect(rRect);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrMarkView::ClearPageView()
{
    UnmarkAllObj();
    SdrSnapView::ClearPageView();
}

void SdrMarkView::HideSdrPage()
{
    bool bMrkChg(false);
    //HMHbool bVis(false);

    if(mpPageView)
    {
        // break all creation actions when hiding page (#75081#)
        BrkAction();
        //HMHbVis = IsMarkHdlShown();

        //HMHif(bVis)
        //HMH{
        //HMH   HideMarkHdl();
        //HMH}

        // Alle Markierungen dieser Seite verwerfen
        bMrkChg = GetMarkedObjectListWriteAccess().DeletePageView(*mpPageView);
    }

    SdrSnapView::HideSdrPage();

    if(bMrkChg)
    {
        MarkListHasChanged();
        AdjustMarkHdl();
    }

    //HMHif(bVis)
    //HMH{
    //HMH   ShowMarkHdl();
    //HMH}
}

////////////////////////////////////////////////////////////////////////////////////////////////////

sal_Bool SdrMarkView::BegMarkObj(const Point& rPnt, sal_Bool bUnmark)
{
    BrkAction();

    DBG_ASSERT(0L == mpMarkObjOverlay, "SdrMarkView::BegMarkObj: There exists a mpMarkObjOverlay (!)");
    basegfx::B2DPoint aStartPos(rPnt.X(), rPnt.Y());
    mpMarkObjOverlay = new ImplMarkingOverlay(*this, aStartPos, bUnmark);

    aDragStat.Reset(rPnt);
    aDragStat.NextPoint();
    aDragStat.SetMinMove(nMinMovLog);

    return sal_True;
}

void SdrMarkView::MovMarkObj(const Point& rPnt)
{
    if(IsMarkObj() && aDragStat.CheckMinMoved(rPnt))
    {
        aDragStat.NextMove(rPnt);
        DBG_ASSERT(mpMarkObjOverlay, "SdrSnapView::MovSetPageOrg: no ImplPageOriginOverlay (!)");
        basegfx::B2DPoint aNewPos(rPnt.X(), rPnt.Y());
        mpMarkObjOverlay->SetSecondPosition(aNewPos);
    }
}

sal_Bool SdrMarkView::EndMarkObj()
{
    sal_Bool bRetval(sal_False);

    if(IsMarkObj())
    {
        if(aDragStat.IsMinMoved())
        {
            Rectangle aRect(aDragStat.GetStart(), aDragStat.GetNow());
            aRect.Justify();
            MarkObj(aRect, mpMarkObjOverlay->IsUnmarking());
            bRetval = sal_True;
        }

        // cleanup
        BrkMarkObj();
    }

    return bRetval;
}

void SdrMarkView::BrkMarkObj()
{
    if(IsMarkObj())
    {
        DBG_ASSERT(mpMarkObjOverlay, "SdrSnapView::MovSetPageOrg: no ImplPageOriginOverlay (!)");
        delete mpMarkObjOverlay;
        mpMarkObjOverlay = 0L;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

sal_Bool SdrMarkView::BegMarkPoints(const Point& rPnt, sal_Bool bUnmark)
{
    if(HasMarkablePoints())
    {
        BrkAction();

        DBG_ASSERT(0L == mpMarkPointsOverlay, "SdrMarkView::BegMarkObj: There exists a mpMarkPointsOverlay (!)");
        basegfx::B2DPoint aStartPos(rPnt.X(), rPnt.Y());
        mpMarkPointsOverlay = new ImplMarkingOverlay(*this, aStartPos, bUnmark);

        aDragStat.Reset(rPnt);
        aDragStat.NextPoint();
        aDragStat.SetMinMove(nMinMovLog);

        return sal_True;
    }

    return sal_False;
}

void SdrMarkView::MovMarkPoints(const Point& rPnt)
{
    if(IsMarkPoints() && aDragStat.CheckMinMoved(rPnt))
    {
        aDragStat.NextMove(rPnt);

        DBG_ASSERT(mpMarkPointsOverlay, "SdrSnapView::MovSetPageOrg: no ImplPageOriginOverlay (!)");
        basegfx::B2DPoint aNewPos(rPnt.X(), rPnt.Y());
        mpMarkPointsOverlay->SetSecondPosition(aNewPos);
    }
}

sal_Bool SdrMarkView::EndMarkPoints()
{
    sal_Bool bRetval(sal_False);

    if(IsMarkPoints())
    {
        if(aDragStat.IsMinMoved())
        {
            Rectangle aRect(aDragStat.GetStart(), aDragStat.GetNow());
            aRect.Justify();
            MarkPoints(aRect, mpMarkPointsOverlay->IsUnmarking());

            bRetval = sal_True;
        }

        // cleanup
        BrkMarkPoints();
    }

    return bRetval;
}

void SdrMarkView::BrkMarkPoints()
{
    if(IsMarkPoints())
    {
        DBG_ASSERT(mpMarkPointsOverlay, "SdrSnapView::MovSetPageOrg: no ImplPageOriginOverlay (!)");
        delete mpMarkPointsOverlay;
        mpMarkPointsOverlay = 0L;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

sal_Bool SdrMarkView::BegMarkGluePoints(const Point& rPnt, sal_Bool bUnmark)
{
    if(HasMarkableGluePoints())
    {
        BrkAction();

        DBG_ASSERT(0L == mpMarkGluePointsOverlay, "SdrMarkView::BegMarkObj: There exists a mpMarkGluePointsOverlay (!)");
        basegfx::B2DPoint aStartPos(rPnt.X(), rPnt.Y());
        mpMarkGluePointsOverlay = new ImplMarkingOverlay(*this, aStartPos, bUnmark);

        aDragStat.Reset(rPnt);
        aDragStat.NextPoint();
        aDragStat.SetMinMove(nMinMovLog);

        return sal_True;
    }

    return sal_False;
}

void SdrMarkView::MovMarkGluePoints(const Point& rPnt)
{
    if(IsMarkGluePoints() && aDragStat.CheckMinMoved(rPnt))
    {
        aDragStat.NextMove(rPnt);

        DBG_ASSERT(mpMarkGluePointsOverlay, "SdrSnapView::MovSetPageOrg: no ImplPageOriginOverlay (!)");
        basegfx::B2DPoint aNewPos(rPnt.X(), rPnt.Y());
        mpMarkGluePointsOverlay->SetSecondPosition(aNewPos);
    }
}

sal_Bool SdrMarkView::EndMarkGluePoints()
{
    sal_Bool bRetval(sal_False);

    if(IsMarkGluePoints())
    {
        if(aDragStat.IsMinMoved())
        {
            Rectangle aRect(aDragStat.GetStart(),aDragStat.GetNow());
            aRect.Justify();
            MarkGluePoints(&aRect, mpMarkGluePointsOverlay->IsUnmarking());

            bRetval = sal_True;
        }

        // cleanup
        BrkMarkGluePoints();
    }

    return bRetval;
}

void SdrMarkView::BrkMarkGluePoints()
{
    if(IsMarkGluePoints())
    {
        DBG_ASSERT(mpMarkGluePointsOverlay, "SdrSnapView::MovSetPageOrg: no ImplPageOriginOverlay (!)");
        delete mpMarkGluePointsOverlay;
        mpMarkGluePointsOverlay = 0L;
    }
}

sal_Bool SdrMarkView::HasMarkableObj() const
{
    sal_uIntPtr nCount=0;

    SdrPageView* pPV = GetSdrPageView();
    if(pPV)
    {
        SdrObjList* pOL=pPV->GetObjList();
        sal_uIntPtr nObjAnz=pOL->GetObjCount();
        for (sal_uIntPtr nObjNum=0; nObjNum<nObjAnz && nCount==0; nObjNum++) {
            SdrObject* pObj=pOL->GetObj(nObjNum);
            if (IsObjMarkable(pObj,pPV)) {
                nCount++;
            }
        }
    }
    return nCount!=0;
}

sal_uIntPtr SdrMarkView::GetMarkableObjCount() const
{
    sal_uIntPtr nCount=0;
    SdrPageView* pPV = GetSdrPageView();

    if(pPV)
    {
        SdrObjList* pOL=pPV->GetObjList();
        sal_uIntPtr nObjAnz=pOL->GetObjCount();
        for (sal_uIntPtr nObjNum=0; nObjNum<nObjAnz; nObjNum++) {
            SdrObject* pObj=pOL->GetObj(nObjNum);
            if (IsObjMarkable(pObj,pPV)) {
                nCount++;
            }
        }
    }
    return nCount;
}

//HMHvoid SdrMarkView::ImpShowMarkHdl(bool /*bNoRefHdl*/)
//HMH{
//HMH   bNoRefHdl=sal_False; // geht leider erstmal nicht anders
//HMH   if (!bHdlShown) {
//HMH       bRefHdlShownOnly=sal_False;
//HMH       bHdlShown=sal_True;
//HMH   }
//HMH}

//HMHvoid SdrMarkView::ShowMarkHdl(bool /*bNoRefHdl*/)
//HMH{
//HMH   bNoRefHdl=sal_False; // geht leider erstmal nicht anders
//HMH   ImpShowMarkHdl(bNoRefHdl);
//HMH}


//HMHvoid SdrMarkView::HideMarkHdl(bool /*bNoRefHdl*/)
//HMH{
//HMH   bNoRefHdl=sal_False; // geht leider erstmal nicht anders
//HMH   if (bHdlShown) {
//HMH       bRefHdlShownOnly=bNoRefHdl;
//HMH       bHdlShown=sal_False;
//HMH   }
//HMH}

void SdrMarkView::hideMarkHandles()
{
    if(!mbMarkHandlesHidden)
    {
        mbMarkHandlesHidden = true;
        AdjustMarkHdl();
    }
}

void SdrMarkView::showMarkHandles()
{
    if(mbMarkHandlesHidden)
    {
        mbMarkHandlesHidden = false;
        AdjustMarkHdl();
    }
}

sal_Bool SdrMarkView::ImpIsFrameHandles() const
{
    sal_uIntPtr nMarkAnz=GetMarkedObjectCount();
    sal_Bool bFrmHdl=nMarkAnz>nFrameHandlesLimit || bForceFrameHandles;
    sal_Bool bStdDrag=eDragMode==SDRDRAG_MOVE;
    if (nMarkAnz==1 && bStdDrag && bFrmHdl)
    {
        const SdrObject* pObj=GetMarkedObjectByIndex(0);
        if (pObj->GetObjInventor()==SdrInventor)
        {
            sal_uInt16 nIdent=pObj->GetObjIdentifier();
            if (nIdent==OBJ_LINE || nIdent==OBJ_EDGE || nIdent==OBJ_CAPTION || nIdent==OBJ_MEASURE || nIdent==OBJ_CUSTOMSHAPE || nIdent==OBJ_TABLE )
            {
                bFrmHdl=sal_False;
            }
        }
    }
    if (!bStdDrag && !bFrmHdl) {
        // Grundsaetzlich erstmal alle anderen Dragmodi nur mit FrameHandles
        bFrmHdl=sal_True;
        if (eDragMode==SDRDRAG_ROTATE) {
            // bei Rotate ObjOwn-Drag, wenn mind. 1 PolyObj
            for (sal_uIntPtr nMarkNum=0; nMarkNum<nMarkAnz && bFrmHdl; nMarkNum++) {
                const SdrMark* pM=GetSdrMarkByIndex(nMarkNum);
                const SdrObject* pObj=pM->GetMarkedSdrObj();
                bFrmHdl=!pObj->IsPolyObj();
            }
        }
    }
    if (!bFrmHdl) {
        // FrameHandles, wenn wenigstens 1 Obj kein SpecialDrag kann
        for (sal_uIntPtr nMarkNum=0; nMarkNum<nMarkAnz && !bFrmHdl; nMarkNum++) {
            const SdrMark* pM=GetSdrMarkByIndex(nMarkNum);
            const SdrObject* pObj=pM->GetMarkedSdrObj();
            bFrmHdl=!pObj->hasSpecialDrag();
        }
    }
    return bFrmHdl;
}

void SdrMarkView::SetMarkHandles()
{
    // #105722# remember old focus handle values to search for it again
    const SdrHdl* pSaveOldFocusHdl = aHdl.GetFocusHdl();
    sal_Bool bSaveOldFocus(sal_False);
    sal_uInt32 nSavePolyNum(0L), nSavePointNum(0L);
    SdrHdlKind eSaveKind(HDL_MOVE);
    SdrObject* pSaveObj = NULL;

    if(pSaveOldFocusHdl
        && pSaveOldFocusHdl->GetObj()
        && pSaveOldFocusHdl->GetObj()->ISA(SdrPathObj)
        && (pSaveOldFocusHdl->GetKind() == HDL_POLY || pSaveOldFocusHdl->GetKind() == HDL_BWGT))
    {
        bSaveOldFocus = sal_True;
        nSavePolyNum = pSaveOldFocusHdl->GetPolyNum();
        nSavePointNum = pSaveOldFocusHdl->GetPointNum();
        pSaveObj = pSaveOldFocusHdl->GetObj();
        eSaveKind = pSaveOldFocusHdl->GetKind();
    }

    // delete/clear all handles. This will always be done, even with areMarkHandlesHidden()
    aHdl.Clear();
    aHdl.SetRotateShear(eDragMode==SDRDRAG_ROTATE);
    aHdl.SetDistortShear(eDragMode==SDRDRAG_SHEAR);
    pMarkedObj=NULL;
    pMarkedPV=NULL;

    // are handles enabled at all? Create only then
    if(!areMarkHandlesHidden())
    {
        sal_uIntPtr nMarkAnz=GetMarkedObjectCount();
        sal_Bool bStdDrag=eDragMode==SDRDRAG_MOVE;
        sal_Bool bSingleTextObjMark=sal_False;

        if (nMarkAnz==1)
        {
            pMarkedObj=GetMarkedObjectByIndex(0);
            bSingleTextObjMark =
                pMarkedObj &&
                pMarkedObj->ISA(SdrTextObj) &&
                static_cast<SdrTextObj*>(pMarkedObj)->IsTextFrame();
        }

        sal_Bool bFrmHdl=ImpIsFrameHandles();

        if (nMarkAnz>0)
        {
            pMarkedPV=GetSdrPageViewOfMarkedByIndex(0);

            for (sal_uIntPtr nMarkNum=0; nMarkNum<nMarkAnz && (pMarkedPV!=NULL || !bFrmHdl); nMarkNum++)
            {
                const SdrMark* pM=GetSdrMarkByIndex(nMarkNum);

                if (pMarkedPV!=pM->GetPageView())
                {
                    pMarkedPV=NULL;
                }
            }
        }

        if (bFrmHdl)
        {
            Rectangle aRect(GetMarkedObjRect());

            // #i33755#
            const sal_Bool bHideHandlesWhenInTextEdit(
                ((SdrView*)this)->IsTextEdit()
                && pMarkedObj
                && pMarkedObj->ISA(SdrTextObj)
                && ((SdrTextObj*)pMarkedObj)->IsInEditMode());

            if(!aRect.IsEmpty() && !bHideHandlesWhenInTextEdit)
            { // sonst nix gefunden
                if( bSingleTextObjMark )
                {
                    const sal_uIntPtr nSiz0=aHdl.GetHdlCount();
                    pMarkedObj->AddToHdlList(aHdl);
                    const sal_uIntPtr nSiz1=aHdl.GetHdlCount();
                    for (sal_uIntPtr i=nSiz0; i<nSiz1; i++)
                    {
                        SdrHdl* pHdl=aHdl.GetHdl(i);
                        pHdl->SetObj(pMarkedObj);
                        pHdl->SetPageView(pMarkedPV);
                        pHdl->SetObjHdlNum(sal_uInt16(i-nSiz0));
                    }
                }
                else if( eDragMode==SDRDRAG_CROP )
                {
                    aHdl.AddHdl(new SdrCropHdl(aRect.TopLeft()     ,HDL_UPLFT));
                    aHdl.AddHdl(new SdrCropHdl(aRect.TopCenter()   ,HDL_UPPER));
                    aHdl.AddHdl(new SdrCropHdl(aRect.TopRight()    ,HDL_UPRGT));
                    aHdl.AddHdl(new SdrCropHdl(aRect.LeftCenter()  ,HDL_LEFT ));
                    aHdl.AddHdl(new SdrCropHdl(aRect.RightCenter() ,HDL_RIGHT));
                    aHdl.AddHdl(new SdrCropHdl(aRect.BottomLeft()  ,HDL_LWLFT));
                    aHdl.AddHdl(new SdrCropHdl(aRect.BottomCenter(),HDL_LOWER));
                    aHdl.AddHdl(new SdrCropHdl(aRect.BottomRight() ,HDL_LWRGT));
                }
                else
                {
                    sal_Bool bWdt0=aRect.Left()==aRect.Right();
                    sal_Bool bHgt0=aRect.Top()==aRect.Bottom();
                    if (bWdt0 && bHgt0)
                    {
                        aHdl.AddHdl(new SdrHdl(aRect.TopLeft(),HDL_UPLFT));
                    }
                    else if (!bStdDrag && (bWdt0 || bHgt0))
                    {
                        aHdl.AddHdl(new SdrHdl(aRect.TopLeft()    ,HDL_UPLFT));
                        aHdl.AddHdl(new SdrHdl(aRect.BottomRight(),HDL_LWRGT));
                    }
                    else
                    {
                        if (!bWdt0 && !bHgt0) aHdl.AddHdl(new SdrHdl(aRect.TopLeft()     ,HDL_UPLFT));
                        if (          !bHgt0) aHdl.AddHdl(new SdrHdl(aRect.TopCenter()   ,HDL_UPPER));
                        if (!bWdt0 && !bHgt0) aHdl.AddHdl(new SdrHdl(aRect.TopRight()    ,HDL_UPRGT));
                        if (!bWdt0          ) aHdl.AddHdl(new SdrHdl(aRect.LeftCenter()  ,HDL_LEFT ));
                        if (!bWdt0          ) aHdl.AddHdl(new SdrHdl(aRect.RightCenter() ,HDL_RIGHT));
                        if (!bWdt0 && !bHgt0) aHdl.AddHdl(new SdrHdl(aRect.BottomLeft()  ,HDL_LWLFT));
                        if (          !bHgt0) aHdl.AddHdl(new SdrHdl(aRect.BottomCenter(),HDL_LOWER));
                        if (!bWdt0 && !bHgt0) aHdl.AddHdl(new SdrHdl(aRect.BottomRight() ,HDL_LWRGT));
                    }
                }
            }
        }
        else
        {
            for (sal_uIntPtr nMarkNum=0; nMarkNum<nMarkAnz; nMarkNum++)
            {
                const SdrMark* pM=GetSdrMarkByIndex(nMarkNum);
                SdrObject* pObj=pM->GetMarkedSdrObj();
                SdrPageView* pPV=pM->GetPageView();
                const sal_uIntPtr nSiz0=aHdl.GetHdlCount();
                pObj->AddToHdlList(aHdl);
                const sal_uIntPtr nSiz1=aHdl.GetHdlCount();
                bool bPoly=pObj->IsPolyObj();
                const SdrUShortCont* pMrkPnts=pM->GetMarkedPoints();
                for (sal_uIntPtr i=nSiz0; i<nSiz1; i++)
                {
                    SdrHdl* pHdl=aHdl.GetHdl(i);
                    pHdl->SetObj(pObj);
                    pHdl->SetPageView(pPV);
                    pHdl->SetObjHdlNum(sal_uInt16(i-nSiz0));
                    if (bPoly)
                    {
                        sal_Bool bSelected=pMrkPnts!=NULL && pMrkPnts->Exist(sal_uInt16(i-nSiz0));
                        pHdl->SetSelected(bSelected);
                        //sal_Bool bPlus=bPlusHdlAlways;
                        if (bPlusHdlAlways || bSelected)
                        {
                            sal_uInt32 nPlusAnz=pObj->GetPlusHdlCount(*pHdl);
                            for (sal_uInt32 nPlusNum=0; nPlusNum<nPlusAnz; nPlusNum++)
                            {
                                SdrHdl* pPlusHdl=pObj->GetPlusHdl(*pHdl,nPlusNum);
                                if (pPlusHdl!=NULL)
                                {
                                    pPlusHdl->SetObj(pObj);
                                    pPlusHdl->SetPageView(pPV);
                                    pPlusHdl->SetPlusHdl(sal_True);
                                    aHdl.AddHdl(pPlusHdl);
                                }
                            }
                        }
                    }
                }
            } // for nMarkNum
        } // if bFrmHdl else

        // GluePoint-Handles
        for (sal_uIntPtr nMarkNum=0; nMarkNum<nMarkAnz; nMarkNum++)
        {
            const SdrMark* pM=GetSdrMarkByIndex(nMarkNum);
            SdrObject* pObj=pM->GetMarkedSdrObj();
            SdrPageView* pPV=pM->GetPageView();
            const SdrUShortCont* pMrkGlue=pM->GetMarkedGluePoints();
            if (pMrkGlue!=NULL)
            {
                const SdrGluePointList* pGPL=pObj->GetGluePointList();
                if (pGPL!=NULL)
                {
                    //sal_uInt16 nGlueAnz=pGPL->GetCount();
                    sal_uInt16 nAnz=(sal_uInt16)pMrkGlue->GetCount();
                    for (sal_uInt16 nNum=0; nNum<nAnz; nNum++)
                    {
                        sal_uInt16 nId=pMrkGlue->GetObject(nNum);
                        //nNum changed to nNumGP because already used in for loop
                        sal_uInt16 nNumGP=pGPL->FindGluePoint(nId);
                        if (nNumGP!=SDRGLUEPOINT_NOTFOUND)
                        {
                            const SdrGluePoint& rGP=(*pGPL)[nNumGP];
                            Point aPos(rGP.GetAbsolutePos(*pObj));
                            SdrHdl* pGlueHdl=new SdrHdl(aPos,HDL_GLUE);
                            pGlueHdl->SetObj(pObj);
                            pGlueHdl->SetPageView(pPV);
                            pGlueHdl->SetObjHdlNum(nId);
                            aHdl.AddHdl(pGlueHdl);
                        }
                    }
                }
            }
        }

        // Drehpunkt/Spiegelachse
        AddDragModeHdl(eDragMode);

        // add custom handles (used by other apps, e.g. AnchorPos)
        AddCustomHdl();

        // sort handles
        aHdl.Sort();

        // #105722# try to restore focus handle index from remembered values
        if(bSaveOldFocus)
        {
            for(sal_uInt32 a(0); a < aHdl.GetHdlCount(); a++)
            {
                SdrHdl* pCandidate = aHdl.GetHdl(a);

                if(pCandidate->GetObj()
                    && pCandidate->GetObj() == pSaveObj
                    && pCandidate->GetKind() == eSaveKind
                    && pCandidate->GetPolyNum() == nSavePolyNum
                    && pCandidate->GetPointNum() == nSavePointNum)
                {
                    aHdl.SetFocusHdl(pCandidate);
                    break;
                }
            }
        }
    }
}

void SdrMarkView::AddCustomHdl()
{
    // add custom handles (used by other apps, e.g. AnchorPos)
}

void SdrMarkView::SetDragMode(SdrDragMode eMode)
{
    SdrDragMode eMode0=eDragMode;
    eDragMode=eMode;
    if (eDragMode==SDRDRAG_RESIZE) eDragMode=SDRDRAG_MOVE;
    if (eDragMode!=eMode0) {
        //HMHBOOL bVis=IsMarkHdlShown();
        //HMHif (bVis) HideMarkHdl();
        ForceRefToMarked();
        SetMarkHandles();
        //HMHif (bVis) ShowMarkHdl();
        {
            if (AreObjectsMarked()) MarkListHasChanged();
        }
    }
}

void SdrMarkView::AddDragModeHdl(SdrDragMode eMode)
{
    switch(eMode)
    {
        case SDRDRAG_ROTATE:
        {
            // add rotation center
            SdrHdl* pHdl = new SdrHdl(aRef1, HDL_REF1);

            aHdl.AddHdl(pHdl);

            break;
        }
        case SDRDRAG_MIRROR:
        {
            // add mirror axis
            SdrHdl* pHdl3 = new SdrHdl(aRef2, HDL_REF2);
            SdrHdl* pHdl2 = new SdrHdl(aRef1, HDL_REF1);
            SdrHdl* pHdl1 = new SdrHdlLine(*pHdl2, *pHdl3, HDL_MIRX);

            pHdl1->SetObjHdlNum(1); // fuer Sortierung
            pHdl2->SetObjHdlNum(2); // fuer Sortierung
            pHdl3->SetObjHdlNum(3); // fuer Sortierung

            aHdl.AddHdl(pHdl1); // Linie als erstes, damit als letztes im HitTest
            aHdl.AddHdl(pHdl2);
            aHdl.AddHdl(pHdl3);

            break;
        }
        case SDRDRAG_TRANSPARENCE:
        {
            // add interactive transparence handle
            sal_uIntPtr nMarkAnz = GetMarkedObjectCount();
            if(nMarkAnz == 1)
            {
                SdrObject* pObj = GetMarkedObjectByIndex(0);
                SdrModel* pModel = GetModel();
                const SfxItemSet& rSet = pObj->GetMergedItemSet();

                if(SFX_ITEM_SET != rSet.GetItemState(XATTR_FILLFLOATTRANSPARENCE, sal_False))
                {
                    // add this item, it's not yet there
                    XFillFloatTransparenceItem aNewItem(
                        (const XFillFloatTransparenceItem&)rSet.Get(XATTR_FILLFLOATTRANSPARENCE));
                    XGradient aGrad = aNewItem.GetGradientValue();

                    aNewItem.SetEnabled(sal_True);
                    aGrad.SetStartIntens(100);
                    aGrad.SetEndIntens(100);
                    aNewItem.SetGradientValue(aGrad);

                    // add undo to allow user to take back this step
                    if( pModel->IsUndoEnabled() )
                    {
                        pModel->BegUndo(SVX_RESSTR(SIP_XA_FILLTRANSPARENCE));
                        pModel->AddUndo(pModel->GetSdrUndoFactory().CreateUndoAttrObject(*pObj));
                        pModel->EndUndo();
                    }

                    //pObj->SetItemAndBroadcast(aNewItem);
                    SfxItemSet aNewSet(pModel->GetItemPool());
                    aNewSet.Put(aNewItem);
                    pObj->SetMergedItemSetAndBroadcast(aNewSet);
                }

                // set values and transform to vector set
                GradTransformer aGradTransformer;
                GradTransVector aGradTransVector;
                GradTransGradient aGradTransGradient;

                aGradTransGradient.aGradient = ((XFillFloatTransparenceItem&)rSet.Get(XATTR_FILLFLOATTRANSPARENCE)).GetGradientValue();
                aGradTransformer.GradToVec(aGradTransGradient, aGradTransVector, pObj);

                // build handles
                const Point aTmpPos1(basegfx::fround(aGradTransVector.maPositionA.getX()), basegfx::fround(aGradTransVector.maPositionA.getY()));
                const Point aTmpPos2(basegfx::fround(aGradTransVector.maPositionB.getX()), basegfx::fround(aGradTransVector.maPositionB.getY()));
                SdrHdlColor* pColHdl1 = new SdrHdlColor(aTmpPos1, aGradTransVector.aCol1, SDR_HANDLE_COLOR_SIZE_NORMAL, sal_True);
                SdrHdlColor* pColHdl2 = new SdrHdlColor(aTmpPos2, aGradTransVector.aCol2, SDR_HANDLE_COLOR_SIZE_NORMAL, sal_True);
                SdrHdlGradient* pGradHdl = new SdrHdlGradient(aTmpPos1, aTmpPos2, sal_False);
                DBG_ASSERT(pColHdl1 && pColHdl2 && pGradHdl, "Got not all necessary handles!!");

                // link them
                pGradHdl->SetColorHandles(pColHdl1, pColHdl2);
                pGradHdl->SetObj(pObj);
                pColHdl1->SetColorChangeHdl(LINK(pGradHdl, SdrHdlGradient, ColorChangeHdl));
                pColHdl2->SetColorChangeHdl(LINK(pGradHdl, SdrHdlGradient, ColorChangeHdl));

                // insert them
                aHdl.AddHdl(pColHdl1);
                aHdl.AddHdl(pColHdl2);
                aHdl.AddHdl(pGradHdl);
            }
            break;
        }
        case SDRDRAG_GRADIENT:
        {
            // add interactive gradient handle
            sal_uIntPtr nMarkAnz = GetMarkedObjectCount();
            if(nMarkAnz == 1)
            {
                SdrObject* pObj = GetMarkedObjectByIndex(0);
                const SfxItemSet& rSet = pObj->GetMergedItemSet();
                XFillStyle eFillStyle = ((XFillStyleItem&)(rSet.Get(XATTR_FILLSTYLE))).GetValue();

                if(eFillStyle == XFILL_GRADIENT)
                {
                    // set values and transform to vector set
                    GradTransformer aGradTransformer;
                    GradTransVector aGradTransVector;
                    GradTransGradient aGradTransGradient;
                    Size aHdlSize(15, 15);

                    aGradTransGradient.aGradient = ((XFillGradientItem&)rSet.Get(XATTR_FILLGRADIENT)).GetGradientValue();
                    aGradTransformer.GradToVec(aGradTransGradient, aGradTransVector, pObj);

                    // build handles
                    const Point aTmpPos1(basegfx::fround(aGradTransVector.maPositionA.getX()), basegfx::fround(aGradTransVector.maPositionA.getY()));
                    const Point aTmpPos2(basegfx::fround(aGradTransVector.maPositionB.getX()), basegfx::fround(aGradTransVector.maPositionB.getY()));
                    SdrHdlColor* pColHdl1 = new SdrHdlColor(aTmpPos1, aGradTransVector.aCol1, aHdlSize, sal_False);
                    SdrHdlColor* pColHdl2 = new SdrHdlColor(aTmpPos2, aGradTransVector.aCol2, aHdlSize, sal_False);
                    SdrHdlGradient* pGradHdl = new SdrHdlGradient(aTmpPos1, aTmpPos2, sal_True);
                    DBG_ASSERT(pColHdl1 && pColHdl2 && pGradHdl, "Got not all necessary handles!!");

                    // link them
                    pGradHdl->SetColorHandles(pColHdl1, pColHdl2);
                    pGradHdl->SetObj(pObj);
                    pColHdl1->SetColorChangeHdl(LINK(pGradHdl, SdrHdlGradient, ColorChangeHdl));
                    pColHdl2->SetColorChangeHdl(LINK(pGradHdl, SdrHdlGradient, ColorChangeHdl));

                    // insert them
                    aHdl.AddHdl(pColHdl1);
                    aHdl.AddHdl(pColHdl2);
                    aHdl.AddHdl(pGradHdl);
                }
            }
            break;
        }
        case SDRDRAG_CROP:
        {
            // todo
            break;
        }
        default: break;
    }
}

/** handle mouse over effects for handles */
sal_Bool SdrMarkView::MouseMove(const MouseEvent& rMEvt, Window* pWin)
{
    if(aHdl.GetHdlCount())
    {
        SdrHdl* pMouseOverHdl = 0;
        if( !rMEvt.IsLeaveWindow() && pWin )
        {
            Point aMDPos( pWin->PixelToLogic( rMEvt.GetPosPixel() ) );
            pMouseOverHdl = PickHandle(aMDPos);
        }

        // notify last mouse over handle that he lost the mouse
        const sal_uIntPtr nHdlCount = aHdl.GetHdlCount();

        for(sal_uIntPtr nHdl = 0; nHdl < nHdlCount; nHdl++ )
        {
            SdrHdl* pCurrentHdl = GetHdl(nHdl);
            if( pCurrentHdl->mbMouseOver )
            {
                if( pCurrentHdl != pMouseOverHdl )
                {
                    pCurrentHdl->mbMouseOver = false;
                    pCurrentHdl->onMouseLeave();
                }
                break;
            }
        }

        // notify current mouse over handle
        if( pMouseOverHdl /* && !pMouseOverHdl->mbMouseOver */ )
        {
            pMouseOverHdl->mbMouseOver = true;
            pMouseOverHdl->onMouseEnter(rMEvt);
        }
    }
    return SdrSnapView::MouseMove(rMEvt, pWin);
}

void SdrMarkView::ForceRefToMarked()
{
    switch(eDragMode)
    {
        case SDRDRAG_ROTATE:
        {
            Rectangle aR(GetMarkedObjRect());
            aRef1 = aR.Center();

            break;
        }

        case SDRDRAG_MIRROR:
        {
            // Erstmal die laenge der Spiegelachsenlinie berechnen
            long nOutMin=0;
            long nOutMax=0;
            long nMinLen=0;
            long nObjDst=0;
            long nOutHgt=0;
            OutputDevice* pOut=GetFirstOutputDevice();
            //OutputDevice* pOut=GetWin(0);
            if (pOut!=NULL) {
                // Mindestlaenge 50 Pixel
                nMinLen=pOut->PixelToLogic(Size(0,50)).Height();
                // 20 Pixel fuer RefPt-Abstand vom Obj
                nObjDst=pOut->PixelToLogic(Size(0,20)).Height();
                // MinY/MaxY
                // Abstand zum Rand = Mindestlaenge = 10 Pixel
                long nDst=pOut->PixelToLogic(Size(0,10)).Height();
                nOutMin=-pOut->GetMapMode().GetOrigin().Y();
                nOutMax=pOut->GetOutputSize().Height()-1+nOutMin;
                nOutMin+=nDst;
                nOutMax-=nDst;
                // Absolute Mindestlaenge jedoch 10 Pixel
                if (nOutMax-nOutMin<nDst) {
                    nOutMin+=nOutMax+1;
                    nOutMin/=2;
                    nOutMin-=(nDst+1)/2;
                    nOutMax=nOutMin+nDst;
                }
                nOutHgt=nOutMax-nOutMin;
                // Sonst Mindestlaenge = 1/4 OutHgt
                long nTemp=nOutHgt/4;
                if (nTemp>nMinLen) nMinLen=nTemp;
            }

            Rectangle aR(GetMarkedObjBoundRect());
            Point aCenter(aR.Center());
            long nMarkHgt=aR.GetHeight()-1;
            long nHgt=nMarkHgt+nObjDst*2;       // 20 Pixel obej und unten ueberstehend
            if (nHgt<nMinLen) nHgt=nMinLen;     // Mindestlaenge 50 Pixel bzw. 1/4 OutHgt

            long nY1=aCenter.Y()-(nHgt+1)/2;
            long nY2=nY1+nHgt;

            if (pOut!=NULL && nMinLen>nOutHgt) nMinLen=nOutHgt; // evtl. noch etwas verkuerzen

            if (pOut!=NULL) { // nun vollstaendig in den sichtbaren Bereich schieben
                if (nY1<nOutMin) {
                    nY1=nOutMin;
                    if (nY2<nY1+nMinLen) nY2=nY1+nMinLen;
                }
                if (nY2>nOutMax) {
                    nY2=nOutMax;
                    if (nY1>nY2-nMinLen) nY1=nY2-nMinLen;
                }
            }

            aRef1.X()=aCenter.X();
            aRef1.Y()=nY1;
            aRef2.X()=aCenter.X();
            aRef2.Y()=nY2;

            break;
        }

        case SDRDRAG_TRANSPARENCE:
        case SDRDRAG_GRADIENT:
        case SDRDRAG_CROP:
        {
            Rectangle aRect(GetMarkedObjBoundRect());
            aRef1 = aRect.TopLeft();
            aRef2 = aRect.BottomRight();
            break;
        }
        default: break;
    }
}

void SdrMarkView::SetRef1(const Point& rPt)
{
    if(eDragMode == SDRDRAG_ROTATE || eDragMode == SDRDRAG_MIRROR)
    {
        aRef1 = rPt;
        SdrHdl* pH = aHdl.GetHdl(HDL_REF1);
        if(pH)
            pH->SetPos(rPt);
        //HMHShowMarkHdl();
    }
}

void SdrMarkView::SetRef2(const Point& rPt)
{
    if(eDragMode == SDRDRAG_MIRROR)
    {
        aRef2 = rPt;
        SdrHdl* pH = aHdl.GetHdl(HDL_REF2);
        if(pH)
            pH->SetPos(rPt);
        //HMHShowMarkHdl();
    }
}

void SdrMarkView::CheckMarked()
{
    for (sal_uIntPtr nm=GetMarkedObjectCount(); nm>0;) {
        nm--;
        SdrMark* pM=GetSdrMarkByIndex(nm);
        SdrObject* pObj=pM->GetMarkedSdrObj();
        SdrPageView* pPV=pM->GetPageView();
        SdrLayerID nLay=pObj->GetLayer();
        sal_Bool bRaus=!pObj->IsInserted(); // Obj geloescht?
        if (!pObj->Is3DObj()) {
            bRaus=bRaus || pObj->GetPage()!=pPV->GetPage();   // Obj ploetzlich in anderer Page oder Group
        }
        bRaus=bRaus || pPV->GetLockedLayers().IsSet(nLay) ||  // Layer gesperrt?
                       !pPV->GetVisibleLayers().IsSet(nLay);  // Layer nicht sichtbar?

        if( !bRaus )
            bRaus = !pObj->IsVisible(); // not visible objects can not be marked

        if (!bRaus) {
            // Joe am 9.3.1997: Gruppierte Objekten koennen nun auch
            // markiert werden. Nach EnterGroup muessen aber die Objekte
            // der hoeheren Ebene deselektiert werden.
            const SdrObjList* pOOL=pObj->GetObjList();
            const SdrObjList* pVOL=pPV->GetObjList();
            while (pOOL!=NULL && pOOL!=pVOL) {
                pOOL=pOOL->GetUpList();
            }
            bRaus=pOOL!=pVOL;
        }

        if (bRaus)
        {
            GetMarkedObjectListWriteAccess().DeleteMark(nm);
        }
        else
        {
            if (!IsGluePointEditMode()) { // Markierte GluePoints nur im GlueEditMode
                SdrUShortCont* pPts=pM->GetMarkedGluePoints();
                if (pPts!=NULL && pPts->GetCount()!=0) {
                    pPts->Clear();
                }
            }
        }
    }

    // #97995# at least reset the remembered BoundRect to prevent handle
    // generation if bForceFrameHandles is TRUE.
    bMarkedObjRectDirty = sal_True;
}

void SdrMarkView::SetMarkRects()
{
    SdrPageView* pPV = GetSdrPageView();

    if(pPV)
    {
        pPV->SetHasMarkedObj(GetSnapRectFromMarkedObjects(pPV, pPV->MarkSnap()));
        GetBoundRectFromMarkedObjects(pPV, pPV->MarkBound());
    }
}

void SdrMarkView::SetFrameHandles(sal_Bool bOn)
{
    if (bOn!=bForceFrameHandles) {
        sal_Bool bOld=ImpIsFrameHandles();
        bForceFrameHandles=bOn;
        sal_Bool bNew=ImpIsFrameHandles();
        if (bNew!=bOld) {
            AdjustMarkHdl(); //HMHTRUE);
            MarkListHasChanged();
        }
    }
}

void SdrMarkView::SetEditMode(SdrViewEditMode eMode)
{
    if (eMode!=eEditMode) {
        sal_Bool bGlue0=eEditMode==SDREDITMODE_GLUEPOINTEDIT;
        sal_Bool bEdge0=((SdrCreateView*)this)->IsEdgeTool();
        eEditMode0=eEditMode;
        eEditMode=eMode;
        sal_Bool bGlue1=eEditMode==SDREDITMODE_GLUEPOINTEDIT;
        sal_Bool bEdge1=((SdrCreateView*)this)->IsEdgeTool();
        // etwas Aufwand um Flackern zu verhindern beim Umschalten
        // zwischen GlueEdit und EdgeTool
        if (bGlue1 && !bGlue0) ImpSetGlueVisible2(bGlue1);
        if (bEdge1!=bEdge0) ImpSetGlueVisible3(bEdge1);
        if (!bGlue1 && bGlue0) ImpSetGlueVisible2(bGlue1);
        if (bGlue0 && !bGlue1) UnmarkAllGluePoints();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

sal_Bool SdrMarkView::IsObjMarkable(SdrObject* pObj, SdrPageView* pPV) const
{
    if (pObj)
    {
        if (pObj->IsMarkProtect() ||
            (!bDesignMode && pObj->IsUnoObj()))
        {
            // Objekt nicht selektierbar oder
            // SdrUnoObj nicht im DesignMode
            return sal_False;
        }
    }
    return pPV!=NULL ? pPV->IsObjMarkable(pObj) : sal_True;
}

sal_Bool SdrMarkView::IsMarkedObjHit(const Point& rPnt, short nTol) const
{
    sal_Bool bRet=sal_False;
    nTol=ImpGetHitTolLogic(nTol,NULL);
    Point aPt(rPnt);
    for (sal_uIntPtr nm=0; nm<GetMarkedObjectCount() && !bRet; nm++) {
        SdrMark* pM=GetSdrMarkByIndex(nm);
        bRet = 0 != CheckSingleSdrObjectHit(aPt,sal_uInt16(nTol),pM->GetMarkedSdrObj(),pM->GetPageView(),0,0);
    }
    return bRet;
}

SdrHdl* SdrMarkView::PickHandle(const Point& rPnt, sal_uIntPtr nOptions, SdrHdl* pHdl0) const
{
    if (bSomeObjChgdFlag) { // ggf. Handles neu berechnen lassen!
        FlushComeBackTimer();
    }
    sal_Bool bBack=(nOptions & SDRSEARCH_BACKWARD) !=0;
    sal_Bool bNext=(nOptions & SDRSEARCH_NEXT) !=0;
    Point aPt(rPnt);
    return aHdl.IsHdlListHit(aPt,bBack,bNext,pHdl0);
}

sal_Bool SdrMarkView::MarkObj(const Point& rPnt, short nTol, sal_Bool bToggle, sal_Bool bDeep)
{
    SdrObject* pObj;
    SdrPageView* pPV;
    nTol=ImpGetHitTolLogic(nTol,NULL);
    sal_uIntPtr nOptions=SDRSEARCH_PICKMARKABLE;
    if (bDeep) nOptions=nOptions|SDRSEARCH_DEEP;
    sal_Bool bRet=PickObj(rPnt,(sal_uInt16)nTol,pObj,pPV,nOptions);
    if (bRet) {
        sal_Bool bUnmark=bToggle && IsObjMarked(pObj);
        MarkObj(pObj,pPV,bUnmark);
    }
    return bRet;
}

sal_Bool SdrMarkView::MarkNextObj(sal_Bool bPrev)
{
    SdrPageView* pPageView = GetSdrPageView();

    if(!pPageView)
    {
        return sal_False;
    }

    SortMarkedObjects();
    sal_uIntPtr  nMarkAnz=GetMarkedObjectCount();
    sal_uIntPtr  nChgMarkNum = ULONG_MAX; // Nummer des zu ersetzenden MarkEntries
    sal_uIntPtr  nSearchObjNum = bPrev ? 0 : ULONG_MAX;
    if (nMarkAnz!=0) {
        nChgMarkNum=bPrev ? 0 : sal_uIntPtr(nMarkAnz-1);
        SdrMark* pM=GetSdrMarkByIndex(nChgMarkNum);
        OSL_ASSERT(pM!=NULL);
        if (pM->GetMarkedSdrObj() != NULL)
            nSearchObjNum = pM->GetMarkedSdrObj()->GetNavigationPosition();
    }

    SdrObject* pMarkObj=NULL;
    SdrObjList* pSearchObjList=pPageView->GetObjList();
    sal_uIntPtr nObjAnz=pSearchObjList->GetObjCount();
    if (nObjAnz!=0) {
        if (nSearchObjNum>nObjAnz) nSearchObjNum=nObjAnz;
        while (pMarkObj==NULL && ((!bPrev && nSearchObjNum>0) || (bPrev && nSearchObjNum<nObjAnz)))
        {
            if (!bPrev)
                nSearchObjNum--;
            SdrObject* pSearchObj = pSearchObjList->GetObjectForNavigationPosition(nSearchObjNum);
            if (IsObjMarkable(pSearchObj,pPageView))
            {
                if (TryToFindMarkedObject(pSearchObj)==CONTAINER_ENTRY_NOTFOUND)
                {
                    pMarkObj=pSearchObj;
                }
            }
            if (bPrev) nSearchObjNum++;
        }
    }

    if(!pMarkObj)
    {
        return sal_False;
    }

    if (nChgMarkNum!=ULONG_MAX)
    {
        GetMarkedObjectListWriteAccess().DeleteMark(nChgMarkNum);
    }
    MarkObj(pMarkObj,pPageView); // ruft auch MarkListHasChanged(), AdjustMarkHdl()
    return sal_True;
}

sal_Bool SdrMarkView::MarkNextObj(const Point& rPnt, short nTol, sal_Bool bPrev)
{
    SortMarkedObjects();
    nTol=ImpGetHitTolLogic(nTol,NULL);
    Point aPt(rPnt);
    SdrMark* pTopMarkHit=NULL;
    SdrMark* pBtmMarkHit=NULL;
    sal_uIntPtr nTopMarkHit=0;
    sal_uIntPtr nBtmMarkHit=0;
    // oberstes der markierten Objekte suchen, das von rPnt getroffen wird
    sal_uIntPtr nMarkAnz=GetMarkedObjectCount();
    sal_uIntPtr nm=0;
    for (nm=nMarkAnz; nm>0 && pTopMarkHit==NULL;) {
        nm--;
        SdrMark* pM=GetSdrMarkByIndex(nm);
        if(CheckSingleSdrObjectHit(aPt,sal_uInt16(nTol),pM->GetMarkedSdrObj(),pM->GetPageView(),0,0))
        {
            pTopMarkHit=pM;
            nTopMarkHit=nm;
        }
    }
    // Nichts gefunden, dann ganz normal ein Obj markieren.
    if (pTopMarkHit==NULL) return MarkObj(rPnt,sal_uInt16(nTol),sal_False);

    SdrObject* pTopObjHit=pTopMarkHit->GetMarkedSdrObj();
    SdrObjList* pObjList=pTopObjHit->GetObjList();
    SdrPageView* pPV=pTopMarkHit->GetPageView();
    // unterstes der markierten Objekte suchen, das von rPnt getroffen wird
    // und auf der gleichen PageView liegt wie pTopMarkHit
    for (nm=0; nm<nMarkAnz && pBtmMarkHit==NULL; nm++) {
        SdrMark* pM=GetSdrMarkByIndex(nm);
        SdrPageView* pPV2=pM->GetPageView();
        if (pPV2==pPV && CheckSingleSdrObjectHit(aPt,sal_uInt16(nTol),pM->GetMarkedSdrObj(),pPV2,0,0))
        {
            pBtmMarkHit=pM;
            nBtmMarkHit=nm;
        }
    }
    if (pBtmMarkHit==NULL) { pBtmMarkHit=pTopMarkHit; nBtmMarkHit=nTopMarkHit; }
    SdrObject* pBtmObjHit=pBtmMarkHit->GetMarkedSdrObj();
    sal_uIntPtr nObjAnz=pObjList->GetObjCount();

    // #110988#
    //sal_uIntPtr nSearchBeg=bPrev ? pBtmObjHit->GetOrdNum()+1 : pTopObjHit->GetOrdNum();
    sal_uInt32 nSearchBeg;
    E3dScene* pScene = NULL;
    SdrObject* pObjHit = (bPrev) ? pBtmObjHit : pTopObjHit;
    sal_Bool bRemap = pObjHit->ISA(E3dCompoundObject)
        ? ((E3dCompoundObject*)pObjHit)->IsAOrdNumRemapCandidate(pScene)
        : sal_False;

    if(bPrev)
    {
        sal_uInt32 nOrdNumBtm(pBtmObjHit->GetOrdNum());

        if(bRemap)
        {
            nOrdNumBtm = pScene->RemapOrdNum(nOrdNumBtm);
        }

        nSearchBeg = nOrdNumBtm + 1;
    }
    else
    {
        sal_uInt32 nOrdNumTop(pTopObjHit->GetOrdNum());

        if(bRemap)
        {
            nOrdNumTop = pScene->RemapOrdNum(nOrdNumTop);
        }

        nSearchBeg = nOrdNumTop;
    }

    sal_uIntPtr no=nSearchBeg;
    SdrObject* pFndObj=NULL;
    //SdrObject* pAktObj=NULL;
    while (pFndObj==NULL && ((!bPrev && no>0) || (bPrev && no<nObjAnz))) {
        if (!bPrev) no--;
        SdrObject* pObj;

        if(bRemap)
        {
            pObj = pObjList->GetObj(pScene->RemapOrdNum(no));
        }
        else
        {
            pObj = pObjList->GetObj(no);
        }

        if (CheckSingleSdrObjectHit(aPt,sal_uInt16(nTol),pObj,pPV,SDRSEARCH_TESTMARKABLE,0))
        {
            if (TryToFindMarkedObject(pObj)==CONTAINER_ENTRY_NOTFOUND) {
                pFndObj=pObj;
            } else {
                // hier wg. Performance ggf. noch no auf Top bzw. auf Btm stellen
            }
        }
        if (bPrev) no++;
    }
    if (pFndObj!=NULL)
    {
        GetMarkedObjectListWriteAccess().DeleteMark(bPrev?nBtmMarkHit:nTopMarkHit);
        GetMarkedObjectListWriteAccess().InsertEntry(SdrMark(pFndObj,pPV));
        MarkListHasChanged();
        AdjustMarkHdl(); //HMHTRUE);
    }
    return pFndObj!=NULL;
}

sal_Bool SdrMarkView::MarkObj(const Rectangle& rRect, sal_Bool bUnmark)
{
    sal_Bool bFnd=sal_False;
    Rectangle aR(rRect);
    SdrObject* pObj;
    SdrObjList* pObjList;
    BrkAction();
    SdrPageView* pPV = GetSdrPageView();

    if(pPV)
    {
        pObjList=pPV->GetObjList();
        Rectangle aFrm1(aR);
        sal_uIntPtr nObjAnz=pObjList->GetObjCount();
        for (sal_uIntPtr nO=0; nO<nObjAnz; nO++) {
            pObj=pObjList->GetObj(nO);
            Rectangle aRect(pObj->GetCurrentBoundRect());
            if (aFrm1.IsInside(aRect)) {
                if (!bUnmark) {
                    if (IsObjMarkable(pObj,pPV))
                    {
                        GetMarkedObjectListWriteAccess().InsertEntry(SdrMark(pObj,pPV));
                        bFnd=sal_True;
                    }
                } else {
                    sal_uIntPtr nPos=TryToFindMarkedObject(pObj);
                    if (nPos!=CONTAINER_ENTRY_NOTFOUND)
                    {
                        GetMarkedObjectListWriteAccess().DeleteMark(nPos);
                        bFnd=sal_True;
                    }
                }
            }
        }
    }
    if (bFnd) {
        SortMarkedObjects();
        MarkListHasChanged();
        AdjustMarkHdl(); //HMHTRUE);
        //HMHShowMarkHdl();
    }
    return bFnd;
}

void SdrMarkView::MarkObj(SdrObject* pObj, SdrPageView* pPV, sal_Bool bUnmark, sal_Bool bImpNoSetMarkHdl)
{
    if (pObj!=NULL && pPV!=NULL && IsObjMarkable(pObj, pPV)) {
        BrkAction();
        if (!bUnmark)
        {
            GetMarkedObjectListWriteAccess().InsertEntry(SdrMark(pObj,pPV));
        }
        else
        {
            sal_uIntPtr nPos=TryToFindMarkedObject(pObj);
            if (nPos!=CONTAINER_ENTRY_NOTFOUND)
            {
                GetMarkedObjectListWriteAccess().DeleteMark(nPos);
            }
        }
        if (!bImpNoSetMarkHdl) {
            MarkListHasChanged();
            AdjustMarkHdl(); //HMHTRUE);
            //HMHif (!bSomeObjChgdFlag) {
                // ShowMarkHdl kommt sonst mit dem AfterPaintTimer
                //HMHShowMarkHdl();
            //HMH}
        }
    }
}

sal_Bool SdrMarkView::IsObjMarked(SdrObject* pObj) const
{
    // nicht so ganz die feine Art: Da FindObject() nicht const ist
    // muss ich mich hier auf non-const casten.
    sal_uIntPtr nPos=((SdrMarkView*)this)->TryToFindMarkedObject(pObj);
    return nPos!=CONTAINER_ENTRY_NOTFOUND;
}

sal_uInt16 SdrMarkView::GetMarkHdlSizePixel() const
{
    return aHdl.GetHdlSize()*2+1;
}

void SdrMarkView::SetSolidMarkHdl(sal_Bool bOn)
{
    if (bOn!=aHdl.IsFineHdl()) {
        //HMHBOOL bMerk=IsMarkHdlShown();
        //HMHif (bMerk) HideMarkHdl();
        aHdl.SetFineHdl(bOn);
        //HMHif (bMerk) ShowMarkHdl();
    }
}

void SdrMarkView::SetMarkHdlSizePixel(sal_uInt16 nSiz)
{
    if (nSiz<3) nSiz=3;
    nSiz/=2;
    if (nSiz!=aHdl.GetHdlSize()) {
        //HMHBOOL bMerk=IsMarkHdlShown();
        //HMHif (bMerk) HideMarkHdl();
        aHdl.SetHdlSize(nSiz);
        //HMHif (bMerk) ShowMarkHdl();
    }
}

#define SDRSEARCH_IMPISMASTER 0x80000000 /* MasterPage wird gerade durchsucht */
SdrObject* SdrMarkView::CheckSingleSdrObjectHit(const Point& rPnt, sal_uInt16 nTol, SdrObject* pObj, SdrPageView* pPV, sal_uIntPtr nOptions, const SetOfByte* pMVisLay) const
{
    if(((nOptions & SDRSEARCH_IMPISMASTER) && pObj->IsNotVisibleAsMaster()) || (!pObj->IsVisible()))
    {
        return NULL;
    }

    const bool bCheckIfMarkable(nOptions & SDRSEARCH_TESTMARKABLE);
    const bool bDeep(nOptions & SDRSEARCH_DEEP);
    const bool bOLE(pObj->ISA(SdrOle2Obj));
    const bool bTXT(pObj->ISA(SdrTextObj) && ((SdrTextObj*)pObj)->IsTextFrame());
    SdrObject* pRet=NULL;
    Rectangle aRect(pObj->GetCurrentBoundRect());
    sal_uInt16 nTol2(nTol);

    // double tolerance for OLE, text frames and objects in
    // active text edit
    if(bOLE || bTXT || pObj==((SdrObjEditView*)this)->GetTextEditObject())
    {
        nTol2*=2;
    }

    aRect.Left  ()-=nTol2; // Einmal Toleranz drauf fuer alle Objekte
    aRect.Top   ()-=nTol2;
    aRect.Right ()+=nTol2;
    aRect.Bottom()+=nTol2;

    if (aRect.IsInside(rPnt))
    {
        if ((!bCheckIfMarkable || IsObjMarkable(pObj,pPV)))
        {
            SdrObjList* pOL=pObj->GetSubList();

            if (pOL!=NULL && pOL->GetObjCount()!=0)
            {
                SdrObject* pTmpObj;
                // OD 30.06.2003 #108784# - adjustment hit point for virtual
                // objects.
                Point aPnt( rPnt );

                if ( pObj->ISA(SdrVirtObj) )
                {
                    Point aOffset = static_cast<SdrVirtObj*>(pObj)->GetOffset();
                    aPnt.Move( -aOffset.X(), -aOffset.Y() );
                }

                pRet=CheckSingleSdrObjectHit(aPnt,nTol,pOL,pPV,nOptions,pMVisLay,pTmpObj);
            }
            else
            {
                if(!pMVisLay || pMVisLay->IsSet(pObj->GetLayer()))
                {
                    pRet = SdrObjectPrimitiveHit(*pObj, rPnt, nTol2, *pPV, &pPV->GetVisibleLayers(), false);
                }
            }
        }
    }

    if (!bDeep && pRet!=NULL)
    {
        pRet=pObj;
    }

    return pRet;
}

SdrObject* SdrMarkView::CheckSingleSdrObjectHit(const Point& rPnt, sal_uInt16 nTol, SdrObjList* pOL, SdrPageView* pPV, sal_uIntPtr nOptions, const SetOfByte* pMVisLay, SdrObject*& rpRootObj) const
{
    sal_Bool bBack=(nOptions & SDRSEARCH_BACKWARD)!=0;
    SdrObject* pRet=NULL;
    rpRootObj=NULL;
    if (pOL!=NULL)
    {
        // #110988#
        sal_Bool bRemap(pOL->GetOwnerObj() && pOL->GetOwnerObj()->ISA(E3dScene));
        E3dScene* pRemapScene = (bRemap ? (E3dScene*)pOL->GetOwnerObj() : 0L);

        sal_uIntPtr nObjAnz=pOL->GetObjCount();
        sal_uIntPtr nObjNum=bBack ? 0 : nObjAnz;
        while (pRet==NULL && (bBack ? nObjNum<nObjAnz : nObjNum>0)) {
            if (!bBack) nObjNum--;
            SdrObject* pObj;

            // #110988#
            if(bRemap)
            {
                pObj = pOL->GetObj(pRemapScene->RemapOrdNum(nObjNum));
            }
            else
            {
                pObj = pOL->GetObj(nObjNum);
            }

            pRet=CheckSingleSdrObjectHit(rPnt,nTol,pObj,pPV,nOptions,pMVisLay);
            if (pRet!=NULL) rpRootObj=pObj;
            if (bBack) nObjNum++;
        }
    }
    return pRet;
}

sal_Bool SdrMarkView::PickObj(const Point& rPnt, short nTol, SdrObject*& rpObj, SdrPageView*& rpPV, sal_uIntPtr nOptions) const
{
    return PickObj(rPnt,nTol,rpObj,rpPV,nOptions,NULL,NULL,NULL);
}

sal_Bool SdrMarkView::PickObj(const Point& rPnt, short nTol, SdrObject*& rpObj, SdrPageView*& rpPV, sal_uIntPtr nOptions, SdrObject** ppRootObj, sal_uIntPtr* pnMarkNum, sal_uInt16* pnPassNum) const
{ // Fehlt noch Pass2,Pass3
    SortMarkedObjects();
    if (ppRootObj!=NULL) *ppRootObj=NULL;
    if (pnMarkNum!=NULL) *pnMarkNum=CONTAINER_ENTRY_NOTFOUND;
    if (pnPassNum!=NULL) *pnPassNum=0;
    rpObj=NULL;
    rpPV=NULL;
    sal_Bool bWholePage=(nOptions & SDRSEARCH_WHOLEPAGE) !=0;
    sal_Bool bMarked=(nOptions & SDRSEARCH_MARKED) !=0;
    sal_Bool bMasters=!bMarked && (nOptions & SDRSEARCH_ALSOONMASTER) !=0;
    sal_Bool bBack=(nOptions & SDRSEARCH_BACKWARD) !=0;
#if OSL_DEBUG_LEVEL > 0
    sal_Bool bNext=(nOptions & SDRSEARCH_NEXT) !=0; (void)bNext; // n.i.
    sal_Bool bBoundCheckOn2ndPass=(nOptions & SDRSEARCH_PASS2BOUND) !=0; (void)bBoundCheckOn2ndPass;// n.i.
    sal_Bool bCheckNearestOn3rdPass=(nOptions & SDRSEARCH_PASS3NEAREST) !=0; (void)bCheckNearestOn3rdPass;// n.i.
#endif
    if (nTol<0) nTol=ImpGetHitTolLogic(nTol,NULL);
    Point aPt(rPnt);
    SdrObject* pObj=NULL;
    SdrObject* pHitObj=NULL;
    SdrPageView* pPV=NULL;
    if (!bBack && ((SdrObjEditView*)this)->IsTextEditFrameHit(rPnt)) {
        pObj=((SdrObjEditView*)this)->GetTextEditObject();
        pHitObj=pObj;
        pPV=((SdrObjEditView*)this)->GetTextEditPageView();
    }
    if (bMarked) {
        sal_uIntPtr nMrkAnz=GetMarkedObjectCount();
        sal_uIntPtr nMrkNum=bBack ? 0 : nMrkAnz;
        while (pHitObj==NULL && (bBack ? nMrkNum<nMrkAnz : nMrkNum>0)) {
            if (!bBack) nMrkNum--;
            SdrMark* pM=GetSdrMarkByIndex(nMrkNum);
            pObj=pM->GetMarkedSdrObj();
            pPV=pM->GetPageView();
            pHitObj=CheckSingleSdrObjectHit(aPt,nTol,pObj,pPV,nOptions,NULL);
            if (bBack) nMrkNum++;
        }
    }
    else
    {
        pPV = GetSdrPageView();

        if(pPV)
        {
            SdrPage* pPage=pPV->GetPage();
            sal_uInt16 nPgAnz=1;

            if(bMasters && pPage->TRG_HasMasterPage())
            {
                nPgAnz++;
            }

            sal_Bool bExtraPassForWholePage=bWholePage && pPage!=pPV->GetObjList();
            if (bExtraPassForWholePage) nPgAnz++; // Suche erst in AktObjList, dann auf der gesamten Page
            sal_uInt16 nPgNum=bBack ? 0 : nPgAnz;
            while (pHitObj==NULL && (bBack ? nPgNum<nPgAnz : nPgNum>0)) {
                sal_uIntPtr nTmpOptions=nOptions;
                if (!bBack) nPgNum--;
                const SetOfByte* pMVisLay=NULL;
                SdrObjList* pObjList=NULL;
                if (pnPassNum!=NULL) *pnPassNum&=~(SDRSEARCHPASS_MASTERPAGE|SDRSEARCHPASS_INACTIVELIST);
                if (nPgNum>=nPgAnz-1 || (bExtraPassForWholePage && nPgNum>=nPgAnz-2))
                {
                    pObjList=pPV->GetObjList();
                    if (bExtraPassForWholePage && nPgNum==nPgAnz-2) {
                        pObjList=pPage;
                        if (pnPassNum!=NULL) *pnPassNum|=SDRSEARCHPASS_INACTIVELIST;
                    }
                }
                else
                {
                    // sonst MasterPage
                    SdrPage& rMasterPage = pPage->TRG_GetMasterPage();
                    pMVisLay = &pPage->TRG_GetMasterPageVisibleLayers();
                    pObjList = &rMasterPage;

                    if (pnPassNum!=NULL) *pnPassNum|=SDRSEARCHPASS_MASTERPAGE;
                    nTmpOptions=nTmpOptions | SDRSEARCH_IMPISMASTER;
                }
                pHitObj=CheckSingleSdrObjectHit(aPt,nTol,pObjList,pPV,nTmpOptions,pMVisLay,pObj);
                if (bBack) nPgNum++;
            }
        }
    }
    if (pHitObj!=NULL) {
        if (ppRootObj!=NULL) *ppRootObj=pObj;
        if ((nOptions & SDRSEARCH_DEEP) !=0) pObj=pHitObj;
        if ((nOptions & SDRSEARCH_TESTTEXTEDIT) !=0) {
            if (!pObj->HasTextEdit() || pPV->GetLockedLayers().IsSet(pObj->GetLayer())) {
                pObj=NULL;
            }
        }
        if (pObj!=NULL && (nOptions & SDRSEARCH_TESTMACRO) !=0) {
            SdrObjMacroHitRec aHitRec;
            aHitRec.aPos=aPt;
            aHitRec.aDownPos=aPt;
            aHitRec.nTol=nTol;
            aHitRec.pVisiLayer=&pPV->GetVisibleLayers();
            aHitRec.pPageView=pPV;
            if (!pObj->HasMacro() || !pObj->IsMacroHit(aHitRec)) pObj=NULL;
        }
        if (pObj!=NULL && (nOptions & SDRSEARCH_WITHTEXT) !=0 && pObj->GetOutlinerParaObject()==NULL) pObj=NULL;
        if (pObj!=NULL && (nOptions & SDRSEARCH_TESTTEXTAREA) !=0)
        {
            if(!SdrObjectPrimitiveHit(*pObj, aPt, 0, *pPV, 0, true))
            {
                pObj = 0;
            }
        }
        if (pObj!=NULL) {
            rpObj=pObj;
            rpPV=pPV;
            if (pnPassNum!=NULL) *pnPassNum|=SDRSEARCHPASS_DIRECT;
        }
    }
    return rpObj!=NULL;
}

sal_Bool SdrMarkView::PickMarkedObj(const Point& rPnt, SdrObject*& rpObj, SdrPageView*& rpPV, sal_uIntPtr* pnMarkNum, sal_uIntPtr nOptions) const
{
    SortMarkedObjects();
    sal_Bool bBoundCheckOn2ndPass=(nOptions & SDRSEARCH_PASS2BOUND) !=0;
    sal_Bool bCheckNearestOn3rdPass=(nOptions & SDRSEARCH_PASS3NEAREST) !=0;
    rpObj=NULL;
    rpPV=NULL;
    if (pnMarkNum!=NULL) *pnMarkNum=CONTAINER_ENTRY_NOTFOUND;
    Point aPt(rPnt);
    sal_uInt16 nTol=(sal_uInt16)nHitTolLog;
    sal_Bool bFnd=sal_False;
    sal_uIntPtr nMarkAnz=GetMarkedObjectCount();
    sal_uIntPtr nMarkNum;
    for (nMarkNum=nMarkAnz; nMarkNum>0 && !bFnd;) {
        nMarkNum--;
        SdrMark* pM=GetSdrMarkByIndex(nMarkNum);
        SdrPageView* pPV=pM->GetPageView();
        SdrObject* pObj=pM->GetMarkedSdrObj();
        bFnd = 0 != CheckSingleSdrObjectHit(aPt,nTol,pObj,pPV,SDRSEARCH_TESTMARKABLE,0);
        if (bFnd) {
            rpObj=pObj;
            rpPV=pPV;
            if (pnMarkNum!=NULL) *pnMarkNum=nMarkNum;
        }
    }
    if ((bBoundCheckOn2ndPass || bCheckNearestOn3rdPass) && !bFnd) {
        SdrObject* pBestObj=NULL;
        SdrPageView* pBestPV=NULL;
        sal_uIntPtr nBestMarkNum=0;
        sal_uIntPtr nBestDist=ULONG_MAX;
        for (nMarkNum=nMarkAnz; nMarkNum>0 && !bFnd;) {
            nMarkNum--;
            SdrMark* pM=GetSdrMarkByIndex(nMarkNum);
            SdrPageView* pPV=pM->GetPageView();
            SdrObject* pObj=pM->GetMarkedSdrObj();
            Rectangle aRect(pObj->GetCurrentBoundRect());
            aRect.Left  ()-=nTol;
            aRect.Top   ()-=nTol;
            aRect.Right ()+=nTol;
            aRect.Bottom()+=nTol;
            if (aRect.IsInside(aPt)) {
                bFnd=sal_True;
                rpObj=pObj;
                rpPV=pPV;
                if (pnMarkNum!=NULL) *pnMarkNum=nMarkNum;
            } else if (bCheckNearestOn3rdPass) {
                sal_uIntPtr nDist=0;
                if (aPt.X()<aRect.Left())   nDist+=aRect.Left()-aPt.X();
                if (aPt.X()>aRect.Right())  nDist+=aPt.X()-aRect.Right();
                if (aPt.Y()<aRect.Top())    nDist+=aRect.Top()-aPt.Y();
                if (aPt.Y()>aRect.Bottom()) nDist+=aPt.Y()-aRect.Bottom();
                if (nDist<nBestDist) {
                    pBestObj=pObj;
                    pBestPV=pPV;
                    nBestMarkNum=nMarkNum;
                }
            }
        }
        if (bCheckNearestOn3rdPass && !bFnd) {
            rpObj=pBestObj;
            rpPV=pBestPV;
            if (pnMarkNum!=NULL) *pnMarkNum=nBestMarkNum;
            bFnd=pBestObj!=NULL;
        }
    }
    return bFnd;
}

SdrHitKind SdrMarkView::PickSomething(const Point& rPnt, short nTol) const
{
    nTol=ImpGetHitTolLogic(nTol,NULL);
    SdrHitKind eRet=SDRHIT_NONE;
    Point aPt(rPnt);
    SdrObject* pObj=NULL;
    SdrPageView* pPV=NULL;
    if (eRet==SDRHIT_NONE && PickObj(rPnt,sal_uInt16(nTol),pObj,pPV,SDRSEARCH_PICKMARKABLE)) {
        Rectangle aRct1(aPt-Point(nTol,nTol),aPt+Point(nTol,nTol)); // HitRect fuer Toleranz
        Rectangle aBR(pObj->GetCurrentBoundRect());
        if      (aRct1.IsInside(aBR.TopLeft()))      eRet=SDRHIT_BOUNDTL;
        else if (aRct1.IsInside(aBR.TopCenter()))    eRet=SDRHIT_BOUNDTC;
        else if (aRct1.IsInside(aBR.TopRight()))     eRet=SDRHIT_BOUNDTR;
        else if (aRct1.IsInside(aBR.LeftCenter()))   eRet=SDRHIT_BOUNDCL;
        else if (aRct1.IsInside(aBR.RightCenter()))  eRet=SDRHIT_BOUNDCR;
        else if (aRct1.IsInside(aBR.BottomLeft()))   eRet=SDRHIT_BOUNDBL;
        else if (aRct1.IsInside(aBR.BottomCenter())) eRet=SDRHIT_BOUNDBC;
        else if (aRct1.IsInside(aBR.BottomRight()))  eRet=SDRHIT_BOUNDBR;
        else eRet=SDRHIT_OBJECT;
    }
    return eRet;
}

void SdrMarkView::UnmarkAllObj(SdrPageView* pPV)
{
    if (GetMarkedObjectCount()!=0) {
        BrkAction();
        //HMHBOOL bVis=bHdlShown;
        //HMHif (bVis) HideMarkHdl();
        if (pPV!=NULL)
        {
            GetMarkedObjectListWriteAccess().DeletePageView(*pPV);
        }
        else
        {
            GetMarkedObjectListWriteAccess().Clear();
        }
        pMarkedObj=NULL;
        pMarkedPV=NULL;
        MarkListHasChanged();
        AdjustMarkHdl(); //HMHTRUE);
        //HMHif (bVis) ShowMarkHdl(); // ggf. fuer die RefPoints
    }
}

void SdrMarkView::MarkAllObj(SdrPageView* _pPV)
{
    BrkAction();
    //HMHHideMarkHdl();

    if(!_pPV)
    {
        _pPV = GetSdrPageView();
    }

    // #i69171# _pPV may still be NULL if there is no SDrPageView (!), e.g. when inserting
    // other files
    if(_pPV)
    {
        const bool bMarkChg(GetMarkedObjectListWriteAccess().InsertPageView(*_pPV));

        if(bMarkChg)
        {
            MarkListHasChanged();
        }
    }

    if(GetMarkedObjectCount())
    {
        AdjustMarkHdl(); //HMHTRUE);
        //HMHShowMarkHdl();
    }
}

void SdrMarkView::AdjustMarkHdl() //HMHBOOL bRestraintPaint)
{
    //HMHBOOL bVis=bHdlShown;
    //HMHif (bVis) HideMarkHdl();
    CheckMarked();
    SetMarkRects();
    SetMarkHandles();
    //HMHif(bRestraintPaint && bVis)
    //HMH{
    //HMH   ShowMarkHdl();
    //HMH}
}

Rectangle SdrMarkView::GetMarkedObjBoundRect() const
{
    Rectangle aRect;
    for (sal_uIntPtr nm=0; nm<GetMarkedObjectCount(); nm++) {
        SdrMark* pM=GetSdrMarkByIndex(nm);
        SdrObject* pO=pM->GetMarkedSdrObj();
        Rectangle aR1(pO->GetCurrentBoundRect());
        if (aRect.IsEmpty()) aRect=aR1;
        else aRect.Union(aR1);
    }
    return aRect;
}

const Rectangle& SdrMarkView::GetMarkedObjRect() const
{
    if (bMarkedObjRectDirty) {
        ((SdrMarkView*)this)->bMarkedObjRectDirty=sal_False;
        Rectangle aRect;
        for (sal_uIntPtr nm=0; nm<GetMarkedObjectCount(); nm++) {
            SdrMark* pM=GetSdrMarkByIndex(nm);
            SdrObject* pO=pM->GetMarkedSdrObj();
            Rectangle aR1(pO->GetSnapRect());
            if (aRect.IsEmpty()) aRect=aR1;
            else aRect.Union(aR1);
        }
        ((SdrMarkView*)this)->aMarkedObjRect=aRect;
    }
    return aMarkedObjRect;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrMarkView::ImpTakeDescriptionStr(sal_uInt16 nStrCacheID, XubString& rStr, sal_uInt16 nVal, sal_uInt16 nOpt) const
{
    rStr = ImpGetResStr(nStrCacheID);
    xub_StrLen nPos = rStr.SearchAscii("%1");

    if(nPos != STRING_NOTFOUND)
    {
        rStr.Erase(nPos, 2);

        if(nOpt == IMPSDR_POINTSDESCRIPTION)
        {
            rStr.Insert(GetDescriptionOfMarkedPoints(), nPos);
        }
        else if(nOpt == IMPSDR_GLUEPOINTSDESCRIPTION)
        {
            rStr.Insert(GetDescriptionOfMarkedGluePoints(), nPos);
        }
        else
        {
            rStr.Insert(GetDescriptionOfMarkedObjects(), nPos);
        }
    }

    nPos = rStr.SearchAscii("%2");

    if(nPos != STRING_NOTFOUND)
    {
        rStr.Erase(nPos, 2);
        rStr.Insert(UniString::CreateFromInt32(nVal), nPos);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

sal_Bool SdrMarkView::EnterMarkedGroup()
{
    sal_Bool bRet=sal_False;
    // Es wird nur die erste gefundene Gruppe (also nur in einer PageView) geentert
    // Weil PageView::EnterGroup ein AdjustMarkHdl ruft.
    // Das muss ich per Flag mal unterbinden  vvvvvvvv
    SdrPageView* pPV = GetSdrPageView();

    if(pPV)
    {
        sal_Bool bEnter=sal_False;
        for (sal_uInt32 nm(GetMarkedObjectCount()); nm > 0 && !bEnter;)
        {
            nm--;
            SdrMark* pM=GetSdrMarkByIndex(nm);
            if (pM->GetPageView()==pPV) {
                SdrObject* pObj=pM->GetMarkedSdrObj();
                if (pObj->IsGroupObject()) {
                    if (pPV->EnterGroup(pObj)) {
                        bRet=sal_True;
                        bEnter=sal_True;
                    }
                }
            }
        }
    }
    return bRet;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrMarkView::MarkListHasChanged()
{
    GetMarkedObjectListWriteAccess().SetNameDirty();
    SetEdgesOfMarkedNodesDirty(); // bEdgesOfMarkedNodesDirty=sal_True;

    bMarkedObjRectDirty=sal_True;
    bMarkedPointsRectsDirty=sal_True;
#ifdef DBG_UTIL
    if (pItemBrowser!=NULL) pItemBrowser->SetDirty();
#endif
    sal_Bool bOneEdgeMarked=sal_False;
    if (GetMarkedObjectCount()==1) {
        const SdrObject* pObj=GetMarkedObjectByIndex(0);
        if (pObj->GetObjInventor()==SdrInventor) {
            sal_uInt16 nIdent=pObj->GetObjIdentifier();
            bOneEdgeMarked=nIdent==OBJ_EDGE;
        }
    }
    ImpSetGlueVisible4(bOneEdgeMarked);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrMarkView::SetMoveOutside(sal_Bool bOn)
{
    aHdl.SetMoveOutside(bOn);
}

sal_Bool SdrMarkView::IsMoveOutside() const
{
    return aHdl.IsMoveOutside();
}

void SdrMarkView::SetDesignMode( sal_Bool _bOn )
{
    if ( bDesignMode != _bOn )
    {
        bDesignMode = _bOn;
        SdrPageView* pPageView = GetSdrPageView();
        if ( pPageView )
            pPageView->SetDesignMode( _bOn );
    }
}

// MarkHandles Objektaenderung:
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// - Bei Notify mit HINT_OBJCHG (oder so) werden die Handles erstmal versteckt
//   (wenn nicht schon wegen Dragging versteckt).
// - XorHdl: Bei ModelHasChanged() werden sie dann wieder angezeigt.
// - PaintEvents kommen nun durch.
//   - Die XorHandles werden z.T. wieder uebermalt.
//   - Xor:  Nach dem Painten werden die Handles im (vom PaintHandler gerufenen)
//           CompleteRedraw per ToggleShownXor bei gesetzter ClipRegion nochmal gemalt
//           und damit ist alles in Butter.
//   - ToggleShownXor macht bei SolidHdl nix weil bHdlShown=FALSE
//   - Der AfterPaintTimer wird gestartet.
// - SolidHdl: Im AfterPaintHandler wird ShowMarkHdl gerufen.
//   Da die Handles zu diesem Zeitpunkt nicht angezeigt sind wird:
//   - SaveBackground durchgefuehrt.
//   - DrawMarkHdl gerufen und bHdlShown gesetzt.
//
// MarkHandles bei sonstigem Invalidate:
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// In diesem Fall bekomme ich kein Notify und beim Aufruf des
// PaintHandlers->CompleteRedraw() sind auch die SolidHandles sichtbar.

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
